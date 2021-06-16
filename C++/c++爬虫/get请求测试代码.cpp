#include <iostream>
#include <string>
#include <winsock2.h>
#include <windows.h>
#include <vector>
#include <locale>
#include <stdlib.h>
#include <Urlmon.h>
#include <Wininet.h>
#include <cstring>
#include <fstream>
using namespace std;
BOOL GetIpByDomainName(char *szHost, char* szIp){
    WSADATA        wsaData;
    HOSTENT   *pHostEnt;
    int             nAdapter = 0;
    struct       sockaddr_in   sAddr;
    if (WSAStartup(0x0101, &wsaData)){
        printf(" gethostbyname error for host:\n");
        return FALSE;
    }

    pHostEnt = gethostbyname(szHost);
    if (pHostEnt){
        if (pHostEnt->h_addr_list[nAdapter])
        {
            memcpy(&sAddr.sin_addr.s_addr, pHostEnt->h_addr_list[nAdapter], pHostEnt->h_length);
            sprintf(szIp, "%s", inet_ntoa(sAddr.sin_addr));
        }
    }
    else
    {
        //      DWORD  dwError = GetLastError();
        //      CString  csError;
        //      csError.Format("%d", dwError);
    }
    WSACleanup();
    return TRUE;
}

void sendGetRequest(){
     //开始进行socket初始化;
    WSADATA wData;  
    ::WSAStartup(MAKEWORD(2,2),&wData);  
    SOCKET clientSocket = socket(AF_INET,SOCK_STREAM,0);      
    struct sockaddr_in ServerAddr = {0};  
    int Ret=0;  
    int AddrLen=0;  
    HANDLE hThread=0; 
    string bufSend = "GET / HTTP/1.1\r\n"
    	"Host:www.fznews.com.cn\r\n"
    	"Content-Type=text/html\r\n"
        "Connection:Close\r\n\r\n";
    cout<<"请求头："<<bufSend<<'\n';
    char addIp[256] = {0};
    GetIpByDomainName("www.fznews.com.cn" , addIp);//ip地址用来检测网址是否正确 
    ServerAddr.sin_addr.s_addr = inet_addr(addIp);  
    ServerAddr.sin_port = htons(80);;  
    ServerAddr.sin_family = AF_INET;
    char bufRecv[1024];  
    int errNo = 0;  
    errNo = connect(clientSocket,(sockaddr*)&ServerAddr,sizeof(ServerAddr));  
    if(errNo==0){  
        //如果发送成功，则返回发送成功的字节数;
        int se=send(clientSocket,bufSend.c_str(),bufSend.size(),0); 
        if(se>0){  
            cout<<"发送成功,响应码为："<<se<<"\n";  
        }  
        string allHtmlData;
    	while(1){
    		memset(bufRecv, 0, sizeof(bufRecv));
			int n = recv(clientSocket,bufRecv,sizeof(bufRecv)-1,0);
        	if(n<=0)
            	break;
        	allHtmlData += bufRecv;
    	}
    	std::cout<<"完整数据："<<'\n';
    	//无法直接在控制台完整输出allHtmlData内容，可能是缓存限制。 
    	ofstream fout("./123.txt");
    	fout<<allHtmlData;
    	std::cout<<"#######"<<'\n';
    } 
    else errNo=WSAGetLastError();  
    //socket环境清理;
    ::WSACleanup();  
}
int main(){
	sendGetRequest();
	return 0;
}
