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
	bool isOutDate(string limitcode="");
	bool WriteRegCode(string regcode);
	string ReadRegCode();

	bool WriteTimeLimitCode(string limitcode,bool bupdate=false);
	bool WriteTimeLimit(unsigned int time,unsigned int times,unsigned int year,unsigned int month,unsigned int day);
	string ReadTimeLimit();

    bool WriteSession(string flag, int times);
    string GetCurrentSessionFlag();
    int GetCurrentSessionTimes();

	unsigned int GetTimes();
	unsigned int GetTime();



	void UpdateTimesLimit();
	void UpdateTimeLimit();

private:
	unsigned int m_ntime = 0;
	unsigned int m_ntimes = 0;
	unsigned int m_nyear = 0;
	unsigned int m_nmonth = 0;
	unsigned int m_nday = 0;
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
	string m_currentlimitcode = "";
};

