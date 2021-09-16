
// interop2.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols
#include "upload.h"
#include "thread.h"


// Cinterop2App:
// See interop2.cpp for the implementation of this class
//

class Cinterop2App : public CWinApp
{
public:
	Cinterop2App();

// Overrides
public:
	virtual BOOL InitInstance();

	DWORD getthreadid( void ) const { return m_dwThreadID; }
	std::shared_ptr<const scheduler> getscheduler( void ) const { return m_spGlobalScheduler; }

// Implementation

	DECLARE_MESSAGE_MAP()

protected:
	DWORD m_dwThreadID;
	std::shared_ptr<globalcurl> m_spGlobalCurl;
	std::shared_ptr<scheduler> m_spGlobalScheduler;
};

extern Cinterop2App theApp;
