#include "stdafx.h"
#include "StateManger.h"


#define CONFIGFILE "conf.dat"
#define FLOWFILE "flow.dat"

StateManger::StateManger()
{
	m_bFirstToSpeaker = true;
}


StateManger::~StateManger()
{
}
void StateManger::InitCasterState()
{
	
	//读取配置
	ReadCasterSettting();
	ReadFlow();
	//listener
	m_nState = 0;
	m_findSpeaker = new FindSpeakerLoop();
	m_findSpeaker->Start();


	//创建目录
	CreateDirectory(L"log", NULL);
	CreateDirectory(L"chat", NULL);
	SetFileAttributes(L"log", FILE_ATTRIBUTE_HIDDEN);

	m_pLicense = new License();

}
bool	StateManger::IsRegOK()
{
	return m_pLicense->isOk();
}
bool  StateManger::IsOutDate()
{
	return m_pLicense->isOutDate();
	
}



wstring StateManger::GetMCode()
{
	return String2WString(m_pLicense->GetMCode());
}
wstring StateManger::GetRegCode()
{
	return String2WString(m_pLicense->ReadTimeLimit()+ m_pLicense->ReadRegCode() );
}
bool	StateManger::SetRegCode(wstring regcode)
{
	string regc = WString2String(regcode);
	if (regc.length() <= 32)
	{
		LOGE("注册码长度不够");
		return false;
	}
	string limitcode = regc.substr(0, 32);
	string regicode = regc.substr(32, regc.length());
	bool ret =  m_pLicense->WriteRegCode(regicode);
	if (ret)
	{

		return m_pLicense->WriteTimeLimitCode(limitcode);
	}
	else
	{
		LOGE("注册码和机器不符");
	}
	return ret;
}
bool StateManger::ChangeToSpeaker()
{
	//speaker
	if (CanChangeSpeaker()&& m_nState==0)
	{
		if (m_findSpeaker)
		{
			m_findSpeaker->Stop();
		}
		if (m_speakEcho == NULL)
		{
			m_speakEcho = new SpeakerEcho();
			if (m_speakEcho->Start())
			{
				m_nState = 1;
				if (m_bFirstToSpeaker)
				{
					m_bFirstToSpeaker = false;
					m_pLicense->UpdateTimesLimit();
				}
                if (!m_bStartUsingTime)
                {
                    m_bStartUsingTime = true;
                    std::thread UsingTimeThread = thread(std::bind(&StateManger::UsingTimeThread, this));
                    UsingTimeThread.detach();
                }
				return true;
			}
			
		}
	}
	if (m_nState == 2)
	{
		m_nState = 1;
		return true;
	}
	return false;
}
bool StateManger::ChangeToSpeakerSample()
{
	if (CanChangeSpeaker() && m_nState == 0)
	{
		if (m_findSpeaker)
		{
			m_findSpeaker->Stop();
		}
		if (m_speakEcho == NULL)
		{
			m_speakEcho = new SpeakerEcho();
			if (m_speakEcho->Start())
			{
				if (m_bFirstToSpeaker)
				{
					m_bFirstToSpeaker = false;
					m_pLicense->UpdateTimesLimit();
				}
				m_nState = 2;
                if (!m_bStartUsingTime)
                {
                    m_bStartUsingTime = true;
                    std::thread UsingTimeThread = thread(std::bind(&StateManger::UsingTimeThread, this));
                    UsingTimeThread.detach();
                }
				return true;
			}
            else
            {
                ChangeToListener();
                return false;
            }


            
		}
	}
	if (m_nState == 1)
	{
		m_nState = 2;
		return true;
	}
	return false;
}
bool StateManger::ChangeToListener()
{
	if (m_speakEcho != NULL)
	{
		if (m_speakEcho->Stop())
		{
			m_nState = 0;
			delete m_speakEcho;
			m_speakEcho = NULL;
			if (m_findSpeaker)
			{
				m_findSpeaker->Start();
			}
			return true;
		}

	}
	return false;
}
bool StateManger::CanChangeSpeaker()
{

	if (m_findSpeaker&&m_pLicense->isOk()&&!m_pLicense->isOutDate())
	{
		bool ret =  m_findSpeaker->isFindSpeaker();
		return !ret;
	}
	return false;
}
void StateManger::ChangeToExit()
{
	if (m_nState == 0)
	{
		if (m_findSpeaker)
		{
			m_findSpeaker->Stop();
			m_findSpeaker->StopMQ();
		}
	}
	if (m_nState == 1)
	{
		if (m_speakEcho)
		{
			m_speakEcho->Stop();
		}
	}
	//exit
	m_nState = -1;
}
void StateManger::SetListenerCB(MessageCallBack *pMCB)
{
	if (m_findSpeaker)
	{
		m_findSpeaker->SetMessageCallback(pMCB);
	}

}
void StateManger::SetSpeakerCB(MessageCallBack *pMCB)
{
	if (m_speakEcho)
	{
		m_speakEcho->SetMessageCallback(pMCB);
	}
}
int StateManger::GetCurrentState()
{
	return m_nState;
}

