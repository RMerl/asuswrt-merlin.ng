/**
 * @file
 * @brief
 * WL (per-port) event queue, for use by dongle offloads that need to process wl events
 * asynchronously. Not to be confused with wlc_eventq, which is used by the common code
 * to send events to the host.
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
 * $Id: wl_eventq.c 708017 2017-06-29 14:11:45Z $
 *
 */

/**
 * @file
 * @brief
 * XXX Apple specific feature
 * Twiki: [OffloadsEventQueue]
 */

#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <wlioctl.h>
#include <bcmendian.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_channel.h>
#include <wlc_pub.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wl_export.h>
#include <wl_eventq.h>

#define MAX_QUEUE_DEPTH		4

typedef	struct client {
	struct client		*next;

	wl_eventq_cb_t		cb;			/* callback function */
	void			*arg;			/* callback function argument */

	uint			count;			/* interested event count */
	uint32			event[1];		/* interested events */
} client_t;

/* wl_eventq private info structure */
struct wl_eventq_info {
	wlc_info_t		*wlc;			/* Pointer back to wlc structure */

	wlc_event_t		*head;			/* head of wl event queue */
	uint			qdepth;			/* depth of wl event queue */
	uint			max_qdepth;		/* max depth of wl event queue */
	struct wl_timer		*timer;			/* timer to consume events
							 * in wl event queue
							 */
	client_t		*client;		/* client list */
};

/* wlc_pub_t struct access macros */
#define WLCUNIT(x)	((x)->wlc->pub->unit)
#define WLCOSH(x)	((x)->wlc->osh)

static void wl_eventq_process_event(void *arg);
static void wl_eventq_free_event(wl_eventq_info_t *wlevtq, wlc_event_t *e);

/*
 * initialize wl event queue private context.
 * returns a pointer to the wl event queue private context, NULL on failure.
 */
wl_eventq_info_t *
BCMATTACHFN(wl_eventq_attach)(wlc_info_t *wlc)
{
	wl_eventq_info_t *wlevtq;

	/* allocate wl event queue private info struct */
	wlevtq = MALLOC(wlc->osh, sizeof(wl_eventq_info_t));
	if (!wlevtq) {
		WL_ERROR(("wl%d: %s: MALLOC failed; total mallocs %d bytes\n",
		          WLCWLUNIT(wlc), __FUNCTION__, MALLOCED(wlc->osh)));
		return NULL;
	}

	/* init wl event queue private info struct */
	bzero(wlevtq, sizeof(wl_eventq_info_t));
	wlevtq->wlc = wlc;

	if (!(wlevtq->timer = wl_init_timer(wlc->wl,
		wl_eventq_process_event, wlevtq, "wl_eventq"))) {
		WL_ERROR(("wl%d: wl_eventq timer failed\n", WLCWLUNIT(wlc)));
		MFREE(WLCOSH(wlevtq), wlevtq, sizeof(wl_eventq_info_t));
		return NULL;
	}

	/* set variable to avoid rom invalidation */
	wlevtq->max_qdepth = MAX_QUEUE_DEPTH;

	return wlevtq;
}

/* cleanup wl event queue private context */
void
BCMATTACHFN(wl_eventq_detach)(wl_eventq_info_t *wlevtq)
{
	WL_INFORM(("wl%d: wl_eventq_detach()\n", WLCUNIT(wlevtq)));

	if (!wlevtq)
		return;

	/* empty event q */
	while (wlevtq->head) {
		wlc_event_t *e = wlevtq->head;
		wlevtq->head = e->next;
		wl_eventq_free_event(wlevtq, e);
	}

	/* free client list */
	while (wlevtq->client) {
		client_t *client = wlevtq->client;
		wlevtq->client = client->next;
		MFREE(WLCOSH(wlevtq), client, sizeof(client_t) +
			sizeof(client->event) * (sizeof(client->count) - 1));
	}

	wl_del_timer(wlevtq->wlc->wl, wlevtq->timer);
	wl_free_timer(wlevtq->wlc->wl, wlevtq->timer);

	MFREE(WLCOSH(wlevtq), wlevtq, sizeof(wl_eventq_info_t));

	wlevtq = NULL;
}

static void
wl_eventq_free_event(wl_eventq_info_t *wlevtq, wlc_event_t *e)
{
	if (e) {
		if (e->addr)
			MFREE(WLCOSH(wlevtq), e->addr, sizeof(struct ether_addr));
		if (e->data)
			MFREE(WLCOSH(wlevtq), e->data, e->event.datalen);
		MFREE(WLCOSH(wlevtq), e, sizeof(wlc_event_t));
	}
}

/* register a callback fn to handle events */
int
wl_eventq_register_event_cb(wl_eventq_info_t *wlevtq, uint32 event[], uint count,
	wl_eventq_cb_t cb, void *arg)
{
	int i;
	int new_client_size;
	client_t *new_client;
	client_t **clientp;

	if (wlevtq == 0) {
		WL_ERROR(("%s: wl_eventq not initialized\n", __FUNCTION__));
		return BCME_ERROR;
	}

	if (cb == 0 || count == 0)
		return BCME_ERROR;

	/* remove existing client w/ same callback if any */
	wl_eventq_unregister_event_cb(wlevtq, cb);

	/* create new client */
	new_client_size = sizeof(client_t) + sizeof(*event) * (count - 1);
	new_client = MALLOC(WLCOSH(wlevtq), new_client_size);
	if (new_client == 0) {
		WL_ERROR(("wl%d: %s: MALLOC failed; total mallocs %d bytes\n",
			WLCUNIT(wlevtq), __FUNCTION__, MALLOCED(WLCOSH(wlevtq))));
		return BCME_NOMEM;
	}
	memset(new_client, 0, new_client_size);
	new_client->cb = cb;
	new_client->arg = arg;
	new_client->count = count;

	for (i = 0; i < count; i++)
		new_client->event[i] = event[i];

	/* add to the end of client list */
	clientp = &wlevtq->client;
	while (*clientp)
		clientp = &(*clientp)->next;
	*clientp = new_client;

	return BCME_OK;
}

