/*
 * Wrapper to scb power selection algorithm of Broadcom
 * 802.11 Networking Adapter Device Driver.
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
 * $Id: wlc_scb_powersel.c 708017 2017-06-29 14:11:45Z $
 */

/**
 * @file
 * @brief
 * Link Margin Transmit Power Control feature
 * Without this feature, for transmitting MPDUs the nominal power is used regardless of the good
 * link quality. There is a potential to conserve power and reduce the BSS size (radio range), in
 * case the current rate can be maintained with lower transmit power.
 */

/**
 * @file
 * @brief
 * XXX Twiki: [LinkPowerControl]
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
#include <bcmendian.h>
#include <wlioctl.h>

#include <proto/802.11.h>
#include <d11.h>

#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_bsscfg.h>
#include <wlc.h>

#include <wlc_scb.h>
#include <wlc_phy_hal.h>
#include <wlc_antsel.h>
#include <wlc_scb_powersel.h>
#include <wlc_scb_ratesel.h>

#include <wl_dbg.h>

struct wlc_lpc_info {
	wlc_info_t		*wlc;		/* pointer to main wlc structure */
	wlc_pub_t		*pub;		/* public common code handler */
	osl_t			*osh;		/* OSL handler */
	lpc_info_t 		*lpci;
	int32 			scb_handle;
	int32 			cubby_sz;
};

struct lpc_cubby {
	lcb_t *scb_cubby;
};

typedef struct lpc_cubby lpc_cubby_t;

#define SCB_LPC_INFO(wlpci, scb) ((SCB_CUBBY((scb), (wlpci)->scb_handle)))

#if defined(WME_PER_AC_TX_PARAMS)
#define SCB_LPC_CUBBY(wlpci, scb, ac) 	\
	((void *)(((char*)((lpc_cubby_t *)SCB_LPC_INFO(wlpci, scb))->scb_cubby) + \
		(ac * (wlpci)->cubby_sz)))
#else /* WME_PER_AC_TX_PARAMS */
#define SCB_LPC_CUBBY(wlpci, scb, ac)	\
	(((lpc_cubby_t *)SCB_LPC_INFO(wlpci, scb))->scb_cubby)
#endif /* WME_PER_AC_TX_PARAMS */

#ifdef BCMDBG
void wlc_scb_lpc_dump_scb(void *ctx, struct scb *scb, struct bcmstrbuf *b);
#endif // endif

static int wlc_scb_lpc_scb_init(void *context, struct scb *scb);
static void wlc_scb_lpc_scb_deinit(void *context, struct scb *scb);
static lcb_t *wlc_scb_lpc_get_cubby(wlc_lpc_info_t *wlpci,
	struct scb *scb, uint8 ac);
static int wlc_scb_lpc_cubby_sz(void);