void StateManger::ReadCasterSettting()
{
	ifstream ifs;
	ifs.open(CONFIGFILE);
	assert(ifs.is_open());
	if (!m_JsonReader.parse(ifs, SettingValue, false))
	{
		//错误
	}
	ifs.close();
	//主要配置项
	//listener
	//SettingValue["ListenerName"]
	//SettingValue["LastSpeakerIP"]
	//SettingValue["LNfont"]
	//SettingValue["LOfont"]

	//speaker
	//SettingValue["SpeakerName"]
	//SettingValue["SNfont"]
	//SettingValue["SOfont"]
	//SettingValue["EventMap"]
    m_keepalivemsg  =  String2WString(U2G(SettingValue["KeepAliveMsg"].asCString()));


}

void StateManger::ReadFlow()
{
	ifstream ifs;
	ifs.open(FLOWFILE);
	assert(ifs.is_open());
	if (!m_JsonReader.parse(ifs, FlowValue, false))
	{
		//错误
	}
	ifs.close();
}

static wstring unicode2string(const char * str) {
	wstring rst;
	bool escape = false;
	int len = strlen(str);
	int intHex;
	char tmp[5];
	memset(tmp, 0, 5);
	for (int i = 0; i < len; i++)
	{
		char c = str[i];
		switch (c)
		{
		case '//':
		case '%':
			escape = true;
			break;
		case 'u':
		case 'U':
			if (escape)
			{
				memcpy(tmp, str + i + 1, 4);
				sscanf(tmp, "%x", &intHex); //把16进制字符转换为数字  
				rst.push_back(intHex);
				i += 4;
				escape = false;
			}
			else {
				rst.push_back(c);
			}
			break;
		default:
			rst.push_back(c);
			break;
		}
	}
	return rst;
}

void StateManger::SaveCasterSetting()
{
	string setting = m_JsonWriter.write(SettingValue);
	ofstream out(CONFIGFILE);
	if (out.is_open())
	{
		out << setting << endl;
		out.close();
	}
}
void StateManger::ReadMatchInfo(string path)
{
	ifstream ifs;
	ifs.open(path);
	assert(ifs.is_open());
	if (!m_JsonReader.parse(ifs, MatchValue, false))
	{
		//错误
	}
	ifs.close();
}
void StateManger::SaveMatchInfo(string path)
{
	string setting = m_JsonWriter.write(MatchValue);
	ofstream out(path);
	if (out.is_open())
	{
		out << setting << endl;
		out.close();
	}
}
void StateManger::AddChatlog(wstring time, wstring text)
{
	mapMutex.lock();
	m_chatlog_map.insert(pair<wstring, wstring>(time, text));
	mapMutex.unlock();
	if (m_nState >0) //speaker
	{
		Message msg;
		msg.SetType(PUBMSG);
		msg.AppendMsg(L"time", time);
		msg.AppendMsg(L"text", text);
		m_speakEcho->SendMsg(msg.ToString());
	}
	if (!m_bStartSaveChat)
	{
		m_bStartSaveChat = true;
		std::thread ChatFileThread = thread(std::bind(&StateManger::ChatToFileThread,this));
		ChatFileThread.detach();
	}
}

wstring StateManger::GetLocalIP()
{
	IPInfo ips[64];
	int count = 0;
	GetLocalIPs(ips, 64, &count);
	if(count)
		return String2WString(string(ips[0].ip));
	return L"0.0.0.0";
}
wstring StateManger::GetMatchInfo()
{
	return m_matchinfo;
}

