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

#if !defined(AFX_ABOUTDIALOG_H__4B8A04B2_9735_4F4A_AFCA_15F85FB3D763__INCLUDED_)
#define AFX_ABOUTDIALOG_H__4B8A04B2_9735_4F4A_AFCA_15F85FB3D763__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include	"Resource.h"

//===========================================================================================================================
//	AboutDialog
//===========================================================================================================================

class	AboutDialog : public CDialog
{
	public:
		
		// Creation/Deletion
		
		AboutDialog(CWnd* pParent = NULL);   // standard constructor
		
		//{{AFX_DATA(AboutDialog)
		enum { IDD = IDD_ABOUT_DIALOG };
			// Note: the ClassWizard will add data members here
		//}}AFX_DATA
		
		// ClassWizard generated virtual function overrides
		//{{AFX_VIRTUAL(AboutDialog)
		protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		//}}AFX_VIRTUAL

	protected:

		// Generated message map functions
		//{{AFX_MSG(AboutDialog)
		virtual BOOL OnInitDialog();
		//}}AFX_MSG
		DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ABOUTDIALOG_H__4B8A04B2_9735_4F4A_AFCA_15F85FB3D763__INCLUDED_)
