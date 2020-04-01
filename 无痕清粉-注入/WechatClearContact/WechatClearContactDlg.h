
// WechatClearContactDlg.h: 头文件
//

#pragma once


// CWechatClearContactDlg 对话框
class CWechatClearContactDlg : public CDialogEx
{
// 构造
public:
	CWechatClearContactDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_WECHATCLEARCONTACT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_ContactList;
	CProgressCtrl m_progressVerify;
	CProgressCtrl m_progressClear;
	int m_posVerify;
	int m_posClear;
	afx_msg void OnBnClickedButtonWechat();
	afx_msg BOOL OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct);
	void UpdateProgress();
	afx_msg void OnBnClickedButtonAllVerify();
	afx_msg void OnBnClickedButtonClear();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	CEdit m_editVerifyMin;
	CEdit m_editVerifyMax;
	CEdit m_editClearMin;
	CEdit m_editClearMax;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
