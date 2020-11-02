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
 * $Id: wlc_phy_lcn.h 582411 2015-08-27 11:56:05Z $
 */

#ifndef _wlc_phy_lcn_h_
#define _wlc_phy_lcn_h_

#include <typedefs.h>
#include <wlc_phy_int.h>

#ifdef PHY_LCN_TXGAINTBL
#define TXGAINTBL(pi_lcn)	((void)(pi_lcn), PHY_LCN_TXGAINTBL)
#else
#define TXGAINTBL(pi_lcn)	((pi_lcn)->txgaintbl)
#endif // endif

#ifdef PHY_LCN_TXGAINTBL5G
#define TXGAINTBL5G(pi_lcn)	((void)(pi_lcn), PHY_LCN_TXGAINTBL5G)
#else
#define TXGAINTBL5G(pi_lcn)	((pi_lcn)->txgaintbl5g)
#endif // endif

#ifdef PHY_LCN_EPA
#define EPA(pi_lcn)		((void)(pi_lcn), PHY_LCN_EPA)
#else
#define EPA(pi_lcn)		((pi_lcn)->ePA)
#endif // endif

#ifdef PHY_PAPD_4336_MODE
#define PAPD_4336_MODE(pi_lcn)	((void)(pi_lcn), PHY_PAPD_4336_MODE)
#else
#define PAPD_4336_MODE(pi_lcn)	((pi_lcn)->lcnphy_papd_4336_mode)
#endif // endif

#define LCNPHY_SWCTRL_NVRAM_PARAMS 5

#define RX_GAIN_CAL_FINAL_PASS          0x0
#define RX_GAIN_CAL_FINAL_TIMEOUT       0x1
#define RX_GAIN_CAL_NO_DECISION        0x00
#define RX_GAIN_CAL_3DB_DECISION       0x10
#define RX_GAIN_CAL_THRESH_DECISION    0x20
#define RX_GAIN_CAL_3DB_ADJUST        0x100
#define RX_GAIN_CAL_GS_TIMEOUT        0x200

/* ********************************************************* */
#include "phy_api.h"
#include "phy_lcn_ana.h"
#include "phy_lcn_radio.h"
#include "phy_lcn_tbl.h"
#include "phy_lcn_tpc.h"
#include "phy_lcn_antdiv.h"
#include "phy_lcn_rssi.h"
#include "phy_lcn_noise.h"
/* ********************************************************* */

struct phy_info_lcnphy {
/* ********************************************************* */
	phy_info_t *pi;
	phy_lcn_ana_info_t *anai;
	phy_lcn_radio_info_t *radioi;
	phy_lcn_tbl_info_t *tbli;
	phy_lcn_tpc_info_t *tpci;
	phy_lcn_antdiv_info_t *antdivi;
	phy_lcn_rssi_info_t *rssii;
	phy_lcn_noise_info_t *noisei;
/* ********************************************************* */
	uint8 	lcnphy_cal_counter;
	bool	lcnphy_recal;
	bool	lcnphy_papd_cal_done_at_init;
	uint32  lcnphy_mcs20_po;

	uint8	lcnphy_tr_isolation_mid;	/* TR switch isolation for each sub-band */

	uint8	lcnphy_rx_power_offset;	 /* Input power offset */
	uint8	lcnphy_rssi_vf;		/* RSSI Vmid fine */
	uint8	lcnphy_rssi_vc;		/* RSSI Vmid coarse */
	uint8	lcnphy_rssi_gs;		/* RSSI gain select */
	uint8	lcnphy_rssi_vf_lowtemp;	/* RSSI Vmid fine */
	uint8	lcnphy_rssi_vc_lowtemp; /* RSSI Vmid coarse */
	uint8	lcnphy_rssi_gs_lowtemp; /* RSSI gain select */

	uint8	lcnphy_rssi_vf_hightemp;	/* RSSI Vmid fine */
	uint8	lcnphy_rssi_vc_hightemp;	/* RSSI Vmid coarse */
	uint8	lcnphy_rssi_gs_hightemp;	/* RSSI gain select */

	uint8	lcnphy_vbat_vf;		/* Vbatsense Vmid fine */
	uint8	lcnphy_vbat_vc;		/* Vbatsense Vmid coarse */
	uint8	lcnphy_vbat_gs;		/* Vbatsense gain select */

