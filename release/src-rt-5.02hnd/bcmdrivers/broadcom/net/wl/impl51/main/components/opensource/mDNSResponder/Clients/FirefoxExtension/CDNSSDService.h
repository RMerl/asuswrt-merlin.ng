/* -*- Mode: C; tab-width: 4 -*-
 *
 * Copyright (c) 2009 Apple Computer, Inc. All rights reserved.
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


#ifndef _CDNSSDSERVICE_H
#define _CDNSSDSERVICE_H

#include "IDNSSDService.h"
#include "nsCOMPtr.h"
#include "nsComponentManagerUtils.h"
#include "nsIThread.h"
#include "nsIRunnable.h"
#include "prtpool.h"
#include <dns_sd.h>
#include <stdio.h>
#include <string>


#define CDNSSDSERVICE_CONTRACTID "@apple.com/DNSSDService;1"
#define CDNSSDSERVICE_CLASSNAME "CDNSSDService"
#define CDNSSDSERVICE_CID { 0x944ED267, 0x465A, 0x4989, { 0x82, 0x72, 0x7E, 0xE9, 0x28, 0x6C, 0x99, 0xA5 } }


/* Header file */
class CDNSSDService : public IDNSSDService, nsIRunnable
{
public:
	NS_DECL_ISUPPORTS
	NS_DECL_IDNSSDSERVICE
	NS_DECL_NSIRUNNABLE

	CDNSSDService();
	CDNSSDService( DNSServiceRef mainRef, nsISupports * listener );

	virtual ~CDNSSDService();
	
private:

	static void DNSSD_API
	BrowseReply
		(
		DNSServiceRef		sdRef,
		DNSServiceFlags		flags,
		uint32_t			interfaceIndex,
		DNSServiceErrorType	errorCode,
		const char		*	serviceName,
		const char		*	regtype,
		const char		*	replyDomain,
		void			*	context
		);
		
	static void DNSSD_API
	ResolveReply
		(
		DNSServiceRef			sdRef,
		DNSServiceFlags			flags,
		uint32_t				interfaceIndex,
		DNSServiceErrorType		errorCode,
		const char			*	fullname,
		const char			*	hosttarget,
		uint16_t				port,
		uint16_t				txtLen,
		const unsigned char	*	txtRecord,
		void				*	context
		);
		
	static void
	Read
		(
		void * arg
		);
		
	nsresult
	SetupNotifications();
	
	void
	Cleanup();

	char				m_master;
	PRThreadPool	*	m_threadPool;
	DNSServiceRef		m_mainRef;
	DNSServiceRef		m_subRef;
	nsISupports		*	m_listener;
	PRFileDesc		*	m_fileDesc;
	PRJobIoDesc			m_iod;
	PRJob			*	m_job;
};


#endif