wlc_lpc_info_t *
BCMATTACHFN(wlc_scb_lpc_attach)(wlc_info_t *wlc)
{
	wlc_lpc_info_t *wlpci;
	if (!(wlpci = (wlc_lpc_info_t *)MALLOC(wlc->osh, sizeof(wlc_lpc_info_t)))) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		return NULL;
	}

	bzero((char *)wlpci, sizeof(wlc_lpc_info_t));
	wlpci->wlc = wlc;
	wlpci->pub = wlc->pub;
	wlpci->osh = wlc->osh;

	if ((wlpci->lpci = wlc_lpc_attach(wlc)) == NULL) {
		WL_ERROR(("%s: failed\n", __FUNCTION__));
		goto fail;
	}

	/* if PHY type & Chip are not supported then do not do cuby reserve, just return */
	if (!wlc_lpc_capable_chip(wlc)) {
		wlc->pub->_lpc_algo = FALSE;
		return wlpci;
	}

	/* reserve cubby in the scb container for per-scb-ac private data */
	wlpci->scb_handle = wlc_scb_cubby_reserve(wlc, wlc_scb_lpc_cubby_sz(),
	                                        wlc_scb_lpc_scb_init,
	                                        wlc_scb_lpc_scb_deinit,
#ifdef BCMDBG
	                                        wlc_scb_lpc_dump_scb,
#else
	                                        NULL,
#endif // endif
	                                        (void *) wlpci);

	if (wlpci->scb_handle < 0) {
		WL_ERROR(("wl%d: %s:wlc_scb_cubby_reserve failed\n", wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	wlpci->cubby_sz = wlc_lpc_lcb_sz();
	return wlpci;

fail:
	if (wlpci->lpci)
		wlc_lpc_detach(wlpci->lpci);

	MFREE(wlc->osh, wlpci, sizeof(wlc_lpc_info_t));
	return NULL;
}

void
BCMATTACHFN(wlc_scb_lpc_detach)(wlc_lpc_info_t *wlpci)
{
	if (!wlpci)
		return;

	/* No need to disable the algo state, as detaching. Just detach and free */
	wlc_lpc_detach(wlpci->lpci);
	MFREE(wlpci->pub->osh, wlpci, sizeof(wlc_lpc_info_t));
}

/* alloc per ac cubby space on scb attach. */
static int
wlc_scb_lpc_scb_init(void *context, struct scb *scb)
{
	wlc_lpc_info_t *wlpci = (wlc_lpc_info_t *) context;
	lpc_cubby_t *cubby_info = NULL;
	lcb_t *scb_power_cubby = NULL;
	int cubby_size;

	ASSERT(scb);

	cubby_info = SCB_LPC_INFO(wlpci, scb);

#if defined(WME_PER_AC_TX_PARAMS)
	cubby_size = AC_COUNT * wlpci->cubby_sz;
#else
	cubby_size = wlpci->cubby_sz;
#endif // endif

	if (scb && !SCB_INTERNAL(scb)) {
		scb_power_cubby = (lcb_t *)MALLOC(wlpci->osh, cubby_size);
		if (!scb_power_cubby) {
			return BCME_NOMEM;
		}
		bzero(scb_power_cubby, cubby_size);
		cubby_info->scb_cubby = scb_power_cubby;
	}
	return BCME_OK;
}

/* free cubby space after scb detach */
static void
wlc_scb_lpc_scb_deinit(void *context, struct scb *scb)
{
	wlc_lpc_info_t *wlpci = (wlc_lpc_info_t *) context;
	lpc_cubby_t *cubby_info = NULL;
	int cubby_size;

	ASSERT(scb);

	cubby_info = SCB_LPC_INFO(wlpci, scb);

#if defined(WME_PER_AC_TX_PARAMS)
	cubby_size = (AC_COUNT * wlpci->cubby_sz);
#else
	cubby_size = wlpci->cubby_sz;
#endif // endif

	if (wlpci->wlc && cubby_info && !SCB_INTERNAL(scb) && cubby_info->scb_cubby) {
		MFREE(wlpci->osh, cubby_info->scb_cubby, cubby_size);
		cubby_info->scb_cubby = NULL;
	}
	return;
}

#ifdef BCMDBG
void
wlc_scb_lpc_dump_scb(void *ctx, struct scb *scb, struct bcmstrbuf *b)
{
	wlc_lpc_info_t *wlpci = (wlc_lpc_info_t *)ctx;
	int32 ac;
	lcb_t *lcb;

	ASSERT(scb);

	for (ac = 0; ac < WME_MAX_AC(wlpci->wlc, scb); ac++) {
		lcb = SCB_LPC_CUBBY(wlpci, scb, ac);
		wlc_lpc_dump_lcb(lcb, ac, b);
	}
}
#endif /* BCMDBG */

static lcb_t *
wlc_scb_lpc_get_cubby(wlc_lpc_info_t *wlpci, struct scb *scb, uint8 ac)
{
	ASSERT(wlpci);
	ASSERT(scb);

	ac = (WME_PER_AC_MAXRATE_ENAB(wlpci->pub) ? ac : 0);
	return (SCB_LPC_CUBBY(wlpci, scb, ac));
}

static int
wlc_scb_lpc_cubby_sz(void)
{
	return (sizeof(lpc_cubby_t));
}

/* External Functions */
/* initialize per-scb state utilized by LPC */
void
wlc_scb_lpc_init(wlc_lpc_info_t *wlpci, struct scb *scb)
{
	int32 ac;
	lcb_t *state;

	ASSERT(wlpci);
	ASSERT(scb);

	if (SCB_INTERNAL(scb) || !wlc_lpc_capable_chip(wlpci->wlc))
		return;

	for (ac = 0; ac < WME_MAX_AC(wlpci->wlc, scb); ac++) {
		state = SCB_LPC_CUBBY(wlpci, scb, ac);
		wlc_lpc_init(wlpci->lpci, state, scb);
	}
}

void
wlc_scb_lpc_init_all(wlc_lpc_info_t *wlpci)
{
	int idx;
	wlc_info_t *wlc = wlpci->wlc;
	wlc_bsscfg_t *cfg;

	ASSERT(wlpci);

	FOREACH_BSS(wlc, idx, cfg)
		wlc_scb_lpc_init_bss(wlpci, cfg);
	return;
}

void
wlc_scb_lpc_init_bss(wlc_lpc_info_t *wlpci, wlc_bsscfg_t *bsscfg)
{
	struct scb *scb;
	struct scb_iter scbiter;
	wlc_info_t *wlc = wlpci->wlc;

	FOREACH_BSS_SCB(wlc->scbstate, &scbiter, bsscfg, scb)
		wlc_scb_lpc_init(wlpci, scb);
}

uint8
wlc_scb_lpc_getcurrpwr(wlc_lpc_info_t *wlpci, struct scb *scb, uint8 ac)
{
	lcb_t *state = NULL;

	ASSERT(wlpci);
	ASSERT(scb);

	state = wlc_scb_lpc_get_cubby(wlpci, scb, ac);
	return wlc_lpc_getcurrpwr(state);
}

void
wlc_scb_lpc_update_pwr(wlc_lpc_info_t *wlpci, struct scb *scb, uint8 ac,
	uint16 PhyTxControlWord0, uint16 PhyTxControlWord1)
{
	lcb_t *state = NULL;
	wlc_info_t *wlc = wlpci->wlc;
	uint16 phy_ctrl_word;

	if (!scb)
		return;

	/* Find the PhyTxControl Word containing the rate information. The PhyTxControl word
	 * is compared in the wlc_lpc_update_pwr() with the last value to note any rate change.
	 */
	if (WLCISACPHY(wlc->band))
		phy_ctrl_word = PhyTxControlWord1;
	else if (WLCISLCN40PHY(wlc->band))
		phy_ctrl_word = PhyTxControlWord0;
	else
		phy_ctrl_word = 0;

	state = wlc_scb_lpc_get_cubby(wlpci, scb, ac);
	wlc_lpc_update_pwr(state, ac, phy_ctrl_word);
	return;
}

void
wlc_scb_lpc_store_pcw(wlc_lpc_info_t *wlpci, struct scb *scb, uint8 ac,
	uint16 phy_ctrl_word)
{
	lcb_t *state = NULL;

	ASSERT(scb);
	ASSERT(wlpci);
	state = wlc_scb_lpc_get_cubby(wlpci, scb, ac);
	wlc_lpc_store_pcw(state, phy_ctrl_word);
	return;
}
