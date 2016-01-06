# cpp_sdk
c++ sdk for 腾讯云万象图片服务

#linux等类UINX系统使用手册
##需要安装的库和工具
openssl: ubuntu下运行 sudo apt-get install libssl-dev 安装  
其他类uinx系统按照各自的方法安装好该库  


curl: 在 Ubuntu 12.04.2 LTS,linux 版本 3.13.0-32-generic，编译了.a放在lib目录下。  
      如果有问题，在http://curl.haxx.se/download/curl-7.43.0.tar.gz 下载源码，  
      编译生成 .a 或者 .so 放到 lib目录下，替换掉原来的libcurl.a  

jsoncpp： 在 Ubuntu 12.04.2 LTS,linux 版本 3.13.0-32-generic，编译了.a放在lib目录下。  
      如果有问题，在 https://github.com/open-source-parsers/jsoncpp 下载源码，  
      编译生成 .a 或者 .so 放到 lib目录下，替换掉原来的libjsoncpp.a  

cmake: 去http://www.cmake.org/download/ 下载cmake安装好即可  

##编译生成静态库.a
执行下面的命令  
cd ${cpp-sdk}  
mkdir -p build  
cd build  
cmake ..  
make  

需要将sample.cpp里的appid、secretId、secretKey、bucket等信息换成你自己的。  
生成的sample就可以直接运行，试用，  

生成的静态库，名称为:libimagesdk.a

##将生成的库链接进自己的项目
生成的libimagesdk.a放到你自己的工程里lib路径下，  
include目录下的 Auth.h  Imageapi.h curl  json都放到你自己的工程的include路径下。  

例如我的项目里只有一个sample.cpp,项目目录和sdk在同级目录，copy libimagesdk.a 到项目所在目录  
那么编译命令为:  
g++ -o sample sample.cpp -I ./include/ -L. -L../cpp-sdk/lib/ -limagesdk -lcurl -lcrypto -lssl -lrt -ljsoncpp

#windows系统咱不支持

How to start
----------------------------------- 
### 1. 在[腾讯云](http://app.qcloud.com) 申请业务的授权
授权包括：
		
	APP_ID 
	SECRET_ID
	SECRET_KEY
2.0版本的云服务在使用前，还需要先创建空间。在使用2.0 api时，需要使用空间名（BUCKET）。

### 2. 创建对应操作类的对象
使用接口前，必须调用：  
    Imageapi::global_init();  
如果要使用图片，需要创建图片操作类对象	
	//v1版本	
	Imageapi api(APP_ID_V1,	SECRET_ID_V1,SECRET_KEY_V1,30);
	//v2版本
	Imageapi api(APP_ID_V2,	SECRET_ID_V2, SECRET_KEY_V2, BUCKET,30);
	

### 3. 调用对应的方法
在创建完对象后，根据实际需求，调用对应的操作方法就可以了。sdk提供的方法包括：签名计算、上传、复制、查询、下载和删除等。

##签名
	//1. 多次签名
    string sign = Auth::appSign(
                        APP_ID, 
						SECRET_ID,
						SECRET_KEY,
                        expired, 
                        BUCKET,
						fileId);
	//1. 单次签名
    string sign = Auth::appSignOnce(
                        APP_ID, 
						SECRET_ID,
						SECRET_KEY,
                        BUCKET,
						fileId);					
						
#### 上传图片
	//1. 最简单的上传接口
	api.upload(srcPath);
	api.dump_res();
	//2. 可以自定义fileid的上传接口
	api.upload(srcPath,fileId);
	api.dump_res();
	
	//分片上传，适用于大图片,可以指定分片大小
    //如果中途失败，以相同的参数再次调用uploadSlice可以自动断点续传
    api.uploadSlice(
            srcPath, "", 128*1024);
    api.dump_res();


#### 复制图片		
	api.copy(fileId);
	api.dump_res();

#### 查询图片
	api.stat(fileId);
	api.dump_res();

#### 删除图片
	api.del(fileId);
	api.dump_res();

#### 下载图片
下载图片直接利用图片的下载url即可。
如果开启了防盗链，还需要在下载url后面追加签名，请参考腾讯云的wiki页，熟悉鉴权签名的算法。

#### 黄图识别
	api.pornDetect(url);
	api.dump_res();
	//返回结果的各字段含义，请参考官网wiki文档
	//http://www.qcloud.com/wiki/%E4%B8%87%E8%B1%A1%E4%BC%98%E5%9B%BE%E6%99%BA%E8%83%BD%E9%89%B4%E9%BB%84%E6%96%87%E6%A1%A3	