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

#if !defined(BCMDRIVER) || defined(DONGLEBUILD)
#include <stdlib.h>
#endif /* BCMDRIVER || DONGLEBUILD */
#include "proto/802.11.h"
#include "bcmendian.h"
#ifndef BCMDRIVER
#include "dsp.h"
#include "tmr.h"
#include "wlu_api.h"
#else
#include "wl_gas.h"
#endif /* BCMDRIVER */
#include "wl_dbg.h"
#include "bcm_encode_gas.h"
#include "bcm_decode_gas.h"
#include "bcm_encode_ie.h"
#include "bcm_decode_ie.h"
#include "bcm_gas.h"

#ifndef WL_PRMAC
#define WL_PRMAC(args)
#endif // endif

#ifdef BCMDRIVER
#define BCM_GAS_NO_MULTIPLE_INTERFACES	/* single wlan chip */
#define BCM_GAS_NO_DISPATCHER			/* direct function call */
#define BCM_GAS_NO_REASSEMBLY			/* host to reassemble GAS comeback */
#endif /* BCMDRIVER */

#define QUERY_RESPONSE_TIMEOUT				1000	/* milliseconds */
#define COMEBACK_DELAY_RESPONSE_UNPAUSE		QUERY_RESPONSE_TIMEOUT
#define COMEBACK_DELAY_RESPONSE_PAUSE		1		/* milliseconds */
#define DEFAULT_MAX_COMEBACK_DELAY			0xffff
#define DEFAULT_RESPONSE_TIMEOUT			2000	/* milliseconds */

#define DEFAULT_MAX_RETRANSMIT				0

#define MAX_FRAGMENT_SIZE				1024
#define MORE_BIT	0x80

#define NUM_FRAGMENT_PER_REALLOC		4

#define BCMGAS_PKT_ID(data)		(((unsigned long)data) & 0xffffffff)

#define TU_TO_US_SHIFT_BITS         10   /* 1024 = 2**10 */
#define TU_TO_MS(x)	((((uint32) x) << TU_TO_US_SHIFT_BITS) / 1000)
#define MS_TO_TU(x)	((((uint32) x) * 1000) >> TU_TO_US_SHIFT_BITS)

typedef struct {
	struct bcm_gas_wl_drv_hdl *drv;
	int pause;
} IfGASPauseForServerResponseT;

typedef struct {
	struct bcm_gas_wl_drv_hdl *drv;
	int CBDelay;
} IfCBDelayT;

#ifndef BCM_GAS_NO_MULTIPLE_INTERFACES
static IfGASPauseForServerResponseT gIfGASPause[MAX_WLIF_NUM];
static int gIfGASPause_num = 0;

static IfCBDelayT gIfCBDelayUnpause[MAX_WLIF_NUM];
static int gIfCBDelayUnpause_num = 0;
#endif	/* BCM_GAS_NO_MULTIPLE_INTERFACES */

/* support incoming request */
static int gIncomingRequest = TRUE;

/* implements pause/no pause for server response */
static int gDot11GASPauseForServerResponse = TRUE;

/* override comeback delay */
static int gComebackDelayOverride = 0;

/* comeback delay in GAS response with false gDot11GASPauseForServerResponse */
static int gComebackDelayResponseUnpause = COMEBACK_DELAY_RESPONSE_UNPAUSE;

/* comeback delay in GAS response with true gDot11GASPauseForServerResponse */
static int gComebackDelayResponsePause = COMEBACK_DELAY_RESPONSE_PAUSE;

/* fragment response */
static int gFragmentResponse =
#ifdef BCM_GAS_NO_REASSEMBLY
	TRUE;
#else
	FALSE;
#endif	/* BCM_GAS_NO_REASSEMBLY */

/* dialog token */
static uint8 gNextDialogToken = 0;

/* head of link list of instances for initiated and incoming */
static int gGasInstanceMaximum = 4;
static bcm_gas_t *gGasInstance = 0;
static int gGasInstanceIncomingMaximum = 4;
static bcm_gas_t *gGasInstanceIncoming = 0;

typedef struct {
	void (*fn)(void *context, bcm_gas_t *gas, bcm_gas_event_t *event);
	void *context;
} gEventCallback_t;

gEventCallback_t gEventCallback;

/* state machine states */
typedef enum {
	/* initial idle state - pending START or RX_REQUEST events */
	STATE_IDLE,
	/* request tx'ed - pending RX_RESPONSE event */
	STATE_TX_REQUEST,
	/* response tx'ed - pending COMEBACK_REQUEST event (if more data) */
	STATE_TX_RESPONSE,
	/* comeback request tx'ed - pending COMEBACK_RESPONSE event */
	STATE_TX_COMEBACK_REQUEST,
	/* comeback response tx'ed - pending COMEBACK_REQUEST event (if more data) */
	STATE_TX_COMEBACK_RESPONSE,
	/* request tx'ed, query request posted - pending COMEBACK_REQUEST event, query response */
	STATE_TX_RESPONSE_AND_QUERY_REQUEST,
	/* query request posted - pending query response */
	STATE_WAIT_FOR_QUERY_RESPONSE,
	/* response rx'ed - waiting comeback delay */
	STATE_WAIT_COMEBACK_DELAY,
	/* error response tx'ed */
	STATE_ERROR,
	/* last state */
	STATE_LAST
} stateT;

#if defined(BCMDBG)
/* state to string */
static char *state_str[] = {
	"STATE_IDLE",
	"STATE_TX_REQUEST",
	"STATE_TX_RESPONSE",
	"STATE_TX_COMEBACK_REQUEST",
	"STATE_TX_COMEBACK_RESPONSE",
	"STATE_TX_RESPONSE_AND_QUERY_REQUEST",
	"STATE_WAIT_FOR_QUERY_RESPONSE",
	"STATE_WAIT_COMEBACK_DELAY",
	"STATE_ERROR"
};

/* event to string */
static char *event_str[] = {
	"EVENT_RESET",
	"EVENT_CONFIGURE",
	"EVENT_START",
	"EVENT_RX_REQUEST",
	"EVENT_RX_RESPONSE",
	"EVENT_RX_COMEBACK_REQUEST",
	"EVENT_RX_COMEBACK_RESPONSE",
	"EVENT_RX_QUERY_RESPONSE",
	"EVENT_RESPONSE_TIMEOUT",
	"EVENT_COMEBACK_DELAY_TIMEOUT",
	"EVENT_QUERY_RESPONSE_TIMEOUT",
	"EVENT_ACTION_FRAME_TX_SUCCESS",
	"EVENT_ACTION_FRAME_TX_FAILED",
};
#endif	/* BCMDBG */

/* state machine events */
typedef enum {
	EVENT_RESET,					/* reset state machine */
	EVENT_CONFIGURE,				/* configure state machine */
	EVENT_START,					/* transmit GAS request */
	EVENT_RX_REQUEST,				/* receive GAS request */
	EVENT_RX_RESPONSE,				/* receive GAS response */
	EVENT_RX_COMEBACK_REQUEST,		/* receive GAS comeback request */
	EVENT_RX_COMEBACK_RESPONSE,		/* receive GAS comeback response */
	EVENT_RX_QUERY_RESPONSE,		/* receive query resoponse */
	EVENT_RESPONSE_TIMEOUT,			/* no response timeout */
	EVENT_COMEBACK_DELAY_TIMEOUT,	/* comeback delay timeout */
	EVENT_QUERY_RESPONSE_TIMEOUT,	/* no query response timeout */
	EVENT_ACTION_FRAME_TX_SUCCESS,	/* ACK received for action frame transmit */
	EVENT_ACTION_FRAME_TX_FAILED,	/* no ACK received for action frame transmit */
	EVENT_LAST			/* last event */
} eventT;

struct bcm_gas_struct
{
	/* wl driver handle */
	struct bcm_gas_wl_drv_hdl *drv;

	/* bsscfg index */
	int bsscfgIndex;

	/* incoming request */
	int isIncoming;

	/* channel */
	uint16 channel;

	/* mac address */
	struct ether_addr mac;

	/* peer address */
	struct ether_addr peer;

	/* advertisement protocol */
	uint8 advertisementProtocol;

	/* maximum retransmit */
	uint16 maxRetransmit;

	/* response timeout */
	uint16 responseTimeout;

	/* maximum comeback delay */
	uint16 maxComebackDelayTU;

	/* timers */
	tmrT *responseTimer;
	tmrT *comebackDelayTimer;
	tmrT *queryResponseTimer;

	/* FSM state */
	stateT state;
	stateT nextState;

	/* current dialog token */
	uint8 dialogToken;

	/* current status code */
	uint16 statusCode;

	/* pending status notification */
	int isPendingStatusNotification;

	/* retransmit count */
	int retransmit;

	/* action frame tx parameters */
	struct {
		uint32 packetId;
		uint32 channel;
		uint16 responseTimeout;
		struct ether_addr bssid;
		struct ether_addr dst;
		int gasActionFrame;
		int length;
		uint8 data[ACTION_FRAME_SIZE];
	} tx;

	/* tx request data */
	struct {
		uint32 length;
		uint8 *data;
	} txRequest;

	/* tx response data */
	struct {
		uint8 fragmentId;
		uint8 lastFragmentId;
		uint32 length;
		uint8 *data;
	} txResponse;

	/* rx response data */
	struct {
		int isValid;
		uint8 fragmentId;
		uint32 maxLength;
		uint32 length;
		uint8 *data;
	} rxResponse;

	/* next in linked list */
	struct bcm_gas_struct *next;
	bool isNoResponse;
};

typedef struct bcm_gas_req bcm_gas_req_t;

/* request handler */
typedef void (*request_handler_t)(bcm_gas_t *gas,
	int reqLength, bcm_gas_req_t *req, void *rspData);

typedef struct {
	int isIncoming;
	struct bcm_gas_wl_drv_hdl *drv;
	int bsscfgIndex;
	uint16 channel;
	struct ether_addr peer;
} bcm_gas_create_req_t;

typedef struct {
	bcm_gas_t *gas;
} bcm_gas_create_rsp_t;

typedef struct {
	uint16 value;
} bcm_gas_set_param_req_t;

