#include "stdafx.h"
#include "SpeakerWindow.h"
#include "GroupBoxUI.h"
#include "UIMenu.h"



const TCHAR* const kCloseButtonControlName = _T("closebtn");
const TCHAR* const kMinButtonControlName = _T("minbtn");
const TCHAR* const kMaxButtonControlName = _T("maxbtn");
const TCHAR* const kRestoreButtonControlName = _T("restorebtn");

void  SpeakerWindow::InitWindow()
{

	CacheKeyMap.insert(pair<wstring, bool>(L"0", false));
	CacheKeyMap.insert(pair<wstring, bool>(L"1", false));
	CacheKeyMap.insert(pair<wstring, bool>(L"2", false));
	CacheKeyMap.insert(pair<wstring, bool>(L"3", false));
	CacheKeyMap.insert(pair<wstring, bool>(L"4", false));
	CacheKeyMap.insert(pair<wstring, bool>(L"5", false));
	CacheKeyMap.insert(pair<wstring, bool>(L"6", false));
	CacheKeyMap.insert(pair<wstring, bool>(L"7", false));
	CacheKeyMap.insert(pair<wstring, bool>(L"8", false));
	CacheKeyMap.insert(pair<wstring, bool>(L"9", false));
	CacheKeyMap.insert(pair<wstring, bool>(L"Q", false));
	CacheKeyMap.insert(pair<wstring, bool>(L"W", false));
	CacheKeyMap.insert(pair<wstring, bool>(L"E", false));
	HWND hwnd = m_PaintManager.GetPaintWindow();
	SetWindowText(hwnd, L"电子公告栏");
	SetTimer(hwnd, 1, 1000, NULL);
	CTreeViewUI* pTreeViewControl = static_cast<CTreeViewUI*>(m_PaintManager.FindControl(_T("eventtree")));
	pTreeViewControl->SetVisible(false);
	pTreeViewControl->SetEnabled(false);
	CTextUI* pControl = static_cast<CTextUI*>(m_PaintManager.FindControl(_T("ipaddr")));
	wstring ip = pStateManger->GetLocalIP();
	pControl->SetText(ip.c_str());
	CRichEditUI *pNickNameControl = static_cast<CRichEditUI*>(m_PaintManager.FindControl(_T("localnickname")));
	if (pNickNameControl)
	{

		if (pStateManger->GetSpeakerNickName() == L"")
		{
			int pos = ip.find_last_of('.');
			wstring nickname = L"Speaker" + ip.substr(pos + 1, ip.length());
			pNickNameControl->SetText(nickname.c_str());
		}
		else
		{
			pNickNameControl->SetText(pStateManger->GetSpeakerNickName().c_str());
		}
	}
	pStateManger->GetEventMap(EMmap);

	EventMap::iterator item = EMmap.begin();
	list<wstring>::iterator itemc;
	for (; item != EMmap.end(); item++)
	{
		AddEvent(item->second.EventName);
		GetFreeKey(item->first);
		itemc = item->second.MsgList.begin();
		for (; itemc != item->second.MsgList.end(); itemc++)
		{
			AddMsg(*itemc, item->first);
		}
	}
	
	ImportMatchInfo(L"default.match");
	UpdateMatchInfo();
	UpdateMsgInfo();
	UpdateMainUI();
	UpdateSettingUI();

	
}
CControlUI* SpeakerWindow::CreateControl(LPCTSTR pstrClassName)
{
	if (_tcsicmp(pstrClassName, kGroupBoxUIInterFace) == 0)
		return  new CGroupBoxUI();
	return NULL;
}
void SpeakerWindow::SelectItem()
{
	CRichEditUI *pRControl = static_cast<CRichEditUI*>(m_PaintManager.FindControl(_T("editmsg")));
	pRControl->GetPos();
	CDuiPoint point(pRControl->GetX(), pRControl->GetY());
	ClientToScreen(m_hWnd, &point);
	m_CurrentFindPos = 0;
	if (pRControl)
	{

		m_CurrentFindPos = m_CurrentMsgStr.Find('#');
		if (m_CurrentFindPos >= 0)
		{
			m_CurrentReplaceStr = m_CurrentMsgStr.Mid(m_CurrentFindPos, 2);
			m_CurrentFindPos += 2;
			CMenuWnd* pMenu = CMenuWnd::CreateMenu(NULL, _T("menu.xml"), point, &m_PaintManager, NULL);
			CMenuUI* rootMenu = pMenu->GetMenuUI();

			list<wstring>   templist;

			if (rootMenu != NULL)
			{
				if (m_CurrentReplaceStr == L"#P")
				{
					if (m_CurrentTeam == 0)
					{
						std::copy(Plist.begin(), Plist.end(), std::back_inserter(templist));
					}
					if (m_CurrentTeam == 1)
					{
						std::copy(P1list.begin(), P1list.end(), std::back_inserter(templist));
					}
					if (m_CurrentTeam == 2)
					{
						std::copy(P2list.begin(), P2list.end(), std::back_inserter(templist));
					}
				}
				if (m_CurrentReplaceStr == L"#C")
				{
					if (m_CurrentTeam == 0)
					{
						std::copy(Clist.begin(), Clist.end(), std::back_inserter(templist));
					}
					if (m_CurrentTeam == 1)
					{
						std::copy(C1list.begin(), C1list.end(), std::back_inserter(templist));
					}
					if (m_CurrentTeam == 2)
					{
						std::copy(C2list.begin(), C2list.end(), std::back_inserter(templist));
					}
				}
				if (m_CurrentReplaceStr == L"#R")
				{
					std::copy(Rlist.begin(), Rlist.end(), std::back_inserter(templist));
				}
				if (m_CurrentReplaceStr == L"#T")
				{
					m_bStartSelectTeam = true;
					std::copy(Tlist.begin(), Tlist.end(), std::back_inserter(templist));
				}

				if (templist.size() == 1)
				{
					list<wstring>::iterator item = templist.begin();
					m_CurrentMsgStr = ReplaceOneItem(m_CurrentMsgStr, m_CurrentFindPos - 2, 2, (*item).c_str());
					//m_CurrentMsgStr.Replace(m_CurrentReplaceStr, (*item).c_str());
					pRControl->SetText(m_CurrentMsgStr);
					int pos = m_CurrentMsgStr.Find('#');
					if (pos >= 0)
					{
						SetTimer(m_hWnd, 2, 1, NULL);

					}
				}
				else if(templist.size()>1)
				{

					CDuiString key;
					list<wstring>::iterator item = templist.begin();
					for (int i = 0; item != templist.end(); item++, i++)
					{
						CMenuElementUI* pNew = new CMenuElementUI;
						pNew->SetName((*item).c_str());
						CDuiString text;
						if (i < 10)
						{
							key.Format(_T("%d"), i);
							text.Format(_T("%s  (%d)"), (*item).c_str(), i);
						}
						else
						{
							text.Format(_T("%s"), (*item).c_str());
						}
						pNew->SetText(text);
						pNew->SetUserData(key);
						rootMenu->Add(pNew);
					}
					pMenu->ResizeMenu();
					RECT rc;
					GetWindowRect(pMenu->GetHWND(), &rc);
					MoveWindow(pMenu->GetHWND(), rc.left, rc.top - (rc.bottom - rc.top), rc.right - rc.left, rc.bottom - rc.top, true);
				}
			}
		}
		else
		{
			KillTimer(m_hWnd, 2);
		}
	}
}
CDuiString SpeakerWindow::ReplaceOneItem(CDuiString Dst, int Pos, int len, CDuiString src)
{
	CDuiString newStr;
	CDuiString left = Dst.Left(Pos);
	CDuiString right = Dst.Right(Dst.GetLength() - Pos - len);
	newStr.Append(left);
	newStr.Append(src);
	newStr.Append(right);
	return newStr;

}
LRESULT SpeakerWindow::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (uMsg == WM_MENUCLICK)
	{
		CDuiString strMenuName = CMenuWnd::GetClickedMenuName();
		if (m_CurrentMsgStr == L"")
		{
			m_CurrentMsgStr = strMenuName;
		}
		BOOL bChecked = (BOOL)lParam;
		OutputDebugString(strMenuName);
		CRichEditUI *pRControl = static_cast<CRichEditUI*>(m_PaintManager.FindControl(_T("editmsg")));
		if (pRControl)
		{
			
			pRControl->SetText(m_CurrentMsgStr);
			int pos = strMenuName.Find('#');
			if (pos >= 0)
			{
				m_CurrentTeam = 0;
				SetTimer(m_hWnd, 2, 1, NULL);

			}
			else
			{
				//替换字符串
				m_CurrentMsgStr = ReplaceOneItem(m_CurrentMsgStr, m_CurrentFindPos - 2, 2, strMenuName);
				//m_CurrentMsgStr.Replace(m_CurrentReplaceStr, strMenuName);
				pRControl->SetText(m_CurrentMsgStr);	
				if (m_bStartSelectTeam)
				{
					CComboUI *pComboControl = static_cast<CComboUI*>(m_PaintManager.FindControl(_T("team1combo")));
					if (strMenuName == pComboControl->GetText())
					{
						m_CurrentTeam = 1;
					}
					pComboControl = static_cast<CComboUI*>(m_PaintManager.FindControl(_T("team2combo")));
					if (strMenuName == pComboControl->GetText())
					{
						m_CurrentTeam = 2;
					}
					m_bStartSelectTeam = false;
				}
				pos = m_CurrentMsgStr.Find('#');
				if (pos >= 0)
				{
					SetTimer(m_hWnd, 2, 1, NULL);
				}
			}	
		}
	}
	bHandled = false;
	return 0;
}
LRESULT SpeakerWindow::MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
	if (uMsg == WM_KEYDOWN)
	{
		switch (wParam)
		{
		case VK_ESCAPE:
		{
			int ret = ::GetKeyState(VK_SHIFT); //>0 up <0 down
			if (ret < 0)
			{
				//切换到Listener
				pStateManger->ChangeToListener();
				break;

			}
			else
			{
				//屏蔽ESC退出
				return S_OK;
			}
			return S_OK;
		}
		case VK_CONTROL:
		{
			//AddNewMessage(L"12312312312312312390909090007777777777777777777111111111111111111111111");
			break;
		}
		}
		if ((wParam >= '0'&&wParam <= '9')|| wParam == 'q'|| wParam == 'Q'|| wParam == 'w'|| wParam == 'W'|| wParam == 'e'|| wParam == 'E')
		{
			if (GetKeyState(VK_CONTROL) < 0&& m_CurrentCanShotKey)
			{
				//触发事件选取
				CDuiString key;
				key.Format(_T("%c"), wParam);

				map<wstring, EventMsg>::iterator item = EMmap.find(wstring(key));
				if (item != EMmap.end())
				{
					CRichEditUI* pControl = static_cast<CRichEditUI*>(m_PaintManager.FindControl(_T("editmsg")));
					pControl->GetPos();
					CDuiPoint point(pControl->GetX(), pControl->GetY());
					//GetCursorPos(&point);
					ClientToScreen(m_hWnd, &point);
					

					CMenuWnd* pMenu = CMenuWnd::CreateMenu(NULL, _T("menu.xml"), point, &m_PaintManager, NULL);
					CMenuUI* rootMenu = pMenu->GetMenuUI();
					if (rootMenu != NULL)
					{
						
						list<wstring>::iterator childitem = item->second.MsgList.begin();
						for (int i=0; childitem != item->second.MsgList.end(); childitem++,i++)
						{
							CMenuElementUI* pNew = new CMenuElementUI;
							pNew->SetName((*childitem).c_str());
							key.Format(_T("%d"), i);
							CDuiString text;
							text.Format(_T("%s  (%d)"), (*childitem).c_str(),i);
							pNew->SetText(text);
							pNew->SetUserData(key);
							rootMenu->Add(pNew);
						}
						pMenu->ResizeMenu();
						RECT rc;

						GetWindowRect(pMenu->GetHWND(), &rc);
						MoveWindow(pMenu->GetHWND(), rc.left, rc.top-(rc.bottom - rc.top), rc.right - rc.left, rc.bottom - rc.top, true);
						m_CurrentMsgStr = L"";
					}
				}
			}
		}
	}
	if (uMsg == WM_TIMER)
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
			KillTimer(m_hWnd, 2);
			SelectItem();
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
void SpeakerWindow::Notify(TNotifyUI& msg)
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
		else if (_tcsicmp(msg.pSender->GetName(), L"addp1") == 0)
		{
			AddP1(L"",true);
		}
		else if (_tcsicmp(msg.pSender->GetName(), L"addp2") == 0)
		{
			AddP2(L"", true);
		}
		else if (_tcsicmp(msg.pSender->GetName(), L"addmatchevent") == 0)
		{
			AddEvent(L"",true);
		}
		else if (_tcsicmp(msg.pSender->GetName(), L"addmsg") == 0)
		{
			AddMsg(L"",wstring(msg.pSender->GetUserData()),true);
		}
		else if (_tcsicmp(msg.pSender->GetName(), L"send") == 0)
		{
			CRichEditUI* pControl = static_cast<CRichEditUI*>(m_PaintManager.FindControl(_T("editmsg")));
			wstring sendmsg = wstring(pControl->GetText());
			SendNewMessage(sendmsg);
			pControl->SetText(L"");
		}
		else if (_tcsicmp(msg.pSender->GetName(), L"import") == 0)
		{
			ImportMatchInfo();
		}
		else if (_tcsicmp(msg.pSender->GetName(), L"reset") == 0)
		{
			CComboUI *pComboControl = static_cast<CComboUI*>(m_PaintManager.FindControl(_T("team1combo")));
			SelectTeam1(wstring(pComboControl->GetText()));
			pComboControl = static_cast<CComboUI*>(m_PaintManager.FindControl(_T("team2combo")));
			SelectTeam2(wstring(pComboControl->GetText()));
		}
		
	}
	else if (msg.sType == _T("selectchanged"))
	{
		CDuiString name = msg.pSender->GetName();
		CTabLayoutUI* pControl = static_cast<CTabLayoutUI*>(m_PaintManager.FindControl(_T("pageswitch")));

		if (name == _T("infopub"))
		{
			msg.pSender->SetAttribute(L"foreimage", L"file='info_selected.png' dest='76,30,110,64'");
			m_PaintManager.FindControl(_T("match"))->SetAttribute(L"foreimage", L"file='match.png' dest='76,30,110,64'");
			m_PaintManager.FindControl(_T("messages"))->SetAttribute(L"foreimage", L"file='messages.png' dest='76,30,110,64'");
			m_PaintManager.FindControl(_T("settting"))->SetAttribute(L"foreimage", L"file='setting.png' dest='76,30,110,64'");

			pControl->SelectItem(0);
			m_CurrentCanShotKey = true;
		
		}
		else if (name == _T("match"))
		{
			msg.pSender->SetAttribute(L"foreimage", L"file='match_selected.png' dest='76,30,110,64'");
			m_PaintManager.FindControl(_T("infopub"))->SetAttribute(L"foreimage", L"file='info.png' dest='76,30,110,64'");
			m_PaintManager.FindControl(_T("messages"))->SetAttribute(L"foreimage", L"file='messages.png' dest='76,30,110,64'");
			m_PaintManager.FindControl(_T("settting"))->SetAttribute(L"foreimage", L"file='setting.png' dest='76,30,110,64'");

			pControl->SelectItem(1);
			m_CurrentCanShotKey = false;
			
		}
		else if (name == _T("messages"))
		{
			msg.pSender->SetAttribute(L"foreimage", L"file='messages_selected.png' dest='76,30,110,64'");
			m_PaintManager.FindControl(_T("infopub"))->SetAttribute(L"foreimage", L"file='info.png' dest='76,30,110,64'");
			m_PaintManager.FindControl(_T("match"))->SetAttribute(L"foreimage", L"file='match.png' dest='76,30,110,64'");
			m_PaintManager.FindControl(_T("settting"))->SetAttribute(L"foreimage", L"file='setting.png' dest='76,30,110,64'");

			pControl->SelectItem(2);
			m_CurrentCanShotKey = false;
		}
		else if (name == _T("settting"))
		{
			msg.pSender->SetAttribute(L"foreimage", L"file='setting_selected.png' dest='76,30,110,64'");
			m_PaintManager.FindControl(_T("infopub"))->SetAttribute(L"foreimage", L"file='info.png' dest='76,30,110,64'");
			m_PaintManager.FindControl(_T("match"))->SetAttribute(L"foreimage", L"file='match.png' dest='76,30,110,64'");
			m_PaintManager.FindControl(_T("messages"))->SetAttribute(L"foreimage", L"file='messages.png' dest='76,30,110,64'");

			pControl->SelectItem(3);
			m_CurrentCanShotKey = false;
		}
		else if (name == _T("eventshow"))
		{
			CVerticalLayoutUI* pControl = static_cast<CVerticalLayoutUI*>(m_PaintManager.FindControl(_T("eventtip")));
			if (pControl->GetWidth() < 100)
			{
				pControl->SetFixedWidth(180);
				CTreeViewUI* pControl = static_cast<CTreeViewUI*>(m_PaintManager.FindControl(_T("eventtree")));
				pControl->SetVisible(true);
				pControl->SetEnabled(false);
			}
			else
			{
				pControl->SetFixedWidth(30);
				CTreeViewUI* pControl = static_cast<CTreeViewUI*>(m_PaintManager.FindControl(_T("eventtree")));
				pControl->SetVisible(false);
				pControl->SetEnabled(false);
			}
		}
	}
	else if (msg.sType == _T("return"))
	{
		CDuiString name = msg.pSender->GetName();
		CRichEditUI* pControl = static_cast<CRichEditUI*>(m_PaintManager.FindControl(_T("editmsg")));
		wstring sendmsg = wstring(pControl->GetText());
		SendNewMessage(sendmsg);
		pControl->SetText(L"");

	}
	else if (msg.sType == L"setfocus")
	{
		CDuiString name = msg.pSender->GetClass();
		if (name == L"Edit")
		{
			msg.pSender->SetAttribute(L"bkcolor", L"#FFFFFFFF");
		}
	}
	else if (msg.sType == L"killfocus")
	{
		CDuiString classname = msg.pSender->GetClass();
		CDuiString sendername = msg.pSender->GetName();
		if (sendername == L"localnickname")
		{
			pStateManger->SetSpeakerNickName(wstring(msg.pSender->GetText()));
			UpdateMainUI();
			return;
		}
		if (classname == L"Edit")
		{
			msg.pSender->SetAttribute(L"bkcolor", L"#FFF3F3F3");
			//设置名称

			//更新比赛信息
			UpdateMatchInfo();
			//更新常用消息
			UpdateMsgInfo();
			//更新主界面上信息
			UpdateMainUI();
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
		if (name == L"team1combo")
		{
			SelectTeam1(wstring(msg.pSender->GetText()));

			CComboUI* pControl = static_cast<CComboUI*>(msg.pSender);
			pStateManger->SetTeam1(pControl->GetCurSel());
		}
		if (name == L"team2combo")
		{
			SelectTeam2(wstring(msg.pSender->GetText()));
			CComboUI* pControl = static_cast<CComboUI*>(msg.pSender);
			pStateManger->SetTeam2(pControl->GetCurSel());
		}
		pStateManger->SaveCasterSetting();
	}
}
void SpeakerWindow::OnListenerOnLine(usermap map)
{
	CListUI *pControl = static_cast<CListUI*>(m_PaintManager.FindControl(_T("memberlist")));
	pControl->SetAttribute(L"enabled", L"false");
	if (pControl)
	{
		pControl->RemoveAll();
		usermap::iterator item = map.begin();
		for (; item != map.end(); item++)
		{
			CListContainerElementUI *Listener = new CListContainerElementUI();
			Listener->SetFixedWidth(140);
			Listener->SetFixedHeight(50);
			Listener->SetBorderSize(1);
			Listener->SetBorderColor(0xFFAAAAAA);
			CHorizontalLayoutUI * p1 = new CHorizontalLayoutUI();

			CButtonUI  *head = new CButtonUI();
			head->SetAttribute(L"bkimage", L"file='listener1.png' corner='5,5,5,5'");
			head->SetFixedWidth(30);
			head->SetFixedHeight(30);
			head->SetAttribute(L"padding", L"5,10,0,10");
			p1->Add(head);

			CVerticalLayoutUI  *p2 = new CVerticalLayoutUI();
			p2->SetAttribute(L"padding", L"0,10,0,0");
			CTextUI * nickname = new CTextUI();
			nickname->SetText(String2WString(item->second.name).c_str());
			nickname->SetFont(1);
			CTextUI * iptext = new CTextUI();
			iptext->SetText(String2WString(item->second.ip).c_str());
			p2->Add(nickname);
			p2->Add(iptext);
			p1->Add(p2);
			Listener->Add(p1);
			pControl->Add(Listener);
		}
	}


}
void SpeakerWindow::SetStateManger(StateManger *pSm)
{
	pStateManger = pSm;
}
int SpeakerWindow::SendNewMessage(wstring str)
{
	if (str.length())
	{
		return AddNewMessage(str);
	}
	return -1;
}
int SpeakerWindow::AddNewMessage(wstring str)
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
	time_node->SetAttribute(L"text", timestr);
	time_node->SetAttribute(L"height", L"25");
	time_node->SetAttribute(L"enabled", L"false");
	time_node->SetAttribute(L"align", L"center");
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
				int inset_x = (max_width - 20 - px.cx) / 2;
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

	wsprintf(timestr, L"%04d-%02d-%02d %02d:%02d:%02d:%04d", st.wYear, st.wMonth, st.wDay, (st.wHour + 8) % 24, st.wMinute, st.wSecond, st.wMilliseconds);
	pStateManger->AddChatlog(wstring(timestr), str);
	pControl->EndDown();
	return 0;

}


