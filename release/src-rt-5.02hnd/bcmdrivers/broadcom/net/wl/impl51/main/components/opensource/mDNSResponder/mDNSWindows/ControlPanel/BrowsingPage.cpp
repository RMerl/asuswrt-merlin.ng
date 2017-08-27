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

#include "BrowsingPage.h"
#include "resource.h"

#include "ConfigPropertySheet.h"

#include <WinServices.h>
    
#define MAX_KEY_LENGTH 255


IMPLEMENT_DYNCREATE(CBrowsingPage, CPropertyPage)


//---------------------------------------------------------------------------------------------------------------------------
//	CBrowsingPage::CBrowsingPage
//---------------------------------------------------------------------------------------------------------------------------

CBrowsingPage::CBrowsingPage()
:
	CPropertyPage(CBrowsingPage::IDD)
{
	//{{AFX_DATA_INIT(CBrowsingPage)
	//}}AFX_DATA_INIT

	m_firstTime = true;
}


//---------------------------------------------------------------------------------------------------------------------------
//	CBrowsingPage::~CBrowsingPage
//---------------------------------------------------------------------------------------------------------------------------

CBrowsingPage::~CBrowsingPage()
{
}


//---------------------------------------------------------------------------------------------------------------------------
//	CBrowsingPage::DoDataExchange
//---------------------------------------------------------------------------------------------------------------------------

void CBrowsingPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBrowsingPage)
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_BROWSE_LIST, m_browseListCtrl);
	DDX_Control(pDX, IDC_REMOVE_BROWSE_DOMAIN, m_removeButton);
}

BEGIN_MESSAGE_MAP(CBrowsingPage, CPropertyPage)
	//{{AFX_MSG_MAP(CBrowsingPage)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_ADD_BROWSE_DOMAIN, OnBnClickedAddBrowseDomain)
	ON_BN_CLICKED(IDC_REMOVE_BROWSE_DOMAIN, OnBnClickedRemoveBrowseDomain)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_BROWSE_LIST, OnLvnItemchangedBrowseList)
END_MESSAGE_MAP()


//---------------------------------------------------------------------------------------------------------------------------
//	CBrowsingPage::SetModified
//---------------------------------------------------------------------------------------------------------------------------

void CBrowsingPage::SetModified( BOOL bChanged )
{
	m_modified = bChanged;

	CPropertyPage::SetModified( bChanged );
}


//---------------------------------------------------------------------------------------------------------------------------
//	CBrowsingPage::OnSetActive
//---------------------------------------------------------------------------------------------------------------------------

BOOL
CBrowsingPage::OnSetActive()
{
	CConfigPropertySheet	*	psheet;
	HKEY						key = NULL;
	HKEY						subKey = NULL;
	DWORD						dwSize;
	DWORD						enabled;
	DWORD						err;
	TCHAR						subKeyName[MAX_KEY_LENGTH];
	DWORD						cSubKeys = 0;
	DWORD						cbMaxSubKey;
	DWORD						cchMaxClass;
	int							nIndex;
    DWORD						i; 
	BOOL						b = CPropertyPage::OnSetActive();

	psheet = reinterpret_cast<CConfigPropertySheet*>(GetParent());
	require_quiet( psheet, exit );
	
	m_modified = FALSE;

	if ( m_firstTime )
	{
		m_browseListCtrl.SetExtendedStyle((m_browseListCtrl.GetStyle() & (~LVS_EX_GRIDLINES))|LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);

		m_browseListCtrl.InsertColumn(0, L"", LVCFMT_LEFT, 20 );
		m_browseListCtrl.InsertColumn(1, L"", LVCFMT_LEFT, 345);

		m_firstTime = false;
	}

	m_initialized = false;

	// Clear out what's there

	m_browseListCtrl.DeleteAllItems();

	// Now populate the browse domain box

	err = RegCreateKeyEx( HKEY_LOCAL_MACHINE, kServiceParametersNode L"\\DynDNS\\Setup\\" kServiceDynDNSBrowseDomains, 0,
		                  NULL, REG_OPTION_NON_VOLATILE, KEY_READ|KEY_WRITE|KEY_WOW64_32KEY, NULL, &key, NULL );
	require_noerr( err, exit );

	// Get information about this node

    err = RegQueryInfoKey( key, NULL, NULL, NULL, &cSubKeys, &cbMaxSubKey, &cchMaxClass, NULL, NULL, NULL, NULL, NULL );       
	require_noerr( err, exit );

	for ( i = 0; i < cSubKeys; i++)
	{	
		dwSize = MAX_KEY_LENGTH;
            
		err = RegEnumKeyEx( key, i, subKeyName, &dwSize, NULL, NULL, NULL, NULL );
		require_noerr( err, exit );

		err = RegOpenKey( key, subKeyName, &subKey );
		require_noerr( err, exit );

		dwSize = sizeof( DWORD );
		err = RegQueryValueEx( subKey, L"Enabled", NULL, NULL, (LPBYTE) &enabled, &dwSize );
		require_noerr( err, exit );

		nIndex = m_browseListCtrl.InsertItem( m_browseListCtrl.GetItemCount(), L"");
		m_browseListCtrl.SetItemText( nIndex, 1, subKeyName );
		m_browseListCtrl.SetCheck( nIndex, enabled );

		RegCloseKey( subKey );
		subKey = NULL;
    }

	m_browseListCtrl.SortItems( SortFunc, (DWORD_PTR) this );

	m_removeButton.EnableWindow( FALSE );
 
exit:

	if ( subKey )
	{
		RegCloseKey( subKey );
	}

	if ( key )
	{
		RegCloseKey( key );
	}

	m_initialized = true;

	return b;
}

 

