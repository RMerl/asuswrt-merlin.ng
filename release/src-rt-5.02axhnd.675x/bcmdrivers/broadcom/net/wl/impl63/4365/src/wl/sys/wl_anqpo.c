/*
 * ANQP Offload
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
 * $Id: wl_anqpo.c 733849 2017-11-30 07:25:00Z $
 *
 */

/**
 * @file
 * @brief
 * XXX ANQP Offload is a feature requested by Olympic which allows the dongle to perform ANQP
 * queries (using 802.11u GAS) to devices and have the ANQP response returned to the host using an
 * event notification. Any query using ANQP such as hotspot and service discovery may be sent using
 * the offload.
 *
 * Twiki: [OffloadsPhase2]
 */

#include <wlc_cfg.h>
#include <typedefs.h>
#include <osl.h>
#include <wlioctl.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_channel.h>
#include <wlc_pub.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_scan.h>
#include <wl_gas.h>
#include <proto/bcmevent.h>
#include <bcm_gas.h>
#include <bcm_decode_ie.h>
#include <wl_gas.h>
#include <wl_anqpo.h>

/** ignore mode */
enum {
	IGNORE_SSID,
	IGNORE_BSSID
} ignore_e;

/** ANQP peer */
typedef struct peer_s {
	uint16 channel;				/* peer channel */
	struct ether_addr addr;		/* peer destination address */
	wlc_ssid_t ssid;			/* peer SSID */
	bcm_gas_t	*gas;			/* gas instance */
	uint16 num_retries;			/* number of retries */
} peer_t;

/** linked list of ssid */
typedef struct ssid_llist {
	struct ssid_llist *next;
	wlc_ssid_t ssid;
} ssid_llist_t;

/** linked list of ether_addr */
typedef struct ether_llist {
	struct ether_llist *next;
	struct ether_addr addr;
} ether_llist_t;

/** anqpo private info structure */
struct wl_anqpo_info {
	wlc_info_t *wlc;			/* pointer back to wlc structure */

	bool is_start_query;		/* start query iovar enabled */

	uint16 max_retransmit;		/* -1 use default, max retransmit on no ACK from peer */
	uint16 response_timeout;	/* -1 use default, msec to wait for resp after tx packet */
	uint16 max_comeback_delay;	/* -1 use default, max comeback delay in resp else fail */
	uint16 max_retries;			/* -1 use default, max retries on failure */
	uint16 query_len;			/* ANQP query length */
	uint8 *query_data;			/* ANQP query data */

	uint16 peer_count;			/* number of ANQP peers */
	peer_t *peer;				/* ANQP peer list */
	uint16 peer_len;			/* size of peer list */

	uint16 active_peer_count;	/* number of active ANQP peers */
	uint16 started_peer_count;	/* number of completed ANQP peers */

	uint16 max_auto_hotspot;	/* automatic ANQP to scanned hotspot APs */
	uint8 ignore_mode;			/* ignore duplicate SSIDs or BSSIDs */

	ssid_llist_t *ignore_ssid_llist;	/* linked list of ignored SSIDs */
	ether_llist_t *ignore_bssid_llist;	/* linked list of ignored BSSIDs */

	wl_gas_info_t		*gasi;		/* gas handler */
};

/* start query iovar enabled */
#define is_start_query_enabled(anqpo)	(anqpo->is_start_query)

/* auto hotspot enabled */
#define is_auto_hotspot_enabled(anqpo)	(anqpo->max_auto_hotspot > 0)

#define PARAM_NOT_CONFIGURED		(0xffff)	/* parameter not config - use default */
#define DEFAULT_MAX_RETRANSMIT		0		/* max retransmit on no ACK from peer */
#define DEFAULT_RESPONSE_TIMEOUT	200		/* msec to wait for resp after tx packet */
#define DEFAULT_MAX_COMEBACK_DELAY	(0xffff)	/* max comeback delay in resp else fail */
#define DEFAULT_MAX_RETRIES			0		/* max retries on failure */

/* number of active ANQP peers for start query */
#define MAX_ACTIVE_PEERS_START_QUERY	1

/* number of active ANQP peers for auto hotspot */
#define MAX_ACTIVE_PEERS_AUTO_HOTSPOT	4

/* wlc_pub_t struct access macros */
#define WLCUNIT(x)	((x)->wlc->pub->unit)
#define WLCOSH(x)	((x)->wlc->osh)

enum {
	IOV_ANQPO_SET,
	IOV_ANQPO_STOP_QUERY,
	IOV_ANQPO_START_QUERY,
	IOV_ANQPO_AUTO_HOTSPOT,
	IOV_ANQPO_IGNORE_MODE,
	IOV_ANQPO_IGNORE_SSID_LIST,
	IOV_ANQPO_IGNORE_BSSID_LIST
};