	uint8	lcnphy_temp_vf;		/* Tempsense Vmid fine */
	uint8	lcnphy_temp_vc;		/* Tempsense Vmid coarse */
	uint8	lcnphy_temp_gs;		/* Tempsense gain select */

	/* next 3 are for tempcompensated tx power control algo of 4313A0 */
	uint16 	lcnphy_rawtempsense;
	uint8   lcnphy_measPower;
	uint8   lcnphy_measPower1;
	uint8   lcnphy_measPower2;
	uint8  	lcnphy_tempsense_slope;
	uint8	lcnphy_freqoffset_corr;
	uint8	lcnphy_tempsense_option;
	int8	lcnphy_tempcorrx;
	bool	lcnphy_iqcal_swp_dis;
	bool	lcnphy_hw_iqcal_en;
	uint    lcnphy_bandedge_corr;
	bool    lcnphy_spurmod;
	uint16	lcnphy_tssi_tx_cnt; /* Tx frames at that level for NPT calculations */
	uint16	lcnphy_tssi_idx;	/* Estimated index for target power */
	uint16	lcnphy_tssi_npt;	/* NPT for TSSI averaging */

	int8	lcnphy_tx_power_idx_override; /* Forced tx power index */
	uint16	lcnphy_noise_samples;

	uint32	lcnphy_papdRxGnIdx;
	uint32	lcnphy_papd_rxGnCtrl_init;

	uint32	lcnphy_gain_idx_14_lowword;
	uint32	lcnphy_gain_idx_14_hiword;
	uint32	lcnphy_gain_idx_27_hiword;
	int16	lcnphy_ofdmgainidxtableoffset;  /* reference ofdm gain index table offset */
	int16	lcnphy_dsssgainidxtableoffset;  /* reference dsss gain index table offset */
	uint32	lcnphy_tr_R_gain_val;  /* reference value of gain_val_tbl at index 64 */
	uint32	lcnphy_tr_T_gain_val;	/* reference value of gain_val_tbl at index 65 */
	int8	lcnphy_input_pwr_offset_db;
	uint16	lcnphy_Med_Low_Gain_db;
	uint16	lcnphy_Very_Low_Gain_db;
	int8	lcnphy_lastsensed_temperature;
	int8	lcnphy_pkteng_rssi_slope;
	uint8	lcnphy_cck;
	bool    lcnphy_papd_4336_mode;
	int16	lcnphy_cck_dig_filt_type;
	int16	lcnphy_ofdm_dig_filt_type;
	int16	lcnphy_ofdm_dig_filt_type_2g;
	int16	lcnphy_ofdm_dig_filt_type_5g;
	bool	lcnphy_uses_rate_offset_table;

#if !defined(PHYCAL_CACHING)
	lcnphy_cal_results_t lcnphy_cal_results;
	uint8	lcnphy_full_cal_channel;
	uint16 	lcnphy_cal_temper;
#endif // endif

	/* used for debug */
	uint8	lcnphy_start_idx;
	uint8	lcnphy_current_index;
	uint8	lcnphy_capped_index;
	bool	lcnphy_calreqd;
	bool    lcnphy_CalcPapdCapEnable;
	uint16	lcnphy_logen_buf_1;
	uint16	lcnphy_local_ovr_2;
	uint16	lcnphy_local_oval_6;
	uint16	lcnphy_local_oval_5;
	uint16	lcnphy_logen_mixer_1;

	uint8	lcnphy_aci_stat;
	uint	lcnphy_aci_start_time;
	/* noise cal data */
	noise_t noise;
	lcnphy_aci_t    lcnphy_aci;
	bool    ePA;
	uint8   extpagain2g;
	uint8   extpagain5g;
	uint8	txpwrindex_nvram;
	int16	pa_gain_ovr_val_2g;
	int16	pa_gain_ovr_val_5g;

	int16	lcnphy_tx_iqlo_tone_freq_ovr_val;

	/* Coefficients for Temperature Conversion to Centigrade */
	/* Temp in deg = (temp_add - (T2-T1)*temp_mult)>>temp_q;  */
	int32 temp_mult;
	int32 temp_add;
	int32 temp_q;