/* unregister a callback fn */
void
wl_eventq_unregister_event_cb(wl_eventq_info_t *wlevtq, wl_eventq_cb_t cb)
{
	if (wlevtq == 0) {
		WL_ERROR(("%s: wl_eventq not initialized\n", __FUNCTION__));
		return;
	}

	if (cb) {
		client_t **clientp = &wlevtq->client;
		while (*clientp) {
			if ((*clientp)->cb == cb) {
				client_t *found = *clientp;
				*clientp = (*clientp)->next;
				MFREE(WLCOSH(wlevtq), found, sizeof(client_t) +
					(sizeof(*found->event) * (found->count - 1)));
				break;
			}
			clientp = &(*clientp)->next;
		}
	}
}

/* timer function to consume events in wl event queue */
static void
wl_eventq_process_event(void *arg)
{
	wl_eventq_info_t *wlevtq = (wl_eventq_info_t *)arg;

	/* consume one event at each invocation */
	if (wlevtq->head) {
		wlc_event_t *e = wlevtq->head;
		uint32 event_type = e->event.event_type;
		wl_event_msg_t *wl_event = &e->event;
		uint8 *data = e->data;
		uint32 length = e->event.datalen;

		client_t *client = wlevtq->client;
		while (client) {
			if (client->cb) {
				int i;

				for (i = 0; i < client->count; i++) {
					if (client->event[i] == event_type) {
						(client->cb)(client->arg,
							event_type, wl_event, data, length);
						break;
					}
				}
			}
			client = client->next;
		}

		wlevtq->head = e->next;

		wl_eventq_free_event(wlevtq, e);
		wlevtq->qdepth--;

		/* schedule timer for next event in wl event queue */
		if (wlevtq->head) {
			wl_del_timer(wlevtq->wlc->wl, wlevtq->timer);
			wl_add_timer(wlevtq->wlc->wl, wlevtq->timer, 0, 0);
		}
	}
}

/* add event to wl event queue */
static void
wl_eventq_enq_event(wl_eventq_info_t *wlevtq, wlc_event_t *new_e)
{
	wlc_event_t **ep = &wlevtq->head;

	while (*ep)
		ep = &(*ep)->next;
	*ep = new_e;

	wlevtq->qdepth++;

	/* handle event immediately with zero delay timer */
	wl_del_timer(wlevtq->wlc->wl, wlevtq->timer);
	wl_add_timer(wlevtq->wlc->wl, wlevtq->timer, 0, 0);
}

/* make a new copy of event */
static wlc_event_t *
wl_eventq_copy_event(wl_eventq_info_t *wlevtq, wlc_event_t *e)
{
	wlc_event_t *new_e;

	new_e = MALLOC(WLCOSH(wlevtq), sizeof(wlc_event_t));
	if (new_e == 0)
		goto out_of_mem;
	memcpy(new_e, e, sizeof(wlc_event_t));
	new_e->data = new_e->addr = NULL;

	if (e->data && e->event.datalen) {
		new_e->data = MALLOC(WLCOSH(wlevtq), e->event.datalen);
		if (new_e->data == 0)
			goto out_of_mem;
		memcpy(new_e->data, e->data, e->event.datalen);
	}
	if (e->addr) {
		new_e->addr = MALLOC(WLCOSH(wlevtq), sizeof(struct ether_addr));
		if (new_e->addr == 0)
			goto out_of_mem;
		memcpy(new_e->addr, e->addr, sizeof(struct ether_addr));
	}
	new_e->next = NULL;

	return new_e;

out_of_mem:
	WL_ERROR(("wl%d: %s: MALLOC failed; total mallocs %d bytes\n",
	          WLCUNIT(wlevtq), __FUNCTION__, MALLOCED(WLCOSH(wlevtq))));
	wl_eventq_free_event(wlevtq, new_e);

	return NULL;
}

/* duplicate an event for wl event queue */
void wl_eventq_dup_event(wl_eventq_info_t *wlevtq, wlc_event_t *e)
{
	client_t *client;

	if (wlevtq == 0) {
		WL_ERROR(("%s: wl_eventq not initialized\n", __FUNCTION__));
		return;
	}

	/* ignore event if max queue depth is reached */
	if (wlevtq->qdepth >= wlevtq->max_qdepth) {
		WL_ERROR(("wl%d: %s: exceeding max queue depth(%d)\n",
			WLCUNIT(wlevtq), __FUNCTION__, wlevtq->max_qdepth));
		return;
	}

	/* dup if any registered clients have interest */
	client = wlevtq->client;
	while (client) {
		if (client->cb) {
			int i;
			for (i = 0; i < client->count; i++)
				if (client->event[i] == e->event.event_type) {
					wlc_event_t *new_e;

					/* make a new copy */
					new_e = wl_eventq_copy_event(wlevtq, e);

					/* put new copy to wl event queue */
					if (new_e)
						wl_eventq_enq_event(wlevtq, new_e);
					return;
			}
		}
		client = client->next;
	}
}
