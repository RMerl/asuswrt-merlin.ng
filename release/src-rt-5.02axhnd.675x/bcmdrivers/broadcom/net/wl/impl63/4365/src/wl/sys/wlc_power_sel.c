/*
 * Common [OS-independent] power selection algorithm of Broadcom
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
 * $Id: wlc_power_sel.c 708017 2017-06-29 14:11:45Z $
 */

#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>
#include <wlioctl.h>
#include <bcmdevs.h>
#include <proto/802.11.h>
#include <d11.h>

#include <wlc_pub.h>
#include <wlc_bsscfg.h>
#include <wlc.h>

#include <wlc_scb_ratesel.h>
#include <wlc_power_sel.h>

#ifdef WLLMAC
#include <wlc_lmac.h>
#endif // endif

#define NOW(wlc)		(wlc->pub->now)

/* iovar table */
enum {
	IOV_LPC,
	IOV_LPC_PARAMS,
	IOV_LPC_MSGLEVEL
};

static const bcm_iovar_t lpc_iovars[] = {
	{"lpc", IOV_LPC,
	(IOVF_SET_DOWN), IOVT_BOOL, 0
	},
	{"lpc_params", IOV_LPC_PARAMS,
	(0), IOVT_BUFFER, sizeof(lpc_params_t)
	},
	{"lpc_msglevel", IOV_LPC_MSGLEVEL,
	(0), IOVT_UINT8, 0},
	{NULL, 0, 0, 0, 0}
};

#define LPC_MAX_IDX 	0

/* Global LPC Parameter default values */
/* Number of successful MSDUs before LPC kicks in */
#define LPC_RATE_STAB_THRESH	10
/* Number of successful MSDUs at the current power level before LPC considers decreasing
* the power again
*/
#define LPC_PWR_STAB_THRESH 	15
/* Amount of time (in secs) after which the LPC database for the particular link expires */
#define LPC_STAB_EXPIRY 		100

/* The steps in which the power is increased when the last attempt was good but the
* throughput ratio is still below the threshold
*/
#define LPC_STEPUP_SLOW		2
/* The steps in which the power is increased when the last attempt was bad and the
* throughput ratio is below the threshold
*/
#define LPC_STEPUP_FAST		6
/* The steps in which the power is decreased under good link conditions */
#define LPC_STEPDN_SLOW		1
#define LPC_STEPDN_FAST		2 /* Not used currently */

uint8 lpc_msglevel = 0;

/* A  global structure which contains the tunable parameters for */
/* the power selection algorithm. This is initialized when LPC is attached */
struct lpc_info {
	wlc_info_t	*wlc;		/* pointer to main wlc structure */
	wlc_pub_t	*pub;		/* public common code handler */
	osl_t		*osh;		/* pointer to OSL handler */

	/* Global params exposed via IOVAR */
	lpc_params_t	lpc_params;
};

/* Structure which contains parameters for links currently active. */
struct lcb {
	struct scb	*scb;			/* back pointer to scb */
	struct lpc_info *lpci;	/* back pointer to LPC module local structure */

	uint32		last_update_time;	/* Time of last refresh */
	uint8		power_stab_count;	/* Num of times MSDU succeded with this power */
	int8		curr_pwr_level_idx;	/* The power level index in effect */
	bool		info_expired;		/* Is the LPC history stale now? */
	uint16		phy_ctrl_word;		/* Current PHY specifc info for the link */
};

static int wlc_lpc_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *p, uint plen, void *a, int alen, int val_size, struct wlc_if *wlcif);
static void wlc_lpc_refresh_info(lcb_t *state, uint32 now);
static uint8 wlc_lpc_step_up(lcb_t *state, uint8 steps);
static uint8 wlc_lpc_step_down(lcb_t *state, uint8 steps);
static uint8 wlc_lpc_nominal(lcb_t *state);
static void wlc_lpc_maintain_power(lcb_t *state);
static void wlc_lpc_reset_stab_count(lcb_t *state);
#ifdef WL_LPC_DEBUG
static void wlc_lpc_print_dbm(wlc_info_t *wlc, lcb_t *state);
#endif // endif

/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */
#include <wlc_patch.h>

