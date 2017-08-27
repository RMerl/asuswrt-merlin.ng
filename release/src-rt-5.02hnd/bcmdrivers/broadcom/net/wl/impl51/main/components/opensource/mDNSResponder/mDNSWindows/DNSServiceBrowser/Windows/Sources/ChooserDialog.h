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

#if !defined(AFX_CHOOSERDIALOG_H__AC258704_B307_4901_9F98_A0AC022FD8AC__INCLUDED_)
#define AFX_CHOOSERDIALOG_H__AC258704_B307_4901_9F98_A0AC022FD8AC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include	<string>
#include	<vector>

#include	"afxcmn.h"

#include	"Resource.h"

#include	"DNSServices.h"

//===========================================================================================================================
//	Structures
//===========================================================================================================================

struct	ServiceInstanceInfo
{
	std::string		name;
	std::string		type;
	std::string		domain;
	std::string		ip;
	std::string		text;
	std::string		ifIP;
	std::string		hostName;
};

struct	ServiceTypeInfo
{
	std::string		serviceType;
	std::string		description;
	std::string		urlScheme;
};

//===========================================================================================================================
//	ChooserDialog
//===========================================================================================================================

class	ChooserDialog : public CDialog
{
	public:

		ChooserDialog(CWnd* pParent = NULL);
		virtual	~ChooserDialog( void );
		
		//{{AFX_DATA(ChooserDialog)
		enum { IDD = IDD_CHOOSER_DIALOG };
		CListCtrl mServiceList;
		CListCtrl mDomainList;
		CListCtrl mChooserList;
		//}}AFX_DATA

		// ClassWizard generated virtual function overrides
		//{{AFX_VIRTUAL(ChooserDialog)
		public:
		virtual BOOL PreTranslateMessage(MSG* pMsg);
		protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		virtual void PostNcDestroy();
		//}}AFX_VIRTUAL

	protected:
		
		typedef std::vector < ServiceInstanceInfo >		ServiceInstanceVector;
		typedef std::vector < ServiceTypeInfo >			ServiceTypeVector;
		
		HACCEL						mMenuAcceleratorTable;
		DNSBrowserRef				mBrowser;
		BOOL						mIsServiceBrowsing;
		ServiceInstanceVector		mServiceInstances;
		ServiceTypeVector			mServiceTypes;
		
	public:

		void	PopulateServicesList( void );
		void	UpdateInfoDisplay( void );
		
		void	StartBrowsing( const char *inType, const char *inDomain );
		void	StopBrowsing( void );

	protected:

		//{{AFX_MSG(ChooserDialog)
		virtual BOOL OnInitDialog();
		afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
		afx_msg void OnDomainListChanged(NMHDR* pNMHDR, LRESULT* pResult);
		afx_msg void OnServiceListChanged(NMHDR* pNMHDR, LRESULT* pResult);
		afx_msg void OnChooserListChanged(NMHDR* pNMHDR, LRESULT* pResult);
		afx_msg void OnChooserListDoubleClick(NMHDR* pNMHDR, LRESULT* pResult);
		afx_msg void OnAbout();
		afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
		afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
		afx_msg void OnFileClose();
		virtual void OnCancel();
		afx_msg void OnExit();
		afx_msg void OnClose();
		afx_msg void OnNcDestroy();
		//}}AFX_MSG
		afx_msg LONG OnDomainAdd( WPARAM inWParam, LPARAM inLParam );
		afx_msg LONG OnDomainRemove( WPARAM inWParam, LPARAM inLParam );
		afx_msg LONG OnServiceAdd( WPARAM inWParam, LPARAM inLParam );
		afx_msg LONG OnServiceRemove( WPARAM inWParam, LPARAM inLParam );
		afx_msg LONG OnResolve( WPARAM inWParam, LPARAM inLParam );
		DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHOOSERDIALOG_H__AC258704_B307_4901_9F98_A0AC022FD8AC__INCLUDED_)
