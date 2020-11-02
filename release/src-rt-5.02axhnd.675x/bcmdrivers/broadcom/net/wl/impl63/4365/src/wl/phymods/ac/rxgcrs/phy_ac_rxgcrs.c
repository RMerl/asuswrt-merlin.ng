/*
 * ACPHY Rx Gain Control and Carrier Sense module implementation
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
#include <phy_dbg.h>
#include <phy_mem.h>
#include "phy_type_rxgcrs.h"
#include <phy_utils_var.h>
#include <phy_ac.h>
#include <phy_ac_rxgcrs.h>

/* ************************ */
/* Modules used by this module */
/* ************************ */
#include <wlc_radioreg_20691.h>
#include <wlc_radioreg_20693.h>
#include <wlc_phy_radio.h>
#include <wlc_phyreg_ac.h>

#include <phy_utils_reg.h>
#include <phy_ac_info.h>

/* module private states */
struct phy_ac_rxgcrs_info {
	phy_info_t *pi;
	phy_ac_info_t *aci;
	phy_rxgcrs_info_t *cmn_info;
	acphy_desense_values_t *lte_desense;
	bool thresh_noise_cal;	/* enable/disable additional entries in noise cal table */
};

static const char BCMATTACHDATA(rstr_thresh_noise_cal)[] = "thresh_noise_cal";

/* local functions */
static void phy_ac_rxgcrs_set_locale(phy_type_rxgcrs_ctx_t *ctx, uint8 region_group);

/* register phy type specific implementation */
phy_ac_rxgcrs_info_t *
BCMATTACHFN(phy_ac_rxgcrs_register_impl)(phy_info_t *pi, phy_ac_info_t *aci,
	phy_rxgcrs_info_t *cmn_info)
{
	phy_ac_rxgcrs_info_t *ac_info;
	acphy_desense_values_t *lte_desense = NULL;
	phy_type_rxgcrs_fns_t fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage together */
	if ((ac_info = phy_malloc(pi, sizeof(phy_ac_rxgcrs_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}

	if ((lte_desense = phy_malloc(pi, sizeof(acphy_desense_values_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc lte_desense failed\n", __FUNCTION__));
		goto fail;
	}

	ac_info->pi = pi;
	ac_info->aci = aci;
	ac_info->cmn_info = cmn_info;
	ac_info->lte_desense = lte_desense;
	ac_info->thresh_noise_cal = (bool)PHY_GETINTVAR_DEFAULT(pi, rstr_thresh_noise_cal, 1);

	/* register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.ctx = ac_info;
	fns.set_locale = phy_ac_rxgcrs_set_locale;

	if (phy_rxgcrs_register_impl(cmn_info, &fns) != BCME_OK) {
		PHY_ERROR(("%s: phy_rxgcrs_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	return ac_info;

	/* error handling */
fail:
	if (lte_desense != NULL)
		phy_mfree(pi, lte_desense, sizeof(acphy_desense_values_t));
	if (ac_info != NULL)
		phy_mfree(pi, ac_info, sizeof(phy_ac_rxgcrs_info_t));
	return NULL;
}

void
BCMATTACHFN(phy_ac_rxgcrs_unregister_impl)(phy_ac_rxgcrs_info_t *ac_info)
{
	phy_info_t *pi = ac_info->pi;
	phy_rxgcrs_info_t *cmn_info = ac_info->cmn_info;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* unregister from common */
	phy_rxgcrs_unregister_impl(cmn_info);

	phy_mfree(pi, ac_info->lte_desense, sizeof(acphy_desense_values_t));

	phy_mfree(pi, ac_info, sizeof(phy_ac_rxgcrs_info_t));
}

/* ********************************************* */
/*				Internal Definitions					*/
/* ********************************************* */
static const uint8 ac_lna1_2g[]       = {0xf6, 0xff, 0x6, 0xc, 0x12, 0x19};
static const uint8 ac_lna1_2g_tiny[]	= {0xfa, 0x00, 0x6, 0xc, 0x12, 0x18};
static const uint8 ac_tiny_g_lna_rout_map[] = {9, 9, 9, 9, 6, 0};
static const uint8 ac_tiny_g_lna_gain_map[] = {2, 3, 4, 5, 5, 5};
static const uint8 ac_tiny_a_lna_rout_map[] = {11, 11, 11, 11, 7, 0};
static const uint8 ac_tiny_a_lna_gain_map[] = {4, 5, 6, 7, 7, 7};
static const uint8 ac_4365_a_lna_rout_map[] = {9, 9, 9, 9, 6, 0};
static const uint8 ac_lna1_2g_ltecoex[]       = {0xf6, 0xff, 0x6, 0xc, 0x12, 0x16};
static const uint8 ac_lna1_5g_tiny[]	= {0xfb, 0x00, 0x6, 0xc, 0x12, 0x18};
static const uint8 ac_lna1_5g_4365[]	= {0xfe, 0x03, 0x9, 0x10, 0x14, 0x18};
static const uint8 ac_lna1_5g[] = {0xf9, 0xfe, 0x4, 0xa, 0x10, 0x17};
static const uint8 ac_lna1_2g_43352_ilna[]    = {0xff, 0xff, 0x6, 0xc, 0x12, 0x19};
static const uint8 ac_lna2_2g_ltecoex[] = {0xf4, 0xf8, 0xfc, 0xff, 0xff, 0x5, 0x9};
static const uint8 ac_lna2_2g_43352_ilna[] = {0xfa, 0xfa, 0xfe, 0x01, 0x4, 0x7, 0xb};
static const uint8 ac_lna2_5g[] = {0xf5, 0xf8, 0xfb, 0xfe, 0x2, 0x5, 0x9};
static const uint8 ac_lna2_2g_gm2[] = {0xf4, 0xf8, 0xfc, 0xff, 0x2, 0x5, 0x9};
static const uint8 ac_lna2_2g_gm3[] = {0xf6, 0xfa, 0xfe, 0x01, 0x4, 0x7, 0xb};
static const uint8 ac_lna2_tiny[] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
static const uint8 ac_lna2_tiny_4349[] = {0xEE, 0xf4, 0xfa, 0x0, 0x0, 0x0, 0x0};
static const uint8 ac_lna2_tiny_4365_2g[] = {0xf4, 0xfa, 0, 6, 6, 6, 6};
static const uint8 ac_lna2_tiny_4365_5g[] = {0xf1, 0xf7, 0xfd, 3, 3, 3, 3};
static const uint8 ac_lna2_tiny_ilna_dcc_comp[] = {0, 20, 16, 12, 8, 4, 0};
static const uint8 ac_lna1_rout_delta_2g[] = {0, 1, 2, 3, 5, 7, 8, 11, 13, 15, 18, 20};
static const uint8 ac_lna1_rout_delta_5g[] = {10, 7, 4, 2, 0};

#ifndef WLC_DISABLE_ACI
static void wlc_phy_desense_mf_high_thresh_acphy(phy_info_t *pi, bool on);
static void wlc_phy_desense_print_phyregs_acphy(phy_info_t *pi, const char str[]);
#endif // endif

static int8 wlc_phy_rxgainctrl_calc_low_sens_acphy(phy_info_t *pi, int8 clipgain,
                                                   bool trtx, bool lna1byp, uint8 core);
static void wlc_phy_set_analog_rxgain(phy_info_t *pi, uint8 clipgain,
                                      uint8 *gain_idx, bool trtx, bool lna1byp, uint8 core);
static int8 wlc_phy_rxgainctrl_calc_high_sens_acphy(phy_info_t *pi, int8 clipgain,
                                                    bool trtx, bool lna1byp, uint8 core);
static uint8 wlc_phy_rxgainctrl_set_init_clip_gain_acphy(phy_info_t *pi, uint8 clipgain,
	int8 gain_dB, bool trtx, bool lna1byp, uint8 core);
static void wlc_phy_rxgainctrl_nbclip_acphy(phy_info_t *pi, uint8 core,
	int8 rxpwr_dBm);
static void wlc_phy_rxgainctrl_w1clip_acphy(phy_info_t *pi, uint8 core,
	int8 rxpwr_dBm);
#ifndef WLC_DISABLE_ACI
static void wlc_phy_set_crs_min_pwr_higain_acphy(phy_info_t *pi, uint8 thresh);
#endif // endif
static void wlc_phy_limit_rxgaintbl_acphy(uint8 gaintbl[], uint8 gainbitstbl[], uint8 sz,
	const uint8 default_gaintbl[], uint8 min_idx, uint8 max_idx);
static void wlc_phy_rxgainctrl_nbclip_acphy_tiny(phy_info_t *pi, uint8 core, int8 rxpwr_dBm);
#define ACPHY_NUM_NB_THRESH 8
#define ACPHY_NUM_NB_THRESH_TINY 9
#define ACPHY_NUM_W1_THRESH 12

#ifndef WLC_DISABLE_ACI

/*
Default - High MF thresholds are used only if pktgain < 81dB.
To always use high mf thresholds, change this to 98dBs
*/
static void
wlc_phy_desense_mf_high_thresh_acphy(phy_info_t *pi, bool on)
{
	uint16 val;

	if (on) {
		val = 0x5f62;
	} else {
		val = (TINY_RADIO(pi)) ? 0x454b : 0x4e51;
	}

	if (ACREV_GE(pi->pubpi.phy_rev, 32)) {
		/* Use default values for now */
	} else {
		WRITE_PHYREG(pi, crshighlowpowThresholdl, val);
		WRITE_PHYREG(pi, crshighlowpowThresholdu, val);
		WRITE_PHYREG(pi, crshighlowpowThresholdlSub1, val);
		WRITE_PHYREG(pi, crshighlowpowThresholduSub1, val);
	}
}

static void
wlc_phy_desense_print_phyregs_acphy(phy_info_t *pi, const char str[])
{
}

#endif /* #ifndef WLC_DISABLE_ACI */

static int8
wlc_phy_rxgainctrl_calc_low_sens_acphy(phy_info_t *pi, int8 clipgain, bool trtx,
                                       bool lna1byp, uint8 core)
{
	int sens, sens_bw_c9[] = {-66, -63, -60};   /* c9s1 1% sensitivity for 20/40/80 mhz */
	int sens_bw_c11[]= {-63, -60, -57};   /* c11s1_ldpc 1% sensitivity for 20/40/80 mhz */
	uint8 low_sen_adjust[] = {25, 22, 19};   /* low_end_sens = -clip_gain - low_sen_adjust */
	uint8 bw_idx, elna_idx, trloss, elna_bypass_tr;
	int8 elna, detection, demod;
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;
	acphy_desense_values_t *desense = pi_ac->total_desense;
	uint8 idx, max_lna2;
	int8 extra_loss = 0;

	ASSERT(core < PHY_CORE_MAX);
	bw_idx = CHSPEC_IS20(pi->radio_chanspec) ? 0 : CHSPEC_IS40(pi->radio_chanspec) ? 1 : 2;
	if (ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev))
		sens =  sens_bw_c11[bw_idx];
	else
		sens =  sens_bw_c9[bw_idx];

	if (lna1byp) {
		sens += 7;  /* djain - HACK, analyze this more */
	}

	elna_idx = READ_PHYREGFLDC(pi, InitGainCodeA, core, initExtLnaIndex);
	elna = pi_ac->rxgainctrl_params[core].gaintbl[0][elna_idx];
	trloss = pi_ac->fem_rxgains[core].trloss;
	elna_bypass_tr = pi_ac->fem_rxgains[core].elna_bypass_tr;

	if (TINY_RADIO(pi))
		detection = -6 - (clipgain + low_sen_adjust[bw_idx]);
	else
		detection = 0 - (clipgain + low_sen_adjust[bw_idx]);

	demod = trtx ? (sens + trloss - (elna_bypass_tr * elna)) : sens;
	demod += desense->nf_hit_lna12;

	if (ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev)) {
		/* sens is worse if lower lna2 gains are used */
		if (trtx) {
			idx = pi_ac->rxgainctrl_stage_len[LNA2_ID] - 1;
			max_lna2 = pi_ac->rxgainctrl_params[core].gainbitstbl[LNA2_ID][idx];
			extra_loss = 3 * (3 - max_lna2);
			demod += MAX(0, extra_loss);
		}
	}

	return MAX(detection, demod);
}

static void
wlc_phy_set_analog_rxgain(phy_info_t *pi, uint8 clipgain, uint8 *gain_idx, bool trtx,
                          bool lna1byp, uint8 core)
{
	uint8 lna1, lna2, mix, bq0, bq1, tx, rx, dvga;
	uint16 gaincodeA, gaincodeB, final_gain;

	ASSERT(core < PHY_CORE_MAX);

	lna1 = gain_idx[1];
	lna2 = gain_idx[2];
	mix = gain_idx[3];
	bq0 = (TINY_RADIO(pi)) ? 0 : gain_idx[4];
	bq1 = gain_idx[5];
	dvga = (TINY_RADIO(pi)) ? gain_idx[6] : 0;

	if (lna1byp) trtx = 0;  /* force in rx mode is using lna1byp */
	tx = trtx;
	rx = !trtx;

	gaincodeA = ((mix << 7) | (lna2 << 4) | (lna1 << 1));
	gaincodeB = (dvga<<12) | (bq1 << 8) | (bq0 << 4) | (tx << 3) | (rx << 2) | (lna1byp << 1);

	if (clipgain == 0) {
		WRITE_PHYREGC(pi, InitGainCodeA, core, gaincodeA);
		WRITE_PHYREGC(pi, InitGainCodeB, core, gaincodeB);
		final_gain = ((bq1 << 13) | (bq0 << 10) | (mix << 6) | (lna2 << 3) | (lna1 << 0));
		if (core == 3)
		    wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x509, 16,
		                          &final_gain);
		else
		    wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, (0xf9 + core), 16,
		                          &final_gain);
		if (TINY_RADIO(pi)) {
			uint8 gmrout;
			uint16 rfseq_init_aux;
			uint8 offset = ACPHY_LNAROUT_BAND_OFFSET(pi,
				pi->radio_chanspec)	+ lna1 +
				ACPHY_LNAROUT_CORE_RD_OFST(pi, core);

			wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_LNAROUT, 1, offset, 8, &gmrout);

			rfseq_init_aux = (((0xf & (gmrout >> 3)) << 4) | dvga);
			if (ACMAJORREV_4(pi->pubpi.phy_rev) || ACMAJORREV_32(pi->pubpi.phy_rev) ||
				ACMAJORREV_33(pi->pubpi.phy_rev)) {
				if (core == 3)
					wlc_phy_table_write_acphy(
						pi, ACPHY_TBL_ID_RFSEQ, 1, 0x506, 16,
						&rfseq_init_aux);
				else
					wlc_phy_table_write_acphy(
						pi, ACPHY_TBL_ID_RFSEQ, 1, (0xf6 + core),
						16, &rfseq_init_aux);
			}
			/* elna index always zero, not required */
		}
	} else if (clipgain == 1) {
		WRITE_PHYREGC(pi, clipHiGainCodeA, core, gaincodeA);
		WRITE_PHYREGC(pi, clipHiGainCodeB, core, gaincodeB);
	} else if (clipgain == 2) {
		WRITE_PHYREGC(pi, clipmdGainCodeA, core, gaincodeA);
		WRITE_PHYREGC(pi, clipmdGainCodeB, core, gaincodeB);
	} else if (clipgain == 3) {
		WRITE_PHYREGC(pi, cliploGainCodeA, core, gaincodeA);
		WRITE_PHYREGC(pi, cliploGainCodeB, core, gaincodeB);
	} else if (clipgain == 4) {
		WRITE_PHYREGC(pi, clip2GainCodeA, core, gaincodeA);
		WRITE_PHYREGC(pi, clip2GainCodeB, core, gaincodeB);
	}
}

static int8
wlc_phy_rxgainctrl_calc_high_sens_acphy(phy_info_t *pi, int8 clipgain, bool trtx,
                                        bool lna1byp, uint8 core)
{
	uint8 high_sen_adjust = 23;  /* high_end_sens = high_sen_adjust - clip_gain */
	uint8 elna_idx, trloss;
	int8 elna, saturation, clipped, lna1_sat;
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;

	if (TINY_RADIO(pi)) {
		if (ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev)) {
			lna1_sat = CHSPEC_IS2G(pi->radio_chanspec) ? -12 : -10;
			high_sen_adjust = 20;
		} else {
			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				lna1_sat = -10;
				high_sen_adjust = 17;
			} else {
				lna1_sat = -2;
				high_sen_adjust = 19;
			}
		}
	} else {
		lna1_sat = -16;
		high_sen_adjust = 23;
	}

	if (lna1byp)
		lna1_sat += CHSPEC_IS2G(pi->radio_chanspec) ? 10 : 7;

	ASSERT(core < PHY_CORE_MAX);
	elna_idx = READ_PHYREGFLDC(pi, InitGainCodeA, core, initExtLnaIndex);
	elna = pi_ac->rxgainctrl_params[core].gaintbl[0][elna_idx];
	trloss = pi_ac->fem_rxgains[core].trloss;

	/* c9 needs lna1 input to be below -lna1_sat */
	saturation = trtx ? (trloss + lna1_sat - elna) : 0 + lna1_sat - elna;
	clipped = high_sen_adjust - clipgain;

	return MIN(saturation, clipped);
}

/* Wrapper to call encode_gain & set init/clip gains */
static uint8
wlc_phy_rxgainctrl_set_init_clip_gain_acphy(phy_info_t *pi, uint8 clipgain, int8 gain_dB,
	bool trtx, bool lna1byp, uint8 core)
{
	uint8 gain_idx[ACPHY_MAX_RX_GAIN_STAGES];
	uint8 gain_applied;

	gain_applied = wlc_phy_rxgainctrl_encode_gain_acphy(pi, clipgain, core, gain_dB, trtx,
	                                                    lna1byp, gain_idx);
	wlc_phy_set_analog_rxgain(pi, clipgain, gain_idx, trtx, lna1byp, core);

	return gain_applied;
}

static void
wlc_phy_max_anagain_to_initgain_lesi_acphy(phy_info_t *pi, uint8 i, uint8 core, uint8 gidx)
{
	uint8 l, offset = 0, gainbits[15];
	int8 gains[15];
	uint16 gainbits_tblid, gains_tblid;
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;
	uint8 gainbit_tbl_entry_size = sizeof(pi_ac->rxgainctrl_params[0].gainbitstbl[0][0]);
	uint8 gain_tbl_entry_size = sizeof(pi_ac->rxgainctrl_params[0].gaintbl[0][0]);

	if (i == 1) {
		offset = 8;
	} else if (i == 2) {
		offset = 16;
	} else if (i == 3) {
		offset = 32;
	} else {
		ASSERT(0);
	}

	/* gainBits */
	for (l = 0; l < pi_ac->rxgainctrl_stage_len[i]; l++)
		gainbits[l] = (l > gidx) ? gidx : pi_ac->rxgainctrl_params[core].gainbitstbl[i][l];
	gainbits_tblid = (core == 0) ? ACPHY_TBL_ID_GAINBITS0 : (core == 1) ?
	        ACPHY_TBL_ID_GAINBITS1 : (core == 2) ?
	        ACPHY_TBL_ID_GAINBITS2 : ACPHY_TBL_ID_GAINBITS3;
	wlc_phy_table_write_acphy(
		pi, gainbits_tblid, pi_ac->rxgainctrl_stage_len[i],
		offset, ACPHY_GAINBITS_TBL_WIDTH, gainbits);
	memcpy(pi_ac->rxgainctrl_params[core].gainbitstbl[i], gainbits,
	       gainbit_tbl_entry_size * pi_ac->rxgainctrl_stage_len[i]);

	/* gaintbl */
	for (l = 0; l < pi_ac->rxgainctrl_stage_len[i]; l++)
		gains[l] = (l > gidx) ?
		        pi_ac->rxgainctrl_params[core].gaintbl[i][gidx] :
		        pi_ac->rxgainctrl_params[core].gaintbl[i][l];
	gains_tblid = (core == 0) ? ACPHY_TBL_ID_GAIN0 : (core == 1) ?
	        ACPHY_TBL_ID_GAIN1 : (core == 2) ?
	        ACPHY_TBL_ID_GAIN2 : ACPHY_TBL_ID_GAIN3;
	wlc_phy_table_write_acphy(
		pi, gains_tblid, pi_ac->rxgainctrl_stage_len[i],
		offset, ACPHY_GAINDB_TBL_WIDTH, gains);
	memcpy(pi_ac->rxgainctrl_params[core].gaintbl[i], gains,
	       gain_tbl_entry_size * pi_ac->rxgainctrl_stage_len[i]);

}

