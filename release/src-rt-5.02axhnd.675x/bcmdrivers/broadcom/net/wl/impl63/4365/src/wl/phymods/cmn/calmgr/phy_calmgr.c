/*
 * CALibrationManaGeR module implementation
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
 * $Id$
 */

#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmutils.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include <phy_api.h>
#include <phy_cache.h>
#include <phy_chanmgr_notif.h>
#include "phy_calmgr_cfg.h"
#include "phy_type_calmgr.h"
#include <phy_calmgr.h>
#include <phy_rstr.h>

/* trigger registry callback entry */
typedef struct {
	phy_calmgr_trigger_fn_t fn;
	phy_calmgr_trigger_ctx_t *ctx;
	phy_calmgr_trigger_id_t tid;
} phy_calmgr_trigger_reg_t;

/* calibration registry callback entry */
typedef struct {
	phy_calmgr_cal_fn_t cal;
	phy_calmgr_reset_fn_t reset;
	phy_calmgr_cal_ctx_t *ctx;
	phy_calmgr_cal_id_t cmid;
	phy_cache_cubby_id_t ccid;
} phy_calmgr_cal_reg_t;

/* calibration duration registry entry */
typedef uint32 phy_calmgr_dur_reg_t;	/* calibration duration in 1us units */

/* per chspec context - cached */
typedef struct {
	/* config */
	phy_calmgr_cal_mode_t init_mode; /* initial calibration mode */
	uint32 cal_prd;			/* calibration interval */
	/* runtime states */
	uint32 last_start;		/* last watchdog tick the cal was started */
	uint8 cal_state;		/* calibration process state */
	uint8 cal_cur;			/* calibration registry entry index */
	uint8 flags;
	phy_calmgr_cal_id_t cur_cmid;	/* current cal id */
	/* error */
	int last_err;			/* last error */
	phy_calmgr_cal_id_t last_cmid;	/* last cal id */
} phy_calmgr_cache_t;

/* calibration manager state */
#define CALMGR_ST_UNK		0
#define CALMGR_ST_START		1
#define CALMGR_ST_INPROG	2
#define CALMGR_ST_DONE		3

/* calibration registry entry index */
#define CALMGR_IDX_INV	255

/* calibration flags */
#define CACHE_FLAG_CFG	(1<<0)	/* configured */

/* stats */
typedef struct {
	uint32 nodur;
} phy_calmgr_stats_t;

/* module private states */
struct phy_calmgr_info {
	phy_info_t *pi;

	/* global states */
	phy_cache_cubby_id_t ccid;	/* cache cubby ID */
	uint32 flags;
	uint32 allow_dur;	/* cal duration allowed for each watchdog tick, in us units */
	uint32 left_dur;	/* cal duration left unused for the tick, in us units */

	/* current context */
	phy_calmgr_cache_t *ctx;

	/* calibration registry */
	uint8 cal_sz;
	uint8 cal_cnt;
	phy_calmgr_cal_reg_t *cal_tbl;
	uint8 *cal_dur;		/* index to the first dur_tbl entry */

	/* duration database */
	uint8 dur_sz;
	uint8 dur_cnt;
	phy_calmgr_dur_reg_t *dur_tbl;

	/* trigger registry */
	uint8 trigger_sz;
	uint8 trigger_cnt;
	phy_calmgr_trigger_reg_t *trigger_tbl;

	/* PHY specific implementation */
	phy_type_calmgr_fns_t *fns;

	/* statistics */
	phy_calmgr_stats_t *stats;

	struct wlapi_timer *timer;
};

/* calmgr flags */
#define CALMGR_FLAG_DUR_MEASURED	(1<<0)	/* duration measured */
#define CALMGR_FLAG_BYPASS_CACHE	(1<<1)	/* bypass cache operation */
#define CALMGR_FLAG_FORCED_CAL		(1<<2)	/* forced cal */

/* speedy cal config */
#define CALMGR_SPEEDY_CAL_DUR_MARGIN	500	/* in us units */

