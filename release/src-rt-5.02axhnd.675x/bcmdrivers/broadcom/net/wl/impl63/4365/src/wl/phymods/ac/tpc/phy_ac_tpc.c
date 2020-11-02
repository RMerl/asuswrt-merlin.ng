/*
 * ACPHY TxPowerCtrl module implementation
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
#if (ACCONF != 0) || (ACCONF2 != 0)
#include <typedefs.h>
#include <bcmdefs.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include <phy_wd.h>
#include "phy_type_tpc.h"
#include <phy_ac.h>
#include <phy_ac_tpc.h>
#include <phy_utils_reg.h>
#include <phy_ac_info.h>
#include <wlc_phyreg_ac.h>
#include <wlc_phytbl_ac.h>

/* module private states */
struct phy_ac_tpc_info {
	phy_info_t *pi;
	phy_ac_info_t *aci;
	phy_tpc_info_t *ti;

/* add other variable size variables here at the end */
};

static const char BCMATTACHDATA(rstr_offtgpwr)[] = "offtgpwr";

/* local functions */
static void wlc_phy_txpwrctrl_pwr_setup_acphy(phy_info_t *pi);
static void wlc_phy_txpwrctrl_pwr_setup_srom12_acphy(phy_info_t *pi);
int8 wlc_phy_fittype_srom12_acphy(phy_info_t *pi);
static void wlc_phy_get_srom12_pdoffset_acphy(phy_info_t *pi, int8 *pdoffs);
static void wlc_phy_get_paparams_percore_srom12_acphy(phy_info_t *pi,
		uint8 chan_freq_range, int16 *a, int16 *b, int16 *c, int16 *d, uint8 core);
static void wlc_phy_get_paparams_srom12_acphy(phy_info_t *pi,
		uint8 chan_freq_range, int16 *a, int16 *b, int16 *c, int16 *d);
static void wlc_phy_get_paparams_80p80_srom12_acphy(phy_info_t *pi,
		uint8 *chan_freqs, int16 *a, int16 *b, int16 *c, int16 *d);
static int32 wlc_phy_get_estpwrlut_srom12_acphy(int16 *a, int16 *b, int16 *c, int16 *d,
		uint8 pa_fittype, uint8 core, int32 idx);
static void wlc_phy_get_tssi_floor_acphy(phy_info_t *pi, int16 *floor);
static uint32 wlc_phy_pdoffset_cal_acphy(uint32 pdoffs, uint16 pdoffset, uint8 band, uint8 core);
static void wlc_phy_gaintbl_blanking(phy_info_t *pi, uint16 *tx_pwrctrl_tbl, uint8 txidxcap);
static bool wlc_phy_txpwrctrl_ison_acphy(phy_info_t *pi);
static uint8 wlc_phy_set_txpwr_clamp_acphy(phy_info_t *pi);
static void wlc_phy_txpower_recalc_target_acphy(phy_info_t *pi);
static void phy_ac_tpc_recalc_tgt(phy_type_tpc_ctx_t *ctx);
static int phy_ac_tpc_read_srom(phy_type_tpc_ctx_t *ctx, int bandtype);
static void wlc_phy_txpwrctrl_setminpwr(phy_info_t *pi);
#ifdef WL_SARLIMIT
static int16 wlc_phy_calc_adjusted_cap_rgstr_acphy(phy_info_t *pi, uint8 core);
#endif // endif

#if defined(PREASSOC_PWRCTRL) && !defined(WLC_LOW_ONLY)
static bool phy_ac_tpc_wd(phy_wd_ctx_t *ctx);
#endif // endif

#ifdef PREASSOC_PWRCTRL
static void wlc_phy_pwrctrl_shortwindow_upd_acphy(phy_info_t *pi, bool shortterm);
static uint8 wlc_phy_txpwrctrl_get_target_acphy(phy_info_t *pi, uint8 core);
#endif // endif
#ifdef POWPERCHANNL
static void wlc_phy_tx_target_pwr_per_channel_set_acphy(phy_info_t *pi);
void wlc_phy_tx_target_pwr_per_channel_decide_run_acphy(phy_info_t *pi);
void BCMATTACHFN(wlc_phy_tx_target_pwr_per_channel_limit_acphy)(phy_info_t *pi);
#endif /* POWPERCHANNL */