uint8
wlc_phy_rxgainctrl_encode_gain_acphy(phy_info_t *pi, uint8 clipgain, uint8 core, int8 gain_dB,
	bool trloss, bool lna1byp, uint8 *gidx)
{
	int16 min_gains[ACPHY_MAX_RX_GAIN_STAGES], max_gains[ACPHY_MAX_RX_GAIN_STAGES];
	int8 k, idx, maxgain_this_stage;
	int16 sum_min_gains, gain_needed, tr = 0;
	uint8 i, j;
	int8 *gaintbl_this_stage, gain_this_stage;
	int16 total_gain = 0;
	int16 gain_applied = 0;
	uint8 *gainbitstbl_this_stage;
	uint8 gaintbl_len, lowest_idx;
	int8 lna1mingain, lna1maxgain, lna1byp_gain = 0;
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;
	acphy_rxgainctrl_t gainctrl_params;
	uint8 rnd_stage = TINY_RADIO(pi) ? ACPHY_MAX_RX_GAIN_STAGES - 4
	                                 : ACPHY_MAX_RX_GAIN_STAGES - 3;

	PHY_TRACE(("%s: TARGET %d\n", __FUNCTION__, gain_dB));
	ASSERT(core < PHY_CORE_MAX);

	memcpy(&gainctrl_params, &pi_ac->rxgainctrl_params[core], sizeof(acphy_rxgainctrl_t));

	/* Over-write tia table to all same if using tia_high_mode */
	if ((ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev)) &&
		(clipgain == 0)) {
		idx = 7;
		for (j = 0; j < pi_ac->rxgainctrl_stage_len[TIA_ID]; j++) {
			gainctrl_params.gainbitstbl[TIA_ID][j] = idx;
			gainctrl_params.gaintbl[TIA_ID][j] =
			        pi_ac->rxgainctrl_params[core].gaintbl[TIA_ID][idx];
		}
	}

	if (trloss) {
		tr =  pi_ac->fem_rxgains[core].trloss;
	} else if (lna1byp) {
		if (ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev)) {
		lna1byp_gain = (READ_PHYREGC(pi, _lna1BypVals, core) &
		                ACPHY_Core0_lna1BypVals_lna1BypGain0_MASK(pi->pubpi.phy_rev))
		        >> ACPHY_Core0_lna1BypVals_lna1BypGain0_SHIFT(pi->pubpi.phy_rev);
		} else {
			tr = pi_ac->fem_rxgains[core].trloss;
			lna1mingain = gainctrl_params.gaintbl[1][0];
			lna1maxgain = gainctrl_params.gaintbl[1][5];
			tr = tr - (lna1maxgain - lna1mingain);
			PHY_INFORM(("lna1mingain=%d ,lna1maxgain=%d, new tr=%d \n",
			            lna1mingain, lna1maxgain, tr));
		}
	}

	gain_needed = gain_dB + tr;
	for (i = 0; i < ACPHY_MAX_RX_GAIN_STAGES; i++)
		max_gains[i] = pi_ac->rxgainctrl_maxout_gains[i] + tr;

	for (i = 0; i < ACPHY_MAX_RX_GAIN_STAGES; i++) {
		min_gains[i] = gainctrl_params.gaintbl[i][0];
	}

	for (i = 0; i < ACPHY_MAX_RX_GAIN_STAGES; i++) {
		if (i == rnd_stage) {
			if ((gain_needed % 3) == 2)
				++gain_needed;
			if ((!TINY_RADIO(pi)) && (gain_needed > 30))
				gain_needed = 30;
		}
		sum_min_gains = 0;

		for (j = i + 1; j < ACPHY_MAX_RX_GAIN_STAGES; j++) {
			if (TINY_RADIO(pi) && i < 5 && j >= 5)
				break;
			sum_min_gains += min_gains[j];
		}

		maxgain_this_stage = gain_needed - sum_min_gains;
		gaintbl_this_stage = gainctrl_params.gaintbl[i];
		gainbitstbl_this_stage = gainctrl_params.gainbitstbl[i];
		gaintbl_len = pi_ac->rxgainctrl_stage_len[i];
		if (lna1byp && (i == 1)) {
			gaintbl_len = 1;
		}

		for (k = gaintbl_len - 1; k >= 0; k--) {
			idx = k;
			if (lna1byp && (i == 1)) {
				gain_this_stage = lna1byp_gain;
			} else {
				gain_this_stage = gaintbl_this_stage[idx];
			}
			total_gain = gain_this_stage + gain_applied;
			lowest_idx = 0;

			if (gainbitstbl_this_stage[idx] == gainbitstbl_this_stage[0])
				lowest_idx = 1;

			if ((lowest_idx == 1) || ((lna1byp == 1) && (i == 1)) ||
			    ((gain_this_stage <= maxgain_this_stage) && (total_gain
			                                                 <= max_gains[i]))) {
				gidx[i] = ((lna1byp == 1) && (i == 1)) ? 6 :
				        gainbitstbl_this_stage[idx];
				gain_applied += gain_this_stage;
				gain_needed = gain_needed - gain_this_stage;
				break;
			}
		}

		/* If we want to fix max_tia_gain = initgain (for lesi) */
		if ((clipgain == 0) && (i > 0) && (i <= 3) && pi_ac->tia_idx_max_eq_init) {
			wlc_phy_max_anagain_to_initgain_lesi_acphy(pi, i, core, gidx[i]);
		}
	}
	PHY_INFORM(("gain_applied = %d, tr = %d \n", gain_applied, tr));

	return (gain_applied - tr);
}

static void
wlc_phy_rxgainctrl_nbclip_acphy(phy_info_t *pi, uint8 core, int8 rxpwr_dBm)
{
	/* Multuply all pwrs by 10 to avoid floating point math */
	int rxpwrdBm_60mv, pwr;
	int pwr_60mv[] = {-40, -40, -40};     /* 20, 40, 80 */
	uint8 nb_thresh[] = {0, 35, 60, 80, 95, 120, 140, 156}; /* nb_thresh*10 to avoid float */
	const char *reg_name[ACPHY_NUM_NB_THRESH] = {"low", "low", "mid", "mid", "mid",
					       "mid", "high", "high"};
	uint8 mux_sel[] = {0, 0, 1, 1, 1, 1, 2, 2};
	uint8 reg_val[] = {1, 0, 1, 2, 0, 3, 1, 0};
	uint8 nb, i;
	int nb_thresh_bq[ACPHY_NUM_NB_THRESH];
	int v1, v2, vdiff1, vdiff2;
	uint8 idx[ACPHY_MAX_RX_GAIN_STAGES];
	uint16 initgain_codeA;
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;

	ASSERT(core < PHY_CORE_MAX);
	ASSERT(RADIOID(pi->pubpi.radioid) == BCM2069_ID);

	rxpwrdBm_60mv = (CHSPEC_IS80(pi->radio_chanspec) ||
		PHY_AS_80P80(pi, pi->radio_chanspec)) ? pwr_60mv[2]
		: CHSPEC_IS160(pi->radio_chanspec) ? 0 // FIXME
		: (CHSPEC_IS40(pi->radio_chanspec)) ? pwr_60mv[1] : pwr_60mv[0];

	for (i = 0; i < ACPHY_NUM_NB_THRESH; i++) {
		nb_thresh_bq[i] = rxpwrdBm_60mv + nb_thresh[i];
	}

	/* Get the INITgain code */
	initgain_codeA = READ_PHYREGC(pi, InitGainCodeA, core);

	idx[0] = (initgain_codeA &
		ACPHY_REG_FIELD_MASK(pi, InitGainCodeA, core, initExtLnaIndex)) >>
		ACPHY_REG_FIELD_SHIFT(pi, InitGainCodeA, core, initExtLnaIndex);
	idx[1] = (initgain_codeA &
		ACPHY_REG_FIELD_MASK(pi, InitGainCodeA, core, initLnaIndex)) >>
		ACPHY_REG_FIELD_SHIFT(pi, InitGainCodeA, core, initLnaIndex);
	idx[2] = (initgain_codeA &
		ACPHY_REG_FIELD_MASK(pi, InitGainCodeA, core, initlna2Index)) >>
		ACPHY_REG_FIELD_SHIFT(pi, InitGainCodeA, core, initlna2Index);
	idx[3] = (initgain_codeA &
		ACPHY_REG_FIELD_MASK(pi, InitGainCodeA, core, initmixergainIndex)) >>
		ACPHY_REG_FIELD_SHIFT(pi, InitGainCodeA, core, initmixergainIndex);
	idx[4] = READ_PHYREGFLDC(pi, InitGainCodeB, core, InitBiQ0Index);
	idx[5] = READ_PHYREGFLDC(pi, InitGainCodeB, core, InitBiQ1Index);
	idx[6] = READ_PHYREGFLDC(pi, InitGainCodeB, core, initvgagainIndex);

	pwr = rxpwr_dBm;
	for (i = 0; i < ACPHY_MAX_RX_GAIN_STAGES - 2; i++)
		pwr += pi_ac->rxgainctrl_params[core].gaintbl[i][idx[i]];
	if (pi_ac->curr_desense->elna_bypass == 1)
		pwr = pwr - pi_ac->fem_rxgains[core].trloss;
	pwr = pwr * 10;

	nb = 0;
	if (pwr < nb_thresh_bq[0]) {
		nb = 0;
	} else if (pwr > nb_thresh_bq[ACPHY_NUM_NB_THRESH - 1]) {
		nb = ACPHY_NUM_NB_THRESH - 1;

		/* Reduce the bq0 gain, if can't achieve nbclip with highest nbclip thresh */
		if ((pwr - nb_thresh_bq[ACPHY_NUM_NB_THRESH - 1]) > 20) {
			if ((idx[4] > 0) && (idx[5] < 7)) {
				MOD_PHYREGC(pi, InitGainCodeB, core, InitBiQ0Index, idx[4] - 1);
				MOD_PHYREGC(pi, InitGainCodeB, core, InitBiQ1Index, idx[5] + 1);
			}
		}
	} else {
		for (i = 0; i < ACPHY_NUM_NB_THRESH - 1; i++) {
			v1 = nb_thresh_bq[i];
			v2 = nb_thresh_bq[i + 1];
			if ((pwr >= v1) && (pwr <= v2)) {
				vdiff1 = pwr > v1 ? (pwr - v1) : (v1 - pwr);
				vdiff2 = pwr > v2 ? (pwr - v2) : (v2 - pwr);

				if (vdiff1 < vdiff2)
					nb = i;
				else
					nb = i+1;
				break;
			}
		}
	}

	MOD_PHYREGC(pi, RssiClipMuxSel, core, fastAgcNbClipMuxSel, mux_sel[nb]);

	if (strcmp(reg_name[nb], "low") == 0) {
		MOD_RADIO_REGC(pi, NBRSSI_CONFG, core, nbrssi_Refctrl_low, reg_val[nb]);
	} else if (strcmp(reg_name[nb], "mid") == 0) {
		MOD_RADIO_REGC(pi, NBRSSI_CONFG, core, nbrssi_Refctrl_mid, reg_val[nb]);
	} else {
		MOD_RADIO_REGC(pi, NBRSSI_CONFG, core, nbrssi_Refctrl_high, reg_val[nb]);
	}
}

static void
wlc_phy_rxgainctrl_w1clip_acphy(phy_info_t *pi, uint8 core, int8 rxpwr_dBm)
{
	/* Multuply all pwrs by 10 to avoid floating point math */

	int lna1_rxpwrdBm_lo4;
	int lna1_pwrs_w1clip[] = {-340, -340, -340};   /* 20, 40, 80 */
	uint8 *w1_hi, w1_delta[] = {0, 19, 35, 49, 60, 70, 80, 88, 95, 102, 109, 115};
	uint8 w1_delta_hi2g[] = {0, 19, 35, 49, 60, 70, 80, 92, 105, 120, 130, 140};
	uint8 w1_delta_hi5g[] = {0, 19, 35, 49, 60, 70, 80, 96, 113, 130, 155, 180};
	int w1_thresh_low[ACPHY_NUM_W1_THRESH], w1_thresh_mid[ACPHY_NUM_W1_THRESH];
	int w1_thresh_high[ACPHY_NUM_W1_THRESH];
	int *w1_thresh;
	uint8 i, w1_muxsel, w1;
	uint8 elna, lna1_idx;
	int v1, v2, vdiff1, vdiff2, pwr, lna1_diff;
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;

	ASSERT(core < PHY_CORE_MAX);
	w1_hi = CHSPEC_IS2G(pi->radio_chanspec) ? &w1_delta_hi2g[0] : &w1_delta_hi5g[0];

	if (TINY_RADIO(pi)) {
		if (CHSPEC_IS2G(pi->radio_chanspec))
			lna1_rxpwrdBm_lo4 = (ACMAJORREV_32(pi->pubpi.phy_rev) ||
				ACMAJORREV_33(pi->pubpi.phy_rev))? -360 : -370;
		else
			lna1_rxpwrdBm_lo4 = -310;
	} else
		lna1_rxpwrdBm_lo4 = (CHSPEC_IS80(pi->radio_chanspec) ||
				PHY_AS_80P80(pi, pi->radio_chanspec)) ? lna1_pwrs_w1clip[2]
				: CHSPEC_IS160(pi->radio_chanspec) ? 0 // FIXME
		      : (CHSPEC_IS40(pi->radio_chanspec)) ? lna1_pwrs_w1clip[1]
				: lna1_pwrs_w1clip[0];

	/* mid is 6dB higher than low, and high is 6dB higher than mid */
	for (i = 0; i < ACPHY_NUM_W1_THRESH; i++) {
		w1_thresh_low[i] = lna1_rxpwrdBm_lo4 + w1_delta[i];
		w1_thresh_mid[i] = 60 + w1_thresh_low[i];
		w1_thresh_high[i] = 120 + lna1_rxpwrdBm_lo4 + w1_hi[i];
	}

	elna = pi_ac->rxgainctrl_params[core].gaintbl[0][0];

	lna1_idx = READ_PHYREGFLDC(pi, InitGainCodeA, core, initLnaIndex);

	if (TINY_RADIO(pi)) {
		lna1_diff = 24 - pi_ac->rxgainctrl_params[core].gaintbl[1][lna1_idx];
	} else if (CHSPEC_IS2G(pi->radio_chanspec) &&
	    (ACMAJORREV_2(pi->pubpi.phy_rev) && ACMINORREV_3(pi))) {
		lna1_diff = 25 - pi_ac->rxgainctrl_params[core].gaintbl[1][lna1_idx];
	} else {
		lna1_diff = pi_ac->rxgainctrl_params[core].gaintbl[1][5] -
		            pi_ac->rxgainctrl_params[core].gaintbl[1][lna1_idx];
	}

	pwr = rxpwr_dBm + elna - lna1_diff;
	if (pi_ac->curr_desense->elna_bypass == 1)
		pwr = pwr - pi_ac->fem_rxgains[core].trloss;
	pwr = pwr * 10;

	if (pwr <= w1_thresh_low[0]) {
		w1 = 0;
		w1_muxsel = 0;
	} else if (pwr >= w1_thresh_high[ACPHY_NUM_W1_THRESH - 1]) {
		w1 = 11;
		w1_muxsel = 2;
	} else {
		if (pwr > w1_thresh_mid[ACPHY_NUM_W1_THRESH - 1]) {
			w1_thresh = w1_thresh_high;
			w1_muxsel = 2;
		} else if (pwr < w1_thresh_mid[0]) {
			w1_thresh = w1_thresh_low;
			w1_muxsel = 0;
		} else {
			w1_thresh = w1_thresh_mid;
			w1_muxsel = 1;
		}

		for (w1 = 0; w1 < ACPHY_NUM_W1_THRESH - 1; w1++) {
			v1 = w1_thresh[w1];
			v2 = w1_thresh[w1 + 1];
			if ((pwr >= v1) && (pwr <= v2)) {
				vdiff1 = pwr > v1 ? (pwr - v1) : (v1 - pwr);
				vdiff2 = pwr > v2 ? (pwr - v2) : (v2 - pwr);

				if (vdiff2 <= vdiff1)
					w1 = w1 + 1;

				break;
			}
		}
	}

	if (TINY_RADIO(pi)) {
		MOD_PHYREGC(pi, RssiClipMuxSel, core, fastAgcW1ClipMuxSel, w1_muxsel);

		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			MOD_RADIO_REG_TINY(pi, LNA2G_RSSI1, core, lna2g_dig_wrssi1_threshold, w1+4);
		} else {
			MOD_RADIO_REG_TINY(pi, LNA5G_RSSI1, core, lna5g_dig_wrssi1_threshold, w1+4);
		}
	} else {
		/* the w1 thresh array is wrt w1 code = 4 */
		/*	MOD_PHYREGC(pi, FastAgcClipCntTh, core, fastAgcW1ClipCntTh, w1 + 4); */
		MOD_PHYREGC(pi, RssiClipMuxSel, core, fastAgcW1ClipMuxSel, w1_muxsel);
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			MOD_RADIO_REGC(pi, LNA2G_RSSI, core, dig_wrssi1_threshold, w1 + 4);
		} else {
			MOD_RADIO_REGC(pi, LNA5G_RSSI, core, dig_wrssi1_threshold, w1 + 4);
		}
	}
}

#ifndef WLC_DISABLE_ACI
static void
wlc_phy_set_crs_min_pwr_higain_acphy(phy_info_t *pi, uint8 thresh)
{
	MOD_PHYREG(pi, crsminpoweru0, crsminpower1, thresh);
	MOD_PHYREG(pi, crsmfminpoweru0, crsmfminpower1, thresh);
	MOD_PHYREG(pi, crsminpowerl0, crsminpower1, thresh);
	MOD_PHYREG(pi, crsmfminpowerl0, crsmfminpower1, thresh);
	MOD_PHYREG(pi, crsminpoweruSub10, crsminpower1, thresh);
	MOD_PHYREG(pi, crsmfminpoweruSub10, crsmfminpower1,  thresh);
	MOD_PHYREG(pi, crsminpowerlSub10, crsminpower1, thresh);
	MOD_PHYREG(pi, crsmfminpowerlSub10, crsmfminpower1,  thresh);
}
#endif /* !WLC_DISABLE_ACI */

static void
wlc_phy_limit_rxgaintbl_acphy(uint8 gaintbl[], uint8 gainbitstbl[], uint8 sz,
	const uint8 default_gaintbl[], uint8 min_idx, uint8 max_idx)
{
	uint8 i;

	for (i = 0; i < sz; i++) {
		if (i < min_idx) {
			gaintbl[i] = default_gaintbl[min_idx];
			gainbitstbl[i] = min_idx;
		} else if (i > max_idx) {
			gaintbl[i] = default_gaintbl[max_idx];
			gainbitstbl[i] = max_idx;
		} else {
			gaintbl[i] = default_gaintbl[i];
			gainbitstbl[i] = i;
		}
	}
}

