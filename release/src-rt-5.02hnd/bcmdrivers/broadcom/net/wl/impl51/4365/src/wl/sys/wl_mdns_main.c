
/*
 * Copyright (C) 2017, Broadcom. All Rights Reserved.
 * 
 * This source code was modified by Broadcom. It is distributed under the
 * original license terms described below.
 *
 * Copyright (c) 2002-2006 Apple Computer, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */
/*
 * $Id: wl_mdns_main.c 470212 2014-04-14 17:41:11Z $
 *
 * mdns_sd_main.c
 *
 * Multicast DNS Service Discovery offload main routines.
 *
 */
 
 
/* FILE-CSTYLED */

#define USE_IPv6_to_6		/* Undo an odd v6 to v4 conversion in original eth code */
#define MCAST_FILTER		/* Filter out our own packets that might be reflected back */

#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmendian.h>
#include <bcmutils.h>
#include <siutils.h>
#include <wlioctl.h>
#include <d11.h>
#include <wlc_pub.h>
#include <wlc_rate.h>
#include <wlc.h>
#include <wlc_bmac.h>
#include <wl_export.h>

#include <wl_export.h>
#include <wlc_wowlol.h>
#ifdef BCM_OL_DEV
#include <wlc_types.h>
#include <bcm_ol_msg.h>
#include <wlc_hw_priv.h>
#include <wlc_dngl_ol.h>
#else
#include <wlc_bsscfg.h>
#endif

#include "wl_mdns_common.h"
#include "wl_mdns_main.h"
#include "wl_mdns.h"

#define MDNS_DEFAULT_TIME	100 	/* 100 millisecs */

mdns_share_mem_block_t *mdns_share_blk;
wlc_dngl_ol_mdns_info_t *glob_mdnsi;

/* mdns private info structure */
struct wlc_dngl_ol_mdns_info {
	wlc_dngl_ol_info_t      *wlc_dngl_ol;
	wlc_if_t        	*wlcif;
	osl_t			*osh;
	uint32			magic;
	bool			dns_enabled;
	u8 			glob_txPktBuff[2048];
	void  			*rx_dataptr;
	uint16			rx_datalen;
	uchar 			NicMacAddr[8];
	bool			sent_wakeup;
	struct wl_timer 	*mdns_execute_timer;
	mdns_share_mem_block_t 	mdns_share_blk_struct;
	mDNSBool 		mDnsGratuitousArpSent;
	u32 			seed;
};

#define DNS_INFINITY	0x78000000

/* Table 6 of Firmare Interface Spec */
typedef struct {
	u32 signature;
	u16 maxIpv4Addr;
	u16 maxIpv6Addr;
	u16 maxIpv6SolAddr;
	u16 maxServices;
	u16 dataSize;
	uint8 * dataAddr;
} PACKED_STRUCT bonjourProxyIF_t;

/* Brett:  Firmware uses this to advertise its capabilities to
 * the host. MDNS_SD_DRV_INTF_ADDR is where the host should
 * start stashing data (in format of Table 7).
*/
bonjourProxyIF_t bonjourProxyInfo =
{
	BONJOUR_PROXY_SIG,
	MAX_MDNS_IPV4_ADDRESSES,
	MAX_MDNS_IPV6_ADDRESSES,
	MAX_MDNS_IPV6_SOL_ADDRESSES,
	MAX_MDNS_SD_PROXY_SERVICES,
	MDNS_SD_DRV_INTF_SIZE,
	MDNS_SD_DRV_INTF_ADDR
};

static void wl_mdns_wake(uint32 reason);
static void *mDNSPlatformMemAllocate(mDNSu32 inSize);
void mDNSPlatformMemFree(void *addr,  mDNSu32 inSize);
static u32 simple_mult(u32 oper1, u32 oper2);
static u8 *AppendDNSNameString(domainname *name, char *cstring);
static u8 *AppendDomainName(domainname *name, domainname *append);
static u8 *MakeDomainNameFromDNSNameString(domainname *name, char *cstr);
static bool ValidateRData(u16 rrtype, u16 rdlength, RData *rd);
static u8 *ConstructServiceName(domainname *fqdn, domainlabel *name,
	domainname *type, domainname *domain);
static mDNSBool DeconstructServiceName(domainname *fqdn, domainlabel *name,
	domainname *type, domainname *domain, bool verbose, mDNSIPPort port, u32 numOfSubTypes);
static mStatus mDNS_Register_internal(AuthRecord *rr);
static mStatus mDNS_RegisterService(domainlabel *name, domainname *type, domainname *domain,
	mDNSIPPort port, u8 txtinfo[], u16 txtlen, u32 ttl, domainname *SubTypes, u32 NumSubTypes);
static void AdvertiseInterface(NetworkInterfaceInfo *set);
mDNSOpaque16 mDNSOpaque16fromIntVal(mDNSu16 v);
static void mDNSPlatformInit(wlc_dngl_ol_mdns_info_t *mdns, proxy_services_t *proxy_services);

/* proto.h stuff */
static void setupMacHeader(pMAC_HDR pkt, u8 *srcMacAddr, u8 *destMacAddr, u16 frame_type);
static void setupIpHeader(pTXPORT_HDR pkt, u8 *srcIpAddr, u8 *destIpAddr, u8 ttl,
	u8 upper_protocol, u16 frameId, u16 pktLen);
static void setupUdpHeader(void *phdr, void *pdata, u8 ipVer, u8 *srcIp, u8 *destIp,
	u16 srcPort, u16 destPort, u16 udpLen);
static void setupArpPacket(u8 *packet, u16 opcode, u8 *srcMacAddr, u8 *destMacAddr,
	u8 *srcIpAddr, u8 *destIpAddr);
static void setupIpV6Header(ipV6Hdr_t *pkt, u8 *srcIpAddr, u8 *destIpAddr, u8 hopLimit,
	u8 next_hdr, u16 pktLen);

/* cpub.c */
static s32 mDNSPlatformRawTime(void);
static void mDNS_Lock(void);
static s32 GetNextScheduledEvent(void);
static void mDNS_Unlock(void);
static u32 mDNSPlatformRandomSeed(void);
static u32 mDNSRandom(u32 max);
static mDNSu8 *skipDomainName(DNSMessage *msg, mDNSu8 *ptr, mDNSu8 *end);
static mDNSu8 *skipQuestion(DNSMessage *msg, mDNSu8 *ptr, mDNSu8 *end);
static mDNSu8 *LocateAnswers(DNSMessage *msg, mDNSu8 *end);
static u8 *FindCompressionPointer(u8 *base, u8 *end, u8 *domname);

extern int generic_send_packet(wlc_dngl_ol_info_t *ol_info, uchar *params, uint p_len);

static void
wl_mdns_wake(uint32 reason)
{

	if (glob_mdnsi->sent_wakeup) {
		return;
	}

	WL_ERROR(("%s: Waking host, reason: ", __FUNCTION__));
	switch (reason) {
		case WL_WOWL_MDNS_CONFLICT:
			WL_ERROR(("MDNS Conflict Resolution "));
			break;
		case WL_WOWL_MDNS_SERVICE:
			WL_ERROR(("MDNS Service Usage "));
			break;
	}
	WL_ERROR(("(0x%x)\n", reason));

	/* Pass up saved pkt to host */
	if (glob_mdnsi->rx_dataptr && glob_mdnsi->rx_datalen) {

		glob_mdnsi->sent_wakeup = TRUE;
		wlc_wowl_ol_wake_host(glob_mdnsi->wlc_dngl_ol->wowl_ol,
			NULL, 0, glob_mdnsi->rx_dataptr,
			glob_mdnsi->rx_datalen, reason);
	}

}

u32 simple_mult(u32 oper1, u32 oper2)
{
	return (oper1 * oper2);
}

void *mDNSPlatformMemAllocate(mDNSu32 inSize)
{
	void *foo;
	foo = MALLOC(glob_mdnsi->osh, inSize);
	if (!foo)
		WL_ERROR(("%s: MALLOC of %d bytes failed\n", __FUNCTION__, inSize));
	return foo;
}
void mDNSPlatformMemFree(void *addr,  mDNSu32 inSize)
{
	MFREE(glob_mdnsi->osh, addr, inSize);
}

/* AppendDNSNameString appends zero or more labels to an existing (possibly empty) domainname.
 * The C string is in conventional DNS syntax:
 * Textual labels, escaped as necessary using the usual DNS '\' notation, separated by dots.
 * If successful, AppendDNSNameString returns a pointer to the next unused byte
 * in the domainname bufer (i.e., the next byte after the terminating zero).
 * If unable to construct a legal domain name (i.e. label more than 63 bytes,
 * or total more than 255 bytes) AppendDNSNameString returns mDNSNULL.
 */
u8 *AppendDNSNameString(domainname *name, char *cstring)
{
	char	*cstr   = cstring;
	u8    	*ptr 	= name->c + DomainNameLength(name) - 1;	/* Find end of current name */
	u8 	*lim 	= name->c + MAX_DOMAIN_NAME - 1;	/* Limit of how much we can add */
								/* (not counting final zero) */

	while (*cstr && ptr < lim) /* While more characters, and space to put them... */
	{
		u8 *lengthbyte = ptr++; /* Record where the length is going to go */

		if (*cstr == '.') return (mDNSNULL);
		while (*cstr && *cstr != '.' && ptr < lim) {
			/* While we have characters in the label... */
			u8 c = (u8)*cstr++;	/* Read the character */
			if (c == '\\')		/* If escape character, check next character */
			{
				c = (u8)*cstr++; /* Assume we'll just take the next character */

				/* If three decimal digits, interpret as three-digit decimal */
				if (mdnsIsDigit(cstr[-1]) &&
				    mdnsIsDigit(cstr[0]) && mdnsIsDigit(cstr[1])) {
					int v0 = cstr[-1] - '0';
					int v1 = cstr[ 0] - '0';
					int v2 = cstr[ 1] - '0';
					int val = v0 * 100 + v1 * 10 + v2;
					if (val <= 255) {
						/* If valid three-digit decimal value, use it */
						c = (u8)val; cstr += 2;
					}
				}
			}
			*ptr++ = c;		/* Write the character */
		}

		if (*cstr) cstr++;		/* Skip over the trailing dot (if present) */
		if (ptr - lengthbyte - 1 > MAX_DOMAIN_LABEL)	/* If illegal label, abort */
			return (mDNSNULL);
		*lengthbyte = (u8)(ptr - lengthbyte - 1);	/* Fill in the length byte */
	}

	*ptr++ = 0;			/* Put the null root label on the end */
	if (*cstr) return (mDNSNULL);	/* Failure: We didn't successfully consume all input */
	else return (ptr);		/* Success: return new value of ptr */
}

u8 *AppendDomainName(domainname *name, domainname *append)
{
	u8 *ptr = name->c + DomainNameLength(name) - 1;	/* Find end of current name */
	u8 *lim = name->c + MAX_DOMAIN_NAME - 1; /* how much we can add (not counting final zero) */
	u8 *src = append->c;

	while (src[0]) {
		int i;
		if (ptr + 1 + src[0] > lim) return (mDNSNULL);
		for (i = 0; i <= src[0]; i++) *ptr++ = src[i];
		*ptr = 0;	/* Put the null root label on the end */
		src += i;
	}
	return (ptr);
}

/* MakeDomainNameFromDNSNameString makes a native DNS-format domainname from a C string.
 * The C string is in conventional DNS syntax:
 * Textual labels, escaped as necessary using the usual DNS '\' notation, separated by dots.
 * If successful, MakeDomainNameFromDNSNameString returns a pointer to the next unused byte
 * in the domainname bufer (i.e., the next byte after the terminating zero).
 * If unable to construct a legal domain name (i.e. label more than 63 bytes, or
 * total more than 255 bytes) MakeDomainNameFromDNSNameString returns mDNSNULL.
 */
u8 *MakeDomainNameFromDNSNameString(domainname *name, char *cstr)
{
	name->c[0] = 0;					/* Make an empty domain name */
	return (AppendDNSNameString(name, cstr));	/* And then add this string to it */
}

bool ValidateRData(u16 rrtype, u16 rdlength, RData *rd)
{
	u16 len;

	switch (rrtype)
	{
		case kDNSType_A:	return (rdlength == sizeof(mDNSv4Addr));
		case kDNSType_AAAA:	return (rdlength == sizeof(mDNSv6Addr));

		case kDNSType_NS:	/* Same as PTR */
		case kDNSType_MD:	/* Same as PTR */
		case kDNSType_MF:	/* Same as PTR */
		case kDNSType_CNAME:	/* Same as PTR */
		/* case kDNSType_SOA not checked */
		case kDNSType_MB:	/* Same as PTR */
		case kDNSType_MG:	/* Same as PTR */
		case kDNSType_MR:	/* Same as PTR */
		/* case kDNSType_NULL not checked (no specified format, so always valid) */
		/* case kDNSType_WKS not checked */
		case kDNSType_PTR:	if (!rdlength) return (mDNSfalse);
			len = DomainNameLength(rd->u.name);
			return (len <= MAX_DOMAIN_NAME && rdlength == len);

		case kDNSType_HINFO:	/* Same as TXT (roughly) */
		case kDNSType_TXT:
			/* TXT record has to be at least one byte (RFC 1035) */
			if (!rdlength) return (mDNSfalse);
			{
				mDNSu8 *ptr = rd->u.txt->c;
				mDNSu8 *end = rd->u.txt->c + rdlength;
				while (ptr < end) ptr += 1 + ptr[0];
				return (ptr == end);
			}

		case kDNSType_SRV:
			if (!rdlength) return (mDNSfalse);
			len = DomainNameLength(rd->u.srv->target);
			return (len <= MAX_DOMAIN_NAME && rdlength == 6+len);

		default:
			return (mDNStrue);	/* Allow all other types without checking */
	}
}

u8 *ConstructServiceName(domainname *fqdn, domainlabel *name, domainname *type, domainname *domain)
{
	int	i, len;
	u8 	*dst = fqdn->c;
	u8 	*src;

	/* In the case where there is no name (and ONLY in that case), */
	/* a single-label subtype is allowed as the first label */
	/* of a three-part "type" */
	if (!name && type)
	{
		u8 *s0 = type->c;
		/* If legal first label (at least one character, and no more than 63) */
		if (s0[0] && s0[0] < 0x40)
		{
			 u8 * s1 = s0 + 1 + s0[0];
			/* and legal second label (at least one character, and no more than 63) */
			if (s1[0] && s1[0] < 0x40)
			{
				 u8 *s2 = s1 + 1 + s1[0];
				/* and we have three and only three labels */
				if (s2[0] && s2[0] < 0x40 && s2[1+s2[0]] == 0)
				{
					static  u8 SubTypeLabel[5] = "\x04_sub";
					src = s0;	/* Copy the first label */
					len = *src;
					for (i = 0; i <= len; i++) *dst++ = *src++;
					for (i = 0; i < (int)sizeof(SubTypeLabel); i++)
						*dst++ = SubTypeLabel[i];
					type = (domainname *)s1;

					/* Special support for queries done by some */
					/* third-party network monitoring software For */
					/* these queries, we retract the "._sub" we just */
					/* added between the subtype and the main type */
					if (SameDomainName((domainname*)s0,
						(domainname*)"\x09_services\x07_dns-sd\x04_udp") ||
						SameDomainName((domainname*)s0,
						(domainname*)"\x09_services\x05_mdns\x04_udp"))
							dst -= sizeof(SubTypeLabel);
				}
			}
		}
	}

	if (name && name->c[0])
	{
		src = name->c; /* Put the service name into the domain name */
		len = *src;
		if (len >= 0x40) goto fail;
		for (i = 0; i <= len; i++) *dst++ = *src++;
	} else {
		/* Set this up to be non-null, to avoid errors if we have to call LogMsg() below */
		name = (domainlabel*)"";
	}

	/* Put the service type into the domain name */
	src = type->c;
	len = *src;
	if (len < 2 || len >= 0x40 || (len > 15 && !SameDomainName(domain, &localdomain)))
		goto fail;
	if (src[1] != '_') goto fail;
	for (i = 2; i <= len; i++)
		if (!mdnsIsLetter(src[i]) && !mdnsIsDigit(src[i]) && src[i] != '-' && src[i] != '_')
			goto fail;
	for (i = 0; i <= len; i++) *dst++ = *src++;

	len = *src;
	if (!(len == 4 && src[1] == '_' &&
		(((src[2] | 0x20) == 'u' && (src[3] | 0x20) == 'd') ||
		((src[2] | 0x20) == 't' && (src[3] | 0x20) == 'c')) &&
		(src[4] | 0x20) == 'p'))
		goto fail;
	for (i = 0; i <= len; i++) *dst++ = *src++;

	if (*src) goto fail;

	*dst = 0;
	if (!domain->c[0]) goto fail;
	if (SameDomainName(domain, (domainname*)"\x05" "local" "\x04" "arpa"))
		goto fail;
	dst = AppendDomainName(fqdn, domain);
	if (!dst) goto fail;

	return (dst);

fail:
	return (mDNSNULL);
}

/* A service name has the form: instance.application-protocol.transport-protocol.domain */
/* DeconstructServiceName is currently fairly forgiving: It doesn't try to enforce character */
/* set or length limits for the protocol names, and the final domain is allowed to be empty. */
/* However, if the given FQDN doesn't contain at least three labels, */
/* DeconstructServiceName will reject it and return mDNSfalse. */
mDNSBool DeconstructServiceName(domainname *fqdn, domainlabel *name,
	domainname *type, domainname *domain, bool verbose, mDNSIPPort port, u32 numOfSubTypes)
{
	int i, len;
	mDNSu8 *src = fqdn->c;
	mDNSu8 *dst;
	u16 *portp = (u16 *)&port.b;
	UNUSED_PARAMETER(portp);

	dst = name->c;			/* Extract the service name */
	len = *src;
	if (verbose) WL_ERROR(("mDNS_Register: Subtypes %d Name: ", numOfSubTypes));
	for (i = 0; i <= len; i++) {
		if (i && verbose)
			WL_ERROR(("%c", *src));
		*dst++ = *src++;
	}

	if (verbose) WL_ERROR((" Type: "));
	dst = type->c;			/* Extract the service type */
	len = *src;
	for (i = 0; i <= len; i++) {
		if (i && verbose)
			WL_ERROR(("%c", *src));
		*dst++ = *src++;
	}
	if (verbose) WL_ERROR(("."));

	/* Keep copying data onto 'type' */
	len = *src;
	for (i = 0; i <= len; i++) {
		if (i && verbose)
			WL_ERROR(("%c", *src));
		*dst++ = *src++;
	}
	*dst++ = 0;			/* Put terminator on the end of service type */

	/* Now do domain */
	if (verbose) WL_ERROR((" Domain: "));
	dst = domain->c;		/* Extract the service domain */
	while (*src)
	{
		len = *src;
		for (i = 0; i <= len; i++) {
			if (i && verbose)
				WL_ERROR(("%c", *src));
			*dst++ = *src++;
		}
		if (verbose && *src) WL_ERROR(("."));
	}
	*dst++ = 0;		/* Put the null root label on the end */
	if (verbose) WL_ERROR((" Port: %d\n", ntoh16(*portp)));

	return (mDNStrue);
}

/* Two records qualify to be local duplicates if the RecordTypes are the same, or if one */
/* is Unique and the other Verified */
#define RecordLDT(A, B) ((A)->resrec.RecordType == (B)->resrec.RecordType || \
	((A)->resrec.RecordType | (B)->resrec.RecordType) == \
	(kDNSRecordTypeUnique | kDNSRecordTypeVerified))
#define RecordIsLocalDuplicate(A, B) \
	(RecordLDT((A), (B)) && IdenticalResourceRecord(&(A)->resrec, &(B)->resrec))

mStatus mDNS_Register_internal(AuthRecord *rr)
{
	AuthRecord **p = &mdns_share_blk->m->ResourceRecords;

	while (*p && *p != rr) p = &(*p)->next;
	if (*p)
		return (mStatus_AlreadyRegistered);

	if (rr->DependentOn)
	{
		if (rr->resrec.RecordType == kDNSRecordTypeUnique)
			rr->resrec.RecordType =  kDNSRecordTypeVerified;
		else
			return (mStatus_Invalid);

		if (!(rr->DependentOn->resrec.RecordType &
			(kDNSRecordTypeUnique | kDNSRecordTypeVerified)))
			return (mStatus_Invalid);
	}

	rr->next = mDNSNULL;
	rr->resrec.rdlength   = GetRDLength(&rr->resrec, mDNSfalse);
	rr->resrec.rdestimate = GetRDLength(&rr->resrec, mDNStrue);

	if (!ValidateDomainName(rr->resrec.name))
		return (mStatus_Invalid);

	/* BIND named (name daemon) doesn't allow TXT records with zero-length rdata.
	   This is strictly speaking correct, since RFC 1035 specifies a TXT record as
	   "One or more <character-string>s", not "Zero or more <character-string>s".

	   Since some legacy apps try to create zero-length TXT records, we'll silently
	   correct it here.
	*/
	if (rr->resrec.rrtype == kDNSType_TXT && rr->resrec.rdlength == 0)
	{
		rr->resrec.rdlength = 1;
		rr->resrec.rdata->u.txt->c[0] = 0;
	}

	/* Don't do this until *after* we've set rr->resrec.rdlength */
	if (!ValidateRData(rr->resrec.rrtype, rr->resrec.rdlength, rr->resrec.rdata))
		return (mStatus_Invalid);

	if (!mdns_share_blk->m->NewLocalRecords)
		mdns_share_blk->m->NewLocalRecords = rr;

	*p = rr;
	return (mStatus_NoError);
}

/* Note:
 * Name is first label of domain name (any dots in the name are actual dots,
 *	not label separators)
 * Type is service type (e.g. "_ipp._tcp.")
 * Domain is fully qualified domain name (i.e. ending with a null label)
 * We always register a TXT, even if it is empty (so that clients are not
 * left waiting forever looking for a nonexistent record.)
 * If the host parameter is mDNSNULL or the root domain (ASCII NUL),
 * then the default host name (m->MulticastHostname) is automatically used.
 */