lpc_info_t *
BCMATTACHFN(wlc_lpc_attach)(wlc_info_t *wlc)
{
	lpc_info_t *lpci;

	if (!(lpci = (lpc_info_t *)MALLOCZ(wlc->osh, sizeof(lpc_info_t)))) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		return NULL;
	}

	lpci->wlc = wlc;
	lpci->osh = wlc->osh;
	lpci->pub = wlc->pub;

	/* register module */
	if (wlc_module_register(lpci->pub, lpc_iovars, "lpc", lpci, wlc_lpc_doiovar,
		NULL, NULL, NULL)) {
		WL_ERROR(("wl%d: %s:wlc_module_register failed\n", lpci->pub->unit, __FUNCTION__));
		goto fail;
	}

	if (wlc_lpc_capable_chip(wlc)) {
		lpc_params_t *lpc_params = &lpci->lpc_params;

		/* Initialize the global LPC structure. */
		lpc_params->rate_stab_thresh = LPC_RATE_STAB_THRESH;
		lpc_params->pwr_stab_thresh = LPC_PWR_STAB_THRESH;
		lpc_params->lpc_exp_time = LPC_STAB_EXPIRY;
		lpc_params->pwrup_slow_step = LPC_STEPUP_SLOW;
		lpc_params->pwrup_fast_step = LPC_STEPUP_FAST;
		lpc_params->pwrdn_slow_step = LPC_STEPDN_SLOW;
		/* lpci->lpc_algo = TRUE; to enable the algo by default */
	}

	return lpci;

fail:
	MFREE(lpci->osh, lpci, sizeof(lpc_info_t));
	return NULL;
}

void
BCMATTACHFN(wlc_lpc_detach)(lpc_info_t *lpci)
{
	if (!lpci)
		return;

	/* No need to disable the Phy specific vars, as detaching. Just unregister and free */
	wlc_module_unregister(lpci->pub, "lpc", lpci);
	MFREE(lpci->osh, lpci, sizeof(lpc_info_t));
}

/* handle LPC related iovars */
static int
wlc_lpc_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *p, uint plen, void *a, int alen, int val_size, struct wlc_if *wlcif)
{
	lpc_info_t *lpci = (lpc_info_t *)hdl;
	int32 int_val = 0;
	bool bool_val = FALSE;
	uint8 uint8_val = 0;
	int err = 0;
	wlc_info_t *wlc;
	int32 *ret_int_ptr;

	ret_int_ptr = (int32 *)a;

	if (plen >= (int)sizeof(int_val)) {
		bcopy(p, &int_val, sizeof(int_val));
		bool_val = (bool)int_val;
		uint8_val = (uint8)int_val;
	}

	wlc = lpci->wlc;
	BCM_REFERENCE(wlc);

	switch (actionid) {
		case IOV_GVAL(IOV_LPC):
		{
			/* Only some chips are supported */
			if (wlc_lpc_capable_chip(wlc)) {
				/* Get the LPC mode */
				*ret_int_ptr = wlc->pub->_lpc_algo;
			}
			else
				err = BCME_UNSUPPORTED;
		}
		break;

		case IOV_SVAL(IOV_LPC):
		{
			/* Only some chips are supported */
			if (wlc_lpc_capable_chip(wlc)) {
				/* Set the LPC mode */
				wlc->pub->_lpc_algo = bool_val;
				wlc_phy_lpc_algo_set(WLC_PI(wlc), bool_val);
			}
			else
				err = BCME_UNSUPPORTED;
		}
		break;

		case IOV_GVAL(IOV_LPC_PARAMS):
		{
			/* Only some chips are supported */
			if ((wlc_lpc_capable_chip(wlc)) && wlc->pub->_lpc_algo) {
				lpc_params_t *lpc_params = &lpci->lpc_params;
				lpc_params_t *ret_params = (lpc_params_t *) a;
				bcopy(lpc_params, ret_params, sizeof(lpc_params_t));
			} else
				err = BCME_UNSUPPORTED;
		}
		break;

		case IOV_SVAL(IOV_LPC_PARAMS):
		{
			/* Only some chips are supported */
			if ((wlc_lpc_capable_chip(wlc)) && wlc->pub->_lpc_algo) {
				lpc_params_t *lpc_params = &lpci->lpc_params;
				lpc_params_t *ip_params = (lpc_params_t *) p;
				bcopy(ip_params, lpc_params, sizeof(lpc_params_t));
			} else
				err = BCME_UNSUPPORTED;
		}
		break;

		case IOV_GVAL(IOV_LPC_MSGLEVEL):
		{
			/* Only some chips are supported */
			if ((wlc_lpc_capable_chip(wlc)) && wlc->pub->_lpc_algo)
				*ret_int_ptr = lpc_msglevel;
			else
				err = BCME_UNSUPPORTED;
		}
		break;

		case IOV_SVAL(IOV_LPC_MSGLEVEL):
		{
			if ((wlc_lpc_capable_chip(wlc)) && wlc->pub->_lpc_algo)
				lpc_msglevel = uint8_val;
			else
				err = BCME_UNSUPPORTED;
		}
		break;

		default:
			err = BCME_UNSUPPORTED;
	}

	return err;
}

