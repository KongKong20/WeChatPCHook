
// RemarkHelperDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "RemarkHelper.h"
#include "RemarkHelperDlg.h"
#include "afxdialogex.h"
#include "tlhelp32.h"

#include <fstream>
using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CRemarkHelperDlg 对话框

string CStringToString(CString& csStr)
{
	string str;

	DWORD dBufSize = WideCharToMultiByte(CP_UTF8, 0, csStr, -1, NULL, 0, NULL, FALSE);

	char *dBuf = new char[dBufSize];
	memset(dBuf, 0, dBufSize);
	WideCharToMultiByte(CP_UTF8, 0, csStr, csStr.GetLength(), dBuf, dBufSize, NULL, FALSE);
	str = dBuf;
	delete[] dBuf;

	return str;
}

CString StringToCString(string& str)
{
	//预转换，得到所需空间的大小;
	int wcsLen = ::MultiByteToWideChar(CP_UTF8, NULL, str.c_str(), strlen(str.c_str()), NULL, 0);

	//分配空间要给'\0'留个空间，MultiByteToWideChar不会给'\0'空间
	wchar_t* wszString = new wchar_t[wcsLen + 1];

	//转换
	::MultiByteToWideChar(CP_UTF8, NULL, str.c_str(), strlen(str.c_str()), wszString, wcsLen);

	//最后加上'\0'
	wszString[wcsLen] = '\0';

	CString unicodeString(wszString);

	delete[] wszString;
	wszString = NULL;

	return unicodeString;
}



CRemarkHelperDlg::CRemarkHelperDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_REMARKHELPER_DIALOG, pParent)
	, curChatId(_T(""))
	, check1(0)
	, check2(0)
	, check3(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CRemarkHelperDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, cbProgress);
	DDX_Control(pDX, IDC_LIST_RRMARK, lbRemark);
}

BEGIN_MESSAGE_MAP(CRemarkHelperDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, &CRemarkHelperDlg::OnBnClickedButtonClose)
	ON_BN_CLICKED(IDC_BUTTON_REMAEK, &CRemarkHelperDlg::OnBnClickedButtonRemaek)
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CRemarkHelperDlg 消息处理程序

BOOL CRemarkHelperDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	AttachWindow();

	SetTimer(1, 1000, NULL);
	SetTimer(2, 1000, NULL);

	std::ifstream remarkJsonFile("remark.json", std::ifstream::binary);
	remarkJsonFile >> remarkJson;
	remarkJsonFile.close();

	Json::Value configJson;
	std::ifstream configJsonFile("config.json", std::ifstream::binary);
	configJsonFile >> configJson;
	configJsonFile.close();

	const Json::Value tagList = configJson["tagList"];
	for (UINT index = 0; index < tagList.size(); ++index)
	{
		CString tagName = CString(tagList[index].asString().c_str());
		if (index == 0) ((CButton*)GetDlgItem(IDC_CHECK_1))->SetWindowText(tagName);
		else if (index == 1) ((CButton*)GetDlgItem(IDC_CHECK_2))->SetWindowText(tagName);
		else if (index == 2) ((CButton*)GetDlgItem(IDC_CHECK_3))->SetWindowText(tagName);
	}

	const Json::Value progressList = configJson["progressList"];
	for (UINT index = 0; index < progressList.size(); ++index)
	{
		CString progressName = CString(progressList[index].asString().c_str());
		cbProgress.AddString(progressName);
	}
	cbProgress.SetCurSel(0);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CRemarkHelperDlg::OnPaint()
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
HCURSOR CRemarkHelperDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

BOOL CRemarkHelperDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		switch (pMsg->wParam)
		{
		case VK_RETURN:
		{
			return TRUE;
		}

		case VK_ESCAPE:
		{
			return TRUE;
		}
		default:
			break;
		}
	}
	else if (pMsg->message == WM_CLOSE)
	{
		SaveJsonData();
	}

	return CDialog::PreTranslateMessage(pMsg);
}

// 伴随窗口
void CRemarkHelperDlg::AttachWindow()
{
	CWnd *pWxWnd = CWnd::FindWindow(TEXT("WeChatMainWndForPC"), NULL);
	if (pWxWnd == NULL) return;
	if (!pWxWnd->IsWindowVisible()) return;

	CRect wxRect;
	pWxWnd->GetWindowRect(wxRect);

	int wxLeft = wxRect.left;
	int wxTop = wxRect.top;
	int wxRight = wxRect.right;
	int wxBottom = wxRect.bottom;
	int wxWidth = wxRight - wxLeft;
	int wxHeight = wxBottom - wxTop;

	int myHeight = wxHeight;
	int myWidth = myHeight / 2;
	int myLeft = wxRight;
	int myTop = wxTop;
	::SetWindowPos(this->m_hWnd, pWxWnd->m_hWnd, myLeft, myTop, myWidth, myHeight, 0);
}


