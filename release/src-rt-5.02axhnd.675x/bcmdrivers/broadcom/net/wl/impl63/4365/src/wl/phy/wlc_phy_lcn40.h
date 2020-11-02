/*
 * LCNPHY module header file
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
 * $Id: wlc_phy_lcn40.h 464069 2014-03-22 09:48:15Z $*
 */

#ifndef _wlc_phy_lcn40_h_
#define _wlc_phy_lcn40_h_

#include <typedefs.h>
#include <wlc_phy_int.h>
#include <wlc_phy_lcn.h>
#define LCN40PHY_SWCTRL_NVRAM_PARAMS 5
#define LCN40PHY_RXIQCOMP_PARAMS 2
#define LCN40PHY_NOTCHFILTER_COEFFS 10

#define LCN40PHY_RCAL_OFFSET 0x10
#define LCN40PHY_PAPR_NVRAM_PARAMS 20

#define LCN40PHY_GAIN_DELTA_2G_PARAMS 9
#define LCN40PHY_GAIN_DELTA_5G_PARAMS 4

#define WORKAROUND_CRLCNPHY_680_EN 0x2

#define LCN40PHY_TEMPER_THRESHOLD_HIGH 55
#define LCN40PHY_TEMPER_THRESHOLD_LOW (-5)
#define LCN40PHY_TEMPER_DIFF 10

#define LCN40_TXIDXCAP_INVALID	(-1)

/* PAPD linearization path selection */
#ifdef WLPHY_IPA_ONLY
#define LCN40_LINPATH(val)	(LCN40PHY_PAPDLIN_IPA)
#else
#define LCN40_LINPATH(val)	(val)
#endif // endif

typedef enum {
	LCN40PHY_PAPDLIN_PAD = 0,
	LCN40PHY_PAPDLIN_EPA,
	LCN40PHY_PAPDLIN_IPA
} lcn40phy_papd_lin_path_t;

/* This is the size of the largest structure for which
 * a memory allocation is required in the wlc_ply_lcn40.c file
 */
#define LCN40PHY_CALBUFFER_MAX_SZ 1024

#define TEMPER_VBAT_TRIGGER_NEW_MEAS 1

/* ********************************************************* */
#include "phy_api.h"
#include "phy_lcn40_ana.h"
#include "phy_lcn40_radio.h"
#include "phy_lcn40_tbl.h"
#include "phy_lcn40_tpc.h"
#include "phy_lcn40_antdiv.h"
#include "phy_lcn40_noise.h"
#include "phy_lcn40_rssi.h"
/* ********************************************************* */

