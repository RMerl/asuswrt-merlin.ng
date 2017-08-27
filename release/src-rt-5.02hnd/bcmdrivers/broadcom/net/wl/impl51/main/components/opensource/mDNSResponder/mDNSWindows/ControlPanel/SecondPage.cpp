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

#include "SecondPage.h"
#include "resource.h"

#include "ConfigPropertySheet.h"
#include "SharedSecret.h"

#include <WinServices.h>
    
#define MAX_KEY_LENGTH 255

IMPLEMENT_DYNCREATE(CSecondPage, CPropertyPage)


//---------------------------------------------------------------------------------------------------------------------------
//	CSecondPage::CSecondPage
//---------------------------------------------------------------------------------------------------------------------------

CSecondPage::CSecondPage()
:
	CPropertyPage(CSecondPage::IDD),
	m_setupKey( NULL )
{
	//{{AFX_DATA_INIT(CSecondPage)
	//}}AFX_DATA_INIT

	OSStatus err;

	err = RegCreateKeyEx( HKEY_LOCAL_MACHINE, kServiceParametersNode L"\\DynDNS\\Setup\\" kServiceDynDNSRegistrationDomains, 0,
	                      NULL, REG_OPTION_NON_VOLATILE, KEY_READ|KEY_WRITE|KEY_WOW64_32KEY, NULL, &m_setupKey, NULL );
	check_noerr( err );
}


//---------------------------------------------------------------------------------------------------------------------------
//	CSecondPage::~CSecondPage
//---------------------------------------------------------------------------------------------------------------------------

CSecondPage::~CSecondPage()
{
	if ( m_setupKey )
	{
		RegCloseKey( m_setupKey );
		m_setupKey = NULL;
	}
}


//---------------------------------------------------------------------------------------------------------------------------
//	CSecondPage::DoDataExchange
//---------------------------------------------------------------------------------------------------------------------------

void CSecondPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSecondPage)
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_CHECK1, m_advertiseServicesButton);
	DDX_Control(pDX, IDC_BUTTON1, m_sharedSecretButton);
	DDX_Control(pDX, IDC_COMBO2, m_regDomainsBox);
}

BEGIN_MESSAGE_MAP(CSecondPage, CPropertyPage)
	//{{AFX_MSG_MAP(CSecondPage)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedSharedSecret)
	ON_BN_CLICKED(IDC_CHECK1, OnBnClickedAdvertise)
	ON_CBN_SELCHANGE(IDC_COMBO1, OnCbnSelChange)
	ON_CBN_EDITCHANGE(IDC_COMBO1, OnCbnEditChange)
	ON_CBN_EDITCHANGE(IDC_COMBO2, OnCbnEditChange)
	ON_CBN_SELCHANGE(IDC_COMBO2, OnCbnSelChange)
	
END_MESSAGE_MAP()


//---------------------------------------------------------------------------------------------------------------------------
//	CSecondPage::SetModified
//---------------------------------------------------------------------------------------------------------------------------

void CSecondPage::SetModified( BOOL bChanged )
{
	m_modified = bChanged;

	CPropertyPage::SetModified( bChanged );
}


//---------------------------------------------------------------------------------------------------------------------------
//	CSecondPage::OnSetActive
//---------------------------------------------------------------------------------------------------------------------------

BOOL
CSecondPage::OnSetActive()
{
	CConfigPropertySheet	*	psheet;
	DWORD						err;
	BOOL						b = CPropertyPage::OnSetActive();

	psheet = reinterpret_cast<CConfigPropertySheet*>(GetParent());
	require_quiet( psheet, exit );
	
	m_modified = FALSE;

	// Clear out what's there

	EmptyComboBox( m_regDomainsBox );

	// Now populate the registration domain box

	err = Populate( m_regDomainsBox, m_setupKey, psheet->m_regDomains );
	check_noerr( err );

exit:

	return b;
}


//---------------------------------------------------------------------------------------------------------------------------
//	CSecondPage::OnOK
//---------------------------------------------------------------------------------------------------------------------------

void
CSecondPage::OnOK()
{
	if ( m_modified )
	{
		Commit();
	}
}



//---------------------------------------------------------------------------------------------------------------------------
//	CSecondPage::Commit
//---------------------------------------------------------------------------------------------------------------------------

void
CSecondPage::Commit()
{
	DWORD err;

	if ( m_setupKey != NULL )
	{
		err = Commit( m_regDomainsBox, m_setupKey, m_advertiseServicesButton.GetCheck() == BST_CHECKED );
		check_noerr( err );
	}
}


//---------------------------------------------------------------------------------------------------------------------------
//	CSecondPage::Commit
//---------------------------------------------------------------------------------------------------------------------------