// 获取当前聊天ID
CString CRemarkHelperDlg::GetCurChatId()
{
	CString csChatId;

	CString processName = L"WeChat.exe";
	CString moduleName = L"WeChatWin.dll";
	DWORD moduleBaseAddress = 0;

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

	if (processID == 0)
	{
		return csChatId;
	}

	HANDLE hProcessSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processID);
	if (hProcessSnapshot == INVALID_HANDLE_VALUE) return csChatId;

	MODULEENTRY32 me32;
	SecureZeroMemory(&me32, sizeof(MODULEENTRY32));
	me32.dwSize = sizeof(MODULEENTRY32);

	while (Module32Next(hProcessSnapshot, &me32))
	{
		me32.dwSize = sizeof(MODULEENTRY32);

		if (!_tcscmp(me32.szModule, moduleName.GetBuffer()))
		{
			moduleBaseAddress = (DWORD)me32.modBaseAddr;

			HANDLE hProcess = OpenProcess(PROCESS_VM_READ, FALSE, processID);
			if (hProcess)
			{
				DWORD wxIdAddress = 0;
				//ReadProcessMemory(hProcess, (LPCVOID)(moduleBaseAddress + 0x13A99D4), &wxIdAddress, 4, NULL);

				ReadProcessMemory(hProcess, (LPCVOID)(moduleBaseAddress + 0x13978CC), &wxIdAddress, 4, NULL);

				BYTE wxIdBuffer[200] = { 0 };
				ReadProcessMemory(hProcess, (LPCVOID)wxIdAddress, wxIdBuffer, 200, NULL);
				csChatId = CString((wchar_t*)wxIdBuffer);

				CloseHandle(hProcess);
			}

			break;
		}
	}

	CloseHandle(hProcessSnapshot);

	//string sChatId = CStringToString(csChatId);
	//size_t hChatId = hash<string>()(sChatId);
	return csChatId;
}


void CRemarkHelperDlg::OnTimer(UINT_PTR nIDEvent)
{
	switch (nIDEvent)
	{
	case 1:
	{
		AttachWindow();
	}
	break;

	case 2:
	{
		CString chatId = GetCurChatId();
		if (chatId != curChatId)
		{
			string oldChatId = CStringToString(curChatId);
			if (oldChatId.size() > 0)
			{
				SaveJsonData();
			}

			curChatId = chatId;
			OutputDebugString(chatId);
			OutputDebugString(L"\r\n");
			GetDlgItem(IDC_STATIC_ID)->SetWindowText(L"ID："+ chatId);

			string newChatId = CStringToString(curChatId);
			if (remarkJson[newChatId]["progress"].isNull())
			{
				remarkJson[newChatId]["progress"] = 0;
			}
			int iProgress = remarkJson[newChatId]["progress"].asInt();
			cbProgress.SetCurSel(iProgress);

			CString csProgress;
			csProgress.Format(L"%d", iProgress);
			OutputDebugString(csProgress);
			OutputDebugString(L"\r\n");

			((CButton*)GetDlgItem(IDC_CHECK_1))->SetCheck(0);
			((CButton*)GetDlgItem(IDC_CHECK_2))->SetCheck(0);
			((CButton*)GetDlgItem(IDC_CHECK_3))->SetCheck(0);

			const Json::Value tagList = remarkJson[newChatId]["tags"];
			int count = tagList.size();
			for (UINT index = 0; index < tagList.size(); ++index)
			{
				int status = tagList[index].asInt();
				if (index == 0) ((CButton*)GetDlgItem(IDC_CHECK_1))->SetCheck(status);
				else if (index == 1) ((CButton*)GetDlgItem(IDC_CHECK_2))->SetCheck(status);
				else if (index == 2) ((CButton*)GetDlgItem(IDC_CHECK_3))->SetCheck(status);
			}

			lbRemark.ResetContent();
			const Json::Value remarkList = remarkJson[newChatId]["remark"];
			for (UINT index = 0; index < remarkList.size(); ++index)
			{
				string sRemark = remarkList[index].asCString();
				CString csRemark = StringToCString(sRemark);
				int count = lbRemark.GetCount();
				lbRemark.InsertString(index, csRemark);
			}
		}
	}
	break;

	default:
		break;
	}

	CDialogEx::OnTimer(nIDEvent);
}

