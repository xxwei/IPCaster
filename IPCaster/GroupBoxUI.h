#pragma once


#ifndef _UIGROUPBOX_H_
#define _UIGROUPBOX_H_
using namespace DuiLib;
#define  GROUPBOX_TEXT_OFFSET        40        //定义GroupBox中的Text相对于左边的偏移

const TCHAR kGroupBoxUIClassName[] = _T("GroupBoxUI");
const TCHAR kGroupBoxUIInterFace[] = _T("GroupBox");


class CGroupBoxUI :public CContainerUI
{
public:
	CGroupBoxUI();

	~CGroupBoxUI();

	virtual LPCTSTR GetClass() const;

	virtual LPVOID GetInterface(LPCTSTR pstrName);

	virtual void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

	void PaintText(HDC hDC);

	void PaintBorder(HDC hDC);

	void SetTextColor(DWORD dwTextColor);

	void SetFont(int iFont);

	void SetShowHtml(bool bShowHtml);

private:
	DWORD    m_dwTextColor;        ///字休颜色
	int        m_iFont;            ///字体号，大小
	bool    m_bShowHtml;        ///是否显示HTML代码
	int        m_iTextWidth;        ///Text文字宽
	int        m_iTextHeigh;        ///Text文字高
	int		m_nBorderSize = 1;
};

#endif//_UIGROUPBOX_H_

