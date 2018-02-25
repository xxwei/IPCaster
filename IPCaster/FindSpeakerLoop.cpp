#include "stdafx.h"
#include "FindSpeakerLoop.h"





//获取多个ip地址信息列表
bool GetLocalIPs(IPInfo* ips, int maxCnt, int* cnt)
{
	//1.初始化wsa  
	WSADATA wsaData;
	int ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (ret != 0)
	{
		return false;
	}
	//2.获取主机名  
	char hostname[256];
	ret = gethostname(hostname, sizeof(hostname));
	if (ret == SOCKET_ERROR)
	{
		return false;
	}
	//3.获取主机ip  
	HOSTENT* host = gethostbyname(hostname);
	if (host == NULL)
	{
		return false;
	}
	//4.逐个转化为char*并拷贝返回  
	*cnt = host->h_length<maxCnt ? host->h_length : maxCnt;
	for (int i = 0; i<*cnt; i++)
	{
		in_addr* addr = (in_addr*)*host->h_addr_list;
		strcpy(ips[i].ip, inet_ntoa(addr[i]));
	}
	return true;
}
/*-----------------------------------------
局域网IP地址范围
A类：10.0.0.0-10.255.255.255
B类：172.16.0.0-172.31.255.255
C类：192.168.0.0-192.168.255.255
-------------------------------------------*/
bool isLAN(string ipstring)
{
	istringstream st(ipstring);
	int ip[2];
	for (int i = 0; i < 2; i++)
	{
		string temp;
		getline(st, temp, '.');
		istringstream a(temp);
		a >> ip[i];
	}
	if ((ip[0] == 10) || (ip[0] == 172 && ip[1] >= 16 && ip[1] <= 31) || (ip[0] == 192 && ip[1] == 168))
		return true;
	else return false;
}




FindSpeakerLoop::FindSpeakerLoop()
{
	BroadCastSocket = 0;
	m_pContext = zmq_ctx_new();

}


FindSpeakerLoop::~FindSpeakerLoop()
{
	zmq_close(m_pSubSocket);
	zmq_ctx_destroy(m_pContext);
}
bool FindSpeakerLoop::InitFindSpeaker()
{
	BroadCastSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (INVALID_SOCKET == BroadCastSocket)
	{
		int err = WSAGetLastError();
		printf("\"socket\" error! error code is %d\n", err);
		return false;
	}
	BOOL optval = TRUE;
	sockaddr_in bindAddr;
	bindAddr.sin_family = AF_INET;
	bindAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	bindAddr.sin_port = htons(REPLAYPORT);
	setsockopt(BroadCastSocket, SOL_SOCKET, SO_BROADCAST, (char FAR *)&optval, sizeof(optval));
	

	if (::bind(BroadCastSocket, (sockaddr *)&bindAddr, sizeof(sockaddr_in))==0)
	{
		optval = TRUE;
		setsockopt(BroadCastSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&optval, sizeof(BOOL));
		optval = FALSE;
		setsockopt(BroadCastSocket, SOL_SOCKET, SO_DONTLINGER, (const char*)&optval, sizeof(BOOL));
		int nNetTimeout = 1000;//1秒
	   //发送时限
		setsockopt(BroadCastSocket,SOL_SOCKET, SO_SNDTIMEO, (char *)&nNetTimeout, sizeof(int));
		//接收时限
		setsockopt(BroadCastSocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&nNetTimeout, sizeof(int));
		return true;
	}
	//启动结束信号线程
	return false;
}
void FindSpeakerLoop::SetMessageCallback(MessageCallBack *pmcb)
{
	m_pMCB = pmcb;
}
void FindSpeakerLoop::AsyncFindSpeaker(char *self)
{
	IPInfo ips[64];
	int count = 0;
	GetLocalIPs(ips, 64, &count);
	list<string>		brlist;
	for (int i = 0; i < count; i++)
	{
		if (isLAN(string(ips[i].ip)))
		{
			string ip = string(ips[i].ip);
			int pos = ip.find_last_of(".");
			ip = ip.substr(0, pos);
			ip = ip + ".255";
			brlist.push_back(ip);
		}
	}
	list<SOCKADDR_IN>		sockaddrlist;

	list<string>::iterator item = brlist.begin();
	for (; item != brlist.end(); item++)
	{
		SOCKADDR_IN braddr;
		braddr.sin_family = AF_INET;
		braddr.sin_port = htons(BOARDPORT);
		braddr.sin_addr.s_addr = inet_addr((*item).c_str());
		sockaddrlist.push_back(braddr);
	}

	//设置该套接字为广播类型，   
	bool bOpt = true;
	setsockopt(BroadCastSocket, SOL_SOCKET, SO_BROADCAST, (char*)&bOpt, sizeof(bOpt));
	list<SOCKADDR_IN>::iterator addritem = sockaddrlist.begin();
	for (; addritem != sockaddrlist.end(); addritem++)
	{
		int nLen = sizeof((*addritem));
		if (SOCKET_ERROR == sendto(BroadCastSocket, self, strlen(self), 0, (sockaddr*)&(*addritem), nLen))
		{
			continue;
		}
	}

}