/* Register/unregister ACPHY specific implementation to common layer. */
phy_ac_tpc_info_t *
BCMATTACHFN(phy_ac_tpc_register_impl)(phy_info_t *pi, phy_ac_info_t *aci, phy_tpc_info_t *ti)
{
	phy_ac_tpc_info_t *info;
	phy_type_tpc_fns_t fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage in once */
	if ((info = phy_malloc(pi, sizeof(phy_ac_tpc_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	info->pi = pi;
	info->aci = aci;
	info->ti = ti;

#if defined(PREASSOC_PWRCTRL) && !defined(WLC_LOW_ONLY)
	/* register watchdog fn */
	if (phy_wd_add_fn(pi->wdi, phy_ac_tpc_wd, info,
	                  PHY_WD_PRD_1TICK, PHY_WD_1TICK_TPC, PHY_WD_FLAG_NONE) != BCME_OK) {
		PHY_ERROR(("%s: phy_wd_add_fn failed\n", __FUNCTION__));
		goto fail;
	}
#endif // endif

	/* Register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.recalc = phy_ac_tpc_recalc_tgt;
	fns.read_srom = phy_ac_tpc_read_srom;
	fns.ctx = info;

	phy_tpc_register_impl(ti, &fns);

	/* PHY-Feature specific parameter initialization */
	/* Read the offset target power var */

	return info;
fail:
	if (info != NULL)
		phy_mfree(pi, info, sizeof(phy_ac_tpc_info_t));
	return NULL;
}

void
BCMATTACHFN(phy_ac_tpc_unregister_impl)(phy_ac_tpc_info_t *info)
{
	phy_info_t *pi;
	phy_tpc_info_t *ti;

	ASSERT(info);
	pi = info->pi;
	ti = info->ti;

	PHY_TRACE(("%s\n", __FUNCTION__));

	phy_tpc_unregister_impl(ti);

	phy_mfree(pi, info, sizeof(phy_ac_tpc_info_t));
}

/* recalc target txpwr and apply to h/w */
static void
phy_ac_tpc_recalc_tgt(phy_type_tpc_ctx_t *ctx)
{
	phy_ac_tpc_info_t *info = (phy_ac_tpc_info_t *)ctx;
	phy_info_t *pi = info->pi;

	PHY_TRACE(("%s\n", __FUNCTION__));

	wlc_phy_txpower_recalc_target_acphy(pi);
}

/* read srom txpwr limits */
static int
BCMATTACHFN(phy_ac_tpc_read_srom)(phy_type_tpc_ctx_t *ctx, int bandtype)
{
	phy_ac_tpc_info_t *info = (phy_ac_tpc_info_t *)ctx;
	phy_info_t *pi = info->pi;

	PHY_TRACE(("%s: band %d\n", __FUNCTION__, bandtype));
	if (SROMREV(pi->sh->sromrev) >= 12) {
	return wlc_phy_txpwr_srom12_read(pi) ? BCME_OK : BCME_ERROR;
	}
	return wlc_phy_txpwr_srom11_read(pi) ? BCME_OK : BCME_ERROR;
}

#if defined(PREASSOC_PWRCTRL)
#if !defined(WLC_LOW_ONLY)
static bool
phy_ac_tpc_wd(phy_wd_ctx_t *ctx)
{
	phy_ac_tpc_info_t *info = (phy_ac_tpc_info_t *)ctx;
	phy_info_t *pi = info->pi;

	phy_ac_tpc_shortwindow_upd(pi, FALSE);
	return TRUE;
}
#endif // endif
#endif /* PREASSOC_PWRCTRL */

#ifdef WL_SARLIMIT
static int16
wlc_phy_calc_adjusted_cap_rgstr_acphy(phy_info_t *pi, uint8 core)
{
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;
	return MIN(pi->sarlimit[core], pi->tx_power_max_per_core[core] + pi_ac->txpwr_offset[core]);
}
#endif /* WL_SARLIMIT */

/* ********************************************* */
/*				Internal Definitions					*/
/* ********************************************* */
#define PWRCTRL_SHORTW_AVG 1
#define PWRCTRL_LONGW_AVG 4
#define PWRCTRL_MIN_INIT_IDX 5
#define PWRCTRL_MAX_INIT_IDX 127
#define SAMPLE_TSSI_AFTER_100_SAMPLES 100
#define SAMPLE_TSSI_AFTER_110_SAMPLES 110
#define SAMPLE_TSSI_AFTER_115_SAMPLES 115
#define SAMPLE_TSSI_AFTER_150_SAMPLES 150
#define SAMPLE_TSSI_AFTER_170_SAMPLES 170
#define SAMPLE_TSSI_AFTER_185_SAMPLES 185
#define SAMPLE_TSSI_AFTER_200_SAMPLES 200
#define SAMPLE_TSSI_AFTER_220_SAMPLES 220

#define ACPHY_TBL_ID_ESTPWRLUTS(core)	\
	(((core == 0) ? ACPHY_TBL_ID_ESTPWRLUTS0 : \
	((core == 1) ? ACPHY_TBL_ID_ESTPWRLUTS1 : \
	((core == 2) ? ACPHY_TBL_ID_ESTPWRLUTS2 : ACPHY_TBL_ID_ESTPWRLUTS3))))

static void
wlc_phy_txpwrctrl_pwr_setup_acphy(phy_info_t *pi)
{
	uint8 stall_val;
	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;
	srom11_pwrdet_t *pwrdet = pi->pwrdet_ac;
	int8   target_min_limit;
	int16  a1[PAPARAM_SET_NUM], b0[PAPARAM_SET_NUM], b1[PAPARAM_SET_NUM];
	uint8  chan_freq_range, iidx;
	uint8  core, core2range, idx_set[2], k;
	int32  num, den, pwr_est, pwr_est2range;
	uint32 tbl_len, tbl_offset, idx, shfttbl_len;
	uint16 regval[128];
	uint32 shfttblval[24];
	uint8  tssi_delay, tssi_delay_cck = 0;
	uint32 pdoffs = 0;
	int16  tssifloor = 1023;
	uint8  maxpwr = 0;
	uint8  mult_mode = 1;
	bool flag2rangeon;
	int8 targetidx, tx_idx;
#ifdef PREASSOC_PWRCTRL
	bool init_idx_carry_from_lastchan;
	uint8 step_size, prev_target_qdbm;
#endif // endif
	struct _tp_qtrdbm {
		uint8 core;
		int8 target_pwr_qtrdbm;
	} tp_qtrdbm_each_core[PHY_CORE_MAX]; /* TP for each core */
	uint core_count = 0;

#if defined(PHYCAL_CACHING)
	ch_calcache_t *ctx = NULL;
	ctx = wlc_phy_get_chanctx(pi, pi->radio_chanspec);
	BCM_REFERENCE(ctx);
#endif /* PHYCAL_CACHING */
	iidx = 0;
	tbl_len = 128;
	tbl_offset = 0;
	shfttbl_len = 24;
	flag2rangeon =
		((CHSPEC_IS2G(pi->radio_chanspec) && pi->u.pi_acphy->srom_tworangetssi2g) ||
		(CHSPEC_IS5G(pi->radio_chanspec) && pi->u.pi_acphy->srom_tworangetssi5g)) &&
		PHY_IPA(pi);

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	PHY_CHANLOG(pi, __FUNCTION__, TS_ENTER, 0);

	stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
	ACPHY_DISABLE_STALL(pi);
	/* enable TSSI */
	MOD_PHYREG(pi, TSSIMode, tssiEn, 1);
	MOD_PHYREG(pi, TxPwrCtrlCmd, txPwrCtrl_en, 0);

	/* Initialize ALL PA Param arrays a1, b0, b1 to be all zero */
	for (idx = 0; idx < PAPARAM_SET_NUM; idx++) {
		a1[idx] = 0;
		b0[idx] = 0;
		b1[idx] = 0;
	}

	if (ACMAJORREV_3(pi->pubpi.phy_rev)) {
			MOD_PHYREG(pi, TxPwrCtrlCmd, bbMultInt_en, 1);
#ifdef PREASSOC_PWRCTRL
		step_size = 2;
#endif // endif
	} else if (RADIOREV(pi->pubpi.radiorev) == 4 || RADIOREV(pi->pubpi.radiorev) == 8 ||
		ACMAJORREV_2(pi->pubpi.phy_rev) || ACMAJORREV_5(pi->pubpi.phy_rev)	||
		ACMAJORREV_4(pi->pubpi.phy_rev)) {
		/* 4360B0/B1/4350 using 0.5dB-step gaintbl, bbmult interpolation enabled */
		MOD_PHYREG(pi, TxPwrCtrlCmd, bbMultInt_en, 1);
#ifdef PREASSOC_PWRCTRL
		step_size = 2;
#endif // endif
	} else {
	/* disable bbmult interpolation
	   to work with a 0.25dB step txGainTbl
	*/
		MOD_PHYREG(pi, TxPwrCtrlCmd, bbMultInt_en, 0);
#ifdef PREASSOC_PWRCTRL
		step_size = 1;
#endif // endif
	}

	/* Get pwrdet params from SROM for current subband */
	chan_freq_range = wlc_phy_get_chan_freq_range_acphy(pi, 0);
	/* Check if 2 range should be activated for band */

	FOREACH_CORE(pi, core) {
		/* First load PA Param sets for corresponding band/frequemcy range */
		/* for all cores 0 to PHYNUMCORE()-1 */
		switch (chan_freq_range) {
		case WL_CHAN_FREQ_RANGE_2G:
		case WL_CHAN_FREQ_RANGE_5G_BAND0:
		case WL_CHAN_FREQ_RANGE_5G_BAND1:
		case WL_CHAN_FREQ_RANGE_5G_BAND2:
		case WL_CHAN_FREQ_RANGE_5G_BAND3:
			a1[core] =  pwrdet->pwrdet_a1[core][chan_freq_range];
			b0[core] =  pwrdet->pwrdet_b0[core][chan_freq_range];
			b1[core] =  pwrdet->pwrdet_b1[core][chan_freq_range];
			PHY_TXPWR(("wl%d: %s: pwrdet core%d: a1=%d b0=%d b1=%d\n",
				pi->sh->unit, __FUNCTION__, core,
				a1[core], b0[core], b1[core]));
			break;
		}

		/* Set cck pwr offset from nvram */
		if (CHSPEC_IS2G(pi->radio_chanspec) && TINY_RADIO(pi)) {
			MOD_PHYREGCEE(pi, TxPwrCtrlTargetPwr_path, core,
				cckPwrOffset, pi->cckpwroffset[core] - pi->sh->cckPwrIdxCorr);
		}
		/* Next if special consitions are met, load additional PA Param sets */
		/* for corresponding band/frequemcy range */
		if (flag2rangeon || (BF3_TSSI_DIV_WAR(pi_ac) &&
		    (ACMAJORREV_2(pi->pubpi.phy_rev) || ACMAJORREV_4(pi->pubpi.phy_rev)))) {
			/* If two-range TSSI scheme is enabled via flag2rangeon, */
			/* or if TSSI divergence WAR is enable for 4350, load PHYCORENUM() */
			/* additional PA Param sets for corresponding band/frequemcy range. */
			/* For 4350, extra PA Params are used for CCK in 2G band or */
			/* for 40/80 MHz bands in 5G band */
			if ((phy_get_phymode(pi) == PHYMODE_RSDB) &&
				ACMAJORREV_4(pi->pubpi.phy_rev)) {
				core2range = TSSI_DIVWAR_INDX;
			} else {
				core2range = PHYCORENUM(pi->pubpi.phy_corenum) + core;
			}
			ASSERT(core2range < PAPARAM_SET_NUM);
			a1[core2range] = pwrdet->pwrdet_a1[core2range][chan_freq_range];
			b0[core2range] = pwrdet->pwrdet_b0[core2range][chan_freq_range];
			b1[core2range] = pwrdet->pwrdet_b1[core2range][chan_freq_range];

			PHY_TXPWR(("wl%d: %s: pwrdet %s core%d: a1=%d b0=%d b1=%d\n",
			           pi->sh->unit, __FUNCTION__,
			           (flag2rangeon) ? "2nd-TSSI" : "CCK/40/80MHz",
			           core, a1[core2range], b0[core2range], b1[core2range]));
		} else if (BF3_TSSI_DIV_WAR(pi_ac) && (ACMAJORREV_1(pi->pubpi.phy_rev) ||
			(ACMAJORREV_3(pi->pubpi.phy_rev)))) {
			/* If TSSI divergence WAR is enable for 4335, */
			/* use core1 and core2 paparams for 40Mhz and 40/80 paparams. */
			a1[1] =	pwrdet->pwrdet_a1[1][chan_freq_range];
			b0[1] =	pwrdet->pwrdet_b0[1][chan_freq_range];
			b1[1] =	pwrdet->pwrdet_b1[1][chan_freq_range];
			PHY_TXPWR(("wl%d: %s: pwrdet 40mhz case: %d: a1=%d b0=%d b1=%d\n",
				pi->sh->unit, __FUNCTION__, 1,
				a1[1], b0[1], b1[1]));

			if (CHSPEC_IS5G(pi->radio_chanspec)) {
				a1[2] =	pwrdet->pwrdet_a1[2][chan_freq_range];
				b0[2] =	pwrdet->pwrdet_b0[2][chan_freq_range];
				b1[2] =	pwrdet->pwrdet_b1[2][chan_freq_range];
				PHY_TXPWR(("wl%d: %s: pwrdet 80mhz case: %d: a1=%d b0=%d b1=%d\n",
					pi->sh->unit, __FUNCTION__, 2,
					a1[2], b0[2], b1[2]));
			}
		}
	}

	/* target power */
	wlc_phy_txpwrctrl_update_minpwr_acphy(pi);
	target_min_limit = pi->min_txpower * WLC_TXPWR_DB_FACTOR;

	FOREACH_ACTV_CORE(pi, pi->sh->phyrxchain, core) {
		int8 target_pwr_qtrdbm;
		target_pwr_qtrdbm = (int8)pi->tx_power_max_per_core[core];
		/* never target below the min threashold */
		if (target_pwr_qtrdbm < target_min_limit)
			target_pwr_qtrdbm = target_min_limit;

		if (ACMAJORREV_1(pi->pubpi.phy_rev)) {
			chan_freq_range = wlc_phy_get_chan_freq_range_acphy(pi, 0);
			tssifloor = (int16)pwrdet->tssifloor[core][chan_freq_range];
			if ((tssifloor != 0x3ff) && (tssifloor != 0)) {
				maxpwr = wlc_phy_set_txpwr_clamp_acphy(pi);
				if (maxpwr < target_pwr_qtrdbm) {
					target_pwr_qtrdbm = maxpwr;
				}
			}
		}

		tp_qtrdbm_each_core[core_count].core = core;
		tp_qtrdbm_each_core[core_count].target_pwr_qtrdbm = target_pwr_qtrdbm;
		++core_count;
	        /* PHY_ERROR(("####targetPwr: %d#######\n",
	         * tp_qtrdbm_each_core[core_count].target_pwr_qtrdbm));
		 */
	}

	/* determine pos/neg TSSI slope */
	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		MOD_PHYREG(pi, TSSIMode, tssiPosSlope, pi->fem2g->tssipos);
	} else {
		MOD_PHYREG(pi, TSSIMode, tssiPosSlope, pi->fem5g->tssipos);
	}
	MOD_PHYREG(pi, TSSIMode, tssiPosSlope, 1);

	/* disable txpwrctrl during idleTssi measurement, etc */
	MOD_PHYREG(pi, TxPwrCtrlCmd, txPwrCtrl_en, 0);
	if (flag2rangeon) {
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			MOD_PHYREG(pi, TxPwrCtrlPwrRange2, maxPwrRange2, 80);
			MOD_PHYREG(pi, TxPwrCtrlPwrRange2, minPwrRange2, 44);
		} else {
			MOD_PHYREG(pi, TxPwrCtrlPwrRange2, maxPwrRange2, 80);
			MOD_PHYREG(pi, TxPwrCtrlPwrRange2, minPwrRange2, 44);
		}
	} else {
		MOD_PHYREG(pi, TxPwrCtrlPwrRange2, maxPwrRange2, 0);
		MOD_PHYREG(pi, TxPwrCtrlPwrRange2, minPwrRange2, 1);
	}

#ifdef PREASSOC_PWRCTRL
	core_count = 0;

	FOREACH_ACTV_CORE(pi, pi->sh->phyrxchain, core) {
		init_idx_carry_from_lastchan = (CHSPEC_IS5G(pi->radio_chanspec)) ?
		        pi->u.pi_acphy->pwr_ctrl_save->status_idx_carry_5g[core]
		        : pi->u.pi_acphy->pwr_ctrl_save->status_idx_carry_2g[core];
		if (!init_idx_carry_from_lastchan) {
			/* 4360B0 using 0.5dB-step gaintbl so start with a lower starting idx */
			if ((RADIOREV(pi->pubpi.radiorev) == 4) ||
			    (RADIOREV(pi->pubpi.radiorev) == 8) ||
			    (ACMAJORREV_5(pi->pubpi.phy_rev))) {
				iidx = 20;
			} else {
				iidx = 50;
			}
			MOD_PHYREGCEE(pi, TxPwrCtrlInit_path, core, pwrIndex_init_path, iidx);
			if (CHSPEC_IS5G(pi->radio_chanspec)) {
				pi->u.pi_acphy->pwr_ctrl_save->status_idx_carry_5g[core] = TRUE;
			} else {
				pi->u.pi_acphy->pwr_ctrl_save->status_idx_carry_2g[core] = TRUE;
			}
			/* XXX This is the handshake for te code that
			   is put in for open loop power control
			   128 means do NOT restore the value in the
			   open loop function
			*/
			pi_ac->txpwrindex_hw_save[core] = 128;
		} else {
		/* set power index initial condition */
			int32 new_iidx;

			if (CHSPEC_IS5G(pi->radio_chanspec)) {
				iidx = pi->u.pi_acphy->pwr_ctrl_save->status_idx_5g[core];
				prev_target_qdbm = pi->u.pi_acphy->pwr_ctrl_save->pwr_qdbm_5g[core];
				if ((pi->u.pi_acphy->pwr_ctrl_save->stored_not_restored_5g[core])) {
					/* XXX This is the handshake for
					   the code that is put in for open loop power control
					   128 means do NOT restore the value in the
					   open loop function
					*/
					pi->u.pi_acphy->pwr_ctrl_save->stored_not_restored_5g[core]
					        = FALSE;
					pi_ac->txpwrindex_hw_save[core] = 128;

				}
			} else {
				iidx = pi->u.pi_acphy->pwr_ctrl_save->status_idx_2g[core];
				prev_target_qdbm = pi->u.pi_acphy->pwr_ctrl_save->pwr_qdbm_2g[core];
				if ((pi->u.pi_acphy->pwr_ctrl_save->stored_not_restored_2g[core])) {
					/* XXX This is the handshake for
					   the code that is put in for open loop power control
					   128 means do NOT restore the value in the
					   open loop function
					*/
					pi_ac->pwr_ctrl_save->stored_not_restored_2g[core] = FALSE;
					pi_ac->txpwrindex_hw_save[core] = 128;

				}
			}
			new_iidx = (int32)iidx
				+ ((int32)tp_qtrdbm_each_core[core_count].target_pwr_qtrdbm
				- prev_target_qdbm) / step_size;

			/* XXX Sanity Check
			   XXX make sure iidx is within bounds to be safe
			*/
			if (new_iidx < PWRCTRL_MIN_INIT_IDX) {
				iidx = PWRCTRL_MIN_INIT_IDX;
			} else if (new_iidx > PWRCTRL_MAX_INIT_IDX) {
				iidx = PWRCTRL_MAX_INIT_IDX;
			} else {
				iidx = (uint8)new_iidx;
			}

			MOD_PHYREGCEE(pi, TxPwrCtrlInit_path, core, pwrIndex_init_path, iidx);
		}
		core_count++;
	}
#else
	if ((RADIOREV(pi->pubpi.radiorev) == 4) ||
	    (RADIOREV(pi->pubpi.radiorev) == 8) ||
	    (ACMAJORREV_5(pi->pubpi.phy_rev))) {
		iidx = 20;
	} else {
		iidx = 50;
	}
#if defined(PHYCAL_CACHING)
	if (!ctx || !ctx->valid)
#endif /* PHYCAL_CACHING */
		FOREACH_ACTV_CORE(pi, pi->sh->phyrxchain, core) {
			MOD_PHYREGCEE(pi, TxPwrCtrlInit_path, core,
			pwrIndex_init_path, iidx);
		}
#endif /* PREASSOC_PWRCTRL */
	/* MOD_PHYREG(pi, TxPwrCtrlIdleTssi, rawTssiOffsetBinFormat, 1); */

	/* sample TSSI at 7.5us */
	if (ACMAJORREV_2(pi->pubpi.phy_rev) || ACMAJORREV_5(pi->pubpi.phy_rev)) {
		if (PHY_IPA(pi)) {
			tssi_delay = SAMPLE_TSSI_AFTER_170_SAMPLES;
		} else {
			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				if (pi->u.pi_acphy->srom_2g_pdrange_id == 21) {
					tssi_delay = SAMPLE_TSSI_AFTER_200_SAMPLES;
				} else {
					tssi_delay = SAMPLE_TSSI_AFTER_150_SAMPLES;
				}
			} else {
				if (pi->u.pi_acphy->srom_5g_pdrange_id == 23) {
					tssi_delay = SAMPLE_TSSI_AFTER_200_SAMPLES;
				} else {
					tssi_delay = SAMPLE_TSSI_AFTER_150_SAMPLES;
				}
			}
		}
	} else if ((ACMAJORREV_1(pi->pubpi.phy_rev) && ACMINORREV_2(pi)) || TINY_RADIO(pi)) {
		tssi_delay = SAMPLE_TSSI_AFTER_150_SAMPLES;
		if (TINY_RADIO(pi)) {
			if (ACMAJORREV_4(pi->pubpi.phy_rev)) {
				tssi_delay = SAMPLE_TSSI_AFTER_185_SAMPLES;
				MOD_PHYREG(pi, perPktIdleTssiCtrl, perPktIdleTssiUpdate_en, 0);
				MOD_PHYREG(pi, TSSIMode, TwoPwrRange, 0);
				MOD_PHYREG(pi, TxPwrCtrlPwrRange2, maxPwrRange2, 127);
				MOD_PHYREG(pi, TxPwrCtrlPwrRange2, minPwrRange2, -128);
				if (CHSPEC_IS5G(pi->radio_chanspec) &&
					(BF3_TSSI_DIV_WAR(pi_ac))) {
					mult_mode = 4;
				} else {
					mult_mode = 1;
				}
				FOREACH_ACTV_CORE(pi, pi->sh->phyrxchain, core) {
					if (core == 0) {
						MOD_PHYREG(pi, TxPwrCtrl_Multi_Mode0,
						multi_mode, mult_mode);
					} else if (core == 1) {
						MOD_PHYREG(pi, TxPwrCtrl_Multi_Mode1,
						multi_mode, mult_mode);
					}
				}
			} else {
				tssi_delay = SAMPLE_TSSI_AFTER_100_SAMPLES;
			}
			if (PHY_IPA(pi)) {
				tssi_delay_cck =  SAMPLE_TSSI_AFTER_110_SAMPLES;
			} else {
				tssi_delay_cck =  SAMPLE_TSSI_AFTER_115_SAMPLES;
			}
		}
		if (PHY_IPA(pi) && !(TINY_RADIO(pi))) {		/* this is for 4335C0 iPA */
			tssi_delay = SAMPLE_TSSI_AFTER_170_SAMPLES;
		} else {
			/* Enable tssi accum for C0. also change the tssi digi filter position. */
			/* this helps to reduce the tssi noise. */
			MOD_PHYREG(pi, TssiAccumCtrl, Ntssi_accum_delay, tssi_delay);
			MOD_PHYREG(pi, TssiAccumCtrl, Ntssi_intg_log2, 4);
			MOD_PHYREG(pi, TssiAccumCtrl, tssi_accum_en, 1);
			MOD_PHYREG(pi, TssiAccumCtrl, tssi_filter_pos, 1);
			if (TINY_RADIO(pi)) {
				MOD_PHYREG(pi, TssiAccumCtrlcck, Ntssi_accum_delay_cck,
					tssi_delay_cck);
				MOD_PHYREG(pi, TssiAccumCtrlcck, Ntssi_intg_log2_cck, 1);
			}
		}
	} else {
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			if (pi->u.pi_acphy->srom_2g_pdrange_id >= 5) {
				tssi_delay = SAMPLE_TSSI_AFTER_200_SAMPLES;
			} else if (pi->u.pi_acphy->srom_2g_pdrange_id >= 4) {
				tssi_delay = SAMPLE_TSSI_AFTER_220_SAMPLES;
			} else {
				tssi_delay = SAMPLE_TSSI_AFTER_150_SAMPLES;
			}
		} else {
			if (pi->u.pi_acphy->srom_5g_pdrange_id >= 5) {
				tssi_delay = SAMPLE_TSSI_AFTER_200_SAMPLES;
			} else if (pi->u.pi_acphy->srom_5g_pdrange_id >= 4) {
				tssi_delay = SAMPLE_TSSI_AFTER_220_SAMPLES;
			} else {
				tssi_delay = SAMPLE_TSSI_AFTER_150_SAMPLES;
			}
		}
	}
	MOD_PHYREG(pi, TxPwrCtrlNnum, Ntssi_delay, tssi_delay);
	if (pi->bphy_scale != 0) {
		MOD_PHYREG(pi, BphyControl3, bphyScale20MHz, pi->bphy_scale);
	}

#if defined(PREASSOC_PWRCTRL) && !defined(WLC_LOW_ONLY)
	/* average over 2 or 16 packets */
	wlc_phy_pwrctrl_shortwindow_upd_acphy(pi, pi->channel_short_window);
#else
	MOD_PHYREG(pi, TxPwrCtrlNnum, Npt_intg_log2, PWRCTRL_LONGW_AVG);
#endif /* PREASSOC_PWRCTRL */

	/* decouple IQ comp and LOFT comp from Power Control */
	MOD_PHYREG(pi, TxPwrCtrlCmd, use_txPwrCtrlCoefsIQ, 0);
	if (ACREV_IS(pi->pubpi.phy_rev, 1) || ACMAJORREV_5(pi->pubpi.phy_rev)) {
		MOD_PHYREG(pi, TxPwrCtrlCmd, use_txPwrCtrlCoefsLO, 1);
	} else {
		MOD_PHYREG(pi, TxPwrCtrlCmd, use_txPwrCtrlCoefsLO, 0);
	}

	/* adding maxCap for each Tx chain */
	if (0) {
		FOREACH_ACTV_CORE(pi, pi->sh->phyrxchain, core) {
			if (core == 0) {
				MOD_PHYREG(pi, TxPwrCapping_path0, maxTxPwrCap_path0, 80);
			} else if (core == 1) {
				MOD_PHYREG(pi, TxPwrCapping_path1, maxTxPwrCap_path1, 32);
			} else if (core == 2) {
				MOD_PHYREG(pi, TxPwrCapping_path2, maxTxPwrCap_path2, 32);
			}
		}
	}
#ifdef WL_SARLIMIT
	wlc_phy_set_sarlimit_acphy(pi);
#endif // endif
	while (core_count > 0) {
		--core_count;
		if (pi_ac->offset_targetpwr) {
			uint8 tgt_pwr_qdbm = tp_qtrdbm_each_core[core_count].target_pwr_qtrdbm;
			tgt_pwr_qdbm -= (pi_ac->offset_targetpwr << 2);
			wlc_phy_txpwrctrl_set_target_acphy(pi,
				tgt_pwr_qdbm,
				0);
		} else {
			/* set target powers */
			wlc_phy_txpwrctrl_set_target_acphy(pi,
				tp_qtrdbm_each_core[core_count].target_pwr_qtrdbm,
				tp_qtrdbm_each_core[core_count].core);
		}
		PHY_TXPWR(("wl%d: %s: txpwrctl[%d]: %d\n",
			pi->sh->unit, __FUNCTION__, tp_qtrdbm_each_core[core_count].core,
		              tp_qtrdbm_each_core[core_count].target_pwr_qtrdbm));
	}
#ifdef ENABLE_FCBS
	if (IS_FCBS(pi) && pi->phy_fcbs->FCBS_INPROG)
		pi->phy_fcbs->FCBS_INPROG = 0;
	else {
#endif // endif
	/* load estimated power tables (maps TSSI to power in dBm)
	 *    entries in tx power table 0000xxxxxx
	 */
	tbl_len = 128;
	tbl_offset = 0;

	if (BF3_TSSI_DIV_WAR(pi_ac) && (ACMAJORREV_1(pi->pubpi.phy_rev) ||
		(TINY_RADIO(pi) && !ACMAJORREV_4(pi->pubpi.phy_rev)))) {

		if ((CHSPEC_IS80(pi->radio_chanspec) || PHY_AS_80P80(pi, pi->radio_chanspec)) &&
				CHSPEC_IS5G(pi->radio_chanspec)) {
			/* core 2 conatins 40-80mhz paparam
			 * core 0 conatins 20mhz paparam
			 */
			idx_set[0] = 2; idx_set[1] = 0;
		} else if (CHSPEC_IS160(pi->radio_chanspec)) {
			idx_set[0] = 2; idx_set[1] = 0; // FIXME
			ASSERT(0);
		} else if (CHSPEC_IS40(pi->radio_chanspec)) {
			/* core 1 conatins 40mhz paparam
			 * core 0 contains 20mhz paparam
			 */
			idx_set[0] = 1; idx_set[1] = 0;
			if (PHY_IPA(pi) && CHSPEC_IS2G(pi->radio_chanspec)) {
			    idx_set[0] = 0; idx_set[1] = 1;
			}
		} else {
			/* core 0 conatins 20mhz OFDM paparam
			 * core 1 contains 20mhz CCK paparam
			 */
			idx_set[0] = 0;
			if (!TINY_RADIO(pi))
				idx_set[1] = 1;
			else
				idx_set[1] = 0;
		}

		for (idx = 0; idx < tbl_len; idx++) {
			for (k = 0, regval[idx] = 0; k < 2; k++) {
				core = idx_set[k];
				num = 8 * (16 * b0[core] + b1[core] * idx);
				den = 32768 + a1[core] * idx;
				pwr_est = MAX(((4 * num + den/2)/den), -8);
				pwr_est = MIN(pwr_est, 0x7F);
				regval[idx] |= (uint16)(pwr_est&0xff) << (8*k);
			}
		}
		/* Est Pwr Table is 128x16 Table for 4335 */
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_ESTPWRLUTS0, tbl_len,
		                          tbl_offset, 16, regval);
	} else if (BF3_TSSI_DIV_WAR(pi_ac) &&
	           (ACMAJORREV_2(pi->pubpi.phy_rev) || ACMAJORREV_4(pi->pubpi.phy_rev))) {
		if ((CHSPEC_IS80(pi->radio_chanspec) ||
				CHSPEC_IS40(pi->radio_chanspec)) &&
				CHSPEC_IS5G(pi->radio_chanspec) && !PHY_IPA(pi) &&
				(ACMAJORREV_2(pi->pubpi.phy_rev))) {
			/* core 2/3 contains 40-80mhz paparam
			 * core 0/1 contains 20mhz paparam
			 */
			idx_set[0] = 2; idx_set[1] = 0;
		} else if ((CHSPEC_IS80(pi->radio_chanspec) ||
				CHSPEC_IS40(pi->radio_chanspec)) &&
				CHSPEC_IS5G(pi->radio_chanspec) && PHY_IPA(pi) &&
				(ACMAJORREV_4(pi->pubpi.phy_rev))) {
			/* core 2/3 contains 40-80mhz paparam
			 * core 0/1 contains 20mhz paparam
			 */
			idx_set[0] = 0; idx_set[1] = 2;
		} else if (CHSPEC_IS20(pi->radio_chanspec) &&
		    CHSPEC_IS2G(pi->radio_chanspec) && PHY_IPA(pi)) {
			/* core 0,1 conatins 20mhz OFDM paparam
			 * core 2,3 contains 20mhz CCK paparam
			 */
			idx_set[0] = 0;  idx_set[1] = 2;
		} else {
			idx_set[0] = 0; idx_set[1] = 0;
		}

		FOREACH_ACTV_CORE(pi, pi->sh->phyrxchain, core) {
			uint8 core_off;

			for (idx = 0; idx < tbl_len; idx++) {
				for (k = 0, regval[idx] = 0; k < 2; k++) {
					core_off = core + ((core < 2)  ?
					                   idx_set[k] : 0);
					num = 8 * (16 * b0[core_off] + b1[core_off] * idx);
					den = 32768 + a1[core_off] * idx;
					pwr_est = MAX(((4 * num + den/2)/den), -8);
					pwr_est = MIN(pwr_est, 0x7F);
					regval[idx] |= (uint16)(pwr_est&0xff) << (8*k);
				}
			}
			/* Est Pwr Table is 128x8 Table. Limit Write to 8 bits */
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_ESTPWRLUTS(core), tbl_len,
			                          tbl_offset, 16, regval);
		}
	} else {
		FOREACH_ACTV_CORE(pi, pi->sh->phyrxchain, core) {
			for (idx = 0; idx < tbl_len; idx++) {
				num = 8 * (16 * b0[core] + b1[core] * idx);
				den = 32768 + a1[core] * idx;
				if (pi->iboard) {
					pwr_est = MAX(((4 * num + den/2)/den), -60);
					pwr_est = MIN(pwr_est, 0x28);
				} else {
					pwr_est = MAX(((4 * num + den/2)/den), -8);
					pwr_est = MIN(pwr_est, 0x7f);
				}
				if (flag2rangeon) {
					/* iPa - ToDo 2 range TSSI */
					core2range = PHYCORENUM(pi->pubpi.phy_corenum) + core;
					ASSERT(core2range < PAPARAM_SET_NUM);
					num = 8 * (16 * b0[core2range] + b1[core2range] * idx);
					den = 32768 + a1[core2range] * idx;
					pwr_est2range = MAX(((4 * num + den/2)/den), -8);
					pwr_est2range = MIN(pwr_est2range, 0x7F);
					regval[idx] =
						(uint16)((pwr_est2range&0xff) +
						((pwr_est&0xff)<<8));
				} else {
					regval[idx] = (uint16)(pwr_est&0xff);
				}
			}
			/* Est Pwr Table is 128x8 Table. Limit Write to 8 bits */
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_ESTPWRLUTS(core), tbl_len,
			                          tbl_offset, 16, regval);
		}
	}

	if (pi->iboard) {
		tx_idx = 0;
		wlc_phy_tiny_rfseq_mode_set(pi, 1);
		targetidx = wlc_phy_tone_pwrctrl(pi, tx_idx, 0);
		wlc_phy_tiny_rfseq_mode_set(pi, 0);
		wlc_phy_txpwr_by_index_acphy(pi, 1, targetidx);
		if (targetidx < 0) {
			wlc_phy_txpwr_by_index_acphy(pi, 1, 0);
		}
	}

	/* start to populate estPwrShftTbl */
	for (idx = 0; idx < 24; idx++) {
		if (CHSPEC_IS5G(pi->radio_chanspec)) {
			if ((idx == 0)||((idx > 1)&&(idx < 5))||((idx > 6)&&(idx < 10))) {
				shfttblval[idx] = 0;
			} else if ((idx == 1)||((idx > 4)&&(idx < 7))) {
				pdoffs = 0;
				FOREACH_ACTV_CORE(pi, pi->sh->hw_phyrxchain, core) {
					pdoffs = wlc_phy_pdoffset_cal_acphy(pdoffs,
						pwrdet->pdoffset40[core], chan_freq_range, core);
				}
				shfttblval[idx] = pdoffs & 0xffffff;
				if ((idx == 5) && (BF3_TSSI_DIV_WAR(pi_ac) &&
				(ACMAJORREV_1(pi->pubpi.phy_rev) ||
				ACMAJORREV_3(pi->pubpi.phy_rev)))) {
					shfttblval[idx] = 0;
				}
			} else if (idx == 10) {
				pdoffs = 0;
				FOREACH_ACTV_CORE(pi, pi->sh->hw_phyrxchain, core) {
					pdoffs = wlc_phy_pdoffset_cal_acphy(pdoffs,
						pwrdet->pdoffset80[core], chan_freq_range, core);
				}
				shfttblval[idx] = pdoffs & 0xffffff;
			} else {
				shfttblval[idx] = 0;
			}
		} else {
			/* hardcoding for 4335 wlbga for now, will add nvram var later if needed */
			if (CHIPID(pi->sh->chip) == BCM4345_CHIP_ID ||
				(CHIPID(pi->sh->chip) == BCM4335_CHIP_ID &&
				CHSPEC_IS2G(pi->radio_chanspec) &&
				CHSPEC_IS20(pi->radio_chanspec))) {
				if (BF3_TSSI_DIV_WAR(pi_ac)) {
					/* Note: SROM entry rpcal2g and rpcal5gb0 is redefined for
					 * 4335 to represent the 2G channel-dependent TSSI offset
					 */
					if (idx == 3 || idx == 17) {
						switch (CHSPEC_CHANNEL(pi->radio_chanspec)) {
						case 1:
							pdoffs =
							((pi->sromi->rpcal2g >> 0)  & 0xF);
							break;
						case 2:
							pdoffs =
							((pi->sromi->rpcal2g >> 4)  & 0xF);
							break;
						case 3:
							pdoffs =
							((pi->sromi->rpcal2g >> 8)  & 0xF);
							break;
						case 12:
							pdoffs =
							((pi->sromi->rpcal2g >> 12) & 0xF);
							break;
						case 13:
							pdoffs =
							((pi->sromi->rpcal5gb0 >> 0) & 0xF);
							break;
						case 14:
							pdoffs =
							((pi->sromi->rpcal5gb0 >> 4) & 0xF);
							break;
						case 4:
							pdoffs =
							((pi->sromi->rpcal5gb0 >> 8) & 0xF);
							break;
						case 5:
							pdoffs =
							((pi->sromi->rpcal5gb0 >> 12) & 0xF);
							break;
						case 6:
							pdoffs =
							((pi->sromi->rpcal5gb1 >> 0) & 0xF);
							break;
						case 7:
							pdoffs =
							((pi->sromi->rpcal5gb1 >> 4) & 0xF);
							break;
						case 8:
							pdoffs =
							((pi->sromi->rpcal5gb1 >> 8) & 0xF);
							break;
						case 9:
							pdoffs =
							((pi->sromi->rpcal5gb1 >> 12) & 0xF);
							break;
						case 10:
							pdoffs =
							((pi->sromi->rpcal5gb2 >> 0) & 0xF);
							break;
						case 11:
							pdoffs =
							((pi->sromi->rpcal5gb2 >> 4) & 0xF);
							break;

						default:
							pdoffs = 0;
							break;
						}
						pdoffs = (pdoffs > 7) ? (0xf0 | pdoffs) : pdoffs;
						shfttblval[idx] = pdoffs & 0xff;
					} else {
						shfttblval[idx] = 0;
					}
				} else { /* when tssi_div WAR is off, only cck offset is used */
					if (idx == 17) {
						pdoffs = pwrdet->pdoffsetcck[0];
						pdoffs = (pdoffs > 7) ? (0xf0 | pdoffs) : pdoffs;
						shfttblval[idx] = pdoffs & 0xff;
					} else {
						shfttblval[idx] = 0;
					}
				}
			} else {
				if (idx == 17) {
					pdoffs = 0;
					FOREACH_ACTV_CORE(pi, pi->sh->hw_phyrxchain, core) {
						pdoffs = wlc_phy_pdoffset_cal_acphy(pdoffs,
							pwrdet->pdoffsetcck[core],
							chan_freq_range, core);
					}
					shfttblval[idx] = pdoffs & 0xffffff;
				} else {
					if (pwrdet->pdoffset2g40_flag == 1) {
						shfttblval[idx] = 0;
					} else {
						shfttblval[idx] = 0;
						if (idx == 5) {
							pdoffs = 0;
							FOREACH_ACTV_CORE(pi, pi->sh->hw_phyrxchain,
								core) {
								pdoffs =
								wlc_phy_pdoffset_cal_acphy(pdoffs,
								        pwrdet->pdoffset2g40[core],
								       chan_freq_range, core);
							}
							shfttblval[idx] = pdoffs & 0xffffff;
						}
					}
				}
			}
		}
	}

	if (ACMAJORREV_4(pi->pubpi.phy_rev)) {
		/*
		1.	In RSDB mode, just write to EstPWrShiftluts0
		2.	In 2x2 mode, since this table used to be a common table,
			write same entries to EstPWrShiftluts0 and EstPWrShiftluts1.
		3.	In 80p80 mode, TBD by SubraK
		*/
		FOREACH_ACTV_CORE(pi, pi->sh->hw_phyrxchain, core) {
			for (idx = 0; idx < 24; idx++) {
				shfttblval[idx] = (shfttblval[idx]>>(8*core));
			}
			wlc_phy_table_write_acphy(pi, wlc_phy_get_tbl_id_estpwrshftluts(pi, core),
				shfttbl_len, tbl_offset, 32, shfttblval);
		}
	} else {
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_ESTPWRSHFTLUTS, shfttbl_len,
			tbl_offset, 32, shfttblval);
	}
