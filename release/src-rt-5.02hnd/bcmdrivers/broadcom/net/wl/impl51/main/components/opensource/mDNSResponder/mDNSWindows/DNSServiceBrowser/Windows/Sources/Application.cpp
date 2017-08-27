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

#include	<assert.h>

#include	"StdAfx.h"

#include	"DNSServices.h"

#include	"Application.h"

#include	"ChooserDialog.h"

#include	"stdafx.h"

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
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

//===========================================================================================================================
//	Globals
//===========================================================================================================================

Application		gApp;

//===========================================================================================================================
//	Application
//===========================================================================================================================

Application::Application( void )
{
	//
}

//===========================================================================================================================
//	InitInstance
//===========================================================================================================================

BOOL	Application::InitInstance()
{
	DNSStatus		err;
	
	// Standard MFC initialization.

#if( !defined( AFX_DEPRECATED ) )
	#ifdef _AFXDLL
		Enable3dControls();			// Call this when using MFC in a shared DLL
	#else
		Enable3dControlsStatic();	// Call this when linking to MFC statically
	#endif
#endif

	InitCommonControls();
	
	// Set up DNS Services.
	
	err = DNSServicesInitialize( 0, 512 );
	assert( err == kDNSNoErr );
	
	// Create the chooser dialog.
	
	ChooserDialog *		dialog;
	
	m_pMainWnd = NULL;
	dialog = new ChooserDialog;
	dialog->Create( IDD_CHOOSER_DIALOG );
	m_pMainWnd = dialog;
	dialog->ShowWindow( SW_SHOW );
	
	return( true );
}

//===========================================================================================================================
//	ExitInstance
//===========================================================================================================================

int	Application::ExitInstance( void )
{
	// Clean up DNS Services.
	
	DNSServicesFinalize();
	return( 0 );
}