mStatus mDNS_RegisterService(domainlabel *name, domainname *type, domainname *domain,
	mDNSIPPort port, u8 txtinfo[], u16 txtlen, u32 ttl, domainname *SubTypes, u32 NumSubTypes)
{
	ServiceRecordSet	*sr;
	domainname		buffer;
	u32			size;
	int			i;

	mStatus err = mStatus_NoError;

	sr = (ServiceRecordSet *)mDNSPlatformMemAllocate(sizeof(ServiceRecordSet));
	memset(sr, 0, sizeof(ServiceRecordSet));
	sr->next = mdns_share_blk->mdns_sr;
	mdns_share_blk->mdns_sr = sr;

	/* Initialize the AuthRecord objects to sane values */
	mDNS_SetupResourceRecord(&sr->RR_ADV, kDNSType_PTR, kStandardTTL, kDNSRecordTypeAdvisory);
	mDNS_SetupResourceRecord(&sr->RR_PTR, kDNSType_PTR, kStandardTTL, kDNSRecordTypeShared);
	mDNS_SetupResourceRecord(&sr->RR_SRV, kDNSType_SRV, ttl, kDNSRecordTypeVerified);
	mDNS_SetupResourceRecord(&sr->RR_TXT, kDNSType_TXT, kStandardTTL, kDNSRecordTypeUnique);

	sr->RR_SRV.resrec.rdata->u.srv = (rdataSRV *)mDNSPlatformMemAllocate(sizeof(rdataSRV));
	sr->RR_SRV.resrec.rdata->MaxRDLength = sizeof(rdataSRV_t);

	sr->RR_TXT.resrec.rdata->u.txt = (UTF8str255 *)mDNSPlatformMemAllocate(txtlen+1);
	sr->RR_TXT.resrec.rdata->MaxRDLength = txtlen+1;
	sr->RR_TXT.DependentOn = &sr->RR_SRV;

	/* If the client is registering an oversized TXT record,
	 * it is the client's responsibility to alloate a ServiceRecordSet structure
	 * that is large enough for it.
	 */
	if (sr->RR_TXT.resrec.rdata->MaxRDLength < txtlen)
		sr->RR_TXT.resrec.rdata->MaxRDLength = txtlen;

	/* Set up the record names
	 * For now we only create an advisory record for the main type, not for subtypes
	 * We need to gain some operational experience before we decide if there's a need
	 * to create them for subtypes too.
	 */
	if (ConstructServiceName(&buffer, (domainlabel*)"\x09_services",
		(domainname*)"\x07_dns-sd\x04_udp", domain) == mDNSNULL)
		return (mStatus_BadParamErr);
	size = DomainNameLength(&buffer);
	sr->RR_ADV.resrec.name = mDNSPlatformMemAllocate(size);
	memcpy(sr->RR_ADV.resrec.name->c, buffer.c, size);

	if (ConstructServiceName(&buffer, mDNSNULL, type, domain) == mDNSNULL)
		return (mStatus_BadParamErr);
	size = DomainNameLength(&buffer);
	sr->RR_PTR.resrec.name = mDNSPlatformMemAllocate(size);
	memcpy(sr->RR_PTR.resrec.name->c, buffer.c, size);

	if (ConstructServiceName(&buffer, name, type, domain) == mDNSNULL)
		return (mStatus_BadParamErr);
	size = DomainNameLength(&buffer);
	sr->RR_SRV.resrec.name = mDNSPlatformMemAllocate(size);
	memcpy(sr->RR_SRV.resrec.name->c, buffer.c, size);

	sr->RR_TXT.resrec.name = sr->RR_SRV.resrec.name;

	/* 1. Set up the ADV record rdata to advertise our service type */
	sr->RR_ADV.resrec.rdata->u.name = sr->RR_PTR.resrec.name;

	/* 2. Set up the PTR record rdata to point to our service name We set up two */
	/* additionals, so when a client asks for this PTR we automatically send the SRV */
	/* and the TXT too */
	sr->RR_PTR.resrec.rdata->u.name = sr->RR_SRV.resrec.name;

	sr->RR_PTR.Additional1 = &sr->RR_SRV;
	sr->RR_PTR.Additional2 = &sr->RR_TXT;

	/* 2a. Set up any subtype PTRs to point to our service name */
	/* If the client is using subtypes, it is the client's responsibility to have */
	/* already set the first label of the record name to the subtype being registered */
	if (NumSubTypes)
	{
		mDNSu8 		*ptr;
		mDNSu8 		*end;

		sr->NumSubTypes = NumSubTypes;
		sr->SubTypes =
			(AuthRecord *)mDNSPlatformMemAllocate(NumSubTypes * sizeof(AuthRecord));
		memset(sr->SubTypes, 0, NumSubTypes * sizeof(AuthRecord));

		ptr = (mDNSu8 *)SubTypes;
		end = ptr + (NumSubTypes * sizeof(domainname));
		for (i = 0; i < NumSubTypes; i++)
		{
			domainname	st;

			ptr = getDomainName((DNSMessage *)SubTypes, ptr, end, &st);
			if (!ptr)
				continue;

			AppendDomainName(&st, type);
			mDNS_SetupResourceRecord(&sr->SubTypes[i], kDNSType_PTR,
				kStandardTTL, kDNSRecordTypeShared);
			if (ConstructServiceName(&buffer, mDNSNULL, &st, domain) == mDNSNULL)
				return (mStatus_BadParamErr);

			size = DomainNameLength(&buffer);
			sr->SubTypes[i].resrec.name = mDNSPlatformMemAllocate(size);
			memcpy(sr->SubTypes[i].resrec.name->c, buffer.c, size);
			sr->SubTypes[i].resrec.rdata->u.name = sr->RR_SRV.resrec.name;
			sr->SubTypes[i].Additional1 = &sr->RR_SRV;
			sr->SubTypes[i].Additional2 = &sr->RR_TXT;
		}
	}

	/* 3. Set up the SRV record rdata. */
	sr->RR_SRV.resrec.rdata->u.srv->priority = 0;
	sr->RR_SRV.resrec.rdata->u.srv->weight   = 0;
	sr->RR_SRV.resrec.rdata->u.srv->port     = port;

	sr->RR_SRV.resrec.rdata->u.srv->target = &mdns_share_blk->m->MulticastHostname;

	/* 4. Set up the TXT record rdata, and set DependentOn because we're depending on */
	/* the SRV record to find and resolve conflicts for us */
	if (txtinfo == mDNSNULL) sr->RR_TXT.resrec.rdlength = 0;
	else if (txtinfo != sr->RR_TXT.resrec.rdata->u.txt->c)
	{
		sr->RR_TXT.resrec.rdlength = txtlen;
		if (sr->RR_TXT.resrec.rdlength > sr->RR_TXT.resrec.rdata->MaxRDLength)
			return (mStatus_BadParamErr);
		memcpy(sr->RR_TXT.resrec.rdata->u.txt->c, txtinfo, txtlen);
	}

	err = mDNS_Register_internal(&sr->RR_PTR);
	if (!err) err = mDNS_Register_internal(&sr->RR_SRV);
	if (!err) err = mDNS_Register_internal(&sr->RR_TXT);
	if (!err) err = mDNS_Register_internal(&sr->RR_ADV);
	for (i = 0; i < NumSubTypes; i++)
		if (!err)
			err = mDNS_Register_internal(&sr->SubTypes[i]);

	return (err);
}

void AdvertiseInterface(NetworkInterfaceInfo *set)
{
	char 		buffer[MAX_REVERSE_MAPPING_NAME];
	domainname 	domainBuffer;
	u32			nameLen;
	int 		i;
	int 		div;
	int 		val;
	int 		idx = 0;
	int 		digit;

	/*
	set->next = mdns_intf;
	*/
	mdns_share_blk->mdns_intf = set;

	WL_TRACE(("%s\n", __FUNCTION__));
	/* Send dynamic update for non-linklocal IPv4 Addresses */
	mDNS_SetupResourceRecord(&set->RR_A,   kDNSType_A,
		kHostNameTTL, kDNSRecordTypeVerified);
	mDNS_SetupResourceRecord(&set->RR_PTR, kDNSType_PTR,
		kHostNameTTL, kDNSRecordTypeKnownUnique);

	/* 1. Set up Address record to map from host name ("foo.local.") to IP address */
	/* 2. Set up reverse-lookup PTR record to map from our address back to our host name */
	set->RR_A.resrec.name = &mdns_share_blk->m->MulticastHostname;
	set->RR_PTR.resrec.rdata->u.name = &mdns_share_blk->m->MulticastHostname;

	if (set->ip.type == mDNSAddrType_IPv4)
	{
		set->RR_A.resrec.rrtype = kDNSType_A;
		set->RR_A.resrec.rdata->u.ipv4 =
			(mDNSv4Addr *)mDNSPlatformMemAllocate(sizeof(mDNSv4Addr));
		set->RR_A.resrec.rdata->u.ipv4->NotAnInteger = set->ip.ip.v4.NotAnInteger;
		for (i = 0; i < 4; i++)
		{
			val = set->ip.ip.v4.b[3 - i];
			for (div = 100; div > 0;)
			{
				if (val == 0) {
					buffer[idx++] = '0';
					break;
				}
				else {
					digit = simple_divide(val, div);
					if (digit) {
						buffer[idx++] = digit + '0';
						val -= simple_mult(digit, div);
					}
				}

				div = simple_divide(div, 10);
			}

			buffer[idx++] = '.';
		}
		strncpy(&buffer[idx], "in-addr.arpa.", 14);
	}
	else if (set->ip.type == mDNSAddrType_IPv6)
	{
		set->RR_A.resrec.rrtype = kDNSType_AAAA;
		set->RR_A.resrec.rdata->u.ipv6 =
			(mDNSv6Addr *)mDNSPlatformMemAllocate(sizeof(mDNSv6Addr));
		memcpy(set->RR_A.resrec.rdata->u.ipv6, &set->ip.ip.v6, sizeof(mDNSv6Addr));

		for (i = 0; i < 16; i++)
		{
			static const char hexValues[] = "0123456789ABCDEF";
			buffer[i * 4    ] = hexValues[set->ip.ip.v6.b[15 - i] & 0x0F];
			buffer[i * 4 + 1] = '.';
			buffer[i * 4 + 2] = hexValues[set->ip.ip.v6.b[15 - i] >> 4];
			buffer[i * 4 + 3] = '.';
		}
		strncpy(&buffer[64], "ip6.arpa.", 10);
	}

	MakeDomainNameFromDNSNameString(&domainBuffer, buffer);
	nameLen = DomainNameLength(&domainBuffer);
	set->RR_PTR.resrec.name = mDNSPlatformMemAllocate(nameLen);
	memcpy(set->RR_PTR.resrec.name, &domainBuffer, nameLen);

	set->McastTxRx = mDNStrue;

	mDNS_Register_internal(&set->RR_A);
	mDNS_Register_internal(&set->RR_PTR);
}

mDNSOpaque16 mDNSOpaque16fromIntVal(mDNSu16 v)
{
	mDNSOpaque16 x;

	x.b[1] = (u8)(v >> 8);
	x.b[0] = (u8)(v & 0xFF);

	return (x);
}

static void print_real_raw(void *buffer, int len)
{
	u16 *p = (u16 *)buffer;
	int i = 0;
	while (i < len) {
		WL_ERROR(("%x:", ntoh16(*p)));
		i++; p++;
	}
}
static void print_raw(void *buffer, int len)
{
	uchar *p = (uchar *)buffer;
	int i = 0;
	while (i < len) {
		if (*p >= 0x21 && *p <= 0x7a)
			WL_ERROR(("%c", *p));
		else
			WL_ERROR(("%x ", *p));
		i++; p++;
		if (!(i % 40))
			WL_ERROR(("\n"));
	}
	WL_ERROR(("\n"));
}

/* mDNSPlatformInit atakes the incoming flattened database, parses it into a transient
 * state and kicks off the mDNS engine.
 * Not sure yet who will call this... equivelent of wl_up()
 */
void mDNSPlatformInit(wlc_dngl_ol_mdns_info_t *mdns, proxy_services_t *proxy_services)
{
	proxy_services_offload_addr_info_t	*pOffloadAddrInfo;
	proxy_services_secondary_addr_info_t	*pSecondaryAddrInfo;
	proxy_services_data_t			*pData;
	domainlabel 				name;
	domainname 				type;
	domainname 				domain;
	mDNSu8 					*pFirstRR;
	mDNSu16 				*pSRVOffsets;
	mDNSu16 				*pTXTOffsets;
	mDNSu8 					*ptr;
	mDNSu8 					*end;
	mDNSu8 					*pInfo;
	NetworkInterfaceInfo 			*intf;
	NetworkInterfaceInfo 			*intfList = mDNSNULL;
	NetworkInterfaceInfo 			**p;
	int 					i;
	int 					j;
	mDNS *m = mdns_share_blk->m;
	int nSRV_Rec, nTXT_Rec, nUDP_Wake, nTCP_Wake;
	int tot_ports, tot_recs, n_serv, n_txt;

	UNUSED_PARAMETER(pSRVOffsets);

	WL_TRACE(("Enter %s\n", __FUNCTION__));

	memcpy(mdns->NicMacAddr, (uchar *)&RXOETXINFO(mdns->wlc_dngl_ol)->cur_etheraddr,
		ETHER_ADDR_LEN);

	WL_ERROR(("%s: Mac addr %x:%x:%x:%x:%x:%x\n", __FUNCTION__,
		mdns->NicMacAddr[0], mdns->NicMacAddr[1], mdns->NicMacAddr[2],
		mdns->NicMacAddr[3], mdns->NicMacAddr[4], mdns->NicMacAddr[5]));

	/*
	 *	Populate  Table 7 (proxy_services)
	 */
	pOffloadAddrInfo = (proxy_services_offload_addr_info_t *)&proxy_services->offloadAddresses;

	WL_ERROR(("%s: numIPv4 %d, numIPv6 %d, numIPV6Sol %d\n", __FUNCTION__,
		pOffloadAddrInfo->numIPv4, pOffloadAddrInfo->numIPv6, pOffloadAddrInfo->numIPv6Sol));

	/* Calculate length of Table 8, so we can find table 9 */
	pSecondaryAddrInfo = (proxy_services_secondary_addr_info_t *)((u8 *)pOffloadAddrInfo +
		sizeof(proxy_services_offload_addr_info_t) +
		(pOffloadAddrInfo->numIPv4 * 8) +	/* IPv4 address and masks */
		(pOffloadAddrInfo->numIPv6 * 17) +	/* IPv6 addresses and subnet length */
		(pOffloadAddrInfo->numIPv6Sol * 16));	/* IPv6 solicited addresses */

	/* Now get Table 7 numUDPWakeupPorts, numTCPWakeupPorts, numSRVRecords, numTXTRecords */
	pData = (proxy_services_data_t *)((u8 *)pSecondaryAddrInfo +
		sizeof(proxy_services_secondary_addr_info_t) +
		(pSecondaryAddrInfo->numIPv4 * 8) +
		(pSecondaryAddrInfo->numIPv6 * 17));

	WL_TRACE(("%s: numSRVR %d, numTXT %d numUDPWakeup %d numTCPWakeup %d\n",
		__FUNCTION__,
		pData->numSRVRecords, 	pData->numTXTRecords,
		pData->numUDPWakeupPorts, pData->numTCPWakeupPorts));

	nSRV_Rec = pData->numSRVRecords;
	nTXT_Rec = pData->numTXTRecords;
	nUDP_Wake = pData->numUDPWakeupPorts;
	nTCP_Wake = pData->numTCPWakeupPorts;
	tot_ports = nUDP_Wake + nTCP_Wake;
	tot_recs = nSRV_Rec + nTXT_Rec;

	WL_ERROR(("%s: numSRVR %d, numTXT %d numUDPWakeup %d numTCPWakeup %d\n",
		__FUNCTION__, nSRV_Rec, nTXT_Rec, nUDP_Wake, nTCP_Wake));

	pSRVOffsets = (mDNSu16 *)((u8 *)pData + sizeof(proxy_services_data_t) +
		((nUDP_Wake + nTCP_Wake) * sizeof(mDNSu16)));

	pTXTOffsets = (mDNSu16 *)((u8 *)pSRVOffsets + (nSRV_Rec * sizeof(mDNSu16)));

	pFirstRR = (u8 *)((u8 *)pData + sizeof(proxy_services_data_t) +
		((nUDP_Wake + nTCP_Wake) * sizeof(mDNSu16)) +
		((nSRV_Rec + nTXT_Rec) * sizeof(mDNSu16)));


	/*
	 *  Table 7 is now complete
	 */

	/* Now that Table 7 is parsed, start transferring to mDNS struct */
	/* #define m  mdns_share_blk->struct mDNS */
	m->OffloadFlags = proxy_services->flags;
	m->numServices = pData->numSRVRecords;
	m->numTxt = pData->numTXTRecords;

	end = pFirstRR + MDNS_SD_DRV_INTF_SIZE;

	WL_TRACE(("%s: Numservices = %d, numTXT %d\n", __FUNCTION__,
		m->numServices, m->numTxt));

	WL_TRACE(("%s: pSRVOffsets[0] = %d\n", __FUNCTION__, pSRVOffsets[0]));
	WL_TRACE(("%s: pSRVOffsets[1] = %d\n", __FUNCTION__, pSRVOffsets[1]));
	WL_TRACE(("%s: pSRVOffsets[2] = %d\n", __FUNCTION__, pSRVOffsets[2]));

	for (n_serv = 0; n_serv < m->numServices; n_serv++)
	{
		u16		offset;
		u16 		numOfSubTypes;
		domainname	*subTypes;
		mDNSIPPort 	port;

		WL_TRACE(("%s: Service %d\n", __FUNCTION__, n_serv));
		/* Make sure the 16-bit pTXTOffsets[n_serv] is algned. */
		memcpy((u8 *)&offset, &pSRVOffsets[n_serv], sizeof(u16));

		WL_TRACE(("Offset is %d\n", offset));
		ptr = pFirstRR + offset;
		ptr = GetLargeResourceRecord((DNSMessage *)pFirstRR, ptr, end,
			kDNSRecordTypePacketAns, &m->rec);
		if (!ptr) {
			WL_TRACE(("%s: ptr is NULL, continue\n", __FUNCTION__));
			continue;
		}

		m->numServicesFound++;

		if (m->MulticastHostname.c[0] == 0)
		{
			memcpy(&m->MulticastHostname, m->rec.r.resrec.rdata->u.srv->target,
				DomainNameLength(m->rec.r.resrec.rdata->u.srv->target));

			pInfo = (mDNSu8 *)((mDNSu8 *)pOffloadAddrInfo +
				sizeof(proxy_services_offload_addr_info_t));
			m->numIpv4Addr = pOffloadAddrInfo->numIPv4;
			for (j = 0; j < pOffloadAddrInfo->numIPv4; j++)
			{
				p = &intfList;
				while (*p)
					p = &(*p)->next;

				intf = (NetworkInterfaceInfo *)
					mDNSPlatformMemAllocate(sizeof(NetworkInterfaceInfo));
				memset(intf, 0, sizeof(NetworkInterfaceInfo));
				intf->ip.type = mDNSAddrType_IPv4;
				memcpy(intf->ip.ip.v4.b, pInfo, sizeof(mDNSv4Addr));
				pInfo += sizeof(mDNSv4Addr);
				memcpy(intf->mask.ip.v4.b, pInfo, sizeof(mDNSv4Addr));
				pInfo += sizeof(mDNSv4Addr);

				*p = intf;
				m->IPv4Available = mDNStrue;
				WL_TRACE(("%s: Have IPv4 interface\n", __FUNCTION__));

				if (j == 0)
					memcpy(m->hostIpv4Addr, intf->ip.ip.v4.b,
						sizeof(mDNSv4Addr));
			}

			m->numIpv6Addr = pOffloadAddrInfo->numIPv6;
			for (j = 0; j < pOffloadAddrInfo->numIPv6; j++)
			{
				u8	prefixLen;
				int	k;

				p = &intfList;
				while (*p)
					p = &(*p)->next;

				intf = (NetworkInterfaceInfo *)
					mDNSPlatformMemAllocate(sizeof(NetworkInterfaceInfo));
				memset(intf, 0, sizeof(NetworkInterfaceInfo));
				intf->ip.type = mDNSAddrType_IPv6;
				memcpy(intf->ip.ip.v6.b, pInfo, sizeof(mDNSv6Addr));
				pInfo += sizeof(mDNSv6Addr);
				prefixLen = *pInfo;
				for (k = 0; k < (prefixLen >> 3); k++)
					intf->mask.ip.v6.b[0] = 0xff;

				pInfo++;
				*p = intf;
				m->IPv6Available = mDNStrue;
				WL_TRACE(("%s: Have IPv6 interface\n", __FUNCTION__));

				if (j == 0)
					memcpy(m->hostIpv6Addr, intf->ip.ip.v6.b,
						sizeof(mDNSv6Addr));
			}

			if (pOffloadAddrInfo->numIPv6Sol)
			{
				m->ipv6SolAddr = mDNSPlatformMemAllocate
					(pOffloadAddrInfo->numIPv6Sol * sizeof(mDNSv6Addr));
				memcpy(m->ipv6SolAddr, pInfo,
					pOffloadAddrInfo->numIPv6Sol * sizeof(mDNSv6Addr));
				m->numIpv6SolAddr = pOffloadAddrInfo->numIPv6Sol;
			}

			if (pSecondaryAddrInfo->numIPv4)
			{
				pInfo = (mDNSu8 *)((mDNSu8 *)pSecondaryAddrInfo +
					sizeof(proxy_services_secondary_addr_info_t));
				m->secondaryIpv4Addr =
					(u8 *)mDNSPlatformMemAllocate
					(pSecondaryAddrInfo->numIPv4 * sizeof(mDNSv4Addr));
				for (j = 0; j < pSecondaryAddrInfo->numIPv4; j++)
					/* Copy the address - skip over the mask */
					memcpy(m->secondaryIpv4Addr,
						(u8 *)(pInfo + (n_serv * (2 * sizeof(mDNSv4Addr)))),
						sizeof(mDNSv4Addr));

				m->numSecondaryIpv4Addr = pSecondaryAddrInfo->numIPv4;
			}

			if (pSecondaryAddrInfo->numIPv6)
			{
				pInfo = (mDNSu8 *)((mDNSu8 *)pSecondaryAddrInfo +
					sizeof(proxy_services_secondary_addr_info_t) +
					(pSecondaryAddrInfo->numIPv6 *
					(2 * sizeof(mDNSv6Addr))));
				m->secondaryIpv6Addr = (u8 *)mDNSPlatformMemAllocate
					(pSecondaryAddrInfo->numIPv6 * sizeof(mDNSv6Addr));
				for (j = 0; j < pSecondaryAddrInfo->numIPv6; j++)
				{
					/* Copy the address */
					memcpy(m->secondaryIpv6Addr, pInfo, sizeof(mDNSv6Addr));

					/* Skip over prefix length */
					pInfo += (sizeof(mDNSv6Addr) + 1);
				}

				m->numSecondaryIpv6Addr = pSecondaryAddrInfo->numIPv6;
			}

			if (pData->numUDPWakeupPorts)
			{
				pInfo = (u8 *)((u8 *)pData + sizeof(proxy_services_data_t));
				m->udpWakeUpPorts = (u16 *)mDNSPlatformMemAllocate
					(pData->numUDPWakeupPorts * sizeof(mDNSu16));
				memcpy(m->udpWakeUpPorts, pInfo,
					pData->numUDPWakeupPorts * sizeof(mDNSu16));
				m->numUdpWakeUpPorts = pData->numUDPWakeupPorts;
				/* Wakeup ports are Network order objects but numports
				 *	is host order
				 */
				WL_ERROR(("%s: %d UDP Wakeup Ports: ",
					__FUNCTION__, m->numUdpWakeUpPorts));
				for (i = 0; i < m->numUdpWakeUpPorts; i++)
					WL_ERROR(("%d ", ntoh16(m->udpWakeUpPorts[i])));
				WL_ERROR(("\n"));
			} else {
				WL_ERROR(("%s: No UDP Wakeup ports\n", __FUNCTION__));
			}

			if (pData->numTCPWakeupPorts)
			{
				pInfo = (u8 *)((u8 *)pData + sizeof(proxy_services_data_t) +
					(pData->numUDPWakeupPorts * sizeof(mDNSu16)));
				m->tcpWakeUpPorts = (u16 *)mDNSPlatformMemAllocate
					(pData->numTCPWakeupPorts * sizeof(mDNSu16));
				memcpy((u8 *)m->tcpWakeUpPorts, pInfo,
					pData->numTCPWakeupPorts * sizeof(mDNSu16));

				/* Wakeup ports are Network order objects but
				 *	numports is host order
				 */
				m->numTcpWakeUpPorts = pData->numTCPWakeupPorts;
				WL_ERROR(("%s: %d TCP Wakeup Ports: ",
					__FUNCTION__, m->numTcpWakeUpPorts));
				for (i = 0; i < m->numTcpWakeUpPorts; i++)
					WL_ERROR(("%d ", ntoh16(m->tcpWakeUpPorts[i])));
				WL_ERROR(("\n"));
			} else {
				WL_ERROR(("%s: No TCP Wakeup ports\n", __FUNCTION__));
			}
		}

		memcpy(&numOfSubTypes, ptr, sizeof(u16));
		subTypes = (domainname *)(ptr + sizeof(u16));
		port = mDNSOpaque16fromIntVal(m->rec.r.resrec.rdata->u.srv->port.NotAnInteger);

		DeconstructServiceName(m->rec.r.resrec.name, &name, &type, &domain, TRUE, port, numOfSubTypes);
		mDNS_RegisterService(&name, &type, &domain,
			port,
			mDNSNULL, 0, m->rec.r.resrec.rroriginalttl,
			subTypes, numOfSubTypes);
	}

	for (n_txt = 0; n_txt < m->numTxt; n_txt++) {
		AuthRecord	*pRR_TXT;
		AuthRecord 	*rr;
		mDNSBool	found;
		u16		offset;

		/* Make sure the 16-bit pTXTOffsets[n_txt] is algned. */
		memcpy((u8 *)&offset, &pTXTOffsets[n_txt], sizeof(u16));
		WL_TRACE(("%s: TXT record %d. offset = %d\n", __FUNCTION__, n_txt, pTXTOffsets[n_txt]));

		ptr = pFirstRR + offset;
		ptr = GetLargeResourceRecord((DNSMessage *)pFirstRR, ptr, end,
			kDNSRecordTypePacketAns, &m->rec);
		if (!ptr)
			continue;

		rr = m->ResourceRecords;
		found = FALSE;
		while (rr) {
			if ((rr->resrec.rrtype == kDNSType_TXT) &&
			    (SameDomainName(rr->resrec.name, m->rec.r.resrec.name))) {
				found = TRUE;
				break;
			}

			rr = rr->next;
		}

		if (found) {
			/* There's already a TXT record - let's add the text string. */
			rr->resrec.rdata->u.txt =
				(UTF8str255 *)mDNSPlatformMemAllocate(m->rec.r.resrec.rdlength);
			rr->resrec.rdata->MaxRDLength = m->rec.r.resrec.rdlength;
			memcpy(rr->resrec.rdata->u.txt, m->rec.r.resrec.rdata->u.txt,
				m->rec.r.resrec.rdlength);
			rr->resrec.rdlength = m->rec.r.resrec.rdlength;
		} else {
			mDNSIPPort 	port;
			m->numTxtFound++;

			DeconstructServiceName(m->rec.r.resrec.name, &name, &type, &domain, FALSE, port, 0);
			if (SameDomainLabel(type.c, (mDNSu8 *)&DeviceInfoName) &&
				(m->DeviceInfo.resrec.RecordType == kDNSRecordTypeUnregistered))
			{
				mDNS_SetupResourceRecord(&m->DeviceInfo, kDNSType_TXT,
					m->rec.r.resrec.rroriginalttl, kDNSRecordTypeAdvisory);
				m->DeviceInfo.resrec.name = mDNSPlatformMemAllocate
					(DomainNameLength(m->rec.r.resrec.name));
				AssignDomainName(m->DeviceInfo.resrec.name, m->rec.r.resrec.name);
				m->DeviceInfo.resrec.rdata->u.txt = (UTF8str255 *)
					mDNSPlatformMemAllocate(m->rec.r.resrec.rdlength);
				m->DeviceInfo.resrec.rdata->MaxRDLength =
					m->rec.r.resrec.rdlength;
				memcpy(m->DeviceInfo.resrec.rdata->u.txt,
					m->rec.r.resrec.rdata->u.txt, m->rec.r.resrec.rdlength);
				m->DeviceInfo.resrec.rdlength = m->rec.r.resrec.rdlength;
				mDNS_Register_internal(&m->DeviceInfo);
			} else {
				pRR_TXT = (AuthRecord *)mDNSPlatformMemAllocate(sizeof(AuthRecord));
				memset(pRR_TXT, 0, sizeof(AuthRecord));
				mDNS_SetupResourceRecord(pRR_TXT, kDNSType_TXT,
					m->rec.r.resrec.rroriginalttl, kDNSRecordTypeKnownUnique);
				pRR_TXT->resrec.name = mDNSPlatformMemAllocate
					(DomainNameLength(m->rec.r.resrec.name));
				AssignDomainName(pRR_TXT->resrec.name, m->rec.r.resrec.name);
				pRR_TXT->resrec.rdata->u.txt = (UTF8str255 *)mDNSPlatformMemAllocate
					(m->rec.r.resrec.rdlength);
				pRR_TXT->resrec.rdata->MaxRDLength = m->rec.r.resrec.rdlength;
				memcpy(pRR_TXT->resrec.rdata->u.txt, m->rec.r.resrec.rdata->u.txt,
					m->rec.r.resrec.rdlength);
				pRR_TXT->resrec.rdlength = m->rec.r.resrec.rdlength;
				mDNS_Register_internal(pRR_TXT);
			}
		}
	}

	if (!m->numServices && !m->numTxt) {
		mdns->dns_enabled = FALSE;
		WL_ERROR(("%s: No TXT or SRV records, not starting mDNS\n", __FUNCTION__));
	} else {
		mdns->dns_enabled = TRUE;

		m->HostInterfaces = intfList;
		intf = intfList;
		while (intf) {
			AdvertiseInterface(intf);
			intf = intf->next;
		}
	}
}

