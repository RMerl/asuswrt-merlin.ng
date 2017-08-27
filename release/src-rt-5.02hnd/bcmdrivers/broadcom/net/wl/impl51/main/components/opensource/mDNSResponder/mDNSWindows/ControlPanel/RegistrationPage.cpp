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

#include <Secret.h>
#include "RegistrationPage.h"
#include "resource.h"

#include "ConfigPropertySheet.h"
extern "C"
{
#include <ClientCommon.h>
}
#include <WinServices.h>

#define MAX_KEY_LENGTH 255


IMPLEMENT_DYNCREATE(CRegistrationPage, CPropertyPage)

//---------------------------------------------------------------------------------------------------------------------------
//	CRegistrationPage::CRegistrationPage
//---------------------------------------------------------------------------------------------------------------------------

CRegistrationPage::CRegistrationPage()
:
	CPropertyPage(CRegistrationPage::IDD),
	m_ignoreChanges( false ),
	m_hostnameSetupKey( NULL ),
	m_registrationSetupKey( NULL ),
	m_statusKey( NULL )
{
	//{{AFX_DATA_INIT(CRegistrationPage)
	//}}AFX_DATA_INIT

	OSStatus err;

	err = RegCreateKeyEx( HKEY_LOCAL_MACHINE, kServiceParametersNode L"\\DynDNS\\Setup\\Hostnames", 0,
	                      NULL, REG_OPTION_NON_VOLATILE, KEY_READ|KEY_WRITE|KEY_WOW64_32KEY, NULL, &m_hostnameSetupKey, NULL );
	check_noerr( err );

	err = RegCreateKeyEx( HKEY_LOCAL_MACHINE, kServiceParametersNode L"\\DynDNS\\Setup\\" kServiceDynDNSRegistrationDomains, 0,
	                      NULL, REG_OPTION_NON_VOLATILE, KEY_READ|KEY_WRITE|KEY_WOW64_32KEY, NULL, &m_registrationSetupKey, NULL );
	check_noerr( err );

	err = RegCreateKeyEx( HKEY_LOCAL_MACHINE, kServiceParametersNode L"\\DynDNS\\State\\Hostnames", 0,
	                      NULL, REG_OPTION_NON_VOLATILE, KEY_READ|KEY_WRITE|KEY_WOW64_32KEY, NULL, &m_statusKey, NULL );
	check_noerr( err );

	
}

CRegistrationPage::~CRegistrationPage()
{
	if ( m_hostnameSetupKey )
	{
		RegCloseKey( m_hostnameSetupKey );
		m_hostnameSetupKey = NULL;
	}

	if ( m_registrationSetupKey )
	{
		RegCloseKey( m_registrationSetupKey );
		m_registrationSetupKey = NULL;
	}

	if ( m_statusKey )
	{
		RegCloseKey( m_statusKey );
		m_statusKey = NULL;
	}
}


//---------------------------------------------------------------------------------------------------------------------------
//	CRegistrationPage::DoDataExchange
//---------------------------------------------------------------------------------------------------------------------------

void CRegistrationPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRegistrationPage)
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_HOSTNAME, m_hostnameControl);
	DDX_Control(pDX, IDC_USERNAME, m_usernameControl);
	DDX_Control(pDX, IDC_PASSWORD, m_passwordControl);
	DDX_Control(pDX, IDC_ADVERTISE_SERVICES, m_advertiseServices);
}

BEGIN_MESSAGE_MAP(CRegistrationPage, CPropertyPage)
	//{{AFX_MSG_MAP(CRegistrationPage)
	//}}AFX_MSG_MAP
	ON_EN_CHANGE(IDC_HOSTNAME, OnEnChangeHostname)
	ON_EN_CHANGE(IDC_USERNAME, OnEnChangeUsername)
	ON_EN_CHANGE(IDC_PASSWORD, OnEnChangePassword)
	ON_BN_CLICKED(IDC_ADVERTISE_SERVICES, OnBnClickedAdvertiseServices)
END_MESSAGE_MAP()


//---------------------------------------------------------------------------------------------------------------------------
//	CRegistrationPage::OnEnChangedHostname
//---------------------------------------------------------------------------------------------------------------------------

void CRegistrationPage::OnEnChangeHostname()
{
	if ( !m_ignoreChanges )
	{
		SetModified( TRUE );
	}
}


