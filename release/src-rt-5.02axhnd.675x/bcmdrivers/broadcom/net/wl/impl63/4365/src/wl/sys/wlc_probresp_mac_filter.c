/**
 * @file
 * @brief
 * MAC based SW probe response module source file -
 * It uses the MAC filter list to influence the decision about
 * which MAC to send SW probe response.
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
 * $Id$
 */

/* XXX: Define wlc_cfg.h to be the first header file included as some builds
 * get their feature flags thru this file.
 */
#include <wlc_cfg.h>

#ifdef WLPROBRESP_MAC_FILTER

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <wlioctl.h>
#include <d11.h>
#include <wlc_pub.h>
#include <wlc.h>
#include <wlc_macfltr.h>
#include <wlc_bsscfg.h>
#include <wlc_probresp.h>
#include <wlc_probresp_mac_filter.h>

/* iovar table */
enum {
	IOV_PROB_RESP_MAC_FILTER = 1,
	IOV_LAST
};

static const bcm_iovar_t prb_iovars[] = {
	{"probresp_mac_filter", IOV_PROB_RESP_MAC_FILTER, (0), IOVT_BOOL, 0},
	{NULL, 0, 0, 0, 0}
};

/* The maximum number of probe responses that may be pending in firmware,
 * to avoid out-of-memory issues
 */
#ifndef MAX_PROBRESP
#define MAX_PROBRESP 32
#elif (MAX_PROBRESP > 32)
#error "MAX_PROBRESP is not properly defined!"
#endif // endif

#define PROB_RESP_EA_HASH(ea) ((((uint8*)(ea))[5] + ((uint8*)(ea))[4] + ((uint8*)(ea))[3] +\
	((uint8*)(ea))[2] + ((uint8*)(ea))[1]) % MAX_PROBRESP)

#define PROB_RESP_EA_CMP(ea1, ea2) \
	((((uint16 *)(ea1))[0] ^ ((uint16 *)(ea2))[0]) | \
	 (((uint16 *)(ea1))[1] ^ ((uint16 *)(ea2))[1]) | \
	 (((uint16 *)(ea1))[2] ^ ((uint16 *)(ea2))[2]))

/* This structure will be used to store ethernet address (destination in probe response) in
 * a linked list
 */
typedef struct prob_resp_ea {
	struct ether_addr mac;
	struct prob_resp_ea *next;
} prob_resp_ea_t;

/* module info */
struct wlc_probresp_mac_filter_info {
	wlc_info_t *wlc;
	int bsscfgh;
	uint32 intransit_inuse; /* number of probe response that are currently in transit */
};

/* bsscfg private states */
typedef struct {
	bool	probresp_mac_filter_mode;
	prob_resp_ea_t *intransit_maclist[MAX_PROBRESP]; /* hash table */
} bss_probresp_mac_filter_info_t;

/* macros to retrieve the pointer to module specific opaque data in bsscfg container */
#define BSS_PRBRSP_MAC_FILTER_CUBBY_LOC(probresp_mac_filter, cfg) \
	((bss_probresp_mac_filter_info_t **)BSSCFG_CUBBY(cfg, (probresp_mac_filter)->bsscfgh))
#define BSS_PRBRSP_MAC_FILTER_INFO(probresp_mac_filter, cfg) \
	(*BSS_PRBRSP_MAC_FILTER_CUBBY_LOC(probresp_mac_filter, cfg))

/* filter function */
static bool wlc_probresp_mac_filter_check_probe_req(void *handle, wlc_bsscfg_t *cfg,
	wlc_d11rxhdr_t *wrxh, uint8 *plcp, struct dot11_management_header *hdr, uint8 *body,
	int body_len, bool *psendProbeResp);
#ifdef WLPROBRESP_INTRANSIT_FILTER
static void wlc_probresp_intransit_filter_clear_bss(
	wlc_probresp_mac_filter_info_t *mprobresp_mac_filter, wlc_bsscfg_t *cfg);
static bool wlc_probresp_intransit_filter_check_probe_req(void *handle, wlc_bsscfg_t *cfg,
	wlc_d11rxhdr_t *wrxh, uint8 *plcp, struct dot11_management_header *hdr,
	uint8 *body, int body_len, bool *psendProbeResp);