OSStatus
CSecondPage::Commit( CComboBox & box, HKEY key, DWORD enabled )
{
	CString		selected;
	HKEY		subKey	= NULL;
	TCHAR		subKeyName[MAX_KEY_LENGTH];
	DWORD		cSubKeys = 0;
	DWORD		cbMaxSubKey;
	DWORD		cchMaxClass;
	DWORD		dwSize;
	int			i;
	OSStatus	err = kNoErr;

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

	// Get selected text
	
	box.GetWindowText( selected );
	
	// If we haven't seen this string before, add the string to the box and
	// the registry
	
	if ( ( selected.GetLength() > 0 ) && ( box.FindStringExact( -1, selected ) == CB_ERR ) )
	{
		CString string;

		box.AddString( selected );

		err = RegQueryString( key, L"UserDefined", string );
		check_noerr( err );

		if ( string.GetLength() )
		{
			string += L"," + selected;
		}
		else
		{
			string = selected;
		}

		err = RegSetValueEx( key, L"UserDefined", 0, REG_SZ, (LPBYTE) (LPCTSTR) string, ( string.GetLength() + 1) * sizeof( TCHAR ) );
		check_noerr ( err );
	}

	// Save selected text in registry.  This will trigger mDNSResponder to setup
	// DynDNS config again

	err = RegCreateKeyEx( key, selected, 0,
	                      NULL, REG_OPTION_NON_VOLATILE, KEY_READ|KEY_WRITE|KEY_WOW64_32KEY, NULL, &subKey, NULL );
	require_noerr( err, exit );

	err = RegSetValueEx( subKey, L"Enabled", 0, REG_DWORD, (LPBYTE) &enabled, sizeof( DWORD ) );
	check_noerr( err );

exit:

	if ( subKey )
	{
		RegCloseKey( subKey );
		subKey = NULL;
	}

	return err;
}


//---------------------------------------------------------------------------------------------------------------------------
//	CSecondPage::OnBnClickedSharedSecret
//---------------------------------------------------------------------------------------------------------------------------

void CSecondPage::OnBnClickedSharedSecret()
{
	CString name;

	m_regDomainsBox.GetWindowText( name );

	CSharedSecret dlg;

	dlg.Load( name );

	if ( dlg.DoModal() == IDOK )
	{
		DWORD		wakeup = 0;
		DWORD		dwSize = sizeof( DWORD );
		OSStatus	err;

		dlg.Commit( name );

		// We have now updated the secret, however the system service
		// doesn't know about it yet.  So we're going to update the
		// registry with a dummy value which will cause the system
		// service to re-initialize it's DynDNS setup
		//

		RegQueryValueEx( m_setupKey, L"Wakeup", NULL, NULL, (LPBYTE) &wakeup, &dwSize );      

		wakeup++;
		
		err = RegSetValueEx( m_setupKey, L"Wakeup", 0, REG_DWORD, (LPBYTE) &wakeup, sizeof( DWORD ) );
		require_noerr( err, exit );
	}

exit:

	return;
}


//---------------------------------------------------------------------------------------------------------------------------
//	CSecondPage::OnBnClickedAdvertise
//---------------------------------------------------------------------------------------------------------------------------

void CSecondPage::OnBnClickedAdvertise()
{
	int state;

	state = m_advertiseServicesButton.GetCheck();

	m_regDomainsBox.EnableWindow( state );
	m_sharedSecretButton.EnableWindow( state );

	SetModified( TRUE );
}


//---------------------------------------------------------------------------------------------------------------------------
//	CSecondPage::OnCbnSelChange
//---------------------------------------------------------------------------------------------------------------------------

void CSecondPage::OnCbnSelChange()
{
	SetModified( TRUE );
}


//---------------------------------------------------------------------------------------------------------------------------
//	CSecondPage::OnCbnEditChange
//---------------------------------------------------------------------------------------------------------------------------

void CSecondPage::OnCbnEditChange()
{
	SetModified( TRUE );
}



//---------------------------------------------------------------------------------------------------------------------------
//	CSecondPage::OnAddRegistrationDomain
//---------------------------------------------------------------------------------------------------------------------------

void
CSecondPage::OnAddRegistrationDomain( CString & domain )
{
	int index = m_regDomainsBox.FindStringExact( -1, domain );

	if ( index == CB_ERR )
	{
		m_regDomainsBox.AddString( domain );
	}
}


//---------------------------------------------------------------------------------------------------------------------------
//	CSecondPage::OnRemoveRegistrationDomain
//---------------------------------------------------------------------------------------------------------------------------

void
CSecondPage::OnRemoveRegistrationDomain( CString & domain )
{
	int index = m_regDomainsBox.FindStringExact( -1, domain );

	if ( index != CB_ERR )
	{
		m_regDomainsBox.DeleteString( index );
	}
}