//---------------------------------------------------------------------------------------------------------------------------
//	CRegistrationPage::OnEnChangedUsername
//---------------------------------------------------------------------------------------------------------------------------

void CRegistrationPage::OnEnChangeUsername()
{
	if ( !m_ignoreChanges )
	{
		SetModified( TRUE );
	}
}


//---------------------------------------------------------------------------------------------------------------------------
//	CRegistrationPage::OnEnChangedPassword
//---------------------------------------------------------------------------------------------------------------------------

void CRegistrationPage::OnEnChangePassword()
{
	if ( !m_ignoreChanges )
	{
		SetModified( TRUE );
	}
}


//---------------------------------------------------------------------------------------------------------------------------
//	CRegistrationPage::OnBnClickedAdvertiseServices
//---------------------------------------------------------------------------------------------------------------------------

void CRegistrationPage::OnBnClickedAdvertiseServices()
{
	if ( !m_ignoreChanges )
	{
		SetModified( TRUE );
	}
}


//---------------------------------------------------------------------------------------------------------------------------
//	CRegistrationPage::SetModified
//---------------------------------------------------------------------------------------------------------------------------

void CRegistrationPage::SetModified( BOOL bChanged )
{
	m_modified = bChanged ? true : false;

	CPropertyPage::SetModified( bChanged );
}


//---------------------------------------------------------------------------------------------------------------------------
//	CRegistrationPage::OnSetActive
//---------------------------------------------------------------------------------------------------------------------------

BOOL
CRegistrationPage::OnSetActive()
{
	TCHAR	name[kDNSServiceMaxDomainName + 1];
	DWORD	nameLen = ( kDNSServiceMaxDomainName + 1 ) * sizeof( TCHAR );
	DWORD	err;

	BOOL b = CPropertyPage::OnSetActive();

	m_ignoreChanges = true;
	m_modified = FALSE;
	
	if ( m_hostnameSetupKey )
	{
		err = RegQueryValueEx( m_hostnameSetupKey, L"", NULL, NULL, (LPBYTE) name, &nameLen );

		if ( !err )
		{
			char	hostnameUTF8[ 256 ];
			char	outDomain[ 256 ];
			char	outUsername[ 256 ];
			char	outPassword[ 256 ];
			CString hostname = name;
			CString username;
			CString password;

			m_hostnameControl.SetWindowText( hostname );

			StringObjectToUTF8String( hostname, hostnameUTF8, sizeof( hostnameUTF8 ) );

			if ( LsaGetSecret( hostnameUTF8, outDomain, sizeof( outDomain ) / sizeof( TCHAR ), outUsername, sizeof( outUsername ) / sizeof( TCHAR ), outPassword, sizeof( outPassword ) / sizeof( TCHAR ) ) )
			{
				username = outUsername;
				m_usernameControl.SetWindowText( username );

				password = outPassword;
				m_passwordControl.SetWindowText( password );
			}
		}
	}

	m_advertiseServices.SetCheck( 0 );

	if ( m_registrationSetupKey )
	{
		HKEY		subKey = NULL;
		DWORD		dwSize;
		DWORD		enabled = 0;
		TCHAR		subKeyName[MAX_KEY_LENGTH];
		DWORD		cSubKeys = 0;
		DWORD		cbMaxSubKey;
		DWORD		cchMaxClass;
		OSStatus	err;

		err = RegQueryInfoKey( m_registrationSetupKey, NULL, NULL, NULL, &cSubKeys, &cbMaxSubKey, &cchMaxClass, NULL, NULL, NULL, NULL, NULL );       
		if ( !err )
		{
			if ( cSubKeys > 0 )
			{	
				dwSize = MAX_KEY_LENGTH;
	            
				err = RegEnumKeyEx( m_registrationSetupKey, 0, subKeyName, &dwSize, NULL, NULL, NULL, NULL );
				if ( !err )
				{
					err = RegOpenKey( m_registrationSetupKey, subKeyName, &subKey );
					if ( !err )
					{
						dwSize = sizeof( DWORD );
						err = RegQueryValueEx( subKey, L"Enabled", NULL, NULL, (LPBYTE) &enabled, &dwSize );
						if ( !err && enabled )
						{
							m_advertiseServices.SetCheck( enabled );
						}

						RegCloseKey( subKey );
						subKey = NULL;
					}
				}
			}
		}
	}

	m_ignoreChanges = false;

	return b;
}


