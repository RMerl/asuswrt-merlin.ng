/**
 * @file
 * @brief
 * packet tx complete callback management module source
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
 * $Id: wlc_pcb.c 708017 2017-06-29 14:11:45Z $
 */

/* XXX: Define wlc_cfg.h to be the first header file included as some builds
 * get their feature flags thru this file.
 */
#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <wlioctl.h>
#include <bcmwpa.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc.h>
#ifdef PROP_TXSTATUS
#include <wlfc_proto.h>
#include <wl_wlfc.h>
#endif // endif

#if defined(BCMCCX) && defined(CCX_SDK)
#include <wlc_rm.h>
#include <wlc_ccx.h>
#endif // endif

#include <wlc_pcb.h>

/* packet callback descriptor */
typedef struct pkt_cb {
	pkcb_fn_t fn;		/* function to call when tx frame completes */
	void *arg;		/* void arg for fn */
	uint8 nextidx;		/* index of next call back if threading */
	bool entered;		/* recursion check */
} pkt_cb_t;

typedef struct pktfree_cb {
	wlc_pcb_pktfree_fn_t fn;		/* function to call when tx frame completes */
	void *ctxt;
	bool entered;		/* recursion check */
} pktfree_cb_t;

/* packet class callback descriptor */
typedef struct {
	wlc_pcb_fn_t *cb;	/* callback function table */
	uint8 size;		/* callback function table size */
	uint8 offset;		/* index byte offset in wlc_pkttag_t */
	uint8 mask;		/* mask in above byte */
	uint8 shift;		/* shift in above byte */
} wlc_pcb_cd_t;

/* module states */
struct wlc_pcb_info {
	wlc_info_t *wlc;
	int maxpktcb;
	pkt_cb_t *pkt_callback;	/* tx completion callback handlers */
	int maxpcbcds;
	wlc_pcb_cd_t *pcb_cd;	/* packet class callback registry */
	pktfree_cb_t* pktfree_callback;	/* pktfree callback */
};

/* packet class callback descriptor template - freed after attach! */
static wlc_pcb_cd_t BCMATTACHDATA(pcb_cd_def)[] = {
	{NULL, MAXCD1PCBS, OFFSETOF(wlc_pkttag_t, flags2), WLF2_PCB1_MASK, WLF2_PCB1_SHIFT},
	{NULL, MAXCD2PCBS, OFFSETOF(wlc_pkttag_t, flags2), WLF2_PCB2_MASK, WLF2_PCB2_SHIFT},
	{NULL, MAXCD3PCBS, OFFSETOF(wlc_pkttag_t, flags2), WLF2_PCB3_MASK, WLF2_PCB3_SHIFT},
	{NULL, MAXCD4PCBS, OFFSETOF(wlc_pkttag_t, flags2), WLF2_PCB4_MASK, WLF2_PCB4_SHIFT},
};

enum {
	WLF2_PCB1_INDEX = 0,
	WLF2_PCB2_INDEX,
	WLF2_PCB3_INDEX,
	WLF2_PCB4_INDEX,
	WLF2_PCBMAX_INDEX
};

/* local functions */
/* module entries */
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static int wlc_pcb_dump(void *ctx, struct bcmstrbuf *b);
#endif // endif

/* Noop packet callback, used to abstract cancellled callbacks. */
static void wlc_pktcb_noop_fn(wlc_info_t *wlc, uint txs, void *arg);

/* callback invocation */
static void wlc_pcb_callback(wlc_pcb_info_t *pcbi, void *pkt, uint txs);
static void wlc_pcb_invoke(wlc_pcb_info_t *pcbi, void *pkt, uint txs);

static void _wlc_pcb_fn_invoke(void *ctx, void *pkt, uint txs);
static void _wlc_pcb_pktfree_fn_invoke(void *ctx, void *pkt, uint txs);

static void BCMFASTPATH
wlc_pktcb_noop_fn(wlc_info_t *wlc, uint txs, void *arg)
{
	return; /* noop */
}

