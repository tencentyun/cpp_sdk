#include "Auth.h"
#include <assert.h>
#include <cstring>
#include <openssl/hmac.h>
#include <sys/time.h>

namespace Qcloud_image{

const int Auth::AUTH_URL_FORMAT_ERROR = -1;
const int Auth::AUTH_SECRET_ID_KEY_ERROR = -2;

int HmacEncode(const char * algo,  
                const char * key, unsigned int key_length,  
                const char * input, unsigned int input_length,  
                unsigned char * &output, unsigned int &output_length) {  
        const EVP_MD * engine = NULL;  
        if(strcasecmp("sha512", algo) == 0) {  
                engine = EVP_sha512();  
        }  
        else if(strcasecmp("sha256", algo) == 0) {  
                engine = EVP_sha256();  
        }  
        else if(strcasecmp("sha1", algo) == 0) {  
                engine = EVP_sha1();  
        }  
        else if(strcasecmp("md5", algo) == 0) {  
                engine = EVP_md5();  
        }  
        else if(strcasecmp("sha224", algo) == 0) {  
                engine = EVP_sha224();  
        }  
        else if(strcasecmp("sha384", algo) == 0) {  
                engine = EVP_sha384();  
        }  
        else if(strcasecmp("sha", algo) == 0) {  
                engine = EVP_sha();  
        }  
        else {  
                cout << "Algorithm " << algo << " is not supported by this program!" << endl;  
                return -1;  
        }  
  
        output = (unsigned char*)malloc(EVP_MAX_MD_SIZE);  
  
        HMAC_CTX ctx;  
        HMAC_CTX_init(&ctx);  
        HMAC_Init_ex(&ctx, key, strlen(key), engine, NULL);  
        HMAC_Update(&ctx, (unsigned char*)input, strlen(input));        // input is OK; &input is WRONG !!!  
  
        HMAC_Final(&ctx, output, &output_length);  
        HMAC_CTX_cleanup(&ctx);  
  
        return 0;  
}  

static const std::string base64_chars =   
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"  
             "abcdefghijklmnopqrstuvwxyz"  
             "0123456789+/";  
  
  
static inline bool is_base64(unsigned char c) {  
  return (isalnum(c) || (c == '+') || (c == '/'));  
}  
  
std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len) {  
  std::string ret;  
  int i = 0;  
  int j = 0;  
  unsigned char char_array_3[3];  
  unsigned char char_array_4[4];  
  
  while (in_len--) {  
    char_array_3[i++] = *(bytes_to_encode++);  
    if (i == 3) {  
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;  
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);  
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);  
      char_array_4[3] = char_array_3[2] & 0x3f;  
  
      for(i = 0; (i <4) ; i++)  
        ret += base64_chars[char_array_4[i]];  
      i = 0;  
    }  
  }  
  
  if (i)  
  {  
    for(j = i; j < 3; j++)  
      char_array_3[j] = '\0';  
  
    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;  
    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);  
    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);  
    char_array_4[3] = char_array_3[2] & 0x3f;  
  
    for (j = 0; (j < i + 1); j++)  
      ret += base64_chars[char_array_4[j]];  
  
    while((i++ < 3))  
      ret += '=';  
  
  }  
  
  return ret;  
  
}  

unsigned char ToHexAuth(unsigned char x) 
{ 
    return  x > 9 ? x + 55 : x + 48; 
}

unsigned char FromHex(unsigned char x) 
{ 
    unsigned char y;
    if (x >= 'A' && x <= 'Z') y = x - 'A' + 10;
	else if (x >= 'a' && x <= 'z') y = x - 'a' + 10;
	else if (x >= '0' && x <= '9') y = x - '0';
	else assert(0);
	return y;
}

string UrlEncode(const string& str)
{
	std::string strTemp = "";
	size_t length = str.length();
	for (size_t i = 0; i < length; i++)
	{
		if (isalnum((unsigned char)str[i]) || 
				(str[i] == '-') ||
				(str[i] == '_') || 
				(str[i] == '.') || 
				(str[i] == '~'))
			strTemp += str[i];
		else if (str[i] == ' ')
			strTemp += "+";
		else
		{
			strTemp += '%';
			strTemp += ToHexAuth((unsigned char)str[i] >> 4);
			strTemp += ToHexAuth((unsigned char)str[i] % 16);
		}
	}
	return strTemp;
}