//---------------------------------------------------------------------------------------------------------------------------
//	CRegistrationPage::OnOK
//---------------------------------------------------------------------------------------------------------------------------

void
CRegistrationPage::OnOK()
{
	if ( m_modified )
	{
		Commit();
	}
}


//---------------------------------------------------------------------------------------------------------------------------
//	CRegistrationPage::Commit
//---------------------------------------------------------------------------------------------------------------------------

void
CRegistrationPage::Commit()
{
	CString	hostname;
	char	hostnameUTF8[ 256 ];
	CString username;
	char	usernameUTF8[ 256 ];
	CString password;
	char	passwordUTF8[ 256 ];
	DWORD	enabled = 1;
	BOOL	secret = FALSE;
	DWORD	err;

	m_hostnameControl.GetWindowText( hostname );
	hostname.MakeLower();
	hostname.TrimRight( '.' );
	StringObjectToUTF8String( hostname, hostnameUTF8, sizeof( hostnameUTF8 ) );
	
	m_usernameControl.GetWindowText( username );
	m_passwordControl.GetWindowText( password );
	
	if ( username.GetLength() && password.GetLength() )
	{
		StringObjectToUTF8String( username, usernameUTF8, sizeof( usernameUTF8 ) );	
		StringObjectToUTF8String( password, passwordUTF8, sizeof( passwordUTF8 ) );
		secret = TRUE;
	}

	if ( m_hostnameSetupKey != NULL )
	{
		err = RegSetValueEx( m_hostnameSetupKey, L"", 0, REG_SZ, (LPBYTE) (LPCTSTR) hostname, ( hostname.GetLength() + 1 ) * sizeof( TCHAR ) );
		require_noerr( err, exit );
		
		err = RegSetValueEx( m_hostnameSetupKey, L"Enabled", 0, REG_DWORD, (LPBYTE) &enabled, sizeof( DWORD ) );
		require_noerr( err, exit );

		if ( secret )
		{
			LsaSetSecret( hostnameUTF8, usernameUTF8, passwordUTF8 );
		}
	}

	if ( m_registrationSetupKey != NULL )
	{
		TCHAR		subKeyName[MAX_KEY_LENGTH];
		DWORD		cSubKeys = 0;
		DWORD		cbMaxSubKey;
		DWORD		cchMaxClass;
		DWORD		dwSize;
		int			i;
		OSStatus	err = kNoErr;

		// First, remove all the entries that are there

		err = RegQueryInfoKey( m_registrationSetupKey, NULL, NULL, NULL, &cSubKeys, &cbMaxSubKey, &cchMaxClass, NULL, NULL, NULL, NULL, NULL );       
		if ( !err )
		{
			for ( i = 0; i < (int) cSubKeys; i++ )
			{	
				dwSize = MAX_KEY_LENGTH;
		            
				err = RegEnumKeyEx( m_registrationSetupKey, 0, subKeyName, &dwSize, NULL, NULL, NULL, NULL );
				require_noerr( err, exit );
					
				err = RegDeleteKey( m_registrationSetupKey, subKeyName );
				require_noerr( err, exit );
			}
		}

		if ( m_advertiseServices.GetCheck() )
		{
			const char	* domainUTF8;
			CString		  domain;
			char		  label[ 64 ];
			HKEY		  subKey = NULL;

			domainUTF8	= GetNextLabel( hostnameUTF8, label );
			domain		= domainUTF8;

			err = RegCreateKeyEx( m_registrationSetupKey, domain, 0,
									 NULL, REG_OPTION_NON_VOLATILE, KEY_READ|KEY_WRITE|KEY_WOW64_32KEY, NULL, &subKey, NULL );
			if ( !err )
			{
				err = RegSetValueEx( subKey, L"Enabled", 0, REG_DWORD, (LPBYTE) &enabled, sizeof( DWORD ) );
				check_noerr( err );

				RegCloseKey( subKey );
				subKey = NULL;
			}

			if ( secret )
			{
				LsaSetSecret( domainUTF8, usernameUTF8, passwordUTF8 );
			}
		}
	}

exit:

	return;
}