#ifdef ENABLE_FCBS
	}
#endif // endif

	ACPHY_ENABLE_STALL(pi, stall_val);
	PHY_CHANLOG(pi, __FUNCTION__, TS_EXIT, 0);
}

int8
wlc_phy_tone_pwrctrl(phy_info_t *pi, int8 tx_idx, uint8 core)
{
	int8 pwr;
	int8 targetpwr = -99, tgt_pwr_qdbm;
	int16  idle_tssi[PHY_CORE_MAX], tone_tssi[PHY_CORE_MAX];
	uint16 adjusted_tssi[PHY_CORE_MAX];
	int16 a1[PHY_CORE_MAX];
	int16 b1[PHY_CORE_MAX];
	int16 b0[PHY_CORE_MAX];
	int8 postive_slope = 1;
	int8 targetidx;
	int8 org_tx_idx;
	int8 deltapwr;
	txgain_setting_t txgain_settings;
	uint8 band;
	int8 orig_rxfarrow_shift = 0;
	/* save txidx  that comes as input */
	org_tx_idx = tx_idx;
	/* Get current subband information */
	band = wlc_phy_get_chan_freq_range_acphy(pi, 0);

	switch (band) {
		case WL_CHAN_FREQ_RANGE_2G:
			targetpwr = pi->pacalpwr2g;
			tx_idx = pi->patoneidx2g;
			break;
		case WL_CHAN_FREQ_RANGE_5G_BAND0:
			targetpwr = pi->pacalpwr5g[core*4 + 0];
			if (ACMAJORREV_2(pi->pubpi.phy_rev) || ACMAJORREV_5(pi->pubpi.phy_rev)) {
				if (CHSPEC_IS40(pi->radio_chanspec)) {
					targetpwr = pi->pacalpwr5g40[core*4 + 0];
				} else if (CHSPEC_IS80(pi->radio_chanspec)) {
					targetpwr = pi->pacalpwr5g80[core*4 + 0];
				}
			}
			tx_idx = pi->patoneidx5g[0];
			break;
		case WL_CHAN_FREQ_RANGE_5G_BAND1:
			targetpwr = pi->pacalpwr5g[core*4 + 1];
			if (ACMAJORREV_2(pi->pubpi.phy_rev) || ACMAJORREV_5(pi->pubpi.phy_rev)) {
				if (CHSPEC_IS40(pi->radio_chanspec)) {
					targetpwr = pi->pacalpwr5g40[core*4 + 1];
				} else if (CHSPEC_IS80(pi->radio_chanspec)) {
					targetpwr = pi->pacalpwr5g80[core*4 + 1];
				}
			}
			tx_idx = pi->patoneidx5g[1];
			break;
		case WL_CHAN_FREQ_RANGE_5G_BAND2:
			targetpwr = pi->pacalpwr5g[core*4 + 2];
			if (ACMAJORREV_2(pi->pubpi.phy_rev) || ACMAJORREV_5(pi->pubpi.phy_rev)) {
				if (CHSPEC_IS40(pi->radio_chanspec)) {
					targetpwr = pi->pacalpwr5g40[core*4 + 2];
				} else if (CHSPEC_IS80(pi->radio_chanspec)) {
					targetpwr = pi->pacalpwr5g80[core*4 + 2];
				}
			}
			tx_idx = pi->patoneidx5g[2];
			break;
		case WL_CHAN_FREQ_RANGE_5G_BAND3:
			targetpwr = pi->pacalpwr5g[core*4 + 3];
			if (ACMAJORREV_2(pi->pubpi.phy_rev) || ACMAJORREV_5(pi->pubpi.phy_rev)) {
				if (CHSPEC_IS40(pi->radio_chanspec)) {
					targetpwr = pi->pacalpwr5g40[core*4 + 3];
				} else if (CHSPEC_IS80(pi->radio_chanspec)) {
					targetpwr = pi->pacalpwr5g80[core*4 + 3];
				}
			}
			tx_idx = pi->patoneidx5g[3];
			break;
	}
	if (tx_idx == -1) {
		tx_idx = org_tx_idx;
	}

	if (pi->iboard) {
		tgt_pwr_qdbm = READ_PHYREGFLD(pi, TxPwrCtrlTargetPwr_path0, targetPwr0);
		targetpwr = tgt_pwr_qdbm / 4;
	} else {
		tgt_pwr_qdbm = targetpwr * 4;
	}

	if (targetpwr == -99) {
		targetidx = -1;
	} else {
		wlc_phy_get_paparams_for_band_acphy(pi, a1, b0, b1);
		/* meas the idle tssi */
		wlc_phy_txpwrctrl_idle_tssi_meas_acphy(pi);
		idle_tssi[core] = READ_PHYREGCE(pi, TxPwrCtrlIdleTssi_path, core) & 0x3ff;
		idle_tssi[core] = idle_tssi[core] - 1023;

		/* prevent crs trigger */
		wlc_phy_stay_in_carriersearch_acphy(pi, TRUE);
		if (!(ACMAJORREV_2(pi->pubpi.phy_rev) || ACMAJORREV_5(pi->pubpi.phy_rev))) {
			orig_rxfarrow_shift = READ_PHYREGFLD(pi, RxSdFeConfig6, rx_farrow_rshift_0);
			MOD_PHYREG(pi, RxSdFeConfig6, rx_farrow_rshift_0, 2);
		}
		if (RADIOID(pi->pubpi.radioid) == BCM20691_ID)
			wlc_phy_tssi_radio_setup_acphy_tiny(pi, pi->sh->hw_phyrxchain, 0);
		else
			wlc_phy_tssi_radio_setup_acphy(pi, pi->sh->hw_phyrxchain, 0);

		wlc_phy_txpwr_by_index_acphy(pi, 1, tx_idx);
		wlc_phy_get_txgain_settings_by_index_acphy(
			pi, &txgain_settings, tx_idx);
		wlc_phy_poll_samps_WAR_acphy(pi, tone_tssi,
			TRUE, FALSE, &txgain_settings, FALSE, TRUE, 0, 0);

		adjusted_tssi[core] = 1023 - postive_slope * (tone_tssi[core] - idle_tssi[core]);
		adjusted_tssi[core] = adjusted_tssi[core] >> 3;
		/* prevent crs trigger */
		wlc_phy_stay_in_carriersearch_acphy(pi, FALSE);
		if (!(ACMAJORREV_2(pi->pubpi.phy_rev) || ACMAJORREV_5(pi->pubpi.phy_rev))) {
			MOD_PHYREG(pi, RxSdFeConfig6, rx_farrow_rshift_0, orig_rxfarrow_shift);
		}
		pwr = wlc_phy_tssi2dbm_acphy(pi, adjusted_tssi[core], a1[core], b0[core], b1[core]);

		/* delta pwr in qdb */
		deltapwr = tgt_pwr_qdbm - pwr;
		if (ACMAJORREV_2(pi->pubpi.phy_rev) || ACMAJORREV_5(pi->pubpi.phy_rev)) {
			/* for 4350 with 0.5dB step size gaintable */
			targetidx = tx_idx - (deltapwr >> 1);
		} else {
			targetidx = tx_idx - deltapwr;
		}
	}

	return targetidx;
}

