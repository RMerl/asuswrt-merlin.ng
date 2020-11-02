/*
 * ACPHY Channel Manager module interface (to other PHY modules).
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

#ifndef _phy_ac_chanmgr_h_
#define _phy_ac_chanmgr_h_

#include <phy_api.h>
#include <phy_ac.h>
#include <phy_chanmgr.h>

#define NUM_CHANS_IN_CHAN_BONDING 2
/* forward declaration */
typedef struct phy_ac_chanmgr_info phy_ac_chanmgr_info_t;

/* register/unregister ACPHY specific implementations to/from common */
phy_ac_chanmgr_info_t *phy_ac_chanmgr_register_impl(phy_info_t *pi,
	phy_ac_info_t *aci, phy_chanmgr_info_t *cmn_info);
void phy_ac_chanmgr_unregister_impl(phy_ac_chanmgr_info_t *ac_info);

/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */
/* SMTH MACROS */
#define SMTH_DISABLE                0x0
#define SMTH_ENABLE                 0x1
#define SMTH_ENABLE_NO_NW           0x2
#define SMTH_ENABLE_NO_NW_GD        0x3
#define SMTH_ENABLE_NO_NW_GD_MTE    0x4
#define DISABLE_SIGB_AND_SMTH       0x5
#define SMTH_FOR_TXBF               0x6
#define ALTCLKPLN_ENABLE            0x0

#define PHYBW_20 20
#define PHYBW_40 40
#define PHYBW_80 80
#define PHYBW_160 160

#define ADC_DIV_FAST 1
#define ADC_DIV_SLOW 2

#define SIPO_DIV_FAST 12
#define SIPO_DIV_SLOW 8

#define AFE_DIV_20   8
#define AFE_DIV_40   4
#define AFE_DIV_FAST 3

#define AFE_DIV_BW(bw) ((bw == PHYBW_20) ? AFE_DIV_20 : \
	((bw == PHYBW_40) ? AFE_DIV_40 : AFE_DIV_FAST))

#define TINY_GET_ADC_MODE(pi, chanspec)		\
	((CHSPEC_IS20(chanspec) || CHSPEC_IS40(chanspec)) ?	\
	(pi->u.pi_acphy->use_fast_adc_20_40) : 1)

/* 4335C0 LP Mode definitions */
/* "NORMAL_SETTINGS" --> VCO 2.5V + B0's tuning file changes */
/* "LOW_PWR_SETTINGS_1" --> VCO 2.5V + low power settings + tuning file changes */
/* "LOW_PWR_SETTINGS_2" --> VCO 1.35V + low power settings + tuning file changes */

typedef enum {
	ACPHY_LPMODE_NONE = -1,
	ACPHY_LPMODE_NORMAL_SETTINGS,
	ACPHY_LPMODE_LOW_PWR_SETTINGS_1,
	ACPHY_LPMODE_LOW_PWR_SETTINGS_2
} acphy_lp_modes_t;

typedef struct _chan_info_radio2069 {
	uint16 chan;            /* channel number */
	uint16 freq;            /* in Mhz */

	uint16 RFP_pll_vcocal5;
	uint16 RFP_pll_vcocal6;
	uint16 RFP_pll_vcocal2;
	uint16 RFP_pll_vcocal1;
	uint16 RFP_pll_vcocal11;
	uint16 RFP_pll_vcocal12;
	uint16 RFP_pll_frct2;
	uint16 RFP_pll_frct3;
	uint16 RFP_pll_vcocal10;
	uint16 RFP_pll_xtal3;
	uint16 RFP_pll_vco2;
	uint16 RF0_logen5g_cfg1;
	uint16 RFP_pll_vco8;
	uint16 RFP_pll_vco6;
	uint16 RFP_pll_vco3;
	uint16 RFP_pll_xtalldo1;
	uint16 RFP_pll_hvldo1;
	uint16 RFP_pll_hvldo2;
	uint16 RFP_pll_vco5;
	uint16 RFP_pll_vco4;
	uint16 RFP_pll_lf4;
	uint16 RFP_pll_lf5;
	uint16 RFP_pll_lf7;
	uint16 RFP_pll_lf2;
	uint16 RFP_pll_lf3;
	uint16 RFP_pll_cp4;
	uint16 RFP_pll_dsp1;
	uint16 RFP_pll_dsp2;
	uint16 RFP_pll_dsp3;
	uint16 RFP_pll_dsp4;
	uint16 RFP_pll_dsp6;
	uint16 RFP_pll_dsp7;
	uint16 RFP_pll_dsp8;
	uint16 RFP_pll_dsp9;
	uint16 RF0_logen2g_tune;
	uint16 RFX_lna2g_tune;
	uint16 RFX_txmix2g_cfg1;
	uint16 RFX_pga2g_cfg2;
	uint16 RFX_pad2g_tune;
	uint16 RF0_logen5g_tune1;
	uint16 RF0_logen5g_tune2;
	uint16 RFX_logen5g_rccr;
	uint16 RFX_lna5g_tune;
	uint16 RFX_txmix5g_cfg1;
	uint16 RFX_pga5g_cfg2;
	uint16 RFX_pad5g_tune;
	uint16 RFP_pll_cp5;
	uint16 RF0_afediv1;
	uint16 RF0_afediv2;
	uint16 RFX_adc_cfg5;

	uint16 PHY_BW1a;
	uint16 PHY_BW2;
	uint16 PHY_BW3;
	uint16 PHY_BW4;
	uint16 PHY_BW5;
	uint16 PHY_BW6;
} chan_info_radio2069_t;

