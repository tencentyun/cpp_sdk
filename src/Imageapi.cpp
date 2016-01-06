#include <fstream>
#include "cstdio"
#include <cstring>
#include <unistd.h>
#include <errno.h>

#include <openssl/sha.h>

#include "Imageapi.h"

namespace Qcloud_image{

size_t post_callback( 
        char *ptr, 
        size_t size, 
        size_t nmemb, 
        void *userdata) {

    Imageapi * ImageapiPtr = 
        static_cast<Imageapi*>(userdata);
    size_t byte_num = size * nmemb;
    char tmp[byte_num+1];
    memcpy(tmp, ptr, byte_num);
    tmp[byte_num] = 0;
    ImageapiPtr->response_str += tmp;
    return byte_num;
}

unsigned char ToHex(unsigned char x)
{
    static char table[16] = {
        '0', '1', '2', '3', '4', '5', '6', '7',
        '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
    };
    return table[x];
}

//特殊的urlEncode，'/' 不转义
std::string cosUrlEncode(const std::string& str)
{
    std::string strTemp = "";
    size_t length = str.length();
    for (size_t i = 0; i < length; i++)
    {
        if (isalnum((unsigned char)str[i]) ||
            (str[i] == '-') ||
            (str[i] == '_') ||
            (str[i] == '.') ||
            (str[i] == '~') ||(str[i] == '/'))

            strTemp += str[i];
        else
        {
            strTemp += '%';
            strTemp += ToHex((unsigned char)str[i] >> 4);
            strTemp += ToHex((unsigned char)str[i] % 16);
        }
    }
    return strTemp;
}

string genFileSHA1AndLen(
        const string &filePath,
        uint64_t *fileLen = NULL) {

    static char hexBytes[16] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8',
    '9', 'a', 'b', 'c', 'd', 'e', 'f'};

    unsigned char sha1[SHA_DIGEST_LENGTH];
    ifstream fileInput(filePath.c_str(),
            ios::in | ios::binary);
    if (!fileInput) {
        return "";
    }

    if (fileLen != NULL) {
        fileInput.seekg(0 , std::ios::end);
        *fileLen = fileInput.tellg();
        fileInput.seekg(0 , std::ios::beg);
    }

    const int sliceSize = 2*1024*1024;
    char buf[sliceSize];
    SHA_CTX ctx;
    SHA1_Init(&ctx);

    uint64_t len = 0;
    while(!fileInput.eof()) {
        fileInput.read(buf, sliceSize);
        len = fileInput.gcount();
        SHA1_Update(
                &ctx, buf, len);
    }

    SHA1_Final(sha1, &ctx);

    fileInput.close();

    string shaStr = "";
    for (int i=0; i<SHA_DIGEST_LENGTH; i++) {
        shaStr += hexBytes[sha1[i] >> 4];
        shaStr += hexBytes[sha1[i] & 0x0f];
    }

    return shaStr;
}

const int Imageapi::EXPIRED_SECONDS = 60;
const int Imageapi::DEFAULT_SLICE_SIZE = 3145728;
const int Imageapi::MIN_SLICE_FILE_SIZE = 10485760;
const int Imageapi::MAX_RETRY_TIMES = 3;
const string Imageapi::API_IMAGEAPI_V2_END_POINT = 
    "http://web.image.myqcloud.com/photos/v2/";
const string Imageapi::API_IMAGEAPI_V1_END_POINT = 
    "http://web.image.myqcloud.com/photos/v1/";
const string Imageapi::API_IMAGEAPI_PRONDETECT_END_POINT = 
    "http://service.image.myqcloud.com/detection/pornDetect";

int32_t Imageapi::global_init() {
    CURLcode retCode;
    retCode = curl_global_init(CURL_GLOBAL_ALL);
    if (CURLE_OK != retCode){
        return -1;
    }
    return 0;
}

void Imageapi::global_finit() {
    curl_global_cleanup();
    return;
}

void Imageapi::reset() {
    response_str = "";
    retCode = 0;
    retMsg = "";
    retJson.clear();
}

void Imageapi::dump_res() {
    cout << "retJson:" << retJson << endl;
    cout << "retCode:" << retCode << endl;
    cout << "retMsg:" << retMsg << endl;
}

Imageapi::Imageapi(
        const uint64_t appid, 
        const string &secretId,
        const string &secretKey,
        uint64_t timeout)
            :APPID(appid),
            SECRET_ID(secretId),
            SECRET_KEY(secretKey) {

    _curl_handle = curl_easy_init();
    _timeout = timeout;
}

