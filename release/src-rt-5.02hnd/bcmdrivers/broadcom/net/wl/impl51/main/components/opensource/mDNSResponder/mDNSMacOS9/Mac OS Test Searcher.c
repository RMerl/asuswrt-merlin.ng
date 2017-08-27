/* -*- Mode: C; tab-width: 4 -*-
 *
 * Copyright (c) 2002-2003 Apple Computer, Inc. All rights reserved.
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

#include <stdio.h>						// For printf()
#include <Events.h>						// For WaitNextEvent()
#include <SIOUX.h>						// For SIOUXHandleOneEvent()

#include "mDNSEmbeddedAPI.h"			// Defines the interface to the client layer above
#include "mDNSMacOS9.h"					// Defines the specific types needed to run mDNS on this platform

typedef struct
	{
	OTLIFO serviceinfolist;
	Boolean headerPrinted;
	Boolean lostRecords;
	} SearcherServices;

typedef struct { ServiceInfo i; mDNSBool add; mDNSBool dom; OTLink link; } linkedServiceInfo;

// These don't have to be globals, but their memory does need to remain valid for as
// long as the search is going on. They are declared as globals here for simplicity.
#define RR_CACHE_SIZE 1000
static CacheEntity rrcachestorage[RR_CACHE_SIZE];
static mDNS mDNSStorage;
static mDNS_PlatformSupport PlatformSupportStorage;
static SearcherServices services;
static DNSQuestion browsequestion, domainquestion;

// PrintServiceInfo prints the service information to standard out
// A real application might want to do something else with the information
static void PrintServiceInfo(SearcherServices *services)
	{
	OTLink *link = OTReverseList(OTLIFOStealList(&services->serviceinfolist));
	
	while (link)
		{
		linkedServiceInfo *ls = OTGetLinkObject(link, linkedServiceInfo, link);
		ServiceInfo *s = &ls->i;

		if (!services->headerPrinted)
			{
			printf("%-55s Type             Domain         IP Address       Port Info\n", "Name");
			services->headerPrinted = true;
			}

		if (ls->dom)
			{
			char c_dom[MAX_ESCAPED_DOMAIN_NAME];
			ConvertDomainNameToCString(&s->name, c_dom);
			if (ls->add) printf("%-55s available for browsing\n", c_dom);
			else         printf("%-55s no longer available for browsing\n", c_dom);
			}
		else
			{
			domainlabel name;
			domainname type, domain;
			char c_name[MAX_DOMAIN_LABEL+1], c_type[MAX_ESCAPED_DOMAIN_NAME], c_dom[MAX_ESCAPED_DOMAIN_NAME], c_ip[20];
			DeconstructServiceName(&s->name, &name, &type, &domain);
			ConvertDomainLabelToCString_unescaped(&name, c_name);
			ConvertDomainNameToCString(&type, c_type);
			ConvertDomainNameToCString(&domain, c_dom);
			sprintf(c_ip, "%d.%d.%d.%d", s->ip.ip.v4.b[0], s->ip.ip.v4.b[1], s->ip.ip.v4.b[2], s->ip.ip.v4.b[3]);

			printf("%-55s %-16s %-14s ", c_name, c_type, c_dom);
			if (ls->add) printf("%-15s %5d %#s\n", c_ip, mDNSVal16(s->port), s->TXTinfo);
			else         printf("Removed\n");
			}

		link = link->fNext;
		OTFreeMem(ls);
		}
	}

// When the name, address, port, and txtinfo for a service is found, FoundInstanceInfo()
// enqueues a record for PrintServiceInfo() to print.
// Note, a browsing application would *not* normally need to get all this information --
// all it needs is the name, to display to the user.
// Finding out the address, port, and txtinfo should be deferred to the time that the user
// actually needs to contact the service to use it.
static void FoundInstanceInfo(mDNS *const m, ServiceInfoQuery *query)
	{
	SearcherServices *services = (SearcherServices *)query->ServiceInfoQueryContext;
	linkedServiceInfo *info = (linkedServiceInfo *)(query->info);
	if (query->info->ip.type == mDNSAddrType_IPv4)
		{
		mDNS_StopResolveService(m, query);		// For this test code, one answer is sufficient
		OTLIFOEnqueue(&services->serviceinfolist, &info->link);
		OTFreeMem(query);
		}
	}

// When a new named instance of a service is found, FoundInstance() is called.
// In this sample code we turn around and immediately issue a query to resolve that service name to
// find its address, port, and txtinfo, but a normal browing application would just display the name.
static void FoundInstance(mDNS *const m, DNSQuestion *question, const ResourceRecord *const answer, QC_result AddRecord)
	{
	#pragma unused (question)
	SearcherServices *services = (SearcherServices *)question->QuestionContext;
	linkedServiceInfo *info;

	debugf("FoundInstance %##s PTR %##s", answer->name->c, answer->rdata->u.name.c);

	if (answer->rrtype != kDNSType_PTR) return;
	if (!services) { debugf("FoundInstance: services is NULL"); return; }
	
	info = (linkedServiceInfo *)OTAllocMem(sizeof(linkedServiceInfo));
	if (!info) { services->lostRecords = true; return; }
	
	info->i.name          = answer->rdata->u.name;
	info->i.InterfaceID   = answer->InterfaceID;
	info->i.ip.type		  = mDNSAddrType_IPv4;
	info->i.ip.ip.v4      = zerov4Addr;
	info->i.port          = zeroIPPort;
	info->add             = AddRecord;
	info->dom             = mDNSfalse;
	
	if (!AddRecord)	// If TTL == 0 we're deleting a service,
		OTLIFOEnqueue(&services->serviceinfolist, &info->link);
	else								// else we're adding a new service
		{
		ServiceInfoQuery *q = (ServiceInfoQuery *)OTAllocMem(sizeof(ServiceInfoQuery));
		if (!q) { OTFreeMem(info); services->lostRecords = true; return; }
		mDNS_StartResolveService(m, q, &info->i, FoundInstanceInfo, services);
		}
	}

static void FoundDomain(mDNS *const m, DNSQuestion *question, const ResourceRecord *const answer, QC_result AddRecord)
	{
	#pragma unused (m)
	#pragma unused (question)
	SearcherServices *services = (SearcherServices *)question->QuestionContext;
	linkedServiceInfo *info;

	debugf("FoundDomain %##s PTR %##s", answer->name->c, answer->rdata->u.name.c);

	if (answer->rrtype != kDNSType_PTR) return;
	if (!services) { debugf("FoundDomain: services is NULL"); return; }
	
	info = (linkedServiceInfo *)OTAllocMem(sizeof(linkedServiceInfo));
	if (!info) { services->lostRecords = true; return; }
	
	info->i.name          = answer->rdata->u.name;
	info->i.InterfaceID   = answer->InterfaceID;
	info->i.ip.type		  = mDNSAddrType_IPv4;
	info->i.ip.ip.v4      = zerov4Addr;
	info->i.port          = zeroIPPort;
	info->add             = AddRecord;
	info->dom             = mDNStrue;
		
	OTLIFOEnqueue(&services->serviceinfolist, &info->link);
	}

// YieldSomeTime() just cooperatively yields some time to other processes running on classic Mac OS
static Boolean YieldSomeTime(UInt32 milliseconds)
	{
	extern Boolean SIOUXQuitting;
	EventRecord e;
	WaitNextEvent(everyEvent, &e, milliseconds / 17, NULL);
	SIOUXHandleOneEvent(&e);
	return(SIOUXQuitting);
	}

int main()
	{
	mStatus err;
	Boolean DoneSetup = false;
	void *tempmem;

	SIOUXSettings.asktosaveonclose = false;
	SIOUXSettings.userwindowtitle  = "\pMulticast DNS Searcher";
	SIOUXSettings.rows             = 40;
	SIOUXSettings.columns          = 132;

	printf("Multicast DNS Searcher\n\n");
	printf("This software reports errors using MacsBug breaks,\n");
	printf("so if you don't have MacsBug installed your Mac may crash.\n\n");
	printf("******************************************************************************\n");

	err = InitOpenTransport();
	if (err) { debugf("InitOpenTransport failed %d", err); return(err); }

	err = mDNS_Init(&mDNSStorage, &PlatformSupportStorage, rrcachestorage, RR_CACHE_SIZE,
		mDNS_Init_DontAdvertiseLocalAddresses, mDNS_Init_NoInitCallback, mDNS_Init_NoInitCallbackContext);
	if (err) return(err);

	// Make sure OT has a large enough memory pool for us to draw from at OTNotifier (interrupt) time
	tempmem = OTAllocMem(0x10000);
	if (tempmem) OTFreeMem(tempmem);
	else printf("**** Warning: OTAllocMem couldn't pre-allocate 64K for us.\n");

	services.serviceinfolist.fHead = NULL;
	services.headerPrinted         = false;
	services.lostRecords           = false;

	while (!YieldSomeTime(35))
		{
#if MDNS_ONLYSYSTEMTASK
		// For debugging, use "#define MDNS_ONLYSYSTEMTASK 1" and call mDNSPlatformIdle() periodically.
		// For shipping code, don't define MDNS_ONLYSYSTEMTASK, and you don't need to call mDNSPlatformIdle()
		extern void mDNSPlatformIdle(mDNS *const m);
		mDNSPlatformIdle(&mDNSStorage);	// Only needed for debugging version
#endif
		if (mDNSStorage.mDNSPlatformStatus == mStatus_NoError && !DoneSetup)
			{
			domainname srvtype, srvdom;
			DoneSetup = true;
			printf("\nSending mDNS service lookup queries and waiting for responses...\n\n");
			MakeDomainNameFromDNSNameString(&srvtype, "_http._tcp.");
			MakeDomainNameFromDNSNameString(&srvdom, "local.");
			err = mDNS_StartBrowse(&mDNSStorage, &browsequestion, &srvtype, &srvdom, mDNSInterface_Any, mDNSfalse, FoundInstance, &services);
			if (err) break;
			err = mDNS_GetDomains(&mDNSStorage, &domainquestion, mDNS_DomainTypeBrowse, NULL, mDNSInterface_Any, FoundDomain, &services);
			if (err) break;
			}

		if (services.serviceinfolist.fHead)
			PrintServiceInfo(&services);

		if (services.lostRecords)
			{
			services.lostRecords = false;
			printf("**** Warning: Out of memory: Records have been missed.\n");
			}
		}

	mDNS_StopBrowse(&mDNSStorage, &browsequestion);
	mDNS_Close(&mDNSStorage);
	return(0);
	}