static int32
wlc_phy_get_estpwrlut_srom12_acphy(int16 *a, int16 *b, int16 *c, int16 *d,
		uint8 pa_fittype, uint8 core, int32 idx)
{
	int32 firstTerm = 0, secondTerm = 0, thirdTerm = 0, fourthTerm = 0;
	int32 ctrSqr = idx * idx;
	int32 pwr_est = 0;

	if (pa_fittype == 0) {
		/* logdetect */
		firstTerm  = (int32)a[core] * 128;
		secondTerm = ((int32)b[core] * idx) / 2;
		thirdTerm  = ((int32)c[core] * ctrSqr) / 128;
		fourthTerm = ((int32)d[core] * idx) / (((int32)idx - 128));
		pwr_est = (firstTerm + secondTerm +
				thirdTerm + fourthTerm) / 8192;
	} else if (pa_fittype == 1) {
		/* diode type */
		firstTerm = (int32)a[core] * 16;
		secondTerm = (b[core] * ctrSqr) / 4096;
		if (idx == 0)
			thirdTerm = 0;
		else
			thirdTerm = c[core] * ctrSqr /
				(ctrSqr - ((int32)d[core] * 2));
		pwr_est = (firstTerm + secondTerm + thirdTerm) / 1024;
	} else {
		/* original */
		firstTerm  = 8 * (16 * (int32)b[core]
				+ (int32)c[core] * idx);
		secondTerm = 32768 + (int32)a[core] * idx;
		pwr_est = MAX(((4 * firstTerm + secondTerm/2)
					/secondTerm), -8);
		pwr_est = MIN(pwr_est, 0x7F);
	}
	return pwr_est;

}

static void
wlc_phy_get_paparams_percore_srom12_acphy(phy_info_t *pi, uint8 chan_freq_range,
		int16 *a, int16 *b, int16 *c, int16 *d, uint8 core)
{
	srom12_pwrdet_t *pwrdet = pi->pwrdet_ac;

	switch (chan_freq_range) {
		case WL_CHAN_FREQ_RANGE_2G:
		case WL_CHAN_FREQ_RANGE_5G_BAND0:
		case WL_CHAN_FREQ_RANGE_5G_BAND1:
		case WL_CHAN_FREQ_RANGE_5G_BAND2:
		case WL_CHAN_FREQ_RANGE_5G_BAND3:
		case WL_CHAN_FREQ_RANGE_5G_BAND4:
		   a[core] =  pwrdet->pwrdet_a[core][chan_freq_range];
		   b[core] =  pwrdet->pwrdet_b[core][chan_freq_range];
		   c[core] =  pwrdet->pwrdet_c[core][chan_freq_range];
		   d[core] =  pwrdet->pwrdet_d[core][chan_freq_range];
		   break;
		case WL_CHAN_FREQ_RANGE_2G_40:
		case WL_CHAN_FREQ_RANGE_5G_BAND0_40:
		case WL_CHAN_FREQ_RANGE_5G_BAND1_40:
		case WL_CHAN_FREQ_RANGE_5G_BAND2_40:
		case WL_CHAN_FREQ_RANGE_5G_BAND3_40:
		case WL_CHAN_FREQ_RANGE_5G_BAND4_40:
		   /* Adjust index for 40M */
		   a[core] =  pwrdet->pwrdet_a_40[core][chan_freq_range - 6];
		   b[core] =  pwrdet->pwrdet_b_40[core][chan_freq_range - 6];
		   c[core] =  pwrdet->pwrdet_c_40[core][chan_freq_range - 6];
		   d[core] =  pwrdet->pwrdet_d_40[core][chan_freq_range - 6];
		   break;
		case WL_CHAN_FREQ_RANGE_5G_BAND0_80:
		case WL_CHAN_FREQ_RANGE_5G_BAND1_80:
		case WL_CHAN_FREQ_RANGE_5G_BAND2_80:
		case WL_CHAN_FREQ_RANGE_5G_BAND3_80:
		case WL_CHAN_FREQ_RANGE_5G_BAND4_80:
		   /* Adjust index for 80M */
		   a[core] =  pwrdet->pwrdet_a_80[core][chan_freq_range - 12];
			b[core] =  pwrdet->pwrdet_b_80[core][chan_freq_range - 12];
		   c[core] =  pwrdet->pwrdet_c_80[core][chan_freq_range - 12];
		   d[core] =  pwrdet->pwrdet_d_80[core][chan_freq_range - 12];
		   break;
		default:
			PHY_ERROR(("wl%d: %s: invalid freq_range for core%d\n",
					pi->sh->unit, __FUNCTION__, core));
			ASSERT(0);
			break;
	}
}

static void
wlc_phy_get_paparams_srom12_acphy(phy_info_t *pi, uint8 chan_freq_range,
		int16 *a, int16 *b, int16 *c, int16 *d)
{
	uint8 core;

	FOREACH_CORE(pi, core) {
		wlc_phy_get_paparams_percore_srom12_acphy(pi,
				chan_freq_range, a, b, c, d, core);
	}
}

static void
wlc_phy_get_paparams_80p80_srom12_acphy(phy_info_t *pi, uint8 *chan_freqs,
		int16 *a, int16 *b, int16 *c, int16 *d)
{
	uint8 core, chan_freq_range;

	FOREACH_CORE(pi, core) {
		if (ACMAJORREV_33(pi->pubpi.phy_rev)) {
			/* core 0/1: 80L, core 2/3: 80U */
			chan_freq_range = (core <= 1) ? chan_freqs[0] : chan_freqs[1];
		} else {
			chan_freq_range = chan_freqs[0];
		}

		wlc_phy_get_paparams_percore_srom12_acphy(pi,
				chan_freq_range, a, b, c, d, core);
	}
}

static void
wlc_phy_txpwrctrl_pwr_setup_srom12_acphy(phy_info_t *pi)
{
	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;
	srom12_pwrdet_t *pwrdet = pi->pwrdet_ac;
	int16  a[PHY_CORE_MAX], b[PHY_CORE_MAX];
	int16  c[PHY_CORE_MAX], d[PHY_CORE_MAX];
	int16  ak[PHY_CORE_MAX], bk[PHY_CORE_MAX];
	int16  ck[PHY_CORE_MAX], dk[PHY_CORE_MAX];
	int32  idx;
	uint32 pdoffs = 0;
	uint32 shfttbl_len = 24, shfttblval[40], idx1;
	int32  pwr_est, pwr_est0, pwr_est1, pwr_est_cck, tbl_len, tbl_offset;
	uint32 regval[128];
	int8   pdoffsets[PHY_CORE_MAX*2];
	int8   target_min_limit;
	uint8  stall_val;
	uint8  chan_freq_range, iidx, chan_freq = 0, poffs = 0;
	uint8  chan_freqs[NUM_CHANS_IN_CHAN_BONDING];
	uint8  core;
	uint8  tssi_delay = 0;
	uint8  mult_mode = 0;
	uint8  pa_fittype = 0;
	uint8  using_estpwr_lut_cck = 0;
#ifdef PREASSOC_PWRCTRL
	bool init_idx_carry_from_lastchan;
	uint8 step_size, prev_target_qdbm;
#endif // endif
	int8 tp_qtrdbm_each_core[PHY_CORE_MAX]; /* TP for each core */

#if defined(PHYCAL_CACHING)
	ch_calcache_t *ctx = NULL;
	ctx = wlc_phy_get_chanctx(pi, pi->radio_chanspec);
	BCM_REFERENCE(ctx);
#endif /* PHYCAL_CACHING */

	if (SROMREV(pi->sh->sromrev) < 12) {
		return;
	}

	iidx = 0;
	tbl_len = 128;
	tbl_offset = 0;
	pwr_est_cck = 0;

	//24~40 of ESTPWRSHFTLUTS0 is for BW160 definition
	if (PHY_AS_80P80_CAP(pi))
		shfttbl_len = 40;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
	ACPHY_DISABLE_STALL(pi);
	/* enable TSSI */
	MOD_PHYREG(pi, TSSIMode, tssiEn, 1);
	MOD_PHYREG(pi, TxPwrCtrlCmd, txPwrCtrl_en, 0);

	/* initialize a, b, c,d to be all zero */
	for (idx = 0; idx < PHY_CORE_MAX; idx++) {
		a[idx] = 0; ak[idx] = 0;
		b[idx] = 0; bk[idx] = 0;
		c[idx] = 0; ck[idx] = 0;
		d[idx] = 0; dk[idx] = 0;
	}

	if (RADIOREV(pi->pubpi.radiorev) == 4 || RADIOREV(pi->pubpi.radiorev) == 8 ||
	    ACMAJORREV_2(pi->pubpi.phy_rev) || ACMAJORREV_5(pi->pubpi.phy_rev) ||
	    ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev)) {
		/* 4360B0/B1/4350 using 0.5dB-step gaintbl, bbmult interpolation enabled */
		MOD_PHYREG(pi, TxPwrCtrlCmd, bbMultInt_en, 1);
#ifdef PREASSOC_PWRCTRL
		step_size = 2;
#endif // endif
	} else {
		/* disable bbmult interpolation
		   to work with a 0.25dB step txGainTbl
		*/
		MOD_PHYREG(pi, TxPwrCtrlCmd, bbMultInt_en, 0);
#ifdef PREASSOC_PWRCTRL
		step_size = 1;
#endif // endif
	}

	/* Get pwrdet params from SROM for current subband */
	if (PHY_AS_80P80(pi, pi->radio_chanspec)) {
		wlc_phy_get_chan_freq_range_80p80_srom12_acphy(pi,
			pi->radio_chanspec, chan_freqs);
		wlc_phy_get_paparams_80p80_srom12_acphy(pi, chan_freqs, a, b, c, d);
		// FIXME: 80p80
		chan_freq_range = chan_freqs[0];
	} else {
		chan_freq_range = wlc_phy_get_chan_freq_range_srom12_acphy(pi,
			pi->radio_chanspec);
		wlc_phy_get_paparams_srom12_acphy(pi, chan_freq_range, a, b, c, d);
	}

	if (ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev)) {
		if (CHSPEC_IS2G(pi->radio_chanspec) &&
		    (pi->u.pi_acphy->srom_2g_pdrange_id == 2)) {
			/* Using separate estPwrLuts for 2G CCK for
			 * the 2G-only board to improve the performance
			 */
			using_estpwr_lut_cck = 1;
			if (CHSPEC_IS20(pi->radio_chanspec)) {
				wlc_phy_get_paparams_srom12_acphy(pi,
					WL_CHAN_FREQ_RANGE_5G_BAND0, ak, bk, ck, dk);
			} else {
				wlc_phy_get_paparams_srom12_acphy(pi,
					WL_CHAN_FREQ_RANGE_5G_BAND0_40, ak, bk, ck, dk);
			}
		}
	}

	/* target power */
	wlc_phy_txpwrctrl_update_minpwr_acphy(pi);
	target_min_limit = pi->min_txpower * WLC_TXPWR_DB_FACTOR;

	FOREACH_ACTV_CORE(pi, pi->sh->hw_phyrxchain, core) {
		int8 target_pwr_qtrdbm;
		target_pwr_qtrdbm = (int8)pi->tx_power_max_per_core[core];
		/* never target below the min threashold */
		if (target_pwr_qtrdbm < target_min_limit)
			target_pwr_qtrdbm = target_min_limit;

		tp_qtrdbm_each_core[core] = target_pwr_qtrdbm;
	}

	/* determine pos/neg TSSI slope */
	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		MOD_PHYREG(pi, TSSIMode, tssiPosSlope, pi->fem2g->tssipos);
	} else {
		MOD_PHYREG(pi, TSSIMode, tssiPosSlope, pi->fem5g->tssipos);
	}
	MOD_PHYREG(pi, TSSIMode, tssiPosSlope, 1);

	/* disable txpwrctrl during idleTssi measurement, etc */
	MOD_PHYREG(pi, TxPwrCtrlCmd, txPwrCtrl_en, 0);

#ifdef PREASSOC_PWRCTRL
	FOREACH_ACTV_CORE(pi, pi->sh->hw_phyrxchain, core) {
		init_idx_carry_from_lastchan = (CHSPEC_IS5G(pi->radio_chanspec)) ?
		        pi->u.pi_acphy->pwr_ctrl_save->status_idx_carry_5g[core]
		        : pi->u.pi_acphy->pwr_ctrl_save->status_idx_carry_2g[core];
		if (!init_idx_carry_from_lastchan) {
			/* 4360B0 using 0.5dB-step gaintbl so start with a lower starting idx */
			if ((RADIOREV(pi->pubpi.radiorev) == 4) ||
			    (RADIOREV(pi->pubpi.radiorev) == 8) ||
			    (ACMAJORREV_5(pi->pubpi.phy_rev))) {
				iidx = 20;
			} else {
				iidx = 50;
			}
			MOD_PHYREGCEE(pi, TxPwrCtrlInit_path, core, pwrIndex_init_path, iidx);
			if (CHSPEC_IS5G(pi->radio_chanspec)) {
				pi->u.pi_acphy->pwr_ctrl_save->status_idx_carry_5g[core] = TRUE;
			} else {
				pi->u.pi_acphy->pwr_ctrl_save->status_idx_carry_2g[core] = TRUE;
			}
			/* XXX This is the handshake for te code that
			   is put in for open loop power control
			   128 means do NOT restore the value in the
			   open loop function
			*/
			pi_ac->txpwrindex_hw_save[core] = 128;
		} else {
			/* set power index initial condition */
			int32 new_iidx;

			if (CHSPEC_IS5G(pi->radio_chanspec)) {
				iidx = pi->u.pi_acphy->pwr_ctrl_save->status_idx_5g[core];
				prev_target_qdbm = pi->u.pi_acphy->pwr_ctrl_save->pwr_qdbm_5g[core];
				if ((pi->u.pi_acphy->pwr_ctrl_save->stored_not_restored_5g[core])) {
					/* XXX This is the handshake for
					   the code that is put in for open loop power control
					   128 means do NOT restore the value in the
					   open loop function
					*/
					pi->u.pi_acphy->pwr_ctrl_save->stored_not_restored_5g[core]
						= FALSE;
					pi_ac->txpwrindex_hw_save[core] = 128;
				}
			} else {
				iidx = pi->u.pi_acphy->pwr_ctrl_save->status_idx_2g[core];
				prev_target_qdbm = pi->u.pi_acphy->pwr_ctrl_save->pwr_qdbm_2g[core];
				if ((pi->u.pi_acphy->pwr_ctrl_save->stored_not_restored_2g[core])) {
					/* XXX This is the handshake for
					   the code that is put in for open loop power control
					   128 means do NOT restore the value in the
					   open loop function
					*/
					pi->u.pi_acphy->pwr_ctrl_save->stored_not_restored_2g[core]
						= FALSE;
					pi_ac->txpwrindex_hw_save[core] = 128;
				}
			}
			new_iidx = (int32)iidx + ((int32)tp_qtrdbm_each_core[core]
			                          - prev_target_qdbm) / step_size;

			/* XXX Sanity Check
			   XXX make sure iidx is within bounds to be safe
			*/
			if (new_iidx < PWRCTRL_MIN_INIT_IDX) {
				iidx = PWRCTRL_MIN_INIT_IDX;
			} else if (new_iidx > PWRCTRL_MAX_INIT_IDX) {
				iidx = PWRCTRL_MAX_INIT_IDX;
			} else {
				iidx = (uint8)new_iidx;
			}

			MOD_PHYREGCEE(pi, TxPwrCtrlInit_path, core, pwrIndex_init_path, iidx);
		}
	}
