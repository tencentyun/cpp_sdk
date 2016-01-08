#include "Imageapi.h"
#include "cstring"

using namespace std;
using namespace Qcloud_image;

const int APP_ID_V1 = 201893;
const  string SECRET_ID_V1 = "AKIDAq4qH0S1npdWKarPPyuQv0nrnZdoPtWu";
const  string SECRET_KEY_V1 = "No6ZubvQGg8d8CDC8xQQyIMrSZm1Cm9X";
	
const int APP_ID_V2 = 10004877;
const  string SECRET_ID_V2 = "AKIDGEXDZ3dFhjp2BNyERA29yMp4XvRiYqUy";
const  string SECRET_KEY_V2 = "f4htD982j7tJjAXA9yHXdACmt9qaAtBJ";
const  string BUCKET = "testtest";		//空间名

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

void pronDemo(const string &url) {
	Imageapi::global_init();
	
	Imageapi api(APP_ID_V2,
			SECRET_ID_V2,
			SECRET_KEY_V2,
			BUCKET,30);
	
	cout<<"pornDetect-----------------"<<endl;
	api.pornDetect(url);
	api.dump_res();

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

int main () {
	signDemo();
	apiV1Demo("../v1.jpg");
	apiV2Demo("../v2.jpg");		
	pronDemo("http://b.hiphotos.baidu.com/image/pic/item/8ad4b31c8701a18b1efd50a89a2f07082938fec7.jpg");
}

