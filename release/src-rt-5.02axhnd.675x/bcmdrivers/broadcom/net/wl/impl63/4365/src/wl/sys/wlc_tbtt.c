/*
 * TBTT module source - generate TBTT event
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
 * $Id: wlc_tbtt.c 708017 2017-06-29 14:11:45Z $
 */

/**
 * @file
 * @brief
 * dependencies:
 * - a preTBTT timer that runs in low power or doze mode and a high resolution timer
 *   (microseconds) that extends the preTBTT to TBTT
 *
 * implementation:
 * - uses p2p ucode preTBTT timer to generate periodic preTBTT interrupt, and
 * - uses high res. timer to extends the preTBTT to TBTT
 *
 * For various reason pretbtt interrupt may come at variable timing
 * so deal with it here...
 *
 * -------------------------p--------------t-------c--------------->
 *                  ^       ^      ^       ^       ^
 *                  |<--1-->|      |<--2-->|<--3-->|
 *                  | ^     |      |     ^ |    ^  |
 *                  | |     |      |     | |    |  |
 *                  |pretbtt|      |pretbtt|pretbtt|
 *                  |early  |      |late   |fake   |
 *                  |       |      |       |       |
 *                  |       |<--4->|       |<--5-->|
 * p: pretbtt
 * t: tbtt
 * c: toss point
 * 1: pretbtt early, before pretbtt
 * 2: pretbtt late, after normal pretbtt range
 * 3: pretbtt fake, after tbtt but before toss point
 * 4: normal pretbtt range - norm_pre_tbtt_max
 * 5: fake pretbtt range - fake_pre_tbtt_max
 */

/* XXX: Define wlc_cfg.h to be the first header file included as some builds
 * get their feature flags thru this file.
 */
#include <wlc_cfg.h>
#ifdef WLMCNX
#include <osl.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <siutils.h>
#include <wlioctl.h>
#include <sbhnddma.h>
#include <hnddma.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_mcnx.h>
#include <wlc_bmac.h>
#include <wlc_hrt.h>
#include <wlc_utils.h>
#include <bcm_notif_pub.h>
#include <wlc_tbtt.h>

/* module states */
struct wlc_tbtt_info {
	wlc_info_t *wlc;
	int cfgh;		/* per bsscfg private storage handle */
};

/* tbtt entry states */
typedef struct {
	/* back pointer to tbtt module */
	wlc_tbtt_info_t *ti;
	/* back pointer to bsscfg this tbtt belongs to */
	wlc_bsscfg_t *cfg;
	/* configuration */
	int enabled;
	int tsf_adj;
	uint pre_tbtt_cfg;		/* preTBTT setting */
	uint norm_pre_tbtt_max;		/* preTBTT interrupt normal arrival range */
	uint fake_pre_tbtt_max;		/* preTBTT interrupt fake arrival range */
	/* notifier handle */
	bcm_notif_h pre_notif_hdl;
	bcm_notif_h notif_hdl;
	/* timer to relay pre-tbtt to tbtt */
	wlc_hrt_to_t *timer;
	bool active;			/* is timer armed? */
	bool req_adj;			/* request to update tbtt */
/* ==== please keep these debug stuff at the bottom ==== */
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
	/* stats */
	uint collision;			/* schedule conflict */
	uint pretbtt;			/* pretbtt interrupt */
	uint tbtt;			/* tbtt interrupt (simulated) */
	uint pretbttlate;		/* pretbtt interrupt arrived late */
	uint pretbttlatemin;
	uint pretbttearly;		/* pretbtt interrupt arrived early */
	uint pretbttearlymax;
	uint pretbttfake;		/* pretbtt is past tbtt but still before the toss point */
	uint pretbttfakemax;
	uint pretbtttoss;		/* pretbtt is past tbtt and is too late so it is tossed */
	uint pretbtttossmax;
#endif // endif
} wlc_tbtt_ent_t;