/*
 * Primary runtime entry point.  Everything is based on flattened dbase.
 */
void
wl_mDNS_Init(wlc_dngl_ol_mdns_info_t *mdnsi, uint8 *dbase, uint32 dbase_len)
{
	proxy_services_t *proxy_services;
	s32	timenow;
	mDNS *m;

	if (mdnsi == NULL) {
		WL_ERROR(("%s: NULL mdnsi_ol, bail\n", __FUNCTION__));
		return;
	}
	if (mdnsi->magic != BONJOUR_PROXY_SIG) {
		WL_ERROR(("%s: Invalid magic\n", __FUNCTION__));
		return;
	}

        if (!mdnsi->wlc_dngl_ol->wowl_cfg.associated) {
                WL_ERROR(("%s: Not associated! Not enabling mDNS offload.\n", __FUNCTION__));
                return;
        }

	/* Sanity check the chunk of memory downloaded from host via flatten() 		*/
	/* bonjourProxyInfo is the struct defined in Table 6 of Hoans download spec 	*/

	/* proxy_servives should point to the output of flatten() 			*/
	/* bonjourProxyInfo is used nowhere else except here 				*/
	/* glob_mdns_fOffLoadData needs to morph into olmsg_shared_info 		*/

	/* At minimum dbase needs to have 32 bit signature */
	if (dbase_len <= sizeof(uint32)) {
		WL_ERROR(("%s: Empty dbase, not enabling mDNS offload. (len = %d)\n",
			__FUNCTION__, dbase_len));
		return;
	}

	if (*(uint32 *)dbase != BONJOUR_PROXY_SIG) {
		WL_ERROR(("%s: Invalid signature from UserClient, disabling mdns. 0x%x != 0x%x\n",
			__FUNCTION__, *(uint32 *)dbase, BONJOUR_PROXY_SIG));
		return;
	}

	bonjourProxyInfo.dataAddr = dbase;
	bonjourProxyInfo.dataSize = dbase_len;
	proxy_services = (proxy_services_t *)(bonjourProxyInfo.dataAddr);

	WL_TRACE(("%s: Signature from UserClient verified, len %d\n",
		__FUNCTION__, bonjourProxyInfo.dataSize));
	if (0) print_raw(dbase, 4);

	proxy_services->signature = 0;
	/* Init all fields of mdns_share_blk */
	memset(mdns_share_blk, 0, sizeof(mdns_share_mem_block_t));
	mdns_share_blk->signature = BONJOUR_PROXY_SIG;
	mdns_share_blk->mdns_sr = mDNSNULL;
	mdns_share_blk->mdns_intf = mDNSNULL;

	mdns_share_blk->m = (mDNS *)mDNSPlatformMemAllocate(sizeof(mDNS));
	memset(mdns_share_blk->m, 0, sizeof(mDNS));
	mdns_share_blk->m->signature = BONJOUR_PROXY_SIG;

	/* Task Scheduling variables */
	m = mdns_share_blk->m;
	m->timenow_adjust = 0;
	timenow = 0;

	/* MUST only be set within mDNS_Lock/mDNS_Unlock section */
	m->timenow                 = 0;
	m->timenow_last            = timenow;
	m->NextScheduledEvent      = timenow;
	m->SuppressSending         = timenow;
	m->NextScheduledResponse   = timenow + DNS_INFINITY;
	m->PktNum                  = 0;
	m->CanReceiveUnicastOn5353 = mDNSfalse; /* Assume we can't receive unicasts on 5353, */
						/* unless platform layer tells us otherwise */

	/* Fields below only required for mDNS Responder... */
	m->MulticastHostname.c[0]  = 0;
	m->ResourceRecords         = mDNSNULL;
	m->NewLocalRecords         = mDNSNULL;
	m->CurrentRecord           = mDNSNULL;

	m->omsg = (DNSMessage *)mdnsi->glob_txPktBuff;

	mDNSPlatformInit(mdnsi, proxy_services);

	if (mdnsi->dns_enabled) {
		wl_add_timer(glob_mdnsi->wlc_dngl_ol->wlc->wl, glob_mdnsi->mdns_execute_timer, 20, FALSE);
	}
}

void
wl_mDNS_event_handler(wlc_dngl_ol_mdns_info_t *mdnsi, uint32 event, void *event_data)
{
	switch (event) {
	case BCM_OL_E_WOWL_COMPLETE:
		WL_TRACE(("%s: %s: Starting mdns\n", __FUNCTION__, bcm_ol_event_str[event]));
		wl_mDNS_Init(mdnsi, (uint8 *)RXOESHARED(mdnsi->wlc_dngl_ol)->mdns_dbase,
			RXOESHARED(mdnsi->wlc_dngl_ol)->mdns_len);
		break;
	case BCM_OL_E_DEAUTH:
	case BCM_OL_E_DISASSOC:
	case BCM_OL_E_BCN_LOSS:
	case BCM_OL_E_PME_ASSERTED:
#ifdef BCMDBG
		if (mdnsi->dns_enabled && !mdnsi->sent_wakeup)
			WL_ERROR(("%s: %s, Disabling mdns\n",
				__FUNCTION__, bcm_ol_event_str[event]));
#endif
		mdnsi->dns_enabled = FALSE;
		break;
	}
}

char v6_multi[] =
	{ 0xFF, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFB };
char v4_multi[] = { 224,   0,   0, 251 };
mDNSv4Addr      zerov4Addr         = { { 0 } };
mDNSv6Addr      zerov6Addr         = { { 0 } };
mDNSv4Addr      onesIPv4Addr       = { { 255, 255, 255, 255 } };
mDNSv6Addr      onesIPv6Addr       =
	{ { 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255 } };

mDNSAddr   	AllDNSLinkGroup_v4 = { mDNSAddrType_IPv4, { { { 224,   0,   0, 251 } } } };
mDNSAddr   	AllDNSLinkGroup_v6 = { mDNSAddrType_IPv6,
	{ { { 0xFF, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFB } } } };

mDNSEthAddr     MulicastV4EthAddr  = { { 0x01, 0x00, 0x5E, 0x00, 0x00, 0xFB } };
mDNSEthAddr     MulicastV6EthAddr  = { { 0x33, 0x33, 0x00, 0x00, 0x00, 0xFB } };
mDNSIPPort 	MulticastDNSPort   =
	{ { MulticastDNSPortAsNumber >> 8, MulticastDNSPortAsNumber & 0xFF } };

mDNSOpaque16 	zeroID        = { { 0, 0 } };
mDNSOpaque16 	QueryFlags    = { { kDNSFlag0_QR_Query | kDNSFlag0_OP_StdQuery, 0 } };
mDNSOpaque16 	ResponseFlags =
	{ { kDNSFlag0_QR_Response | kDNSFlag0_OP_StdQuery | kDNSFlag0_AA, 0 } };

mDNSInterfaceID mDNSInterfaceMark = (mDNSInterfaceID)~0;


static bool
is_multi(u8 *addr, int type)
{
	if (type == mDNSAddrType_IPv6)
		return (memcmp(addr, v6_multi, sizeof(v6_multi)) == 0);
	else
		if (type == mDNSAddrType_IPv4)
			return (memcmp(addr, v4_multi, sizeof(v4_multi)) == 0);
	return 0;
}

static s32
mDNSPlatformRawTime(void)
{
	return OSL_SYSUPTIME();
}

static s32
mDNS_TimeNow_NoLock(mDNS *m)
{
	return mDNSPlatformRawTime() + m->timenow_adjust;
}

static void mDNS_Lock(void)
{
	mDNS *m = mdns_share_blk->m;
	/* If this is an initial entry into the mDNSCore code, set m->timenow
	 * else, if this is a re-entrant entry into the mDNSCore code,
	 *  m->timenow should already be set.
	 */
	if (m->mDNS_busy == 0)
	{
		m->timenow = mDNS_TimeNow_NoLock(m);
		if (m->timenow == 0)
			m->timenow = 1;
	} else {
		if (m->timenow == 0) {
			m->timenow = mDNS_TimeNow_NoLock(m);
			if (m->timenow == 0)
				m->timenow = 1;
		}
	}

	if (m->timenow_last - m->timenow > 0) {
		WL_ERROR(("%s: Detected backwards time, set adjust\n", __FUNCTION__));
		m->timenow_adjust += m->timenow_last - m->timenow;
		m->timenow = m->timenow_last;
	}
	m->timenow_last = m->timenow;

	/* Increment mDNS_busy so we'll recognise re-entrant calls */
	m->mDNS_busy++;
}

static s32 GetNextScheduledEvent(void)
{
	mDNS *m = mdns_share_blk->m;
	s32 e = m->timenow + DNS_INFINITY;

	if (m->NewLocalRecords) {
		//BR: This is ALWAY true... hoarkage seems to be intro'ed in ethernet port
		DNS_TIME("%s: NewLocalRecords is true, return timenow %d\n",
			__FUNCTION__, m->timenow);
		return (m->timenow);
	}

	if (m->SuppressSending) {
		WL_ERROR(("%s: SuppressSending is true, return SuppressSending %d\n",
			__FUNCTION__, m->SuppressSending));
		return (m->SuppressSending);
	}

	if (e - m->NextScheduledResponse > 0) {
		WL_ERROR(("%s: NextScheduledResponse is true, return NextScheduledResponse %d\n",
			__FUNCTION__, m->NextScheduledResponse));
		e = m->NextScheduledResponse;
	} else {
		WL_ERROR(("%s: IMPOSSIBLE! Infinity is smaller! %d %d, raw %d %d\n",
			__FUNCTION__, e, m->NextScheduledResponse, e, m->NextScheduledResponse));
	}
	return (e);
}

static void mDNS_Unlock(void)
{
	mDNS *m = mdns_share_blk->m;
	/* Decrement mDNS_busy */
	m->mDNS_busy--;

	/* If this is a final exit from the mDNSCore code,
	 * set m->NextScheduledEvent and clear m->timenow.
	 */
	if (m->mDNS_busy == 0)
	{
		m->NextScheduledEvent = GetNextScheduledEvent();
		m->timenow = 0;
	}
	DNS_TIME("%s: Next Scheduled = GetNextScheduledEvent %d\n",
		__FUNCTION__, m->NextScheduledEvent);
}

static u32 mDNS_send_packet(void *pHdr, u8 hdrLen, void *pBuffer, u16 bufferLen)
{
	int ret = 0;
	uint8 *pnew_pkt = NULL;

	if (!glob_mdnsi->dns_enabled) {
		WL_ERROR(("%s: mdns is disabled!\n", __FUNCTION__));
		return ret;
	}
	if (hdrLen) {
		pnew_pkt = mDNSPlatformMemAllocate(hdrLen + bufferLen);
		if (!pnew_pkt) {
			WL_ERROR(("%s: Failed to allocate %d buf\n",
				__FUNCTION__, hdrLen + bufferLen));
		} else {
			memcpy(pnew_pkt, pHdr, hdrLen);
			memcpy(pnew_pkt + hdrLen, pBuffer, bufferLen);
			ret = generic_send_packet(glob_mdnsi->wlc_dngl_ol, pnew_pkt,
				hdrLen + bufferLen);
		}
	} else {
		ret = generic_send_packet(glob_mdnsi->wlc_dngl_ol, pBuffer, bufferLen);
	}
	if (ret)
		WL_ERROR(("Error mdns_send_packet returns %d\n", ret));

	if (hdrLen && pnew_pkt) {
		mDNSPlatformMemFree(pnew_pkt, hdrLen + bufferLen);
	}
	return ret;
}

static u32 mDNSPlatformRandomSeed(void)
{
	return hnd_time() >> 3;
}

static u32 mDNSRandom(u32 max)
{
	u32 mask = 1;

	if (!glob_mdnsi->seed)
	{
		int i;
		glob_mdnsi->seed = mDNSPlatformRandomSeed();		/* Pick an initial seed */
		for (i= 0; i<100; i++)
			glob_mdnsi->seed = glob_mdnsi->seed * 21 + 1;	/* And mix it up a bit */
	}
	while (mask < max)
		mask = (mask << 1) | 1;

	do
		glob_mdnsi->seed = glob_mdnsi->seed * 21 + 1;
	while ((glob_mdnsi->seed & mask) > max);

	return (glob_mdnsi->seed & mask);
}

static mDNSu8 *skipDomainName(DNSMessage *msg, mDNSu8 *ptr, mDNSu8 *end)
{
	mDNSu16 total = 0;

	if (ptr < (mDNSu8*)msg || ptr >= end)
	{
		return (mDNSNULL);
	}

	while (1)			/* Read sequence of labels */
	{
		mDNSu8 len = *ptr++;	/* Read length of this label */
		/* If length is zero, that means this name is complete */
		if (len == 0) return (ptr);
		switch (len & 0xC0)
		{
			case 0x00:
				/* Remember: expect at least one more byte for the root label */
				if (ptr + len >= end)
				{
					return (mDNSNULL);
				}
				/* Remember: expect at least one more byte for the root label */
				if (total + 1 + len >= MAX_DOMAIN_NAME)
				{
					return (mDNSNULL);
				}
				ptr += len;
				total += 1 + len;
				break;

			case 0x40:
				return (mDNSNULL);
			case 0x80:
				return (mDNSNULL);
			case 0xC0:
				return (ptr+1);
		}
	}
}

static mDNSu8 *skipQuestion(DNSMessage *msg, mDNSu8 *ptr, mDNSu8 *end)
{
	ptr = skipDomainName(msg, ptr, end);
	if (!ptr)
	{
		return (mDNSNULL);
	}
	if (ptr+4 > end)
	{
		return (mDNSNULL);
	}
	return (ptr+4);
}

