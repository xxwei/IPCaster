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

	string GetLimitCode(string datecode);
	bool isOk();
	bool WriteRegCode(string regcode);
	string ReadRegCode();

	bool WriteTimeLimit(string timelimit);
	string ReadTimeLimit();
private:
	BOOL GetCpuByCmd(char *lpszCpu, int len = 128);
	CAES	*m_paesL;
	CAES	*m_paesS;
	CAES	*m_paesD;
	void	*m_pdestL = NULL;
	int		m_ndestL = 0;
	void	*m_pdestS = NULL;
	int		m_ndestS = 0;
	void	*m_pdestD = NULL;
	int		m_ndestD = 0;
	string	m_localshouldregcode="";
};

