#include "stdafx.h"
#include "ListenerWindow.h"


const TCHAR* const kCloseButtonControlName = _T("closebtn");
const TCHAR* const kMinButtonControlName = _T("minbtn");
const TCHAR* const kMaxButtonControlName = _T("maxbtn");
const TCHAR* const kRestoreButtonControlName = _T("restorebtn");
const TCHAR* const kConnectSpeakerName = _T("Connect");


void ListenerWindow::OnSpeakerOnLine(wstring ip, wstring name)
{
	CLabelUI *pControl = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("loginfo")));
	if (pControl)
	{
		CDuiString info;
		info.Append(name.c_str());
		info.Append(L":");
		info.Append(ip.c_str());
		pControl->SetText(info);
	}
}
void ListenerWindow::OnSpeakerOffLine()
{
	CLabelUI *pControl = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("loginfo")));
	if (pControl)
	{
		pControl->SetText(L"Speaker offline");
	}
}
void ListenerWindow::OnRecvMessage(Message *msg)
{
	if (msg->GetType() == PUBMSG)
	{
		m_newmsg = msg->GetMsgValue(L"text");
		SetTimer(m_hWnd, 3, 1, NULL);
		//AddNewMessage(msg->GetMsgValue(L"text"));
	}
	if (msg->GetType() == MATCHMSG)
	{
		wstring matchinfo = msg->GetMsgValue(L"matchinfo");
		if (matchinfo != pStateManger->GetMatchInfo())
		{
			pStateManger->SetMatchInfo(matchinfo);
			SetTimer(m_hWnd, 4, 1, NULL);
		}
		//AddNewMessage(msg->GetMsgValue(L"text"));
	}
}