/* tbtt entry states accessor */
#define TBTT_BSSCFG_CUBBY_LOC(ti, cfg) ((wlc_tbtt_ent_t **)BSSCFG_CUBBY(cfg, (ti)->cfgh))
#define TBTT_BSSCFG_CUBBY(ti, cfg) (*(TBTT_BSSCFG_CUBBY_LOC(ti, cfg)))
#define TBTT_ENT(ti, cfg) TBTT_BSSCFG_CUBBY(ti, cfg)

/* debug/stats macros */
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
#define COLCNTINC(te)		((te)->collision++)
#define PRETBTTCNTINC(te)	((te)->pretbtt++)
#define PRETBTTLATECNTINC(te)	((te)->pretbttlate++)
#define PRETBTTEARLYCNTINC(te)	((te)->pretbttearly++)
#define TBTTCNTINC(te)		((te)->tbtt++)
#define PRETBTTFAKECNTINC(te)	((te)->pretbttfake++)
#define PRETBTTTOSSCNTINC(te)	((te)->pretbtttoss++)
#else
#define COLCNTINC(te)
#define PRETBTTCNTINC(te)
#define PRETBTTLATECNTINC(te)
#define PRETBTTEARLYCNTINC(te)
#define TBTTCNTINC(te)
#define PRETBTTFAKECNTINC(te)
#define PRETBTTTOSSCNTINC(te)
#endif /* BCMDBG || BCMDBG_DUMP */

/* local functions */
/* module entries */
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static int wlc_tbtt_dump(void *ctx, struct bcmstrbuf *b);
#endif // endif

/* callbacks */
static void wlc_tbtt_bss_updn_cb(void *ctx, bsscfg_up_down_event_data_t *notif_data);
static void wlc_tbtt_pre_tbtt_cb(void *ctx, wlc_mcnx_intr_data_t *notif_data);
static void wlc_tbtt_tbtt_cb(void *arg);

/* bsscfg cubby */
static void wlc_tbtt_ent_deinit(void *ctx, wlc_bsscfg_t *cfg);
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static void wlc_tbtt_ent_dump(void *ctx, wlc_bsscfg_t *cfg, struct bcmstrbuf *b);
#else
#define wlc_tbtt_ent_dump NULL
#endif // endif

/* module entries */
wlc_tbtt_info_t *
BCMATTACHFN(wlc_tbtt_attach)(wlc_info_t *wlc)
{
	wlc_tbtt_info_t *ti;

	if ((ti = MALLOCZ(wlc->osh, sizeof(wlc_tbtt_info_t))) == NULL) {
		WL_ERROR(("wl%d: %s: MALLOC failed, malloced %d bytes\n",
		          wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->pub->osh)));
		goto fail;
	}
	ti->wlc = wlc;

	/* reserve cubby in the bsscfg container for per-bsscfg private data */
	if ((ti->cfgh = wlc_bsscfg_cubby_reserve(wlc, sizeof(wlc_tbtt_ent_t *),
	                NULL, wlc_tbtt_ent_deinit, wlc_tbtt_ent_dump,
	                ti)) < 0) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_cubby_reserve() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* register bsscfg up/down callback */
	if (wlc_bsscfg_updown_register(wlc, wlc_tbtt_bss_updn_cb, ti) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_updown_register() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* register preTBTT callback */
	if ((wlc_mcnx_intr_register(wlc->mcnx, wlc_tbtt_pre_tbtt_cb, ti)) != BCME_OK) {
		WL_ERROR(("wl%d: wlc_mcnx_intr_register failed (tbtt)\n",
		          wlc->pub->unit));
		goto fail;
	}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
	wlc_dump_register(wlc->pub, "tbtt", wlc_tbtt_dump, (void *)ti);
#endif // endif

	return ti;

fail:
	wlc_tbtt_detach(ti);
	return NULL;
}

