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

#include "ServicesPage.h"
#include "resource.h"

#include "ControlPanelExe.h"
#include "ConfigPropertySheet.h"

#include <WinServices.h>
    
#define MAX_KEY_LENGTH 255


IMPLEMENT_DYNCREATE(CServicesPage, CPropertyPage)


//---------------------------------------------------------------------------------------------------------------------------
//	CServicesPage::CServicesPage
//---------------------------------------------------------------------------------------------------------------------------

CServicesPage::CServicesPage()
:
	CPropertyPage(CServicesPage::IDD)
{
	//{{AFX_DATA_INIT(CServicesPage)
	//}}AFX_DATA_INIT
}


//---------------------------------------------------------------------------------------------------------------------------
//	CServicesPage::~CServicesPage
//---------------------------------------------------------------------------------------------------------------------------

CServicesPage::~CServicesPage()
{
}


//---------------------------------------------------------------------------------------------------------------------------
//	CServicesPage::DoDataExchange
//---------------------------------------------------------------------------------------------------------------------------

void CServicesPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CServicesPage)
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_ADVERTISE_SMB, m_SMBCheckBox);
	DDX_Control(pDX, IDC_POWER_MANAGEMENT, m_powerManagementCheckBox);
}

BEGIN_MESSAGE_MAP(CServicesPage, CPropertyPage)
	//{{AFX_MSG_MAP(CServicesPage)
	//}}AFX_MSG_MAP

	ON_BN_CLICKED(IDC_ADVERTISE_SMB, &CServicesPage::OnBnClickedAdvertiseSMB)
	ON_BN_CLICKED(IDC_POWER_MANAGEMENT, &CServicesPage::OnBnClickedPowerManagement)

END_MESSAGE_MAP()


//---------------------------------------------------------------------------------------------------------------------------
//	CServicesPage::SetModified
//---------------------------------------------------------------------------------------------------------------------------

void CServicesPage::SetModified( BOOL bChanged )
{
	m_modified = bChanged;

	CPropertyPage::SetModified( bChanged );
}


//---------------------------------------------------------------------------------------------------------------------------
//	CServicesPage::OnSetActive
//---------------------------------------------------------------------------------------------------------------------------

BOOL
CServicesPage::OnSetActive()
{
	CConfigPropertySheet	*	psheet;
	HKEY						key = NULL;
	DWORD						dwSize;
	DWORD						enabled;
	DWORD						err;
	BOOL						b = CPropertyPage::OnSetActive();

	psheet = reinterpret_cast<CConfigPropertySheet*>(GetParent());
	require_quiet( psheet, exit );

	m_SMBCheckBox.SetCheck( 0 );

	// Now populate the browse domain box

	err = RegCreateKeyEx( HKEY_LOCAL_MACHINE, kServiceParametersNode L"\\Services\\SMB", 0,
		                  NULL, REG_OPTION_NON_VOLATILE, KEY_READ|KEY_WRITE|KEY_WOW64_32KEY, NULL, &key, NULL );
	require_noerr( err, exit );

	dwSize = sizeof( DWORD );
	err = RegQueryValueEx( key, L"Advertise", NULL, NULL, (LPBYTE) &enabled, &dwSize );
	require_noerr( err, exit );

	m_SMBCheckBox.SetCheck( enabled );

	RegCloseKey( key );
	key = NULL;

	m_powerManagementCheckBox.SetCheck( 0 );

	// Now populate the browse domain box

	err = RegCreateKeyEx( HKEY_LOCAL_MACHINE, kServiceParametersNode L"\\Power Management", 0,
		                  NULL, REG_OPTION_NON_VOLATILE, KEY_READ|KEY_WRITE|KEY_WOW64_32KEY, NULL, &key, NULL );
	require_noerr( err, exit );

	dwSize = sizeof( DWORD );
	err = RegQueryValueEx( key, L"Enabled", NULL, NULL, (LPBYTE) &enabled, &dwSize );
	require_noerr( err, exit );

	m_powerManagementCheckBox.SetCheck( enabled );
 
exit:

	if ( key )
	{
		RegCloseKey( key );
	}

	return b;
}


