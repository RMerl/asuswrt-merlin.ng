/*
 * NOISEmeasure module implementation.
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
#include <bcmdefs.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include <phy_init.h>
#include "phy_type_noise.h"
#include <phy_noise.h>

/* module private states */
struct phy_noise_info {
	phy_info_t *pi;
	phy_type_noise_fns_t *fns;
	int bandtype;
};

/* module private states memory layout */
typedef struct {
	phy_noise_info_t info;
	phy_type_noise_fns_t fns;
/* add other variable size variables here at the end */
} phy_noise_mem_t;

/* local function declaration */
static int phy_noise_reset(phy_init_ctx_t *ctx);
static int phy_noise_init(phy_init_ctx_t *ctx);
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static int phy_noise_dump(void *ctx, struct bcmstrbuf *b);
#endif // endif
static bool phy_noise_start_wd(phy_wd_ctx_t *ctx);
static bool phy_noise_stop_wd(phy_wd_ctx_t *ctx);
static bool phy_noise_reset_wd(phy_wd_ctx_t *ctx);

/* attach/detach */
phy_noise_info_t *
BCMATTACHFN(phy_noise_attach)(phy_info_t *pi, int bandtype)
{
	phy_noise_info_t *info;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate attach info storage */
	if ((info = phy_malloc(pi, sizeof(phy_noise_mem_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	info->pi = pi;
	info->fns = &((phy_noise_mem_t *)info)->fns;

	/* Initialize noise params */
	info->bandtype = bandtype;

	pi->interf->curr_home_channel = CHSPEC_CHANNEL(pi->radio_chanspec);
	pi->sh->interference_mode_override = FALSE;
	pi->sh->interference_mode_2G = INTERFERE_NONE;
	pi->sh->interference_mode_5G = INTERFERE_NONE;

	/* Default to 60. Each PHY specific attach should initialize it
	 * to PHY/chip specific.
	 */
	pi->aci_exit_check_period = 60;
	pi->aci_enter_check_period = 16;
	pi->aci_state = 0;

	pi->interf->scanroamtimer = 0;
	pi->interf->rssi_index = 0;
	pi->interf->rssi = 0;

	/* register watchdog fn */
	if (phy_wd_add_fn(pi->wdi, phy_noise_start_wd, info,
	                  PHY_WD_PRD_1TICK, PHY_WD_1TICK_NOISE_START,
	                  PHY_WD_FLAG_SCAN_SKIP|PHY_WD_FLAG_PLT_SKIP) != BCME_OK) {
		PHY_ERROR(("%s: phy_wd_add_fn failed\n", __FUNCTION__));
		goto fail;
	}
	if (phy_wd_add_fn(pi->wdi, phy_noise_stop_wd, info,
	                  PHY_WD_PRD_1TICK, PHY_WD_1TICK_NOISE_STOP,
	                  PHY_WD_FLAG_NONE) != BCME_OK) {
		PHY_ERROR(("%s: phy_wd_add_fn failed\n", __FUNCTION__));
		goto fail;
	}
	if (phy_wd_add_fn(pi->wdi, phy_noise_reset_wd, info,
	                  PHY_WD_PRD_1TICK, PHY_WD_1TICK_NOISE_RESET,
	                  PHY_WD_FLAG_NONE) != BCME_OK) {
		PHY_ERROR(("%s: phy_wd_add_fn failed\n", __FUNCTION__));
		goto fail;
	}

	/* register reset fn */
	if (phy_init_add_init_fn(pi->initi, phy_noise_reset, info, PHY_INIT_NOISERST)
			!= BCME_OK) {
		PHY_ERROR(("%s: phy_init_add_init_fn failed\n", __FUNCTION__));
		goto fail;
	}

	/* register init fn */
	if (phy_init_add_init_fn(pi->initi, phy_noise_init, info, PHY_INIT_NOISE) != BCME_OK) {
		PHY_ERROR(("%s: phy_init_add_init_fn failed\n", __FUNCTION__));
		goto fail;
	}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
	/* register dump callback */
	phy_dbg_add_dump_fn(pi, "phynoise", (phy_dump_fn_t)phy_noise_dump, info);
#endif // endif

	return info;

	/* error */
fail:
	phy_noise_detach(info);
	return NULL;
}

void
BCMATTACHFN(phy_noise_detach)(phy_noise_info_t *info)
{
	phy_info_t *pi;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (info == NULL) {
		PHY_INFORM(("%s: null noise module\n", __FUNCTION__));
		return;
	}

	pi = info->pi;

	phy_mfree(pi, info, sizeof(phy_noise_mem_t));
}

/* watchdog callbacks */
static bool
phy_noise_start_wd(phy_wd_ctx_t *ctx)
{
	phy_noise_info_t *nxi = (phy_noise_info_t *)ctx;
	phy_info_t *pi = nxi->pi;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* update phy noise moving average only if no scan or rm in progress */
	wlc_phy_noise_sample_request(pi, PHY_NOISE_SAMPLE_MON,
	                             CHSPEC_CHANNEL(pi->radio_chanspec));

	return TRUE;
}

static bool
phy_noise_stop_wd(phy_wd_ctx_t *ctx)
{
	phy_noise_info_t *nxi = (phy_noise_info_t *)ctx;
	phy_type_noise_fns_t *fns = nxi->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (fns->stop != NULL)
		(fns->stop)(fns->ctx);

	return TRUE;
}

/* register phy type specific implementations */
int
BCMATTACHFN(phy_noise_register_impl)(phy_noise_info_t *nxi, phy_type_noise_fns_t *fns)
{
	phy_info_t *pi = nxi->pi;

	PHY_TRACE(("%s\n", __FUNCTION__));

	*nxi->fns = *fns;

	/* N.B.: This is the signal of module attach completion. */
	/* Summarize the PHY type setting... */
	if (BAND_2G(nxi->bandtype)) {
		pi->sh->interference_mode =
			pi->sh->interference_mode_2G;
	} else {
		pi->sh->interference_mode =
			pi->sh->interference_mode_5G;
	}

	return BCME_OK;
}

void
BCMATTACHFN(phy_noise_unregister_impl)(phy_noise_info_t *ti)
{
	PHY_TRACE(("%s\n", __FUNCTION__));
}

/* change mode */
int
phy_noise_set_mode(phy_noise_info_t *ii, int wanted_mode, bool init)
{
	phy_type_noise_fns_t *fns = ii->fns;
	phy_info_t *pi = ii->pi;

	PHY_TRACE(("%s: mode %d init %d\n", __FUNCTION__, wanted_mode, init));

	if (fns->mode != NULL)
		(fns->mode)(fns->ctx, wanted_mode, init);

	pi->cur_interference_mode = wanted_mode;
	return BCME_OK;
}

static bool
phy_noise_reset_wd(phy_wd_ctx_t *ctx)
{
	phy_noise_info_t *nxi = (phy_noise_info_t *)ctx;
	phy_info_t *pi = nxi->pi;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* reset phynoise state if ucode interrupt doesn't arrive for so long */
	if (pi->phynoise_state && (pi->sh->now - pi->phynoise_now) > 5) {
		PHY_TMP(("wlc_phy_watchdog: ucode phy noise sampling overdue\n"));
		pi->phynoise_state = 0;
	}

	return TRUE;
}

/* driver up/init processing */
static int
WLBANDINITFN(phy_noise_init)(phy_init_ctx_t *ctx)
{
	phy_noise_info_t *ii = (phy_noise_info_t *)ctx;
	phy_info_t *pi = ii->pi;

	PHY_TRACE(("%s\n", __FUNCTION__));

	    /* do not reinitialize interference mode, could be scanning */
	if (((ISNPHY(pi) && NREV_GE(pi->pubpi.phy_rev, 3)) ||
	     ISHTPHY(pi) ||
	     ISACPHY(pi)) &&
	    SCAN_INPROG_PHY(pi))
		return BCME_OK;

	/* initialize interference algorithms */
	if (pi->sh->interference_mode_override == TRUE) {
		/* keep the same values */
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			pi->sh->interference_mode = pi->sh->interference_mode_2G_override;
		} else {
			/* for 5G, only values 0 and 1 are valid options */
			if (pi->sh->interference_mode_5G_override == 0 ||
			    pi->sh->interference_mode_5G_override == 1) {
				pi->sh->interference_mode = pi->sh->interference_mode_5G_override;
			} else {
				/* used invalid value. so default to 0 */
				pi->sh->interference_mode = 0;
			}
		}
	} else {
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			pi->sh->interference_mode = pi->sh->interference_mode_2G;
		} else {
			pi->sh->interference_mode = pi->sh->interference_mode_5G;
		}
	}

	return 	phy_noise_set_mode(ii, pi->sh->interference_mode, FALSE);
}

/* Reset */
static int
WLBANDINITFN(phy_noise_reset)(phy_init_ctx_t *ctx)
{
	phy_noise_info_t *ii = (phy_noise_info_t *)ctx;
	phy_type_noise_fns_t *fns = ii->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* Reset aci on band-change */
	if (fns->reset != NULL)
		(fns->reset)(fns->ctx);
	return BCME_OK;
}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
/* common dump functions for non-ac phy */
int
phy_noise_dump_common(phy_info_t *pi, struct bcmstrbuf *b)
{
	int channel, indx;
	int val;
	char *ptr;

	val = pi->sh->interference_mode;
	if (pi->aci_state & ACI_ACTIVE)
		val |= AUTO_ACTIVE;

	if (val & AUTO_ACTIVE)
		bcm_bprintf(b, "ACI is Active\n");
	else {
		bcm_bprintf(b, "ACI is Inactive\n");
		return BCME_OK;
	}

	for (channel = 0; channel < ACI_LAST_CHAN; channel++) {
		bcm_bprintf(b, "Channel %d : ", channel + 1);
		for (indx = 0; indx < 50; indx++) {
			ptr = &(pi->interf->aci.rssi_buf[channel][indx]);
			if (*ptr == (char)-1)
				break;
			bcm_bprintf(b, "%d ", *ptr);
		}
		bcm_bprintf(b, "\n");
	}

	return BCME_OK;
}

/* Dump rssi values from aci scans, and print phycai dump */
static int
phy_noise_dump(void *ctx, struct bcmstrbuf *b)
{
	phy_noise_info_t *nxi = (phy_noise_info_t *)ctx;
	phy_type_noise_fns_t *fns = nxi->fns;
	phy_info_t *pi = nxi->pi;

	uint32 i, idx, antidx;
	int32 tot;

	if (!pi->sh->up)
		return BCME_NOTUP;

	bcm_bprintf(b, "History and average of latest %d noise values:\n",
		PHY_NOISE_WINDOW_SZ);

	FOREACH_CORE(pi, antidx) {
		tot = 0;
		bcm_bprintf(b, "Ant%d: [", antidx);

		idx = pi->phy_noise_index;
		for (i = 0; i < PHY_NOISE_WINDOW_SZ; i++) {
			bcm_bprintf(b, "%4d", pi->phy_noise_win[antidx][idx]);
			tot += pi->phy_noise_win[antidx][idx];
			idx = MODINC_POW2(idx, PHY_NOISE_WINDOW_SZ);
		}
		bcm_bprintf(b, "]");

		tot /= PHY_NOISE_WINDOW_SZ;
		bcm_bprintf(b, " [%4d]\n", tot);
	}

	/* Print phyaci dump */
	if (fns->dump != NULL)
		(fns->dump)(fns->ctx, b);

	return BCME_OK;
}
#endif /* BCMDBG || BCMDBG_DUMP */
