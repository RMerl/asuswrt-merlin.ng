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

// ***************************************************************************
// mDNSMacOSX.c:
// Supporting routines to run mDNS on a CFRunLoop platform
// ***************************************************************************

// For debugging, set LIST_ALL_INTERFACES to 1 to display all found interfaces,
// including ones that mDNSResponder chooses not to use.
#define LIST_ALL_INTERFACES 0

// For enabling AAAA records over IPv4. Setting this to 0 sends only
// A records over IPv4 and AAAA over IPv6. Setting this to 1 sends both
// AAAA and A records over both IPv4 and IPv6.
#define AAAA_OVER_V4 1

// In Mac OS X 10.4 and earlier, to reduce traffic, we would send and receive using IPv6 only on interfaces that had no routable
// IPv4 address. Having a routable IPv4 address assigned is a reasonable indicator of being on a large configured network,
// which means there's a good chance that most or all the other devices on that network should also have IPv4.
// By doing this we lost the ability to talk to true IPv6-only devices on that link, but we cut the packet rate in half.
// At that time, reducing the packet rate was more important than v6-only devices on a large configured network,
// so were willing to make that sacrifice.
// In Mac OS X 10.5, in 2007, two things have changed:
// 1. IPv6-only devices are starting to become more common, so we can't ignore them.
// 2. Other efficiency improvements in the code mean that crude hacks like this should no longer be necessary.

#define USE_V6_ONLY_WHEN_NO_ROUTABLE_V4 0

#include "mDNSEmbeddedAPI.h"		// Defines the interface provided to the client layer above
#include "DNSCommon.h"
#include "uDNS.h"
#include "mDNSMacOSX.h"				// Defines the specific types needed to run mDNS on this platform
#include "dns_sd.h"					// For mDNSInterface_LocalOnly etc.
#include "PlatformCommon.h"
#include "uds_daemon.h"

#include <stdio.h>
#include <stdarg.h>                 // For va_list support
#include <stdlib.h>                 // For arc4random
#include <net/if.h>
#include <net/if_types.h>			// For IFT_ETHER
#include <net/if_dl.h>
#include <net/bpf.h>				// For BIOCSETIF etc.
#include <sys/uio.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <sys/event.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <time.h>                   // platform support for UTC time
#include <arpa/inet.h>              // for inet_aton
#include <pthread.h>
#include <netdb.h>					// for getaddrinfo

#include <netinet/in.h>             // For IP_RECVTTL
#ifndef IP_RECVTTL
#define IP_RECVTTL 24               // bool; receive reception TTL w/dgram
#endif

#include <netinet/in_systm.h>       // For n_long, required by <netinet/ip.h> below
#include <netinet/ip.h>             // For IPTOS_LOWDELAY etc.
#include <netinet6/in6_var.h>       // For IN6_IFF_NOTREADY etc.
#include <netinet6/nd6.h>           // For ND6_INFINITE_LIFETIME etc.

#if TARGET_OS_EMBEDDED
#define NO_SECURITYFRAMEWORK 1
#define NO_CFUSERNOTIFICATION 1
#endif

#ifndef NO_SECURITYFRAMEWORK
#include <Security/SecureTransport.h>
#include <Security/Security.h>
#endif /* NO_SECURITYFRAMEWORK */

#include <DebugServices.h>
#include "dnsinfo.h"

// Code contributed by Dave Heller:
// Define RUN_ON_PUMA_WITHOUT_IFADDRS to compile code that will
// work on Mac OS X 10.1, which does not have the getifaddrs call.
#define RUN_ON_PUMA_WITHOUT_IFADDRS 0
#if RUN_ON_PUMA_WITHOUT_IFADDRS
#include "mDNSMacOSXPuma.c"
#else
#include <ifaddrs.h>
#endif

#include <IOKit/IOKitLib.h>
#include <IOKit/IOMessage.h>

#ifdef MDNSRESPONDER_USES_LIB_DISPATCH_AS_PRIMARY_EVENT_LOOP_MECHANISM
// This is currently defined in IOKit/PrivateHeaders/IOKitLibPrivate.h. Till it becomes an Public
// API, we will have our own declaration
void IONotificationPortSetDispatchQueue(IONotificationPortRef notify, dispatch_queue_t queue);
#endif

#if USE_IOPMCOPYACTIVEPMPREFERENCES
#include <IOKit/ps/IOPowerSources.h>
#include <IOKit/ps/IOPowerSourcesPrivate.h>
#endif

#include <mach/mach_error.h>
#include <mach/mach_port.h>
#include <mach/mach_time.h>
#include "helper.h"
#include "P2PPacketFilter.h"

#include <asl.h>

#include <SystemConfiguration/SCDynamicStorePrivate.h>

#if APPLE_OSX_mDNSResponder
#include <DeviceToDeviceManager/DeviceToDeviceManager.h>
#include <AWACS.h>

#if ! NO_D2D
D2DStatus D2DInitialize(CFRunLoopRef runLoop, D2DServiceCallback serviceCallback, void* userData) __attribute__((weak_import));
D2DStatus D2DTerminate() __attribute__((weak_import));
D2DStatus D2DStartAdvertisingPair(const Byte *key, const size_t keySize, const Byte *value, const size_t valueSize) __attribute__((weak_import));
D2DStatus D2DStopAdvertisingPair(const Byte *key, const size_t keySize, const Byte *value, const size_t valueSize) __attribute__((weak_import));
D2DStatus D2DStartBrowsingForKey(const Byte *key, const size_t keySize) __attribute__((weak_import));
D2DStatus D2DStopBrowsingForKey(const Byte *key, const size_t keySize) __attribute__((weak_import));
void D2DStartResolvingPair(const Byte *key, const size_t keySize, const Byte *value, const size_t valueSize) __attribute__((weak_import));
void D2DStopResolvingPair(const Byte *key, const size_t keySize, const Byte *value, const size_t valueSize) __attribute__((weak_import));
D2DStatus D2DRetain(D2DServiceInstance instanceHandle, D2DTransportType transportType) __attribute__((weak_import));
D2DStatus D2DRelease(D2DServiceInstance instanceHandle, D2DTransportType transportType) __attribute__((weak_import));

#define CHECK_D2D_FUNCTION(X) if (X)

#endif // ! NO_D2D

#else
#define NO_D2D 1
#define NO_AWACS 1
#endif // APPLE_OSX_mDNSResponder

#define kInterfaceSpecificOption "interface="

// ***************************************************************************
// Globals

#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark - Globals
#endif

// By default we don't offer sleep proxy service
// If OfferSleepProxyService is set non-zero (typically via command-line switch),
// then we'll offer sleep proxy service on desktop Macs that are set to never sleep.
// We currently do not offer sleep proxy service on laptops, or on machines that are set to go to sleep.
mDNSexport int OfferSleepProxyService = 0;
mDNSexport int DisableSleepProxyClient = 0;
mDNSexport int UseInternalSleepProxy = 1;		// Set to non-zero to use internal (in-NIC) Sleep Proxy

// We disable inbound relay connection if this value is set to true (typically via command-line switch).
mDNSBool DisableInboundRelayConnection = mDNSfalse;
mDNSexport int OSXVers, iOSVers;
mDNSexport int KQueueFD;

#ifndef NO_SECURITYFRAMEWORK
static CFArrayRef ServerCerts;
OSStatus SSLSetAllowAnonymousCiphers(SSLContextRef context, Boolean enable);
#endif /* NO_SECURITYFRAMEWORK */

static CFStringRef NetworkChangedKey_IPv4;
static CFStringRef NetworkChangedKey_IPv6;
static CFStringRef NetworkChangedKey_Hostnames;
static CFStringRef NetworkChangedKey_Computername;
static CFStringRef NetworkChangedKey_DNS;
static CFStringRef NetworkChangedKey_DynamicDNS       = CFSTR("Setup:/Network/DynamicDNS");
static CFStringRef NetworkChangedKey_BackToMyMac      = CFSTR("Setup:/Network/BackToMyMac");
static CFStringRef NetworkChangedKey_BTMMConnectivity = CFSTR("State:/Network/Connectivity");
static CFStringRef NetworkChangedKey_PowerSettings    = CFSTR("State:/IOKit/PowerManagement/CurrentSettings");

static char  HINFO_HWstring_buffer[32];
static char *HINFO_HWstring = "Device";
static int   HINFO_HWstring_prefixlen = 6;

mDNSexport int WatchDogReportingThreshold = 250;

#ifdef MDNSRESPONDER_USES_LIB_DISPATCH_AS_PRIMARY_EVENT_LOOP_MECHANISM
dispatch_queue_t SSLqueue;
#endif

#if APPLE_OSX_mDNSResponder
static mDNSu8 SPMetricPortability   = 99;
static mDNSu8 SPMetricMarginalPower = 99;
static mDNSu8 SPMetricTotalPower    = 99;
mDNSexport domainname ActiveDirectoryPrimaryDomain;
mDNSexport int        ActiveDirectoryPrimaryDomainLabelCount;
mDNSexport mDNSAddr   ActiveDirectoryPrimaryDomainServer;
#endif // APPLE_OSX_mDNSResponder

// Used by AutoTunnel
const char btmmprefix[] = "btmmdns:";
const char dnsprefix[] = "dns:";

// ***************************************************************************
#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark -
#pragma mark - D2D Support
#endif

#if ! NO_D2D

// Name compression items for fake packet version number 1
static const mDNSu8 compression_packet_v1 = 0x01;

static DNSMessage compression_base_msg = { { {{0}}, {{0}}, 2, 0, 0, 0 }, "\x04_tcp\x05local\x00\x00\x0C\x00\x01\x04_udp\xC0\x11\x00\x0C\x00\x01" };
static mDNSu8 *const compression_limit = (mDNSu8 *) &compression_base_msg + sizeof(DNSMessage);
static mDNSu8 *const compression_lhs = (mDNSu8 *const) compression_base_msg.data + 27;

mDNSlocal void FreeD2DARElemCallback(mDNS *const m, AuthRecord *const rr, mStatus result);
mDNSlocal void PrintHex(mDNSu8 *data, mDNSu16 len);

static ARListElem *D2DRecords = NULL; // List of locally-generated PTR records to records found via D2D

typedef struct D2DBrowseListElem
	{
	struct D2DBrowseListElem *next;
	domainname name;
	mDNSu16 type;
	unsigned int refCount;
	} D2DBrowseListElem;

D2DBrowseListElem* D2DBrowseList = NULL;

mDNSlocal mDNSu8 *putVal16(mDNSu8 *ptr, mDNSu16 val)
	{
	ptr[0] = (mDNSu8)((val >> 8 ) & 0xFF);
	ptr[1] = (mDNSu8)((val      ) & 0xFF);
	return ptr + sizeof(mDNSu16);
	}

mDNSlocal mDNSu8 *putVal32(mDNSu8 *ptr, mDNSu32 val)
	{
	ptr[0] = (mDNSu8)((val >> 24) & 0xFF);
	ptr[1] = (mDNSu8)((val >> 16) & 0xFF);
	ptr[2] = (mDNSu8)((val >>  8) & 0xFF);
	ptr[3] = (mDNSu8)((val      ) & 0xFF);
	return ptr + sizeof(mDNSu32);
	}

mDNSlocal void DomainnameToLower(const domainname * const in, domainname * const out)
	{
	const mDNSu8 * const start = (const mDNSu8 * const)in;
	mDNSu8 *ptr = (mDNSu8*)start;
	while(*ptr)
		{
		mDNSu8 c = *ptr;
		out->c[ptr-start] = *ptr;
		ptr++;
		for (;c;c--,ptr++) out->c[ptr-start] = mDNSIsUpperCase(*ptr) ? (*ptr - 'A' + 'a') : *ptr;
		}
	out->c[ptr-start] = *ptr;
	}

mDNSlocal mStatus DNSNameCompressionParseBytes(mDNS *const m, const mDNSu8 *const lhs, const mDNSu16 lhs_len, const mDNSu8 *const rhs, const mDNSu16 rhs_len, AuthRecord *rr)
	{
	if (mDNS_LoggingEnabled)
		{
		LogInfo("%s", __func__);
		LogInfo("  Static Bytes: ");
		PrintHex((mDNSu8*)&compression_base_msg, compression_lhs - (mDNSu8*)&compression_base_msg);
		}
	
	mDNSu8 *ptr = compression_lhs; // pointer to the end of our fake packet

	// Check to make sure we're not going to go past the end of the DNSMessage data
	// 7 = 2 for CLASS (-1 for our version) + 4 for TTL + 2 for RDLENGTH
	if (ptr + lhs_len - 7 + rhs_len >= compression_limit) return mStatus_NoMemoryErr;
	
	// Copy the LHS onto our fake wire packet
	mDNSPlatformMemCopy(ptr, lhs, lhs_len);
	ptr += lhs_len - 1;
	
	// Check the 'fake packet' version number, to ensure that we know how to decompress this data
	if (*ptr != compression_packet_v1) return mStatus_Incompatible;
	
	// two bytes of CLASS
	ptr = putVal16(ptr, kDNSClass_IN | kDNSClass_UniqueRRSet);
	
	// four bytes of TTL
	ptr = putVal32(ptr, 120);
	
	// Copy the RHS length into the RDLENGTH of our fake wire packet
	ptr = putVal16(ptr, rhs_len);
	
	// Copy the RHS onto our fake wire packet
	mDNSPlatformMemCopy(ptr, rhs, rhs_len);
	ptr += rhs_len;
	
	if (mDNS_LoggingEnabled)
		{
		LogInfo("  Our Bytes %d: ", __LINE__);
		PrintHex(compression_lhs, ptr - compression_lhs);
		}

	ptr = (mDNSu8 *) GetLargeResourceRecord(m, &compression_base_msg, compression_lhs, ptr, mDNSInterface_Any, kDNSRecordTypePacketAns, &m->rec);
	if (!ptr || m->rec.r.resrec.RecordType == kDNSRecordTypePacketNegative)
		{ LogMsg("DNSNameCompressionParseBytes: failed to get large RR"); m->rec.r.resrec.RecordType = 0; return mStatus_UnknownErr; }
	else LogInfo("DNSNameCompressionParseBytes: got rr: %s", CRDisplayString(m, &m->rec.r));	

	mDNS_SetupResourceRecord(rr, mDNSNULL, mDNSInterface_P2P, m->rec.r.resrec.rrtype, 7200, kDNSRecordTypeShared, AuthRecordP2P, FreeD2DARElemCallback, NULL);
	AssignDomainName(&rr->namestorage, &m->rec.namestorage);
	rr->resrec.rdlength = m->rec.r.resrec.rdlength;
	rr->resrec.rdata->MaxRDLength = m->rec.r.resrec.rdlength;
	mDNSPlatformMemCopy(rr->resrec.rdata->u.data, m->rec.r.resrec.rdata->u.data, m->rec.r.resrec.rdlength);
	rr->resrec.namehash = DomainNameHashValue(rr->resrec.name);
	SetNewRData(&rr->resrec, mDNSNULL, 0);	// Sets rr->rdatahash for us
	
	m->rec.r.resrec.RecordType = 0; // Mark m->rec as no longer in use

	return mStatus_NoError;
	}

mDNSlocal mDNSu8 * DNSNameCompressionBuildLHS(const domainname const *typeDomain, DNS_TypeValues qtype)
	{
	mDNSu8 *ptr = putDomainNameAsLabels(&compression_base_msg, compression_lhs, compression_limit, typeDomain);
	if (!ptr) return ptr;
	*ptr = (qtype >> 8) & 0xff;
	ptr += 1;
	*ptr = qtype & 0xff;
	ptr += 1;
	*ptr = compression_packet_v1;
	return ptr + 1;
	}

mDNSlocal mDNSu8 * DNSNameCompressionBuildRHS(mDNSu8 *start, const ResourceRecord *const resourceRecord)
	{
	return putRData(&compression_base_msg, start, compression_limit, resourceRecord);
	}

mDNSlocal void PrintHex(mDNSu8 *data, mDNSu16 len)
	{
	mDNSu8 *end = data + len;
	char buffer[49] = {0};
	char *bufend = buffer + sizeof(buffer);
	while(data<end)
		{
		char *ptr = buffer;
		for(; data < end && ptr < bufend-1; ptr+=3,data++)
			mDNS_snprintf(ptr, bufend - ptr, "%02X ", *data);
		LogInfo("    %s", buffer);
		}
	}

mDNSlocal void PrintHelper(const char *const tag, mDNSu8 *lhs, mDNSu16 lhs_len, mDNSu8 *rhs, mDNSu16 rhs_len)
	{
	if (!mDNS_LoggingEnabled) return;
	
	LogInfo("%s:", tag);
	LogInfo("  LHS: ");
	PrintHex(lhs, lhs_len);
	
	if (!rhs) return;
	
	LogInfo("  RHS: ");
	PrintHex(rhs, rhs_len);
	}

mDNSlocal void FreeD2DARElemCallback(mDNS *const m, AuthRecord *const rr, mStatus result)
	{
	(void)m;  // unused
	if (result == mStatus_MemFree)
		{
		ARListElem **ptr = &D2DRecords;
		ARListElem *tmp;
		while (*ptr && &(*ptr)->ar != rr) ptr = &(*ptr)->next;
		if (!*ptr) { LogMsg("FreeD2DARElemCallback: Could not find in D2DRecords: %s", ARDisplayString(m, rr)); return; }
		LogInfo("FreeD2DARElemCallback: Found in D2DRecords: %s", ARDisplayString(m, rr));
		tmp = *ptr;
		*ptr = (*ptr)->next;
		// Just because we stoppped browsing, doesn't mean we should tear down the PAN connection.
		mDNSPlatformMemFree(tmp);
		}
	}

mDNSlocal void xD2DClearCache(mDNS *const m, const domainname *regType)
	{
	ARListElem *ptr = D2DRecords;
	for ( ; ptr ; ptr = ptr->next)
		{
		if (SameDomainName(&ptr->ar.namestorage, regType)) 
			{
			char buffer[MAX_ESCAPED_DOMAIN_NAME];
			mDNS_Deregister(m, &ptr->ar);
			ConvertDomainNameToCString(regType, buffer);
			LogInfo("xD2DClearCache: Clearing cache record and deregistering %s", buffer);	
			}
		}
	}

mDNSlocal D2DBrowseListElem ** D2DFindInBrowseList(const domainname *const name, mDNSu16 type)
	{
	D2DBrowseListElem **ptr = &D2DBrowseList;

	for ( ; *ptr; ptr = &(*ptr)->next)
		if ((*ptr)->type == type && SameDomainName(&(*ptr)->name, name))
			break;
	
	return ptr;
	}

mDNSlocal unsigned int D2DBrowseListRefCount(const domainname *const name, mDNSu16 type)
	{
	D2DBrowseListElem **ptr = D2DFindInBrowseList(name, type);
	return *ptr ? (*ptr)->refCount : 0;
	}

mDNSlocal void D2DBrowseListRetain(const domainname *const name, mDNSu16 type)
	{
	D2DBrowseListElem **ptr = D2DFindInBrowseList(name, type);
	
	if (!*ptr)
	{
		*ptr = mDNSPlatformMemAllocate(sizeof(**ptr));
		mDNSPlatformMemZero(*ptr, sizeof(**ptr));
		(*ptr)->type = type;
		AssignDomainName(&(*ptr)->name, name);
	}
	(*ptr)->refCount += 1;
	
	LogInfo("D2DBrowseListRetain: %##s %s refcount now %u", (*ptr)->name.c, DNSTypeName((*ptr)->type), (*ptr)->refCount);	
	}

mDNSlocal void D2DBrowseListRelease(const domainname *const name, mDNSu16 type)
	{	
	D2DBrowseListElem **ptr = D2DFindInBrowseList(name, type);
	
	if (!*ptr) { LogMsg("D2DBrowseListRelease: Didn't find %##s %s in list", name->c, DNSTypeName(type)); return; }
	
	(*ptr)->refCount -= 1;
	
	LogInfo("D2DBrowseListRelease: %##s %s refcount now %u", (*ptr)->name.c, DNSTypeName((*ptr)->type), (*ptr)->refCount);
	
	if (!(*ptr)->refCount)
	{
		D2DBrowseListElem *tmp = *ptr;
		*ptr = (*ptr)->next;
		mDNSPlatformMemFree(tmp);
	}
	}

mDNSlocal mStatus xD2DParse(mDNS *const m, const mDNSu8 * const lhs, const mDNSu16 lhs_len, const mDNSu8 * const rhs, const mDNSu16 rhs_len, AuthRecord *rr)
	{
	if (*(lhs + (lhs_len - 1)) == compression_packet_v1)
		return DNSNameCompressionParseBytes(m, lhs, lhs_len, rhs, rhs_len, rr);
	else
		return mStatus_Incompatible;
	}

mDNSlocal void xD2DAddToCache(mDNS *const m, D2DStatus result, D2DServiceInstance instanceHandle, D2DTransportType transportType, const Byte *key, size_t keySize, const Byte *value, size_t valueSize)
	{
	(void)transportType; // We don't care about this, yet.
	(void)instanceHandle; // We don't care about this, yet.
	
	if (result == kD2DSuccess)
		{
		if ( key == NULL || value == NULL || keySize == 0 || valueSize == 0) { LogMsg("xD2DAddToCache: NULL Byte * passed in or length == 0"); return; }
		
		mStatus err;
		ARListElem *ptr = mDNSPlatformMemAllocate(sizeof(ARListElem) + (valueSize < sizeof(RData) ? 0 : valueSize - sizeof(RData)));

		if (ptr == NULL) { LogMsg("xD2DAddToCache: memory allocation failure"); return; }

		err = xD2DParse(m, (const mDNSu8 * const)key, (const mDNSu16)keySize, (const mDNSu8 * const)value, (const mDNSu16)valueSize, &ptr->ar);
		if (err)
			{
			LogMsg("xD2DAddToCache: xD2DParse returned error: %d", err);
			PrintHelper(__func__, (mDNSu8 *)key, (mDNSu16)keySize, (mDNSu8 *)value, (mDNSu16)valueSize);
			mDNSPlatformMemFree(ptr);
			return;
			}
		err = mDNS_Register(m, &ptr->ar);
		if (err)
			{
			LogMsg("xD2DAddToCache: mDNS_Register returned error %d for %s", err, ARDisplayString(m, &ptr->ar));
			mDNSPlatformMemFree(ptr);
			return;
			}

		LogInfo("xD2DAddToCache: mDNS_Register succeeded for %s", ARDisplayString(m, &ptr->ar));
		ptr->next = D2DRecords;
		D2DRecords = ptr;
		}
	else
		LogMsg("xD2DAddToCache: Unexpected result %d", result);
	}

mDNSlocal ARListElem * xD2DFindInList(mDNS *const m, const Byte *const key, const size_t keySize, const Byte *const value, const size_t valueSize)
	{
	ARListElem *ptr = D2DRecords;
	ARListElem *arptr;
	
	if ( key == NULL || value == NULL || keySize == 0 || valueSize == 0) { LogMsg("xD2DFindInList: NULL Byte * passed in or length == 0"); return NULL; }

	arptr = mDNSPlatformMemAllocate(sizeof(ARListElem) + (valueSize < sizeof(RData) ? 0 : valueSize - sizeof(RData)));
	if (arptr == NULL) { LogMsg("xD2DFindInList: memory allocation failure"); return NULL; }

	if (xD2DParse(m, (const mDNSu8 *const)key, (const mDNSu16)keySize, (const mDNSu8 *const)value, (const mDNSu16)valueSize, &arptr->ar) != mStatus_NoError)
		{
		LogMsg("xD2DFindInList: xD2DParse failed for key: %p (%u) value: %p (%u)", key, keySize, value, valueSize);
		mDNSPlatformMemFree(arptr);
		return NULL;
		}
		
	while (ptr)
		{
		if (IdenticalResourceRecord(&arptr->ar.resrec, &ptr->ar.resrec)) break;
		ptr = ptr->next;
		}
		
	if (!ptr) LogMsg("xD2DFindInList: Could not find in D2DRecords: %s", ARDisplayString(m, &arptr->ar));
	mDNSPlatformMemFree(arptr);
	return ptr;
	}

mDNSlocal void xD2DRemoveFromCache(mDNS *const m, D2DStatus result, D2DServiceInstance instanceHandle, D2DTransportType transportType, const Byte *key, size_t keySize, const Byte *value, size_t valueSize)
	{
	(void)transportType; // We don't care about this, yet.
	(void)instanceHandle; // We don't care about this, yet.
	
	if (result == kD2DSuccess)
		{
		ARListElem *ptr = xD2DFindInList(m, key, keySize, value, valueSize);
		if (ptr) 
			{
			LogInfo("xD2DRemoveFromCache: Remove from cache: %s", ARDisplayString(m, &ptr->ar));
			mDNS_Deregister(m, &ptr->ar);
			}
		}
	else
		LogMsg("xD2DRemoveFromCache: Unexpected result %d", result);
	}

mDNSlocal void xD2DServiceResolved(mDNS *const m, D2DStatus result, D2DServiceInstance instanceHandle, D2DTransportType transportType, const Byte *key, size_t keySize, const Byte *value, size_t valueSize)
	{
	(void)m;
	(void)key;
	(void)keySize;
	(void)value;
	(void)valueSize;
	
	if (result == kD2DSuccess) 
		{
		LogInfo("xD2DServiceResolved: Starting up PAN connection for %p", instanceHandle);
		CHECK_D2D_FUNCTION(D2DRetain) D2DRetain(instanceHandle, transportType);
		}
	else LogMsg("xD2DServiceResolved: Unexpected result %d", result);
	}

mDNSlocal void xD2DRetainHappened(mDNS *const m, D2DStatus result, D2DServiceInstance instanceHandle, D2DTransportType transportType, const Byte *key, size_t keySize, const Byte *value, size_t valueSize)
	{
	(void)m;
	(void)instanceHandle;
	(void)transportType;
	(void)key;
	(void)keySize;
	(void)value;
	(void)valueSize;
	
	if (result == kD2DSuccess) LogInfo("xD2DRetainHappened: Opening up PAN connection for %p", instanceHandle);
	else LogMsg("xD2DRetainHappened: Unexpected result %d", result);
	}

mDNSlocal void xD2DReleaseHappened(mDNS *const m, D2DStatus result, D2DServiceInstance instanceHandle, D2DTransportType transportType, const Byte *key, size_t keySize, const Byte *value, size_t valueSize)
	{
	(void)m;
	(void)instanceHandle;
	(void)transportType;
	(void)key;
	(void)keySize;
	(void)value;
	(void)valueSize;
	
	if (result == kD2DSuccess) LogInfo("xD2DReleaseHappened: Closing PAN connection for %p", instanceHandle);
	else LogMsg("xD2DReleaseHappened: Unexpected result %d", result);
	}

mDNSlocal void xD2DServiceCallback(D2DServiceEvent event, D2DStatus result, D2DServiceInstance instanceHandle, D2DTransportType transportType, const Byte *key, size_t keySize, const Byte *value, size_t valueSize, void *userData)
	{
	mDNS *m = (mDNS *) userData;
	const char *eventString = "unknown";
	
	KQueueLock(m);
	
	if (keySize   > 0xFFFF) LogMsg("xD2DServiceCallback: keySize too large: %u", keySize);
	if (valueSize > 0xFFFF) LogMsg("xD2DServiceCallback: valueSize too large: %u", valueSize);
		
	switch (event)
		{
		case D2DServiceFound:
			eventString = "D2DServiceFound";
			break;
		case D2DServiceLost:
			eventString = "D2DServiceLost";
			break;
		case D2DServiceResolved:
			eventString = "D2DServiceResolved";
			break;
		case D2DServiceRetained:
			eventString = "D2DServiceRetained";
			break;
		case D2DServiceReleased:
			eventString = "D2DServiceReleased";
			break;
		default:
			break;
		}
	
	LogInfo("xD2DServiceCallback: event=%s result=%d instanceHandle=%p transportType=%d LHS=%p (%u) RHS=%p (%u) userData=%p", eventString, result, instanceHandle, transportType, key, keySize, value, valueSize, userData);
	PrintHelper(__func__, (mDNSu8 *)key, (mDNSu16)keySize, (mDNSu8 *)value, (mDNSu16)valueSize);
	
	switch (event)
		{
		case D2DServiceFound:
			xD2DAddToCache(m, result, instanceHandle, transportType, key, keySize, value, valueSize);
			break;
		case D2DServiceLost:
			xD2DRemoveFromCache(m, result, instanceHandle, transportType, key, keySize, value, valueSize);
			break;
		case D2DServiceResolved:
			xD2DServiceResolved(m, result, instanceHandle, transportType, key, keySize, value, valueSize);
			break;
		case D2DServiceRetained:
			xD2DRetainHappened(m, result, instanceHandle, transportType, key, keySize, value, valueSize);
			break;
		case D2DServiceReleased:
			xD2DReleaseHappened(m, result, instanceHandle, transportType, key, keySize, value, valueSize);
			break;
		default:
			break;
		}
	
	// Need to tickle the main kqueue loop to potentially handle records we removed or added.
	KQueueUnlock(m, "xD2DServiceCallback");
	}

mDNSexport void external_start_browsing_for_service(mDNS *const m, const domainname *const typeDomain, DNS_TypeValues qtype)
	{
	(void)m;
	domainname lower;
	
	if (qtype == kDNSServiceType_A || qtype == kDNSServiceType_AAAA)
		{
		LogInfo("external_start_browsing_for_service: ignoring address record");
		return;
		}
	
	DomainnameToLower(typeDomain, &lower);
	
	if (!D2DBrowseListRefCount(&lower, qtype))
		{
		LogInfo("external_start_browsing_for_service: Starting browse for: %##s %s", lower.c, DNSTypeName(qtype));
		mDNSu8 *end = DNSNameCompressionBuildLHS(&lower, qtype);
		PrintHelper(__func__, compression_lhs, end - compression_lhs, mDNSNULL, 0);
		CHECK_D2D_FUNCTION(D2DStartBrowsingForKey) D2DStartBrowsingForKey(compression_lhs, end - compression_lhs);
		}
	D2DBrowseListRetain(&lower, qtype);
	}

mDNSexport void external_stop_browsing_for_service(mDNS *const m, const domainname *const typeDomain, DNS_TypeValues qtype)
	{
	domainname lower;
	
	if (qtype == kDNSServiceType_A || qtype == kDNSServiceType_AAAA)
 		{
		LogInfo("external_stop_browsing_for_service: ignoring address record");
		return;
		}
	
	DomainnameToLower(typeDomain, &lower);
	
	D2DBrowseListRelease(&lower, qtype);
	if (!D2DBrowseListRefCount(&lower, qtype))
		{
		LogInfo("external_stop_browsing_for_service: Stopping browse for: %##s %s", lower.c, DNSTypeName(qtype));
		mDNSu8 *end = DNSNameCompressionBuildLHS(&lower, qtype);
		PrintHelper(__func__, compression_lhs, end - compression_lhs, mDNSNULL, 0);
		CHECK_D2D_FUNCTION(D2DStopBrowsingForKey) D2DStopBrowsingForKey(compression_lhs, end - compression_lhs);
		xD2DClearCache(m, &lower);
		}
	}

mDNSexport void external_start_advertising_service(const ResourceRecord *const resourceRecord)
	{
	domainname lower;
	mDNSu8 *rhs = NULL;
	mDNSu8 *end = NULL;
	DomainnameToLower(resourceRecord->name, &lower);
	
	LogInfo("external_start_advertising_service: %s", RRDisplayString(&mDNSStorage, resourceRecord));
	// For SRV records, update packet filter if p2p interface already exists, otherwise,
	// if will be updated when we get the KEV_DL_IF_ATTACHED event for the interface.
	// Bonjour filter rules are removed when p2p interface KEV_DL_IF_DETACHED event is received.
	if (resourceRecord->rrtype == kDNSType_SRV)
		mDNSInitPacketFilter();

	if (resourceRecord->rrtype == kDNSServiceType_A || resourceRecord->rrtype == kDNSServiceType_AAAA)
		{
		LogInfo("external_start_advertising_service: ignoring address record");
		return;
		}
	rhs = DNSNameCompressionBuildLHS(&lower, resourceRecord->rrtype);
	end = DNSNameCompressionBuildRHS(rhs, resourceRecord);
	PrintHelper(__func__, compression_lhs, rhs - compression_lhs, rhs, end - rhs);
	CHECK_D2D_FUNCTION(D2DStartAdvertisingPair) D2DStartAdvertisingPair(compression_lhs, rhs - compression_lhs, rhs, end - rhs);
	}

mDNSexport void external_stop_advertising_service(const ResourceRecord *const resourceRecord)
	{
	domainname lower;
	mDNSu8 *rhs = NULL;
	mDNSu8 *end = NULL;
	DomainnameToLower(resourceRecord->name, &lower);
	
	LogInfo("external_stop_advertising_service: %s", RRDisplayString(&mDNSStorage, resourceRecord));
	if (resourceRecord->rrtype == kDNSServiceType_A || resourceRecord->rrtype == kDNSServiceType_AAAA)
		{
		LogInfo("external_stop_advertising_service: ignoring address record");
		return;
		}
	rhs = DNSNameCompressionBuildLHS(&lower, resourceRecord->rrtype);
	end = DNSNameCompressionBuildRHS(rhs, resourceRecord);
	PrintHelper(__func__, compression_lhs, rhs - compression_lhs, rhs, end - rhs);
	CHECK_D2D_FUNCTION(D2DStopAdvertisingPair) D2DStopAdvertisingPair(compression_lhs, rhs - compression_lhs, rhs, end - rhs);
	}

mDNSexport void external_start_resolving_service(const domainname *const fqdn)
	{
	domainname lower;
	mDNSu8 *rhs = NULL;
	mDNSu8 *end = NULL;
	DomainnameToLower(SkipLeadingLabels(fqdn, 1), &lower);
	
	LogInfo("external_start_resolving_service: %##s", fqdn->c);
	rhs = DNSNameCompressionBuildLHS(&lower, kDNSType_PTR);
	end = putDomainNameAsLabels(&compression_base_msg, rhs, compression_limit, fqdn);
	PrintHelper(__func__, compression_lhs, rhs - compression_lhs, rhs, end - rhs);
	CHECK_D2D_FUNCTION(D2DStartResolvingPair) D2DStartResolvingPair(compression_lhs, rhs - compression_lhs, rhs, end - rhs);
	}

mDNSexport void external_stop_resolving_service(const domainname *const fqdn) 
	{ 
	domainname lower;
	mDNSu8 *rhs = NULL;
	mDNSu8 *end = NULL;
	DomainnameToLower(SkipLeadingLabels(fqdn, 1), &lower);
	
	LogInfo("external_stop_resolving_service: %##s", fqdn->c);
	rhs = DNSNameCompressionBuildLHS(&lower, kDNSType_PTR);
	end = putDomainNameAsLabels(&compression_base_msg, rhs, compression_limit, fqdn);
	PrintHelper(__func__, compression_lhs, rhs - compression_lhs, rhs, end - rhs);
	CHECK_D2D_FUNCTION(D2DStopResolvingPair) D2DStopResolvingPair(compression_lhs, rhs - compression_lhs, rhs, end - rhs);
	}

#elif APPLE_OSX_mDNSResponder

mDNSexport void external_start_browsing_for_service(mDNS *const m, const domainname *const type, DNS_TypeValues qtype) { (void)m; (void)type; (void)qtype; }
mDNSexport void external_stop_browsing_for_service(mDNS *const m, const domainname *const type, DNS_TypeValues qtype) { (void)m; (void)type; (void)qtype; }
mDNSexport void external_start_advertising_service(const ResourceRecord *const resourceRecord) { (void)resourceRecord; }
mDNSexport void external_stop_advertising_service(const ResourceRecord *const resourceRecord) { (void)resourceRecord; }
mDNSexport void external_start_resolving_service(const domainname *const fqdn)  { (void)fqdn; }
mDNSexport void external_stop_resolving_service(const domainname *const fqdn)  { (void)fqdn; }

#endif // ! NO_D2D

// ***************************************************************************
// Functions

#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark -
#pragma mark - Utility Functions
#endif

// We only attempt to send and receive multicast packets on interfaces that are
// (a) flagged as multicast-capable
// (b) *not* flagged as point-to-point (e.g. modem)
// Typically point-to-point interfaces are modems (including mobile-phone pseudo-modems), and we don't want
// to run up the user's bill sending multicast traffic over a link where there's only a single device at the
// other end, and that device (e.g. a modem bank) is probably not answering Multicast DNS queries anyway.
#define MulticastInterface(i) (((i)->ifa_flags & IFF_MULTICAST) && !((i)->ifa_flags & IFF_POINTOPOINT))

mDNSexport void NotifyOfElusiveBug(const char *title, const char *msg)	// Both strings are UTF-8 text
	{
	static int notifyCount = 0;
	if (notifyCount) return;

	// If we display our alert early in the boot process, then it vanishes once the desktop appears.
	// To avoid this, we don't try to display alerts in the first three minutes after boot.
	if ((mDNSu32)(mDNSPlatformRawTime()) < (mDNSu32)(mDNSPlatformOneSecond * 180)) return;

	// Unless ForceAlerts is defined, we only show these bug report alerts on machines that have a 17.x.x.x address
	#if !ForceAlerts
		{
		// Determine if we're at Apple (17.*.*.*)
		extern mDNS mDNSStorage;
		NetworkInterfaceInfoOSX *i;
		for (i = mDNSStorage.p->InterfaceList; i; i = i->next)
			if (i->ifinfo.ip.type == mDNSAddrType_IPv4 && i->ifinfo.ip.ip.v4.b[0] == 17)
				break;
		if (!i) return;	// If not at Apple, don't show the alert
		}
	#endif

	LogMsg("%s", title);
	LogMsg("%s", msg);
	// Display a notification to the user
	notifyCount++;

#ifndef NO_CFUSERNOTIFICATION
	mDNSNotify(title, msg);
#endif /* NO_CFUSERNOTIFICATION */
	}

mDNSlocal struct ifaddrs *myGetIfAddrs(int refresh)
	{
	static struct ifaddrs *ifa = NULL;

	if (refresh && ifa)
		{
		freeifaddrs(ifa);
		ifa = NULL;
		}

	if (ifa == NULL) getifaddrs(&ifa);

	return ifa;
	}

// To match *either* a v4 or v6 instance of this interface name, pass AF_UNSPEC for type
mDNSlocal NetworkInterfaceInfoOSX *SearchForInterfaceByName(mDNS *const m, const char *ifname, int type)
	{
	NetworkInterfaceInfoOSX *i;
	for (i = m->p->InterfaceList; i; i = i->next)
		if (i->Exists && !strcmp(i->ifinfo.ifname, ifname) &&
			((type == AF_UNSPEC                                         ) ||
			 (type == AF_INET  && i->ifinfo.ip.type == mDNSAddrType_IPv4) ||
			 (type == AF_INET6 && i->ifinfo.ip.type == mDNSAddrType_IPv6))) return(i);
	return(NULL);
	}

mDNSlocal int myIfIndexToName(u_short ifindex, char *name)
	{
	struct ifaddrs *ifa;
	for (ifa = myGetIfAddrs(0); ifa; ifa = ifa->ifa_next)
		if (ifa->ifa_addr->sa_family == AF_LINK)
			if (((struct sockaddr_dl*)ifa->ifa_addr)->sdl_index == ifindex)
				{ strlcpy(name, ifa->ifa_name, IF_NAMESIZE); return 0; }
	return -1;
	}

mDNSexport NetworkInterfaceInfoOSX *IfindexToInterfaceInfoOSX(const mDNS *const m, mDNSInterfaceID ifindex)
{
	mDNSu32 scope_id = (mDNSu32)(uintptr_t)ifindex;
	NetworkInterfaceInfoOSX *i;

	// Don't get tricked by inactive interfaces
	for (i = m->p->InterfaceList; i; i = i->next)
		if (i->Registered && i->scope_id == scope_id) return(i);

	return mDNSNULL;
}

mDNSexport mDNSInterfaceID mDNSPlatformInterfaceIDfromInterfaceIndex(mDNS *const m, mDNSu32 ifindex)
	{
	if (ifindex == kDNSServiceInterfaceIndexLocalOnly) return(mDNSInterface_LocalOnly);
	if (ifindex == kDNSServiceInterfaceIndexP2P      ) return(mDNSInterface_P2P);
	if (ifindex == kDNSServiceInterfaceIndexAny      ) return(mDNSNULL);

	NetworkInterfaceInfoOSX* ifi = IfindexToInterfaceInfoOSX(m, (mDNSInterfaceID)(uintptr_t)ifindex);
	if (!ifi)
		{
		// Not found. Make sure our interface list is up to date, then try again.
		LogInfo("mDNSPlatformInterfaceIDfromInterfaceIndex: InterfaceID for interface index %d not found; Updating interface list", ifindex);
		mDNSMacOSXNetworkChanged(m);
		ifi = IfindexToInterfaceInfoOSX(m, (mDNSInterfaceID)(uintptr_t)ifindex);
		}

	if (!ifi) return(mDNSNULL);	
	
	return(ifi->ifinfo.InterfaceID);
	}


mDNSexport mDNSu32 mDNSPlatformInterfaceIndexfromInterfaceID(mDNS *const m, mDNSInterfaceID id, mDNSBool suppressNetworkChange)
	{
	NetworkInterfaceInfoOSX *i;
	if (id == mDNSInterface_LocalOnly) return(kDNSServiceInterfaceIndexLocalOnly);
	if (id == mDNSInterface_P2P      ) return(kDNSServiceInterfaceIndexP2P);
	if (id == mDNSInterface_Any      ) return(0);

	mDNSu32 scope_id = (mDNSu32)(uintptr_t)id;

	// Don't use i->Registered here, because we DO want to find inactive interfaces, which have no Registered set
	for (i = m->p->InterfaceList; i; i = i->next)
		if (i->scope_id == scope_id) return(i->scope_id);

	// If we are supposed to suppress network change, return "id" back 
	if (suppressNetworkChange) return scope_id;

	// Not found. Make sure our interface list is up to date, then try again.
	LogInfo("Interface index for InterfaceID %p not found; Updating interface list", id);
	mDNSMacOSXNetworkChanged(m);
	for (i = m->p->InterfaceList; i; i = i->next)
		if (i->scope_id == scope_id) return(i->scope_id);

	return(0);
	}

#if APPLE_OSX_mDNSResponder
mDNSexport void mDNSASLLog(uuid_t *uuid, const char *subdomain, const char *result, const char *signature, const char *fmt, ...)
	{
	if (OSXVers < OSXVers_10_6_SnowLeopard) return;	// Only do ASL on Mac OS X 10.6 and later (not on iOS)

	static char		buffer[512];
	aslmsg 			asl_msg = asl_new(ASL_TYPE_MSG);

	if (!asl_msg)	{ LogMsg("mDNSASLLog: asl_new failed"); return; }
	if (uuid)
		{
		char		uuidStr[37];
		uuid_unparse(*uuid, uuidStr);
		asl_set		(asl_msg, "com.apple.message.uuid", uuidStr);
		}

	static char 	domainBase[] = "com.apple.mDNSResponder.%s";
	mDNS_snprintf	(buffer, sizeof(buffer), domainBase, subdomain);
	asl_set			(asl_msg, "com.apple.message.domain", buffer);

	if (result)		asl_set(asl_msg, "com.apple.message.result", result);
	if (signature)	asl_set(asl_msg, "com.apple.message.signature", signature);

	va_list ptr;
	va_start(ptr,fmt);
	mDNS_vsnprintf(buffer, sizeof(buffer), fmt, ptr);
	va_end(ptr);

	int	old_filter = asl_set_filter(NULL,ASL_FILTER_MASK_UPTO(ASL_LEVEL_DEBUG));
	asl_log(NULL, asl_msg, ASL_LEVEL_DEBUG, "%s", buffer);
	asl_set_filter(NULL, old_filter);
	asl_free(asl_msg);
	}
#endif // APPLE_OSX_mDNSResponder

#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark -
#pragma mark - UDP & TCP send & receive
#endif

mDNSlocal mDNSBool AddrRequiresPPPConnection(const struct sockaddr *addr)
	{
	mDNSBool result = mDNSfalse;
	SCNetworkConnectionFlags flags;
	SCNetworkReachabilityRef ReachRef = NULL;

	ReachRef = SCNetworkReachabilityCreateWithAddress(kCFAllocatorDefault, addr);
	if (!ReachRef) { LogMsg("ERROR: RequiresConnection - SCNetworkReachabilityCreateWithAddress"); goto end; }
	if (!SCNetworkReachabilityGetFlags(ReachRef, &flags)) { LogMsg("ERROR: AddrRequiresPPPConnection - SCNetworkReachabilityGetFlags"); goto end; }
	result = flags & kSCNetworkFlagsConnectionRequired;

	end:
	if (ReachRef) CFRelease(ReachRef);
	return result;
	}

// Note: If InterfaceID is NULL, it means, "send this packet through our anonymous unicast socket"
// Note: If InterfaceID is non-NULL it means, "send this packet through our port 5353 socket on the specified interface"
// OR send via our primary v4 unicast socket
// UPDATE: The UDPSocket *src parameter now allows the caller to specify the source socket
mDNSexport mStatus mDNSPlatformSendUDP(const mDNS *const m, const void *const msg, const mDNSu8 *const end,
	mDNSInterfaceID InterfaceID, UDPSocket *src, const mDNSAddr *dst, mDNSIPPort dstPort)
	{
	NetworkInterfaceInfoOSX *info = mDNSNULL;
	struct sockaddr_storage to;
	int s = -1, err;
	mStatus result = mStatus_NoError;

	if (InterfaceID)
		{
		info = IfindexToInterfaceInfoOSX(m, InterfaceID);
		if (info == NULL)
			{
			LogMsg("mDNSPlatformSendUDP: Invalid interface index %p", InterfaceID);
			return mStatus_BadParamErr;
			}
		}

	char *ifa_name = InterfaceID ? info->ifinfo.ifname : "unicast";

	if (dst->type == mDNSAddrType_IPv4)
		{
		struct sockaddr_in *sin_to = (struct sockaddr_in*)&to;
		sin_to->sin_len            = sizeof(*sin_to);
		sin_to->sin_family         = AF_INET;
		sin_to->sin_port           = dstPort.NotAnInteger;
		sin_to->sin_addr.s_addr    = dst->ip.v4.NotAnInteger;
		s = (src ? src->ss : m->p->permanentsockets).sktv4;

		if (info)	// Specify outgoing interface
			{
			if (!mDNSAddrIsDNSMulticast(dst))
				{
				#ifdef IP_BOUND_IF
					if (info->scope_id == 0)
						LogInfo("IP_BOUND_IF socket option not set -- info %p (%s) scope_id is zero", info, ifa_name);
					else
						setsockopt(s, IPPROTO_IP, IP_BOUND_IF, &info->scope_id, sizeof(info->scope_id));
				#else
					{
					static int displayed = 0;
					if (displayed < 1000)
						{
						displayed++;
						LogInfo("IP_BOUND_IF socket option not defined -- cannot specify interface for unicast packets");
						}
					}
				#endif
				}
			else
				#ifdef IP_MULTICAST_IFINDEX
				{
				err = setsockopt(s, IPPROTO_IP, IP_MULTICAST_IFINDEX, &info->scope_id, sizeof(info->scope_id));
				// We get an error when we compile on a machine that supports this option and run the binary on
				// a different machine that does not support it
				if (err < 0)
					{
					if (errno != ENOPROTOOPT) LogInfo("mDNSPlatformSendUDP: setsockopt: IP_MUTLTICAST_IFINDEX returned %d", errno);
					err = setsockopt(s, IPPROTO_IP, IP_MULTICAST_IF, &info->ifa_v4addr, sizeof(info->ifa_v4addr));
					if (err < 0 && !m->p->NetworkChanged)
						LogMsg("setsockopt - IP_MULTICAST_IF error %.4a %d errno %d (%s)", &info->ifa_v4addr, err, errno, strerror(errno));
					}
				}
				#else
				{
				err = setsockopt(s, IPPROTO_IP, IP_MULTICAST_IF, &info->ifa_v4addr, sizeof(info->ifa_v4addr));
				if (err < 0 && !m->p->NetworkChanged)
					LogMsg("setsockopt - IP_MULTICAST_IF error %.4a %d errno %d (%s)", &info->ifa_v4addr, err, errno, strerror(errno));

				}
				#endif
			}
		}
#ifndef NO_IPV6
	else if (dst->type == mDNSAddrType_IPv6)
		{
		struct sockaddr_in6 *sin6_to = (struct sockaddr_in6*)&to;
		sin6_to->sin6_len            = sizeof(*sin6_to);
		sin6_to->sin6_family         = AF_INET6;
		sin6_to->sin6_port           = dstPort.NotAnInteger;
		sin6_to->sin6_flowinfo       = 0;
		sin6_to->sin6_addr           = *(struct in6_addr*)&dst->ip.v6;
		sin6_to->sin6_scope_id       = info ? info->scope_id : 0;
		s = (src ? src->ss : m->p->permanentsockets).sktv6;
		if (info && mDNSAddrIsDNSMulticast(dst))	// Specify outgoing interface
			{
			err = setsockopt(s, IPPROTO_IPV6, IPV6_MULTICAST_IF, &info->scope_id, sizeof(info->scope_id));
			if (err < 0)
				{
				char name[IFNAMSIZ];
				if (if_indextoname(info->scope_id, name) != NULL)
					LogMsg("setsockopt - IPV6_MULTICAST_IF error %d errno %d (%s)", err, errno, strerror(errno));
				else
					LogInfo("setsockopt - IPV6_MUTLICAST_IF scopeid %d, not a valid interface", info->scope_id);
				}
			}
		}
#endif
	else
		{
		LogMsg("mDNSPlatformSendUDP: dst is not an IPv4 or IPv6 address!");
#if ForceAlerts
		*(long*)0 = 0;
#endif
		return mStatus_BadParamErr;
		}

	if (s >= 0)
		verbosedebugf("mDNSPlatformSendUDP: sending on InterfaceID %p %5s/%ld to %#a:%d skt %d",
			InterfaceID, ifa_name, dst->type, dst, mDNSVal16(dstPort), s);
	else
		verbosedebugf("mDNSPlatformSendUDP: NOT sending on InterfaceID %p %5s/%ld (socket of this type not available)",
			InterfaceID, ifa_name, dst->type, dst, mDNSVal16(dstPort));

	// Note: When sending, mDNSCore may often ask us to send both a v4 multicast packet and then a v6 multicast packet
	// If we don't have the corresponding type of socket available, then return mStatus_Invalid
	if (s < 0) return(mStatus_Invalid);

	err = sendto(s, msg, (UInt8*)end - (UInt8*)msg, 0, (struct sockaddr *)&to, to.ss_len);
	if (err < 0)
		{
		static int MessageCount = 0;
		// Don't report EHOSTDOWN (i.e. ARP failure), ENETDOWN, or no route to host for unicast destinations
		if (!mDNSAddressIsAllDNSLinkGroup(dst))
			if (errno == EHOSTDOWN || errno == ENETDOWN || errno == EHOSTUNREACH || errno == ENETUNREACH) return(mStatus_TransientErr);
		// Don't report EHOSTUNREACH in the first three minutes after boot
		// This is because mDNSResponder intentionally starts up early in the boot process (See <rdar://problem/3409090>)
		// but this means that sometimes it starts before configd has finished setting up the multicast routing entries.
		if (errno == EHOSTUNREACH && (mDNSu32)(mDNSPlatformRawTime()) < (mDNSu32)(mDNSPlatformOneSecond * 180)) return(mStatus_TransientErr);
		// Don't report EADDRNOTAVAIL ("Can't assign requested address") if we're in the middle of a network configuration change
		if (errno == EADDRNOTAVAIL && m->p->NetworkChanged) return(mStatus_TransientErr);
		if (MessageCount < 1000)
			{
			MessageCount++;
			if (errno == EHOSTUNREACH || errno == EADDRNOTAVAIL || errno == ENETDOWN)
				LogInfo("mDNSPlatformSendUDP sendto(%d) failed to send packet on InterfaceID %p %5s/%d to %#a:%d skt %d error %d errno %d (%s) %lu",
					s, InterfaceID, ifa_name, dst->type, dst, mDNSVal16(dstPort), s, err, errno, strerror(errno), (mDNSu32)(m->timenow));
			else
				LogMsg("mDNSPlatformSendUDP sendto(%d) failed to send packet on InterfaceID %p %5s/%d to %#a:%d skt %d error %d errno %d (%s) %lu",
					s, InterfaceID, ifa_name, dst->type, dst, mDNSVal16(dstPort), s, err, errno, strerror(errno), (mDNSu32)(m->timenow));
			}
		result = mStatus_UnknownErr;
		}

#ifdef IP_BOUND_IF
	if (dst->type == mDNSAddrType_IPv4 && info && !mDNSAddrIsDNSMulticast(dst))
		{
		static const mDNSu32 ifindex = 0;
		setsockopt(s, IPPROTO_IP, IP_BOUND_IF, &ifindex, sizeof(ifindex));
		}
#endif

	return(result);
	}

mDNSlocal ssize_t myrecvfrom(const int s, void *const buffer, const size_t max,
	struct sockaddr *const from, size_t *const fromlen, mDNSAddr *dstaddr, char ifname[IF_NAMESIZE], mDNSu8 *ttl)
	{
	static unsigned int numLogMessages = 0;
	struct iovec databuffers = { (char *)buffer, max };
	struct msghdr   msg;
	ssize_t         n;
	struct cmsghdr *cmPtr;
	char            ancillary[1024];

	*ttl = 255;  // If kernel fails to provide TTL data (e.g. Jaguar doesn't) then assume the TTL was 255 as it should be

	// Set up the message
	msg.msg_name       = (caddr_t)from;
	msg.msg_namelen    = *fromlen;
	msg.msg_iov        = &databuffers;
	msg.msg_iovlen     = 1;
	msg.msg_control    = (caddr_t)&ancillary;
	msg.msg_controllen = sizeof(ancillary);
	msg.msg_flags      = 0;

	// Receive the data
	n = recvmsg(s, &msg, 0);
	if (n<0)
		{
		if (errno != EWOULDBLOCK && numLogMessages++ < 100) LogMsg("mDNSMacOSX.c: recvmsg(%d) returned error %d errno %d", s, n, errno);
		return(-1);
		}
	if (msg.msg_controllen < (int)sizeof(struct cmsghdr))
		{
		if (numLogMessages++ < 100) LogMsg("mDNSMacOSX.c: recvmsg(%d) returned %d msg.msg_controllen %d < sizeof(struct cmsghdr) %lu",
			s, n, msg.msg_controllen, sizeof(struct cmsghdr));
		return(-1);
		}
	if (msg.msg_flags & MSG_CTRUNC)
		{
		if (numLogMessages++ < 100) LogMsg("mDNSMacOSX.c: recvmsg(%d) msg.msg_flags & MSG_CTRUNC", s);
		return(-1);
		}

	*fromlen = msg.msg_namelen;

	// Parse each option out of the ancillary data.
	for (cmPtr = CMSG_FIRSTHDR(&msg); cmPtr; cmPtr = CMSG_NXTHDR(&msg, cmPtr))
		{
		// debugf("myrecvfrom cmsg_level %d cmsg_type %d", cmPtr->cmsg_level, cmPtr->cmsg_type);
		if (cmPtr->cmsg_level == IPPROTO_IP && cmPtr->cmsg_type == IP_RECVDSTADDR)
			{
			dstaddr->type = mDNSAddrType_IPv4;
			dstaddr->ip.v4 = *(mDNSv4Addr*)CMSG_DATA(cmPtr);
			//LogMsg("mDNSMacOSX.c: recvmsg IP_RECVDSTADDR %.4a", &dstaddr->ip.v4);
			}
		if (cmPtr->cmsg_level == IPPROTO_IP && cmPtr->cmsg_type == IP_RECVIF)
			{
			struct sockaddr_dl *sdl = (struct sockaddr_dl *)CMSG_DATA(cmPtr);
			if (sdl->sdl_nlen < IF_NAMESIZE)
				{
				mDNSPlatformMemCopy(ifname, sdl->sdl_data, sdl->sdl_nlen);
				ifname[sdl->sdl_nlen] = 0;
				// debugf("IP_RECVIF sdl_index %d, sdl_data %s len %d", sdl->sdl_index, ifname, sdl->sdl_nlen);
				}
			}
		if (cmPtr->cmsg_level == IPPROTO_IP && cmPtr->cmsg_type == IP_RECVTTL)
			*ttl = *(u_char*)CMSG_DATA(cmPtr);
		if (cmPtr->cmsg_level == IPPROTO_IPV6 && cmPtr->cmsg_type == IPV6_PKTINFO)
			{
			struct in6_pktinfo *ip6_info = (struct in6_pktinfo*)CMSG_DATA(cmPtr);
			dstaddr->type = mDNSAddrType_IPv6;
			dstaddr->ip.v6 = *(mDNSv6Addr*)&ip6_info->ipi6_addr;
			myIfIndexToName(ip6_info->ipi6_ifindex, ifname);
			}
		if (cmPtr->cmsg_level == IPPROTO_IPV6 && cmPtr->cmsg_type == IPV6_HOPLIMIT)
			*ttl = *(int*)CMSG_DATA(cmPtr);
		}

	return(n);
	}

mDNSlocal void myKQSocketCallBack(int s1, short filter, void *context)
	{
	KQSocketSet *const ss = (KQSocketSet *)context;
	mDNS *const m = ss->m;
	int err = 0, count = 0, closed = 0;

	if (filter != EVFILT_READ)
		LogMsg("myKQSocketCallBack: Why is filter %d not EVFILT_READ (%d)?", filter, EVFILT_READ);

	if (s1 != ss->sktv4
#ifndef NO_IPV6
		&& s1 != ss->sktv6
#endif
		)
		{
		LogMsg("myKQSocketCallBack: native socket %d", s1);
		LogMsg("myKQSocketCallBack: sktv4 %d", ss->sktv4);
#ifndef NO_IPV6
		LogMsg("myKQSocketCallBack: sktv6 %d", ss->sktv6);
#endif
 		}

	while (!closed)
		{
		mDNSAddr senderAddr, destAddr;
		mDNSIPPort senderPort;
		struct sockaddr_storage from;
		size_t fromlen = sizeof(from);
		char packetifname[IF_NAMESIZE] = "";
		mDNSu8 ttl;
		err = myrecvfrom(s1, &m->imsg, sizeof(m->imsg), (struct sockaddr *)&from, &fromlen, &destAddr, packetifname, &ttl);
		if (err < 0) break;

		count++;
		if (from.ss_family == AF_INET)
			{
			struct sockaddr_in *s = (struct sockaddr_in*)&from;
			senderAddr.type = mDNSAddrType_IPv4;
			senderAddr.ip.v4.NotAnInteger = s->sin_addr.s_addr;
			senderPort.NotAnInteger = s->sin_port;
			//LogInfo("myKQSocketCallBack received IPv4 packet from %#-15a to %#-15a on skt %d %s", &senderAddr, &destAddr, s1, packetifname);
			}
		else if (from.ss_family == AF_INET6)
			{
			struct sockaddr_in6 *sin6 = (struct sockaddr_in6*)&from;
			senderAddr.type = mDNSAddrType_IPv6;
			senderAddr.ip.v6 = *(mDNSv6Addr*)&sin6->sin6_addr;
			senderPort.NotAnInteger = sin6->sin6_port;
			//LogInfo("myKQSocketCallBack received IPv6 packet from %#-15a to %#-15a on skt %d %s", &senderAddr, &destAddr, s1, packetifname);
			}
		else
			{
			LogMsg("myKQSocketCallBack from is unknown address family %d", from.ss_family);
			return;
			}

		// Note: When handling multiple packets in a batch, MUST reset InterfaceID before handling each packet
		mDNSInterfaceID InterfaceID = mDNSNULL;
		//NetworkInterfaceInfo *intf = m->HostInterfaces;
		//while (intf && strcmp(intf->ifname, packetifname)) intf = intf->next;

		NetworkInterfaceInfoOSX *intf = m->p->InterfaceList;
		while (intf && strcmp(intf->ifinfo.ifname, packetifname)) intf = intf->next;

		// When going to sleep we deregister all our interfaces, but if the machine
		// takes a few seconds to sleep we may continue to receive multicasts
		// during that time, which would confuse mDNSCoreReceive, because as far
		// as it's concerned, we should have no active interfaces any more.
		// Hence we ignore multicasts for which we can find no matching InterfaceID.
		if (intf) InterfaceID = intf->ifinfo.InterfaceID;
		else if (mDNSAddrIsDNSMulticast(&destAddr)) continue;

//		LogMsg("myKQSocketCallBack got packet from %#a to %#a on interface %#a/%s",
//			&senderAddr, &destAddr, &ss->info->ifinfo.ip, ss->info->ifinfo.ifname);

		// mDNSCoreReceive may close the socket we're reading from.  We must break out of our
		// loop when that happens, or we may try to read from an invalid FD.  We do this by
		// setting the closeFlag pointer in the socketset, so CloseSocketSet can inform us
		// if it closes the socketset.
		ss->closeFlag = &closed;

		mDNSCoreReceive(m, &m->imsg, (unsigned char*)&m->imsg + err, &senderAddr, senderPort, &destAddr, ss->port, InterfaceID);

		// if we didn't close, we can safely dereference the socketset, and should to
		// reset the closeFlag, since it points to something on the stack
		if (!closed) ss->closeFlag = mDNSNULL;
		}

	if (err < 0 && (errno != EWOULDBLOCK || count == 0))
		{
		// Something is busted here.
		// kqueue says there is a packet, but myrecvfrom says there is not.
		// Try calling select() to get another opinion.
		// Find out about other socket parameter that can help understand why select() says the socket is ready for read
		// All of this is racy, as data may have arrived after the call to select()
		static unsigned int numLogMessages = 0;
		int save_errno = errno;
		int so_error = -1;
		int so_nread = -1;
		int fionread = -1;
		socklen_t solen = sizeof(int);
		fd_set readfds;
		struct timeval timeout;
		int selectresult;
		FD_ZERO(&readfds);
		FD_SET(s1, &readfds);
		timeout.tv_sec  = 0;
		timeout.tv_usec = 0;
		selectresult = select(s1+1, &readfds, NULL, NULL, &timeout);
		if (getsockopt(s1, SOL_SOCKET, SO_ERROR, &so_error, &solen) == -1)
			LogMsg("myKQSocketCallBack getsockopt(SO_ERROR) error %d", errno);
		if (getsockopt(s1, SOL_SOCKET, SO_NREAD, &so_nread, &solen) == -1)
			LogMsg("myKQSocketCallBack getsockopt(SO_NREAD) error %d", errno);
		if (ioctl(s1, FIONREAD, &fionread) == -1)
			LogMsg("myKQSocketCallBack ioctl(FIONREAD) error %d", errno);
		if (numLogMessages++ < 100)
			LogMsg("myKQSocketCallBack recvfrom skt %d error %d errno %d (%s) select %d (%spackets waiting) so_error %d so_nread %d fionread %d count %d",
				s1, err, save_errno, strerror(save_errno), selectresult, FD_ISSET(s1, &readfds) ? "" : "*NO* ", so_error, so_nread, fionread, count);
		if (numLogMessages > 5)
			NotifyOfElusiveBug("Flaw in Kernel (select/recvfrom mismatch)",
				"Congratulations, you've reproduced an elusive bug.\r"
				"Please contact the current assignee of <rdar://problem/3375328>.\r"
				"Alternatively, you can send email to radar-3387020@group.apple.com. (Note number is different.)\r"
				"If possible, please leave your machine undisturbed so that someone can come to investigate the problem.");

		sleep(1);		// After logging this error, rate limit so we don't flood syslog
		}
	}

// TCP socket support

typedef enum
	{
	handshake_required,
	handshake_in_progress,
	handshake_completed,
	handshake_to_be_closed
	} handshakeStatus;
	
struct TCPSocket_struct
	{
	TCPSocketFlags flags;		// MUST BE FIRST FIELD -- mDNSCore expects every TCPSocket_struct to begin with TCPSocketFlags flags
	TCPConnectionCallback callback;
	int fd;
	KQueueEntry *kqEntry;	
	KQSocketSet ss;
#ifndef NO_SECURITYFRAMEWORK
	SSLContextRef tlsContext;
	pthread_t handshake_thread;
#endif /* NO_SECURITYFRAMEWORK */
	domainname hostname;
	void *context;
	mDNSBool setup;
	mDNSBool connected;
	handshakeStatus handshake;
	mDNS *m; // So we can call KQueueLock from the SSLHandshake thread
	mStatus err;
	};

mDNSlocal void doTcpSocketCallback(TCPSocket *sock)
	{
	mDNSBool c = !sock->connected;
	sock->connected = mDNStrue;
	sock->callback(sock, sock->context, c, sock->err);
	// Note: the callback may call CloseConnection here, which frees the context structure!
	}

#ifndef NO_SECURITYFRAMEWORK

mDNSlocal OSStatus tlsWriteSock(SSLConnectionRef connection, const void *data, size_t *dataLength)
	{
	int ret = send(((TCPSocket *)connection)->fd, data, *dataLength, 0);
	if (ret >= 0 && (size_t)ret < *dataLength) { *dataLength = ret; return(errSSLWouldBlock); }
	if (ret >= 0)                              { *dataLength = ret; return(noErr); }
	*dataLength = 0;
	if (errno == EAGAIN                      ) return(errSSLWouldBlock);
	if (errno == ENOENT                      ) return(errSSLClosedGraceful);
	if (errno == EPIPE || errno == ECONNRESET) return(errSSLClosedAbort);
	LogMsg("ERROR: tlsWriteSock: %d error %d (%s)\n", ((TCPSocket *)connection)->fd, errno, strerror(errno));
	return(errSSLClosedAbort);
	}

mDNSlocal OSStatus tlsReadSock(SSLConnectionRef connection, void *data, size_t *dataLength)
	{
	int ret = recv(((TCPSocket *)connection)->fd, data, *dataLength, 0);
	if (ret > 0 && (size_t)ret < *dataLength) { *dataLength = ret; return(errSSLWouldBlock); }
	if (ret > 0)                              { *dataLength = ret; return(noErr); }
	*dataLength = 0;
	if (ret == 0 || errno == ENOENT    ) return(errSSLClosedGraceful);
	if (            errno == EAGAIN    ) return(errSSLWouldBlock);
	if (            errno == ECONNRESET) return(errSSLClosedAbort);
	LogMsg("ERROR: tlsSockRead: error %d (%s)\n", errno, strerror(errno));
	return(errSSLClosedAbort);
	}

mDNSlocal OSStatus tlsSetupSock(TCPSocket *sock, mDNSBool server)
	{
	char domname_cstr[MAX_ESCAPED_DOMAIN_NAME];

	mStatus err = SSLNewContext(server, &sock->tlsContext);
	if (err) { LogMsg("ERROR: tlsSetupSock: SSLNewContext failed with error code: %d", err); return(err); }

	err = SSLSetIOFuncs(sock->tlsContext, tlsReadSock, tlsWriteSock);
	if (err) { LogMsg("ERROR: tlsSetupSock: SSLSetIOFuncs failed with error code: %d", err); return(err); }

	err = SSLSetConnection(sock->tlsContext, (SSLConnectionRef) sock);
	if (err) { LogMsg("ERROR: tlsSetupSock: SSLSetConnection failed with error code: %d", err); return(err); }

	// Instead of listing all the acceptable ciphers, we just disable the bad ciphers. It does not disable
	// all the bad ciphers like RC4_MD5, but it assumes that the servers don't offer them.
	err = SSLSetAllowAnonymousCiphers(sock->tlsContext, 0);
	if (err) { LogMsg("ERROR: tlsSetupSock: SSLSetAllowAnonymousCiphers failed with error code: %d", err); return(err); }

	// We already checked for NULL in hostname and this should never happen. Hence, returning -1
	// (error not in OSStatus space) is okay.
	if (!sock->hostname.c[0]) {LogMsg("ERROR: tlsSetupSock: hostname NULL"); return -1; }

	ConvertDomainNameToCString(&sock->hostname, domname_cstr);
	err = SSLSetPeerDomainName(sock->tlsContext, domname_cstr, strlen(domname_cstr));
	if (err) { LogMsg("ERROR: tlsSetupSock: SSLSetPeerDomainname: %s failed with error code: %d", domname_cstr, err); return(err); }

	return(err);
	}

#ifdef MDNSRESPONDER_USES_LIB_DISPATCH_AS_PRIMARY_EVENT_LOOP_MECHANISM
mDNSlocal void doSSLHandshake(void *ctx)
	{
	TCPSocket *sock = (TCPSocket*)ctx;
	mStatus err = SSLHandshake(sock->tlsContext);
	
	//Can't have multiple threads in mDNS core. When MDNSRESPONDER_USES_LIB_DISPATCH_AS_PRIMARY_EVENT_LOOP_MECHANISM is
	//defined, KQueueLock is a noop. Hence we need to serialize here
	//
	//NOTE: We just can't serialize doTcpSocketCallback alone on the main queue.
	//We need the rest of the logic also. Otherwise, we can enable the READ
	//events below, dispatch a doTcpSocketCallback on the main queue. Assume it is
	//ConnFailed which means we are going to free the tcpInfo. While it
	//is waiting to be dispatched, another read event can come into tcpKQSocketCallback
	//and potentially call doTCPCallback with error which can close the fd and free the
	//tcpInfo. Later when the thread gets dispatched it will crash because the tcpInfo
	//is already freed.

	dispatch_async(dispatch_get_main_queue(), ^{

		LogInfo("doSSLHandshake %p: got lock", sock); // Log *after* we get the lock

		if (sock->handshake == handshake_to_be_closed)
			{
			LogInfo("SSLHandshake completed after close");
			mDNSPlatformTCPCloseConnection(sock);
			}
		else
			{
			if (sock->fd != -1) KQueueSet(sock->fd, EV_ADD, EVFILT_READ, sock->kqEntry);
			else LogMsg("doSSLHandshake: sock->fd is -1");
	
			if (err == errSSLWouldBlock)
				sock->handshake = handshake_required;
			else
				{
				if (err)
					{
					LogMsg("SSLHandshake failed: %d%s", err, err == errSSLPeerInternalError ? " (server busy)" : "");
					SSLDisposeContext(sock->tlsContext);
					sock->tlsContext = NULL;
					}
				
				sock->err = err ? mStatus_ConnFailed : 0;
				sock->handshake = handshake_completed;
				
				LogInfo("doSSLHandshake: %p calling doTcpSocketCallback fd %d", sock, sock->fd);
				doTcpSocketCallback(sock);
				}
			}
	
		LogInfo("SSLHandshake %p: dropping lock for fd %d", sock, sock->fd);
		return;
		});
	}
#else
mDNSlocal void *doSSLHandshake(void *ctx)
	{
	// Warning: Touching sock without the kqueue lock!
	// We're protected because sock->handshake == handshake_in_progress
	TCPSocket *sock = (TCPSocket*)ctx;
	mDNS * const m = sock->m; // Get m now, as we may free sock if marked to be closed while we're waiting on SSLHandshake
	mStatus err = SSLHandshake(sock->tlsContext);
	
	KQueueLock(m);
	debugf("doSSLHandshake %p: got lock", sock); // Log *after* we get the lock

	if (sock->handshake == handshake_to_be_closed)
		{
		LogInfo("SSLHandshake completed after close");
		mDNSPlatformTCPCloseConnection(sock);
		}
	else
		{
		if (sock->fd != -1) KQueueSet(sock->fd, EV_ADD, EVFILT_READ, sock->kqEntry);
		else LogMsg("doSSLHandshake: sock->fd is -1");

		if (err == errSSLWouldBlock)
			sock->handshake = handshake_required;
		else
			{
			if (err)
				{
				LogMsg("SSLHandshake failed: %d%s", err, err == errSSLPeerInternalError ? " (server busy)" : "");
				SSLDisposeContext(sock->tlsContext);
				sock->tlsContext = NULL;
				}
			
			sock->err = err ? mStatus_ConnFailed : 0;
			sock->handshake = handshake_completed;
			
			debugf("doSSLHandshake: %p calling doTcpSocketCallback fd %d", sock, sock->fd);
			doTcpSocketCallback(sock);
			}
		}
	
	debugf("SSLHandshake %p: dropping lock for fd %d", sock, sock->fd);
	KQueueUnlock(m, "doSSLHandshake");
	return NULL;
	}
#endif

mDNSlocal mStatus spawnSSLHandshake(TCPSocket* sock)
	{
	debugf("spawnSSLHandshake %p: entry", sock);
	mStatus err;

	if (sock->handshake != handshake_required) LogMsg("spawnSSLHandshake: handshake status not required: %d", sock->handshake);
	sock->handshake = handshake_in_progress;
	KQueueSet(sock->fd, EV_DELETE, EVFILT_READ, sock->kqEntry);
#ifdef MDNSRESPONDER_USES_LIB_DISPATCH_AS_PRIMARY_EVENT_LOOP_MECHANISM

	// Dispatch it on a separate serial queue to avoid deadlocks with threads running on main queue
	dispatch_async(SSLqueue, ^{doSSLHandshake(sock);});
	err = 0;
#else
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	err = pthread_create(&sock->handshake_thread, &attr, doSSLHandshake, sock);
	pthread_attr_destroy(&attr);
	if (err)
		{
		LogMsg("Could not start SSLHandshake thread: (%d) %s", err, strerror(err));
		sock->handshake = handshake_completed;
		sock->err = err;
		KQueueSet(sock->fd, EV_ADD, EVFILT_READ, sock->kqEntry);
		}
#endif
	debugf("spawnSSLHandshake %p: done for %d", sock, sock->fd);
	return err;
	}

mDNSlocal mDNSBool IsTunnelModeDomain(const domainname *d)
	{
	static const domainname *mmc = (const domainname*) "\x7" "members" "\x3" "mac" "\x3" "com";
	const domainname *d1 = mDNSNULL;	// TLD
	const domainname *d2 = mDNSNULL;	// SLD
	const domainname *d3 = mDNSNULL;
	while (d->c[0]) { d3 = d2; d2 = d1; d1 = d; d = (const domainname*)(d->c + 1 + d->c[0]); }
	return(d3 && SameDomainName(d3, mmc));
	}

#endif /* NO_SECURITYFRAMEWORK */

mDNSlocal void tcpKQSocketCallback(__unused int fd, short filter, void *context)
	{
	TCPSocket *sock = context;
	sock->err = mStatus_NoError;

	//if (filter == EVFILT_READ ) LogMsg("myKQSocketCallBack: tcpKQSocketCallback %d is EVFILT_READ", filter);
	//if (filter == EVFILT_WRITE) LogMsg("myKQSocketCallBack: tcpKQSocketCallback %d is EVFILT_WRITE", filter);
	// EV_ONESHOT doesn't seem to work, so we add the filter with EV_ADD, and explicitly delete it here with EV_DELETE
	if (filter == EVFILT_WRITE) KQueueSet(sock->fd, EV_DELETE, EVFILT_WRITE, sock->kqEntry);

	if (sock->flags & kTCPSocketFlags_UseTLS)
		{
#ifndef NO_SECURITYFRAMEWORK
		if (!sock->setup) { sock->setup = mDNStrue; tlsSetupSock(sock, mDNSfalse); }
		
		if (sock->handshake == handshake_required) { if (spawnSSLHandshake(sock) == 0) return; }
		else if (sock->handshake == handshake_in_progress || sock->handshake == handshake_to_be_closed) return;
		else if (sock->handshake != handshake_completed)
			{
			if (!sock->err) sock->err = mStatus_UnknownErr;
			LogMsg("tcpKQSocketCallback called with unexpected SSLHandshake status: %d", sock->handshake);
			}
#else
		sock->err = mStatus_UnsupportedErr;
#endif /* NO_SECURITYFRAMEWORK */
		}
	
	doTcpSocketCallback(sock);
	}

#ifdef MDNSRESPONDER_USES_LIB_DISPATCH_AS_PRIMARY_EVENT_LOOP_MECHANISM
mDNSexport int KQueueSet(int fd, u_short flags, short filter, KQueueEntry *const entryRef)
	{
	dispatch_queue_t queue = dispatch_get_main_queue();
	dispatch_source_t source;
	if (flags == EV_DELETE)
		{
		if (filter == EVFILT_READ)
			{
			dispatch_source_cancel(entryRef->readSource);
			dispatch_release(entryRef->readSource);
			entryRef->readSource = mDNSNULL;
			debugf("KQueueSet: source cancel for read %p, %p", entryRef->readSource, entryRef->writeSource);
			}
		else if (filter == EVFILT_WRITE)
			{
			dispatch_source_cancel(entryRef->writeSource);
			dispatch_release(entryRef->writeSource);
			entryRef->writeSource = mDNSNULL;
			debugf("KQueueSet: source cancel for write %p, %p", entryRef->readSource, entryRef->writeSource);
			}
		else
			LogMsg("KQueueSet: ERROR: Wrong filter value %d for EV_DELETE", filter);
		return 0;
		}
	if (flags != EV_ADD) LogMsg("KQueueSet: Invalid flags %d", flags);

	if (filter == EVFILT_READ)
		{
		source = dispatch_source_create(DISPATCH_SOURCE_TYPE_READ, fd, 0, queue);
		}
	else if (filter == EVFILT_WRITE)
		{
		source = dispatch_source_create(DISPATCH_SOURCE_TYPE_WRITE, fd, 0, queue);
		}
	else 
		{
		LogMsg("KQueueSet: ERROR: Wrong filter value %d for EV_ADD", filter);
		return -1;
		}
	if (!source) return -1;
	dispatch_source_set_event_handler(source, ^{

		mDNSs32 stime = mDNSPlatformRawTime();
		entryRef->KQcallback(fd, filter, entryRef->KQcontext);
		mDNSs32 etime = mDNSPlatformRawTime();
		if (etime - stime >= WatchDogReportingThreshold)
			LogInfo("KQEntryCallback Block: WARNING: took %dms to complete", etime - stime);

		// Trigger the event delivery to the application. Even though we trigger the
		// event completion after handling every event source, these all will hopefully
		// get merged
		TriggerEventCompletion();

		});
    dispatch_source_set_cancel_handler(source, ^{
		if (entryRef->fdClosed)
			{
			//LogMsg("CancelHandler: closing fd %d", fd);
			close(fd);
			}
		});
	dispatch_resume(source);
	if (filter == EVFILT_READ)
		entryRef->readSource = source;
	else
		entryRef->writeSource = source;
		
	return 0;
	}

mDNSexport void KQueueLock(mDNS *const m)
	{
	(void)m; //unused
	}
mDNSexport void KQueueUnlock(mDNS *const m, const char const *task)
	{
	(void)m; //unused
	(void)task; //unused
	}
#else
mDNSexport int KQueueSet(int fd, u_short flags, short filter, const KQueueEntry *const entryRef)
	{
	struct kevent new_event;
	EV_SET(&new_event, fd, filter, flags, 0, 0, (void*)entryRef);
	return (kevent(KQueueFD, &new_event, 1, NULL, 0, NULL) < 0) ? errno : 0;
	}

mDNSexport void KQueueLock(mDNS *const m)
	{
	pthread_mutex_lock(&m->p->BigMutex);
	m->p->BigMutexStartTime = mDNSPlatformRawTime();
	}

mDNSexport void KQueueUnlock(mDNS *const m, const char const *task)
	{
	mDNSs32 end = mDNSPlatformRawTime();
	(void)task;
	if (end - m->p->BigMutexStartTime >= WatchDogReportingThreshold)
		LogInfo("WARNING: %s took %dms to complete", task, end - m->p->BigMutexStartTime);

	pthread_mutex_unlock(&m->p->BigMutex);

	char wake = 1;
	if (send(m->p->WakeKQueueLoopFD, &wake, sizeof(wake), 0) == -1)
		LogMsg("ERROR: KQueueWake: send failed with error code: %d (%s)", errno, strerror(errno));
	}
#endif

mDNSexport void mDNSPlatformCloseFD(KQueueEntry *kq, int fd)
	{
#ifdef MDNSRESPONDER_USES_LIB_DISPATCH_AS_PRIMARY_EVENT_LOOP_MECHANISM
	(void)fd; //unused
	if (kq->readSource)
		{
		dispatch_source_cancel(kq->readSource);
		kq->readSource = mDNSNULL;
		}
	if (kq->writeSource)
		{	
		dispatch_source_cancel(kq->writeSource);
		kq->writeSource = mDNSNULL;
		}
	// Close happens in the cancellation handler
	debugf("mDNSPlatformCloseFD: resetting sources for %d", fd);
	kq->fdClosed = mDNStrue;
#else
	(void)kq; //unused
	close(fd);
#endif
	}

mDNSlocal mStatus SetupTCPSocket(TCPSocket *sock, u_short sa_family, mDNSIPPort *port)
	{
	KQSocketSet *cp = &sock->ss;
#ifndef NO_IPV6
    int         *s        = (sa_family == AF_INET) ? &cp->sktv4 : &cp->sktv6;
    KQueueEntry *k        = (sa_family == AF_INET) ? &cp->kqsv4 : &cp->kqsv6;
#else
    int         *s        = &cp->sktv4;
    KQueueEntry *k        = &cp->kqsv4;
#endif
	const int on = 1;  // "on" for setsockopt
	mStatus err;

    int skt = socket(sa_family, SOCK_STREAM, IPPROTO_TCP);
    if (skt < 3) { if (errno != EAFNOSUPPORT) LogMsg("SetupTCPSocket: socket error %d errno %d (%s)", skt, errno, strerror(errno)); return(skt); }
	if (sa_family == AF_INET)
		{
		// Bind it
		struct sockaddr_in addr;
		mDNSPlatformMemZero(&addr, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_port = port->NotAnInteger;
		err = bind(skt, (struct sockaddr*) &addr, sizeof(addr));
		if (err < 0) { LogMsg("ERROR: bind %s", strerror(errno)); return err; }
	
		// Receive interface identifiers
		err = setsockopt(skt, IPPROTO_IP, IP_RECVIF, &on, sizeof(on));
		if (err < 0) { LogMsg("setsockopt IP_RECVIF - %s", strerror(errno)); return err; }
	
		mDNSPlatformMemZero(&addr, sizeof(addr));
		socklen_t len = sizeof(addr);
		err = getsockname(skt, (struct sockaddr*) &addr, &len);
		if (err < 0) { LogMsg("getsockname - %s", strerror(errno)); return err; }
	
		port->NotAnInteger = addr.sin_port;
		}
	else
		{
		// Bind it
		struct sockaddr_in6 addr6;
		mDNSPlatformMemZero(&addr6, sizeof(addr6));
		addr6.sin6_family = AF_INET6;
		addr6.sin6_port = port->NotAnInteger;
		err = bind(skt, (struct sockaddr*) &addr6, sizeof(addr6));
		if (err < 0) { LogMsg("ERROR: bind6 %s", strerror(errno)); return err; }

		// We want to receive destination addresses and receive interface identifiers
        err = setsockopt(skt, IPPROTO_IPV6, IPV6_RECVPKTINFO, &on, sizeof(on));
		if (err < 0) { LogMsg("ERROR: setsockopt IPV6_RECVPKTINFO %s", strerror(errno)); return err; }

		mDNSPlatformMemZero(&addr6, sizeof(addr6));
		socklen_t len = sizeof(addr6);
		err = getsockname(skt, (struct sockaddr *) &addr6, &len);
		if (err < 0) { LogMsg("getsockname6 - %s", strerror(errno)); return err; }
	
		port->NotAnInteger = addr6.sin6_port;

		}
	*s = skt;
	k->KQcallback = tcpKQSocketCallback;
	k->KQcontext  = sock;
	k->KQtask     = "mDNSPlatformTCPSocket";
#ifdef MDNSRESPONDER_USES_LIB_DISPATCH_AS_PRIMARY_EVENT_LOOP_MECHANISM
	k->readSource = mDNSNULL;
	k->writeSource = mDNSNULL;
	k->fdClosed = mDNSfalse;
#endif
	return mStatus_NoError;
	}

mDNSexport TCPSocket *mDNSPlatformTCPSocket(mDNS *const m, TCPSocketFlags flags, mDNSIPPort *port)
	{
	mStatus err;
	(void) m;

	TCPSocket *sock = mallocL("TCPSocket/mDNSPlatformTCPSocket", sizeof(TCPSocket));
	if (!sock) { LogMsg("mDNSPlatformTCPSocket: memory allocation failure"); return(mDNSNULL); }

	mDNSPlatformMemZero(sock, sizeof(TCPSocket));

	sock->ss.m     = m;
	sock->ss.sktv4 = -1;
#ifndef NO_IPV6
	sock->ss.sktv6 = -1;
#endif
	err = SetupTCPSocket(sock, AF_INET, port);
#ifndef NO_IPV6
	if (!err)
		{
		err = SetupTCPSocket(sock, AF_INET6, port);
		if (err) { mDNSPlatformCloseFD(&sock->ss.kqsv4, sock->ss.sktv4); sock->ss.sktv4 = -1; }
		}
#endif
	if (err)
		{
		LogMsg("mDNSPlatformTCPSocket: socket error %d errno %d (%s)", sock->fd, errno, strerror(errno));
		freeL("TCPSocket/mDNSPlatformTCPSocket", sock);
		return(mDNSNULL);
		}

	sock->callback          = mDNSNULL;
	sock->flags             = flags;
	sock->context           = mDNSNULL;
	sock->setup             = mDNSfalse;
	sock->connected         = mDNSfalse;
	sock->handshake         = handshake_required;
	sock->m                 = m;
	sock->err               = mStatus_NoError;
	
	return sock;
	}

mDNSexport mStatus mDNSPlatformTCPConnect(TCPSocket *sock, const mDNSAddr *dst, mDNSOpaque16 dstport, domainname *hostname, mDNSInterfaceID InterfaceID, TCPConnectionCallback callback, void *context)
	{
	KQSocketSet *cp = &sock->ss;
#ifndef NO_IPV6
    int         *s        = (dst->type == mDNSAddrType_IPv4) ? &cp->sktv4 : &cp->sktv6;
    KQueueEntry *k        = (dst->type == mDNSAddrType_IPv4) ? &cp->kqsv4 : &cp->kqsv6;
#else
    int         *s        = &cp->sktv4;
    KQueueEntry *k        = &cp->kqsv4;
#endif
	mStatus err = mStatus_NoError;
	struct sockaddr_storage ss;

	sock->callback          = callback;
	sock->context           = context;
	sock->setup             = mDNSfalse;
	sock->connected         = mDNSfalse;
	sock->handshake         = handshake_required;
	sock->err               = mStatus_NoError;

	if (hostname) { debugf("mDNSPlatformTCPConnect: hostname %##s", hostname->c); AssignDomainName(&sock->hostname, hostname); }

	if (dst->type == mDNSAddrType_IPv4)
		{
		struct sockaddr_in *saddr = (struct sockaddr_in *)&ss;
		mDNSPlatformMemZero(saddr, sizeof(*saddr));
		saddr->sin_family      = AF_INET;
		saddr->sin_port        = dstport.NotAnInteger;
		saddr->sin_len         = sizeof(*saddr);
		saddr->sin_addr.s_addr = dst->ip.v4.NotAnInteger;
		}
	else
		{
		struct sockaddr_in6 *saddr6 = (struct sockaddr_in6 *)&ss;
		mDNSPlatformMemZero(saddr6, sizeof(*saddr6));
		saddr6->sin6_family      = AF_INET6;
		saddr6->sin6_port        = dstport.NotAnInteger;
		saddr6->sin6_len         = sizeof(*saddr6);
		saddr6->sin6_addr        = *(struct in6_addr *)&dst->ip.v6;
		}

	// Watch for connect complete (write is ready)
	// EV_ONESHOT doesn't seem to work, so we add the filter with EV_ADD, and explicitly delete it in tcpKQSocketCallback using EV_DELETE
	if (KQueueSet(*s, EV_ADD /* | EV_ONESHOT */, EVFILT_WRITE, k))
		{
		LogMsg("ERROR: mDNSPlatformTCPConnect - KQueueSet failed");
		return errno;
		}

	// Watch for incoming data
	if (KQueueSet(*s, EV_ADD, EVFILT_READ, k))
		{
		LogMsg("ERROR: mDNSPlatformTCPConnect - KQueueSet failed");
		return errno;
 		}

	if (fcntl(*s, F_SETFL, fcntl(*s, F_GETFL, 0) | O_NONBLOCK) < 0) // set non-blocking
		{
		LogMsg("ERROR: setsockopt O_NONBLOCK - %s", strerror(errno));
		return mStatus_UnknownErr;
		}

	// We bind to the interface and all subsequent packets including the SYN will be sent out
	// on this interface
	//
	// Note: If we are in Active Directory domain, we may try TCP (if the response can't fit in
	// UDP). mDNSInterface_Unicast indicates this case and not a valid interface.
	if (InterfaceID && InterfaceID != mDNSInterface_Unicast)
		{
		extern mDNS mDNSStorage;
		NetworkInterfaceInfoOSX *info = IfindexToInterfaceInfoOSX(&mDNSStorage, InterfaceID);
		if (dst->type == mDNSAddrType_IPv4)
			{
		#ifdef IP_BOUND_IF
			if (info) setsockopt(*s, IPPROTO_IP, IP_BOUND_IF, &info->scope_id, sizeof(info->scope_id));
			else { LogMsg("mDNSPlatformTCPConnect: Invalid interface index %p", InterfaceID); return mStatus_BadParamErr; }
		#else
			(void)InterfaceID; // Unused
			(void)info; // Unused
		#endif
			}
		else
			{
		#ifdef IPV6_BOUND_IF
			if (info) setsockopt(*s, IPPROTO_IPV6, IPV6_BOUND_IF, &info->scope_id, sizeof(info->scope_id));
			else { LogMsg("mDNSPlatformTCPConnect: Invalid interface index %p", InterfaceID); return mStatus_BadParamErr; }
		#else
			(void)InterfaceID; // Unused
			(void)info; // Unused
		#endif
			}
		}

	// mDNSPlatformReadTCP/WriteTCP (unlike the UDP counterpart) does not provide the destination address
	// from which we can infer the destination address family. Hence we need to remember that here.
	// Instead of remembering the address family, we remember the right fd.
	sock->fd = *s;
	sock->kqEntry = k;
	// initiate connection wth peer
	if (connect(*s, (struct sockaddr *)&ss, ss.ss_len) < 0)
		{
		if (errno == EINPROGRESS) return mStatus_ConnPending;
		if (errno == EHOSTUNREACH || errno == EADDRNOTAVAIL || errno == ENETDOWN)
			LogInfo("ERROR: mDNSPlatformTCPConnect - connect failed: socket %d: Error %d (%s)", sock->fd, errno, strerror(errno));
		else
			LogMsg("ERROR: mDNSPlatformTCPConnect - connect failed: socket %d: Error %d (%s) length %d", sock->fd, errno, strerror(errno), ss.ss_len);
		return mStatus_ConnFailed;
		}

	LogMsg("NOTE: mDNSPlatformTCPConnect completed synchronously");
	// kQueue should notify us, but this LogMsg is to help track down if it doesn't
	return err;
	}

// Why doesn't mDNSPlatformTCPAccept actually call accept() ?
mDNSexport TCPSocket *mDNSPlatformTCPAccept(TCPSocketFlags flags, int fd)
	{
	mStatus err = mStatus_NoError;

	TCPSocket *sock = mallocL("TCPSocket/mDNSPlatformTCPAccept", sizeof(TCPSocket));
	if (!sock) return(mDNSNULL);

	mDNSPlatformMemZero(sock, sizeof(*sock));
	sock->fd = fd;
	sock->flags = flags;

	if (flags & kTCPSocketFlags_UseTLS)
		{
#ifndef NO_SECURITYFRAMEWORK
		if (!ServerCerts) { LogMsg("ERROR: mDNSPlatformTCPAccept: unable to find TLS certificates"); err = mStatus_UnknownErr; goto exit; }

		err = tlsSetupSock(sock, mDNStrue);
		if (err) { LogMsg("ERROR: mDNSPlatformTCPAccept: tlsSetupSock failed with error code: %d", err); goto exit; }

		err = SSLSetCertificate(sock->tlsContext, ServerCerts);
		if (err) { LogMsg("ERROR: mDNSPlatformTCPAccept: SSLSetCertificate failed with error code: %d", err); goto exit; }
#else
		err = mStatus_UnsupportedErr;
#endif /* NO_SECURITYFRAMEWORK */
		}
#ifndef NO_SECURITYFRAMEWORK
exit:
#endif

	if (err) { freeL("TCPSocket/mDNSPlatformTCPAccept", sock); return(mDNSNULL); }
	return(sock);
	}

mDNSlocal void CloseSocketSet(KQSocketSet *ss)
	{
	if (ss->sktv4 != -1)
		{
		mDNSPlatformCloseFD(&ss->kqsv4,  ss->sktv4);
		ss->sktv4 = -1;
		}
#ifndef NO_IPV6
	if (ss->sktv6 != -1)
		{
		mDNSPlatformCloseFD(&ss->kqsv6,  ss->sktv6);
		ss->sktv6 = -1;
		}
#endif
	if (ss->closeFlag) *ss->closeFlag = 1;
	}

mDNSexport void mDNSPlatformTCPCloseConnection(TCPSocket *sock)
	{
	if (sock)
		{	
#ifndef NO_SECURITYFRAMEWORK
		if (sock->tlsContext)
			{
			if (sock->handshake == handshake_in_progress) // SSLHandshake thread using this sock (esp. tlsContext)
				{
				LogInfo("mDNSPlatformTCPCloseConnection: called while handshake in progress");
				// When we come back from SSLHandshake, we will notice that a close was here and
				// call this function again which will do the cleanup then.
				sock->handshake = handshake_to_be_closed;
				return;
				}

			SSLClose(sock->tlsContext);
			SSLDisposeContext(sock->tlsContext);
			sock->tlsContext = NULL;
			}
#endif /* NO_SECURITYFRAMEWORK */
			if (sock->ss.sktv4 != -1) shutdown(sock->ss.sktv4, 2);
#ifndef NO_IPV6
			if (sock->ss.sktv6 != -1) shutdown(sock->ss.sktv6, 2);
#endif
			CloseSocketSet(&sock->ss);
			sock->fd = -1;

		freeL("TCPSocket/mDNSPlatformTCPCloseConnection", sock);
		}
	}

mDNSexport long mDNSPlatformReadTCP(TCPSocket *sock, void *buf, unsigned long buflen, mDNSBool *closed)
	{
	size_t nread = 0;
	*closed = mDNSfalse;

	if (sock->flags & kTCPSocketFlags_UseTLS)
		{
#ifndef NO_SECURITYFRAMEWORK
		if (sock->handshake == handshake_required) { LogMsg("mDNSPlatformReadTCP called while handshake required"); return 0; }
		else if (sock->handshake == handshake_in_progress) return 0;
		else if (sock->handshake != handshake_completed) LogMsg("mDNSPlatformReadTCP called with unexpected SSLHandshake status: %d", sock->handshake);

		//LogMsg("Starting SSLRead %d %X", sock->fd, fcntl(sock->fd, F_GETFL, 0));
		mStatus err = SSLRead(sock->tlsContext, buf, buflen, &nread);
		//LogMsg("SSLRead returned %d (%d) nread %d buflen %d", err, errSSLWouldBlock, nread, buflen);
		if (err == errSSLClosedGraceful) { nread = 0; *closed = mDNStrue; }
		else if (err && err != errSSLWouldBlock)
			{ LogMsg("ERROR: mDNSPlatformReadTCP - SSLRead: %d", err); nread = -1; *closed = mDNStrue; }
#else
		nread = -1;
		*closed = mDNStrue;
#endif /* NO_SECURITYFRAMEWORK */
		}
	else
		{
		static int CLOSEDcount = 0;
		static int EAGAINcount = 0;
		nread = recv(sock->fd, buf, buflen, 0);

		if (nread > 0) { CLOSEDcount = 0; EAGAINcount = 0; } // On success, clear our error counters
		else if (nread == 0)
			{
			*closed = mDNStrue;
			if ((++CLOSEDcount % 1000) == 0) { LogMsg("ERROR: mDNSPlatformReadTCP - recv %d got CLOSED %d times", sock->fd, CLOSEDcount); sleep(1); }
			}
		// else nread is negative -- see what kind of error we got
		else if (errno == ECONNRESET) { nread = 0; *closed = mDNStrue; }
		else if (errno != EAGAIN) { LogMsg("ERROR: mDNSPlatformReadTCP - recv: %d (%s)", errno, strerror(errno)); nread = -1; }
		else // errno is EAGAIN (EWOULDBLOCK) -- no data available
			{
			nread = 0;
			if ((++EAGAINcount % 1000) == 0) { LogMsg("ERROR: mDNSPlatformReadTCP - recv %d got EAGAIN %d times", sock->fd, EAGAINcount); sleep(1); }
			}
		}

	return nread;
	}

mDNSexport long mDNSPlatformWriteTCP(TCPSocket *sock, const char *msg, unsigned long len)
	{
	int nsent;

	if (sock->flags & kTCPSocketFlags_UseTLS)
		{
#ifndef NO_SECURITYFRAMEWORK
		size_t	processed;
		if (sock->handshake == handshake_required) { LogMsg("mDNSPlatformWriteTCP called while handshake required"); return 0; }
		if (sock->handshake == handshake_in_progress) return 0;
		else if (sock->handshake != handshake_completed) LogMsg("mDNSPlatformWriteTCP called with unexpected SSLHandshake status: %d", sock->handshake);
		
		mStatus	err = SSLWrite(sock->tlsContext, msg, len, &processed);

		if (!err) nsent = (int) processed;
		else if (err == errSSLWouldBlock) nsent = 0;
		else { LogMsg("ERROR: mDNSPlatformWriteTCP - SSLWrite returned %d", err); nsent = -1; }
#else
		nsent = -1;
#endif /* NO_SECURITYFRAMEWORK */
		}
	else
		{
		nsent = send(sock->fd, msg, len, 0);
		if (nsent < 0)
			{
			if (errno == EAGAIN) nsent = 0;
			else { LogMsg("ERROR: mDNSPlatformWriteTCP - send %s", strerror(errno)); nsent = -1; }
			}
		}

	return nsent;
	}

mDNSexport int mDNSPlatformTCPGetFD(TCPSocket *sock)
	{
	return sock->fd;
	}

// If mDNSIPPort port is non-zero, then it's a multicast socket on the specified interface
// If mDNSIPPort port is zero, then it's a randomly assigned port number, used for sending unicast queries
mDNSlocal mStatus SetupSocket(KQSocketSet *cp, const mDNSIPPort port, u_short sa_family, mDNSIPPort *const outport)
	{
#ifndef NO_IPV6
	int         *s        = (sa_family == AF_INET) ? &cp->sktv4 : &cp->sktv6;
	KQueueEntry	*k        = (sa_family == AF_INET) ? &cp->kqsv4 : &cp->kqsv6;
#else
	int         *s        = &cp->sktv4;
	KQueueEntry	*k        = &cp->kqsv4;
#endif
	const int on = 1;
	const int twofivefive = 255;
	mStatus err = mStatus_NoError;
	char *errstr = mDNSNULL;

#ifdef NO_IPV6
	if (sa_family != AF_INET) return -1;
#endif
	
	cp->closeFlag = mDNSNULL;

	int skt = socket(sa_family, SOCK_DGRAM, IPPROTO_UDP);
	if (skt < 3) { if (errno != EAFNOSUPPORT) LogMsg("SetupSocket: socket error %d errno %d (%s)", skt, errno, strerror(errno)); return(skt); }

	// ... with a shared UDP port, if it's for multicast receiving
	if (mDNSSameIPPort(port, MulticastDNSPort) || mDNSSameIPPort(port, NATPMPAnnouncementPort)) err = setsockopt(skt, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on));
	if (err < 0) { errstr = "setsockopt - SO_REUSEPORT"; goto fail; }

	if (sa_family == AF_INET)
		{
		// We want to receive destination addresses
		err = setsockopt(skt, IPPROTO_IP, IP_RECVDSTADDR, &on, sizeof(on));
		if (err < 0) { errstr = "setsockopt - IP_RECVDSTADDR"; goto fail; }

		// We want to receive interface identifiers
		err = setsockopt(skt, IPPROTO_IP, IP_RECVIF, &on, sizeof(on));
		if (err < 0) { errstr = "setsockopt - IP_RECVIF"; goto fail; }

		// We want to receive packet TTL value so we can check it
		err = setsockopt(skt, IPPROTO_IP, IP_RECVTTL, &on, sizeof(on));
		// We ignore errors here -- we already know Jaguar doesn't support this, but we can get by without it

		// Send unicast packets with TTL 255
		err = setsockopt(skt, IPPROTO_IP, IP_TTL, &twofivefive, sizeof(twofivefive));
		if (err < 0) { errstr = "setsockopt - IP_TTL"; goto fail; }

		// And multicast packets with TTL 255 too
		err = setsockopt(skt, IPPROTO_IP, IP_MULTICAST_TTL, &twofivefive, sizeof(twofivefive));
		if (err < 0) { errstr = "setsockopt - IP_MULTICAST_TTL"; goto fail; }

		// And start listening for packets
		struct sockaddr_in listening_sockaddr;
		listening_sockaddr.sin_family      = AF_INET;
		listening_sockaddr.sin_port        = port.NotAnInteger;		// Pass in opaque ID without any byte swapping
		listening_sockaddr.sin_addr.s_addr = mDNSSameIPPort(port, NATPMPAnnouncementPort) ? AllHosts_v4.NotAnInteger : 0;
		err = bind(skt, (struct sockaddr *) &listening_sockaddr, sizeof(listening_sockaddr));
		if (err) { errstr = "bind"; goto fail; }
		if (outport) outport->NotAnInteger = listening_sockaddr.sin_port;
		}
#ifndef NO_IPV6
	else if (sa_family == AF_INET6)
		{
		// NAT-PMP Announcements make no sense on IPv6, so bail early w/o error
		if (mDNSSameIPPort(port, NATPMPAnnouncementPort)) { if (outport) *outport = zeroIPPort; return mStatus_NoError; }
		
		// We want to receive destination addresses and receive interface identifiers
		err = setsockopt(skt, IPPROTO_IPV6, IPV6_RECVPKTINFO, &on, sizeof(on));
		if (err < 0) { errstr = "setsockopt - IPV6_RECVPKTINFO"; goto fail; }

		// We want to receive packet hop count value so we can check it
		err = setsockopt(skt, IPPROTO_IPV6, IPV6_RECVHOPLIMIT, &on, sizeof(on));
		if (err < 0) { errstr = "setsockopt - IPV6_RECVHOPLIMIT"; goto fail; }

		// We want to receive only IPv6 packets. Without this option we get IPv4 packets too,
		err = setsockopt(skt, IPPROTO_IPV6, IPV6_V6ONLY, &on, sizeof(on));
		if (err < 0) { errstr = "setsockopt - IPV6_V6ONLY"; goto fail; }

		// Send unicast packets with TTL 255
		err = setsockopt(skt, IPPROTO_IPV6, IPV6_UNICAST_HOPS, &twofivefive, sizeof(twofivefive));
		if (err < 0) { errstr = "setsockopt - IPV6_UNICAST_HOPS"; goto fail; }

		// And multicast packets with TTL 255 too
		err = setsockopt(skt, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, &twofivefive, sizeof(twofivefive));
		if (err < 0) { errstr = "setsockopt - IPV6_MULTICAST_HOPS"; goto fail; }

		// Want to receive our own packets
		err = setsockopt(skt, IPPROTO_IPV6, IPV6_MULTICAST_LOOP, &on, sizeof(on));
		if (err < 0) { errstr = "setsockopt - IPV6_MULTICAST_LOOP"; goto fail; }

		// And start listening for packets
		struct sockaddr_in6 listening_sockaddr6;
		mDNSPlatformMemZero(&listening_sockaddr6, sizeof(listening_sockaddr6));
		listening_sockaddr6.sin6_len         = sizeof(listening_sockaddr6);
		listening_sockaddr6.sin6_family      = AF_INET6;
		listening_sockaddr6.sin6_port        = port.NotAnInteger;		// Pass in opaque ID without any byte swapping
		listening_sockaddr6.sin6_flowinfo    = 0;
		listening_sockaddr6.sin6_addr        = in6addr_any; // Want to receive multicasts AND unicasts on this socket
		listening_sockaddr6.sin6_scope_id    = 0;
		err = bind(skt, (struct sockaddr *) &listening_sockaddr6, sizeof(listening_sockaddr6));
		if (err) { errstr = "bind"; goto fail; }
		if (outport) outport->NotAnInteger = listening_sockaddr6.sin6_port;
		}
#endif

	fcntl(skt, F_SETFL, fcntl(skt, F_GETFL, 0) | O_NONBLOCK); // set non-blocking
	fcntl(skt, F_SETFD, 1); // set close-on-exec
	*s = skt;
	k->KQcallback = myKQSocketCallBack;
	k->KQcontext  = cp;
	k->KQtask     = "UDP packet reception";
#ifdef MDNSRESPONDER_USES_LIB_DISPATCH_AS_PRIMARY_EVENT_LOOP_MECHANISM
	k->readSource = mDNSNULL;
	k->writeSource = mDNSNULL;
	k->fdClosed = mDNSfalse;
#endif
	KQueueSet(*s, EV_ADD, EVFILT_READ, k);

	return(err);

	fail:
	// For "bind" failures, only write log messages for our shared mDNS port, or for binding to zero
	if (strcmp(errstr, "bind") || mDNSSameIPPort(port, MulticastDNSPort) || mDNSIPPortIsZero(port))
		LogMsg("%s skt %d port %d error %d errno %d (%s)", errstr, skt, mDNSVal16(port), err, errno, strerror(errno));

	// If we got a "bind" failure of EADDRINUSE, inform the caller as it might need to try another random port
	if (!strcmp(errstr, "bind") && errno == EADDRINUSE)
		{
		err = EADDRINUSE;
		if (mDNSSameIPPort(port, MulticastDNSPort))
			NotifyOfElusiveBug("Setsockopt SO_REUSEPORT failed",
				"Congratulations, you've reproduced an elusive bug.\r"
				"Please contact the current assignee of <rdar://problem/3814904>.\r"
				"Alternatively, you can send email to radar-3387020@group.apple.com. (Note number is different.)\r"
				"If possible, please leave your machine undisturbed so that someone can come to investigate the problem.");
		}

	mDNSPlatformCloseFD(k, skt);
	return(err);
	}

mDNSexport UDPSocket *mDNSPlatformUDPSocket(mDNS *const m, const mDNSIPPort requestedport)
	{
	mStatus err;
	mDNSIPPort port = requestedport;
	mDNSBool randomizePort = mDNSIPPortIsZero(requestedport);
	int i = 10000; // Try at most 10000 times to get a unique random port
	UDPSocket *p = mallocL("UDPSocket", sizeof(UDPSocket));
	if (!p) { LogMsg("mDNSPlatformUDPSocket: memory exhausted"); return(mDNSNULL); }
	mDNSPlatformMemZero(p, sizeof(UDPSocket));
	p->ss.port  = zeroIPPort;
	p->ss.m     = m;
	p->ss.sktv4 = -1;
#ifndef NO_IPV6
	p->ss.sktv6 = -1;
#endif

	do
		{
		// The kernel doesn't do cryptographically strong random port allocation, so we do it ourselves here
		if (randomizePort) port = mDNSOpaque16fromIntVal(0xC000 + mDNSRandom(0x3FFF));
		err = SetupSocket(&p->ss, port, AF_INET, &p->ss.port);
#ifndef NO_IPV6
		if (!err)
			{
			err = SetupSocket(&p->ss, port, AF_INET6, &p->ss.port);
			if (err) { mDNSPlatformCloseFD(&p->ss.kqsv4, p->ss.sktv4); p->ss.sktv4 = -1; }
			}
#endif
		i--;
		} while (err == EADDRINUSE && randomizePort && i);

	if (err)
		{
		// In customer builds we don't want to log failures with port 5351, because this is a known issue
		// of failing to bind to this port when Internet Sharing has already bound to it
		// We also don't want to log about port 5350, due to a known bug when some other
		// process is bound to it.
		if (mDNSSameIPPort(requestedport, NATPMPPort) || mDNSSameIPPort(requestedport, NATPMPAnnouncementPort))
			LogInfo("mDNSPlatformUDPSocket: SetupSocket %d failed error %d errno %d (%s)", mDNSVal16(requestedport), err, errno, strerror(errno));
		else LogMsg("mDNSPlatformUDPSocket: SetupSocket %d failed error %d errno %d (%s)", mDNSVal16(requestedport), err, errno, strerror(errno));
		freeL("UDPSocket", p);
		return(mDNSNULL);
		}
	return(p);
	}

mDNSexport void mDNSPlatformUDPClose(UDPSocket *sock)
	{
	CloseSocketSet(&sock->ss);
	freeL("UDPSocket", sock);
	}

#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark -
#pragma mark - BPF Raw packet sending/receiving
#endif

#if APPLE_OSX_mDNSResponder

mDNSexport void mDNSPlatformSendRawPacket(const void *const msg, const mDNSu8 *const end, mDNSInterfaceID InterfaceID)
	{
	if (!InterfaceID) { LogMsg("mDNSPlatformSendRawPacket: No InterfaceID specified"); return; }
	NetworkInterfaceInfoOSX *info;

	extern mDNS mDNSStorage;
	info = IfindexToInterfaceInfoOSX(&mDNSStorage, InterfaceID);
	if (info == NULL)
		{
		LogMsg("mDNSPlatformSendUDP: Invalid interface index %p", InterfaceID);
		return;
		}
	if (info->BPF_fd < 0)
		LogMsg("mDNSPlatformSendRawPacket: %s BPF_fd %d not ready", info->ifinfo.ifname, info->BPF_fd);
	else
		{
		//LogMsg("mDNSPlatformSendRawPacket %d bytes on %s", end - (mDNSu8 *)msg, info->ifinfo.ifname);
		if (write(info->BPF_fd, msg, end - (mDNSu8 *)msg) < 0)
			LogMsg("mDNSPlatformSendRawPacket: BPF write(%d) failed %d (%s)", info->BPF_fd, errno, strerror(errno));
		}
	}

mDNSexport void mDNSPlatformSetLocalAddressCacheEntry(mDNS *const m, const mDNSAddr *const tpa, const mDNSEthAddr *const tha, mDNSInterfaceID InterfaceID)
	{
	if (!InterfaceID) { LogMsg("mDNSPlatformSetLocalAddressCacheEntry: No InterfaceID specified"); return; }
	NetworkInterfaceInfoOSX *info;
	info = IfindexToInterfaceInfoOSX(m, InterfaceID);
	if (info == NULL) { LogMsg("mDNSPlatformSetLocalAddressCacheEntry: Invalid interface index %p", InterfaceID); return; }
	// Manually inject an entry into our local ARP cache.
	// (We can't do this by sending an ARP broadcast, because the kernel only pays attention to incoming ARP packets, not outgoing.)
	if (!mDNS_AddressIsLocalSubnet(m, InterfaceID, tpa))
		LogSPS("Don't need address cache entry for %s %#a %.6a",            info->ifinfo.ifname, tpa, tha);
	else
		{
		int result = mDNSSetLocalAddressCacheEntry(info->scope_id, tpa->type, tpa->ip.v6.b, tha->b);
		if (result) LogMsg("Set local address cache entry for %s %#a %.6a failed: %d", info->ifinfo.ifname, tpa, tha, result);
		else        LogSPS("Set local address cache entry for %s %#a %.6a",            info->ifinfo.ifname, tpa, tha);
		}
	}

mDNSlocal void CloseBPF(NetworkInterfaceInfoOSX *const i)
	{
	LogSPS("%s closing BPF fd %d", i->ifinfo.ifname, i->BPF_fd);
#ifdef MDNSRESPONDER_USES_LIB_DISPATCH_AS_PRIMARY_EVENT_LOOP_MECHANISM
	// close will happen in the cancel handler
	dispatch_source_cancel(i->BPF_source);
#else

	// Note: MUST NOT close() the underlying native BSD sockets.
	// CFSocketInvalidate() will do that for us, in its own good time, which may not necessarily be immediately, because
	// it first has to unhook the sockets from its select() call on its other thread, before it can safely close them.
	CFRunLoopRemoveSource(i->m->p->CFRunLoop, i->BPF_rls, kCFRunLoopDefaultMode);
	CFRelease(i->BPF_rls);
	CFSocketInvalidate(i->BPF_cfs);
	CFRelease(i->BPF_cfs);
#endif
	i->BPF_fd = -1;
	if (i->BPF_mcfd >= 0) { close(i->BPF_mcfd); i->BPF_mcfd = -1; }
	}

mDNSlocal void bpf_callback_common(NetworkInterfaceInfoOSX *info)
	{
	KQueueLock(info->m);

	// Now we've got the lock, make sure the kqueue thread didn't close the fd out from under us (will not be a problem once the OS X
	// kernel has a mechanism for dispatching all events to a single thread, but for now we have to guard against this race condition).
	if (info->BPF_fd < 0) goto exit;

	ssize_t n = read(info->BPF_fd, &info->m->imsg, info->BPF_len);
	const mDNSu8 *ptr = (const mDNSu8 *)&info->m->imsg;
	const mDNSu8 *end = (const mDNSu8 *)&info->m->imsg + n;
	debugf("%3d: bpf_callback got %d bytes on %s", info->BPF_fd, n, info->ifinfo.ifname);

	if (n<0)
		{
		LogMsg("Closing %s BPF fd %d due to error %d (%s)", info->ifinfo.ifname, info->BPF_fd, errno, strerror(errno));
		CloseBPF(info);
		goto exit;
		}

	while (ptr < end)
		{
		const struct bpf_hdr *const bh = (const struct bpf_hdr *)ptr;
		debugf("%3d: bpf_callback ptr %p bh_hdrlen %d data %p bh_caplen %4d bh_datalen %4d next %p remaining %4d",
			info->BPF_fd, ptr, bh->bh_hdrlen, ptr + bh->bh_hdrlen, bh->bh_caplen, bh->bh_datalen,
			ptr + BPF_WORDALIGN(bh->bh_hdrlen + bh->bh_caplen), end - (ptr + BPF_WORDALIGN(bh->bh_hdrlen + bh->bh_caplen)));
		// Note that BPF guarantees that the NETWORK LAYER header will be word aligned, not the link-layer header.
		// Given that An Ethernet header is 14 bytes, this means that if the network layer header (e.g. IP header,
		// ARP message, etc.) is 4-byte aligned, then necessarily the Ethernet header will be NOT be 4-byte aligned.
		mDNSCoreReceiveRawPacket(info->m, ptr + bh->bh_hdrlen, ptr + bh->bh_hdrlen + bh->bh_caplen, info->ifinfo.InterfaceID);
		ptr += BPF_WORDALIGN(bh->bh_hdrlen + bh->bh_caplen);
		}
exit:
	KQueueUnlock(info->m, "bpf_callback");
	}
#ifdef MDNSRESPONDER_USES_LIB_DISPATCH_AS_PRIMARY_EVENT_LOOP_MECHANISM
mDNSlocal void bpf_callback_dispatch(NetworkInterfaceInfoOSX *const info)
	{
	bpf_callback_common(info);
	}
#else
mDNSlocal void bpf_callback(const CFSocketRef cfs, const CFSocketCallBackType CallBackType, const CFDataRef address, const void *const data, void *const context)
	{
	(void)cfs;
	(void)CallBackType;
	(void)address;
	(void)data;
	bpf_callback_common((NetworkInterfaceInfoOSX *)context);
	}
#endif

#define BPF_SetOffset(from, cond, to) (from)->cond = (to) - 1 - (from)

mDNSlocal int CountProxyTargets(mDNS *const m, NetworkInterfaceInfoOSX *x, int *p4, int *p6)
	{
	int numv4 = 0, numv6 = 0;
	AuthRecord *rr;

	for (rr = m->ResourceRecords; rr; rr=rr->next)
		if (rr->resrec.InterfaceID == x->ifinfo.InterfaceID && rr->AddressProxy.type == mDNSAddrType_IPv4)
			{
			if (p4) LogSPS("CountProxyTargets: fd %d %-7s IP%2d %.4a", x->BPF_fd, x->ifinfo.ifname, numv4, &rr->AddressProxy.ip.v4);
			numv4++;
			}

	for (rr = m->ResourceRecords; rr; rr=rr->next)
		if (rr->resrec.InterfaceID == x->ifinfo.InterfaceID && rr->AddressProxy.type == mDNSAddrType_IPv6)
			{
			if (p6) LogSPS("CountProxyTargets: fd %d %-7s IP%2d %.16a", x->BPF_fd, x->ifinfo.ifname, numv6, &rr->AddressProxy.ip.v6);
			numv6++;
			}

	if (p4) *p4 = numv4;
	if (p6) *p6 = numv6;
	return(numv4 + numv6);
	}

mDNSexport void mDNSPlatformUpdateProxyList(mDNS *const m, const mDNSInterfaceID InterfaceID)
	{
	NetworkInterfaceInfoOSX *x;

	// Note: We can't use IfIndexToInterfaceInfoOSX because that looks for Registered also.
	for (x = m->p->InterfaceList; x; x = x->next) if (x->ifinfo.InterfaceID == InterfaceID) break;

	if (!x) { LogMsg("mDNSPlatformUpdateProxyList: ERROR InterfaceID %p not found", InterfaceID); return; }

	#define MAX_BPF_ADDRS 250
	int numv4 = 0, numv6 = 0;

	if (CountProxyTargets(m, x, &numv4, &numv6) > MAX_BPF_ADDRS)
		{
		LogMsg("mDNSPlatformUpdateProxyList: ERROR Too many address proxy records v4 %d v6 %d", numv4, numv6);
		if (numv4 > MAX_BPF_ADDRS) numv4 = MAX_BPF_ADDRS;
		numv6 = MAX_BPF_ADDRS - numv4;
		}

	LogSPS("mDNSPlatformUpdateProxyList: fd %d %-7s MAC  %.6a %d v4 %d v6", x->BPF_fd, x->ifinfo.ifname, &x->ifinfo.MAC, numv4, numv6);

	// Caution: This is a static structure, so we need to be careful that any modifications we make to it
	// are done in such a way that they work correctly when mDNSPlatformUpdateProxyList is called multiple times
	static struct bpf_insn filter[17 + MAX_BPF_ADDRS] =
		{
		BPF_STMT(BPF_LD  + BPF_H   + BPF_ABS, 12),				// 0 Read Ethertype (bytes 12,13)

		BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, 0x0806, 0, 1),		// 1 If Ethertype == ARP goto next, else 3
		BPF_STMT(BPF_RET + BPF_K,             42),				// 2 Return 42-byte ARP

		BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, 0x0800, 4, 0),		// 3 If Ethertype == IPv4 goto 8 (IPv4 address list check) else next

		BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, 0x86DD, 0, 9),		// 4 If Ethertype == IPv6 goto next, else exit
		BPF_STMT(BPF_LD  + BPF_H   + BPF_ABS, 20),				// 5 Read Protocol and Hop Limit (bytes 20,21)
		BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, 0x3AFF, 0, 9),		// 6 If (Prot,TTL) == (3A,FF) goto next, else IPv6 address list check
		BPF_STMT(BPF_RET + BPF_K,             86),				// 7 Return 86-byte ND

		// Is IPv4 packet; check if it's addressed to any IPv4 address we're proxying for
		BPF_STMT(BPF_LD  + BPF_W   + BPF_ABS, 30),				// 8 Read IPv4 Dst (bytes 30,31,32,33)
		};

	struct bpf_insn *pc   = &filter[9];
	struct bpf_insn *chk6 = pc   + numv4 + 1;	// numv4 address checks, plus a "return 0"
	struct bpf_insn *fail = chk6 + 1 + numv6;	// Get v6 Dst LSW, plus numv6 address checks
	struct bpf_insn *ret4 = fail + 1;
	struct bpf_insn *ret6 = ret4 + 4;

	static const struct bpf_insn rf  = BPF_STMT(BPF_RET + BPF_K, 0);				// No match: Return nothing

	static const struct bpf_insn g6  = BPF_STMT(BPF_LD  + BPF_W   + BPF_ABS, 50);	// Read IPv6 Dst LSW (bytes 50,51,52,53)

	static const struct bpf_insn r4a = BPF_STMT(BPF_LDX + BPF_B   + BPF_MSH, 14);	// Get IP Header length (normally 20)
	static const struct bpf_insn r4b = BPF_STMT(BPF_LD  + BPF_IMM,           54);	// A = 54 (14-byte Ethernet plus 20-byte TCP + 20 bytes spare)
	static const struct bpf_insn r4c = BPF_STMT(BPF_ALU + BPF_ADD + BPF_X,    0);	// A += IP Header length
	static const struct bpf_insn r4d = BPF_STMT(BPF_RET + BPF_A, 0);				// Success: Return Ethernet + IP + TCP + 20 bytes spare (normally 74)

	static const struct bpf_insn r6a = BPF_STMT(BPF_RET + BPF_K, 94);				// Success: Return Eth + IPv6 + TCP + 20 bytes spare

	BPF_SetOffset(&filter[4], jf, fail);	// If Ethertype not ARP, IPv4, or IPv6, fail
	BPF_SetOffset(&filter[6], jf, chk6);	// If IPv6 but not ICMPv6, go to IPv6 address list check

	// BPF Byte-Order Note
	// The BPF API designers apparently thought that programmers would not be smart enough to use htons
	// and htonl correctly to convert numeric values to network byte order on little-endian machines,
	// so instead they chose to make the API implicitly byte-swap *ALL* values, even literal byte strings
	// that shouldn't be byte-swapped, like ASCII text, Ethernet addresses, IP addresses, etc.
	// As a result, if we put Ethernet addresses and IP addresses in the right byte order, the BPF API
	// will byte-swap and make them backwards, and then our filter won't work. So, we have to arrange
	// that on little-endian machines we deliberately put addresses in memory with the bytes backwards,
	// so that when the BPF API goes through and swaps them all, they end up back as they should be.
	// In summary, if we byte-swap all the non-numeric fields that shouldn't be swapped, and we *don't*
	// swap any of the numeric values that *should* be byte-swapped, then the filter will work correctly.

	// IPSEC capture size notes:
	//  8 bytes UDP header
	//  4 bytes Non-ESP Marker
	// 28 bytes IKE Header
	// --
	// 40 Total. Capturing TCP Header + 20 gets us enough bytes to receive the IKE Header in a UDP-encapsulated IKE packet.

	AuthRecord *rr;
	for (rr = m->ResourceRecords; rr; rr=rr->next)
		if (rr->resrec.InterfaceID == InterfaceID && rr->AddressProxy.type == mDNSAddrType_IPv4)
			{
			mDNSv4Addr a = rr->AddressProxy.ip.v4;
			pc->code = BPF_JMP + BPF_JEQ + BPF_K;
			BPF_SetOffset(pc, jt, ret4);
			pc->jf   = 0;
			pc->k    = (bpf_u_int32)a.b[0] << 24 | (bpf_u_int32)a.b[1] << 16 | (bpf_u_int32)a.b[2] << 8 | (bpf_u_int32)a.b[3];
			pc++;
			}
	*pc++ = rf;

	if (pc != chk6) LogMsg("mDNSPlatformUpdateProxyList: pc %p != chk6 %p", pc, chk6);
	*pc++ = g6;	// chk6 points here

	// First cancel any previous ND group memberships we had, then create a fresh socket
	if (x->BPF_mcfd >= 0) close(x->BPF_mcfd);
	x->BPF_mcfd = socket(AF_INET6, SOCK_DGRAM, 0);

	for (rr = m->ResourceRecords; rr; rr=rr->next)
		if (rr->resrec.InterfaceID == InterfaceID && rr->AddressProxy.type == mDNSAddrType_IPv6)
			{
			const mDNSv6Addr *const a = &rr->AddressProxy.ip.v6;
			pc->code = BPF_JMP + BPF_JEQ + BPF_K;
			BPF_SetOffset(pc, jt, ret6);
			pc->jf   = 0;
			pc->k    = (bpf_u_int32)a->b[0x0C] << 24 | (bpf_u_int32)a->b[0x0D] << 16 | (bpf_u_int32)a->b[0x0E] << 8 | (bpf_u_int32)a->b[0x0F];
			pc++;

			struct ipv6_mreq i6mr;
			i6mr.ipv6mr_interface = x->scope_id;
			i6mr.ipv6mr_multiaddr = *(const struct in6_addr*)&NDP_prefix;
			i6mr.ipv6mr_multiaddr.s6_addr[0xD] = a->b[0xD];
			i6mr.ipv6mr_multiaddr.s6_addr[0xE] = a->b[0xE];
			i6mr.ipv6mr_multiaddr.s6_addr[0xF] = a->b[0xF];

			// Do precautionary IPV6_LEAVE_GROUP first, necessary to clear stale kernel state
			mStatus err = setsockopt(x->BPF_mcfd, IPPROTO_IPV6, IPV6_LEAVE_GROUP, &i6mr, sizeof(i6mr));
			if (err < 0 && (errno != EADDRNOTAVAIL))
				LogMsg("mDNSPlatformUpdateProxyList: IPV6_LEAVE_GROUP error %d errno %d (%s) group %.16a on %u", err, errno, strerror(errno), &i6mr.ipv6mr_multiaddr, i6mr.ipv6mr_interface);

			err = setsockopt(x->BPF_mcfd, IPPROTO_IPV6, IPV6_JOIN_GROUP, &i6mr, sizeof(i6mr));
			if (err < 0 && (errno != EADDRINUSE))	// Joining same group twice can give "Address already in use" error -- no need to report that
				LogMsg("mDNSPlatformUpdateProxyList: IPV6_JOIN_GROUP error %d errno %d (%s) group %.16a on %u", err, errno, strerror(errno), &i6mr.ipv6mr_multiaddr, i6mr.ipv6mr_interface);
			
			LogSPS("Joined IPv6 ND multicast group %.16a for %.16a", &i6mr.ipv6mr_multiaddr, a);
			}

	if (pc != fail) LogMsg("mDNSPlatformUpdateProxyList: pc %p != fail %p", pc, fail);
	*pc++ = rf;		// fail points here

	if (pc != ret4) LogMsg("mDNSPlatformUpdateProxyList: pc %p != ret4 %p", pc, ret4);
	*pc++ = r4a;	// ret4 points here
	*pc++ = r4b;
	*pc++ = r4c;
	*pc++ = r4d;

	if (pc != ret6) LogMsg("mDNSPlatformUpdateProxyList: pc %p != ret6 %p", pc, ret6);
	*pc++ = r6a;	// ret6 points here

	struct bpf_program prog = { pc - filter, filter };


	if (!numv4 && !numv6)
		{
		LogSPS("mDNSPlatformUpdateProxyList: No need for filter");
		if (m->timenow == 0) LogMsg("mDNSPlatformUpdateProxyList: m->timenow == 0");
		// Schedule check to see if we can close this BPF_fd now
		if (!m->p->NetworkChanged) m->p->NetworkChanged = NonZeroTime(m->timenow + mDNSPlatformOneSecond * 2);
		// prog.bf_len = 0; This seems to panic the kernel
		if (x->BPF_fd < 0) return;		// If we've already closed our BPF_fd, no need to generate an error message below
		}

	if (ioctl(x->BPF_fd, BIOCSETF, &prog) < 0) LogMsg("mDNSPlatformUpdateProxyList: BIOCSETF(%d) failed %d (%s)", prog.bf_len, errno, strerror(errno));
	else LogSPS("mDNSPlatformUpdateProxyList: BIOCSETF(%d) successful", prog.bf_len);
	}

mDNSexport void mDNSPlatformReceiveBPF_fd(mDNS *const m, int fd)
	{
	mDNS_Lock(m);
	
	NetworkInterfaceInfoOSX *i;
	for (i = m->p->InterfaceList; i; i = i->next) if (i->BPF_fd == -2) break;
	if (!i) { LogSPS("mDNSPlatformReceiveBPF_fd: No Interfaces awaiting BPF fd %d; closing", fd); close(fd); }
	else
		{
		LogSPS("%s using   BPF fd %d", i->ifinfo.ifname, fd);
	
		struct bpf_version v;
		if (ioctl(fd, BIOCVERSION, &v) < 0)
			LogMsg("mDNSPlatformReceiveBPF_fd: %d %s BIOCVERSION failed %d (%s)", fd, i->ifinfo.ifname, errno, strerror(errno));
		else if (BPF_MAJOR_VERSION != v.bv_major || BPF_MINOR_VERSION != v.bv_minor)
			LogMsg("mDNSPlatformReceiveBPF_fd: %d %s BIOCVERSION header %d.%d kernel %d.%d",
				fd, i->ifinfo.ifname, BPF_MAJOR_VERSION, BPF_MINOR_VERSION, v.bv_major, v.bv_minor);
	
		if (ioctl(fd, BIOCGBLEN, &i->BPF_len) < 0)
			LogMsg("mDNSPlatformReceiveBPF_fd: %d %s BIOCGBLEN failed %d (%s)", fd, i->ifinfo.ifname, errno, strerror(errno));
	
		if (i->BPF_len > sizeof(m->imsg))
			{
			i->BPF_len = sizeof(m->imsg);
			if (ioctl(fd, BIOCSBLEN, &i->BPF_len) < 0)
				LogMsg("mDNSPlatformReceiveBPF_fd: %d %s BIOCSBLEN failed %d (%s)", fd, i->ifinfo.ifname, errno, strerror(errno));
			else 
				LogSPS("mDNSPlatformReceiveBPF_fd: %d %s BIOCSBLEN %d", fd, i->ifinfo.ifname, i->BPF_len);
			}
	
		static const u_int opt_one = 1;
		if (ioctl(fd, BIOCIMMEDIATE, &opt_one) < 0)
			LogMsg("mDNSPlatformReceiveBPF_fd: %d %s BIOCIMMEDIATE failed %d (%s)", fd, i->ifinfo.ifname, errno, strerror(errno));
	
		//if (ioctl(fd, BIOCPROMISC, &opt_one) < 0)
		//	LogMsg("mDNSPlatformReceiveBPF_fd: %d %s BIOCPROMISC failed %d (%s)", fd, i->ifinfo.ifname, errno, strerror(errno));
	
		//if (ioctl(fd, BIOCSHDRCMPLT, &opt_one) < 0)
		//	LogMsg("mDNSPlatformReceiveBPF_fd: %d %s BIOCSHDRCMPLT failed %d (%s)", fd, i->ifinfo.ifname, errno, strerror(errno));
	
		struct ifreq ifr;
		mDNSPlatformMemZero(&ifr, sizeof(ifr));
		strlcpy(ifr.ifr_name, i->ifinfo.ifname, sizeof(ifr.ifr_name));
		if (ioctl(fd, BIOCSETIF, &ifr) < 0)
			{ LogMsg("mDNSPlatformReceiveBPF_fd: %d %s BIOCSETIF failed %d (%s)", fd, i->ifinfo.ifname, errno, strerror(errno)); i->BPF_fd = -3; }
		else
			{
#ifdef MDNSRESPONDER_USES_LIB_DISPATCH_AS_PRIMARY_EVENT_LOOP_MECHANISM
			i->BPF_fd  = fd;
			i->BPF_source = dispatch_source_create(DISPATCH_SOURCE_TYPE_READ, fd, 0, dispatch_get_main_queue());
			if (!i->BPF_source) {LogMsg("mDNSPlatformReceiveBPF_fd: dispatch source create failed");return;}
			dispatch_source_set_event_handler(i->BPF_source, ^{bpf_callback_dispatch(i);});
			dispatch_source_set_cancel_handler(i->BPF_source, ^{close(fd);});
			dispatch_resume(i->BPF_source);
#else
			CFSocketContext myCFSocketContext = { 0, i, NULL, NULL, NULL };
			i->BPF_fd  = fd;
			i->BPF_cfs = CFSocketCreateWithNative(kCFAllocatorDefault, fd, kCFSocketReadCallBack, bpf_callback, &myCFSocketContext);
			i->BPF_rls = CFSocketCreateRunLoopSource(kCFAllocatorDefault, i->BPF_cfs, 0);
			CFRunLoopAddSource(i->m->p->CFRunLoop, i->BPF_rls, kCFRunLoopDefaultMode);
#endif
			mDNSPlatformUpdateProxyList(m, i->ifinfo.InterfaceID);
			}
		}

	mDNS_Unlock(m);
	}

#endif // APPLE_OSX_mDNSResponder

#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark -
#pragma mark - Key Management
#endif

#ifndef NO_SECURITYFRAMEWORK
mDNSlocal CFArrayRef GetCertChain(SecIdentityRef identity)
	{
	CFMutableArrayRef certChain = NULL;
	if (!identity) { LogMsg("getCertChain: identity is NULL"); return(NULL); }
	SecCertificateRef cert;
	OSStatus err = SecIdentityCopyCertificate(identity, &cert);
	if (err || !cert) LogMsg("getCertChain: SecIdentityCopyCertificate() returned %d", (int) err);
	else
		{
		SecPolicySearchRef searchRef;
		err = SecPolicySearchCreate(CSSM_CERT_X_509v3, &CSSMOID_APPLE_X509_BASIC, NULL, &searchRef);
		if (err || !searchRef) LogMsg("getCertChain: SecPolicySearchCreate() returned %d", (int) err);
		else
			{
			SecPolicyRef policy;
			err = SecPolicySearchCopyNext(searchRef, &policy);
			if (err || !policy) LogMsg("getCertChain: SecPolicySearchCopyNext() returned %d", (int) err);
			else
				{
				CFArrayRef wrappedCert = CFArrayCreate(NULL, (const void**) &cert, 1, &kCFTypeArrayCallBacks);
				if (!wrappedCert) LogMsg("getCertChain: wrappedCert is NULL");
				else
					{
					SecTrustRef trust;
					err = SecTrustCreateWithCertificates(wrappedCert, policy, &trust);
					if (err || !trust) LogMsg("getCertChain: SecTrustCreateWithCertificates() returned %d", (int) err);
					else
						{
						err = SecTrustEvaluate(trust, NULL);
						if (err) LogMsg("getCertChain: SecTrustEvaluate() returned %d", (int) err);
						else
							{
							CFArrayRef rawCertChain;
							CSSM_TP_APPLE_EVIDENCE_INFO *statusChain = NULL;
							err = SecTrustGetResult(trust, NULL, &rawCertChain, &statusChain);
							if (err || !rawCertChain || !statusChain) LogMsg("getCertChain: SecTrustGetResult() returned %d", (int) err);
							else
								{
								certChain = CFArrayCreateMutableCopy(NULL, 0, rawCertChain);
								if (!certChain) LogMsg("getCertChain: certChain is NULL");
								else
									{
									// Replace the SecCertificateRef at certChain[0] with a SecIdentityRef per documentation for SSLSetCertificate:
									// <http://devworld.apple.com/documentation/Security/Reference/secureTransportRef/index.html>
									CFArraySetValueAtIndex(certChain, 0, identity);
									// Remove root from cert chain, but keep any and all intermediate certificates that have been signed by the root certificate
									if (CFArrayGetCount(certChain) > 1) CFArrayRemoveValueAtIndex(certChain, CFArrayGetCount(certChain) - 1);
									}
								CFRelease(rawCertChain);
								// Do not free statusChain:
								// <http://developer.apple.com/documentation/Security/Reference/certifkeytrustservices/Reference/reference.html> says:
								// certChain: Call the CFRelease function to release this object when you are finished with it.
								// statusChain: Do not attempt to free this pointer; it remains valid until the trust management object is released...
								}
							}
						CFRelease(trust);
						}
					CFRelease(wrappedCert);
					}
				CFRelease(policy);
				}
			CFRelease(searchRef);
			}
		CFRelease(cert);
		}
	return certChain;
	}
#endif /* NO_SECURITYFRAMEWORK */

mDNSexport mStatus mDNSPlatformTLSSetupCerts(void)
	{
#ifdef NO_SECURITYFRAMEWORK
	return mStatus_UnsupportedErr;
#else
	SecIdentityRef			identity = nil;
	SecIdentitySearchRef	srchRef = nil;
	OSStatus				err;

	// search for "any" identity matching specified key use
	// In this app, we expect there to be exactly one
	err = SecIdentitySearchCreate(NULL, CSSM_KEYUSE_DECRYPT, &srchRef);
	if (err) { LogMsg("ERROR: mDNSPlatformTLSSetupCerts: SecIdentitySearchCreate returned %d", (int) err); return err; }

	err = SecIdentitySearchCopyNext(srchRef, &identity);
	if (err) { LogMsg("ERROR: mDNSPlatformTLSSetupCerts: SecIdentitySearchCopyNext returned %d", (int) err); return err; }

	if (CFGetTypeID(identity) != SecIdentityGetTypeID())
		{ LogMsg("ERROR: mDNSPlatformTLSSetupCerts: SecIdentitySearchCopyNext CFTypeID failure"); return mStatus_UnknownErr; }

	// Found one. Call getCertChain to create the correct certificate chain.
	ServerCerts = GetCertChain(identity);
	if (ServerCerts == nil) { LogMsg("ERROR: mDNSPlatformTLSSetupCerts: getCertChain error"); return mStatus_UnknownErr; }

	return mStatus_NoError;
#endif /* NO_SECURITYFRAMEWORK */
	}

mDNSexport  void  mDNSPlatformTLSTearDownCerts(void)
	{
#ifndef NO_SECURITYFRAMEWORK
	if (ServerCerts) { CFRelease(ServerCerts); ServerCerts = NULL; }
#endif /* NO_SECURITYFRAMEWORK */
	}

// This gets the text of the field currently labelled "Computer Name" in the Sharing Prefs Control Panel
mDNSlocal void GetUserSpecifiedFriendlyComputerName(domainlabel *const namelabel)
	{
	CFStringEncoding encoding = kCFStringEncodingUTF8;
	CFStringRef cfs = SCDynamicStoreCopyComputerName(NULL, &encoding);
	if (cfs)
		{
		CFStringGetPascalString(cfs, namelabel->c, sizeof(*namelabel), kCFStringEncodingUTF8);
		CFRelease(cfs);
		}
	}

// This gets the text of the field currently labelled "Local Hostname" in the Sharing Prefs Control Panel
mDNSlocal void GetUserSpecifiedLocalHostName(domainlabel *const namelabel)
	{
	CFStringRef cfs = SCDynamicStoreCopyLocalHostName(NULL);
	if (cfs)
		{
		CFStringGetPascalString(cfs, namelabel->c, sizeof(*namelabel), kCFStringEncodingUTF8);
		CFRelease(cfs);
		}
	}

mDNSexport mDNSBool DictionaryIsEnabled(CFDictionaryRef dict)
	{
	mDNSs32 val;
	CFNumberRef state = (CFNumberRef)CFDictionaryGetValue(dict, CFSTR("Enabled"));
	if (!state) return mDNSfalse;
	if (!CFNumberGetValue(state, kCFNumberSInt32Type, &val))
		{ LogMsg("ERROR: DictionaryIsEnabled - CFNumberGetValue"); return mDNSfalse; }
	return val ? mDNStrue : mDNSfalse;
	}

mDNSlocal mStatus SetupAddr(mDNSAddr *ip, const struct sockaddr *const sa)
	{
	if (!sa) { LogMsg("SetupAddr ERROR: NULL sockaddr"); return(mStatus_Invalid); }

	if (sa->sa_family == AF_INET)
		{
		struct sockaddr_in *ifa_addr = (struct sockaddr_in *)sa;
		ip->type = mDNSAddrType_IPv4;
		ip->ip.v4.NotAnInteger = ifa_addr->sin_addr.s_addr;
		return(mStatus_NoError);
		}

	if (sa->sa_family == AF_INET6)
		{
		struct sockaddr_in6 *ifa_addr = (struct sockaddr_in6 *)sa;
		// Inside the BSD kernel they use a hack where they stuff the sin6->sin6_scope_id
		// value into the second word of the IPv6 link-local address, so they can just
		// pass around IPv6 address structures instead of full sockaddr_in6 structures.
		// Those hacked IPv6 addresses aren't supposed to escape the kernel in that form, but they do.
		if (IN6_IS_ADDR_LINKLOCAL(&ifa_addr->sin6_addr)) ifa_addr->sin6_addr.__u6_addr.__u6_addr16[1] = 0;
		ip->type = mDNSAddrType_IPv6;
		ip->ip.v6 = *(mDNSv6Addr*)&ifa_addr->sin6_addr;
		return(mStatus_NoError);
		}

	LogMsg("SetupAddr invalid sa_family %d", sa->sa_family);
	return(mStatus_Invalid);
	}

mDNSlocal mDNSEthAddr GetBSSID(char *ifa_name)
	{
	mDNSEthAddr eth = zeroEthAddr;
	SCDynamicStoreRef store = SCDynamicStoreCreate(NULL, CFSTR("mDNSResponder:GetBSSID"), NULL, NULL);
	if (!store)
		LogMsg("GetBSSID: SCDynamicStoreCreate failed: %s", SCErrorString(SCError()));
	else
		{
		CFStringRef entityname = CFStringCreateWithFormat(NULL, NULL, CFSTR("State:/Network/Interface/%s/AirPort"), ifa_name);
		if (entityname)
			{
			CFDictionaryRef dict = SCDynamicStoreCopyValue(store, entityname);
			if (dict)
				{
				CFRange range = { 0, 6 };		// Offset, length
				CFDataRef data = CFDictionaryGetValue(dict, CFSTR("BSSID"));
				if (data && CFDataGetLength(data) == 6) CFDataGetBytes(data, range, eth.b);
				CFRelease(dict);
				}
			CFRelease(entityname);
			}
		CFRelease(store);
		}
	return(eth);
	}

mDNSlocal int GetMAC(mDNSEthAddr *eth, u_short ifindex)
	{
	struct ifaddrs *ifa;
	for (ifa = myGetIfAddrs(0); ifa; ifa = ifa->ifa_next)
		if (ifa->ifa_addr->sa_family == AF_LINK)
			{
			const struct sockaddr_dl *const sdl = (const struct sockaddr_dl *)ifa->ifa_addr;
			if (sdl->sdl_index == ifindex)
				{ mDNSPlatformMemCopy(eth->b, sdl->sdl_data + sdl->sdl_nlen, 6); return 0; }
			}
	*eth = zeroEthAddr;
	return -1;
	}

#ifndef SIOCGIFWAKEFLAGS
#define SIOCGIFWAKEFLAGS _IOWR('i', 136, struct ifreq) /* get interface wake property flags */
#endif

#ifndef IF_WAKE_ON_MAGIC_PACKET
#define IF_WAKE_ON_MAGIC_PACKET 0x01
#endif

#ifndef ifr_wake_flags
#define ifr_wake_flags ifr_ifru.ifru_intval
#endif

mDNSlocal mDNSBool NetWakeInterface(NetworkInterfaceInfoOSX *i)
	{
	if (!MulticastInterface(i)     ) return(mDNSfalse);	// We only use Sleep Proxy Service on multicast-capable interfaces
	if (i->ifa_flags & IFF_LOOPBACK) return(mDNSfalse);	// except loopback

	int s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) { LogMsg("NetWakeInterface socket failed %s error %d errno %d (%s)", i->ifinfo.ifname, s, errno, strerror(errno)); return(mDNSfalse); }

	struct ifreq ifr;
	strlcpy(ifr.ifr_name, i->ifinfo.ifname, sizeof(ifr.ifr_name));
	if (ioctl(s, SIOCGIFWAKEFLAGS, &ifr) < 0)
		{
		// For some strange reason, in /usr/include/sys/errno.h, EOPNOTSUPP is defined to be
		// 102 when compiling kernel code, and 45 when compiling user-level code. Since this
		// error code is being returned from the kernel, we need to use the kernel version.
		#define KERNEL_EOPNOTSUPP 102
		if (errno != KERNEL_EOPNOTSUPP)	// "Operation not supported on socket", the expected result on Leopard and earlier
			LogMsg("NetWakeInterface SIOCGIFWAKEFLAGS %s errno %d (%s)", i->ifinfo.ifname, errno, strerror(errno));
		// If on Leopard or earlier, we get EOPNOTSUPP, so in that case
		// we enable WOL if this interface is not AirPort and "Wake for Network access" is turned on.
		ifr.ifr_wake_flags = (errno == KERNEL_EOPNOTSUPP && !(i)->BSSID.l[0] && i->m->SystemWakeOnLANEnabled) ? IF_WAKE_ON_MAGIC_PACKET : 0;
		}

	close(s);

	// ifr.ifr_wake_flags = IF_WAKE_ON_MAGIC_PACKET;	// For testing with MacBook Air, using a USB dongle that doesn't actually support Wake-On-LAN

	LogSPS("%-6s %#-14a %s WOMP", i->ifinfo.ifname, &i->ifinfo.ip, (ifr.ifr_wake_flags & IF_WAKE_ON_MAGIC_PACKET) ? "supports" : "no");

	return((ifr.ifr_wake_flags & IF_WAKE_ON_MAGIC_PACKET) != 0);
	}

// Returns pointer to newly created NetworkInterfaceInfoOSX object, or
// pointer to already-existing NetworkInterfaceInfoOSX object found in list, or
// may return NULL if out of memory (unlikely) or parameters are invalid for some reason
// (e.g. sa_family not AF_INET or AF_INET6)
mDNSlocal NetworkInterfaceInfoOSX *AddInterfaceToList(mDNS *const m, struct ifaddrs *ifa, mDNSs32 utc)
	{
	mDNSu32 scope_id  = if_nametoindex(ifa->ifa_name);
	mDNSEthAddr bssid = GetBSSID(ifa->ifa_name);

	mDNSAddr ip, mask;
	if (SetupAddr(&ip,   ifa->ifa_addr   ) != mStatus_NoError) return(NULL);
	if (SetupAddr(&mask, ifa->ifa_netmask) != mStatus_NoError) return(NULL);

	NetworkInterfaceInfoOSX **p;
	for (p = &m->p->InterfaceList; *p; p = &(*p)->next)
		if (scope_id == (*p)->scope_id &&
			mDNSSameAddress(&ip, &(*p)->ifinfo.ip) &&
			mDNSSameEthAddress(&bssid, &(*p)->BSSID))
			{
			debugf("AddInterfaceToList: Found existing interface %lu %.6a with address %#a at %p, ifname before %s, after %s", scope_id, &bssid, &ip, *p, (*p)->ifinfo.ifname, ifa->ifa_name);
			// The name should be updated to the new name so that we don't report a wrong name in our SIGINFO output.
			// When interfaces are created with same MAC address, kernel resurrects the old interface.
			// Even though the interface index is the same (which should be sufficient), when we receive a UDP packet
			// we get the corresponding name for the interface index on which the packet was received and check against
			// the InterfaceList for a matching name. So, keep the name in sync
			strlcpy((*p)->ifinfo.ifname, ifa->ifa_name, sizeof((*p)->ifinfo.ifname));
			(*p)->Exists = mDNStrue;
			// If interface was not in getifaddrs list last time we looked, but it is now, update 'AppearanceTime' for this record
			if ((*p)->LastSeen != utc) (*p)->AppearanceTime = utc;

			// If Wake-on-LAN capability of this interface has changed (e.g. because power cable on laptop has been disconnected)
			// we may need to start or stop or sleep proxy browse operation
			const mDNSBool NetWake = NetWakeInterface(*p);
			if ((*p)->ifinfo.NetWake != NetWake)
				{
				(*p)->ifinfo.NetWake = NetWake;
				// If this interface is already registered with mDNSCore, then we need to start or stop its NetWake browse on-the-fly.
				// If this interface is not already registered (i.e. it's a dormant interface we had in our list
				// from when we previously saw it) then we mustn't do that, because mDNSCore doesn't know about it yet.
				// In this case, the mDNS_RegisterInterface() call will take care of starting the NetWake browse if necessary.
				if ((*p)->Registered)
					{
					mDNS_Lock(m);
					if (NetWake) mDNS_ActivateNetWake_internal  (m, &(*p)->ifinfo);
					else         mDNS_DeactivateNetWake_internal(m, &(*p)->ifinfo);
					mDNS_Unlock(m);
					}
				}

			return(*p);
			}

	NetworkInterfaceInfoOSX *i = (NetworkInterfaceInfoOSX *)mallocL("NetworkInterfaceInfoOSX", sizeof(*i));
	debugf("AddInterfaceToList: Making   new   interface %lu %.6a with address %#a at %p", scope_id, &bssid, &ip, i);
	if (!i) return(mDNSNULL);
	mDNSPlatformMemZero(i, sizeof(NetworkInterfaceInfoOSX));
	i->ifinfo.InterfaceID = (mDNSInterfaceID)(uintptr_t)scope_id;
	i->ifinfo.ip          = ip;
	i->ifinfo.mask        = mask;
	strlcpy(i->ifinfo.ifname, ifa->ifa_name, sizeof(i->ifinfo.ifname));
	i->ifinfo.ifname[sizeof(i->ifinfo.ifname)-1] = 0;
	// We can be configured to disable multicast advertisement, but we want to to support
	// local-only services, which need a loopback address record.
	i->ifinfo.Advertise   = m->DivertMulticastAdvertisements ? ((ifa->ifa_flags & IFF_LOOPBACK) ? mDNStrue : mDNSfalse) : m->AdvertiseLocalAddresses;
	i->ifinfo.McastTxRx   = mDNSfalse; // For now; will be set up later at the end of UpdateInterfaceList
	i->ifinfo.Loopback    = ((ifa->ifa_flags & IFF_LOOPBACK) != 0) ? mDNStrue : mDNSfalse;

	i->next            = mDNSNULL;
	i->m               = m;
	i->Exists          = mDNStrue;
	i->Flashing        = mDNSfalse;
	i->Occulting       = mDNSfalse;
	i->AppearanceTime  = utc;		// Brand new interface; AppearanceTime is now
	i->LastSeen        = utc;
	i->ifa_flags       = ifa->ifa_flags;
	i->scope_id        = scope_id;
	i->BSSID           = bssid;
	i->sa_family       = ifa->ifa_addr->sa_family;
	i->BPF_fd          = -1;
	i->BPF_mcfd        = -1;
	i->BPF_len         = 0;
	i->Registered	   = mDNSNULL;

	// Do this AFTER i->BSSID has been set up
	i->ifinfo.NetWake  = NetWakeInterface(i);
	GetMAC(&i->ifinfo.MAC, scope_id);
	if (i->ifinfo.NetWake && !i->ifinfo.MAC.l[0])
		LogMsg("AddInterfaceToList: Bad MAC address %.6a for %d %s %#a", &i->ifinfo.MAC, scope_id, i->ifinfo.ifname, &ip);

	*p = i;
	return(i);
	}

#if USE_V6_ONLY_WHEN_NO_ROUTABLE_V4
mDNSlocal NetworkInterfaceInfoOSX *FindRoutableIPv4(mDNS *const m, mDNSu32 scope_id)
	{
	NetworkInterfaceInfoOSX *i;
	for (i = m->p->InterfaceList; i; i = i->next)
		if (i->Exists && i->scope_id == scope_id && i->ifinfo.ip.type == mDNSAddrType_IPv4)
			if (!mDNSv4AddressIsLinkLocal(&i->ifinfo.ip.ip.v4))
				return(i);
	return(mDNSNULL);
	}
#endif

#if APPLE_OSX_mDNSResponder

#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark -
#pragma mark - AutoTunnel
#endif

#define kRacoonPort 4500

static DomainAuthInfo* AnonymousRacoonConfig = mDNSNULL;

#ifndef NO_SECURITYFRAMEWORK

static CFMutableDictionaryRef domainStatusDict = NULL;

// MUST be called with lock held
mDNSlocal void RemoveAutoTunnelDomainStatus(const mDNS *const m, const DomainAuthInfo *const info)
	{
	char buffer[1024];
	mDNSu32 buflen;
	CFStringRef domain;

	LogInfo("RemoveAutoTunnelDomainStatus: %##s", info->domain.c);

	if (!domainStatusDict) { LogMsg("RemoveAutoTunnelDomainStatus: No domainStatusDict"); return; }
	
	buflen = mDNS_snprintf(buffer, sizeof(buffer), "%##s", info->domain.c);
	if (info->AutoTunnel == dnsprefix) buffer[buflen-1] = 0; // Strip the trailing dot for Classic
	domain = CFStringCreateWithCString(NULL, buffer, kCFStringEncodingUTF8);
	if (!domain) { LogMsg("RemoveAutoTunnelDomainStatus: Could not create CFString domain"); return; }
	
	if (CFDictionaryContainsKey(domainStatusDict, domain))
		{
		CFDictionaryRemoveValue(domainStatusDict, domain);
		if (!m->ShutdownTime) mDNSDynamicStoreSetConfig(kmDNSBackToMyMacConfig, mDNSNULL, domainStatusDict);
		}
	CFRelease(domain);
	}

mDNSlocal mStatus CheckQuestionForStatus(const DNSQuestion *const q)
	{
	if (q->LongLived)
		{
		if (q->servAddr.type == mDNSAddrType_IPv4 && mDNSIPv4AddressIsOnes(q->servAddr.ip.v4))
			return mStatus_NoSuchRecord;
		else if (q->state == LLQ_Poll)
			return mStatus_PollingMode;
		else if (q->state != LLQ_Established && !q->DuplicateOf)
			return mStatus_TransientErr;
		}
	
	return mStatus_NoError;
}

mDNSlocal mStatus UpdateLLQStatus(const mDNS *const m, char *buffer, int bufsz, const DomainAuthInfo *const info)
	{
	mStatus status = mStatus_NoError;
	DNSQuestion* q, *worst_q = mDNSNULL;
	for (q = m->Questions; q; q=q->next)
		if (q->AuthInfo == info)
			{
			mStatus newStatus = CheckQuestionForStatus(q);
			if 		(newStatus == mStatus_NoSuchRecord) { status = newStatus; worst_q = q; break; }
			else if (newStatus == mStatus_PollingMode)  { status = newStatus; worst_q = q; }
			else if (newStatus == mStatus_TransientErr && status == mStatus_NoError) { status = newStatus; worst_q = q; }
			}
	
	if      (status == mStatus_NoError)      mDNS_snprintf(buffer, bufsz, "Success");
	else if (status == mStatus_NoSuchRecord) mDNS_snprintf(buffer, bufsz, "GetZoneData %s: %##s", worst_q->nta ? "not yet complete" : "failed", worst_q->qname.c);
	else if (status == mStatus_PollingMode)  mDNS_snprintf(buffer, bufsz, "Query polling %##s", worst_q->qname.c);
	else if (status == mStatus_TransientErr) mDNS_snprintf(buffer, bufsz, "Query not yet established %##s", worst_q->qname.c);
	return status;
	}

mDNSlocal mStatus UpdateRRStatus(const mDNS *const m, char *buffer, int bufsz, const DomainAuthInfo *const info)
	{
	AuthRecord *r;

	if (info->deltime) return mStatus_NoError;
	for (r = m->ResourceRecords; r; r = r->next)
		{
		// This function is called from UpdateAutoTunnelDomainStatus which in turn may be called from
		// a callback e.g., CheckNATMappings. GetAuthInfoFor_internal does not like that (reentrancy being 1),
		// hence we inline the code here. We just need the lock to walk the list of AuthInfos which the caller
		// has already checked
		const domainname *n = r->resrec.name;
		while (n->c[0])
			{
			DomainAuthInfo *ptr;
			for (ptr = m->AuthInfoList; ptr; ptr = ptr->next)
				if (SameDomainName(&ptr->domain, n))
					{
					if (ptr == info && (r->updateError == mStatus_BadSig || r->updateError == mStatus_BadKey))
						{
						mDNS_snprintf(buffer, bufsz, "Resource record update failed for %##s", r->resrec.name);
						return r->updateError;
						}
					}
			n = (const domainname *)(n->c + 1 + n->c[0]);
			}
		}
	return mStatus_NoError;
	}

#endif // ndef NO_SECURITYFRAMEWORK

// MUST be called with lock held
mDNSlocal void UpdateAutoTunnelDomainStatus(const mDNS *const m, const DomainAuthInfo *const info)
	{
#ifdef NO_SECURITYFRAMEWORK
	(void)m;
	(void)info;
#else
	const NATTraversalInfo *const llq = m->LLQNAT.clientContext ? &m->LLQNAT : mDNSNULL;
	const NATTraversalInfo *const tun = info->AutoTunnelNAT.clientContext ? &info->AutoTunnelNAT : mDNSNULL;
	char buffer[1024];
	mDNSu32 buflen = 0;
	CFMutableDictionaryRef dict = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	CFStringRef domain = NULL;
	CFStringRef tmp = NULL;
	CFNumberRef num = NULL;
	mStatus status = mStatus_NoError;
	mStatus llqStatus = mStatus_NoError;
	char llqBuffer[1024];
	
	if (!m->mDNS_busy) LogMsg("UpdateAutoTunnelDomainStatus: ERROR!! Lock not held");
	if (!domainStatusDict)
		{
		domainStatusDict = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
		if (!domainStatusDict) { LogMsg("UpdateAutoTunnelDomainStatus: Could not create CFDictionary domainStatusDict"); return; }
		}
	
	if (!dict) { LogMsg("UpdateAutoTunnelDomainStatus: Could not create CFDictionary dict"); return; }

	buflen = mDNS_snprintf(buffer, sizeof(buffer), "%##s", info->domain.c);
	if (info->AutoTunnel == dnsprefix) buffer[buflen-1] = 0; // Strip the trailing dot for Classic
	domain = CFStringCreateWithCString(NULL, buffer, kCFStringEncodingUTF8);
	if (!domain) { LogMsg("UpdateAutoTunnelDomainStatus: Could not create CFString domain"); return; }

	mDNS_snprintf(buffer, sizeof(buffer), "%#a", &m->Router);
	tmp = CFStringCreateWithCString(NULL, buffer, kCFStringEncodingUTF8);
	if (!tmp)
		LogMsg("UpdateAutoTunnelDomainStatus: Could not create CFString RouterAddress");
	else
		{
		CFDictionarySetValue(dict, CFSTR("RouterAddress"), tmp);
		CFRelease(tmp);
		}
	
	mDNS_snprintf(buffer, sizeof(buffer), "%.4a", &m->ExternalAddress);
	tmp = CFStringCreateWithCString(NULL, buffer, kCFStringEncodingUTF8);
	if (!tmp)
		LogMsg("UpdateAutoTunnelDomainStatus: Could not create CFString ExternalAddress");
	else
		{
		CFDictionarySetValue(dict, CFSTR("ExternalAddress"), tmp);
		CFRelease(tmp);
		}
	
	if (llq)
		{
		mDNSu32 port = mDNSVal16(llq->ExternalPort);
		
		num = CFNumberCreate(NULL, kCFNumberSInt32Type, &port);
		if (!num)
			LogMsg("UpdateAutoTunnelDomainStatus: Could not create CFNumber LLQExternalPort");
		else
			{
			CFDictionarySetValue(dict, CFSTR("LLQExternalPort"), num);
			CFRelease(num);
			}

		if (llq->Result)
			{
			num = CFNumberCreate(NULL, kCFNumberSInt32Type, &llq->Result);
			if (!num)
				LogMsg("UpdateAutoTunnelDomainStatus: Could not create CFNumber LLQNPMStatus");
			else
				{
				CFDictionarySetValue(dict, CFSTR("LLQNPMStatus"), num);
				CFRelease(num);
				}
			}
		}
	
	if (tun)
		{
		mDNSu32 port = mDNSVal16(tun->ExternalPort);
		
		num = CFNumberCreate(NULL, kCFNumberSInt32Type, &port);
		if (!num)
			LogMsg("UpdateAutoTunnelDomainStatus: Could not create CFNumber AutoTunnelExternalPort");
		else
			{
			CFDictionarySetValue(dict, CFSTR("AutoTunnelExternalPort"), num);
			CFRelease(num);
			}

		if (tun->Result)
			{
			num = CFNumberCreate(NULL, kCFNumberSInt32Type, &tun->Result);
			if (!num)
				LogMsg("UpdateAutoTunnelDomainStatus: Could not create CFNumber AutoTunnelNPMStatus");
			else
				{
				CFDictionarySetValue(dict, CFSTR("AutoTunnelNPMStatus"), num);
				CFRelease(num);
				}
			}
		}
	if (tun || llq)
		{
		mDNSu32 code = m->LastNATMapResultCode;
		
		num = CFNumberCreate(NULL, kCFNumberSInt32Type, &code);
		if (!num)
			LogMsg("UpdateAutoTunnelDomainStatus: Could not create CFNumber LastNATMapResultCode");
		else
			{
			CFDictionarySetValue(dict, CFSTR("LastNATMapResultCode"), num);
			CFRelease(num);
			}
		}
	
	mDNS_snprintf(buffer, sizeof(buffer), "Success");
	llqStatus = UpdateLLQStatus(m, llqBuffer, sizeof(llqBuffer), info);
	status = UpdateRRStatus(m, buffer, sizeof(buffer), info);

	// If we have a bad signature error updating a RR, it overrides any error as it needs to be
	// reported so that it can be fixed automatically (or the user needs to be notified)
	if (status != mStatus_NoError)
		{
		LogInfo("UpdateAutoTunnelDomainStatus: RR Status %d, %s", status, buffer);
		}
	else if (m->Router.type == mDNSAddrType_None)
		{
		status = mStatus_NoRouter;
		mDNS_snprintf(buffer, sizeof(buffer), "No network connection - none");
		}
	else if (m->Router.type == mDNSAddrType_IPv4 && mDNSIPv4AddressIsZero(m->Router.ip.v4))
		{
		status = mStatus_NoRouter;
		mDNS_snprintf(buffer, sizeof(buffer), "No network connection - v4 zero");
		}
	else if (mDNSIPv6AddressIsZero(m->AutoTunnelRelayAddrOut))
		{
		status = info->AutoTunnel == btmmprefix ? mStatus_ServiceNotRunning : mStatus_PollingMode;
		mDNS_snprintf(buffer, sizeof(buffer), "No relay connection");
		}
	else if (!llq && !tun)
		{
		status = mStatus_NotInitializedErr;
		mDNS_snprintf(buffer, sizeof(buffer), "Neither LLQ nor AutoTunnel NAT port mapping is currently active");
		}
	else if (llqStatus == mStatus_NoSuchRecord)
		{
		status = llqStatus;
		mDNS_snprintf(buffer, sizeof(buffer), llqBuffer);
		}
	else if (info->AutoTunnel == btmmprefix && ((llq && llq->Result == mStatus_DoubleNAT) || (tun && tun->Result == mStatus_DoubleNAT)))
		{
		status = mStatus_DoubleNAT;
		mDNS_snprintf(buffer, sizeof(buffer), "Double NAT: Router is reporting an external address");
		}
	else if (info->AutoTunnel == btmmprefix && ((llq && llq->Result == mStatus_NATPortMappingDisabled) || (tun && tun->Result == mStatus_NATPortMappingDisabled) ||
	         (m->LastNATMapResultCode == NATErr_Refused && ((llq && !llq->Result && mDNSIPPortIsZero(llq->ExternalPort)) || (tun && !tun->Result && mDNSIPPortIsZero(tun->ExternalPort))))))
		{
		status = mStatus_NATPortMappingDisabled;
		mDNS_snprintf(buffer, sizeof(buffer), "NAT-PMP is disabled on the router");
		}
	else if (info->AutoTunnel == btmmprefix && ((llq && llq->Result) || (tun && tun->Result)))
		{
		status = mStatus_NATTraversal;
		mDNS_snprintf(buffer, sizeof(buffer), "Error obtaining NAT port mapping from router");
		}
	else if (info->AutoTunnel == btmmprefix && ((llq && mDNSIPPortIsZero(llq->ExternalPort)) || (tun && mDNSIPPortIsZero(tun->ExternalPort))))
		{
		status = mStatus_NATTraversal;
		mDNS_snprintf(buffer, sizeof(buffer), "Unable to obtain NAT port mapping from router");
		}
	else if (info->AutoTunnel == btmmprefix || llqStatus != mStatus_PollingMode)
		{
		status = llqStatus;
		mDNS_snprintf(buffer, sizeof(buffer), llqBuffer);		
		LogInfo("UpdateAutoTunnelDomainStatus: LLQ Status %d, %s", status, buffer);
		}
	else
		{
		mDNS_snprintf(buffer, sizeof(buffer), "Polling success");		
		}
	
	num = CFNumberCreate(NULL, kCFNumberSInt32Type, &status);
	if (!num)
		LogMsg("UpdateAutoTunnelDomainStatus: Could not create CFNumber StatusCode");
	else
		{
		CFDictionarySetValue(dict, CFSTR("StatusCode"), num);
		CFRelease(num);
		}
		
	tmp = CFStringCreateWithCString(NULL, buffer, kCFStringEncodingUTF8);
	if (!tmp)
		LogMsg("UpdateAutoTunnelDomainStatus: Could not create CFString StatusMessage");
	else
		{
		CFDictionarySetValue(dict, CFSTR("StatusMessage"), tmp);
		CFRelease(tmp);
		}

	if (!CFDictionaryContainsKey(domainStatusDict, domain) ||
	    !CFEqual(dict, (CFMutableDictionaryRef)CFDictionaryGetValue(domainStatusDict, domain)))
	    {
		CFDictionarySetValue(domainStatusDict, domain, dict);
		if (!m->ShutdownTime) 
			{
			static char statusBuf[16];
			mDNS_snprintf(statusBuf, sizeof(statusBuf), "%d", (int)status);
			mDNSASLLog((uuid_t *)&m->asl_uuid, "autotunnel.domainstatus", status ? "failure" : "success", statusBuf, "");
			mDNSDynamicStoreSetConfig(kmDNSBackToMyMacConfig, mDNSNULL, domainStatusDict);
			}
		}
		
	CFRelease(domain);
	CFRelease(dict);

	debugf("UpdateAutoTunnelDomainStatus: %s", buffer);
#endif // def NO_SECURITYFRAMEWORK
	}

// MUST be called with lock held
mDNSexport void UpdateAutoTunnelDomainStatuses(const mDNS *const m)
	{
#ifdef NO_SECURITYFRAMEWORK
	(void)m;
#else
	if (!m->mDNS_busy) LogMsg("UpdateAutoTunnelDomainStatuses: ERROR!! Lock not held");
	DomainAuthInfo* info;
	for (info = m->AuthInfoList; info; info = info->next)
		if (info->AutoTunnel && !info->deltime)
			UpdateAutoTunnelDomainStatus(m, info);
#endif // def NO_SECURITYFRAMEWORK
	}
	
// MUST be called with lock held
mDNSlocal mDNSBool TunnelServers(mDNS *const m)
	{
	AuthRecord *r;
	for (r = m->ResourceRecords; r; r = r->next)
		if (r->resrec.rrtype == kDNSType_SRV)
			{
			DomainAuthInfo *AuthInfo = GetAuthInfoForName_internal(m, r->resrec.name);
			if (AuthInfo && AuthInfo->AutoTunnel && !AuthInfo->deltime) return(mDNStrue);
			}

	return(mDNSfalse);
	}

// MUST be called with lock held
mDNSlocal mDNSBool TunnelClients(mDNS *const m)
	{
	ClientTunnel *p;
	for (p = m->TunnelClients; p; p = p->next)
		if (p->q.ThisQInterval < 0)
			return(mDNStrue);
	return(mDNSfalse);
	}

mDNSlocal void UpdateAnonymousRacoonConfig(mDNS *m)		// Determine whether we need racoon to accept incoming connections
	{
	DomainAuthInfo *info;
	
	for (info = m->AuthInfoList; info; info = info->next)
		if (info->AutoTunnel && !info->deltime && (!mDNSIPPortIsZero(info->AutoTunnelNAT.ExternalPort) || !mDNSIPv6AddressIsZero(m->AutoTunnelRelayAddrIn)))
			break;
	
	if (info != AnonymousRacoonConfig)
		{
		AnonymousRacoonConfig = info;
		// Create or revert configuration file, and start (or SIGHUP) Racoon
		(void)mDNSConfigureServer(AnonymousRacoonConfig ? kmDNSUp : kmDNSDown, AnonymousRacoonConfig ? AnonymousRacoonConfig->AutoTunnel : mDNSNULL, AnonymousRacoonConfig ? &AnonymousRacoonConfig->domain : mDNSNULL);
		}
	}

// Caller should hold the lock. We don't call mDNS_Register (which acquires the lock) in this function because
// sometimes the caller may already be holding the lock e.g., RegisterAutoTunnel6Record and sometimes
// not e.g., RegisterAutoTunnelServiceRecord
mDNSlocal void RegisterAutoTunnelHostRecord(mDNS *m, DomainAuthInfo *info)
	{
	mStatus err;
	mDNSBool NATProblem;

	if (!m->mDNS_busy) LogMsg("RegisterAutoTunnelHostRecord: ERROR!! Lock not held");

	// We use AutoTunnelNAT.clientContext to infer that SetupLocalAutoTunnelInterface_internal has been
	// called at least once with some Services/Records in the domain and hence it is safe to register
	// records when this function is called.
	if (!info->AutoTunnelNAT.clientContext) { LogInfo("RegisterAutoTunnelHostRecord: No services registered, not registering the record\n"); return; }

	// Are we behind a NAT with no NAT-PMP support or behind a Double NAT ? Double NATs may have
	// NAT-PMP support but it still does not provide inbound connectivity. If there is no NAT-PMP
	// support, ExternalPort is zero. If we are behind a Double NAT, then the NATResult is non-zero.

	NATProblem = mDNSIPPortIsZero(info->AutoTunnelNAT.ExternalPort) || info->AutoTunnelNAT.Result;

	if (mDNSIPv6AddressIsZero(m->AutoTunnelRelayAddrIn))
		{
		// If we don't have a relay address, check to see if we are behind a Double NAT or NAT with no NAT-PMP
		// support.  
		if (NATProblem)
			{
			LogInfo("RegisterAutoTunnelHostRecord %##s, not registering the Host Record, Neither AutoTunnel6 nor NAT is available, ClientContext %p, ExternalPort %d, NAT Result %d", info->domain.c, info->AutoTunnelNAT.clientContext, mDNSVal16(info->AutoTunnelNAT.ExternalPort), info->AutoTunnelNAT.Result);
			return;
			}
		}
	else
		{
		// Relay address may be non-zero but we might be going to sleep as the utun interface is not removed
		// when going to sleep. If we are awake, we don't care about the NATProblem as the relay connnection
		// is up. If we are going to sleep, we should not register the host record if we have a NAT problem.
		if (m->SleepState != SleepState_Awake && NATProblem)
			{
			LogInfo("RegisterAutoTunnelHostRecord %##s, not registering the Host Record, Not in awake state(%d), and some NAT Problem, ClientContext %p, ExternalPort %d, NAT Result %d", info->domain.c, m->SleepState, info->AutoTunnelNAT.clientContext, mDNSVal16(info->AutoTunnelNAT.ExternalPort), info->AutoTunnelNAT.Result);
			return;
			}
		}

	// Note:
	//
	// We use zero Requested port to infer that we should not be calling Register anymore as it might
	// be shutdown or the DomainAuthInfo is going away.
	//
	// We can use a different set of state variables to track the above as the records registered in
	// this function is not dependent on NAT traversal info. For the sake of simplicity, we just
	// reuse the NAT variables.

	// Set up our address record for the internal tunnel address
	// (User-visible user-friendly host name, used as target in AutoTunnel SRV records)
	if (!mDNSIPPortIsZero(info->AutoTunnelNAT.RequestedPort) && info->AutoTunnelHostRecord.resrec.RecordType == kDNSRecordTypeUnregistered)
		{
		info->AutoTunnelHostRecord.namestorage.c[0] = 0;
		AppendDomainLabel(&info->AutoTunnelHostRecord.namestorage, &m->hostlabel);
		AppendDomainName (&info->AutoTunnelHostRecord.namestorage, &info->domain);
		info->AutoTunnelHostRecord.resrec.rdata->u.ipv6 = m->AutoTunnelHostAddr;
		info->AutoTunnelHostRecord.resrec.RecordType = kDNSRecordTypeKnownUnique;

		err = mDNS_Register_internal(m, &info->AutoTunnelHostRecord);

		if (err) LogMsg("RegisterAutoTunnelHostRecord error %d registering AutoTunnelHostRecord %##s", err, info->AutoTunnelHostRecord.namestorage.c);
		else
			{
			// Make sure we trigger the registration of all SRV records in regState_NoTarget again
			m->NextSRVUpdate = NonZeroTime(m->timenow);
			LogInfo("RegisterAutoTunnelHostRecord registering AutoTunnelHostRecord %##s", info->AutoTunnelHostRecord.namestorage.c);
			}
		}
	else LogInfo("RegisterAutoTunnelHostRecord: Not registering Context %p Port %d Type %d", info->AutoTunnelNAT.clientContext, mDNSVal16(info->AutoTunnelNAT.RequestedPort), info->AutoTunnelHostRecord.resrec.RecordType);
	}

mDNSlocal void DeregisterAutoTunnelHostRecord(mDNS *m, DomainAuthInfo *info)
	{
	LogInfo("DeregisterAutoTunnelHostRecord %##s", info->domain.c);

	// Don't deregister if we have the AutoTunnel6 or AutoTunnelService records are registered.
	// They indicate that BTMM is working
	if (info->AutoTunnel6Record.resrec.RecordType > kDNSRecordTypeDeregistering ||
	    info->AutoTunnelService.resrec.RecordType > kDNSRecordTypeDeregistering)
		{
		LogInfo("DeregisterAutoTunnelHostRecord %##s, not deregistering the Host Record AutoTunnel6 RecordType:%d AutoTunnel RecordType: %d", info->domain.c,
			info->AutoTunnel6Record.resrec.RecordType, info->AutoTunnelService.resrec.RecordType);
		return;
		}

	if (info->AutoTunnelHostRecord.resrec.RecordType > kDNSRecordTypeDeregistering)
		{
		mStatus err = mDNS_Deregister(m, &info->AutoTunnelHostRecord);
		if (err)
			{
			info->AutoTunnelHostRecord.resrec.RecordType = kDNSRecordTypeUnregistered;
			LogMsg("DeregisterAutoTunnelHostRecord error %d deregistering AutoTunnelHostRecord %##s", err, info->AutoTunnelHostRecord.namestorage.c);
			}
		else LogInfo("DeregisterAutoTunnelHostRecord: Deregistered AutoTunnel Host Record");
		}
	else LogInfo("DeregisterAutoTunnelHostRecord: Not deregistering Host Record state:%d", info->AutoTunnelHostRecord.resrec.RecordType);
	}

mDNSlocal void RegisterAutoTunnelServiceRecords(mDNS *m, DomainAuthInfo *info)
	{
	mStatus err;

	//if (m->mDNS_busy) LogMsg("RegisterAutoTunnelServiceRecords: ERROR!! Lock already held");

	if (info->AutoTunnelNAT.clientContext && !info->AutoTunnelNAT.Result && !mDNSIPPortIsZero(info->AutoTunnelNAT.ExternalPort) && info->AutoTunnelTarget.resrec.RecordType == kDNSRecordTypeUnregistered)
		{
		LogInfo("RegisterAutoTunnelServiceRecords %##s (%#s)", info->domain.c, m->hostlabel.c);

		// 1. Set up our address record for the external tunnel address
		// (Constructed name, not generally user-visible, used as target in IKE tunnel's SRV record)
		info->AutoTunnelTarget.namestorage.c[0] = 0;
		AppendDomainLabel(&info->AutoTunnelTarget.namestorage, &m->AutoTunnelLabel);
		AppendDomainName (&info->AutoTunnelTarget.namestorage, &info->domain);
		info->AutoTunnelTarget.resrec.rdata->u.ipv4 = info->AutoTunnelNAT.ExternalAddress;
		info->AutoTunnelTarget.resrec.RecordType = kDNSRecordTypeKnownUnique;

		err = mDNS_Register(m, &info->AutoTunnelTarget);
		if (err) LogMsg("RegisterAutoTunnelServiceRecords error %d registering AutoTunnelTarget %##s", err, info->AutoTunnelTarget.namestorage.c);
		else LogInfo("RegisterAutoTunnelServiceRecords registering AutoTunnelTarget %##s", info->AutoTunnelTarget.namestorage.c);

		}

	if (info->AutoTunnelNAT.clientContext && !info->AutoTunnelNAT.Result && !mDNSIPPortIsZero(info->AutoTunnelNAT.ExternalPort) && info->AutoTunnelService.resrec.RecordType == kDNSRecordTypeUnregistered)
		{
		// 2. Set up IKE tunnel's SRV record: "AutoTunnelHostRecord SRV 0 0 port AutoTunnelTarget"
		AssignDomainName (&info->AutoTunnelService.namestorage, (const domainname*) "\x0B" "_autotunnel" "\x04" "_udp");
		AppendDomainLabel(&info->AutoTunnelService.namestorage, &m->hostlabel);
		AppendDomainName (&info->AutoTunnelService.namestorage, &info->domain);
		info->AutoTunnelService.resrec.rdata->u.srv.priority = 0;
		info->AutoTunnelService.resrec.rdata->u.srv.weight   = 0;
		info->AutoTunnelService.resrec.rdata->u.srv.port     = info->AutoTunnelNAT.ExternalPort;
		AssignDomainName(&info->AutoTunnelService.resrec.rdata->u.srv.target, &info->AutoTunnelTarget.namestorage);
		info->AutoTunnelService.resrec.RecordType = kDNSRecordTypeKnownUnique;
		err = mDNS_Register(m, &info->AutoTunnelService);
		if (err) LogMsg("RegisterAutoTunnelServiceRecords error %d registering AutoTunnelService %##s", err, info->AutoTunnelService.namestorage.c);
		else LogInfo("RegisterAutoTunnelServiceRecords registering AutoTunnelService %##s", info->AutoTunnelService.namestorage.c);

		LogInfo("AutoTunnel server listening for connections on %##s[%.4a]:%d:%##s[%.16a]",
			info->AutoTunnelTarget.namestorage.c,     &m->AdvertisedV4.ip.v4, mDNSVal16(info->AutoTunnelNAT.IntPort),
			info->AutoTunnelHostRecord.namestorage.c, &m->AutoTunnelHostAddr);
		}
	mDNS_Lock(m);
	RegisterAutoTunnelHostRecord(m, info);
	mDNS_Unlock(m);
	}

mDNSlocal void DeregisterAutoTunnelServiceRecords(mDNS *m, DomainAuthInfo *info)
	{
	LogInfo("DeregisterAutoTunnelServiceRecords %##s", info->domain.c);
	if (info->AutoTunnelTarget.resrec.RecordType > kDNSRecordTypeDeregistering)
		{
		mStatus err = mDNS_Deregister(m, &info->AutoTunnelTarget);
		if (err)
			{
			info->AutoTunnelTarget.resrec.RecordType = kDNSRecordTypeUnregistered;
			LogMsg("DeregisterAutoTunnelServiceRecords error %d deregistering AutoTunnelTarget %##s", err, info->AutoTunnelTarget.namestorage.c);
			}
		else LogInfo("DeregisterAutoTunnelServiceRecords: Deregistered AutoTunnel Target  Record");

		}
	else LogInfo("DeregisterAutoTunnelServiceRecords: Not deregistering Target record state:%d", info->AutoTunnelService.resrec.RecordType);

	if (info->AutoTunnelService.resrec.RecordType > kDNSRecordTypeDeregistering)
		{
		mStatus err = mDNS_Deregister(m, &info->AutoTunnelService);
		if (err)
			{
			info->AutoTunnelService.resrec.RecordType = kDNSRecordTypeUnregistered;
			LogMsg("DeregisterAutoTunnelServiceRecords error %d deregistering AutoTunnelService %##s", err, info->AutoTunnelService.namestorage.c);
			}
		else LogInfo("DeregisterAutoTunnelServiceRecords: Deregistered AutoTunnel Service Record");

		}
	else LogInfo("DeregisterAutoTunnelServiceRecords: Not deregistering service records state:%d", info->AutoTunnelService.resrec.RecordType);

	DeregisterAutoTunnelHostRecord(m, info);
	}

// Caller should hold the lock. We don't call mDNS_Register (which acquires the lock) in this function because
// sometimes the caller may already be holding the lock e.g., SetupLocalAutoTunnelInterface_internal and sometimes
// not e.g., AutoTunnelHostNameChanged
mDNSlocal void RegisterAutoTunnelDevInfoRecord(mDNS *m, DomainAuthInfo *info)
	{
	mStatus err;

	if (!m->mDNS_busy) LogMsg("RegisterAutoTunnelDevInfoRecord: Lock not held");
	// Note:
	// a. We use AutoTunnelNAT.clientContext to infer that SetupLocalAutoTunnelInterface_internal has been
	//    called at least once with some Services/Records in the domain and hence it is safe to register
	//    records when this function is called.
	//
	// b. We use zero Requested port to infer that we should not be calling Register anymore as it might
	//    be shutdown or the DomainAuthInfo is going away.
	//
	// We can use a different set of state variables to track the above as the records registered in
	// this function is not dependent on NAT traversal info. For the sake of simplicity, we just
	// reuse the NAT variables.

	// Set up device info record
	if (info->AutoTunnelNAT.clientContext && !mDNSIPPortIsZero(info->AutoTunnelNAT.RequestedPort) && info->AutoTunnelDeviceInfo.resrec.RecordType == kDNSRecordTypeUnregistered)
		{
		ConstructServiceName(&info->AutoTunnelDeviceInfo.namestorage, &m->nicelabel, &DeviceInfoName, &info->domain);
		mDNSu8 len = m->HIHardware.c[0] < 255 - 6 ? m->HIHardware.c[0] : 255 - 6;
		mDNSPlatformMemCopy(info->AutoTunnelDeviceInfo.resrec.rdata->u.data + 1, "model=", 6);
		mDNSPlatformMemCopy(info->AutoTunnelDeviceInfo.resrec.rdata->u.data + 7, m->HIHardware.c + 1, len);
		info->AutoTunnelDeviceInfo.resrec.rdata->u.data[0] = 6 + len;	// "model=" plus the device string
		info->AutoTunnelDeviceInfo.resrec.rdlength         = 7 + len;	// One extra for the length byte at the start of the string
		info->AutoTunnelDeviceInfo.resrec.RecordType = kDNSRecordTypeKnownUnique;

		err = mDNS_Register_internal(m, &info->AutoTunnelDeviceInfo);
		if (err) LogMsg("RegisterAutoTunnelDevInfoRecord error %d registering AutoTunnelDeviceInfo %##s", err, info->AutoTunnelDeviceInfo.namestorage.c);
		else LogInfo("RegisterAutoTunnelDevInfoRecord registering AutoTunnelDeviceInfo %##s", info->AutoTunnelDeviceInfo.namestorage.c);
		}
	}

#ifndef NO_SECURITYFRAMEWORK
mDNSlocal void DeregisterAutoTunnelDevInfoRecord(mDNS *m, DomainAuthInfo *info)
	{
	LogInfo("DeregisterAutoTunnelDevInfoRecord %##s", info->domain.c);

	if (info->AutoTunnelDeviceInfo.resrec.RecordType > kDNSRecordTypeDeregistering)
		{
		mStatus err = mDNS_Deregister(m, &info->AutoTunnelDeviceInfo);
		if (err)
			{
			info->AutoTunnelDeviceInfo.resrec.RecordType = kDNSRecordTypeUnregistered;
			LogMsg("DeregisterAutoTunnelDevInfoRecord error %d deregistering AutoTunnelDeviceInfo %##s", err, info->AutoTunnelDeviceInfo.namestorage.c);
			}
		else LogInfo("DeregisterAutoTunnelDevInfoRecord: Deregistered AutoTunnel Device Info");
		}
	else LogInfo("DeregisterAutoTunnelDevInfoRecord: Not deregistering DeviceInfo  Record state:%d", info->AutoTunnelDeviceInfo.resrec.RecordType);
	}
#endif


// Caller should hold the lock. We don't call mDNS_Register (which acquires the lock) in this function because
// sometimes the caller may already be holding the lock e.g., SetupLocalAutoTunnelInterface_internal and sometimes
// not e.g., AutoTunnelHostNameChanged
mDNSlocal void RegisterAutoTunnel6Record(mDNS *m, DomainAuthInfo *info)
	{
	mStatus err;

	if (!m->mDNS_busy) LogMsg("RegisterAutoTunnel6Record: ERROR!! Lock not held");

	// We deregister the AutoTunnel6Record during sleep and come back here (AutoTunnelRecordCallback) to
	// register the address if needed. During that time, we might get a network change event which finds
	// that the utun interface exists and tries to register the AutoTunnel6Record which should be stopped.
	// Also the RelayAddress is reinitialized during that process which in turn causes the AutoTunnelRecordCallback
	// to re-register again. To stop these, we check for the SleepState and register only if we are awake.
	if (m->SleepState != SleepState_Awake)
		{
		LogInfo("RegisterAutoTunnel6Record: Not in awake state, SleepState %d", m->SleepState);
		return;
		}

	// if disabled administratively, don't register
	if (!m->RegisterAutoTunnel6 || DisableInboundRelayConnection)
		{
		LogInfo("RegisterAutoTunnel6Record: registration Disabled RegisterAutoTunnel6 %d, DisableInbound %d",
			m->RegisterAutoTunnel6, DisableInboundRelayConnection);
		return;
		}
	//
	// If we have a valid Relay address, we need to register it now. When we got a valid address, we may not
	// have registered it because it was waiting for at least one service to become active in the BTMM domain.
	// During network change event, we might be called multiple times while the "Connectivity" key did not
	// change, so check to see if the value has changed. This can also be zero when we are deregistering and
	// getting called from the AutoTunnelRecordCallback

	if (mDNSIPv6AddressIsZero(m->AutoTunnelRelayAddrIn))
		{
		LogInfo("RegisterAutoTunnel6Record: Relay address is zero, not registering");
		return;
		}

	if ((info->AutoTunnel6Record.resrec.RecordType > kDNSRecordTypeDeregistering) &&
		(mDNSSameIPv6Address(info->AutoTunnel6Record.resrec.rdata->u.ipv6, m->AutoTunnelRelayAddrIn)))
		{
		LogInfo("RegisterAutoTunnel6Record: Relay address %.16a same, not registering", &m->AutoTunnelRelayAddrIn);
		return;
		}

	// Note:
	// a. We use AutoTunnelNAT.clientContext to infer that SetupLocalAutoTunnelInterface_internal has been
	//    called at least once with some Services/Records in the domain and hence it is safe to register
	//    records when this function is called.
	//
	// b. We use zero Requested port to infer that we should not be calling Register anymore as it might
	//    be shutdown or the DomainAuthInfo is going away.
	//
	// We can use a different set of state variables to track the above as the records registered in
	// this function is not dependent on NAT traversal info. For the sake of simplicity, we just
	// reuse the NAT variables.

	if (info->AutoTunnelNAT.clientContext && !mDNSIPPortIsZero(info->AutoTunnelNAT.RequestedPort) &&
	    info->AutoTunnel6Record.resrec.RecordType == kDNSRecordTypeUnregistered)
		{
		AssignDomainName (&info->AutoTunnel6Record.namestorage, (const domainname*) "\x0C" "_autotunnel6");
		AppendDomainLabel(&info->AutoTunnel6Record.namestorage, &m->hostlabel);
		AppendDomainName (&info->AutoTunnel6Record.namestorage, &info->domain);
		info->AutoTunnel6Record.resrec.rdata->u.ipv6 = m->AutoTunnelRelayAddrIn;
		info->AutoTunnel6Record.resrec.RecordType = kDNSRecordTypeKnownUnique;

		err = mDNS_Register_internal(m, &info->AutoTunnel6Record);
		if (err) LogMsg("RegisterAutoTunnel6Record error %d registering AutoTunnel6 Record %##s", err, info->AutoTunnel6Record.namestorage.c);
		else LogInfo("RegisterAutoTunnel6Record registering AutoTunnel6 Record %##s", info->AutoTunnel6Record.namestorage.c);

		LogInfo("AutoTunnel6 server listening for connections on %##s[%.16a] :%##s[%.16a]",
			info->AutoTunnel6Record.namestorage.c,     &m->AutoTunnelRelayAddrIn,
			info->AutoTunnelHostRecord.namestorage.c, &m->AutoTunnelHostAddr);

		} else {LogInfo("RegisterAutoTunnel6Record: client context %p, RequestedPort %d, Address %.16a, record type %d", info->AutoTunnelNAT.clientContext, info->AutoTunnelNAT.RequestedPort, &m->AutoTunnelRelayAddrIn, info->AutoTunnel6Record.resrec.RecordType);}

	RegisterAutoTunnelHostRecord(m, info);
	// When the AutoTunnel6 record comes up, we need to kick racoon and update the status.
	// If we had a port mapping, we would have done it in RegisterAutoTunnelServiceRecords.
	// If we don't have a port mapping, we need to do it here.
	UpdateAnonymousRacoonConfig(m);		// Determine whether we need racoon to accept incoming connections
	UpdateAutoTunnelDomainStatus(m, info);
	}

mDNSlocal void DeregisterAutoTunnel6Record(mDNS *m, DomainAuthInfo *info)
	{
	LogInfo("DeregisterAutoTunnel6Record %##s", info->domain.c);

	if (info->AutoTunnel6Record.resrec.RecordType > kDNSRecordTypeDeregistering)
		{
		mStatus err = mDNS_Deregister(m, &info->AutoTunnel6Record);
		if (err)
			{
			info->AutoTunnel6Record.resrec.RecordType = kDNSRecordTypeUnregistered;
			info->AutoTunnel6Record.resrec.rdata->u.ipv6 = zerov6Addr;
			LogMsg("DeregisterAutoTunnel6Record error %d deregistering AutoTunnel6Record %##s", err, info->AutoTunnel6Record.namestorage.c);
			}
		else LogInfo("DeregisterAutoTunnel6Record: Deregistered AutoTunnel6 Record");
		}
	else LogInfo("DeregisterAutoTunnel6Record: Not deregistering AuoTunnel6 record state:%d", info->AutoTunnel6Record.resrec.RecordType);

	DeregisterAutoTunnelHostRecord(m, info);
	// UpdateAutoTunnelDomainStatus is careful enough not to turn it on if we don't have
	// a external port mapping. Otherwise, it will be turned off.
	mDNS_Lock(m);
	UpdateAutoTunnelDomainStatus(m, info);
	mDNS_Unlock(m);
	}

mDNSlocal void AutoTunnelRecordCallback(mDNS *const m, AuthRecord *const rr, mStatus result)
	{
	DomainAuthInfo *info = (DomainAuthInfo *)rr->RecordContext;
	if (result == mStatus_MemFree)
		{
		LogInfo("AutoTunnelRecordCallback MemFree %s", ARDisplayString(m, rr));
		// Reset the host record namestorage to force high-level PTR/SRV/TXT to deregister
		if (rr == &info->AutoTunnelHostRecord)
			{
			rr->namestorage.c[0] = 0;
			m->NextSRVUpdate = NonZeroTime(m->timenow);
			LogInfo("AutoTunnelRecordCallback: NextSRVUpdate in %d %d", m->NextSRVUpdate - m->timenow, m->timenow);
			}
		if (m->ShutdownTime) {LogInfo("AutoTunnelRecordCallback: Shutdown, returning");return;}
		if (rr == &info->AutoTunnelHostRecord)
			{
			LogInfo("AutoTunnelRecordCallback: calling RegisterAutoTunnelHostRecord");
			RegisterAutoTunnelHostRecord(m,info);
			}
		else if (rr == &info->AutoTunnelDeviceInfo)
			{
			LogInfo("AutoTunnelRecordCallback: Calling RegisterAutoTunnelDevInfoRecord");
			RegisterAutoTunnelDevInfoRecord(m,info);
			}
		else if (rr == &info->AutoTunnelService || rr == &info->AutoTunnelTarget)
			{
			LogInfo("AutoTunnelRecordCallback: Calling RegisterAutoTunnelServiceRecords");
			RegisterAutoTunnelServiceRecords(m,info);
			}
		else if (rr == &info->AutoTunnel6Record)
			{
			LogInfo("AutoTunnelRecordCallback: Calling RegisterAutoTunnel6Record");
			info->AutoTunnel6Record.resrec.rdata->u.ipv6 = zerov6Addr;
			RegisterAutoTunnel6Record(m,info);
			}
		}
	}

#ifndef NO_SECURITYFRAMEWORK
mDNSlocal void AutoTunnelDeleteAuthInfoState(mDNS *m, DomainAuthInfo *info)
	{
	LogInfo("AutoTunnelDeleteAuthInfoState: Cleaning up state releated to Domain AuthInfo %##s", info->domain.c);

	m->NextSRVUpdate = NonZeroTime(m->timenow);
	DeregisterAutoTunnelDevInfoRecord(m, info);
	DeregisterAutoTunnelServiceRecords(m, info);
	DeregisterAutoTunnel6Record(m, info);
	UpdateAnonymousRacoonConfig(m);		// Determine whether we need racoon to accept incoming connections
	UpdateAutoTunnelDomainStatus(m, info);
	}
#endif // ndef NO_SECURITYFRAMEWORK

mDNSlocal void AutoTunnelNATCallback(mDNS *m, NATTraversalInfo *n)
	{
	DomainAuthInfo *info = (DomainAuthInfo *)n->clientContext;
	LogInfo("AutoTunnelNATCallback Result %d %.4a Internal %d External %d %#s.%##s",
		n->Result, &n->ExternalAddress, mDNSVal16(n->IntPort), mDNSVal16(n->ExternalPort), m->hostlabel.c, info->domain.c);

	m->NextSRVUpdate = NonZeroTime(m->timenow);
	LogInfo("AutoTunnelNATCallback: NextSRVUpdate in %d %d", m->NextSRVUpdate - m->timenow, m->timenow);

	DeregisterAutoTunnelServiceRecords(m, info);
	RegisterAutoTunnelServiceRecords(m, info);
	
	UpdateAnonymousRacoonConfig(m);		// Determine whether we need racoon to accept incoming connections

	UpdateAutoTunnelDomainStatus(m, (DomainAuthInfo *)n->clientContext);
	}

mDNSlocal void AbortDeregistration(mDNS *const m, AuthRecord *rr)
	{
	if (rr->resrec.RecordType == kDNSRecordTypeDeregistering)
		{
		LogInfo("Aborting deregistration of %s", ARDisplayString(m, rr));
		CompleteDeregistration(m, rr);
		}
	else if (rr->resrec.RecordType != kDNSRecordTypeUnregistered)
		LogMsg("AbortDeregistration ERROR RecordType %02X for %s", ARDisplayString(m, rr));
	}

mDNSlocal void AutoTunnelHostNameChanged(mDNS *m, DomainAuthInfo *info)
	{
	LogInfo("AutoTunnelHostNameChanged %#s.%##s", m->hostlabel.c, info->domain.c);

#ifndef NO_SECURITYFRAMEWORK
	DeregisterAutoTunnelDevInfoRecord(m, info);
#endif
	DeregisterAutoTunnelServiceRecords(m, info);
	DeregisterAutoTunnel6Record(m, info);
	RegisterAutoTunnelServiceRecords(m, info);

	mDNS_Lock(m);
	RegisterAutoTunnelDevInfoRecord(m, info);
	RegisterAutoTunnel6Record(m, info);
	m->NextSRVUpdate = NonZeroTime(m->timenow);
	mDNS_Unlock(m);
	}

mDNSlocal void SetupLocalAutoTunnel6Records(mDNS *const m, DomainAuthInfo *info)
	{
	AbortDeregistration(m, &info->AutoTunnelDeviceInfo);
	AbortDeregistration(m, &info->AutoTunnel6Record);

	// When the BTMM is turned on/off too quickly, following things happen.
	//
	// 1. Turning off BTMM triggers deregistration of the DevInfo/AutoTunnel6 etc. records
	// 2. While (1) is in progress, the BTMM is turned on
	//
	// At step (2), mDNS_SetSecretForDomain clears info->deltime indicating that the domain is valid
	// while we have not processed the turning off BTMM completely. Hence, we end up calling this
	// function to re-register the records. AbortDeregistration above aborts the Deregistration as the
	// records are still in Deregistering state and in AutoTunnelRecordCallback we end up registering
	// again. So, we have to be careful below to not call mDNS_SetupResourceRecord again which will
	// reset the state to Unregistered and registering again will lead to error as it is registered
	// and already in the list. Hence, register below only if needed.

	if (info->AutoTunnelDeviceInfo.resrec.RecordType != kDNSRecordTypeUnregistered ||
	    info->AutoTunnel6Record.resrec.RecordType != kDNSRecordTypeUnregistered)
		{
		LogInfo("SetupLocalAutoTunnel6Records: AutoTunnel Records not in Unregistered state: Device: %d, AutoTunnel6:%d",
			info->AutoTunnelDeviceInfo.resrec.RecordType, info->AutoTunnel6Record.resrec.RecordType);
		}

	if (info->AutoTunnelDeviceInfo.resrec.RecordType == kDNSRecordTypeUnregistered)
		{
		mDNS_SetupResourceRecord(&info->AutoTunnelDeviceInfo, mDNSNULL, mDNSInterface_Any, kDNSType_TXT,  kStandardTTL, kDNSRecordTypeUnregistered, AuthRecordAny, AutoTunnelRecordCallback, info);
		RegisterAutoTunnelDevInfoRecord(m, info);
		}
	if (info->AutoTunnel6Record.resrec.RecordType == kDNSRecordTypeUnregistered)
		{
		mDNS_SetupResourceRecord(&info->AutoTunnel6Record, mDNSNULL, mDNSInterface_Any, kDNSType_AAAA, kHostNameTTL, kDNSRecordTypeUnregistered, AuthRecordAny, AutoTunnelRecordCallback, info);
		RegisterAutoTunnel6Record(m, info);
		}

	UpdateAnonymousRacoonConfig(m);		// Determine whether we need racoon to accept incoming connections
	
	UpdateAutoTunnelDomainStatus(m, info);
	m->NextSRVUpdate = NonZeroTime(m->timenow);
	}

// Before SetupLocalAutoTunnelInterface_internal is called,
// m->AutoTunnelHostAddr.b[0] must be non-zero, and there must be at least one TunnelClient or TunnelServer
// Must be called with the lock held
mDNSexport void SetupLocalAutoTunnelInterface_internal(mDNS *const m, mDNSBool servicesStarting)
	{
	// 1. Configure the local IPv6 ULA BTMM address
	if (!m->AutoTunnelHostAddrActive)
		{
		m->AutoTunnelHostAddrActive = mDNStrue;
		LogInfo("SetupLocalAutoTunnelInterface_internal: Setting up AutoTunnel address %.16a", &m->AutoTunnelHostAddr);
		(void)mDNSAutoTunnelInterfaceUpDown(kmDNSUp, m->AutoTunnelHostAddr.b);
		}

	// 2. If we have at least one server (pending) listening, publish our records
	// The services may not be in the list when it is first trying to resolve the target of the SRV record.
	// servicesStarting is an indication of that. Use that instead of looking up in the list of Services/Records.
	if (servicesStarting || TunnelServers(m))
		{
		DomainAuthInfo *info;
		for (info = m->AuthInfoList; info; info = info->next)
			{
			if (info->AutoTunnel && !info->deltime && !info->AutoTunnelNAT.clientContext)
				{
				// If we just resurrected a DomainAuthInfo that is still deregistering, we need to abort the
				// deregistration process before re-using the AuthRecord memory
				//
				// Note: We don't need the same caution as in SetupLocalAutoTunnel6Records (see the comments there)
				// as AutoTunnelRecordCallback (called as a result of AbortDeregistration) will not end up registering
				// the records because clientContext is still NULL

				AbortDeregistration(m, &info->AutoTunnelTarget);
				AbortDeregistration(m, &info->AutoTunnelService);
				AbortDeregistration(m, &info->AutoTunnelHostRecord);

				mDNS_SetupResourceRecord(&info->AutoTunnelTarget,     mDNSNULL, mDNSInterface_Any, kDNSType_A,    kHostNameTTL,
					kDNSRecordTypeUnregistered, AuthRecordAny, AutoTunnelRecordCallback, info);
				mDNS_SetupResourceRecord(&info->AutoTunnelService,    mDNSNULL, mDNSInterface_Any, kDNSType_SRV,  kHostNameTTL,
					kDNSRecordTypeUnregistered, AuthRecordAny, AutoTunnelRecordCallback, info);
				mDNS_SetupResourceRecord(&info->AutoTunnelHostRecord, mDNSNULL, mDNSInterface_Any, kDNSType_AAAA, kHostNameTTL,
					kDNSRecordTypeUnregistered, AuthRecordAny, AutoTunnelRecordCallback, info);

				// Try to get a NAT port mapping for the AutoTunnelService
				info->AutoTunnelNAT.clientCallback   = AutoTunnelNATCallback;
				info->AutoTunnelNAT.clientContext    = info;
				info->AutoTunnelNAT.Protocol         = NATOp_MapUDP;
				info->AutoTunnelNAT.IntPort          = IPSECPort;
				info->AutoTunnelNAT.RequestedPort    = IPSECPort;
				info->AutoTunnelNAT.NATLease         = 0;
				mStatus err = mDNS_StartNATOperation_internal(m, &info->AutoTunnelNAT);
				if (err) LogMsg("SetupLocalAutoTunnelInterface_internal: error %d starting NAT mapping", err);

				// Register the records that can be done without communicating with the NAT
				//
				// Note: This should be done after we setup the AutoTunnelNAT information
				// as some of the fields in that structure are used to infer other information
				// e.g., is it okay to register now ?

				SetupLocalAutoTunnel6Records(m, info);

				}
			}
		}
	}

mDNSlocal mStatus AutoTunnelSetKeys(ClientTunnel *tun, mDNSBool AddNew)
	{
	mDNSv6Addr loc_outer6;
	mDNSv6Addr rmt_outer6;

	// When we are tunneling over IPv6 Relay address, the port number is zero
	if (mDNSIPPortIsZero(tun->rmt_outer_port))
		{
		loc_outer6 = tun->loc_outer6;
		rmt_outer6 = tun->rmt_outer6;
		}
	else
		{
		loc_outer6 = zerov6Addr;
		loc_outer6.b[0] = tun->loc_outer.b[0];
		loc_outer6.b[1] = tun->loc_outer.b[1];
		loc_outer6.b[2] = tun->loc_outer.b[2];
		loc_outer6.b[3] = tun->loc_outer.b[3];

		rmt_outer6 = zerov6Addr;
		rmt_outer6.b[0] = tun->rmt_outer.b[0];
		rmt_outer6.b[1] = tun->rmt_outer.b[1];
		rmt_outer6.b[2] = tun->rmt_outer.b[2];
		rmt_outer6.b[3] = tun->rmt_outer.b[3];
		}

	return(mDNSAutoTunnelSetKeys(AddNew ? kmDNSAutoTunnelSetKeysReplace : kmDNSAutoTunnelSetKeysDelete, tun->loc_inner.b, loc_outer6.b, kRacoonPort, tun->rmt_inner.b, rmt_outer6.b, mDNSVal16(tun->rmt_outer_port), tun->prefix, SkipLeadingLabels(&tun->dstname, 1)));
	}

// If the EUI-64 part of the IPv6 ULA matches, then that means the two addresses point to the same machine
#define mDNSSameClientTunnel(A,B) ((A)->l[2] == (B)->l[2] && (A)->l[3] == (B)->l[3])

mDNSlocal void ReissueBlockedQuestionWithType(mDNS *const m, domainname *d, mDNSBool success, mDNSu16 qtype)
	{
	DNSQuestion *q = m->Questions;
	while (q)
		{
		if (q->NoAnswer == NoAnswer_Suspended && q->qtype == qtype && q->AuthInfo && q->AuthInfo->AutoTunnel && SameDomainName(&q->qname, d))
			{
			LogInfo("Restart %##s (%s)", q->qname.c, DNSTypeName(q->qtype));
			mDNSQuestionCallback *tmp = q->QuestionCallback;
			q->QuestionCallback = AutoTunnelCallback;	// Set QuestionCallback to suppress another call back to AddNewClientTunnel
			mDNS_StopQuery(m, q);
			mDNS_StartQuery(m, q);
			q->QuestionCallback = tmp;					// Restore QuestionCallback back to the real value
			if (!success) q->NoAnswer = NoAnswer_Fail;
			// When we call mDNS_StopQuery, it's possible for other subordinate questions like the GetZoneData query to be cancelled too.
			// In general we have to assume that the question list might have changed in arbitrary ways.
			// This code is itself called from a question callback, so the m->CurrentQuestion mechanism is
			// already in use. The safest solution is just to go back to the start of the list and start again.
			// In principle this sounds like an n^2 algorithm, but in practice we almost always activate
			// just one suspended question, so it's really a 2n algorithm.
			q = m->Questions;
			}
		else
			q = q->next;
		}
	}

mDNSlocal void ReissueBlockedQuestions(mDNS *const m, domainname *d, mDNSBool success)
	{
	// 1. We deliberately restart AAAA queries before A queries, because in the common case where a BTTM host has
	//    a v6 address but no v4 address, we prefer the caller to get the positive AAAA response before the A NXDOMAIN.
	// 2. In the case of AAAA queries, if our tunnel setup failed, then we return a deliberate failure indication to the caller --
	//    even if the name does have a valid AAAA record, we don't want clients trying to connect to it without a properly encrypted tunnel.
	// 3. For A queries we never fabricate failures -- if a BTTM service is really using raw IPv4, then it doesn't need the IPv6 tunnel.
	ReissueBlockedQuestionWithType(m, d, success, kDNSType_AAAA);
	ReissueBlockedQuestionWithType(m, d, mDNStrue, kDNSType_A);
	}

mDNSlocal void UnlinkAndReissueBlockedQuestions(mDNS *const m, ClientTunnel *tun, mDNSBool success)
	{
	ClientTunnel **p = &m->TunnelClients;
	while (*p != tun && *p) p = &(*p)->next;
	if (*p) *p = tun->next;
	ReissueBlockedQuestions(m, &tun->dstname, success);
	LogInfo("UnlinkAndReissueBlockedQuestions: Disposing ClientTunnel %p", tun);
	freeL("ClientTunnel", tun);
	}

mDNSlocal mDNSBool TunnelClientDeleteMatching(mDNS *const m, ClientTunnel *tun, mDNSBool v6Tunnel)
	{
	ClientTunnel **p;
	mDNSBool needSetKeys = mDNStrue;

	p = &tun->next;
	while (*p)
		{
		// Is this a tunnel to the same host that we are trying to setup now?
		if (!mDNSSameClientTunnel(&(*p)->rmt_inner, &tun->rmt_inner)) p = &(*p)->next;
		else
			{
			ClientTunnel *old = *p;
			if (v6Tunnel)
				{
				if (!mDNSIPPortIsZero(old->rmt_outer_port)) { p = &old->next; continue; }
				LogInfo("TunnelClientDeleteMatching: Found existing IPv6 AutoTunnel for %##s %.16a", old->dstname.c, &old->rmt_inner);
				if (old->q.ThisQInterval >= 0)
					{
					LogInfo("TunnelClientDeleteMatching: Stopping query on IPv6 AutoTunnel for %##s %.16a", old->dstname.c, &old->rmt_inner);
					mDNS_StopQuery(m, &old->q);
					}
				else if (!mDNSSameIPv6Address((*p)->rmt_inner, tun->rmt_inner) ||
						!mDNSSameIPv6Address(old->loc_inner, tun->loc_inner)   ||
					 	!mDNSSameIPv6Address(old->loc_outer6, tun->loc_outer6) ||
					 	!mDNSSameIPv6Address(old->rmt_outer6, tun->rmt_outer6))
					{
					// Delete the old tunnel if the current tunnel to the same host does not have the same ULA or
					// the other parameters of the tunnel are different
					LogInfo("TunnelClientDeleteMatching: Deleting existing IPv6 AutoTunnel for %##s %.16a", old->dstname.c, &old->rmt_inner);
					AutoTunnelSetKeys(old, mDNSfalse);
					}
				else
					{
					// Reusing the existing tunnel means that we reuse the IPsec SAs and the policies. We delete the old
					// as "tun" and "old" are identical
					LogInfo("TunnelClientDeleteMatching: Reusing the existing IPv6 AutoTunnel for %##s %.16a", old->dstname.c,
						&old->rmt_inner);
					needSetKeys = mDNSfalse;
					}
				}
			else 
				{
				if (mDNSIPPortIsZero(old->rmt_outer_port)) { p = &old->next; continue; }
				LogInfo("TunnelClientDeleteMatching: Found existing IPv4 AutoTunnel for %##s %.16a", old->dstname.c, &old->rmt_inner);
				if (old->q.ThisQInterval >= 0)
					{
					LogInfo("TunnelClientDeleteMatching: Stopping query on IPv4 AutoTunnel for %##s %.16a", old->dstname.c, &old->rmt_inner);
					mDNS_StopQuery(m, &old->q);
					}
				else if (!mDNSSameIPv6Address((*p)->rmt_inner, tun->rmt_inner) ||
						!mDNSSameIPv6Address(old->loc_inner, tun->loc_inner)   ||
					 	!mDNSSameIPv4Address(old->loc_outer, tun->loc_outer)   ||
					 	!mDNSSameIPv4Address(old->rmt_outer, tun->rmt_outer)   ||
					 	!mDNSSameIPPort(old->rmt_outer_port, tun->rmt_outer_port))
					{
					// Delete the old tunnel if the current tunnel to the same host does not have the same ULA or
					// the other parameters of the tunnel are different
					LogInfo("TunnelClientDeleteMatching: Deleting existing IPv4 AutoTunnel for %##s %.16a", old->dstname.c, &old->rmt_inner);
					AutoTunnelSetKeys(old, mDNSfalse);
					}
				else
					{
					// Reusing the existing tunnel means that we reuse the IPsec SAs and the policies. We delete the old
					// as "tun" and "old" are identical
					LogInfo("TunnelClientDeleteMatching: Reusing the existing IPv4 AutoTunnel for %##s %.16a", old->dstname.c,
						&old->rmt_inner);
					needSetKeys = mDNSfalse;
					}
				}

			*p = old->next;
			LogInfo("TunnelClientDeleteMatching: Disposing ClientTunnel %p", old);
			freeL("ClientTunnel", old);
			}
		}
		return needSetKeys;
	}

// v6Tunnel indicates whether to delete a tunnel whose outer header is IPv6. If false, outer IPv4
// tunnel will be deleted
mDNSlocal void TunnelClientDeleteAny(mDNS *const m, ClientTunnel *tun, mDNSBool v6Tunnel)
	{
	ClientTunnel **p;

	p = &tun->next;
	while (*p)
		{
		// If there is more than one client tunnel to the same host, delete all of them.
		// We do this by just checking against the EUI64 rather than the full address
		if (!mDNSSameClientTunnel(&(*p)->rmt_inner, &tun->rmt_inner)) p = &(*p)->next;
		else
			{
			ClientTunnel *old = *p;
			if (v6Tunnel)
				{
				if (!mDNSIPPortIsZero(old->rmt_outer_port)) { p = &old->next; continue;}
				LogInfo("TunnelClientDeleteAny: Found existing IPv6 AutoTunnel for %##s %.16a", old->dstname.c, &old->rmt_inner);
				}
			else 
				{
				if (mDNSIPPortIsZero(old->rmt_outer_port)) { p = &old->next; continue;}
				LogInfo("TunnelClientDeleteAny: Found existing IPv4 AutoTunnel for %##s %.16a", old->dstname.c, &old->rmt_inner);
				}
			if (old->q.ThisQInterval >= 0)
				{
				LogInfo("TunnelClientDeleteAny: Stopping query on AutoTunnel for %##s %.16a", old->dstname.c, &old->rmt_inner);
				mDNS_StopQuery(m, &old->q);
				}
			else 
				{
				LogInfo("TunnelClientDeleteAny: Deleting existing AutoTunnel for %##s %.16a", old->dstname.c, &old->rmt_inner);
				AutoTunnelSetKeys(old, mDNSfalse);
				}
			*p = old->next;
			LogInfo("TunnelClientDeleteAny: Disposing ClientTunnel %p", old);
			freeL("ClientTunnel", old);
			}
		}
	}

mDNSlocal void TunnelClientFinish(mDNS *const m, DNSQuestion *question, const ResourceRecord *const answer)
	{
	mDNSBool needSetKeys = mDNStrue;
	ClientTunnel *tun = (ClientTunnel *)question->QuestionContext;
	mDNSBool v6Tunnel = mDNSfalse;

	// If the port is zero, then we have a relay address of the peer
	if (mDNSIPPortIsZero(tun->rmt_outer_port))
		v6Tunnel = mDNStrue;

	if (v6Tunnel)
		{
		LogInfo("TunnelClientFinish: Relay address %.16a", &answer->rdata->u.ipv6);
		tun->rmt_outer6 = answer->rdata->u.ipv6;
		tun->loc_outer6 = m->AutoTunnelRelayAddrOut;
		}
	else
		{
		LogInfo("TunnelClientFinish: SRV target address %.4a", &answer->rdata->u.ipv4);
		tun->rmt_outer = answer->rdata->u.ipv4;
		mDNSAddr tmpDst = { mDNSAddrType_IPv4, {{{0}}} };
		tmpDst.ip.v4 = tun->rmt_outer;
		mDNSAddr tmpSrc = zeroAddr;
		mDNSPlatformSourceAddrForDest(&tmpSrc, &tmpDst);
		if (tmpSrc.type == mDNSAddrType_IPv4) tun->loc_outer = tmpSrc.ip.v4;
		else tun->loc_outer = m->AdvertisedV4.ip.v4;
		}

	question->ThisQInterval = -1;		// So we know this tunnel setup has completed
	tun->loc_inner = m->AutoTunnelHostAddr;

	// If we found a v6Relay address for our peer, delete all the v4Tunnels for our peer and
	// look for existing tunnels to see whether they have the same information for our peer.
	// If not, delete them and need to create a new tunnel. If they are same, just use the
	// same tunnel. Do the similar thing if we found a v4Tunnel end point for our peer.
	TunnelClientDeleteAny(m, tun, !v6Tunnel);
	needSetKeys = TunnelClientDeleteMatching(m, tun, v6Tunnel);

	if (needSetKeys) LogInfo("TunnelClientFinish: New %s AutoTunnel for %##s %.16a", (v6Tunnel ? "IPv6" : "IPv4"), tun->dstname.c, &tun->rmt_inner);
	else LogInfo("TunnelClientFinish: Reusing exiting %s AutoTunnel for %##s %.16a", (v6Tunnel ? "IPv6" : "IPv4"), tun->dstname.c, &tun->rmt_inner);

	if (m->AutoTunnelHostAddr.b[0]) { mDNS_Lock(m); SetupLocalAutoTunnelInterface_internal(m, mDNSfalse); mDNS_Unlock(m); };

	mStatus result = needSetKeys ? AutoTunnelSetKeys(tun, mDNStrue) : mStatus_NoError;
	static char msgbuf[32];
	mDNS_snprintf(msgbuf, sizeof(msgbuf), "Tunnel setup - %d", result);
	mDNSASLLog((uuid_t *)&m->asl_uuid, "autotunnel.config", result ? "failure" : "success", msgbuf, "");
	// Kick off any questions that were held pending this tunnel setup
	ReissueBlockedQuestions(m, &tun->dstname, (result == mStatus_NoError) ? mDNStrue : mDNSfalse);
	}

mDNSexport void AutoTunnelCallback(mDNS *const m, DNSQuestion *question, const ResourceRecord *const answer, QC_result AddRecord)
	{
	ClientTunnel *tun = (ClientTunnel *)question->QuestionContext;

	LogInfo("AutoTunnelCallback tun %p AddRecord %d rdlength %d qtype %d", tun, AddRecord, answer->rdlength, question->qtype);

	if (!AddRecord) return;
	mDNS_StopQuery(m, question);

	// If we are looking up the AAAA record for _autotunnel6, don't consider it as failure.
	// The code below will look for _autotunnel._udp SRV record followed by A record
	if (tun->tc_state != TC_STATE_AAAA_PEER_RELAY && !answer->rdlength)
		{
		LogInfo("AutoTunnelCallback NXDOMAIN %##s (%s)", question->qname.c, DNSTypeName(question->qtype));
		static char msgbuf[16];
		mDNS_snprintf(msgbuf, sizeof(msgbuf), "%s lookup", DNSTypeName(question->qtype));
		mDNSASLLog((uuid_t *)&m->asl_uuid, "autotunnel.config", "failure", msgbuf, "");
		UnlinkAndReissueBlockedQuestions(m, tun, mDNSfalse);
		return;
		}

	switch (tun->tc_state)
		{
		case TC_STATE_AAAA_PEER:
			if (question->qtype != kDNSType_AAAA)
				{
				LogMsg("AutoTunnelCallback: Bad question type %d in TC_STATE_AAAA_PEER", question->qtype);
				}
			if (mDNSSameIPv6Address(answer->rdata->u.ipv6, m->AutoTunnelHostAddr))
				{
				LogInfo("AutoTunnelCallback: suppressing tunnel to self %.16a", &answer->rdata->u.ipv6);
				UnlinkAndReissueBlockedQuestions(m, tun, mDNStrue);
				return;
				}
			tun->rmt_inner = answer->rdata->u.ipv6;
			LogInfo("AutoTunnelCallback:TC_STATE_AAAA_PEER: dst host %.16a", &tun->rmt_inner);
			if (!mDNSIPv6AddressIsZero(m->AutoTunnelRelayAddrOut))
				{
				LogInfo("AutoTunnelCallback: Looking up _autotunnel6 AAAA");
				tun->tc_state = TC_STATE_AAAA_PEER_RELAY;
				question->qtype = kDNSType_AAAA;
				AssignDomainName(&question->qname, (const domainname*) "\x0C" "_autotunnel6");
				}
			else
				{
				LogInfo("AutoTunnelCallback: Looking up _autotunnel._udp SRV");
				tun->tc_state = TC_STATE_SRV_PEER;
				question->qtype = kDNSType_SRV;
				AssignDomainName(&question->qname, (const domainname*) "\x0B" "_autotunnel" "\x04" "_udp");
				}
			AppendDomainName(&question->qname, &tun->dstname);
			mDNS_StartQuery(m, &tun->q);
			return;
		case TC_STATE_AAAA_PEER_RELAY:
			if (question->qtype != kDNSType_AAAA)
				{
				LogMsg("AutoTunnelCallback: Bad question type %d in TC_STATE_AAAA_PEER_RELAY", question->qtype);
				}
			// If it failed, look for the SRV record.
			if (!answer->rdlength)
				{
				LogInfo("AutoTunnelCallback: Looking up _autotunnel6 AAAA failed, trying SRV");
				tun->tc_state = TC_STATE_SRV_PEER;
				AssignDomainName(&question->qname, (const domainname*) "\x0B" "_autotunnel" "\x04" "_udp");
				AppendDomainName(&question->qname, &tun->dstname);
				question->qtype = kDNSType_SRV;
				mDNS_StartQuery(m, &tun->q);
				return;
				}
			TunnelClientFinish(m, question, answer);
			return;
		case TC_STATE_SRV_PEER:
			if (question->qtype != kDNSType_SRV)
				{
				LogMsg("AutoTunnelCallback: Bad question type %d in TC_STATE_SRV_PEER", question->qtype);
				}
			LogInfo("AutoTunnelCallback: SRV target name %##s", answer->rdata->u.srv.target.c);
			tun->tc_state = TC_STATE_ADDR_PEER;
			AssignDomainName(&tun->q.qname, &answer->rdata->u.srv.target);
			tun->rmt_outer_port = answer->rdata->u.srv.port;
			question->qtype = kDNSType_A;
			mDNS_StartQuery(m, &tun->q);
			return;
		case TC_STATE_ADDR_PEER:
			if (question->qtype != kDNSType_A)
				{
				LogMsg("AutoTunnelCallback: Bad question type %d in TC_STATE_ADDR_PEER", question->qtype);
				}
			TunnelClientFinish(m, question, answer);
			return;
		default:
			LogMsg("AutoTunnelCallback: Unknown question %p", question);
		}
	}

// Must be called with the lock held
mDNSexport void AddNewClientTunnel(mDNS *const m, DNSQuestion *const q)
	{
	ClientTunnel *p = mallocL("ClientTunnel", sizeof(ClientTunnel));
	if (!p) return;
	p->prefix = q->AuthInfo->AutoTunnel;
	AssignDomainName(&p->dstname, &q->qname);
	p->MarkedForDeletion = mDNSfalse;
	p->loc_inner      = zerov6Addr;
	p->loc_outer      = zerov4Addr;
	p->loc_outer6     = zerov6Addr;
	p->rmt_inner      = zerov6Addr;
	p->rmt_outer      = zerov4Addr;
	p->rmt_outer6     = zerov6Addr;
	p->rmt_outer_port = zeroIPPort;
	p->tc_state = TC_STATE_AAAA_PEER;
	p->next = m->TunnelClients;
	m->TunnelClients = p;		// We intentionally build list in reverse order

	p->q.InterfaceID      = mDNSInterface_Any;
	p->q.Target           = zeroAddr;
	AssignDomainName(&p->q.qname, &q->qname);
	p->q.qtype            = kDNSType_AAAA;
	p->q.qclass           = kDNSClass_IN;
	p->q.LongLived        = mDNSfalse;
	p->q.ExpectUnique     = mDNStrue;
	p->q.ForceMCast       = mDNSfalse;
	p->q.ReturnIntermed   = mDNStrue;
	p->q.SuppressUnusable = mDNSfalse;
	p->q.SearchListIndex  = 0;
	p->q.AppendSearchDomains = 0;
	p->q.RetryWithSearchDomains = mDNSfalse;
	p->q.TimeoutQuestion  = 0;
	p->q.WakeOnResolve    = 0;
	p->q.qnameOrig        = mDNSNULL;
	p->q.QuestionCallback = AutoTunnelCallback;
	p->q.QuestionContext  = p;

	LogInfo("AddNewClientTunnel start tun %p %##s (%s)%s", p, &q->qname.c, DNSTypeName(q->qtype), q->LongLived ? " LongLived" : "");
	mDNS_StartQuery_internal(m, &p->q);
	}

#endif // APPLE_OSX_mDNSResponder

#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark -
#pragma mark - Power State & Configuration Change Management
#endif

mDNSlocal mStatus UpdateInterfaceList(mDNS *const m, mDNSs32 utc)
	{
	mDNSBool foundav4           = mDNSfalse;
	mDNSBool foundav6           = mDNSfalse;
	struct ifaddrs *ifa         = myGetIfAddrs(1);
	struct ifaddrs *v4Loopback  = NULL;
	struct ifaddrs *v6Loopback  = NULL;
	char defaultname[64];
#ifndef NO_IPV6
	int InfoSocket              = socket(AF_INET6, SOCK_DGRAM, 0);
	if (InfoSocket < 3 && errno != EAFNOSUPPORT) LogMsg("UpdateInterfaceList: InfoSocket error %d errno %d (%s)", InfoSocket, errno, strerror(errno));
#endif

	// During wakeup, we may get a network change notification e.g., new addresses, before we get
	// a wake notification. This means that we have not set AnnounceOwner. Registering interfaces with
	// core would cause us to probe again which will conflict with the sleep proxy server, if we had
	// registered with it when going to sleep. Hence, need to delay until we get the wake notification

	if (m->SleepState == SleepState_Sleeping) ifa = NULL;

	while (ifa)
		{
#if LIST_ALL_INTERFACES
		if (ifa->ifa_addr->sa_family == AF_APPLETALK)
			LogMsg("UpdateInterfaceList: %5s(%d) Flags %04X Family %2d is AF_APPLETALK",
				ifa->ifa_name, if_nametoindex(ifa->ifa_name), ifa->ifa_flags, ifa->ifa_addr->sa_family);
		else if (ifa->ifa_addr->sa_family == AF_LINK)
			LogMsg("UpdateInterfaceList: %5s(%d) Flags %04X Family %2d is AF_LINK",
				ifa->ifa_name, if_nametoindex(ifa->ifa_name), ifa->ifa_flags, ifa->ifa_addr->sa_family);
		else if (ifa->ifa_addr->sa_family != AF_INET && ifa->ifa_addr->sa_family != AF_INET6)
			LogMsg("UpdateInterfaceList: %5s(%d) Flags %04X Family %2d not AF_INET (2) or AF_INET6 (30)",
				ifa->ifa_name, if_nametoindex(ifa->ifa_name), ifa->ifa_flags, ifa->ifa_addr->sa_family);
		if (!(ifa->ifa_flags & IFF_UP))
			LogMsg("UpdateInterfaceList: %5s(%d) Flags %04X Family %2d Interface not IFF_UP",
				ifa->ifa_name, if_nametoindex(ifa->ifa_name), ifa->ifa_flags, ifa->ifa_addr->sa_family);
		if (!(ifa->ifa_flags & IFF_MULTICAST))
			LogMsg("UpdateInterfaceList: %5s(%d) Flags %04X Family %2d Interface not IFF_MULTICAST",
				ifa->ifa_name, if_nametoindex(ifa->ifa_name), ifa->ifa_flags, ifa->ifa_addr->sa_family);
		if (ifa->ifa_flags & IFF_POINTOPOINT)
			LogMsg("UpdateInterfaceList: %5s(%d) Flags %04X Family %2d Interface IFF_POINTOPOINT",
				ifa->ifa_name, if_nametoindex(ifa->ifa_name), ifa->ifa_flags, ifa->ifa_addr->sa_family);
		if (ifa->ifa_flags & IFF_LOOPBACK)
			LogMsg("UpdateInterfaceList: %5s(%d) Flags %04X Family %2d Interface IFF_LOOPBACK",
				ifa->ifa_name, if_nametoindex(ifa->ifa_name), ifa->ifa_flags, ifa->ifa_addr->sa_family);
#endif

		if (ifa->ifa_addr->sa_family == AF_LINK)
			{
			struct sockaddr_dl *sdl = (struct sockaddr_dl *)ifa->ifa_addr;
			if (sdl->sdl_type == IFT_ETHER && sdl->sdl_alen == sizeof(m->PrimaryMAC) && mDNSSameEthAddress(&m->PrimaryMAC, &zeroEthAddr))
				mDNSPlatformMemCopy(m->PrimaryMAC.b, sdl->sdl_data + sdl->sdl_nlen, 6);
			}

		if (ifa->ifa_flags & IFF_UP && ifa->ifa_addr)
			if (ifa->ifa_addr->sa_family == AF_INET || ifa->ifa_addr->sa_family == AF_INET6)
				{
				if (!ifa->ifa_netmask)
					{
					mDNSAddr ip;
					SetupAddr(&ip, ifa->ifa_addr);
					LogMsg("getifaddrs: ifa_netmask is NULL for %5s(%d) Flags %04X Family %2d %#a",
						ifa->ifa_name, if_nametoindex(ifa->ifa_name), ifa->ifa_flags, ifa->ifa_addr->sa_family, &ip);
					}
				// Apparently it's normal for the sa_family of an ifa_netmask to sometimes be zero, so we don't complain about that
				// <rdar://problem/5492035> getifaddrs is returning invalid netmask family for fw0 and vmnet
				else if (ifa->ifa_netmask->sa_family != ifa->ifa_addr->sa_family && ifa->ifa_netmask->sa_family != 0)
					{
					mDNSAddr ip;
					SetupAddr(&ip, ifa->ifa_addr);
					LogMsg("getifaddrs ifa_netmask for %5s(%d) Flags %04X Family %2d %#a has different family: %d",
						ifa->ifa_name, if_nametoindex(ifa->ifa_name), ifa->ifa_flags, ifa->ifa_addr->sa_family, &ip, ifa->ifa_netmask->sa_family);
					}
					// Currently we use a few internal ones like mDNSInterfaceID_LocalOnly etc. that are negative values (0, -1, -2).
				else if ((int)if_nametoindex(ifa->ifa_name) <= 0)
					{
					LogMsg("UpdateInterfaceList: if_nametoindex returned zero/negative value for %5s(%d)", ifa->ifa_name, if_nametoindex(ifa->ifa_name));
					}
				else
					{
					// Make sure ifa_netmask->sa_family is set correctly
					// <rdar://problem/5492035> getifaddrs is returning invalid netmask family for fw0 and vmnet
					ifa->ifa_netmask->sa_family = ifa->ifa_addr->sa_family;
					int ifru_flags6 = 0;
#ifndef NO_IPV6
					struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)ifa->ifa_addr;
					if (ifa->ifa_addr->sa_family == AF_INET6 && InfoSocket >= 0)
						{
						struct in6_ifreq ifr6;
						mDNSPlatformMemZero((char *)&ifr6, sizeof(ifr6));
						strlcpy(ifr6.ifr_name, ifa->ifa_name, sizeof(ifr6.ifr_name));
						ifr6.ifr_addr = *sin6;
						if (ioctl(InfoSocket, SIOCGIFAFLAG_IN6, &ifr6) != -1)
							ifru_flags6 = ifr6.ifr_ifru.ifru_flags6;
						verbosedebugf("%s %.16a %04X %04X", ifa->ifa_name, &sin6->sin6_addr, ifa->ifa_flags, ifru_flags6);
						}
#endif
					if (!(ifru_flags6 & (IN6_IFF_NOTREADY | IN6_IFF_DETACHED | IN6_IFF_DEPRECATED | IN6_IFF_TEMPORARY)))
						{
						if (ifa->ifa_flags & IFF_LOOPBACK)
							{
							if (ifa->ifa_addr->sa_family == AF_INET)     v4Loopback = ifa;
#ifndef NO_IPV6
							else if (sin6->sin6_addr.s6_addr[0] != 0xFD) v6Loopback = ifa;
#endif
							}
						else
							{
							NetworkInterfaceInfoOSX *i = AddInterfaceToList(m, ifa, utc);
							if (i && MulticastInterface(i) && i->ifinfo.Advertise)
								{
								if (ifa->ifa_addr->sa_family == AF_INET) foundav4 = mDNStrue;
								else                                     foundav6 = mDNStrue;
								}
							}
						}
					}
				}
		ifa = ifa->ifa_next;
		}

	// For efficiency, we don't register a loopback interface when other interfaces of that family are available and advertising
	if (!foundav4 && v4Loopback) AddInterfaceToList(m, v4Loopback, utc);
	if (!foundav6 && v6Loopback) AddInterfaceToList(m, v6Loopback, utc);

	// Now the list is complete, set the McastTxRx setting for each interface.
	NetworkInterfaceInfoOSX *i;
	for (i = m->p->InterfaceList; i; i = i->next)
		if (i->Exists)
			{
			mDNSBool txrx = MulticastInterface(i);
#if USE_V6_ONLY_WHEN_NO_ROUTABLE_V4
			txrx = txrx && ((i->ifinfo.ip.type == mDNSAddrType_IPv4) || !FindRoutableIPv4(m, i->scope_id));
#endif
			if (i->ifinfo.McastTxRx != txrx)
				{
				i->ifinfo.McastTxRx = txrx;
				i->Exists = 2; // State change; need to deregister and reregister this interface
				}
			}

#ifndef NO_IPV6
	if (InfoSocket >= 0) close(InfoSocket);
#endif

	// If we haven't set up AutoTunnelHostAddr yet, do it now
	if (!mDNSSameEthAddress(&m->PrimaryMAC, &zeroEthAddr) && m->AutoTunnelHostAddr.b[0] == 0)
		{
		m->AutoTunnelHostAddr.b[0x0] = 0xFD;		// Required prefix for "locally assigned" ULA (See RFC 4193)
		m->AutoTunnelHostAddr.b[0x1] = mDNSRandom(255);
		m->AutoTunnelHostAddr.b[0x2] = mDNSRandom(255);
		m->AutoTunnelHostAddr.b[0x3] = mDNSRandom(255);
		m->AutoTunnelHostAddr.b[0x4] = mDNSRandom(255);
		m->AutoTunnelHostAddr.b[0x5] = mDNSRandom(255);
		m->AutoTunnelHostAddr.b[0x6] = mDNSRandom(255);
		m->AutoTunnelHostAddr.b[0x7] = mDNSRandom(255);
		m->AutoTunnelHostAddr.b[0x8] = m->PrimaryMAC.b[0] ^ 0x02;	// See RFC 3513, Appendix A for explanation
		m->AutoTunnelHostAddr.b[0x9] = m->PrimaryMAC.b[1];
		m->AutoTunnelHostAddr.b[0xA] = m->PrimaryMAC.b[2];
		m->AutoTunnelHostAddr.b[0xB] = 0xFF;
		m->AutoTunnelHostAddr.b[0xC] = 0xFE;
		m->AutoTunnelHostAddr.b[0xD] = m->PrimaryMAC.b[3];
		m->AutoTunnelHostAddr.b[0xE] = m->PrimaryMAC.b[4];
		m->AutoTunnelHostAddr.b[0xF] = m->PrimaryMAC.b[5];
		m->AutoTunnelLabel.c[0] = mDNS_snprintf((char*)m->AutoTunnelLabel.c+1, 254, "AutoTunnel-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X",
			m->AutoTunnelHostAddr.b[0x8], m->AutoTunnelHostAddr.b[0x9], m->AutoTunnelHostAddr.b[0xA], m->AutoTunnelHostAddr.b[0xB],
			m->AutoTunnelHostAddr.b[0xC], m->AutoTunnelHostAddr.b[0xD], m->AutoTunnelHostAddr.b[0xE], m->AutoTunnelHostAddr.b[0xF]);
		LogInfo("m->AutoTunnelLabel %#s", m->AutoTunnelLabel.c);
		}
	
	mDNS_snprintf(defaultname, sizeof(defaultname), "%.*s-%02X%02X%02X%02X%02X%02X", HINFO_HWstring_prefixlen, HINFO_HWstring,
		m->PrimaryMAC.b[0], m->PrimaryMAC.b[1], m->PrimaryMAC.b[2], m->PrimaryMAC.b[3], m->PrimaryMAC.b[4], m->PrimaryMAC.b[5]);

	// Set up the nice label
	domainlabel nicelabel;
	nicelabel.c[0] = 0;
	GetUserSpecifiedFriendlyComputerName(&nicelabel);
	if (nicelabel.c[0] == 0)
		{
		debugf("Couldn’t read user-specified Computer Name; using default “%s” instead", defaultname);
		MakeDomainLabelFromLiteralString(&nicelabel, defaultname);
		}

	// Set up the RFC 1034-compliant label
	domainlabel hostlabel;
	hostlabel.c[0] = 0;
	GetUserSpecifiedLocalHostName(&hostlabel);
	if (hostlabel.c[0] == 0)
		{
		debugf("Couldn’t read user-specified Local Hostname; using default “%s.local” instead", defaultname);
		MakeDomainLabelFromLiteralString(&hostlabel, defaultname);
		}

	mDNSBool namechange = mDNSfalse;

	// We use a case-sensitive comparison here because even though changing the capitalization
	// of the name alone is not significant to DNS, it's still a change from the user's point of view
	if (SameDomainLabelCS(m->p->usernicelabel.c, nicelabel.c))
		debugf("Usernicelabel (%#s) unchanged since last time; not changing m->nicelabel (%#s)", m->p->usernicelabel.c, m->nicelabel.c);
	else
		{
		if (m->p->usernicelabel.c[0])	// Don't show message first time through, when we first read name from prefs on boot
			LogMsg("User updated Computer Name from “%#s” to “%#s”", m->p->usernicelabel.c, nicelabel.c);
		m->p->usernicelabel = m->nicelabel = nicelabel;
		namechange = mDNStrue;
		}

	if (SameDomainLabelCS(m->p->userhostlabel.c, hostlabel.c))
		debugf("Userhostlabel (%#s) unchanged since last time; not changing m->hostlabel (%#s)", m->p->userhostlabel.c, m->hostlabel.c);
	else
		{
		if (m->p->userhostlabel.c[0])	// Don't show message first time through, when we first read name from prefs on boot
			LogMsg("User updated Local Hostname from “%#s” to “%#s”", m->p->userhostlabel.c, hostlabel.c);
		m->p->userhostlabel = m->hostlabel = hostlabel;
		mDNS_SetFQDN(m);
		namechange = mDNStrue;
		}

#if APPLE_OSX_mDNSResponder
	if (namechange)		// If either name has changed, we need to tickle our AutoTunnel state machine to update its registered records
		{
		DomainAuthInfo *info;
		for (info = m->AuthInfoList; info; info = info->next)
				if (info->AutoTunnel) AutoTunnelHostNameChanged(m, info);
		}
#endif // APPLE_OSX_mDNSResponder

	return(mStatus_NoError);
	}

// Returns number of leading one-bits in mask: 0-32 for IPv4, 0-128 for IPv6
// Returns -1 if all the one-bits are not contiguous
mDNSlocal int CountMaskBits(mDNSAddr *mask)
	{
	int i = 0, bits = 0;
	int bytes = mask->type == mDNSAddrType_IPv4 ? 4 : mask->type == mDNSAddrType_IPv6 ? 16 : 0;
	while (i < bytes)
		{
		mDNSu8 b = mask->ip.v6.b[i++];
		while (b & 0x80) { bits++; b <<= 1; }
		if (b) return(-1);
		}
	while (i < bytes) if (mask->ip.v6.b[i++]) return(-1);
	return(bits);
	}

// returns count of non-link local V4 addresses registered
mDNSlocal int SetupActiveInterfaces(mDNS *const m, mDNSs32 utc)
	{
	NetworkInterfaceInfoOSX *i;
	int count = 0;
	for (i = m->p->InterfaceList; i; i = i->next)
		if (i->Exists)
			{
			NetworkInterfaceInfo *const n = &i->ifinfo;
			NetworkInterfaceInfoOSX *primary = SearchForInterfaceByName(m, i->ifinfo.ifname, AAAA_OVER_V4 ? AF_UNSPEC : i->sa_family);
			if (!primary) LogMsg("SetupActiveInterfaces ERROR! SearchForInterfaceByName didn't find %s", i->ifinfo.ifname);

			if (i->Registered && i->Registered != primary)	// Sanity check
				{
				LogMsg("SetupActiveInterfaces ERROR! n->Registered %p != primary %p", i->Registered, primary);
				i->Registered = mDNSNULL;
				}

			if (!i->Registered)
				{
				// Note: If i->Registered is set, that means we've called mDNS_RegisterInterface() for this interface,
				// so we need to make sure we call mDNS_DeregisterInterface() before disposing it.
				// If i->Registered is NOT set, then we haven't registered it and we should not try to deregister it
				//

				i->Registered = primary;

				// If i->LastSeen == utc, then this is a brand-new interface, just created, or an interface that never went away.
				// If i->LastSeen != utc, then this is an old interface, previously seen, that went away for (utc - i->LastSeen) seconds.
				// If the interface is an old one that went away and came back in less than a minute, then we're in a flapping scenario.
				i->Occulting = !(i->ifa_flags & IFF_LOOPBACK) && (utc - i->LastSeen > 0 && utc - i->LastSeen < 60);

				// Temporary fix to handle P2P flapping. P2P reuses the scope-id, mac address and the IP address
				// everytime it creates a new interface. We think it is a duplicate and hence consider it
				// as flashing and occulting, that is, flapping. If an interface is marked as flapping, 
				// mDNS_RegisterInterface() changes the probe delay from 1/2 second to 5 seconds and
				// logs a warning message to system.log noting frequent interface transitions.
				if (strncmp(i->ifinfo.ifname, "p2p", 3) == 0)
					{
					LogInfo("SetupActiveInterfaces: P2P %s interface registering %s %s", i->ifinfo.ifname,
						i->Flashing               ? " (Flashing)"  : "",
						i->Occulting              ? " (Occulting)" : "");
					mDNS_RegisterInterface(m, n, 0);
					}
				else
					{
					mDNS_RegisterInterface(m, n, i->Flashing && i->Occulting);
					}

				if (!mDNSAddressIsLinkLocal(&n->ip)) count++;
				LogInfo("SetupActiveInterfaces:   Registered    %5s(%lu) %.6a InterfaceID %p(%p), primary %p, %#a/%d%s%s%s",
					i->ifinfo.ifname, i->scope_id, &i->BSSID, i->ifinfo.InterfaceID, i, primary, &n->ip, CountMaskBits(&n->mask),
					i->Flashing        ? " (Flashing)"  : "",
					i->Occulting       ? " (Occulting)" : "",
					n->InterfaceActive ? " (Primary)"   : "");

				if (!n->McastTxRx)
					debugf("SetupActiveInterfaces:   No Tx/Rx on   %5s(%lu) %.6a InterfaceID %p %#a", i->ifinfo.ifname, i->scope_id, &i->BSSID, i->ifinfo.InterfaceID, &n->ip);
				else
					{
					if (i->sa_family == AF_INET)
						{
						struct ip_mreq imr;
						primary->ifa_v4addr.s_addr = n->ip.ip.v4.NotAnInteger;
						imr.imr_multiaddr.s_addr = AllDNSLinkGroup_v4.ip.v4.NotAnInteger;
						imr.imr_interface        = primary->ifa_v4addr;
	
						// If this is our *first* IPv4 instance for this interface name, we need to do a IP_DROP_MEMBERSHIP first,
						// before trying to join the group, to clear out stale kernel state which may be lingering.
						// In particular, this happens with removable network interfaces like USB Ethernet adapters -- the kernel has stale state
						// from the last time the USB Ethernet adapter was connected, and part of the kernel thinks we've already joined the group
						// on that interface (so we get EADDRINUSE when we try to join again) but a different part of the kernel thinks we haven't
						// joined the group (so we receive no multicasts). Doing an IP_DROP_MEMBERSHIP before joining seems to flush the stale state.
						// Also, trying to make the code leave the group when the adapter is removed doesn't work either,
						// because by the time we get the configuration change notification, the interface is already gone,
						// so attempts to unsubscribe fail with EADDRNOTAVAIL (errno 49 "Can't assign requested address").
						// <rdar://problem/5585972> IP_ADD_MEMBERSHIP fails for previously-connected removable interfaces
						if (SearchForInterfaceByName(m, i->ifinfo.ifname, AF_INET) == i)
							{
							LogInfo("SetupActiveInterfaces: %5s(%lu) Doing precautionary IP_DROP_MEMBERSHIP for %.4a on %.4a", i->ifinfo.ifname, i->scope_id, &imr.imr_multiaddr, &imr.imr_interface);
							mStatus err = setsockopt(m->p->permanentsockets.sktv4, IPPROTO_IP, IP_DROP_MEMBERSHIP, &imr, sizeof(imr));
							if (err < 0 && (errno != EADDRNOTAVAIL))
								LogMsg("setsockopt - IP_DROP_MEMBERSHIP error %d errno %d (%s)", err, errno, strerror(errno));
							}
	
						LogInfo("SetupActiveInterfaces: %5s(%lu) joining IPv4 mcast group %.4a on %.4a", i->ifinfo.ifname, i->scope_id, &imr.imr_multiaddr, &imr.imr_interface);
						mStatus err = setsockopt(m->p->permanentsockets.sktv4, IPPROTO_IP, IP_ADD_MEMBERSHIP, &imr, sizeof(imr));
						// Joining same group twice can give "Address already in use" error -- no need to report that
						if (err < 0 && (errno != EADDRINUSE))
							LogMsg("setsockopt - IP_ADD_MEMBERSHIP error %d errno %d (%s) group %.4a on %.4a", err, errno, strerror(errno), &imr.imr_multiaddr, &imr.imr_interface);
						}
#ifndef NO_IPV6
					if (i->sa_family == AF_INET6)
						{
						struct ipv6_mreq i6mr;
						i6mr.ipv6mr_interface = primary->scope_id;
						i6mr.ipv6mr_multiaddr = *(struct in6_addr*)&AllDNSLinkGroup_v6.ip.v6;
	
						if (SearchForInterfaceByName(m, i->ifinfo.ifname, AF_INET6) == i)
							{
							LogInfo("SetupActiveInterfaces: %5s(%lu) Doing precautionary IPV6_LEAVE_GROUP for %.16a on %u", i->ifinfo.ifname, i->scope_id, &i6mr.ipv6mr_multiaddr, i6mr.ipv6mr_interface);
							mStatus err = setsockopt(m->p->permanentsockets.sktv6, IPPROTO_IPV6, IPV6_LEAVE_GROUP, &i6mr, sizeof(i6mr));
							if (err < 0 && (errno != EADDRNOTAVAIL))
								LogMsg("setsockopt - IPV6_LEAVE_GROUP error %d errno %d (%s) group %.16a on %u", err, errno, strerror(errno), &i6mr.ipv6mr_multiaddr, i6mr.ipv6mr_interface);
							}
	
						LogInfo("SetupActiveInterfaces: %5s(%lu) joining IPv6 mcast group %.16a on %u", i->ifinfo.ifname, i->scope_id, &i6mr.ipv6mr_multiaddr, i6mr.ipv6mr_interface);
						mStatus err = setsockopt(m->p->permanentsockets.sktv6, IPPROTO_IPV6, IPV6_JOIN_GROUP, &i6mr, sizeof(i6mr));
						// Joining same group twice can give "Address already in use" error -- no need to report that
						if (err < 0 && (errno != EADDRINUSE))
							LogMsg("setsockopt - IPV6_JOIN_GROUP error %d errno %d (%s) group %.16a on %u", err, errno, strerror(errno), &i6mr.ipv6mr_multiaddr, i6mr.ipv6mr_interface);
						}
#endif
					}
				}
			}

	return count;
	}

mDNSlocal void MarkAllInterfacesInactive(mDNS *const m, mDNSs32 utc)
	{
	NetworkInterfaceInfoOSX *i;
	for (i = m->p->InterfaceList; i; i = i->next)
		{
		if (i->Exists) i->LastSeen = utc;
		i->Exists = mDNSfalse;
		}
	}

// returns count of non-link local V4 addresses deregistered
mDNSlocal int ClearInactiveInterfaces(mDNS *const m, mDNSs32 utc)
	{
	// First pass:
	// If an interface is going away, then deregister this from the mDNSCore.
	// We also have to deregister it if the primary interface that it's using for its InterfaceID is going away.
	// We have to do this because mDNSCore will use that InterfaceID when sending packets, and if the memory
	// it refers to has gone away we'll crash.
	NetworkInterfaceInfoOSX *i;
	int count = 0;
	for (i = m->p->InterfaceList; i; i = i->next)
		{
		// If this interface is no longer active, or its InterfaceID is changing, deregister it
		NetworkInterfaceInfoOSX *primary = SearchForInterfaceByName(m, i->ifinfo.ifname, AAAA_OVER_V4 ? AF_UNSPEC : i->sa_family);
		if (i->Registered)
			if (i->Exists == 0 || i->Exists == 2 || i->Registered != primary)
				{
				i->Flashing = !(i->ifa_flags & IFF_LOOPBACK) && (utc - i->AppearanceTime < 60);
				LogInfo("ClearInactiveInterfaces: Deregistering %5s(%lu) %.6a InterfaceID %p(%p), primary %p, %#a/%d%s%s%s",
					i->ifinfo.ifname, i->scope_id, &i->BSSID, i->ifinfo.InterfaceID, i, primary,
					&i->ifinfo.ip, CountMaskBits(&i->ifinfo.mask),
					i->Flashing               ? " (Flashing)"  : "",
					i->Occulting              ? " (Occulting)" : "",
					i->ifinfo.InterfaceActive ? " (Primary)"   : "");

				// Temporary fix to handle P2P flapping. P2P reuses the scope-id, mac address and the IP address
				// everytime it creates a new interface. We think it is a duplicate and hence consider it
				// as flashing and occulting. The "core" does not flush the cache for this case. This leads to
				// stale data returned to the application even after the interface is removed. The application
				// then starts to send data but the new interface is not yet created.
				if (strncmp(i->ifinfo.ifname, "p2p", 3) == 0)
					{
					LogInfo("ClearInactiveInterfaces: P2P %s interface deregistering %s %s", i->ifinfo.ifname,
						i->Flashing               ? " (Flashing)"  : "",
						i->Occulting              ? " (Occulting)" : "");
					mDNS_DeregisterInterface(m, &i->ifinfo, 0);
					}
				else
					{
					mDNS_DeregisterInterface(m, &i->ifinfo, i->Flashing && i->Occulting);
					}
				if (!mDNSAddressIsLinkLocal(&i->ifinfo.ip)) count++;
				i->Registered = mDNSNULL;
				// Note: If i->Registered is set, that means we've called mDNS_RegisterInterface() for this interface,
				// so we need to make sure we call mDNS_DeregisterInterface() before disposing it.
				// If i->Registered is NOT set, then it's not registered and we should not call mDNS_DeregisterInterface() on it.

				// Caution: If we ever decide to add code here to leave the multicast group, we need to make sure that this
				// is the LAST representative of this physical interface, or we'll unsubscribe from the group prematurely.
				}
		}

	// Second pass:
	// Now that everything that's going to deregister has done so, we can clean up and free the memory
	NetworkInterfaceInfoOSX **p = &m->p->InterfaceList;
	while (*p)
		{
		i = *p;
		// If no longer active, delete interface from list and free memory
		if (!i->Exists)
			{
			if (i->LastSeen == utc) i->LastSeen = utc - 1;
			mDNSBool delete = (NumCacheRecordsForInterfaceID(m, i->ifinfo.InterfaceID) == 0) && (utc - i->LastSeen >= 60);
			LogInfo("ClearInactiveInterfaces: %-13s %5s(%lu) %.6a InterfaceID %p(%p) %#a/%d Age %d%s", delete ? "Deleting" : "Holding",
				i->ifinfo.ifname, i->scope_id, &i->BSSID, i->ifinfo.InterfaceID, i,
				&i->ifinfo.ip, CountMaskBits(&i->ifinfo.mask), utc - i->LastSeen,
				i->ifinfo.InterfaceActive ? " (Primary)" : "");
#if APPLE_OSX_mDNSResponder
			if (i->BPF_fd >= 0) CloseBPF(i);
#endif // APPLE_OSX_mDNSResponder
			if (delete)
				{
				*p = i->next;
				freeL("NetworkInterfaceInfoOSX", i);
				continue;	// After deleting this object, don't want to do the "p = &i->next;" thing at the end of the loop
				}
			}
		p = &i->next;
		}
	return count;
	}

mDNSlocal void AppendDNameListElem(DNameListElem ***List, mDNSu32 uid, domainname *name)
	{
	DNameListElem *dnle = (DNameListElem*) mallocL("DNameListElem/AppendDNameListElem", sizeof(DNameListElem));
	if (!dnle) LogMsg("ERROR: AppendDNameListElem: memory exhausted");
	else
		{
		dnle->next = mDNSNULL;
		dnle->uid  = uid;
		AssignDomainName(&dnle->name, name);
		**List = dnle;
		*List = &dnle->next;
		}
	}

mDNSlocal int compare_dns_configs(const void *aa, const void *bb)
	{
	dns_resolver_t *a = *(dns_resolver_t**)aa;
	dns_resolver_t *b = *(dns_resolver_t**)bb;
	
	return (a->search_order < b->search_order) ? -1 : (a->search_order == b->search_order) ? 0 : 1;
	}

// ConfigResolvers is called twice - once to parse the "scoped_resolver" list and second time to parse the "resolver" list.
// "scoped_resolver" has entries that should only be used for "scoped_questions" (for questions that specify an interface index
// q->InterfaceID) and "resolver" entries should only be used for non-scoped questions. Entries in either of the list can specify
// an ifindex. This means that the dns query should be scoped to that interface when sent out on the wire. The flag value
// "DNS_RESOLVER_FLAGS_SCOPED" itself appears only in "scoped" list of resolvers.
//
// Before "scoped_resolver" was introduced, the entries in "resolver" list can contain options like "interface=en0" which
// was meant to scope the query (non-scoped queries) to a specific interface. We still support this option. On top of that,
// we also support a new way of specifying the interface index as described above.
mDNSlocal void ConfigResolvers(mDNS *const m, dns_config_t *config, mDNSBool scope, mDNSBool setsearch, mDNSBool setservers)
	{
	int i, j;
	domainname d;
#if DNSINFO_VERSION >= 20091104
	dns_resolver_t **resolver = scope ? config->scoped_resolver : config->resolver;
	int nresolvers = scope ? config->n_scoped_resolver : config->n_resolver;
#else
	(void) scope; // unused
	dns_resolver_t **resolver = config->resolver;
	int nresolvers = config->n_resolver;
#endif

	if (setsearch && !scope && nresolvers)
		{
		// Due to the vagaries of Apple's SystemConfiguration and dnsinfo.h APIs, if there are no search domains
		// listed, then you're supposed to interpret the "domain" field as also being the search domain, but if
		// there *are* search domains listed, then you're supposed to ignore the "domain" field completely and
		// instead use the search domain list as the sole authority for what domains to search and in what order
		// (and the domain from the "domain" field will also appear somewhere in that list).
		// Also, all search domains get added to the search list for resolver[0], so the domains and/or
		// search lists for other resolvers in the list need to be ignored.
		//
		// Note: Starting DNSINFO_VERSION 20091104, search list is present only in the first resolver (resolver 0).
		// i.e., n_search for the first resolver is always non-zero. We don't guard it with #ifs for better readability

		if (resolver[0]->n_search == 0)
			{
			LogInfo("ConfigResolvers: (%s) configuring zeroth domain as search list %s", scope ? "Scoped" : "Non-scoped", resolver[0]->domain);
			mDNS_AddSearchDomain_CString(resolver[0]->domain, mDNSNULL);
			}
		else
			{
			for (i = 0; i < resolver[0]->n_search; i++)
				{
				LogInfo("ConfigResolvers: (%s) configuring search list %s", scope ? "Scoped" : "Non-scoped", resolver[0]->search[i]);
				mDNS_AddSearchDomain_CString(resolver[0]->search[i], mDNSNULL);
				}
			}
		}

	// scoped search domains are set below. If neither scoped nor setting servers, we have nothing to do
	if (!scope && !setservers) return;

	// For the "default" resolver ("resolver #1") the "domain" value is bogus and we need to ignore it.
	// e.g. the default resolver's "domain" value might say "apple.com", which indicates that this resolver
	// is only for names that fall under "apple.com", but that's not correct. Actually the default resolver is
	// for all names not covered by a more specific resolver (i.e. its domain should be ".", the root domain).
	//
	// Note: Starting DNSINFO_VERSION 20091104, domain value of this first resolver (resolver 0) is always NULL.
	// We don't guard it with #ifs for better readability
	// 
	if ((nresolvers != 0) && resolver[0]->domain)
		resolver[0]->domain[0] = 0; // don't stop pointing at the memory, just change the first byte

	qsort(resolver, nresolvers, sizeof(dns_resolver_t*), compare_dns_configs);
				
	for (i = 0; i < nresolvers; i++)
		{	
		int n;
		dns_resolver_t *r = resolver[i];
		mDNSInterfaceID interface = mDNSInterface_Any;
		int disabled = 0;
					
		LogInfo("ConfigResolvers: %s resolver[%d] domain %s n_nameserver %d", scope ? "Scoped" : "", i, r->domain, r->n_nameserver);

		// On Tiger, dnsinfo entries for mDNS domains have port 5353, the mDNS port.  Ignore them.
		// Note: Unlike the BSD Sockets APIs (where TCP and UDP port numbers are universally in network byte order)
		// in Apple's "dnsinfo.h" API the port number is declared to be a "uint16_t in host byte order"
		// We also don't need to do any more work if there are no nameserver addresses
		if (r->port == 5353 || r->n_nameserver == 0)
			{
			char *opt = r->options;
			if (opt && !strncmp(opt, "mdns", strlen(opt)))
				{
				if (!MakeDomainNameFromDNSNameString(&d, r->domain))
					{ LogMsg("ConfigResolvers: config->resolver[%d] bad domain %s", i, r->domain); continue; }
				mDNS_AddMcastResolver(m, &d, interface, r->timeout);
				}
			continue;
			}

					
		if (!r->domain || !*r->domain) d.c[0] = 0;
		else if (!MakeDomainNameFromDNSNameString(&d, r->domain))
			{ LogMsg("ConfigResolvers: config->resolver[%d] bad domain %s", i, r->domain); continue; }

		// DNS server option parsing
		if (r->options != NULL)
			{
			char *nextOption = r->options;
			char *currentOption = NULL;
			while ((currentOption = strsep(&nextOption, " ")) != NULL && currentOption[0] != 0)
				{
				if (strncmp(currentOption, kInterfaceSpecificOption, sizeof(kInterfaceSpecificOption) - 1) == 0)
					{
					NetworkInterfaceInfoOSX *ni;
					char	ifname[IF_NAMESIZE+1];
					mDNSu32	ifindex = 0;
					// If something goes wrong finding the interface, create the server entry anyhow but mark it as disabled.
					// This allows us to block these special queries from going out on the wire.
					strlcpy(ifname, currentOption + sizeof(kInterfaceSpecificOption)-1, sizeof(ifname));
					ifindex = if_nametoindex(ifname);
					if (ifindex == 0) { disabled = 1; LogMsg("ConfigResolvers: RegisterSplitDNS interface specific - interface %s not found", ifname); continue; }
					LogInfo("ConfigResolvers: interface specific entry: %s on %s (%d)", r->domain, ifname, ifindex);
					// Find the interface. Can't use mDNSPlatformInterfaceIDFromInterfaceIndex
					// because that will call mDNSMacOSXNetworkChanged if the interface doesn't exist
					for (ni = m->p->InterfaceList; ni; ni = ni->next)
						if (ni->ifinfo.InterfaceID && ni->scope_id == ifindex) break;
					if (ni != NULL) interface = ni->ifinfo.InterfaceID;
					if (interface == mDNSNULL)
						{
						disabled = 1;
						LogMsg("ConfigResolvers: RegisterSplitDNS interface specific - index %d (%s) not found", ifindex, ifname);
						continue;
						}
					}
				}
			}

		// flags and if_index are defined only from this DNSINFO_VERSION onwards.
		// Parse the interface index if we have not already parsed one above.
#if DNSINFO_VERSION >= 20091104
		if ((interface == mDNSInterface_Any) && (r->if_index != 0))
			{
			NetworkInterfaceInfoOSX *ni;
			interface = mDNSNULL;
			for (ni = m->p->InterfaceList; ni; ni = ni->next)
				if (ni->ifinfo.InterfaceID && ni->scope_id == r->if_index) break;
			if (ni != NULL) interface = ni->ifinfo.InterfaceID;
			if (interface == mDNSNULL)
				{
				disabled = 1;
				LogMsg("ConfigResolvers: interface specific index %d not found", r->if_index);
				continue;
				}
			}
#endif

		if (setsearch)
			{
			// For non-scoped resolvers unlike scoped resolvers, only zeroth resolver has search lists if any. For scoped
			// resolvers, we need to parse all the entries.
			if (scope)
				{
				for (j = 0; j < resolver[i]->n_search; j++)
					{
					LogInfo("ConfigResolvers: (%s) configuring search list %s, Interface %p", scope ? "Scoped" : "Non-scoped", resolver[i]->search[j], interface);
					mDNS_AddSearchDomain_CString(resolver[i]->search[j], interface);
					}
				// Parse other scoped resolvers for search lists
				if (!setservers) continue;
				}
			}

		for (n = 0; n < r->n_nameserver; n++)
			if (r->nameserver[n]->sa_family == AF_INET || r->nameserver[n]->sa_family == AF_INET6)
				{
				mDNSAddr saddr;
				// mDNSAddr saddr = { mDNSAddrType_IPv4, { { { 192, 168, 1, 1 } } } }; // for testing
				if (SetupAddr(&saddr, r->nameserver[n])) LogMsg("RegisterSplitDNS: bad IP address");
				else
					{
					mDNSBool scopedDNS = mDNSfalse;
					DNSServer *s;
#if DNSINFO_VERSION >= 20091104
					// By setting scoped, this DNSServer can only be picked if the right interfaceID
					// is given in the question. 
					if (scope && (r->flags & DNS_RESOLVER_FLAGS_SCOPED) && (interface == mDNSNULL))
						LogMsg("ConfigResolvers: ERROR: scoped is set but if_index %d is invalid for DNSServer %#a:%d",
							r->if_index, &saddr, mDNSVal16(r->port ? mDNSOpaque16fromIntVal(r->port) : UnicastDNSPort));
					else
						scopedDNS = (scope && (r->flags & DNS_RESOLVER_FLAGS_SCOPED)) ? mDNStrue : mDNSfalse;
#endif
					// The timeout value is for all the DNS servers in a given resolver, hence we pass
					// the timeout value only for the first DNSServer. If we don't have a value in the
					// resolver, then use the core's default value
					//
					// Note: this assumes that when the core picks a list of DNSServers for a question,
					// it takes the sum of all the timeout values for all DNS servers. By doing this, it
					// tries all the DNS servers in a specified timeout
					s = mDNS_AddDNSServer(m, &d, interface, &saddr, r->port ? mDNSOpaque16fromIntVal(r->port) : UnicastDNSPort, scopedDNS,
						(n == 0 ? (r->timeout ? r->timeout : DEFAULT_UDNS_TIMEOUT) : 0));
					if (s)
						{
						if (disabled) s->teststate = DNSServer_Disabled;
						LogInfo("ConfigResolvers: DNS server %#a:%d for domain %##s from slot %d, %d",
							&s->addr, mDNSVal16(s->port), d.c, i, n);
						}
					}
				}
		}
	}

mDNSexport void mDNSPlatformSetDNSConfig(mDNS *const m, mDNSBool setservers, mDNSBool setsearch, domainname *const fqdn, DNameListElem **RegDomains, DNameListElem **BrowseDomains)
	{
	int i;
	char buf[MAX_ESCAPED_DOMAIN_NAME];	// Max legal C-string name, including terminating NUL
	domainname d;

	// Need to set these here because we need to do this even if SCDynamicStoreCreate() or SCDynamicStoreCopyValue() below don't succeed
	if (fqdn)          fqdn->c[0]      = 0;
	if (RegDomains   ) *RegDomains     = NULL;
	if (BrowseDomains) *BrowseDomains  = NULL;

	LogInfo("mDNSPlatformSetDNSConfig:%s%s%s%s%s",
	        setservers    ? " setservers"    : "",
	        setsearch     ? " setsearch"     : "",
	        fqdn          ? " fqdn"          : "",
	        RegDomains    ? " RegDomains"    : "",
	        BrowseDomains ? " BrowseDomains" : "");

	// Add the inferred address-based configuration discovery domains
	// (should really be in core code I think, not platform-specific)
	if (setsearch)
		{
		struct ifaddrs *ifa = mDNSNULL;
		struct sockaddr_in saddr;
		mDNSPlatformMemZero(&saddr, sizeof(saddr));
		saddr.sin_len = sizeof(saddr);
		saddr.sin_family = AF_INET;
		saddr.sin_port = 0;
		saddr.sin_addr.s_addr = *(in_addr_t *)&m->Router.ip.v4;

		// Don't add any reverse-IP search domains if doing the WAB bootstrap queries would cause dial-on-demand connection initiation
		if (!AddrRequiresPPPConnection((struct sockaddr *)&saddr)) ifa =  myGetIfAddrs(1);
		
		while (ifa)
			{
			mDNSAddr a, n;
			if (ifa->ifa_addr->sa_family == AF_INET	&&
				ifa->ifa_netmask                    &&
				!(ifa->ifa_flags & IFF_LOOPBACK)	&&
				!SetupAddr(&a, ifa->ifa_addr)		&&
				!mDNSv4AddressIsLinkLocal(&a.ip.v4)  )
				{
				// Apparently it's normal for the sa_family of an ifa_netmask to sometimes be incorrect, so we explicitly fix it here before calling SetupAddr
				// <rdar://problem/5492035> getifaddrs is returning invalid netmask family for fw0 and vmnet
				ifa->ifa_netmask->sa_family = ifa->ifa_addr->sa_family;		// Make sure ifa_netmask->sa_family is set correctly
				SetupAddr(&n, ifa->ifa_netmask);
				// Note: This is reverse order compared to a normal dotted-decimal IP address, so we can't use our customary "%.4a" format code
				mDNS_snprintf(buf, sizeof(buf), "%d.%d.%d.%d.in-addr.arpa.", a.ip.v4.b[3] & n.ip.v4.b[3],
																			 a.ip.v4.b[2] & n.ip.v4.b[2],
																			 a.ip.v4.b[1] & n.ip.v4.b[1],
																			 a.ip.v4.b[0] & n.ip.v4.b[0]);
				mDNS_AddSearchDomain_CString(buf, mDNSNULL);
				}
			ifa = ifa->ifa_next;
			}
		}

#ifndef MDNS_NO_DNSINFO
	if (setservers || setsearch)
		{
		dns_config_t *config = dns_configuration_copy();
		if (!config)
			{
			// When running on 10.3 (build 7xxx) and earlier, we don't expect dns_configuration_copy() to succeed
			// On 10.4, calls to dns_configuration_copy() early in the boot process often fail.
			// Apparently this is expected behaviour -- "not a bug".
			// Accordingly, we suppress syslog messages for the first three minutes after boot.
			// If we are still getting failures after three minutes, then we log them.
			if ((mDNSu32)mDNSPlatformRawTime() > (mDNSu32)(mDNSPlatformOneSecond * 180))
				LogMsg("mDNSPlatformSetDNSConfig: Error: dns_configuration_copy returned NULL");
			}
		else
			{
			LogInfo("mDNSPlatformSetDNSConfig: config->n_resolver = %d", config->n_resolver);

#if APPLE_OSX_mDNSResponder
				// Record the so-called "primary" domain, which we use as a hint to tell if the user is on a network set up
				// by someone using Microsoft Active Directory using "local" as a private internal top-level domain
				if (config->n_resolver && config->resolver[0]->domain && config->resolver[0]->n_nameserver && config->resolver[0]->nameserver[0])
					MakeDomainNameFromDNSNameString(&ActiveDirectoryPrimaryDomain, config->resolver[0]->domain);
				else ActiveDirectoryPrimaryDomain.c[0] = 0;
				//MakeDomainNameFromDNSNameString(&ActiveDirectoryPrimaryDomain, "test.local");
				ActiveDirectoryPrimaryDomainLabelCount = CountLabels(&ActiveDirectoryPrimaryDomain);
				if (config->n_resolver && config->resolver[0]->n_nameserver && SameDomainName(SkipLeadingLabels(&ActiveDirectoryPrimaryDomain, ActiveDirectoryPrimaryDomainLabelCount - 1), &localdomain))
					SetupAddr(&ActiveDirectoryPrimaryDomainServer, config->resolver[0]->nameserver[0]);
				else
					{
					AssignDomainName(&ActiveDirectoryPrimaryDomain, (const domainname *)"");
					ActiveDirectoryPrimaryDomainLabelCount = 0;
					ActiveDirectoryPrimaryDomainServer = zeroAddr;
					}
#endif

			ConfigResolvers(m, config, mDNSfalse, setsearch, setservers);
#if DNSINFO_VERSION >= 20091104
			ConfigResolvers(m, config, mDNStrue, setsearch, setservers);
#endif
			dns_configuration_free(config);
			if (setsearch) RetrySearchDomainQuestions(m);
			setservers = mDNSfalse;  // Done these now -- no need to fetch the same data from SCDynamicStore
			setsearch  = mDNSfalse;
			}
		}
#endif // MDNS_NO_DNSINFO

	SCDynamicStoreRef store = SCDynamicStoreCreate(NULL, CFSTR("mDNSResponder:mDNSPlatformSetDNSConfig"), NULL, NULL);
	if (!store)
		LogMsg("mDNSPlatformSetDNSConfig: SCDynamicStoreCreate failed: %s", SCErrorString(SCError()));
	else
		{
		CFDictionaryRef ddnsdict = SCDynamicStoreCopyValue(store, NetworkChangedKey_DynamicDNS);
		if (ddnsdict)
			{
			if (fqdn)
				{
				CFArrayRef fqdnArray = CFDictionaryGetValue(ddnsdict, CFSTR("HostNames"));
				if (fqdnArray && CFArrayGetCount(fqdnArray) > 0)
					{
					// for now, we only look at the first array element.  if we ever support multiple configurations, we will walk the list
					CFDictionaryRef fqdnDict = CFArrayGetValueAtIndex(fqdnArray, 0);
					if (fqdnDict && DictionaryIsEnabled(fqdnDict))
						{
						CFStringRef name = CFDictionaryGetValue(fqdnDict, CFSTR("Domain"));
						if (name)
							{
							if (!CFStringGetCString(name, buf, sizeof(buf), kCFStringEncodingUTF8) ||
								!MakeDomainNameFromDNSNameString(fqdn, buf) || !fqdn->c[0])
								LogMsg("GetUserSpecifiedDDNSConfig SCDynamicStore bad DDNS host name: %s", buf[0] ? buf : "(unknown)");
							else debugf("GetUserSpecifiedDDNSConfig SCDynamicStore DDNS host name: %s", buf);
							}
						}
					}
				}

			if (RegDomains)
				{
				CFArrayRef regArray = CFDictionaryGetValue(ddnsdict, CFSTR("RegistrationDomains"));
				if (regArray && CFArrayGetCount(regArray) > 0)
					{
					CFDictionaryRef regDict = CFArrayGetValueAtIndex(regArray, 0);
					if (regDict && DictionaryIsEnabled(regDict))
						{
						CFStringRef name = CFDictionaryGetValue(regDict, CFSTR("Domain"));
						if (name)
							{
							if (!CFStringGetCString(name, buf, sizeof(buf), kCFStringEncodingUTF8) ||
								!MakeDomainNameFromDNSNameString(&d, buf) || !d.c[0])
								LogMsg("GetUserSpecifiedDDNSConfig SCDynamicStore bad DDNS registration domain: %s", buf[0] ? buf : "(unknown)");
							else
								{
								debugf("GetUserSpecifiedDDNSConfig SCDynamicStore DDNS registration domain: %s", buf);
								AppendDNameListElem(&RegDomains, 0, &d);
								}
							}
						}
					}
				}

			if (BrowseDomains)
				{
				CFArrayRef browseArray = CFDictionaryGetValue(ddnsdict, CFSTR("BrowseDomains"));
				if (browseArray)
					{
					for (i = 0; i < CFArrayGetCount(browseArray); i++)
						{
						CFDictionaryRef browseDict = CFArrayGetValueAtIndex(browseArray, i);
						if (browseDict && DictionaryIsEnabled(browseDict))
							{
							CFStringRef name = CFDictionaryGetValue(browseDict, CFSTR("Domain"));
							if (name)
								{
								if (!CFStringGetCString(name, buf, sizeof(buf), kCFStringEncodingUTF8) ||
									!MakeDomainNameFromDNSNameString(&d, buf) || !d.c[0])
									LogMsg("GetUserSpecifiedDDNSConfig SCDynamicStore bad DDNS browsing domain: %s", buf[0] ? buf : "(unknown)");
								else
									{
									debugf("GetUserSpecifiedDDNSConfig SCDynamicStore DDNS browsing domain: %s", buf);
									AppendDNameListElem(&BrowseDomains, 0, &d);
									}
								}
							}
						}
					}
				}
			CFRelease(ddnsdict);
			}

		if (RegDomains)
			{
			CFDictionaryRef btmm = SCDynamicStoreCopyValue(store, NetworkChangedKey_BackToMyMac);
			if (btmm)
				{
				CFIndex size = CFDictionaryGetCount(btmm);
				const void *key[size];
				const void *val[size];
				CFDictionaryGetKeysAndValues(btmm, key, val);
				for (i = 0; i < size; i++)
					{
					LogInfo("BackToMyMac %d", i);
					if (!CFStringGetCString(key[i], buf, sizeof(buf), kCFStringEncodingUTF8))
						LogMsg("Can't read BackToMyMac %d key %s", i, buf);
					else
						{
						mDNSu32 uid = atoi(buf);
						if (!CFStringGetCString(val[i], buf, sizeof(buf), kCFStringEncodingUTF8))
							LogMsg("Can't read BackToMyMac %d val %s", i, buf);
						else if (MakeDomainNameFromDNSNameString(&d, buf) && d.c[0])
							{
							LogInfo("BackToMyMac %d %d %##s", i, uid, d.c);
							AppendDNameListElem(&RegDomains, uid, &d);
							}
						}
					}
				CFRelease(btmm);
				}
			}

		if (setservers || setsearch)
			{
			CFDictionaryRef dict = SCDynamicStoreCopyValue(store, NetworkChangedKey_DNS);
			if (dict)
				{
				if (setservers)
					{
					CFArrayRef values = CFDictionaryGetValue(dict, kSCPropNetDNSServerAddresses);
					if (values)
						{
						LogInfo("DNS Server Address values: %d", (int)CFArrayGetCount(values));
						for (i = 0; i < CFArrayGetCount(values); i++)
							{
							CFStringRef s = CFArrayGetValueAtIndex(values, i);
							mDNSAddr addr = { mDNSAddrType_IPv4, { { { 0 } } } };
							if (s && CFStringGetCString(s, buf, 256, kCFStringEncodingUTF8) &&
								inet_aton(buf, (struct in_addr *) &addr.ip.v4))
								{
								LogInfo("Adding DNS server from dict: %s", buf);
								mDNS_AddDNSServer(m, mDNSNULL, mDNSInterface_Any, &addr, UnicastDNSPort, mDNSfalse, 0);
								}
							}
						}
					else LogInfo("No DNS Server Address values");
					}
				if (setsearch)
					{
					// Add the manual and/or DHCP-dicovered search domains
					CFArrayRef searchDomains = CFDictionaryGetValue(dict, kSCPropNetDNSSearchDomains);
					if (searchDomains)
						{
						for (i = 0; i < CFArrayGetCount(searchDomains); i++)
							{
							CFStringRef s = CFArrayGetValueAtIndex(searchDomains, i);
							if (s && CFStringGetCString(s, buf, sizeof(buf), kCFStringEncodingUTF8))
								mDNS_AddSearchDomain_CString(buf, mDNSNULL);
							}
						}
					else	// No kSCPropNetDNSSearchDomains, so use kSCPropNetDNSDomainName
						{
						// Due to the vagaries of Apple's SystemConfiguration and dnsinfo.h APIs, if there are no search domains
						// listed, then you're supposed to interpret the "domain" field as also being the search domain, but if
						// there *are* search domains listed, then you're supposed to ignore the "domain" field completely and
						// instead use the search domain list as the sole authority for what domains to search and in what order
						// (and the domain from the "domain" field will also appear somewhere in that list).
						CFStringRef string = CFDictionaryGetValue(dict, kSCPropNetDNSDomainName);
						if (string && CFStringGetCString(string, buf, sizeof(buf), kCFStringEncodingUTF8))
							mDNS_AddSearchDomain_CString(buf, mDNSNULL);
						}
					RetrySearchDomainQuestions(m);
					}
				CFRelease(dict);
				}
			}
		CFRelease(store);
		}
	}

mDNSexport mStatus mDNSPlatformGetPrimaryInterface(mDNS *const m, mDNSAddr *v4, mDNSAddr *v6, mDNSAddr *r)
	{
	char				buf[256];
	(void)m; // Unused

	SCDynamicStoreRef store = SCDynamicStoreCreate(NULL, CFSTR("mDNSResponder:mDNSPlatformGetPrimaryInterface"), NULL, NULL);
	if (!store)
		LogMsg("mDNSPlatformGetPrimaryInterface: SCDynamicStoreCreate failed: %s", SCErrorString(SCError()));
	else
		{
		CFDictionaryRef dict = SCDynamicStoreCopyValue(store, NetworkChangedKey_IPv4);
		if (dict)
			{
			r->type  = mDNSAddrType_IPv4;
			r->ip.v4 = zerov4Addr;
			CFStringRef string = CFDictionaryGetValue(dict, kSCPropNetIPv4Router);
			if (string)
				{
				if (!CFStringGetCString(string, buf, 256, kCFStringEncodingUTF8))
					LogMsg("Could not convert router to CString");
				else
					{
					struct sockaddr_in saddr;
					saddr.sin_len = sizeof(saddr);
					saddr.sin_family = AF_INET;
					saddr.sin_port = 0;
					inet_aton(buf, &saddr.sin_addr);
		
					*(in_addr_t *)&r->ip.v4 = saddr.sin_addr.s_addr;
					}
				}
		
			string = CFDictionaryGetValue(dict, kSCDynamicStorePropNetPrimaryInterface);
			if (string)
				{
				mDNSBool HavePrimaryGlobalv6 = mDNSfalse;  // does the primary interface have a global v6 address?
				struct ifaddrs *ifa = myGetIfAddrs(1);
		
				*v4 = *v6 = zeroAddr;
		
				if (!CFStringGetCString(string, buf, 256, kCFStringEncodingUTF8)) { LogMsg("Could not convert router to CString"); goto exit; }
		
				// find primary interface in list
				while (ifa && (mDNSIPv4AddressIsZero(v4->ip.v4) || mDNSv4AddressIsLinkLocal(&v4->ip.v4) || !HavePrimaryGlobalv6))
					{
					mDNSAddr tmp6 = zeroAddr;
					if (!strcmp(buf, ifa->ifa_name))
						{
						if (ifa->ifa_addr->sa_family == AF_INET)
							{
							if (mDNSIPv4AddressIsZero(v4->ip.v4) || mDNSv4AddressIsLinkLocal(&v4->ip.v4)) SetupAddr(v4, ifa->ifa_addr);
							}
						else if (ifa->ifa_addr->sa_family == AF_INET6)
							{
							SetupAddr(&tmp6, ifa->ifa_addr);
							if (tmp6.ip.v6.b[0] >> 5 == 1)   // global prefix: 001
								{ HavePrimaryGlobalv6 = mDNStrue; *v6 = tmp6; }
							}
						}
					else
						{
						// We'll take a V6 address from the non-primary interface if the primary interface doesn't have a global V6 address
						if (!HavePrimaryGlobalv6 && ifa->ifa_addr->sa_family == AF_INET6 && !v6->ip.v6.b[0])
							{
							SetupAddr(&tmp6, ifa->ifa_addr);
							if (tmp6.ip.v6.b[0] >> 5 == 1) *v6 = tmp6;
							}
						}
					ifa = ifa->ifa_next;
					}
		
				// Note that while we advertise v6, we still require v4 (possibly NAT'd, but not link-local) because we must use
				// V4 to communicate w/ our DNS server
				}
		
			exit:
			CFRelease(dict);
			}
		CFRelease(store);
		}
	return mStatus_NoError;
	}

mDNSexport void mDNSPlatformDynDNSHostNameStatusChanged(const domainname *const dname, const mStatus status)
	{
	LogInfo("mDNSPlatformDynDNSHostNameStatusChanged %d %##s", status, dname->c);
	char uname[MAX_ESCAPED_DOMAIN_NAME];	// Max legal C-string name, including terminating NUL
	ConvertDomainNameToCString(dname, uname);

	char *p = uname;
	while (*p)
		{
		*p = tolower(*p);
		if (!(*(p+1)) && *p == '.') *p = 0; // if last character, strip trailing dot
		p++;
		}

	// We need to make a CFDictionary called "State:/Network/DynamicDNS" containing (at present) a single entity.
	// That single entity is a CFDictionary with name "HostNames".
	// The "HostNames" CFDictionary contains a set of name/value pairs, where the each name is the FQDN
	// in question, and the corresponding value is a CFDictionary giving the state for that FQDN.
	// (At present we only support a single FQDN, so this dictionary holds just a single name/value pair.)
	// The CFDictionary for each FQDN holds (at present) a single name/value pair,
	// where the name is "Status" and the value is a CFNumber giving an errror code (with zero meaning success).

	const CFStringRef StateKeys [1] = { CFSTR("HostNames") };
	const CFStringRef HostKeys  [1] = { CFStringCreateWithCString(NULL, uname, kCFStringEncodingUTF8) };
	const CFStringRef StatusKeys[1] = { CFSTR("Status") };
	if (!HostKeys[0]) LogMsg("SetDDNSNameStatus: CFStringCreateWithCString(%s) failed", uname);
	else
		{
		const CFNumberRef StatusVals[1] = { CFNumberCreate(NULL, kCFNumberSInt32Type, &status) };
		if (!StatusVals[0]) LogMsg("SetDDNSNameStatus: CFNumberCreate(%d) failed", status);
		else
			{
			const CFDictionaryRef HostVals[1] = { CFDictionaryCreate(NULL, (void*)StatusKeys, (void*)StatusVals, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks) };
			if (HostVals[0])
				{
				const CFDictionaryRef StateVals[1] = { CFDictionaryCreate(NULL, (void*)HostKeys, (void*)HostVals, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks) };
				if (StateVals[0])
					{
					CFDictionaryRef StateDict = CFDictionaryCreate(NULL, (void*)StateKeys, (void*)StateVals, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
					if (StateDict)
						{
						mDNSDynamicStoreSetConfig(kmDNSDynamicConfig, mDNSNULL, StateDict);
						CFRelease(StateDict);
						}
					CFRelease(StateVals[0]);
					}
				CFRelease(HostVals[0]);
				}
			CFRelease(StatusVals[0]);
			}
		CFRelease(HostKeys[0]);
		}
	}

#if APPLE_OSX_mDNSResponder
#if ! NO_AWACS

// checks whether a domain is present in Setup:/Network/BackToMyMac. Just because there is a key in the
// keychain for a domain, it does not become a valid BTMM domain. If things get inconsistent, this will
// help catch it
mDNSlocal mDNSBool IsBTMMDomain(domainname *d)
	{
	SCDynamicStoreRef store = SCDynamicStoreCreate(NULL, CFSTR("mDNSResponder:IsBTMMDomain"), NULL, NULL);
	if (!store)
		{
		LogMsg("IsBTMMDomain: SCDynamicStoreCreate failed: %s", SCErrorString(SCError()));
		return mDNSfalse;
		}
	CFDictionaryRef btmm = SCDynamicStoreCopyValue(store, NetworkChangedKey_BackToMyMac);
	if (btmm)
		{
		CFIndex size = CFDictionaryGetCount(btmm);
		char buf[MAX_ESCAPED_DOMAIN_NAME];	// Max legal C-string name, including terminating NUL
		const void *key[size];
		const void *val[size];
		domainname dom;
		int i;
		CFDictionaryGetKeysAndValues(btmm, key, val);
		for (i = 0; i < size; i++)
			{
			LogInfo("BackToMyMac %d", i);
			if (!CFStringGetCString(key[i], buf, sizeof(buf), kCFStringEncodingUTF8))
				LogMsg("IsBTMMDomain: ERROR!! Can't read BackToMyMac %d key %s", i, buf);
			else
				{
				mDNSu32 uid = atoi(buf);
				if (!CFStringGetCString(val[i], buf, sizeof(buf), kCFStringEncodingUTF8))
					LogMsg("IsBTMMDomain: Can't read BackToMyMac %d val %s", i, buf);
				else if (MakeDomainNameFromDNSNameString(&dom, buf) && dom.c[0])
					{
					if (SameDomainName(&dom, d))
						{
						LogInfo("IsBTMMDomain: Domain %##s is a btmm domain, uid %u", d->c, uid);
						CFRelease(btmm);
						CFRelease(store);
						return mDNStrue;
						}
					}
				}
			}
		CFRelease(btmm);
		}
	CFRelease(store);
	LogInfo("IsBTMMDomain: Domain %##s not a btmm domain", d->c);
	return mDNSfalse;
	}

// Appends data to the buffer
mDNSlocal int AddOneItem(char *buf, int bufsz, char *data, int *currlen)
	{
	int len;

	len = strlcpy(buf + *currlen, data, bufsz - *currlen);
	if (len >= (bufsz - *currlen))
		{
		// if we have exceeded the space in buf, it has already been NULL terminated
		// and we have nothing more to do. Set currlen to the last byte so that the caller
		// knows to do the right thing
		LogMsg("AddOneItem: Exceeded the max buffer size currlen %d, len %d", *currlen, len);
		*currlen = bufsz - 1;
		return -1;
		}
	else { (*currlen) += len; }

	buf[*currlen] = ',';
	if (*currlen >= bufsz)
		{
		LogMsg("AddOneItem: ERROR!! How can currlen be %d", *currlen);
		*currlen = bufsz - 1;
		buf[*currlen] = 0;
		return -1;
		}
	// if we have filled up the buffer exactly, then there is no more work to do
	if (*currlen == bufsz - 1) { buf[*currlen] = 0; return -1; }
	(*currlen)++;
	return *currlen;
	}

// If we have at least one BTMM domain, then trigger the connection to the relay. If we have no
// BTMM domains, then bring down the connection to the relay.
mDNSlocal void UpdateBTMMRelayConnection(mDNS *const m)
	{
	DomainAuthInfo *BTMMDomain = mDNSNULL;
	DomainAuthInfo *FoundInList;
	static mDNSBool AWACSDConnected = mDNSfalse;
	char AllUsers[1024];	// maximum size of mach message
	char AllPass[1024];  	// maximum size of mach message
	char username[MAX_DOMAIN_LABEL + 1];
	int currulen = 0;
	int currplen = 0;

	// if a domain is being deleted, we want to send a disconnect. If we send a disconnect now,
	// we may not be able to send the dns queries over the relay connection which may be needed
	// for sending the deregistrations. Hence, we need to delay sending the disconnect. But we
	// need to make sure that we send the disconnect before attempting the next connect as the
	// awacs connections are redirected based on usernames.
	//
	// For now we send a disconnect immediately. When we start sending dns queries over the relay
	// connection, we will need to fix this.

	for (FoundInList = m->AuthInfoList; FoundInList; FoundInList = FoundInList->next)
		if (!FoundInList->deltime && FoundInList->AutoTunnel && IsBTMMDomain(&FoundInList->domain))
			{
			// We need the passwd from the first domain.
			BTMMDomain = FoundInList;
			ConvertDomainLabelToCString_unescaped((domainlabel *)BTMMDomain->domain.c, username);
			LogInfo("UpdateBTMMRelayConnection: user %s for domain %##s", username, BTMMDomain->domain.c);
			if (AddOneItem(AllUsers, sizeof(AllUsers), username, &currulen) == -1) break;
			if (AddOneItem(AllPass, sizeof(AllPass), BTMMDomain->b64keydata, &currplen) == -1) break;
			}

	if (BTMMDomain) 
		{
		// In the normal case (where we neither exceed the buffer size nor write bytes that
		// fit exactly into the buffer), currulen/currplen should be a different size than
		// (AllUsers - 1) / (AllPass - 1). In that case, we need to override the "," with a NULL byte.

		if (currulen != (int)(sizeof(AllUsers) - 1)) AllUsers[currulen - 1] = 0;
		if (currplen != (int)(sizeof(AllPass) - 1)) AllPass[currplen - 1] = 0;

		LogInfo("UpdateBTMMRelayConnection: AWS_Connect for user %s", AllUsers);
		AWACS_Connect(AllUsers, AllPass, "hello.connectivity.me.com");
		AWACSDConnected = mDNStrue;
		}
	else
		{
		// Disconnect only if we connected previously
		if (AWACSDConnected)
			{
			LogInfo("UpdateBTMMRelayConnection: AWS_Disconnect");
			AWACS_Disconnect();
			AWACSDConnected = mDNSfalse;
			}
		else LogInfo("UpdateBTMMRelayConnection: Not calling AWS_Disconnect");
		}
	}
#else
mDNSlocal void UpdateBTMMRelayConnection(mDNS *const m)
	{
	(void) m; // Unused
	LogInfo("UpdateBTMMRelayConnection: AWACS connection not started, no AWACS library");
	}
#endif // ! NO_AWACS
#endif // APPLE_OSX_mDNSResponder

// MUST be called holding the lock -- this routine calls SetupLocalAutoTunnelInterface_internal()
mDNSexport void SetDomainSecrets(mDNS *m)
	{
#ifdef NO_SECURITYFRAMEWORK
	(void)m;
	LogMsg("Note: SetDomainSecrets: no keychain support");
#else
	mDNSBool	haveAutoTunnels = mDNSfalse;

	LogInfo("SetDomainSecrets");

	// Rather than immediately deleting all keys now, we mark them for deletion in ten seconds.
	// In the case where the user simultaneously removes their DDNS host name and the key
	// for it, this gives mDNSResponder ten seconds to gracefully delete the name from the
	// server before it loses access to the necessary key. Otherwise, we'd leave orphaned
	// address records behind that we no longer have permission to delete.
	DomainAuthInfo *ptr;
	for (ptr = m->AuthInfoList; ptr; ptr = ptr->next)
		ptr->deltime = NonZeroTime(m->timenow + mDNSPlatformOneSecond*10);

#if APPLE_OSX_mDNSResponder
	{
	// Mark all TunnelClients for deletion
	ClientTunnel *client;
	for (client = m->TunnelClients; client; client = client->next)
		{
		LogInfo("SetDomainSecrets: tunnel to %##s marked for deletion", client->dstname.c);
		client->MarkedForDeletion = mDNStrue;
		}
	}
#endif // APPLE_OSX_mDNSResponder

	// String Array used to write list of private domains to Dynamic Store
	CFMutableArrayRef sa = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	if (!sa) { LogMsg("SetDomainSecrets: CFArrayCreateMutable failed"); return; }
	CFIndex i;
	CFDataRef data = NULL;
	const int itemsPerEntry = 4; // domain name, key name, key value, Name value
	CFArrayRef secrets = NULL;
	int err = mDNSKeychainGetSecrets(&secrets);
	if (err || !secrets)
		LogMsg("SetDomainSecrets: mDNSKeychainGetSecrets failed error %d CFArrayRef %p", err, secrets);
	else
		{
		CFIndex ArrayCount = CFArrayGetCount(secrets);
		// Iterate through the secrets
		for (i = 0; i < ArrayCount; ++i)
			{
			mDNSBool AutoTunnel;
			int j, offset;
			CFArrayRef entry = CFArrayGetValueAtIndex(secrets, i);
			if (CFArrayGetTypeID() != CFGetTypeID(entry) || itemsPerEntry != CFArrayGetCount(entry))
				{ LogMsg("SetDomainSecrets: malformed entry %d, itemsPerEntry %d", i, itemsPerEntry); continue; }
			for (j = 0; j < CFArrayGetCount(entry); ++j)
				if (CFDataGetTypeID() != CFGetTypeID(CFArrayGetValueAtIndex(entry, j)))
					{ LogMsg("SetDomainSecrets: malformed entry item %d", j); continue; }

			// The names have already been vetted by the helper, but checking them again here helps humans and automated tools verify correctness

			// Max legal domainname as C-string, including space for btmmprefix and terminating NUL
			// Get DNS domain this key is for (kmDNSKcWhere)
			char stringbuf[MAX_ESCAPED_DOMAIN_NAME + sizeof(btmmprefix)];
			data = CFArrayGetValueAtIndex(entry, kmDNSKcWhere);
			if (CFDataGetLength(data) >= (int)sizeof(stringbuf))
				{ LogMsg("SetDomainSecrets: Bad kSecServiceItemAttr length %d", CFDataGetLength(data)); continue; }
			CFDataGetBytes(data, CFRangeMake(0, CFDataGetLength(data)), (UInt8 *)stringbuf);
			stringbuf[CFDataGetLength(data)] = '\0';

			AutoTunnel = mDNSfalse;
			offset = 0;
			if (!strncmp(stringbuf, dnsprefix, strlen(dnsprefix)))
				offset = strlen(dnsprefix);
			else if (!strncmp(stringbuf, btmmprefix, strlen(btmmprefix)))
				{
				AutoTunnel = mDNStrue;
				offset = strlen(btmmprefix);
				}
			domainname domain;
			if (!MakeDomainNameFromDNSNameString(&domain, stringbuf + offset)) { LogMsg("SetDomainSecrets: bad key domain %s", stringbuf); continue; }

			// Get key name (kmDNSKcAccount)
			data = CFArrayGetValueAtIndex(entry, kmDNSKcAccount);
			if (CFDataGetLength(data) >= (int)sizeof(stringbuf))
				{ LogMsg("SetDomainSecrets: Bad kSecAccountItemAttr length %d", CFDataGetLength(data)); continue; }
			CFDataGetBytes(data, CFRangeMake(0,CFDataGetLength(data)), (UInt8 *)stringbuf);
			stringbuf[CFDataGetLength(data)] = '\0';

			domainname keyname;
			if (!MakeDomainNameFromDNSNameString(&keyname, stringbuf)) { LogMsg("SetDomainSecrets: bad key name %s", stringbuf); continue; }

			// Get key data (kmDNSKcKey)
			data = CFArrayGetValueAtIndex(entry, kmDNSKcKey);
			if (CFDataGetLength(data) >= (int)sizeof(stringbuf))
				{ LogMsg("SetDomainSecrets: Shared secret too long: %d", CFDataGetLength(data)); continue; }
			CFDataGetBytes(data, CFRangeMake(0, CFDataGetLength(data)), (UInt8 *)stringbuf);
			stringbuf[CFDataGetLength(data)] = '\0';	// mDNS_SetSecretForDomain requires NULL-terminated C string for key

			// Get the Name of the keychain entry (kmDNSKcName) host or host:port
			// The hostname also has the port number and ":". It should take a maximum of 6 bytes.
			char hostbuf[MAX_ESCAPED_DOMAIN_NAME + 6];	// Max legal domainname as C-string, including terminating NUL
			data = CFArrayGetValueAtIndex(entry, kmDNSKcName);
			if (CFDataGetLength(data) >= (int)sizeof(hostbuf))
				{ LogMsg("SetDomainSecrets: Shared secret too long: %d", CFDataGetLength(data)); continue; }
			CFDataGetBytes(data, CFRangeMake(0,CFDataGetLength(data)), (UInt8 *)hostbuf);
			hostbuf[CFDataGetLength(data)] = '\0';

			domainname hostname;
			mDNSIPPort port;
			char *hptr;
			hptr = strchr(hostbuf, ':');
			
			port.NotAnInteger = 0;
			if (hptr)
				{
				mDNSu8 *p;
				mDNSu16 val = 0;

				*hptr++ = '\0'; 
				while(hptr && *hptr != 0)
					{
					if (*hptr < '0' || *hptr > '9')
						{ LogMsg("SetDomainSecrets: Malformed Port number %d, val %d", *hptr, val); val = 0; break;}
					val = val * 10 + *hptr - '0';
					hptr++;
					}
				if (!val) continue;
				p = (mDNSu8 *)&val;
				port.NotAnInteger = p[0] << 8 | p[1];
				}
			// The hostbuf is of the format dsid@hostname:port. We don't care about the dsid. 
			hptr = strchr(hostbuf, '@');
			if (hptr)
				hptr++;
			else
				hptr = hostbuf;
			if (!MakeDomainNameFromDNSNameString(&hostname, hptr)) { LogMsg("SetDomainSecrets: bad host name %s", hptr); continue; }

			DomainAuthInfo *FoundInList;
			for (FoundInList = m->AuthInfoList; FoundInList; FoundInList = FoundInList->next)
				if (SameDomainName(&FoundInList->domain, &domain)) break;

#if APPLE_OSX_mDNSResponder
			if (FoundInList)
				{
				// If any client tunnel destination is in this domain, set deletion flag to false
				ClientTunnel *client;
				for (client = m->TunnelClients; client; client = client->next)
					if (FoundInList == GetAuthInfoForName_internal(m, &client->dstname))
					{
					LogInfo("SetDomainSecrets: tunnel to %##s no longer marked for deletion", client->dstname.c);
					client->MarkedForDeletion = mDNSfalse;
					}
				}

#endif // APPLE_OSX_mDNSResponder

			// Uncomment the line below to view the keys as they're read out of the system keychain
			// DO NOT SHIP CODE THIS WAY OR YOU'LL LEAK SECRET DATA INTO A PUBLICLY READABLE FILE!
			//LogInfo("SetDomainSecrets: domain %##s keyname %##s key %s hostname %##s port %d", &domain.c, &keyname.c, stringbuf, hostname.c, (port.b[0] << 8 | port.b[1]));
			LogInfo("SetDomainSecrets: domain %##s keyname %##s hostname %##s port %d", &domain.c, &keyname.c, hostname.c, (port.b[0] << 8 | port.b[1]));

			// If didn't find desired domain in the list, make a new entry
			ptr = FoundInList;
			if (FoundInList && FoundInList->AutoTunnel && haveAutoTunnels == mDNSfalse) haveAutoTunnels = mDNStrue;
			if (!FoundInList)
				{
				ptr = (DomainAuthInfo*)mallocL("DomainAuthInfo", sizeof(*ptr));
				if (!ptr) { LogMsg("SetDomainSecrets: No memory"); continue; }
				}

			//LogInfo("SetDomainSecrets: %d of %d %##s", i, ArrayCount, &domain);

			// It is an AutoTunnel if the keychains tells us so (with btmm prefix) or if it is a TunnelModeDomain
			if (mDNS_SetSecretForDomain(m, ptr, &domain, &keyname, stringbuf, &hostname, &port, (AutoTunnel ? btmmprefix : (IsTunnelModeDomain(&domain) ? dnsprefix : NULL))) == mStatus_BadParamErr)
				{
				if (!FoundInList) mDNSPlatformMemFree(ptr);		// If we made a new DomainAuthInfo here, and it turned out bad, dispose it immediately
				continue;
				}

#if APPLE_OSX_mDNSResponder
			if (ptr->AutoTunnel) UpdateAutoTunnelDomainStatus(m, ptr);
#endif // APPLE_OSX_mDNSResponder

			ConvertDomainNameToCString(&domain, stringbuf);
			CFStringRef cfs = CFStringCreateWithCString(NULL, stringbuf, kCFStringEncodingUTF8);
			if (cfs) { CFArrayAppendValue(sa, cfs); CFRelease(cfs); }
			}
		CFRelease(secrets);
		}
	mDNSDynamicStoreSetConfig(kmDNSPrivateConfig, mDNSNULL, sa);
	CFRelease(sa);

#if APPLE_OSX_mDNSResponder
		{
		// clean up ClientTunnels
		ClientTunnel **pp = &m->TunnelClients;
		while (*pp)
			{
			if ((*pp)->MarkedForDeletion)
				{
				ClientTunnel *cur = *pp;
				LogInfo("SetDomainSecrets: removing client %p %##s from list", cur, cur->dstname.c);
				if (cur->q.ThisQInterval >= 0) mDNS_StopQuery(m, &cur->q);
				AutoTunnelSetKeys(cur, mDNSfalse);
				*pp = cur->next;
				freeL("ClientTunnel", cur);
				}
			else
				pp = &(*pp)->next;
			}

		DomainAuthInfo *info = m->AuthInfoList;
		while (info)
			{
			if (info->AutoTunnel && info->deltime)
				{
				if (info->AutoTunnelNAT.clientContext)
					{
					// stop the NAT operation
					mDNS_StopNATOperation_internal(m, &info->AutoTunnelNAT);
					if (info->AutoTunnelNAT.clientCallback)
						{
						// Reset port and cleanup the state
						info->AutoTunnelNAT.ExternalAddress = m->ExternalAddress;
						info->AutoTunnelNAT.ExternalPort    = zeroIPPort;
						info->AutoTunnelNAT.RequestedPort    = zeroIPPort;
						info->AutoTunnelNAT.Lifetime        = 0;
						info->AutoTunnelNAT.Result          = mStatus_NoError;
						mDNS_DropLockBeforeCallback(); // Allow client to legally make mDNS API calls from the callback
						AutoTunnelDeleteAuthInfoState(m, info);
						mDNS_ReclaimLockAfterCallback(); // Decrement mDNS_reentrancy to block mDNS API calls again
						}
					info->AutoTunnelNAT.clientContext = mDNSNULL;
					}
				RemoveAutoTunnelDomainStatus(m, info);
				}
			info = info->next;
			}

		if (!haveAutoTunnels && !m->TunnelClients && m->AutoTunnelHostAddrActive)
			{
			// remove interface if no autotunnel servers and no more client tunnels
			LogInfo("SetDomainSecrets: Bringing tunnel interface DOWN");
			m->AutoTunnelHostAddrActive = mDNSfalse;
			(void)mDNSAutoTunnelInterfaceUpDown(kmDNSDown, m->AutoTunnelHostAddr.b);
			mDNSPlatformMemZero(m->AutoTunnelHostAddr.b, sizeof(m->AutoTunnelHostAddr.b));
			}
		
		if (m->AutoTunnelHostAddr.b[0])
			if (TunnelClients(m) || TunnelServers(m))
				SetupLocalAutoTunnelInterface_internal(m, mDNSfalse);

		UpdateAnonymousRacoonConfig(m);		// Determine whether we need racoon to accept incoming connections
		UpdateBTMMRelayConnection(m);
		}
#endif // APPLE_OSX_mDNSResponder

	CheckSuppressUnusableQuestions(m);

#endif /* NO_SECURITYFRAMEWORK */
	}

mDNSlocal void SetLocalDomains(void)
	{
	CFMutableArrayRef sa = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	if (!sa) { LogMsg("SetLocalDomains: CFArrayCreateMutable failed"); return; }

	CFArrayAppendValue(sa, CFSTR("local"));
	CFArrayAppendValue(sa, CFSTR("254.169.in-addr.arpa"));
	CFArrayAppendValue(sa, CFSTR("8.e.f.ip6.arpa"));
	CFArrayAppendValue(sa, CFSTR("9.e.f.ip6.arpa"));
	CFArrayAppendValue(sa, CFSTR("a.e.f.ip6.arpa"));
	CFArrayAppendValue(sa, CFSTR("b.e.f.ip6.arpa"));

	mDNSDynamicStoreSetConfig(kmDNSMulticastConfig, mDNSNULL, sa);
	CFRelease(sa);
	}

mDNSlocal void GetCurrentPMSetting(const CFStringRef name, mDNSs32 *val)
	{
#if USE_IOPMCOPYACTIVEPMPREFERENCES
	CFTypeRef blob = NULL;
	CFStringRef str = NULL;
	CFDictionaryRef odict = NULL;
	CFDictionaryRef idict = NULL;
	CFNumberRef number = NULL;

	blob = IOPSCopyPowerSourcesInfo();
	if (!blob) { LogMsg("GetCurrentPMSetting: IOPSCopyPowerSourcesInfo failed!"); goto end; }

	odict = IOPMCopyActivePMPreferences();
	if (!odict) { LogMsg("GetCurrentPMSetting: IOPMCopyActivePMPreferences failed!"); goto end; }

	str = IOPSGetProvidingPowerSourceType(blob);
	if (!str) { LogMsg("GetCurrentPMSetting: IOPSGetProvidingPowerSourceType failed!"); goto end; }

	idict = CFDictionaryGetValue(odict, str);
	if (!idict)
		{
		char buf[256];
		if (!CFStringGetCString(str, buf, sizeof(buf), kCFStringEncodingUTF8)) buf[0] = 0;
		LogMsg("GetCurrentPMSetting: CFDictionaryGetValue (%s) failed!", buf);
		goto end;
		}
	
	number = CFDictionaryGetValue(idict, name);
	if (!number || CFGetTypeID(number) != CFNumberGetTypeID() || !CFNumberGetValue(number, kCFNumberSInt32Type, val))
		*val = 0;
end:
	if (blob) CFRelease(blob);
	if (odict) CFRelease(odict);

#else

	SCDynamicStoreRef store = SCDynamicStoreCreate(NULL, CFSTR("mDNSResponder:GetCurrentPMSetting"), NULL, NULL);
	if (!store) LogMsg("GetCurrentPMSetting: SCDynamicStoreCreate failed: %s", SCErrorString(SCError()));
	else
		{
		CFDictionaryRef dict = SCDynamicStoreCopyValue(store, NetworkChangedKey_PowerSettings);
		if (!dict) LogSPS("GetCurrentPMSetting: Could not get IOPM CurrentSettings dict");
		else
			{
			CFNumberRef number = CFDictionaryGetValue(dict, name);
			if (!number || CFGetTypeID(number) != CFNumberGetTypeID() || !CFNumberGetValue(number, kCFNumberSInt32Type, val))
				*val = 0;
			CFRelease(dict);
			}
		CFRelease(store);
		}
	
#endif
	}

#if APPLE_OSX_mDNSResponder

static CFMutableDictionaryRef spsStatusDict = NULL;
static const CFStringRef kMetricRef = CFSTR("Metric");

mDNSlocal void SPSStatusPutNumber(CFMutableDictionaryRef dict, const mDNSu8* const ptr, CFStringRef key)
	{
	mDNSu8 tmp = (ptr[0] - '0') * 10 + ptr[1] - '0';
	CFNumberRef num = CFNumberCreate(NULL, kCFNumberSInt8Type, &tmp);
	if (!num)
		LogMsg("SPSStatusPutNumber: Could not create CFNumber");
	else
		{
		CFDictionarySetValue(dict, key, num);
		CFRelease(num);
		}
	}

mDNSlocal CFMutableDictionaryRef SPSCreateDict(const mDNSu8* const ptr)
	{
	CFMutableDictionaryRef dict = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	if (!dict) { LogMsg("SPSCreateDict: Could not create CFDictionary dict"); return dict; }
	
	char buffer[1024];
	buffer[mDNS_snprintf(buffer, sizeof(buffer), "%##s", ptr) - 1] = 0;
	CFStringRef spsname = CFStringCreateWithCString(NULL, buffer, kCFStringEncodingUTF8);
	if (!spsname) { LogMsg("SPSCreateDict: Could not create CFString spsname full"); CFRelease(dict); return NULL; }
	CFDictionarySetValue(dict, CFSTR("FullName"), spsname);
	CFRelease(spsname);

	if (ptr[0] >=  2) SPSStatusPutNumber(dict, ptr + 1, CFSTR("Type"));
	if (ptr[0] >=  5) SPSStatusPutNumber(dict, ptr + 4, CFSTR("Portability"));
	if (ptr[0] >=  8) SPSStatusPutNumber(dict, ptr + 7, CFSTR("MarginalPower"));
	if (ptr[0] >= 11) SPSStatusPutNumber(dict, ptr +10, CFSTR("TotalPower"));

	mDNSu32 tmp = SPSMetric(ptr);
	CFNumberRef num = CFNumberCreate(NULL, kCFNumberSInt32Type, &tmp);
	if (!num)
		LogMsg("SPSCreateDict: Could not create CFNumber");
	else
		{
		CFDictionarySetValue(dict, kMetricRef, num);
		CFRelease(num);
		}

	if (ptr[0] >= 12)
		{
		memcpy(buffer, ptr + 13, ptr[0] - 12);
		buffer[ptr[0] - 12] = 0;
		spsname = CFStringCreateWithCString(NULL, buffer, kCFStringEncodingUTF8);
		if (!spsname) { LogMsg("SPSCreateDict: Could not create CFString spsname"); CFRelease(dict); return NULL; }
		else
			{
			CFDictionarySetValue(dict, CFSTR("PrettyName"), spsname);
			CFRelease(spsname);
			}
		}
	
	return dict;
	}
	
mDNSlocal CFComparisonResult CompareSPSEntries(const void *val1, const void *val2, void *context)
	{
	(void)context;
	return CFNumberCompare((CFNumberRef)CFDictionaryGetValue((CFDictionaryRef)val1, kMetricRef),
						   (CFNumberRef)CFDictionaryGetValue((CFDictionaryRef)val2, kMetricRef),
						   NULL);
	}

mDNSlocal void UpdateSPSStatus(mDNS *const m, DNSQuestion *question, const ResourceRecord *const answer, QC_result AddRecord)
	{
	NetworkInterfaceInfo* info = (NetworkInterfaceInfo*)question->QuestionContext;
	debugf("UpdateSPSStatus: %s %##s %s %s", info->ifname, question->qname.c, AddRecord ? "Add" : "Rmv", answer ? RRDisplayString(m, answer) : "<null>");

	mDNS_Lock(m);
	mDNS_UpdateAllowSleep(m);
	mDNS_Unlock(m);

	if (answer && SPSMetric(answer->rdata->u.name.c) > 999999) return;	// Ignore instances with invalid names

	if (!spsStatusDict)
		{
		spsStatusDict = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
		if (!spsStatusDict) { LogMsg("UpdateSPSStatus: Could not create CFDictionary spsStatusDict"); return; }
		}
	
	CFStringRef ifname = CFStringCreateWithCString(NULL, info->ifname, kCFStringEncodingUTF8);
	if (!ifname) { LogMsg("UpdateSPSStatus: Could not create CFString ifname"); return; }
	
	CFMutableArrayRef array = NULL;
	
	if (!CFDictionaryGetValueIfPresent(spsStatusDict, ifname, (const void**) &array))
		{
		array = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
		if (!array) { LogMsg("UpdateSPSStatus: Could not create CFMutableArray"); CFRelease(ifname); return; }
		CFDictionarySetValue(spsStatusDict, ifname, array);
		CFRelease(array); // let go of our reference, now that the dict has one
		}
	else
		if (!array) { LogMsg("UpdateSPSStatus: Could not get CFMutableArray for %s", info->ifname); CFRelease(ifname); return; }
	
	if (!answer) // special call that means the question has been stopped (because the interface is going away)
		CFArrayRemoveAllValues(array);
	else
		{
		CFMutableDictionaryRef dict = SPSCreateDict(answer->rdata->u.name.c);
		if (!dict) { CFRelease(ifname); return; }
		
		if (AddRecord)
			{
			if (!CFArrayContainsValue(array, CFRangeMake(0, CFArrayGetCount(array)), dict))
				{
				int i=0;
				for (i=0; i<CFArrayGetCount(array); i++)
					if (CompareSPSEntries(CFArrayGetValueAtIndex(array, i), dict, NULL) != kCFCompareLessThan)
						break;
				CFArrayInsertValueAtIndex(array, i, dict);
				}
			else LogMsg("UpdateSPSStatus: %s array already contains %##s", info->ifname, answer->rdata->u.name.c);
			}
		else
			{
			CFIndex i = CFArrayGetFirstIndexOfValue(array, CFRangeMake(0, CFArrayGetCount(array)), dict);
			if (i != -1) CFArrayRemoveValueAtIndex(array, i);
			else LogMsg("UpdateSPSStatus: %s array does not contain %##s", info->ifname, answer->rdata->u.name.c);
			}
			
		CFRelease(dict);
		}

	if (!m->ShutdownTime) mDNSDynamicStoreSetConfig(kmDNSSleepProxyServersState, info->ifname, array);
	
	CFRelease(ifname);
	}

mDNSlocal mDNSs32 GetSystemSleepTimerSetting(void)
	{
	mDNSs32 val = -1;
	SCDynamicStoreRef store = SCDynamicStoreCreate(NULL, CFSTR("mDNSResponder:GetSystemSleepTimerSetting"), NULL, NULL);
	if (!store)
		LogMsg("GetSystemSleepTimerSetting: SCDynamicStoreCreate failed: %s", SCErrorString(SCError()));
	else
		{
		CFDictionaryRef dict = SCDynamicStoreCopyValue(store, NetworkChangedKey_PowerSettings);
		if (dict)
			{
			CFNumberRef number = CFDictionaryGetValue(dict, CFSTR("System Sleep Timer"));
			if (number) CFNumberGetValue(number, kCFNumberSInt32Type, &val);
			CFRelease(dict);
			}
		CFRelease(store);
		}
	return val;
	}

mDNSlocal void SetSPS(mDNS *const m)
	{
	SCPreferencesSynchronize(m->p->SCPrefs);
	CFDictionaryRef dict = SCPreferencesGetValue(m->p->SCPrefs, CFSTR("NAT"));
	mDNSBool natenabled = (dict && (CFGetTypeID(dict) == CFDictionaryGetTypeID()) && DictionaryIsEnabled(dict));
	mDNSu8 sps = natenabled ? mDNSSleepProxyMetric_PrimarySoftware :
		(OfferSleepProxyService && GetSystemSleepTimerSetting() == 0) ? mDNSSleepProxyMetric_IncidentalSoftware : 0;

	// For devices that are not running NAT, but are set to never sleep, we may choose to act
	// as a Sleep Proxy, but only for non-portable Macs (Portability > 35 means nominal weight < 3kg)
	//if (sps > mDNSSleepProxyMetric_PrimarySoftware && SPMetricPortability > 35) sps = 0;

	// If we decide to let laptops act as Sleep Proxy, we should do it only when running on AC power, not on battery

	// For devices that are unable to sleep at all to save power, or save 1W or less by sleeping,
	// it makes sense for them to offer low-priority Sleep Proxy service on the network.
	// We rate such a device as metric 70 ("Incidentally Available Hardware")
	if (SPMetricMarginalPower <= 60 && !sps) sps = mDNSSleepProxyMetric_IncidentalHardware;

	// If the launchd plist specifies an explicit value for the Intent Metric, then use that instead of the
	// computed value (currently 40 "Primary Network Infrastructure Software" or 80 "Incidentally Available Software")
	if (sps && OfferSleepProxyService && OfferSleepProxyService < 100) sps = OfferSleepProxyService;

	mDNSCoreBeSleepProxyServer(m, sps, SPMetricPortability, SPMetricMarginalPower, SPMetricTotalPower);
	}

mDNSlocal void InternetSharingChanged(SCPreferencesRef prefs, SCPreferencesNotification notificationType, void *context)
	{
	(void)prefs;             // Parameter not used
	(void)notificationType;  // Parameter not used
	mDNS *const m = (mDNS *const)context;
	KQueueLock(m);
	mDNS_Lock(m);

	// Tell platform layer to open or close its BPF fds
	if (!m->p->NetworkChanged ||
		m->p->NetworkChanged - NonZeroTime(m->timenow + mDNSPlatformOneSecond * 2) < 0)
		{
		m->p->NetworkChanged = NonZeroTime(m->timenow + mDNSPlatformOneSecond * 2);
		LogInfo("InternetSharingChanged: Set NetworkChanged to %d (%d)", m->p->NetworkChanged - m->timenow, m->p->NetworkChanged);
		}

	mDNS_Unlock(m);
	KQueueUnlock(m, "InternetSharingChanged");
	}

mDNSlocal mStatus WatchForInternetSharingChanges(mDNS *const m)
	{
	SCPreferencesRef SCPrefs = SCPreferencesCreate(NULL, CFSTR("mDNSResponder:WatchForInternetSharingChanges"), CFSTR("com.apple.nat.plist"));
	if (!SCPrefs) { LogMsg("SCPreferencesCreate failed: %s", SCErrorString(SCError())); return(mStatus_NoMemoryErr); }

	SCPreferencesContext context = { 0, m, NULL, NULL, NULL };
	if (!SCPreferencesSetCallback(SCPrefs, InternetSharingChanged, &context))
		{ LogMsg("SCPreferencesSetCallback failed: %s", SCErrorString(SCError())); CFRelease(SCPrefs); return(mStatus_NoMemoryErr); }

#ifdef MDNSRESPONDER_USES_LIB_DISPATCH_AS_PRIMARY_EVENT_LOOP_MECHANISM
	if (!SCPreferencesSetDispatchQueue( SCPrefs, dispatch_get_main_queue()))
		{ LogMsg("SCPreferencesSetDispatchQueue failed: %s", SCErrorString(SCError())); return(mStatus_NoMemoryErr); }
#else
	if (!SCPreferencesScheduleWithRunLoop(SCPrefs, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode))
		{ LogMsg("SCPreferencesScheduleWithRunLoop failed: %s", SCErrorString(SCError())); CFRelease(SCPrefs); return(mStatus_NoMemoryErr); }
#endif

	m->p->SCPrefs = SCPrefs;
	return(mStatus_NoError);
	}

// The definitions below should eventually come from some externally-supplied header file.
// However, since these definitions can't really be changed without breaking binary compatibility,
// they should never change, so in practice it should not be a big problem to have them defined here.

#define mDNS_IOREG_KEY               "mDNS_KEY"
#define mDNS_IOREG_VALUE             "2009-07-30"
#define mDNS_USER_CLIENT_CREATE_TYPE 'mDNS'

enum
	{							// commands from the daemon to the driver
    cmd_mDNSOffloadRR = 21,		// give the mdns update buffer to the driver
	};

typedef union { void *ptr; mDNSOpaque64 sixtyfourbits; } FatPtr;

typedef struct
	{                                   // cmd_mDNSOffloadRR structure
    uint32_t  command;                // set to OffloadRR
    uint32_t  rrBufferSize;           // number of bytes of RR records
    uint32_t  numUDPPorts;            // number of SRV UDP ports
    uint32_t  numTCPPorts;            // number of SRV TCP ports 
    uint32_t  numRRRecords;           // number of RR records
    uint32_t  compression;            // rrRecords - compression is base for compressed strings
    FatPtr    rrRecords;              // address of array of pointers to the rr records
    FatPtr    udpPorts;               // address of udp port list (SRV)
    FatPtr    tcpPorts;               // address of tcp port list (SRV)
	} mDNSOffloadCmd;

#include <IOKit/IOKitLib.h>
#include <dns_util.h>

mDNSlocal mDNSu16 GetPortArray(mDNS *const m, int trans, mDNSIPPort *portarray)
	{
	const domainlabel *const tp = (trans == mDNSTransport_UDP) ? (const domainlabel *)"\x4_udp" : (const domainlabel *)"\x4_tcp";
	int count = 0;
	AuthRecord *rr;
	for (rr = m->ResourceRecords; rr; rr=rr->next)
		if (rr->resrec.rrtype == kDNSType_SRV && SameDomainLabel(ThirdLabel(rr->resrec.name)->c, tp->c))
			{
			if (portarray) portarray[count] = rr->resrec.rdata->u.srv.port;
			count++;
			}

	// If Back to My Mac is on, also wake for packets to the IPSEC UDP port (4500)
	if (trans == mDNSTransport_UDP && TunnelServers(m))
		{
		LogSPS("GetPortArray Back to My Mac at %d", count);
		if (portarray) portarray[count] = IPSECPort;
		count++;
		}
	return(count);
	}

#define TfrRecordToNIC(RR) \
	((!(RR)->resrec.InterfaceID && ((RR)->ForceMCast || IsLocalDomain((RR)->resrec.name))))

mDNSlocal mDNSu32 CountProxyRecords(mDNS *const m, uint32_t *const numbytes)
	{
	*numbytes = 0;
	int count = 0;
	AuthRecord *rr;
	for (rr = m->ResourceRecords; rr; rr=rr->next)
		if (rr->resrec.RecordType > kDNSRecordTypeDeregistering)
			if (TfrRecordToNIC(rr))
				{
				*numbytes += DomainNameLength(rr->resrec.name) + 10 + rr->resrec.rdestimate;
				LogSPS("CountProxyRecords: %3d size %5d total %5d %s",
					count, DomainNameLength(rr->resrec.name) + 10 + rr->resrec.rdestimate, *numbytes, ARDisplayString(m,rr));
				count++;
				}
	return(count);
	}

mDNSlocal void GetProxyRecords(mDNS *const m, DNSMessage *const msg, uint32_t *const numbytes, FatPtr *const records)
	{
	mDNSu8 *p = msg->data;
	const mDNSu8 *const limit = p + *numbytes;
	InitializeDNSMessage(&msg->h, zeroID, zeroID);

	int count = 0;
	AuthRecord *rr;
	for (rr = m->ResourceRecords; rr; rr=rr->next)
		if (rr->resrec.RecordType > kDNSRecordTypeDeregistering)
			if (TfrRecordToNIC(rr))
				{
				records[count].sixtyfourbits = zeroOpaque64;
				records[count].ptr = p;
				if (rr->resrec.RecordType & kDNSRecordTypeUniqueMask)
					rr->resrec.rrclass |= kDNSClass_UniqueRRSet;	// Temporarily set the 'unique' bit so PutResourceRecord will set it
				p = PutResourceRecordTTLWithLimit(msg, p, &msg->h.mDNS_numUpdates, &rr->resrec, rr->resrec.rroriginalttl, limit);
				rr->resrec.rrclass &= ~kDNSClass_UniqueRRSet;		// Make sure to clear 'unique' bit back to normal state
				LogSPS("GetProxyRecords: %3d start %p end %p size %5d total %5d %s",
					count, records[count].ptr, p, p - (mDNSu8 *)records[count].ptr, p - msg->data, ARDisplayString(m,rr));
				count++;
				}
	*numbytes = p - msg->data;
	}

// If compiling with old headers and libraries (pre 10.5) that don't include IOConnectCallStructMethod
// then we declare a dummy version here so that the code at least compiles
#ifndef AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER
static kern_return_t
IOConnectCallStructMethod(
	mach_port_t	 connection,		// In
	uint32_t	 selector,			// In
	const void	*inputStruct,		// In
	size_t		 inputStructCnt,	// In
	void		*outputStruct,		// Out
	size_t		*outputStructCnt)	// In/Out
	{
	(void)connection;
	(void)selector;
	(void)inputStruct;
	(void)inputStructCnt;
	(void)outputStruct;
	(void)outputStructCnt;
	LogMsg("Compiled without IOConnectCallStructMethod");
	return(KERN_FAILURE);
	}
#endif

mDNSexport mStatus ActivateLocalProxy(mDNS *const m, char *ifname)	// Called with the lock held
	{
	mStatus result = mStatus_UnknownErr;
	io_service_t service = IOServiceGetMatchingService(kIOMasterPortDefault, IOBSDNameMatching(kIOMasterPortDefault, 0, ifname));
	if (!service) { LogMsg("ActivateLocalProxy: No service for interface %s", ifname); return(mStatus_UnknownErr); }

	io_name_t n1, n2;
	IOObjectGetClass(service, n1);
	io_object_t parent;
	kern_return_t kr = IORegistryEntryGetParentEntry(service, kIOServicePlane, &parent);
	if (kr != KERN_SUCCESS) LogMsg("ActivateLocalProxy: IORegistryEntryGetParentEntry for %s/%s failed %d", ifname, n1, kr);
	else
		{
		IOObjectGetClass(parent, n2);
		LogSPS("ActivateLocalProxy: Interface %s service %s parent %s", ifname, n1, n2);
		const CFTypeRef ref = IORegistryEntryCreateCFProperty(parent, CFSTR(mDNS_IOREG_KEY), kCFAllocatorDefault, mDNSNULL);
		if (!ref) LogSPS("ActivateLocalProxy: No mDNS_IOREG_KEY for interface %s/%s/%s", ifname, n1, n2);
		else
			{
			if (CFGetTypeID(ref) != CFStringGetTypeID() || !CFEqual(ref, CFSTR(mDNS_IOREG_VALUE)))
				LogMsg("ActivateLocalProxy: mDNS_IOREG_KEY for interface %s/%s/%s value %s != %s",
					ifname, n1, n2, CFStringGetCStringPtr(ref, mDNSNULL), mDNS_IOREG_VALUE);
			else if (!UseInternalSleepProxy)
				LogSPS("ActivateLocalProxy: Not using internal (NIC) sleep proxy for interface %s", ifname);
			else
				{
				io_connect_t conObj;
				kr = IOServiceOpen(parent, mach_task_self(), mDNS_USER_CLIENT_CREATE_TYPE, &conObj);
				if (kr != KERN_SUCCESS) LogMsg("ActivateLocalProxy: IOServiceOpen for %s/%s/%s failed %d", ifname, n1, n2, kr);
				else
					{
					mDNSOffloadCmd cmd;
					mDNSPlatformMemZero(&cmd, sizeof(cmd)); // When compiling 32-bit, make sure top 32 bits of 64-bit pointers get initialized to zero
					cmd.command       = cmd_mDNSOffloadRR;
					cmd.numUDPPorts   = GetPortArray(m, mDNSTransport_UDP, mDNSNULL);
					cmd.numTCPPorts   = GetPortArray(m, mDNSTransport_TCP, mDNSNULL);
					cmd.numRRRecords  = CountProxyRecords(m, &cmd.rrBufferSize);
					cmd.compression   = sizeof(DNSMessageHeader);

					DNSMessage *msg = (DNSMessage *)mallocL("mDNSOffloadCmd msg", sizeof(DNSMessageHeader) + cmd.rrBufferSize);
					cmd.rrRecords.ptr = mallocL("mDNSOffloadCmd rrRecords", cmd.numRRRecords * sizeof(FatPtr));
					cmd.udpPorts .ptr = mallocL("mDNSOffloadCmd udpPorts",  cmd.numUDPPorts  * sizeof(mDNSIPPort));
					cmd.tcpPorts .ptr = mallocL("mDNSOffloadCmd tcpPorts",  cmd.numTCPPorts  * sizeof(mDNSIPPort));

					LogSPS("ActivateLocalProxy: msg %p %d RR %p %d, UDP %p %d, TCP %p %d",
						msg, cmd.rrBufferSize,
						cmd.rrRecords.ptr, cmd.numRRRecords,
						cmd.udpPorts .ptr, cmd.numUDPPorts,
						cmd.tcpPorts .ptr, cmd.numTCPPorts);

					if (!msg || !cmd.rrRecords.ptr || !cmd.udpPorts.ptr || !cmd.tcpPorts.ptr)
						LogMsg("ActivateLocalProxy: Failed to allocate memory: msg %p %d RR %p %d, UDP %p %d, TCP %p %d",
						msg, cmd.rrBufferSize,
						cmd.rrRecords.ptr, cmd.numRRRecords,
						cmd.udpPorts .ptr, cmd.numUDPPorts,
						cmd.tcpPorts .ptr, cmd.numTCPPorts);
					else
						{
						GetProxyRecords(m, msg, &cmd.rrBufferSize, cmd.rrRecords.ptr);
						GetPortArray(m, mDNSTransport_UDP, cmd.udpPorts.ptr);
						GetPortArray(m, mDNSTransport_TCP, cmd.tcpPorts.ptr);
						char outputData[2];
						size_t outputDataSize = sizeof(outputData);
						kr = IOConnectCallStructMethod(conObj, 0, &cmd, sizeof(cmd), outputData, &outputDataSize);
						LogSPS("ActivateLocalProxy: IOConnectCallStructMethod for %s/%s/%s %d", ifname, n1, n2, kr);
						if (kr == KERN_SUCCESS) result = mStatus_NoError;
						}

				 	if (cmd.tcpPorts. ptr) freeL("mDNSOffloadCmd udpPorts",  cmd.tcpPorts .ptr);
					if (cmd.udpPorts. ptr) freeL("mDNSOffloadCmd tcpPorts",  cmd.udpPorts .ptr);
					if (cmd.rrRecords.ptr) freeL("mDNSOffloadCmd rrRecords", cmd.rrRecords.ptr);
					if (msg)               freeL("mDNSOffloadCmd msg",       msg);
					IOServiceClose(conObj);
					}
				}
			CFRelease(ref);
			}
		IOObjectRelease(parent);
		}
	IOObjectRelease(service);
	return result;
	}

#endif // APPLE_OSX_mDNSResponder

static io_service_t g_rootdomain = MACH_PORT_NULL;

mDNSlocal mDNSBool SystemWakeForNetworkAccess(void)
	{
	mDNSs32 val = 0;
	CFBooleanRef clamshellStop = NULL;
	mDNSBool retnow = mDNSfalse;

	if (DisableSleepProxyClient) { LogSPS("SystemWakeForNetworkAccess: Sleep Proxy Client disabled by command-line option"); return mDNSfalse; }

	GetCurrentPMSetting(CFSTR("Wake On LAN"), &val);
	LogSPS("SystemWakeForNetworkAccess: Wake On LAN: %d", val);
	if (!val) return mDNSfalse;
	
	if (!g_rootdomain) g_rootdomain = IORegistryEntryFromPath(MACH_PORT_NULL, kIOPowerPlane ":/IOPowerConnection/IOPMrootDomain");
	if (!g_rootdomain) { LogMsg("SystemWakeForNetworkAccess: IORegistryEntryFromPath failed; assuming no clamshell so can WOMP"); return mDNStrue; }
	
	clamshellStop = (CFBooleanRef)IORegistryEntryCreateCFProperty(g_rootdomain, CFSTR(kAppleClamshellStateKey), kCFAllocatorDefault, 0);
	if (!clamshellStop) { LogSPS("SystemWakeForNetworkAccess: kAppleClamshellStateKey does not exist; assuming no clamshell so can WOMP"); return mDNStrue; }
	retnow = clamshellStop == kCFBooleanFalse;
	CFRelease(clamshellStop);
	if (retnow) { LogSPS("SystemWakeForNetworkAccess: kAppleClamshellStateKey is false; clamshell is open so can WOMP"); return mDNStrue; }
	
	clamshellStop = (CFBooleanRef)IORegistryEntryCreateCFProperty(g_rootdomain, CFSTR(kAppleClamshellCausesSleepKey), kCFAllocatorDefault, 0);
	if (!clamshellStop) { LogSPS("SystemWakeForNetworkAccess: kAppleClamshellCausesSleepKey does not exist; assuming no clamshell so can WOMP"); return mDNStrue; }
	retnow = (clamshellStop == kCFBooleanFalse);
	CFRelease(clamshellStop);
	if (retnow) { LogSPS("SystemWakeForNetworkAccess: kAppleClamshellCausesSleepKey is false; clamshell is closed but can WOMP"); return mDNStrue; }
	
	LogSPS("SystemWakeForNetworkAccess: clamshell is closed and can't WOMP");
	return mDNSfalse;
	}

mDNSlocal mDNSBool SystemSleepOnlyIfWakeOnLAN(void)
	{
	mDNSs32 val = 0;
	GetCurrentPMSetting(CFSTR("PrioritizeNetworkReachabilityOverSleep"), &val);
	return val != 0 ? mDNStrue : mDNSfalse;
	}

#if APPLE_OSX_mDNSResponder
// If the _autotunnel6 record is still there in the list, we are waiting for the ack from
// the server.
//
// If we are behind a double-NAT or NAT with no NAT-PMP support, we should make sure that all our
// BTMM records are deregistered so that it does not appear on the Finder sidebar of our peers
// when we go to sleep. First _autotunnel6 and the host record gets deregistered, then SRV
// (UpdateAllSrvRecords) and then PTR and TXT
//
// Note: We wait up to a maximum of 10 seconds before we ack the sleep. So, returning "false"
// here does not necessarily mean that it will be honored.
mDNSexport mDNSBool RecordReadyForSleep(mDNS *const m, AuthRecord *rr)
	{
	if (!AuthRecord_uDNS(rr)) return mDNStrue;

	if (SameDomainLabel(rr->namestorage.c, (const mDNSu8 *)"\x0c_autotunnel6"))
		{
		LogInfo("RecordReadyForSleep: %s not ready for sleep", ARDisplayString(m, rr));
		return mDNSfalse;
		}
	// Just check for the SRV record alone as the PTR and TXT records are dependent on SRV
	// and will get deregistered together in a single update. We also don't check for TXT
	// records  as _kerberos TXT record is always there even when there are no services
	// and we don't want to delay the sleep in that case.
	if (mDNSIPPortIsZero(m->LLQNAT.ExternalPort) || m->LLQNAT.Result)
		{
		if ((rr->resrec.rrtype == kDNSType_SRV) && rr->state != regState_NoTarget && rr->zone)
			{
			DomainAuthInfo *info = GetAuthInfoForName_internal(m, rr->zone);
			if (info && info->AutoTunnel)
				{	
				LogInfo("RecordReadyForSleep: %s not ready for sleep", ARDisplayString(m, rr));
				return mDNSfalse;
				}
			}
		}
	return mDNStrue;
	}

// Note: BTMMDict needs to be retained by the caller if needed
mDNSlocal CFDictionaryRef ParseBackToMyMacKey(CFDictionaryRef connd)
	{
	CFDictionaryRef BTMMDict = CFDictionaryGetValue(connd, CFSTR("BackToMyMac"));
	if (!BTMMDict)
		{
		LogInfo("ParseBackToMyMacKey: CFDictionaryGetValue No value for BackToMyMac");
		return NULL;
		}

	// Non-dictionary is treated as non-existent dictionary
	if (CFGetTypeID(BTMMDict) != CFDictionaryGetTypeID())
		{LogMsg("ERROR: ParseBackToMyMacKey: CFDictionaryGetValue BackToMyMac not a dictionary"); CFRelease(BTMMDict); return NULL;}

	return BTMMDict;
	}

mDNSlocal void ParseBTMMInterfaceKey(CFDictionaryRef BTMMDict, char *buf, int buflen)
	{
	CFTypeRef string;
	mDNSBool ifExists;

	ifExists = CFDictionaryGetValueIfPresent(BTMMDict, CFSTR("Interface"), &string);
	if (ifExists)
		{
		if (!CFStringGetCString(string, buf, buflen, kCFStringEncodingUTF8))
			{
			LogMsg("ERROR: ParseBTMMInterfaceKey: Could not convert Interface to CString");
			if (buflen) buf[0] = 0;
			return;
			}
		else
			debugf("ParseBTMMInterfaceKey: Interface Key exists %s", buf);
		}
	else
		{
		if (buflen) buf[0] = 0;
		debugf("ParseBTMMInterfaceKey: Interface Key does not exist");
		}
	}

mDNSexport void RemoveAutoTunnel6Record(mDNS *const m)
	{
	DomainAuthInfo *info;
	char buf[IFNAMSIZ];

	// Did we parse a non-empty dictionary before ?
	if (!m->p->ConndBTMMDict || (CFDictionaryGetCount(m->p->ConndBTMMDict) == 0))
		{
		LogInfo("RemoveAutoTunnel6Record: Never registered any records before, not deregistering %p", m->p->ConndBTMMDict);
		return;
		}

	// Did we have a non-NULL Interface name before ?
	ParseBTMMInterfaceKey(m->p->ConndBTMMDict, buf, sizeof(buf));
	if (!strlen(buf))
		{
		LogInfo("RemoveAutoTunnel6Record: Interface name already NULL, not deregistering");
		return;
		}
		
	// Set the address to zero before calling DeregisterAutoTunnel6Record. If we call
	// Deregister too quickly before the previous Register completed (just scheduled
	// to be sent out) and when DeregisterAutoTunnel6Record calls mDNS_Register_internal,
	// it invokes the AutoTunnelRecordCallback immediately and AutoTunnelRelayAddrIn should
	// be zero so that we don't register again.
	m->AutoTunnelRelayAddrIn = zerov6Addr;
	if (!m->AuthInfoList) LogInfo("RemoveAutoTunnel6Record: No Domain AuthInfo");
	for (info = m->AuthInfoList; info; info = info->next)
		{
		if (!info->AutoTunnel) { LogInfo("RemoveAutoTunnel6Record: Domain %##s not an AutoTunnel", info->domain.c); continue;}

		if (info->deltime) {LogInfo("RemoveAutoTunnel6Record: Domain %##s about to be deleted", info->domain.c); continue;}

		LogInfo("RemoveAutoTunnel6Record: Deregistering records for domain %##s", info->domain.c);
		DeregisterAutoTunnel6Record(m, info);
		}
	CFRelease(m->p->ConndBTMMDict);
	m->p->ConndBTMMDict = NULL;
	}

// Returns zero on success
mDNSlocal int GetIPv6AddressForIfname(char *ifname, mDNSv6Addr *ipv6Addr)
	{
	struct ifaddrs  *ifa;
	struct ifaddrs  *ifaddrs;
	mDNSAddr addr;

	if (if_nametoindex(ifname) == 0) {LogInfo("GetIPv6AddressForIfname: Invalid name %s", ifname); return (-1);}

	if (getifaddrs(&ifaddrs) < 0) {LogInfo("GetIPv6AddressForIfname: getifaddrs failed"); return (-1);}

	/*
	 * Find the ifaddr entry corresponding to the interface name,
	 * and return the first matching non-linklocal IPv6 address.
	 */
	for (ifa = ifaddrs; ifa != NULL; ifa = ifa->ifa_next)
		{
		if (strncmp(ifa->ifa_name, ifname, IFNAMSIZ) != 0)
			continue;
		if (ifa->ifa_flags & IFF_UP && ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET6)
			{
			struct sockaddr_in6 *ifa_addr = (struct sockaddr_in6 *)ifa->ifa_addr;
			if (IN6_IS_ADDR_LINKLOCAL(&ifa_addr->sin6_addr))
				continue;
			if (SetupAddr(&addr, ifa->ifa_addr) != mStatus_NoError)
				{
				LogInfo("GetIPv6AddressForIfname: SetupAddr error, continuing to the next address");
				continue;
				}
			else
				{
				*ipv6Addr = *(mDNSv6Addr *)&addr.ip.v6;
				LogInfo("GetIPv6AddressForIfname: Returning IPv6 address %.16a", ipv6Addr);
				freeifaddrs(ifaddrs);
				return 0;
				}
			}
       }
	LogInfo("GetIPv6AddressForIfname: No Valid IPv6 address");
	freeifaddrs(ifaddrs);
	return (-1);
	}

mDNSlocal void AddAutoTunnel6Record(mDNS *const m, char *ifname, CFDictionaryRef BTMMDict)
	{
	mDNSv6Addr v6addr;
	DomainAuthInfo *info;

	if (GetIPv6AddressForIfname(ifname, &v6addr) != 0)
		{
		LogInfo("AddAutoTunnel6Record: No Valid IPv6 addresses found for %s", ifname);
		// If the interface does not exist but the dictionary has the value, we treat
		// this case as though the dictionary does not have the value
		RemoveAutoTunnel6Record(m);
		// If awacsd crashes or exits for some reason, restart the relay connection
		UpdateBTMMRelayConnection(m);
		return;
		}
	
	m->AutoTunnelRelayAddrOut = v6addr;

	// if disabled administratively, don't bother to register. RegisterAutoTunnel6Record makes these same
	// checks, but we do it here not just as an optimization but mainly to keep AutoTunnelRelayAddrIn zero
	// as a non-zero AutoTunnelRelayAddrIn indicates that we have registered _autotunnel6 record and hence
	// other hosts can connect to this host through the relay
	if (!m->RegisterAutoTunnel6 || DisableInboundRelayConnection)
		{
		LogInfo("RegisterAutoTunnel6Record: registration Disabled RegisterAutoTunnel6 %d, DisableInbound %d",
			m->RegisterAutoTunnel6, DisableInboundRelayConnection);
		return;
		}
	m->AutoTunnelRelayAddrIn = v6addr;

	if (!m->AuthInfoList) LogInfo("AddAutoTunnel6Record: No Domain AuthInfo");
	for (info = m->AuthInfoList; info; info = info->next)
		{
		// clientContext for a domain tells us that we are listening for at least one Service/Record
		// in a domain and SetLocalAutoTunnelInterface_internal was called
		if (!info->AutoTunnel) { LogInfo("AddAutoTunnel6Record: Domain %##s not an AutoTunnel", info->domain.c); continue;}

		if (!info->AutoTunnelNAT.clientContext) {LogInfo("AddAutoTunnel6Record: Domain %##s has no services", info->domain.c); continue;}

		if (info->deltime) {LogInfo("AddAutoTunnel6Record: Domain %##s about to be deleted", info->domain.c); continue;}

		LogInfo("AddAutoTunnel6Record: Registering records for domain %##s", info->domain.c);
		mDNS_Lock(m);
		RegisterAutoTunnel6Record(m, info);
		mDNS_Unlock(m);
		}
	if (m->p->ConndBTMMDict) CFRelease(m->p->ConndBTMMDict);
	m->p->ConndBTMMDict = CFRetain(BTMMDict);
	}

mDNSlocal void ParseBackToMyMac(mDNS *const m, CFDictionaryRef connd)
	{
	CFDictionaryRef BTMMDict;
	char buf[IFNAMSIZ];

	BTMMDict = ParseBackToMyMacKey(connd);
	if (!BTMMDict)
		{
		LogInfo("ParseBackToMyMac: CFDictionaryGetValue No value for BackToMyMac, Removing autotunnel6");
		RemoveAutoTunnel6Record(m);
		// Note: AutoTunnelRelayAddrIn is zeroed out in RemoveAutoTunnel6Record as it is called
		// from other places.
		m->AutoTunnelRelayAddrOut = zerov6Addr;
		mDNS_Lock(m);
		UpdateAutoTunnelDomainStatuses(m);
		mDNS_Unlock(m);
		return;
		}

	ParseBTMMInterfaceKey(BTMMDict, buf, sizeof(buf));
	if (!strlen(buf))
		{
		LogInfo("ParseBackToMyMac: NULL value for Interface, Removing autotunnel6"); 
		RemoveAutoTunnel6Record(m);
		m->AutoTunnelRelayAddrOut = zerov6Addr;
		// We don't have a utun interface, start the relay connection if possible
		UpdateBTMMRelayConnection(m);
		}
	else
		{
		LogInfo("ParseBackToMyMac: non-NULL value for Interface, Adding autotunnel6"); 
		AddAutoTunnel6Record(m, buf, BTMMDict);
		}

	mDNS_Lock(m);
	UpdateAutoTunnelDomainStatuses(m);
	mDNS_Unlock(m);
	}

mDNSlocal void SetupConndConfigChanges(mDNS *const m)
	{
	CFDictionaryRef connd;
	SCDynamicStoreRef store;

	store = SCDynamicStoreCreate(NULL, CFSTR("mDNSResponder:SetupConndConfigChanges"), NULL, NULL);
	if (!store) {LogMsg("SetupConndConfigChanges: SCDynamicStoreCreate failed: %s", SCErrorString(SCError())); return;}

	connd = SCDynamicStoreCopyValue(store, NetworkChangedKey_BTMMConnectivity);
	if (!connd)
		{LogInfo("SetupConndConfigChanges: SCDynamicStoreCopyValue failed: %s", SCErrorString(SCError())); CFRelease(store); return;}
	else
		{
		ParseBackToMyMac(m, connd);
		}
	CFRelease(connd);
	CFRelease(store);
	}
#endif /* APPLE_OSX_mDNSResponder */

		
mDNSexport void mDNSMacOSXNetworkChanged(mDNS *const m)
	{
	LogInfo("***   Network Configuration Change   ***  (%d)%s",
		m->p->NetworkChanged ? mDNS_TimeNow(m) - m->p->NetworkChanged : 0,
		m->p->NetworkChanged ? "" : " (no scheduled configuration change)");
	m->p->NetworkChanged = 0;		// If we received a network change event and deferred processing, we're now dealing with it
	mDNSs32 utc = mDNSPlatformUTC();
	m->SystemWakeOnLANEnabled = SystemWakeForNetworkAccess();
	m->SystemSleepOnlyIfWakeOnLAN = SystemSleepOnlyIfWakeOnLAN();
	MarkAllInterfacesInactive(m, utc);
	UpdateInterfaceList(m, utc);
	ClearInactiveInterfaces(m, utc);
	SetupActiveInterfaces(m, utc);

#if APPLE_OSX_mDNSResponder

	SetupConndConfigChanges(m);

	if (m->AutoTunnelHostAddr.b[0])
		{
		mDNS_Lock(m);
		if (TunnelClients(m) || TunnelServers(m))
			SetupLocalAutoTunnelInterface_internal(m, mDNSfalse);
		mDNS_Unlock(m);
		}
	
	// Scan to find client tunnels whose questions have completed,
	// but whose local inner/outer addresses have changed since the tunnel was set up
	ClientTunnel *p;
	for (p = m->TunnelClients; p; p = p->next)
		if (p->q.ThisQInterval < 0)
			{
			if (!mDNSIPPortIsZero(p->rmt_outer_port))
				{
				mDNSAddr tmpSrc = zeroAddr;
				mDNSAddr tmpDst = { mDNSAddrType_IPv4, {{{0}}} };
				tmpDst.ip.v4 = p->rmt_outer;
				mDNSPlatformSourceAddrForDest(&tmpSrc, &tmpDst);
				if (!mDNSSameIPv6Address(p->loc_inner, m->AutoTunnelHostAddr) ||
					!mDNSSameIPv4Address(p->loc_outer, tmpSrc.ip.v4))
					{
					AutoTunnelSetKeys(p, mDNSfalse);
					p->loc_inner = m->AutoTunnelHostAddr;
					p->loc_outer = tmpSrc.ip.v4;
					AutoTunnelSetKeys(p, mDNStrue);
					}
				}
				else
					{
					if (!mDNSSameIPv6Address(p->loc_inner, m->AutoTunnelHostAddr) ||
						!mDNSSameIPv6Address(p->loc_outer6, m->AutoTunnelRelayAddrOut))
						{
						AutoTunnelSetKeys(p, mDNSfalse);
						p->loc_inner = m->AutoTunnelHostAddr;
						p->loc_outer6 = m->AutoTunnelRelayAddrOut;
						AutoTunnelSetKeys(p, mDNStrue);
						}
					}
			}


	SetSPS(m);

	NetworkInterfaceInfoOSX *i;
	for (i = m->p->InterfaceList; i; i = i->next)
		{
		if (!m->SPSSocket)		// Not being Sleep Proxy Server; close any open BPF fds
			{
			if (i->BPF_fd >= 0 && CountProxyTargets(m, i, mDNSNULL, mDNSNULL) == 0) CloseBPF(i);
			}
		else								// else, we're Sleep Proxy Server; open BPF fds
			{
			if (i->Exists && i->Registered == i && i->ifinfo.McastTxRx && !(i->ifa_flags & IFF_LOOPBACK) && i->BPF_fd == -1)
				{ LogSPS("%s requesting BPF", i->ifinfo.ifname); i->BPF_fd = -2; mDNSRequestBPF(); }
			}
		}

#endif // APPLE_OSX_mDNSResponder

	uDNS_SetupDNSConfig(m);
	mDNS_ConfigChanged(m);
	}

// Called with KQueueLock & mDNS lock
mDNSlocal void SetNetworkChanged(mDNS *const m, mDNSs32 delay)
	{
	if (!m->p->NetworkChanged || m->p->NetworkChanged - NonZeroTime(m->timenow + delay) < 0)
		{
		m->p->NetworkChanged = NonZeroTime(m->timenow + delay);
		LogInfo("SetNetworkChanged: setting network changed to %d (%d)", delay, m->p->NetworkChanged);
		}
	}

// Called with KQueueLock & mDNS lock
mDNSlocal void SetKeyChainTimer(mDNS *const m, mDNSs32 delay)
	{
	// If it's not set or it needs to happen sooner than when it's currently set
	if (!m->p->KeyChainTimer || m->p->KeyChainTimer - NonZeroTime(m->timenow + delay) > 0)
		{
		m->p->KeyChainTimer = NonZeroTime(m->timenow + delay);
		LogInfo("SetKeyChainTimer: %d", delay);
		}
	}

// Copy the fourth slash-delimited element from either:
//   State:/Network/Interface/<bsdname>/IPv4
// or
//   Setup:/Network/Service/<servicename>/Interface
mDNSlocal CFStringRef CopyNameFromKey(CFStringRef key)
	{
	CFArrayRef a;
	CFStringRef name = NULL;

	a = CFStringCreateArrayBySeparatingStrings(NULL, key, CFSTR("/"));
	if (a && CFArrayGetCount(a) == 5) name = CFRetain(CFArrayGetValueAtIndex(a, 3));
	if (a != NULL) CFRelease(a);

	return name;
	}

// Whether a key from a network change notification corresponds to
// an IP service that is explicitly configured for IPv4 Link Local
mDNSlocal mDNSBool ChangedKeysHaveIPv4LL(CFArrayRef inkeys)
	{
	SCDynamicStoreRef store = NULL;
	CFDictionaryRef dict = NULL;
	CFMutableArrayRef a;
	const void **keys = NULL, **vals = NULL;
	CFStringRef pattern = NULL;
	int i, ic, j, jc;
	mDNSBool found = mDNSfalse;
	
	jc = CFArrayGetCount(inkeys);
	if (!jc) goto done;

	store = SCDynamicStoreCreate(NULL, CFSTR("mDNSResponder:ChangedKeysHaveIPv4LL"), NULL, NULL);
	if (store == NULL) goto done;

	a = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	if (a == NULL) goto done;

	// Setup:/Network/Service/[^/]+/Interface
	pattern = SCDynamicStoreKeyCreateNetworkServiceEntity(NULL, kSCDynamicStoreDomainSetup, kSCCompAnyRegex, kSCEntNetInterface);
	if (pattern == NULL) goto done;
	CFArrayAppendValue(a, pattern);
	CFRelease(pattern);

	// Setup:/Network/Service/[^/]+/IPv4
	pattern = SCDynamicStoreKeyCreateNetworkServiceEntity(NULL, kSCDynamicStoreDomainSetup, kSCCompAnyRegex, kSCEntNetIPv4);
	if (pattern == NULL) goto done;
	CFArrayAppendValue(a, pattern);
	CFRelease(pattern);

	dict = SCDynamicStoreCopyMultiple(store, NULL, a);
	CFRelease(a);

	if (!dict)
		{
		LogMsg("ChangedKeysHaveIPv4LL: Empty dictionary");
		goto done;
		}

	ic = CFDictionaryGetCount(dict);
	vals = mDNSPlatformMemAllocate(sizeof (void *) * ic);
	keys = mDNSPlatformMemAllocate(sizeof (void *) * ic);
	CFDictionaryGetKeysAndValues(dict, keys, vals);
	
	for (j = 0; j < jc && !found; j++)
		{
		CFStringRef key = CFArrayGetValueAtIndex(inkeys, j);
		CFStringRef ifname = NULL;

		char buf[256];

		// It would be nice to use a regex here
		if (!CFStringHasPrefix(key, CFSTR("State:/Network/Interface/")) || !CFStringHasSuffix(key, kSCEntNetIPv4)) continue;
		
		if ((ifname = CopyNameFromKey(key)) == NULL) continue;
		if (mDNS_LoggingEnabled)
			{
			if (!CFStringGetCString(ifname, buf, sizeof(buf), kCFStringEncodingUTF8)) buf[0] = 0;
			LogInfo("ChangedKeysHaveIPv4LL: potential ifname %s", buf);
			}

		for (i = 0; i < ic; i++)
			{
			CFDictionaryRef ipv4dict;
			CFStringRef name;
			CFStringRef serviceid;
			CFStringRef configmethod;
			
			if (!CFStringHasSuffix(keys[i], kSCEntNetInterface)) continue;
			
			if (CFDictionaryGetTypeID() != CFGetTypeID(vals[i])) continue;
	
			if ((name = CFDictionaryGetValue(vals[i], kSCPropNetInterfaceDeviceName)) == NULL) continue;
	
			if (!CFEqual(ifname, name)) continue;
			
			if ((serviceid = CopyNameFromKey(keys[i])) == NULL) continue;
			if (mDNS_LoggingEnabled)
				{
				if (!CFStringGetCString(serviceid, buf, sizeof(buf), kCFStringEncodingUTF8)) buf[0] = 0;
				LogInfo("ChangedKeysHaveIPv4LL: found serviceid %s", buf);
				}
			
			pattern = SCDynamicStoreKeyCreateNetworkServiceEntity(NULL, kSCDynamicStoreDomainSetup, serviceid, kSCEntNetIPv4);
			CFRelease(serviceid);
			if (pattern == NULL) continue;
			
			ipv4dict = CFDictionaryGetValue(dict, pattern);
			CFRelease(pattern);
			if (!ipv4dict || CFDictionaryGetTypeID() != CFGetTypeID(ipv4dict)) continue;
			
			configmethod = CFDictionaryGetValue(ipv4dict, kSCPropNetIPv4ConfigMethod);
			if (!configmethod) continue;

			if (mDNS_LoggingEnabled)
				{
				if (!CFStringGetCString(configmethod, buf, sizeof(buf), kCFStringEncodingUTF8)) buf[0] = 0;
				LogInfo("ChangedKeysHaveIPv4LL: configmethod %s", buf);
				}
				
			if (CFEqual(configmethod, kSCValNetIPv4ConfigMethodLinkLocal)) { found = mDNStrue; break; }
			}
		
		CFRelease(ifname);
		}

	done:
	if (vals != NULL) mDNSPlatformMemFree(vals);
	if (keys != NULL) mDNSPlatformMemFree(keys);
	if (dict != NULL) CFRelease(dict);
	if (store != NULL) CFRelease(store);

	return found;
	}

mDNSlocal void NetworkChanged(SCDynamicStoreRef store, CFArrayRef changedKeys, void *context)
	{
	(void)store;        // Parameter not used
	mDNSBool changeNow = mDNSfalse;
	mDNS *const m = (mDNS *const)context;
	KQueueLock(m);
	mDNS_Lock(m);

	mDNSs32 delay = mDNSPlatformOneSecond * 2;				// Start off assuming a two-second delay

	int c = CFArrayGetCount(changedKeys);					// Count changes
	CFRange range = { 0, c };
	int c1 = (CFArrayContainsValue(changedKeys, range, NetworkChangedKey_Hostnames   ) != 0);
	int c2 = (CFArrayContainsValue(changedKeys, range, NetworkChangedKey_Computername) != 0);
	int c3 = (CFArrayContainsValue(changedKeys, range, NetworkChangedKey_DynamicDNS  ) != 0);
	int c4 = (CFArrayContainsValue(changedKeys, range, NetworkChangedKey_DNS         ) != 0);
	if (c && c - c1 - c2 - c3 - c4 == 0) delay = mDNSPlatformOneSecond/10;	// If these were the only changes, shorten delay
	
	{
	int i;
	for (i=0; i<c; i++)
		{
		char buf[256];
		if (!CFStringGetCString(CFArrayGetValueAtIndex(changedKeys, i), buf, sizeof(buf), kCFStringEncodingUTF8)) buf[0] = 0;
		if (buf[0])
			{
			if (strstr(buf, "p2p"))
				{
				LogInfo("NetworkChanged SC key: %s, not delaying network change", buf);
				changeNow = mDNStrue;
				break;
				}
			}
		}
	}

	if (mDNS_LoggingEnabled)
		{
		int i;
		for (i=0; i<c; i++)
			{
			char buf[256];
			if (!CFStringGetCString(CFArrayGetValueAtIndex(changedKeys, i), buf, sizeof(buf), kCFStringEncodingUTF8)) buf[0] = 0;
			LogInfo("***   NetworkChanged SC key: %s", buf);
			}
		LogInfo("***   NetworkChanged   *** %d change%s %s%s%s%sdelay %d",
			c, c>1?"s":"",
			c1 ? "(Local Hostname) " : "",
			c2 ? "(Computer Name) "  : "",
			c3 ? "(DynamicDNS) "     : "",
			c4 ? "(DNS) "            : "",
			delay);
		}

	mDNSBool btmmChanged = CFArrayContainsValue(changedKeys, range, NetworkChangedKey_BackToMyMac);
	if (btmmChanged) delay = 0;

	SetNetworkChanged(m, delay);
	
	// Other software might pick up these changes to register or browse in WAB or BTMM domains,
	// so in order for secure updates to be made to the server, make sure to read the keychain and
	// setup the DomainAuthInfo before handing the network change.
	// If we don't, then we will first try to register services in the clear, then later setup the
	// DomainAuthInfo, which is incorrect.
	if (c3 || btmmChanged)
		SetKeyChainTimer(m, delay);

	mDNS_Unlock(m);

	// If DNS settings changed, immediately force a reconfig (esp. cache flush)
	// Similarly, if an interface changed that is explicitly IPv4 link local, immediately force a reconfig
	if (c4 || ChangedKeysHaveIPv4LL(changedKeys) || changeNow) mDNSMacOSXNetworkChanged(m);

	KQueueUnlock(m, "NetworkChanged");
	}

#if APPLE_OSX_mDNSResponder
mDNSlocal void RefreshSPSStatus(const void *key, const void *value, void *context)
	{
	(void)context;
	char buf[IFNAMSIZ];

	CFStringRef ifnameStr = (CFStringRef)key;
	CFArrayRef array = (CFArrayRef)value;
	if (!CFStringGetCString(ifnameStr, buf, sizeof(buf), kCFStringEncodingUTF8)) buf[0] = 0;

	LogInfo("RefreshSPSStatus: Updating SPS state for key %s, array count %d", buf, CFArrayGetCount(array));
	mDNSDynamicStoreSetConfig(kmDNSSleepProxyServersState, buf, value);
	}
#endif

mDNSlocal void DynamicStoreReconnected(SCDynamicStoreRef store, void *info)
	{
	mDNS *const m = (mDNS *const)info;
	(void)store;

	LogInfo("DynamicStoreReconnected: Reconnected");

	// State:/Network/MulticastDNS
	SetLocalDomains();

	// State:/Network/DynamicDNS
	if (m->FQDN.c[0])
		mDNSPlatformDynDNSHostNameStatusChanged(&m->FQDN, 1);

	// Note: PrivateDNS and BackToMyMac are automatically populated when configd is restarted
	// as we receive network change notifications and thus not necessary. But we leave it here
	// so that if things are done differently in the future, this code still works.

	// State:/Network/PrivateDNS
	CFMutableArrayRef sa = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	if (!sa)
		LogMsg("DynamicStoreReconnected:PrivateDNS: CFArrayCreateMutable failed");
	else
		{
		DomainAuthInfo *FoundInList;
		char stringbuf[MAX_ESCAPED_DOMAIN_NAME];	// Max legal domainname as C-string, including terminating NUL
		for (FoundInList = m->AuthInfoList; FoundInList; FoundInList = FoundInList->next)
			{
			ConvertDomainNameToCString(&FoundInList->domain, stringbuf);
			CFStringRef cfs = CFStringCreateWithCString(NULL, stringbuf, kCFStringEncodingUTF8);
			if (cfs) { CFArrayAppendValue(sa, cfs); CFRelease(cfs); }
			}
		mDNSDynamicStoreSetConfig(kmDNSPrivateConfig, mDNSNULL, sa);
		CFRelease(sa);
		}

	// State:/Network/BackToMyMac
#if APPLE_OSX_mDNSResponder
	mDNS_Lock(m);	
	UpdateAutoTunnelDomainStatuses(m);
	mDNS_Unlock(m);	

	// State:/Network/Interface/en0/SleepProxyServers
	if (spsStatusDict) CFDictionaryApplyFunction(spsStatusDict, RefreshSPSStatus, NULL);
#endif
	}

mDNSlocal mStatus WatchForNetworkChanges(mDNS *const m)
	{
	mStatus err = -1;
	SCDynamicStoreContext context = { 0, m, NULL, NULL, NULL };
	SCDynamicStoreRef     store    = SCDynamicStoreCreate(NULL, CFSTR("mDNSResponder:WatchForNetworkChanges"), NetworkChanged, &context);
	CFMutableArrayRef     keys     = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	CFStringRef           pattern1 = SCDynamicStoreKeyCreateNetworkInterfaceEntity(NULL, kSCDynamicStoreDomainState, kSCCompAnyRegex, kSCEntNetIPv4);
	CFStringRef           pattern2 = SCDynamicStoreKeyCreateNetworkInterfaceEntity(NULL, kSCDynamicStoreDomainState, kSCCompAnyRegex, kSCEntNetIPv6);
	CFMutableArrayRef     patterns = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);

	if (!store) { LogMsg("SCDynamicStoreCreate failed: %s", SCErrorString(SCError())); goto error; }
	if (!keys || !pattern1 || !pattern2 || !patterns) goto error;

	CFArrayAppendValue(keys, NetworkChangedKey_IPv4);
	CFArrayAppendValue(keys, NetworkChangedKey_IPv6);
	CFArrayAppendValue(keys, NetworkChangedKey_Hostnames);
	CFArrayAppendValue(keys, NetworkChangedKey_Computername);
	CFArrayAppendValue(keys, NetworkChangedKey_DNS);
	CFArrayAppendValue(keys, NetworkChangedKey_DynamicDNS);
	CFArrayAppendValue(keys, NetworkChangedKey_BackToMyMac);
	CFArrayAppendValue(keys, NetworkChangedKey_PowerSettings); // should remove as part of <rdar://problem/6751656>
	CFArrayAppendValue(keys, NetworkChangedKey_BTMMConnectivity);
	CFArrayAppendValue(patterns, pattern1);
	CFArrayAppendValue(patterns, pattern2);
	CFArrayAppendValue(patterns, CFSTR("State:/Network/Interface/[^/]+/AirPort"));
	if (!SCDynamicStoreSetNotificationKeys(store, keys, patterns))
		{ LogMsg("SCDynamicStoreSetNotificationKeys failed: %s", SCErrorString(SCError())); goto error; }

#ifdef MDNSRESPONDER_USES_LIB_DISPATCH_AS_PRIMARY_EVENT_LOOP_MECHANISM
	if (!SCDynamicStoreSetDispatchQueue(store, dispatch_get_main_queue()))
		{ LogMsg("SCDynamicStoreCreateRunLoopSource failed: %s", SCErrorString(SCError())); goto error; }
#else
	m->p->StoreRLS = SCDynamicStoreCreateRunLoopSource(NULL, store, 0);
	if (!m->p->StoreRLS) { LogMsg("SCDynamicStoreCreateRunLoopSource failed: %s", SCErrorString(SCError())); goto error; }
	CFRunLoopAddSource(CFRunLoopGetCurrent(), m->p->StoreRLS, kCFRunLoopDefaultMode);
#endif
	SCDynamicStoreSetDisconnectCallBack(store, DynamicStoreReconnected);
	m->p->Store = store;
	err = 0;
	goto exit;

	error:
	if (store)    CFRelease(store);

	exit:
	if (patterns) CFRelease(patterns);
	if (pattern2) CFRelease(pattern2);
	if (pattern1) CFRelease(pattern1);
	if (keys)     CFRelease(keys);

	return(err);
	}

#if 0 // <rdar://problem/6751656>
mDNSlocal void PMChanged(void *context)
	{
	mDNS *const m = (mDNS *const)context;

	KQueueLock(m);
	mDNS_Lock(m);
	
	LogSPS("PMChanged");
	
	SetNetworkChanged(m, mDNSPlatformOneSecond * 2);
	
	mDNS_Unlock(m);
	KQueueUnlock(m, "PMChanged");
	}

mDNSlocal mStatus WatchForPMChanges(mDNS *const m)
	{
	m->p->PMRLS = IOPMPrefsNotificationCreateRunLoopSource(PMChanged, m);
	if (!m->p->PMRLS) { LogMsg("IOPMPrefsNotificationCreateRunLoopSource failed!"); return mStatus_UnknownErr; }

	CFRunLoopAddSource(CFRunLoopGetCurrent(), m->p->PMRLS, kCFRunLoopDefaultMode);

	return mStatus_NoError;
	}
#endif

#ifndef KEV_DL_WAKEFLAGS_CHANGED
#define KEV_DL_WAKEFLAGS_CHANGED 17
#endif

#if !TARGET_OS_EMBEDDED		// don't setup packet filter rules on embedded

mDNSlocal void mDNSSetPacketFilterRules(mDNS *const m, char * ifname)
	{
	AuthRecord *rr;
	int	found = 0;
	
	for (rr = m->ResourceRecords; rr; rr=rr->next)
		{
		if ((rr->resrec.rrtype == kDNSServiceType_SRV) && (rr->ARType == AuthRecordAnyIncludeP2P))
			{
			uint16_t 		port = rr->resrec.rdata->u.srv.port.NotAnInteger;
			uint16_t 		protocol;
			const mDNSu8	*p;

			LogInfo("mDNSSetPacketFilterRules: found %s", ARDisplayString(m, rr));

			// Note presence of more than one p2p service in list
			// since code is currently opening only one port in the packet filter.
			found++;
			if (found > 1)
				{
				LogMsg("mDNSSetPacketFilterRules: found service #%d %s", found, ARDisplayString(m, rr));
				}

			// Assume <Service Instance>.<App Protocol>.<Transport Protocol>.<Name>
			p = rr->resrec.name->c;

			// Skip to App Protocol
			if (p[0]) p += 1 + p[0];
			// Skip to Transport Protocol
			if (p[0]) p += 1 + p[0];

			if      (SameDomainLabel(p, (mDNSu8 *)"\x4" "_tcp")) protocol = IPPROTO_TCP;
			else if (SameDomainLabel(p, (mDNSu8 *)"\x4" "_udp")) protocol = IPPROTO_UDP;
			else 
				{ 
				LogMsg("mDNSSetPacketFilterRules: could not determine transport protocol of service");
				LogMsg("mDNSSetPacketFilterRules: %s", ARDisplayString(m, rr));
				return;
				}

			LogInfo("mDNSSetPacketFilterRules: ifname %s, port(in NBO)0x%X, protocol %d",
					ifname, port, protocol);
			mDNSPacketFilterControl(PF_SET_RULES, ifname, port, protocol);
			}
		}
	}
#endif // !TARGET_OS_EMBEDDED


#if APPLE_OSX_mDNSResponder	

#if !TARGET_OS_EMBEDDED	
// If the p2p interface already exists, set the Bonjour packet filter rules for it.
mDNSexport void mDNSInitPacketFilter(void)
	{
	mDNS *const m = & mDNSStorage;

	NetworkInterfaceInfo *intf = GetFirstActiveInterface(m->HostInterfaces);
	while (intf)
		{
		if (strncmp(intf->ifname, "p2p", 3) == 0)
			{
			LogInfo("mDNSInitPacketFilter: Setting rules for ifname %s", intf->ifname);
			mDNSSetPacketFilterRules(m, intf->ifname);
			break;
			}
		intf = GetFirstActiveInterface(intf->next);
		}
	}

#else // !TARGET_OS_EMBEDDED

	// Currently no packet filter setup required embedded on platforms.
mDNSexport void mDNSInitPacketFilter()
	{
	}

#endif // !TARGET_OS_EMBEDDED
#endif // APPLE_OSX_mDNSResponder

mDNSlocal void SysEventCallBack(int s1, short __unused filter, void *context)
	{
	mDNS *const m = (mDNS *const)context;

	mDNS_Lock(m);

	struct { struct kern_event_msg k; char extra[256]; } msg;
	int bytes = recv(s1, &msg, sizeof(msg), 0);
	if (bytes < 0)
		LogMsg("SysEventCallBack: recv error %d errno %d (%s)", bytes, errno, strerror(errno));
	else
		{
		LogInfo("SysEventCallBack got %d bytes size %d %X %s %X %s %X %s id %d code %d %s",
			bytes, msg.k.total_size,
			msg.k.vendor_code , msg.k.vendor_code  == KEV_VENDOR_APPLE  ? "KEV_VENDOR_APPLE"  : "?",
			msg.k.kev_class   , msg.k.kev_class    == KEV_NETWORK_CLASS ? "KEV_NETWORK_CLASS" : "?",
			msg.k.kev_subclass, msg.k.kev_subclass == KEV_DL_SUBCLASS   ? "KEV_DL_SUBCLASS"   : "?",
			msg.k.id, msg.k.event_code,
			msg.k.event_code == KEV_DL_SIFFLAGS             ? "KEV_DL_SIFFLAGS"             :
			msg.k.event_code == KEV_DL_SIFMETRICS           ? "KEV_DL_SIFMETRICS"           :
			msg.k.event_code == KEV_DL_SIFMTU               ? "KEV_DL_SIFMTU"               :
			msg.k.event_code == KEV_DL_SIFPHYS              ? "KEV_DL_SIFPHYS"              :
			msg.k.event_code == KEV_DL_SIFMEDIA             ? "KEV_DL_SIFMEDIA"             :
			msg.k.event_code == KEV_DL_SIFGENERIC           ? "KEV_DL_SIFGENERIC"           :
			msg.k.event_code == KEV_DL_ADDMULTI             ? "KEV_DL_ADDMULTI"             :
			msg.k.event_code == KEV_DL_DELMULTI             ? "KEV_DL_DELMULTI"             :
			msg.k.event_code == KEV_DL_IF_ATTACHED          ? "KEV_DL_IF_ATTACHED"          :
			msg.k.event_code == KEV_DL_IF_DETACHING         ? "KEV_DL_IF_DETACHING"         :
			msg.k.event_code == KEV_DL_IF_DETACHED          ? "KEV_DL_IF_DETACHED"          :
			msg.k.event_code == KEV_DL_LINK_OFF             ? "KEV_DL_LINK_OFF"             :
			msg.k.event_code == KEV_DL_LINK_ON              ? "KEV_DL_LINK_ON"              :
			msg.k.event_code == KEV_DL_PROTO_ATTACHED       ? "KEV_DL_PROTO_ATTACHED"       :
			msg.k.event_code == KEV_DL_PROTO_DETACHED       ? "KEV_DL_PROTO_DETACHED"       :
			msg.k.event_code == KEV_DL_LINK_ADDRESS_CHANGED ? "KEV_DL_LINK_ADDRESS_CHANGED" :
			msg.k.event_code == KEV_DL_WAKEFLAGS_CHANGED    ? "KEV_DL_WAKEFLAGS_CHANGED"    : "?");

		// We receive network change notifications both through configd and through SYSPROTO_EVENT socket.
		// Configd may not generate network change events for manually configured interfaces (i.e., non-DHCP)
		// always during sleep/wakeup due to some race conditions (See radar:8666757). At the same time, if
		// "Wake on Network Access" is not turned on, the notification will not have KEV_DL_WAKEFLAGS_CHANGED.
		// Hence, during wake up, if we see a KEV_DL_LINK_ON (i.e., link is UP), we trigger a network change.

		if (msg.k.event_code == KEV_DL_WAKEFLAGS_CHANGED || msg.k.event_code == KEV_DL_LINK_ON)
			SetNetworkChanged(m, mDNSPlatformOneSecond * 2);

#if !TARGET_OS_EMBEDDED		// don't setup packet filter rules on embedded

		// For p2p interfaces, need to open the advertised service port in the firewall.
		if (msg.k.event_code == KEV_DL_IF_ATTACHED)
			{
			struct net_event_data   * p;
			p = (struct net_event_data *) & msg.k.event_data;

			if (strncmp(p->if_name, "p2p", 3) == 0)
				{
				char	ifname[IFNAMSIZ];
				snprintf(ifname, IFNAMSIZ, "%s%d", p->if_name, p->if_unit);

				LogInfo("SysEventCallBack: KEV_DL_IF_ATTACHED if_family = %d, if_unit = %d, if_name = %s", p->if_family, p->if_unit, p->if_name);

				mDNSSetPacketFilterRules(m, ifname);
				}
			}

		// For p2p interfaces, need to clear the firewall rules on interface detach
		if (msg.k.event_code == KEV_DL_IF_DETACHED)
			{
			struct net_event_data   * p;
			p = (struct net_event_data *) & msg.k.event_data;

			if (strncmp(p->if_name, "p2p", 3) == 0)
				{
				char	ifname[IFNAMSIZ];
				snprintf(ifname, IFNAMSIZ, "%s%d", p->if_name, p->if_unit);

				LogInfo("SysEventCallBack: KEV_DL_IF_DETACHED if_family = %d, if_unit = %d, if_name = %s", p->if_family, p->if_unit, p->if_name);

				mDNSPacketFilterControl(PF_CLEAR_RULES, ifname, 0, 0);
				}
			}
#endif // !TARGET_OS_EMBEDDED

		}

	mDNS_Unlock(m);
	}

mDNSlocal mStatus WatchForSysEvents(mDNS *const m)
	{
	m->p->SysEventNotifier = socket(PF_SYSTEM, SOCK_RAW, SYSPROTO_EVENT);
	if (m->p->SysEventNotifier < 0)
		{ LogMsg("WatchForSysEvents: socket failed error %d errno %d (%s)", m->p->SysEventNotifier, errno, strerror(errno)); return(mStatus_NoMemoryErr); }

	struct kev_request kev_req = { KEV_VENDOR_APPLE, KEV_NETWORK_CLASS, KEV_DL_SUBCLASS };
	int err = ioctl(m->p->SysEventNotifier, SIOCSKEVFILT, &kev_req);
	if (err < 0)
		{
		LogMsg("WatchForSysEvents: SIOCSKEVFILT failed error %d errno %d (%s)", err, errno, strerror(errno));
		close(m->p->SysEventNotifier);
		m->p->SysEventNotifier = -1;
		return(mStatus_UnknownErr);
		}

	m->p->SysEventKQueue.KQcallback = SysEventCallBack;
	m->p->SysEventKQueue.KQcontext  = m;
	m->p->SysEventKQueue.KQtask     = "System Event Notifier";
	KQueueSet(m->p->SysEventNotifier, EV_ADD, EVFILT_READ, &m->p->SysEventKQueue);

	return(mStatus_NoError);
	}

#ifndef NO_SECURITYFRAMEWORK
mDNSlocal OSStatus KeychainChanged(SecKeychainEvent keychainEvent, SecKeychainCallbackInfo *info, void *context)
	{
	LogInfo("***   Keychain Changed   ***");
	mDNS *const m = (mDNS *const)context;
	SecKeychainRef skc;
	OSStatus err = SecKeychainCopyDefault(&skc);
	if (!err)
		{
		if (info->keychain == skc)
			{
			// For delete events, attempt to verify what item was deleted fail because the item is already gone, so we just assume they may be relevant
			mDNSBool relevant = (keychainEvent == kSecDeleteEvent);
			if (!relevant)
				{
				UInt32 tags[3] = { kSecTypeItemAttr, kSecServiceItemAttr, kSecAccountItemAttr };
				SecKeychainAttributeInfo attrInfo = { 3, tags, NULL };	// Count, array of tags, array of formats
				SecKeychainAttributeList *a = NULL;
				err = SecKeychainItemCopyAttributesAndData(info->item, &attrInfo, NULL, &a, NULL, NULL);
				if (!err)
					{
					relevant = ((a->attr[0].length == 4 && (!strncasecmp(a->attr[0].data, "ddns", 4) || !strncasecmp(a->attr[0].data, "sndd", 4))) ||
								(a->attr[1].length >= mDNSPlatformStrLen(dnsprefix) && (!strncasecmp(a->attr[1].data, dnsprefix, mDNSPlatformStrLen(dnsprefix)))) || 
								(a->attr[1].length >= mDNSPlatformStrLen(btmmprefix) && (!strncasecmp(a->attr[1].data, btmmprefix, mDNSPlatformStrLen(btmmprefix)))));
					SecKeychainItemFreeAttributesAndData(a, NULL);
					}
				}
			if (relevant)
				{
				LogInfo("***   Keychain Changed   *** KeychainEvent=%d %s",
					keychainEvent,
					keychainEvent == kSecAddEvent    ? "kSecAddEvent"    :
					keychainEvent == kSecDeleteEvent ? "kSecDeleteEvent" :
					keychainEvent == kSecUpdateEvent ? "kSecUpdateEvent" : "<Unknown>");
				// We're running on the CFRunLoop (Mach port) thread, not the kqueue thread, so we need to grab the KQueueLock before proceeding
				KQueueLock(m);
				mDNS_Lock(m);

				// To not read the keychain twice: when BTMM is enabled, changes happen to the keychain
				// then the BTMM DynStore dictionary, so delay reading the keychain for a second.
				// NetworkChanged() will reset the keychain timer to fire immediately when the DynStore changes.
				//
				// In the "fixup" case where the BTMM DNS servers aren't accepting the key mDNSResponder has,
				// the DynStore dictionary won't change (because the BTMM zone won't change).  In that case,
				// a one second delay is ok, as we'll still converge to correctness, and there's no race
				// condition between the RegistrationDomain and the DomainAuthInfo.
				//
				// Lastly, non-BTMM WAB cases can use the keychain but not the DynStore, so we need to set
				// the timer here, as it will not get set by NetworkChanged().
				SetKeyChainTimer(m, mDNSPlatformOneSecond);

				mDNS_Unlock(m);
				KQueueUnlock(m, "KeychainChanged");
				}
			}
		CFRelease(skc);
		}

	return 0;
	}
#endif

mDNSlocal void PowerOn(mDNS *const m)
	{
	mDNSCoreMachineSleep(m, false);		// Will set m->SleepState = SleepState_Awake;
	if (m->p->WakeAtUTC)
		{
		long utc = mDNSPlatformUTC();
		mDNSPowerRequest(-1,-1);		// Need to explicitly clear any previous power requests -- they're not cleared automatically on wake
		if      (m->p->WakeAtUTC - utc > 30)               LogSPS("PowerChanged PowerOn %d seconds early, assuming not maintenance wake",     m->p->WakeAtUTC - utc);
		else if (utc - m->p->WakeAtUTC > 30)               LogSPS("PowerChanged PowerOn %d seconds late, assuming not maintenance wake",      utc - m->p->WakeAtUTC);
		else if (!strncasecmp(HINFO_HWstring, "K66AP", 5)) LogSPS("PowerChanged PowerOn %d seconds late, device is K66AP so not re-sleeping", utc - m->p->WakeAtUTC);
		else
			{
			LogSPS("PowerChanged: Waking for network maintenance operations %d seconds early; re-sleeping in 20 seconds", m->p->WakeAtUTC - utc);
			m->p->RequestReSleep = mDNS_TimeNow(m) + 20 * mDNSPlatformOneSecond;
			}
		}
	}

mDNSlocal void PowerChanged(void *refcon, io_service_t service, natural_t messageType, void *messageArgument)
	{
	mDNS *const m = (mDNS *const)refcon;
	KQueueLock(m);
	(void)service;    // Parameter not used
	debugf("PowerChanged %X %lX", messageType, messageArgument);

	// Make sure our m->SystemWakeOnLANEnabled value correctly reflects the current system setting
	m->SystemWakeOnLANEnabled = SystemWakeForNetworkAccess();

	switch(messageType)
		{
		case kIOMessageCanSystemPowerOff:		LogSPS("PowerChanged kIOMessageCanSystemPowerOff     (no action)");	break;		// E0000240
		case kIOMessageSystemWillPowerOff:		LogSPS("PowerChanged kIOMessageSystemWillPowerOff");							// E0000250
												mDNSCoreMachineSleep(m, true);
												if (m->SleepState == SleepState_Sleeping) mDNSMacOSXNetworkChanged(m);
												break;
		case kIOMessageSystemWillNotPowerOff:	LogSPS("PowerChanged kIOMessageSystemWillNotPowerOff (no action)");	break;		// E0000260
		case kIOMessageCanSystemSleep:			LogSPS("PowerChanged kIOMessageCanSystemSleep        (no action)");	break;		// E0000270
		case kIOMessageSystemWillSleep:			LogSPS("PowerChanged kIOMessageSystemWillSleep");								// E0000280
												mDNSCoreMachineSleep(m, true);
												break;
		case kIOMessageSystemWillNotSleep:		LogSPS("PowerChanged kIOMessageSystemWillNotSleep    (no action)");	break;		// E0000290
		case kIOMessageSystemHasPoweredOn:		LogSPS("PowerChanged kIOMessageSystemHasPoweredOn");							// E0000300
												// If still sleeping (didn't get 'WillPowerOn' message for some reason?) wake now
												if (m->SleepState)
													{
													LogMsg("PowerChanged kIOMessageSystemHasPoweredOn: ERROR m->SleepState %d", m->SleepState);
													PowerOn(m);
													}
												// Just to be safe, schedule a mDNSMacOSXNetworkChanged(), in case we never received
												// the System Configuration Framework "network changed" event that we expect
												// to receive some time shortly after the kIOMessageSystemWillPowerOn message
												mDNS_Lock(m);
												if (!m->p->NetworkChanged ||
													m->p->NetworkChanged - NonZeroTime(m->timenow + mDNSPlatformOneSecond * 2) < 0)
													m->p->NetworkChanged = NonZeroTime(m->timenow + mDNSPlatformOneSecond * 2);
												mDNS_Unlock(m);
											
												break;
		case kIOMessageSystemWillRestart:		LogSPS("PowerChanged kIOMessageSystemWillRestart     (no action)");	break;		// E0000310
		case kIOMessageSystemWillPowerOn:		LogSPS("PowerChanged kIOMessageSystemWillPowerOn");								// E0000320

												// On Leopard and earlier, on wake from sleep, instead of reporting link state down, Apple
												// Ethernet drivers report "hardware incapable of detecting link state", which the kernel
												// interprets as "should assume we have networking", which results in the first 4-5 seconds
												// we block for five seconds to let Ethernet come up, and then resume normal operation.
												if (OSXVers && OSXVers < OSXVers_10_6_SnowLeopard)
													{
													sleep(5);
													LogMsg("Running on Mac OS X version 10.%d earlier than 10.6; "
														"PowerChanged did sleep(5) to wait for Ethernet hardware", OSXVers - OSXVers_Base);
													}

												// Make sure our interface list is cleared to the empty state, then tell mDNSCore to wake
												if (m->SleepState != SleepState_Sleeping)
													{
													LogMsg("kIOMessageSystemWillPowerOn: ERROR m->SleepState %d", m->SleepState);
													m->SleepState = SleepState_Sleeping;
													mDNSMacOSXNetworkChanged(m);
													}
												PowerOn(m);
												break;
		default:								LogSPS("PowerChanged unknown message %X", messageType); break;
		}

	if (messageType == kIOMessageSystemWillSleep) m->p->SleepCookie = (long)messageArgument;
	else IOAllowPowerChange(m->p->PowerConnection, (long)messageArgument);

	KQueueUnlock(m, "PowerChanged Sleep/Wake");
	}

// iPhone OS doesn't currently have SnowLeopard's IO Power Management
// but it does define kIOPMAcknowledgmentOptionSystemCapabilityRequirements
#if defined(kIOPMAcknowledgmentOptionSystemCapabilityRequirements) && \
	!TARGET_OS_EMBEDDED
mDNSlocal void SnowLeopardPowerChanged(void *refcon, IOPMConnection connection, IOPMConnectionMessageToken token, IOPMSystemPowerStateCapabilities eventDescriptor)
	{
	mDNS *const m = (mDNS *const)refcon;
	KQueueLock(m);
	LogSPS("SnowLeopardPowerChanged %X %X %X%s%s%s%s%s",
		connection, token, eventDescriptor,
		eventDescriptor & kIOPMSystemPowerStateCapabilityCPU     ? " CPU"     : "",
		eventDescriptor & kIOPMSystemPowerStateCapabilityVideo   ? " Video"   : "",
		eventDescriptor & kIOPMSystemPowerStateCapabilityAudio   ? " Audio"   : "",
		eventDescriptor & kIOPMSystemPowerStateCapabilityNetwork ? " Network" : "",
		eventDescriptor & kIOPMSystemPowerStateCapabilityDisk    ? " Disk"    : "");

	// Make sure our m->SystemWakeOnLANEnabled value correctly reflects the current system setting
	m->SystemWakeOnLANEnabled = SystemWakeForNetworkAccess();

	if (eventDescriptor & kIOPMSystemPowerStateCapabilityCPU)
		{
		// We might be in Sleeping or Transferring state. When we go from "wakeup" to "sleep" state, we don't
		// go directly to sleep state, but transfer in to the sleep state during which SleepState is set to
		// SleepState_Transferring. During that time, we might get another wakeup before we transition to Sleeping
		// state. In that case, we need to acknowledge the previous "sleep" before we acknowledge the wakeup.
		if (m->SleepLimit)
			{
			LogSPS("SnowLeopardPowerChanged: Waking up, Acking old Sleep, SleepLimit %d SleepState %d", m->SleepLimit, m->SleepState);
			IOPMConnectionAcknowledgeEvent(connection, m->p->SleepCookie);
			m->SleepLimit = 0;
			}
		LogSPS("SnowLeopardPowerChanged: Waking up, Acking Wakeup, SleepLimit %d SleepState %d", m->SleepLimit, m->SleepState);
		// If the network notifications have already come before we got the wakeup, we ignored them and
		// in case we get no more, we need to trigger one.
		mDNS_Lock(m);
		SetNetworkChanged(m, 2 * mDNSPlatformOneSecond);
		mDNS_Unlock(m);
		// CPU Waking. Note: Can get this message repeatedly, as other subsystems power up or down.
		if (m->SleepState != SleepState_Awake) PowerOn(m);
		IOPMConnectionAcknowledgeEvent(connection, token);
		}
	else
		{
		// CPU sleeping. Should not get this repeatedly -- once we're told that the CPU is halting
		// we should hear nothing more until we're told that the CPU has started executing again.
		if (m->SleepState) LogMsg("SnowLeopardPowerChanged: Sleep Error %X m->SleepState %d", eventDescriptor, m->SleepState);
		//sleep(5);
		//mDNSMacOSXNetworkChanged(m);
		mDNSCoreMachineSleep(m, true);
		//if (m->SleepState == SleepState_Sleeping) mDNSMacOSXNetworkChanged(m);
		m->p->SleepCookie = token;
		}

	KQueueUnlock(m, "SnowLeopardPowerChanged Sleep/Wake");
	}
#endif

#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark -
#pragma mark - /etc/hosts support
#endif

// Implementation Notes
//
// As /etc/hosts file can be huge (1000s of entries - when this comment was written, the test file had about
// 23000 entries with about 4000 duplicates), we can't use a linked list to store these entries. So, we parse
// them into a hash table. The implementation need to be able to do the following things efficiently
//
// 1. Detect duplicates e.g., two entries with "1.2.3.4 foo" 
// 2. Detect whether /etc/hosts has changed and what has changed since the last read from the disk
// 3. Ability to support multiple addresses per name e.g., "1.2.3.4 foo, 2.3.4.5 foo". To support this, we
//    need to be able set the RRSet of a resource record to the first one in the list and also update when
//    one of them go away. This is needed so that the core thinks that they are all part of the same RRSet and
//    not a duplicate
// 4. Don't maintain any local state about any records registered with the core to detect changes to /etc/hosts
//
// CFDictionary is not a suitable candidate because it does not support duplicates and even if we use a custom
// "hash" function to solve this, the others are hard to solve. Hence, we share the hash (AuthHash) implementation
// of the core layer which does all of the above very efficiently

#define ETCHOSTS_BUFSIZE	1024	// Buffer size to parse a single line in /etc/hosts

mDNSexport void FreeEtcHosts(mDNS *const m, AuthRecord *const rr, mStatus result)
    {
    (void)m;  // unused
	(void)rr;
	(void)result;
	if (result == mStatus_MemFree)
		{
		LogInfo("FreeEtcHosts: %s", ARDisplayString(m, rr));
		freeL("etchosts", rr);
		}
	}

// Returns true on success and false on failure
mDNSlocal mDNSBool mDNSMacOSXCreateEtcHostsEntry(mDNS *const m, const domainname *domain, const struct sockaddr *sa, const domainname *cname, char *ifname, AuthHash *auth)
	{
	AuthRecord *rr;
	mDNSu32 slot;
	mDNSu32 namehash;
	AuthGroup *ag;
	mDNSInterfaceID InterfaceID = mDNSInterface_LocalOnly;
	mDNSu16 rrtype;

	if (!domain)
		{
		LogMsg("mDNSMacOSXCreateEtcHostsEntry: ERROR!! name NULL");
		return mDNSfalse;
		}
	if (!sa && !cname)
		{
		LogMsg("mDNSMacOSXCreateEtcHostsEntry: ERROR!! sa and cname both NULL");
		return mDNSfalse;
		}

	if (sa && sa->sa_family != AF_INET && sa->sa_family != AF_INET6)
		{
		LogMsg("mDNSMacOSXCreateEtcHostsEntry: ERROR!! sa with bad family %d", sa->sa_family);
		return mDNSfalse;
		}
	

	if (ifname)
		{
		mDNSu32 ifindex = if_nametoindex(ifname);
		if (!ifindex)
			{
			LogMsg("mDNSMacOSXCreateEtcHostsEntry: hosts entry %##s with invalid ifname %s", domain->c, ifname);
			return mDNSfalse;
			}
		InterfaceID = (mDNSInterfaceID)(uintptr_t)ifindex;
		}

	if (sa)
		rrtype = (sa->sa_family == AF_INET ? kDNSType_A : kDNSType_AAAA);
	else
		rrtype = kDNSType_CNAME;

	// Check for duplicates. See whether we parsed an entry before like this ?
	slot = AuthHashSlot(domain);
	namehash = DomainNameHashValue(domain);
	ag = AuthGroupForName(auth, slot, namehash, domain);
	if (ag)
		{
		rr = ag->members;
		while (rr)
			{
			if (rr->resrec.rrtype == rrtype)
				{
				if (rrtype == kDNSType_A)
					{
					mDNSv4Addr ip;
					ip.NotAnInteger = ((struct sockaddr_in*)sa)->sin_addr.s_addr;
					if (mDNSSameIPv4Address(rr->resrec.rdata->u.ipv4, ip))
						{
						LogInfo("mDNSMacOSXCreateEtcHostsEntry: Same IPv4 address for name %##s", domain->c);
						return mDNSfalse;
						}
					}
				else if (rrtype == kDNSType_AAAA)
					{
					mDNSv6Addr ip6;
					ip6.l[0] = ((struct sockaddr_in6*)sa)->sin6_addr.__u6_addr.__u6_addr32[0];
					ip6.l[1] = ((struct sockaddr_in6*)sa)->sin6_addr.__u6_addr.__u6_addr32[1];
					ip6.l[2] = ((struct sockaddr_in6*)sa)->sin6_addr.__u6_addr.__u6_addr32[2];
					ip6.l[3] = ((struct sockaddr_in6*)sa)->sin6_addr.__u6_addr.__u6_addr32[3];
					if (mDNSSameIPv6Address(rr->resrec.rdata->u.ipv6, ip6))
						{
						LogInfo("mDNSMacOSXCreateEtcHostsEntry: Same IPv6 address for name %##s", domain->c);
						return mDNSfalse;
						}
					}
				else if (rrtype == kDNSType_CNAME)
					{
					if (SameDomainName(&rr->resrec.rdata->u.name, cname))
						{
						LogInfo("mDNSMacOSXCreateEtcHostsEntry: Same cname %##s for name %##s", cname->c, domain->c);
						return mDNSfalse;
						}
					}
				}
			rr = rr->next;
			}
		}
	rr= mallocL("etchosts", sizeof(*rr));
	if (rr == NULL) return mDNSfalse;
	mDNSPlatformMemZero(rr, sizeof(*rr));
	mDNS_SetupResourceRecord(rr, NULL, InterfaceID, rrtype, 1, kDNSRecordTypeKnownUnique, AuthRecordLocalOnly, FreeEtcHosts, NULL);
	AssignDomainName(&rr->namestorage, domain);

	if (sa)
		{
		rr->resrec.rdlength = sa->sa_family == AF_INET ? sizeof(mDNSv4Addr) : sizeof(mDNSv6Addr);
		if (sa->sa_family == AF_INET)
			rr->resrec.rdata->u.ipv4.NotAnInteger = ((struct sockaddr_in*)sa)->sin_addr.s_addr;
		else
			{
			rr->resrec.rdata->u.ipv6.l[0] = ((struct sockaddr_in6*)sa)->sin6_addr.__u6_addr.__u6_addr32[0];
			rr->resrec.rdata->u.ipv6.l[1] = ((struct sockaddr_in6*)sa)->sin6_addr.__u6_addr.__u6_addr32[1];
			rr->resrec.rdata->u.ipv6.l[2] = ((struct sockaddr_in6*)sa)->sin6_addr.__u6_addr.__u6_addr32[2];
			rr->resrec.rdata->u.ipv6.l[3] = ((struct sockaddr_in6*)sa)->sin6_addr.__u6_addr.__u6_addr32[3];
			}
		}
	else
		{
		rr->resrec.rdlength = DomainNameLength(cname);
		rr->resrec.rdata->u.name.c[0] = 0;
		AssignDomainName(&rr->resrec.rdata->u.name, cname);
		}
	rr->resrec.namehash = DomainNameHashValue(rr->resrec.name);
	SetNewRData(&rr->resrec, mDNSNULL, 0);	// Sets rr->rdatahash for us
	LogInfo("mDNSMacOSXCreateEtcHostsEntry: Adding resource record %s", ARDisplayString(m, rr));
	InsertAuthRecord(m, auth, rr);
	return mDNStrue;
	}

mDNSlocal int EtcHostsParseOneName(int start, int length, char *buffer, char **name)
	{
	int i;

	*name = NULL;
	for (i = start; i < length; i++)
		{
		if (buffer[i] == '#')
			return -1;
		if (buffer[i] != ' ' && buffer[i] != ',' && buffer[i] != '\t')
			{
			*name = &buffer[i];
			
			// Found the start of a name, find the end and null terminate
			for (i++; i < length; i++)
				{
				if (buffer[i] == ' ' || buffer[i] == ',' || buffer[i] == '\t')
					{
					buffer[i] = 0;
					break;
					}
				}
			return i;
			}
		}
	return -1;
	}

mDNSlocal void mDNSMacOSXParseEtcHostsLine(mDNS *const m, char *buffer, ssize_t length, AuthHash *auth)
	{
	int i;
	int ifStart = 0;
	char *ifname = NULL;
	domainname name1d;
	domainname name2d;
	char *name1;
	char *name2;
	int aliasIndex;
	
	// Find the end of the address string
	for (i = 0; i < length; i++)
		{
		if (buffer[i] == ' ' || buffer[i] == ',' || buffer[i] == '\t' || buffer[i] == '%')
			{
			if (buffer[i] == '%')
				ifStart = i + 1;
			buffer[i] = 0;
			break;
			}
		}
	
	// Convert the address string to an address
	struct addrinfo	hints;
	bzero(&hints, sizeof(hints));
	hints.ai_flags = AI_NUMERICHOST;
	struct addrinfo	*gairesults = NULL;
	if (getaddrinfo(buffer, NULL, &hints, &gairesults) != 0)
		{
		LogInfo("mDNSMacOSXParseEtcHostsLine: getaddrinfo returning null");
		return;
		}

	if (ifStart)
		{
		// Parse the interface
		ifname = &buffer[ifStart];
		for (i = ifStart + 1; i < length; i++)
			{
			if (buffer[i] == ' ' || buffer[i] == ',' || buffer[i] == '\t')
				{
				buffer[i] = 0;
				break;
				}
			}
		}
		
	i = EtcHostsParseOneName(i + 1, length, buffer, &name1);
	if (i == length)
		{
		// Common case (no aliases) : The entry is of the form "1.2.3.4 somehost" with no trailing white spaces/tabs etc.
		if (!MakeDomainNameFromDNSNameString(&name1d, name1))
			{
			LogMsg("mDNSMacOSXParseEtcHostsLine: ERROR!! cannot convert to domain name %s", name1);
			freeaddrinfo(gairesults);
			return;
			}
		mDNSMacOSXCreateEtcHostsEntry(m, &name1d, gairesults->ai_addr, mDNSNULL, ifname, auth);
		}
	else if (i != -1)
		{
		domainname first;
		// We might have some extra white spaces at the end for the common case of "1.2.3.4 somehost".
		// When we parse again below, EtchHostsParseOneName would return -1 and we will end up
		// doing the right thing.
		if (!MakeDomainNameFromDNSNameString(&first, name1))
			{
			LogMsg("mDNSMacOSXParseEtcHostsLine: ERROR!! cannot convert to domain name %s", name1);
			freeaddrinfo(gairesults);
			return;
			}
		// If the /etc/hosts has an entry like this
		//
		// 1.2.3.4 sun star bright
		//
		// star and bright are aliases (gethostbyname h_alias should point to these) and sun is the canonical
		// name (getaddrinfo ai_cannonname and gethostbyname h_name points to "sun")
		//
		// To achieve this, we need to add the entry like this:
		//
		// star CNAME bright
		// bright CNAME sun
		// sun A 1.2.3.4
		//
		// We store the first name we parsed in "first". Then we parse additional names adding CNAME records
		// till we reach the end. When we reach the end, we wrap around and add one final CNAME with the last 
		// entry and the first entry. Finally, we add the Address (A/AAAA) record.
		aliasIndex = 0;
		while (i <= length)
			{
			// Parse a name. If there are no names, we need to know whether we
			// parsed CNAMEs before or not. If we parsed CNAMEs before, then we
			// add a CNAME with the last name and the first name. Otherwise, this
			// is same as the common case above where the line has just one name
			// but with trailing white spaces.
			i = EtcHostsParseOneName(i + 1, length, buffer, &name2);
			if (name2)
				{
				if (!MakeDomainNameFromDNSNameString(&name2d, name2))
					{
					LogMsg("mDNSMacOSXParseEtcHostsLine: ERROR!! cannot convert to domain name %s", name2);
					freeaddrinfo(gairesults);
					return;
					}
				aliasIndex++;
				}
			else if (!aliasIndex)
				{
				// We have never parsed any aliases. This case happens if there
				// is just one name and some extra white spaces at the end.
				LogInfo("mDNSMacOSXParseEtcHostsLine: White space at the end of %##s", first.c);
				break;
				}
			else
				{
				// We have parsed at least one alias before and we reached the end of the line.
				// Setup a CNAME for the last name with "first" name as its RDATA
				name2d.c[0] = 0;
				AssignDomainName(&name2d, &first);
				}

			// Don't add a CNAME for the first alias we parse (see the example above).
			// As we parse more, we might discover that there are no more aliases, in
			// which case we would have set "name2d" to "first" above. We need to add
			// the CNAME in that case.

			if (aliasIndex > 1 || SameDomainName(&name2d, &first))
				{
				// Ignore if it points to itself 
				if (!SameDomainName(&name1d, &name2d))
					{
		 			if (!mDNSMacOSXCreateEtcHostsEntry(m, &name1d, mDNSNULL, &name2d, ifname, auth))
						{
						freeaddrinfo(gairesults);
						return;
						}
					}
				else
					LogMsg("mDNSMacOSXParseEtcHostsLine: Ignoring entry with same names name1 %##s, name2 %##s", name1d.c, name2d.c);
				}

			// If we have already wrapped around, we just need to add the A/AAAA record alone
			// which is done below
			if (SameDomainName(&name2d, &first)) break;

			// Remember the current name so that we can set the CNAME record if we parse one
			// more name
			name1d.c[0] = 0;
			AssignDomainName(&name1d, &name2d);
			}
		// Added all the CNAMEs if any, add the "A/AAAA" record
		mDNSMacOSXCreateEtcHostsEntry(m, &first, gairesults->ai_addr, mDNSNULL, ifname, auth);
		}
	freeaddrinfo(gairesults);
	}

mDNSlocal void mDNSMacOSXParseEtcHosts(mDNS *const m, int fd, AuthHash *auth)
	{
	mDNSBool good;
	char	buf[ETCHOSTS_BUFSIZE];
	int len;
	FILE *fp;

	if (fd == -1) { LogInfo("mDNSMacOSXParseEtcHosts: fd is -1"); return; }

	fp = fopen("/etc/hosts", "r");
	if (!fp) { LogInfo("mDNSMacOSXParseEtcHosts: fp is NULL"); return; }

	while (1)
		{
		good = (fgets(buf, ETCHOSTS_BUFSIZE, fp) != NULL);
		if (!good) break;

		// skip comment and empty lines
		if (buf[0] == '#' || buf[0] == '\r' || buf[0] == '\n')
			continue;

		len = strlen(buf);
		if (!len) break;	// sanity check

		if (buf[len - 1] == '\r' || buf[len - 1] == '\n')
			buf[len - 1] = '\0';

		// fgets always null terminates and hence even if we have no
		// newline at the end, it is null terminated. The callee expects
		// the length to be such that buf[length] to be zero and hence
		// we pass len - 1.
		mDNSMacOSXParseEtcHostsLine(m, buf, len - 1, auth);
		}
	fclose(fp);
	}

mDNSlocal void mDNSMacOSXUpdateEtcHosts(mDNS *const m);

mDNSlocal int mDNSMacOSXGetEtcHostsFD(mDNS *const m)
	{
#ifdef __DISPATCH_GROUP__
	// Can't do this stuff to be notified of changes in /etc/hosts if we don't have libdispatch
	static dispatch_queue_t		etcq	 = 0;
	static dispatch_source_t	etcsrc	 = 0;
	static dispatch_source_t	hostssrc = 0;
	
	// First time through? just schedule ourselves on the main queue and we'll do the work later
	if (!etcq)
	{
		etcq = dispatch_get_main_queue();
		if (etcq)
			{
			// Do this work on the queue, not here - solves potential synchronization issues
			dispatch_async(etcq, ^{mDNSMacOSXUpdateEtcHosts(m);});
			}
		return -1;
	}
	
	if (hostssrc) return dispatch_source_get_handle(hostssrc);
#endif

	int fd = open("/etc/hosts", O_RDONLY);
	
#ifdef __DISPATCH_GROUP__
	// Can't do this stuff to be notified of changes in /etc/hosts if we don't have libdispatch
	if (fd == -1)
		{
		// If the open failed and we're already watching /etc, we're done
		if (etcsrc) { LogInfo("mDNSMacOSXGetEtcHostsFD: Returning etcfd because no etchosts"); return fd; }
		
		// we aren't watching /etc, we should be
		fd = open("/etc", O_RDONLY);
		if (fd == -1) { LogInfo("mDNSMacOSXGetEtcHostsFD: etc does not exist"); return -1; }
		etcsrc = dispatch_source_create(DISPATCH_SOURCE_TYPE_VNODE, fd, DISPATCH_VNODE_DELETE | DISPATCH_VNODE_WRITE | DISPATCH_VNODE_RENAME, etcq);
		if (etcsrc == NULL)
			{
			close(fd);
			return -1;
			}
		dispatch_source_set_event_handler(etcsrc,
			^{
			u_int32_t	flags = dispatch_source_get_data(etcsrc);
			LogMsg("mDNSMacOSXGetEtcHostsFD: /etc changed 0x%x", flags);
			if ((flags & (DISPATCH_VNODE_DELETE | DISPATCH_VNODE_RENAME)) != 0)
				{
				dispatch_source_cancel(etcsrc);
				dispatch_release(etcsrc);
				etcsrc = NULL;
				dispatch_async(etcq, ^{mDNSMacOSXUpdateEtcHosts(m);});
				return;
				}
			if ((flags & DISPATCH_VNODE_WRITE) != 0 && hostssrc == NULL)
				{
				mDNSMacOSXUpdateEtcHosts(m);
				}
			});
		dispatch_source_set_cancel_handler(etcsrc, ^{close(fd);});
		dispatch_resume(etcsrc);
		
		// Try and open /etc/hosts once more now that we're watching /etc, in case we missed the creation
		fd = open("/etc/hosts", O_RDONLY | O_EVTONLY);
		if (fd == -1) { LogMsg("mDNSMacOSXGetEtcHostsFD etc hosts does not exist, watching etc"); return -1; }
		}
	
	// create a dispatch source to watch for changes to hosts file
	hostssrc = dispatch_source_create(DISPATCH_SOURCE_TYPE_VNODE, fd,
					(DISPATCH_VNODE_DELETE | DISPATCH_VNODE_WRITE | DISPATCH_VNODE_RENAME |
					DISPATCH_VNODE_ATTRIB | DISPATCH_VNODE_EXTEND | DISPATCH_VNODE_LINK | DISPATCH_VNODE_REVOKE), etcq);
	if (hostssrc == NULL)
		{
		close(fd);
		return -1;
		}
	dispatch_source_set_event_handler(hostssrc,
		^{
		u_int32_t	flags = dispatch_source_get_data(hostssrc);
		LogInfo("mDNSMacOSXGetEtcHostsFD: /etc/hosts changed 0x%x", flags);
		if ((flags & (DISPATCH_VNODE_DELETE | DISPATCH_VNODE_RENAME)) != 0)
			{
			dispatch_source_cancel(hostssrc);
			dispatch_release(hostssrc);
			hostssrc = NULL;
			// Bug in LibDispatch: wait a second before scheduling the block. If we schedule
			// the block immediately, we try to open the file and the file may not exist and may
			// fail to get a notification in the future. When the file does not exist and
			// we start to monitor the directory, on "dispatch_resume" of that source, there
			// is no guarantee that the file creation will be notified always because when
			// the dispatch_resume returns, the kevent manager may not have registered the
			// kevent yet but the file may have been created
			usleep(1000000);
			dispatch_async(etcq, ^{mDNSMacOSXUpdateEtcHosts(m);});
			return;
			}
		if ((flags & DISPATCH_VNODE_WRITE) != 0)
			{
			mDNSMacOSXUpdateEtcHosts(m);
			}
		});
	dispatch_source_set_cancel_handler(hostssrc, ^{LogInfo("mDNSMacOSXGetEtcHostsFD: Closing etchosts fd %d", fd); close(fd);});
	dispatch_resume(hostssrc);
	
	// Cleanup /etc source, no need to watch it if we already have /etc/hosts
	if (etcsrc)
		{
		dispatch_source_cancel(etcsrc);
		dispatch_release(etcsrc);
		etcsrc = NULL;
		}
	
	LogInfo("mDNSMacOSXGetEtcHostsFD: /etc/hosts being monitored, and not etc");
	return hostssrc ? (int)dispatch_source_get_handle(hostssrc) : -1;
#else
	(void)m;
	return fd;
#endif;
	}

// When /etc/hosts is modified, flush all the cache records as there may be local
// authoritative answers now
mDNSlocal void FlushAllCacheRecords(mDNS *const m)
	{
	CacheRecord *cr;
	mDNSu32 slot;
	CacheGroup *cg;

	FORALL_CACHERECORDS(slot, cg, cr)
		{
		// Skip multicast.
		if (cr->resrec.InterfaceID) continue;

		// If a resource record can answer A or AAAA, they need to be flushed so that we will
		// never used to deliver an ADD or RMV
		if (RRTypeAnswersQuestionType(&cr->resrec, kDNSType_A) ||
			RRTypeAnswersQuestionType(&cr->resrec, kDNSType_AAAA))
			{
			LogInfo("FlushAllCacheRecords: Purging Resourcerecord %s", CRDisplayString(m, cr));
			mDNS_PurgeCacheResourceRecord(m, cr);
			}
		}
	}

// Add new entries to the core. If justCheck is set, this function does not add, just returns true
mDNSlocal mDNSBool EtcHostsAddNewEntries(mDNS *const m, AuthHash *newhosts, mDNSBool justCheck)
	{
	AuthGroup *ag;
	mDNSu32 slot;
	AuthRecord *rr, *primary, *rrnext;
	for (slot = 0; slot < AUTH_HASH_SLOTS; slot++)
		for (ag = newhosts->rrauth_hash[slot]; ag; ag = ag->next)
			{
			primary = NULL;
			for (rr = ag->members; rr; rr = rrnext)
				{
				rrnext = rr->next;
				AuthGroup *ag1;
				AuthRecord *rr1;
				mDNSBool found = mDNSfalse;
				ag1 = AuthGroupForRecord(&m->rrauth, slot, &rr->resrec);
				if (ag1 && ag1->members)
					{
					if (!primary) primary = ag1->members;
					rr1 = ag1->members;
					while (rr1)
						{
						// We are not using InterfaceID in checking for duplicates. This means,
						// if there are two addresses for a given name e.g., fe80::1%en0 and
						// fe80::1%en1, we only add the first one. It is not clear whether
						// this is a common case. To fix this, we also need to modify
						// mDNS_Register_internal in how it handles duplicates. If it becomes a
						// common case, we will fix it then.
						if (IdenticalResourceRecord(&rr1->resrec, &rr->resrec))
							{
							LogInfo("EtcHostsAddNewEntries: Skipping, not adding %s", ARDisplayString(m, rr1));
							found = mDNStrue;
							break;
							}
						rr1 = rr1->next;
						}
					}
				if (!found)
					{
					if (justCheck) 
						{
						LogInfo("EtcHostsAddNewEntries: Entry %s not registered with core yet", ARDisplayString(m, rr));
						return mDNStrue;
						}
					RemoveAuthRecord(m, newhosts, rr);
					// if there is no primary, point to self
					rr->RRSet = (primary ? primary : rr);
					rr->next = NULL;
					LogInfo("EtcHostsAddNewEntries: Adding %s", ARDisplayString(m, rr));
					if (mDNS_Register_internal(m, rr) != mStatus_NoError)
						LogMsg("EtcHostsAddNewEntries: mDNS_Register failed for %s", ARDisplayString(m, rr));
					}
				}
			}
		return mDNSfalse;
	}

// Delete entries from the core that are no longer needed. If justCheck is set, this function
// does not delete, just returns true
mDNSlocal mDNSBool EtcHostsDeleteOldEntries(mDNS *const m, AuthHash *newhosts, mDNSBool justCheck)
	{
	AuthGroup *ag;
	mDNSu32 slot;
	AuthRecord *rr, *primary, *rrnext;
	for (slot = 0; slot < AUTH_HASH_SLOTS; slot++)
		for (ag = m->rrauth.rrauth_hash[slot]; ag; ag = ag->next)
			for (rr = ag->members; rr; rr = rrnext)
				{
				mDNSBool found = mDNSfalse;
				AuthGroup *ag1;
				AuthRecord *rr1;
				rrnext = rr->next;
				if (rr->RecordCallback != FreeEtcHosts) continue;
				ag1 = AuthGroupForRecord(newhosts, slot, &rr->resrec);
				if (ag1)
					{
					primary = rr1 = ag1->members;
					while (rr1)
						{
						if (IdenticalResourceRecord(&rr1->resrec, &rr->resrec))
							{
							LogInfo("EtcHostsDeleteOldEntries: Old record %s found in new, skipping", ARDisplayString(m, rr));
							found = mDNStrue;
							break;
							}
						rr1 = rr1->next;
						}
					}
				// there is no corresponding record in newhosts for the same name. This means
				// we should delete this from the core.
				if (!found)
					{
					if (justCheck)
						{
						LogInfo("EtcHostsDeleteOldEntries: Record %s not found in new, deleting", ARDisplayString(m, rr));
						return mDNStrue;
						}
					// if primary is going away, make sure that the rest of the records
					// point to the new primary
					if (rr == ag->members)
						{
						AuthRecord *new_primary = rr->next;
						AuthRecord *r = new_primary;
						while (r)
							{
							if (r->RRSet == rr)
								{
								LogInfo("EtcHostsDeleteOldEntries: Updating Resource Record %s to primary", ARDisplayString(m, r));
								r->RRSet = new_primary;
								}
							else LogMsg("EtcHostsDeleteOldEntries: ERROR!! Resource Record %s not pointing to primary %##s", ARDisplayString(m, r), r->resrec.name);
							r = r->next;
							}
						}
					LogInfo("EtcHostsDeleteOldEntries: Deleting %s", ARDisplayString(m, rr));
					mDNS_Deregister_internal(m, rr, mDNS_Dereg_normal);
					}
				}
	return mDNSfalse;
	}

mDNSlocal void UpdateEtcHosts(mDNS *const m, void *context)
	{
	AuthHash *newhosts = (AuthHash *)context;

	if (!m->mDNS_busy) LogMsg("UpdateEtcHosts: ERROR!! Lock not held");

	//Delete old entries from the core if they are not present in the newhosts
	EtcHostsDeleteOldEntries(m, newhosts, mDNSfalse);
	// Add the new entries to the core if not already present in the core
	EtcHostsAddNewEntries(m, newhosts, mDNSfalse);
	}

mDNSlocal void FreeNewHosts(AuthHash *newhosts)
	{
	mDNSu32 slot;
	AuthGroup *ag, *agnext;
	AuthRecord *rr, *rrnext;

	for (slot = 0; slot < AUTH_HASH_SLOTS; slot++)
		for (ag = newhosts->rrauth_hash[slot]; ag; ag = agnext)
			{
			agnext = ag->next;
			for (rr = ag->members; rr; rr = rrnext)
				{
				rrnext = rr->next;
				freeL("etchosts", rr);
				}
			freeL("AuthGroups", ag);
			}
	}

mDNSlocal void mDNSMacOSXUpdateEtcHosts(mDNS *const m)
	{
	AuthHash newhosts;
	
	// As we will be modifying the core, we can only have one thread running at
	// any point in time.
	KQueueLock(m);

	mDNSPlatformMemZero(&newhosts, sizeof(AuthHash));

	// Get the file desecriptor (will trigger us to start watching for changes)
	int fd = mDNSMacOSXGetEtcHostsFD(m);
	if (fd != -1)
		{
		LogInfo("mDNSMacOSXUpdateEtcHosts: Parsing /etc/hosts fd %d", fd);
		mDNSMacOSXParseEtcHosts(m, fd, &newhosts);
		}
	else LogInfo("mDNSMacOSXUpdateEtcHosts: /etc/hosts is not present");
	
	// Optimization: Detect whether /etc/hosts changed or not.
	//
	// 1. Check to see if there are any new entries. We do this by seeing whether any entries in 
	//    newhosts is already registered with core.  If we find at least one entry that is not
	//    registered with core, then it means we have work to do.
	//    
	// 2. Next, we check to see if any of the entries that are registered with core is not present
	//   in newhosts. If we find at least one entry that is not present, it means we have work to
	//   do.
	// 
	// Note: We may not have to hold the lock right here as KQueueLock is held which prevents any
	// other thread from running. But mDNS_Lock is needed here as we will be traversing the core
	// data structure in EtcHostsDeleteOldEntries/NewEntries which might expect the lock to be held
	// in the future and this code does not have to change.
	mDNS_Lock(m);
	// Add the new entries to the core if not already present in the core
	if (!EtcHostsAddNewEntries(m, &newhosts, mDNStrue))
		{
		// No new entries to add, check to see if we need to delete any old entries from the
		// core if they are not present in the newhosts
		if (!EtcHostsDeleteOldEntries(m, &newhosts, mDNStrue))
			{
			LogInfo("mDNSMacOSXUpdateEtcHosts: No work");
			mDNS_Unlock(m);
			KQueueUnlock(m, "/etc/hosts changed");
			FreeNewHosts(&newhosts);
			return;
			}
		}

	// This will flush the cache, stop and start the query so that the queries
	// can look at the /etc/hosts again
	//
	// Notes:
	//
	// We can't delete and free the records here. We wait for the mDNSCoreRestartAddressQueries to
	// deliver RMV events. It has to be done in a deferred way because we can't deliver RMV
	// events for local records *before* the RMV events for cache records. mDNSCoreRestartAddressQueries
	// delivers these events in the right order and then calls us back to delete them.
	//
	// Similarly, we do a deferred Registration of the record because mDNSCoreRestartAddressQueries
	// is a common function that looks at all local auth records and delivers a RMV including
	// the records that we might add here. If we deliver a ADD here, it will get a RMV and then when
	// the query is restarted, it will get another ADD. To avoid this (ADD-RMV-ADD), we defer registering
	// the record until the RMVs are delivered in mDNSCoreRestartAddressQueries after which UpdateEtcHosts
	// is called back where we do the Registration of the record. This results in RMV followed by ADD which
    // looks normal.
	mDNSCoreRestartAddressQueries(m, mDNSfalse, FlushAllCacheRecords, UpdateEtcHosts, &newhosts);
	mDNS_Unlock(m);

	KQueueUnlock(m, "/etc/hosts changed");
	FreeNewHosts(&newhosts);
	}

#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark -
#pragma mark - Initialization & Teardown
#endif

CF_EXPORT CFDictionaryRef _CFCopySystemVersionDictionary(void);
CF_EXPORT const CFStringRef _kCFSystemVersionProductNameKey;
CF_EXPORT const CFStringRef _kCFSystemVersionProductVersionKey;
CF_EXPORT const CFStringRef _kCFSystemVersionBuildVersionKey;

// Major version  6 is 10.2.x (Jaguar)
// Major version  7 is 10.3.x (Panther)
// Major version  8 is 10.4.x (Tiger)
// Major version  9 is 10.5.x (Leopard)
// Major version 10 is 10.6.x (SnowLeopard)
mDNSexport void mDNSMacOSXSystemBuildNumber(char *HINFO_SWstring)
	{
	int major = 0, minor = 0;
	char letter = 0, prodname[256]="<Unknown>", prodvers[256]="<Unknown>", buildver[256]="<Unknown>";
	CFDictionaryRef vers = _CFCopySystemVersionDictionary();
	if (vers)
		{
		CFStringRef cfprodname = CFDictionaryGetValue(vers, _kCFSystemVersionProductNameKey);
		CFStringRef cfprodvers = CFDictionaryGetValue(vers, _kCFSystemVersionProductVersionKey);
		CFStringRef cfbuildver = CFDictionaryGetValue(vers, _kCFSystemVersionBuildVersionKey);
		if (cfprodname) CFStringGetCString(cfprodname, prodname, sizeof(prodname), kCFStringEncodingUTF8);
		if (cfprodvers) CFStringGetCString(cfprodvers, prodvers, sizeof(prodvers), kCFStringEncodingUTF8);
		if (cfbuildver && CFStringGetCString(cfbuildver, buildver, sizeof(buildver), kCFStringEncodingUTF8))
			sscanf(buildver, "%d%c%d", &major, &letter, &minor);
		CFRelease(vers);
		}
	if (!major) { major=8; LogMsg("Note: No Major Build Version number found; assuming 8"); }
	if (HINFO_SWstring) mDNS_snprintf(HINFO_SWstring, 256, "%s %s (%s), %s", prodname, prodvers, buildver, STRINGIFY(mDNSResponderVersion));
	//LogMsg("%s %s (%s), %d %c %d", prodname, prodvers, buildver, major, letter, minor);
	
	// If product name is "Mac OS X" (or similar) we set OSXVers, else we set iOSVers;
	if ((prodname[0] & 0xDF) == 'M') OSXVers = major; else iOSVers = major;
	}

// Test to see if we're the first client running on UDP port 5353, by trying to bind to 5353 without using SO_REUSEPORT.
// If we fail, someone else got here first. That's not a big problem; we can share the port for multicast responses --
// we just need to be aware that we shouldn't expect to successfully receive unicast UDP responses.
mDNSlocal mDNSBool mDNSPlatformInit_CanReceiveUnicast(void)
	{
	int err = -1;
	int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (s < 3)
		LogMsg("mDNSPlatformInit_CanReceiveUnicast: socket error %d errno %d (%s)", s, errno, strerror(errno));
	else
		{
		struct sockaddr_in s5353;
		s5353.sin_family      = AF_INET;
		s5353.sin_port        = MulticastDNSPort.NotAnInteger;
		s5353.sin_addr.s_addr = 0;
		err = bind(s, (struct sockaddr *)&s5353, sizeof(s5353));
		close(s);
		}

	if (err) LogMsg("No unicast UDP responses");
	else     debugf("Unicast UDP responses okay");
	return(err == 0);
	}

// Construction of Default Browse domain list (i.e. when clients pass NULL) is as follows:
// 1) query for b._dns-sd._udp.local on LocalOnly interface
//    (.local manually generated via explicit callback)
// 2) for each search domain (from prefs pane), query for b._dns-sd._udp.<searchdomain>.
// 3) for each result from (2), register LocalOnly PTR record b._dns-sd._udp.local. -> <result>
// 4) result above should generate a callback from question in (1).  result added to global list
// 5) global list delivered to client via GetSearchDomainList()
// 6) client calls to enumerate domains now go over LocalOnly interface
//    (!!!KRS may add outgoing interface in addition)

mDNSlocal mStatus mDNSPlatformInit_setup(mDNS *const m)
	{
	mStatus err;
	m->p->CFRunLoop = CFRunLoopGetCurrent();

	char HINFO_SWstring[256] = "";
	mDNSMacOSXSystemBuildNumber(HINFO_SWstring);

	// In 10.4, mDNSResponder is launched very early in the boot process, while other subsystems are still in the process of starting up.
	// If we can't read the user's preferences, then we sleep a bit and try again, for up to five seconds before we give up.
	int i;
	for (i=0; i<100; i++)
		{
		domainlabel testlabel;
		testlabel.c[0] = 0;
		GetUserSpecifiedLocalHostName(&testlabel);
		if (testlabel.c[0]) break;
		usleep(50000);
		}

	m->hostlabel.c[0]        = 0;

	int    get_model[2] = { CTL_HW, HW_MODEL };
	size_t len_model = sizeof(HINFO_HWstring_buffer);
	
	// Normal Apple model names are of the form "iPhone2,1", and
	// internal code names are strings containing no commas, e.g. "N88AP".
	// We used to ignore internal code names, but Apple now uses these internal code names
	// even in released shipping products, so we no longer ignore strings containing no commas.
//	if (sysctl(get_model, 2, HINFO_HWstring_buffer, &len_model, NULL, 0) == 0 && strchr(HINFO_HWstring_buffer, ','))
	if (sysctl(get_model, 2, HINFO_HWstring_buffer, &len_model, NULL, 0) == 0)
		HINFO_HWstring = HINFO_HWstring_buffer;

	// For names of the form "iPhone2,1" we use "iPhone" as the prefix for automatic name generation.
	// For names of the form "N88AP" containg no comma, we use the entire string.
	HINFO_HWstring_prefixlen = strchr(HINFO_HWstring_buffer, ',') ? strcspn(HINFO_HWstring, "0123456789") : strlen(HINFO_HWstring);

	if (OSXVers && OSXVers <= OSXVers_10_6_SnowLeopard) m->KnownBugs |= mDNS_KnownBug_LimitedIPv6;
	if (OSXVers && OSXVers >= OSXVers_10_6_SnowLeopard) m->KnownBugs |= mDNS_KnownBug_LossySyslog;
	if (mDNSPlatformInit_CanReceiveUnicast()) m->CanReceiveUnicastOn5353 = mDNStrue;

	mDNSu32 hlen = mDNSPlatformStrLen(HINFO_HWstring);
	mDNSu32 slen = mDNSPlatformStrLen(HINFO_SWstring);
	if (hlen + slen < 254)
		{
		m->HIHardware.c[0] = hlen;
		m->HISoftware.c[0] = slen;
		mDNSPlatformMemCopy(&m->HIHardware.c[1], HINFO_HWstring, hlen);
		mDNSPlatformMemCopy(&m->HISoftware.c[1], HINFO_SWstring, slen);
		}

 	m->p->permanentsockets.port  = MulticastDNSPort;
 	m->p->permanentsockets.m     = m;
	m->p->permanentsockets.sktv4 = -1;
	m->p->permanentsockets.kqsv4.KQcallback = myKQSocketCallBack;
	m->p->permanentsockets.kqsv4.KQcontext  = &m->p->permanentsockets;
	m->p->permanentsockets.kqsv4.KQtask     = "UDP packet reception";
#ifndef NO_IPV6
	m->p->permanentsockets.sktv6 = -1;
	m->p->permanentsockets.kqsv6.KQcallback = myKQSocketCallBack;
	m->p->permanentsockets.kqsv6.KQcontext  = &m->p->permanentsockets;
	m->p->permanentsockets.kqsv6.KQtask     = "UDP packet reception";
#endif

	err = SetupSocket(&m->p->permanentsockets, MulticastDNSPort, AF_INET, mDNSNULL);
#ifndef NO_IPV6
	err = SetupSocket(&m->p->permanentsockets, MulticastDNSPort, AF_INET6, mDNSNULL);
#endif

	struct sockaddr_in s4;
	socklen_t n4 = sizeof(s4);
	if (getsockname(m->p->permanentsockets.sktv4, (struct sockaddr *)&s4, &n4) < 0) LogMsg("getsockname v4 error %d (%s)", errno, strerror(errno));
	else m->UnicastPort4.NotAnInteger = s4.sin_port;
#ifndef NO_IPV6
	if (m->p->permanentsockets.sktv6 >= 0)
		{
		struct sockaddr_in6 s6;
		socklen_t n6 = sizeof(s6);
		if (getsockname(m->p->permanentsockets.sktv6, (struct sockaddr *)&s6, &n6) < 0) LogMsg("getsockname v6 error %d (%s)", errno, strerror(errno));
		else m->UnicastPort6.NotAnInteger = s6.sin6_port;
		}
#endif

	m->p->InterfaceList      = mDNSNULL;
	m->p->userhostlabel.c[0] = 0;
	m->p->usernicelabel.c[0] = 0;
	m->p->prevoldnicelabel.c[0] = 0;
	m->p->prevnewnicelabel.c[0] = 0;
	m->p->prevoldhostlabel.c[0] = 0;
	m->p->prevnewhostlabel.c[0] = 0;
	m->p->NotifyUser         = 0;
	m->p->KeyChainTimer      = 0;
	m->p->WakeAtUTC          = 0;
	m->p->RequestReSleep     = 0;
	
#if APPLE_OSX_mDNSResponder
	uuid_generate(m->asl_uuid);
#endif

	m->AutoTunnelHostAddr.b[0] = 0;		// Zero out AutoTunnelHostAddr so UpdateInterfaceList() know it has to set it up
	m->AutoTunnelRelayAddrIn = zerov6Addr;
	m->AutoTunnelRelayAddrOut = zerov6Addr;

	NetworkChangedKey_IPv4         = SCDynamicStoreKeyCreateNetworkGlobalEntity(NULL, kSCDynamicStoreDomainState, kSCEntNetIPv4);
	NetworkChangedKey_IPv6         = SCDynamicStoreKeyCreateNetworkGlobalEntity(NULL, kSCDynamicStoreDomainState, kSCEntNetIPv6);
	NetworkChangedKey_Hostnames    = SCDynamicStoreKeyCreateHostNames(NULL);
	NetworkChangedKey_Computername = SCDynamicStoreKeyCreateComputerName(NULL);
	NetworkChangedKey_DNS          = SCDynamicStoreKeyCreateNetworkGlobalEntity(NULL, kSCDynamicStoreDomainState, kSCEntNetDNS);
	if (!NetworkChangedKey_IPv4 || !NetworkChangedKey_IPv6 || !NetworkChangedKey_Hostnames || !NetworkChangedKey_Computername || !NetworkChangedKey_DNS)
		{ LogMsg("SCDynamicStore string setup failed"); return(mStatus_NoMemoryErr); }

	err = WatchForNetworkChanges(m);
	if (err) { LogMsg("mDNSPlatformInit_setup: WatchForNetworkChanges failed %d", err); return(err); }

#if 0 // <rdar://problem/6751656>
	err = WatchForPMChanges(m);
	if (err) { LogMsg("mDNSPlatformInit_setup: WatchForPMChanges failed %d", err); return(err); }
#endif

	err = WatchForSysEvents(m);
	if (err) { LogMsg("mDNSPlatformInit_setup: WatchForSysEvents failed %d", err); return(err); }

	mDNSs32 utc = mDNSPlatformUTC();
	m->SystemWakeOnLANEnabled = SystemWakeForNetworkAccess();
	UpdateInterfaceList(m, utc);
	SetupActiveInterfaces(m, utc);

	// Explicitly ensure that our Keychain operations utilize the system domain.
#ifndef NO_SECURITYFRAMEWORK
	SecKeychainSetPreferenceDomain(kSecPreferencesDomainSystem);
#endif

	mDNS_Lock(m);
	SetDomainSecrets(m);
	SetLocalDomains();
	mDNS_Unlock(m);

#ifndef NO_SECURITYFRAMEWORK
	err = SecKeychainAddCallback(KeychainChanged, kSecAddEventMask|kSecDeleteEventMask|kSecUpdateEventMask, m);
	if (err) { LogMsg("mDNSPlatformInit_setup: SecKeychainAddCallback failed %d", err); return(err); }
#endif

#if !defined(kIOPMAcknowledgmentOptionSystemCapabilityRequirements) || \
	TARGET_OS_EMBEDDED
	LogMsg("Note: Compiled without SnowLeopard Fine-Grained Power Management support");
#else
	IOPMConnection c;
	IOReturn iopmerr = IOPMConnectionCreate(CFSTR("mDNSResponder"), kIOPMSystemPowerStateCapabilityCPU, &c);
	if (iopmerr) LogMsg("IOPMConnectionCreate failed %d", iopmerr);
	else
		{
		iopmerr = IOPMConnectionSetNotification(c, m, SnowLeopardPowerChanged);
		if (iopmerr) LogMsg("IOPMConnectionSetNotification failed %d", iopmerr);
		else
			{
			iopmerr = IOPMConnectionScheduleWithRunLoop(c, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
			if (iopmerr) LogMsg("IOPMConnectionScheduleWithRunLoop failed %d", iopmerr);
			}
		}
	m->p->IOPMConnection = iopmerr ? mDNSNULL : c;
	if (iopmerr) // If IOPMConnectionCreate unavailable or failed, proceed with old-style power notification code below
#endif // kIOPMAcknowledgmentOptionSystemCapabilityRequirements
		{
		m->p->PowerConnection = IORegisterForSystemPower(m, &m->p->PowerPortRef, PowerChanged, &m->p->PowerNotifier);
		if (!m->p->PowerConnection) { LogMsg("mDNSPlatformInit_setup: IORegisterForSystemPower failed"); return(-1); }
		else
			{
#ifdef MDNSRESPONDER_USES_LIB_DISPATCH_AS_PRIMARY_EVENT_LOOP_MECHANISM
			IONotificationPortSetDispatchQueue(m->p->PowerPortRef, dispatch_get_main_queue());
#else
			CFRunLoopAddSource(CFRunLoopGetCurrent(), IONotificationPortGetRunLoopSource(m->p->PowerPortRef), kCFRunLoopDefaultMode);
#endif /* MDNSRESPONDER_USES_LIB_DISPATCH_AS_PRIMARY_EVENT_LOOP_MECHANISM */
			}
		}

#if APPLE_OSX_mDNSResponder
	// Note: We use SPMetricPortability > 35 to indicate a laptop of some kind
	// SPMetricPortability <= 35 means nominally a non-portable machine (i.e. Mac mini or better)
	// Apple TVs, AirPort base stations, and Time Capsules do not actually weigh 3kg, but we assign them
	// higher 'nominal' masses to indicate they should be treated as being relatively less portable than a laptop
	if      (!strncasecmp(HINFO_HWstring, "Xserve",       6)) { SPMetricPortability = 25 /* 30kg */; SPMetricMarginalPower = 84 /* 250W */; SPMetricTotalPower = 85 /* 300W */; }
	else if (!strncasecmp(HINFO_HWstring, "RackMac",      7)) { SPMetricPortability = 25 /* 30kg */; SPMetricMarginalPower = 84 /* 250W */; SPMetricTotalPower = 85 /* 300W */; }
	else if (!strncasecmp(HINFO_HWstring, "MacPro",       6)) { SPMetricPortability = 27 /* 20kg */; SPMetricMarginalPower = 84 /* 250W */; SPMetricTotalPower = 85 /* 300W */; }
	else if (!strncasecmp(HINFO_HWstring, "PowerMac",     8)) { SPMetricPortability = 27 /* 20kg */; SPMetricMarginalPower = 82 /* 160W */; SPMetricTotalPower = 83 /* 200W */; }
	else if (!strncasecmp(HINFO_HWstring, "iMac",         4)) { SPMetricPortability = 30 /* 10kg */; SPMetricMarginalPower = 77 /*  50W */; SPMetricTotalPower = 78 /*  60W */; }
	else if (!strncasecmp(HINFO_HWstring, "Macmini",      7)) { SPMetricPortability = 33 /*  5kg */; SPMetricMarginalPower = 73 /*  20W */; SPMetricTotalPower = 74 /*  25W */; }
	else if (!strncasecmp(HINFO_HWstring, "TimeCapsule", 11)) { SPMetricPortability = 34 /*  4kg */; SPMetricMarginalPower = 10 /*  ~0W */; SPMetricTotalPower = 70 /*  13W */; }
	else if (!strncasecmp(HINFO_HWstring, "AirPort",      7)) { SPMetricPortability = 35 /*  3kg */; SPMetricMarginalPower = 10 /*  ~0W */; SPMetricTotalPower = 70 /*  12W */; }
	else if (!strncasecmp(HINFO_HWstring, "AppleTV",      7)) { SPMetricPortability = 35 /*  3kg */; SPMetricMarginalPower = 10 /*  ~0W */; SPMetricTotalPower = 73 /*  20W */; }
	else if (!strncasecmp(HINFO_HWstring, "K66AP",        5)) { SPMetricPortability = 35 /*  3kg */; SPMetricMarginalPower = 60 /*   1W */; SPMetricTotalPower = 63 /*   2W */; }
	else if (!strncasecmp(HINFO_HWstring, "MacBook",      7)) { SPMetricPortability = 37 /*  2kg */; SPMetricMarginalPower = 71 /*  13W */; SPMetricTotalPower = 72 /*  15W */; }
	else if (!strncasecmp(HINFO_HWstring, "PowerBook",    9)) { SPMetricPortability = 37 /*  2kg */; SPMetricMarginalPower = 71 /*  13W */; SPMetricTotalPower = 72 /*  15W */; }
	LogSPS("HW_MODEL: %.*s (%s) Portability %d Marginal Power %d Total Power %d",
		HINFO_HWstring_prefixlen, HINFO_HWstring, HINFO_HWstring, SPMetricPortability, SPMetricMarginalPower, SPMetricTotalPower);

	err = WatchForInternetSharingChanges(m);
	if (err) { LogMsg("WatchForInternetSharingChanges failed %d", err); return(err); }

	m->p->ConndBTMMDict = mDNSNULL;
#endif // APPLE_OSX_mDNSResponder

#ifdef MDNSRESPONDER_USES_LIB_DISPATCH_AS_PRIMARY_EVENT_LOOP_MECHANISM
	// Currently this is not defined. SSL code will eventually fix this. If it becomes
#ifdef __SSL_NEEDS_SERIALIZATION__
	SSLqueue = dispatch_queue_create("com.apple.mDNSResponder.SSLQueue", NULL);
#else
	SSLqueue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
#endif
	if (SSLqueue == mDNSNULL) LogMsg("dispatch_queue_create: SSL queue NULL");
#endif
	mDNSMacOSXUpdateEtcHosts(m);
	return(mStatus_NoError);
	}

mDNSexport mStatus mDNSPlatformInit(mDNS *const m)
	{
#if MDNS_NO_DNSINFO
	LogMsg("Note: Compiled without Apple-specific Split-DNS support");
#endif

	// Adding interfaces will use this flag, so set it now.
	m->DivertMulticastAdvertisements = !m->AdvertiseLocalAddresses;
	
#if APPLE_OSX_mDNSResponder
	m->SPSBrowseCallback = UpdateSPSStatus;
#endif // APPLE_OSX_mDNSResponder

	mStatus result = mDNSPlatformInit_setup(m);

	// We don't do asynchronous initialization on OS X, so by the time we get here the setup will already
	// have succeeded or failed -- so if it succeeded, we should just call mDNSCoreInitComplete() immediately
	if (result == mStatus_NoError)
		{
		mDNSCoreInitComplete(m, mStatus_NoError);

#if ! NO_D2D
		// We only initialize if mDNSCore successfully initialized.
		CHECK_D2D_FUNCTION(D2DInitialize)
			{
			D2DStatus ds = D2DInitialize(m->p->CFRunLoop, xD2DServiceCallback, m) ;
			if (ds != kD2DSuccess)
				LogMsg("D2DInitialiize failed: %d", ds);
			else
				LogMsg("D2DInitialize succeeded");
			}
#endif // ! NO_D2D
			
		}
	return(result);
	}

mDNSexport void mDNSPlatformClose(mDNS *const m)
	{
	if (m->p->PowerConnection)
		{
#ifdef MDNSRESPONDER_USES_LIB_DISPATCH_AS_PRIMARY_EVENT_LOOP_MECHANISM
		IONotificationPortSetDispatchQueue(m->p->PowerPortRef, NULL);		
#else
		CFRunLoopRemoveSource(CFRunLoopGetCurrent(), IONotificationPortGetRunLoopSource(m->p->PowerPortRef), kCFRunLoopDefaultMode);
#endif
		// According to <http://developer.apple.com/qa/qa2004/qa1340.html>, a single call
		// to IORegisterForSystemPower creates *three* objects that need to be disposed individually:
		IODeregisterForSystemPower(&m->p->PowerNotifier);
		IOServiceClose            ( m->p->PowerConnection);
		IONotificationPortDestroy ( m->p->PowerPortRef);
		m->p->PowerConnection = 0;
		}

	if (m->p->Store)
		{
#ifdef MDNSRESPONDER_USES_LIB_DISPATCH_AS_PRIMARY_EVENT_LOOP_MECHANISM
		if (!SCDynamicStoreSetDispatchQueue(m->p->Store, NULL))
			LogMsg("mDNSPlatformClose: SCDynamicStoreSetDispatchQueue failed");
#else
		CFRunLoopRemoveSource(CFRunLoopGetCurrent(), m->p->StoreRLS, kCFRunLoopDefaultMode);
		CFRunLoopSourceInvalidate(m->p->StoreRLS);
		CFRelease(m->p->StoreRLS);
		m->p->StoreRLS = NULL;
#endif
		CFRelease(m->p->Store);
		m->p->Store    = NULL;
		}
	
	if (m->p->PMRLS)
		{
		CFRunLoopRemoveSource(CFRunLoopGetCurrent(), m->p->PMRLS, kCFRunLoopDefaultMode);
		CFRunLoopSourceInvalidate(m->p->PMRLS);
		CFRelease(m->p->PMRLS);
		m->p->PMRLS = NULL;
		}

	if (m->p->SysEventNotifier >= 0) { close(m->p->SysEventNotifier); m->p->SysEventNotifier = -1; }

#if ! NO_D2D
		CHECK_D2D_FUNCTION(D2DTerminate)
		{
		D2DStatus ds = D2DTerminate();
		if (ds != kD2DSuccess)
			LogMsg("D2DTerminate failed: %d", ds);
		else
			LogMsg("D2DTerminate succeeded");
		}
#endif // ! NO_D2D

	mDNSs32 utc = mDNSPlatformUTC();
	MarkAllInterfacesInactive(m, utc);
	ClearInactiveInterfaces(m, utc);
	CloseSocketSet(&m->p->permanentsockets);

#if APPLE_OSX_mDNSResponder
	// clean up tunnels
	while (m->TunnelClients)
		{
		ClientTunnel *cur = m->TunnelClients;
		LogInfo("mDNSPlatformClose: removing client tunnel %p %##s from list", cur, cur->dstname.c);
		if (cur->q.ThisQInterval >= 0) mDNS_StopQuery(m, &cur->q);
		AutoTunnelSetKeys(cur, mDNSfalse);
		m->TunnelClients = cur->next;
		freeL("ClientTunnel", cur);
		}

	if (AnonymousRacoonConfig)
		{
		AnonymousRacoonConfig = mDNSNULL;
		LogInfo("mDNSPlatformClose: Deconfiguring autotunnel");
		(void)mDNSConfigureServer(kmDNSDown, mDNSNULL, mDNSNULL);
		}

	if (m->AutoTunnelHostAddrActive && m->AutoTunnelHostAddr.b[0])
		{
		m->AutoTunnelHostAddrActive = mDNSfalse;
		LogInfo("mDNSPlatformClose: Removing AutoTunnel address %.16a", &m->AutoTunnelHostAddr);
		(void)mDNSAutoTunnelInterfaceUpDown(kmDNSDown, m->AutoTunnelHostAddr.b);
		}
	if (m->p->ConndBTMMDict) CFRelease(m->p->ConndBTMMDict);
#endif // APPLE_OSX_mDNSResponder
	}

#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark -
#pragma mark - General Platform Support Layer functions
#endif

mDNSexport mDNSu32 mDNSPlatformRandomNumber(void)
	{
	return(arc4random());
	}

mDNSexport mDNSs32 mDNSPlatformOneSecond = 1000;
mDNSexport mDNSu32 mDNSPlatformClockDivisor = 0;

mDNSexport mStatus mDNSPlatformTimeInit(void)
	{
	// Notes: Typical values for mach_timebase_info:
	// tbi.numer = 1000 million
	// tbi.denom =   33 million
	// These are set such that (mach_absolute_time() * numer/denom) gives us nanoseconds;
	//          numer  / denom = nanoseconds per hardware clock tick (e.g. 30);
	//          denom  / numer = hardware clock ticks per nanosecond (e.g. 0.033)
	// (denom*1000000) / numer = hardware clock ticks per millisecond (e.g. 33333)
	// So: mach_absolute_time() / ((denom*1000000)/numer) = milliseconds
	//
	// Arithmetic notes:
	// tbi.denom is at least 1, and not more than 2^32-1.
	// Therefore (tbi.denom * 1000000) is at least one million, but cannot overflow a uint64_t.
	// tbi.denom is at least 1, and not more than 2^32-1.
	// Therefore clockdivisor should end up being a number roughly in the range 10^3 - 10^9.
	// If clockdivisor is less than 10^3 then that means that the native clock frequency is less than 1MHz,
	// which is unlikely on any current or future Macintosh.
	// If clockdivisor is greater than 10^9 then that means the native clock frequency is greater than 1000GHz.
	// When we ship Macs with clock frequencies above 1000GHz, we may have to update this code.
	struct mach_timebase_info tbi;
	kern_return_t result = mach_timebase_info(&tbi);
	if (result == KERN_SUCCESS) mDNSPlatformClockDivisor = ((uint64_t)tbi.denom * 1000000) / tbi.numer;
	return(result);
	}

mDNSexport mDNSs32 mDNSPlatformRawTime(void)
	{
	if (mDNSPlatformClockDivisor == 0) { LogMsg("mDNSPlatformRawTime called before mDNSPlatformTimeInit"); return(0); }

	static uint64_t last_mach_absolute_time = 0;
	//static uint64_t last_mach_absolute_time = 0x8000000000000000LL;	// Use this value for testing the alert display
	uint64_t this_mach_absolute_time = mach_absolute_time();
	if ((int64_t)this_mach_absolute_time - (int64_t)last_mach_absolute_time < 0)
		{
		LogMsg("mDNSPlatformRawTime: last_mach_absolute_time %08X%08X", last_mach_absolute_time);
		LogMsg("mDNSPlatformRawTime: this_mach_absolute_time %08X%08X", this_mach_absolute_time);
		// Update last_mach_absolute_time *before* calling NotifyOfElusiveBug()
		last_mach_absolute_time = this_mach_absolute_time;
		// Note: This bug happens all the time on 10.3
		NotifyOfElusiveBug("mach_absolute_time went backwards!",
			"This error occurs from time to time, often on newly released hardware, "
			"and usually the exact cause is different in each instance.\r\r"
			"Please file a new Radar bug report with the title “mach_absolute_time went backwards” "
			"and assign it to Radar Component “Kernel” Version “X”.");
		}
	last_mach_absolute_time = this_mach_absolute_time;

	return((mDNSs32)(this_mach_absolute_time / mDNSPlatformClockDivisor));
	}

mDNSexport mDNSs32 mDNSPlatformUTC(void)
	{
	return time(NULL);
	}

// Locking is a no-op here, because we're single-threaded with a CFRunLoop, so we can never interrupt ourselves
mDNSexport void     mDNSPlatformLock   (const mDNS *const m) { (void)m; }
mDNSexport void     mDNSPlatformUnlock (const mDNS *const m) { (void)m; }
mDNSexport void     mDNSPlatformStrCopy(      void *dst, const void *src)              { strcpy((char *)dst, (char *)src); }
mDNSexport mDNSu32  mDNSPlatformStrLen (                 const void *src)              { return(strlen((char*)src)); }
mDNSexport void     mDNSPlatformMemCopy(      void *dst, const void *src, mDNSu32 len) { memcpy(dst, src, len); }
mDNSexport mDNSBool mDNSPlatformMemSame(const void *dst, const void *src, mDNSu32 len) { return(memcmp(dst, src, len) == 0); }
mDNSexport void     mDNSPlatformMemZero(      void *dst,                  mDNSu32 len) { memset(dst, 0, len); }
#if !(APPLE_OSX_mDNSResponder && MACOSX_MDNS_MALLOC_DEBUGGING)
mDNSexport void *   mDNSPlatformMemAllocate(mDNSu32 len) { return(mallocL("mDNSPlatformMemAllocate", len)); }
#endif
mDNSexport void     mDNSPlatformMemFree    (void *mem)   { freeL("mDNSPlatformMemFree", mem); }

mDNSexport void mDNSPlatformSetAllowSleep(mDNS *const m, mDNSBool allowSleep, const char *reason)
	{
	if (allowSleep && m->p->IOPMAssertion)
		{
		LogInfo("%s Destroying NoIdleSleep power assertion", __FUNCTION__);
		IOPMAssertionRelease(m->p->IOPMAssertion);
		m->p->IOPMAssertion = 0;
		}
	else if (!allowSleep && m->p->IOPMAssertion == 0)
		{
#ifdef kIOPMAssertionTypeNoIdleSleep
		CFStringRef assertionName = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("%s.%d %s"), getprogname(), getpid(), reason ? reason : "");
		IOPMAssertionCreateWithName(kIOPMAssertionTypeNoIdleSleep, kIOPMAssertionLevelOn, assertionName ? assertionName : CFSTR("mDNSResponder"), &m->p->IOPMAssertion);
		if (assertionName) CFRelease(assertionName);
		LogInfo("%s Creating NoIdleSleep power assertion", __FUNCTION__);
#endif
		}
	}

mDNSexport void mDNSPlatformSendWakeupPacket(mDNS *const m, mDNSInterfaceID InterfaceID, char *EthAddr, char *IPAddr, int iteration)
	{
	mDNSu32 ifindex;

	// Sanity check
	ifindex = mDNSPlatformInterfaceIndexfromInterfaceID(m, InterfaceID, mDNStrue);
	if (ifindex <= 0)
		{
		LogMsg("mDNSPlatformSendWakeupPacket: ERROR!! Invalid InterfaceID %u", ifindex);
		return;
		}
	mDNSSendWakeupPacket(ifindex, EthAddr, IPAddr, iteration);
	}

// Called for rr->InterfaceID == mDNSInterface_Any.  
// If current interface is P2P, verify that record is marked to IncludeP2P.
mDNSexport mDNSBool mDNSPlatformValidRecordForInterface(AuthRecord *rr, const NetworkInterfaceInfo *intf)
	{
	mDNSBool p2pInterface = (strncmp(intf->ifname, "p2p", 3) == 0);

	if (!p2pInterface || (rr->ARType == AuthRecordAnyIncludeP2P))
		return mDNStrue;
	else
		return mDNSfalse;
	}