wstring SpeakerWindow::GetFreeKey(wstring key)
{
	map<wstring, bool>::iterator item = CacheKeyMap.begin();
	if (key == L"")
	{
		while (item != CacheKeyMap.end())
		{
			if (!item->second)
			{
				item->second = true;
				return item->first;
			}
			item++;
		}
	}
	else
	{
		item = CacheKeyMap.find(key);
		if (item != CacheKeyMap.end())
		{
			if (!item->second)
			{
				item->second = true;
				return item->first;
			}
		}
	}
	return L"";

}
void SpeakerWindow::FreeKey(wstring key)
{
	map<wstring, bool>::iterator item = CacheKeyMap.find(key);
	if (item != CacheKeyMap.end())
	{
		item->second = false;
	}
}

int SpeakerWindow::AddR1(wstring str)
{
	CTreeNodeUI *pR1Control = static_cast<CTreeNodeUI*>(m_PaintManager.FindControl(_T("wildcardR1")));
	if (pR1Control)
	{
		CTreeNodeUI *EditNode = new CTreeNodeUI();
		EditNode->SetAttribute(L"height", L"33");
		EditNode->SetAttribute(L"width", L"0");
		EditNode->SetAttribute(L"bordersize", L"1");
		EditNode->SetAttribute(L"bordercolor", L"#FFE5E5E5");
		EditNode->SetUserData(L"R1");
		CEditUI *InfoEdit = new CEditUI();
		InfoEdit->SetAttribute(L"textpadding", L"50,3,0,3");
		InfoEdit->SetAttribute(L"bordersize", L"1");
		InfoEdit->SetAttribute(L"bordercolor", L"#FFE5E5E5");
		InfoEdit->SetAttribute(L"bkcolor", L"#FFF3F3F3");
		InfoEdit->SetText(str.c_str());
		EditNode->Add(InfoEdit);
		pR1Control->AddAt(EditNode, pR1Control->GetCountChild());
		return 0;
	}
	return -1;
}
int SpeakerWindow::AddR2(wstring str)
{
	CTreeNodeUI *pR1Control = static_cast<CTreeNodeUI*>(m_PaintManager.FindControl(_T("wildcardR2")));
	if (pR1Control)
	{
		CTreeNodeUI *EditNode = new CTreeNodeUI();
		EditNode->SetAttribute(L"height", L"33");
		EditNode->SetAttribute(L"width", L"0");
		EditNode->SetAttribute(L"bordersize", L"1");
		EditNode->SetAttribute(L"bordercolor", L"#FFE5E5E5");
		EditNode->SetUserData(L"R2");
		CEditUI *InfoEdit = new CEditUI();
		InfoEdit->SetAttribute(L"textpadding", L"50,3,0,3");
		InfoEdit->SetAttribute(L"bordersize", L"1");
		InfoEdit->SetAttribute(L"bordercolor", L"#FFE5E5E5");
		InfoEdit->SetAttribute(L"bkcolor", L"#FFF3F3F3");
		InfoEdit->SetText(str.c_str());
		EditNode->Add(InfoEdit);
		pR1Control->AddAt(EditNode, pR1Control->GetCountChild());
		return 0;
	}
	return -1;
}
int SpeakerWindow::AddR3(wstring str)
{
	CTreeNodeUI *pR1Control = static_cast<CTreeNodeUI*>(m_PaintManager.FindControl(_T("wildcardR3")));
	if (pR1Control)
	{
		CTreeNodeUI *EditNode = new CTreeNodeUI();
		EditNode->SetAttribute(L"height", L"33");
		EditNode->SetAttribute(L"width", L"0");
		EditNode->SetAttribute(L"bordersize", L"1");
		EditNode->SetAttribute(L"bordercolor", L"#FFE5E5E5");
		EditNode->SetUserData(L"R3");
		CEditUI *InfoEdit = new CEditUI();
		InfoEdit->SetAttribute(L"textpadding", L"50,3,0,3");
		InfoEdit->SetAttribute(L"bordersize", L"1");
		InfoEdit->SetAttribute(L"bordercolor", L"#FFE5E5E5");
		InfoEdit->SetAttribute(L"bkcolor", L"#FFF3F3F3");
		InfoEdit->SetText(str.c_str());
		EditNode->Add(InfoEdit);
		pR1Control->AddAt(EditNode, pR1Control->GetCountChild());
		return 0;
	}
	return -1;
}

