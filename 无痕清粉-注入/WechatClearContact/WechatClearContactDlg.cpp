
// WechatClearContactDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "WechatClearContact.h"
#include "WechatClearContactDlg.h"
#include "afxdialogex.h"
#include "Inject.h"
#include "Cmd.h"
#include <string>
#include <vector>
#include <ctime>
using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CWechatClearContactDlg 对话框

int RandInt(int min, int max)
{
	if (min >= max) return 20;

	srand((unsigned)time(NULL));
	int r = rand() % (max - min + 1) + min;

	return r;
}

CWechatClearContactDlg::CWechatClearContactDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_WECHATCLEARCONTACT_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CWechatClearContactDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_CONTACTS, m_ContactList);
	DDX_Control(pDX, IDC_PROGRESS_VERIFY, m_progressVerify);
	DDX_Control(pDX, IDC_PROGRESS_CLEAR, m_progressClear);
	DDX_Control(pDX, IDC_EDIT_VERIFY_MIN, m_editVerifyMin);
	DDX_Control(pDX, IDC_EDIT_VERIFY_MAX, m_editVerifyMax);
	DDX_Control(pDX, IDC_EDIT_CLEAR_MIN, m_editClearMin);
	DDX_Control(pDX, IDC_EDIT_CLEAR_MAX, m_editClearMax);
}

BEGIN_MESSAGE_MAP(CWechatClearContactDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_WECHAT, &CWechatClearContactDlg::OnBnClickedButtonWechat)
	ON_WM_COPYDATA()
	ON_BN_CLICKED(IDC_BUTTON_ALL_VERIFY, &CWechatClearContactDlg::OnBnClickedButtonAllVerify)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR, &CWechatClearContactDlg::OnBnClickedButtonClear)
	ON_WM_HSCROLL()
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CWechatClearContactDlg 消息处理程序

BOOL CWechatClearContactDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标


	m_ContactList.InsertColumn(0, _T("序号"), LVCFMT_LEFT, 50, 3);
	m_ContactList.InsertColumn(1, _T("微信ID"), LVCFMT_CENTER, 180, 0);
	m_ContactList.InsertColumn(2, _T("微信号"), LVCFMT_CENTER, 180, 0);
	m_ContactList.InsertColumn(3, _T("检查结果"), LVCFMT_CENTER, 100, 1);
	m_ContactList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_SUBITEMIMAGES);

	m_progressVerify.SetRange(0, 1);
	m_progressVerify.SetStep(1);
	m_progressVerify.SetPos(0);

	m_progressClear.SetRange(0, 1);
	m_progressClear.SetStep(1);
	m_progressVerify.SetPos(0);

	m_editVerifyMin.SetWindowText(L"10");
	m_editVerifyMax.SetWindowText(L"30");

	m_editClearMin.SetWindowText(L"10");
	m_editClearMax.SetWindowText(L"30");

	UpdateProgress();

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CWechatClearContactDlg::OnPaint()
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
HCURSOR CWechatClearContactDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

bool isLoad = false;
DWORD pid = 0;
void CWechatClearContactDlg::OnBnClickedButtonWechat()
{
	isLoad = !isLoad;
	if (isLoad)
	{
		pid = FindWeChatPid();
		if (pid == 0)
		{
			MessageBox(L"请先登陆微信");
			isLoad = !isLoad;

			return;

		}
		LoadDll(pid);

		GetDlgItem(IDC_BUTTON_WECHAT)->SetWindowText(L"脱离微信");
	}
	else
	{
		UnloadDll(pid);

		GetDlgItem(IDC_BUTTON_WECHAT)->SetWindowText(L"附加微信");
	}
}

vector<ContactInfoStruct> contactList;

BOOL CWechatClearContactDlg::OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct)
{
	int messageType = pCopyDataStruct->dwData;
	switch (messageType)
	{

	case CT_CONTACT_LIST:
	{
		ContactInfoStruct contactInfo = *(ContactInfoStruct*)(pCopyDataStruct->lpData);
		contactList.push_back(contactInfo);
		UpdateProgress();
	}
	break;

	case CT_CONTACT_CHECK:
	{
		ContactInfoStruct contactInfo = *(ContactInfoStruct*)(pCopyDataStruct->lpData);
		for (size_t i = 0; i < contactList.size(); i++)
		{
			if (0 == wcscmp(contactList[i].wcWxId, contactInfo.wcWxId))
			{
				wcscpy_s(contactList[i].wcCheckResult, contactInfo.wcCheckResult);
				break;
			}
		}

		if (0 != wcscmp(contactInfo.wcCheckResult, L"好友"))
		{
			int index = m_ContactList.GetItemCount();
			m_ContactList.InsertItem(index, to_wstring(index).c_str());
			m_ContactList.SetItemText(index, 1, contactInfo.wcWxId);
			m_ContactList.SetItemText(index, 2, contactInfo.wcWxName);
			m_ContactList.SetItemText(index, 3, contactInfo.wcCheckResult);
		}

		UpdateProgress();
	}
	break;

	case CT_FRIEND_DEL:
	{
		FriendOperStruct msg = *(FriendOperStruct*)(pCopyDataStruct->lpData);
		for (size_t i = 0; i < contactList.size(); i++)
		{
			if (0 == wcscmp(contactList[i].wcWxId, msg.wcWxId))
			{
				wcscpy_s(contactList[i].wcCheckResult, L"已清");
				break;
			}
		}

		CString itemWxId;
		int len = m_ContactList.GetItemCount();
		for (int row = 0; row < len; row++)
		{
			itemWxId = m_ContactList.GetItemText(row, 1);
			if (itemWxId == msg.wcWxId)
			{
				m_ContactList.SetItemText(row, 3, L"已清");
				break;
			}
		}

		UpdateProgress();
	}
	break;
	default:
		break;
	}

	return CDialogEx::OnCopyData(pWnd, pCopyDataStruct);
}