#else
	if ((RADIOREV(pi->pubpi.radiorev) == 4) || (RADIOREV(pi->pubpi.radiorev) == 8) ||
	    (ACMAJORREV_5(pi->pubpi.phy_rev))) {
		iidx = 20;
	} else {
		iidx = 50;
	}
#if defined(PHYCAL_CACHING)
	if (!ctx || !ctx->valid)
#endif /* PHYCAL_CACHING */
		FOREACH_ACTV_CORE(pi, pi->sh->hw_phyrxchain, core) {
			MOD_PHYREGCEE(pi, TxPwrCtrlInit_path, core, pwrIndex_init_path, iidx);
		}
#endif /* PREASSOC_PWRCTRL */
	/* MOD_PHYREG(pi, TxPwrCtrlIdleTssi, rawTssiOffsetBinFormat, 1); */

	/* sample TSSI at 7.5us */
	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		if (pi->u.pi_acphy->srom_2g_pdrange_id >= 5) {
			tssi_delay = 150;
		} else if (pi->u.pi_acphy->srom_2g_pdrange_id >= 4) {
			tssi_delay = 220;
		} else {
			tssi_delay = 150;
		}
	} else {
		if (pi->u.pi_acphy->srom_5g_pdrange_id >= 5) {
			tssi_delay = 150;
		} else if (pi->u.pi_acphy->srom_5g_pdrange_id >= 4) {
			tssi_delay = 220;
		} else {
			tssi_delay = 150;
		}
	}

	if (ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev)) {
		tssi_delay = 200;
		MOD_PHYREG(pi, perPktIdleTssiCtrl, perPktIdleTssiUpdate_en, 0);
		MOD_PHYREG(pi, TSSIMode, TwoPwrRange, 0);

		MOD_PHYREG(pi, TssiAccumCtrl, tssi_accum_en, 1);
		MOD_PHYREG(pi, TssiAccumCtrl, tssi_filter_pos, 1);
		MOD_PHYREG(pi, TssiAccumCtrl, Ntssi_accum_delay, 200);
		MOD_PHYREG(pi, TssiAccumCtrl, Ntssi_intg_log2, 4);

		MOD_PHYREG(pi, TssiAccumCtrlcck, Ntssi_accum_delay_cck, 200);
		MOD_PHYREG(pi, TssiAccumCtrlcck, Ntssi_intg_log2_cck, 4);

		mult_mode = 2;
		MOD_PHYREG(pi, TxPwrCtrl_Multi_Mode0, multi_mode, mult_mode);
		MOD_PHYREG(pi, TxPwrCtrl_Multi_Mode1, multi_mode, mult_mode);
		MOD_PHYREG(pi, TxPwrCtrl_Multi_Mode2, multi_mode, mult_mode);
		MOD_PHYREG(pi, TxPwrCtrl_Multi_Mode3, multi_mode, mult_mode);
	}

	MOD_PHYREG(pi, TxPwrCtrlNnum, Ntssi_delay, tssi_delay);

#if defined(PREASSOC_PWRCTRL) && !defined(WLC_LOW_ONLY)
	/* average over 2 or 16 packets */
	wlc_phy_pwrctrl_shortwindow_upd_acphy(pi, pi->channel_short_window);
#else
	MOD_PHYREG(pi, TxPwrCtrlNnum, Npt_intg_log2, PWRCTRL_LONGW_AVG);
#endif /* PREASSOC_PWRCTRL */

	/* decouple IQ comp and LOFT comp from Power Control */
	MOD_PHYREG(pi, TxPwrCtrlCmd, use_txPwrCtrlCoefsIQ, 0);
	MOD_PHYREG(pi, TxPwrCtrlCmd, use_txPwrCtrlCoefsLO,
		(ACREV_IS(pi->pubpi.phy_rev, 1) || ACREV_IS(pi->pubpi.phy_rev, 9)) ? 1 : 0);

#ifdef WL_SARLIMIT
	wlc_phy_set_sarlimit_acphy(pi);
#endif // endif
	FOREACH_ACTV_CORE(pi, pi->sh->hw_phyrxchain, core) {
		if (pi_ac->offset_targetpwr) {
			if (core == 0) {
				uint8 tgt_pwr_qdbm = tp_qtrdbm_each_core[core];
				tgt_pwr_qdbm -= (pi_ac->offset_targetpwr << 2);
				wlc_phy_txpwrctrl_set_target_acphy(pi, tgt_pwr_qdbm, 0);
			}
		} else {
			/* set target powers */
			wlc_phy_txpwrctrl_set_target_acphy(pi, tp_qtrdbm_each_core[core], core);
		}
		PHY_TXPWR(("wl%d: %s: txpwrctl[%d]: %d\n",
		           pi->sh->unit, __FUNCTION__, core, tp_qtrdbm_each_core[core]));
	}
#ifdef ENABLE_FCBS
	if (IS_FCBS(pi) && pi->phy_fcbs->FCBS_INPROG)
		pi->phy_fcbs->FCBS_INPROG = 0;
	else {
#endif // endif
		/* load estimated power tables (maps TSSI to power in dBm)
		 *    entries in tx power table 0000xxxxxx
		 */
		tbl_len = 128;
		tbl_offset = 0;

		if (ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev)) {
			wlc_phy_get_srom12_pdoffset_acphy(pi, pdoffsets);

			// the following is for debugging only
			PHY_TXPWR(("wl%d: %s: pdoffsets: ", pi->sh->unit, __FUNCTION__));
			for (idx = 0; idx < 8; idx++) {
				PHY_TXPWR(("%d ", pdoffsets[idx]));
			}
			PHY_TXPWR(("\n"));
		}

		/* getting pa fittype */
		pa_fittype = wlc_phy_fittype_srom12_acphy(pi);
		PHY_TXPWR(("wl%d: %s:pa_fittype = %d\n",
			pi->sh->unit, __FUNCTION__, pa_fittype));

		FOREACH_ACTV_CORE(pi, pi->sh->hw_phyrxchain, core) {

			PHY_TXPWR(("wl%d: %s: paparam of core%d = [%4x, %4x, %4x, %4x]\n",
				pi->sh->unit, __FUNCTION__, core,
				a[core], b[core], c[core], d[core]));

			for (idx = 0; idx < tbl_len; idx++) {
				pwr_est = wlc_phy_get_estpwrlut_srom12_acphy(
						a, b, c, d,
						pa_fittype, core, idx);

				if (using_estpwr_lut_cck == 1) {
					pwr_est_cck = wlc_phy_get_estpwrlut_srom12_acphy(
							ak, bk, ck, dk,
							pa_fittype, core, idx);
				}

				if (ACMAJORREV_32(pi->pubpi.phy_rev) ||
					ACMAJORREV_33(pi->pubpi.phy_rev)) {
					if (CHSPEC_IS5G(pi->radio_chanspec)) {
						pwr_est0 = pwr_est + pdoffsets[core];
						pwr_est1 = pwr_est + pdoffsets[core+PHY_CORE_MAX];
					} else {
						if (CHSPEC_IS20(pi->radio_chanspec)) {
							pwr_est0 = pwr_est;
							pwr_est1 = 0;
							if (using_estpwr_lut_cck == 1) {
								pwr_est = pwr_est_cck;
							} else {
								pwr_est = pwr_est0
								+ pdoffsets[core+PHY_CORE_MAX];
							}
						} else {
							pwr_est1 = pwr_est;
							pwr_est0 = pwr_est1 + pdoffsets[core];
							/* CCK pdoffset in 40MHz only */
							if (using_estpwr_lut_cck == 1) {
								pwr_est = pwr_est_cck;
							} else {
								pwr_est = pwr_est1
								+ pdoffsets[core+PHY_CORE_MAX];
							}
						}
					}

					pwr_est  = MIN(0x7f, (MAX(pwr_est,  0)));
					pwr_est0 = MIN(0x7f, (MAX(pwr_est0, 0)));
					pwr_est1 = MIN(0x7f, (MAX(pwr_est1, 0)));

					regval[idx] = ((uint32)(pwr_est0&0xff)) |
					        (((uint32)(pwr_est1&0xff)) << 8) |
					        (((uint32)(pwr_est&0xff)) << 16);
				PHY_TXPWR(("%s: EstPower Table (core =%d),(idx = %d),Value = %x \n",
					__FUNCTION__, core, idx, regval[idx]));

				} else {
					pwr_est  = MIN(0x7f, (MAX(pwr_est, 0)));
					regval[idx] = (uint16)(pwr_est&0xff);
				PHY_TXPWR(("%s: EstPower Table (core =%d),(idx = %d),Value = %u \n",
					__FUNCTION__, core, idx, regval[idx]));
				}
			}

			if (ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev)) {
				/* Est Pwr Table is 128x24 Table. Limit Write to 8 bits */
				wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_ESTPWRLUTS(core),
					tbl_len, tbl_offset, 32, regval);
			} else {
				/* Est Pwr Table is 128x8 Table. Limit Write to 8 bits */
				wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_ESTPWRLUTS(core),
					tbl_len, tbl_offset, 16, regval);
			}
		}

		/* start to populate estPwrShftTbl */

		if (CHSPEC_IS5G(pi->radio_chanspec)) {
			if (CHSPEC_IS80(pi->radio_chanspec) ||
			    PHY_AS_80P80(pi, pi->radio_chanspec)) {
				// FIXME for 160 and 80p80
				chan_freq = chan_freq_range - 12;
			} else if (CHSPEC_IS160(pi->radio_chanspec)) {
				chan_freq = chan_freq_range - 12;
				ASSERT(0); // FIXME
			} else if (CHSPEC_IS40(pi->radio_chanspec)) {
				chan_freq = chan_freq_range - 7;
			} else {
				chan_freq = chan_freq_range - 1;
			}
		}
		for (idx1 = 0; idx1 < shfttbl_len; idx1++) {
			pdoffs = 0;
			FOREACH_ACTV_CORE(pi, pi->sh->hw_phyrxchain, core) {
				if (CHSPEC_IS5G(pi->radio_chanspec)) {
					if ((idx1 == 0)||((idx1 > 6)&&(idx1 < 10))) {
					poffs = (uint8)(pwrdet->pdoffset20in80[core][chan_freq+1]);
					} else if ((idx1 == 1)||(idx1 == 6)) {
					poffs = (uint8)(pwrdet->pdoffset40in80[core][chan_freq+1]);
					} else if ((idx1 == 2) || (idx1 == 4)) {
					poffs = (uint8)(pwrdet->pdoffset20in40[core][chan_freq+1]);
					} else {
						poffs = 0;
					}
				} else {
					if (idx1 == 17) {
						poffs = (uint8)(pwrdet->pdoffsetcck[core]);
					} else {
						poffs = 0;
					}
				}
				poffs = (poffs > 15) ? (0xe0 | poffs) : poffs;
				pdoffs = pdoffs | (poffs << core*8);
			}
			shfttblval[idx1] = (ACMAJORREV_32(pi->pubpi.phy_rev) ||
				ACMAJORREV_33(pi->pubpi.phy_rev))? 0 : (pdoffs & 0xffffff);
		}
		if (ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev)) {
			wlc_phy_table_write_acphy(pi, AC2PHY_TBL_ID_ESTPWRSHFTLUTS0, shfttbl_len,
				tbl_offset, 32, shfttblval);
		} else {
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_ESTPWRSHFTLUTS, shfttbl_len,
				tbl_offset, 32, shfttblval);
		}
#ifdef ENABLE_FCBS
	}
#endif // endif
	ACPHY_ENABLE_STALL(pi, stall_val);
}

int8 wlc_phy_fittype_srom12_acphy(phy_info_t *pi)
{
	int8 pdet_range;
	if (CHSPEC_IS5G(pi->radio_chanspec))
		pdet_range = pi->u.pi_acphy->srom_5g_pdrange_id;
	else
		pdet_range = pi->u.pi_acphy->srom_2g_pdrange_id;

	switch (pdet_range) {
	case 24:
		return 0; /* Log detector for both 2G and 5G */
	case 25:
	case 26:
		return 1; /* Diode detector for both 2G and 5G */
	default:
		if (pi->sromi->sr13_dettype_en) {
			if (CHSPEC_IS5G(pi->radio_chanspec))
				return pi->sromi->dettype_5g;
			else
				return pi->sromi->dettype_2g;
		} else {
			if (!(ACMAJORREV_32(pi->pubpi.phy_rev) ||
				ACMAJORREV_33(pi->pubpi.phy_rev))) {
				if (CHSPEC_IS5G(pi->radio_chanspec)) {
					return 0;
				} else {
					return 1;
				}
			} else {
				if (pdet_range == 0) {
					/* logdetect - 4366 MC card */
					return 0;
				} else if ((CHSPEC_IS5G(pi->radio_chanspec) &&
						(pdet_range == 2))) {
					/* original: 4366 MCH5L - will be obsolete soon */
					return 2;
				} else {
					/* diode type: 4366 MCH2L,MCM2/5L, etc */
					return 1;
				}
			}
		} /* end-if */
	} /* end-switch */
}

static void
wlc_phy_get_srom12_pdoffset_acphy(phy_info_t *pi, int8 *pdoffs)
{

	memset(pdoffs, 0, 2*PHY_CORE_MAX * sizeof(int8));

	if (!(SROMREV(pi->sh->sromrev) < 12)) {
		srom12_pwrdet_t *pwrdet = pi->pwrdet_ac;
		int8  poffs1, poffs2;
		uint8 core, band = 0;
		uint8 bands[NUM_CHANS_IN_CHAN_BONDING];

		/* to figure out which subband is in 5G */
		/* in the range of 0, 1, 2, 3, 4, 5 */
		if (PHY_AS_80P80(pi, pi->radio_chanspec)) {
			wlc_phy_get_chan_freq_range_80p80_srom12_acphy(pi,
				pi->radio_chanspec, bands);
			bands[0] =  bands[0] - WL_CHAN_FREQ_RANGE_5G_BAND0_80 + 1;
			bands[1] =  bands[1] - WL_CHAN_FREQ_RANGE_5G_BAND0_80 + 1;
		} else {
			band = wlc_phy_get_chan_freq_range_srom12_acphy(pi, pi->radio_chanspec);
			if (band >= WL_CHAN_FREQ_RANGE_5G_BAND0_80) {
				ASSERT((CHSPEC_IS80(pi->radio_chanspec)));
				band = band - WL_CHAN_FREQ_RANGE_5G_BAND0_80 + 1;
			} else if (band >= WL_CHAN_FREQ_RANGE_2G_40) {
				ASSERT(CHSPEC_IS40(pi->radio_chanspec));
				band = band - WL_CHAN_FREQ_RANGE_2G_40;
			} else {
				ASSERT(CHSPEC_IS20(pi->radio_chanspec));
			}
		}

		FOREACH_CORE(pi, core) {
			if (CHSPEC_IS5G(pi->radio_chanspec)) {
				if (PHY_AS_80P80(pi, pi->radio_chanspec)) {
					if (ACMAJORREV_33(pi->pubpi.phy_rev)) {
						/* core 0/1: 80L, core 2/3: 80U */
						band = (core <= 1) ? bands[0] : bands[1];
					} else {
						band = bands[0];
					}
					poffs1 = (uint8)(pwrdet->pdoffset20in80[core][band]);
					poffs2 = (uint8)(pwrdet->pdoffset40in80[core][band]);
					poffs1 -= (poffs1 >= 16)? 32 : 0;
					poffs2 -= (poffs2 >= 16)? 32 : 0;
					pdoffs[core] = poffs1;
					pdoffs[PHY_CORE_MAX+core] = poffs2;
				} else if (CHSPEC_IS160(pi->radio_chanspec)) {
					pdoffs[PHY_CORE_MAX+core] = pdoffs[core] = 0;
					ASSERT(0); // FIXME
				} else if (CHSPEC_IS80(pi->radio_chanspec)) {
					poffs1 = (uint8)(pwrdet->pdoffset20in80[core][band]);
					poffs2 = (uint8)(pwrdet->pdoffset40in80[core][band]);
					poffs1 -= (poffs1 >= 16)? 32 : 0;
					poffs2 -= (poffs2 >= 16)? 32 : 0;
					pdoffs[core] = poffs1;
					pdoffs[PHY_CORE_MAX+core] = poffs2;
				} else if (CHSPEC_IS40(pi->radio_chanspec)) {
					poffs1 = (uint8)(pwrdet->pdoffset20in40[core][band]);
					poffs1 -= (poffs1 >= 16)? 32 : 0;
					pdoffs[core] = poffs1;
					pdoffs[PHY_CORE_MAX+core] = 0;
				} else {
					pdoffs[core] = 0;
					pdoffs[PHY_CORE_MAX+core] = 0;
				}
			} else {
				if (CHSPEC_IS40(pi->radio_chanspec)) {
					poffs1 = (uint8)(pwrdet->pdoffset20in40[core][band]);
					poffs1 -= (poffs1 >= 16)? 32 : 0;
					pdoffs[core] = poffs1;
					/* pdoffset for cck 20in40MHz */
					poffs2 = (uint8)(pwrdet->pdoffsetcck20m[core]);
					poffs2 -= (poffs2 >= 16)? 32 : 0;
					pdoffs[PHY_CORE_MAX+core] = poffs2;
				} else {
					pdoffs[core] = 0;
					/* pdoffset for cck in 20MHz */
					poffs2 = (uint8)(pwrdet->pdoffsetcck[core]);
					poffs2 -= (poffs2 >= 16)? 32 : 0;
					pdoffs[PHY_CORE_MAX+core] = poffs2;
				}
			}
		}
	}
}