int SpeakerWindow::AddP1(wstring str, bool bfocus)
{
	CTreeViewUI *pTControl = static_cast<CTreeViewUI*>(m_PaintManager.FindControl(_T("team1tree")));
	if (pTControl)
	{
		CTreeNodeUI *pControl = static_cast<CTreeNodeUI*>(m_PaintManager.FindControl(_T("wildcardP1")));
		CTreeNodeUI *EditNode = new CTreeNodeUI();
		EditNode->SetAttribute(L"height", L"33");
		EditNode->SetAttribute(L"width", L"0");
		EditNode->SetAttribute(L"bordersize", L"1");
		EditNode->SetAttribute(L"bordercolor", L"#FFE5E5E5");
		EditNode->SetUserData(L"P1");
		CEditUI *InfoEdit = new CEditUI();
		InfoEdit->SetAttribute(L"textpadding", L"30,3,0,3");
		InfoEdit->SetAttribute(L"bordersize", L"1");
		InfoEdit->SetAttribute(L"bordercolor", L"#FFE5E5E5");
		InfoEdit->SetText(str.c_str());
		if(!bfocus)
			InfoEdit->SetAttribute(L"bkcolor", L"#FFF3F3F3");
		EditNode->Add(InfoEdit);
		
		//pTControl->AddAt(EditNode, pControl->GetNodeIndex() + 1);
		//EditNode->SetParentNode(pControl);
		int index = pControl->GetCountChild();
		pControl->AddAt(EditNode,pControl->GetCountChild()-1);
		return 0;
	}
	return -1;
}
int SpeakerWindow::AddC1(wstring str)
{
	CTreeViewUI *pTControl = static_cast<CTreeViewUI*>(m_PaintManager.FindControl(_T("team1tree")));
	if (pTControl)
	{
		CTreeNodeUI *pControl = static_cast<CTreeNodeUI*>(m_PaintManager.FindControl(_T("wildcardC1")));
		CTreeNodeUI *EditNode = new CTreeNodeUI();
		EditNode->SetAttribute(L"height", L"33");
		EditNode->SetAttribute(L"width", L"0");
		EditNode->SetAttribute(L"bordersize", L"1");
		EditNode->SetAttribute(L"bordercolor", L"#FFE5E5E5");
		EditNode->SetUserData(L"C1");
		CEditUI *InfoEdit = new CEditUI();
		InfoEdit->SetAttribute(L"textpadding", L"30,3,0,3");
		InfoEdit->SetAttribute(L"bordersize", L"1");
		InfoEdit->SetAttribute(L"bordercolor", L"#FFE5E5E5");
		InfoEdit->SetText(str.c_str());
		InfoEdit->SetAttribute(L"bkcolor", L"#FFF3F3F3");
		EditNode->Add(InfoEdit);
		pControl->AddAt(EditNode, pControl->GetCountChild());
		return 0;
	}
	return -1;
}
int SpeakerWindow::AddT1(wstring str)
{
	CTreeViewUI *pTControl = static_cast<CTreeViewUI*>(m_PaintManager.FindControl(_T("team1tree")));
	if (pTControl)
	{
		CTreeNodeUI *pControl = static_cast<CTreeNodeUI*>(m_PaintManager.FindControl(_T("wildcardT1")));
		CTreeNodeUI *EditNode = new CTreeNodeUI();
		EditNode->SetAttribute(L"height", L"33");
		EditNode->SetAttribute(L"width", L"0");
		EditNode->SetAttribute(L"bordersize", L"1");
		EditNode->SetAttribute(L"bordercolor", L"#FFE5E5E5");
		EditNode->SetUserData(L"T1");
		CEditUI *InfoEdit = new CEditUI();
		InfoEdit->SetAttribute(L"textpadding", L"30,3,0,3");
		InfoEdit->SetAttribute(L"bordersize", L"1");
		InfoEdit->SetAttribute(L"bordercolor", L"#FFE5E5E5");
		InfoEdit->SetText(str.c_str());
		
		InfoEdit->SetAttribute(L"bkcolor", L"#FFF3F3F3");
		EditNode->Add(InfoEdit);
		pControl->AddAt(EditNode, pControl->GetCountChild());
		
		return 0;
	}
	return -1;
}
int SpeakerWindow::AddP2(wstring str, bool bfocus)
{

	CTreeViewUI *pTControl = static_cast<CTreeViewUI*>(m_PaintManager.FindControl(_T("team2tree")));
	if (pTControl)
	{
		CTreeNodeUI *pControl = static_cast<CTreeNodeUI*>(m_PaintManager.FindControl(_T("wildcardP2")));
		CTreeNodeUI *EditNode = new CTreeNodeUI();
		EditNode->SetAttribute(L"height", L"33");
		EditNode->SetAttribute(L"width", L"0");
		EditNode->SetAttribute(L"bordersize", L"1");
		EditNode->SetAttribute(L"bordercolor", L"#FFE5E5E5");
		EditNode->SetUserData(L"P2");
		CEditUI *InfoEdit = new CEditUI();
		InfoEdit->SetAttribute(L"textpadding", L"30,3,0,3");
		InfoEdit->SetAttribute(L"bordersize", L"1");
		InfoEdit->SetAttribute(L"bordercolor", L"#FFE5E5E5");
		InfoEdit->SetText(str.c_str());
		if (!bfocus)
			InfoEdit->SetAttribute(L"bkcolor", L"#FFF3F3F3");
		EditNode->Add(InfoEdit);
		pControl->AddAt(EditNode, pControl->GetCountChild()-1);
		return 0;
	}
	return -1;
}
int SpeakerWindow::AddC2(wstring str)
{
	CTreeViewUI *pTControl = static_cast<CTreeViewUI*>(m_PaintManager.FindControl(_T("team2tree")));
	if (pTControl)
	{
		CTreeNodeUI *pControl = static_cast<CTreeNodeUI*>(m_PaintManager.FindControl(_T("wildcardC2")));
		CTreeNodeUI *EditNode = new CTreeNodeUI();
		EditNode->SetAttribute(L"height", L"33");
		EditNode->SetAttribute(L"width", L"0");
		EditNode->SetAttribute(L"bordersize", L"1");
		EditNode->SetAttribute(L"bordercolor", L"#FFE5E5E5");
		EditNode->SetUserData(L"C2");
		CEditUI *InfoEdit = new CEditUI();
		InfoEdit->SetAttribute(L"textpadding", L"30,3,0,3");
		InfoEdit->SetAttribute(L"bordersize", L"1");
		InfoEdit->SetAttribute(L"bordercolor", L"#FFE5E5E5");
		InfoEdit->SetText(str.c_str());
		InfoEdit->SetAttribute(L"bkcolor", L"#FFF3F3F3");
		EditNode->Add(InfoEdit);
		pControl->AddAt(EditNode, pControl->GetCountChild());
		return 0;
	}
	return -1;
}
int SpeakerWindow::AddT2(wstring str)
{
	CTreeViewUI *pTControl = static_cast<CTreeViewUI*>(m_PaintManager.FindControl(_T("team2tree")));
	if (pTControl)
	{
		CTreeNodeUI *pControl = static_cast<CTreeNodeUI*>(m_PaintManager.FindControl(_T("wildcardT2")));
		CTreeNodeUI *EditNode = new CTreeNodeUI();
		EditNode->SetAttribute(L"height", L"33");
		EditNode->SetAttribute(L"width", L"0");
		EditNode->SetAttribute(L"bordersize", L"1");
		EditNode->SetAttribute(L"bordercolor", L"#FFE5E5E5");
		EditNode->SetUserData(L"T2");
		CEditUI *InfoEdit = new CEditUI();
		InfoEdit->SetAttribute(L"textpadding", L"30,3,0,3");
		InfoEdit->SetAttribute(L"bordersize", L"1");
		InfoEdit->SetAttribute(L"bordercolor", L"#FFE5E5E5");
		InfoEdit->SetText(str.c_str());
		InfoEdit->SetAttribute(L"bkcolor", L"#FFF3F3F3");
		EditNode->Add(InfoEdit);
		pControl->AddAt(EditNode, pControl->GetCountChild());
		return 0;
	}
	return -1;
}
int SpeakerWindow::AddTeam(wstring str)
{
	CComboUI *pComboControl = static_cast<CComboUI*>(m_PaintManager.FindControl(_T("team1combo")));
	CListLabelElementUI *team1 = new CListLabelElementUI();
	team1->SetAttribute(L"text", str.c_str());
	pComboControl->Add(team1);
	pComboControl = static_cast<CComboUI*>(m_PaintManager.FindControl(_T("team2combo")));
	CListLabelElementUI *team2 = new CListLabelElementUI();
	team2->SetAttribute(L"text", str.c_str());
	pComboControl->Add(team2);
	return 0;
}
int SpeakerWindow::AddEvent(wstring str, bool bfocus)
{
	CTreeViewUI *pRControl = static_cast<CTreeViewUI*>(m_PaintManager.FindControl(_T("msgtree")));
	if (pRControl)
	{


		wstring key = GetFreeKey();
		CTreeNodeUI *pControl = static_cast<CTreeNodeUI*>(m_PaintManager.FindControl(_T("eventnode")));
		
		//添加消息类型
		CTreeNodeUI *EditNode = new CTreeNodeUI();
		EditNode->SetAttribute(L"height", L"33");
		EditNode->SetAttribute(L"width", L"0");
		EditNode->SetAttribute(L"bordersize", L"1");
		EditNode->SetAttribute(L"bordercolor", L"#FFE5E5E5");
		EditNode->SetName(key.c_str());
		CHorizontalLayoutUI *InfoLayout = new CHorizontalLayoutUI();
		InfoLayout->SetAttribute(L"bordersize", L"1");
		InfoLayout->SetAttribute(L"bordercolor", L"#FFE5E5E5");
		InfoLayout->SetAttribute(L"inset", L"1,0,0,0");
		CEditUI *InfoEdit = new CEditUI();
		InfoEdit->SetAttribute(L"textpadding", L"30,3,30,3");
		InfoEdit->SetAttribute(L"bordersize", L"0");
		InfoEdit->SetAttribute(L"bordercolor", L"#FFE5E5E5");
		InfoEdit->SetText(str.c_str());
		if(!bfocus)
			InfoEdit->SetAttribute(L"bkcolor", L"#FFF3F3F3");
		CLabelUI *keyLabel = new CLabelUI();
		keyLabel->SetText(key.c_str());
		keyLabel->SetFixedWidth(100);
		keyLabel->SetMaxWidth(100);
		keyLabel->SetAttribute(L"align", L"center");
		InfoLayout->Add(InfoEdit);
		InfoLayout->Add(keyLabel);
		InfoLayout->SetUserData(key.c_str());
		EditNode->Add(InfoLayout);

		//添加本消息按钮
		CTreeNodeUI *AddNode = new CTreeNodeUI();
		AddNode->SetAttribute(L"height", L"33");
		AddNode->SetAttribute(L"width", L"0");
		AddNode->SetAttribute(L"bordersize", L"1");
		AddNode->SetAttribute(L"bordercolor", L"#FFE5E5E5");
		AddNode->SetUserData(L"addmsg");
		CButtonUI * AddBtn = new CButtonUI();
		AddBtn->SetText(L"+添加消息");
		AddBtn->SetAttribute(L"textcolor", L"#FF0000FF");
		AddBtn->SetAttribute(L"textpadding", L"0,0,0,0");
		AddBtn->SetAttribute(L"name", L"addmsg");
		AddBtn->SetAttribute(L"userdata", key.c_str());
		AddBtn->SetFixedWidth(100);
		AddBtn->SetMaxWidth(100);
		AddNode->Add(AddBtn);
		EditNode->Add(AddNode);
		pControl->AddAt(EditNode, pControl->GetCountChild()-1);		
		return 0;
	}
	return -1;
}
int SpeakerWindow::AddMsg(wstring str, wstring type, bool bfocus)
{
	CTreeNodeUI *pControl = static_cast<CTreeNodeUI*>(m_PaintManager.FindControl(type.c_str()));
	if (pControl)
	{
		CTreeNodeUI *EditNode = new CTreeNodeUI();
		EditNode->SetAttribute(L"height", L"33");
		EditNode->SetAttribute(L"width", L"0");
		EditNode->SetAttribute(L"bordersize", L"1");
		EditNode->SetAttribute(L"bordercolor", L"#FFE5E5E5");
		CEditUI *InfoEdit = new CEditUI();
		InfoEdit->SetAttribute(L"textpadding", L"50,3,30,3");
		InfoEdit->SetAttribute(L"bordersize", L"1");
		InfoEdit->SetAttribute(L"bordercolor", L"#FFE5E5E5");
		if (!bfocus)
			InfoEdit->SetAttribute(L"bkcolor", L"#FFF3F3F3");
		InfoEdit->SetText(str.c_str());
		EditNode->Add(InfoEdit);
		pControl->AddAt(EditNode, pControl->GetCountChild()-1);
		return 0;
	}
	return -1;
}
void SpeakerWindow::SelectTeam1(wstring str)
{
	ClearT1UIInfo();
	if (!pStateManger->MatchValue["teams"].isNull())
	{
		int count = pStateManger->MatchValue["teams"].size();
		for (int i = 0; i < count; i++)
		{
			wstring teamname = String2WString(string(U2G(pStateManger->MatchValue["teams"][i]["T"][0].asCString())));
			if (teamname == str)
			{
				AddT1(teamname);
				int Ccount = pStateManger->MatchValue["teams"][i]["C"].size();
				for (int j = 0; j < Ccount; j++)
				{
					wstring cname = String2WString(string(U2G(pStateManger->MatchValue["teams"][i]["C"][j].asCString())));
					AddC1(cname);
				}
				int Pcount = pStateManger->MatchValue["teams"][i]["P"].size();
				for (int j = 0; j < Pcount; j++)
				{
					wstring cname = String2WString(string(U2G(pStateManger->MatchValue["teams"][i]["P"][j].asCString())));
					AddP1(cname);
				}
				break;
			}
			
		}
	}
	UpdateMatchInfo();
	UpdateMainUI();

}
void SpeakerWindow::SelectTeam2(wstring str)
{
	ClearT2UIInfo();
	if (!pStateManger->MatchValue["teams"].isNull())
	{
		int count = pStateManger->MatchValue["teams"].size();
		for (int i = 0; i < count; i++)
		{
			wstring teamname = String2WString(string(U2G(pStateManger->MatchValue["teams"][i]["T"][0].asCString())));
			if (teamname == str)
			{
				AddT2(teamname);
				int Ccount = pStateManger->MatchValue["teams"][i]["C"].size();
				for (int j = 0; j < Ccount; j++)
				{
					wstring cname = String2WString(string(U2G(pStateManger->MatchValue["teams"][i]["C"][j].asCString())));
					AddC2(cname);
				}
				int Pcount = pStateManger->MatchValue["teams"][i]["P"].size();
				for (int j = 0; j < Pcount; j++)
				{
					wstring cname = String2WString(string(U2G(pStateManger->MatchValue["teams"][i]["P"][j].asCString())));
					AddP2(cname);
				}
				break;
			}

		}
	}
	UpdateMatchInfo();
	UpdateMainUI();

}

