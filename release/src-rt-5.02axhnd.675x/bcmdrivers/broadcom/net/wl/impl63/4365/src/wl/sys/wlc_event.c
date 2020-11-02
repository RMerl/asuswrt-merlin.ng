/*
 * Event mechanism
 *
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
 * $Id: wlc_event.c 754150 2018-03-26 07:02:59Z $
 */

/**
 * @file
 * @brief
 * The WLAN driver currently has tight coupling between different components. In particular,
 * components know about each other, and call each others functions, access data, and invoke
 * callbacks. This means that maintenance and new features require changing these
 * relationships. This is fundamentally a tightly coupled system where everything touches
 * many other things.
 *
 * @brief
 * We can reduce the coupling between our features by reducing their need to directly call
 * each others functions, and access each others data. An mechanism for accomplishing this is
 * a generic event signaling mechanism. The event infrastructure enables modules to communicate
 * indirectly through events, rather than directly by calling each others routines and
 * callbacks.
 */

/**
 * @file
 * @brief
 * XXX Twiki: [WlanSwArchitectureEventNotification]
 */

/* XXX: Define wlc_cfg.h to be the first header file included as some builds
 * get their feature flags thru this file.
 */
#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>
#include <wlioctl.h>
#include <wl_dbg.h>

#include <wlc_pub.h>
#include <wl_export.h>
#include <wlc_event.h>
#include <bcm_mpool_pub.h>

/* For wlc.h */
#include <d11.h>
#include <wlc_bsscfg.h>
#include <wlc_rate.h>
#include <wlc.h>
#include <wlc_rate_sel.h>
#ifdef MSGTRACE
#include <msgtrace.h>
#endif // endif
#ifdef LOGTRACE
#include <logtrace.h>
#endif // endif

/* Local prototypes */
#ifndef WLNOEIND
static void wlc_event_sendup(wlc_eventq_t *eq, const wlc_event_t *e,
	struct ether_addr *da, struct ether_addr *sa, uint8 *data, uint32 len);
#endif /* WLNOEIND */
static void wlc_timer_cb(void *arg);
static int wlc_eventq_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
        void *params, uint p_len, void *arg, int len, int val_size, struct wlc_if *wlcif);

enum {
	IOV_EVENT_MSGS = 1,
	IOV_EVENT_MSGS_EXT = 2
};

static const bcm_iovar_t eventq_iovars[] = {
	{"event_msgs", IOV_EVENT_MSGS,
	(IOVF_OPEN_ALLOW|IOVF_RSDB_SET), IOVT_BUFFER, WL_EVENTING_MASK_LEN
	},
	{"event_msgs_ext", IOV_EVENT_MSGS_EXT,
	(IOVF_OPEN_ALLOW|IOVF_RSDB_SET), IOVT_BUFFER, EVENTMSGS_EXT_STRUCT_SIZE
	},
	{NULL, 0, 0, 0, 0 }
};

#define WL_EVENTING_MASK_EXT_LEN \
	MAX(WL_EVENTING_MASK_LEN, (ROUNDUP(WLC_E_LAST, NBBY)/NBBY))

/* Private data structures */
struct wlc_eventq
{
	wlc_event_t		*head;
	wlc_event_t		*tail;
	struct wlc_info		*wlc;
	void			*wl;
	wlc_pub_t 		*pub;
	bool			tpending;
	bool			workpending;
	struct wl_timer		*timer;
	wlc_eventq_cb_t		cb;
	bcm_mp_pool_h		mpool_h;
	uint8			event_inds_mask_len;
	uint8			*event_inds_mask;
};

/*
 * Export functions
 */
