
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
 * Mulicast DNS Service Discovery offload common routines.
 *
 * $Id: wl_mdns_common.c 467328 2014-04-03 01:23:40Z $
 * 
 */


/* FILE-CSTYLED */

#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmendian.h>
#include <bcmutils.h>
#include <siutils.h>
#include <wlioctl.h>
#include <d11.h>
#include <wlc_key.h>
#include <wlc_rate.h>
#include <wlc.h>
#include <wlc_bmac.h>
#include <wl_export.h>

#include "wl_mdns_common.h"

u32 simple_divide(u32 dividend, u32 divisor)
{
	u32 quotient = 0;
	u32 remainder = dividend;

	while (remainder >= divisor)
	{
		remainder -= divisor;
		quotient++;
	}

	return quotient;
}


bool mDNSPlatformMemSame( void *src,  void *dst, u32 len)
{
	return memcmp((u8 *)dst, (u8 *)src, len) == 0;
}

s32 NonZeroTime(s32 t)
{
	if (t)
		return (t);
	else
		return (1);
}

/* Returns length of a domain name INCLUDING the byte for the final null label */
/* i.e. for the root label "." it returns one */
/* For the FQDN "com." it returns 5 (length byte, three data bytes, final zero) */
/* Legal results are 1 (just root label) to 255 (MAX_DOMAIN_NAME) */
/* If the given domainname is invalid, result is 256 */
u16 DomainNameLength(domainname *name)
{
	u8 *src;
	if (!name) 
		WL_ERROR(("%s: Error name is NULL\n", __FUNCTION__));
		
	src = name->c;

	while (*src)
	{
		if (*src > MAX_DOMAIN_LABEL) return (MAX_DOMAIN_NAME+1);
		src += 1 + *src;
		if (src - name->c >= MAX_DOMAIN_NAME) return (MAX_DOMAIN_NAME+1);
	}
	return ((u16)(src - name->c + 1));
}

bool SameDomainLabel(u8 *a, u8 *b)
{
	int i;
	int len = *a++;

	if (len > MAX_DOMAIN_LABEL)
		/* Malformed label (too long) */
		return (mDNSfalse);

	if (len != *b++) return (mDNSfalse);
	for (i = 0; i < len; i++)
	{
		u8 ac = *a++;
		u8 bc = *b++;
		if (mDNSIsUpperCase(ac)) ac += 'a' - 'A';
		if (mDNSIsUpperCase(bc)) bc += 'a' - 'A';
		if (ac != bc) return (mDNSfalse);
	}
	return (mDNStrue);
}

bool SameDomainName(domainname *d1, domainname *d2)
{
	u8 *a   = d1->c;
	u8 *b   = d2->c;
	u8 *max = d1->c + MAX_DOMAIN_NAME;			/* Maximum that's valid */

	while (*a || *b)
	{
		if (a + 1 + *a >= max)
			/* Malformed domain name (more than 255 characters) */
			return (mDNSfalse);

		if (!SameDomainLabel(a, b)) return (mDNSfalse);
		a += 1 + *a;
		b += 1 + *b;
	}

	return (mDNStrue);
}

/* SameResourceRecordSignature returns true if two resources records have the same name, */
/* type, and class, and may be sent (or were received) on the same interface (i.e. if */
/* *both* records specify an interface, then it has to match). */
/* TTL and rdata may differ. */
/* This is used for cache flush management: */
/* When sending a unique record, all other records matching "SameResourceRecordSignature" must also be sent */
/* When receiving a unique record, all old cache records matching "SameResourceRecordSignature" are flushed */

/* SameResourceRecordNameClassInterface is functionally the same as */
/* SameResourceRecordSignature, except rrtype does not have to match */

mDNSBool SameResourceRecordNameClassInterface(AuthRecord *r1, AuthRecord *r2)
{
	if (!r1) return (mDNSfalse);
	if (!r2) return (mDNSfalse);

	return (mDNSBool)(
		r1->resrec.rrclass  == r2->resrec.rrclass &&
		SameDomainName(r1->resrec.name, r2->resrec.name));
}

