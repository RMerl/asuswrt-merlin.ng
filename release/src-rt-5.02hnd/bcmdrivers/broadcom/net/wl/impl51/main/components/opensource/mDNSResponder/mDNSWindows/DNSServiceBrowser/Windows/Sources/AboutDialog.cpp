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

#include	<stdlib.h>

#include	"stdafx.h"

#include	"AboutDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//===========================================================================================================================
//	Message Map
//===========================================================================================================================

BEGIN_MESSAGE_MAP(AboutDialog, CDialog)
	//{{AFX_MSG_MAP(AboutDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//===========================================================================================================================
//	AboutDialog
//===========================================================================================================================

AboutDialog::AboutDialog(CWnd* pParent /*=NULL*/)
	: CDialog(AboutDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(AboutDialog)
		// Note: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

//===========================================================================================================================
//	OnInitDialog
//===========================================================================================================================

BOOL	AboutDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	return( true );
}

//===========================================================================================================================
//	DoDataExchange
//===========================================================================================================================

void	AboutDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(AboutDialog)
		// Note: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}
