
// MFCGetUserInfoDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "MFCGetUserInfo.h"
#include "MFCGetUserInfoDlg.h"
#include "afxdialogex.h"

#include <TlHelp32.h>

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


// CMFCGetUserInfoDlg 对话框



CMFCGetUserInfoDlg::CMFCGetUserInfoDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MFCGETUSERINFO_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMFCGetUserInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CMFCGetUserInfoDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_GET_USER_INFO, &CMFCGetUserInfoDlg::OnBnClickedButtonGetUserInfo)
END_MESSAGE_MAP()


// CMFCGetUserInfoDlg 消息处理程序

BOOL CMFCGetUserInfoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
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

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CMFCGetUserInfoDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CMFCGetUserInfoDlg::OnPaint()
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
HCURSOR CMFCGetUserInfoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

DWORD FindProgressPidByName(const char* progressName)
{
	DWORD processID = 0;

	PROCESSENTRY32 pe32 = { 0 };
	pe32.dwSize = sizeof(PROCESSENTRY32);

	// 获取所有进程的信息
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	// 拿到第一个进程的信息
	if (Process32First(hSnapshot, &pe32) == TRUE)
	{
		do
		{
			USES_CONVERSION;
			// 进程名字是progressName就返回
			if (strcmp(progressName, W2A(pe32.szExeFile)) == 0)
			{
				processID = pe32.th32ProcessID;
				break;
			}
			// 进程名字不是progressName，获取下一个进程信息
		} while (Process32Next(hSnapshot, &pe32));
	}

	CloseHandle(hSnapshot);

	return processID;
}

DWORD GetDLLBaseAddress(DWORD processID, const wchar_t* moduleName)
{
	DWORD moduleBaseAddress = 0;
	
	// 获取进程ID processID 对应的进程信息
	HANDLE hProcessSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processID);
	if (hProcessSnapshot == INVALID_HANDLE_VALUE) return moduleBaseAddress;

	MODULEENTRY32 me32;
	SecureZeroMemory(&me32, sizeof(MODULEENTRY32));
	me32.dwSize = sizeof(MODULEENTRY32);
	
	// 遍历进程的模块信息
	while (Module32Next(hProcessSnapshot, &me32))
	{
		me32.dwSize = sizeof(MODULEENTRY32);
		
		// 判断是不是目标模块moduleName
		if (!_tcscmp(me32.szModule, moduleName))
		{
			moduleBaseAddress = (DWORD)me32.modBaseAddr;
			break;
		}
	}

	CloseHandle(hProcessSnapshot);

	return moduleBaseAddress;
}

DWORD GetIntByAddress(HANDLE hProcess, DWORD address)
{
	DWORD intValue = 0;

	ReadProcessMemory(hProcess, (LPVOID)address, &intValue, 4, 0);

	return intValue;
}

CString GetCStringByAddress(HANDLE hProcess, DWORD address)
{
	CString csValue = L"";

	char cValue[500] = { 0 };
	if (ReadProcessMemory(hProcess, (LPVOID)address, cValue, 500, 0))
	{
		csValue = CString(cValue);
	}

	return csValue;
}

void CMFCGetUserInfoDlg::OnBnClickedButtonGetUserInfo()
{
	// 1、获取DLL模块基址

	// 进程IP
	DWORD processID = FindProgressPidByName("WeChat.exe");
	// 进程句柄
	HANDLE hProcess = OpenProcess(PROCESS_VM_READ, FALSE, processID);
	// 模块基址
	DWORD dllBaseAddress = GetDLLBaseAddress(processID, L"WeChatWin.dll");

	// 2、获取用户名
	
	// 用户名偏移
	DWORD wxNameOffset = 0x16B498C;
	// 用户名内存地址
	DWORD wxNameAddress = dllBaseAddress + wxNameOffset;
	// 用户名
	CString csWxName = GetCStringByAddress(hProcess, wxNameAddress);

	// 3、获取WXID

	// WXID指针偏移
	DWORD wxIDPointerOffset = 0x16B4D90;
	// WXID指针地址
	DWORD wxIDPointerAddress = dllBaseAddress + wxIDPointerOffset;
	// WXID内存地址
	DWORD wxIDAddress = GetIntByAddress(hProcess, wxIDPointerAddress);
	// WXID
	CString csWxId = GetCStringByAddress(hProcess, wxIDAddress);

	// 4、显示结果

	// 用户信息
	CString csUserInfo = L"用户信息： 用户名：" + csWxName + L" 用户ID: " + csWxId;
	GetDlgItem(IDC_STATIC_USER_INFO)->SetWindowText(csUserInfo);
}