static const bcm_iovar_t anqpo_iovars[] = {
	{"anqpo_set", IOV_ANQPO_SET,
	0, IOVT_BUFFER, OFFSETOF(wl_anqpo_set_t, query_data)},
	{"anqpo_stop_query", IOV_ANQPO_STOP_QUERY,
	0, IOVT_BUFFER, 0},
	{"anqpo_start_query", IOV_ANQPO_START_QUERY,
	0, IOVT_BUFFER, sizeof(wl_anqpo_peer_list_t)},
	{"anqpo_auto_hotspot", IOV_ANQPO_AUTO_HOTSPOT,
	0, IOVT_UINT16, 0},
	{"anqpo_ignore_mode", IOV_ANQPO_IGNORE_MODE,
	0, IOVT_UINT16, 0},
	{"anqpo_ignore_ssid_list", IOV_ANQPO_IGNORE_SSID_LIST,
	0, IOVT_BUFFER, OFFSETOF(wl_anqpo_ignore_ssid_list_t, ssid)},
	{"anqpo_ignore_bssid_list", IOV_ANQPO_IGNORE_BSSID_LIST,
	0, IOVT_BUFFER, OFFSETOF(wl_anqpo_ignore_bssid_list_t, bssid)},
	{NULL, 0, 0, 0, 0}
};

static int wl_anqpo_auto_hotspot(wl_anqpo_info_t *anqpo, int max_ap);
static int wl_anqpo_get_ignore_bssid_list(wl_anqpo_info_t *anqpo, void *arg, int len);
static int wl_anqpo_set_ignore_bssid_list(wl_anqpo_info_t *anqpo, void *arg, int len);
static int wl_anqpo_ignore_mode(wl_anqpo_info_t *anqpo, int mode);
static int wl_anqpo_get_ignore_ssid_list(wl_anqpo_info_t *anqpo, void *arg, int len);
static int wl_anqpo_set_ignore_ssid_list(wl_anqpo_info_t *anqpo, void *arg, int len);
static int wl_anqpo_set(wl_anqpo_info_t *anqpo, void *arg, int len);
static int wl_anqpo_start_query(wl_anqpo_info_t *anqpo, void *arg, int len);
static int wl_anqpo_stop_query(wl_anqpo_info_t *anqpo, void *arg, int len);

/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */
#include <wlc_patch.h>

/* ----------------------------------------------------------- */

static void
free_ssid_llist(wlc_info_t *wlc, ssid_llist_t **head)
{
	ssid_llist_t **curr;

	/* free from head of link list */
	curr = head;
	while (*curr != 0) {
		ssid_llist_t *temp = *curr;
		*curr = temp->next;
		MFREE(wlc->osh, temp, sizeof(ssid_llist_t));
	}
	*head = 0;
}

static ssid_llist_t *
find_ssid_llist(ssid_llist_t *head, uint32 ssid_len, uchar *ssid)
{
	ssid_llist_t *curr;

	/* find from head of link list */
	curr = head;
	while (curr != 0) {
		if (ssid_len == curr->ssid.SSID_len &&
			memcmp(&curr->ssid.SSID, ssid, ssid_len) == 0)
			return curr;
		curr = curr->next;
	}
	return 0;
}

static ssid_llist_t *
add_ssid_llist(wlc_info_t *wlc, ssid_llist_t **head, uint32 ssid_len, uchar *ssid)
{
	ssid_llist_t **curr, *item;

	/* find if already exist */
	item = find_ssid_llist(*head, ssid_len, ssid);
	if (item != 0) {
		return item;
	}

	item = MALLOCZ(wlc->osh, sizeof(ssid_llist_t));
	if (item == 0)
		return 0;
	item->ssid.SSID_len = MIN(ssid_len, sizeof(item->ssid.SSID));
	memcpy(item->ssid.SSID, ssid, item->ssid.SSID_len);

	/* add to end of link list */
	curr = head;
	while (*curr != 0) {
		curr = &(*curr)->next;
	}
	*curr = item;
	return *curr;
}

/* ----------------------------------------------------------- */

static void
free_ether_llist(wlc_info_t *wlc, ether_llist_t **head)
{
	ether_llist_t **curr;

	/* free from head of link list */
	curr = head;
	while (*curr != 0) {
		ether_llist_t *temp = *curr;
		*curr = temp->next;
		MFREE(wlc->osh, temp, sizeof(ether_llist_t));
	}
	*head = 0;
}

static ether_llist_t *
find_ether_llist(ether_llist_t *head, struct ether_addr *bssid)
{
	ether_llist_t *curr;

	/* find from head of link list */
	curr = head;
	while (curr != 0) {
		if (memcmp(&curr->addr, bssid, sizeof(curr->addr)) == 0)
			return curr;
		curr = curr->next;
	}
	return 0;
}

