/*
 * HTPHY ACI module implementation
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
#include <bcmdevs.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include <phy_wd.h>
#include "phy_type_noise.h"
#include <phy_ht.h>
#include <phy_ht_noise.h>

/* module private states */
struct phy_ht_noise_info {
	phy_info_t *pi;
	phy_ht_info_t *hti;
	phy_noise_info_t *ii;
};

/* local functions */
static void phy_ht_noise_attach_modes(phy_info_t *pi);
static void phy_ht_noise_set_mode(phy_type_noise_ctx_t *ctx, int wanted_mode, bool init);
static void phy_ht_noise_reset(phy_type_noise_ctx_t *ctx);
static bool phy_ht_noise_noise_wd(phy_wd_ctx_t *ctx);
static bool phy_ht_noise_aci_wd(phy_wd_ctx_t *ctx);
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static int phy_ht_noise_dump(phy_type_noise_ctx_t *ctx, struct bcmstrbuf *b);
#else
#define phy_ht_noise_dump NULL
#endif // endif

/* register phy type specific implementation */
phy_ht_noise_info_t *
BCMATTACHFN(phy_ht_noise_register_impl)(phy_info_t *pi, phy_ht_info_t *hti, phy_noise_info_t *ii)
{
	phy_ht_noise_info_t *info;
	phy_type_noise_fns_t fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage together */
	if ((info = phy_malloc(pi, sizeof(phy_ht_noise_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	bzero(info, sizeof(phy_ht_noise_info_t));
	info->pi = pi;
	info->hti = hti;
	info->ii = ii;

	phy_ht_noise_attach_modes(pi);

	/* register watchdog fn */
	if (phy_wd_add_fn(pi->wdi, phy_ht_noise_noise_wd, info,
	                  PHY_WD_PRD_1TICK, PHY_WD_1TICK_INTF_NOISE,
	                  PHY_WD_FLAG_NONE) != BCME_OK) {
		PHY_ERROR(("%s: phy_wd_add_fn failed\n", __FUNCTION__));
		goto fail;
	}
	if (phy_wd_add_fn(pi->wdi, phy_ht_noise_aci_wd, info,
	                  PHY_WD_PRD_1TICK, PHY_WD_1TICK_NOISE_ACI,
	                  PHY_WD_FLAG_NONE) != BCME_OK) {
		PHY_ERROR(("%s: phy_wd_add_fn failed\n", __FUNCTION__));
		goto fail;
	}

	/* register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.mode = phy_ht_noise_set_mode;
	fns.reset = phy_ht_noise_reset;
	fns.dump = phy_ht_noise_dump;
	fns.ctx = info;

	phy_noise_register_impl(ii, &fns);

	return info;

	/* error handling */
fail:
	if (info != NULL)
		phy_mfree(pi, info, sizeof(phy_ht_noise_info_t));
	return NULL;
}

static void
BCMATTACHFN(phy_ht_noise_attach_modes)(phy_info_t *pi)
{
	if (IS_X12_BOARDTYPE(pi)) {
		pi->sh->interference_mode_2G = INTERFERE_NONE;
		pi->sh->interference_mode_5G = INTERFERE_NONE;
	} else if (IS_X29B_BOARDTYPE(pi)) {
		pi->sh->interference_mode_2G = NON_WLAN;
		pi->sh->interference_mode_5G = NON_WLAN;
	} else {
		if (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA) {
			/* XXX
			 * only mode 1 supported in 2G if ELNA present
			 */
			pi->sh->interference_mode_2G = NON_WLAN;
		} else {
			pi->sh->interference_mode_2G = WLAN_AUTO_W_NOISE;
		}
	}
}

void
BCMATTACHFN(phy_ht_noise_unregister_impl)(phy_ht_noise_info_t *info)
{
	phy_info_t *pi = info->pi;
	phy_noise_info_t *ii = info->ii;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* unregister from common */
	phy_noise_unregister_impl(ii);

	phy_mfree(pi, info, sizeof(phy_ht_noise_info_t));
}

/* set mode */
static void
phy_ht_noise_set_mode(phy_type_noise_ctx_t *ctx, int wanted_mode, bool init)
{
	phy_ht_noise_info_t *info = (phy_ht_noise_info_t *)ctx;
	phy_info_t *pi = info->pi;

	PHY_TRACE(("%s: mode %d init %d\n", __FUNCTION__, wanted_mode, init));

	if (init) {
		pi->interference_mode_crs_time = 0;
		pi->crsglitch_prev = 0;

		/* clear out all the state */
		wlc_phy_noisemode_reset_htphy(pi);
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			wlc_phy_acimode_reset_htphy(pi);
		}
	}

	/* NPHY 5G, supported for NON_WLAN and INTERFERE_NONE only */
	if (CHSPEC_IS2G(pi->radio_chanspec) ||
	    (CHSPEC_IS5G(pi->radio_chanspec) &&
	     (wanted_mode == NON_WLAN || wanted_mode == INTERFERE_NONE))) {
		if (wanted_mode == INTERFERE_NONE) {	/* disable */
			/* XXX
			 * PR114196: Set MF thresholds back to default for mode 0
			 */
			wlc_phy_noise_raise_MFthresh_htphy(pi, FALSE);

			switch (pi->cur_interference_mode) {
			case WLAN_AUTO:
			case WLAN_AUTO_W_NOISE:
			case WLAN_MANUAL:
				if (CHSPEC_IS2G(pi->radio_chanspec)) {
					wlc_phy_acimode_set_htphy(pi, FALSE,
						PHY_ACI_PWR_NOTPRESENT);
				}
				pi->aci_state &= ~ACI_ACTIVE;
				break;
			case NON_WLAN:
				if (CHSPEC_IS2G(pi->radio_chanspec)) {
					wlc_phy_acimode_set_htphy(pi,
						FALSE,
						PHY_ACI_PWR_NOTPRESENT);
					pi->aci_state &= ~ACI_ACTIVE;
				} else {
					pi->interference_mode_crs = 0;

				}
				break;
			}
		} else {	/* Enable */
			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				/* XXX
				 * PR114196: Set high MF threshold for all RSSIs in 2G if
				 * interference mode is not 0
				 */
				wlc_phy_noise_raise_MFthresh_htphy(pi, TRUE);
			}

			switch (wanted_mode) {
			case NON_WLAN:
				pi->interference_mode_crs = 1;
				break;
			case WLAN_AUTO:
			case WLAN_AUTO_W_NOISE:
				/* fall through */
				break;
			case WLAN_MANUAL:
				if (CHSPEC_IS2G(pi->radio_chanspec)) {
					wlc_phy_acimode_set_htphy(pi, TRUE,
						PHY_ACI_PWR_HIGH);
				}
				break;
			}
		}
	}
}

static void
phy_ht_noise_reset(phy_type_noise_ctx_t *ctx)
{
	phy_ht_noise_info_t *info = (phy_ht_noise_info_t *)ctx;
	phy_info_t *pi = info->pi;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* Reset ACI internals if not scanning and not in aci_detection */
	if (!(SCAN_INPROG_PHY(pi) ||
	      pi->interf->aci.nphy.detection_in_progress)) {
		wlc_phy_aci_sw_reset_htphy(pi);
	}
}

/* watchdog callback */
static bool
phy_ht_noise_noise_wd(phy_wd_ctx_t *ctx)
{
	phy_ht_noise_info_t *ii = (phy_ht_noise_info_t *)ctx;
	phy_info_t *pi = ii->pi;

	if (pi->tunings[0]) {
		pi->interf->noise.nphy_noise_assoc_enter_th = pi->tunings[0];
		pi->interf->noise.nphy_noise_noassoc_enter_th = pi->tunings[0];
	}

	if (pi->tunings[2]) {
		pi->interf->noise.nphy_noise_assoc_glitch_th_dn = pi->tunings[2];
		pi->interf->noise.nphy_noise_noassoc_glitch_th_dn = pi->tunings[2];
	}

	if (pi->tunings[1]) {
		pi->interf->noise.nphy_noise_noassoc_glitch_th_up = pi->tunings[1];
		pi->interf->noise.nphy_noise_assoc_glitch_th_up = pi->tunings[1];
	}

	return TRUE;
}

static bool
phy_ht_noise_aci_wd(phy_wd_ctx_t *ctx)
{
	phy_ht_noise_info_t *ii = (phy_ht_noise_info_t *)ctx;
	phy_info_t *pi = ii->pi;

	/* defer interference checking, scan and update if RM is progress */
	if (!SCAN_RM_IN_PROGRESS(pi)) {
		/* interf->scanroamtimer counts transient time coming out of scan */
		if (pi->interf->scanroamtimer != 0)
			pi->interf->scanroamtimer -= 1;

		wlc_phy_aci_upd(pi);

	} else {
		/* in a scan/radio meas, don't update moving average when we
		 * first come out of scan or roam
		 */
		pi->interf->scanroamtimer = 2;
	}

	return TRUE;
}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static int
phy_ht_noise_dump(phy_type_noise_ctx_t *ctx, struct bcmstrbuf *b)
{
	phy_ht_noise_info_t *info = (phy_ht_noise_info_t *)ctx;
	phy_info_t *pi = info->pi;

	return phy_noise_dump_common(pi, b);
}
#endif // endif