Imageapi::Imageapi(
        const uint64_t appid, 
        const string &secretId,
        const string &secretKey,
        const string &bucketName,
        uint64_t timeout)
            :APPID(appid),
            SECRET_ID(secretId),
            SECRET_KEY(secretKey),
            BUCKET_NAME(bucketName)
            {

    _curl_handle = curl_easy_init();
    _timeout = timeout;
}


Imageapi::~Imageapi() {
    curl_easy_cleanup(_curl_handle);
}

string Imageapi::generateResUrl(
        const string &fileId,
        const uint64_t userId) {

    string url = "";
    char urlBytes[10240];
	if(!BUCKET_NAME.empty())
	{
    	snprintf(urlBytes, sizeof(urlBytes),
#if __WORDSIZE == 64
            "%s%lu/%s/%lu/%s",
#else
            "%s%llu/%s/%llu/%s",
#endif
            API_IMAGEAPI_V2_END_POINT.c_str(),
            APPID, 
            BUCKET_NAME.c_str(), 
            userId,
            fileId.c_str());
	}
	else
	{
    	snprintf(urlBytes, sizeof(urlBytes),
#if __WORDSIZE == 64
            "%s%lu/%lu/%s",
#else
            "%s%llu/%llu/%s",
#endif
            API_IMAGEAPI_V1_END_POINT.c_str(),
            APPID, 
            userId,
            fileId.c_str());
	}
	
    url = urlBytes;
    return url;
}
int Imageapi::sendRequest(
        const string &url,
        const int isPost,
        const vector<string> *headers,
        const char *data,
        struct curl_httppost * form_data) {
    curl_easy_reset(_curl_handle);

    CURLcode curl_ret = CURLE_OK;

    curl_easy_setopt(
            _curl_handle, CURLOPT_URL, url.c_str());
    curl_easy_setopt(
            _curl_handle, CURLOPT_TIMEOUT, _timeout);

    if (isPost) {
            curl_easy_setopt(
            _curl_handle, CURLOPT_CUSTOMREQUEST, "POST");          
        if (data) {
            curl_easy_setopt(				
                _curl_handle, CURLOPT_POSTFIELDS, data);
        }

        if (form_data) {
            curl_easy_setopt(
                _curl_handle, CURLOPT_HTTPPOST, form_data);
        }
    }

    struct curl_slist *list = NULL;
    if (headers) {
        vector<string>::const_iterator it = 
            headers->begin();
        while (it != headers->end()) {
            list = curl_slist_append(
                    list, it->c_str());
            it++;
        }
        curl_easy_setopt(
                _curl_handle, CURLOPT_HTTPHEADER, list);

    }
    if (isPost) {
        list = curl_slist_append(
                    list, "Expect: ");
    }

	list = curl_slist_append(list, "Connection: Keep-Alive");

    curl_easy_setopt(
            _curl_handle, CURLOPT_SSL_VERIFYHOST, 0);
    curl_easy_setopt(
            _curl_handle, CURLOPT_SSL_VERIFYPEER, 1);

    curl_easy_setopt(
            _curl_handle, CURLOPT_WRITEFUNCTION, 
            post_callback);
    curl_easy_setopt(
            _curl_handle, CURLOPT_WRITEDATA, 
            (void*)this );
    curl_ret = curl_easy_perform(_curl_handle);
    curl_slist_free_all(list);
    Json::Reader reader;
    if (reader.parse(response_str, retJson)) {
        retCode = retJson["code"].asInt();
        retMsg = retJson["message"].asString();
    } else {
        if (response_str.empty()) {
            response_str = "connect failed!";
        }
        retCode = IMAGEAPI_NETWORK_ERROR;
        retMsg = response_str;
        retJson["code"] = retCode;
        retJson["message"] = retMsg;
    }
    
    return curl_ret;
}

int Imageapi::upload(
        const string &srcPath,
        const string &fileId,
        const string &magicContext) {
    reset();
    int ret = 0;
    ret = access(srcPath.c_str(), F_OK | R_OK);
    if (ret != 0) {
        retCode = IMAGEAPI_FILE_NOT_EXISTS;
        retMsg = "file not exist or can not be read...";
        retJson["code"] = retCode;
        retJson["message"] = retMsg;
        return retCode;
    }

    uint64_t expired = time(NULL) + EXPIRED_SECONDS;
    string url = generateResUrl(fileId);
	  
    string sign =
        Auth::appSign(
                APPID, SECRET_ID, SECRET_KEY,
                expired, BUCKET_NAME,fileId);

    vector<string> headers;
    headers.push_back("Authorization: " + sign);

    string sha1 = genFileSHA1AndLen(srcPath);

    struct curl_httppost *firstitem = NULL,
                         *lastitem = NULL;
	if(!magicContext.empty())
	{
		ret = curl_formadd(&firstitem, &lastitem,
        	CURLFORM_COPYNAME, "MagicContext",
        	CURLFORM_COPYCONTENTS, "magicContext",
        	CURLFORM_END);
	}

    ret = curl_formadd(&firstitem, &lastitem,
            CURLFORM_COPYNAME, "filecontent",
            CURLFORM_FILE, srcPath.c_str(),
            CURLFORM_END);

    sendRequest(url, 1, &headers, NULL, firstitem);
    curl_formfree(firstitem);
    return retCode;
}


