/*
 * Copyright (c) 2007 Apple Inc. All rights reserved.
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

#include <mach/mach.h>
#include <mach/mach_error.h>
#include <mach/vm_map.h>
#include <servers/bootstrap.h>
#include <IOKit/IOReturn.h>
#include <CoreFoundation/CoreFoundation.h>
#include "mDNSDebug.h"
#include "helper.h"
#include "helpermsg.h"

#define ERROR(x, y) y,
static const char *errorstring[] =
	{
	#include "helper-error.h"
	NULL
	};
#undef ERROR

static mach_port_t getHelperPort(int retry)
	{
	static mach_port_t port = MACH_PORT_NULL;
	if (retry) port = MACH_PORT_NULL;
	if (port == MACH_PORT_NULL && BOOTSTRAP_SUCCESS != bootstrap_look_up(bootstrap_port, kmDNSHelperServiceName, &port))
		LogMsg("%s: cannot contact helper", __func__);
	return port;
	}

const char *mDNSHelperError(int err)
	{
	static const char *p = "<unknown error>";
	if (mDNSHelperErrorBase < err && mDNSHelperErrorEnd > err)
		p = errorstring[err - mDNSHelperErrorBase - 1];
	return p;
	}

/* Ugly but handy. */
// We don't bother reporting kIOReturnNotReady because that error code occurs in "normal" operation
// and doesn't indicate anything unexpected that needs to be investigated

#define MACHRETRYLOOP_BEGIN(kr, retry, err, fin)                                            \
	for (;;)                                                                                \
		{
#define MACHRETRYLOOP_END(kr, retry, err, fin)												\
		if (KERN_SUCCESS == (kr)) break;													\
		else if (MACH_SEND_INVALID_DEST == (kr) && 0 == (retry)++) continue;				\
		else																				\
			{																				\
			(err) = kmDNSHelperCommunicationFailed;											\
			LogMsg("%s: Mach communication failed: %d %X %s", __func__, kr, kr, mach_error_string(kr));	\
			goto fin;																		\
			}																				\
		}																					\
	if (0 != (err) && kIOReturnNotReady != (err))											\
		{ LogMsg("%s: %d 0x%X (%s)", __func__, (err), (err), mDNSHelperError(err)); goto fin; }

void mDNSPreferencesSetName(int key, domainlabel *old, domainlabel *new)
	{
	kern_return_t kr = KERN_FAILURE;
	int retry = 0;
	int err = 0;
	char oldname[MAX_DOMAIN_LABEL+1] = {0};
	char newname[MAX_DOMAIN_LABEL+1] = {0};
	ConvertDomainLabelToCString_unescaped(old, oldname);
	if (new) ConvertDomainLabelToCString_unescaped(new, newname);

	MACHRETRYLOOP_BEGIN(kr, retry, err, fin);
	kr = proxy_mDNSPreferencesSetName(getHelperPort(retry), key, oldname, newname);
	MACHRETRYLOOP_END(kr, retry, err, fin);

fin:
	(void)err;
	}

void mDNSDynamicStoreSetConfig(int key, const char *subkey, CFPropertyListRef value)
	{
	CFWriteStreamRef stream = NULL;
	CFDataRef bytes = NULL;
	kern_return_t kr = KERN_FAILURE;
	int retry = 0;
	int err = 0;

	if (NULL == (stream = CFWriteStreamCreateWithAllocatedBuffers(NULL, NULL)))
		{
		err = kmDNSHelperCreationFailed;
		LogMsg("%s: CFWriteStreamCreateWithAllocatedBuffers failed", __func__);
		goto fin;
		}
	CFWriteStreamOpen(stream);
	if (0 == CFPropertyListWriteToStream(value, stream, kCFPropertyListBinaryFormat_v1_0, NULL))
		{
		err = kmDNSHelperPListWriteFailed;
		LogMsg("%s: CFPropertyListWriteToStream failed", __func__);
		goto fin;
		}
	if (NULL == (bytes = CFWriteStreamCopyProperty(stream, kCFStreamPropertyDataWritten)))
		{
		err = kmDNSHelperCreationFailed;
		LogMsg("%s: CFWriteStreamCopyProperty failed", __func__);
		goto fin;
		}
	CFWriteStreamClose(stream);
	CFRelease(stream);
	stream = NULL;
	MACHRETRYLOOP_BEGIN(kr, retry, err, fin);
	kr = proxy_mDNSDynamicStoreSetConfig(getHelperPort(retry), key, subkey ? subkey : "", (vm_offset_t)CFDataGetBytePtr(bytes), CFDataGetLength(bytes));
	MACHRETRYLOOP_END(kr, retry, err, fin);

fin:
	if (NULL != stream) { CFWriteStreamClose(stream); CFRelease(stream); }
	if (NULL != bytes) CFRelease(bytes);
	(void)err;
	}