static void
wlc_phy_rxgainctrl_nbclip_acphy_tiny(phy_info_t *pi, uint8 core, int8 rxpwr_dBm)
{
	/* Multuply all pwrs by 10 to avoid floating point math */
	int16 rxpwrdBm_bw, pwr, tmp_pwr;
	int16 pwr_dBm[] = {-40, -40, -40};	/* 20, 40, 80 */
	int16 nb_thresh[] = { 0, 20, 40, 60, 80, 100, 120, 140, 160}; /* nb_thresh*10 avoid float */
	const char * const reg_name[ACPHY_NUM_NB_THRESH_TINY] = {"low", "low", "low", "mid", "mid",
	                                                    "high", "high", "high", "high"};
	uint8 mux_sel[] = {0, 0, 0, 1, 1, 2, 2, 2, 2};
	uint8 reg_val[] = {0, 2, 4, 2, 4, 1, 3, 5, 7};
	uint8 nb, i;
	int v1, v2, vdiff1, vdiff2;
	int8 idx[ACPHY_MAX_RX_GAIN_STAGES] = {-1, -1, -1, -1, -1, -1, -1};
	uint16 initgain_codeA;
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;
	bool use_w3_detector = FALSE; /* To chhose betwen NB(T2) and W3(T1) detectors */
	int16 t2_delta_dB = 0; /* the power delta between T2 and T1 detectors */
	int16 nb_thresh_end;

	ASSERT(core < PHY_CORE_MAX);
	ASSERT(TINY_RADIO(pi));

	rxpwrdBm_bw = (CHSPEC_IS80(pi->radio_chanspec) ||
		PHY_AS_80P80(pi, pi->radio_chanspec)) ? pwr_dBm[2]
		: (CHSPEC_IS160(pi->radio_chanspec)) ? 0 // FIXME
		: (CHSPEC_IS40(pi->radio_chanspec)) ? pwr_dBm[1] : pwr_dBm[0];

	PHY_TRACE(("%s: adc pwr %d \n", __FUNCTION__, rxpwrdBm_bw));

	nb_thresh_end = nb_thresh[ACPHY_NUM_NB_THRESH_TINY - 1] + rxpwrdBm_bw;

	/* Get the INITgain code */
	initgain_codeA = READ_PHYREGC(pi, InitGainCodeA, core);

	idx[0] = (initgain_codeA &
		ACPHY_REG_FIELD_MASK(pi, InitGainCodeA, core, initExtLnaIndex)) >>
		ACPHY_REG_FIELD_SHIFT(pi, InitGainCodeA, core, initExtLnaIndex);
	idx[1] = (initgain_codeA &
		ACPHY_REG_FIELD_MASK(pi, InitGainCodeA, core, initLnaIndex)) >>
		ACPHY_REG_FIELD_SHIFT(pi, InitGainCodeA, core, initLnaIndex);
	idx[3] = (initgain_codeA &
		ACPHY_REG_FIELD_MASK(pi, InitGainCodeA, core, initmixergainIndex)) >>
		ACPHY_REG_FIELD_SHIFT(pi, InitGainCodeA, core, initmixergainIndex);

	pwr = rxpwr_dBm;

	for (i = 0; i < ACPHY_MAX_RX_GAIN_STAGES - 2; i++) {
		if (idx[i] >= 0) {
			tmp_pwr = pi_ac->rxgainctrl_params[core].gaintbl[i][idx[i]];
			pwr += tmp_pwr;
		}
	}

	if (pi_ac->curr_desense->elna_bypass == 1)
		pwr -= pi_ac->fem_rxgains[core].trloss;

	pwr *= 10;

	/* Use T1 detector bank if T2 bank is not covering pwr, and in 5G only (for the */
	/* time being). As it can be seen for T1 vs. T2 delta to be > 0 TIA index >4 is */
	/* needed, typically for the current 2G gain line up TIA index = 4 (BIQ 2 is off), */
	/* and T1 doesn't help. To make this work for 2G, it is needed to decresse LNA */
	/* index which can lower sensitivity. */
	if (pwr > nb_thresh_end) {
	  use_w3_detector = TRUE;

		if (CHSPEC_IS80(pi->radio_chanspec) ||
				PHY_AS_80P80(pi, pi->radio_chanspec)) {
	    if (idx[3] >= 8)
	      t2_delta_dB = 75;
	    else if (idx[3] >= 4)
	      t2_delta_dB = 60;
	    else
	      t2_delta_dB = 0;
	  } else if (CHSPEC_IS160(pi->radio_chanspec)) {
		  ASSERT(0);
	  } else if (CHSPEC_IS40(pi->radio_chanspec)) {
	    if (idx[3] >= 8)
	      t2_delta_dB = 75;
	    else if (idx[3] >= 5)
	      t2_delta_dB = 60;
	    else
	      t2_delta_dB = 0;
	  } else {
	    if (idx[3] >= 5)
	      t2_delta_dB = 60;
	    else
	      t2_delta_dB = 0;
	  }

	}

	use_w3_detector = use_w3_detector && (t2_delta_dB > 0);
	for (i = 0; i < ACPHY_NUM_NB_THRESH_TINY; i++) {
	  nb_thresh[i] += (rxpwrdBm_bw + t2_delta_dB);
	}

	if (pwr < nb_thresh[0]) {
		nb = 0;
	} else if (pwr > nb_thresh[ACPHY_NUM_NB_THRESH_TINY - 1]) {
		nb = ACPHY_NUM_NB_THRESH_TINY - 1;
	} else {
		nb = 0;
		for (i = 0; i < ACPHY_NUM_NB_THRESH_TINY - 1; i++) {
			v1 = nb_thresh[i];
			v2 = nb_thresh[i + 1];
			if ((pwr >= v1) && (pwr <= v2)) {
				vdiff1 = pwr > v1 ? (pwr - v1) : (v1 - pwr);
				vdiff2 = pwr > v2 ? (pwr - v2) : (v2 - pwr);

				nb = (vdiff1 < vdiff2) ? i : i + 1;
				break;
			}
		}
	}

	if (ACMAJORREV_4(pi->pubpi.phy_rev)) {
		MOD_RADIO_REG_TINY(pi, TIA_CFG13, core, wrssi3_ref_low_sel, 0);
		MOD_RADIO_REG_TINY(pi, TIA_CFG12, core, wrssi3_ref_mid_sel, 0);
		MOD_RADIO_REG_TINY(pi, TIA_CFG12, core, wrssi3_ref_high_sel, 0);
		MOD_RADIO_REG_TINY(pi, TIA_CFG13, core, nbrssi_ref_low_sel, 0);
		MOD_RADIO_REG_TINY(pi, TIA_CFG13, core, nbrssi_ref_mid_sel, 0);
		MOD_RADIO_REG_TINY(pi, TIA_CFG13, core, nbrssi_ref_high_sel, 0);
	}

	MOD_PHYREGC(pi, RssiClipMuxSel, core, fastAgcNbClipMuxSel, mux_sel[nb]);
	if (use_w3_detector) {
	  MOD_PHYREGC(pi, RssiClipMuxSel, core, fastAgcW3ClipMuxSel, 1);
	  if (strcmp(reg_name[nb], "low") == 0) {
	    MOD_RADIO_REG_TINY(pi, TIA_CFG13, core, wrssi3_ref_low_sel, reg_val[nb]);
	  } else if (strcmp(reg_name[nb], "mid") == 0) {
	    MOD_RADIO_REG_TINY(pi, TIA_CFG12, core, wrssi3_ref_mid_sel, reg_val[nb]);
	  } else {
	    MOD_RADIO_REG_TINY(pi, TIA_CFG12, core, wrssi3_ref_high_sel, reg_val[nb]);
	  }
	} else {
	  MOD_PHYREGC(pi, RssiClipMuxSel, core, fastAgcW3ClipMuxSel, 0);
	  if (strcmp(reg_name[nb], "low") == 0) {
	    MOD_RADIO_REG_TINY(pi, TIA_CFG13, core, nbrssi_ref_low_sel, reg_val[nb]);
	  } else if (strcmp(reg_name[nb], "mid") == 0) {
	    MOD_RADIO_REG_TINY(pi, TIA_CFG13, core, nbrssi_ref_mid_sel, reg_val[nb]);
	  } else {
	    MOD_RADIO_REG_TINY(pi, TIA_CFG13, core, nbrssi_ref_high_sel, reg_val[nb]);
	  }
	}
}

/* ********************************************* */
/*				External Definitions					*/
/* ********************************************* */
/**********  DESENSE : ACI, NOISE, BT (start)  ******** */

void
wlc_phy_rxgainctrl_gainctrl_acphy_tiny(phy_info_t *pi, uint8 init_desense)
{
	/* at present this is just a place holder for
	 * 'static' ELNA configuration. Eventually both TCL and
	 * driver should be changes to follow the auto-calc
	 * routine used in wlc_phy_rxgainctrl_gainctrl_acphy
	 */
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;
	bool elna_present = (CHSPEC_IS2G(pi->radio_chanspec)) ? BF_ELNA_2G(pi_ac)
	                                                      : BF_ELNA_5G(pi_ac);
	uint8 max_analog_gain;
	int16 maxgain[ACPHY_MAX_RX_GAIN_STAGES] = {0, 0, 0, 0, -100, 125, 125};
	uint8 i, core;
	bool init_trtx, hi_trtx, md_trtx, lo_trtx, clip2_trtx;
	bool lna1byp_fem, lo_lna1byp_core, lo_lna1byp = FALSE, md_lna1byp = FALSE;
	int8 md_low_end, hi_high_end, lo_low_end, md_high_end;
	int8 clip2_gain;
	int8 hi_gain1, mid_gain1, lo_gain1;
	int8 nbclip_pwrdBm, w1clip_pwrdBm;
	int8 clip_gain[] = {61, 41, 19, 1};

	pi_ac->mdgain_trtx_allowed = FALSE;

	MOD_PHYREG(pi, RxSdFeConfig6, rx_farrow_rshift_0, 0);

	max_analog_gain = READ_PHYREGFLD(pi, Core0HpFBw, maxAnalogGaindb);

	/* fill in gain limits for analog stages */
	maxgain[0] = maxgain[1] = maxgain[2] = maxgain[3] = max_analog_gain;

	for (i = 0; i < ACPHY_MAX_RX_GAIN_STAGES; i++)
		pi_ac->rxgainctrl_maxout_gains[i] = maxgain[i];

	/* Keep track of it (used in interf_mitigation) */
	pi_ac->curr_desense->elna_bypass = wlc_phy_get_max_lna_index_acphy(pi, ELNA_ID);
	init_trtx = elna_present & pi_ac->curr_desense->elna_bypass;
	hi_trtx = elna_present & pi_ac->curr_desense->elna_bypass;
	md_trtx = elna_present & (pi_ac->mdgain_trtx_allowed | pi_ac->curr_desense->elna_bypass);
	lo_trtx = elna_present;
	clip2_trtx = md_trtx;

	if (ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev)) {
		clip_gain[0] = CHSPEC_IS2G(pi->radio_chanspec) ?
		        ACPHY_INIT_GAIN_4365_2G : ACPHY_INIT_GAIN_4365_5G;

#ifdef WL11ULB
		if (CHSPEC_IS10(pi->radio_chanspec) || CHSPEC_IS5(pi->radio_chanspec))
			clip_gain[0] += 3;
		PHY_CAL(("---%s: ulb: init_gain = %d\n", __FUNCTION__, clip_gain[0]));
#endif /* WL11ULB */

		clip_gain[1] = CHSPEC_IS2G(pi->radio_chanspec) ? 47 : 44;
		clip_gain[2] = 31;
		clip_gain[3] = 21;

		clip2_trtx = lo_trtx;
		if (!elna_present) {
			lo_lna1byp = TRUE;
		}
	} else if (!elna_present || md_trtx) {
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			clip_gain[2] = 19;
			clip_gain[3] = 1;
		} else {
			clip_gain[2] = 19;
			clip_gain[3] = 1;
		}
	} else {
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			clip_gain[2] = 24;
			clip_gain[3] = 15;
		} else {
			clip_gain[2] = 24;
			clip_gain[3] = 15;
		}
	}

	if (ACPHY_ENABLE_FCBS_HWACI(pi))
		/* Apply desense */
		for (i = 0; i < 4; i++)
			clip_gain[i] -= pi_ac->curr_desense->clipgain_desense[i];

	/* with elna if md_gain uses TR != T, then LO_gain needs to be higher */
	clip2_gain = clip2_trtx ? clip_gain[3] : (clip_gain[2] + clip_gain[3]) >> 1;

	FOREACH_CORE(pi, core) {
		lna1byp_fem = pi_ac->fem_rxgains[core].lna1byp;
		lo_lna1byp_core = lo_lna1byp | lna1byp_fem;

		if (lna1byp_fem && ACMAJORREV_4(pi->pubpi.phy_rev)) {
			WRITE_PHYREGC(pi, _lna1BypVals, core, 0xfa1);
			MOD_PHYREG(pi, AfePuCtrl, lna1_pd_during_byp, 1);
		}

		/* 0,1,2,3 for Init, hi, md and lo gains respectively */
		wlc_phy_rxgainctrl_set_init_clip_gain_acphy(pi, 0, clip_gain[0] - init_desense,
		                                            init_trtx, FALSE, core);
		hi_gain1 = wlc_phy_rxgainctrl_set_init_clip_gain_acphy(pi, 1, clip_gain[1],
		                                                       hi_trtx, FALSE, core);
		mid_gain1 = wlc_phy_rxgainctrl_set_init_clip_gain_acphy(pi, 2, clip_gain[2],
		                                                        md_trtx, md_lna1byp, core);
		wlc_phy_rxgainctrl_set_init_clip_gain_acphy(pi, 4, clip2_gain, clip2_trtx,
		                                            FALSE, core);
		lo_gain1 = wlc_phy_rxgainctrl_set_init_clip_gain_acphy(pi, 3, clip_gain[3],
		                                                       lo_trtx,
		                                                       lo_lna1byp_core, core);

		/* NB_CLIP */
		md_low_end = wlc_phy_rxgainctrl_calc_low_sens_acphy(pi, mid_gain1, md_trtx,
		                                                    md_lna1byp, core);
		hi_high_end = wlc_phy_rxgainctrl_calc_high_sens_acphy(pi, hi_gain1, hi_trtx,
		                                                      FALSE, core);
		lo_low_end = wlc_phy_rxgainctrl_calc_low_sens_acphy(pi, lo_gain1, lo_trtx,
		                                                    lo_lna1byp_core, core);
		md_high_end = wlc_phy_rxgainctrl_calc_high_sens_acphy(pi, mid_gain1, md_trtx,
		                                                      md_lna1byp, core);

		w1clip_pwrdBm = (lo_low_end + md_high_end) >> 1;

		/* -1 times pwr to avoid rounding off error */
		if (CHSPEC_IS20(pi->radio_chanspec)) {
			/*
			 * use mid-point, as late clip2 is causing minor humps,
			 * move nb/w1 clip to a lower pwr
			 */
			nbclip_pwrdBm = (md_low_end + hi_high_end) >> 1;
		} else {
			/*
			 * (0.4*md/lo + 0.6*hi/md) fixed point. On fading channels,
			 * low-end sensitivity degrades, keep more margin there.
			 */
			nbclip_pwrdBm = (((-2 * md_low_end) + (-3 * hi_high_end)) * 13) >> 6;
			nbclip_pwrdBm = -nbclip_pwrdBm;
		}

		wlc_phy_rxgainctrl_nbclip_acphy_tiny(pi, core, nbclip_pwrdBm);
		wlc_phy_rxgainctrl_w1clip_acphy(pi, core, w1clip_pwrdBm);
	}

	/* Saving the values of Init gain to be used
	 * in "wlc_phy_get_rxgain_acphy" function (used in rxiqest)
	 */
	pi_ac->initGain_codeA = READ_PHYREGC(pi, InitGainCodeA, 0);
	pi_ac->initGain_codeB = READ_PHYREGC(pi, InitGainCodeB, 0);
}

uint8
wlc_phy_get_max_lna_index_acphy(phy_info_t *pi, uint8 lna)
{
	uint8 max_idx, desense_state;
	acphy_desense_values_t *desense = NULL;
	uint8 elna_bypass, lna1_backoff, lna2_backoff;
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;

	desense = pi_ac->total_desense;
	elna_bypass = desense->elna_bypass;
	lna1_backoff = desense->lna1_tbl_desense;
	lna2_backoff = desense->lna2_tbl_desense;

	if (pi_ac->aci != NULL) {
		desense_state = pi_ac->aci->hwaci_desense_state;
	} else {
		desense_state = 0;
	}

	/* Find default max_idx */
	if (lna == ELNA_ID) {
		max_idx = elna_bypass;       /* elna */
	} else if (lna == LNA1_ID) {               /* lna1 */
		max_idx = (ACMAJORREV_32(pi->pubpi.phy_rev) ||
			ACMAJORREV_33(pi->pubpi.phy_rev)) ? (desense_state != 0 ?
		    desense->lna1_idx_max: ACPHY_4365_MAX_LNA1_IDX) : ACPHY_MAX_LNA1_IDX;
		max_idx = MAX(0, max_idx - lna1_backoff);
	} else {                             /* lna2 */
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			if (BF_ELNA_2G(pi_ac) && (elna_bypass == 0)) {
				uint8 core = 0;
				if (pi_ac->sromi->femrx_2g[core].elna > 10) {
					max_idx = ACPHY_ELNA2G_MAX_LNA2_IDX;
				} else if (pi_ac->sromi->femrx_2g[core].elna > 8) {
					max_idx = ACPHY_ELNA2G_MAX_LNA2_IDX_L;
				} else {
					max_idx = ACPHY_ILNA2G_MAX_LNA2_IDX;
				}
			} else {
				max_idx = ACPHY_ILNA2G_MAX_LNA2_IDX;
			}
		} else {
			max_idx = (BF_ELNA_5G(pi_ac) && (elna_bypass == 0)) ?
			        ACPHY_ELNA5G_MAX_LNA2_IDX : ACPHY_ILNA5G_MAX_LNA2_IDX;
		}

		/* Fix max lna2 idx for 4349 to 3 */
		if (ACMAJORREV_4(pi->pubpi.phy_rev))
			max_idx = ACPHY_20693_MAX_LNA2_IDX;

		/* Fix max lna2 idx for 4365 to 1 */
		if (ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev))
			max_idx = desense_state != 0 ? desense->lna2_idx_max:
				ACPHY_4365_MAX_LNA2_IDX;

		max_idx = MAX(0, max_idx - lna2_backoff);
	}

	return max_idx;
}

#ifndef WLC_DISABLE_ACI