static int wlc_probresp_mac_filter_down(void *ctx);
#endif /* WLPROBRESP_INTRANSIT_FILTER */

/* IOVAR management */
static int
wlc_probresp_mac_filter_doiovar(void *ctx, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *params, uint plen, void *arg, int alen, int vsize, struct wlc_if *wlcif);

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
/* register dump routine */
static int wlc_probresp_mac_filter_dump(void *ctx, struct bcmstrbuf *b);
#endif // endif

/* bss cubby */
static int wlc_probresp_mac_filter_bss_init(void *ctx, wlc_bsscfg_t *cfg);
static void wlc_probresp_mac_filter_bss_deinit(void *ctx, wlc_bsscfg_t *cfg);
static void wlc_probresp_mac_filter_bss_dump(void *ctx, wlc_bsscfg_t *cfg, struct bcmstrbuf *b);

/* module entries */
void *
BCMATTACHFN(wlc_probresp_mac_filter_attach)(wlc_info_t *wlc)
{
	wlc_probresp_mac_filter_info_t *mprobresp_mac_filter;

	if ((mprobresp_mac_filter = MALLOCZ(wlc->osh,
		sizeof(wlc_probresp_mac_filter_info_t))) == NULL)
	{
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n", wlc->pub->unit,
			__FUNCTION__, MALLOCED(wlc->osh)));
		goto fail;
	}

	mprobresp_mac_filter->wlc = wlc;

	/* reserve cubby in the bsscfg container for per-bsscfg data */
	if ((mprobresp_mac_filter->bsscfgh = wlc_bsscfg_cubby_reserve(wlc,
		sizeof(bss_probresp_mac_filter_info_t *), wlc_probresp_mac_filter_bss_init,
		wlc_probresp_mac_filter_bss_deinit, wlc_probresp_mac_filter_bss_dump,
		(void *)mprobresp_mac_filter)) < 0) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_cubby_reserve() failed\n", wlc->pub->unit,
			__FUNCTION__));
		goto fail;
	}

	/* register module up/down, watchdog, and iovar callbacks */
	if (wlc_module_register(wlc->pub, prb_iovars, "probresp_mac_filter", mprobresp_mac_filter,
		wlc_probresp_mac_filter_doiovar, NULL, NULL,
#ifdef WLPROBRESP_INTRANSIT_FILTER
		wlc_probresp_mac_filter_down)
#else
		NULL)
#endif /* WLPROBRESP_INTRANSIT_FILTER */
		!= BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_module_register() failed\n", wlc->pub->unit,
			__FUNCTION__));
		goto fail;
	}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
	wlc_dump_register(wlc->pub, "probresp_mac_filter", wlc_probresp_mac_filter_dump,
		mprobresp_mac_filter);
#endif // endif
	if (wlc_probresp_register(wlc->mprobresp, mprobresp_mac_filter,
		wlc_probresp_mac_filter_check_probe_req, FALSE) != 0)
		goto fail;
#ifdef WLPROBRESP_INTRANSIT_FILTER
	if (wlc_probresp_register(wlc->mprobresp, mprobresp_mac_filter,
		wlc_probresp_intransit_filter_check_probe_req, FALSE) != 0) {
		WL_ERROR(("could not register probe response in transit filter!\n"));
		goto fail;
	}
#endif /* WLPROBRESP_INTRANSIT_FILTER */
	return mprobresp_mac_filter;

fail:
	wlc_probresp_mac_filter_detach(mprobresp_mac_filter);
	return NULL;
}

void
BCMATTACHFN(wlc_probresp_mac_filter_detach)(void *mprobresp_mac_filter)
{
	wlc_info_t *wlc;

	if (mprobresp_mac_filter == NULL)
		return;

	wlc = ((wlc_probresp_mac_filter_info_t *)mprobresp_mac_filter)->wlc;

	wlc_probresp_unregister(wlc->mprobresp, mprobresp_mac_filter);
#ifdef WLPROBRESP_INTRANSIT_FILTER
	wlc_probresp_unregister(wlc->mprobresp, mprobresp_mac_filter);
#endif /* WLPROBRESP_INTRANSIT_FILTER */
	wlc_module_unregister(wlc->pub, "probresp_mac_filter", mprobresp_mac_filter);

	MFREE(wlc->osh, mprobresp_mac_filter, sizeof(wlc_probresp_mac_filter_info_t));
}

