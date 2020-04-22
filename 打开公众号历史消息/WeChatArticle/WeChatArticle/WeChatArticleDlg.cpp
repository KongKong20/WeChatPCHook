
// WeChatArticleDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "WeChatArticle.h"
#include "WeChatArticleDlg.h"
#include "afxdialogex.h"
#include <afxinet.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CWeChatArticleDlg 对话框



CWeChatArticleDlg::CWeChatArticleDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_WECHATARTICLE_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CWeChatArticleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CWeChatArticleDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_OPEN_WECHAT, &CWeChatArticleDlg::OnBnClickedButtonOpenWechat)
	ON_BN_CLICKED(IDC_BUTTON_OPEN_ARTIClE, &CWeChatArticleDlg::OnBnClickedButtonOpenArticle)
END_MESSAGE_MAP()


// CWeChatArticleDlg 消息处理程序

BOOL CWeChatArticleDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	srand((unsigned)time(NULL));

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CWeChatArticleDlg::OnPaint()
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
HCURSOR CWeChatArticleDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


int GetWeChatPath(WCHAR* Path)
{
	int ret = -1;
	//HKEY_CURRENT_USER\Software\Tencent\WeChat InstallPath = xx
	HKEY hKey = NULL;
	if (ERROR_SUCCESS != RegOpenKey(HKEY_CURRENT_USER, L"Software\\Tencent\\WeChat", &hKey))
	{
		ret = GetLastError();
		return ret;
	}

	DWORD Type = REG_SZ;
	// WCHAR Path[MAX_PATH] = { 0 };
	DWORD cbData = MAX_PATH * sizeof(WCHAR);
	if (ERROR_SUCCESS != RegQueryValueEx(hKey, L"InstallPath", 0, &Type, (LPBYTE)Path, &cbData))
	{
		ret = GetLastError();
		goto __exit;
	}

	PathAppend(Path, L"WeChat.exe");
	//PathAppend(Path, L"WeChat.exe");

__exit:
	if (hKey)
	{
		RegCloseKey(hKey);
	}

	return ERROR_SUCCESS;
}

void CWeChatArticleDlg::OnBnClickedButtonOpenWechat()
{
	WCHAR Path[MAX_PATH] = { 0 };
	int ret = GetWeChatPath(Path);
	if (ERROR_SUCCESS == ret) 
	{
		ShellExecute(NULL, L"open", Path, L"--remote-debugging-port=8200", NULL, SW_SHOW);
	}
}


void CWeChatArticleDlg::OnBnClickedButtonOpenArticle()
{
	CString random;
	random.Format(_T("%d"), rand());

	CInternetSession session(NULL, 0);
	CHttpFile* htmlFile = NULL;

	CString strLine, strHtml;
	CString url = _T("http://localhost:8200/json?") + random;
	TCHAR sRecv[1024];
	UINT CodePage = 65001;//CP_UTF8:65001 CP_ACP:0  
	strHtml = _T("");
	//获取网页源码  
	htmlFile = (CHttpFile*)session.OpenURL(url);//重新打开连接  
	DWORD dwStatusCode;  //接受请求返回值
	htmlFile->QueryInfoStatusCode(dwStatusCode);
	if (dwStatusCode == HTTP_STATUS_OK)
	{
		while (htmlFile->ReadString(sRecv, 1024))
		{
			// 编码转换，可解决中文乱码问题  
			//gb2312转为unicode,则用CP_ACP  
			//gbk转为unicode,也用CP_ACP  
			//utf-8转为unicode,则用CP_UTF8  
			int nBufferSize = MultiByteToWideChar(CodePage, 0, (LPCSTR)sRecv, -1, NULL, 0);

			wchar_t *pBuffer = new wchar_t[nBufferSize + 1];
			memset(pBuffer, 0, (nBufferSize + 1) * sizeof(wchar_t));

			//gb2312转为unicode,则用CP_ACP  
			//gbk转为unicode,也用CP_ACP  
			//utf-8转为unicode,则用CP_UTF8  
			MultiByteToWideChar(CodePage, 0, (LPCSTR)sRecv, -1, pBuffer, nBufferSize * sizeof(wchar_t));

			strHtml += pBuffer;
			strHtml += "\r\n";
			delete pBuffer;
		}

		int begin = strHtml.Find(L"https");
		if (begin >= 0)
		{
			int end = strHtml.Find(L"\"", begin);
			if (end >= 0)
			{
				CString articleUrl = strHtml.Mid(begin, end-begin);
				OutputDebugString(articleUrl);
				ShellExecute(0, NULL, articleUrl, NULL, NULL, SW_NORMAL);
			}
		}
	}

	htmlFile->Close();
	session.Close();
	delete htmlFile;
}