//---------------------------------------------------------------------------------------------------------------------------
//	CSecondPage::EmptyComboBox
//---------------------------------------------------------------------------------------------------------------------------

void
CSecondPage::EmptyComboBox( CComboBox & box )
{
	while ( box.GetCount() > 0 )
	{
		box.DeleteString( 0 );
	}
}


//---------------------------------------------------------------------------------------------------------------------------
//	CSecondPage::Populate
//---------------------------------------------------------------------------------------------------------------------------

OSStatus
CSecondPage::Populate( CComboBox & box, HKEY key, StringList & l )
{
	CString		string;
	HKEY		subKey = NULL;
	DWORD		dwSize;
	DWORD		enabled = 0;
	TCHAR		subKeyName[MAX_KEY_LENGTH];
	DWORD		cSubKeys = 0;
	DWORD		cbMaxSubKey;
	DWORD		cchMaxClass;
	OSStatus	err;

	err = RegQueryString( key, L"UserDefined", string );

	if ( !err && string.GetLength() )
	{
		bool done = false;

		while ( !done )
		{
			CString tok;

			tok = string.SpanExcluding( L"," );

			box.AddString( tok );

			if ( tok != string )
			{
				// Get rid of that string and comma

				string = string.Right( string.GetLength() - tok.GetLength() - 1 );
			}
			else
			{
				done = true;
			}
		}
	}

	StringList::iterator it;

	for ( it = l.begin(); it != l.end(); it++ )
	{
		if ( box.FindStringExact( -1, *it ) == CB_ERR )
		{
			box.AddString( *it );
		}
	}

	err = RegQueryInfoKey( key, NULL, NULL, NULL, &cSubKeys, &cbMaxSubKey, &cchMaxClass, NULL, NULL, NULL, NULL, NULL );       
	require_noerr( err, exit );

	if ( cSubKeys > 0 )
	{	
		dwSize = MAX_KEY_LENGTH;
            
		err = RegEnumKeyEx( key, 0, subKeyName, &dwSize, NULL, NULL, NULL, NULL );
		require_noerr( err, exit );

		err = RegOpenKey( key, subKeyName, &subKey );
		require_noerr( err, exit );

		dwSize = sizeof( DWORD );
		err = RegQueryValueEx( subKey, L"Enabled", NULL, NULL, (LPBYTE) &enabled, &dwSize );
		require_noerr( err, exit );

		// See if it's there

		if ( box.SelectString( -1, subKeyName ) == CB_ERR )
		{
			// If not, add it

			box.AddString( subKeyName );
		}

		box.SelectString( -1, subKeyName );

		RegCloseKey( subKey );
		subKey = NULL;
	}

exit:

	m_advertiseServicesButton.SetCheck( ( !err && enabled ) ? BST_CHECKED : BST_UNCHECKED );
	m_regDomainsBox.EnableWindow( ( !err && enabled ) );
	m_sharedSecretButton.EnableWindow( (!err && enabled ) );

	return err;
}


//---------------------------------------------------------------------------------------------------------------------------
//	CSecondPage::CreateKey
//---------------------------------------------------------------------------------------------------------------------------

OSStatus
CSecondPage::CreateKey( CString & name, DWORD enabled )
{
	HKEY		key = NULL;
	OSStatus	err;

	err = RegCreateKeyEx( HKEY_LOCAL_MACHINE, (LPCTSTR) name, 0,
		                  NULL, REG_OPTION_NON_VOLATILE, KEY_READ|KEY_WRITE|KEY_WOW64_32KEY, NULL, &key, NULL );
	require_noerr( err, exit );

	err = RegSetValueEx( key, L"Enabled", 0, REG_DWORD, (LPBYTE) &enabled, sizeof( DWORD ) );
	check_noerr( err );

exit:

	if ( key )
	{
		RegCloseKey( key );
	}

	return err;
}


//---------------------------------------------------------------------------------------------------------------------------
//	CSecondPage::RegQueryString
//---------------------------------------------------------------------------------------------------------------------------

OSStatus
CSecondPage::RegQueryString( HKEY key, CString valueName, CString & value )
{
	TCHAR	*	string;
	DWORD		stringLen;
	int			i;
	OSStatus	err;

	stringLen	= 1024;
	string		= NULL;
	i			= 0;

	do
	{
		if ( string )
		{
			free( string );
		}

		string = (TCHAR*) malloc( stringLen );
		require_action( string, exit, err = kUnknownErr );
		*string = '\0';

		err = RegQueryValueEx( key, valueName, 0, NULL, (LPBYTE) string, &stringLen );

		i++;
	}
	while ( ( err == ERROR_MORE_DATA ) && ( i < 100 ) );

	value = string;

exit:

	if ( string )
	{
		free( string );
	}

	return err;
}
