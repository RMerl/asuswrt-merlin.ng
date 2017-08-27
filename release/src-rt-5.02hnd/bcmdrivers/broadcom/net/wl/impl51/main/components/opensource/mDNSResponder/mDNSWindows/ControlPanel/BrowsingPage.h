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
//	CBrowsingPage
//---------------------------------------------------------------------------------------------------------------------------

class CBrowsingPage : public CPropertyPage
{
public:
	CBrowsingPage();
	~CBrowsingPage();

protected:

	//{{AFX_DATA(CBrowsingPage)
	enum { IDD = IDR_APPLET_PAGE3 };
	//}}AFX_DATA

	//{{AFX_VIRTUAL(CBrowsingPage)
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	DECLARE_DYNCREATE(CBrowsingPage)

	//{{AFX_MSG(CBrowsingPage)
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

	static int CALLBACK 

	SortFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);



	CListCtrl	m_browseListCtrl;

	bool		m_initialized;

	bool		m_firstTime;



public:



	afx_msg void OnBnClickedAddBrowseDomain();

	afx_msg void OnBnClickedRemoveBrowseDomain();

	afx_msg void OnLvnItemchangedBrowseList(NMHDR *pNMHDR, LRESULT *pResult);

	CButton m_removeButton;

};





//---------------------------------------------------------------------------------------------------------------------------
//	CAddBrowseDomain
//---------------------------------------------------------------------------------------------------------------------------


class CAddBrowseDomain : public CDialog

{

	DECLARE_DYNAMIC(CAddBrowseDomain)



public:

	CAddBrowseDomain(CWnd* pParent = NULL);   // standard constructor

	virtual ~CAddBrowseDomain();



// Dialog Data

	enum { IDD = IDR_ADD_BROWSE_DOMAIN };



protected:

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	virtual BOOL OnInitDialog();

	virtual void OnOK();

	DECLARE_MESSAGE_MAP()

public:

	CComboBox	m_comboBox;

	CString		m_text;

};