static mDNSu8 *LocateAnswers(DNSMessage *msg, mDNSu8 *end)
{
	int i;
	mDNSu8 *ptr = msg->data;
	for (i = 0; i < msg->h.numQuestions && ptr; i++)
		ptr = skipQuestion(msg, ptr, end);
	return (ptr);
}

static u8 *FindCompressionPointer(u8 *base, u8 *end, u8 *domname)
{
	u8 *result = end - *domname - 1;

	/* There's no point trying to match just the root label */
	if (*domname == 0) return (mDNSNULL);

	/* This loop examines each possible starting position in packet, starting */
	/* end of the packet and working backwards */
	while (result >= base)
	{
		/* If the length byte and first character of the label match,
		 * then check further to see if this location in the packet
		 * will yield a useful name compression pointer.
		 */
		if (result[0] == domname[0] && result[1] == domname[1])
		{
			u8 *name = domname;
			u8 *targ = result;

			while (targ + *name < end)
			{
				/* First see if this label matches */
				int i;
				u8 *pointertarget;

				for (i = 0; i <= *name; i++) if (targ[i] != name[i]) break;
				if (i <= *name)
					break;	/* If label did not match, bail out */
				targ += 1 + *name; /* Else, did match, so advance target pointer */
				name += 1 + *name; /* and proceed to check next label */

				/* If no more labels, we found a match! */
				if (*name == 0 && *targ == 0)
					return (result);

				/* If no more labels to match, we failed, so bail out */
				if (*name == 0)
					break;

				/* The label matched, so now follow the pointer
				 * (if appropriate) and then see if
				 * the next label matches.
				 */

				/* If length value, continue to check next label */
				if (targ[0] < 0x40)
					continue;
				/* If 40-BF, not valid */
				if (targ[0] < 0xC0)
					break;
				/* Second byte not present! */
				if (targ+1 >= end)
					break;
				pointertarget = base + (((u16)(targ[0] & 0x3F)) << 8) + targ[1];

				/* Pointertarget must point *backwards* in the packet */
				if (targ < pointertarget)
					break;

				/* Pointertarget must point to a valid length byte */
				if (pointertarget[0] >= 0x40)
					break;
				targ = pointertarget;
			}
		}
		result--;	/* We failed to match at this search position, so back up */
				/* the tentative result pointer and try again */
	}
	return (mDNSNULL);
}

/* Put a string of dot-separated labels as length-prefixed labels */
/* domainname is a fully-qualified name (i.e. assumed to be ending in a dot, even if it doesn't) */
/* msg points to the message we're building (pass mDNSNULL if we don't want */
/* to use compression pointers) */
/* end points to the end of the message so far */
/* ptr points to where we want to put the name */
/* limit points to one byte past the end of the buffer that we must not overrun */
/* domainname is the name to put */
u8 *putDomainNameAsLabels(DNSMessage *msg, u8 *ptr, u8 *limit, domainname *name);
u8 *putDomainNameAsLabels(DNSMessage *msg, u8 *ptr, u8 *limit, domainname *name)
{
	u8 *base        = (u8 *)msg;
	u8 *np          = name->c;
	u8 *max         = name->c + MAX_DOMAIN_NAME;	/* Maximum that's valid */
	u8 *pointer     = mDNSNULL;
	u8 *searchlimit = ptr;

	/* While we've got characters in the name, and space to write them in the message... */
	while (*np && ptr < limit-1)
	{
		if (*np > MAX_DOMAIN_LABEL)
			/* Malformed domain name %##s (label more than 63 bytes) */
			return (mDNSNULL);

		/*
		* This check correctly allows for the final trailing root label: e.g.
		* Suppose our domain name is exactly 255 bytes long, including the final
		* trailing root label.  Suppose np is now at name->c[248], and we're about
		* to write our last non-null label ("local").
		* We know that max will be at name->c[255]
		* That means that np + 1 + 5 == max - 1, so we (just) pass the "if" test below,
		* write our six bytes, then exit the loop, write the final terminating root label,
		* and the domain name we've written is exactly 255 bytes long, exactly
		* at the correct legal limit.
		* If the name is one byte longer, then we fail the "if" test below,
		* and correctly bail out.
		*/
		if (np + 1 + *np >= max)
			/* Malformed domain name %##s (more than 255 bytes) */
			return (mDNSNULL);

		if (base) pointer = FindCompressionPointer(base, searchlimit, np);
		if (pointer)	/* Use a compression pointer if we can */
		{
			u16 offset = (u16)(pointer - base);
			*ptr++ = (u8)(0xC0 | (offset >> 8));
			*ptr++ = (u8)(offset &  0xFF);
			return (ptr);
		}
		else		/* Else copy one label and try again */
		{
			int i;
			u8 len = *np++;

			if (ptr + 1 + len >= limit)
				return (mDNSNULL);

			*ptr++ = len;

			for (i = 0; i < len; i++)
				*ptr++ = *np++;
		}
	}

	if (ptr < limit)	/* If we didn't run out of space */
	{
		*ptr++ = 0;	/* Put the final root label */
		return (ptr);	/* and return */
	}

	return (mDNSNULL);
}

u8 *putRData(DNSMessage *msg, u8 *ptr,  u8 *limit, ResourceRecord *rr);
u8 *putRData(DNSMessage *msg, u8 *ptr,  u8 *limit, ResourceRecord *rr)
{
	u8 *ret;
	switch (rr->rrtype)
	{
		case kDNSType_A:
			if (rr->rdlength != 4)
				return (mDNSNULL);
			if (ptr + 4 > limit)
				return (mDNSNULL);
			*ptr++ = rr->rdata->u.ipv4->b[0];
			*ptr++ = rr->rdata->u.ipv4->b[1];
			*ptr++ = rr->rdata->u.ipv4->b[2];
			*ptr++ = rr->rdata->u.ipv4->b[3];
			return (ptr);

		case kDNSType_AAAA:
			if (rr->rdlength != sizeof(mDNSv6Addr)) return (mDNSNULL);
			if (ptr + sizeof(mDNSv6Addr) > limit) return (mDNSNULL);
			memcpy(ptr, rr->rdata->u.ipv6, sizeof(mDNSv6Addr));
			return (ptr + sizeof(mDNSv6Addr));

		case kDNSType_SRV:
			WL_TRACE(("%s: Type kDNSType_SRV\n", __FUNCTION__));
			if (ptr + 6 > limit) {
				WL_ERROR(("%s: ptr + 6 > limit\n", __FUNCTION__));
				return (mDNSNULL);
			}
			*ptr++ = (mDNSu8)(rr->rdata->u.srv->priority >> 8);
			*ptr++ = (mDNSu8)(rr->rdata->u.srv->priority &  0xFF);
			*ptr++ = (mDNSu8)(rr->rdata->u.srv->weight   >> 8);
			*ptr++ = (mDNSu8)(rr->rdata->u.srv->weight   &  0xFF);
			*ptr++ = rr->rdata->u.srv->port.b[0];
			*ptr++ = rr->rdata->u.srv->port.b[1];
			return (putDomainNameAsLabels(msg, ptr, limit, rr->rdata->u.srv->target));

		case kDNSType_CNAME: /* Same as PTR */
		case kDNSType_PTR:
			WL_TRACE(("%s: Type kDNSType_PTR\n", __FUNCTION__));
			ret = putDomainNameAsLabels(msg, ptr, limit, rr->rdata->u.name);
			return ret;

		case kDNSType_NSEC:
			{
				WL_TRACE(("%s: Type kDNSType_NSEC\n", __FUNCTION__));
				/* For our simplified use of NSEC synthetic records: */
				/* nextname is always the record's own name, */
				/* the block number is always 0, */
				/* the count byte is a value in the range 1-32, */
				/* followed by the 1-32 data bytes */

				int i, j;

				for (i = sizeof(rdataNSEC); i > 0; i--)
					if (rr->rdata->u.nsec->bitmap[i-1])
						break;

				ptr = putDomainNameAsLabels(msg, ptr, limit, rr->name);
				if (!ptr) return (mDNSNULL);
				if (ptr + 2 + i > limit) return (mDNSNULL);
				*ptr++ = 0;
				*ptr++ = i;
				for (j = 0; j < i; j++) *ptr++ = rr->rdata->u.nsec->bitmap[j];
				return ptr;
			}

		case kDNSType_TXT:
		case kDNSType_HINFO:
		default:
			WL_TRACE(("%s: Type kDNSType_TXT\n", __FUNCTION__));
			if (ptr + rr->rdlength > limit)
				return (mDNSNULL);
			memcpy(ptr, rr->rdata->u.data, rr->rdlength);
			return (ptr + rr->rdlength);
	}
}

u8 *PutResourceRecordTTLWithLimit(DNSMessage * msg, u8 *ptr, u16 *count,
	ResourceRecord *rr, u32 ttl,  u8 *limit);
u8 *PutResourceRecordTTLWithLimit(DNSMessage * msg, u8 *ptr, u16 *count,
	ResourceRecord *rr, u32 ttl,  u8 *limit)
{
	u8 *endofrdata;
	u16 actualLength;

	if (rr->RecordType == kDNSRecordTypeUnregistered)
	{
		return (ptr);
	}

	ptr = putDomainNameAsLabels(msg, ptr, limit, rr->name);
	if (!ptr || ptr + 10 >= limit) {
		WL_ERROR(("%s: Hit the limit, returning\n", __FUNCTION__));
		return (mDNSNULL);	/* If we're out-of-space, return mDNSNULL */
	}
	ptr[0] = (u8)(rr->rrtype  >> 8);
	ptr[1] = (u8)(rr->rrtype  &  0xFF);
	ptr[2] = (u8)(rr->rrclass >> 8);
	ptr[3] = (u8)(rr->rrclass &  0xFF);
	ptr[4] = (u8)((ttl >> 24) &  0xFF);
	ptr[5] = (u8)((ttl >> 16) &  0xFF);
	ptr[6] = (u8)((ttl >>  8) &  0xFF);
	ptr[7] = (u8)(ttl &  0xFF);
	endofrdata = putRData(msg, ptr+10, limit, rr);
	if (!endofrdata) {
		WL_ERROR(("%s: Endofrdata, returning\n", __FUNCTION__));
		return (mDNSNULL);
	}

	/* Go back and fill in the actual number of data bytes we wrote */
	/* (actualLength can be less than rdlength when domain name compression is used) */
	actualLength = (u16)(endofrdata - ptr - 10);
	ptr[8] = (u8)(actualLength >> 8);
	ptr[9] = (u8)(actualLength &  0xFF);

	if (count) (*count)++;
	return (endofrdata);
}

u8 *PutResourceRecordCappedTTL(DNSMessage *msg, u8 *ptr, u16 *count,
	ResourceRecord *rr, u32 maxttl);
u8 *PutResourceRecordCappedTTL(DNSMessage *msg, u8 *ptr, u16 *count,
	ResourceRecord *rr, u32 maxttl)
{
	if (maxttl > rr->rroriginalttl)
		maxttl = rr->rroriginalttl;

	return (PutResourceRecordTTL(msg, ptr, count, rr, maxttl));
}

mDNSu8 *putQuestion(DNSMessage *msg, mDNSu8 *ptr, mDNSu8 *limit,
	domainname *name, mDNSu16 rrtype, mDNSu16 rrclass);
mDNSu8 *putQuestion(DNSMessage *msg, mDNSu8 *ptr, mDNSu8 *limit,
	domainname *name, mDNSu16 rrtype, mDNSu16 rrclass)
{
	ptr = putDomainNameAsLabels(msg, ptr, limit, name);
	if (!ptr || ptr+4 >= limit) {
		WL_ERROR(("%s: Hit the limit, returning\n", __FUNCTION__));
		return (mDNSNULL);	/* If we're out-of-space, return mDNSNULL */
	}
	ptr[0] = (mDNSu8)(rrtype  >> 8);
	ptr[1] = (mDNSu8)(rrtype  &  0xFF);
	ptr[2] = (mDNSu8)(rrclass >> 8);
	ptr[3] = (mDNSu8)(rrclass &  0xFF);
	msg->h.numQuestions++;
	return (ptr+4);
}

void InitializeDNSMessage(DNSMessageHeader *h, mDNSOpaque16 id, mDNSOpaque16 flags);
void InitializeDNSMessage(DNSMessageHeader *h, mDNSOpaque16 id, mDNSOpaque16 flags)
{
	h->id             = id;
	h->flags          = flags;
	h->numQuestions   = 0;
	h->numAnswers     = 0;
	h->numAuthorities = 0;
	h->numAdditionals = 0;
}

mDNSBool AddressIsLocalSubnet(mDNSAddr *addr);
mDNSBool AddressIsLocalSubnet(mDNSAddr *addr)
{
	NetworkInterfaceInfo *intf;
	mDNS *m = mdns_share_blk->m;

	if (addr->type == mDNSAddrType_IPv4)
	{
		if (addr->ip.v4.b[0] == 169 && addr->ip.v4.b[1] == 254)
			return (mDNStrue);
		for (intf = m->HostInterfaces; intf; intf = intf->next)
			if (intf->ip.type == addr->type && intf->McastTxRx)
				if (((intf->ip.ip.v4.NotAnInteger ^ addr->ip.v4.NotAnInteger) &
				    intf->mask.ip.v4.NotAnInteger) == 0)
					return (mDNStrue);
	}

	if (addr->type == mDNSAddrType_IPv6)
	{
		if (addr->ip.v6.b[0] == 0xFE && addr->ip.v6.b[1] == 0x80)
			return (mDNStrue);
		for (intf = m->HostInterfaces; intf; intf = intf->next) {
			if (intf->ip.type == addr->type && intf->McastTxRx) {
				if ((((intf->ip.ip.v6.l[0] ^ addr->ip.v6.l[0]) &
					    intf->mask.ip.v6.l[0]) == 0) &&
				    (((intf->ip.ip.v6.l[1] ^ addr->ip.v6.l[1]) &
					    intf->mask.ip.v6.l[1]) == 0) &&
				    (((intf->ip.ip.v6.l[2] ^ addr->ip.v6.l[2]) &
					    intf->mask.ip.v6.l[2]) == 0) &&
				    (((intf->ip.ip.v6.l[3] ^ addr->ip.v6.l[3]) &
					    intf->mask.ip.v6.l[3]) == 0)) {
						return (mDNStrue);
				}
			}
		}
	}

	return (mDNSfalse);
}

bool mDNSAddrIsDNSMulticast(mDNSAddr *ip);
bool mDNSAddrIsDNSMulticast(mDNSAddr *ip)
{
	switch (ip->type) {
	case mDNSAddrType_IPv4:
		return (bool)(ip->ip.v4.NotAnInteger == AllDNSLinkGroupv4.NotAnInteger);
	case mDNSAddrType_IPv6:
		return (bool)(!memcmp(ip->ip.v6.b, AllDNSLinkGroupv6.b, sizeof(mDNSv6Addr)));
	default:
		WL_ERROR(("%s: unknown type 0x%x\n", __FUNCTION__, ip->type));
		return (mDNSfalse);
	}
}

/* CacheRecord *ks is the CacheRecord from the known answer list in the query. */
/* This is the information that the requester believes to be correct. */
/* AuthRecord *rr is the answer we are proposing to give, if not suppressed. */
/* This is the information that we believe to be correct. */
/* We've already determined that we plan to give this answer on this interface */
/* (either the record is non-specific, or it is specific to this interface) */
/* so now we just need to check the name, type, class, rdata and TTL. */
bool ShouldSuppressKnownAnswer(CacheRecord *ka, AuthRecord *rr);
bool ShouldSuppressKnownAnswer(CacheRecord *ka, AuthRecord *rr)
{
	/* If RR signature is different, or data is different, then don't suppress our answer */
	if (!IdenticalResourceRecord(&ka->resrec, &rr->resrec)) return (mDNSfalse);

	/* If the requester's indicated TTL is less than half the real TTL, */
	/* we need to give our answer before the requester's copy expires. */
	/* If the requester's indicated TTL is at least half the real TTL, */
	/* then we can suppress our answer this time. */
	/* If the requester's indicated TTL is greater than the TTL we believe, */
	/* then that's okay, and we don't need to do anything about it. */
	/* (If two responders on the network are offering the same information, */
	/* that's okay, and if they are offering the information with different TTLs, */
	/* the one offering the lower TTL should defer to the one offering the higher TTL.) */
	return (bool)(ka->resrec.rroriginalttl >= (rr->resrec.rroriginalttl >> 1));
}

u8 *getQuestion(DNSMessage *msg, u8 *ptr, u8 *end, DNSQuestion *question);
u8 *getQuestion(DNSMessage *msg, u8 *ptr, u8 *end, DNSQuestion *question)
{
	ptr = getDomainName(msg, ptr, end, &question->qname);
	if (!ptr)
		/* Malformed domain name in DNS question section */
		return (mDNSNULL);
	if (ptr+4 > end)
		/* Malformed DNS question section -- no query type and class! */
		return (mDNSNULL);

	question->qtype  = (u16)((u16)ptr[0] << 8 | ptr[1]);		/* Get type */
	question->qclass = (u16)((u16)ptr[2] << 8 | ptr[3]);		/* and class */
	return (ptr+4);
}

bool ResourceRecordAnswersQuestion(ResourceRecord *rr, DNSQuestion *q);
bool ResourceRecordAnswersQuestion(ResourceRecord *rr, DNSQuestion *q)
{
	/* RR type CNAME matches any query type.
	 * QTYPE ANY matches any RR type.
	 * QCLASS ANY matches any RR class.
	 */
	if (!RRTypeAnswersQuestionType(rr, q->qtype)) return (mDNSfalse);
	if (rr->rrclass != q->qclass && q->qclass != kDNSQClass_ANY) return (mDNSfalse);

	return (SameDomainName(rr->name, &q->qname));
}

mDNSBool AnyTypeRecordAnswersQuestion(ResourceRecord *rr, DNSQuestion *q);
mDNSBool AnyTypeRecordAnswersQuestion(ResourceRecord *rr, DNSQuestion *q)
{
	if (rr->rrclass != q->qclass && q->qclass != kDNSQClass_ANY)
		return (mDNSfalse);

	return (SameDomainName(rr->name, &q->qname));
}

void AddRecordToResponseList(AuthRecord ***nrpp, AuthRecord *rr, AuthRecord *add);
void AddRecordToResponseList(AuthRecord ***nrpp, AuthRecord *rr, AuthRecord *add)
{
	if (rr->NextResponse == mDNSNULL && *nrpp != &rr->NextResponse)
	{
		**nrpp = rr;
		/* NR_AdditionalTo must point to a record with NR_AnswerTo set
		 * (and not NR_AdditionalTo)
		 * If 'add' does not meet this requirement, then follow its
		 * NR_AdditionalTo pointer to a record that does.
		 * The referenced record will definitely be acceptable
		 * (by recursive application of this rule)
		 */
		if (add && add->NR_AdditionalTo) add = add->NR_AdditionalTo;
		rr->NR_AdditionalTo = add;
		*nrpp = &rr->NextResponse;
	}
}

#define RRTypeIsAddressType(T) ((T) == kDNSType_A || (T) == kDNSType_AAAA)

void AddAdditionalsToResponseList(AuthRecord *ResponseRecords, AuthRecord ***nrpp);
void AddAdditionalsToResponseList(AuthRecord *ResponseRecords, AuthRecord ***nrpp)
{
	AuthRecord  *rr, *rr2;
	mDNS *m = mdns_share_blk->m;

	for (rr=ResponseRecords; rr; rr=rr->NextResponse) /* For each record we plan to put */
	{
		if (rr->Additional1)
			AddRecordToResponseList(nrpp, rr->Additional1, rr);

		if (rr->Additional2)
			AddRecordToResponseList(nrpp, rr->Additional2, rr);

		/* For SRV records, automatically add the Address record(s) for the target host */
		if (rr->resrec.rrtype == kDNSType_SRV)
		{
			/* Scan list of resource records */
			for (rr2 = m->ResourceRecords; rr2; rr2 = rr2->next)
				/* For all address records (A/AAAA) ... */
				if (RRTypeIsAddressType(rr2->resrec.rrtype) &&
					SameDomainName(rr->resrec.rdata->u.srv->target,
					rr2->resrec.name))
					AddRecordToResponseList(nrpp, rr2, rr);
		}
		/* For service PTR, see if we want to add DeviceInfo record */
		else if (rr->resrec.rrtype == kDNSType_PTR)
		{
			if (SameDomainLabel(rr->resrec.rdata->u.name->c,
			    m->DeviceInfo.resrec.name->c)) {
				AddRecordToResponseList(nrpp, &m->DeviceInfo, rr);
			}
		}
	}
}

#define MustSendRecord(RR) ((RR)->NR_AnswerTo || (RR)->NR_AdditionalTo)

u8 *GenerateUnicastResponse(DNSMessage *query, mDNSu8 *end, bool LegacyQuery,
	AuthRecord *ResponseRecords);
