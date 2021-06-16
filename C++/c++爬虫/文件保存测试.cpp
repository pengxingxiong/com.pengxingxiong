#include <iostream>
#include <Windows.h>
#include <Urlmon.h>
//#pragma comment(lib, "Urlmon.lib")
using namespace std;
int main()
{   
    string dwnld_URL = "https://www.baidu.com/img/PCtm_d9c8750bed0b3c7d089fa7d55720d6cf.png";
    string savepath = "C:/Users/pengxx/Desktop/123.png";
    URLDownloadToFile(NULL, dwnld_URL.c_str(), savepath.c_str(), 0, NULL);

    return 0;
}
