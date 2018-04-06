// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // 从 Windows 头中排除极少使用的资料
// Windows 头文件: 
#include <windows.h>

// C 运行时头文件
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include "resource.h"		// 主符号

// TODO:  在此处引用程序需要的其他头文件
#include <WinSock2.h>  
#include <iphlpapi.h>  
#include <stdlib.h>  
#include <zmq.h>



#include <UIlib.h>

using namespace DuiLib;
#pragma comment(lib, "ws2_32.lib")  
#pragma comment(lib, "IPHLPAPI.lib")  
#include <thread>
#include <mutex>
#include <list>
#include <string>  
#include <iostream>  
#include <sstream>  
#include <map>
#include <thread>
#include <fstream>
#include <iterator>

#include "P2PLog.h"
using namespace std;

#define lengthof(x) (sizeof(x)/sizeof(*x))
#define MAX max
#define MIN min
#define CLAMP(x,a,b) (MIN(b,MAX(a,x)))

std::string WString2String(const std::wstring& ws);
std::wstring String2WString(const std::string& s);

char* U2G(const char* utf8);
char* G2U(const char* gb2312);


