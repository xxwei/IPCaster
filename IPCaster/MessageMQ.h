#pragma once
#include <json.h>
using namespace Json;

enum MSGTYPE {
	KEEPALIVE=0, //time|name
	MATCHMSG,//time|matchinfo
	PUBMSG  //time|text
};


class Message {

private:
	int nType;
	map<wstring, wstring>	msglist;
	Value					msg;
public:
	int		GetType();
	void	SetType(int type);
	bool	AppendMsg(wstring key, wstring value);
	wstring	GetMsgValue(wstring key);


	string  ToString();
	bool	PraseMsg(string msgstr);
};


struct ListenUser
{
	string ip;
	string name;
	long	lasttick;
};
typedef map<string, ListenUser>		usermap;

class MessageCallBack
{
public:
	virtual void OnSpeakerOnLine(wstring ip,wstring name) {};
	virtual void OnSpeakerOffLine() {};
	virtual void OnRecvMessage(Message *msg) {};
	virtual void OnListenerOnLine(usermap map) {};
	virtual void OnListenerOffLine() {};
};


class MessageMQ
{
public:
	MessageMQ();
	~MessageMQ();
private:
	list<Message *>		m_list;
public:

	Message *GetNewMsg();
	void	InMsgMQ(Message *msg);
};

//结构体记录ip信息
typedef struct tagIPInfo
{
	char ip[30];
}IPInfo;

//获取多个ip地址信息列表
bool GetLocalIPs(IPInfo* ips, int maxCnt, int* cnt);