	/* Coefficients for Vbat Conversion to Volts */
	/* Voltage = (vbat_add - (vbat_reading)*vbat_mult)>>vbat_q;  */
	int32 vbat_mult;
	int32 vbat_add;
	int32 vbat_q;

	int16 cckPwrOffset;
	int16 cckPwrIdxCorr;
	uint8 lcnphy_twopwr_txpwrctrl_en;

	uint8	dacrate;	/* DAC rate from srom */
	uint8	dacrate_2g;	/* DAC rate from srom */
	uint8	dacrate_5g;	/* DAC rate from srom */
	uint8	rfreg033;	/* rfreg033 values from srom */
	uint8	rfreg033_cck;	/* rfreg033 values from srom (CCK) */
	uint8	rfreg038;	/* rfreg038 values from srom */
	int8 	pacalidx;       /* to force papd cal index */
	int8	pacalidx_2g;	/* PA Cal Idx 2g from srom */
	int8	pacalidx_2g1;	/* PA Cal Idx 2g for second lut from srom */
	int8	pacalidx_5g;	/* PA Cal Idx 5g from srom */
	int8	pacalidx_5g1;	/* PA Cal Idx 5g for second lut from srom */
	int8	pacalalim;
	int16 	pacalamamth_2g;	/* PA cal amam threshold 2g */
	int16 	pacalamamth_2g1; /* PA cal amam threshold for second lut 2g */
	int16 	pacalamamth_5glo;	/* PA cal amam threshold 5glo */
	int16 	pacalamamth_5g;	/* PA cal amam threshold 5g */
	int16 	pacalamamth_5ghi;	/* PA cal amam threshold 5ghi */
	int16 	pacalamamth_5glo1;	/* PA cal amam threshold for second lut 5glo */
	int16 	pacalamamth_5g1;	/* PA cal amam threshold for second lut 5g */
	int16 	pacalamamth_5ghi1;	/* PA cal amam threshold for second lut 5ghi */
	uint8 	papd_rf_pwr_scale;
	uint8 	papd_rf_pwr_scale_2g;
	uint8 	papd_rf_pwr_scale_5g;
	int8 	papd_lut0_cal_idx;  /* PAPD index for lut0 */
	int8 	papd_lut1_cal_idx;	 /* PAPD index for lut1 */
	int8	pacalfcd1; /* PAPD failed cal detection, backoff rx gain if */
					   /* AMAM is above threshold, CSP 536770 */

	/*
	Switch control table params
	0 -> wl_pa1 wl_pa0 wl_tx1 wl_tx0
	1 -> wl_eLNArx1 wl_eLNArx0 wl_rx1 wl_rx0
	2 -> wl_eLNAAttnRx1 wl_eLNAAttnRx0 wl_AttnRx1 wl_AttnRx0
	3 -> bt_tx bt_eLNArx bt_rx
	4 -> ant(1 bit) ovr_en(1 bit) tdm(1 bit) wl_mask(8 bits)
	*/
	uint32 swctrlmap_2g[LCNPHY_SWCTRL_NVRAM_PARAMS];
	uint32 swctrlmap_5g[LCNPHY_SWCTRL_NVRAM_PARAMS];

	uint32 lcnphy_tssical_time;
	uint32 lcnphy_last_tssical;
	uint16 lcnphy_tssical_txdelay;

	/* 2g params from NVRAM */
	int16 dacgc2g;
	int16 gmgc2g;

#ifdef BAND5G
	/* 5g params from NVRAM */
	int16 dacgc5g;
	int16 gmgc5g;
	int16 rssismf5g;
	int16 rssismc5g;
	int16 rssisav5g;
#endif // endif

