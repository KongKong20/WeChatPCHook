
// RemarkHelperDlg.h: 头文件
//

#pragma once
#include "json/json.h"

// CRemarkHelperDlg 对话框
class CRemarkHelperDlg : public CDialogEx
{
// 构造
public:
	CRemarkHelperDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_REMARKHELPER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;
	POINT old;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg BOOL PreTranslateMessage(MSG* pMsg);
	DECLARE_MESSAGE_MAP()
public:
	// 伴随窗口
	void AttachWindow();
	// 获取当前聊天ID
	CString GetCurChatId();

	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedButtonClose();
	// 当前聊天ID
	CString curChatId;

	Json::Value remarkJson;
	CComboBox cbProgress;
	CListBox lbRemark;
	afx_msg void OnBnClickedButtonRemaek();
	UINT check1;
	UINT check2;
	UINT check3;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	void SaveJsonData();
};
