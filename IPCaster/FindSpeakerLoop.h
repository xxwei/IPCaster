#pragma once


#define BOARDPORT	12215
#define REPLAYPORT	12214

#define MQPORT		17787
#include "MessageMQ.h"






class FindSpeakerLoop
{
public:
	FindSpeakerLoop();
	~FindSpeakerLoop();
private:   
	SOCKET BroadCastSocket;
	bool   bFindSpeaker = false;
	long   m_lastFindTick = 0;
	void	*m_pContext = NULL;
	void	*m_pSubSocket = NULL;

	bool	m_bFindRun = false;
	bool	m_bMQRun = false;

	MessageMQ	*m_pMMQ = NULL;
	MessageCallBack		*m_pMCB = NULL;
	int		m_model = 0; //0 查找模式，1 订阅模式
	string		m_listenername="listener";
	string		m_speakerip = "";
	string		m_speakeraddr = "";
private:
	bool InitFindSpeaker();
	void FindThread();
	void MQThread();
	
	//   listerner 有两种模式，0查找speaker ,1 订阅speaker

	bool ChangeModel(int model);
	int GetModel();
	
public:
	bool ConnectSpeaker(string ip, short port = MQPORT);
	bool DisConnectSpeaker();
	void SetMessageCallback(MessageCallBack *pmcb);
	void AsyncFindSpeaker(char *self);
	bool AsyncFindDestSpeaker(string ip, char *self=NULL);
	void SetListenerNickName(string name);
	bool isFindSpeaker();

	bool Start();
	bool Stop();
	bool StartMQ();
	bool StopMQ();
};

class SpeakerEcho
{
public:
	SpeakerEcho();
	~SpeakerEcho();
private:
	SOCKET RecvBroadSocket;
	void	*m_pContext = NULL;
	void	*m_pPubSocket = NULL;
	usermap			m_usermap;
	bool	m_bRun = false;
	MessageCallBack		*m_pMCB = NULL;
	string	m_speakername = "speaker";
	string  m_matchinfo = " ";
private:
	void UpdateUser(string key,ListenUser lu);
	bool InitSpeakEcho();
	void EchoThread();
	void MatchInfoThread();
public:
	void SetSpeakerNickName(string name);
	void SetMatchInfo(string info);
	void SetMessageCallback(MessageCallBack *pmcb);
	bool Start();
	bool Stop();

	bool SendMsg(string msg);
};