	int32 tssi_maxpwr_limit;
	int32 tssi_minpwr_limit;
	uint8 tssi_ladder_offset_maxpwr_2g;
	uint8 tssi_ladder_offset_minpwr_2g;
	uint8 tssi_ladder_offset_maxpwr_5glo;
	uint8 tssi_ladder_offset_minpwr_5glo;
	uint8 tssi_ladder_offset_maxpwr_5gmid;
	uint8 tssi_ladder_offset_minpwr_5gmid;
	uint8 tssi_ladder_offset_maxpwr_5ghi;
	uint8 tssi_ladder_offset_minpwr_5ghi;
	uint8 init_txpwrindex_2g;
	uint8 init_txpwrindex_5g;
	uint8 idx0cnt;
	uint8 idx127cnt;
	uint8 dynamic_pwr_limit_en;
	int32 target_pwr_cck_max;
	int32 target_pwr_ofdm_max;
	uint8 tssi_floor_2g;
	uint8 tssi_floor_5glo;
	uint8 tssi_floor_5gmid;
	uint8 tssi_floor_5ghi;
	uint32 rate_table[WL_RATESET_SZ_DSSS + WL_RATESET_SZ_OFDM + WL_RATESET_SZ_HT_MCS];
	uint8 txpwr_clamp_dis;
	uint8 txpwr_tssifloor_clamp_dis;
	uint8 txpwr_tssioffset_clamp_dis;
	uint8 txpwr_tssifloor_clamp_dis_2g;
	uint8 txpwr_tssioffset_clamp_dis_2g;
	uint8 txpwr_tssifloor_clamp_dis_5g;
	uint8 txpwr_tssioffset_clamp_dis_5g;
	uint8 xtal_mode[4];
	uint8 triso2g;
	uint8 tridx2g;
#ifdef BAND5G
	uint8 triso5g[3];
	uint8 tridx5g;
#endif // endif
	uint8 tssi_max_npt;
	uint16 papd_corr_norm;

	/* Subset of registers/tables to identify corruption
	 * cause by pll phase drift
	 */
	uint16 rfseqtbl[3];
	uint16 resamplertbl[3];
	int8 Rssi;
	uint32 rssi_dly;
	int32 rssi_acc;
	int32 rssi_avg;

	uint8 openlp_pwrctrl;
	int32 openlp_refpwrqdB;
	uint8 openlp_gainidx_b[CH_MAX_2G_CHANNEL];
	uint8 openlp_gainidx_a36;
	uint8 openlp_gainidx_a40;
	uint8 openlp_gainidx_a44;
	uint8 openlp_gainidx_a48;
	uint8 openlp_gainidx_a52;
	uint8 openlp_gainidx_a56;
	uint8 openlp_gainidx_a60;
	uint8 openlp_gainidx_a64;
	uint8 openlp_gainidx_a100;
	uint8 openlp_gainidx_a104;
	uint8 openlp_gainidx_a108;
	uint8 openlp_gainidx_a112;
	uint8 openlp_gainidx_a116;
	uint8 openlp_gainidx_a120;
	uint8 openlp_gainidx_a124;
	uint8 openlp_gainidx_a128;
	uint8 openlp_gainidx_a132;
	uint8 openlp_gainidx_a136;
	uint8 openlp_gainidx_a140;
	uint8 openlp_gainidx_a149;
	uint8 openlp_gainidx_a153;
	uint8 openlp_gainidx_a157;
	uint8 openlp_gainidx_a161;
	uint8 openlp_gainidx_a165;
	int32 openlp_pwrlimqdB;
	uint16 openlp_tempcorr;
	uint16 openlp_voltcorr;

	bool rfpll_doubler_2g;
	bool rfpll_doubler_5g;

	bool spuravoid_2g;
	bool spuravoid_5g;
	int8 iqlocalidx2goffs; /* IQLO Cal Idx backoff 2g */
	int8 iqlocalidx5goffs; /* IQLO Cal Idx backoff 5g */
	int8 iqlocalidx_2g; /* IQLO Cal Idx 2g from srom */
	int8 iqlocalidx_5g; /* IQLO Cal Idx 5g from srom */
	int16 iqlocalpwr_2g; /* IQLO Cal Pwr 2g from srom */
	int16 iqlocalpwr_5g; /* IQLO Cal Pwr 5g from srom */
	int8 pmin;    /* pwrMin_range2 */
	int8 pmax;    /* pwrMax_range2 */
	bool temppwrctrl_capable;
	bool rfpll_doubler_mode2g;
	bool rfpll_doubler_mode5g;
	int16 pacalpwr_2g; /* PA Cal Pwr 2g from srom */
	int16 pacalpwr_2g1; /* PA Cal Pwr 2g for second lut from srom */
	int16 pacalpwr_5glo; /* PA Cal Pwr 5g from srom */
	int16 pacalpwr_5glo1; /* PA Cal Pwr 5g for second lut from srom */
	int16 pacalpwr_5g;
	int16 pacalpwr_5g1;
	int16 pacalpwr_5ghi;
	int16 pacalpwr_5ghi1;