#ifdef PREASSOC_PWRCTRL
static void
wlc_phy_pwrctrl_shortwindow_upd_acphy(phy_info_t *pi, bool shortterm)
{
	if (shortterm) {
		/* 2 packet avergaing */
		MOD_PHYREG(pi, TxPwrCtrlNnum, Npt_intg_log2, PWRCTRL_SHORTW_AVG);
		if (ACMAJORREV_4(pi->pubpi.phy_rev) || ACMAJORREV_32(pi->pubpi.phy_rev) ||
			ACMAJORREV_33(pi->pubpi.phy_rev)) {
			MOD_PHYREG(pi, TxPwrCtrlDamping, DeltaPwrDamp, 64);
		}
	} else {
		if (ACMAJORREV_4(pi->pubpi.phy_rev) || ACMAJORREV_32(pi->pubpi.phy_rev) ||
			ACMAJORREV_33(pi->pubpi.phy_rev)) {
			/* 4 packet avergaing with Damp of 0.25 */
			MOD_PHYREG(pi, TxPwrCtrlNnum, Npt_intg_log2, 2);
			MOD_PHYREG(pi, TxPwrCtrlDamping, DeltaPwrDamp, 16);
		} else {
			/* 16 packet avergaing */
			MOD_PHYREG(pi, TxPwrCtrlNnum, Npt_intg_log2, PWRCTRL_LONGW_AVG);

		}
	}
}

static uint8
wlc_phy_txpwrctrl_get_target_acphy(phy_info_t *pi, uint8 core)
{
	/* set target powers in 6.2 format (in dBs) */
	switch (core) {
	case 0:
		return READ_PHYREGFLD(pi, TxPwrCtrlTargetPwr_path0, targetPwr0);
		break;
	case 1:
		return READ_PHYREGFLD(pi, TxPwrCtrlTargetPwr_path1, targetPwr1);
		break;
	case 2:
		return READ_PHYREGFLD(pi, TxPwrCtrlTargetPwr_path2, targetPwr2);
		break;
	case 3:
		return READ_PHYREGFLD(pi, TxPwrCtrlTargetPwr_path3, targetPwr3);
		break;
	}
	return 0;
}

#endif /* PREASSOC_PWRCTRL */

static
void wlc_phy_get_tssi_floor_acphy(phy_info_t *pi, int16 *floor)
{
	srom11_pwrdet_t *pwrdet = pi->pwrdet_ac;
	uint8 chan_freq_range, core;

	chan_freq_range = wlc_phy_get_chan_freq_range_acphy(pi, 0);

	FOREACH_CORE(pi, core) {
		switch (chan_freq_range) {
		case WL_CHAN_FREQ_RANGE_2G:
		case WL_CHAN_FREQ_RANGE_5G_BAND0:
		case WL_CHAN_FREQ_RANGE_5G_BAND1:
		case WL_CHAN_FREQ_RANGE_5G_BAND2:
		case WL_CHAN_FREQ_RANGE_5G_BAND3:
			*floor = pwrdet->tssifloor[core][chan_freq_range];
		break;
		}
	}

}

static uint32
wlc_phy_pdoffset_cal_acphy(uint32 pdoffs, uint16 pdoffset, uint8 band, uint8 core)
{
	uint8 pdoffs_t;
	switch (band) {
	case WL_CHAN_FREQ_RANGE_2G:
		pdoffs_t = pdoffset & 0xf; break;
	case WL_CHAN_FREQ_RANGE_5G_BAND0:
		pdoffs_t = pdoffset & 0xf; break;
	case WL_CHAN_FREQ_RANGE_5G_BAND1:
		pdoffs_t = (pdoffset >> 4) & 0xf; break;
	case WL_CHAN_FREQ_RANGE_5G_BAND2:
		pdoffs_t = (pdoffset >> 8) & 0xf; break;
	case WL_CHAN_FREQ_RANGE_5G_BAND3:
		pdoffs_t = (pdoffset >> 12) & 0xf; break;
	default:
		pdoffs_t = pdoffset & 0xf; break;
	}

	pdoffs_t = (pdoffs_t > 7) ? (0xf0|pdoffs_t) : pdoffs_t;
	pdoffs   = pdoffs | (pdoffs_t << core*8);
	return pdoffs;

}

static void
wlc_phy_gaintbl_blanking(phy_info_t *pi, uint16 *tx_pwrctrl_tbl, uint8 txidxcap)
{
	uint16 k, m, K;
	/* ACPHY has 48bit gaincode = 3 16-bit word */
	uint16 nword_per_gaincode = 3;

	K = txidxcap & 0xFF;

	for (k = 0; k < K; k++) {
		for (m = 0; m < nword_per_gaincode; m++) {
			tx_pwrctrl_tbl[k*nword_per_gaincode + m] =
			        tx_pwrctrl_tbl[K*nword_per_gaincode + m];
		}
	}
}

static bool
wlc_phy_txpwrctrl_ison_acphy(phy_info_t *pi)
{
	uint16 mask = (ACPHY_TxPwrCtrlCmd_txPwrCtrl_en_MASK(pi->pubpi.phy_rev) |
	               ACPHY_TxPwrCtrlCmd_hwtxPwrCtrl_en_MASK(pi->pubpi.phy_rev) |
	               ACPHY_TxPwrCtrlCmd_use_txPwrCtrlCoefs_MASK(pi->pubpi.phy_rev));

	return ((READ_PHYREG(pi, TxPwrCtrlCmd) & mask) == mask);
}

static uint8
wlc_phy_set_txpwr_clamp_acphy(phy_info_t *pi)
{
	uint16 idle_tssi_shift, adj_tssi_min;
	int16 tssi_floor = 0;
	int16 idleTssi_2C = 0;
	int16 a1 = 0, b0 = 0, b1 = 0;
	uint8 pwr = 0;

	wlc_phy_get_tssi_floor_acphy(pi, &tssi_floor);
	wlc_phy_get_paparams_for_band_acphy(pi, &a1, &b0, &b1);

	idleTssi_2C = READ_PHYREGFLD(pi, TxPwrCtrlIdleTssi_path0, idleTssi0);
	if (idleTssi_2C >= 512) {
		idle_tssi_shift = idleTssi_2C - 1023 - (-512);
	} else {
		idle_tssi_shift = 1023 + idleTssi_2C - 511;
	}
	idle_tssi_shift = idle_tssi_shift + 4;
	adj_tssi_min = MAX(tssi_floor, idle_tssi_shift);
	/* convert to 7 bits */
	adj_tssi_min = adj_tssi_min >> 3;

	pwr = wlc_phy_tssi2dbm_acphy(pi, adj_tssi_min, a1, b0, b1);

	return pwr;
}

static void
wlc_phy_txpower_recalc_target_acphy(phy_info_t *pi)
{
	srom11_pwrdet_t *pwrdet = pi->pwrdet_ac;
	uint8 chan_freq_range, core;
	int16 tssifloor;

	PHY_CHANLOG(pi, __FUNCTION__, TS_ENTER, 0);
	if (ACREV_IS(pi->pubpi.phy_rev, 2)) {
		chan_freq_range = wlc_phy_get_chan_freq_range_acphy(pi, 0);

		FOREACH_CORE(pi, core) {
			tssifloor = (int16)pwrdet->tssifloor[core][chan_freq_range];
			if (tssifloor != 0) {
				wlc_phy_set_txpwr_clamp_acphy(pi);
			}
		}
	}
#ifdef POWPERCHANNL
	/* update the board - limits per channel if in 2G Band */
	wlc_phy_tx_target_pwr_per_channel_set_acphy(pi);
#endif /* POWPERCHANNL */
	wlapi_high_update_txppr_offset(pi->sh->physhim, pi->tx_power_offset);
	/* recalc targets -- turns hwpwrctrl off */

	if (SROMREV(pi->sh->sromrev) < 12) {
		wlc_phy_txpwrctrl_pwr_setup_acphy(pi);
	} else {
		wlc_phy_txpwrctrl_pwr_setup_srom12_acphy(pi);
	}

	/* restore power control */
	wlc_phy_txpwrctrl_enable_acphy(pi, pi->txpwrctrl);
	PHY_CHANLOG(pi, __FUNCTION__, TS_EXIT, 0);
}

static void
wlc_phy_txpwrctrl_setminpwr(phy_info_t *pi)
{
	if (ISACPHY(pi)) {
		if (ACMAJORREV_2(pi->pubpi.phy_rev)) {
			pi->min_txpower = PHY_TXPWR_MIN_ACPHY2X2;
		} else if (ACMAJORREV_1(pi->pubpi.phy_rev)) {
			if (PHY_IPA(pi)) {
				pi->min_txpower = PHY_TXPWR_MIN_ACPHY1X1IPA;
			} else {
				pi->min_txpower = PHY_TXPWR_MIN_ACPHY1X1EPA;
			}
		} else {
			pi->min_txpower = PHY_TXPWR_MIN_ACPHY;
		}
	}
}

int8
wlc_phy_txpwrctrl_update_minpwr_acphy(phy_info_t *pi)
{
	int8 mintxpwr;

	wlc_phy_txpwrctrl_setminpwr(pi);

#if defined(PHYCAL_CACHING)
	/* Update the min_txpower
	 * - equals OLPC threshold when OLPC is enabled
	 * - equals tssivisible threshold when OLPC is disabled
	 * - equals board minimum when tssivisible is not defined
	 */
	mintxpwr = wlc_phy_get_thresh_acphy(pi);
	if (mintxpwr == WL_RATE_DISABLED) {
		/* If tssivisible is not defined
		 * use min_txpower instead.
		 */
		mintxpwr = pi->min_txpower * WLC_TXPWR_DB_FACTOR;
	}
	else {
		/* Update min_txpower to OLPC thresh/tssivisible */
		pi->min_txpower = mintxpwr / WLC_TXPWR_DB_FACTOR;
	}
#else
	mintxpwr = pi->min_txpower * WLC_TXPWR_DB_FACTOR;
#endif /* PHYCAL_CACHING */
	return mintxpwr;
}

#ifdef POWPERCHANNL
void
BCMATTACHFN(wlc_phy_tx_target_pwr_per_channel_limit_acphy)(phy_info_t *pi)
{ /* Limit the max and min offset values */
	srom11_pwrdet_t *pwrdet = pi->pwrdet_ac;
	uint8 core, ch_ind;
	FOREACH_CORE(pi, core) {
		for (ch_ind = 0; ch_ind < CH20MHz_NUM_2G; ch_ind++) {
			if (pwrdet->PwrOffsets2GNormTemp[core][ch_ind] >
				PWR_PER_CH_POS_OFFSET_LIMIT_QDBM) {
				pwrdet->PwrOffsets2GNormTemp[core][ch_ind] =
					PWR_PER_CH_POS_OFFSET_LIMIT_QDBM;
			} else if (pwrdet->PwrOffsets2GNormTemp[core][ch_ind] <
					-PWR_PER_CH_NEG_OFFSET_LIMIT_QDBM) {
				pwrdet->PwrOffsets2GNormTemp[core][ch_ind] =
					-PWR_PER_CH_NEG_OFFSET_LIMIT_QDBM;
			}
			if (pwrdet->PwrOffsets2GLowTemp[core][ch_ind] >
				PWR_PER_CH_POS_OFFSET_LIMIT_QDBM) {
				pwrdet->PwrOffsets2GLowTemp[core][ch_ind] =
					PWR_PER_CH_POS_OFFSET_LIMIT_QDBM;
			} else if (pwrdet->PwrOffsets2GLowTemp[core][ch_ind] <
					-PWR_PER_CH_NEG_OFFSET_LIMIT_QDBM) {
				pwrdet->PwrOffsets2GLowTemp[core][ch_ind] =
					-PWR_PER_CH_NEG_OFFSET_LIMIT_QDBM;
			}
			if (pwrdet->PwrOffsets2GHighTemp[core][ch_ind] >
					PWR_PER_CH_POS_OFFSET_LIMIT_QDBM) {
					pwrdet->PwrOffsets2GHighTemp[core][ch_ind] =
						PWR_PER_CH_POS_OFFSET_LIMIT_QDBM;
			} else if (pwrdet->PwrOffsets2GHighTemp[core][ch_ind] <
						-PWR_PER_CH_NEG_OFFSET_LIMIT_QDBM) {
					pwrdet->PwrOffsets2GHighTemp[core][ch_ind] =
						-PWR_PER_CH_NEG_OFFSET_LIMIT_QDBM;
			}
		}
	}
}
void
wlc_phy_tx_target_pwr_per_channel_decide_run_acphy(phy_info_t *pi)
{ /* Decide if should recaculate power per channel due to temp diff */
	srom11_pwrdet_t *pwrdet = pi->pwrdet_ac;
	uint8 ch = CHSPEC_CHANNEL(pi->radio_chanspec);
	int16 temp= pi->u.pi_acphy->current_temperature; /* previous temprature */

	if (ch > CH20MHz_NUM_2G) /* 2 GHz channels only */
		return;
	if ((temp == 255) ){ /* This value is invalid - do not decide based on temp */
		return;
	}

	/* Check if temprature measurment is in a diffrenent temprature zone, */
	/*	with margin, than the Target power settings */
	if (((temp < pwrdet->Low2NormTemp - PWR_PER_CH_TEMP_MIN_STEP) &&
		(pwrdet->CurrentTempZone != PWR_PER_CH_LOW_TEMP)) ||
		((temp > pwrdet->High2NormTemp + PWR_PER_CH_TEMP_MIN_STEP) &&
		(pwrdet->CurrentTempZone != PWR_PER_CH_HIGH_TEMP)) ||
		((temp > pwrdet->Low2NormTemp + PWR_PER_CH_TEMP_MIN_STEP) &&
		(temp < pwrdet->High2NormTemp - PWR_PER_CH_TEMP_MIN_STEP) &&
		(pwrdet->CurrentTempZone != PWR_PER_CH_NORM_TEMP))) {
			wlc_phy_txpower_recalc_target_acphy(pi);
		}
}

static void
wlc_phy_tx_target_pwr_per_channel_set_acphy(phy_info_t *pi)
{
	srom11_pwrdet_t *pwrdet = pi->pwrdet_ac;
	uint8 core, ch_ind = CHSPEC_CHANNEL(pi->radio_chanspec)-1;
	int16 temp= pi->u.pi_acphy->current_temperature; /* Copy temprture without measuring */

	ASSERT(pi->sh->sromrev >= 11);

	if (ch_ind >= CH20MHz_NUM_2G)
		return;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	FOREACH_CORE(pi, core) {
		if ((pwrdet->Low2NormTemp != 0xff) && (temp < pwrdet->Low2NormTemp) &&
			(temp != 255)) {
			pwrdet->max_pwr[core][0] = pwrdet->max_pwr_SROM2G[core] +
				pwrdet->PwrOffsets2GLowTemp[core][ch_ind];
			pwrdet->CurrentTempZone = PWR_PER_CH_LOW_TEMP;
		} else if ((pwrdet->High2NormTemp != 0xff) && (temp > pwrdet->High2NormTemp) &&
			(temp != 255)) {
			pwrdet->max_pwr[core][0] = pwrdet->max_pwr_SROM2G[core] +
				pwrdet->PwrOffsets2GHighTemp[core][ch_ind];
			pwrdet->CurrentTempZone = PWR_PER_CH_HIGH_TEMP;
		} else {
			pwrdet->max_pwr[core][0] = pwrdet->max_pwr_SROM2G[core] +
				pwrdet->PwrOffsets2GNormTemp[core][ch_ind];
			pwrdet->CurrentTempZone = PWR_PER_CH_NORM_TEMP;
		}
		PHY_TXPWR(("wl%d: %s: core = %d  ChannelInd=%d Temprature=%d, ",
			pi->sh->unit, __FUNCTION__,
			core, ch_ind, temp));
		PHY_TXPWR(("Ch max pwr=%d, 2G max pwr =%d, Offset = %d \n",
			pwrdet->max_pwr[core][0],
			pwrdet->max_pwr_SROM2G[core],
			pwrdet->PwrOffsets2GNormTemp[core][ch_ind]));
	}
}
#endif /* POWPERCHANNL */

/* ********************************************* */
/*				External Definitions					*/
/* ********************************************* */

uint8
wlc_phy_tssi2dbm_acphy(phy_info_t *pi, int32 tssi, int32 a1, int32 b0, int32 b1)
{
		int32 num, den;
		int8 pwrest = 0;
		uint8 core;

		FOREACH_ACTV_CORE(pi, pi->sh->phyrxchain, core) {
			num = 8*(16*b0+b1*tssi);
			den = 32768+a1*tssi;
			if (pi->iboard) {
				pwrest = MAX(((4*num+den/2)/den), -60);
				pwrest = MIN(pwrest, 0x28);
			} else {
				pwrest = MAX(((4*num+den/2)/den), -8);
				pwrest = MIN(pwrest, 0x7f);
			}
		}
		return pwrest;
}