struct phy_info_lcn40phy {
	phy_info_lcnphy_t lcnphycommon;
/* ********************************************************* */
	phy_info_t *pi;
	phy_lcn40_ana_info_t *anai;
	phy_lcn40_radio_info_t *radioi;
	phy_lcn40_tbl_info_t *tbli;
	phy_lcn40_tpc_info_t *tpci;
	phy_lcn40_antdiv_info_t *antdivi;
	phy_lcn40_noise_info_t *noisei;
	phy_lcn40_rssi_info_t *rssii;
/* ********************************************************* */
	uint16 rx_iq_comp_5g[LCN40PHY_RXIQCOMP_PARAMS];
	uint8 trGain;
	int16 tx_iir_filter_type_cck;
	int16 tx_iir_filter_type_ofdm;
	int16 tx_iir_filter_type_ofdm40;
	bool phycrs_war_en;
	int16 pwr_offset40mhz_2g;
	int16 pwr_offset40mhz_5g;
	uint8 dac2x_enable;
	uint8 dac2x_enable_nvm;
	uint8 rcal;
	uint16 mixboost_5g;
	uint8 dacpu;
	uint8 elna_off_gain_idx_2g;
	uint8 elna_off_gain_idx_5g;
	uint8 gain_settle_dly_2g;
	uint8 gain_settle_dly_5g;
	bool tia_dc_loop_enable_2g;
	bool hpc_sequencer_enable_2g;
	bool tia_dc_loop_enable_5g;
	bool hpc_sequencer_enable_5g;
	int16 padbias5g;
	int8 aci_detect_en_2g;
	int8 tx_agc_reset_2g;
	int8 gaintbl_force_2g;
	int8 gaintbl_presel_2g;
	int8 aci_detect_en_5g;
	int8 tx_agc_reset_5g;
	int8 gaintbl_force_5g;
	int8 gaintbl_presel_5g;
	int iqcalidx5g;
	bool tx_alpf_pu;
	int16 lcnphy_idletssi_corr;
	int16 tempsenseCorr;
	bool epa_or_pad_lpbk;
	bool loflag;
	int8 dlocalidx5g;
	int8 dlorange_lowlimit;
	int32 noise_cal_deltamax;
	int32 noise_cal_deltamin;
	int8 dsss_thresh;
	uint32 startdiq_2g;
	uint32 startdiq_5g;
	bool btc_clamp;
	uint16 rx_iq_comp_2g[LCN40PHY_RXIQCOMP_PARAMS];
	int8 high_temp_threshold; /* Temperature in Deg Celsius */
	int8 temp_offs1_2g;   /* qdB steps, applied to Per rate targets */
	int8 temp_offs1_5g;   /* qdB steps, applied to Per rate targets */
	int8 cond_offs1;      /* qdB steps, applied to Per rate targets */
	int8 low_temp_threshold; /* Temperature in Deg Celsius */
	int8 temp_offs2_2g; /* qdB steps, applied to Per rate targets */
	int8 temp_offs2_5g; /* qdB steps, applied to Per rate targets */
	int8 cond_offs2;    /* qdB steps, applied to Per rate targets */
	int8 temp_diff;     /* Temperature Difference in Deg Celsius */
	int8 high_vbat_threshold; /* Voltage in 1/16 V */
	int8 vbat_offs1_2g;  /* qdB steps, applied to Per rate targets */
	int8 vbat_offs1_5g;  /* qdB steps, applied to Per rate targets */
	int8 cond_offs3;     /* qdB steps, applied to Per rate targets */
	int8 low_vbat_threshold; /* Voltage in 1/16 V */
	int8 vbat_offs2_2g;  /* qdB steps, applied to Per rate targets */
	int8 vbat_offs2_5g;  /* qdB steps, applied to Per rate targets */
	int8 cond_offs4;     /* qdB steps, applied to Per rate targets */
	int8 vbat_diff;      /* Voltage Difference in 1/16 V */
	uint16	cck_tssi_idx;
	uint16	init_ccktxpwrindex;
	int lowpower_beacon_mode;
	int8 localoffs5gmh;
	int16 temp_offset_2g;
	int16 temp_offset_5g;
	int16 pretemp;
	int8 temp_cal_en_2g;
	int8 temp_cal_en_5g;
	int8 sample_collect_gainadj;
	uint8 sample_collect_gainidx;
	int16  ofdm40_dig_filt_type_2g;
	int16  ofdm40_dig_filt_type_5g;
	uint16 save_digi_gain_ovr;
	phy_idletssi_perband_info_t lcn40_idletssi0_cache;
	phy_idletssi_perband_info_t lcn40_idletssi1_cache;
	int8 vlin2g;
	int8 vlin5g;
	int16 edonthreshold40;
	int16 edoffthreshold40;
	int16 edonthreshold20U;
	int16 edonthreshold20L;
	int16 edoffthreshold20UL;
	uint8 ppidletssi_en;
	uint8 ppidletssi_en_2g;
	uint8 ppidletssi_en_5g;
	int16 ppidletssi_corr_2g;
	int16 ppidletssi_corr_5g;
	uint16 tempsense_tx_cnt;
	int16 last_tempsense_avg;
	int8 papden2g;
	int8 papden5g;
	bool papd_enable;
	int16 papdlinpath2g;
	int16 papdlinpath5g;
	int16 papd_lin_path;
	uint16 papd_bbmult_init_bw20;
	uint16 papd_bbmult_init_bw40;
	uint16 papd_bbmult_step_bw20;
	uint16 papd_bbmult_step_bw40;
	uint16 papd_lut_step;
	uint16 papd_lut_begin;
	uint8 papd_num_symbols;
	uint16 papd_bbmult_init_rev_bw20;
	uint16 papd_bbmult_init_rev_bw40;
	uint16 papd_bbmult_step_rev_bw20;
	uint16 papd_bbmult_step_rev_bw40;
	bool papd_stop_after_last_update;