string StateManger::getunitstring(string flag)
{
    SYSTEMTIME st;
    GetLocalTime(&st);
    char timestr[1024] = { 0 };
    sprintf(timestr, "%04d-%02d-%02d:",st.wYear, st.wMonth, st.wDay);
    string unit = string(timestr) + flag;
    unit = m_pLicense->GetRegCode(unit);
    return unit;
}

void    StateManger::UsingTimeThread()
{
    string lastunitstring = m_pLicense->GetCurrentSessionFlag();
    int currentflagtimes = m_pLicense->GetCurrentSessionTimes();

    string currentnuitstring = "";
    int loopnum = 0;
    while (1)
    {
        Sleep(1000);
        loopnum++;
        if (loopnum > 60)
        {
            loopnum = 0;
            currentnuitstring = getunitstring(WString2String(m_matchinfo));
            lastunitstring = m_pLicense->GetCurrentSessionFlag();
            currentflagtimes = m_pLicense->GetCurrentSessionTimes();
            if (currentnuitstring != lastunitstring)
            {
                currentflagtimes = 0;
                m_pLicense->WriteSession(currentnuitstring, currentflagtimes);
            }
            else
            {
                currentflagtimes +=1;
                m_pLicense->WriteSession(currentnuitstring, currentflagtimes);
                if (currentflagtimes > 90)//大于90分钟算一个场次
                {
                    m_pLicense->UpdateTimeLimit();
                }
            }
        }
    }
}
void StateManger::SetMatchInfo(wstring info)
{
	m_matchinfo = info;
	if (m_speakEcho)
	{
		m_speakEcho->SetMatchInfo(WString2String(info));
	}
}
int StateManger::GetTeam1()
{
	return SettingValue["Team1ID"].asInt();
}
int StateManger::GetTeam2()
{
	return SettingValue["Team2ID"].asInt();
}

void StateManger::SetTeam1(int id)
{
	SettingValue["Team1ID"] = id;
}
void StateManger::SetTeam2(int id)
{
	SettingValue["Team2ID"] = id;
}
wstring StateManger::GetSpeakerNickName()
{
	return String2WString(U2G(SettingValue["SpeakerName"].asCString()));
}
void StateManger::SetSpeakerNickName(wstring name)
{
	string newname = WString2String(name);
	SettingValue["SpeakerName"] = G2U(newname.c_str());
}
wstring StateManger::GetListenerNickName()
{
	return String2WString(U2G(SettingValue["ListenerName"].asCString()));
}
void StateManger::SetListenerNickName(wstring name)
{
	string newname = WString2String(name);
	SettingValue["ListenerName"] = G2U(newname.c_str());
	if (m_findSpeaker)
	{
		m_findSpeaker->SetListenerNickName(newname);
	}
}
void StateManger::SetResetMsgTimeHour(int hour)
{
    if (m_nState != 0)
    {
        SettingValue["ResetHour"] = hour;
       
    }
}
int StateManger::GetResetMsgTimeHour()
{
    if (m_nState != 0)
    {
        if (!SettingValue["ResetHour"].isNull())
        {
            return SettingValue["ResetHour"].asInt();
        }
    }
    return 18;
}

void StateManger::SetResetMsgTimeMin(int min)
{
    if (m_nState != 0)
    {
        SettingValue["ResetMin"] = min;
    }
}
int StateManger::GetResetMsgTimeMin()
{
    if (m_nState != 0)
    {
        if (!SettingValue["ResetMin"].isNull())
        {
            return SettingValue["ResetMin"].asInt();
        }
    }
    return 2;
}
void StateManger::SetNFont(CDuiString fontid)
{
	if (m_nState == 0)
	{
		SettingValue["LNfont"] = WString2String(wstring(fontid));
	}
	else
	{
		SettingValue["SNfont"] = WString2String(wstring(fontid));
	}
	m_nfontid = fontid;
}
void StateManger::SetOFont(CDuiString fontid)
{
	if (m_nState == 0)
	{
		SettingValue["LOfont"] = WString2String(wstring(fontid));
	}
	else
	{
		SettingValue["SOfont"] = WString2String(wstring(fontid));
	}
	m_ofontid = fontid;
}
CDuiString StateManger::GetNFont()
{
	if (m_nState == 0)
	{
		if (!SettingValue["LNfont"].isNull())
		{
			return String2WString(SettingValue["LNfont"].asString()).c_str();
		}
	}
	else
	{
		if (!SettingValue["SNfont"].isNull())
		{
			return String2WString(SettingValue["SNfont"].asString()).c_str();
		}
	}
	return m_nfontid;
}
CDuiString StateManger::GetOFont()
{
	if (m_nState == 0)
	{
		if (!SettingValue["LOfont"].isNull())
		{
			return String2WString(SettingValue["LOfont"].asString()).c_str();
		}
	}
	else
	{
		if (!SettingValue["SOfont"].isNull())
		{
			return String2WString(SettingValue["SOfont"].asString()).c_str();
		}
	}
	return m_ofontid;
}