/* module private states memory layout */
typedef struct {
	phy_calmgr_info_t info;
	phy_calmgr_cache_t ctx;
	phy_calmgr_cal_reg_t cal_tbl[PHY_CALMGR_CAL_REG_SZ];
	uint8 cal_dur[PHY_CALMGR_CAL_REG_SZ + 1];
	phy_calmgr_trigger_reg_t trigger_tbl[PHY_CALMGR_TRIGGER_REG_SZ];
	phy_calmgr_dur_reg_t dur_tbl[PHY_CALMGR_DUR_REG_SZ];
	phy_type_calmgr_fns_t fns;
	phy_calmgr_stats_t stats;
} phy_calmgr_mem_t;

/* debug */
#ifdef BCMDBG
#define CALMGR_DBGMSG_VERBOSE (1<<0)
static uint32 calmgr_dbgmsg = CALMGR_DBGMSG_VERBOSE;
#define PHY_CALMGR(x) do {if (calmgr_dbgmsg & CALMGR_DBGMSG_VERBOSE) PHY_CAL(x);} while (0)
#else
#define PHY_CALMGR(x)
#endif // endif

/* local function declaration */
static bool phy_calmgr_wd(phy_wd_ctx_t *ctx);
static bool phy_calmgr_initial_trigger(phy_calmgr_trigger_ctx_t *ctx);
static bool phy_calmgr_period_trigger(phy_calmgr_trigger_ctx_t *ctx);
static bool phy_calmgr_eval_triggers(phy_calmgr_info_t *ci);
#ifdef NEW_PHY_CAL_ARCH
static void phy_calmgr_init_ctx(phy_cache_ctx_t *ctx);
static int phy_calmgr_save_ctx(phy_cache_ctx_t *ctx, uint8 *buf);
static int phy_calmgr_restore_ctx(phy_cache_ctx_t *ctx, uint8 *buf);
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static int phy_calmgr_dump_ctx(phy_cache_ctx_t *ctx, uint8 *buf, struct bcmstrbuf *b);
#else
#define phy_calmgr_dump_ctx NULL
#endif // endif
static int phy_calmgr_chspec_notif(phy_chanmgr_notif_ctx_t *ctx, phy_chanmgr_notif_data_t *data);
#endif /* NEW_PHY_CAL_ARCH */
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static int phy_calmgr_dump(void *ctx, struct bcmstrbuf *b);
#endif // endif

/* periodic timer callback */
static void phy_calmgr_timer_cb(void *ctx);

#ifdef TEST_CALM
static phy_calmgr_cal_status_t phy_calmgr_test_cal(phy_calmgr_cal_ctx_t *ctx,
	phy_calmgr_cal_mode_t mode);
#endif // endif

