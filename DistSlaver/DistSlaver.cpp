// DistSlaver.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "DistSlaver.h"
#include "DistSlaverDlg.h"
#include "DistSlaverSocket.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CDistSlaverApp

BEGIN_MESSAGE_MAP(CDistSlaverApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CDistSlaverApp construction

CDistSlaverApp::CDistSlaverApp()
{
}


// The one and only CDistSlaverApp object

CDistSlaverApp theApp;

CString strCaption;
// CDistSlaverApp initialization

BOOL CDistSlaverApp::InitInstance()
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

	ifstream ifs;
	
	ifs.open("Slaver.Conf");
	if (ifs.fail()) {
		MessageBox(NULL, L"ERROR ON CONFIGULATION FILE: SLAVER.CONF .\n", strCaption, MB_ICONERROR | MB_OK);

		return FALSE;
	}

	char szLine[256];
	
	ifs.getline(szLine, 255);
	SiteId = atoi(szLine);

	ifs.getline(szLine, 255);
	SocketPort = atoi(szLine);

	if (!SocketInitSlaver()) {
		return FALSE;
	}

	if (!InitSocketInfo()) {
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

	CDistSlaverDlg dlg;
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

	SocketCleanSlaver();

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

VOID SetStatusSlaver(LPCTSTR sStatus) {
	if (theApp.m_pMainWnd) {
		((CDistSlaverDlg*)theApp.m_pMainWnd)->lblStatus.SetWindowText(sStatus);
	}
}

extern double MINSUP_IN_PERCENT;
extern char SEQDATA_FILENAME[];

VOID SetMasterInfo() {
	if (theApp.m_pMainWnd) {
		CString str;

		if (MINSUP_IN_PERCENT < 0) {
			str = L"N/A";
		}
		else {
			str.Format(L"%.04f", MINSUP_IN_PERCENT);
		}
		((CDistSlaverDlg*)theApp.m_pMainWnd)->txtMinSup.SetWindowText(str);

		if (strlen(SEQDATA_FILENAME) == 0) {
			str = L"N/A";
		}
		else {
			str = CA2W(SEQDATA_FILENAME);
		}
		((CDistSlaverDlg*)theApp.m_pMainWnd)->txtSeqDataFilename.SetWindowText(str);
	}
}