static ether_llist_t *
add_ether_llist(wlc_info_t *wlc, ether_llist_t **head, struct ether_addr *bssid)
{
	ether_llist_t **curr, *item;

	/* find if already exist */
	item = find_ether_llist(*head, bssid);
	if (item != 0) {
		return item;
	}

	item = MALLOC(wlc->osh, sizeof(ether_llist_t));
	if (item == 0)
		return 0;
	item->next = 0;
	memcpy(&item->addr, bssid, sizeof(item->addr));

	/* add to end of link list */
	curr = head;
	while (*curr != 0) {
		curr = &(*curr)->next;
	}
	*curr = item;
	return *curr;
}

/* ----------------------------------------------------------- */

/* find peer by gas */
static peer_t *
wl_anqpo_find_peer_by_gas(wl_anqpo_info_t *anqpo, bcm_gas_t *gas)
{
	peer_t *peer = 0;
	int i;

	for (i = 0; i < anqpo->peer_count; i++) {
		if (anqpo->peer[i].gas == gas) {
			peer = &anqpo->peer[i];
			break;
		}
	}

	return peer;
}

/** GAS fragment event notification */
static void gas_fragment_event(wlc_info_t* wlc, wlc_bsscfg_t *bsscfg,
	const struct ether_addr* addr, uint status, void *data, int datalen)
{
	wlc_bss_mac_event(wlc, bsscfg, WLC_E_GAS_FRAGMENT_RX, addr,
		status, 0, 0, data, datalen);
}

/** retrieve query response/fragment and notify host */
static void
process_query_response(wlc_info_t *wlc, peer_t *peer, uint status,
	uint16 channel, uint8 dialog_token, uint8 fragment_id, uint16 status_code)
{
	int rsp_length;
	int length;
	uint8 *buffer;

	/* get query response/fragment length */
	rsp_length = bcm_gas_get_query_response_length(peer->gas);
	length = OFFSETOF(wl_event_gas_t, data) + rsp_length;
	buffer = MALLOCZ(wlc->osh, length);
	if (buffer != 0) {
		wl_event_gas_t *gas_data = (wl_event_gas_t *)buffer;
		int bufLen;

		/* initialize gas event data */
		gas_data->channel = channel;
		gas_data->dialog_token = dialog_token;
		gas_data->fragment_id = fragment_id;
		gas_data->status_code = status_code;
		gas_data->data_len = rsp_length;

		/* get query response/fragment */
		if (bcm_gas_get_query_response(peer->gas, rsp_length, &bufLen, gas_data->data)) {
			/* generate event to host */
			gas_fragment_event(wlc, wlc->cfg, &peer->addr, status, gas_data, length);
			WL_INFORM(("%s:GAS fragment 0x%02x\n", __FUNCTION__, fragment_id));
		}
		MFREE(wlc->osh, buffer, length);
	} else {
		WL_ERROR(("wl%d: %s:MALLOC failed\n", WLCWLUNIT(wlc), __FUNCTION__));
	}
}

static bool
create_start_gas(wl_anqpo_info_t *anqpo, peer_t *peer)
{
	peer->gas = bcm_gas_create((struct bcm_gas_wl_drv_hdl *)anqpo->wlc,
		0, peer->channel, &peer->addr);
	if (peer->gas == 0)
		return FALSE;
	if (is_auto_hotspot_enabled(anqpo)) {
		/* max retransmit and comeback disabled for auto ANQP */
		bcm_gas_set_max_retransmit(peer->gas, 0);
		bcm_gas_set_max_comeback_delay(peer->gas, 0);

	} else {
		bcm_gas_set_max_retransmit(peer->gas, anqpo->max_retransmit);
		bcm_gas_set_max_comeback_delay(peer->gas, anqpo->max_comeback_delay);
	}
	bcm_gas_set_response_timeout(peer->gas, anqpo->response_timeout);
	bcm_gas_set_query_request(peer->gas, anqpo->query_len,
		anqpo->query_data);
	bcm_gas_start(peer->gas);
	return TRUE;
}

static bool
is_queries_completed(wl_anqpo_info_t *anqpo)
{
	if (anqpo->active_peer_count == 0 &&
		anqpo->started_peer_count == anqpo->peer_count)
		return TRUE;
	else
		return FALSE;
}

