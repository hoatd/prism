// DistMaster.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "DistMaster.h"
#include "DistMasterDlg.h"
#include "DistMasterSocket.h"

#include <iostream>
#include <fstream>

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CDistMasterApp

BEGIN_MESSAGE_MAP(CDistMasterApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CDistMasterApp construction

CDistMasterApp::CDistMasterApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CDistMasterApp object

CDistMasterApp theApp;


// CDistMasterApp initialization

BOOL CDistMasterApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	DS_CONF = dataset_conf_init();
	//dataset_conf_params(DS_CONF);

	ifstream ifs;
	
	ifs.open("Master.Conf");
	if (ifs.fail()) {
		AfxMessageBox(L"ERROR ON CONFIGULATION FILE: MASTER.CONF .\n");

		return FALSE;
	}

	char szLine[256];
	
	char szSeqData[256];
	char szSeps[]   = " \t\n#";
	ifs.getline(szLine, 255);
	char* szToken = strtok(szLine, szSeps);
	strcpy(szSeqData, szToken);
	strSeqData = CA2W(szSeqData);

	ifs.getline(szLine, 255);
	DS_CONF->numseqs = atoi(szLine);

	ifs.getline(szLine, 255);
	DS_CONF->miniid = atoi(szLine);

	ifs.getline(szLine, 255);
	DS_CONF->maxiid = atoi(szLine);

	ifs.getline(szLine, 255);
	DS_CONF->maxslen = atoi(szLine);

	ifs.getline(szLine, 255);
	DS_CONF->maxilen = atoi(szLine);

	ifs.getline(szLine, 255);
	NUM_SLAVER_ADDRESS = atoi(szLine);

	SLAVER_ADDRESSES = new SLAVER_ADDRESS[NUM_SLAVER_ADDRESS];

	for (int i = 0; i < NUM_SLAVER_ADDRESS; i ++) {
		ifs.getline(szLine, 255);
		
		char* szPort = strstr(szLine, ":") + 1;
		size_t lHost = szPort - szLine - 1;
	
		strncpy(SLAVER_ADDRESSES[i].strAddress, szLine, lHost);
		SLAVER_ADDRESSES[i].strAddress[lHost] = '\0';

		SLAVER_ADDRESSES[i].nPort = atoi(szPort);
	}
	
	ifs.close();

	
	dataset_conf_precompute(DS_CONF);
	
	if (!SocketInitMaster()) {
		return FALSE;
	}

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	CDistMasterDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	SocketCleanMaster();

	dataset_conf_clean(DS_CONF);
	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

VOID SetStatusMaster(LPCTSTR sStatus) {
	if (theApp.m_pMainWnd) {
		((CDistMasterDlg*)theApp.m_pMainWnd)->lblStatus.SetWindowText(sStatus);
	}
}

VOID SetStatusSite(int site, LPCTSTR sStatus) {
	if (theApp.m_pMainWnd) {
		LVITEM lvi;
		CString strItem;
		lvi.mask =  LVIF_TEXT;
		lvi.iItem = site;
		lvi.iSubItem = 3;
		lvi.pszText = (LPTSTR)(LPCTSTR)(sStatus);
		((CDistMasterDlg*)theApp.m_pMainWnd)->lstSlaverAddresses.SetItem(&lvi);
	}
}

int GetTreeFilename(char* filename) {
	if (theApp.m_pMainWnd) {
		CString& strTreeFile = ((CDistMasterDlg*)theApp.m_pMainWnd)->strTreeFile;
		return wcstombs(filename, (LPCTSTR)strTreeFile, 256);
	}
	return 0;
}

int GetSeqDataName(char* dataname) {
	return (int)wcstombs(dataname, (LPCTSTR)strSeqData, 256);
}