void CWechatClearContactDlg::UpdateProgress()
{
	int maxProgressVerify = contactList.size();
	int curProgressVerify = contactList.size();
	for (size_t i = 0; i < contactList.size(); i++)
	{
		if (0 == wcscmp(contactList[i].wcCheckResult, L"等待"))
		{
			curProgressVerify--;
		}
	}

	int maxProgressClear = 0;
	int curProgressClear = 0;

	int len = m_ContactList.GetItemCount();//取行数
	maxProgressClear = len;

	CString itemVerifyResult;
	for (int row = 0; row < len; row++)
	{
		itemVerifyResult = m_ContactList.GetItemText(row, 3);
		if (itemVerifyResult == L"已清")
		{
			curProgressClear++;
		}
	}

	m_progressVerify.SetRange(0, maxProgressVerify);
	m_progressVerify.SetPos(curProgressVerify);

	wchar_t tempVerify[30];
	swprintf_s(tempVerify, 30, L"%d/%d", curProgressVerify, maxProgressVerify);
	GetDlgItem(IDC_STATIC_PROGRESS_VERIFY)->SetWindowText(tempVerify);

	m_progressClear.SetRange(0, maxProgressClear);
	m_progressClear.SetPos(curProgressClear);
	wchar_t tempClear[30];
	swprintf_s(tempClear, 30, L"%d/%d", curProgressClear, maxProgressClear);
	GetDlgItem(IDC_STATIC_PROGRESS_CLEAR)->SetWindowText(tempClear);
}


bool isCheck = false;
void CWechatClearContactDlg::OnBnClickedButtonAllVerify()
{
	isCheck = !isCheck;
	if (isCheck)
	{
		CString csMin;
		m_editVerifyMin.GetWindowTextW(csMin);
		int min = _wtoi(csMin.GetBuffer());

		CString csMax;
		m_editVerifyMax.GetWindowTextW(csMax);
		int max = _wtoi(csMax.GetBuffer());

		int interval = RandInt(min, max);
		SetTimer(1, interval*1000, NULL);
		GetDlgItem(IDC_BUTTON_ALL_VERIFY)->SetWindowText(L"暂停检查");
	}
	else
	{
		KillTimer(1);
		GetDlgItem(IDC_BUTTON_ALL_VERIFY)->SetWindowText(L"开始检查");
	}
}

bool isClear = false;
void CWechatClearContactDlg::OnBnClickedButtonClear()
{
	isClear = !isClear;
	if (isClear)
	{
		CString csMin;
		m_editClearMin.GetWindowTextW(csMin);
		int min = _wtoi(csMin.GetBuffer());

		CString csMax;
		m_editClearMax.GetWindowTextW(csMax);
		int max = _wtoi(csMax.GetBuffer());
		int interval = RandInt(min, max);

		SetTimer(2, interval*1000, NULL);
		GetDlgItem(IDC_BUTTON_CLEAR)->SetWindowText(L"暂停清粉");
	}
	else
	{
		KillTimer(2);
		GetDlgItem(IDC_BUTTON_CLEAR)->SetWindowText(L"开始清粉");
	}
}


void CWechatClearContactDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (nPos > 0)
	{
		if (pScrollBar->GetDlgCtrlID() == IDC_SLIDER_VERIFY)
		{
			m_posVerify = nPos;

			wchar_t temp[30];
			swprintf_s(temp, 30, L"检查时间间隔/秒：%d", nPos);
			GetDlgItem(IDC_STATIC_VERIFY_INTERVAL)->SetWindowText(temp);
		}
		else if (pScrollBar->GetDlgCtrlID() == IDC_SLIDER_CLEAR)
		{
			m_posClear = nPos;

			wchar_t temp[30];
			swprintf_s(temp, 30, L"清粉时间间隔/秒：%d", nPos);
			GetDlgItem(IDC_STATIC_CLEAR_INTERVAL)->SetWindowText(temp);
		}
	}


	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}


void CWechatClearContactDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 1)
	{
		for (size_t i = 0; i < contactList.size(); i++)
		{
			if (0 == wcscmp(contactList[i].wcCheckResult, L"等待"))
			{
				FriendOperStruct message = { 0 };
				wchar_t* wxid = contactList[i].wcWxId;
				wcscpy_s(message.wcWxId, wcslen(wxid) + 1, wxid);
				wcscpy_s(contactList[i].wcCheckResult, L"好友");
				SendCmdToSlave(CT_CONTACT_CHECK, sizeof(FriendOperStruct), &message);
				break;
			}
		}
	}
	if (nIDEvent == 2)
	{
		CString itemVerifyResult;
		for (int row = 0; row < m_ContactList.GetItemCount(); row++)
		{
			itemVerifyResult = m_ContactList.GetItemText(row, 3);
			if (itemVerifyResult != L"已清")
			{
				CString csWxId = m_ContactList.GetItemText(row, 1);

				FriendOperStruct message = { 0 };
				wchar_t* wxid = csWxId.AllocSysString();
				wcscpy_s(message.wcWxId, wcslen(wxid) + 1, wxid);
				SendCmdToSlave(CT_FRIEND_DEL, sizeof(FriendOperStruct), &message);
				break;
			}
		}
		
		
	}

	CDialogEx::OnTimer(nIDEvent);
}