#ifdef BCMDBG
void
wlc_lpc_dump_lcb(lcb_t *lcb, int32 ac, struct bcmstrbuf *b)
{
	if (!lcb)
		return;

	bcm_bprintf(b, "\tAC[%d] --- ", ac);
	return;
}
#endif /* BCMDBG */

static void
wlc_lpc_refresh_info(lcb_t *state, uint32 now)
{
	/* Refresh the database for future use */
	state->last_update_time = now;
	state->info_expired = FALSE;
}

static uint8
wlc_lpc_step_up(lcb_t *state, uint8 steps)
{
	/* Reset the stability count */
	wlc_lpc_reset_stab_count(state);

	/* Switch to a higher power level */
	state->curr_pwr_level_idx -= steps;
	if (state->curr_pwr_level_idx < LPC_MAX_IDX)
		state->curr_pwr_level_idx = LPC_MAX_IDX;
	return state->curr_pwr_level_idx;
}

static uint8
wlc_lpc_step_down(lcb_t *state, uint8 steps)
{
	wlc_info_t *wlc = state->lpci->wlc;
	uint8 min_idx = wlc_phy_lpc_getminidx(WLC_PI(wlc));

	/* Reset the stability count */
	wlc_lpc_reset_stab_count(state);

	/* Switch to a lower power level */
	state->curr_pwr_level_idx += steps;
	if (state->curr_pwr_level_idx >= min_idx)
		state->curr_pwr_level_idx = min_idx;
	return state->curr_pwr_level_idx;
}

static uint8
wlc_lpc_nominal(lcb_t *state)
{
	/* Reset the stability count */
	wlc_lpc_reset_stab_count(state);

	/* Switch to a lower power level */
	state->curr_pwr_level_idx = LPC_MAX_IDX;
	return state->curr_pwr_level_idx;
}

static void
wlc_lpc_maintain_power(lcb_t *state)
{
	/* Maintain Tx power */
	state->power_stab_count++;
}

static void
wlc_lpc_reset_stab_count(lcb_t *state)
{
	state->power_stab_count = 0;
}

#ifdef WL_LPC_DEBUG
static void
wlc_lpc_print_dbm(wlc_info_t *wlc, lcb_t *state)
{
	uint8 *lpc_pwr_level = wlc_phy_lpc_get_pwrlevelptr(WLC_PI(wlc));
	uint8 pwrlvl_macaddr_lut =
		lpc_pwr_level[state->curr_pwr_level_idx];
	int8 pwr_lvl_dbm = 0;
	uint8 pwr_lvl_dbm_frac = 0;
#if defined(WL11AC)
	uint8 db_incr = 2; /* 0.5 dB steps */
#elif defined(LCN40CONF)
	uint8 db_incr = 4; /* 0.25 dB steps */
#endif // endif

	uint8 nom_pwr = 0;
	pwr_lvl_dbm = nom_pwr - (pwrlvl_macaddr_lut / db_incr);
	pwr_lvl_dbm_frac = pwrlvl_macaddr_lut % db_incr;
	if (pwr_lvl_dbm_frac) {
		pwr_lvl_dbm_frac = pwr_lvl_dbm_frac * (100 / db_incr);
		if (pwr_lvl_dbm == 0) {
			LPC_INFO((", Backoff, -%i.%d\n", pwr_lvl_dbm, pwr_lvl_dbm_frac));
			return;
		}
	}
	LPC_INFO((", Backoff, %i.%d\n", pwr_lvl_dbm, pwr_lvl_dbm_frac));
	return;
}
#endif /* WL_LPC_DEBUG */