void SpeakerWindow::UpdateMatchInfo()
{
	Plist.clear();
	P1list.clear();
	P2list.clear();
	Clist.clear();
	C1list.clear();
	C2list.clear();
	Tlist.clear();
	Rlist.clear();
	CTreeNodeUI *pControl = static_cast<CTreeNodeUI*>(m_PaintManager.FindControl(_T("wildcardP1")));
	int count = pControl->GetCountChild();
	int j = 0;
	for (int i = 0; i < count; i++)
	{
		CDuiString userdata = pControl->GetChildNode(i)->GetUserData();

		if (userdata == L"P1")
		{
			CDuiString  text = pControl->GetChildNode(i)->GetItemAt(1)->GetText();
			if (text == L"")
			{
				pControl->RemoveAt(pControl->GetChildNode(i));
				i--;
				count--;
			}
			else
			{
				Plist.push_back(wstring(text));
				P1list.push_back(wstring(text));
				j++;
			}
			
		}
	}
	pControl = static_cast<CTreeNodeUI*>(m_PaintManager.FindControl(_T("wildcardC1")));
	count = pControl->GetCountChild();
	j = 0;
	for (int i = 0; i < count; i++)
	{
		CDuiString userdata = pControl->GetChildNode(i)->GetUserData();

		if (userdata == L"C1")
		{
			CDuiString  text = pControl->GetChildNode(i)->GetItemAt(1)->GetText();
			if (text == L"")
			{
				//pControl->RemoveAt(pControl->GetChildNode(i));
				//i--;
				//count--;
			}
			else
			{
				Clist.push_back(wstring(text));
				C1list.push_back(wstring(text));
				j++;
			}

		}
	}
	pControl = static_cast<CTreeNodeUI*>(m_PaintManager.FindControl(_T("wildcardT1")));
	count = pControl->GetCountChild();
	j = 0;
	for (int i = 0; i < count; i++)
	{
		CDuiString userdata = pControl->GetChildNode(i)->GetUserData();

		if (userdata == L"T1")
		{
			CDuiString  text = pControl->GetChildNode(i)->GetItemAt(1)->GetText();
			if (text == L"")
			{
				//pControl->RemoveAt(pControl->GetChildNode(i));
				//i--;
				//count--;
			}
			else
			{
				Tlist.push_back(wstring(text));
				j++;
			}

		}
	}
	pControl = static_cast<CTreeNodeUI*>(m_PaintManager.FindControl(_T("wildcardP2")));
	count = pControl->GetCountChild();
	j = 0;
	for (int i = 0; i < count; i++)
	{
		CDuiString userdata = pControl->GetChildNode(i)->GetUserData();

		if (userdata == L"P2")
		{
			CDuiString  text = pControl->GetChildNode(i)->GetItemAt(1)->GetText();
			if (text == L"")
			{
				pControl->RemoveAt(pControl->GetChildNode(i));
				i--;
				count--;
			}
			else
			{
				Plist.push_back(wstring(text));
				P2list.push_back(wstring(text));
				j++;
			}

		}
	}
	pControl = static_cast<CTreeNodeUI*>(m_PaintManager.FindControl(_T("wildcardC2")));
	count = pControl->GetCountChild();
	j = 0;
	for (int i = 0; i < count; i++)
	{
		CDuiString userdata = pControl->GetChildNode(i)->GetUserData();

		if (userdata == L"C2")
		{
			CDuiString  text = pControl->GetChildNode(i)->GetItemAt(1)->GetText();
			if (text == L"")
			{
				//pControl->RemoveAt(pControl->GetChildNode(i));
				//i--;
				//count--;
			}
			else
			{
				Clist.push_back(wstring(text));
				C2list.push_back(wstring(text));
				j++;
			}

		}
	}
	pControl = static_cast<CTreeNodeUI*>(m_PaintManager.FindControl(_T("wildcardT2")));
	count = pControl->GetCountChild();
	j = 0;
	for (int i = 0; i < count; i++)
	{
		CDuiString userdata = pControl->GetChildNode(i)->GetUserData();

		if (userdata == L"T2")
		{
			CDuiString  text = pControl->GetChildNode(i)->GetItemAt(1)->GetText();
			if (text == L"")
			{
				//pControl->RemoveAt(pControl->GetChildNode(i));
				//i--;
				//count--;
			}
			else
			{
				Tlist.push_back(wstring(text));
				j++;
			}
		}
	}
	pControl = static_cast<CTreeNodeUI*>(m_PaintManager.FindControl(_T("wildcardR1")));
	count = pControl->GetCountChild();
	j = 0;
	for (int i = 0; i < count; i++)
	{
		CDuiString userdata = pControl->GetChildNode(i)->GetUserData();

		if (userdata == L"R1")
		{
			CDuiString  text = pControl->GetChildNode(i)->GetItemAt(1)->GetText();
			if (text == L"")
			{
				//pControl->RemoveAt(pControl->GetChildNode(i));
				//i--;
				//count--;
			}
			else
			{
				Rlist.push_back(wstring(text));
				pStateManger->MatchValue["R1"][j] = G2U(WString2String(wstring(text)).c_str());
				j++;
			}
		}
	}

	pControl = static_cast<CTreeNodeUI*>(m_PaintManager.FindControl(_T("wildcardR2")));
	count = pControl->GetCountChild();
	j = 0;
	for (int i = 0; i < count; i++)
	{
		CDuiString userdata = pControl->GetChildNode(i)->GetUserData();

		if (userdata == L"R2")
		{
			CDuiString  text = pControl->GetChildNode(i)->GetItemAt(1)->GetText();
			if (text == L"")
			{
				//pControl->RemoveAt(pControl->GetChildNode(i));
				//i--;
				//count--;
			}
			else
			{
				Rlist.push_back(wstring(text));
				pStateManger->MatchValue["R2"][j] = G2U(WString2String(wstring(text)).c_str());
				j++;
			}
		}
	}

	pControl = static_cast<CTreeNodeUI*>(m_PaintManager.FindControl(_T("wildcardR3")));
	count = pControl->GetCountChild();
	j = 0;
	for (int i = 0; i < count; i++)
	{
		CDuiString userdata = pControl->GetChildNode(i)->GetUserData();

		if (userdata == L"R3")
		{
			CDuiString  text = pControl->GetChildNode(i)->GetItemAt(1)->GetText();
			if (text == L"")
			{
				//pControl->RemoveAt(pControl->GetChildNode(i));
				//i--;
				//count--;
			}
			else
			{
				Rlist.push_back(wstring(text));
				pStateManger->MatchValue["R3"][j] = G2U(WString2String(wstring(text)).c_str());
				j++;
			}
		}
	}


	CDuiString matchinfo;
	CComboUI *pComboControl = static_cast<CComboUI*>(m_PaintManager.FindControl(_T("team1combo")));
	matchinfo.Append(pComboControl->GetText());
	matchinfo.Append(L" VS ");
	pComboControl = static_cast<CComboUI*>(m_PaintManager.FindControl(_T("team2combo")));
	matchinfo.Append(pComboControl->GetText());
	pStateManger->SetMatchInfo(wstring(matchinfo));
}