wlc_eventq_t*
BCMATTACHFN(wlc_eventq_attach)(wlc_pub_t *pub, struct wlc_info *wlc, void *wl, wlc_eventq_cb_t cb)
{
	wlc_eventq_t *eq;
	uint		 eventqsize;

	eventqsize = sizeof(wlc_eventq_t) + WL_EVENTING_MASK_EXT_LEN;

	eq = (wlc_eventq_t*)MALLOCZ(pub->osh, eventqsize);

	if (eq == NULL)
		return NULL;

	eq->event_inds_mask_len = WL_EVENTING_MASK_EXT_LEN;

	eq->event_inds_mask = (uint8*)((uintptr)eq + sizeof(wlc_eventq_t));

	/* Create memory pool for 'wlc_event_t' data structs. */
	if (bcm_mpm_create_heap_pool(wlc->mem_pool_mgr, sizeof(wlc_event_t),
	                             "event", &eq->mpool_h) != BCME_OK) {
		WL_ERROR(("wl%d: %s: bcm_mpm_create_heap_pool failed\n",
			pub->unit, __FUNCTION__));
		goto exit;
	}
	eq->cb = cb;
	eq->wlc = wlc;
	eq->wl = wl;
	eq->pub = pub;

	/* register event module */
	if (wlc_module_register(pub, eventq_iovars, "eventq", eq, wlc_eventq_doiovar,
		NULL, NULL, NULL)) {
		WL_ERROR(("wl%d: %s: event wlc_module_register() failed",
			pub->unit, __FUNCTION__));
		goto exit;
	}
	if (!(eq->timer = wl_init_timer(eq->wl, wlc_timer_cb, eq, "eventq"))) {
		WL_ERROR(("wl%d: %s: timer failed\n", pub->unit, __FUNCTION__));
		wlc_module_unregister(eq->pub, "eventq", eq);
		goto exit;
	}
	return eq;

exit:
	MFREE(eq->pub->osh, eq, eventqsize);
	return NULL;
}

int
BCMATTACHFN(wlc_eventq_detach)(wlc_eventq_t *eq)
{
uint		 eventqsize;

	/* Clean up pending events */
	wlc_eventq_down(eq);

	wlc_module_unregister(eq->pub, "eventq", eq);

	if (eq->timer) {
		if (eq->tpending) {
			wl_del_timer(eq->wl, eq->timer);
			eq->tpending = FALSE;
		}
		wl_free_timer(eq->wl, eq->timer);
		eq->timer = NULL;
	}

	ASSERT(wlc_eventq_avail(eq) == FALSE);

	bcm_mpm_delete_heap_pool(eq->wlc->mem_pool_mgr, &eq->mpool_h);

	eventqsize = sizeof(wlc_eventq_t) + WL_EVENTING_MASK_EXT_LEN;

	MFREE(eq->pub->osh, eq, eventqsize);
	return 0;
}

static int
wlc_eventq_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
        void *params, uint p_len, void *arg, int len, int val_size, struct wlc_if *wlcif)
{
	wlc_eventq_t *eq = (wlc_eventq_t *)hdl;
	int err = BCME_OK;

	switch (actionid) {
		case IOV_GVAL(IOV_EVENT_MSGS): {
			eventmsgs_ext_t in_iovar_msg, out_iovar_msg;
			bzero(arg, WL_EVENTING_MASK_LEN);
			in_iovar_msg.len = WL_EVENTING_MASK_LEN;
			out_iovar_msg.len = 0;
			err = wlc_eventq_query_ind_ext(eq, &in_iovar_msg, &out_iovar_msg, arg);
			break;
		}

		case IOV_SVAL(IOV_EVENT_MSGS): {
			eventmsgs_ext_t iovar_msg;
			iovar_msg.len = WL_EVENTING_MASK_LEN;
			iovar_msg.command = EVENTMSGS_SET_MASK;
			err = wlc_eventq_register_ind_ext(eq, &iovar_msg, arg);
			break;
		}

		case IOV_GVAL(IOV_EVENT_MSGS_EXT): {
			if (((eventmsgs_ext_t*)params)->ver != EVENTMSGS_VER) {
				err = BCME_VERSION;
				break;
			}
			if (len < (int)((EVENTMSGS_EXT_STRUCT_SIZE +
				((eventmsgs_ext_t*)params)->len))) {
					err = BCME_BUFTOOSHORT;
					break;
			}
			err = wlc_eventq_query_ind_ext(eq, (eventmsgs_ext_t*)params,
				(eventmsgs_ext_t*)arg, ((eventmsgs_ext_t*)arg)->mask);
			break;
		}

		case IOV_SVAL(IOV_EVENT_MSGS_EXT):
			if (((eventmsgs_ext_t*)arg)->ver != EVENTMSGS_VER) {
				err = BCME_VERSION;
				break;
			}
			if (len < (int)((EVENTMSGS_EXT_STRUCT_SIZE +
				((eventmsgs_ext_t*)arg)->len))) {
					err = BCME_BUFTOOSHORT;
					break;
			}
			err = wlc_eventq_register_ind_ext(eq, (eventmsgs_ext_t*)arg,
				((eventmsgs_ext_t*)arg)->mask);
			break;

		default: {
			err = BCME_UNSUPPORTED;
			break;
		}
	}
	return err;
}