/* Extern functions */
void
wlc_lpc_init(lpc_info_t *lpci, lcb_t *state, struct scb *scb)
{
	ASSERT(state);
	ASSERT(lpci);

	bzero((char *)state, sizeof(lcb_t));

	/* store pointer to LPC module */
	state->lpci = lpci;
	state->scb = scb;

	/* All the AC specific parameters are initialized */
	state->last_update_time = 0;
	state->power_stab_count = 0;
	state->curr_pwr_level_idx = LPC_MAX_IDX;
	state->info_expired = FALSE;
	return;
}

uint8
wlc_lpc_getcurrpwr(lcb_t *state)
{
	lpc_info_t *lpci;
	wlc_info_t *wlc;
	/* Init to nominal power */
	uint8 pwr_offset = 0;

	if (!state)
		return pwr_offset;

	lpci = state->lpci;
	wlc = lpci->wlc;

	/* Confirm info not expired, then get the power index */
	if ((NOW(wlc) - state->last_update_time) < lpci->lpc_params.lpc_exp_time) {
		/* Query the power to be used for a particular SCB/AC */
		pwr_offset = wlc_phy_lpc_getoffset(WLC_PI(wlc), state->curr_pwr_level_idx);
	} else
		state->info_expired = TRUE;

	return pwr_offset;
}