u8 *GenerateUnicastResponse(DNSMessage *query, mDNSu8 *end, bool LegacyQuery,
	AuthRecord *ResponseRecords)
{
	mDNS *m = mdns_share_blk->m;
	u8 *responseptr	= m->omsg->data;
	u8 *limit = m->omsg->data + sizeof(m->omsg->data);
	u8 *ptr	= query->data;
	AuthRecord  *rr;
	u32 maxttl = 0x70000000;
	int i;

	WL_ERROR(("%s\n", __FUNCTION__));
	/* Initialize the response fields so we can answer the questions */
	InitializeDNSMessage(&m->omsg->h, query->h.id, ResponseFlags);

	/*  */
	/* 1. Write out the list of questions we are actually going to answer with this packet */
	/*  */
	if (LegacyQuery)
	{
		maxttl = 10;
		for (i= 0; i<query->h.numQuestions; i++)	/* For each question... */
		{
			DNSQuestion q;

			ptr = getQuestion(query, ptr, end, &q);	/* get the question... */
			if (!ptr)
				return (mDNSNULL);

			/* and search our list of proposed answers */
			for (rr = ResponseRecords; rr; rr = rr->NextResponse)
			{
				/* If we're going to generate a record answering this question */
				if (rr->NR_AnswerTo == ptr)
				{	/* then put the question in the question section */
					responseptr = putQuestion(m->omsg, responseptr, limit,
						&q.qname, q.qtype, q.qclass);
					/* Ran out of space for questions! */
					if (!responseptr)
						return (mDNSNULL);

					/* break out of the ResponseRecords loop,
					   and go on to the next question
					 */
					break;
				}
			}
		}

		/* LogMsg("GenerateUnicastResponse: ERROR! Why no questions?"); */
		if (m->omsg->h.numQuestions == 0)
			return (mDNSNULL);
	}

	/* *** */
	/* *** 2. Write Answers */
	/* *** */
	for (rr = ResponseRecords; rr; rr = rr->NextResponse)
		if (rr->NR_AnswerTo)
		{
			u8 *p = PutResourceRecordCappedTTL(m->omsg, responseptr,
				&m->omsg->h.numAnswers, &rr->resrec, maxttl);
			if (p)
				responseptr = p;
			else
				/* debugf("Ran out of space for answers!"); */
				m->omsg->h.flags.b[0] |= kDNSFlag0_TC;
		}

	/* *** */
	/* *** 3. Write Additionals */
	/* *** */
	for (rr = ResponseRecords; rr; rr = rr->NextResponse)
		if (rr->NR_AdditionalTo && !rr->NR_AnswerTo)
		{
			u8 *p = PutResourceRecordCappedTTL(m->omsg, responseptr,
				&m->omsg->h.numAdditionals, &rr->resrec, maxttl);
			if (p)
				responseptr = p;
			/* else debugf("No more space for additionals"); */
		}

	return (responseptr);
}

mStatus mDNSPlatformSendUDP(void *msg, u8 *end, u8 *dstMacAddr, mDNSAddr *dst, mDNSIPPort dstPort);
mStatus mDNSPlatformSendUDP(void *msg, u8 *end, u8 *dstMacAddr, mDNSAddr *dst, mDNSIPPort dstPort)
{
	TXPORT_HDR	mdns_ipv4_pkt;
	TXPORT_IPV6_UDP_HDR mdns_ipv6_pkt;
	mDNS *m = mdns_share_blk->m;

	u32 len = (u8 *)end - (u8 *)msg;

	if (dst->type == mDNSAddrType_IPv4)
	{
		WL_TRACE(("%s Sending IPV4 \n", __FUNCTION__));
		memset((u8 *)&mdns_ipv4_pkt, 0, sizeof(TXPORT_HDR));

		setupUdpHeader(&mdns_ipv4_pkt, msg, IPV4_VER,
			(u8 *)m->hostIpv4Addr,
			(u8 *)&dst->ip.v4.NotAnInteger,
			MulticastDNSPort.NotAnInteger,
			dstPort.NotAnInteger,
			len + sizeof(UdpHdr_t));

		setupIpHeader(&mdns_ipv4_pkt,
			(u8 *)m->hostIpv4Addr,
			(u8 *)&dst->ip.v4.NotAnInteger,
			IP_DEFAULT_TTL,	/* 8 bit */
			PROTOCOL_UDP,	/* 8 bit */
			0,
			len + sizeof(UdpHdr_t) + sizeof(ipHdr_t));

		setupMacHeader(&mdns_ipv4_pkt.mac,
			(u8 *)&glob_mdnsi->NicMacAddr,
			(u8 *)dstMacAddr,
			PROTOCOL_IP);

		mDNS_send_packet(&mdns_ipv4_pkt, sizeof(TXPORT_HDR), msg, len);
	}
	else if (dst->type == mDNSAddrType_IPv6)
	{
		WL_TRACE(("%s Sending IPV6\n", __FUNCTION__));
		memset((u8 *)&mdns_ipv6_pkt, 0, sizeof(TXPORT_IPV6_UDP_HDR));

		setupUdpHeader(&mdns_ipv6_pkt, msg, IPV6_VER,
			(u8 *)m->hostIpv6Addr,
			(u8 *)&dst->ip.v6.b,
			MulticastDNSPort.NotAnInteger,
			dstPort.NotAnInteger,
			len + sizeof(UdpHdr_t));

		setupIpV6Header(&mdns_ipv6_pkt.ip,
			(u8 *)m->hostIpv6Addr,
			(u8 *)&dst->ip.v6.b,
			0xff,
			PROTOCOL_UDP,
			len + sizeof(UdpHdr_t));

		setupMacHeader(&mdns_ipv6_pkt.mac,
			(u8 *)&glob_mdnsi->NicMacAddr,
			(u8 *)dstMacAddr,
			PROTOCOL_IPV6);

		mDNS_send_packet(&mdns_ipv6_pkt, sizeof(TXPORT_IPV6_UDP_HDR), msg, len);
	}

	return mStatus_NoError;
}

mStatus mDNSSendDNSMessage(DNSMessage *msg, u8 *end, mDNSu8 *dstMacAddr, mDNSAddr *dst,
	mDNSIPPort dstport);
mStatus mDNSSendDNSMessage(DNSMessage *msg, u8 *end, mDNSu8 *dstMacAddr, mDNSAddr *dst,
	mDNSIPPort dstport)
{
	mStatus status;
	u16 numQuestions   = msg->h.numQuestions;
	u16 numAnswers     = msg->h.numAnswers;
	u16 numAuthorities = msg->h.numAuthorities;
	u16 numAdditionals = msg->h.numAdditionals;
	u8 *ptr = (u8 *)&msg->h.numQuestions;

	WL_TRACE(("%s\n", __FUNCTION__));

	/* Put all the integer values in IETF byte-order (MSB first, LSB second) */
	*ptr++ = (u8)(numQuestions   >> 8);
	*ptr++ = (u8)(numQuestions   &  0xFF);
	*ptr++ = (u8)(numAnswers     >> 8);
	*ptr++ = (u8)(numAnswers     &  0xFF);
	*ptr++ = (u8)(numAuthorities >> 8);
	*ptr++ = (u8)(numAuthorities &  0xFF);
	*ptr++ = (u8)(numAdditionals >> 8);
	*ptr++ = (u8)(numAdditionals &  0xFF);

	/* Send the packet on the wire */
	status = mDNSPlatformSendUDP(msg, end, dstMacAddr, dst, dstport);

	/* Put all the integer values back the way they were before we return */
	msg->h.numQuestions   = numQuestions;
	msg->h.numAnswers     = numAnswers;
	msg->h.numAuthorities = numAuthorities;
	msg->h.numAdditionals = (u16)(numAdditionals);

	return (status);
}

/* ProcessQuery examines a received query to see if we have any answers to give */
u8 *ProcessQuery(DNSMessage *query, u8 *end, mDNSu8 *srcMacAddr, mDNSAddr *srcaddr,
	bool LegacyQuery, bool QueryWasMulticast, bool QueryWasLocalUnicast);
u8 *ProcessQuery(DNSMessage *query, u8 *end, mDNSu8 *srcMacAddr, mDNSAddr *srcaddr,
	bool LegacyQuery, bool QueryWasMulticast, bool QueryWasLocalUnicast)
{
	mDNS *m = mdns_share_blk->m;
	bool        FromLocalSubnet	= AddressIsLocalSubnet(srcaddr);
	AuthRecord  *ResponseRecords	= mDNSNULL;
	AuthRecord  **nrp		= &ResponseRecords;
	s32         delayresponse	= 0;
	mDNSBool    SendLegacyResponse	= mDNSfalse;
	u8          *ptr		= query->data;
	mDNSu8      *responseptr	= mDNSNULL;
	AuthRecord  *rr;
	int	i;

	WL_TRACE(("%s: %s %d questions %d answers. Legacy %d, Multicast %d Unicast %d\n",
		__FUNCTION__, srcaddr->type == mDNSAddrType_IPv4 ? "IPV4" : "IPV6",
		query->h.numQuestions, query->h.numAnswers,
		LegacyQuery, QueryWasMulticast, QueryWasLocalUnicast));

	/* *** */
	/* *** 1. Parse Question Section and mark potential answers */
	/* *** */
	for (i= 0; i<query->h.numQuestions; i++)	/* For each question... */
	{
		bool QuestionNeedsMulticastResponse;
		int NumAnswersForThisQuestion = 0;
		AuthRecord *NSECAnswer = mDNSNULL;
		DNSQuestion pktq;

		ptr = getQuestion(query, ptr, end, &pktq);	/* get the question... */
		if (!ptr)  {
			WL_ERROR(("%s: Bailing!! getQuestion fails\n", __FUNCTION__));
			goto exit;
		}

		/*

		The only queries that *need* a multicast response are:
		* Queries sent via multicast
		* from port 5353
		* that don't have the kDNSQClass_UnicastResponse bit set

		These queries need multicast responses because other clients will:
		* suppress their own identical questions when they see these questions, and
		* expire their cache records if they don't see the expected responses

		For other queries, we may still choose to send the occasional multicast
		response anyway, to keep our neighbours caches warm, and for
		ongoing conflict detection.

		*/
		QuestionNeedsMulticastResponse =
		   QueryWasMulticast && !LegacyQuery && !(pktq.qclass & kDNSQClass_UnicastResponse);
		if (QuestionNeedsMulticastResponse)
			WL_TRACE(("%s: Got a QuestionNeedsMulticastResponse\n", __FUNCTION__));

		/* Clear the UnicastResponse flag -- don't want to confuse
		 * the rest of the code that follows later
		 */
		pktq.qclass &= ~kDNSQClass_UnicastResponse;

		/* Note:
		* We use the m->CurrentRecord mechanism here because calling
		* ResolveSimultaneousProbe can result in user callbacks which may
		* change the record list and/or question list.
		* Also note: we just mark potential answer records here, without
		* trying to build the "ResponseRecords" list, because we don't want to
		* risk user callbacks deleting records from that list while we're in
		* the middle of trying to build it.
		*/
		m->CurrentRecord = m->ResourceRecords;
		while (m->CurrentRecord)
		{
			rr = m->CurrentRecord;
			m->CurrentRecord = rr->next;
			if (AnyTypeRecordAnswersQuestion(&rr->resrec, &pktq) &&
				(QueryWasMulticast || QueryWasLocalUnicast))
			{
				if (RRTypeAnswersQuestionType(&rr->resrec, pktq.qtype))
				{
					if (ResourceRecordIsValidAnswer(rr))
					{
						NumAnswersForThisQuestion++;
/*
Notes:
NR_AnswerTo pointing into query packet
	means "answer via immediate legacy unicast" (may *also* choose to multicast)
NR_AnswerTo == (mDNSu8*)~1
	means "answer via delayed unicast" (to modern querier; may promote to multicast instead)
NR_AnswerTo == (mDNSu8*)~0
	means "definitely answer via multicast" (can't downgrade to unicast later)

If we're not multicasting this record because the
kDNSQClass_UnicastResponse bit was set, but the multicast querier is not on
a matching subnet (e.g. because of overalyed subnets on one link) then we'll
multicast it anyway (if we unicast, the receiver will ignore it because it has
an apparently non-local source)
*/

			if (QuestionNeedsMulticastResponse ||
				(!FromLocalSubnet && QueryWasMulticast && !LegacyQuery))
			{
/* We only mark this question for sending if it is at least one second since the last time */
/* we multicast it on this interface. If it is more than a second, or LastMCInterface is */
/* different, then we may multicast it.  This is to guard against the case where someone */
/* blasts us with queries as fast as they can. */
				if (QuestionNeedsMulticastResponse)
					WL_TRACE(("%s: Question needs a Multicast response\n",
						__FUNCTION__));
				if (m->timenow - (rr->LastMCTime + mDNSPlatformOneSecond) >= 0) {
					rr->NR_AnswerTo = (mDNSu8*)~0;
				} else {
					WL_TRACE(("%s: Needs multi but < 1 sec, now %d, lastMcast %d\n",
						__FUNCTION__, m->timenow, rr->LastMCTime));
				}
			}
			else if (!rr->NR_AnswerTo) {
				rr->NR_AnswerTo = LegacyQuery ? ptr : (mDNSu8*)~1;
			}

					}
				}
				else if (rr->resrec.RecordType == kDNSRecordTypeVerified)
				{
/* If we don't have any answers for this question, but we do own another record with the */
/* same name, then mark it to generate an NSEC record on this interface */
					if (!NSECAnswer) NSECAnswer = rr;
				}
			}
		}

		if (NumAnswersForThisQuestion == 0 && NSECAnswer) {
			NumAnswersForThisQuestion++;
			NSECAnswer->SendNSECNow = (mDNSInterfaceID)1;
			m->NextScheduledResponse = m->timenow;
			WL_ERROR(("%s: No answers Nextresponse = Now %d\n",
				__FUNCTION__, m->NextScheduledResponse));
		}

		/* If we couldn't answer this question, someone else might be able to, */
		/* so use random delay on response to reduce collisions */
		if (NumAnswersForThisQuestion == 0) {
			WL_TRACE(("%s: Delaying response for 1 second\n", __FUNCTION__));
			delayresponse = mDNSPlatformOneSecond;	/* Divided by 50 = 20ms */
		}

	}

	/* *** */
	/* *** 2. Now we can safely build the list of marked answers */
	/* *** */
	/* Now build our list of potential answers */
	for (rr = m->ResourceRecords; rr; rr = rr->next) {
		/* If we marked the record... */
		if (rr->NR_AnswerTo) {
			/* ... add it to the list */
			DNS_QUERY("%s Adding to response list\n", __FUNCTION__);
			AddRecordToResponseList(&nrp, rr, mDNSNULL);
		} else {
			//DNS_QUERY("%s Not AnswerTo, not adding to response list\n", __FUNCTION__);
		}
	}

	/* *** */
	/* *** 3. Add additional records */
	/* *** */
	AddAdditionalsToResponseList(ResponseRecords, &nrp);

	/* *** */
	/* *** 4. Parse Answer Section and cancel any records disallowed by Known-Answer list */
	/* *** */

	/* For each record in the query's answer section... */
	for (i = 0; i < query->h.numAnswers; i++)
	{
		ptr = GetLargeResourceRecord(query, ptr, end, kDNSRecordTypePacketAns, &m->rec);
		if (!ptr) {
			WL_ERROR(("%s: Bailing!! GetLargeResourceRecord fails\n", __FUNCTION__));
			goto exit;
		}

		/* See if this Known-Answer suppresses any of our currently planned */
		/* answers */
		for (rr = ResponseRecords; rr; rr = rr->NextResponse) {
			if (MustSendRecord(rr) && ShouldSuppressKnownAnswer(&m->rec.r, rr))
			{
				rr->NR_AnswerTo = mDNSNULL;
				rr->NR_AdditionalTo = mDNSNULL;
			}
		}

			/* See if this Known-Answer suppresses any previously scheduled */
			/* answers (for multi-packet KA suppression) */
			for (rr = m->ResourceRecords; rr; rr = rr->next) {
				/* If we're planning to send this answer on this */
				/* interface, and only on this interface, then allow KA */
				/* suppression */
				if (ShouldSuppressKnownAnswer(&m->rec.r, rr))
				{
					if (srcaddr->type == mDNSAddrType_IPv4)
					{
						if (mDNSSameIPv4Address(rr->v4Requester,
						    srcaddr->ip.v4)) {
							WL_ERROR(("%s: srcaddr->ip.v4 Matches\n",
								__FUNCTION__));
							rr->v4Requester.NotAnInteger = 0;
						}
					} else {
						if (srcaddr->type == mDNSAddrType_IPv6) {
							if (mDNSSameIPv6Address(rr->v6Requester,
							    srcaddr->ip.v6))
								memset(rr->v6Requester.b, 0,
								    sizeof(mDNSv6Addr));
						}
					}

					if (mDNSIPv4AddressIsZero(rr->v4Requester) &&
					    mDNSIPv6AddressIsZero(rr->v6Requester)) {
						WL_TRACE(("%s Unset ImmedUnicast rqstr is all 0.\n",
							__FUNCTION__));
						rr->ImmedUnicast = mDNSfalse;
					} else {
						WL_TRACE(("%s Not Changing ImmedUnicast %d\n",
							__FUNCTION__, rr->ImmedUnicast));
					}
				}
			}
			/* Clear RecordType to show we're not still using it */
			m->rec.r.resrec.RecordType = 0;
	}

	/* *** */
	/* *** 5. Cancel any additionals that were added because of now-deleted records */
	/* *** */
	for (rr = ResponseRecords; rr; rr = rr->NextResponse) {
		if (rr->NR_AdditionalTo && !MustSendRecord(rr->NR_AdditionalTo))
		{
			rr->NR_AnswerTo = mDNSNULL;
			rr->NR_AdditionalTo = mDNSNULL;
		}
	}

	/* *** */
	/* *** 6. Mark the send flags on the records we plan to send */
	/* *** */
	for (rr = ResponseRecords; rr; rr = rr->NextResponse)
	{
		if (rr->NR_AnswerTo)
		{
			/* Send modern multicast response */
			bool SendMulticastResponse = mDNSfalse;
			/* Send modern unicast response (not legacy unicast response) */
			bool SendUnicastResponse   = mDNSfalse;


			WL_TRACE(("%s: NR_AnswerTo is true\n", __FUNCTION__));
			/* If the client insists on a multicast response,
			 *   then we'd better send one
			 */
			if (rr->NR_AnswerTo == (u8*)~0) {
				SendMulticastResponse = mDNStrue;
				WL_TRACE(("%s: Selecting multicast record\n", __FUNCTION__));
			} else if (rr->NR_AnswerTo == (u8*)~1)
				SendUnicastResponse = mDNStrue;
			else if (rr->NR_AnswerTo)
				SendLegacyResponse = mDNStrue;

			if (SendLegacyResponse) {
				WL_ERROR(("%s: SendLegacyResponse\n", __FUNCTION__));
			} else {
				WL_TRACE(("%s: No SendLegacyResponse\n", __FUNCTION__));
			}

			if (SendMulticastResponse || SendUnicastResponse)
			{
				m->NextScheduledResponse = m->timenow;
				rr->SendRNow = mDNSInterfaceMark; /* Record intf to send it on */
				if (SendUnicastResponse)
					rr->ImmedUnicast = mDNStrue;

				if (srcaddr->type == mDNSAddrType_IPv4)
				{
					if (mDNSIPv4AddressIsZero(rr->v4Requester)) {
						rr->v4Requester = srcaddr->ip.v4;
						WL_TRACE(("%s: setting requestor to 0x%x\n",
							__FUNCTION__, srcaddr->ip.v4.NotAnInteger));
					} else if (!mDNSSameIPv4Address(rr->v4Requester,
					    srcaddr->ip.v4))
						rr->v4Requester = onesIPv4Addr;
				}
				else if (srcaddr->type == mDNSAddrType_IPv6)
				{
					if (mDNSIPv6AddressIsZero(rr->v6Requester))
						rr->v6Requester = srcaddr->ip.v6;
					else if (!mDNSSameIPv6Address(rr->v6Requester,
					    srcaddr->ip.v6))
						rr->v6Requester = onesIPv6Addr;
				}

				memcpy(rr->macAddrRequester, srcMacAddr,
					sizeof(rr->macAddrRequester));
			}

/*
 * If TC flag is set, it means we should expect that additional known
 * answers may be coming in another packet, so we allow roughly half a second
 * before deciding to reply (we've observed inter-packet delays of 100-200ms on 802.11)
 * else, if record is a shared one, spread responses over 100ms to avoid
 * implosion of simultaneous responses
 * else, for a simple unique record reply, we can reply immediately; no need for delay
 */

			/* Divided by 50 = 400ms */
			if  (query->h.flags.b[0] & kDNSFlag0_TC)
				delayresponse = mDNSPlatformOneSecond * 20;
			/* Divided by 50 = 20ms */
			else if (rr->resrec.RecordType == kDNSRecordTypeShared)
				delayresponse = mDNSPlatformOneSecond;
		}
		else if (rr->NR_AdditionalTo && rr->NR_AdditionalTo->NR_AnswerTo == (mDNSu8*)~0)
		{
			/* Since additional records are an optimization anyway, we only */
			/* ever send them on one interface at a time If two clients on */
			/* different interfaces do queries that invoke the same optional */
			/* additional answer, then the earlier client is out of luck */
			rr->ImmedAdditional = mDNSInterfaceMark;
			/* No need to set m->NextScheduledResponse here We'll send these */
			/* additional records when we send them, or not, as the case may */
			/* be */
		}
	}

	/* 7. If we think other machines are likely to answer these questions,
	 * set our packet suppression timer
	 */
	if (delayresponse)
		WL_TRACE(("%s: delayresponse is set\n", __FUNCTION__));

	if (delayresponse && (!m->SuppressSending ||
	    (m->SuppressSending - m->timenow) < (simple_divide(delayresponse + 49, 50))))
	{
		WL_TRACE(("%s: setting supress timer\n", __FUNCTION__));

	/*
		Pick a random delay:

		We start with the base delay chosen above (typically either 1
		second or 20 seconds), and add a random value in the range 0-5
		seconds (making 1-6 seconds or 20-25 seconds).  This is an
		integer value, with resolution determined by the platform clock
		rate.
		We then divide that by 50 to get the delay value in
		ticks. We defer the division until last to get better results
		on platforms with coarse clock granularity (e.g. ten ticks per
		second).  The +49 before dividing is to ensure we round up, not
		down, to ensure that even on platforms where the native clock
		rate is less than fifty ticks per second, we still guarantee
		that the final calculated delay is at least one platform tick.

		We want to make sure we don't ever allow the delay to be zero
		ticks, because if that happens we'll fail the Bonjour
		Conformance Test.

		Our final computed delay is 20-120ms for
		normal delayed replies, or 400-500ms in the case of
		multi-packet known-answer lists.
	*/

		m->SuppressSending =
			m->timenow + simple_divide(delayresponse +
			(s32)mDNSRandom((u32)mDNSPlatformOneSecond*5) + 49, 50);
		if (m->SuppressSending == 0) m->SuppressSending = 1;
	}

	/* 8. If query is from a legacy client, or from a new client requesting a unicast reply,
	      then generate a unicast response too
	*/
	if (SendLegacyResponse) {
		WL_TRACE(("%s: Have legacy response\n", __FUNCTION__));
		responseptr = GenerateUnicastResponse(query, end, LegacyQuery, ResponseRecords);
	}

exit:
	/* Clear RecordType to show we're not still using it */
	m->rec.r.resrec.RecordType = 0;

	/* 9. Finally, clear our link chains ready for use next time */
	while (ResponseRecords)
	{
		rr = ResponseRecords;
		ResponseRecords = rr->NextResponse;
		rr->NextResponse    = mDNSNULL;
		rr->NR_AnswerTo     = mDNSNULL;
		rr->NR_AdditionalTo = mDNSNULL;
	}

	WL_TRACE(("%s: Return %s\n", __FUNCTION__, responseptr ? "True" : "False"));
	return (responseptr);
}