/* module entries */
wlc_pcb_info_t *
BCMATTACHFN(wlc_pcb_attach)(wlc_info_t *wlc)
{
	wlc_pcb_info_t *pcbi;
	int i;

	/* module states */
	if ((pcbi = MALLOCZ(wlc->osh, sizeof(wlc_pcb_info_t))) == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
		          wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		goto fail;
	}
	pcbi->wlc = wlc;

	/* packet callbacks array */
	pcbi->pkt_callback = (pkt_cb_t *)
	        MALLOCZ(wlc->osh, (sizeof(pkt_cb_t) * (wlc->pub->tunables->maxpktcb + 1)));

	if (pcbi->pkt_callback == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
		          wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		goto fail;
	}
	pcbi->maxpktcb = wlc->pub->tunables->maxpktcb;

	/* support one pktfree callback for now */
	pcbi->pktfree_callback = (pktfree_cb_t *)
	        MALLOCZ(wlc->osh, (sizeof(pktfree_cb_t)));
	if (pcbi->pktfree_callback == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
		          wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		goto fail;
	}

	/* packet class callback class descriptors array */
	ASSERT(wlc->pub->tunables->maxpcbcds == ARRAYSIZE(pcb_cd_def));
	pcbi->maxpcbcds = wlc->pub->tunables->maxpcbcds;
	if ((pcbi->pcb_cd = MALLOC(wlc->osh, sizeof(pcb_cd_def))) == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
		          wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		goto fail;
	}
	bcopy(pcb_cd_def, pcbi->pcb_cd, sizeof(pcb_cd_def));
	/* packet class callback descriptors */
	for (i = 0; i < pcbi->maxpcbcds; i++) {
		pcbi->pcb_cd[i].cb = (wlc_pcb_fn_t *)
		        MALLOCZ(wlc->osh, sizeof(wlc_pcb_fn_t) * pcb_cd_def[i].size);

		if (pcbi->pcb_cd[i].cb == NULL) {
			WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			          wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
			goto fail;
		}
	}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
	wlc_dump_register(wlc->pub, "pcb", wlc_pcb_dump, (void *)pcbi);
#endif // endif

	/* Register with OSL the callback to be called when PKTFREE is called (w/o tx attempt) */
	PKTFREESETCB(wlc->osh, _wlc_pcb_pktfree_fn_invoke, pcbi);

	return pcbi;

fail:
	wlc_pcb_detach(pcbi);
	return NULL;
}

void
BCMATTACHFN(wlc_pcb_detach)(wlc_pcb_info_t *pcbi)
{
	wlc_info_t *wlc;
	int i;

	if (pcbi == NULL)
		return;

	wlc = pcbi->wlc;

#ifdef BCMDBG
	/* Since all the packets should have been freed,
	 * all callbacks should have been called
	 */
	for (i = 1; i <= pcbi->maxpktcb; i++)
		ASSERT(pcbi->pkt_callback[i].fn == NULL);
#endif // endif

	if (pcbi->pkt_callback != NULL) {
		MFREE(wlc->osh, pcbi->pkt_callback, sizeof(pkt_cb_t) * (pcbi->maxpktcb + 1));
	}
	if (pcbi->pktfree_callback != NULL) {
		MFREE(wlc->osh, pcbi->pktfree_callback, sizeof(pktfree_cb_t));
	}
	if (pcbi->pcb_cd != NULL) {
		for (i = 0; i < pcbi->maxpcbcds; i ++) {
			if (pcbi->pcb_cd[i].cb == NULL)
				continue;
			MFREE(wlc->osh, pcbi->pcb_cd[i].cb,
			      sizeof(wlc_pcb_fn_t) * pcbi->pcb_cd[i].size);
		}
		MFREE(wlc->osh, pcbi->pcb_cd, sizeof(pcb_cd_def));
	}

	MFREE(wlc->osh, pcbi, sizeof(wlc_pcb_info_t));
}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static int
wlc_pcb_dump(void *ctx, struct bcmstrbuf *b)
{
	wlc_pcb_info_t *pcbi = (wlc_pcb_info_t *)ctx;
	int i, j;

	bcm_bprintf(b, "callbacks: %d\n", pcbi->maxpktcb);
	for (i = 1; i <= pcbi->maxpktcb; i ++) {
		bcm_bprintf(b, "\t%d: fn %p arg %p next %d\n", i,
		            pcbi->pkt_callback[i].fn, pcbi->pkt_callback[i].arg,
		            pcbi->pkt_callback[i].nextidx);
	}
	for (i = 0; i < pcbi->maxpcbcds; i ++) {
		bcm_bprintf(b, "class %d callbacks: %d\n", i, pcbi->pcb_cd[i].size);
		bcm_bprintf(b, "class %d pkttag: %d 0x%x %d\n", i,
		            pcbi->pcb_cd[i].offset, pcbi->pcb_cd[i].mask,
		            pcbi->pcb_cd[i].shift);
		for (j = 1; j < pcbi->pcb_cd[i].size; j ++) {
			bcm_bprintf(b, "\t%d: fn %p\n", j, pcbi->pcb_cd[i].cb[j]);
		}
	}

	return BCME_OK;
}
#endif /* BCMDBG || BCMDBG_DUMP */