//---------------------------------------------------------------------------------------------------------------------------
//	CServicesPage::OnOK
//---------------------------------------------------------------------------------------------------------------------------

void
CServicesPage::OnOK()
{
	if ( m_modified )
	{
		Commit();
	}
}



//---------------------------------------------------------------------------------------------------------------------------
//	CServicesPage::Commit
//---------------------------------------------------------------------------------------------------------------------------

void
CServicesPage::Commit()
{
	HKEY		key		= NULL;
	DWORD		enabled;
	DWORD		err;

	err = RegCreateKeyEx( HKEY_LOCAL_MACHINE, kServiceParametersNode L"\\Services\\SMB", 0,
	                   	NULL, REG_OPTION_NON_VOLATILE, KEY_READ|KEY_WRITE|KEY_WOW64_32KEY, NULL, &key, NULL );
	require_noerr( err, exit );

	enabled = m_SMBCheckBox.GetCheck();
	err = RegSetValueEx( key, L"Advertise", NULL, REG_DWORD, (LPBYTE) &enabled, sizeof( enabled ) );
	require_noerr( err, exit );

	RegCloseKey( key );
	key = NULL;

	err = RegCreateKeyEx( HKEY_LOCAL_MACHINE, kServiceParametersNode L"\\Power Management", 0,
		                  NULL, REG_OPTION_NON_VOLATILE, KEY_READ|KEY_WRITE|KEY_WOW64_32KEY, NULL, &key, NULL );
	require_noerr( err, exit );

	enabled = m_powerManagementCheckBox.GetCheck();
	err = RegSetValueEx( key, L"Enabled", NULL, REG_DWORD, (LPBYTE) &enabled, sizeof( enabled ) );
	require_noerr( err, exit );
	
exit:

	if ( key )
	{
		RegCloseKey( key );
	}
}


//---------------------------------------------------------------------------------------------------------------------------
//	CServicesPage::OnBnClickedAdvertiseSMB
//---------------------------------------------------------------------------------------------------------------------------

void CServicesPage::OnBnClickedAdvertiseSMB()
{
	SetModified( TRUE );
}


//---------------------------------------------------------------------------------------------------------------------------
//	CServicesPage::OnBnClickedPowerManagement
//---------------------------------------------------------------------------------------------------------------------------

void CServicesPage::OnBnClickedPowerManagement()
{
	SetModified( TRUE );

	if ( m_powerManagementCheckBox.GetCheck() )
	{
		CPowerManagementWarning dlg( GetParent() );

		dlg.DoModal();
	}
}


// CPowerManagementWarning dialog

IMPLEMENT_DYNAMIC(CPowerManagementWarning, CDialog)
CPowerManagementWarning::CPowerManagementWarning(CWnd* pParent /*=NULL*/)
	: CDialog(CPowerManagementWarning::IDD, pParent)
{
}

CPowerManagementWarning::~CPowerManagementWarning()
{
}

void CPowerManagementWarning::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ENERGY_SAVER, m_energySaverIcon);
}


BOOL
CPowerManagementWarning::OnInitDialog()
{	
	BOOL b = CDialog::OnInitDialog();

	const HICON hIcon = ( HICON ) ::LoadImage( GetNonLocalizedResources(), MAKEINTRESOURCE( IDI_ENERGY_SAVER ), IMAGE_ICON, 0, 0, 0);
	
	if ( hIcon )
	{
		m_energySaverIcon.SetIcon( hIcon );
	}

	return b;
}


void
CPowerManagementWarning::OnOK()
{
	CDialog::OnOK();
}


BEGIN_MESSAGE_MAP(CPowerManagementWarning, CDialog)
END_MESSAGE_MAP()