	uint8 txgaintbl;
	bool txgaintbl5g;

	int8 loccmode1;

#define LCNPHY_MAX_SAMP_CAP_DATA 128

	uint16 samp_cap_r_idx;
	uint16 samp_cap_w_idx;
	uint32 samp_cap_data[LCNPHY_MAX_SAMP_CAP_DATA];
	int16 lcnphy_ofdm_dig_filt_type_curr;
	int16 lcnphy_cck_dig_filt_type_curr;

	int8 txiqlopapu_2g;
	int16 txiqlopag_2g;
#ifdef BAND5G
	int8 txiqlopapu_5g;
	int16 txiqlopag_5g;
#endif // endif

	bool use_per_modulation_loft_cal;
	bool lpf_cck_tx_byp;
	bool lpf_ofdm_tx_byp;
	int16 di_ofdm;
	int16 dq_ofdm;
	int16 di_cck;
	int16 dq_cck;

	int16 rxgain_tempadj_2g;
	int16 rxgain_tempadj_5gl;
	int16 rxgain_tempadj_5gm;
	int16 rxgain_tempadj_5gh;

	int16 srom_rxgainerr_2g;
	int16 srom_rxgainerr_5gl;
	int16 srom_rxgainerr_5gm;
	int16 srom_rxgainerr_5gh;

	int8 srom_noiselvl_2g;
	int8 srom_noiselvl_5gl;
	int8 srom_noiselvl_5gm;
	int8 srom_noiselvl_5gh;

	int16 rxpath_gain;
	uint32 rxpath_gainselect_power;
	uint32 rxpath_final_power;
	uint32 rxpath_status;
	uint8 rxpath_steps;
	uint8 rxpath_index;
	uint8 rxpath_elna;
	int16 noise_offset_2g;
	int16 noise_offset_5gl;
	int16 noise_offset_5gm;
	int16 noise_offset_5gh;
	uint32 pwr_thresh_2g;
	uint32 pwr_thresh_5g;

	/*
	0 -> Validity
	1 -> RF_REG05E
	2 -> RF_REG02A
	3 -> RF_REG02B
	4 -> RF_REG02C
	5 -> RF_REG02D
	*/
	uint8 logen_mode[6];
#ifdef BAND5G
	uint8 logen_mode5g[5][5];
#endif // endif
	int8 loidacmode_5g;
	int8 pacalidx_5glo;  /* PA Cal Idx 5g lo from srom */
	int8 pacalidx_5ghi;  /* PA Cal Idx 5g hi from srom */
	int8 pacalidx_5glo1; /* PA Cal Idx 5g low chans for second lut from srom */
	int8 pacalidx_5ghi1; /* PA Cal Idx 5g hi chans for second lut from srom */
	int16 pacalidx2g1th;  /* gain index to switch from papd lut 0 to lut1 */
	int16 pacalidx5glo1th;
	int16 pacalidx5g1th;
	int16 pacalidx5ghi1th;
	/* use .5 dB tx gain table step in hardware and .25 dB gain step in software */
	int8 virtual_p25_tx_gain_step;
	int8 papd_num_lut_used;
	uint16 papd_lut1_thres; /* gain index to start using papd lut1 */
	int16 papd_analog_gain_ref_offset;
	uint16 tempsensereg1;
	uint16 tempsensereg2;
	bool rxgaintbl_elna_100;

	/* Tx Gain tables */
	void *gain_table_tx_2g;
#ifdef BAND5G
	void *gain_table_tx_5g;
#endif // endif

	/* Rx Gain tables */
	uint32 num_gain_table_rx_elem_2g;
	void *gain_table_rx_2g;
#ifdef BAND5G
	uint32 num_gain_table_rx_elem_5g;
	void *gain_table_rx_5g;
#endif // endif
	uint8 rxgaintbl_wlbga_aci;
	uint8 rxgaintbl_fcbga;
	uint8 rxgaintbl_wlcsp;

	uint8 rxgaintbl_wlbga_elna_aci;