/* Prepend pkt's callback list to new_pkt. This is useful mainly for A-MSDU */
void
wlc_pcb_fn_move(wlc_pcb_info_t *pcbi, void *new_pkt, void *pkt)
{
	wlc_pkttag_t *pt = WLPKTTAG(pkt);
	uint8 idx = pt->callbackidx;
	wlc_pkttag_t *npt;

	if (idx == 0)
		return;

	/* Go to the end of the chain */
	while (pcbi->pkt_callback[idx].nextidx)
		idx = pcbi->pkt_callback[idx].nextidx;

	npt = WLPKTTAG(new_pkt);
	pcbi->pkt_callback[idx].nextidx = npt->callbackidx;
	npt->callbackidx = pt->callbackidx;
	pt->callbackidx = 0;
}

/* register packet callback */
int
wlc_pcb_fn_register(wlc_pcb_info_t *pcbi, pkcb_fn_t fn, void *arg, void *pkt)
{
	pkt_cb_t *pcb;
	uint8 i;
	wlc_pkttag_t *pt;

	/* find a free entry */
	for (i = 1; i <= pcbi->maxpktcb; i++) {
		if (pcbi->pkt_callback[i].fn == NULL)
			break;
	}

	if (i > pcbi->maxpktcb) {
		WLCNTINCR(pcbi->wlc->pub->_cnt->pkt_callback_reg_fail);
		WL_ERROR(("wl%d: failed to register callback %p\n",
		          pcbi->wlc->pub->unit, fn));
		ASSERT(0);
		return (-1);
	}

	/* alloc and init next free callback struct */
	pcb = &pcbi->pkt_callback[i];
	pcb->fn = fn;
	pcb->arg = arg;
	pcb->entered = FALSE;
	/* Chain this callback */
	pt = WLPKTTAG(pkt);
	pcb->nextidx = pt->callbackidx;
	pt->callbackidx = i;

	return (0);
}

/* Search the callback table for a matching <fn,arg> callback, optionally
 * cancelling all matching callback(s). Return number of matching callbacks.
 */
int BCMFASTPATH
wlc_pcb_fn_find(wlc_pcb_info_t *pcbi, pkcb_fn_t fn, void *arg, bool cancel_all)
{
	int i;
	int matches = 0; /* count of matching callbacks */

	/* search the pktcb array for a matching <fn,arg> entry */
	for (i = 1; i <= pcbi->maxpktcb; i++) {

		pkt_cb_t *pcb = &pcbi->pkt_callback[i];

		if ((pcb->fn == fn) && (pcb->arg == arg)) { /* matching callback */
			matches++;

			if (cancel_all) {
				pcb->fn = wlc_pktcb_noop_fn; /* callback: noop */
				pcb->arg = (void *)pcb; /* self: dummy non zero arg */
			}
		}
	}

	return matches;
}

/* set packet class callback */
int
BCMATTACHFN(wlc_pcb_fn_set)(wlc_pcb_info_t *pcbi, int tbl, int cls, wlc_pcb_fn_t pcb)
{
	if (tbl >= pcbi->maxpcbcds)
		return BCME_NORESOURCE;

	if (cls >= pcbi->pcb_cd[tbl].size)
		return BCME_NORESOURCE;

	pcbi->pcb_cd[tbl].cb[cls] = pcb;
	return BCME_OK;
}