static void
start_queries(wl_anqpo_info_t *anqpo)
{
	wlc_info_t *wlc = anqpo->wlc;

	while (((is_start_query_enabled(anqpo) &&
		anqpo->active_peer_count < MAX_ACTIVE_PEERS_START_QUERY) ||
		(is_auto_hotspot_enabled(anqpo) &&
		anqpo->active_peer_count < MAX_ACTIVE_PEERS_AUTO_HOTSPOT)) &&
		anqpo->started_peer_count < anqpo->peer_count) {
		uint16 curr_channel;
		peer_t *peer = &anqpo->peer[anqpo->started_peer_count];

		if (is_auto_hotspot_enabled(anqpo)) {
			/* current scan channel */
			curr_channel = wf_chspec_ctlchan(WLC_BAND_PI_RADIO_CHANSPEC);
		}
		else {
			if (anqpo->active_peer_count == 0) {
				/* no peer running - current channel is next peer channel */
				curr_channel = peer->channel;
			}
			else {
				/* peers running - current channel is last started peer */
				curr_channel = anqpo->peer[anqpo->started_peer_count - 1].channel;
			}
		}

		/* channel of next peer is not current channel */
		if (peer->channel != curr_channel)
			break;

		/* start next peer */
		if (!create_start_gas(anqpo, peer))
			return;
		anqpo->started_peer_count++;
		anqpo->active_peer_count++;
	}
}

static void
reset_queries(wl_anqpo_info_t *anqpo)
{
	int i;

	/* stop/destroy any GAS */
	for (i = 0; i < anqpo->peer_count; i++) {
		peer_t *peer = &anqpo->peer[i];
		if (peer->gas != 0) {
			bcm_gas_destroy(peer->gas);
			peer->gas = 0;
		}
	}

	/* discard existing peer list */
	if (anqpo->peer) {
		MFREE(WLCOSH(anqpo), anqpo->peer, anqpo->peer_len);
		anqpo->peer = NULL;
		anqpo->peer_len = 0;
		anqpo->peer_count = 0;
	}

	anqpo->active_peer_count = 0;
	anqpo->started_peer_count = 0;
}

/* callback to handle gas events */
static void
wl_anqpo_gas_event_cb(void *context, bcm_gas_t *gas, bcm_gas_event_t *event)
{
	wlc_info_t *wlc = (wlc_info_t *)context;
	wl_anqpo_info_t *anqpo = wlc->anqpo;
	peer_t *peer;

	if (event->type == BCM_GAS_EVENT_QUERY_REQUEST) {
		return;
	}

	peer = wl_anqpo_find_peer_by_gas(anqpo, gas);
	if (peer != 0) {
		if (event->type == BCM_GAS_EVENT_RESPONSE_FRAGMENT) {
			process_query_response(wlc, peer, WLC_E_STATUS_PARTIAL,
				peer->channel, event->dialogToken,
				event->rspFragment.fragmentId, DOT11_SC_SUCCESS);
		}
		else if (event->type == BCM_GAS_EVENT_STATUS) {
			anqpo->active_peer_count--;

			if (event->status.statusCode == DOT11_SC_SUCCESS) {
				process_query_response(wlc, peer, WLC_E_STATUS_SUCCESS,
					peer->channel, event->dialogToken,
					0, event->status.statusCode);
				bcm_gas_destroy(peer->gas);
				peer->gas = 0;
				if (is_auto_hotspot_enabled(anqpo)) {
					if (anqpo->ignore_mode == IGNORE_BSSID) {
						add_ether_llist(anqpo->wlc,
							&anqpo->ignore_bssid_llist, &peer->addr);
					} else {
						add_ssid_llist(anqpo->wlc,
							&anqpo->ignore_ssid_llist,
							peer->ssid.SSID_len, peer->ssid.SSID);
					}
				}
			}
			else if (is_start_query_enabled(anqpo) &&
				event->status.statusCode != DOT11_SC_FAILURE &&
				peer->num_retries < anqpo->max_retries) {
				/* retry manual ANQP if not unspecified failure
				 * which is comeback delay exceeded
				 */
				bcm_gas_start(peer->gas);
				peer->num_retries++;
				anqpo->active_peer_count++;
			}
			else {
				wl_event_gas_t gas_data;

				/* fail event to host */
				memset(&gas_data, 0, sizeof(gas_data));
				gas_data.channel = peer->channel;
				gas_data.dialog_token = event->dialogToken;
				gas_data.fragment_id = 0;
				gas_data.status_code = event->status.statusCode;
				gas_data.data_len = 0;
				gas_fragment_event(wlc, wlc->cfg, &peer->addr, WLC_E_STATUS_FAIL,
					&gas_data, sizeof(gas_data));

				bcm_gas_destroy(peer->gas);
				peer->gas = 0;
			}

			if (is_start_query_enabled(anqpo)) {
				if (anqpo->active_peer_count == 0) {
					/* abort dwell time */
					wlc_scan_abort(wlc->scan, WLC_E_STATUS_ABORT);
				}

				/* start next queries */
				start_queries(anqpo);

				if (is_queries_completed(anqpo)) {
					wl_gas_stop_eventq(anqpo->gasi);
					bcm_gas_unsubscribe_event(wl_anqpo_gas_event_cb);
					anqpo->is_start_query = FALSE;
					wlc_bss_mac_event(wlc, wlc->cfg, WLC_E_GAS_COMPLETE, 0,
						WLC_E_STATUS_SUCCESS, 0, 0, 0, 0);
				}

			}
			else if (is_auto_hotspot_enabled(anqpo)) {
				/* start next queries */
				start_queries(anqpo);
			}
		}
	}
}