void
wlc_phy_read_txgain_acphy(phy_info_t *pi)
{
	uint8 core;
	uint8 stall_val;
	txgain_setting_t txcal_txgain[4];

	stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
	ACPHY_DISABLE_STALL(pi);

	FOREACH_ACTV_CORE(pi, pi->sh->phyrxchain, core) {
		/* store off orig tx radio gain */
		if (core < 3) {
			wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, (0x100 + core), 16,
				&(txcal_txgain[core].rad_gain));
			wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, (0x103 + core), 16,
				&(txcal_txgain[core].rad_gain_mi));
			wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, (0x106 + core), 16,
				&(txcal_txgain[core].rad_gain_hi));
		} else {
			wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x501, 16,
				&(txcal_txgain[core].rad_gain));
			wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x504, 16,
				&(txcal_txgain[core].rad_gain_mi));
			wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x507, 16,
				&(txcal_txgain[core].rad_gain_hi));
		}
		wlc_phy_get_tx_bbmult_acphy(pi, &(txcal_txgain[core].bbmult),  core);
		PHY_NONE(("\n radio gain = 0x%x%x%x, bbm=%d  \n",
			txcal_txgain[core].rad_gain_hi,
			txcal_txgain[core].rad_gain_mi,
			txcal_txgain[core].rad_gain,
			txcal_txgain[core].bbmult));
	}
	ACPHY_ENABLE_STALL(pi, stall_val);
}

void
wlc_phy_txpwr_by_index_acphy(phy_info_t *pi, uint8 core_mask, int8 txpwrindex)
{
	uint8 core;
	uint8 stall_val;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	(void) wlc_phy_set_txpwr_by_index_acphy(pi, core_mask, txpwrindex);

	stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
	ACPHY_DISABLE_STALL(pi);

	/* Set tx power based on an input "index"
	 * (Emulate what HW power control would use for a given table index)
	 */
	FOREACH_ACTV_CORE(pi, core_mask, core) {

		/* Check txprindex >= 0 */
		if (txpwrindex < 0)
			ASSERT(0); /* negative index not supported */

		if (PHY_PAPDEN(pi)) {
			if ((pi->acphy_txpwrctrl == PHY_TPC_HW_OFF) ||
				(TINY_RADIO(pi)))  {
				int16 rfPwrLutVal;
				uint8 rfpwrlut_table_ids[] = { ACPHY_TBL_ID_RFPWRLUTS0,
					ACPHY_TBL_ID_RFPWRLUTS1, ACPHY_TBL_ID_RFPWRLUTS2};

				if (!TINY_RADIO(pi)) {
					MOD_PHYREGCEE(pi, EpsilonTableAdjust, core, epsilonOffset,
					0);
				}
				MOD_PHYREGCEE(pi, PapdEnable, core, gain_dac_rf_override,
					1);
				wlc_phy_table_read_acphy(pi, rfpwrlut_table_ids[core],
					1, txpwrindex, 16, &rfPwrLutVal);
				MOD_PHYREGCEE(pi, PapdEnable, core, gain_dac_rf_reg,
					rfPwrLutVal);
			}
		}

		/* Update the per-core state of power index */
		pi->u.pi_acphy->txpwrindex[core] = txpwrindex;
	}
	ACPHY_ENABLE_STALL(pi, stall_val);
}

void
wlc_phy_get_txgain_settings_by_index_acphy(phy_info_t *pi, txgain_setting_t *txgain_settings,
                                     int8 txpwrindex)
{
	uint16 txgain[3];

	wlc_phy_table_read_acphy(pi, wlc_phy_get_tbl_id_gainctrlbbmultluts(pi, 0), 1,
		txpwrindex, 48, &txgain);

	txgain_settings->rad_gain    = ((txgain[0] >> 8) & 0xff) + ((txgain[1] & 0xff) << 8);
	txgain_settings->rad_gain_mi = ((txgain[1] >> 8) & 0xff) + ((txgain[2] & 0xff) << 8);
	txgain_settings->rad_gain_hi = ((txgain[2] >> 8) & 0xff);
	txgain_settings->bbmult      = (txgain[0] & 0xff);
}

void
wlc_phy_get_tx_bbmult_acphy(phy_info_t *pi, uint16 *bb_mult, uint16 core)
{
	uint16 tbl_ofdm_offset[] = { 99, 103, 107, 111};
	uint8 iqlocal_tbl_id = wlc_phy_get_tbl_id_iqlocal(pi, core);

	if (ACMAJORREV_4(pi->pubpi.phy_rev)) {
		core = 0;
	}

	wlc_phy_table_read_acphy(pi, iqlocal_tbl_id, 1,
	                         tbl_ofdm_offset[core], 16, bb_mult);
}

void
wlc_phy_set_tx_bbmult_acphy(phy_info_t *pi, uint16 *bb_mult, uint16 core)
{
	uint16 tbl_ofdm_offset[] = { 99, 103, 107, 111};
	uint16 tbl_bphy_offset[] = {115, 119, 123, 127};
	uint8 stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
	uint8 iqlocal_tbl_id = wlc_phy_get_tbl_id_iqlocal(pi, core);

	ACPHY_DISABLE_STALL(pi);

	if (ACMAJORREV_4(pi->pubpi.phy_rev)) {
		core = 0;
	}

	wlc_phy_table_write_acphy(pi, iqlocal_tbl_id, 1,
	                          tbl_ofdm_offset[core], 16, bb_mult);
	wlc_phy_table_write_acphy(pi, iqlocal_tbl_id, 1,
	                          tbl_bphy_offset[core], 16, bb_mult);
	ACPHY_ENABLE_STALL(pi, stall_val);
}

uint32
wlc_phy_txpwr_idx_get_acphy(phy_info_t *pi)
{
	uint8 core;
	uint32 pwr_idx[] = {0, 0, 0, 0};
	uint32 tmp = 0;

	if (wlc_phy_txpwrctrl_ison_acphy(pi)) {
		FOREACH_ACTV_CORE(pi, pi->sh->hw_phyrxchain, core) {
			pwr_idx[core] = READ_PHYREGFLDCE(pi, TxPwrCtrlStatus_path, core, baseIndex);
		}
	} else {
		FOREACH_ACTV_CORE(pi, pi->sh->hw_phyrxchain, core) {
			pwr_idx[core] = (pi->u.pi_acphy->txpwrindex[core] & 0xff);
		}
	}
	tmp = (pwr_idx[3] << 24) | (pwr_idx[2] << 16) | (pwr_idx[1] << 8) | pwr_idx[0];

	return tmp;
}

void
wlc_phy_txpwrctrl_enable_acphy(phy_info_t *pi, uint8 ctrl_type)
{
	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;
	uint16 mask;
	uint8 core;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	/* check for recognized commands */
	switch (ctrl_type) {
	case PHY_TPC_HW_OFF:
	case PHY_TPC_HW_ON:
		pi->txpwrctrl = ctrl_type;
		break;
	default:
		PHY_ERROR(("wl%d: %s: Unrecognized ctrl_type: %d\n",
			pi->sh->unit, __FUNCTION__, ctrl_type));
		break;
	}

	if (ctrl_type == PHY_TPC_HW_OFF) {
		/* save previous txpwr index if txpwrctl was enabled */
		if (wlc_phy_txpwrctrl_ison_acphy(pi)) {
			FOREACH_ACTV_CORE(pi, pi->sh->hw_phyrxchain, core) {
				pi_ac->txpwrindex_hw_save[core] =
					READ_PHYREGFLDCE(pi, TxPwrCtrlStatus_path, core, baseIndex);
				PHY_TXPWR(("wl%d: %s PWRCTRL: %d Cache Baseindex: %d Core: %d\n",
					pi->sh->unit, __FUNCTION__, ctrl_type,
					pi_ac->txpwrindex_hw_save[core], core));
			}
		}

		/* Disable hw txpwrctrl */
		mask = ACPHY_TxPwrCtrlCmd_txPwrCtrl_en_MASK(pi->pubpi.phy_rev) |
		       ACPHY_TxPwrCtrlCmd_hwtxPwrCtrl_en_MASK(pi->pubpi.phy_rev) |
		       ACPHY_TxPwrCtrlCmd_use_txPwrCtrlCoefs_MASK(pi->pubpi.phy_rev);
		_PHY_REG_MOD(pi, ACPHY_TxPwrCtrlCmd(pi->pubpi.phy_rev), mask, 0);

	} else {
		/* XXX FIXME: measure idle tssi when txpwrctrl is enabled
		   to be removed when txpwrctrl is turned on by default
		*/
		/* Setup the loopback path here */
		phy_ac_tssi_loopback_path_setup(pi, LOOPBACK_FOR_TSSICAL);
		if (!pi->iboard) {
			/* Enable hw txpwrctrl */
			mask = ACPHY_TxPwrCtrlCmd_txPwrCtrl_en_MASK(pi->pubpi.phy_rev) |
				ACPHY_TxPwrCtrlCmd_hwtxPwrCtrl_en_MASK(pi->pubpi.phy_rev) |
				ACPHY_TxPwrCtrlCmd_use_txPwrCtrlCoefs_MASK(pi->pubpi.phy_rev);
			_PHY_REG_MOD(pi, ACPHY_TxPwrCtrlCmd(pi->pubpi.phy_rev), mask, mask);
			FOREACH_ACTV_CORE(pi, pi->sh->hw_phyrxchain, core) {
				if (pi_ac->txpwrindex_hw_save[core] != 128) {
					MOD_PHYREGCEE(pi, TxPwrCtrlInit_path, core,
					pwrIndex_init_path, pi_ac->txpwrindex_hw_save[core]);
				PHY_TXPWR(("wl%d:%s PWRCTRL:%d Restore Baseindex:%d Core:%d\n",
					pi->sh->unit, __FUNCTION__, ctrl_type,
					pi_ac->txpwrindex_hw_save[core], core));
				}
			}
		}
		if (!TINY_RADIO(pi)) {
			FOREACH_ACTV_CORE(pi, pi->sh->hw_phyrxchain, core) {
				MOD_PHYREGCEE(pi, PapdEnable, core, gain_dac_rf_override, 0);
			}
		}
	}

	if (TINY_RADIO(pi)) {
		uint16 txPwrCtrlCmd = READ_PHYREGFLD(pi, TxPwrCtrlCmd, hwtxPwrCtrl_en);
		/* set phyreg(PapdEnable$core.gain_dac_rf_override$core)
		 * [expr !$phyreg(TxPwrCtrlCmd.hwtxPwrCtrl_en)]
		 */
		FOREACH_ACTV_CORE(pi, pi->sh->hw_phyrxchain, core) {
			MOD_PHYREGCEE(pi, PapdEnable, core, gain_dac_rf_override,
				(txPwrCtrlCmd ? 0 : 1));
		}
	}
}

/* set txgain in case txpwrctrl is disabled (fixed power) */
void
wlc_phy_txpwr_fixpower_acphy(phy_info_t *pi)
{
	uint8 core;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	PHY_CHANLOG(pi, __FUNCTION__, TS_ENTER, 0);
	FOREACH_ACTV_CORE(pi, pi->sh->hw_phyrxchain, core) {
		wlc_phy_txpwr_by_index_acphy(pi, (1 << core), pi->u.pi_acphy->txpwrindex[core]);
	}
	PHY_CHANLOG(pi, __FUNCTION__, TS_EXIT, 0);
}

void
wlc_phy_txpower_sromlimit_get_acphy(phy_info_t *pi, chanspec_t chanspec,
                                        ppr_t *max_pwr, uint8 core)
{
	uint8 band = 0, band_srom = 0;
	uint8 tmp_max_pwr = 0;
	int8 deltaPwr = 0;
	srom11_pwrdet_t *pwrdet11 = pi->pwrdet_ac;

	ASSERT(core < PHY_CORE_MAX);
	ASSERT((pi->sromi->subband5Gver == PHY_SUBBAND_4BAND) ||
	       (pi->sromi->subband5Gver == PHY_MAXNUM_5GSUBBANDS));

	/* to figure out which subband is in 5G */
	/* in the range of 0, 1, 2, 3 */
	band = wlc_phy_get_chan_freq_range_acphy(pi, chanspec);

	tmp_max_pwr = pwrdet11->max_pwr[0][band];
	if (PHYCORENUM(pi->pubpi.phy_corenum) > 1)
	    tmp_max_pwr = MIN(tmp_max_pwr, pwrdet11->max_pwr[1][band]);
	if (PHYCORENUM(pi->pubpi.phy_corenum) > 2)
	    tmp_max_pwr = MIN(tmp_max_pwr, pwrdet11->max_pwr[2][band]);

	/*	--------  in 5g_ext case  -----------
	 *	if 5170 <= freq < 5250, then band = 1;
	 *	if 5250 <= freq < 5500, then band = 2;
	 *	if 5500 <= freq < 5745, then band = 3;
	 *	if 5745 <= freq,		then band = 4;

	 *	--------  in 5g case  ---------------
	 *	if 5170 <= freq < 5500, then band = 1;
	 *	if 5500 <= freq < 5745, then band = 2;
	 *	if 5745 <= freq,		then band = 3;
	 */
	/*  -------- 4 subband to 3 subband mapping --------
	 *	subband#0 -> low
	 *	subband#1 -> mid
	 *	subband#2 -> high
	 *	subband#3 -> high
	 */

	if (band <= WL_CHAN_FREQ_RANGE_5G_BAND2)
	    band_srom = band;
	else
	    band_srom = band - 1;
	wlc_phy_txpwr_apply_srom11(pi, band_srom, chanspec, tmp_max_pwr, max_pwr);
	deltaPwr = pwrdet11->max_pwr[core][band] - tmp_max_pwr;

	if (deltaPwr > 0)
	    ppr_plus_cmn_val(max_pwr, deltaPwr);

	ppr_apply_max(max_pwr, pwrdet11->max_pwr[core][band]);
}

void
wlc_phy_txpower_sromlimit_get_srom12_acphy(phy_info_t *pi, chanspec_t chanspec,
                                        ppr_t *max_pwr, uint8 core)
{
	if (!(SROMREV(pi->sh->sromrev) < 12)) {
		uint8 band = 0;
		uint8 tmp_max_pwr = 0;
		uint8 chans[NUM_CHANS_IN_CHAN_BONDING];
		int8 deltaPwr = 0;
		srom12_pwrdet_t *pwrdet = pi->pwrdet_ac;

		ASSERT(core < PHY_CORE_MAX);
		if (CHSPEC_IS5G(pi->radio_chanspec)) {
			ASSERT(pi->sromi->subband5Gver == PHY_MAXNUM_5GSUBBANDS);
		}

		/* to figure out which subband is in 5G */
		/* in the range of 0, 1, 2, 3, 4, 5 */
		if (ACMAJORREV_33(pi->pubpi.phy_rev) && PHY_AS_80P80(pi, chanspec)) {
			wlc_phy_get_chan_freq_range_80p80_srom12_acphy(pi, chanspec, chans);
			band = (core <= 1) ? chans[0] : chans[1];
		} else {
			band = wlc_phy_get_chan_freq_range_srom12_acphy(pi, chanspec);
		}

		if (band >= WL_CHAN_FREQ_RANGE_2G_40) {
			if (CHSPEC_IS80(chanspec) || CHSPEC_IS8080(chanspec) ||
					CHSPEC_IS160(chanspec)) {
				band = band - WL_CHAN_FREQ_RANGE_5G_BAND0_80 + 1;
			} else {
				ASSERT(CHSPEC_IS40(chanspec));
				band = band - WL_CHAN_FREQ_RANGE_2G_40;
			}
		}

		tmp_max_pwr = pwrdet->max_pwr[0][band];
		if (PHYCORENUM(pi->pubpi.phy_corenum) > 1)
			tmp_max_pwr = MIN(tmp_max_pwr, pwrdet->max_pwr[1][band]);
		if (PHYCORENUM(pi->pubpi.phy_corenum) > 2)
			tmp_max_pwr = MIN(tmp_max_pwr, pwrdet->max_pwr[2][band]);
		if (PHYCORENUM(pi->pubpi.phy_corenum) > 3)
			tmp_max_pwr = MIN(tmp_max_pwr, pwrdet->max_pwr[3][band]);

		if (SROMREV(pi->sh->sromrev) < 13) {
			wlc_phy_txpwr_apply_srom11(pi, band, chanspec, tmp_max_pwr, max_pwr);
		}
#ifdef WL11AC
		else {
			wlc_phy_txpwr_apply_srom13(pi, band, chanspec, tmp_max_pwr, max_pwr);
		}
#endif // endif

		deltaPwr = pwrdet->max_pwr[core][band] - tmp_max_pwr;

		if (deltaPwr > 0)
			ppr_plus_cmn_val(max_pwr, deltaPwr);

		ppr_apply_max(max_pwr, pwrdet->max_pwr[core][band]);
	}
}