/*********** Desnese (geneal) ********** */
/* IMP NOTE: make sure whatever regs are chagned here are either:
1. reset back to defualt below OR
2. Updated in gainctrl()
*/
void
wlc_phy_desense_apply_acphy(phy_info_t *pi, bool apply_desense)
{
	/* reset:
	   1 --> clear aci settings (the ones that gainctrl does not clear)
	   0 --> apply aci_noise_bt mitigation
	*/

	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;
	acphy_desense_values_t *desense, *curr_desense;
	uint8 ofdm_desense, bphy_desense, initgain_desense;
	uint8 crsmin_thresh, crsmin_init;
	int8 crsmin_high;
	uint16 digigainlimit;
	uint8 bphy_minshiftbits[] = {0x77, 0x02, 0x03, 0x03, 0x03, 0x04, 0x04, 0x04, 0x04, 0x04,
	     0x04, 0x04, 0x04};
	uint16 bphy_peakenergy[]  = {0x10, 0x60, 0x10, 0x4c, 0x60, 0x30, 0x40, 0x40, 0x38, 0x2e,
	     0x40, 0x34, 0x40};
	uint8 bphy_initgain_backoff[] = {0, 0, 0, 0, 0, 0, 0, 3, 6, 9, 9, 12, 12};
	uint8 max_bphy_shiftbits = sizeof(bphy_minshiftbits) / sizeof(uint8);

	uint8 max_initgain_desense = 12, max_anlg_desense = 9 ;   /* only desnese bq0 */
	bool  ofdm_desense_extra_halfdB = 0;

	uint8 core, bphy_idx = 0;
	int8 zeros[] = {0, 0, 0, 0};

	bool call_gainctrl = FALSE;
	uint8 init_gain;

	if (TINY_RADIO(pi)) {
		if (ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev)) {
			init_gain = CHSPEC_IS2G(pi->radio_chanspec) ?
			        ACPHY_INIT_GAIN_4365_2G : ACPHY_INIT_GAIN_4365_5G;
		} else {
			init_gain = ACPHY_INIT_GAIN_TINY;
		}
	} else {
		init_gain = ACPHY_INIT_GAIN;
	}

	desense = pi_ac->total_desense;
	curr_desense = pi_ac->curr_desense;

	wlapi_suspend_mac_and_wait(pi->sh->physhim);

	if (!apply_desense && !desense->forced) {
		/* when channel is changed, and the current channel is in mitigatation, then
		   we need to restore the values. wlc_phy_rxgainctrl_gainctrl_acphy() takes
		   care of all the gainctrl part, but we need to still restore back bphy regs
		*/

		wlc_phy_set_crs_min_pwr_higain_acphy(pi, ACPHY_CRSMIN_GAINHI);

		wlc_phy_desense_mf_high_thresh_acphy(pi, FALSE);
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			digigainlimit = READ_PHYREG(pi, DigiGainLimit0) & 0x8000;
			WRITE_PHYREG(pi, DigiGainLimit0, digigainlimit | 0x4477);
			WRITE_PHYREG(pi, PeakEnergyL, 0x10);
		}

		wlc_phy_desense_print_phyregs_acphy(pi, "restore");
		/* turn on LESI */
		wlc_phy_lesi_acphy(pi, TRUE, 0);

		/* channal update reset for default */
		if (ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev)) {
		  wlc_phy_mlua_adjust_acphy(pi, FALSE);
		}
	} else {
		/* Get total desense based on aci & bt & lte */
		wlc_phy_desense_calc_total_acphy(pi);

		bphy_desense = MIN(ACPHY_ACI_MAX_DESENSE_BPHY_DB, desense->bphy_desense);
		ofdm_desense = MIN(ACPHY_ACI_MAX_DESENSE_OFDM_DB, desense->ofdm_desense);

		ofdm_desense_extra_halfdB = desense->ofdm_desense_extra_halfdB;

  		/* channal update set to interference mode */
		if (ofdm_desense > 0) {
		  if (ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev)) {
		    wlc_phy_mlua_adjust_acphy(pi, TRUE);
		  }
		} else {
		  if (ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev)) {
		    wlc_phy_mlua_adjust_acphy(pi, FALSE);
		  }

		}

		/* Update total desense */
		if (ACMAJORREV_4(pi->pubpi.phy_rev)) {
			desense->ofdm_desense = ofdm_desense;
			desense->bphy_desense = bphy_desense;
		}

		/* Initgain can change due to bphy desense */
		if ((ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev)) &&
		    (curr_desense->bphy_desense != bphy_desense))
			call_gainctrl = TRUE;

		/* Update current desense */
		curr_desense->ofdm_desense = ofdm_desense;
		curr_desense->bphy_desense = bphy_desense;
		curr_desense->ofdm_desense_extra_halfdB = ofdm_desense_extra_halfdB;

		/* if any ofdm desense is needed, first start using higher
		   mf thresholds (1dB sens loss)
		*/
		wlc_phy_desense_mf_high_thresh_acphy(pi, (ofdm_desense > 0));

        if (pi_ac->lesi) {
            /*
               First ACPHY_ACI_MAX_LESI_DESENSE_DB-1 of desense is done using lesi_crs
                ACPHY_ACI_MAX_LESI_DESENSE_DB+ dB is done by turning off lesi
               remainder is done using initgain/crsmin (with lesi off)
            */
            if(ofdm_desense < ACPHY_ACI_MAX_LESI_DESENSE_DB) {
                wlc_phy_lesi_acphy(pi, TRUE, (2*ofdm_desense) + ofdm_desense_extra_halfdB);
                ofdm_desense = 0; ofdm_desense_extra_halfdB = 0;
            } else {
                wlc_phy_lesi_acphy(pi, FALSE, 0);
                ofdm_desense -= ACPHY_ACI_MAX_LESI_DESENSE_DB;
            }
        } else {
            /* 1st dB is for mf_high thresh */
            if (ofdm_desense > 0) ofdm_desense --;
        }

		/* Distribute desense between INITgain & crsmin(ofdm) & digigain(bphy) */
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			/* round to 2, as bphy desnese table is in 2dB steps */
			bphy_idx = MIN((bphy_desense + 1) >> 1, max_bphy_shiftbits - 1);
			initgain_desense = bphy_initgain_backoff[bphy_idx];
		} else {
			initgain_desense = 0;
		}

#ifdef BCMLTECOEX
		if (pi_ac->ltecx_mode == 1)
			initgain_desense = ofdm_desense;
		else
#endif // endif
		initgain_desense = MIN(initgain_desense, max_initgain_desense);
		desense->analog_gain_desense_ofdm = MAX(desense->analog_gain_desense_ofdm,
			MIN(max_anlg_desense, ofdm_desense));

		if (ACPHY_HWACI_WITH_DESENSE_ENG(pi) && CHSPEC_IS2G(pi->radio_chanspec)) {
			desense->clipgain_desense[0] = MAX(desense->clipgain_desense[0],
				initgain_desense);
			if (curr_desense->clipgain_desense[0] != desense->clipgain_desense[0])
			{
				curr_desense->clipgain_desense[0] =
					desense->clipgain_desense[0];
				call_gainctrl = TRUE;
			}
		}

		/* OFDM Desense */
		/* With init gain, max crsmin desense = 30dB, after which ADC will start clipping */
		/* With HI gain, crsmin desense = new_sens - old_sens
		   = (-96 + desense) - (0 - (higain + 27))
		   = -69 + desense + higain
		*/

		/*  In TINY, clipping happens close to -60 dBm. SO Max desense of crsmin_init
		 *  should be increased from 30 to 35 dB.
		 *
		 *  crsmin_high is changed to take care of any change in default HI or INIT gain.
		*/
		if (ACMAJORREV_4(pi->pubpi.phy_rev) || ACMAJORREV_32(pi->pubpi.phy_rev) ||
			ACMAJORREV_33(pi->pubpi.phy_rev)) {
			crsmin_init = MIN(35, MAX(0, ofdm_desense - desense->clipgain_desense[0]));
			crsmin_high = ofdm_desense + ACPHY_HI_GAIN_TINY -
			        init_gain - desense->clipgain_desense[1];
		} else {
			crsmin_init = MIN(30, MAX(0, ofdm_desense - initgain_desense));
			crsmin_high = ofdm_desense + ACPHY_HI_GAIN - 69;
		}
		PHY_ACI(("aci_mode1, desense, init = %d, bphy_idx = %d, crsmin = {%d %d}\n",
		         initgain_desense, bphy_idx, crsmin_init, crsmin_high));

		if ((curr_desense->elna_bypass != desense->elna_bypass) ||
		    (curr_desense->lna1_gainlmt_desense != desense->lna1_gainlmt_desense) ||
		    (curr_desense->lna1_tbl_desense != desense->lna1_tbl_desense) ||
		    (curr_desense->lna2_tbl_desense != desense->lna2_tbl_desense) ||
		    (curr_desense->lna2_gainlmt_desense != desense->lna2_gainlmt_desense) ||
		    (curr_desense->mixer_setting_desense != desense->mixer_setting_desense) ||
		    (curr_desense->analog_gain_desense_ofdm != desense->analog_gain_desense_ofdm) ||
		    (curr_desense->analog_gain_desense_bphy != desense->analog_gain_desense_bphy)) {
			if (ACPHY_ENABLE_FCBS_HWACI(pi)) {
				/* Update the current structure to hold the new desense values */
				memcpy(curr_desense, desense, sizeof(acphy_desense_values_t));
				wlc_phy_rxgainctrl_set_gaintbls_acphy(pi, TRUE, TRUE, TRUE);
			} else {
				wlc_phy_upd_lna1_lna2_gains_acphy(pi);
			}
			call_gainctrl = TRUE;
		}

		/* if lna1/lna2 gaintable has changed, call gainctrl as it effects all clip gains */
		if (call_gainctrl) {
			if (TINY_RADIO(pi)) {
				if (ACMAJORREV_32(pi->pubpi.phy_rev) ||
					ACMAJORREV_33(pi->pubpi.phy_rev))
					wlc_phy_rxgainctrl_gainctrl_acphy_tiny(pi,
					                                       initgain_desense);
				else
					wlc_phy_rxgainctrl_gainctrl_acphy_tiny(pi, 0);
			} else {
				wlc_phy_rxgainctrl_gainctrl_acphy(pi);
			}
		}

		if (!TINY_RADIO(pi)) {
			/* Update INITgain */
			FOREACH_CORE(pi, core) {
				wlc_phy_rxgainctrl_set_init_clip_gain_acphy(pi, 0,
				                                            ACPHY_INIT_GAIN
				                                            - initgain_desense,
				                                            desense->elna_bypass,
				                                            FALSE, core);
			}
		}

		/* adjust crsmin threshold, 8 ticks increase gives 3dB rejection */
		crsmin_thresh = ACPHY_CRSMIN_DEFAULT +
				((44 * (2*crsmin_init + ofdm_desense_extra_halfdB)) >> 5);

#ifdef BCMLTECOEX
		if (pi_ac->ltecx_mode == 1)
			wlc_phy_set_crs_min_pwr_acphy(pi,
				MAX(crsmin_thresh, ACPHY_CRSMIN_DEFAULT), zeros);
		else
#endif // endif
		wlc_phy_set_crs_min_pwr_acphy(pi,
			MAX(crsmin_thresh, pi->u.pi_acphy->phy_crs_th_from_crs_cal), zeros);

		/* crs high_gain */
		crsmin_thresh = MAX(ACPHY_CRSMIN_GAINHI,
		                    ACPHY_CRSMIN_DEFAULT + ((88 * crsmin_high) >> 5));
		wlc_phy_set_crs_min_pwr_higain_acphy(pi, crsmin_thresh);

		/* bphy desense */
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			digigainlimit = READ_PHYREG(pi, DigiGainLimit0) & 0x8000;
			WRITE_PHYREG(pi, DigiGainLimit0,
			             digigainlimit | 0x4400 | bphy_minshiftbits[bphy_idx]);
			WRITE_PHYREG(pi, PeakEnergyL, bphy_peakenergy[bphy_idx]);
		}
		wlc_phy_desense_print_phyregs_acphy(pi, "apply");
	}

	/* Inform rate contorl to slow down is mitigation is on */
	wlc_phy_aci_updsts_acphy(pi);

	wlapi_enable_mac(pi->sh->physhim);
}

void
wlc_phy_desense_calc_total_acphy(phy_info_t *pi)
{
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;

#ifdef BCMLTECOEX
	acphy_desense_values_t *bt, *aci, *lte;
#else
	acphy_desense_values_t *bt, *aci;
#endif // endif
	uint8 i;

	acphy_desense_values_t *total = pi_ac->total_desense;

	aci = (pi_ac->aci == NULL) ? pi_ac->zero_desense : &pi_ac->aci->desense;

	/* if desense is forced, then skip without calculation and just use the forced value */
	if (total->forced)
		return;

#ifdef BCMLTECOEX
	if ((pi_ac->btc_mode == 0 && pi_ac->ltecx_mode == 0) || wlc_phy_is_scan_chan_acphy(pi) ||
	    CHSPEC_IS5G(pi->radio_chanspec) || ACMAJORREV_32(pi->pubpi.phy_rev) ||
	    ACMAJORREV_33(pi->pubpi.phy_rev)) {
		/* only consider aci desense */
		memcpy(total, aci, sizeof(acphy_desense_values_t));
	} else {
		/* Merge BT & ACI & LTE desense, take max */
		bt  = pi_ac->bt_desense;
		lte = pi_ac->rxgcrsi->lte_desense;
		total->ofdm_desense = MAX(MAX(aci->ofdm_desense, bt->ofdm_desense),
			lte->ofdm_desense);
		total->ofdm_desense_extra_halfdB = aci->ofdm_desense_extra_halfdB;
		total->bphy_desense = MAX(MAX(aci->bphy_desense, bt->bphy_desense),
			lte->bphy_desense);
		total->lna1_tbl_desense = MAX(MAX(aci->lna1_tbl_desense, bt->lna1_tbl_desense),
			lte->lna1_tbl_desense);
		total->analog_gain_desense_ofdm = MAX(MAX(aci->analog_gain_desense_ofdm,
			bt->analog_gain_desense_ofdm), lte->analog_gain_desense_ofdm);
		total->analog_gain_desense_bphy = MAX(MAX(aci->analog_gain_desense_bphy,
			bt->analog_gain_desense_bphy), lte->analog_gain_desense_bphy);
		total->lna2_tbl_desense = MAX(MAX(aci->lna2_tbl_desense, bt->lna2_tbl_desense),
			lte->lna2_tbl_desense);
		total->lna1_gainlmt_desense = MAX(MAX(aci->lna1_gainlmt_desense,
			bt->lna1_gainlmt_desense), lte->lna1_gainlmt_desense);
		total->lna2_gainlmt_desense = MAX(MAX(aci->lna2_gainlmt_desense,
			bt->lna2_gainlmt_desense), lte->lna2_gainlmt_desense);
		total->elna_bypass = MAX(MAX(aci->elna_bypass, bt->elna_bypass),
			lte->elna_bypass);
		total->mixer_setting_desense = MAX(MAX(aci->mixer_setting_desense,
			bt->mixer_setting_desense), lte->mixer_setting_desense);
		total->nf_hit_lna12 =  MAX(MAX(aci->nf_hit_lna12, bt->nf_hit_lna12),
			lte->nf_hit_lna12);
		total->on = aci->on | bt->on | lte->on;
		for (i = 0; i < 4; i++)
			total->clipgain_desense[i] = MAX(MAX(aci->clipgain_desense[i],
				bt->clipgain_desense[i]), lte->clipgain_desense[i]);
	}
#else
	if ((pi_ac->btc_mode == 0) || wlc_phy_is_scan_chan_acphy(pi) ||
	    CHSPEC_IS5G(pi->radio_chanspec) || ACMAJORREV_32(pi->pubpi.phy_rev) ||
	    ACMAJORREV_33(pi->pubpi.phy_rev)) {
		/* only consider aci desense */
		memcpy(total, aci, sizeof(acphy_desense_values_t));
	} else {
		/* Merge BT & ACI desense, take max */
		bt  = pi_ac->bt_desense;
		total->ofdm_desense = MAX(aci->ofdm_desense, bt->ofdm_desense);
		total->ofdm_desense_extra_halfdB = aci->ofdm_desense_extra_halfdB;

		total->bphy_desense = MAX(aci->bphy_desense, bt->bphy_desense);
		total->analog_gain_desense_ofdm = MAX(aci->analog_gain_desense_ofdm,
			bt->analog_gain_desense_ofdm);
		total->analog_gain_desense_bphy = MAX(aci->analog_gain_desense_bphy,
			bt->analog_gain_desense_bphy);
		total->lna1_tbl_desense = MAX(aci->lna1_tbl_desense, bt->lna1_tbl_desense);
		total->lna2_tbl_desense = MAX(aci->lna2_tbl_desense, bt->lna2_tbl_desense);
		total->lna1_gainlmt_desense =
		  MAX(aci->lna1_gainlmt_desense, bt->lna1_gainlmt_desense);
		total->lna2_gainlmt_desense =
		  MAX(aci->lna2_gainlmt_desense, bt->lna2_gainlmt_desense);
		total->mixer_setting_desense =
		  MAX(aci->mixer_setting_desense, bt->mixer_setting_desense);
		total->elna_bypass = MAX(aci->elna_bypass, bt->elna_bypass);
		total->nf_hit_lna12 =  MAX(aci->nf_hit_lna12, bt->nf_hit_lna12);
		total->on = aci->on | bt->on;
		for (i = 0; i < 4; i++)
			total->clipgain_desense[i] = MAX(aci->clipgain_desense[i],
				bt->clipgain_desense[i]);
	}
#endif /* BCMLTECOEX */

	/* uCode detected high pwr RSSI. Time to save ilna1 */
	if (CHSPEC_IS2G(pi->radio_chanspec) && (pi_ac->hirssi_timer2g > PHY_SW_HIRSSI_OFF))
		total->elna_bypass = TRUE;
	if (CHSPEC_IS5G(pi->radio_chanspec) && (pi_ac->hirssi_timer5g > PHY_SW_HIRSSI_OFF))
		total->elna_bypass = TRUE;
}

/********** Desnese BT  ******** */
void
wlc_phy_desense_btcoex_acphy(phy_info_t *pi, int32 mode)
{
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;
	acphy_desense_values_t *desense = pi_ac->bt_desense;
	int32 old_mode = pi_ac->btc_mode;

	if (ACPHY_ENABLE_FCBS_HWACI(pi) && !ACPHY_HWACI_WITH_DESENSE_ENG(pi))
		return;

	/* Start with everything at 0 */
	bzero(desense, sizeof(acphy_desense_values_t));
	pi_ac->btc_mode = mode;
	desense->on = (mode > 0);

	switch (mode) {
	case 1: /* BT power =  -30dBm, -35dBm */
		desense->lna1_gainlmt_desense = 1;   /* 4 */
		desense->lna2_gainlmt_desense = 3;   /* 3 */
		desense->elna_bypass = 0;
		break;
	case 2: /* BT power = -20dBm , -25dB */
		desense->lna1_gainlmt_desense = 0;   /* 5 */
		desense->lna2_gainlmt_desense = 0;   /* 6 */
		desense->elna_bypass = 1;
		break;
	case 3: /* BT power = -15dBm */
		desense->lna1_gainlmt_desense = 0;   /* 5 */
		desense->lna2_gainlmt_desense = 2;   /* 4 */
		desense->elna_bypass = 1;
		desense->nf_hit_lna12 = 2;
		break;
	case 4: /* BT power = -10dBm */
		desense->lna1_gainlmt_desense = 1;   /* 4 */
		desense->lna2_gainlmt_desense = 2;   /* 4 */
		desense->elna_bypass = 1;
		desense->nf_hit_lna12 = 3;
		break;
	case 5: /* BT power = -5dBm */
		desense->lna1_gainlmt_desense = 3;   /* 2 */
		desense->lna2_gainlmt_desense = 0;   /* 6 */
		desense->elna_bypass = 1;
		desense->nf_hit_lna12 = 13;
		break;
	case 6: /* BT power = 0dBm */
		desense->lna1_gainlmt_desense = 3;   /* 2 */
		desense->lna2_gainlmt_desense = 4;   /* 2 */
		desense->elna_bypass = 1;
		desense->nf_hit_lna12 = 24;
		break;
	case 7: /* Case added for 4359 */
		desense->lna1_tbl_desense = 3;	/* 1 */
		desense->mixer_setting_desense = 7; /* Value should be take between 2-11 for Tiny */
		break;

	default:
		break;
	}

	/* Apply these settings if this is called while on an active 2g channel */
	if (CHSPEC_IS2G(pi->radio_chanspec) && !SCAN_RM_IN_PROGRESS(pi)) {
		/* If bt desense changed, then reset aci params. But, keep the aci settings intact
		   if bt is switched off (as you will still need aci desense)
		*/
		if ((mode != old_mode) && (mode > 0))
			wlc_phy_desense_aci_reset_params_acphy(pi, FALSE, FALSE, FALSE);
		wlc_phy_desense_apply_acphy(pi, TRUE);
	}
}

/********** Desense LTE  ******** */
#ifdef BCMLTECOEX
void
wlc_phy_desense_ltecx_acphy(phy_info_t *pi, int32 mode)
{
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;
	acphy_desense_values_t *desense = pi_ac->rxgcrsi->lte_desense;

	/* Start with everything at 0 */
	bzero(desense, sizeof(acphy_desense_values_t));
	pi_ac->ltecx_mode = mode;
	if (pi_ac->ltecx_mode == 0) {
			pi->u.pi_acphy->ltecx_elna_bypass_status = 0;
		}
	desense->on = (mode > 0);
	switch (mode) {
	case 1: /* LTE - Add new cases in the future */
		desense->ofdm_desense = 24;
		desense->bphy_desense = 24;
		desense->elna_bypass = 1;
		desense->nf_hit_lna12 = 24;
		pi->u.pi_acphy->ltecx_elna_bypass_status = 1;
		break;
	default:
		break;
	}

	/* Apply these settings if this is called while on an active 2g channel */
	if (CHSPEC_IS2G(pi->radio_chanspec) && !SCAN_RM_IN_PROGRESS(pi))
		wlc_phy_desense_apply_acphy(pi, TRUE);
}
#endif /* BCMLTECOEX */

#endif /* #ifndef WLC_DISABLE_ACI */