//---------------------------------------------------------------------------------------------------------------------------
//	CBrowsingPage::OnOK
//---------------------------------------------------------------------------------------------------------------------------

void
CBrowsingPage::OnOK()
{
	if ( m_modified )
	{
		Commit();
	}
}



//---------------------------------------------------------------------------------------------------------------------------
//	CBrowsingPage::Commit
//---------------------------------------------------------------------------------------------------------------------------

void
CBrowsingPage::Commit()
{
	HKEY		key		= NULL;
	HKEY		subKey	= NULL;
	TCHAR		subKeyName[MAX_KEY_LENGTH];
	DWORD		cSubKeys = 0;
	DWORD		cbMaxSubKey;
	DWORD		cchMaxClass;
	DWORD		dwSize;
	int			i;
	DWORD		err;

	err = RegCreateKeyEx( HKEY_LOCAL_MACHINE, kServiceParametersNode L"\\DynDNS\\Setup\\" kServiceDynDNSBrowseDomains, 0,
	                      NULL, REG_OPTION_NON_VOLATILE, KEY_READ|KEY_WRITE|KEY_WOW64_32KEY, NULL, &key, NULL );
	require_noerr( err, exit );

	// First, remove all the entries that are there

    err = RegQueryInfoKey( key, NULL, NULL, NULL, &cSubKeys, &cbMaxSubKey, &cchMaxClass, NULL, NULL, NULL, NULL, NULL );       
	require_noerr( err, exit );

	for ( i = 0; i < (int) cSubKeys; i++ )
	{	
		dwSize = MAX_KEY_LENGTH;
            
		err = RegEnumKeyEx( key, 0, subKeyName, &dwSize, NULL, NULL, NULL, NULL );
		require_noerr( err, exit );
			
		err = RegDeleteKey( key, subKeyName );
		require_noerr( err, exit );
	}

	// Now re-populate

	for ( i = 0; i < m_browseListCtrl.GetItemCount(); i++ )
	{
		DWORD enabled = (DWORD) m_browseListCtrl.GetCheck( i );

		err = RegCreateKeyEx( key, m_browseListCtrl.GetItemText( i, 1 ), 0,
		                      NULL, REG_OPTION_NON_VOLATILE, KEY_READ|KEY_WRITE|KEY_WOW64_32KEY, NULL, &subKey, NULL );
		require_noerr( err, exit );

		err = RegSetValueEx( subKey, L"Enabled", NULL, REG_DWORD, (LPBYTE) &enabled, sizeof( enabled ) );
		require_noerr( err, exit );

		RegCloseKey( subKey );
		subKey = NULL;
	}
	
exit:

	if ( subKey )
	{
		RegCloseKey( subKey );
	}

	if ( key )
	{
		RegCloseKey( key );
	}
}



//---------------------------------------------------------------------------------------------------------------------------
//	CBrowsingPage::OnBnClickedAddBrowseDomain
//---------------------------------------------------------------------------------------------------------------------------

