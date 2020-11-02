/*
 * OID encapsulation defines for user-mode to driver interface.
 *
 * Definitions subject to change without notice.
 *
 * Copyright 2020 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * $Id: oidencap.h 708017 2017-06-29 14:11:45Z $
 */

#ifndef _oidencap_h_
#define	_oidencap_h_

#include <typedefs.h>
/*
 * NOTE: same as OID_EPI_BASE defined in epiioctl.h
 */
#define OID_BCM_BASE					0xFFFEDA00

/*
 * These values are now set in stone to preserve forward
 * binary compatibility.
 */
#define	OID_BCM_SETINFORMATION 			(OID_BCM_BASE + 0x3e)
#define	OID_BCM_GETINFORMATION 			(OID_BCM_BASE + 0x3f)
#ifdef WLFIPS
#define OID_BCM_FIPS_MODE			(OID_BCM_BASE + 0x40)
#endif /* WLFIPS */
#define OID_DHD_IOCTLS					(OID_BCM_BASE + 0x41)
#ifdef WLFIPS
#define OID_FSW_FIPS_MODE			(OID_BCM_BASE + 0x42)
#endif /* WLFIPS */

#if defined(BCMCCX) && defined(CCX_SDK)
#define OID_BCM_CCX	0x00181000	/* based on BRCM_OUI value */
#endif /* BCMCCX && CCX_SDK */

#define	OIDENCAP_COOKIE	0xABADCEDE

#if defined(_MSC_VER)
#pragma pack(push, 1)
#endif // endif

/*
 * In the following two structs keep cookie as last element
 * before data. This allows struct validation when fields
 * are added or deleted.  The data immediately follows the
 * structure and is required to be 4-byte aligned.
 *
 * OID_BCM_SETINFORMATION uses setinformation_t
 * OID_BCM_GETINFORMATION uses getinformation_t
*/
typedef struct _setinformation {
	uint32 cookie;   /* OIDENCAP_COOKIE */
	uint32 oid;	     /* actual OID value for set */
} setinformation_t;

#define SETINFORMATION_SIZE			(sizeof(setinformation_t))
#define SETINFORMATION_DATA(shdr)		((UCHAR *)&(shdr)[1])

typedef struct _getinformation {
	uint32 oid;	    /* actual OID value for query */
	uint32 len;	    /* length of response buffer, including this header */
	uint32 cookie;	/* OIDENCAP_COOKIE; altered by driver if more data available */
} getinformation_t;

#define GETINFORMATION_SIZE			(sizeof(getinformation_t))
#define GETINFORMATION_DATA(ghdr)		((UCHAR *)&(ghdr)[1])

typedef struct _reqinformation_hdr {
	uint32 version; /* REQINFORMATION_XXX_VERSION */
	uint32 cookie;  /* OIDENCAP_COOKIE; altered by driver if more data available */
	uint32 len;     /* REQINFORMATION_XXX_SIZE */
} reqinformation_hdr_t;

#define REQINFORMATION_HDR_SIZE			(sizeof(reqinformation_hdr_t))

/* This structure should be used as a replacement to
 * getinfomation_t and setinformation_t.
 * When new fields are added to this structure, add them to the end
 * and increment the version field.
*/
typedef struct _reqinformation_0 {
	reqinformation_hdr_t hdr;
	uint32 oid;     /* actual OID value for the request */
	uint32 idx;     /* bsscfg index */
	uint32 status;  /* NDIS_STATUS for actual OID */
/* Add new fields here... */
/* 4-byte aligned data follows */
} reqinformation_0_t;

#define REQINFORMATION_0_VERSION		0
#define REQINFORMATION_0_SIZE			(sizeof(reqinformation_0_t))
#define REQINFORMATION_0_DATA(ghdr)		((UCHAR *)(ghdr) + REQINFORMATION_0_SIZE)

typedef reqinformation_0_t reqinformation_t;

#define REQINFORMATION_VERSION			REQINFORMATION_0_VERSION
#define REQINFORMATION_SIZE			REQINFORMATION_0_SIZE
#define REQINFORMATION_DATA(ghdr)		REQINFORMATION_0_DATA(ghdr)

#if defined(_MSC_VER)
#pragma pack(pop)
#endif // endif

#endif /* _oidencap_h_ */
