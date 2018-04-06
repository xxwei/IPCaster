﻿#pragma once
#include "MessageMQ.h"
#include "FindSpeakerLoop.h"
#include "License.h"
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
	bool ChangeToSpeakerSample();
	bool ChangeToListener();
	bool CanChangeSpeaker();
	void ChangeToExit();
	int GetCurrentState();

	void ReadCasterSettting();
	void SaveCasterSetting();

	void ReadMatchInfo(string path);
	void SaveMatchInfo(string path);

	void ReadFlow();

	void SetListenerCB(MessageCallBack *pMCB);
	void SetSpeakerCB(MessageCallBack *pMCB);
//公用
	void AddChatlog(wstring time, wstring text);
	wstring GetLocalIP();
//speaker
	
	wstring GetMatchInfo();
	void SetMatchInfo(wstring info);

	int GetTeam1();
	int GetTeam2();

	void SetTeam1(int id);
	void SetTeam2(int id);


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

	void	ExitState();


	bool	IsRegOK();

	bool  IsOutDate();
	wstring GetMCode();
	bool	SetRegCode(wstring regcode);
	wstring GetRegCode();

    void    UsingTimeThread(); // 计算场次线程
    string getunitstring(string flag);
public:
	Value			SettingValue;
	Value			MatchValue;
	Value			FlowValue;
	Value			CurrentFlowValue;

private:
	int				m_nState = 0;  //0 listener 1 speaker 2 samplespeaker
	FindSpeakerLoop *m_findSpeaker = NULL;
	SpeakerEcho		*m_speakEcho = NULL;
	CDuiString		m_nfontid = L"5";
	CDuiString		m_ofontid = L"1";
	map<wstring, wstring> m_chatlog_map;
	mutex			mapMutex;
	std::ofstream	ChatFileStream;
	bool			m_bStartSaveChat = false;
    bool         m_bStartUsingTime = false;//计算场次线程开关
	wstring			m_matchinfo = L" ";

	Reader			m_JsonReader;
	StyledWriter		m_JsonWriter;

	void			ChatToFileThread();

	License			*m_pLicense = NULL;

	bool			 m_bFirstToSpeaker = true;
};



