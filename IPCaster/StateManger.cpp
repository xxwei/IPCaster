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
	string limitcode = regc.substr(0, 32);
	string regicode = regc.substr(32, regc.length());
	bool ret =  m_pLicense->WriteRegCode(regicode);
	if (ret)
	{
		return m_pLicense->WriteTimeLimitCode(limitcode);
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
				return true;
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
	int m = 0;


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