/* handle related iovars */
static int
wlc_probresp_mac_filter_doiovar(void *ctx, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *params, uint plen, void *arg, int alen, int vsize, struct wlc_if *wlcif)
{
	wlc_probresp_mac_filter_info_t *mprobresp_mac_filter =
		(wlc_probresp_mac_filter_info_t *)ctx;
	wlc_info_t *wlc = mprobresp_mac_filter->wlc;
	bss_probresp_mac_filter_info_t *bpi;
	wlc_bsscfg_t *bsscfg;
	int32 *ret_int_ptr;
	bool bool_val;
	int32 int_val = 0;
	int err = BCME_OK;

	/* update bsscfg w/provided interface context */
	bsscfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif);
	ASSERT(bsscfg != NULL);

	/* convenience int ptr for 4-byte gets (requires int aligned arg) */
	ret_int_ptr = (int32 *)arg;

	if (plen >= (int)sizeof(int_val))
		bcopy(params, &int_val, sizeof(int_val));

	bool_val = (int_val != 0) ? TRUE : FALSE;

	/* update wlcif pointer */
	if (wlcif == NULL)
		wlcif = bsscfg->wlcif;
	ASSERT(wlcif != NULL);

	bpi = BSS_PRBRSP_MAC_FILTER_INFO(mprobresp_mac_filter, bsscfg);
	ASSERT(bpi != NULL);

	/* Do the actual parameter implementation */
	switch (actionid) {
	case IOV_GVAL(IOV_PROB_RESP_MAC_FILTER):
		*ret_int_ptr = bpi->probresp_mac_filter_mode;
		break;

	case IOV_SVAL(IOV_PROB_RESP_MAC_FILTER):
		bpi->probresp_mac_filter_mode = bool_val;
		break;

	default:
		err = BCME_UNSUPPORTED;
		break;
	}

	return err;
}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static int
wlc_probresp_mac_filter_dump(void *ctx, struct bcmstrbuf *b)
{
	wlc_probresp_mac_filter_info_t *mprobresp_mac_filter =
		(wlc_probresp_mac_filter_info_t *)ctx;

	bcm_bprintf(b, "bsscfgh %d \n", mprobresp_mac_filter->bsscfgh);

	return BCME_OK;
}
#endif /* BCMDBG || BCMDBG_DUMP */

static bool
wlc_probresp_mac_filter_check_probe_req(void *handle, wlc_bsscfg_t *cfg,
	wlc_d11rxhdr_t *wrxh, uint8 *plcp, struct dot11_management_header *hdr,
	uint8 *body, int body_len, bool *psendProbeResp)
{
	wlc_probresp_mac_filter_info_t *mprobresp_mac_filter =
		(wlc_probresp_mac_filter_info_t *)handle;
	bss_probresp_mac_filter_info_t *bpi = BSS_PRBRSP_MAC_FILTER_INFO(mprobresp_mac_filter, cfg);
	wlc_info_t *wlc = mprobresp_mac_filter->wlc;
	int addr_match;

	if (bpi->probresp_mac_filter_mode == 0)
		return TRUE;

	addr_match = wlc_macfltr_addr_match(wlc->macfltr, cfg, &hdr->sa);
	if (addr_match == WLC_MACFLTR_ADDR_DENY ||
		addr_match == WLC_MACFLTR_ADDR_NOT_ALLOW) {
		return FALSE;
	}
	return TRUE;
}

