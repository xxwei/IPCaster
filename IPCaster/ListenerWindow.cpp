#include "stdafx.h"
#include "ListenerWindow.h"


const TCHAR* const kCloseButtonControlName = _T("closebtn");
const TCHAR* const kMinButtonControlName = _T("minbtn");
const TCHAR* const kMaxButtonControlName = _T("maxbtn");
const TCHAR* const kRestoreButtonControlName = _T("restorebtn");
const TCHAR* const kConnectSpeakerName = _T("Connect");
const TCHAR* const kRegAppName = _T("reg");

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
	if (m_bConnecting)
	{
		CLabelUI* pControl = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("ErrorInfo")));
		if (pControl)
		{
			pControl->SetText(L"连接成功");
			pControl->SetAttribute(L"textcolor", L"#FF00FF00");
		}
		m_bConnecting = false;
	}
}
void ListenerWindow::OnSpeakerOffLine(int ntype)
{
	if (!ntype)
	{
		CLabelUI *pControl = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("loginfo")));
		if (pControl)
		{
			pControl->SetText(L"Speaker offline");
		}
	}
	if (m_bConnecting)
	{
		CLabelUI* pControl = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("ErrorInfo")));
		if (pControl)
		{
			pControl->SetText(L"连接失败");
			pControl->SetAttribute(L"textcolor", L"#FFFF0000");
		}
		m_bConnecting = false;
	}
}