int
BCMUNINITFN(wlc_eventq_down)(wlc_eventq_t *eq)
{
	int callbacks = 0;
	if (eq->tpending && !eq->workpending) {
		if (!wl_del_timer(eq->wl, eq->timer))
			callbacks++;

		ASSERT(wlc_eventq_avail(eq) == TRUE);
		ASSERT(eq->workpending == FALSE);
		eq->workpending = TRUE;
		if (eq->cb)
			eq->cb(eq->wlc);

		ASSERT(eq->workpending == TRUE);
		eq->workpending = FALSE;
		eq->tpending = FALSE;
	}
	else {
		ASSERT(eq->workpending || wlc_eventq_avail(eq) == FALSE);
	}
	return callbacks;
}

wlc_event_t*
wlc_event_alloc(wlc_eventq_t *eq)
{
	wlc_event_t *e;

	e = (wlc_event_t *) bcm_mp_alloc(eq->mpool_h);

	if (e == NULL)
		return NULL;

	bzero(e, sizeof(wlc_event_t));
	return e;
}

void
wlc_event_free(wlc_eventq_t *eq, wlc_event_t *e)
{
	ASSERT(e->data == NULL);
	ASSERT(e->next == NULL);
	bcm_mp_free(eq->mpool_h, e);
}

void
wlc_eventq_enq(wlc_eventq_t *eq, wlc_event_t *e)
{
	ASSERT(e->next == NULL);
	e->next = NULL;

	if (eq->tail) {
		eq->tail->next = e;
		eq->tail = e;
	}
	else
		eq->head = eq->tail = e;

	if (!eq->tpending) {
		eq->tpending = TRUE;
		/* Use a zero-delay timer to trigger
		 * delayed processing of the event.
		 */
		wl_add_timer(eq->wl, eq->timer, 0, 0);
	}
}

wlc_event_t*
wlc_eventq_deq(wlc_eventq_t *eq)
{
	wlc_event_t *e;

	e = eq->head;
	if (e) {
		eq->head = e->next;
		e->next = NULL;

		if (eq->head == NULL)
			eq->tail = eq->head;
	}
	else if (eq->tpending) {
		/* Timer might have been started within event/timeout handlers,
		 * but, since all the events are processed and event queue
		 * is empty delete the pending timer
		 */
		wl_del_timer(eq->wl, eq->timer);
		eq->tpending = FALSE;
	}

	return e;
}

wlc_event_t*
wlc_eventq_next(wlc_eventq_t *eq, wlc_event_t *e)
{
#ifdef BCMDBG
	wlc_event_t *etmp;

	for (etmp = eq->head; etmp; etmp = etmp->next) {
		if (etmp == e)
			break;
	}
	ASSERT(etmp != NULL);
#endif // endif

	return e->next;
}

int
wlc_eventq_cnt(wlc_eventq_t *eq)
{
	wlc_event_t *etmp;
	int cnt = 0;

	for (etmp = eq->head; etmp; etmp = etmp->next)
		cnt++;

	return cnt;
}

bool
wlc_eventq_avail(wlc_eventq_t *eq)
{
	return (eq->head != NULL);
}

#ifndef WLNOEIND
int
wlc_eventq_register_ind_ext(wlc_eventq_t *eq, eventmsgs_ext_t* iovar_msg, uint8 *mask)
{
	int i;
	int current_event_mask_size;

	/*  re-using the event_msgs_ext iovar struct for convenience, */
	/*	but only using some fields -- */
	/*	if changed remember to check callers */

	current_event_mask_size = MIN(eq->event_inds_mask_len, iovar_msg->len);

	switch (iovar_msg->command) {
		case EVENTMSGS_SET_BIT:
			for (i = 0; i < current_event_mask_size; i++)
				eq->event_inds_mask[i] |= mask[i];
			break;
		case EVENTMSGS_RESET_BIT:
			for (i = 0; i < current_event_mask_size; i++)
				eq->event_inds_mask[i] &= mask[i];
			break;
		case EVENTMSGS_SET_MASK:
			bcopy(mask, eq->event_inds_mask, current_event_mask_size);
			break;
		default:
			return BCME_BADARG;
	};

	wlc_enable_probe_req(
		eq->wlc,
		PROBE_REQ_EVT_MASK,
		wlc_eventq_test_ind(eq, WLC_E_PROBREQ_MSG)? PROBE_REQ_EVT_MASK:0);
#if defined(MSGTRACE) || defined(LOGTRACE)
	if (isset(eq->event_inds_mask, WLC_E_TRACE)) {
#ifdef MSGTRACE
		msgtrace_start();
#endif // endif
#ifdef LOGTRACE
		logtrace_start();
#endif // endif
	} else {
#ifdef MSGTRACE
		msgtrace_stop();
#endif // endif
#ifdef LOGTRACE
		logtrace_stop();
#endif // endif
	}
#endif /* MSGTRACE || LOGTRACE */

	return 0;
}