/* Invokes the callback chain attached to 'pkt' */
static void BCMFASTPATH
wlc_pcb_callback(wlc_pcb_info_t *pcbi, void *pkt, uint txs)
{
	wlc_pkttag_t *pt = WLPKTTAG(pkt);
	uint8 idx = pt->callbackidx;
	pkt_cb_t *pcb;

	/* If callbacks */
	if (idx != 0) {
		ASSERT(idx <= pcbi->maxpktcb && pcbi->pkt_callback[idx].fn != NULL);

		if (pcbi->wlc->going_down) {
			/* only clean up bookkeeping  -- going down */
			while (idx != 0 && idx <= pcbi->maxpktcb) {
				pcb = &pcbi->pkt_callback[idx];
				pcb->fn = NULL;
				idx = pcb->nextidx;
				pcb->nextidx = 0;
			}

		} else {
			while (idx != 0 && idx <= pcbi->maxpktcb) {
				pcb = &pcbi->pkt_callback[idx];

				/* ensure the callback is not called recursively */
				ASSERT(pcb->entered == FALSE);
				ASSERT(pcb->fn != NULL);
				pcb->entered = TRUE;
				/* call the function */
				(pcb->fn)(pcbi->wlc, txs, pcb->arg);
				pcb->fn = NULL;
				pcb->entered = FALSE;
				idx = pcb->nextidx;
				pcb->nextidx = 0;
			}
		}
		pt->callbackidx = 0;

	}
}

