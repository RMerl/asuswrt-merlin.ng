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

/* Passpoint IE OUI type */
#define HSPOT_IE_OUI_TYPE	0x10

/* Passpoint ANQP OUI type */
#define HSPOT_ANQP_OUI_TYPE	0x11
#define HSPOT_ANQP_OUI			"\x50\x6F\x9A\x11"

/* WNM type */
#define HSPOT_WNM_TYPE		1

/* Passpoint WNM type */
#define HSPOT_WNM_SUBSCRIPTION_REMEDIATION		0x00
#define HSPOT_WNM_DEAUTHENTICATION_IMMINENT		0x01

/* Passpoint config */
#define HSPOT_IE_DGAF_DISABLED	0x01	/* downstream group-addressed forward */

/* Passpoint config release2 */
#define HSPOT_DGAF_DISABLED_SHIFT	0
#define HSPOT_DGAF_DISABLED_MASK	(0x01 << HSPOT_DGAF_DISABLED_SHIFT)
#define HSPOT_PPS_MO_ID_SHIFT		1
#define HSPOT_PPS_MO_ID_MASK		(0x01 << HSPOT_PPS_MO_ID_SHIFT)
#define HSPOT_ANQP_DOMAIN_ID_SHIFT	2
#define HSPOT_ANQP_DOMAIN_ID_MASK	(0x01 << HSPOT_ANQP_DOMAIN_ID_SHIFT)

#define HSPOT_RELEASE_SHIFT		4
#define HSPOT_RELEASE_MASK		(0x0f << HSPOT_RELEASE_SHIFT)

/* Passpoint release numbers */
#define HSPOT_RELEASE_1		0
#define HSPOT_RELEASE_2		1

/* length includes OUI + type + subtype + reserved */
#define HSPOT_LENGTH_OVERHEAD	(WFA_OUI_LEN + 1 + 1 + 1)

/* subtype */
#define HSPOT_SUBTYPE_RESERVED				0
#define HSPOT_SUBTYPE_QUERY_LIST			1
#define HSPOT_SUBTYPE_CAPABILITY_LIST			2
#define HSPOT_SUBTYPE_OPERATOR_FRIENDLY_NAME		3
#define HSPOT_SUBTYPE_WAN_METRICS			4
#define HSPOT_SUBTYPE_CONNECTION_CAPABILITY		5
#define HSPOT_SUBTYPE_NAI_HOME_REALM_QUERY		6
#define HSPOT_SUBTYPE_OPERATING_CLASS_INDICATION	7
#define HSPOT_SUBTYPE_ONLINE_SIGNUP_PROVIDERS		8
#define HSPOT_SUBTYPE_ANONYMOUS_NAI			9
#define HSPOT_SUBTYPE_ICON_REQUEST			10
#define HSPOT_SUBTYPE_ICON_BINARY_FILE			11

/* WAN info - link status */
#define HSPOT_WAN_LINK_STATUS_SHIFT		0
#define HSPOT_WAN_LINK_STATUS_MASK		(0x03 << HSPOT_WAN_LINK_STATUS_SHIFT)
#define	HSPOT_WAN_LINK_UP			0x01
#define HSPOT_WAN_LINK_DOWN			0x02
#define HSPOT_WAN_LINK_TEST			0x03

/* WAN info - symmetric link */
#define HSPOT_WAN_SYMMETRIC_LINK_SHIFT		2
#define HSPOT_WAN_SYMMETRIC_LINK_MASK		(0x01 << HSPOT_WAN_SYMMETRIC_LINK_SHIFT)
#define HSPOT_WAN_SYMMETRIC_LINK		0x01
#define HSPOT_WAN_NOT_SYMMETRIC_LINK		0x00

/* WAN info - at capacity */
#define HSPOT_WAN_AT_CAPACITY_SHIFT		3
#define HSPOT_WAN_AT_CAPACITY_MASK		(0x01 << HSPOT_WAN_AT_CAPACITY_SHIFT)
#define HSPOT_WAN_AT_CAPACITY			0x01
#define HSPOT_WAN_NOT_AT_CAPACITY		0x00

/* IP Protocols for Connection Capability */
#define HSPOT_CC_IPPROTO_NONE			-1
#define HSPOT_CC_IPPROTO_ICMP			1
#define HSPOT_CC_IPPROTO_TCP			6
#define HSPOT_CC_IPPROTO_UDP			17
#define HSPOT_CC_IPPROTO_ESP			50

/* Port Numbers for Connection Capability */
#define HSPOT_CC_PORT_NONE				-1
#define HSPOT_CC_PORT_RESERVED			0
#define HSPOT_CC_PORT_FTP				20
#define HSPOT_CC_PORT_SSH				22
#define HSPOT_CC_PORT_HTTP				80
#define HSPOT_CC_PORT_HTTPS				443
#define HSPOT_CC_PORT_ISAKMP			500
#define HSPOT_CC_PORT_PPTP				1723
#define HSPOT_CC_PORT_IPSEC				4500
#define HSPOT_CC_PORT_SIP				5060

/* Port Status for Connection Capability */
#define HSPOT_CC_STATUS_NONE			-1
#define HSPOT_CC_STATUS_CLOSED			0
#define HSPOT_CC_STATUS_OPEN			1
#define HSPOT_CC_STATUS_UNKNOWN			2

/* OSU method */
#define HSPOT_OSU_METHOD_OMA_DM			0
#define HSPOT_OSU_METHOD_SOAP_XML		1

/* icon download status */
#define HSPOT_ICON_STATUS_SUCCESS			0
#define HSPOT_ICON_STATUS_FILE_NOT_FOUND		1
#define HSPOT_ICON_STATUS_UNSPECIFIED_FILE_ERROR	2

/* deauthentication reason */
#define HSPOT_DEAUTH_RC_BSS_DISALLOW	0
#define HSPOT_DEAUTH_RC_ESS_DISALLOW	1

#endif /* _BCM_HSPOT_H_ */