void CRemarkHelperDlg::OnBnClickedButtonClose()
{
	SaveJsonData();
	DestroyWindow();
	//CString chatId = L"filehelper";
	//std::string sChatId = CT2A(chatId.GetBuffer());
	//if (remarkJson[sChatId].isNull()) {
	//	CString csName = L"文件助手";
	//	remarkJson[sChatId]["name"] = CStringToString(csName).c_str();

	//	
	//}
	//else {
	//	CString csName = CString(remarkJson[sChatId]["name"].asCString());
	//}
	
	//Json::Value root;
	//std::ifstream config_doc("config.json", std::ifstream::binary);
	//config_doc >> root;
	//config_doc.close();

	//std::string my_encoding = root["my-encoding"].asString();
	//const Json::Value my_plugins = root["my-plug-ins"];
	//for (int index = 0; index < my_plugins.size(); ++index)  // Iterates over the sequence elements.
	//	my_plugins[index].asString();

	//int length = root["my-indent"]["length"].asInt();

	//root["test"] = 1;

	//std::fstream f;
	//f.open("config.json", std::ios::out);
	//if (f.is_open()) {
	//	f << root.toStyledString(); //转换为json格式并存到文件流
	//}
	//f.close();
}


void CRemarkHelperDlg::OnBnClickedButtonRemaek()
{
	CString remark;
	GetDlgItem(IDC_EDIT_REMARK)->GetWindowText(remark);
	//lbRemark.AddString(remark);

	int count = lbRemark.GetCount();
	lbRemark.InsertString(count, remark);
}


void CRemarkHelperDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	if (nType != SIZE_MINIMIZED)  //判断窗口是不是最小化了，因为窗口最小化之后 ，窗口的长和宽会变成0，当前一次变化的时就会出现除以0的错误操作
	{
		float fsp[2];
		POINT Newp; //获取现在对话框的大小  
		CRect recta;
		GetClientRect(&recta);     //取客户区大小    
		Newp.x = recta.right - recta.left;
		Newp.y = recta.bottom - recta.top;
		fsp[0] = (float)Newp.x / old.x;
		fsp[1] = (float)Newp.y / old.y;
		CRect Rect;
		int woc;
		CPoint OldTLPoint, TLPoint; //左上角  
		CPoint OldBRPoint, BRPoint; //右下角  
		HWND  hwndChild = ::GetWindow(m_hWnd, GW_CHILD);  //列出所有控件    
		while (hwndChild) {
			woc = ::GetDlgCtrlID(hwndChild);//取得ID  
			GetDlgItem(woc)->GetWindowRect(Rect);
			ScreenToClient(Rect);
			OldTLPoint = Rect.TopLeft();
			TLPoint.x = long(OldTLPoint.x*fsp[0]);
			TLPoint.y = long(OldTLPoint.y*fsp[1]);
			OldBRPoint = Rect.BottomRight();
			BRPoint.x = long(OldBRPoint.x *fsp[0]);
			BRPoint.y = long(OldBRPoint.y *fsp[1]);
			Rect.SetRect(TLPoint, BRPoint);
			GetDlgItem(woc)->MoveWindow(Rect, TRUE);
			hwndChild = ::GetWindow(hwndChild, GW_HWNDNEXT);
		}
		old = Newp;
	}
}


void CRemarkHelperDlg::SaveJsonData()
{
	string sChatId = CStringToString(curChatId);
	int saveProgress = cbProgress.GetCurSel();
	remarkJson[sChatId]["progress"] = saveProgress;

	Json::Value tagList;
	if (BST_CHECKED == IsDlgButtonChecked(IDC_CHECK_1))
	{
		tagList.append(1);
	}
	else
	{
		tagList.append(0);
	}
	if (BST_CHECKED == IsDlgButtonChecked(IDC_CHECK_2))
	{
		tagList.append(1);
	}
	else
	{
		tagList.append(0);
	}
	if (BST_CHECKED == IsDlgButtonChecked(IDC_CHECK_3))
	{
		tagList.append(1);
	}
	else
	{
		tagList.append(0);
	}
	remarkJson[sChatId]["tags"] = tagList;

	Json::Value remarkList;
	for (int index = 0; index < lbRemark.GetCount(); ++index)
	{
		CString csRemark;
		lbRemark.GetText(index, csRemark);
		string sRemark = CStringToString(csRemark);
		remarkList.append(sRemark.c_str());
	}

	remarkJson[sChatId]["remark"] = remarkList;
	std::fstream fRemark;
	fRemark.open("remark.json", std::ios::out);
	if (fRemark.is_open()) {
		fRemark << remarkJson.toStyledString(); //转换为json格式并存到文件流
	}
	fRemark.close();

}