void SpeakerWindow::UpdateMsgInfo()
{
	EMmap.clear();
	CTreeNodeUI *pControl = static_cast<CTreeNodeUI*>(m_PaintManager.FindControl(_T("eventnode")));
	int count = pControl->GetCountChild();
	for (int i = 0; i < count; i++)
	{
		CTreeNodeUI *pEventChildControl = pControl->GetChildNode(i);
		CDuiString userdata = pEventChildControl->GetUserData();
		if (userdata != L"addmatchevent") //非按钮
		{
			CHorizontalLayoutUI *pLayOut = static_cast<CHorizontalLayoutUI*>(pControl->GetChildNode(i)->GetItemAt(1));
			int nc = pLayOut->GetCount();
			CDuiString  text = pLayOut->GetItemAt(0)->GetText();
			wstring key = wstring(pLayOut->GetItemAt(1)->GetText());
			if (text == L"")
			{
				pControl->RemoveAt(pControl->GetChildNode(i));
				i--;
				count--;
				FreeKey(key);
				//wstring key = wstring(pLayOut->GetItemAt(1)->GetText());
				//EMmap.erase(key);
			}
			else
			{
				//此类型事件
				
				map<wstring, EventMsg>::iterator item = EMmap.find(wstring(key));
				if (item == EMmap.end())
				{
					EventMsg em;
					em.EventName = wstring(text);
					EMmap.insert(pair<wstring, EventMsg>(wstring(key), em));
				}
				int nCount = pEventChildControl->GetCountChild();//此类事件的消息个数
				for (int j = 0; j < nCount; j++)
				{
					CTreeNodeUI * pMsgChildControl = pEventChildControl->GetChildNode(j); //消息体
					userdata = pMsgChildControl->GetUserData();
					int nc = pMsgChildControl->GetCount();
					if (userdata != L"addmsg")//非按钮
					{
						CDuiString  msgtext = pMsgChildControl->GetItemAt(1)->GetText();
						if (msgtext == L"")
						{
							pEventChildControl->RemoveAt(pMsgChildControl);
							j--;
							nCount--;
						}
						else
						{
							//有输入
							map<wstring, EventMsg>::iterator item = EMmap.find(wstring(key));
							if (item != EMmap.end())
							{
								item->second.EventName = wstring(text);
								item->second.MsgList.push_back(wstring(msgtext));
							}
							else
							{
								EventMsg em;
								em.EventName = wstring(text);
								em.MsgList.push_back(wstring(msgtext));
								EMmap.insert(pair<wstring, EventMsg>(wstring(key), em));
							}
							
						}
						
					}
				}
			}
		}
	}

	//EMmap信息更新到JSON对象中
	pStateManger->SetEventMap(EMmap);
	pStateManger->SaveCasterSetting();
}