void  ListenerWindow::InitWindow()
{
	
	HWND hwnd = m_PaintManager.GetPaintWindow();
	SetWindowText(hwnd, L"电子公告栏");
	SetTimer(hwnd, 1, 1000, NULL);
	CTextUI* pControl = static_cast<CTextUI*>(m_PaintManager.FindControl(_T("ipaddr")));
	wstring ip = pStateManger->GetLocalIP();
	pControl->SetText(ip.c_str());
	//读取配置
	CRichEditUI *pNickNameControl = static_cast<CRichEditUI*>(m_PaintManager.FindControl(_T("localnickname")));
	if (pNickNameControl)
	{
		
		if (pStateManger->GetListenerNickName() == L"")
		{
			int pos = ip.find_last_of('.');
			wstring nickname = L"listener" + ip.substr(pos + 1, ip.length());
			pNickNameControl->SetText(nickname.c_str());
		}
		else
		{
			pNickNameControl->SetText(pStateManger->GetListenerNickName().c_str());
		}
		pStateManger->SetListenerNickName(wstring(pStateManger->GetListenerNickName()));
	}
	UpdateMainUI();


}
LRESULT ListenerWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg== WM_TIMER)
	{
		if (wParam == 1)
		{
			SYSTEMTIME st;
			GetSystemTime(&st);
			wchar_t timestr[1024] = { 0 };
			wsprintf(timestr, L"%02d:%02d:%02d", (st.wHour + 8) % 24, st.wMinute, st.wSecond);
			CTextUI* pControl = static_cast<CTextUI*>(m_PaintManager.FindControl(_T("time")));
			pControl->SetText(timestr);
		}
		if (wParam == 2)
		{
			CListUI* pListControl = static_cast<CListUI*>(m_PaintManager.FindControl(_T("msglist")));
			if (pListControl)
			{
				pListControl->EndDown();
				HWND hwnd = m_PaintManager.GetPaintWindow();
				KillTimer(hwnd, 2);
			}
		}
		if (wParam == 3)
		{
			AddNewMessage(m_newmsg);
			KillTimer(m_hWnd, 3);
		}
		if (wParam == 4)
		{
			CTextUI* pControl = static_cast<CTextUI*>(m_PaintManager.FindControl(_T("matchinfo")));
			pControl->SetText(pStateManger->GetMatchInfo().c_str());
			KillTimer(m_hWnd, 4);
		}
	}
	return __super::HandleMessage(uMsg,wParam,lParam);
}
LRESULT ListenerWindow::MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
	if (uMsg == WM_KEYDOWN)
	{
		switch (wParam)
		{
		case VK_ESCAPE:
		{
			int ret = ::GetKeyState(VK_SHIFT); //>0 up <0 down
			if (pStateManger->CanChangeSpeaker() && ret < 0)
			{
				//切换到speaker
				if (pStateManger->ChangeToSpeaker())
				{
					break;
				}
				//屏蔽ESC退出
				return S_FALSE;

			}
			else
			{
				//屏蔽ESC退出
				return S_OK;
			}
		}
		case VK_CONTROL:
		{
			AddNewMessage(L"12312312312312312390909090007777777777777777777111111111111111111111111");
			break;
		}
			
		}
	}
	//else if (uMsg == WM_KEYUP)
	//{
	//	switch (wParam)
	//	{
	//	case VK_ESCAPE:
	//		::GetKeyState(VK_SHIFT); //>0 up <0 down
	//		return S_OK;
	//	}
	//}
	return __super::MessageHandler(uMsg, wParam, lParam, bHandled);
}
bool isIPAddressValid(const char* pszIPAddr)
{
	if (!pszIPAddr) return false; //若pszIPAddr为空    
	char IP1[100], cIP[4];
	int len = strlen(pszIPAddr);
	int i = 0, j = len - 1;
	int k, m = 0, n = 0, num = 0;
	//去除首尾空格(取出从i-1到j+1之间的字符):    
	while (pszIPAddr[i++] == ' ');
	while (pszIPAddr[j--] == ' ');

	for (k = i - 1; k <= j + 1; k++)
	{
		IP1[m++] = *(pszIPAddr + k);
	}
	IP1[m] = '\0';

	char *p = IP1;

	while (*p != '\0')
	{
		if (*p == ' ' || *p<'0' || *p>'9') return false;
		cIP[n++] = *p; //保存每个子段的第一个字符，用于之后判断该子段是否为0开头    

		int sum = 0;  //sum为每一子段的数值，应在0到255之间    
		while (*p != '.'&&*p != '\0')
		{
			if (*p == ' ' || *p<'0' || *p>'9') return false;
			sum = sum * 10 + *p - 48;  //每一子段字符串转化为整数    
			p++;
		}
		if (*p == '.') {
			if ((*(p - 1) >= '0'&&*(p - 1) <= '9') && (*(p + 1) >= '0'&&*(p + 1) <= '9'))//判断"."前后是否有数字，若无，则为无效IP，如“1.1.127.”    
				num++;  //记录“.”出现的次数，不能大于3    
			else
				return false;
		};
		if ((sum > 255) || (sum > 0 && cIP[0] == '0') || num>3) return false;//若子段的值>255或为0开头的非0子段或“.”的数目>3，则为无效IP    

		if (*p != '\0') p++;
		n = 0;
	}
	if (num != 3) return false;
	return true;
}
void ListenerWindow::Notify(TNotifyUI& msg)
{
	if (msg.sType == L"click")
	{
		if (_tcsicmp(msg.pSender->GetName(), kCloseButtonControlName) == 0)
		{
			::PostQuitMessage(0);
		}
		else if (_tcsicmp(msg.pSender->GetName(), kMinButtonControlName) == 0)
		{
			SendMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0);
		}
		else if (_tcsicmp(msg.pSender->GetName(), kMaxButtonControlName) == 0)
		{
			SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE, 0);
		}
		else if (_tcsicmp(msg.pSender->GetName(), kRestoreButtonControlName) == 0)
		{
			SendMessage(WM_SYSCOMMAND, SC_RESTORE, 0);
		}
		else if (_tcsicmp(msg.pSender->GetName(), kConnectSpeakerName) == 0)
		{
			
			
			CRichEditUI* pIPControl = static_cast<CRichEditUI*>(m_PaintManager.FindControl(_T("speakerip")));
			if (pIPControl)
			{
				wstring retinfo = L"";
				bool bok = false;
				CDuiString  ip = pIPControl->GetText();
				do
				{
					if (!isIPAddressValid(WString2String(wstring(ip)).c_str()))
					{
						retinfo = L"IP地址非法";
						break;
					}

					if (pStateManger->ConnectSpeaker(wstring(ip)))
					{
						retinfo = L"链接成功";
						bok = true;	
					}
					else
					{
						retinfo = L"链接失败";
					}
					break;
				} while (true);
				CLabelUI* pControl = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("ErrorInfo")));
				if (pControl)
				{
					if (bok)
					{
						pControl->SetAttribute(L"textcolor", L"#FF00FF00");
					}
					else
					{
						pControl->SetAttribute(L"textcolor", L"#FFFF0000");
					}
					
					pControl->SetText(retinfo.c_str());

			}
			}
		}
	}
	else if (msg.sType == _T("selectchanged"))
	{
		CDuiString name = msg.pSender->GetName();
		CTabLayoutUI* pControl = static_cast<CTabLayoutUI*>(m_PaintManager.FindControl(_T("pageswitch")));
		if (name == _T("page0"))
		{
			msg.pSender->SetAttribute(L"foreimage", L"file='info_selected.png' dest='76,30,110,64'");
			m_PaintManager.FindControl(_T("page1"))->SetAttribute(L"foreimage", L"file='setting.png' dest='76,30,110,64'");
			pControl->SelectItem(0);
		}
		else if (name == _T("page1"))
		{
			msg.pSender->SetAttribute(L"foreimage", L"file='setting_selected.png' dest='76,30,110,64'");
			m_PaintManager.FindControl(_T("page0"))->SetAttribute(L"foreimage", L"file='info.png' dest='76,30,110,64'");

			pControl->SelectItem(1);
		}
	}
	else if (msg.sType == L"setfocus")
	{
		CDuiString name = msg.pSender->GetName();
		if (name == L"localnickname")
		{
			msg.pSender->SetAttribute(L"bkcolor", L"#FFFFFFFF");
		}
	}
	else if (msg.sType == L"killfocus")
	{
		CDuiString name = msg.pSender->GetName();
		if (name == L"localnickname")
		{
			msg.pSender->SetAttribute(L"bkcolor", L"#FFF3F3F3");
			//设置名称
			pStateManger->SetListenerNickName(wstring(msg.pSender->GetText()));
			UpdateMainUI();
		}
	}
	else if (msg.sType == _T("return"))
	{
		CDuiString name = msg.pSender->GetName();
		if (name == L"localnickname")
		{
			msg.pSender->SetAttribute(L"bkcolor", L"#FFF3F3F3");
		}
	}
	else if (msg.sType == _T("itemselect"))
	{
		CDuiString name = msg.pSender->GetName();
		if (name == L"ofontsize")
		{
			//msg.pSender->SetAttribute(L"bkcolor", L"#FFF3F3F3");
			CComboUI* pControl = static_cast<CComboUI*>(m_PaintManager.FindControl(_T("ofontsize")));
			if (pControl)
			{
				CDuiString fontid = pControl->GetItemAt(msg.wParam)->GetUserData();
				CLabelUI* pPreviewControl = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("opreview")));
				if (pPreviewControl)
				{
					pPreviewControl->SetAttribute(L"font", fontid);
					if (pStateManger)
					{
						pStateManger->SetOFont(fontid);
					}
				}
			}
			
			
		}
		if (name == L"nfontsize")
		{
			CComboUI* pControl = static_cast<CComboUI*>(m_PaintManager.FindControl(_T("nfontsize")));
			if (pControl)
			{
				CDuiString fontid = pControl->GetItemAt(msg.wParam)->GetUserData();
				CLabelUI* pPreviewControl = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("npreview")));
				if (pPreviewControl)
				{
					pPreviewControl->SetAttribute(L"font", fontid);
					if (pStateManger)
					{
						pStateManger->SetNFont(fontid);
					}
				}
			}
		}
	}

}
LRESULT ListenerWindow::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return __super::OnClose(uMsg, wParam, lParam, bHandled);
}
LRESULT ListenerWindow::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return __super::OnDestroy(uMsg, wParam, lParam, bHandled);
}
void ListenerWindow::SetStateManger(StateManger *pSm)
{
	pStateManger = pSm;
}
int ListenerWindow::AddNewMessage(wstring str)
{
	CListUI* pControl = static_cast<CListUI*>(m_PaintManager.FindControl(_T("msglist")));
	if (!pControl)
		return -1;
	SYSTEMTIME st;
	GetSystemTime(&st);
	wchar_t timestr[1024] = { 0 };

	int itemcount = pControl->GetCount();
	wsprintf(timestr, L"%02d:%02d:%02d", (st.wHour + 8) % 24, st.wMinute, st.wSecond);
	CListLabelElementUI *time_node = new CListLabelElementUI();
	time_node->SetAttribute(L"height", L"25");
	time_node->SetAttribute(L"enabled", L"false");
	time_node->SetAttribute(L"text", timestr);
	time_node->SetAttribute(L"align", L"center");
	if (itemcount)
	{
		pControl->AddAt(time_node,itemcount-1);
	}
	else
	{
		pControl->Add(time_node);
	}

	if (itemcount)
	{
		//有记录
		
		CListContainerElementUI *max_node = static_cast<CListContainerElementUI*>(pControl->GetItemAt(itemcount));
		CLabelUI *max_item = static_cast<CLabelUI*>(max_node->GetItemAt(0));
		if(max_item)
		{
			CDuiString oldmsg = max_item->GetText();
			CListContainerElementUI *new_node = new CListContainerElementUI;
			new_node->SetAttribute(L"inset", L"60,5,60,5");
			new_node->SetAttribute(L"enabled", L"false");
			CRichEditUI *pOldMsg = new CRichEditUI();
			pOldMsg->SetAttribute(L"bkcolor", L"0xFFEEEEEE");
			pOldMsg->SetAttribute(L"align", L"center");
			pOldMsg->SetAttribute(L"multiline", L"true");
			pOldMsg->SetAttribute(L"borderround", L"10,10");
			pOldMsg->SetAttribute(L"font", pStateManger->GetOFont());
			RECT rc;
			rc.left = 10;
			rc.top = 10;
			rc.right = 10;
			rc.bottom = 10;
			pOldMsg->SetInset(rc);
			pOldMsg->SetText(oldmsg);

			SIZE px;
			int linecount = 1;
			HDC hdc = m_PaintManager.GetPaintDC();
			int dis_num = 0, line_width = 0;//区域内可显示的字符个数，及区域大小（像素点的范围）
			BOOL c_back = TRUE;
			c_back = ::GetTextExtentExPoint(hdc, oldmsg, lstrlen(oldmsg), line_width, &dis_num, NULL, &px);
			int max_width = pControl->GetWidth();
			if (c_back)
			{
				linecount = px.cx / (max_width - 140) + 1;
			}
			int height = 30 + linecount * (px.cy + 8);
			new_node->SetMinHeight(height);
			new_node->SetMaxHeight(height);

			if (linecount == 1)
			{
				wchar_t insetstr[64] = { 0 };
				int inset_x = (max_width - 40 - px.cx) / 2;
				wsprintf(insetstr, L"%d,5,%d,5", inset_x, inset_x);
				new_node->SetAttribute(L"inset", insetstr);
			}
			new_node->Add(pOldMsg);
			pControl->AddAt(new_node, itemcount - 1);
		}
		max_item->SetAttribute(L"font", pStateManger->GetNFont());
		max_item->SetText(str.c_str());
		
		
	}
	else
	{
		CListContainerElementUI *max_node = new CListContainerElementUI;
		max_node->SetAttribute(L"inset", L"60,5,60,5");
		max_node->SetAttribute(L"enabled", L"false");
		CLabelUI *pMaxMsg = new CLabelUI();
		pMaxMsg->SetAttribute(L"bkcolor", L"0xFFD4F3FC");
		pMaxMsg->SetAttribute(L"align", L"center");
		pMaxMsg->SetAttribute(L"valign", L"center");
		pMaxMsg->SetAttribute(L"borderround", L"10,10");
		pMaxMsg->SetAttribute(L"font", pStateManger->GetNFont());
		pMaxMsg->SetText(str.c_str());
		max_node->SetMinHeight(500);
		max_node->SetMaxHeight(500);
		max_node->Add(pMaxMsg);
		pControl->Add(max_node);
	}

	wsprintf(timestr, L"%04d-%02d-%02d %02d:%02d:%02d:%04d",st.wYear,st.wMonth,st.wDay, (st.wHour + 8) % 24, st.wMinute, st.wSecond,st.wMilliseconds);
	pStateManger->AddChatlog(wstring(timestr), str);
	HWND hwnd = m_PaintManager.GetPaintWindow();
	SetTimer(hwnd, 2, 60, NULL);
	return 0;

}
void ListenerWindow::UpdateMainUI()
{
	CTextUI *pNickNameControl = static_cast<CTextUI*>(m_PaintManager.FindControl(_T("nickname")));
	if (pNickNameControl)
	{
		pNickNameControl->SetText(pStateManger->GetListenerNickName().c_str());

	}
	CTextUI *pMatchControl = static_cast<CTextUI*>(m_PaintManager.FindControl(_T("matchinfo")));
	if (pMatchControl)
	{
		pMatchControl->SetText(pStateManger->GetMatchInfo().c_str());

	}
}
void ListenerWindow::SaveSetting()
{

}