/* attach/detach */
phy_calmgr_info_t *
BCMATTACHFN(phy_calmgr_attach)(phy_info_t *pi)
{
	phy_calmgr_info_t *info;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate attach info storage */
	if ((info = phy_malloc(pi, sizeof(phy_calmgr_mem_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	info->pi = pi;

	info->ctx = &((phy_calmgr_mem_t *)info)->ctx;

	info->cal_sz = PHY_CALMGR_CAL_REG_SZ;
	info->cal_tbl = ((phy_calmgr_mem_t *)info)->cal_tbl;
	info->cal_dur = ((phy_calmgr_mem_t *)info)->cal_dur;

	info->dur_sz = PHY_CALMGR_DUR_REG_SZ;
	info->dur_tbl = ((phy_calmgr_mem_t *)info)->dur_tbl;

	info->trigger_sz = PHY_CALMGR_TRIGGER_REG_SZ;
	info->trigger_tbl = ((phy_calmgr_mem_t *)info)->trigger_tbl;

	info->fns = &((phy_calmgr_mem_t *)info)->fns;

	info->stats = &((phy_calmgr_mem_t *)info)->stats;

	/* register watchdog fn */
	if (phy_wd_add_fn(pi->wdi, phy_calmgr_wd, info,
	                PHY_WD_PRD_1TICK, PHY_WD_1TICK_CALMGR,
	                PHY_WD_FLAG_DEF_SKIP | PHY_WD_FLAG_MCHAN_AWARE) != BCME_OK) {
		PHY_ERROR(("%s: phy_wd_add_fn failed\n", __FUNCTION__));
		goto fail;
	}
#ifdef NEW_PHY_CAL_ARCH
	/* reserve some space in cache */
	if (phy_cache_reserve_cubby(pi->cachei, phy_calmgr_init_ctx,
	                phy_calmgr_save_ctx, phy_calmgr_restore_ctx, phy_calmgr_dump_ctx, info,
	                sizeof(phy_calmgr_cache_t), PHY_CACHE_FLAG_AUTO_SAVE,
	                &info->ccid) != BCME_OK) {
		PHY_ERROR(("%s: phy_cache_reserve_cubby failed\n", __FUNCTION__));
		goto fail;
	}
#endif // endif
	/* register calibration trigger callback */
	if (phy_calmgr_add_trigger_fn(info, phy_calmgr_initial_trigger, info,
	                PHY_CALMGR_TRIGGER_INITIAL) != BCME_OK) {
		PHY_ERROR(("%s: phy_calmgr_add_trigger_fn failed\n", __FUNCTION__));
		goto fail;
	}
	if (phy_calmgr_add_trigger_fn(info, phy_calmgr_period_trigger, info,
	                PHY_CALMGR_TRIGGER_PERIOD) != BCME_OK) {
		PHY_ERROR(("%s: phy_calmgr_add_trigger_fn failed\n", __FUNCTION__));
		goto fail;
	}

#ifdef TEST_CALM
	/* test cal callback */
	if (phy_calmgr_add_cal_fn(info, phy_calmgr_test_cal, NULL, info, 0, 0) != BCME_OK) {
		PHY_ERROR(("%s: phy_calmgr_add_cal_fn failed\n", __FUNCTION__));
		goto fail;
	}
#endif // endif
#ifdef NEW_PHY_CAL_ARCH
	/* register chspec notification callback */
	if (phy_chanmgr_notif_add_interest(pi->chanmgr_notifi,
		phy_calmgr_chspec_notif, info, PHY_CHANMGR_NOTIF_ORDER_CALMGR,
		PHY_CHANMGR_NOTIF_CH_CHG) != BCME_OK) {
		PHY_ERROR(("%s: phy_chanmgr_notif_add_interest failed\n", __FUNCTION__));
		goto fail;
	}
#endif // endif
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
	/* register dump callback */
	phy_dbg_add_dump_fn(pi, "phycalmgr", (phy_dump_fn_t)phy_calmgr_dump, info);
#endif // endif

	return info;

fail:
	phy_calmgr_detach(info);
	return NULL;
}

void
BCMATTACHFN(phy_calmgr_detach)(phy_calmgr_info_t *ci)
{
	phy_info_t *pi;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (ci == NULL)
		return;

	pi = ci->pi;

	phy_mfree(pi, ci, sizeof(phy_calmgr_mem_t));
}

/* register phy type specific implementations */
int
BCMATTACHFN(phy_calmgr_register_impl)(phy_calmgr_info_t *ci, phy_type_calmgr_fns_t *fns)
{
	phy_info_t *pi = ci->pi;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (ci->timer != NULL) {
		PHY_ERROR(("%s: implementation has already been registered\n", __FUNCTION__));
		return BCME_ERROR;
	}

	if ((ci->timer =
	     wlapi_init_timer(pi->sh->physhim, phy_calmgr_timer_cb, ci, rstr_calmgr)) == NULL) {
		PHY_ERROR(("%s: wlapi_init_timer for %s failed\n", __FUNCTION__, rstr_calmgr));
		return BCME_NORESOURCE;
	}
	/* TODO: remove 'pi->phycal_timer' */
	pi->phycal_timer = ci->timer;

	*ci->fns = *fns;
	return BCME_OK;
}

void
BCMATTACHFN(phy_calmgr_unregister_impl)(phy_calmgr_info_t *ci)
{
	phy_info_t *pi = ci->pi;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (ci->timer != NULL) {
		wlapi_del_timer(pi->sh->physhim, ci->timer);
		wlapi_free_timer(pi->sh->physhim, ci->timer);
		ci->timer = NULL;
	}
	/* TODO: remove 'pi->phycal_timer' */
	pi->phycal_timer = NULL;
}

/* evaluate a single calibration trigger */
static bool
phy_calmgr_eval_trigger(phy_calmgr_info_t *ci, phy_calmgr_trigger_id_t tid)
{
	uint cur;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* TODO: use a faster binary search */

	for (cur = 0; cur < ci->trigger_cnt; cur ++) {
		if (ci->trigger_tbl[cur].tid != tid)
			continue;
		ASSERT(ci->trigger_tbl[cur].fn != NULL);
		return (ci->trigger_tbl[cur].fn)(ci->trigger_tbl[cur].ctx);
	}

	PHY_NONE(("%s: unable to find trigger %u\n", __FUNCTION__, tid));

#ifdef TEST_CALM
	return TRUE;
#else
	return FALSE;
#endif // endif
}

/* evaluate all calibration triggers */
static bool
phy_calmgr_eval_triggers(phy_calmgr_info_t *ci)
{
	return phy_calmgr_eval_trigger(ci, PHY_CALMGR_TRIGGER_INITIAL) ||
	        (phy_calmgr_eval_trigger(ci, PHY_CALMGR_TRIGGER_PERIOD) &&
	         phy_calmgr_eval_trigger(ci, PHY_CALMGR_TRIGGER_TEMP_SENSE));
}

static void
phy_calmgr_timer_cb(void *ctx)
{
	phy_calmgr_info_t *ci = (phy_calmgr_info_t *)ctx;
	phy_info_t *pi = ci->pi;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* TODO: move the code from wlc_phy_cmn.c to here */

	wlc_phy_timercb_phycal(pi);
}

#ifdef NEW_PHY_CAL_ARCH
/* init calmgr states */
static void
phy_calmgr_init_ctx(phy_cache_ctx_t *ctx)
{
	phy_calmgr_info_t *ci = (phy_calmgr_info_t *)ctx;
	phy_calmgr_cache_t *cache = ci->ctx;

	PHY_TRACE(("%s\n", __FUNCTION__));

	cache->cal_state = CALMGR_ST_UNK;
	cache->cal_cur = CALMGR_IDX_INV;

	/* default initial clibration mode */
	if (!(cache->flags & CACHE_FLAG_CFG)) {
		cache->init_mode = PHY_CALMGR_CAL_MODE_SSHOT;
		cache->cal_prd = PHY_WD_PRD_GLACIAL;
	}

	cache->last_cmid = PHY_CALMGR_CAL_MOD_START;
	cache->last_err = BCME_OK;
}

/* save calmgr states */
static int
phy_calmgr_save_ctx(phy_cache_ctx_t *ctx, uint8 *buf)
{
	phy_calmgr_info_t *ci = (phy_calmgr_info_t *)ctx;

	PHY_TRACE(("%s: buf %p\n", __FUNCTION__, buf));

	bcopy((uint8 *)ci->ctx, buf, sizeof(phy_calmgr_cache_t));

	return BCME_OK;
}

/* restore calmgr states */
static int
phy_calmgr_restore_ctx(phy_cache_ctx_t *ctx, uint8 *buf)
{
	phy_calmgr_info_t *ci = (phy_calmgr_info_t *)ctx;

	PHY_TRACE(("%s: buf %p\n", __FUNCTION__, buf));

	bcopy(buf, (uint8 *)ci->ctx, sizeof(phy_calmgr_cache_t));

	return BCME_OK;
}
#endif /* NEW_PHY_CAL_ARCH */

/* reset calmgr states */
static void
_phy_calmgr_reset(phy_calmgr_info_t *ci, uint8 state)
{
	phy_calmgr_cache_t *cache = ci->ctx;

	/* reset states */
	cache->cal_state = state;
	cache->cal_cur = CALMGR_IDX_INV;

	/* reset flags */
	ci->flags &= ~CALMGR_FLAG_BYPASS_CACHE;
	ci->flags &= ~CALMGR_FLAG_FORCED_CAL;
}

/* Kick off calibration if needed; invoke calibration callbacks.
 * return TRUE to indicate it is in progress.
 */
static bool
phy_calmgr_docal(phy_calmgr_info_t *ci, phy_calmgr_cal_mode_t mode, uint32 *dur)
{
	phy_info_t *pi = ci->pi;
	phy_type_calmgr_fns_t *fns = ci->fns;
	phy_calmgr_cache_t *cache = ci->ctx;
	bool meas, speedy;

	PHY_TRACE(("%s: mode %u\n", __FUNCTION__, mode));

	/* does anyone want to preceed? */
	if (cache->cal_state != CALMGR_ST_INPROG &&
	    !phy_calmgr_eval_triggers(ci))
		return FALSE;

	/* measurement is done only on the first MPHASE calibration */
	meas = mode == PHY_CALMGR_CAL_MODE_MPHASE &&
	        (ci->flags & CALMGR_FLAG_DUR_MEASURED) == 0;
	/* time based speedy cal */
	speedy = mode == PHY_CALMGR_CAL_MODE_MPHASE &&
	        (ci->flags & CALMGR_FLAG_DUR_MEASURED) != 0 &&
	        ci->allow_dur > 0;

	if (cache->cal_state == CALMGR_ST_UNK ||
	    cache->cal_state == CALMGR_ST_DONE) {
		/* init states */
		cache->cal_state = CALMGR_ST_START;
		cache->cal_cur = 0;
		/* save the watchdog tick */
		cache->last_start = pi->sh->now;
		/* reset measurement result index */
		ci->dur_cnt = 0;
	}

	/* Common preparation... */

	/* PHY specific preparation ... */
	if (fns->prepare != NULL)
		(fns->prepare)(fns->ctx);

	/* do one phase or one cal per iteration... */
	while (cache->cal_cur < ci->cal_cnt) {
		phy_calmgr_cal_reg_t *cal;
		phy_calmgr_cal_status_t st;
		volatile uint32 tsf;
		uint cal_cur;
		bool save;

		/* vaildation */
		if (meas &&
		    ci->dur_cnt == ci->dur_sz) {
			PHY_CALMGR(("%s: run out of cal. duration entries\n", __FUNCTION__));
			ci->stats->nodur++;
			break;
		}

		cache->cal_state = CALMGR_ST_INPROG;

		cal_cur = cache->cal_cur;
		cal = &ci->cal_tbl[cal_cur];

		PHY_CALMGR(("%s: calling %p\n", __FUNCTION__, cal->cal));

		/* perform one phase or one cal and record the duration */
		if (meas)
			tsf = R_REG(pi->sh->osh, &pi->regs->tsf_timerlow);
		ASSERT(cal->cal != NULL);
		st = (cal->cal)(cal->ctx, mode);
		if (meas) {
			tsf = R_REG(pi->sh->osh, &pi->regs->tsf_timerlow) - tsf;
			ci->dur_tbl[ci->dur_cnt] = tsf;
		}

		/* calculate how much time left */
		if (speedy) {
			*dur -= ci->dur_tbl[ci->dur_cnt];
		}

		/* move to next dur. */
		ci->dur_cnt++;

		/* advance to next phase or cal */
		switch (st) {
		case PHY_CALMGR_CAL_ST_PHASE_DONE:
			PHY_CALMGR(("%s: phase done\n", __FUNCTION__));
			break;

		case PHY_CALMGR_CAL_ST_CAL_DONE:
			PHY_CALMGR(("%s: cal done\n", __FUNCTION__));
			/* move to next cal. */
			cache->cal_cur ++;
			break;

		default:
			/* ASSERT! */
			PHY_ERROR(("%s: callback %p is in unknown state %u, "
			           "move to next cal.\n", __FUNCTION__, cal->cal, st));
			/* move to next cal. */
			cache->cal_cur ++;
			break;
		}
		cache->cur_cmid = ci->cal_tbl[cache->cal_cur].cmid;

		/* N.B.: The duration entry 0 is the first entry of
		 * the first calibration; the next duration entry is
		 * the first entry of the next calibration.
		 */
		if (meas &&
		    /* it is moving to the next cal */
		    cache->cal_cur > cal_cur) {
			ci->cal_dur[cache->cal_cur] = ci->dur_cnt;
		}

		/* save when moving to the next cal */
		if (cache->cal_cur > cal_cur) {
			save = TRUE;
		}
		/* save only in MPHASE mode when moving to the next phase */
		else {
			save = mode == PHY_CALMGR_CAL_MODE_MPHASE;
		}

		/* save the phase or cal result */
		if (save &&
		    (ci->flags & CALMGR_FLAG_BYPASS_CACHE) == 0) {
			if ((int)cal->ccid >= 0)
				phy_cache_save(pi->cachei, cal->ccid);
		}

		/* perform another phase or cal */
		if (mode != PHY_CALMGR_CAL_MODE_MPHASE)
			continue;

		/* do we have time for another phase or cal? */
		if (speedy &&
		    (int)*dur - ci->dur_tbl[ci->dur_cnt] > CALMGR_SPEEDY_CAL_DUR_MARGIN) {
			/* TODO: scehdule a 0-length timer */
		}

		/* we are done for now, will be back on next invocation */
		break;
	}

	/* PHY specific cleanup ... */
	if (fns->cleanup != NULL)
		(fns->cleanup)(fns->ctx);

	/* Common cleanup... */

	if (cache->cal_cur == ci->cal_cnt) {
		/* measurement is done only on the first MPHASE calibration */
		if (meas)
			ci->flags |= CALMGR_FLAG_DUR_MEASURED;
		/* reset states */
		_phy_calmgr_reset(ci, CALMGR_ST_DONE);
	}

	return cache->cal_state == CALMGR_ST_INPROG;
}

static int
phy_calmgr_reset(phy_calmgr_info_t *ci)
{
	phy_calmgr_cache_t *cache = ci->ctx;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (cache->cal_state == CALMGR_ST_INPROG) {
		uint cal_cur;

		ASSERT(cache->cal_cur < ci->cal_cnt);

		cal_cur = cache->cal_cur;
		if (ci->cal_tbl[cal_cur].reset != NULL)
			ci->cal_tbl[cal_cur].reset(ci->cal_tbl[cal_cur].ctx);
	}

	_phy_calmgr_reset(ci, CALMGR_ST_UNK);

	return BCME_OK;
}

/* request a calibration */
int
phy_calmgr_req_cal(phy_calmgr_info_t *ci, phy_calmgr_cal_req_t *req)
{
	PHY_TRACE(("%s\n", __FUNCTION__));

	/* TODO: force a cache save operation */

	phy_calmgr_reset(ci);
	if (req->cache == 0)
		ci->flags |= CALMGR_FLAG_BYPASS_CACHE;
	ci->flags |= CALMGR_FLAG_FORCED_CAL;
	if (req->mode == PHY_CALMGR_CAL_MODE_SSHOT)
		phy_calmgr_docal(ci, PHY_CALMGR_CAL_MODE_SSHOT, 0);

	/* TODO: force a cache restore operation */

	return BCME_OK;
}

/* configure the calibration, must be done prior to any calibration */
int
phy_calmgr_config_cal(phy_calmgr_info_t *ci, phy_calmgr_cal_cfg_t *cfg)
{
	phy_calmgr_cache_t *cache = ci->ctx;

	cache->init_mode = cfg->init_mode;
	cache->cal_prd = cfg->cal_prd;

	cache->flags |= CACHE_FLAG_CFG;

	return BCME_OK;
}

/* query calibration info */
void
phy_calmgr_query_cal(phy_calmgr_info_t *ci, phy_calmgr_cal_info_t *st)
{
	phy_calmgr_cache_t *cache = ci->ctx;

	st->in_prog = cache->cal_state == CALMGR_ST_INPROG;
	st->cur_cmid = cache->cur_cmid;
	st->last_err = cache->last_err;
	st->last_cmid = cache->last_cmid;
}

/* add calibration callback entry */
int
BCMATTACHFN(phy_calmgr_add_cal_fn)(phy_calmgr_info_t *ci,
	phy_calmgr_cal_fn_t cal, phy_calmgr_reset_fn_t reset, phy_calmgr_cal_ctx_t *ctx,
        phy_calmgr_cal_id_t cmid, phy_cache_cubby_id_t ccid)
{
	uint i;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (ci->cal_cnt == ci->cal_sz) {
		PHY_ERROR(("%s: too many callbacks\n", __FUNCTION__));
		return BCME_NORESOURCE;
	}

	ASSERT(cal != NULL);

	/* linear search the registered callbacks table */
	for (i = 0; i < ci->cal_cnt; i ++) {
		/* insert it right at here */
		if (cmid == ci->cal_tbl[i].cmid ||
		    ((i == 0 || cmid > ci->cal_tbl[i - 1].cmid) &&
		     cmid <= ci->cal_tbl[i].cmid)) {
			PHY_CALMGR(("%s: insert %u\n", __FUNCTION__, i));
			break;
		}
	}
	/* insert at after all existing entries when i == cal_cnt */
	if (i == ci->cal_cnt) {
		PHY_CALMGR(("%s: append %u\n", __FUNCTION__, i));
	}

	/* move down callbacks down by 1 entry */
	if (i < ci->cal_cnt) {
		memmove(&ci->cal_tbl[i + 1], &ci->cal_tbl[i],
		        (ci->cal_cnt - i) * sizeof(*ci->cal_tbl));
	}

	/* populate the entry */
	ci->cal_tbl[i].cal = cal;
	ci->cal_tbl[i].reset = reset;
	ci->cal_tbl[i].ctx = ctx;
	ci->cal_tbl[i].cmid = cmid;
	ci->cal_tbl[i].ccid = ccid;

	ci->cal_cnt ++;

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
#endif /* BCMDBG || BCMDBG_DUMP */

	return BCME_OK;
}

#ifdef TEST_CALM
static phy_calmgr_cal_status_t
phy_calmgr_test_cal(phy_calmgr_cal_ctx_t *ctx, phy_calmgr_cal_mode_t mode)
{
	phy_calmgr_cal_status_t st = PHY_CALMGR_CAL_ST_CAL_DONE;
	static uint tick = 0;
	switch (tick % 3) {
	case 0:
	case 1:
		st = PHY_CALMGR_CAL_ST_PHASE_DONE;
		break;
	case 2:
		st = PHY_CALMGR_CAL_ST_CAL_DONE;
		break;
	}
	tick ++;
	return st;
}
#endif /* TEST_CALM */

/* add trigger callback entry */
int
BCMATTACHFN(phy_calmgr_add_trigger_fn)(phy_calmgr_info_t *ci,
	phy_calmgr_trigger_fn_t fn, phy_calmgr_trigger_ctx_t *ctx,
	phy_calmgr_trigger_id_t tid)
{
	uint i;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (ci->trigger_cnt == ci->trigger_sz) {
		PHY_ERROR(("%s: too many callbacks\n", __FUNCTION__));
		return BCME_NORESOURCE;
	}

	ASSERT(fn != NULL);

	/* linear search the registered callbacks table */
	for (i = 0; i < ci->trigger_cnt; i ++) {
		/* insert it right at here */
		if (tid == ci->trigger_tbl[i].tid ||
		    ((i == 0 || tid > ci->trigger_tbl[i - 1].tid) &&
		     tid <= ci->trigger_tbl[i].tid)) {
			PHY_CALMGR(("%s: insert %u\n", __FUNCTION__, i));
			break;
		}
	}
	/* insert at after all existing entries when i == trigger_cnt */
	if (i == ci->trigger_cnt) {
		PHY_CALMGR(("%s: append %u\n", __FUNCTION__, i));
	}

	/* move down callbacks down by 1 entry */
	if (i < ci->trigger_cnt) {
		memmove(&ci->trigger_tbl[i + 1], &ci->trigger_tbl[i],
		        (ci->trigger_cnt - i) * sizeof(*ci->trigger_tbl));
	}

	/* populate the entry */
	ci->trigger_tbl[i].fn = fn;
	ci->trigger_tbl[i].ctx = ctx;
	ci->trigger_tbl[i].tid = tid;

	ci->trigger_cnt ++;

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
#endif /* BCMDBG || BCMDBG_DUMP */

	return BCME_OK;
}

/* watchdog callback */
static bool
phy_calmgr_wd(phy_wd_ctx_t *ctx)
{
	phy_calmgr_info_t *ci = (phy_calmgr_info_t *)ctx;
	phy_calmgr_cache_t *cache = ci->ctx;
	phy_calmgr_cal_mode_t mode;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (cache->cal_state == CALMGR_ST_UNK) {
		/* make sure we are in the right state */
		phy_calmgr_reset(ci);
		/* use the configured initial mode */
		mode = cache->init_mode;
	}
	else
		mode = PHY_CALMGR_CAL_MODE_MPHASE;
	ci->left_dur = ci->allow_dur;
	phy_calmgr_docal(ci, mode, &ci->left_dur);

	return TRUE;
}

/* initial trigger callback */
static bool
phy_calmgr_initial_trigger(phy_calmgr_trigger_ctx_t *ctx)
{
	phy_calmgr_info_t *ci = (phy_calmgr_info_t *)ctx;
	phy_calmgr_cache_t *cache = ci->ctx;
	phy_info_t *pi = ci->pi;

	(void)pi;

	PHY_TRACE(("%s: state %d cnt %u now %u\n",
	           __FUNCTION__, cache->cal_state, ci->cal_cnt, pi->sh->now));

	return cache->cal_state == CALMGR_ST_UNK ||
	        (ci->flags & CALMGR_FLAG_FORCED_CAL);
}

/* periodic trigger callback */
static bool
phy_calmgr_period_trigger(phy_calmgr_trigger_ctx_t *ctx)
{
	phy_calmgr_info_t *ci = (phy_calmgr_info_t *)ctx;
	phy_calmgr_cache_t *cache = ci->ctx;
	phy_info_t *pi = ci->pi;

	PHY_TRACE(("%s: state %d cnt %u now %u\n",
	           __FUNCTION__, cache->cal_state, ci->cal_cnt, pi->sh->now));

	return cache->cal_state == CALMGR_ST_DONE &&
	        pi->sh->now - cache->last_start >= cache->cal_prd;
}

#ifdef NEW_PHY_CAL_ARCH
/* chspec notification callback */
static int
phy_calmgr_chspec_notif(phy_chanmgr_notif_ctx_t *ctx, phy_chanmgr_notif_data_t *data)
{
	phy_calmgr_info_t *ci = (phy_calmgr_info_t *)ctx;

	return phy_calmgr_reset(ci);
}
#endif // endif

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static int
phy_calmgr_dump_ctx(phy_cache_ctx_t *ctx, uint8 *buf, struct bcmstrbuf *b)
{
#ifdef NEW_PHY_CAL_ARCH
	phy_calmgr_cache_t *cache = (phy_calmgr_cache_t *)buf;

	bcm_bprintf(b, "calmgr:\n");
	bcm_bprintf(b, "  cal_cur %d state %u last_start %u\n",
	            cache->cal_cur, cache->cal_state, cache->last_start);
#endif // endif
	return BCME_OK;
}

static int
phy_calmgr_dump(void *ctx, struct bcmstrbuf *b)
{
	phy_calmgr_info_t *info = (phy_calmgr_info_t *)ctx;
	uint i;

	phy_calmgr_dump_ctx(info, (uint8 *)info->ctx, b);

	bcm_bprintf(b, "stats:\n");
	bcm_bprintf(b, "  nodur %u\n", info->stats->nodur);

	bcm_bprintf(b, "trigger: max %u cnt %u\n", info->trigger_sz, info->trigger_cnt);
	for (i = 0; i < info->trigger_cnt; i ++) {
		bcm_bprintf(b, "  idx %u: tid %u fn %p ctx %p\n",
		            i, info->trigger_tbl[i].tid,
		            info->trigger_tbl[i].fn, info->trigger_tbl[i].ctx);
	}

	bcm_bprintf(b, "cal: max %u cnt %u\n", info->cal_sz, info->cal_cnt);
	for (i = 0; i < info->cal_cnt; i ++) {
		bcm_bprintf(b, "  idx %u: cmid %u cal %p reset %p ctx %p dur %u\n",
		            i, info->cal_tbl[i].cmid,
		            info->cal_tbl[i].cal, info->cal_tbl[i].reset, info->cal_tbl[i].ctx,
		            info->cal_tbl[i].ccid, info->cal_dur[i]);
	}

	bcm_bprintf(b, "dur: max %u cnt %u\n", info->dur_sz, info->dur_cnt);
	for (i = 0; i < info->dur_cnt; i ++) {
		bcm_bprintf(b, "  idx %u: dur %u\n",
		            i, info->dur_tbl[i]);
	}

	return BCME_OK;
}
#endif /* BCMDBG || BCMDBG_DUMP */
