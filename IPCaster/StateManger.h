#pragma once
#include "MessageMQ.h"
#include "FindSpeakerLoop.h"
//#include <bson.h>
//#include <bcon.h>



struct EventMsg
{
	wstring EventName;
	list<wstring>  MsgList;
};

typedef map<wstring, EventMsg> EventMap;


class StateManger
{
public:
	StateManger();
	~StateManger();
public:
	void InitCasterState();
	bool ChangeToSpeaker();
	bool ChangeToListener();
	bool CanChangeSpeaker();
	void ChangeToExit();
	int GetCurrentState();

	void ReadCasterSettting();
	void SaveCasterSetting();

	void ReadMatchInfo(string path);
	void SaveMatchInfo(string path);

	void SetListenerCB(MessageCallBack *pMCB);
	void SetSpeakerCB(MessageCallBack *pMCB);
//¹«ÓÃ
	void AddChatlog(wstring time, wstring text);
	wstring GetLocalIP();
//speaker
	
	wstring GetMatchInfo();
	void SetMatchInfo(wstring info);
	wstring GetSpeakerNickName();
	void SetSpeakerNickName(wstring name);

	void SetEventMap(EventMap em);
	void GetEventMap(EventMap &em);




//listener
	wstring GetListenerNickName();
	void SetListenerNickName(wstring name);
	bool	ConnectSpeaker(wstring ip);


	void SetNFont(CDuiString fontid);
	void SetOFont(CDuiString fontid);

	CDuiString GetNFont();
	CDuiString GetOFont();

	bool SendMsg(Message msg);
public:
	Value			SettingValue;
	Value			MatchValue;
private:
	int				m_nState = 0;
	FindSpeakerLoop *m_findSpeaker = NULL;
	SpeakerEcho		*m_speakEcho = NULL;
	CDuiString		m_nfontid = L"5";
	CDuiString		m_ofontid = L"1";
	map<wstring, wstring> m_chatlog_map;


	wstring			m_matchinfo = L" ";

	Reader			m_JsonReader;
	StyledWriter		m_JsonWriter;

};