void
BCMATTACHFN(wlc_tbtt_detach)(wlc_tbtt_info_t *ti)
{
	wlc_info_t *wlc;

	if (ti == NULL)
		return;

	wlc = ti->wlc;

	wlc_mcnx_intr_unregister(wlc->mcnx, wlc_tbtt_pre_tbtt_cb, ti);

	wlc_bsscfg_updown_unregister(wlc, wlc_tbtt_bss_updn_cb, ti);

	MFREE(wlc->osh, ti, sizeof(wlc_tbtt_info_t));
}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static int
wlc_tbtt_dump(void *ctx, struct bcmstrbuf *b)
{
	wlc_tbtt_info_t *ti = (wlc_tbtt_info_t *)ctx;
	wlc_info_t *wlc = ti->wlc;
	int idx;
	wlc_bsscfg_t *cfg;

	bcm_bprintf(b, "tbtt: cfgh %d\n", ti->cfgh);

	FOREACH_BSS(wlc, idx, cfg) {
		bcm_bprintf(b, "bsscfg > %d\n", WLC_BSSCFG_IDX(cfg));
		wlc_tbtt_ent_dump(ti, cfg, b);
	}

	return BCME_OK;
}
#endif /* BCMDBG || BCMDBG_DUMP */

void
wlc_tbtt_ent_init(wlc_tbtt_info_t *ti, wlc_bsscfg_t *cfg)
{
	wlc_tbtt_ent_t *te;

	ASSERT(cfg != NULL);

	te = TBTT_ENT(ti, cfg);
	if (te == NULL)
		return;

	/* preTBTT configuration */
	te->pre_tbtt_cfg = wlc_pretbtt_calc(cfg);
	te->pre_tbtt_cfg += (1 << P2P_UCODE_TIME_SHIFT) - 1;
	te->pre_tbtt_cfg >>= P2P_UCODE_TIME_SHIFT;
	te->pre_tbtt_cfg <<= P2P_UCODE_TIME_SHIFT;

	/* normal preTBTT interrupt arrival time after expected preTBTT time */
	te->norm_pre_tbtt_max = te->pre_tbtt_cfg * 20 / 100;
	te->norm_pre_tbtt_max += (1 << P2P_UCODE_TIME_SHIFT) - 1;
	te->norm_pre_tbtt_max >>= P2P_UCODE_TIME_SHIFT;
	te->norm_pre_tbtt_max <<= P2P_UCODE_TIME_SHIFT;

	/* abnormal preTBTT interrupt arrival time after expected TBTT time */
	te->fake_pre_tbtt_max = 3000;

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
	te->pretbttlatemin = te->pre_tbtt_cfg - te->norm_pre_tbtt_max;
	te->pretbttearlymax = 0;
	te->pretbttfakemax = 0;
	te->pretbtttossmax = 0;
#endif // endif
}

