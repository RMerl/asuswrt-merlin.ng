/*
 * ACPHY RADIO control module interface (to other PHY modules).
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

#ifndef _phy_ac_radio_h_
#define _phy_ac_radio_h_

#include <phy_api.h>
#include <phy_ac.h>
#include <phy_radio.h>

#include <wlc_phytbl_20691.h>

#ifdef ATE_BUILD
#include <wl_ate.h>
#endif /* ATE_BUILD */

/* 20693 Radio functions */
typedef enum {
	RADIO_20693_FAST_ADC,
	RADIO_20693_SLOW_ADC
} radio_20693_adc_modes_t;

/*
 * Channel Info table for the 20693 rev 3 (4349 A0).
 */

typedef struct _chan_info_radio20693_pll {
	uint8 chan;            /* channel number */
	uint16 freq;            /* in Mhz */
	/* other stuff */
	uint16 pll_vcocal1;
	uint16 pll_vcocal11;
	uint16 pll_vcocal12;
	uint16 pll_frct2;
	uint16 pll_frct3;
	uint16 pll_hvldo1;
	uint16 pll_lf4;
	uint16 pll_lf5;
	uint16 pll_lf7;
	uint16 pll_lf2;
	uint16 pll_lf3;
	uint16 spare_cfg1;
	uint16 spare_cfg14;
	uint16 spare_cfg13;
	uint16 txmix2g_cfg5;
	uint16 txmix5g_cfg6;
	uint16 pa5g_cfg4;
	uint16 PHY_BW1a;
	uint16 PHY_BW2;
	uint16 PHY_BW3;
	uint16 PHY_BW4;
	uint16 PHY_BW5;
	uint16 PHY_BW6;
} chan_info_radio20693_pll_t;

/* Rev5,7 & 6,8 */
typedef struct _chan_info_radio20693_rffe {
	uint16 lna2g_tune;
	uint16 lna5g_tune;
	uint16 spare_cfg6;
	uint16 spare_cfg7;
	uint16 spare_cfg4;
	uint16 pa2g_cfg2;
} chan_info_radio20693_rffe_t;

typedef struct _chan_info_radio20693_altclkplan {
	uint8 channel;
	uint8 bw;
	uint8 afeclkdiv;
	uint8 adcclkdiv;
	uint8 sipodiv;
	uint8 dacclkdiv;
	uint8 dacdiv;
} chan_info_radio20693_altclkplan_t;

typedef struct _acphy_pmu_core1_off_radregs_t {
	uint16 pll_xtalldo1[PHY_CORE_MAX];
	uint16 pll_cp1[PHY_CORE_MAX];
	uint16 vreg_cfg[PHY_CORE_MAX];
	uint16 vreg_ovr1[PHY_CORE_MAX];
	uint16 pmu_op[PHY_CORE_MAX];
	uint16 pmu_cfg4[PHY_CORE_MAX];
	uint16 logen_cfg2[PHY_CORE_MAX];
	uint16 logen_ovr1[PHY_CORE_MAX];
	uint16 logen_cfg3[PHY_CORE_MAX];
	uint16 logen_ovr2[PHY_CORE_MAX];
	uint16 clk_div_cfg1[PHY_CORE_MAX];
	uint16 clk_div_ovr1[PHY_CORE_MAX];
	uint16 spare_cfg2[PHY_CORE_MAX];
	bool   is_orig;
} acphy_pmu_core1_off_radregs_t;