typedef struct {
	int len;
	uint8 *data;
} bcm_gas_set_query_request_req_t;

typedef struct {
	int len;
	uint8 *data;
} bcm_gas_set_query_response_req_t;

typedef struct {
	int index;
} bcm_gas_set_bsscfg_index_req_t;

typedef struct {
	int len;
} bcm_gas_get_query_response_length_rsp_t;

typedef struct {
	int maxLen;
	int *len;
	uint8 *data;
} bcm_gas_get_query_response_rsp_t;

struct bcm_gas_req {
	request_handler_t handler;
	union {
		bcm_gas_create_req_t create;
		bcm_gas_set_param_req_t setParam;
		bcm_gas_set_query_request_req_t setQueryRequest;
		bcm_gas_set_query_response_req_t setQueryResponse;
		bcm_gas_set_bsscfg_index_req_t setBsscfgIndex;
	};
};

/* state transition occurs at the end of fsm() processing */
#define change_state(gas, next)	((gas)->nextState = (next))

/* forward declarations */
static void delete_response_data(bcm_gas_t *gas);
static void rx_query_response(bcm_gas_t *gas, int dataLen, uint8 *data);
static void state_wait_query_response_process_query_response(bcm_gas_t *gas);
static void rx_notification(bcm_gas_t *gas, bcm_decode_gas_t *gasDecode, int length);
static void fsm(bcm_gas_t *gas, eventT event,
	bcm_decode_gas_t *gasDecode, int dataLen, uint8 *data);
static gEventCallback_t * get_gEventCallback(void);

/* ----------------------------------------------------------- */

/** To prevent ROMming shdat issue because of ROMmed functions accessing RAM */
static gEventCallback_t* BCMRAMFN(get_gEventCallback)(void)
{
	return &gEventCallback;
}

static int add_entry(bcm_gas_t *gas)
{
	int max, i;
	bcm_gas_t **head, **curr;
#ifdef BCMDBG
	char eabuf[ETHER_ADDR_STR_LEN];
#endif // endif

	if (gas->isIncoming) {
		max = gGasInstanceIncomingMaximum;
		head = &gGasInstanceIncoming;
	} else {
		max = gGasInstanceMaximum;
		head = &gGasInstance;
	}

	/* find from head of link list */
	curr = head;
	while (*curr != 0) {
		if ((memcmp(&(*curr)->peer, &gas->peer,	sizeof((*curr)->peer)) == 0) &&
			((*curr)->drv == gas->drv)) {
#ifdef BCMDBG
			WL_PRMAC(("already existing mac %s\n", bcm_ether_ntoa(&(*curr)->peer,
				eabuf)));
#endif // endif
		}
		curr = &(*curr)->next;
	}

	/* add to end of link list */
	curr = head;
	i = 0;
	while (*curr != 0) {
		if (++i >= max) {
			WL_ERROR(("maximum number of instances %d\n", max));
			return FALSE;
		}
		curr = &(*curr)->next;
	}
	*curr = gas;
	WL_NONE(("gas add_entry: %p\n", gas));
	gas->next = 0;
	return TRUE;
}

static int del_entry(bcm_gas_t *gas)
{
	bcm_gas_t **head, **curr;

	if (gas->isIncoming) {
		head = &gGasInstanceIncoming;
	} else {
		head = &gGasInstance;
	}

	/* find from head of link list */
	curr = head;
	while (*curr != 0) {
		if (*curr == gas) {
			*curr = (*curr)->next;
			return TRUE;
		}
		curr = &(*curr)->next;
	}

	return FALSE;
}

static bcm_gas_t *find_entry(struct ether_addr *mac, int isIncoming)
{
	bcm_gas_t **head, **curr;

	if (isIncoming) {
		head = &gGasInstanceIncoming;
	} else {
		head = &gGasInstance;
	}

	/* find from head of link list */
	curr = head;
	while (*curr != 0) {
		if (memcmp(&(*curr)->peer, mac, sizeof((*curr)->peer)) == 0) {
			return *curr;
		}
		curr = &(*curr)->next;
	}

	return 0;
}

static int is_entry_valid(bcm_gas_t *gas)
{
	bcm_gas_t **curr;

	/* find from head of link list */
	curr = &gGasInstanceIncoming;
	while (*curr != 0) {
		if (*curr == gas) {
			return TRUE;
		}
		curr = &(*curr)->next;
	}

	/* find from head of link list */
	curr = &gGasInstance;
	while (*curr != 0) {
		if (*curr == gas) {
			return TRUE;
		}
		curr = &(*curr)->next;
	}

	return FALSE;
}

/* Lookup GAS pointer from a 32-bit action frame packet ID */
static bcm_gas_t *
find_gas_entry_core(uint32 packet_id, bcm_gas_t *list)
{
	uint32 gas_ptr_id;
	bcm_gas_t *curr;

	curr = list;
	while (curr != NULL) {
		gas_ptr_id = BCMGAS_PKT_ID(curr);
		if (packet_id == gas_ptr_id) {
			WL_NONE(("%s incoming: found %p\n", __FUNCTION__, curr));
			return curr;
		}
		curr = curr->next;
	}
	return NULL;
}

static bcm_gas_t *
find_gas_entry(uint32* packet_id_ptr)
{
	bcm_gas_t *gas;

	gas = find_gas_entry_core(*packet_id_ptr, gGasInstanceIncoming);
	if (gas == NULL) {
		gas = find_gas_entry_core(*packet_id_ptr, gGasInstance);
	}
	return gas;
}

/* returns TRUE if active GAS instance */
int bcm_gas_is_active(struct ether_addr *mac, uint8 dialogToken, int isIncoming)
{
	bcm_gas_t *gas;

	gas = find_entry(mac, isIncoming);
	if (gas != 0 && gas->dialogToken == dialogToken) {
		return TRUE;
	}

	return FALSE;
}

/* ----------------------------------------------------------- */

static void response_timeout(void *arg)
{
	bcm_gas_t *gas = (bcm_gas_t *)arg;

	WL_TRACE(("response_timeout callback\n"));
	fsm(gas, EVENT_RESPONSE_TIMEOUT, 0, 0, 0);
}

static void comeback_delay_timeout(void *arg)
{
	bcm_gas_t *gas = (bcm_gas_t *)arg;

	WL_TRACE(("comeback_delay_timeout callback\n"));
	fsm(gas, EVENT_COMEBACK_DELAY_TIMEOUT, 0, 0, 0);
}

static void query_response_timeout(void *arg)
{
	bcm_gas_t *gas = (bcm_gas_t *)arg;

	WL_TRACE(("queryResponseTimer callback\n"));
	fsm(gas, EVENT_QUERY_RESPONSE_TIMEOUT, 0, 0, 0);
}

/* ----------------------------------------------------------- */

/* enable/disable incoming GAS request (default is enable) */
void bcm_gas_incoming_request(int enable)
{
	gIncomingRequest = enable;
}

/* ----------------------------------------------------------- */

/* if incoming GAS request handling is enabled */
int bcm_gas_incoming_request_enabled(void)
{
	return gIncomingRequest;
}

/* ----------------------------------------------------------- */

/* set maximum number of GAS instances */
void bcm_gas_maximum_instances(int max)
{
	gGasInstanceMaximum = max;
}

/* set maximum number of incoming GAS instances */
void bcm_gas_maximum_incoming_instances(int max)
{
	gGasInstanceIncomingMaximum = max;
}

/* ----------------------------------------------------------- */

/* configure dot11GASPauseForServerResponse (default is FALSE) */
void bcm_gas_pause_for_server_response(int isPause)
{
	gDot11GASPauseForServerResponse = isPause;
}

#ifndef BCM_GAS_NO_MULTIPLE_INTERFACES
/* configure dot11GASPauseForServerResponse per interface */
void bcm_gas_set_if_gas_pause(int isPause, struct bcm_gas_wl_drv_hdl *drv)
{
	int i;
	for (i = 0; i < gIfGASPause_num; i++) {
		if (gIfGASPause[i].drv == drv) {
			gIfGASPause[i].pause = isPause;
			return;
		}
	}

	if (gIfGASPause_num < MAX_WLIF_NUM) {
		gIfGASPause[gIfGASPause_num].pause = isPause;
		gIfGASPause[gIfGASPause_num].drv = drv;
		gIfGASPause_num ++;
	}
}
#endif	/* BCM_GAS_NO_MULTIPLE_INTERFACES */

static int get_gas_pause_for_server_response(struct bcm_gas_wl_drv_hdl *drv)
{
#ifndef BCM_GAS_NO_MULTIPLE_INTERFACES
	int i;
	for (i = 0; i < gIfGASPause_num; i++) {
		if (gIfGASPause[i].drv == drv) {
			return gIfGASPause[i].pause;
		}
	}
#else
	(void)drv;
#endif	/* BCM_GAS_NO_MULTIPLE_INTERFACES */
	return gDot11GASPauseForServerResponse;
}

/* ----------------------------------------------------------- */

/* override comeback delay timeout (default is 0) */
void bcm_gas_comeback_delay_override(int msec)
{
	gComebackDelayOverride = msec;
}

/* set comeback delay in GAS response for unpause */
void bcm_gas_set_comeback_delay_response_unpause(int msec)
{
	gComebackDelayResponseUnpause = msec;
}

#ifndef BCM_GAS_NO_MULTIPLE_INTERFACES
/* set comeback delay in GAS response for unpause per interface */
void bcm_gas_set_if_cb_delay_unpause(int msec, struct bcm_gas_wl_drv_hdl *drv)
{
	int i;
	for (i = 0; i < gIfCBDelayUnpause_num; i++) {
		if (gIfCBDelayUnpause[i].drv == drv) {
			gIfCBDelayUnpause[i].CBDelay = msec;
			return;
		}
	}

	if (gIfCBDelayUnpause_num < MAX_WLIF_NUM) {
		gIfCBDelayUnpause[gIfCBDelayUnpause_num].CBDelay = msec;
		gIfCBDelayUnpause[gIfCBDelayUnpause_num].drv = drv;
		gIfCBDelayUnpause_num ++;
	}
}
#endif	/* BCM_GAS_NO_MULTIPLE_INTERFACES */