	phy_tx_gain_tbl_entry *txgaintable;
	uint8 paprr_enable2g;
	uint8 paprr_enable5g;
	uint32 paprgamtbl2g[LCN40PHY_PAPR_NVRAM_PARAMS];
	uint32 paprgamtbl5g[LCN40PHY_PAPR_NVRAM_PARAMS];
	uint32 papr40gamtbl5g[LCN40PHY_PAPR_NVRAM_PARAMS];

	int8 paprofftbl2g[LCN40PHY_PAPR_NVRAM_PARAMS];
	int8 paprofftbl5g[LCN40PHY_PAPR_NVRAM_PARAMS];
	int8 papr40offtbl5g[LCN40PHY_PAPR_NVRAM_PARAMS];

	uint8 paprgaintbl2g[LCN40PHY_PAPR_NVRAM_PARAMS];
	uint8 paprgaintbl5g[LCN40PHY_PAPR_NVRAM_PARAMS];
	uint8 papr40gaintbl5g[LCN40PHY_PAPR_NVRAM_PARAMS];
	int16 papdsfac5g;
	int16 papdsfac2g;
	int16 papd_mag_th_2g;
	int16 papd_mag_th_5g;
	int16 max_amam_dB;
	bool rfpll_doubler_2g;
	bool rfpll_doubler_5g;
	uint16 pll_loop_bw_desired_2g;
	uint16 pll_loop_bw_desired_5g;
	uint32 temp_cal_adj_2g;
	uint32 temp_cal_adj_5g;
	uint8 fstr_flag;
	uint8 cdd_mod;
	uint8 rssi_log_nsamps;
	uint8 rssi_iqest_en;
	int8 rssi_iqest_gain_adj;
	int8 rssi_iqest_iov_gain_adj;
	uint8 rssi_iqest_jssi_en;
	int32 papden2gchan;
	uint8 do_papd_calidx_est;
	int32 target_pwr_dbm;
	int32 base_pwr_dbm;
	int32 calidxestbase2g;
	int32 calidxesttarget2g;
	int32 calidxestbase5g;
	int32 calidxesttarget5g;
	int32 calidxesttarget405g;
	int32 calidxesttargetlo5g;
	int32 calidxesttarget40lo5g;
	int32 calidxesttargethi5g;
	int32 calidxesttarget40hi5g;
	int16 clipthr_eLNA2g;
	uint16 papdrx2g;
	uint16 papdrx5g;
	int8 rssi_gain_delta_2g[LCN40PHY_GAIN_DELTA_2G_PARAMS];
	int8 rssi_gain_delta_2gh[LCN40PHY_GAIN_DELTA_2G_PARAMS];
	int8 rssi_gain_delta_2ghh[LCN40PHY_GAIN_DELTA_2G_PARAMS];
	int8 rssi_gain_delta_5gl[LCN40PHY_GAIN_DELTA_5G_PARAMS];
	int8 rssi_gain_delta_5gml[LCN40PHY_GAIN_DELTA_5G_PARAMS];
	int8 rssi_gain_delta_5gmu[LCN40PHY_GAIN_DELTA_5G_PARAMS];
	int8 rssi_gain_delta_5gh[LCN40PHY_GAIN_DELTA_5G_PARAMS];
	int8 gain_cal_temp;
	int16 rssi_rxsaw_slope_2g[3];
	int16 rssi_dp2_slope_2g[3];
	int16 rssi_eLNAbyp_slope_2g;
	int16 rssi_eLNAbyp_slope_5g;
	int16 rssi_eLNAon_slope_2g[3];
	int16 rssi_eLNAon_slope_5gh;
	int16 rssi_eLNAon_slope_5gmu;
	int16 rssi_eLNAon_slope_5gml;
	int16 rssi_eLNAon_slope_5gl;
	uint8 noise_iqest_en;
	uint8 noise_log_nsamps;
	int8 noise_iqest_gain_adj_2g;
	int8 noise_iqest_gain_adj_5gl;
	int8 noise_iqest_gain_adj_5gm;
	int8 noise_iqest_gain_adj_5gh;
	/* Flag and array to be used for cals related buffer in lcn40 PHY */
	bool calbuffer_inuse;
	uint8 calbuffer[LCN40PHY_CALBUFFER_MAX_SZ];
	uint8 new_lpf_rccal;
	int8 cckscale_fctr_db;
	int8 irdsw;
	int16 nom_txidxcap_2g;
	int16 nom_txidxcap_5g;
	int16 txidxcap_2g_high;
	int16 txidxcap_2g_low;
	int16 txidxcap_5g_high;
	int16 txidxcap_5g_low;
	int8 txidxcap_2g_off;
	int8 txidxcap_5g_off;
	int16 txidxcap_high;
	int16 txidxcap_low;
	bool txidxcap_hi_inuse;
	bool txidxcap_lo_inuse;