void StateManger::SetEventMap(EventMap em)
{
	SettingValue.removeMember("EventMap");
	EventMap::iterator item = em.begin();
	list<wstring>::iterator itemc;
	Value EventMap;
	Value Event;
	for (int i=0; item != em.end(); item++,i++)
	{
		string eve = WString2String(item->second.EventName);
		itemc = item->second.MsgList.begin();
		Value Msg;
		for (int j=0 ; itemc != item->second.MsgList.end(); itemc++,j++)
		{
			Msg[j] = G2U(WString2String(*itemc).c_str());
		}
		Event["msgs"] = Msg;
		Event["name"] = G2U(eve.c_str());
		Event["key"] = WString2String(item->first);
		EventMap[i] = Event;
	}
	SettingValue["EventMap"]= EventMap;
}
void StateManger::GetEventMap(EventMap &emap)
{
	//程序启动是调用
	emap.clear();
	if (!SettingValue["EventMap"].isNull())
	{
		int ecount = SettingValue["EventMap"].size();
		for (int i = 0; i < ecount; i++)
		{
			EventMsg  em;
			em.EventName = String2WString(string(U2G(SettingValue["EventMap"][i]["name"].asCString())));
			wstring key = String2WString(string(U2G(SettingValue["EventMap"][i]["key"].asCString())));
			int mcount = SettingValue["EventMap"][i]["msgs"].size();
			for (int j = 0; j < mcount; j++)
			{
				string msg = string(U2G(SettingValue["EventMap"][i]["msgs"][j].asCString()));
				em.MsgList.push_back(String2WString(msg));
			}
			emap.insert(pair<wstring, EventMsg>(key, em));
		}
	}
}
void StateManger::SendCmdMsg(wstring cmd)
{
    Message msg;
    msg.SetType(CMDMSG);
    msg.AppendMsg(L"cmd", cmd);
    SendMsg(msg);
}
bool StateManger::SendMsg(Message msg)
{
	if (m_speakEcho)
	{
		return m_speakEcho->SendMsg(msg.ToString());
	}
	return false;
}
bool	StateManger::ConnectSpeaker(wstring ip)
{
	if (m_findSpeaker)
	{
		return m_findSpeaker->AsyncFindDestSpeaker(WString2String(ip));
	}
	return false;
}
void	StateManger::ExitState()
{
	m_nState = -1;
}
void StateManger::ChatToFileThread()
{
	if (!m_pLicense->isOk())
	{
		return;
	}
	int nOldState = m_nState;
	m_bStartSaveChat = true;
	while (m_nState >= 0)
	{
		
		if (m_chatlog_map.size())
		{
			if (nOldState != m_nState)
			{
				//重新创建文件
				if (ChatFileStream.is_open())
				{
					ChatFileStream.close();
				}
			}

			if (!ChatFileStream.is_open())
			{
				time_t rawtime;
				struct tm  timeinfo;
				time(&rawtime);
				localtime_s(&timeinfo, &rawtime);
				char filename[80];
				strftime(filename, 80, "%F-%H-%M-%S.txt",&timeinfo);
				string filestr = "chat\\"+WString2String(m_matchinfo) + string(filename);
				ChatFileStream.open(filestr.c_str());
			}
			if (mapMutex.try_lock())
			{
				map<wstring, wstring>::iterator item = m_chatlog_map.begin();
				for (; item != m_chatlog_map.end(); item++)
				{
					ChatFileStream << WString2String(item->first)<<" ";
					ChatFileStream << WString2String(item->second)<<endl;
				}
				m_chatlog_map.clear();
				mapMutex.unlock();
			}

			ChatFileStream.flush();
		}
		
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
	m_bStartSaveChat = false;
}