typedef struct _chan_info_radio20693_pll_wave2 {
	uint8 chan;            /* channel number */
	uint16 freq;            /* in Mhz */
	/* other stuff */
	uint16 wl_xtal_cfg3;
	uint16 pll_vcocal18;
	uint16 pll_vcocal3;
	uint16 pll_vcocal4;
	uint16 pll_vcocal7;
	uint16 pll_vcocal8;
	uint16 pll_vcocal20;
	uint16 pll_vcocal1;
	uint16 pll_vcocal12;
	uint16 pll_vcocal13;
	uint16 pll_vcocal10;
	uint16 pll_vcocal11;
	uint16 pll_vcocal19;
	uint16 pll_vcocal6;
	uint16 pll_vcocal9;
	uint16 pll_vcocal17;
	uint16 pll_vcocal15;
	uint16 pll_vcocal2;
	uint16 pll_vcocal24;
	uint16 pll_vcocal26;
	uint16 pll_vcocal25;
	uint16 pll_vcocal21;
	uint16 pll_vcocal22;
	uint16 pll_vcocal23;
	uint16 pll_vco7;
	uint16 pll_frct2;
	uint16 pll_frct3;
	uint16 pll_vco2;
	uint16 pll_vco6;
	uint16 pll_vco5;
	uint16 pll_vco4;
	uint16 pll_lf4;
	uint16 pll_lf5;
	uint16 pll_lf7;
	uint16 pll_lf2;
	uint16 pll_lf3;
	uint16 pll_cp4;
	uint16 pll_vcocal5;
	uint16 lo2g_logen0_drv;
	uint16 lo2g_vco_drv_cfg2;
	uint16 lo2g_logen1_drv;
	uint16 lo5g_core0_cfg1;
	uint16 lo5g_core1_cfg1;
	uint16 lo5g_core2_cfg1;
	uint16 lo5g_core3_cfg1;
	uint16 lna2g_tune;
	uint16 logen2g_rccr;
	uint16 txmix2g_cfg5;
	uint16 pa2g_cfg2;
	uint16 tx_logen2g_cfg1;
	uint16 lna5g_tune;
	uint16 logen5g_rccr;
	uint16 tx5g_tune;
	uint16 tx_logen5g_cfg1;
	uint16 PHY_BW1a;
	uint16 PHY_BW2;
	uint16 PHY_BW3;
	uint16 PHY_BW4;
	uint16 PHY_BW5;
	uint16 PHY_BW6;
} chan_info_radio20693_pll_wave2_t;

/* forward declaration */
typedef struct phy_ac_radio_info phy_ac_radio_info_t;

/* register/unregister ACPHY specific implementations to/from common */
phy_ac_radio_info_t *phy_ac_radio_register_impl(phy_info_t *pi,
	phy_ac_info_t *aci, phy_radio_info_t *ri);
void phy_ac_radio_unregister_impl(phy_ac_radio_info_t *info);

/* query and parse idcode */
uint32 phy_ac_radio_query_idcode(phy_info_t *pi);
void phy_ac_radio_parse_idcode(phy_info_t *pi, uint32 idcode);

/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */
extern void wlc_phy_set_regtbl_on_pwron_acphy(phy_info_t *pi);
extern void wlc_phy_radio20693_mimo_core1_pmu_off(phy_info_t *pi);
extern void wlc_phy_radio20693_mimo_core1_pmu_on(phy_info_t *pi);
extern void wlc_phy_radio2069_pwrdwn_seq(phy_info_t *pi);
extern void wlc_phy_radio2069_pwrup_seq(phy_info_t *pi);
extern void wlc_acphy_get_radio_loft(phy_info_t *pi, uint8 *ei0,
	uint8 *eq0, uint8 *fi0, uint8 *fq0);
extern void wlc_acphy_set_radio_loft(phy_info_t *pi, uint8, uint8, uint8, uint8);
extern int wlc_phy_chan2freq_20691(phy_info_t *pi, uint8 channel, const chan_info_radio20691_t
	**chan_info);
extern void wlc_phy_radio20693_config_bf_mode(phy_info_t *pi, uint8 core);
extern int wlc_phy_chan2freq_20693(phy_info_t *pi, uint8 channel,
	const chan_info_radio20693_pll_t **chan_info_pll,
	const chan_info_radio20693_rffe_t **chan_info_rffe,
	const chan_info_radio20693_pll_wave2_t **chan_info_pll_wave2);
extern void wlc_phy_chanspec_radio20693_setup(phy_info_t *pi, uint8 ch,
	uint8 toggle_logen_reset, uint8 core, uint8 mode);
extern int8 wlc_phy_tiny_radio_minipmu_cal(phy_info_t *pi);
extern void wlc_phy_radio20693_afeclkpath_setup(phy_info_t *pi, uint8 core,
	radio_20693_adc_modes_t adc_mode, uint8 direct_ctrl_en);
extern void wlc_phy_radio20693_adc_powerupdown(phy_info_t *pi,
	radio_20693_adc_modes_t adc_mode, uint8 pupdbit, uint8 core);
extern int wlc_phy_radio20693_altclkpln_get_chan_row(phy_info_t *pi);
extern void wlc_phy_radio20693_afecal(phy_info_t *pi);
extern void wlc_phy_logen_reset(phy_info_t *pi, uint8 core);
extern void wlc_phy_radio20693_pmu_pll_config_wave2(phy_info_t *pi, uint8 pll_mode);
#endif /* _phy_ac_radio_h_ */