void mDNSRequestBPF(void)
	{
	kern_return_t kr = KERN_FAILURE;
	int retry = 0, err = 0;
	MACHRETRYLOOP_BEGIN(kr, retry, err, fin);
	kr = proxy_mDNSRequestBPF(getHelperPort(retry));
	MACHRETRYLOOP_END(kr, retry, err, fin);
fin:
	(void)err;
	}

int mDNSPowerRequest(int key, int interval)
	{
	kern_return_t kr = KERN_FAILURE;
	int retry = 0, err = 0;
	MACHRETRYLOOP_BEGIN(kr, retry, err, fin);
	kr = proxy_mDNSPowerRequest(getHelperPort(retry), key, interval, &err);
	MACHRETRYLOOP_END(kr, retry, err, fin);
fin:
	return err;
	}

int mDNSSetLocalAddressCacheEntry(int ifindex, int family, const v6addr_t ip, const ethaddr_t eth)
	{
	kern_return_t kr = KERN_FAILURE;
	int retry = 0, err = 0;
	MACHRETRYLOOP_BEGIN(kr, retry, err, fin);
	kr = proxy_mDNSSetLocalAddressCacheEntry(getHelperPort(retry), ifindex, family, (uint8_t*)ip, (uint8_t*)eth, &err);
	MACHRETRYLOOP_END(kr, retry, err, fin);
fin:
	return err;
	}

void mDNSNotify(const char *title, const char *msg)	// Both strings are UTF-8 text
	{
	kern_return_t kr = KERN_FAILURE;
	int retry = 0, err = 0;
	MACHRETRYLOOP_BEGIN(kr, retry, err, fin);
	kr = proxy_mDNSNotify(getHelperPort(retry), title, msg);
	MACHRETRYLOOP_END(kr, retry, err, fin);
fin:
	(void)err;
	}

int mDNSKeychainGetSecrets(CFArrayRef *result)
	{
	CFPropertyListRef plist = NULL;
	CFDataRef bytes = NULL;
	kern_return_t kr = KERN_FAILURE;
	unsigned int numsecrets = 0;
	vm_offset_t secrets = 0;
	mach_msg_type_number_t secretsCnt = 0;
	int retry = 0, err = 0;

	MACHRETRYLOOP_BEGIN(kr, retry, err, fin);
	kr = proxy_mDNSKeychainGetSecrets(getHelperPort(retry), &numsecrets, &secrets, &secretsCnt, &err);
	MACHRETRYLOOP_END(kr, retry, err, fin);

	if (NULL == (bytes = CFDataCreateWithBytesNoCopy(kCFAllocatorDefault, (void*)secrets, secretsCnt, kCFAllocatorNull)))
		{
		err = kmDNSHelperCreationFailed;
		LogMsg("%s: CFDataCreateWithBytesNoCopy failed", __func__);
		goto fin;
		}
	if (NULL == (plist = CFPropertyListCreateFromXMLData(kCFAllocatorDefault, bytes, kCFPropertyListImmutable, NULL)))
		{
		err = kmDNSHelperInvalidPList;
		LogMsg("%s: CFPropertyListCreateFromXMLData failed", __func__);
		goto fin;
		}
	if (CFArrayGetTypeID() != CFGetTypeID(plist))
		{
		err = kmDNSHelperTypeError;
		LogMsg("%s: Unexpected result type", __func__);
		CFRelease(plist);
		plist = NULL;
		goto fin;
		}
	*result = (CFArrayRef)plist;

fin:
	if (bytes) CFRelease(bytes);
	if (secrets) vm_deallocate(mach_task_self(), secrets, secretsCnt);
	return err;
	}