static int get_comeback_delay_response_unpause(void *drv)
{
#ifndef BCM_GAS_NO_MULTIPLE_INTERFACES
	int i;
	for (i = 0; i < gIfCBDelayUnpause_num; i++) {
		if (gIfCBDelayUnpause[i].drv == drv) {
			return gIfCBDelayUnpause[i].CBDelay;
		}
	}
#else
	(void)drv;
#endif	/* BCM_GAS_NO_MULTIPLE_INTERFACES */
	return gComebackDelayResponseUnpause;
}

/* set comeback delay in GAS response for pause */
void bcm_gas_set_comeback_delay_response_pause(int msec)
{
	gComebackDelayResponsePause = msec;
}

/* enable/disable fragment response (default is disable) */
void bcm_gas_fragment_response(int enable)
{
	gFragmentResponse = enable;
}

/* get driver handle */
struct bcm_gas_wl_drv_hdl *bcm_gas_get_drv(bcm_gas_t *gas)
{
	return gas->drv;
}

/* ----------------------------------------------------------- */

#ifndef BCMDRIVER
void bcm_gas_init_dsp(void)
{
	/* attach handler to dispatcher */
	dspSubscribe(dsp(), 0, bcm_gas_process_wlan_event);
}

void bcm_gas_init_wlan_handler(void)
{
	void *ifr;
	int i = 0;

	WL_TRACE(("bcm_gas_init_wlan_handler\n"));

	for (ifr = wl(); ifr; ifr = wlif(++i)) {
		/* enable action frame rx event */
		if (wl_enable_event_msg(ifr, WLC_E_ACTION_FRAME_COMPLETE) < 0) {
			WL_ERROR(("failed to enable action frame complete event\n"));
		}
		if (wl_enable_event_msg(ifr, WLC_E_ACTION_FRAME_OFF_CHAN_COMPLETE) < 0) {
			WL_ERROR(("failed to enable act frame off chan complete event\n"));
		}
		if (wl_enable_event_msg(ifr, WLC_E_ACTION_FRAME_RX) < 0) {
			WL_ERROR(("failed to enable action frame rx event\n"));
		}
	}
}

static void bcm_gas_init_handler(bcm_gas_t *gasNull,
	int reqLength, bcm_gas_req_t *req, void *rspNull)
{
	(void)gasNull;
	(void)rspNull;
	if (reqLength != sizeof(bcm_gas_req_t) || req == 0) {
		WL_ERROR(("invalid parameter\n"));
		return;
	}
	WL_TRACE(("bcm_gas_init_handler\n"));
	bcm_gas_init_wlan_handler();
}

/* initialize GAS protocol */
int bcm_gas_initialize(void)
{
	bcm_gas_req_t req;

	WL_TRACE(("bcm_gas_initialize\n"));

	/* attach handler to dispatcher */
	dspSubscribe(dsp(), 0, bcm_gas_process_wlan_event);

	req.handler = (request_handler_t)bcm_gas_init_handler;
	return dspRequestSynch(dsp(), 0, sizeof(req), (uint8 *)&req, 0);
}

/* ----------------------------------------------------------- */

/* deinitialize GAS protocol */
int bcm_gas_deinitialize(void)
{
	/* detach handler */
	dspUnsubscribe(dsp(), bcm_gas_process_wlan_event);
	return TRUE;
}
#endif /* BCMDRIVER */

/* ----------------------------------------------------------- */

bcm_gas_t *bcm_gas_create(struct bcm_gas_wl_drv_hdl *drv, int bsscfg_idx,
	uint16 channel, struct ether_addr *dst)
{
	bcm_gas_t *gas;
#ifdef BCMDBG
	char eabuf[ETHER_ADDR_STR_LEN];
#endif // endif

	WL_TRACE(("bcm_gas_create\n"));

	gas = wl_gas_malloc(drv, sizeof(*gas));
	if (gas == 0)
		return 0;

	memset(gas, 0, sizeof(*gas));
	gas->isIncoming = FALSE;
	gas->drv = drv;
	gas->bsscfgIndex = bsscfg_idx;
	gas->channel = channel;
	memcpy(&gas->peer, dst, sizeof(gas->peer));
	gas->advertisementProtocol = ADVP_ANQP_PROTOCOL_ID;
	gas->maxRetransmit = DEFAULT_MAX_RETRANSMIT;
	gas->responseTimeout = DEFAULT_RESPONSE_TIMEOUT;
	gas->maxComebackDelayTU = DEFAULT_MAX_COMEBACK_DELAY;
	WL_PRMAC(("creating instance for peer mac %s\n", bcm_ether_ntoa(&gas->peer, eabuf)));

	/* save new entry */
	del_entry(gas);
	if (!add_entry(gas)) {
		WL_ERROR(("failed to add new entry\n"));
		goto fail;
	}

	/* retrieve MAC address */
	if (wl_cur_etheraddr(gas->drv, gas->bsscfgIndex, &gas->mac) < 0) {
		WL_ERROR(("failed to get mac address\n"));
	}
	WL_PRMAC(("MAC addr %s\n", bcm_ether_ntoa(&gas->mac, eabuf)));

	/* create timers */
	gas->responseTimer = tmrCreate(
#ifndef BCMDRIVER
		dsp(),
#else
		gas->drv,
#endif	/* BCMDRIVER */
		response_timeout, gas, "responseTimer");
	if (gas->responseTimer == 0) {
		WL_ERROR(("failed to create timer\n"));
		goto fail;
	}
	gas->comebackDelayTimer = tmrCreate(
#ifndef BCMDRIVER
		dsp(),
#else
		gas->drv,
#endif	/* BCMDRIVER */
		comeback_delay_timeout, gas, "comebackDelayTimer");
	if (gas->comebackDelayTimer == 0) {
		WL_ERROR(("failed to create timer\n"));
		goto fail;
	}
	gas->queryResponseTimer = tmrCreate(
#ifndef BCMDRIVER
		dsp(),
#else
		gas->drv,
#endif	/* BCMDRIVER */
		query_response_timeout, gas, "queryResponseTimer");
	if (gas->queryResponseTimer == 0) {
		WL_ERROR(("failed to create timer\n"));
		goto fail;
	}

	/* reset state machine */
	fsm(gas, EVENT_RESET, 0, 0, 0);

	/* return created instance */
	return gas;

fail:
	del_entry(gas);
	wl_gas_free(gas->drv, gas);
		return 0;
	}

/* ----------------------------------------------------------- */

int bcm_gas_destroy(bcm_gas_t *gas)
{
	if (gas == 0) {
		WL_ERROR(("invalid parameter\n"));
		return 0;
	}
	WL_TRACE(("bcm_gas_destroy\n"));

	/* stop timers */
	tmrStop(gas->responseTimer);
	tmrStop(gas->comebackDelayTimer);
	tmrStop(gas->queryResponseTimer);

	/* destroy timers */
	tmrDestroy(gas->responseTimer);
	tmrDestroy(gas->comebackDelayTimer);
	tmrDestroy(gas->queryResponseTimer);

	del_entry(gas);

	if (gas->txRequest.data)
		wl_gas_free(gas->drv, gas->txRequest.data);
	if (gas->txResponse.data)
		wl_gas_free(gas->drv, gas->txResponse.data);
	if (gas->rxResponse.data)
		wl_gas_free(gas->drv, gas->rxResponse.data);

	wl_gas_free(gas->drv, gas);
	return 1;
}

/* ----------------------------------------------------------- */

int bcm_gas_reset(bcm_gas_t *gas)
{
	if (gas == 0) {
		WL_ERROR(("invalid parameter\n"));
		return 0;
	}
	WL_TRACE(("bcm_gas_reset\n"));
	fsm(gas, EVENT_RESET, 0, 0, 0);
	return 1;
}

/* ----------------------------------------------------------- */

int bcm_gas_set_max_retransmit(bcm_gas_t *gas, uint16 count)
{
	if (gas == 0) {
		WL_ERROR(("invalid parameter\n"));
		return 0;
	}
	WL_TRACE(("bcm_gas_set_max_retransmit\n"));
	if (gas->state == STATE_IDLE) {
		gas->maxRetransmit = count;
	}
	return 1;
}

/* ----------------------------------------------------------- */

int bcm_gas_set_response_timeout(bcm_gas_t *gas, uint16 msec)
{
	if (gas == 0) {
		WL_ERROR(("invalid parameter\n"));
		return 0;
	}
	WL_TRACE(("bcm_gas_set_response_timeout\n"));
	if (gas->state == STATE_IDLE) {
		gas->responseTimeout = msec;
	}
	return 1;
}

/* ----------------------------------------------------------- */

int bcm_gas_set_max_comeback_delay(bcm_gas_t *gas, uint16 msec)
{
	if (gas == 0) {
		WL_ERROR(("invalid parameter\n"));
		return 0;
	}
	WL_TRACE(("bcm_gas_set_max_comeback_delay\n"));
	if (gas->state == STATE_IDLE) {
		gas->maxComebackDelayTU = MS_TO_TU(msec);
	}
	return 1;
}

/* ----------------------------------------------------------- */

int bcm_gas_start(bcm_gas_t *gas)
{
	if (gas == 0) {
		WL_ERROR(("invalid parameter\n"));
		return 0;
	}
	WL_TRACE(("bcm_gas_start\n"));
	fsm(gas, EVENT_START, 0, 0, 0);
	return 1;
}

/* ----------------------------------------------------------- */

/* subscribe for GAS event notification callback */
int bcm_gas_subscribe_event(void *context,
	void (*fn)(void *context, bcm_gas_t *gas, bcm_gas_event_t *event))
{
	gEventCallback_t *p = get_gEventCallback();
	p->fn = fn;
	p->context = context;
	return TRUE;
}

/* ----------------------------------------------------------- */

/* unsubscribe for GAS event notification callback */
int bcm_gas_unsubscribe_event(void (*fn)(void *context,
	bcm_gas_t *gas, bcm_gas_event_t *event))
{
	gEventCallback_t *p = get_gEventCallback();

	(void)fn;
	memset(p, 0, sizeof(gEventCallback));
	return TRUE;
}

/* ----------------------------------------------------------- */