string UrlDecode(const string& str)
{
	std::string strTemp = "";
	size_t length = str.length();
	for (size_t i = 0; i < length; i++)
	{
		if (str[i] == '+') strTemp += ' ';
		else if (str[i] == '%')
		{
			assert(i + 2 < length);
			unsigned char high = FromHex((unsigned char)str[++i]);
			unsigned char low = FromHex((unsigned char)str[++i]);
			strTemp += high*16 + low;
		}
		else strTemp += str[i];
	}
	return strTemp;
}

string Auth::appSign(
    const uint64_t appId, 
    const string &secretId,
    const string &secretKey,
    const uint64_t expired,
    const string &bucketName,
    const string &fileId) {

    return appSignBase(
            appId, secretId, secretKey, 
            expired, bucketName,fileId);
}

string Auth::appSignOnce(
    const uint64_t appId, 
    const string &secretId,
    const string &secretKey,
    const string &bucketName,
    const string &fileId
    ) {
    return 	appSignBase(
            appId, secretId, secretKey, 
            0, bucketName,fileId);
}


string Auth::appSignBase(
    const uint64_t appId, 
    const string &secretId,
    const string &secretKey,
    const uint64_t expired,    
    const string &bucketName,
    const string &fileId,
    const uint64_t userId
    ) {
    if (secretId.empty() || secretKey.empty()) {
        return "";
    }

	time_t now = time(NULL);
    struct timeval tv;
	gettimeofday(&tv, NULL);
    unsigned int seed = tv.tv_usec;
    uint64_t rdm = rand_r(&seed);
    char plainText[10240];
    unsigned char * output = NULL;
    unsigned int output_len = 0;

    unsigned int input_length;
if(!bucketName.empty())
{
	input_length = snprintf(plainText, 10240, 
#if __WORDSIZE == 64
            "a=%lu&b=%s&k=%s&e=%lu&t=%lu&r=%lu&u=%lu&f=%s",
#else
            "a=%llu&b=%s&k=%s&e=%llu&t=%lu&r=%llu&u=%llu&f=%s",
#endif
            appId, bucketName.c_str(),secretId.c_str(), expired,
            now, rdm, userId,fileId.c_str());
}
else
{
		input_length = snprintf(plainText, 10240, 
#if __WORDSIZE == 64
				"a=%lu&k=%s&e=%lu&t=%lu&r=%lu&u=%lu&f=%s",
#else
				"a=%llu&k=%s&e=%llu&t=%lu&r=%llu&u=%llu&f=%s",
#endif
				appId,secretId.c_str(), expired,
				now, rdm, userId,fileId.c_str());

}

    int ret = HmacEncode("SHA1", 
            secretKey.c_str(), secretKey.length(),
            plainText, input_length,
            output, output_len);

    if (ret != 0) {
        return "";
    }

    unsigned char bin[10240];
    unsigned int bin_len = 0;
    memcpy(bin, output, output_len);
    memcpy(bin + output_len, plainText, input_length);
    bin_len = output_len + input_length;
    free(output);

    string sign = base64_encode(bin, bin_len);

    return sign;
} 

string Auth::appPornDetectSign(
    const uint64_t appId, 
    const string &secretId,
    const string &secretKey,
    const uint64_t expired,    
    const string &bucketName,
    const string &url
    ) {
    if (secretId.empty() || secretKey.empty()) {
        return "";
    }

    time_t now = time(NULL);
    unsigned int seed = now;
    uint64_t rdm = rand_r(&seed);
    char plainText[10240];
    unsigned char * output = NULL;
    unsigned int output_len = 0;

    unsigned int input_length;

	input_length = snprintf(plainText, 10240, 
#if __WORDSIZE == 64
            "a=%lu&b=%s&k=%s&t=%lu&e=%lu&l=%s",
#else
            "a=%llu&b=%s&k=%s&t=%lu&e=%llu&l=%s",
#endif
            appId, bucketName.c_str(),secretId.c_str(), 
            now, expired, UrlEncode(url).c_str());

    int ret = HmacEncode("SHA1", 
            secretKey.c_str(), secretKey.length(),
            plainText, input_length,
            output, output_len);

    if (ret != 0) {
        return "";
    }

    unsigned char bin[10240];
    unsigned int bin_len = 0;
    memcpy(bin, output, output_len);
    memcpy(bin + output_len, plainText, input_length);
    bin_len = output_len + input_length;
    free(output);

    string sign = base64_encode(bin, bin_len);

    return sign;
} 



}//namespace Qcloud_cos