void FindSpeakerLoop::FindThread()
{
	sockaddr_in clientAddr;
	char buf[1024] = { 0 };
	int fromlength = sizeof(SOCKADDR);
	long error = 0;
	while (m_bFindRun)
	{
		AsyncFindSpeaker((char *)m_listenername.c_str());
		int nRet = recvfrom(BroadCastSocket, buf, 1024, 0, (struct sockaddr FAR *)&clientAddr, (int FAR *)&fromlength);
		if (SOCKET_ERROR != nRet)
		{
			char    *pIPAddr = inet_ntoa(clientAddr.sin_addr);
			if (NULL != pIPAddr)
			{
				printf("SpeakerAddr: %s\n", pIPAddr);
				printf("receive command: %s\n", buf);
				bFindSpeaker = true;
				m_lastFindTick = GetTickCount();
				if (m_pMCB)
				{
					m_pMCB->OnSpeakerOnLine(String2WString(string(pIPAddr)), String2WString(string(buf)));
				}

				ConnectSpeaker(string(pIPAddr), MQPORT);
				memset(buf, 0, 1024);
			}
		}
		else
		{
			if (GetTickCount() - m_lastFindTick > 3000)
			{
				//speaker下线
				if (ChangeModel(0))
				{
					if (m_pMCB)
					{
						m_pMCB->OnSpeakerOffLine();
						bFindSpeaker = false;
					}
				}
				
			}
		}
		error = GetLastError();
		Sleep(1000);
		
	}
}
void FindSpeakerLoop::MQThread()
{
	while (m_bMQRun)
	{
		zmq_msg_t request;
		zmq_msg_init(&request);
		int nSize = zmq_msg_recv(&request, m_pSubSocket, 0);
		if(nSize> 0)
		{
			//int nSie = zmq_msg_size(&request);
			//char *wmsg = new char[nSize];
			//memcpy(wmsg, zmq_msg_data(&request), nSize);

			Message *pmsg = new Message();
			if (pmsg->PraseMsg(string((char *)zmq_msg_data(&request))))
			{
				if (m_pMCB)
				{
					m_pMCB->OnRecvMessage(pmsg);
				}
				m_lastFindTick = GetTickCount();
			}
			//OutputDebugStringA(wmsg);
			//OutputDebugString(L"\n");
			zmq_msg_close(&request);
		}
		else
		{
			//m_bMQRun = false;
		}
	}
}
bool FindSpeakerLoop::ConnectSpeaker(string ip, short port)
{
	std::stringstream url;
	url << "tcp://";
	url << ip;
	url << ":";
	url << port;
	m_speakeraddr = url.str();
	if (!m_bMQRun)
	{
		OutputDebugStringA(m_speakeraddr.c_str());
		m_pSubSocket = zmq_socket(m_pContext, ZMQ_SUB);
		int iRcvTimeout = 2000;
		int ret;
		//ret = zmq_setsockopt(m_pSubSocket, ZMQ_RCVTIMEO, &iRcvTimeout, sizeof(iRcvTimeout));
		ret = zmq_connect(m_pSubSocket, m_speakeraddr.c_str());
		if (!ret)
		{
			ret = zmq_errno();
			ret = zmq_setsockopt(m_pSubSocket, ZMQ_SUBSCRIBE, "", 0);
			if (StartMQ())
			{
				ChangeModel(1);
			}
			return true;
		}
	}
	return false;

}
bool FindSpeakerLoop::ChangeModel(int model)
{
	if (m_model != model)
	{
		if (m_model==0)  //转入订阅模式
		{
			//没啥要做的
		}
		if (m_model == 1) //由订阅模式转入查找模式
		{
			zmq_disconnect(m_pSubSocket, m_speakeraddr.c_str());
		}
		m_model = model;
		return true;
	}
	return false;

	
}
int FindSpeakerLoop::GetModel()
{
	return m_model;
}
bool FindSpeakerLoop::Start()
{
	if (!m_bFindRun)
	{
		m_bFindRun = true;
		InitFindSpeaker();
		std::thread findthread(std::bind(&FindSpeakerLoop::FindThread, this));
		findthread.detach();
		return true;
	}
	return false;
}
bool FindSpeakerLoop::Stop()
{
	m_bFindRun = false;
	closesocket(BroadCastSocket);
	return false;
}
bool FindSpeakerLoop::StartMQ()
{
	if (!m_bMQRun)
	{
		m_bMQRun = true;
		std::thread findthread(std::bind(&FindSpeakerLoop::MQThread, this));
		findthread.detach();
		return true;
	}
	return false;
}
bool FindSpeakerLoop::StopMQ()
{
	m_bMQRun = false;
	zmq_close(m_pSubSocket);
	return false;
}
bool FindSpeakerLoop::isFindSpeaker()
{
	return bFindSpeaker;
}
void FindSpeakerLoop::SetListenerNickName(string name)
{
	m_listenername = name;
}

