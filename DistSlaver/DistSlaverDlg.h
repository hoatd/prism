// DistSlaverDlg.h : header file
//

#pragma once
#include "afxwin.h"


// CDistSlaverDlg dialog
class CDistSlaverDlg : public CDialog
{
// Construction
public:
	CDistSlaverDlg(CWnd* pParent = NULL);	// standard constructor
	
// Dialog Data
	enum { IDD = IDD_DISTSLAVER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedOk();
	afx_msg void OnClose();
	CEdit txtPort;
	CButton btnStart;
	CStatic lblStatus;
	CEdit txtAddress;
	CEdit txtMinSup;
	VOID EnableCloseDialog(bool bEnable);
	static void EnableProcess(bool bEnable);
	static UINT CommWithMasterSite(LPVOID lParam);
	CEdit txtSeqDataFilename;
};