	uint8 rxgain_backoff_val;
	uint8	rfreg088;	/* rfreg088 values from srom */
	/* Below are the temp correction rate dB/c
	*   The rate is multiplied by 1000 when passed.
	*/
	uint16 rxgain_tempcorr_2g;
	uint16 rxgain_tempcorr_5gh;
	uint16 rxgain_tempcorr_5gm;
	uint16 rxgain_tempcorr_5gl;
	int32 unmod_rssi_offset;

	int16 adcrfseq_2g;
	int16 adcrfseq_5g;

	bool mux_gain_table;

#ifdef LP_P2P_SOFTAP
	int8 pwr_offset_val;
#endif // endif

	uint8 iqlocalst1off_2g;
	uint8 iqlocalst1off_5g;
	uint16 offset_targetpwr;
	uint8 startup_init;
	uint32 swctrlmapext_2g[LCNPHY_SWCTRL_NVRAM_PARAMS];
	uint32 swctrlmapext_5g[LCNPHY_SWCTRL_NVRAM_PARAMS];
	uint8	txpwrindex5g_nvram;
	int16 edonthreshold;
	int16 edoffthreshold;
	/* The below are used for caching rc cal results
	* lpf_rccal_tbl stores the results
	* rccalcache_valid specifies the validity
	*/
	uint16 lpf_rccal_tbl[10];
	uint8 rccalcache_valid;
#ifdef MULTICHAN_4313
	/* The below are used to store the papd tx power
	 * index and papd tx power control value for the
	 * particular channel, implemented for 2g lcnphy
	 * hence only 14 (14 channels)
	 */
	uint8 tx_pwr_idx[15];
	uint16 tx_pwr_ctrl[15];
#endif /* MULTICHAN_4313 */
	uint32 min_txpwrindex_2g;
	uint32 min_txpwrindex_papd;
	uint32 min_txpwrindex_5g;
#ifdef BAND5G
	bool alt_gaintbl_5g;
#endif // endif
};

extern int wlc_lcnphy_tssi_cal(phy_info_t *pi);

extern uint8 wlc_lcnphy_get_bbmult(phy_info_t *pi);
extern void wlc_lcnphy_clear_tx_power_offsets(phy_info_t *pi);
uint8 wlc_lcnphy_get_bbmult_from_index(phy_info_t *pi, int indx);
extern void wlc_lcnphy_read_papdepstbl(phy_info_t *pi, struct bcmstrbuf *b);
extern void wlc_lcnphy_tx_pwr_limit_check(phy_info_t *pi);
extern void wlc_lcnphy_check_pllcorruption(phy_info_t *pi);
extern void wlc_lcnphy_4313war(phy_info_t *pi);
extern void wlc_lcnphy_force_adj_gain(phy_info_t *pi, bool on, int mode);
extern void wlc_phy_lcn_updatemac_rssi(phy_info_t *pi, int8 rssi, int8 antenna);
extern void wlc_lcnphy_aci(phy_info_t *pi, bool on);
extern void wlc_lcnphy_aci_noise_measure(phy_info_t *pi);
extern void wlc_lcnphy_aci_init(phy_info_t *pi);
#if defined(WLTEST)
extern void wlc_phy_get_rxgainerr_lcnphy(phy_info_t *pi, int16 *gainerr);
extern void wlc_phy_get_SROMnoiselvl_lcnphy(phy_info_t *pi, int8 *noiselvl);
extern void wlc_lcnphy_rx_power(phy_info_t *pi, uint16 num_samps,
	uint8 wait_time, uint8 wait_for_crs, phy_iq_est_t* est);
extern void wlc_phy_get_noiseoffset_lcnphy(phy_info_t *pi, int16 *noiseoff);
extern void wlc_lcnphy_get_lna_freq_correction(phy_info_t *pi, int8 *freq_offset_fact);
#endif // endif
#if defined(PHYCAL_CACHING) && defined(BCMDBG)
extern void wlc_phy_cal_cache_dbg_lcnphy(ch_calcache_t *ctx);
#endif // endif

#ifdef WL_LPC
extern void wlc_lcnphy_lpc_write_maclut(wlc_phy_t *ppi);
extern void wlc_lcnphy_lpc_mode(wlc_phy_t *ppi, bool enable);
#endif /* WL_LPC */

/* *************************** REMOVE ************************** */
void wlc_phy_detach_lcnphy(phy_info_t *pi);

#endif /* _wlc_phy_lcn_h_ */