void mDNSAutoTunnelInterfaceUpDown(int updown, v6addr_t address)
	{
	kern_return_t kr = KERN_SUCCESS;
	int retry = 0, err = 0;
	MACHRETRYLOOP_BEGIN(kr, retry, err, fin);
	kr = proxy_mDNSAutoTunnelInterfaceUpDown(getHelperPort(retry), updown, address);
	MACHRETRYLOOP_END(kr, retry, err, fin);
fin:
	(void)err;
	}

extern const char dnsprefix[];

void mDNSConfigureServer(int updown, const char *const prefix, const domainname *const fqdn)
	{
	kern_return_t kr = KERN_SUCCESS;
	int retry = 0, err = 0;
	char fqdnStr[MAX_ESCAPED_DOMAIN_NAME + 10] = { 0 }; // Assume the prefix is no larger than 10 chars
	if (fqdn)
		{
		mDNSPlatformStrCopy(fqdnStr, prefix);
		if (ConvertDomainNameToCString(fqdn, fqdnStr + mDNSPlatformStrLen(prefix)) && prefix == dnsprefix)
			{
			// remove the trailing dot, as that is not used in the keychain entry racoon will lookup
			mDNSu32 fqdnEnd = mDNSPlatformStrLen(fqdnStr);
			if (fqdnEnd) fqdnStr[fqdnEnd - 1] = 0;
			}
		}
	MACHRETRYLOOP_BEGIN(kr, retry, err, fin);
	kr = proxy_mDNSConfigureServer(getHelperPort(retry), updown, fqdnStr);
	MACHRETRYLOOP_END(kr, retry, err, fin);
fin:
	(void)err;
	}

int mDNSAutoTunnelSetKeys(int replacedelete, v6addr_t local_inner,
    v6addr_t local_outer, short local_port, v6addr_t remote_inner,
    v6addr_t remote_outer, short remote_port, const char* const prefix, const domainname *const fqdn)
	{
	kern_return_t kr = KERN_SUCCESS;
	int retry = 0, err = 0;
	char fqdnStr[MAX_ESCAPED_DOMAIN_NAME + 10] = { 0 }; // Assume the prefix is no larger than 10 chars
	if (fqdn)
		{
		mDNSPlatformStrCopy(fqdnStr, prefix);
		if (ConvertDomainNameToCString(fqdn, fqdnStr + mDNSPlatformStrLen(prefix)) && prefix == dnsprefix)
			{
			// remove the trailing dot, as that is not used in the keychain entry racoon will lookup
			mDNSu32 fqdnEnd = mDNSPlatformStrLen(fqdnStr);
			if (fqdnEnd) fqdnStr[fqdnEnd - 1] = 0;
			}
		}
	MACHRETRYLOOP_BEGIN(kr, retry, err, fin);
	kr = proxy_mDNSAutoTunnelSetKeys(getHelperPort(retry), replacedelete, local_inner, local_outer, local_port, remote_inner, remote_outer, remote_port, fqdnStr, &err);
	MACHRETRYLOOP_END(kr, retry, err, fin);
fin:
	return err;
	}

void mDNSSendWakeupPacket(unsigned ifid, char *eth_addr, char *ip_addr, int iteration)
	{
	kern_return_t kr = KERN_SUCCESS;
	int retry = 0, err = 0;
	MACHRETRYLOOP_BEGIN(kr, retry, err, fin);
	kr = proxy_mDNSSendWakeupPacket(getHelperPort(retry), ifid, eth_addr, ip_addr, iteration);
	MACHRETRYLOOP_END(kr, retry, err, fin);
fin:
	(void) err;
	}

void mDNSPacketFilterControl(uint32_t command, char * ifname, uint16_t servicePort, uint16_t protocol)
	{
	kern_return_t kr = KERN_SUCCESS;
	int retry = 0, err = 0;
	MACHRETRYLOOP_BEGIN(kr, retry, err, fin);
	kr = proxy_mDNSPacketFilterControl(getHelperPort(retry), command, ifname, servicePort, protocol);
	MACHRETRYLOOP_END(kr, retry, err, fin);
fin:
	(void) err;
	}
