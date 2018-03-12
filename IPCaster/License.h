#pragma once
#include"AES.h"

class License
{
public:
	License();
	~License();
public:
	string GetMCode();
	string GetRegCode(string mcode);
	bool isOk();
	bool WriteRegCode(string regcode);
	string ReadRegCode();
private:
	BOOL GetCpuByCmd(char *lpszCpu, int len = 128);
	CAES	*m_paesL;
	CAES	*m_paesS;
	void	*m_pdestL = NULL;
	int		m_ndestL = 0;
	void	*m_pdestS = NULL;
	int		m_ndestS = 0;
	string	m_localshouldregcode="";
};

