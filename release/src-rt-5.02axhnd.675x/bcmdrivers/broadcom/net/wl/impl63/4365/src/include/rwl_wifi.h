/*
 * RWL definitions  of
 * Broadcom 802.11bang Networking Device Driver
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
 * $Id: rwl_wifi.h 708017 2017-06-29 14:11:45Z $
 *
 */

#ifndef _rwl_wifi_h_
#define _rwl_wifi_h_

#if defined(RWL_WIFI) || defined(WIFI_REFLECTOR) || defined(RFAWARE)

#define RWL_ACTION_WIFI_CATEGORY	127  /* Vendor-specific category value for WiFi */
#define RWL_WIFI_OUI_BYTE0		0x00 /* BRCM-specific public OUI */
#define RWL_WIFI_OUI_BYTE1		0x90
#define RWL_WIFI_OUI_BYTE2		0x4c
#define RWL_WIFI_ACTION_FRAME_SIZE	sizeof(struct dot11_action_wifi_vendor_specific)

/*
 * Information about the action frame data fields in the dot11_action_wifi_vendor_specific
 * cdc structure (1 to 16). This does not include the status flag. Since this
 * is not directly visible to the driver code, we can't use sizeof(struct cdc_ioctl).
 * Hence Ref MAC address offset starts from byte 17.
 * REF MAC ADDR (6 bytes (MAC Address len) from byte 17 to 22)
 * DUT MAC ADDR (6 bytes after the REF MAC Address byte 23 to 28)
 * unused (byte 29 to 49)
 * REF/Client Channel offset (50)
 * DUT/Server channel offset (51)
 * ---------------------------------------------------------------------------------------
 * cdc struct|REF MAC ADDR|DUT_MAC_ADDR|un used|REF Channel|DUT channel|Action frame Data|
 * 1---------17-----------23-------------------50----------51----------52----------------1040
 * REF MAC addr after CDC struct without status flag (status flag not used by wifi)
 */

#define RWL_REF_MAC_ADDRESS_OFFSET	17
#define RWL_DUT_MAC_ADDRESS_OFFSET	23
#define RWL_WIFI_CLIENT_CHANNEL_OFFSET	50
#define RWL_WIFI_SERVER_CHANNEL_OFFSET	51

#ifdef WIFI_REFLECTOR
#include <bcmcdc.h>
#define REMOTE_FINDSERVER_CMD 	16
#define RWL_WIFI_ACTION_CMD		"wifiaction"
#define RWL_WIFI_ACTION_CMD_LEN		11	/* With the NULL terminator */
#define REMOTE_SET_CMD 		1
#define REMOTE_GET_CMD 		2
#define REMOTE_REPLY 			4
#define RWL_WIFI_DEFAULT_TYPE           0x00
#define RWL_WIFI_DEFAULT_SUBTYPE        0x00
#define RWL_ACTION_FRAME_DATA_SIZE      1024	/* fixed size for the wifi frame data */
#define RWL_WIFI_CDC_HEADER_OFFSET      0
#define RWL_WIFI_FRAG_DATA_SIZE         960	/* max size of the frag data */
#define RWL_DEFAULT_WIFI_FRAG_COUNT 	127 	/* maximum fragment count */
#define RWL_WIFI_RETRY			5       /* CMD retry count for wifi */
#define RWL_WIFI_SEND			5	/* WIFI frame sent count */
#define RWL_WIFI_SEND_DELAY		100	/* delay between two frames */
#define MICROSEC_CONVERTOR_VAL		1000
#ifndef IFNAMSIZ
#define IFNAMSIZ			16
#endif // endif

typedef struct rem_packet {
	rem_ioctl_t rem_cdc;
	uchar message [RWL_ACTION_FRAME_DATA_SIZE];
} rem_packet_t;

#include <packed_section_start.h>
struct BWL_PRE_PACKED_STRUCT send_packet {
	char command [RWL_WIFI_ACTION_CMD_LEN];
	dot11_action_wifi_vendor_specific_t response;
} BWL_POST_PACKED_STRUCT;
#include <packed_section_end.h>

typedef struct send_packet send_packet_t;

#define REMOTE_SIZE     sizeof(rem_ioctl_t)
#endif /* WIFI_REFLECTOR */

typedef struct rwl_request {
	struct rwl_request* next_request;
	struct dot11_action_wifi_vendor_specific action_frame;
} rwl_request_t;

#endif /* defined(RWL_WIFI) || defined(WIFI_REFLECTOR) */
#endif	/* _rwl_wifi_h_ */
