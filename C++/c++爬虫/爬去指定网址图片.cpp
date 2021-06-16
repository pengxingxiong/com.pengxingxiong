/*
filename demo0.cpp
make g++ demo0.cpp -l ws2_32 -l Urlmon -l Wininet
环境：windows
编译器 g++
编辑器 vscode
*/
#include <iostream>
#include <string>
#include <winsock2.h>
#include <windows.h>
#include <vector>
#include <locale>
#include <stdlib.h>
#include <Urlmon.h>
#include <Wininet.h>
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib, "Wininet.lib")
#pragma comment(lib,"Urlmon.lib")
#include <tchar.h>
#include <regex>//正则表达式
//存储主机名
char g_zhuji[256];
//存储主机名后的路径
char g_path[256];
//socket
SOCKET g_socket;
//图片序列
int count;
//图片文件夹名
std::string pathfile;
//保存所有图片地址
std::vector<std::string> g_photoAddr;
std::vector<std::string> g_htmlAddr;
/*
1、用户输入起始网址并保存
2、创建文件夹用来保存图片
3、遍历搜索（找所有网址，从网站下载图片）
    3.1从初始网址获取网页源代码
    3.2从网页源代码中解析出 图片地址 和网站地址
    3.3去除重复后保存到一个地方
    3.4下载图片
    3.5连接下一个网站
*/
//3.1.1解析网址，得到主机名
void jiexiAddr(char* addr){
    //http://www.win4000.com/meitu.html
    //协议前缀 http:// 
    //主机名 www.win4000.com
    //二级网址 meitu.html
    char* pos = strstr(addr,"http://"); //参数一中参数二部分的首地址
    char* pos1 = strstr(addr,"https://"); //参数一中参数二部分的首地址
    if(NULL == pos &&NULL == pos1){
        return;
    }
    else{
        if(pos==NULL){
            pos1 +=8;
            sscanf(pos1,"%[^/]%s",g_zhuji,g_path);//从字符串读取格式化输入
        }else{
            pos +=7;
            sscanf(pos,"%[^/]%s",g_zhuji,g_path);
        }
    }
    //"%[^/]%s"到斜杠为止
    std::cout<<"host "<<g_zhuji<<std::endl;
    std::cout<<"path "<<g_path<<std::endl;
}
//3.1.2 连接主机
void lianjieAddr(){
    //1 获取协议版本号
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2),&wsaData);
    if(LOBYTE(wsaData.wVersion)!=2 || HIBYTE(wsaData.wVersion)!=2){
        std::cout<<" failed"<<std::endl;
        return;
    }
    //2 创建socket
    g_socket = socket(AF_INET,SOCK_STREAM,0);
    if(INVALID_SOCKET == g_socket){
        std::cout<<" failed"<<std::endl;    //创建socket失败
        std::cout<<WSAGetLastError()<<std::endl;    //输出错误码
        return;
    }
    //3 拿到主机协议地址族
    sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    //4 绑定
    int r = bind(g_socket,(sockaddr*)&addr,sizeof addr);
    if(r==-1){
        std::cout<<" failed"<<std::endl;
        return;
    }
    //5 通过名字拿到ip地址,域名解析
    struct hostent* p = gethostbyname(g_zhuji);
    if(p==NULL){
        std::cout<<" failed"<<std::endl;
        return;
    }
    //6 地址放到协议地址族中
    memcpy(&addr.sin_addr,p->h_addr,4);
    addr.sin_port = htons(80);
    //7 连接服务器
    r = connect(g_socket,(sockaddr*)&addr,sizeof addr);
    if(-1==r){
        std::cout<<" failed"<<std::endl;
        return;
    }
    //8 通信
    std::string reqInfo="GET "+
						(std::string)g_path+
						" HTTP/1.1\r\nHost:"+(std::string)g_zhuji+
                        "\r\nConnection:Close\r\n\r\n";
    std::cout<<"请求头："<<reqInfo<<'\n'; 
    r = send(g_socket,reqInfo.c_str(),reqInfo.size(),0);
    if(-1 == r)
    {
        std::cout<<" failed"<<std::endl;
        return;
    }
    std::cout<<" send succeed!"<<std::endl;
 
 
}
//从网页源代码中解析网站地址
void getHtml(std::string& allHtmlData){
    std::smatch mat;
    std::regex pattern("href=\"(http://[^\\s\'\"]+)\"");
    std::string::const_iterator start = allHtmlData.begin();
    std::string::const_iterator end = allHtmlData.end();
    std::cout<<"网站地址解析"<<std::endl;
    while(std::regex_search(start,end,mat,pattern)){
        std::string msg(mat[1].first,mat[1].second);
        g_htmlAddr.push_back(msg);
        std::cout<<msg<<std::endl;
        start = mat[0].second;
    }
}
//从网页源代码中解析图片地址
void getImage(std::string& allHtmlData){
    std::smatch mat;
    std::regex pattern(" src=\"(.*?\\.jpg)\" ");//仅抓取src资源的图片 
    std::string::const_iterator start = allHtmlData.begin();
    std::string::const_iterator end = allHtmlData.end();
    std::cout<<"图片解析"<<std::endl;
    while(std::regex_search(start,end,mat,pattern)){
        std::string msg(mat[1].first,mat[1].second);
        if (msg.substr(0,4)!="http") msg="https:"+msg;//补充完整图片地址 
        g_photoAddr.push_back(msg);
        std::cout<<msg<<std::endl;
        start = mat[0].second;
    }
	std::cout<<"图片数量："<<g_photoAddr.size()<<'\n';
 
}