/** set ANQP query */
static int
wl_anqpo_set(wl_anqpo_info_t *anqpo, void *arg, int len)
{
	wl_anqpo_set_t *set = (wl_anqpo_set_t *)arg;

	/* discard existing query */
	if (anqpo->query_data) {
		MFREE(WLCOSH(anqpo), anqpo->query_data, anqpo->query_len);
		anqpo->query_data = NULL;
		anqpo->query_len = 0;
	}

	/* default values */
	anqpo->max_retransmit = DEFAULT_MAX_RETRANSMIT;
	anqpo->response_timeout = DEFAULT_RESPONSE_TIMEOUT;
	anqpo->max_comeback_delay = DEFAULT_MAX_COMEBACK_DELAY;
	anqpo->max_retries = DEFAULT_MAX_RETRIES;

	if (set->max_retransmit != PARAM_NOT_CONFIGURED)
		anqpo->max_retransmit = set->max_retransmit;
	if (set->response_timeout != PARAM_NOT_CONFIGURED)
		anqpo->response_timeout = set->response_timeout;
	if (set->max_comeback_delay != PARAM_NOT_CONFIGURED)
		anqpo->max_comeback_delay = set->max_comeback_delay;
	if (set->max_retries != PARAM_NOT_CONFIGURED)
		anqpo->max_retries = set->max_retries;

	/* save new query */
	if (set->query_len) {
		anqpo->query_data = MALLOC(WLCOSH(anqpo), set->query_len);
		if (!anqpo->query_data) {
			WL_ERROR(("wl%d: %s: MALLOC failed; total mallocs %d bytes\n",
			          WLCUNIT(anqpo), __FUNCTION__, MALLOCED(WLCOSH(anqpo))));
			anqpo->query_len = 0;
			return BCME_NOMEM;
		}
		anqpo->query_len = set->query_len;
		memcpy(anqpo->query_data, set->query_data, set->query_len);
	}

	return BCME_OK;
}

/** stop ANQP query */
static int
wl_anqpo_stop_query(wl_anqpo_info_t *anqpo, void *arg, int len)
{
	(void)arg;
	(void)len;

	/* abort any scan in progress */
	if (SCAN_IN_PROGRESS(anqpo->wlc->scan)) {
		wlc_scan_abort(anqpo->wlc->scan, WLC_E_STATUS_ABORT);
	}

	if (is_start_query_enabled(anqpo)) {
		reset_queries(anqpo);
		wl_gas_stop_eventq(anqpo->gasi);
		bcm_gas_unsubscribe_event(wl_anqpo_gas_event_cb);
		anqpo->is_start_query = FALSE;
	}
	return BCME_OK;
}

/** add peer to query list */
static void
add_peer(wl_anqpo_info_t *anqpo, uint16 channel, struct ether_addr *addr,
	uint32 ssid_len, uchar *ssid)
{
	int i;
	peer_t *p;

	/* check if addr already exists */
	for (i = 0; i < anqpo->peer_count; i++) {
		p = &anqpo->peer[i];
		if (memcmp(&p->addr, addr, sizeof(p->addr)) == 0)
			return;
	}

	p = &anqpo->peer[anqpo->peer_count];
	p->channel = channel;
	memcpy(&p->addr, addr, sizeof(p->addr));
	p->ssid.SSID_len = MIN(ssid_len, sizeof(p->ssid.SSID));
	memcpy(p->ssid.SSID, ssid, p->ssid.SSID_len);
	anqpo->peer_count++;
}

/** initialize queries */
static int
init_queries(wl_anqpo_info_t *anqpo, int count, wl_anqpo_peer_t *peer)
{
	reset_queries(anqpo);
	anqpo->peer_len = count * sizeof(*anqpo->peer);
	anqpo->peer = MALLOC(WLCOSH(anqpo), anqpo->peer_len);
	if (!anqpo->peer) {
		WL_ERROR(("wl%d: %s: MALLOC failed; total mallocs %d bytes\n",
		          WLCUNIT(anqpo), __FUNCTION__, MALLOCED(WLCOSH(anqpo))));
		anqpo->peer_len = 0;
		return BCME_NOMEM;
	}
	memset(anqpo->peer, 0, anqpo->peer_len);
	anqpo->peer_count = 0;

	/* initialize peer info if available */
	if (peer != 0) {
		int i;

		for (i = 0; i < count; i++) {
			add_peer(anqpo, peer[i].channel, &peer[i].addr, 0, 0);
		}
	}

	return BCME_OK;
}