/* r1 has to be a full ResourceRecord including rrtype and rdlength */
/* r2 is just a bare RDataBody, which MUST be the same rrtype and rdlength as r1 */
bool SameRDataBody(ResourceRecord *r1, RDataBody *r2)
{
	switch(r1->rrtype)
	{
		case kDNSType_CNAME:/* Same as PTR */
		case kDNSType_PTR:
			return (SameDomainName(r1->rdata->u.name, r2->name));

		case kDNSType_SRV:
			return (bool)(r1->rdata->u.srv->priority == r2->srv->priority && r1->rdata->u.srv->weight == r2->srv->weight && r1->rdata->u.srv->port.NotAnInteger == r2->srv->port.NotAnInteger && SameDomainName(r1->rdata->u.srv->target, r2->srv->target));

		case kDNSType_NSEC:
			return (mDNSPlatformMemSame(r1->rdata->u.data, r2->data, sizeof(rdataNSEC)));

		default:
			return (mDNSPlatformMemSame(&r1->rdata->u.data, &r2->data, r1->rdlength));
	}
}

bool SameRData(ResourceRecord *r1, ResourceRecord *r2)
{
	if (r1->rrtype     != r2->rrtype)     return (mDNSfalse);
	if (r1->rdlength   != r2->rdlength)   return (mDNSfalse);
	return (SameRDataBody(r1, &r2->rdata->u));
}

/* IdenticalResourceRecord returns true if two resources records have */
/* the same name, type, class, and identical rdata (TTL may differ) */
bool IdenticalResourceRecord(ResourceRecord *r1, ResourceRecord *r2)
{
	if (!r1) 
		/* IdenticalResourceRecord ERROR: r1 is mDNSNULL */
		return (mDNSfalse);
	if (!r2) 
		/* IdenticalResourceRecord ERROR: r2 is mDNSNULL */
		return (mDNSfalse);
	if (r1->rrtype != r2->rrtype || r1->rrclass != r2->rrclass || !SameDomainName(r1->name, r2->name))
		return (mDNSfalse);

	return (SameRData(r1, r2));
}

/* CompressedDomainNameLength returns the length of a domain name INCLUDING the byte */
/* for the final null label i.e. for the root label "." it returns one. */
/* E.g. for the FQDN "foo.com." it returns 9 */
/* (length, three data bytes, length, three more data bytes, final zero). */
/* In the case where a parent domain name is provided, and the given name is a child */
/* of that parent, CompressedDomainNameLength returns the length of the prefix portion */
/* of the child name, plus TWO bytes for the compression pointer. */
/* E.g. for the name "foo.com." with parent "com.", it returns 6 */
/* (length, three data bytes, two-byte compression pointer). */
u16 CompressedDomainNameLength(domainname *name, domainname *parent)
{
	u8 *src;
	
	if (!name){
		WL_ERROR(("%s: Null name, return\n", __FUNCTION__));
		return 0;
	}
	if (!parent)
		WL_TRACE(("%s: Null parent\n", __FUNCTION__));
	
	src = name->c;

	if (parent && parent->c[0] == 0) {
		WL_ERROR(("%s: Parent string is NULL\n", __FUNCTION__));
		parent = mDNSNULL;
	}
	while (*src)
	{
		if (*src > MAX_DOMAIN_LABEL) {
			WL_ERROR(("%s: > MAX_DOMAIN_LABEL\n", __FUNCTION__));
			return (MAX_DOMAIN_NAME+1);
		}
		if (parent && SameDomainName((domainname *)src, parent)) {
			WL_TRACE(("%s: same_domain\n", __FUNCTION__));
			return ((u16)(src - name->c + 2));
		}
		src += 1 + *src;
		if (src - name->c >= MAX_DOMAIN_NAME) {
			WL_ERROR(("%s: length > MAX_DOMAIN_NAME\n", __FUNCTION__));
			return (MAX_DOMAIN_NAME+1);
		}
	}
	return ((u16)(src - name->c + 1));
}

u16 GetRDLength(ResourceRecord *rr, bool estimate)
{
	RDataBody 	*rd = &rr->rdata->u;
	domainname 	*name = estimate ? rr->name : mDNSNULL;
	u16 result = 0;

	WL_TRACE(("Enter GetRDLength estmate %d, type %d, class %d ttl 0x%x\n",
		estimate, rr->rrtype, rr->rrclass, rr->rroriginalttl));

	switch (rr->rrtype)
	{
		case kDNSType_A:	
			return (sizeof(mDNSv4Addr));
		case kDNSType_AAAA:	
			return (sizeof(mDNSv6Addr));
		case kDNSType_HINFO:
			return (mDNSu16)(2 + (int)rd->data[0] + (int)rd->data[1 + (int)rd->data[0]]);


		case kDNSType_CNAME:/* Same as PTR */
		case kDNSType_NS:   /* Same as PTR */
		case kDNSType_PTR:	
			return (CompressedDomainNameLength(rd->name, name));

		case kDNSType_SRV:	
				if (rd->srv) {
					result = CompressedDomainNameLength(rd->srv->target, name);
				} else {
					WL_TRACE(("Skipping call to CompressedDomainNameLengt\n"));
				}
				return (u16)(6 + result);

		case kDNSType_TXT:  
			return (rr->rdlength); /* TXT is not self-describing, so have */
							  /*to just trust rdlength */
		case kDNSType_NSEC:
		{
			int i;

			for (i = sizeof(rdataNSEC); i > 0; i--) 
				if (rd->nsec->bitmap[i-1])
					break;
			/* For our simplified use of NSEC synthetic records: */
			/* nextname is always the record's own name, */
			/* the block number is always 0, */
			/* the count byte is a value in the range 1-32, */
			/* followed by the 1-32 data bytes */
			return ((estimate ? 1 : DomainNameLength(rr->name)) + 2 + i);
		}
		default:
			return (rr->rdlength);
	}
}