typedef struct _chan_info_radio2069revGE16 {
	uint16 chan;            /* channel number */
	uint16 freq;            /* in Mhz */

	uint16 RFP_pll_vcocal5;
	uint16 RFP_pll_vcocal6;
	uint16 RFP_pll_vcocal2;
	uint16 RFP_pll_vcocal1;
	uint16 RFP_pll_vcocal11;
	uint16 RFP_pll_vcocal12;
	uint16 RFP_pll_frct2;
	uint16 RFP_pll_frct3;
	uint16 RFP_pll_vcocal10;
	uint16 RFP_pll_xtal3;
	uint16 RFP_pll_vco2;
	uint16 RFP_logen5g_cfg1;
	uint16 RFP_pll_vco8;
	uint16 RFP_pll_vco6;
	uint16 RFP_pll_vco3;
	uint16 RFP_pll_xtalldo1;
	uint16 RFP_pll_hvldo1;
	uint16 RFP_pll_hvldo2;
	uint16 RFP_pll_vco5;
	uint16 RFP_pll_vco4;
	uint16 RFP_pll_lf4;
	uint16 RFP_pll_lf5;
	uint16 RFP_pll_lf7;
	uint16 RFP_pll_lf2;
	uint16 RFP_pll_lf3;
	uint16 RFP_pll_cp4;
	uint16 RFP_pll_lf6;
	uint16 RFP_logen2g_tune;
	uint16 RF0_lna2g_tune;
	uint16 RF0_txmix2g_cfg1;
	uint16 RF0_pga2g_cfg2;
	uint16 RF0_pad2g_tune;
	uint16 RFP_logen5g_tune1;
	uint16 RFP_logen5g_tune2;
	uint16 RF0_logen5g_rccr;
	uint16 RF0_lna5g_tune;
	uint16 RF0_txmix5g_cfg1;
	uint16 RF0_pga5g_cfg2;
	uint16 RF0_pad5g_tune;
	uint16 PHY_BW1a;
	uint16 PHY_BW2;
	uint16 PHY_BW3;
	uint16 PHY_BW4;
	uint16 PHY_BW5;
	uint16 PHY_BW6;
} chan_info_radio2069revGE16_t;