void
wlc_phy_rxgainctrl_gainctrl_acphy(phy_info_t *pi)
{
	bool elna_present;
	bool init_trtx, hi_trtx, md_trtx, lo_trtx, lna1byp;
	uint8 init_gain = ACPHY_INIT_GAIN, hi_gain = ACPHY_HI_GAIN;
	uint8 mid_gain = 35, lo_gain, clip2_gain;
	uint8 hi_gain1, mid_gain1, lo_gain1;

	/* 1% PER point used for all the PER numbers */

	/* For bACI/ACI: max output pwrs {elna, lna1, lna2, mix, bq0, bq1, dvga} */
	uint8 maxgain_2g[] = {43, 43, 43, 52, 52, 100, 0};
	uint8 maxgain_5g[] = {47, 47, 47, 52, 52, 100, 0};

	uint8 i, core, elna_idx;
	int8 md_low_end, hi_high_end, lo_low_end, md_high_end, max_himd_hi_end;
	int8 nbclip_pwrdBm, w1clip_pwrdBm;
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;
	bool elnabyp_en = (CHSPEC_IS2G(pi->radio_chanspec) & pi_ac->hirssi_elnabyp2g_en) ||
	        (CHSPEC_IS5G(pi->radio_chanspec) & pi_ac->hirssi_elnabyp5g_en);

	pi_ac->mdgain_trtx_allowed = TRUE;
	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		for (i = 0; i < ACPHY_MAX_RX_GAIN_STAGES; i++)
			pi_ac->rxgainctrl_maxout_gains[i] = maxgain_2g[i];
		elna_present = BF_ELNA_2G(pi_ac);
	} else {
		for (i = 0; i < ACPHY_MAX_RX_GAIN_STAGES; i++)
			pi_ac->rxgainctrl_maxout_gains[i] = maxgain_5g[i];
		elna_present = BF_ELNA_5G(pi_ac);
	}

	/* Keep track of it (used in interf_mitigation) */
	pi_ac->curr_desense->elna_bypass = wlc_phy_get_max_lna_index_acphy(pi, ELNA_ID);
	init_trtx = elna_present & pi_ac->curr_desense->elna_bypass;
	hi_trtx = elna_present & pi_ac->curr_desense->elna_bypass;
	md_trtx = elna_present & (pi_ac->mdgain_trtx_allowed | pi_ac->curr_desense->elna_bypass);
	lo_trtx = TRUE;

	/* with elna if md_gain uses TR != T, then LO_gain needs to be higher */
	if (elnabyp_en)
		lo_gain = ((!elna_present) || md_trtx) ? 15 : 30;
	else
		lo_gain = ((!elna_present) || md_trtx) ? 20 : 30;
	clip2_gain = md_trtx ? lo_gain : (mid_gain + lo_gain) >> 1;

	FOREACH_CORE(pi, core) {
		lna1byp = pi_ac->fem_rxgains[core].lna1byp;
		if (lna1byp) {
			mid_gain = 35;
			lo_gain = 15;
			PHY_INFORM((" iPa chip, set lo_gain = 15 \n"));
			hi_gain = 48;
		}
		elna_idx = READ_PHYREGFLDC(pi, InitGainCodeA, core, initExtLnaIndex);
		max_himd_hi_end = - 16 - pi_ac->rxgainctrl_params[core].gaintbl[0][elna_idx];
		if (pi_ac->curr_desense->elna_bypass == 1)
			max_himd_hi_end += pi_ac->fem_rxgains[core].trloss;

		/* 0,1,2,3 for Init, hi, md and lo gains respectively */
		wlc_phy_rxgainctrl_set_init_clip_gain_acphy(pi, 0, init_gain, init_trtx,
			FALSE, core);
		hi_gain1 = wlc_phy_rxgainctrl_set_init_clip_gain_acphy(pi, 1, hi_gain,
			hi_trtx, FALSE, core);
		mid_gain1 = wlc_phy_rxgainctrl_set_init_clip_gain_acphy(pi, 2, mid_gain,
			md_trtx, FALSE, core);
		wlc_phy_rxgainctrl_set_init_clip_gain_acphy(pi, 4, clip2_gain, md_trtx,
			FALSE, core);
		lo_gain1 = wlc_phy_rxgainctrl_set_init_clip_gain_acphy(pi, 3, lo_gain,
			lo_trtx, lna1byp, core);

		/* NB_CLIP */
		md_low_end = wlc_phy_rxgainctrl_calc_low_sens_acphy(pi, mid_gain1, md_trtx,
		                                                    FALSE, core);
		hi_high_end = wlc_phy_rxgainctrl_calc_high_sens_acphy(pi, hi_gain1, hi_trtx,
		                                                      FALSE, core);
		lo_low_end = wlc_phy_rxgainctrl_calc_low_sens_acphy(pi, lo_gain1, lo_trtx,
		                                                    lna1byp, core);
		md_high_end = wlc_phy_rxgainctrl_calc_high_sens_acphy(pi, mid_gain1, md_trtx,
		                                                      FALSE, core);

		/* -1 times pwr to avoid rounding off error */
		if (CHSPEC_IS20(pi->radio_chanspec)) {
			/* use mid-point, as late clip2 is causing minor humps,
			   move nb/w1 clip to a lower pwr
			*/
			nbclip_pwrdBm = (md_low_end + hi_high_end) >> 1;
		} else {
			/* (0.4*md/lo + 0.6*hi/md) fixed point. On fading channels,
			   low-end sensitivity degrades, keep more margin there.
			*/
			nbclip_pwrdBm = (((-2*md_low_end)+(-3*hi_high_end)) * 13) >> 6;
			nbclip_pwrdBm *= -1;
			if (CHSPEC_IS80(pi->radio_chanspec) &&
			    ((ACMAJORREV_2(pi->pubpi.phy_rev) && ACMINORREV_1(pi)) ||
			     ACMAJORREV_5(pi->pubpi.phy_rev))) {
				nbclip_pwrdBm -= 4;
			}
		}

		if (CHSPEC_IS20(pi->radio_chanspec) ||
		    (ACMAJORREV_1(pi->pubpi.phy_rev) && ACMINORREV_2(pi) && !PHY_ILNA(pi))) {
			if (elnabyp_en) {
				w1clip_pwrdBm = (((-2*lo_low_end) + (-3*md_high_end)) * 13) >> 6;
				w1clip_pwrdBm *= -1;
			} else {
				w1clip_pwrdBm = (lo_low_end + md_high_end) >> 1;
			}
		} else {
			w1clip_pwrdBm = (((-2*lo_low_end) + (-3*md_high_end)) * 13) >> 6;
			w1clip_pwrdBm *= -1;
		}

		wlc_phy_rxgainctrl_nbclip_acphy(pi, core, nbclip_pwrdBm);
		wlc_phy_rxgainctrl_w1clip_acphy(pi, core, w1clip_pwrdBm);

		/* 2G/5G VHT20 hump in eLNA, WAR for 43162 */
		if (ACMAJORREV_1(pi->pubpi.phy_rev) && ACMINORREV_2(pi) &&
		     !PHY_ILNA(pi) && CHSPEC_IS20(pi->radio_chanspec) && pi->xtalfreq == 40000000) {
			if (CHSPEC_IS5G(pi->radio_chanspec))
				nbclip_pwrdBm = ((md_low_end + hi_high_end) >> 1) - 5;
			else
				nbclip_pwrdBm = ((md_low_end + hi_high_end) >> 1) - 4;

			wlc_phy_rxgainctrl_nbclip_acphy(pi, core, nbclip_pwrdBm);
		}
	}
	/* After comming out of this loop, gain ctrl is done. So, values of Init gain in the phyregs
	 * are the correct/default ones. These should be the ones that should be used in
	 * "wlc_phy_get_rxgain_acphy" function (used in rxiqest). So, we have to save the default
	 * Init gain phyregs in some variables. We assume, that both core's init gains will be same
	 * and thus save only core 0's init gain and this should be fine for single core chips too.
	 */
	pi_ac->initGain_codeA = READ_PHYREGC(pi, InitGainCodeA, 0);
	pi_ac->initGain_codeB = READ_PHYREGC(pi, InitGainCodeB, 0);
}

void
wlc_phy_upd_lna1_lna2_gains_acphy(phy_info_t *pi)
{
	wlc_phy_upd_lna1_lna2_gainlimittbls_acphy(pi, 1);
	wlc_phy_upd_lna1_lna2_gainlimittbls_acphy(pi, 2);
	wlc_phy_upd_lna1_lna2_gaintbls_acphy(pi, 1);
	wlc_phy_upd_lna1_lna2_gaintbls_acphy(pi, 2);
}

void
wlc_phy_upd_lna1_lna2_gaintbls_acphy(phy_info_t *pi, uint8 lna12)
{
	uint8 offset, sz, core;
	uint8 gaintbl[10], gainbitstbl[10];
	uint8 max_idx, min_idx, desense_state;
	const uint8 *default_gaintbl = NULL;
	uint16 gain_tblid, gainbits_tblid;
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;
	acphy_desense_values_t *desense = pi_ac->total_desense;
	uint8 lna1Rout = 0x25;
	uint8 lna2Rout = 0x44;
	uint8 lna1rout_tbl[6], lna1rout_offset;
	uint8 lna1, lna1_idx, lna1_rout;
	uint8 stall_val;

	if ((lna12 < 1) || (lna12 > 2)) return;

	stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
	ACPHY_DISABLE_STALL(pi);

	sz = pi_ac->rxgainctrl_stage_len[lna12];

	if (pi_ac->aci != NULL) {
		desense_state = pi_ac->aci->hwaci_desense_state;
	} else {
		desense_state = 0;
	}

	if (lna12 == LNA1_ID) {          /* lna1 */
		offset = 8;
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			if (!TINY_RADIO(pi)) {
				if (PHY_ILNA(pi)) { /* iLNA only chip */
					default_gaintbl = ac_lna1_2g_43352_ilna;
				} else {
					if (BF3_LTECOEX_GAINTBL_EN(pi_ac) == 1 &&
					    ACMAJORREV_1(pi->pubpi.phy_rev)) {
						default_gaintbl = ac_lna1_2g_ltecoex;
					} else {
						default_gaintbl = ac_lna1_2g;
					}
				}
			} else {
				default_gaintbl = ac_lna1_2g_tiny;
			}
		} else {
			if (ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev))
				default_gaintbl = ac_lna1_5g_4365;
			else
				default_gaintbl = (TINY_RADIO(pi)) ? ac_lna1_5g_tiny : ac_lna1_5g;
		}
	} else {  /* lna2 */
		offset = 16;
		if (TINY_RADIO(pi)) {
			if (ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev)) {
				default_gaintbl = CHSPEC_IS2G(pi->radio_chanspec) ?
				        ac_lna2_tiny_4365_2g : ac_lna2_tiny_4365_5g;
			} else if (ACMAJORREV_4(pi->pubpi.phy_rev)) {
				default_gaintbl = ac_lna2_tiny_4349;
			} else if (ACMAJORREV_3(pi->pubpi.phy_rev) &&
				ACREV_GE(pi->pubpi.phy_rev, 11) &&
				PHY_ILNA(pi)) {
				default_gaintbl = ac_lna2_tiny_ilna_dcc_comp;
			} else {
				default_gaintbl = ac_lna2_tiny;
			}
		} else if (CHSPEC_IS2G(pi->radio_chanspec)) {
			if (PHY_ILNA(pi)) { /* iLNA only chip */
				default_gaintbl = ac_lna2_2g_43352_ilna;
			} else {
				if (BF3_LTECOEX_GAINTBL_EN(pi_ac) == 1 &&
				    ACMAJORREV_1(pi->pubpi.phy_rev)) {
					default_gaintbl = ac_lna2_2g_ltecoex;
				} else {
					default_gaintbl = (desense->elna_bypass)
						? ac_lna2_2g_gm3 : ac_lna2_2g_gm2;
				}
			}
		} else {
			default_gaintbl = ac_lna2_5g;
		}
		memcpy(pi_ac->lna2_complete_gaintbl, default_gaintbl, sizeof(uint8)*sz);
	}

	max_idx = wlc_phy_get_max_lna_index_acphy(pi, lna12);
	if (ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev)) {
		if (desense_state != 0)
			min_idx = (lna12 == 1) ? desense->lna1_idx_min: desense->lna2_idx_min;
		else
		min_idx = (lna12 == 1) ? 0 : max_idx;
	} else {
		min_idx = ACMAJORREV_4(pi->pubpi.phy_rev) ? max_idx :
		        ((pi->pubpi.phy_rev == 0) ? 1 : 0);
	}

	wlc_phy_limit_rxgaintbl_acphy(gaintbl, gainbitstbl, sz, default_gaintbl, min_idx, max_idx);

	/* Update pi_ac->curr_desense (used in interf_mitigation) */
	if (lna12 == 1) {
		pi_ac->curr_desense->lna1_tbl_desense = desense->lna1_tbl_desense;
	} else {
		pi_ac->curr_desense->lna2_tbl_desense = desense->lna2_tbl_desense;
	}

	if (BF3_LTECOEX_GAINTBL_EN(pi_ac) == 1 && ACMAJORREV_1(pi->pubpi.phy_rev)) {
		/* for now changing rout of lna1 and lna2 to achieve */
		/* lower gain of 3dB for the init gain index only */
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_LNAROUT, 1, 5, 8, &lna1Rout);
		MOD_RADIO_REG(pi, RFX, OVR6, ovr_lna2g_lna1_Rout, 0);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_LNAROUT, 1, 20, 8, &lna2Rout);
		MOD_RADIO_REG(pi, RFX, OVR6, ovr_lna2g_lna2_Rout, 0);
	} else if (TINY_RADIO(pi)) {
		uint8 x;
		uint8 lnarout_val, lnarout_val2G, lnarout_val5G;

		if (ACMAJORREV_4(pi->pubpi.phy_rev) || ACMAJORREV_32(pi->pubpi.phy_rev) ||
			ACMAJORREV_33(pi->pubpi.phy_rev)) {
			FOREACH_CORE(pi, core) {
				for (x = 0; x < 6; x++) {
					lnarout_val2G = (ac_tiny_g_lna_rout_map[x] << 3) |
						ac_tiny_g_lna_gain_map[x];
					if (ACMAJORREV_32(pi->pubpi.phy_rev) ||
						ACMAJORREV_33(pi->pubpi.phy_rev))
						lnarout_val5G = (ac_4365_a_lna_rout_map[x] << 3) |
						        ac_tiny_a_lna_gain_map[x];
					else
						lnarout_val5G = (ac_tiny_a_lna_rout_map[x] << 3) |
						        ac_tiny_a_lna_gain_map[x];
					lnarout_val = CHSPEC_IS5G(pi->radio_chanspec) ?
						lnarout_val5G : lnarout_val2G;
					wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_LNAROUT, 1,
						(ACPHY_LNAROUT_BAND_OFFSET(pi,
						pi->radio_chanspec)	+ x +
						ACPHY_LNAROUT_CORE_WRT_OFST(pi->pubpi.phy_rev,
						core)),	8, &lnarout_val);
				}

				if (ACMINORREV_0(pi) || ACMINORREV_1(pi))
					MOD_PHYREGCE(pi, RfctrlCoreRXGAIN1, core, rxrf_lna2_gain,
						wlc_phy_get_max_lna_index_acphy(pi, LNA2_ID));
			}
		} else {
			/* 2G index is 0->5 */
			for (x = 0; x < 6; x++) {
				lnarout_val = (ac_tiny_g_lna_rout_map[x] << 3) |
					ac_tiny_g_lna_gain_map[x];
				wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_LNAROUT, 1, x, 8,
					&lnarout_val);
			}

			/* 5G index is 8->13 */
			for (x = 0; x < 6; x++) {
				lnarout_val =  (ac_tiny_a_lna_rout_map[x] << 3) |
					ac_tiny_a_lna_gain_map[x];
				wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_LNAROUT, 1, 8 + x, 8,
				                          &lnarout_val);
			}
		}
	}

	/* Update gaintbl */
	FOREACH_CORE(pi, core) {
		if (core == 0) {
			gain_tblid =  ACPHY_TBL_ID_GAIN0;
			gainbits_tblid =  ACPHY_TBL_ID_GAINBITS0;
		} else if (core == 1) {
			gain_tblid =  ACPHY_TBL_ID_GAIN1;
			gainbits_tblid =  ACPHY_TBL_ID_GAINBITS1;
		} else if (core == 2) {
			gain_tblid =  ACPHY_TBL_ID_GAIN2;
			gainbits_tblid =  ACPHY_TBL_ID_GAINBITS2;
		} else {
			gain_tblid =  ACPHY_TBL_ID_GAIN3;
			gainbits_tblid =  ACPHY_TBL_ID_GAINBITS3;
		}

		if (!TINY_RADIO(pi)) {
			lna1rout_offset = core * 24;
			if (CHSPEC_IS5G(pi->radio_chanspec))
				lna1rout_offset += 8;

			if (lna12 == 1) {
				uint8 i;
				wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_LNAROUT, 6,
				                         lna1rout_offset, 8, lna1rout_tbl);
				for (i = 0; i < 6; i++) {
					lna1 = lna1rout_tbl[gainbitstbl[i]];
					lna1_idx = lna1 & 0x7;
					lna1_rout = (lna1 >> 3) & 0xf;
					gaintbl[i] = CHSPEC_IS2G(pi->radio_chanspec) ?
					        default_gaintbl[lna1_idx] -
					        ac_lna1_rout_delta_2g[lna1_rout]:
					        default_gaintbl[lna1_idx] -
					        ac_lna1_rout_delta_5g[lna1_rout];
				}
			}
		}

		memcpy(pi_ac->rxgainctrl_params[core].gaintbl[lna12], gaintbl, sizeof(uint8)*sz);
		wlc_phy_table_write_acphy(pi, gain_tblid, sz, offset, 8, gaintbl);
		memcpy(pi_ac->rxgainctrl_params[core].gainbitstbl[lna12], gainbitstbl,
			sizeof(uint8)*sz);
		wlc_phy_table_write_acphy(pi, gainbits_tblid, sz, offset, 8, gainbitstbl);
	}

	ACPHY_ENABLE_STALL(pi, stall_val);
}

