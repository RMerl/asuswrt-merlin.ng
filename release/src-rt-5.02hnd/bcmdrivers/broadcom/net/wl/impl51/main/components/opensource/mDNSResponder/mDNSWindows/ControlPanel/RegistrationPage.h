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
#include "afxwin.h"

    
//---------------------------------------------------------------------------------------------------------------------------
//	CRegistrationPage
//---------------------------------------------------------------------------------------------------------------------------

class CRegistrationPage : public CPropertyPage
{
public:
	CRegistrationPage();
	~CRegistrationPage();

protected:
	//{{AFX_DATA(CRegistrationPage)
	enum { IDD = IDR_APPLET_PAGE1 };
	//}}AFX_DATA

	//{{AFX_VIRTUAL(CRegistrationPage)
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	DECLARE_DYNCREATE(CRegistrationPage)

	//{{AFX_MSG(CRegistrationPage)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

	afx_msg BOOL	OnSetActive();
	afx_msg void	OnOK();

	void			SetModified( BOOL bChanged = TRUE );
	void			Commit();

	CEdit			m_hostnameControl;
	CEdit			m_usernameControl;
	CEdit			m_passwordControl;
	CButton			m_advertiseServices;
	bool			m_ignoreChanges;
	bool			m_modified;
	HKEY			m_hostnameSetupKey;
	HKEY			m_registrationSetupKey;
	HKEY			m_statusKey;
	
public:
	
	afx_msg void OnEnChangeHostname();
	afx_msg void OnEnChangeUsername();
	afx_msg void OnEnChangePassword();
	afx_msg void OnBnClickedAdvertiseServices();
};