typedef struct _chan_info_radio2069revGE25 {
	uint16 chan;            /* channel number */
	uint16 freq;            /* in Mhz */

	uint16 RFP_pll_vcocal5;
	uint16 RFP_pll_vcocal6;
	uint16 RFP_pll_vcocal2;
	uint16 RFP_pll_vcocal1;
	uint16 RFP_pll_vcocal11;
	uint16 RFP_pll_vcocal12;
	uint16 RFP_pll_frct2;
	uint16 RFP_pll_frct3;
	uint16 RFP_pll_vcocal10;
	uint16 RFP_pll_xtal3;
	uint16 RFP_pll_cfg3;
	uint16 RFP_pll_vco2;
	uint16 RFP_logen5g_cfg1;
	uint16 RFP_pll_vco8;
	uint16 RFP_pll_vco6;
	uint16 RFP_pll_vco3;
	uint16 RFP_pll_xtalldo1;
	uint16 RFP_pll_hvldo1;
	uint16 RFP_pll_hvldo2;
	uint16 RFP_pll_vco5;
	uint16 RFP_pll_vco4;
	uint16 RFP_pll_lf4;
	uint16 RFP_pll_lf5;
	uint16 RFP_pll_lf7;
	uint16 RFP_pll_lf2;
	uint16 RFP_pll_lf3;
	uint16 RFP_pll_cp4;
	uint16 RFP_pll_lf6;
	uint16 RFP_logen2g_tune;
	uint16 RF0_lna2g_tune;
	uint16 RF0_txmix2g_cfg1;
	uint16 RF0_pga2g_cfg2;
	uint16 RF0_pad2g_tune;
	uint16 RFP_logen5g_tune1;
	uint16 RFP_logen5g_tune2;
	uint16 RF0_logen5g_rccr;
	uint16 RF0_lna5g_tune;
	uint16 RF0_txmix5g_cfg1;
	uint16 RF0_pga5g_cfg2;
	uint16 RF0_pad5g_tune;
	uint16 PHY_BW1a;
	uint16 PHY_BW2;
	uint16 PHY_BW3;
	uint16 PHY_BW4;
	uint16 PHY_BW5;
	uint16 PHY_BW6;
} chan_info_radio2069revGE25_t;

typedef struct _chan_info_radio2069revGE32 {
	uint16 chan;            /* channel number */
	uint16 freq;            /* in Mhz */

	uint16 RFP_pll_vcocal5;
	uint16 RFP_pll_vcocal6;
	uint16 RFP_pll_vcocal2;
	uint16 RFP_pll_vcocal1;
	uint16 RFP_pll_vcocal11;
	uint16 RFP_pll_vcocal12;
	uint16 RFP_pll_frct2;
	uint16 RFP_pll_frct3;
	uint16 RFP_pll_vcocal10;
	uint16 RFP_pll_xtal3;
	uint16 RFP_pll_vco2;
	uint16 RFP_logen5g_cfg1;
	uint16 RFP_pll_vco8;
	uint16 RFP_pll_vco6;
	uint16 RFP_pll_vco3;
	uint16 RFP_pll_xtalldo1;
	uint16 RFP_pll_hvldo1;
	uint16 RFP_pll_hvldo2;
	uint16 RFP_pll_vco5;
	uint16 RFP_pll_vco4;
	uint16 RFP_pll_lf4;
	uint16 RFP_pll_lf5;
	uint16 RFP_pll_lf7;
	uint16 RFP_pll_lf2;
	uint16 RFP_pll_lf3;
	uint16 RFP_pll_cp4;
	uint16 RFP_pll_lf6;
	uint16 RFP_pll_xtal4;
	uint16 RFP_logen2g_tune;
	uint16 RFX_lna2g_tune;
	uint16 RFX_txmix2g_cfg1;
	uint16 RFX_pga2g_cfg2;
	uint16 RFX_pad2g_tune;
	uint16 RFP_logen5g_tune1;
	uint16 RFP_logen5g_tune2;
	uint16 RFP_logen5g_idac1;
	uint16 RFX_lna5g_tune;
	uint16 RFX_txmix5g_cfg1;
	uint16 RFX_pga5g_cfg2;
	uint16 RFX_pad5g_tune;
	uint16 PHY_BW1a;
	uint16 PHY_BW2;
	uint16 PHY_BW3;
	uint16 PHY_BW4;
	uint16 PHY_BW5;
	uint16 PHY_BW6;
} chan_info_radio2069revGE32_t;