#define CB_INVOKE(cbnum) \
	do { \
		if (*loc & WLF2_PCB##cbnum##_MASK) { \
			idx = (*loc & WLF2_PCB##cbnum##_MASK) >> WLF2_PCB##cbnum##_SHIFT; \
			(pcbi->pcb_cd[WLF2_PCB##cbnum##_INDEX].cb[idx])(pcbi->wlc, pkt, txs); \
			*loc = (*loc & ~WLF2_PCB##cbnum##_MASK); \
		} \
	} while (0)
/*
 * For example
 * CB_INVOKE(1)
 *	if (*loc & WLF2_PCB1_MASK) {
 *		idx = (*loc & WLF2_PCB1_MASK) >> WLF2_PCB1_SHIFT;
 *		(pcbi->pcb_cd[WLF2_PCB1_INDEX].cb[idx])(pcbi->wlc, pkt, txs);
 *		*loc = (*loc & ~WLF2_PCB1_MASK);
 *	}
 */

/* invokes the packet class callback chain attached to 'pkt' */
static void BCMFASTPATH
wlc_pcb_invoke(wlc_pcb_info_t *pcbi, void *pkt, uint txs)
{
	wlc_pkttag_t *pt;
	uint8 *loc;
	uint idx;

	ASSERT(pcbi->maxpcbcds == WLF2_PCBMAX_INDEX);
	if (pcbi->wlc->going_down) {
		return;
	}
	pt = WLPKTTAG(pkt);
	loc = (uint8 *)pt + OFFSETOF(wlc_pkttag_t, flags2);

	/* If there are no registered callbacks, return early */
	if ((*loc & (WLF2_PCB1_MASK | WLF2_PCB2_MASK | WLF2_PCB3_MASK | WLF2_PCB4_MASK)) == 0) {
		return;
	}

	/* invoke all registered callbacks in the order defined by wlc_pcb_tbl */
	CB_INVOKE(1);
	CB_INVOKE(2);
	CB_INVOKE(3);
	CB_INVOKE(4);
}

/* move the packet class callback chain attached from 'pkt_from' to 'pkt_to' */
void BCMFASTPATH
wlc_pcb_cb_move(wlc_pcb_info_t *pcbi, void *pkt_from, void *pkt_to)
{
	wlc_pkttag_t *pt = WLPKTTAG(pkt_from);
	wlc_pkttag_t *pt_to = WLPKTTAG(pkt_to);
	uint8 idx = pt->callbackidx;
	int tbl;

	/* TBD: pcb->arg is not updated with new pkt pointer */
	if (idx != 0) {
		ASSERT(idx <= pcbi->maxpktcb && pcbi->pkt_callback[idx].fn != NULL);
		pt_to->callbackidx = pt->callbackidx;
		pt->callbackidx = 0;

	}

	/* move packet class callbacks */
	for (tbl = 0; tbl < pcbi->maxpcbcds; tbl ++) {
		wlc_pcb_cd_t *pcb_cd = &pcbi->pcb_cd[tbl];
		uint8 *loc = (uint8 *)pt + pcb_cd->offset;
		uint8 *loc_to = (uint8 *)pt_to + pcb_cd->offset;
		idx = (*loc & pcb_cd->mask) >> pcb_cd->shift;

		/* No callback for this packet. Do nothing */
		if (idx == 0)
			continue;

		/* copy the index */
		*loc_to |= (*loc & pcb_cd->mask);

		/* clear index as it got moved to new pkt */
		*loc = (*loc & ~pcb_cd->mask);
	}
}

static bool BCMFASTPATH
wlc_pkt_cb_ignore(wlc_info_t *wlc, uint txs)
{
	uint supr_indication = (txs & TX_STATUS_SUPR_MASK) >> TX_STATUS_SUPR_SHIFT;

	/* If the frame has been suppressed for deferred delivery, skip the callback
	 * now as it will be attempted later.
	 */

	/* skip power save (PMQ) */
	if (supr_indication == TX_STATUS_SUPR_PMQ) {
		return 1;
	}

#ifdef WLP2P
	/* skip P2P (NACK_ABS) */
	if (P2P_ENAB(wlc->pub) &&
	    supr_indication == TX_STATUS_SUPR_NACK_ABS) {
		return 1;
	}
#endif // endif

	return 0;
}

static void BCMFASTPATH
_wlc_pcb_pktfree_fn_invoke(void *ctx, void *pkt, uint txs)
{
	wlc_pcb_info_t *pcbi = (wlc_pcb_info_t *)ctx;
	if (pcbi->pktfree_callback->fn && !pcbi->pktfree_callback->entered) {
		pcbi->pktfree_callback->entered = TRUE;
		pcbi->pktfree_callback->fn(pcbi->pktfree_callback->ctxt, pkt);
		pcbi->pktfree_callback->entered = FALSE;
	}
	_wlc_pcb_fn_invoke(pcbi, pkt, txs);
}

/* called from either at the end of transmit attempt (success or fail [dotxstatus()]),
 * or when it's freed by OSL [PKTFREE()].
 */
static void BCMFASTPATH
_wlc_pcb_fn_invoke(void *ctx, void *pkt, uint txs)
{
	wlc_pcb_info_t *pcbi = (wlc_pcb_info_t *)ctx;

#ifdef PROP_TXSTATUS
	if (PROP_TXSTATUS_ENAB(pcbi->wlc->pub)) {
		wlc_process_wlhdr_txstatus(pcbi->wlc, WLFC_CTL_PKTFLAG_DISCARD, pkt, FALSE);
	}
#endif /* PROP_TXSTATUS */

#if defined(BCMCCX) && defined(CCX_SDK)
	/* handle the case where the callbacks are via pkt free i.e. not via dotxstatus */
	if ((WLPKTTAG(pkt)->flags & WLF_IHV_TX_PKT) && (pcbi->wlc->ccx->ihv_txpkt == NULL)) {
		pcbi->wlc->ccx->ihv_txpkt = pkt;
		pcbi->wlc->ccx->ihv_txpkt_sent = FALSE;
		pcbi->wlc->ccx->ihv_txpkt_max_retries = FALSE;
	}
#endif // endif

	wlc_pcb_callback(pcbi, pkt, txs);
	wlc_pcb_invoke(pcbi, pkt, txs);
}

void BCMFASTPATH
wlc_pcb_fn_invoke(wlc_pcb_info_t *pcbi, void *pkt, uint txs)
{
	wlc_info_t *wlc = pcbi->wlc;

	/* skip the callback if the frame txs indicates one of the ignorance conditions */
	if (wlc_pkt_cb_ignore(wlc, txs))
		return;

	_wlc_pcb_fn_invoke(pcbi, pkt, txs);
}

int
wlc_pcb_pktfree_cb_register(wlc_pcb_info_t *pcbi, wlc_pcb_pktfree_fn_t fn, void* ctxt)
{
	ASSERT(pcbi->pktfree_callback->fn == NULL);
	ASSERT(pcbi->pktfree_callback->ctxt == NULL);

	pcbi->pktfree_callback->fn = fn;
	pcbi->pktfree_callback->ctxt = ctxt;

	return BCME_OK;
}

int
wlc_pcb_pktfree_cb_unregister(wlc_pcb_info_t *pcbi, wlc_pcb_pktfree_fn_t fn, void* ctxt)
{
	ASSERT(pcbi->pktfree_callback->fn == fn);
	ASSERT(pcbi->pktfree_callback->ctxt == ctxt);
	memset(pcbi->pktfree_callback, 0, sizeof(*(pcbi->pktfree_callback)));
	return BCME_OK;
}