void SetNewRData(ResourceRecord *const rr, RData *NewRData, u16 rdlength)
{
	if (NewRData) {
		WL_ERROR(("%s: NewRData is Something!!!!!\n", __FUNCTION__));
		rr->rdata    = NewRData;
		rr->rdlength = rdlength;
		/*No return here????? */
	}
	rr->rdlength   = GetRDLength(rr, mDNSfalse);
	rr->rdestimate = GetRDLength(rr, mDNStrue);
}

/* Routine to fetch an FQDN from the DNS message, following compression pointers if necessary. */
u8 *getDomainName(DNSMessage *msg, u8 *ptr, u8 *end, domainname *name)
{
	u8 *nextbyte = mDNSNULL;/* Record where we got to before we started following pointers */
	u8 *np = name->c;	/* Name pointer */
	u8 *limit = np + MAX_DOMAIN_NAME;/* Limit so we don't overrun buffer */
	bool verbose = FALSE;

	if (ptr < (u8*)msg || ptr >= end) {
		WL_ERROR(("%s: Illegal ptr\n", __FUNCTION__));
		/* Illegal ptr not within packet boundaries */
		return (mDNSNULL);
	}

	*np = 0;/* Tentatively place the root label here (may be overwritten if we have more labels) */

	while (1)/* Read sequence of labels */
	{
		u8 len = *ptr++;	/* Read length of this label */

		if (len == 0) break;	/* If length is zero, that means this name is complete */
		switch (len & 0xC0)
		{
			int i;
			u16 offset;

			case 0x00:	
				WL_TRACE(("No C0: "));
				if (ptr + len >= end) {/* Remember: expect at least one more byte for the root label */
					int z;
					/* Malformed domain name (overruns packet end) */
					WL_TRACE(("%s: Malformed, overrun length, len %d, expected < %d\n",
						__FUNCTION__, len, end - ptr));
					for (z = 0; z < end - ptr; z++)
						WL_TRACE(("%x ", *(ptr+z)));
					WL_TRACE(("\n"));
					return (mDNSNULL);
				}

				if (np + 1 + len >= limit){ /* Remember: expect at least one more */
							  /* byte for the root label */
					/* Malformed domain name (more than 255 characters) */
					WL_ERROR(("%s: Pt3, Malformed > 255\n", __FUNCTION__));
					return (mDNSNULL);
				}
				*np++ = len;
				for (i = 0; i < len; i++) {
					if (verbose) WL_ERROR(("%c", *ptr));
					*np++ = *ptr++;
				}
				if (verbose) WL_ERROR(("."));
				*np = 0;	/* Tentatively place the root label here */
						/* (may be overwritten if we have more labels) */
				break;

			case 0x40:
			case 0x80:	
				WL_TRACE(("%s: Pt4, return NULL\n", __FUNCTION__));
				return (mDNSNULL);

			case 0xC0:
				offset = (u16)((((u16)(len & 0x3F)) << 8) | *ptr++);
				WL_TRACE(("C0, offset = %d\n", offset));

				if (!nextbyte) nextbyte = ptr;	/* Record where we got to before */
								/*we started following pointers */
				ptr = (u8 *)msg + offset;
				if (ptr < (u8*)msg || ptr >= end) {
					/* Illegal compression pointer not within packet boundaries */
					WL_ERROR(("%s: Pt6 Illegal compression pointer\n", __FUNCTION__));
					if (ptr >= end)
						WL_ERROR(("%s: ptr is >= end\n", __FUNCTION__));
					return (mDNSNULL);
				}
				if (*ptr & 0xC0) {
					/* Compression pointer must point to real label */
					WL_ERROR(("%s: Illegal Pt7\n", __FUNCTION__));
					return (mDNSNULL);
				}
				break;
		}
	}
	if (verbose)
		WL_ERROR(("\n"));

	if (nextbyte)
		return (nextbyte);
	else
		return (ptr);
}