void SpeakerWindow::UpdateMainUI()
{
	CTextUI* pControl = static_cast<CTextUI*>(m_PaintManager.FindControl(_T("ipaddr")));
	wstring ip = pStateManger->GetLocalIP();
	pControl->SetText(ip.c_str());
	//对阵信息
	pControl = static_cast<CTextUI*>(m_PaintManager.FindControl(_T("matchinfo")));
	pControl->SetText(pStateManger->GetMatchInfo().c_str());
	//昵称信息
	CTextUI *pNickNameControl = static_cast<CTextUI*>(m_PaintManager.FindControl(_T("nickname")));
	if (pNickNameControl)
	{
		pNickNameControl->SetText(pStateManger->GetSpeakerNickName().c_str());

	}
	///事件信息
	CTreeNodeUI *pMainEventControl = static_cast<CTreeNodeUI*>(m_PaintManager.FindControl(_T("eventtreenode")));
	while (pMainEventControl->IsHasChild())
	{
		pMainEventControl->RemoveAt(pMainEventControl->GetChildNode(0));
	}
	map<wstring, EventMsg >::reverse_iterator item = EMmap.rbegin();
	for (; item != EMmap.rend(); item++)
	{
		CTreeNodeUI *EditNode = new CTreeNodeUI();
		EditNode->SetAttribute(L"height", L"25");
		EditNode->SetAttribute(L"width", L"0");
		EditNode->SetAttribute(L"bordersize", L"1");
		EditNode->SetAttribute(L"bordercolor", L"#FFE5E5E5");

		CDuiString str;
		str.Append(L"(CTRL+");
		str.Append(item->first.c_str());
		str.Append(L")");
		str.Append(L"    ");
		str.Append(item->second.EventName.c_str());
		EditNode->SetItemText(str);
		pMainEventControl->AddAt(EditNode,0);
		
	}
}

