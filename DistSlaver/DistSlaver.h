// DistSlaver.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CDistSlaverApp:
// See DistSlaver.cpp for the implementation of this class
//

class CDistSlaverApp : public CWinApp
{
public:
	CDistSlaverApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CDistSlaverApp theApp;
extern CString strCaption;
VOID SetStatusSlaver(LPCTSTR sStatus);
VOID SetMasterInfo();