void
wlc_phy_upd_lna1_lna2_gainlimittbls_acphy(phy_info_t *pi, uint8 lna12)
{
	uint8 i, sz, max_idx, core, lna2_limit_size, lna1_rout, lna1rout_tbl[5];
	uint8 max_idx_pktgain_limit, max_idx_non_init_lna1rout_tbl, lna1rout_offset;
	uint8 lna1_tbl[] = {11, 12, 14, 32, 36, 40};
	uint8 lna2_tbl[] = {0, 0, 0, 3, 3, 3, 3};
	uint8 lna2_gainlmt_tiny[] = {127, 127, 127, 127};
	uint8 tia_gainlmt_tiny[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 127};
	uint8 lna2_gainlmt_4365[] = {0, 0, 0, 0, 0, 0, 0};
	uint8 tia_gainlmt_4365[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127};
	uint8 *tiny_gainlmt_lna2, *tiny_gainlmt_tia;
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;
	acphy_desense_values_t *desense = pi_ac->total_desense;
	uint16 gainlimitid;

	if (!TINY_RADIO(pi)) {
		sz = (lna12 == 1) ? 6 : 7;

		/* Limit based on desense mitigation mode */
		if (lna12 == 1) {
			max_idx = MAX(0, (sz - 1) - desense->lna1_gainlmt_desense);
			pi_ac->curr_desense->lna1_gainlmt_desense = desense->lna1_gainlmt_desense;

			max_idx_pktgain_limit = max_idx;
			/* if rout desense is active, we have to change the non-init gain entries of
			 * lna1_rout tbl and modify pkt gain limit tbl to not use init-gain instead
			 * of just limiting LNA1 gain in the pkt gain limit tbls
			 */
			max_idx_non_init_lna1rout_tbl = (desense->lna1rout_gainlmt_desense > 0) ?
			        max_idx : (sz - 2);

			/* LNA1 gain in pktgain limit tbl has to be capped
			   only to not use init gain
			*/
			max_idx_pktgain_limit = (desense->lna1rout_gainlmt_desense > 0) ?
			        (sz - 2) : max_idx;
			lna1_rout = CHSPEC_IS2G(pi->radio_chanspec) ?
			        0 + desense->lna1rout_gainlmt_desense :
			        4 - desense->lna1rout_gainlmt_desense;
			for (i = 0; i <= sz - 2; i++) {
				lna1rout_tbl[i] = (lna1_rout << 3) |
				        MAX(0, max_idx_non_init_lna1rout_tbl - (sz - 2 - i));
			}

			if (!(ACREV_IS(pi->pubpi.phy_rev, 0) || TINY_RADIO(pi))) {
				/* WRITE the lna1 rout table (only first 5 entries) */
				FOREACH_CORE(pi, core) {
					lna1rout_offset = core * 24;
					if (CHSPEC_IS5G(pi->radio_chanspec))
						lna1rout_offset += 8;
					wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_LNAROUT, sz-1,
					                          lna1rout_offset, 8, lna1rout_tbl);
				}
			}
		} else {
			max_idx_pktgain_limit = MAX(0, (sz - 1) - desense->lna2_gainlmt_desense);
			pi_ac->curr_desense->lna2_gainlmt_desense = desense->lna2_gainlmt_desense;
		}

		/* Write 0x7f to entries not to be used */
		for (i = (max_idx_pktgain_limit + 1); i < sz; i++) {
			if (lna12 == 1) {
				lna1_tbl[i] = 0x7f;
			} else {
				lna2_tbl[i] = 0x7f;
			}
		}

		if (lna12 == 1) {
		    if (ACMAJORREV_5(pi->pubpi.phy_rev)) {
				wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_GAINLIMIT0, sz,
					8, 8,  lna1_tbl);
				wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_GAINLIMIT1, sz,
					8, 8,  lna1_tbl);
				wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_GAINLIMIT2, sz,
					8, 8,  lna1_tbl);
			} else
				wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_GAINLIMIT, sz,
					8, 8, lna1_tbl);

			/* 4335C0: This is for DSSS_CCK packet gain limit */
			if (ACMAJORREV_1(pi->pubpi.phy_rev) && ACMINORREV_2(pi))
				wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_GAINLIMIT, sz, 72, 8,
				                          lna1_tbl);
		}
		else {
			if (ACMAJORREV_5(pi->pubpi.phy_rev)) {
				wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_GAINLIMIT0, sz,
					16, 8, lna2_tbl);
				wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_GAINLIMIT1, sz,
					16, 8, lna2_tbl);
				wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_GAINLIMIT2, sz,
					16, 8, lna2_tbl);
			} else
				wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_GAINLIMIT, sz,
					16, 8, lna2_tbl);

			/* 4335C0: This is for DSSS_CCK packet gain limit */
			if (ACMAJORREV_1(pi->pubpi.phy_rev) && ACMINORREV_2(pi))
				wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_GAINLIMIT, sz,
					80, 8, lna2_tbl);
		}
	} else {
		tiny_gainlmt_lna2 = lna2_gainlmt_tiny;
		tiny_gainlmt_tia = tia_gainlmt_tiny;

		FOREACH_CORE(pi, core) {
			if (ACMAJORREV_4(pi->pubpi.phy_rev)) {
				gainlimitid = (core) ? ACPHY_TBL_ID_GAINLIMIT1
					: ACPHY_TBL_ID_GAINLIMIT0;
				/* get max lna2 indx */
				lna2_limit_size = wlc_phy_get_max_lna_index_acphy(pi, LNA2_ID) + 1;
			} else if (ACMAJORREV_32(pi->pubpi.phy_rev) ||
				ACMAJORREV_33(pi->pubpi.phy_rev)) {
				/* Use different for 4365 */
				tiny_gainlmt_lna2 = lna2_gainlmt_4365;
				tiny_gainlmt_tia = tia_gainlmt_4365;
				if (core == 0)
					gainlimitid = ACPHY_TBL_ID_GAINLIMIT0;
				else if (core == 1)
					gainlimitid = ACPHY_TBL_ID_GAINLIMIT1;
				else if (core == 2)
					gainlimitid = ACPHY_TBL_ID_GAINLIMIT2;
				else /* (core == 3) */
					gainlimitid = ACPHY_TBL_ID_GAINLIMIT3;
				/* get max lna2 indx */
				lna2_limit_size = wlc_phy_get_max_lna_index_acphy(pi, LNA2_ID) + 1;
				lna2_limit_size = 7;
			} else {
				gainlimitid = ACPHY_TBL_ID_GAINLIMIT;
				lna2_limit_size = 2;
			}
			wlc_phy_table_write_acphy(pi, gainlimitid, lna2_limit_size,
				16, 8, tiny_gainlmt_lna2);
			wlc_phy_table_write_acphy(pi, gainlimitid, lna2_limit_size,
				16+64, 8, tiny_gainlmt_lna2);
			wlc_phy_table_write_acphy(pi, gainlimitid, 13, 32,    8, tiny_gainlmt_tia);
			wlc_phy_table_write_acphy(pi, gainlimitid, 13, 32+64, 8, tiny_gainlmt_tia);
		}
	}
}

void wlc_phy_rfctrl_override_rxgain_acphy(phy_info_t *pi, uint8 restore,
                                           rxgain_t rxgain[], rxgain_ovrd_t rxgain_ovrd[])
{
	uint8 core, lna1_Rout, lna2_Rout;
	uint16 reg_rxgain, reg_rxgain2, reg_lpfgain;
	uint8 stall_val;
	uint8 lna1_gm;
	uint8 offset;

	if (restore == 1) {
		/* restore the stored values */
		FOREACH_CORE(pi, core) {
			WRITE_PHYREGCE(pi, RfctrlOverrideGains, core, rxgain_ovrd[core].rfctrlovrd);
			WRITE_PHYREGCE(pi, RfctrlCoreRXGAIN1, core, rxgain_ovrd[core].rxgain);
			WRITE_PHYREGCE(pi, RfctrlCoreRXGAIN2, core, rxgain_ovrd[core].rxgain2);
			WRITE_PHYREGCE(pi, RfctrlCoreLpfGain, core, rxgain_ovrd[core].lpfgain);
			PHY_INFORM(("%s, Restoring RfctrlOverride(rxgain) values\n", __FUNCTION__));
		}
	} else {
		stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
		ACPHY_DISABLE_STALL(pi);
		FOREACH_CORE(pi, core) {
			/* Save the original values */
			rxgain_ovrd[core].rfctrlovrd = READ_PHYREGCE(pi, RfctrlOverrideGains, core);
			rxgain_ovrd[core].rxgain = READ_PHYREGCE(pi, RfctrlCoreRXGAIN1, core);
			rxgain_ovrd[core].rxgain2 = READ_PHYREGCE(pi, RfctrlCoreRXGAIN2, core);
			rxgain_ovrd[core].lpfgain = READ_PHYREGCE(pi, RfctrlCoreLpfGain, core);

			offset = TINY_RADIO(pi) ? (rxgain[core].lna1 +
				ACPHY_LNAROUT_BAND_OFFSET(pi, pi->radio_chanspec)) :
				(5 + ACPHY_LNAROUT_BAND_OFFSET(pi, pi->radio_chanspec));

			wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_LNAROUT,
				1, offset + ACPHY_LNAROUT_CORE_RD_OFST(pi, core),
				8, &lna1_Rout);
			/* LnaRoutLUT WAR for 4349A2 */
			if ((ACMAJORREV_4((pi)->pubpi.phy_rev)) && (ACMINORREV_1(pi))) {
				wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_LNAROUT,
					1, 0 + ACPHY_LNAROUT_CORE_RD_OFST(pi, core),
					8, &lna2_Rout);
			} else {
				wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_LNAROUT,
					1, 22 + ACPHY_LNAROUT_CORE_RD_OFST(pi, core),
					8, &lna2_Rout);
			}

			/* Write the rxgain override registers */
			lna1_gm = TINY_RADIO(pi) ? (lna1_Rout & 0x7) : rxgain[core].lna1;

			WRITE_PHYREGCE(pi, RfctrlCoreRXGAIN1, core,
			              (rxgain[core].dvga << 10) | (rxgain[core].mix << 6) |
			              (rxgain[core].lna2 << 3) | lna1_gm);

			WRITE_PHYREGCE(pi, RfctrlCoreRXGAIN2, core,
			              (((lna2_Rout >> 3) & 0xf) << 4 | ((lna1_Rout >> 3) & 0xf)));
			WRITE_PHYREGCE(pi, RfctrlCoreLpfGain, core,
			              (rxgain[core].lpf1 << 3) | rxgain[core].lpf0);

			MOD_PHYREGCE(pi, RfctrlOverrideGains, core, rxgain, 1);
			MOD_PHYREGCE(pi, RfctrlOverrideGains, core, lpf_bq1_gain, 1);
			MOD_PHYREGCE(pi, RfctrlOverrideGains, core, lpf_bq2_gain, 1);

			reg_rxgain = READ_PHYREGCE(pi, RfctrlCoreRXGAIN1, core);
			reg_rxgain2 = READ_PHYREGCE(pi, RfctrlCoreRXGAIN2, core);
			reg_lpfgain = READ_PHYREGCE(pi, RfctrlCoreLpfGain, core);
			PHY_INFORM(("%s, core %d. rxgain_ovrd = 0x%x, lpf_ovrd = 0x%x\n",
			            __FUNCTION__, core, reg_rxgain, reg_lpfgain));
			PHY_INFORM(("%s, core %d. rxgain_rout_ovrd = 0x%x\n",
			            __FUNCTION__, core, reg_rxgain2));
			BCM_REFERENCE(reg_rxgain);
			BCM_REFERENCE(reg_rxgain2);
			BCM_REFERENCE(reg_lpfgain);
		}
		ACPHY_ENABLE_STALL(pi, stall_val);
	}
}

uint8 wlc_phy_calc_extra_init_gain_acphy(phy_info_t *pi, uint8 extra_gain_3dB,
	rxgain_t rxgain[])
{
	uint16 init_gain_code[4];
	uint8 core, MAX_DVGA, MAX_LPF, MAX_MIX;
	uint8 dvga, mix, lpf0, lpf1;
	uint8 dvga_inc, lpf0_inc, lpf1_inc;
	uint8 max_inc, gain_ticks = extra_gain_3dB;

	MAX_DVGA = 4; MAX_LPF = 10; MAX_MIX = 4;
	wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 3, 0xf9, 16, &init_gain_code);

	/* Find if the requested gain increase is possible */
	FOREACH_CORE(pi, core) {
		dvga = 0;
		mix = (init_gain_code[core] >> 6) & 0xf;
		lpf0 = (init_gain_code[core] >> 10) & 0x7;
		lpf1 = (init_gain_code[core] >> 13) & 0x7;
		max_inc = MAX(0, MAX_DVGA - dvga) + MAX(0, MAX_LPF - lpf0 - lpf1) +
		        MAX(0, MAX_MIX - mix);
		gain_ticks = MIN(gain_ticks, max_inc);
	}
	if (gain_ticks != extra_gain_3dB) {
		PHY_INFORM(("%s: Unable to find enough extra gain. Using extra_gain = %d\n",
		            __FUNCTION__, 3 * gain_ticks));
	}
		/* Do nothing if no gain increase is required/possible */
	if (gain_ticks == 0) {
		return gain_ticks;
	}
	/* Find the mix, lpf0, lpf1 gains required for extra INITgain */
	FOREACH_CORE(pi, core) {
		uint8 gain_inc = gain_ticks;
		dvga = 0;
		mix = (init_gain_code[core] >> 6) & 0xf;
		lpf0 = (init_gain_code[core] >> 10) & 0x7;
		lpf1 = (init_gain_code[core] >> 13) & 0x7;
		dvga_inc = MIN((uint8) MAX(0, MAX_DVGA - dvga), gain_inc);
		dvga += dvga_inc;
		gain_inc -= dvga_inc;
		lpf1_inc = MIN((uint8) MAX(0, MAX_LPF - lpf1 - lpf0), gain_inc);
		lpf1 += lpf1_inc;
		gain_inc -= lpf1_inc;
		lpf0_inc = MIN((uint8) MAX(0, MAX_LPF - lpf1 - lpf0), gain_inc);
		lpf0 += lpf0_inc;
		gain_inc -= lpf0_inc;
		mix += MIN((uint8) MAX(0, MAX_MIX - mix), gain_inc);
		rxgain[core].lna1 = init_gain_code[core] & 0x7;
		rxgain[core].lna2 = (init_gain_code[core] >> 3) & 0x7;
		rxgain[core].mix  = mix;
		rxgain[core].lpf0 = lpf0;
		rxgain[core].lpf1 = lpf1;
		rxgain[core].dvga = dvga;
	}
	return gain_ticks;
}

/* ************************************* */
/*		Carrier Sense related definitions		*/
/* ************************************* */
static void wlc_phy_ofdm_crs_acphy(phy_info_t *pi, bool enable);
static void wlc_phy_clip_det_acphy(phy_info_t *pi, bool enable);

static void
wlc_phy_ofdm_crs_acphy(phy_info_t *pi, bool enable)
{
	uint8 core;
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	/* MAC should be suspended before calling this function */
	ASSERT((R_REG(pi->sh->osh, &pi->regs->maccontrol) & MCTL_EN_MAC) == 0);

	if (enable) {
		if (ACREV_GE(pi->pubpi.phy_rev, 32)) {
		  FOREACH_CORE(pi, core) {
		    MOD_PHYREGCE(pi, crsControlu, core, totEnable, 1);
		    MOD_PHYREGCE(pi, crsControll, core, totEnable, 1);
		    MOD_PHYREGCE(pi, crsControluSub1, core, totEnable, 1);
		    MOD_PHYREGCE(pi, crsControllSub1, core, totEnable, 1);
		  }
		} else {
			MOD_PHYREG(pi, crsControlu, totEnable, 1);
			MOD_PHYREG(pi, crsControll, totEnable, 1);
			MOD_PHYREG(pi, crsControluSub1, totEnable, 1);
			MOD_PHYREG(pi, crsControllSub1, totEnable, 1);
		}
	} else {
		if (ACREV_GE(pi->pubpi.phy_rev, 32)) {
		  FOREACH_CORE(pi, core) {
		    MOD_PHYREGCE(pi, crsControlu, core, totEnable, 0);
		    MOD_PHYREGCE(pi, crsControll, core, totEnable, 0);
		    MOD_PHYREGCE(pi, crsControluSub1, core, totEnable, 0);
		    MOD_PHYREGCE(pi, crsControllSub1, core, totEnable, 0);
		  }
		} else {
			MOD_PHYREG(pi, crsControlu, totEnable, 0);
			MOD_PHYREG(pi, crsControll, totEnable, 0);
			MOD_PHYREG(pi, crsControluSub1, totEnable, 0);
			MOD_PHYREG(pi, crsControllSub1, totEnable, 0);
		}
	}
}

static void
wlc_phy_clip_det_acphy(phy_info_t *pi, bool enable)
{
	uint8 core;
	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;

	/* Make clip detection difficult (impossible?) */
	/* don't change this loop to active core loop, gives 100% per, why? */
	FOREACH_CORE(pi, core) {
		if (ACREV_IS(pi->pubpi.phy_rev, 0)) {
			if (enable) {
				WRITE_PHYREGC(pi, Clip1Threshold, core,
					pi_ac->clip1_th);
			} else {
				WRITE_PHYREGC(pi, Clip1Threshold, core, 0xffff);
			}
		} else {
			if (enable) {
				phy_utils_and_phyreg(pi, ACPHYREGC(pi, computeGainInfo, core),
					(uint16)~ACPHY_REG_FIELD_MASK(pi, computeGainInfo, core,
					disableClip1detect));
			} else {
				phy_utils_or_phyreg(pi, ACPHYREGC(pi, computeGainInfo, core),
					ACPHY_REG_FIELD_MASK(pi, computeGainInfo, core,
					disableClip1detect));
			}
		}
	}

}

void wlc_phy_force_crsmin_acphy(phy_info_t *pi, void *p)
{
	int8 *set_crs_thr = p;
	/* Local copy of phyrxchains & EnTx bits before overwrite */
	uint8 enRx = 0;
	uint8 enTx = 0;
	int8 zeros[] = {0, 0, 0, 0};
	/* Prepare Mac and Phregs */
	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	phy_utils_phyreg_enter(pi);

	/* Save and overwrite Rx chains */
	wlc_phy_update_rxchains((wlc_phy_t *)pi, &enRx, &enTx);

	if (set_crs_thr[0] == -1) {
		/* Auto crsmin power mode */
		PHY_CAL(("Setting auto crsmin power mode\n"));
		wlc_phy_noise_sample_request_crsmincal((wlc_phy_t*)pi);
		pi->u.pi_acphy->crsmincal_enable = TRUE;
	} else if (set_crs_thr[0] == 0) {
		/* Default crsmin value */
		PHY_CAL(("Setting default crsmin: %d\n", ACPHY_CRSMIN_DEFAULT));
		wlc_phy_set_crs_min_pwr_acphy(pi, ACPHY_CRSMIN_DEFAULT, zeros);
		pi->u.pi_acphy->crsmincal_enable = FALSE;
	} else {
		/* Set the crsmin power value to be 'set_crs_thr' */
		PHY_CAL(("Setting crsmin: %d %d %d %d\n",
			set_crs_thr[0], set_crs_thr[1], set_crs_thr[2], set_crs_thr[3]));
		wlc_phy_set_crs_min_pwr_acphy(pi, set_crs_thr[0], set_crs_thr);
		pi->u.pi_acphy->crsmincal_enable = FALSE;
	}

	/* Restore Rx chains */
	wlc_phy_restore_rxchains((wlc_phy_t *)pi, enRx, enTx);

	/* Enable mac */
	phy_utils_phyreg_exit(pi);
	wlapi_enable_mac(pi->sh->physhim);
}

void wlc_phy_force_lesiscale_acphy(phy_info_t *pi, void *p)
{
	int8  *set_lesi_scale = p;
	/* Local copy of phyrxchains & EnTx bits before overwrite */
	uint8 enRx = 0;
	uint8 enTx = 0;
	int8 zerodBs[] = {64, 64, 64, 64};
	/* Prepare Mac and Phregs */
	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	phy_utils_phyreg_enter(pi);

	/* Save and overwrite Rx chains */
	wlc_phy_update_rxchains((wlc_phy_t *)pi, &enRx, &enTx);

	if (set_lesi_scale[0] == -1) {
		/* Auto crsmin power mode */
		PHY_CAL(("Setting auto crsmin power mode\n"));
		wlc_phy_noise_sample_request_crsmincal((wlc_phy_t*)pi);
		pi->u.pi_acphy->lesiscalecal_enable = TRUE;
	} else if (set_lesi_scale[0] == 0) {
		/* Default crsmin value */
		PHY_CAL(("Setting default crsmin: %d\n", ACPHY_LESISCALE_DEFAULT));
		wlc_phy_set_lesiscale_acphy(pi, zerodBs);
		pi->u.pi_acphy->lesiscalecal_enable = FALSE;
	} else {
		/* Set the crsmin power value to be 'set_crs_thr' */
		printf("Setting LESI scale: %d %d %d %d\n",
			set_lesi_scale[0], set_lesi_scale[1], set_lesi_scale[2], set_lesi_scale[3]);
		wlc_phy_set_lesiscale_acphy(pi, set_lesi_scale);
		pi->u.pi_acphy->lesiscalecal_enable = FALSE;
	}

	/* Restore Rx chains */
	wlc_phy_restore_rxchains((wlc_phy_t *)pi, enRx, enTx);

	/* Enable mac */
	phy_utils_phyreg_exit(pi);
	wlapi_enable_mac(pi->sh->physhim);
}