int bcm_gas_set_query_request(bcm_gas_t *gas, int len, uint8 *data)
{
	if (gas == 0) {
		WL_ERROR(("invalid parameter\n"));
		return 0;
	}
	WL_TRACE(("bcm_gas_set_query_request\n"));
	if (gas->state == STATE_IDLE) {
		if (gas->txRequest.data != 0) {
			wl_gas_free(gas->drv, gas->txRequest.data);
			gas->txRequest.data = 0;
		}
		gas->txRequest.length = len;
		ASSERT(len);
		gas->txRequest.data = wl_gas_malloc(gas->drv, len);
		if (gas->txRequest.data == 0) {
			gas->txRequest.length = 0;
		} else {
			memcpy(gas->txRequest.data, data,
			gas->txRequest.length);
		}
	}
	return 1;
}

/* ----------------------------------------------------------- */

int bcm_gas_set_query_response(bcm_gas_t *gas, int len, uint8 *data)
{
	if (gas == 0) {
		WL_ERROR(("invalid parameter\n"));
		return 0;
	}
	WL_TRACE(("bcm_gas_set_query_response\n"));

	if (!is_entry_valid(gas)) {
		/* timeout will cause entry to be no longer valid */
		return 0;
	}

	/* only valid in idle state */
	if (gas->state != STATE_IDLE) {
		return 0;
	}
	rx_query_response(gas, len, data);

	return 1;
}

/* ----------------------------------------------------------- */

int bcm_gas_get_query_response_length(bcm_gas_t *gas)
{
	int length;

	if (gas == 0) {
		WL_ERROR(("invalid parameter\n"));
		return 0;
	}
	WL_TRACE(("bcm_gas_get_query_response_length\n"));

	if (gFragmentResponse || gas->rxResponse.isValid)
		length = gas->rxResponse.length;
	else
		length = 0;

	return length;
}

/* ----------------------------------------------------------- */

int bcm_gas_get_query_response(bcm_gas_t *gas, int dataLen, int *len, uint8 *data)
{
	if (gas == 0) {
		WL_ERROR(("invalid parameter\n"));
		return 0;
	}
	WL_TRACE(("bcm_gas_get_query_response\n"));

	if ((gFragmentResponse || gas->rxResponse.isValid) &&
		(int)gas->rxResponse.length <= dataLen) {
		*len = gas->rxResponse.length;
		memcpy(data, gas->rxResponse.data, *len);
	}
	else {
		*len = 0;
	}

	if (gFragmentResponse)
		delete_response_data(gas);

	return 1;
}

/* ----------------------------------------------------------- */

int bcm_gas_set_bsscfg_index(bcm_gas_t *gas, int index)
{
	if (gas == 0) {
		WL_ERROR(("invalid parameter\n"));
		return 0;
	}
	WL_TRACE(("bcm_gas_set_bsscfg_index\n"));

	gas->bsscfgIndex = index;

	return 1;
}

/* ----------------------------------------------------------- */

static int is_valid_advertisement_protocol(bcm_decode_ie_adv_proto_tuple_t *apie)
{
	if (apie == 0)
		return FALSE;

	if (apie->protocolId != ADVP_ANQP_PROTOCOL_ID) {
		WL_P2PO(("invalid protocol ID %d\n", apie->protocolId));
		return FALSE;
	}

	return TRUE;
}

void bcm_gas_process_wlan_event(void *context, uint32 eventType,
	wl_event_msg_t *wlEvent, uint8 *data, uint32 length)
{
	eventT event;
	struct bcm_gas_wl_drv_hdl *drv = context;
#ifdef BCMDBG
	char eabuf[ETHER_ADDR_STR_LEN];
#endif // endif

	switch (eventType) {
	case WLC_E_ACTION_FRAME_COMPLETE:
	{
		uint32 *packetId = (uint32 *)data;
		bcm_gas_t *gas = find_gas_entry(packetId);

		WL_TRACE(("WLC_E_ACTION_FRAME_COMPLETE packetId=0x%p\n", packetId));

		if (wlEvent->status == WLC_E_STATUS_SUCCESS) {
			WL_P2PO(("WLC_E_ACTION_FRAME_COMPLETE - WLC_E_STATUS_SUCCESS\n"));
			event = EVENT_ACTION_FRAME_TX_SUCCESS;
		}
		else if (wlEvent->status == WLC_E_STATUS_NO_ACK) {
			WL_P2PO(("WLC_E_ACTION_FRAME_COMPLETE - WLC_E_STATUS_NO_ACK\n"));
			event = EVENT_ACTION_FRAME_TX_FAILED;
		}
		else {
			return;
		}

		if (is_entry_valid(gas)) {
			fsm(gas, event, 0, 0, 0);
		}
	}
		break;
	case WLC_E_ACTION_FRAME_OFF_CHAN_COMPLETE:
		WL_P2PO(("WLC_E_ACTION_FRAME_OFF_CHAN_COMPLETE\n"));
		break;
	case WLC_E_ACTION_FRAME_RX:
	{
		struct ether_addr* src = &wlEvent->addr;
		wl_event_rx_frame_data_t *frameInfo =
			(wl_event_rx_frame_data_t*) data;
		uint16 channel = wf_chspec_ctlchan(ntoh16(frameInfo->channel));
		uint8 *actFrame = (uint8 *) (frameInfo + 1);
		uint32 actFrameLen = length - sizeof(wl_event_rx_frame_data_t);
		bcm_decode_t dec;
		bcm_decode_gas_t gasDecode;

		WL_P2PO(("WLC_E_ACTION_FRAME_RX on channel %d\n", channel));
		if (WL_P2PO_ON()) {
			WL_PRMAC(("action frame src MAC %s\n", bcm_ether_ntoa(src, eabuf)));
		}
		WL_PRPKT("RX action frame", actFrame, actFrameLen);

		/* decode GAS packet */
		if (bcm_decode_init(&dec, actFrameLen, actFrame) &&
			bcm_decode_gas(&dec, &gasDecode)) {
			bcm_gas_t *gas;
			int isIncoming;

			switch (gasDecode.action) {
			case GAS_REQUEST_ACTION_FRAME:
				event = EVENT_RX_REQUEST;
				isIncoming = TRUE;
				break;
			case GAS_RESPONSE_ACTION_FRAME:
				event = EVENT_RX_RESPONSE;
				isIncoming = FALSE;
				break;
			case GAS_COMEBACK_REQUEST_ACTION_FRAME:
				event = EVENT_RX_COMEBACK_REQUEST;
				isIncoming = TRUE;
				break;
			case GAS_COMEBACK_RESPONSE_ACTION_FRAME:
				event = EVENT_RX_COMEBACK_RESPONSE;
				isIncoming = FALSE;
				break;
			default:
				WL_P2PO(("invalid GAS action %d\n", gasDecode.action));
				return;
				break;
			}

			/* find instance else create it */
			gas = find_entry(src, isIncoming);
			if (gas != 0 &&
				gasDecode.action == GAS_REQUEST_ACTION_FRAME) {
					/* new incoming request - destroy previous instance */
					/* new incoming request may have same dialog token */
					bcm_gas_destroy(gas);
					gas = 0;
			}
			if (gas == 0) {

				if (gasDecode.action !=
					GAS_REQUEST_ACTION_FRAME &&
					gasDecode.action !=
					GAS_COMEBACK_REQUEST_ACTION_FRAME) {
#ifdef BCMDBG
					WL_PRMAC(("no instance found %s\n",
						bcm_ether_ntoa(src, eabuf)));
#endif // endif
					return;
				}

				if (!gIncomingRequest) {
					WL_ERROR(("incoming GAS request support disabled\n"));
					return;
				}

#ifndef BCMDRIVER
				/* router supports multiple physical interfaces */
				drv = wl_getifbyname(wlEvent->ifname);
				if (!drv) {
					WL_ERROR(("failed to find ifname %s\n",
						wlEvent->ifname));
					return;
				}
#endif /* BCMDRIVER */
				WL_P2PO(("creating incoming instance\n"));
				gas = bcm_gas_create(drv, wlEvent->bsscfgidx,
					channel, src);
				if (gas == 0) {
					WL_ERROR(("failed to create instance\n"));
					return;
				}
			}

			rx_notification(gas, &gasDecode, actFrameLen);
			fsm(gas, event, &gasDecode, 0, 0);
		}

		break;
	}
	default:
	{
	}
		break;
	}
}

/* reset all variables going into idle state */
static void idle_reset(bcm_gas_t *gas)
{
	/* free previous query response */
	if (gas->txResponse.data) {
		wl_gas_free(gas->drv, gas->txResponse.data);
		gas->txResponse.data = 0;
	}

	/* stop timers */
	tmrStop(gas->responseTimer);
	tmrStop(gas->comebackDelayTimer);
	tmrStop(gas->queryResponseTimer);

	/* reset variables */
	gas->nextState = 0;
	gas->statusCode = DOT11_SC_SUCCESS;
	gas->isPendingStatusNotification = FALSE;
	gas->retransmit = 0;
	memset(&gas->tx, 0, sizeof(gas->tx));
	gas->txResponse.fragmentId = 0;
	gas->rxResponse.fragmentId = 0;
}

static bcm_gas_event_t *prepare_notification(bcm_gas_t *gas)
{
	bcm_gas_event_t *eventp;

	eventp = wl_gas_malloc(gas->drv, sizeof(*eventp));
	if (eventp == NULL) {
		WL_ERROR(("%s: malloc failed\n", __FUNCTION__));
	} else {
		eventp->gas = gas;
		memcpy(&eventp->peer, &gas->peer, sizeof(eventp->peer));
		eventp->dialogToken = gas->dialogToken;
	}
	return eventp;
}

static void do_notification(bcm_gas_t *gas, bcm_gas_event_t *eventp)
{
	gEventCallback_t *p = get_gEventCallback();
	struct bcm_gas_wl_drv_hdl *drv = gas->drv;

	/* Note: calling p->fn() can potentially result in freeing the gas
	 * instance via bcm_gas_destroy().  Do not use the gas pointer after
	 * calling p->fn().
	 */
	if (p)
		p->fn(p->context, gas, eventp);

	/* Free the event buffer allocated in prepare_notification() */
	wl_gas_free(drv, eventp);
}

