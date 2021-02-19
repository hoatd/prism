// DistSlaverDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DistSlaver.h"
#include "DistSlaverDlg.h"
#include "DistSlaverSocket.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CDistSlaverDlg dialog

CDistSlaverDlg::CDistSlaverDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDistSlaverDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDistSlaverDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_PORT, txtPort);
	DDX_Control(pDX, IDOK, btnStart);
	DDX_Control(pDX, IDC_STATIC_STATUS, lblStatus);
	DDX_Control(pDX, IDC_EDIT_ADDRESS, txtAddress);
	DDX_Text(pDX, IDC_EDIT_ADDRESS, SocketAddress);
	DDX_Text(pDX, IDC_EDIT_PORT, SocketPort);
	DDX_Control(pDX, IDC_EDIT_MINSUP, txtMinSup);
	DDX_Control(pDX, IDC_EDIT_SEQFILE, txtSeqDataFilename);
}

BEGIN_MESSAGE_MAP(CDistSlaverDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDCANCEL, &CDistSlaverDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDOK, &CDistSlaverDlg::OnBnClickedOk)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CDistSlaverDlg message handlers

BOOL CDistSlaverDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	strCaption.Format(L"Slaver Site %02i", SiteId);
	SetWindowText(strCaption);

	SetMasterInfo();

	txtAddress.SetWindowText(SocketAddress);
	lblStatus.SetWindowText(L"READY");

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CDistSlaverDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CDistSlaverDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CDistSlaverDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CDistSlaverDlg::OnBnClickedCancel()
{
}

void CDistSlaverDlg::OnBnClickedOk()
{
	UpdateData();
	
	EnableProcess(FALSE);
	
	AfxBeginThread(CommWithMasterSite, NULL);
}

void CDistSlaverDlg::OnClose()
{
	CDialog::OnOK();
}

VOID CDistSlaverDlg::EnableCloseDialog(bool bEnable)
{
	UINT menuf = bEnable ? (MF_BYCOMMAND) : (MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);

	CMenu* pSM = GetSystemMenu(FALSE);
	if(pSM) {
		pSM->EnableMenuItem(SC_CLOSE, menuf);
	}
}

void CDistSlaverDlg::EnableProcess(bool bEnable)
{
	((CDistSlaverDlg*)theApp.m_pMainWnd)->EnableCloseDialog(bEnable);
	((CDistSlaverDlg*)theApp.m_pMainWnd)->btnStart.EnableWindow(bEnable);
	((CDistSlaverDlg*)theApp.m_pMainWnd)->txtAddress.EnableWindow(bEnable);
	((CDistSlaverDlg*)theApp.m_pMainWnd)->txtPort.EnableWindow(bEnable);
}

UINT CDistSlaverDlg::CommWithMasterSite(LPVOID lParam)
{
	HANDLE hThread = AfxBeginThread(SocketThreadFuncSlaverServer, NULL)->m_hThread;

	WaitForMultipleObjects(1, &hThread, TRUE, INFINITE);

	EnableProcess(TRUE);

	return 1;
}
