#include <iostream>
#include <string>
#include <vector>
#include <stdint.h>

#include "json/json.h"
#include "curl/curl.h"

#include "Auth.h"

#ifndef _TENCENTYUN_IMAGE_IMAGEAPI_H_H_
#define _TENCENTYUN_IMAGE_IMAGEAPI_H_H_

using namespace std;

namespace Qcloud_image{

class Imageapi
{

public:
    static const string API_IMAGEAPI_V2_END_POINT;
	static const string API_IMAGEAPI_V1_END_POINT;
	static const string API_IMAGEAPI_PRONDETECT_END_POINT;

    static const int EXPIRED_SECONDS;
    static const int DEFAULT_SLICE_SIZE;
    static const int MIN_SLICE_FILE_SIZE;
    static const int MAX_RETRY_TIMES;
    
    const uint64_t APPID;
    const string SECRET_ID;
    const string SECRET_KEY;
	const string BUCKET_NAME;

	enum IMAGEAPI_ERR_T {
        IMAGEAPI_FILE_NOT_EXISTS = -1,
        IMAGEAPI_NETWORK_ERROR = -2,
        IMAGEAPI_PARAMS_ERROR = -3,
        IMAGEAPI_ILLEGAL_SLICE_SIZE_ERROR = -4
    };

    static int32_t global_init();
    static void global_finit();

    void reset();

    Imageapi(
        const uint64_t appid, 
        const string &secretId,
        const string &secretKey,
        const string &bucketName,
        const uint64_t timeout = 30);
		
	 Imageapi(
        const uint64_t appid, 
        const string &secretId,
        const string &secretKey,       
       	const uint64_t timeout = 30);
	 
    virtual ~Imageapi();

    int sendRequest(
            const string &url,
            const int isPost,
            const vector<string> *headers,
            const char *data = NULL,
            struct curl_httppost * form_data = NULL);

    string generateResUrl(
			const string &fileId,
			const uint64_t userId=0);

    int upload(
       	const string &srcPath,
        const string &fileId="",
        const string &magicContext="");

	int uploadPrepare(
			const uint64_t fileSize,
			const string &sha,
			const string &sign,
			const string &url,
			const string &magicContext="",
			const string &session="",
			const uint64_t sliceSize=0
			);
	
	int uploadSlice(
        const string &srcPath,
        const string &fileId="",
        const uint64_t sliceSize=0,
        const string &session="",
        const string &magicContext="");

	int uploadData(
        const uint64_t fileSize,
        const string &sha,
        const uint64_t sliceSize,
        const string &sign,
        const string &url,
        const string &srcPath,
        const uint64_t offset,
        const string &session );

	int stat(
        const string &fileId,
        const uint64_t userId=0);

	int del(
        const string &fileId,
        const uint64_t userId=0);

	int copy(
        const string &fileId,
        const uint64_t userId=0);

	int pornDetect(
        const string &pronUrl);

    void dump_res();

protected:
    CURL * _curl_handle;
    uint64_t _timeout;

public:
    string response_str;

    int retCode;
    string retMsg;
    Json::Value retJson;

};


}//namespace Qcloud_image

#endif
