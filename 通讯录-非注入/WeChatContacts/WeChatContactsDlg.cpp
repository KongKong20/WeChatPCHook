// WeChatContactsDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "WeChatContacts.h"
#include "WeChatContactsDlg.h"
#include "afxdialogex.h"
#include "tlhelp32.h"
#include "afxdialogex.h"
#include <string>
#include <vector>

#include <map>
using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CWeChatContactsDlg 对话框
CWeChatContactsDlg::CWeChatContactsDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_WECHATCONTACTS_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CWeChatContactsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_ContactList);
}

BEGIN_MESSAGE_MAP(CWeChatContactsDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_CONTACTS, &CWeChatContactsDlg::OnBnClickedButtonContacts)
END_MESSAGE_MAP()


// CWeChatContactsDlg 消息处理程序

BOOL CWeChatContactsDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	m_ContactList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_ContactList.InsertColumn(0, _T("编号"), LVCFMT_LEFT, 100);
	m_ContactList.InsertColumn(1, _T("微信ID"), LVCFMT_LEFT, 200); // 插入第2列的列名
	m_ContactList.InsertColumn(2, _T("微信号"), LVCFMT_LEFT, 200); // 插入第3列的列名
	m_ContactList.InsertColumn(3, _T("微信名"), LVCFMT_LEFT, 300); // 插入第4列的列名

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CWeChatContactsDlg::OnPaint()
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
HCURSOR CWeChatContactsDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

DWORD GetMemoryIntByAddress(HANDLE hProcess, DWORD address)
{
	DWORD value = 0;

	byte data[4] = {0};
	if (ReadProcessMemory(hProcess, (LPVOID)address, data, 4, 0))
	{
		value = data[0] & 0xFF;
		value |= ((data[1] << 8) & 0xFF00);
		value |= ((data[2] << 16) & 0xFF0000);
		value |= ((data[3] << 24) & 0xFF000000);
	}

	return value;
}

CString GetUnicodeInfoByAddress(HANDLE hProcess, DWORD address)
{
	CString value = L"";
	
	DWORD strAddress = GetMemoryIntByAddress(hProcess, address);
	DWORD strLen = GetMemoryIntByAddress(hProcess, address + 0x4);
	if (strLen > 500) return value;

	wchar_t cValue[500] = { 0 };
	memset(cValue, 0, sizeof(cValue) / sizeof(wchar_t));
	if (ReadProcessMemory(hProcess, (LPVOID)strAddress, cValue, (strLen +1)*2, 0))
	{
		value = CString(cValue);
	}

	return value;
}

void CWeChatContactsDlg::OnBnClickedButtonContacts()
{
	class WxString
	{
	public:
		wstring text;
		char fill[0x8];
		bool operator<(const WxString &other)const {
			return text < other.text;
		}
	};

	CString processName = L"WeChat.exe";
	CString moduleName = L"WeChatWin.dll";
	DWORD processID = 0;

	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 pe32 = { sizeof(PROCESSENTRY32) };
	while (Process32Next(hSnapshot, &pe32))
	{
		CString strProcess = pe32.szExeFile;
		if (strProcess == processName)
		{
			processID = pe32.th32ProcessID;
			break;
		}
	}
	CloseHandle(hSnapshot);
	if (processID == 0) return;

	DWORD moduleBaseAddress = 0;

	HANDLE hProcessSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processID);
	if (hProcessSnapshot == INVALID_HANDLE_VALUE) return;

	MODULEENTRY32 me32;
	SecureZeroMemory(&me32, sizeof(MODULEENTRY32));
	me32.dwSize = sizeof(MODULEENTRY32);

	while (Module32Next(hProcessSnapshot, &me32))
	{
		me32.dwSize = sizeof(MODULEENTRY32);

		if (!_tcscmp(me32.szModule, moduleName.GetBuffer()))
		{
			moduleBaseAddress = (DWORD)me32.modBaseAddr;
			break;
		}
	}

	CloseHandle(hProcessSnapshot);
	if (moduleBaseAddress == 0) return;

	HANDLE hProcess = OpenProcess(PROCESS_VM_READ, FALSE, processID);
	if (hProcess == 0) return;

	//[[[WeChatWin.dll + 0x1397A8C] + 0x24 + 0x68] + 0x4]
	DWORD Address1 = moduleBaseAddress + 0x1397A8C;
	DWORD Address2 = GetMemoryIntByAddress(hProcess, Address1);
	DWORD Address3 = GetMemoryIntByAddress(hProcess, Address2 + 0x24 + 0x68);

	vector<DWORD> nodeAddressList;
	nodeAddressList.push_back(Address3);

	DWORD nodeAddress1 = GetMemoryIntByAddress(hProcess, Address3 + 0x0);
	DWORD nodeAddress2 = GetMemoryIntByAddress(hProcess, Address3 + 0x4);
	DWORD nodeAddress3 = GetMemoryIntByAddress(hProcess, Address3 + 0x8);
	if (find(nodeAddressList.begin(), nodeAddressList.end(), nodeAddress1) == nodeAddressList.end())
		nodeAddressList.push_back(nodeAddress1);
	if (find(nodeAddressList.begin(), nodeAddressList.end(), nodeAddress2) == nodeAddressList.end())
		nodeAddressList.push_back(nodeAddress2);
	if (find(nodeAddressList.begin(), nodeAddressList.end(), nodeAddress3) == nodeAddressList.end())
		nodeAddressList.push_back(nodeAddress3);

	int index = 1;
	m_ContactList.DeleteAllItems();
	while(index < nodeAddressList.size())
	{
		DWORD nodeAddress = nodeAddressList[index++];
		DWORD checkNullResult = GetMemoryIntByAddress(hProcess, nodeAddress + 0xD);
		if (checkNullResult == 0) {
			index++;
			continue;
		}

		CString wxId = GetUnicodeInfoByAddress(hProcess, nodeAddress + 0x30);
		CString wxCode = GetUnicodeInfoByAddress(hProcess, nodeAddress + 0x44);
		CString wxName = GetUnicodeInfoByAddress(hProcess, nodeAddress + 0x8C);

		int listIndex = m_ContactList.GetItemCount();

		CString listIndexTemp;
		listIndexTemp.Format(L"%d", listIndex);
		m_ContactList.InsertItem(listIndex, listIndexTemp);
		m_ContactList.SetItemText(listIndex, 0, listIndexTemp);
		m_ContactList.SetItemText(listIndex, 1, wxId);
		m_ContactList.SetItemText(listIndex, 2, wxCode);
		m_ContactList.SetItemText(listIndex, 3, wxName);

		DWORD nodeAddress1 = GetMemoryIntByAddress(hProcess, nodeAddress + 0x0);
		DWORD nodeAddress2 = GetMemoryIntByAddress(hProcess, nodeAddress + 0x4);
		DWORD nodeAddress3 = GetMemoryIntByAddress(hProcess, nodeAddress + 0x8);
		if (find(nodeAddressList.begin(), nodeAddressList.end(), nodeAddress1) == nodeAddressList.end())
			nodeAddressList.push_back(nodeAddress1);
		if (find(nodeAddressList.begin(), nodeAddressList.end(), nodeAddress2) == nodeAddressList.end())
			nodeAddressList.push_back(nodeAddress2);
		if (find(nodeAddressList.begin(), nodeAddressList.end(), nodeAddress3) == nodeAddressList.end())
			nodeAddressList.push_back(nodeAddress3);
	}

	CloseHandle(hProcess);
}
