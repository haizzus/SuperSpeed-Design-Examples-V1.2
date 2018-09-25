
// Fx3ReceiveData.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CFx3ReceiveDataApp:
// See Fx3ReceiveData.cpp for the implementation of this class
//

class CFx3ReceiveDataApp : public CWinAppEx
{
public:
	CFx3ReceiveDataApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CFx3ReceiveDataApp theApp;