void
wlc_phy_crs_min_pwr_cal_acphy(phy_info_t *pi, uint8 crsmin_cal_mode)
{
	int8   cmplx_pwr_dbm[PHY_CORE_MAX], cnt, tmp_diff, cache_up_th = 2;
	int8   offset[PHY_CORE_MAX];
	int8   thresh_20[] = {39, 42, 45, 48, 51, 53, 54, 57, 60, 63,
			      66, 68, 70, 72, 75, 78, 80, 83, 86, 90}; /* idx 0 --> -36 dBm */
	int8   thresh_40[] = {41, 44, 46, 48, 50, 52, 54, 56, 58, 60,
			      63, 66, 69, 71, 74, 76, 79, 82, 86, 89}; /* idx 0 --> -34 dBm */
	int8   thresh_80[] = {41, 44, 46, 48, 50, 52, 55, 57, 60, 63,
			      65, 68, 70, 72, 74, 77, 80, 84, 87, 90}; /* idx 0 --> -30 dBm */
#ifdef WL11ULB
	int8   thresh_ULB[] = {33, 36, 39, 42, 45, 48, 51, 53, 54, 57,
			60, 63, 66, 68, 70, 72, 75, 78, 80, 83, 86, 90}; /* idx 0 --> -38 dBm */
#endif /* WL11ULB */
	int8   scale_lesi[] = {64, 57, 50, 45, 40, 35, 32, 28, 25, 22,
		20, 18, 16, 14, 12, 11, 10,  9,  8,  7,  6,  5,  5};
	uint8  thresh_sz = 15;	/* i.e. not the full size of the array */
	uint8  thresh_sz_lesi = 18;	/* i.e. not the full size of the array */
	uint8  i;
	uint8  fp[PHY_CORE_NUM_4];
	int8   lesi_scale[PHY_CORE_MAX];
	int8   noises_core[PHY_SIZE_NOISE_ARRAY];
	uint8  chan_freq_range, run_cal = 0, abs_diff_cache_curr = 0;
	uint8  dvga;
	uint8  min_of_all_cores = 54, max_desense, max_fp = 255;
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;

	struct Tidxlist {
		int8 idx;
		int8 core;
	} idxlists[PHY_CORE_MAX];

	struct Tidxlist_lesi {
		int8 idx;
		int8 core;
	} idxlists_lesi[PHY_CORE_MAX];

	uint core_count = 0;

	/* Local copy of phyrxchains & EnTx bits before overwrite */
	uint8 enRx = 0;
	uint8 enTx = 0;
	uint8 chans[NUM_CHANS_IN_CHAN_BONDING] = {0, 0};

	bzero((uint8 *)cmplx_pwr_dbm, sizeof(cmplx_pwr_dbm));
	bzero((uint8 *)fp, sizeof(fp));

	/* Get weakest rssi, and find maximum crsmin allowed based on rssi */
	if (pi_ac->aci != NULL) {
		if (pi_ac->aci->weakest_rssi != 0) {
			max_desense = MAX(0, pi_ac->aci->weakest_rssi + 87);
			// 8 ticks is 3dBs. 8/3 = 43/16
			max_fp = MIN(max_fp, ACPHY_CRSMIN_DEFAULT + ((max_desense*43)>>4));
		}
	}

	/* Increase crsmin by 1.5dB for 4360/43602. Helps with Zero-pkt-loss (less fall triggers) */
	if (ACMAJORREV_0(pi->pubpi.phy_rev) || ACMAJORREV_5(pi->pubpi.phy_rev)) {
		for (i = 0; i < thresh_sz; i++) {
			thresh_20[i] += 4;
			thresh_40[i] += 4;
			thresh_80[i] += 4;
		}
	} else if (ACMAJORREV_3(pi->pubpi.phy_rev) && pi->u.pi_acphy->rxgcrsi != NULL &&
	           pi->u.pi_acphy->rxgcrsi->thresh_noise_cal) {
		thresh_sz = sizeof(thresh_20);
	}

	/* Initialize */
	fp[0] = 0x36;

	/* Save and overwrite Rx chains */
	wlc_phy_update_rxchains((wlc_phy_t *)pi, &enRx, &enTx);

	/* check the freq range of the current channel */
	/* 2G, 5GL, 5GM, 5GH, 5GX */
	if (ACMAJORREV_33(pi->pubpi.phy_rev) && PHY_AS_80P80(pi, pi->radio_chanspec)) {
		wlc_phy_get_chan_freq_range_80p80_acphy(pi, pi->radio_chanspec, chans);
		chan_freq_range = chans[0]; // FIXME - core0/1: chans[0], core2/3 chans[1]
	} else {
		chan_freq_range = wlc_phy_get_chan_freq_range_acphy(pi, 0);
	}
	ASSERT(chan_freq_range < PHY_SIZE_NOISE_CACHE_ARRAY);

	/* upate the noise pwr array with most recent noise pwr */
	if (crsmin_cal_mode == PHY_CRS_RUN_AUTO) {
		FOREACH_ACTV_CORE(pi, pi->sh->phyrxchain, i)  {
			pi->u.pi_acphy->phy_noise_pwr_array
					[pi->u.pi_acphy->phy_noise_counter][i]
					=  pi->u.pi_acphy->phy_noise_all_core[i]+1;
		}
		pi->u.pi_acphy->phy_noise_counter++;
		pi->u.pi_acphy->phy_noise_counter
				= pi->u.pi_acphy->phy_noise_counter%(PHY_SIZE_NOISE_ARRAY);
	}

	FOREACH_ACTV_CORE(pi, pi->sh->phyrxchain, i)  {
		if (crsmin_cal_mode == PHY_CRS_RUN_AUTO) {
			/* noises_core would be change in get_avg_noisepwr, so be careful */
			for (cnt = 0; cnt < PHY_SIZE_NOISE_ARRAY; cnt++) {
				noises_core[cnt] = pi->u.pi_acphy->phy_noise_pwr_array[cnt][i];
			}

			/* Get avg noise value discarding few max values */
			cmplx_pwr_dbm[i] =
			        wlc_phy_get_avg_noisepwr_acphy(noises_core);

			if (cmplx_pwr_dbm[i] == 0) {
				/* Restore Rx chains */
				wlc_phy_restore_rxchains((wlc_phy_t *)pi, enRx, enTx);
#ifdef WL_PSMX
				/* trigger MU resound due to the resetcca
				 * triggered by updating rxchains
				 */
				if (D11REV_IS(pi->sh->corerev, 64)) {
					OR_REG(pi->sh->osh, &pi->regs->maccommand_x, MCMDX_SND);
				}
#endif /* WL_PSMX */
				return;
			}

			/* cache_update threshold: 2 4335 and 4350, and 3 for 4360 */
			if (ACMAJORREV_0(pi->pubpi.phy_rev))
				cache_up_th = 3;

			/* update noisecal_cache with valid noise power */
			/* check if the new noise pwr reading is same as cache */
			tmp_diff = 0;
			if (CHSPEC_IS20(pi->radio_chanspec)) {
				tmp_diff = pi->u.pi_acphy->phy_noise_cache_crsmin
				        [chan_freq_range][i] - cmplx_pwr_dbm[i];
			} else if (CHSPEC_IS40(pi->radio_chanspec)) {
				tmp_diff = (pi->u.pi_acphy->phy_noise_cache_crsmin
				            [chan_freq_range][i] + 3) - cmplx_pwr_dbm[i];
			} else {
				tmp_diff = (pi->u.pi_acphy->phy_noise_cache_crsmin
				            [chan_freq_range][i]+7) - cmplx_pwr_dbm[i];
			}

			abs_diff_cache_curr = tmp_diff < 0 ? (0 - tmp_diff) : tmp_diff;

			/* run crscal with the current noise pwr if the call comes from phy_cals */
			if (abs_diff_cache_curr >= cache_up_th ||
				pi->u.pi_acphy->force_crsmincal)  {
				run_cal++;
				if (CHSPEC_IS20(pi->radio_chanspec)) {
					pi->u.pi_acphy->phy_noise_cache_crsmin[chan_freq_range][i]
					        = cmplx_pwr_dbm[i];
				} else if (CHSPEC_IS40(pi->radio_chanspec)) {
					pi->u.pi_acphy->phy_noise_cache_crsmin[chan_freq_range][i] =
					        cmplx_pwr_dbm[i] - 3;
				} else {
					pi->u.pi_acphy->phy_noise_cache_crsmin[chan_freq_range][i]
					        = cmplx_pwr_dbm[i] - 7;
				}
			}
		} else {
			/* Enter here only from chan_change */

			/* During chan_change, read back the noise pwr from cache */
			if (CHSPEC_IS20(pi->radio_chanspec)) {
				cmplx_pwr_dbm[i] = pi->u.pi_acphy->phy_noise_cache_crsmin
				        [chan_freq_range][i];
			} else if (CHSPEC_IS40(pi->radio_chanspec)) {
				cmplx_pwr_dbm[i] = pi->u.pi_acphy->phy_noise_cache_crsmin
				        [chan_freq_range][i] + 3;
			} else {
				cmplx_pwr_dbm[i] = pi->u.pi_acphy->phy_noise_cache_crsmin
				        [chan_freq_range][i] + 7;
			}
		}

		dvga = READ_PHYREGFLDC(pi, InitGainCodeB, i, initvgagainIndex);
		/* get the index number for crs_th table */
		if (CHSPEC_IS20(pi->radio_chanspec)) {
			idxlists[core_count].idx = cmplx_pwr_dbm[i] + 36;
			idxlists_lesi[core_count].idx = cmplx_pwr_dbm[i] - 3*dvga + 40;
		} else if (CHSPEC_IS40(pi->radio_chanspec)) {
			idxlists[core_count].idx = cmplx_pwr_dbm[i] + 34;
			idxlists_lesi[core_count].idx = cmplx_pwr_dbm[i] - 3*dvga + 37;
		} else {
			idxlists[core_count].idx = cmplx_pwr_dbm[i] + 30;
			idxlists_lesi[core_count].idx = cmplx_pwr_dbm[i] - 3*dvga + 34;
		}

#ifdef WL11ULB
		if (CHSPEC_IS10(pi->radio_chanspec) || CHSPEC_IS5(pi->radio_chanspec) ||
			CHSPEC_IS2P5(pi->radio_chanspec)) {
			idxlists[core_count].idx = cmplx_pwr_dbm[i] + 38;
		}
#endif /* WL11ULB */

		PHY_CAL(("%s: cmplx_pwr (%d) =======  %d\n", __FUNCTION__, i, cmplx_pwr_dbm[i]));

		/* out of bound */
		if ((idxlists[core_count].idx < 0) ||
		    (idxlists[core_count].idx > (thresh_sz - 1))) {
			idxlists[core_count].idx = idxlists[core_count].idx < 0 ?
			        0 : (thresh_sz - 1);
		}
		if ((idxlists_lesi[core_count].idx < 0) ||
		    (idxlists[core_count].idx > (thresh_sz_lesi - 1))) {
			idxlists_lesi[core_count].idx = idxlists_lesi[core_count].idx < 0 ?
			        0 : (thresh_sz_lesi - 1);
		}
		idxlists[core_count].core = i;
		idxlists_lesi[core_count].core = i;
		if (ISSIM_ENAB(pi->sh->sih)) {
			fp[i] = fp[0];
		} else {
			if (CHSPEC_IS20(pi->radio_chanspec)) {
				fp[i] = thresh_20[idxlists[core_count].idx];
			} else if (CHSPEC_IS40(pi->radio_chanspec)) {
				fp[i] = thresh_40[idxlists[core_count].idx];
			} else if (CHSPEC_IS80(pi->radio_chanspec) ||
					PHY_AS_80P80(pi, pi->radio_chanspec)) {
				fp[i] = thresh_80[idxlists[core_count].idx];
			} else if (CHSPEC_IS160(pi->radio_chanspec)) {
				ASSERT(0);
			}
		}
#ifdef WL11ULB
		if (CHSPEC_IS10(pi->radio_chanspec) || CHSPEC_IS5(pi->radio_chanspec) ||
			CHSPEC_IS2P5(pi->radio_chanspec)) {
			fp[i] = thresh_ULB[idxlists[core_count].idx];
		}
#endif /* WL11ULB */

		/* Limit fp based on weakest rssi */
		fp[i] = MIN(max_fp, fp[i]);

		if (i == 0)
			min_of_all_cores = fp[0];
		else
			min_of_all_cores = MIN(min_of_all_cores, fp[i]);

		lesi_scale[i] = scale_lesi[idxlists_lesi[core_count].idx];
		++core_count;
	}

	FOREACH_ACTV_CORE(pi, pi->sh->phyrxchain, i)  {
		offset[i] = fp[i] - min_of_all_cores;
	}

	fp[0] = min_of_all_cores;

	/* check if current noise pwr is different from the one in cache */
	if ((run_cal == 0) && (crsmin_cal_mode == PHY_CRS_RUN_AUTO)) {
		/* Restore Rx chains */
		wlc_phy_restore_rxchains((wlc_phy_t *)pi, enRx, enTx);
#ifdef WL_PSMX
		/* trigger MU resound due to the resetcca triggered by updating rxchains */
		if (D11REV_IS(pi->sh->corerev, 64)) {
			OR_REG(pi->sh->osh, &pi->regs->maccommand_x, MCMDX_SND);
		}
#endif /* WL_PSMX */
		return;
	}

	/* Below flag is set from phy_cals only */
	if (crsmin_cal_mode == PHY_CRS_RUN_AUTO) {
		pi->u.pi_acphy->force_crsmincal = FALSE;
	}

	pi->u.pi_acphy->crsmincal_run = 1;

	/* if noise desense is on, then the below variable will be used for comparison */
	pi->u.pi_acphy->phy_crs_th_from_crs_cal = MAX(MAX(fp[0], fp[1]), fp[2]);

	if (!ACPHY_ENABLE_FCBS_HWACI(pi) || ACPHY_HWACI_WITH_DESENSE_ENG(pi)) {
		/* if desense is forced, then reset the variable below to default */
		if (pi->u.pi_acphy->total_desense->forced) {
			pi->u.pi_acphy->phy_crs_th_from_crs_cal = ACPHY_CRSMIN_DEFAULT;
			pi->u.pi_acphy->crsmincal_run = 2;

			/* Restore Rx chains */
			wlc_phy_restore_rxchains((wlc_phy_t *)pi, enRx, enTx);
#ifdef WL_PSMX
			/* trigger MU resound due to the resetcca triggered by updating rxchains */
			if (D11REV_IS(pi->sh->corerev, 64)) {
				OR_REG(pi->sh->osh, &pi->regs->maccommand_x, MCMDX_SND);
			}
#endif /* WL_PSMX */
			return;
		}

		/* Don't update the crsmin registers if any desense(aci/bt) is on */
		if (pi->u.pi_acphy->total_desense->on) {
			pi->u.pi_acphy->crsmincal_run = 2;
			/* Restore Rx chains */
			wlc_phy_restore_rxchains((wlc_phy_t *)pi, enRx, enTx);
#ifdef WL_PSMX
			/* trigger MU resound due to the resetcca triggered by updating rxchains */
			if (D11REV_IS(pi->sh->corerev, 64)) {
				OR_REG(pi->sh->osh, &pi->regs->maccommand_x, MCMDX_SND);
			}
#endif /* WL_PSMX */
			return;
		}
	}

	/* Debug: keep count of all calls to crsmin_cal  */
	/* Debug: store the channel info  */
	/* Debug: store the noise pwr used for updating crs thresholds */
	pi->u.pi_acphy->phy_debug_crscal_counter++;
	pi->u.pi_acphy->phy_debug_crscal_channel = CHSPEC_CHANNEL(pi->radio_chanspec);
	FOREACH_ACTV_CORE(pi, pi->sh->phyrxchain, i)  {
		/* Debug info: for dumping the noise pwr used in crsmin_cal */
		pi->u.pi_acphy->phy_noise_in_crs_min[i] = cmplx_pwr_dbm[i];
	}

	/* since we are touching phy regs mac has to be suspended */
	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	phy_utils_phyreg_enter(pi);

	/* call for updating the crsmin thresholds */
	if (pi->u.pi_acphy->crsmincal_enable)
	  wlc_phy_set_crs_min_pwr_acphy(pi, fp[0], offset);
	if (pi->u.pi_acphy->lesiscalecal_enable)
	  wlc_phy_set_lesiscale_acphy(pi, lesi_scale);

	/* resume mac */
	phy_utils_phyreg_exit(pi);
	wlapi_enable_mac(pi->sh->physhim);

	/* Restore Rx chains */
	wlc_phy_restore_rxchains((wlc_phy_t *)pi, enRx, enTx);

#ifdef WL_PSMX
	/* trigger MU resound due to the resetcca triggered by updating rxchains */
	if (D11REV_IS(pi->sh->corerev, 64)) {
		OR_REG(pi->sh->osh, &pi->regs->maccommand_x, MCMDX_SND);
	}
#endif /* WL_PSMX */
}

int8
wlc_phy_get_avg_noisepwr_acphy(int8 noises[])
{
	int16 accum = 0;
	int8 min_val, avg_val = 0;
	uint8 i, j, loop, min_idx, cnt = 0;

	/* Take only lowest half noise vals, as higher values could be over pkt */
	loop = PHY_SIZE_NOISE_ARRAY >> 1;
	for (j = 0; j < loop; j++) {
		min_val = 100;  min_idx = PHY_SIZE_NOISE_ARRAY;
		for (i = 0; i < PHY_SIZE_NOISE_ARRAY; i++) {
			if ((noises[i] != 0) && (noises[i] < min_val)) {
				min_val = noises[i];
				min_idx = i;
			}
		}

		if (min_idx < PHY_SIZE_NOISE_ARRAY) {
			accum += noises[min_idx];
			noises[min_idx] = 0;
			cnt++;
		} else {
			break;    // no more valid values
		}
	}

	/* Average it */
	if (cnt > 0) avg_val = accum / cnt;

	return avg_val;
}

void
wlc_phy_set_crs_min_pwr_acphy(phy_info_t *pi, uint8 ac_th, int8 *offset)
{
	uint8 core;
	uint8 mfcrs_1bit = 0; /* match filter carrier sense 1 bit mode */
	uint8 mf_th = ac_th;
	int8  mf_off1 = offset[1];
	/* 1-bit MF: 4335A0, 4335B0, 4350 */
	/* 6-bit MF: 4335C0 */

	if (ACMAJORREV_1(pi->pubpi.phy_rev) || ACMAJORREV_2(pi->pubpi.phy_rev)) {
		mfcrs_1bit = 1;
	} else {
		mfcrs_1bit = 0;
	}

	/* did not see any positive pwr consumption impact of 1 bit mode, so increase sensitivity */
	if (ACMAJORREV_1(pi->pubpi.phy_rev) && ACMINORREV_2(pi)) {
		mfcrs_1bit = 0;
	}

	if (ACMAJORREV_2(pi->pubpi.phy_rev) && !ACMINORREV_0(pi)) {
		mfcrs_1bit = 0; /* >= v2.1 */
	}

	if (ac_th == 0) {
		if (mfcrs_1bit == 1) {
			if (CHSPEC_IS20(pi->radio_chanspec)) {
				mf_th = 60;
				ac_th = 58;
			} else if (CHSPEC_IS40(pi->radio_chanspec)) {
				mf_th = 60;
				ac_th = 58;
			} else if (CHSPEC_IS80(pi->radio_chanspec) ||
					PHY_AS_80P80(pi, pi->radio_chanspec)) {
				mf_th = 67;
				ac_th = 60;
			} else if (CHSPEC_IS160(pi->radio_chanspec)) {
				ASSERT(0);
			}
		} else {
			mf_th = ACPHY_CRSMIN_DEFAULT;
			ac_th = ACPHY_CRSMIN_DEFAULT;
		}
		pi->u.pi_acphy->phy_crs_th_from_crs_cal = ac_th;
	} else {
		if (mfcrs_1bit == 1) {
			if (CHSPEC_IS20(pi->radio_chanspec)) {
				mf_th = ((ac_th*101)/100) + 2;
			} else if (CHSPEC_IS40(pi->radio_chanspec)) {
				mf_th = ((ac_th*109)/100) - 2;
			} else if (CHSPEC_IS80(pi->radio_chanspec) ||
					PHY_AS_80P80(pi, pi->radio_chanspec)) {
				mf_th = ((ac_th*105)/100) + 2;
			} if (CHSPEC_IS160(pi->radio_chanspec)) {
				ASSERT(0);
			}
		}
	}

	/* Adjust offset values for 1-bit MF */
	/* Not needed for 4335 and 4360, will be needed for 4350, disabled for now */

	if (ACMAJORREV_2(pi->pubpi.phy_rev)) {
		FOREACH_ACTV_CORE(pi, pi->sh->phyrxchain, core) {
			if (core == 1) {
				if (CHSPEC_IS5G(pi->radio_chanspec)) {
					if (CHSPEC_IS20(pi->radio_chanspec))
					{
						mf_off1 = ((ac_th+offset[core])*101/100 + 2)-mf_th;
					} else if (CHSPEC_IS40(pi->radio_chanspec)) {
						mf_off1 = ((ac_th+offset[core])*109/100 - 2)-mf_th;
					} else if (CHSPEC_IS80(pi->radio_chanspec)) {
						mf_off1 = ((ac_th+offset[core])*105/100 + 2)-mf_th;
					}
				}
			}
		}
	}

#ifdef WL11ULB
	if (ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev)) {
		if (CHSPEC_IS10(pi->radio_chanspec)) {
			ac_th -= 0;
			mf_th -= 0;
		} else if (CHSPEC_IS5(pi->radio_chanspec)) {
			ac_th -= 8;
			mf_th -= 8;
		}
	}