static void query_request_notification(bcm_gas_t *gas, uint16 reqLen, uint8 *req)
{
	gEventCallback_t *p = get_gEventCallback();

	if (p->fn) {
		bcm_gas_event_t *eventp = prepare_notification(gas);

		if (eventp == NULL)
			return;
		WL_PRPKT("query request notification", req, reqLen);
		eventp->type = BCM_GAS_EVENT_QUERY_REQUEST;
		eventp->queryReq.len = reqLen;
		if (eventp->queryReq.len > BCM_GAS_MAX_QUERY_REQUEST_LENGTH) {
			WL_ERROR(("truncating query request to %d\n",
				BCM_GAS_MAX_QUERY_REQUEST_LENGTH));
			eventp->queryReq.len = BCM_GAS_MAX_QUERY_REQUEST_LENGTH;
		}
		memcpy(eventp->queryReq.data, req, eventp->queryReq.len);

		/* eventp is freed inside do_notification() */
		do_notification(gas, eventp);
	}
}

static void tx_notification(bcm_gas_t *gas, int gasActionFrame)
{
	gEventCallback_t *p = get_gEventCallback();

	if (p->fn) {
		bcm_gas_event_t *eventp = prepare_notification(gas);

		if (eventp == NULL)
			return;
		WL_TRACE(("tx notification = %d\n", gasActionFrame));
		eventp->type = BCM_GAS_EVENT_TX;
		eventp->tx.gasActionFrame = gasActionFrame;
		eventp->tx.length = gas->tx.length;
		eventp->tx.fragmentId =
			gas->txResponse.fragmentId == gas->txResponse.lastFragmentId ?
			gas->txResponse.fragmentId :
			gas->txResponse.fragmentId | MORE_BIT;

		/* eventp is freed inside do_notification() */
		do_notification(gas, eventp);
	}
}

static void rx_notification(bcm_gas_t *gas, bcm_decode_gas_t *gasDecode, int length)
{
	gEventCallback_t *p = get_gEventCallback();

	if (p->fn) {
		bcm_gas_event_t *eventp = prepare_notification(gas);
		uint8 fragmentId = 0;

		if (eventp == NULL)
			return;
		WL_TRACE(("rx notification = %d\n", gasDecode->action));
		if (gasDecode->action == GAS_COMEBACK_RESPONSE_ACTION_FRAME)
			fragmentId = gasDecode->comebackResponse.fragmentId;

		eventp->type = BCM_GAS_EVENT_RX;
		eventp->rx.gasActionFrame = gasDecode->action;
		eventp->rx.length = length;
		eventp->rx.fragmentId = fragmentId;

		/* eventp is freed inside do_notification() */
		do_notification(gas, eventp);
	}
}

static void response_fragment_notification(bcm_gas_t *gas, int length, uint8 fragmentId)
{
	gEventCallback_t *p = get_gEventCallback();

	if (p->fn) {
		bcm_gas_event_t *eventp = prepare_notification(gas);

		if (eventp == NULL)
			return;
		WL_TRACE(("response fragment notification = %d\n", fragmentId));
		eventp->type = BCM_GAS_EVENT_RESPONSE_FRAGMENT;
		eventp->rspFragment.length = length;
		eventp->rspFragment.fragmentId = fragmentId;

		/* eventp is freed inside do_notification() */
		do_notification(gas, eventp);
	}
}

static void status_notification(bcm_gas_t *gas)
{
	gEventCallback_t *p = get_gEventCallback();

	if (p->fn) {
		bcm_gas_event_t *eventp = prepare_notification(gas);

		if (eventp == NULL)
			return;
		WL_TRACE(("status notification = %d\n", gas->statusCode));
		eventp->type = BCM_GAS_EVENT_STATUS;
		eventp->status.statusCode = gas->statusCode;
		gas->isPendingStatusNotification = FALSE;

		/* eventp is freed inside do_notification() */
		do_notification(gas, eventp);
	}
}

static void set_status_notification(bcm_gas_t *gas, uint16 statusCode)
{
	gas->statusCode = statusCode;
	gas->isPendingStatusNotification = TRUE;
}

static void success(bcm_gas_t *gas)
{
	WL_TRACE(("SUCCESS - returning to IDLE\n"));
	idle_reset(gas);
	change_state(gas, STATE_IDLE);
	set_status_notification(gas, DOT11_SC_SUCCESS);
}

static void fail(bcm_gas_t *gas, uint16 statusCode)
{
	WL_TRACE(("FAILED - returning to IDLE\n"));
	idle_reset(gas);
	change_state(gas, STATE_IDLE);
	set_status_notification(gas, statusCode);
}

static void delete_response_data(bcm_gas_t *gas)
{
	WL_TRACE(("deleting previous reponse data\n"));
	gas->rxResponse.isValid = FALSE;
	if (gas->rxResponse.data) {
		wl_gas_free(gas->drv, gas->rxResponse.data);
		gas->rxResponse.data = 0;
	}
	gas->rxResponse.maxLength = 0;
	gas->rxResponse.length = 0;
}

static void save_response_data(bcm_gas_t *gas, uint32 length, uint8 *data)
{
	WL_TRACE(("save response data %d bytes\n", length));

	if (gFragmentResponse) {
		if (gas->rxResponse.length != 0) {
			WL_ERROR(("previous fragment response not retrieved\n"));
			return;
		}
	}

	/* alloc memory if doesn't fit in buffer */
	if (length > gas->rxResponse.maxLength - gas->rxResponse.length) {
		/* allow some reassembly headroom to reduce alloc overhead */
		gas->rxResponse.maxLength += gFragmentResponse ?
			length : NUM_FRAGMENT_PER_REALLOC * length;
#ifdef BCM_GAS_NO_REASSEMBLY
		WL_TRACE(("malloc %d bytes\n", gas->rxResponse.maxLength));
		gas->rxResponse.data = wl_gas_malloc(gas->drv, gas->rxResponse.maxLength);
#else
		WL_TRACE(("realloc %d bytes\n", gas->rxResponse.maxLength));
		gas->rxResponse.data = realloc(gas->rxResponse.data,
			gas->rxResponse.maxLength);
#endif // endif
		if (gas->rxResponse.data == 0) {
			WL_ERROR(("realloc failed\n"));
			return;
		}
	}

	memcpy(&gas->rxResponse.data[gas->rxResponse.length], data, length);
	gas->rxResponse.length += length;
	WL_TRACE(("response data %d bytes\n", gas->rxResponse.length));
}

/* common event processing for all states */
static void default_event_processing(bcm_gas_t *gas, eventT event)
{
	switch (event) {
	case EVENT_RESET:
		WL_TRACE(("default_event_processing: EVENT_RESET\n"));
		/* reset dialog token and variables */
		gas->state = 0;
		idle_reset(gas);
		change_state(gas, STATE_IDLE);
		break;
	case EVENT_CONFIGURE:
		break;
	case EVENT_START:
		break;
	case EVENT_RX_REQUEST:
		break;
	case EVENT_RX_RESPONSE:
		break;
	case EVENT_RX_COMEBACK_REQUEST:
		break;
	case EVENT_RX_COMEBACK_RESPONSE:
		break;
	case EVENT_RX_QUERY_RESPONSE:
		break;
	case EVENT_RESPONSE_TIMEOUT:
		break;
	case EVENT_COMEBACK_DELAY_TIMEOUT:
		break;
	case EVENT_QUERY_RESPONSE_TIMEOUT:
		break;
	case EVENT_ACTION_FRAME_TX_SUCCESS:
		break;
	case EVENT_ACTION_FRAME_TX_FAILED:
		break;
	default:
		WL_ERROR(("invalid event %d\n", event));
		break;
	}
}

static void tx_action_frame(bcm_gas_t *gas, uint32 channel, uint32 responseTimeout,
	struct ether_addr *bssid, struct ether_addr *dst, int gasActionFrame,
	int length, uint8 *buffer)
{
	/* reset retransmit count */
	gas->retransmit = 0;

	/* save tx parameters for retransmit */
	gas->tx.channel = channel;
	gas->tx.responseTimeout = responseTimeout;
	memcpy(&gas->tx.bssid, bssid, sizeof(gas->tx.bssid));
	memcpy(&gas->tx.dst, dst, sizeof(gas->tx.dst));
	gas->tx.gasActionFrame = gasActionFrame;

	WL_PRPKT("TX action frame",	gas->tx.data, gas->tx.length);
	WL_NONE(("%s: bssid=%02x:%02x:%02x:%02x:%02x:%02x"
		" dst=%02x:%02x:%02x:%02x:%02x:%02x\n", __FUNCTION__,
		gas->tx.bssid.octet[0], gas->tx.bssid.octet[1], gas->tx.bssid.octet[2],
		gas->tx.bssid.octet[3], gas->tx.bssid.octet[4], gas->tx.bssid.octet[5],
		gas->tx.dst.octet[0], gas->tx.dst.octet[1], gas->tx.dst.octet[2],
		gas->tx.dst.octet[3], gas->tx.dst.octet[4], gas->tx.dst.octet[5]));

	if (wl_actframe(gas->drv, gas->bsscfgIndex,
		BCMGAS_PKT_ID(gas), gas->tx.channel, gas->tx.responseTimeout,
		&gas->tx.bssid, &gas->tx.dst, length, buffer) != 0) {
		WL_ERROR(("wl_actframe failed\n"));
	}
	tx_notification(gas, gas->tx.gasActionFrame);
	if (gas->tx.responseTimeout > 0) {
		tmrStart(gas->responseTimer, gas->tx.responseTimeout, FALSE);
	}
}