//3.1.3 获取html代码
void huoquHtmlData(){
    int n;
    char buff[10240];
    std::string allHtmlData;
    while(1){
    	memset(buff, 0, sizeof(buff));
        n = recv(g_socket,buff,sizeof(buff)-1,0);
        if(n<=0)
            break;
//        buff[n] = n;
        allHtmlData += buff;
    }
    //3.2从网页源代码中解析出 图片地址 和网站地址
//    getHtml(allHtmlData);
    getImage(allHtmlData);
}
//去重
void deletecp(){ 
    sort(g_photoAddr.begin(),g_photoAddr.end());//unique只能比较相邻元素是否重复
    g_photoAddr.erase(unique(g_photoAddr.begin(), g_photoAddr.end()), g_photoAddr.end());  
    sort(g_htmlAddr.begin(),g_htmlAddr.end());
    g_htmlAddr.erase(unique(g_htmlAddr.begin(), g_htmlAddr.end()), g_htmlAddr.end());//unique将重复的元素移到末尾，返回末尾中第一个重复值的地址
}
//转义函数
void downloadImage(){
	std::cout<<"开始下载图片...\n";
    std::string str = "0";
    for(int i=0;i<g_photoAddr.size();i++){
        std::string pURL = g_photoAddr[i];
        std::string path = pathfile;
        path.append("/ .jpg");
        char *str;
        sprintf(str,"%d",count);
        int pos = path.find(' ');
        path.replace(pos,1,str);
        std::cout<<pURL<<'\n';
        count++;
        std::cout<<path<<std::endl;
        char szBuffer[1024*128] = {0};
        unsigned long iSize = 0;
        char szPreCommand[128] = {0};
        DeleteUrlCacheEntry(pURL.c_str());//清空缓存，否则服务器上的文件修改后，无法下载最新的文件
        if (URLDownloadToFile(NULL, pURL.c_str(), path.c_str(), 0, NULL)==S_OK)
        {      
            printf("下载成功 OK\n");
        }
        else
        {
            printf("下载失败，Error:%d\n", GetLastError());
        }
        //CoUninitialize();
    }
    std::cout << "all is ok" << std::endl;
 
}
//核心逻辑 
void snapJpg(const char* addr){
    //3.1从网页源代码中解析出 图片地址 和网站地址
    //3.1.1解析网址，得到主机名
    char buff[256] = {0};
    strcpy(buff,addr);
    jiexiAddr(buff);
    //3.1.2 连接主机
    lianjieAddr();
    //3.1.3 获取html代码
    huoquHtmlData();
    //3.3去除重复后保存到一个地方
    deletecp();
    //3.4下载图片
    downloadImage();
}
int main(){
    //1、用户输入起始网址并保存
    std::string str;
    std::cout<<"begin url:"<<std::endl;  
//    std::cin>>str;
	str="http://www.fznews.com.cn/"; 
    count = 0;
    std::string file;
    std::cout<<"files name:"<<std::endl; 
//    std::cin>>file;
	file="123";
    std::string path = "./";
    path.append(file);
    std::cout<<path<<std::endl;
    pathfile = path;
    //2、创建文件夹用来保存图片
    //system("mkdir images");
    CreateDirectory(path.c_str(),NULL);//创建文件夹 
    //3、遍历搜索
    snapJpg(str.c_str());
    //3.5连接下一个网站
    for(int i=0;i<g_htmlAddr.size();i++){
        std::cout<<"this num:"<<i<<std::endl;
        str = g_htmlAddr.at(i);
        snapJpg(str.c_str());
    }
    return 0;
}