int Imageapi::uploadSlice(
        const string &srcPath,
        const string &fileId,
        const uint64_t sliceSize,
        const string &session,
        const string &magicContext)
{
	reset();
	int ret = 0;
	ret = access(srcPath.c_str(), F_OK | R_OK);
	if (ret != 0) {
		retCode = IMAGEAPI_FILE_NOT_EXISTS;
		retMsg = "file not exist or can not be read...";
		retJson["code"] = retCode;
		retJson["message"] = retMsg;
		return retCode;
	}

	uint64_t expired = time(NULL) + EXPIRED_SECONDS;
    string url = generateResUrl(fileId);
	  
    string sign =
        Auth::appSign(
                APPID, SECRET_ID, SECRET_KEY,
                expired, BUCKET_NAME,fileId);

    vector<string> headers;
    headers.push_back("Authorization: " + sign);

	uint64_t fileSize = 0;
    string sha1 = genFileSHA1AndLen(srcPath, &fileSize);

	uploadPrepare(fileSize, sha1, 
			  sign, url,magicContext, session, sliceSize);

	  if (retCode != 0) {
		  return retCode;
	  }

	  if (retJson["data"].isMember("url")) {
		  //秒传成功，直接返回了url
		  return retCode;
	  } 

	  if (!retJson["data"].isMember("offset")
			  || !retJson["data"].isMember("session")
			  || !retJson["data"].isMember("slice_size")) {
		  return retCode;
	  }

	  uploadData(fileSize, sha1, 
		  retJson["data"]["slice_size"].asUInt64(),
		  sign, url, srcPath, 
		  retJson["data"]["offset"].asUInt64(),
		  retJson["data"]["session"].asString());

  	return retCode;

}

int Imageapi::uploadPrepare(
        const uint64_t fileSize,
        const string &sha,
        const string &sign,
        const string &url,
        const string &magicContext,
        const string &session,
        const uint64_t sliceSize
        ) {
    reset();

    int ret = 0;
    char buf[1024];
    struct curl_httppost *firstitem = NULL,
                         *lastitem = NULL;

    ret = curl_formadd(&firstitem, &lastitem,
            CURLFORM_COPYNAME, "op",
            CURLFORM_COPYCONTENTS, "upload_slice",
            CURLFORM_END);

#if __WORDSIZE == 64
    snprintf(buf, sizeof(buf), "%lu", fileSize);
#else
    snprintf(buf, sizeof(buf), "%llu", fileSize);
#endif
    ret = curl_formadd(&firstitem, &lastitem,
            CURLFORM_COPYNAME, "filesize",
            CURLFORM_COPYCONTENTS, buf,
            CURLFORM_END);

    ret = curl_formadd(&firstitem, &lastitem,
            CURLFORM_COPYNAME, "sha",
            CURLFORM_COPYCONTENTS, sha.c_str(),
            CURLFORM_END);

    if (!magicContext.empty()) {
        ret = curl_formadd(&firstitem, &lastitem,
                CURLFORM_COPYNAME, "magicContext",
                CURLFORM_COPYCONTENTS, magicContext.c_str(),
                CURLFORM_END);
    }

    if (!session.empty()) {
        ret = curl_formadd(&firstitem, &lastitem,
                CURLFORM_COPYNAME, "session",
                CURLFORM_COPYCONTENTS, session.c_str(),
                CURLFORM_END);
    }

    if (sliceSize > 0) {
#if __WORDSIZE == 64
        snprintf(buf, sizeof(buf), "%lu", sliceSize);
#else
        snprintf(buf, sizeof(buf), "%llu", sliceSize);
#endif

        ret = curl_formadd(&firstitem, &lastitem,
                CURLFORM_COPYNAME, "slice_size",
                CURLFORM_COPYCONTENTS, buf,
                CURLFORM_END);
    }

    vector<string> headers;
    headers.push_back("Authorization: " + sign);

    sendRequest(url, 1, &headers, NULL, firstitem);
    curl_formfree(firstitem);
    return retCode;
}