typedef struct _chan_info_radio2069revGE25_52MHz {
	uint16 chan;            /* channel number */
	uint16 freq;            /* in Mhz */

	uint16 RFP_pll_vcocal5;
	uint16 RFP_pll_vcocal6;
	uint16 RFP_pll_vcocal2;
	uint16 RFP_pll_vcocal1;
	uint16 RFP_pll_vcocal11;
	uint16 RFP_pll_vcocal12;
	uint16 RFP_pll_frct2;
	uint16 RFP_pll_frct3;
	uint16 RFP_pll_vcocal10;
	uint16 RFP_pll_xtal3;
	uint16 RFP_pll_vco2;
	uint16 RFP_logen5g_cfg1;
	uint16 RFP_pll_vco8;
	uint16 RFP_pll_vco6;
	uint16 RFP_pll_vco3;
	uint16 RFP_pll_xtalldo1;
	uint16 RFP_pll_hvldo1;
	uint16 RFP_pll_hvldo2;
	uint16 RFP_pll_vco5;
	uint16 RFP_pll_vco4;
	uint16 RFP_pll_lf4;
	uint16 RFP_pll_lf5;
	uint16 RFP_pll_lf7;
	uint16 RFP_pll_lf2;
	uint16 RFP_pll_lf3;
	uint16 RFP_pll_cp4;
	uint16 RFP_pll_lf6;
	uint16 RFP_logen2g_tune;
	uint16 RF0_lna2g_tune;
	uint16 RF0_txmix2g_cfg1;
	uint16 RF0_pga2g_cfg2;
	uint16 RF0_pad2g_tune;
	uint16 RFP_logen5g_tune1;
	uint16 RFP_logen5g_tune2;
	uint16 RF0_logen5g_rccr;
	uint16 RF0_lna5g_tune;
	uint16 RF0_txmix5g_cfg1;
	uint16 RF0_pga5g_cfg2;
	uint16 RF0_pad5g_tune;
	uint16 PHY_BW1a;
	uint16 PHY_BW2;
	uint16 PHY_BW3;
	uint16 PHY_BW4;
	uint16 PHY_BW5;
	uint16 PHY_BW6;
} chan_info_radio2069revGE25_52MHz_t;

typedef struct _chan_info_rx_farrow {
#ifndef ACPHY_1X1_ONLY
	uint8 chan;            /* channel number */
	uint16 freq;           /* in Mhz */
	uint16 deltaphase_lo;
	uint16 deltaphase_hi;
	uint16 drift_period;
	uint16 farrow_ctrl;
#else /* ACPHY_1X1_ONLY */
	uint8 chan;            /* channel number */
	uint16 farrow_ctrl_20_40;
	uint32 deltaphase_20_40;
	uint16 farrow_ctrl_80;
	uint32 deltaphase_80;
#endif /* ACPHY_1X1_ONLY */
} chan_info_rx_farrow;

typedef struct _chan_info_tx_farrow {
#ifdef ACPHY_1X1_ONLY
	uint8 chan;            /* channel number */
	uint32 dac_resamp_fcw;
#else /* ACPHY_1X1_ONLY */
	uint16 chan;            /* channel number */
	uint16 freq;            /* in Mhz */
	uint16 MuDelta_l;
	uint16 MuDelta_u;
	uint16 MuDeltaInit_l;
	uint16 MuDeltaInit_u;
#endif /* ACPHY_1X1_ONLY */
} chan_info_tx_farrow;

typedef struct {
		uint8 idx;
		uint16 val;
} sparse_array_entry_t;

extern void wlc_phy_chanspec_set_acphy(phy_info_t *pi, chanspec_t chanspec);
extern void wlc_phy_set_phymode(phy_info_t *pi, chanspec_t chanspec,
	chanspec_t chanspec_sc, uint16 phymode);
extern int wlc_phy_chan2freq_acphy(phy_info_t *pi, uint8 channel, const void **chan_info);
extern void wlc_phy_set_lowpwr_phy_reg_rev3(phy_info_t *pi);
extern void wlc_phy_set_lowpwr_phy_reg(phy_info_t *pi);
extern void wlc_phy_angle_to_phasor_lut(uint16 angle, uint16* packed_word);
extern void wlc_phy_populate_recipcoeffs_acphy(phy_info_t *pi);
extern uint8 wlc_phy_get_chan_freq_range_acphy(phy_info_t *pi, chanspec_t channel);
extern void  wlc_phy_get_chan_freq_range_80p80_acphy(phy_info_t *pi,
		chanspec_t channel, uint8 *freq);
extern uint8 wlc_phy_chan2freq_range_acphy(phy_info_t *pi, chanspec_t chanspec, uint8 channel);
extern uint8 wlc_phy_get_chan_freq_range_srom12_acphy(phy_info_t *pi, chanspec_t channel);
extern void  wlc_phy_get_chan_freq_range_80p80_srom12_acphy(phy_info_t *pi,
		chanspec_t chanspec, uint8 *freq);
