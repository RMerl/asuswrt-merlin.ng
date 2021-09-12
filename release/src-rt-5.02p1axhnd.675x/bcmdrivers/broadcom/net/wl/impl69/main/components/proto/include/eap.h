/*
 * Extensible Authentication Protocol (EAP) definitions
 *
 * See
 * RFC 2284: PPP Extensible Authentication Protocol (EAP)
 *
 * Copyright (C) 2020, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: eap.h 774214 2019-04-16 07:03:49Z $
 */

#ifndef _eap_h_
#define _eap_h_

/* This marks the start of a packed structure section. */
#include <packed_section_start.h>

/* EAP packet format */
typedef BWL_PRE_PACKED_STRUCT struct {
	unsigned char code;	/* EAP code */
	unsigned char id;	/* Current request ID */
	unsigned short length;	/* Length including header */
	unsigned char type;	/* EAP type (optional) */
	unsigned char data[1];	/* Type data (optional) */
} BWL_POST_PACKED_STRUCT eap_header_t;

#define EAP_HEADER_LEN			4u
#define EAP_HEADER_LEN_WITH_TYPE	5u
#define ERP_FLAGS_LEN			1u
#define ERP_SEQ_LEN			2u
#define ERP_KEYNAMENAI_HEADER_LEN	2u
#define ERP_CRYPTOSUITE_LEN		1u

/* EAP codes */
#define EAP_REQUEST		1u
#define EAP_RESPONSE		2u
#define EAP_SUCCESS		3u
#define EAP_FAILURE		4u
#define EAP_INITIATE		5u
#define EAP_FINISH		6u

/* EAP types */
#define EAP_IDENTITY		1
#define EAP_NOTIFICATION	2
#define EAP_NAK			3
#define EAP_MD5			4
#define EAP_OTP			5
#define EAP_GTC			6
#define EAP_TLS			13
#define EAP_EXPANDED		254
#define BCM_EAP_SES		10
#define BCM_EAP_EXP_LEN		12  /* EAP_LEN 5 + 3 bytes for SMI ID + 4 bytes for ven type */
#define BCM_SMI_ID		0x113d
#define WFA_VENDOR_SMI	0x009F68

/* ERP types */
#define EAP_ERP_TYPE_REAUTH_START	1u
#define EAP_ERP_TYPE_REAUTH		2u

/* EAP FLAGS */
#define ERP_R_FLAG	0x80 /* result flag, set = failure */
#define ERP_B_FLAG	0x40 /* bootstrap flag, set = bootstrap */
#define ERP_L_FLAG	0x20 /* rrk lifetime tlv is present */

/* ERP TV/TLV types */
#define EAP_ERP_TLV_KEYNAME_NAI		1u

/* ERP Cryptosuite */
#define EAP_ERP_CS_HMAC_SHA256_128	2u

/* This marks the end of a packed structure section. */
#include <packed_section_end.h>

#endif /* _eap_h_ */
