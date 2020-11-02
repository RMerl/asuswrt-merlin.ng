/*
 * GAS state machine functions which implements the GAS protocol
 * as defined in 802.11u.
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

#ifndef _BCM_GAS_H_
#define _BCM_GAS_H_

#include "typedefs.h"
#include "ethernet.h"

typedef struct bcm_gas_struct bcm_gas_t;

/* Opaque driver handle type. In dongle this is struct wlc_info_t, representing
 * the driver. On linux host this is struct ifreq, representing the primary OS
 * interface for a driver instance. To specify a virtual interface this should
 * be used together with a bsscfg index.
 */
struct bcm_gas_wl_drv_hdl;

/* event notification type */
typedef enum
{
	BCM_GAS_EVENT_QUERY_REQUEST,	 /* query request to the advertisement server */
	BCM_GAS_EVENT_TX,				 /* GAS packet transmited */
	BCM_GAS_EVENT_RX,				 /* GAS packet received */
	BCM_GAS_EVENT_RESPONSE_FRAGMENT, /* response fragment received */
	BCM_GAS_EVENT_STATUS,			 /* status at the completion of GAS exchange */
} bcm_gas_event_type_t;

#define BCM_GAS_MAX_QUERY_REQUEST_LENGTH	128

typedef struct
{
	uint16 len;
	uint8 data[BCM_GAS_MAX_QUERY_REQUEST_LENGTH];
} bcm_gas_query_request_t;

typedef struct
{
	int gasActionFrame;		/* GAS action frame type (eg. GAS_REQUEST_ACTION_FRAME) */
	int length;				/* packet length */
	uint8 fragmentId;		/* fragment ID and more bit (0x80) */
} bcm_gas_packet_t;

typedef struct
{
	int length;				/* fragment length */
	uint8 fragmentId;		/* fragment ID and more bit (0x80) */
} bcm_gas_fragment_t;

typedef struct
{
	uint16 statusCode;
} bcm_gas_status_t;

typedef struct
{
	bcm_gas_t *gas;
	struct ether_addr peer;
	uint8 dialogToken;
	bcm_gas_event_type_t type;
	union {
		bcm_gas_query_request_t queryReq;
		bcm_gas_packet_t tx;
		bcm_gas_packet_t rx;
		bcm_gas_fragment_t rspFragment;
		bcm_gas_status_t status;
	};
} bcm_gas_event_t;

/* enable/disable incoming GAS request (default is enable) */
void bcm_gas_incoming_request(int enable);

/* if incoming GAS request handling is enabled */
int bcm_gas_incoming_request_enabled(void);

/* set maximum number of GAS instances */
void bcm_gas_maximum_instances(int max);

/* set maximum number of incoming GAS instances */
void bcm_gas_maximum_incoming_instances(int max);

/* configure dot11GASPauseForServerResponse (default is FALSE) */
void bcm_gas_pause_for_server_response(int isPause);

/* override comeback delay timeout (default is 0) */
void bcm_gas_comeback_delay_override(int msec);

/* set comeback delay in GAS response for unpause */
void bcm_gas_set_comeback_delay_response_unpause(int msec);

/* set comeback delay in GAS response for pause */
void bcm_gas_set_comeback_delay_response_pause(int msec);

/* enable/disable fragment response (default is disable) */
void bcm_gas_fragment_response(int enable);

/* initialize GAS protocol */
int bcm_gas_initialize(void);
void bcm_gas_init_dsp(void);
void bcm_gas_init_wlan_handler(void);

/* deinitialize GAS protocol */
int bcm_gas_deinitialize(void);

/* create GAS protocol instance to destination */
bcm_gas_t *bcm_gas_create(struct bcm_gas_wl_drv_hdl *drv, int bsscfg_idx,
	uint16 channel, struct ether_addr *dst);

/* destroy GAS protocol instance */
int bcm_gas_destroy(bcm_gas_t *gas);

/* reset GAS protocol instance */
int bcm_gas_reset(bcm_gas_t *gas);

/* set maximum retransmit on no ACK from peer */
int bcm_gas_set_max_retransmit(bcm_gas_t *gas, uint16 count);

/* set transmit response timeout */
int bcm_gas_set_response_timeout(bcm_gas_t *gas, uint16 msec);

/* set maximum comeback delay expected for a response */
int bcm_gas_set_max_comeback_delay(bcm_gas_t *gas, uint16 msec);

/* start GAS protocol instance - send initial GAS request configued by bcm_gas_set_query_request */
int bcm_gas_start(bcm_gas_t *gas);

/* subscribe for GAS event notification callback */
int bcm_gas_subscribe_event(void *context,
	void (*fn)(void *context, bcm_gas_t *gas, bcm_gas_event_t *event));

/* unsubscribe for GAS event notification callback */
int bcm_gas_unsubscribe_event(void (*fn)(void *context,
	bcm_gas_t *gas, bcm_gas_event_t *event));

/* set query request */
int bcm_gas_set_query_request(bcm_gas_t *gas, int len, uint8 *data);

/* set query response data in response to a BCM_GAS_EVENT_QUERY_REQUEST event */
int bcm_gas_set_query_response(bcm_gas_t *gas, int len, uint8 *data);

/* retrieve query response length after protocol completes */
/* retrieve fragment response length in response to BCM_GAS_EVENT_RESPONSE_FRAGMENT */
int bcm_gas_get_query_response_length(bcm_gas_t *gas);

/* retrieve query response after protocol completes */
/* retrieve fragment response in response to BCM_GAS_EVENT_RESPONSE_FRAGMENT */
int bcm_gas_get_query_response(bcm_gas_t *gas, int dataLen, int *len, uint8 *data);

/* set bsscfg index */
int bcm_gas_set_bsscfg_index(bcm_gas_t *gas, int index);

/* configure dot11GASPauseForServerResponse per interface */
void bcm_gas_set_if_gas_pause(int isPause, struct bcm_gas_wl_drv_hdl *drv);

/* set comeback delay in GAS response for unpause per interface */
void bcm_gas_set_if_cb_delay_unpause(int msec, struct bcm_gas_wl_drv_hdl *drv);

/* returns TRUE if active GAS instance */
int bcm_gas_is_active(struct ether_addr *mac, uint8 dialogToken, int isIncoming);

/* get driver handle */
struct bcm_gas_wl_drv_hdl *bcm_gas_get_drv(bcm_gas_t *gas);

/* wlan event handler */
void bcm_gas_process_wlan_event(void *context, uint32 eventType,
	wl_event_msg_t *wlEvent, uint8 *data, uint32 length);

int bcm_gas_no_query_response(bcm_gas_t *gas);
#endif /* _BCM_GAS_H_ */