static int
wlc_probresp_mac_filter_bss_init(void *ctx, wlc_bsscfg_t *cfg)
{
	wlc_probresp_mac_filter_info_t *mprobresp_mac_filter =
		(wlc_probresp_mac_filter_info_t *)ctx;
	wlc_info_t *wlc = mprobresp_mac_filter->wlc;
	bss_probresp_mac_filter_info_t **pbpi =
		BSS_PRBRSP_MAC_FILTER_CUBBY_LOC(mprobresp_mac_filter, cfg);
	bss_probresp_mac_filter_info_t *bpi;

	if (!(bpi = (bss_probresp_mac_filter_info_t *)MALLOCZ(wlc->osh,
		sizeof(bss_probresp_mac_filter_info_t)))) {
		WL_ERROR(("wl%d: %s: out of memory\n", wlc->pub->unit, __FUNCTION__));
		return BCME_NOMEM;
	}

	*pbpi = bpi;
	return BCME_OK;
}

static void
wlc_probresp_mac_filter_bss_deinit(void *ctx, wlc_bsscfg_t *cfg)
{
	wlc_probresp_mac_filter_info_t *mprobresp_mac_filter =
		(wlc_probresp_mac_filter_info_t *)ctx;
	wlc_info_t *wlc = mprobresp_mac_filter->wlc;
	bss_probresp_mac_filter_info_t **pbpi =
		BSS_PRBRSP_MAC_FILTER_CUBBY_LOC(mprobresp_mac_filter, cfg);
	bss_probresp_mac_filter_info_t *bpi;
	if (pbpi) {
		bpi = *pbpi;
		if (bpi) {
#ifdef WLPROBRESP_INTRANSIT_FILTER
			wlc_probresp_intransit_filter_clear_bss(mprobresp_mac_filter, cfg);
#endif /* WLPROBRESP_INTRANSIT_FILTER */
			MFREE(wlc->osh, bpi, sizeof(bss_probresp_mac_filter_info_t));
			*pbpi = NULL;
		}
	}
}

static void
wlc_probresp_mac_filter_bss_dump(void *ctx, wlc_bsscfg_t *cfg, struct bcmstrbuf *b)
{
	wlc_probresp_mac_filter_info_t *mprobresp_mac_filter =
		(wlc_probresp_mac_filter_info_t *)ctx;
	bss_probresp_mac_filter_info_t *bpi = BSS_PRBRSP_MAC_FILTER_INFO(mprobresp_mac_filter, cfg);

	ASSERT(cfg != NULL);

	if (bpi == NULL)
		return;

	bcm_bprintf(b, "\tprobresp_mac_filter_mode %u\n", bpi->probresp_mac_filter_mode);
}

#ifdef WLPROBRESP_INTRANSIT_FILTER
/* Add the destination address of the probe response to the intransit hash table.
 * If hash already in use, creates a linked list.
 */
bool
wlc_probresp_intransit_filter_add_da(wlc_probresp_mac_filter_info_t *mprobresp_mac_filter,
	wlc_bsscfg_t *cfg, struct ether_addr *da)
{
	wlc_info_t *wlc = mprobresp_mac_filter->wlc;
	bss_probresp_mac_filter_info_t *bpi = BSS_PRBRSP_MAC_FILTER_INFO(mprobresp_mac_filter, cfg);
	uint8 hash;
	prob_resp_ea_t *stap;

	if ((stap = MALLOC(wlc->osh, sizeof(prob_resp_ea_t))) == NULL) {
		return FALSE;
	}

	eacopy(da, &stap->mac);
	hash = PROB_RESP_EA_HASH(da);
	stap->next = bpi->intransit_maclist[hash];
	bpi->intransit_maclist[hash] = stap;
	mprobresp_mac_filter->intransit_inuse++;

	return TRUE;
}

/* Remove the destination address of the probe response from the intransit hash table.
 * At calculated index traverse linked list to find the address.
 */
void
wlc_probresp_intransit_filter_rem_da(wlc_probresp_mac_filter_info_t *mprobresp_mac_filter,
	wlc_bsscfg_t *cfg, struct ether_addr *da)
{
	wlc_info_t *wlc = mprobresp_mac_filter->wlc;
	bss_probresp_mac_filter_info_t *bpi = BSS_PRBRSP_MAC_FILTER_INFO(mprobresp_mac_filter, cfg);
	uint8 hash;
	prob_resp_ea_t *stap;
	prob_resp_ea_t *p;
	prob_resp_ea_t *prevp;

	hash = PROB_RESP_EA_HASH(da);
	prevp = stap = bpi->intransit_maclist[hash];
	while (stap != NULL) {
		p = stap->next;
		if (!PROB_RESP_EA_CMP(&stap->mac, da)) {
			if (stap == bpi->intransit_maclist[hash]) {
				bpi->intransit_maclist[hash] = p;
			} else {
				prevp->next = p;
			}
			MFREE(wlc->osh, stap, sizeof(prob_resp_ea_t));
			mprobresp_mac_filter->intransit_inuse--;
			break;
		}
		prevp = stap;
		stap = p;
	}
}