int
wlc_eventq_query_ind_ext(wlc_eventq_t *eq, eventmsgs_ext_t* in_iovar_msg,
	eventmsgs_ext_t* out_iovar_msg, uint8 *mask)
{
	out_iovar_msg->len = MIN(eq->event_inds_mask_len, in_iovar_msg->len);
	out_iovar_msg->maxgetsize = eq->event_inds_mask_len;
	bcopy(eq->event_inds_mask, mask, out_iovar_msg->len);
	return 0;
}

int
wlc_eventq_test_ind(wlc_eventq_t *eq, int et)
{
	return isset(eq->event_inds_mask, et);
}

int
wlc_eventq_handle_ind(wlc_eventq_t *eq, wlc_event_t *e)
{
	wlc_bsscfg_t *cfg;
	struct ether_addr *da;
	struct ether_addr *sa;

	cfg = wlc_bsscfg_find_by_wlcif(eq->wlc, e->wlcif);
	ASSERT(cfg != NULL);

	if (!cfg) {
		WL_ERROR(("wl%d: wlc_eventq_handle_ind: cfg is null\n", eq->wlc->pub->unit));
		return BCME_ERROR;
	}

	da = &cfg->cur_etheraddr;
	sa = &cfg->cur_etheraddr;

	if (wlc_eventq_test_ind(eq, e->event.event_type))
		wlc_event_sendup(eq, e, da, sa, e->data, e->event.datalen);
	return 0;
}

void
wlc_eventq_flush(wlc_eventq_t *eq)
{
	if (eq == NULL)
		return;

	if (eq->cb)
		eq->cb(eq->wlc);
	if (eq->tpending) {
		wl_del_timer(eq->wl, eq->timer);
		eq->tpending = FALSE;
	}
}
#endif /* !WLNOEIND */

/*
 * Local Functions
 */
static void
wlc_timer_cb(void *arg)
{
	struct wlc_eventq* eq = (struct wlc_eventq*)arg;

	ASSERT(eq->tpending == TRUE);
	ASSERT(wlc_eventq_avail(eq) == TRUE);
	ASSERT(eq->workpending == FALSE);
	eq->workpending = TRUE;
	eq->tpending = FALSE;

	if (eq->cb)
		eq->cb(eq->wlc);

	ASSERT(wlc_eventq_avail(eq) == FALSE);
	ASSERT(eq->tpending == FALSE);
	eq->workpending = FALSE;
}

#ifndef WLNOEIND
/* Abandonable helper function for PROP_TXSTATUS */
static void
wlc_event_mark_packet(wlc_info_t *wlc, void *p)
{
#ifdef PROP_TXSTATUS
	if (PROP_TXSTATUS_ENAB(wlc->pub)) {
		PKTSETTYPEEVENT(wlc->pub->osh, p);
		/* this is implied for event packets anyway */
		PKTSETNODROP(wlc->pub->osh, p);
	}
#endif // endif
}

void
wlc_assign_event_msg(wlc_info_t *wlc, wl_event_msg_t *msg, const wlc_event_t *e,
	uint8 *data, uint32 len)
{
	void *databuf;

	ASSERT(msg && e);

	/* translate the wlc event into bcm event msg */
	msg->version = hton16(BCM_EVENT_MSG_VERSION);
	msg->event_type = hton32(e->event.event_type);
	msg->status = hton32(e->event.status);
	msg->reason = hton32(e->event.reason);
	msg->auth_type = hton32(e->event.auth_type);
	msg->datalen = hton32(e->event.datalen);
	msg->flags = hton16(e->event.flags);
	bzero(msg->ifname, sizeof(msg->ifname));
	strncpy(msg->ifname, e->event.ifname, sizeof(msg->ifname) - 1);
	msg->ifidx = e->event.ifidx;
	msg->bsscfgidx = e->event.bsscfgidx;

	if (e->addr)
		bcopy(e->event.addr.octet, msg->addr.octet, ETHER_ADDR_LEN);

	databuf = (char *)(msg + 1);
	if (len)
		bcopy(data, databuf, len);
}

