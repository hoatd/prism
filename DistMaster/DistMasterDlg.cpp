// DistMasterDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DistMaster.h"
#include "DistMasterDlg.h"
#include "DistMasterSocket.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern double MINSUP_IN_PERCENT;
CString strSeqData = _T("N1kD1k");
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


// CDistMasterDlg dialog

CDistMasterDlg::CDistMasterDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDistMasterDlg::IDD, pParent)
	, strTreeFile(_T("MASTER.TREE"))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDistMasterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_STATUS, lblStatus);
	DDX_Control(pDX, IDOK, btnStart);
	DDX_Control(pDX, IDC_LIST_SLAVER_ADDRESSES, lstSlaverAddresses);
	DDX_Text(pDX, IDC_EDIT_MINSUP, DS_CONF->rel_minsup);
	DDV_MinMaxDouble(pDX, DS_CONF->rel_minsup, 0, 1);
	DDX_Control(pDX, IDC_EDIT_MINSUP, txtMinSup);
	DDX_Text(pDX, IDC_EDIT_TREEFILE, strTreeFile);
	DDX_Control(pDX, IDC_EDIT_TREEFILE, txtTreeFilename);
	DDX_Control(pDX, IDC_EDIT_SEQDATA, txtSeqData);
	DDX_Text(pDX, IDC_EDIT_SEQDATA, strSeqData);
}

BEGIN_MESSAGE_MAP(CDistMasterDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, &CDistMasterDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CDistMasterDlg::OnBnClickedCancel)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CDistMasterDlg message handlers

