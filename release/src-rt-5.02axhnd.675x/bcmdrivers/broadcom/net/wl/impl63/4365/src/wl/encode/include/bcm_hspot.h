/*
 * Hotspot2.0 specific constants as defined in Hotspot2.0 specification.
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
 * $Id:$
 */

#ifndef _BCM_HSPOT_H_
#define _BCM_HSPOT_H_

/* hotspot IE OUI type */
#define HSPOT_IE_OUI_TYPE		0x10

/* hotspot ANQP OUI type */
#define HSPOT_ANQP_OUI_TYPE		0x11
#define HSPOT_ANQP_OUI			"\x50\x6F\x9A\x11"

/* hotspot WNM OUI type */
#define HSPOT_WNM_OUI_TYPE		0x12

/* hostspot config */
#define HSPOT_IE_DGAF_DISABLED	0x01	/* downstream group-addressed forward */

/* hotspot config release2 */
#define HSPOT_DGAF_DISABLED_SHIFT	0
#define HSPOT_DGAF_DISABLED_MASK	(0x01 << HSPOT_DGAF_DISABLED_SHIFT)
#define HSPOT_OSU_BSSID_SHIFT		1
#define HSPOT_OSU_BSSID_MASK		(0x01 << HSPOT_OSU_BSSID_SHIFT)
#define HSPOT_RELEASE_SHIFT			4
#define HSPOT_RELEASE_MASK			(0x0f << HSPOT_RELEASE_SHIFT)

/* hotspot release numbers */
#define HSPOT_RELEASE_1		0
#define HSPOT_RELEASE_2		1

/* length includes OUI + type + subtype + reserved */
#define HSPOT_LENGTH_OVERHEAD	(WFA_OUI_LEN + 1 + 1 + 1)

/* subtype */
#define HSPOT_SUBTYPE_RESERVED						0
#define HSPOT_SUBTYPE_QUERY_LIST					1
#define HSPOT_SUBTYPE_CAPABILITY_LIST				2
#define HSPOT_SUBTYPE_OPERATOR_FRIENDLY_NAME		3
#define HSPOT_SUBTYPE_WAN_METRICS					4
#define HSPOT_SUBTYPE_CONNECTION_CAPABILITY			5
#define HSPOT_SUBTYPE_NAI_HOME_REALM_QUERY			6
#define HSPOT_SUBTYPE_OPERATING_CLASS_INDICATION	7
#define HSPOT_SUBTYPE_ONLINE_SIGNUP_PROVIDERS		8
#define HSPOT_SUBTYPE_ANONYMOUS_NAI					9
#define HSPOT_SUBTYPE_ICON_REQUEST					10
#define HSPOT_SUBTYPE_ICON_BINARY_FILE				11

/* WAN info - link status */
#define HSPOT_WAN_LINK_STATUS_SHIFT		0
#define HSPOT_WAN_LINK_STATUS_MASK		(0x03 << HSPOT_WAN_LINK_STATUS_SHIFT)
#define	HSPOT_WAN_LINK_UP				0x01
#define HSPOT_WAN_LINK_DOWN				0x02
#define HSPOT_WAN_LINK_TEST				0x03

/* WAN info - symmetric link */
#define HSPOT_WAN_SYMMETRIC_LINK_SHIFT	2
#define HSPOT_WAN_SYMMETRIC_LINK_MASK	(0x01 << HSPOT_WAN_SYMMETRIC_LINK_SHIFT)
#define HSPOT_WAN_SYMMETRIC_LINK		0x01
#define HSPOT_WAN_NOT_SYMMETRIC_LINK	0x00

/* WAN info - at capacity */
#define HSPOT_WAN_AT_CAPACITY_SHIFT		3
#define HSPOT_WAN_AT_CAPACITY_MASK		(0x01 << HSPOT_WAN_AT_CAPACITY_SHIFT)
#define HSPOT_WAN_AT_CAPACITY			0x01
#define HSPOT_WAN_NOT_AT_CAPACITY		0x00

/* connection capability */
#define HSPOT_CC_STATUS_CLOSED			0
#define HSPOT_CC_STATUS_OPEN			1
#define HSPOT_CC_STATUS_UNKNOWN			2

/* OSU method */
#define HSPOT_OSU_METHOD_OMA_DM			0
#define HSPOT_OSU_METHOD_SOAP_XML		1

/* icon download status */
#define HSPOT_ICON_STATUS_SUCCESS					0
#define HSPOT_ICON_STATUS_FILE_NOT_FOUND			1
#define HSPOT_ICON_STATUS_UNSPECIFIED_FILE_ERROR	2

/* WNM type */
#define HSPOT_WNM_TYPE		1

#endif /* _BCM_HSPOT_H_ */
