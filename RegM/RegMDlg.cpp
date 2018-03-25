
// RegMDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "RegM.h"
#include "RegMDlg.h"
#include "afxdialogex.h"
#include"License.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)

END_MESSAGE_MAP()


// CRegMDlg 对话框



CRegMDlg::CRegMDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_REGM_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CRegMDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DATETIMEPICKER_USE, m_lasttime);
	DDX_Text(pDX, IDC_EDIT_USETIMIE, m_nusetime);
	DDV_MinMaxUInt(pDX, m_nusetime, 0, 9999);
	DDX_Text(pDX, IDC_EDIT_USENUM, m_usetimes);
	DDV_MinMaxUInt(pDX, m_usetimes, 0, 9999);
}

BEGIN_MESSAGE_MAP(CRegMDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_CALC, &CRegMDlg::OnBnClickedButtonCalc)
END_MESSAGE_MAP()


// CRegMDlg 消息处理程序

BOOL CRegMDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	SetDlgItemText(IDC_EDIT_USETIMIE, L"9999");
	SetDlgItemText(IDC_EDIT_USENUM, L"9999");
	
	// TODO: 在此添加额外的初始化代码
	m_pLicense = new License();
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CRegMDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CRegMDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CRegMDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CRegMDlg::OnBnClickedButtonCalc()
{

	// TODO: 在此添加控件通知处理程序代码
	if (UpdateData())
	{
		CString m_code;
		GetDlgItemText(IDC_EDIT_MCODE, m_code);
		CTime m_time;
		m_lasttime.GetTime(m_time);
		wchar_t str[128] = { 0 };
		wsprintf(str, L"%04d%04d%04d%02d%02d", m_nusetime, m_usetimes, m_time.GetYear(), m_time.GetMonth(), m_time.GetDay());
		string limitcode = m_pLicense->GetLimitCode(WString2String(wstring(str)));
		string regcode = m_pLicense->GetRegCode(WString2String(wstring(m_code.GetBuffer())));
		string totalcode = limitcode + regcode;
		//int len = limitcode.length();
		//limitcode = WString2String(wstring(str));
		//int m_ntime = atoi(limitcode.substr(0, 4).c_str());
		//int m_ntimes = atoi(limitcode.substr(4, 4).c_str());
		//int m_nyear = atoi(limitcode.substr(8, 4).c_str());
		//int m_nmonth = atoi(limitcode.substr(12, 2).c_str());
		//int m_nday = atoi(limitcode.substr(14, 2).c_str());

		SetDlgItemTextA(m_hWnd, IDC_EDIT_RCODE, totalcode.c_str());
	}
}