BOOL CDistMasterDlg::OnInitDialog()
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

	lstSlaverAddresses.SetExtendedStyle( LVS_EX_FULLROWSELECT );
	lstSlaverAddresses.InsertColumn(0, L"SITE", LVCFMT_CENTER, 40);
	lstSlaverAddresses.InsertColumn(1, L"ADDRESS", LVCFMT_LEFT, 120);
	lstSlaverAddresses.InsertColumn(2, L"PORT", LVCFMT_CENTER, 50);
	lstSlaverAddresses.InsertColumn(3, L"STATUS", LVCFMT_LEFT, 280);

	LVITEM lvi;
	CString strItem;

	for (int i = 0; i < NUM_SLAVER_ADDRESS; i ++) {
		lvi.mask =  LVIF_TEXT;
		lvi.iItem = i;

		strItem.Format(_T("%02i"), i + 1);
		lvi.iSubItem = 0;
		lvi.pszText = (LPTSTR)(LPCTSTR)(strItem);

		lstSlaverAddresses.InsertItem(&lvi);

		strItem.Format(_T("%s"), CA2W(SLAVER_ADDRESSES[i].strAddress));
		lvi.iSubItem = 1;
		lvi.pszText = (LPTSTR)(LPCTSTR)(strItem);

		lstSlaverAddresses.SetItem(&lvi);

		strItem.Format(_T("%i"), SLAVER_ADDRESSES[i].nPort);
		lvi.iSubItem = 2;
		lvi.pszText = (LPTSTR)(LPCTSTR)(strItem);

		lstSlaverAddresses.SetItem(&lvi);

		strItem.Format(_T("%s"), _T("READY."));
		lvi.iSubItem = 3;
		lvi.pszText = (LPTSTR)(LPCTSTR)(strItem);

		lstSlaverAddresses.SetItem(&lvi);
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CDistMasterDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CDistMasterDlg::OnPaint()
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
HCURSOR CDistMasterDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

extern encoding_block_collection* EBS;
extern INT ERROR_CODE;

UINT CommWithMultiClient(LPVOID lParam) {
	ERROR_CODE = 0;

	clock_t start_receiving = clock();

	sync_encoding_block_collection_init();

	HANDLE* hThreads = new HANDLE[NUM_SLAVER_ADDRESS];

	for (int i = 0; i < NUM_SLAVER_ADDRESS; i ++) {
		hThreads[i] = AfxBeginThread(SocketThreadFuncMasterClient, &i)->m_hThread;

		Sleep(300);
	}

	WaitForMultipleObjects(NUM_SLAVER_ADDRESS, hThreads, TRUE, INFINITE);
	
	delete []hThreads;

	if (ERROR_CODE != 2) {

		if (ERROR_CODE == 0) {
			SetStatusMaster(L"ERROR ON SOCKET OR NETWORK.");
			AfxMessageBox(L"ERROR ON SOCKET OR NETWORK.");
		}
		else {
			SetStatusMaster(L"ERROR ON SEQUENCE DATA FILE.");
			AfxMessageBox(L"ERROR ON SEQUENCE DATA FILE.");
		}
		
		((CDistMasterDlg*)theApp.m_pMainWnd)->txtSeqData.EnableWindow(1);
		((CDistMasterDlg*)theApp.m_pMainWnd)->txtMinSup.EnableWindow(1);
		((CDistMasterDlg*)theApp.m_pMainWnd)->txtTreeFilename.EnableWindow(1);
		((CDistMasterDlg*)theApp.m_pMainWnd)->btnStart.EnableWindow(1);
		((CDistMasterDlg*)theApp.m_pMainWnd)->EnableCloseDialog(0);

		return 0;
	}

	sync_encoding_block_collection_sort_by_item();

	clock_t finish_receiving = clock();

	//print_encoded_blocks(EBS);

	SetStatusMaster(L"EXTENDING TREE ...");

	clock_t start_extending = clock();

	unsigned int numblks = (unsigned int)EBS->size() + extend_root(EBS, DS_CONF);

	clock_t finish_extending = clock();

	CString str;

	double extending_sec = ((double)(finish_extending - start_extending) / CLOCKS_PER_SEC);
	double extending_min = extending_sec / 60;
	double receiving_sec = ((double)(finish_receiving - start_receiving) / CLOCKS_PER_SEC) - (300 * NUM_SLAVER_ADDRESS) / (double)10000 ;
	double receiving_min = receiving_sec / 60;

	str.Format(L"MINING SUCCESS: %i FREQUENT SEQUENCES.\n\nEXTENDING TIME:\t %f SEC (%f MIN)\nWAITING TIME:\t %f SEC (%f MIN)\n\nMINING TIME:\t %f SEC (%f MIN).",
		numblks, extending_sec, extending_min, receiving_sec, receiving_min, extending_sec + receiving_sec, extending_min + receiving_min);

	char treefilename[256];

	int sz = GetTreeFilename(treefilename);
	
	if (sz > 0 ) {
		ofstream tree(treefilename);
	
		print_tree(EBS, DS_CONF, true, tree);

		CString filename(treefilename);

		str += L"\n\nTREE RESULT WROTE TO FILE " + filename;
	}

	AfxMessageBox(str);	

	sync_encoding_block_collection_clean();

	SetStatusMaster(L"READY");

	((CDistMasterDlg*)theApp.m_pMainWnd)->txtSeqData.EnableWindow(1);
	((CDistMasterDlg*)theApp.m_pMainWnd)->txtMinSup.EnableWindow(1);
	((CDistMasterDlg*)theApp.m_pMainWnd)->txtTreeFilename.EnableWindow(1);
	((CDistMasterDlg*)theApp.m_pMainWnd)->btnStart.EnableWindow(1);
	((CDistMasterDlg*)theApp.m_pMainWnd)->EnableCloseDialog(0);

	return 1;
}

void CDistMasterDlg::OnBnClickedOk()
{
	UpdateData();

	if (strSeqData.GetLength() <= 0) return;

	txtSeqData.EnableWindow(0);
	txtMinSup.EnableWindow(0);
	txtTreeFilename.EnableWindow(0);
	btnStart.EnableWindow(0);
	EnableCloseDialog(1);

	DS_CONF->abs_minsup = (unsigned int)ceil(DS_CONF->rel_minsup * DS_CONF->numseqs);

	AfxBeginThread(CommWithMultiClient, 0);
}

void CDistMasterDlg::OnBnClickedCancel()
{
}

void CDistMasterDlg::OnClose()
{
	OnOK();
}

VOID CDistMasterDlg::EnableCloseDialog(bool bEnable)
{
	bEnable = !bEnable;

	UINT menuf = bEnable ? (MF_BYCOMMAND) : (MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);

	CMenu* pSM = GetSystemMenu(FALSE);
	if(pSM) {
		pSM->EnableMenuItem(SC_CLOSE, menuf);
	}
}