void
wlc_lpc_update_pwr(lcb_t *state, uint8 ac, uint16 phy_ctrl_word)
{
	uint32 new_rate_kbps;
	wlc_info_t *wlc = state->lpci->wlc;
	lpc_info_t *lpci = state->lpci;
	lpc_params_t *lpc_params = &lpci->lpc_params;
	wlc_ratesel_info_t *wrsi = wlc->wrsi;
	struct scb *scb = state->scb;
	rate_lcb_info_t lcb_info;
	bool rate_stable, status = FALSE;
	uint8 pwridx_in_cubby = state->curr_pwr_level_idx;

#ifdef WL_LPC_DEBUG
	/* The vars below are for debug purposes only */
	int8 dbg_tpr_surplus;
#endif // endif

	/* Gather all the power sel related rate selection information */
	wlc_scb_ratesel_get_info(wrsi, scb, ac, lpc_params->rate_stab_thresh,
		&new_rate_kbps, &rate_stable, &lcb_info);

#ifdef WL_LPC_DEBUG
	dbg_tpr_surplus = lcb_info.tpr_val - lcb_info.tpr_thresh;
#endif // endif

	/* Update the statistics, decide on the power to be used for a particular SCB/AC */

	/* Check if a. LPC database expired or
	 *			b. rate below threshold or
	 *			c. rate is unstable
	 * If yes, Tx at Nominal power
	 */
	if ((state->info_expired) || (new_rate_kbps < lcb_info.hi_rate_kbps) ||
		(!rate_stable)) {
#ifdef WL_LPC_DEBUG
		if (WL_LPC_ON()) {
			LPC_INFO(("Rate, %d", new_rate_kbps));
			LPC_MORE((", LA_good, , , TR_Surplus, , TR_good, , NOM"));
			if (state->info_expired)
				LPC_MORE((", (Info_Expired)"));
			else if (new_rate_kbps < lcb_info.hi_rate_kbps)
				LPC_MORE((", (Low_Rate)"));
			else if (!rate_stable)
				LPC_MORE((", (Rate_Unstable)"));
		}
#endif /* WL_LPC_DEBUG */

		/* Go at nominal power */
		wlc_lpc_nominal(state);

		status = TRUE;
		goto exit;
	} else {
		/* Check if we can use the information gathered from the status received */
		if (!(lcb_info.tpr_good_valid && lcb_info.la_good_valid))
			goto exit;

		/* Is pwr level and PPR from status relevant for update. If not, exit */
		if (phy_ctrl_word != state->phy_ctrl_word)
			goto exit;
	}

	/* Process the status to decide the power level */
	LPC_INFO(("Rate, %d", new_rate_kbps));
	LPC_MORE((", LA_good, %d, TR_Surplus, %i, TR_good, %d",
		lcb_info.la_good, dbg_tpr_surplus, lcb_info.tpr_good));
	if (lcb_info.la_good) {
		/* Is the throughput ratio bad */
		if (!lcb_info.tpr_good) {
			/* The link quality is not yet good */
			wlc_lpc_step_up(state, lpc_params->pwrup_slow_step);
			LPC_MORE((", SUP, 0"));
		} else {
			if ((state->power_stab_count >= lpc_params->pwr_stab_thresh)) {
				/* The link quality has stabilized */
				wlc_lpc_step_down(state, lpc_params->pwrdn_slow_step);
				LPC_MORE((", SDN, 0"));
			} else {
				/* Link non yet stable, maintain Tx power */
				wlc_lpc_maintain_power(state);
				LPC_MORE((", MTN, 0"));
			}
		}
	} else {
		/* If the throughput drop is drastic */
		if (!lcb_info.tpr_good) {
			/* The link quality has deteriorated */
			wlc_lpc_step_up(state, lpc_params->pwrup_fast_step);
			LPC_MORE((", FUP, 0"));
		} else {
			/* The link quality is not yet good */
			if ((state->power_stab_count >= lpc_params->pwr_stab_thresh / 2)) {
				LPC_MORE((", MTN, -1"));
			} else {
				wlc_lpc_step_up(state, lpc_params->pwrup_slow_step);
				LPC_MORE((", SUP, -2"));
			}
		}
		/* Reset stability count */
		wlc_lpc_reset_stab_count(state);
	}
	status = TRUE;

exit:
	if (status) {
		/* Refresh the database for future use, if valid txstatus */
		wlc_lpc_refresh_info(state, NOW(wlc));

		LPC_MORE((", TPR_Thresh, %d", lcb_info.tpr_thresh));

#ifdef WL_LPC_DEBUG
	if (WL_LPC_ON())
		wlc_lpc_print_dbm(wlc, state);
#endif // endif
	}

	/* In case the power level has been changed in this function, clear cache */
	if (pwridx_in_cubby != state->curr_pwr_level_idx) {
		/* clear the external 'clear-on-update' variable */
		wlc_scb_ratesel_clr_cache(wrsi, scb, ac);
	}

	return;
}

void
wlc_lpc_store_pcw(lcb_t *state, uint16 phy_ctrl_word)
{
	ASSERT(state);
	state->phy_ctrl_word = phy_ctrl_word;
	return;
}

int
wlc_lpc_lcb_sz(void)
{
	return (sizeof(lcb_t));
}

bool
wlc_lpc_capable_chip(wlc_info_t *wlc)
{
	if ((WLCISLCN40PHY(wlc->band)) || (WLCISACPHY(wlc->band))) {
		/* Only some chips are supported */
		if ((CHIPID(wlc->pub->sih->chip) == BCM4334_CHIP_ID) ||
			(CHIPID(wlc->pub->sih->chip) == BCM43340_CHIP_ID) ||
			(CHIPID(wlc->pub->sih->chip) == BCM43341_CHIP_ID) ||
			(CHIPID(wlc->pub->sih->chip) == BCM43142_CHIP_ID) ||
			(CHIPID(wlc->pub->sih->chip) == BCM4345_CHIP_ID) ||
#ifdef UNRELEASEDCHIP
			BCM4349_CHIP(wlc->pub->sih->chip) ||
#endif /* UNRELEASEDCHIP */
			(CHIPID(wlc->pub->sih->chip) == BCM4335_CHIP_ID) ||
			(CHIPID(wlc->pub->sih->chip) == BCM43602_CHIP_ID) ||
			BCM4350_CHIP(wlc->pub->sih->chip)) {
				return TRUE;
		}
	}
	return FALSE;
}
