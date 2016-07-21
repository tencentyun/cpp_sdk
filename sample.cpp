#include "Imageapi.h"
#include "cstring"

using namespace std;
using namespace Qcloud_image;

const int APP_ID_V1 = 111;
const  string SECRET_ID_V1 = "SECRET_ID_V1";
const  string SECRET_KEY_V1 = "SECRET_KEY_V1";
	
const int APP_ID_V2 = 111;
const  string SECRET_ID_V2 = "SECRET_ID_V2";
const  string SECRET_KEY_V2 = "SECRET_KEY_V2";
const  string BUCKET = "BUCKET";		//空间名

void picBase(Imageapi &api, const string &srcPath)
{
	cout<<"upload-----------------"<<endl;
	api.upload(srcPath);
	api.dump_res();
	
	/*
	cout<<"upload-----------------"<<endl;
	api.uploadSlice(srcPath);
	api.dump_res();
	*/
	
	if(0 == api.retCode)
	{
		string fileId = api.retJson["data"]["fileid"].asString();
		string downloadUrl = api.retJson["data"]["download_url"].asString();

		cout<<"stat-----------------"<<endl;
		// 查询管理信息
		api.stat(fileId);
		api.dump_res();

		cout<<"copy-----------------"<<endl;
		// 查询管理信息
		api.copy(fileId);
		api.dump_res();

		cout<<"del-----------------"<<endl;
		//删除图片
		api.del(fileId);
		api.dump_res();
	}

}

void apiV2Demo(const string &srcPath) {
	Imageapi::global_init();
	
	Imageapi api(APP_ID_V2,
			SECRET_ID_V2,
			SECRET_KEY_V2,
			BUCKET);
	
	picBase(api, srcPath);

	Imageapi::global_finit();
	
	return;	
}

void apiV1Demo(const string &srcPath) {
	Imageapi::global_init();
	
	Imageapi api(APP_ID_V1,
			SECRET_ID_V1,
			SECRET_KEY_V1);
	
	picBase(api, srcPath);

	Imageapi::global_finit();
	
	return;	
}

void signDemo() {  
	string signV1 = Auth::appSign(
            APP_ID_V1, SECRET_ID_V1,
            SECRET_KEY_V1,
            time(NULL) + 3600);
    cout << "signV1:" << signV1 << endl;

	string signOnceV1 = Auth::appSignOnce(
            APP_ID_V1, SECRET_ID_V1,
            SECRET_KEY_V1);
    cout << "signOnceV1:" << signOnceV1 << endl;

	string signV2 = Auth::appSign(
        APP_ID_V2, SECRET_ID_V2,
        SECRET_KEY_V2,
        time(NULL) + 3600, BUCKET);
    cout << "sign:" << signV2 << endl;
	
	string signOnceV2 = Auth::appSignOnce(
        APP_ID_V2, SECRET_ID_V2,
        SECRET_KEY_V2,BUCKET);
    cout << "signOnceV2:" << signOnceV2 << endl;
	
	
	return;	
}

void pornDemo() {
    Imageapi::global_init();

    Imageapi api(APP_ID_V2,
            SECRET_ID_V2,
            SECRET_KEY_V2,
            BUCKET,30);
    string url = "http://b.hiphotos.baidu.com/image/pic/item/8ad4b31c8701a18b1efd50a89a2f07082938fec7.jpg";
    cout<<"pornDetect-----------------"<<endl;
    api.pornDetect(url);
    api.dump_res();

    Imageapi::global_finit();

    return; 
}

void pornUrlDemo() {
	Imageapi::global_init();
	Imageapi api(APP_ID_V2,
			SECRET_ID_V2,
			SECRET_KEY_V2,
            BUCKET,30);
    string pornUrl[] = {"http://b.hiphotos.baidu.com/image/pic/item/8ad4b31c8701a18b1efd50a89a2f07082938fec7.jpg",
                        "http://c.hiphotos.baidu.com/image/h%3D200/sign=7b991b465eee3d6d3dc680cb73176d41/96dda144ad3459829813ed730bf431adcaef84b1.jpg"};
	cout<<"pornDetect-----------------"<<endl;
	api.pornDetectUrl(pornUrl, 2);
	api.dump_res();
	Imageapi::global_finit();
	return;	
}

void pornFileDemo() {
	Imageapi::global_init();
	Imageapi api(APP_ID_V2,
			SECRET_ID_V2,
			SECRET_KEY_V2,
            BUCKET,30);
    string pornFile[] = {"../test1.jpg",
                         "../test2.jpg",
                         "../test3.png"};
	cout<<"pornDetect-----------------"<<endl;
	api.pornDetectFile(pornFile, 3);
	api.dump_res();
	Imageapi::global_finit();
	return;	
}

int main () {
	//signDemo();
	//apiV1Demo("../v1.jpg");
	//apiV2Demo("../v2.jpg");		
    pornDemo();
    pornUrlDemo();
	pornFileDemo();
}