/** start ANQP query */
static int
wl_anqpo_start_query(wl_anqpo_info_t *anqpo, void *arg, int len)
{
	wl_anqpo_peer_list_t *list = (wl_anqpo_peer_list_t *)arg;
	int err;

	if (is_auto_hotspot_enabled(anqpo))
		return BCME_BUSY;

	if (list->count == 0 || anqpo->query_len == 0)
		return BCME_ERROR;

	/* abort any scan in progress */
	if (SCAN_IN_PROGRESS(anqpo->wlc->scan)) {
		wlc_scan_abort(anqpo->wlc->scan, WLC_E_STATUS_ABORT);
	}

	/* disable if already enabled */
	if (is_start_query_enabled(anqpo)) {
		reset_queries(anqpo);
		wl_gas_stop_eventq(anqpo->gasi);
		bcm_gas_unsubscribe_event(wl_anqpo_gas_event_cb);
		anqpo->is_start_query = FALSE;
	}

	if ((err = init_queries(anqpo, list->count, list->peer)) != BCME_OK)
		return err;

	bcm_gas_subscribe_event(anqpo->wlc, wl_anqpo_gas_event_cb);
	wl_gas_start_eventq(anqpo->gasi);
	anqpo->is_start_query = TRUE;
	anqpo->active_peer_count = 0;
	anqpo->started_peer_count = 0;
	start_queries(anqpo);

	return BCME_OK;
}

/** enable/disable automatic ANQP query to scanned hotspot APs */
static int
wl_anqpo_auto_hotspot(wl_anqpo_info_t *anqpo, int max_ap)
{
	if (is_start_query_enabled(anqpo))
		return BCME_BUSY;

	/* disable if already enabled */
	if (is_auto_hotspot_enabled(anqpo)) {
		wl_gas_stop_eventq(anqpo->gasi);
		bcm_gas_unsubscribe_event(wl_anqpo_gas_event_cb);
		reset_queries(anqpo);
		anqpo->max_auto_hotspot = 0;
	}

	/* enable if requested */
	if (max_ap > 0) {
		int err;

		if (anqpo->query_len == 0)
			return BCME_ERROR;

		if ((err = init_queries(anqpo, max_ap, 0)) != BCME_OK)
			return err;

		bcm_gas_subscribe_event(anqpo->wlc, wl_anqpo_gas_event_cb);
		wl_gas_start_eventq(anqpo->gasi);
		anqpo->max_auto_hotspot = max_ap;
	}

	return BCME_OK;
}

/** ignore duplicate SSIDs or BSSIDs */
static int
wl_anqpo_ignore_mode(wl_anqpo_info_t *anqpo, int mode)
{
	if (!(mode == IGNORE_SSID || mode == IGNORE_BSSID)) {
		return BCME_ERROR;
	}
	if (is_auto_hotspot_enabled(anqpo)) {
		return BCME_BUSY;
	}
	anqpo->ignore_mode = mode;

	return BCME_OK;
}

/** get ANQP ignore SSID list */
static int
wl_anqpo_get_ignore_ssid_list(wl_anqpo_info_t *anqpo, void *arg, int len)
{
	wl_anqpo_ignore_ssid_list_t *list = (wl_anqpo_ignore_ssid_list_t *)arg;
	ssid_llist_t *ignore = anqpo->ignore_ssid_llist;
	int i;

	list->count = 0;
	for (i = 0; i < ANQPO_MAX_IGNORE_SSID && ignore != 0;
		i++, ignore = ignore->next) {
		memcpy(&list->ssid[list->count++], &ignore->ssid,
			sizeof(ignore->ssid));
	}

	/* not all retrieved */
	if (list->count == ANQPO_MAX_IGNORE_SSID && ignore != 0)
		return BCME_RANGE;

	return BCME_OK;
}

/** set ANQP ignore SSID list */
static int
wl_anqpo_set_ignore_ssid_list(wl_anqpo_info_t *anqpo, void *arg, int len)
{
	wl_anqpo_ignore_ssid_list_t *list = (wl_anqpo_ignore_ssid_list_t *)arg;
	int i;

	if (list->is_clear)
		free_ssid_llist(anqpo->wlc, &anqpo->ignore_ssid_llist);

	for (i = 0; i < list->count; i++) {
		if (add_ssid_llist(anqpo->wlc, &anqpo->ignore_ssid_llist,
			list->ssid[i].SSID_len, list->ssid[i].SSID) == 0)
			return BCME_NOMEM;
	}

	return BCME_OK;
}