/* report estimated power and adjusted estimated power in quarter dBms */
void
wlc_phy_txpwr_est_pwr_acphy(phy_info_t *pi, uint8 *Pout, uint8 *Pout_adj)
{
	uint8 core;
	int8 val;

	FOREACH_ACTV_CORE(pi, pi->sh->hw_phyrxchain, core) {
		val = READ_PHYREGFLDCE(pi, EstPower_path, core, estPowerValid);

		/* Read the Actual Estimated Powers without adjustment */
		if (val) {
			Pout[core] = READ_PHYREGFLDCE(pi, EstPower_path, core, estPower);
		} else {
			Pout[core] = 0;
		}

		val = READ_PHYREGFLDCE(pi, TxPwrCtrlStatus_path, core, estPwrAdjValid);

		if (val) {
			Pout_adj[core] = READ_PHYREGFLDCE(pi, TxPwrCtrlStatus_path, core,
			                                  estPwr_adj);
		} else {
			Pout_adj[core] = 0;
		}
	}
}

uint16 *
wlc_phy_get_tx_pwrctrl_tbl_2069(phy_info_t *pi)
{
	uint16 *tx_pwrctrl_tbl = NULL;

	if (CHSPEC_IS2G(pi->radio_chanspec)) {

		tx_pwrctrl_tbl = acphy_txgain_epa_2g_2069rev0;

		if (PHY_IPA(pi)) {

			switch (RADIOREV(pi->pubpi.radiorev)) {
			case 16:
				tx_pwrctrl_tbl = acphy_txgain_ipa_2g_2069rev16;
				break;
			case 17:
				tx_pwrctrl_tbl = acphy_txgain_epa_2g_2069rev17;
				break;
			case 23: /* iPa  case TXGain tables */
				tx_pwrctrl_tbl = acphy_txgain_ipa_2g_2069rev17;
				break;
			case 25: /* iPa  case TXGain tables */
				tx_pwrctrl_tbl = acphy_txgain_ipa_2g_2069rev25;
				break;
			case 18:
			case 24:
			case 26:
				tx_pwrctrl_tbl = acphy_txgain_ipa_2g_2069rev18;
				break;
			case 32:
			case 33:
			case 34:
			case 35:
			case 37:
			case 38:
				tx_pwrctrl_tbl = acphy_txgain_ipa_2g_2069rev33_37;
				break;
			case 39:
			case 40:
				tx_pwrctrl_tbl = acphy_txgain_ipa_2g_2069rev39;
				break;
			default:
				tx_pwrctrl_tbl = acphy_txgain_ipa_2g_2069rev0;
				break;

			}
		} else {

			switch (RADIOREV(pi->pubpi.radiorev)) {
			case 17:
			case 23:
			case 25:
				tx_pwrctrl_tbl = acphy_txgain_epa_2g_2069rev17;
				if (BF3_2GTXGAINTBL_BLANK(pi->u.pi_acphy)) {
					wlc_phy_gaintbl_blanking(pi, tx_pwrctrl_tbl,
					                         pi->sromi->txidxcap2g);
				}
				break;
			case 18:
			case 24:
			case 26:
				tx_pwrctrl_tbl = acphy_txgain_epa_2g_2069rev18;
				break;
			case 4:
			case 8:
			case 7: /* 43602a0 uses radio rev4 tx pwr ctrl tables */
				switch (BF3_TXGAINTBLID(pi->u.pi_acphy)) {
				case 0:
					tx_pwrctrl_tbl = acphy_txgain_epa_2g_2069rev4;
					break;
				case 1:
					tx_pwrctrl_tbl = acphy_txgain_epa_2g_2069rev4_id1;
					break;
				default:
					tx_pwrctrl_tbl = acphy_txgain_epa_2g_2069rev4;
					break;
				}
				break;
			case 16:
				tx_pwrctrl_tbl = acphy_txgain_epa_2g_2069rev16;
				break;
			case 32:
			case 33:
				tx_pwrctrl_tbl = acphy_txgain_epa_2g_2069rev33_35_36_37;
				break;
			case 34:
			case 36:
				tx_pwrctrl_tbl = acphy_txgain_epa_2g_2069rev34;
				break;
			case 35:
			case 37:
			case 38:
			case 39:
				tx_pwrctrl_tbl = acphy_txgain_epa_2g_2069rev33_35_36_37;
				break;
			default:
				tx_pwrctrl_tbl = acphy_txgain_epa_2g_2069rev0;
				break;
			}
		}
	} else {
		if (PHY_IPA(pi)) {
			switch (RADIOREV(pi->pubpi.radiorev)) {
			case 17:
			case 23:
				tx_pwrctrl_tbl = acphy_txgain_ipa_5g_2069rev17;
				break;
			case 25:
				tx_pwrctrl_tbl = acphy_txgain_ipa_5g_2069rev25;
				break;
			case 18:
			case 24:
			case 26:
				tx_pwrctrl_tbl = acphy_txgain_ipa_5g_2069rev18;
				break;
			case 16:
				tx_pwrctrl_tbl = acphy_txgain_ipa_5g_2069rev16;
				break;
			case 32:
			case 33:
			case 34:
			case 35:
			case 37:
			case 38:
				tx_pwrctrl_tbl = acphy_txgain_ipa_5g_2069rev33_37;
				break;
			case 39:
			case 40:
				tx_pwrctrl_tbl = acphy_txgain_ipa_5g_2069rev39;
				break;
			default:
				tx_pwrctrl_tbl = acphy_txgain_ipa_5g_2069rev0;
				break;
			}
		} else {

			switch (RADIOREV(pi->pubpi.radiorev)) {
			case 17:
			case 23:
			case 25:
				tx_pwrctrl_tbl = acphy_txgain_epa_5g_2069rev17;
				if (BF3_5GTXGAINTBL_BLANK(pi->u.pi_acphy)) {
					wlc_phy_gaintbl_blanking(pi, tx_pwrctrl_tbl,
					                         pi->sromi->txidxcap5g);
				}
				break;
			case 18:
			case 24:
			case 26:
				tx_pwrctrl_tbl = acphy_txgain_epa_5g_2069rev18;
				break;
			case 4:
			case 8:
			case 7: /* 43602a0 uses radio rev4 tx pwr ctrl tables */
				tx_pwrctrl_tbl = acphy_txgain_epa_5g_2069rev4;
				break;
			case 16:
				tx_pwrctrl_tbl = acphy_txgain_epa_5g_2069rev16;
				break;
			case 32:
			case 33:
				tx_pwrctrl_tbl = acphy_txgain_epa_5g_2069rev33_35_36;
				break;
			case 34:
			case 36:
				tx_pwrctrl_tbl = acphy_txgain_epa_5g_2069rev34;
				break;
			case 35:
				tx_pwrctrl_tbl = acphy_txgain_epa_5g_2069rev33_35_36;
				break;
			case 37:
			case 38:
			case 39:
				tx_pwrctrl_tbl = acphy_txgain_epa_5g_2069rev37_38;
				break;
			default:
				tx_pwrctrl_tbl = acphy_txgain_epa_5g_2069rev0;
				break;

			}
		}
	}

	return tx_pwrctrl_tbl;
}

#ifdef PREASSOC_PWRCTRL
void
wlc_phy_store_tx_pwrctrl_setting_acphy(phy_info_t *pi, chanspec_t previous_channel)
{
	uint8 core, iidx;

	if (CHSPEC_IS5G(previous_channel)) {
		pi->u.pi_acphy->pwr_ctrl_save->last_chan_stored_5g = previous_channel;

	} else {
		pi->u.pi_acphy->pwr_ctrl_save->last_chan_stored_2g = previous_channel;

	}
	FOREACH_ACTV_CORE(pi, pi->sh->hw_phyrxchain, core) {
		iidx = READ_PHYREGFLDCE(pi, TxPwrCtrlStatus_path, core, baseIndex);
		if (CHSPEC_IS5G(previous_channel)) {
			pi->u.pi_acphy->pwr_ctrl_save->status_idx_5g[core] = iidx;
			pi->u.pi_acphy->pwr_ctrl_save->pwr_qdbm_5g[core] =
			        wlc_phy_txpwrctrl_get_target_acphy(pi, core);
			pi->u.pi_acphy->pwr_ctrl_save->stored_not_restored_5g[core] = TRUE;

		} else {
			pi->u.pi_acphy->pwr_ctrl_save->status_idx_2g[core] = iidx;
			pi->u.pi_acphy->pwr_ctrl_save->pwr_qdbm_2g[core] =
			        wlc_phy_txpwrctrl_get_target_acphy(pi, core);
			pi->u.pi_acphy->pwr_ctrl_save->stored_not_restored_5g[core] = TRUE;
		}

	}
}
#endif /* PREASSOC_PWRCTRL */

void
wlc_phy_txpwrctrl_set_target_acphy(phy_info_t *pi, uint8 pwr_qtrdbm, uint8 core)
{
	/* set target powers in 6.2 format (in dBs) */
	switch (core) {
	case 0:
		MOD_PHYREG(pi, TxPwrCtrlTargetPwr_path0, targetPwr0, pwr_qtrdbm);
		break;
	case 1:
		MOD_PHYREG(pi, TxPwrCtrlTargetPwr_path1, targetPwr1, pwr_qtrdbm);
		break;
	case 2:
		MOD_PHYREG(pi, TxPwrCtrlTargetPwr_path2, targetPwr2, pwr_qtrdbm);
		break;
	case 3:
		MOD_PHYREG(pi, TxPwrCtrlTargetPwr_path3, targetPwr3, pwr_qtrdbm);
		break;
	}
}

void
BCMATTACHFN(wlc_phy_txpwrctrl_config_acphy)(phy_info_t *pi)
{
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	pi->hwpwrctrl_capable = TRUE;
	pi->txpwrctrl = PHY_TPC_HW_ON;
	pi->phy_5g_pwrgain = TRUE;
}

int
wlc_phy_txpower_core_offset_set_acphy(phy_info_t *pi, struct phy_txcore_pwr_offsets *offsets)
{
	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;
	int8 core_offset;
	int core;
	int offset_changed = FALSE;

	FOREACH_CORE(pi, core) {
		core_offset = offsets->offset[core];
		if (core_offset != 0 && core > pi->pubpi.phy_corenum) {
			return BCME_BADARG;
		}

		if (pi_ac->txpwr_offset[core] != core_offset) {
			offset_changed = TRUE;
			pi_ac->txpwr_offset[core] = core_offset;
		}
	}

	/* Apply the new per-core targets to the hw */
	if (pi->sh->clk && offset_changed) {
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		wlc_phy_txpower_recalc_target_acphy(pi);
		wlapi_enable_mac(pi->sh->physhim);
	}
	return BCME_OK;
}

int
wlc_phy_txpower_core_offset_get_acphy(phy_info_t *pi, struct phy_txcore_pwr_offsets *offsets)
{
	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;
	int core;

	memset(offsets, 0, sizeof(struct phy_txcore_pwr_offsets));

	FOREACH_CORE(pi, core) {
		offsets->offset[core] = pi_ac->txpwr_offset[core];
	}
	return BCME_OK;
}

#if defined(WL_SARLIMIT) || defined(BCM_OL_DEV) || defined(WL_SAR_SIMPLE_CONTROL)
void
wlc_phy_set_sarlimit_acphy(phy_info_t *pi)
{
	uint core;
	int16 txpwr_sarcap[3] = { 0, 0, 0 };

	ASSERT(pi->sh->clk);
	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	phy_utils_phyreg_enter(pi);
	FOREACH_ACTV_CORE(pi, pi->sh->phyrxchain, core) {
		txpwr_sarcap[core] = pi->sarlimit[core];
#ifdef WL_SARLIMIT
		if ((pi->u.pi_acphy->txpwr_offset[core] != 0) &&
		    !CHSPEC_IS5G(pi->radio_chanspec))
			txpwr_sarcap[core] = wlc_phy_calc_adjusted_cap_rgstr_acphy(pi, core);
#endif // endif
	}

	IF_ACTV_CORE(pi, pi->sh->phyrxchain, 0)
	        MOD_PHYREG(pi, TxPwrCapping_path0,
	                   maxTxPwrCap_path0, txpwr_sarcap[0]);
	IF_ACTV_CORE(pi, pi->sh->phyrxchain, 1)
	        MOD_PHYREG(pi, TxPwrCapping_path1,
	                   maxTxPwrCap_path1, txpwr_sarcap[1]);
	IF_ACTV_CORE(pi, pi->sh->phyrxchain, 2)
	        MOD_PHYREG(pi, TxPwrCapping_path2,
	                   maxTxPwrCap_path2, txpwr_sarcap[2]);
	phy_utils_phyreg_exit(pi);
	wlapi_enable_mac(pi->sh->physhim);
}
#endif /* WL_SARLIMIT || BCM_OL_DEV || WL_SAR_SIMPLE_CONTROL */

#if defined(WLTEST)
void
wlc_phy_iovar_patrim_acphy(phy_info_t *pi, int32 *ret_int_ptr)
{
	if ((ACMAJORREV_2(pi->pubpi.phy_rev) || (ACMAJORREV_4(pi->pubpi.phy_rev))) &&
		BF3_TSSI_DIV_WAR(pi->u.pi_acphy)) {
		if (CHSPEC_IS5G(pi->radio_chanspec)) {
			if (CHSPEC_IS20(pi->radio_chanspec)) {
				*ret_int_ptr = 0x0;
			}
			else {
				*ret_int_ptr = 0x3;
			}
		}
		else {
			if (ACMAJORREV_4(pi->pubpi.phy_rev))
				*ret_int_ptr = 0x14;
			else
				*ret_int_ptr = 0x0;
		}
	} else if ((ACMAJORREV_1(pi->pubpi.phy_rev) || ACMAJORREV_3(pi->pubpi.phy_rev)) &&
		BF3_TSSI_DIV_WAR(pi->u.pi_acphy)) {
		if (CHSPEC_IS5G(pi->radio_chanspec)) {
			*ret_int_ptr = 0x21;
		}
		else
			*ret_int_ptr = 0x14;
	}
	else
		*ret_int_ptr = 0x0;

}
#endif // endif

void
wlc_phy_get_paparams_for_band_acphy(phy_info_t *pi, int16 *a1, int16 *b0, int16 *b1)
{

	srom11_pwrdet_t *pwrdet = pi->pwrdet_ac;
	uint8 chan_freq_range, core;

	/* Get pwrdet params from SROM for current subband */
	chan_freq_range = wlc_phy_get_chan_freq_range_acphy(pi, 0);

	FOREACH_CORE(pi, core) {
		switch (chan_freq_range) {
		case WL_CHAN_FREQ_RANGE_2G:
		case WL_CHAN_FREQ_RANGE_5G_BAND0:
		case WL_CHAN_FREQ_RANGE_5G_BAND1:
		case WL_CHAN_FREQ_RANGE_5G_BAND2:
		case WL_CHAN_FREQ_RANGE_5G_BAND3:
			*a1 =  (int16)pwrdet->pwrdet_a1[core][chan_freq_range];
			*b0 =  (int16)pwrdet->pwrdet_b0[core][chan_freq_range];
			*b1 =  (int16)pwrdet->pwrdet_b1[core][chan_freq_range];
			PHY_TXPWR(("wl%d: %s: pwrdet core%d: a1=%d b0=%d b1=%d\n",
				pi->sh->unit, __FUNCTION__, core,
				*a1, *b0, *b1));
			break;
		}
	}
}

#if defined(PREASSOC_PWRCTRL)
void
phy_ac_tpc_shortwindow_upd(phy_info_t *pi, bool new_channel)
{
	uint shmaddr;
	uint32 txallfrm_cnt, txallfrm_diff;

	if ((!pi->sh->up))
		return;

	shmaddr = MACSTAT_ADDR(MCSTOFF_TXFRAME);
	/* default is long term */
	pi->channel_short_window = FALSE;

	phy_utils_phyreg_enter(pi);
	if (new_channel) {
		wlc_phy_pwrctrl_shortwindow_upd_acphy(pi, TRUE);
		pi->channel_short_window = TRUE;
		pi->txallfrm_cnt_ref = wlapi_bmac_read_shm(pi->sh->physhim, shmaddr);
	} else {
		txallfrm_cnt = wlapi_bmac_read_shm(pi->sh->physhim, shmaddr);
		if (pi->txallfrm_cnt_ref > txallfrm_cnt) {
			pi->txallfrm_cnt_ref = 0;
		} else {
			txallfrm_diff = txallfrm_cnt - pi->txallfrm_cnt_ref;
			if (txallfrm_diff > NUM_FRAME_BEFORE_PWRCTRL_CHANGE) {
				wlc_phy_pwrctrl_shortwindow_upd_acphy(pi, FALSE);
				pi->channel_short_window = FALSE;

			} else {
				wlc_phy_pwrctrl_shortwindow_upd_acphy(pi, TRUE);
				pi->channel_short_window = TRUE;
			}
		}
	}
	phy_utils_phyreg_exit(pi);
}
#endif /* PREASSOC_PWRCTRL */

#endif /* (ACCONF != 0) || (ACCONF2 != 0) */