void
CBrowsingPage::OnBnClickedAddBrowseDomain()
{
	CAddBrowseDomain dlg( GetParent() );

	if ( ( dlg.DoModal() == IDOK ) && ( dlg.m_text.GetLength() > 0 ) )
	{
		int nIndex;

		nIndex = m_browseListCtrl.InsertItem( m_browseListCtrl.GetItemCount(), L"");
		m_browseListCtrl.SetItemText( nIndex, 1, dlg.m_text );
		m_browseListCtrl.SetCheck( nIndex, 1 );

		m_browseListCtrl.SortItems( SortFunc, (DWORD_PTR) this );

		m_browseListCtrl.Invalidate();

		SetModified( TRUE );
	}
}


//---------------------------------------------------------------------------------------------------------------------------
//	CBrowsingPage::OnBnClickedRemoveBrowseDomain
//---------------------------------------------------------------------------------------------------------------------------

void
CBrowsingPage::OnBnClickedRemoveBrowseDomain()
{
	UINT	selectedCount = m_browseListCtrl.GetSelectedCount();
	int		nItem = -1;
	UINT	i;

	// Update all of the selected items.

	for ( i = 0; i < selectedCount; i++ )
	{
		nItem = m_browseListCtrl.GetNextItem( -1, LVNI_SELECTED );
		check( nItem != -1 );

		m_browseListCtrl.DeleteItem( nItem );

		SetModified( TRUE );
	}

	m_removeButton.EnableWindow( FALSE );
}


void
CBrowsingPage::OnLvnItemchangedBrowseList(NMHDR *pNMHDR, LRESULT *pResult)
{
	if ( m_browseListCtrl.GetSelectedCount() )
	{
		m_removeButton.EnableWindow( TRUE );
	}

	if ( m_initialized )
	{
		NM_LISTVIEW * pNMListView = (NM_LISTVIEW*)pNMHDR; 
	 
		BOOL bPrevState = (BOOL) ( ( ( pNMListView->uOldState & LVIS_STATEIMAGEMASK ) >> 12 ) - 1 ); 
	 
		if ( bPrevState < 0 )
		{
			bPrevState = 0;
		}


		BOOL bChecked = ( BOOL ) ( ( ( pNMListView->uNewState & LVIS_STATEIMAGEMASK ) >> 12) - 1 ); 
	 
		if ( bChecked < 0 )
		{
			bChecked = 0;
		}

		if ( bPrevState != bChecked )
		{
			SetModified( TRUE );
		}
	}

	*pResult = 0;
}



int CALLBACK 
CBrowsingPage::SortFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	CString str1;
	CString	str2;
	int		ret = 0;

	CBrowsingPage * self = reinterpret_cast<CBrowsingPage*>( lParamSort );
	require_quiet( self, exit );

	str1 = self->m_browseListCtrl.GetItemText( (int) lParam1, 1 );
	str2 = self->m_browseListCtrl.GetItemText( (int) lParam2, 1 );

	ret = str1.Compare( str2 );

exit:

	return ret;
}


// CAddBrowseDomain dialog

IMPLEMENT_DYNAMIC(CAddBrowseDomain, CDialog)
CAddBrowseDomain::CAddBrowseDomain(CWnd* pParent /*=NULL*/)
	: CDialog(CAddBrowseDomain::IDD, pParent)
{
}

CAddBrowseDomain::~CAddBrowseDomain()
{
}

void CAddBrowseDomain::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, m_comboBox);
}


BOOL
CAddBrowseDomain::OnInitDialog()
{
	CConfigPropertySheet	*	psheet;
	CConfigPropertySheet::StringList::iterator		it;
	
	BOOL b = CDialog::OnInitDialog();

	psheet = reinterpret_cast<CConfigPropertySheet*>(GetParent());
	require_quiet( psheet, exit );

	for ( it = psheet->m_browseDomains.begin(); it != psheet->m_browseDomains.end(); it++ )
	{
		CString text = *it;

		if ( m_comboBox.FindStringExact( -1, *it ) == CB_ERR )
		{
			m_comboBox.AddString( *it );
		}
	}

exit:

	return b;
}


void
CAddBrowseDomain::OnOK()
{
	m_comboBox.GetWindowText( m_text );

	CDialog::OnOK();
}



BEGIN_MESSAGE_MAP(CAddBrowseDomain, CDialog)
END_MESSAGE_MAP()
