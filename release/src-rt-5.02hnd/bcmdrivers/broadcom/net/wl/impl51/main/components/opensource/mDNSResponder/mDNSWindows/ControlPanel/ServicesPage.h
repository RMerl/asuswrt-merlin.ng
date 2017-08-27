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

#pragma once

#include "stdafx.h"
#include "resource.h"

#include <DebugServices.h>
#include <list>
#include "afxcmn.h"

#include "afxwin.h"





//---------------------------------------------------------------------------------------------------------------------------
//	CServicesPage
//---------------------------------------------------------------------------------------------------------------------------

class CServicesPage : public CPropertyPage
{
public:
	CServicesPage();
	~CServicesPage();

protected:

	//{{AFX_DATA(CServicesPage)
	enum { IDD = IDR_APPLET_PAGE5 };
	//}}AFX_DATA

	//{{AFX_VIRTUAL(CServicesPage)
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	DECLARE_DYNCREATE(CServicesPage)

	//{{AFX_MSG(CServicesPage)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	
private:
	
	typedef std::list<CString> StringList;

	afx_msg BOOL
	OnSetActive();
	
	afx_msg void
	OnOK();
	
	void
	SetModified( BOOL bChanged = TRUE );
	
	void
	Commit();

	BOOL			m_modified;

public:
private:

	CButton m_SMBCheckBox;
	CButton m_powerManagementCheckBox;

public:


	afx_msg void OnBnClickedAdvertiseSMB();
	afx_msg void OnBnClickedPowerManagement();
};


//---------------------------------------------------------------------------------------------------------------------------
//	CPowerManagementWarning
//---------------------------------------------------------------------------------------------------------------------------

class CPowerManagementWarning : public CDialog
{
	DECLARE_DYNAMIC(CPowerManagementWarning)

public:

	CPowerManagementWarning(CWnd* pParent = NULL);   // standard constructor

	virtual ~CPowerManagementWarning();

// Dialog Data

	enum { IDD = IDR_POWER_MANAGEMENT_WARNING };

protected:

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	virtual BOOL OnInitDialog();

	virtual void OnOK();

	DECLARE_MESSAGE_MAP()

public:

	CStatic m_energySaverIcon;
};