static void retransmit(bcm_gas_t *gas)
{
	if (gas->retransmit < gas->maxRetransmit) {
		WL_PRPKT("TX action frame (retransmit)", gas->tx.data, gas->tx.length);

		if (wl_actframe(gas->drv, gas->bsscfgIndex,
			BCMGAS_PKT_ID(gas), gas->tx.channel, gas->tx.responseTimeout,
			&gas->tx.bssid, &gas->tx.dst,
			gas->tx.length, gas->tx.data) != 0) {
			WL_ERROR(("wl_actframe failed\n"));
		}
		gas->retransmit++;
		tx_notification(gas, gas->tx.gasActionFrame);
		if (gas->tx.responseTimeout > 0) {
			tmrStart(gas->responseTimer, gas->tx.responseTimeout, FALSE);
		}
	}
	else {
		WL_P2PO(("max retransmits %d\n", gas->maxRetransmit));
		/* fail occurs on response timeout or immediately if no response timeout */
		if (gas->tx.responseTimeout == 0) {
			fail(gas, DOT11_SC_TRANSMIT_FAILURE);
		}
	}
}

static void encode_advertisement_protocol(bcm_encode_t *pkt,
	uint8 qResponseLimit, uint8 advertisementProtocol)
{
	uint8 buffer[32];
	bcm_encode_t ap;

	bcm_encode_init(&ap, sizeof(buffer), buffer);
	bcm_encode_ie_advertisement_protocol_tuple(&ap,
		ADVP_PAME_BI_DEPENDENT, qResponseLimit, advertisementProtocol);
	bcm_encode_ie_advertisement_protocol_from_tuple(pkt,
		bcm_encode_length(&ap), bcm_encode_buf(&ap));
}

static void tx_gas_request(bcm_gas_t *gas, uint32 responseTimeout,
	uint8 dialogToken, uint8 advertisementProtocol,	uint16 reqLen, uint8 *req)
{
	uint8 buffer[32];
	bcm_encode_t apie;
	bcm_encode_t enc;

	/* encode advertisement protocol IE */
	bcm_encode_init(&apie, sizeof(buffer), buffer);
	encode_advertisement_protocol(&apie, ADVP_QRL_REQUEST,
		advertisementProtocol);

	/* encode GAS request */
	bcm_encode_init(&enc, sizeof(gas->tx.data), gas->tx.data);
	gas->tx.length = bcm_encode_gas_request(&enc, dialogToken,
		bcm_encode_length(&apie), bcm_encode_buf(&apie), reqLen, req);

	tx_action_frame(gas, gas->channel, responseTimeout, &gas->peer,
		&gas->peer, GAS_REQUEST_ACTION_FRAME, gas->tx.length, gas->tx.data);
}

static void tx_gas_response(bcm_gas_t *gas, uint32 responseTimeout,
	uint8 dialogToken, uint16 statusCode, uint16 comebackDelay,
	uint8 advertisementProtocol, uint16 rspLen, uint8 *rsp)
{
	uint8 buffer[32];
	bcm_encode_t apie;
	bcm_encode_t enc;

	/* encode advertisement protocol IE */
	bcm_encode_init(&apie, sizeof(buffer), buffer);
	encode_advertisement_protocol(&apie, ADVP_QRL_RESPONSE,
		advertisementProtocol);

	/* encode GAS response */
	bcm_encode_init(&enc, sizeof(gas->tx.data), gas->tx.data);
	gas->tx.length = bcm_encode_gas_response(&enc, dialogToken, statusCode, comebackDelay,
		bcm_encode_length(&apie), bcm_encode_buf(&apie), rspLen, rsp);
	WL_NONE(("%s: sz=%u rspLen=%u rsp=%p tx.len=%u\n", __FUNCTION__,
		sizeof(gas->tx.data), rspLen, rsp, gas->tx.length));

	tx_action_frame(gas, gas->channel, responseTimeout,
		&gas->mac, &gas->peer, GAS_RESPONSE_ACTION_FRAME,
		gas->tx.length, gas->tx.data);
}

static void tx_gas_comeback_request(bcm_gas_t *gas, uint32 responseTimeout,
	uint8 dialogToken)
{
	bcm_encode_t enc;

	/* encode GAS comeback request */
	bcm_encode_init(&enc, sizeof(gas->tx.data), gas->tx.data);
	gas->tx.length = bcm_encode_gas_comeback_request(&enc, dialogToken);

	tx_action_frame(gas, gas->channel, responseTimeout,
		&gas->peer, &gas->peer,	GAS_COMEBACK_REQUEST_ACTION_FRAME,
		gas->tx.length, gas->tx.data);
}

static void tx_gas_comeback_response(bcm_gas_t *gas, uint32 responseTimeout,
	uint8 dialogToken, uint16 statusCode, uint8 fragmentId,
	uint8 advertisementProtocol, uint16 rspLen, uint8 *rsp)
{
	uint8 buffer[32];
	bcm_encode_t apie;
	bcm_encode_t enc;

	/* encode advertisement protocol IE */
	bcm_encode_init(&apie, sizeof(buffer), buffer);
	encode_advertisement_protocol(&apie, ADVP_QRL_RESPONSE,
		advertisementProtocol);

	/* encode GAS comeback response */
	bcm_encode_init(&enc, sizeof(gas->tx.data), gas->tx.data);
	gas->tx.length = bcm_encode_gas_comeback_response(&enc, dialogToken,
		statusCode, fragmentId, 0,
		bcm_encode_length(&apie), bcm_encode_buf(&apie), rspLen, rsp);

	tx_action_frame(gas, gas->channel, (fragmentId & MORE_BIT) ?
		responseTimeout : 0, &gas->mac, &gas->peer,
		GAS_COMEBACK_RESPONSE_ACTION_FRAME, gas->tx.length, gas->tx.data);
}

static void state_idle(bcm_gas_t *gas, eventT event,
	bcm_decode_gas_t *gasDecode, int dataLen, uint8 *data)
{
	(void)dataLen;
	(void)data;

	switch (event) {
	case EVENT_CONFIGURE:
		break;
	case EVENT_START:
		delete_response_data(gas);
		/* assign dialog token */
		gas->dialogToken = gNextDialogToken++;
		tx_gas_request(gas, gas->responseTimeout,
			gas->dialogToken, gas->advertisementProtocol,
			gas->txRequest.length, gas->txRequest.data);
		change_state(gas, STATE_TX_REQUEST);
		break;
	case EVENT_RX_REQUEST:
	{
		/* save dialog token from request */
		gas->dialogToken = gasDecode->dialogToken;

		if (!is_valid_advertisement_protocol(&gasDecode->request.apie)) {
			/* transmit failed GAS response */
			gas->statusCode = DOT11_SC_ADV_PROTO_NOT_SUPPORTED;
			tx_gas_response(gas, 0, gas->dialogToken, gas->statusCode,
				0, gasDecode->request.apie.protocolId, 0, 0);
			change_state(gas, STATE_ERROR);
		}
		else {
			/* send request to server */
			query_request_notification(gas,
				gasDecode->request.reqLen, gasDecode->request.req);
#ifndef BCM_GAS_NO_DISPATCHER
			tmrStart(gas->queryResponseTimer, QUERY_RESPONSE_TIMEOUT, FALSE);
#endif	/* BCM_GAS_NO_DISPATCHER */

			if (get_gas_pause_for_server_response(gas->drv)) {
#ifndef BCM_GAS_NO_DISPATCHER
				change_state(gas, STATE_WAIT_FOR_QUERY_RESPONSE);
#else
				state_wait_query_response_process_query_response(gas);
#endif	/* BCM_GAS_NO_DISPATCHER */
			}
			else {
				int comebackDelay = get_comeback_delay_response_unpause(gas->drv);
				tx_gas_response(gas, comebackDelay + gas->responseTimeout,
					gas->dialogToken, DOT11_SC_SUCCESS,
					comebackDelay, gas->advertisementProtocol, 0, 0);
				change_state(gas, STATE_TX_RESPONSE_AND_QUERY_REQUEST);
			}
		}
	}
		break;
	case EVENT_RX_RESPONSE:
		break;
	case EVENT_RX_COMEBACK_REQUEST:
		/* save dialog token from request */
		gas->dialogToken = gasDecode->dialogToken;
		/* transmit failed GAS comeback response */
		gas->statusCode = DOT11_SC_NO_OUTSTAND_REQ;
		tx_gas_comeback_response(gas, 0, gas->dialogToken, gas->statusCode,
			0, gas->advertisementProtocol, 0, 0);
		change_state(gas, STATE_ERROR);
		break;
	case EVENT_RX_COMEBACK_RESPONSE:
		break;
	case EVENT_RX_QUERY_RESPONSE:
		break;
	case EVENT_RESPONSE_TIMEOUT:
		break;
	case EVENT_COMEBACK_DELAY_TIMEOUT:
		break;
	case EVENT_QUERY_RESPONSE_TIMEOUT:
		break;
	case EVENT_ACTION_FRAME_TX_SUCCESS:
		break;
	case EVENT_ACTION_FRAME_TX_FAILED:
		break;
	default:
		default_event_processing(gas, event);
		break;
	}
}

static void state_tx_request(bcm_gas_t *gas, eventT event,
	bcm_decode_gas_t *gasDecode, int dataLen, uint8 *data)
{
	(void)dataLen;
	(void)data;

	switch (event) {
	case EVENT_CONFIGURE:
		break;
	case EVENT_START:
		break;
	case EVENT_RX_REQUEST:
		break;
	case EVENT_RX_RESPONSE:
		tmrStop(gas->responseTimer);
		if (gasDecode->dialogToken != gas->dialogToken) {
			WL_P2PO(("dialog token mismatch %d != %d\n",
				gasDecode->dialogToken, gas->dialogToken));
			fail(gas, DOT11_SC_NO_OUTSTAND_REQ);
		}
		else {
			if (gasDecode->response.statusCode != DOT11_SC_SUCCESS) {
				fail(gas, gasDecode->response.statusCode);
			}
			else {
				if (gasDecode->response.comebackDelayTU == 0) {
					/* data in response */
					save_response_data(gas,
						gasDecode->response.rspLen,
						gasDecode->response.rsp);
					gas->rxResponse.isValid = TRUE;
					success(gas);
				}
				else if (gasDecode->response.comebackDelayTU <=
					gas->maxComebackDelayTU) {
					/* data fragmented */
					tmrStart(gas->comebackDelayTimer,
						gComebackDelayOverride == 0 ?
						TU_TO_MS(gasDecode->response.comebackDelayTU) :
						gComebackDelayOverride, FALSE);
					change_state(gas, STATE_WAIT_COMEBACK_DELAY);
				}
				else {
					/* fail due to comeback delay exceeded */
					fail(gas, DOT11_SC_FAILURE);
				}
			}
		}
		break;
	case EVENT_RX_COMEBACK_REQUEST:
		break;
	case EVENT_RX_COMEBACK_RESPONSE:
		break;
	case EVENT_RX_QUERY_RESPONSE:
		break;
	case EVENT_RESPONSE_TIMEOUT:
		fail(gas, DOT11_SC_TIMEOUT);
		break;
	case EVENT_COMEBACK_DELAY_TIMEOUT:
		break;
	case EVENT_QUERY_RESPONSE_TIMEOUT:
		break;
	case EVENT_ACTION_FRAME_TX_SUCCESS:
		break;
	case EVENT_ACTION_FRAME_TX_FAILED:
		retransmit(gas);
		break;
	default:
		default_event_processing(gas, event);
		break;
	}
}