void ListenerWindow::OnRecvMessage(Message *msg)
{
	if (msg->GetType() == PUBMSG)
	{
		m_newmsg = msg->GetMsgValue(L"text");
		SetTimer(m_hWnd, 3, 1, NULL);
        //SetTimer(m_hWnd, 5, 10, NULL);
		//AddNewMessage(msg->GetMsgValue(L"text"));
	}
    if (msg->GetType() == CMDMSG)
    {
        m_newcmd = msg->GetMsgValue(L"cmd");
        SetTimer(m_hWnd,6, 1, NULL);
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

	CRichEditUI* pRControl = static_cast<CRichEditUI*>(m_PaintManager.FindControl(_T("memcode")));
	wstring mcode = pStateManger->GetMCode();
	pRControl->SetText(mcode.c_str());
	
	CLabelUI* pLControl = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("RegErrorInfo")));
	if (!pStateManger->IsRegOK())
	{
		
		pLControl->SetText(L"未注册");
		pLControl->SetTextColor(0xFFFF0000);
		
	}
	else
	{
		if (pStateManger->IsOutDate())
		{
			pLControl->SetText(L"已过期");
			pLControl->SetTextColor(0xFFFF0000);
		}
		else
		{
			pLControl->SetText(L"已注册");
			pLControl->SetTextColor(0xFF00FF00);
			pRControl = static_cast<CRichEditUI*>(m_PaintManager.FindControl(_T("regcode")));
			wstring regcode = pStateManger->GetRegCode();
			pRControl->SetText(regcode.c_str());
		}
	}
	//读取配置
	UpdateSettingUI();
	UpdateMainUI();
	m_bInit = true;
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
				KillTimer(m_hWnd, 2);
			}
		}
		if (wParam == 3)
		{
			AddNewMessage(m_newmsg);
			KillTimer(m_hWnd, 3);
            KillTimer(m_hWnd, 5);
			SetTimer(m_hWnd, 2, 500,NULL);
            SetTimer(m_hWnd, 5, 10000, NULL);
		}
		if (wParam == 4)
		{
			CTextUI* pControl = static_cast<CTextUI*>(m_PaintManager.FindControl(_T("matchinfo")));
			pControl->SetText(pStateManger->GetMatchInfo().c_str());
			KillTimer(m_hWnd, 4);
		}
        if (wParam == 5)
        {
            //最新消息变灰
            NewMessageOld();
            KillTimer(m_hWnd, 5);
        }
        if (wParam == 6)
        {
            KillTimer(m_hWnd, 6);
            if (m_newcmd == L"CLEAR")
            {
                CListUI* pControl = static_cast<CListUI*>(m_PaintManager.FindControl(_T("msglist")));
                if (pControl)
                {
                    pControl->RemoveAll();
                }
            }
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
				return S_FALSE;
			}
			case VK_F2:
			{
				int ret = ::GetKeyState(VK_SHIFT); //>0 up <0 down
				if (pStateManger->CanChangeSpeaker() && ret < 0)
				{
					//切换到speaker
					if (pStateManger->ChangeToSpeaker())
					{
						Close();
						break;
					}

				}
			}
			case VK_F1:
			{
				int ret = ::GetKeyState(VK_SHIFT); //>0 up <0 down
				if (pStateManger->CanChangeSpeaker() && ret < 0)
				{
					//切换到speaker
					if (pStateManger->ChangeToSpeakerSample())
					{
						Close();
						break;
					}
					return S_FALSE;
				}
			}

		}
	}
	return __super::MessageHandler(uMsg, wParam, lParam, bHandled);
}
LRESULT ListenerWindow::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	LRESULT ret = __super::OnSize(uMsg, wParam, lParam, bHandled);
	if (m_bInit)
	{
		CListUI* pControl = static_cast<CListUI*>(m_PaintManager.FindControl(_T("msglist")));
		if (!pControl)
			return -1;
		wchar_t insetstr[64] = { 0 };
		int width = LOWORD(lParam);
		int height = HIWORD(lParam);
		int nCount = pControl->GetCount();
		for (int i = 0; i < nCount; i++)
		{
			if (pControl->GetItemAt(i)->GetUserData() == L"msga")
			{
				ReSizeMsgaNode(width - 426, static_cast<CListContainerElementUI*>(pControl->GetItemAt(i)));
			}
			else if (pControl->GetItemAt(i)->GetUserData() == L"mmsg")
			{
				ReSizeMaxMsgNode(width - 426, static_cast<CListContainerElementUI*>(pControl->GetItemAt(i)));
			}
		}
	}
	return ret;
}
void ListenerWindow::ReSizeMsgaNode(int max_width, CListContainerElementUI *new_node)
{
	SIZE px;
	int linecount = 1;
	TFontInfo *tfi = m_PaintManager.GetFontInfo(_wtoi(pStateManger->GetOFont()));
	TFontInfo *dtfi = m_PaintManager.GetDefaultFontInfo();
	HDC hdc = m_PaintManager.GetPaintDC();
	CDuiString msg = new_node->GetItemAt(0)->GetText();
	int dis_num = 0, line_width = 0;//区域内可显示的字符个数，及区域大小（像素点的范围）
	BOOL c_back = TRUE;
	c_back = ::GetTextExtentExPoint(hdc, msg, lstrlen(msg), line_width, &dis_num, NULL, &px);
	int str_width = px.cx;
	int str_height = px.cy;
	str_width = str_width*tfi->tm.tmHeight / dtfi->tm.tmHeight;
	str_height = str_height*tfi->tm.tmHeight / dtfi->tm.tmHeight;
	//int max_width = pList->GetWidth();
	if (c_back)
	{
		linecount = str_width / (max_width - 140) + 1;
	}
	int height = 30 + linecount * (str_height *1.5);
	new_node->SetMinHeight(height);
	new_node->SetMaxHeight(height);

	if (linecount == 1)
	{
		wchar_t insetstr[64] = { 0 };
		int inset_x = (max_width - 20 - str_width*1.2) / 2;
		wsprintf(insetstr, L"%d,5,%d,5", inset_x, inset_x);
		OutputDebugString(insetstr);
		new_node->SetAttribute(L"inset", insetstr);
	}
	else
	{
		wchar_t insetstr[64] = { 0 };
		wsprintf(insetstr, L"%d,5,%d,5", 0, 0);
		OutputDebugString(insetstr);
		new_node->SetAttribute(L"inset", insetstr);
	}
}

