
#pragma once  


#include "stdafx.h"
#include "License.h"



License::License()
{
	m_paesL = new CAES((unsigned char *)"柴晓伟0211691561");
	m_paesS = new CAES((unsigned char *)"柴晓伟0211691562");
	m_paesD = new CAES((unsigned char *)"柴晓伟0211691563");
	char ps[128] = { 1 };
	GetCpuByCmd(ps);
	string mcode = GetMCode();
	m_localshouldregcode = GetRegCode(mcode);
}


License::~License()
{

}
string License::GetMCode()
{
	char pl[128] = { 0 };
	GetCpuByCmd(pl);
	m_ndestL = m_paesL->Encrypt(reinterpret_cast<void*>(pl), strlen(pl), m_pdestL, 0);
	string mcode;
	for (UINT i = 0; i < m_ndestL&&i<16; i++)
	{
		char str[128] = { 0 };
		sprintf(str,"%02X", reinterpret_cast<BYTE*>(m_pdestL)[i]);
		mcode.append(str);
	}
	return mcode;
}
string License::GetRegCode(string mcode)
{
	m_ndestS = m_paesL->Encrypt((void *)mcode.c_str(), mcode.length(), m_pdestS, 0);
	string regcode;
	for (UINT i = 0; i < m_ndestS; i++)
	{
		char str[128] = { 0 };
		sprintf(str, "%02X", reinterpret_cast<BYTE*>(m_pdestS)[i]);
		regcode.append(str);
	}
	m_ndestS = m_paesL->Encrypt((void *)regcode.c_str(), regcode.length(), m_pdestS, 0);
	string reg2code;
	for (UINT i = 0; i < m_ndestS&&i<16; i++)
	{
		char str[128] = { 0 };
		sprintf(str, "%02X", reinterpret_cast<BYTE*>(m_pdestS)[i] + reinterpret_cast<BYTE*>(m_pdestS)[m_ndestS - i - 1]);
		reg2code.append(str);
	}
	return reg2code;
}
string License::GetLimitCode(string datecode)
{
	m_ndestD = m_paesD->Encrypt((void *)datecode.c_str(), datecode.length(), m_pdestD, 0);
	string limitcode;
	for (UINT i = 0; i < m_ndestD; i++)
	{
		char str[128] = { 0 };
		sprintf(str, "%02X", reinterpret_cast<BYTE*>(m_pdestD)[i]);
		limitcode.append(str);
	}
	return limitcode;
}
bool License::isOk()
{
	string regcode = ReadRegCode();
	if (m_localshouldregcode.compare(regcode) == 0)
	{
		return true;
	}
	return false;
}
bool License::WriteRegCode(string regcode)
{

	BOOL isWOW64;
	REGSAM p;
	IsWow64Process(GetCurrentProcess(), &isWOW64);
	if (isWOW64) {
		p = KEY_WRITE | KEY_WOW64_64KEY;
	}
	else {
		p = KEY_WRITE;
	}
	if (m_localshouldregcode.compare(regcode) == 0)
	{
		HKEY hcuKey;
		if (RegCreateKeyEx(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\IPCaster"), 0, NULL, 0, p, NULL, &hcuKey, NULL) != ERROR_SUCCESS) {
			//失败  
			return false;
		}
		if (RegSetValueEx(hcuKey, TEXT("regcode"), 0, REG_SZ, (BYTE*)regcode.c_str(), regcode.length()) != ERROR_SUCCESS) {
			//失败  
			return false;
		}
		RegCloseKey(hcuKey);
		return true;
	}
	return false;
}
string License::ReadRegCode()
{
	BOOL isWOW64;
	REGSAM p;
	IsWow64Process(GetCurrentProcess(), &isWOW64);
	if (isWOW64) {
		p = KEY_READ | KEY_WOW64_64KEY;
	}
	else {
		p = KEY_READ;
	}
	HKEY hcuKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\IPCaster"), 0, p, &hcuKey) != ERROR_SUCCESS) {
		//失败  
		return "";
	}
	CHAR  dwValue[128] = {0};//长整型数据，如果是字符串数据用char数组  
	DWORD dwSize = 128;
	DWORD dwType = REG_SZ;

	if (::RegQueryValueEx(hcuKey, _T("regcode"), 0, &dwType, (LPBYTE)&dwValue, &dwSize) != ERROR_SUCCESS)
	{
		RegCloseKey(hcuKey);
		return "";
	}
	RegCloseKey(hcuKey);
	return  string(dwValue);
	
}
bool License::WriteTimeLimit(string timelimit)
{
	BOOL isWOW64;
	REGSAM p;
	IsWow64Process(GetCurrentProcess(), &isWOW64);
	if (isWOW64) {
		p = KEY_WRITE | KEY_WOW64_64KEY;
	}
	else {
		p = KEY_WRITE;
	}
	//if (m_localshouldregcode.compare(regcode) == 0)
	{
		HKEY hcuKey;
		if (RegCreateKeyEx(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\IPCaster"), 0, NULL, 0, p, NULL, &hcuKey, NULL) != ERROR_SUCCESS) {
			//失败  
			return false;
		}
		if (RegSetValueEx(hcuKey, TEXT("timelimit"), 0, REG_SZ, (BYTE*)timelimit.c_str(), timelimit.length()) != ERROR_SUCCESS) {
			//失败  
			return false;
		}
		RegCloseKey(hcuKey);
		return true;
	}
	return false;
}
string License::ReadTimeLimit()
{
	BOOL isWOW64;
	REGSAM p;
	IsWow64Process(GetCurrentProcess(), &isWOW64);
	if (isWOW64) {
		p = KEY_READ | KEY_WOW64_64KEY;
	}
	else {
		p = KEY_READ;
	}
	HKEY hcuKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\IPCaster"), 0, p, &hcuKey) != ERROR_SUCCESS) {
		//失败  
		return "";
	}
	CHAR  dwValue[128] = { 0 };//长整型数据，如果是字符串数据用char数组  
	DWORD dwSize = 128;
	DWORD dwType = REG_SZ;

	if (::RegQueryValueEx(hcuKey, _T("timelimit"), 0, &dwType, (LPBYTE)&dwValue, &dwSize) != ERROR_SUCCESS)
	{
		RegCloseKey(hcuKey);
		return "";
	}
	RegCloseKey(hcuKey);
	return  string(dwValue);
}
//--------------------------------------------------------------  
//                      CPU序列号  
//--------------------------------------------------------------  
BOOL License::GetCpuByCmd(char *lpszCpu, int len/*=128*/)
{
	const long MAX_COMMAND_SIZE = 10000; // 命令行输出缓冲大小     
	WCHAR szFetCmd[] = L"wmic cpu get processorid"; // 获取CPU序列号命令行    
	const string strEnSearch = "ProcessorId"; // CPU序列号的前导信息  

	BOOL   bret = FALSE;
	HANDLE hReadPipe = NULL; //读取管道  
	HANDLE hWritePipe = NULL; //写入管道      
	PROCESS_INFORMATION pi;   //进程信息      
	STARTUPINFO         si;   //控制命令行窗口信息  
	SECURITY_ATTRIBUTES sa;   //安全属性  

	char            szBuffer[MAX_COMMAND_SIZE + 1] = { 0 }; // 放置命令行结果的输出缓冲区  
	string          strBuffer;
	unsigned long   count = 0;
	long            ipos = 0;

	memset(&pi, 0, sizeof(pi));
	memset(&si, 0, sizeof(si));
	memset(&sa, 0, sizeof(sa));

	pi.hProcess = NULL;
	pi.hThread = NULL;
	si.cb = sizeof(STARTUPINFO);
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;

	//1.0 创建管道  
	bret = CreatePipe(&hReadPipe, &hWritePipe, &sa, 0);
	if (!bret)
	{
		goto END;
	}

	//2.0 设置命令行窗口的信息为指定的读写管道  
	GetStartupInfo(&si);
	si.hStdError = hWritePipe;
	si.hStdOutput = hWritePipe;
	si.wShowWindow = SW_HIDE; //隐藏命令行窗口  
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;

	//3.0 创建获取命令行的进程  
	bret = CreateProcess(NULL, szFetCmd, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);
	if (!bret)
	{
		goto END;
	}

	//4.0 读取返回的数据  
	WaitForSingleObject(pi.hProcess, 500/*INFINITE*/);
	bret = ReadFile(hReadPipe, szBuffer, MAX_COMMAND_SIZE, &count, 0);
	if (!bret)
	{
		goto END;
	}

	//5.0 查找CPU序列号  
	bret = FALSE;
	strBuffer = szBuffer;
	ipos = strBuffer.find(strEnSearch);

	if (ipos < 0) // 没有找到  
	{
		goto END;
	}
	else
	{
		strBuffer = strBuffer.substr(ipos + strEnSearch.length());
	}

	memset(szBuffer, 0x00, sizeof(szBuffer));
	strcpy_s(szBuffer, strBuffer.c_str());

	//去掉中间的空格 \r \n  
	int j = 0;
	for (int i = 0; i < strlen(szBuffer); i++)
	{
		if (szBuffer[i] != ' ' && szBuffer[i] != '\n' && szBuffer[i] != '\r')
		{
			lpszCpu[j] = szBuffer[i];
			j++;
		}
	}
	lpszCpu[j] = '\0';

	bret = TRUE;

END:
	//关闭所有的句柄  
	CloseHandle(hWritePipe);
	CloseHandle(hReadPipe);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	return(bret);
}