/* Parse raw input into a flat LargeCacheRecord and local ResourceRecord points to it */
u8 *GetLargeResourceRecord(DNSMessage *msg, u8 *ptr, u8 *end, u8 RecordType, LargeCacheRecord *largecr)
{
	CacheRecord *rr = &largecr->r;
	u16 pktrdlength;
	
	rr->next              = mDNSNULL;
	rr->resrec.name       = &largecr->namestorage;

	/*Copy input ptr into LargeCache */
	ptr = getDomainName(msg, ptr, end, rr->resrec.name);

	if (!ptr) {
		WL_TRACE(("%s: getDomain returns NULL\n", __FUNCTION__));
		return (mDNSNULL);
	}
	if (ptr + 10 > end) {
		WL_ERROR(("%s: Record len < 10 bytes, ptr 0x%p, end 0x%p\n", __FUNCTION__, ptr, end));
		return (mDNSNULL);
	}
	if (IS_WL_TRACE_ON()) {
		WL_TRACE(("%s: ", __FUNCTION__));
		print_domainname(rr->resrec.name);
		WL_TRACE(("\n"));
	}


	/* Ptr must point to Table 11 immediatly after 'Name' at type  */
	rr->resrec.rrtype            = (u16) ((u16)ptr[0] <<  8 | ptr[1]);
	rr->resrec.rrclass           = (u16)(((u16)ptr[2] <<  8 | ptr[3]) & kDNSClass_Mask);
	rr->resrec.rroriginalttl     = (u32) ((u32)ptr[4] << 24 | (u32)ptr[5] << 16 | (u32)ptr[6] << 8 | ptr[7]);
	pktrdlength           	     = (u16)((u16)ptr[8] <<  8 | ptr[9]);

	WL_TRACE(("%s: rrtype 0x%x, rrclass 0x%x, origttl 0x%x, rdlength 0x%x\n", __FUNCTION__,
		rr->resrec.rrtype, rr->resrec.rrclass, rr->resrec.rroriginalttl, pktrdlength));

	if (ptr[2] & (kDNSClass_UniqueRRSet >> 8))
		RecordType |= kDNSRecordTypePacketUniqueMask;

	ptr += 10;	/* Skip the last 10 bytes just parsed. Looking at Table 11?  */
	if (ptr + pktrdlength > end) {
		WL_TRACE(("%s: ptr + pktrdlength (%d) > end, expectd < %d\n",
			__FUNCTION__, pktrdlength, end - ptr));
		return (mDNSNULL);
	}

	end = ptr + pktrdlength;/* Adjust end to indicate the end of the rdata for this resource record */

	rr->resrec.rdata = (RData*)&rr->rdatastorage;	/*Make the resrec (pointer based) */
							/*point to rdatastorage (with inline data)  */
	rr->resrec.rdata->MaxRDLength = MaximumRDSize;

	/*From the assigment (left) side it looks like a pointer... */
	rr->resrec.rdata->u.data = (u8 *)rr->data;  /*Hoan added this */

	switch (rr->resrec.rrtype)
	{
		case kDNSType_A:	
			WL_TRACE(("%s: Got a A\n", __FUNCTION__));
			rr->resrec.rdata->u.ipv4->b[0] = ptr[0];
			rr->resrec.rdata->u.ipv4->b[1] = ptr[1];
			rr->resrec.rdata->u.ipv4->b[2] = ptr[2];
			rr->resrec.rdata->u.ipv4->b[3] = ptr[3];
			break;

		case kDNSType_AAAA:	
			WL_TRACE(("%s: Got a AAAA\n", __FUNCTION__));
			memcpy(&rr->resrec.rdata->u.ipv6, ptr, sizeof(mDNSv6Addr));
			break;

		case kDNSType_CNAME:/* Same as PTR */
		case kDNSType_PTR:	
			WL_TRACE(("%s: Got a PTR or CNAME\n", __FUNCTION__));
			if (!getDomainName(msg, ptr, end, rr->resrec.rdata->u.name)) {
				WL_ERROR(("%s: Pt4 fail\n", __FUNCTION__));
				return (mDNSNULL);
			}
			break;

		case kDNSType_TXT:  
			WL_TRACE(("%s: Got a TXT\n", __FUNCTION__));
			if (pktrdlength > rr->resrec.rdata->MaxRDLength) {
				WL_ERROR(("%s: Pt5 fail\n", __FUNCTION__));
				return (mDNSNULL);
			}
			rr->resrec.rdlength = pktrdlength;
			memcpy(rr->resrec.rdata->u.data, ptr, pktrdlength);
			break;

		case kDNSType_SRV:
			WL_TRACE(("%s: Got a SRV\n", __FUNCTION__));
			rr->resrec.rdata->u.srv->priority = (u16)((u16)ptr[0] <<  8 | ptr[1]);
			rr->resrec.rdata->u.srv->weight   = (u16)((u16)ptr[2] <<  8 | ptr[3]);
			rr->resrec.rdata->u.srv->port.b[0] = ptr[4];
			rr->resrec.rdata->u.srv->port.b[1] = ptr[5];

			/*Left side (srv->target) is pointer */
			/*Right side (rdatastorage.u.srv.target) is array */
			rr->resrec.rdata->u.srv->target = &rr->rdatastorage.u.srv.target; /*Hoan added this */

			/* Move passed the 6 bytes parsed above */
			/* Copy the name from ptr into rr->resrec.rdata->u.srv->target */

			if (!getDomainName(msg, ptr+6, end, rr->resrec.rdata->u.srv->target)) {
				WL_ERROR(("%s: Pt6 fail\n", __FUNCTION__));
				return (mDNSNULL);
			}
			break;

		case kDNSType_NSEC:
		{
			unsigned int i, j;

			WL_TRACE(("Got a kDNSType_NSEC\n"));
			domainname d;
			ptr = getDomainName(msg, ptr, end, &d);/* Ignored for our simplified use */
							       /* of NSEC synthetic records */
			if (!ptr) return mDNSNULL;
			if (*ptr++ != 0) return mDNSNULL;
			i = *ptr++;
			if (i < 1 || i > sizeof(rdataNSEC)) return mDNSNULL;
			memset(rr->resrec.rdata->u.nsec->bitmap, 0, sizeof(rdataNSEC));
			for (j = 0; j < i; j++) rr->resrec.rdata->u.nsec->bitmap[j] = *ptr++;
			if (ptr != end) return (mDNSNULL);
			break;
		}

		default:
			WL_ERROR(("%s: Unknown record type\n", __FUNCTION__));
			if (pktrdlength > rr->resrec.rdata->MaxRDLength)
				return (mDNSNULL);
			/* Note: Just because we don't understand the record type, that doesn't */
			/* mean we fail. The DNS protocol specifies rdlength, so we can */
			/* safely skip over unknown records and ignore them. */
			/* We also grab a binary copy of the rdata anyway, since the caller */
			/* might know how to interpret it even if we don't. */
			rr->resrec.rdlength = pktrdlength;
			memcpy(rr->resrec.rdata->u.data, ptr, pktrdlength);
			break;
	}

	SetNewRData(&rr->resrec, mDNSNULL, 0);

	/* Success! Now fill in RecordType to show this record contains valid data */
	rr->resrec.RecordType = RecordType;
	return (ptr + pktrdlength);
}