static void
wlc_event_sendup(wlc_eventq_t *eq, const wlc_event_t *e,
	struct ether_addr *da, struct ether_addr *sa, uint8 *data, uint32 len)
{
	wlc_info_t *wlc = eq->wlc;
	void *p;
	char *ptr;
	bcm_event_t *msg;
	uint pktlen;
	wlc_bsscfg_t *cfg;
	struct scb *scb = NULL;

	BCM_REFERENCE(wlc);

	ASSERT(e != NULL);
	ASSERT(e->wlcif != NULL);

#if defined(EXT_STA) && !defined(DONGLEBUILD)
	if (WLEXTSTA_ENAB(wlc->pub)) {
		wl_event_sendup(eq->wl, e, data, len);
		return;
	}
#endif // endif

	pktlen = sizeof(bcm_event_t) + len + 2;
#ifdef DONGLEBUILD
	pktlen += (BCMEXTRAHDROOM + 2);
#endif // endif
	if ((p = PKTGET(wlc->osh, pktlen, FALSE)) == NULL) {
		WL_ERROR(("wl%d: wlc_event_sendup: failed to get a pkt\n", wlc->pub->unit));
		return;
	}

	ASSERT(ISALIGNED(PKTDATA(wlc->osh, p), sizeof(uint32)));

#ifdef DONGLEBUILD
	/* make room for headers; ensure we start on an odd 16 bit offset */
	PKTPULL(wlc->osh, p, BCMEXTRAHDROOM + 2);
#endif // endif
	msg = (bcm_event_t *) PKTDATA(wlc->osh, p);

	bcopy(da, &msg->eth.ether_dhost, ETHER_ADDR_LEN);
	bcopy(sa, &msg->eth.ether_shost, ETHER_ADDR_LEN);

	/* Set the locally administered bit on the source mac address if both
	 * SRC and DST mac addresses are the same. This prevents the downstream
	 * bridge from dropping the packet.
	 * Clear it if both addresses are the same and it's already set.
	 */
	if (!bcmp(&msg->eth.ether_shost, &msg->eth.ether_dhost, ETHER_ADDR_LEN))
		ETHER_TOGGLE_LOCALADDR(&msg->eth.ether_shost);

	msg->eth.ether_type = hton16(ETHER_TYPE_BRCM);

	/* BCM Vendor specific header... */
	msg->bcm_hdr.subtype = hton16(BCMILCP_SUBTYPE_VENDOR_LONG);
	msg->bcm_hdr.version = BCMILCP_BCM_SUBTYPEHDR_VERSION;
	bcopy(BRCM_OUI, &msg->bcm_hdr.oui[0], DOT11_OUI_LEN);
	/* vendor spec header length + pvt data length (private indication
	 * hdr + actual message itself)
	 */
	msg->bcm_hdr.length = hton16(BCMILCP_BCM_SUBTYPEHDR_MINLENGTH +
	                             BCM_MSG_LEN +
	                             (uint16)len);
	msg->bcm_hdr.usr_subtype = hton16(BCMILCP_BCM_SUBTYPE_EVENT);

	/* update the event struct */
	wlc_assign_event_msg(wlc, &msg->event, e, data, len);

	/* fixup lengths */
	msg->bcm_hdr.length = ntoh16(msg->bcm_hdr.length);
	msg->bcm_hdr.length += sizeof(wl_event_msg_t);
	msg->bcm_hdr.length = hton16(msg->bcm_hdr.length);

	PKTSETLEN(wlc->osh, p, (sizeof(bcm_event_t) + len + 2));

	ptr = (char *)(msg + 1);
	/* Last 2 bytes of the message are 0x00 0x00 to signal that there are
	 * no ethertypes which are following this
	 */
	ptr[len + 0] = 0x00;
	ptr[len + 1] = 0x00;

	wlc_event_mark_packet(wlc, p);

	cfg = wlc_bsscfg_find_by_wlcif(wlc, e->wlcif);
	ASSERT(cfg != NULL);

	if (e->wlcif->type == WLC_IFTYPE_WDS)
		scb = e->wlcif->u.scb;

	wlc_sendup(wlc, cfg, scb, p);
}