	phy_tx_gain_tbl_entry *gaintable;
	uint8 dac_scram_off_2g;
	uint8 dac_scram_off_5g;
};

extern void wlc_lcn40phy_set_bbmult(phy_info_t *pi, uint8 m0);
#if defined(WLTEST)
extern void wlc_phy_get_rxgainerr_lcn40phy(phy_info_t *pi, int16 *gainerr);
extern void wlc_phy_get_SROMnoiselvl_lcn40phy(phy_info_t *pi, int8 *noiselvl);
extern void wlc_lcn40phy_rx_power(phy_info_t *pi, uint16 num_samps,
	uint8 wait_time, uint8 wait_for_crs, phy_iq_est_t* est);
extern int16 wlc_lcn40phy_rxgaincal_tempadj(phy_info_t *pi);
extern void wlc_phy_get_noiseoffset_lcn40phy(phy_info_t *pi, int16 *noiseoff);
extern void wlc_lcn40phy_get_lna_freq_correction(phy_info_t *pi, int8 *freq_offset_fact);
#endif // endif
extern void wlc_lcn40phy_aci_init(phy_info_t *pi);
extern void wlc_lcn40phy_aci(phy_info_t *pi, bool on);
extern void wlc_lcn40phy_rev6_aci(phy_info_t *pi, int wanted_mode);
extern void wlc_lcn40phy_aci_upd(phy_info_t *pi);
extern int wlc_lcn40phy_idle_tssi_reg_iovar(phy_info_t *pi, int32 int_val, bool set, int *err);
extern int wlc_lcn40phy_avg_tssi_reg_iovar(phy_info_t *pi);
extern int16 wlc_lcn40phy_rssi_tempcorr(phy_info_t *pi, bool mode);
extern int16 wlc_lcn40phy_iqest_rssi_tempcorr(phy_info_t *pi, bool mode, uint16 board_atten);
extern uint8 wlc_lcn40phy_max_cachedchans(phy_info_t *pi);
extern int16 wlc_lcn40phy_get_rxpath_gain_by_index(phy_info_t *pi, uint8 gain_index,
	uint16 board_atten);
extern void
wlc_lcn40phy_apply_cond_chg(phy_info_lcn40phy_t *pi_lcn40, ppr_t *tx_pwr_target);
extern void wlc_lcn40phy_update_cond_backoff_boost(phy_info_t* pi);
extern void wlc_lcn40phy_trigger_noise_iqest(phy_info_t* pi);
extern int8 wlc_lcn40phy_get_noise_iqest_gainadjust(phy_info_t *pi);
extern void wlc_lcn40phy_txgainindex_cap_adjust(phy_info_t *pi);

/* ********************** REMOVE ********************* */
void wlc_phy_init_lcn40phy(phy_info_t *pi);

#endif /* _wlc_phy_lcn40_h_ */