extern uint8 wlc_phy_chan2freq_range_srom12_acphy(phy_info_t *pi,
		chanspec_t chanspec, uint8 channel);
extern void wlc_phy_smth(phy_info_t *pi, int8 enable_smth, int8 smth_dumpmode);
extern void wlc_phy_ac_core2core_sync_setup(phy_info_t *pi, bool enable);
extern void wlc_phy_preempt(phy_info_t *pi, bool enable_preempt);
extern void wlc_phy_hwobss(phy_info_t *pi, bool enable_hwobss);
extern void acphy_get_lpmode(phy_info_t *pi);
extern void wlc_phy_lp_mode(phy_info_t *pi, int8 lp_mode);
extern void wlc_phy_force_lpvco_2G(phy_info_t *pi, int8 force_lpvco_2G);
extern void wlc_phy_rxcore_setstate_acphy(wlc_phy_t *pih, uint8 rxcore_bitmask);
extern void wlc_phy_update_rxchains(wlc_phy_t *pih, uint8 *rxcore_bitmask, uint8 *txcore_bitmask);
extern void wlc_phy_restore_rxchains(wlc_phy_t *pih, uint8 enRx, uint8 enTx);
extern uint8 wlc_phy_rxcore_getstate_acphy(wlc_phy_t *pih);
extern bool wlc_phy_is_scan_chan_acphy(phy_info_t *pi);
extern void wlc_phy_resetcca_acphy(phy_info_t *pi);
extern void wlc_phy_radio_tiny_lpf_tx_set(phy_info_t *pi, int8 bq_bw, int8 bq_gain,
	int8 rc_bw_ofdm, int8 rc_bw_cck);
extern void wlc_phy_rxgainctrl_set_gaintbls_acphy(phy_info_t *pi, bool init,
	bool band_change, bool bw_change);
extern void wlc_phy_farrow_setup_acphy(phy_info_t *pi,
	chanspec_t chanspec);
extern void wlc_phy_write_regtbl_fc_from_nvram(phy_info_t *pi);
extern void wlc_phy_write_rx_farrow_tiny(phy_info_t *pi, chanspec_t chanspec,
	chanspec_t chanspec_sc, int sc_mode);
extern void wlc_phy_rxgainctrl_set_gaintbls_acphy_wave2(phy_info_t *pi, uint8 core,
        uint16 gain_tblid, uint16  gainbits_tblid);
extern void wlc_phy_rxgainctrl_set_gaintbls_acphy_tiny(phy_info_t *pi, uint8 core,
	uint16 gain_tblid, uint16 gainbits_tblid);
extern bool wlc_phy_poweron_dac_clocks(phy_info_t *pi, uint8 core, uint16 *orig_dac_clk_pu,
	uint16 *orig_ovr_dac_clk_pu);
extern void wlc_phy_restore_dac_clocks(phy_info_t *pi, uint8 core, uint16 orig_dac_clk_pu,
	uint16 orig_ovr_dac_clk_pu);
extern void wlc_phy_enable_lna_dcc_comp_20691(phy_info_t *pi, bool on);
void wlc_phy_radio20693_sel_logen_mode(phy_info_t *pi);
void wlc_phy_radio20693_sel_logen_2g_mode(phy_info_t *pi, int mode);
void wlc_phy_radio20693_sel_logen_5g_mode(phy_info_t *pi, int mode);
void wlc_phy_radio20693_afe_clkdistribtion_mode(phy_info_t *pi, int mode);
void wlc_phy_radio20693_force_dacbuf_setting(phy_info_t *pi);
#ifdef WL11ULB
extern void wlc_phy_ulb_mode(phy_info_t *pi, uint8 ulb_mode);
#endif /* WL11ULB */

// to help with 3+1 mode and radar scan core
extern int wlc_phy_set_val_sc_chspec(phy_info_t *pi, int32 set_val);
extern int wlc_phy_get_val_sc_chspec(phy_info_t *pi, int32 *ret_val);
extern int wlc_phy_set_val_phymode(phy_info_t *pi, int32 set_val);
extern int wlc_phy_get_val_phymode(phy_info_t *pi, int32 *ret_val);
extern int wlc_phy_get_val_phy_vcore(phy_info_t *pi, int32 *ret_val);
#endif /* _phy_ac_chanmgr_h_ */
