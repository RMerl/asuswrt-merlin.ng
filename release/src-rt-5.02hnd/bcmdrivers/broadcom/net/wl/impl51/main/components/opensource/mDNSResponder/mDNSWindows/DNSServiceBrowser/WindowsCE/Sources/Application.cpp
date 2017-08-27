/* -*- Mode: C; tab-width: 4 -*-
 *
 * Copyright (c) 2002-2004 Apple Computer, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include	"stdafx.h"

#include	"DNSServices.h"

#include	"BrowserDialog.h"

#include	"Application.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//===========================================================================================================================
//	Message Map
//===========================================================================================================================

BEGIN_MESSAGE_MAP(Application, CWinApp)
	//{{AFX_MSG_MAP(Application)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//===========================================================================================================================
//	Globals
//===========================================================================================================================

Application		gApp;

//===========================================================================================================================
//	Application
//===========================================================================================================================

Application::Application()
	: CWinApp()
{
	//
}

//===========================================================================================================================
//	InitInstance
//===========================================================================================================================

BOOL Application::InitInstance()
{
	DNSStatus			err;
	BrowserDialog		dialog;
	BOOL				dnsInitialized;
	
	dnsInitialized = FALSE;
	
	err = DNSServicesInitialize( kDNSFlagAdvertise, 0 );
	if( err )
	{
		AfxMessageBox( IDP_SOCKETS_INIT_FAILED );
		goto exit;
	}
	dnsInitialized = TRUE;

	// Display the main browser dialog.
	
	m_pMainWnd = &dialog;
	dialog.DoModal();

	// Dialog has been closed. Return false to exit the app and not start the app's message pump.

exit:
	if( dnsInitialized )
	{
		DNSServicesFinalize();
	}
	return( FALSE );
}