static void rx_comeback_request(bcm_gas_t *gas, bcm_decode_gas_t *gasDecode)
{
	tmrStop(gas->responseTimer);
	if (gasDecode->dialogToken != gas->dialogToken) {
		WL_P2PO(("dialog token mismatch %d != %d\n",
			gasDecode->dialogToken, gas->dialogToken));
		fail(gas, DOT11_SC_NO_OUTSTAND_REQ);
	}
	else {
		uint32 sent;
		int more;
		uint16 statusCode = 0;
		uint8 fragmentId;
		uint16 rspLen;
		uint8 *rsp;

		/* determine query data size */
		sent = gas->txResponse.fragmentId * MAX_FRAGMENT_SIZE;
		if (gas->txResponse.length - sent > MAX_FRAGMENT_SIZE) {
			more = TRUE;
			rspLen = MAX_FRAGMENT_SIZE;
		}
		else {
			more = FALSE;
			rspLen = gas->txResponse.length - sent;
		}
		rsp = &gas->txResponse.data[sent];

		if (more)
			fragmentId = gas->txResponse.fragmentId | MORE_BIT;
		else
			fragmentId = gas->txResponse.fragmentId & ~MORE_BIT;

		tx_gas_comeback_response(gas, gas->responseTimeout,
			gas->dialogToken,	statusCode, fragmentId,
			gas->advertisementProtocol,	rspLen, rsp);
		change_state(gas, STATE_TX_COMEBACK_RESPONSE);
	}
}

static void state_tx_response(bcm_gas_t *gas, eventT event,
	bcm_decode_gas_t *gasDecode, int dataLen, uint8 *data)
{
	(void)dataLen;
	(void)data;

	switch (event) {
	case EVENT_CONFIGURE:
		break;
	case EVENT_START:
		break;
	case EVENT_RX_REQUEST:
		break;
	case EVENT_RX_RESPONSE:
		break;
	case EVENT_RX_COMEBACK_REQUEST:
		rx_comeback_request(gas, gasDecode);
		break;
	case EVENT_RX_COMEBACK_RESPONSE:
		break;
	case EVENT_RX_QUERY_RESPONSE:
		break;
	case EVENT_RESPONSE_TIMEOUT:
		fail(gas, DOT11_SC_TIMEOUT);
		break;
	case EVENT_COMEBACK_DELAY_TIMEOUT:
		break;
	case EVENT_QUERY_RESPONSE_TIMEOUT:
		break;
	case EVENT_ACTION_FRAME_TX_SUCCESS:
		if (gas->txResponse.lastFragmentId == 0) {
			tmrStop(gas->responseTimer);
			success(gas);
		}
		break;
	case EVENT_ACTION_FRAME_TX_FAILED:
		retransmit(gas);
		break;
	default:
		default_event_processing(gas, event);
		break;
	}
}

static void rx_comeback_response(bcm_gas_t *gas, bcm_decode_gas_t *gasDecode)
{
	tmrStop(gas->responseTimer);
	if (gasDecode->dialogToken != gas->dialogToken) {
		WL_P2PO(("dialog token mismatch %d != %d\n",
			gasDecode->dialogToken, gas->dialogToken));
		fail(gas, DOT11_SC_NO_OUTSTAND_REQ);
	}
	else if (gasDecode->comebackResponse.statusCode != DOT11_SC_SUCCESS) {
		fail(gas, gasDecode->comebackResponse.statusCode);
	}
	else {
		int isMore = gasDecode->comebackResponse.fragmentId & MORE_BIT ? TRUE : FALSE;
		uint8 fragmentId = gasDecode->comebackResponse.fragmentId & ~MORE_BIT;

		if (isMore) {
			if (gas->rxResponse.fragmentId == fragmentId) {

				/* save response data */
				save_response_data(gas,
					gasDecode->comebackResponse.rspLen,
					gasDecode->comebackResponse.rsp);

				if (gFragmentResponse) {
					response_fragment_notification(gas,
					gasDecode->comebackResponse.rspLen,
					gasDecode->comebackResponse.fragmentId);
				}

				if (gasDecode->comebackResponse.comebackDelayTU == 0) {
					tx_gas_comeback_request(
						gas, gas->responseTimeout,
						gas->dialogToken);
					change_state(gas, STATE_TX_COMEBACK_REQUEST);
				}
				else if (gasDecode->comebackResponse.comebackDelayTU <=
					gas->maxComebackDelayTU) {
					/* wait the comeback delay */
					tmrStart(gas->comebackDelayTimer,
						gComebackDelayOverride == 0 ?
						TU_TO_MS(
						gasDecode->comebackResponse.comebackDelayTU) :
						gComebackDelayOverride,
						FALSE);
					change_state(gas, STATE_WAIT_COMEBACK_DELAY);
				}
				else {
					/* fail due to comeback delay exceeded */
					fail(gas, DOT11_SC_FAILURE);
				}

				/* next expected rx fragment id */
				gas->rxResponse.fragmentId++;
			}
			else if (gas->rxResponse.fragmentId < fragmentId) {
				WL_P2PO(("expecting fragment id %d, receive %d\n",
					gas->rxResponse.fragmentId, fragmentId));
				fail(gas, DOT11_SC_NO_OUTSTAND_REQ);
			}
			else {
				/* already received fragment ID */
				WL_P2PO(("expecting fragment id %d, receive %d\n",
					gas->rxResponse.fragmentId, fragmentId));

				tx_gas_comeback_request(
					gas, gas->responseTimeout,
					gas->dialogToken);
				change_state(gas, STATE_TX_COMEBACK_REQUEST);
			}
		}
		else {
			/* save last response data */
			save_response_data(gas,
				gasDecode->comebackResponse.rspLen,
				gasDecode->comebackResponse.rsp);
			gas->rxResponse.isValid = TRUE;
			success(gas);
		}
	}
}

static void state_tx_comeback_request(bcm_gas_t *gas, eventT event,
	bcm_decode_gas_t *gasDecode, int dataLen, uint8 *data)
{
	(void)dataLen;
	(void)data;

	switch (event) {
	case EVENT_CONFIGURE:
		break;
	case EVENT_START:
		break;
	case EVENT_RX_REQUEST:
		break;
	case EVENT_RX_RESPONSE:
		break;
	case EVENT_RX_COMEBACK_REQUEST:
		break;
	case EVENT_RX_COMEBACK_RESPONSE:
		rx_comeback_response(gas, gasDecode);
		break;
	case EVENT_RX_QUERY_RESPONSE:
		break;
	case EVENT_RESPONSE_TIMEOUT:
		fail(gas, DOT11_SC_TIMEOUT);
		break;
	case EVENT_COMEBACK_DELAY_TIMEOUT:
		break;
	case EVENT_QUERY_RESPONSE_TIMEOUT:
		break;
	case EVENT_ACTION_FRAME_TX_SUCCESS:
		break;
	case EVENT_ACTION_FRAME_TX_FAILED:
		retransmit(gas);
		break;
	default:
		default_event_processing(gas, event);
		break;
	}
}

static void state_tx_comeback_response(bcm_gas_t *gas, eventT event,
	bcm_decode_gas_t *gasDecode, int dataLen, uint8 *data)
{
	(void)dataLen;
	(void)data;

	switch (event) {
	case EVENT_CONFIGURE:
		break;
	case EVENT_START:
		break;
	case EVENT_RX_REQUEST:
		break;
	case EVENT_RX_RESPONSE:
		break;
	case EVENT_RX_COMEBACK_REQUEST:
		rx_comeback_request(gas, gasDecode);
		break;
	case EVENT_RX_COMEBACK_RESPONSE:
		break;
	case EVENT_RX_QUERY_RESPONSE:
		break;
	case EVENT_RESPONSE_TIMEOUT:
		fail(gas, DOT11_SC_TIMEOUT);
		break;
	case EVENT_COMEBACK_DELAY_TIMEOUT:
		break;
	case EVENT_QUERY_RESPONSE_TIMEOUT:
		break;
	case EVENT_ACTION_FRAME_TX_SUCCESS:
		WL_P2PO(("fragment ID=%d last=%d\n",
			gas->txResponse.fragmentId, gas->txResponse.lastFragmentId));
		if (gas->txResponse.fragmentId == gas->txResponse.lastFragmentId) {
			tmrStop(gas->responseTimer);
			success(gas);
		}
		else {
			gas->txResponse.fragmentId++;
		}
		break;
	case EVENT_ACTION_FRAME_TX_FAILED:
		retransmit(gas);
		break;
	default:
		default_event_processing(gas, event);
		break;
	}
}

static void rx_query_response(bcm_gas_t *gas, int dataLen, uint8 *data)
{
	/* free previous query response */
	if (gas->txResponse.data) {
		wl_gas_free(gas->drv, gas->txResponse.data);
		gas->txResponse.data = 0;
	}

	/* copy query response */
	gas->txResponse.fragmentId = 0;
	gas->txResponse.length = dataLen;
	gas->txResponse.lastFragmentId =
		gas->txResponse.length / MAX_FRAGMENT_SIZE +
		(gas->txResponse.length % MAX_FRAGMENT_SIZE == 0 ? 0 : 1) - 1;
	gas->txResponse.data = wl_gas_malloc(gas->drv, gas->txResponse.length);
	WL_NONE(("%s: len=%u txrsp=%p data=%p\n", __FUNCTION__,
		gas->txResponse.length, gas->txResponse.data, data));
	if (gas->txResponse.data == 0)
		gas->txResponse.length = 0;
	else
		memcpy(gas->txResponse.data, data, gas->txResponse.length);
}