/* tbtt entity allocation/deletion */
static wlc_tbtt_ent_t *
wlc_tbtt_ent_alloc(wlc_tbtt_info_t *ti, wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = ti->wlc;
	wlc_tbtt_ent_t *te;
	wlc_tbtt_ent_t **pte;

	ASSERT(cfg != NULL);

	/* create a tbtt entry */
	if ((te = MALLOCZ(wlc->osh, sizeof(wlc_tbtt_ent_t))) == NULL) {
		WL_ERROR(("wl%d: %s: MALLOC failed, malloced %d bytes\n",
		          wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->pub->osh)));
		goto fail;
	}
	te->ti = ti;
	te->cfg = cfg;
	pte = TBTT_BSSCFG_CUBBY_LOC(ti, cfg);
	*pte = te;

	/* create the timer */
	if ((te->timer = wlc_hrt_alloc_timeout(wlc->hrti)) == NULL) {
		WL_ERROR(("wl%d: %s: wlc_hrt_alloc_timeout failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* create notification list for pretbtt. */
	if (bcm_notif_create_list(wlc->notif, &te->pre_notif_hdl) != BCME_OK) {
		WL_ERROR(("wl%d: %s: tbtt bcm_notif_create_list() failed\n",
		         wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* create notification list for tbtt. */
	if (bcm_notif_create_list(wlc->notif, &te->notif_hdl) != BCME_OK) {
		WL_ERROR(("wl%d: %s: tbtt bcm_notif_create_list() failed\n",
		         wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	wlc_tbtt_ent_init(ti, cfg);

	return te;

fail:
	wlc_tbtt_ent_deinit(ti, cfg);
	return NULL;
}

static void
wlc_tbtt_ent_deinit(void *ctx, wlc_bsscfg_t *cfg)
{
	wlc_tbtt_info_t *ti = (wlc_tbtt_info_t *)ctx;
	wlc_info_t *wlc;
	wlc_tbtt_ent_t **pte;
	wlc_tbtt_ent_t *te;

	ASSERT(cfg != NULL);

	pte = TBTT_BSSCFG_CUBBY_LOC(ti, cfg);
	te = *pte;
	*pte = NULL;

	if (te == NULL)
		return;

	if (te->notif_hdl != NULL)
		bcm_notif_delete_list(&te->notif_hdl);
	if (te->pre_notif_hdl != NULL)
		bcm_notif_delete_list(&te->pre_notif_hdl);
	if (te->timer != NULL)
		wlc_hrt_free_timeout(te->timer);

	wlc = ti->wlc;

	MFREE(wlc->osh, te, sizeof(wlc_tbtt_ent_t));
}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static void
wlc_tbtt_ent_dump(void *ctx, wlc_bsscfg_t *cfg, struct bcmstrbuf *b)
{
	wlc_tbtt_info_t *ti = (wlc_tbtt_info_t *)ctx;
	wlc_tbtt_ent_t *te;

	ASSERT(cfg != NULL);

	te = TBTT_ENT(ti, cfg);
	if (te == NULL)
		return;

	bcm_bprintf(b, "\tenabled %d active %d tsfadj %d\n",
	            te->enabled, te->active, te->tsf_adj);
	bcm_bprintf(b, "\tpretbtt %u tbtt %u\n", te->pretbtt, te->tbtt);
	bcm_bprintf(b, "\tcollision %u late %u early %u fake %u miss %u\n",
	            te->collision, te->pretbttlate, te->pretbttearly,
	            te->pretbttfake, te->pretbtttoss);
	bcm_bprintf(b, "\tlatemin %u earlymax %u fakemax %u missmax %u\n",
	            te->pretbttlatemin, te->pretbttearlymax,
	            te->pretbttfakemax, te->pretbtttossmax);
	bcm_bprintf(b, "\tpretbtt config %u normal pretbtt max %u fake pretbtt max %u\n",
	            te->pre_tbtt_cfg, te->norm_pre_tbtt_max, te->fake_pre_tbtt_max);
}
#endif /* BCMDBG || BCMDBG_DUMP */

/* enable/disable the entry */
static void
wlc_tbtt_ent_enab(wlc_tbtt_ent_t *te, bool enab)
{
	wlc_tbtt_info_t *ti = te->ti;
	wlc_info_t *wlc = ti->wlc;
	int idx;
	wlc_bsscfg_t *cfg;

	if (enab) {
		te->enabled ++;
		wlc_bmac_enable_tbtt(wlc->hw, TBTT_TBTT_MASK, TBTT_TBTT_MASK);
		return;
	}

	/* disable */
	if (te->enabled == 0)
		return;
	te->enabled --;
	FOREACH_BSS(wlc, idx, cfg) {
		wlc_tbtt_ent_t *e = TBTT_ENT(ti, cfg);
		if (e != NULL && e->enabled != 0)
			break;
	}
	if (cfg == NULL) {
		wlc_bmac_enable_tbtt(wlc->hw, TBTT_TBTT_MASK, 0);
	}
}

static void
wlc_tbtt_ent_reset(wlc_tbtt_info_t *ti, wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = ti->wlc;
	wlc_tbtt_ent_t *te;

	ASSERT(cfg != NULL);

	te = TBTT_ENT(ti, cfg);
	if (te == NULL)
		return;

	if (te->timer != NULL &&
	    te->active) {
		wlc_gptimer_wake_upd(wlc, WLC_GPTIMER_AWAKE_TBTT, FALSE);
		wlc_hrt_del_timeout(te->timer);
		te->active = FALSE;
	}
}

static wlc_tbtt_ent_t *
wlc_tbtt_ent_lookup(wlc_tbtt_info_t *ti, wlc_bsscfg_t *cfg)
{
	wlc_tbtt_ent_t **pte;
	wlc_tbtt_ent_t *te;

	ASSERT(cfg != NULL);

	if ((te = TBTT_ENT(ti, cfg)) != NULL)
		return te;

	te = wlc_tbtt_ent_alloc(ti, cfg);
	pte = TBTT_BSSCFG_CUBBY_LOC(ti, cfg);
	*pte = te;

	return te;
}

/* configuration */
int
wlc_tbtt_ent_tsf_adj_set(wlc_tbtt_info_t *ti, wlc_bsscfg_t *cfg, int adj)
{
	wlc_tbtt_ent_t *te;

	ASSERT(cfg != NULL);

	te = TBTT_ENT(ti, cfg);
	if (te == NULL)
		return BCME_NORESOURCE;

	te->tsf_adj = adj;
	return BCME_OK;
}

/* tbtt handler register/remove */
int
wlc_tbtt_ent_fn_add(wlc_tbtt_info_t *ti, wlc_bsscfg_t *cfg,
	wlc_tbtt_ent_fn_t pre_fn, wlc_tbtt_ent_fn_t fn, void *arg)
{
	wlc_tbtt_ent_t *te;
	int ret;

	te = wlc_tbtt_ent_lookup(ti, cfg);
	if (te == NULL)
		return BCME_NORESOURCE;

	if ((ret = bcm_notif_add_interest(te->pre_notif_hdl,
	                                  (bcm_notif_client_callback)pre_fn, arg)) == BCME_OK &&
	    (ret = bcm_notif_add_interest(te->notif_hdl,
	                                  (bcm_notif_client_callback)fn, arg)) == BCME_OK)
		wlc_tbtt_ent_enab(te, TRUE);
	return ret;
}

int
wlc_tbtt_ent_fn_del(wlc_tbtt_info_t *ti, wlc_bsscfg_t *cfg,
	wlc_tbtt_ent_fn_t pre_fn, wlc_tbtt_ent_fn_t fn, void *arg)
{
	wlc_tbtt_ent_t *te;
	int ret;

	ASSERT(cfg != NULL);

	te = TBTT_ENT(ti, cfg);
	if (te == NULL)
		return BCME_NORESOURCE;

	if ((ret = bcm_notif_remove_interest(te->pre_notif_hdl,
	                                     (bcm_notif_client_callback)pre_fn, arg)) == BCME_OK &&
	    (ret = bcm_notif_remove_interest(te->notif_hdl,
	                                     (bcm_notif_client_callback)fn, arg)) == BCME_OK)
		wlc_tbtt_ent_enab(te, FALSE);
	return ret;
}

/* bsscfg up/down */
static void
wlc_tbtt_bss_updn_cb(void *ctx, bsscfg_up_down_event_data_t *notif_data)
{
	wlc_tbtt_info_t *ti = (wlc_tbtt_info_t *)ctx;
	wlc_bsscfg_t *cfg;

	ASSERT(notif_data != NULL);

	cfg = notif_data->bsscfg;
	ASSERT(cfg != NULL);

	if (notif_data->up) {
		wlc_tbtt_ent_init(ti, cfg);
	}
	else {
		wlc_tbtt_ent_reset(ti, cfg);
	}
}

#ifdef BCMDBG
static uint32 last_pre_tbtt = 0;
#define WL_TBTT_P(x) WL_NONE(x)
#define WL_TBTT_PP(x) WL_PRINT(x)
#endif // endif

/* pretbtt interrupt handler */
static void
wlc_tbtt_pre_tbtt_cb(void *ctx, wlc_mcnx_intr_data_t *notif_data)
{
	wlc_tbtt_info_t *ti = (wlc_tbtt_info_t *)ctx;
	wlc_info_t *wlc = ti->wlc;
	wlc_tbtt_ent_t *te;
	wlc_bsscfg_t *cfg;
	uint32 tsf_h, tsf_l;
	uint32 tbtt_h, tbtt_l;
	uint32 tt_next_tbtt;
	uint32 tf_last_tbtt;
	wlc_mcnx_info_t *mcnx = wlc->mcnx;

	if (notif_data->intr != M_P2P_I_PRE_TBTT)
		return;

	cfg = notif_data->cfg;
	ASSERT(cfg != NULL);

	te = TBTT_ENT(ti, cfg);
	if (te == NULL)
		return;

	if (te->active) {
		wlc_gptimer_wake_upd(wlc, WLC_GPTIMER_AWAKE_TBTT, FALSE);
		wlc_hrt_del_timeout(te->timer);
		te->active = FALSE;
		COLCNTINC(te);
	}

	PRETBTTCNTINC(te);

	/* FIXME: The TSF in the notification data block is in an earlier time
	 * than it is here now so the calculation of tbtt may be off a bit...
	 */

	/* calculate how much to extend from preTBTT to TBTT */
	tsf_h = notif_data->tsf_h;
	tsf_l = notif_data->tsf_l;

	/* immediate next tbtt */
	wlc_mcnx_next_l_tbtt64(mcnx, cfg, tsf_h, tsf_l, &tbtt_h, &tbtt_l);
	/* distance between the next tbtt and the tsf */
	tt_next_tbtt = U32_DUR(tsf_l, tbtt_l);
#ifdef BCMDBG
	WL_TBTT_P(("%s: now at %u, to next tbtt %u, interval %u\n", __FUNCTION__,
	           tsf_l, tt_next_tbtt, U32_DUR(last_pre_tbtt, tsf_l)));
	last_pre_tbtt = tsf_l;
#endif // endif
	/* the preTBTT comes at different time point... */
	/* after normal pretbtt range... */
	if (tt_next_tbtt > 0 &&
	    tt_next_tbtt <= te->pre_tbtt_cfg - te->norm_pre_tbtt_max) {
		PRETBTTLATECNTINC(te);
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
		if (tt_next_tbtt < te->pretbttlatemin)
			te->pretbttlatemin = tt_next_tbtt;
#endif // endif
	}
	/* before pretbtt... */
	else if (tt_next_tbtt > te->pre_tbtt_cfg &&
	         tt_next_tbtt <= te->pre_tbtt_cfg + te->norm_pre_tbtt_max) {
		PRETBTTEARLYCNTINC(te);
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
		if (tt_next_tbtt > te->pretbttearlymax)
			te->pretbttearlymax = tt_next_tbtt;
#endif // endif
	}

	/* consider all above cases "normal" preTBTT too */
	/* arm the relay timer to fire at TBTT */
	if (tt_next_tbtt > 0 &&
	    tt_next_tbtt <= te->pre_tbtt_cfg + te->norm_pre_tbtt_max) {
		wlc_tbtt_ent_data_t notif_data1;

		wlc_hrt_add_timeout(te->timer, tt_next_tbtt, wlc_tbtt_tbtt_cb, te);
		wlc_gptimer_wake_upd(wlc, WLC_GPTIMER_AWAKE_TBTT, TRUE);
		te->active = TRUE;

		notif_data1.cfg = cfg;
		notif_data1.tsf_h = tsf_h;
		notif_data1.tsf_l = tsf_l;
		bcm_notif_signal(te->pre_notif_hdl, &notif_data1);
		return;
	}

	/* preTBTT comes too late (passed TBTT)... */
	wlc_mcnx_last_l_tbtt64(mcnx, cfg, tsf_h, tsf_l, &tbtt_h, &tbtt_l);
	/* distance between the last tbtt and the tsf */
	tf_last_tbtt = U32_DUR(tbtt_l, tsf_l);
	/* fake a pretbtt/tbtt... */
	if (tf_last_tbtt < te->fake_pre_tbtt_max) {
		wlc_tbtt_ent_data_t notif_data1;

		notif_data1.cfg = cfg;
		notif_data1.tsf_h = tsf_h;
		notif_data1.tsf_l = tsf_l;
		bcm_notif_signal(te->pre_notif_hdl, &notif_data1);

		wlc_tbtt_tbtt_cb(te);
		PRETBTTFAKECNTINC(te);
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
		if (tf_last_tbtt > te->pretbttfakemax)
			te->pretbttfakemax = tf_last_tbtt;
#endif // endif
		return;
	}

	/* preTBTT comes way too late... */
	PRETBTTTOSSCNTINC(te);
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
	if (tf_last_tbtt > te->pretbtttossmax)
		te->pretbtttossmax = tf_last_tbtt;
#endif // endif
#ifdef BCMDBG
{
	wlc_mcnx_next_l_tbtt64(mcnx, cfg, tsf_h, tsf_l, &tbtt_h, &tbtt_l);
	WL_TBTT_PP(("%s: bss %d problem at tick %x (local) %x (remote) offset %u "
	            "to tbtt %x (local) %x (remote)\n",
	            __FUNCTION__,
	            wlc_mcnx_BSS_idx(mcnx, cfg),
	            tsf_l, wlc_mcnx_l2r_tsf32(mcnx, cfg, tsf_l),
	            tt_next_tbtt,
	            tbtt_l, wlc_mcnx_l2r_tsf32(mcnx, cfg, tbtt_l)));
}
#endif // endif
}

#ifdef BCMDBG
static uint32 last_tbtt = 0;
#define WL_TBTT_I(x) WL_NONE(x)
#endif // endif

/* tbtt interrupt handler */
static void
wlc_tbtt_tbtt_cb(void *arg)
{
	wlc_tbtt_ent_t *te = (wlc_tbtt_ent_t *)arg;
	wlc_tbtt_info_t *ti = te->ti;
	wlc_info_t *wlc = ti->wlc;
	wlc_bsscfg_t *cfg;
	wlc_tbtt_ent_data_t notif_data;
	wlc_mcnx_info_t *mcnx = wlc->mcnx;
	uint32 tsf_h, tsf_l;

	TBTTCNTINC(te);

	cfg = te->cfg;

	wlc_read_tsf(wlc, &tsf_l, &tsf_h);

#ifdef BCMDBG
{
	WL_TBTT_I(("%s: now at %u, interval %u\n",
	           __FUNCTION__, tsf_l, U32_DUR(last_tbtt, tsf_l)));
	last_tbtt = tsf_l;
}
#endif // endif

	wlc_gptimer_wake_upd(wlc, WLC_GPTIMER_AWAKE_TBTT, FALSE);
	te->active = FALSE;

	notif_data.cfg = cfg;
	notif_data.tsf_h = tsf_h;
	notif_data.tsf_l = tsf_l;
	bcm_notif_signal(te->notif_hdl, &notif_data);

	if (te->tsf_adj != 0 || te->req_adj) {
		te->req_adj = FALSE;
		wlc_mcnx_tbtt_adj(mcnx, cfg, te->tsf_adj);
	}
}
#endif /* WLMCNX */