#define DNS_MAX_TTL  (0x7FFFFFFF / mDNSPlatformOneSecond)
/* Set up a AuthRecord with sensible default values. */
/* These defaults may be overwritten with new values before mDNS_Register is called */
void mDNS_SetupResourceRecord(AuthRecord *rr, u16 rrtype, u32 ttl, u8 RecordType)
{
	WL_TRACE(("%s\n", __FUNCTION__));
	/* Don't try to store a TTL bigger than we can represent in platform time units */
	if (ttl > DNS_MAX_TTL) /* 0x7FFFFFFF / mDNSPlatformOneSecond */
		ttl = DNS_MAX_TTL;
	else if (ttl == 0)		/* And Zero TTL is illegal */
		ttl = DefaultTTLforRRType(rrtype);

	/* Field Group 1: The actual information pertaining to this resource record */
	rr->resrec.RecordType        = RecordType;
	rr->resrec.rrtype            = rrtype;
	rr->resrec.rrclass           = kDNSClass_IN;
	rr->resrec.rroriginalttl     = ttl;
/*	rr->resrec.rdlength          = MUST set by client and/or in mDNS_Register_internal */
/*	rr->resrec.rdestimate        = set in mDNS_Register_internal */
/*	rr->resrec.rdata             = MUST be set by client */

	rr->resrec.rdata = &rr->rdatastorage;
	rr->resrec.rdata->MaxRDLength = sizeof(RDataBody_t);

	/* Field Group 2: Persistent metadata for Authoritative Records */
	rr->Additional1       = mDNSNULL;
	rr->Additional2       = mDNSNULL;

	/* Field Group 3: Transient state for Authoritative Records (set in mDNS_Register_internal) */
	
}