#endif /* WL11ULB */

	PHY_CAL(("%s: (AC_th, MF_th) = (%d, %d)\n", __FUNCTION__, ac_th, mf_th));

	FOREACH_ACTV_CORE(pi, pi->sh->phyrxchain, core) {

		switch (core) {
			case 0:
				MOD_PHYREG(pi, crsminpoweru0, crsminpower0, ac_th);
				MOD_PHYREG(pi, crsmfminpoweru0, crsmfminpower0, mf_th);
				MOD_PHYREG(pi, crsminpowerl0, crsminpower0, ac_th);
				MOD_PHYREG(pi, crsmfminpowerl0, crsmfminpower0, mf_th);
				MOD_PHYREG(pi, crsminpoweruSub10, crsminpower0, ac_th);
				MOD_PHYREG(pi, crsmfminpoweruSub10, crsmfminpower0,  mf_th);
				MOD_PHYREG(pi, crsminpowerlSub10, crsminpower0, ac_th);
				MOD_PHYREG(pi, crsmfminpowerlSub10, crsmfminpower0,  mf_th);
				/* Force the offsets for core-0 */
				/* Core 0 */
				MOD_PHYREG(pi, crsminpoweroffset0,
				           crsminpowerOffsetu, offset[core]);
				MOD_PHYREG(pi, crsminpoweroffset0,
				           crsminpowerOffsetl, offset[core]);
				MOD_PHYREG(pi, crsmfminpoweroffset0,
				           crsmfminpowerOffsetu, offset[core]);
				MOD_PHYREG(pi, crsmfminpoweroffset0,
				           crsmfminpowerOffsetl, offset[core]);
				MOD_PHYREG(pi, crsminpoweroffsetSub10,
				           crsminpowerOffsetlSub1, offset[core]);
				MOD_PHYREG(pi, crsminpoweroffsetSub10,
				           crsminpowerOffsetuSub1, offset[core]);
				MOD_PHYREG(pi, crsmfminpoweroffsetSub10,
				           crsmfminpowerOffsetlSub1, offset[core]);
				MOD_PHYREG(pi, crsmfminpoweroffsetSub10,
				           crsmfminpowerOffsetuSub1, offset[core]);
				break;
			case 1:
				/* Force the offsets for core-1 */
				/* Core 1 */
				MOD_PHYREG(pi, crsminpoweroffset1,
				           crsminpowerOffsetu, offset[core]);
				MOD_PHYREG(pi, crsminpoweroffset1,
				           crsminpowerOffsetl, offset[core]);
				MOD_PHYREG(pi, crsmfminpoweroffset1,
				           crsmfminpowerOffsetu, mf_off1);
				MOD_PHYREG(pi, crsmfminpoweroffset1,
				           crsmfminpowerOffsetl, mf_off1);
				MOD_PHYREG(pi, crsminpoweroffsetSub11,
				           crsminpowerOffsetlSub1, offset[core]);
				MOD_PHYREG(pi, crsminpoweroffsetSub11,
				           crsminpowerOffsetuSub1, offset[core]);
				MOD_PHYREG(pi, crsmfminpoweroffsetSub11,
				           crsmfminpowerOffsetlSub1, mf_off1);
				MOD_PHYREG(pi, crsmfminpoweroffsetSub11,
				           crsmfminpowerOffsetuSub1, mf_off1);
				break;
			case 2:
				/* Force the offsets for core-2 */
				/* Core 2 */
				MOD_PHYREG(pi, crsminpoweroffset2,
				           crsminpowerOffsetu, offset[core]);
				MOD_PHYREG(pi, crsminpoweroffset2,
				           crsminpowerOffsetl, offset[core]);
				MOD_PHYREG(pi, crsmfminpoweroffset2,
				           crsmfminpowerOffsetu, offset[core]);
				MOD_PHYREG(pi, crsmfminpoweroffset2,
				           crsmfminpowerOffsetl, offset[core]);
				MOD_PHYREG(pi, crsminpoweroffsetSub12,
				           crsminpowerOffsetlSub1, offset[core]);
				MOD_PHYREG(pi, crsminpoweroffsetSub12,
				           crsminpowerOffsetuSub1, offset[core]);
				MOD_PHYREG(pi, crsmfminpoweroffsetSub12,
				           crsmfminpowerOffsetlSub1, offset[core]);
				MOD_PHYREG(pi, crsmfminpoweroffsetSub12,
				           crsmfminpowerOffsetuSub1, offset[core]);
				break;
			case 3:
				/* Force the offsets for core-3 */
				/* Core 3 */
				MOD_PHYREG(pi, crsminpoweroffset3,
					crsminpowerOffsetu, offset[core]);
				MOD_PHYREG(pi, crsminpoweroffset3,
					crsminpowerOffsetl, offset[core]);
				MOD_PHYREG(pi, crsmfminpoweroffset3,
					crsmfminpowerOffsetu, offset[core]);
				MOD_PHYREG(pi, crsmfminpoweroffset3,
					crsmfminpowerOffsetl, offset[core]);
				MOD_PHYREG(pi, crsminpoweroffsetSub13,
					crsminpowerOffsetlSub1, offset[core]);
				MOD_PHYREG(pi, crsminpoweroffsetSub13,
					crsminpowerOffsetuSub1, offset[core]);
				MOD_PHYREG(pi, crsmfminpoweroffsetSub13,
					crsmfminpowerOffsetlSub1, offset[core]);
				MOD_PHYREG(pi, crsmfminpoweroffsetSub13,
					crsmfminpowerOffsetuSub1, offset[core]);
				break;
		default:
			break;
		}
	}
}

void
wlc_phy_set_lesiscale_acphy(phy_info_t *pi, int8 *lesi_scale)
{
	uint8 core;
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;
	if (pi_ac->lesi > 0) {
	  FOREACH_ACTV_CORE(pi, pi->sh->phyrxchain, core) {
	    if (ACMAJORREV_32(pi->pubpi.phy_rev)) {
		MOD_PHYREGCE(pi, lesiPathControl0, core, inpScalingFactor, lesi_scale[core]);
	      } else {
		MOD_PHYREGCE(pi, lesiInputScaling0_, core, inpScalingFactor_0, lesi_scale[core]);
		MOD_PHYREGCE(pi, lesiInputScaling1_, core, inpScalingFactor_1, lesi_scale[core]);
		MOD_PHYREGCE(pi, lesiInputScaling2_, core, inpScalingFactor_2, lesi_scale[core]);
		MOD_PHYREGCE(pi, lesiInputScaling3_, core, inpScalingFactor_3, lesi_scale[core]);
	      }
	  }
	}
}

void
wlc_phy_stay_in_carriersearch_acphy(phy_info_t *pi, bool enable)
{
	uint8 class_mask;
	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	/* MAC should be suspended before calling this function */
	ASSERT((R_REG(pi->sh->osh, &pi->regs->maccontrol) & MCTL_EN_MAC) == 0);
	if (enable) {
		if (pi_ac->deaf_count == 0) {
			wlc_phy_classifier_acphy(pi, ACPHY_ClassifierCtrl_classifierSel_MASK, 4);
			wlc_phy_ofdm_crs_acphy(pi, FALSE);
			wlc_phy_clip_det_acphy(pi, FALSE);
			WRITE_PHYREG(pi, ed_crsEn, 0);
		}

		pi_ac->deaf_count++;
#ifdef WL_MUPKTENG
		if (!(D11REV_IS(pi->sh->corerev, 64) && wlapi_is_mutx_pkteng_on(pi->sh->physhim)))
#endif // endif
		{
		        wlc_phy_resetcca_acphy(pi);
		}
	} else {
	  ASSERT(pi_ac->deaf_count > 0);

		pi_ac->deaf_count--;
		if (pi_ac->deaf_count == 0) {
			class_mask = CHSPEC_IS2G(pi->radio_chanspec) ? 7 : 6;   /* No bphy in 5g */
			wlc_phy_classifier_acphy(pi, ACPHY_ClassifierCtrl_classifierSel_MASK,
			                         class_mask);
			wlc_phy_ofdm_crs_acphy(pi, TRUE);
			wlc_phy_clip_det_acphy(pi, TRUE);
			WRITE_PHYREG(pi, ed_crsEn, pi_ac->edcrs_en);
		}
	}
}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
void wlc_phy_force_gainlevel_acphy(phy_info_t *pi, int16 int_val)
{
	uint8 core;

	FOREACH_ACTV_CORE(pi, pi->sh->phyrxchain, core) {

		/* disable clip2 */
		MOD_PHYREGC(pi, computeGainInfo, core, disableClip2detect, 1);
		WRITE_PHYREGC(pi, Clip2Threshold, core, 0xffff);
		printf("wlc_phy_force_gainlevel_acphy (%d) : ", int_val);
	switch (int_val) {
	case 0:
		printf("initgain -- adc never clips.\n");
		if (ACREV_IS(pi->pubpi.phy_rev, 0)) {
			WRITE_PHYREGC(pi, Clip1Threshold, core, 0xffff);
		} else {
			MOD_PHYREGC(pi, computeGainInfo, core, disableClip1detect, 1);
		}
		break;
	case 1:
		printf("clip hi -- adc always clips, nb never clips.\n");
		MOD_PHYREGC(pi, FastAgcClipCntTh, core, fastAgcNbClipCntTh, 0xff);
		break;
	case 2:
		printf("clip md -- adc/nb always clips, w1 never clips.\n");
		MOD_PHYREGC(pi, FastAgcClipCntTh, core, fastAgcNbClipCntTh, 0);
		MOD_PHYREGC(pi, FastAgcClipCntTh, core, fastAgcW1ClipCntTh, 0xff);
		break;
	case 3:
		printf("clip lo -- adc/nb/w1 always clips.\n");
		MOD_PHYREGC(pi, FastAgcClipCntTh, core, fastAgcNbClipCntTh, 0);
		MOD_PHYREGC(pi, FastAgcClipCntTh, core, fastAgcW1ClipCntTh, 0);
		break;
	case 4:
		printf("adc clip.\n");
		WRITE_PHYREGC(pi, clipHiGainCodeA, core, 0x0);
		WRITE_PHYREGC(pi, clipHiGainCodeB, core, 0x8);
		MOD_PHYREGC(pi, FastAgcClipCntTh, core, fastAgcNbClipCntTh, 0xff);
		break;
	case 5:
		printf("nb clip.\n");
		WRITE_PHYREGC(pi, clipmdGainCodeA, core, 0xfffe);
		WRITE_PHYREGC(pi, clipmdGainCodeB, core, 0x554);
		MOD_PHYREGC(pi, FastAgcClipCntTh, core, fastAgcW1ClipCntTh, 0xff);
		break;
	case 6:
		printf("w1 clip.\n");
		WRITE_PHYREGC(pi, cliploGainCodeA, core, 0xfffe);
		WRITE_PHYREGC(pi, cliploGainCodeB, core, 0x554);
		MOD_PHYREGC(pi, RssiClipMuxSel, core, fastAgcNbClipMuxSel, 0);
		ASSERT(RADIOID(pi->pubpi.radioid) == BCM2069_ID);
		MOD_RADIO_REGC(pi, NBRSSI_CONFG, core, nbrssi_Refctrl_low, 1);
		break;
	}
	}
	printf("wlc_phy_force_gainlevel_acphy (%d)\n", int_val);
}
#endif /* defined(BCMDBG) || defined(BCMDBG_DUMP) */
uint8 wlc_phy_get_lna_gain_rout(phy_info_t *pi, uint8 idx, acphy_lna_gain_rout_t type)
{
	uint8 ret_val;
	if (type == GET_LNA_GAINCODE) {
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			ret_val = ac_tiny_g_lna_gain_map[idx];
		} else {
			ret_val = ac_tiny_a_lna_gain_map[idx];
		}
	} else {
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			ret_val = ac_tiny_g_lna_rout_map[idx];
		} else {
			ret_val = ac_tiny_a_lna_rout_map[idx];
		}
	}
	return (ret_val);
}

/* This function tells you locale info, e.g EU, so that correct edcrs setting could be done */
static void
phy_ac_rxgcrs_set_locale(phy_type_rxgcrs_ctx_t *ctx, uint8 region_group)
{
	/* USAGE: if (region_group == LOCALE_EU) */
	/* Nothing for now - just a template */
}

void wlc_phy_set_srom_eu_edthresh_acphy(phy_info_t *pi)
{
	int32 eu_edthresh;
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;

	eu_edthresh = CHSPEC_IS2G(pi->radio_chanspec) ?
	        pi->srom_eu_edthresh2g : pi->srom_eu_edthresh5g;
	if (eu_edthresh < -10) /* 0 & 0xff(-1) are invalid values */
		wlc_phy_adjust_ed_thres_acphy(pi, &eu_edthresh, TRUE);
	else
		wlc_phy_adjust_ed_thres_acphy(pi, &pi_ac->sromi->ed_thresh_default, TRUE);
}

void
wlc_phy_lesi_acphy(phy_info_t *pi, bool on, uint8 delta_halfdB)
{
	  phy_info_acphy_t *pi_ac = pi->u.pi_acphy;
      uint8 lsb, msb, delta = (delta_halfdB << 3);   // 16 ticks = 1dB
      uint16 val = 0;

      // LESI not supported for this chip OR don't want to enable it
      if (pi_ac->lesi == 0)
        return;

      if (on) {
          MOD_PHYREG(pi, lesi_control, lesiFstrEn, 1);
          MOD_PHYREG(pi, lesi_control, lesiCrsEn, 1);
          MOD_PHYREG(pi, lesiFstrControl0, winmf_1stPeak_Scl, 0x8);
          MOD_PHYREG(pi, lesiCrsTypRxPowerPerCore, PowerLevelPerCore,
                     CHSPEC_IS20(pi->radio_chanspec) ?
                     0x15b : CHSPEC_IS40(pi->radio_chanspec) ? 0x228 : 0x308);
          MOD_PHYREG(pi, lesiCrsHighRxPowerPerCore, PowerLevelPerCore,
                     CHSPEC_IS20(pi->radio_chanspec) ?
                     0x76e : CHSPEC_IS40(pi->radio_chanspec) ? 0x9c1 : 0xAA2);
          MOD_PHYREG(pi, lesiCrsMinRxPowerPerCore, PowerLevelPerCore,
                     CHSPEC_IS20(pi->radio_chanspec) ?
                     0x5 : CHSPEC_IS40(pi->radio_chanspec) ? 0x18 : 0x4);
          MOD_PHYREG(pi, lesiCrs1stDetThreshold_1, crsDetTh1_1Core, delta +
                     (CHSPEC_IS20(pi->radio_chanspec) ?
                      0x41 : CHSPEC_IS40(pi->radio_chanspec) ? 0x2d : 0x23));
          MOD_PHYREG(pi, lesiCrs1stDetThreshold_1, crsDetTh1_2Core, delta +
                     (CHSPEC_IS20(pi->radio_chanspec) ?
                     0x30 : CHSPEC_IS40(pi->radio_chanspec) ? 0x20 : 0x1b));
          MOD_PHYREG(pi, lesiCrs1stDetThreshold_2, crsDetTh1_3Core, delta +
                     (CHSPEC_IS20(pi->radio_chanspec) ?
                     0x28 : CHSPEC_IS40(pi->radio_chanspec) ? 0x1b : 0x17));
          MOD_PHYREG(pi, lesiCrs1stDetThreshold_2, crsDetTh1_4Core, delta +
                     (CHSPEC_IS20(pi->radio_chanspec) ?
                     0x23 : CHSPEC_IS40(pi->radio_chanspec) ? 0x18 : 0x14));
          MOD_PHYREG(pi, lesiCrs2ndDetThreshold_1, crsDetTh1_1Core, delta +
                     (CHSPEC_IS20(pi->radio_chanspec) ?
                     0x41 : CHSPEC_IS40(pi->radio_chanspec) ? 0x2d : 0x23));
          MOD_PHYREG(pi, lesiCrs2ndDetThreshold_1, crsDetTh1_2Core, delta +
                     (CHSPEC_IS20(pi->radio_chanspec) ?
                     0x30 : CHSPEC_IS40(pi->radio_chanspec) ? 0x20 : 0x1b));
          MOD_PHYREG(pi, lesiCrs2ndDetThreshold_2, crsDetTh1_3Core, delta +
                     (CHSPEC_IS20(pi->radio_chanspec) ?
                     0x28 : CHSPEC_IS40(pi->radio_chanspec) ? 0x1b : 0x17));
          MOD_PHYREG(pi, lesiCrs2ndDetThreshold_2, crsDetTh1_4Core, delta +
                     (CHSPEC_IS20(pi->radio_chanspec) ?
                     0x23 : CHSPEC_IS40(pi->radio_chanspec) ? 0x18 : 0x14));
          MOD_PHYREG(pi, lesiFstrControl3, cCrsFftInpAdj, CHSPEC_IS20(pi->radio_chanspec) ?
                     0x0 : CHSPEC_IS40(pi->radio_chanspec) ? 0x1 : 0x3);
          MOD_PHYREG(pi, lesiFstrClassifyThreshold0, MaxScaleHighValue, 0x5a);
          if (ACMAJORREV_33(pi->pubpi.phy_rev)) {
              /* the default value caused the degradation on SGI for C1s4 */
              MOD_PHYREG(pi, lesiFstrControl5, lesi_sgi_hw_adj, 0x13);

              /* Increase the 20 primary crs detection  thresold which cause the PER floor */
              msb = 0x33 + delta; lsb = 0x44 + delta;         /* 0x3344 + delta */
              val = (msb << 8) | lsb;
              WRITE_PHYREG(pi, lesiCrs20P1stDetThreshold_1, val);
              WRITE_PHYREG(pi, lesiCrs20P2ndDetThreshold_1, val);

              msb = 0x26 + delta; lsb = 0x2a + delta;        /* 0x262a + delta */
              val = (msb << 8) | lsb;
              WRITE_PHYREG(pi, lesiCrs20P1stDetThreshold_2, val);
              WRITE_PHYREG(pi, lesiCrs20P2ndDetThreshold_2, val);
              /* PER hump at low power regime siwthcing point at 2G band, the WAR is to set fstrSwitchEn */
              if (CHSPEC_IS2G(pi->radio_chanspec)) {
                  MOD_PHYREG(pi, lesiFstrControl4, fstrSwitchEn, 1);
                  MOD_PHYREG(pi, lesiFstrControl4, selLesiCstr, 0);
              }
		  }
      } else {
          MOD_PHYREG(pi, lesi_control, lesiFstrEn, 0);
          MOD_PHYREG(pi, lesi_control, lesiCrsEn, 0);
      }

	  if (PHY_AS_80P80(pi, pi->radio_chanspec)) {
		    MOD_PHYREG(pi, lesi_control, lesiFstrEn, 0);
		    MOD_PHYREG(pi, lesi_control, lesiCrsEn, 0);
	  }
}