void mDNSCoreReceiveQuery(DNSMessage *msg, mDNSu8 *end, mDNSu8 *srcMacAddr, mDNSAddr *srcaddr,
	mDNSIPPort srcport, mDNSAddr *dstaddr, mDNSIPPort dstport);
void mDNSCoreReceiveQuery(DNSMessage *msg, mDNSu8 *end, mDNSu8 *srcMacAddr, mDNSAddr *srcaddr,
	mDNSIPPort srcport, mDNSAddr *dstaddr, mDNSIPPort dstport)
{
	mDNS *m = mdns_share_blk->m;
	mDNSu8 	*responseend = mDNSNULL;
	bool QueryWasLocalUnicast = !mDNSAddrIsDNSMulticast(dstaddr) &&
		AddressIsLocalSubnet(srcaddr);

	responseend = ProcessQuery(msg, end, srcMacAddr, srcaddr,
		(srcport.NotAnInteger != MulticastDNSPort.NotAnInteger),
		mDNSAddrIsDNSMulticast(dstaddr), QueryWasLocalUnicast);

	/* If responseend is non-null, that means we built a unicast response packet */
	if (responseend) {
		WL_ERROR(("Calling mDNSSendDNSMessage from %s\n", __FUNCTION__));
		mDNSSendDNSMessage(m->omsg, responseend, srcMacAddr, srcaddr, srcport);
	}
}

/*
PacketRRMatchesSignature behaves as SameResourceRecordSignature, except that
types may differ if our authoratative record is unique (as opposed to shared).
For unique records, we are supposed to have complete ownership of *all* types
for this name, so *any* record type with the same name is a conflict.  In
addition, when probing we send our questions with the wildcard type
kDNSQType_ANY, so a response of any type should match, even if it is not
actually the type the client plans to use.
*/

mDNSBool PacketRRMatchesSignature(CacheRecord *pktrr, AuthRecord *authrr);
mDNSBool PacketRRMatchesSignature(CacheRecord *pktrr, AuthRecord *authrr)
{
	if (!pktrr)
		return (mDNSfalse);
	if (!authrr)
		return (mDNSfalse);
	if (!(authrr->resrec.RecordType & kDNSRecordTypeUniqueMask) &&
	    pktrr->resrec.rrtype != authrr->resrec.rrtype)
		return (mDNSfalse);

	return (mDNSBool)(pktrr->resrec.rrclass == authrr->resrec.rrclass &&
		 SameDomainName(pktrr->resrec.name, authrr->resrec.name));
}

/* See if we have an authoritative record that's identical to this packet record, */
/* whose canonical DependentOn record is the specified master record. */
/* The DependentOn pointer is typically used for the TXT record of service registrations */
/* It indicates that there is no inherent conflict detection for the TXT record */
/* -- it depends on the SRV record to resolve name conflicts */
/* If we find any identical ResourceRecords in our authoritative list, then follow */
/* their DependentOn */
/* pointer chain (if any) to make sure we reach the canonical DependentOn record */
/* If the record has no DependentOn, then just return that record's pointer */
/* Returns NULL if we don't have any local RRs that are identical to the one from the packet */
mDNSBool MatchDependentOn(CacheRecord *pktrr, AuthRecord *master);
mDNSBool MatchDependentOn(CacheRecord *pktrr, AuthRecord *master)
{
	mDNS *m = mdns_share_blk->m;
	AuthRecord *r1;
	for (r1 = m->ResourceRecords; r1; r1 = r1->next)
	{
		if (IdenticalResourceRecord(&r1->resrec, &pktrr->resrec))
		{
			AuthRecord *r2 = r1;
			while (r2->DependentOn) r2 = r2->DependentOn;
			if (r2 == master) return (mDNStrue);
		}
	}
	return (mDNSfalse);
}

/*
	PacketRRConflict is called when we've received an RR (pktrr) which has the same name
	as one of our records (our) but different rdata.
	1. If our record is not a type that's supposed to be unique, we don't care.
	2a. If our record is marked as dependent on some other record for conflict detection,
		ignore this one.
	2b. If the packet rr exactly matches one of our other RRs, and *that* record's
		DependentOn pointer points to our record, ignore this conflict
		(e.g. the packet record matches one of our TXT records, and that record is
		marked as dependent on 'our', its SRV record).
	3. If we have some *other* RR that exactly matches the one from the packet,
		and that record and our record are members of the same RRSet,
		then this is not a conflict.
*/
mDNSBool PacketRRConflict(AuthRecord *our, CacheRecord *pktrr);
mDNSBool PacketRRConflict(AuthRecord *our, CacheRecord *pktrr)
{

	/* If not supposed to be unique, not a conflict */
	if (!(our->resrec.RecordType & kDNSRecordTypeUniqueMask)) return (mDNSfalse);

	/* If a dependent record, not a conflict */
	if (our->DependentOn || MatchDependentOn(pktrr, our)) return (mDNSfalse);


	/* Okay, this is a conflict */
	return (mDNStrue);
}

void mDNSWakeUpConflictResolution(void);
void mDNSWakeUpConflictResolution(void)
{
	WL_TRACE(("%s\n", __FUNCTION__));

	/* force wake up system */
	wl_mdns_wake(WL_WOWL_MDNS_CONFLICT);

}

/*
	NOTE: mDNSCoreReceiveResponse calls mDNS_Deregister_internal which can call a
	user callback, which may change the record list and/or question list.  Any code
	walking either list must use the CurrentQuestion and/or CurrentRecord mechanism
	to protect against this.
*/
void mDNSCoreReceiveResponse(DNSMessage *response, mDNSu8 *end, mDNSu8 *srcMacAddr,
	mDNSAddr *srcaddr, mDNSIPPort srcport, mDNSAddr *dstaddr, mDNSIPPort dstport);
void mDNSCoreReceiveResponse(DNSMessage *response, mDNSu8 *end, mDNSu8 *srcMacAddr,
	mDNSAddr *srcaddr, mDNSIPPort srcport, mDNSAddr *dstaddr, mDNSIPPort dstport)
{
	int i;
	mDNS *m = mdns_share_blk->m;

	/* We ignore questions (if any) in a DNS response packet */
	mDNSu8 *ptr = LocateAnswers(response, end);

/*
	All records in a DNS response packet are treated as equally valid
	statements of truth. If we want to guard against spoof responses, then
	the only credible protection against that is cryptographic security,
	e.g. DNSSEC., not worring about which section in the spoof packet
	contained the record
*/

	int totalrecords = response->h.numAnswers + response->h.numAuthorities +
		response->h.numAdditionals;

	(void)srcaddr;	/* Currently used only for display in debugging message */
	(void)srcport;
	(void)dstport;

	WL_TRACE(("%s\n", __FUNCTION__));

	/* If we get a unicast response when we weren't expecting one,
		then we assume it is someone trying to spoof us.
	*/
	if (!mDNSAddrIsDNSMulticast(dstaddr))
	{
		WL_ERROR(("%s: Have a non - multicast\n", __FUNCTION__));
		if (!AddressIsLocalSubnet(srcaddr))
			return;
		/* For now we don't put standard wide-area unicast responses in our main cache */
		/* (Later we should fix this and cache all known results in a unified manner.) */
		if (response->h.id.NotAnInteger != 0 ||
			srcport.NotAnInteger != MulticastDNSPort.NotAnInteger)
			return;
	}

	for (i = 0; i < totalrecords && ptr && ptr < end; i++)
		{
		mDNSu8 RecordType = (mDNSu8)((i < response->h.numAnswers) ?
			kDNSRecordTypePacketAns : kDNSRecordTypePacketAdd);

		ptr = GetLargeResourceRecord(response, ptr, end, RecordType, &m->rec);
		/* Break out of the loop and clean up our CacheFlushRecords list before exiting */
		if (!ptr) goto exit;

		/* Don't want to cache OPT or TSIG pseudo-RRs */
		if (m->rec.r.resrec.rrtype == kDNSType_OPT ||
			m->rec.r.resrec.rrtype == kDNSType_TSIG)
		{
			m->rec.r.resrec.RecordType = 0;
			continue;
		}

		/* 1. Check that this packet resource record does not conflict with any of ours */
		if (mDNSOpaque16IsZero(response->h.id) && m->rec.r.resrec.rrtype != kDNSType_NSEC)
		{
			m->CurrentRecord = m->ResourceRecords;
			while (m->CurrentRecord)
			{
				AuthRecord *rr = m->CurrentRecord;
				m->CurrentRecord = rr->next;
				/* If interface, name, type (if shared record) and class match... */
				if (PacketRRMatchesSignature(&m->rec.r, rr))
				{
					/* else, the packet RR has different type or different
					   rdata -- check to see if this is a conflict.
					*/
					if (m->rec.r.resrec.rroriginalttl > 0 &&
						PacketRRConflict(rr, &m->rec.r))
					{
						/* If this record is marked DependentOn another
						   record for conflict detection purposes,
						   then *that* record has to be bumped back to
						   probing state to resolve the conflict
						*/
						while (rr->DependentOn) rr = rr->DependentOn;

						/* If we'd previously verified this record, put it
						   back to probing state and try again
						*/
						if (rr->resrec.RecordType == kDNSRecordTypeVerified)
						{
					WL_ERROR(("%s: Wakeup due to kDNSRecordTypeVerified\n",
						__FUNCTION__));
							mDNSWakeUpConflictResolution();
						}
						/* If we're probing for this record,
						      we just failed
						*/
						else if (rr->resrec.RecordType ==
							kDNSRecordTypeUnique)
						{
					WL_ERROR(("%s: Wakeup due to kDNSRecordTypeUnique\n",
						__FUNCTION__));
							mDNSWakeUpConflictResolution();
						}
						/*
						We assumed this record must be unique,
						but we were wrong.  (e.g. There are two
						mDNSResponders on the same machine
						giving different answers for the
						reverse mapping record.) This is simply
						a misconfiguration, and we don't try to
						recover from it.
						*/
						else if (rr->resrec.RecordType ==
							kDNSRecordTypeKnownUnique)
						{
					WL_ERROR(("%s: Wakeup due to kDNSRecordTypeKnownUnique\n",
						__FUNCTION__));
							mDNSWakeUpConflictResolution();
						}
					}
					/*
					Else, matching signature, different type or rdata, but
					not a considered a conflict.  If the packet record has
					the cache-flush bit set, then we check to see if we
					have any record(s) of the same type that we should
					re-assert to rescue them (see note about "multi-homing
					and bridged networks" at the end of this function).
					*/
					else if (m->rec.r.resrec.rrtype == rr->resrec.rrtype)
		if ((m->rec.r.resrec.RecordType & kDNSRecordTypePacketUniqueMask) &&
			m->timenow - rr->LastMCTime > mDNSPlatformOneSecond/2)
						{
							m->NextScheduledResponse = m->timenow;
					WL_ERROR(("%s Set NextScheduledResponse to now %d\n",
						__FUNCTION__, m->NextScheduledResponse));
						}
				}
			}
		}

		/* Clear RecordType to show we're not still using it */
		m->rec.r.resrec.RecordType = 0;
		}

exit:
	/* Clear RecordType to show we're not still using it */
	m->rec.r.resrec.RecordType = 0;
}

bool mDNSAddrIsSecondary(mDNSAddr *ip);
bool mDNSAddrIsSecondary(mDNSAddr *ip)
{
	int i;
	mDNS *m = mdns_share_blk->m;

	switch (ip->type)
	{
		case mDNSAddrType_IPv4:
		{
			mDNSu32	*secondaryIpAddr = (mDNSu32 *)m->secondaryIpv4Addr;

			for (i = 0; i < m->numSecondaryIpv4Addr; i++)
				if (ip->ip.v4.NotAnInteger == secondaryIpAddr[i])
					return (mDNStrue);
		}
		break;

		case mDNSAddrType_IPv6:
		{
			mDNSv6Addr	*secondaryIpAddr = (mDNSv6Addr *)m->secondaryIpv6Addr;

			for (i = 0; i < m->numSecondaryIpv6Addr; i++)
				if (!memcmp(ip->ip.v6.b, secondaryIpAddr[i].b, sizeof(mDNSv6Addr)))
					return (mDNStrue);
		}
		break;
	}

	return (mDNSfalse);
}

void mDNSCoreReceive(void *pkt, u8 *end, u8 *srcMacAddr, mDNSAddr *srcaddr,
	mDNSIPPort srcport, mDNSAddr *dstaddr, mDNSIPPort dstport)
{
	mDNS *m = mdns_share_blk->m;
	DNSMessage	*msg	= (DNSMessage *)pkt;
	u8 			StdQ 	= kDNSFlag0_QR_Query    | kDNSFlag0_OP_StdQuery;
	mDNSu8 		StdR 	= kDNSFlag0_QR_Response | kDNSFlag0_OP_StdQuery;
	u8 			*ptr 	= mDNSNULL;
	u8 			QR_OP;

	WL_TRACE(("Enter %s\n", __FUNCTION__));
	if ((unsigned)(end - (u8 *)pkt) < sizeof(DNSMessageHeader)) {
		 WL_ERROR(("%s: DNS Message too short\n", __FUNCTION__));
		 return;
	}

	QR_OP = (u8)(msg->h.flags.b[0] & kDNSFlag0_QROP_Mask);

	/* Read the integer parts which are in IETF byte-order (MSB first, LSB second) */
	ptr = (u8 *)&msg->h.numQuestions;
	msg->h.numQuestions   = (u16)((u16)ptr[0] <<  8 | ptr[1]);
	msg->h.numAnswers     = (u16)((u16)ptr[2] <<  8 | ptr[3]);
	msg->h.numAuthorities = (u16)((u16)ptr[4] <<  8 | ptr[5]);
	msg->h.numAdditionals = (u16)((u16)ptr[6] <<  8 | ptr[7]);

	WL_TRACE(("%s: Questions %d, answers %d, Authorities %d, Additionals %d\n", __FUNCTION__,
		msg->h.numQuestions, msg->h.numAnswers, msg->h.numAuthorities,
		msg->h.numAdditionals));
	if (!m)
		return;

	/* We use zero addresses and all-ones addresses at various places in the code to */
	/* indicate special values like "no address" If we accept and try to process a */
	/* packet with zero or all-ones source address, that could really mess things up */
	if (!mDNSAddressIsValid(srcaddr)) {
		WL_ERROR(("%s: !!invalid address\n", __FUNCTION__));
		return;
	}

	if (mDNSAddrIsSecondary(srcaddr)) {
		WL_ERROR(("%s: !!address is secondary\n", __FUNCTION__));
		return;
	}

	mDNS_Lock();
	m->PktNum++;

	if (QR_OP == StdQ)
		mDNSCoreReceiveQuery(msg, end, srcMacAddr, srcaddr, srcport, dstaddr, dstport);
	else if (QR_OP == StdR)
		mDNSCoreReceiveResponse(msg, end, srcMacAddr, srcaddr, srcport, dstaddr, dstport);

	mDNS_Unlock();
}

void SendDelayedUnicastResponse(mDNSu8 *destMacAddr, mDNSAddr *dest);
void SendDelayedUnicastResponse(mDNSu8 *destMacAddr, mDNSAddr *dest)
{
	mDNS *m = mdns_share_blk->m;
	AuthRecord *rr;
	AuthRecord  *ResponseRecords = mDNSNULL;
	AuthRecord **nrp             = &ResponseRecords;

	WL_TRACE(("%s\n", __FUNCTION__));

	/* Make a list of all our records that need to be unicast to this destination */
	for (rr = m->ResourceRecords; rr; rr = rr->next)
	{
		/* If we find we can no longer unicast this answer, clear ImmedUnicast */
		if (mDNSSameIPv4Address(rr->v4Requester, onesIPv4Addr) ||
			mDNSSameIPv6Address(rr->v6Requester, onesIPv6Addr))
			rr->ImmedUnicast = mDNSfalse;

		if (rr->ImmedUnicast)
			if ((dest->type == mDNSAddrType_IPv4 &&
				mDNSSameIPv4Address(rr->v4Requester, dest->ip.v4)) ||
				(dest->type == mDNSAddrType_IPv6 &&
				mDNSSameIPv6Address(rr->v6Requester, dest->ip.v6)))
			{
				rr->SendRNow = mDNSNULL;
				rr->ImmedUnicast = mDNSfalse;
				rr->v4Requester.NotAnInteger  = 0;
				memset(rr->v6Requester.b, 0, sizeof(mDNSv6Addr));

				/* rr->NR_AnswerTo */
				if (rr->NextResponse == mDNSNULL && nrp != &rr->NextResponse)
				{
					rr->NR_AnswerTo = (mDNSu8*)~0;
					*nrp = rr; nrp = &rr->NextResponse;
				}
			}
	}

	AddAdditionalsToResponseList(ResponseRecords, &nrp);

	while (ResponseRecords)
	{
		u8 *responseptr = m->omsg->data;
		u8 *newptr;

		InitializeDNSMessage(&m->omsg->h, zeroID, ResponseFlags);

		/* Put answers in the packet */
		while (ResponseRecords && ResponseRecords->NR_AnswerTo) {
			rr = ResponseRecords;
			if (rr->resrec.RecordType & kDNSRecordTypeUniqueMask)
				/* Temporarily set the cache flush bit so
				   PutResourceRecord will set it
				*/
				rr->resrec.rrclass |= kDNSClass_UniqueRRSet;
			newptr = PutResourceRecord(m->omsg, responseptr,
				&m->omsg->h.numAnswers, &rr->resrec);

			/* Make sure to clear cache flush bit back to normal state */
			rr->resrec.rrclass &= ~kDNSClass_UniqueRRSet;

			if (!newptr && m->omsg->h.numAnswers)
				break;	/* If packet full, send it now */

			if (newptr)
				responseptr = newptr;

			ResponseRecords = rr->NextResponse;
			rr->NextResponse    = mDNSNULL;
			rr->NR_AnswerTo     = mDNSNULL;
			rr->NR_AdditionalTo = mDNSNULL;
		}

		/* Add additionals, if there's space */
		while (ResponseRecords && !ResponseRecords->NR_AnswerTo) {
			rr = ResponseRecords;
			if (rr->resrec.RecordType & kDNSRecordTypeUniqueMask)
				/* Temporarily set the cache flush bit so PutResourceRecord
				   will set it
				*/
				rr->resrec.rrclass |= kDNSClass_UniqueRRSet;
			newptr = PutResourceRecord(m->omsg, responseptr,
				&m->omsg->h.numAdditionals, &rr->resrec);
			/* Make sure to clear cache flush bit back to normal state */
			rr->resrec.rrclass &= ~kDNSClass_UniqueRRSet;

			if (newptr)
				responseptr = newptr;

			ResponseRecords = rr->NextResponse;
			rr->NextResponse    = mDNSNULL;
			rr->NR_AnswerTo     = mDNSNULL;
			rr->NR_AdditionalTo = mDNSNULL;
		}

		if (m->omsg->h.numAnswers) {
			WL_TRACE(("Calling mDNSSendDNSMessage from %s\n", __FUNCTION__));
			mDNSSendDNSMessage(m->omsg, responseptr, destMacAddr, dest,
				MulticastDNSPort);
		}
	}
}