/** get ANQP ignore BSSID list */
static int
wl_anqpo_get_ignore_bssid_list(wl_anqpo_info_t *anqpo, void *arg, int len)
{
	wl_anqpo_ignore_bssid_list_t *list = (wl_anqpo_ignore_bssid_list_t *)arg;
	ether_llist_t *ignore = anqpo->ignore_bssid_llist;
	int i;

	list->count = 0;
	for (i = 0; i < ANQPO_MAX_IGNORE_BSSID && ignore != 0;
		i++, ignore = ignore->next) {
		memcpy(&list->bssid[list->count++], &ignore->addr,
			sizeof(ignore->addr));
	}

	/* not all retrieved */
	if (list->count == ANQPO_MAX_IGNORE_BSSID && ignore != 0)
		return BCME_RANGE;

	return BCME_OK;
}

/** set ANQP ignore BSSID list */
static int
wl_anqpo_set_ignore_bssid_list(wl_anqpo_info_t *anqpo, void *arg, int len)
{
	wl_anqpo_ignore_bssid_list_t *list = (wl_anqpo_ignore_bssid_list_t *)arg;
	int i;

	if (list->is_clear)
		free_ether_llist(anqpo->wlc, &anqpo->ignore_bssid_llist);

	for (i = 0; i < list->count; i++) {
		if (add_ether_llist(anqpo->wlc, &anqpo->ignore_bssid_llist,
			&list->bssid[i]) == 0)
			return BCME_NOMEM;
	}

	return BCME_OK;
}

/** handling anqpo related iovars */
static int
anqpo_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
            void *p, uint plen, void *a, int alen, int vsize, struct wlc_if *wlcif)
{
	wl_anqpo_info_t *anqpo = hdl;
	int32 int_val = 0;
	uint32 *ret_uint_ptr;
	int err = BCME_OK;
	ASSERT(anqpo);

#ifndef BCMROMOFFLOAD
	WL_INFORM(("wl%d: anqpo_doiovar()\n", WLCUNIT(anqpo)));
#endif /* !BCMROMOFFLOAD */

	/* Do nothing if is not yet enabled */
	if (!ANQPO_ENAB(anqpo->wlc->pub)) {
		return BCME_UNSUPPORTED;
	}

	if (plen >= (int)sizeof(int_val))
		bcopy(p, &int_val, sizeof(int_val));

	/* convenience int ptr for 4-byte gets (requires int aligned arg) */
	ret_uint_ptr = (uint32 *)a;

	switch (actionid) {
	case IOV_SVAL(IOV_ANQPO_SET):
		err = wl_anqpo_set(anqpo, a, alen);
		break;
	case IOV_SVAL(IOV_ANQPO_STOP_QUERY):
		err = wl_anqpo_stop_query(anqpo, a, alen);
		break;
	case IOV_SVAL(IOV_ANQPO_START_QUERY):
		err = wl_anqpo_start_query(anqpo, a, alen);
		break;
	case IOV_GVAL(IOV_ANQPO_AUTO_HOTSPOT):
		*ret_uint_ptr = anqpo->max_auto_hotspot;
		break;
	case IOV_SVAL(IOV_ANQPO_AUTO_HOTSPOT):
		err = wl_anqpo_auto_hotspot(anqpo, int_val);
		break;
	case IOV_GVAL(IOV_ANQPO_IGNORE_MODE):
		*ret_uint_ptr = anqpo->ignore_mode;
		break;
	case IOV_SVAL(IOV_ANQPO_IGNORE_MODE):
		err = wl_anqpo_ignore_mode(anqpo, int_val);
		break;
	case IOV_GVAL(IOV_ANQPO_IGNORE_SSID_LIST):
		err = wl_anqpo_get_ignore_ssid_list(anqpo, a, alen);
		break;
	case IOV_SVAL(IOV_ANQPO_IGNORE_SSID_LIST):
		err = wl_anqpo_set_ignore_ssid_list(anqpo, a, alen);
		break;
	case IOV_GVAL(IOV_ANQPO_IGNORE_BSSID_LIST):
		err = wl_anqpo_get_ignore_bssid_list(anqpo, a, alen);
		break;
	case IOV_SVAL(IOV_ANQPO_IGNORE_BSSID_LIST):
		err = wl_anqpo_set_ignore_bssid_list(anqpo, a, alen);
		break;
	default:
		err = BCME_UNSUPPORTED;
		break;
	}

	return err;
}

/**
 * initialize anqpo private context.
 * returns a pointer to the anqpo private context, NULL on failure.
 */