static void state_tx_response_and_query_request(bcm_gas_t *gas, eventT event,
	bcm_decode_gas_t *gasDecode, int dataLen, uint8 *data)
{
	(void)dataLen;
	(void)data;

	switch (event) {
	case EVENT_CONFIGURE:
		break;
	case EVENT_START:
		break;
	case EVENT_RX_REQUEST:
		break;
	case EVENT_RX_RESPONSE:
		break;
	case EVENT_RX_COMEBACK_REQUEST:
		tmrStop(gas->responseTimer);
		if (gas->txResponse.length > 0) {
			rx_comeback_request(gas, gasDecode);
		}
		else {
			/* transmit GAS comeback response with no server response status */
			gas->statusCode = DOT11_SC_RSP_NOT_RX_FROM_SERVER;
			tx_gas_comeback_response(gas, 0, gas->dialogToken, gas->statusCode,
				0, gas->advertisementProtocol, 0, 0);
		}
		break;
	case EVENT_RX_COMEBACK_RESPONSE:
		break;
	case EVENT_RX_QUERY_RESPONSE:
		tmrStop(gas->queryResponseTimer);
		rx_query_response(gas, dataLen, data);
		break;
	case EVENT_RESPONSE_TIMEOUT:
		fail(gas, DOT11_SC_TIMEOUT);
		break;
	case EVENT_COMEBACK_DELAY_TIMEOUT:
		break;
	case EVENT_QUERY_RESPONSE_TIMEOUT:
		tmrStop(gas->responseTimer);
		/* transmit GAS comeback response with timeout status */
		gas->statusCode = DOT11_SC_RSP_NOT_RX_FROM_SERVER;
		tx_gas_comeback_response(gas, 0, gas->dialogToken, gas->statusCode,
			0, gas->advertisementProtocol, 0, 0);
		change_state(gas, STATE_ERROR);
		break;
	case EVENT_ACTION_FRAME_TX_SUCCESS:
		/* check if query response is available */
		if (gas->txResponse.length > 0) {
			change_state(gas, STATE_TX_RESPONSE);
		}
		break;
	case EVENT_ACTION_FRAME_TX_FAILED:
		retransmit(gas);
		break;
	default:
		default_event_processing(gas, event);
		break;
	}
}

static void state_wait_query_response_process_query_response(bcm_gas_t *gas)
{
	uint16 statusCode = DOT11_SC_SUCCESS;
	uint16 comebackDelay;
	uint16 rspLen;
	uint8 *rsp;

	if (gas->isNoResponse) {
		idle_reset(gas);
		change_state(gas, STATE_IDLE);
		return;
	}
	/* determine query data size */
	if (gas->txResponse.length <= MAX_FRAGMENT_SIZE) {
		comebackDelay = 0;
		rspLen = gas->txResponse.length;
		rsp = gas->txResponse.data;
	}
	else {
		comebackDelay = gComebackDelayResponsePause;
		rspLen = 0;
		rsp = 0;
		gas->txResponse.fragmentId = 0;
	}

	tx_gas_response(gas, comebackDelay == 0 ? 0 : comebackDelay + gas->responseTimeout,
		gas->dialogToken, statusCode, comebackDelay, gas->advertisementProtocol,
		rspLen, rsp);
	change_state(gas, STATE_TX_RESPONSE);
}

static void state_wait_query_response(bcm_gas_t *gas, eventT event,
	bcm_decode_gas_t *gasDecode, int dataLen, uint8 *data)
{
	(void)gasDecode;
	(void)dataLen;
	(void)data;

	switch (event) {
	case EVENT_CONFIGURE:
		break;
	case EVENT_START:
		break;
	case EVENT_RX_REQUEST:
		break;
	case EVENT_RX_RESPONSE:
		break;
	case EVENT_RX_COMEBACK_REQUEST:
		break;
	case EVENT_RX_COMEBACK_RESPONSE:
		break;
	case EVENT_RX_QUERY_RESPONSE:
		tmrStop(gas->queryResponseTimer);
		rx_query_response(gas, dataLen, data);
		state_wait_query_response_process_query_response(gas);
		break;
	case EVENT_RESPONSE_TIMEOUT:
		break;
	case EVENT_COMEBACK_DELAY_TIMEOUT:
		break;
	case EVENT_QUERY_RESPONSE_TIMEOUT:
		/* transmit GAS response with timeout status */
		gas->statusCode = DOT11_SC_RSP_NOT_RX_FROM_SERVER;
		tx_gas_response(gas, 0,	gas->dialogToken, gas->statusCode, 0,
			gas->advertisementProtocol, 0, 0);
		change_state(gas, STATE_ERROR);
		break;
	case EVENT_ACTION_FRAME_TX_SUCCESS:
		break;
	case EVENT_ACTION_FRAME_TX_FAILED:
		break;
	default:
		default_event_processing(gas, event);
		break;
	}
}

static void state_wait_comeback_delay(bcm_gas_t *gas, eventT event,
	bcm_decode_gas_t *gasDecode, int dataLen, uint8 *data)
{
	(void)gasDecode;
	(void)dataLen;
	(void)data;

	switch (event) {
	case EVENT_CONFIGURE:
		break;
	case EVENT_START:
		break;
	case EVENT_RX_REQUEST:
		break;
	case EVENT_RX_RESPONSE:
		break;
	case EVENT_RX_COMEBACK_REQUEST:
		break;
	case EVENT_RX_COMEBACK_RESPONSE:
		/* due to comeback request retransmits - possible to receive event in this state */
		tmrStop(gas->comebackDelayTimer);
		rx_comeback_response(gas, gasDecode);
		break;
	case EVENT_RX_QUERY_RESPONSE:
		break;
	case EVENT_RESPONSE_TIMEOUT:
		break;
	case EVENT_COMEBACK_DELAY_TIMEOUT:
		tx_gas_comeback_request(gas,
			gas->responseTimeout, gas->dialogToken);
		change_state(gas, STATE_TX_COMEBACK_REQUEST);
		break;
	case EVENT_QUERY_RESPONSE_TIMEOUT:
		break;
	case EVENT_ACTION_FRAME_TX_SUCCESS:
		break;
	case EVENT_ACTION_FRAME_TX_FAILED:
		break;
	default:
		default_event_processing(gas, event);
		break;
	}
}

static void state_error(bcm_gas_t *gas, eventT event,
	bcm_decode_gas_t *gasDecode, int dataLen, uint8 *data)
{
	(void)gasDecode;
	(void)dataLen;
	(void)data;

	switch (event) {
	case EVENT_CONFIGURE:
		break;
	case EVENT_START:
		break;
	case EVENT_RX_REQUEST:
		break;
	case EVENT_RX_RESPONSE:
		break;
	case EVENT_RX_COMEBACK_REQUEST:
		break;
	case EVENT_RX_COMEBACK_RESPONSE:
		break;
	case EVENT_RX_QUERY_RESPONSE:
		break;
	case EVENT_RESPONSE_TIMEOUT:
		/* report initial error instead of timeout */
		fail(gas, gas->statusCode);
		break;
	case EVENT_COMEBACK_DELAY_TIMEOUT:
		break;
	case EVENT_QUERY_RESPONSE_TIMEOUT:
		break;
	case EVENT_ACTION_FRAME_TX_SUCCESS:
		fail(gas, gas->statusCode);
		break;
	case EVENT_ACTION_FRAME_TX_FAILED:
		retransmit(gas);
		break;
	default:
		default_event_processing(gas, event);
		break;
	}
}

typedef void (*state_func_t)(bcm_gas_t *gas, eventT event,
	bcm_decode_gas_t *gasDecode, int dataLen, uint8 *data);

static state_func_t table[STATE_LAST] = {
	state_idle,
	state_tx_request,
	state_tx_response,
	state_tx_comeback_request,
	state_tx_comeback_response,
	state_tx_response_and_query_request,
	state_wait_query_response,
	state_wait_comeback_delay,
	state_error
};

/* GAS protocol finite state machine */
static void fsm(bcm_gas_t *gas, eventT event,
	bcm_decode_gas_t *gasDecode, int dataLen, uint8 *data)
{
	WL_P2PO(("--------------------------------------------------------\n"));

	if (gas->state < STATE_LAST && event < EVENT_LAST) {
		WL_P2PO(("current state=%s event=%s\n",
			state_str[gas->state], event_str[event]));

		/* process event based on current state */
		(table[gas->state])(gas, event, gasDecode, dataLen, data);

		/* transition to next state */
		if (gas->state != gas->nextState) {
			int isIncoming = gas->isIncoming;

			WL_P2PO(("state change %s -> %s\n",
				state_str[gas->state], state_str[gas->nextState]));
			gas->state = gas->nextState;

			if (gas->isPendingStatusNotification && gas->state == STATE_IDLE) {
				/* gas instance may be destroyed during
				 * the status notification callback
				 */
				status_notification(gas);
			}

			/* destroy incoming on return to idle */
			if (isIncoming && gas->state == STATE_IDLE) {
				WL_P2PO(("destroying incoming instance\n"));
				bcm_gas_destroy(gas);
			}
		}
	}
	else {
		WL_P2PO(("invalid state=%d event=%d\n", gas->state, event));
	}

	WL_P2PO(("--------------------------------------------------------\n"));
}
/* Inform GAS framework, Dont send Response in case there is no response from Server */
int bcm_gas_no_query_response(bcm_gas_t *gas)
{
	if (!gas) {
		return 0;
	}
	gas->isNoResponse = TRUE;
	return 1;
}

struct ether_addr*
bcm_gas_get_mac_addr(bcm_gas_t *gas)
{
	return (&gas->mac);
}
/* update gas instance as incoming, by default will be False */
void
bcm_gas_update_gas_incoming_info(bcm_gas_t* gas, bool isIncoming)
{
	gas->isIncoming = isIncoming;
}
