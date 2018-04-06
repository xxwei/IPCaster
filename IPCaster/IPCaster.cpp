// IPCaster.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "IPCaster.h"
#include "ListenerWindow.h"
#include "SpeakerWindow.h"
#include "SpeakerWindowSample.h"


//#ifdef _DEBUG
//#pragma comment(lib, "DuiLib_d.lib")
//#pragma comment(lib, "libzmq_d.lib")
//#else
//#pragma comment(lib, "DuiLib.lib")
//#pragma comment(lib, "libzmq.lib")
//#endif

#define SAMPLE

#ifdef _DEBUG
#pragma comment(lib, "DuiLib_ud.lib")
#pragma comment(lib, "libzmq_d.lib")
#pragma comment(lib, "jsoncpp_d.lib")
#else
#pragma comment(lib, "DuiLib_u.lib")
#pragma comment(lib, "libzmq.lib")
#pragma comment(lib, "jsoncpp.lib")
#endif

void InitConsoleWindow()
{
	AllocConsole();

	freopen("CONIN$", "r", stdin);
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);
	std::wcout.clear();
	std::cout.clear();
	std::wcerr.clear();
	std::cerr.clear();
	std::wcin.clear();
	std::cin.clear();
}


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	//InitConsoleWindow();


	HRESULT Hr = ::CoInitialize(NULL);
	if (FAILED(Hr))
	{
		return 0;
	}
	CPaintManagerUI::SetInstance(hInstance);
	CPaintManagerUI::SetResourcePath(CPaintManagerUI::GetResourcePath());
	//CPaintManagerUI::SetResourcePath(L"./");
	HINSTANCE hInstRich = ::LoadLibrary(_T("Riched20.dll"));	
	::OleInitialize(NULL);

	//初始化SOCKET
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	wVersionRequested = MAKEWORD(2, 2);
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0)
	{
		return -1;
	}
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
		WSACleanup();
		return -1;
	}
	//int ipv6mode = 0;
	//ULONG		family = ipv6mode == 0 ? AF_INET : ipv6mode == 1 ? AF_INET6 : AF_UNSPEC;
	//DWORD		flags = GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_DNS_SERVER | GAA_FLAG_SKIP_FRIENDLY_NAME
	//	| GAA_FLAG_INCLUDE_PREFIX | GAA_FLAG_INCLUDE_GATEWAYS;


	StateManger * m_pStateManger = new StateManger();
	m_pStateManger->InitCasterState();
	if (m_pStateManger->IsRegOK() && !m_pStateManger->IsOutDate())
	{
		Sleep(300);
		if (m_pStateManger->CanChangeSpeaker())
		{
			if (m_pStateManger->ChangeToSpeakerSample())
			{
				goto SPAGE;
			}
			
		}
	}

LPAGE:
	ListenerWindow *pLFrame = new ListenerWindow();
	pLFrame->SetStateManger(m_pStateManger);
	m_pStateManger->SetListenerCB(pLFrame);
	pLFrame->Create(NULL, _T("电子公告栏"), UI_WNDSTYLE_FRAME, WS_EX_WINDOWEDGE);
	pLFrame->ShowWindow();
	pLFrame->CenterWindow();
	pLFrame->ShowModal();

SPAGE:

	if (m_pStateManger->GetCurrentState() == 1)
	{
		SpeakerWindow *pSFrame = new SpeakerWindow();
		pSFrame->SetStateManger(m_pStateManger);
		m_pStateManger->SetSpeakerCB(pSFrame);
		pSFrame->Create(NULL, _T("电子公告栏"), UI_WNDSTYLE_FRAME, WS_EX_WINDOWEDGE);
		pSFrame->ShowWindow();
		pSFrame->CenterWindow();
		pSFrame->ShowModal();
		if (m_pStateManger->GetCurrentState() == 0)
		{
			goto LPAGE;
		}
	}

	if (m_pStateManger->GetCurrentState() ==2)
	{
		SpeakerWindowSample *pSFrame = new SpeakerWindowSample();
		pSFrame->SetStateManger(m_pStateManger);
		m_pStateManger->SetSpeakerCB(pSFrame);
		pSFrame->Create(NULL, _T("电子公告栏"), UI_WNDSTYLE_FRAME, WS_EX_WINDOWEDGE);
		pSFrame->ShowWindow();
		pSFrame->CenterWindow();
		pSFrame->ShowModal();
		if (m_pStateManger->GetCurrentState() == 0)
		{
			goto LPAGE;
		}
		if (m_pStateManger->GetCurrentState() == 1)
		{
			goto SPAGE;
		}
	}	

	m_pStateManger->SaveCasterSetting();
	m_pStateManger->ExitState();
	//退出程序并释放COM库
	::CoUninitialize();
	return 0;
}