#if defined(MSGTRACE) || defined(LOGTRACE)
#include <rte_dev.h>
void
wlc_event_sendup_trace(wlc_info_t *wlc, hnd_dev_t *wl_rtedev, uint8 *hdr, uint16 hdrlen,
                       uint8 *buf, uint16 buflen)
{
	void *p;
	bcm_event_t *msg;
	char *ptr, *databuf;
	struct lbuf *lb;
	uint16 len;
	osl_t *osh = wlc->osh;
	hnd_dev_t *busdev = wl_rtedev->chained;

	if (busdev == NULL)
		return;

	if (! wlc_eventq_test_ind(wlc->eventq, WLC_E_TRACE))
		return;

	len = hdrlen + buflen;
	ASSERT(len < (wlc->pub->tunables->rxbufsz - sizeof(bcm_event_t) - 2));

	if ((p = PKTGET(osh, wlc->pub->tunables->rxbufsz, FALSE)) == NULL) {
		return;
	}

	ASSERT(ISALIGNED(PKTDATA(osh, p), sizeof(uint32)));

	/* make room for headers; ensure we start on an odd 16 bit offset */
	PKTPULL(osh, p, BCMEXTRAHDROOM + 2);

	msg = (bcm_event_t *) PKTDATA(osh, p);

	msg->eth.ether_type = hton16(ETHER_TYPE_BRCM);

	/* BCM Vendor specific header... */
	msg->bcm_hdr.subtype = hton16(BCMILCP_SUBTYPE_VENDOR_LONG);
	msg->bcm_hdr.version = BCMILCP_BCM_SUBTYPEHDR_VERSION;
	bcopy(BRCM_OUI, &msg->bcm_hdr.oui[0], DOT11_OUI_LEN);
	/* vendor spec header length + pvt data length (private indication hdr + actual message
	 * itself)
	 */
	msg->bcm_hdr.length = hton16(BCMILCP_BCM_SUBTYPEHDR_MINLENGTH + BCM_MSG_LEN + (uint16)len);
	msg->bcm_hdr.usr_subtype = hton16(BCMILCP_BCM_SUBTYPE_EVENT);

	PKTSETLEN(osh, p, (sizeof(bcm_event_t) + len + 2));

	/* update the event struct */
	/* translate the wlc event into bcm event msg */
	msg->event.version = hton16(BCM_EVENT_MSG_VERSION);
	msg->event.event_type = hton32(WLC_E_TRACE);
	msg->event.status = hton32(WLC_E_STATUS_SUCCESS);
	msg->event.reason = 0;
	msg->event.auth_type = 0;
	msg->event.datalen = hton32(len);
	msg->event.flags = 0;
	msg->event.addr = wlc->perm_etheraddr;
	bzero(msg->event.ifname, sizeof(msg->event.ifname));
	msg->event.ifidx = 0;
	msg->event.flags |= WLC_EVENT_MSG_UNKIF;
	msg->event.bsscfgidx = 0;
	msg->event.flags |= WLC_EVENT_MSG_UNKBSS;

	/* fixup lengths */
	msg->bcm_hdr.length = ntoh16(msg->bcm_hdr.length);
	msg->bcm_hdr.length += sizeof(wl_event_msg_t);
	msg->bcm_hdr.length = hton16(msg->bcm_hdr.length);

	PKTSETLEN(osh, p, (sizeof(bcm_event_t) + len + 2));

	/* Copy the data */
	databuf = (char *)(msg + 1);
	bcopy(hdr, databuf, hdrlen);
	bcopy(buf, databuf+hdrlen, buflen);

	ptr = (char *)databuf;

	PKTSETMSGTRACE(p, TRUE);

	/* Last 2 bytes of the message are 0x00 0x00 to signal that there are no ethertypes which
	 * are following this
	 */
	ptr[len+0] = 0x00;
	ptr[len+1] = 0x00;
	lb = PKTTONATIVE(osh, p);

	if (busdev->ops->xmit(NULL, busdev, lb) != 0) {
		lb_free(lb);
	}
}
#endif /* MSGTRACE */

int
wlc_eventq_set_ind(wlc_eventq_t* eq, uint et, bool enab)
{
	if (et >= WLC_E_LAST)
		return -1;
	if (enab)
		setbit(eq->event_inds_mask, et);
	else
		clrbit(eq->event_inds_mask, et);

	if (et == WLC_E_PROBREQ_MSG)
		wlc_enable_probe_req(eq->wlc, PROBE_REQ_EVT_MASK, enab? PROBE_REQ_EVT_MASK:0);

	return 0;
}
#endif /* !WLNOEIND */