SpeakerEcho::SpeakerEcho()
{
	m_pContext = zmq_ctx_new();
	m_pPubSocket = zmq_socket(m_pContext, ZMQ_PUB);
	
}
SpeakerEcho::~SpeakerEcho()
{
	zmq_close(m_pPubSocket);
	zmq_ctx_destroy(m_pContext);
}
void SpeakerEcho::SetMessageCallback(MessageCallBack *pmcb)
{
	m_pMCB = pmcb;
}
void SpeakerEcho::SetSpeakerNickName(string name)
{
	m_speakername = name;
}
void SpeakerEcho::SetMatchInfo(string info)
{
	m_matchinfo = info;
}
void SpeakerEcho::UpdateUser(string key, ListenUser lu)
{
	int nSize = m_usermap.size();
	if (key != "")
	{
		usermap::iterator item = m_usermap.find(key);
		
		if (item != m_usermap.end())
		{
			item->second.lasttick = lu.lasttick;
		}
		else
		{
			m_usermap.insert(pair<string, ListenUser>(key, lu));
		}
	}
	if (m_usermap.size())
	{
		long currenttick = GetTickCount();
		usermap::iterator item = m_usermap.begin();
		for (; item != m_usermap.end(); )
		{
			if (currenttick - item->second.lasttick > 3000)
			{
				item = m_usermap.erase(item);
				if (item == m_usermap.end())
				{
					break;
				}
			}
			else
			{
				item++;
			}
		}
	}
	if (nSize != m_usermap.size())
	{
		if (m_pMCB)
		{
			m_pMCB->OnListenerOnLine(m_usermap);
		}
	}
}
bool SpeakerEcho::InitSpeakEcho()
{
	RecvBroadSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (INVALID_SOCKET == RecvBroadSocket)
	{
		int err = WSAGetLastError();
		printf("\"socket\" error! error code is %d\n", err);
		return false;
	}
	BOOL optval = TRUE;
	sockaddr_in bindAddr;
	bindAddr.sin_family = AF_INET;
	bindAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	bindAddr.sin_port = htons(BOARDPORT);
	setsockopt(RecvBroadSocket, SOL_SOCKET, SO_BROADCAST, (char FAR *)&optval, sizeof(optval));
	if (::bind(RecvBroadSocket, (sockaddr *)&bindAddr, sizeof(sockaddr_in)) == 0)
	{
		optval = TRUE;
		setsockopt(RecvBroadSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&optval, sizeof(BOOL));
		optval = FALSE;
		setsockopt(RecvBroadSocket, SOL_SOCKET, SO_DONTLINGER, (const char*)&optval, sizeof(BOOL));
		int nNetTimeout = 1000;//1秒
		//发送时限
		setsockopt(RecvBroadSocket, SOL_SOCKET, SO_SNDTIMEO, (char *)&nNetTimeout, sizeof(int));
		//接收时限
		setsockopt(RecvBroadSocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&nNetTimeout, sizeof(int));
		return true;
	}
	//启动结束信号线程
	return false;
}
void SpeakerEcho::EchoThread()
{
	while (m_bRun)
	{
		sockaddr_in clientAddr;
		char buf[1024] = { 0 };
		int fromlength = sizeof(SOCKADDR);
		ListenUser lu;
		while (m_bRun)
		{
			int nRet = recvfrom(RecvBroadSocket, buf, 1024, 0, (struct sockaddr FAR *)&clientAddr, (int FAR *)&fromlength);
			if (SOCKET_ERROR != nRet)
			{
				char    *pIPAddr = inet_ntoa(clientAddr.sin_addr);
				if (NULL != pIPAddr)
				{
					printf("ListenerAddr: %s\n", pIPAddr);
					printf("ListenerName: %s\n", buf);
					//char    szMsg[] = "Speaker Copy";
					if (SOCKET_ERROR != sendto(RecvBroadSocket, m_speakername.c_str(), m_speakername.length(), 0, (sockaddr*)&clientAddr, fromlength))
					{
						//收到listener广播
						string key = string(((sockaddr*)&clientAddr)->sa_data);
						
						lu.ip = string(pIPAddr);
						lu.name = string(buf);
						lu.lasttick = GetTickCount();
						UpdateUser(key, lu);
					}
					
				}
			}
			else
			{

				UpdateUser("", lu);
			}
		}
	}
}
void SpeakerEcho::MatchInfoThread()
{
	int i = 0;
	wchar_t timestr[1024] = { 0 };
	while (m_bRun)
	{
		i++;
		Sleep(100);
		if (i > 30)
		{
			i = 0;
			Message msg;
			SYSTEMTIME st;
			GetSystemTime(&st);
			wsprintf(timestr, L"%04d-%02d-%02d-%02d:%02d:%02d:%04d", st.wYear, st.wMonth, st.wDay, (st.wHour + 8) % 24, st.wMinute, st.wSecond, st.wMilliseconds);
			//wsprintf(timestr, L"%d", GetTickCount());
			msg.SetType(MATCHMSG);
			msg.AppendMsg(L"matchinfo", String2WString(m_matchinfo));
			msg.AppendMsg(L"time", wstring(timestr));
			
			SendMsg(msg.ToString());
		}
	}
}
bool SpeakerEcho::Start()
{
	if (!m_bRun)
	{
		if (InitSpeakEcho())
		{
			std::stringstream url;
			url << "tcp://*:";
			url << MQPORT;
			if (zmq_bind(m_pPubSocket, url.str().c_str()) != 0)
			{
				return false;
			}
			//启动echothread
			m_bRun = true;
			std::thread echothread(std::bind(&SpeakerEcho::EchoThread, this));
			echothread.detach();
			std::thread matchinfothread(std::bind(&SpeakerEcho::MatchInfoThread, this));
			matchinfothread.detach();
			return true;
		}

	}
	return false;
}
bool SpeakerEcho::Stop()
{
	m_bRun = false;
	Sleep(200);
	std::stringstream url;
	url << "tcp://*:";
	url << MQPORT;
	zmq_unbind(m_pPubSocket, url.str().c_str());
	closesocket(RecvBroadSocket);
	
	return true;
}

bool SpeakerEcho::SendMsg(string msg)
{
	zmq_msg_t msgt;
	//OutputDebugString(msg.c_str());
	//OutputDebugString(L"\n");
	zmq_msg_init_size(&msgt, msg.length());
	memcpy(zmq_msg_data(&msgt), msg.c_str(), msg.length());
	if(zmq_msg_send(&msgt, m_pPubSocket, ZMQ_DONTWAIT)>0)
	{
		zmq_msg_close(&msgt);
		return true;
	}
	return false;
}