wl_anqpo_info_t *
BCMATTACHFN(wl_anqpo_attach)(wlc_info_t *wlc, wl_gas_info_t *gas)
{
	wl_anqpo_info_t *anqpo;

	/* allocate anqpo private info struct */
	anqpo = MALLOCZ(wlc->osh, sizeof(wl_anqpo_info_t));
	if (!anqpo) {
		WL_ERROR(("wl%d: %s: MALLOC failed; total mallocs %d bytes\n",
			WLCWLUNIT(wlc), __FUNCTION__, MALLOCED(wlc->osh)));
		return NULL;
	}

	/* init anqpo private info struct */
	anqpo->wlc = wlc;
	wlc->anqpo = anqpo;
	anqpo->gasi = gas;

	/* register module */
	if (wlc_module_register(wlc->pub, anqpo_iovars, "anqpo",
		anqpo, anqpo_doiovar, NULL, NULL, NULL)) {
		WL_ERROR(("wl%d: %s wlc_module_register() failed\n",
			WLCWLUNIT(wlc), __FUNCTION__));
		return NULL;
	}

#if !defined(P2PO)	/* P2PO supports incoming queries */
	/* initiated ANQP queries only */
	bcm_gas_incoming_request(FALSE);
#endif	/* P2PO */

	/* turn on event forwarding */
	wlc_eventq_set_ind(anqpo->wlc->eventq, WLC_E_GAS_FRAGMENT_RX, 1);

	wlc->pub->_anqpo = TRUE;
	return anqpo;
}

/** cleanup anqpo private context */
void
BCMATTACHFN(wl_anqpo_detach)(wl_anqpo_info_t *anqpo)
{
	WL_INFORM(("wl%d: anqpo_detach()\n", WLCUNIT(anqpo)));

	if (!anqpo)
		return;

	anqpo->wlc->pub->_anqpo = FALSE;

	/* turn off event forwarding */
	wlc_eventq_set_ind(anqpo->wlc->eventq, WLC_E_GAS_FRAGMENT_RX, 0);

	/* discard existing query */
	if (anqpo->query_data) {
		MFREE(WLCOSH(anqpo), anqpo->query_data, anqpo->query_len);
		anqpo->query_data = NULL;
		anqpo->query_len = 0;
	}

	wlc_module_unregister(anqpo->wlc->pub, "anqpo", anqpo);
	MFREE(WLCOSH(anqpo), anqpo, sizeof(wl_anqpo_info_t));

	anqpo = NULL;
}

/** initialize on scan start */
void wl_anqpo_scan_start(wl_anqpo_info_t *anqpo)
{
	if (is_auto_hotspot_enabled(anqpo)) {
		init_queries(anqpo, anqpo->max_auto_hotspot, 0);
	}
}

/** deinitialize on scan stop */
extern void wl_anqpo_scan_stop(wl_anqpo_info_t *anqpo)
{
	if (is_auto_hotspot_enabled(anqpo)) {
		reset_queries(anqpo);
	}
}

/** process scan result */
extern void wl_anqpo_process_scan_result(wl_anqpo_info_t *anqpo,
	wlc_bss_info_t *bi, uint8 *ie, uint32 ie_len)
{
	wlc_info_t *wlc = anqpo->wlc;
	bcm_decode_t dec1, dec2;
	bcm_decode_ie_t ies;
	uint16 channel;
	uint8 hotspotConfig;

	if (anqpo->max_auto_hotspot == 0 ||
		anqpo->peer_count == anqpo->max_auto_hotspot ||
		anqpo->query_len == 0)
		return;

	/* ignore if found in ignore list */
	if (anqpo->ignore_mode == IGNORE_BSSID) {
		if (find_ether_llist(anqpo->ignore_bssid_llist, &bi->BSSID) != 0)
			return;
	} else {
		if (find_ssid_llist(anqpo->ignore_ssid_llist, bi->SSID_len, bi->SSID) != 0)
			return;
	}

	bcm_decode_init(&dec1, ie_len, ie);
	bcm_decode_ie(&dec1, &ies);

	/* no hotspot indication IE */
	if (ies.hotspotIndication == 0)
		return;

	/* determine channel */
	if (ies.ds != 0 && ies.dsLength == 1) {
		/* channel from DS IE */
		channel = *ies.ds;
	} else if (CHSPEC_IS5G(bi->chanspec)) {
		/* channel from chanspec for 5G */
		channel = wf_chspec_ctlchan(bi->chanspec);
	} else {
		return;
	}

	/* ignore if not from current channel */
	if (channel != wf_chspec_ctlchan(WLC_BAND_PI_RADIO_CHANSPEC)) {
		return;
	}

	bcm_decode_init(&dec2, ies.hotspotIndicationLength, ies.hotspotIndication);
	/* failed to decode hotspot indication IE */
	if (!bcm_decode_ie_hotspot_indication(&dec2, &hotspotConfig))
		return;

	/* add peer to query list */
	if (anqpo->peer_count < anqpo->max_auto_hotspot) {
		add_peer(anqpo, channel, &bi->BSSID, bi->SSID_len, bi->SSID);
	}

	start_queries(anqpo);
}