int Imageapi::uploadData(
        const uint64_t fileSize,
        const string &sha,
        const uint64_t sliceSize,
        const string &sign,
        const string &url,
        const string &srcPath,
        const uint64_t offset,
        const string &session
        ) {

	int ret = 0;
    char tmp_buf[1024];
    char buf[sliceSize];

    vector<string> headers;
    headers.push_back("Authorization: " + sign);

    ifstream fileInput(srcPath.c_str(),
            ios::in | ios::binary);

    fileInput.seekg(offset);

    uint64_t pos = offset;
    while (fileSize > pos) {
        reset();
        uint64_t len =0;
        fileInput.read(buf, sliceSize); 
        len = fileInput.gcount();

        struct curl_httppost *firstitem = NULL,
                             *lastitem = NULL;

        ret = curl_formadd(&firstitem, &lastitem,
                CURLFORM_COPYNAME, "op",
                CURLFORM_COPYCONTENTS, "upload_slice",
                CURLFORM_END);

#if __WORDSIZE == 64
        snprintf(tmp_buf, sizeof(tmp_buf), "%lu", pos);
#else
        snprintf(tmp_buf, sizeof(tmp_buf), "%llu", pos);
#endif
        ret = curl_formadd(&firstitem, &lastitem,
                CURLFORM_COPYNAME, "offset",
                CURLFORM_COPYCONTENTS, tmp_buf,
                CURLFORM_END);

        ret = curl_formadd(&firstitem, &lastitem,
                CURLFORM_COPYNAME, "session",
                CURLFORM_COPYCONTENTS, session.c_str(),
                CURLFORM_END);

        ret = curl_formadd(&firstitem, &lastitem,
                CURLFORM_COPYNAME, "filecontent",
                CURLFORM_BUFFER, "data",
                CURLFORM_BUFFERPTR, buf,
                CURLFORM_BUFFERLENGTH, (long)len,
                CURLFORM_END);

        int retry_times = 0;
        do {
            sendRequest(
                    url, 1, &headers, 
                    NULL, firstitem);
            if (retCode == 0) {
                break;
            }
            retry_times++;
        } while(retry_times < MAX_RETRY_TIMES);

        curl_formfree(firstitem);

        if (retCode != 0) {
            return retCode;
        }

        pos += sliceSize;
    }

    return retCode;
}

int Imageapi::stat(
        const string &fileId,
        const uint64_t userId)
{
    reset();
    int ret = 0;
    
    uint64_t expired = time(NULL) + EXPIRED_SECONDS;
    string url = generateResUrl(fileId, userId);
    string sign = Auth::appSign(
    			  APPID, SECRET_ID, SECRET_KEY,
   				  expired, BUCKET_NAME,fileId);
    
    vector<string> headers;
    headers.push_back("Authorization: " + sign);
    
    sendRequest(url, 0, &headers);
	
    return retCode;
    
}

int Imageapi::copy(
        const string &fileId,
        const uint64_t userId)
{
    reset();
    int ret = 0;
    
    string url = generateResUrl(fileId, userId) + "/copy";
    string sign = Auth::appSignOnce(
    			  APPID, SECRET_ID, SECRET_KEY,
   				  BUCKET_NAME,fileId);
    vector<string> headers;
    headers.push_back("Authorization: " + sign);
    
    sendRequest(url, 1, &headers);
	
    return retCode;
    
}

int Imageapi::del(
        const string &fileId,
        const uint64_t userId)

{
    reset();
    int ret = 0;

    string url = generateResUrl(fileId, userId) + "/del";
    string sign = Auth::appSignOnce(
    			  APPID, SECRET_ID, SECRET_KEY,
   				  BUCKET_NAME,fileId);
    vector<string> headers;
    headers.push_back("Authorization: " + sign);
    
    sendRequest(url, 1, &headers);
	
    return retCode;   
}

int Imageapi::pornDetect(
        const string &pronUrl)
 {
	reset();
	int ret = 0;
	char buf[1024]; 
	uint64_t expired = time(NULL) + EXPIRED_SECONDS;
	  
	string sign =
		Auth::appPornDetectSign(
				APPID, SECRET_ID, SECRET_KEY,
				expired, BUCKET_NAME,pronUrl);

	vector<string> headers;
	headers.push_back("Authorization: " + sign);
	headers.push_back("Content-Type: application/json");

    int appid = APPID;
	Json::Value reqJson;
    reqJson["bucket"] = BUCKET_NAME;
	reqJson["url"] = pronUrl;
	reqJson["appid"] = appid;

	 Json::FastWriter writer;
    string data = writer.write(reqJson);

	retCode = sendRequest(API_IMAGEAPI_PRONDETECT_END_POINT, 1, &headers, data.c_str());

	return retCode;
}

}//namespace Qcloud_image