void SpeakerWindow::UpdateSettingUI()
{
	CComboUI *pComboControl = static_cast<CComboUI*>(m_PaintManager.FindControl(_T("nfontsize")));
	pComboControl->SetInternVisible();
	int lnfont = atoi(pStateManger->SettingValue["SNfont"].asCString());
	pComboControl->SelectItem(lnfont - 5);
	pComboControl = static_cast<CComboUI*>(m_PaintManager.FindControl(_T("ofontsize")));
	pComboControl->SetInternVisible();
	int lofont = atoi(pStateManger->SettingValue["SOfont"].asCString());
	pComboControl->SelectItem(lofont-1);

	pComboControl = static_cast<CComboUI*>(m_PaintManager.FindControl(_T("team1combo")));
	pComboControl->SelectItem(pStateManger->GetTeam1());
	pComboControl = static_cast<CComboUI*>(m_PaintManager.FindControl(_T("team2combo")));
	pComboControl->SelectItem(pStateManger->GetTeam2());
}

void SpeakerWindow::ClearMatchUIInfo()
{

	CComboUI *pComboControl = static_cast<CComboUI*>(m_PaintManager.FindControl(_T("team1combo")));
	pComboControl->RemoveAll();
	pComboControl = static_cast<CComboUI*>(m_PaintManager.FindControl(_T("team2combo")));
	pComboControl->RemoveAll();
	CTreeNodeUI *pControl = static_cast<CTreeNodeUI*>(m_PaintManager.FindControl(_T("wildcardP1")));
	int count = pControl->GetCountChild();
	for (int i = 0; i < count; i++)
	{
		CDuiString userdata = pControl->GetChildNode(i)->GetUserData();
		if (userdata == L"P1")
		{
			pControl->RemoveAt(pControl->GetChildNode(i));
			i--;
			count--;
		}
	}
	pControl = static_cast<CTreeNodeUI*>(m_PaintManager.FindControl(_T("wildcardP2")));
	count = pControl->GetCountChild();
	for (int i = 0; i < count; i++)
	{
		CDuiString userdata = pControl->GetChildNode(i)->GetUserData();
		if (userdata == L"P2")
		{
			pControl->RemoveAt(pControl->GetChildNode(i));
			i--;
			count--;
		}
	}
	pControl = static_cast<CTreeNodeUI*>(m_PaintManager.FindControl(_T("wildcardT1")));
	count = pControl->GetCountChild();
	for (int i = 0; i < count; i++)
	{
		CDuiString userdata = pControl->GetChildNode(i)->GetUserData();
		if (userdata == L"T1")
		{
			pControl->RemoveAt(pControl->GetChildNode(i));
			i--;
			count--;
		}
	}
	pControl = static_cast<CTreeNodeUI*>(m_PaintManager.FindControl(_T("wildcardT2")));
	count = pControl->GetCountChild();
	for (int i = 0; i < count; i++)
	{
		CDuiString userdata = pControl->GetChildNode(i)->GetUserData();
		if (userdata == L"T2")
		{
			pControl->RemoveAt(pControl->GetChildNode(i));
			i--;
			count--;
		}
	}
	pControl = static_cast<CTreeNodeUI*>(m_PaintManager.FindControl(_T("wildcardC1")));
	count = pControl->GetCountChild();
	for (int i = 0; i < count; i++)
	{
		CDuiString userdata = pControl->GetChildNode(i)->GetUserData();
		if (userdata == L"C1")
		{
			pControl->RemoveAt(pControl->GetChildNode(i));
			i--;
			count--;
		}
	}
	pControl = static_cast<CTreeNodeUI*>(m_PaintManager.FindControl(_T("wildcardC2")));
	count = pControl->GetCountChild();
	for (int i = 0; i < count; i++)
	{
		CDuiString userdata = pControl->GetChildNode(i)->GetUserData();
		if (userdata == L"C2")
		{
			pControl->RemoveAt(pControl->GetChildNode(i));
			i--;
			count--;
		}
	}
	pControl = static_cast<CTreeNodeUI*>(m_PaintManager.FindControl(_T("wildcardR1")));
	count = pControl->GetCountChild();
	for (int i = 0; i < count; i++)
	{
		CDuiString userdata = pControl->GetChildNode(i)->GetUserData();
		if (userdata == L"R1")
		{
			pControl->RemoveAt(pControl->GetChildNode(i));
			i--;
			count--;
		}
	}
	pControl = static_cast<CTreeNodeUI*>(m_PaintManager.FindControl(_T("wildcardR2")));
	count = pControl->GetCountChild();
	for (int i = 0; i < count; i++)
	{
		CDuiString userdata = pControl->GetChildNode(i)->GetUserData();
		if (userdata == L"R2")
		{
			pControl->RemoveAt(pControl->GetChildNode(i));
			i--;
			count--;
		}
	}
	pControl = static_cast<CTreeNodeUI*>(m_PaintManager.FindControl(_T("wildcardR3")));
	count = pControl->GetCountChild();
	for (int i = 0; i < count; i++)
	{
		CDuiString userdata = pControl->GetChildNode(i)->GetUserData();
		if (userdata == L"R3")
		{
			pControl->RemoveAt(pControl->GetChildNode(i));
			i--;
			count--;
		}
	}
}