void ListenerWindow::ReSizeMaxMsgNode(int max_width, CListContainerElementUI *max_node)
{
	CLabelUI *max_item = static_cast<CLabelUI*>(max_node->GetItemAt(0));
	if (max_item)
	{
		SIZE px;
		int linecount = 1;
		HDC hdc = m_PaintManager.GetPaintDC();
		CDuiString msg = max_node->GetItemAt(0)->GetText();
		int dis_num = 0, line_width = 0;//区域内可显示的字符个数，及区域大小（像素点的范围）
		BOOL c_back = TRUE;
		c_back = ::GetTextExtentExPoint(hdc, msg, lstrlen(msg), line_width, &dis_num, NULL, &px);
		TFontInfo *tfi = m_PaintManager.GetFontInfo(_wtoi(pStateManger->GetNFont()));
		TFontInfo *dtfi = m_PaintManager.GetDefaultFontInfo();
		int str_width = px.cx;
		int str_height = px.cy;
		str_width = str_width*tfi->tm.tmHeight / dtfi->tm.tmHeight;
		str_height = str_height*tfi->tm.tmHeight / dtfi->tm.tmHeight;
		//int max_width = pList->GetWidth();
		if (c_back)
		{
			linecount = str_width / max_width + 1;
		}
		int height = 30 + linecount * (str_height + 8);
		int labheight = height + 100;
		if (linecount < 4)
		{
			labheight = str_height * 3 + 100;
		}
		wchar_t insetstr[64] = { 0 };
		int inset_y = (labheight - height) / 2;
		wsprintf(insetstr, L"0,%d,0,5", inset_y);
		max_item->SetAttribute(L"textpadding", insetstr);
		max_item->SetAttribute(L"font", pStateManger->GetNFont());
		max_node->SetMinHeight(labheight);
		max_node->SetMaxHeight(labheight);

	}
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
		else if (_tcsicmp(msg.pSender->GetName(), kRegAppName) == 0)
		{
			CRichEditUI* pIPControl = static_cast<CRichEditUI*>(m_PaintManager.FindControl(_T("regcode")));
			if (pIPControl)
			{
				CDuiString  regcode = pIPControl->GetText();
				bool ret = pStateManger->SetRegCode(wstring(regcode));
				CLabelUI* pControl = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("RegErrorInfo")));
				if (pControl)
				{
					if (ret)
					{
						pControl->SetAttribute(L"textcolor", L"#FF00FF00");
						pControl->SetText(L"注册成功");
					}
					else
					{
						pControl->SetAttribute(L"textcolor", L"#FFFF0000");
						pControl->SetText(L"注册失败");
					}
				}
			}
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
						m_bConnecting = true;
						retinfo = L"正在尝试连接";
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
			msg.pSender->SetAttribute(L"foreimage", L"file='info_selected.png' dest='61,30,95,64'");
			m_PaintManager.FindControl(_T("page1"))->SetAttribute(L"foreimage", L"file='setting.png' dest='61,30,95,64'");
			pControl->SelectItem(0);
		}
		else if (name == _T("page1"))
		{
			msg.pSender->SetAttribute(L"foreimage", L"file='setting_selected.png' dest='61,30,95,64'");
			m_PaintManager.FindControl(_T("page0"))->SetAttribute(L"foreimage", L"file='info.png' dest='61,30,95,64'");

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
			pStateManger->SaveCasterSetting();
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
			pStateManger->SaveCasterSetting();
			
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
			pStateManger->SaveCasterSetting();
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
void ListenerWindow::NewMessageOld()
{
    CListUI* pControl = static_cast<CListUI*>(m_PaintManager.FindControl(_T("msglist")));
    if (!pControl)
        return ;
    int itemcount = pControl->GetCount();
    if (itemcount)
    {
        CListContainerElementUI *max_node = static_cast<CListContainerElementUI*>(pControl->GetItemAt(itemcount-1));
        if (max_node)
        {
            CLabelUI *max_item = static_cast<CLabelUI*>(max_node->GetItemAt(0));
            if (max_item)
            {
                max_item->SetAttribute(L"bkcolor", L"0xFFEEEEEE");
            }
        }
    }
}
int ListenerWindow::AddNewMessage(wstring str)
{
	//if (!pStateManger->IsRegOK())
	//{
	//	str = L"未注册，无法收到消息";
	//}
    wstring textcolor = L"#0xFF000000";
    if (str.find('#') == 0)
    {
        textcolor = str.substr(1, 10);
        str = str.substr(11, str.length());
    }


	CListUI* pControl = static_cast<CListUI*>(m_PaintManager.FindControl(_T("msglist")));
	if (!pControl)
		return -1;
	SYSTEMTIME st;
	GetSystemTime(&st);
	wchar_t timestr[1024] = { 0 };

	int itemcount = pControl->GetCount();
	wsprintf(timestr, L"%02d:%02d:%02d", (st.wHour + 8) % 24, st.wMinute, st.wSecond);
	CListLabelElementUI *time_node = new CListLabelElementUI();
	time_node->SetAttribute(L"text", timestr);
	time_node->SetAttribute(L"height", L"25");
	time_node->SetAttribute(L"enabled", L"false");
	time_node->SetAttribute(L"align", L"center");
	time_node->SetUserData(L"time");
	if (itemcount)
	{
		pControl->AddAt(time_node, itemcount - 1);
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
		if (max_item)
		{
			CDuiString oldmsg = max_item->GetText();
			CListContainerElementUI *new_node = new CListContainerElementUI;
			new_node->SetAttribute(L"inset", L"60,5,60,5");
			new_node->SetAttribute(L"enabled", L"false");
			new_node->SetUserData(L"msga");
			//new_node->SetAttribute(L"float", L"true");
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
			TFontInfo *tfi = m_PaintManager.GetFontInfo(_wtoi(pStateManger->GetOFont()));
			TFontInfo *dtfi = m_PaintManager.GetDefaultFontInfo();
			HDC hdc = m_PaintManager.GetPaintDC();
			int dis_num = 0, line_width = 0;//区域内可显示的字符个数，及区域大小（像素点的范围）
			BOOL c_back = TRUE;
			c_back = ::GetTextExtentExPoint(hdc, oldmsg, lstrlen(oldmsg), line_width, &dis_num, NULL, &px);
			int str_width = px.cx;
			int str_height = px.cy;
			str_width = str_width*tfi->tm.tmHeight / dtfi->tm.tmHeight;
			str_height = str_height*tfi->tm.tmHeight / dtfi->tm.tmHeight;
			int max_width = pControl->GetWidth();
			if (c_back)
			{
				linecount = str_width / (max_width - 140) + 1;
			}
			int height = 30 + linecount * (str_height *1.5);
			new_node->SetMinHeight(height);
			new_node->SetMaxHeight(height);

			if (linecount == 1)
			{
				wchar_t insetstr[64] = { 0 };
				int inset_x = (max_width - 20 - str_width*1.2) / 2;
				wsprintf(insetstr, L"%d,5,%d,5", inset_x, inset_x);
				new_node->SetAttribute(L"inset", insetstr);
			}
			new_node->Add(pOldMsg);
			pControl->AddAt(new_node, itemcount - 1);

			// 更新文本
			tfi = m_PaintManager.GetFontInfo(_wtoi(pStateManger->GetNFont()));
			c_back = ::GetTextExtentExPoint(hdc, str.c_str(), str.length(), line_width, &dis_num, NULL, &px);
			str_width = px.cx;
			str_height = px.cy;
			str_width = str_width*tfi->tm.tmHeight / dtfi->tm.tmHeight;
			str_height = str_height*tfi->tm.tmHeight / dtfi->tm.tmHeight;
			max_width = pControl->GetWidth();
			if (c_back&&max_width)
			{
				linecount = str_width / max_width + 1;
			}
			height = 30 + linecount * (str_height + 8);

			int labheight = height + 100;
			if (linecount < 4)
			{
				labheight = str_height * 3 + 100;
			}

			wchar_t insetstr[64] = { 0 };
			int inset_x = (max_width - 20 - str_width*1.2) / 2;
			int inset_y = (labheight - height) / 2;
			max_node->SetMinHeight(labheight);
			max_node->SetMaxHeight(labheight);
			wsprintf(insetstr, L"0,%d,0,5", inset_y);
			max_item->SetAttribute(L"textpadding", insetstr);
			max_item->SetAttribute(L"font", pStateManger->GetNFont());
            max_item->SetAttribute(L"bkcolor", L"0xFFD4F3FC");
            max_item->SetAttribute(L"textcolor", textcolor.c_str());
			max_item->SetText(str.c_str());
		}
	}
	else
	{
		CListContainerElementUI *max_node = new CListContainerElementUI;
		max_node->SetAttribute(L"inset", L"60,5,60,5");
		max_node->SetAttribute(L"enabled", L"false");
		max_node->SetUserData(L"mmsg");
		int linecount = 1;
		SIZE px;
		TFontInfo *tfi = m_PaintManager.GetFontInfo(_wtoi(pStateManger->GetNFont()));
		TFontInfo *dtfi = m_PaintManager.GetDefaultFontInfo();
		HDC hdc = m_PaintManager.GetPaintDC();
		int dis_num = 0, line_width = 0;//区域内可显示的字符个数，及区域大小（像素点的范围）
		BOOL c_back = TRUE;
		c_back = ::GetTextExtentExPoint(hdc, str.c_str(), str.length(), line_width, &dis_num, NULL, &px);
		int str_width = px.cx;
		int str_height = px.cy;
		str_width = str_width*tfi->tm.tmHeight / dtfi->tm.tmHeight;
		str_height = str_height*tfi->tm.tmHeight / dtfi->tm.tmHeight;
		int max_width = pControl->GetWidth();
		if (c_back)
		{
			linecount = str_width / (max_width - 140) + 1;
		}
		int height = 30 + linecount * (str_height + 8);

		CLabelUI *pMaxMsg = new CLabelUI();
		pMaxMsg->SetAttribute(L"bkcolor", L"0xFFD4F3FC");
		pMaxMsg->SetAttribute(L"align", L"center");
		pMaxMsg->SetAttribute(L"valign", L"vcenter");
		pMaxMsg->SetAttribute(L"borderround", L"10,10");
		pMaxMsg->SetAttribute(L"multiline", L"true");
        pMaxMsg->SetAttribute(L"textcolor", textcolor.c_str());
		pMaxMsg->SetAttribute(L"font", pStateManger->GetNFont());

		int labheight = height + 100;
		if (linecount < 4)
		{
			labheight = str_height * 3 + 100;
		}

		wchar_t insetstr[64] = { 0 };
		int inset_x = (max_width - 20 - str_width*1.2) / 2;
		int inset_y = (labheight - height) / 2;
		wsprintf(insetstr, L"0,%d,0,5", inset_y);
		pMaxMsg->SetAttribute(L"textpadding", insetstr);

		pMaxMsg->SetText(str.c_str());


		max_node->SetMinHeight(labheight);
		max_node->SetMaxHeight(labheight);
		max_node->Add(pMaxMsg);
		pControl->Add(max_node);
	}

	wsprintf(timestr, L"%04d-%02d-%02d %02d:%02d:%02d:%04d", st.wYear, st.wMonth, st.wDay, (st.wHour + 8) % 24, st.wMinute, st.wSecond, st.wMilliseconds);
	pStateManger->AddChatlog(wstring(timestr), str);
	pControl->EndDown();
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
void ListenerWindow::UpdateSettingUI()
{
	wstring ip = pStateManger->GetLocalIP();
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
	CComboUI *pComboControl = static_cast<CComboUI*>(m_PaintManager.FindControl(_T("nfontsize")));
	pComboControl->SetInternVisible();
	int lnfont = atoi(pStateManger->SettingValue["LNfont"].asCString());
	pComboControl->SelectItem(lnfont - 5);
	pComboControl = static_cast<CComboUI*>(m_PaintManager.FindControl(_T("ofontsize")));
	pComboControl->SetInternVisible();
	int lofont = atoi(pStateManger->SettingValue["LOfont"].asCString());
	pComboControl->SelectItem(lofont-1);
}

void ListenerWindow::SaveSetting()
{

}