/* NOTE: SendResponses calls mDNS_Deregister_internal which can call a user callback, */
/* which may change the record list and/or question list.  Any code walking either list */
/* must use the CurrentQuestion and/or CurrentRecord mechanism to protect against this. */
void SendResponses(void);
void SendResponses(void)
{
	mDNS *m = mdns_share_blk->m;
	int pktcount = 0;
	AuthRecord *rr, *r2;

	WL_TRACE(("%s, setting next response to INFINITY\n", __FUNCTION__));

	m->NextScheduledResponse = m->timenow + DNS_INFINITY;

	for (rr = m->ResourceRecords; rr; rr = rr->next) {
		if (rr->ImmedUnicast) {
			WL_TRACE(("%s: rr->ImmedUnicast\n", __FUNCTION__));
			mDNSAddr v4 = { mDNSAddrType_IPv4, {{{0}}} };
			mDNSAddr v6 = { mDNSAddrType_IPv6, {{{0}}} };
			mDNSu8	 macAddr[6];

			v4.ip.v4 = rr->v4Requester;
			v6.ip.v6 = rr->v6Requester;
			memcpy(macAddr, rr->macAddrRequester, sizeof(rr->macAddrRequester));
			if (!mDNSIPv4AddressIsZero(rr->v4Requester))
				SendDelayedUnicastResponse(macAddr, &v4);
			if (!mDNSIPv6AddressIsZero(rr->v6Requester))
				SendDelayedUnicastResponse(macAddr, &v6);
			if (rr->ImmedUnicast) {
				/* LogMsg("SendResponses: ERROR: rr->ImmedUnicast still set: %s",
					ARDisplayString(m, rr));
				*/
				rr->ImmedUnicast = mDNSfalse;
			}
		}
	}

	/* *** 1. Setup: Set the SendRNow and ImmedAnswer fields to indicate which interface(s)
		the records need to be sent on
	*/

	/* *** 2. Loop through interface list, sending records as appropriate */

	while (1)
	{
		int numAnswer   = 0;
		u8 *responseptr = m->omsg->data;
		u8 *newptr;
		int sent_one = 0;
		InitializeDNSMessage(&m->omsg->h, zeroID, ResponseFlags);

		/* First Pass. Look for: */
		/* 1. Deregistering records that need to send their goodbye packet */
		/* 2. Updated records that need to retract their old data */
		/* 3. Answers and announcements we need to send */
		/* In all cases, if we fail, and we've put at least one answer, we break out */
		/* of the for loop so we can send this packet and then try again. */
		/* If we have not put even one answer, then we don't bail out. We pretend */
		/* we succeeded anyway, */
		/* because otherwise we'll end up in an infinite loop trying to send a record */
		/* that will never fit. */
		for (rr = m->ResourceRecords; rr; rr = rr->next)  {
			if (rr->SendRNow == mDNSInterfaceMark) {
				rr->LastMCTime = m->timenow;
				/* Temporarily set the cache flush bit so PutResourceRecord
					will set it
				*/
				if (rr->resrec.RecordType & kDNSRecordTypeUniqueMask)
					rr->resrec.rrclass |= kDNSClass_UniqueRRSet;
				newptr = PutResourceRecordTTL(m->omsg, responseptr,
					&m->omsg->h.numAnswers, &rr->resrec,
					rr->resrec.rroriginalttl);
				/* Make sure to clear cache flush bit back to normal state */
				rr->resrec.rrclass &= ~kDNSClass_UniqueRRSet;
				if (!newptr && m->omsg->h.numAnswers) break;
				numAnswer++;
				responseptr = newptr;

		/* The first time through (pktcount== 0), if this record is verified unique */
		/* (i.e. typically A, AAAA, SRV and TXT), set the flag to add an NSEC too. */
				if (!pktcount && rr->resrec.RecordType == kDNSRecordTypeVerified &&
				    !rr->SendNSECNow)
					rr->SendNSECNow = (mDNSInterfaceID)1;

				/* If succeeded in sending, advance to next interface */
				if (newptr)
					rr->SendRNow = mDNSNULL;
			}
		}

		/* Second Pass. Add additional records, if there's space. */
		newptr = responseptr;
		for (rr = m->ResourceRecords; rr; rr = rr->next)
			if (rr->ImmedAdditional == mDNSInterfaceMark)
				if (ResourceRecordIsValidAnswer(rr))
				{
					/* If we have at least one answer already in the packet, */
					/* then plan to add additionals too */
					mDNSBool SendAdditional = (m->omsg->h.numAnswers > 0);

				/* If we're not planning to send any additionals, but this */
				/* record is a unique one, then make sure we haven't */
				/* already sent any other members of its RRSet -- if we */
				/* have, then they will have had the cache flush bit set, */
				/* so now we need to finish the job and send the rest. */
					if (!SendAdditional && (rr->resrec.RecordType &
					kDNSRecordTypeUniqueMask))
					{
						AuthRecord *a;
						for (a = m->ResourceRecords; a; a = a->next)
							if (a->LastMCTime == m->timenow &&
								SameResourceRecordSignature(a, rr))
							{
								SendAdditional = mDNStrue;
								break;
							}
					}

					/* If we don't want to send this after all, */
					if (!SendAdditional)
						/* then cancel its ImmedAdditional field */
						rr->ImmedAdditional = mDNSNULL;
					else if (newptr)
						/* Else, try to add it if we can */
					{
		/* The first time through (pktcount== 0), if this record is verified unique */
		/* (i.e. typically A, AAAA, SRV and TXT), set the flag to add an NSEC too. */
						if (!pktcount && rr->resrec.RecordType ==
							kDNSRecordTypeVerified && !rr->SendNSECNow)
							rr->SendNSECNow = (mDNSInterfaceID)1;

						if (rr->resrec.RecordType &
							kDNSRecordTypeUniqueMask)
		/* Temporarily set the cache flush bit so PutResourceRecord will set it */
							rr->resrec.rrclass |= kDNSClass_UniqueRRSet;
						newptr = PutResourceRecord(m->omsg, newptr,
							&m->omsg->h.numAdditionals, &rr->resrec);
		/* Make sure to clear cache flush bit back to normal state */
						rr->resrec.rrclass &= ~kDNSClass_UniqueRRSet;
						if (newptr)
						{
							responseptr = newptr;
							rr->ImmedAdditional = mDNSNULL;
			/* If we successfully put this additional record in */
			/* the packet, we record LastMCTime & */
			/* LastMCInterface.  This matters particularly in */
			/* the case where we have more than one IPv6 (or */
			/* IPv4) address, because otherwise, when we see */
			/* our own multicast with the cache flush bit set, */
			/* if we haven't set LastMCTime, then we'll get all */
			/* concerned and re-announce our record again to */
			/* make sure it doesn't get flushed from peer */
			/* caches. */
							rr->LastMCTime      = m->timenow;
						}
					}
				}

		/* Third Pass. Add NSEC records, if there's space. */
		for (rr = m->ResourceRecords; rr; rr = rr->next)
			if (rr->SendNSECNow == (mDNSInterfaceID)1)
			{
				AuthRecord nsec;
				WL_TRACE(("%s: SendNSECNow  == 1\n", __FUNCTION__));

				mDNS_SetupResourceRecord(&nsec, kDNSType_NSEC,
					rr->resrec.rroriginalttl, kDNSRecordTypeUnique);
				nsec.resrec.rrclass |= kDNSClass_UniqueRRSet;
				AssignDomainName(&m->rec.namestorage, rr->resrec.name);
				nsec.resrec.name = &m->rec.namestorage;
				nsec.rdatastorage.u.nsec = &m->nsecStorage;
				memset(nsec.rdatastorage.u.nsec, 0, sizeof(rdataNSEC));
				for (r2 = m->ResourceRecords; r2; r2 = r2->next) {
					if (ResourceRecordIsValidAnswer(r2) &&
						SameResourceRecordNameClassInterface(r2, rr))
					{
						if (r2->resrec.rrtype >= kDNSQType_ANY)
						{
						WL_ERROR(("%s: Can't create NSEC for record\n",
							__FUNCTION__));
							break;
						} else {
			nsec.rdatastorage.u.nsec->bitmap[r2->resrec.rrtype >> 3] |= 128 >>
				(r2->resrec.rrtype & 7);
						}
					}
				}
				newptr = responseptr;
				/* If we successfully built our NSEC record, add it
					to the packet now
				*/
				if (!r2)
				{
					newptr = PutResourceRecord(m->omsg, newptr,
						&m->omsg->h.numAdditionals, &nsec.resrec);
					if (newptr) responseptr = newptr;
				} else {
					WL_ERROR(("%s: Failed to build our NSEC record\n",
						__FUNCTION__));
				}

				/* If we successfully put the NSEC record, clear the */
				/* SendNSECNow flag If we consider this NSEC optional, then we */
				/* unconditionally clear the SendNSECNow flag, even if we fail */
				/* to put this additional record */
				if (newptr || rr->SendNSECNow == (mDNSInterfaceID)1)
				{
					rr->SendNSECNow = mDNSNULL;
					/* Run through remainder of list clearing SendNSECNow */
					/* for all other records which would generate same NSEC */
					for (r2 = rr->next; r2; r2 = r2->next) {
						if (SameResourceRecordNameClassInterface(r2, rr)) {
							if (r2->SendNSECNow == (mDNSInterfaceID)1) {
								r2->SendNSECNow = mDNSNULL;
							}
						}
					}
				}
			}

		if (m->omsg->h.numAnswers > 0 || m->omsg->h.numAdditionals)
		{
			if (m->IPv4Available) {
				WL_TRACE(("%s: Have IPV4, Sending Multicast\n", __FUNCTION__));
				sent_one++;
				mDNSSendDNSMessage(m->omsg, responseptr, MulicastV4EthAddr.b,
					&AllDNSLinkGroup_v4, MulticastDNSPort);
			} else {
				WL_ERROR(("%s: IPv4 unavailable to send\n", __FUNCTION__));
			}
			if (m->IPv6Available) {
				WL_TRACE(("%s: Have IPV6, Sending Multicast\n", __FUNCTION__));
				sent_one++;
				mDNSSendDNSMessage(m->omsg, responseptr, MulicastV6EthAddr.b,
					&AllDNSLinkGroup_v6, MulticastDNSPort);
			} else {
				WL_ERROR(("%s: IPv6 unavailable to send\n", __FUNCTION__));
			}

			if (!m->SuppressSending) {
				m->SuppressSending =
					NonZeroTime(m->timenow + (mDNSPlatformOneSecond+9)/10);
				WL_ERROR(("%s: SupressSending for %d msecs\n",
					__FUNCTION__, m->SuppressSending - m->timenow));
			}

			if (++pktcount >= 1000)
				break;
		} else {
			if (!sent_one) {
				WL_TRACE(("%s: ** No more answers or additionals and didn't get to send any!!\n",
					__FUNCTION__));
			}
			break;
		}
	}

	/* 3. Cleanup: Now that everything is sent, call client callback functions,
		and reset state variables
	*/

	m->CurrentRecord = m->ResourceRecords;
	while (m->CurrentRecord)
	{
		rr = m->CurrentRecord;
		m->CurrentRecord = rr->next;

		if (rr->SendRNow)
		{
			rr->SendRNow = mDNSNULL;
			rr->v4Requester.NotAnInteger  = 0;
			memset(rr->v6Requester.b, 0, sizeof(mDNSv6Addr));
			memset(rr->macAddrRequester, 0, sizeof(rr->macAddrRequester));
		}

		if (rr->ImmedAnswer)
		{
			rr->ImmedAnswer  = mDNSNULL;
			rr->v4Requester.NotAnInteger  = 0;
			memset(rr->v6Requester.b, 0, sizeof(mDNSv6Addr));
		}
	}
	WL_TRACE(("%s: Exit\n", __FUNCTION__));
}

s32 mDNS_Execute()
{
	mDNS *m = mdns_share_blk->m;
	int32 nexttime;

	DNS_TIME("Enter %s RawTime  %d timenow %d\n", __FUNCTION__, mDNSPlatformRawTime(), m->timenow);
	/* m->timenow will be zero since we are not in lock */
	if (m->signature != BONJOUR_PROXY_SIG) {
		WL_ERROR(("%s: mdns not initt'ed, restart timer\n", __FUNCTION__));
		return 20; /* Wait a while and try again */
	}

	if ((m->timenow_last > 100) && !glob_mdnsi->mDnsGratuitousArpSent) {
		u8 bcast[6];
		u8 *pkt = glob_mdnsi->glob_txPktBuff;
		WL_TRACE(("%s: Sending Gratuitous ARP REQ\n", __FUNCTION__));

		memset(bcast, 0xFF, 6);
		setupArpPacket(pkt,
		   ARP_REQUEST,
		   glob_mdnsi->NicMacAddr,
		   bcast,
		   m->hostIpv4Addr,
		   m->hostIpv4Addr);

		mDNS_send_packet(NULL, 0, pkt, sizeof(MAC_HDR) + sizeof(ARP_PKT));
		glob_mdnsi->mDnsGratuitousArpSent = mDNStrue;
	}

	mDNS_Lock();	/* Must grab lock before trying to read m->timenow */

	if (m->timenow - m->NextScheduledEvent >= 0) {
		/* 5. See what packets we need to send */
		if (m->SuppressSending == 0 || m->timenow - m->SuppressSending >= 0) {
			DNS_TIME("%s: suppress is ready to send\n", __FUNCTION__);
			/* If the platform code is ready, and we're not suppressing packet */
			/* generation right now then send our responses, probes, and */
			/* questions.  We check the cache first, because there might be */
			/* records close to expiring that trigger questions to refresh */
			/* them.  We send queries next, because there might be final-stage */
			/* probes that complete their probing here, causing them to advance */
			/* to announcing state, and we want those to be included in any */
			/* announcements we send out.  Finally, we send responses, */
			/* including the previously mentioned records that just completed */
			/* probing. */
			m->SuppressSending = 0;

			/* 7. Send Response packets, including probing records just advanced
				to announcing state
			*/
			if (m->timenow - m->NextScheduledResponse >= 0) {
				SendResponses();
			} else {
		DNS_TIME("%s: Too soon for response, timenow %d, NextScheduledResponse %d\n",
			__FUNCTION__, m->timenow, m->NextScheduledResponse);
			}
			if (m->timenow - m->NextScheduledResponse >= 0) {
				/* SendResponses didn't send all its responses;
					try again in 1 second
				*/
				m->NextScheduledResponse = m->timenow + mDNSPlatformOneSecond;
				DNS_TIME("%s Set NextScheduledResponse + one sec %d\n",
					__FUNCTION__, m->NextScheduledResponse);
			}
		} else {
			DNS_TIME("%s: Not end of supression yet. timenow %d, SuppressSending %d\n",
				__FUNCTION__, m->timenow, m->SuppressSending);
		}
	} else {
		WL_ERROR(("%s: Not NextScheduledEvent yet. timenow %d, NextScheduled %d\n",
			__FUNCTION__, m->timenow, m->NextScheduledEvent));
	}

	mDNS_Unlock();	/* Calling mDNS_Unlock is what gives m->NextScheduledEvent its new value */

	nexttime = (mDNSPlatformRawTime() - m->NextScheduledEvent);

	DNS_TIME("%s: Next %d - now %d = Next time %d msecs\n", __FUNCTION__,
		m->NextScheduledEvent, mDNSPlatformRawTime(), nexttime);

	if (nexttime <= 0) {
		if (nexttime < 0)
			WL_ERROR(("%s: Time went backward!\n", __FUNCTION__));

		nexttime = MDNS_DEFAULT_TIME;
	}

	return (nexttime);
}

void mdns_sd_main_init(void)
{
	glob_mdnsi->seed = 0;
	glob_mdnsi->mDnsGratuitousArpSent = mDNSfalse;
}

/* prot.c  */

u32     *ptr;

/* local routines */

u32 compute16BitSum(u8 *val, u32 count);
u16 compute16BitChecksum(u16 *val16, u32 count, u32 sum);

u32 compute16BitSum(u8 *val, u32 count)
{
	u32 sum, i;
	u16 *val16;

	/* assuming count is even */
	WL_TRACE(("%s: bytes=%d ", __FUNCTION__, count));

	if (count & 1)
		WL_ERROR(("%s: ODD COUNT\n", __FUNCTION__));

	sum = 0;
	val16 = (u16 *) val;
	count /= 2;

	for (i = 0; i < count; i++)
		sum += *val16++;

	WL_TRACE(("%s: sum=%x\n", __FUNCTION__, sum));
	return (sum);
}

u16 compute16BitChecksum(u16 *val16, u32 count, u32 sum)
{
	WL_TRACE(("compute16BitChecksum\n"));
	while (count > 1) {
		sum += *val16++;
		count -= 2;
	}
	/*  Add left-over byte, if any */
	if (count > 0)
		sum += *(u8 *)val16;

	/*  Fold 32-bit sum to 16 bits */
	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);

	return (~sum);
}

void setupMacHeader(pMAC_HDR pkt, u8 *srcMacAddr, u8 *destMacAddr, u16 frame_type)
{
	WL_TRACE(("%s frame_type = 0x%x\n", __FUNCTION__, frame_type));
	memcpy(pkt->src_addr, srcMacAddr, 6);
	memcpy(pkt->dest_addr, destMacAddr, 6);
	pkt->fr_type = hton16(frame_type);
}

void setupIpHeader(pTXPORT_HDR pkt, u8 *srcIpAddr, u8 *destIpAddr, u8 ttl,
	u8 upper_protocol, u16 frameId, u16 pktLen)
{
	WL_TRACE(("%s pktLen=%d\n", __FUNCTION__, pktLen));
	pkt->ip.ver_hdr_len = 5 | (IPV4_VER << 4);
	pkt->ip.svc_type = 0;

	if (upper_protocol == PROTOCOL_UDP) {
		pkt->ip.tot_len = hton16(sizeof(ipHdr_t) + ntoh16(pkt->txp.udp.len));
	} else {
		/* we will change this later */
		pkt->ip.tot_len = hton16(pktLen - sizeof(MAC_HDR));
	}

	pkt->ip.id = hton16(frameId);

	pkt->ip.flags_frag_offset = 0;
	pkt->ip.time_to_live = ttl;
	pkt->ip.protocol = upper_protocol;
	pkt->ip.hdr_checksum = 0;
	memcpy(pkt->ip.src_addr, srcIpAddr, 4);
	memcpy(pkt->ip.dest_addr, destIpAddr, 4);

	pkt->ip.hdr_checksum = compute16BitChecksum((u16 *) &pkt->ip, sizeof(ipHdr_t), 0);
}

void setupUdpHeader(void *phdr, void *pdata, u8 ipVer, u8 *srcIp, u8 *destIp,
	u16 srcPort, u16 destPort, u16 udpLen)
{
	u32 sum = 0;
	struct {
		u8 source_ip[4];
		u8 dest_ip[4];
		u8 zero;
		u8 proto;
		u16 udp_len;
	} pseudo_hdr;

	pTXPORT_HDR pIpv4Hdr;
	IPV6_PSEUDO_HDR	ipV6Pseudo;
	pTXPORT_IPV6_UDP_HDR pIpv6Hdr;

	WL_TRACE(("%s srcport %d, dstport %d\n", __FUNCTION__, ntoh16(srcPort), ntoh16(destPort)));

	if (ipVer == IPV4_VER) {
		WL_ERROR(("%s IPV4 src %d.%d.%d.%d:%d, dst %d.%d.%d.%d:%d\n", __FUNCTION__,
			srcIp[0], srcIp[1], srcIp[2], srcIp[3], ntoh16(srcPort),
			destIp[0], destIp[1], destIp[2], destIp[3], ntoh16(destPort)));
		memcpy(pseudo_hdr.source_ip, srcIp, 4);
		memcpy(pseudo_hdr.dest_ip, destIp, 4);
		pseudo_hdr.zero = 0;
		pseudo_hdr.proto = PROTOCOL_UDP;
		pseudo_hdr.udp_len = hton16(udpLen);
		sum = compute16BitSum((u8 *)&pseudo_hdr, sizeof(pseudo_hdr));

		pIpv4Hdr = (TXPORT_HDR *)phdr;
		pIpv4Hdr->txp.udp.src_port = srcPort;   /* Port nums already in network order */
		pIpv4Hdr->txp.udp.dest_port = destPort;
		pIpv4Hdr->txp.udp.checksum = 0;
		pIpv4Hdr->txp.udp.len = hton16(udpLen);
		if (pdata != NULL)
		{
			sum += compute16BitSum((u8 *)&pIpv4Hdr->txp.udp, sizeof(UdpHdr_t));
			pIpv4Hdr->txp.udp.checksum = compute16BitChecksum((u16 *)pdata,
			udpLen - sizeof(UdpHdr_t), sum);
		} else {
			pIpv4Hdr->txp.udp.checksum = compute16BitChecksum((u16
			*)&pIpv4Hdr->txp.udp, udpLen, sum);
		}
	}
	else if (ipVer == IPV6_VER) {
		WL_ERROR(("%s IPV6 src ", __FUNCTION__));
		print_real_raw(srcIp, 8);
		WL_ERROR((":%d dst ", ntoh16(srcPort)));
		print_real_raw(destIp, 8);
		WL_ERROR((":%d\n", ntoh16(destPort)));

		memcpy(ipV6Pseudo.dest_ip, destIp, 16);
		memcpy(ipV6Pseudo.source_ip, srcIp, 16);
		ipV6Pseudo.upper_layer_len = hton32(udpLen);
		ipV6Pseudo.zero_next_hdr = hton32(PROTOCOL_UDP);
		sum = compute16BitSum((u8 *)&ipV6Pseudo, sizeof(ipV6Pseudo));

		pIpv6Hdr = (TXPORT_IPV6_UDP_HDR *)phdr;
		pIpv6Hdr->udp.src_port = srcPort;
		pIpv6Hdr->udp.dest_port = destPort;
		pIpv6Hdr->udp.checksum = 0;
		pIpv6Hdr->udp.len = hton16(udpLen);
		if (pdata != NULL)
		{
			sum += compute16BitSum((u8 *)&pIpv6Hdr->udp, sizeof(UdpHdr_t));
			pIpv6Hdr->udp.checksum = compute16BitChecksum((u16 *)pdata,
			udpLen - sizeof(UdpHdr_t), sum);
		} else {
			pIpv6Hdr->udp.checksum = compute16BitChecksum((u16
			*)&pIpv6Hdr->udp, udpLen, sum);
		}
	}
}

void setupArpPacket(u8 *packet, u16 opcode, u8 *srcMacAddr, u8 *destMacAddr,
	u8 *srcIpAddr, u8 *destIpAddr)
{
    ARP_PKT *arp;
    TXPORT_HDR *pkt;

    pkt = (TXPORT_HDR *) packet;
    arp = (ARP_PKT *) &pkt->ip;

    /*WL_ERROR(("%s src IP %d %d %d %d\n", */
    /*	__FUNCTION__, srcIpAddr[0], srcIpAddr[1],srcIpAddr[2],srcIpAddr[3]); */
    /*WL_ERROR(("%s dst IP %d %d %d %d\n", */
    	/*__FUNCTION__, destIpAddr[0], destIpAddr[1],destIpAddr[2],destIpAddr[3]); */

    memcpy(pkt->mac.src_addr, srcMacAddr, 6);
    memcpy(pkt->mac.dest_addr, destMacAddr, 6);

    pkt->mac.fr_type = hton16(PROTOCOL_ARP);

    arp->hw_type = hton16(1); /* Ethernet */
    arp->prot_type = hton16(PROTOCOL_IP);
    arp->hw_addr_len = 6;
    arp->prot_addr_len = 4;
    arp->opcode = hton16(opcode);
    memcpy(arp->src_hw_addr, srcMacAddr, 6);
    memcpy(arp->src_prot_addr, srcIpAddr, 4);

    if (opcode == ARP_RESPONSE) {
       memcpy(arp->dest_hw_addr, destMacAddr, 6);
    } else {
       memset(arp->dest_hw_addr, 0, 6);
    }
    memcpy(arp->dest_prot_addr, destIpAddr, 4);

#ifdef BONJOUR_TESTING
    WL_ERROR(("%s Dump\n", __FUNCTION__));
    WL_ERROR(("Mac Hdr: srcMac %x %x %x %x %x %x  destmac %x %x %x %x %x %x\n",
    	pkt->mac.src_addr[0], pkt->mac.src_addr[1], pkt->mac.src_addr[2],
    	pkt->mac.src_addr[3], pkt->mac.src_addr[4], pkt->mac.src_addr[5],

    	pkt->mac.dest_addr[0], pkt->mac.dest_addr[1], pkt->mac.dest_addr[2],
    	pkt->mac.dest_addr[3], pkt->mac.dest_addr[4], pkt->mac.dest_addr[5]));

	WL_ERROR(("fr_type 0x%x\n",  ntoh16(pkt->mac.fr_type)));
	WL_ERROR(("ARP hw_type %d, prot_type %d, hw_add_len %d, prot_addr_len %d, opcode %d\n",
		ntoh16(arp->hw_type), ntoh16(arp->prot_type), arp->hw_addr_len,
		arp->prot_addr_len, ntoh16(arp->opcode)));
	WL_ERROR(("srcMacAddr %x %x %x %x %x %x\n",
		arp->src_hw_addr[0],  arp->src_hw_addr[1], arp->src_hw_addr[2],
		arp->src_hw_addr[3],  arp->src_hw_addr[4], arp->src_hw_addr[5]));
	WL_ERROR(("srcIpAddr %d %d %d %d\n",
		arp->src_prot_addr[0], arp->src_prot_addr[1],
		arp->src_prot_addr[2], arp->src_prot_addr[3]));

	WL_ERROR(("destMacAddr %x %x %x %x %x %x\n",
		arp->dest_hw_addr[0],  arp->dest_hw_addr[1], arp->dest_hw_addr[2],
		arp->dest_hw_addr[3],  arp->dest_hw_addr[4], arp->dest_hw_addr[5]));
	WL_ERROR(("destIpAddr %d %d %d %d\n",
		arp->dest_prot_addr[0], arp->dest_prot_addr[1], arp->dest_prot_addr[2],
		arp->dest_prot_addr[3]));
	WL_ERROR(("--\n"));
#endif
}


