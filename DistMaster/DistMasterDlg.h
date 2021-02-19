// DistMasterDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"


// CDistMasterDlg dialog
class CDistMasterDlg : public CDialog
{
// Construction
public:
	CDistMasterDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_DISTMASTER_DIALOG };

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
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnClose();
	CStatic lblStatus;
	CButton btnStart;
	CListCtrl lstSlaverAddresses;
	CEdit txtMinSup;
	CString strTreeFile;
	CEdit txtTreeFilename;
	VOID EnableCloseDialog(bool bEnable);
	CEdit txtSeqData;
};

extern CString strSeqData;