/* This function is called on 'wl down' and when bss is removed.
 * When called, this function will clear the intransit table in bsscfg cubby.
 */
static void
wlc_probresp_intransit_filter_clear_bss(wlc_probresp_mac_filter_info_t *mprobresp_mac_filter,
	wlc_bsscfg_t *cfg)
{
	bss_probresp_mac_filter_info_t *bpi = BSS_PRBRSP_MAC_FILTER_INFO(mprobresp_mac_filter, cfg);
	wlc_info_t *wlc = mprobresp_mac_filter->wlc;
	uint8 hash;
	prob_resp_ea_t *stap;
	prob_resp_ea_t *p;

	for (hash = 0; hash < MAX_PROBRESP; hash++) {
		stap = bpi->intransit_maclist[hash];
		while (stap != NULL) {
			p = stap->next;

			MFREE(wlc->osh, stap, sizeof(prob_resp_ea_t));
			mprobresp_mac_filter->intransit_inuse--;
			stap = p;
		}
		bpi->intransit_maclist[hash] = NULL;
	}
}

/* This function is registered as a filter and will be called when a probe request is received.
 * When called, this function will check if the source address from the probe request is present
 * in the intransit hash table.
 * returns FALSE if nr of probe response frames exceeds MAX_PROBRESP or if probe response is
 *	already intransit for this station.
 * returns TRUE when it is allowed to send a probe response frame.
 */
static bool
wlc_probresp_intransit_filter_check_probe_req(void *handle, wlc_bsscfg_t *cfg,
	wlc_d11rxhdr_t *wrxh, uint8 *plcp, struct dot11_management_header *hdr,
	uint8 *body, int body_len, bool *psendProbeResp)
{
	wlc_probresp_mac_filter_info_t *mprobresp_mac_filter =
		(wlc_probresp_mac_filter_info_t *)handle;
	bss_probresp_mac_filter_info_t *bpi = BSS_PRBRSP_MAC_FILTER_INFO(mprobresp_mac_filter, cfg);
	uint8 hash;
	prob_resp_ea_t *stap;
	prob_resp_ea_t *p;

	if (mprobresp_mac_filter->intransit_inuse == MAX_PROBRESP) {
		/* Maximum nr of probe responses are in transit */
		return FALSE;
	}

	if (bpi == NULL) {
		return TRUE;
	}

	hash = PROB_RESP_EA_HASH(&hdr->sa);
	stap = bpi->intransit_maclist[hash];
	while (stap != NULL) {
		p = stap->next;
		if (!PROB_RESP_EA_CMP(&stap->mac, &hdr->sa)) {
			return FALSE;
		}
		stap = p;
	}

	return TRUE;
}

/* This function is called on 'wl down'.
 * Packet callbacks are disabled when going down and destination address can get stuck in
 * the intransit table. To prevent this, clear the inuse counter and the intransit table
 * for each bss.
 */
static int
wlc_probresp_mac_filter_down(void *ctx)
{
	wlc_probresp_mac_filter_info_t *mprobresp_mac_filter =
		(wlc_probresp_mac_filter_info_t *)ctx;
	wlc_info_t *wlc = mprobresp_mac_filter->wlc;
	int idx;
	wlc_bsscfg_t *cfg;

	FOREACH_AP(wlc, idx, cfg) {
		wlc_probresp_intransit_filter_clear_bss(mprobresp_mac_filter, cfg);
	}

	mprobresp_mac_filter->intransit_inuse = 0;

	return 0;
}
#endif /* WLPROBRESP_INTRANSIT_FILTER */
#endif /* WLPROBRESP_MAC_FILTER */
