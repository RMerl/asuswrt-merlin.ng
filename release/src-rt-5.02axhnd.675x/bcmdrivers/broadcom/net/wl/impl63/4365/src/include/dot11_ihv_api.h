/*
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
 * I/O interface for the Broadcom Native Wi-Fi IHV
 * Service Extension.
 *
 * $Id: $
 */

#if !defined(__DOT11_IHV_API_H__)
#define __DOT11_IHV_API_H__

#define BCM_IHV_NOTIFICATION_CODE_GEN_INDICATION	0

#define BCM_IHV_INIT_REQUEST(R, C, O, IL, OL) \
	do { \
	(*(PBCM_IHV_CONTROL_REQUEST_HDR)(R)) = _BCM_IHV_REQUEST_TEMPLATE_VAR; \
	((PBCM_IHV_CONTROL_REQUEST_HDR)(R))->command = (C); \
	((PBCM_IHV_CONTROL_REQUEST_HDR)(R))->info_offset = (O); \
	((PBCM_IHV_CONTROL_REQUEST_HDR)(R))->info_in_length = (IL); \
	((PBCM_IHV_CONTROL_REQUEST_HDR)(R))->info_out_length = (OL); \
	} while (0)

#define BCM_IHV_REQUEST_TEMPLATE_DEFINE() \
	const BCM_IHV_CONTROL_REQUEST_HDR _BCM_IHV_REQUEST_TEMPLATE_VAR = { \
		0x00, 0x10, 0x18, 0x00, \
		BCM_IHV_CONTROL_REQUEST_VERSION, \
		BCM_IHV_CONTROL_REQUEST_HDR_SIZEOF, \
		0, 0, 0, 0 }

typedef struct BCM_IHV_OUI {
	UINT8 octet[4];
} BCM_IHV_OUI, *PBCM_IHV_OUI;

typedef struct BCM_IHV_HDR {
	UINT8 version;
	UINT32 length;
} BCM_IHV_HDR, *PBCM_IHV_HDR;
#define BCM_IHV_HDR_SIZEOF (sizeof(BCM_IHV_HDR))

typedef struct BCM_IHV_MAC_ADDRESS {
	UINT8 octet[6];
} BCM_IHV_MAC_ADDRESS, *PBCM_IHV_MAC_ADDRESS;

#define BCM_IHV_CONTROL_REQUEST_VERSION 0

/* length is an in/out parameter. */
typedef struct BCM_IHV_CONTROL_REQUEST_HDR {
	BCM_IHV_OUI oui;	/* 00-10-18 (must be first) */
	BCM_IHV_HDR hdr;
	UINT32 command;		/* BCM_IHV_CMD */
	UINT32 info_offset;	/* offset from struct start to info buffer */
	UINT32 info_in_length;
	UINT32 info_out_length;
} BCM_IHV_CONTROL_REQUEST_HDR, *PBCM_IHV_CONTROL_REQUEST_HDR;
#define BCM_IHV_CONTROL_REQUEST_HDR_SIZEOF (sizeof(BCM_IHV_CONTROL_REQUEST_HDR))

extern const BCM_IHV_CONTROL_REQUEST_HDR _BCM_IHV_REQUEST_TEMPLATE_VAR;

typedef enum BCM_IHV_CMD {
	BCM_IHV_CMD_IE_CACHE = 0
} BCM_IHV_CMD;

#define BCM_IHV_CMD_SET_OP(C) ((C) & 0x80000000UL)

#define BCM_IHV_IE_CACHE_PARAMS_VERSION 0

typedef struct BCM_IHV_IE_CACHE_PARAMS {
	BCM_IHV_HDR hdr;
	BCM_IHV_MAC_ADDRESS mac_address;
} BCM_IHV_IE_CACHE_PARAMS, *PBCM_IHV_IE_CACHE_PARAMS;
#define BCM_IHV_IE_CACHE_PARAMS_SIZEOF (sizeof(BCM_IHV_IE_CACHE_PARAMS))

#if defined(NOTYET)
typedef struct BCM_IHV_IE_BLOB_INFO {
	ULONG length;
	ULONG frame_type;
	ULONG ie_blob_length;
	UCHAR ie_blob_offset;
} BCM_IHV_IE_BLOB_INFO, *PBCM_IHV_IE_BLOB_INFO;
#define BCM_IHV_IE_BLOB_INFO_SIZEOF (sizeof(BCM_IHV_IE_BLOB_INFO))
#endif /* NOTYET */

#define BCM_IHV_IE_BLOB_INFO_SIZE(_E_) ((_E_)->length + (_E_)->ie_blob_length)

#define BCM_IHV_IE_CACHE_DATA_VERSION 0

#define BCM_IHV_BYTE_ARRAY_VERSION 0

typedef struct BCM_IHV_BYTE_ARRAY {
	BCM_IHV_HDR hdr;
	ULONG buffer_length;
	ULONG buffer_max_length;
	UCHAR buffer[1];
} BCM_IHV_BYTE_ARRAY, *PBCM_IHV_BYTE_ARRAY;
#define BCM_IHV_BYTE_ARRAY_SIZEOF (OFFSETOF(BCM_IHV_BYTE_ARRAY, buffer))

/* Each field in this union must begin with BCM_IHV_HDR to
 * ensure correct alignment.
*/
typedef union BCM_IHV_CONTROL_INFO {
	/* BCM_IHV_CMD_IE_CACHE */
	BCM_IHV_IE_CACHE_PARAMS ie_cache_params;
	BCM_IHV_BYTE_ARRAY ie_cache_data;
} BCM_IHV_CONTROL_INFO, *PBCM_IHV_CONTROL_INFO;

typedef struct BCM_IHV_CONTROL_REQUEST {
	BCM_IHV_CONTROL_REQUEST_HDR hdr;
	BCM_IHV_CONTROL_INFO info;
} BCM_IHV_CONTROL_REQUEST, *PBCM_IHV_CONTROL_REQUEST;
#define BCM_IHV_CONTROL_REQUEST_SIZEOF (sizeof(BCM_IHV_CONTROL_REQUEST))

#define BCM_IHV_CONTROL_REQUEST_INFO_OFFSET \
	(OFFSETOF(BCM_IHV_CONTROL_REQUEST, info))

#endif /* !__DOT11_IHV_API_H__ */