void SpeakerWindow::ClearT1UIInfo()
{
	CTreeNodeUI *pControl = static_cast<CTreeNodeUI*>(m_PaintManager.FindControl(_T("wildcardP1")));
	int count = pControl->GetCountChild();
	for (int i = 0; i < count; i++)
	{
		CDuiString userdata = pControl->GetChildNode(i)->GetUserData();
		if (userdata == L"P1")
		{
			pControl->RemoveAt(pControl->GetChildNode(i));
			i--;
			count--;
		}
	}
	pControl = static_cast<CTreeNodeUI*>(m_PaintManager.FindControl(_T("wildcardT1")));
	count = pControl->GetCountChild();
	for (int i = 0; i < count; i++)
	{
		CDuiString userdata = pControl->GetChildNode(i)->GetUserData();
		if (userdata == L"T1")
		{
			pControl->RemoveAt(pControl->GetChildNode(i));
			i--;
			count--;
		}
	}
	pControl = static_cast<CTreeNodeUI*>(m_PaintManager.FindControl(_T("wildcardC1")));
	count = pControl->GetCountChild();
	for (int i = 0; i < count; i++)
	{
		CDuiString userdata = pControl->GetChildNode(i)->GetUserData();
		if (userdata == L"C1")
		{
			pControl->RemoveAt(pControl->GetChildNode(i));
			i--;
			count--;
		}
	}
}
void SpeakerWindow::ClearT2UIInfo()
{
	CTreeNodeUI *pControl = static_cast<CTreeNodeUI*>(m_PaintManager.FindControl(_T("wildcardP2")));
	int count = pControl->GetCountChild();
	for (int i = 0; i < count; i++)
	{
		CDuiString userdata = pControl->GetChildNode(i)->GetUserData();
		if (userdata == L"P2")
		{
			pControl->RemoveAt(pControl->GetChildNode(i));
			i--;
			count--;
		}
	}
	pControl = static_cast<CTreeNodeUI*>(m_PaintManager.FindControl(_T("wildcardT2")));
	count = pControl->GetCountChild();
	for (int i = 0; i < count; i++)
	{
		CDuiString userdata = pControl->GetChildNode(i)->GetUserData();
		if (userdata == L"T2")
		{
			pControl->RemoveAt(pControl->GetChildNode(i));
			i--;
			count--;
		}
	}
	pControl = static_cast<CTreeNodeUI*>(m_PaintManager.FindControl(_T("wildcardC2")));
	count = pControl->GetCountChild();
	for (int i = 0; i < count; i++)
	{
		CDuiString userdata = pControl->GetChildNode(i)->GetUserData();
		if (userdata == L"C2")
		{
			pControl->RemoveAt(pControl->GetChildNode(i));
			i--;
			count--;
		}
	}
}
#include"Commdlg.h"
bool SpeakerWindow::ImportMatchInfo(wstring path)
{
	if (path == L"")
	{
		OPENFILENAME ofn;
		TCHAR szFile[MAX_PATH] = _T("");

		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = *this;
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = _T("Match文件(*.match)\0*.match\0所有文件(*.*)\0*.*\0");;
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = L"导入";
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = L".\\";
		ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

		if (GetOpenFileName(&ofn))
		{
			//std::vector<string_t> vctString(1, szFile);
			pStateManger->ReadMatchInfo(WString2String(wstring(szFile)));
			//更新比赛信息
			ClearMatchUIInfo();
			if (!pStateManger->MatchValue["R1"].isNull())
			{
				int count = pStateManger->MatchValue["R1"].size();
				for (int i = 0; i < count; i++)
				{
					wstring str = String2WString(string(U2G(pStateManger->MatchValue["R1"][i].asCString())));
					AddR1(str);
				}
			}
			if (!pStateManger->MatchValue["R2"].isNull())
			{
				int count = pStateManger->MatchValue["R2"].size();
				for (int i = 0; i < count; i++)
				{
					wstring str = String2WString(string(U2G(pStateManger->MatchValue["R2"][i].asCString())));
					AddR2(str);
				}
			}
			if (!pStateManger->MatchValue["R3"].isNull())
			{
				int count = pStateManger->MatchValue["R3"].size();
				for (int i = 0; i < count; i++)
				{
					wstring str = String2WString(string(U2G(pStateManger->MatchValue["R3"][i].asCString())));
					AddR3(str);
				}
			}
			if (!pStateManger->MatchValue["teams"].isNull())
			{
				int count = pStateManger->MatchValue["teams"].size();
				if (count < 2)
				{
					return false;
				}
				for (int i = 0; i < count; i++)
				{
					AddTeam(String2WString(string(U2G(pStateManger->MatchValue["teams"][i]["T"][0].asCString()))));
				}
				CComboUI *pComboControl = static_cast<CComboUI*>(m_PaintManager.FindControl(_T("team1combo")));
				pComboControl->SelectItem(0);
				pComboControl = static_cast<CComboUI*>(m_PaintManager.FindControl(_T("team2combo")));
				pComboControl->SelectItem(1);

			}

			UpdateMatchInfo();
			UpdateMainUI();
		}
	}
	else
	{
		//std::vector<string_t> vctString(1, szFile);
		pStateManger->ReadMatchInfo(WString2String(path));
		//更新比赛信息
		ClearMatchUIInfo();
		if (!pStateManger->MatchValue["R1"].isNull())
		{
			int count = pStateManger->MatchValue["R1"].size();
			for (int i = 0; i < count; i++)
			{
				wstring str = String2WString(string(U2G(pStateManger->MatchValue["R1"][i].asCString())));
				AddR1(str);
			}
		}
		if (!pStateManger->MatchValue["R2"].isNull())
		{
			int count = pStateManger->MatchValue["R2"].size();
			for (int i = 0; i < count; i++)
			{
				wstring str = String2WString(string(U2G(pStateManger->MatchValue["R2"][i].asCString())));
				AddR2(str);
			}
		}
		if (!pStateManger->MatchValue["R3"].isNull())
		{
			int count = pStateManger->MatchValue["R3"].size();
			for (int i = 0; i < count; i++)
			{
				wstring str = String2WString(string(U2G(pStateManger->MatchValue["R3"][i].asCString())));
				AddR3(str);
			}
		}
		if (!pStateManger->MatchValue["teams"].isNull())
		{
			int count = pStateManger->MatchValue["teams"].size();
			if (count < 2)
			{
				return false;
			}
			for (int i = 0; i < count; i++)
			{
				AddTeam(String2WString(string(U2G(pStateManger->MatchValue["teams"][i]["T"][0].asCString()))));
			}
			CComboUI *pComboControl = static_cast<CComboUI*>(m_PaintManager.FindControl(_T("team1combo")));
			pComboControl->SetInternVisible();
			//pComboControl->SelectItem(0);
			pComboControl = static_cast<CComboUI*>(m_PaintManager.FindControl(_T("team2combo")));
			pComboControl->SetInternVisible();
			//pComboControl->SelectItem(1);

		}

		UpdateMatchInfo();
		UpdateMainUI();
	}
	return true;
}
void SpeakerWindow::SaveMatchInfo()
{
	OPENFILENAME ofn;
	TCHAR szFile[MAX_PATH] = _T("");

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = *this;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = _T("Match文件(*.match)\0*.match\0所有文件(*.*)\0*.*\0");;
	ofn.nFilterIndex = 1;
	ofn.lpstrDefExt = L".match";
	ofn.lpstrFileTitle = L"保存为";
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = L".\\";
	ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST ;

	if (GetOpenFileName(&ofn))
	{
		//std::vector<string_t> vctString(1, szFile);
		pStateManger->SaveMatchInfo(WString2String(wstring(szFile)));
	}
}