
// stdafx.cpp : 只包括标准包含文件的源文件
// RegM.pch 将作为预编译头
// stdafx.obj 将包含预编译类型信息

#include "stdafx.h"


std::string WString2String(const std::wstring& wstr)
{
	//int nLen = (int)wstr.length();
	//string str;
	//str.resize(nLen, ' ');
	//int nResult = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)wstr.c_str(), nLen, (LPSTR)str.c_str(), nLen, NULL, NULL);
	//return str;
	std::string strLocale = setlocale(LC_ALL, "");
	const wchar_t* wchSrc = wstr.c_str();
	size_t nDestSize = wcstombs(NULL, wchSrc, 0) + 1;
	char *chDest = new char[nDestSize];
	memset(chDest, 0, nDestSize);
	wcstombs(chDest, wchSrc, nDestSize);
	std::string strResult = chDest;
	delete[]chDest;
	setlocale(LC_ALL, strLocale.c_str());
	return strResult;
}
// string => wstring
std::wstring String2WString(const std::string& s)
{
	std::string strLocale = setlocale(LC_ALL, "");
	const char* chSrc = s.c_str();
	size_t nDestSize = mbstowcs(NULL, chSrc, 0) + 1;
	wchar_t* wchDest = new wchar_t[nDestSize];
	wmemset(wchDest, 0, nDestSize);
	mbstowcs(wchDest, chSrc, nDestSize);
	std::wstring wstrResult = wchDest;
	delete[]wchDest;
	setlocale(LC_ALL, strLocale.c_str());
	return wstrResult;
	//int num = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, NULL, 0);
	//wchar_t *wide = new wchar_t[num];
	//MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, wide, num);
	//std::wstring w_str(wide);
	//delete[] wide;
	//return w_str;
}