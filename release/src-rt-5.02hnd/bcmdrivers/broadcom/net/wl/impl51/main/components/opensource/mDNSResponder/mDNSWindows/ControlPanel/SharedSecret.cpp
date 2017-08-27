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

    
// SharedSecret.cpp : implementation file
//


#include <Secret.h>
#include "stdafx.h"
#include "SharedSecret.h"
#include <WinServices.h>

#include <DebugServices.h>


// SharedSecret dialog

IMPLEMENT_DYNAMIC(CSharedSecret, CDialog)


//---------------------------------------------------------------------------------------------------------------------------
//	CSharedSecret::CSharedSecret
//---------------------------------------------------------------------------------------------------------------------------

CSharedSecret::CSharedSecret(CWnd* pParent /*=NULL*/)
	: CDialog(CSharedSecret::IDD, pParent)
	, m_key(_T(""))
	, m_secret(_T(""))
{
}


//---------------------------------------------------------------------------------------------------------------------------
//	CSharedSecret::~CSharedSecret
//---------------------------------------------------------------------------------------------------------------------------

CSharedSecret::~CSharedSecret()
{
}


//---------------------------------------------------------------------------------------------------------------------------
//	CSharedSecret::DoDataExchange
//---------------------------------------------------------------------------------------------------------------------------

void CSharedSecret::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_KEY, m_key );
	DDX_Text(pDX, IDC_SECRET, m_secret );
}


BEGIN_MESSAGE_MAP(CSharedSecret, CDialog)
END_MESSAGE_MAP()


//---------------------------------------------------------------------------------------------------------------------------
//	CSharedSecret::Load
//---------------------------------------------------------------------------------------------------------------------------

void
CSharedSecret::Load( CString zone )
{
	char	zoneUTF8[ 256 ];
	char	outDomain[ 256 ];
	char	outKey[ 256 ];
	char	outSecret[ 256 ];

	StringObjectToUTF8String( zone, zoneUTF8, sizeof( zoneUTF8 ) );

	if ( LsaGetSecret( zoneUTF8, outDomain, sizeof( outDomain ) / sizeof( TCHAR ), outKey, sizeof( outKey ) / sizeof( TCHAR ), outSecret, sizeof( outSecret ) / sizeof( TCHAR ) ) )
	{
		m_key		= outKey;
		m_secret	= outSecret;
	}
	else
	{
		m_key = zone;
	}
}


//---------------------------------------------------------------------------------------------------------------------------
//	CSharedSecret::Commit
//---------------------------------------------------------------------------------------------------------------------------

void
CSharedSecret::Commit( CString zone )
{
	char	zoneUTF8[ 256 ];
	char	keyUTF8[ 256 ];
	char	secretUTF8[ 256 ];

	StringObjectToUTF8String( zone, zoneUTF8, sizeof( zoneUTF8 ) );
	StringObjectToUTF8String( m_key, keyUTF8, sizeof( keyUTF8 ) );
	StringObjectToUTF8String( m_secret, secretUTF8, sizeof( secretUTF8 ) );

	LsaSetSecret( zoneUTF8, keyUTF8, secretUTF8 );
}
