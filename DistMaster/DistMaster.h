// DistMaster.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CDistMasterApp:
// See DistMaster.cpp for the implementation of this class
//

class CDistMasterApp : public CWinApp
{
public:
	CDistMasterApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

typedef struct __SLAVER_ADDRESS {
	char strAddress[256];
	int nPort;
} SLAVER_ADDRESS;

extern CDistMasterApp theApp;

VOID SetStatusMaster(LPCTSTR sStatus);
VOID SetStatusSite(int site, LPCTSTR sStatus);
int GetTreeFilename(char* filename);
int GetSeqDataName(char* dataname);

