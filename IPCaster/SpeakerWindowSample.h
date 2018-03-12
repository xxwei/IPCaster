#pragma once

#include "StateManger.h"
using namespace DuiLib;



class SpeakerWindowSample :public WindowImplBase, public MessageCallBack
{
public:
	CControlUI* CreateControl(LPCTSTR pstrClassName);
	virtual void  InitWindow();
	virtual LPCTSTR GetResourceID() const
	{
		return MAKEINTRESOURCE(IDR_ZIP_SKIN);
	};
	virtual UILIB_RESOURCETYPE GetResourceType() const
	{
		//return UILIB_FILE;
		return UILIB_ZIPRESOURCE;
	};
	LPCTSTR GetWindowClassName() const
	{
		return L"SWS";
	}
	UINT GetClassStyle() const {
		return UI_CLASSSTYLE_FRAME | CS_DBLCLKS;
	}
	CDuiString GetSkinFile() {
		return L"sws.xml";
	}
	CDuiString GetSkinFolder()
	{
		return L"./";
	}

	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, bool& bHandled);

	LRESULT HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) override;

	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) override;

	void Notify(TNotifyUI& msg);

	void OnListenerOnLine(usermap map);
private:

	CDuiString		m_CurrentReplaceStr;
	int				m_CurrentFindPos = 0;
	CDuiString		m_CurrentMsgStr;
	StateManger		*pStateManger = NULL;
	//通配符P
	list<wstring>		Plist;
	list<wstring>		P1list;
	list<wstring>		P2list;
	//通配符C
	list<wstring>		Clist;
	list<wstring>		C1list;
	list<wstring>		C2list;
	//通配符T
	list<wstring>		Tlist;
	//通配符R
	list<wstring>		Rlist;

	//通配符类别
	list<wstring>		Ulist;
	
	//消息列表
	EventMap			EMmap;

	map<wstring, bool>		CacheKeyMap;

	mutex				SelectMutex;

	int					m_CurrentTeam = 0;
	int					m_CurrentCanShotKey = true;
	bool				m_bStartSelectTeam = false;
	bool				m_bInit = false;

	int					m_lastwidth = 0;
	int					m_lastheight = 0;
	bool				m_bAuto = false;
	CDuiString				m_CurrentTypeMsg = L"";

	
public:

	void SelectItem();
	void SetStateManger(StateManger *pSm);
	int AddNewMessage(wstring str);
	int SendNewMessage(wstring str);


	wstring GetFreeKey(wstring key=L"");
	void FreeKey(wstring key);

	int AddR1(wstring str);
	int AddR2(wstring str);
	int AddR3(wstring str);

	int AddP1(wstring str,bool bfocus = false);
	int AddC1(wstring str);
	int AddT1(wstring str);
	int AddP2(wstring str,bool bfocus = false);
	int AddC2(wstring str);
	int AddT2(wstring str);

	int AddTeam(wstring str);
	void SelectTeam1(wstring str);
	void SelectTeam2(wstring str);

	int AddEvent(wstring str, bool bfocus = false);
	int AddMsg(wstring str,wstring type, bool bfocus = false);


	void UpdateMatchInfo();
	void UpdateMsgInfo();
	void UpdateMainUI();
	void UpdateSettingUI();


	void UpdateFlowItem(int step);

	void SaveMatchInfo();
	bool ImportMatchInfo(wstring path=L"");
	void ClearMatchUIInfo();
	void ClearT1UIInfo();
	void ClearT2UIInfo();

	CDuiString ReplaceOneItem(CDuiString Dst, int Pos, int len, CDuiString src);

	void ReSizeItem(int max_width,int max_height);
	void ClickItem(CDuiString ItemName,bool bOver = false);


	
};