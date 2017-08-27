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

#include "ConfigPropertySheet.h"
#include <WinServices.h>
extern "C"
{
#include <ClientCommon.h>
}
#include <process.h>

// Custom events

#define WM_DATAREADY		( WM_USER + 0x100 )


IMPLEMENT_DYNCREATE(CConfigPropertySheet, CPropertySheet)


//---------------------------------------------------------------------------------------------------------------------------
//	CConfigPropertySheet::CConfigPropertySheet
//---------------------------------------------------------------------------------------------------------------------------

CConfigPropertySheet::CConfigPropertySheet()
:
	CPropertySheet(),
	m_browseDomainsRef( NULL ),
	m_thread( NULL ),
	m_threadExited( NULL )
{
	AddPage(&m_firstPage );
	AddPage(&m_secondPage);
	AddPage(&m_thirdPage);

	InitializeCriticalSection( &m_lock );
}


//---------------------------------------------------------------------------------------------------------------------------
//	CConfigPropertySheet::~CConfigPropertySheet
//---------------------------------------------------------------------------------------------------------------------------

CConfigPropertySheet::~CConfigPropertySheet()
{
	DeleteCriticalSection( &m_lock );
}


BEGIN_MESSAGE_MAP(CConfigPropertySheet, CPropertySheet)
	//{{AFX_MSG_MAP(CConfigPropertySheet)
	//}}AFX_MSG_MAP
	ON_MESSAGE( WM_DATAREADY, OnDataReady )
END_MESSAGE_MAP()


//---------------------------------------------------------------------------------------------------------------------------
//	CConfigPropertySheet::OnInitDialog
//---------------------------------------------------------------------------------------------------------------------------

BOOL
CConfigPropertySheet::OnInitDialog()
{
	OSStatus err;

	BOOL b = CPropertySheet::OnInitDialog();

	err = SetupBrowsing();
	require_noerr( err, exit );

exit:

	return b;
}


//---------------------------------------------------------------------------------------------------------------------------
//	CConfigPropertySheet::OnCommand
//---------------------------------------------------------------------------------------------------------------------------

BOOL
CConfigPropertySheet::OnCommand(WPARAM wParam, LPARAM lParam)
{
   // Check if OK or Cancel was hit

   if ( ( wParam == ID_WIZFINISH ) || ( wParam == IDOK ) || ( wParam == IDCANCEL ) ) 
   {
      OnEndDialog();
   }

   return CPropertySheet::OnCommand(wParam, lParam);
}


//---------------------------------------------------------------------------------------------------------------------------
//	CConfigPropertySheet::OnDataReady
//---------------------------------------------------------------------------------------------------------------------------

LRESULT
CConfigPropertySheet::OnDataReady(WPARAM inWParam, LPARAM inLParam)
{
	if (WSAGETSELECTERROR(inLParam) && !(HIWORD(inLParam)))
	{
		dlog( kDebugLevelError, "OnSocket: window error\n" );
	}
	else
	{
		SOCKET sock = (SOCKET) inWParam;

		if ( m_browseDomainsRef && DNSServiceRefSockFD( m_browseDomainsRef ) == (int) sock )
		{
			DNSServiceProcessResult( m_browseDomainsRef );
		}
	}

	return 0;
}


//---------------------------------------------------------------------------------------------------------------------------
//	CConfigPropertySheet::OnEndDialog
//---------------------------------------------------------------------------------------------------------------------------

void
CConfigPropertySheet::OnEndDialog()
{
	OSStatus err;

	err = TearDownBrowsing();
	check_noerr( err );
}


//---------------------------------------------------------------------------------------------------------------------------
//	CConfigPropertySheet::SetupBrowsing
//---------------------------------------------------------------------------------------------------------------------------

OSStatus
CConfigPropertySheet::SetupBrowsing()
{
	OSStatus err;

	// Start browsing for browse domains

	err = DNSServiceEnumerateDomains( &m_browseDomainsRef, kDNSServiceFlagsBrowseDomains, 0, BrowseDomainsReply, this );
	require_noerr( err, exit );

	err = WSAAsyncSelect( DNSServiceRefSockFD( m_browseDomainsRef ), m_hWnd, WM_DATAREADY, FD_READ|FD_CLOSE );
	require_noerr( err, exit );

exit:

	if ( err )
	{
		TearDownBrowsing();
	}

	return err;
}


//---------------------------------------------------------------------------------------------------------------------------
//	CConfigPropertySheet::TearDownBrowsing
//---------------------------------------------------------------------------------------------------------------------------

OSStatus
CConfigPropertySheet::TearDownBrowsing()
{
	OSStatus err = kNoErr;

	if ( m_browseDomainsRef )
	{
		err = WSAAsyncSelect( DNSServiceRefSockFD( m_browseDomainsRef ), m_hWnd, 0, 0 );
		check_noerr( err );

		DNSServiceRefDeallocate( m_browseDomainsRef );
	
		m_browseDomainsRef = NULL;
	}

	return err;
}


//---------------------------------------------------------------------------------------------------------------------------
//	CConfigPropertySheet::DecodeDomainName
//---------------------------------------------------------------------------------------------------------------------------

OSStatus
CConfigPropertySheet::DecodeDomainName( const char * raw, CString & decoded )
{
	char nextLabel[128] = "\0";
	char decodedDomainString[kDNSServiceMaxDomainName];
    char * buffer = (char *) raw;
    int labels = 0, i;
    char text[64];
	const char *label[128];
	OSStatus	err;
    
	// Initialize

	decodedDomainString[0] = '\0';

    // Count the labels

	while ( *buffer )
	{
		label[labels++] = buffer;
		buffer = (char *) GetNextLabel(buffer, text);
    }
        
    buffer = (char*) raw;

    for (i = 0; i < labels; i++)
	{
		buffer = (char *)GetNextLabel(buffer, nextLabel);
        strcat(decodedDomainString, nextLabel);
        strcat(decodedDomainString, ".");
    }
    
    // Remove trailing dot from domain name.
    
	decodedDomainString[ strlen( decodedDomainString ) - 1 ] = '\0';

	// Convert to Unicode

	err = UTF8StringToStringObject( decodedDomainString, decoded );

	return err;
}


//---------------------------------------------------------------------------------------------------------------------------
//	CConfigPropertySheet::BrowseDomainsReply
//---------------------------------------------------------------------------------------------------------------------------

void DNSSD_API
CConfigPropertySheet::BrowseDomainsReply
							(
							DNSServiceRef			sdRef,
							DNSServiceFlags			flags,
							uint32_t				interfaceIndex,
							DNSServiceErrorType		errorCode,
							const char			*	replyDomain,
							void				*	context
							)
{
	CConfigPropertySheet	*	self = reinterpret_cast<CConfigPropertySheet*>(context);
	CString						decoded;
	OSStatus					err;

	DEBUG_UNUSED( sdRef );
	DEBUG_UNUSED( interfaceIndex );

	if ( errorCode )
	{
		goto exit;
	}

	check( replyDomain );
	
	// Ignore local domains

	if ( strcmp( replyDomain, "local." ) == 0 )
	{
		goto exit;
	}

	err = self->DecodeDomainName( replyDomain, decoded );
	require_noerr( err, exit );

	// Remove trailing '.'

	decoded.TrimRight( '.' );

	if ( flags & kDNSServiceFlagsAdd )
	{
		self->m_browseDomains.push_back( decoded );
	}
	else
	{
		self->m_browseDomains.remove( decoded );
	}

exit:

	return;
}
