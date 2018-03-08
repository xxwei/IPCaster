#pragma once
#include "StateManger.h"


using namespace DuiLib;





class ListenerWindow: public WindowImplBase,public MessageCallBack
{
public:
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
		return L"LW";
	}
	UINT GetClassStyle() const {
		return UI_CLASSSTYLE_FRAME | CS_DBLCLKS;
	}
	CDuiString GetSkinFile() {
		return L"lw.xml";
	}
	CDuiString GetSkinFolder()
	{
		return L"./";
	}


	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, bool& bHandled);
	void Notify(TNotifyUI& msg);
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);


	///// overlay MessageCallBack
	virtual void OnSpeakerOnLine(wstring ip, wstring name);
	virtual void OnSpeakerOffLine();
	virtual void OnRecvMessage(Message *msg);


	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) override;

	void ReSizeMsgaNode(int max_width, CListContainerElementUI *node);
	void ReSizeMaxMsgNode(int max_width, CListContainerElementUI *node);

private:
	StateManger		*pStateManger = NULL;
	wstring			m_newmsg = L"";
	void UpdateMainUI();
	void UpdateSettingUI();
	bool	m_bInit = false;
public:
	void SetStateManger(StateManger *pSm);
	int AddNewMessage(wstring str);
	void SaveSetting();
};