bool matchOffloadIpAddr(u8 *ipAddr, u32 addrType);
bool matchOffloadIpAddr(u8 *ipAddr, u32 addrType)
{
	mDNS *m = mdns_share_blk->m;
	NetworkInterfaceInfo *intfList = m->HostInterfaces;

	DNS_QUERY("%s: Enter\n", __FUNCTION__);

	if (!ipAddr) {
		WL_ERROR(("%s: NULL ipAddr!!\n", __FUNCTION__));
		return FALSE;
	}

	while (intfList) {
		DNS_QUERY("%s: intflist\n", __FUNCTION__);
		if (intfList->ip.type == addrType) {
			if (!memcmp(&intfList->ip.ip, ipAddr,
				addrType == mDNSAddrType_IPv4 ?
					sizeof(mDNSv4Addr) : sizeof(mDNSv6Addr))) {

				DNS_QUERY("%s:  Matches\n", __FUNCTION__);
				return TRUE;
			}
		}
		intfList = intfList->next;
	}
	DNS_QUERY("%s: Done\n", __FUNCTION__);
	return FALSE;
}


/*
 *
 */
void setupIpV6Header( ipV6Hdr_t *pkt, u8 *srcIpAddr, u8 *destIpAddr, u8 hopLimit,
	u8 next_hdr, u16 payload_len)
{
	/*
	    pkt->ver_traffic_class_flow_label = 0x60000000;
	*/
	pkt->vtf.b[0] = 0x60;
	pkt->payload_len = hton16(payload_len);

	pkt->next_hdr = next_hdr;
	pkt->hop_limit = hopLimit;

	memcpy(pkt->src_addr, srcIpAddr, 16);
	memcpy(pkt->dest_addr, destIpAddr, 16);
}


/* public routines */
uint16 wakeupPort(u16 port, u16 *portList, u16 numPorts);
uint16 wakeupPort(u16 port, u16 *portList, u16 numPorts)
{
	int i;
	for (i = 0; i < numPorts; i++) {
		if (port == portList[i])
			return port;
	}
	return 0;
}

uint16 wakeupTCPPort(pTcpHdr  tcpHdr);
uint16 wakeupTCPPort(pTcpHdr  tcpHdr)
{
	/* Ports in list are in Network order */
	mDNS *m = mdns_share_blk->m;
	u16 port = tcpHdr->dest_port;

	if (wakeupPort(port, m->tcpWakeUpPorts, m->numTcpWakeUpPorts)) {
		uint16 ecn = ntoh16(tcpHdr->data_ecn_ctrl);
		if (port == hton16(SSHPort)) {
			/* For ssh connections, where we'll wake for plain data packets too */
			if (!(ecn & TCP_FLAG_RST) &&
				(ecn & (TCP_FLAG_SYN | TCP_FLAG_FIN)) != TCP_FLAG_FIN) {
				return port;
			}
			WL_ERROR(("%s: Port match on SSHPort %d but bad ecn 0x%x\n",
				__FUNCTION__, ntoh16(port), ecn));
		} else {
			if (ecn & TCP_FLAG_SYN) {
				return port;
			}
			WL_ERROR(("%s: Port match on port %d but bad ecn_ctrl 0x%x\n",
				__FUNCTION__, ntoh16(port), ecn));
		}
	}

	return 0;
}

uint16 wakeupUDPPort(pUdpHdr udp);
uint16 wakeupUDPPort(pUdpHdr udp)
{
	bool wake = FALSE;
	mDNS *m = mdns_share_blk->m;
	u16 udp_len;
	u16 port = udp->dest_port;

#ifdef UNTESTED
	u16 datalen;
	u8 *payload = NULL;
#endif /* UNTESTED */

	/* Ports in list are in Network order */
	if (!wakeupPort(port, m->udpWakeUpPorts, m->numUdpWakeUpPorts)) {
		return 0;
	}
	WL_ERROR(("%s: Match on UDP port %d\n", __FUNCTION__, ntoh16(port)));

	wake = TRUE;

	udp_len = ntoh16(udp->len);
	/* Handle exceptions */
	if (udp_len <= sizeof(UdpHdr_t)) {
		WL_ERROR(("%s: datalen too small, wake host\n", __FUNCTION__));
		return wake;
	}

	if (port == hton16(IPSECPort) || (port == hton16(ARDPort))) {
		WL_ERROR(("%s: Port %d matches %d (IPSEC) or %d (ARDPort)\n",
			__FUNCTION__, ntoh16(port), IPSECPort, ARDPort));
	}
#ifdef UNTESTED
	datalen = udp_len - sizeof(UdpHdr_t);
	payload = (u8 *)((u8 *)&udp->checksum + sizeof(udp->checksum));

	if (port == hton16(IPSECPort)) {
		WL_ERROR(("%s: IPSEC Port - no idea\n", __FUNCTION__));
		if ((udp_len == 0x9) && (udp->checksum == 0) && (*(u8 *)payload == 0xFF)) {
			/* Don't wake-up on NAT Keep-Alive packets */
			wake = FALSE;
		} else {
			/* Skip over the Non-ESP Marker if present */
			bool NonESP =
			    (udp_len >= 12 && payload[0] == 0 && payload[1] == 0 &&
			    payload[2] == 0 && payload[3] == 0);
			IKEHeader *ike 	= (IKEHeader *)((u8 *)payload + (NonESP ? 4 : 0));
			u16 ikelen = datalen - (NonESP ? 4 : 0);

			if (ikelen >= sizeof(IKEHeader)) {
				if ((ike->Version & 0x10) == 0x10) {
				/* ExchangeType ==  5 means 'Informational' */
				/*	<http://www.ietf.org/rfc/rfc2408.txt> */
				/* ExchangeType == 34 means 'IKE_SA_INIT' */
				/* 	<http://www.iana.org/assignments/ikev2-parameters> */
					if (ike->ExchangeType == 5 || ike->ExchangeType == 34)
						wake = FALSE;
				}
			}
		}
	} else {
		if (port == hton16(ARDPort)) {
			WL_ERROR(("%s: Remote Desktop Port - no idea\n", __FUNCTION__));
		/* For now, because we haven't yet worked out a clean elegant way to do */
		/* this, we just special-case the Apple Remote Desktop port number -- we */
		/* ignore all packets to UDP 3283 (the "Net Assistant" port), except for */
		/* Apple Remote Desktop's explicit manual wakeup packet, which looks like */
		/* this: UDP header (8 bytes) */
		/* Payload: 13 88 00 6a 41 4e 41 20 (8 bytes) ffffffffffff (6 bytes) */
		/* 16xMAC (96 bytes) = 110 bytes total */
			wake = (datalen >= 110 && payload[0] == 0x13 && payload[1] == 0x88);
			if (wake)
				WL_ERROR(("%s: Waking for Remote Desktop\n", __FUNCTION__));
			else
				WL_ERROR(("%s:Remote Desk, payload didn't match len %d,0x%x 0x%x\n",
					__FUNCTION__, datalen, payload[0], payload[1]));
		}
	}
#endif /* UNTESTED */
	return wake;
}

static void
print_v4(char *output, uchar *buf)
{
	WL_ERROR(("%s %d.%d.%d.%d\n", output, buf[0], buf[1], buf[2], buf[3]));
}

uint32 mdns_rx(wlc_dngl_ol_mdns_info_t *mdnsi, void *pkt, uint16 len)
{
	TXPORT_HDR  *hdr = (TXPORT_HDR *) pkt;
	u32 rc;
	MDNS_IPV4_PKT	*mdns_ipv4_pkt;
	MDNS_IPV6_PKT	*mdns_ipv6_pkt;
	mDNSAddr    	mdns_srcaddr;
	mDNSIPPort 	mdns_srcport;
	mDNSAddr 	mdns_dstaddr;
	mDNSIPPort 	mdns_dstport;
	uint16 		wake = 0;
	uint16		type;

	rc = ERR_NO_ERROR;

	if (len < sizeof(MAC_HDR))
		return (ERR_PKT_TOO_SHORT);

	if (!mdnsi->dns_enabled) {
		WL_TRACE(("%s: mdns is disabled\n", __FUNCTION__));
		return (ERR_PROT_UNHANDLED);
	}

	/* Save pkt for sending to host during wakeup */
	mdnsi->rx_dataptr = pkt;
	mdnsi->rx_datalen = len;
	if (len > ETHER_MAX_DATA)
		mdnsi->rx_datalen = ETHER_MAX_DATA;

	type = ntoh16(hdr->mac.fr_type);
	switch (type) {
	case PROTOCOL_IP: /* IPV4 */
		/* Sanity check */
		if (len < (sizeof(MAC_HDR) + sizeof(ipHdr_t))) {
			WL_ERROR(("%s:  len < sizeof(MAC_HDR) + sizeof(ipHdr_t))\n", __FUNCTION__));
			return (ERR_PKT_TOO_SHORT);
		}

		/* we don't support fragmented IP packets */
		if (hdr->ip.flags_frag_offset & hton16(IP_FRAG_OFFSET_MASK)) {
			WL_ERROR(("%s: FRAG, unmasked frag_offset 0x%x\n",
				__FUNCTION__, hdr->ip.flags_frag_offset));
			return (ERR_PROT_UNHANDLED);
		}

		/* weed out unwanted packets */
		if (!is_multi(hdr->ip.dest_addr, mDNSAddrType_IPv4)) {  /* ! multicast */
			if (!matchOffloadIpAddr(hdr->ip.dest_addr, mDNSAddrType_IPv4)) { /*! ours */
				if (memcmp(hdr->ip.dest_addr, onesIPv4Addr.b, 4) != 0) { /* !bcst */
					if (0)
					print_v4("mdns_rx: !bcast && !mcast && !ours; reject",
						hdr->ip.dest_addr);
					return (ERR_ADDR_FAILED);
				}
			}
		}
#ifdef MCAST_FILTER
		if (matchOffloadIpAddr(hdr->ip.src_addr, mDNSAddrType_IPv4)) { /* came from us! */
			//print_v4("mdns_rx: came from us reject", hdr->ip.src_addr);
			return (ERR_ADDR_FAILED);
		}
#endif

		switch (hdr->ip.protocol) {
		case PROTOCOL_TCP: {
			/* Check if the TCP SYNC packet dest port matches any Bonjour services */
			pTcpHdr		tcpHdr;
			tcpHdr = (pTcpHdr)&hdr->txp;
			WL_TRACE(("%s: Incoming TCP IPV4 on port %d %s\n",
				__FUNCTION__, ntoh16(tcpHdr->dest_port),
				ntoh16(tcpHdr->dest_port) == 3689 ?  "(itunes)" : ""));
			wake = wakeupTCPPort(tcpHdr);
			if (wake) {
				WL_ERROR(("%s: Incoming TCP IPV4 on advertised port %d\n",
					__FUNCTION__, ntoh16(tcpHdr->dest_port)));
			}
		}
			break;
		case PROTOCOL_UDP:
			if (len < (sizeof(MAC_HDR) + sizeof(ipHdr_t) + sizeof(UdpHdr_t)))
				return (ERR_PKT_TOO_SHORT);

			if (hdr->txp.udp.dest_port == hton16(UDP_PORT_MDNS)) {
				WL_TRACE(("%s: Incoming IPV4 UDP port 5353 (MDNS)\n", __FUNCTION__));
				mdns_ipv4_pkt = (MDNS_IPV4_PKT *)pkt;

				mdns_srcaddr.type = mDNSAddrType_IPv4; /* host endan for type */
				memcpy(&mdns_srcaddr.ip.v4.NotAnInteger,
					&mdns_ipv4_pkt->ip.src_addr, 4);
				mdns_srcport.NotAnInteger = mdns_ipv4_pkt->udp.src_port;

				mdns_dstaddr.type = mDNSAddrType_IPv4; /* host endan for type */
				memcpy(&mdns_dstaddr.ip.v4.NotAnInteger,
					&mdns_ipv4_pkt->ip.dest_addr, 4);
				mdns_dstport.NotAnInteger = mdns_ipv4_pkt->udp.dest_port;

				mDNSCoreReceive(&mdns_ipv4_pkt->h, (u8 *)pkt + len,
					mdns_ipv4_pkt->mac.src_addr, &mdns_srcaddr, mdns_srcport,
					&mdns_dstaddr, mdns_dstport);
			} else {
				/* Check if the UDP dest port matches any Bonjour services */
				WL_TRACE(("%s: Incoming IPV4 UDP on non-MDNS port\n", __FUNCTION__));
				wake = wakeupUDPPort(&hdr->txp.udp);
				if (wake) {
					WL_ERROR(("%s: IPV4 UDP on advertised port %d\n",
						__FUNCTION__, ntoh16(hdr->txp.udp.dest_port)));
				}
			}
			break;
		case PROTOCOL_ICMP:
			break;

		default:
			WL_TRACE(("%s: Unknown ip protocol 0x%x\n", __FUNCTION__, hdr->ip.protocol));
			return (ERR_PROT_UNKNOWN);
		} /* switch (hdr->ip.protocol) */

		break;


	case PROTOCOL_IPV6: {
		ipV6Hdr_t *pIpv6Hdr = (ipV6Hdr_t *)&hdr->ip;
		bool ismulti = FALSE;
		/* Filter out IPV6 packets not for us */
		ismulti = is_multi(pIpv6Hdr->dest_addr, mDNSAddrType_IPv6);
		if (!ismulti) {
			if (!matchOffloadIpAddr(pIpv6Hdr->dest_addr, mDNSAddrType_IPv6)) {
				WL_TRACE(("%s: V6 UDP, Not multi and NOT for us, toss\n",
					__FUNCTION__));
				if (0) print_real_raw(pIpv6Hdr->dest_addr, sizeof(v6_multi)/2);
				break;
			}
		}
#ifdef MCAST_FILTER
		if (matchOffloadIpAddr(pIpv6Hdr->src_addr, mDNSAddrType_IPv6)) { /* came from us! */
			//WL_ERROR(("%s: IPV6 came from us reject: ", __FUNCTION__));
			//print_real_raw(pIpv6Hdr->src_addr, sizeof(v6_multi)/2);
			return (ERR_ADDR_FAILED);
		}
#endif

		switch (pIpv6Hdr->next_hdr) {
			case PROTOCOL_ICMPV6:
				break;
			case PROTOCOL_UDP: {
				TXPORT_IPV6_UDP_HDR	*pIpv6UdpHdr;
				WL_TRACE(("%s: Incoming V6 UDP %s ",
					__FUNCTION__, ismulti ? "Multicast" : "Unicast"));

				pIpv6UdpHdr = (TXPORT_IPV6_UDP_HDR *)hdr;
				if (pIpv6UdpHdr->udp.dest_port == hton16(UDP_PORT_MDNS)) {
					WL_TRACE(("on port %d (MDNS)\n",
						ntoh16(pIpv6UdpHdr->udp.dest_port)));
					mdns_ipv6_pkt = (MDNS_IPV6_PKT *)pkt;
					mdns_srcaddr.type = mDNSAddrType_IPv6;
					memcpy(mdns_srcaddr.ip.v6.b, &mdns_ipv6_pkt->ip.src_addr,
						16);
					mdns_srcport.NotAnInteger = mdns_ipv6_pkt->udp.src_port;

#ifdef USE_IPv6_to_6
					mdns_dstaddr.type = mDNSAddrType_IPv6;
					memcpy(&mdns_dstaddr.ip.v6.b,
						&mdns_ipv6_pkt->ip.dest_addr, 16);
#else
					/* This is original code from ethernet firmware */
					mdns_dstaddr.type = mDNSAddrType_IPv4;
					memcpy(&mdns_dstaddr.ip.v4.NotAnInteger,
						&mdns_ipv6_pkt->ip.dest_addr, 4);
#endif /* USE_IPv6_to_6 */
					mdns_dstport.NotAnInteger = mdns_ipv6_pkt->udp.dest_port;

					mDNSCoreReceive(&mdns_ipv6_pkt->h, (u8 *)pkt + len,
						mdns_ipv6_pkt->mac.src_addr,
						&mdns_srcaddr, mdns_srcport,
						&mdns_dstaddr, mdns_dstport);
				} else {
					/* Check if UDP dest port matches any Bonjour services */
					wake = wakeupUDPPort(&pIpv6UdpHdr->udp);
					if (wake) {
						WL_ERROR(("IPV6 Incoming on advertised port %d\n",
							ntoh16(pIpv6UdpHdr->udp.dest_port)));
					}
				}
			}
				break;
			case PROTOCOL_TCP: {
				/* Check if the TCP SYNC packet dest port matches any
				   Bonjour services
				*/
				TXPORT_IPV6_TCP_HDR *pTcp;
				pTcp = (TXPORT_IPV6_TCP_HDR *)hdr;
				wake = wakeupTCPPort(&pTcp->tcp);
				WL_ERROR(("%s: Incoming V6 TCP %s port %d %s\n",
					__FUNCTION__, ismulti ?  "Multicast" : "Unicast",
					ntoh16(pTcp->tcp.dest_port),
					wake ? "matches, wakeup" : "doesn't match"));
			}
				break;

			default:
				WL_TRACE(("%s: Unknown pIpv6Hdr->next_hdr 0x%x\n",
					__FUNCTION__, pIpv6Hdr->next_hdr));
				rc = ERR_PROT_UNKNOWN;
				break;
		} /* switch (pIpv6Hdr->next_hdr) */
	} /* case PROTOCOL_IPV6 */
	break;

	default:
		WL_TRACE(("%s: Unknown Protocol 0x%x\n", __FUNCTION__, type));
		rc = ERR_PROT_UNKNOWN;
	} /* switch (type) */

	if (wake) {
		WL_TRACE(("%s: Main loop calling Wakeup\n", __FUNCTION__));

		/* force wake up system */
		wl_mdns_wake(WL_WOWL_MDNS_SERVICE);
	}
	return (rc);
}

static void
wl_mdns_timer(void *arg)
{
	s32 nexttime = mDNS_Execute();

	if (glob_mdnsi->dns_enabled) {
		wl_add_timer(glob_mdnsi->wlc_dngl_ol->wlc->wl,
			glob_mdnsi->mdns_execute_timer, nexttime, FALSE);
	} else {
		WL_TRACE(("%s: Canceling mdns timers\n", __FUNCTION__));
	}
}

wlc_dngl_ol_mdns_info_t *
wlc_dngl_ol_mdns_attach(wlc_dngl_ol_info_t *wlc_dngl_ol)
{
	wlc_dngl_ol_mdns_info_t *mdnsi;
	wlc_info_t      *wlc = wlc_dngl_ol->wlc;

	WL_ERROR(("%s\n", __FUNCTION__));
	/* allocate arp private info struct */
	mdnsi = MALLOC(wlc_dngl_ol->osh, sizeof(wlc_dngl_ol_mdns_info_t));
	if (!mdnsi) {
		WL_ERROR(("wl %s: MALLOC failed; total mallocs %d bytes\n",
		          __FUNCTION__, MALLOCED(wlc_dngl_ol->osh)));
		return NULL;
	}

	/* init arp private info struct */
	bzero(mdnsi, sizeof(wlc_dngl_ol_mdns_info_t));
	mdnsi->wlc_dngl_ol = wlc_dngl_ol;
	mdnsi->magic = BONJOUR_PROXY_SIG;
	mdnsi->osh = wlc_dngl_ol->osh;
	glob_mdnsi = mdnsi;
	mdns_share_blk = &mdnsi->mdns_share_blk_struct;

	/* Initialize mdns shared area */
	RXOESHARED(wlc_dngl_ol)->mdns_dbase[0] = 0;
	RXOESHARED(wlc_dngl_ol)->mdns_len = 0;

	if ((mdnsi->mdns_execute_timer =
		wl_init_timer(wlc->wl, wl_mdns_timer, wlc_dngl_ol, "mdns")) == NULL) {
		WL_ERROR(("%s: wl_init_timer failed, bailing\n", __FUNCTION__));
		return NULL;
	}

	return mdnsi;
}

void
BCMATTACHFN(wl_mdns_detach)(wlc_dngl_ol_mdns_info_t *mdnsi)
{
	WL_ERROR(("%s\n", __FUNCTION__));
	if (!mdnsi)
		return;
	MFREE(mdnsi->osh, mdnsi, sizeof(wlc_dngl_ol_mdns_info_t));
}
