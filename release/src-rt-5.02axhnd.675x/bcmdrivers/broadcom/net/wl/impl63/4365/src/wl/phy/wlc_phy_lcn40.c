/*
 * PHY and RADIO specific portion of Broadcom BCM43XX 802.11abgn
 * Networking Device Driver.
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
 * $Id: wlc_phy_lcn40.c 586978 2015-09-17 06:20:42Z $*
 */

/* XXX WARNING: phy structure has been changed, read this first
 *
 * This submodule is for LCN40 phy only. It depends on the common submodule wlc_phy_cmn.c
 */

#include <wlc_cfg.h>

#if LCN40CONF != 0
#include <typedefs.h>
#include <qmath.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>
#include <wlioctl.h>
#include <wlc_phy_radio.h>
#include <bitfuncs.h>
#include <bcmdevs.h>
#include <bcmnvram.h>
#include <proto/802.11.h>
#include <hndpmu.h>
#include <bcmsrom_fmt.h>
#include <sbsprom.h>
#include <wlc_phy_hal.h>
#include <wlc_phy_int.h>
#include <wlc_phy_lcn40.h>
#include <sbchipc.h>
#include <wlc_phyreg_lcn40.h>
#include <wlc_phytbl_lcn40.h>
#include <bcmotp.h>
#include <wlc_phy_shim.h>

#include <phy_utils_channel.h>
#include <phy_utils_math.h>
#include <phy_utils_var.h>
#include <phy_utils_reg.h>

#include <phy_lcn40_radio.h>
#include <phy_lcn40_antdiv.h>

/* contains settings from BCM2064_JTAG.xls */
#define PLL_2065_NDIV		65
#define PLL_2065_KVCO       50
#define PLL_2065_LOW_END_VCO 	3200
#define PLL_2065_LOW_END_KVCO_Q16 	1514349
#define PLL_2065_HIGH_END_VCO	4000
#define PLL_2065_HIGH_END_KVCO_Q16  3146447
#define PLL_2065_LOOP_BW_DESIRED_2G	300
#define PLL_2065_LOOP_BW_DESIRED_5G	300
#define PLL_2065_LOOP_BW	266
#define PLL_2065_LF_R1     4250
#define PLL_2065_LF_R2     2000
#define PLL_2065_LF_R3     2000
#define PLL_2065_LF_C1     408
#define PLL_2065_LF_C2_X10     272
#define PLL_2065_LF_C3_X100     816
#define PLL_2065_LF_C4_X100     816
#define PLL_2065_CP_CURRENT     384
#define PLL_2065_D35        408
#define PLL_2065_D32        5178

/* 2065/2067 RFPLL common constants */
#define PLL_2065_VCO_DIVFACT	6
#define PLL_2065_PM_FACTOR_DIV_VAR1	636942675 /* spreadsheet/tcl c39/c14; 4/6280*1e-12 */
#define PLL_2065_VAR2_INV		95085	/* spreadsheet/tcl c15; 1 / (6280^2*1e-12*4/15) */
#define PLL_2065_VAR3			385	/* spreadsheet/tcl c16 */
#define PLL_2065_R1_STEP		320	/* spreadsheet/tcl c18 */
#define PLL_2065_R2_STEP		320	/* spreadsheet/tcl c20 */
#define PLL_2065_R3_STEP		320	/* spreadsheet/tcl c22 */
#define PLL_2065_KPD_MIN		4	/* spreadsheet/tcl c31 */
#define PLL_2065_ENABLE_TIMEOUT_WANTED	8	/* spreadsheet/tcl c43; 0.8 * 10 */
#define PLL_2065_DELAY_BEFORE_OPEN_LOOP	5	/* spreadsheet/tcl d44 */
#define PLL_2065_CAL_REF_TIMEOUT	5	/* spreadsheet/tcl c45 */
#define PLL_2065_CAPS_CAL_SEL		4	/* spreadsheet/tcl d48 */

/* 2065 RFPLL constants */
#define PLL_2065_LOOP_BW_43143	100	/* spreadsheet/tcl d60 for 43143 */
#define PLL_2065_LOOP_BW_43142	150	/* spreadsheet/tcl d60 for 4314/43142 */
#define PLL_2065_R1_MIN		3600	/* spreadsheet/tcl c17 */
#define PLL_2065_R2_MIN		1800	/* spreadsheet/tcl c19 */
#define PLL_2065_R3_MIN		1800	/* spreadsheet/tcl c21 */
#define PLL_2065_C_MAX		330	/* spreadsheet/tcl c38 */
#define PLL_2065_FVCO_LO	3600	/* spreadsheet/tcl c55 */
#define PLL_2065_FVCO_HI	3750	/* spreadsheet/tcl c56 */
#define PLL_2065_C1_REF		330	/* spreadsheet/tcl c64 */
#define PLL_2065_C3_REF		545	/* spreadsheet/tcl c66; 5.45 * 100 */
#define PLL_2065_C4_REF		545	/* spreadsheet/tcl c67; 5.45 * 100 */
#define PLL_2065_R1_REF		10600	/* spreadsheet/tcl c68 */
#define PLL_2065_R2_REF		5000	/* spreadsheet/tcl c69 */
#define PLL_2065_R3_REF		5000	/* spreadsheet/tcl c70 */
#define PLL_2065_C1_MIN		50	/* spreadsheet/tcl c23 */
#define PLL_2065_C1_STEP	9	/* spreadsheet/tcl c24 */
#define PLL_2065_C2_MIN		99	/* spreadsheet/tcl c25; 3.3 * 15 * 2 */
#define PLL_2065_C2_STEP	18	/* spreadsheet/tcl c26; 0.6 * 15 * 2 */
#define PLL_2065_C3_MIN		80	/* spreadsheet/tcl c27; 0.8 * 100 */
#define PLL_2065_C3_STEP	15	/* spreadsheet/tcl c28; 0.15 * 100 */
#define PLL_2065_C4_MIN		80	/* spreadsheet/tcl c29; 0.8 * 100 */
#define PLL_2065_C4_STEP	15	/* spreadsheet/tcl c30; 0.15 * 100 */
#define PLL_2065_KPD_STEP	8	/* spreadsheet/tcl c32 */
#define PLL_2065_KPD_SCALE	3	/* spreadsheet/tcl c33 */
#define PLL_2065_IOFF_STEP	8	/* spreadsheet/tcl c35; 0.8 * 10 */
#define PLL_2065_IOFF_SCALE	20	/* spreadsheet/tcl c36; 2 * 10 */
#define PLL_2065_KVCO_REF	40	/* spreadsheet/tcl c59 */

/* 2067 RFPLL constants */
#define PLL_2067_R1_MIN		850	/* spreadsheet/tcl c17 */
#define PLL_2067_R2_MIN		850	/* spreadsheet/tcl c19 */
#define PLL_2067_R3_MIN		850	/* spreadsheet/tcl c21 */
#define PLL_2067_C_MAX		408	/* spreadsheet/tcl c38 */
#define PLL_2067_FVCO_LO	3200	/* spreadsheet/tcl c55 */
#define PLL_2067_FVCO_HI	4000	/* spreadsheet/tcl c56 */
#define PLL_2067_C1_REF		408	/* spreadsheet/tcl c64 */
#define PLL_2067_C3_REF		816	/* spreadsheet/tcl c66; 8.16 * 100 */
#define PLL_2067_C4_REF		816	/* spreadsheet/tcl c67; 8.16 * 100 */
#define PLL_2067_R1_REF		4250	/* spreadsheet/tcl c68 */
#define PLL_2067_R2_REF		2000	/* spreadsheet/tcl c69 */
#define PLL_2067_R3_REF		2000	/* spreadsheet/tcl c70 */
#define PLL_2067_C1_MIN		36	/* spreadsheet/tcl c23 */
#define PLL_2067_C1_STEP	12	/* spreadsheet/tcl c24 */
#define PLL_2067_C2_MIN		72	/* spreadsheet/tcl c25; 2.4 * 15 * 2 */
#define PLL_2067_C2_STEP	24	/* spreadsheet/tcl c26; 0.8 * 15 * 2 */
#define PLL_2067_C3_MIN		72	/* spreadsheet/tcl c27; 0.72 * 100 */
#define PLL_2067_C3_STEP	24	/* spreadsheet/tcl c28; 0.24 * 100 */
#define PLL_2067_C4_MIN		72	/* spreadsheet/tcl c29; 0.72 * 100 */
#define PLL_2067_C4_STEP	24	/* spreadsheet/tcl c30; 0.24 * 100 */
#define PLL_2067_KPD_STEP	12	/* spreadsheet/tcl c32 */
#define PLL_2067_KPD_SCALE	4	/* spreadsheet/tcl c34 */
#define PLL_2067_IOFF_STEP	20	/* spreadsheet/tcl c35; 2 * 10 */
#define PLL_2067_IOFF_SCALE	10	/* spreadsheet/tcl c36; 1 * 10 */
#define PLL_2067_KVCO_REF	35	/* spreadsheet/tcl c59 */

#define TEMPSENSE 			1
#define VBATSENSE           2

#define PAPD_BLANKING_PROFILE 		0
#define PAPD2LUT			0
#define PAPD_CORR_NORM 			0
#define PAPD_NUM_SKIP_COUNT 39
#define PAPD_BLANKING_THRESHOLD 	0
#define PAPD_STOP_AFTER_LAST_UPDATE	1
#define PAPD_LNA1_ROUT_2G         8
#define PAPD_LNA2_GAIN_2G         1
#define PAPD_LNA2_GAIN_5G         2
#define PAPD_COARSE_BBMULT_STEP		18383

/* PAPD Reverse cal related params */
#define PAPD_SYM_DB		141
#define PAPD_MX2BASE_DB		488
#define PAPD_REV_LUTDB		9
#define PAPD_REV_FAC		43

#define LCN40_TARGET_TSSI  30
#define LCN40_TARGET_PWR  60

#define LCN40_VBAT_OFFSET_433X 34649679 /* 528.712121 * (2 ^ 16) */
#define LCN40_VBAT_SLOPE_433X  8258032 /* 126.007576 * (2 ^ 16) */

#define LCN40_VBAT_SCALE_NOM  53	/* 3.3 * (2 ^ 4) */
#define LCN40_VBAT_SCALE_DEN  432

#define LCN40_TEMPSENSE_OFFSET  80812 /* 78.918 << 10 */
#define LCN40_TEMPSENSE_DEN  2647	/* 2.5847 << 10 */

#define IDLE_TSSI_PER_CAL_EN 1

#define TWO_POWER_RANGE_TXPWR_CTRL 1

#define LCN40_BCK_TSSI_UCODE_POST 1
#define LCN40_BCK_TSSI_DEBUG_LUT_DIRECT !LCN40_BCK_TSSI_UCODE_POST
#if LCN40_BCK_TSSI_DEBUG_LUT_DIRECT
#define LCN40_BCK_TSSI_DEBUG_VIA_DUMMY 0
#endif // endif
#define LCN40_BCK_TSSI_DEBUG 0

#define LCN40_AMAM_TARGET                 179     /* 1.4 * 128 */
#define LCN40_AMAM_TARGET_40MHZ           243     /* 1.9 * 128 */
#define LCN40_AMAM_TARGET_43341           244
#define LCN40_AMAM_TARGET_40MHZ_43341     249
#define LCN40_AMAM_TARGET_43143           244     /* 1.9 * 128 */
#define LCN40_AMAM_THRESHOLD              192     /* 1.5 * 128 */
#define LCN40_AMAM_THRESHOLD_40MHZ        235     /* 1.84 * 128 */
#define LCN40_AMAM_THRESHOLD_43341        250
#define LCN40_AMAM_THRESHOLD_40MHZ_43341  242
#define LCN40_AMAM_THRESHOLD_43143        244     /* 1.9 * 128 */

/* Vmid/Gain settings from tcl proc lcn40_temp_sense_vbatTemp_on */
#define AUXPGA_VBAT_VMID_VAL 0x95
#define AUXPGA_VBAT_GAIN_VAL 0x2
#define AUXPGA_TEMPER_VMID_VAL 0xad
#define AUXPGA_TEMPER_GAIN_VAL 0x1
#define AUXPGA_TEMPER_VMID_VAL_43143 0xa7

/* %%%%%% LCN40PHY macros/structure */

#define LCN40PHY_txgainctrlovrval1_pagain_ovr_val1_SHIFT \
	(LCN40PHY_txgainctrlovrval1_txgainctrl_ovr_val1_SHIFT + 8)
#define LCN40PHY_txgainctrlovrval1_pagain_ovr_val1_MASK \
	(0xff << LCN40PHY_txgainctrlovrval1_pagain_ovr_val1_SHIFT)

#define LCN40PHY_stxtxgainctrlovrval1_pagain_ovr_val1_SHIFT \
	(LCN40PHY_stxtxgainctrlovrval1_stxtxgainctrl_ovr_val1_SHIFT + 8)
#define LCN40PHY_stxtxgainctrlovrval1_pagain_ovr_val1_MASK \
	(0x7f << LCN40PHY_stxtxgainctrlovrval1_pagain_ovr_val1_SHIFT)

#define wlc_lcn40phy_enable_tx_gain_override(pi) \
	wlc_lcn40phy_set_tx_gain_override(pi, TRUE)
#define wlc_lcn40phy_disable_tx_gain_override(pi) \
	wlc_lcn40phy_set_tx_gain_override(pi, FALSE)

/* Turn off all the crs signals to the MAC */
#define wlc_lcn40phy_iqcal_active(pi)	\
	(phy_utils_read_phyreg((pi), LCN40PHY_iqloCalCmd) & \
	(LCN40PHY_iqloCalCmd_iqloCalCmd_MASK | LCN40PHY_iqloCalCmd_iqloCalDFTCmd_MASK))

#define txpwrctrl_off(pi) \
	(0x7 != ((phy_utils_read_phyreg(pi, LCN40PHY_TxPwrCtrlCmd) & 0xE000) >> 13))
#define wlc_lcn40phy_tempsense_based_pwr_ctrl_enabled(pi) \
	(pi->temppwrctrl_capable)
#define wlc_lcn40phy_tssi_based_pwr_ctrl_enabled(pi) \
	(pi->hwpwrctrl_capable)

#define wlc_radio_2065_rc_cal_done(pi) \
	(0 != (phy_utils_read_radioreg(pi, RADIO_2065_RCCAL_LOGIC2) & 0x10))
#define wlc_radio_2065_rcal_done(pi) \
	(0 != (phy_utils_read_radioreg(pi, RADIO_2065_RCAL_CFG) & 0x8))
#define wlc_radio_2065_minipmu_cal_done(pi) \
	(0 != (phy_utils_read_radioreg(pi, RADIO_2065_PMU_STAT) & 0x1))
#define wlc_radio_2065_vco_cal_done(pi) \
	(phy_utils_read_radioreg(pi, RADIO_2065_RFPLL_STATUS) & 0x1)
/* swctrl table */
#define LCN40PHY_I_WL_TX 0 /* PA1 PA0 Tx1 TX0 */
#define LCN40PHY_I_WL_RX 1 /* eLNARx1 eLNARx0 Rx1 RX0 */
#define LCN40PHY_I_WL_RX_ATTN 2 /* eLNAAttnRx1 eLNAAttnRx0 AttnRx1 AttnRx0 */
#define LCN40PHY_I_BT 3 /* Tx eLNARx Rx */
#define LCN40PHY_I_WL_MASK 4 /* ant(1 bit) ovr_en(1 bit) tdm(1 bit) mask(8 bits) */

#define LCN40PHY_MASK_BT_RX	0xff
#define LCN40PHY_SHIFT_BT_RX	0
#define LCN40PHY_MASK_BT_ELNARX	0xff00
#define LCN40PHY_SHIFT_BT_ELNARX	8
#define LCN40PHY_MASK_BT_TX	0xff0000
#define LCN40PHY_SHIFT_BT_TX	16

#define LCN40PHY_MASK_WL_MASK	0xff
#define LCN40PHY_MASK_TDM	0x100
#define LCN40PHY_MASK_OVR_EN	0x200
#define LCN40PHY_MASK_ANT	0x400

#define LCN40PHY_SW_CTRL_MAP_ANT 0x1
#define LCN40PHY_SW_CTRL_MAP_WL_RX 0x2
#define LCN40PHY_SW_CTRL_MAP_WL_TX 0x4
#define LCN40PHY_SW_CTRL_MAP_BT_TX 0x8
#define LCN40PHY_SW_CTRL_MAP_BT_PRIO 0x10
#define LCN40PHY_SW_CTRL_MAP_ELNA 0x20

#define LCN40PHY_SW_CTRL_TBL_LENGTH	64
#define LCN40PHY_SW_CTRL_TBL_WIDTH	16

#define SWCTRL_BT_TX		0x18
#define SWCTRL_OVR_DISABLE	0x40

#define	AFE_CLK_INIT_MODE_TXRX2X	1
#define	AFE_CLK_INIT_MODE_PAPD		0

#define LCN40PHY_TBL_ID_IQLOCAL			0x00
#define LCN40PHY_TBL_ID_MIN_SIG_SQ		0x02
#define LCN40PHY_TBL_ID_TXPWRCTL 		0x07
#define LCN40PHY_TBL_ID_RFSEQ         0x08
#define LCN40PHY_TBL_ID_FLTR_CTRL      		0x0b
#define LCN40PHY_TBL_ID_GAIN_IDX		0x0d
#define LCN40PHY_TBL_ID_SW_CTRL			0x0f
#define LCN40PHY_TBL_ID_NF_CTRL			0x10
#define LCN40PHY_TBL_ID_GAIN_VAL_TBL    0x11
#define LCN40PHY_TBL_ID_GAIN_TBL		0x12
#define LCN40PHY_TBL_ID_SPUR			0x14
#define LCN40PHY_TBL_ID_SAMPLEPLAY		0x15
#define LCN40PHY_TBL_ID_SAMPLEPLAY1		0x1b
#define LCN40PHY_TBL_ID_PAPDCOMPDELTATBL	0x18

#define LCN40PHY_TX_PWR_CTRL_PAPRRATE_OFFSET 	832
#define LCN40PHY_TX_PWR_CTRL_PAPRGAIN_OFFSET 	852
#define LCN40PHY_TX_PWR_CTRL_PAPRGAMMA_OFFSET 	872

#define LCN40PHY_TX_PWR_CTRL_RATE_OFFSET 	832
#define LCN40PHY_TX_PWR_CTRL_MAC_OFFSET 	128
#define LCN40PHY_TX_PWR_CTRL_GAIN_OFFSET 	192
#define LCN40PHY_TX_PWR_CTRL_IQ_OFFSET		320
#define LCN40PHY_TX_PWR_CTRL_LO_OFFSET		448
#define LCN40PHY_TX_PWR_CTRL_PWR_OFFSET		576
#define LCN40PHY_TX_PWR_CTRL_EST_PWR_OFFSET	704

#define LCN40PHY_TX_PWR_CTRL_START_NPT		0
#define LCN40PHY_TX_PWR_CTRL_MAX_NPT			0

#define PHY_NOISE_SAMPLES_DEFAULT 5000

#define LCN40PHY_ACI_DETECT_START      1
#define LCN40PHY_ACI_DETECT_PROGRESS   2
#define LCN40PHY_ACI_DETECT_STOP       3

#define LCN40PHY_ACI_CRSHIFRMLO_TRSH 100
#define LCN40PHY_ACI_GLITCH_TRSH 2000
#define	LCN40PHY_ACI_TMOUT 250		/* Time for CRS HI and FRM LO (in micro seconds) */
#define LCN40PHY_ACI_DETECT_TIMEOUT  2	/* in  seconds */
#define LCN40PHY_ACI_START_DELAY 0

#define LCN40PHY_MAX_CAL_CACHE	   4	/* Max number of cal cache contexts reqd */

#define LCN40PHY_LOWTEMP_TXPWR_IDX_ADJ	20
#define LCN40PHY_UMC_FABID	4
#define LCN40PHY_TSMC_FABID	0
#define wlc_lcn40phy_tx_gain_override_enabled(pi) \
	(0 != (phy_utils_read_phyreg((pi), LCN40PHY_AfeCtrlOvr) & \
	       LCN40PHY_AfeCtrlOvr_dacattctrl_ovr_MASK))

#define LCN40PHY_TSSI_CAL_DBG_EN 0

#define RXIQEST_TIMEOUT 500 /* Timeout in ms */

#define PHY_NOISE_TEST_METRIC 0
#define NOISE_CAL_SHM 0
/* ********************** NOISE CAL ********************************* */

#define k_noise_active_flag 1
#define k_noise_sync_mask 0xfff0
#define k_noise_sync_flag 0x10
#define k_noise_sync_us_per_tick 20
/* k_noise_sync_stop_timeout: in k_noise_sync_us_per_tick units */
#define k_noise_sync_stop_timeout 25

#define k_noise_cal_timeout 4000

#define k_noise_log2_nsamps 6
#define k_noise_spb_samples 128
#define k_noise_min_metric 3000
#define k_noise_max_metric (1 << 28)
#define k_noise_ap0_Q 2
#define k_noise_ap1_Q 0
#define k_noise_rnd ((1 << (k_noise_ap0_Q + k_noise_ap1_Q)) >> 1)
#define k_noise_shft (k_noise_ap0_Q + k_noise_ap1_Q)

#define k_noise_end_state 0
#define k_noise_adj_state 1
#define k_noise_measure_state 2

#define k_noise_cal_gain_adj_increase_threshold -3
#define k_noise_cal_gain_adj_decrease_threshold  3
#define k_noise_cal_max_gain_delta 3
#define k_noise_cal_min_gain_delta 0
#define k_noise_cal_input_pwr_offset_bias 6
#define k_noise_cal_max_positive_po_adj  9
#define k_noise_cal_max_negative_po_adj  6

#define k_noise_cal_min_ok_cnt 100

#define LCN40PHY_TX_PWR_CTRL_START_INDEX_2G	90
#define LCN40PHY_TX_PWR_CTRL_START_INDEX_5G	80

#define LCN40_RSSI_NOMINAL_TEMP	25

/*
 * Init values for 2065/2067 radio regs (autogenerated by 2065_regs_tcl2c.tcl
 * and 2067_regs_tcl2c.tcl).
 * Entries: radio register addr, init value G.
 * Last line (addr FFFF) is dummy as delimiter. This table is used during radio initialization.
 */
#if defined(BCMDBG) || defined(WLTEST)
lcn40phy_radio_regs_t lcn40phy_radio_regs_2067[] = {
{ 0x31,             0x757c },
{ 0x3F,             0x4842 },
{ 0x43,              0x478 },
{ 0x48,              0x400 },
{ 0x49,              0x400 },
{ 0x4F,             0x3000 },
{ 0x50,              0x303 },
{ 0x51,             0x36e2 },
{ 0x53,             0x6000 },
{ 0x54,                0x3 },
{ 0x5A,              0x26f },
{ 0x5B,              0x5af },
{ 0x60,               0x5e },
{ 0x61,              0x335 },
{ 0x62,             0x1616 },
{ 0x63,              0xb76 },
{ 0x64,              0x7a0 },
{ 0x66,             0x1515 },
{ 0x67,               0x77 },
{ 0x68,             0x4f32 },
{ 0x6A,               0x77 },
{ 0x6B,               0x75 },
{ 0x6C,             0x7777 },
{ 0x6E,             0x780a },
{ 0x71,             0x7777 },
{ 0x7F,               0x80 },
{ 0x8D,              0x580 },
{ 0x9E,             0x5540 },
{ 0xA2,             0x7733 },
{ 0xA3,             0x7734 },
{ 0xA4,             0x7777 },
{ 0xFFFF,                0 },
};

lcn40phy_radio_regs_t lcn40phy_radio_regs_2067_rev64[] = {
{ 0x0C,          0x20 },
{ 0x31,        0x777c },
{ 0x34,         0x240 },
{ 0x36,         0x27c },
{ 0x3F,        0x2842 },
{ 0x43,         0x478 },
{ 0x48,         0x400 },
{ 0x49,         0x400 },
{ 0x4F,        0x3000 },
{ 0x50,         0x303 },
{ 0x51,        0x36e2 },
{ 0x53,        0x6000 },
{ 0x54,           0x3 },
{ 0x58,        0x1070 },
{ 0x5A,         0x37f },
{ 0x61,         0x773 },
{ 0x63,        0x1a77 },
{ 0x67,          0x76 },
{ 0x69,        0x141a },
{ 0x6B,          0x73 },
{ 0x6E,        0xc818 },
{ 0x6F,          0xff },
{ 0x71,         0x404 },
{ 0x73,        0x9500 },
{ 0x75,           0x0 },
{ 0x76,        0x4747 },
{ 0x77,        0x7575 },
{ 0x78,          0x29 },
{ 0x7A,        0x1072 },
{ 0x7C,        0x4028 },
{ 0x7E,        0x7000 },
{ 0x84,        0x7041 },
{ 0x85,         0x5ea },
{ 0x87,        0x30fa },
{ 0x88,          0x27 },
{ 0x89,        0x1818 },
{ 0x8A,        0x1818 },
{ 0x8B,           0x5 },
{ 0x8C,         0x50e },
{ 0x8D,         0x580 },
{ 0x93,        0xddc1 },
{ 0x96,        0x1d09 },
{ 0x9A,          0xbd },
{ 0x9B,        0x4e08 },
{ 0x9E,        0x5540 },
{ 0xA2,        0x4434 },
{ 0xA3,        0x4444 },
{ 0xFB,          0xf0 },
{ 0xFC,           0x8 },
{ 0xFE,          0xaa },
{ 0x105,        0x80ff },
{ 0x106,         0x30f },
{ 0x109,         0x404 },
{ 0x10A,           0x5 },
{ 0x10D,           0x4 },
{ 0xFFFF,           0x0 },
};

lcn40phy_radio_regs_t lcn40phy_radio_regs_2065[] = {
	{ 0x07,            0x310 },
	{ 0x26,           0x5000 },
	{ 0x28,            0xf77 },
	{ 0x39,              0xc },
	{ 0x4F,           0x3000 },
	{ 0x50,            0x303 },
	{ 0x51,           0x36e2 },
	{ 0x53,           0x6000 },
	{ 0x54,              0x3 },
	{ 0x5A,            0x3af },
	{ 0x60,             0x3c },
	{ 0x61,              0x1 },
	{ 0x62,           0x1d1d },
	{ 0x63,           0x1c77 },
	{ 0x69,            0xf1c },
	{ 0x6B,             0x77 },
	{ 0x6C,           0x7474 },
	{ 0x72,           0xff27 },
	{ 0x73,           0x2e00 },
	{ 0x74,           0xf300 },
	{ 0x75,           0x1f17 },
	{ 0x76,           0x2020 },
	{ 0x77,           0x7474 },
	{ 0x78,           0x4900 },
	{ 0x79,             0x10 },
	{ 0x84,           0x6666 },
	{ 0x85,             0xb7 },
	{ 0x87,           0x3105 },
	{ 0x88,             0x2a },
	{ 0x89,           0x1515 },
	{ 0x8A,           0x1515 },
	{ 0x8B,              0xe },
	{ 0x8C,            0xe1f },
	{ 0x8D,            0x580 },
	{ 0x93,           0x7271 },
	{ 0x96,            0x709 },
	{ 0x99,              0x0 },
	{ 0x9A,            0x16e },
	{ 0x9B,           0x4ccd },
	{ 0x9D,             0xc0 },
	{ 0x9E,           0x5540 },
	{ 0xC9,           0x8000 },
	{ 0xD3,             0xcc },
	{ 0xFFFF,            0x0 },
};

lcn40phy_radio_regs_t lcn40phy_radio_regs_2065_rev2[] = {
	{ 0x49,            0xc00 },
	{ 0x58,           0x1370 },
	{ 0x5A,            0x36f },
	{ 0x60,             0x4d },
	{ 0x61,            0xff7 },
	{ 0x62,           0x2d2d },
	{ 0x63,           0x1c07 },
	{ 0x68,           0xff30 },
	{ 0x69,            0xb1c },
	{ 0x6A,            0x7ff },
	{ 0x6B,             0x70 },
	{ 0x6C,            0x404 },
	{ 0x72,           0xff00 },
	{ 0x73,           0x2300 },
	{ 0x74,           0xf300 },
	{ 0x75,           0x1f17 },
	{ 0x76,           0x2323 },
	{ 0x77,           0x7676 },
	{ 0xFFFF,            0x0 },
};
#else /* defined(BCMDBG) || defined(WLTEST) */
lcn40phy_radio_regs_t lcn40phy_radio_regs_2067[] = {
{ 0x31,             0x757c },
{ 0x3F,             0x4842 },
{ 0x43,              0x478 },
{ 0x48,              0x400 },
{ 0x49,              0x400 },
{ 0x4F,             0x3000 },
{ 0x50,              0x303 },
{ 0x51,             0x36e2 },
{ 0x53,             0x6000 },
{ 0x54,                0x3 },
{ 0x5A,              0x26f },
{ 0x5B,              0x5af },
{ 0x60,               0x5e },
{ 0x61,              0x335 },
{ 0x62,             0x1616 },
{ 0x63,              0xb76 },
{ 0x64,              0x7a0 },
{ 0x66,             0x1515 },
{ 0x67,               0x77 },
{ 0x68,             0x4f32 },
{ 0x6A,               0x77 },
{ 0x6B,               0x75 },
{ 0x6C,             0x7777 },
{ 0x6E,             0x780a },
{ 0x71,             0x7777 },
{ 0x7F,               0x80 },
{ 0x8D,              0x580 },
{ 0x9E,             0x5540 },
{ 0xA2,             0x7733 },
{ 0xA3,             0x7734 },
{ 0xA4,             0x7777 },
{ 0xFFFF,                0 },
};

lcn40phy_radio_regs_t lcn40phy_radio_regs_2067_rev64[] = {
{ 0x0C,          0x20 },
{ 0x31,        0x777c },
{ 0x34,         0x240 },
{ 0x36,         0x27c },
{ 0x3F,        0x2842 },
{ 0x43,         0x478 },
{ 0x48,         0x400 },
{ 0x49,         0x400 },
{ 0x4F,        0x3000 },
{ 0x50,         0x303 },
{ 0x51,        0x36e2 },
{ 0x53,        0x6000 },
{ 0x54,           0x3 },
{ 0x58,        0x1070 },
{ 0x5A,         0x37f },
{ 0x61,         0x773 },
{ 0x63,        0x1a77 },
{ 0x67,          0x76 },
{ 0x69,        0x141a },
{ 0x6B,          0x73 },
{ 0x6E,        0xc818 },
{ 0x6F,          0xff },
{ 0x71,         0x404 },
{ 0x73,        0x9500 },
{ 0x75,           0x0 },
{ 0x76,        0x4747 },
{ 0x77,        0x7575 },
{ 0x78,          0x29 },
{ 0x7A,        0x1072 },
{ 0x7C,        0x4028 },
{ 0x7E,        0x7000 },
{ 0x84,        0x7041 },
{ 0x85,         0x5ea },
{ 0x87,        0x30fa },
{ 0x88,          0x27 },
{ 0x89,        0x1818 },
{ 0x8A,        0x1818 },
{ 0x8B,           0x5 },
{ 0x8C,         0x50e },
{ 0x8D,         0x580 },
{ 0x93,        0xddc1 },
{ 0x96,        0x1d09 },
{ 0x9A,          0xbd },
{ 0x9B,        0x4e08 },
{ 0x9E,        0x5540 },
{ 0xA2,        0x4434 },
{ 0xA3,        0x4444 },
{ 0xFB,          0xf0 },
{ 0xFC,           0x8 },
{ 0xFE,          0xaa },
{ 0x105,        0x80ff },
{ 0x106,         0x30f },
{ 0x109,         0x404 },
{ 0x10A,           0x5 },
{ 0x10D,           0x4 },
{ 0xFFFF,           0x0 },
};

lcn40phy_radio_regs_t lcn40phy_radio_regs_2065[] = {
	{ 0x07,            0x310 },
	{ 0x26,           0x5000 },
	{ 0x28,            0xf77 },
	{ 0x39,              0xc },
	{ 0x4F,           0x3000 },
	{ 0x50,            0x303 },
	{ 0x51,           0x36e2 },
	{ 0x53,           0x6000 },
	{ 0x54,              0x3 },
	{ 0x5A,            0x3af },
	{ 0x60,             0x3c },
	{ 0x61,              0x1 },
	{ 0x62,           0x1d1d },
	{ 0x63,           0x1c77 },
	{ 0x69,            0xf1c },
	{ 0x6B,             0x77 },
	{ 0x6C,           0x7474 },
	{ 0x72,           0xff27 },
	{ 0x73,           0x2e00 },
	{ 0x74,           0xf300 },
	{ 0x75,           0x1f17 },
	{ 0x76,           0x2020 },
	{ 0x77,           0x7474 },
	{ 0x78,           0x4900 },
	{ 0x79,             0x10 },
	{ 0x84,           0x6666 },
	{ 0x85,             0xb7 },
	{ 0x87,           0x3105 },
	{ 0x88,             0x2a },
	{ 0x89,           0x1515 },
	{ 0x8A,           0x1515 },
	{ 0x8B,              0xe },
	{ 0x8C,            0xe1f },
	{ 0x8D,            0x580 },
	{ 0x93,           0x7271 },
	{ 0x96,            0x709 },
	{ 0x99,              0x0 },
	{ 0x9A,            0x16e },
	{ 0x9B,           0x4ccd },
	{ 0x9D,             0xc0 },
	{ 0x9E,           0x5540 },
	{ 0xC9,           0x8000 },
	{ 0xD3,             0xcc },
	{ 0xFFFF,            0x0 },
};

lcn40phy_radio_regs_t lcn40phy_radio_regs_2065_rev2[] = {
	{ 0x49,            0xc00 },
	{ 0x58,           0x1370 },
	{ 0x5A,            0x36f },
	{ 0x60,             0x4d },
	{ 0x61,            0xff7 },
	{ 0x62,           0x2d2d },
	{ 0x63,           0x1c07 },
	{ 0x68,           0xff30 },
	{ 0x69,            0xb1c },
	{ 0x6A,            0x7ff },
	{ 0x6B,             0x70 },
	{ 0x6C,            0x404 },
	{ 0x72,           0xff00 },
	{ 0x73,           0x2300 },
	{ 0x74,           0xf300 },
	{ 0x75,           0x1f17 },
	{ 0x76,           0x2323 },
	{ 0x77,           0x7676 },
	{ 0xFFFF,            0x0 },
};
#endif /* defined(BCMDBG) || defined(WLTEST) */

/* a band channel info type for 2065 radio used in lcn40phy */
typedef struct _chan_info_2065_lcn40phy {
	uint16 freq;            /* in Mhz */
	uint8  chan;            /* channel number */
	uint8  logen1;		/* AWKWARD: different field */
	uint8  logen2;		/* offsets between 5g  */
	uint8  logen3;		/* and 2g */
	uint8  lna_freq1;
	uint8  lna_freq2;
	uint8  lna_tx;
	uint8  txmix;
	uint8  pga;
	uint8  pad;
} chan_info_2065_lcn40phy_t;

/* Autogenerated by 2065_chantbl_tcl2c.tcl */
static chan_info_2065_lcn40phy_t chan_info_2067_lcn40phy[] = {
{  2412,  1, 0x01, 0x00, 0x00, 0x07, 0x03, 0x00, 0x05, 0x05, 0x05 },
{  2417,  2, 0x01, 0x00, 0x00, 0x07, 0x03, 0x00, 0x05, 0x05, 0x05 },
{  2422,  3, 0x01, 0x00, 0x00, 0x07, 0x03, 0x00, 0x04, 0x05, 0x05 },
{  2427,  4, 0x01, 0x00, 0x00, 0x07, 0x02, 0x00, 0x04, 0x05, 0x05 },
{  2432,  5, 0x01, 0x00, 0x00, 0x06, 0x02, 0x00, 0x04, 0x05, 0x05 },
{  2437,  6, 0x01, 0x00, 0x00, 0x06, 0x01, 0x00, 0x03, 0x05, 0x04 },
{  2442,  7, 0x01, 0x00, 0x00, 0x06, 0x00, 0x00, 0x03, 0x05, 0x04 },
{  2447,  8, 0x01, 0x00, 0x00, 0x06, 0x00, 0x00, 0x03, 0x05, 0x04 },
{  2452,  9, 0x01, 0x00, 0x00, 0x06, 0x00, 0x00, 0x03, 0x05, 0x04 },
{  2457, 10, 0x01, 0x00, 0x00, 0x06, 0x00, 0x00, 0x03, 0x05, 0x04 },
{  2462, 11, 0x01, 0x00, 0x00, 0x06, 0x00, 0x00, 0x03, 0x04, 0x04 },
{  2467, 12, 0x01, 0x00, 0x00, 0x05, 0x00, 0x00, 0x03, 0x04, 0x04 },
{  2472, 13, 0x01, 0x00, 0x00, 0x05, 0x00, 0x00, 0x02, 0x04, 0x04 },
{  2484, 14, 0x01, 0x00, 0x00, 0x05, 0x00, 0x00, 0x02, 0x03, 0x04 },
#ifdef BAND5G
{ 5170,  34, 0x0C, 0x0C, 0x06, 0x08, 0x06, 0x00, 0x0F, 0x0B, 0x0D },
{ 5180,  36, 0x0B, 0x0B, 0x06, 0x08, 0x06, 0x00, 0x0F, 0x0B, 0x0D },
{ 5190,  38, 0x0B, 0x0B, 0x06, 0x08, 0x06, 0x00, 0x0E, 0x0B, 0x0D },
{ 5200,  40, 0x0B, 0x0B, 0x05, 0x07, 0x06, 0x00, 0x0E, 0x0A, 0x0D },
{ 5210,  42, 0x0B, 0x0B, 0x05, 0x07, 0x06, 0x00, 0x0E, 0x0A, 0x0D },
{ 5220,  44, 0x0A, 0x0B, 0x05, 0x07, 0x05, 0x00, 0x0E, 0x0A, 0x0C },
{ 5230,  46, 0x0A, 0x0B, 0x05, 0x07, 0x05, 0x00, 0x0E, 0x0A, 0x0B },
{ 5240,  48, 0x0A, 0x0B, 0x05, 0x07, 0x05, 0x00, 0x0D, 0x0A, 0x0B },
{ 5260,  52, 0x0A, 0x0A, 0x04, 0x06, 0x05, 0x00, 0x0D, 0x09, 0x0B },
{ 5270,  54, 0x09, 0x0A, 0x04, 0x06, 0x05, 0x00, 0x0D, 0x09, 0x0B },
{ 5280,  56, 0x09, 0x0A, 0x04, 0x06, 0x05, 0x00, 0x0D, 0x09, 0x0B },
{ 5300,  60, 0x09, 0x08, 0x03, 0x06, 0x05, 0x00, 0x0D, 0x08, 0x0B },
{ 5310,  62, 0x09, 0x08, 0x03, 0x06, 0x05, 0x00, 0x0C, 0x08, 0x07 },
{ 5320,  64, 0x09, 0x08, 0x03, 0x06, 0x04, 0x00, 0x0C, 0x08, 0x07 },
{ 5500, 100, 0x05, 0x08, 0x00, 0x04, 0x03, 0x00, 0x09, 0x04, 0x07 },
{ 5510, 102, 0x05, 0x08, 0x00, 0x04, 0x03, 0x00, 0x08, 0x04, 0x07 },
{ 5520, 104, 0x05, 0x08, 0x00, 0x04, 0x03, 0x00, 0x09, 0x04, 0x07 },
{ 5540, 108, 0x04, 0x08, 0x00, 0x04, 0x02, 0x00, 0x08, 0x04, 0x07 },
{ 5550, 110, 0x04, 0x08, 0x00, 0x03, 0x02, 0x00, 0x08, 0x04, 0x07 },
{ 5560, 112, 0x04, 0x08, 0x00, 0x03, 0x02, 0x00, 0x08, 0x03, 0x06 },
{ 5580, 116, 0x03, 0x07, 0x00, 0x03, 0x02, 0x00, 0x08, 0x03, 0x06 },
{ 5590, 118, 0x03, 0x07, 0x00, 0x03, 0x02, 0x00, 0x08, 0x03, 0x06 },
{ 5600, 120, 0x03, 0x07, 0x00, 0x02, 0x02, 0x00, 0x08, 0x03, 0x06 },
{ 5620, 124, 0x03, 0x07, 0x00, 0x02, 0x02, 0x00, 0x08, 0x03, 0x06 },
{ 5630, 126, 0x03, 0x07, 0x00, 0x02, 0x02, 0x00, 0x07, 0x02, 0x06 },
{ 5640, 128, 0x03, 0x07, 0x00, 0x02, 0x02, 0x00, 0x06, 0x02, 0x06 },
{ 5660, 132, 0x02, 0x07, 0x00, 0x02, 0x01, 0x00, 0x06, 0x02, 0x06 },
{ 5670, 134, 0x02, 0x07, 0x00, 0x02, 0x01, 0x00, 0x06, 0x02, 0x06 },
{ 5680, 136, 0x02, 0x06, 0x00, 0x02, 0x01, 0x00, 0x06, 0x01, 0x06 },
{ 5700, 140, 0x02, 0x06, 0x00, 0x02, 0x01, 0x00, 0x06, 0x01, 0x06 },
{ 5745, 149, 0x01, 0x06, 0x00, 0x01, 0x01, 0x00, 0x05, 0x00, 0x06 },
{ 5755, 151, 0x01, 0x06, 0x00, 0x01, 0x01, 0x00, 0x05, 0x00, 0x06 },
{ 5765, 153, 0x01, 0x06, 0x00, 0x01, 0x01, 0x00, 0x05, 0x00, 0x05 },
{ 5785, 157, 0x00, 0x06, 0x00, 0x01, 0x00, 0x00, 0x05, 0x00, 0x04 },
{ 5795, 159, 0x00, 0x06, 0x00, 0x01, 0x00, 0x00, 0x04, 0x00, 0x04 },
{ 5805, 161, 0x00, 0x05, 0x00, 0x01, 0x00, 0x00, 0x04, 0x00, 0x03 },
{ 5825, 165, 0x00, 0x05, 0x00, 0x01, 0x00, 0x00, 0x03, 0x00, 0x03 },
{ 4920, 184, 0x0F, 0x0F, 0x0B, 0x0B, 0x09, 0x00, 0x0F, 0x0F, 0x0F },
{ 4925, 185, 0x0F, 0x0F, 0x0B, 0x0B, 0x09, 0x00, 0x0F, 0x0F, 0x0F },
{ 4935, 187, 0x0F, 0x0F, 0x0B, 0x0B, 0x09, 0x00, 0x0F, 0x0F, 0x0F },
{ 4940, 188, 0x0F, 0x0F, 0x0B, 0x0B, 0x09, 0x00, 0x0F, 0x0F, 0x0F },
{ 4945, 189, 0x0F, 0x0E, 0x0A, 0x0A, 0x09, 0x00, 0x0F, 0x0F, 0x0F },
{ 4960, 192, 0x0F, 0x0E, 0x0A, 0x0A, 0x09, 0x00, 0x0F, 0x0F, 0x0F },
{ 4980, 196, 0x0F, 0x0E, 0x0A, 0x0A, 0x08, 0x00, 0x0F, 0x0F, 0x0E },
{ 5035, 207, 0x0F, 0x0D, 0x09, 0x0A, 0x08, 0x00, 0x0F, 0x0E, 0x0E },
{ 5040, 208, 0x0F, 0x0D, 0x09, 0x09, 0x08, 0x00, 0x0F, 0x0E, 0x0E },
{ 5045, 209, 0x0E, 0x0D, 0x09, 0x09, 0x08, 0x00, 0x0F, 0x0E, 0x0E },
{ 5050, 210, 0x0E, 0x0D, 0x08, 0x09, 0x08, 0x00, 0x0F, 0x0E, 0x0E },
{ 5060, 212, 0x0E, 0x0D, 0x08, 0x09, 0x07, 0x00, 0x0F, 0x0D, 0x0E },
{ 5080, 216, 0x0D, 0x0D, 0x08, 0x08, 0x07, 0x00, 0x0F, 0x0D, 0x0E },
#endif /* BAND5G */
};

static chan_info_2065_lcn40phy_t chan_info_2067_rev60_lcn40phy[] = {
{  2412,  1, 0x01, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00 },
{  2417,  2, 0x01, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00 },
{  2422,  3, 0x01, 0x01, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00 },
{  2427,  4, 0x01, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00 },
{  2432,  5, 0x01, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00 },
{  2437,  6, 0x01, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00 },
{  2442,  7, 0x01, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00 },
{  2447,  8, 0x01, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00 },
{  2452,  9, 0x01, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00 },
{  2457, 10, 0x01, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00 },
{  2462, 11, 0x01, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00 },
{  2467, 12, 0x01, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00 },
{  2472, 13, 0x01, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00 },
{  2484, 14, 0x01, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00 },
#ifdef BAND5G
{ 5170,  34, 0x0E, 0x0B, 0x02, 0x07, 0x07, 0x00, 0x0C, 0x0F, 0x0D },
{ 5180,  36, 0x0E, 0x0B, 0x02, 0x07, 0x07, 0x00, 0x0C, 0x0F, 0x0D },
{ 5190,  38, 0x0E, 0x0B, 0x02, 0x07, 0x07, 0x00, 0x0C, 0x0F, 0x0D },
{ 5200,  40, 0x0E, 0x0B, 0x02, 0x07, 0x06, 0x00, 0x0C, 0x0F, 0x0D },
{ 5220,  44, 0x0D, 0x0B, 0x01, 0x07, 0x06, 0x00, 0x0B, 0x0F, 0x0D },
{ 5230,  46, 0x0D, 0x0B, 0x01, 0x07, 0x06, 0x00, 0x0A, 0x0F, 0x0C },
{ 5240,  48, 0x0D, 0x0A, 0x01, 0x07, 0x06, 0x00, 0x0A, 0x0F, 0x0C },
{ 5260,  52, 0x0D, 0x0A, 0x01, 0x07, 0x06, 0x00, 0x09, 0x0F, 0x0B },
{ 5270,  54, 0x0D, 0x0A, 0x01, 0x07, 0x06, 0x00, 0x09, 0x0F, 0x0B },
{ 5280,  56, 0x0C, 0x0A, 0x00, 0x06, 0x06, 0x00, 0x09, 0x0F, 0x0B },
{ 5300,  60, 0x0C, 0x0A, 0x00, 0x06, 0x05, 0x00, 0x09, 0x0F, 0x0B },
{ 5310,  62, 0x0C, 0x0A, 0x00, 0x06, 0x05, 0x00, 0x08, 0x0F, 0x0C },
{ 5320,  64, 0x0B, 0x0A, 0x00, 0x06, 0x05, 0x00, 0x08, 0x0F, 0x0C },
{ 5500, 100, 0x08, 0x08, 0x00, 0x05, 0x03, 0x00, 0x04, 0x0D, 0x08 },
{ 5510, 102, 0x08, 0x08, 0x00, 0x05, 0x03, 0x00, 0x04, 0x0D, 0x08 },
{ 5520, 104, 0x08, 0x08, 0x00, 0x04, 0x03, 0x00, 0x04, 0x0D, 0x08 },
{ 5540, 108, 0x07, 0x07, 0x00, 0x04, 0x03, 0x00, 0x03, 0x0D, 0x09 },
{ 5550, 110, 0x07, 0x07, 0x00, 0x04, 0x03, 0x00, 0x03, 0x0C, 0x08 },
{ 5560, 112, 0x06, 0x07, 0x00, 0x04, 0x03, 0x00, 0x02, 0x0C, 0x08 },
{ 5580, 116, 0x06, 0x07, 0x00, 0x04, 0x03, 0x00, 0x02, 0x0B, 0x07 },
{ 5590, 118, 0x06, 0x07, 0x00, 0x04, 0x03, 0x00, 0x02, 0x0A, 0x07 },
{ 5600, 120, 0x06, 0x07, 0x00, 0x04, 0x02, 0x00, 0x02, 0x0A, 0x06 },
{ 5620, 124, 0x06, 0x07, 0x00, 0x04, 0x02, 0x00, 0x01, 0x09, 0x06 },
{ 5630, 126, 0x06, 0x07, 0x00, 0x04, 0x02, 0x00, 0x01, 0x09, 0x06 },
{ 5640, 128, 0x06, 0x06, 0x00, 0x04, 0x02, 0x00, 0x01, 0x0A, 0x06 },
{ 5660, 132, 0x05, 0x06, 0x00, 0x03, 0x02, 0x00, 0x01, 0x0A, 0x06 },
{ 5670, 134, 0x05, 0x06, 0x00, 0x03, 0x02, 0x00, 0x01, 0x0A, 0x06 },
{ 5680, 136, 0x04, 0x06, 0x00, 0x03, 0x02, 0x00, 0x00, 0x0A, 0x06 },
{ 5700, 140, 0x04, 0x06, 0x00, 0x03, 0x01, 0x00, 0x00, 0x09, 0x05 },
{ 5745, 149, 0x04, 0x06, 0x00, 0x03, 0x01, 0x00, 0x00, 0x07, 0x04 },
{ 5755, 151, 0x04, 0x06, 0x00, 0x03, 0x01, 0x00, 0x00, 0x07, 0x04 },
{ 5765, 153, 0x04, 0x06, 0x00, 0x02, 0x01, 0x00, 0x00, 0x07, 0x04 },
{ 5785, 157, 0x03, 0x05, 0x00, 0x02, 0x01, 0x00, 0x00, 0x07, 0x04 },
{ 5795, 159, 0x03, 0x05, 0x00, 0x02, 0x01, 0x00, 0x00, 0x08, 0x04 },
{ 5805, 161, 0x03, 0x05, 0x00, 0x02, 0x01, 0x00, 0x00, 0x08, 0x04 },
{ 5825, 165, 0x03, 0x05, 0x00, 0x02, 0x00, 0x00, 0x00, 0x07, 0x04 },
{ 4920, 184, 0x0F, 0x0C, 0x04, 0x08, 0x08, 0x00, 0x0D, 0x0F, 0x0F },
{ 4925, 185, 0x0F, 0x0C, 0x04, 0x08, 0x08, 0x00, 0x0D, 0x0F, 0x0F },
{ 4935, 187, 0x0F, 0x0C, 0x04, 0x08, 0x08, 0x00, 0x0D, 0x0F, 0x0F },
{ 4940, 188, 0x0F, 0x0C, 0x04, 0x08, 0x08, 0x00, 0x0D, 0x0F, 0x0F },
{ 4945, 189, 0x0F, 0x0C, 0x04, 0x08, 0x08, 0x00, 0x0D, 0x0F, 0x0F },
{ 4960, 192, 0x0F, 0x0C, 0x04, 0x08, 0x08, 0x00, 0x0D, 0x0F, 0x0F },
{ 4980, 196, 0x0F, 0x0C, 0x04, 0x08, 0x08, 0x00, 0x0D, 0x0F, 0x0F },
{ 5035, 207, 0x0F, 0x0C, 0x04, 0x08, 0x08, 0x00, 0x0D, 0x0F, 0x0F },
{ 5040, 208, 0x0F, 0x0C, 0x04, 0x08, 0x08, 0x00, 0x0D, 0x0F, 0x0F },
{ 5045, 209, 0x0F, 0x0C, 0x04, 0x08, 0x08, 0x00, 0x0D, 0x0F, 0x0F },
{ 5050, 210, 0x0F, 0x0C, 0x04, 0x08, 0x08, 0x00, 0x0D, 0x0F, 0x0F },
{ 5060, 212, 0x0F, 0x0C, 0x04, 0x08, 0x08, 0x00, 0x0D, 0x0F, 0x0F },
{ 5080, 216, 0x0F, 0x0C, 0x04, 0x08, 0x08, 0x00, 0x0D, 0x0F, 0x0F },
#endif /* BAND5G */
};

static chan_info_2065_lcn40phy_t chan_info_2067_rev64_lcn40phy[] = {
{ 2412,   1, 0x01, 0x00, 0x00, 0x07, 0x05, 0x00, 0x03, 0x01, 0x01 },
{ 2417,   2, 0x01, 0x00, 0x00, 0x07, 0x05, 0x00, 0x02, 0x01, 0x00 },
{ 2422,   3, 0x01, 0x00, 0x00, 0x07, 0x04, 0x00, 0x02, 0x01, 0x00 },
{ 2427,   4, 0x01, 0x00, 0x00, 0x07, 0x04, 0x00, 0x02, 0x01, 0x00 },
{ 2432,   5, 0x01, 0x00, 0x00, 0x07, 0x03, 0x00, 0x02, 0x01, 0x00 },
{ 2437,   6, 0x01, 0x00, 0x00, 0x06, 0x03, 0x00, 0x02, 0x00, 0x00 },
{ 2442,   7, 0x01, 0x00, 0x00, 0x06, 0x02, 0x00, 0x02, 0x00, 0x00 },
{ 2447,   8, 0x01, 0x00, 0x00, 0x06, 0x02, 0x00, 0x02, 0x00, 0x00 },
{ 2452,   9, 0x01, 0x00, 0x00, 0x06, 0x01, 0x00, 0x01, 0x00, 0x00 },
{ 2457,  10, 0x01, 0x00, 0x00, 0x06, 0x01, 0x00, 0x00, 0x00, 0x00 },
{ 2462,  11, 0x01, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00 },
{ 2467,  12, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00 },
{ 2472,  13, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00 },
{ 2484,  14, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00 },
#ifdef BAND5G
{ 5170,  34, 0x0D, 0x0F, 0x0F, 0x07, 0x06, 0x00, 0x0B, 0x08, 0x0E },
{ 5180,  36, 0x0D, 0x0F, 0x0F, 0x07, 0x06, 0x00, 0x0B, 0x08, 0x0E },
{ 5190,  38, 0x0D, 0x0F, 0x0F, 0x07, 0x06, 0x00, 0x0A, 0x08, 0x0E },
{ 5200,  40, 0x0C, 0x0F, 0x0F, 0x07, 0x06, 0x00, 0x0A, 0x08, 0x0E },
{ 5220,  44, 0x0C, 0x0F, 0x0F, 0x07, 0x06, 0x00, 0x09, 0x08, 0x0D },
{ 5230,  46, 0x0C, 0x0F, 0x0F, 0x07, 0x06, 0x00, 0x09, 0x08, 0x0D },
{ 5240,  48, 0x0C, 0x0F, 0x0F, 0x07, 0x06, 0x00, 0x08, 0x08, 0x0D },
{ 5260,  52, 0x0B, 0x0F, 0x0F, 0x07, 0x06, 0x00, 0x08, 0x08, 0x0D },
{ 5270,  54, 0x0B, 0x0F, 0x0F, 0x07, 0x06, 0x00, 0x08, 0x08, 0x0D },
{ 5280,  56, 0x0B, 0x0F, 0x0F, 0x07, 0x06, 0x00, 0x08, 0x08, 0x0C },
{ 5300,  60, 0x0A, 0x0F, 0x0E, 0x06, 0x05, 0x00, 0x08, 0x07, 0x0C },
{ 5310,  62, 0x0A, 0x0F, 0x0E, 0x06, 0x05, 0x00, 0x07, 0x06, 0x0C },
{ 5320,  64, 0x0A, 0x0F, 0x0E, 0x06, 0x05, 0x00, 0x06, 0x05, 0x0C },
{ 5500, 100, 0x07, 0x0D, 0x0B, 0x05, 0x04, 0x00, 0x03, 0x03, 0x09 },
{ 5510, 102, 0x07, 0x0D, 0x0B, 0x05, 0x04, 0x00, 0x03, 0x03, 0x08 },
{ 5520, 104, 0x06, 0x0D, 0x0A, 0x05, 0x04, 0x00, 0x02, 0x02, 0x08 },
{ 5540, 108, 0x06, 0x0C, 0x0A, 0x04, 0x03, 0x00, 0x02, 0x02, 0x07 },
{ 5550, 110, 0x06, 0x0C, 0x09, 0x04, 0x03, 0x00, 0x02, 0x02, 0x07 },
{ 5560, 112, 0x06, 0x0C, 0x09, 0x04, 0x02, 0x00, 0x02, 0x02, 0x07 },
{ 5580, 116, 0x05, 0x0C, 0x08, 0x04, 0x02, 0x00, 0x01, 0x01, 0x07 },
{ 5590, 118, 0x05, 0x0C, 0x08, 0x04, 0x02, 0x00, 0x01, 0x01, 0x07 },
{ 5600, 120, 0x05, 0x0C, 0x08, 0x04, 0x02, 0x00, 0x00, 0x00, 0x07 },
{ 5620, 124, 0x04, 0x0C, 0x08, 0x04, 0x02, 0x00, 0x00, 0x00, 0x07 },
{ 5630, 126, 0x04, 0x0B, 0x07, 0x03, 0x01, 0x00, 0x00, 0x00, 0x07 },
{ 5640, 128, 0x04, 0x0B, 0x07, 0x03, 0x01, 0x00, 0x00, 0x00, 0x06 },
{ 5660, 132, 0x04, 0x0B, 0x07, 0x03, 0x01, 0x00, 0x00, 0x00, 0x06 },
{ 5670, 134, 0x04, 0x0B, 0x07, 0x03, 0x01, 0x00, 0x00, 0x00, 0x06 },
{ 5680, 136, 0x04, 0x0B, 0x07, 0x03, 0x01, 0x00, 0x00, 0x00, 0x06 },
{ 5700, 140, 0x03, 0x0B, 0x06, 0x03, 0x01, 0x00, 0x00, 0x00, 0x05 },
{ 5745, 149, 0x02, 0x0B, 0x05, 0x02, 0x00, 0x00, 0x00, 0x00, 0x05 },
{ 5755, 151, 0x02, 0x0B, 0x05, 0x02, 0x00, 0x00, 0x00, 0x00, 0x05 },
{ 5765, 153, 0x02, 0x0B, 0x05, 0x02, 0x00, 0x00, 0x00, 0x00, 0x05 },
{ 5785, 157, 0x02, 0x0A, 0x05, 0x02, 0x00, 0x00, 0x00, 0x00, 0x05 },
{ 5795, 159, 0x02, 0x0A, 0x05, 0x02, 0x00, 0x00, 0x00, 0x00, 0x04 },
{ 5805, 161, 0x02, 0x0A, 0x05, 0x02, 0x00, 0x00, 0x00, 0x00, 0x04 },
{ 5825, 165, 0x01, 0x0A, 0x05, 0x02, 0x00, 0x00, 0x00, 0x00, 0x05 },
{ 4920, 184, 0x0F, 0x0F, 0x0F, 0x08, 0x08, 0x00, 0x0C, 0x0A, 0x0F },
{ 4925, 185, 0x0F, 0x0F, 0x0F, 0x08, 0x08, 0x00, 0x0C, 0x0A, 0x0F },
{ 4935, 187, 0x0F, 0x0F, 0x0F, 0x08, 0x08, 0x00, 0x0C, 0x0A, 0x0F },
{ 4940, 188, 0x0F, 0x0F, 0x0F, 0x08, 0x08, 0x00, 0x0C, 0x0A, 0x0F },
{ 4945, 189, 0x0F, 0x0F, 0x0F, 0x08, 0x08, 0x00, 0x0C, 0x0A, 0x0F },
{ 4960, 192, 0x0F, 0x0F, 0x0F, 0x08, 0x08, 0x00, 0x0C, 0x0A, 0x0F },
{ 4980, 196, 0x0F, 0x0F, 0x0F, 0x08, 0x08, 0x00, 0x0C, 0x0A, 0x0F },
{ 5035, 207, 0x0F, 0x0F, 0x0F, 0x08, 0x08, 0x00, 0x0C, 0x0A, 0x0F },
{ 5040, 208, 0x0F, 0x0F, 0x0F, 0x08, 0x08, 0x00, 0x0C, 0x0A, 0x0F },
{ 5045, 209, 0x0F, 0x0F, 0x0F, 0x08, 0x08, 0x00, 0x0C, 0x0A, 0x0F },
{ 5050, 210, 0x0F, 0x0F, 0x0F, 0x08, 0x08, 0x00, 0x0C, 0x0A, 0x0F },
{ 5060, 212, 0x0F, 0x0F, 0x0F, 0x08, 0x08, 0x00, 0x0C, 0x0A, 0x0F },
{ 5080, 216, 0x0F, 0x0F, 0x0F, 0x08, 0x08, 0x00, 0x0C, 0x0A, 0x0F },
#endif /* BAND5G */
};

static chan_info_2065_lcn40phy_t chan_info_2065_lcn40phy[] = {
{  2412,  1, 0x01, 0x00, 0x00, 0x08, 0x00, 0x00, 0x04, 0x01, 0x07 },
{  2417,  2, 0x01, 0x00, 0x00, 0x08, 0x00, 0x00, 0x04, 0x01, 0x07 },
{  2422,  3, 0x01, 0x00, 0x00, 0x08, 0x00, 0x00, 0x04, 0x01, 0x07 },
{  2427,  4, 0x01, 0x00, 0x00, 0x07, 0x00, 0x00, 0x04, 0x01, 0x07 },
{  2432,  5, 0x01, 0x00, 0x00, 0x07, 0x00, 0x00, 0x03, 0x01, 0x07 },
{  2437,  6, 0x01, 0x00, 0x00, 0x07, 0x00, 0x00, 0x03, 0x01, 0x07 },
{  2442,  7, 0x01, 0x00, 0x00, 0x07, 0x00, 0x00, 0x03, 0x01, 0x07 },
{  2447,  8, 0x01, 0x00, 0x00, 0x07, 0x00, 0x00, 0x03, 0x01, 0x07 },
{  2452,  9, 0x01, 0x00, 0x00, 0x07, 0x00, 0x00, 0x03, 0x01, 0x07 },
{  2457, 10, 0x01, 0x00, 0x00, 0x07, 0x00, 0x00, 0x03, 0x01, 0x07 },
{  2462, 11, 0x01, 0x00, 0x00, 0x07, 0x00, 0x00, 0x03, 0x01, 0x06 },
{  2467, 12, 0x01, 0x00, 0x00, 0x07, 0x00, 0x00, 0x03, 0x01, 0x06 },
{  2472, 13, 0x01, 0x00, 0x00, 0x06, 0x00, 0x00, 0x03, 0x00, 0x06 },
{  2484, 14, 0x01, 0x00, 0x00, 0x06, 0x00, 0x00, 0x03, 0x00, 0x06 },
#ifdef BAND5G
{ 5170,  34, 0x0B, 0x0B, 0x05, 0x08, 0x06, 0x00, 0x0F, 0x0F, 0x0B },
{ 5180,  36, 0x0B, 0x0B, 0x05, 0x08, 0x06, 0x00, 0x0F, 0x0F, 0x0B },
{ 5190,  38, 0x0B, 0x0A, 0x04, 0x08, 0x06, 0x00, 0x0F, 0x0F, 0x0B },
{ 5200,  40, 0x0B, 0x0A, 0x04, 0x07, 0x06, 0x00, 0x0F, 0x0F, 0x0B },
{ 5220,  44, 0x0A, 0x0A, 0x03, 0x07, 0x05, 0x00, 0x0F, 0x0F, 0x0B },
{ 5230,  46, 0x0A, 0x09, 0x03, 0x07, 0x05, 0x00, 0x0F, 0x0F, 0x0B },
{ 5240,  48, 0x0A, 0x09, 0x02, 0x07, 0x05, 0x00, 0x0F, 0x0F, 0x0B },
{ 5260,  52, 0x09, 0x09, 0x02, 0x06, 0x05, 0x00, 0x0F, 0x0F, 0x0A },
{ 5270,  54, 0x09, 0x09, 0x02, 0x06, 0x05, 0x00, 0x0F, 0x0F, 0x09 },
{ 5280,  56, 0x09, 0x09, 0x01, 0x06, 0x05, 0x00, 0x0F, 0x0F, 0x09 },
{ 5300,  60, 0x08, 0x09, 0x01, 0x06, 0x05, 0x00, 0x0E, 0x0F, 0x08 },
{ 5310,  62, 0x08, 0x08, 0x01, 0x06, 0x05, 0x00, 0x0E, 0x0F, 0x08 },
{ 5320,  64, 0x08, 0x08, 0x01, 0x06, 0x04, 0x00, 0x0A, 0x0E, 0x08 },
{ 5500, 100, 0x05, 0x07, 0x00, 0x04, 0x03, 0x00, 0x08, 0x0A, 0x08 },
{ 5510, 102, 0x05, 0x07, 0x00, 0x04, 0x03, 0x00, 0x08, 0x0A, 0x08 },
{ 5520, 104, 0x04, 0x07, 0x00, 0x04, 0x03, 0x00, 0x08, 0x0A, 0x08 },
{ 5540, 108, 0x04, 0x07, 0x00, 0x04, 0x02, 0x00, 0x07, 0x0A, 0x07 },
{ 5550, 110, 0x04, 0x07, 0x00, 0x03, 0x02, 0x00, 0x07, 0x0A, 0x06 },
{ 5560, 112, 0x03, 0x07, 0x00, 0x03, 0x02, 0x00, 0x07, 0x0A, 0x05 },
{ 5580, 116, 0x03, 0x06, 0x00, 0x03, 0x02, 0x00, 0x07, 0x0A, 0x05 },
{ 5590, 118, 0x03, 0x06, 0x00, 0x03, 0x02, 0x00, 0x06, 0x09, 0x05 },
{ 5600, 120, 0x03, 0x06, 0x00, 0x02, 0x02, 0x00, 0x06, 0x09, 0x05 },
{ 5620, 124, 0x04, 0x05, 0x00, 0x02, 0x02, 0x00, 0x06, 0x08, 0x05 },
{ 5640, 128, 0x02, 0x05, 0x00, 0x02, 0x02, 0x00, 0x05, 0x08, 0x04 },
{ 5660, 132, 0x02, 0x05, 0x00, 0x02, 0x01, 0x00, 0x04, 0x08, 0x04 },
{ 5680, 136, 0x01, 0x05, 0x00, 0x02, 0x01, 0x00, 0x04, 0x08, 0x04 },
{ 5700, 140, 0x01, 0x05, 0x00, 0x02, 0x01, 0x00, 0x03, 0x08, 0x04 },
{ 5745, 149, 0x01, 0x05, 0x00, 0x01, 0x01, 0x00, 0x02, 0x08, 0x04 },
{ 5765, 153, 0x01, 0x04, 0x00, 0x01, 0x01, 0x00, 0x01, 0x08, 0x04 },
{ 5785, 157, 0x01, 0x04, 0x00, 0x01, 0x00, 0x00, 0x01, 0x07, 0x03 },
{ 5795, 159, 0x00, 0x04, 0x00, 0x01, 0x00, 0x00, 0x01, 0x06, 0x04 },
{ 5805, 161, 0x00, 0x04, 0x00, 0x01, 0x00, 0x00, 0x01, 0x05, 0x03 },
{ 5825, 165, 0x00, 0x04, 0x00, 0x01, 0x00, 0x00, 0x01, 0x05, 0x02 },
{ 4920, 184, 0x0F, 0x0E, 0x0A, 0x0B, 0x09, 0x00, 0x0F, 0x0F, 0x0E },
{ 4925, 185, 0x0F, 0x0D, 0x09, 0x0B, 0x09, 0x00, 0x0F, 0x0F, 0x0E },
{ 4935, 187, 0x0F, 0x0D, 0x09, 0x0B, 0x09, 0x00, 0x0F, 0x0F, 0x0E },
{ 4940, 188, 0x0F, 0x0D, 0x08, 0x0B, 0x09, 0x00, 0x0F, 0x0F, 0x0E },
{ 4945, 189, 0x0F, 0x0D, 0x08, 0x0A, 0x09, 0x00, 0x0F, 0x0F, 0x0E },
{ 4960, 192, 0x0F, 0x0D, 0x08, 0x0A, 0x09, 0x00, 0x0F, 0x0F, 0x0E },
{ 4980, 196, 0x0E, 0x0D, 0x08, 0x0A, 0x08, 0x00, 0x0F, 0x0F, 0x0E },
{ 5035, 207, 0x0E, 0x0D, 0x08, 0x0A, 0x08, 0x00, 0x0F, 0x0F, 0x0E },
{ 5040, 208, 0x0E, 0x0C, 0x07, 0x09, 0x08, 0x00, 0x0F, 0x0F, 0x0D },
{ 5045, 209, 0x0E, 0x0C, 0x07, 0x09, 0x08, 0x00, 0x0F, 0x0F, 0x0D },
{ 5050, 210, 0x0D, 0x0C, 0x07, 0x09, 0x08, 0x00, 0x0F, 0x0F, 0x0C },
{ 5060, 212, 0x0D, 0x0C, 0x06, 0x09, 0x07, 0x00, 0x0F, 0x0F, 0x0C },
{ 5080, 216, 0x0C, 0x0B, 0x06, 0x08, 0x07, 0x00, 0x0F, 0x0F, 0x0B },
#endif /* BAND5G */
};
static chan_info_2065_lcn40phy_t chan_info_2065_rev2_lcn40phy[] = {
{ 2412,   1, 0x01, 0x00, 0x00, 0x06, 0x00, 0x00, 0x04, 0x01, 0x00 },
{ 2417,   2, 0x01, 0x00, 0x00, 0x06, 0x00, 0x00, 0x04, 0x01, 0x00 },
{ 2422,   3, 0x01, 0x00, 0x00, 0x06, 0x00, 0x00, 0x04, 0x01, 0x00 },
{ 2427,   4, 0x01, 0x00, 0x00, 0x06, 0x00, 0x00, 0x04, 0x01, 0x00 },
{ 2432,   5, 0x01, 0x00, 0x00, 0x05, 0x00, 0x00, 0x04, 0x01, 0x00 },
{ 2437,   6, 0x01, 0x00, 0x00, 0x05, 0x00, 0x00, 0x04, 0x01, 0x00 },
{ 2442,   7, 0x01, 0x00, 0x00, 0x05, 0x00, 0x00, 0x04, 0x00, 0x00 },
{ 2447,   8, 0x01, 0x00, 0x00, 0x05, 0x00, 0x00, 0x04, 0x00, 0x00 },
{ 2452,   9, 0x01, 0x00, 0x00, 0x04, 0x00, 0x00, 0x04, 0x00, 0x00 },
{ 2457,  10, 0x01, 0x00, 0x00, 0x04, 0x00, 0x00, 0x04, 0x00, 0x00 },
{ 2462,  11, 0x01, 0x00, 0x00, 0x04, 0x00, 0x00, 0x04, 0x00, 0x00 },
{ 2467,  12, 0x01, 0x00, 0x00, 0x04, 0x00, 0x00, 0x04, 0x00, 0x00 },
{ 2472,  13, 0x01, 0x00, 0x00, 0x03, 0x00, 0x00, 0x04, 0x00, 0x00 },
{ 2484,  14, 0x01, 0x00, 0x00, 0x03, 0x00, 0x00, 0x04, 0x00, 0x00 },
};

#if defined(WLTEST)
typedef struct _chan_info_2065_lcn40phy_lna_corr {
	uint8   chan;            /* channel number */
	int8   corr_qdBm;        /* Correction in qdBm */
} chan_info_2065_lcn40phy_lna_corr_t;

static chan_info_2065_lcn40phy_lna_corr_t chan_info_2065_lcn40phy_lna_corr[] = {
{   1,  0},
{   2,  0},
{   3,  0},
{   4, -1},
{   5, -1},
{   6, -2},
{   7, -3},
{   8, -2},
{   9, -3},
{  10, -5},
{  11, -5},
{  12, -6},
{  13, -7},
{  14, -7},
#ifdef BAND5G
{  36,  0},
{  40,  1},
{  44,  4},
{  48,  6},
{  52,  9},
{  56, 12},
{  60, 13},
{  64, 14},
{ 100,  0},
{ 104,  1},
{ 108,  3},
{ 112,  4},
{ 116,  7},
{ 120,  8},
{ 124, 10},
{ 128, 13},
{ 132, 15},
{ 136, 17},
{ 140, 20},
{ 149,  0},
{ 153, -3},
{ 157, -7},
{ 161, -10},
{ 165, -13},
#endif /* BAND5G */
};
#endif /* #if defined(WLTEST) */

#define LCN40PHY_NUM_DIG_FILT_COEFFS 17
#define LCN40PHY_NUM_TX_DIG_FILTERS_CCK 17
/* filter id, followed by coefficients */
uint16 LCN40PHY_txdigfiltcoeffs_cck[LCN40PHY_NUM_TX_DIG_FILTERS_CCK]
	[1+LCN40PHY_NUM_DIG_FILT_COEFFS] = {
	{ 0, 1, 0x19f, 0xff52, 0x40, 0x80, 0x40, 0x318, 0xfe78, 0x40, 0x80, 0x40, 0x30a,
	0xfe2e, 0x40, 0x80, 0x40, 8},
	{ 1, 1, 0x192, 0xff37, 0x29f, 0xff02, 0x260, 0xff47, 0x103, 0x3b, 0x103, 0x44, 0x36,
	0x44, 0x5d, 0xa7, 0x5d, 8},
	{ 2, 1, 415, 1874, 64, 128, 64, 792, 1656, 192, 384, 192, 778, 1582, 64, 128, 64, 8},
	{ 3, 1, 302, 1841, 129, 258, 129, 658, 1720, 205, 410, 205, 754, 1760, 170, 340, 170, 8},
	{ 20, 1, 360, -164, 242, -314, 242, 752, -328, 205, -203, 205, 767, -288, 253, 183, 253, 8},
	{ 21, 1, 360, 1884, 149, 1874, 149, 752, 1720, 205, 1884, 205, 767, 1760, 256, 273, 256, 8},
	{ 22, 1, 360, 1884, 98, 1948, 98, 752, 1720, 205, 1924, 205, 767, 1760, 256, 352, 256, 8},
	{ 23, 1, 350, 1884, 116, 1966, 116, 752, 1720, 205, 2008, 205, 767, 1760, 129, 235, 129, 8},
	{ 24, 1, 325, 1884, 32, 40, 32, 756, 1720, 256, 471, 256, 766, 1760, 262, 1878, 262, 8},
	{ 25, 1, 299, 1884, 51, 64, 51, 736, 1720, 256, 471, 256, 765, 1760, 262, 1878, 262, 8},
	{ 26, 1, 277, 1943, 39, 117, 88, 637, 1838, 64, 192, 144, 614, 1864, 128, 384, 288, 8},
	{ 27, 1, 245, 1943, 49, 147, 110, 626, 1838, 162, 485, 363, 613, 1864, 62, 186, 139, 8},
	{ 28, 1, 360, 1884, 149, 1874, 149, 752, 1720, 205, 1884, 205, 767, 1760, 114, 121, 114, 8},
	{ 30, 1, 302, 1841, 61, 122, 61, 658, 1720, 205, 410, 205, 754, 1760, 170, 340, 170, 8},
	{ 31, 1, 319, 1817, 490, 0, 490, 699, 1678, 324, 0, 324, 754, 1760, 114, 0, 114, 8},
	{ 40, 1, 360, 1884, 242, 1734, 242, 752, 1720, 205, 1845, 205, 767, 1760, 511, 370, 511, 8},
	{ 50, 1, 0x1d9, 0xff0c, 0x20, 0x40, 0x20, 0x3a2, 0xfe41, 0x10, 0x20, 0x10, 0x3a1,
	0xfe58, 0x10, 0x20, 0x10, 8}
	};

#define  MAX_CCK_DB_SCALING 7
#define  DB2SCALEFCTR_SHIFT 6
/* The scale factors are based on the below formula
	round ((pow(10, (-db)/20.0)) << DB2SCALEFCTR_SHIFT)
*/
uint8 LCN40PHY_db2scalefctr_cck[MAX_CCK_DB_SCALING] = {57, 51, 45, 40, 36, 32, 29};

#define LCN40PHY_NUM_TX_DIG_FILTERS_CCK_HIGH 9
uint16 LCN40PHY_txdigfiltcoeffs_cck_high /* high dacrate */
	[LCN40PHY_NUM_TX_DIG_FILTERS_CCK_HIGH][1+LCN40PHY_NUM_DIG_FILT_COEFFS] = {
	{ 0, 1, 0x1d9, 0xff0c, 0x20, 0x40, 0x20, 0x3a2, 0xfe41, 0x10, 0x20, 0x10, 0x3a1,
	0xfe58, 0x10, 0x20, 0x10, 8},
	{ 21, 1, 447, 1841, 224, 1649, 224, 903, 1633, 117, 1852, 117, 896, 1656, 334,
	1856, 334, 8},
	{ 22, 1, 447, 1841, 98, 1877, 98, 903, 1633, 225, 1685, 225, 889, 1662, 268, 1955, 268, 8},
	{ 23, 1, 444, 1841, 116, 1858, 116, 903, 1633, 174, 1790, 174, 896, 1656, 129, 15, 129, 8},
	{ 24, 1, 436, 1841, 33, 2019, 33, 904, 1633, 121, 112, 121, 895, 1656, 307, 1548, 307, 8},
	{ 25, 1, 429, 1841, 72, 1986, 72, 898, 1633, 86, 79, 86, 895, 1656, 313, 1538, 313, 8},
	{ 26, 1, 403, 1876, 39, 2036, 39, 795, 1735, 180, 1992, 180, 819, 1720, 128, 2008, 128, 8},
	{ 27, 1, 393, 1876, 49, 2029, 49, 791, 1735, 256, 1968, 256, 814, 1725, 117, 2011, 117, 8},
	{ 30, 1, 443, 1817, 61, 2019, 61, 875, 1633, 154, 1976, 154, 892, 1656, 170, 1969, 170, 8}
	};

#define LCN40PHY_NUM_TX_DIG_FILTERS_OFDM 9
uint16 LCN40PHY_txdigfiltcoeffs_ofdm[LCN40PHY_NUM_TX_DIG_FILTERS_OFDM]
	[1+LCN40PHY_NUM_DIG_FILT_COEFFS] = {
	{ 0, 0, 0xa2, 0, 0x100, 0x100, 0x0, 0x0, 0x0, 0x100, 0x0, 0x0,
	0x278, 0xfea0, 152, 304, 152, 8},
	{ 1, 0, 374, -135, 16, 32, 16, 799, -396, 50, 32, 50,
	0x750, -469, 212, -50, 212, 8},
	{ 2, 0, 375, -234, 37, 76, 37, 799, -396, 32, 20, 32,
	748, -270, 128, -30, 128, 8},
	{3, 0, 375, 0xFF16, 37, 76, 37, 799, 0xFE74, 32, 20, 32, 748,
	0xFEF2, 148, 0xFFDD, 148, 8},
	{4, 0, 307, 1966, 53, 106, 53, 779, 1669, 53, 2038, 53, 765,
	1579, 212, 1846, 212, 8},
	{5, 0, 0x1c5, 0xff1d, 0x20, 0x40, 0x20, 0, 0, 0x100, 0, 0, 0x36b,
	0xfe82, 0x14, 0x29, 0x14, 8},
	{ 6, 0, 375, -234, 37, 76, 37, 799, -396, 32, 20, 32,
	748, -270, 114, -27, 114, 8},
	{10, 0, 0xa2, 0, 0x100, 0x100, 0x0, 0, 0, 511, 0, 0, 0x278,
	0xfea0, 256, 511, 256, 8},
	{12, 0, 394, -234, 29, 58, 29, 800, -394, 24, 48, 24, 836,
	-352, 38, 76, 38, 8}
	};

#define LCN40PHY_NUM_TX_DIG_FILTERS_OFDM40 8
uint16 LCN40PHY_txdigfiltcoeffs_ofdm40 /* high dacrate */
	[LCN40PHY_NUM_TX_DIG_FILTERS_OFDM40][1+LCN40PHY_NUM_DIG_FILT_COEFFS] = {
	{0, 0, 0x97, 0, 0x100, 0x100, 0, 0, 0, 0x100, 0, 0, 0x236, 0xfeb7, 0x80, 0x100, 0x80, 8},
	{2, 0, 375, -234, 37, 76, 37, 799, -396, 32, 20, 32, 748, -270, 72, -17, 72, 8},
	{3, 0, 366, -234, 37, 74, 37, 791, -396, 32, 20, 32, 748, -270, 81, -19, 81, 8},
	{4, 0, 366, -234, 37, 74, 37, 791, -396, 32, 20, 32, 748, -270, 128, -30, 128, 8},
	{5, 0, 162, 0, 205, 205, 0, 0, 0, 181, 0, 0, 568, -272, 171, 341, 171, 8},
	{6, 0, 375, -234, 37, 76, 37, 799, -396, 32, 20, 32, 748, -270, 102, -24, 102, 8},
	{7, 0, 366, -234, 37, 74, 37, 791, -396, 32, 20, 32, 748, -270, 128, -30, 128, 8},
	{8, 0, 162, 0, 205, 205, 0, 0, 0, 181, 0, 0, 568, -272, 144, 287, 144, 8},
	};

#define LCN40PHY_NUM_TX_DIG_FILTERS_OFDM_HIGH 3
uint16 LCN40PHY_txdigfiltcoeffs_ofdm_high /* high dacrate */
	[LCN40PHY_NUM_TX_DIG_FILTERS_OFDM_HIGH][1+LCN40PHY_NUM_DIG_FILT_COEFFS] = {
	{0, 0, 453, 1821, 32, 64, 32, 0, 0, 256, 0, 0, 875, 1666, 25, 52, 25, 8},
	{2, 0, 471, 1803, 18, 36, 18, 935, 1596, 20, 2025, 20, 882, 1669, 338, 1544, 338, 7},
	{4, 0, 408, 1888, 53, 106, 53, 924, 1605, 37, 1985, 37, 945, 1558, 284, 1627, 284, 7}
	};

/* LCN40PHY IQCAL parameters for various Tx gain settings */
/* table format: */
/*	target, gm, pga, pad, ncorr for each of 5 cal types */
typedef uint16 iqcal_gain_params_lcn40phy[9];

static const iqcal_gain_params_lcn40phy tbl_iqcal_gainparams_lcn40phy_2G[] = {
	{0, 0, 0, 0, 0, 0, 0, 0, 0},
	};

#ifdef BAND5G
static const iqcal_gain_params_lcn40phy tbl_iqcal_gainparams_lcn40phy_5G[] = {
	{0, 7, 14, 14, 0, 0, 0, 0, 0},
	};
static const iqcal_gain_params_lcn40phy *tbl_iqcal_gainparams_lcn40phy[2] = {
	tbl_iqcal_gainparams_lcn40phy_2G,
	tbl_iqcal_gainparams_lcn40phy_5G
	};
static const uint16 iqcal_gainparams_numgains_lcn40phy[2] = {
	sizeof(tbl_iqcal_gainparams_lcn40phy_2G) / sizeof(*tbl_iqcal_gainparams_lcn40phy_2G),
	sizeof(tbl_iqcal_gainparams_lcn40phy_5G) / sizeof(*tbl_iqcal_gainparams_lcn40phy_5G)
	};
#else
static const iqcal_gain_params_lcn40phy *tbl_iqcal_gainparams_lcn40phy[1] = {
	tbl_iqcal_gainparams_lcn40phy_2G,
	};

static const uint16 iqcal_gainparams_numgains_lcn40phy[1] = {
	sizeof(tbl_iqcal_gainparams_lcn40phy_2G) / sizeof(*tbl_iqcal_gainparams_lcn40phy_2G),
	};
#endif /* BAND5G */

static const phy_sfo_cfg_t lcn40phy_sfo_cfg[] = {
	{965, 1087},
	{967, 1085},
	{969, 1082},
	{971, 1080},
	{973, 1078},
	{975, 1076},
	{977, 1073},
	{979, 1071},
	{981, 1069},
	{983, 1067},
	{985, 1065},
	{987, 1063},
	{989, 1060},
	{994, 1055}
};

/* LO Comp Gain ladder. Format: {m genv} */
static const
uint16 lcn40phy_iqcal_loft_gainladder[]  = {
	((2 << 8) | 0),
	((3 << 8) | 0),
	((4 << 8) | 0),
	((6 << 8) | 0),
	((8 << 8) | 0),
	((11 << 8) | 0),
	((16 << 8) | 0),
	((16 << 8) | 1),
	((16 << 8) | 2),
	((16 << 8) | 3),
	((16 << 8) | 4),
	((16 << 8) | 5),
	((16 << 8) | 6),
	((16 << 8) | 7),
	((23 << 8) | 7),
	((32 << 8) | 7),
	((45 << 8) | 7),
	((64 << 8) | 7),
	((91 << 8) | 7),
	((128 << 8) | 7)
};

/* Image Rejection Gain ladder. Format: {m genv} */
static const
uint16 lcn40phy_iqcal_ir_gainladder[] = {
	((1 << 8) | 0),
	((2 << 8) | 0),
	((4 << 8) | 0),
	((6 << 8) | 0),
	((8 << 8) | 0),
	((11 << 8) | 0),
	((16 << 8) | 0),
	((23 << 8) | 0),
	((32 << 8) | 0),
	((45 << 8) | 0),
	((64 << 8) | 0),
	((64 << 8) | 1),
	((64 << 8) | 2),
	((64 << 8) | 3),
	((64 << 8) | 4),
	((64 << 8) | 5),
	((64 << 8) | 6),
	((64 << 8) | 7),
	((91 << 8) | 7),
	((128 << 8) | 7)
};
static const
uint16 rxiq_cal_rf_reg[] = {
	RADIO_2065_LPF_CFG1,
	RADIO_2065_LPF_CFG2,
	RADIO_2065_LPF_RESP_BQ1,
	RADIO_2065_LPF_RESP_BQ2,
	RADIO_2065_AUXPGA_CFG1,
	RADIO_2065_OVR1,
	RADIO_2065_OVR2,
	RADIO_2065_OVR3,
	RADIO_2065_OVR4,
	RADIO_2065_OVR5,
	RADIO_2065_OVR6,
	RADIO_2065_OVR7,
	RADIO_2065_OVR8,
	RADIO_2065_OVR11,
	RADIO_2065_OVR12,
	RADIO_2065_LPF_BIAS0,
	RADIO_2065_TXRX2G_CAL_BAK,
	RADIO_2065_TXRX2G_CAL,
	RADIO_2065_OVR14_ACCOREREG,
	RADIO_2065_TRSW2G_CFG2,
	RADIO_2065_LNA2G_CFG1,
	RADIO_2065_PA2G_CFG1,
#ifdef BAND5G
	RADIO_2065_TXRX5G_CAL_BAK,
	RADIO_2065_TXRX5G_CAL,
	RADIO_2065_PA5G_CFG1,
	RADIO_2065_LNA5G_CFG1,
#endif // endif
	};

static const
uint16 rxiq_cal_phy_reg[] = {
	LCN40PHY_RFOverride0,
	LCN40PHY_RFOverrideVal0,
	LCN40PHY_rfoverride2,
	LCN40PHY_rfoverride2val,
	LCN40PHY_rfoverride4,
	LCN40PHY_rfoverride4val,
	LCN40PHY_rfoverride7,
	LCN40PHY_rfoverride7val,
	LCN40PHY_rfoverride8,
	LCN40PHY_rfoverride8val,
	LCN40PHY_Core1TxControl,
	LCN40PHY_AfeCtrlOvr1,
	LCN40PHY_AfeCtrlOvr1Val,
	LCN40PHY_sslpnCalibClkEnCtrl,
	LCN40PHY_sslpnRxFeClkEnCtrl,
	LCN40PHY_lpfgainlutreg,
	};

#define PHY_REG		0
#define RADIO_REG	1

/* registers that need to be saved/restored for papd loopback */

static const
uint16 papd_loopback_rf_reg2g[] = {
	RADIO_2065_OVR1,
	RADIO_2065_OVR3,
	RADIO_2065_OVR4,
	RADIO_2065_OVR5,
	RADIO_2065_OVR6,
	RADIO_2065_OVR7,
	RADIO_2065_OVR11,
	RADIO_2065_OVR12,
	RADIO_2065_AUXPGA_CFG1,
	RADIO_2065_LPF_GAIN,
	RADIO_2065_LNA2G_CFG1,
	RADIO_2065_LPF_CFG1,
	RADIO_2065_TIA_CFG1,
	RADIO_2065_TOP_SPARE1,
	RADIO_2065_RX_REG_BACKUP_1,
	RADIO_2065_PA2G_CFG1,
	RADIO_2065_TRSW2G_CFG2,
	RADIO_2065_OVR14_ACCOREREG,
	RADIO_2065_TXRX5G_CAL_BAK
	};

#ifdef BAND5G
static const
uint16 papd_loopback_rf_reg5g[] = {
	RADIO_2065_OVR1,
	RADIO_2065_OVR2,
	RADIO_2065_OVR3,
	RADIO_2065_OVR4,
	RADIO_2065_OVR5,
	RADIO_2065_OVR6,
	RADIO_2065_OVR7,
	RADIO_2065_OVR11,
	RADIO_2065_OVR12,
	RADIO_2065_OVR13,
	RADIO_2065_AUXPGA_CFG1,
	RADIO_2065_LPF_GAIN,
	RADIO_2065_LNA5G_CFG1,
	RADIO_2065_LPF_CFG1,
	RADIO_2065_TIA_CFG1,
	RADIO_2065_TOP_SPARE1,
	RADIO_2065_RX_REG_BACKUP_1,
	RADIO_2065_PA5G_CFG1,
	RADIO_2065_TXRX5G_CAL_BAK,
	RADIO_2065_OVR14_ACCOREREG
	};
#endif /* #ifdef BAND5G */
/* PHY registers that need to be saved/restored for papd cal */
static const
uint16 papd_cal_phy_reg[] = {
	LCN40PHY_papr_iir_group_dly,
	LCN40PHY_lpfbwlutreg1,
	LCN40PHY_crsgainCtrl,
	LCN40PHY_radioCtrl,
	LCN40PHY_RFinputOverrideVal,
	LCN40PHY_RFinputOverride,
	LCN40PHY_AphyControlAddr,
	LCN40PHY_Core1TxControl,
	LCN40PHY_sslpnCtrl3,
	LCN40PHY_sslpnCalibClkEnCtrl,
	LCN40PHY_AfeCtrlOvr1,
	LCN40PHY_AfeCtrlOvr1Val,
	LCN40PHY_RFOverride0,
	LCN40PHY_RFOverrideVal0,
	LCN40PHY_rfoverride4,
	LCN40PHY_rfoverride4val,
	LCN40PHY_rfoverride2,
	LCN40PHY_rfoverride2val,
	LCN40PHY_rfoverride7,
	LCN40PHY_rfoverride7val,
	LCN40PHY_rfoverride8,
	LCN40PHY_rfoverride8val
};

static const
uint16 papd_calidx_estimate_rf_reg[] = {
	RADIO_2065_OVR1,
	RADIO_2065_OVR2,
	RADIO_2065_OVR7,
	RADIO_2065_OVR12,
	RADIO_2065_OVR13,
	RADIO_2065_OVR16,
	RADIO_2065_TX2G_TSSI,
	RADIO_2065_TX5G_TSSI,
	RADIO_2065_IQCAL_CFG1,
	RADIO_2065_TESTBUF_CFG1,
	RADIO_2065_AUXPGA_CFG1,
	RADIO_2065_AUXPGA_VMID,
	RADIO_2065_LPF_CFG1,
	RADIO_2065_VBAT_CFG,
	RADIO_2065_TEMPSENSE_CFG,
	RADIO_2065_LPF_GAIN,
	RADIO_2065_IQCAL_IDAC,
};

static const
uint16 papd_calidx_estimate_phy_reg[] = {
	LCN40PHY_agcControl4,
	LCN40PHY_RFOverride0,
	LCN40PHY_RFOverrideVal0,
	LCN40PHY_ClkEnCtrl,
	LCN40PHY_AfeCtrlOvr1,
	LCN40PHY_AfeCtrlOvr1Val,
	LCN40PHY_sslpnCalibClkEnCtrl,
	LCN40PHY_TxPwrCtrlRfCtrlOverride0,
	LCN40PHY_TxPwrCtrlRfCtrl0,
	LCN40PHY_TxPwrCtrlCmdNew,
	LCN40PHY_TxPwrCtrlNum_Vbat,
	LCN40PHY_TxPwrCtrlNum_temp,
	LCN40PHY_TxPwrCtrlCmd,
	LCN40PHY_papd_control2,
	LCN40PHY_TxPwrCtrlIdleTssi,
	LCN40PHY_radioCtrl,
	LCN40PHY_RFinputOverride,
	LCN40PHY_RFinputOverrideVal,
	LCN40PHY_TxPwrCtrlRangeCmd,
	LCN40PHY_TxPwrCtrlNnum,
};

struct lr_t {
	uint16 radio;	/* 1 if radio reg, 0 if phy reg */
	uint16 offset;	/* byte offset of register */
};

static const
struct lr_t papd_loopback_reg[] = {
	{PHY_REG,	LCN40PHY_RFOverride0},
	{PHY_REG,	LCN40PHY_RFOverrideVal0},
	{RADIO_REG,	RADIO_2065_AUXPGA_CFG1},
	{RADIO_REG,	RADIO_2065_LPF_GAIN},
	{PHY_REG,	LCN40PHY_AfeCtrlOvr1},
	{PHY_REG,	LCN40PHY_AfeCtrlOvr1Val},
	{PHY_REG,	LCN40PHY_rfoverride2},
	{PHY_REG,	LCN40PHY_rfoverride2val},
	{PHY_REG,	LCN40PHY_rfoverride4},
	{PHY_REG,	LCN40PHY_rfoverride4val},
	{PHY_REG,	LCN40PHY_rfoverride7val},
	{RADIO_REG,	RADIO_2065_PA2G_CFG1},
	{RADIO_REG,	RADIO_2065_RXRF2G_CFG1},
	{RADIO_REG,	RADIO_2065_OVR1},
	{RADIO_REG,	RADIO_2065_OVR3},
	{RADIO_REG,	RADIO_2065_OVR4},
	{RADIO_REG,	RADIO_2065_OVR5},
	{RADIO_REG,	RADIO_2065_OVR6},
	{RADIO_REG,	RADIO_2065_OVR7},
	{RADIO_REG,	RADIO_2065_OVR11},
	{RADIO_REG,	RADIO_2065_OVR12},
	{RADIO_REG,	RADIO_2065_LOGEN2G_TXDIV},
	{RADIO_REG,	RADIO_2065_LOGEN2G_RXDIV},
	{RADIO_REG,	RADIO_2065_LNA2G_CFG2},
	{RADIO_REG,	RADIO_2065_LNA2G_CFG1},
	{RADIO_REG,	RADIO_2065_RXMIX2G_CFG1},
	{RADIO_REG,	RADIO_2065_RX_REG_BACKUP_1},
	{RADIO_REG,	RADIO_2065_TIA_CFG1},
	{RADIO_REG,	RADIO_2065_TOP_SPARE1},
	{RADIO_REG,	RADIO_2065_TXRX2G_CAL},
	{RADIO_REG,	RADIO_2065_TXRX5G_CAL},
	{RADIO_REG,	RADIO_2065_OVR14_ACCOREREG},
	{RADIO_REG,	RADIO_2065_OVR12},
	{RADIO_REG,	RADIO_2065_TRSW2G_CFG2},
	{RADIO_REG,	RADIO_2065_PA5G_CFG1},
	{RADIO_REG,	RADIO_2065_LNA5G_CFG1},
	{RADIO_REG,	RADIO_2065_OVR13_ACCOREREG},
	{RADIO_REG,	RADIO_2065_TXRX5G_CAL_BAK},
	{RADIO_REG,	RADIO_2065_TX_REG_BACKUP_2},

};

/* 20MHz Notch filter 1 settings for spur cancellation */
static
int8 spurblk_phy_reg_cfg [14][LCN40PHY_NOTCHFILTER_COEFFS] = {
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{15, 1, -121, 0, 79, 1, -95, 2, 123, 4},
	{15, 1, 79, 1, 121, 0, 123, 4, 95, 2},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{15, 1, -79, 1, -121, 0, -123, 4, -95, 2},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{15, 1, -108, 1, 115, 0, -85, 3, 90, 2},
	{15, 1, 115, 0, 108, 1, 90, 2, 85, 3},
	{15, 1, 108, 1, -115, 0, 85, 3, -90, 2},
	{15, 1, -115, 0, -108, 1, -90, 2, -85, 3},
};

static const
uint32 mu_deltaLUT[14] = {
	7533053,
	7526147,
	7519268,
	7512418,
	7505597,
	7498803,
	7492037,
	7485299,
	7478588,
	7471904,
	7465248,
	7458618,
	7452016,
	7436278
};

typedef struct _pll_constants_lcn40phy {
	uint16 r1_min;
	uint16 r2_min;
	uint16 r3_min;
	uint16 c_max;
	uint16 fvco_lo;
	uint16 fvco_hi;
	uint16 c1_ref;
	uint16 c3_ref;
	uint16 c4_ref;
	uint16 r1_ref;
	uint16 r2_ref;
	uint16 r3_ref;
	uint8 c1_min;
	uint8 c1_step;
	uint8 c2_min;
	uint8 c2_step;
	uint8 c3_min;
	uint8 c3_step;
	uint8 c4_min;
	uint8 c4_step;
	uint8 kpd_step;
	uint8 kpd_scale;
	uint8 ioff_step;
	uint8 ioff_scale;
	uint8 kvco_ref;
} pll_constants_lcn40phy_t;

static const
pll_constants_lcn40phy_t pll_constants_2065 = {
	PLL_2065_R1_MIN,
	PLL_2065_R2_MIN,
	PLL_2065_R3_MIN,
	PLL_2065_C_MAX,
	PLL_2065_FVCO_LO,
	PLL_2065_FVCO_HI,
	PLL_2065_C1_REF,
	PLL_2065_C3_REF,
	PLL_2065_C4_REF,
	PLL_2065_R1_REF,
	PLL_2065_R2_REF,
	PLL_2065_R3_REF,
	PLL_2065_C1_MIN,
	PLL_2065_C1_STEP,
	PLL_2065_C2_MIN,
	PLL_2065_C2_STEP,
	PLL_2065_C3_MIN,
	PLL_2065_C3_STEP,
	PLL_2065_C4_MIN,
	PLL_2065_C4_STEP,
	PLL_2065_KPD_STEP,
	PLL_2065_KPD_SCALE,
	PLL_2065_IOFF_STEP,
	PLL_2065_IOFF_SCALE,
	PLL_2065_KVCO_REF
};

static const
pll_constants_lcn40phy_t pll_constants_2067 = {
	PLL_2067_R1_MIN,
	PLL_2067_R2_MIN,
	PLL_2067_R3_MIN,
	PLL_2067_C_MAX,
	PLL_2067_FVCO_LO,
	PLL_2067_FVCO_HI,
	PLL_2067_C1_REF,
	PLL_2067_C3_REF,
	PLL_2067_C4_REF,
	PLL_2067_R1_REF,
	PLL_2067_R2_REF,
	PLL_2067_R3_REF,
	PLL_2067_C1_MIN,
	PLL_2067_C1_STEP,
	PLL_2067_C2_MIN,
	PLL_2067_C2_STEP,
	PLL_2067_C3_MIN,
	PLL_2067_C3_STEP,
	PLL_2067_C4_MIN,
	PLL_2067_C4_STEP,
	PLL_2067_KPD_STEP,
	PLL_2067_KPD_SCALE,
	PLL_2067_IOFF_STEP,
	PLL_2067_IOFF_SCALE,
	PLL_2067_KVCO_REF
};

static const
int8 lcn40phy_gain_index_offset_for_rx_gain_2g[] = {
	0,      /* 0 */
	0,      /* 1 */
	0,      /* 2 */
	0,      /* 3 */
	0,      /* 4 */
	0,      /* 5 */
	0,      /* 6 */
	0,      /* 7 */
	0,      /* 8 */
	0,      /* 9 */
	0,      /* 10 */
	0,      /* 11 */
	0,      /* 12 */
	0,      /* 13 */
	0,      /* 14 */
	0,      /* 15 */
	0,      /* 16 */
	0,      /* 17 */
	0,      /* 18 */
	0,      /* 19 */
	0,      /* 20 */
	0,      /* 21 */
	0,      /* 22 */
	0,      /* 23 */
	0,      /* 24 */
	0,      /* 25 */
	0,      /* 26 */
	0,      /* 27 */
	0,      /* 28 */
	0,      /* 29 */
	0,      /* 30 */
	1,      /* 31 */
	1,      /* 32 */
	1,      /* 33 */
	2,      /* 34 */
	3,      /* 35 */
	4,      /* 36 */
	6,      /* 37 */
};

#ifdef BAND5G
static const
int8 lcn40phy_gain_index_offset_for_rx_gain_5gl[] = {
	0,   	/* 0 */
	0,   	/* 1 */
	0,   	/* 2 */
	0,   	/* 3 */
	0,   	/* 4 */
	0,   	/* 5 */
	0,   	/* 6 */
	0,   	/* 7 */
	0,   	/* 8 */
	0,   	/* 9 */
	0,   	/* 10 */
	0,   	/* 11 */
	0,   	/* 12 */
	0,   	/* 13 */
	0,   	/* 14 */
	0,   	/* 15 */
	0,   	/* 16 */
	0,   	/* 17 */
	0,   	/* 18 */
	0,   	/* 19 */
	-5,   	/* 20 */
	-4, 	/* 21 */
	-4, 	/* 22 */
	-4, 	/* 23 */
	-5, 	/* 24 */
	-5, 	/* 25 */
	-5, 	/* 26 */
	-5, 	/* 27 */
	-5, 	/* 28 */
	-5, 	/* 29 */
	0, 	/* 30 */
	0,   	/* 31 */
	0,   	/* 32 */
	0,   	/* 33 */
	0,   	/* 34 */
	0,   	/* 35 */
	0,   	/* 36 */
	0,   	/* 37 */
};
static const
int8 lcn40phy_gain_index_offset_for_rx_gain_5gml[] = {
	0,    /* 0 */
	0,    /* 1 */
	0,    /* 2 */
	0,    /* 3 */
	0,    /* 4 */
	0,    /* 5 */
	0,    /* 6 */
	0,    /* 7 */
	0,    /* 8 */
	0,    /* 9 */
	0,    /* 10 */
	0,    /* 11 */
	0,    /* 12 */
	0,    /* 13 */
	0,    /* 14 */
	0,    /* 15 */
	0,    /* 16 */
	0,    /* 17 */
	0,    /* 18 */
	0,    /* 19 */
	-5,    /* 20 */
	-5,  /* 21 */
	-5,  /* 22 */
	-5,  /* 23 */
	-5,  /* 24 */
	-5,  /* 25 */
	-5,  /* 26 */
	-5,  /* 27 */
	-5,  /* 28 */
	-5,  /* 29 */
	0,  /* 30 */
	0,    /* 31 */
	0,    /* 32 */
	0,    /* 33 */
	0,    /* 34 */
	0,    /* 35 */
	0,    /* 36 */
	0,    /* 37 */
};
static const
int8 lcn40phy_gain_index_offset_for_rx_gain_5gmu[] = {
	0,     /* 0 */
	0,     /* 1 */
	0,     /* 2 */
	0,     /* 3 */
	0,     /* 4 */
	0,     /* 5 */
	0,     /* 6 */
	0,     /* 7 */
	0,     /* 8 */
	0,     /* 9 */
	0,     /* 10 */
	0,     /* 11 */
	0,     /* 12 */
	0,     /* 13 */
	0,     /* 14 */
	0,     /* 15 */
	0,     /* 16 */
	0,     /* 17 */
	0,     /* 18 */
	0,     /* 19 */
	-5,     /* 20 */
	-5,   /* 21 */
	-5,   /* 22 */
	-5,   /* 23 */
	-5,   /* 24 */
	-5,   /* 25 */
	-5,   /* 26 */
	-5,   /* 27 */
	-5,   /* 28 */
	-6,   /* 29 */
	0,   /* 30 */
	0,     /* 31 */
	0,     /* 32 */
	0,     /* 33 */
	0,     /* 34 */
	0,     /* 35 */
	0,     /* 36 */
	0,     /* 37 */
};

static const
int8 lcn40phy_gain_index_offset_for_rx_gain_5gh[] = {
	0,	/* 0 */
	0,	/* 1 */
	0,	/* 2 */
	0,	/* 3 */
	0,	/* 4 */
	0,	/* 5 */
	0,	/* 6 */
	0,	/* 7 */
	0,	/* 8 */
	0,	/* 9 */
	0,	/* 10 */
	0,	/* 11 */
	0,	/* 12 */
	0,	/* 13 */
	0,	/* 14 */
	0,	/* 15 */
	0,	/* 16 */
	0,	/* 17 */
	0,	/* 18 */
	0,	/* 19 */
	-4,	/* 20 */
	-4,	/* 21 */
	-4,	/* 22 */
	-4,	/* 23 */
	-4,	/* 24 */
	-4,	/* 25 */
	-4,	/* 26 */
	-4,	/* 27 */
	-4,	/* 28 */
	-5,	/* 29 */
	0,	/* 30 */
	0,	/* 31 */
	0,	/* 32 */
	0,	/* 33 */
	0,	/* 34 */
	0,	/* 35 */
	0,	/* 36 */
	0,	/* 37 */
};
#endif /* BAND5G */

static const
uint16 clip_gain_2g_acimode_extlna_rev5[] =
	{48, 43, 39, 39, 36, 33, 26, 23, 20, 14, 9, 3, 54};
static const
uint16 clip_thresh_2g_acimode_extlna_rev5[] =
	{10, 10, 10, 15, 15, 10, 20, 20, 20, 20, 20, 20};
static const
uint16 clip_gain_2g_acimode_extlna[] =
	{48, 42, 39, 39, 36, 33, 26, 23, 20, 14, 9, 3, 54};
static const
uint16 clip_thresh_2g_acimode_extlna[] =
	{10, 10, 10, 0, 20, 20, 20, 20, 20, 15, 15, 15};
static
uint32 dot11lcn40_fltr_ctrl_2G_tbl_rev4[] = {
	0x000003f8,
	0x0001fed8,
	0x000065f8,
	0x0001fed8,
	0x0001fff8,
	0x0001fff8,
	0x0001fff8,
	0x0001fff8,
	0x0001fff8,
	0x0001fff8,
};
#ifdef BAND5G
static
uint32 dot11lcn40_fltr_ctrl_5G_tbl_rev4[] = {
	0x000003f9,
	0x0001fff9,
	0x00001ff8,
	0x00001f68,
	0x0001fed8,
	0x00000968,
	0x000008d8,
	0x00000848,
	0x0001fe00,
	0x0001fe00,
};
static
uint32 dot11lcn40_fltr_ctrl_5G_tbl_rev4_fstr[] = {
	0x000003f8,
	0x0001fed8,
	0x000051f9,
	0x000051f8,
	0x000051d8,
	0x0001fed8,
	0x0001fff8,
	0x0001fff8,
	0x0001fff8,
	0x0001fff8,
};
#endif /* #ifdef BAND5G */

/* #define lcn40phy routines */
#define wlc_lcn40phy_common_read_table(pi, tbl_id, tbl_ptr, tbl_len, tbl_width, tbl_offset) \
	phy_utils_read_common_phytable(pi, tbl_id, tbl_ptr, tbl_len, tbl_width, tbl_offset, \
	wlc_lcn40phy_read_table)

#define wlc_lcn40phy_common_write_table(pi, tbl_id, tbl_ptr, tbl_len, tbl_width, tbl_offset) \
	phy_utils_write_common_phytable(pi, tbl_id, tbl_ptr, tbl_len, tbl_width, tbl_offset, \
	wlc_lcn40phy_write_table)

#define wlc_lcn40phy_set_start_tx_pwr_idx(pi, idx) \
	phy_utils_mod_phyreg(pi, LCN40PHY_TxPwrCtrlCmd, \
		LCN40PHY_TxPwrCtrlCmd_pwrIndex_init_MASK, \
		(uint16)(idx*2) << LCN40PHY_TxPwrCtrlCmd_pwrIndex_init_SHIFT)

#define wlc_lcn40phy_set_start_CCK_tx_pwr_idx(pi, idx) \
	if (LCN40REV_GE(pi->pubpi.phy_rev, 4)) \
		phy_utils_mod_phyreg(pi, LCN40PHY_TxPwrCtrlCmdCCK, \
			LCN40PHY_TxPwrCtrlCmdCCK_pwrIndex_init_cck_MASK, \
			(uint16)(idx*2) << LCN40PHY_TxPwrCtrlCmdCCK_pwrIndex_init_cck_SHIFT)

#define wlc_lcn40phy_set_tx_pwr_npt(pi, npt) \
	phy_utils_mod_phyreg(pi, LCN40PHY_TxPwrCtrlNnum, \
		LCN40PHY_TxPwrCtrlNnum_Npt_intg_log2_MASK, \
		(uint16)(npt) << LCN40PHY_TxPwrCtrlNnum_Npt_intg_log2_SHIFT)

#define wlc_lcn40phy_get_tx_pwr_ctrl(pi) \
	(phy_utils_read_phyreg((pi), LCN40PHY_TxPwrCtrlCmd) & \
			(LCN40PHY_TxPwrCtrlCmd_txPwrCtrl_en_MASK | \
			LCN40PHY_TxPwrCtrlCmd_hwtxPwrCtrl_en_MASK | \
			LCN40PHY_TxPwrCtrlCmd_use_txPwrCtrlCoefs_MASK))

#define wlc_lcn40phy_get_tx_pwr_npt(pi) \
	((phy_utils_read_phyreg(pi, LCN40PHY_TxPwrCtrlNnum) & \
		LCN40PHY_TxPwrCtrlNnum_Npt_intg_log2_MASK) >> \
		LCN40PHY_TxPwrCtrlNnum_Npt_intg_log2_SHIFT)

/* the bitsize of the register is 9 bits for lcn40phy */
#define wlc_lcn40phy_get_current_tx_pwr_idx_if_pwrctrl_on(pi) \
	(phy_utils_read_phyreg(pi, LCN40PHY_TxPwrCtrlStatusExt) & 0x1ff)

#define wlc_lcn40phy_get_target_tx_pwr(pi) \
	((phy_utils_read_phyreg(pi, LCN40PHY_TxPwrCtrlTargetPwr) & \
		LCN40PHY_TxPwrCtrlTargetPwr_targetPwr0_MASK) >> \
		LCN40PHY_TxPwrCtrlTargetPwr_targetPwr0_SHIFT)

#define wlc_lcn40phy_set_target_tx_pwr(pi, target) \
	phy_utils_mod_phyreg(pi, LCN40PHY_TxPwrCtrlTargetPwr, \
		LCN40PHY_TxPwrCtrlTargetPwr_targetPwr0_MASK, \
		(uint16)MAX(pi->u.pi_lcn40phy->lcnphycommon.tssi_minpwr_limit, \
		(MIN(pi->u.pi_lcn40phy->lcnphycommon.tssi_maxpwr_limit, \
		(uint16)(target)))) << LCN40PHY_TxPwrCtrlTargetPwr_targetPwr0_SHIFT)

#define LCN40PHY_IQLOCC_READ(val) ((uint8)(-(int8)(((val) & 0xf0) >> 4) + (int8)((val) & 0x0f)))
#define FIXED_TXPWR 78
#define LCN40PHY_TEMPSENSE(val) ((int16)((val > 255)?(val - 512):val))

#define BTC_POWER_CLAMP 20  /* 10dBm in 1/2dBm */

typedef enum {
	LCN40PHY_TSSI_PRE_PA,
	LCN40PHY_TSSI_POST_PA,
	LCN40PHY_TSSI_EXT,
	LCN40PHY_TSSI_EXT_POST_PAD
} lcn40phy_tssi_mode_t;

/* can only be accessed by wlc_phy_cmn.c by function pointer */
static void wlc_lcn40phy_papd_set_bbmult0(phy_info_t *pi, int flag);
static void wlc_phy_cal_init_lcn40phy(phy_info_t *pi);
static void wlc_phy_chanspec_set_lcn40phy(phy_info_t *pi, chanspec_t chanspec);

static void wlc_lcn40phy_set_tx_pwr_ctrl(phy_info_t *pi, uint16 mode);
static void wlc_lcn40phy_set_tx_pwr_by_index(phy_info_t *pi, int indx);

static void wlc_lcn40phy_set_tx_iqcc(phy_info_t *pi, uint16 a, uint16 b);
static void wlc_lcn40phy_get_tx_iqcc(phy_info_t *pi, uint16 *a, uint16 *b);
static uint16 wlc_lcn40phy_get_tx_locc(phy_info_t *pi);
static void wlc_lcn40phy_set_tx_locc(phy_info_t *pi, uint16 didq);
static void wlc_lcn40phy_get_radio_loft(phy_info_t *pi, uint8 *ei0,
	uint8 *eq0, uint8 *fi0, uint8 *fq0);
static void wlc_lcn40phy_set_radio_loft(phy_info_t *pi, uint8, uint8, uint8, uint8);

#if defined(WLTEST)
static void wlc_phy_carrier_suppress_lcn40phy(phy_info_t *pi);
static void wlc_lcn40phy_reset_radio_loft(phy_info_t *pi);
#endif // endif
#if defined(BCMDBG) || defined(WLTEST)
static int wlc_phy_long_train_lcn40phy(phy_info_t *pi, int channel);
#endif // endif
/* LCN40PHY static function declaration */
static void wlc_lcn40phy_noise_attach(phy_info_t *pi);
static void wlc_lcn40phy_radio_init(phy_info_t *pi);
static void wlc_lcn40phy_radio_reset(phy_info_t *pi);
static void wlc_lcn40phy_rcal(phy_info_t *pi);
static void wlc_lcn40phy_rc_cal(phy_info_t *pi);
static void wlc_lcn40phy_restore_rc_cal(phy_info_t *pi);
static void wlc_lcn40phy_minipmu_cal(phy_info_t *pi);
static void wlc_lcn40phy_baseband_init(phy_info_t *pi);
static void	wlc_lcn40phy_tx_pwr_ctrl_init(phy_info_t *pi);
static void	wlc_lcn40phy_agc_temp_init(phy_info_t *pi);
static void	wlc_lcn40phy_temp_adj(phy_info_t *pi);
static void wlc_lcn40phy_noise_init(phy_info_t *pi);
static void wlc_lcn40phy_rev0_reg_init(phy_info_t *pi);
static void wlc_lcn40phy_rev1_reg_init(phy_info_t *pi);
static void wlc_lcn40phy_bu_tweaks(phy_info_t *pi);
static void wlc_lcn40phy_tbl_init(phy_info_t *pi);
static void wlc_lcn40phy_clear_papd_comptable(phy_info_t *pi);
static void wlc_lcn40phy_force_pwr_index(phy_info_t *pi, int indx);
static void wlc_lcn40phy_txpower_recalc_target(phy_info_t *pi);
static void wlc_lcn40phy_set_tx_gain_override(phy_info_t *pi, bool bEnable);
static void wlc_lcn40phy_set_tx_gain(phy_info_t *pi,  phy_txgains_t *target_gains);
/* static uint8 wlc_lcn40phy_get_bbmult(phy_info_t *pi); */
static void wlc_lcn40phy_set_pa_gain(phy_info_t *pi, uint16 gain);
static void wlc_lcn40phy_set_dac_gain(phy_info_t *pi, uint16 dac_gain);
static uint16 wlc_lcn40phy_get_pa_gain(phy_info_t *pi);
void wlc_lcn40phy_set_bbmult(phy_info_t *pi, uint8 m0);
static void wlc_lcn40phy_txpower_reset_npt(phy_info_t *pi);
static void wlc_lcn40phy_set_chanspec_tweaks(phy_info_t *pi, chanspec_t chanspec);
static void wlc_lcn40phy_restore_calibration_results(phy_info_t *pi);
static void wlc_lcn40phy_agc_reset(phy_info_t *pi);
static void wlc_lcn40phy_radio_2065_channel_tune(phy_info_t *pi, uint8 channel);
static void wlc_lcn40phy_radio_2065_channel_tune_new(phy_info_t *pi, uint8 channel);
static void wlc_lcn40phy_decode_aa2g(phy_info_t *pi, uint8 val);
static bool wlc_lcn40phy_cal_reqd(phy_info_t *pi);
static void wlc_lcn40phy_periodic_cal(phy_info_t *pi);
static void wlc_phy_watchdog_lcn40phy(phy_info_t *pi);
static void wlc_lcn40phy_btc_adjust(phy_info_t *pi, bool btactive);
static void wlc_lcn40phy_txpwrtbl_iqlo_cal(phy_info_t *pi);
static bool wlc_lcn40phy_rx_iq_cal(phy_info_t *pi, const phy_rx_iqcomp_t *iqcomp, int iqcomp_sz,
	bool tx_switch, bool rx_switch, int module, int tx_gain_idx);
static void wlc_lcn40phy_papd_phase_jump_corr(phy_info_t *pi);
static void wlc_lcn40phy_papd_cal(phy_info_t *pi, phy_papd_cal_type_t cal_type,
	phy_txcalgains_t *txgains, bool frcRxGnCtrl, bool txGnCtrl, bool samplecapture,
	bool papd_dbg_mode, uint8 num_symbols, uint8 init_papd_lut, uint16 bbmult_step);
static void wlc_lcn40phy_papd_loopback(phy_info_t *pi, lcn40phy_papd_lin_path_t papd_lin_path);
static void wlc_lcn40phy_idle_tssi_est(phy_info_t *pi);
static void wlc_lcn40phy_perpkt_idle_tssi_est(phy_info_t *pi);
static void wlc_lcn40phy_set_estPwrLUT(phy_info_t *pi, int32 lut_num);
static void wlc_lcn40phy_restore_txiqlo_calibration_results(phy_info_t *pi, uint16 startidx,
	uint16 stopidx, uint8 index);
static void wlc_lcn40phy_restore_papd_calibration_results(phy_info_t *pi);
static void wlc_lcn40phy_set_rx_iq_comp(phy_info_t *pi, uint16 a, uint16 b);
static void wlc_lcn40phy_get_tx_gain(phy_info_t *pi,  phy_txgains_t *gains);
static uint8 wlc_lcn40phy_get_bbmult(phy_info_t *pi);
static void wlc_lcn40phy_tx_iqlo_cal(phy_info_t *pi, phy_txgains_t *target_gains,
	phy_cal_mode_t cal_mode, bool keep_tone, bool epa_or_pad_lpbk);
static void wlc_2065_vco_cal(phy_info_t *pi, bool legacy);
static void wlc_lcn40phy_tx_farrow_init(phy_info_t *pi, uint8 channel);
static void wlc_lcn40phy_rx_farrow_init(phy_info_t *pi, uint8 channel);
static chan_info_2065_lcn40phy_t *wlc_lcn40phy_find_channel(phy_info_t *pi, uint8 channel);
static void wlc_lcn40phy_tx_iqlo_loopback(phy_info_t *pi, uint16 *values_to_save,
	bool epa_or_pad_lpbk);
static void wlc_lcn40phy_tx_iqlo_loopback_cleanup(phy_info_t *pi, uint16 *values_to_save);
static bool wlc_lcn40phy_iqcal_wait(phy_info_t *pi);
static void wlc_lcn40phy_set_tssi_pwr_limit(phy_info_t *pi, uint8 mode);
static void wlc_lcn40phy_tssi_setup(phy_info_t *pi);
static void wlc_lcn40phy_save_restore_dig_filt_state(phy_info_t *pi, bool save, uint16 *filtcoeffs);
static int wlc_lcn40phy_load_tx_iir_filter(phy_info_t *pi, phy_tx_iir_filter_mode_t filter_mode,
	int16 filt_type);
static void wlc_lcn40phy_rx_gain_override_enable(phy_info_t *pi, bool enable);
static void wlc_lcn40phy_rx_pu(phy_info_t *pi, bool bEnable);
static void wlc_lcn40phy_clear_trsw_override(phy_info_t *pi);

static uint32 wlc_lcn40phy_papd_rxGnCtrl(phy_info_t *pi, phy_papd_cal_type_t cal_type,
	bool frcRxGnCtrl, uint8 CurTxGain);

static void wlc_lcn40phy_GetpapdMaxMinIdxupdt(phy_info_t *pi,
	int16 *maxUpdtIdx, int16 *minUpdtIdx);

static void wlc_lcn40phy_save_papd_calibration_results(phy_info_t *pi);
static void wlc_lcn40phy_set_rx_gain_by_distribution(phy_info_t *pi, uint16 trsw, uint16 ext_lna,
	uint16 slna_byp, uint16 slna_rout, uint16 slna_gain, uint16 lna2_gain, uint16 lna2_rout,
	uint16 tia, uint16 biq1, uint16 biq2, uint16 digi_gain, uint16 digi_offset);
static void wlc_lcn40phy_set_txpwr_clamp(phy_info_t *pi);

/* papr functions */
/* static void wlc_lcn40phy_papr_tx_power_offsets(phy_info_t *pi); */

static void wlc_lcn40phy_scbbmult_tx_gain_table(phy_info_t *pi, int mult_factor);
static int wlc_lcn40phy_scale_tx_iir_filter(phy_info_t *pi, phy_tx_iir_filter_mode_t mode,
	int16 filt_type, int shift_factor);

static void
wlc_lcn40phy_papd_cal_core(
	phy_info_t *pi,
	phy_papd_cal_type_t calType,
	bool rxGnCtrl,
	bool txGnCtrl,
	bool samplecapture,
	bool papd_dbg_mode,
	uint16 num_symbols,
	bool init_papd_lut,
	uint16 papd_bbmult_init,
	uint16 papd_bbmult_step,
	bool lpgn_only,
	bool papd_lpgn_ovr,
	uint16 LPGN_I,
	uint16 LPGN_Q);

static void wlc_lcn40phy_papd_cal_setup_cw(phy_info_t *pi, bool lpgn_only);
static void wlc_lcn40phy_epa_pd(phy_info_t *pi, bool disable);
static bool wlc_lcn40phy_rx_iq_est(phy_info_t *pi, uint16 num_samps, uint8 wait_time,
	uint8 wait_for_crs, phy_iq_est_t *iq_est, uint16 timeout_ms);
static bool wlc_lcn40phy_calc_rx_iq_comp(phy_info_t *pi,  uint16 num_samps);
static void wlc_lcn40phy_run_samples(phy_info_t *pi, uint16 num_samps, uint16 num_loops,
	uint16 wait, bool iqcalmode);
static void wlc_lcn40phy_set_trsw_override(phy_info_t *pi, bool tx, bool rx);
static void wlc_lcn40phy_papd_calc_capindex(phy_info_t *pi, phy_txcalgains_t *txgains);
static void wlc_lcn40phy_papd_idx_sel(phy_info_t *pi, int32 amam_cmp,
	phy_txcalgains_t *txgains, int8 step);
static void wlc_lcn40phy_load_txgainwithcappedindex(phy_info_t *pi, bool cap);
static void wlc_lcn40phy_filt_bw_set(phy_info_t *pi, uint16 bw);
static void wlc_lcn40phy_clkstall_WAR(phy_info_t *pi);
static void wlc_lcn40phy_agc_tweaks(phy_info_t *pi);
static void wlc_lcn40phy_rev0_agc_tweaks(phy_info_t *pi);
static void wlc_lcn40phy_rev1_agc_tweaks(phy_info_t *pi);
static void wlc_lcn40phy_rev3_agc_tweaks(phy_info_t *pi);
static void wlc_lcn40phy_rev4_agc_tweaks(phy_info_t *pi);
static void wlc_lcn40phy_rev6_agc_tweaks(phy_info_t *pi);
static void wlc_lcn40phy_rev7_agc_tweaks(phy_info_t *pi);
static void wlc_lcn40_spur_war(phy_info_t *pi, uint8 channel);
static void wlc_lcn40phy_set_ed_thres(phy_info_t *pi);
#if defined(WLC_LOWPOWER_BEACON_MODE)
static void wlc_lcn40phy_adc_lowpower(phy_info_t *pi, int adc_mode);
static void wlc_lcn40phy_lpf_lowpower(phy_info_t *pi, int lpf_mode);
static void wlc_lcn40phy_lna1_2g_lowpower(phy_info_t *pi, int lna1_mode);
static void wlc_lcn40phy_lna2_2g_lowpower(phy_info_t *pi, int lna2_mode);
static void wlc_lcn40phy_rxmix_2g_lowpower(phy_info_t *pi, int rxmix_mode);
#endif /* WLC_LOWPOWER_BEACON_MODE */
static void wlc_lcn40phy_tx_vco_freq_divider(phy_info_t *pi, uint8 channel);
static void wlc_lcn40phy_set_sfo_chan_centers(phy_info_t *pi, uint8 channel);
static void wlc_lcn40phy_adc_init(phy_info_t *pi, phy_adc_mode_t adc_mode, bool cal_mode);
static bool wlc_lcn40phy_is_papd_block_enable(phy_info_t *pi);
static void wlc_lcn40phy_papd_block_enable(phy_info_t *pi, bool enable);
static void wlc_lcn40phy_play_sample_table1(phy_info_t *pi, int32 f_Hz, uint16 max_val);
static void wlc_lcn40phy_tx_tone_samples(phy_info_t *pi, int32 f_Hz, uint16 max_val,
	uint32 *data_buf, uint32 phy_bw, uint16 num_samps);
static uint16 wlc_lcn40phy_num_samples(phy_info_t *pi, int32 f_Hz, uint32 phy_bw);
static void wlc_lcn40phy_papd_txiqlo_rxiq_enable(phy_info_t *pi, bool enable, chanspec_t chanspec);
static void wlc_lcn40phy_set_tssi_mux(phy_info_t *pi, lcn40phy_tssi_mode_t pos);
static void wlc_phy_txpwr_sromlcn40_read_ppr_parameters(phy_info_t *pi);
static void wlc_phy_txpwr_apply_sromlcn40(phy_info_t *pi, uint8 band, ppr_t *tx_srom_max_pwr);

static void wlc_lcn40phy_per_modulation_papd(phy_info_t *pi, uint8 enable,
	uint16 txctrlwordOverridePapdLutswap,
	uint16 StandardOverridepapdLutswap,
	uint16 HigherQAMOverridepapdLutswap);

#if defined(LP_P2P_SOFTAP) || defined(WL_LPC)
#ifdef WL_LPC
uint8 wlc_lcn40phy_lpc_getminidx(void);
void wlc_lcn40phy_lpc_setmode(phy_info_t *pi, bool enable);
uint8 wlc_lcn40phy_lpc_getoffset(uint8 index);
uint8 wlc_lcn40phy_lpc_get_txcpwrval(uint16 phytxctrlword);
void wlc_lcn40phy_lpc_set_txcpwrval(uint16 *phytxctrlword, uint8 txcpwrval);
#ifdef WL_LPC_DEBUG
uint8 * wlc_lcn40phy_lpc_get_pwrlevelptr(void);
#endif // endif
#endif /* WL_LPC */
static void wlc_lcn40phy_lpc_write_maclut(phy_info_t *pi);
#endif /* LP_P2P_SOFTAP || WL_LPC */
static void wlc_lcn40phy_noise_set_input_pwr_offset(phy_info_t *pi);
#if defined(WLC_LOWPOWER_BEACON_MODE)
static void wlc_lcn40phy_lowpower_beacon_mode(phy_info_t *pih, int lowpower_beacon_mode);
#endif /* WLC_LOWPOWER_BEACON_MODE */

/* Functions for fresh buffer access in wlc_phy_lcn40.c file */
#define LCN40PHY_MALLOC(pi, size) wlc_lcn40_malloc((pi)->u.pi_lcn40phy, size, __LINE__)
#define LCN40PHY_MFREE(pi, addr, size) wlc_lcn40_mfree((pi)->u.pi_lcn40phy, __LINE__)

static void *wlc_lcn40_malloc(phy_info_lcn40phy_t *pi_lcn40phy, uint16 size, uint32 line);
static void wlc_lcn40_mfree(phy_info_lcn40phy_t *pi_lcn40phy, uint32 line);

/* ZYX */
/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
/*  function implementation   					*/
/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
/* ATTACH */
bool
BCMATTACHFN(wlc_phy_attach_lcn40phy)(phy_info_t *pi)
{
	phy_info_lcnphy_t* pi_lcn;

	pi->u.pi_lcn40phy = (phy_info_lcn40phy_t*)MALLOC(pi->sh->osh, sizeof(phy_info_lcn40phy_t));
	if (pi->u.pi_lcn40phy == NULL) {
		PHY_ERROR(("wl%d: %s: MALLOC failure\n", pi->sh->unit, __FUNCTION__));
		return FALSE;
	}
	bzero((char *)pi->u.pi_lcn40phy, sizeof(phy_info_lcn40phy_t));

	pi_lcn = wlc_phy_getlcnphy_common(pi);

#if defined(PHYCAL_CACHING)
	/* Reset the var as no cal cache context should exist yet */
	pi->phy_calcache_num = 0;
#endif // endif
	if (!NORADIO_ENAB(pi->pubpi)) {
		pi->hwpwrctrl = TRUE;
		pi->hwpwrctrl_capable = TRUE;
	}
	/* Get xtal frequency from PMU */
	pi->xtalfreq = si_alp_clock(pi->sh->sih);
	ASSERT((PHY_XTALFREQ(pi->xtalfreq) % 1000) == 0);

	/* set papd_rxGnCtrl_init to 0 */
	pi_lcn->lcnphy_papd_rxGnCtrl_init = 0;

	PHY_INFORM(("wl%d: %s: using %d.%d MHz xtalfreq for RF PLL\n",
		pi->sh->unit, __FUNCTION__,
		PHY_XTALFREQ(pi->xtalfreq) / 1000000, PHY_XTALFREQ(pi->xtalfreq) % 1000000));

	pi->pi_fptr.calinit = wlc_phy_cal_init_lcn40phy;
	pi->pi_fptr.chanset = wlc_phy_chanspec_set_lcn40phy;
	pi->pi_fptr.settxpwrctrl = wlc_lcn40phy_set_tx_pwr_ctrl;
	pi->pi_fptr.settxpwrbyindex = wlc_lcn40phy_set_tx_pwr_by_index;
	pi->pi_fptr.ishwtxpwrctrl = wlc_phy_tpc_iovar_isenabled_lcn40phy;

	pi->pi_fptr.txiqccget = wlc_lcn40phy_get_tx_iqcc;
	pi->pi_fptr.txiqccset = wlc_lcn40phy_set_tx_iqcc;
	pi->pi_fptr.txloccget = wlc_lcn40phy_get_tx_locc;
	pi->pi_fptr.txloccset = wlc_lcn40phy_set_tx_locc;
	pi->pi_fptr.radioloftget = wlc_lcn40phy_get_radio_loft;
	pi->pi_fptr.radioloftset = wlc_lcn40phy_set_radio_loft;
	pi->pi_fptr.phywatchdog = wlc_phy_watchdog_lcn40phy;
	pi->pi_fptr.phybtcadjust = wlc_lcn40phy_btc_adjust;
#if defined(BCMDBG) || defined(WLTEST)
	pi->pi_fptr.longtrn = wlc_phy_long_train_lcn40phy;
#endif // endif
#if defined(WLTEST)
	pi->pi_fptr.carrsuppr = wlc_phy_carrier_suppress_lcn40phy;
#endif // endif
	pi->pi_fptr.calibmodes = wlc_lcn40phy_calib_modes;
#if defined(WLC_LOWPOWER_BEACON_MODE)
	pi->pi_fptr.lowpowerbeaconmode = wlc_lcn40phy_lowpower_beacon_mode;
#endif /* WLC_LOWPOWER_BEACON_MODE */

#ifdef WL_LPC
	pi->pi_fptr.lpcgetminidx = wlc_lcn40phy_lpc_getminidx;
	pi->pi_fptr.lpcgetpwros = wlc_lcn40phy_lpc_getoffset;
	pi->pi_fptr.lpcgettxcpwrval = wlc_lcn40phy_lpc_get_txcpwrval;
	pi->pi_fptr.lpcsettxcpwrval = wlc_lcn40phy_lpc_set_txcpwrval;
	pi->pi_fptr.lpcsetmode = wlc_lcn40phy_lpc_setmode;
#ifdef WL_LPC_DEBUG
	pi->pi_fptr.lpcgetpwrlevelptr = wlc_lcn40phy_lpc_get_pwrlevelptr;
#endif // endif
#endif /* WL_LPC */

	/* These function are not defined for this phy. */
	pi->pi_fptr.txswctrlmapsetptr = NULL;
	pi->pi_fptr.txswctrlmapgetptr = NULL;

#if defined(WLTEST) || defined(BCMDBG)
	pi->pi_fptr.epadpdsetptr = NULL;
#endif // endif

	if (CHIPID(pi->sh->chip) == BCM4314_CHIP_ID ||
		CHIPID(pi->sh->chip) == BCM43142_CHIP_ID ||
		CHIPID(pi->sh->chip) == BCM43143_CHIP_ID) {
		uint16 data = 0;

		/* Read RCAL value from OTP */
		if (!otp_read_word(pi->sh->sih, LCN40PHY_RCAL_OFFSET, &data))
			data &= 0xf;

		pi->u.pi_lcn40phy->rcal = (uint8)(data ? data : 0xa);
		PHY_INFORM(("wl%d: %s:  RCAL set to %x, otp value %x\n",
			pi->sh->unit, __FUNCTION__, pi->u.pi_lcn40phy->rcal, data));
	} else if (LCN40REV_IS(pi->pubpi.phy_rev, 7)) {
		pi->u.pi_lcn40phy->rcal = 0x8;
	} else {
		pi->u.pi_lcn40phy->rcal = 0xb;
	}

	/* Read nvram parameters */
	if (!wlc_lcn40phy_txpwr_srom_read(pi))
		return FALSE;

#if defined(LP_P2P_SOFTAP)
	/* Update the MACAddr LUT */
	if (pi_lcn->pwr_offset_val)
		wlc_lcn40phy_lpc_write_maclut(pi);
#elif defined(WL_LPC)
	/* Enable the LCN Phy specific var and MACADDR LUT */
	wlc_lcn40phy_lpc_setmode(pi, TRUE);
#endif // endif

	wlc_lcn40phy_noise_attach(pi);

	return TRUE;
}

static void *
wlc_lcn40_malloc(phy_info_lcn40phy_t *pi_lcn40phy, uint16 size, uint32 line)
{
	uint8 *ret_ptr = NULL;
	if (pi_lcn40phy->calbuffer_inuse) {
		PHY_ERROR(("FATAL Error: Concurrent LCN40PHY memory allocation @ line %d\n", line));
		ASSERT(FALSE);
	} else if (size > LCN40PHY_CALBUFFER_MAX_SZ) {
		PHY_ERROR(("FATAL Error: Buffer size (%d) required > MAX @ line %d\n", size, line));
		ASSERT(FALSE);
	} else {
		/* printf("Allocation @ line %d ...", line); */
		pi_lcn40phy->calbuffer_inuse = TRUE;
		ret_ptr = pi_lcn40phy->calbuffer;
	}
	return ret_ptr;
}

static void
wlc_lcn40_mfree(phy_info_lcn40phy_t *pi_lcn40phy, uint32 line)
{
	if (!pi_lcn40phy->calbuffer_inuse) {
		PHY_ERROR(("FATAL Error: MFree called but no prev alloc @ line %d\n", line));
		ASSERT(FALSE);
	} else {
		/* printf("Deallocation @ line %d\n", line); */
		pi_lcn40phy->calbuffer_inuse = FALSE;
	}
}

static void
wlc_lcn40phy_clkstall_WAR(phy_info_t *pi)
{
	phy_utils_write_phyreg(pi, LCN40PHY_resetCtrl, 0x0005);
	phy_utils_write_phyreg(pi, LCN40PHY_resetCtrl, 0x0);
}

/* INIT */
void
WLBANDINITFN(wlc_phy_init_lcn40phy)(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	phy_info_lcn40phy_t *pi_lcn40 = pi->u.pi_lcn40phy;
	uint16 lcn40phyregs_shm_addr;

	PHY_TRACE(("%s:\n", __FUNCTION__));
	pi_lcn->lcnphy_cal_counter = 0;
	pi_lcn->lcnphy_capped_index = 0;
	pi_lcn->lcnphy_calreqd = 0;
	pi_lcn->lcnphy_CalcPapdCapEnable = 0;
	pi_lcn40->cond_offs1 = 0;
	pi_lcn40->cond_offs2 = 0;
	pi_lcn40->cond_offs3 = 0;
	pi_lcn40->cond_offs4 = 0;
	pi_lcn40->sample_collect_gainidx = 0xff;

#if !defined(PHYCAL_CACHING)
	pi_lcn->lcnphy_cal_temper = pi_lcn->lcnphy_rawtempsense;
#endif // endif
	if ((CHSPEC_IS2G(pi->radio_chanspec) &&
		pi_lcn->extpagain2g) ||
		(CHSPEC_IS5G(pi->radio_chanspec) &&
		pi_lcn->extpagain5g)) {
		pi_lcn->ePA = 1;
	} else {
		pi_lcn->ePA = 0;
	}
	/* 4334/43342 always enable ePA */
	if ((CHIPID(pi->sh->chip) == BCM4334_CHIP_ID) ||
		(CHIPID(pi->sh->chip) == BCM43342_CHIP_ID))
			pi_lcn->ePA = 1;

	/* Initialize PAPD enabling and linearization path */
	if ((CHSPEC_IS2G(pi->radio_chanspec) &&
		pi_lcn->extpagain2g) ||
		(CHSPEC_IS5G(pi->radio_chanspec) &&
		pi_lcn->extpagain5g))
		if ((BOARDTYPE(pi->sh->boardtype) == BCM94334FCAGBI_SSID) ||
			(BOARDTYPE(pi->sh->boardtype) == BCM94334WLAGBI_SSID) ||
			(BOARDTYPE(pi->sh->boardtype) == BCM943342FCAGBI_SSID))
			pi_lcn40->papd_lin_path = LCN40PHY_PAPDLIN_PAD;
		else
			pi_lcn40->papd_lin_path = LCN40PHY_PAPDLIN_EPA;
	else
		pi_lcn40->papd_lin_path = LCN40PHY_PAPDLIN_IPA;

	if (LCN40_LINPATH(pi_lcn40->papd_lin_path) == LCN40PHY_PAPDLIN_IPA)
		pi_lcn40->papd_enable = TRUE;
	else
		pi_lcn40->papd_enable = FALSE;

	/* Modify PAPD enabling or linearization path if nvram param exists */
	if (CHSPEC_IS2G(pi->radio_chanspec) && (pi_lcn40->papden2g != -1)) {
		pi_lcn40->papd_enable = pi_lcn40->papden2g != 0;
		if (pi_lcn40->papdlinpath2g != -1)
			pi_lcn40->papd_lin_path = pi_lcn40->papdlinpath2g;
	}
#ifdef BAND5G
	else if (CHSPEC_IS5G(pi->radio_chanspec) && (pi_lcn40->papden5g != -1)) {
		pi_lcn40->papd_enable = pi_lcn40->papden5g != 0;
		if (pi_lcn40->papdlinpath5g != -1)
			pi_lcn40->papd_lin_path = pi_lcn40->papdlinpath5g;
	}
#endif /* BAND5G */

	if (LCN40REV_IS(pi->pubpi.phy_rev, 5)) {
		uint8 enable = 0;

		if (CHSPEC_IS2G(pi->radio_chanspec))
			enable = (pi_lcn40->papdrx2g >> 4) & 0x1;
#ifdef BAND5G
		else
			enable = (pi_lcn40->papdrx5g >> 4) & 0x1;
#endif /* BAND5G */
		wlc_lcn40phy_per_modulation_papd(pi, enable, 0, 0, 0);
	}

	/* PAPD enabled for ePA */
	if (!PHY_EPA_SUPPORT(pi_lcn->ePA))
		pi_lcn->lcnphy_papd_4336_mode = TRUE;

	/* reset the radio */
	wlc_lcn40phy_radio_reset(pi);

	/* Initialize baseband : bu_tweaks is a placeholder */
	wlc_lcn40phy_baseband_init(pi);
	/* Initialize radio */
	wlc_lcn40phy_radio_init(pi);
	/* run minipmu cal */
	wlc_lcn40phy_minipmu_cal(pi);
	/* run rcal */
	wlc_lcn40phy_rcal(pi);
	/* run rc_cal */
	if (!pi_lcn->rccalcache_valid)
		wlc_lcn40phy_rc_cal(pi);
	else
		wlc_lcn40phy_restore_rc_cal(pi);
	/* Initialize power control */
	wlc_lcn40phy_tx_pwr_ctrl_init(pi);
	/* Tune to the current channel */
	/* mapped to lcn40phy_set_chan_raw minus the agc_temp_init, txpwrctrl init */
	/* Forcing the cal during the init itself */
#if !defined(PHYCAL_CACHING)
	pi_lcn->lcnphy_full_cal_channel = 0;
#else
	#if defined(WLTEST)
	{
	ch_calcache_t *ctx = wlc_phy_get_chanctx(pi, pi->radio_chanspec);
	if (ctx)
		ctx->valid = 0;
	}
	#endif
#endif /* PHYCAL_CAHING */

	wlc_phy_chanspec_set((wlc_phy_t*)pi, pi->radio_chanspec);

	wlc_lcn40phy_clkstall_WAR(pi);

	/* save default params for AGC temp adjustments */
	wlc_lcn40phy_agc_temp_init(pi);

	wlc_lcn40phy_temp_adj(pi);

	pi_lcn->lcnphy_noise_samples = PHY_NOISE_SAMPLES_DEFAULT;
	/* set the flag so we can reduce band switch by avoiding some of the init code */
	pi->fast_bs = 1;
	PHY_REG_LIST_START
		PHY_REG_MOD_ENTRY(LCN40PHY, agcControl4, c_agc_fsm_en, 0)
		PHY_REG_MOD_ENTRY(LCN40PHY, agcControl4, c_agc_fsm_en, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, agcControl4, c_agc_fsm_en, 0)
		PHY_REG_MOD_ENTRY(LCN40PHY, agcControl4, c_agc_fsm_en, 1)
	PHY_REG_LIST_EXECUTE(pi);

	lcn40phyregs_shm_addr =
		2 * wlapi_bmac_read_shm(pi->sh->physhim, M_LCN40PHYREGS_PTR);

	if (pi_lcn40->rssi_iqest_en) {
		wlapi_bmac_write_shm(pi->sh->physhim, lcn40phyregs_shm_addr + M_RSSI_NSAMPS,
			1 << pi_lcn40->rssi_log_nsamps);
		wlapi_bmac_write_shm(pi->sh->physhim, lcn40phyregs_shm_addr + M_RSSI_LOGNSAMPS,
			pi_lcn40->rssi_log_nsamps);
		wlapi_bmac_write_shm(pi->sh->physhim, lcn40phyregs_shm_addr + M_RSSI_IQEST_PENDING,
			0);
	}

	wlapi_bmac_write_shm(pi->sh->physhim, lcn40phyregs_shm_addr + M_RSSI_IQEST_EN,
		pi_lcn40->rssi_iqest_en);

	if (pi_lcn40->noise_iqest_en) {
		wlapi_bmac_write_shm(pi->sh->physhim, lcn40phyregs_shm_addr + M_NOISE_NSAMPS,
			1 << pi_lcn40->noise_log_nsamps);
		wlapi_bmac_write_shm(pi->sh->physhim, lcn40phyregs_shm_addr + M_NOISE_LOGNSAMPS,
			pi_lcn40->noise_log_nsamps);
		wlapi_bmac_write_shm(pi->sh->physhim, lcn40phyregs_shm_addr + M_NOISE_IQEST_EN,
			pi_lcn40->noise_iqest_en);
		wlapi_bmac_write_shm(pi->sh->physhim, lcn40phyregs_shm_addr + M_NOISE_IQEST_PENDING,
			0);
	}
}

/* To scale down bbmult0 if necessary to accomodate high AM-AM value and avoid digital clipping */
static void
wlc_lcn40phy_papd_set_bbmult0(phy_info_t *pi, int flag)
{
	if (flag == 0) {
		/* set to default */
		PHY_REG_MOD(pi, LCN40PHY, bbmult0, bbmult0_enable, 1);
		PHY_REG_MOD(pi, LCN40PHY, bbmult0, bbmult0_coeff, 64);
	} else {
		phytbl_info_t tab;
		uint32 raw, amam_max;
		uint32 amam_thresh;
		int16 i, j;

		tab.tbl_ptr = &raw; /* ptr to buf */
		tab.tbl_len = 1;        /* # values   */
		tab.tbl_id = LCN40PHY_TBL_ID_PAPDCOMPDELTATBL;         /* papdcompdeltatbl */
		tab.tbl_width = 32;     /* 32 bit wide */
		tab.tbl_offset = 63; /* tbl offset */
		wlc_lcn40phy_read_table(pi,  &tab);

		/* i, j are two's complement. mask them out and convert them to int. */
		i = (int16)((raw >> 12) << 4);
		i = i >> 4;
		j = (int16)((raw & 0xfff) << 4);
		j = j >> 4;
		amam_max = phy_utils_sqrt_int((uint32)(i*i + j*j));

		if (CHIPID(pi->sh->chip) == BCM43143_CHIP_ID) {
			if (CHSPEC_IS40(pi->radio_chanspec))
				amam_thresh = LCN40_AMAM_THRESHOLD_40MHZ;
			else
				amam_thresh = LCN40_AMAM_THRESHOLD_43143;
		} else if (LCN40REV_IS(pi->pubpi.phy_rev, 7)) {
			if (CHSPEC_IS40(pi->radio_chanspec))
				amam_thresh = LCN40_AMAM_THRESHOLD_40MHZ_43341;
			else
				amam_thresh = LCN40_AMAM_THRESHOLD_43341;
		} else {
			if (CHSPEC_IS40(pi->radio_chanspec))
				amam_thresh = LCN40_AMAM_THRESHOLD_40MHZ;
			else
				amam_thresh = LCN40_AMAM_THRESHOLD;
		}
		/* 43143 SS at 0 degC and 3V6 suffers from EVM hump when bbmult0 is increased,
		 * so don't increase for 43143
		 */
		if ((amam_max > amam_thresh) ||
			(CHSPEC_IS40(pi->radio_chanspec) && (amam_max < 218) &&
			(CHIPID(pi->sh->chip) != BCM43143_CHIP_ID))) {
			uint32 old_bbmult0, new_bbmult0;

			old_bbmult0 = PHY_REG_READ(pi, LCN40PHY, bbmult0, bbmult0_coeff);
			/* need rounding here */
			new_bbmult0 = (old_bbmult0 * amam_thresh) / amam_max;
			PHY_REG_MOD(pi, LCN40PHY, bbmult0, bbmult0_coeff, new_bbmult0);

			PHY_ERROR(("wl%d: %s: bbmult0 changed from %d => %d\n", pi->sh->unit,
				__FUNCTION__, old_bbmult0, new_bbmult0));
		}
	}
}
static void
wlc_lcn40phy_agc_tweaks(phy_info_t *pi)
{
	if (LCN40REV_IS(pi->pubpi.phy_rev, 0) ||
		LCN40REV_IS(pi->pubpi.phy_rev, 2))
		wlc_lcn40phy_rev0_agc_tweaks(pi);
	else if (LCN40REV_IS(pi->pubpi.phy_rev, 1))
		wlc_lcn40phy_rev1_agc_tweaks(pi);
	else if (LCN40REV_IS(pi->pubpi.phy_rev, 3))
		wlc_lcn40phy_rev3_agc_tweaks(pi);
	else if (LCN40REV_IS(pi->pubpi.phy_rev, 4))
		wlc_lcn40phy_rev4_agc_tweaks(pi);
	else if (LCN40REV_IS(pi->pubpi.phy_rev, 5))
		wlc_lcn40phy_rev4_agc_tweaks(pi);
	else if (LCN40REV_IS(pi->pubpi.phy_rev, 6))
		wlc_lcn40phy_rev6_agc_tweaks(pi);
	else if (LCN40REV_IS(pi->pubpi.phy_rev, 7))
		wlc_lcn40phy_rev7_agc_tweaks(pi);
	else
		wlc_lcn40phy_rev0_agc_tweaks(pi);
}

#if defined(WLC_LOWPOWER_BEACON_MODE)
static void
wlc_lcn40phy_adc_lowpower(phy_info_t *pi, int adc_lowpower_mode)
{
	if (adc_lowpower_mode >= 0 && adc_lowpower_mode <= 4) {
		PHY_REG_LIST_START
			/* set reg(RF_OVR1.ovr_afe_iqadc_LF_order) 1; #loop filter order ovr */
			RADIO_REG_OR_ENTRY(RADIO_2065_OVR1, 1<<1)

			/* set reg(RF_OVR2.ovr_afe_iqadc_pwrup_Ich) 1; #adc power up ovr */
			RADIO_REG_OR_ENTRY(RADIO_2065_OVR2, 1<<15)

			/* set reg(RF_OVR2.ovr_afe_iqadc_pwrup_Qch) 1 */
			RADIO_REG_OR_ENTRY(RADIO_2065_OVR2, 1<<14)

			/* set reg(RF_OVR2.ovr_afe_iqadc_WLAN_LP) 1; # lowpower ovr */
			RADIO_REG_OR_ENTRY(RADIO_2065_OVR2, 1<<8)
			/* set reg(RF_OVR1.ovr_afe_iqadc_flash_only) 1; #flash only ovr */
			RADIO_REG_OR_ENTRY(RADIO_2065_OVR1, 1<<3)

			/* set reg(RF_OVR1.ovr_afe_iqadc_flash_rez) 1; #flash resolution ovr */
			RADIO_REG_OR_ENTRY(RADIO_2065_OVR1, 1<<2)
		PHY_REG_LIST_EXECUTE(pi);
	}

	switch (adc_lowpower_mode) {
	case 0:
		PHY_REG_LIST_START
			/* set reg(RF_adc_cfg4.iqadc_LF_order) 0 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_ADC_CFG4, 0x0200, 0<<9)
			/* set reg(RF_adc_cfg1.Ich_pwrup) 0x1f */
			RADIO_REG_MOD_ENTRY(RADIO_2065_ADC_CFG1, 0x001f, 0x1f)

			/* set reg(RF_adc_cfg1.Qch_pwrup) 0x1f */
			RADIO_REG_MOD_ENTRY(RADIO_2065_ADC_CFG1, 0x1f00, (0x1f)<<8)

			/* set reg(RF_adc_cfg4.iqadc_WLAN_LP) 0 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_ADC_CFG4, 0x2000, 0<<13)

			/* set reg(RF_adc_cfg4.iqadc_flash_only) */
			RADIO_REG_MOD_ENTRY(RADIO_2065_ADC_CFG4, 0x1000, 0<<12)

			/* set reg(RF_adc_cfg4.iqadc_flash_rez) 0 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_ADC_CFG4, 0x0800, 0<<11)
		PHY_REG_LIST_EXECUTE(pi);
		break;
	case 1:
		PHY_REG_LIST_START
			/* set reg(RF_adc_cfg4.iqadc_LF_order) 1 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_ADC_CFG4, 0x0200, 1<<9)

			/* set reg(RF_adc_cfg1.Ich_pwrup) 0x19 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_ADC_CFG1, 0x001f, 0x19)
			/* set reg(RF_adc_cfg1.Qch_pwrup) 0x19 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_ADC_CFG1, 0x1f00, (0x19)<<8)

			/* set reg(RF_adc_cfg4.iqadc_WLAN_LP) 1 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_ADC_CFG4, 0x2000, 1<<13)

			/* set reg(RF_adc_cfg4.iqadc_flash_only) 0 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_ADC_CFG4, 0x1000, 0<<12)
			/* set reg(RF_adc_cfg4.iqadc_flash_rez) 0 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_ADC_CFG4, 0x0800, 0<<11)
		PHY_REG_LIST_EXECUTE(pi);
		break;
	case 2:
		PHY_REG_LIST_START
			/* set reg(RF_adc_cfg4.iqadc_LF_order) 1 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_ADC_CFG4, 0x0200, 1<<9)

			/* set reg(RF_adc_cfg1.Ich_pwrup) 0x19 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_ADC_CFG1, 0x001f, 0x19)

			/* set reg(RF_adc_cfg1.Qch_pwrup) 0x19 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_ADC_CFG1, 0x1f00, (0x19)<<8)

			/* set reg(RF_adc_cfg4.iqadc_WLAN_LP) 1 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_ADC_CFG4, 0x2000, 1<<13)

			/* set reg(RF_adc_cfg4.iqadc_flash_only) 0 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_ADC_CFG4, 0x1000, 0<<12)

			/* set reg(RF_adc_cfg4.iqadc_flash_rez) 1 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_ADC_CFG4, 0x0800, 1<<11)
		PHY_REG_LIST_EXECUTE(pi);
		break;

	case 3:
		PHY_REG_LIST_START
			/* set reg(RF_adc_cfg4.iqadc_LF_order) 1 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_ADC_CFG4, 0x0200, 1<<9)

			/* set reg(RF_adc_cfg1.Ich_pwrup) 0x1 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_ADC_CFG1, 0x001f, 1)

			/* set reg(RF_adc_cfg1.Qch_pwrup) 0x1 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_ADC_CFG1, 0x1f00, 1<<8)

			/* set reg(RF_adc_cfg4.iqadc_WLAN_LP) 1 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_ADC_CFG4, 0x2000, 1<<13)

			/* set reg(RF_adc_cfg4.iqadc_flash_only) 1 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_ADC_CFG4, 0x1000, 1<<12)

			/* set reg(RF_adc_cfg4.iqadc_flash_rez) 0 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_ADC_CFG4, 0x0800, 0<<11)
		PHY_REG_LIST_EXECUTE(pi);
		break;

	case 4:
		PHY_REG_LIST_START
			/* set reg(RF_adc_cfg4.iqadc_LF_order) 1 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_ADC_CFG4, 0x0200, 1<<9)

			/* set reg(RF_adc_cfg1.Ich_pwrup) 0x1 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_ADC_CFG1, 0x001f, 1)

			/* set reg(RF_adc_cfg1.Qch_pwrup) 0x1 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_ADC_CFG1, 0x1f00, 1<<8)

			/* set reg(RF_adc_cfg4.iqadc_WLAN_LP) 1 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_ADC_CFG4, 0x2000, 1<<13)

			/* set reg(RF_adc_cfg4.iqadc_flash_only) 1 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_ADC_CFG4, 0x1000, 1<<12)

			/* set reg(RF_adc_cfg4.iqadc_flash_rez) 1 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_ADC_CFG4, 0x0800, 1<<11)
		PHY_REG_LIST_EXECUTE(pi);
		break;
	}
}

static void
wlc_lcn40phy_lpf_lowpower(phy_info_t *pi, int lpf_lowpower_mode)
{

	if (lpf_lowpower_mode >= 0 && lpf_lowpower_mode <= 2) {
		PHY_REG_LIST_START
			/* set reg(RF_OVR6.ovr_lpf_puI) 1 */
			RADIO_REG_OR_ENTRY(RADIO_2065_OVR6, 1<<4)
			/* set reg(RF_OVR6.ovr_lpf_puQ) 1 */
			RADIO_REG_OR_ENTRY(RADIO_2065_OVR6, 1<<3)

			/* set reg(RF_OVR7.ovr_lpf_sel_byp_rxlpf) 1 */
			RADIO_REG_OR_ENTRY(RADIO_2065_OVR7, 1<<5)
		PHY_REG_LIST_EXECUTE(pi);
	}

	if (lpf_lowpower_mode == 0) {
		PHY_REG_LIST_START
			/* following 3 ovr customized by both 2nd-order mode and bypass mode */
			/* need to be reversed */
			/* set reg(RF_OVR6.ovr_lpf_puI) 0 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_OVR6, 0x0010, 0<<4)
			/* set reg(RF_OVR6.ovr_lpf_puQ) 0 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_OVR6, 0x0008, 0<<3)
			/* set reg(RF_OVR7.ovr_lpf_sel_byp_rxlpf) 0 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_OVR7, 0x0020, 0<<5)

			/* following 4 ovr changed by lpf in 2nd order mode, need to be reversed */
			/* set reg(RF_OVR6.ovr_lpf_bq2_gain) 0 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_OVR6, 0x1000, 0<<12)
			/* set reg(RF_OVR6.ovr_lpf_bq2_bw) 0 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_OVR6, 0x4000, 0<<14)
			/* set reg(RF_OVR16.ovr_dclp_from_bq1) 0 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_OVR16, 0x0010, 0<<4)
			/* set reg(RF_OVR16.ovr_bq1_1p8x_bw) 0 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_OVR16, 0x0020, 0<<5)

		/* following 10 ovr changed by lpf completely bypassed mode, need to be reversed */
			/* set reg(RF_OVR5.ovr_lpf_bias_puI) 0 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_OVR5, 0x0008, 0<<3)
			/* set reg(RF_OVR5.ovr_lpf_bias_puQ) 0 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_OVR5, 0x0004, 0<<2)
			/* set reg(RF_OVR7.ovr_lpf_rxbuf_puI) 0 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_OVR7, 0x0080, 0<<7)
			/* set reg(RF_OVR7.ovr_lpf_rxbuf_puQ) 0 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_OVR7, 0x0040, 0<<6)
			/* set reg(RF_OVR7.ovr_lpf_sel_tx_rx)  0 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_OVR7, 0x0004, 0<<2)
			/* set reg(RF_OVR7.ovr_lpf_sel_byp_txlpf)  0 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_OVR7, 0x0010, 0<<4)
			/* set reg(RF_OVR7.ovr_lpf_sel_rx_buffer)  0 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_OVR7, 0x0008, 0<<3)
			/* set reg(RF_OVR7.ovr_lpf_rxbuf_gain) 0 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_OVR7, 0x0100, 0<<8)
		PHY_REG_LIST_EXECUTE(pi);
		/* set dot11lpphyreg(rfoverride4.lpf_byp_dc_ovr) 0 */
		PHY_REG_MOD(pi, LCN40PHY, rfoverride4, lpf_byp_dc_ovr, 0);
		/* set dot11lpphyreg(rfoverride7val.tia_hpc_ovr_val) 1 */
		PHY_REG_MOD(pi, LCN40PHY, rfoverride7val, tia_hpc_ovr_val, 1);
	}
	else if (lpf_lowpower_mode == 1) {
		PHY_REG_LIST_START
		/* following 10 ovr changed by lpf completely bypassed mode, need to be reversed */
			/* set reg(RF_OVR5.ovr_lpf_bias_puI) 0 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_OVR5, 0x0008, 0<<3)
			/* set reg(RF_OVR5.ovr_lpf_bias_puQ) 0 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_OVR5, 0x0004, 0<<2)
			/* set reg(RF_OVR7.ovr_lpf_rxbuf_puI) 0 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_OVR7, 0x0080, 0<<7)
			/* set reg(RF_OVR7.ovr_lpf_rxbuf_puQ) 0 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_OVR7, 0x0040, 0<<6)
			/* set reg(RF_OVR7.ovr_lpf_sel_tx_rx)  0 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_OVR7, 0x0004, 0<<2)
			/* set reg(RF_OVR7.ovr_lpf_sel_byp_txlpf)  0 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_OVR7, 0x0010, 0<<4)
			/* set reg(RF_OVR7.ovr_lpf_sel_rx_buffer)  0 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_OVR7, 0x0008, 0<<3)
			/* set reg(RF_OVR7.ovr_lpf_rxbuf_gain) 0 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_OVR7, 0x0100, 0<<8)
			/* set dot11lpphyreg(rfoverride4.lpf_byp_dc_ovr) 0 */
		PHY_REG_LIST_EXECUTE(pi);
		PHY_REG_MOD(pi, LCN40PHY, rfoverride4, lpf_byp_dc_ovr, 0);
		/* set dot11lpphyreg(rfoverride7val.tia_hpc_ovr_val) 1 */
		PHY_REG_MOD(pi, LCN40PHY, rfoverride7val, tia_hpc_ovr_val, 1);

		PHY_REG_LIST_START
			/* set reg(RF_lpf_cfg1.puI) 0x1 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_LPF_CFG1, 0x000c, 1<<2)
			/* set reg(RF_lpf_cfg1.puQ) 0x1 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_LPF_CFG1, 0x0003, 1)

			/* set reg(RF_lpf_cfg1.sel_byp_rxlpf) 3; #connect bq1 to adc */
			RADIO_REG_MOD_ENTRY(RADIO_2065_LPF_CFG1, 0xc000, 3<<14)

			/* #dcloop to bq1 input */
			/* set reg(RF_OVR16.ovr_dclp_from_bq1) 1 */
			RADIO_REG_OR_ENTRY(RADIO_2065_OVR16, 1<<4)
			/* set reg(RF_lpf_misc.dclp_from_bq1) 3 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_LPF_MISC, 0x0300, 3<<8)

			/* set reg(RF_OVR6.ovr_lpf_bq2_gain) 1 */
			RADIO_REG_OR_ENTRY(RADIO_2065_OVR6, 1<<12)
			/* set reg(RF_lpf_gain.bq2_gain) 0 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_LPF_GAIN, 0x00f0, 0<<4)

			/* set reg(RF_OVR6.ovr_lpf_bq2_bw) 1 */
			RADIO_REG_OR_ENTRY(RADIO_2065_OVR6, 1<<14)
			/* set reg(RF_lpf_resp_bq2.bq2_bw) 6 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_LPF_RESP_BQ2, 0x0007, 6)

			/* set reg(RF_OVR16.ovr_bq1_1p8x_bw) 1 */
			RADIO_REG_OR_ENTRY(RADIO_2065_OVR16, 1<<5)
			/* set reg(RF_lpf_misc.bq1_1p8x_bw) 3 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_LPF_MISC, 0x0c00, 3<<10)
		PHY_REG_LIST_EXECUTE(pi);
	}
	else if (lpf_lowpower_mode == 2) {
		PHY_REG_LIST_START
			/* following 4 ovr changed by lpf in 2nd order mode, need to be reversed */
			/* set reg(RF_OVR6.ovr_lpf_bq2_gain) 0 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_OVR6, 0x1000, 0<<12)
			/* set reg(RF_OVR6.ovr_lpf_bq2_bw) 0 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_OVR6, 0x4000, 0<<14)
			/* set reg(RF_OVR16.ovr_dclp_from_bq1) 0 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_OVR16, 0x0010, 0<<4)
			/* set reg(RF_OVR16.ovr_bq1_1p8x_bw) 0 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_OVR16, 0x0020, 0<<5)

			/* power controls */
			/* set reg(RF_OVR5.ovr_lpf_bias_puI) 1 */
			RADIO_REG_OR_ENTRY(RADIO_2065_OVR5, 1<<3)
			/* set reg(RF_lpf_cfg1.bias_puI) 1; */
			/* needs to turn on since it supplies current to rxbuffer */
			RADIO_REG_MOD_ENTRY(RADIO_2065_LPF_CFG1, 0x0020, 1<<5)
			/* set reg(RF_OVR5.ovr_lpf_bias_puQ) 1 */
			RADIO_REG_OR_ENTRY(RADIO_2065_OVR5, 1<<2)
			/* set reg(RF_lpf_cfg1.bias_puQ) 1 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_LPF_CFG1, 0x0010, 1<<4)

			/* set reg(RF_lpf_cfg1.puI) 0x0 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_LPF_CFG1, 0x000c, 0<<2)
			/* set reg(RF_lpf_cfg1.puQ) 0x0 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_LPF_CFG1, 0x0003, 0)

			/* set reg(RF_OVR7.ovr_lpf_rxbuf_puI) 1 */
			RADIO_REG_OR_ENTRY(RADIO_2065_OVR7, 1<<7)
			/* set reg(RF_lpf_cfg1.rxbuf_puI) 1 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_LPF_CFG1, 0x0080, 1<<7)
			/* set reg(RF_OVR7.ovr_lpf_rxbuf_puQ) 1 */
			RADIO_REG_OR_ENTRY(RADIO_2065_OVR7, 1<<6)
			/* set reg(RF_lpf_cfg1.rxbuf_puQ) 1 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_LPF_CFG1, 0x0040, 1<<6)

			/* mux cotrols */
			/* set reg(RF_lpf_misc) 0 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_LPF_MISC, 0xffff, 0)
			/* set reg(RF_OVR7.ovr_lpf_sel_tx_rx)  1 */
			RADIO_REG_OR_ENTRY(RADIO_2065_OVR7, 1<<2)
			/* set reg(RF_lpf_cfg1.sel_tx_rx)      0 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_LPF_CFG1, 0x0c00, 0<<10)
			/* set reg(RF_OVR7.ovr_lpf_sel_byp_txlpf)  1 */
			RADIO_REG_OR_ENTRY(RADIO_2065_OVR7, 1<<4)
			/* set reg(RF_lpf_cfg1.sel_byp_txlpf)      0 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_LPF_CFG1, 0x0200, 0<<9)

			/* set reg(RF_lpf_cfg1.sel_byp_rxlpf)      0x1 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_LPF_CFG1, 0xc000, 1<<14)

			/* set reg(RF_OVR7.ovr_lpf_sel_rx_buffer)  1 */
			RADIO_REG_OR_ENTRY(RADIO_2065_OVR7, 1<<3)
			/* set reg(RF_lpf_cfg1.sel_rx_buffer)      0x1 */
			RADIO_REG_MOD_ENTRY(RADIO_2065_LPF_CFG1, 0x3000, 1<<12)

			/* set reg(RF_OVR7.ovr_lpf_rxbuf_gain) 1 */
			RADIO_REG_OR_ENTRY(RADIO_2065_OVR7, 1<<8)
			/* set reg(RF_lpf_gain.rxbuf_gain) 3; #rxbuf supply 6dB of gain */
			RADIO_REG_MOD_ENTRY(RADIO_2065_LPF_GAIN, 0x0f00, 3<<8)

			/* To disable LPF DC loop */
			/* set dot11lpphyreg(rfoverride4.lpf_byp_dc_ovr) 1 */
			PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride4, lpf_byp_dc_ovr, 1)

			/* turn on tia dc loop */
			/* set dot11lpphyreg(rfoverride7val.tia_hpc_ovr_val) 3 */
			PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride7val, tia_hpc_ovr_val, 3)
		PHY_REG_LIST_EXECUTE(pi);
	}
}

static void
wlc_lcn40phy_lna1_2g_lowpower(phy_info_t *pi, int lna1_2g_lowpower_mode)
{

	if (lna1_2g_lowpower_mode >= 0 && lna1_2g_lowpower_mode) {
		/* set reg(RF_OVR3.ovr_lna2g_lna1_low_ct) 1 */
		phy_utils_or_radioreg(pi, RADIO_2065_OVR3, 1<<8);
	}
	if (lna1_2g_lowpower_mode == 0) {
		/* set reg(RF_lna2g_cfg1.lna1_low_ct) 0 */
		phy_utils_mod_radioreg(pi, RADIO_2065_LNA2G_CFG1, 0x0002, 0<<1);
	}
	else if (lna1_2g_lowpower_mode == 1) {
		/* set reg(RF_lna2g_cfg1.lna1_low_ct) 1 */
		phy_utils_mod_radioreg(pi, RADIO_2065_LNA2G_CFG1, 0x0002, 1<<1);
	}
}

static void
	wlc_lcn40phy_lna2_2g_lowpower(phy_info_t *pi, int lna2_2g_lowpower_mode)
{
	if (lna2_2g_lowpower_mode >= 0 && lna2_2g_lowpower_mode <= 1) {
		/* set reg(RF_OVR3.ovr_lna2g_lna2_gm_size) 1 */
		phy_utils_or_radioreg(pi, RADIO_2065_OVR3, 1<<4);
	}
	if (lna2_2g_lowpower_mode == 0) {
		/* set reg(RF_lna2g_cfg2.lna2_gm_size) 3 */
		phy_utils_mod_radioreg(pi, RADIO_2065_LNA2G_CFG2, 0x3000, 3<<12);
	}
	else if (lna2_2g_lowpower_mode == 1) {
		/* set reg(RF_lna2g_cfg2.lna2_gm_size) 1 */
		phy_utils_mod_radioreg(pi, RADIO_2065_LNA2G_CFG2, 0x3000, 1<<12);
	}
}

static void
wlc_lcn40phy_rxmix_2g_lowpower(phy_info_t *pi, int rxmix_2g_lowpower_mode)
{

	if (rxmix_2g_lowpower_mode >= 0 && rxmix_2g_lowpower_mode <= 1) {
		/* set reg(RF_OVR11.ovr_rxmix2g_gm_size) 1 */
		phy_utils_or_radioreg(pi, RADIO_2065_OVR11, 1<<12);
		/* set reg(RF_OVR11.ovr_rxmix2g_gm_half_en) 1 */
		phy_utils_or_radioreg(pi, RADIO_2065_OVR11, 1<<13);
	}
	if (rxmix_2g_lowpower_mode == 0) {
		/* set reg(RF_rxmix2g_cfg1.gm_size) 1 */
		phy_utils_mod_radioreg(pi, RADIO_2065_RXMIX2G_CFG1, 0x0300, 1<<8);
		/* set reg(RF_rxmix2g_cfg1.gm_half_en) 0 */
		phy_utils_mod_radioreg(pi, RADIO_2065_RXMIX2G_CFG1, 0x0002, 0<<1);
	}
	else if (rxmix_2g_lowpower_mode == 1) {
		/* set reg(RF_rxmix2g_cfg1.gm_size) 0 */
		phy_utils_mod_radioreg(pi, RADIO_2065_RXMIX2G_CFG1, 0x0300, 0<<8);
		/* set reg(RF_rxmix2g_cfg1.gm_half_en) 1 */
		phy_utils_mod_radioreg(pi, RADIO_2065_RXMIX2G_CFG1, 0x0002, 1<<1);
	}
}
#endif /* WLC_LOWPOWER_BEACON_MODE */

static void
wlc_lcn40phy_rev0_agc_tweaks(phy_info_t *pi)
{
	uint16 clip_gain_2g[] = {56, 45, 42, 37, 32, 27, 23, 14, 11};
	uint16 clip_thresh_2g[] = {15, 12, 10, 10, 10, 10, 10, 10, 10};
	uint16 clip_gain_2g_extlna[] = {66, 61, 56, 49, 41, 36, 32, 13, 10};
	uint16 clip_thresh_2g_extlna[] = {10, 10, 10, 10, 10, 19, 10, 10, 10};
#ifdef BAND5G
	uint16 clip_gain_5g[] = {67, 63, 57, 47, 41, 36, 30, 17, 10};
	uint16 clip_gain_5g_extlna[] = {58, 50, 43, 38, 34, 31, 25, 20, 10};
	uint16 clip_thresh_5g[] = {30, 30, 30, 10, 10, 10, 15, 20, 10};
	uint16 clip_thresh_5g_extlna[] = {10, 10, 10, 10, 10, 10, 10, 10, 10};
#endif // endif
	uint16 *clip_gains = NULL, *clip_threshs = NULL;

	phy_info_lcn40phy_t *pi_lcn40 = pi->u.pi_lcn40phy;
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);

	PHY_REG_LIST_START
		PHY_REG_MOD_ENTRY(LCN40PHY, ClipCtrThresh, ClipCtrThreshHiGain, 15)
		RADIO_REG_MOD_ENTRY(RADIO_2065_NBRSSI_BIAS, 0xff03, 0x303)
		RADIO_REG_WRITE_ENTRY(RADIO_2065_NBRSSI_IB, 0x36e2)
		RADIO_REG_MOD_ENTRY(RADIO_2065_NBRSSI_CONFG, 0x7000, 0x3000)
		PHY_REG_MOD_ENTRY(LCN40PHY, crsMiscCtrl0, matchFiltEn, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, crsMiscCtrl0_new, matchFiltEn40, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, radioTRCtrl, gainrequestTRAttnOnEn, 0)
		PHY_REG_MOD_ENTRY(LCN40PHY, agcControl11, gain_settle_dly_cnt, 3)
	PHY_REG_LIST_EXECUTE(pi);

	if (CHSPEC_IS20(pi->radio_chanspec)) {
		PHY_REG_MOD(pi, LCN40PHY, ClipDetector, Clip_detecotr_thresh, 40);
	}
	/* Tuning to avoid being stuck at BPHY_CONFIRM, an FSM bug */
	PHY_REG_MOD(pi, LCN40PHY, DSSSConfirmCnt, DSSSConfirmCntLoGain, 3);
	/* bit inversion for 4334 A0 and 4334 A2 only */
	if (LCN40REV_LE(pi->pubpi.phy_rev, 2))
		PHY_REG_MOD(pi, LCN40PHY, agcControl4, c_pkt_rcv_clip_det_abort_disable, 1);

	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		if (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA) {
			if (pi_lcn40->trGain != 0xFF) {
				PHY_REG_MOD(pi, LCN40PHY, radioTRCtrlCrs1, trGainThresh,
					pi_lcn40->trGain);
				PHY_REG_MOD(pi, LCN40PHY, radioTRCtrl, gainrequestTRAttnOnEn, 1);
				PHY_REG_MOD(pi, LCN40PHY, radioTRCtrl, gainrequestTRAttnOnOffset,
					pi_lcn->lcnphy_tr_isolation_mid);
			}
			clip_gains = clip_gain_2g_extlna;
			clip_threshs = clip_thresh_2g_extlna;
		} else {
			clip_gains = clip_gain_2g;
			clip_threshs = clip_thresh_2g;
		}

		PHY_REG_LIST_START
			RADIO_REG_WRITE_ENTRY(RADIO_2065_LNA2G_RSSI, 0x51f1)
			PHY_REG_MOD_ENTRY(LCN40PHY, agcControl38, rssi_no_clip_gain_normal, 62)

			PHY_REG_MOD_ENTRY(LCN40PHY, SignalBlock_edet1, signalblk_det_thresh_dsss,
				99)
		PHY_REG_LIST_EXECUTE(pi);
	}
#ifdef BAND5G
	else {
		if (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA_5GHz) {
			clip_gains = clip_gain_5g_extlna;
			clip_threshs = clip_thresh_5g_extlna;
			PHY_REG_LIST_START
				PHY_REG_MOD_ENTRY(LCN40PHY, agcControl38, rssi_no_clip_gain_normal,
					60)
				PHY_REG_MOD_ENTRY(LCN40PHY, radioTRCtrl, gainrequestTRAttnOnEn, 1)
				PHY_REG_MOD_ENTRY(LCN40PHY, radioTRCtrlCrs1, trGainThresh, 35)
			PHY_REG_LIST_EXECUTE(pi);
			if (pi_lcn->triso5g[0] != 0xff)
				PHY_REG_MOD(pi, LCN40PHY, radioTRCtrl, gainrequestTRAttnOnOffset,
					pi_lcn->triso5g[0]);
			PHY_REG_MOD(pi, LCN40PHY, agcControl24, rssi_no_clip_gain_adj, 0);
		} else {
			clip_gains = clip_gain_5g;
			clip_threshs = clip_thresh_5g;
		}

		phy_utils_write_radioreg(pi, RADIO_2065_LNA5G_RSSI, 0x51f1);
		PHY_REG_MOD(pi, LCN40PHY, rxfe, bypass_iqcomp, 0);
		if (pi_lcn40->rx_iq_comp_5g[0])
		{
			PHY_REG_MOD(pi, LCN40PHY, RxCompcoeffa0, a0, pi_lcn40->rx_iq_comp_5g[0]);
			PHY_REG_MOD(pi, LCN40PHY, RxCompcoeffb0, b0, pi_lcn40->rx_iq_comp_5g[1]);
			PHY_REG_MOD(pi, LCN40PHY, RxCompcoeffa1, a1, pi_lcn40->rx_iq_comp_5g[0]);
			PHY_REG_MOD(pi, LCN40PHY, RxCompcoeffb1, b1, pi_lcn40->rx_iq_comp_5g[1]);
			PHY_REG_MOD(pi, LCN40PHY, RxCompcoeffa2, a2, pi_lcn40->rx_iq_comp_5g[0]);
			PHY_REG_MOD(pi, LCN40PHY, RxCompcoeffb2, b2, pi_lcn40->rx_iq_comp_5g[1]);
		}

		PHY_REG_LIST_START
			PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride7, lpf_hpc_ovr, 0x1)
			PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride7val, lpf_hpc_ovr_val, 3)
			PHY_REG_MOD_ENTRY(LCN40PHY, SignalBlock_edet1, signalblk_det_thresh_dsss,
				19)
		PHY_REG_LIST_EXECUTE(pi);

	}
#endif  /* BAND5G */
	if (clip_gains) {
		phy_utils_mod_phyreg(pi, LCN40PHY_agcControl14,
			LCN40PHY_agcControl14_rssi_clip_gain_norm_0_MASK |
			LCN40PHY_agcControl14_rssi_clip_gain_norm_1_MASK,
			(clip_gains[0] << LCN40PHY_agcControl14_rssi_clip_gain_norm_0_SHIFT) |
		(clip_gains[1] << LCN40PHY_agcControl14_rssi_clip_gain_norm_1_SHIFT));
		phy_utils_mod_phyreg(pi, LCN40PHY_agcControl15,
			LCN40PHY_agcControl15_rssi_clip_gain_norm_2_MASK |
			LCN40PHY_agcControl15_rssi_clip_gain_norm_3_MASK,
			(clip_gains[2] << LCN40PHY_agcControl15_rssi_clip_gain_norm_2_SHIFT) |
			(clip_gains[3] << LCN40PHY_agcControl15_rssi_clip_gain_norm_3_SHIFT));
		phy_utils_mod_phyreg(pi, LCN40PHY_agcControl16,
			LCN40PHY_agcControl16_rssi_clip_gain_norm_4_MASK |
			LCN40PHY_agcControl16_rssi_clip_gain_norm_5_MASK,
			(clip_gains[4] << LCN40PHY_agcControl16_rssi_clip_gain_norm_4_SHIFT) |
			(clip_gains[5] << LCN40PHY_agcControl16_rssi_clip_gain_norm_5_SHIFT));
		phy_utils_mod_phyreg(pi, LCN40PHY_agcControl17,
			LCN40PHY_agcControl17_rssi_clip_gain_norm_6_MASK |
			LCN40PHY_agcControl17_rssi_clip_gain_norm_7_MASK,
			(clip_gains[6] << LCN40PHY_agcControl17_rssi_clip_gain_norm_6_SHIFT) |
			(clip_gains[7] << LCN40PHY_agcControl17_rssi_clip_gain_norm_7_SHIFT));
		PHY_REG_MOD(pi, LCN40PHY, agcControl18, rssi_clip_gain_norm_8, clip_gains[8]);
	}

	if (clip_threshs) {
		phy_utils_mod_phyreg(pi, LCN40PHY_agcControl19,
			LCN40PHY_agcControl19_rssi_clip_thresh_norm_0_MASK |
			LCN40PHY_agcControl19_rssi_clip_thresh_norm_1_MASK,
			(clip_threshs[0] << LCN40PHY_agcControl19_rssi_clip_thresh_norm_0_SHIFT) |
			(clip_threshs[1] << LCN40PHY_agcControl19_rssi_clip_thresh_norm_1_SHIFT));
		phy_utils_mod_phyreg(pi, LCN40PHY_agcControl20,
			LCN40PHY_agcControl20_rssi_clip_thresh_norm_2_MASK |
			LCN40PHY_agcControl20_rssi_clip_thresh_norm_3_MASK,
			(clip_threshs[2] << LCN40PHY_agcControl20_rssi_clip_thresh_norm_2_SHIFT) |
			(clip_threshs[3] << LCN40PHY_agcControl20_rssi_clip_thresh_norm_3_SHIFT));
		phy_utils_mod_phyreg(pi, LCN40PHY_agcControl21,
			LCN40PHY_agcControl21_rssi_clip_thresh_norm_4_MASK |
			LCN40PHY_agcControl21_rssi_clip_thresh_norm_5_MASK,
			(clip_threshs[4] << LCN40PHY_agcControl21_rssi_clip_thresh_norm_4_SHIFT) |
			(clip_threshs[5] << LCN40PHY_agcControl21_rssi_clip_thresh_norm_5_SHIFT));
		phy_utils_mod_phyreg(pi, LCN40PHY_agcControl22,
			LCN40PHY_agcControl22_rssi_clip_thres_norm_6_MASK |
			LCN40PHY_agcControl22_rssi_clip_thresh_norm_7_MASK,
			(clip_threshs[6] << LCN40PHY_agcControl22_rssi_clip_thres_norm_6_SHIFT) |
			(clip_threshs[7] << LCN40PHY_agcControl22_rssi_clip_thresh_norm_7_SHIFT));
		PHY_REG_MOD(pi, LCN40PHY, agcControl23, rssi_clip_thresh_norm_8, clip_threshs[8]);
	}
	if (CHSPEC_IS2G(pi->radio_chanspec) && (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags)
		& BFL_EXTLNA) && CHSPEC_IS40(pi->radio_chanspec))
		PHY_REG_MOD(pi, LCN40PHY, agcControl21, rssi_clip_thresh_norm_5, 10);

	if (LCN40REV_LE(pi->pubpi.phy_rev, 2))
		PHY_REG_MOD(pi, LCN40PHY, CFOblkConfig, signalblk_det_thresh_ofdm_40, 94);
	if (LCN40REV_LE(pi->pubpi.phy_rev, 2) && pi->u.pi_lcn40phy->phycrs_war_en)
	{
		phy_utils_write_phyreg(pi, LCN40PHY_SignalBlock_edet1, 0x5858);
		PHY_REG_MOD(pi, LCN40PHY, CFOblkConfig, signalblk_det_thresh_ofdm_40, 0x58);
	}

	wlc_lcn40phy_agc_reset(pi);
}

static void
wlc_lcn40phy_rev3_agc_tweaks(phy_info_t *pi)
{
	uint16 clip_gain_2g_20mhz[] = {50, 46, 40, 36, 31, 26, 17, 9, 4};
	uint16 clip_thresh_2g_20mhz[] = {10, 10, 10, 10, 10, 10, 20, 20, 10};
	uint16 clip_gain_2g_40mhz[] = {50, 46, 40, 36, 30, 26, 18, 10, 4};
	uint16 clip_thresh_2g_40mhz[] = {2, 10, 10, 20, 15, 8, 10, 15, 10};
	phytbl_info_t tab;
	uint16 val = 20;
	uint16 *clip_gains = NULL, *clip_threshs = NULL;
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);

	PHY_REG_LIST_START
		PHY_REG_MOD_ENTRY(LCN40PHY, ClipCtrThresh, ClipCtrThreshHiGain, 15)
		RADIO_REG_MOD_ENTRY(RADIO_2065_NBRSSI_BIAS, 0xff03, 0x303)
		RADIO_REG_WRITE_ENTRY(RADIO_2065_NBRSSI_IB, 0x36e2)
		RADIO_REG_MOD_ENTRY(RADIO_2065_NBRSSI_CONFG, 0x7000, 0x3000)
		PHY_REG_MOD_ENTRY(LCN40PHY, crsMiscCtrl0, matchFiltEn, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, crsMiscCtrl0_new, matchFiltEn40, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, radioTRCtrl, gainrequestTRAttnOnEn, 0)
		PHY_REG_MOD_ENTRY(LCN40PHY, readsym2resetCtrl, readsym2resetwaitlen, 110)
	PHY_REG_LIST_EXECUTE(pi);

	if (CHSPEC_IS20(pi->radio_chanspec)) {
		PHY_REG_MOD(pi, LCN40PHY, ClipDetector, Clip_detecotr_thresh, 40);
	}
	/* Tuning to avoid being stuck at BPHY_CONFIRM, an FSM bug */
	PHY_REG_MOD(pi, LCN40PHY, DSSSConfirmCnt, DSSSConfirmCntLoGain, 3);

	PHY_REG_MOD(pi, LCN40PHY, agcControl38, rssi_no_clip_gain_normal, 62);
	if (CHSPEC_IS40(pi->radio_chanspec)) {
		clip_gains = clip_gain_2g_40mhz;
		clip_threshs = clip_thresh_2g_40mhz;
		PHY_REG_WRITE(pi, LCN40PHY, ClipThresh, 0x2d);
		if (pi->sh->rx_antdiv <= ANT_RX_DIV_FORCE_1)
			PHY_REG_MOD(pi, LCN40PHY, agcControl11, gain_settle_dly_cnt, 3);
	} else {
		clip_gains = clip_gain_2g_20mhz;
		clip_threshs = clip_thresh_2g_20mhz;
		PHY_REG_WRITE(pi, LCN40PHY, ClipThresh, 0x3f);
		if (pi->sh->rx_antdiv <= ANT_RX_DIV_FORCE_1)
			PHY_REG_MOD(pi, LCN40PHY, agcControl11, gain_settle_dly_cnt, 5);
	}

	PHY_REG_LIST_START
		PHY_REG_MOD_ENTRY(LCN40PHY, agcControl27, data_mode_gain_db_adj_dsss, 0)

		RADIO_REG_WRITE_ENTRY(RADIO_2065_LNA2G_RSSI, 0x51f1)
		PHY_REG_MOD_ENTRY(LCN40PHY, agcControl23, rssi_clip_thresh_norm_8, 0)
		PHY_REG_MOD_ENTRY(LCN40PHY, agcControl24, rssi_no_clip_gain_adj, 0)
	PHY_REG_LIST_EXECUTE(pi);

	if (clip_gains) {
		phy_utils_mod_phyreg(pi, LCN40PHY_agcControl14,
			LCN40PHY_agcControl14_rssi_clip_gain_norm_0_MASK |
			LCN40PHY_agcControl14_rssi_clip_gain_norm_1_MASK,
			(clip_gains[0] << LCN40PHY_agcControl14_rssi_clip_gain_norm_0_SHIFT) |
		(clip_gains[1] << LCN40PHY_agcControl14_rssi_clip_gain_norm_1_SHIFT));
		phy_utils_mod_phyreg(pi, LCN40PHY_agcControl15,
			LCN40PHY_agcControl15_rssi_clip_gain_norm_2_MASK |
			LCN40PHY_agcControl15_rssi_clip_gain_norm_3_MASK,
			(clip_gains[2] << LCN40PHY_agcControl15_rssi_clip_gain_norm_2_SHIFT) |
			(clip_gains[3] << LCN40PHY_agcControl15_rssi_clip_gain_norm_3_SHIFT));
		phy_utils_mod_phyreg(pi, LCN40PHY_agcControl16,
			LCN40PHY_agcControl16_rssi_clip_gain_norm_4_MASK |
			LCN40PHY_agcControl16_rssi_clip_gain_norm_5_MASK,
			(clip_gains[4] << LCN40PHY_agcControl16_rssi_clip_gain_norm_4_SHIFT) |
			(clip_gains[5] << LCN40PHY_agcControl16_rssi_clip_gain_norm_5_SHIFT));
		phy_utils_mod_phyreg(pi, LCN40PHY_agcControl17,
			LCN40PHY_agcControl17_rssi_clip_gain_norm_6_MASK |
			LCN40PHY_agcControl17_rssi_clip_gain_norm_7_MASK,
			(clip_gains[6] << LCN40PHY_agcControl17_rssi_clip_gain_norm_6_SHIFT) |
			(clip_gains[7] << LCN40PHY_agcControl17_rssi_clip_gain_norm_7_SHIFT));
		PHY_REG_MOD(pi, LCN40PHY, agcControl18, rssi_clip_gain_norm_8, clip_gains[8]);
	}
	if (clip_threshs) {
		phy_utils_mod_phyreg(pi, LCN40PHY_agcControl19,
			LCN40PHY_agcControl19_rssi_clip_thresh_norm_0_MASK |
			LCN40PHY_agcControl19_rssi_clip_thresh_norm_1_MASK,
			(clip_threshs[0] << LCN40PHY_agcControl19_rssi_clip_thresh_norm_0_SHIFT) |
			(clip_threshs[1] << LCN40PHY_agcControl19_rssi_clip_thresh_norm_1_SHIFT));
		phy_utils_mod_phyreg(pi, LCN40PHY_agcControl20,
			LCN40PHY_agcControl20_rssi_clip_thresh_norm_2_MASK |
			LCN40PHY_agcControl20_rssi_clip_thresh_norm_3_MASK,
			(clip_threshs[2] << LCN40PHY_agcControl20_rssi_clip_thresh_norm_2_SHIFT) |
			(clip_threshs[3] << LCN40PHY_agcControl20_rssi_clip_thresh_norm_3_SHIFT));
		phy_utils_mod_phyreg(pi, LCN40PHY_agcControl21,
			LCN40PHY_agcControl21_rssi_clip_thresh_norm_4_MASK |
			LCN40PHY_agcControl21_rssi_clip_thresh_norm_5_MASK,
			(clip_threshs[4] << LCN40PHY_agcControl21_rssi_clip_thresh_norm_4_SHIFT) |
			(clip_threshs[5] << LCN40PHY_agcControl21_rssi_clip_thresh_norm_5_SHIFT));
		phy_utils_mod_phyreg(pi, LCN40PHY_agcControl22,
			LCN40PHY_agcControl22_rssi_clip_thres_norm_6_MASK |
			LCN40PHY_agcControl22_rssi_clip_thresh_norm_7_MASK,
			(clip_threshs[6] << LCN40PHY_agcControl22_rssi_clip_thres_norm_6_SHIFT) |
			(clip_threshs[7] << LCN40PHY_agcControl22_rssi_clip_thresh_norm_7_SHIFT));
		PHY_REG_MOD(pi, LCN40PHY, agcControl23, rssi_clip_thresh_norm_8, clip_threshs[8]);
	}

	PHY_REG_LIST_START
		/* LOCK LPF HPC at 3 */
		PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride7, lpf_hpc_ovr, 0x1)
		PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride7val, lpf_hpc_ovr_val, 3)

		PHY_REG_WRITE_ENTRY(LCN40PHY, SignalBlock_edet1, 0x6161)
		PHY_REG_MOD_ENTRY(LCN40PHY, CFOblkConfig, signalblk_det_thresh_ofdm_40, 0x61)
	PHY_REG_LIST_EXECUTE(pi);

	/* Not synch up with TCL, needs further investigation on actual cause */
	if (CHIPREV(pi->sh->chiprev) < 1)
		PHY_REG_MOD(pi, LCN40PHY, AfeCtrlOvr1, afe_reset_ov_det_ovr, 0x1);
	PHY_REG_MOD(pi, LCN40PHY, SignalBlock_edet1, signalblk_det_thresh_dsss,
		pi_lcn->lcnphy_aci.SignalBlock_edet1_dsss_Latch);

	tab.tbl_id = LCN40PHY_TBL_ID_RFSEQ;
	tab.tbl_width = 16;	/* 12 bit wide	*/
	tab.tbl_ptr = &val;
	tab.tbl_len = 1;
	tab.tbl_offset = 0x3c;
	wlc_lcnphy_write_table(pi,  &tab);

	wlc_lcn40phy_agc_reset(pi);
}

static void
wlc_lcn40phy_rev1_agc_tweaks(phy_info_t *pi)
{
	uint16 clip_gain_2g_20mhz[] = {61, 55, 49, 46, 39, 32, 28, 21, 11};
	uint16 clip_gain_2g_40mhz[] = {60, 54, 49, 44, 39, 32, 28, 21, 11};
	uint16 clip_thresh_2g_20mhz[] = {10, 10, 10, 10, 13, 13, 8, 10, 0};
	uint16 clip_thresh_2g_40mhz[] = {10, 10, 10, 11, 10, 10, 7, 10, 0};

	uint16 *clip_gains = NULL, *clip_threshs = NULL;

	PHY_REG_LIST_START
		PHY_REG_MOD_ENTRY(LCN40PHY, ClipCtrThresh, ClipCtrThreshHiGain, 30)
		RADIO_REG_MOD_ENTRY(RADIO_2065_NBRSSI_BIAS, 0xff03, 0x303)
		RADIO_REG_WRITE_ENTRY(RADIO_2065_NBRSSI_IB, 0x36e2)
		RADIO_REG_MOD_ENTRY(RADIO_2065_NBRSSI_CONFG, 0x7000, 0x3000)
		PHY_REG_MOD_ENTRY(LCN40PHY, crsMiscCtrl0, matchFiltEn, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, crsMiscCtrl0_new, matchFiltEn40, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, radioTRCtrl, gainrequestTRAttnOnEn, 0)
		/* Set gainsettle delay, works better for 4314iPA */
		PHY_REG_MOD_ENTRY(LCN40PHY, agcControl11, gain_settle_dly_cnt, 3)

		PHY_REG_MOD_ENTRY(LCN40PHY, agcControl38, rssi_no_clip_gain_normal, 64)
	PHY_REG_LIST_EXECUTE(pi);

	if (CHSPEC_IS40(pi->radio_chanspec)) {
		PHY_REG_MOD(pi, LCN40PHY, agcControl38, rssi_no_clip_gain_normal, 63);
		clip_gains = clip_gain_2g_40mhz;
		clip_threshs = clip_thresh_2g_40mhz;
	} else {
		clip_gains = clip_gain_2g_20mhz;
		clip_threshs = clip_thresh_2g_20mhz;
	}

	PHY_REG_LIST_START
		RADIO_REG_WRITE_ENTRY(RADIO_2065_LNA2G_RSSI, 0x51c1)
		PHY_REG_MOD_ENTRY(LCN40PHY, agcControl23, rssi_clip_thresh_norm_8, 0)
		PHY_REG_MOD_ENTRY(LCN40PHY, agcControl24, rssi_no_clip_gain_adj, -7)
	PHY_REG_LIST_EXECUTE(pi);

	if (clip_gains) {
		phy_utils_mod_phyreg(pi, LCN40PHY_agcControl14,
			LCN40PHY_agcControl14_rssi_clip_gain_norm_0_MASK |
			LCN40PHY_agcControl14_rssi_clip_gain_norm_1_MASK,
			(clip_gains[0] << LCN40PHY_agcControl14_rssi_clip_gain_norm_0_SHIFT) |
		(clip_gains[1] << LCN40PHY_agcControl14_rssi_clip_gain_norm_1_SHIFT));
		phy_utils_mod_phyreg(pi, LCN40PHY_agcControl15,
			LCN40PHY_agcControl15_rssi_clip_gain_norm_2_MASK |
			LCN40PHY_agcControl15_rssi_clip_gain_norm_3_MASK,
			(clip_gains[2] << LCN40PHY_agcControl15_rssi_clip_gain_norm_2_SHIFT) |
			(clip_gains[3] << LCN40PHY_agcControl15_rssi_clip_gain_norm_3_SHIFT));
		phy_utils_mod_phyreg(pi, LCN40PHY_agcControl16,
			LCN40PHY_agcControl16_rssi_clip_gain_norm_4_MASK |
			LCN40PHY_agcControl16_rssi_clip_gain_norm_5_MASK,
			(clip_gains[4] << LCN40PHY_agcControl16_rssi_clip_gain_norm_4_SHIFT) |
			(clip_gains[5] << LCN40PHY_agcControl16_rssi_clip_gain_norm_5_SHIFT));
		phy_utils_mod_phyreg(pi, LCN40PHY_agcControl17,
			LCN40PHY_agcControl17_rssi_clip_gain_norm_6_MASK |
			LCN40PHY_agcControl17_rssi_clip_gain_norm_7_MASK,
			(clip_gains[6] << LCN40PHY_agcControl17_rssi_clip_gain_norm_6_SHIFT) |
			(clip_gains[7] << LCN40PHY_agcControl17_rssi_clip_gain_norm_7_SHIFT));
		PHY_REG_MOD(pi, LCN40PHY, agcControl18, rssi_clip_gain_norm_8, clip_gains[8]);
	}
	if (clip_threshs) {
		phy_utils_mod_phyreg(pi, LCN40PHY_agcControl19,
			LCN40PHY_agcControl19_rssi_clip_thresh_norm_0_MASK |
			LCN40PHY_agcControl19_rssi_clip_thresh_norm_1_MASK,
			(clip_threshs[0] << LCN40PHY_agcControl19_rssi_clip_thresh_norm_0_SHIFT) |
			(clip_threshs[1] << LCN40PHY_agcControl19_rssi_clip_thresh_norm_1_SHIFT));
		phy_utils_mod_phyreg(pi, LCN40PHY_agcControl20,
			LCN40PHY_agcControl20_rssi_clip_thresh_norm_2_MASK |
			LCN40PHY_agcControl20_rssi_clip_thresh_norm_3_MASK,
			(clip_threshs[2] << LCN40PHY_agcControl20_rssi_clip_thresh_norm_2_SHIFT) |
			(clip_threshs[3] << LCN40PHY_agcControl20_rssi_clip_thresh_norm_3_SHIFT));
		phy_utils_mod_phyreg(pi, LCN40PHY_agcControl21,
			LCN40PHY_agcControl21_rssi_clip_thresh_norm_4_MASK |
			LCN40PHY_agcControl21_rssi_clip_thresh_norm_5_MASK,
			(clip_threshs[4] << LCN40PHY_agcControl21_rssi_clip_thresh_norm_4_SHIFT) |
			(clip_threshs[5] << LCN40PHY_agcControl21_rssi_clip_thresh_norm_5_SHIFT));
		phy_utils_mod_phyreg(pi, LCN40PHY_agcControl22,
			LCN40PHY_agcControl22_rssi_clip_thres_norm_6_MASK |
			LCN40PHY_agcControl22_rssi_clip_thresh_norm_7_MASK,
			(clip_threshs[6] << LCN40PHY_agcControl22_rssi_clip_thres_norm_6_SHIFT) |
			(clip_threshs[7] << LCN40PHY_agcControl22_rssi_clip_thresh_norm_7_SHIFT));
		PHY_REG_MOD(pi, LCN40PHY, agcControl23, rssi_clip_thresh_norm_8, clip_threshs[8]);
	}

	PHY_REG_LIST_START
		PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride7, lpf_hpc_ovr, 0x0)
		PHY_REG_MOD_ENTRY(LCN40PHY, agcControl4, dc_bw_lock_en, 0)
		PHY_REG_MOD_ENTRY(LCN40PHY, RxRadioControlFltrState, rx_flt_low_start_state, 0)
		PHY_REG_MOD_ENTRY(LCN40PHY, RxRadioControlFltrState, rx_flt_high_start_state, 1)

		PHY_REG_WRITE_ENTRY(LCN40PHY, SignalBlock_edet1, 0x4747)
		PHY_REG_MOD_ENTRY(LCN40PHY, CFOblkConfig, signalblk_det_thresh_ofdm_40, 0x61)
	PHY_REG_LIST_EXECUTE(pi);

	wlc_lcn40phy_agc_reset(pi);
}

static void
wlc_lcn40phy_rev4_agc_tweaks(phy_info_t *pi)
{
	uint16 clip_gain_2g[] = {50, 45, 40, 40, 40, 40, 34, 30, 25, 22, 14, 11, 61};
	uint16 clip_thresh_2g[] = {20, 20, 20, 0, 0, 0, 10, 10, 10, 10, 10, 10};
	uint16 clip_gain_2g_extlna[] = {58, 55, 55, 52, 49, 43, 40, 34, 28, 23, 17, 7, 61};
	uint16 clip_gain_2g_extlna_rev5[] = {61, 58, 58, 55, 52, 46, 43, 37, 31, 26, 17, 8, 61};
	uint16 clip_thresh_2g_extlna[] = {30, 30, 0, 20, 20, 20, 20, 20, 10, 10, 10, 10};
	uint16 clip_gain_2g_acimode[] = {43, 36, 33, 33, 30, 27, 24, 18, 15, 10, 7, 4, 55};
	uint16 clip_thresh_2g_acimode[] = {20, 20, 20, 0, 10, 10, 10, 10, 10, 10, 10, 10};
	uint16 clip_gain_2g_extlna_fstr[] = {61, 58, 58, 55, 52, 46, 43, 37, 31, 26, 20, 10, 64};

#ifdef BAND5G
	uint16 clip_gain_5g[] = {58, 50, 40, 40, 40, 37, 32, 28, 25, 22, 16, 10, 62};
	uint16 clip_thresh_5g[] = {20, 20, 20, 0, 0, 30, 20, 20, 10, 10, 10, 10};
	uint16 clip_gain_5g_extlna[] = {50, 47, 43, 43, 43, 43, 38, 32, 28, 25, 20, 10, 58};
	uint16 clip_thresh_5g_extlna[] = {20, 20, 20, 0, 0, 0, 10, 10, 10, 10, 10, 10};
	uint16 clip_gain_5g_extlna_fstr[] = {55, 48, 41, 41, 41, 41, 36, 32, 28, 25, 20, 10, 55};
#endif /* #ifdef BAND5G */
	uint16 *clip_gains = NULL, *clip_threshs = NULL;
	const uint16 *clip_gains_acimode = NULL, *clip_threshs_acimode = NULL;
	bool hpc_sequencer_enable = FALSE, tia_dc_loop_enable = FALSE;

	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	phy_info_lcn40phy_t *pi_lcn40 = pi->u.pi_lcn40phy;

	if (pi->fabid == LCN40PHY_UMC_FABID) {
#ifdef BAND5G
		if (CHSPEC_IS5G(pi->radio_chanspec)) {
			clip_gain_5g_extlna[7] = 28;
			clip_gain_5g_extlna[8] = 25;
			clip_gain_5g_extlna[9] = 22;
			if (LCN40REV_IS(pi->pubpi.phy_rev, 5)) {
				clip_gain_5g_extlna[0] = 53;
				clip_gain_5g_extlna[1] = 48;
				clip_gain_5g_extlna[2] = 41;
				clip_gain_5g_extlna[3] = 41;
				clip_gain_5g_extlna[4] = 41;
				clip_gain_5g_extlna[5] = 41;
			}
		}
#endif // endif
	}

	PHY_REG_LIST_START
		PHY_REG_MOD_ENTRY(LCN40PHY, ClipCtrThresh, ClipCtrThreshHiGain, 15)
		/* enable clamp */
		PHY_REG_MOD_ENTRY(LCN40PHY, AfeCtrlOvr1, afe_clamp_en_ovr, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, AfeCtrlOvr1Val, afe_clamp_en_ovr_val, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, AfeCtrlOvr1, afe_reset_ov_det_ovr, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, AfeCtrlOvr1Val, afe_reset_ov_det_ovr_val, 0)

		/* NB RSSI settings */
		RADIO_REG_MOD_ENTRY(RADIO_2065_NBRSSI_BIAS, 0xff03, 0x303)
		RADIO_REG_WRITE_ENTRY(RADIO_2065_NBRSSI_IB, 0x36e2)
		RADIO_REG_MOD_ENTRY(RADIO_2065_NBRSSI_CONFG, 0x73c0, 0x31c0)

		/* W3 RSSI settings */
		RADIO_REG_MOD_ENTRY(RADIO_2065_WRSSI3_BIAS, 0x00ff, 0x3)
		RADIO_REG_MOD_ENTRY(RADIO_2065_WRSSI3_CONFG, 0xe3c0, 0x6340)

		PHY_REG_MOD_ENTRY(LCN40PHY, radioTRCtrl, gainrequestTRAttnOnEn, 0)

		/* Tuning to avoid being stuck at BPHY_CONFIRM, an FSM bug */
		PHY_REG_MOD_ENTRY(LCN40PHY, DSSSConfirmCnt, DSSSConfirmCntLoGain, 3)

		/*  turn off clip backoff */
		PHY_REG_MOD_ENTRY(LCN40PHY, agcControl48, repeat_clip_backoff, 0)

		/* turn on aci detector */
		PHY_REG_MOD_ENTRY(LCN40PHY, aciContro9, aci_detect_aci_det_en, 0)
		PHY_REG_MOD_ENTRY(LCN40PHY, aciContro1, aci_detect_grp_wp_nor, 0x1)
		PHY_REG_MOD_ENTRY(LCN40PHY, aciContro2, aci_detect_grp_wp_aci, 0xff)
		PHY_REG_MOD_ENTRY(LCN40PHY, aciContro3, aci_detect_grp_ws_nor, 15)
		PHY_REG_MOD_ENTRY(LCN40PHY, aciContro4, aci_detect_grp_ws_aci, 15)
		PHY_REG_MOD_ENTRY(LCN40PHY, aciContro9, aci_detect_grp_ci_nor, 2)
		PHY_REG_MOD_ENTRY(LCN40PHY, aciContro9, aci_detect_grp_ci_aci, 15)
		PHY_REG_MOD_ENTRY(LCN40PHY, aciContro5, aci_detect_grp_et_nor, 1000)
		PHY_REG_MOD_ENTRY(LCN40PHY, aciContro6, aci_detect_grp_et_aci, 20)
		PHY_REG_MOD_ENTRY(LCN40PHY, aciContro7, aci_detect_grp_dt_nor, 100)
		PHY_REG_MOD_ENTRY(LCN40PHY, aciContro8, aci_detect_grp_dt_aci, 20)
		PHY_REG_MOD_ENTRY(LCN40PHY, aciContro9, aci_detect_aci_det_force, 0)

		PHY_REG_MOD_ENTRY(LCN40PHY, sslpnRxFeClkEnCtrl, allowCrsClkTxDisable, 1)
	PHY_REG_LIST_EXECUTE(pi);

	/*  turn on the reset for ACI detector */
	if (LCN40REV_IS(pi->pubpi.phy_rev, 5))
		phy_utils_write_phyreg(pi, LCN40PHY_workAround,
		(phy_utils_read_phyreg(pi, LCN40PHY_workAround) | 0x8000));

	PHY_REG_MOD(pi, LCN40PHY, agcControl9, lo_gain_max_ofdm_mismatch, 30);

	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		/* gain settle delay */
		PHY_REG_MOD(pi, LCN40PHY, agcControl11, gain_settle_dly_cnt,
			pi_lcn40->gain_settle_dly_2g);
		PHY_REG_MOD(pi, LCN40PHY, bphyTest, shiftBDigiGain, 1);
		if (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA) {
			if (pi_lcn40->elna_off_gain_idx_2g != 0xFF)
				PHY_REG_MOD(pi, LCN40PHY, radioTRCtrlCrs1, trGainThresh,
					pi_lcn40->elna_off_gain_idx_2g);
			else if (pi_lcn40->trGain != 0xFF)
				PHY_REG_MOD(pi, LCN40PHY, radioTRCtrlCrs1, trGainThresh,
					pi_lcn40->trGain);
			else
				PHY_REG_MOD(pi, LCN40PHY, radioTRCtrlCrs1, trGainThresh,
				30);
			PHY_REG_MOD(pi, LCN40PHY, radioTRCtrl, gainrequestTRAttnOnEn, 1);
			PHY_REG_MOD(pi, LCN40PHY, radioTRCtrl, gainrequestTRAttnOnOffset,
				pi_lcn->lcnphy_tr_isolation_mid);
			if (CHSPEC_IS40(pi->radio_chanspec))
				PHY_REG_WRITE(pi, LCN40PHY, ClipThresh, 55);
			else
				PHY_REG_WRITE(pi, LCN40PHY, ClipThresh,
				pi_lcn40->clipthr_eLNA2g);
			if (pi_lcn40->fstr_flag == 1)
				clip_gains = clip_gain_2g_extlna_fstr;
			else {
				if (LCN40REV_IS(pi->pubpi.phy_rev, 5))
					clip_gains = clip_gain_2g_extlna_rev5;
				else
					clip_gains = clip_gain_2g_extlna;
			}
			clip_threshs = clip_thresh_2g_extlna;
			if (LCN40REV_IS(pi->pubpi.phy_rev, 5)) {
				clip_gains_acimode = clip_gain_2g_acimode_extlna_rev5;
				clip_threshs_acimode = clip_thresh_2g_acimode_extlna_rev5;
			}
			else {
				clip_gains_acimode = clip_gain_2g_acimode_extlna;
				clip_threshs_acimode = clip_thresh_2g_acimode_extlna;
			}
			PHY_REG_LIST_START
				PHY_REG_MOD_ENTRY(LCN40PHY, SignalBlockConfigTable2_new,
					crssignalblk_noi_pwr, 87)
				PHY_REG_MOD_ENTRY(LCN40PHY, SignalBlockConfigTable2_new,
					crssignalblk_noi_pwr_40mhz, 93)
				PHY_REG_MOD_ENTRY(LCN40PHY, agcControl27,
					data_mode_gain_db_adj_dsss, 0)
			PHY_REG_LIST_EXECUTE(pi);
			if (LCN40REV_IS(pi->pubpi.phy_rev, 5))
				PHY_REG_MOD(pi, LCN40PHY, agcControl24,
				rssi_no_clip_gain_mismatch, -8);
		} else {
			clip_gains = clip_gain_2g;
			clip_threshs = clip_thresh_2g;
			clip_gains_acimode = clip_gain_2g_acimode;
			clip_threshs_acimode = clip_thresh_2g_acimode;
			if (CHSPEC_IS40(pi->radio_chanspec))
				PHY_REG_WRITE(pi, LCN40PHY, ClipThresh, 55);
			else
				PHY_REG_WRITE(pi, LCN40PHY, ClipThresh, 70);
			PHY_REG_LIST_START
				PHY_REG_MOD_ENTRY(LCN40PHY, SignalBlockConfigTable2_new,
					crssignalblk_noi_pwr, 83)
				PHY_REG_MOD_ENTRY(LCN40PHY, SignalBlockConfigTable2_new,
					crssignalblk_noi_pwr_40mhz, 89)
				PHY_REG_MOD_ENTRY(LCN40PHY, agcControl27,
					data_mode_gain_db_adj_dsss, 1)
				PHY_REG_MOD_ENTRY(LCN40PHY, agcControl24,
					rssi_no_clip_gain_mismatch, 0)
			PHY_REG_LIST_EXECUTE(pi);
		}

		phy_utils_write_radioreg(pi, RADIO_2065_LNA2G_RSSI, 0x51f1);
		PHY_REG_MOD(pi, LCN40PHY, SignalBlock_edet1, signalblk_det_thresh_dsss,
			pi_lcn40->dsss_thresh);

		PHY_REG_LIST_START
			/* register tweaks to alleviate the bphy-stuck bug */
			PHY_REG_MOD_ENTRY(LCN40PHY, Bphycntctrl, claim_cnt, 1)
			PHY_REG_MOD_ENTRY(LCN40PHY, WaitforPHYSelTimeout,
				BPHYWaitforCCKSeltimeout, 15)
			PHY_REG_MOD_ENTRY(LCN40PHY, DSSSConfirmCnt, DSSSConfirmCntLoGain, 4)
			PHY_REG_MOD_ENTRY(LCN40PHY, DSSSConfirmCnt, DSSSConfirmCntHiGain, 5)
		PHY_REG_LIST_EXECUTE(pi);
		if (LCN40REV_IS(pi->pubpi.phy_rev, 5)) {
			phy_utils_write_phyreg(pi, LCN40PHY_workAround,
			(phy_utils_read_phyreg(pi, LCN40PHY_workAround) | 0x1));
		}
		/* post-aci-filter power calculation */
		PHY_REG_MOD(pi, LCN40PHY, crsMiscCtrl0, usePreFiltPwr, 0);

		/* aci detector */
		if (LCN40REV_IS(pi->pubpi.phy_rev, 4)) {
			if (pi_lcn40->aci_detect_en_2g != -1) {
				PHY_REG_MOD(pi, LCN40PHY, aciContro9, aci_detect_aci_det_en,
					pi_lcn40->aci_detect_en_2g);
				PHY_REG_MOD(pi, LCN40PHY, sslpnRxFeClkEnCtrl,
					allowCrsClkTxDisable, pi_lcn40->tx_agc_reset_2g);
			} else {
				PHY_REG_MOD(pi, LCN40PHY, aciContro9, aci_detect_aci_det_en, 0);
				if (pi_lcn40->gaintbl_force_2g != -1)
					PHY_REG_MOD(pi, LCN40PHY, aciContro9,
					aci_detect_aci_det_force, pi_lcn40->gaintbl_force_2g);
			}
		} else if (LCN40REV_IS(pi->pubpi.phy_rev, 5)) {
			if (pi_lcn40->aci_detect_en_2g != -1)
				PHY_REG_MOD(pi, LCN40PHY, aciContro9, aci_detect_aci_det_en,
				pi_lcn40->aci_detect_en_2g);
			if (pi_lcn40->gaintbl_force_2g != -1) {
				PHY_REG_MOD(pi, LCN40PHY, aciContro9, aci_detect_aci_det_force,
				1);
				if (pi_lcn40->gaintbl_force_2g == 1)
					phy_utils_write_phyreg(pi, LCN40PHY_workAround,
					(phy_utils_read_phyreg(pi, LCN40PHY_workAround) | 0x8));
				else
					phy_utils_write_phyreg(pi, LCN40PHY_workAround,
					(phy_utils_read_phyreg(pi, LCN40PHY_workAround) & 0xfff7));
			}
		}

		tia_dc_loop_enable = pi_lcn40->tia_dc_loop_enable_2g;
		hpc_sequencer_enable = pi_lcn40->hpc_sequencer_enable_2g;
	}
#ifdef BAND5G
	else {
		/* gain settle delay */
		PHY_REG_MOD(pi, LCN40PHY, agcControl11, gain_settle_dly_cnt,
			pi_lcn40->gain_settle_dly_5g);

		if (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA_5GHz) {
			if (pi_lcn40->fstr_flag == 1)
				clip_gains = clip_gain_5g_extlna_fstr;
			else
				clip_gains = clip_gain_5g_extlna;
			clip_threshs = clip_thresh_5g_extlna;
			PHY_REG_MOD(pi, LCN40PHY, radioTRCtrl, gainrequestTRAttnOnEn, 1);
			if (pi_lcn40->elna_off_gain_idx_5g != 0xFF)
				PHY_REG_MOD(pi, LCN40PHY, radioTRCtrlCrs1, trGainThresh,
				pi_lcn40->elna_off_gain_idx_5g);
			else
				PHY_REG_MOD(pi, LCN40PHY, radioTRCtrlCrs1, trGainThresh, 30);
			if (pi_lcn->triso5g[0] != 0xff)
				PHY_REG_MOD(pi, LCN40PHY, radioTRCtrl, gainrequestTRAttnOnOffset,
					pi_lcn->triso5g[0]);
			PHY_REG_MOD(pi, LCN40PHY, agcControl24, rssi_no_clip_gain_adj, -3);
			phy_utils_write_radioreg(pi, RADIO_2065_LNA5G_RSSI, 0x71f1);
		} else {
			clip_gains = clip_gain_5g;
			clip_threshs = clip_thresh_5g;
			phy_utils_write_radioreg(pi, RADIO_2065_LNA5G_RSSI, 0x51d1);
		}

		PHY_REG_LIST_START
			PHY_REG_MOD_ENTRY(LCN40PHY, clipCtrThreshLowGainEx,
				clip_counter_threshold_high_gain_40mhz, 30)
			PHY_REG_MOD_ENTRY(LCN40PHY, rxfe, bypass_iqcomp, 0)

			PHY_REG_MOD_ENTRY(LCN40PHY, SignalBlock_edet1,
				signalblk_det_thresh_dsss, 19)
		PHY_REG_LIST_EXECUTE(pi);

		/* aci detector */
		if (LCN40REV_IS(pi->pubpi.phy_rev, 4)) {
			if (pi_lcn40->aci_detect_en_5g != -1) {
				PHY_REG_MOD(pi, LCN40PHY, aciContro9, aci_detect_aci_det_en,
					pi_lcn40->aci_detect_en_5g);
				PHY_REG_MOD(pi, LCN40PHY, sslpnRxFeClkEnCtrl,
					allowCrsClkTxDisable, pi_lcn40->tx_agc_reset_5g);
			} else {
				PHY_REG_MOD(pi, LCN40PHY, aciContro9, aci_detect_aci_det_en, 0);
				if (pi_lcn40->gaintbl_force_5g != -1)
					PHY_REG_MOD(pi, LCN40PHY, aciContro9,
					aci_detect_aci_det_force, pi_lcn40->gaintbl_force_5g);
			}
		} else if (LCN40REV_IS(pi->pubpi.phy_rev, 5)) {
			if (pi_lcn40->aci_detect_en_5g != -1)
				PHY_REG_MOD(pi, LCN40PHY, aciContro9, aci_detect_aci_det_en,
					pi_lcn40->aci_detect_en_5g);
			if (pi_lcn40->gaintbl_force_5g != -1) {
				PHY_REG_MOD(pi, LCN40PHY, aciContro9, aci_detect_aci_det_force,
					1);
				if (pi_lcn40->gaintbl_force_5g == 1)
					phy_utils_write_phyreg(pi, LCN40PHY_workAround,
					(phy_utils_read_phyreg(pi, LCN40PHY_workAround) | 0x8));
				else
					phy_utils_write_phyreg(pi, LCN40PHY_workAround,
					(phy_utils_read_phyreg(pi, LCN40PHY_workAround) & 0xfff7));
			}
		}

		tia_dc_loop_enable = pi_lcn40->tia_dc_loop_enable_5g;
		hpc_sequencer_enable = pi_lcn40->hpc_sequencer_enable_5g;
	}
#endif  /* BAND5G */

	/* Set TIA DC Loop */
	if (tia_dc_loop_enable) {
		PHY_REG_LIST_START
			PHY_REG_MOD_ENTRY(LCN40PHY, radio_rx_cfg_pu, rx_tia_dc_byp, 0)
			PHY_REG_MOD_ENTRY(LCN40PHY, radio_rx_cfg_pu, rx_tia_dc_pu, 1)
			RADIO_REG_MOD_ENTRY(RADIO_2065_TIA_CFG2, 0x3000, 0x1000)
		PHY_REG_LIST_EXECUTE(pi);
	} else {
		PHY_REG_MOD(pi, LCN40PHY, radio_rx_cfg_pu, rx_tia_dc_byp, 1);
		PHY_REG_MOD(pi, LCN40PHY, radio_rx_cfg_pu, rx_tia_dc_pu, 0);
	}

	/* Set HPC Sequencer */
	if (hpc_sequencer_enable) {
		phytbl_info_t tab;

		PHY_REG_LIST_START
			PHY_REG_MOD_ENTRY(LCN40PHY, agcControl4, dc_bw_lock_en, 0)
			PHY_REG_MOD_ENTRY(LCN40PHY, RxRadioControlFltrState,
				rx_flt_high_start_state, 2)
			PHY_REG_MOD_ENTRY(LCN40PHY, RxRadioControlFltrState,
				rx_flt_low_start_state, 1)
			PHY_REG_MOD_ENTRY(LCN40PHY, low_pwr_rf_cfg_1, rx_flt_wake_start_state, 1)
			PHY_REG_MOD_ENTRY(LCN40PHY, low_pwr_rf_cfg_2, rx_flt_nap_start_state, 1)
			PHY_REG_MOD_ENTRY(LCN40PHY, gainMismatch, GainMismatchHigain, -8)
			PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride7, lpf_hpc_ovr, 0)
			PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride7, tia_hpc_ovr, 0)
		PHY_REG_LIST_EXECUTE(pi);

		tab.tbl_id = LCN40PHY_TBL_ID_FLTR_CTRL;
		tab.tbl_width = 32;
		tab.tbl_offset = 0;

		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			tab.tbl_ptr = dot11lcn40_fltr_ctrl_2G_tbl_rev4;
			if ((pi_lcn40->fstr_flag == 1) || (pi_lcn40->cdd_mod == 1)) {
				dot11lcn40_fltr_ctrl_2G_tbl_rev4[2] = 0x000051d9;
				dot11lcn40_fltr_ctrl_2G_tbl_rev4[3] = 0x000051d8;
				dot11lcn40_fltr_ctrl_2G_tbl_rev4[4] = 0x0001fed8;
			} else if (pi_lcn40->new_lpf_rccal) {
				dot11lcn40_fltr_ctrl_2G_tbl_rev4[3] = 0x0001fedb;
			}
			tab.tbl_len = ARRAYSIZE(dot11lcn40_fltr_ctrl_2G_tbl_rev4);
			wlc_lcn40phy_write_table(pi, &tab);
		}
#ifdef BAND5G
		else {
			if ((pi_lcn40->fstr_flag == 1)||(pi_lcn40->cdd_mod == 1))
				tab.tbl_ptr = dot11lcn40_fltr_ctrl_5G_tbl_rev4_fstr;
			else if (pi_lcn40->new_lpf_rccal) {
				dot11lcn40_fltr_ctrl_5G_tbl_rev4[4] = 0x0001fedb;
				tab.tbl_ptr = dot11lcn40_fltr_ctrl_5G_tbl_rev4;
			}
			else
				tab.tbl_ptr = dot11lcn40_fltr_ctrl_5G_tbl_rev4;
			tab.tbl_len = ARRAYSIZE(dot11lcn40_fltr_ctrl_5G_tbl_rev4);
			wlc_lcn40phy_write_table(pi, &tab);
		}
#endif /* #ifdef BAND5G */
	} else {
		PHY_REG_LIST_START
			PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride2val, lpf_lq_ovr_val, 0)
			PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride2, lpf_lq_ovr, 1)
			PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride7val, lpf_hpc_ovr_val, 3)
			PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride7, lpf_hpc_ovr, 1)
			PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride7val, tia_hpc_ovr_val, 3)
			PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride7, tia_hpc_ovr, 1)
		PHY_REG_LIST_EXECUTE(pi);
	}

	if (clip_gains) {
		phy_utils_mod_phyreg(pi, LCN40PHY_agcControl14,
			LCN40PHY_agcControl14_rssi_clip_gain_norm_0_MASK |
			LCN40PHY_agcControl14_rssi_clip_gain_norm_1_MASK,
			(clip_gains[0] << LCN40PHY_agcControl14_rssi_clip_gain_norm_0_SHIFT) |
		(clip_gains[1] << LCN40PHY_agcControl14_rssi_clip_gain_norm_1_SHIFT));
		phy_utils_mod_phyreg(pi, LCN40PHY_agcControl15,
			LCN40PHY_agcControl15_rssi_clip_gain_norm_2_MASK |
			LCN40PHY_agcControl15_rssi_clip_gain_norm_3_MASK,
			(clip_gains[2] << LCN40PHY_agcControl15_rssi_clip_gain_norm_2_SHIFT) |
			(clip_gains[3] << LCN40PHY_agcControl15_rssi_clip_gain_norm_3_SHIFT));
		phy_utils_mod_phyreg(pi, LCN40PHY_agcControl16,
			LCN40PHY_agcControl16_rssi_clip_gain_norm_4_MASK |
			LCN40PHY_agcControl16_rssi_clip_gain_norm_5_MASK,
			(clip_gains[4] << LCN40PHY_agcControl16_rssi_clip_gain_norm_4_SHIFT) |
			(clip_gains[5] << LCN40PHY_agcControl16_rssi_clip_gain_norm_5_SHIFT));
		phy_utils_mod_phyreg(pi, LCN40PHY_agcControl17,
			LCN40PHY_agcControl17_rssi_clip_gain_norm_6_MASK |
			LCN40PHY_agcControl17_rssi_clip_gain_norm_7_MASK,
			(clip_gains[6] << LCN40PHY_agcControl17_rssi_clip_gain_norm_6_SHIFT) |
			(clip_gains[7] << LCN40PHY_agcControl17_rssi_clip_gain_norm_7_SHIFT));
		PHY_REG_MOD(pi, LCN40PHY, agcControl18, rssi_clip_gain_norm_8, clip_gains[8]);
		phy_utils_mod_phyreg(pi, LCN40PHY_agcContro245,
			LCN40PHY_agcContro245_rssi_clip_gain_norm_9_MASK |
			LCN40PHY_agcContro245_rssi_clip_gain_norm_10_MASK,
			(clip_gains[9] << LCN40PHY_agcContro245_rssi_clip_gain_norm_9_SHIFT) |
			(clip_gains[10] << LCN40PHY_agcContro245_rssi_clip_gain_norm_10_SHIFT));
		PHY_REG_MOD(pi, LCN40PHY, agcContro246, rssi_clip_gain_norm_11, clip_gains[11]);
		PHY_REG_MOD(pi, LCN40PHY, agcControl38, rssi_no_clip_gain_normal, clip_gains[12]);
	}

	if (clip_threshs) {
		phy_utils_mod_phyreg(pi, LCN40PHY_agcControl19,
			LCN40PHY_agcControl19_rssi_clip_thresh_norm_0_MASK |
			LCN40PHY_agcControl19_rssi_clip_thresh_norm_1_MASK,
			(clip_threshs[0] << LCN40PHY_agcControl19_rssi_clip_thresh_norm_0_SHIFT) |
			(clip_threshs[1] << LCN40PHY_agcControl19_rssi_clip_thresh_norm_1_SHIFT));
		phy_utils_mod_phyreg(pi, LCN40PHY_agcControl20,
			LCN40PHY_agcControl20_rssi_clip_thresh_norm_2_MASK |
			LCN40PHY_agcControl20_rssi_clip_thresh_norm_3_MASK,
			(clip_threshs[2] << LCN40PHY_agcControl20_rssi_clip_thresh_norm_2_SHIFT) |
			(clip_threshs[3] << LCN40PHY_agcControl20_rssi_clip_thresh_norm_3_SHIFT));
		phy_utils_mod_phyreg(pi, LCN40PHY_agcControl21,
			LCN40PHY_agcControl21_rssi_clip_thresh_norm_4_MASK |
			LCN40PHY_agcControl21_rssi_clip_thresh_norm_5_MASK,
			(clip_threshs[4] << LCN40PHY_agcControl21_rssi_clip_thresh_norm_4_SHIFT) |
			(clip_threshs[5] << LCN40PHY_agcControl21_rssi_clip_thresh_norm_5_SHIFT));
		phy_utils_mod_phyreg(pi, LCN40PHY_agcControl22,
			LCN40PHY_agcControl22_rssi_clip_thres_norm_6_MASK |
			LCN40PHY_agcControl22_rssi_clip_thresh_norm_7_MASK,
			(clip_threshs[6] << LCN40PHY_agcControl22_rssi_clip_thres_norm_6_SHIFT) |
			(clip_threshs[7] << LCN40PHY_agcControl22_rssi_clip_thresh_norm_7_SHIFT));
		phy_utils_mod_phyreg(pi, LCN40PHY_agcControl23,
			LCN40PHY_agcControl23_rssi_clip_thresh_norm_8_MASK |
			LCN40PHY_agcControl23_rssi_clip_thresh_norm_9_MASK,
			(clip_threshs[8] << LCN40PHY_agcControl23_rssi_clip_thresh_norm_8_SHIFT) |
			(clip_threshs[9] << LCN40PHY_agcControl23_rssi_clip_thresh_norm_9_SHIFT));
		phy_utils_mod_phyreg(pi, LCN40PHY_agcContro240,
			LCN40PHY_agcContro240_rssi_clip_thresh_norm_10_MASK |
			LCN40PHY_agcContro240_rssi_clip_thresh_norm_11_MASK,
			(clip_threshs[10] << LCN40PHY_agcContro240_rssi_clip_thresh_norm_10_SHIFT) |
			(clip_threshs[11] << LCN40PHY_agcContro240_rssi_clip_thresh_norm_11_SHIFT));
	}

	/* ==========================================
	 * ACI mode
	 * ==========================================
	 */

	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		/* Clip gains */
		phy_utils_mod_phyreg(pi, LCN40PHY_agcControl33,
			LCN40PHY_agcControl33_rssi_clip_gain_aci_0_MASK |
			LCN40PHY_agcControl33_rssi_clip_gain_aci_1_MASK,
			(clip_gains_acimode[0]
			<< LCN40PHY_agcControl33_rssi_clip_gain_aci_0_SHIFT) |
			(clip_gains_acimode[1]
			<< LCN40PHY_agcControl33_rssi_clip_gain_aci_1_SHIFT));
		phy_utils_mod_phyreg(pi, LCN40PHY_agcControl34,
			LCN40PHY_agcControl34_rssi_clip_gain_aci_2_MASK |
			LCN40PHY_agcControl34_rssi_clip_gain_aci_3_MASK,
			(clip_gains_acimode[2]
			<< LCN40PHY_agcControl34_rssi_clip_gain_aci_2_SHIFT) |
			(clip_gains_acimode[3]
			<< LCN40PHY_agcControl34_rssi_clip_gain_aci_3_SHIFT));
		phy_utils_mod_phyreg(pi, LCN40PHY_agcControl35,
			LCN40PHY_agcControl35_rssi_clip_gain_aci_4_MASK |
			LCN40PHY_agcControl35_rssi_clip_gain_aci_5_MASK,
			(clip_gains_acimode[4]
			<< LCN40PHY_agcControl35_rssi_clip_gain_aci_4_SHIFT) |
			(clip_gains_acimode[5]
			<< LCN40PHY_agcControl35_rssi_clip_gain_aci_5_SHIFT));
		phy_utils_mod_phyreg(pi, LCN40PHY_agcControl36,
			LCN40PHY_agcControl36_rssi_clip_gain_aci_6_MASK |
			LCN40PHY_agcControl36_rssi_clip_gain_aci_7_MASK,
			(clip_gains_acimode[6]
			<< LCN40PHY_agcControl36_rssi_clip_gain_aci_6_SHIFT) |
			(clip_gains_acimode[7]
			<< LCN40PHY_agcControl36_rssi_clip_gain_aci_7_SHIFT));
		PHY_REG_MOD(pi, LCN40PHY, agcControl37,
			rssi_clip_gain_aci_8, clip_gains_acimode[8]);
		phy_utils_mod_phyreg(pi, LCN40PHY_agcControl238,
			LCN40PHY_agcControl238_rssi_clip_gain_aci_9_MASK |
			LCN40PHY_agcControl238_rssi_clip_gain_aci_10_MASK,
			(clip_gains_acimode[9]
			<< LCN40PHY_agcControl238_rssi_clip_gain_aci_9_SHIFT) |
			(clip_gains_acimode[10]
			<< LCN40PHY_agcControl238_rssi_clip_gain_aci_10_SHIFT));
		PHY_REG_MOD(pi, LCN40PHY, agcControl239, rssi_clip_gain_aci_11,
			clip_gains_acimode[11]);
		PHY_REG_MOD(pi, LCN40PHY, agcControl37, rssi_no_clip_gain_aci,
			clip_gains_acimode[12]);

		/* Clip thresholds */
		PHY_REG_MOD(pi, LCN40PHY, agcControl43, rssi_clip_thresh_aci_0,
		clip_threshs_acimode[0]);
		phy_utils_mod_phyreg(pi, LCN40PHY_agcControl44,
			LCN40PHY_agcControl44_rssi_clip_thresh_aci_1_MASK |
			LCN40PHY_agcControl44_rssi_clip_thresh_aci_2_MASK,
			(clip_threshs_acimode[1]
			<< LCN40PHY_agcControl44_rssi_clip_thresh_aci_1_SHIFT) |
			(clip_threshs_acimode[2]
			<< LCN40PHY_agcControl44_rssi_clip_thresh_aci_2_SHIFT));
		phy_utils_mod_phyreg(pi, LCN40PHY_agcControl45,
			LCN40PHY_agcControl45_rssi_clip_thresh_aci_3_MASK |
			LCN40PHY_agcControl45_rssi_clip_thresh_aci_4_MASK,
			(clip_threshs_acimode[3]
			<< LCN40PHY_agcControl45_rssi_clip_thresh_aci_3_SHIFT) |
			(clip_threshs_acimode[4]
			<< LCN40PHY_agcControl45_rssi_clip_thresh_aci_4_SHIFT));
		phy_utils_mod_phyreg(pi, LCN40PHY_agcControl46,
			LCN40PHY_agcControl46_rssi_clip_thresh_aci_5_MASK |
			LCN40PHY_agcControl46_rssi_clip_thresh_aci_6_MASK,
			(clip_threshs_acimode[5]
			<< LCN40PHY_agcControl46_rssi_clip_thresh_aci_5_SHIFT) |
			(clip_threshs_acimode[6]
			<< LCN40PHY_agcControl46_rssi_clip_thresh_aci_6_SHIFT));
		phy_utils_mod_phyreg(pi, LCN40PHY_agcControl47,
			LCN40PHY_agcControl47_rssi_clip_thresh_aci_7_MASK |
			LCN40PHY_agcControl47_rssi_clip_thresh_aci_8_MASK,
			(clip_threshs_acimode[7]
			<< LCN40PHY_agcControl47_rssi_clip_thresh_aci_7_SHIFT) |
			(clip_threshs_acimode[8]
			<< LCN40PHY_agcControl47_rssi_clip_thresh_aci_8_SHIFT));
		phy_utils_mod_phyreg(pi, LCN40PHY_agcContro243,
			LCN40PHY_agcContro243_rssi_clip_thresh_aci_9_MASK |
			LCN40PHY_agcContro243_rssi_clip_thresh_aci_10_MASK,
			(clip_threshs_acimode[9]
			<< LCN40PHY_agcContro243_rssi_clip_thresh_aci_9_SHIFT) |
			(clip_threshs_acimode[10]
			<< LCN40PHY_agcContro243_rssi_clip_thresh_aci_10_SHIFT));
		PHY_REG_MOD(pi, LCN40PHY, agcContro244, rssi_clip_thresh_aci_11,
			clip_threshs_acimode[11]);
	}

	if (LCN40REV_LE(pi->pubpi.phy_rev, 2))
		PHY_REG_MOD(pi, LCN40PHY, CFOblkConfig, signalblk_det_thresh_ofdm_40, 94);
	if (LCN40REV_LE(pi->pubpi.phy_rev, 2) && pi->u.pi_lcn40phy->phycrs_war_en)
	{
		phy_utils_write_phyreg(pi, LCN40PHY_SignalBlock_edet1, 0x5858);
		PHY_REG_MOD(pi, LCN40PHY, CFOblkConfig, signalblk_det_thresh_ofdm_40, 0x58);
	}

	if (LCN40REV_GE(pi->pubpi.phy_rev, 4)) {
		PHY_REG_MOD(pi, LCN40PHY, SignalBlock_edet1, signalblk_det_thresh_ofdm, 0x61);
		PHY_REG_MOD(pi, LCN40PHY, CFOblkConfig, signalblk_det_thresh_ofdm_40, 0x61);
	}

	wlc_lcn40phy_agc_reset(pi);

	PHY_REG_LIST_START
		PHY_REG_MOD_ENTRY(LCN40PHY, agcControl4, hg_gate_ofdm_filt_en, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, pktfsmctrl, dmdStExitEn_alt, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, watchdog_en, global_en, 1)
		PHY_REG_WRITE_ENTRY(LCN40PHY, watchdog_pktfsm_timeout_CRS_STR1, 10)
		PHY_REG_WRITE_ENTRY(LCN40PHY, watchdog_agcfsm_timeout_SS_PACKET_RX_STATE, 4095)
	PHY_REG_LIST_EXECUTE(pi);

#ifdef BAND5G
	if (CHSPEC_IS5G(pi->radio_chanspec)) {
		phy_utils_mod_phyreg(pi, LCN40PHY_crsgainCtrl_new,
			LCN40PHY_crsgainCtrl_new_DSSSDetectionEnable20_MASK |
			LCN40PHY_crsgainCtrl_new_DSSSDetectionEnable20U_MASK |
			LCN40PHY_crsgainCtrl_new_DSSSDetectionEnable20L_MASK,
			0);
	}
#endif // endif
	PHY_REG_MOD(pi, LCN40PHY, ClkEnCtrl, forcerxfrontendclk, 1);

	if (LCN40REV_IS(pi->pubpi.phy_rev, 5))
	   /* turn off the reset for ACI detector */
	   phy_utils_write_phyreg(pi, LCN40PHY_workAround,
	   (phy_utils_read_phyreg(pi, LCN40PHY_workAround) & 0x7fff));

		if ((pi_lcn40->cdd_mod == 1) && (CHSPEC_IS20(pi->radio_chanspec)))
			PHY_REG_MOD(pi, LCN40PHY, OfdmFineSTR_config, LowMetricThreshEnHi, 0);

		if (pi_lcn40->fstr_flag == 1) {
		    PHY_REG_LIST_START
			PHY_REG_MOD_ENTRY(LCN40PHY, FineStr_config2, FineStrEn, 1)
			PHY_REG_MOD_ENTRY(LCN40PHY, agcControl11, rssi_gain, 73)
			PHY_REG_MOD_ENTRY(LCN40PHY, PreambleConfirmTimeout,
				OFDMPreambleConfirmTimeout, 4)
			PHY_REG_MOD_ENTRY(LCN40PHY, ofdmSyncTimerOffset0_new,
				OFDMPreambleSyncTimeOut20, 2)
			PHY_REG_MOD_ENTRY(LCN40PHY, ofdmSyncTimerOffset0_new,
				OFDMPreambleSyncTimeOut40, 0)
			PHY_REG_MOD_ENTRY(LCN40PHY, ofdmSyncTimerOffset1_new,
				OFDMPreambleSyncTimeOut20L, 2)
			PHY_REG_MOD_ENTRY(LCN40PHY, ofdmSyncTimerOffset1_new,
				OFDMPreambleSyncTimeOut20U, 2)
			PHY_REG_MOD_ENTRY(LCN40PHY, lsnrHold1_new, holdOnSyncCtrVal20, 9)
			PHY_REG_MOD_ENTRY(LCN40PHY, lsnrHold1_new, holdOnSyncCtrVal40, 15)
			PHY_REG_MOD_ENTRY(LCN40PHY, lsnrHold2_new, holdOnSyncCtrVal20U, 9)
			PHY_REG_MOD_ENTRY(LCN40PHY, lsnrHold2_new, holdOnSyncCtrVal20L, 9)
			PHY_REG_MOD_ENTRY(LCN40PHY, FineStr_config6, FstrMinKept_20m, 8)
			PHY_REG_MOD_ENTRY(LCN40PHY, FineStr_config6, FstrMinKept_40m, 11)
			PHY_REG_MOD_ENTRY(LCN40PHY, FineSTRConfigBundlen4, adv_range_control, 6)
			PHY_REG_MOD_ENTRY(LCN40PHY, FineSTRConfigBundlen4, ret_range_control, 20)
			PHY_REG_MOD_ENTRY(LCN40PHY, OfdmFineSTR_config3,
				OfdmFilterMedToLoGainDiff, 8)
			PHY_REG_MOD_ENTRY(LCN40PHY, FineStr_config4, FstrMinSearchLimitNo, 296)
		    PHY_REG_LIST_EXECUTE(pi);

		    if (CHSPEC_IS5G(pi->radio_chanspec))
		        PHY_REG_MOD(pi, LCN40PHY, agcControl24, rssi_no_clip_gain_mismatch, -18);

		    if (CHSPEC_IS40(pi->radio_chanspec)) {
			PHY_REG_LIST_START
				PHY_REG_MOD_ENTRY(LCN40PHY, FineStr_config1, FineStrNorSmoothing,
					48)
				PHY_REG_MOD_ENTRY(LCN40PHY, FineStr_config2, SgSmoothingWindwo, 48)
				PHY_REG_MOD_ENTRY(LCN40PHY, agcControl9, lo_gain_max_ofdm_mismatch,
					20)
				PHY_REG_MOD_ENTRY(LCN40PHY, agcControl10,
					hi_gain_max_ofdm_mismatch, 6)
				PHY_REG_WRITE_ENTRY(LCN40PHY, CrsNapLogicConfig_0_0, 0)
				PHY_REG_WRITE_ENTRY(LCN40PHY, CrsNapLogicConfig_1_0, 0)
				PHY_REG_MOD_ENTRY(LCN40PHY, rfseq_cfg, nap_en, 1)
			PHY_REG_LIST_EXECUTE(pi);
		    }
		    else {
		        PHY_REG_MOD(pi, LCN40PHY, FineStr_config1, FineStrNorSmoothing, 62);
		        PHY_REG_MOD(pi, LCN40PHY, FineStr_config2, SgSmoothingWindwo, 62);
		        if (CHSPEC_IS2G(pi->radio_chanspec))
		            PHY_REG_MOD(pi, LCN40PHY, lpParam2, gainSettleDlySmplCnt, 30);
		        else
		            PHY_REG_MOD(pi, LCN40PHY, lpParam2, gainSettleDlySmplCnt, 40);
			PHY_REG_LIST_START
				PHY_REG_MOD_ENTRY(LCN40PHY, agcControl9, lo_gain_max_ofdm_mismatch,
					20)
				PHY_REG_MOD_ENTRY(LCN40PHY, agcControl10,
					hi_gain_max_ofdm_mismatch, 10)
				PHY_REG_MOD_ENTRY(LCN40PHY, agcControl48,
					fstr_digi_gain_mask_thresh, 77)
				PHY_REG_WRITE_ENTRY(LCN40PHY, CrsNapLogicConfig_0_0, 0)
				PHY_REG_WRITE_ENTRY(LCN40PHY, CrsNapLogicConfig_1_0, 0)
				PHY_REG_MOD_ENTRY(LCN40PHY, rfseq_cfg, nap_en, 1)
			PHY_REG_LIST_EXECUTE(pi);
		    }
		}
}

static void
wlc_lcn40phy_rev6_agc_tweaks(phy_info_t *pi)
{
	const uint16 clip_gains[] = {50, 43, 40, 40, 40, 40, 34, 28, 22, 14, 7, 0, 60};
	const uint16 clip_threshs[] = {10, 10, 10, 0, 0, 0, 10, 10, 10, 10, 10, 10};
	const uint16 clip_gains_acimode[] = {43, 39, 33, 33, 30, 27, 24, 18, 15, 10, 7, 4, 55};
	const uint16 clip_threshs_acimode[] = {10, 10, 10, 0, 10, 10, 10, 10, 10, 10, 10, 10};
	phytbl_info_t tab;
	uint16 val = 0x30;

	phy_info_lcn40phy_t *pi_lcn40 = pi->u.pi_lcn40phy;

	PHY_REG_MOD(pi, LCN40PHY, ClipCtrThresh, ClipCtrThreshHiGain, 15);

	tab.tbl_id = LCN40PHY_TBL_ID_RFSEQ;
	tab.tbl_width = 16;	/* 12 bit wide	*/
	tab.tbl_ptr = &val;
	tab.tbl_len = 1;
	tab.tbl_offset = 0x3c;
	wlc_lcnphy_write_table(pi,  &tab);

	PHY_REG_LIST_START
		/* enable clamp */
		PHY_REG_MOD_ENTRY(LCN40PHY, AfeCtrlOvr1, afe_clamp_en_ovr, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, AfeCtrlOvr1Val, afe_clamp_en_ovr_val, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, AfeCtrlOvr1, afe_reset_ov_det_ovr, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, AfeCtrlOvr1Val, afe_reset_ov_det_ovr_val, 0)

		/* NB RSSI settings */
		RADIO_REG_MOD_ENTRY(RADIO_2065_NBRSSI_BIAS, 0xff03, 0x303)
		RADIO_REG_WRITE_ENTRY(RADIO_2065_NBRSSI_IB, 0x36e2)
		RADIO_REG_MOD_ENTRY(RADIO_2065_NBRSSI_CONFG, 0x73c0, 0x31c0)

		/* W3 RSSI settings */
		RADIO_REG_MOD_ENTRY(RADIO_2065_WRSSI3_BIAS, 0x00ff, 0x3)
		RADIO_REG_MOD_ENTRY(RADIO_2065_WRSSI3_CONFG, 0xe3c0, 0x6340)

		PHY_REG_MOD_ENTRY(LCN40PHY, radioTRCtrl, gainrequestTRAttnOnEn, 0)

		/* Tuning to avoid being stuck at BPHY_CONFIRM, an FSM bug */
		PHY_REG_MOD_ENTRY(LCN40PHY, DSSSConfirmCnt, DSSSConfirmCntLoGain, 3)

		/*  turn off clip backoff */
		PHY_REG_MOD_ENTRY(LCN40PHY, agcControl48, repeat_clip_backoff, 0)

		PHY_REG_MOD_ENTRY(LCN40PHY, agcControl9, lo_gain_max_ofdm_mismatch, 30)

		/* turn off aci detector */
		PHY_REG_MOD_ENTRY(LCN40PHY, aciContro9, aci_detect_aci_det_en, 0)
		PHY_REG_MOD_ENTRY(LCN40PHY, aciContro1, aci_detect_grp_wp_nor, 0x1)
		PHY_REG_MOD_ENTRY(LCN40PHY, aciContro2, aci_detect_grp_wp_aci, 0xffff)
		PHY_REG_MOD_ENTRY(LCN40PHY, aciContro3, aci_detect_grp_ws_nor, 15)
		PHY_REG_MOD_ENTRY(LCN40PHY, aciContro4, aci_detect_grp_ws_aci, 15)
		PHY_REG_MOD_ENTRY(LCN40PHY, aciContro9, aci_detect_grp_ci_nor, 2)
		PHY_REG_MOD_ENTRY(LCN40PHY, aciContro9, aci_detect_grp_ci_aci, 15)
		PHY_REG_MOD_ENTRY(LCN40PHY, aciContro5, aci_detect_grp_et_nor, 3000)
		PHY_REG_MOD_ENTRY(LCN40PHY, aciContro6, aci_detect_grp_et_aci, 0)
		PHY_REG_MOD_ENTRY(LCN40PHY, aciContro7, aci_detect_grp_dt_nor, 1000)
		PHY_REG_MOD_ENTRY(LCN40PHY, aciContro8, aci_detect_grp_dt_aci, 0)
		PHY_REG_MOD_ENTRY(LCN40PHY, aciContro9, aci_detect_aci_det_force, 0)

		PHY_REG_MOD_ENTRY(LCN40PHY, sslpnRxFeClkEnCtrl, allowCrsClkTxDisable, 1)

		PHY_REG_MOD_ENTRY(LCN40PHY, agcControl4, dc_bw_lock_en, 0)
		PHY_REG_MOD_ENTRY(LCN40PHY, RxRadioControlFltrState, rx_flt_high_start_state, 2)
		PHY_REG_MOD_ENTRY(LCN40PHY, RxRadioControlFltrState, rx_flt_low_start_state, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, low_pwr_rf_cfg_1, rx_flt_wake_start_state, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, low_pwr_rf_cfg_2, rx_flt_nap_start_state, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, gainMismatch, GainMismatchHigain, -8)
		PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride7, lpf_hpc_ovr, 0)
		PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride7, tia_hpc_ovr, 0)
	PHY_REG_LIST_EXECUTE(pi);

	/* gain settle delay */
	PHY_REG_MOD(pi, LCN40PHY, agcControl11, gain_settle_dly_cnt,
		pi_lcn40->gain_settle_dly_2g);

	PHY_REG_LIST_START
		PHY_REG_MOD_ENTRY(LCN40PHY, agcControl2, hg_clip_count, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, bphyTest, shiftBDigiGain, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, SignalBlockConfigTable2_new,
			crssignalblk_noi_pwr, 87)
		PHY_REG_MOD_ENTRY(LCN40PHY, SignalBlockConfigTable2_new,
			crssignalblk_noi_pwr_40mhz, 93)
		PHY_REG_MOD_ENTRY(LCN40PHY, agcControl27, data_mode_gain_db_adj_dsss, 0)

		RADIO_REG_WRITE_ENTRY(RADIO_2065_LNA2G_RSSI, 0x51f1)
	PHY_REG_LIST_EXECUTE(pi);

	PHY_REG_MOD(pi, LCN40PHY, SignalBlock_edet1, signalblk_det_thresh_dsss,
		pi_lcn40->dsss_thresh);

	if (CHSPEC_IS40(pi->radio_chanspec))
		PHY_REG_WRITE(pi, LCN40PHY, ClipThresh, 55);
	else
		PHY_REG_WRITE(pi, LCN40PHY, ClipThresh, 70);

	PHY_REG_LIST_START
		/* register tweaks to alleviate the bphy-stuck bug */
		PHY_REG_MOD_ENTRY(LCN40PHY, Bphycntctrl, claim_cnt, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, WaitforPHYSelTimeout, BPHYWaitforCCKSeltimeout, 15)
		PHY_REG_MOD_ENTRY(LCN40PHY, DSSSConfirmCnt, DSSSConfirmCntLoGain, 4)
		PHY_REG_MOD_ENTRY(LCN40PHY, DSSSConfirmCnt, DSSSConfirmCntHiGain, 5)

		/* post-aci-filter power calculation */
		PHY_REG_MOD_ENTRY(LCN40PHY, crsMiscCtrl0, usePreFiltPwr, 0)
	PHY_REG_LIST_EXECUTE(pi);

	/* Select ACI mode */
	if (pi_lcn40->gaintbl_force_2g > 0) {
		pi->sh->interference_mode = WLAN_MANUAL;
	} else if (pi_lcn40->aci_detect_en_2g > 0) {
		pi->sh->interference_mode = WLAN_AUTO;
	} else {
		pi->sh->interference_mode = INTERFERE_NONE;
	}
	wlc_lcn40phy_rev6_aci(pi, pi->sh->interference_mode);
	pi->sh->interference_mode_2G = pi->sh->interference_mode;

	/* Initialize ED based on the channel bandwidth  for 43143 */
	if (CHSPEC_IS20(pi->radio_chanspec)) {
		PHY_REG_LIST_START
			PHY_REG_MOD_ENTRY(LCN40PHY, eddisable20ul, crseddisable_20L, 0)
			PHY_REG_MOD_ENTRY(LCN40PHY, eddisable20ul, crseddisable_20U, 1)
			PHY_REG_MOD_ENTRY(LCN40PHY, eddisable20ul, crseddisable_40, 1)
		PHY_REG_LIST_EXECUTE(pi);
	} else {
		PHY_REG_LIST_START
			PHY_REG_MOD_ENTRY(LCN40PHY, eddisable20ul, crseddisable_20L, 0)
			PHY_REG_MOD_ENTRY(LCN40PHY, eddisable20ul, crseddisable_20U, 0)
			PHY_REG_MOD_ENTRY(LCN40PHY, eddisable20ul, crseddisable_40, 0)
		PHY_REG_LIST_EXECUTE(pi);
	}

	/* Initialize the edon/off thresholds based on NVRAM for 43143 */
	wlc_lcn40phy_set_ed_thres(pi);

	/* Turn off TIA DC Loop */
	PHY_REG_MOD(pi, LCN40PHY, radio_rx_cfg_pu, rx_tia_dc_byp, 1);
	PHY_REG_MOD(pi, LCN40PHY, radio_rx_cfg_pu, rx_tia_dc_pu, 0);

	phy_utils_mod_phyreg(pi, LCN40PHY_agcControl14,
		LCN40PHY_agcControl14_rssi_clip_gain_norm_0_MASK |
		LCN40PHY_agcControl14_rssi_clip_gain_norm_1_MASK,
		(clip_gains[0] << LCN40PHY_agcControl14_rssi_clip_gain_norm_0_SHIFT) |
		(clip_gains[1] << LCN40PHY_agcControl14_rssi_clip_gain_norm_1_SHIFT));
	phy_utils_mod_phyreg(pi, LCN40PHY_agcControl15,
		LCN40PHY_agcControl15_rssi_clip_gain_norm_2_MASK |
		LCN40PHY_agcControl15_rssi_clip_gain_norm_3_MASK,
		(clip_gains[2] << LCN40PHY_agcControl15_rssi_clip_gain_norm_2_SHIFT) |
		(clip_gains[3] << LCN40PHY_agcControl15_rssi_clip_gain_norm_3_SHIFT));
	phy_utils_mod_phyreg(pi, LCN40PHY_agcControl16,
		LCN40PHY_agcControl16_rssi_clip_gain_norm_4_MASK |
		LCN40PHY_agcControl16_rssi_clip_gain_norm_5_MASK,
		(clip_gains[4] << LCN40PHY_agcControl16_rssi_clip_gain_norm_4_SHIFT) |
		(clip_gains[5] << LCN40PHY_agcControl16_rssi_clip_gain_norm_5_SHIFT));
	phy_utils_mod_phyreg(pi, LCN40PHY_agcControl17,
		LCN40PHY_agcControl17_rssi_clip_gain_norm_6_MASK |
		LCN40PHY_agcControl17_rssi_clip_gain_norm_7_MASK,
		(clip_gains[6] << LCN40PHY_agcControl17_rssi_clip_gain_norm_6_SHIFT) |
		(clip_gains[7] << LCN40PHY_agcControl17_rssi_clip_gain_norm_7_SHIFT));
	PHY_REG_MOD(pi, LCN40PHY, agcControl18, rssi_clip_gain_norm_8, clip_gains[8]);
	phy_utils_mod_phyreg(pi, LCN40PHY_agcContro245,
		LCN40PHY_agcContro245_rssi_clip_gain_norm_9_MASK |
		LCN40PHY_agcContro245_rssi_clip_gain_norm_10_MASK,
		(clip_gains[9] << LCN40PHY_agcContro245_rssi_clip_gain_norm_9_SHIFT) |
		(clip_gains[10] << LCN40PHY_agcContro245_rssi_clip_gain_norm_10_SHIFT));
	PHY_REG_MOD(pi, LCN40PHY, agcContro246, rssi_clip_gain_norm_11, clip_gains[11]);
	PHY_REG_MOD(pi, LCN40PHY, agcControl38, rssi_no_clip_gain_normal, clip_gains[12]);
	PHY_REG_MOD(pi, LCN40PHY, agcControl24, rssi_no_clip_gain_mismatch, 5);

	phy_utils_mod_phyreg(pi, LCN40PHY_agcControl19,
		LCN40PHY_agcControl19_rssi_clip_thresh_norm_0_MASK |
		LCN40PHY_agcControl19_rssi_clip_thresh_norm_1_MASK,
		(clip_threshs[0] << LCN40PHY_agcControl19_rssi_clip_thresh_norm_0_SHIFT) |
		(clip_threshs[1] << LCN40PHY_agcControl19_rssi_clip_thresh_norm_1_SHIFT));
	phy_utils_mod_phyreg(pi, LCN40PHY_agcControl20,
		LCN40PHY_agcControl20_rssi_clip_thresh_norm_2_MASK |
		LCN40PHY_agcControl20_rssi_clip_thresh_norm_3_MASK,
		(clip_threshs[2] << LCN40PHY_agcControl20_rssi_clip_thresh_norm_2_SHIFT) |
		(clip_threshs[3] << LCN40PHY_agcControl20_rssi_clip_thresh_norm_3_SHIFT));
	phy_utils_mod_phyreg(pi, LCN40PHY_agcControl21,
		LCN40PHY_agcControl21_rssi_clip_thresh_norm_4_MASK |
		LCN40PHY_agcControl21_rssi_clip_thresh_norm_5_MASK,
		(clip_threshs[4] << LCN40PHY_agcControl21_rssi_clip_thresh_norm_4_SHIFT) |
		(clip_threshs[5] << LCN40PHY_agcControl21_rssi_clip_thresh_norm_5_SHIFT));
	phy_utils_mod_phyreg(pi, LCN40PHY_agcControl22,
		LCN40PHY_agcControl22_rssi_clip_thres_norm_6_MASK |
		LCN40PHY_agcControl22_rssi_clip_thresh_norm_7_MASK,
		(clip_threshs[6] << LCN40PHY_agcControl22_rssi_clip_thres_norm_6_SHIFT) |
		(clip_threshs[7] << LCN40PHY_agcControl22_rssi_clip_thresh_norm_7_SHIFT));
	phy_utils_mod_phyreg(pi, LCN40PHY_agcControl23,
		LCN40PHY_agcControl23_rssi_clip_thresh_norm_8_MASK |
		LCN40PHY_agcControl23_rssi_clip_thresh_norm_9_MASK,
		(clip_threshs[8] << LCN40PHY_agcControl23_rssi_clip_thresh_norm_8_SHIFT) |
		(clip_threshs[9] << LCN40PHY_agcControl23_rssi_clip_thresh_norm_9_SHIFT));
	phy_utils_mod_phyreg(pi, LCN40PHY_agcContro240,
		LCN40PHY_agcContro240_rssi_clip_thresh_norm_10_MASK |
		LCN40PHY_agcContro240_rssi_clip_thresh_norm_11_MASK,
		(clip_threshs[10] << LCN40PHY_agcContro240_rssi_clip_thresh_norm_10_SHIFT) |
		(clip_threshs[11] << LCN40PHY_agcContro240_rssi_clip_thresh_norm_11_SHIFT));

	/* ==========================================
	 * ACI mode
	 * ==========================================
	 */

	/* Clip gains */
	phy_utils_mod_phyreg(pi, LCN40PHY_agcControl33,
		LCN40PHY_agcControl33_rssi_clip_gain_aci_0_MASK |
		LCN40PHY_agcControl33_rssi_clip_gain_aci_1_MASK,
		(clip_gains_acimode[0]
		<< LCN40PHY_agcControl33_rssi_clip_gain_aci_0_SHIFT) |
		(clip_gains_acimode[1]
		<< LCN40PHY_agcControl33_rssi_clip_gain_aci_1_SHIFT));
	phy_utils_mod_phyreg(pi, LCN40PHY_agcControl34,
		LCN40PHY_agcControl34_rssi_clip_gain_aci_2_MASK |
		LCN40PHY_agcControl34_rssi_clip_gain_aci_3_MASK,
		(clip_gains_acimode[2]
		<< LCN40PHY_agcControl34_rssi_clip_gain_aci_2_SHIFT) |
		(clip_gains_acimode[3]
		<< LCN40PHY_agcControl34_rssi_clip_gain_aci_3_SHIFT));
	phy_utils_mod_phyreg(pi, LCN40PHY_agcControl35,
		LCN40PHY_agcControl35_rssi_clip_gain_aci_4_MASK |
		LCN40PHY_agcControl35_rssi_clip_gain_aci_5_MASK,
		(clip_gains_acimode[4]
		<< LCN40PHY_agcControl35_rssi_clip_gain_aci_4_SHIFT) |
		(clip_gains_acimode[5]
		<< LCN40PHY_agcControl35_rssi_clip_gain_aci_5_SHIFT));
	phy_utils_mod_phyreg(pi, LCN40PHY_agcControl36,
		LCN40PHY_agcControl36_rssi_clip_gain_aci_6_MASK |
		LCN40PHY_agcControl36_rssi_clip_gain_aci_7_MASK,
		(clip_gains_acimode[6]
		<< LCN40PHY_agcControl36_rssi_clip_gain_aci_6_SHIFT) |
		(clip_gains_acimode[7]
		<< LCN40PHY_agcControl36_rssi_clip_gain_aci_7_SHIFT));
	PHY_REG_MOD(pi, LCN40PHY, agcControl37,
		rssi_clip_gain_aci_8, clip_gains_acimode[8]);
	phy_utils_mod_phyreg(pi, LCN40PHY_agcControl238,
		LCN40PHY_agcControl238_rssi_clip_gain_aci_9_MASK |
		LCN40PHY_agcControl238_rssi_clip_gain_aci_10_MASK,
		(clip_gains_acimode[9]
		<< LCN40PHY_agcControl238_rssi_clip_gain_aci_9_SHIFT) |
		(clip_gains_acimode[10]
		<< LCN40PHY_agcControl238_rssi_clip_gain_aci_10_SHIFT));
	PHY_REG_MOD(pi, LCN40PHY, agcControl239, rssi_clip_gain_aci_11,
		clip_gains_acimode[11]);
	PHY_REG_MOD(pi, LCN40PHY, agcControl37, rssi_no_clip_gain_aci,
		clip_gains_acimode[12]);

	/* Clip thresholds */
	PHY_REG_MOD(pi, LCN40PHY, agcControl43, rssi_clip_thresh_aci_0,
	clip_threshs_acimode[0]);
	phy_utils_mod_phyreg(pi, LCN40PHY_agcControl44,
		LCN40PHY_agcControl44_rssi_clip_thresh_aci_1_MASK |
		LCN40PHY_agcControl44_rssi_clip_thresh_aci_2_MASK,
		(clip_threshs_acimode[1]
		<< LCN40PHY_agcControl44_rssi_clip_thresh_aci_1_SHIFT) |
		(clip_threshs_acimode[2]
		<< LCN40PHY_agcControl44_rssi_clip_thresh_aci_2_SHIFT));
	phy_utils_mod_phyreg(pi, LCN40PHY_agcControl45,
		LCN40PHY_agcControl45_rssi_clip_thresh_aci_3_MASK |
		LCN40PHY_agcControl45_rssi_clip_thresh_aci_4_MASK,
		(clip_threshs_acimode[3]
		<< LCN40PHY_agcControl45_rssi_clip_thresh_aci_3_SHIFT) |
		(clip_threshs_acimode[4]
		<< LCN40PHY_agcControl45_rssi_clip_thresh_aci_4_SHIFT));
	phy_utils_mod_phyreg(pi, LCN40PHY_agcControl46,
		LCN40PHY_agcControl46_rssi_clip_thresh_aci_5_MASK |
		LCN40PHY_agcControl46_rssi_clip_thresh_aci_6_MASK,
		(clip_threshs_acimode[5]
		<< LCN40PHY_agcControl46_rssi_clip_thresh_aci_5_SHIFT) |
		(clip_threshs_acimode[6]
		<< LCN40PHY_agcControl46_rssi_clip_thresh_aci_6_SHIFT));
	phy_utils_mod_phyreg(pi, LCN40PHY_agcControl47,
		LCN40PHY_agcControl47_rssi_clip_thresh_aci_7_MASK |
		LCN40PHY_agcControl47_rssi_clip_thresh_aci_8_MASK,
		(clip_threshs_acimode[7]
		<< LCN40PHY_agcControl47_rssi_clip_thresh_aci_7_SHIFT) |
		(clip_threshs_acimode[8]
		<< LCN40PHY_agcControl47_rssi_clip_thresh_aci_8_SHIFT));
	phy_utils_mod_phyreg(pi, LCN40PHY_agcContro243,
		LCN40PHY_agcContro243_rssi_clip_thresh_aci_9_MASK |
		LCN40PHY_agcContro243_rssi_clip_thresh_aci_10_MASK,
		(clip_threshs_acimode[9]
		<< LCN40PHY_agcContro243_rssi_clip_thresh_aci_9_SHIFT) |
		(clip_threshs_acimode[10]
		<< LCN40PHY_agcContro243_rssi_clip_thresh_aci_10_SHIFT));
	PHY_REG_MOD(pi, LCN40PHY, agcContro244, rssi_clip_thresh_aci_11,
		clip_threshs_acimode[11]);

	wlc_lcn40phy_agc_reset(pi);

	PHY_REG_LIST_START
		PHY_REG_MOD_ENTRY(LCN40PHY, agcControl4, hg_gate_ofdm_filt_en, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, pktfsmctrl, dmdStExitEn_alt, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, watchdog_en, global_en, 1)
		PHY_REG_WRITE_ENTRY(LCN40PHY, watchdog_pktfsm_timeout_CRS_STR1, 10)
		PHY_REG_WRITE_ENTRY(LCN40PHY, watchdog_agcfsm_timeout_SS_PACKET_RX_STATE, 4095)

		PHY_REG_MOD_ENTRY(LCN40PHY, ClkEnCtrl, forcerxfrontendclk, 1)
	PHY_REG_LIST_EXECUTE(pi);
}

static void
wlc_lcn40phy_rev7_agc_tweaks(phy_info_t *pi)
{

	uint16 clip_gain_2g[] = {56, 45, 42, 37, 32, 31, 28, 23, 20, 13, 7, 1, 62};
	uint16 clip_thresh_2g[] = {15, 12, 10, 0, 0, 0, 10, 10, 20, 15, 10, 10};
	uint16 clip_gain_2g_40mhz[] = {56, 45, 42, 37, 32, 31, 28, 25, 20, 20, 10, 1, 62};
	uint16 clip_thresh_2g_40mhz[] = {10, 10, 10, 0, 0, 0, 10, 10, 10, 10, 15, 20};
	uint16 clip_gain_2g_extlna[] = {66, 61, 56, 49, 41, 36, 32, 13, 10, 20, 10, 1, 62};
	uint16 clip_thresh_2g_extlna[] = {10, 10, 10, 10, 10, 19, 10, 10, 10, 10, 15, 20};
	uint16 aci_clip_gain_2g[] = {50, 43, 38, 34, 32, 32, 28, 24, 22, 10, 7, 4, 60};
	uint16 aci_clip_thresh_2g[] = {10, 10, 10, 10, 10, 10, 10, 10, 1, 10, 10, 10};
#ifdef BAND5G
	uint16 clip_gain_5g[] = {58, 52, 45, 40, 40, 34, 26, 25, 20, 13, 13, 5, 62};
	uint16 clip_gain_5g_40mhz[] = {58, 52, 45, 40, 40, 34, 28, 23, 23, 13, 13, 5, 62};
	uint16 clip_gain_5g_extlna[] = {50, 47, 43, 43, 40, 34, 32, 28, 24, 13, 13, 5, 58};
	uint16 clip_thresh_5g[] = {15, 15, 15, 5, 5, 10, 18, 15, 4, 10, 5, 0};
	uint16 clip_thresh_5g_extlna[] = {20, 20, 20, 0, 0, 0, 10, 10, 10, 10, 5, 0};
#endif /* BAND5G */
	uint16 *clip_gains = NULL, *clip_threshs = NULL;
	uint16 *aci_clip_gains = NULL, *aci_clip_threshs = NULL;

	phy_info_lcn40phy_t *pi_lcn40 = pi->u.pi_lcn40phy;
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);

	PHY_REG_LIST_START
		RADIO_REG_MOD_ENTRY(RADIO_2065_NBRSSI_BIAS, 0xff03, 0x303)
		RADIO_REG_WRITE_ENTRY(RADIO_2065_NBRSSI_IB, 0x36e2)
		RADIO_REG_MOD_ENTRY(RADIO_2065_NBRSSI_CONFG, 0x7000, 0x3000)
		PHY_REG_MOD_ENTRY(LCN40PHY, radioTRCtrl, gainrequestTRAttnOnEn, 0)
		/* Turn on TIA DC loop */
		PHY_REG_MOD_ENTRY(LCN40PHY, radio_rx_cfg_pu, rx_tia_dc_byp, 0)
		PHY_REG_MOD_ENTRY(LCN40PHY, radio_rx_cfg_pu, rx_tia_dc_pu, 1)
	PHY_REG_LIST_EXECUTE(pi);
	/* ACI det parameter settings */
	PHY_REG_LIST_START
		PHY_REG_MOD_ENTRY(LCN40PHY, aciContro9, aci_detect_aci_det_en, 0)
		PHY_REG_MOD_ENTRY(LCN40PHY, aciContro1, aci_detect_grp_wp_nor, 0x1)
		PHY_REG_MOD_ENTRY(LCN40PHY, aciContro2, aci_detect_grp_wp_aci, 0xffff)
		PHY_REG_MOD_ENTRY(LCN40PHY, aciContro3, aci_detect_grp_ws_nor, 15)
		PHY_REG_MOD_ENTRY(LCN40PHY, aciContro4, aci_detect_grp_ws_aci, 15)
		PHY_REG_MOD_ENTRY(LCN40PHY, aciContro9, aci_detect_grp_ci_nor, 2)
		PHY_REG_MOD_ENTRY(LCN40PHY, aciContro9, aci_detect_grp_ci_aci, 15)
		PHY_REG_MOD_ENTRY(LCN40PHY, aciContro5, aci_detect_grp_et_nor, 3000)
		PHY_REG_MOD_ENTRY(LCN40PHY, aciContro6, aci_detect_grp_et_aci, 0)
		PHY_REG_MOD_ENTRY(LCN40PHY, aciContro7, aci_detect_grp_dt_nor, 1800)
		PHY_REG_MOD_ENTRY(LCN40PHY, aciContro8, aci_detect_grp_dt_aci, 0)
		PHY_REG_MOD_ENTRY(LCN40PHY, aciContro9, aci_detect_aci_det_force, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, aciContro9, aci_detect_aci_pre_sel, 0)
		PHY_REG_MOD_ENTRY(LCN40PHY, sslpnRxFeClkEnCtrl, allowCrsClkTxDisable, 1)
	PHY_REG_LIST_EXECUTE(pi);

	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		if (CHSPEC_IS20(pi->radio_chanspec)) {
			PHY_REG_MOD(pi, LCN40PHY, ClipDetector, Clip_detecotr_thresh, 40);
			PHY_REG_WRITE(pi, LCN40PHY, ClipThresh, 0x3c);
		} else {
			PHY_REG_MOD(pi, LCN40PHY, ClipDetector, Clip_detecotr_thresh, 60);
			PHY_REG_WRITE(pi, LCN40PHY, ClipThresh, 0x39);
		}
	}
#ifdef BAND5G
	else {
		if (CHSPEC_IS20(pi->radio_chanspec)) {
			PHY_REG_MOD(pi, LCN40PHY, ClipDetector, Clip_detecotr_thresh, 32);
			PHY_REG_WRITE(pi, LCN40PHY, ClipThresh, 0x31);
		} else {
			PHY_REG_MOD(pi, LCN40PHY, ClipDetector, Clip_detecotr_thresh, 64);
			PHY_REG_WRITE(pi, LCN40PHY, ClipThresh, 0x39);
		}
	}
#endif /* BAND5G */
	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		PHY_REG_LIST_START
			RADIO_REG_WRITE_ENTRY(RADIO_2065_LNA2G_RSSI, 0x51f1)
			PHY_REG_MOD_ENTRY(LCN40PHY, agcControl24, rssi_no_clip_gain_adj, 3)
			/* Revert RX5G Gain boost WAR for 43341 */
			PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride5, rxrf_mix_gm_size_ovr, 0x0)
			PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride5, rxrf_mix_gm_half_en_ovr, 0x0)
			PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride8, tia_dc_loop_pu_ovr, 0x0)
			PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride7, tia_dc_loop_bypass_ovr, 0x0)
			PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride4, lpf_dc1_pwrup_ovr, 0x0)
			PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride7, lpf_hpc_ovr, 0x0)
			PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride7, lpf_aaci_en_ovr, 0x0)
			/* End of RX5G Gain boost WAR for 43341 */
		PHY_REG_LIST_EXECUTE(pi);
		if (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA) {
			if (pi_lcn40->trGain != 0xFF) {
				PHY_REG_MOD(pi, LCN40PHY, radioTRCtrlCrs1, trGainThresh,
					pi_lcn40->trGain);
				PHY_REG_MOD(pi, LCN40PHY, radioTRCtrl, gainrequestTRAttnOnEn, 1);
				PHY_REG_MOD(pi, LCN40PHY, radioTRCtrl, gainrequestTRAttnOnOffset,
					pi_lcn->lcnphy_tr_isolation_mid);
			}
			clip_gains = clip_gain_2g_extlna;
			clip_threshs = clip_thresh_2g_extlna;
		} else {
			if (CHSPEC_IS20(pi->radio_chanspec)) {
				clip_gains = clip_gain_2g;
				clip_threshs = clip_thresh_2g;
				aci_clip_gains = aci_clip_gain_2g;
				aci_clip_threshs = aci_clip_thresh_2g;
			} else {
				clip_gains = clip_gain_2g_40mhz;
				clip_threshs = clip_thresh_2g_40mhz;
				/* the gain/threshold is same as 20Mhz */
				aci_clip_gains = aci_clip_gain_2g;
				aci_clip_threshs = aci_clip_thresh_2g;
			}

		}
		PHY_REG_LIST_START
			PHY_REG_MOD_ENTRY(LCN40PHY, agcControl11, gain_settle_dly_cnt, 5)
			PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride7, lpf_hpc_ovr, 0x1)
			PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride7val, lpf_hpc_ovr_val, 3)
			PHY_REG_MOD_ENTRY(LCN40PHY, agcControl9, lo_gain_max_ofdm_mismatch, 30)
		PHY_REG_LIST_EXECUTE(pi);
		if (CHSPEC_IS20(pi->radio_chanspec)) {
			if ((CHSPEC_CHANNEL(pi->radio_chanspec) == 4) ||
				(CHSPEC_CHANNEL(pi->radio_chanspec) == 5) ||
				(CHSPEC_CHANNEL(pi->radio_chanspec) == 12) ||
				(CHSPEC_CHANNEL(pi->radio_chanspec) == 13)) {
				/* cck_bias_gain adjust to increase cck11 sensitivity */
				/* Somehow driver prefers "0xfd" while tcl is better with "0x2" */
				PHY_REG_MOD(pi, LCN40PHY, agcControl27,
					data_mode_gain_db_adj_dsss, 0xfd);
			} else {
				PHY_REG_MOD(pi, LCN40PHY, agcControl27,
					data_mode_gain_db_adj_dsss, 0x0);
			}
			PHY_REG_MOD(pi, LCN40PHY, SignalBlock_edet1, signalblk_det_thresh_dsss, 99);
		} else {
			PHY_REG_MOD(pi, LCN40PHY, SignalBlock_edet1, signalblk_det_thresh_dsss, 92);
		}
	}
#ifdef BAND5G
	else {
		PHY_REG_LIST_START
			RADIO_REG_WRITE_ENTRY(RADIO_2065_LNA5G_RSSI, 0x51d1)
			RADIO_REG_MOD_ENTRY(RADIO_2065_TIA_CFG2, 0x3000, 0x1000)
			PHY_REG_MOD_ENTRY(LCN40PHY, agcControl24, rssi_no_clip_gain_adj, -5)
			PHY_REG_MOD_ENTRY(LCN40PHY, agcControl9, lo_gain_max_ofdm_mismatch, 30)
			/* RX5G Gain boost WAR for 43341 */
			PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride5, rxrf_mix_gm_size_ovr, 0x1)
			PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride5, rxrf_mix_gm_half_en_ovr, 0x1)
			PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride5val, rxrf_mix_gm_size_ovr_val, 0x2)
			PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride5val,
				rxrf_mix_gm_half_en_ovr_val, 0x1)
			PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride8, tia_dc_loop_pu_ovr, 0x1)
			PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride8val, tia_dc_loop_pu_ovr_val, 0x1)
			PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride7, tia_dc_loop_bypass_ovr, 0x1)
			PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride7val, tia_dc_loop_bypass_ovr_val, 0x0)
			PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride4, lpf_dc1_pwrup_ovr, 0x1)
			PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride4val, lpf_dc1_pwrup_ovr_val, 0x1)
			PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride7, lpf_aaci_en_ovr, 0x1)
			PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride7val, lpf_aaci_en_ovr_val, 0x0)
		PHY_REG_LIST_EXECUTE(pi);
		/* End of RX5G Gain boost WAR for 43341 */
		if (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA_5GHz) {
			clip_gains = clip_gain_5g_extlna;
			clip_threshs = clip_thresh_5g_extlna;
			PHY_REG_MOD(pi, LCN40PHY, radioTRCtrl, gainrequestTRAttnOnEn, 1);
			if (pi_lcn40->elna_off_gain_idx_5g != 0xFF)
				PHY_REG_MOD(pi, LCN40PHY, radioTRCtrlCrs1, trGainThresh,
				pi_lcn40->elna_off_gain_idx_5g);
			else
				PHY_REG_MOD(pi, LCN40PHY, radioTRCtrlCrs1, trGainThresh, 35);
			if (pi_lcn->triso5g[0] != 0xff)
				PHY_REG_MOD(pi, LCN40PHY, radioTRCtrl, gainrequestTRAttnOnOffset,
					pi_lcn->triso5g[0]);
			PHY_REG_MOD(pi, LCN40PHY, agcControl24, rssi_no_clip_gain_adj, 0);
		} else {
			if (CHSPEC_IS20(pi->radio_chanspec)) {
				clip_gains = clip_gain_5g;
			} else {
				clip_gains = clip_gain_5g_40mhz;
			}
			clip_threshs = clip_thresh_5g;
		}
		PHY_REG_LIST_START
			PHY_REG_MOD_ENTRY(LCN40PHY, agcControl11, gain_settle_dly_cnt, 4)
			PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride7, lpf_hpc_ovr, 0x1)
			PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride7val, lpf_hpc_ovr_val, 1)
			PHY_REG_MOD_ENTRY(LCN40PHY, SignalBlock_edet1,
				signalblk_det_thresh_dsss, 19)
		PHY_REG_LIST_EXECUTE(pi);
	}
#endif  /* BAND5G */

	/* aci detector and set related aci detector parameters */
	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		if (pi_lcn40->aci_detect_en_2g != -1) {
			PHY_REG_MOD(pi, LCN40PHY, aciContro9, aci_detect_aci_det_en,
				pi_lcn40->aci_detect_en_2g);
			if (pi_lcn40->aci_detect_en_2g == 1)
				PHY_REG_MOD(pi, LCN40PHY, aciContro9, aci_detect_aci_det_force, 0);
			else
				PHY_REG_MOD(pi, LCN40PHY, aciContro9, aci_detect_aci_det_force, 1);
		} else {
			PHY_REG_MOD(pi, LCN40PHY, aciContro9, aci_detect_aci_det_en, 0);
			/* force gain table */
			PHY_REG_MOD(pi, LCN40PHY, aciContro9,
				aci_detect_aci_det_force, 1);
			/* sel gain table, default is normal table */
			if (pi_lcn40->gaintbl_presel_2g != -1) {
				PHY_REG_MOD(pi, LCN40PHY, aciContro9,
				 aci_detect_aci_pre_sel, pi_lcn40->gaintbl_presel_2g);
			}
		}
	} else {
		PHY_REG_MOD(pi, LCN40PHY, aciContro9, aci_detect_aci_det_en, 0);
		PHY_REG_MOD(pi, LCN40PHY, aciContro9, aci_detect_aci_det_force, 1);
		PHY_REG_MOD(pi, LCN40PHY, aciContro9, aci_detect_aci_pre_sel, 0);
	}
	if (aci_clip_gains) {
		phy_utils_mod_phyreg(pi, LCN40PHY_agcControl33,
			LCN40PHY_agcControl33_rssi_clip_gain_aci_0_MASK |
			LCN40PHY_agcControl33_rssi_clip_gain_aci_1_MASK,
			(aci_clip_gains[0] << LCN40PHY_agcControl33_rssi_clip_gain_aci_0_SHIFT) |
			(aci_clip_gains[1] << LCN40PHY_agcControl33_rssi_clip_gain_aci_1_SHIFT));
		phy_utils_mod_phyreg(pi, LCN40PHY_agcControl34,
			LCN40PHY_agcControl34_rssi_clip_gain_aci_2_MASK |
			LCN40PHY_agcControl34_rssi_clip_gain_aci_3_MASK,
			(aci_clip_gains[2] << LCN40PHY_agcControl34_rssi_clip_gain_aci_2_SHIFT) |
			(aci_clip_gains[3] << LCN40PHY_agcControl34_rssi_clip_gain_aci_3_SHIFT));
		phy_utils_mod_phyreg(pi, LCN40PHY_agcControl35,
			LCN40PHY_agcControl35_rssi_clip_gain_aci_4_MASK |
			LCN40PHY_agcControl35_rssi_clip_gain_aci_5_MASK,
			(aci_clip_gains[4] << LCN40PHY_agcControl35_rssi_clip_gain_aci_4_SHIFT) |
			(aci_clip_gains[5] << LCN40PHY_agcControl35_rssi_clip_gain_aci_5_SHIFT));
		phy_utils_mod_phyreg(pi, LCN40PHY_agcControl36,
			LCN40PHY_agcControl36_rssi_clip_gain_aci_6_MASK |
			LCN40PHY_agcControl36_rssi_clip_gain_aci_7_MASK,
			(aci_clip_gains[6] << LCN40PHY_agcControl36_rssi_clip_gain_aci_6_SHIFT) |
			(aci_clip_gains[7] << LCN40PHY_agcControl36_rssi_clip_gain_aci_7_SHIFT));
		PHY_REG_MOD(pi, LCN40PHY, agcControl37, rssi_clip_gain_aci_8, aci_clip_gains[8]);
		phy_utils_mod_phyreg(pi, LCN40PHY_agcControl238,
			LCN40PHY_agcControl238_rssi_clip_gain_aci_9_MASK |
			LCN40PHY_agcControl238_rssi_clip_gain_aci_10_MASK,
			(aci_clip_gains[9] << LCN40PHY_agcControl238_rssi_clip_gain_aci_9_SHIFT) |
			(aci_clip_gains[10] << LCN40PHY_agcControl238_rssi_clip_gain_aci_10_SHIFT));
		PHY_REG_MOD(pi, LCN40PHY, agcControl239, rssi_clip_gain_aci_11, aci_clip_gains[11]);
		PHY_REG_MOD(pi, LCN40PHY, agcControl37, rssi_no_clip_gain_aci, aci_clip_gains[12]);
	}

	if (clip_gains) {
		phy_utils_mod_phyreg(pi, LCN40PHY_agcControl14,
			LCN40PHY_agcControl14_rssi_clip_gain_norm_0_MASK |
			LCN40PHY_agcControl14_rssi_clip_gain_norm_1_MASK,
			(clip_gains[0] << LCN40PHY_agcControl14_rssi_clip_gain_norm_0_SHIFT) |
		(clip_gains[1] << LCN40PHY_agcControl14_rssi_clip_gain_norm_1_SHIFT));
		phy_utils_mod_phyreg(pi, LCN40PHY_agcControl15,
			LCN40PHY_agcControl15_rssi_clip_gain_norm_2_MASK |
			LCN40PHY_agcControl15_rssi_clip_gain_norm_3_MASK,
			(clip_gains[2] << LCN40PHY_agcControl15_rssi_clip_gain_norm_2_SHIFT) |
			(clip_gains[3] << LCN40PHY_agcControl15_rssi_clip_gain_norm_3_SHIFT));
		phy_utils_mod_phyreg(pi, LCN40PHY_agcControl16,
			LCN40PHY_agcControl16_rssi_clip_gain_norm_4_MASK |
			LCN40PHY_agcControl16_rssi_clip_gain_norm_5_MASK,
			(clip_gains[4] << LCN40PHY_agcControl16_rssi_clip_gain_norm_4_SHIFT) |
			(clip_gains[5] << LCN40PHY_agcControl16_rssi_clip_gain_norm_5_SHIFT));
		phy_utils_mod_phyreg(pi, LCN40PHY_agcControl17,
			LCN40PHY_agcControl17_rssi_clip_gain_norm_6_MASK |
			LCN40PHY_agcControl17_rssi_clip_gain_norm_7_MASK,
			(clip_gains[6] << LCN40PHY_agcControl17_rssi_clip_gain_norm_6_SHIFT) |
			(clip_gains[7] << LCN40PHY_agcControl17_rssi_clip_gain_norm_7_SHIFT));
		PHY_REG_MOD(pi, LCN40PHY, agcControl18, rssi_clip_gain_norm_8, clip_gains[8]);
		phy_utils_mod_phyreg(pi, LCN40PHY_agcContro245,
			LCN40PHY_agcContro245_rssi_clip_gain_norm_9_MASK |
			LCN40PHY_agcContro245_rssi_clip_gain_norm_10_MASK,
			(clip_gains[9] << LCN40PHY_agcContro245_rssi_clip_gain_norm_9_SHIFT) |
			(clip_gains[10] << LCN40PHY_agcContro245_rssi_clip_gain_norm_10_SHIFT));
		PHY_REG_MOD(pi, LCN40PHY, agcContro246, rssi_clip_gain_norm_11, clip_gains[11]);
		PHY_REG_MOD(pi, LCN40PHY, agcControl38, rssi_no_clip_gain_normal, clip_gains[12]);
	}

	if (aci_clip_threshs) {
		PHY_REG_MOD(pi, LCN40PHY, agcControl43,
			rssi_clip_thresh_aci_0, aci_clip_threshs[0]);
		phy_utils_mod_phyreg(pi, LCN40PHY_agcControl44,
			LCN40PHY_agcControl44_rssi_clip_thresh_aci_1_MASK |
			LCN40PHY_agcControl44_rssi_clip_thresh_aci_2_MASK,
			(aci_clip_threshs[1] <<
			LCN40PHY_agcControl44_rssi_clip_thresh_aci_1_SHIFT) |
			(aci_clip_threshs[2] <<
			LCN40PHY_agcControl44_rssi_clip_thresh_aci_2_SHIFT));
		phy_utils_mod_phyreg(pi, LCN40PHY_agcControl45,
			LCN40PHY_agcControl45_rssi_clip_thresh_aci_3_MASK |
			LCN40PHY_agcControl45_rssi_clip_thresh_aci_4_MASK,
			(aci_clip_threshs[3] <<
			LCN40PHY_agcControl45_rssi_clip_thresh_aci_3_SHIFT) |
			(aci_clip_threshs[4] <<
			LCN40PHY_agcControl45_rssi_clip_thresh_aci_4_SHIFT));
		phy_utils_mod_phyreg(pi, LCN40PHY_agcControl46,
			LCN40PHY_agcControl46_rssi_clip_thresh_aci_5_MASK |
			LCN40PHY_agcControl46_rssi_clip_thresh_aci_6_MASK,
			(aci_clip_threshs[5] <<
			LCN40PHY_agcControl46_rssi_clip_thresh_aci_5_SHIFT) |
			(aci_clip_threshs[6] <<
			LCN40PHY_agcControl46_rssi_clip_thresh_aci_6_SHIFT));
		phy_utils_mod_phyreg(pi, LCN40PHY_agcControl47,
			LCN40PHY_agcControl47_rssi_clip_thresh_aci_7_MASK |
			LCN40PHY_agcControl47_rssi_clip_thresh_aci_8_MASK,
			(aci_clip_threshs[7] <<
			LCN40PHY_agcControl47_rssi_clip_thresh_aci_7_SHIFT) |
			(aci_clip_threshs[8] <<
			LCN40PHY_agcControl47_rssi_clip_thresh_aci_8_SHIFT));
		phy_utils_mod_phyreg(pi, LCN40PHY_agcContro243,
			LCN40PHY_agcContro243_rssi_clip_thresh_aci_9_MASK |
			LCN40PHY_agcContro243_rssi_clip_thresh_aci_10_MASK,
			(clip_threshs[9] << LCN40PHY_agcContro243_rssi_clip_thresh_aci_9_SHIFT) |
			(clip_threshs[10] << LCN40PHY_agcContro243_rssi_clip_thresh_aci_10_SHIFT));
		PHY_REG_MOD(pi, LCN40PHY, agcContro244,
			rssi_clip_thresh_aci_11, aci_clip_threshs[11]);
	}
	if (clip_threshs) {
		phy_utils_mod_phyreg(pi, LCN40PHY_agcControl19,
			LCN40PHY_agcControl19_rssi_clip_thresh_norm_0_MASK |
			LCN40PHY_agcControl19_rssi_clip_thresh_norm_1_MASK,
			(clip_threshs[0] << LCN40PHY_agcControl19_rssi_clip_thresh_norm_0_SHIFT) |
			(clip_threshs[1] << LCN40PHY_agcControl19_rssi_clip_thresh_norm_1_SHIFT));
		phy_utils_mod_phyreg(pi, LCN40PHY_agcControl20,
			LCN40PHY_agcControl20_rssi_clip_thresh_norm_2_MASK |
			LCN40PHY_agcControl20_rssi_clip_thresh_norm_3_MASK,
			(clip_threshs[2] << LCN40PHY_agcControl20_rssi_clip_thresh_norm_2_SHIFT) |
			(clip_threshs[3] << LCN40PHY_agcControl20_rssi_clip_thresh_norm_3_SHIFT));
		phy_utils_mod_phyreg(pi, LCN40PHY_agcControl21,
			LCN40PHY_agcControl21_rssi_clip_thresh_norm_4_MASK |
			LCN40PHY_agcControl21_rssi_clip_thresh_norm_5_MASK,
			(clip_threshs[4] << LCN40PHY_agcControl21_rssi_clip_thresh_norm_4_SHIFT) |
			(clip_threshs[5] << LCN40PHY_agcControl21_rssi_clip_thresh_norm_5_SHIFT));
		phy_utils_mod_phyreg(pi, LCN40PHY_agcControl22,
			LCN40PHY_agcControl22_rssi_clip_thres_norm_6_MASK |
			LCN40PHY_agcControl22_rssi_clip_thresh_norm_7_MASK,
			(clip_threshs[6] << LCN40PHY_agcControl22_rssi_clip_thres_norm_6_SHIFT) |
			(clip_threshs[7] << LCN40PHY_agcControl22_rssi_clip_thresh_norm_7_SHIFT));
		phy_utils_mod_phyreg(pi, LCN40PHY_agcControl23,
			LCN40PHY_agcControl23_rssi_clip_thresh_norm_8_MASK |
			LCN40PHY_agcControl23_rssi_clip_thresh_norm_9_MASK,
			(clip_threshs[8] << LCN40PHY_agcControl23_rssi_clip_thresh_norm_8_SHIFT) |
			(clip_threshs[9] << LCN40PHY_agcControl23_rssi_clip_thresh_norm_9_SHIFT));
		phy_utils_mod_phyreg(pi, LCN40PHY_agcContro240,
			LCN40PHY_agcContro240_rssi_clip_thresh_norm_10_MASK |
			LCN40PHY_agcContro240_rssi_clip_thresh_norm_11_MASK,
			(clip_threshs[10] << LCN40PHY_agcContro240_rssi_clip_thresh_norm_10_SHIFT) |
			(clip_threshs[11] << LCN40PHY_agcContro240_rssi_clip_thresh_norm_11_SHIFT));
	}
	if (CHSPEC_IS2G(pi->radio_chanspec) && (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags)
		& BFL_EXTLNA) && CHSPEC_IS40(pi->radio_chanspec))
		PHY_REG_MOD(pi, LCN40PHY, agcControl21, rssi_clip_thresh_norm_5, 10);

	wlc_lcn40phy_agc_reset(pi);
	PHY_REG_MOD(pi, LCN40PHY, ClkEnCtrl, forcerxfrontendclk, 1);
}

/* CHANSPEC */
static void
wlc_lcn40phy_tx_vco_freq_divider(phy_info_t *pi, uint8 channel)
{
	uint freq;
	chan_info_2065_lcn40phy_t *chi = wlc_lcn40phy_find_channel(pi, channel);

	ASSERT(chi != NULL);

	freq = chi->freq;
	if (LCN40REV_GE(pi->pubpi.phy_rev, 4)) {
		/* 4334-B0
		The VCO frequency divider is the same in 40mhz bw mode and
		(20mhz bw mode with dac2x mode enabled)
		*/
		if (CHSPEC_IS40(pi->radio_chanspec) ||
			(CHSPEC_IS20(pi->radio_chanspec) &&
			(pi->u.pi_lcn40phy->dac2x_enable))) {
			/* 40 MHz bandwidth mode, or 20 MHz bandwidth
			     mode with dac2x mode enabled
			*/
			if (freq <= 2484 && freq >= 2412)
			/* VCO/9 */
				phy_utils_write_radioreg(pi, RADIO_2065_RFPLL_CFG2, 0x740);
			else if (freq <= 5330 && freq >= 4910)
			/* VCO/8 */
				phy_utils_write_radioreg(pi, RADIO_2065_RFPLL_CFG2, 0x700);
			else if ((LCN40REV_IS(pi->pubpi.phy_rev, 7)) && freq == 5805)
			/* VCO/5 */
				phy_utils_write_radioreg(pi, RADIO_2065_RFPLL_CFG2, 0x760);
			else if (freq <= 5835 && freq >= 5490)
			/* VCO/9 */
				phy_utils_write_radioreg(pi, RADIO_2065_RFPLL_CFG2, 0x740);
		} else {
			/* Regular 20 MHz bandwidth mode (dac2x mode is not enabled) */
			if (freq <= 2484 && freq >= 2412)
			/* VCO/18 */
				phy_utils_write_radioreg(pi, RADIO_2065_RFPLL_CFG2, 0x750);
			else if (freq <= 5320 && freq >= 4920)
			/* VCO/16 */
				phy_utils_write_radioreg(pi, RADIO_2065_RFPLL_CFG2, 0x710);
			else if (freq <= 5825 && freq >= 5500)
			/* VCO/18 */
				phy_utils_write_radioreg(pi, RADIO_2065_RFPLL_CFG2, 0x750);
		}
	} else {
		if (CHSPEC_IS20(pi->radio_chanspec)) {
			if (freq <= 2484 && freq >= 2412)
				/* VCO/18 */
				phy_utils_write_radioreg(pi, RADIO_2065_RFPLL_CFG2, 0x750);
			else if (freq <= 5320 && freq >= 4920)
				/* VCO/16 */
				phy_utils_write_radioreg(pi, RADIO_2065_RFPLL_CFG2, 0x710);
			else if (freq <= 5825 && freq >= 5500)
				/* VCO/18 */
				phy_utils_write_radioreg(pi, RADIO_2065_RFPLL_CFG2, 0x750);
		} else if (CHSPEC_IS40(pi->radio_chanspec)) {
			if (freq <= 2462 && freq >= 2422)
				/* VCO/9 */
				phy_utils_write_radioreg(pi, RADIO_2065_RFPLL_CFG2, 0x740);
			else if (freq <= 5310 && freq >= 4930)
				/* VCO/8 */
				phy_utils_write_radioreg(pi, RADIO_2065_RFPLL_CFG2, 0x700);
			else if (freq <= 5795 && freq >= 5510)
				/* VCO/9 */
				phy_utils_write_radioreg(pi, RADIO_2065_RFPLL_CFG2, 0x740);
		}
	}
}

static void wlc_lcn40phy_set_sfo_chan_centers(phy_info_t *pi, uint8 channel)
{
	uint freq;
	chan_info_2065_lcn40phy_t *chi = wlc_lcn40phy_find_channel(pi, channel);
	uint tmp;

	ASSERT(chi != NULL);

	freq = chi->freq;
	/* sfo_chan_center_Ts20 = round(fc / 20e6*(ten_mhz+1) * 8), fc in Hz
	*                      = round($channel * 0.4 *($ten_mhz+1)), $channel in MHz
	*/
	 tmp = (freq * 4 / 5 + 1) >> 1;
	PHY_REG_MOD(pi, LCN40PHY, ptcentreTs20, centreTs20, tmp);
	/* sfo_chan_center_factor = round(2^17 ./ (fc/20e6)/(ten_mhz+1)), fc in Hz
	*                        = round(2621440 ./ $channel/($ten_mhz+1)), $channel in MHz
	*/
	tmp = (2621440 * 2 / freq + 1) >> 1;
	PHY_REG_MOD(pi, LCN40PHY, ptcentreFactor, centreFactor, tmp);
}

static void
wlc_lcn40phy_tx_init(phy_info_t *pi, uint8 channel)
{
	phytbl_info_t tab;
	uint16 val = 0;
	int16 rfseq = 0;
	int16 ofdm_dig_type, cck_dig_type, ofdm40_dig_type;
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	phy_info_lcn40phy_t *pi_lcn40 = pi->u.pi_lcn40phy;

	wlc_lcn40phy_tx_farrow_init(pi, channel);
	if (LCN40REV_IS(pi->pubpi.phy_rev, 6) ||
		(LCN40REV_IS(pi->pubpi.phy_rev, 7) &&
		(RADIOVER(pi->pubpi.radiover) == 0x4))) {
#ifdef BAND5G
		if (CHSPEC_IS5G(pi->radio_chanspec)) {
			PHY_REG_MOD(pi, LCN40PHY, BphyControl4, bphyFrmTailCntValue, 511);
			PHY_REG_MOD(pi, LCN40PHY, iq_coeff_a_cck, frmTypBasedIqLoCoeff, 0);
			if (LCN40REV_IS(pi->pubpi.phy_rev, 7)) {
				PHY_REG_MOD(pi, LCN40PHY, Core1TxControl, loft_comp_shift, 0);
				phy_utils_write_phyreg(pi, LCN40PHY_papd_tx_temp_comp, 0x0);
			}
		} else
#endif /* BAND5G */
		{
			PHY_REG_MOD(pi, LCN40PHY, iq_coeff_a_cck, frmTypBasedIqLoCoeff, 1);
			if (LCN40REV_IS(pi->pubpi.phy_rev, 7)) {
				PHY_REG_MOD(pi, LCN40PHY, Core1TxControl, loft_comp_shift, 1);
				phy_utils_write_phyreg(pi, LCN40PHY_papd_tx_temp_comp, 0xff80);
				/* 43341 uses rfseqtbl.t_rx_cckdelay for ramp down failure */
				/* Roll back bphyFrmTailCntValue to 511 default */
				PHY_REG_MOD(pi, LCN40PHY, BphyControl4, bphyFrmTailCntValue, 511);
				tab.tbl_id = LCN40PHY_TBL_ID_RFSEQ;
				tab.tbl_width = 16;	/* 12 bit wide	*/
				tab.tbl_ptr = &rfseq;
				tab.tbl_len = 1;
				tab.tbl_offset = 13;
				rfseq = 4;
				wlc_lcnphy_write_table(pi,  &tab);
			} else if (LCN40REV_IS(pi->pubpi.phy_rev, 6)) {
				PHY_REG_LIST_START
					/* Roll back bphyFrmTailCntValue to 511 default */
					PHY_REG_MOD_ENTRY(LCN40PHY, BphyControl4,
						bphyFrmTailCntValue, 511)
					/* 43143 BPHY ramp down - reduce bphy tx crs */
					PHY_REG_MOD_ENTRY(LCN40PHY, BphyControl5,
						bphyCrsCntShtValue, 0x3E)
					PHY_REG_MOD_ENTRY(LCN40PHY, BphyControl5,
						bphyCrsCntLngValue, 0x3F)
				PHY_REG_LIST_EXECUTE(pi);
			}
		}
		PHY_REG_MOD(pi, LCN40PHY, iq_coeff_a_cck, serializedLoCoeff, 0);
	}
	PHY_REG_LIST_START
		/* On 4334-A0, need to swap I/Q rail. Swap I/Q at LOFTcomp output */
		PHY_REG_MOD_ENTRY(LCN40PHY, adcCompCtrl, flipiq_dacin, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, txTailCountValue, TailCountValue, 150)
		PHY_REG_MOD_ENTRY(LCN40PHY, txTailCountValue40MHZ, TailCountValue40Mhz, 100)
	PHY_REG_LIST_EXECUTE(pi);

	if (LCN40REV_IS(pi->pubpi.phy_rev, 1) || LCN40REV_IS(pi->pubpi.phy_rev, 3)) {
		PHY_REG_LIST_START
			/* 4314/43142 ptat register WAR to narrow power variation */
			RADIO_REG_MOD_ENTRY(RADIO_2065_PGA2G_CFG2, 0xff0, 0xff0)
			RADIO_REG_MOD_ENTRY(RADIO_2065_PGA2G_INCAP, 0x70, 0x0)
			RADIO_REG_MOD_ENTRY(RADIO_2065_PAD2G_SLOPE, 0x7ff, 0x7ff)
			RADIO_REG_MOD_ENTRY(RADIO_2065_PAD2G_INCAP, 0x7070, 0x0)
			/* PMOS compensation to improve linear power */
			RADIO_REG_MOD_ENTRY(RADIO_2065_PGA2G_INCAP, 0xf, 0x2)
			RADIO_REG_MOD_ENTRY(RADIO_2065_PAD2G_INCAP, 0xf0f, 0xe0e)
		PHY_REG_LIST_EXECUTE(pi);

		if (CHSPEC_IS20(pi->radio_chanspec)) {
			phy_utils_mod_radioreg(pi, RADIO_2065_PA2G_INCAP, 0xf0f, 0x404);
			phy_utils_mod_radioreg(pi, RADIO_2065_TXMIX2G_CFG1, 0xf000, 0x0);
		} else {
			phy_utils_mod_radioreg(pi, RADIO_2065_PA2G_INCAP, 0xf0f, 0x505);
			phy_utils_mod_radioreg(pi, RADIO_2065_TXMIX2G_CFG1, 0xf000, 0x4000);
		}
		PHY_REG_MOD(pi, LCN40PHY, txTailCountValue, TailCountValue, 0x45);
		PHY_REG_MOD(pi, LCN40PHY, txTailCountValue40MHZ, TailCountValue40Mhz, 0x29);
	}

	/* RFSEQ setting */
	if ((CHIPID(pi->sh->chip) == BCM4314_CHIP_ID ||
		CHIPID(pi->sh->chip) == BCM43142_CHIP_ID)) {
		/* reduce CCK LO by raising CCK pwrup for avoiding detector mis-trigger */
		tab.tbl_id = LCN40PHY_TBL_ID_RFSEQ;
		tab.tbl_width = 16;
		tab.tbl_ptr = &rfseq;
		tab.tbl_len = 1;
		tab.tbl_offset = 1;
		rfseq = 188;
		wlc_lcnphy_write_table(pi,  &tab);
	} else {
		tab.tbl_id = LCN40PHY_TBL_ID_RFSEQ;
		tab.tbl_width = 16;	/* 12 bit wide	*/
		tab.tbl_ptr = &rfseq;
		tab.tbl_len = 1;
		tab.tbl_offset = 7;
		rfseq = 15;
		wlc_lcnphy_write_table(pi,  &tab);
		tab.tbl_offset = 1;
		rfseq = 148;
		wlc_lcnphy_write_table(pi,  &tab);
	}

	if (LCN40REV_GE(pi->pubpi.phy_rev, 4)) {

		/* Allow dac2xmode only in 20 MHz bandwidth mode */
		if ((CHSPEC_IS20(pi->radio_chanspec)) &&
			(pi_lcn40->dac2x_enable)) {
			PHY_REG_MOD(pi, LCN40PHY, Core1TxControl, DAC2xModeEn, 1);
			phy_utils_mod_radioreg(pi, RADIO_2065_DAC_CFG1, 0x9000, 0x9000);
		} else if (CHSPEC_IS40(pi->radio_chanspec)) {
			PHY_REG_MOD(pi, LCN40PHY, Core1TxControl, DAC2xModeEn, 0);
			phy_utils_mod_radioreg(pi, RADIO_2065_DAC_CFG1, 0x9000, 0x1000);
		} else {
			PHY_REG_MOD(pi, LCN40PHY, Core1TxControl, DAC2xModeEn, 0);
			phy_utils_mod_radioreg(pi, RADIO_2065_DAC_CFG1, 0x9000, 0x0);
		}
		if (pi->tx_alpf_bypass) {
			PHY_REG_MOD(pi, LCN40PHY, radio_tx_cfg_pu, tx_lpf_byp_tx_ofdm, 1);
			if (LCN40REV_IS(pi->pubpi.phy_rev, 6) ||
				(LCN40REV_IS(pi->pubpi.phy_rev, 7) &&
				(RADIOVER(pi->pubpi.radiover) == 0x4))) {
				PHY_REG_MOD(pi, LCN40PHY, radio_tx_cfg_pu, tx_lpf_byp_tx_cck, 0);
			} else {
				PHY_REG_MOD(pi, LCN40PHY, radio_tx_cfg_pu, tx_lpf_byp_tx_cck, 1);
			}
			/* power down TX LPF */
			if (pi_lcn40->tx_alpf_pu) {
				PHY_REG_MOD(pi, LCN40PHY, radio_tx_cfg_pu, tx_lpf_pu, 3);
				wlc_lcn40phy_filt_bw_set(pi, 0);
				if (LCN40REV_IS(pi->pubpi.phy_rev, 6)) {
					/* 43143: set ALPF bandwidth to 1 for DSSS/CCK */
					PHY_REG_MOD(pi, LCN40PHY, lpfbwlutreg1, lpf_cck_tx_bw, 1);
				}
			} else {
				PHY_REG_MOD(pi, LCN40PHY, radio_tx_cfg_pu, tx_lpf_pu, 0);
			}
			/* Make sure LPF override is disabled */
			phy_utils_mod_radioreg(pi, RADIO_2065_OVR7, 0x10, 0x0);
		} else {
			PHY_REG_LIST_START
				PHY_REG_MOD_ENTRY(LCN40PHY, radio_tx_cfg_pu, tx_lpf_byp_tx_ofdm, 0)
				PHY_REG_MOD_ENTRY(LCN40PHY, radio_tx_cfg_pu, tx_lpf_byp_tx_cck, 0)
				/* power up TX LPF */
				PHY_REG_MOD_ENTRY(LCN40PHY, radio_tx_cfg_pu, tx_lpf_pu, 3)
			PHY_REG_LIST_EXECUTE(pi);
			if (CHSPEC_IS20(pi->radio_chanspec)) {
				if (CHSPEC_IS2G(pi->radio_chanspec))
					/* LPF bandwidth set to 25MHz */
					wlc_lcn40phy_filt_bw_set(pi, 3);
#ifdef BAND5G
				else
					/* LPF bandwidth set to 50MHz */
					wlc_lcn40phy_filt_bw_set(pi, 4);
#endif /* BAND5G */
				phy_utils_and_radioreg(pi, RADIO_2065_DAC_CFG1, ~0x1000);
			} else {
				phy_utils_or_radioreg(pi, RADIO_2065_DAC_CFG1, 0x1000);
				wlc_lcn40phy_filt_bw_set(pi, 4);
			}
		}

		/* To suppress DAC images further,
		  * select lowest frequency GM pole when bypassTxALPF_enable = 1
		  * and dac2x_enable = 0
		  */
		if ((pi->tx_alpf_bypass == 1) && (pi_lcn40->dac2x_enable == 0))
			phy_utils_write_radioreg(pi, RADIO_2065_TX_REG_BACKUP_2, 0xF0);
		/* for 43341, need to set bit 12 to 1 for 2G iTR switch */
		if (LCN40REV_IS(pi->pubpi.phy_rev, 7))
			phy_utils_or_radioreg(pi, RADIO_2065_TX_REG_BACKUP_2, 0x1000);
		/* for 4334-B0 only, increase idac scale for 5G band */
#ifdef BAND5G
		if (CHSPEC_IS5G(pi->radio_chanspec))
			phy_utils_or_radioreg(pi, RADIO_2065_TX_REG_BACKUP_2, 0x0F00);
#endif /* BAND5G */

		if (LCN40REV_IS(pi->pubpi.phy_rev, 5))
			phy_utils_or_radioreg(pi, RADIO_2065_TX_REG_BACKUP_2, 0x00F0);
	} else if (!(pi->tx_alpf_bypass)) {
		if (CHSPEC_IS20(pi->radio_chanspec)) {
			if ((LCN40REV_IS(pi->pubpi.phy_rev, 3)) &&
				(CHIPREV(pi->sh->chiprev) >= 1)) {
				wlc_lcn40phy_filt_bw_set(pi, 3);
				PHY_REG_MOD(pi, LCN40PHY, TxRealFrameDelay, realframedelay, 0x28);
				val = 4;
				tab.tbl_id = LCN40PHY_TBL_ID_RFSEQ;
				tab.tbl_width = 16;
				tab.tbl_ptr = &val;
				tab.tbl_len = 1;
				tab.tbl_offset = 5;
				wlc_lcnphy_write_table(pi,  &tab);
				val = 16;
				tab.tbl_ptr = &val;
				tab.tbl_offset = 7;
				wlc_lcnphy_write_table(pi,  &tab);
			} else
				wlc_lcn40phy_filt_bw_set(pi, 3);
			phy_utils_and_radioreg(pi, RADIO_2065_DAC_CFG1, ~0x1000);
		} else {
			phy_utils_or_radioreg(pi, RADIO_2065_DAC_CFG1, 0x1000);
			wlc_lcn40phy_filt_bw_set(pi, 4);
		}
	}
	if (CHSPEC_IS20(pi->radio_chanspec)) {
		ofdm40_dig_type = 0;
#ifdef BAND5G
		if (CHSPEC_IS5G(pi->radio_chanspec))
			ofdm_dig_type = pi_lcn->lcnphy_ofdm_dig_filt_type_5g;
		else
#endif /* BAND5G */
			ofdm_dig_type = pi_lcn->lcnphy_ofdm_dig_filt_type_2g;

		if (PHY_EPA_SUPPORT(pi_lcn->ePA)) {
			cck_dig_type = pi_lcn->lcnphy_cck_dig_filt_type;
			if (cck_dig_type == -1)
				cck_dig_type = 21;
			if (channel == 14)
				cck_dig_type = 31;
			if (ofdm_dig_type == -1)
				ofdm_dig_type = 0;
		} else {
			if (ofdm_dig_type == -1) {
				if (LCN40REV_IS(pi->pubpi.phy_rev, 7))
					ofdm_dig_type = 6;
				else
					ofdm_dig_type = 2;
			}
			if (channel == 14)
				cck_dig_type = 31;
			else if ((LCN40REV_IS(pi->pubpi.phy_rev, 7)) &&
				(RADIOVER(pi->pubpi.radiover) == 0x4))
				cck_dig_type = 28;
			else
				cck_dig_type = 21;
		}
	} else {
		ofdm_dig_type = 5;
		cck_dig_type = 50;
#ifdef BAND5G
		if (CHSPEC_IS5G(pi->radio_chanspec))
			ofdm40_dig_type = pi_lcn40->ofdm40_dig_filt_type_5g;
		else
#endif /* BAND5G */
			ofdm40_dig_type = pi_lcn40->ofdm40_dig_filt_type_2g;
		if (PHY_EPA_SUPPORT(pi_lcn->ePA)) {
			ofdm40_dig_type =
				(ofdm40_dig_type < 0) ?
				0 : ofdm40_dig_type;
		} else if (LCN40REV_IS(pi->pubpi.phy_rev, 6)) {
			ofdm40_dig_type =
				(ofdm40_dig_type < 0) ?
				7 : ofdm40_dig_type;
		} else {
			ofdm40_dig_type =
				(ofdm40_dig_type < 0) ?
				3 : ofdm40_dig_type;
		}
	}
	wlc_lcn40phy_load_tx_iir_filter(pi, TX_IIR_FILTER_CCK, cck_dig_type);
	wlc_lcn40phy_load_tx_iir_filter(pi, TX_IIR_FILTER_OFDM, ofdm_dig_type);
	wlc_lcn40phy_load_tx_iir_filter(pi, TX_IIR_FILTER_OFDM40, ofdm40_dig_type);

	pi_lcn40->tx_iir_filter_type_ofdm = ofdm_dig_type;
	pi_lcn40->tx_iir_filter_type_ofdm40 = ofdm40_dig_type;
	pi_lcn40->tx_iir_filter_type_cck = cck_dig_type;

	PHY_REG_MOD(pi, LCN40PHY, lpphyCtrl, txfiltSelect, 2);
	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		phy_utils_and_radioreg(pi, RADIO_2065_OVR13, ~0x4);
		phy_utils_and_radioreg(pi, RADIO_2065_OVR8, ~0x1);
		/* Explicitly set to reset values to have consistent settings whenever in G-band */
		if (LCN40REV_IS(pi->pubpi.phy_rev, 7))
			phy_utils_mod_radioreg(pi, RADIO_2065_TXGM_CFG1,
			                       ((3<<12)|(7<<8)), ((1<<12)|(0<<8)));
		else if (!LCN40REV_IS(pi->pubpi.phy_rev, 6))
			phy_utils_mod_radioreg(pi, RADIO_2065_TXGM_CFG1,
			                       ((3<<12)|(7<<8)), ((2<<12)|(3<<8)));
	}
#ifdef BAND5G
	if (CHSPEC_IS5G(pi->radio_chanspec)) {
		if (LCN40REV_IS(pi->pubpi.phy_rev, 7)) {
			if (RADIOVER(pi->pubpi.radiover) < 0x4) {
				uint freq =
					phy_utils_channel2freq(CHSPEC_CHANNEL(pi->radio_chanspec));

				if (freq <= 5320)
					phy_utils_mod_radioreg(pi, RADIO_2065_LOGEN5G_TUNE,
					                       0xf0, 0x70);
				else
					phy_utils_mod_radioreg(pi, RADIO_2065_LOGEN5G_TUNE,
					                       0xf0, 0);
			}
			if (!PHY_EPA_SUPPORT(pi_lcn->ePA)) {
				PHY_REG_LIST_START
				/* Radio WAR to boost 43341 5G Pout */
					RADIO_REG_MOD_ENTRY(RADIO_2065_TXMIX5G_CFG1, 0xf0, 0xf0)
					RADIO_REG_MOD_ENTRY(RADIO_2065_OVR13, 0x4, 1 << 2)
					RADIO_REG_MOD_ENTRY(RADIO_2065_TXMIX5G_CFG1, 0xf000,
					5 << 12)
					RADIO_REG_MOD_ENTRY(RADIO_2065_OVR8, 0x1, 1)
					RADIO_REG_MOD_ENTRY(RADIO_2065_PGA5G_CFG1, 0xf000, 5 << 12)
				PHY_REG_LIST_EXECUTE(pi);
			}
		} else {
			PHY_REG_LIST_START
				/* 4334A0 logen5g idac currents */
				RADIO_REG_WRITE_ENTRY(RADIO_2065_LOGEN5G_IDAC1, 0x7332)
				RADIO_REG_WRITE_ENTRY(RADIO_2065_LOGEN5G_IDAC2, 0x2224)
				RADIO_REG_WRITE_ENTRY(RADIO_2065_LOGEN5G_IDAC3, 0x7744)
				RADIO_REG_MOD_ENTRY(RADIO_2065_TXGM_CFG1, 0x3700, 0)
				/* Settings from Ali K, to lower VCO related spurs */
				RADIO_REG_OR_ENTRY(RADIO_2065_OVR13, 0x4)
				RADIO_REG_OR_ENTRY(RADIO_2065_TXMIX5G_CFG1, 0xf000)
				RADIO_REG_OR_ENTRY(RADIO_2065_OVR8, 0x1)
				RADIO_REG_OR_ENTRY(RADIO_2065_PGA5G_CFG1, 0xf000)
				RADIO_REG_MOD_ENTRY(RADIO_2065_LOGEN5G_IDAC2, 0x7000, 0x7000)
				RADIO_REG_MOD_ENTRY(RADIO_2065_LOGEN5G_IDAC2, 0x0070, 0)
				RADIO_REG_MOD_ENTRY(RADIO_2065_LOGEN5G_IDAC1, 0x7, 3)
				/* Disable txmix5g_gainboost to improve 5G EVM floor
				* (need to re-check VCO spurs)
				*/
				RADIO_REG_MOD_ENTRY(RADIO_2065_TXMIX5G_CFG1, 0xf000, 0)
			PHY_REG_LIST_EXECUTE(pi);

			if (pi_lcn40->mixboost_5g)
				phy_utils_mod_radioreg(pi, RADIO_2065_TXMIX5G_CFG1, 0xf000, 0xf000);
			if (pi_lcn40->padbias5g != -1) {
				phy_utils_mod_radioreg(pi, RADIO_2065_PAD5G_IDAC,
				                       0x3f, pi_lcn40->padbias5g);
				phy_utils_mod_radioreg(pi, RADIO_2065_PAD5G_TUNE, 0x3f00,
					(pi_lcn40->padbias5g << 8));
			}
		}

		/* pad5g slope set to max value to increase the gain in A band  */
		phy_utils_mod_radioreg(pi, RADIO_2065_PAD5G_SLOPE, 0x00ff, 0x00ff);
	}
#endif /* BAND5G */
	if (LCN40REV_IS(pi->pubpi.phy_rev, 6)) {
		/* needed for CCK ACPR, reduce digi gain by 4.5 dB */
		PHY_REG_MOD(pi, LCN40PHY, BphyControl3, bphyScale, 0x10);
		PHY_REG_MOD(pi, LCN40PHY, classifierCtrl, macint_crs_bypass, 1);
		/* Turn off DAC scrambler to avoid B-PHY spectral mask violation by 25MHz spurs */
		phy_utils_mod_radioreg(pi, RADIO_2065_DAC_CFG1, 0x0300, 0x0300);
	}
	phy_utils_or_radioreg(pi, RADIO_2065_BG_CFG1, 2);
}

static void
wlc_phy_chanspec_set_lcn40phy(phy_info_t *pi, chanspec_t chanspec)
{
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	phy_info_lcn40phy_t *pi_lcn40 = pi->u.pi_lcn40phy;
	uint8 channel = CHSPEC_CHANNEL(chanspec); /* see wlioctl.h */
#if defined(PHYCAL_CACHING)
	ch_calcache_t *ctx = wlc_phy_get_chanctx(pi, chanspec);
#endif // endif

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	/* WAR to improve EVM for iboards */
	if ((BOARDTYPE(pi->sh->boardtype) == BCM94334FCAGBI_SSID) ||
		(BOARDTYPE(pi->sh->boardtype) == BCM94334WLAGBI_SSID) ||
		(BOARDTYPE(pi->sh->boardtype) == BCM943342FCAGBI_SSID)) {
		if ((channel == 136) || (channel == 161)) {
			pi_lcn40->dac2x_enable = 0;
		} else {
			pi_lcn40->dac2x_enable = pi_lcn40->dac2x_enable_nvm;
		}
	}
	if ((LCN40REV_IS(pi->pubpi.phy_rev, 7)) &&
		(RADIOVER(pi->pubpi.radiover) == 0x4)) {
		if (CHSPEC_CHANNEL(pi->radio_chanspec) == 165) {
			PHY_REG_MOD(pi, LCN40PHY, CFOblkConfig, c_crs_cfo_calc_en_40mhz, 0);
			PHY_REG_MOD(pi, LCN40PHY, CFOblkConfig, c_crs_cfo_update_en_40mhz, 1);
		} else {
			PHY_REG_MOD(pi, LCN40PHY, CFOblkConfig, c_crs_cfo_calc_en_40mhz, 1);
			PHY_REG_MOD(pi, LCN40PHY, CFOblkConfig, c_crs_cfo_update_en_40mhz, 1);
		}
	}

	wlc_phy_chanspec_radio_set((wlc_phy_t *)pi, chanspec);
	/* Set the phy bandwidth as dictated by the chanspec */
	if (CHSPEC_BW(chanspec) != pi->bw)
		wlapi_bmac_bw_set(pi->sh->physhim, CHSPEC_BW(chanspec));

	/* Check channel-dependent PAPD enabling for 2G
	  * (LSB corresponds to chan 1 and so forth).
	  * If exists, 'papden2gchan' overrides 'papden2g'
	  * on a per-channel basis.
	  */
	if ((pi_lcn40->papden2gchan != -1) && CHSPEC_IS2G(chanspec)) {
		uint32 chmask = 1 << (channel - 1);

		if (chmask & pi_lcn40->papden2gchan)
			pi_lcn40->papd_enable = 1;
		else
			pi_lcn40->papd_enable = 0;
	}

	wlc_lcn40phy_set_sfo_chan_centers(pi, channel);
	/* spur and stuff */
	wlc_lcn40phy_set_chanspec_tweaks(pi, pi->radio_chanspec);

	/* lcn40phy_agc_reset */
	wlc_lcn40phy_agc_reset(pi);

	/* Tune radio for the channel */
	if (!NORADIO_ENAB(pi->pubpi)) {
		if (CHIPID(pi->sh->chip) == BCM43143_CHIP_ID)
			wlc_lcn40phy_radio_2065_channel_tune_new(pi, channel);
		else
			wlc_lcn40phy_radio_2065_channel_tune(pi, channel);
	} else
		return;

	/* Reset phy after changing channel. */
	wlc_lcn40phy_agc_reset(pi);

	wlc_lcn40phy_tx_vco_freq_divider(pi, channel);
	wlc_lcn40phy_tx_init(pi, channel);
	wlc_lcn40phy_rx_farrow_init(pi, channel);
	/* check for vco cal done here */
	SPINWAIT(!wlc_radio_2065_vco_cal_done(pi), 100);
	if (!wlc_radio_2065_vco_cal_done(pi))
		PHY_ERROR(("wl%d: %s: Radio VCO CAL failed\n", pi->sh->unit, __FUNCTION__));

	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		PHY_REG_MOD(pi, LCN40PHY, rfoverride2, slna_pu_ovr, 0);
	}
#ifdef BAND5G
	else {
		PHY_REG_MOD(pi, LCN40PHY, rfoverride2, slna_pu_ovr, 1);
		PHY_REG_MOD(pi, LCN40PHY, rfoverride2val, slna_pu_ovr_val, 1);
	}
#endif /* BAND5G */
	wlc_lcn40phy_noise_measure_stop(pi);

	/* Force vbat to updated the cached tempsense values */
	wlc_lcn40phy_tempsense(pi, TEMPER_VBAT_TRIGGER_NEW_MEAS);

	/* Perform Tx IQ, Rx IQ and PAPD cal only if no scan in progress */
	if (!wlc_phy_no_cal_possible(pi))
	{
#if defined(PHYCAL_CACHING)
		/* Fresh calibration or restoration required */
		if (!ctx) {
			if (LCN40PHY_MAX_CAL_CACHE <= pi->phy_calcache_num) {
				/* Already max num ctx exist, reuse oldest */
				ctx = wlc_phy_get_chanctx_oldest(pi);
				ASSERT(ctx);
				wlc_phy_reinit_chanctx(pi, ctx, chanspec);
			} else {
				/* Prepare a fresh calibration context */
				if (BCME_OK == wlc_phy_create_chanctx((wlc_phy_t *)pi,
					pi->radio_chanspec)) {
					ctx = pi->phy_calcache;
				}
			}
		}
#endif /* PHYCAL_CACHING */

#if defined(PHYCAL_CACHING)
		if (!ctx || !ctx->valid)
#else
		if ((pi_lcn->lcnphy_full_cal_channel != CHSPEC_CHANNEL(pi->radio_chanspec)))
#endif // endif
			wlc_lcn40phy_calib_modes(pi, PHY_FULLCAL);
		else
			wlc_lcn40phy_restore_calibration_results(pi);
	} else { /* wlc_phy_no_cal_possible(pi) */
#if defined(PHYCAL_CACHING)
		if (ctx) {
			if (ctx->valid)
#else
		/* If they are in same band, apply the stored cal resutls and enable cals,
		other wise disable cals
		*/
		{
		if (((pi_lcn->lcnphy_full_cal_channel > 14) &&
			(CHSPEC_CHANNEL(pi->radio_chanspec) > 14)) ||
			((pi_lcn->lcnphy_full_cal_channel <= 14) &&
			(CHSPEC_CHANNEL(pi->radio_chanspec) <= 14)))
#endif /* PHYCAL_CACHING */
			wlc_lcn40phy_papd_txiqlo_rxiq_enable(pi, 1, chanspec);
		else
			wlc_lcn40phy_papd_txiqlo_rxiq_enable(pi, 0, chanspec);
		}
	}

	wlc_lcn40phy_agc_tweaks(pi);

	wlc_lcn40phy_noise_set_input_pwr_offset(pi);

	wlc_lcn40phy_noise_measure_start(pi, TRUE);

	if (pi_lcn->txpwr_clamp_dis || pi_lcn->txpwr_tssioffset_clamp_dis) {
		pi_lcn->tssi_maxpwr_limit = 0x7fffffff;
		pi_lcn->tssi_minpwr_limit = 0xffffffff;
	}
	else if ((pi->hwpwrctrl_capable) && (!pi_lcn->txpwr_tssioffset_clamp_dis))
		wlc_lcn40phy_set_tssi_pwr_limit(pi, PHY_TSSI_SET_MIN_MAX_LIMIT);

	if (((CHIPID(pi->sh->chip) == BCM4334_CHIP_ID) &&
		(LCN40REV_GE(pi->pubpi.phy_rev, 4))) ||
		(CHIPID(pi->sh->chip) == BCM43342_CHIP_ID) ||
		(CHIPID(pi->sh->chip) == BCM43341_CHIP_ID))
		wlc_lcn40phy_aci_init(pi);
	if (LCN40REV_IS(pi->pubpi.phy_rev, 7) && (PHY_XTALFREQ(pi->xtalfreq) / 1000 == 37400))
		wlc_lcn40_spur_war(pi, channel);

	PHY_REG_LIST_START
		PHY_REG_MOD_ENTRY(LCN40PHY, RFinputOverrideVal, BTPriority_ovr_val, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, agcControl4, c_agc_fsm_en, 0)
		PHY_REG_MOD_ENTRY(LCN40PHY, agcControl4, c_agc_fsm_en, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, agcControl4, c_agc_fsm_en, 0)
		PHY_REG_MOD_ENTRY(LCN40PHY, agcControl4, c_agc_fsm_en, 1)

		/* The below reg settings are required for the digi gain fix in the ucode */
		PHY_REG_MOD_ENTRY(LCN40PHY, RFinputOverrideVal, BTPriority_ovr_val, 0)
		PHY_REG_MOD_ENTRY(LCN40PHY, RFinputOverride, BTPriority_ovr, 1)
	PHY_REG_LIST_EXECUTE(pi);

	wlc_lcn40phy_trigger_noise_iqest(pi);
}

static void
wlc_lcn40_spur_war(phy_info_t *pi, uint8 channel)
{
	if (CHSPEC_IS20(pi->radio_chanspec)) {
		switch (channel) {
			case 3:
				PHY_REG_LIST_START
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkCtrl, scaler, 15)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkCtrl, NF1En, 1)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaRe, man, -121)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaRe, exp, 0)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaIm, man, 79)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaIm, exp, 1)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaRe, man, -95)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaRe, exp, 2)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaIm, man, 123)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaIm, exp, 4)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkCtrl, NF2En, 0)
				PHY_REG_LIST_EXECUTE(pi);
			break;
			case 4:
				PHY_REG_LIST_START
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkCtrl, scaler, 15)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkCtrl, NF1En, 1)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaRe, man, 79)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaRe, exp, 1)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaIm, man, 121)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaIm, exp, 0)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaRe, man, 123)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaRe, exp, 4)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaIm, man, 95)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaIm, exp, 2)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkCtrl, NF2En, 0)
				PHY_REG_LIST_EXECUTE(pi);
			break;
			case 5:
			/* Notch depth= -10dB */
				PHY_REG_LIST_START
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkCtrl, scaler, 15)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkCtrl, NF1En, 1)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaRe, man, 114)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaRe, exp, 0)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaIm, man, -74)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaIm, exp, 1)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaRe, man, 86)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaRe, exp, 2)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaIm, man, -112)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaIm, exp, 4)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkCtrl, NF2En, 0)
				PHY_REG_LIST_EXECUTE(pi);
			break;
			case 6:
				PHY_REG_LIST_START
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkCtrl, scaler, 15)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkCtrl, NF1En, 1)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaRe, man, -79)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaRe, exp, 1)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaIm, man, -121)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaIm, exp, 0)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaRe, man, -123)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaRe, exp, 4)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaIm, man, -95)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaIm, exp, 2)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkCtrl, NF2En, 0)
				PHY_REG_LIST_EXECUTE(pi);
			break;
			case 11:
				PHY_REG_LIST_START
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkCtrl, scaler, 15)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkCtrl, NF1En, 1)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaRe, man, -108)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaRe, exp, 1)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaIm, man, 115)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaIm, exp, 0)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaRe, man, -85)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaRe, exp, 3)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaIm, man, 90)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaIm, exp, 2)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkCtrl, NF2En, 0)
				PHY_REG_LIST_EXECUTE(pi);
			break;
			case 12:
				PHY_REG_LIST_START
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkCtrl, scaler, 15)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkCtrl, NF1En, 1)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaRe, man, 115)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaRe, exp, 0)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaIm, man, 108)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaIm, exp, 1)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaRe, man, 90)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaRe, exp, 2)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaIm, man, 85)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaIm, exp, 3)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkCtrl, NF2En, 0)
				PHY_REG_LIST_EXECUTE(pi);
			break;
			case 13:
				PHY_REG_LIST_START
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkCtrl, scaler, 15)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkCtrl, NF1En, 1)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaRe, man, 108)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaRe, exp, 1)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaIm, man, -115)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaIm, exp, 0)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaRe, man, 85)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaRe, exp, 3)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaIm, man, -90)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaIm, exp, 2)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkCtrl, NF2En, 0)
				PHY_REG_LIST_EXECUTE(pi);
			break;
			case 14:
				PHY_REG_LIST_START
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkCtrl, scaler, 15)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkCtrl, NF1En, 1)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaRe, man, -115)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaRe, exp, 0)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaIm, man, -108)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaIm, exp, 1)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaRe, man, -90)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaRe, exp, 2)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaIm, man, -85)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaIm, exp, 3)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkCtrl, NF2En, 0)
				PHY_REG_LIST_EXECUTE(pi);
			break;
#ifdef BAND5G
			case 56:
				PHY_REG_LIST_START
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkCtrl, scaler, 15)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkCtrl, NF1En, 1)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaRe, man, -118)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaRe, exp, 1)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaIm, man, -107)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaIm, exp, 0)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaRe, man, -88)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaRe, exp, 4)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaIm, man, -80)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaIm, exp, 3)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkCtrl, NF2En, 0)
				PHY_REG_LIST_EXECUTE(pi);
			break;
			case 100:
				PHY_REG_LIST_START
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkCtrl, scaler, 15)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkCtrl, NF1En, 1)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaRe, man, 94)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaRe, exp, 0)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaIm, man, -78)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaIm, exp, 0)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaRe, man, 70)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaRe, exp, 3)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaIm, man, -117)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaIm, exp, 4)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkCtrl, NF2En, 0)
				PHY_REG_LIST_EXECUTE(pi);
			break;
			case 128:
				PHY_REG_LIST_START
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkCtrl, scaler, 15)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkCtrl, NF1En, 1)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaRe, man, -84)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaRe, exp, 0)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaIm, man, 89)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaIm, exp, 0)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaRe, man, -125)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaRe, exp, 4)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaIm, man, 67)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaIm, exp, 3)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkCtrl, NF2En, 0)
				PHY_REG_LIST_EXECUTE(pi);
			break;
			case 161:
				PHY_REG_LIST_START
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkCtrl, scaler, 15)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkCtrl, NF1En, 1)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaRe, man, -100)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaRe, exp, 0)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaIm, man, -73)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaIm, exp, 0)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaRe, man, -75)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaRe, exp, 3)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaIm, man, -109)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaIm, exp, 4)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkCtrl, NF2En, 0)
				PHY_REG_LIST_EXECUTE(pi);
			break;
#endif /* BAND5G */
			default:
				PHY_REG_LIST_START
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkCtrl, scaler, 0)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkCtrl, NF1En, 0)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaRe, man, 0)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaRe, exp, 0)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaIm, man, 0)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaIm, exp, 0)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaRe, man, 0)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaRe, exp, 0)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaIm, man, 0)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaIm, exp, 0)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkCtrl, NF2En, 0)
				PHY_REG_LIST_EXECUTE(pi);
			break;
		}
	} else {
		/* 40MHz */
		switch (channel) {
			case 3:
				PHY_REG_LIST_START
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkCtrl, scaler, 15)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkCtrl, NF1En, 1)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaRe, man, 78)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaRe, exp, 2)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaIm, man, 123)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaIm, exp, 0)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaRe, man, 117)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaRe, exp, 7)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaIm, man, 92)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaIm, exp, 4)
				PHY_REG_LIST_EXECUTE(pi);
			break;
			case 4:
				PHY_REG_LIST_START
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkCtrl, scaler, 15)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkCtrl, NF1En, 1)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaRe, man, 101)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaRe, exp, 0)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaIm, man, 73)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaIm, exp, 0)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaRe, man, 76)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaRe, exp, 4)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaIm, man, 110)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaIm, exp, 5)
				PHY_REG_LIST_EXECUTE(pi);
			break;
			case 5:
				PHY_REG_LIST_START
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkCtrl, scaler, 15)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkCtrl, NF1En, 1)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaRe, man, 123)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaRe, exp, 0)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaIm, man, -78)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaIm, exp, 2)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaRe, man, 92)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaRe, exp, 4)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaIm, man, -117)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaIm, exp, 7)
				PHY_REG_LIST_EXECUTE(pi);
			break;
			case 6:
				PHY_REG_LIST_START
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkCtrl, scaler, 15)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkCtrl, NF1En, 1)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaRe, man, 73)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaRe, exp, 0)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaIm, man, -101)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaIm, exp, 0)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaRe, man, 110)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaRe, exp, 5)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaIm, man, -76)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaIm, exp, 4)
				PHY_REG_LIST_EXECUTE(pi);
			break;
			case 7:
				PHY_REG_LIST_START
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkCtrl, scaler, 15)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkCtrl, NF1En, 1)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaRe, man, -78)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaRe, exp, 2)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaIm, man, -123)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaIm, exp, 0)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaRe, man, -117)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaRe, exp, 7)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaIm, man, -92)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaIm, exp, 4)

				PHY_REG_LIST_EXECUTE(pi);
			break;
			case 8:
				PHY_REG_LIST_START
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkCtrl, scaler, 15)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkCtrl, NF1En, 1)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaRe, man, -101)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaRe, exp, 0)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaIm, man, -73)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaIm, exp, 0)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaRe, man, -76)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaRe, exp, 4)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaIm, man, -110)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaIm, exp, 5)
				PHY_REG_LIST_EXECUTE(pi);
			break;
			default:
				PHY_REG_LIST_START
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkCtrl, scaler, 0)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkCtrl, NF1En, 0)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaRe, man, 0)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaRe, exp, 0)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaIm, man, 0)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1AlphaIm, exp, 0)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaRe, man, 0)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaRe, exp, 0)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaIm, man, 0)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkNF1GammaIm, exp, 0)
					PHY_REG_MOD_ENTRY(LCN40PHY, RxSpurBlkCtrl, NF2En, 0)
				PHY_REG_LIST_EXECUTE(pi);
			break;
		}
	}
}

static chan_info_2065_lcn40phy_t *
wlc_lcn40phy_find_channel(phy_info_t *pi, uint8 channel)
{
	uint i, length = 0;
	chan_info_2065_lcn40phy_t *chan_info = NULL;

	if (RADIOID(pi->pubpi.radioid) == BCM2067_ID) {
		if (LCN40REV_IS(pi->pubpi.phy_rev, 7)) {
			if (RADIOVER(pi->pubpi.radiover) < 0x4) {
				length = ARRAYSIZE(chan_info_2067_rev60_lcn40phy);
				chan_info = chan_info_2067_rev60_lcn40phy;
			} else {
				length = ARRAYSIZE(chan_info_2067_rev64_lcn40phy);
				chan_info = chan_info_2067_rev64_lcn40phy;
			}
		}
		else {
				length = ARRAYSIZE(chan_info_2067_lcn40phy);
				chan_info = chan_info_2067_lcn40phy;
		}
	} else if (RADIOID(pi->pubpi.radioid) == BCM2065_ID) {
		if (RADIOVER(pi->pubpi.radiover) == 2) {
			length = ARRAYSIZE(chan_info_2065_rev2_lcn40phy);
			chan_info = chan_info_2065_rev2_lcn40phy;
		} else {
			length = ARRAYSIZE(chan_info_2065_lcn40phy);
			chan_info = chan_info_2065_lcn40phy;
		}
	} else {
		length = ARRAYSIZE(chan_info_2067_lcn40phy);
		chan_info = chan_info_2067_lcn40phy;
	}

	/* find freqency given channel */
	for (i = 0; i < length; i++)
		if (chan_info[i].chan == channel) {
			return &chan_info[i];
		}

	PHY_ERROR(("wl%d: %s: Bad channel %d\n", pi->sh->unit, __FUNCTION__, channel));
	if (channel < 15)
		return &chan_info[0];
	else
		return &chan_info[15];
}

static void
wlc_lcn40phy_tx_farrow_init(phy_info_t *pi, uint8 channel)
{
	int freq = 0;
	int fi = 0;

	chan_info_2065_lcn40phy_t *chi = wlc_lcn40phy_find_channel(pi, channel);
	ASSERT(chi != NULL);

	freq = chi->freq;

	if (CHSPEC_IS20(pi->radio_chanspec)) {
		if (freq <= 2484 && freq >= 2412)
			fi = 1920;
		else if (freq <= 5320 && freq >= 4920)
			fi = 3840;
		else if ((LCN40REV_IS(pi->pubpi.phy_rev, 7)) && freq == 5805)
			fi = 4800;
		else if (freq <= 5825 && freq >= 5500)
			fi = 4320;
	} else {
		if (freq <= 2462 && freq >= 2422)
			fi = 1920;
		else if (freq <= 5310 && freq >= 4930)
			fi = 3840;
		else if (freq <= 5795 && freq >= 5510)
			fi = 4320;
	}
	ASSERT(fi != 0);
	if (LCN40REV_GE(pi->pubpi.phy_rev, 4)) {
		uint32 mu_delta;
		if (freq <= 2484 && freq >= 2412)
			mu_delta = mu_deltaLUT[chi->chan - 1];
		else
		mu_delta = (((((((uint32)(fi/10) << 23) / (uint32)freq) * 10) + 1) >> 1)
			- (1 << 22));
		phy_utils_write_phyreg(pi, LCN40PHY_tx_resampler1_high,
		                       ((mu_delta >> 16) & 0x3f) + 4096);
		phy_utils_write_phyreg(pi, LCN40PHY_tx_resampler1, (mu_delta & 0xffff));
	} else {
	/* rounding here */
		phy_utils_write_phyreg(pi, LCN40PHY_tx_resampler1,
		(uint16)((((fi << 16) / freq + 1) >> 1) - (1 << 15)));
	}
	phy_utils_write_phyreg(pi, LCN40PHY_tx_resampler3, 0);
	phy_utils_write_phyreg(pi, LCN40PHY_tx_resampler4, freq / wlc_phy_gcd(freq, fi));
}

static void
wlc_lcn40phy_rx_farrow_init(phy_info_t *pi, uint8 channel)
{
	uint32 freq = 0, farrow_mu, diff, highest_period, GCD, rxperiod;

	chan_info_2065_lcn40phy_t *chi = wlc_lcn40phy_find_channel(pi, channel);
	ASSERT(chi != NULL);

	freq = chi->freq;

	if (freq < 3500) {
		/* band G */
		highest_period = 1920;
		/* need rounding here */
		farrow_mu = (((freq << 9) / 3 / 5 + 1) >> 1) - (1 << 15);
	} else if (freq <= 5320) {
		highest_period = 3840;
		farrow_mu = (((freq << 8) / 3 / 5 + 1) >> 1) - (1 << 15);
	} else if ((LCN40REV_IS(pi->pubpi.phy_rev, 7)) && freq == 5805) {
		highest_period = 4800;
		farrow_mu = 6861;
	} else {
		highest_period = 4320;
		farrow_mu = (((freq << 11) / 27 / 5 + 1) >> 1) - (1 << 15);
	}

	diff = freq - highest_period;
	GCD = wlc_phy_gcd(highest_period, diff);
	rxperiod = highest_period / GCD;

	PHY_REG_MOD(pi, LCN40PHY, rxFarrowDriftPeriod,
		rx_farrow_drift_period, rxperiod);
	PHY_REG_MOD(pi, LCN40PHY, rxFarrowDeltaPhase,
		rx_farrow_mu_delta, farrow_mu);

	PHY_REG_LIST_START
		PHY_REG_MOD_ENTRY(LCN40PHY, agcControl27, data_mode_gain_db_adj_dsss, 0)

		/* Keep the resistors in the dc loop: */
		PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride4, lpf_byp_dc_ovr, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride4val, lpf_byp_dc_ovr_val, 0)
	/* WAR for rxper degradation when ack is enabled issue, transient caused by lpf dc loop */
		PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride4, lpf_dc1_pwrup_ovr, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride4val, lpf_dc1_pwrup_ovr_val, 1)
	PHY_REG_LIST_EXECUTE(pi);

	if (LCN40REV_IS(pi->pubpi.phy_rev, 1) || LCN40REV_IS(pi->pubpi.phy_rev, 3)) {
		PHY_REG_LIST_START
			PHY_REG_MOD_ENTRY(LCN40PHY, PwrDlyAdj0_new, ofdmfiltDlyAdjustment20, -1)
			PHY_REG_MOD_ENTRY(LCN40PHY, agcControl, ofdm_sync_nom_en, 1)
			PHY_REG_MOD_ENTRY(LCN40PHY, agcControl18, rssi_clip_gain_norm_8, 3)
			PHY_REG_MOD_ENTRY(LCN40PHY, radioTRCtrl, gainrequestTRAttnOnEn, 1)
			PHY_REG_MOD_ENTRY(LCN40PHY, agcControl8, div_check_cnt, 1)
			PHY_REG_MOD_ENTRY(LCN40PHY, radioTRCtrlCrs1, gainReqTrAttOnEnByCrs, 1)
			PHY_REG_MOD_ENTRY(LCN40PHY, agcControl, pktRxActiveHoldoff, 10)
			/* CRLCNPHY-650, WAR to OFDM pkt loss preceded by a reset->CCK sequence */
			PHY_REG_MOD_ENTRY(LCN40PHY, crsMiscCtrl1_new, lp_adj_20L, 3)
			PHY_REG_MOD_ENTRY(LCN40PHY, crsMiscCtrl1_new, lp_adj_20U, 5)
			PHY_REG_MOD_ENTRY(LCN40PHY, crsMiscCtrl1_new, lp_adj_40mhz, 0x19)
			PHY_REG_MOD_ENTRY(LCN40PHY, agcSkipCnt20, lp_adj_20, 2)
			PHY_REG_MOD_ENTRY(LCN40PHY, crsMiscCtrl0_new, crs_lpstart_early_en, 1)
		PHY_REG_LIST_EXECUTE(pi);
	} else {
		PHY_REG_LIST_START
			PHY_REG_MOD_ENTRY(LCN40PHY, bphyTest, shiftBDigiGain, 0)
			PHY_REG_MOD_ENTRY(LCN40PHY, IDLEafterPktRXTimeout,
				BPHYIdleAfterPktRxTimeOut, 8)
			PHY_REG_MOD_ENTRY(LCN40PHY, crsSyncOffset, incSyncCnt20U, 2)
			PHY_REG_MOD_ENTRY(LCN40PHY, crsSyncOffset, incSyncCnt20L, 2)
			PHY_REG_MOD_ENTRY(LCN40PHY, clipCtrThreshLowGainEx,
				clip_counter_threshold_low_gain_40mhz, 60)
			PHY_REG_MOD_ENTRY(LCN40PHY, ClipCtrThresh, clipCtrThreshLoGain, 30)
			PHY_REG_MOD_ENTRY(LCN40PHY, ClipCtrDefThresh, clipCtrThresh, 40)
			PHY_REG_MOD_ENTRY(LCN40PHY, agcControl18, rssi_clip_gain_norm_8, 3)
			PHY_REG_MOD_ENTRY(LCN40PHY, radioTRCtrl, gainrequestTRAttnOnEn, 1)
			PHY_REG_MOD_ENTRY(LCN40PHY, radioTRCtrlCrs1, gainReqTrAttOnEnByCrs, 1)
			PHY_REG_MOD_ENTRY(LCN40PHY, agcSkipCnt, skip_cntr_adj_20L, 4)
			PHY_REG_MOD_ENTRY(LCN40PHY, crsMiscCtrl1_new, lp_adj_20L, 1)
			PHY_REG_MOD_ENTRY(LCN40PHY, crsMiscCtrl1_new, lp_adj_40mhz, -9)
			PHY_REG_MOD_ENTRY(LCN40PHY, AgcSynchTiming, skip_out_adj_20L, 1)
			PHY_REG_MOD_ENTRY(LCN40PHY, AgcSynchTiming, skip_out_adj_20U, -1)
			PHY_REG_MOD_ENTRY(LCN40PHY, AgcSynchTiming, skip_out_adj_40mhz, -5)
			/* enable clamp */
			PHY_REG_MOD_ENTRY(LCN40PHY, AfeCtrlOvr1, afe_clamp_en_ovr, 1)
			PHY_REG_MOD_ENTRY(LCN40PHY, AfeCtrlOvr1Val, afe_clamp_en_ovr_val, 1)
		PHY_REG_LIST_EXECUTE(pi);
	}

	PHY_REG_MOD(pi, LCN40PHY, rfoverride7, lpf_hpc_ovr, 0x1);
	PHY_REG_MOD(pi, LCN40PHY, rfoverride7val, lpf_hpc_ovr_val, 3);
	if (CHSPEC_IS40(pi->radio_chanspec)) {
		PHY_REG_MOD(pi, LCN40PHY, lpfbwlutreg1, lpf_rx_bw, 2);
		PHY_REG_MOD(pi, LCN40PHY, rfoverride6val, lpf_bias_ovr_val, 0);
	} else {
		PHY_REG_MOD(pi, LCN40PHY, lpfbwlutreg1, lpf_rx_bw, 0);
		PHY_REG_MOD(pi, LCN40PHY, rfoverride6val, lpf_bias_ovr_val, 2);
	}
	PHY_REG_MOD(pi, LCN40PHY, rfoverride6, lpf_bias_ovr, 0x1);

	wlc_lcn40phy_adc_init(pi, CHSPEC_IS20(pi->radio_chanspec) ? ADC_20M : ADC_40M, TRUE);

	PHY_REG_MOD(pi, LCN40PHY, agcControl4, c_agc_fsm_en, 1);

	/* enable LPF to remove lpf dc offset (note this override enables LPF for both RX and TX) */
	if (!(LCN40REV_IS(pi->pubpi.phy_rev, 4) ||
		LCN40REV_IS(pi->pubpi.phy_rev, 5) ||
		LCN40REV_IS(pi->pubpi.phy_rev, 6) ||
		LCN40REV_IS(pi->pubpi.phy_rev, 7))) {
		phy_utils_or_radioreg(pi, RADIO_2065_OVR7, 0x10);
		phy_utils_and_radioreg(pi, RADIO_2065_LPF_CFG1, ~0x200);
	}

	/* enable clamping for tx/rx cals */
	PHY_REG_MOD(pi, LCN40PHY, AfeCtrlOvr1, afe_clamp_en_ovr, 1);
	PHY_REG_MOD(pi, LCN40PHY, AfeCtrlOvr1Val, afe_clamp_en_ovr_val, 1);
}

/* CAL */
static void
WLBANDINITFN(wlc_phy_cal_init_lcn40phy)(phy_info_t *pi)
{
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	return;
}

/* TX power ctrl */
/* %%%%%% major flow operations */
void
wlc_phy_txpower_recalc_target_lcn40phy(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	uint16 pwr_ctrl = wlc_lcn40phy_get_tx_pwr_ctrl(pi);
	wlc_lcn40phy_set_tx_pwr_ctrl(pi, LCN40PHY_TX_PWR_CTRL_OFF);

	if (pi_lcn->txpwr_clamp_dis || pi_lcn->txpwr_tssioffset_clamp_dis) {
		pi_lcn->tssi_maxpwr_limit = 0x7fffffff;
		pi_lcn->tssi_minpwr_limit = 0xffffffff;
	}
	else if ((pi->hwpwrctrl_capable) && (!pi_lcn->txpwr_tssioffset_clamp_dis))
		wlc_lcn40phy_set_tssi_pwr_limit(pi, PHY_TSSI_SET_MIN_MAX_LIMIT);
	wlc_lcn40phy_txpower_recalc_target(pi);
	/* Restore power control */
	wlc_lcn40phy_set_tx_pwr_ctrl(pi, pwr_ctrl);
}

static void
wlc_lcn40phy_set_tx_pwr_ctrl(phy_info_t *pi, uint16 mode)
{
	uint16 old_mode = wlc_lcn40phy_get_tx_pwr_ctrl(pi);
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	ASSERT(
		(LCN40PHY_TX_PWR_CTRL_OFF == mode) ||
		(LCN40PHY_TX_PWR_CTRL_SW == mode) ||
		(LCN40PHY_TX_PWR_CTRL_HW
		== (mode | LCN40PHY_TxPwrCtrlCmd_use_txPwrCtrlCoefs_MASK)));

	/* Setting txfront end clock also along with hwpwr control */
	PHY_REG_MOD(pi, LCN40PHY, sslpnCalibClkEnCtrl, txFrontEndCalibClkEn,
		(LCN40PHY_TX_PWR_CTRL_HW ==
		(mode | LCN40PHY_TxPwrCtrlCmd_use_txPwrCtrlCoefs_MASK)) ? 1 : 0);
	/* Feed back RF power level to PAPD block */
	PHY_REG_MOD(pi, LCN40PHY, papd_control2, papd_analog_gain_ovr,
		(LCN40PHY_TX_PWR_CTRL_HW ==
		(mode | LCN40PHY_TxPwrCtrlCmd_use_txPwrCtrlCoefs_MASK)) ? 0 : 1);

	if (LCN40PHY_TX_PWR_CTRL_HW ==
		(mode | LCN40PHY_TxPwrCtrlCmd_use_txPwrCtrlCoefs_MASK)) {
		PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlRangeCmd, interpol_en, 1);
		/* interpolate using bbshift */
		PHY_REG_MOD(pi, LCN40PHY, bbShiftCtrl, bbshift_mode, 1);
	} else {
		PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlRangeCmd, interpol_en, 0);
		/* interpolate using bbshift */
		PHY_REG_MOD(pi, LCN40PHY, bbShiftCtrl, bbshift_mode, 0);
	}

	if (old_mode != mode) {
		if (LCN40PHY_TX_PWR_CTRL_HW ==
			(old_mode | LCN40PHY_TxPwrCtrlCmd_use_txPwrCtrlCoefs_MASK)) {
			/* Clear out all power offsets */
			wlc_lcn40phy_clear_tx_power_offsets(pi);
			phy_utils_write_phyreg(pi, LCN40PHY_BBmultCoeffSel, 0);
		}
		if (LCN40PHY_TX_PWR_CTRL_HW ==
			(mode | LCN40PHY_TxPwrCtrlCmd_use_txPwrCtrlCoefs_MASK)) {
			/* Recalculate target power to restore power offsets */
			wlc_lcn40phy_txpower_recalc_target(pi);
			/* Set starting index & NPT to best known values for that target */
			wlc_lcn40phy_set_start_tx_pwr_idx(pi, pi_lcn->lcnphy_tssi_idx);
			wlc_lcn40phy_set_start_CCK_tx_pwr_idx(pi, pi->u.pi_lcn40phy->cck_tssi_idx);
			wlc_lcn40phy_set_tx_pwr_npt(pi, pi_lcn->lcnphy_tssi_npt);
			phy_utils_write_phyreg(pi, LCN40PHY_BBmultCoeffSel, 1);
			/* Reset frame counter for NPT calculations */
			pi_lcn->lcnphy_tssi_tx_cnt = PHY_TOTAL_TX_FRAMES(pi);
			/* Disable any gain overrides */
			wlc_lcn40phy_disable_tx_gain_override(pi);
			pi_lcn->lcnphy_tx_power_idx_override = -1;
		}
		else
			wlc_lcn40phy_enable_tx_gain_override(pi);

		/* Set requested tx power control mode */
		phy_utils_mod_phyreg(pi, LCN40PHY_TxPwrCtrlCmd,
			(LCN40PHY_TxPwrCtrlCmd_txPwrCtrl_en_MASK |
			LCN40PHY_TxPwrCtrlCmd_hwtxPwrCtrl_en_MASK |
			LCN40PHY_TxPwrCtrlCmd_use_txPwrCtrlCoefs_MASK),
			mode);
		if (LCN40REV_IS(pi->pubpi.phy_rev, 6) ||
			(LCN40REV_IS(pi->pubpi.phy_rev, 7) &&
			(RADIOVER(pi->pubpi.radiover) == 0x4) && (CHSPEC_IS2G(pi->radio_chanspec))))
			PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlCmd, use_txPwrCtrlCoefs, 0);

		PHY_INFORM(("wl%d: %s: %s \n", pi->sh->unit, __FUNCTION__,
			mode ? ((LCN40PHY_TX_PWR_CTRL_HW ==
			(mode | LCN40PHY_TxPwrCtrlCmd_use_txPwrCtrlCoefs_MASK)) ?
			"Auto" : "Manual") : "Off"));
	}
}

static void
wlc_lcn40phy_force_pwr_index(phy_info_t *pi, int indx)
{
	phytbl_info_t tab;
	uint16 a, b;
	uint8 bb_mult;
	uint32 bbmultiqcomp, txgain, locoeffs, rfpower;
	phy_txgains_t gains;
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);

#if defined(PHYCAL_CACHING)
	ch_calcache_t *ctx = wlc_phy_get_chanctx(pi, pi->radio_chanspec);
	lcnphy_calcache_t *cache = NULL;

	if (ctx)
		cache = &ctx->u.lcnphy_cache;
#endif // endif

	ASSERT(indx <= LCN40PHY_MAX_TX_POWER_INDEX);

	/* Save forced index */
	pi_lcn->lcnphy_tx_power_idx_override = (int8)indx;
	pi_lcn->lcnphy_current_index = (uint8)indx;

	/* Preset txPwrCtrltbl */
	tab.tbl_id = LCN40PHY_TBL_ID_TXPWRCTL;
	tab.tbl_width = 32;	/* 32 bit wide	*/
	tab.tbl_len = 1;        /* # values   */

	/* Read index based bb_mult, a, b from the table */
	tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_IQ_OFFSET + indx; /* iqCoefLuts */
	tab.tbl_ptr = &bbmultiqcomp; /* ptr to buf */
	wlc_lcn40phy_read_table(pi,  &tab);

	/* Read index based tx gain from the table */
	tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_GAIN_OFFSET + indx; /* gainCtrlLuts */
	tab.tbl_width = 32;
	tab.tbl_ptr = &txgain; /* ptr to buf */
	wlc_lcn40phy_read_table(pi,  &tab);
	/* Apply tx gain */
	gains.gm_gain = (uint16)(txgain & 0xff);
	gains.pga_gain = (uint16)(txgain >> 8) & 0xff;
	gains.pad_gain = (uint16)(txgain >> 16) & 0xff;
	gains.dac_gain = (uint16)(bbmultiqcomp >> 28) & 0x07;
	wlc_lcn40phy_set_tx_gain(pi, &gains);
	wlc_lcn40phy_set_pa_gain(pi,  (uint16)(txgain >> 24) & 0xff);
	/* Apply bb_mult */
	bb_mult = (uint8)((bbmultiqcomp >> 20) & 0xff);
	wlc_lcn40phy_set_bbmult(pi, bb_mult);
	/* Enable gain overrides */
	wlc_lcn40phy_enable_tx_gain_override(pi);
	/* the reading and applying lo, iqcc coefficients is not getting done for 4313A0 */
	/* to be fixed */

	/* Apply iqcc */
	a = (uint16)((bbmultiqcomp >> 10) & 0x3ff);
	b = (uint16)(bbmultiqcomp & 0x3ff);

#if defined(PHYCAL_CACHING)
	if (ctx && cache->txiqlocal_a[0]) {
#else
	if (pi_lcn->lcnphy_cal_results.txiqlocal_a[0]) {
#endif /* defined(PHYCAL_CACHING) */
	wlc_lcn40phy_set_tx_iqcc(pi, a, b);
	/* Read index based di & dq from the table */
	}
#if defined(PHYCAL_CACHING)
	if (ctx && cache->txiqlocal_didq[0]) {
#else
	if (pi_lcn->lcnphy_cal_results.txiqlocal_didq[0]) {
#endif /* defined(PHYCAL_CACHING) */
	tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_LO_OFFSET + indx; /* loftCoefLuts */
	tab.tbl_ptr = &locoeffs; /* ptr to buf */
	wlc_lcn40phy_read_table(pi,  &tab);
	/* Apply locc */
	wlc_lcn40phy_set_tx_locc(pi, (uint16)locoeffs);
	}
	/* Apply PAPD rf power correction */
	tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_PWR_OFFSET + indx;
	tab.tbl_ptr = &rfpower; /* ptr to buf */
	wlc_lcn40phy_read_table(pi,  &tab);
	PHY_REG_MOD(pi, LCN40PHY, papd_analog_gain_ovr_val,
		papd_analog_gain_ovr_val, rfpower * 8);
	/* set vlin only applies to ePA */
	if (PHY_EPA_SUPPORT(pi_lcn->ePA))
		PHY_REG_MOD(pi, LCN40PHY, rfoverride8val, tx_vlin_ovr_val,
			(rfpower >> 10) & 1);
	else
		PHY_REG_MOD(pi, LCN40PHY, rfoverride8val, tx_vlin_ovr_val,
			(rfpower >> 10) & 0);
}

static void
wlc_lcn40phy_set_tx_pwr_by_index(phy_info_t *pi, int indx)
{
	/* Turn off automatic power control */
	wlc_lcn40phy_set_tx_pwr_ctrl(pi, LCN40PHY_TX_PWR_CTRL_OFF);

	/* Force tx power from the index */
	wlc_lcn40phy_force_pwr_index(pi, indx);
}
void
wlc_lcn40phy_tx_pwr_update_npt(phy_info_t *pi)
{
	uint16 tx_cnt, tx_total, npt;
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	phy_info_lcn40phy_t *pi_lcn40 = pi->u.pi_lcn40phy;

	if (LCN40PHY_TX_PWR_CTRL_HW != (wlc_lcn40phy_get_tx_pwr_ctrl((pi)) |
		LCN40PHY_TxPwrCtrlCmd_use_txPwrCtrlCoefs_MASK))
		return;

	tx_total = PHY_TOTAL_TX_FRAMES(pi);
	tx_cnt = tx_total - pi_lcn->lcnphy_tssi_tx_cnt;

	if (CHIPID(pi->sh->chip) != BCM43142_CHIP_ID)
		npt = wlc_lcn40phy_get_tx_pwr_npt(pi);
	else
		npt = 0;

	if (tx_cnt > (1 << npt)) {
		/* Reset frame counter */
		pi_lcn->lcnphy_tssi_tx_cnt = tx_total;

		/* Set new NPT */
		if (npt < pi_lcn->tssi_max_npt) {
			npt++;
			if (CHIPID(pi->sh->chip) == BCM43142_CHIP_ID)
				wlc_lcn40phy_set_tx_pwr_npt(pi, 0);
			else
				wlc_lcn40phy_set_tx_pwr_npt(pi, npt);
		}

		/* Update cached power index & NPT */
		pi_lcn->lcnphy_tssi_idx = wlc_lcn40phy_get_current_tx_pwr_idx(pi);
		if (LCN40REV_GE(pi->pubpi.phy_rev, 4))
			pi_lcn40->cck_tssi_idx =
			(phy_utils_read_phyreg(pi, LCN40PHY_TxPwrCtrlStatusExtCCK) & 0x1ff)/2;
		if (CHSPEC_IS2G(pi->radio_chanspec))
			pi_lcn->init_txpwrindex_2g = (uint8) pi_lcn->lcnphy_tssi_idx;
#ifdef BAND5G
		else
			pi_lcn->init_txpwrindex_5g = (uint8) pi_lcn->lcnphy_tssi_idx;
#endif /* BAND5G */

		pi_lcn40->init_ccktxpwrindex = (uint8) pi_lcn40->cck_tssi_idx;
		pi_lcn->lcnphy_tssi_npt = npt;

		/* Also update starting index as an additional safeguard */
		wlc_lcn40phy_set_start_tx_pwr_idx(pi, pi_lcn->lcnphy_tssi_idx);
		wlc_lcn40phy_set_start_CCK_tx_pwr_idx(pi, pi_lcn40->cck_tssi_idx);

		PHY_INFORM(("wl%d: %s: Index: %d, NPT: %d, TxCount: %d\n",
			pi->sh->unit, __FUNCTION__, pi_lcn->lcnphy_tssi_idx, npt, tx_cnt));
	}
}

void
wlc_lcn40phy_clear_tx_power_offsets(phy_info_t *pi)
{
	uint32 data_buf[64];
	phytbl_info_t tab;
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);

	/* Clear out buffer */
	bzero(data_buf, sizeof(data_buf));

	/* Preset txPwrCtrltbl */
	tab.tbl_id = LCN40PHY_TBL_ID_TXPWRCTL;
	tab.tbl_width = 32;	/* 32 bit wide	*/
	tab.tbl_ptr = data_buf; /* ptr to buf */
	/* Since 4313A0 uses the rate offset table to do tx pwr ctrl for cck, */
	/* we shouldn't be clearing the rate offset table */

	if (!pi_lcn->lcnphy_uses_rate_offset_table) {
		/* Per rate power offset */
		tab.tbl_len = 20; /* # values   */
		tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_RATE_OFFSET;
		wlc_lcn40phy_write_table(pi, &tab);
	}
	/* Per index power offset */
	tab.tbl_len = 64; /* # values   */
	tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_MAC_OFFSET;
	wlc_lcn40phy_write_table(pi, &tab);
}
void
wlc_lcn40phy_load_clear_papr_tbls(phy_info_t *pi, bool load)
{
	phytbl_info_t tab;
	uint16 SAVE_pwrctrl = wlc_lcn40phy_get_tx_pwr_ctrl(pi);
	phy_info_lcn40phy_t *pi_lcn40 = pi->u.pi_lcn40phy;
	uint8 *paprgaintbl, local_paprgaintbl[20];
	uint32 *paprgamtbl;
	uint32 zero_data[20];
	int bbmult_mult_factor, iir_filter_shift_factor;
	int16 papd_max_amam = 0;
	int8 off;

	bzero(zero_data, sizeof(zero_data));

	papd_max_amam = pi_lcn40->max_amam_dB;
	/* scale bbmult */
	if (CHSPEC_IS20(pi->radio_chanspec) &&
		CHSPEC_IS2G(pi->radio_chanspec) && pi_lcn40->paprr_enable2g && load) {
		paprgaintbl = pi_lcn40->paprgaintbl2g;
		paprgamtbl = pi_lcn40->paprgamtbl2g;
		bbmult_mult_factor = 128;
		iir_filter_shift_factor = 32;
		PHY_REG_MOD(pi, LCN40PHY, papr_iir_group_dly, papr_override_enable, 0);
	}
#ifdef BAND5G
	else if (CHSPEC_IS20(pi->radio_chanspec) && CHSPEC_IS5G(pi->radio_chanspec) &&
		pi_lcn40->paprr_enable5g && load) {
		paprgaintbl = pi_lcn40->paprgaintbl5g;
		paprgamtbl = pi_lcn40->paprgamtbl5g;
		bbmult_mult_factor = 128;
		iir_filter_shift_factor = 32;
		PHY_REG_MOD(pi, LCN40PHY, papr_iir_group_dly, papr_override_enable, 0);
	}
	else if (CHSPEC_IS40(pi->radio_chanspec) && CHSPEC_IS5G(pi->radio_chanspec) &&
		pi_lcn40->paprr_enable5g && load) {
		paprgaintbl = pi_lcn40->papr40gaintbl5g;
		paprgamtbl = pi_lcn40->papr40gamtbl5g;
		bbmult_mult_factor = 146;
		iir_filter_shift_factor = 28;
		PHY_REG_MOD(pi, LCN40PHY, papr_iir_group_dly, papr_override_enable, 0);
	}
#endif /* #ifdef BAND5G */
	else {
		paprgaintbl = (uint8 *)zero_data;
		paprgamtbl = zero_data;
		bbmult_mult_factor = 64;
		iir_filter_shift_factor = 64;

		/* disabling PAPR in the override mode */
		PHY_REG_MOD(pi, LCN40PHY, papr_iir_group_dly, papr_override_enable, 1);
		PHY_REG_MOD(pi, LCN40PHY, papr_iir_group_dly, papr_enable, 0);

		papd_max_amam = 0;
	}

	for (off = 0; off < 20; off++) {
		local_paprgaintbl[off] = (paprgaintbl[off] & 0x7f) > papd_max_amam ?
			(paprgaintbl[off] & 0x7f) - papd_max_amam : 0;
		local_paprgaintbl[off] |= (paprgaintbl[off] & 0x80);
	}

	wlc_lcn40phy_scbbmult_tx_gain_table(pi, bbmult_mult_factor);

	/* scale down iir filter */
	wlc_lcn40phy_scale_tx_iir_filter(pi, TX_IIR_FILTER_OFDM,
		pi_lcn40->tx_iir_filter_type_ofdm, iir_filter_shift_factor);
	wlc_lcn40phy_scale_tx_iir_filter(pi, TX_IIR_FILTER_CCK,
		pi_lcn40->tx_iir_filter_type_cck, iir_filter_shift_factor);
	wlc_lcn40phy_scale_tx_iir_filter(pi, TX_IIR_FILTER_OFDM40,
		pi_lcn40->tx_iir_filter_type_ofdm40, iir_filter_shift_factor);

	/* turn off power control before writing papr tables */
	wlc_lcn40phy_set_tx_pwr_ctrl(pi, LCN40PHY_TX_PWR_CTRL_OFF);

	/* load Pwr ctrl table with papr rate offset tables */
	/* wlc_lcn40phy_papr_tx_power_offsets(pi); */

	/* load Pwr ctrl table with papr gain tables */
	tab.tbl_id = LCN40PHY_TBL_ID_TXPWRCTL;
	tab.tbl_width = 8;	/* 32 bit wide	*/
	tab.tbl_ptr = local_paprgaintbl; /* ptr to buf */

	tab.tbl_len = 20; /* # values   */
	tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_PAPRGAIN_OFFSET;
	wlc_lcn40phy_write_table(pi, &tab);

	/* load Pwr ctrl table with papr gamma tables */
	tab.tbl_id = LCN40PHY_TBL_ID_TXPWRCTL;
	tab.tbl_width = 32;	/* 32 bit wide	*/
	tab.tbl_ptr = paprgamtbl; /* ptr to buf */

	tab.tbl_len = 20; /* # values   */
	tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_PAPRGAMMA_OFFSET;
	wlc_lcn40phy_write_table(pi, &tab);

/* papr override and filter enables  */

	if (CHSPEC_IS40(pi->radio_chanspec)) {
		PHY_REG_LIST_START
			PHY_REG_WRITE_ENTRY(LCN40PHY, papr_coeffb10, 72)
			PHY_REG_WRITE_ENTRY(LCN40PHY, papr_coeffb11, 143)
			PHY_REG_WRITE_ENTRY(LCN40PHY, papr_coeffb12, 72)
			PHY_REG_WRITE_ENTRY(LCN40PHY, papr_coeffa11, -201)
			PHY_REG_WRITE_ENTRY(LCN40PHY, papr_coeffa12, 111)
			PHY_REG_WRITE_ENTRY(LCN40PHY, papr_coeffa21, -208)
			PHY_REG_WRITE_ENTRY(LCN40PHY, papr_coeffa22, 89)
			PHY_REG_WRITE_ENTRY(LCN40PHY, papr_coeffb20, 65)
			PHY_REG_WRITE_ENTRY(LCN40PHY, papr_coeffb21, 130)
			PHY_REG_WRITE_ENTRY(LCN40PHY, papr_coeffb22, 65)
			PHY_REG_MOD_ENTRY(LCN40PHY, papr_iir_group_dly, papr_iir_group_dly, 6)
		PHY_REG_LIST_EXECUTE(pi);
	} else {
		PHY_REG_LIST_START
			PHY_REG_WRITE_ENTRY(LCN40PHY, papr_coeffb10, 66)
			PHY_REG_WRITE_ENTRY(LCN40PHY, papr_coeffb11, 131)
			PHY_REG_WRITE_ENTRY(LCN40PHY, papr_coeffb12, 66)
			PHY_REG_WRITE_ENTRY(LCN40PHY, papr_coeffa11, 308)
			PHY_REG_WRITE_ENTRY(LCN40PHY, papr_coeffa12, 111)
			PHY_REG_WRITE_ENTRY(LCN40PHY, papr_coeffa21, 306)
			PHY_REG_WRITE_ENTRY(LCN40PHY, papr_coeffa22, 88)
			PHY_REG_WRITE_ENTRY(LCN40PHY, papr_coeffb20, 52)
			PHY_REG_WRITE_ENTRY(LCN40PHY, papr_coeffb21, 104)
			PHY_REG_WRITE_ENTRY(LCN40PHY, papr_coeffb22, 52)
			PHY_REG_MOD_ENTRY(LCN40PHY, papr_iir_group_dly, papr_iir_group_dly, 6)
		PHY_REG_LIST_EXECUTE(pi);
	}
	PHY_REG_MOD(pi, LCN40PHY, papr_iir_group_dly, papr_2ndcordic_enable, 0);

	wlc_lcn40phy_set_tx_pwr_ctrl(pi, SAVE_pwrctrl);

}

int8
wlc_lcn40phy_get_current_tx_pwr_idx(phy_info_t *pi)
{
	int8 indx;
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	/* for txpwrctrl_off and tempsense_based pwrctrl, return current_index */
	if (txpwrctrl_off(pi))
		indx = pi_lcn->lcnphy_current_index;
	else
		indx = (int8)(wlc_lcn40phy_get_current_tx_pwr_idx_if_pwrctrl_on(pi)/2);

	return indx;
}

int
wlc_lcn40phy_tssi_cal(phy_info_t *pi)
{
	return 0;
}

static uint16
wlc_lcn40phy_get_tx_locc(phy_info_t *pi)
{	phytbl_info_t tab;
	uint16 didq;

	/* Update iqloCaltbl */
	tab.tbl_id = LCN40PHY_TBL_ID_IQLOCAL; /* iqloCaltbl		*/
	tab.tbl_width = 16;	/* 16 bit wide	*/
	tab.tbl_ptr = &didq;
	tab.tbl_len = 1;
	tab.tbl_offset = 85;
	wlc_lcn40phy_read_table(pi, &tab);

	return didq;
}

static void
wlc_lcn40phy_set_tx_locc(phy_info_t *pi, uint16 didq)
{
	phytbl_info_t tab;

	/* Update iqloCaltbl */
	tab.tbl_id = LCN40PHY_TBL_ID_IQLOCAL;			/* iqloCaltbl	*/
	tab.tbl_width = 16;	/* 16 bit wide	*/
	tab.tbl_ptr = &didq;
	tab.tbl_len = 1;
	tab.tbl_offset = 85;
	wlc_lcn40phy_write_table(pi, &tab);
}

static void
wlc_lcn40phy_get_tx_iqcc(phy_info_t *pi, uint16 *a, uint16 *b)
{
	uint16 iqcc[2];
	phytbl_info_t tab;

	tab.tbl_ptr = iqcc; /* ptr to buf */
	tab.tbl_len = 2;        /* # values   */
	tab.tbl_id = LCN40PHY_TBL_ID_IQLOCAL; /* iqloCaltbl      */
	tab.tbl_offset = 80; /* tbl offset */
	tab.tbl_width = 16;     /* 16 bit wide */
	wlc_lcn40phy_read_table(pi, &tab);

	*a = iqcc[0];
	*b = iqcc[1];
}
static void
wlc_lcn40phy_set_tx_iqcc(phy_info_t *pi, uint16 a, uint16 b)
{
	phytbl_info_t tab;
	uint16 iqcc[2];

	/* Fill buffer with coeffs */
	iqcc[0] = a;
	iqcc[1] = b;
	/* Update iqloCaltbl */
	tab.tbl_id = LCN40PHY_TBL_ID_IQLOCAL;			/* iqloCaltbl	*/
	tab.tbl_width = 16;	/* 16 bit wide	*/
	tab.tbl_ptr = iqcc;
	tab.tbl_len = 2;
	tab.tbl_offset = 80;
	wlc_lcn40phy_write_table(pi, &tab);
}

#define RADIO_REG_EI(pi) (RADIO_2065_TXGM_LOFT_FINE_I)
#define RADIO_REG_EQ(pi) (RADIO_2065_TXGM_LOFT_FINE_Q)
#define RADIO_REG_FI(pi) (RADIO_2065_TXGM_LOFT_COARSE_I)
#define RADIO_REG_FQ(pi) (RADIO_2065_TXGM_LOFT_COARSE_Q)

static uint16
lcn40phy_iqlocc_write(phy_info_t *pi, uint8 data)
{
	int32 data32 = (int8)data;
	int32 rf_data32;
	int32 ip, in;
	ip = 8 + (data32 >> 1);
	in = 8 - ((data32+1) >> 1);
	rf_data32 = (in << 4) | ip;
	return (uint16)(rf_data32);
}

static void
wlc_lcn40phy_get_radio_loft(phy_info_t *pi,
	uint8 *ei0,
	uint8 *eq0,
	uint8 *fi0,
	uint8 *fq0)
{
	*ei0 = LCN40PHY_IQLOCC_READ(
		phy_utils_read_radioreg(pi, RADIO_REG_EI(pi)));
	*eq0 = LCN40PHY_IQLOCC_READ(
		phy_utils_read_radioreg(pi, RADIO_REG_EQ(pi)));
	*fi0 = LCN40PHY_IQLOCC_READ(
		phy_utils_read_radioreg(pi, RADIO_REG_FI(pi)));
	*fq0 = LCN40PHY_IQLOCC_READ(
		phy_utils_read_radioreg(pi, RADIO_REG_FQ(pi)));
}
static void
wlc_lcn40phy_set_radio_loft(phy_info_t *pi,
	uint8 ei0,
	uint8 eq0,
	uint8 fi0,
	uint8 fq0)
{
	phy_utils_write_radioreg(pi, RADIO_REG_EI(pi), lcn40phy_iqlocc_write(pi, ei0));
	phy_utils_write_radioreg(pi, RADIO_REG_EQ(pi), lcn40phy_iqlocc_write(pi, eq0));
	phy_utils_write_radioreg(pi, RADIO_REG_FI(pi), lcn40phy_iqlocc_write(pi, fi0));
	phy_utils_write_radioreg(pi, RADIO_REG_FQ(pi), lcn40phy_iqlocc_write(pi, fq0));
}

static int16
wlc_lcn40phy_read_tempsense_regs(phy_info_t *pi)
{
	uint16 tempsenseval1, tempsenseval2;
	uint16 tempsenseval3, tempsenseval4;
	int16 temp2, temp1, temp3, temp4;
	int16 avg;

	tempsenseval1 = phy_utils_read_phyreg(pi, LCN40PHY_TxPwrCtrlStatusTemp) & 0x1FF;
	tempsenseval2 = phy_utils_read_phyreg(pi, LCN40PHY_TxPwrCtrlStatusTemp1) & 0x1FF;
	tempsenseval3 = phy_utils_read_phyreg(pi, LCN40PHY_TxPwrCtrlStatusTemp2) & 0x1FF;
	tempsenseval4 = phy_utils_read_phyreg(pi, LCN40PHY_TxPwrCtrlStatusTemp3) & 0x1FF;

	if (tempsenseval1 > 255)
		temp1 = (int16)(tempsenseval1 - 512);
	else
		temp1 = (int16)tempsenseval1;

	if (tempsenseval2 > 255)
		temp2 = (int16)(tempsenseval2 - 512);
	else
		temp2 = (int16)tempsenseval2;

	if (tempsenseval3 > 255)
		temp3 = (int16)(tempsenseval3 - 512);
	else
		temp3 = (int16)tempsenseval3;

	if (tempsenseval4 > 255)
		temp4 = (int16)(tempsenseval4 - 512);
	else
		temp4 = (int16)tempsenseval4;

	avg = ((temp2 - temp1) + (temp4 - temp3)) >> 1;

	return avg;
}

static void
wlc_lcn40phy_force_digigain_zerodB_WAR(phy_info_t *pi)
{
	PHY_REG_LIST_START
		/* to clear digigain register, see SWWLAN-34440 */
		PHY_REG_MOD_ENTRY(LCN40PHY, resetCtrl, radioctrlSoftReset, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, resetCtrl, radioctrlSoftReset, 0)

		/* Force digi_gain to 0dB */
		PHY_REG_MOD_ENTRY(LCN40PHY, radioCtrl, digi_gain_ovr, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, radioCtrl, digi_gain_ovr_val, 0)
		/* workaround to activate the digi_gain override, for 2G only
		no aci mode is used in 5G, so this is not needed for 5G
		*/
		PHY_REG_MOD_ENTRY(LCN40PHY, crsgainCtrl, wlpriogainChangeEn, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, radioCtrl, internalRadioSharingEn, 1)

		PHY_REG_MOD_ENTRY(LCN40PHY, RFinputOverrideVal, BTPriority_ovr_val, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, RFinputOverride, BTPriority_ovr, 1)
	PHY_REG_LIST_EXECUTE(pi);
	OSL_DELAY(10);
	PHY_REG_MOD(pi, LCN40PHY, RFinputOverride, BTPriority_ovr, 0);
	PHY_REG_MOD(pi, LCN40PHY, radioCtrl, internalRadioSharingEn, 0);
}

static int16
wlc_lcn40phy_temp_sense_vbatTemp_on(phy_info_t *pi, int mode)
{
	uint16 save_RF_testbuf_cfg1, save_RF_auxpga_cfg1;
	uint16 save_RF_lpf_cfg1, save_RF_OVR1, save_RF_OVR7;
	uint16 save_AfeCtrlOvr1, save_RF_OVR16, save_RF_tempsense_cfg;
	uint16 save_AfeCtrlOvr1Val, save_sslpnCalibClkEnCtrl, save_TxPwrCtrlCmd;
	uint16 save_RF_vbat_cfg, save_digi_gain_ovr, save_digi_gain_ovr_val;
	uint16 save_agcControl4;

	uint16 save_TempSenseCorrection;
	uint16 save_wlpriogainChangeEn;
	uint16 save_internalRadioSharingEn, save_BTPriority_ovr_val, save_BTPriority_ovr;

	uint16 vbat;
	uint16 vmid;
	int16 avg = 0;
	uint16 temp;
	bool suspend = !(R_REG(pi->sh->osh, &((phy_info_t *)pi)->regs->maccontrol) & MCTL_EN_MAC);

	if (!suspend)
		wlapi_suspend_mac_and_wait(pi->sh->physhim);

	save_RF_testbuf_cfg1 = phy_utils_read_radioreg(pi, RADIO_2065_TESTBUF_CFG1);
	save_RF_auxpga_cfg1 = phy_utils_read_radioreg(pi, RADIO_2065_AUXPGA_CFG1);
	save_RF_lpf_cfg1 = phy_utils_read_radioreg(pi, RADIO_2065_LPF_CFG1);
	save_RF_OVR1 = phy_utils_read_radioreg(pi, RADIO_2065_OVR1);
	save_RF_OVR7 = phy_utils_read_radioreg(pi, RADIO_2065_OVR7);
	save_AfeCtrlOvr1 = phy_utils_read_phyreg(pi, LCN40PHY_AfeCtrlOvr1);
	save_RF_OVR16 = phy_utils_read_radioreg(pi, RADIO_2065_OVR16);
	save_RF_tempsense_cfg = phy_utils_read_radioreg(pi, RADIO_2065_TEMPSENSE_CFG);
	save_AfeCtrlOvr1Val = phy_utils_read_phyreg(pi, LCN40PHY_AfeCtrlOvr1Val);
	save_sslpnCalibClkEnCtrl = phy_utils_read_phyreg(pi, LCN40PHY_sslpnCalibClkEnCtrl);
	save_TxPwrCtrlCmd = PHY_REG_READ(pi, LCN40PHY, TxPwrCtrlCmd, txPwrCtrl_swap_iq);
	save_TempSenseCorrection = PHY_REG_READ(pi, LCN40PHY, TempSenseCorrection, tempsenseCorr);
	save_wlpriogainChangeEn = phy_utils_read_phyreg(pi, LCN40PHY_crsgainCtrl);
	save_internalRadioSharingEn = phy_utils_read_phyreg(pi, LCN40PHY_radioCtrl);
	save_BTPriority_ovr_val = phy_utils_read_phyreg(pi, LCN40PHY_RFinputOverrideVal);
	save_BTPriority_ovr = phy_utils_read_phyreg(pi, LCN40PHY_RFinputOverride);
	save_RF_vbat_cfg = phy_utils_read_radioreg(pi, RADIO_2065_VBAT_CFG);
	save_digi_gain_ovr = PHY_REG_READ(pi, LCN40PHY, radioCtrl, digi_gain_ovr);
	save_digi_gain_ovr_val = PHY_REG_READ(pi, LCN40PHY, radioCtrl, digi_gain_ovr_val);
	save_agcControl4 = phy_utils_read_phyreg(pi, LCN40PHY_agcControl4);

	PHY_REG_LIST_START
		/* settings for txpwrctrl block */
		PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlNum_Vbat, Nvbat_delay, 64)
		PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlNum_Vbat, Nvbat_intg_log2, 4)
		PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlNum_temp, Ntemp_delay, 64)
		PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlNum_temp, Ntemp_intg_log2, 4)
		PHY_REG_MOD_ENTRY(LCN40PHY, TempSenseCorrection, tempsenseCorr, 0)
		/* Swap iq */
		PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlCmd, txPwrCtrl_swap_iq, 1)
	PHY_REG_LIST_EXECUTE(pi);

	/* auxpga vmid and gain for Vbat */
	PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlRfCtrl5, afeAuxpgaSelVmidVal3, AUXPGA_VBAT_VMID_VAL);
	PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlRfCtrl5, afeAuxpgaSelGainVal3, AUXPGA_VBAT_GAIN_VAL);

	/* auxpga vmid and gain for tempsense */
	if (LCN40REV_IS(pi->pubpi.phy_rev, 6)) {
		vmid = AUXPGA_TEMPER_VMID_VAL_43143;
	} else {
		vmid = AUXPGA_TEMPER_VMID_VAL;
	}
	PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlRfCtrl6, afeAuxpgaSelVmidVal4, vmid);
	PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlRfCtrl6, afeAuxpgaSelGainVal4, AUXPGA_TEMPER_GAIN_VAL);

	PHY_REG_LIST_START
		/* clock for pwrctrl block */
		PHY_REG_MOD_ENTRY(LCN40PHY, sslpnCalibClkEnCtrl, txFrontEndCalibClkEn, 1)

		PHY_REG_MOD_ENTRY(LCN40PHY, AfeCtrlOvr1Val, afe_iqadc_aux_en_ovr_val, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, AfeCtrlOvr1, afe_iqadc_aux_en_ovr, 1)

		/* set reg(RF_testbuf_cfg1.sel_test_port) 1 */
		RADIO_REG_MOD_ENTRY(RADIO_2065_TESTBUF_CFG1, 0x70, 1<<4)

		/* Tempsense powerup */
		/* set reg(RF_OVR16.ovr_tempsense_pu) 1 */
		RADIO_REG_OR_ENTRY(RADIO_2065_OVR16, 4)
		/* set reg(RF_tempsense_cfg.pu) 1 */
		RADIO_REG_OR_ENTRY(RADIO_2065_TEMPSENSE_CFG, 1)

		/* Vbatsense powerup */
		/* set reg(RF_OVR16.ovr_vbat_monitor_pu)   1 */
		RADIO_REG_OR_ENTRY(RADIO_2065_OVR16, 2)
		/* set reg(RF_vbat_cfg.monitor_pu)     1 */
		RADIO_REG_OR_ENTRY(RADIO_2065_VBAT_CFG, 1)

		/* testbuf powerup */
		/* set reg(RF_OVR16.ovr_testbuf_PU) 1 */
		RADIO_REG_OR_ENTRY(RADIO_2065_OVR16, 1)
		/* set reg(RF_testbuf_cfg1.PU) 1 */
		RADIO_REG_OR_ENTRY(RADIO_2065_TESTBUF_CFG1, 1)

		/* Aux pga powerup */
		/* set reg(RF_auxpga_cfg1.auxpga_pu) 1 */
		RADIO_REG_OR_ENTRY(RADIO_2065_AUXPGA_CFG1, 1)

		/* set reg(RF_OVR1.ovr_afe_auxpga_pu) 1 */
		RADIO_REG_OR_ENTRY(RADIO_2065_OVR1, 1<<15)
	PHY_REG_LIST_EXECUTE(pi);

	/* Power up RX buffer, lpf_rxbuf_pu in AMS */
	/* set reg(RF_lpf_cfg1.rxbuf_puQ) 1 */
	/* set reg(RF_lpf_cfg1.rxbuf_puI) 1 */
	/* set reg(RF_OVR7.ovr_lpf_rxbuf_puQ)  1 */
	/* set reg(RF_OVR7.ovr_lpf_rxbuf_puI) 1 */
	temp = (1<<6)|(1<<7);
	phy_utils_or_radioreg(pi, RADIO_2065_LPF_CFG1, temp);
	phy_utils_or_radioreg(pi, RADIO_2065_OVR7, temp);

	PHY_REG_LIST_START
		/* Connect AuxPGA to Rx Buffer */
		/* set reg(RF_lpf_cfg1.sel_rx_buffer) 2 */
		RADIO_REG_MOD_ENTRY(RADIO_2065_LPF_CFG1, 0x3000, 2<<12)

		/* set reg(RF_OVR7.ovr_lpf_sel_rx_buffer) 1 */
		RADIO_REG_OR_ENTRY(RADIO_2065_OVR7, 1<<3)

		/* Connect Rx Buffer to ADC */
		/* set reg(RF_lpf_cfg1.sel_byp_rxlpf) 1 */
		RADIO_REG_MOD_ENTRY(RADIO_2065_LPF_CFG1, 0xc000, 1<<14)

		/* set reg(RF_OVR7.ovr_lpf_sel_byp_rxlpf) 1 */
		RADIO_REG_OR_ENTRY(RADIO_2065_OVR7, 1<<5)

		/* set reg(RF_lpf_cfg1.sel_tx_rx) 3 */
		RADIO_REG_MOD_ENTRY(RADIO_2065_LPF_CFG1, 0xc00, 3<<10)

		/* set reg(RF_OVR7.ovr_lpf_sel_tx_rx) 1 */
		RADIO_REG_OR_ENTRY(RADIO_2065_OVR7, 1<<2)
	PHY_REG_LIST_EXECUTE(pi);

	wlc_lcn40phy_force_digigain_zerodB_WAR(pi);

	OSL_DELAY(10);
	PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlRangeCmd, force_vbatTemp, 1);
	OSL_DELAY(10);

	if (mode == TEMPSENSE) {
		avg = wlc_lcn40phy_read_tempsense_regs(pi);
	}
	else {
		vbat = phy_utils_read_phyreg(pi, LCN40PHY_TxPwrCtrlStatusVbat) & 0x1FF;
		if (vbat > 255)
			avg = (int16)(vbat - 512);
		else
			avg = (int16)vbat;
	}

	phy_utils_write_radioreg(pi, RADIO_2065_TESTBUF_CFG1, save_RF_testbuf_cfg1);
	phy_utils_write_radioreg(pi, RADIO_2065_AUXPGA_CFG1, save_RF_auxpga_cfg1);
	phy_utils_write_radioreg(pi, RADIO_2065_LPF_CFG1, save_RF_lpf_cfg1);
	phy_utils_write_radioreg(pi, RADIO_2065_OVR1, save_RF_OVR1);
	phy_utils_write_radioreg(pi, RADIO_2065_OVR7, save_RF_OVR7);
	PHY_REG_WRITE(pi, LCN40PHY, AfeCtrlOvr1, save_AfeCtrlOvr1);
	phy_utils_write_radioreg(pi, RADIO_2065_OVR16, save_RF_OVR16);
	phy_utils_write_radioreg(pi, RADIO_2065_TEMPSENSE_CFG, save_RF_tempsense_cfg);
	phy_utils_write_radioreg(pi, RADIO_2065_VBAT_CFG, save_RF_vbat_cfg);
	PHY_REG_WRITE(pi, LCN40PHY, agcControl4, save_agcControl4);
	PHY_REG_MOD(pi, LCN40PHY, radioCtrl, digi_gain_ovr_val, save_digi_gain_ovr_val);
	PHY_REG_MOD(pi, LCN40PHY, radioCtrl, digi_gain_ovr, save_digi_gain_ovr);
	PHY_REG_WRITE(pi, LCN40PHY, AfeCtrlOvr1Val, save_AfeCtrlOvr1Val);
	PHY_REG_WRITE(pi, LCN40PHY, sslpnCalibClkEnCtrl, save_sslpnCalibClkEnCtrl);
	PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlCmd, txPwrCtrl_swap_iq, save_TxPwrCtrlCmd);
	PHY_REG_MOD(pi, LCN40PHY, TempSenseCorrection, tempsenseCorr, save_TempSenseCorrection);
	PHY_REG_WRITE(pi, LCN40PHY, crsgainCtrl, save_wlpriogainChangeEn);
	PHY_REG_WRITE(pi, LCN40PHY, radioCtrl, save_internalRadioSharingEn);
	PHY_REG_WRITE(pi, LCN40PHY, RFinputOverrideVal, save_BTPriority_ovr_val);
	PHY_REG_WRITE(pi, LCN40PHY, RFinputOverride, save_BTPriority_ovr);

	if (!suspend)
		wlapi_enable_mac(pi->sh->physhim);

	return avg;
}

/* wlc_lcn40phy_tempsense_new: use mode 1 if tempsense registers
	might be used for other purposes such as background idle tssi measurement
*/
int16
wlc_lcn40phy_tempsense(phy_info_t *pi, bool mode)
{
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);

	int32 b = pi_lcn->temp_add;
	int32 a = pi_lcn->temp_mult;
	int32 q = pi_lcn->temp_q;
	int16 avg, degree;
	phy_info_lcn40phy_t *pi_lcn40 = pi->u.pi_lcn40phy;

	/* mode 1 will use force_vbat and do the measurement */
	if (mode == TEMPER_VBAT_TRIGGER_NEW_MEAS) {
		avg = wlc_lcn40phy_temp_sense_vbatTemp_on(pi, TEMPSENSE);
		pi_lcn40->tempsense_tx_cnt = PHY_TOTAL_TX_FRAMES(pi);
	} else {
#ifdef BAND5G
		if (CHSPEC_IS5G(pi->radio_chanspec)) {
			uint16 tx_total, txcnt;

			tx_total = PHY_TOTAL_TX_FRAMES(pi);
			txcnt = tx_total - pi_lcn40->tempsense_tx_cnt;
			if (txcnt >= 4)
				/* Directly read registers, do not use force_vbat */
				avg = wlc_lcn40phy_read_tempsense_regs(pi);
			else
				avg = pi_lcn40->last_tempsense_avg;
		} else
#endif /* BAND5G */
			avg = wlc_lcn40phy_read_tempsense_regs(pi);
	}

	pi_lcn40->last_tempsense_avg = avg;
	/* Temp in deg = (temp_add + (avg)*temp_mult)>>temp_q; */
	degree = ((b + (avg * a)) + (1 << (q-1))) >> q;

	return degree;
}

/* return value is in volt Q4.4 */
int8
wlc_lcn40phy_vbatsense(phy_info_t *pi, bool mode)
{
	uint16 vbatsenseval;
	int32 avg = 0;
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	int32 b = pi_lcn->vbat_add;
	int32 a = pi_lcn->vbat_mult;
	int32 q = pi_lcn->vbat_q;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	if (NORADIO_ENAB(pi->pubpi))
		return -1;

	/* mode 1 will set-up and do the measurement */
	if (mode == TEMPER_VBAT_TRIGGER_NEW_MEAS) {
		avg = wlc_lcn40phy_temp_sense_vbatTemp_on(pi, VBATSENSE);
	}
	else {
		vbatsenseval = phy_utils_read_phyreg(pi, LCN40PHY_TxPwrCtrlStatusVbat) & 0x1FF;

		if (vbatsenseval > 255)
			avg =  (int32)(vbatsenseval - 512);
		else
			avg = (int32)vbatsenseval;
	}

	/* Voltage = (vbat_add + (vbat_reading)*vbat_mult)>>vbat_q;  */
	avg = ((b + (a * avg)) + (1 << (q-5))) >> (q - 4);
	return (int8)avg;
}

void
wlc_lcn40phy_calib_modes(phy_info_t *pi, uint mode)
{

	switch (mode) {
	case PHY_PERICAL_CHAN:
		/* right now, no channel based calibration */
		break;
	case PHY_FULLCAL:
	case PHY_PERICAL_WATCHDOG:
		wlc_lcn40phy_periodic_cal(pi);
		break;

	case PHY_PAPDCAL:
		if (!PHY_EPA_SUPPORT(wlc_phy_getlcnphy_common(pi)->ePA))
			wlc_lcn40phy_papd_recal(pi);
		break;
	default:
		ASSERT(0);
		break;
	}
}

static bool
wlc_lcn40phy_cal_reqd(phy_info_t *pi)
{
#if defined(PHYCAL_CACHING)
	ch_calcache_t *ctx = wlc_phy_get_chanctx(pi, pi->radio_chanspec);
	uint time_since_last_cal;

	if (!ctx)
		return TRUE;
	time_since_last_cal = (pi->sh->now >= ctx->cal_info.last_cal_time)?
		(pi->sh->now - ctx->cal_info.last_cal_time):
		(((uint)~0) - ctx->cal_info.last_cal_time + pi->sh->now);

	if (ctx->valid)
#else
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	uint time_since_last_cal = (pi->sh->now >= pi->phy_lastcal)?
		(pi->sh->now - pi->phy_lastcal):
		(((uint)~0) - pi->phy_lastcal + pi->sh->now);

	if (pi_lcn->lcnphy_full_cal_channel == CHSPEC_CHANNEL(pi->radio_chanspec))
#endif /* defined(PHYCAL_CACHING) */
		return (time_since_last_cal >= pi->sh->glacial_timer);
	return TRUE;
}

static void
wlc_phy_watchdog_lcn40phy(phy_info_t *pi)
{
	wlc_lcn40phy_update_cond_backoff_boost(pi);
	wlc_lcn40phy_txgainindex_cap_adjust(pi);

	if (pi->phy_forcecal || wlc_lcn40phy_cal_reqd(pi)) {
		if (!(SCAN_RM_IN_PROGRESS(pi) || PLT_INPROG_PHY(pi) ||
			ASSOC_INPROG_PHY(pi) || pi->carrier_suppr_disable ||
			(pi->measure_hold & PHY_HOLD_FOR_PKT_ENG) || pi->disable_percal)) {
			wlc_lcn40phy_noise_measure_stop(pi);
			wlc_lcn40phy_calib_modes(pi, PHY_PERICAL_WATCHDOG);
			wlc_lcn40phy_noise_measure(pi);
		}
	}

	if (!(SCAN_RM_IN_PROGRESS(pi) ||
		PLT_INPROG_PHY(pi))) {
		wlc_lcn40phy_tx_pwr_update_npt(pi);
		wlc_lcn40phy_tempsense(pi, !TEMPER_VBAT_TRIGGER_NEW_MEAS);
	}
}

void
wlc_lcn40phy_get_tssi(phy_info_t *pi, int8 *ofdm_pwr, int8 *cck_pwr)
{
	int8 cck_offset;
	ppr_dsss_rateset_t dsss;
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	*cck_pwr = 0;
	*ofdm_pwr = 0;

	if (wlc_lcn40phy_tssi_based_pwr_ctrl_enabled(pi))	{
		if (phy_utils_read_phyreg(pi, LCN40PHY_TxPwrCtrlStatus)
			& LCN40PHY_TxPwrCtrlStatus_estPwrValid_MASK)
			*ofdm_pwr =
				(int8)(PHY_REG_READ(pi, LCN40PHY, TxPwrCtrlStatus, estPwr) >> 1);
		else if (phy_utils_read_phyreg(pi, LCN40PHY_TxPwrCtrlStatusNew2)
			& LCN40PHY_TxPwrCtrlStatusNew2_estPwrValid1_MASK)
			*ofdm_pwr = (int8)(PHY_REG_READ(pi,
				LCN40PHY, TxPwrCtrlStatusNew2, estPwr1) >> 1);
		ppr_get_dsss(pi->tx_power_offset, WL_TX_BW_20, WL_TX_CHAINS_1, &dsss);
		cck_offset = dsss.pwr[0];
		/* change to 6.2 */
		*cck_pwr = *ofdm_pwr + cck_offset;
	}
}

void
wlc_lcn40phy_tx_pu(phy_info_t *pi, bool bEnable)
{
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	if (!bEnable) {
		/* remove all overrides */
		PHY_REG_MOD(pi, LCN40PHY, AfeCtrlOvr1, dac_pu_ovr, 0);

		phy_utils_and_phyreg(pi, LCN40PHY_RFOverride0,
			~(uint16)(LCN40PHY_RFOverride0_gmode_tx_pu_ovr_MASK |
			LCN40PHY_RFOverride0_amode_tx_pu_ovr_MASK |
			LCN40PHY_RFOverride0_internalrftxpu_ovr_MASK |
			LCN40PHY_RFOverride0_trsw_rx_pu_ovr_MASK |
			LCN40PHY_RFOverride0_trsw_tx_pu_ovr_MASK |
			LCN40PHY_RFOverride0_ant_selp_ovr_MASK));

		phy_utils_and_phyreg(pi, LCN40PHY_rfoverride4,
			~(uint16)(LCN40PHY_rfoverride4_papu_ovr_MASK));

	} else {
		/* Force on DAC */
		PHY_REG_MOD(pi, LCN40PHY, AfeCtrlOvr1Val, dac_pu_ovr_val, 1);
		PHY_REG_MOD(pi, LCN40PHY, AfeCtrlOvr1, dac_pu_ovr, 1);

		/* Force on the transmit chain */
		phy_utils_mod_phyreg(pi, LCN40PHY_RFOverrideVal0,
			LCN40PHY_RFOverrideVal0_internalrftxpu_ovr_val_MASK |
			LCN40PHY_RFOverrideVal0_ant_selp_ovr_val_MASK,
			(1 << LCN40PHY_RFOverrideVal0_internalrftxpu_ovr_val_SHIFT) |
			(0 << LCN40PHY_RFOverrideVal0_ant_selp_ovr_val_SHIFT));

		phy_utils_mod_phyreg(pi, LCN40PHY_RFOverride0,
			LCN40PHY_RFOverride0_internalrftxpu_ovr_MASK |
			LCN40PHY_RFOverride0_ant_selp_ovr_MASK,
			(1 << LCN40PHY_RFOverride0_internalrftxpu_ovr_SHIFT) |
			(1 << LCN40PHY_RFOverride0_ant_selp_ovr_SHIFT));

		/* Force the TR switch to transmit */
		wlc_lcn40phy_set_trsw_override(pi, TRUE, FALSE);

		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			/* Force on the Gband-ePA */
			phy_utils_mod_phyreg(pi, LCN40PHY_RFOverrideVal0,
				LCN40PHY_RFOverrideVal0_gmode_tx_pu_ovr_val_MASK |
				LCN40PHY_RFOverrideVal0_amode_tx_pu_ovr_val_MASK,
				(1 << LCN40PHY_RFOverrideVal0_gmode_tx_pu_ovr_val_SHIFT) |
				(0 << LCN40PHY_RFOverrideVal0_amode_tx_pu_ovr_val_SHIFT));

			phy_utils_mod_phyreg(pi, LCN40PHY_RFOverride0,
				LCN40PHY_RFOverride0_gmode_tx_pu_ovr_MASK |
				LCN40PHY_RFOverride0_amode_tx_pu_ovr_MASK,
				(1 << LCN40PHY_RFOverride0_gmode_tx_pu_ovr_SHIFT) |
				(1 << LCN40PHY_RFOverride0_amode_tx_pu_ovr_SHIFT));

			/* PA PU */
			PHY_REG_MOD(pi, LCN40PHY, rfoverride4val, papu_ovr_val, 1);
			PHY_REG_MOD(pi, LCN40PHY, rfoverride4, papu_ovr, 1);

		}
#ifdef BAND5G
		else {
			/* Force off the Aband-ePA */
			phy_utils_mod_phyreg(pi, LCN40PHY_RFOverrideVal0,
				LCN40PHY_RFOverrideVal0_gmode_tx_pu_ovr_val_MASK |
				LCN40PHY_RFOverrideVal0_amode_tx_pu_ovr_val_MASK,
				(0 << LCN40PHY_RFOverrideVal0_gmode_tx_pu_ovr_val_SHIFT) |
				(1 << LCN40PHY_RFOverrideVal0_amode_tx_pu_ovr_val_SHIFT));

			phy_utils_mod_phyreg(pi, LCN40PHY_RFOverride0,
				LCN40PHY_RFOverride0_gmode_tx_pu_ovr_MASK |
				LCN40PHY_RFOverride0_amode_tx_pu_ovr_MASK,
				(1 << LCN40PHY_RFOverride0_gmode_tx_pu_ovr_SHIFT) |
				(1 << LCN40PHY_RFOverride0_amode_tx_pu_ovr_SHIFT));

			/* PA PU */
		}
#endif /* BAND5G */
	}
}

void
wlc_lcn40phy_write_table(phy_info_t *pi, const phytbl_info_t *pti)
{
	uint16 saved_reg = 0;
	if (pti->tbl_id == LCN40PHY_TBL_ID_TXPWRCTL) {
		if (LCN40REV_LT(pi->pubpi.phy_rev, 4)) {
			saved_reg = phy_utils_read_phyreg(pi, LCN40PHY_ClkEnCtrl);
			PHY_REG_MOD(pi, LCN40PHY, ClkEnCtrl, disable_stalls, 1);
		} else {
			saved_reg = phy_utils_read_phyreg(pi, LCN40PHY_StallCtrl);
			PHY_REG_MOD(pi, LCN40PHY, StallCtrl, disable_stalls, 1);
		}
	}

	phy_utils_write_phytable(pi, pti, LCN40PHY_TableAddress,
		LCN40PHY_TabledataHi, LCN40PHY_TabledataLo);

	if (pti->tbl_id == LCN40PHY_TBL_ID_TXPWRCTL) {
		if (LCN40REV_LT(pi->pubpi.phy_rev, 4)) {
			phy_utils_write_phyreg(pi, LCN40PHY_ClkEnCtrl, saved_reg);
		} else {
			phy_utils_write_phyreg(pi, LCN40PHY_StallCtrl, saved_reg);
		}

	}
}

void
wlc_lcn40phy_read_table(phy_info_t *pi, phytbl_info_t *pti)
{
	phy_utils_read_phytable(pi, pti, LCN40PHY_TableAddress,
	   LCN40PHY_TabledataHi, LCN40PHY_TabledataLo);
}
/* IOVAR */
bool
wlc_phy_tpc_iovar_isenabled_lcn40phy(phy_info_t *pi)
{
	return 	((phy_utils_read_phyreg((pi), LCN40PHY_TxPwrCtrlCmd) &
		(LCN40PHY_TxPwrCtrlCmd_txPwrCtrl_en_MASK |
		LCN40PHY_TxPwrCtrlCmd_hwtxPwrCtrl_en_MASK |
		LCN40PHY_TxPwrCtrlCmd_use_txPwrCtrlCoefs_MASK))
		== LCN40PHY_TX_PWR_CTRL_HW);
}

void
wlc_lcn40phy_iovar_txpwrctrl(phy_info_t *pi, int32 int_val, int32 *ret_int_ptr)
{
	uint16 pwrctrl;

	pwrctrl = int_val ? LCN40PHY_TX_PWR_CTRL_HW : LCN40PHY_TX_PWR_CTRL_OFF;

	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	phy_utils_phyreg_enter(pi);
	if (!int_val)
		wlc_lcn40phy_set_tx_pwr_by_index(pi, LCN40PHY_MAX_TX_POWER_INDEX);

	wlc_lcn40phy_set_tx_pwr_ctrl(pi, pwrctrl);

	pi->phy_forcecal = TRUE;
	phy_utils_phyreg_exit(pi);
	wlapi_enable_mac(pi->sh->physhim);
}

int
wlc_lcn40phy_idle_tssi_est_iovar(phy_info_t *pi, bool type)
{
	/* type = 0 will just do idle_tssi_est */
	/* type = 1 will do full tx_pwr_ctrl_init */

	if (!type)
		wlc_lcn40phy_idle_tssi_est(pi);
	else
		wlc_lcn40phy_tx_pwr_ctrl_init(pi);

	wlc_lcn40phy_set_tx_pwr_ctrl(pi, LCN40PHY_TX_PWR_CTRL_OFF);
	wlc_lcn40phy_set_tx_pwr_ctrl(pi, LCN40PHY_TX_PWR_CTRL_HW);

	return (phy_utils_read_phyreg(pi, LCN40PHY_TxPwrCtrlIdleTssi));
}

int wlc_lcn40phy_idle_tssi_reg_iovar(phy_info_t *pi, int32 int_val, bool set, int *err)
{
	uint16 perPktIdleTssi;
	uint16 idleTssi_2C;

	perPktIdleTssi = PHY_REG_READ(pi, LCN40PHY, TxPwrCtrlIdleTssi2, perPktIdleTssiUpdate_en);
	if (set) {
		if (perPktIdleTssi)
			*err = BCME_UNSUPPORTED; /* avgidletssi is not writeable */
		else
			PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlIdleTssi, idleTssi0, (uint16)int_val);
	}

	if (perPktIdleTssi)
		idleTssi_2C = PHY_REG_READ(pi, LCN40PHY, TxPwrCtrlStatusNew6, avgidletssi);
	else
		idleTssi_2C = PHY_REG_READ(pi, LCN40PHY, TxPwrCtrlIdleTssi, idleTssi0);
	return idleTssi_2C;
}

int wlc_lcn40phy_avg_tssi_reg_iovar(phy_info_t *pi)
{
	uint16 avgTssi_2C;
	avgTssi_2C = PHY_REG_READ(pi, LCN40PHY, TxPwrCtrlStatusNew4, avgTssi);
	return avgTssi_2C;
}

uint8
wlc_lcn40phy_get_bbmult_from_index(phy_info_t *pi, int indx)
{	phytbl_info_t tab;
	uint8 bb_mult;
	uint32 bbmultiqcomp;

	ASSERT(indx <= LCN40PHY_MAX_TX_POWER_INDEX);

	/* Preset txPwrCtrltbl */
	tab.tbl_id = LCN40PHY_TBL_ID_TXPWRCTL;
	tab.tbl_width = 32;	/* 32 bit wide	*/
	tab.tbl_len = 1;        /* # values   */

	/* Read index based bb_mult, a, b from the table */
	tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_IQ_OFFSET + indx; /* iqCoefLuts */
	tab.tbl_ptr = &bbmultiqcomp; /* ptr to buf */
	wlc_lcn40phy_read_table(pi,  &tab);

	/* Apply bb_mult */
	bb_mult = (uint8)((bbmultiqcomp >> 20) & 0xff);

	return bb_mult;
}

/* %%%%%% testing */
#if defined(BCMDBG) || defined(WLTEST)
int
wlc_phy_long_train_lcn40phy(phy_info_t *pi, int channel)
{

	uint16 num_samps;
	phytbl_info_t tab;

	/* stop any test in progress */
	wlc_phy_test_stop(pi);

	/* channel 0 means restore original contents and end the test */
	if (channel == 0) {
		wlc_lcn40phy_stop_tx_tone(pi);
		wlc_lcn40phy_deaf_mode(pi, FALSE);
		return 0;
	}

	if (wlc_phy_test_init(pi, channel, TRUE)) {
		return 1;
	}

	wlc_lcn40phy_deaf_mode(pi, TRUE);

	num_samps = sizeof(ltrn_list)/sizeof(*ltrn_list);
	/* load sample table */
	tab.tbl_ptr = ltrn_list;
	tab.tbl_len = num_samps;
	tab.tbl_id = LCN40PHY_TBL_ID_SAMPLEPLAY;
	tab.tbl_offset = 0;
	tab.tbl_width = 16;
	wlc_lcn40phy_write_table(pi, &tab);

	wlc_lcn40phy_run_samples(pi, num_samps, 0xffff, 0, 0);

	return 0;
}

void
wlc_phy_init_test_lcn40phy(phy_info_t *pi)
{
	/* Force WLAN antenna */
	wlc_btcx_override_enable(pi);
	/* Disable tx power control */
	wlc_lcn40phy_set_tx_pwr_ctrl(pi, LCN40PHY_TX_PWR_CTRL_OFF);
	/* Recalibrate for this channel */
	wlc_lcn40phy_calib_modes(pi, PHY_FULLCAL);
}
#endif // endif

#if defined(WLTEST)
void
wlc_phy_carrier_suppress_lcn40phy(phy_info_t *pi)
{

	wlc_lcn40phy_reset_radio_loft(pi);

	wlc_lcn40phy_clear_tx_power_offsets(pi);
}
static void
wlc_lcn40phy_reset_radio_loft(phy_info_t *pi)
{
	PHY_REG_LIST_START
		RADIO_REG_WRITE_ENTRY(RADIO_REG_EI(pi), 0x88)
		RADIO_REG_WRITE_ENTRY(RADIO_REG_EQ(pi), 0x88)
		RADIO_REG_WRITE_ENTRY(RADIO_REG_FI(pi), 0x88)
		RADIO_REG_WRITE_ENTRY(RADIO_REG_FQ(pi), 0x88)
	PHY_REG_LIST_EXECUTE(pi);
}

#endif // endif

void
wlc_lcn40phy_deaf_mode(phy_info_t *pi, bool mode)
{
	if (LCN40REV_LT(pi->pubpi.phy_rev, 4)) {
		phy_utils_write_phyreg(pi, LCN40PHY_crsgainCtrl_new, mode ? 0 : 0xFF);
		if (mode)
			PHY_REG_MOD(pi, LCN40PHY, agcControl4, c_agc_fsm_en, !mode);
		phy_utils_or_phyreg(pi, LCN40PHY_agcControl4, 1);
		if (mode)
			PHY_REG_MOD(pi, LCN40PHY, agcControl4, c_agc_fsm_en, !mode);
		if (LCN40REV_LT(pi->pubpi.phy_rev, 4))
			PHY_REG_MOD(pi, LCN40PHY, crsgainCtrl, crseddisable, mode);
		else {
			PHY_REG_MOD(pi, LCN40PHY, eddisable20ul, crseddisable_40, mode);
			PHY_REG_MOD(pi, LCN40PHY, eddisable20ul, crseddisable_20L, mode);
			PHY_REG_MOD(pi, LCN40PHY, eddisable20ul, crseddisable_20U, mode);
		}
	}
	else {
		PHY_REG_MOD(pi, LCN40PHY, agcControl4, c_agc_fsm_en, !mode);
	}

#ifdef FIXME
	PHY_REG_MOD(pi, LCN40PHY, rfoverride2, ext_lna_gain_ovr, mode);
	PHY_REG_MOD(pi, LCN40PHY, rfoverride2val, ext_lna_gain_ovr_val, 0);
	phy_utils_write_phyreg(pi, LCN40PHY_crsgainCtrl_new, mode ? 0 : 0xFF);
	PHY_REG_MOD(pi, LCN40PHY, agcControl4, c_agc_fsm_en, !mode);

	phy_utils_mod_phyreg((pi), LCN40PHY_crsgainCtrl,
		LCN40PHY_crsgainCtrl_DSSSDetectionEnable_MASK |
		LCN40PHY_crsgainCtrl_OFDMDetectionEnable_MASK,
		((CHSPEC_IS2G(pi->radio_chanspec)) ? (!mode) : 0) <<
		LCN40PHY_crsgainCtrl_DSSSDetectionEnable_SHIFT |
		(!mode) << LCN40PHY_crsgainCtrl_OFDMDetectionEnable_SHIFT);
	if (LCN40REV_LT(pi->pubpi.phy_rev, 4))
		PHY_REG_MOD(pi, LCN40PHY, crsgainCtrl, crseddisable, mode);
	else {
		PHY_REG_MOD(pi, LCN40PHY, eddisable20ul, crseddisable_40, mode);
		PHY_REG_MOD(pi, LCN40PHY, eddisable20ul, crseddisable_20L, mode);
		PHY_REG_MOD(pi, LCN40PHY, eddisable20ul, crseddisable_20U, mode);
	}
#endif // endif
}

static void
wlc_lcn40phy_tx_tone_samples(phy_info_t *pi, int32 f_Hz, uint16 max_val, uint32 *data_buf,
	uint32 phy_bw, uint16 num_samps)
{
	math_fixed theta = 0, rot = 0;
	uint16 i_samp, q_samp, t;
	math_cint32 tone_samp;
	/* set up params to generate tone */
	rot = FIXED((f_Hz * 36)/(phy_bw * 1000)) / 100; /* 2*pi*f/bw/1000  Note: f in KHz */
	theta = 0;			/* start angle 0 */

	/* tone freq = f_c MHz ; phy_bw = phy_bw MHz ; # samples = phy_bw (1us) ; max_val = 151 */
	/* TCL: set tone_buff [mimophy_gen_tone $f_c $phy_bw $phy_bw $max_val] */
	for (t = 0; t < num_samps; t++) {
		/* compute phasor */
		phy_utils_cordic(theta, &tone_samp);
		/* update rotation angle */
		theta += rot;
		/* produce sample values for play buffer */
		i_samp = (uint16)(FLOAT(tone_samp.i * max_val) & 0x3ff);
		q_samp = (uint16)(FLOAT(tone_samp.q * max_val) & 0x3ff);
		data_buf[t] = (i_samp << 10) | q_samp;
	}
}

static uint16
wlc_lcn40phy_num_samples(phy_info_t *pi, int32 f_Hz, uint32 phy_bw)
{
	uint16 num_samps, k;
	uint32 bw;

	/* allocate buffer */
	if (f_Hz) {
		k = 1;
		do {
			bw = phy_bw * 1000 * k * 1000;
			num_samps = bw / ABS(f_Hz);
			ASSERT(num_samps <= 256);
			k++;
		} while ((num_samps * (uint32)(ABS(f_Hz))) !=  bw);
	} else
		num_samps = 2;

	return num_samps;
}
/*
* Given a test tone frequency, continuously play the samples. Ensure that num_periods
* specifies the number of periods of the underlying analog signal over which the
* digital samples are periodic
*/
/* equivalent to lcn40phy_play_tone */
void
wlc_lcn40phy_start_tx_tone(phy_info_t *pi, int32 f_Hz, uint16 max_val, bool iqcalmode)
{
	uint8 phy_bw;
	uint16 num_samps;
	uint32 *data_buf;

	phytbl_info_t tab;

	if ((data_buf = LCN40PHY_MALLOC(pi, sizeof(uint32) * 256)) == NULL) {
		PHY_ERROR(("wl%d: %s: MALLOC failure\n", pi->sh->unit, __FUNCTION__));
		return;
	}

	/* Save active tone frequency */
	pi->phy_tx_tone_freq = f_Hz;

	PHY_REG_MOD(pi, LCN40PHY, AphyControlAddr, phyloopbackEn, 1);

	/* Turn off all the crs signals to the MAC */
	wlc_lcn40phy_deaf_mode(pi, TRUE);

	if (CHSPEC_IS20(pi->radio_chanspec))
		phy_bw = 40;
	else
		phy_bw = 80;

	num_samps = wlc_lcn40phy_num_samples(pi, f_Hz, phy_bw);

	PHY_INFORM(("wl%d: %s: %d Hz, %d samples\n",
		pi->sh->unit, __FUNCTION__,
		f_Hz, num_samps));

	if (num_samps > 256) {
		PHY_ERROR(("wl%d: %s: Too many samples to fit in SPB\n",
			pi->sh->unit, __FUNCTION__));
		LCN40PHY_MFREE(pi, data_buf, 256 * sizeof(uint32));
		return;
	}

	/* in LC40NPHY, we need to bring SPB out of standby before using it */
	PHY_REG_MOD(pi, LCN40PHY, sslpnCtrl3, sram_stby, 0);

	PHY_REG_MOD(pi, LCN40PHY, sslpnCalibClkEnCtrl, samplePlayClkEn, 1);

	wlc_lcn40phy_tx_tone_samples(pi, f_Hz, max_val, data_buf, phy_bw, num_samps);

	/* lcn40phy_load_sample_table */
	tab.tbl_ptr = data_buf;
	tab.tbl_len = num_samps;
	tab.tbl_id = LCN40PHY_TBL_ID_SAMPLEPLAY;
	tab.tbl_offset = 0;
	tab.tbl_width = 32;
	wlc_lcn40phy_write_table(pi, &tab);
	/* play samples from the sample play buffer */
	wlc_lcn40phy_run_samples(pi, num_samps, 0xffff, 0, iqcalmode);
	LCN40PHY_MFREE(pi, data_buf, 256 * sizeof(uint32));
}

void
wlc_lcn40phy_stop_tx_tone(phy_info_t *pi)
{
	int16 playback_status, mask;
	int cnt = 0;
	pi->phy_tx_tone_freq = 0;

	/* Stop sample buffer playback */
	playback_status = phy_utils_read_phyreg(pi, LCN40PHY_sampleStatus);
	mask = LCN40PHY_sampleStatus_NormalPlay_MASK | LCN40PHY_sampleStatus_iqlocalPlay_MASK;
	do {
		playback_status = phy_utils_read_phyreg(pi, LCN40PHY_sampleStatus);
		if (playback_status & LCN40PHY_sampleStatus_NormalPlay_MASK) {
			wlc_lcn40phy_tx_pu(pi, 0);
			PHY_REG_MOD(pi, LCN40PHY, sampleCmd, stop, 1);
		} else if (playback_status & LCN40PHY_sampleStatus_iqlocalPlay_MASK)
			PHY_REG_MOD(pi, LCN40PHY, iqloCalCmdGctl, iqlo_cal_en, 0);
		OSL_DELAY(1);
		playback_status = phy_utils_read_phyreg(pi, LCN40PHY_sampleStatus);
		cnt++;
	} while ((cnt < 10) && (playback_status & mask));

	ASSERT(!(playback_status & mask));

	PHY_REG_LIST_START
		/* put back SPB into standby */
		PHY_REG_WRITE_ENTRY(LCN40PHY, sslpnCtrl3, 1)
		/* disable clokc to spb */
		PHY_REG_MOD_ENTRY(LCN40PHY, sslpnCalibClkEnCtrl, samplePlayClkEn, 0)
		/* disable clock to txFrontEnd */
		PHY_REG_MOD_ENTRY(LCN40PHY, sslpnCalibClkEnCtrl, forceaphytxFeclkOn, 0)
	PHY_REG_LIST_EXECUTE(pi);

	/* Restore all the crs signals to the MAC */
	wlc_lcn40phy_deaf_mode(pi, FALSE);
	PHY_REG_MOD(pi, LCN40PHY, AphyControlAddr, phyloopbackEn, 0);
}

static void
wlc_lcn40phy_play_sample_table1(phy_info_t *pi, int32 f_Hz, uint16 max_val)
{
	uint8 phy_bw;
	uint16 num_samps;
	uint32 *data_buf;

	phytbl_info_t tab;

	if ((data_buf = LCN40PHY_MALLOC(pi, sizeof(uint32) * 256)) == NULL) {
		PHY_ERROR(("wl%d: %s: MALLOC failure\n", pi->sh->unit, __FUNCTION__));
		return;
	}

	/* Save active tone frequency */
	pi->phy_tx_tone_freq = f_Hz;

	if (CHSPEC_IS20(pi->radio_chanspec))
		phy_bw = 40;
	else
		phy_bw = 80;

	num_samps = wlc_lcn40phy_num_samples(pi, f_Hz, phy_bw);

	PHY_REG_MOD(pi, LCN40PHY, sampleCmd, buf2foriqlocal, 1);
	PHY_REG_MOD(pi, LCN40PHY, sampleDepthCount, DepthCount1, num_samps - 1);

	PHY_INFORM(("wl%d: %s: %d Hz, %d samples\n",
		pi->sh->unit, __FUNCTION__,
		f_Hz, num_samps));

	if (num_samps > 256) {
		PHY_ERROR(("wl%d: %s: Too many samples to fit in SPB\n",
			pi->sh->unit, __FUNCTION__));
		LCN40PHY_MFREE(pi, data_buf, 256 * sizeof(uint32));
		return;
	}

	wlc_lcn40phy_tx_tone_samples(pi, f_Hz, max_val, data_buf, phy_bw, num_samps);

	/* lcn40phy_load_sample_table */
	tab.tbl_ptr = data_buf;
	tab.tbl_len = num_samps;
	tab.tbl_id = LCN40PHY_TBL_ID_SAMPLEPLAY1;
	tab.tbl_offset = 0;
	tab.tbl_width = 32;
	wlc_lcn40phy_write_table(pi, &tab);
	LCN40PHY_MFREE(pi, data_buf, 256 * sizeof(uint32));
}

void
wlc_lcn40phy_set_tx_tone_and_gain_idx(phy_info_t *pi)
{
	int8 curr_pwr_idx_val;

	/* Force WLAN antenna */
	wlc_btcx_override_enable(pi);

	if (LCN40PHY_TX_PWR_CTRL_OFF != wlc_lcn40phy_get_tx_pwr_ctrl(pi)) {
		curr_pwr_idx_val = wlc_lcn40phy_get_current_tx_pwr_idx(pi);
		wlc_lcn40phy_set_tx_pwr_by_index(pi, (int)curr_pwr_idx_val);
}

	phy_utils_write_phyreg(pi, LCN40PHY_sslpnCalibClkEnCtrl, 0xffff);
	wlc_lcn40phy_start_tx_tone(pi, pi->phy_tx_tone_freq, 120, 0); /* play tone */
}
void
wlc_lcn40phy_crsuprs(phy_info_t *pi, int channel)
{
	uint16 afectrlovr, afectrlovrval;
	afectrlovr = phy_utils_read_phyreg(pi, LCN40PHY_AfeCtrlOvr1);
	afectrlovrval = phy_utils_read_phyreg(pi, LCN40PHY_AfeCtrlOvr1Val);
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	if (channel != 0) {
		PHY_REG_LIST_START
			PHY_REG_MOD_ENTRY(LCN40PHY, AfeCtrlOvr1, dac_pu_ovr, 1)
			PHY_REG_MOD_ENTRY(LCN40PHY, AfeCtrlOvr1Val, dac_pu_ovr_val, 1)
			PHY_REG_WRITE_ENTRY(LCN40PHY, ClkEnCtrl, 0xffff)
		PHY_REG_LIST_EXECUTE(pi);
		wlc_lcn40phy_tx_pu(pi, 1);
		PHY_REG_LIST_START
			/* Turn ON bphyframe */
			PHY_REG_MOD_ENTRY(LCN40PHY, BphyControl3, bphyFrmStartCntValue, 0)
			/* Turn on Tx Front End clks */
			PHY_REG_OR_ENTRY(LCN40PHY, sslpnCalibClkEnCtrl, 0x0080)
			/* start the rfcs signal */
			PHY_REG_OR_ENTRY(LCN40PHY, bphyTest, 0x228)
		PHY_REG_LIST_EXECUTE(pi);
	} else {
		phy_utils_and_phyreg(pi, LCN40PHY_bphyTest, ~(0x228));
		/* disable clk to txFrontEnd */
		phy_utils_and_phyreg(pi, LCN40PHY_sslpnCalibClkEnCtrl, 0xFF7F);
		phy_utils_write_phyreg(pi, LCN40PHY_AfeCtrlOvr1, afectrlovr);
		phy_utils_write_phyreg(pi, LCN40PHY_AfeCtrlOvr1Val, afectrlovrval);
	}
}

void
wlc_lcn40phy_dummytx(wlc_phy_t *ppi, uint16 nframes, uint16 wait_delay)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	uint8 counter = 0;
	uint16 max_pwr_idx = 0;
	uint16 min_pwr_idx = 127;
	uint16 current_txidx = 0;
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);

	wlc_btcx_override_enable(pi);
	wlc_lcn40phy_deaf_mode(pi, TRUE);

	for (counter = 0; counter < nframes; counter ++) {
		wlc_phy_do_dummy_tx(pi, TRUE, OFF);
		OSL_DELAY(wait_delay);
		current_txidx = wlc_lcnphy_get_current_tx_pwr_idx(pi);
		if (current_txidx > max_pwr_idx)
			max_pwr_idx = current_txidx;
		if (current_txidx < min_pwr_idx)
			min_pwr_idx = current_txidx;
	}

	wlc_lcn40phy_deaf_mode(pi, FALSE);

	pi_lcn->lcnphy_start_idx = (uint8)current_txidx; 	/* debug information */
}

void
wlc_lcn40phy_papd_recal(phy_info_t *pi)
{

	uint16 tx_pwr_ctrl;
	bool suspend;
	uint16 current_txidx = 0;
	phy_txcalgains_t txgains;
	phy_info_lcnphy_t *pi_lcn;
	uint16 nframes;
	uint16 wait_delay;
	uint8 npt;

	nframes = 50;
	wait_delay = 10*100;

	phy_utils_phyreg_enter(pi);
	pi_lcn = wlc_phy_getlcnphy_common(pi);
#ifndef PHYCAL_CACHING
	pi_lcn->lcnphy_full_cal_channel = CHSPEC_CHANNEL(pi->radio_chanspec);
#endif // endif
	suspend = !(R_REG(pi->sh->osh, &pi->regs->maccontrol) & MCTL_EN_MAC);
	if (!suspend) {
		/* Set non-zero duration for CTS-to-self */
		wlapi_bmac_write_shm(pi->sh->physhim, M_CTS_DURATION, 10000);
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
	}

	/* Save npt */
	npt = wlc_lcn40phy_get_tx_pwr_npt(pi);

	/* Save tx power control mode */
	tx_pwr_ctrl = wlc_lcn40phy_get_tx_pwr_ctrl(pi);

	/* Disable tx power control */
	wlc_lcn40phy_set_tx_pwr_ctrl(pi, LCN40PHY_TX_PWR_CTRL_OFF);

	/* Enable pwr ctrl */
	wlc_lcn40phy_set_tx_pwr_ctrl(pi, LCN40PHY_TX_PWR_CTRL_HW);

	/* clear all offsets */
	wlc_lcn40phy_clear_tx_power_offsets(pi);

	/* set target pwr for papd */
	wlc_lcn40phy_set_target_tx_pwr(pi, 64);

	/* Setting npt to 0 for index settling with 30 frames */
	wlc_lcn40phy_set_tx_pwr_npt(pi, 0);

	PHY_TRACE(("dummy TX Start Called\n"));

	/* transmit dummy tx pkts and find the txidx */
	wlc_lcn40phy_dummytx((wlc_phy_t *)pi, nframes, wait_delay);

	current_txidx = pi_lcn->lcnphy_start_idx; 	/* debug information */

	wlc_lcn40phy_deaf_mode(pi, TRUE);

	wlc_btcx_override_enable(pi);

	/* Restore npt */
	wlc_lcn40phy_set_tx_pwr_npt(pi, npt);

	/* Disable tx power control */
	wlc_lcn40phy_set_tx_pwr_ctrl(pi, LCN40PHY_TX_PWR_CTRL_OFF);

	if (pi_lcn->lcnphy_CalcPapdCapEnable == 1)
	{
		wlc_lcn40phy_papd_calc_capindex(pi, &txgains);
		wlc_lcn40phy_load_txgainwithcappedindex(pi, 1);
		/* set target pwr for papd = 15.5 dbm */
		wlc_lcn40phy_set_target_tx_pwr(pi, 62);

		/* Setting npt to 0 for index settling with 30 frames */
		wlc_lcn40phy_set_tx_pwr_npt(pi, 0);

		PHY_TRACE(("dummy TX Start Called\n"));

		/* transmit dummy tx pkts and find the txidx */
		wlc_lcn40phy_dummytx((wlc_phy_t *)pi, nframes, wait_delay);

		current_txidx = pi_lcn->lcnphy_start_idx; 	/* debug information */
		if (current_txidx == 0)
		{
			pi_lcn->lcnphy_capped_index += 4;
			wlc_lcn40phy_load_txgainwithcappedindex(pi, 1);

		}
	}
	else
	{
		txgains.index = (uint8) current_txidx;
		txgains.useindex = 1;
		/* run papd corresponding to the target pwr */
		wlc_lcn40phy_papd_cal(pi, PHY_PAPD_CAL_CW, &txgains, 0, 0, 0, 0, 219, 1, 0);
	}

	/* Restore tx power control */
	wlc_lcn40phy_set_tx_pwr_ctrl(pi, tx_pwr_ctrl);

	/* Reset radio ctrl and crs gain */
	phy_utils_or_phyreg(pi, LCN40PHY_resetCtrl, 0x47);
	phy_utils_write_phyreg(pi, LCN40PHY_resetCtrl, 0x0);

	if (!suspend)
		wlapi_enable_mac(pi->sh->physhim);

	wlc_lcn40phy_deaf_mode(pi, FALSE);
	phy_utils_phyreg_exit(pi);
}

static void
wlc_lcn40phy_btc_adjust(phy_info_t *pi, bool btactive)
{
	int btc_mode = wlapi_bmac_btc_mode_get(pi->sh->physhim);
	bool suspend = !(R_REG(pi->sh->osh, &((phy_info_t *)pi)->regs->maccontrol) & MCTL_EN_MAC);
	phy_lcn40_info_t *lcn40i = pi->u.pi_lcn40phy;

	/* for dual antenna design, a gain table switch is to ensure good performance
	 * in simultaneous WLAN RX
	 */
	if (pi->aa2g > 2) {
		if (!suspend)
			wlapi_suspend_mac_and_wait(pi->sh->physhim);
		if (LCN40REV_IS(pi->pubpi.phy_rev, 3)) {
			if (btactive && !pi->bt_active) {
				phy_lcn40_antdiv_set_rx(lcn40i->antdivi, 0);
				PHY_REG_MOD(pi, LCN40PHY, RFOverride0, ant_selp_ovr, 1);
				PHY_REG_MOD(pi, LCN40PHY, RFOverrideVal0, ant_selp_ovr_val, 0);
				wlc_lcn40phy_write_table(pi, dot11lcn40_gain_tbl_2G_info_rev3_BT);
			} else if (!btactive && pi->bt_active) {
				phy_lcn40_antdiv_set_rx(lcn40i->antdivi, pi->sh->rx_antdiv);
				PHY_REG_MOD(pi, LCN40PHY, RFOverride0, ant_selp_ovr, 0);
				wlc_lcn40phy_write_table(pi, dot11lcn40_gain_tbl_2G_info_rev3);
			}
		}
		if ((btc_mode == WL_BTC_PARALLEL) || (btc_mode == WL_BTC_LITE)) {
			if (btactive) {
				pi->u.pi_lcn40phy->btc_clamp = TRUE;
				wlc_lcn40phy_set_txpwr_clamp(pi);
			} else if (pi->u.pi_lcn40phy->btc_clamp) {
				pi->u.pi_lcn40phy->btc_clamp = FALSE;
				wlc_phy_txpower_recalc_target_lcn40phy(pi);
			}
		}
		if (!suspend)
			wlapi_enable_mac(pi->sh->physhim);
	}

}

void wlc_lcn40phy_read_papdepstbl(phy_info_t *pi, struct bcmstrbuf *b)
{

	phytbl_info_t tab;
	uint32 val, j;
	int32 eps_real, eps_imag;
	/* Save epsilon table */
	tab.tbl_id = LCN40PHY_TBL_ID_PAPDCOMPDELTATBL;
	tab.tbl_ptr = &val; /* ptr to buf */
	tab.tbl_width = 32;
	tab.tbl_len = 1;        /* # values   */

	bcm_bprintf(b, "\n");
	for (j = 0; j < PHY_PAPD_EPS_TBL_SIZE_LCNPHY; j++) {
		tab.tbl_offset = j;
		wlc_lcnphy_read_table(pi, &tab);
		eps_real = (val & 0x00fff000) << 8;
	        eps_imag = (val & 0x00000fff) << 20;
		eps_real = eps_real >> 20;
		eps_imag = eps_imag >> 20;
		bcm_bprintf(b, "%d \t %d \n", eps_real, eps_imag);
	}
	bcm_bprintf(b, "\n");
}

static void
wlc_lcn40phy_radioid_set_reg(phy_info_t *pi, uint16 val)
{
	uint16 lcnphy_shm_ptr;
	lcnphy_shm_ptr = wlapi_bmac_read_shm(pi->sh->physhim, M_LCN40PHYREGS_PTR);
	wlapi_bmac_write_shm(pi->sh->physhim,
		2*(lcnphy_shm_ptr + M_RADIOID_L_OFFSET), val);
}

static void
WLBANDINITFN(wlc_lcn40phy_reg_init)(phy_info_t *pi)
{
	if (LCN40REV_IS(pi->pubpi.phy_rev, 0) || LCN40REV_IS(pi->pubpi.phy_rev, 2) ||
		LCN40REV_IS(pi->pubpi.phy_rev, 4) || LCN40REV_IS(pi->pubpi.phy_rev, 5) ||
		LCN40REV_IS(pi->pubpi.phy_rev, 6) || LCN40REV_IS(pi->pubpi.phy_rev, 7))
			wlc_lcn40phy_rev0_reg_init(pi);
	else if (LCN40REV_IS(pi->pubpi.phy_rev, 1) || LCN40REV_IS(pi->pubpi.phy_rev, 3))
		wlc_lcn40phy_rev1_reg_init(pi);
	wlc_lcn40phy_bu_tweaks(pi);
}

static void
WLBANDINITFN(wlc_lcn40phy_baseband_init)(phy_info_t *pi)
{
	PHY_TRACE(("%s:***CHECK***\n", __FUNCTION__));
	/* Initialize LCN40PHY tables */
	wlc_lcn40phy_tbl_init(pi);
	wlc_lcn40phy_reg_init(pi);
	wlc_lcn40phy_set_tx_pwr_by_index(pi, 40);
	wlc_lcn40phy_noise_init(pi);
	if ((CHIPID(pi->sh->chip) == BCM43340_CHIP_ID) ||
		(CHIPID(pi->sh->chip) == BCM43341_CHIP_ID) ||
		(CHIPID(pi->sh->chip) == BCM43142_CHIP_ID)) {
		wlc_lcn40phy_aci_init(pi);
	}
#ifdef WLC_SW_DIVERSITY
	wlc_lcn40phy_swdiv_init(pi);
#endif /* WLC_SW_DIVERSITY */
}

/* mapped onto lcn40phy_rev0_rf_init proc */
static void
WLBANDINITFN(wlc_radio_2065_init)(phy_info_t *pi)
{
	uint32 i;
	lcn40phy_radio_regs_t *lcn40phyregs = NULL;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	/* init radio regs */
	if (RADIOID(pi->pubpi.radioid) == BCM2067_ID) {
		/* rev60/64 table only differs in logen5g_idac setting */
		/* share 1 table then hack it later */
		if (LCN40REV_IS(pi->pubpi.phy_rev, 7)) {
			/* init radio regs */
			wlc_lcn40phy_radioid_set_reg(pi, pi->pubpi.radiover);
			lcn40phyregs = lcn40phy_radio_regs_2067_rev64;
		} else if (LCN40REV_LE(pi->pubpi.phy_rev, 6))
			lcn40phyregs = lcn40phy_radio_regs_2067;
		else {
			ASSERT(0);
			return;
		}
	} else if (RADIOID(pi->pubpi.radioid) == BCM2065_ID) {
		if (RADIOVER(pi->pubpi.radiover) == 2)
			lcn40phyregs = lcn40phy_radio_regs_2065_rev2;
		else
			lcn40phyregs = lcn40phy_radio_regs_2065;
	} else {
		ASSERT(0);
		return;
	}
	for (i = 0; lcn40phyregs[i].address != 0xffff; i++) {
#ifdef BAND5G
		if (CHSPEC_IS5G(pi->radio_chanspec))
			phy_utils_write_radioreg(pi,
				((lcn40phyregs[i].address & 0x3fff) |
				RADIO_DEFAULT_CORE),
				(uint16)lcn40phyregs[i].init_g);
		else
#endif /* BAND5G */
		phy_utils_write_radioreg(pi,
			((lcn40phyregs[i].address & 0x3fff) |
			RADIO_DEFAULT_CORE),
			(uint16)lcn40phyregs[i].init_g);
	}
	if (LCN40REV_IS(pi->pubpi.phy_rev, 1) || LCN40REV_IS(pi->pubpi.phy_rev, 3)) {
		/* tweaks for 4314/43142 only */
		phy_utils_mod_radioreg(pi, RADIO_2065_TIA_CFG2, 0x7, 0x4);
		/* Reduce LDO vlotage by one notch on A1 to reduce reliability risk */
		if (CHIPREV(pi->sh->chiprev) < 1)
			phy_utils_mod_radioreg(pi, RADIO_2065_PMU_CFG2, 0x3333, 0x1103);
		else
			phy_utils_mod_radioreg(pi, RADIO_2065_PMU_CFG2, 0x3333, 0x1102);
	} else if (LCN40REV_IS(pi->pubpi.phy_rev, 6)) {
		PHY_REG_LIST_START
			/* PLL settings and tweaks for 43143 only */
			RADIO_REG_MOD_ENTRY(RADIO_2065_TIA_CFG2, 0x7, 0x4)
			RADIO_REG_MOD_ENTRY(RADIO_2065_PMU_CFG2, 0x3333, 0x1102)
			RADIO_REG_MOD_ENTRY(RADIO_2065_XTAL_CFG3, 0x6, 0x0)
		PHY_REG_LIST_EXECUTE(pi);
		if (RADIOREV(pi->pubpi.radiorev) == 3) {
			/* 43143b0: TRSW gate bias change */
			phy_utils_mod_radioreg(pi, RADIO_2065_TRSW2G_CFG2, 0xf00, 0x4 << 8);
		}
	} else if (LCN40REV_IS(pi->pubpi.phy_rev, 7)) {
		phy_utils_mod_radioreg(pi, RADIO_2065_SHADOWACCESS, 0x4, 0x4);

		if (CHSPEC_IS2G(pi->radio_chanspec) && CHSPEC_IS20(pi->radio_chanspec)) {
			/* Tweaks for better 2G 20MHz NF */
			PHY_REG_LIST_START
				RADIO_REG_MOD_ENTRY(RADIO_2065_PMU_CFG2, 0x7000, 2 << 12)
				RADIO_REG_MOD_ENTRY(RADIO_2065_OVR11, 0x3000, 0x3000)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RXMIX2G_CFG1, 0x302, 0x202)
				RADIO_REG_MOD_ENTRY(RADIO_2065_OVR3, 0x2000, 1 << 13)
				RADIO_REG_MOD_ENTRY(RADIO_2065_LNA2G_IDAC1, 0xf, 0xa)
			PHY_REG_LIST_EXECUTE(pi);
		} else {
			/* Roll back to default setting */
			PHY_REG_LIST_START
				RADIO_REG_MOD_ENTRY(RADIO_2065_PMU_CFG2, 0x7000, 0 << 12)
				RADIO_REG_MOD_ENTRY(RADIO_2065_OVR11, 0x3000, 0)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RXMIX2G_CFG1, 0x302, 0x100)
				RADIO_REG_MOD_ENTRY(RADIO_2065_OVR3, 0x2000, 0 << 13)
				RADIO_REG_MOD_ENTRY(RADIO_2065_LNA2G_IDAC1, 0xf, 0x4)
			PHY_REG_LIST_EXECUTE(pi);
		}
	}
	/* setting tx lo coefficients to 0 */
	wlc_lcn40phy_set_tx_locc(pi, 0);
}

static void
WLBANDINITFN(wlc_lcn40phy_radio_init)(phy_info_t *pi)
{
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	if (NORADIO_ENAB(pi->pubpi))
		return;
	/* Initialize 2065 radio */

	wlc_radio_2065_init(pi);
}

static void
wlc_lcn40phy_radio_reset(phy_info_t *pi)
{
	if (NORADIO_ENAB(pi->pubpi))
		return;
	/* reset the radio, bit 8. do not reset for rev0 due to HSIC issue */
	if (!(LCN40REV_IS(pi->pubpi.phy_rev, 0)))
	{
		phy_utils_or_phyreg(pi, LCN40PHY_resetCtrl, 0x100);
		phy_utils_and_phyreg(pi, LCN40PHY_resetCtrl, ~0x100);
	}
}

static void
wlc_lcn40phy_rcal(phy_info_t *pi)
{

	if (NORADIO_ENAB(pi->pubpi))
		return;
	phy_utils_mod_radioreg(pi, RADIO_2065_OVR8, 0x8000, 0x0);
	phy_utils_or_radioreg(pi, RADIO_2065_OVR2, 1 << 6);
	phy_utils_mod_radioreg(pi, RADIO_2065_BG_CFG1, 0xf0, pi->u.pi_lcn40phy->rcal << 4);

#ifdef BRINGUPDONE
	phy_utils_mod_radioreg(pi, RADIO_2065_BG_CFG1, 0xf0, 0xa << 4);
	phy_utils_mod_radioreg(pi, RADIO_2065_OVR8, 0x8000, 0x0);
	phy_utils_or_radioreg(pi, RADIO_2065_OVR2, 1 << 6);

	/* power down set reg(RF_rcal_cfg.pu) 0 */
	phy_utils_mod_radioreg(pi, RADIO_2065_RCAL_CFG, 0x1, 0);
	/* Enable Pwrsw (Needed ?)
	   set reg(RF_pmu_op.misc_pwrsw_en) 1
	   set reg(RF_OVR7.ovr_misc_pwrsw_en) 1
	*/
	phy_utils_or_radioreg(pi, RADIO_2065_PMU_OP, 0x40);
	phy_utils_or_radioreg(pi, RADIO_2065_OVR7, 1);
	/* Enable RCAL Clock
	   set reg(RF_xtal_cfg1.pll_clock_rcal_clk_pu) 1
	*/
	phy_utils_or_radioreg(pi, RADIO_2065_XTAL_CFG1, 0x8000);
	/* Power up RCAL
	   set reg(RF_rcal_cfg.pu) 1
	*/
	phy_utils_or_radioreg(pi, RADIO_2065_RCAL_CFG, 1);
	OSL_DELAY(5000);
	SPINWAIT(!wlc_radio_2065_rcal_done(pi), 10 * 1000 * 1000);
	/* wait for RCAL valid bit to be set */
	if (wlc_radio_2065_rcal_done(pi)) {
		PHY_INFORM(("wl%d: %s:  Rx RCAL completed, code: %x\n",
			pi->sh->unit, __FUNCTION__,
			phy_utils_read_radioreg(pi, RADIO_2065_RCAL_CFG) & 0x1f));
	} else {
		PHY_ERROR(("wl%d: %s: RCAL failed\n", pi->sh->unit, __FUNCTION__));
	}
	/* power down RCAL
	   set reg(RF_rcal_cfg.pu) 0
	*/
	phy_utils_mod_radioreg(pi, RADIO_2065_RCAL_CFG, 0x1, 0);
	/* Disable RCAL Clock
	   set reg(RF_xtal_cfg1.pll_clock_rcal_clk_pu) 0
	*/
	phy_utils_mod_radioreg(pi, RADIO_2065_XTAL_CFG1, 0x8000, 0);
#endif /* BRINGUPDONE */
}

static void
wlc_lcn40phy_restore_rc_cal(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);

	phy_utils_write_phyreg(pi, LCN40PHY_lpf_rccal_tbl_1, pi_lcn->lpf_rccal_tbl[0]);
	phy_utils_write_phyreg(pi, LCN40PHY_lpf_rccal_tbl_2, pi_lcn->lpf_rccal_tbl[1]);
	phy_utils_write_phyreg(pi, LCN40PHY_lpf_rccal_tbl_3, pi_lcn->lpf_rccal_tbl[2]);
	phy_utils_write_phyreg(pi, LCN40PHY_lpf_rccal_tbl_4, pi_lcn->lpf_rccal_tbl[3]);
	phy_utils_write_phyreg(pi, LCN40PHY_lpf_rccal_tbl_5, pi_lcn->lpf_rccal_tbl[4]);
	phy_utils_write_phyreg(pi, LCN40PHY_lpf_rccal_tbl_6, pi_lcn->lpf_rccal_tbl[5]);
	phy_utils_write_phyreg(pi, LCN40PHY_lpf_rccal_tbl_7, pi_lcn->lpf_rccal_tbl[6]);
	if (LCN40REV_GE(pi->pubpi.phy_rev, 4)) {
		phy_utils_write_phyreg(pi, LCN40PHY_lpf_rccal_tbl_8, pi_lcn->lpf_rccal_tbl[7]);
		phy_utils_write_phyreg(pi, LCN40PHY_lpf_rccal_tbl_9, pi_lcn->lpf_rccal_tbl[8]);
		phy_utils_write_phyreg(pi, LCN40PHY_lpf_rccal_tbl_10, pi_lcn->lpf_rccal_tbl[9]);
	}
}

static void
wlc_lcn40phy_rc_cal(phy_info_t *pi)
{
	uint16 old_rccal_big, old_rccal_small, old_rccal_adc;
	uint16 old_rccal_hpc, hpc_code_big, hpc_code_small, hpc_code_adc, hpc_offset;
	uint16 b_cap_rc_code_raw, s_cap_rc_code_raw, a_cap_rc_code_raw;
	uint16 flt_val, trc, c1, c2, c3, c4;
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);

	if (NORADIO_ENAB(pi->pubpi))
		return;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	if (PHY_XTALFREQ(pi->xtalfreq) / 1000 == 37400) {
		if (LCN40REV_GE(pi->pubpi.phy_rev, 4))
			trc = 0x16e;
		else
			trc = 0x164;
		hpc_code_big = hpc_code_small = 0x8;
		hpc_code_adc = 2;
		hpc_offset = 0;
	} else if (PHY_XTALFREQ(pi->xtalfreq) / 1000 == 20000) {
		if (LCN40REV_GE(pi->pubpi.phy_rev, 4))
			trc = 0xbe;
		else
			trc = 0xc6;

		hpc_code_big = hpc_code_small = 0x8;
		hpc_code_adc = 2;
		hpc_offset = 0;
	} else if ((PHY_XTALFREQ(pi->xtalfreq) / 1000 == 26000) &&
		LCN40REV_IS(pi->pubpi.phy_rev, 7)) {
		trc = 0xfb;
		hpc_code_big = hpc_code_small = 0x8;
		hpc_code_adc = 2;
		hpc_offset = 0;
	} else {
		PHY_INFORM(("wl%d: %s:  RCCAL not run, xtal "
					"%d not known\n",
					pi->sh->unit, __FUNCTION__, PHY_XTALFREQ(pi->xtalfreq)));
		return;
	}

	/*    #Save old value of rccal_clk_pu
		  set Old_xtal_cfg1_pll_clock_rccal_clk_pu
		  $reg(RF_xtal_cfg1.pll_clock_rccal_clk_pu)
	*/
	/*     #Save old HPC value incase RCCal fails
		   set Old_RCCAL_HPC $reg(RF_tia_cfg3.rccal_hpc)
	*/
	old_rccal_hpc = phy_utils_read_radioreg(pi, RADIO_2065_TIA_CFG3) & 0x1F;
	/* 	set reg(RF_rccal_cfg.rccal_mode) 1 */
	phy_utils_or_radioreg(pi, RADIO_2065_RCCAL_CFG, 0x2);
	/* 	### Big cap
		#Save old Big Cap value incase RCCal fails
		set Old_RCCAL_big $reg(RF_rccal_logic5.rccal_raw_big)
	*/
	old_rccal_big = phy_utils_read_radioreg(pi, RADIO_2065_RCCAL_LOGIC5) & 0x1F00;

	PHY_REG_LIST_START
		/* 	#  power down rccal
			set reg(RF_rccal_cfg.pu) 0
		*/
		RADIO_REG_MOD_ENTRY(RADIO_2065_RCCAL_LOGIC1, 1, 0)
		RADIO_REG_MOD_ENTRY(RADIO_2065_RCCAL_CFG, 0x1, 0)
		RADIO_REG_OR_ENTRY(RADIO_2065_RCCAL_CFG, 0x1)
		/*     #Enable Pwrsw (Needed ?)
			   set reg(RF_pmu_op.misc_pwrsw_en) 1
			   set reg(RF_OVR7.ovr_misc_pwrsw_en) 1
		*/
		RADIO_REG_OR_ENTRY(RADIO_2065_PMU_OP, 0x40)
		RADIO_REG_OR_ENTRY(RADIO_2065_OVR7, 0x1)
		/* 	#Enable RCCal clock
			set reg(RF_xtal_cfg1.pll_clock_rccal_clk_pu) 1
		*/
		RADIO_REG_OR_ENTRY(RADIO_2065_XTAL_CFG1, 0x4000)
	PHY_REG_LIST_EXECUTE(pi);

	/*     #Set Trc
		   set reg(RF_rccal_trc.rccal_Trc) $Trc
		   #RCCAL on Big Cap first
		   set reg(RF_rccal_cfg.sc) 1
	*/
	phy_utils_mod_radioreg(pi, RADIO_2065_RCCAL_TRC, 0x1FFF, trc);
	phy_utils_mod_radioreg(pi, RADIO_2065_RCCAL_CFG, 0x18, 0);
	/*     #setup to run RX RC Cal and setup R1/Q1/P1
		   set reg(RF_rccal_logic1.rccal_P1) 0x1

		   set reg(RF_rccal_logic1.rccal_Q1) 0x1

		   set reg(RF_rccal_logic1.rccal_R1) 0x1

		   #Set X1
		   set reg(RF_rccal_logic1.rccal_X1) 0x63
		   #Start RCCAL
		   set reg(RF_rccal_logic1.rccal_START) 1
	*/
	phy_utils_mod_radioreg(pi, RADIO_2065_RCCAL_LOGIC1, 0xFFFD, 0x6355);

	/* check to see if RC CAL is done */
	OSL_DELAY(50);
	SPINWAIT(!wlc_radio_2065_rc_cal_done(pi), 10 * 1000 * 1000);
	if (!wlc_radio_2065_rc_cal_done(pi)) {
		PHY_ERROR(("wl%d: %s: Big Cap RC Cal failed\n", pi->sh->unit, __FUNCTION__));
		b_cap_rc_code_raw = old_rccal_big;
		hpc_code_big = old_rccal_hpc;
	} else {
		/* RCCAL successful */
		b_cap_rc_code_raw =
		        (phy_utils_read_radioreg(pi, RADIO_2065_RCCAL_LOGIC5) &	0x1F00) >> 8;
		phy_utils_mod_radioreg(pi, RADIO_2065_TIA_CFG3, 0x1f, b_cap_rc_code_raw - 4);
		if (b_cap_rc_code_raw < 0x1E)
			hpc_code_big = b_cap_rc_code_raw + hpc_offset;
		PHY_INFORM(("wl%d: %s:  Big Rx RC Cal completed for "
			"Trc: %x, N0: %x, N1: %x, b_cap_code_raw: %x, hpc_code: %x\n",
			pi->sh->unit, __FUNCTION__, trc,
			phy_utils_read_radioreg(pi, RADIO_2065_RCCAL_LOGIC3) & 0x1FFF,
			phy_utils_read_radioreg(pi, RADIO_2065_RCCAL_LOGIC4) & 0x1FFF,
			b_cap_rc_code_raw, hpc_code_big));
	}
	/* RCCAL on small cap */
	/* Save old small cap value in case RCCal fails */
	old_rccal_small = phy_utils_read_radioreg(pi, RADIO_2065_RCCAL_LOGIC5) & 0x1F;

	PHY_REG_LIST_START
		/* Stop RCCAL */
		RADIO_REG_MOD_ENTRY(RADIO_2065_RCCAL_LOGIC1, 0x1, 0)
		/* Power down RC CAL */
		RADIO_REG_MOD_ENTRY(RADIO_2065_RCCAL_CFG, 0x1, 0)
		/* Power up RC Cal */
		/* 	#RCCAL on Small Cap */
		RADIO_REG_MOD_ENTRY(RADIO_2065_RCCAL_CFG, 0x19, 1 | (1 << 3))
	PHY_REG_LIST_EXECUTE(pi);

	/* #Set Trc
	  set reg(RF_rccal_trc.rccal_Trc) $Trc
	*/
	phy_utils_mod_radioreg(pi, RADIO_2065_RCCAL_TRC, 0x1FFF, trc);

	/* set reg(RF_rccal_logic1.rccal_X1) 0x6F
	set reg(RF_rccal_logic1.rccal_START) 1
	*/
	phy_utils_mod_radioreg(pi, RADIO_2065_RCCAL_LOGIC1, 0xFF01, 0x6301);
	/* check to see if RC CAL is done */
	OSL_DELAY(50);
	SPINWAIT(!wlc_radio_2065_rc_cal_done(pi), 10 * 1000 * 1000);
	if (!wlc_radio_2065_rc_cal_done(pi)) {
		PHY_ERROR(("wl%d: %s: Small Cap RC Cal failed\n", pi->sh->unit, __FUNCTION__));
		s_cap_rc_code_raw = old_rccal_small;
	} else {
		/* RCCAL successful */
		s_cap_rc_code_raw = phy_utils_read_radioreg(pi, RADIO_2065_RCCAL_LOGIC5) & 0x1F;
		if (s_cap_rc_code_raw < 0x1E)
			hpc_code_small = s_cap_rc_code_raw + hpc_offset;

		PHY_INFORM(("wl%d: %s:  Small Rx RC Cal completed for cap: "
			"N0: %x, N1: %x, s_cap_rc_code_raw: %x, hpc: %x\n",
			pi->sh->unit, __FUNCTION__,
			phy_utils_read_radioreg(pi, RADIO_2065_RCCAL_LOGIC3) & 0x1FFF,
			phy_utils_read_radioreg(pi, RADIO_2065_RCCAL_LOGIC4) & 0x1FFF,
			s_cap_rc_code_raw, hpc_code_small));
	}

	/*     #ADC Cap RC CAL
		   #Save old ADC Cap value incase RCCal fails
		   set Old_RCCAL_adc $reg(RF_rccal_logic2.rccal_adc_code)
	*/
	old_rccal_adc = phy_utils_read_radioreg(pi, RADIO_2065_RCCAL_LOGIC2) & 0xF;

	PHY_REG_LIST_START
		/* Stop RCCAL */
		RADIO_REG_MOD_ENTRY(RADIO_2065_RCCAL_LOGIC1, 0x1, 0)
		/* Power down RC CAL */
		RADIO_REG_MOD_ENTRY(RADIO_2065_RCCAL_CFG, 0x1, 0)
		/* Power up RC Cal */
		/* 	#RCCAL on Small Cap */
		RADIO_REG_MOD_ENTRY(RADIO_2065_RCCAL_CFG, 0x19, 1 | (2 << 3))
	PHY_REG_LIST_EXECUTE(pi);

	/*    #Set Trc
		  set reg(RF_rccal_trc.rccal_Trc) $Trc
	*/
	phy_utils_mod_radioreg(pi, RADIO_2065_RCCAL_TRC, 0x1FFF, trc);

	/* set reg(RF_rccal_logic1.rccal_X1) 0x6F
	   set reg(RF_rccal_logic1.rccal_START) 1
	*/
	phy_utils_mod_radioreg(pi, RADIO_2065_RCCAL_LOGIC1, 0xFF01, 0x3001);
	/* check to see if RC CAL is done */
	OSL_DELAY(50);
	SPINWAIT(!wlc_radio_2065_rc_cal_done(pi), 10 * 1000 * 1000);
	if (!wlc_radio_2065_rc_cal_done(pi)) {
		PHY_ERROR(("wl%d: %s: ADC Cap RC Cal failed\n", pi->sh->unit, __FUNCTION__));
		a_cap_rc_code_raw = old_rccal_adc;
	} else {
		/* RCCAL successful */
		a_cap_rc_code_raw = phy_utils_read_radioreg(pi, RADIO_2065_RCCAL_LOGIC2) & 0xF;
		if (a_cap_rc_code_raw < 0x1E)
			hpc_code_adc = a_cap_rc_code_raw + hpc_offset;
		PHY_INFORM(("wl%d: %s:  ADC Rx RC Cal completed for cap: "
			"N0: %x, N1: %x, a_cap_rc_code_raw: %x, hpc_code: %x\n",
			pi->sh->unit, __FUNCTION__,
			phy_utils_read_radioreg(pi, RADIO_2065_RCCAL_LOGIC3) & 0x1FFF,
			phy_utils_read_radioreg(pi, RADIO_2065_RCCAL_LOGIC3) & 0x1FFF,
			a_cap_rc_code_raw, hpc_code_adc));
		BCM_REFERENCE(hpc_code_adc);
	}
	c2 = s_cap_rc_code_raw + 19;
	c4 = c2 * 2 + 14;
	c3 = (c2 + c4) / 2 - 3;
	c1 = c3 /2 - 5;
	c1 -= 3;
	c2 -= 4;
	c3 -= 6;
	c4 -= 9;

	PHY_REG_LIST_START
		/* Stop RCCAL */
		RADIO_REG_MOD_ENTRY(RADIO_2065_RCCAL_LOGIC1, 0x1, 0)
		/* Power down RC CAL */
		RADIO_REG_MOD_ENTRY(RADIO_2065_RCCAL_CFG, 0x1, 0)
		/* 	#disable RCCal clock
			set reg(RF_xtal_cfg1.pll_clock_rccal_clk_pu) 1
		*/
		RADIO_REG_MOD_ENTRY(RADIO_2065_XTAL_CFG1, 0x4000, 0)
	PHY_REG_LIST_EXECUTE(pi);

	if (pi->u.pi_lcn40phy->new_lpf_rccal) {
		flt_val =
		(b_cap_rc_code_raw << 10) | (b_cap_rc_code_raw << 5) | (b_cap_rc_code_raw-4);
		phy_utils_write_phyreg(pi, LCN40PHY_lpf_rccal_tbl_1, flt_val);
		pi_lcn->lpf_rccal_tbl[0] = flt_val;
		flt_val =
			(b_cap_rc_code_raw << 10) | (b_cap_rc_code_raw << 5) | (b_cap_rc_code_raw);
		phy_utils_write_phyreg(pi, LCN40PHY_lpf_rccal_tbl_2, flt_val);
		pi_lcn->lpf_rccal_tbl[1] = flt_val;
	} else {
		flt_val =
			(b_cap_rc_code_raw << 10) | (b_cap_rc_code_raw << 5) | (b_cap_rc_code_raw);
		phy_utils_write_phyreg(pi, LCN40PHY_lpf_rccal_tbl_1, flt_val);
		phy_utils_write_phyreg(pi, LCN40PHY_lpf_rccal_tbl_2, flt_val);
		pi_lcn->lpf_rccal_tbl[0] = pi_lcn->lpf_rccal_tbl[1] = flt_val;
	}

	flt_val =
		(s_cap_rc_code_raw << 10) | (b_cap_rc_code_raw << 5) | (b_cap_rc_code_raw);
	phy_utils_write_phyreg(pi, LCN40PHY_lpf_rccal_tbl_3, flt_val);
	pi_lcn->lpf_rccal_tbl[2] = flt_val;

	flt_val =
		(s_cap_rc_code_raw << 10) | (s_cap_rc_code_raw << 5) | (s_cap_rc_code_raw);
	phy_utils_write_phyreg(pi, LCN40PHY_lpf_rccal_tbl_4, flt_val);
	pi_lcn->lpf_rccal_tbl[3] = flt_val;

	flt_val = (hpc_code_big << 5) | (hpc_code_big);
	phy_utils_write_phyreg(pi, LCN40PHY_lpf_rccal_tbl_5, flt_val);
	pi_lcn->lpf_rccal_tbl[4] = flt_val;

	flt_val = (c2 << 7) | c1;
	phy_utils_write_phyreg(pi, LCN40PHY_lpf_rccal_tbl_6, flt_val);
	pi_lcn->lpf_rccal_tbl[5] = flt_val;

	flt_val = (c4 << 7) | c3;
	phy_utils_write_phyreg(pi, LCN40PHY_lpf_rccal_tbl_7, flt_val);
	pi_lcn->lpf_rccal_tbl[6] = flt_val;

	if (LCN40REV_GE(pi->pubpi.phy_rev, 4)) {
		pi_lcn->lpf_rccal_tbl[7] = phy_utils_read_phyreg(pi, LCN40PHY_lpf_rccal_tbl_8);
		pi_lcn->lpf_rccal_tbl[8] = phy_utils_read_phyreg(pi, LCN40PHY_lpf_rccal_tbl_9);
		pi_lcn->lpf_rccal_tbl[9] = phy_utils_read_phyreg(pi, LCN40PHY_lpf_rccal_tbl_10);
	}
	pi_lcn->rccalcache_valid = 1;

	if (LCN40REV_GE(pi->pubpi.phy_rev, 4)) {
		uint16 rc_map[] = {14, 15, 0, 1, 5, 3, 7, 4, 9, 9, 8, 9, 10, 11, 12, 13};
		uint16 adc_rc;
		phy_utils_and_radioreg(pi, RADIO_2065_OVR1, ~0x280);
		adc_rc = phy_utils_read_radioreg(pi, RADIO_2065_ADC_RC);
		phy_utils_or_radioreg(pi, RADIO_2065_OVR1, 0x280);
		phy_utils_mod_radioreg(pi, RADIO_2065_ADC_RC, 0xff,
			(rc_map[(adc_rc >> 4) & 0xf] << 4) | rc_map[adc_rc & 0xf]);
	}
}

static void
wlc_lcn40phy_minipmu_cal(phy_info_t *pi)
{
	/* PER DANDAN, 4314 use cbuck skip minipmu cal */
	/* PER ABBY, disable minipmu cal for 43143 */
	if (LCN40REV_IS(pi->pubpi.phy_rev, 3) || LCN40REV_IS(pi->pubpi.phy_rev, 6)) {
		phy_utils_mod_radioreg(pi, RADIO_2065_PMU_OP, 0x8000, 0);
		return;
	}
	PHY_REG_LIST_START
		/* set reg(RF_pmu_cfg3.selavg) 2 */
		RADIO_REG_MOD_ENTRY(RADIO_2065_PMU_CFG3, 0xc0, 2 << 6)
		/* set reg(RF_pmu_op.vref_select) 0 */
		RADIO_REG_MOD_ENTRY(RADIO_2065_PMU_OP, 0x8000, 0)
		/* set reg(RF_pmu_cfg2.wlpmu_cntl) 0 */
		RADIO_REG_MOD_ENTRY(RADIO_2065_PMU_CFG2, 0x8, 0)
		/* set reg(RF_pmu_op.wlpmu_ldobg_clk_en) 0 */
		RADIO_REG_AND_ENTRY(RADIO_2065_PMU_OP, ~0x2000)
		/* set reg(RF_pmu_op.ldoref_start_cal) 1 */
		RADIO_REG_OR_ENTRY(RADIO_2065_PMU_OP, 0x4000)
		/* set reg(RF_pmu_op.ldoref_start_cal) 0 */
		RADIO_REG_AND_ENTRY(RADIO_2065_PMU_OP, (uint16)~0x4000)
		/* set reg(RF_pmu_op.wlpmu_ldobg_clk_en) 1 */
		RADIO_REG_OR_ENTRY(RADIO_2065_PMU_OP, 0x2000)
		/* set reg(RF_pmu_op.vref_select) 1 */
		RADIO_REG_OR_ENTRY(RADIO_2065_PMU_OP, 0x8000)
		/* set reg(RF_pmu_op.ldoref_start_cal) 1 */
		RADIO_REG_OR_ENTRY(RADIO_2065_PMU_OP, 0x4000)
	PHY_REG_LIST_EXECUTE(pi);

	SPINWAIT(!wlc_radio_2065_minipmu_cal_done(pi), 100 * 1000);

	if (!wlc_radio_2065_minipmu_cal_done(pi)) {
		PHY_ERROR(("wl%d: %s: minipmu cal failed\n", pi->sh->unit, __FUNCTION__));
	}
	/* set reg(RF_pmu_op.wlpmu_ldobg_clk_en) 0 */
	phy_utils_and_radioreg(pi, RADIO_2065_PMU_OP, ~0x2000);
	/* set reg(RF_pmu_op.ldoref_start_cal) 0 */
	phy_utils_and_radioreg(pi, RADIO_2065_PMU_OP, (uint16)~0x4000);
	/* 	set reg(RF_pmu_cfg3.mancodes) $reg(RF_pmu_stat.pmu_calcode) */
	phy_utils_mod_radioreg(pi, RADIO_2065_PMU_CFG3, 0xf,
		(phy_utils_read_radioreg(pi, RADIO_2065_PMU_STAT) & 0xf00) >> 8);
	/* set reg(RF_pmu_cfg2.wlpmu_cntl) 1 */
	phy_utils_or_radioreg(pi, RADIO_2065_PMU_CFG2, 0x8);

}

static void wlc_lcn40phy_load_tx_gain_table(phy_info_t *pi,
	const phy_tx_gain_tbl_entry * gain_table)
{
	uint32 j, k, min_txpwrindex;
	phytbl_info_t tab;
	uint32 val;
	uint16 pa_gain;
	uint16 gm_gain;
	int16 dac;
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);

	uint32* val_array = NULL;

	if (LCN40REV_IS(pi->pubpi.phy_rev, 7)) {
		if (CHSPEC_IS5G(pi->radio_chanspec) && PHY_EPA_SUPPORT(pi_lcn->ePA))
			pa_gain = 0x11;
		else
			pa_gain = 0xff;
#ifdef BAND5G
	} else if (CHSPEC_IS5G(pi->radio_chanspec)) {
		pa_gain = 0x70;
#endif /* BAND5G */
	} else {		/* 2g */
		if (!PHY_EPA_SUPPORT(pi_lcn->ePA))
			pa_gain = 0xff;
		else
			pa_gain = 0x30;
	}

	tab.tbl_id = LCN40PHY_TBL_ID_TXPWRCTL;
	tab.tbl_width = 32;     /* 32 bit wide  */
	tab.tbl_len = 1;        /* # values   */
	tab.tbl_ptr = &val; /* ptr to buf */

	if ((val_array = (uint32 *)LCN40PHY_MALLOC(pi, 128 * sizeof(uint32))) == NULL) {
		PHY_ERROR(("wl%d: %s: MALLOC failure\n", pi->sh->unit, __FUNCTION__));
		return;
	}

	if (CHSPEC_IS2G(pi->radio_chanspec))
		min_txpwrindex = pi_lcn->min_txpwrindex_2g;
	else
		min_txpwrindex = pi_lcn->min_txpwrindex_5g;

	pi_lcn->min_txpwrindex_papd = min_txpwrindex;

	for (j = 0; j < 128; j++) {

		if (j <= min_txpwrindex)
			k = min_txpwrindex;
		else
			k = j;

		gm_gain = gain_table[k].gm;

		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			if (pi_lcn->gmgc2g != -1)
				gm_gain = pi_lcn->gmgc2g;
			if (pi_lcn->pa_gain_ovr_val_2g != -1)
				pa_gain = pi_lcn->pa_gain_ovr_val_2g;
		}
#ifdef BAND5G
		else {
			if (pi_lcn->gmgc5g != -1)
				gm_gain = pi_lcn->gmgc5g;
			if (pi_lcn->pa_gain_ovr_val_5g != -1)
				pa_gain = pi_lcn->pa_gain_ovr_val_5g;
		}
#endif // endif
		val = (((uint32)pa_gain << 24) |
			(gain_table[k].pad << 16) |
			(gain_table[k].pga << 8) | gm_gain);

		*(val_array + j) = val;
	}
	tab.tbl_ptr = val_array;
	tab.tbl_len = 128;
	tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_GAIN_OFFSET;
	wlc_lcn40phy_write_table(pi, &tab);

	tab.tbl_ptr = val_array; /* ptr to buf */
	tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_IQ_OFFSET;
	wlc_lcn40phy_read_table(pi, &tab);

	for (j = 0; j < 128; j++) {

		if (j <= min_txpwrindex)
			k = min_txpwrindex;
		else
			k = j;

		dac = gain_table[k].dac;

		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			if (pi_lcn->dacgc2g != -1)
				dac = pi_lcn->dacgc2g;
		}
#ifdef BAND5G
		else {
			if (pi_lcn->dacgc5g != -1)
				dac = pi_lcn->dacgc5g;
		}
#endif // endif
		val = *(val_array + j);
		val = val & (0xFFFFF);
		val |= (dac << 28) |
			(gain_table[k].bb_mult << 20);

		*(val_array + j) = val;
	}

	wlc_lcn40phy_write_table(pi, &tab);
	pi->u.pi_lcn40phy->txgaintable = (phy_tx_gain_tbl_entry *)gain_table;

	if (val_array)
		LCN40PHY_MFREE(pi, val_array, 128 * sizeof(uint32));

}

static void
wlc_lcn40phy_txgainindex_cap_update(phy_info_t *pi, uint32 txidx_cap)
{
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	phy_info_lcn40phy_t *pi_lcn40 = pi->u.pi_lcn40phy;
	uint16 pwr_ctrl;
	bool suspend = !(R_REG(pi->sh->osh, &((phy_info_t *)pi)->regs->maccontrol) & MCTL_EN_MAC);

	if (!suspend)
		wlapi_suspend_mac_and_wait(pi->sh->physhim);

	pwr_ctrl = wlc_lcn40phy_get_tx_pwr_ctrl(pi);

	if (CHSPEC_IS2G(pi->radio_chanspec))
		pi_lcn->min_txpwrindex_2g = txidx_cap;
	else
		pi_lcn->min_txpwrindex_5g = txidx_cap;

	wlc_lcn40phy_set_tx_pwr_ctrl(pi, LCN40PHY_TX_PWR_CTRL_OFF);
	wlc_lcn40phy_load_tx_gain_table(pi, pi_lcn40->gaintable);
	wlc_lcn40phy_set_tx_pwr_ctrl(pi, pwr_ctrl);

	if (!suspend)
		wlapi_enable_mac(pi->sh->physhim);
}

void
wlc_lcn40phy_txgainindex_cap_adjust(phy_info_t *pi)
{
	phy_info_lcn40phy_t *pi_lcn40 = pi->u.pi_lcn40phy;
	int16 temper;
	int16 nom_txidxcap;

	if ((pi_lcn40->txidxcap_high == LCN40_TXIDXCAP_INVALID) &&
		(pi_lcn40->txidxcap_low == LCN40_TXIDXCAP_INVALID))
		return;

	if (CHSPEC_IS2G(pi->radio_chanspec))
		nom_txidxcap = pi_lcn40->nom_txidxcap_2g;
	else
		nom_txidxcap = pi_lcn40->nom_txidxcap_5g;

	temper = wlc_lcn40phy_tempsense(pi, !TEMPER_VBAT_TRIGGER_NEW_MEAS);

	if (pi_lcn40->txidxcap_high != LCN40_TXIDXCAP_INVALID) {
		if (pi_lcn40->txidxcap_hi_inuse) {
			/* If offset is already applied, wait for temper  */
			/* to go lower than threshold-diff to undo it */
			if (temper < (LCN40PHY_TEMPER_THRESHOLD_HIGH - LCN40PHY_TEMPER_DIFF)) {
				PHY_TMP(("Undo txidxcap_adjust_high, temper = %d\n", temper));
				pi_lcn40->txidxcap_hi_inuse = FALSE;
				wlc_lcn40phy_txgainindex_cap_update(pi, (uint32)nom_txidxcap);
			}
		}
		else if (temper > LCN40PHY_TEMPER_THRESHOLD_HIGH) {
			pi_lcn40->txidxcap_hi_inuse = TRUE;
			PHY_TMP(("Apply txidxcap_adjust_high, temper = %d\n", temper));
			wlc_lcn40phy_txgainindex_cap_update(pi,
				(uint32)pi_lcn40->txidxcap_high);
		}
	}

	if (pi_lcn40->txidxcap_low != LCN40_TXIDXCAP_INVALID) {
		if (pi_lcn40->txidxcap_lo_inuse) {
			/* If offset is already applied, wait for temper  */
			/* to go higher than threshold+diff to undo it */
			if (temper > (LCN40PHY_TEMPER_THRESHOLD_LOW + LCN40PHY_TEMPER_DIFF)) {
				PHY_TMP(("Undo txidxcap_adjust_low, temper = %d\n", temper));
				pi_lcn40->txidxcap_lo_inuse = FALSE;
				wlc_lcn40phy_txgainindex_cap_update(pi, (uint32)nom_txidxcap);
			}
		}
		else if (temper < LCN40PHY_TEMPER_THRESHOLD_LOW) {
			pi_lcn40->txidxcap_lo_inuse = TRUE;
			PHY_TMP(("Apply txidxcap_adjust_low, temper = %d\n", temper));
			wlc_lcn40phy_txgainindex_cap_update(pi,
				(uint32)pi_lcn40->txidxcap_low);
		}
	}
}

static void wlc_lcn40phy_scbbmult_tx_gain_table(phy_info_t *pi, int mult_factor)
{

	uint32 j;
	phytbl_info_t tab;
	uint32 val, tempval;
	uint32* val_array = NULL;

	tab.tbl_id = LCN40PHY_TBL_ID_TXPWRCTL;
	tab.tbl_width = 32;     /* 32 bit wide  */
	tab.tbl_len = 128;        /* # values   */
	tab.tbl_ptr = &val; /* ptr to buf */

	if ((val_array = (uint32 *)LCN40PHY_MALLOC(pi, 128 * sizeof(uint32))) == NULL) {
		PHY_ERROR(("wl%d: %s: MALLOC failure\n", pi->sh->unit, __FUNCTION__));
		return;
	}

	tab.tbl_ptr = val_array; /* ptr to buf */
	tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_IQ_OFFSET;
	wlc_lcn40phy_read_table(pi, &tab);

	for (j = 0; j < 128; j++) {

		val = *(val_array + j);
		val = val & (0xF00FFFFF);

		/* scale bbmult by 2 */
		tempval = ((mult_factor * pi->u.pi_lcn40phy->txgaintable[j].bb_mult) >> 6) & 0xff;
		val |= (tempval << 20);

		*(val_array + j) = val;
	}

	wlc_lcn40phy_write_table(pi, &tab);

	if (val_array)
		LCN40PHY_MFREE(pi, val_array, 128 * sizeof(uint32));

}

static void wlc_lcn40phy_load_rfpower(phy_info_t *pi)
{
	uint8 k;
	phytbl_info_t tab;
	uint32 *vals = NULL, val, bbmult, bbmult_old = -1;
	/* bbmult_old is -1 so that it always gets computed	first time */
	uint8 indx;
	uint8 scale_factor = 1;
	uint8 papd_rf_pwr_scale; /* Q4, scale papd rf power scale adjustment */
	int16 temp = 0, temp1, temp2, qQ = 0, qQ1, qQ2, shift = 0, temp3, qQ1_old = 0;
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	phy_info_lcn40phy_t *pi_lcn40 = pi->u.pi_lcn40phy;

	if ((vals = (uint32*) LCN40PHY_MALLOC(pi, 128 * sizeof(uint32))) == NULL) {
		PHY_ERROR(("wl%d: %s: MALLOC failure\n", pi->sh->unit, __FUNCTION__));
		return;
	}

	if (CHSPEC_IS2G(pi->radio_chanspec) && (pi_lcn->papd_rf_pwr_scale_2g != 0))
		papd_rf_pwr_scale = pi_lcn->papd_rf_pwr_scale_2g;
#ifdef BAND5G
	else if (CHSPEC_IS5G(pi->radio_chanspec) && (pi_lcn->papd_rf_pwr_scale_5g != 0))
		papd_rf_pwr_scale = pi_lcn->papd_rf_pwr_scale_5g;
#endif // endif
	else {
		papd_rf_pwr_scale = 8;
	}

	tab.tbl_id = LCN40PHY_TBL_ID_TXPWRCTL;
	tab.tbl_width = 32;     /* 32 bit wide  */
	tab.tbl_len = 128;      /* # values   */
	tab.tbl_ptr = vals; /* ptr to buf */
	tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_IQ_OFFSET;
	wlc_lcn40phy_read_table(pi, &tab);

	qm_log10((int32)(1<<6), 0, &temp2, &qQ2);

	for (indx = 0; indx < 128; indx++) {
		bbmult = vals[indx];
		bbmult = bbmult >> 20;

		if (bbmult_old == bbmult) {
			qQ1 = qQ1_old;
		} else {
			temp3 = temp2;
			qm_log10((int32)(bbmult), 0, &temp1, &qQ1);

			/* Update for next comparison */
			bbmult_old = bbmult;
			qQ1_old = qQ1;

			if (qQ1 < qQ2) {
				temp3 = qm_shr16(temp2, qQ2-qQ1);
				qQ = qQ1;
			}
			else {
				temp1 = qm_shr16(temp1, qQ1-qQ2);
				qQ = qQ2;
			}
			temp = qm_sub16(temp1, temp3);

			if (qQ >= 4)
				shift = qQ-4;
			else
				shift = 4-qQ;
		}
		/* now in q4 */
		val = (((indx << shift) + (5*temp) +
			(1<<(scale_factor+shift-3)))>>(scale_factor+shift-2));

		/* papd_rf_pwr_scale, q4 */
		/* factor of 2 implicit in previous */
		/* calculation */
		/* scale factor =0.6 for 43341 */
		if (LCN40REV_IS(pi->pubpi.phy_rev, 7))
			val = (val * papd_rf_pwr_scale) / 4;
		else if (LCN40REV_IS(pi->pubpi.phy_rev, 6) ||
			LCN40REV_IS(pi->pubpi.phy_rev, 3))
			val = (val * papd_rf_pwr_scale) / 8;
		else
			val = (val*papd_rf_pwr_scale + 8)/16;
		/* add vlin at bit 10 */
		if (PHY_EPA_SUPPORT(pi_lcn->ePA)) {
			/* By default, vlin is set to 1 to enable high-linearity mode of FEM.
			*   RFMD FEMs require vlin=0 to enable high-linearity mode.
			*/
			int8 vlin = 1;
			if (CHSPEC_IS2G(pi->radio_chanspec) &&
				(pi_lcn40->vlin2g != -1))
				vlin = pi_lcn40->vlin2g;
#ifdef BAND5G
			else if (CHSPEC_IS5G(pi->radio_chanspec) &&
				(pi_lcn40->vlin5g != -1))
				vlin = pi_lcn40->vlin5g;
#endif /* BAND5G */
			val |= (vlin << 10);
		} else
			val |= (0 << 10);
		PHY_INFORM(("idx = %d, bb: %d, tmp = %d, qQ = %d, sh = %d, val = %d\n",
			indx, bbmult, temp, qQ, shift, val));
		vals[indx] = val;
	}
	tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_PWR_OFFSET;
	wlc_lcn40phy_write_table(pi, &tab);
	if (vals)
		LCN40PHY_MFREE(pi, vals, 128 * sizeof(uint32));

	k =  pi_lcn->lcnphy_capped_index;
	/* min index to cap the gain table indexing for PAPD cals */
	if (LCN40REV_IS(pi->pubpi.phy_rev, 5) &&
		(LCN40_LINPATH(pi_lcn40->papd_lin_path) ==
		LCN40PHY_PAPDLIN_EPA))
		k = (uint8) pi_lcn->min_txpwrindex_papd;

	tab.tbl_len = 1;
	for (indx = 0; indx <= k; indx++) {
		tab.tbl_ptr = &val; /* ptr to buf */
		tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_PWR_OFFSET + k;
		wlc_lcn40phy_read_table(pi, &tab);

		tab.tbl_ptr = &val;
		tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_PWR_OFFSET + indx;
		wlc_lcn40phy_write_table(pi, &tab);
	}
}

static void
WLBANDINITFN(wlc_lcn40phy_tx_pwr_ctrl_init)(phy_info_t *pi)
{
	bool suspend;
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	phy_info_lcn40phy_t *pi_lcn40 = pi->u.pi_lcn40phy;
	int16 power_correction;

	suspend = !(R_REG(pi->sh->osh, &pi->regs->maccontrol) & MCTL_EN_MAC);
	if (!suspend)
		wlapi_suspend_mac_and_wait(pi->sh->physhim);

	if (NORADIO_ENAB(pi->pubpi)) {
		if (!suspend)
			wlapi_enable_mac(pi->sh->physhim);
		return;
	}

	if (!pi->hwpwrctrl_capable) {
		wlc_lcn40phy_set_tx_pwr_ctrl(pi, LCN40PHY_TX_PWR_CTRL_OFF);
	} else {
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			pi_lcn->txpwr_tssifloor_clamp_dis = pi_lcn->txpwr_tssifloor_clamp_dis_2g;
			pi_lcn->txpwr_tssioffset_clamp_dis = pi_lcn->txpwr_tssioffset_clamp_dis_2g;
			pi_lcn40->txidxcap_high = pi_lcn40->txidxcap_2g_high;
			pi_lcn40->txidxcap_low = pi_lcn40->txidxcap_2g_low;
		}
#ifdef BAND5G
		else {
			pi_lcn->txpwr_tssifloor_clamp_dis = pi_lcn->txpwr_tssifloor_clamp_dis_5g;
			pi_lcn->txpwr_tssioffset_clamp_dis = pi_lcn->txpwr_tssioffset_clamp_dis_5g;
			pi_lcn40->txidxcap_high = pi_lcn40->txidxcap_5g_high;
			pi_lcn40->txidxcap_low = pi_lcn40->txidxcap_5g_low;
		}
#endif /* BAND5G */
		pi_lcn40->txidxcap_hi_inuse = FALSE;
		pi_lcn40->txidxcap_lo_inuse = FALSE;

		/* Clear out all power offsets */
		wlc_lcn40phy_clear_tx_power_offsets(pi);

		if (LCN40REV_GE(pi->pubpi.phy_rev, 4) &&
			(pi_lcn->lcnphy_tssical_time))
				wlc_lcn40phy_perpkt_idle_tssi_est(pi);
		else
			wlc_lcn40phy_idle_tssi_est(pi);

		/* Convert tssi to power LUT */
		wlc_lcn40phy_set_estPwrLUT(pi, 0);

		PHY_REG_LIST_START
			PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlRangeCmd, pwrMinMaxEnable, 0)
			PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlPwrMinMaxVal, pwrMinVal, 0)
			PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlPwrMinMaxVal, pwrMaxVal, 0)

			PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlRangeCmd, txGainTable_mode, 0)
			PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlRangeCmd, interpol_en, 0)
		PHY_REG_LIST_EXECUTE(pi);

#if TWO_POWER_RANGE_TXPWR_CTRL
		if (pi_lcn->lcnphy_twopwr_txpwrctrl_en) {
			wlc_lcn40phy_set_estPwrLUT(pi, 1);

			PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlPwrRange2, pwrMin_range2, pi_lcn->pmin);
			PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlPwrRange2, pwrMax_range2, pi_lcn->pmax);

			PHY_REG_LIST_START
				PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlRangeCmd, pwrMinMaxEnable2, 0)
				PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlPwrMinMaxVal2, pwrMinVal2, 0)
				PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlPwrMinMaxVal2, pwrMaxVal2, 0)

				PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlRangeCmd, interpol_en1, 0)
			PHY_REG_LIST_EXECUTE(pi);
		}
#endif /* TWO_POWER_RANGE_TXPWR_CTRL */

		if (LCN40REV_LT(pi->pubpi.phy_rev, 4))
			PHY_REG_MOD(pi, LCN40PHY, crsgainCtrl, crseddisable, 0);
		phy_utils_write_phyreg(pi, LCN40PHY_TxPwrCtrlDeltaPwrLimit, 10);
		PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlRangeCmd, cckPwrOffset,
			pi_lcn->cckPwrOffset + (pi_lcn->cckPwrIdxCorr<<1));

		if (LCN40REV_GE(pi->pubpi.phy_rev, 4)) {
			PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlCmdCCK, baseIndex_cck_en, 1);
			PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlCmdCCK, pwrIndex_init_cck, 180);
		}

		PHY_REG_MOD(pi, LCN40PHY, TempSenseCorrection, tempsenseCorr, 0);
		if ((LCN40REV_GE(pi->pubpi.phy_rev, 4)) &&
			(CHSPEC_IS40(pi->radio_chanspec))) {
			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				PHY_REG_MOD(pi, LCN40PHY, TempSenseCorrection,
				tempsenseCorr, pi_lcn40->pwr_offset40mhz_2g);
				pi_lcn40->tempsenseCorr = pi_lcn40->pwr_offset40mhz_2g;
			}
#ifdef BAND5G
			else {
				PHY_REG_MOD(pi, LCN40PHY, TempSenseCorrection,
				tempsenseCorr, pi_lcn40->pwr_offset40mhz_5g);
				pi_lcn40->tempsenseCorr = pi_lcn40->pwr_offset40mhz_5g;
			}
#endif /* BAND5G */
		}

		if (LCN40REV_GE(pi->pubpi.phy_rev, 4)) {
			if (pi_lcn->lcnphy_tssical_time) {
				power_correction = pi_lcn40->tempsenseCorr +
					pi_lcn40->lcnphy_idletssi_corr;
				PHY_REG_MOD(pi, LCN40PHY, TempSenseCorrection,
				tempsenseCorr, power_correction);
			}
		}

		wlc_lcn40phy_set_target_tx_pwr(pi, LCN40_TARGET_PWR);

#ifdef WLNOKIA_NVMEM
		/* update the cck power detector offset, these are in 1/8 dBs */
		{
			int8 cckoffset, ofdmoffset;
			int16 diff;
			/* update the cck power detector offset */
			wlc_phy_get_pwrdet_offsets(pi, &cckoffset, &ofdmoffset);
			PHY_INFORM(("cckoffset is %d and ofdmoffset is %d\n",
				cckoffset, ofdmoffset));

			diff = cckoffset - ofdmoffset;
			if (diff >= 0)
				diff &= 0x0FF;
			else
				diff &= 0xFF | 0x100;

			/* program the cckpwoffset bits(6-14) in reg 0x4d0 */
			PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlRangeCmd, cckPwrOffset, diff);

			/* program the tempsense curr in reg 0x50c */
			if (ofdmoffset >= 0)
				diff = ofdmoffset;
			else
				diff = 0x100 | ofdmoffset;
			PHY_REG_MOD(pi, LCN40PHY, TempSenseCorrection, tempsenseCorr, diff);
		}
#endif /* WLNOKIA_NVMEM */

		if (CHSPEC_IS2G(pi->radio_chanspec))
			pi_lcn->lcnphy_tssi_idx = pi_lcn->init_txpwrindex_2g;
#ifdef BAND5G
		else
			pi_lcn->lcnphy_tssi_idx = pi_lcn->init_txpwrindex_5g;
#endif /* BAND5G */

		pi_lcn40->cck_tssi_idx = pi_lcn40->init_ccktxpwrindex;

		ASSERT(pi_lcn->lcnphy_tssi_idx > 0);
		ASSERT(pi_lcn40->cck_tssi_idx > 0);

		/* Enable hardware power control */
		wlc_lcn40phy_set_tx_pwr_ctrl(pi, LCN40PHY_TX_PWR_CTRL_HW);
	}
	if (!suspend)
		wlapi_enable_mac(pi->sh->physhim);
}

static uint16
lcn40phy_div_based_init_h(uint16 ant, uint16 swidx, uint32 *swmap)
{
	uint16 ret;

	if (ant)
		ret = (uint16) ((swmap[swidx] & 0xff000000) >> 24);
	else
		ret = (uint16) ((swmap[swidx] & 0xff0000) >> 16);

	return ret;
}

static uint16
lcn40phy_div_based_init_l(uint16 ant, uint16 swidx, uint32 *swmap)
{
	uint16 ret;

	if (ant)
		ret = (uint16) ((swmap[swidx] & 0xff00) >> 8);
	else
		ret = (uint16) (swmap[swidx] & 0xff);

	return ret;
}

void
wlc_lcn40phy_sw_ctrl_tbl_init(phy_info_t *pi)
{

	phytbl_info_t tab;
	uint16 *tbl_ptr;
	uint16 tblsz;
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	uint8 idx;
	uint32 *swctrlmap, *swctrlmapext;
	uint16 tdm, ovr_en;
	uint16 mask;

	tblsz = LCN40PHY_SW_CTRL_TBL_LENGTH * (LCN40PHY_SW_CTRL_TBL_WIDTH >> 3);

	if ((tbl_ptr = LCN40PHY_MALLOC(pi, tblsz)) == NULL) {
		PHY_ERROR(("wl%d: %s: MALLOC failure\n",
			pi->sh->unit, __FUNCTION__));
		return;
	}
	swctrlmap = pi_lcn->swctrlmap_2g;
	swctrlmapext = pi_lcn->swctrlmapext_2g;
#ifdef BAND5G
	if (CHSPEC_IS5G(pi->radio_chanspec)) {
		swctrlmap = pi_lcn->swctrlmap_5g;
		swctrlmapext = pi_lcn->swctrlmapext_5g;
	}
#endif // endif

	tdm = (swctrlmap[LCN40PHY_I_WL_MASK] & LCN40PHY_MASK_TDM);
	ovr_en = (swctrlmap[LCN40PHY_I_WL_MASK] & LCN40PHY_MASK_OVR_EN);

	for (idx = 0; idx < LCN40PHY_SW_CTRL_TBL_LENGTH; idx++) {
		uint8	bt_pri = idx & LCN40PHY_SW_CTRL_MAP_BT_PRIO;
		uint16	ant = idx & LCN40PHY_SW_CTRL_MAP_ANT;

		tbl_ptr[idx] = 0;
		/* BT Prio */
		if (bt_pri) {
			/* Diasble diversity in WL to ensure
			both BT and WL recieve from the same ant
			*/
			if (ovr_en)
				ant = (swctrlmap[LCN40PHY_I_WL_MASK] & LCN40PHY_MASK_ANT);

			if (idx & LCN40PHY_SW_CTRL_MAP_BT_TX) {
				/* BT Tx */

				tbl_ptr[idx] |=
				(swctrlmap[LCN40PHY_I_BT] & LCN40PHY_MASK_BT_TX)
					>> LCN40PHY_SHIFT_BT_TX;
				tbl_ptr[idx] |=
				((swctrlmapext[LCN40PHY_I_BT] & LCN40PHY_MASK_BT_TX)
					>> (LCN40PHY_SHIFT_BT_TX - 8));
			} else {
				/* BT Rx */
				if (idx & LCN40PHY_SW_CTRL_MAP_ELNA) {
					tbl_ptr[idx] |=
					(swctrlmap[LCN40PHY_I_BT] & LCN40PHY_MASK_BT_ELNARX)
					>> LCN40PHY_SHIFT_BT_ELNARX;
					tbl_ptr[idx] |=
					((swctrlmapext[LCN40PHY_I_BT] & LCN40PHY_MASK_BT_ELNARX)
					>> (LCN40PHY_SHIFT_BT_ELNARX - 8));
				} else {
					tbl_ptr[idx] |=
					(swctrlmap[LCN40PHY_I_BT] & LCN40PHY_MASK_BT_RX)
					>> LCN40PHY_SHIFT_BT_RX;
					tbl_ptr[idx] |=
					((swctrlmapext[LCN40PHY_I_BT] & LCN40PHY_MASK_BT_RX)
					<< 8);
				}
			}
		}
		/* WL Tx/Rx */
		if (!tdm || !bt_pri) {
			if (idx & LCN40PHY_SW_CTRL_MAP_WL_TX) {
				/* PA on */
				if (idx & LCN40PHY_SW_CTRL_MAP_WL_RX) {
					/* Rx with PA on */
					tbl_ptr[idx] |=
					lcn40phy_div_based_init_h(ant, LCN40PHY_I_WL_TX, swctrlmap);
					tbl_ptr[idx] |=
					((lcn40phy_div_based_init_h(ant, LCN40PHY_I_WL_TX,
					swctrlmapext)) << 8);
				} else {
					/* WL Tx with PA on */
					tbl_ptr[idx] |=
					lcn40phy_div_based_init_l(ant, LCN40PHY_I_WL_TX, swctrlmap);
					tbl_ptr[idx] |=
					((lcn40phy_div_based_init_l(ant, LCN40PHY_I_WL_TX,
					swctrlmapext)) << 8);
				}
			} else {
				if (idx & LCN40PHY_SW_CTRL_MAP_ELNA) {
					if (idx & LCN40PHY_SW_CTRL_MAP_WL_RX) {
						/* WL Rx eLNA */
						tbl_ptr[idx] |=
						lcn40phy_div_based_init_h(ant, LCN40PHY_I_WL_RX,
						swctrlmap);
						tbl_ptr[idx] |=
						((lcn40phy_div_based_init_h(ant, LCN40PHY_I_WL_RX,
						swctrlmapext)) << 8);
					} else {
						/* WL Rx Attn eLNA */
						tbl_ptr[idx] |=
						lcn40phy_div_based_init_h
						(ant, LCN40PHY_I_WL_RX_ATTN, swctrlmap);
						tbl_ptr[idx] |=
						((lcn40phy_div_based_init_h
						(ant, LCN40PHY_I_WL_RX_ATTN, swctrlmapext)) << 8);
					}
				} else { /* Without eLNA */
					if (idx & LCN40PHY_SW_CTRL_MAP_WL_RX) {
						/* WL Rx */
						tbl_ptr[idx] |=
						lcn40phy_div_based_init_l
						(ant, LCN40PHY_I_WL_RX, swctrlmap);
						tbl_ptr[idx] |=
						((lcn40phy_div_based_init_l
						(ant, LCN40PHY_I_WL_RX, swctrlmapext)) << 8);
					} else {
						/* WL Rx Attn */
						tbl_ptr[idx] |=
						lcn40phy_div_based_init_l
						(ant, LCN40PHY_I_WL_RX_ATTN, swctrlmap);
						tbl_ptr[idx] |=
						((lcn40phy_div_based_init_l
						(ant, LCN40PHY_I_WL_RX_ATTN, swctrlmapext)) << 8);
					}
				}
			}
		}
	}

	/* Writing the fields into the LCN40PHYk_swctrlconfig register */
	mask = (uint16)(swctrlmap[LCN40PHY_I_WL_MASK] & LCN40PHY_MASK_WL_MASK)
			<< LCN40PHY_sw_ctrl_config_sw_ctrl_mask_SHIFT;
	mask |= (uint16)((swctrlmapext[LCN40PHY_I_WL_MASK] & LCN40PHY_MASK_WL_MASK)
			<< 8);
	phy_utils_mod_phyreg(pi, LCN40PHY_sw_ctrl_config,
	                     LCN40PHY_sw_ctrl_config_sw_ctrl_mask_MASK, mask);

	/* Write the populated sw ctrl table to the default switch ctrl table location */
	tab.tbl_len = LCN40PHY_SW_CTRL_TBL_LENGTH;
	tab.tbl_id = LCN40PHY_TBL_ID_SW_CTRL;
	tab.tbl_offset = 0;
	tab.tbl_width = LCN40PHY_SW_CTRL_TBL_WIDTH;
	tab.tbl_ptr = tbl_ptr;
	wlc_lcn40phy_write_table(pi, &tab);

	/* [BITS] (FLOLYMPIC-8) RADAR 11947746: BT sends clb2lcn40_bt_ext_lna_gain inverted */
	if (LCN40REV_GE(pi->pubpi.phy_rev, 4))
		PHY_REG_MOD(pi, LCN40PHY, swctrlOvr_val, bt_extlna_lut, 1);

	LCN40PHY_MFREE(pi, tbl_ptr, tblsz);
}

static void
WLBANDINITFN(wlc_lcn40phy_tbl_init)(phy_info_t *pi)
{
	uint idx, tbl_info_sz;
	phytbl_info_t *tbl_info = NULL;
	phy_tx_gain_tbl_entry *gaintable = NULL;
	phy_info_lcn40phy_t *pi_lcn40 = pi->u.pi_lcn40phy;
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	phytbl_info_t tab;
#ifdef BAND5G
	uint32 dot11lcn40_gain_idx_tbl_5G_ext_lna_rev4_fstr_cdd[] = {
		0x00484780,
		0x00484800,
		0x00484880,
		0x00484900,
		0x00484980,
		0x00484a00,
		0x00484a80,
		0x00484b00,
		0x00400000,
		0x00400000,
		0x00400000,
		0x00400000,
		0x00400000,
		0x00400000,
		0x00400000,
		0x00400080,
		0x00404000,
		0x00404080,
		0x00404100,
		0x00408080,
		0x00408b80,
		0x00408c00,
		0x00408c80,
		0x0040cd00,
		0x0040cc80,
		0x00410d00,
		0x00410c80,
		0x00414d00,
		0x00414d80,
		0x00414e00,
		0x00414e80,
		0x00414f00,
		0x00414f80,
		0x00415000,
		0x00415080,
		0x00415100,
		0x00415180,
		0x00415200,
		0x00415280,
		0x00415300,
		0x00415380,
		0x00415400,
		0x00415480,
		0x00415500,
		0x00415580,
		0x00415600
	};
	uint32 dot11lcn40_gain_tbl_5G_ext_lna_rev4_fstr_cdd[] =
		{0x0002929b, 0x000a929b, 0x0012929b, 0x001a929b,
		0x0022929b, 0x002a929b, 0x0032929b, 0x003a929b};
#endif /* #ifdef BAND5G */

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	if (LCN40REV_IS(pi->pubpi.phy_rev, 0) || LCN40REV_IS(pi->pubpi.phy_rev, 2) ||
		LCN40REV_IS(pi->pubpi.phy_rev, 4) || LCN40REV_IS(pi->pubpi.phy_rev, 5)) {
		tbl_info_sz = dot11lcn40phytbl_info_sz_rev0;
		tbl_info = (phytbl_info_t *)dot11lcn40phytbl_info_rev0;
		if (CHSPEC_IS2G(pi->radio_chanspec) || NORADIO_ENAB(pi->pubpi))
			gaintable = (phy_tx_gain_tbl_entry *)dot11lcn40phy_2GHz_gaintable_rev0;
#ifdef BAND5G
		else
			gaintable = (phy_tx_gain_tbl_entry *)dot11lcn40phy_5GHz_gaintable_rev0;
#endif // endif
	} else if (LCN40REV_IS(pi->pubpi.phy_rev, 1)) {
		tbl_info_sz = dot11lcn40phytbl_info_sz_rev1;
		tbl_info = (phytbl_info_t *)dot11lcn40phytbl_info_rev1;
		if (PHY_EPA_SUPPORT(wlc_phy_getlcnphy_common(pi)->ePA))
			gaintable = (phy_tx_gain_tbl_entry *)
						dot11lcn40phy_2GHz_extPA_gaintable_rev1;
		else
			gaintable = (phy_tx_gain_tbl_entry *)dot11lcn40phy_2GHz_gaintable_rev1;
	} else if (LCN40REV_IS(pi->pubpi.phy_rev, 3)) {
		tbl_info_sz = dot11lcn40phytbl_info_sz_rev3;
		tbl_info = (phytbl_info_t *)dot11lcn40phytbl_info_rev3;
		if (PHY_EPA_SUPPORT(wlc_phy_getlcnphy_common(pi)->ePA))
			gaintable = (phy_tx_gain_tbl_entry *)
						dot11lcn40phy_2GHz_extPA_gaintable_rev1;
		else
			gaintable = (phy_tx_gain_tbl_entry *)dot11lcn40phy_2GHz_gaintable_rev3;
	} else if (LCN40REV_IS(pi->pubpi.phy_rev, 7)) {
		tbl_info_sz = dot11lcn40phytbl_info_sz_rev7;
		tbl_info = (phytbl_info_t *)dot11lcn40phytbl_info_rev7;
		if (CHSPEC_IS2G(pi->radio_chanspec) || NORADIO_ENAB(pi->pubpi))
			gaintable = (phy_tx_gain_tbl_entry *)dot11lcn40phy_2GHz_gaintable_rev7;
#ifdef BAND5G
		else
			if (PHY_EPA_SUPPORT(wlc_phy_getlcnphy_common(pi)->ePA))
				gaintable = (phy_tx_gain_tbl_entry *)
							dot11lcn40phy_5GHz_extPA_gaintable_rev7;
			else
				gaintable = (phy_tx_gain_tbl_entry *)
							dot11lcn40phy_5GHz_gaintable_rev7;
#endif // endif
	} else if (LCN40REV_IS(pi->pubpi.phy_rev, 6)) {
		tbl_info_sz = dot11lcn40phytbl_info_sz_rev6;
		tbl_info = (phytbl_info_t *)dot11lcn40phytbl_info_rev6;
		gaintable = (phy_tx_gain_tbl_entry *)dot11lcn40phy_2GHz_gaintable_rev6;
	} else {
		tbl_info_sz = dot11lcn40phytbl_info_sz_rev0;
		tbl_info = (phytbl_info_t *)dot11lcn40phytbl_info_rev0;
		if (CHSPEC_IS2G(pi->radio_chanspec) || NORADIO_ENAB(pi->pubpi))
			gaintable = (phy_tx_gain_tbl_entry *)dot11lcn40phy_2GHz_gaintable_rev0;
#ifdef BAND5G
		else
			gaintable = (phy_tx_gain_tbl_entry *)dot11lcn40phy_5GHz_gaintable_rev0;
#endif // endif
	}

	/* Add the tables that are manuplated after the init here otherwise
	     the other band might get affected
	*/
	if (pi->fast_bs) {
		for (idx = 0; idx < tbl_info_sz; idx++) {
			if ((tbl_info[idx].tbl_id == LCN40PHY_TBL_ID_FLTR_CTRL) ||
				(tbl_info[idx].tbl_id == LCN40PHY_TBL_ID_IQLOCAL) ||
				(tbl_info[idx].tbl_id == LCN40PHY_TBL_ID_RFSEQ) ||
				(tbl_info[idx].tbl_id == LCN40PHY_TBL_ID_PAPDCOMPDELTATBL) ||
				(tbl_info[idx].tbl_id == LCN40PHY_TBL_ID_MIN_SIG_SQ))
			wlc_lcn40phy_write_table(pi, &tbl_info[idx]);
		}
	} else {
		for (idx = 0; idx < tbl_info_sz; idx++)
			wlc_lcn40phy_write_table(pi, &tbl_info[idx]);
	}

	if (LCN40REV_IS(pi->pubpi.phy_rev, 0) || LCN40REV_IS(pi->pubpi.phy_rev, 2)) {
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			if (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA) {
				for (idx = 0;
				     idx < dot11lcn40phytbl_2G_ext_lna_rx_gain_info_sz_rev0;
				     idx++) {
					wlc_lcn40phy_write_table(pi,
					    &dot11lcn40phytbl_2G_ext_lna_rx_gain_info_rev0[idx]);
				}
			} else {
				for (idx = 0;
				     idx < dot11lcn40phytbl_2G_rx_gain_info_sz_rev0;
				     idx++) {
					wlc_lcn40phy_write_table(pi,
					    &dot11lcn40phytbl_2G_rx_gain_info_rev0[idx]);
				}
			}
		}
#ifdef BAND5G
		else {
			if (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA_5GHz) {
				for (idx = 0;
				     idx < dot11lcn40phytbl_5G_ext_lna_rx_gain_info_sz_rev0;
				     idx++) {
					wlc_lcn40phy_write_table(pi,
					    &dot11lcn40phytbl_5G_ext_lna_rx_gain_info_rev0[idx]);
				}
			} else {
				for (idx = 0;
				     idx < dot11lcn40phytbl_5G_rx_gain_info_sz_rev0;
				     idx++) {
					wlc_lcn40phy_write_table(pi,
					    &dot11lcn40phytbl_5G_rx_gain_info_rev0[idx]);
				}
			}
		}
#endif /* BAND5G */
	} else if (LCN40REV_IS(pi->pubpi.phy_rev, 4) || LCN40REV_IS(pi->pubpi.phy_rev, 5)) {

		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			if (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA) {
				for (idx = 0;
				     idx < dot11lcn40phytbl_2G_ext_lna_rx_gain_info_sz_rev4;
				     idx++) {
					wlc_lcn40phy_write_table(pi,
					    &dot11lcn40phytbl_2G_ext_lna_rx_gain_info_rev4[idx]);
				}
			} else {
				for (idx = 0;
				     idx < dot11lcn40phytbl_2G_rx_gain_info_sz_rev4;
				     idx++) {
					wlc_lcn40phy_write_table(pi,
					    &dot11lcn40phytbl_2G_rx_gain_info_rev4[idx]);
				}
			}
		}
#ifdef BAND5G
		else {
			if (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA_5GHz) {
				for (idx = 0;
				     idx < dot11lcn40phytbl_5G_ext_lna_rx_gain_info_sz_rev4;
				     idx++) {
					wlc_lcn40phy_write_table(pi,
					    &dot11lcn40phytbl_5G_ext_lna_rx_gain_info_rev4[idx]);
				}
			} else {
				for (idx = 0;
				     idx < dot11lcn40phytbl_5G_rx_gain_info_sz_rev4;
				     idx++) {
					wlc_lcn40phy_write_table(pi,
					    &dot11lcn40phytbl_5G_rx_gain_info_rev4[idx]);
				}
			}
		}
#endif /* BAND5G */
	} else if (LCN40REV_IS(pi->pubpi.phy_rev, 1)) {
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			if (CHSPEC_IS40(pi->radio_chanspec)) {
				for (idx = 0;
				      idx < dot11lcn40phytbl_2G_40mhz_rx_gain_info_sz_rev1;
				      idx++) {
					wlc_lcn40phy_write_table(pi,
						&dot11lcn40phytbl_2G_40mhz_rx_gain_info_rev1[idx]);
				}
			} else {
				for (idx = 0;
				      idx < dot11lcn40phytbl_2G_rx_gain_info_sz_rev1;
				      idx++) {
					wlc_lcn40phy_write_table(pi,
						&dot11lcn40phytbl_2G_rx_gain_info_rev1[idx]);
				}
			}
		}
	} else if (LCN40REV_IS(pi->pubpi.phy_rev, 3)) {
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			if (CHSPEC_IS40(pi->radio_chanspec)) {
				for (idx = 0;
				      idx < dot11lcn40phytbl_2G_rx_gain_info_sz_rev3;
				      idx++) {
					wlc_lcn40phy_write_table(pi,
						&dot11lcn40phytbl_2G_rx_gain_info_rev3[idx]);
				}
			} else {
				for (idx = 0;
				      idx < dot11lcn40phytbl_2G_rx_gain_info_sz_rev3;
				      idx++) {
					wlc_lcn40phy_write_table(pi,
						&dot11lcn40phytbl_2G_rx_gain_info_rev3[idx]);
				}
			}
		}
	} else if (LCN40REV_IS(pi->pubpi.phy_rev, 7)) {
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			for (idx = 0;
			     idx < dot11lcn40phytbl_2G_rx_gain_info_sz_rev7;
			     idx++) {
				wlc_lcn40phy_write_table(pi,
					&dot11lcn40phytbl_2G_rx_gain_info_rev7[idx]);
			}
		}
#ifdef BAND5G
		else {
			if (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA_5GHz) {
				for (idx = 0;
					idx < dot11lcn40phytbl_5G_ext_lna_rx_gain_info_sz_rev7;
					idx++) {
					wlc_lcn40phy_write_table(pi,
					&dot11lcn40phytbl_5G_ext_lna_rx_gain_info_rev7[idx]);
					}
			} else if (CHSPEC_IS20(pi->radio_chanspec)) {
				for (idx = 0;
					idx < dot11lcn40phytbl_5G_rx_gain_info_sz_rev7;
					idx++) {
					wlc_lcn40phy_write_table(pi,
						&dot11lcn40phytbl_5G_rx_gain_info_rev7[idx]);
					}
			} else {
				for (idx = 0;
					idx < dot11lcn40phytbl_5G_40mhz_rx_gain_info_sz_rev7;
					idx++) {
					wlc_lcn40phy_write_table(pi,
						&dot11lcn40phytbl_5G_40mhz_rx_gain_info_rev7[idx]);
					}
			}
		}
#endif /* BAND5G */
	} else if (LCN40REV_IS(pi->pubpi.phy_rev, 6)) {
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			for (idx = 0;
				idx < dot11lcn40phytbl_2G_rx_gain_info_sz_rev6;
				idx++) {
				wlc_lcn40phy_write_table(pi,
					&dot11lcn40phytbl_2G_rx_gain_info_rev6[idx]);
			}
		}
	} else {
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			if (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA) {
				for (idx = 0;
				     idx < dot11lcn40phytbl_2G_ext_lna_rx_gain_info_sz_rev0;
				     idx++) {
					wlc_lcn40phy_write_table(pi,
					    &dot11lcn40phytbl_2G_ext_lna_rx_gain_info_rev0[idx]);
				}
			}
		}
#ifdef BAND5G
		else {
			if (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA_5GHz) {
				for (idx = 0;
				     idx < dot11lcn40phytbl_5G_ext_lna_rx_gain_info_sz_rev0;
				     idx++) {
					wlc_lcn40phy_write_table(pi,
					    &dot11lcn40phytbl_5G_ext_lna_rx_gain_info_rev0[idx]);
				}
			} else {
				for (idx = 0;
				     idx < dot11lcn40phytbl_5G_rx_gain_info_sz_rev0;
				     idx++) {
					wlc_lcn40phy_write_table(pi,
					    &dot11lcn40phytbl_5G_rx_gain_info_rev0[idx]);
				}
			}
		}
#endif /* BAND5G */
	}

	pi_lcn40->gaintable = gaintable;
	pi_lcn->min_txpwrindex_2g = pi_lcn40->nom_txidxcap_2g;
	pi_lcn->min_txpwrindex_5g = pi_lcn40->nom_txidxcap_5g;
	wlc_lcn40phy_load_tx_gain_table(pi, gaintable);

	/* Change switch table if neccessary based on chip and board type */
	wlc_lcn40phy_sw_ctrl_tbl_init(pi);

	wlc_lcn40phy_load_rfpower(pi);

	/* clear our PAPD Compensation table */
	wlc_lcn40phy_clear_papd_comptable(pi);
	if ((pi_lcn40->fstr_flag == 1) || (pi_lcn40->cdd_mod == 1))
	{
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
		    uint32 fstr_cdd_idx = 0x00474800;
		    uint32 fstr_cdd_gain = 0x0010922e;

		    tab.tbl_id = LCN40PHY_TBL_ID_GAIN_IDX;
		    tab.tbl_width = 32;
		    tab.tbl_ptr = &fstr_cdd_idx;
		    tab.tbl_len = 1;
		    tab.tbl_offset = 30;
		    wlc_lcnphy_write_table(pi, &tab);

		    tab.tbl_id = LCN40PHY_TBL_ID_GAIN_TBL;
		    tab.tbl_ptr = &fstr_cdd_gain;
		    tab.tbl_offset = 16;
		    wlc_lcnphy_write_table(pi, &tab);
		}
#ifdef BAND5G
		else {
			const uint32 *temp_ptr =
			(const uint32 *)dot11lcn40phytbl_5G_ext_lna_rx_gain_info_rev4[1].tbl_ptr;
			tab.tbl_id = LCN40PHY_TBL_ID_GAIN_IDX;
			tab.tbl_width = 32;
			tab.tbl_ptr = dot11lcn40_gain_idx_tbl_5G_ext_lna_rev4_fstr_cdd;
			tab.tbl_len = 46;
			tab.tbl_offset = 30;
			wlc_lcnphy_write_table(pi, &tab);

			tab.tbl_id = LCN40PHY_TBL_ID_GAIN_TBL;
			tab.tbl_width = 32;
			tab.tbl_ptr = dot11lcn40_gain_tbl_5G_ext_lna_rev4_fstr_cdd;
			tab.tbl_len = 8;
			tab.tbl_offset = 15;
			wlc_lcnphy_write_table(pi, &tab);

			tab.tbl_id = LCN40PHY_TBL_ID_GAIN_TBL;
			tab.tbl_width = 32;
			/* dot11lcn40_gain_tbl_5G_ext_lna_rev4[18] --> offset 23 to (23 + 73) */
			tab.tbl_ptr = (const void   *)(&temp_ptr[18]);
			tab.tbl_len = 73;
			tab.tbl_offset = 23;
			wlc_lcnphy_write_table(pi, &tab);
		}
#endif /* #ifdef BAND5G */
		if (pi_lcn40->cdd_mod == 1) {
			uint16 min_sig_sq_cdd = 0x160;
			int index;

			tab.tbl_id = LCN40PHY_TBL_ID_MIN_SIG_SQ;
			tab.tbl_width = 16;
			tab.tbl_ptr = &min_sig_sq_cdd;
			tab.tbl_len = 1;
			for (index = 0; index < 128; index++)
			{
			    tab.tbl_offset = index;
			    wlc_lcnphy_write_table(pi,  &tab);
			}
		}
	} /* if ((pi_lcn40->fstr_flag == 1) ||(pi_lcn40->cdd_mod == 1)) */
}

/* mapped to lcn40phy_rev0_reg_init */
static void
WLBANDINITFN(wlc_lcn40phy_rev0_reg_init)(phy_info_t *pi)
{
	phy_info_lcn40phy_t *pi_lcn40 = pi->u.pi_lcn40phy;

	if (NORADIO_ENAB(pi->pubpi)) {
		PHY_REG_LIST_START
			PHY_REG_WRITE_ENTRY(LCN40PHY, tx_resampler1, 0xe666)
			PHY_REG_WRITE_ENTRY(LCN40PHY, tx_resampler3, 0)
			PHY_REG_WRITE_ENTRY(LCN40PHY, tx_resampler4, 0x32)
			PHY_REG_WRITE_ENTRY(LCN40PHY, rxFarrowCtrl, 0x57)
			PHY_REG_WRITE_ENTRY(LCN40PHY, rxFarrowDeltaPhase, 0x2000)
			PHY_REG_WRITE_ENTRY(LCN40PHY, rxFarrowDriftPeriod, 0x780)
		PHY_REG_LIST_EXECUTE(pi);
	}
	if (LCN40REV_LT(pi->pubpi.phy_rev, 4))
		PHY_REG_MOD(pi, LCN40PHY, ClkEnCtrl, disable_stalls, 1);
	else {
		phy_utils_write_phyreg(pi, LCN40PHY_StallCtrl, 0);
		PHY_REG_MOD(pi, LCN40PHY, sslpnCalibClkEnCtrl, forcePhyRegsClkOn, 0);
	}

	PHY_REG_LIST_START
		PHY_REG_WRITE_ENTRY(LCN40PHY, resetCtrl, 0x0047)
		PHY_REG_MOD_ENTRY(LCN40PHY, agcControl4, c_agc_fsm_en, 0)
		PHY_REG_WRITE_ENTRY(LCN40PHY, AfeCtrlOvr, 0x0000)
		PHY_REG_WRITE_ENTRY(LCN40PHY, AfeCtrlOvr1, 0x0000)
		PHY_REG_WRITE_ENTRY(LCN40PHY, RFOverride0, 0x0000)
		PHY_REG_WRITE_ENTRY(LCN40PHY, rfoverride2, 0x0000)
		PHY_REG_WRITE_ENTRY(LCN40PHY, rfoverride3, 0x0000)
		PHY_REG_WRITE_ENTRY(LCN40PHY, rfoverride4, 0x0000)
		PHY_REG_WRITE_ENTRY(LCN40PHY, rfoverride7, 0x0000)
		PHY_REG_WRITE_ENTRY(LCN40PHY, rfoverride8, 0x0000)
		PHY_REG_WRITE_ENTRY(LCN40PHY, swctrlOvr, 0x0000)

		PHY_REG_MOD_ENTRY(LCN40PHY, wl_gain_tbl_offset, wl_gain_tbl_offset, 18)
		PHY_REG_MOD_ENTRY(LCN40PHY, nftrAdj, bt_gain_tbl_offset, 6)
		RADIO_REG_MOD_ENTRY(RADIO_2065_OVR9, 0x8, 0x8)
	PHY_REG_LIST_EXECUTE(pi);

	/* Force internal diversity switch to Main or Aux if so specified in nvram */
	if (pi_lcn40->irdsw == 0 || pi_lcn40->irdsw == 1) {
		PHY_REG_MOD(pi, LCN40PHY, RFOverrideVal0, ant_sel_int_ovr_val, pi_lcn40->irdsw);
		PHY_REG_MOD(pi, LCN40PHY, RFOverride0, ant_sel_int_ovr, 1);
	}

	if (CHSPEC_IS40(pi->radio_chanspec))
		phy_utils_write_radioreg(pi, RADIO_2065_RFPLL_CFG2, 0x740);
	else
	    phy_utils_write_radioreg(pi, RADIO_2065_RFPLL_CFG2, 0x750);

	PHY_REG_LIST_START
		RADIO_REG_WRITE_ENTRY(RADIO_2065_RFPLL_WILD_BASE0, 0x1b61)
		RADIO_REG_WRITE_ENTRY(RADIO_2065_RFPLL_WILD_BASE1, 0x604)

		PHY_REG_MOD_ENTRY(LCN40PHY, RFOverrideVal0, rfpll_pu_ovr_val, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, RFOverride0, rfpll_pu_ovr, 1)

		/* Have finished radio, RFPLL stable, so reenable PHY */
		PHY_REG_WRITE_ENTRY(LCN40PHY, resetCtrl, 0x0000)
		PHY_REG_MOD_ENTRY(LCN40PHY, agcControl4, c_agc_fsm_en, 1)
	PHY_REG_LIST_EXECUTE(pi);

	if (LCN40REV_LT(pi->pubpi.phy_rev, 4))
		PHY_REG_MOD(pi, LCN40PHY, ClkEnCtrl, disable_stalls, 0);

	if (LCN40REV_GE(pi->pubpi.phy_rev, 4))
		PHY_REG_MOD(pi, LCN40PHY, rxfe, swap_rxfe_iq_b0, 1);
	else
		PHY_REG_MOD(pi, LCN40PHY, rxfe, swap_rxfe_iq, 1);
	/* band selection */
#ifdef BAND5G
	if (CHSPEC_IS5G(pi->radio_chanspec)) {
		PHY_REG_MOD(pi, LCN40PHY, lpphyCtrl, muxGmode, 0);
	} else
#endif /* #ifdef BAND5G */
	{
		PHY_REG_MOD(pi, LCN40PHY, lpphyCtrl, muxGmode, 1);
	}
	/* bphy filter selection , for channel 14 it is 2 */
	PHY_REG_MOD(pi, LCN40PHY, lpphyCtrl, txfiltSelect, 1);

	if (NORADIO_ENAB(pi->pubpi)) {
		wlc_lcn40phy_set_bbmult(pi, 32);
		return;
	}
}

/* mapped to lcn40phy_rev1_reg_init */
static void
WLBANDINITFN(wlc_lcn40phy_rev1_reg_init)(phy_info_t *pi)
{
	if (NORADIO_ENAB(pi->pubpi)) {
		PHY_REG_LIST_START
			PHY_REG_WRITE_ENTRY(LCN40PHY, tx_resampler1, 0xe666)
			PHY_REG_WRITE_ENTRY(LCN40PHY, tx_resampler3, 0)
			PHY_REG_WRITE_ENTRY(LCN40PHY, tx_resampler4, 0x32)
			PHY_REG_WRITE_ENTRY(LCN40PHY, rxFarrowCtrl, 0x57)
			PHY_REG_WRITE_ENTRY(LCN40PHY, rxFarrowDeltaPhase, 0x2000)
			PHY_REG_WRITE_ENTRY(LCN40PHY, rxFarrowDriftPeriod, 0x780)
			PHY_REG_MOD_ENTRY(LCN40PHY, ClkEnCtrl, disable_stalls, 0)
		PHY_REG_LIST_EXECUTE(pi);
	}

	PHY_REG_LIST_START
		/* Put agc+fe in reset before adjusting radio later */
		PHY_REG_WRITE_ENTRY(LCN40PHY, resetCtrl, 0x0047)
		PHY_REG_MOD_ENTRY(LCN40PHY, agcControl4, c_agc_fsm_en, 0x0)

		PHY_REG_MOD_ENTRY(LCN40PHY, ClkEnCtrl, disable_stalls, 1)
		PHY_REG_WRITE_ENTRY(LCN40PHY, AfeCtrlOvr, 0x0000)
		PHY_REG_WRITE_ENTRY(LCN40PHY, AfeCtrlOvr1, 0x0000)
		PHY_REG_WRITE_ENTRY(LCN40PHY, RFOverride0, 0x0000)
		PHY_REG_WRITE_ENTRY(LCN40PHY, rfoverride2, 0x0000)
		PHY_REG_WRITE_ENTRY(LCN40PHY, rfoverride3, 0x0000)
		PHY_REG_WRITE_ENTRY(LCN40PHY, rfoverride4, 0x0000)
		PHY_REG_WRITE_ENTRY(LCN40PHY, rfoverride7, 0x0000)
		PHY_REG_WRITE_ENTRY(LCN40PHY, rfoverride8, 0x0000)
		PHY_REG_WRITE_ENTRY(LCN40PHY, swctrlOvr, 0x0000)

		PHY_REG_MOD_ENTRY(LCN40PHY, wl_gain_tbl_offset, wl_gain_tbl_offset, 18)
		PHY_REG_MOD_ENTRY(LCN40PHY, nftrAdj, bt_gain_tbl_offset, 6)

		RADIO_REG_MOD_ENTRY(RADIO_2065_OVR9, 0x8, 0x8)
	PHY_REG_LIST_EXECUTE(pi);

	if (CHSPEC_IS40(pi->radio_chanspec))
		phy_utils_write_radioreg(pi, RADIO_2065_RFPLL_CFG2, 0x740);
	else
		phy_utils_write_radioreg(pi, RADIO_2065_RFPLL_CFG2, 0x750);

	PHY_REG_LIST_START
		RADIO_REG_WRITE_ENTRY(RADIO_2065_RFPLL_WILD_BASE0, 0x1b61)
		RADIO_REG_WRITE_ENTRY(RADIO_2065_RFPLL_WILD_BASE1, 0x604)

		PHY_REG_MOD_ENTRY(LCN40PHY, RFOverrideVal0, rfpll_pu_ovr_val, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, RFOverride0, rfpll_pu_ovr, 1)
	PHY_REG_LIST_EXECUTE(pi);

	if (CHIPID(pi->sh->chip) == BCM4314_CHIP_ID) {
		PHY_ERROR(("%s: Fixing jtag2065 register init values.\n", __FUNCTION__));
		PHY_REG_LIST_START
			RADIO_REG_WRITE_ENTRY(RADIO_2065_RFPLL_WILD_BASE0, 0xcccc)
			RADIO_REG_WRITE_ENTRY(RADIO_2065_RFPLL_WILD_BASE1, 0xba4)
			RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CP_IDAC, 0x1f0, 0x70)
			RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_KPD, 0x1f, 0x4)
			RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_C4_C3, 0x1f1f, 0x1c1c)
			RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_C2_C1, 0x1f1f, 0x1c1c)
			RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_R3, 0x3f, 0xf)
			RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_R2_R1, 0x3f3f, 0xf21)
			RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CAL_CFG1, 0xe0, 0xa0)
			RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CAL_OVR_COUNT, 0xfff0, 0xe8d0)
			RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CAL_DELAYS3, 0x1f00, 0xf00)
			RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_REF_VAL1, 0xfff, 0x174)
			RADIO_REG_WRITE_ENTRY(RADIO_2065_RFPLL_REF_VAL0, 0x999a)
		PHY_REG_LIST_EXECUTE(pi);
	}

	OSL_DELAY(1000);

	PHY_REG_LIST_START
		PHY_REG_WRITE_ENTRY(LCN40PHY, resetCtrl, 0x0000)
		PHY_REG_MOD_ENTRY(LCN40PHY, agcControl4, c_agc_fsm_en, 0x1)
		PHY_REG_MOD_ENTRY(LCN40PHY, ClkEnCtrl, disable_stalls, 0)
		PHY_REG_MOD_ENTRY(LCN40PHY, rxfe, swap_rxfe_iq, 1)
	PHY_REG_LIST_EXECUTE(pi);
	/* band selection */
#ifdef BAND5G
	if (CHSPEC_IS5G(pi->radio_chanspec)) {
		PHY_REG_MOD(pi, LCN40PHY, lpphyCtrl, muxGmode, 0);
	} else
#endif // endif
	{
		PHY_REG_MOD(pi, LCN40PHY, lpphyCtrl, muxGmode, 1);
	}
	/* bphy filter selection , for channel 14 it is 2 */
	PHY_REG_MOD(pi, LCN40PHY, lpphyCtrl, txfiltSelect, 1);

	if (NORADIO_ENAB(pi->pubpi)) {
		wlc_lcn40phy_set_bbmult(pi, 32);
		return;
	}
}

static void
wlc_lcn40phy_agc_temp_init(phy_info_t *pi)
{
}

static void
WLBANDINITFN(wlc_lcn40phy_bu_tweaks)(phy_info_t *pi)
{
}

#define LCN40_NOTCH_NVRAM_NAME_SIZE 20
static const char BCMATTACHDATA(rstr_interference)[] = "interference";
static const char BCMATTACHDATA(rstr_triso2g)[] = "triso2g";
static const char BCMATTACHDATA(rstr_gain)[] = "gain";
static const char BCMATTACHDATA(rstr_elna_off_gain_idx_2g)[] = "elna_off_gain_idx_2g";
static const char BCMATTACHDATA(rstr_elna_off_gain_idx_5g)[] = "elna_off_gain_idx_5g";
static const char BCMATTACHDATA(rstr_rxpo2g)[] = "rxpo2g";
static const char BCMATTACHDATA(rstr_pa0b0)[] = "pa0b0";
static const char BCMATTACHDATA(rstr_pa0b1)[] = "pa0b1";
static const char BCMATTACHDATA(rstr_pa0b2)[] = "pa0b2";
static const char BCMATTACHDATA(rstr_pa0b0_lo)[] = "pa0b0_lo";
static const char BCMATTACHDATA(rstr_pa0b1_lo)[] = "pa0b1_lo";
static const char BCMATTACHDATA(rstr_pa0b2_lo)[] = "pa0b2_lo";
static const char BCMATTACHDATA(rstr_pmin)[] = "pmin";
static const char BCMATTACHDATA(rstr_pmax)[] = "pmax";
static const char BCMATTACHDATA(rstr_tssitime)[] = "tssitime";
static const char BCMATTACHDATA(rstr_itssicorr)[] = "itssicorr";
static const char BCMATTACHDATA(rstr_initxidx)[] = "initxidx";
static const char BCMATTACHDATA(rstr_tssioffsetmax)[] = "tssioffsetmax";
static const char BCMATTACHDATA(rstr_tssioffsetmin)[] = "tssioffsetmin";
static const char BCMATTACHDATA(rstr_tssioffsetmax5gl)[] = "tssioffsetmax5gl";
static const char BCMATTACHDATA(rstr_tssioffsetmin5gl)[] = "tssioffsetmin5gl";
static const char BCMATTACHDATA(rstr_tssioffsetmax5gm)[] = "tssioffsetmax5gm";
static const char BCMATTACHDATA(rstr_tssioffsetmin5gm)[] = "tssioffsetmin5gm";
static const char BCMATTACHDATA(rstr_tssioffsetmax5gh)[] = "tssioffsetmax5gh";
static const char BCMATTACHDATA(rstr_tssioffsetmin5gh)[] = "tssioffsetmin5gh";
static const char BCMATTACHDATA(rstr_tssifloor2g)[] = "tssifloor2g";
static const char BCMATTACHDATA(rstr_tssifloor5gl)[] = "tssifloor5gl";
static const char BCMATTACHDATA(rstr_tssifloor5gm)[] = "tssifloor5gm";
static const char BCMATTACHDATA(rstr_tssifloor5gh)[] = "tssifloor5gh";
static const char BCMATTACHDATA(rstr_tssimaxnpt)[] = "tssimaxnpt";
static const char BCMATTACHDATA(rstr_rssismf2g)[] = "rssismf2g";
static const char BCMATTACHDATA(rstr_rssismc2g)[] = "rssismc2g";
static const char BCMATTACHDATA(rstr_rssisav2g)[] = "rssisav2g";
static const char BCMATTACHDATA(rstr_rssismf2g_low0)[] = "rssismf2g_low0";
static const char BCMATTACHDATA(rstr_rssismc2g_low1)[] = "rssismc2g_low1";
static const char BCMATTACHDATA(rstr_rssisav2g_low2)[] = "rssisav2g_low2";
static const char BCMATTACHDATA(rstr_rssismf2g_hi0)[] = "rssismf2g_hi0";
static const char BCMATTACHDATA(rstr_rssismc2g_hi1)[] = "rssismc2g_hi1";
static const char BCMATTACHDATA(rstr_rssisav2g_hi2)[] = "rssisav2g_hi2";
static const char BCMATTACHDATA(rstr_rxgainerr2g)[] = "rxgainerr2g";
static const char BCMATTACHDATA(rstr_noiselvl2g)[] = "noiselvl2g";
static const char BCMATTACHDATA(rstr_noiseoff2g)[] = "noiseoff2g";
static const char BCMATTACHDATA(rstr_pwrthresh2g)[] = "pwrthresh2g";
static const char BCMATTACHDATA(rstr_rxgainerr5gl)[] = "rxgainerr5gl";
static const char BCMATTACHDATA(rstr_rxgainerr5gm)[] = "rxgainerr5gm";
static const char BCMATTACHDATA(rstr_rxgainerr5gh)[] = "rxgainerr5gh";
static const char BCMATTACHDATA(rstr_noiselvl5gl)[] = "noiselvl5gl";
static const char BCMATTACHDATA(rstr_noiselvl5gm)[] = "noiselvl5gm";
static const char BCMATTACHDATA(rstr_noiselvl5gh)[] = "noiselvl5gh";
static const char BCMATTACHDATA(rstr_noiseoff5gl)[] = "noiseoff5gl";
static const char BCMATTACHDATA(rstr_noiseoff5gm)[] = "noiseoff5gm";
static const char BCMATTACHDATA(rstr_noiseoff5gh)[] = "noiseoff5gh";
static const char BCMATTACHDATA(rstr_pwrthresh5g)[] = "pwrthresh5g";
static const char BCMATTACHDATA(rstr_rxgainfreqcorr_d)[] = "rxgainfreqcorr%d";
static const char BCMATTACHDATA(rstr_rxgaintempcoeff2g)[] = "rxgaintempcoeff2g";
static const char BCMATTACHDATA(rstr_rxgaintempcoeff5gl)[] = "rxgaintempcoeff5gl";
static const char BCMATTACHDATA(rstr_rxgaintempcoeff5gm)[] = "rxgaintempcoeff5gm";
static const char BCMATTACHDATA(rstr_rxgaintempcoeff5gh)[] = "rxgaintempcoeff5gh";
static const char BCMATTACHDATA(rstr_rssi_log_nsamps)[] = "rssi_log_nsamps";
static const char BCMATTACHDATA(rstr_rssi_iqest_en)[] = "rssi_iqest_en";
static const char BCMATTACHDATA(rstr_rssi_iqest_jssi_en)[] = "rssi_iqest_jssi_en";
static const char BCMATTACHDATA(rstr_rssi_iqest_gain_adj)[] = "rssi_iqest_gain_adj";
static const char BCMATTACHDATA(rstr_rssi_iqest_iov_gain_adj)[] = "rssi_iqest_iov_gain_adj";
static const char BCMATTACHDATA(rstr_noise_log_nsamps)[] = "noise_log_nsamps";
static const char BCMATTACHDATA(rstr_noise_iqest_en)[] = "noise_iqest_en";
static const char BCMATTACHDATA(rstr_noiseiqgainadj2g)[] = "noiseiqgainadj2g";
static const char BCMATTACHDATA(rstr_noiseiqgainadj5gl)[] = "noiseiqgainadj5gl";
static const char BCMATTACHDATA(rstr_noiseiqgainadj5gm)[] = "noiseiqgainadj5gm";
static const char BCMATTACHDATA(rstr_noiseiqgainadj5gh)[] = "noiseiqgainadj5gh";
static const char BCMATTACHDATA(rstr_maxp2ga0)[] = "maxp2ga0";
static const char BCMATTACHDATA(rstr_sromrev)[] = "sromrev";
static const char BCMATTACHDATA(rstr_cckbw202gpo)[] = "cckbw202gpo";
static const char BCMATTACHDATA(rstr_cck2gpo)[] = "cck2gpo";
static const char BCMATTACHDATA(rstr_legofdmbw202gpo)[] = "legofdmbw202gpo";
static const char BCMATTACHDATA(rstr_ofdm2gpo)[] = "ofdm2gpo";
static const char BCMATTACHDATA(rstr_mcsbw202gpo)[] = "mcsbw202gpo";
static const char BCMATTACHDATA(rstr_mcs2gpo1)[] = "mcs2gpo1";
static const char BCMATTACHDATA(rstr_mcs2gpo0)[] = "mcs2gpo0";
static const char BCMATTACHDATA(rstr_mcsbw402gpo)[] = "mcsbw402gpo";
static const char BCMATTACHDATA(rstr_mcs2gpo3)[] = "mcs2gpo3";
static const char BCMATTACHDATA(rstr_mcs2gpo2)[] = "mcs2gpo2";
static const char BCMATTACHDATA(rstr_maxp5ga0)[] = "maxp5ga0";
static const char BCMATTACHDATA(rstr_ofdm5gpo)[] = "ofdm5gpo";
static const char BCMATTACHDATA(rstr_mcs5gpo1)[] = "mcs5gpo1";
static const char BCMATTACHDATA(rstr_mcs5gpo0)[] = "mcs5gpo0";
static const char BCMATTACHDATA(rstr_mcs5gpo3)[] = "mcs5gpo3";
static const char BCMATTACHDATA(rstr_mcs5gpo2)[] = "mcs5gpo2";
static const char BCMATTACHDATA(rstr_maxp5gla0)[] = "maxp5gla0";
static const char BCMATTACHDATA(rstr_ofdm5glpo)[] = "ofdm5glpo";
static const char BCMATTACHDATA(rstr_mcs5glpo1)[] = "mcs5glpo1";
static const char BCMATTACHDATA(rstr_mcs5glpo0)[] = "mcs5glpo0";
static const char BCMATTACHDATA(rstr_mcs5glpo3)[] = "mcs5glpo3";
static const char BCMATTACHDATA(rstr_mcs5glpo2)[] = "mcs5glpo2";
static const char BCMATTACHDATA(rstr_maxp5gha0)[] = "maxp5gha0";
static const char BCMATTACHDATA(rstr_ofdm5ghpo)[] = "ofdm5ghpo";
static const char BCMATTACHDATA(rstr_mcs5ghpo1)[] = "mcs5ghpo1";
static const char BCMATTACHDATA(rstr_mcs5ghpo0)[] = "mcs5ghpo0";
static const char BCMATTACHDATA(rstr_mcs5ghpo3)[] = "mcs5ghpo3";
static const char BCMATTACHDATA(rstr_mcs5ghpo2)[] = "mcs5ghpo2";
static const char BCMATTACHDATA(rstr_rawtempsense)[] = "rawtempsense";
static const char BCMATTACHDATA(rstr_measpower)[] = "measpower";
static const char BCMATTACHDATA(rstr_measpower1)[] = "measpower1";
static const char BCMATTACHDATA(rstr_measpower2)[] = "measpower2";
static const char BCMATTACHDATA(rstr_tempsense_slope)[] = "tempsense_slope";
static const char BCMATTACHDATA(rstr_hw_iqcal_en)[] = "hw_iqcal_en";
static const char BCMATTACHDATA(rstr_iqcal_swp_dis)[] = "iqcal_swp_dis";
static const char BCMATTACHDATA(rstr_tempcorrx)[] = "tempcorrx";
static const char BCMATTACHDATA(rstr_tempsense_option)[] = "tempsense_option";
static const char BCMATTACHDATA(rstr_freqoffset_corr)[] = "freqoffset_corr";
static const char BCMATTACHDATA(rstr_aa2g)[] = "aa2g";
static const char BCMATTACHDATA(rstr_extpagain2g)[] = "extpagain2g";
static const char BCMATTACHDATA(rstr_extpagain5g)[] = "extpagain5g";
static const char BCMATTACHDATA(rstr_txpwrindex)[] = "txpwrindex";
static const char BCMATTACHDATA(rstr_txpwrindex5g)[] = "txpwrindex5g";
static const char BCMATTACHDATA(rstr_cckdigfilttype)[] = "cckdigfilttype";
static const char BCMATTACHDATA(rstr_ofdmdigfilttype)[] = "ofdmdigfilttype";
static const char BCMATTACHDATA(rstr_ofdmdigfilttype2g)[] = "ofdmdigfilttype2g";
static const char BCMATTACHDATA(rstr_ofdmdigfilttype5g)[] = "ofdmdigfilttype5g";
static const char BCMATTACHDATA(rstr_ofdm40digfilttype)[] = "ofdm40digfilttype";
static const char BCMATTACHDATA(rstr_ofdm40digfilttype2g)[] = "ofdm40digfilttype2g";
static const char BCMATTACHDATA(rstr_ofdm40digfilttype5g)[] = "ofdm40digfilttype5g";
static const char BCMATTACHDATA(rstr_pagc2g)[] = "pagc2g";
static const char BCMATTACHDATA(rstr_pagc5g)[] = "pagc5g";
static const char BCMATTACHDATA(rstr_txiqlotf)[] = "txiqlotf";
static const char BCMATTACHDATA(rstr_temp_mult)[] = "temp_mult";
static const char BCMATTACHDATA(rstr_temp_add)[] = "temp_add";
static const char BCMATTACHDATA(rstr_temp_q)[] = "temp_q";
static const char BCMATTACHDATA(rstr_vbat_mult)[] = "vbat_mult";
static const char BCMATTACHDATA(rstr_vbat_add)[] = "vbat_add";
static const char BCMATTACHDATA(rstr_vbat_q)[] = "vbat_q";
static const char BCMATTACHDATA(rstr_cckPwrOffset)[] = "cckPwrOffset";
static const char BCMATTACHDATA(rstr_boardtype)[] = "boardtype";
static const char BCMATTACHDATA(rstr_boardrev)[] = "boardrev";
static const char BCMATTACHDATA(rstr_cckPwrIdxCorr)[] = "cckPwrIdxCorr";
static const char BCMATTACHDATA(rstr_dacrate2xen)[] = "dacrate2xen";
static const char BCMATTACHDATA(rstr_dacpu)[] = "dacpu";
static const char BCMATTACHDATA(rstr_rfreg033)[] = "rfreg033";
static const char BCMATTACHDATA(rstr_rfreg033_cck)[] = "rfreg033_cck";
static const char BCMATTACHDATA(rstr_pacalidx2g)[] = "pacalidx2g";
static const char BCMATTACHDATA(rstr_pacalidx5g)[] = "pacalidx5g";
static const char BCMATTACHDATA(rstr_parfps)[] = "parfps";
static const char BCMATTACHDATA(rstr_papdsfac2g)[] = "papdsfac2g";
static const char BCMATTACHDATA(rstr_papdsfac5g)[] = "papdsfac5g";
static const char BCMATTACHDATA(rstr_swctrlmap_2g)[] = "swctrlmap_2g";
static const char BCMATTACHDATA(rstr_swctrlmap_5g)[] = "swctrlmap_5g";

static const char BCMATTACHDATA(rstr_paprr_enable2g)[] = "paprr_enable2g";
static const char BCMATTACHDATA(rstr_paprr_enable5g)[] = "paprr_enable5g";

static const char BCMATTACHDATA(rstr_paprgaintbl2g)[] = "paprgaintbl2g";
static const char BCMATTACHDATA(rstr_paprgaintbl5g)[] = "paprgaintbl5g";

static const char BCMATTACHDATA(rstr_paprgamtbl2g)[] = "paprgamtbl2g";
static const char BCMATTACHDATA(rstr_paprgamtbl5g)[] = "paprgamtbl5g";

static const char BCMATTACHDATA(rstr_paprofftbl2g)[] = "paprofftbl2g";
static const char BCMATTACHDATA(rstr_paprofftbl5g)[] = "paprofftbl5g";

static const char BCMATTACHDATA(rstr_dacgc5g)[] = "dacgc5g";
static const char BCMATTACHDATA(rstr_gmgc5g)[] = "gmgc5g";
static const char BCMATTACHDATA(rstr_pa1lob0)[] = "pa1lob0";
static const char BCMATTACHDATA(rstr_pa1lob1)[] = "pa1lob1";
static const char BCMATTACHDATA(rstr_pa1lob2)[] = "pa1lob2";
static const char BCMATTACHDATA(rstr_pa1b0)[] = "pa1b0";
static const char BCMATTACHDATA(rstr_pa1b1)[] = "pa1b1";
static const char BCMATTACHDATA(rstr_pa1b2)[] = "pa1b2";
static const char BCMATTACHDATA(rstr_pa1hib0)[] = "pa1hib0";
static const char BCMATTACHDATA(rstr_pa1hib1)[] = "pa1hib1";
static const char BCMATTACHDATA(rstr_pa1hib2)[] = "pa1hib2";
static const char BCMATTACHDATA(rstr_rssismf5g)[] = "rssismf5g";
static const char BCMATTACHDATA(rstr_rssismc5g)[] = "rssismc5g";
static const char BCMATTACHDATA(rstr_rssisav5g)[] = "rssisav5g";
static const char BCMATTACHDATA(rstr_initxidx5g)[] = "initxidx5g";
static const char BCMATTACHDATA(rstr_rx_iq_comp_5g)[] = "rx_iq_comp_5g";
static const char BCMATTACHDATA(rstr_rx_iq_comp_2g)[] = "rx_iq_comp_2g";
static const char BCMATTACHDATA(rstr_dacgc2g)[] = "dacgc2g";
static const char BCMATTACHDATA(rstr_gmgc2g)[] = "gmgc2g";
static const char BCMATTACHDATA(rstr_ofdmanalogfiltbw)[] = "ofdmanalogfiltbw";
static const char BCMATTACHDATA(rstr_txalpfbyp)[] = "txalpfbyp";
static const char BCMATTACHDATA(rstr_txalpfpu)[] = "txalpfpu";
static const char BCMATTACHDATA(rstr_bphyscale)[] = "bphyscale";
static const char BCMATTACHDATA(rstr_noise_cal_high_gain_2g)[] = "noise_cal_high_gain_2g";
static const char BCMATTACHDATA(rstr_noise_cal_high_gain_5g)[] = "noise_cal_high_gain_5g";
static const char BCMATTACHDATA(rstr_noise_cal_nf_substract_val_2g)[] =
  "noise_cal_nf_substract_val_2g";
static const char BCMATTACHDATA(rstr_noise_cal_nf_substract_val_5g)[] =
  "noise_cal_nf_substract_val_5g";
static const char BCMATTACHDATA(rstr_noise_cal_update)[] = "noise_cal_update";
static const char BCMATTACHDATA(rstr_noise_cal_po_2g)[] = "noise_cal_po_2g";
static const char BCMATTACHDATA(rstr_noise_cal_po_5g)[] = "noise_cal_po_5g";
static const char BCMATTACHDATA(rstr_noise_cal_po_40_2g)[] = "noise_cal_po_40_2g";
static const char BCMATTACHDATA(rstr_noise_cal_po_40_5g)[] = "noise_cal_po_40_5g";
static const char BCMATTACHDATA(rstr_noise_cal_enable_2g)[] = "noise_cal_enable_2g";
static const char BCMATTACHDATA(rstr_noise_cal_enable_5g)[] = "noise_cal_enable_5g";
static const char BCMATTACHDATA(rstr_noise_cal_ref_2g)[] = "noise_cal_ref_2g";
static const char BCMATTACHDATA(rstr_noise_cal_ref_5g)[] = "noise_cal_ref_5g";
static const char BCMATTACHDATA(rstr_noise_cal_ref_40_2g)[] = "noise_cal_ref_40_2g";
static const char BCMATTACHDATA(rstr_noise_cal_ref_40_5g)[] = "noise_cal_ref_40_5g";
static const char BCMATTACHDATA(rstr_noise_cal_adj_2g)[] = "noise_cal_adj_2g";
static const char BCMATTACHDATA(rstr_noise_cal_adj_5g)[] = "noise_cal_adj_5g";
static const char BCMATTACHDATA(rstr_rxgaintempcorr2g)[] = "rxgaintempcorr2g";
static const char BCMATTACHDATA(rstr_rxgaintempcorr5gh)[] = "rxgaintempcorr5gh";
static const char BCMATTACHDATA(rstr_rxgaintempcorr5gm)[] = "rxgaintempcorr5gm";
static const char BCMATTACHDATA(rstr_rxgaintempcorr5gl)[] = "rxgaintempcorr5gl";
static const char BCMATTACHDATA(rstr_xtalmode)[] = "xtalmode";
static const char BCMATTACHDATA(rstr_triso5g)[] = "triso5g";
static const char BCMATTACHDATA(rstr_PwrOffset40mhz2g)[] = "PwrOffset40mhz2g";
static const char BCMATTACHDATA(rstr_PwrOffset40mhz5g)[] = "PwrOffset40mhz5g";
static const char BCMATTACHDATA(rstr_lpoff)[] = "lpoff";
static const char BCMATTACHDATA(rstr_iqlocalidx5g)[] = "iqlocalidx5g";
static const char BCMATTACHDATA(rstr_txiqlopapu2g)[] = "txiqlopapu2g";
static const char BCMATTACHDATA(rstr_txiqlopapu5g)[] = "txiqlopapu5g";
static const char BCMATTACHDATA(rstr_mixboost5g)[] = "mixboost5g";
static const char BCMATTACHDATA(rstr_gain_settle_dly_2g)[] = "gain_settle_dly_2g";
static const char BCMATTACHDATA(rstr_gain_settle_dly_5g)[] = "gain_settle_dly_5g";
static const char BCMATTACHDATA(rstr_tia_dc_loop_enable_2g)[] = "tia_dc_loop_enable_2g";
static const char BCMATTACHDATA(rstr_tia_dc_loop_enable_5g)[] = "tia_dc_loop_enable_5g";
static const char BCMATTACHDATA(rstr_hpc_sequencer_enable_2g)[] = "hpc_sequencer_enable_2g";
static const char BCMATTACHDATA(rstr_hpc_sequencer_enable_5g)[] = "hpc_sequencer_enable_5g";
static const char BCMATTACHDATA(rstr_padbias5g)[] = "padbias5g";
static const char BCMATTACHDATA(rstr_aci_detect_en_2g)[] = "aci_detect_en_2g";
static const char BCMATTACHDATA(rstr_adc_lowpower_mode)[] = "adc_lowpower_mode";
static const char BCMATTACHDATA(rstr_lpf_lowpower_mode)[] = "lpf_lowpower_mode";
static const char BCMATTACHDATA(rstr_lna1_2g_lowpower_mode)[] = "lna1_2g_lowpower_mode";
static const char BCMATTACHDATA(rstr_lna2_2g_lowpower_mode)[] = "lna2_2g_lowpower_mode";
static const char BCMATTACHDATA(rstr_rxmix_2g_lowpower_mode)[] = "rxmix_2g_lowpower_mode";
static const char BCMATTACHDATA(rstr_tx_agc_reset_2g)[] = "tx_agc_reset_2g";
static const char BCMATTACHDATA(rstr_gaintbl_force_2g)[] = "gaintbl_force_2g";
static const char BCMATTACHDATA(rstr_gaintbl_presel_2g)[] = "gaintbl_presel_2g";
static const char BCMATTACHDATA(rstr_aci_detect_en_5g)[] = "aci_detect_en_5g";
static const char BCMATTACHDATA(rstr_tx_agc_reset_5g)[] = "tx_agc_reset_5g";
static const char BCMATTACHDATA(rstr_gaintbl_force_5g)[] = "gaintbl_force_5g";
static const char BCMATTACHDATA(rstr_gaintbl_presel_5g)[] = "gaintbl_presel_5g";
static const char BCMATTACHDATA(rstr_iqcalidx5g)[] = "iqcalidx5g";
static const char BCMATTACHDATA(rstr_tpc_temp_hi)[] = "tpc_temp_hi";
static const char BCMATTACHDATA(rstr_tpc_temp_offs1_2g)[] = "tpc_temp_offs1_2g";
static const char BCMATTACHDATA(rstr_tpc_temp_offs1_5g)[] = "tpc_temp_offs1_5g";
static const char BCMATTACHDATA(rstr_tpc_temp_low)[] = "tpc_temp_low";
static const char BCMATTACHDATA(rstr_tpc_temp_offs2_2g)[] = "tpc_temp_offs2_2g";
static const char BCMATTACHDATA(rstr_tpc_temp_offs2_5g)[] = "tpc_temp_offs2_5g";
static const char BCMATTACHDATA(rstr_tpc_temp_diff)[] = "tpc_temp_diff";
static const char BCMATTACHDATA(rstr_tpc_vbat_hi)[] = "tpc_vbat_hi";
static const char BCMATTACHDATA(rstr_tpc_vbat_offs1_2g)[] = "tpc_vbat_offs1_2g";
static const char BCMATTACHDATA(rstr_tpc_vbat_offs1_5g)[] = "tpc_vbat_offs1_5g";
static const char BCMATTACHDATA(rstr_tpc_vbat_low)[] = "tpc_vbat_low";
static const char BCMATTACHDATA(rstr_tpc_vbat_offs2_2g)[] = "tpc_vbat_offs2_2g";
static const char BCMATTACHDATA(rstr_tpc_vbat_offs2_5g)[] = "tpc_vbat_offs2_5g";
static const char BCMATTACHDATA(rstr_tpc_vbat_diff)[] = "tpc_vbat_diff";
static const char BCMATTACHDATA(rstr_lpbckmode5g)[] = "lpbckmode5g";
static const char BCMATTACHDATA(rstr_loflag)[] = "loflag";
static const char BCMATTACHDATA(rstr_dlocalidx5g)[] = "dlocalidx5g";
static const char BCMATTACHDATA(rstr_dlorange_lowlimit)[] = "dlorange_lowlimit";
static const char BCMATTACHDATA(rstr_noise_cal_deltamax)[] = "noise_cal_deltamax";
static const char BCMATTACHDATA(rstr_noise_cal_deltamin)[] = "noise_cal_deltamin";
static const char BCMATTACHDATA(rstr_dsss_thresh)[] = "dsss_thresh";
static const char BCMATTACHDATA(rstr_startdiq2g)[] = "startdiq2g";
static const char BCMATTACHDATA(rstr_startdiq5g)[] = "startdiq5g";
static const char BCMATTACHDATA(rstr_offtgpwr)[] = "offtgpwr";
static const char BCMATTACHDATA(rstr_boostackpwr)[] = "boostackpwr";
static const char BCMATTACHDATA(rstr_initxidx2g)[] = "initxidx2g";
static const char BCMATTACHDATA(rstr_cckinitxidx)[] = "cckinitxidx";
static const char BCMATTACHDATA(rstr_notch)[][LCN40_NOTCH_NVRAM_NAME_SIZE] =
	{
		"notch2412", "notch2417", "notch2422", "notch2427",
		"notch2432", "notch2437", "notch2442", "notch2447",
		"notch2452", "notch2457", "notch2462", "notch2467",
		"notch2472", "notch2484"
	};
static const char BCMATTACHDATA(rstr_localoffs5gmh)[] = "localoffs5gmh";
static const char BCMATTACHDATA(rstr_temp_offset_2g)[] = "temp_offset_2g";
static const char BCMATTACHDATA(rstr_temp_offset_5g)[] = "temp_offset_5g";
static const char BCMATTACHDATA(rstr_temp_cal_en_2g)[] = "temp_cal_en_2g";
static const char BCMATTACHDATA(rstr_temp_cal_en_5g)[] = "temp_cal_en_5g";
static const char BCMATTACHDATA(rstr_vlin2g)[] = "vlin2g";
static const char BCMATTACHDATA(rstr_vlin5g)[] = "vlin5g";
static const char BCMATTACHDATA(rstr_edonthd40)[] = "edonthd40";
static const char BCMATTACHDATA(rstr_edoffthd40)[] = "edoffthd40";
static const char BCMATTACHDATA(rstr_edonthd20u)[] = "edonthd20u";
static const char BCMATTACHDATA(rstr_edonthd20l)[] = "edonthd20l";
static const char BCMATTACHDATA(rstr_edoffthd20ul)[] = "edoffthd20ul";
static const char BCMATTACHDATA(rstr_papden2g)[] = "papden2g";
static const char BCMATTACHDATA(rstr_papden5g)[] = "papden5g";
static const char BCMATTACHDATA(rstr_papdlinpath2g)[] = "papdlinpath2g";
static const char BCMATTACHDATA(rstr_papdlinpath5g)[] = "papdlinpath5g";
static const char BCMATTACHDATA(rstr_papdmagth2g)[] = "papdmagth2g";
static const char BCMATTACHDATA(rstr_papdmagth5g)[] = "papdmagth5g";
static const char BCMATTACHDATA(rstr_plldoubler_enable2g)[] = "plldoubler_enable2g";
static const char BCMATTACHDATA(rstr_plldoubler_enable5g)[] = "plldoubler_enable5g";
static const char BCMATTACHDATA(rstr_loopbw2g)[] = "loopbw2g";
static const char BCMATTACHDATA(rstr_loopbw5g)[] = "loopbw5g";
static const char BCMATTACHDATA(rstr_tempcaladj2g)[] = "tempcaladj2g";
static const char BCMATTACHDATA(rstr_tempcaladj5g)[] = "tempcaladj5g";
static const char BCMATTACHDATA(rstr_fstr)[] = "fstr";
static const char BCMATTACHDATA(rstr_cddmod)[] = "cddmod";
static const char BCMATTACHDATA(rstr_papden2gchan)[] = "papden2gchan";
static const char BCMATTACHDATA(rstr_calidxestbase2g)[] = "calidxestbase2g";
static const char BCMATTACHDATA(rstr_calidxesttarget2g)[] = "calidxesttarget2g";
static const char BCMATTACHDATA(rstr_calidxestbase5g)[] = "calidxestbase5g";
static const char BCMATTACHDATA(rstr_calidxesttarget5g)[] = "calidxesttarget5g";
static const char BCMATTACHDATA(rstr_calidxesttarget405g)[] = "calidxesttarget405g";
static const char BCMATTACHDATA(rstr_calidxesttargetlo5g)[] = "calidxesttargetlo5g";
static const char BCMATTACHDATA(rstr_calidxesttarget40lo5g)[] = "calidxesttarget40lo5g";
static const char BCMATTACHDATA(rstr_calidxesttargethi5g)[] = "calidxesttargethi5g";
static const char BCMATTACHDATA(rstr_calidxesttarget40hi5g)[] = "calidxesttarget40hi5g";
static const char BCMATTACHDATA(rstr_clipthr_eLNA2g)[] = "clipthr_eLNA2g";
static const char BCMATTACHDATA(rstr_papdrx2g)[] = "papdrx2g";
static const char BCMATTACHDATA(rstr_papdrx5g)[] = "papdrx5g";
static const char BCMATTACHDATA(rstr_rssi_gain_delta_2g)[] = "rssi_gain_delta_2g";
static const char BCMATTACHDATA(rstr_rssi_gain_delta_2gh)[] = "rssi_gain_delta_2gh";
static const char BCMATTACHDATA(rstr_rssi_gain_delta_2ghh)[] = "rssi_gain_delta_2ghh";
static const char BCMATTACHDATA(rstr_rssi_gain_delta_5gl)[] = "rssi_gain_delta_5gl";
static const char BCMATTACHDATA(rstr_rssi_gain_delta_5gml)[] = "rssi_gain_delta_5gml";
static const char BCMATTACHDATA(rstr_rssi_gain_delta_5gmu)[] = "rssi_gain_delta_5gmu";
static const char BCMATTACHDATA(rstr_rssi_gain_delta_5gh)[] = "rssi_gain_delta_5gh";
static const char BCMATTACHDATA(rstr_gain_cal_temp)[] = "gain_cal_temp";
static const char BCMATTACHDATA(rstr_txidxcap2g)[] = "txidxcap2g";
static const char BCMATTACHDATA(rstr_txidxcap5g)[] = "txidxcap5g";
static const char BCMATTACHDATA(rstr_txidxcap2g_hi)[] = "txidxcap2g_hi";
static const char BCMATTACHDATA(rstr_txidxcap2g_lo)[] = "txidxcap2g_lo";
static const char BCMATTACHDATA(rstr_txidxcap5g_hi)[] = "txidxcap5g_hi";
static const char BCMATTACHDATA(rstr_txidxcap5g_lo)[] = "txidxcap5g_lo";
static const char BCMATTACHDATA(rstr_txidxcap2g_off)[] = "txidxcap2g_off";
static const char BCMATTACHDATA(rstr_txidxcap5g_off)[] = "txidxcap5g_off";
static const char BCMATTACHDATA(rstr_rssi_rxsaw_slope_2g)[] = "rssi_rxsaw_slope_2g";
static const char BCMATTACHDATA(rstr_rssi_dp2_slope_2g)[] = "rssi_dp2_slope_2g";
static const char BCMATTACHDATA(rstr_rssi_eLNAbyp_slope_2g)[] = "rssi_eLNAbyp_slope_2g";
static const char BCMATTACHDATA(rstr_rssi_eLNAbyp_slope_5g)[] = "rssi_eLNAbyp_slope_5g";
static const char BCMATTACHDATA(rstr_rssi_eLNAon_slope_2g)[] = "rssi_eLNAon_slope_2g";
static const char BCMATTACHDATA(rstr_rssi_eLNAon_slope_5gh)[] = "rssi_eLNAon_slope_5gh";
static const char BCMATTACHDATA(rstr_rssi_eLNAon_slope_5gmu)[] = "rssi_eLNAon_slope_5gmu";
static const char BCMATTACHDATA(rstr_rssi_eLNAon_slope_5gml)[] = "rssi_eLNAon_slope_5gml";
static const char BCMATTACHDATA(rstr_rssi_eLNAon_slope_5gl)[] = "rssi_eLNAon_slope_5gl";
static const char BCMATTACHDATA(rstr_lpfflg)[] = "lpfflg";
static const char BCMATTACHDATA(rstr_cckscale)[] = "cckscale";
static const char BCMATTACHDATA(rstr_irdsw)[] = "irdsw";
static const char BCMATTACHDATA(rstr_scram_off_2g)[] = "scram_off_2g";
static const char BCMATTACHDATA(rstr_scram_off_5g)[] = "scram_off_5g";

static const uint8 paprgaintbl2g_t[20] = {0, 0, 0, 0, 167, 165, 163, 159, 154, 149,
	147, 148, 165, 163, 157, 153, 148, 147, 146, 145};
static const uint32 paprgamtbl2g_t[20]  = {0, 0, 0, 0, 0x19a000, 0x21c000, 0x280000,
	0x2f8000, 0x370000, 0x410000, 0x46e000, 0x47a000, 0x17c000, 0x26c000, 0x2f2000,
	0x366000, 0x40a000, 0x46a000, 0x496000, 0x4ca000};
static const int8 paprofftbl2g_t[20]  =  {0, 0, 0, 0, -6, -6, -5, -4, -1,
	0, 0, 0, -4, -4, -2, -1, 0, 0, 0, 0};

#ifdef BAND5G
static const uint8 paprgaintbl5g_t[20] = {0, 0, 0, 0, 167, 165, 163, 159, 155,
	151, 146, 146, 164, 162, 156, 153, 151, 147, 149, 147};
static const uint32 paprgamtbl5g_t[20]  = {0, 0, 0, 0, 0x1a0000, 0x222000, 0x278000,
	0x2ea000, 0x356000, 0x3d4000, 0x474000,
	0x4e2000, 0x1a8000, 0x294000, 0x302000,
	0x350000, 0x3ac000, 0x464000, 0x420000, 0x460000};
static const int8 paprofftbl5g_t[20]  =  {0, 0, 0, 0, -6, -7, -5, -3, -1, -1, -1,
	-1, -4, -3, -2, -1, -1, -1, -1, -1};

static const uint8 papr40gaintbl5g_t[20] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	158, 156, 150, 151, 148, 144, 145, 142};
static const uint32 papr40gamtbl5g_t[20]  = {0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0x226000, 0x2c4000, 0x320000,
	0x374000, 0x3de000, 0x474000, 0x438000, 0x488000};
static const int8 papr40offtbl5g_t[20]  =  {0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, -2, -2, -2, -2, -1, -1, -1, -1};
#endif /* BAND5G */

/* Read band specific data from the SROM */
bool
BCMATTACHFN(wlc_lcn40phy_txpwr_srom_read)(phy_info_t *pi)
{
	int i, ch;
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	phy_info_lcn40phy_t *pi_lcn40 = pi->u.pi_lcn40phy;
#if defined(WLTEST)
	chan_info_2065_lcn40phy_lna_corr_t *freq_corr;
	char str1[20];
#endif // endif
	uint boardtype, boardrev;
	int16 txidxcap_2g = 0;

	boardtype = PHY_GETINTVAR(pi, rstr_boardtype);
	boardrev = PHY_GETINTVAR(pi, rstr_boardrev);

	/* TR switch isolation */
	pi_lcn->lcnphy_tr_isolation_mid = (uint8)PHY_GETINTVAR(pi, rstr_triso2g);

	pi_lcn40->trGain = (uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_gain, 0xff);
	pi_lcn40->elna_off_gain_idx_2g =
		(uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_elna_off_gain_idx_2g, 0xff);
	pi_lcn40->elna_off_gain_idx_5g =
		(uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_elna_off_gain_idx_5g, 0xff);
	/* Input power offset */
	pi_lcn->lcnphy_rx_power_offset = (uint8)PHY_GETINTVAR(pi, rstr_rxpo2g);
	/* pa0b0 */
	pi->txpa_2g[0] = (int16)PHY_GETINTVAR(pi, rstr_pa0b0);
	pi->txpa_2g[1] = (int16)PHY_GETINTVAR(pi, rstr_pa0b1);
	pi->txpa_2g[2] = (int16)PHY_GETINTVAR(pi, rstr_pa0b2);

#if TWO_POWER_RANGE_TXPWR_CTRL
		if (PHY_GETVAR(pi, rstr_pa0b0_lo))
			pi_lcn->lcnphy_twopwr_txpwrctrl_en = 1;
		if (pi_lcn->lcnphy_twopwr_txpwrctrl_en) {
			pi->txpa_2g_lo[0] = (int16)PHY_GETINTVAR(pi, rstr_pa0b0_lo);
			pi->txpa_2g_lo[1] = (int16)PHY_GETINTVAR(pi, rstr_pa0b1_lo);
			pi->txpa_2g_lo[2] = (int16)PHY_GETINTVAR(pi, rstr_pa0b2_lo);
			pi_lcn->pmin = (int8)PHY_GETINTVAR(pi, rstr_pmin);
			pi_lcn->pmax = (int8)PHY_GETINTVAR(pi, rstr_pmax);
		}
#endif // endif

	pi_lcn->lcnphy_tssical_time = (uint32)PHY_GETINTVAR(pi, rstr_tssitime);
	pi_lcn40->lcnphy_idletssi_corr = (uint32)PHY_GETINTVAR(pi, rstr_itssicorr);

	pi_lcn->init_txpwrindex_2g = (uint32)PHY_GETINTVAR_DEFAULT(pi, rstr_initxidx2g,
		LCN40PHY_TX_PWR_CTRL_START_INDEX_2G);
	pi_lcn->tssi_ladder_offset_maxpwr_2g =
		(uint32)PHY_GETINTVAR_DEFAULT(pi, rstr_tssioffsetmax, 8);
	pi_lcn->tssi_ladder_offset_minpwr_2g =
		(uint32)PHY_GETINTVAR_DEFAULT(pi, rstr_tssioffsetmin, 3);
	pi_lcn->tssi_ladder_offset_maxpwr_5glo = (
		uint32)PHY_GETINTVAR_DEFAULT(pi, rstr_tssioffsetmax5gl, 8);
	pi_lcn->tssi_ladder_offset_minpwr_5glo =
		(uint32)PHY_GETINTVAR_DEFAULT(pi, rstr_tssioffsetmin5gl, 3);
	pi_lcn->tssi_ladder_offset_maxpwr_5gmid =
		(uint32)PHY_GETINTVAR_DEFAULT(pi, rstr_tssioffsetmax5gm, 8);
	pi_lcn->tssi_ladder_offset_minpwr_5gmid =
		(uint32)PHY_GETINTVAR_DEFAULT(pi, rstr_tssioffsetmin5gm, 3);
	pi_lcn->tssi_ladder_offset_maxpwr_5ghi =
		(uint32)PHY_GETINTVAR_DEFAULT(pi, rstr_tssioffsetmax5gh, 8);
	pi_lcn->tssi_ladder_offset_minpwr_5ghi =
		(uint32)PHY_GETINTVAR_DEFAULT(pi, rstr_tssioffsetmin5gh, 3);

	pi_lcn->tssi_floor_2g =
		(uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_tssifloor2g, 0);
	pi_lcn->tssi_floor_5glo = (
		uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_tssifloor5gl, 0);
	pi_lcn->tssi_floor_5gmid =
		(uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_tssifloor5gm, 0);
	pi_lcn->tssi_floor_5ghi =
		(uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_tssifloor5gh, 0);

	if (PHY_GETVAR(pi, rstr_tssifloor5gl))
		pi_lcn->txpwr_tssifloor_clamp_dis = 0;
	else
		pi_lcn->txpwr_tssifloor_clamp_dis = 1;

	if (PHY_GETVAR(pi, rstr_tssioffsetmax5gl))
		pi_lcn->txpwr_tssioffset_clamp_dis = 0;

	if (pi_lcn->txpwr_tssifloor_clamp_dis == 0)
		pi_lcn->txpwr_tssioffset_clamp_dis = 1;
	else
		pi_lcn->txpwr_tssioffset_clamp_dis = 0;

		pi_lcn->txpwr_tssifloor_clamp_dis_5g = pi_lcn->txpwr_tssifloor_clamp_dis;
		pi_lcn->txpwr_tssioffset_clamp_dis_5g = pi_lcn->txpwr_tssioffset_clamp_dis;

	if (PHY_GETVAR(pi, rstr_tssifloor2g))
		pi_lcn->txpwr_tssifloor_clamp_dis = 0;
	else
		pi_lcn->txpwr_tssifloor_clamp_dis = 1;

	if (PHY_GETVAR(pi, rstr_tssioffsetmax))
		pi_lcn->txpwr_tssioffset_clamp_dis = 0;

	if (pi_lcn->txpwr_tssifloor_clamp_dis == 0)
		pi_lcn->txpwr_tssioffset_clamp_dis = 1;
	else
		pi_lcn->txpwr_tssioffset_clamp_dis = 0;

		pi_lcn->txpwr_tssifloor_clamp_dis_2g = pi_lcn->txpwr_tssifloor_clamp_dis;
		pi_lcn->txpwr_tssioffset_clamp_dis_2g = pi_lcn->txpwr_tssioffset_clamp_dis;

	pi_lcn->tssi_max_npt = (uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_tssimaxnpt,
		LCN40PHY_TX_PWR_CTRL_MAX_NPT);

	/* RSSI */
	pi_lcn->lcnphy_rssi_vf = (uint8)PHY_GETINTVAR(pi, rstr_rssismf2g);
	pi_lcn->lcnphy_rssi_vc = (uint8)PHY_GETINTVAR(pi, rstr_rssismc2g);
	pi_lcn->lcnphy_rssi_gs = (uint8)PHY_GETINTVAR(pi, rstr_rssisav2g);

	{
		pi_lcn->lcnphy_rssi_vf_lowtemp = pi_lcn->lcnphy_rssi_vf;
		pi_lcn->lcnphy_rssi_vc_lowtemp = pi_lcn->lcnphy_rssi_vc;
		pi_lcn->lcnphy_rssi_gs_lowtemp = pi_lcn->lcnphy_rssi_gs;

		pi_lcn->lcnphy_rssi_vf_hightemp = pi_lcn->lcnphy_rssi_vf;
		pi_lcn->lcnphy_rssi_vc_hightemp = pi_lcn->lcnphy_rssi_vc;
		pi_lcn->lcnphy_rssi_gs_hightemp = pi_lcn->lcnphy_rssi_gs;
	}

	pi_lcn->rxgain_tempadj_2g =
		(int16)PHY_GETINTVAR_DEFAULT(pi, rstr_rxgaintempcoeff2g, 72);
	pi_lcn->rxgain_tempadj_5gl =
		(int16)PHY_GETINTVAR_DEFAULT(pi, rstr_rxgaintempcoeff5gl, 54);
	pi_lcn->rxgain_tempadj_5gm =
		(int16)PHY_GETINTVAR_DEFAULT(pi, rstr_rxgaintempcoeff5gm, 60);
	pi_lcn->rxgain_tempadj_5gh =
		(int16)PHY_GETINTVAR_DEFAULT(pi, rstr_rxgaintempcoeff5gh, 62);

	pi_lcn40->rssi_log_nsamps =
		(uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_rssi_log_nsamps, 8);
	pi_lcn40->rssi_iqest_en =
		(uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_rssi_iqest_en, 0);
	pi_lcn40->rssi_iqest_iov_gain_adj =
		(uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_rssi_iqest_iov_gain_adj, -32);
	pi_lcn40->rssi_iqest_gain_adj =
		(uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_rssi_iqest_gain_adj, -30);
	pi_lcn40->rssi_iqest_jssi_en =
		(uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_rssi_iqest_jssi_en, 0);

	pi_lcn40->gain_cal_temp = (int8)PHY_GETINTVAR_DEFAULT(pi, rstr_gain_cal_temp,
		LCN40_RSSI_NOMINAL_TEMP);

	pi_lcn40->noise_log_nsamps =
		(uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_noise_log_nsamps, 6);
	pi_lcn40->noise_iqest_en =
		(uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_noise_iqest_en, 0);
	pi_lcn40->noise_iqest_gain_adj_2g =
		(int8)PHY_GETINTVAR_DEFAULT(pi, rstr_noiseiqgainadj2g, -31);
	pi_lcn40->noise_iqest_gain_adj_5gl =
		(int8)PHY_GETINTVAR_DEFAULT(pi, rstr_noiseiqgainadj5gl, -27);
	pi_lcn40->noise_iqest_gain_adj_5gm =
		(int8)PHY_GETINTVAR_DEFAULT(pi, rstr_noiseiqgainadj5gm, -27);
	pi_lcn40->noise_iqest_gain_adj_5gh =
		(int8)PHY_GETINTVAR_DEFAULT(pi, rstr_noiseiqgainadj5gh, -27);

	for (i = 0; i < LCN40PHY_GAIN_DELTA_2G_PARAMS; i++) {
		pi_lcn40->rssi_gain_delta_2g[i] = (int8)PHY_GETINTVAR_ARRAY_DEFAULT(pi,
			rstr_rssi_gain_delta_2g, i, 0);

		if (PHY_GETVAR(pi, rstr_rssi_gain_delta_2gh))
			pi_lcn40->rssi_gain_delta_2gh[i] =
				(int8) PHY_GETINTVAR_ARRAY(pi, rstr_rssi_gain_delta_2gh, i);
		else
			pi_lcn40->rssi_gain_delta_2gh[i] = pi_lcn40->rssi_gain_delta_2g[i];

		if (PHY_GETVAR(pi, rstr_rssi_gain_delta_2ghh))
			pi_lcn40->rssi_gain_delta_2ghh[i] =
				(int8) PHY_GETINTVAR_ARRAY(pi, rstr_rssi_gain_delta_2ghh, i);
		else
			pi_lcn40->rssi_gain_delta_2ghh[i] = pi_lcn40->rssi_gain_delta_2gh[i];
	}

	for (i = 0; i < LCN40PHY_GAIN_DELTA_5G_PARAMS; i++) {
		pi_lcn40->rssi_gain_delta_5gl[i] = (int8)PHY_GETINTVAR_ARRAY_DEFAULT(pi,
			rstr_rssi_gain_delta_5gl, i, 0);
		pi_lcn40->rssi_gain_delta_5gml[i] = (int8)PHY_GETINTVAR_ARRAY_DEFAULT(pi,
			rstr_rssi_gain_delta_5gml, i, 0);
		pi_lcn40->rssi_gain_delta_5gmu[i] = (int8)PHY_GETINTVAR_ARRAY_DEFAULT(pi,
			rstr_rssi_gain_delta_5gmu, i, 0);
		pi_lcn40->rssi_gain_delta_5gh[i] = (int8)PHY_GETINTVAR_ARRAY_DEFAULT(pi,
			rstr_rssi_gain_delta_5gh, i, 0);
	}

#if defined(WLTEST)
		pi_lcn->srom_rxgainerr_2g  = (int16)PHY_GETINTVAR(pi, rstr_rxgainerr2g);
		pi_lcn->srom_noiselvl_2g  = (int8)PHY_GETINTVAR(pi, rstr_noiselvl2g);
		pi_lcn->noise_offset_2g = (int16)PHY_GETINTVAR_DEFAULT(pi, rstr_noiseoff2g, -37);
		pi_lcn->pwr_thresh_2g = (uint32)PHY_GETINTVAR_DEFAULT(pi, rstr_pwrthresh2g, 2000);

		pi_lcn->srom_rxgainerr_5gl = (int16)PHY_GETINTVAR(pi, rstr_rxgainerr5gl);
		pi_lcn->srom_rxgainerr_5gm = (int16)PHY_GETINTVAR(pi, rstr_rxgainerr5gm);
		pi_lcn->srom_rxgainerr_5gh = (int16)PHY_GETINTVAR(pi, rstr_rxgainerr5gh);
		pi_lcn->srom_noiselvl_5gl = (int8)PHY_GETINTVAR(pi, rstr_noiselvl5gl);
		pi_lcn->srom_noiselvl_5gm = (int8)PHY_GETINTVAR(pi, rstr_noiselvl5gm);
		pi_lcn->srom_noiselvl_5gh = (int8)PHY_GETINTVAR(pi, rstr_noiselvl5gh);
		pi_lcn->noise_offset_5gl = (int16)PHY_GETINTVAR_DEFAULT(pi, rstr_noiseoff5gl, -44);
		pi_lcn->noise_offset_5gm = (int16)PHY_GETINTVAR_DEFAULT(pi, rstr_noiseoff5gm, -44);
		pi_lcn->noise_offset_5gh = (int16)PHY_GETINTVAR_DEFAULT(pi, rstr_noiseoff5gh, -43);
		pi_lcn->pwr_thresh_5g = (uint32)PHY_GETINTVAR_DEFAULT(pi, rstr_pwrthresh5g, 1000);
		pi_lcn->rxpath_index = 0xFF;

		freq_corr = chan_info_2065_lcn40phy_lna_corr;
		for (i = 0; i < ARRAYSIZE(chan_info_2065_lcn40phy_lna_corr); i++) {
			snprintf(str1, sizeof(str1), rstr_rxgainfreqcorr_d,
			        chan_info_2065_lcn40phy_lna_corr[i].chan);
			freq_corr[i].corr_qdBm =
				(int8)PHY_GETINTVAR_DEFAULT(pi, str1, freq_corr[i].corr_qdBm);
		}
#endif /* #if defined(WLTEST) */

	for (i = 0; i < PWRTBL_NUM_COEFF; i++) {
		pi->txpa_2g_low_temp[i] = pi->txpa_2g[i];
		pi->txpa_2g_high_temp[i] = pi->txpa_2g[i];
	}

	wlc_phy_txpwr_sromlcn40_read_ppr_parameters(pi);

	pi_lcn->iqlocalidx_5g = (int8)PHY_GETINTVAR_DEFAULT(pi, rstr_iqlocalidx5g, -1);

	/* Params for tx power control environmental back-offs */
	pi_lcn40->high_temp_threshold = (int8)PHY_GETINTVAR(pi, rstr_tpc_temp_hi);
	pi_lcn40->temp_offs1_2g = (int8)PHY_GETINTVAR(pi, rstr_tpc_temp_offs1_2g);
	pi_lcn40->temp_offs1_5g = (int8)PHY_GETINTVAR(pi, rstr_tpc_temp_offs1_5g);
	pi_lcn40->low_temp_threshold = (int8)PHY_GETINTVAR(pi, rstr_tpc_temp_low);
	pi_lcn40->temp_offs2_2g = (int8)PHY_GETINTVAR(pi, rstr_tpc_temp_offs2_2g);
	pi_lcn40->temp_offs2_5g = (int8)PHY_GETINTVAR(pi, rstr_tpc_temp_offs2_5g);
	pi_lcn40->temp_diff = (int8)PHY_GETINTVAR_DEFAULT(pi, rstr_tpc_temp_diff, 10);

	pi_lcn40->high_vbat_threshold = (int8)PHY_GETINTVAR(pi, rstr_tpc_vbat_hi);
	pi_lcn40->vbat_offs1_2g = (int8)PHY_GETINTVAR(pi, rstr_tpc_vbat_offs1_2g);
	pi_lcn40->vbat_offs1_5g = (int8)PHY_GETINTVAR(pi, rstr_tpc_vbat_offs1_5g);
	pi_lcn40->low_vbat_threshold = (int8)PHY_GETINTVAR(pi, rstr_tpc_vbat_low);
	pi_lcn40->vbat_offs2_2g = (int8)PHY_GETINTVAR(pi, rstr_tpc_vbat_offs2_2g);
	pi_lcn40->vbat_offs2_5g = (int8)PHY_GETINTVAR(pi, rstr_tpc_vbat_offs2_5g);
	pi_lcn40->vbat_diff =	(int8)PHY_GETINTVAR_DEFAULT(pi, rstr_tpc_vbat_diff, 3);

	/* for tempcompensated tx power control */
	pi_lcn->lcnphy_rawtempsense = (uint16)PHY_GETINTVAR(pi, rstr_rawtempsense);
	pi_lcn->lcnphy_measPower = (uint8)PHY_GETINTVAR(pi, rstr_measpower);
	pi_lcn->lcnphy_measPower1 = (uint8)PHY_GETINTVAR(pi, rstr_measpower1);
	pi_lcn->lcnphy_measPower2 = (uint8)PHY_GETINTVAR(pi, rstr_measpower2);
	pi_lcn->lcnphy_tempsense_slope = (uint8)PHY_GETINTVAR(pi, rstr_tempsense_slope);
	pi_lcn->lcnphy_hw_iqcal_en = (bool)PHY_GETINTVAR(pi, rstr_hw_iqcal_en);
	pi_lcn->lcnphy_iqcal_swp_dis = (bool)PHY_GETINTVAR(pi, rstr_iqcal_swp_dis);
	pi_lcn->lcnphy_tempcorrx = (int8)PHY_GETINTVAR(pi, rstr_tempcorrx);
	pi_lcn->lcnphy_tempsense_option = (uint8)PHY_GETINTVAR(pi, rstr_tempsense_option);
	pi_lcn->lcnphy_freqoffset_corr = (uint8)PHY_GETINTVAR(pi, rstr_freqoffset_corr);
	pi->aa2g = (uint8) PHY_GETINTVAR(pi, rstr_aa2g);

	pi_lcn->extpagain2g = (uint8)PHY_GETINTVAR(pi, rstr_extpagain2g);
	pi_lcn->extpagain5g = (uint8)PHY_GETINTVAR(pi, rstr_extpagain5g);
	pi_lcn->txpwrindex_nvram = (uint8)PHY_GETINTVAR(pi, rstr_txpwrindex);
	pi_lcn->txpwrindex5g_nvram = (uint8)PHY_GETINTVAR(pi, rstr_txpwrindex5g);

	if (pi->aa2g >= 1)
		wlc_lcn40phy_decode_aa2g(pi, pi->aa2g);

	pi_lcn->lcnphy_cck_dig_filt_type =
		(int16)PHY_GETINTVAR_DEFAULT(pi, rstr_cckdigfilttype, -1);
	if (pi_lcn->lcnphy_cck_dig_filt_type < 0) {
		pi_lcn->lcnphy_cck_dig_filt_type = -1;
	}

	pi_lcn->lcnphy_ofdm_dig_filt_type_2g = -1;
	pi_lcn->lcnphy_ofdm_dig_filt_type_5g = -1;

	if (PHY_GETVAR(pi, rstr_ofdmdigfilttype)) {
		int16 temp;
		temp = (int16) PHY_GETINTVAR(pi, rstr_ofdmdigfilttype);
		if (temp >= 0) {
			pi_lcn->lcnphy_ofdm_dig_filt_type_2g  = temp;
			pi_lcn->lcnphy_ofdm_dig_filt_type_5g  = temp;
		}
	}
	if (PHY_GETVAR(pi, rstr_ofdmdigfilttype2g)) {
		int16 temp;
		temp = (int16) PHY_GETINTVAR(pi, rstr_ofdmdigfilttype2g);
		if (temp >= 0) {
			pi_lcn->lcnphy_ofdm_dig_filt_type_2g  = temp;
		}
	}
	if (PHY_GETVAR(pi, rstr_ofdmdigfilttype5g)) {
		int16 temp;
		temp = (int16) PHY_GETINTVAR(pi, rstr_ofdmdigfilttype5g);
		if (temp >= 0) {
			pi_lcn->lcnphy_ofdm_dig_filt_type_5g  = temp;
		}
	}

	pi_lcn40->ofdm40_dig_filt_type_2g = -1;
	pi_lcn40->ofdm40_dig_filt_type_5g = -1;

	if (PHY_GETVAR(pi, rstr_ofdm40digfilttype)) {
		int16 temp;
		temp = (int16) PHY_GETINTVAR(pi, rstr_ofdm40digfilttype);
		if (temp >= 0) {
			pi_lcn40->ofdm40_dig_filt_type_2g  = temp;
			pi_lcn40->ofdm40_dig_filt_type_5g  = temp;
		}
	}
	if (PHY_GETVAR(pi, rstr_ofdm40digfilttype2g)) {
		int16 temp;
		temp = (int16) PHY_GETINTVAR(pi, rstr_ofdm40digfilttype2g);
		if (temp >= 0) {
			pi_lcn40->ofdm40_dig_filt_type_2g  = temp;
		}
	}
	if (PHY_GETVAR(pi, rstr_ofdm40digfilttype5g)) {
		int16 temp;
		temp = (int16) PHY_GETINTVAR(pi, rstr_ofdm40digfilttype5g);
		if (temp >= 0) {
			pi_lcn40->ofdm40_dig_filt_type_5g  = temp;
		}
	}

	/* pa gain override */
	pi_lcn->pa_gain_ovr_val_2g = (int16)PHY_GETINTVAR_DEFAULT(pi, rstr_pagc2g, -1);
	pi_lcn->pa_gain_ovr_val_5g = (int16)PHY_GETINTVAR_DEFAULT(pi, rstr_pagc5g, -1);
	pi_lcn->lcnphy_tx_iqlo_tone_freq_ovr_val = (int16)PHY_GETINTVAR(pi, rstr_txiqlotf);

	/* Coefficients for Temperature Conversion to Centigrade */
	if (LCN40REV_IS(pi->pubpi.phy_rev, 6)) {
		/* 0.76793608<<8 */
		pi_lcn->temp_mult = (int32)PHY_GETINTVAR_DEFAULT(pi, rstr_temp_mult, 197);
		/* 108.624792<<8 */
		pi_lcn->temp_add = (int32)PHY_GETINTVAR_DEFAULT(pi, rstr_temp_add, 27808);
	} else {
		/* 0.7367<<8 */
		pi_lcn->temp_mult = (int32)PHY_GETINTVAR_DEFAULT(pi, rstr_temp_mult, 189);
		/* 104.83<<8 */
		pi_lcn->temp_add = (int32)PHY_GETINTVAR_DEFAULT(pi, rstr_temp_add, 26836);
	}
	pi_lcn->temp_q = (int32)PHY_GETINTVAR_DEFAULT(pi, rstr_temp_q, 8);

	/* Coefficients for vbat conversion to Volts */
	pi_lcn->vbat_mult = (int32)PHY_GETINTVAR_DEFAULT(pi, rstr_vbat_mult, -9); /* -.009<<10 */
	pi_lcn->vbat_add = (int32)PHY_GETINTVAR_DEFAULT(pi, rstr_vbat_add, 4027); /* 3.932<<10 */
	pi_lcn->vbat_q = (int32)PHY_GETINTVAR_DEFAULT(pi, rstr_vbat_q, 10);

	/* Offset for the CCK power detector */
	if (PHY_GETVAR(pi, rstr_cckPwrOffset)) {
		pi_lcn->cckPwrOffset = (int16)PHY_GETINTVAR(pi, rstr_cckPwrOffset);
	} else {
		if (boardtype == 0x5e0 && boardrev >= 0x1203)
			pi_lcn->cckPwrOffset = 16;
		else
			pi_lcn->cckPwrOffset = 10;

		PHY_INFORM(("Use driver defaults %d for cckPwrOffset.\n", pi_lcn->cckPwrOffset));
	}

	/* CCK Power Index Correction */
	pi_lcn->cckPwrIdxCorr = (int16)PHY_GETINTVAR(pi, rstr_cckPwrIdxCorr);
#if TWO_POWER_RANGE_TXPWR_CTRL
		if ((pi_lcn->lcnphy_twopwr_txpwrctrl_en) && (pi_lcn->cckPwrIdxCorr > 0))
			pi_lcn->cckPwrIdxCorr = 0;
#endif // endif

	if (CHIPID(pi->sh->chip) == BCM43142_CHIP_ID)
		pi_lcn->cckPwrIdxCorr = -28;

	/* DAC rate */
	if (LCN40REV_IS(pi->pubpi.phy_rev, 6))
		pi_lcn40->dac2x_enable = (uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_dacrate2xen, 1);
	else if (LCN40REV_GE(pi->pubpi.phy_rev, 4))
		pi_lcn40->dac2x_enable =
		(uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_dacrate2xen, 0);
	pi_lcn40->dac2x_enable_nvm = pi_lcn40->dac2x_enable;

	/* DAC PU  */
	pi_lcn40->dacpu = (uint8)PHY_GETINTVAR(pi, rstr_dacpu);
	/* rfreg033 value */
	pi_lcn->rfreg033 = (uint8)PHY_GETINTVAR(pi, rstr_rfreg033);
	pi_lcn->rfreg033_cck = (uint8)PHY_GETINTVAR(pi, rstr_rfreg033_cck);

	/* PA Cal Idx 2g */
	pi_lcn->pacalidx_2g = (uint8)PHY_GETINTVAR(pi, rstr_pacalidx2g);

	/* PA Cal Idx 5g */
	pi_lcn->pacalidx_5g = (uint8)PHY_GETINTVAR(pi, rstr_pacalidx5g);
	pi_lcn->papd_rf_pwr_scale = (uint8)PHY_GETINTVAR(pi, rstr_parfps);
	if (pi_lcn->papd_rf_pwr_scale == 0)
		pi_lcn->papd_rf_pwr_scale_2g = (uint8)PHY_GETINTVAR(pi, rstr_papdsfac2g);
	pi_lcn->papd_rf_pwr_scale_5g = (uint8)PHY_GETINTVAR(pi, rstr_papdsfac5g);

	/* For 4314/43142 PCIE, the swctrlmap_2g come from here. For SDIO, it should be in nvram */
	if ((CHIPID(pi->sh->chip) == BCM4314_CHIP_ID || CHIPID(pi->sh->chip) == BCM43142_CHIP_ID) &&
		BUSTYPE(pi->sh->bustype) == PCI_BUS) {
		if (PHY_GETVAR(pi, rstr_swctrlmap_2g)) {
			for (i = 0; i < LCN40PHY_SWCTRL_NVRAM_PARAMS; i++) {
				pi_lcn->swctrlmap_2g[i] =
					(uint32) PHY_GETINTVAR_ARRAY(pi, rstr_swctrlmap_2g, i);
			}
			/* this code is here in case SROM is outdated, should remove later */
			for (i = 1; i <= 2; i++)
				pi_lcn->swctrlmap_2g[i] = (pi_lcn->swctrlmap_2g[i] & 0xffff) |
					((pi_lcn->swctrlmap_2g[i] & 0xff) << 24) |
					((pi_lcn->swctrlmap_2g[i] & 0xff00) << 8);

		} else {
			/* XXX temp hack for 43142 bringup. Need to remove when SROM map is
			 * finalized and provides swctrlmap_2g.
			 */
			PHY_ERROR((" No swctrlmap_2g found. Use driver defaults.\n"));

			if (PHY_GETINTVAR(pi, rstr_extpagain2g)) {
				pi_lcn->swctrlmap_2g[LCN40PHY_I_WL_TX] = 0x06020602;
				pi_lcn->swctrlmap_2g[LCN40PHY_I_WL_RX] = 0x0c080c08;
				pi_lcn->swctrlmap_2g[LCN40PHY_I_WL_RX_ATTN] = 0x04000400;
				pi_lcn->swctrlmap_2g[LCN40PHY_I_BT] = 0x00080808;
				pi_lcn->swctrlmap_2g[LCN40PHY_I_WL_MASK] = 0x6ff;
			} else { /* iPA */
				if (boardtype == 0x5e0) { /* bcm943142hm */
					pi_lcn->swctrlmap_2g[LCN40PHY_I_WL_TX] = 0x04000400;
					pi_lcn->swctrlmap_2g[LCN40PHY_I_WL_RX] = 0x0c080c08;
					pi_lcn->swctrlmap_2g[LCN40PHY_I_WL_RX_ATTN] = 0x04000400;
					pi_lcn->swctrlmap_2g[LCN40PHY_I_BT] = 0x00080808;
					pi_lcn->swctrlmap_2g[LCN40PHY_I_WL_MASK] = 0x6ff;
				} else if (boardtype == 0x5d1 && boardrev >= 0x1200 &&
					boardrev <= 0x1250) {	/* bcm94314hm, p200-p250 */
					pi_lcn->swctrlmap_2g[LCN40PHY_I_WL_TX] = 0x06020602;
					pi_lcn->swctrlmap_2g[LCN40PHY_I_WL_RX] = 0x0c080c08;
					pi_lcn->swctrlmap_2g[LCN40PHY_I_WL_RX_ATTN] = 0x04000400;
					pi_lcn->swctrlmap_2g[LCN40PHY_I_BT] = 0x00080808;
					pi_lcn->swctrlmap_2g[LCN40PHY_I_WL_MASK] = 0x6ff;
				} else if (boardtype == 0x5d1 && boardrev >= 0x1251 &&
					boardrev < 0x1300) {	/* bcm94314hm, p251+ */
					pi_lcn->swctrlmap_2g[LCN40PHY_I_WL_TX] = 0x04000400;
					pi_lcn->swctrlmap_2g[LCN40PHY_I_WL_RX] = 0x0c080c08;
					pi_lcn->swctrlmap_2g[LCN40PHY_I_WL_RX_ATTN] = 0x04000400;
					pi_lcn->swctrlmap_2g[LCN40PHY_I_BT] = 0x00080808;
					pi_lcn->swctrlmap_2g[LCN40PHY_I_WL_MASK] = 0x6ff;
				} else if (boardtype == 0x5d1 && boardrev >= 0x1300) {
					pi_lcn->swctrlmap_2g[LCN40PHY_I_WL_TX] = 0x090a090a;
					pi_lcn->swctrlmap_2g[LCN40PHY_I_WL_RX] = 0x05060506;
					pi_lcn->swctrlmap_2g[LCN40PHY_I_WL_RX_ATTN] = 0x090a090a;
					pi_lcn->swctrlmap_2g[LCN40PHY_I_BT] = 0x00060606;
					pi_lcn->swctrlmap_2g[LCN40PHY_I_WL_MASK] = 0x6ff;
				} else {
					pi_lcn->swctrlmap_2g[LCN40PHY_I_WL_TX] = 0x090a090a;
					pi_lcn->swctrlmap_2g[LCN40PHY_I_WL_RX] = 0x05060506;
					pi_lcn->swctrlmap_2g[LCN40PHY_I_WL_RX_ATTN] = 0x090a090a;
					pi_lcn->swctrlmap_2g[LCN40PHY_I_BT] = 0x00060606;
					pi_lcn->swctrlmap_2g[LCN40PHY_I_WL_MASK] = 0x6ff;
				}
			}

			PHY_INFORM(("swctrlmap_2g=0x%x,0x%x,0x%x,0x%x,0x%x\n",
				pi_lcn->swctrlmap_2g[LCN40PHY_I_WL_TX],
				pi_lcn->swctrlmap_2g[LCN40PHY_I_WL_RX],
				pi_lcn->swctrlmap_2g[LCN40PHY_I_WL_RX_ATTN],
				pi_lcn->swctrlmap_2g[LCN40PHY_I_BT],
				pi_lcn->swctrlmap_2g[LCN40PHY_I_WL_MASK]));
		}
	} else {
		if (PHY_GETVAR(pi, rstr_swctrlmap_2g)) {
			for (i = 0; i < LCN40PHY_SWCTRL_NVRAM_PARAMS; i++) {
				pi_lcn->swctrlmap_2g[i] =
					(uint32) PHY_GETINTVAR_ARRAY(pi, rstr_swctrlmap_2g, i);
			}
		} else {
			PHY_ERROR((" Switch control map(swctrlmap_2g) is NOT found"
				"in the NVRAM file %s \n", __FUNCTION__));
			return FALSE;
		}

		if (PHY_GETVAR(pi, "swctrlmapext_2g")) {
			for (i = 0; i < LCN40PHY_SWCTRL_NVRAM_PARAMS; i++) {
				pi_lcn->swctrlmapext_2g[i] =
					(uint32) PHY_GETINTVAR_ARRAY(pi, "swctrlmapext_2g", i);
			}
		}
#ifdef BAND5G
		if (PHY_GETVAR(pi, rstr_swctrlmap_5g)) {
			for (i = 0; i < LCN40PHY_SWCTRL_NVRAM_PARAMS; i++) {
				pi_lcn->swctrlmap_5g[i] =
					(uint32) PHY_GETINTVAR_ARRAY(pi, rstr_swctrlmap_5g, i);
			}
		} else {
			PHY_ERROR((" Switch control map(swctrlmap_5g) is NOT found"
				"in the NVRAM file %s \n", __FUNCTION__));
			return FALSE;
		}

		if (PHY_GETVAR(pi, "swctrlmapext_5g")) {
			for (i = 0; i < LCN40PHY_SWCTRL_NVRAM_PARAMS; i++) {
				pi_lcn->swctrlmapext_5g[i] =
					(uint32) PHY_GETINTVAR_ARRAY(pi, "swctrlmapext_5g", i);
			}
		}
#endif /* BAND5G */

		pi_lcn40->paprr_enable2g = (uint8)PHY_GETINTVAR(pi, rstr_paprr_enable2g);
		pi_lcn40->paprr_enable5g = (uint8)PHY_GETINTVAR(pi, rstr_paprr_enable5g);

		/* read PAPR tables */
		for (i = 0; i < LCN40PHY_PAPR_NVRAM_PARAMS; i++) {
		  pi_lcn40->paprgaintbl2g[i] = paprgaintbl2g_t[i];
		  pi_lcn40->paprgamtbl2g[i]  = paprgamtbl2g_t[i];
		  pi_lcn40->paprofftbl2g[i]  = paprofftbl2g_t[i];
		}
#ifdef BAND5G
		for (i = 0; i < LCN40PHY_PAPR_NVRAM_PARAMS; i++) {
		  pi_lcn40->paprgaintbl5g[i] = paprgaintbl5g_t[i];
		  pi_lcn40->paprgamtbl5g[i]  = paprgamtbl5g_t[i];
		  pi_lcn40->paprofftbl5g[i]  = paprofftbl5g_t[i];
		}
		for (i = 0; i < LCN40PHY_PAPR_NVRAM_PARAMS; i++) {
		  pi_lcn40->papr40gaintbl5g[i] = papr40gaintbl5g_t[i];
		  pi_lcn40->papr40gamtbl5g[i]  = papr40gamtbl5g_t[i];
		  pi_lcn40->papr40offtbl5g[i]  = papr40offtbl5g_t[i];
		}
#endif /* BAND5G */

	}

#ifdef BAND5G
	pi_lcn->dacgc5g = (int16)PHY_GETINTVAR_DEFAULT(pi, rstr_dacgc5g, -1);
	pi_lcn->gmgc5g = (int16)PHY_GETINTVAR_DEFAULT(pi, rstr_gmgc5g, -1);

	if (PHY_GETVAR(pi, rstr_pa1lob0) &&
	    PHY_GETVAR(pi, rstr_pa1lob1) &&
	    PHY_GETVAR(pi, rstr_pa1lob2)) {
		pi->txpa_5g_low[0] = (int16)PHY_GETINTVAR(pi, rstr_pa1lob0);
		pi->txpa_5g_low[1] = (int16)PHY_GETINTVAR(pi, rstr_pa1lob1);
		pi->txpa_5g_low[2] = (int16)PHY_GETINTVAR(pi, rstr_pa1lob2);
	} else {
		pi->txpa_5g_low[0] = -1;
		pi->txpa_5g_low[1] = -1;
		pi->txpa_5g_low[2] = -1;
	}

	if (PHY_GETVAR(pi, rstr_pa1b0) &&
	    PHY_GETVAR(pi, rstr_pa1b1) &&
	    PHY_GETVAR(pi, rstr_pa1b2)) {
		pi->txpa_5g_mid[0] = (int16)PHY_GETINTVAR(pi, rstr_pa1b0);
		pi->txpa_5g_mid[1] = (int16)PHY_GETINTVAR(pi, rstr_pa1b1);
		pi->txpa_5g_mid[2] = (int16)PHY_GETINTVAR(pi, rstr_pa1b2);
	} else {
		pi->txpa_5g_mid[0] = -1;
		pi->txpa_5g_mid[1] = -1;
		pi->txpa_5g_mid[2] = -1;
	}

	if (PHY_GETVAR(pi, rstr_pa1hib0) &&
	    PHY_GETVAR(pi, rstr_pa1hib1) &&
	    PHY_GETVAR(pi, rstr_pa1hib2)) {
		pi->txpa_5g_hi[0] = (int16)PHY_GETINTVAR(pi, rstr_pa1hib0);
		pi->txpa_5g_hi[1] = (int16)PHY_GETINTVAR(pi, rstr_pa1hib1);
		pi->txpa_5g_hi[2] = (int16)PHY_GETINTVAR(pi, rstr_pa1hib2);
	} else {
		pi->txpa_5g_hi[0] = -1;
		pi->txpa_5g_hi[1] = -1;
		pi->txpa_5g_hi[2] = -1;
	}

	pi_lcn->rssismf5g = (int16)PHY_GETINTVAR_DEFAULT(pi, rstr_rssismf5g, -1);
	pi_lcn->rssismc5g = (int16)PHY_GETINTVAR_DEFAULT(pi, rstr_rssismc5g, -1);
	pi_lcn->rssisav5g = (int16)PHY_GETINTVAR_DEFAULT(pi, rstr_rssisav5g, -1);
	pi_lcn->init_txpwrindex_5g = (uint32)PHY_GETINTVAR_DEFAULT(pi, rstr_initxidx5g,
		LCN40PHY_TX_PWR_CTRL_START_INDEX_5G);

	if (PHY_GETVAR(pi, rstr_rx_iq_comp_5g)) {
		for (i = 0; i < LCN40PHY_RXIQCOMP_PARAMS; i++) {
			pi_lcn40->rx_iq_comp_5g[i] = (uint16)
				getintvararray(pi->vars, rstr_rx_iq_comp_5g, i);
		}
	}
#endif /* BAND5G */
	if (PHY_GETVAR(pi, rstr_rx_iq_comp_2g)) {
		for (i = 0; i < LCN40PHY_RXIQCOMP_PARAMS; i++) {
			pi_lcn40->rx_iq_comp_2g[i] = (uint16)
				getintvararray(pi->vars, rstr_rx_iq_comp_2g, i);
		}
	}
	pi_lcn->dacgc2g = (int16)PHY_GETINTVAR_DEFAULT(pi, rstr_dacgc2g, -1);
	pi_lcn->gmgc2g = (int16)PHY_GETINTVAR_DEFAULT(pi, rstr_gmgc2g, -1);

	/* ofdm analog filt bw */
	pi->ofdm_analog_filt_bw_override = (int16)PHY_GETINTVAR_DEFAULT(pi,
	                                                                rstr_ofdmanalogfiltbw, -1);

	if (LCN40REV_IS(pi->pubpi.phy_rev, 6)) {
		pi->tx_alpf_bypass = 1;
		pi_lcn40->tx_alpf_pu = 1;
	}
	if (LCN40REV_GE(pi->pubpi.phy_rev, 4) && PHY_GETVAR(pi, rstr_txalpfbyp)) {
		pi->tx_alpf_bypass = (int16)PHY_GETINTVAR(pi, rstr_txalpfbyp);
	}

	if (LCN40REV_GE(pi->pubpi.phy_rev, 4) && PHY_GETVAR(pi, rstr_txalpfpu)) {
		pi_lcn40->tx_alpf_pu = (bool)PHY_GETINTVAR(pi, rstr_txalpfpu);
	}

	if (PHY_GETVAR(pi, rstr_bphyscale)) {
		pi->bphy_scale = (int16)PHY_GETINTVAR(pi, rstr_bphyscale);
	}

	pi_lcn->noise.nvram_enable_2g =
		(bool)PHY_GETINTVAR_DEFAULT(pi, rstr_noise_cal_enable_2g, FALSE);

	pi_lcn->noise.nvram_enable_5g =
		(bool)PHY_GETINTVAR_DEFAULT(pi, rstr_noise_cal_enable_5g, FALSE);

	pi_lcn->noise.nvram_ref_2g = (int8)PHY_GETINTVAR_DEFAULT(pi, rstr_noise_cal_ref_2g, -1);
	pi_lcn->noise.nvram_ref_5g = (int8)PHY_GETINTVAR_DEFAULT(pi, rstr_noise_cal_ref_5g, -1);
	pi_lcn->noise.nvram_ref_40_2g = (int8)PHY_GETINTVAR_DEFAULT(pi,
		rstr_noise_cal_ref_40_2g, -1);
	pi_lcn->noise.nvram_ref_40_5g = (int8)PHY_GETINTVAR_DEFAULT(pi,
		rstr_noise_cal_ref_40_5g, -1);
	if ((pi_lcn->noise.nvram_ref_40_2g < 0) && (pi_lcn->noise.nvram_ref_2g > 0))
		pi_lcn->noise.nvram_ref_40_2g = pi_lcn->noise.nvram_ref_2g + 3;
	if ((pi_lcn->noise.nvram_ref_40_5g < 0) && (pi_lcn->noise.nvram_ref_5g > 0))
		pi_lcn->noise.nvram_ref_40_5g = pi_lcn->noise.nvram_ref_5g + 3;

	pi_lcn->noise.nvram_input_pwr_offset_2g =
		(int16)PHY_GETINTVAR_DEFAULT(pi, rstr_noise_cal_po_2g, 0xff);
	pi_lcn->noise.nvram_input_pwr_offset_40_2g =
		(int16)PHY_GETINTVAR_DEFAULT(pi, rstr_noise_cal_po_40_2g, 0xff);

	for (i = 0; i < 3; i++) {
	  pi_lcn->noise.nvram_input_pwr_offset_5g[i] = 0xff;
	  pi_lcn->noise.nvram_input_pwr_offset_40_5g[i] = 0xff;
	}

	for (i = 0; i < 2; i++) {
	  const char* po_str;
	  int16* po_data;

	  if (i == 0) {
	    po_str = rstr_noise_cal_po_5g;
	    po_data = pi_lcn->noise.nvram_input_pwr_offset_5g;
	  } else {
	    po_str = rstr_noise_cal_po_40_5g;
	    po_data = pi_lcn->noise.nvram_input_pwr_offset_40_5g;
	  }

	  if (PHY_GETVAR(pi, po_str)) {
	    int k = getintvararraysize(pi->vars, po_str);
	    int j = 0;
	    if (k > 3)
	      k = 3;
	    while (j < k) {
	      po_data[j] =
	       (int16) PHY_GETINTVAR_ARRAY(pi, po_str, j);
	      j++;
	    }
	    while (j < 3) {
	      po_data[j++] = po_data[0];
	    }
	  }
	}

	pi_lcn->noise.nvram_gain_tbl_adj_2g =
		(int8)PHY_GETINTVAR_DEFAULT(pi, rstr_noise_cal_adj_2g, 18);
	pi_lcn->noise.nvram_gain_tbl_adj_5g =
		(int8)PHY_GETINTVAR_DEFAULT(pi, rstr_noise_cal_adj_5g, 18);

	pi_lcn->noise.nvram_high_gain_2g =
	  (int)PHY_GETINTVAR_DEFAULT(pi, rstr_noise_cal_high_gain_2g, 70);
	pi_lcn->noise.nvram_high_gain_5g =
	  (int)PHY_GETINTVAR_DEFAULT(pi, rstr_noise_cal_high_gain_5g, 70);

	pi_lcn->noise.nvram_nf_substract_val_2g =
	   (int)PHY_GETINTVAR_DEFAULT(pi, rstr_noise_cal_nf_substract_val_2g, -1);
	pi_lcn->noise.nvram_nf_substract_val_5g =
	   (int)PHY_GETINTVAR_DEFAULT(pi, rstr_noise_cal_nf_substract_val_5g, -1);

	if (PHY_GETVAR(pi, rstr_noise_cal_update) &&
	    (getintvararraysize(pi->vars, rstr_noise_cal_update) ==
	     (3*k_noise_cal_update_steps))) {
	  for (i = 0; i < (k_noise_cal_update_steps); i++) {
	    pi_lcn->noise.update_ucode_interval[i] =
	      (uint8) getintvararray(pi->vars,
	      rstr_noise_cal_update, 3*i);
	    pi_lcn->noise.update_data_interval[i] =
	      (uint8) getintvararray(pi->vars,
	      rstr_noise_cal_update, (3*i+1));
	    pi_lcn->noise.update_step_interval[i] =
	      (uint8) getintvararray(pi->vars,
	      rstr_noise_cal_update, (3*i+2));
	  }
	} else {
	  pi_lcn->noise.update_ucode_interval[0] = 1;
	  pi_lcn->noise.update_ucode_interval[1] = 50;
	  pi_lcn->noise.update_data_interval[0] = 10;
	  pi_lcn->noise.update_data_interval[1] = 10;
	  pi_lcn->noise.update_step_interval[0] = 255;
	  pi_lcn->noise.update_step_interval[1] = 255;
	}

	/* Below is the rx gain change rate per degree C multiplied by 1000 */
	pi_lcn->rxgain_tempcorr_2g = (uint16)PHY_GETINTVAR(pi, rstr_rxgaintempcorr2g);
	pi_lcn->rxgain_tempcorr_5gh = (uint16)PHY_GETINTVAR(pi, rstr_rxgaintempcorr5gh);
	pi_lcn->rxgain_tempcorr_5gm = (uint16)PHY_GETINTVAR(pi, rstr_rxgaintempcorr5gm);
	pi_lcn->rxgain_tempcorr_5gl = (uint16)PHY_GETINTVAR(pi, rstr_rxgaintempcorr5gl);

	if (PHY_GETVAR(pi, rstr_xtalmode)) {
		pi_lcn->xtal_mode[0] = 1;
		pi_lcn->xtal_mode[1] =
			(uint8) PHY_GETINTVAR_ARRAY(pi, rstr_xtalmode, 0);
		pi_lcn->xtal_mode[2] =
			(uint8) PHY_GETINTVAR_ARRAY(pi, rstr_xtalmode, 1);
		pi_lcn->xtal_mode[3] =
			(uint8) PHY_GETINTVAR_ARRAY(pi, rstr_xtalmode, 2);
	} else {
		pi_lcn->xtal_mode[0] = 0;
	}
	if (PHY_GETVAR(pi, rstr_txiqlopapu2g))
		pi_lcn->txiqlopapu_2g = (bool)PHY_GETINTVAR(pi, rstr_txiqlopapu2g);
	else
		pi_lcn->txiqlopapu_2g = -1;
#ifdef BAND5G
	if (PHY_GETVAR(pi, rstr_txiqlopapu5g))
		pi_lcn->txiqlopapu_5g = (bool)PHY_GETINTVAR(pi, rstr_txiqlopapu5g);
	else
		pi_lcn->txiqlopapu_5g = -1;

	pi_lcn->triso5g[0] = (uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_triso5g, 0xff);
#endif /* #ifdef BAND5G */
	pi_lcn40->pwr_offset40mhz_2g = (int16)PHY_GETINTVAR_DEFAULT(pi, rstr_PwrOffset40mhz2g, 0);
	pi_lcn40->pwr_offset40mhz_5g = (int16)PHY_GETINTVAR_DEFAULT(pi, rstr_PwrOffset40mhz5g, 0);
#ifdef LP_P2P_SOFTAP
	pi_lcn->pwr_offset_val = (int16)PHY_GETINTVAR_DEFAULT(pi, rstr_lpoff, 0);
#endif // endif
	pi_lcn->offset_targetpwr = (uint16)PHY_GETINTVAR_DEFAULT(pi, rstr_offtgpwr, 0);
#ifdef WLTEST
	pi->boostackpwr = (uint8) PHY_GETINTVAR_DEFAULT(pi, rstr_boostackpwr, 0);
#endif /* #ifdef WLTEST */
	pi_lcn40->mixboost_5g = (uint16)PHY_GETINTVAR_DEFAULT(pi, rstr_mixboost5g, 0);
	pi_lcn40->gain_settle_dly_2g = (uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_gain_settle_dly_2g, 3);
	pi_lcn40->gain_settle_dly_5g = (uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_gain_settle_dly_5g, 3);
	pi_lcn40->fstr_flag = (uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_fstr, 0);
	pi_lcn40->cdd_mod = (uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_cddmod, 0);

	pi_lcn40->tia_dc_loop_enable_2g =
		(uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_tia_dc_loop_enable_2g, 0);
	pi_lcn40->hpc_sequencer_enable_2g =
		(uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_hpc_sequencer_enable_2g, 1);
#ifdef BAND5G
	pi_lcn40->tia_dc_loop_enable_5g =
		(uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_tia_dc_loop_enable_5g, 1);
	pi_lcn40->hpc_sequencer_enable_5g =
		(uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_hpc_sequencer_enable_5g, 1);
#endif // endif
	pi_lcn40->padbias5g = (int16)PHY_GETINTVAR_DEFAULT(pi, rstr_padbias5g, -1);

	pi_lcn40->aci_detect_en_2g = (int8)PHY_GETINTVAR_DEFAULT(pi, rstr_aci_detect_en_2g, -1);
	pi_lcn40->tx_agc_reset_2g = (int8)PHY_GETINTVAR_DEFAULT(pi, rstr_tx_agc_reset_2g, 0);
	pi_lcn40->gaintbl_force_2g = (int8)PHY_GETINTVAR_DEFAULT(pi, rstr_gaintbl_force_2g, -1);
	pi_lcn40->gaintbl_presel_2g = (int8)PHY_GETINTVAR_DEFAULT(pi, rstr_gaintbl_presel_2g, -1);
	pi_lcn40->aci_detect_en_5g = (int8)PHY_GETINTVAR_DEFAULT(pi, rstr_aci_detect_en_5g, -1);
	pi_lcn40->tx_agc_reset_5g = (int8)PHY_GETINTVAR_DEFAULT(pi, rstr_tx_agc_reset_5g, 0);
	pi_lcn40->gaintbl_force_5g = (int8)PHY_GETINTVAR_DEFAULT(pi, rstr_gaintbl_force_5g, -1);
	pi_lcn40->gaintbl_presel_5g = (int8)PHY_GETINTVAR_DEFAULT(pi, rstr_gaintbl_presel_5g, -1);

	pi_lcn40->iqcalidx5g = (int)PHY_GETINTVAR_DEFAULT(pi, rstr_iqcalidx5g, -1);

	pi_lcn40->epa_or_pad_lpbk = (bool)PHY_GETINTVAR_DEFAULT(pi, rstr_lpbckmode5g, 0);
	pi_lcn40->loflag = (bool)PHY_GETINTVAR_DEFAULT(pi, rstr_loflag, 0);
	pi_lcn40->dlocalidx5g = (int8)PHY_GETINTVAR_DEFAULT(pi, rstr_dlocalidx5g, -1);
	pi_lcn40->dlorange_lowlimit = (int8)PHY_GETINTVAR_DEFAULT(pi, rstr_dlorange_lowlimit, -128);

	pi_lcn40->noise_cal_deltamax = (int32)PHY_GETINTVAR_DEFAULT(pi, rstr_noise_cal_deltamax, 0);
	pi_lcn40->noise_cal_deltamin =
		(int32)PHY_GETINTVAR_DEFAULT(pi, rstr_noise_cal_deltamin, -3);

	pi_lcn40->dsss_thresh = (int8)PHY_GETINTVAR_DEFAULT(pi, rstr_dsss_thresh, 99);

	pi_lcn40->startdiq_2g = (uint32)PHY_GETINTVAR_DEFAULT(pi, rstr_startdiq2g, 0x0a0a);
	pi_lcn40->startdiq_5g = (uint32)PHY_GETINTVAR_DEFAULT(pi, rstr_startdiq5g, 0x1414);
	pi_lcn40->init_ccktxpwrindex = (uint16)PHY_GETINTVAR_DEFAULT(pi, rstr_cckinitxidx,
		LCN40PHY_TX_PWR_CTRL_START_INDEX_2G);

	for (ch = 0; ch < 14; ch++)
		for (i = 0; i < LCN40PHY_NOTCHFILTER_COEFFS; i++)
			spurblk_phy_reg_cfg[ch][i] =
			(int8) PHY_GETINTVAR_ARRAY_DEFAULT(pi, rstr_notch[ch], i, 0);

	pi_lcn40->localoffs5gmh = (int8)PHY_GETINTVAR_DEFAULT(pi, rstr_localoffs5gmh, 0);

	pi_lcn40->temp_offset_2g = (int16)PHY_GETINTVAR_DEFAULT(pi, rstr_temp_offset_2g, 0);
	pi_lcn40->temp_offset_5g = (int16)PHY_GETINTVAR_DEFAULT(pi, rstr_temp_offset_5g, 0);
	pi_lcn40->pretemp = 25;
	pi_lcn40->temp_cal_en_2g = (int8)PHY_GETINTVAR_DEFAULT(pi, rstr_temp_cal_en_2g, 1);
	pi_lcn40->temp_cal_en_5g = (int8)PHY_GETINTVAR_DEFAULT(pi, rstr_temp_cal_en_5g, 0);
	pi_lcn40->vlin2g = (int8)PHY_GETINTVAR_DEFAULT(pi, rstr_vlin2g, -1);
	pi_lcn40->vlin5g = (int8)PHY_GETINTVAR_DEFAULT(pi, rstr_vlin5g, -1);
	pi_lcn40->edonthreshold40 = (int16)PHY_GETINTVAR_DEFAULT(pi, rstr_edonthd40, -65);
	pi_lcn40->edoffthreshold40 = (int16)PHY_GETINTVAR_DEFAULT(pi, rstr_edoffthd40, -71);
	pi_lcn40->edonthreshold20U = (int16)PHY_GETINTVAR_DEFAULT(pi, rstr_edonthd20u, -65);
	pi_lcn40->edonthreshold20L = (int16)PHY_GETINTVAR_DEFAULT(pi, rstr_edonthd20l, -65);
	pi_lcn40->edoffthreshold20UL = (int16)PHY_GETINTVAR_DEFAULT(pi, rstr_edoffthd20ul, -71);

	pi_lcn40->papden2g = (int8)PHY_GETINTVAR_DEFAULT(pi, rstr_papden2g, -1);
	pi_lcn40->papden5g = (int8)PHY_GETINTVAR_DEFAULT(pi, rstr_papden5g, -1);
	pi_lcn40->papdlinpath2g = (int16)PHY_GETINTVAR_DEFAULT(pi, rstr_papdlinpath2g, -1);
	pi_lcn40->papdlinpath5g = (int16)PHY_GETINTVAR_DEFAULT(pi, rstr_papdlinpath5g, -1);
	pi_lcn40->papd_mag_th_2g = (int16)PHY_GETINTVAR_DEFAULT(pi, rstr_papdmagth2g, -1);
	pi_lcn40->papd_mag_th_5g = (int16)PHY_GETINTVAR_DEFAULT(pi, rstr_papdmagth5g, -1);
	if (CHIPID(pi->sh->chip) == BCM4314_CHIP_ID || CHIPID(pi->sh->chip) == BCM43142_CHIP_ID) {
		pi_lcn40->pll_loop_bw_desired_2g = PLL_2065_LOOP_BW_43142;
	} else if (CHIPID(pi->sh->chip) == BCM43143_CHIP_ID) {
		pi_lcn40->pll_loop_bw_desired_2g = PLL_2065_LOOP_BW_43143;
	} else {
		pi_lcn40->pll_loop_bw_desired_2g = PLL_2065_LOOP_BW_DESIRED_2G;
	}
	pi_lcn40->rfpll_doubler_2g =
		(bool) PHY_GETINTVAR_DEFAULT(pi, rstr_plldoubler_enable2g, FALSE);
	pi_lcn40->pll_loop_bw_desired_2g =
		(uint16) PHY_GETINTVAR_DEFAULT(pi, rstr_loopbw2g, pi_lcn40->pll_loop_bw_desired_2g);
#ifdef BAND5G
	pi_lcn40->rfpll_doubler_5g =
		(bool) PHY_GETINTVAR_DEFAULT(pi, rstr_plldoubler_enable5g, FALSE);
	pi_lcn40->pll_loop_bw_desired_5g =
		(uint16) PHY_GETINTVAR_DEFAULT(pi, rstr_loopbw5g, PLL_2065_LOOP_BW_DESIRED_5G);
#endif // endif
	pi_lcn40->temp_cal_adj_2g =
		(uint32) PHY_GETINTVAR_DEFAULT(pi, rstr_tempcaladj2g, 0x03fefb);
	pi_lcn40->temp_cal_adj_5g =
		(uint32) PHY_GETINTVAR_DEFAULT(pi, rstr_tempcaladj5g, 0x03fefb);

	pi_lcn40->papden2gchan =
		(int32) PHY_GETINTVAR_DEFAULT(pi, rstr_papden2gchan, -1);

	pi_lcn40->calidxestbase2g =
		(int32) PHY_GETINTVAR_DEFAULT(pi, rstr_calidxestbase2g, 96);
	pi_lcn40->calidxesttarget2g =
		(int32) PHY_GETINTVAR_DEFAULT(pi, rstr_calidxesttarget2g, -1);
	pi_lcn40->calidxestbase5g =
		(int32) PHY_GETINTVAR_DEFAULT(pi, rstr_calidxestbase5g, 96);
	pi_lcn40->calidxesttarget5g =
		(int32) PHY_GETINTVAR_DEFAULT(pi, rstr_calidxesttarget5g, -1);
	pi_lcn40->calidxesttarget405g =
		(int32) PHY_GETINTVAR_DEFAULT(pi, rstr_calidxesttarget405g, -1);
	pi_lcn40->calidxesttargetlo5g =
		(int32) PHY_GETINTVAR_DEFAULT(pi, rstr_calidxesttargetlo5g, -1);
	pi_lcn40->calidxesttarget40lo5g =
		(int32) PHY_GETINTVAR_DEFAULT(pi, rstr_calidxesttarget40lo5g, -1);
	pi_lcn40->calidxesttargethi5g =
		(int32) PHY_GETINTVAR_DEFAULT(pi, rstr_calidxesttargethi5g, -1);
	pi_lcn40->calidxesttarget40hi5g =
		(int32) PHY_GETINTVAR_DEFAULT(pi, rstr_calidxesttarget40hi5g, -1);

	pi_lcn40->clipthr_eLNA2g =
		(int16) PHY_GETINTVAR_DEFAULT(pi, rstr_clipthr_eLNA2g, 0x3f);

	pi_lcn40->papdrx2g =
		(uint16) PHY_GETINTVAR_DEFAULT(pi, rstr_papdrx2g, PAPD_LNA2_GAIN_2G);

	pi_lcn40->papdrx5g =
		(uint16) PHY_GETINTVAR_DEFAULT(pi, rstr_papdrx5g, PAPD_LNA2_GAIN_5G);

	if (LCN40REV_IS(pi->pubpi.phy_rev, 2) ||
		LCN40REV_IS(pi->pubpi.phy_rev, 4) ||
		LCN40REV_IS(pi->pubpi.phy_rev, 5)) {
		if (pi->fabid == LCN40PHY_UMC_FABID)
			txidxcap_2g = 18;
		else if (pi->fabid == LCN40PHY_TSMC_FABID)
			txidxcap_2g = 20;
	}

	pi_lcn40->nom_txidxcap_2g =
	(int16) PHY_GETINTVAR_DEFAULT(pi, rstr_txidxcap2g, txidxcap_2g);

	pi_lcn40->nom_txidxcap_5g =
	(int16) PHY_GETINTVAR_DEFAULT(pi, rstr_txidxcap5g, 0);

	pi_lcn40->txidxcap_2g_high =
	(int16) PHY_GETINTVAR_DEFAULT(pi, rstr_txidxcap2g_hi, LCN40_TXIDXCAP_INVALID);

	pi_lcn40->txidxcap_2g_low =
	(int16) PHY_GETINTVAR_DEFAULT(pi, rstr_txidxcap2g_lo, LCN40_TXIDXCAP_INVALID);

	pi_lcn40->txidxcap_5g_high =
	(int16) PHY_GETINTVAR_DEFAULT(pi, rstr_txidxcap5g_hi, LCN40_TXIDXCAP_INVALID);

	pi_lcn40->txidxcap_5g_low =
	(int16) PHY_GETINTVAR_DEFAULT(pi, rstr_txidxcap5g_lo, LCN40_TXIDXCAP_INVALID);

	if (pi->fabid == LCN40PHY_UMC_FABID) {
		pi_lcn40->txidxcap_2g_off =
		(int8) PHY_GETINTVAR_DEFAULT(pi, rstr_txidxcap2g_off, 0);

		pi_lcn40->txidxcap_5g_off =
		(int8) PHY_GETINTVAR_DEFAULT(pi, rstr_txidxcap5g_off, 0);

		pi_lcn40->nom_txidxcap_2g += pi_lcn40->txidxcap_2g_off;
		pi_lcn40->nom_txidxcap_2g = MIN(pi_lcn40->nom_txidxcap_2g, 127);
		pi_lcn40->nom_txidxcap_2g = MAX(pi_lcn40->nom_txidxcap_2g, 0);

		if (pi_lcn40->txidxcap_2g_high != LCN40_TXIDXCAP_INVALID) {
			pi_lcn40->txidxcap_2g_high += pi_lcn40->txidxcap_2g_off;
			pi_lcn40->txidxcap_2g_high = MIN(pi_lcn40->txidxcap_2g_high, 127);
			pi_lcn40->txidxcap_2g_high = MAX(pi_lcn40->txidxcap_2g_high, 0);
		}

		if (pi_lcn40->txidxcap_2g_low != LCN40_TXIDXCAP_INVALID) {
			pi_lcn40->txidxcap_2g_low += pi_lcn40->txidxcap_2g_off;
			pi_lcn40->txidxcap_2g_low = MIN(pi_lcn40->txidxcap_2g_low, 127);
			pi_lcn40->txidxcap_2g_low = MAX(pi_lcn40->txidxcap_2g_low, 0);
		}

		pi_lcn40->nom_txidxcap_5g += pi_lcn40->txidxcap_5g_off;
		pi_lcn40->nom_txidxcap_5g = MIN(pi_lcn40->nom_txidxcap_5g, 127);
		pi_lcn40->nom_txidxcap_5g = MAX(pi_lcn40->nom_txidxcap_5g, 0);

		if (pi_lcn40->txidxcap_5g_high != LCN40_TXIDXCAP_INVALID) {
			pi_lcn40->txidxcap_5g_high += pi_lcn40->txidxcap_5g_off;
			pi_lcn40->txidxcap_5g_high = MIN(pi_lcn40->txidxcap_5g_high, 127);
			pi_lcn40->txidxcap_5g_high = MAX(pi_lcn40->txidxcap_5g_high, 0);
		}

		if (pi_lcn40->txidxcap_5g_low != LCN40_TXIDXCAP_INVALID) {
			pi_lcn40->txidxcap_5g_low += pi_lcn40->txidxcap_5g_off;
			pi_lcn40->txidxcap_5g_low = MIN(pi_lcn40->txidxcap_5g_low, 127);
			pi_lcn40->txidxcap_5g_low = MAX(pi_lcn40->txidxcap_5g_low, 0);
		}
	}

	pi_lcn->min_txpwrindex_2g = pi_lcn40->nom_txidxcap_2g;
	pi_lcn->min_txpwrindex_5g = pi_lcn40->nom_txidxcap_5g;

	for (i = 0; i < 3; i++) {
		pi_lcn40->rssi_rxsaw_slope_2g[i] =
		(int16) PHY_GETINTVAR_ARRAY_DEFAULT(pi, rstr_rssi_rxsaw_slope_2g, i, 0);
		pi_lcn40->rssi_dp2_slope_2g[i] =
		(int16) PHY_GETINTVAR_ARRAY_DEFAULT(pi, rstr_rssi_dp2_slope_2g, i, 0);
		pi_lcn40->rssi_eLNAon_slope_2g[i] =
		(int16) PHY_GETINTVAR_ARRAY_DEFAULT(pi, rstr_rssi_eLNAon_slope_2g, i, 0);
	}
	pi_lcn40->rssi_eLNAbyp_slope_2g =
	(int16) PHY_GETINTVAR_DEFAULT(pi, rstr_rssi_eLNAbyp_slope_2g, 0);
	pi_lcn40->rssi_eLNAbyp_slope_5g =
	(int16) PHY_GETINTVAR_DEFAULT(pi, rstr_rssi_eLNAbyp_slope_5g, 0);
	pi_lcn40->rssi_eLNAon_slope_5gh =
	(int16) PHY_GETINTVAR_DEFAULT(pi, rstr_rssi_eLNAon_slope_5gh, 0);
	pi_lcn40->rssi_eLNAon_slope_5gmu =
	(int16) PHY_GETINTVAR_DEFAULT(pi, rstr_rssi_eLNAon_slope_5gmu, 0);
	pi_lcn40->rssi_eLNAon_slope_5gml =
	(int16) PHY_GETINTVAR_DEFAULT(pi, rstr_rssi_eLNAon_slope_5gml, 0);
	pi_lcn40->rssi_eLNAon_slope_5gl =
	(int16) PHY_GETINTVAR_DEFAULT(pi, rstr_rssi_eLNAon_slope_5gl, 0);
	pi_lcn40->irdsw =
	(int8) PHY_GETINTVAR_DEFAULT(pi, rstr_irdsw, -1);

	pi_lcn40->new_lpf_rccal =
	(uint8) PHY_GETINTVAR_DEFAULT(pi, rstr_lpfflg, 0);

	pi_lcn40->cckscale_fctr_db =
		(int8) PHY_GETINTVAR_DEFAULT(pi, rstr_cckscale, -1);

	pi_lcn40->dac_scram_off_2g =
		(uint8) PHY_GETINTVAR_DEFAULT(pi, rstr_scram_off_2g, 0x3);
	pi_lcn40->dac_scram_off_5g =
		(uint8) PHY_GETINTVAR_DEFAULT(pi, rstr_scram_off_5g, 0);

	return TRUE;
}

static void
wlc_phy_txpwr_sromlcn40_read_ppr_parameters(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	srom_pwrdet_t *pwrdet  = pi->pwrdet;

	uint sromrev;

	/* Max tx power */
	pi->tx_srom_max_2g = (int8)PHY_GETINTVAR(pi, rstr_maxp2ga0);
	pwrdet->max_pwr[PHY_CORE_0][WL_CHAN_FREQ_RANGE_2G] = pi->tx_srom_max_2g;

	sromrev = (uint)PHY_GETINTVAR(pi, rstr_sromrev);
	if (sromrev >= 9)
		pi->ppr.sr_lcn40.cck202gpo = (uint16)PHY_GETINTVAR(pi, rstr_cckbw202gpo);
	else
		pi->ppr.sr_lcn40.cck202gpo = (uint16)PHY_GETINTVAR(pi, rstr_cck2gpo);

	/* Extract offsets for 8 OFDM rates */
	if (sromrev >= 9)
		pi->ppr.sr_lcn40.ofdmbw202gpo = (uint32)PHY_GETINTVAR(pi, rstr_legofdmbw202gpo);
	else
		pi->ppr.sr_lcn40.ofdmbw202gpo = (uint32)PHY_GETINTVAR(pi, rstr_ofdm2gpo);

	/* Extract offsets for 8 MCS rates */
	/* mcs2gpo(x) are 16 bit numbers */
	if (sromrev >= 9)
		pi->ppr.sr_lcn40.mcsbw202gpo = (uint32)PHY_GETINTVAR(pi, rstr_mcsbw202gpo);
	else
		pi->ppr.sr_lcn40.mcsbw202gpo =
			((uint16)PHY_GETINTVAR(pi, rstr_mcs2gpo1) << 16) |
			(uint16)PHY_GETINTVAR(pi, rstr_mcs2gpo0);
	pi_lcn->lcnphy_mcs20_po = pi->ppr.sr_lcn40.mcsbw202gpo;

	/* Extract offsets for 8 MCS 40MHz rates */
	/* mcs2gpo(x) are 16 bit numbers */
	if (sromrev >= 9)
		pi->ppr.sr_lcn40.mcsbw402gpo = (uint32)PHY_GETINTVAR(pi, rstr_mcsbw402gpo);
	else
		pi->ppr.sr_lcn40.mcsbw402gpo =
			((uint16)PHY_GETINTVAR(pi, rstr_mcs2gpo3) << 16) |
			(uint16)PHY_GETINTVAR(pi, rstr_mcs2gpo2);
	if (!pi->ppr.sr_lcn40.mcsbw402gpo)
		pi->ppr.sr_lcn40.mcsbw402gpo = pi->ppr.sr_lcn40.mcsbw202gpo;

#ifdef BAND5G
	/* Max tx power for 5G */
	pi->tx_srom_max_5g_mid = (int8)PHY_GETINTVAR(pi, rstr_maxp5ga0);

	/* Extract offsets for 8 OFDM mid rates */
	pi->ppr.sr_lcn40.ofdm5gpo = (uint32)PHY_GETINTVAR(pi, rstr_ofdm5gpo);

	/* Extract offsets for 8 MCS mid rates */
	/* mcs2gpo(x) are 16 bit numbers */
	pi->ppr.sr_lcn40.mcs5gpo0 = (uint16)PHY_GETINTVAR(pi, rstr_mcs5gpo0);
	pi->ppr.sr_lcn40.mcs5gpo1 = (uint16)PHY_GETINTVAR(pi, rstr_mcs5gpo1);

	/* Extract offsets for 8 MCS 40MHz rates */
	/* mcs2gpo(x) are 16 bit numbers */
	pi->ppr.sr_lcn40.mcs5gpo2 = (uint16)PHY_GETINTVAR(pi, rstr_mcs5gpo2);
	pi->ppr.sr_lcn40.mcs5gpo3 = (uint16)PHY_GETINTVAR(pi, rstr_mcs5gpo3);

	pi->tx_srom_max_5g_low = (int8)PHY_GETINTVAR(pi, rstr_maxp5gla0);

	/* Extract offsets for 8 OFDM low rates */
	pi->ppr.sr_lcn40.ofdm5glpo = (uint32)PHY_GETINTVAR(pi, rstr_ofdm5glpo);

	/* Extract offsets for 8 MCS low rates */
	/* mcs2gpo(x) are 16 bit numbers */
	pi->ppr.sr_lcn40.mcs5glpo0 = (uint16)PHY_GETINTVAR(pi, rstr_mcs5glpo0);
	pi->ppr.sr_lcn40.mcs5glpo1 = (uint16)PHY_GETINTVAR(pi, rstr_mcs5glpo1);

	/* Extract offsets for 8 MCS 40MHz rates */
	/* mcs2gpo(x) are 16 bit numbers */
	pi->ppr.sr_lcn40.mcs5glpo2 = (uint16)PHY_GETINTVAR(pi, rstr_mcs5glpo2);
	pi->ppr.sr_lcn40.mcs5glpo3 = (uint16)PHY_GETINTVAR(pi, rstr_mcs5glpo3);

	pi->tx_srom_max_5g_hi = (int8)PHY_GETINTVAR(pi, rstr_maxp5gha0);

	/* Extract offsets for 8 OFDM high rates */
	pi->ppr.sr_lcn40.ofdm5ghpo = (uint32)PHY_GETINTVAR(pi, rstr_ofdm5ghpo);

	/* Extract offsets for 8 MCS high rates */
	/* mcs2gpo(x) are 16 bit numbers */
	pi->ppr.sr_lcn40.mcs5ghpo0 = (uint16)PHY_GETINTVAR(pi, rstr_mcs5ghpo0);
	pi->ppr.sr_lcn40.mcs5ghpo1 = (uint16)PHY_GETINTVAR(pi, rstr_mcs5ghpo1);

	/* Extract offsets for 8 MCS 40MHz rates */
	/* mcs2gpo(x) are 16 bit numbers */
	pi->ppr.sr_lcn40.mcs5ghpo2 = (uint16)PHY_GETINTVAR(pi, rstr_mcs5ghpo2);
	pi->ppr.sr_lcn40.mcs5ghpo3 = (uint16)PHY_GETINTVAR(pi, rstr_mcs5ghpo3);

	pwrdet->max_pwr[PHY_CORE_0][WL_CHAN_FREQ_RANGE_5GL] = pi->tx_srom_max_5g_low;
	pwrdet->max_pwr[PHY_CORE_0][WL_CHAN_FREQ_RANGE_5GM] = pi->tx_srom_max_5g_mid;
	pwrdet->max_pwr[PHY_CORE_0][WL_CHAN_FREQ_RANGE_5GH] = pi->tx_srom_max_5g_hi;
#endif /* #ifdef BAND5G */
}

void
wlc_phy_txpwr_apply_sromlcn40(phy_info_t *pi, uint8 band, ppr_t *tx_srom_max_pwr)
{
	srom_lcn40_ppr_t *sr_lcn40 = &pi->ppr.sr_lcn40;
	int8 max_pwr_ref = 0;

	ppr_dsss_rateset_t ppr_dsss;
	ppr_ofdm_rateset_t ppr_ofdm;
	ppr_ht_mcs_rateset_t ppr_mcs;

#ifdef BAND5G
	uint32 offset_ofdm20, offset_mcs20, offset_mcs40;
#endif // endif

	switch (band)
	{
		case WL_CHAN_FREQ_RANGE_2G:
		{
			max_pwr_ref = pi->tx_srom_max_2g;

			/* 2G - CCK_20 */
			wlc_phy_txpwr_srom_convert_cck(sr_lcn40->cck202gpo, max_pwr_ref, &ppr_dsss);
			ppr_set_dsss(tx_srom_max_pwr, WL_TX_BW_20, WL_TX_CHAINS_1, &ppr_dsss);
			/* Infer 20in40 DSSS from this limit */
			ppr_set_dsss(tx_srom_max_pwr, WL_TX_BW_20IN40, WL_TX_CHAINS_1, &ppr_dsss);

			/* 2G - OFDM_20 */
			wlc_phy_txpwr_srom_convert_ofdm(sr_lcn40->ofdmbw202gpo, max_pwr_ref,
				&ppr_ofdm);
			ppr_set_ofdm(tx_srom_max_pwr, WL_TX_BW_20, WL_TX_MODE_NONE, WL_TX_CHAINS_1,
				&ppr_ofdm);

			/* 2G - MCS_20 */
			wlc_phy_txpwr_srom_convert_mcs(sr_lcn40->mcsbw202gpo, max_pwr_ref,
				&ppr_mcs);
			ppr_set_ht_mcs(tx_srom_max_pwr, WL_TX_BW_20, WL_TX_NSS_1, WL_TX_MODE_NONE,
				WL_TX_CHAINS_1, &ppr_mcs);

			/* 2G - MCS_40 */
			wlc_phy_txpwr_srom_convert_mcs(sr_lcn40->mcsbw402gpo, max_pwr_ref,
				&ppr_mcs);
			ppr_set_ht_mcs(tx_srom_max_pwr, WL_TX_BW_40, WL_TX_NSS_1, WL_TX_MODE_NONE,
				WL_TX_CHAINS_1, &ppr_mcs);
			/* Infer MCS_20in40 from MCS_40 */
			ppr_set_ht_mcs(tx_srom_max_pwr, WL_TX_BW_20IN40, WL_TX_NSS_1,
				WL_TX_MODE_NONE, WL_TX_CHAINS_1, &ppr_mcs);
			/* Infer OFDM_20in40 from MCS_40 */
			ppr_set_ofdm(tx_srom_max_pwr, WL_TX_BW_20IN40, WL_TX_MODE_NONE,
				WL_TX_CHAINS_1, (ppr_ofdm_rateset_t*)&ppr_mcs);
			/* Infer OFDM_40 from MCS_40 */
			ppr_set_ofdm(tx_srom_max_pwr, WL_TX_BW_40, WL_TX_MODE_NONE, WL_TX_CHAINS_1,
				(ppr_ofdm_rateset_t*)&ppr_mcs);

			break;
		}
#ifdef BAND5G
		case WL_CHAN_FREQ_RANGE_5GM:
		{
			max_pwr_ref = pi->tx_srom_max_5g_mid;
			offset_ofdm20 = sr_lcn40->ofdm5gpo;
			offset_mcs20 = (sr_lcn40->mcs5gpo1 << 16) | sr_lcn40->mcs5gpo0;
			offset_mcs40 = (sr_lcn40->mcs5gpo3 << 16) | sr_lcn40->mcs5gpo2;

			wlc_phy_txpwr_apply_srom_5g_subband(max_pwr_ref, tx_srom_max_pwr,
				offset_ofdm20, offset_mcs20, offset_mcs40);
			break;
		}
		case WL_CHAN_FREQ_RANGE_5GL:
		{
			max_pwr_ref = pi->tx_srom_max_5g_low;
			offset_ofdm20 = sr_lcn40->ofdm5glpo;
			offset_mcs20 = (sr_lcn40->mcs5glpo1 << 16) | sr_lcn40->mcs5glpo0;
			offset_mcs40 = (sr_lcn40->mcs5glpo3 << 16) | sr_lcn40->mcs5glpo2;

			wlc_phy_txpwr_apply_srom_5g_subband(max_pwr_ref, tx_srom_max_pwr,
				offset_ofdm20, offset_mcs20, offset_mcs40);
			break;
		}
		case WL_CHAN_FREQ_RANGE_5GH:
		{
			max_pwr_ref = pi->tx_srom_max_5g_hi;
			offset_ofdm20 = sr_lcn40->ofdm5ghpo;
			offset_mcs20 = (sr_lcn40->mcs5ghpo1 << 16) | sr_lcn40->mcs5ghpo0;
			offset_mcs40 = (sr_lcn40->mcs5ghpo3 << 16) | sr_lcn40->mcs5ghpo2;

			wlc_phy_txpwr_apply_srom_5g_subband(max_pwr_ref, tx_srom_max_pwr,
				offset_ofdm20, offset_mcs20, offset_mcs40);
			break;
		}
#endif /* #ifdef BAND5G */
	}
}

void
wlc_phy_txpower_sromlimit_get_lcn40phy(phy_info_t *pi, uint channel, ppr_t *max_pwr, uint8 core)
{
	srom_pwrdet_t *pwrdet  = pi->pwrdet;
	uint8 band;

	band = wlc_phy_get_band_from_channel(pi, channel);
	wlc_phy_txpwr_apply_sromlcn40(pi, band, max_pwr);
	ppr_apply_max(max_pwr, pwrdet->max_pwr[core][band]);
}

static void	wlc_lcn40phy_temp_adj(phy_info_t *pi)
{
}

static void
wlc_lcn40phy_clear_papd_comptable(phy_info_t *pi)
{
	phytbl_info_t tab;
	uint16 j;
	uint32 *papdcompdeltatbl_init_val = NULL;

	if ((papdcompdeltatbl_init_val =
		(uint32*) LCN40PHY_MALLOC(pi, 256 * sizeof(uint32))) == NULL) {
		PHY_ERROR(("wl%d: %s: MALLOC failure\n", pi->sh->unit, __FUNCTION__));
		return;
	}
	tab.tbl_ptr = papdcompdeltatbl_init_val; /* ptr to buf */
	tab.tbl_len = 256;        /* # values   */
	tab.tbl_id = LCN40PHY_TBL_ID_PAPDCOMPDELTATBL;         /* papdcompdeltatbl */
	tab.tbl_width = 32;     /* 32 bit wide */
	tab.tbl_offset = 0; /* tbl offset */

	for (j = 0; j < 256; j++) {
		papdcompdeltatbl_init_val[j] = 0x80000; /* lut init val */
	}
	wlc_lcn40phy_write_table(pi, &tab);

	if (papdcompdeltatbl_init_val)
		LCN40PHY_MFREE(pi, papdcompdeltatbl_init_val, 256 * sizeof(uint32));
	return;
}

static void
wlc_lcn40phy_txpower_recalc_target(phy_info_t *pi)
{
	phytbl_info_t tab;
	ppr_dsss_rateset_t dsss_limits;
	ppr_ofdm_rateset_t ofdm_limits;
	ppr_ht_mcs_rateset_t mcs_limits;
	uint32 rate_table[WL_RATESET_SZ_DSSS + WL_RATESET_SZ_OFDM + WL_RATESET_SZ_HT_MCS];
	wl_tx_bw_t bw_mcs = CHSPEC_IS40(pi->radio_chanspec) ? WL_TX_BW_40 : WL_TX_BW_20;
	uint i, j;
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
#if TWO_POWER_RANGE_TXPWR_CTRL
	int8 pmin_range;
	int8 diff;
#endif // endif

	/* FIX ME: The below is a hack for setting negative dBm power 	*/
	if (pi_lcn->offset_targetpwr) {
		PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlTargetPwr,
			targetPwr0, (wlc_phy_txpower_get_target_min((wlc_phy_t*)pi) -
			(pi_lcn->offset_targetpwr * 4)));
		return;
	}

	if (pi->tx_power_offset == NULL)
		return;

	/* Adjust rate based power offset */
	ppr_get_dsss(pi->tx_power_offset, WL_TX_BW_20, WL_TX_CHAINS_1, &dsss_limits);
	/* FTODO the below code is the correct inference, should we enable it? */
	/*	OFDM40 should be inferred from MCS40 not OFDM20.
	 * 	if (CHSPEC_IS40(pi->radio_chanspec)) {
	 * 		ppr_get_ht_mcs(pi->tx_power_offset, bw_mcs, WL_TX_NSS_1,
	 * 			WL_TX_MODE_NONE, WL_TX_CHAINS_1, &mcs_limits);
	 * 		bcopy(ofdm_limits.pwr, mcs_limits.pwr, sizeof(ofdm_limits.pwr));
	 * 	} else
	 */
	ppr_get_ofdm(pi->tx_power_offset, WL_TX_BW_20, WL_TX_MODE_NONE, WL_TX_CHAINS_1,
		&ofdm_limits);
	ppr_get_ht_mcs(pi->tx_power_offset, bw_mcs, WL_TX_NSS_1,
		WL_TX_MODE_NONE, WL_TX_CHAINS_1, &mcs_limits);

	j = 0;
	for (i = 0; i < WL_RATESET_SZ_DSSS; i++, j++) {
		rate_table[j] = (uint32)((int32)(-dsss_limits.pwr[i]));
		pi_lcn->rate_table[j] = rate_table[j];
		PHY_TMP((" Rate %d, offset %d\n", j, rate_table[j]));
	}

	for (i = 0; i < WL_RATESET_SZ_OFDM; i++, j++) {
		rate_table[j] = (uint32)((int32)(-ofdm_limits.pwr[i]));
		pi_lcn->rate_table[j] = rate_table[j];
		PHY_TMP((" Rate %d, offset %d\n", j, rate_table[j]));
	}

	for (i = 0; i < WL_RATESET_SZ_HT_MCS; i++, j++) {
		rate_table[j] = (uint32)((int32)(-mcs_limits.pwr[i]));
		pi_lcn->rate_table[j] = rate_table[j];
		PHY_TMP((" Rate %d, offset %d\n", j, rate_table[j]));
	}

	if (!pi_lcn->lcnphy_uses_rate_offset_table) {
		/* Preset txPwrCtrltbl */
		tab.tbl_id = LCN40PHY_TBL_ID_TXPWRCTL;
		tab.tbl_width = 32;	/* 32 bit wide	*/
		tab.tbl_len = ARRAYSIZE(rate_table); /* # values   */
		tab.tbl_ptr = rate_table; /* ptr to buf */
		tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_RATE_OFFSET;
		wlc_lcn40phy_write_table(pi, &tab);
	}
	/* a hack to boost power when duty cycle is low */
	if (CHIPID(pi->sh->chip) == BCM43142_CHIP_ID)
	{
		uint8 ii;
		uint32 pwr_lvl_qdB[64];
		/* Assign values from 0 to 63 qdB for now */
		for (ii = 1; ii < 64; ii++)
			pwr_lvl_qdB[ii] = -12;
		pwr_lvl_qdB[0] = 0;
		/* Prepare table structure */
		tab.tbl_ptr = pwr_lvl_qdB;
		tab.tbl_len = 64;
		tab.tbl_id = LCN40PHY_TBL_ID_TXPWRCTL;
		tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_MAC_OFFSET;
		tab.tbl_width = 32;

		/* Write to it */
		wlc_lcn40phy_write_table(pi, &tab);
	}

	wlc_lcn40phy_set_txpwr_clamp(pi);

#if defined(LP_P2P_SOFTAP) || defined(WL_LPC)
	/* Update the MACAddr LUT which is cleared when doing recal */
#ifdef LP_P2P_SOFTAP
	if (pi_lcn->pwr_offset_val)
#endif // endif
#ifdef WL_LPC
	    if (pi->lpc_algo)
#endif /* WL_LPC */
			wlc_lcn40phy_lpc_write_maclut(pi);
#endif /* LP_P2P_SOFTAP || WL_LPC */

	/* Set new target power */
	wlc_lcn40phy_set_target_tx_pwr(pi, wlc_phy_txpower_get_target_min((wlc_phy_t*)pi));

#if TWO_POWER_RANGE_TXPWR_CTRL
		if (pi_lcn->lcnphy_twopwr_txpwrctrl_en) {
			uint8 tx_power_min = wlc_phy_txpower_get_target_min((wlc_phy_t*)pi);
			/* RTL calculates intended TXPwr as targetPwr + RateOffset */
			/* While actual intended power is targetPower - RateOffset */
			ASSERT(pi_lcn->cckPwrIdxCorr <= 0);
			PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlPwrRange2,
				pwrMax_range2, tx_power_min - pi_lcn->cckPwrIdxCorr);
			diff = pi_lcn->pmax - tx_power_min;
			pmin_range = tx_power_min - diff - pi_lcn->cckPwrIdxCorr;
			PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlPwrRange2, pwrMin_range2, pmin_range);
		}
#endif /* #if TWO_POWER_RANGE_TXPWR_CTRL */

	/* Should reset power index cache */
	wlc_lcn40phy_txpower_reset_npt(pi);
}

static void
wlc_lcn40phy_set_tx_gain_override(phy_info_t *pi, bool bEnable)
{
	uint16 bit = bEnable ? 1 : 0;

	PHY_REG_MOD(pi, LCN40PHY, rfoverride2, txgainctrl_ovr, bit);

	PHY_REG_MOD(pi, LCN40PHY, AfeCtrlOvr, dacattctrl_ovr, bit);
	PHY_REG_MOD(pi, LCN40PHY, rfoverride8, tx_vlin_ovr, bit);
}

static void
wlc_lcn40phy_set_tx_gain(phy_info_t *pi,  phy_txgains_t *target_gains)
{
	uint16 pa_gain = wlc_lcn40phy_get_pa_gain(pi);

	PHY_REG_MOD(pi, LCN40PHY, txgainctrlovrval0, txgainctrl_ovr_val0,
		(target_gains->gm_gain) | (target_gains->pga_gain << 8));

	PHY_REG_MOD(pi, LCN40PHY, txgainctrlovrval1, txgainctrl_ovr_val1,
		(target_gains->pad_gain) | (pa_gain << 8));

	wlc_lcn40phy_set_dac_gain(pi, target_gains->dac_gain);
	/* Enable gain overrides */
	wlc_lcn40phy_enable_tx_gain_override(pi);
}

static void
wlc_lcn40phy_set_pa_gain(phy_info_t *pi, uint16 gain)
{
	PHY_REG_MOD(pi, LCN40PHY, txgainctrlovrval1, pagain_ovr_val1, gain);
}

static void
wlc_lcn40phy_set_dac_gain(phy_info_t *pi, uint16 dac_gain)
{
	uint16 dac_ctrl;

	dac_ctrl = (phy_utils_read_phyreg(pi, LCN40PHY_AfeDACCtrl) >>
	            LCN40PHY_AfeDACCtrl_dac_ctrl_SHIFT);
	dac_ctrl = dac_ctrl & 0xc7f;
	dac_ctrl = dac_ctrl | (dac_gain << 7);
	PHY_REG_MOD(pi, LCN40PHY, AfeDACCtrl, dac_ctrl, dac_ctrl);
}

static uint16
wlc_lcn40phy_get_pa_gain(phy_info_t *pi)
{
	uint16 pa_gain;

	pa_gain = (phy_utils_read_phyreg(pi, LCN40PHY_txgainctrlovrval1) &
		LCN40PHY_txgainctrlovrval1_pagain_ovr_val1_MASK) >>
		LCN40PHY_txgainctrlovrval1_pagain_ovr_val1_SHIFT;

	return pa_gain;
}

void
wlc_lcn40phy_set_bbmult(phy_info_t *pi, uint8 m0)
{
	uint16 m0m1 = (uint16)m0 << 8;
	phytbl_info_t tab;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	tab.tbl_ptr = &m0m1; /* ptr to buf */
	tab.tbl_len = 1;        /* # values   */
	tab.tbl_id = LCN40PHY_TBL_ID_IQLOCAL;         /* iqloCaltbl      */
	tab.tbl_offset = 87; /* tbl offset */
	tab.tbl_width = 16;     /* 16 bit wide */
	wlc_lcn40phy_write_table(pi, &tab);
}

static void
wlc_lcn40phy_filt_bw_set(phy_info_t *pi, uint16 bw)
{
	phy_utils_mod_phyreg(pi, LCN40PHY_lpfbwlutreg1, LCN40PHY_lpfbwlutreg1_lpf_ofdm_tx_bw_MASK |
		LCN40PHY_lpfbwlutreg1_lpf_cck_tx_bw_MASK, bw | (bw << 3));
	PHY_REG_MOD(pi, LCN40PHY, lpfofdmhtlutreg, lpf_ofdm_tx_ht_bw, bw);
}

static void
wlc_lcn40phy_txpower_reset_npt(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	pi_lcn->lcnphy_tssi_npt = LCN40PHY_TX_PWR_CTRL_START_NPT;
}

static void
wlc_lcn40phy_set_chanspec_tweaks(phy_info_t *pi, chanspec_t chanspec)
{
	phy_info_lcn40phy_t *pi_lcn40 = pi->u.pi_lcn40phy;
	uint8 channel = CHSPEC_CHANNEL(chanspec);

	if (pi_lcn40->dacpu) {
		phy_utils_or_radioreg(pi, RADIO_2065_OVR1, 0x800);
		phy_utils_or_radioreg(pi, RADIO_2065_DAC_CFG1, 1);
	}

	if (LCN40REV_GE(pi->pubpi.phy_rev, 4)) {
		if (CHSPEC_IS20(pi->radio_chanspec) &&
			CHSPEC_IS2G(pi->radio_chanspec)) {
			PHY_REG_MOD(pi, LCN40PHY, RxSpurBlkCtrl, scaler,
				spurblk_phy_reg_cfg[channel-1][0]);
			PHY_REG_MOD(pi, LCN40PHY, RxSpurBlkCtrl, NF1En,
				spurblk_phy_reg_cfg[channel-1][1]);
			PHY_REG_MOD(pi, LCN40PHY, RxSpurBlkNF1AlphaRe, man,
				spurblk_phy_reg_cfg[channel-1][2]);
			PHY_REG_MOD(pi, LCN40PHY, RxSpurBlkNF1AlphaRe, exp,
				spurblk_phy_reg_cfg[channel-1][3]);
			PHY_REG_MOD(pi, LCN40PHY, RxSpurBlkNF1AlphaIm, man,
				spurblk_phy_reg_cfg[channel-1][4]);
			PHY_REG_MOD(pi, LCN40PHY, RxSpurBlkNF1AlphaIm, exp,
				spurblk_phy_reg_cfg[channel-1][5]);
			PHY_REG_MOD(pi, LCN40PHY, RxSpurBlkNF1GammaRe, man,
				spurblk_phy_reg_cfg[channel-1][6]);
			PHY_REG_MOD(pi, LCN40PHY, RxSpurBlkNF1GammaRe, exp,
				spurblk_phy_reg_cfg[channel-1][7]);
			PHY_REG_MOD(pi, LCN40PHY, RxSpurBlkNF1GammaIm, man,
				spurblk_phy_reg_cfg[channel-1][8]);
			PHY_REG_MOD(pi, LCN40PHY, RxSpurBlkNF1GammaIm, exp,
				spurblk_phy_reg_cfg[channel-1][9]);
		} else {
			PHY_REG_WRITE(pi, LCN40PHY, RxSpurBlkCtrl, 0);
		}
	}
}

static void
wlc_lcn40phy_restore_calibration_results(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
#if defined(PHYCAL_CACHING)
	ch_calcache_t *ctx = wlc_phy_get_chanctx(pi, pi->radio_chanspec);
	lcnphy_calcache_t *cache;

	if (ctx)
		cache = &ctx->u.lcnphy_cache;
	else
		return;
#endif // endif

#if defined(PHYCAL_CACHING) && defined(BCMDBG)
	/* Print the params to be restored */
	wlc_phy_cal_cache_dbg_lcnphy(ctx);
#endif // endif
	/* restore tx iq cal results */
	wlc_lcn40phy_restore_txiqlo_calibration_results(pi, 0, 127, 0);

	/* restore PAPD cal results */
	if (pi_lcn->lcnphy_papd_4336_mode) {
		wlc_lcn40phy_restore_papd_calibration_results(pi);
	}

	/* restore rx iq cal results */
#if defined(PHYCAL_CACHING)
	wlc_lcn40phy_set_rx_iq_comp(pi, cache->rxiqcal_coeff_a0,
		cache->rxiqcal_coeff_b0);
#else
	wlc_lcn40phy_set_rx_iq_comp(pi, pi_lcn->lcnphy_cal_results.rxiqcal_coeff_a0,
		pi_lcn->lcnphy_cal_results.rxiqcal_coeff_b0);
#endif // endif
}
/* Enable and restore calibrations or disable calibration */
static void
wlc_lcn40phy_papd_txiqlo_rxiq_enable(phy_info_t *pi, bool enable, chanspec_t chanspec)
{
#if defined(PHYCAL_CACHING)
	ch_calcache_t *ctx = wlc_phy_get_chanctx(pi, chanspec);
#endif // endif

	if (enable) {
#if defined(PHYCAL_CACHING)
		if (ctx && ctx->valid) {
#else
		if (wlc_phy_getlcnphy_common(pi)->lcnphy_full_cal_channel) {
#endif // endif
			wlc_lcn40phy_restore_calibration_results(pi);
			PHY_REG_MOD(pi, LCN40PHY, rxfe, bypass_iqcomp, 0);
			if (!PHY_EPA_SUPPORT(wlc_phy_getlcnphy_common(pi)->ePA))
				PHY_REG_MOD(pi, LCN40PHY, papd_control, papdCompEn, 1);
		}
	} else {
		PHY_REG_MOD(pi, LCN40PHY, rxfe, bypass_iqcomp, 1);
		if (!PHY_EPA_SUPPORT(wlc_phy_getlcnphy_common(pi)->ePA))
			PHY_REG_MOD(pi, LCN40PHY, papd_control, papdCompEn, 0);
		wlc_lcn40phy_set_tx_locc(pi, 0);
		wlc_lcn40phy_set_tx_iqcc(pi, 0, 0);
	}
}

static void
wlc_lcn40phy_agc_reset(phy_info_t *pi)
{
	PHY_REG_LIST_START
		PHY_REG_WRITE_ENTRY(LCN40PHY, crsgainCtrl_new, 0)
		PHY_REG_MOD_ENTRY(LCN40PHY, agcControl4, c_agc_fsm_en, 0)
		PHY_REG_OR_ENTRY(LCN40PHY, resetCtrl, 0x47)
		PHY_REG_WRITE_ENTRY(LCN40PHY, resetCtrl, 0x0)
		PHY_REG_WRITE_ENTRY(LCN40PHY, crsgainCtrl_new, 0xff)
		PHY_REG_MOD_ENTRY(LCN40PHY, agcControl4, c_agc_fsm_en, 1)
	PHY_REG_LIST_EXECUTE(pi);
}

#if defined(WLTEST) || defined(SAMPLE_COLLECT)
static void
wlc_lcn40phy_agc_fsm_toggle(phy_info_t *pi)
{
	uint8 agcfsmstate =
	PHY_REG_READ(pi, LCN40PHY, agcControl4, c_agc_fsm_en);
	PHY_REG_MOD(pi, LCN40PHY, agcControl4, c_agc_fsm_en, !agcfsmstate);
	PHY_REG_MOD(pi, LCN40PHY, agcControl4, c_agc_fsm_en, agcfsmstate);
}
#endif /* #if defined(WLTEST) || defined(SAMPLE_COLLECT) */
static void
wlc_lcn40phy_radio_2065_channel_tune(phy_info_t *pi, uint8 channel)
{
	bool rfpll_doubler = 0;
	uint32 c7, c10, c11 = 0, d15, d16, f16, d18_int, d18_frac;
	uint32 d20, d21, d30_q16, d40, e32, d33, e33, g30, g35, g40, den;
	uint32 d34, e34, e35, e36, e37, e38, d41, e40, e41;
	uint32 d42, e42, e43;
	uint8 c11_divfact;

	chan_info_2065_lcn40phy_t *chi = wlc_lcn40phy_find_channel(pi, channel);
	phy_info_lcn40phy_t *pi_lcn40 = pi->u.pi_lcn40phy;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	ASSERT(chi != NULL);

	/* Turn on PLL power supplies */
	if (((CHSPEC_IS2G(pi->radio_chanspec)) && (pi_lcn40->rfpll_doubler_2g == TRUE)) ||
		((CHSPEC_IS5G(pi->radio_chanspec)) && (pi_lcn40->rfpll_doubler_5g == TRUE)))
		rfpll_doubler = 1;
	else
		rfpll_doubler = 0;

	/* Calculate various input frequencies */
	if (rfpll_doubler) {
		c7 = PHY_XTALFREQ(pi->xtalfreq) << 1;
		phy_utils_or_radioreg(pi, RADIO_2065_XTAL_CFG3, 1);
	} else {
		c7 = PHY_XTALFREQ(pi->xtalfreq);
		phy_utils_and_radioreg(pi, RADIO_2065_XTAL_CFG3, ~1);
	}
	/* xtal freq in MHz, C10 in KHz */
	c10 = PHY_XTALFREQ(pi->xtalfreq) / 1000;
	/* VCO freq */
	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		c11 = chi->freq * 9;
	}
#ifdef BAND5G
	else {
		c11 = chi->freq * 4;
		phy_utils_mod_radioreg(pi, RADIO_2065_LOGEN2G_CFG1, 0x7, 0);
	}
#endif // endif
	c11_divfact = 6;
	/* PLL_delayBeforeOpenLoop */
	phy_utils_mod_radioreg(pi, RADIO_2065_RFPLL_CAL_DELAYS2, 0xff, 5);
	/* PLL_enableTimeOut */
	d15 = (((c10 * 4) +  5000)/ 10000) - 1;
/* round here */
	phy_utils_mod_radioreg(pi, RADIO_2065_RFPLL_CAL_DELAYS3, 0xff00, d15 << 8);
	/* PLL_cal_ref_timeout */
	if ((c10 * 4/1000) % (d15 + 1))
		d16 = c10 * 4 / ((d15 + 1)*1000);
	else
		d16 = (c10 * 4 / ((d15 + 1)*1000)) - 1;
	phy_utils_mod_radioreg(pi, RADIO_2065_RFPLL_CAL_DELAYS3, 0xff, (uint16)d16);
	/* PLL_calSetCount */
	f16 = (((d16 + 1) * (d15 + 1) * 1000) << 10)/ c10;
	phy_utils_mod_radioreg(pi, RADIO_2065_RFPLL_CAL_OVR_COUNT, 0xfff0,
		((((f16 * c11 / (8 * c11_divfact)) + (2 << 9)) >> 10) -1) << 4);
	/* ref_value vars and wrtie */
	den = (c10/100) * 8 * c11_divfact;
	d18_int = (c11 * 10)/den;
	d18_frac = (c11 * 10)%den;
	d18_frac = ((d18_frac << 17) + (den >> 4)) / (den >> 3);

	phy_utils_write_radioreg(pi, RADIO_2065_RFPLL_REF_VAL1,
		(uint16)((d18_int << 4) | ((d18_frac >> 16) & 0xf)));
	phy_utils_write_radioreg(pi, RADIO_2065_RFPLL_REF_VAL0, (uint16)(d18_frac & 0xffff));
	/* cal_caps_sel vars and write */
	phy_utils_mod_radioreg(pi, RADIO_2065_RFPLL_CAL_CFG1, 0xe0, 3 << 5);

	/* divider, integer bits */
	den = (c7/100000) * c11_divfact;
	d20 = (c11 * 10)/den;
	d21 = (c11 * 10)%den;
	d21 = phy_utils_qdiv(d21, den, 20, FALSE);

	/* divider (wide base) */
	phy_utils_write_radioreg(pi, RADIO_2065_RFPLL_WILD_BASE0, (uint16)d21);
	phy_utils_write_radioreg(pi, RADIO_2065_RFPLL_WILD_BASE1, (d21 >> 16) + (d20 << 4));

	/* PLL_lf_r1 */
	e32 = ((PLL_2065_D32 - 850) + 160) / 320;
	/* PLL_lf_r2 */
	g30 = (e32 * 320) + 850;
	d33 = (g30 * PLL_2065_LF_R2) / PLL_2065_LF_R1;
	e33 = ((d33 - 850) + 160)/ 320;
	/* PLL_lf_r3 */
	d34 = g30 * PLL_2065_LF_R3 / PLL_2065_LF_R1;
	e34 = ((d34 - 850) + 160) / 320;
	/* PLL_lf_c1 */
	e35 = ((PLL_2065_D35 - 36) + 6) / 12;
	/* PLL_lf_c2 */
	/* set g35 [expr ($e35*12.0)+36]
	 * set d36 [expr $g35/15.0]
	* set e36 [expr round(($d36-2.4)/0.8)]
	 */
	g35 = (e35 * 12) + 36;
	e36 = ((g35 * 10 - 24 * 15) + 60) / (15 * 8);

	/* PLL_lf_c3 */
	/* set d37 [expr $g35*$c37/$c35]
	* set e37 [expr round(($d37-0.72)/0.24)]
	 */
	e37 = ((g35 * PLL_2065_LF_C3_X100 - 72 * PLL_2065_LF_C1) + (12 * PLL_2065_LF_C1)) /
		(24 * PLL_2065_LF_C1);
	/* PLL_lf_c4 */
	/* set d38 [expr ($g35*$c38)/$c35]
	* set e38 [expr round(($d38-0.72)/0.24)]
	 */
	e38 = ((g35 * PLL_2065_LF_C4_X100 - 72 * PLL_2065_LF_C1) + (12 * PLL_2065_LF_C1)) /
		(24 * PLL_2065_LF_C1);

	/* PLL_cp_current */
	d30_q16 = ((((PLL_2065_HIGH_END_KVCO_Q16 - PLL_2065_LOW_END_KVCO_Q16) /
		(PLL_2065_HIGH_END_VCO - PLL_2065_LOW_END_VCO)) *
		(c11 - (PLL_2065_LOW_END_VCO*c11_divfact)))/c11_divfact) +
	PLL_2065_LOW_END_KVCO_Q16;
#ifdef BAND5G
	if (CHSPEC_IS5G(pi->radio_chanspec))
		d40 = ((d20 * 41 * ((uint32)pi_lcn40->pll_loop_bw_desired_5g/100) *
		(pi_lcn40->pll_loop_bw_desired_5g/100)) << 16);
	else
#endif /* BAND5G */
		d40 = ((d20 * 41 * ((uint32)pi_lcn40->pll_loop_bw_desired_2g/100) *
		(pi_lcn40->pll_loop_bw_desired_2g/100)) << 16);
	d40 = d40 / (d30_q16 * 4);
	d40 = (d40 * 628 * 628)/100000;
	if (d40 > 200)
		d41 = 1;
	else
		d41 = 0;
	e41 = d41;
	den = 12 * (d41 * 4 + 1);
	e40 = (d40 + (den >> 1)) / den - 4;

	g40 = 12*(e40+4)*((d41*4)+1);
	d42 = 5 * g40 * (c7/100000) * c11_divfact;

	if (d42 / c11 / 10 > 60)
		e43 = 1;
	else
		e43 = 0;

	e42 = d42 / (2 * (e43 + 1) * c11 * 10);
	if (d42 % (2 * (e43 + 1) * c11 * 10))
		e42 += 1;

	phy_utils_mod_radioreg(pi, RADIO_2065_RFPLL_R2_R1, 0x3f, (uint16)e32);
	phy_utils_mod_radioreg(pi, RADIO_2065_RFPLL_R2_R1, 0x3f00, e33 << 8);
	phy_utils_mod_radioreg(pi, RADIO_2065_RFPLL_R3, 0x3f, (uint16)e34);
	phy_utils_mod_radioreg(pi, RADIO_2065_RFPLL_C2_C1, 0x1f, (uint16)e35);
	phy_utils_mod_radioreg(pi, RADIO_2065_RFPLL_C2_C1, 0x1f00, e36 << 8);
	phy_utils_mod_radioreg(pi, RADIO_2065_RFPLL_C4_C3, 0x1f, (uint16)e37);
	phy_utils_mod_radioreg(pi, RADIO_2065_RFPLL_C4_C3, 0x1f00, e38 << 8);
	phy_utils_mod_radioreg(pi, RADIO_2065_RFPLL_KPD, 0x1f, (uint16)e40);
	phy_utils_mod_radioreg(pi, RADIO_2065_RFPLL_KPD, 0x20, e41 << 5);
	phy_utils_mod_radioreg(pi, RADIO_2065_RFPLL_CP_IDAC, 0x1f0, e42 << 4);
	phy_utils_mod_radioreg(pi, RADIO_2065_RFPLL_CP_IDAC, 0x200, e43 << 9);

	wlc_2065_vco_cal(pi, TRUE);

	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		phy_utils_or_radioreg(pi, RADIO_2065_OVR3, 0x400);
		phy_utils_mod_radioreg(pi, RADIO_2065_LOGEN2G_CFG1, 0x3300,
			(chi->logen1 << 8) | (chi->logen2 << 12));
		phy_utils_mod_radioreg(pi, RADIO_2065_LNA2G_TUNE, 0xfff,
			(chi->lna_freq1) | (chi->lna_freq2 << 4) | (chi->lna_tx << 8));
		phy_utils_mod_radioreg(pi, RADIO_2065_TXMIX2G_CFG1, 0xf00, chi->txmix << 8);
		phy_utils_mod_radioreg(pi, RADIO_2065_PGA2G_CFG2, 0x7, chi->pga);
		phy_utils_mod_radioreg(pi, RADIO_2065_PAD2G_TUNE, 0x7, chi->pad);
	}
#ifdef BAND5G
	else {
		phy_utils_and_radioreg(pi, RADIO_2065_OVR3, ~0x400);
		phy_utils_mod_radioreg(pi, RADIO_2065_LOGEN5G_TUNE, 0xfff,
			(chi->logen1) | (chi->logen2 << 4) | (chi->logen3 << 8));
		phy_utils_mod_radioreg(pi, RADIO_2065_LNA5G_TUNE, 0xfff,
			(chi->lna_freq1) | (chi->lna_freq2 << 4) | (chi->lna_tx << 8));
		phy_utils_mod_radioreg(pi, RADIO_2065_TXMIX5G_CFG1, 0xf00, chi->txmix << 8);
		phy_utils_mod_radioreg(pi, RADIO_2065_PGA5G_CFG2, 0xf000, chi->pga << 12);
		phy_utils_mod_radioreg(pi, RADIO_2065_PAD5G_TUNE, 0xf, chi->pad);
		/* Off-tune pga5g_cfg2.tune to improve PAPD cal idx selection for high channels */
		if (LCN40REV_IS(pi->pubpi.phy_rev, 7)) {
			if (channel == 165 || channel == 159)
				phy_utils_mod_radioreg(pi, RADIO_2065_PGA5G_CFG2, 0xf000, 2 << 12);
		}
	}
#endif /* BAND5G */
	if (CHIPID(pi->sh->chip) == BCM4314_CHIP_ID ||
		CHIPID(pi->sh->chip) == BCM43142_CHIP_ID) {
		switch (channel) {
			case 1:
			PHY_REG_LIST_START
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_R2_R1, 0xffff, 0x0d1d)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_R3, 0xff, 0x000d)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_C2_C1, 0xffff, 0x1f1f)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_C4_C3, 0xffff, 0x1f1f)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_KPD, 0xff, 0x0028)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CP_IDAC, 0xffff, 0x3105)
				RADIO_REG_MOD_ENTRY(RADIO_2065_PAD2G_TUNE, 0xffff, 0x0073)
				RADIO_REG_MOD_ENTRY(RADIO_2065_LOGEN2G_CFG1, 0xffff, 0x01c7)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CAL_DELAYS2, 0xf, 0x5)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CAL_DELAYS3, 0xffff, 0x0709)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CAL_OVR_COUNT, 0xffff, 0x7101)
				RADIO_REG_WRITE_ENTRY(RADIO_2065_RFPLL_REF_VAL0, 0xcccc)
				RADIO_REG_WRITE_ENTRY(RADIO_2065_RFPLL_REF_VAL1, 0x0169)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CAL_CFG1, 0xffff, 0x0283)
				RADIO_REG_WRITE_ENTRY(RADIO_2065_RFPLL_WILD_BASE0, 0x6666)
				RADIO_REG_WRITE_ENTRY(RADIO_2065_RFPLL_WILD_BASE1, 0xb4e)
			PHY_REG_LIST_EXECUTE(pi);
			break;
			case 2:
			PHY_REG_LIST_START
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_R2_R1, 0xffff, 0x0d1d)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_R3, 0xff, 0x000d)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_C2_C1, 0xffff, 0x1f1f)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_C4_C3, 0xffff, 0x1f1f)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_KPD, 0xff, 0x0027)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CP_IDAC, 0xffff, 0x30f5)
				RADIO_REG_MOD_ENTRY(RADIO_2065_PAD2G_TUNE, 0xffff, 0x0073)
				RADIO_REG_MOD_ENTRY(RADIO_2065_LOGEN2G_CFG1, 0xffff, 0x01c7)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CAL_DELAYS2, 0xf, 0x5)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CAL_DELAYS3, 0xffff, 0x0709)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CAL_OVR_COUNT, 0xffff, 0x7141)
				RADIO_REG_WRITE_ENTRY(RADIO_2065_RFPLL_REF_VAL0, 0x8ccc)
				RADIO_REG_WRITE_ENTRY(RADIO_2065_RFPLL_REF_VAL1, 0x016a)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CAL_CFG1, 0xffff, 0x0283)
				RADIO_REG_WRITE_ENTRY(RADIO_2065_RFPLL_WILD_BASE0, 0x6666)
				RADIO_REG_WRITE_ENTRY(RADIO_2065_RFPLL_WILD_BASE1, 0xb54)
			PHY_REG_LIST_EXECUTE(pi);
			break;
			case 3:
			PHY_REG_LIST_START
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_R2_R1, 0xffff, 0x0d1d)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_R3, 0xff, 0x000d)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_C2_C1, 0xffff, 0x1f1f)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_C4_C3, 0xffff, 0x1f1f)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_KPD, 0xff, 0x0027)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CP_IDAC, 0xffff, 0x30f5)
				RADIO_REG_MOD_ENTRY(RADIO_2065_PAD2G_TUNE, 0xffff, 0x0073)
				RADIO_REG_MOD_ENTRY(RADIO_2065_LOGEN2G_CFG1, 0xffff, 0x01c7)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CAL_DELAYS2, 0xf, 0x5)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CAL_DELAYS3, 0xffff, 0x0709)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CAL_OVR_COUNT, 0xffff, 0x7181)
				RADIO_REG_WRITE_ENTRY(RADIO_2065_RFPLL_REF_VAL0, 0x4ccc)
				RADIO_REG_WRITE_ENTRY(RADIO_2065_RFPLL_REF_VAL1, 0x016b)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CAL_CFG1, 0xffff, 0x0283)
				RADIO_REG_WRITE_ENTRY(RADIO_2065_RFPLL_WILD_BASE0, 0x6666)
				RADIO_REG_WRITE_ENTRY(RADIO_2065_RFPLL_WILD_BASE1, 0xb5a)
			PHY_REG_LIST_EXECUTE(pi);
			break;
			case 4:
			PHY_REG_LIST_START
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_R2_R1, 0xffff, 0x0d1d)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_R3, 0xff, 0x000d)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_C2_C1, 0xffff, 0x1f1f)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_C4_C3, 0xffff, 0x1f1f)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_KPD, 0xff, 0x0027)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CP_IDAC, 0xffff, 0x30f5)
				RADIO_REG_MOD_ENTRY(RADIO_2065_PAD2G_TUNE, 0xffff, 0x0073)
				RADIO_REG_MOD_ENTRY(RADIO_2065_LOGEN2G_CFG1, 0xffff, 0x01c7)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CAL_DELAYS2, 0xf, 0x5)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CAL_DELAYS3, 0xffff, 0x0709)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CAL_OVR_COUNT, 0xffff, 0x71b1)
				RADIO_REG_WRITE_ENTRY(RADIO_2065_RFPLL_REF_VAL0, 0x0ccc)
				RADIO_REG_WRITE_ENTRY(RADIO_2065_RFPLL_REF_VAL1, 0x016c)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CAL_CFG1, 0xffff, 0x0283)
				RADIO_REG_WRITE_ENTRY(RADIO_2065_RFPLL_WILD_BASE0, 0x6666)
				RADIO_REG_WRITE_ENTRY(RADIO_2065_RFPLL_WILD_BASE1, 0xb60)
			PHY_REG_LIST_EXECUTE(pi);
			break;
			case 5:
			PHY_REG_LIST_START
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_R2_R1, 0xffff, 0x0d1d)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_R3, 0xff, 0x000d)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_C2_C1, 0xffff, 0x1f1f)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_C4_C3, 0xffff, 0x1f1f)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_KPD, 0xff, 0x0027)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CP_IDAC, 0xffff, 0x30f5)
				RADIO_REG_MOD_ENTRY(RADIO_2065_PAD2G_TUNE, 0xffff, 0x0072)
				RADIO_REG_MOD_ENTRY(RADIO_2065_LOGEN2G_CFG1, 0xffff, 0x01c7)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CAL_DELAYS2, 0xf, 0x5)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CAL_DELAYS3, 0xffff, 0x0709)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CAL_OVR_COUNT, 0xffff, 0x71f1)
				RADIO_REG_WRITE_ENTRY(RADIO_2065_RFPLL_REF_VAL0, 0xcccc)
				RADIO_REG_WRITE_ENTRY(RADIO_2065_RFPLL_REF_VAL1, 0x016c)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CAL_CFG1, 0xffff, 0x0283)
				RADIO_REG_WRITE_ENTRY(RADIO_2065_RFPLL_WILD_BASE0, 0x6666)
				RADIO_REG_WRITE_ENTRY(RADIO_2065_RFPLL_WILD_BASE1, 0xb66)
			PHY_REG_LIST_EXECUTE(pi);
			break;
			case 6:
			PHY_REG_LIST_START
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_R2_R1, 0xffff, 0x0d1d)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_R3, 0xff, 0x000d)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_C2_C1, 0xffff, 0x1f1f)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_C4_C3, 0xffff, 0x1f1f)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_KPD, 0xff, 0x0027)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CP_IDAC, 0xffff, 0x30f5)
				RADIO_REG_MOD_ENTRY(RADIO_2065_PAD2G_TUNE, 0xffff, 0x0072)
				RADIO_REG_MOD_ENTRY(RADIO_2065_LOGEN2G_CFG1, 0xffff, 0x01c7)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CAL_DELAYS2, 0xf, 0x5)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CAL_DELAYS3, 0xffff, 0x0709)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CAL_OVR_COUNT, 0xffff, 0x7231)
				RADIO_REG_WRITE_ENTRY(RADIO_2065_RFPLL_REF_VAL0, 0x8ccc)
				RADIO_REG_WRITE_ENTRY(RADIO_2065_RFPLL_REF_VAL1, 0x016d)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CAL_CFG1, 0xffff, 0x0283)
				RADIO_REG_WRITE_ENTRY(RADIO_2065_RFPLL_WILD_BASE0, 0x6666)
				RADIO_REG_WRITE_ENTRY(RADIO_2065_RFPLL_WILD_BASE1, 0xb6c)
			PHY_REG_LIST_EXECUTE(pi);
			break;
			case 7:
			PHY_REG_LIST_START
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_R2_R1, 0xffff, 0x0d1d)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_R3, 0xff, 0x000d)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_C2_C1, 0xffff, 0x1f1f)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_C4_C3, 0xffff, 0x1f1f)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_KPD, 0xff, 0x0027)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CP_IDAC, 0xffff, 0x30f5)
				RADIO_REG_MOD_ENTRY(RADIO_2065_PAD2G_TUNE, 0xffff, 0x0072)
				RADIO_REG_MOD_ENTRY(RADIO_2065_LOGEN2G_CFG1, 0xffff, 0x01c7)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CAL_DELAYS2, 0xf, 0x5)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CAL_DELAYS3, 0xffff, 0x0709)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CAL_OVR_COUNT, 0xffff, 0x7271)
				RADIO_REG_WRITE_ENTRY(RADIO_2065_RFPLL_REF_VAL0, 0x4ccc)
				RADIO_REG_WRITE_ENTRY(RADIO_2065_RFPLL_REF_VAL1, 0x016e)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CAL_CFG1, 0xffff, 0x0283)
				RADIO_REG_WRITE_ENTRY(RADIO_2065_RFPLL_WILD_BASE0, 0x6666)
				RADIO_REG_WRITE_ENTRY(RADIO_2065_RFPLL_WILD_BASE1, 0xb72)
			PHY_REG_LIST_EXECUTE(pi);
			break;
			case 8:
			PHY_REG_LIST_START
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_R2_R1, 0xffff, 0x0d1d)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_R3, 0xff, 0x000d)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_C2_C1, 0xffff, 0x1f1f)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_C4_C3, 0xffff, 0x1f1f)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_KPD, 0xff, 0x0027)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CP_IDAC, 0xffff, 0x30f5)
				RADIO_REG_MOD_ENTRY(RADIO_2065_PAD2G_TUNE, 0xffff, 0x0072)
				RADIO_REG_MOD_ENTRY(RADIO_2065_LOGEN2G_CFG1, 0xffff, 0x01c7)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CAL_DELAYS2, 0xf, 0x5)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CAL_DELAYS3, 0xffff, 0x0709)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CAL_OVR_COUNT, 0xffff, 0x72a1)
				RADIO_REG_WRITE_ENTRY(RADIO_2065_RFPLL_REF_VAL0, 0x0ccc)
				RADIO_REG_WRITE_ENTRY(RADIO_2065_RFPLL_REF_VAL1, 0x016f)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CAL_CFG1, 0xffff, 0x0283)
				RADIO_REG_WRITE_ENTRY(RADIO_2065_RFPLL_WILD_BASE0, 0x6666)
				RADIO_REG_WRITE_ENTRY(RADIO_2065_RFPLL_WILD_BASE1, 0xb78)
			PHY_REG_LIST_EXECUTE(pi);
			break;
			case 9:
			PHY_REG_LIST_START
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_R2_R1, 0xffff, 0x0d1d)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_R3, 0xff, 0x000d)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_C2_C1, 0xffff, 0x1f1f)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_C4_C3, 0xffff, 0x1f1f)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_KPD, 0xff, 0x0027)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CP_IDAC, 0xffff, 0x30f5)
				RADIO_REG_MOD_ENTRY(RADIO_2065_PAD2G_TUNE, 0xffff, 0x0072)
				RADIO_REG_MOD_ENTRY(RADIO_2065_LOGEN2G_CFG1, 0xffff, 0x01c7)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CAL_DELAYS2, 0xf, 0x5)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CAL_DELAYS3, 0xffff, 0x0709)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CAL_OVR_COUNT, 0xffff, 0x72e1)
				RADIO_REG_WRITE_ENTRY(RADIO_2065_RFPLL_REF_VAL0, 0xcccc)
				RADIO_REG_WRITE_ENTRY(RADIO_2065_RFPLL_REF_VAL1, 0x016f)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CAL_CFG1, 0xffff, 0x0283)
				RADIO_REG_WRITE_ENTRY(RADIO_2065_RFPLL_WILD_BASE0, 0x6666)
				RADIO_REG_WRITE_ENTRY(RADIO_2065_RFPLL_WILD_BASE1, 0xb7e)
			PHY_REG_LIST_EXECUTE(pi);
			break;
			case 10:
			PHY_REG_LIST_START
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_R2_R1, 0xffff, 0x0d1d)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_R3, 0xff, 0x000d)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_C2_C1, 0xffff, 0x1f1f)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_C4_C3, 0xffff, 0x1f1f)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_KPD, 0xff, 0x0027)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CP_IDAC, 0xffff, 0x30f5)
				RADIO_REG_MOD_ENTRY(RADIO_2065_PAD2G_TUNE, 0xffff, 0x0072)
				RADIO_REG_MOD_ENTRY(RADIO_2065_LOGEN2G_CFG1, 0xffff, 0x01c7)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CAL_DELAYS2, 0xf, 0x5)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CAL_DELAYS3, 0xffff, 0x0709)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CAL_OVR_COUNT, 0xffff, 0x7321)
				RADIO_REG_WRITE_ENTRY(RADIO_2065_RFPLL_REF_VAL0, 0x8ccc)
				RADIO_REG_WRITE_ENTRY(RADIO_2065_RFPLL_REF_VAL1, 0x0170)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CAL_CFG1, 0xffff, 0x0283)
				RADIO_REG_WRITE_ENTRY(RADIO_2065_RFPLL_WILD_BASE0, 0x6666)
				RADIO_REG_WRITE_ENTRY(RADIO_2065_RFPLL_WILD_BASE1, 0xb84)
			PHY_REG_LIST_EXECUTE(pi);
			break;
			case 11:
			PHY_REG_LIST_START
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_R2_R1, 0xffff, 0x0d1d)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_R3, 0xff, 0x000d)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_C2_C1, 0xffff, 0x1f1f)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_C4_C3, 0xffff, 0x1f1f)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_KPD, 0xff, 0x0027)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CP_IDAC, 0xffff, 0x30f5)
				RADIO_REG_MOD_ENTRY(RADIO_2065_PAD2G_TUNE, 0xffff, 0x0072)
				RADIO_REG_MOD_ENTRY(RADIO_2065_LOGEN2G_CFG1, 0xffff, 0x01c7)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CAL_DELAYS2, 0xf, 0x5)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CAL_DELAYS3, 0xffff, 0x0709)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CAL_OVR_COUNT, 0xffff, 0x7361)
				RADIO_REG_WRITE_ENTRY(RADIO_2065_RFPLL_REF_VAL0, 0x4ccc)
				RADIO_REG_WRITE_ENTRY(RADIO_2065_RFPLL_REF_VAL1, 0x0171)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CAL_CFG1, 0xffff, 0x0283)
				RADIO_REG_WRITE_ENTRY(RADIO_2065_RFPLL_WILD_BASE0, 0x6666)
				RADIO_REG_WRITE_ENTRY(RADIO_2065_RFPLL_WILD_BASE1, 0xb8a)
			PHY_REG_LIST_EXECUTE(pi);
			break;
			case 12:
			PHY_REG_LIST_START
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_R2_R1, 0xffff, 0x0d1d)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_R3, 0xff, 0x000d)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_C2_C1, 0xffff, 0x1f1f)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_C4_C3, 0xffff, 0x1f1f)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_KPD, 0xff, 0x0027)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CP_IDAC, 0xffff, 0x30f5)
				RADIO_REG_MOD_ENTRY(RADIO_2065_PAD2G_TUNE, 0xffff, 0x0071)
				RADIO_REG_MOD_ENTRY(RADIO_2065_LOGEN2G_CFG1, 0xffff, 0x01c7)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CAL_DELAYS2, 0xf, 0x5)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CAL_DELAYS3, 0xffff, 0x0709)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CAL_OVR_COUNT, 0xffff, 0x7391)
				RADIO_REG_WRITE_ENTRY(RADIO_2065_RFPLL_REF_VAL0, 0x0ccc)
				RADIO_REG_WRITE_ENTRY(RADIO_2065_RFPLL_REF_VAL1, 0x0172)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CAL_CFG1, 0xffff, 0x0283)
				RADIO_REG_WRITE_ENTRY(RADIO_2065_RFPLL_WILD_BASE0, 0x6666)
				RADIO_REG_WRITE_ENTRY(RADIO_2065_RFPLL_WILD_BASE1, 0xb90)
			PHY_REG_LIST_EXECUTE(pi);
			break;
			case 13:
			PHY_REG_LIST_START
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_R2_R1, 0xffff, 0x0d1d)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_R3, 0xff, 0x000d)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_C2_C1, 0xffff, 0x1f1f)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_C4_C3, 0xffff, 0x1f1f)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_KPD, 0xff, 0x0027)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CP_IDAC, 0xffff, 0x30f5)
				RADIO_REG_MOD_ENTRY(RADIO_2065_PAD2G_TUNE, 0xffff, 0x0071)
				RADIO_REG_MOD_ENTRY(RADIO_2065_LOGEN2G_CFG1, 0xffff, 0x01c7)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CAL_DELAYS2, 0xf, 0x5)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CAL_DELAYS3, 0xffff, 0x0709)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CAL_OVR_COUNT, 0xffff, 0x73d1)
				RADIO_REG_WRITE_ENTRY(RADIO_2065_RFPLL_REF_VAL0, 0xcccc)
				RADIO_REG_WRITE_ENTRY(RADIO_2065_RFPLL_REF_VAL1, 0x0172)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CAL_CFG1, 0xffff, 0x0283)
				RADIO_REG_WRITE_ENTRY(RADIO_2065_RFPLL_WILD_BASE0, 0x6666)
				RADIO_REG_WRITE_ENTRY(RADIO_2065_RFPLL_WILD_BASE1, 0xb96)
			PHY_REG_LIST_EXECUTE(pi);
			break;
			default:
			PHY_REG_LIST_START
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_R2_R1, 0xffff, 0x0d1d)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_R3, 0xff, 0x000d)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_C2_C1, 0xffff, 0x1f1f)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_C4_C3, 0xffff, 0x1f1f)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_KPD, 0xff, 0x0027)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CP_IDAC, 0xffff, 0x30f5)
				RADIO_REG_MOD_ENTRY(RADIO_2065_PAD2G_TUNE, 0xffff, 0x0071)
				RADIO_REG_MOD_ENTRY(RADIO_2065_LOGEN2G_CFG1, 0xffff, 0x01c7)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CAL_DELAYS2, 0xf, 0x5)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CAL_DELAYS3, 0xffff, 0x0709)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CAL_OVR_COUNT, 0xffff, 0x7461)
				RADIO_REG_WRITE_ENTRY(RADIO_2065_RFPLL_REF_VAL0, 0x9999)
				RADIO_REG_WRITE_ENTRY(RADIO_2065_RFPLL_REF_VAL1, 0x0174)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CAL_CFG1, 0xffff, 0x0283)
				RADIO_REG_WRITE_ENTRY(RADIO_2065_RFPLL_WILD_BASE0, 0xcccc)
				RADIO_REG_WRITE_ENTRY(RADIO_2065_RFPLL_WILD_BASE1, 0xba4)
			PHY_REG_LIST_EXECUTE(pi);
			break;
		}
		if (CHIPID(pi->sh->chip) == BCM4314_CHIP_ID) {
			phy_utils_mod_radioreg(pi, RADIO_2065_PGA2G_CFG2, 0xffff, 0x0005);
			phy_utils_mod_radioreg(pi, RADIO_2065_TXMIX2G_CFG1, 0xf00, 0x100);
			if (CHSPEC_IS40(pi->radio_chanspec)) {
				phy_utils_mod_radioreg(pi, RADIO_2065_PA2G_INCAP, 0xf0f, 0x806);
				phy_utils_mod_radioreg(pi, RADIO_2065_PA2G_CFG2, 0xff, 0x66);
			} else {
				phy_utils_mod_radioreg(pi, RADIO_2065_PA2G_INCAP, 0xf0f, 0x404);
				phy_utils_mod_radioreg(pi, RADIO_2065_PA2G_CFG2, 0xff, 0x0);
			}
		} else {
			phy_utils_mod_radioreg(pi, RADIO_2065_PA2G_INCAP, 0xf0f, 0x606);
			if ((CHIPREV(pi->sh->chiprev) < 1) || (CHSPEC_IS40(pi->radio_chanspec)))
				phy_utils_mod_radioreg(pi, RADIO_2065_PGA2G_CFG2, 0x7, 0x7);
			else
				phy_utils_mod_radioreg(pi, RADIO_2065_PGA2G_CFG2, 0x7, 0x5);
		}
	}

	if (CHIPID(pi->sh->chip) == BCM43342_CHIP_ID) {
		if (CHSPEC_IS2G(pi->radio_chanspec))
			phy_utils_mod_radioreg(pi, RADIO_2065_DAC_CFG1, 0x300,
				pi_lcn40->dac_scram_off_2g << 8);
		else
			phy_utils_mod_radioreg(pi, RADIO_2065_DAC_CFG1, 0x300,
				pi_lcn40->dac_scram_off_5g << 8);
	}
}

static void
wlc_lcn40phy_radio_2065_channel_tune_new(phy_info_t *pi, uint8 channel)
{
	bool rfpll_doubler = 0;
	uint32 ref_freq, xtal_freq, vco_freq = 0;
	uint32 int_part, frac_part;
	uint32 reg_val, reg_val2, exact_val;
	uint32 lf_c1_des, lf_c1_exact;
	uint32 enable_timeout;
	uint32 tmp;
	uint32 loop_bw;
	uint32 a_fact_init, a_fact;
	uint32 kvco_lo, kvco_hi, kvco_des;
	uint32 kpd_upscale;
	uint32 cpd_cur_des;
	uint32 cp_ioff_des;
	const pll_constants_lcn40phy_t *pll_const = NULL;

	chan_info_2065_lcn40phy_t *chi = wlc_lcn40phy_find_channel(pi, channel);
	phy_info_lcn40phy_t *pi_lcn40 = pi->u.pi_lcn40phy;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	ASSERT(chi != NULL);

	/* Calculate various input frequencies */
	if (((CHSPEC_IS2G(pi->radio_chanspec)) && (pi_lcn40->rfpll_doubler_2g == TRUE)) ||
		((CHSPEC_IS5G(pi->radio_chanspec)) && (pi_lcn40->rfpll_doubler_5g == TRUE)))
		rfpll_doubler = 1;
	else
		rfpll_doubler = 0;

	/* PFD ref freq (in Hz) ==> spreadsheet/tcl c7 (in MHz) */
	if (rfpll_doubler) {
		ref_freq = PHY_XTALFREQ(pi->xtalfreq) << 1;
		phy_utils_or_radioreg(pi, RADIO_2065_XTAL_CFG3, 1);
	} else {
		ref_freq = PHY_XTALFREQ(pi->xtalfreq);
		phy_utils_and_radioreg(pi, RADIO_2065_XTAL_CFG3, ~1);
	}

	/* cal xtal freq (in kHz) ==> spreadsheet/tcl c10 (in MHz) */
	xtal_freq = PHY_XTALFREQ(pi->xtalfreq) / 1000;

	/* VCO frequency * 6 (in MHz) ==> spreadsheet/tcl c11 (in MHz) */
	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		vco_freq = chi->freq * (PLL_2065_VCO_DIVFACT * 3 / 2);
	}
#ifdef BAND5G
	else {
		vco_freq = chi->freq * (PLL_2065_VCO_DIVFACT * 2 / 3);
		if (RADIOID(pi->pubpi.radioid) == BCM2067_ID)
			phy_utils_mod_radioreg(pi, RADIO_2065_LOGEN2G_CFG1, 0x7, 0);
	}
#endif // endif

	/* *********************** RF PLL programming *********************** */

	/* select appropriate PLL constants for our radio */
	if (RADIOID(pi->pubpi.radioid) == BCM2067_ID)
		pll_const = &pll_constants_2067;
	else
		pll_const = &pll_constants_2065;

	/* enableTimeOut ==> spreadsheet/tcl d43 */
	enable_timeout = (((xtal_freq * (PLL_2065_ENABLE_TIMEOUT_WANTED / 2)) + 5000) / 10000) - 1;
	phy_utils_mod_radioreg(pi, RADIO_2065_RFPLL_CAL_DELAYS3, 0xff00, enable_timeout << 8);

	/* delayBeforeOpenLoop ==> spreadsheet/tcl d44 */
	phy_utils_mod_radioreg(pi, RADIO_2065_RFPLL_CAL_DELAYS2,
	                       0xff, PLL_2065_DELAY_BEFORE_OPEN_LOOP);

	/* cal_ref_timeout ==> spreadsheet/tcl d45 */
	if ((xtal_freq * PLL_2065_CAL_REF_TIMEOUT / 1000) % (enable_timeout + 1))
		reg_val = (xtal_freq * PLL_2065_CAL_REF_TIMEOUT) / ((enable_timeout + 1) * 1000);
	else
		reg_val = (xtal_freq * PLL_2065_CAL_REF_TIMEOUT) /
			((enable_timeout + 1) * 1000) - 1;
	phy_utils_mod_radioreg(pi, RADIO_2065_RFPLL_CAL_DELAYS3, 0xff, (uint16) reg_val);
	exact_val = (((reg_val + 1) * (enable_timeout + 1) * 10000000) + (xtal_freq >> 1)) /
		xtal_freq;

	/* calSetCount ==> spreadsheet/tcl d46 */
	reg_val = (exact_val * vco_freq + (4 * 10000 * PLL_2065_VCO_DIVFACT)) /
		(8 * 10000 * PLL_2065_VCO_DIVFACT) - 1;
	phy_utils_mod_radioreg(pi, RADIO_2065_RFPLL_CAL_OVR_COUNT, 0xfff0, reg_val << 4);

	/* ref_value ==> spreadsheet/tcl d47 */
	tmp = (xtal_freq / 100) * 8 * PLL_2065_VCO_DIVFACT;
	int_part = (vco_freq * 10) / tmp;
	frac_part = (vco_freq * 10) % tmp;
	frac_part = (frac_part << 16) / (tmp >> 4);
	phy_utils_write_radioreg(pi, RADIO_2065_RFPLL_REF_VAL0, (uint16) frac_part);
	phy_utils_write_radioreg(pi, RADIO_2065_RFPLL_REF_VAL1,
	                         (int_part << 4) | (frac_part >> 16));

	/* cal_caps_sel ==> spreadsheet/tcl d48 */
	phy_utils_mod_radioreg(pi, RADIO_2065_RFPLL_CAL_CFG1, 0xe0, PLL_2065_CAPS_CAL_SEL << 5);

	/* divider (wide base) ==> spreadsheet/tcl d49.d50 */
	tmp = (ref_freq / 100000) * PLL_2065_VCO_DIVFACT;
	int_part = (vco_freq * 10) / tmp;
	frac_part = (vco_freq * 10) % tmp;
	frac_part = (frac_part << 18) / (tmp >> 2);
	phy_utils_write_radioreg(pi, RADIO_2065_RFPLL_WILD_BASE0, (uint16) frac_part);
	phy_utils_write_radioreg(pi, RADIO_2065_RFPLL_WILD_BASE1,
	                         (int_part << 4) | (frac_part >> 16));

	/* loop bandwidth ==> spreadsheet/tcl d60 */
#ifdef BAND5G
	if (CHSPEC_IS5G(pi->radio_chanspec))
		loop_bw = pi_lcn40->pll_loop_bw_desired_5g;
	else
#endif /* BAND5G */
		loop_bw = pi_lcn40->pll_loop_bw_desired_2g;

	/* lf_c1 and lf_c2 ==> spreadsheet/tcl e64 and e65 */
	if (RADIOID(pi->pubpi.radioid) == BCM2067_ID) {
		uint32 C0;
		uint32 bw_sq = loop_bw * loop_bw;
		a_fact_init = 600;
		/* C0 ==> spreadsheet/tcl c62 */
		C0 = (a_fact_init * PLL_2065_VAR2_INV + (bw_sq >> 1)) / bw_sq;
		lf_c1_des = loop_bw > PLL_2065_VAR3 ? C0 : pll_const->c_max;
	} else {
		uint32 C0;
		uint32 bw_sq = loop_bw * loop_bw;
		a_fact_init = ref_freq < 30000000 ? 100 : ref_freq > 60000000 ? 400 : 200;
		/* C0 ==> spreadsheet/tcl c62 */
		C0 = (a_fact_init * PLL_2065_VAR2_INV + (bw_sq >> 1)) / bw_sq;
		lf_c1_des = C0 <= pll_const->c_max ? C0 : pll_const->c_max;
	}
	reg_val = (lf_c1_des - pll_const->c1_min + (pll_const->c1_step >> 1)) / pll_const->c1_step;
	lf_c1_exact = (reg_val * pll_const->c1_step) + pll_const->c1_min;
	reg_val2 = ((lf_c1_exact << 1) - pll_const->c2_min + (pll_const->c2_step >> 1)) /
		pll_const->c2_step;
	phy_utils_mod_radioreg(pi, RADIO_2065_RFPLL_C2_C1, 0x1f1f, (reg_val2 << 8) | reg_val);

	/* lf_c3 and lf_c4 ==> spreadsheet/tcl e66 and e67 */
	reg_val = pll_const->c3_step * pll_const->c1_ref;
	reg_val = (lf_c1_exact * pll_const->c3_ref - pll_const->c3_min * pll_const->c1_ref +
		(reg_val >> 1)) / reg_val;
	reg_val2 = pll_const->c4_step * pll_const->c1_ref;
	reg_val2 = (lf_c1_exact * pll_const->c4_ref - pll_const->c4_min * pll_const->c1_ref +
		(reg_val2 >> 1)) / reg_val2;
	phy_utils_mod_radioreg(pi, RADIO_2065_RFPLL_C4_C3, 0x1f1f, (reg_val2 << 8) | reg_val);

	/* lf_r1 and lf_r2 ==> spreadsheet/tcl e68 and e69 */
	reg_val = PLL_2065_PM_FACTOR_DIV_VAR1 / (loop_bw * lf_c1_des);
	reg_val = (reg_val - pll_const->r1_min + (PLL_2065_R1_STEP >> 1)) / PLL_2065_R1_STEP;
	exact_val = pll_const->r1_min + reg_val * PLL_2065_R1_STEP;
	reg_val2 = (exact_val * pll_const->r2_ref + (pll_const->r1_ref >> 1)) / pll_const->r1_ref;
	reg_val2 = (reg_val2 - pll_const->r2_min + (PLL_2065_R2_STEP >> 1)) / PLL_2065_R2_STEP;
	phy_utils_mod_radioreg(pi, RADIO_2065_RFPLL_R2_R1, 0x3f3f, (reg_val2 << 8) | reg_val);

	/* lf_r3 ==> spreadsheet/tcl e70 */
	reg_val = (exact_val * pll_const->r3_ref + (pll_const->r1_ref >> 1)) / pll_const->r1_ref;
	reg_val = (reg_val - pll_const->r3_min + (PLL_2065_R3_STEP >> 1)) / PLL_2065_R3_STEP;
	phy_utils_mod_radioreg(pi, RADIO_2065_RFPLL_R3, 0x3f, (uint16) reg_val);

	/* kpd_upscale ==> spreadsheet/tcl e74 */
	a_fact = (((lf_c1_des * loop_bw * loop_bw) << 4) + (PLL_2065_VAR2_INV >> 1)) /
		PLL_2065_VAR2_INV;
	kvco_lo = (pll_const->fvco_lo << 16) / ((pll_const->fvco_lo + pll_const->fvco_hi) >> 1);
	tmp = (kvco_lo * kvco_lo) >> 16;
	kvco_lo = (tmp * kvco_lo) >> 14;
	kvco_lo = (pll_const->kvco_ref * kvco_lo) >> 4;
	kvco_hi = (pll_const->fvco_hi << 15) / ((pll_const->fvco_lo + pll_const->fvco_hi) >> 1);
	tmp = (kvco_hi * kvco_hi) >> 14;
	kvco_hi = (tmp * kvco_hi) >> 13;
	kvco_hi = (pll_const->kvco_ref * kvco_hi) >> 4;
	kvco_des = (kvco_hi - kvco_lo) * (vco_freq / PLL_2065_VCO_DIVFACT - pll_const->fvco_lo) /
		(pll_const->fvco_hi - pll_const->fvco_lo) + kvco_lo;
	if (RADIOID(pi->pubpi.radioid) == BCM2067_ID) {
		cpd_cur_des = loop_bw > PLL_2065_VAR3 ?
			(((a_fact_init * int_part) << 15) + (kvco_des >> 1)) / kvco_des :
			(((a_fact * int_part) << 11) + (kvco_des >> 1)) / kvco_des;
		kpd_upscale = cpd_cur_des > (200 * 2) ? 1 : 0;
	} else {
		cpd_cur_des = (((a_fact * int_part) << 11) + (kvco_des >> 1)) / kvco_des;
		kpd_upscale = cpd_cur_des >= (130 * 2) ? 1 : 0;
	}
	phy_utils_mod_radioreg(pi, RADIO_2065_RFPLL_KPD, 0x20, kpd_upscale << 5);

	/* kpd_scale ==> spreadsheet/tcl e75 */
	tmp = (pll_const->kpd_step * (kpd_upscale * pll_const->kpd_scale + 1)) << 1;
	reg_val = (cpd_cur_des + (tmp >> 1)) / tmp;
	exact_val = pll_const->kpd_step * reg_val * (kpd_upscale * pll_const->kpd_scale + 1);
	reg_val = reg_val > 31 + PLL_2065_KPD_MIN ? 31 :
		reg_val < PLL_2065_KPD_MIN ? 0 : reg_val - PLL_2065_KPD_MIN;
	phy_utils_mod_radioreg(pi, RADIO_2065_RFPLL_KPD, 0x1f, (uint16) reg_val);

	/* cp_ioff_upscale ==> spreadsheet/tcl e76 */
	if (CHIPID(pi->sh->chip) == BCM4314_CHIP_ID || CHIPID(pi->sh->chip) == BCM43142_CHIP_ID)
		cp_ioff_des = 6 * (rfpll_doubler + 1);
	else if (CHIPID(pi->sh->chip) == BCM43143_CHIP_ID)
		cp_ioff_des = 6 + 18 * rfpll_doubler;
	else
		cp_ioff_des = 5 + 8 * rfpll_doubler;
	cp_ioff_des = (cp_ioff_des * exact_val * (ref_freq / 1000) * PLL_2065_VCO_DIVFACT / 10 +
		(vco_freq >> 1)) / vco_freq;
	if (RADIOID(pi->pubpi.radioid) == BCM2067_ID)
		reg_val = cp_ioff_des >= (180 * 100) ? 3 :
			cp_ioff_des > (120 * 100) ? 2 : cp_ioff_des > (60 * 100) ? 1 : 0;
	else
		reg_val = cp_ioff_des > (20 * 100) ? 1 : 0;
	phy_utils_mod_radioreg(pi, RADIO_2065_RFPLL_CP_IDAC, 0x200, ((1 << reg_val) - 1) << 9);

	/* cp_ioff ==> spreadsheet/tcl e77 */
	if ((cp_ioff_des) % (pll_const->ioff_step * (reg_val * pll_const->ioff_scale + 10)))
		reg_val = (cp_ioff_des) /
			(pll_const->ioff_step * (reg_val * pll_const->ioff_scale + 10)) + 1;
	else
		reg_val = (cp_ioff_des) /
			(pll_const->ioff_step * (reg_val * pll_const->ioff_scale + 10));
	phy_utils_mod_radioreg(pi, RADIO_2065_RFPLL_CP_IDAC,
	                       0x1f0, (reg_val > 31 ? 31 : reg_val) << 4);

	wlc_2065_vco_cal(pi, TRUE);

	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		phy_utils_or_radioreg(pi, RADIO_2065_OVR3, 0x400);
		phy_utils_mod_radioreg(pi, RADIO_2065_LOGEN2G_CFG1, 0x3300,
			(chi->logen1 << 8) | (chi->logen2 << 12));
		phy_utils_mod_radioreg(pi, RADIO_2065_LNA2G_TUNE, 0xfff,
			(chi->lna_freq1) | (chi->lna_freq2 << 4) | (chi->lna_tx << 8));
		phy_utils_mod_radioreg(pi, RADIO_2065_TXMIX2G_CFG1, 0xf00, chi->txmix << 8);
		phy_utils_mod_radioreg(pi, RADIO_2065_PGA2G_CFG2, 0x7, chi->pga);
		phy_utils_mod_radioreg(pi, RADIO_2065_PAD2G_TUNE, 0x7, chi->pad);
	}
#ifdef BAND5G
	else {
		phy_utils_and_radioreg(pi, RADIO_2065_OVR3, ~0x400);
		phy_utils_mod_radioreg(pi, RADIO_2065_LOGEN5G_TUNE, 0xfff,
			(chi->logen1) | (chi->logen2 << 4) | (chi->logen3 << 8));
		phy_utils_mod_radioreg(pi, RADIO_2065_LNA5G_TUNE, 0xfff,
			(chi->lna_freq1) | (chi->lna_freq2 << 4) | (chi->lna_tx << 8));
		phy_utils_mod_radioreg(pi, RADIO_2065_TXMIX5G_CFG1, 0xf00, chi->txmix << 8);
		phy_utils_mod_radioreg(pi, RADIO_2065_PGA5G_CFG2, 0xf000, chi->pga << 12);
		phy_utils_mod_radioreg(pi, RADIO_2065_PAD5G_TUNE, 0xf, chi->pad);
	}
#endif // endif
}

static void
wlc_2065_vco_cal(phy_info_t *pi, bool legacy)
{
	if (legacy) {
		phy_utils_mod_radioreg(pi, RADIO_2065_RFPLL_CAL_CFG1, 0x10, 0);
	} else {
		phy_utils_mod_radioreg(pi, RADIO_2065_RFPLL_CAL_CFG1, 0x10, 1 << 4);
	}

	PHY_REG_LIST_START
		RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CAL_OVR_COUNT, 1, 1)
		RADIO_REG_MOD_ENTRY(RADIO_2065_OVR10, 0x8210, 0x8210)

		RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CFG1, 0x40, 0)
		RADIO_REG_MOD_ENTRY(RADIO_2065_RFPLL_CAL_CFG1, 0x3, 0)
	PHY_REG_LIST_EXECUTE(pi);

	OSL_DELAY(20);
	phy_utils_mod_radioreg(pi, RADIO_2065_RFPLL_CFG1, 0x40, 1 << 6);
	OSL_DELAY(20);
	phy_utils_mod_radioreg(pi, RADIO_2065_RFPLL_CAL_CFG1, 1, 1);
	OSL_DELAY(20);
	phy_utils_mod_radioreg(pi, RADIO_2065_RFPLL_CAL_CFG1, 2, 1 << 1);
	/* Remove 100us delay to save time, and will check vco done later */

}

static void
BCMATTACHFN(wlc_lcn40phy_decode_aa2g)(phy_info_t *pi, uint8 val)
{
	switch (val)
	{
		case 1:
			pi->sh->rx_antdiv = 0;
		break;
		case 2:
			pi->sh->rx_antdiv = 1;
		break;
		case 3:
			pi->sh->rx_antdiv = 3;
		break;
		default:
			PHY_ERROR(("wl%d: %s: AA2G = %d is Unsupported\n",
				pi->sh->unit, __FUNCTION__, val));
			ASSERT(0);

		break;

	}
	return;
}

/* Estimate power in dBm using tssi.
*   1. if a tone is used the pwrctrl block is forced to sample tssi in tempsense mode,
*   2. if a packet is used then average tssi is read.
*/
static int32
wlc_lcn40phy_get_signal_power(phy_info_t *pi, bool tone)
{
	int32 a1 = 0, b0 = 0, b1 = 0;
	int32 estPwr;
	uint16 signal_tssi, adjusted_tssi, tssi_idx, idle_tssi;

	ASSERT(PHY_EPA_SUPPORT(wlc_phy_getlcnphy_common(pi)->ePA));

	if (tone == TRUE)
		signal_tssi =
		(phy_utils_read_phyreg(pi, LCN40PHY_TxPwrCtrlStatusTemp) & 0x1ff) ^ 0x100;
	else
		signal_tssi =
		(phy_utils_read_phyreg(pi, LCN40PHY_TxPwrCtrlStatusNew4) & 0x1ff) ^ 0x100;

	idle_tssi =
		(PHY_REG_READ(pi, LCN40PHY, TxPwrCtrlIdleTssi, idleTssi0) ^ 0x100);
	adjusted_tssi = signal_tssi - idle_tssi + 511;
	tssi_idx = adjusted_tssi >> 2;
	wlc_phy_get_paparams_for_band(pi, &a1, &b0, &b1);
	estPwr = wlc_lcnphy_tssi2dbm(tssi_idx, a1, b0, b1);

	return (estPwr);

}

static void
wlc_lcn40phy_tssi_loopback(phy_info_t *pi, bool tone)
{
	uint16 auxpga_vmid, auxpga_gain = 0;
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);

	PHY_REG_LIST_START
		/* force tx on, rx off */
		PHY_REG_MOD_ENTRY(LCN40PHY, RFOverride0, internalrftxpu_ovr, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, RFOverrideVal0, internalrftxpu_ovr_val, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, RFOverride0, internalrfrxpu_ovr, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, RFOverrideVal0, internalrfrxpu_ovr_val, 0)

		/* Enable clk */
		PHY_REG_MOD_ENTRY(LCN40PHY, ClkEnCtrl, forcerxfrontendclk, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, AfeCtrlOvr1Val, afe_iqadc_aux_en_ovr_val, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, AfeCtrlOvr1, afe_iqadc_aux_en_ovr, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, sslpnCalibClkEnCtrl, forceTxfiltClkOn, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, sslpnCalibClkEnCtrl, forceaphytxFeclkOn, 1)

		PHY_REG_MOD_ENTRY(LCN40PHY, AfeCtrlOvr1, dac_pu_ovr, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, AfeCtrlOvr1Val, dac_pu_ovr_val, 1)

		PHY_REG_MOD_ENTRY(LCN40PHY, AfeCtrlOvr1, adc_pu_ovr, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, AfeCtrlOvr1Val, adc_pu_ovr_val, 0x1f)
	PHY_REG_LIST_EXECUTE(pi);

	wlc_lcn40phy_set_tssi_mux(pi, LCN40PHY_TSSI_EXT);

	if (CHSPEC_IS20(pi->radio_chanspec)) {
		PHY_REG_LIST_START
			PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlNum_Vbat, Nvbat_delay, 64)
			PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlNum_Vbat, Nvbat_intg_log2, 6)
			PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlNum_temp, Ntemp_delay, 64)
			PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlNum_temp, Ntemp_intg_log2, 6)
		PHY_REG_LIST_EXECUTE(pi);
	} else {
		PHY_REG_LIST_START
			PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlNum_Vbat, Nvbat_delay, 128)
			PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlNum_Vbat, Nvbat_intg_log2, 6)
			PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlNum_temp, Ntemp_delay, 128)
			PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlNum_temp, Ntemp_intg_log2, 6)
		PHY_REG_LIST_EXECUTE(pi);
	}

	PHY_REG_LIST_START
		PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlCmd, txPwrCtrl_swap_iq, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlCmd, txPwrCtrl_en, 0)
		PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlCmd, hwtxPwrCtrl_en, 0)
		PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlCmd, use_txPwrCtrlCoefs, 0)
		PHY_REG_MOD_ENTRY(LCN40PHY, papd_control2, papd_analog_gain_ovr, 1)

		PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlIdleTssi, rawTssiOffsetBinFormat, 1)

		/* Select TSSI as auxPGA input */
		PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlRfCtrlOverride0, amuxSelPortOverride, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlRfCtrlOverride0, amuxSelPortOverrideVal, 2)
	PHY_REG_LIST_EXECUTE(pi);

	/* Only needed for internal tssi path;
	  * Power up envelope detector and bias circuit
	  */

	if (!PHY_EPA_SUPPORT(wlc_phy_getlcnphy_common(pi)->ePA)) {
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			phy_utils_or_radioreg(pi, RADIO_2065_TX2G_TSSI, 1);
			phy_utils_or_radioreg(pi, RADIO_2065_OVR12, 0x2);
		}
#ifdef BAND5G
		else {
			phy_utils_or_radioreg(pi, RADIO_2065_TX5G_TSSI, 1);
			if (LCN40REV_IS(pi->pubpi.phy_rev, 7))
				phy_utils_or_radioreg(pi, RADIO_2065_OVR13_ACCOREREG, 0x100);
			else
				phy_utils_or_radioreg(pi, RADIO_2065_OVR13, 0x800);
		}
#endif /* BAND5G */

		PHY_REG_LIST_START
		/* Power up tssi path after envelope detector, only needed for internal tssi */
			RADIO_REG_OR_ENTRY(RADIO_2065_OVR2, 0x2)
			RADIO_REG_OR_ENTRY(RADIO_2065_IQCAL_CFG1, 1)

			/* increase envelope detector gain */
			PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlRfCtrlOverride0, paCtrlTssiOverride, 1)
			PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlRfCtrlOverride0, paCtrlTssiOverrideVal,
				1)
		PHY_REG_LIST_EXECUTE(pi);
		if (LCN40REV_IS(pi->pubpi.phy_rev, 7))
			phy_utils_mod_radioreg(pi, RADIO_2065_TX5G_TSSI, 0xf0, 0x0);
	}
	PHY_REG_LIST_START
		/* Power up AMUX (a.k.a. testbuf) */
		RADIO_REG_OR_ENTRY(RADIO_2065_OVR16, 1)
		RADIO_REG_OR_ENTRY(RADIO_2065_TESTBUF_CFG1, 1)

		/* Power up AUX PGA */
		RADIO_REG_OR_ENTRY(RADIO_2065_OVR1, 0x8000)
		RADIO_REG_OR_ENTRY(RADIO_2065_AUXPGA_CFG1, 1)

		/* Power up RX buffer
		*   lpf_rxbuf_pu in AMS
		*  TSSI configuration in AMS
		*/
		RADIO_REG_OR_ENTRY(RADIO_2065_LPF_CFG1, 0xCC0)
		RADIO_REG_OR_ENTRY(RADIO_2065_OVR7, 0xC4)

		/* sel_byp_txlpf in AMS */
		RADIO_REG_MOD_ENTRY(RADIO_2065_OVR7, 0x10, 1 << 4)
	PHY_REG_LIST_EXECUTE(pi);
	if (pi->tx_alpf_bypass)
		phy_utils_mod_radioreg(pi, RADIO_2065_LPF_CFG1, 0x200, 1 << 9);
	else
		phy_utils_mod_radioreg(pi, RADIO_2065_LPF_CFG1, 0x200, 0 << 9);

	PHY_REG_LIST_START
		/* Connect Rx Buffer to ADC */
		RADIO_REG_OR_ENTRY(RADIO_2065_OVR7, 1<<5)
		RADIO_REG_MOD_ENTRY(RADIO_2065_LPF_CFG1, 0xc000, 1<<14)

		/* Connect AuxPGA to Rx Buffer */
		RADIO_REG_OR_ENTRY(RADIO_2065_OVR7, 1<<3)
		RADIO_REG_MOD_ENTRY(RADIO_2065_LPF_CFG1, 0x3000, 2<<12)
	PHY_REG_LIST_EXECUTE(pi);

#ifdef BAND5G
	if (CHSPEC_IS5G(pi->radio_chanspec)) {
		auxpga_vmid = (pi_lcn->rssismc5g << 4) | pi_lcn->rssismf5g;
		auxpga_gain = pi_lcn->rssisav5g;
	} else
#endif // endif
	{
		auxpga_gain = pi_lcn->lcnphy_rssi_gs;
		auxpga_vmid = (pi_lcn->lcnphy_rssi_vc << 4) |
			pi_lcn->lcnphy_rssi_vf;
	}

	phy_utils_or_radioreg(pi, RADIO_2065_OVR1, 0x6000);
	phy_utils_mod_radioreg(pi, RADIO_2065_AUXPGA_VMID, 0x3FF, auxpga_vmid);
	phy_utils_mod_radioreg(pi, RADIO_2065_AUXPGA_CFG1, 0x700, auxpga_gain << 8);

	PHY_REG_LIST_START
		/* set rxbufgain to zero and enable override */
		RADIO_REG_MOD_ENTRY(RADIO_2065_OVR7, (1<<8), 1 << 8)
		RADIO_REG_MOD_ENTRY(RADIO_2065_LPF_GAIN, (0xf<<8), 0)
		RADIO_REG_MOD_ENTRY(RADIO_2065_IQCAL_IDAC, 0xf, 8)
	PHY_REG_LIST_EXECUTE(pi);
}

static uint8
wlc_lcn40phy_papd_calidx_estimate(phy_info_t *pi, int32 target_pwr_dbm,
	int32 base_pwr_dbm, bool tone)
{
	bool suspend;
	int j;
	uint16 SAVE_txpwrctrl_state;
	uint16 SAVE_upconv = 0;
	bool SAVE_papd;
	uint16 SAVE_papr_iir_group_dly;
	uint16 values_to_save_phy[ARRAYSIZE(papd_calidx_estimate_phy_reg)];
	uint16 values_to_save_rf[ARRAYSIZE(papd_calidx_estimate_rf_reg)];
	int tmp_tx_idx, final_idx, step_size;
	int32 sig_pwr_dbm;
	uint8 ret_idx;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	/* Turn off all the crs signals to the MAC */
	wlc_lcn40phy_deaf_mode(pi, TRUE);

	suspend = !(R_REG(pi->sh->osh, &pi->regs->maccontrol) & MCTL_EN_MAC);
	if (!suspend)
		wlapi_suspend_mac_and_wait(pi->sh->physhim);

	/* Save the phy and rf regs */
	SAVE_txpwrctrl_state = wlc_lcn40phy_get_tx_pwr_ctrl(pi);
	SAVE_papd = wlc_lcn40phy_is_papd_block_enable(pi);
	SAVE_papr_iir_group_dly = phy_utils_read_phyreg(pi, LCN40PHY_papr_iir_group_dly);
	/* Save all PHY regs that are changed later on */
	for (j = 0; j < ARRAYSIZE(papd_calidx_estimate_phy_reg); j++)
		values_to_save_phy[j] = phy_utils_read_phyreg(pi, papd_calidx_estimate_phy_reg[j]);
	/* Save all rf regs that are changed later on */
	for (j = 0; j < ARRAYSIZE(papd_calidx_estimate_rf_reg); j++)
		values_to_save_rf[j] = phy_utils_read_radioreg(pi, papd_calidx_estimate_rf_reg[j]);

	/* Turn off automatic power control */
	wlc_lcn40phy_set_tx_pwr_ctrl(pi, LCN40PHY_TX_PWR_CTRL_OFF);

	/* Disable PAPD */
	wlc_lcn40phy_papd_block_enable(pi, FALSE);

	/* Disable PAPRR block */
	PHY_REG_MOD(pi, LCN40PHY, papr_iir_group_dly, papr_override_enable, 1);
	PHY_REG_MOD(pi, LCN40PHY, papr_iir_group_dly, papr_enable, 0);

	if (LCN40REV_GE(pi->pubpi.phy_rev, 4) && CHSPEC_IS40(pi->radio_chanspec)) {
		SAVE_upconv = phy_utils_read_phyreg(pi, LCN40PHY_upconv);
		/* In 40MHz bw mode,
		* this override will disable the -10MHz shift in the +/-10MHz upconversion block.
		*/
		PHY_REG_MOD(pi, LCN40PHY, upconv, bw20in40_override, 1);
		PHY_REG_MOD(pi, LCN40PHY, upconv, bw20in40_val, 0);
	}

	PHY_REG_LIST_START
		/* WAR to force digi_gain */
		PHY_REG_MOD_ENTRY(LCN40PHY, radioCtrl, digi_gain_ovr_val, 0)
		PHY_REG_MOD_ENTRY(LCN40PHY, radioCtrl, digi_gain_ovr, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, agcControl4, c_agc_fsm_en, 0)
		PHY_REG_MOD_ENTRY(LCN40PHY, agcControl4, c_agc_fsm_en, 1)
	PHY_REG_LIST_EXECUTE(pi);
	OSL_DELAY(2);
	PHY_REG_MOD(pi, LCN40PHY, agcControl4, c_agc_fsm_en, 0);

	/* Init tx index */
	tmp_tx_idx = 90;
	final_idx = tmp_tx_idx;
	wlc_lcn40phy_set_tx_pwr_by_index(pi, tmp_tx_idx);

	wlc_btcx_override_enable(pi);

	if (tone == TRUE) {
		/* Set up tone tssi loopback */
		wlc_lcn40phy_tssi_loopback(pi, tone);

		if (CHSPEC_IS5G(pi->radio_chanspec)) {
			if (CHSPEC_IS20(pi->radio_chanspec))
				wlc_lcn40phy_start_tx_tone(pi, (2000 * 1000),
					98, 0);
			else
				wlc_lcn40phy_start_tx_tone(pi, (4000 * 1000),
					55, 0);
		} else {
			if (CHSPEC_IS20(pi->radio_chanspec))
				wlc_lcn40phy_start_tx_tone(pi, (2000 * 1000),
					115, 0);
			else
				wlc_lcn40phy_start_tx_tone(pi, (4000 * 1000),
					64, 0);
		}
	} else {
		PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlNnum, Ntssi_intg_log2, 0);
	}

	/* Search for the idx closest to base_pwr_dbm */
	for (j = 0; j < 15; j++) {
		OSL_DELAY(10); /* delay required */
		if (tone == TRUE)
			PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlRangeCmd,
			force_vbatTemp, 1);
		else {
			PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlCmd,
			txPwrCtrl_en, 1);
			wlc_phy_do_dummy_tx(pi, TRUE, OFF);
		}

		OSL_DELAY(50); /* delay required */
		sig_pwr_dbm =
			wlc_lcn40phy_get_signal_power(pi, tone);
		step_size = (sig_pwr_dbm - base_pwr_dbm + 1) >> 1;
		if (ABS(sig_pwr_dbm - base_pwr_dbm) <= 1)
			break;
		tmp_tx_idx += step_size;

		if (tmp_tx_idx > 127)
			tmp_tx_idx = 127;
		else if (tmp_tx_idx < 0)
			tmp_tx_idx = 0;

		wlc_lcn40phy_set_tx_pwr_by_index(pi, tmp_tx_idx);
	}

	if (tone == TRUE)
		wlc_lcn40phy_stop_tx_tone(pi);

		wlc_phy_btcx_override_disable(pi);

	/* Assume linear curve and estimate
	  * the idx for target_pwr_dbm
	  */
	step_size = (sig_pwr_dbm - target_pwr_dbm + 1) >> 1;
	final_idx = tmp_tx_idx + step_size;

	if (final_idx > 127)
		ret_idx = 127;
	else if (final_idx < 0)
		ret_idx = 0;
	else
		ret_idx = (uint8) final_idx;

	PHY_INFORM(("Closest idx for %d dBm = %d (loopcnt %d)\n", base_pwr_dbm, tmp_tx_idx, j));
	PHY_INFORM(("Estimated idx for %d dBm = %d\n", target_pwr_dbm, final_idx));

	/* Restore RF Registers */
	for (j = 0; j < ARRAYSIZE(papd_calidx_estimate_rf_reg); j++) {
		phy_utils_write_radioreg(pi, papd_calidx_estimate_rf_reg[j], values_to_save_rf[j]);
	}
	/* Restore PHY Registers */
	for (j = 0; j < ARRAYSIZE(papd_calidx_estimate_phy_reg); j++) {
		phy_utils_write_phyreg(pi, papd_calidx_estimate_phy_reg[j], values_to_save_phy[j]);
	}
	if (LCN40REV_GE(pi->pubpi.phy_rev, 4) && CHSPEC_IS40(pi->radio_chanspec)) {
		phy_utils_write_phyreg(pi, LCN40PHY_upconv, SAVE_upconv);
	}
	wlc_lcn40phy_set_tx_pwr_ctrl(pi, SAVE_txpwrctrl_state);
	if (SAVE_papd)
		wlc_lcn40phy_papd_block_enable(pi, TRUE);
	phy_utils_write_phyreg(pi, LCN40PHY_papr_iir_group_dly, SAVE_papr_iir_group_dly);

	if (!suspend)
		wlapi_enable_mac(pi->sh->physhim);

	wlc_lcn40phy_deaf_mode(pi, FALSE);

	return (ret_idx);
}

static uint8
wlc_lcn40phy_get_calidxest_params_for_band(phy_info_t *pi)
{
	phy_info_lcn40phy_t *pi_lcn40 = pi->u.pi_lcn40phy;

	pi_lcn40->target_pwr_dbm = -1;

	switch (wlc_phy_chanspec_bandrange_get(pi, pi->radio_chanspec)) {
		case WL_CHAN_FREQ_RANGE_2G:
			pi_lcn40->base_pwr_dbm = pi_lcn40->calidxestbase2g;
			pi_lcn40->target_pwr_dbm = pi_lcn40->calidxesttarget2g;
			break;
#ifdef BAND5G
		case WL_CHAN_FREQ_RANGE_5GL:
			/* 5 GHz low */
			if (CHSPEC_IS40(pi->radio_chanspec))
				pi_lcn40->target_pwr_dbm = pi_lcn40->calidxesttarget40lo5g;
			else
				pi_lcn40->target_pwr_dbm = pi_lcn40->calidxesttargetlo5g;
			break;
		case WL_CHAN_FREQ_RANGE_5GH:
			/* 5 GHz high */
			if (CHSPEC_IS40(pi->radio_chanspec))
				pi_lcn40->target_pwr_dbm = pi_lcn40->calidxesttarget40hi5g;
			else
				pi_lcn40->target_pwr_dbm = pi_lcn40->calidxesttargethi5g;
			break;
#endif /* BAND5G */
		default:
			ASSERT(FALSE);
			break;
	}

	if (CHSPEC_IS5G(pi->radio_chanspec)) {
		pi_lcn40->base_pwr_dbm = pi_lcn40->calidxestbase5g;
		if (pi_lcn40->target_pwr_dbm == -1) {
			pi_lcn40->target_pwr_dbm = pi_lcn40->calidxesttarget5g;
			if (CHSPEC_IS40(pi->radio_chanspec) &&
				pi_lcn40->calidxesttarget405g != -1)
				pi_lcn40->target_pwr_dbm = pi_lcn40->calidxesttarget405g;
		}
	}

	if (pi_lcn40->target_pwr_dbm != -1)
		return (1);
	else
		return (0);
}

/* periodic cal does tx iqlo cal, rx iq cal, (tempcompensated txpwrctrl for P200 4313A0 board) */
static void
wlc_lcn40phy_periodic_cal(phy_info_t *pi)
{
	bool suspend;
	phy_txcalgains_t txgains;
	uint16 SAVE_pwrctrl;
	uint16 SAVE_dlo_coeff_di_cck = 0, SAVE_dlo_coeff_dq_cck = 0;
	int8 indx;
	phy_info_lcnphy_t *pi_lcn;
	phy_info_lcn40phy_t *pi_lcn40;
	int btc_mode = wlapi_bmac_btc_mode_get(pi->sh->physhim);
#if defined(PHYCAL_CACHING)
	ch_calcache_t *ctx;
	lcnphy_calcache_t *cache = NULL;
#endif // endif
	phy_utils_phyreg_enter(pi);

	pi_lcn = wlc_phy_getlcnphy_common(pi);
	pi_lcn40 = pi->u.pi_lcn40phy;
#if defined(PHYCAL_CACHING)
	ctx = wlc_phy_get_chanctx(pi, pi->radio_chanspec);

	if (!ctx) {
		if (!wlc_phy_no_cal_possible(pi)) {
			/* Fresh calibration or restoration required */
			if (LCN40PHY_MAX_CAL_CACHE <= pi->phy_calcache_num) {
				/* Already max num ctx exist, reuse oldest */
				ctx = wlc_phy_get_chanctx_oldest(pi);
				ASSERT(ctx);
				wlc_phy_reinit_chanctx(pi, ctx,  pi->radio_chanspec);
			} else {
				/* Prepare a fresh calibration context */
				if (BCME_OK == wlc_phy_create_chanctx((wlc_phy_t *)pi,
					pi->radio_chanspec)) {
					ctx = pi->phy_calcache;
				}
			}
		}
	}
	if (ctx)
		cache = &ctx->u.lcnphy_cache;
#endif /* PHYCAL_CACHING */

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	if (NORADIO_ENAB(pi->pubpi)) {
		phy_utils_phyreg_exit(pi);
		return;
	}

#if defined(PHYCAL_CACHING)
	if (ctx) {
		ctx->cal_info.last_cal_time = pi->sh->now;
		ctx->cal_info.last_cal_temp = pi_lcn->lcnphy_rawtempsense;
	}
#else
	pi->phy_lastcal = pi->sh->now;
	pi_lcn->lcnphy_full_cal_channel = CHSPEC_CHANNEL(pi->radio_chanspec);
#endif // endif

	pi->phy_forcecal = FALSE;
	suspend = !(R_REG(pi->sh->osh, &pi->regs->maccontrol) & MCTL_EN_MAC);
	if (!suspend) {
		/* Set non-zero duration for CTS-to-self */
		wlapi_bmac_write_shm(pi->sh->physhim, M_CTS_DURATION, 10000);
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
	}

	SAVE_pwrctrl = wlc_lcn40phy_get_tx_pwr_ctrl(pi);

	if (LCN40REV_IS(pi->pubpi.phy_rev, 5))
		wlc_lcn40phy_load_clear_papr_tbls(pi, FALSE);

	wlc_lcn40phy_deaf_mode(pi, TRUE);

	/* Force vbat to updated the cached tempsense values */
	wlc_lcn40phy_tempsense(pi, TEMPER_VBAT_TRIGGER_NEW_MEAS);

	wlc_lcn40phy_txpwrtbl_iqlo_cal(pi);

	if (LCN40REV_IS(pi->pubpi.phy_rev, 2) ||
		(LCN40REV_GT(pi->pubpi.phy_rev, 3) && !LCN40REV_IS(pi->pubpi.phy_rev, 6))) {
		/* In case Tx ALPF is bypassed and powered down,
		  * need to power it up here (required for rx iqcal)
		  */
		if (pi->tx_alpf_bypass) {
			PHY_REG_LIST_START
				/* don't bypass Tx LPF */
				PHY_REG_MOD_ENTRY(LCN40PHY, radio_tx_cfg_pu, tx_lpf_byp_tx_ofdm, 0)
				PHY_REG_MOD_ENTRY(LCN40PHY, radio_tx_cfg_pu, tx_lpf_byp_tx_cck, 0)
				/* Power up TX LPF */
				PHY_REG_MOD_ENTRY(LCN40PHY, radio_tx_cfg_pu, tx_lpf_pu, 3)
			PHY_REG_LIST_EXECUTE(pi);
		}

		if (CHSPEC_IS40(pi->radio_chanspec) && LCN40REV_GT(pi->pubpi.phy_rev, 3)) {
			/* In 40MHz bw mode, this override will disable
			the -10MHz shift in the +/-10MHz upconversion block.
			*/
			PHY_REG_MOD(pi, LCN40PHY, upconv, bw20in40_override, 1);
			PHY_REG_MOD(pi, LCN40PHY, upconv, bw20in40_val, 0);
		}

		wlc_lcn40phy_rx_iq_cal(pi, NULL, 0, TRUE, FALSE, 1, 20);

		if (CHSPEC_IS40(pi->radio_chanspec) && LCN40REV_GT(pi->pubpi.phy_rev, 3))
			PHY_REG_MOD(pi, LCN40PHY, upconv, bw20in40_override, 0);

		if (pi->tx_alpf_bypass) {
		PHY_REG_MOD(pi, LCN40PHY, radio_tx_cfg_pu, tx_lpf_byp_tx_ofdm, 1);
			if ((LCN40REV_IS(pi->pubpi.phy_rev, 7)) &&
				(RADIOVER(pi->pubpi.radiover) == 0x4)) {
				PHY_REG_MOD(pi, LCN40PHY, radio_tx_cfg_pu, tx_lpf_byp_tx_cck, 0);
			} else {
				/* Don't bypass Tx LPF */
				PHY_REG_MOD(pi, LCN40PHY, radio_tx_cfg_pu, tx_lpf_byp_tx_cck, 1);
			}
			/* Power up TX LPF */
			if (pi_lcn40->tx_alpf_pu)
				PHY_REG_MOD(pi, LCN40PHY, radio_tx_cfg_pu, tx_lpf_pu, 3);
			else
				PHY_REG_MOD(pi, LCN40PHY, radio_tx_cfg_pu, tx_lpf_pu, 0);
		}
	}

#if IDLE_TSSI_PER_CAL_EN
	/* Idle TSSI Estimate */
	if (!(LCN40REV_GE(pi->pubpi.phy_rev, 4) &&
		(pi_lcn->lcnphy_tssical_time)))
		wlc_lcn40phy_idle_tssi_est(pi);
#endif /* #if IDLE_TSSI_PER_CAL_EN */
	/* PAPD cal */

	/* WAR to disable existing pu settings from radio-ovr.
	  * This allows the following
	  * to function properly,
	  * 1. phy-ovr,
	  * 2. direct-control
	  */
	phy_utils_mod_radioreg(pi, RADIO_2065_TXRX2G_CAL, 0x3, 0);
	phy_utils_mod_radioreg(pi, RADIO_2065_TXRX5G_CAL, 0x3, 0);

	/* Default PAPD settings */
	pi_lcn40->papd_num_symbols = 219;
	pi_lcn40->papd_stop_after_last_update = PAPD_STOP_AFTER_LAST_UPDATE;

	if (PHY_PAPD_ENABLE(pi_lcn40->papd_enable) == TRUE) {
		indx = wlc_lcn40phy_get_current_tx_pwr_idx(pi);

		pi_lcn40->do_papd_calidx_est =
			wlc_lcn40phy_get_calidxest_params_for_band(pi);

		if (LCN40_LINPATH(pi_lcn40->papd_lin_path) == LCN40PHY_PAPDLIN_IPA) {
			PHY_PAPD(("--- PAPD iPA CAL\n"));
			if (LCN40REV_IS(pi->pubpi.phy_rev, 6) ||
				(LCN40REV_IS(pi->pubpi.phy_rev, 7) &&
				(RADIOVER(pi->pubpi.radiover) == 0x4) &&
				(CHSPEC_IS2G(pi->radio_chanspec)))) {
				SAVE_dlo_coeff_di_cck = PHY_REG_READ(pi, LCN40PHY,
					dlo_coeff_di_cck, dlo_coeff_di_cck);
				SAVE_dlo_coeff_dq_cck = PHY_REG_READ(pi, LCN40PHY,
					dlo_coeff_dq_cck, dlo_coeff_dq_cck);
				PHY_REG_MOD(pi, LCN40PHY, dlo_coeff_di_cck,
					dlo_coeff_di_cck, (PHY_REG_READ(pi, LCN40PHY,
					dlo_coeff_di_ofdm, dlo_coeff_di_ofdm)));
				PHY_REG_MOD(pi, LCN40PHY, dlo_coeff_dq_cck,
					dlo_coeff_dq_cck, (PHY_REG_READ(pi, LCN40PHY,
					dlo_coeff_dq_ofdm, dlo_coeff_dq_ofdm)));
			}
			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				if (CHIPID(pi->sh->chip) == BCM43143_CHIP_ID)
					txgains.index = 30;
				else
					txgains.index = 45;
				if (LCN40REV_IS(pi->pubpi.phy_rev, 7)) {
					pi_lcn40->papd_lut_begin = 4460;
					pi_lcn40->papd_lut_step =  560;
					pi_lcn40->papd_bbmult_init_bw20 = 1000;
					pi_lcn40->papd_bbmult_step_bw20 = 16589;
					pi_lcn40->papd_bbmult_init_bw40 = 800;
					pi_lcn40->papd_bbmult_step_bw40 = 16589;
				} else if (LCN40REV_IS(pi->pubpi.phy_rev, 6)) {
					pi_lcn40->papd_lut_begin = 6300;
					pi_lcn40->papd_lut_step =  1408;
					pi_lcn40->papd_bbmult_init_bw20 = 1000;
					pi_lcn40->papd_bbmult_step_bw20 = 16589;
					pi_lcn40->papd_bbmult_init_bw40 = 1400;
					pi_lcn40->papd_bbmult_step_bw40 = 16589;
				} else {
					pi_lcn40->papd_lut_begin = 6350;
					pi_lcn40->papd_lut_step =  1280;
					pi_lcn40->papd_bbmult_init_bw20 = 1000;
					pi_lcn40->papd_bbmult_step_bw20 = 16589;
					pi_lcn40->papd_bbmult_init_bw40 = 1400;
					pi_lcn40->papd_bbmult_step_bw40 = 16589;
				}
			}
#ifdef BAND5G
			else {
				txgains.index = 55;
				if (CHSPEC_IS20(pi->radio_chanspec))
					pi_lcn40->papd_lut_begin = 4140;
				else
					pi_lcn40->papd_lut_begin = 4076;
				pi_lcn40->papd_lut_step =  560;
				pi_lcn40->papd_bbmult_init_bw20 = 1200;
				pi_lcn40->papd_bbmult_step_bw20 = 16589;
				pi_lcn40->papd_bbmult_init_bw40 = 1200;
				pi_lcn40->papd_bbmult_step_bw40 = 16589;
			}
#endif /* BAND5G */
			txgains.useindex = 1;

			/* set bbmult0 to default value */
			wlc_lcn40phy_papd_set_bbmult0(pi, 0);

			if (CHIPID(pi->sh->chip) == BCM43143_CHIP_ID) {
				if (CHSPEC_IS40(pi->radio_chanspec)) {
					wlc_lcn40phy_papd_idx_sel(pi, LCN40_AMAM_TARGET_40MHZ,
						&txgains, 3);
				} else {
					wlc_lcn40phy_papd_idx_sel(pi, LCN40_AMAM_TARGET_43143,
						&txgains, 3);
				}
			} else if (LCN40REV_IS(pi->pubpi.phy_rev, 7)) {
				if (CHSPEC_IS40(pi->radio_chanspec)) {
					wlc_lcn40phy_papd_idx_sel(pi, LCN40_AMAM_TARGET_40MHZ_43341,
						&txgains, 3);
					wlc_lcn40phy_papd_idx_sel(pi, LCN40_AMAM_TARGET_40MHZ_43341,
						&txgains, 1);
				} else {
					wlc_lcn40phy_papd_idx_sel(pi, LCN40_AMAM_TARGET_43341,
						&txgains, 3);
					wlc_lcn40phy_papd_idx_sel(pi, LCN40_AMAM_TARGET_43341,
						&txgains, 1);
				}
			} else {
				if (CHSPEC_IS40(pi->radio_chanspec)) {
					wlc_lcn40phy_papd_idx_sel(pi, LCN40_AMAM_TARGET_40MHZ,
						&txgains, 3);
				} else {
					wlc_lcn40phy_papd_idx_sel(pi, LCN40_AMAM_TARGET,
						&txgains, 3);
					wlc_lcn40phy_papd_idx_sel(pi, LCN40_AMAM_TARGET,
						&txgains, 1);
				}
			}
		} else if (LCN40_LINPATH(pi_lcn40->papd_lin_path) == LCN40PHY_PAPDLIN_EPA) {
			PHY_PAPD(("--- PAPD ePA CAL\n"));
			txgains.useindex = 1;

			/* PAPD reverse cal params */
			pi_lcn40->papd_num_symbols = 207;
			pi_lcn40->papd_bbmult_init_rev_bw20 = 15047;
			pi_lcn40->papd_bbmult_init_rev_bw40 = 15047;
			pi_lcn40->papd_bbmult_step_rev_bw20 = 16171;
			pi_lcn40->papd_bbmult_step_rev_bw40 = 16171;

			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				txgains.index = 46;
				pi_lcn40->papd_lut_begin = 6400;
				pi_lcn40->papd_lut_step =  1092;
				pi_lcn40->papd_bbmult_init_bw20 = 1000;
				pi_lcn40->papd_bbmult_step_bw20 = 16600;
				pi_lcn40->papd_bbmult_init_bw40 = 1000;
				pi_lcn40->papd_bbmult_step_bw40 = 16600;
			}
#ifdef BAND5G
			else {
				txgains.index = 39;
				pi_lcn40->papd_lut_begin = 6400;
				pi_lcn40->papd_lut_step =  1092;
				pi_lcn40->papd_bbmult_init_bw20 = 1000;
				pi_lcn40->papd_bbmult_step_bw20 = 16600;
				pi_lcn40->papd_bbmult_init_bw40 = 1000;
				pi_lcn40->papd_bbmult_step_bw40 = 16600;
			}
#endif /* BAND5G */
		} else if (LCN40_LINPATH(pi_lcn40->papd_lin_path) == LCN40PHY_PAPDLIN_PAD) {
			PHY_PAPD(("--- PAPD PAD CAL\n"));
			txgains.useindex = 1;

			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				txgains.index = 36;
				pi_lcn40->papd_lut_begin = 6600;
				pi_lcn40->papd_lut_step =  1280;
				pi_lcn40->papd_bbmult_init_bw20 = 1000;
				pi_lcn40->papd_bbmult_step_bw20 = 16589;
				pi_lcn40->papd_bbmult_init_bw40 = 1000;
				pi_lcn40->papd_bbmult_step_bw40 = 16589;
			}
#ifdef BAND5G
			else {
				txgains.index = 28;
				pi_lcn40->papd_lut_begin = 6600;
				pi_lcn40->papd_lut_step =  1280;
				pi_lcn40->papd_bbmult_init_bw20 = 1000;
				pi_lcn40->papd_bbmult_step_bw20 = 16589;
				pi_lcn40->papd_bbmult_init_bw40 = 1000;
				pi_lcn40->papd_bbmult_step_bw40 = 16589;
			}
#endif /* BAND5G */
		}

		if (pi_lcn40->do_papd_calidx_est) {
			/* If cal-idx-estimate is ENABLED then call setpwrctrl
			  * BEFORE papd cal since idle-tssi needs to be established first.
			  */
			txgains.index = wlc_lcn40phy_papd_calidx_estimate(pi,
				pi_lcn40->target_pwr_dbm, pi_lcn40->base_pwr_dbm, TRUE);
		} else if (CHSPEC_IS2G(pi->radio_chanspec) && pi_lcn->pacalidx_2g)
			txgains.index = pi_lcn->pacalidx_2g;
#ifdef BAND5G
		else if (CHSPEC_IS5G(pi->radio_chanspec) && pi_lcn->pacalidx_5g)
			txgains.index = pi_lcn->pacalidx_5g;
#endif /* BAND5G */

		PHY_PAPD(("CAL AT IDX = %d\n", txgains.index));
		wlc_lcn40phy_papd_cal(pi, PHY_PAPD_CAL_CW, &txgains, 0, 0, 0, 0,
			pi_lcn40->papd_num_symbols, 1, 0);

		if (CHIPID(pi->sh->chip) == BCM43142_CHIP_ID ||
			CHIPID(pi->sh->chip) == BCM43143_CHIP_ID)

			wlc_lcn40phy_papd_set_bbmult0(pi, 1);

		if (CHIPID(pi->sh->chip) == BCM43143_CHIP_ID && CHSPEC_IS40(pi->radio_chanspec)) {
			wlc_lcn40phy_papd_phase_jump_corr(pi);
		}

		wlc_lcn40phy_set_tx_pwr_by_index(pi, indx);
		if (LCN40REV_IS(pi->pubpi.phy_rev, 6) ||
			(LCN40REV_IS(pi->pubpi.phy_rev, 7) &&
			(RADIOVER(pi->pubpi.radiover) == 0x4) &&
			CHSPEC_IS2G(pi->radio_chanspec))) {
			PHY_REG_MOD(pi, LCN40PHY, dlo_coeff_di_cck,
				dlo_coeff_di_cck, SAVE_dlo_coeff_di_cck);
			PHY_REG_MOD(pi, LCN40PHY, dlo_coeff_dq_cck,
				dlo_coeff_dq_cck, SAVE_dlo_coeff_dq_cck);
		}
	} else {
		/* Explicitly disable PAPD */
		wlc_lcn40phy_papd_block_enable(pi, FALSE);
	}
	/* Convert tssi to power LUT */
	wlc_lcn40phy_set_estPwrLUT(pi, 0);
	PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlDeltaPwrLimit, DeltaPwrLimit, 0x10);

#if TWO_POWER_RANGE_TXPWR_CTRL
		if (pi_lcn->lcnphy_twopwr_txpwrctrl_en) {
			wlc_lcn40phy_set_estPwrLUT(pi, 1);
		}
#endif /* TWO_POWER_RANGE_TXPWR_CTRL */

	wlc_lcn40phy_set_tx_pwr_ctrl(pi, SAVE_pwrctrl);
	wlc_lcn40phy_deaf_mode(pi, FALSE);

	if (LCN40REV_IS(pi->pubpi.phy_rev, 5))
		wlc_lcn40phy_load_clear_papr_tbls(pi, TRUE);
	if ((btc_mode == WL_BTC_HYBRID) || (btc_mode == WL_BTC_LITE))
		wlc_btcx_override_enable(pi);
	if (!suspend)
		wlapi_enable_mac(pi->sh->physhim);

#if defined(PHYCAL_CACHING)
	if (ctx) {
		/* Cache the power index used for this channel */
		cache->lcnphy_gain_index_at_last_cal = wlc_lcnphy_get_current_tx_pwr_idx(pi);
		/* Already cached the Tx, Rx IQ and PAPD Cal results */
		ctx->valid = TRUE;
	}
#endif /* PHYCAL_CACHING */
	phy_utils_phyreg_exit(pi);
}

/* Back off tx gain index until max AM/AM is less than threshold */
static void wlc_lcn40phy_papd_idx_sel(phy_info_t *pi, int32 amam_cmp,
	phy_txcalgains_t *txgains, int8 step)
{
	phytbl_info_t tab;
	uint32 val;
	uint8 num_symbols = 25;
	int32 lut_i;
	int32 lut_j;
	int32 amam;
	int32 previous_difference;
	int16 tx_idx;
	int8 pacalalim = 0;
	uint8 gain_incr;
	tab.tbl_id = LCN40PHY_TBL_ID_PAPDCOMPDELTATBL;
	tab.tbl_ptr = &val; /* ptr to buf */
	tab.tbl_width = 32;
	tab.tbl_len = 1;
	tab.tbl_offset = 63;

	wlc_lcn40phy_papd_cal(pi, PHY_PAPD_CAL_CW, txgains, 0, 0, 0, 0, num_symbols,
		1, PAPD_COARSE_BBMULT_STEP);
	wlc_lcn40phy_read_table(pi, &tab);
	lut_i = (val & 0x00fff000) << 8;
	lut_j = (val & 0x00000fff) << 20;
	lut_i = lut_i >> 20;
	lut_j = lut_j >> 20;
	amam = ((lut_i*lut_i) + (lut_j*lut_j));
	amam_cmp *= amam_cmp;
	tx_idx = txgains->index;
	gain_incr = 0;
	if ((pacalalim == 0) && (amam < amam_cmp)) {
		step = -step;
		gain_incr = 1;
	}
	while (1) {
		tx_idx += step;
		if (tx_idx > 127) {
			tx_idx = 127;
			break;
		} else if (tx_idx < 0) {
			tx_idx = 0;
			break;
		}
		previous_difference = ABS(amam_cmp - amam);
		txgains->index = (uint8)tx_idx;
		wlc_lcn40phy_papd_cal(pi, PHY_PAPD_CAL_CW, txgains, 0, 0, 0, 0, num_symbols,
			1, PAPD_COARSE_BBMULT_STEP);
		wlc_lcn40phy_read_table(pi, &tab);
		lut_i = (val & 0x00fff000) << 8;
		lut_j = (val & 0x00000fff) << 20;
		lut_i = lut_i >> 20;
		lut_j = lut_j >> 20;
		amam = ((lut_i*lut_i) + (lut_j*lut_j));

		if (((gain_incr == 1) && (amam > amam_cmp)) ||
			((gain_incr == 0) && (amam < amam_cmp))) {
			if (previous_difference < ABS(amam_cmp - amam))
				txgains->index = (uint8)(tx_idx - step);
			else
				txgains->index = (uint8)tx_idx;
			break;
		}
	}
}

#ifdef BAND5G
static void
wlc_lcn40phy_dlo_coeff_interpolation(phy_info_t *pi, uint16 didq1,
	uint8 Idx1, uint16 didq2, uint8 Idx2)
{
	uint idx;
	phytbl_info_t tab;
	phy_info_lcn40phy_t *pi_lcn40 = pi->u.pi_lcn40phy;
	int8 Di1 = 0, Di2 = 0, Dq1 = 0, Dq2 = 0;
	uint32 val;
	int8 dmin = pi_lcn40->dlorange_lowlimit;
	int32 di, dq;
	int j;

	Di1 = (didq1 & 0xff00) >> 8;
	Dq1 = (didq1 & 0xff);
	Di2 = (didq2 & 0xff00) >> 8;
	Dq2 = (didq2 & 0xff);

	/* Populate tx power control table with coeffs */
	tab.tbl_id = LCN40PHY_TBL_ID_TXPWRCTL;
	tab.tbl_width = 32;	/* 32 bit wide	*/
	tab.tbl_ptr = &val; /* ptr to buf */

	/* Per rate power offset */
	tab.tbl_len = 1; /* # values   */

	/* Interpolating digital loft coefficients from idx1 to idx2
	and extrapolating beyond (idx1, idx2) for 4334-B2 in A band
	*/
	val = didq1;
	tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_LO_OFFSET +
		Idx1;
	wlc_lcn40phy_write_table(pi, &tab);

	val = didq2;
	tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_LO_OFFSET +
		Idx2;
	wlc_lcn40phy_write_table(pi, &tab);

	/* Interpolating digital loft coeffs from idx1 to idx2
	and Extrapolating from idx2 to 127
	*/
	for (idx = (Idx1+1); idx < 128; idx++) {
		di = (Di1*(Idx2-Idx1))+((Di2-Di1)*(idx-Idx1));
		di = di /(Idx2-Idx1);
		dq = (Dq1*(Idx2-Idx1))+((Dq2-Dq1)*(idx-Idx1));
		dq = dq / (Idx2-Idx1);
		tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_LO_OFFSET + idx-1;
		wlc_lcn40phy_read_table(pi, &tab);
		if ((di < dmin) || (di > 127))
			di = (val & 0xff00) >> 8;
		if ((dq < dmin) || (dq > 127))
			dq = val & 0xff;
		val = (((di & 0xff) << 8) | (dq & 0xff));
		tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_LO_OFFSET + idx;
		wlc_lcn40phy_write_table(pi, &tab);
	}
	/* Extrapolation from 0 to idx1 */
	for (j = Idx1-1; j >= 0; j--) {
		di = (Di1*(Idx2-Idx1))+((Di2-Di1)*(j-Idx1));
		di = di /(Idx2-Idx1);
		if (CHIPID(pi->sh->chip) == BCM43342_CHIP_ID) {
			dq = Dq1;
		} else {
			dq = (Dq1*(Idx2-Idx1))+((Dq2-Dq1)*(j-Idx1));
			dq = dq / (Idx2-Idx1);
		}
		tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_LO_OFFSET + j+1;
		wlc_lcn40phy_read_table(pi, &tab);
		if ((di < dmin) || (di > 127))
			di = (val & 0xff00) >> 8;
		if ((dq < dmin) || (dq > 127))
			dq = val & 0xff;
		val = (((di & 0xff) << 8) | (dq & 0xff));
		tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_LO_OFFSET + j;
		wlc_lcn40phy_write_table(pi, &tab);
	}
}
#endif /* BAND5G */

/* Run iqlo cal and populate iqlo portion of tx power control table */
static void
wlc_lcn40phy_txpwrtbl_iqlo_cal(phy_info_t *pi)
{

	phy_txgains_t old_gains;
	uint8 save_bb_mult;
	uint16 a, b, didq = 0, save_bbmult0, save_pa_gain = 0;
	uint idx, SAVE_txpwrindex = 0xFF;
	uint32 val;
	uint16 SAVE_txpwrctrl = wlc_lcn40phy_get_tx_pwr_ctrl(pi);
	uint16 SAVE_lpf_ofdm_tx_bw, SAVE_lpf_cck_tx_bw;
	uint16 a_ofdm = 0, b_ofdm = 0;
	uint8 di_ofdm = 0, dq_ofdm = 0, di_cck = 0, dq_cck = 0;
	phytbl_info_t tab;
	uint8 ei0 = 0, eq0 = 0, fi0 = 0, fq0 = 0;
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	phy_info_lcn40phy_t *pi_lcn40 = pi->u.pi_lcn40phy;
	uint8 index;
#ifdef BAND5G
	uint16 didq1 = 0, didq2 = 0;
	int8 Di1 = 0, Di2 = 0, Dq1 = 0, Dq2 = 0;
	int8 Idx1 = 0, Idx2 = 0;
	int8 iq_cal_idx = 0;
	int16 temp_sense_val = 0;
	int fullcal_idx = 50;
#endif /* BAND5G */

#if defined(PHYCAL_CACHING)
	ch_calcache_t *ctx = wlc_phy_get_chanctx(pi, pi->radio_chanspec);
	lcnphy_calcache_t *cache = NULL;

	if (ctx)
		cache = &ctx->u.lcnphy_cache;
#endif // endif

	wlc_lcn40phy_get_tx_gain(pi, &old_gains);
	save_pa_gain = wlc_lcn40phy_get_pa_gain(pi);

	/* Store state */
	save_bb_mult = wlc_lcn40phy_get_bbmult(pi);

	if (SAVE_txpwrctrl == LCN40PHY_TX_PWR_CTRL_OFF)
		SAVE_txpwrindex = wlc_lcn40phy_get_current_tx_pwr_idx(pi);

	if (CHIPID(pi->sh->chip) == BCM4314_CHIP_ID ||
		CHIPID(pi->sh->chip) == BCM43142_CHIP_ID ||
		CHIPID(pi->sh->chip) == BCM43143_CHIP_ID)

		wlc_lcn40phy_set_tx_pwr_by_index(pi, 60);
	else {
#ifdef BAND5G
		if (CHSPEC_IS5G(pi->radio_chanspec)) {
			temp_sense_val = wlc_lcn40phy_tempsense(pi, TEMPER_VBAT_TRIGGER_NEW_MEAS);
			if (pi_lcn->iqlocalidx_5g == -1) {
				fullcal_idx = 50;
			} else {
				fullcal_idx = pi_lcn->iqlocalidx_5g;
			}
			if ((pi->radio_chanspec & WL_CHANSPEC_CHAN_MASK) >= 100)
				fullcal_idx += pi_lcn40->localoffs5gmh;
			if (temp_sense_val < 0)
				fullcal_idx += LCN40PHY_LOWTEMP_TXPWR_IDX_ADJ;
			wlc_lcn40phy_set_tx_pwr_by_index(pi, fullcal_idx);
		} else
#endif /* BAND5G */
			wlc_lcn40phy_set_tx_pwr_by_index(pi, 40);
	}
	save_bbmult0 = phy_utils_read_phyreg(pi, LCN40PHY_bbmult0);
	if (LCN40REV_LT(pi->pubpi.phy_rev, 4)) {
		phy_utils_write_phyreg(pi, LCN40PHY_bbmult0, 0x1FF);
		/* load iir filter with high gain */
		wlc_lcn40phy_load_tx_iir_filter(pi, TX_IIR_FILTER_OFDM, 10);
	} else {
		if (CHSPEC_IS40(pi->radio_chanspec)) {
			/* In 40MHz bw mode, this override will disable
			the -10MHz shift in the +/-10MHz upconversion block.
			*/
			PHY_REG_MOD(pi, LCN40PHY, upconv, bw20in40_override, 1);
			PHY_REG_MOD(pi, LCN40PHY, upconv, bw20in40_val, 0);
		}
	}

	/* FIX ME: We should take another look at this implementation to
		optimize it further.
	*/
#ifdef BAND5G
	if (LCN40REV_GE(pi->pubpi.phy_rev, 4) && CHSPEC_IS5G(pi->radio_chanspec)) {
		if (pi_lcn40->loflag) {
			wlc_lcn40phy_tx_iqlo_cal(pi, NULL, CAL_FULL2, FALSE, 0);

			if (pi_lcn40->dlocalidx5g != -1) {
				uint16 lo_value;
				int32 Di, Dq;
				/* Digital LO recal and interpolation
				  * or extrapolation to find digital
				  * loft coefficients at iqcal2idx
				*/
				didq1 = wlc_lcn40phy_get_tx_locc(pi);
				Idx2 = pi_lcn40->dlocalidx5g;

				if ((pi->radio_chanspec & WL_CHANSPEC_CHAN_MASK) >= 100)
					Idx2 += pi_lcn40->localoffs5gmh;

				/* if temperature is less than 0C, increase the Cal index by 20.
				  * This is done as gain decreases by ~5dB at -20C when
				  * compared to room temperature.
				  */
				if (temp_sense_val < 0)
					Idx2 += LCN40PHY_LOWTEMP_TXPWR_IDX_ADJ;

				wlc_lcn40phy_set_tx_pwr_by_index(pi, Idx2);
				wlc_lcn40phy_tx_iqlo_cal(pi, NULL, CAL_DIGLO, FALSE,
					pi_lcn40->epa_or_pad_lpbk);
				didq2 = wlc_lcn40phy_get_tx_locc(pi);

				Di1 = (didq1 & 0xff00) >> 8;
				Dq1 = (didq1 & 0xff);
				Di2 = (didq2 & 0xff00) >> 8;
				Dq2 = (didq2 & 0xff);

				Idx1 = (int8) fullcal_idx;

				if (pi_lcn40->iqcalidx5g == -1)
					iq_cal_idx = 50;
				else
					iq_cal_idx = (int8) pi_lcn40->iqcalidx5g;

				Di = ((Di1*(Idx2-Idx1))+((Di2-Di1)*(iq_cal_idx-Idx1)));
				Di = Di / (Idx2-Idx1);
				Dq = ((Dq1*(Idx2-Idx1))+((Dq2-Dq1)*(iq_cal_idx-Idx1)));
				Dq = Dq / (Idx2-Idx1);

				if ((Di < -128) || (Di > 127) || (Dq < -128) || (Dq > 127)) {
					Di = Di2;
					Dq = Dq2;
				}

				lo_value = (((Di & 0xff) << 8) | (Dq & 0xff));
#if defined(PHYCAL_CACHING)
				if (ctx)
					cache->txiqlocal_bestcoeffs[5] = lo_value;
#else
				pi_lcn->lcnphy_cal_results.txiqlocal_bestcoeffs[5] = lo_value;
#endif // endif
			}
			if (pi_lcn40->iqcalidx5g != -1)
				wlc_lcn40phy_set_tx_pwr_by_index(pi, pi_lcn40->iqcalidx5g);
			else
				wlc_lcn40phy_set_tx_pwr_by_index(pi, 50);

			wlc_lcn40phy_tx_iqlo_cal(pi, NULL, CAL_IQ_CAL3, FALSE, 0);
		} else {
			wlc_lcn40phy_tx_iqlo_cal(pi, NULL, CAL_FULL, FALSE, 0);
			if (pi_lcn40->iqcalidx5g != -1)
				wlc_lcn40phy_set_tx_pwr_by_index(pi, pi_lcn40->iqcalidx5g);
			else
				wlc_lcn40phy_set_tx_pwr_by_index(pi, 50);
			wlc_lcn40phy_tx_iqlo_cal(pi, NULL, CAL_IQ_RECAL, FALSE, 0);
		}
		wlc_lcn40phy_get_radio_loft(pi, &ei0, &eq0, &fi0, &fq0);
		didq = wlc_lcn40phy_get_tx_locc(pi);
	} else
#endif /* BAND5G */
	{
		if (LCN40REV_IS(pi->pubpi.phy_rev, 6) ||
			(LCN40REV_IS(pi->pubpi.phy_rev, 7) &&
			(RADIOVER(pi->pubpi.radiover) == 0x4))) {
			SAVE_lpf_ofdm_tx_bw = PHY_REG_READ(pi, LCN40PHY,
				lpfbwlutreg1, lpf_ofdm_tx_bw);
			SAVE_lpf_cck_tx_bw = PHY_REG_READ(pi, LCN40PHY,
				lpfbwlutreg1, lpf_cck_tx_bw);

			/* OFDM coeffs */
			wlc_lcn40phy_tx_iqlo_cal(pi, NULL, (pi_lcn->lcnphy_recal ?
				CAL_RECAL : CAL_FULL), FALSE, 0);

			wlc_lcn40phy_get_tx_iqcc(pi, &a_ofdm, &b_ofdm);
			didq = wlc_lcn40phy_get_tx_locc(pi);
			di_ofdm = (didq & 0xff00) >> 8;
			dq_ofdm = (didq & 0xff);

			PHY_REG_MOD(pi, LCN40PHY, iq_coeff_a_cck, iq_coeff_a_cck, a_ofdm);
			PHY_REG_MOD(pi, LCN40PHY, iq_coeff_b_cck, iq_coeff_b_cck, b_ofdm);
			PHY_REG_MOD(pi, LCN40PHY, iq_coeff_a_ofdm, iq_coeff_a_ofdm, a_ofdm);
			PHY_REG_MOD(pi, LCN40PHY, iq_coeff_b_ofdm, iq_coeff_b_ofdm, b_ofdm);
			PHY_REG_MOD(pi, LCN40PHY, iq_coeff_a_ht, iq_coeff_a_ht, a_ofdm);
			PHY_REG_MOD(pi, LCN40PHY, iq_coeff_b_ht, iq_coeff_b_ht, b_ofdm);

			/* For CCK coeffs */
			PHY_REG_MOD(pi, LCN40PHY, lpfbwlutreg1, lpf_ofdm_tx_bw, SAVE_lpf_cck_tx_bw);
			PHY_REG_MOD(pi, LCN40PHY, rfoverride4, lpf_dc1_pwrup_ovr, 0x0);
			wlc_lcn40phy_tx_iqlo_cal(pi, NULL, CAL_DIGLO, FALSE, 0);

			didq = wlc_lcn40phy_get_tx_locc(pi);
			di_cck = (didq & 0xff00) >> 8;
			dq_cck = (didq & 0xff);
			PHY_REG_MOD(pi, LCN40PHY, dlo_coeff_di_ofdm, dlo_coeff_di_ofdm, di_ofdm);
			PHY_REG_MOD(pi, LCN40PHY, dlo_coeff_dq_ofdm, dlo_coeff_dq_ofdm, dq_ofdm);
			PHY_REG_MOD(pi, LCN40PHY, dlo_coeff_di_ht, dlo_coeff_di_ht, di_ofdm);
			PHY_REG_MOD(pi, LCN40PHY, dlo_coeff_dq_ht, dlo_coeff_dq_ht, dq_ofdm);
			PHY_REG_MOD(pi, LCN40PHY, dlo_coeff_di_cck, dlo_coeff_di_cck, di_cck);
			PHY_REG_MOD(pi, LCN40PHY, dlo_coeff_dq_cck, dlo_coeff_dq_cck, dq_cck);

			wlc_lcn40phy_set_tx_locc(pi, ((di_ofdm << 8) | (dq_ofdm & 0xff)));

			PHY_REG_MOD(pi, LCN40PHY, lpfbwlutreg1,
				lpf_ofdm_tx_bw, SAVE_lpf_ofdm_tx_bw);
			PHY_REG_MOD(pi, LCN40PHY, lpfbwlutreg1,
				lpf_cck_tx_bw, SAVE_lpf_cck_tx_bw);
		} else {
			wlc_lcn40phy_tx_iqlo_cal(pi, NULL, (pi_lcn->lcnphy_recal ?
				CAL_RECAL : CAL_FULL), FALSE, 0);
		}
	}
	if (LCN40REV_LT(pi->pubpi.phy_rev, 4) || CHSPEC_IS2G(pi->radio_chanspec)) {
		wlc_lcn40phy_get_radio_loft(pi, &ei0, &eq0, &fi0, &fq0);
		didq = wlc_lcn40phy_get_tx_locc(pi);
	}

	/* Get calibration results */
	wlc_lcn40phy_get_tx_iqcc(pi, &a, &b);
	PHY_INFORM(("TXIQCal: %d %d\n", a, b));

	/* Populate tx power control table with coeffs */
	tab.tbl_id = LCN40PHY_TBL_ID_TXPWRCTL;
	tab.tbl_width = 32;	/* 32 bit wide	*/
	tab.tbl_ptr = &val; /* ptr to buf */

	/* Per rate power offset */
	tab.tbl_len = 1; /* # values   */

	for (idx = 0; idx < 128; idx++) {
		tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_IQ_OFFSET + idx;
		/* iq */
		wlc_lcn40phy_read_table(pi, &tab);
		val = (val & 0xfff00000) |
			((uint32)(a & 0x3FF) << 10) | (b & 0x3ff);
		wlc_lcn40phy_write_table(pi, &tab);

		/* loft */
		val = didq;
		tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_LO_OFFSET + idx;
		wlc_lcn40phy_write_table(pi, &tab);
	}

#ifdef BAND5G
	if ((LCN40REV_GE(pi->pubpi.phy_rev, 4)) &&
		(CHSPEC_IS5G(pi->radio_chanspec))) {
		if ((pi_lcn40->dlocalidx5g != -1) && pi_lcn40->loflag) {
			wlc_lcn40phy_dlo_coeff_interpolation(pi, didq1, Idx1, didq2, Idx2);
		}
	}
#endif /* BAND5G */

	/* Save Cal Results */
	index = 0;
#if defined(PHYCAL_CACHING)
	if (ctx) {
		cache->txiqlocal_a[index] = a;
		cache->txiqlocal_b[index] = b;
		/* Use "txiqlocal_didq[0]" to store 2G OFDM didq */
		/*     "txiqlocal_didq[1]" to store 2G CCK didq */
		if (LCN40REV_IS(pi->pubpi.phy_rev, 6) ||
			(LCN40REV_IS(pi->pubpi.phy_rev, 7) && CHSPEC_IS2G(pi->radio_chanspec) &&
			(RADIOVER(pi->pubpi.radiover) == 0x4))) {
			cache->txiqlocal_didq[0] = ((di_ofdm << 8) | (dq_ofdm & 0xff));
			cache->txiqlocal_didq[1] = ((di_cck << 8) | (dq_cck & 0xff));
		} else {
			cache->txiqlocal_didq[index] = didq;
		}
#ifdef BAND5G
		if ((LCN40REV_GE(pi->pubpi.phy_rev, 4)) &&
			(CHSPEC_IS5G(pi->radio_chanspec))) {
			if ((pi_lcn40->dlocalidx5g != -1) && pi_lcn40->loflag) {
				cache->txiqlocal_didq[0] = didq1;
				cache->txiqlocal_index[0] = Idx1;
				cache->txiqlocal_didq[1] = didq2;
				cache->txiqlocal_index[1] = Idx2;
			}
		}
#endif /* BAND5G */
		cache->txiqlocal_ei0 = ei0;
		cache->txiqlocal_eq0 = eq0;
		cache->txiqlocal_fi0 = fi0;
		cache->txiqlocal_fq0 = fq0;
	}
#else
	pi_lcn->lcnphy_cal_results.txiqlocal_a[index] = a;
	pi_lcn->lcnphy_cal_results.txiqlocal_b[index] = b;
	/* Use "txiqlocal_didq[0]" to store 2G OFDM didq */
	/*     "txiqlocal_didq[1]" to store 2G CCK didq */
	if (LCN40REV_IS(pi->pubpi.phy_rev, 6) ||
		(LCN40REV_IS(pi->pubpi.phy_rev, 7) && (CHSPEC_IS2G(pi->radio_chanspec)) &&
		(RADIOVER(pi->pubpi.radiover) == 0x4))) {
		pi_lcn->lcnphy_cal_results.txiqlocal_didq[0] =
			((di_ofdm << 8) | (dq_ofdm & 0xff));
		pi_lcn->lcnphy_cal_results.txiqlocal_didq[1] =
			((di_cck << 8) | (dq_cck & 0xff));
	} else {
		pi_lcn->lcnphy_cal_results.txiqlocal_didq[index] = didq;
	}
#ifdef BAND5G
	if ((LCN40REV_GE(pi->pubpi.phy_rev, 4)) &&
	(CHSPEC_IS5G(pi->radio_chanspec))) {
		if ((pi_lcn40->dlocalidx5g != -1) && pi_lcn40->loflag) {
			pi_lcn->lcnphy_cal_results.txiqlocal_didq[0] = didq1;
			pi_lcn->lcnphy_cal_results.txiqlocal_index[0] = Idx1;
			pi_lcn->lcnphy_cal_results.txiqlocal_didq[1] = didq2;
			pi_lcn->lcnphy_cal_results.txiqlocal_index[1] = Idx2;
		}
	}
#endif /* BAND5G */
	pi_lcn->lcnphy_cal_results.txiqlocal_ei0 = ei0;
	pi_lcn->lcnphy_cal_results.txiqlocal_eq0 = eq0;
	pi_lcn->lcnphy_cal_results.txiqlocal_fi0 = fi0;
	pi_lcn->lcnphy_cal_results.txiqlocal_fq0 = fq0;
#endif /* PHYCAL_CACHING */

	/* Restore state */
	wlc_lcn40phy_set_bbmult(pi, save_bb_mult);
	wlc_lcn40phy_set_pa_gain(pi, save_pa_gain);
	wlc_lcn40phy_set_tx_gain(pi, &old_gains);

	if (SAVE_txpwrctrl != LCN40PHY_TX_PWR_CTRL_OFF)
		wlc_lcn40phy_set_tx_pwr_ctrl(pi, SAVE_txpwrctrl);
	else
		wlc_lcn40phy_set_tx_pwr_by_index(pi, SAVE_txpwrindex);
	phy_utils_write_phyreg(pi, LCN40PHY_bbmult0, save_bbmult0);
	if (LCN40REV_LT(pi->pubpi.phy_rev, 4)) {
		/* restore default iir filter */
		wlc_lcn40phy_load_tx_iir_filter(pi, TX_IIR_FILTER_OFDM,
		pi_lcn40->tx_iir_filter_type_ofdm);
	} else {
		if (CHSPEC_IS40(pi->radio_chanspec))
			PHY_REG_MOD(pi, LCN40PHY, upconv, bw20in40_override, 0);
	}
}
/*
* RX IQ Calibration
*/
static bool
wlc_lcn40phy_rx_iq_cal(phy_info_t *pi, const phy_rx_iqcomp_t *iqcomp, int iqcomp_sz,
	bool tx_switch, bool rx_switch, int module, int tx_gain_idx)
{
	phy_txgains_t old_gains;
	uint16 tx_pwr_ctrl, save_bbmult0, tone_amplitude;
	uint8 tx_gain_index_old = 0, save_bbmult;
	bool result = FALSE, tx_gain_override_old = FALSE;
	uint16 i;
	int tia_gain, biq1_gain;
	uint16 values_to_save[ARRAYSIZE(rxiq_cal_rf_reg)];
	uint16 values_to_save_phy[ARRAYSIZE(rxiq_cal_phy_reg)];
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	bool set_gain = FALSE, papd_en = wlc_lcn40phy_is_papd_block_enable(pi);
	phy_info_lcn40phy_t *pi_lcn40 = pi->u.pi_lcn40phy;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

#ifdef BAND5G
	if (CHSPEC_IS5G(pi->radio_chanspec) && pi_lcn40->rx_iq_comp_5g[0])
	{
		wlc_lcn40phy_set_rx_iq_comp(pi, pi_lcn40->rx_iq_comp_5g[0],
			pi_lcn40->rx_iq_comp_5g[1]);
		return TRUE;
	}
#endif /* BAND5G */

	if (CHSPEC_IS2G(pi->radio_chanspec) && pi_lcn40->rx_iq_comp_2g[0])
	{
		wlc_lcn40phy_set_rx_iq_comp(pi, pi_lcn40->rx_iq_comp_2g[0],
			pi_lcn40->rx_iq_comp_2g[1]);
		return TRUE;
	}

	if (LCN40REV_GT(pi->pubpi.phy_rev, 3))
		tone_amplitude = 100;
	else
		tone_amplitude = 200;

	if ((BOARDTYPE(pi->sh->boardtype) != BCM94334FCAGBI_SSID) &&
		(BOARDTYPE(pi->sh->boardtype) != BCM94334WLAGBI_SSID) &&
		(BOARDTYPE(pi->sh->boardtype) != BCM943342FCAGBI_SSID))
		if (PHY_EPA_SUPPORT(pi_lcn->ePA))
			wlc_lcn40phy_epa_pd(pi, 1);

	if (module == 2) {
		ASSERT(iqcomp_sz);

		while (iqcomp_sz--) {
			if (iqcomp[iqcomp_sz].chan == CHSPEC_CHANNEL(pi->radio_chanspec)) {
				/* Apply new coeffs */
				wlc_lcn40phy_set_rx_iq_comp(pi, (uint16)iqcomp[iqcomp_sz].a,
					(uint16)iqcomp[iqcomp_sz].b);
				result = TRUE;
				break;
			}
		}
		ASSERT(result);
		goto cal_done;
	}
	/* module : 1 = loopback */
	if (module == 1) {

		/* turn off tx power control */
		tx_pwr_ctrl = wlc_lcn40phy_get_tx_pwr_ctrl(pi);
		wlc_lcn40phy_set_tx_pwr_ctrl(pi, LCN40PHY_TX_PWR_CTRL_OFF);
		/* turn off papd */
		wlc_lcn40phy_papd_block_enable(pi, FALSE);

		/* save rf register states */
		for (i = 0; i < ARRAYSIZE(rxiq_cal_rf_reg); i++) {
			values_to_save[i] = phy_utils_read_radioreg(pi, rxiq_cal_rf_reg[i]);
		}
		for (i = 0; i < ARRAYSIZE(rxiq_cal_phy_reg); i++) {
			values_to_save_phy[i] = phy_utils_read_phyreg(pi, rxiq_cal_phy_reg[i]);
		}
		PHY_REG_LIST_START
			/* disable aux path
			* set reg(RF_auxpga_cfg1.auxpga_bias_ctrl) 0
			* set reg(RF_auxpga_cfg1.auxpga_vcm_ctrl) 1
			* set reg(RF_OVR1.ovr_afe_auxpga_pu)  1
			* set reg(RF_auxpga_cfg1.auxpga_pu) 0
			*/
			RADIO_REG_MOD_ENTRY(RADIO_2065_AUXPGA_CFG1, 0x3031, 0x10)
			RADIO_REG_OR_ENTRY(RADIO_2065_OVR1, 0x8000)
			/* loft comp , iqmm comp enable */
			PHY_REG_OR_ENTRY(LCN40PHY, Core1TxControl, 0x0015)
		PHY_REG_LIST_EXECUTE(pi);
		/* Save old tx gain settings */
		tx_gain_override_old = wlc_lcn40phy_tx_gain_override_enabled(pi);
		if (tx_gain_override_old) {
			wlc_lcn40phy_get_tx_gain(pi, &old_gains);
			tx_gain_index_old = pi_lcn->lcnphy_current_index;
		}
		/* Apply new tx gain */
		wlc_lcn40phy_set_tx_pwr_by_index(pi, tx_gain_idx);

		/* Force DAC/ADC on */
		phy_utils_mod_phyreg(pi, LCN40PHY_AfeCtrlOvr1,
			(LCN40PHY_AfeCtrlOvr1_adc_pu_ovr_MASK |
			LCN40PHY_AfeCtrlOvr1_dac_pu_ovr_MASK),
			((1 << LCN40PHY_AfeCtrlOvr1_adc_pu_ovr_SHIFT) |
			(1 << LCN40PHY_AfeCtrlOvr1_dac_pu_ovr_SHIFT)));
		phy_utils_mod_phyreg(pi, LCN40PHY_AfeCtrlOvr1Val,
			(LCN40PHY_AfeCtrlOvr1Val_adc_pu_ovr_val_MASK |
			LCN40PHY_AfeCtrlOvr1Val_dac_pu_ovr_val_MASK),
			((31 << LCN40PHY_AfeCtrlOvr1Val_adc_pu_ovr_val_SHIFT) |
			(1 << LCN40PHY_AfeCtrlOvr1Val_dac_pu_ovr_val_SHIFT)));
		/* TR switch */
		wlc_lcn40phy_set_trsw_override(pi, tx_switch, rx_switch);

		PHY_REG_LIST_START
			/* AMS configuration */
			RADIO_REG_MOD_ENTRY(RADIO_2065_LPF_CFG1, 0xcec0, 0x8a00)
			RADIO_REG_MOD_ENTRY(RADIO_2065_OVR7, 0xd4, 0x14)
			PHY_REG_MOD_RAW_ENTRY(LCN40PHY_rfoverride7,
				(LCN40PHY_rfoverride7_lpf_sel_rx_buffer_ovr_MASK |
				LCN40PHY_rfoverride7_lpf_sel_byp_rxlpf_ovr_MASK),
				((1 << LCN40PHY_rfoverride7_lpf_sel_rx_buffer_ovr_SHIFT) |
				(1 << LCN40PHY_rfoverride7_lpf_sel_byp_rxlpf_ovr_SHIFT)))
			PHY_REG_MOD_RAW_ENTRY(LCN40PHY_rfoverride7val,
				(LCN40PHY_rfoverride7val_lpf_sel_rx_buffer_ovr_val_MASK |
				LCN40PHY_rfoverride7val_lpf_sel_byp_rxlpf_ovr_val_MASK),
				((0 << LCN40PHY_rfoverride7val_lpf_sel_rx_buffer_ovr_val_SHIFT) |
				(2 << LCN40PHY_rfoverride7val_lpf_sel_byp_rxlpf_ovr_val_SHIFT)))

			RADIO_REG_MOD_ENTRY(RADIO_2065_LPF_CFG2, 0x7f, 0x1b)
			RADIO_REG_MOD_ENTRY(RADIO_2065_OVR6, 0x43e0, 0x43e0)
			RADIO_REG_MOD_ENTRY(RADIO_2065_OVR5, 0x12, 0x12)
		PHY_REG_LIST_EXECUTE(pi);

		if (CHSPEC_IS20(pi->radio_chanspec)) {
			phy_utils_mod_radioreg(pi, RADIO_2065_LPF_RESP_BQ1, 0x7, 0);
			phy_utils_mod_radioreg(pi, RADIO_2065_LPF_RESP_BQ2, 0x7, 0);
		} else {
			phy_utils_mod_radioreg(pi, RADIO_2065_LPF_RESP_BQ1, 0x7, 2);
			phy_utils_mod_radioreg(pi, RADIO_2065_LPF_RESP_BQ2, 0x7, 2);
		}

		phy_utils_write_radioreg(pi, RADIO_2065_LPF_BIAS0, 2);

		/* Enable loopback from PAD to MIXER */
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			PHY_REG_LIST_START
				RADIO_REG_OR_ENTRY(RADIO_2065_OVR12, 0x10)
				RADIO_REG_OR_ENTRY(RADIO_2065_OVR11, 0x8000)
				RADIO_REG_OR_ENTRY(RADIO_2065_OVR2, 0x20)
				RADIO_REG_MOD_ENTRY(RADIO_2065_TXRX2G_CAL, 0x303, 0x103)

				PHY_REG_MOD_RAW_ENTRY(LCN40PHY_RFOverrideVal0,
				    (LCN40PHY_RFOverrideVal0_internalrfrxpu_ovr_val_MASK |
				    LCN40PHY_RFOverrideVal0_internalrftxpu_ovr_val_MASK),
				    ((1 << LCN40PHY_RFOverrideVal0_internalrfrxpu_ovr_val_SHIFT) |
				    (1 << LCN40PHY_RFOverrideVal0_internalrftxpu_ovr_val_SHIFT)))
				PHY_REG_MOD_RAW_ENTRY(LCN40PHY_RFOverride0,
				    (LCN40PHY_RFOverride0_internalrfrxpu_ovr_MASK |
				    LCN40PHY_RFOverride0_internalrftxpu_ovr_MASK),
				    ((1 << LCN40PHY_RFOverride0_internalrfrxpu_ovr_SHIFT) |
				    (1 << LCN40PHY_RFOverride0_internalrftxpu_ovr_SHIFT)))
			PHY_REG_LIST_EXECUTE(pi);
			/* New radio register setting for 43341 */
			if (LCN40REV_IS(pi->pubpi.phy_rev, 7)) {
				phy_utils_mod_radioreg(pi, RADIO_2065_TXRX2G_CAL_BAK, 0x3, 0x3);
				phy_utils_mod_radioreg(pi, RADIO_2065_TXRX2G_CAL, 0xf00, 0xa00);
			}
		}
#ifdef BAND5G
		else {
			PHY_REG_LIST_START
				RADIO_REG_OR_ENTRY(RADIO_2065_OVR13, 0x2000)
				RADIO_REG_OR_ENTRY(RADIO_2065_OVR11, 0x4000)
				RADIO_REG_OR_ENTRY(RADIO_2065_OVR2, 0x10)
				RADIO_REG_MOD_ENTRY(RADIO_2065_TXRX5G_CAL, 0xf03, 0x303)
				/* Force off the Aband-ePA */

				PHY_REG_MOD_RAW_ENTRY(LCN40PHY_RFOverride0,
					LCN40PHY_RFOverride0_amode_tx_pu_ovr_MASK |
					LCN40PHY_RFOverride0_amode_rx_pu_ovr_MASK |
					LCN40PHY_RFOverride0_internalrfrxpu_ovr_MASK,
					(1 << LCN40PHY_RFOverride0_amode_tx_pu_ovr_SHIFT) |
					(1 << LCN40PHY_RFOverride0_amode_rx_pu_ovr_SHIFT) |
					(1 << LCN40PHY_RFOverride0_internalrfrxpu_ovr_SHIFT))
				PHY_REG_MOD_RAW_ENTRY(LCN40PHY_RFOverrideVal0,
					LCN40PHY_RFOverrideVal0_amode_tx_pu_ovr_val_MASK |
					LCN40PHY_RFOverrideVal0_amode_rx_pu_ovr_val_MASK |
					LCN40PHY_RFOverrideVal0_internalrfrxpu_ovr_val_MASK,
					(1 << LCN40PHY_RFOverrideVal0_amode_tx_pu_ovr_val_SHIFT) |
					(1 << LCN40PHY_RFOverrideVal0_amode_rx_pu_ovr_val_SHIFT) |
					(1 << LCN40PHY_RFOverrideVal0_internalrfrxpu_ovr_val_SHIFT))
			PHY_REG_LIST_EXECUTE(pi);
			/* New radio register setting for 43341 */
			if (LCN40REV_IS(pi->pubpi.phy_rev, 7)) {
				phy_utils_mod_radioreg(pi, RADIO_2065_TXRX5G_CAL_BAK, 0x3, 0x3);
				phy_utils_mod_radioreg(pi, RADIO_2065_TXRX5G_CAL, 0xf00, 0xa00);
			}
		}
#endif /* BAND5G */
		PHY_REG_LIST_START
			/* shut off LNA's */
			PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride2val, slna_pu_ovr_val, 0)
			PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride2, slna_pu_ovr, 1)
			PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride8val, lna2_pu_ovr_val, 0)
			PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride8, lna2_pu_ovr, 1)
			PHY_REG_MOD_RAW_ENTRY(LCN40PHY_rfoverride4val,
				LCN40PHY_rfoverride4val_lpf_dc1_pwrup_ovr_val_MASK |
				LCN40PHY_rfoverride4val_lpf_byp_dc_ovr_val_MASK,
				(1 << LCN40PHY_rfoverride4val_lpf_dc1_pwrup_ovr_val_SHIFT) |
				(0 << LCN40PHY_rfoverride4val_lpf_byp_dc_ovr_val_SHIFT))

			PHY_REG_MOD_RAW_ENTRY(LCN40PHY_rfoverride4,
				LCN40PHY_rfoverride4_lpf_dc1_pwrup_ovr_MASK |
				LCN40PHY_rfoverride4_lpf_byp_dc_ovr_MASK,
				(1 << LCN40PHY_rfoverride4_lpf_dc1_pwrup_ovr_SHIFT) |
				(1 << LCN40PHY_rfoverride4_lpf_byp_dc_ovr_SHIFT))
			PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride7val, lpf_hpc_ovr_val, 1)
			PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride7, lpf_hpc_ovr, 0x1)
			PHY_REG_MOD_ENTRY(LCN40PHY, ClkEnCtrl, forcerxfrontendclk, 1)
			/* make sure tx/rx is powered up */
			PHY_REG_OR_RAW_ENTRY(LCN40PHY_rfoverride8val,
				LCN40PHY_rfoverride8val_fast_nap_bias_pu_ovr_val_MASK |
				LCN40PHY_rfoverride8val_logen_rx_pu_ovr_val_MASK)

			PHY_REG_OR_RAW_ENTRY(LCN40PHY_rfoverride8,
				LCN40PHY_rfoverride8_fast_nap_bias_pu_ovr_MASK |
				LCN40PHY_rfoverride8_logen_rx_pu_ovr_MASK)
		PHY_REG_LIST_EXECUTE(pi);

		save_bbmult = wlc_lcn40phy_get_bbmult(pi);
		if (LCN40REV_LT(pi->pubpi.phy_rev, 4))
			wlc_lcn40phy_set_bbmult(pi, 200);
		save_bbmult0 = phy_utils_read_phyreg(pi, LCN40PHY_bbmult0);
		if (LCN40REV_LT(pi->pubpi.phy_rev, 4))
			phy_utils_write_phyreg(pi, LCN40PHY_bbmult0, 0x1FF);

		PHY_REG_LIST_START
			/* Run calibration */
			PHY_REG_WRITE_ENTRY(LCN40PHY, sslpnCalibClkEnCtrl, 0xffff)
			PHY_REG_OR_ENTRY(LCN40PHY, sslpnRxFeClkEnCtrl, 0x3)

			PHY_REG_MOD_ENTRY(LCN40PHY, rxfe, bypass_iqcomp, 1)
		PHY_REG_LIST_EXECUTE(pi);
		/* adjust rx power */
		#define k_lcnphy_rx_iq_est_n_samps 1024

		tia_gain = 5;
		while ((tia_gain >= 0) && !set_gain) {

			biq1_gain = 2;
			while ((biq1_gain >= 0) && !set_gain) {
				phy_iq_est_t iq_est_h;
				phy_iq_est_t iq_est_l;
				uint32 i_thresh_l, i_thresh_h;
				uint32 q_thresh_l, q_thresh_h;

				wlc_lcn40phy_set_rx_gain_by_distribution(pi, 0, 0, 0, 0, 0, 0, 0,
					(uint16)tia_gain, (uint16)biq1_gain, 0, 0, 0);
				/* PHY thinks LPF is in tx mode, need radio override */
				PHY_REG_MOD(pi, LCN40PHY, lpfgainlutreg, lpf_ofdm_tx_biq2_gain, 0);
				PHY_REG_MOD(pi, LCN40PHY, lpfgainlutreg, lpf_ofdm_tx_biq1_gain,
					(uint16)biq1_gain);
				PHY_REG_MOD(pi, LCN40PHY, lpfgainlutreg, lpf_cck_tx_biq2_gain, 0);
				PHY_REG_MOD(pi, LCN40PHY, lpfgainlutreg, lpf_cck_tx_biq1_gain,
					(uint16)biq1_gain);

				wlc_lcn40phy_rx_gain_override_enable(pi, TRUE);

				wlc_lcn40phy_start_tx_tone(pi, (2000 * 1000),
					(tone_amplitude>>1), 0);
				/* PA override */
				phy_utils_mod_phyreg(pi, LCN40PHY_rfoverride4,
					LCN40PHY_rfoverride4_papu_ovr_MASK,
					1 << LCN40PHY_rfoverride4_papu_ovr_SHIFT);
				phy_utils_mod_phyreg(pi, LCN40PHY_rfoverride4val,
					LCN40PHY_rfoverride4val_papu_ovr_val_MASK,
					0 << LCN40PHY_rfoverride4val_papu_ovr_val_SHIFT);

				if (!wlc_lcn40phy_rx_iq_est(pi, k_lcnphy_rx_iq_est_n_samps,
					32, 0, &iq_est_l, RXIQEST_TIMEOUT))
				{
					PHY_INFORM(("wl%d: %s Error getting low iq est",
						pi->sh->unit, __FUNCTION__));
					break;
				}
				wlc_lcn40phy_stop_tx_tone(pi);
				wlc_lcn40phy_start_tx_tone(pi, (2000 * 1000),
					tone_amplitude, 0);
				/* PA override : tx tone will force PA on */
				phy_utils_mod_phyreg(pi, LCN40PHY_rfoverride4,
					LCN40PHY_rfoverride4_papu_ovr_MASK,
					1 << LCN40PHY_rfoverride4_papu_ovr_SHIFT);
				phy_utils_mod_phyreg(pi, LCN40PHY_rfoverride4val,
					LCN40PHY_rfoverride4val_papu_ovr_val_MASK,
					0 << LCN40PHY_rfoverride4val_papu_ovr_val_SHIFT);

				if (!wlc_lcn40phy_rx_iq_est(pi, k_lcnphy_rx_iq_est_n_samps,
					32, 0, &iq_est_h, RXIQEST_TIMEOUT))
				{
					PHY_INFORM(("wl%d: %s Error getting high iq est",
						pi->sh->unit, __FUNCTION__));
					break;
				}

				i_thresh_l = (iq_est_l.i_pwr << 1) + iq_est_l.i_pwr;
				i_thresh_h = (iq_est_l.i_pwr << 2) + iq_est_l.i_pwr;

				q_thresh_l = (iq_est_l.q_pwr << 1) + iq_est_l.q_pwr;
				q_thresh_h = (iq_est_l.q_pwr << 2) + iq_est_l.q_pwr;

				/* Check that rx power drops by 6dB after pwr of */
				/* dac input drops by 6dB */
				if ((iq_est_h.i_pwr > i_thresh_l) &&
					(iq_est_h.i_pwr < i_thresh_h) &&
					(iq_est_h.q_pwr > q_thresh_l) &&
					(iq_est_h.q_pwr < q_thresh_h)) {
					set_gain = TRUE;

					PHY_INFORM(("wl%d: %s gains are "
								"tia=%d biq1=%d biq2=0",
								pi->sh->unit, __FUNCTION__,
								tia_gain, biq1_gain));
					break;
				}
				if ((iq_est_h.i_pwr < i_thresh_l) ||
					(iq_est_h.q_pwr < q_thresh_l)) {
#ifdef BAND5G
						if (LCN40REV_IS(pi->pubpi.phy_rev, 7) &&
							CHSPEC_IS5G(pi->radio_chanspec) &&
							CHSPEC_IS40(pi->radio_chanspec))
							tone_amplitude = 70;
						else
#endif /* BAND5G */
							tone_amplitude = 100;
					}
				biq1_gain--;
			}
			tia_gain--;
		}

		PHY_REG_MOD(pi, LCN40PHY, rxfe, bypass_iqcomp, 0);

		if (set_gain)
			result = wlc_lcn40phy_calc_rx_iq_comp(pi, 0x4000);
		else {
				wlc_lcn40phy_set_rx_gain_by_distribution(pi, 0, 0, 0, 0, 0, 0, 0,
					5, 4, 0, 0, 0);
				phy_utils_write_phyreg(pi, LCN40PHY_lpfgainlutreg, 0);
				wlc_lcn40phy_rx_gain_override_enable(pi, TRUE);
				wlc_lcn40phy_calc_rx_iq_comp(pi, 0x4000);
				result = FALSE;
		}

		/* clean up */
		wlc_lcn40phy_stop_tx_tone(pi);
		/* restore papd state */
		if (papd_en)
			wlc_lcn40phy_papd_block_enable(pi, TRUE);
		/* restore Core1TxControl */
		/* Restore PHY Registers */
		for (i = 0; i < ARRAYSIZE(rxiq_cal_phy_reg); i++) {
			phy_utils_write_phyreg(pi, rxiq_cal_phy_reg[i], values_to_save_phy[i]);
		}

		/* Restore RF Registers */
		for (i = 0; i < ARRAYSIZE(rxiq_cal_rf_reg); i++) {
			phy_utils_write_radioreg(pi, rxiq_cal_rf_reg[i], values_to_save[i]);
		}
		/* Restore Tx gain */
		if (tx_gain_override_old) {
			wlc_lcn40phy_set_tx_pwr_by_index(pi, tx_gain_index_old);
		} else
			wlc_lcn40phy_disable_tx_gain_override(pi);

		wlc_lcn40phy_set_tx_pwr_ctrl(pi, tx_pwr_ctrl);

		/* Clear various overrides */
		wlc_lcn40phy_rx_gain_override_enable(pi, FALSE);

		/* restore bbmult */
		phy_utils_write_phyreg(pi, LCN40PHY_bbmult0, save_bbmult0);
	    wlc_lcn40phy_set_bbmult(pi, save_bbmult);
	}

cal_done:
	if (PHY_EPA_SUPPORT(pi_lcn->ePA))
		wlc_lcn40phy_epa_pd(pi, 0);

	PHY_INFORM(("wl%d: %s: Rx IQ cal complete, coeffs: A0: %d, B0: %d\n",
		pi->sh->unit, __FUNCTION__,
		(int16)((phy_utils_read_phyreg(pi, LCN40PHY_RxCompcoeffa0) &
		LCN40PHY_RxCompcoeffa0_a0_MASK)
		>> LCN40PHY_RxCompcoeffa0_a0_SHIFT),
		(int16)((phy_utils_read_phyreg(pi, LCN40PHY_RxCompcoeffb0) &
		LCN40PHY_RxCompcoeffb0_b0_MASK)
		>> LCN40PHY_RxCompcoeffb0_b0_SHIFT)));
	return result;
}

static void
wlc_lcn40phy_amuxsel_get(phy_info_t *pi, uint16 *save_ovr, uint16 *save_ovr_val)
{
	/* AMUX SEL logic */
	*save_ovr =
		PHY_REG_READ(pi, LCN40PHY, TxPwrCtrlRfCtrlOverride0, amuxSelPortOverride);
	*save_ovr_val =
		PHY_REG_READ(pi, LCN40PHY, TxPwrCtrlRfCtrlOverride0, amuxSelPortOverrideVal);
}

static void
wlc_lcn40phy_amuxsel_set(phy_info_t *pi, uint16 save_ovr, uint16 save_ovr_val)
{
	PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlRfCtrlOverride0, amuxSelPortOverride, save_ovr);
	PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlRfCtrlOverride0, amuxSelPortOverrideVal, save_ovr_val);

}

static void
wlc_lcn40phy_papd_block_enable(phy_info_t *pi, bool enable)
{
	/* Disable filters, turn off PAPD */
	PHY_REG_MOD(pi, LCN40PHY, papd_control, papdCompEn, enable);
	PHY_REG_MOD(pi, LCN40PHY, sslpnCalibClkEnCtrl, papdTxClkEn, enable);
	PHY_REG_MOD(pi, LCN40PHY, txfefilterconfig, cck_papden, enable);
	PHY_REG_MOD(pi, LCN40PHY, txfefilterconfig, ofdm_papden, enable);
	PHY_REG_MOD(pi, LCN40PHY, txfefilterconfig, ht_papden, enable);
}

static bool
wlc_lcn40phy_is_papd_block_enable(phy_info_t *pi)
{
	/* Disable filters, turn off PAPD */
	return (bool)(phy_utils_read_phyreg(pi, LCN40PHY_papd_control) &
		LCN40PHY_papd_control_papdCompEn_MASK);
}

static void
wlc_lcn40phy_papd_loopback(phy_info_t *pi, lcn40phy_papd_lin_path_t papd_lin_path)
{
	PHY_REG_MOD(pi, LCN40PHY, rfoverride7, lpf_hpc_ovr, 0x1);
	PHY_REG_MOD(pi, LCN40PHY, rfoverride7val, lpf_hpc_ovr_val, 0x1);

	if (LCN40_LINPATH(papd_lin_path) == LCN40PHY_PAPDLIN_EPA) {
		PHY_REG_LIST_START
			/* LPF gain */
			RADIO_REG_MOD_ENTRY(RADIO_2065_OVR6,	   0x1000, 1 << 12)
			RADIO_REG_MOD_ENTRY(RADIO_2065_LPF_GAIN, 0x78,   0)
			RADIO_REG_MOD_ENTRY(RADIO_2065_OVR5,	   0x1,    1)
			RADIO_REG_MOD_ENTRY(RADIO_2065_LPF_GAIN, 0x7,    0)
			RADIO_REG_OR_ENTRY(RADIO_2065_TOP_SPARE1, 0x0001)

			/* txrx2g_cal.calpath_pa_pu & .calpath_mix_pu */
			PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride4, papd_cal_path_pwrup_ovr, 1)
			PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride4val, papd_cal_path_pwrup_ovr_val, 0)

			/* logen2g_rxdiv.pu_bias and rxrf2g_cfg1.globe_pu */
			PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride8, fast_nap_bias_pu_ovr, 1)
			PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride8val, fast_nap_bias_pu_ovr_val, 1)

			/* logen2g_rxdiv.pu */
			PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride8, logen_rx_pu_ovr, 1)
			PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride8val, logen_rx_pu_ovr_val, 1)

			/* lna2g_cfg2.lna2_pu */
			PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride8, lna2_pu_ovr, 1)
			PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride8val, lna2_pu_ovr_val, 1)
		PHY_REG_LIST_EXECUTE(pi);

		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			PHY_REG_LIST_START
				RADIO_REG_MOD_ENTRY(RADIO_2065_OVR7,		0x100,	1 << 8)
				RADIO_REG_MOD_ENTRY(RADIO_2065_LPF_GAIN,	0xf00,	3 << 8)

				RADIO_REG_MOD_ENTRY(RADIO_2065_TIA_CFG1,	0x1,	1)
				RADIO_REG_MOD_ENTRY(RADIO_2065_OVR12,		0x200,	1 << 9)
				RADIO_REG_MOD_ENTRY(RADIO_2065_TIA_CFG1,	0x4,	1 << 2)
				RADIO_REG_MOD_ENTRY(RADIO_2065_OVR12,		0x100,	1 << 8)
				RADIO_REG_MOD_ENTRY(RADIO_2065_TIA_CFG1,	0x2,	1 << 1)
				RADIO_REG_MOD_ENTRY(RADIO_2065_OVR12,		0x2000,	1 << 13)

				RADIO_REG_MOD_ENTRY(RADIO_2065_LNA2G_CFG1,	0x1,	0)
				RADIO_REG_MOD_ENTRY(RADIO_2065_OVR3,		0x80,	1 << 7)
				RADIO_REG_MOD_ENTRY(RADIO_2065_OVR3,		0x2,	1 << 1)
				RADIO_REG_MOD_ENTRY(RADIO_2065_LNA2G_CFG1,	0x8,	1 << 3)
			PHY_REG_LIST_EXECUTE(pi);
		}
#ifdef BAND5G
		else { /* A-band */
			PHY_REG_LIST_START
				RADIO_REG_MOD_ENTRY(RADIO_2065_OVR7,		0x100,	1 << 8)
				RADIO_REG_MOD_ENTRY(RADIO_2065_LPF_GAIN,	0xf00,	3 << 8)

				RADIO_REG_MOD_ENTRY(RADIO_2065_TIA_CFG1,	0x10,	1 << 4)
				RADIO_REG_MOD_ENTRY(RADIO_2065_OVR12,		0x80,	1 << 7)

				RADIO_REG_MOD_ENTRY(RADIO_2065_TIA_CFG1,	0x1,	1)
				RADIO_REG_MOD_ENTRY(RADIO_2065_OVR12,		0x200,	1 << 9)
				RADIO_REG_MOD_ENTRY(RADIO_2065_TIA_CFG1,	0x4,	1 << 2)
				RADIO_REG_MOD_ENTRY(RADIO_2065_OVR12,		0x100,	1 << 8)
				RADIO_REG_MOD_ENTRY(RADIO_2065_TIA_CFG1,	0x2,	1 << 1)
				RADIO_REG_MOD_ENTRY(RADIO_2065_OVR12,		0x2000,	1 << 13)

				RADIO_REG_MOD_ENTRY(RADIO_2065_LNA5G_CFG1,	0x1,	0)
				RADIO_REG_MOD_ENTRY(RADIO_2065_OVR4,		0x200,	1 << 9)
				RADIO_REG_MOD_ENTRY(RADIO_2065_LNA5G_CFG1,	0x4,	1 << 2)
				RADIO_REG_MOD_ENTRY(RADIO_2065_OVR4,		0x80,	1 << 7)

				RADIO_REG_MOD_ENTRY(RADIO_2065_LNA5G_CFG1,	0x700,	0 << 8)
				RADIO_REG_MOD_ENTRY(RADIO_2065_OVR4,		0x2000,	1 << 13)
			PHY_REG_LIST_EXECUTE(pi);
		}
#endif /* BAND5G */

		wlc_lcn40phy_rx_gain_override_enable(pi, TRUE);
		wlc_lcn40phy_set_rx_gain_by_distribution(pi, 1, 0, 1, 0, 0, 0, 0, 0,
			0, 0, 0, 0);
	} else { /* iPA or PAD */

		/* PAPD loopback path PU and atten */
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			PHY_REG_LIST_START
				RADIO_REG_MOD_ENTRY(RADIO_2065_OVR12,		0x10,	1 << 4)
				RADIO_REG_MOD_ENTRY(RADIO_2065_TXRX2G_CAL,	0x2,	1 << 1)
				RADIO_REG_MOD_ENTRY(RADIO_2065_OVR11,		0x8000,	1 << 15)
				RADIO_REG_MOD_ENTRY(RADIO_2065_TXRX2G_CAL,	0x1,	1)
			PHY_REG_LIST_EXECUTE(pi);
			if (LCN40REV_IS(pi->pubpi.phy_rev, 7))
				phy_utils_mod_radioreg(pi, RADIO_2065_TXRX5G_CAL_BAK, 0x4, 1 << 2);

			if (LCN40_LINPATH(papd_lin_path) == LCN40PHY_PAPDLIN_PAD) {
				PHY_REG_LIST_START
				    RADIO_REG_MOD_ENTRY(RADIO_2065_TXRX2G_CAL,	0xc00,	0 << 10)
				    RADIO_REG_MOD_ENTRY(RADIO_2065_TXRX2G_CAL,	0x300,	0 << 8)

				    RADIO_REG_MOD_ENTRY(RADIO_2065_OVR7,	0x100,	1 << 8)
				    RADIO_REG_MOD_ENTRY(RADIO_2065_LPF_GAIN,	0xf00,	3 << 8)
				PHY_REG_LIST_EXECUTE(pi);
			} else {
				if (LCN40REV_IS(pi->pubpi.phy_rev, 7)) {
					phy_utils_mod_radioreg(pi, RADIO_2065_TXRX2G_CAL,
					                       0xc0,  0 << 6);
					phy_utils_mod_radioreg(pi, RADIO_2065_TXRX2G_CAL,
					                       0x30,  1 << 4);
				} else {
					phy_utils_mod_radioreg(pi, RADIO_2065_TXRX2G_CAL,
					                       0xc0,  3 << 6);
					phy_utils_mod_radioreg(pi, RADIO_2065_TXRX2G_CAL,
					                       0x30,  1 << 4);
				}
			}
		}
#ifdef BAND5G
		else {
			PHY_REG_LIST_START
				RADIO_REG_MOD_ENTRY(RADIO_2065_OVR13,		0x2000,	1 << 13)
				RADIO_REG_MOD_ENTRY(RADIO_2065_TXRX5G_CAL,	0x2,	1 << 1)
				RADIO_REG_MOD_ENTRY(RADIO_2065_OVR11,		0x4000,	1 << 14)
				RADIO_REG_MOD_ENTRY(RADIO_2065_TXRX5G_CAL,	0x1,	1)
			PHY_REG_LIST_EXECUTE(pi);
			if (LCN40REV_IS(pi->pubpi.phy_rev, 7))
				phy_utils_mod_radioreg(pi, RADIO_2065_TXRX5G_CAL_BAK, 0x8, 1 << 3);

			if (LCN40_LINPATH(papd_lin_path) == LCN40PHY_PAPDLIN_PAD) {
				PHY_REG_LIST_START
				    RADIO_REG_MOD_ENTRY(RADIO_2065_TXRX5G_CAL,	0xc00,	0 << 10)
				    RADIO_REG_MOD_ENTRY(RADIO_2065_TXRX5G_CAL,	0x300,	0 << 8)

				    RADIO_REG_MOD_ENTRY(RADIO_2065_OVR7,	0x100,	1 << 8)
				    RADIO_REG_MOD_ENTRY(RADIO_2065_LPF_GAIN,	0xf00,	3 << 8)
				PHY_REG_LIST_EXECUTE(pi);
			} else {
			    if (LCN40REV_IS(pi->pubpi.phy_rev, 7)) {
					phy_utils_mod_radioreg(pi, RADIO_2065_TXRX5G_CAL,
					                       0xf000, 1 << 14);
				} else {
					phy_utils_mod_radioreg(pi, RADIO_2065_TXRX5G_CAL,
					                       0xc00, 0 << 10);
					phy_utils_mod_radioreg(pi, RADIO_2065_TXRX5G_CAL,
					                       0x300, 0 << 8);
				}
			}
		}
#endif /* BAND5G */

		if (CHIPID(pi->sh->chip) == BCM4314_CHIP_ID ||
			CHIPID(pi->sh->chip) == BCM43142_CHIP_ID ||
			CHIPID(pi->sh->chip) == BCM43143_CHIP_ID)
			phy_utils_write_radioreg(pi, RADIO_2065_RX_REG_BACKUP_1, 0x1);

		PHY_REG_LIST_START
			/* shut off LNA's */
			PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride2, slna_pu_ovr, 1)
			PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride2val, slna_pu_ovr_val, 0)
			/* LPF gain */
			RADIO_REG_MOD_ENTRY(RADIO_2065_OVR6,			0x1000,	1 << 12)
			RADIO_REG_MOD_ENTRY(RADIO_2065_LPF_GAIN,		0x78,	0)
			RADIO_REG_MOD_ENTRY(RADIO_2065_OVR5,			0x1,	1)
			RADIO_REG_MOD_ENTRY(RADIO_2065_LPF_GAIN,		0x7,	0)
			RADIO_REG_OR_ENTRY(RADIO_2065_TOP_SPARE1,		0x0001)
		PHY_REG_LIST_EXECUTE(pi);

		/* various Rx settings, ground LNA1 input etc. */
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			PHY_REG_LIST_START
				RADIO_REG_MOD_ENTRY(RADIO_2065_TIA_CFG1,	0x1,	1)
				RADIO_REG_MOD_ENTRY(RADIO_2065_OVR12,		0x200,	1 << 9)
				RADIO_REG_MOD_ENTRY(RADIO_2065_TIA_CFG1,	0x4,	1 << 2)
				RADIO_REG_MOD_ENTRY(RADIO_2065_OVR12,		0x100,	1 << 8)
				RADIO_REG_MOD_ENTRY(RADIO_2065_TIA_CFG1,	0x2,	1 << 1)
				RADIO_REG_MOD_ENTRY(RADIO_2065_OVR12,		0x2000,	1 << 13)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RXMIX2G_CFG1,	0x1,	1)
				RADIO_REG_MOD_ENTRY(RADIO_2065_OVR11, 		0x800,	1 << 11)
				RADIO_REG_MOD_ENTRY(RADIO_2065_LNA2G_CFG1,	0x1,	0)
				RADIO_REG_MOD_ENTRY(RADIO_2065_OVR3,		0x80,	1 << 7)
				RADIO_REG_MOD_ENTRY(RADIO_2065_OVR3,		0x2,	1 << 1)
				RADIO_REG_MOD_ENTRY(RADIO_2065_LNA2G_CFG1,	0x8,	0)
				RADIO_REG_MOD_ENTRY(RADIO_2065_LNA2G_CFG2,	0x1,	0)
				RADIO_REG_MOD_ENTRY(RADIO_2065_OVR3,		0x8,	1 << 3)
			PHY_REG_LIST_EXECUTE(pi);
			if (CHIPID(pi->sh->chip) == BCM43143_CHIP_ID)
				phy_utils_mod_radioreg(pi, RADIO_2065_LNA2G_CFG1, 0x4, 1 << 2);
			else if (LCN40REV_IS(pi->pubpi.phy_rev, 7))
				wlc_lcn40phy_set_trsw_override(pi, TRUE, FALSE);
			else {
				phy_utils_mod_radioreg(pi, RADIO_2065_PA2G_CFG1, 0x20, 0);
				phy_utils_mod_radioreg(pi, RADIO_2065_OVR12, 0x40, 1 << 6);
			}
			PHY_REG_LIST_START
				RADIO_REG_MOD_ENTRY(RADIO_2065_OVR4,		0x8,	1 << 3)
				RADIO_REG_MOD_ENTRY(RADIO_2065_LOGEN2G_RXDIV,	0x1,	1)
				RADIO_REG_MOD_ENTRY(RADIO_2065_OVR4,		0x4,	1 << 2)
				RADIO_REG_MOD_ENTRY(RADIO_2065_LOGEN2G_RXDIV,	0x2,	1 << 1)
				RADIO_REG_MOD_ENTRY(RADIO_2065_OVR11,		0x40,	1 << 6)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RXRF2G_CFG1,	0x1,	1)
			PHY_REG_LIST_EXECUTE(pi);

			/* extra settings for better isolation at LNA1/2 */
			if (LCN40_LINPATH(papd_lin_path) == LCN40PHY_PAPDLIN_PAD) {
				PHY_REG_LIST_START
				    RADIO_REG_MOD_ENTRY(RADIO_2065_OVR3,	0x40,	1 << 6)
				    RADIO_REG_MOD_ENTRY(RADIO_2065_LNA2G_CFG1,	0xf00,	15 << 8)
				    RADIO_REG_MOD_ENTRY(RADIO_2065_OVR3,	0x4,	1 << 2)
				    RADIO_REG_MOD_ENTRY(RADIO_2065_LNA2G_CFG2,	0xf0,	15 << 4)
				    RADIO_REG_MOD_ENTRY(RADIO_2065_LNA2G_CFG1,	0x4,	1 << 2)
				PHY_REG_LIST_EXECUTE(pi);
			}
		}
#ifdef BAND5G
		else { /* A-band */
			PHY_REG_LIST_START
				RADIO_REG_MOD_ENTRY(RADIO_2065_TIA_CFG1, 0x10,       1 << 4)
				RADIO_REG_MOD_ENTRY(RADIO_2065_OVR12,    0x80,       1 << 7)

				RADIO_REG_MOD_ENTRY(RADIO_2065_TIA_CFG1, 0x1,        1)
				RADIO_REG_MOD_ENTRY(RADIO_2065_OVR12,    0x200,      1 << 9)
				RADIO_REG_MOD_ENTRY(RADIO_2065_TIA_CFG1, 0x4,        1 << 2)
				RADIO_REG_MOD_ENTRY(RADIO_2065_OVR12,    0x100,      1 << 8)

				RADIO_REG_MOD_ENTRY(RADIO_2065_TIA_CFG1, 0x2,        1 << 1)
				RADIO_REG_MOD_ENTRY(RADIO_2065_OVR12,    0x2000,     1 << 13)

				RADIO_REG_MOD_ENTRY(RADIO_2065_RXMIX5G_CFG1, 0x1,    1)
				RADIO_REG_MOD_ENTRY(RADIO_2065_OVR11,        0x80,   1 << 7)
				RADIO_REG_MOD_ENTRY(RADIO_2065_LNA5G_CFG1,   0x1,    0)
				RADIO_REG_MOD_ENTRY(RADIO_2065_OVR4,         0x200,  1 << 9)
				RADIO_REG_MOD_ENTRY(RADIO_2065_LNA5G_CFG1,   0x4,    0 << 2)
				RADIO_REG_MOD_ENTRY(RADIO_2065_OVR4,         0x80,   1 << 7)

				RADIO_REG_MOD_ENTRY(RADIO_2065_LNA5G_CFG1,   0x700,  0 << 8)
				RADIO_REG_MOD_ENTRY(RADIO_2065_OVR4,         0x2000, 1 << 13)

				RADIO_REG_MOD_ENTRY(RADIO_2065_LNA5G_CFG2,   0x1,    0)
				RADIO_REG_MOD_ENTRY(RADIO_2065_OVR4,         0x100,  1 << 8)
				RADIO_REG_MOD_ENTRY(RADIO_2065_RXRF5G_CFG1,  0x1,    1)
				RADIO_REG_MOD_ENTRY(RADIO_2065_OVR11,        0x10,   1 << 4)

				RADIO_REG_MOD_ENTRY(RADIO_2065_LOGEN5G_CFG1, 0x4,    1 << 2)
				RADIO_REG_MOD_ENTRY(RADIO_2065_OVR5,         0x100,  1 << 8)
			PHY_REG_LIST_EXECUTE(pi);
		}
#endif /* BAND5G */

		wlc_lcn40phy_rx_gain_override_enable(pi, TRUE);
		wlc_lcn40phy_set_rx_gain_by_distribution(pi, 1, 0, 0, 0, 2, 0, 0, 0,
			0, 0, 0, 0);
	}

	PHY_REG_LIST_START
		/* disable aux (tssi etc.) path */
		RADIO_REG_MOD_ENTRY(RADIO_2065_AUXPGA_CFG1, 0x3000, 0)
		RADIO_REG_MOD_ENTRY(RADIO_2065_AUXPGA_CFG1, 0x30,   1 << 4)
		RADIO_REG_MOD_ENTRY(RADIO_2065_OVR1, 0x8000, 1 << 15)
		RADIO_REG_MOD_ENTRY(RADIO_2065_AUXPGA_CFG1, 0x1, 0)
		/*	lpf_pu in AMS */
		RADIO_REG_MOD_ENTRY(RADIO_2065_OVR6, 0x10, 1 << 4)
		RADIO_REG_MOD_ENTRY(RADIO_2065_LPF_CFG1, 0xc,  3 << 2)
		RADIO_REG_MOD_ENTRY(RADIO_2065_OVR6, 0x8,  1 << 3)
		RADIO_REG_MOD_ENTRY(RADIO_2065_LPF_CFG1, 0x3,  3)
		/* lpf_rxbuf_pu in AMS */
		RADIO_REG_MOD_ENTRY(RADIO_2065_OVR7, 0x80, 1 << 7)
		RADIO_REG_MOD_ENTRY(RADIO_2065_LPF_CFG1, 0x80, 1 << 7)
		RADIO_REG_MOD_ENTRY(RADIO_2065_OVR7, 0x40, 1 << 6)
		RADIO_REG_MOD_ENTRY(RADIO_2065_LPF_CFG1, 0x40, 1 << 6)
		/* sel_tx_rx in AMS */
		RADIO_REG_MOD_ENTRY(RADIO_2065_LPF_CFG1, 0xc00, 3 << 10)
		RADIO_REG_MOD_ENTRY(RADIO_2065_OVR7, 0x4,   1 << 2)
	PHY_REG_LIST_EXECUTE(pi);
	/* sel_byp_txlpf in AMS */
	if (pi->tx_alpf_bypass) {
		phy_utils_mod_radioreg(pi, RADIO_2065_OVR7, 0x10, 1 << 4);
		phy_utils_mod_radioreg(pi, RADIO_2065_LPF_CFG1, 0x200, 1 << 9);
	} else {
		phy_utils_mod_radioreg(pi, RADIO_2065_OVR7, 0x10, 1 << 4);
		phy_utils_mod_radioreg(pi, RADIO_2065_LPF_CFG1, 0x200, 0 << 9);
	}
	PHY_REG_LIST_START
		/* sel_byp_rxlpf in AMS */
		RADIO_REG_MOD_ENTRY(RADIO_2065_LPF_CFG1, 0xc000, 1 << 14)
		RADIO_REG_MOD_ENTRY(RADIO_2065_OVR7, 0x20, 1 << 5)
		/* sel_rx_buf in AMS */
		RADIO_REG_MOD_ENTRY(RADIO_2065_LPF_CFG1, 0x3000, 1 << 12)
		RADIO_REG_MOD_ENTRY(RADIO_2065_OVR7, 0x8,    1 << 3)
	PHY_REG_LIST_EXECUTE(pi);
}

static int16
wlc_lcn40phy_calc_angle(phy_info_t *pi, int16 valQ, int16 valI)
{
	/* LUT degree in Q2 */
	int16 degLut[] = {180, 106, 56, 29, 14, 7, 4, 2};
	int16 ang = 0;
	int16 x, y;
	int16 j;

	/* 8-iteration CORDIC; proc returns angle ~[-180,180] deg (i.e. similar to atan2) */
	if (valI < 0) {
		x = -valI;
		y = -valQ;
	} else {
		x = valI;
		y = valQ;
	}

	for (j = 0; j < ARRAYSIZE(degLut); j++) {
		int16 xp, yp;

		if (y > 0) {
			xp = x + (y >> j);
			yp = y - (x >> j);
			ang = ang + degLut[j];
		} else if (y < 0) {
			xp = x - (y >> j);
			yp = y + (x >> j);
			ang = ang - degLut[j];
		} else {
			xp = x;
			yp = y;
		}

		x = xp;
		y = yp;
	}

	ang = (ang+2)>>2; /* convert to Q0 */

	if (valI < 0) {
		if (valQ > 0)
			ang = ang + 180;
		else
			ang = ang - 180;
	}

	return ang;
}

static int16
wlc_lcn40phy_papd_amamTh(phy_info_t *pi)
{
	int16 amamTh = 0;
	phy_info_lcn40phy_t *pi_lcn40 = pi->u.pi_lcn40phy;

	if (CHSPEC_IS2G(pi->radio_chanspec) &&
		(pi_lcn40->papd_mag_th_2g != -1))
		amamTh = pi_lcn40->papd_mag_th_2g;
#ifdef BAND5G
	else if (CHSPEC_IS5G(pi->radio_chanspec) &&
		(pi_lcn40->papd_mag_th_5g != -1))
		amamTh = pi_lcn40->papd_mag_th_5g;
#endif /* BAND5G */

	return amamTh;
}

#define PAPD_PHASE_DELTA_TH 10

static int16
wlc_lcn40phy_papd_phase_jump_det(phy_info_t *pi)
{
	int16 idx = -1;
	int16 maxUpdtIdx, minUpdtIdx;
	int16 amamTh;
	phytbl_info_t tab;
	int32 raw;
	int16 i, j, a;
	int16 angle1, angle2, delta;
	int32 limit, absSquared2;

	wlc_lcn40phy_GetpapdMaxMinIdxupdt(pi, &maxUpdtIdx, &minUpdtIdx);

	if (minUpdtIdx == 0)
		minUpdtIdx = 1;
	else if (maxUpdtIdx == 63)
		maxUpdtIdx = 62;

	amamTh = wlc_lcn40phy_papd_amamTh(pi);
	limit = amamTh*amamTh;

	/* search within the updated portion from left to right */
	tab.tbl_id = LCN40PHY_TBL_ID_PAPDCOMPDELTATBL;
	tab.tbl_ptr = &raw; /* ptr to buf */
	tab.tbl_width = 32;
	tab.tbl_len = 1;
	tab.tbl_offset = minUpdtIdx;
	wlc_lcnphy_read_table(pi, &tab);

	i = (int16)((raw >> 12) << 4);
	i = i >> 4;
	j = (int16)((raw & 0xfff) << 4);
	j = j >> 4;
	angle1 = wlc_lcn40phy_calc_angle(pi, j, i);

	for ((a = minUpdtIdx+1); a <= maxUpdtIdx; a++) {
		tab.tbl_offset = a;
		wlc_lcnphy_read_table(pi, &tab);
		i = (int16)((raw >> 12) << 4);
		i = i >> 4;
		j = (int16)((raw & 0xfff) << 4);
		j = j >> 4;
		angle2 = wlc_lcn40phy_calc_angle(pi, j, i);
		absSquared2 = ((i*i) + (j*j));

		/* no need to compare if the 2nd point is above AMAM threshold since
		  * it won't be part of LUT anyway.
		  */
		if (limit != 0) {
			if (absSquared2 > limit)
				break;
		}

		/* assume reverse-cal */
		delta = angle2 - angle1;

		if (ABS(delta) > 180) {
			if (delta < 0) {
				delta = delta + 360;
			} else {
				delta = delta - 360;
			}
		}

		if (delta > PAPD_PHASE_DELTA_TH) {
			idx = a;
			break;
		}

		angle1 = angle2;
	}

	return idx;
}

/* 43143 BW40 PAPD cal phase jump detection; RxFIFO overflow will cause phase jump of 22.5 degrees.
 * Set threshold to 18 degrees to account for phase changes from PAPD calibration itself
 */
#define PAPD_PHASE_JUMP_TH 18

/* Calculate angle of PAPD epsilon value at offset in the PAPD epsilon table
 * This is a helper function for wlc_lcn40phy_papd_phase_jump_corr().
 */
static int32
wlc_lcn40phy_papd_angle(phy_info_t *pi, phytbl_info_t *tab, int16 offset)
{
	int32 raw;
	int32 angle;
	math_cint32 eps;

	/* read epsilon from PAPD table */
	tab->tbl_ptr = &raw;
	tab->tbl_offset = offset;
	wlc_lcnphy_read_table(pi, tab);

	/* convert fron integer to s15.16 */
	if ((eps.i = FIXED(raw >> 12)) > FIXED(0x7ff))
		eps.i -= FIXED(0x1000); /* Sign extend */
	if ((eps.q = FIXED(raw & 0xfff)) > FIXED(0x7ff))
		eps.q -= FIXED(0x1000); /* Sign extend */

	/* calculate angle (returned in s15.16) */
	phy_utils_invcordic(eps, &angle);
	return angle;
}

/* Rotate PAPD epsilon value at offset in the PAPD epsilon table by 22.5 degrees.
 * This is a helper function for wlc_lcn40phy_papd_phase_jump_corr().
 */
static void
wlc_lcn40phy_papd_rotate(phy_info_t *pi, phytbl_info_t *tab, int16 offset)
{
	/* complex value with magnitude 1 and phase 22.5 degrees in s15.16 format */
	const math_cint32 phase22_5 = {25079, 60546};
	int32 raw;
	math_cint32 eps, eps1;

	/* read current epsilon */
	tab->tbl_ptr = &raw;
	tab->tbl_offset = offset;
	wlc_lcnphy_read_table(pi, tab);

	/* don't scale from integer to s15.16; multiplication will do that implicitly */
	if ((eps.i = (raw >> 12)) > 0x7ff)
		eps.i -= 0x1000; /* Sign extend */
	if ((eps.q = (raw & 0xfff)) > 0x7ff)
		eps.q -= 0x1000; /* Sign extend */

	/* rotate by multiplying with complex number with magnitude 1 and angle 22.5 deg */
	eps1.i = eps.i * phase22_5.i - eps.q * phase22_5.q;
	eps1.q = eps.q * phase22_5.i + eps.i * phase22_5.q;

	/* write back to table */
	raw = (FLOAT(eps1.i) << 12) & 0xfff000;
	raw |= FLOAT(eps1.q) & 0xfff;
	wlc_lcnphy_write_table(pi, tab);
}

/* Interpolate point n+1 from points n and n+2 in the PAPD epsilon table.
 * This is a helper function for wlc_lcn40phy_papd_phase_jump_corr().
 */
static void
wlc_lcn40phy_papd_interpolate(phy_info_t *pi, phytbl_info_t *tab, int16 offset)
{
	int16 i[3], q[3];
	int32 raw;
	int idx;

	/* read epsilon values from PAPD table */
	tab->tbl_ptr = &raw;
	for (idx = 0; idx < 3; idx++) {
		tab->tbl_offset = offset + idx;
		wlc_lcnphy_read_table(pi, tab);
		if ((i[idx] = (raw >> 12)) > 0x7ff)
			i[idx] -= 0x1000;
		if ((q[idx] = (raw & 0xfff)) > 0x7ff)
			q[idx] -= 0x1000;
	}

	/* interpolate and write back */
	raw =  ((i[0] + i[2]) << 11) & 0xfff000;
	raw |= ((q[0] + q[2]) >>  1) & 0x000fff;
	tab->tbl_offset = offset + 1;
	wlc_lcnphy_write_table(pi, tab);
}

/* PAPD epsilon table phase jump detection and correction function for 43143 in BW40.
 * A phase jump of +22.5 degrees will happen when an RxFIFO overflow occurs.
 */
static void
wlc_lcn40phy_papd_phase_jump_corr(phy_info_t *pi)
{
	int16 idx = -1;
	phytbl_info_t tab;
	int16 a;
	int32 angle1, angle2, angle3, delta;

	if (PHY_REG_READ(pi, LCN40PHY, RxFeStatus, rx_fifo_overflow) == 1) {
		/* Search within the table from left to right.
		 * No need to look at points 0->1 and 62->63, because
		 * point 0 was copied from 1 and 63 from 62 earlier on.
		 */
		tab.tbl_id = LCN40PHY_TBL_ID_PAPDCOMPDELTATBL;
		tab.tbl_width = 32;
		tab.tbl_len = 1;
		angle1 = wlc_lcn40phy_papd_angle(pi, &tab, 1);
		angle2 = wlc_lcn40phy_papd_angle(pi, &tab, 2);

		for (a = 3; a < PHY_PAPD_EPS_TBL_SIZE_LCNPHY; a++) {
			angle3 = wlc_lcn40phy_papd_angle(pi, &tab, a);

			/* assume forward-cal */
			delta = angle1 - angle3;

			if (delta > FIXED(PAPD_PHASE_JUMP_TH)) {
				idx = a;
				break;
			}

			angle1 = angle2;
			angle2 = angle3;
		}

		if (idx > 0) {
			/* phase jump detected; first rotate current point */
			wlc_lcn40phy_papd_rotate(pi, &tab, idx);

			/* interpolate previous point; it very likely has a small(er) jump */
			wlc_lcn40phy_papd_interpolate(pi, &tab, idx - 2);

			/* rotate the rest of the points */
			for (a = idx + 1; a < PHY_PAPD_EPS_TBL_SIZE_LCNPHY; a++) {
				wlc_lcn40phy_papd_rotate(pi, &tab, a);
			}
		}
	}
}

static void
wlc_lcn40phy_per_modulation_papd(
	phy_info_t *pi,
	uint8 enable,
	uint16 txctrlwordOverridePapdLutswap,
	uint16 StandardOverridepapdLutswap,
	uint16 HigherQAMOverridepapdLutswap)
{
	/* Set to 0 to enable the LUT-switching feature.
	 * Note this bit also affects per-modulation 1) tx pwr offset and 2) radio reg write.
	 */
	PHY_REG_MOD(pi, LCN40PHY, RadioRegWriteAdd4, GlobalDisable, !enable);

	/* 	0 : 16/64-QAM vs PSK/CCK.
	*	1 : all Legacy-OFDM will use PapdDupLutCtrl.PapdLutSel1 and
	*	CCK will use PapdDupLutCtrl.PapdLutSel0.
	*/

	PHY_REG_MOD(pi, LCN40PHY, radioRegNewselectionScheme,
		newselectionScheme, 0);

	/* 4 lut select registers to determine which lut to use.
	 * Selection is controlled by Standard(MSB) and HigherQAM(LSB).
	 */

	/* L-PSK */
	PHY_REG_MOD(pi, LCN40PHY, PapdDupLutCtrl, PapdLutSel0, 1);
	/* L-QAM */
	PHY_REG_MOD(pi, LCN40PHY, PapdDupLutCtrl, PapdLutSel1, 0);
	/* HT-PSK */
	PHY_REG_MOD(pi, LCN40PHY, PapdDupLutCtrl, PapdLutSel2, 1);
	/* HT-QAM */
	PHY_REG_MOD(pi, LCN40PHY, PapdDupLutCtrl, PapdLutSel3, 0);

	/* Use constellation instead of gain-index to select LUT */
	PHY_REG_MOD(pi, LCN40PHY, PapdDupLutCtrl, gainbasedSelect, 0);

	/* Set txctrlwordOverridePapdLutswap to 1 to enable PHY overrides. */
	PHY_REG_MOD(pi, LCN40PHY, PapdDupLutCtrl,
		txctrlwordOverridePapdLutswap, txctrlwordOverridePapdLutswap);
	PHY_REG_MOD(pi, LCN40PHY, PapdDupLutCtrl,
		HigherQAMOverridepapdLutswap, HigherQAMOverridepapdLutswap);
	PHY_REG_MOD(pi, LCN40PHY, PapdDupLutCtrl,
		StandardOverridepapdLutswap, StandardOverridepapdLutswap);
}

static void
wlc_lcn40phy_papd_cal(
	phy_info_t *pi,
	phy_papd_cal_type_t cal_type,
	phy_txcalgains_t *txgains,
	bool frcRxGnCtrl,
	bool txGnCtrl,
	bool samplecapture,
	bool papd_dbg_mode,
	uint8 num_symbols,
	uint8 init_papd_lut,
	uint16 bbmult_step)
{
	uint8 bb_mult_old;
	uint16 SAVE_amuxSelPortOverride, SAVE_amuxSelPortOverrideVal;
	uint16 SAVE_upconv = 0;
	uint16 SAVE_txfefilterconfig_en;
	uint32 rxGnIdx;
	phytbl_info_t tab;
	uint32 tmpVar;
	uint32 refTxAnGn;
	uint8 CurTxGain;
	phy_txgains_t old_gains;
	uint16 lpf_ofdm_tx_bw;
	uint8 papd_peak_curr_mode = 0;
	int16 maxUpdtIdx = 1, minUpdtIdx = 1, j;
	uint16 *values_to_save_rf;
	uint16 SAVE_logen2g_txdiv = 0, SAVE_logen2g_rxdiv = 0;
	uint16 SAVE_rxrf2g_cfg1 = 0, SAVE_lna2g_cfg2 = 0, SAVE_rxmix2g_cfg1 = 0;
	uint16 SAVE_txrx2g_cal = 0, SAVE_pa2g_cfg1 = 0;
#ifdef BAND5G
	uint16 SAVE_rxrf5g_cfg1 = 0, SAVE_lna5g_cfg2 = 0, SAVE_rxmix5g_cfg1 = 0;
	uint16 SAVE_txrx5g_cal = 0, SAVE_logen5g_cfg1 = 0;
#endif /* BAND5G */
	uint16 values_to_save_phy[ARRAYSIZE(papd_cal_phy_reg)];
	uint16 rfArraySize;
	uint16 save_filtcoeffs[LCN40PHY_NUM_DIG_FILT_COEFFS+1];
	uint32 min_papd_lut_val;
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	phy_info_lcn40phy_t *pi_lcn40 = pi->u.pi_lcn40phy;
	uint16 bbmult_init;
	int16 valI, valQ;
	int16 amamTh = 0;
	int16 temp1, qQ1;

	ASSERT((cal_type == PHY_PAPD_CAL_CW));
	/* txgains should never be null */
	ASSERT(txgains != NULL);

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	PHY_PAPD(("Running papd cal, channel: %d cal type: %d\n",
		CHSPEC_CHANNEL(pi->radio_chanspec),
		cal_type));

	/* allocate memory for register save/restore */
	rfArraySize = ARRAYSIZE(papd_loopback_rf_reg2g);
#ifdef BAND5G
	if (CHSPEC_IS5G(pi->radio_chanspec))
		rfArraySize = ARRAYSIZE(papd_loopback_rf_reg5g);
#endif /* BAND5G */

	values_to_save_rf = LCN40PHY_MALLOC(pi, sizeof(uint16) * rfArraySize);
	if (values_to_save_rf == NULL) {
		PHY_ERROR(("wl%d: %s: MALLOC failure\n", pi->sh->unit, __FUNCTION__));
		return;
	}

	/*			   */
	/* Start setup */
	/*			   */
	bb_mult_old = wlc_lcn40phy_get_bbmult(pi); /* PAPD cal can modify this value */

	SAVE_txfefilterconfig_en = PHY_REG_READ(pi, LCN40PHY, txfefilterctrl, txfefilterconfig_en);
	PHY_REG_MOD(pi, LCN40PHY, txfefilterctrl, txfefilterconfig_en, 0);

	/* Disable PAPD */
	wlc_lcn40phy_papd_block_enable(pi, FALSE);

	/* AMUX SEL logic */
	wlc_lcn40phy_amuxsel_get(pi, &SAVE_amuxSelPortOverride,
		&SAVE_amuxSelPortOverrideVal);
	wlc_lcn40phy_amuxsel_set(pi, 1, 2);

	/* Save digital filter and set to OFDM coeffs */
	wlc_lcn40phy_save_restore_dig_filt_state(pi, TRUE, save_filtcoeffs);

	/*	In 40MHz bw mode, RTL picks the 20MHz OFDM filter */
	/*	which has 10MHz corner frequency and */
	/*	is used for 20-in-40 mode during sample play. */
	/*	load filter coefficients with 20MHz corner freq. into the 20MHz OFDM filter. */
	wlc_lcn40phy_load_tx_iir_filter(pi, TX_IIR_FILTER_OFDM, 12);

	if (LCN40REV_GE(pi->pubpi.phy_rev, 4) && CHSPEC_IS40(pi->radio_chanspec)) {
		SAVE_upconv = phy_utils_read_phyreg(pi, LCN40PHY_upconv);
		/* In 40MHz bw mode,
		* this override will disable the -10MHz shift in the +/-10MHz upconversion block.
		*/
		PHY_REG_MOD(pi, LCN40PHY, upconv, bw20in40_override, 1);
		PHY_REG_MOD(pi, LCN40PHY, upconv, bw20in40_val, 0);
	}

	/* Save all PHY regs that are changed later on */
	for (j = 0; j < ARRAYSIZE(papd_cal_phy_reg); j++)
		values_to_save_phy[j] = phy_utils_read_phyreg(pi, papd_cal_phy_reg[j]);

	/* Save RF registers changed by loopback */
	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		for (j = 0; j < rfArraySize; j++) {
			values_to_save_rf[j] =
			        phy_utils_read_radioreg(pi, papd_loopback_rf_reg2g[j]);
		}
		/* WAR : Restoring these regs in rev 4 and 5 are not taking effect, should be
		  * removed completely if the phy reg setting approach is adapted for other revs
		  */
		if (!LCN40REV_IS(pi->pubpi.phy_rev, 4) &&
			!LCN40REV_IS(pi->pubpi.phy_rev, 5)) {
			SAVE_logen2g_txdiv = phy_utils_read_radioreg(pi, RADIO_2065_LOGEN2G_TXDIV);
			SAVE_logen2g_rxdiv = phy_utils_read_radioreg(pi, RADIO_2065_LOGEN2G_RXDIV);
			SAVE_rxrf2g_cfg1   = phy_utils_read_radioreg(pi, RADIO_2065_RXRF2G_CFG1);
			SAVE_lna2g_cfg2    = phy_utils_read_radioreg(pi, RADIO_2065_LNA2G_CFG2);
			SAVE_rxmix2g_cfg1  = phy_utils_read_radioreg(pi, RADIO_2065_RXMIX2G_CFG1);
			SAVE_txrx2g_cal    = phy_utils_read_radioreg(pi, RADIO_2065_TXRX2G_CAL);
			SAVE_pa2g_cfg1     = phy_utils_read_radioreg(pi, RADIO_2065_PA2G_CFG1);
		}
	}
#ifdef BAND5G
	else {
		for (j = 0; j < rfArraySize; j++) {
			values_to_save_rf[j] =
			        phy_utils_read_radioreg(pi, papd_loopback_rf_reg5g[j]);
		}
		/* WAR : Restoring these regs in rev 4 and 5 are not taking effect, should be
		  * removed completely if the phy reg setting approach is adapted for other revs
		  */
		if (!LCN40REV_IS(pi->pubpi.phy_rev, 4) &&
			!LCN40REV_IS(pi->pubpi.phy_rev, 5)) {
			SAVE_rxrf5g_cfg1  = phy_utils_read_radioreg(pi, RADIO_2065_RXRF5G_CFG1);
			SAVE_lna5g_cfg2   = phy_utils_read_radioreg(pi, RADIO_2065_LNA5G_CFG2);
			SAVE_rxmix5g_cfg1 = phy_utils_read_radioreg(pi, RADIO_2065_RXMIX5G_CFG1);
			SAVE_txrx5g_cal   = phy_utils_read_radioreg(pi, RADIO_2065_TXRX5G_CAL);
			SAVE_logen5g_cfg1 = phy_utils_read_radioreg(pi, RADIO_2065_LOGEN5G_CFG1);
		}
	}
#endif /* BAND5G */

	/* Enable Rx gain override */
	wlc_lcn40phy_rx_gain_override_enable(pi, TRUE);

	/* Disable PAPRR block */
	PHY_REG_MOD(pi, LCN40PHY, papr_iir_group_dly, papr_override_enable, 1);
	PHY_REG_MOD(pi, LCN40PHY, papr_iir_group_dly, papr_enable, 0);

	/* set filter bandwidth in lpphy_rev0_rf_init, so it's common b/n cal and packets tx */
	/* tones use cck setting, we want to cal with ofdm filter setting */
	lpf_ofdm_tx_bw = PHY_REG_READ(pi, LCN40PHY, lpfbwlutreg1, lpf_ofdm_tx_bw);
	PHY_REG_MOD(pi, LCN40PHY, lpfbwlutreg1, lpf_cck_tx_bw, lpf_ofdm_tx_bw);

	PHY_REG_MOD(pi, LCN40PHY, AphyControlAddr, phyloopbackEn, 1);
	wlc_lcn40phy_tx_pu(pi, TRUE);
	PHY_REG_LIST_START
#ifdef BAND5G
		PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride4val, papu_ovr_val, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride4, papu_ovr, 1)
#endif /* BAND5G */

		/* This register should be toggled for papd to work after enabling dac power up */
		PHY_REG_MOD_ENTRY(LCN40PHY, AphyControlAddr, phyloopbackEn, 0)

		/* Force ADC on */
		PHY_REG_MOD_ENTRY(LCN40PHY, AfeCtrlOvr1, adc_pu_ovr, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, AfeCtrlOvr1Val, adc_pu_ovr_val, 31)
	PHY_REG_LIST_EXECUTE(pi);

	/* Enable Rx PU overrides */
	wlc_lcn40phy_rx_pu(pi, TRUE);
#ifdef BAND5G
	PHY_REG_MOD(pi, LCN40PHY, RFOverrideVal0, internalrfrxpu_ovr_val, 1);
	PHY_REG_MOD(pi, LCN40PHY, RFOverride0, internalrfrxpu_ovr, 1);
#endif /* BAND5G */

	/* Set tx gain */
	CurTxGain = pi_lcn->lcnphy_current_index;
	wlc_lcn40phy_get_tx_gain(pi, &old_gains);

	if (txgains->useindex) {
		wlc_lcn40phy_set_tx_pwr_by_index(pi, txgains->index);
		CurTxGain = txgains->index;
		PHY_PAPD(("txgainIndex = %d\n", CurTxGain));
	} else {
		wlc_lcn40phy_set_tx_gain(pi, &txgains->gains);
	}

	PHY_REG_LIST_START
		/* loft comp , iqmm comp enable */
		PHY_REG_OR_ENTRY(LCN40PHY, Core1TxControl, 0x0015)

		/* we need to bring SPB out of standby before using it */
		PHY_REG_MOD_ENTRY(LCN40PHY, sslpnCtrl3, sram_stby, 0)

		/* enable clk (including forceTxfiltClkOn) to SPB, PAPD blks and cal */
		PHY_REG_OR_ENTRY(LCN40PHY, sslpnCalibClkEnCtrl, 0x008f)
	PHY_REG_LIST_EXECUTE(pi);

	wlc_btcx_override_enable(pi);

	/* Set PAPD reference analog gain */
	tab.tbl_ptr = &refTxAnGn; /* ptr to buf */
	tab.tbl_len = 1;		/* # values   */
	tab.tbl_id = LCN40PHY_TBL_ID_TXPWRCTL;

	tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_PWR_OFFSET + txgains->index; /* tbl offset */
	tab.tbl_width = 32; 	/* 32 bit wide */
	wlc_lcnphy_read_table(pi, &tab);
	/* only the lower 10 bits */
	refTxAnGn = (refTxAnGn & 0x3FF);
	/* format change from x.4 to x.7 */
	refTxAnGn = refTxAnGn * 8;
	phy_utils_write_phyreg(pi, LCN40PHY_papd_tx_analog_gain_ref, (uint16)refTxAnGn);
	PHY_PAPD(("refTxAnGn = %d\n", refTxAnGn));

	/* Set papd loopback path */
	wlc_lcn40phy_papd_loopback(pi, pi_lcn40->papd_lin_path);

	PHY_REG_LIST_START
		/* WAR to force digi_gain; this must be placed BEFORE setting phyloopbackEn to 1 */
		PHY_REG_MOD_ENTRY(LCN40PHY, radioCtrl, digi_gain_ovr_val, 0)
		PHY_REG_MOD_ENTRY(LCN40PHY, radioCtrl, digi_gain_ovr, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, agcControl4, c_agc_fsm_en, 0)
		PHY_REG_MOD_ENTRY(LCN40PHY, agcControl4, c_agc_fsm_en, 1)
	PHY_REG_LIST_EXECUTE(pi);
	OSL_DELAY(2);
	PHY_REG_MOD(pi, LCN40PHY, agcControl4, c_agc_fsm_en, 0);

	PHY_REG_MOD(pi, LCN40PHY, sslpnCalibClkEnCtrl, forceTxfiltClkOn, 0x1);

	if (CHSPEC_IS20(pi->radio_chanspec) || LCN40REV_GE(pi->pubpi.phy_rev, 4)) {
		/* set to 1 to pick up correct ALPF setting */
		PHY_REG_MOD(pi, LCN40PHY, AphyControlAddr, phyloopbackEn, 1);
	}

	if (CHIPID(pi->sh->chip) == BCM43143_CHIP_ID)
		/* 43143 requires at least 2 us delay here; add some margin to be safe */
		/* TODO: see if this delay can be safely reduced */
		OSL_DELAY(10);
	else
		/* TODO: limit this huge delay to chips and/or boards that really need it */
		OSL_DELAY(10);

	/* Do Rx Gain Control */
	rxGnIdx = wlc_lcn40phy_papd_rxGnCtrl(pi, cal_type, frcRxGnCtrl, CurTxGain);

	/* Set Rx Gain */
	if (LCN40_LINPATH(pi_lcn40->papd_lin_path) == LCN40PHY_PAPDLIN_EPA) {
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			wlc_lcn40phy_set_rx_gain_by_distribution(pi, 1, 0, 1,
			PAPD_LNA1_ROUT_2G, 0, (pi_lcn40->papdrx2g & 0xf), 0,
			(uint16)rxGnIdx, 0, 0, 0, 0);
		}
#ifdef BAND5G
		else {
			wlc_lcn40phy_set_rx_gain_by_distribution(pi, 1, 0, 1,
			0, 0, (pi_lcn40->papdrx5g & 0xf), 0, (uint16)rxGnIdx,
			0, 0, 0, 0);
		}
#endif /* BAND5G */
	} else {
		wlc_lcn40phy_set_rx_gain_by_distribution(pi, 1, 0, 0,
		 0, 0, 0, 0, (uint16)rxGnIdx, 0, 0, 0, 0);
	}

	/* Set up bbmult init/step */
	if (CHSPEC_IS20(pi->radio_chanspec))
		bbmult_init = pi_lcn40->papd_bbmult_init_bw20;
	else
		bbmult_init = pi_lcn40->papd_bbmult_init_bw40;

	if (bbmult_step == 0) {
		if (CHSPEC_IS20(pi->radio_chanspec))
			bbmult_step = pi_lcn40->papd_bbmult_step_bw20;
		else
			bbmult_step = pi_lcn40->papd_bbmult_step_bw40;
	}

	if (CHIPID(pi->sh->chip) != BCM43143_CHIP_ID)
		/* TODO: limit this huge delay to chips and/or boards that really need it */
		OSL_DELAY(1000);

	if (!LCN40REV_IS(pi->pubpi.phy_rev, 5)) {
		/* Do PAPD Operation - All symbols in one go */
		wlc_lcn40phy_papd_cal_core(pi, cal_type,
			FALSE,
			txGnCtrl,
			samplecapture,
			papd_dbg_mode,
			num_symbols,
			init_papd_lut,
			bbmult_init,
			bbmult_step,
			0,
			0,
			512,
			0);
	} else {
		int32 gaindB, offsetdB, mx_final;
		uint32 V, LpGn;
		int16 lgI, lgQ, lgI_new, lgQ_new;
		uint16 bbmult_init_new, num_sym_short;
		bool SAVE_papd_stop_after_last_update = pi_lcn40->papd_stop_after_last_update;
		uint8 N = 8 - phy_utils_read_phyreg(pi, LCN40PHY_papd_cw_corr_norm);
		uint8 qscale = 16; /* Q4 scaler */

		/* Do the PAPD reverse cal */
		if (LCN40_LINPATH(pi_lcn40->papd_lin_path) != LCN40PHY_PAPDLIN_EPA) {
			PHY_ERROR(("wl%d: %s: Currently reverse-cal is only supported for ePA!\n",
				pi->sh->unit, __FUNCTION__));
			ASSERT(0);
		}

		V = phy_utils_sqrt_int((uint32)((phy_utils_read_phyreg(pi,
			LCN40PHY_papd_cw_sigma_yy_star_15_0)
			+ (phy_utils_read_phyreg(pi,
			LCN40PHY_papd_cw_sigma_yy_star_27_16)<<16)) * 4) >> N);

		lgI = ((int16) phy_utils_read_phyreg(pi, LCN40PHY_papd_loop_gain_cw_i)) << 4;
		lgI = lgI >> 4;
		lgQ = ((int16) phy_utils_read_phyreg(pi, LCN40PHY_papd_loop_gain_cw_q)) << 4;
		lgQ = lgQ >> 4;
		LpGn = phy_utils_sqrt_int((uint32) (lgI*lgI + lgQ*lgQ));

		mx_final = (V*LpGn + 256) >> 9;
		qm_log10(mx_final, 0, &temp1, &qQ1);
		gaindB = (((uint32)20*temp1*qscale + (1<<(qQ1-1))) >> qQ1) - PAPD_MX2BASE_DB;
		num_sym_short = (gaindB * PAPD_SYM_DB + 128) >> 8;
		PHY_PAPD(("V = %d, LpGn = %d, %d dB (%dX), symbols for LpGn update = %d\n",
			V, LpGn, gaindB, qscale, num_sym_short));

		/* required in case LUT[63] is the first entry updated during reverse-cal */
		pi_lcn40->papd_stop_after_last_update = 0;

		if (CHSPEC_IS20(pi->radio_chanspec)) {
			bbmult_init_new = pi_lcn40->papd_bbmult_init_bw20;
			bbmult_init     = pi_lcn40->papd_bbmult_init_rev_bw20;
			bbmult_step     = pi_lcn40->papd_bbmult_step_rev_bw20;
		} else {
			bbmult_init_new = pi_lcn40->papd_bbmult_init_bw40;
			bbmult_init     = pi_lcn40->papd_bbmult_init_rev_bw40;
			bbmult_step     = pi_lcn40->papd_bbmult_step_rev_bw40;
		}

		qm_log10((int32)LpGn, 0, &temp1, &qQ1);
		offsetdB = 867 - (((uint32)20*temp1*qscale + (1<<(qQ1-1))) >> qQ1);
		offsetdB = offsetdB + PAPD_REV_LUTDB*qscale;
		num_sym_short = num_sym_short + (((offsetdB * PAPD_SYM_DB) + 128) >> 8);
		bbmult_init_new = bbmult_init_new * mx_final * PAPD_REV_FAC / LpGn;

		if (bbmult_init_new > bbmult_init)
			bbmult_init_new = bbmult_init;

		PHY_PAPD(("short reverse-cal bbmult_init = %d, sym = %d\n",
			bbmult_init_new, num_sym_short));

		/* run a short reverse-cal to check the potential LUT[0] */
		wlc_lcn40phy_papd_cal_core(pi, cal_type,
			FALSE,
			txGnCtrl,
			samplecapture,
			papd_dbg_mode,
			num_sym_short,
			init_papd_lut,
			bbmult_init_new,
			bbmult_step,
			0,
			1,
			(uint16) lgI,
			(uint16) lgQ);

		tab.tbl_id = LCN40PHY_TBL_ID_PAPDCOMPDELTATBL;
		tab.tbl_offset = 1;
		tab.tbl_ptr = &tmpVar; /* ptr to buf */
		wlc_lcn40phy_read_table(pi, &tab);
		valI = (int16)((tmpVar >> 12) << 4);
		valI = valI >> 4;
		valQ = (int16)((tmpVar & 0xfff) << 4);
		valQ = valQ >> 4;
		PHY_PAPD(("|LUT[1]| * 128 = %d\n",
			phy_utils_sqrt_int((uint32)(valI*valI + valQ*valQ))));

		/* re-normalize loopgain such that final LUT[0] is close to "1"  */
		lgI_new = (lgI*valI - lgQ*valQ + 64) >> 7;
		lgQ_new = (lgQ*valI + lgI*valQ + 64) >> 7;
		OSL_DELAY(1000);

		/* run the final reverse-cal with adjusted loopgain and full symbols */
		wlc_lcn40phy_papd_cal_core(pi, cal_type,
			FALSE,
			txGnCtrl,
			samplecapture,
			papd_dbg_mode,
			num_symbols,
			init_papd_lut,
			bbmult_init,
			bbmult_step,
			0,
			1,
			(uint16) lgI_new,
			(uint16) lgQ_new);

		/* WAR to handle large phase jump due to rx_fifo_overflow
		* (mostly seen in 5G-BW40 mode). Note the process here assumes RX FIFO
		* is reset right before cal-engine enabling.
		*/
		if (CHSPEC_IS5G(pi->radio_chanspec)) {
			int16 idx = -1;

			if (PHY_REG_READ(pi, LCN40PHY, RxFeStatus, rx_fifo_overflow) == 1) {
				int8 loopcnt;

				idx = wlc_lcn40phy_papd_phase_jump_det(pi);
				for (loopcnt = 0; loopcnt < 3; loopcnt++) {
					if (idx != -1) {
						PHY_PAPD(("!!! Large phase discontinuity found "
							"at idx %d, re-run final cal \n", idx));
						OSL_DELAY(1000);
						wlc_lcn40phy_papd_cal_core(pi, cal_type,
							FALSE,
							txGnCtrl,
							samplecapture,
							papd_dbg_mode,
							num_symbols,
							init_papd_lut,
							bbmult_init,
							bbmult_step,
							0,
							1,
							(uint16) lgI_new,
							(uint16) lgQ_new);

						idx = wlc_lcn40phy_papd_phase_jump_det(pi);
					} else {
						PHY_PAPD(("No large phase discontinuity found "
							"extra cal = %d", loopcnt));
						break;
					}
				}
			}

			/* handle the case where all extra-cals failed */
			if (idx != -1) {
				phy_utils_write_phyreg(pi, LCN40PHY_papd_track_pa_lut_end, idx-1);
				PHY_PAPD(("!!! Extra-cal limit reached, cap papd lut end at %d !!!",
					phy_utils_read_phyreg(pi, LCN40PHY_papd_track_pa_lut_end)));
			}
		}

		pi_lcn40->papd_stop_after_last_update = SAVE_papd_stop_after_last_update;
	}

	wlc_phy_btcx_override_disable(pi);

	wlc_lcn40phy_GetpapdMaxMinIdxupdt(pi, &maxUpdtIdx, &minUpdtIdx);

	PHY_PAPD(("wl%d: %s max: %d, min: %d\n",
		pi->sh->unit, __FUNCTION__, maxUpdtIdx, minUpdtIdx));

	if ((minUpdtIdx >= 0) && (minUpdtIdx < PHY_PAPD_EPS_TBL_SIZE_LCN40PHY) &&
		(maxUpdtIdx >= 0) && (maxUpdtIdx < PHY_PAPD_EPS_TBL_SIZE_LCN40PHY) &&
		(minUpdtIdx <= maxUpdtIdx)) {

		if (cal_type == PHY_PAPD_CAL_CW) {
			tab.tbl_id = LCN40PHY_TBL_ID_PAPDCOMPDELTATBL;
			tab.tbl_offset = minUpdtIdx;
			tab.tbl_ptr = &tmpVar; /* ptr to buf */
			wlc_lcn40phy_read_table(pi, &tab);
			min_papd_lut_val = tmpVar;
			tab.tbl_offset = maxUpdtIdx;
			wlc_lcn40phy_read_table(pi, &tab);

			for (j = 0; j < minUpdtIdx; j++) {
				tmpVar = min_papd_lut_val;
				tab.tbl_offset = j;
				tab.tbl_ptr = &tmpVar;
				wlc_lcn40phy_write_table(pi, &tab);
			}
		}

		PHY_PAPD(("wl%d: %s: PAPD cal completed\n", pi->sh->unit, __FUNCTION__));
		if (papd_peak_curr_mode == 0) {
			tab.tbl_id = LCN40PHY_TBL_ID_PAPDCOMPDELTATBL;
			tab.tbl_ptr = &tmpVar; /* ptr to buf */
			tab.tbl_offset = 1;
			wlc_lcn40phy_read_table(pi, &tab);
			tab.tbl_offset = 0;
			wlc_lcn40phy_write_table(pi, &tab);
			tab.tbl_offset = 62;
			wlc_lcn40phy_read_table(pi, &tab);
			tab.tbl_offset = 63;
			wlc_lcn40phy_write_table(pi, &tab);

			/* i, j are two's complement. mask them out and convert them to int. */
			valI = (int16)((tmpVar >> 12) << 4);
			valI = valI >> 4;
			valQ = (int16)((tmpVar & 0xfff) << 4);
			valQ = valQ >> 4;
			PHY_PAPD(("max AMAM value * 128 = %d (real = %d, imag = %d)\n",
				phy_utils_sqrt_int((uint32)(valI*valI + valQ*valQ)), valI, valQ));
		}

		wlc_lcn40phy_save_papd_calibration_results(pi);
	}
	else
		PHY_PAPD(("Error in PAPD Cal. Exiting... \n"));

	/* remove any hump in eps curves */
	if (LCN40REV_IS(pi->pubpi.phy_rev, 5)) {
		int8 a;
		uint32 maxAbsSquared, rawlast;

		tab.tbl_id = LCN40PHY_TBL_ID_PAPDCOMPDELTATBL;
		tab.tbl_ptr = &tmpVar; /* ptr to buf */
		tab.tbl_offset = 63;
		wlc_lcn40phy_read_table(pi, &tab);
		rawlast = tmpVar;

		valI = (int16)((tmpVar >> 12) << 4);
		valI = valI >> 4;
		valQ = (int16)((tmpVar & 0xfff) << 4);
		valQ = valQ >> 4;
		maxAbsSquared = (uint32)(valI*valI + valQ*valQ);

		for (a = 62; a >= 0; a--) {
			uint32 absSquared;

			tab.tbl_offset = a;
			wlc_lcn40phy_read_table(pi, &tab);

			valI = (int16)((tmpVar >> 12) << 4);
			valI = valI >> 4;
			valQ = (int16)((tmpVar & 0xfff) << 4);
			valQ = valQ >> 4;
			absSquared = (uint32)(valI*valI + valQ*valQ);

			if (absSquared > maxAbsSquared) {
				tmpVar = rawlast;
				wlc_lcn40phy_write_table(pi, &tab);
			} else if (absSquared < maxAbsSquared)
				break;
		}
	}

	amamTh = wlc_lcn40phy_papd_amamTh(pi);

	/* 'Cap' the max AMAM value */
	if (amamTh != 0) {
		uint32 limit;
		int8 a, k;
		int8 startIdx = (int8) phy_utils_read_phyreg(pi, LCN40PHY_papd_track_pa_lut_end);

		limit = amamTh * amamTh;

		/* find the LUT index X that has maxAMAM <= threshold */
		for (a = startIdx; a >= 0; a--) {
			uint32 absSquared;

			tab.tbl_id = LCN40PHY_TBL_ID_PAPDCOMPDELTATBL;
			tab.tbl_ptr = &tmpVar; /* ptr to buf */
			tab.tbl_offset = a;
			wlc_lcn40phy_read_table(pi, &tab);

			valI = (int16)((tmpVar >> 12) << 4);
			valI = valI >> 4;
			valQ = (int16)((tmpVar & 0xfff) << 4);
			valQ = valQ >> 4;
			absSquared = (uint32)(valI*valI + valQ*valQ);

			if (absSquared <= limit)
				break;
		}

		if ((a < 63) && (a >= 0)) {
			tab.tbl_id = LCN40PHY_TBL_ID_PAPDCOMPDELTATBL;
			tab.tbl_ptr = &tmpVar; /* ptr to buf */
			/* Replicate the found value to the end of LUT */
			for (k = a; k < 64; k++) {
				tab.tbl_offset = k;
				wlc_lcn40phy_write_table(pi, &tab);
			}
		}
	}

	/* Clear LUT if TIA is too high */
	if (LCN40REV_IS(pi->pubpi.phy_rev, 5) && rxGnIdx > 7) {
		int8 k;
		tmpVar = 0x80000;
		tab.tbl_id = LCN40PHY_TBL_ID_PAPDCOMPDELTATBL;
		tab.tbl_ptr = &tmpVar; /* ptr to buf */
		for (k = 0; k < 64; k++) {
			tab.tbl_offset = k; /* tbl offset */
			wlc_lcn40phy_write_table(pi, &tab);
		}
	}

	/* store the final max AMAM value in dB */
	tab.tbl_id = LCN40PHY_TBL_ID_PAPDCOMPDELTATBL;
	tab.tbl_ptr = &tmpVar; /* ptr to buf */
	tab.tbl_offset = 63;
	wlc_lcn40phy_read_table(pi, &tab);

	valI = (int16)((tmpVar >> 12) << 4);
	valI = valI >> 4;
	valQ = (int16)((tmpVar & 0xfff) << 4);
	valQ = valQ >> 4;

	tmpVar = (valI * valI) + (valQ * valQ);
	qm_log10((int32)(tmpVar), 0, &temp1, &qQ1);
	pi_lcn40->max_amam_dB = ((10*temp1) - (42<<qQ1)) >> (qQ1-2);

	/* TR switch */
	wlc_lcn40phy_clear_trsw_override(pi);

	/* Disable Rx gain override */
	wlc_lcn40phy_rx_gain_override_enable(pi, FALSE);

	/* Restore register states; in exact reverse order of save */
	/* Restore RF regs */
	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		for (j = 0; j < rfArraySize; j++) {
			phy_utils_write_radioreg(pi, papd_loopback_rf_reg2g[j],
			                         values_to_save_rf[j]);
		}
		/* WAR : Restoring these regs in rev 4 and 5 are not taking effect, should be
		  * removed completely if the phy reg setting approach is adapted for other revs
		  */
		if (!LCN40REV_IS(pi->pubpi.phy_rev, 4) &&
			!LCN40REV_IS(pi->pubpi.phy_rev, 5)) {
			phy_utils_write_radioreg(pi, RADIO_2065_LOGEN2G_TXDIV, SAVE_logen2g_txdiv);
			phy_utils_write_radioreg(pi, RADIO_2065_LOGEN2G_RXDIV, SAVE_logen2g_rxdiv);
			phy_utils_write_radioreg(pi, RADIO_2065_RXRF2G_CFG1, SAVE_rxrf2g_cfg1);
			phy_utils_write_radioreg(pi, RADIO_2065_LNA2G_CFG2, SAVE_lna2g_cfg2);
			phy_utils_write_radioreg(pi, RADIO_2065_RXMIX2G_CFG1, SAVE_rxmix2g_cfg1);
			phy_utils_write_radioreg(pi, RADIO_2065_TXRX2G_CAL, SAVE_txrx2g_cal);
			phy_utils_write_radioreg(pi, RADIO_2065_PA2G_CFG1, SAVE_pa2g_cfg1);
		}
	}
#ifdef BAND5G
	else {
		for (j = 0; j < rfArraySize; j++) {
			phy_utils_write_radioreg(pi, papd_loopback_rf_reg5g[j],
			                         values_to_save_rf[j]);
		}

		/* WAR : Restoring these regs in rev 4 and 5 are not taking effect, should be
		  * removed completely if the phy reg setting approach is adapted for other revs
		  */
		if (!LCN40REV_IS(pi->pubpi.phy_rev, 4) &&
			!LCN40REV_IS(pi->pubpi.phy_rev, 5)) {
			phy_utils_write_radioreg(pi, RADIO_2065_RXRF5G_CFG1,  SAVE_rxrf5g_cfg1);
			phy_utils_write_radioreg(pi, RADIO_2065_LNA5G_CFG2,   SAVE_lna5g_cfg2);
			phy_utils_write_radioreg(pi, RADIO_2065_RXMIX5G_CFG1, SAVE_rxmix5g_cfg1);
			phy_utils_write_radioreg(pi, RADIO_2065_TXRX5G_CAL,   SAVE_txrx5g_cal);
			phy_utils_write_radioreg(pi, RADIO_2065_LOGEN5G_CFG1, SAVE_logen5g_cfg1);
		}
	}
#endif /* BAND5G */
	LCN40PHY_MFREE(pi, values_to_save_rf, sizeof(uint16) * rfArraySize);

	/* Restore PHY regs */
	for (j = 0; j < ARRAYSIZE(papd_cal_phy_reg); j++)
		phy_utils_write_phyreg(pi, papd_cal_phy_reg[j], values_to_save_phy[j]);

	if (LCN40REV_GE(pi->pubpi.phy_rev, 4) && CHSPEC_IS40(pi->radio_chanspec)) {
		phy_utils_write_phyreg(pi, LCN40PHY_upconv, SAVE_upconv);
	}

	/* Restore Digital Filter */
	wlc_lcn40phy_save_restore_dig_filt_state(pi, FALSE, save_filtcoeffs);

	/* Restore AMUX sel */
	wlc_lcn40phy_amuxsel_set(pi, SAVE_amuxSelPortOverride, SAVE_amuxSelPortOverrideVal);

	/* Enable PAPD */
	wlc_lcn40phy_papd_block_enable(pi, TRUE);

	PHY_REG_MOD(pi, LCN40PHY, txfefilterctrl, txfefilterconfig_en, SAVE_txfefilterconfig_en);

	/* restore bbmult */
	wlc_lcn40phy_set_bbmult(pi, (uint8)bb_mult_old);
}

static void
wlc_lcn40phy_perpkt_idle_tssi_est(phy_info_t *pi)
{
	bool suspend;
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	uint16 SAVE_txpwrctrl;
	uint8 SAVE_indx;
	uint16 SAVE_lpfgainBIQ1;
	uint16 SAVE_lpfgainBIQ2;

	suspend = !(R_REG(pi->sh->osh, &((phy_info_t *)pi)->regs->maccontrol) & MCTL_EN_MAC);
	if (!suspend)
		wlapi_suspend_mac_and_wait(pi->sh->physhim);

	SAVE_txpwrctrl = wlc_lcn40phy_get_tx_pwr_ctrl(pi);
	SAVE_indx = wlc_lcn40phy_get_current_tx_pwr_idx(pi);
	SAVE_lpfgainBIQ1 = phy_utils_read_radioreg(pi, RADIO_2065_OVR5);
	SAVE_lpfgainBIQ2 = phy_utils_read_radioreg(pi, RADIO_2065_OVR6) & 0x1000;

	wlc_lcn40phy_set_tx_pwr_ctrl(pi, LCN40PHY_TX_PWR_CTRL_OFF);

	wlc_lcn40phy_tssi_setup(pi);

	PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlCmdNew, txPwrCtrlScheme, 0);
#if TWO_POWER_RANGE_TXPWR_CTRL
	if (pi_lcn->lcnphy_twopwr_txpwrctrl_en) {
		PHY_REG_LIST_START
			PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlCmdNew, txPwrCtrlScheme, 2)
			PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlRfCtrl0, tssiRangeVal0, 1)
			PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlRfCtrl0, tssiRangeVal1, 0)
		PHY_REG_LIST_EXECUTE(pi);
	}
#endif  /* #if TWO_POWER_RANGE_TXPWR_CTRL */

	wlc_lcn40phy_set_txpwr_clamp(pi);

	PHY_REG_MOD(pi, LCN40PHY, RFOverride0, internalrftxpu_ovr, 0);

	wlc_lcn40phy_set_tx_pwr_by_index(pi, SAVE_indx);
	wlc_lcn40phy_set_tx_pwr_ctrl(pi, SAVE_txpwrctrl);
	PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlRangeCmd, cckPwrOffset,
		pi_lcn->cckPwrOffset + (pi_lcn->cckPwrIdxCorr<<1));

	/* restore radio registers */
	phy_utils_mod_radioreg(pi, RADIO_2065_OVR5, 1, SAVE_lpfgainBIQ1);
	phy_utils_mod_radioreg(pi, RADIO_2065_OVR6, 0x1000, SAVE_lpfgainBIQ2);

	if (!suspend)
		wlapi_enable_mac(pi->sh->physhim);
}

static void
#if TWO_POWER_RANGE_TXPWR_CTRL
wlc_lcn40phy_save_idletssi(phy_info_t *pi, uint16 idleTssi0_regvalue_2C,
uint16 idleTssi1_regvalue_2C)
#else
wlc_lcn40phy_save_idletssi(phy_info_t *pi, uint16 idleTssi0_regvalue_2C)
#endif // endif
{
	phy_info_lcn40phy_t *pi_lcn40 = pi->u.pi_lcn40phy;
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);

	switch (wlc_phy_chanspec_bandrange_get(pi, pi->radio_chanspec)) {
		case WL_CHAN_FREQ_RANGE_2G:
			pi_lcn40->lcn40_idletssi0_cache.idletssi_2g =
			idleTssi0_regvalue_2C;
		#if TWO_POWER_RANGE_TXPWR_CTRL
			if (pi_lcn->lcnphy_twopwr_txpwrctrl_en)
			pi_lcn40->lcn40_idletssi1_cache.idletssi_2g =
			idleTssi1_regvalue_2C;
		#endif  /* #if TWO_POWER_RANGE_TXPWR_CTRL */
			break;
#ifdef BAND5G
		case WL_CHAN_FREQ_RANGE_5GL:
			/* 5 GHz low */
			pi_lcn40->lcn40_idletssi0_cache.idletssi_5gl =
			idleTssi0_regvalue_2C;
		#if TWO_POWER_RANGE_TXPWR_CTRL
			if (pi_lcn->lcnphy_twopwr_txpwrctrl_en)
			pi_lcn40->lcn40_idletssi1_cache.idletssi_5gl =
			idleTssi1_regvalue_2C;
		#endif  /* #if TWO_POWER_RANGE_TXPWR_CTRL */
			break;

		case WL_CHAN_FREQ_RANGE_5GM:
			/* 5 GHz middle */
			pi_lcn40->lcn40_idletssi0_cache.idletssi_5gm =
			idleTssi0_regvalue_2C;
		#if TWO_POWER_RANGE_TXPWR_CTRL
			if (pi_lcn->lcnphy_twopwr_txpwrctrl_en)
			pi_lcn40->lcn40_idletssi1_cache.idletssi_5gm =
			idleTssi1_regvalue_2C;
		#endif  /* #if TWO_POWER_RANGE_TXPWR_CTRL */
			break;

		case WL_CHAN_FREQ_RANGE_5GH:
			/* 5 GHz high */
			pi_lcn40->lcn40_idletssi0_cache.idletssi_5gh =
			idleTssi0_regvalue_2C;
		#if TWO_POWER_RANGE_TXPWR_CTRL
			if (pi_lcn->lcnphy_twopwr_txpwrctrl_en)
			pi_lcn40->lcn40_idletssi1_cache.idletssi_5gh =
			idleTssi1_regvalue_2C;
		#endif  /* #if TWO_POWER_RANGE_TXPWR_CTRL */
			break;
#endif /* BAND5G */
		default:
			PHY_ERROR(("wl%d: %s: Bad channel/band\n",
				pi->sh->unit, __FUNCTION__));
			break;
	}
}

static void
wlc_lcn40phy_restore_idletssi(phy_info_t *pi)
{
	phy_info_lcn40phy_t *pi_lcn40 = pi->u.pi_lcn40phy;
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);

	switch (wlc_phy_chanspec_bandrange_get(pi, pi->radio_chanspec)) {
		case WL_CHAN_FREQ_RANGE_2G:
	#if TWO_POWER_RANGE_TXPWR_CTRL
		if (pi_lcn->lcnphy_twopwr_txpwrctrl_en)
			PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlIdleTssi1,
			idleTssi1, pi_lcn40->lcn40_idletssi1_cache.idletssi_2g);
	#endif  /* #if TWO_POWER_RANGE_TXPWR_CTRL */
			PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlIdleTssi,
			idleTssi0, pi_lcn40->lcn40_idletssi0_cache.idletssi_2g);
			break;
#ifdef BAND5G
		case WL_CHAN_FREQ_RANGE_5GL:
			/* 5 GHz low */
	#if TWO_POWER_RANGE_TXPWR_CTRL
			if (pi_lcn->lcnphy_twopwr_txpwrctrl_en)
			PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlIdleTssi1,
			idleTssi1, pi_lcn40->lcn40_idletssi1_cache.idletssi_5gl);
	#endif  /* #if TWO_POWER_RANGE_TXPWR_CTRL */
			PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlIdleTssi,
			idleTssi0, pi_lcn40->lcn40_idletssi0_cache.idletssi_5gl);
			break;

		case WL_CHAN_FREQ_RANGE_5GM:
			/* 5 GHz middle */
	#if TWO_POWER_RANGE_TXPWR_CTRL
			if (pi_lcn->lcnphy_twopwr_txpwrctrl_en)
			PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlIdleTssi1,
			idleTssi1, pi_lcn40->lcn40_idletssi1_cache.idletssi_5gm);
	#endif  /* #if TWO_POWER_RANGE_TXPWR_CTRL */
			PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlIdleTssi,
			idleTssi0, pi_lcn40->lcn40_idletssi0_cache.idletssi_5gm);
			break;

		case WL_CHAN_FREQ_RANGE_5GH:
			/* 5 GHz high */
	#if TWO_POWER_RANGE_TXPWR_CTRL
			if (pi_lcn->lcnphy_twopwr_txpwrctrl_en)
			PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlIdleTssi1,
			idleTssi1, pi_lcn40->lcn40_idletssi1_cache.idletssi_5gh);
	#endif  /* #if TWO_POWER_RANGE_TXPWR_CTRL */
			PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlIdleTssi,
			idleTssi0, pi_lcn40->lcn40_idletssi0_cache.idletssi_5gh);
			break;
#endif /* BAND5G */
		default:
			PHY_ERROR(("wl%d: %s: Bad channel/band\n",
				pi->sh->unit, __FUNCTION__));
			break;
	}
}

static void
wlc_lcn40phy_idle_tssi_est(phy_info_t *pi)
{
	bool suspend, tx_gain_override_old;
	phy_txgains_t old_gains;
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	uint8 SAVE_bbmult;
	uint16 idleTssi, idleTssi0_2C, idleTssi0_OB, idleTssi0_regvalue_OB, idleTssi0_regvalue_2C;
#if TWO_POWER_RANGE_TXPWR_CTRL
	uint16 idleTssi1, idleTssi1_2C, idleTssi1_OB, idleTssi1_regvalue_OB, idleTssi1_regvalue_2C;
#endif // endif
	uint16 SAVE_txpwrctrl;
	uint8 SAVE_indx;
	uint16 SAVE_lpfgainBIQ1;
	uint16 SAVE_lpfgainBIQ2;
	uint16 save_wlpriogainChangeEn;
	uint16 save_internalRadioSharingEn;
	uint16 save_BTPriority_ovr_val;
	uint16 save_BTPriority_ovr;

	idleTssi = phy_utils_read_phyreg(pi, LCN40PHY_TxPwrCtrlStatus);
	suspend = !(R_REG(pi->sh->osh, &((phy_info_t *)pi)->regs->maccontrol) & MCTL_EN_MAC);
	if (!suspend)
		wlapi_suspend_mac_and_wait(pi->sh->physhim);

	SAVE_txpwrctrl = wlc_lcn40phy_get_tx_pwr_ctrl(pi);
	SAVE_indx = wlc_lcn40phy_get_current_tx_pwr_idx(pi);

	SAVE_lpfgainBIQ1 = phy_utils_read_radioreg(pi, RADIO_2065_OVR5);
	SAVE_lpfgainBIQ2 = phy_utils_read_radioreg(pi, RADIO_2065_OVR6) & 0x1000;
	save_wlpriogainChangeEn = phy_utils_read_phyreg(pi, LCN40PHY_crsgainCtrl);
	save_internalRadioSharingEn = phy_utils_read_phyreg(pi, LCN40PHY_radioCtrl);
	save_BTPriority_ovr_val = phy_utils_read_phyreg(pi, LCN40PHY_RFinputOverrideVal);
	save_BTPriority_ovr = phy_utils_read_phyreg(pi, LCN40PHY_RFinputOverride);

	wlc_lcn40phy_set_tx_pwr_ctrl(pi, LCN40PHY_TX_PWR_CTRL_OFF);

	/* Save old tx gains if needed */
	tx_gain_override_old = wlc_lcn40phy_tx_gain_override_enabled(pi);
	wlc_lcn40phy_get_tx_gain(pi, &old_gains);
	/* set txgain override */
	wlc_lcn40phy_enable_tx_gain_override(pi);
	wlc_lcn40phy_set_tx_pwr_by_index(pi, 127);

	PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlRfCtrlOverride0, tssiRangeOverride, 1);
	PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlRfCtrlOverride0, tssiRangeOverrideVal, 1);

	wlc_lcn40phy_tssi_setup(pi);
	SAVE_bbmult = wlc_lcn40phy_get_bbmult(pi);
	/* Restore TSSI if
	* 1. cal is not possible
	* 2. idle TSSI for the current band/subband is valid
	* XXX Fix the idle tssi issue casuing low thruput
	*/
	if (wlc_phy_no_cal_possible(pi)) {
		phy_info_lcn40phy_t *pi_lcn40 = pi->u.pi_lcn40phy;
		int range = wlc_phy_chanspec_bandrange_get(pi, pi->radio_chanspec);

		if (((range == WL_CHAN_FREQ_RANGE_2G) &&
			(pi_lcn40->lcn40_idletssi0_cache.idletssi_2g)) ||
			((range == WL_CHAN_FREQ_RANGE_5GL) &&
			(pi_lcn40->lcn40_idletssi0_cache.idletssi_5gl)) ||
			((range == WL_CHAN_FREQ_RANGE_5GM) &&
			(pi_lcn40->lcn40_idletssi0_cache.idletssi_5gm)) ||
			((range == WL_CHAN_FREQ_RANGE_5GH) &&
			(pi_lcn40->lcn40_idletssi0_cache.idletssi_5gh))) {
			wlc_lcn40phy_restore_idletssi(pi);
			goto cleanIdleTSSI;
		}
	}

	wlc_lcn40phy_set_bbmult(pi, 0x0);

	wlc_lcn40phy_force_digigain_zerodB_WAR(pi);
	wlc_btcx_override_enable(pi);
	wlc_phy_do_dummy_tx(pi, TRUE, OFF);
	/* Disable WLAN priority */
	wlc_phy_btcx_override_disable(pi);

	idleTssi = PHY_REG_READ(pi, LCN40PHY, TxPwrCtrlStatus, estPwr);
	/* avgTssi value is in 2C (S9.0) format */
	idleTssi0_2C = PHY_REG_READ(pi, LCN40PHY, TxPwrCtrlStatusNew4, avgTssi);

	/* Convert idletssi1_2C from 2C to OB format by toggling MSB OB value */
	/* ranges from 0 to (2^9-1) = 511, 2C value ranges from -256 to (2^9-1-2^8) = 255 */
	/* Convert 9-bit idletssi1_2C to 9-bit idletssi1_OB. */
	if (idleTssi0_2C >= 256)
		idleTssi0_OB = idleTssi0_2C - 256;
	else
		idleTssi0_OB = idleTssi0_2C + 256;
	/* Convert 9-bit idletssi1_OB to 7-bit value for comparison with idletssi */
	if (idleTssi != (idleTssi0_OB >> 2))
	/* causing cstyle error */
	PHY_ERROR(("wl%d: %s, ERROR: idleTssi estPwr(OB): "
	           "0x%04x Register avgTssi(OB, 7MSB): 0x%04x\n",
	           pi->sh->unit, __FUNCTION__, idleTssi, idleTssi0_OB >> 2));

	idleTssi0_regvalue_OB = idleTssi0_OB;

	if (idleTssi0_regvalue_OB >= 256)
		idleTssi0_regvalue_2C = idleTssi0_regvalue_OB - 256;
	else
		idleTssi0_regvalue_2C = idleTssi0_regvalue_OB + 256;

#if TWO_POWER_RANGE_TXPWR_CTRL
	if (pi_lcn->lcnphy_twopwr_txpwrctrl_en) {
		PHY_REG_LIST_START
			PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlRfCtrlOverride0, tssiRangeOverride, 1)
			PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlRfCtrlOverride0, tssiRangeOverrideVal,
				0)
			PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlCmdNew, txPwrCtrlScheme, 2)
		PHY_REG_LIST_EXECUTE(pi);

		wlc_btcx_override_enable(pi);
		wlc_phy_do_dummy_tx(pi, TRUE, OFF);
		/* Disable WLAN priority */
		wlc_phy_btcx_override_disable(pi);
		/* idletssi1 is calculated from path 0 after attenuator setting */
		/* since HW uses path 0 always when txpower is set by index */
		idleTssi1 = PHY_REG_READ(pi, LCN40PHY, TxPwrCtrlStatus, estPwr);
		/* avgTssi value is in 2C (S9.0) format */
		idleTssi1_2C = PHY_REG_READ(pi, LCN40PHY, TxPwrCtrlStatusNew4, avgTssi);
		/* Convert idletssi1_2C from 2C to OB format by toggling MSB OB value */
		/* ranges from 0 to (2^9-1) = 511, 2C value ranges from -256 to (2^9-1-2^8) = 255 */
		/* Convert 9-bit idletssi1_2C to 9-bit idletssi1_OB. */
		if (idleTssi1_2C >= 256)
			idleTssi1_OB = idleTssi1_2C - 256;
		else
			idleTssi1_OB = idleTssi1_2C + 256;
		/* Convert 9-bit idletssi1_OB to 7-bit value for comparison with idletssi */
		if (idleTssi1 != (idleTssi1_OB >> 2))
		/* causing cstyle error */
		PHY_ERROR(("wl%d: %s, ERROR: idleTssi estPwr1(OB): "
		           "0x%04x Register avgTssi1(OB, 7MSB): 0x%04x\n",
		           pi->sh->unit, __FUNCTION__, idleTssi1, idleTssi1_OB >> 2));

		idleTssi1_regvalue_OB = idleTssi1_OB;

		if (idleTssi1_regvalue_OB >= 256)
			idleTssi1_regvalue_2C = idleTssi1_regvalue_OB - 256;
		else
			idleTssi1_regvalue_2C = idleTssi1_regvalue_OB + 256;

		PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlIdleTssi1, idleTssi1, idleTssi1_regvalue_2C);

		wlc_lcn40phy_save_idletssi(pi, idleTssi0_regvalue_2C, idleTssi1_regvalue_2C);

		PHY_REG_LIST_START
			/* Clear tssiRangeOverride */
			PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlRfCtrlOverride0, tssiRangeOverride, 0)
			PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlRfCtrl0, tssiRangeVal0, 1)
			PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlRfCtrl0, tssiRangeVal1, 0)
		PHY_REG_LIST_EXECUTE(pi);
	}
#endif  /* #if TWO_POWER_RANGE_TXPWR_CTRL */

	/* Write after idletssi1 is calculated since it depends on idleTssi0 set to 0xFF */
	PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlIdleTssi, idleTssi0, idleTssi0_regvalue_2C);
	/* Cache idle TSSI based on band/subband */
	/* XXX FIX ME: need to think of the right way to save
	* (don't do it twice) for now getting around the compiler error
	*/
	if (!pi_lcn->lcnphy_twopwr_txpwrctrl_en)
		wlc_lcn40phy_save_idletssi(pi, idleTssi0_regvalue_2C,
		idleTssi0_regvalue_2C);

cleanIdleTSSI:

	wlc_lcn40phy_set_txpwr_clamp(pi);

	/* Clear tx PU override */
	PHY_REG_MOD(pi, LCN40PHY, RFOverride0, internalrftxpu_ovr, 0);
	wlc_lcn40phy_set_bbmult(pi, SAVE_bbmult);
	/* restore txgain override */
	wlc_lcn40phy_set_tx_gain_override(pi, tx_gain_override_old);
	wlc_lcn40phy_set_tx_gain(pi, &old_gains);
	wlc_lcn40phy_set_tx_pwr_by_index(pi, SAVE_indx);
	wlc_lcn40phy_set_tx_pwr_ctrl(pi, SAVE_txpwrctrl);
	/* restore radio registers */
	phy_utils_mod_radioreg(pi, RADIO_2065_OVR5, 1, SAVE_lpfgainBIQ1);
	phy_utils_mod_radioreg(pi, RADIO_2065_OVR6, 0x1000, SAVE_lpfgainBIQ2);
	PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlRangeCmd, cckPwrOffset,
		pi_lcn->cckPwrOffset + (pi_lcn->cckPwrIdxCorr<<1));
	PHY_REG_WRITE(pi, LCN40PHY, crsgainCtrl, save_wlpriogainChangeEn);
	PHY_REG_WRITE(pi, LCN40PHY, radioCtrl, save_internalRadioSharingEn);
	PHY_REG_WRITE(pi, LCN40PHY, RFinputOverrideVal, save_BTPriority_ovr_val);
	PHY_REG_WRITE(pi, LCN40PHY, RFinputOverride, save_BTPriority_ovr);

	if (!suspend)
		wlapi_enable_mac(pi->sh->physhim);
}

/* Convert tssi to power LUT */
static void
wlc_lcn40phy_set_estPwrLUT(phy_info_t *pi, int32 lut_num)
{
	phytbl_info_t tab;
	int32 tssi;
	uint32 *pwr_table = NULL;
	int32 a1 = 0, b0 = 0, b1 = 0;

	tab.tbl_id = LCN40PHY_TBL_ID_TXPWRCTL;

	if ((pwr_table = (uint32*) LCN40PHY_MALLOC(pi, 128 * sizeof(uint32))) == NULL) {
		PHY_ERROR(("wl%d: %s: MALLOC failure\n", pi->sh->unit, __FUNCTION__));
		return;
	}

	if (lut_num == 0)
		tab.tbl_offset = 0; /* estPwrLuts */
	else
		tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_EST_PWR_OFFSET; /* estPwrLuts1 */
	tab.tbl_width = 32;	/* 32 bit wide	*/
	tab.tbl_ptr = pwr_table; /* ptr to buf */

	if (lut_num == 0) {
		/* Get the PA params for the particular channel we are in */
		wlc_phy_get_paparams_for_band(pi, &a1, &b0, &b1);

	} else {
		b0 = pi->txpa_2g_lo[0];
		b1 = pi->txpa_2g_lo[1];
		a1 = pi->txpa_2g_lo[2];
	}

	for (tssi = 0; tssi < 128; tssi++) {
		*(pwr_table + tssi) = wlc_lcnphy_tssi2dbm(tssi, a1, b0, b1);
	}
	tab.tbl_len = 128;        /* # values   */

	wlc_lcn40phy_write_table(pi,  &tab);

	if (pwr_table)
		LCN40PHY_MFREE(pi, pwr_table, 128 * sizeof(uint32));
}

static void
wlc_lcn40phy_restore_txiqlo_calibration_results(phy_info_t *pi, uint16 startidx,
	uint16 stopidx, uint8 index)
{
	phytbl_info_t tab;
	uint16 a, b;
	uint16 didq, didq1, didq2;
	int idx;
	uint8 ei0, eq0, fi0, fq0;
	uint32* val_array = NULL;
	uint8 idx1, idx2;
#ifdef BAND5G
	phy_info_lcn40phy_t *pi_lcn40 = pi->u.pi_lcn40phy;
#endif /* BAND5G */
#if defined(PHYCAL_CACHING)
	ch_calcache_t *ctx = wlc_phy_get_chanctx(pi, pi->radio_chanspec);
	lcnphy_calcache_t *cache;

	if (!ctx)
		return;

	cache = &ctx->u.lcnphy_cache;
	a = cache->txiqlocal_a[index];
	b = cache->txiqlocal_b[index];
	/* Use "txiqlocal_didq[0]" to store 2G OFDM didq */
	/*     "txiqlocal_didq[1]" to store 2G CCK didq */
	didq = cache->txiqlocal_didq[index];
	didq1 = cache->txiqlocal_didq[0];
	didq2 = cache->txiqlocal_didq[1];
	idx1 = cache->txiqlocal_index[0];
	idx2 = cache->txiqlocal_index[1];
#else
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	a = pi_lcn->lcnphy_cal_results.txiqlocal_a[index];
	b = pi_lcn->lcnphy_cal_results.txiqlocal_b[index];
	/* Use "txiqlocal_didq[0]" to store 2G OFDM didq */
	/*     "txiqlocal_didq[1]" to store 2G CCK didq */
	didq = pi_lcn->lcnphy_cal_results.txiqlocal_didq[index];
	didq1 = pi_lcn->lcnphy_cal_results.txiqlocal_didq[0];
	didq2 = pi_lcn->lcnphy_cal_results.txiqlocal_didq[1];
	idx1 = pi_lcn->lcnphy_cal_results.txiqlocal_index[0];
	idx2 = pi_lcn->lcnphy_cal_results.txiqlocal_index[1];
#endif /* defined(PHYCAL_CACHING) */

	BCM_REFERENCE(didq1);
	BCM_REFERENCE(didq2);
	BCM_REFERENCE(idx1);
	BCM_REFERENCE(idx2);

	wlc_lcn40phy_set_tx_iqcc(pi, a, b);
	/* Use "txiqlocal_didq[0]" to store 2G OFDM didq */
	/*     "txiqlocal_didq[1]" to store 2G CCK didq */
	if (LCN40REV_IS(pi->pubpi.phy_rev, 6) ||
		(LCN40REV_IS(pi->pubpi.phy_rev, 7) && CHSPEC_IS2G(pi->radio_chanspec) &&
		(RADIOVER(pi->pubpi.radiover) == 0x4))) {
			PHY_REG_MOD(pi, LCN40PHY, dlo_coeff_di_ofdm,
				dlo_coeff_di_ofdm, ((didq1 & 0xff00) >> 8));
			PHY_REG_MOD(pi, LCN40PHY, dlo_coeff_dq_ofdm,
				dlo_coeff_dq_ofdm, (didq1 & 0xff));
			PHY_REG_MOD(pi, LCN40PHY, dlo_coeff_di_ht,
				dlo_coeff_di_ht, ((didq1 & 0xff00) >> 8));
			PHY_REG_MOD(pi, LCN40PHY, dlo_coeff_dq_ht,
				dlo_coeff_dq_ht, (didq1 & 0xff));
			PHY_REG_MOD(pi, LCN40PHY, dlo_coeff_di_cck,
				dlo_coeff_di_cck, ((didq2 & 0xff00) >> 8));
			PHY_REG_MOD(pi, LCN40PHY, dlo_coeff_dq_cck,
				dlo_coeff_dq_cck, (didq2 & 0xff));
	}

	wlc_lcn40phy_set_tx_locc(pi, didq);

	if ((val_array = (uint32*)
		LCN40PHY_MALLOC(pi, (stopidx - startidx + 1) * sizeof(uint32))) == NULL) {
		PHY_ERROR(("wl%d: %s: MALLOC failure\n", pi->sh->unit, __FUNCTION__));
		return;
	}

	/* restore iqlo portion of tx power control tables */
	/* remaining element */
	tab.tbl_id = LCN40PHY_TBL_ID_TXPWRCTL;
	tab.tbl_width = 32; /* 32 bit wide	*/
	tab.tbl_len = stopidx - startidx + 1;		/* # values   */
	tab.tbl_ptr = val_array; /* ptr to buf */

	/* iq */
	tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_IQ_OFFSET + startidx;
	wlc_lcnphy_read_table(pi,  &tab);

	for (idx = 0; idx <= (stopidx - startidx); idx++) {
		*(val_array + idx) = (*(val_array + idx) & 0x0ff00000) |
			((uint32)(a & 0x3FF) << 10) | (b & 0x3ff);
	}

	wlc_lcn40phy_write_table(pi,	&tab);
	/* loft */
#ifdef BAND5G
	if ((LCN40REV_GE(pi->pubpi.phy_rev, 4)) &&
		(CHSPEC_IS5G(pi->radio_chanspec))) {
		if ((pi_lcn40->dlocalidx5g != -1) && pi_lcn40->loflag) {
			wlc_lcn40phy_dlo_coeff_interpolation(pi, didq1, idx1, didq2, idx2);
		}
	} else
#endif /* BAND5G */
	{
		tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_LO_OFFSET + startidx;
		for (idx = 0; idx <= (stopidx - startidx); idx++)
			*(val_array + idx) = didq;
		wlc_lcn40phy_write_table(pi,	&tab);
	}
	/* Do not move the below statements up */
	/* We need at least 2us delay to read phytable after writing radio registers */
	/* Apply analog LO */
#if defined(PHYCAL_CACHING)
	ei0 = (uint8)(cache->txiqlocal_ei0);
	eq0 = (uint8)(cache->txiqlocal_eq0);
	fi0 = (uint8)(cache->txiqlocal_fi0);
	fq0 = (uint8)(cache->txiqlocal_fq0);
#else
	ei0 = (uint8)(pi_lcn->lcnphy_cal_results.txiqlocal_ei0);
	eq0 = (uint8)(pi_lcn->lcnphy_cal_results.txiqlocal_eq0);
	fi0 = (uint8)(pi_lcn->lcnphy_cal_results.txiqlocal_fi0);
	fq0 = (uint8)(pi_lcn->lcnphy_cal_results.txiqlocal_fq0);
#endif // endif
	wlc_lcn40phy_set_radio_loft(pi, ei0, eq0, fi0, fq0);

	if (val_array)
		LCN40PHY_MFREE(pi, val_array, (stopidx - startidx + 1) * sizeof(uint32));
}

static void
wlc_lcn40phy_restore_papd_calibration_results(phy_info_t *pi)
{
	phytbl_info_t tab;
#if defined(PHYCAL_CACHING)
	ch_calcache_t *ctx = wlc_phy_get_chanctx(pi, pi->radio_chanspec);
	lcnphy_calcache_t *cache;
	uint16 SAVE_txpwrctrl = wlc_lcn40phy_get_tx_pwr_ctrl(pi);

	if (!ctx)
		return;

	cache = &ctx->u.lcnphy_cache;
#else
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
#endif // endif

	/* write eps table */
	tab.tbl_id = LCN40PHY_TBL_ID_PAPDCOMPDELTATBL;
	tab.tbl_width = 32;
	tab.tbl_len = PHY_PAPD_EPS_TBL_SIZE_LCNPHY;
	tab.tbl_offset = 0;
#if defined(PHYCAL_CACHING)
	tab.tbl_ptr = cache->papd_eps_tbl;
	wlc_lcnphy_write_table(pi, &tab);

	phy_utils_write_phyreg(pi, LCN40PHY_papd_tx_analog_gain_ref,
		cache->analog_gain_ref);
	phy_utils_write_phyreg(pi, LCN40PHY_papd_track_pa_lut_begin,
		cache->lut_begin);
	phy_utils_write_phyreg(pi, LCN40PHY_papd_track_pa_lut_step,
		cache->lut_step);
	phy_utils_write_phyreg(pi, LCN40PHY_papd_track_pa_lut_end,
		cache->lut_end);
	phy_utils_write_phyreg(pi, LCN40PHY_papd_rx_gain_comp_dbm,
		cache->rxcompdbm);
	phy_utils_write_phyreg(pi, LCN40PHY_papd_control,
		cache->papdctrl);
	phy_utils_write_phyreg(pi, LCN40PHY_sslpnCalibClkEnCtrl,
		cache->sslpnCalibClkEnCtrl);

	/* Restore the last gain index for this channel */
	wlc_lcn40phy_set_tx_pwr_by_index(pi, cache->lcnphy_gain_index_at_last_cal);

	/* Restore tx power control */
	wlc_lcn40phy_set_tx_pwr_ctrl(pi, SAVE_txpwrctrl);
#else
	tab.tbl_ptr = pi_lcn->lcnphy_cal_results.papd_eps_tbl;
	wlc_lcn40phy_write_table(pi, &tab);

	phy_utils_write_phyreg(pi, LCN40PHY_papd_tx_analog_gain_ref,
		pi_lcn->lcnphy_cal_results.analog_gain_ref);
	phy_utils_write_phyreg(pi, LCN40PHY_papd_track_pa_lut_begin,
		pi_lcn->lcnphy_cal_results.lut_begin);
	phy_utils_write_phyreg(pi, LCN40PHY_papd_track_pa_lut_step,
		pi_lcn->lcnphy_cal_results.lut_step);
	phy_utils_write_phyreg(pi, LCN40PHY_papd_track_pa_lut_end,
		pi_lcn->lcnphy_cal_results.lut_end);
	phy_utils_write_phyreg(pi, LCN40PHY_papd_rx_gain_comp_dbm,
		pi_lcn->lcnphy_cal_results.rxcompdbm);
	phy_utils_write_phyreg(pi, LCN40PHY_papd_control,
		pi_lcn->lcnphy_cal_results.papdctrl);
	phy_utils_write_phyreg(pi, LCN40PHY_sslpnCalibClkEnCtrl,
		pi_lcn->lcnphy_cal_results.sslpnCalibClkEnCtrl);
#endif /* PHYCAL_CACHING */
}
static void
wlc_lcn40phy_set_rx_iq_comp(phy_info_t *pi, uint16 a, uint16 b)
{
	PHY_REG_MOD(pi, LCN40PHY, RxCompcoeffa0, a0, a);
	PHY_REG_MOD(pi, LCN40PHY, RxCompcoeffb0, b0, b);

	PHY_REG_MOD(pi, LCN40PHY, RxCompcoeffa1, a1, a);
	PHY_REG_MOD(pi, LCN40PHY, RxCompcoeffb1, b1, b);

	PHY_REG_MOD(pi, LCN40PHY, RxCompcoeffa2, a2, a);
	PHY_REG_MOD(pi, LCN40PHY, RxCompcoeffb2, b2, b);
}

static void
wlc_lcn40phy_get_tx_gain(phy_info_t *pi, phy_txgains_t *gains)
{
	uint16 dac_gain;

	dac_gain = phy_utils_read_phyreg(pi, LCN40PHY_AfeDACCtrl) >>
		LCN40PHY_AfeDACCtrl_dac_ctrl_SHIFT;
	gains->dac_gain = (dac_gain & 0x380) >> 7;

	{
		uint16 rfgain0, rfgain1;

		rfgain0 = (phy_utils_read_phyreg(pi, LCN40PHY_txgainctrlovrval0) &
			LCN40PHY_txgainctrlovrval0_txgainctrl_ovr_val0_MASK) >>
			LCN40PHY_txgainctrlovrval0_txgainctrl_ovr_val0_SHIFT;
		rfgain1 = (phy_utils_read_phyreg(pi, LCN40PHY_txgainctrlovrval1) &
			LCN40PHY_txgainctrlovrval1_txgainctrl_ovr_val1_MASK) >>
			LCN40PHY_txgainctrlovrval1_txgainctrl_ovr_val1_SHIFT;

		gains->gm_gain = rfgain0 & 0xff;
		gains->pga_gain = (rfgain0 >> 8) & 0xff;
		gains->pad_gain = rfgain1 & 0xff;
	}
}
static uint8
wlc_lcn40phy_get_bbmult(phy_info_t *pi)
{
	uint16 m0m1;
	phytbl_info_t tab;

	tab.tbl_ptr = &m0m1; /* ptr to buf */
	tab.tbl_len = 1;        /* # values   */
	tab.tbl_id = LCN40PHY_TBL_ID_IQLOCAL;         /* iqloCaltbl      */
	tab.tbl_offset = 87; /* tbl offset */
	tab.tbl_width = 16;     /* 16 bit wide */
	wlc_lcn40phy_read_table(pi, &tab);

	return (uint8)((m0m1 & 0xff00) >> 8);
}

/* these rf registers need to be restored after iqlo_soft_cal_full */
static const
uint16 iqlo_loopback_rf_regs[] = {
	RADIO_2065_OVR12,
	RADIO_2065_TX2G_TSSI,
	RADIO_2065_IQCAL_CFG1,
	RADIO_2065_TESTBUF_CFG1,
	RADIO_2065_OVR1,
	RADIO_2065_AUXPGA_CFG1,
	RADIO_2065_LPF_CFG1,
	RADIO_2065_OVR7,
	RADIO_2065_PA2G_CFG1,
	RADIO_2065_OVR8,
	RADIO_2065_OVR13,
	RADIO_2065_AUXPGA_VMID,
	RADIO_2065_LPF_GAIN,
	RADIO_2065_IQCAL_IDAC,
	RADIO_2065_TX5G_TSSI,
	RADIO_2065_OVR16,
	RADIO_2065_DAC_CFG1,
	RADIO_2065_OVR14_ACCOREREG,
	RADIO_2065_OVR3,
	RADIO_2065_TRSW2G_CFG2,
	RADIO_2065_LNA2G_CFG1,
	RADIO_2065_PA5G_CFG1,
	RADIO_2065_OVR4,
	RADIO_2065_LNA5G_CFG1,
	RADIO_2065_OVR13_ACCOREREG,
	};

static void
wlc_lcn40phy_tx_iqlo_cal(
	phy_info_t *pi,
	phy_txgains_t *target_gains,
	phy_cal_mode_t cal_mode,
	bool keep_tone,
	bool epa_or_pad_lpbk)
{

	/* starting values used in full cal
	 * -- can fill non-zero vals based on lab campaign (e.g., per channel)
	 * -- format: a0,b0,a1,b1,ci0_cq0_ci1_cq1,di0_dq0,di1_dq1,ei0_eq0,ei1_eq1,fi0_fq0,fi1_fq1
	 */
	phy_txgains_t cal_gains, temp_gains;
	uint16 hash;
	uint8 band_idx;
	int j;
	uint16 ncorr_override[5];
	uint16 syst_coeffs[] =
		{0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000};

	/* cal commands full cal and recal */
	uint16 commands_fullcal[] =  { 0x8434, 0x8334, 0x8084, 0x8267, 0x8056, 0x8234 };
	/* added for 4334-B2 ;Aband ; e,f,iq,d cals */
	uint16 commands_fullcal2[] =  { 0x8434, 0x8334, 0x808a, 0x8267 };

	/* do the recal with full cal cmds for now, re-visit if time becomes
	 * an issue.
	 */
	/* uint16 commands_recal[] =  { 0x8312, 0x8055, 0x8212 }; */
	uint16 commands_recal[] =  { 0x8423, 0x8323, 0x8073, 0x8256, 0x8045, 0x8223  };
	uint16 commands_iq_recal[] =  { 0x8067 };
	/* iq imbal cal with starting iq coeffs set to zeroes */
	uint16 commands_iq_cal2[] = { 0x8089 };
	uint16 commands_iq_cal3[] = {0x8071, 0x8071, 0x8071, 0x8071, 0x8071, 0x0000, 0x8078 };
	uint16 commands_dig_lo[] = {0x8267};

	/* for populating tx power control table
	(like full cal but skip radio regs, since they don't exist in tx pwr ctrl table)
	*/
	uint16 commands_txpwrctrl[] = { 0x8084, 0x8267, 0x8056, 0x8234 };

	/* calCmdNum register: log2 of settle/measure times for search/gain-ctrl, 4 bits each */
	uint16 command_nums_fullcal[] = { 0x7a97, 0x7a97, 0x7a97, 0x7a87, 0x7a87, 0x7b97 };
	/* added for 4334-B2 ;Aband; fullcal2 */
	uint16 command_nums_fullcal2[] = { 0x7a97, 0x7a97, 0x7a97, 0x7a87 };
	uint16 command_nums_txpwrctrl[] = { 0x7a97, 0x7a87, 0x7a87, 0x7b97 };

	/* do the recal with full cal cmds for now, re-visit if time becomes
	 * an issue.
	 */
	/* uint16 command_nums_recal[] = {  0x7997, 0x7987, 0x7a97 }; */
	uint16 command_nums_recal[] = { 0x7a97, 0x7a97, 0x7a97, 0x7a87, 0x7a87, 0x7b97 };
	uint16 *command_nums = command_nums_fullcal;
	uint16 command_nums_iq_recal[] = { 0x7b97 };
	uint16 command_nums_iq_cal2[] = {0x7b97};
	uint16 command_nums_iq_cal3[] = {0x7b97, 0x7b97, 0x7b97, 0x7b97, 0x7b97, 0x0000, 0x7b97};
	uint16 command_nums_dig_lo[] = {0x7b97};

	uint16 *start_coeffs = NULL, *cal_cmds = NULL, cal_type;
	uint16 tx_pwr_ctrl_old, save_txpwrctrlrfctrl2, save_txpwrctrlcmd;
	uint16 save_sslpnCalibClkEnCtrl, save_sslpnRxFeClkEnCtrl, save_ClkEnCtrl;
	uint16 save_wlpriogainChangeEn;
	uint16 save_internalRadioSharingEn, save_BTPriority_ovr_val, save_BTPriority_ovr;
	uint16 save_rxFarrowCtrl;

	bool tx_gain_override_old;
	phy_txgains_t old_gains = {0, 0, 0, 0};
	uint i, n_cal_cmds = 0, n_cal_start = 0;
	uint16 values_to_save[sizeof(uint16) * (ARRAYSIZE(iqlo_loopback_rf_regs) + 2)];
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	int16 tone_freq;
	uint8 ei0 = 0, eq0 = 0, fi0 = 0, fq0 = 0;
	phy_info_lcn40phy_t *pi_lcn40 = pi->u.pi_lcn40phy;
#if defined(PHYCAL_CACHING)
	ch_calcache_t *ctx = wlc_phy_get_chanctx(pi, pi->radio_chanspec);
	lcnphy_calcache_t *cache = NULL;

	if (ctx)
		cache = &ctx->u.lcnphy_cache;
#endif // endif

	if (NORADIO_ENAB(pi->pubpi))
		return;

	if (CHSPEC_IS2G(pi->radio_chanspec))
		syst_coeffs[5] = (uint16) pi_lcn40->startdiq_2g;
#ifdef BAND5G
	else
		syst_coeffs[5] = (uint16) pi_lcn40->startdiq_5g;
#endif /* BAND5G */

	save_sslpnRxFeClkEnCtrl = phy_utils_read_phyreg(pi, LCN40PHY_sslpnRxFeClkEnCtrl);
	save_sslpnCalibClkEnCtrl = phy_utils_read_phyreg(pi, LCN40PHY_sslpnCalibClkEnCtrl);
	save_ClkEnCtrl = phy_utils_read_phyreg(pi, LCN40PHY_ClkEnCtrl);
	save_wlpriogainChangeEn = phy_utils_read_phyreg(pi, LCN40PHY_crsgainCtrl);
	save_internalRadioSharingEn = phy_utils_read_phyreg(pi, LCN40PHY_radioCtrl);
	save_BTPriority_ovr_val = phy_utils_read_phyreg(pi, LCN40PHY_RFinputOverrideVal);
	save_BTPriority_ovr = phy_utils_read_phyreg(pi, LCN40PHY_RFinputOverride);

	save_rxFarrowCtrl = phy_utils_read_phyreg(pi, LCN40PHY_rxFarrowCtrl);

	/* turn on clk to iqlo block */
	phy_utils_or_phyreg(pi, LCN40PHY_sslpnCalibClkEnCtrl, 0x40);

	/* Get desired start coeffs and select calibration command sequence */

	switch (cal_mode) {
		case CAL_FULL:
			start_coeffs = syst_coeffs;
			cal_cmds = commands_fullcal;
			n_cal_cmds = ARRAYSIZE(commands_fullcal);
			wlc_lcn40phy_set_radio_loft(pi, 0, 0, 0, 0);
			break;

		case CAL_FULL2:
			start_coeffs = syst_coeffs;
			cal_cmds = commands_fullcal2;
			n_cal_cmds = ARRAYSIZE(commands_fullcal2);
			command_nums = command_nums_fullcal2;
			break;

		case CAL_RECAL:
			/* since re-cal is same as full cal */
			start_coeffs = syst_coeffs;

			cal_cmds = commands_recal;
			n_cal_cmds = ARRAYSIZE(commands_recal);
			command_nums = command_nums_recal;
			break;

		case CAL_DIGLO:
#if defined(PHYCAL_CACHING)
			if (ctx) {
				ASSERT(cache->txiqlocal_bestcoeffs_valid);
				start_coeffs = cache->txiqlocal_bestcoeffs;
			} else
				start_coeffs = syst_coeffs;
#else
			ASSERT(pi_lcn->lcnphy_cal_results.txiqlocal_bestcoeffs_valid);
			start_coeffs = pi_lcn->lcnphy_cal_results.txiqlocal_bestcoeffs;
#endif // endif
			cal_cmds = commands_dig_lo;
			n_cal_cmds = ARRAYSIZE(commands_dig_lo);
			command_nums = command_nums_dig_lo;
			break;

		case CAL_IQ_RECAL:

#if defined(PHYCAL_CACHING)
			if (ctx) {
				ASSERT(cache->txiqlocal_bestcoeffs_valid);
				start_coeffs = cache->txiqlocal_bestcoeffs;
			} else
				start_coeffs = syst_coeffs;
#else
			ASSERT(pi_lcn->lcnphy_cal_results.txiqlocal_bestcoeffs_valid);
			start_coeffs = pi_lcn->lcnphy_cal_results.txiqlocal_bestcoeffs;
#endif // endif

			cal_cmds = commands_iq_recal;
			n_cal_cmds = ARRAYSIZE(commands_iq_recal);
			command_nums = command_nums_iq_recal;
			break;

		case CAL_IQ_CAL2:

#if defined(PHYCAL_CACHING)
			if (ctx) {
				ASSERT(cache->txiqlocal_bestcoeffs_valid);
				start_coeffs = cache->txiqlocal_bestcoeffs;
			} else
				start_coeffs = syst_coeffs;
#else
			ASSERT(pi_lcn->lcnphy_cal_results.txiqlocal_bestcoeffs_valid);
			start_coeffs = pi_lcn->lcnphy_cal_results.txiqlocal_bestcoeffs;
#endif // endif
			/* setting iq start coeffs to zero for iqcal2 */
			start_coeffs[0] = start_coeffs[1] = 0x0000;
			cal_cmds = commands_iq_cal2;
			n_cal_cmds = ARRAYSIZE(commands_iq_cal2);
			command_nums = command_nums_iq_cal2;
			break;

		case CAL_IQ_CAL3:

#if defined(PHYCAL_CACHING)
			if (ctx) {
				ASSERT(cache->txiqlocal_bestcoeffs_valid);
				start_coeffs = cache->txiqlocal_bestcoeffs;
			} else
				start_coeffs = syst_coeffs;
#else
			ASSERT(pi_lcn->lcnphy_cal_results.txiqlocal_bestcoeffs_valid);
			start_coeffs = pi_lcn->lcnphy_cal_results.txiqlocal_bestcoeffs;
#endif // endif
			/* setting iq start coeffs to zero for iqcal3 */
			start_coeffs[0] = start_coeffs[1] = 0x0000;
			cal_cmds = commands_iq_cal3;
			n_cal_cmds = ARRAYSIZE(commands_iq_cal3);
			command_nums = command_nums_iq_cal3;
			break;

		case CAL_TXPWRCTRL:

			wlc_lcn40phy_get_radio_loft(pi, &ei0, &eq0, &fi0, &fq0);
			start_coeffs = syst_coeffs;
			start_coeffs[7] = (((ei0 & 0xff) << 8) | (eq0 & 0xff));
			start_coeffs[8] = 0;
			start_coeffs[9] = (((fi0 & 0xff) << 8) | (fq0 & 0xff));
			start_coeffs[10] = 0;
			cal_cmds = commands_txpwrctrl;
			n_cal_cmds = ARRAYSIZE(commands_txpwrctrl);
			command_nums = command_nums_txpwrctrl;
			break;

		default:
			ASSERT(FALSE);
	}

	/* Fill in Start Coeffs */
	wlc_lcn40phy_common_write_table(pi, LCN40PHY_TBL_ID_IQLOCAL,
		start_coeffs, 11, 16, 64);

	phy_utils_write_phyreg(pi, LCN40PHY_sslpnCalibClkEnCtrl, 0xffff);
	PHY_REG_MOD(pi, LCN40PHY, auxadcCtrl, iqlocalEn, 1);

	/* Save original tx power control mode */
	tx_pwr_ctrl_old = wlc_lcn40phy_get_tx_pwr_ctrl(pi);
	save_txpwrctrlcmd = phy_utils_read_phyreg(pi, LCN40PHY_TxPwrCtrlCmd);
	PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlCmd, txPwrCtrl_swap_iq, 1);

	/* Disable tx power control */
	wlc_lcn40phy_set_tx_pwr_ctrl(pi, LCN40PHY_TX_PWR_CTRL_OFF);

	save_txpwrctrlrfctrl2 = phy_utils_read_phyreg(pi, LCN40PHY_TxPwrCtrlRfCtrl2);

	PHY_REG_LIST_START
		PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlRfCtrl2, afeAuxpgaSelVmidVal0, 0x2a6)
		PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlRfCtrl2, afeAuxpgaSelGainVal0, 2)

#define GENV_ADDR	0xd7
#define BBI_ADDR	0x5c
#define BBQ_ADDR	0x5d
#define RFI_ADDR	0x5e
#define RFQ_ADDR	0x5f
		PHY_REG_MOD_ENTRY(LCN40PHY, iqlocalRadioRegAddr0, txrf_iqcal_gain_waddr, GENV_ADDR)
		PHY_REG_MOD_ENTRY(LCN40PHY, iqlocalRadioRegAdd1, tx_vos_mxi_waddr, BBI_ADDR)
		PHY_REG_MOD_ENTRY(LCN40PHY, iqlocalRadioRegAddr2, tx_vos_mxq_waddr, BBQ_ADDR)
		PHY_REG_MOD_ENTRY(LCN40PHY, iqlocalRadioRegAddr2, tx_idac_lo_rfi_waddr9to4,
			(RFI_ADDR >> 4))
		PHY_REG_MOD_ENTRY(LCN40PHY, iqlocalRadioRegAdd3, tx_idac_lo_rfi_waddr3to0,
			(RFI_ADDR & 0xf))
		PHY_REG_MOD_ENTRY(LCN40PHY, iqlocalRadioRegAdd3, tx_idac_lo_rfq_waddr, RFQ_ADDR)

		/* increase the farrow gain by 6dB */
		PHY_REG_WRITE_ENTRY(LCN40PHY, rxFarrowCtrl, 0x7)
	PHY_REG_LIST_EXECUTE(pi);

	/* setup tx iq loopback path */
	wlc_lcn40phy_tx_iqlo_loopback(pi, values_to_save, epa_or_pad_lpbk);
	if ((cal_mode == CAL_DIGLO) &&
		(LCN40REV_IS(pi->pubpi.phy_rev, 6) ||
		(LCN40REV_IS(pi->pubpi.phy_rev, 7) && (RADIOVER(pi->pubpi.radiover) == 0x4) &&
		CHSPEC_IS2G(pi->radio_chanspec)))) {
		phy_utils_mod_radioreg(pi, RADIO_2065_OVR7, 0x10, 0x10);
		phy_utils_mod_radioreg(pi, RADIO_2065_LPF_CFG1, 0x200, 0x0);
	}

	if (PHY_EPA_SUPPORT(pi_lcn->ePA) && !epa_or_pad_lpbk) {
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			if (pi_lcn->txiqlopapu_2g == -1)
				/* by default do nothing, note: different from lcnphy */
				wlc_lcn40phy_epa_pd(pi, 0);
			else
				wlc_lcn40phy_epa_pd(pi, !pi_lcn->txiqlopapu_2g);
		}
#ifdef BAND5G
		else {
			if (pi_lcn->txiqlopapu_5g == -1)
				/* by default do nothing, note: different from lcnphy */
				wlc_lcn40phy_epa_pd(pi, 0);
			else {
				wlc_lcn40phy_epa_pd(pi, !pi_lcn->txiqlopapu_5g);

				if (((BOARDTYPE(pi->sh->boardtype) == BCM94334FCAGBI_SSID) ||
					(BOARDTYPE(pi->sh->boardtype) == BCM94334WLAGBI_SSID) ||
					(BOARDTYPE(pi->sh->boardtype) == BCM943342FCAGBI_SSID)) &&
					!pi_lcn->txiqlopapu_5g)
					/* extra delay such that IQLO coeffs match
					* TCL at low temperature
					*/
					OSL_DELAY(20000);
			}
		}
#endif /* #ifdef BAND5G */
	}

	/* Save old and apply new tx gains if needed */
	tx_gain_override_old = wlc_lcn40phy_tx_gain_override_enabled(pi);
	if (tx_gain_override_old)
		wlc_lcn40phy_get_tx_gain(pi, &old_gains);

	if (!target_gains) {
		if (!tx_gain_override_old)
			wlc_lcn40phy_set_tx_pwr_by_index(pi, pi_lcn->lcnphy_tssi_idx);
		wlc_lcn40phy_get_tx_gain(pi, &temp_gains);
		target_gains = &temp_gains;
	}

	hash = (target_gains->gm_gain << 8) |
		(target_gains->pga_gain << 4) |
		(target_gains->pad_gain);

	band_idx = (CHSPEC_IS5G(pi->radio_chanspec) ? 1 : 0);

	cal_gains = *target_gains;
	bzero(ncorr_override, sizeof(ncorr_override));
	for (j = 0; j < iqcal_gainparams_numgains_lcn40phy[band_idx]; j++) {
		if (hash == tbl_iqcal_gainparams_lcn40phy[band_idx][j][0]) {
			cal_gains.gm_gain = tbl_iqcal_gainparams_lcn40phy[band_idx][j][1];
			cal_gains.pga_gain = tbl_iqcal_gainparams_lcn40phy[band_idx][j][2];
			cal_gains.pad_gain = tbl_iqcal_gainparams_lcn40phy[band_idx][j][3];
			bcopy(&tbl_iqcal_gainparams_lcn40phy[band_idx][j][3], ncorr_override,
				sizeof(ncorr_override));
			break;
		}
	}
	/* apply cal gains */
	wlc_lcn40phy_set_tx_gain(pi, &cal_gains);

	PHY_INFORM(("wl%d: %s: target gains: %d %d %d %d, cal_gains: %d %d %d %d\n",
		pi->sh->unit, __FUNCTION__,
		target_gains->gm_gain,
		target_gains->pga_gain,
		target_gains->pad_gain,
		target_gains->dac_gain,
		cal_gains.gm_gain,
		cal_gains.pga_gain,
		cal_gains.pad_gain,
		cal_gains.dac_gain));

	/* set gain control parameters */
	phy_utils_write_phyreg(pi, LCN40PHY_iqloCalCmdGctl, 0xaa9);
	phy_utils_write_phyreg(pi, LCN40PHY_iqloCalGainThreshD2, 0xc0);

	if (cal_mode == CAL_IQ_CAL3)
		phy_utils_write_phyreg(pi, LCN40PHY_iqloCalGainThreshD2, 0x88);

	/* Load the LO compensation gain table */
	wlc_lcn40phy_common_write_table(pi, LCN40PHY_TBL_ID_IQLOCAL,
		(CONST void *)lcn40phy_iqcal_loft_gainladder,
		ARRAYSIZE(lcn40phy_iqcal_loft_gainladder), 16, 0);
	/* Load the IQ calibration gain table */
	wlc_lcn40phy_common_write_table(pi, LCN40PHY_TBL_ID_IQLOCAL,
		(CONST void *)lcn40phy_iqcal_ir_gainladder, ARRAYSIZE(lcn40phy_iqcal_ir_gainladder),
		16, 32);

	if (pi_lcn->lcnphy_tx_iqlo_tone_freq_ovr_val == 0) {
		if (CHIPID(pi->sh->chip) == BCM4314_CHIP_ID ||
			CHIPID(pi->sh->chip) == BCM43142_CHIP_ID ||
			CHIPID(pi->sh->chip) == BCM43143_CHIP_ID)

			tone_freq = 2500;
		else
			tone_freq = 1250;
	} else
		tone_freq = pi_lcn->lcnphy_tx_iqlo_tone_freq_ovr_val;

	/* Forcing the digi gain to zero */
	wlc_lcn40phy_force_digigain_zerodB_WAR(pi);
/*
	if (cal_mode == CAL_IQ_RECAL)
		tone_freq = -tone_freq;
*/
	/* Send out calibration tone */
	if (pi->phy_tx_tone_freq) {
		/* if tone is already played out with iq cal mode zero then
		 * stop the tone and re-play with iq cal mode 1.
		 */
		wlc_lcn40phy_stop_tx_tone(pi);
		OSL_DELAY(5);
	}

	if (CHSPEC_IS40(pi->radio_chanspec) && LCN40REV_LE(pi->pubpi.phy_rev, 3)) {
		wlc_lcn40phy_start_tx_tone(pi, ((tone_freq + 10000) * 1000), 88, 1);
		wlc_lcn40phy_play_sample_table1(pi, tone_freq * 1000, 88);

	} else
		wlc_lcn40phy_start_tx_tone(pi, (tone_freq * 1000), 88, 1);

	/* FIX ME: re-enable all the phy clks. */
	phy_utils_write_phyreg(pi, LCN40PHY_sslpnCalibClkEnCtrl, 0xffff);
	/*
	 * Cal Steps
	 */
	if (cal_mode != CAL_IQ_CAL3) {
		for (i = n_cal_start; i < n_cal_cmds; i++) {
			uint16 best_coeffs[11];
			uint16 command_num;

			cal_type = (cal_cmds[i] & 0x0f00) >> 8;

			/* get & set intervals */
			command_num = command_nums[i];
			if (ncorr_override[cal_type])
				command_num = ncorr_override[cal_type] << 8 | (command_num & 0xff);
			/* enable IQLO select MUX */
			phy_utils_or_phyreg(pi, LCN40PHY_sslpnCtrl3, 4);
			phy_utils_write_phyreg(pi, LCN40PHY_iqloCalCmdNnum, command_num);

			PHY_TMP(("wl%d: %s: running cmd: %x, cmd_num: %x\n",
				pi->sh->unit, __FUNCTION__, cal_cmds[i], command_nums[i]));

			if (cal_type == 0 &&
				(LCN40REV_LE(pi->pubpi.phy_rev, 1)))
				continue;

			/* Issue cal command */
			phy_utils_write_phyreg(pi, LCN40PHY_iqloCalCmd, cal_cmds[i]);

			/* Wait until cal command finished */
			if (!wlc_lcn40phy_iqcal_wait(pi)) {
				PHY_ERROR(("wl%d: %s: tx iqlo cal failed to complete\n",
					pi->sh->unit, __FUNCTION__));
				/* No point to continue */
				goto cleanup;
			}

			/* Copy best coefficients to start coefficients */
			wlc_lcn40phy_common_read_table(pi, LCN40PHY_TBL_ID_IQLOCAL,
				best_coeffs, ARRAYSIZE(best_coeffs), 16, 96);
			wlc_lcn40phy_common_write_table(pi, LCN40PHY_TBL_ID_IQLOCAL, best_coeffs,
				ARRAYSIZE(best_coeffs), 16, 64);

#if defined(PHYCAL_CACHING)
			if (ctx)
				wlc_lcn40phy_common_read_table(pi, LCN40PHY_TBL_ID_IQLOCAL,
					cache->txiqlocal_bestcoeffs,
					ARRAYSIZE(cache->txiqlocal_bestcoeffs), 16, 96);
#else
			wlc_lcn40phy_common_read_table(pi, LCN40PHY_TBL_ID_IQLOCAL,
				pi_lcn->lcnphy_cal_results.txiqlocal_bestcoeffs,
				ARRAYSIZE(pi_lcn->lcnphy_cal_results.txiqlocal_bestcoeffs), 16, 96);
#endif /* PHYCAL_CACHING */
		}
	} else {
		uint16 best_coeffs[11];
		uint16 command_num;
		int16 a[] = {128, -128, -128, 128, 0};
		int16 b[] = {128, 128, -128, -128, 0};
		uint16 Bi[] = {0, 0, 0, 0, 0};
		uint16 Bq[] = {0, 0, 0, 0, 0};
		int16 i_min = 0;
		int16 q_min = 0;
		uint8 agidx = 0;

		/*
		7 Cal Steps for iqcal3
		first 5 steps: Auto IQ cal that covers 5 quadrants
		centred at { (128,128) (-128,128) (-128,-128) (128,-128) (0,0)}
		with step size 128 and number of levels 1 and determies minima
		of each quadrant.
		6th step : Manual IQ cal with single point dft to further determine
		the minima among the
		first four quadarants minimas. Quadrant 5 (0,0) used only to find out ladder index.
		7th step: continue auto cal with step size 128 and number of levels 8.
		The minima (a,b) determined in step 6 are	used as start coefficients
		*/

		for (i = n_cal_start; i < n_cal_cmds; i++) {
			if (i != 5) {
				if (i < 5) {
					start_coeffs[0] = a[i];
					start_coeffs[1] = b[i];
				}

				if (i == 6) {
					start_coeffs[0] = i_min;
					start_coeffs[1] = q_min;
				}

				wlc_lcn40phy_common_write_table(pi, LCN40PHY_TBL_ID_IQLOCAL,
					start_coeffs, 2, 16, 64);

				cal_type = (cal_cmds[i] & 0x0f00) >> 8;

				/* get & set intervals */
				command_num = command_nums[i];
				if (ncorr_override[cal_type])
					command_num = ncorr_override[cal_type] << 8 |
					(command_num & 0xff);
				/* enable IQLO select MUX */
				phy_utils_or_phyreg(pi, LCN40PHY_sslpnCtrl3, 4);
				phy_utils_write_phyreg(pi, LCN40PHY_iqloCalCmdNnum, command_num);

				PHY_TMP(("wl%d: %s: running cmd: %x, cmd_num: %x\n",
					pi->sh->unit, __FUNCTION__, cal_cmds[i], command_nums[i]));

				if (cal_type == 0 &&
					(LCN40REV_LE(pi->pubpi.phy_rev, 1)))
					continue;

				/* Issue cal command */
				phy_utils_write_phyreg(pi, LCN40PHY_iqloCalCmd, cal_cmds[i]);

				/* Wait until cal command finished */
				if (!wlc_lcn40phy_iqcal_wait(pi)) {
					PHY_ERROR(("wl%d: %s: tx iqlo cal failed to complete\n",
						pi->sh->unit, __FUNCTION__));
					/* No point to continue */
					goto cleanup;
				}

				if (i < 5) {
					wlc_lcn40phy_common_read_table(pi, LCN40PHY_TBL_ID_IQLOCAL,
						&Bi[i], 1, 16, 96);
					Bi[i] = Bi[i] & 0x3ff;
					wlc_lcn40phy_common_read_table(pi, LCN40PHY_TBL_ID_IQLOCAL,
						&Bq[i], 1, 16, 97);
					Bq[i] = Bq[i] & 0x3ff;
				}

				if (i == 4)
					agidx = PHY_REG_READ(pi, LCN40PHY, IQCaldbg2, agIndex);
			} else {
				/* Manual cal */
				uint32 ripple_bin_min = 1000000, ripple_bin;
				uint16 bbmult_genv;

				wlc_lcn40phy_set_tx_locc(pi, start_coeffs[5]);
				PHY_REG_MOD(pi, LCN40PHY, iqloCalCmd, cal_type, 0);
				phy_utils_write_phyreg(pi, LCN40PHY_iqloCalCmdNnum, 0x7a97);

				bbmult_genv = lcn40phy_iqcal_ir_gainladder[agidx];

				wlc_lcn40phy_set_bbmult(pi, (bbmult_genv >> 8));

				phy_utils_mod_radioreg(pi, RADIO_2065_IQCAL_CFG3, (7 << 4),
					(bbmult_genv & 0xff) << 4);
				phy_utils_mod_radioreg(pi, RADIO_2065_IQCAL_CFG3, (7 << 0),
					(bbmult_genv & 0xff) << 0);

				for (j = 0; j < 4; j++) {
					wlc_lcn40phy_set_tx_iqcc(pi, Bi[j], Bq[j]);
					PHY_REG_MOD(pi, LCN40PHY, iqloCalCmd, iqloCalDFTCmd, 1);

					/* Wait until cal command finished */
					if (!wlc_lcn40phy_iqcal_wait(pi)) {
						PHY_ERROR(
						("wl%d: %s: tx iqlo cal failed to complete\n",
						pi->sh->unit, __FUNCTION__));
						/* No point to continue */
						goto cleanup;
					}

					wlc_lcn40phy_common_read_table(pi, LCN40PHY_TBL_ID_IQLOCAL,
						&ripple_bin, 1, 16, 107);

					if (ripple_bin < ripple_bin_min) {
						ripple_bin_min = ripple_bin;
						i_min = Bi[j];
						q_min = Bq[j];
					}
				}
			} /* i == 5 */
		} /* For loop for cals */
		/* Copy best coefficients to start coefficients */
		wlc_lcn40phy_common_read_table(pi, LCN40PHY_TBL_ID_IQLOCAL,
			best_coeffs, ARRAYSIZE(best_coeffs), 16, 96);
		wlc_lcn40phy_common_write_table(pi, LCN40PHY_TBL_ID_IQLOCAL, best_coeffs,
			ARRAYSIZE(best_coeffs), 16, 64);

#if defined(PHYCAL_CACHING)
		if (ctx)
			wlc_lcn40phy_common_read_table(pi, LCN40PHY_TBL_ID_IQLOCAL,
				cache->txiqlocal_bestcoeffs,
				ARRAYSIZE(cache->txiqlocal_bestcoeffs), 16, 96);
#else
		wlc_lcn40phy_common_read_table(pi, LCN40PHY_TBL_ID_IQLOCAL,
			pi_lcn->lcnphy_cal_results.txiqlocal_bestcoeffs,
			ARRAYSIZE(pi_lcn->lcnphy_cal_results.txiqlocal_bestcoeffs), 16, 96);
#endif /* PHYCAL_CACHING */
	} /* cal_mode == CAL_IQ_CAL3 */

	/*
	 * Apply Results
	 */

	/* Save calibration results */
#if defined(PHYCAL_CACHING)
	if (ctx) {
		wlc_lcn40phy_common_read_table(pi, LCN40PHY_TBL_ID_IQLOCAL,
			cache->txiqlocal_bestcoeffs,
			ARRAYSIZE(cache->txiqlocal_bestcoeffs), 16, 96);
		cache->txiqlocal_bestcoeffs_valid = TRUE;

		/* Apply IQ Cal Results */
		wlc_lcn40phy_common_write_table(pi, LCN40PHY_TBL_ID_IQLOCAL,
			&cache->txiqlocal_bestcoeffs[0], 4, 16, 80);
		/* Apply Digital LOFT Comp */
		wlc_lcn40phy_common_write_table(pi, LCN40PHY_TBL_ID_IQLOCAL,
			&cache->txiqlocal_bestcoeffs[5], 2, 16, 85);
	}
#else
	wlc_lcn40phy_common_read_table(pi, LCN40PHY_TBL_ID_IQLOCAL,
		pi_lcn->lcnphy_cal_results.txiqlocal_bestcoeffs,
		ARRAYSIZE(pi_lcn->lcnphy_cal_results.txiqlocal_bestcoeffs), 16, 96);

	/* Apply IQ Cal Results */
	wlc_lcn40phy_common_write_table(pi, LCN40PHY_TBL_ID_IQLOCAL,
		&pi_lcn->lcnphy_cal_results.txiqlocal_bestcoeffs[0], 4, 16, 80);
	/* Apply Digital LOFT Comp */
	wlc_lcn40phy_common_write_table(pi, LCN40PHY_TBL_ID_IQLOCAL,
		&pi_lcn->lcnphy_cal_results.txiqlocal_bestcoeffs[5], 2, 16, 85);

	/* only for rev0 and 1, not need for rev2 and above */
	if (LCN40REV_LE(pi->pubpi.phy_rev, 1)) {
		wlc_lcn40phy_set_tx_iqcc(pi, 0, 0);
		pi_lcn->lcnphy_cal_results.txiqlocal_bestcoeffs[0] =
			pi_lcn->lcnphy_cal_results.txiqlocal_bestcoeffs[1] = 0;
	}

	pi_lcn->lcnphy_cal_results.txiqlocal_bestcoeffs_valid = TRUE;

	/* Dump results */
	PHY_INFORM(("wl%d: %s complete, IQ %d %d LO %d %d %d %d %d %d\n",
		pi->sh->unit, __FUNCTION__,
		(int16)pi_lcn->lcnphy_cal_results.txiqlocal_bestcoeffs[0],
		(int16)pi_lcn->lcnphy_cal_results.txiqlocal_bestcoeffs[1],
		(int8)((pi_lcn->lcnphy_cal_results.txiqlocal_bestcoeffs[5] & 0xff00) >> 8),
		(int8)(pi_lcn->lcnphy_cal_results.txiqlocal_bestcoeffs[5] & 0x00ff),
		(int8)((pi_lcn->lcnphy_cal_results.txiqlocal_bestcoeffs[7] & 0xff00) >> 8),
		(int8)(pi_lcn->lcnphy_cal_results.txiqlocal_bestcoeffs[7] & 0x00ff),
		(int8)((pi_lcn->lcnphy_cal_results.txiqlocal_bestcoeffs[9] & 0xff00) >> 8),
		(int8)(pi_lcn->lcnphy_cal_results.txiqlocal_bestcoeffs[9] & 0x00ff)));

#endif /* PHYCAL_CACHING */

cleanup:
	/* Switch off test tone */
	if (!keep_tone)
		wlc_lcn40phy_stop_tx_tone(pi);
	/* stop sample play 1 */
	if (CHSPEC_IS40(pi->radio_chanspec) && LCN40REV_LE(pi->pubpi.phy_rev, 3))
		PHY_REG_MOD(pi, LCN40PHY, sampleCmd, buf2foriqlocal, 0);

	if (PHY_EPA_SUPPORT(pi_lcn->ePA))
		wlc_lcn40phy_epa_pd(pi, 0);

	wlc_lcn40phy_tx_iqlo_loopback_cleanup(pi, values_to_save);

	/* restore tx power and reenable tx power control */
	phy_utils_write_phyreg(pi, LCN40PHY_TxPwrCtrlRfCtrl2, save_txpwrctrlrfctrl2);

	/* Reset calibration  command register */
	phy_utils_write_phyreg(pi, LCN40PHY_iqloCalCmdGctl, 0);

	/* Restore tx power and reenable tx power control */
	if (tx_gain_override_old)
		wlc_lcn40phy_set_tx_gain(pi, &old_gains);
	phy_utils_write_phyreg(pi, LCN40PHY_TxPwrCtrlCmd, save_txpwrctrlcmd);
	wlc_lcn40phy_set_tx_pwr_ctrl(pi, tx_pwr_ctrl_old);

	/* Restoration RxFE clk */
	phy_utils_write_phyreg(pi, LCN40PHY_sslpnCalibClkEnCtrl, save_sslpnCalibClkEnCtrl);
	phy_utils_write_phyreg(pi, LCN40PHY_sslpnRxFeClkEnCtrl, save_sslpnRxFeClkEnCtrl);
	phy_utils_write_phyreg(pi, LCN40PHY_ClkEnCtrl, save_ClkEnCtrl);
	PHY_REG_WRITE(pi, LCN40PHY, crsgainCtrl, save_wlpriogainChangeEn);
	PHY_REG_WRITE(pi, LCN40PHY, radioCtrl, save_internalRadioSharingEn);
	PHY_REG_WRITE(pi, LCN40PHY, RFinputOverrideVal, save_BTPriority_ovr_val);
	PHY_REG_WRITE(pi, LCN40PHY, RFinputOverride, save_BTPriority_ovr);
	phy_utils_write_phyreg(pi, LCN40PHY_rxFarrowCtrl, save_rxFarrowCtrl);
}

static void
wlc_lcn40phy_tx_iqlo_loopback(phy_info_t *pi, uint16 *values_to_save, bool epa_or_pad_lpbk)
{
	uint i;
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	/* save rf registers */
	for (i = 0; i < ARRAYSIZE(iqlo_loopback_rf_regs); i++) {
		values_to_save[i] = phy_utils_read_radioreg(pi, iqlo_loopback_rf_regs[i]);
	}
	/* force tx on, rx off , force ADC on */
	PHY_REG_LIST_START
		PHY_REG_MOD_ENTRY(LCN40PHY, RFOverrideVal0, internalrftxpu_ovr_val, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, RFOverride0, internalrftxpu_ovr, 1)

		PHY_REG_MOD_ENTRY(LCN40PHY, RFOverrideVal0, internalrfrxpu_ovr_val, 0)
		PHY_REG_MOD_ENTRY(LCN40PHY, RFOverride0, internalrfrxpu_ovr, 1)

		PHY_REG_MOD_ENTRY(LCN40PHY, AfeCtrlOvr1Val, dac_pu_ovr_val, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, AfeCtrlOvr1, dac_pu_ovr, 1)

		PHY_REG_MOD_ENTRY(LCN40PHY, AfeCtrlOvr1Val, adc_pu_ovr_val, 0x1f)
		PHY_REG_MOD_ENTRY(LCN40PHY, AfeCtrlOvr1, adc_pu_ovr, 1)
	PHY_REG_LIST_EXECUTE(pi);

	/* PA override */
	values_to_save[i++] = wlc_lcn40phy_get_pa_gain(pi);

	PHY_REG_LIST_START
		/* Enable clk */
		PHY_REG_MOD_ENTRY(LCN40PHY, ClkEnCtrl, forcerxfrontendclk, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, AfeCtrlOvr1Val, afe_iqadc_aux_en_ovr_val, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, AfeCtrlOvr1, afe_iqadc_aux_en_ovr, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, sslpnCalibClkEnCtrl, forceTxfiltClkOn, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, sslpnCalibClkEnCtrl, forceaphytxFeclkOn, 1)
		/* Note: We need to turn on the tssi block for txiqcal also */
		RADIO_REG_OR_ENTRY(RADIO_2065_OVR1, 0x8000)
		/* set reg(RF_tx2g_tssi.tx2g_tssi_pu) 1 */
		/* set reg(RF_tx2g_tssi.tx2g_tssi_sel) 1 */
		RADIO_REG_MOD_ENTRY(RADIO_2065_TX2G_TSSI, 0x5, 5)
		/* set reg(RF_iqcal_cfg1.PU_iqcal) 1 */
		RADIO_REG_OR_ENTRY(RADIO_2065_IQCAL_CFG1, 1 << 1)
		/* set reg(RF_testbuf_cfg1.PU) 1 */
		RADIO_REG_OR_ENTRY(RADIO_2065_TESTBUF_CFG1, 1)
		RADIO_REG_OR_ENTRY(RADIO_2065_OVR16, 1)
		/* set reg(RF_auxpga_cfg1.auxpga_pu) 1 */
		RADIO_REG_OR_ENTRY(RADIO_2065_AUXPGA_CFG1, 1)
		/* set reg(RF_pa2g_cfg1.pa2g_pu) 0 */
		/* set reg(RF_OVR8.ovr_pa2g_pu) 1 */
		RADIO_REG_MOD_ENTRY(RADIO_2065_PA2G_CFG1, 0x1, 0)
		RADIO_REG_OR_ENTRY(RADIO_2065_OVR8, 1 << 10)

		/* set test mux to iqcal */
		/* set reg(RF_testbuf_cfg1.sel_test_port) 0x0 */
		/* set reg(RF_OVR12.ovr_testbuf_sel_test_port) 0x1 */
		RADIO_REG_MOD_ENTRY(RADIO_2065_TESTBUF_CFG1, 0x70, 0)
		RADIO_REG_OR_ENTRY(RADIO_2065_OVR12, 1 << 15)
	PHY_REG_LIST_EXECUTE(pi);

	if (pi->tx_alpf_bypass) {
		phy_utils_mod_radioreg(pi, RADIO_2065_LPF_CFG1, 0xfec0, 0x6ec0);
	} else {
		phy_utils_mod_radioreg(pi, RADIO_2065_LPF_CFG1, 0xfec0, 0x6cc0);
	}
	/* set override */
	phy_utils_mod_radioreg(pi, RADIO_2065_OVR7, 0xfc, 0xfc);

	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		PHY_REG_LIST_START
			RADIO_REG_MOD_ENTRY(RADIO_2065_IQCAL_CFG1, 0xF4, 0x84)
			RADIO_REG_MOD_ENTRY(RADIO_2065_TX2G_TSSI, 0x7, 0x5)
			RADIO_REG_OR_ENTRY(RADIO_2065_OVR13, 0x8000)
			RADIO_REG_OR_ENTRY(RADIO_2065_OVR12, 0x3)
		PHY_REG_LIST_EXECUTE(pi);
	}
#ifdef BAND5G
	else {
		if (epa_or_pad_lpbk) {
			if (LCN40REV_IS(pi->pubpi.phy_rev, 7)) /* iPA */
				phy_utils_mod_radioreg(pi, RADIO_2065_IQCAL_CFG1, 0xF4, 0xa4);
			else /* ePA */
				phy_utils_mod_radioreg(pi, RADIO_2065_IQCAL_CFG1, 0xF4, 0xe4);
		} else {
			phy_utils_mod_radioreg(pi, RADIO_2065_IQCAL_CFG1, 0xF4, 0xa4); /* PAD */
		}
		phy_utils_mod_radioreg(pi, RADIO_2065_TX5G_TSSI, 0xF7, 0x75);
		/* 43341 (phy_revid=7) uses different radio OVR registers */
		if (LCN40REV_IS(pi->pubpi.phy_rev, 7))
			phy_utils_or_radioreg(pi, RADIO_2065_OVR13_ACCOREREG, 0x1c0);
		else
			phy_utils_or_radioreg(pi, RADIO_2065_OVR13, 0x1e00);
	}
#endif /* BAND5G */
	/* adc Vmid , range etc. */
	phy_utils_or_radioreg(pi, RADIO_2065_OVR1, 0x6000);
	if ((CHIPID(pi->sh->chip) == BCM4334_CHIP_ID) ||
		(CHIPID(pi->sh->chip) == BCM43340_CHIP_ID) ||
		(CHIPID(pi->sh->chip) == BCM43341_CHIP_ID)||
		(CHIPID(pi->sh->chip) == BCM43342_CHIP_ID))
		phy_utils_mod_radioreg(pi, RADIO_2065_AUXPGA_VMID, 0x3FF, 0x93);
	else
		phy_utils_mod_radioreg(pi, RADIO_2065_AUXPGA_VMID, 0x3FF, 0xa3);

	PHY_REG_LIST_START
		RADIO_REG_MOD_ENTRY(RADIO_2065_AUXPGA_CFG1, 0x700, 0x2 << 8)
		RADIO_REG_MOD_ENTRY(RADIO_2065_OVR7, 0x100, 0x1 << 8)
		/* Lower the rxbuf gain (before ADC) by 6dB to prevent ADC saturation
		  * during iqlo cal. Compensating this by increasing
		  * RX farrow gain (After ADC) by 6dB.
		  */
		RADIO_REG_MOD_ENTRY(RADIO_2065_LPF_GAIN, 0xf00, 5 << 8)
		RADIO_REG_MOD_ENTRY(RADIO_2065_IQCAL_IDAC, 0x3fff, 0x1558)
	PHY_REG_LIST_EXECUTE(pi);
	values_to_save[i] = phy_utils_read_phyreg(pi, LCN40PHY_TxPwrCtrlRfCtrlOverride0);
	if (LCN40REV_IS(pi->pubpi.phy_rev, 7)) {
		PHY_REG_LIST_START
			PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlRfCtrlOverride0, paCtrlTssiOverride, 1)
			PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlRfCtrlOverride0, paCtrlTssiOverrideVal,
				7)
			RADIO_REG_MOD_ENTRY(RADIO_2065_TX5G_TSSI, 0xf0, 0x70)
		PHY_REG_LIST_EXECUTE(pi);
	}

}

static void
wlc_lcn40phy_tx_iqlo_loopback_cleanup(phy_info_t *pi, uint16 *values_to_save)
{
	uint i;
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	PHY_REG_LIST_START
		/* tx, rx PU rewrite to save memory */
		PHY_REG_MOD_ENTRY(LCN40PHY, RFOverride0, internalrftxpu_ovr, 0)
		PHY_REG_MOD_ENTRY(LCN40PHY, RFOverride0, internalrfrxpu_ovr, 0)
		PHY_REG_MOD_ENTRY(LCN40PHY, AfeCtrlOvr1, dac_pu_ovr, 0)
		PHY_REG_MOD_ENTRY(LCN40PHY, AfeCtrlOvr1, adc_pu_ovr, 0)
		PHY_REG_MOD_ENTRY(LCN40PHY, AfeCtrlOvr1, afe_iqadc_aux_en_ovr, 0)
	PHY_REG_LIST_EXECUTE(pi);
	/* restore the state of radio regs */
	for (i = 0; i < ARRAYSIZE(iqlo_loopback_rf_regs); i++) {
		phy_utils_write_radioreg(pi, iqlo_loopback_rf_regs[i], values_to_save[i]);
	}
	wlc_lcn40phy_set_pa_gain(pi, values_to_save[i++]);
	phy_utils_write_phyreg(pi, LCN40PHY_TxPwrCtrlRfCtrlOverride0, values_to_save[i]);
}

static bool
wlc_lcn40phy_iqcal_wait(phy_info_t *pi)
{
	uint delay_count = 0;

	while (wlc_lcn40phy_iqcal_active(pi)) {
		OSL_DELAY(100);
		delay_count++;

		if (delay_count > (10 * 500)) /* 500 ms */
			break;
	}
	PHY_TMP(("wl%d: %s: %u us\n", pi->sh->unit, __FUNCTION__, delay_count * 100));

	return (wlc_lcn40phy_iqcal_active(pi) == 0);
}

static void
wlc_lcn40phy_set_tssi_mux(phy_info_t *pi, lcn40phy_tssi_mode_t pos)
{
	/* Set TSSI/RSSI mux */
	if (LCN40PHY_TSSI_POST_PA == pos) {
		PHY_REG_LIST_START
			PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlRfCtrl0, tssiSelVal0, 0)
			PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlRfCtrl0, tssiSelVal1, 1)
			RADIO_REG_MOD_ENTRY(RADIO_2065_IQCAL_CFG1, 0x4, 0)
		PHY_REG_LIST_EXECUTE(pi);
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			phy_utils_mod_radioreg(pi, RADIO_2065_IQCAL_CFG1, 0xf0, 0);
		}
#ifdef BAND5G
		else {
			phy_utils_mod_radioreg(pi, RADIO_2065_IQCAL_CFG1, 0xf0, 2 << 4);
		}
#endif /* BAND5G */
	} else if (LCN40PHY_TSSI_EXT_POST_PAD == pos) {
		PHY_REG_LIST_START
			PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlRfCtrl0, tssiSelVal0, 1)
			PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlRfCtrl0, tssiSelVal1, 0)
			/* FIX ME: check the below setting for validity */
			PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlRfCtrl0, tssiRangeVal1, 0)
			PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlRfCtrlOverride0, tssiRangeOverride, 1)
			PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlRfCtrlOverride0, tssiRangeOverrideVal,
				0)
			RADIO_REG_MOD_ENTRY(RADIO_2065_IQCAL_CFG1, 0x4, 0)
		PHY_REG_LIST_EXECUTE(pi);
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			phy_utils_mod_radioreg(pi, RADIO_2065_IQCAL_CFG1, 0xf0, 0);
		}
#ifdef BAND5G
		else {
			phy_utils_mod_radioreg(pi, RADIO_2065_IQCAL_CFG1, 0xf0, 2 << 4);
		}
#endif /* BAND5G */
	} else if (LCN40PHY_TSSI_PRE_PA == pos) {
		PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlRfCtrl0, tssiSelVal0, 0x1);
		PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlRfCtrl0, tssiSelVal1, 0);
	} else {
		phy_utils_or_radioreg(pi, RADIO_2065_IQCAL_CFG1, 1 << 2);
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			phy_utils_mod_radioreg(pi, RADIO_2065_IQCAL_CFG1, 0xf0, 1 << 4);
		}
#ifdef BAND5G
		else {
			phy_utils_mod_radioreg(pi, RADIO_2065_IQCAL_CFG1, 0xf0, 3 << 4);
		}
#endif /* BAND5G */
	}

	PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlCmdNew, txPwrCtrlScheme, 0);
}

static uint16
wlc_lcn40phy_rfseq_tbl_adc_pwrup(phy_info_t *pi)
{
	uint16 N1, N2, N3, N4, N5, N6, N;

	N1 = PHY_REG_READ(pi, LCN40PHY, TxPwrCtrlNnum, Ntssi_delay);
	N2 = 1 << PHY_REG_READ(pi, LCN40PHY, TxPwrCtrlNnum, Ntssi_intg_log2);
	N3 = PHY_REG_READ(pi, LCN40PHY, TxPwrCtrlNum_Vbat, Nvbat_delay);
	N4 = 1 << PHY_REG_READ(pi, LCN40PHY, TxPwrCtrlNum_Vbat, Nvbat_intg_log2);
	N5 = PHY_REG_READ(pi, LCN40PHY, TxPwrCtrlNum_temp, Ntemp_delay);
	N6 = 1 << PHY_REG_READ(pi, LCN40PHY, TxPwrCtrlNum_temp, Ntemp_intg_log2);
	N = 2 * (N1 + N2 + N3 + N4 + 2 *(N5 + N6)) + 80;
	if (N < 1600)
		N = 1600; /* min 20 us to avoid tx evm degradation */
	return N;
}

static void
wlc_lcn40phy_pwrctrl_rssiparams(phy_info_t *pi)
{
	uint16 auxpga_vmid, auxpga_gain = 0;

	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
#ifdef BAND5G
	if (CHSPEC_IS5G(pi->radio_chanspec)) {
		auxpga_vmid = (pi_lcn->rssismc5g << 4) | pi_lcn->rssismf5g;
		auxpga_gain = pi_lcn->rssisav5g;
	} else
#endif // endif
	{
		auxpga_gain = pi_lcn->lcnphy_rssi_gs;
		auxpga_vmid = (pi_lcn->lcnphy_rssi_vc << 4) |
			pi_lcn->lcnphy_rssi_vf;
	}

	PHY_REG_LIST_START
		PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlRfCtrlOverride1, afeAuxpgaSelVmidOverride, 0)
		PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlRfCtrlOverride1, afeAuxpgaSelGainOverride, 0)
		PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlRfCtrlOverride0, amuxSelPortOverride, 0)
	PHY_REG_LIST_EXECUTE(pi);

	phy_utils_mod_phyreg(pi, LCN40PHY_TxPwrCtrlRfCtrl2,
		LCN40PHY_TxPwrCtrlRfCtrl2_afeAuxpgaSelVmidVal0_MASK |
		LCN40PHY_TxPwrCtrlRfCtrl2_afeAuxpgaSelGainVal0_MASK,
		(auxpga_vmid << LCN40PHY_TxPwrCtrlRfCtrl2_afeAuxpgaSelVmidVal0_SHIFT) |
		(auxpga_gain << LCN40PHY_TxPwrCtrlRfCtrl2_afeAuxpgaSelGainVal0_SHIFT));

	phy_utils_mod_phyreg(pi, LCN40PHY_TxPwrCtrlRfCtrl3,
		LCN40PHY_TxPwrCtrlRfCtrl3_afeAuxpgaSelVmidVal1_MASK |
		LCN40PHY_TxPwrCtrlRfCtrl3_afeAuxpgaSelGainVal1_MASK,
		(auxpga_vmid << LCN40PHY_TxPwrCtrlRfCtrl3_afeAuxpgaSelVmidVal1_SHIFT) |
		(auxpga_gain << LCN40PHY_TxPwrCtrlRfCtrl3_afeAuxpgaSelGainVal1_SHIFT));

	phy_utils_mod_phyreg(pi, LCN40PHY_TxPwrCtrlRfCtrl4,
		LCN40PHY_TxPwrCtrlRfCtrl4_afeAuxpgaSelVmidVal2_MASK |
		LCN40PHY_TxPwrCtrlRfCtrl4_afeAuxpgaSelGainVal2_MASK,
		(auxpga_vmid << LCN40PHY_TxPwrCtrlRfCtrl4_afeAuxpgaSelVmidVal2_SHIFT) |
		(auxpga_gain << LCN40PHY_TxPwrCtrlRfCtrl4_afeAuxpgaSelGainVal2_SHIFT));

	phy_utils_mod_phyreg(pi, LCN40PHY_TxPwrCtrlRfCtrl5,
		LCN40PHY_TxPwrCtrlRfCtrl5_afeAuxpgaSelVmidVal3_MASK |
		LCN40PHY_TxPwrCtrlRfCtrl5_afeAuxpgaSelGainVal3_MASK,
		(AUXPGA_VBAT_VMID_VAL << LCN40PHY_TxPwrCtrlRfCtrl5_afeAuxpgaSelVmidVal3_SHIFT) |
		(AUXPGA_VBAT_GAIN_VAL << LCN40PHY_TxPwrCtrlRfCtrl5_afeAuxpgaSelGainVal3_SHIFT));

	if (LCN40REV_IS(pi->pubpi.phy_rev, 6)) {
		auxpga_vmid = AUXPGA_TEMPER_VMID_VAL_43143;
	} else {
		auxpga_vmid = AUXPGA_TEMPER_VMID_VAL;
	}
	phy_utils_mod_phyreg(pi, LCN40PHY_TxPwrCtrlRfCtrl6,
		LCN40PHY_TxPwrCtrlRfCtrl6_afeAuxpgaSelVmidVal4_MASK |
		LCN40PHY_TxPwrCtrlRfCtrl6_afeAuxpgaSelGainVal4_MASK,
		(auxpga_vmid << LCN40PHY_TxPwrCtrlRfCtrl6_afeAuxpgaSelVmidVal4_SHIFT) |
		(AUXPGA_TEMPER_GAIN_VAL << LCN40PHY_TxPwrCtrlRfCtrl6_afeAuxpgaSelGainVal4_SHIFT));
}

static void
wlc_lcn40phy_tssi_setup(phy_info_t *pi)
{
	phytbl_info_t tab;
	uint32 *indxTbl, i;
	uint16 rfseq;
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	phy_info_lcn40phy_t *pi_lcn40 = pi->u.pi_lcn40phy;
	int16 power_correction;

	if ((indxTbl = (uint32*) LCN40PHY_MALLOC(pi, 128 * sizeof(uint32))) == NULL) {
		PHY_ERROR(("wl%d: %s: MALLOC failure\n", pi->sh->unit, __FUNCTION__));
		return;
	}

	/* Setup estPwrLuts for measuring idle TSSI */
	tab.tbl_id = LCN40PHY_TBL_ID_TXPWRCTL;
	tab.tbl_width = 32;	/* 32 bit wide	*/
	tab.tbl_ptr = indxTbl; /* ptr to buf */
	tab.tbl_len = 128;        /* # values   */
	tab.tbl_offset = 0;
	for (i = 0; i < 128; i++) {
		*(indxTbl + i) = i;
	}

	wlc_lcnphy_write_table(pi,  &tab);
	tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_EST_PWR_OFFSET;
	wlc_lcnphy_write_table(pi,  &tab);

	if (indxTbl)
		LCN40PHY_MFREE(pi, indxTbl, 128 * sizeof(uint32));

	if ((BOARDTYPE(pi->sh->boardtype) == BCM94334FCAGBI_SSID) ||
		(BOARDTYPE(pi->sh->boardtype) == BCM94334WLAGBI_SSID) ||
		(BOARDTYPE(pi->sh->boardtype) == BCM943342FCAGBI_SSID))
		wlc_lcn40phy_set_tssi_mux(pi, LCN40PHY_TSSI_EXT_POST_PAD);
	else {
		if (!PHY_EPA_SUPPORT(wlc_phy_getlcnphy_common(pi)->ePA))
			wlc_lcn40phy_set_tssi_mux(pi, LCN40PHY_TSSI_POST_PA);
		else
			wlc_lcn40phy_set_tssi_mux(pi, LCN40PHY_TSSI_EXT);
	}

	PHY_REG_LIST_START
		PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlCmd, hwtxPwrCtrl_en, 0)
		PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlCmd, txPwrCtrl_en, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlRangeCmd, force_vbatTemp, 0)
		PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlCmd, pwrIndex_init, 0)
	PHY_REG_LIST_EXECUTE(pi);
	if (CHSPEC_IS20(pi->radio_chanspec)) {
		PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlNnum, Ntssi_delay, 300);
		PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlNnum, Ntssi_intg_log2, 5);
	} else {
		PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlNnum, Ntssi_delay, 500);
		PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlNnum, Ntssi_intg_log2, 6);
	}

	PHY_REG_LIST_START
		PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlNnum, Npt_intg_log2, 0)
		PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlNum_Vbat, Nvbat_delay, 64)
		PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlNum_Vbat, Nvbat_intg_log2, 4)
		PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlNum_temp, Ntemp_delay, 64)
		PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlNum_temp, Ntemp_intg_log2, 4)
		PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlDeltaPwrLimit, DeltaPwrLimit, 0x1)

		PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlRangeCmd, cckPwrOffset, 0)

		PHY_REG_MOD_ENTRY(LCN40PHY, TempSenseCorrection, tempsenseCorr, 0)
	PHY_REG_LIST_EXECUTE(pi);
	if ((LCN40REV_GE(pi->pubpi.phy_rev, 4)) &&
		(CHSPEC_IS40(pi->radio_chanspec))) {
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			PHY_REG_MOD(pi, LCN40PHY, TempSenseCorrection,
			tempsenseCorr, pi_lcn40->pwr_offset40mhz_2g);
			pi_lcn40->tempsenseCorr = pi_lcn40->pwr_offset40mhz_2g;
		}
#ifdef BAND5G
		else {
			PHY_REG_MOD(pi, LCN40PHY, TempSenseCorrection,
			tempsenseCorr, pi_lcn40->pwr_offset40mhz_5g);
			pi_lcn40->tempsenseCorr = pi_lcn40->pwr_offset40mhz_5g;
		}
#endif /* BAND5G */
	}

	PHY_REG_LIST_START
		/*  Set idleTssi to (2^9-1) in OB format = (2^9-1-2^8) = 0xff in 2C format */
		PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlIdleTssi, rawTssiOffsetBinFormat, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlIdleTssi, idleTssi0, 0xff)
		PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlIdleTssi1, idleTssi1, 0xff)
	PHY_REG_LIST_EXECUTE(pi);

	if (LCN40REV_IS(pi->pubpi.phy_rev, 6)) {
		if (pi_lcn->lcnphy_tssical_time) {
			PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlIdleTssi2, perPktIdleTssiUpdate_en, 1);
			power_correction = pi_lcn40->tempsenseCorr + pi_lcn40->lcnphy_idletssi_corr;
			PHY_REG_MOD(pi, LCN40PHY, TempSenseCorrection,
			tempsenseCorr, power_correction);
		}
		else
			PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlIdleTssi2, perPktIdleTssiUpdate_en, 0);

		PHY_REG_LIST_START
			PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlIdleTssi2, perPktIdleTssi_en, 1)
			PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlIdleTssi3, Nidletssi_delay_cck, 45)
		PHY_REG_LIST_EXECUTE(pi);

		if (CHSPEC_IS20(pi->radio_chanspec)) {
		PHY_REG_LIST_START
			PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlIdleTssi2, Nidletssi_delay, 55)
			PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlIdleTssi2, Nidletssi_intg_log2, 4)
			PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlNnumCCK, Ntssi_intg_log2_cck, 0)
			PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlNnumCCK, Ntssi_delay_cck, 300)
		PHY_REG_LIST_EXECUTE(pi);
		} else {
		PHY_REG_LIST_START
			PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlIdleTssi2, Nidletssi_delay, 93)
			PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlIdleTssi2, Nidletssi_intg_log2, 3)
			PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlNnumCCK, Ntssi_intg_log2_cck, 2)
			PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlNnumCCK, Ntssi_delay_cck, 500)
		PHY_REG_LIST_EXECUTE(pi);
		}
	} else if (LCN40REV_GE(pi->pubpi.phy_rev, 4)) {
		if (pi_lcn->lcnphy_tssical_time) {
			PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlIdleTssi2, perPktIdleTssiUpdate_en, 1);
			power_correction = pi_lcn40->tempsenseCorr + pi_lcn40->lcnphy_idletssi_corr;
			PHY_REG_MOD(pi, LCN40PHY, TempSenseCorrection,
			tempsenseCorr, power_correction);
		}
		else
			PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlIdleTssi2, perPktIdleTssiUpdate_en, 0);

		PHY_REG_LIST_START
			PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlIdleTssi2, perPktIdleTssi_en, 1)
			PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlIdleTssi2, Nidletssi_intg_log2, 4)
			PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlIdleTssi2, Nidletssi_delay, 45)
			PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlIdleTssi3, Nidletssi_delay_cck, 45)

			/*  for CCK average over 40<<0 samples */
			PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlNnumCCK, Ntssi_intg_log2_cck, 0)
			PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlNnumCCK, Ntssi_delay_cck, 300)
		PHY_REG_LIST_EXECUTE(pi);
	}

	PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlRfCtrlOverride0, amuxSelPortOverride, 1);
	PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlRfCtrlOverride0, amuxSelPortOverrideVal, 2);

	wlc_lcn40phy_clear_tx_power_offsets(pi);

	rfseq = wlc_lcn40phy_rfseq_tbl_adc_pwrup(pi);

	tab.tbl_id = LCN40PHY_TBL_ID_RFSEQ;
	tab.tbl_width = 16;	/* 12 bit wide	*/
	tab.tbl_ptr = &rfseq;
	tab.tbl_len = 1;
	tab.tbl_offset = 6;
	wlc_lcnphy_write_table(pi,  &tab);

	PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlCmd, txPwrCtrl_swap_iq, 1);
	/* Only needed for internal tssi (iPA):
	 * Power up envelope detector and bias circuit, only needed for internal tssi
	*/
	if ((!PHY_EPA_SUPPORT(wlc_phy_getlcnphy_common(pi)->ePA)) ||
		((BOARDTYPE(pi->sh->boardtype) == BCM94334FCAGBI_SSID) ||
		(BOARDTYPE(pi->sh->boardtype) == BCM94334WLAGBI_SSID) ||
		(BOARDTYPE(pi->sh->boardtype) == BCM943342FCAGBI_SSID))) {
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			PHY_REG_LIST_START
				RADIO_REG_OR_ENTRY(RADIO_2065_TX2G_TSSI, 1)
				RADIO_REG_OR_ENTRY(RADIO_2065_OVR12, 0x2)
				RADIO_REG_AND_ENTRY(RADIO_2065_TX5G_TSSI, ~1)
			PHY_REG_LIST_EXECUTE(pi);
			/* 43341 (phy_revid=7) uses different radio OVR registers */
			if (LCN40REV_IS(pi->pubpi.phy_rev, 7))
				phy_utils_and_radioreg(pi, RADIO_2065_OVR13_ACCOREREG, ~0x100);
			else
				phy_utils_and_radioreg(pi, RADIO_2065_OVR13, ~0x800);
		}
#ifdef BAND5G
		else {
			phy_utils_or_radioreg(pi, RADIO_2065_TX5G_TSSI, 1);
			if (LCN40REV_IS(pi->pubpi.phy_rev, 7))
				phy_utils_or_radioreg(pi, RADIO_2065_OVR13_ACCOREREG, 0x100);
			else
				phy_utils_or_radioreg(pi, RADIO_2065_OVR13, 0x800);
			phy_utils_and_radioreg(pi, RADIO_2065_TX2G_TSSI, ~1);
			phy_utils_and_radioreg(pi, RADIO_2065_OVR12, ~0x2);
		}
#endif /* BAND5G */

		PHY_REG_LIST_START
		/* Power up tssi path after envelope detector, only needed for internal tssi */
			RADIO_REG_OR_ENTRY(RADIO_2065_IQCAL_CFG1, 1)
			RADIO_REG_OR_ENTRY(RADIO_2065_OVR2, 0x2)

			/* increase envelope detector gain */
			PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlRfCtrlOverride0, paCtrlTssiOverride, 1)
			PHY_REG_MOD_ENTRY(LCN40PHY, TxPwrCtrlRfCtrlOverride0, paCtrlTssiOverrideVal,
				1)
		PHY_REG_LIST_EXECUTE(pi);
		if (LCN40REV_IS(pi->pubpi.phy_rev, 7))
			phy_utils_mod_radioreg(pi, RADIO_2065_TX5G_TSSI, 0xf0, 0x0);
	}
	if (LCN40REV_GE(pi->pubpi.phy_rev, 4)) {
		PHY_REG_LIST_START
			/* JIRA-468: Do not enable overrides for tssi feedback
			* path, direct control takes care of pu/pd
			*/

			/* Disable PU override for AMUX (a.k.a. testbuf) */
			RADIO_REG_AND_ENTRY(RADIO_2065_OVR16, ~1)

			/* Disable PU override for AUX PGA */
			RADIO_REG_AND_ENTRY(RADIO_2065_OVR1, 0x7fff)

			/* Disable PU override for RX buffer */
			RADIO_REG_AND_ENTRY(RADIO_2065_OVR7, ~0xc0)

			/* Disable PU for vbat monitor and tempsense */
			RADIO_REG_AND_ENTRY(RADIO_2065_OVR16, ~0x6)
		PHY_REG_LIST_EXECUTE(pi);
	} else {
		PHY_REG_LIST_START
			/* Power up AMUX (a.k.a. testbuf) */
			RADIO_REG_OR_ENTRY(RADIO_2065_TESTBUF_CFG1, 1)

			/* Power up AUX PGA */
			RADIO_REG_OR_ENTRY(RADIO_2065_AUXPGA_CFG1, 1)
			RADIO_REG_OR_ENTRY(RADIO_2065_OVR1, 0x8000)

			/* Power up RX buffer
			*   lpf_rxbuf_pu in AMS
			*/
			RADIO_REG_OR_ENTRY(RADIO_2065_LPF_CFG1, 0xC0)
			RADIO_REG_OR_ENTRY(RADIO_2065_OVR7, 0xC0)

			/* Power up vbat monitor and tempsense */
			RADIO_REG_OR_ENTRY(RADIO_2065_VBAT_CFG, 1)
			RADIO_REG_OR_ENTRY(RADIO_2065_TEMPSENSE_CFG, 1)
		PHY_REG_LIST_EXECUTE(pi);
	}

	wlc_lcn40phy_pwrctrl_rssiparams(pi);

	/* set rxbufgain to zero and enable override */
	phy_utils_mod_radioreg(pi, RADIO_2065_OVR7, (1<<8), 1 << 8);
	phy_utils_mod_radioreg(pi, RADIO_2065_LPF_GAIN, (0xf<<8), 0);
}

static
int32 wlc_lcn40phy_get_tssi_pwr(phy_info_t *pi, int32 a1, int32 b0,
	int32 b1, uint8 maxlimit, uint8 offset)
{
	int32 tssi, pwr, prev_pwr;
	int32 lcn40phy_tssi_pwr_limit;
	uint8 tssi_ladder_cnt = 0;

	if (maxlimit) {
		prev_pwr = 0x7fffffff;
		lcn40phy_tssi_pwr_limit = 0x7fffffff;
		for (tssi = 0; tssi < 128; tssi++) {
			pwr = wlc_lcnphy_tssi2dbm(tssi, a1, b0, b1);
			if (pwr < prev_pwr) {
				prev_pwr = pwr;
				if (++tssi_ladder_cnt == offset) {
					lcn40phy_tssi_pwr_limit = pwr;
					break;
				}
			}
		}
	} else {
		prev_pwr = 0xffffffff;
		lcn40phy_tssi_pwr_limit = 0xffffffff;
		for (tssi = 127; tssi >= 0; tssi--) {
			pwr = wlc_lcnphy_tssi2dbm(tssi, a1, b0, b1);
			if (pwr > prev_pwr) {
				prev_pwr = pwr;
				if (++tssi_ladder_cnt == offset) {
					lcn40phy_tssi_pwr_limit = pwr;
					break;
				}
			}
		}
}

	return lcn40phy_tssi_pwr_limit;

}

static
void wlc_lcn40phy_get_tssi_offset(phy_info_t *pi, uint8 *offset_maxpwr, uint8 *offset_minpwr)
{
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);

	switch (wlc_phy_chanspec_bandrange_get(pi, pi->radio_chanspec)) {
		case WL_CHAN_FREQ_RANGE_2G:
			*offset_maxpwr = pi_lcn->tssi_ladder_offset_maxpwr_2g;
			*offset_minpwr = pi_lcn->tssi_ladder_offset_minpwr_2g;
			break;
#ifdef BAND5G
	case WL_CHAN_FREQ_RANGE_5GL:
			/* 5 GHz low */
			*offset_maxpwr = pi_lcn->tssi_ladder_offset_maxpwr_5glo;
			*offset_minpwr = pi_lcn->tssi_ladder_offset_minpwr_5glo;
			break;

		case WL_CHAN_FREQ_RANGE_5GM:
			/* 5 GHz middle */
			*offset_maxpwr = pi_lcn->tssi_ladder_offset_maxpwr_5gmid;
			*offset_minpwr = pi_lcn->tssi_ladder_offset_minpwr_5gmid;
			break;

		case WL_CHAN_FREQ_RANGE_5GH:
			/* 5 GHz high */
			*offset_maxpwr = pi_lcn->tssi_ladder_offset_maxpwr_5ghi;
			*offset_minpwr = pi_lcn->tssi_ladder_offset_minpwr_5ghi;
			break;
#endif /* BAND5G */
		default:
			ASSERT(FALSE);
			break;
	}
}

static
void wlc_lcn40phy_set_tssi_pwr_limit(phy_info_t *pi, uint8 mode)
{
	int32 a1 = 0, b0 = 0, b1 = 0;
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	int32 lcn40phy_tssi_maxpwr_limit = 0x7fffffff;
	int32 lcn40phy_tssi_minpwr_limit = 0xffffffff;
	uint8 tssi_ladder_offset_maxpwr = 0, tssi_ladder_offset_minpwr = 0;

#if TWO_POWER_RANGE_TXPWR_CTRL
	int32 lcn40phy_tssi_maxpwr_limit_second = 0x7fffffff;
	int32 lcn40phy_tssi_minpwr_limit_second = 0xffffffff;
#endif // endif

	wlc_phy_get_paparams_for_band(pi, &a1, &b0, &b1);
	wlc_lcn40phy_get_tssi_offset(pi, &tssi_ladder_offset_maxpwr, &tssi_ladder_offset_minpwr);
	lcn40phy_tssi_maxpwr_limit =
		wlc_lcn40phy_get_tssi_pwr(pi, a1, b0, b1, 1, tssi_ladder_offset_maxpwr);
	lcn40phy_tssi_minpwr_limit =
		wlc_lcn40phy_get_tssi_pwr(pi, a1, b0, b1, 0, tssi_ladder_offset_minpwr);

#if TWO_POWER_RANGE_TXPWR_CTRL
	if (pi_lcn->lcnphy_twopwr_txpwrctrl_en) {
		b0 = pi->txpa_2g_lo[0];
		b1 = pi->txpa_2g_lo[1];
		a1 = pi->txpa_2g_lo[2];
		lcn40phy_tssi_maxpwr_limit_second = wlc_lcn40phy_get_tssi_pwr(pi, a1, b0, b1, 1,
			tssi_ladder_offset_maxpwr);
		lcn40phy_tssi_minpwr_limit_second = wlc_lcn40phy_get_tssi_pwr(pi, a1, b0, b1, 0,
			tssi_ladder_offset_minpwr);

		lcn40phy_tssi_maxpwr_limit =
			MAX(lcn40phy_tssi_maxpwr_limit, lcn40phy_tssi_maxpwr_limit_second);
		lcn40phy_tssi_minpwr_limit =
			MIN(lcn40phy_tssi_minpwr_limit, lcn40phy_tssi_minpwr_limit_second);
}
#endif /* #if TWO_POWER_RANGE_TXPWR_CTRL */

	if ((mode == PHY_TSSI_SET_MAX_LIMIT) || (mode == PHY_TSSI_SET_MIN_MAX_LIMIT))
		pi_lcn->tssi_maxpwr_limit = lcn40phy_tssi_maxpwr_limit >> 1;

	if ((mode == PHY_TSSI_SET_MIN_LIMIT) || (mode == PHY_TSSI_SET_MIN_MAX_LIMIT))
		pi_lcn->tssi_minpwr_limit = lcn40phy_tssi_minpwr_limit >> 1;
}

static
void wlc_lcn40phy_get_tssi_floor(phy_info_t *pi, uint16 *floor)
{
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);

	switch (wlc_phy_chanspec_bandrange_get(pi, pi->radio_chanspec)) {
		case WL_CHAN_FREQ_RANGE_2G:
			*floor = pi_lcn->tssi_floor_2g;
			break;
#ifdef BAND5G
	case WL_CHAN_FREQ_RANGE_5GL:
			/* 5 GHz low */
			*floor = pi_lcn->tssi_floor_5glo;
			break;

		case WL_CHAN_FREQ_RANGE_5GM:
			/* 5 GHz middle */
			*floor = pi_lcn->tssi_floor_5gmid;
			break;

		case WL_CHAN_FREQ_RANGE_5GH:
			/* 5 GHz high */
			*floor = pi_lcn->tssi_floor_5ghi;
			break;
#endif /* BAND5G */
		default:
			ASSERT(FALSE);
			break;
	}
}

static
void wlc_lcn40phy_set_txpwr_clamp(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	uint16 tssi_floor = 0, idle_tssi_shift, adj_tssi_min;
	uint16 idleTssi_2C, idleTssi_OB, target_pwr_reg, intended_target;
	phy_info_lcn40phy_t *pi_lcn40 = pi->u.pi_lcn40phy;
	int32 a1 = 0, b0 = 0, b1 = 0;
	int32 target_pwr_cck_max, target_pwr_ofdm_max, pwr, max_ovr_pwr;
	int32 fudge = 0*8; /* 1dB */
	phytbl_info_t tab;
	uint32 rate_table[WL_RATESET_SZ_DSSS + WL_RATESET_SZ_OFDM + WL_RATESET_SZ_HT_MCS];
	uint8 ii;
	uint16 perPktIdleTssi;

	if (pi_lcn->txpwr_clamp_dis || pi_lcn->txpwr_tssifloor_clamp_dis) {
		pi_lcn->target_pwr_ofdm_max = 0x7fffffff;
		pi_lcn->target_pwr_cck_max = 0x7fffffff;
		if (pi_lcn40->btc_clamp) {
			target_pwr_cck_max = BTC_POWER_CLAMP;
			target_pwr_ofdm_max = BTC_POWER_CLAMP;
		} else {
			return;
		}
	} else {

		wlc_lcn40phy_get_tssi_floor(pi, &tssi_floor);
		wlc_phy_get_paparams_for_band(pi, &a1, &b0, &b1);

		if (LCN40REV_GE(pi->pubpi.phy_rev, 4)) {
			perPktIdleTssi = PHY_REG_READ(pi, LCN40PHY, TxPwrCtrlIdleTssi2,
				perPktIdleTssiUpdate_en);
			if (perPktIdleTssi)
				idleTssi_2C = PHY_REG_READ(pi, LCN40PHY, TxPwrCtrlStatusNew6,
					avgidletssi);
			else
				idleTssi_2C = PHY_REG_READ(pi, LCN40PHY, TxPwrCtrlIdleTssi,
					idleTssi0);
		}
		else
			idleTssi_2C = PHY_REG_READ(pi, LCN40PHY, TxPwrCtrlIdleTssi, idleTssi0);

		if (idleTssi_2C >= 256)
			idleTssi_OB = idleTssi_2C - 256;
		else
			idleTssi_OB = idleTssi_2C + 256;

		idleTssi_OB = idleTssi_OB >> 2; /* Converting to 7 bits */
		idle_tssi_shift = (127 - idleTssi_OB) + 4;
		adj_tssi_min = MAX(tssi_floor, idle_tssi_shift);
		pwr = wlc_lcnphy_tssi2dbm(adj_tssi_min, a1, b0, b1);
		target_pwr_ofdm_max = (pwr - fudge) >> 1;
		target_pwr_cck_max = (MIN(pwr, (pwr + pi_lcn->cckPwrOffset)) - fudge) >> 1;
		PHY_TMP(("idleTssi_OB= %d, idle_tssi_shift= %d, adj_tssi_min= %d, "
				"pwr = %d, target_pwr_cck_max = %d, target_pwr_ofdm_max = %d\n",
				idleTssi_OB, idle_tssi_shift, adj_tssi_min, pwr,
				target_pwr_cck_max, target_pwr_ofdm_max));
		pi_lcn->target_pwr_ofdm_max = target_pwr_ofdm_max;
		pi_lcn->target_pwr_cck_max = target_pwr_cck_max;

		if (pi_lcn40->btc_clamp) {
			target_pwr_cck_max = MIN(target_pwr_cck_max, BTC_POWER_CLAMP);
			target_pwr_ofdm_max = MIN(target_pwr_ofdm_max, BTC_POWER_CLAMP);
		}
	}

	if (pi->txpwroverride) {
		max_ovr_pwr = MIN(target_pwr_ofdm_max, target_pwr_cck_max);
		{
			uint8 core;
			FOREACH_ACTV_CORE(pi, pi->sh->phyrxchain, core) {
				pi->tx_power_min_per_core[core] =
					MIN(pi->tx_power_min_per_core[core], max_ovr_pwr);
			}
		}
		return;
	}

	for (ii = 0; ii < ARRAYSIZE(rate_table); ii ++)
		rate_table[ii] = pi_lcn->rate_table[ii];

	/* Adjust Rate Offset Table to ensure intended tx power for every OFDM/CCK */
	/* rate is less than target_power_ofdm_max/target_power_cck_max */
	target_pwr_reg = wlc_lcn40phy_get_target_tx_pwr(pi);
	for (ii = 0; ii < WL_RATESET_SZ_DSSS; ii ++) {
		intended_target = target_pwr_reg - rate_table[ii];
		if (intended_target > target_pwr_cck_max)
			rate_table[ii] = rate_table[ii] + (intended_target - target_pwr_cck_max);
		PHY_TMP(("Rate: %d, maxtar = %d, target = %d, origoff: %d, clampoff: %d\n",
			ii, target_pwr_cck_max, intended_target,
			pi_lcn->rate_table[ii], rate_table[ii]));
	}
	for (ii = WL_RATESET_SZ_DSSS;
		ii < WL_RATESET_SZ_DSSS + WL_RATESET_SZ_OFDM + WL_RATESET_SZ_HT_MCS; ii ++) {
		intended_target = target_pwr_reg - rate_table[ii];
		if (intended_target > target_pwr_ofdm_max)
			rate_table[ii] = rate_table[ii] + (intended_target - target_pwr_ofdm_max);
		PHY_TMP(("Rate: %d, maxtar = %d, target = %d, origoff: %d, clampoff: %d\n",
			ii, target_pwr_ofdm_max, intended_target,
			pi_lcn->rate_table[ii], rate_table[ii]));
	}

	tab.tbl_id = LCN40PHY_TBL_ID_TXPWRCTL;
	tab.tbl_width = 32;	/* 32 bit wide	*/
	tab.tbl_len = ARRAYSIZE(rate_table); /* # values   */
	tab.tbl_ptr = rate_table; /* ptr to buf */
	tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_RATE_OFFSET;
	wlc_lcn40phy_write_table(pi, &tab);
}

/* Save/Restore digital filter state OFDM filter settings */
static void
wlc_lcn40phy_save_restore_dig_filt_state(phy_info_t *pi, bool save, uint16 *filtcoeffs)
{
	int j;

	uint16 addr_ofdm[] = {
		LCN40PHY_txfilt20Stg1Shft,
		LCN40PHY_txfilt20CoeffStg0A1,
		LCN40PHY_txfilt20CoeffStg0A2,
		LCN40PHY_txfilt20CoeffStg0B1,
		LCN40PHY_txfilt20CoeffStg0B2,
		LCN40PHY_txfilt20CoeffStg0B3,
		LCN40PHY_txfilt20CoeffStg1A1,
		LCN40PHY_txfilt20CoeffStg1A2,
		LCN40PHY_txfilt20CoeffStg1B1,
		LCN40PHY_txfilt20CoeffStg1B2,
		LCN40PHY_txfilt20CoeffStg1B3,
		LCN40PHY_txfilt20CoeffStg2A1,
		LCN40PHY_txfilt20CoeffStg2A2,
		LCN40PHY_txfilt20CoeffStg2B1,
		LCN40PHY_txfilt20CoeffStg2B2,
		LCN40PHY_txfilt20CoeffStg2B3,
		LCN40PHY_txfilt20CoeffStg0_leftshift /* This coeff is specific to DAC160 */
		};
	/* Assume 80 MHz Digital filter by default */
	uint8 max_filter_coeffs = LCN40PHY_NUM_DIG_FILT_COEFFS;

	if (save) {
		for (j = 0; j < max_filter_coeffs; j++)
			filtcoeffs[j] = phy_utils_read_phyreg(pi, addr_ofdm[j]);
	} else {
		for (j = 0; j < max_filter_coeffs; j++)
			phy_utils_write_phyreg(pi, addr_ofdm[j], filtcoeffs[j]);
	}
}

static void wlc_lcn40phy_reset_iir_filter(phy_info_t *pi)
{
	PHY_REG_LIST_START
		PHY_REG_MOD_ENTRY(LCN40PHY, sslpnCalibClkEnCtrl, forceTxfiltClkOn, 1)

		PHY_REG_MOD_ENTRY(LCN40PHY, sslpnCtrl0, txSoftReset, 1)

		PHY_REG_MOD_ENTRY(LCN40PHY, sslpnCtrl0, txSoftReset, 0)

		PHY_REG_MOD_ENTRY(LCN40PHY, sslpnCalibClkEnCtrl, forceTxfiltClkOn, 0)
	PHY_REG_LIST_EXECUTE(pi);
}

/* Scale CCK coefficients Radar: 13384319 */
static void
wlc_lcn40phy_cckscale_tx_iir_filter(phy_info_t *pi)
{
	int8 scale_fctr_db = pi->u.pi_lcn40phy->cckscale_fctr_db;

	/* scale CCK tx-iir coefficients based on nvram
	* parameter 'cckscale' (default set to -1)
	*/
	if (scale_fctr_db > 0) {
		uint8 scale_fctr;
		uint16 fltcoeffB1, fltcoeffB2, fltcoeffB3;

		if (scale_fctr_db > MAX_CCK_DB_SCALING)
			scale_fctr_db =  MAX_CCK_DB_SCALING;
		scale_fctr = LCN40PHY_db2scalefctr_cck[scale_fctr_db-1];

		fltcoeffB1 = phy_utils_read_phyreg(pi, LCN40PHY_ccktxfilt20CoeffStg2B1);
		fltcoeffB2 = phy_utils_read_phyreg(pi, LCN40PHY_ccktxfilt20CoeffStg2B2);
		fltcoeffB3 = phy_utils_read_phyreg(pi, LCN40PHY_ccktxfilt20CoeffStg2B3);

		phy_utils_write_phyreg(pi, LCN40PHY_ccktxfilt20CoeffStg2B1,
		((fltcoeffB1 * scale_fctr) >> DB2SCALEFCTR_SHIFT));
		phy_utils_write_phyreg(pi, LCN40PHY_ccktxfilt20CoeffStg2B2,
		((fltcoeffB2 * scale_fctr) >> DB2SCALEFCTR_SHIFT));
		phy_utils_write_phyreg(pi, LCN40PHY_ccktxfilt20CoeffStg2B3,
		((fltcoeffB3 * scale_fctr) >> DB2SCALEFCTR_SHIFT));
	}
}

/* set tx digital filter coefficients */
static int
wlc_lcn40phy_load_tx_iir_filter(phy_info_t *pi, phy_tx_iir_filter_mode_t mode, int16 filt_type)
{
	int16 filt_index = -1, j;
	uint16 (*dac_coeffs_table)[LCN40PHY_NUM_DIG_FILT_COEFFS+1];
	uint8 max_filter_type, max_filter_coeffs;
	uint16 *addr_coeff;

	uint16 addr_cck[] = {
		LCN40PHY_ccktxfilt20Stg1Shft,
		LCN40PHY_ccktxfilt20CoeffStg0A1,
		LCN40PHY_ccktxfilt20CoeffStg0A2,
		LCN40PHY_ccktxfilt20CoeffStg0B1,
		LCN40PHY_ccktxfilt20CoeffStg0B2,
		LCN40PHY_ccktxfilt20CoeffStg0B3,
		LCN40PHY_ccktxfilt20CoeffStg1A1,
		LCN40PHY_ccktxfilt20CoeffStg1A2,
		LCN40PHY_ccktxfilt20CoeffStg1B1,
		LCN40PHY_ccktxfilt20CoeffStg1B2,
		LCN40PHY_ccktxfilt20CoeffStg1B3,
		LCN40PHY_ccktxfilt20CoeffStg2A1,
		LCN40PHY_ccktxfilt20CoeffStg2A2,
		LCN40PHY_ccktxfilt20CoeffStg2B1,
		LCN40PHY_ccktxfilt20CoeffStg2B2,
		LCN40PHY_ccktxfilt20CoeffStg2B3,
		LCN40PHY_ccktxfilt20CoeffStg0_leftshift
		};

	uint16 addr_ofdm[] = {
		LCN40PHY_txfilt20Stg1Shft,
		LCN40PHY_txfilt20CoeffStg0A1,
		LCN40PHY_txfilt20CoeffStg0A2,
		LCN40PHY_txfilt20CoeffStg0B1,
		LCN40PHY_txfilt20CoeffStg0B2,
		LCN40PHY_txfilt20CoeffStg0B3,
		LCN40PHY_txfilt20CoeffStg1A1,
		LCN40PHY_txfilt20CoeffStg1A2,
		LCN40PHY_txfilt20CoeffStg1B1,
		LCN40PHY_txfilt20CoeffStg1B2,
		LCN40PHY_txfilt20CoeffStg1B3,
		LCN40PHY_txfilt20CoeffStg2A1,
		LCN40PHY_txfilt20CoeffStg2A2,
		LCN40PHY_txfilt20CoeffStg2B1,
		LCN40PHY_txfilt20CoeffStg2B2,
		LCN40PHY_txfilt20CoeffStg2B3,
		LCN40PHY_txfilt20CoeffStg0_leftshift
		};
	uint16 addr_ofdm_40[] = {
		LCN40PHY_txfilt40Stg1Shft,
		LCN40PHY_txfilt40CoeffStg0A1,
		LCN40PHY_txfilt40CoeffStg0A2,
		LCN40PHY_txfilt40CoeffStg0B1,
		LCN40PHY_txfilt40CoeffStg0B2,
		LCN40PHY_txfilt40CoeffStg0B3,
		LCN40PHY_txfilt40CoeffStg1A1,
		LCN40PHY_txfilt40CoeffStg1A2,
		LCN40PHY_txfilt40CoeffStg1B1,
		LCN40PHY_txfilt40CoeffStg1B2,
		LCN40PHY_txfilt40CoeffStg1B3,
		LCN40PHY_txfilt40CoeffStg2A1,
		LCN40PHY_txfilt40CoeffStg2A2,
		LCN40PHY_txfilt40CoeffStg2B1,
		LCN40PHY_txfilt40CoeffStg2B2,
		LCN40PHY_txfilt40CoeffStg2B3,
		LCN40PHY_txfilt40CoeffStg0_leftshift
		};

	switch (mode) {
	case (TX_IIR_FILTER_OFDM):
		addr_coeff = (uint16 *)addr_ofdm;
		dac_coeffs_table = LCN40PHY_txdigfiltcoeffs_ofdm;
		max_filter_type = LCN40PHY_NUM_TX_DIG_FILTERS_OFDM;
		break;
	case (TX_IIR_FILTER_OFDM40):
		addr_coeff = (uint16 *)addr_ofdm_40;
		dac_coeffs_table = LCN40PHY_txdigfiltcoeffs_ofdm40;
		max_filter_type = LCN40PHY_NUM_TX_DIG_FILTERS_OFDM40;
		break;
	case (TX_IIR_FILTER_CCK):
		addr_coeff = (uint16 *)addr_cck;
		dac_coeffs_table = LCN40PHY_txdigfiltcoeffs_cck;
		max_filter_type = LCN40PHY_NUM_TX_DIG_FILTERS_CCK;
		break;
	default:
		/* something wierd happened if coming here */
		addr_coeff = NULL;
		dac_coeffs_table = NULL;
		max_filter_type = 0;
		ASSERT(FALSE);
	}

	max_filter_coeffs = LCN40PHY_NUM_DIG_FILT_COEFFS - 1;

	/* Search for the right entry in the table */
	for (j = 0; j < max_filter_type; j++) {
		if (filt_type == dac_coeffs_table[j][0]) {
			filt_index = (int16)j;
			break;
		}
	}

	/* Grave problem if entry not found */
	if (filt_index == -1) {
		ASSERT(FALSE);
	} else {
		/* Apply the coefficients to the filter type */
		for (j = 0; j < max_filter_coeffs; j++)
			phy_utils_write_phyreg(pi, addr_coeff[j],
			                       dac_coeffs_table[filt_index][j+1]);

		/* Scale cck coeffs */
		if (mode == TX_IIR_FILTER_CCK)
			wlc_lcn40phy_cckscale_tx_iir_filter(pi);
	}

	/* Reset the iir filter after setting the coefficients */
	wlc_lcn40phy_reset_iir_filter(pi);

	return (filt_index != -1) ? 0 : -1;
}

/* set tx digital filter coefficients */
static int
wlc_lcn40phy_scale_tx_iir_filter(phy_info_t *pi, phy_tx_iir_filter_mode_t mode,
	int16 filt_type, int shift_factor)
{

	int16 filt_index = -1, j;
	uint16 (*dac_coeffs_table)[LCN40PHY_NUM_DIG_FILT_COEFFS+1];
	uint8 max_filter_type, max_filter_coeffs;
	uint16 *addr_coeff;

	uint16 addr_cck[] = {
		LCN40PHY_ccktxfilt20Stg1Shft,
		LCN40PHY_ccktxfilt20CoeffStg0A1,
		LCN40PHY_ccktxfilt20CoeffStg0A2,
		LCN40PHY_ccktxfilt20CoeffStg0B1,
		LCN40PHY_ccktxfilt20CoeffStg0B2,
		LCN40PHY_ccktxfilt20CoeffStg0B3,
		LCN40PHY_ccktxfilt20CoeffStg1A1,
		LCN40PHY_ccktxfilt20CoeffStg1A2,
		LCN40PHY_ccktxfilt20CoeffStg1B1,
		LCN40PHY_ccktxfilt20CoeffStg1B2,
		LCN40PHY_ccktxfilt20CoeffStg1B3,
		LCN40PHY_ccktxfilt20CoeffStg2A1,
		LCN40PHY_ccktxfilt20CoeffStg2A2,
		LCN40PHY_ccktxfilt20CoeffStg2B1,
		LCN40PHY_ccktxfilt20CoeffStg2B2,
		LCN40PHY_ccktxfilt20CoeffStg2B3,
		LCN40PHY_ccktxfilt20CoeffStg0_leftshift
		};

	uint16 addr_ofdm[] = {
		LCN40PHY_txfilt20Stg1Shft,
		LCN40PHY_txfilt20CoeffStg0A1,
		LCN40PHY_txfilt20CoeffStg0A2,
		LCN40PHY_txfilt20CoeffStg0B1,
		LCN40PHY_txfilt20CoeffStg0B2,
		LCN40PHY_txfilt20CoeffStg0B3,
		LCN40PHY_txfilt20CoeffStg1A1,
		LCN40PHY_txfilt20CoeffStg1A2,
		LCN40PHY_txfilt20CoeffStg1B1,
		LCN40PHY_txfilt20CoeffStg1B2,
		LCN40PHY_txfilt20CoeffStg1B3,
		LCN40PHY_txfilt20CoeffStg2A1,
		LCN40PHY_txfilt20CoeffStg2A2,
		LCN40PHY_txfilt20CoeffStg2B1,
		LCN40PHY_txfilt20CoeffStg2B2,
		LCN40PHY_txfilt20CoeffStg2B3,
		LCN40PHY_txfilt20CoeffStg0_leftshift
		};
	uint16 addr_ofdm_40[] = {
		LCN40PHY_txfilt40Stg1Shft,
		LCN40PHY_txfilt40CoeffStg0A1,
		LCN40PHY_txfilt40CoeffStg0A2,
		LCN40PHY_txfilt40CoeffStg0B1,
		LCN40PHY_txfilt40CoeffStg0B2,
		LCN40PHY_txfilt40CoeffStg0B3,
		LCN40PHY_txfilt40CoeffStg1A1,
		LCN40PHY_txfilt40CoeffStg1A2,
		LCN40PHY_txfilt40CoeffStg1B1,
		LCN40PHY_txfilt40CoeffStg1B2,
		LCN40PHY_txfilt40CoeffStg1B3,
		LCN40PHY_txfilt40CoeffStg2A1,
		LCN40PHY_txfilt40CoeffStg2A2,
		LCN40PHY_txfilt40CoeffStg2B1,
		LCN40PHY_txfilt40CoeffStg2B2,
		LCN40PHY_txfilt40CoeffStg2B3,
		LCN40PHY_txfilt40CoeffStg0_leftshift
		};

	switch (mode) {
	case (TX_IIR_FILTER_OFDM):
		addr_coeff = (uint16 *)addr_ofdm;
		dac_coeffs_table = LCN40PHY_txdigfiltcoeffs_ofdm;
		max_filter_type = LCN40PHY_NUM_TX_DIG_FILTERS_OFDM;
		break;
	case (TX_IIR_FILTER_OFDM40):
		addr_coeff = (uint16 *)addr_ofdm_40;
		dac_coeffs_table = LCN40PHY_txdigfiltcoeffs_ofdm40;
		max_filter_type = LCN40PHY_NUM_TX_DIG_FILTERS_OFDM40;
		break;
	case (TX_IIR_FILTER_CCK):
		addr_coeff = (uint16 *)addr_cck;
		dac_coeffs_table = LCN40PHY_txdigfiltcoeffs_cck;
		max_filter_type = LCN40PHY_NUM_TX_DIG_FILTERS_CCK;
		break;
	default:
		/* something wierd happened if coming here */
		addr_coeff = NULL;
		dac_coeffs_table = NULL;
		max_filter_type = 0;
		ASSERT(FALSE);
	}

	max_filter_coeffs = LCN40PHY_NUM_DIG_FILT_COEFFS - 1;

	/* Search for the right entry in the table */
	for (j = 0; j < max_filter_type; j++) {
		if (filt_type == dac_coeffs_table[j][0]) {
			filt_index = (int16)j;
			break;
		}
	}

	/* Grave problem if entry not found */
	if (filt_index == -1) {
		ASSERT(FALSE);
	} else {
		/* Apply the coefficients to the filter type */
		for (j = 0; j < max_filter_coeffs; j++)
			phy_utils_write_phyreg(pi, addr_coeff[j],
			                       dac_coeffs_table[filt_index][j+1]);

		/* Apply the coefficients to the filter type */
		/* Raj: scale down the last stage coeffts by 2 */
		for (j =  max_filter_coeffs-3; j < max_filter_coeffs; j++)
			phy_utils_write_phyreg(pi, addr_coeff[j],
			((dac_coeffs_table[filt_index][j+1] * shift_factor) >> 6));

		/* Scale cck coeffs */
		if (mode == TX_IIR_FILTER_CCK)
			wlc_lcn40phy_cckscale_tx_iir_filter(pi);
	}

	/* Reset the iir filter after setting the coefficients */
	wlc_lcn40phy_reset_iir_filter(pi);

	return (filt_index != -1) ? 0 : -1;

	return (0);

}

static void
wlc_lcn40phy_rx_gain_override_enable(phy_info_t *pi, bool enable)
{
	uint16 ebit = enable ? 1 : 0;

	if (LCN40REV_IS(pi->pubpi.phy_rev, 7)) {
		if (RADIOVER(pi->pubpi.radiover) == 0x4) {
			if (CHSPEC_IS2G(pi->radio_chanspec))
				PHY_REG_MOD(pi, LCN40PHY, rfoverride4, trsw_rx_pwrup_ovr, ebit);
#ifdef BAND5G
			else
				PHY_REG_MOD(pi, LCN40PHY, RFOverride0, trsw_rx_pu_ovr, ebit);
#endif /* BAND5G */
		} else {
			if (CHSPEC_IS2G(pi->radio_chanspec))
				PHY_REG_MOD(pi, LCN40PHY, rfoverride4, trsw_rx_pwrup_ovr, ebit);
#ifdef BAND5G
			else
				PHY_REG_MOD(pi, LCN40PHY, rfoverride8,
					amode_trsw_rx_pwrup_ovr, ebit);
#endif /* BAND5G */
		}
	} else {
		PHY_REG_MOD(pi, LCN40PHY, RFOverride0, trsw_rx_pu_ovr, ebit);
	}
	if ((CHSPEC_IS2G(pi->radio_chanspec) &&
		(BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA)) ||
		(CHSPEC_IS5G(pi->radio_chanspec) &&
		(BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA_5GHz))) {
		phy_utils_mod_phyreg(pi, LCN40PHY_rfoverride2,
			LCN40PHY_rfoverride2_ext_lna_gain_ovr_MASK |
			LCN40PHY_rfoverride2_slna_byp_ovr_MASK |
			LCN40PHY_rfoverride2_slna_gain_ctrl_ovr_MASK,
			(ebit << LCN40PHY_rfoverride2_ext_lna_gain_ovr_SHIFT) |
			(ebit << LCN40PHY_rfoverride2_slna_byp_ovr_SHIFT) |
			(ebit << LCN40PHY_rfoverride2_slna_gain_ctrl_ovr_SHIFT));
	} else {
		phy_utils_mod_phyreg(pi, LCN40PHY_rfoverride2,
			LCN40PHY_rfoverride2_slna_byp_ovr_MASK |
			LCN40PHY_rfoverride2_slna_gain_ctrl_ovr_MASK,
			(ebit << LCN40PHY_rfoverride2_slna_byp_ovr_SHIFT) |
			(ebit << LCN40PHY_rfoverride2_slna_gain_ctrl_ovr_SHIFT));
	}
	PHY_REG_MOD(pi, LCN40PHY, rfoverride3, slna_rout_ctrl_ovr, ebit);

	phy_utils_mod_phyreg(pi, LCN40PHY_rfoverride5,
		LCN40PHY_rfoverride5_rxrf_lna2_gain_ovr_MASK |
		LCN40PHY_rfoverride5_rxrf_lna2_rout_ovr_MASK |
		LCN40PHY_rfoverride5_rxrf_tia_gain_ovr_MASK,
		(ebit << LCN40PHY_rfoverride5_rxrf_lna2_gain_ovr_SHIFT) |
		(ebit << LCN40PHY_rfoverride5_rxrf_lna2_rout_ovr_SHIFT) |
		(ebit << LCN40PHY_rfoverride5_rxrf_tia_gain_ovr_SHIFT));

	phy_utils_mod_phyreg(pi, LCN40PHY_rfoverride6,
		LCN40PHY_rfoverride6_lpf_bq1_gain_ovr_MASK |
		LCN40PHY_rfoverride6_lpf_bq2_gain_ovr_MASK,
		(ebit << LCN40PHY_rfoverride6_lpf_bq1_gain_ovr_SHIFT) |
		(ebit << LCN40PHY_rfoverride6_lpf_bq2_gain_ovr_SHIFT));

	PHY_REG_MOD(pi, LCN40PHY, radioCtrl, digi_gain_ovr, ebit);
}

static void
wlc_lcn40phy_set_rx_gain_by_distribution(phy_info_t *pi, uint16 trsw, uint16 ext_lna,
    uint16 slna_byp, uint16 slna_rout, uint16 slna_gain, uint16 lna2_gain, uint16 lna2_rout,
	uint16 tia, uint16 biq1, uint16 biq2, uint16 digi_gain, uint16 digi_offset)
{
	if (LCN40REV_IS(pi->pubpi.phy_rev, 7)) {
		if (RADIOVER(pi->pubpi.radiover) == 0x4) {
			if (CHSPEC_IS2G(pi->radio_chanspec))
				PHY_REG_MOD(pi, LCN40PHY, rfoverride4val,
					trsw_rx_pwrup_ovr_val, (!trsw));
#ifdef BAND5G
			else
				PHY_REG_MOD(pi, LCN40PHY, RFOverrideVal0,
					trsw_rx_pu_ovr_val, (!trsw));
#endif /* BAND5G */
		} else {
			if (CHSPEC_IS2G(pi->radio_chanspec))
				PHY_REG_MOD(pi, LCN40PHY, rfoverride4val,
					trsw_rx_pwrup_ovr_val, (!trsw));
#ifdef BAND5G
			else
				PHY_REG_MOD(pi, LCN40PHY, rfoverride8val,
					amode_trsw_rx_pwrup_ovr_val, (!trsw));
#endif /* BAND5G */
		}
	} else {
		PHY_REG_MOD(pi, LCN40PHY, RFOverrideVal0, trsw_rx_pu_ovr_val, (!trsw));
	}
	if ((CHSPEC_IS2G(pi->radio_chanspec) &&
		(BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA)) ||
		(CHSPEC_IS5G(pi->radio_chanspec) &&
		(BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA_5GHz))) {
		phy_utils_mod_phyreg(pi, LCN40PHY_rfoverride2val,
			LCN40PHY_rfoverride2val_ext_lna_gain_ovr_val_MASK |
			LCN40PHY_rfoverride2val_slna_byp_ovr_val_MASK |
			LCN40PHY_rfoverride2val_slna_gain_ctrl_ovr_val_MASK,
			(ext_lna << LCN40PHY_rfoverride2val_ext_lna_gain_ovr_val_SHIFT) |
			(slna_byp << LCN40PHY_rfoverride2val_slna_byp_ovr_val_SHIFT) |
			(slna_gain << LCN40PHY_rfoverride2val_slna_gain_ctrl_ovr_val_SHIFT));
	} else {
		phy_utils_mod_phyreg(pi, LCN40PHY_rfoverride2val,
			LCN40PHY_rfoverride2val_slna_byp_ovr_val_MASK |
			LCN40PHY_rfoverride2val_slna_gain_ctrl_ovr_val_MASK,
			(slna_byp << LCN40PHY_rfoverride2val_slna_byp_ovr_val_SHIFT) |
			(slna_gain << LCN40PHY_rfoverride2val_slna_gain_ctrl_ovr_val_SHIFT));
	}
	PHY_REG_MOD(pi, LCN40PHY, rfoverride3_val, slna_rout_ctrl_ovr_val, slna_rout);
	phy_utils_mod_phyreg(pi, LCN40PHY_rfoverride5val,
		LCN40PHY_rfoverride5val_rxrf_lna2_gain_ovr_val_MASK |
		LCN40PHY_rfoverride5val_rxrf_lna2_rout_ovr_val_MASK |
		LCN40PHY_rfoverride5val_rxrf_tia_gain_ovr_val_MASK,
		(lna2_gain << LCN40PHY_rfoverride5val_rxrf_lna2_gain_ovr_val_SHIFT) |
		(lna2_rout << LCN40PHY_rfoverride5val_rxrf_lna2_rout_ovr_val_SHIFT) |
		(tia << LCN40PHY_rfoverride5val_rxrf_tia_gain_ovr_val_SHIFT));

	phy_utils_mod_phyreg(pi, LCN40PHY_rfoverride6val,
		LCN40PHY_rfoverride6val_lpf_bq1_gain_ovr_val_MASK |
		LCN40PHY_rfoverride6val_lpf_bq2_gain_ovr_val_MASK,
		(biq1 << LCN40PHY_rfoverride6val_lpf_bq1_gain_ovr_val_SHIFT) |
		(biq2 << LCN40PHY_rfoverride6val_lpf_bq2_gain_ovr_val_SHIFT));

	PHY_REG_MOD(pi, LCN40PHY, radioCtrl, digi_gain_ovr_val, digi_gain);
}

static void
wlc_lcn40phy_rx_pu(phy_info_t *pi, bool bEnable)
{
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	if (!bEnable) {
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			PHY_REG_MOD(pi, LCN40PHY, RFOverrideVal0, internalrfrxpu_ovr_val, 0);
			PHY_REG_MOD(pi, LCN40PHY, RFOverride0, internalrfrxpu_ovr, 1);
		}
#ifdef BAND5G
		else {
			PHY_REG_MOD(pi, LCN40PHY, RFOverrideVal0, amode_rx_pu_ovr_val, 0);
			PHY_REG_MOD(pi, LCN40PHY, RFOverride0, amode_rx_pu_ovr, 1);
		}
#endif /* BAND5G */
		wlc_lcn40phy_set_rx_gain_by_distribution(pi, 0, 0, 5, 0, 0, 6, 0, 4, 4, 6, 7, 0);
		wlc_lcn40phy_rx_gain_override_enable(pi, TRUE);
	} else {
		/* Force on the receive chain */
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			PHY_REG_LIST_START
				PHY_REG_MOD_ENTRY(LCN40PHY, RFOverrideVal0, internalrfrxpu_ovr_val,
					1)
				PHY_REG_MOD_ENTRY(LCN40PHY, RFOverride0, internalrfrxpu_ovr, 1)
				PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride2val, slna_pu_ovr_val, 0)
				PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride2, slna_pu_ovr, 0)
			PHY_REG_LIST_EXECUTE(pi);
		}
#ifdef BAND5G
		else {
			PHY_REG_LIST_START
				PHY_REG_MOD_ENTRY(LCN40PHY, RFOverrideVal0, amode_rx_pu_ovr_val, 1)
				PHY_REG_MOD_ENTRY(LCN40PHY, RFOverride0, amode_rx_pu_ovr, 1)
				PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride2val, slna_pu_ovr_val, 1)
				PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride2, slna_pu_ovr, 1)
			PHY_REG_LIST_EXECUTE(pi);
		}
#endif /* BAND5G */
		wlc_lcn40phy_set_rx_gain_by_distribution(pi, 0, 0, 5, 0, 0, 4, 0, 2, 0, 3, 0, 0);
		wlc_lcn40phy_rx_gain_override_enable(pi, TRUE);
	}
}

static void
wlc_lcn40phy_set_trsw_override(phy_info_t *pi, bool tx, bool rx)
{
	if (LCN40REV_IS(pi->pubpi.phy_rev, 7)) {
		if (RADIOVER(pi->pubpi.radiover) == 0x4) {
			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				phy_utils_mod_phyreg(pi, LCN40PHY_rfoverride4val,
					LCN40PHY_rfoverride4val_trsw_tx_pwrup_ovr_val_MASK |
					LCN40PHY_rfoverride4val_trsw_rx_pwrup_ovr_val_MASK,
					(tx ?
					LCN40PHY_rfoverride4val_trsw_tx_pwrup_ovr_val_MASK : 0) |
					(rx ?
					LCN40PHY_rfoverride4val_trsw_rx_pwrup_ovr_val_MASK : 0));
				/* Enable overrides */
				phy_utils_or_phyreg(pi, LCN40PHY_rfoverride4,
					LCN40PHY_rfoverride4_trsw_tx_pwrup_ovr_MASK |
					LCN40PHY_rfoverride4_trsw_rx_pwrup_ovr_MASK);
			}
#ifdef BAND5G
			else {
				phy_utils_mod_phyreg(pi, LCN40PHY_RFOverrideVal0,
					LCN40PHY_RFOverrideVal0_trsw_tx_pu_ovr_val_MASK |
					LCN40PHY_RFOverrideVal0_trsw_rx_pu_ovr_val_MASK,
					(tx ?
					LCN40PHY_RFOverrideVal0_trsw_tx_pu_ovr_val_MASK : 0) |
					(rx ?
					LCN40PHY_RFOverrideVal0_trsw_rx_pu_ovr_val_MASK : 0));
				/* Enable overrides */
				phy_utils_or_phyreg(pi, LCN40PHY_RFOverride0,
					LCN40PHY_RFOverride0_trsw_tx_pu_ovr_MASK |
					LCN40PHY_RFOverride0_trsw_rx_pu_ovr_MASK);
			}
#endif /* BAND5G */
		} else {
			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				phy_utils_mod_phyreg(pi, LCN40PHY_rfoverride4val,
					LCN40PHY_rfoverride4val_trsw_tx_pwrup_ovr_val_MASK |
					LCN40PHY_rfoverride4val_trsw_rx_pwrup_ovr_val_MASK,
					(tx ?
					LCN40PHY_rfoverride4val_trsw_tx_pwrup_ovr_val_MASK : 0) |
					(rx ?
					LCN40PHY_rfoverride4val_trsw_rx_pwrup_ovr_val_MASK : 0));
				/* Enable overrides */
				phy_utils_or_phyreg(pi, LCN40PHY_rfoverride4,
					LCN40PHY_rfoverride4_trsw_tx_pwrup_ovr_MASK |
					LCN40PHY_rfoverride4_trsw_rx_pwrup_ovr_MASK);
			}
#ifdef BAND5G
			else {
				phy_utils_mod_phyreg(pi, LCN40PHY_rfoverride4val,
					LCN40PHY_rfoverride4val_trsw_tx_pwrup_ovr_val_MASK,
					(tx ?
					LCN40PHY_rfoverride4val_trsw_tx_pwrup_ovr_val_MASK : 0));
				phy_utils_mod_phyreg(pi, LCN40PHY_rfoverride8val,
					LCN40PHY_rfoverride8val_amode_trsw_rx_pwrup_ovr_val_MASK,
					(rx ?
					LCN40PHY_rfoverride8val_amode_trsw_rx_pwrup_ovr_val_MASK
					: 0));
				/* Enable overrides */
				phy_utils_or_phyreg(pi, LCN40PHY_rfoverride4,
					LCN40PHY_rfoverride4_trsw_tx_pwrup_ovr_MASK);
				phy_utils_or_phyreg(pi, LCN40PHY_rfoverride8,
					LCN40PHY_rfoverride8_amode_trsw_rx_pwrup_ovr_MASK);
			}
#endif /* BAND5G */
		}
	} else if (CHIPID(pi->sh->chip) == BCM43143_CHIP_ID) {
		/* tx == rx is not unsupported for the 43143 iTRsw */
		ASSERT(tx != rx);
		phy_utils_mod_phyreg(pi, LCN40PHY_rfoverride4val,
			LCN40PHY_rfoverride4val_trsw_tx_pwrup_ovr_val_MASK |
			LCN40PHY_rfoverride4val_trsw_rx_pwrup_ovr_val_MASK,
			(tx ? LCN40PHY_rfoverride4val_trsw_tx_pwrup_ovr_val_MASK : 0) |
			(rx ? LCN40PHY_rfoverride4val_trsw_rx_pwrup_ovr_val_MASK : 0));
		/* Enable overrides */
		phy_utils_or_phyreg(pi, LCN40PHY_rfoverride4,
			LCN40PHY_rfoverride4_trsw_tx_pwrup_ovr_MASK |
			LCN40PHY_rfoverride4_trsw_rx_pwrup_ovr_MASK);
	} else {
		/* Set TR switch */
		phy_utils_mod_phyreg(pi, LCN40PHY_RFOverrideVal0,
			LCN40PHY_RFOverrideVal0_trsw_tx_pu_ovr_val_MASK |
			LCN40PHY_RFOverrideVal0_trsw_rx_pu_ovr_val_MASK,
			(tx ? LCN40PHY_RFOverrideVal0_trsw_tx_pu_ovr_val_MASK : 0) |
			(rx ? LCN40PHY_RFOverrideVal0_trsw_rx_pu_ovr_val_MASK : 0));
		/* Enable overrides */
		phy_utils_or_phyreg(pi, LCN40PHY_RFOverride0,
			LCN40PHY_RFOverride0_trsw_tx_pu_ovr_MASK |
			LCN40PHY_RFOverride0_trsw_rx_pu_ovr_MASK);
	}
}

static void
wlc_lcn40phy_clear_trsw_override(phy_info_t *pi)
{
	if (LCN40REV_IS(pi->pubpi.phy_rev, 7)) {
		if (RADIOVER(pi->pubpi.radiover) == 0x4) {
			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				phy_utils_and_phyreg(pi, LCN40PHY_rfoverride4,
					(uint16)~(LCN40PHY_rfoverride4_trsw_tx_pwrup_ovr_MASK |
					LCN40PHY_rfoverride4_trsw_rx_pwrup_ovr_MASK));
		}
#ifdef BAND5G
		else {
				phy_utils_and_phyreg(pi, LCN40PHY_RFOverride0,
					(uint16)~(LCN40PHY_RFOverride0_trsw_tx_pu_ovr_MASK |
					LCN40PHY_RFOverride0_trsw_rx_pu_ovr_MASK));
			}
#endif /* BAND5G */
		} else {
			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				phy_utils_and_phyreg(pi, LCN40PHY_rfoverride4,
					(uint16)~(LCN40PHY_rfoverride4_trsw_tx_pwrup_ovr_MASK |
					LCN40PHY_rfoverride4_trsw_rx_pwrup_ovr_MASK));
			}
#ifdef BAND5G
			else {
				phy_utils_and_phyreg(pi, LCN40PHY_rfoverride4,
					(uint16)~(LCN40PHY_rfoverride4_trsw_tx_pwrup_ovr_MASK));
				phy_utils_and_phyreg(pi, LCN40PHY_rfoverride8,
					(uint16)
					~(LCN40PHY_rfoverride8_amode_trsw_rx_pwrup_ovr_MASK));
			}
#endif /* BAND5G */
		}
	} else if (CHIPID(pi->sh->chip) == BCM43143_CHIP_ID) {
		phy_utils_and_phyreg(pi, LCN40PHY_rfoverride4,
			(uint16)~(LCN40PHY_rfoverride4_trsw_tx_pwrup_ovr_MASK |
			LCN40PHY_rfoverride4_trsw_rx_pwrup_ovr_MASK));
	} else {
		/* Clear overrides */
		phy_utils_and_phyreg(pi, LCN40PHY_RFOverride0,
			(uint16)~(LCN40PHY_RFOverride0_trsw_tx_pu_ovr_MASK |
			LCN40PHY_RFOverride0_trsw_rx_pu_ovr_MASK));
	}
}

static uint32
wlc_lcn40phy_papd_rxGnCtrl(
	phy_info_t *pi,
	phy_papd_cal_type_t cal_type,
	bool frcRxGnCtrl,
	uint8 CurTxGain)
{
	/* Square of Loop Gain (inv) target for CW (reach as close to tgt, but be more than it) */
	/* dB Loop gain (inv) target for OFDM (reach as close to tgt,but be more than it) */
	int32 rxGnInit = 5;
	uint8  bsStep = 3; /* Binary search initial step size */
	uint8  bsDepth = 4; /* Binary search depth */
	int32  cwLpGn2_min = 131072, cwLpGn2_max = 262144;
	uint8  bsCnt;
	int16  lgI, lgQ;
	int32  cwLpGn2;
	uint8  num_symbols4lpgn;
	uint16 bbmult_init, bbmult_step;
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	phy_info_lcn40phy_t *pi_lcn40 = pi->u.pi_lcn40phy;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	/* frcRxGnCtrl conditional missing */
	for (bsCnt = 0; bsCnt < bsDepth; bsCnt++) {
		/* Running PAPD for Tx gain index:CurTxGain */
		/* Rx gain index : tia gain : rxGnInit */
		PHY_PAPD(("Running PAPD for Tx Gain Idx : %d ,Rx Gain Index %d\n",
			CurTxGain, rxGnInit));

		if (LCN40_LINPATH(pi_lcn40->papd_lin_path) == LCN40PHY_PAPDLIN_EPA) {
			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				wlc_lcn40phy_set_rx_gain_by_distribution(pi, 1, 0, 1,
					PAPD_LNA1_ROUT_2G, 0,
					(pi_lcn40->papdrx2g & 0xf), 0,
					(uint16)rxGnInit, 0, 0, 0, 0);
			}
#ifdef BAND5G
			else {
				wlc_lcn40phy_set_rx_gain_by_distribution(pi, 1, 0, 1, 0, 0,
					(pi_lcn40->papdrx5g & 0xf), 0,
					(uint16)rxGnInit, 0, 0, 0, 0);
			}
#endif /* BAND5G */
		} else {
			wlc_lcn40phy_set_rx_gain_by_distribution(pi, 1, 0, 0, 0, 0, 0, 0,
				(uint16)rxGnInit, 0, 0, 0, 0);
		}

		num_symbols4lpgn = 219;
		if (CHIPID(pi->sh->chip) == BCM43143_CHIP_ID) {
			if (CHSPEC_IS20(pi->radio_chanspec))
				bbmult_init = pi_lcn40->papd_bbmult_init_bw20;
			else
				bbmult_init = pi_lcn40->papd_bbmult_init_bw40;
			bbmult_step = PAPD_COARSE_BBMULT_STEP;
		} else {
			if (CHSPEC_IS20(pi->radio_chanspec)) {
				bbmult_init = pi_lcn40->papd_bbmult_init_bw20;
				bbmult_step = pi_lcn40->papd_bbmult_step_bw20;
			} else {
				bbmult_init = pi_lcn40->papd_bbmult_init_bw40;
				bbmult_step = pi_lcn40->papd_bbmult_step_bw40;
			}
		}
		wlc_lcn40phy_papd_cal_core(pi, 0,
			TRUE,
			0,
			0,
			0,
			num_symbols4lpgn,
			1,
			bbmult_init,
			bbmult_step,
			1,
			0,
			512,
			0);
		if (cal_type == PHY_PAPD_CAL_CW) {
			lgI = ((int16)
			       phy_utils_read_phyreg(pi, LCN40PHY_papd_loop_gain_cw_i)) << 4;
			lgI = lgI >> 4;
			lgQ = ((int16)
			       phy_utils_read_phyreg(pi, LCN40PHY_papd_loop_gain_cw_q)) << 4;
			lgQ = lgQ >> 4;
			cwLpGn2 = (lgI * lgI) + (lgQ * lgQ);

			PHY_PAPD(("loopgain %d lgI %d lgQ %d\n", cwLpGn2, lgI, lgQ));

			if (cwLpGn2 < cwLpGn2_min) {
				rxGnInit = rxGnInit - bsStep;
			} else if (cwLpGn2 >= cwLpGn2_max) {
				rxGnInit = rxGnInit + bsStep;
			} else {
				break;
			}
		}
		bsStep = bsStep - 1;
		if (rxGnInit > 9)
			rxGnInit = 9;
		if (rxGnInit < 0)
			rxGnInit = 0; /* out-of-range correction */
	}

	pi_lcn->lcnphy_papdRxGnIdx = rxGnInit;
	PHY_PAPD(("wl%d: %s Settled to rxGnInit: %d\n",
		pi->sh->unit, __FUNCTION__, rxGnInit));
	return rxGnInit;
}

static void
wlc_lcn40phy_GetpapdMaxMinIdxupdt(phy_info_t *pi,
	int16 *maxUpdtIdx,
	int16 *minUpdtIdx)
{
	uint16 papd_lut_index_updt_63_48, papd_lut_index_updt_47_32;
	uint16 papd_lut_index_updt_31_16, papd_lut_index_updt_15_0;
	int8 MaxIdx, MinIdx;
	uint8 MaxIdxUpdated, MinIdxUpdated;
	uint8 i;

	papd_lut_index_updt_63_48 =
	        phy_utils_read_phyreg(pi, LCN40PHY_papd_lut_index_updated_63_48);
	papd_lut_index_updt_47_32 =
	        phy_utils_read_phyreg(pi, LCN40PHY_papd_lut_index_updated_47_32);
	papd_lut_index_updt_31_16 =
	        phy_utils_read_phyreg(pi, LCN40PHY_papd_lut_index_updated_31_16);
	papd_lut_index_updt_15_0  =
	        phy_utils_read_phyreg(pi, LCN40PHY_papd_lut_index_updated_15_0);

	PHY_PAPD(("63_48  47_32  31_16  15_0\n"));
	PHY_PAPD((" %4x   %4x   %4x   %4x\n",
		papd_lut_index_updt_63_48, papd_lut_index_updt_47_32,
		papd_lut_index_updt_31_16, papd_lut_index_updt_15_0));

	MaxIdx = 63;
	MinIdx = 0;
	MinIdxUpdated = 0;
	MaxIdxUpdated = 0;

	for (i = 0; i < 16 && MinIdxUpdated == 0; i++) {
		if ((papd_lut_index_updt_15_0 & (1 << i)) == 0) {
			if (MinIdxUpdated == 0)
				MinIdx = MinIdx + 1;
		} else {
			MinIdxUpdated = 1;
		}
	}
	for (; i < 32 && MinIdxUpdated == 0; i++) {
		if ((papd_lut_index_updt_31_16 & (1 << (i - 16))) == 0) {
			if (MinIdxUpdated == 0)
				MinIdx = MinIdx + 1;
		} else {
			MinIdxUpdated = 1;
		}
	}
	for (; i < 48 && MinIdxUpdated == 0; i++) {
		if ((papd_lut_index_updt_47_32 & (1 << (i - 32))) == 0) {
			if (MinIdxUpdated == 0)
				MinIdx = MinIdx + 1;
		} else {
			MinIdxUpdated = 1;
		}
	}
	for (; i < 64 && MinIdxUpdated == 0; i++) {
		if ((papd_lut_index_updt_63_48 & (1 << (i - 48))) == 0) {
			if (MinIdxUpdated == 0)
				MinIdx = MinIdx + 1;
		} else {
			MinIdxUpdated = 1;
		}
	}

	/* loop for getting max index updated */
	for (i = 0; i < 16 && MaxIdxUpdated == 0; i++) {
		if ((papd_lut_index_updt_63_48 & (1 << (15 - i))) == 0) {
			if (MaxIdxUpdated == 0)
				MaxIdx = MaxIdx - 1;
		} else {
			MaxIdxUpdated = 1;
		}
	}
	for (; i < 32 && MaxIdxUpdated == 0; i++) {
		if ((papd_lut_index_updt_47_32 & (1 << (31 - i))) == 0) {
			if (MaxIdxUpdated == 0)
				MaxIdx = MaxIdx - 1;
		} else {
			MaxIdxUpdated = 1;
		}
	}
	for (; i < 48 && MaxIdxUpdated == 0; i++) {
		if ((papd_lut_index_updt_31_16 & (1 << (47 - i))) == 0) {
			if (MaxIdxUpdated == 0)
				MaxIdx = MaxIdx - 1;
		} else {
			MaxIdxUpdated = 1;
		}
	}
	for (; i < 64 && MaxIdxUpdated == 0; i++) {
		if ((papd_lut_index_updt_15_0 & (1 << (63 - i))) == 0) {
			if (MaxIdxUpdated == 0)
				MaxIdx = MaxIdx - 1;
		} else {
			MaxIdxUpdated = 1;
		}
	}
	*maxUpdtIdx = MaxIdx;
	*minUpdtIdx = MinIdx;
}

static void
wlc_lcn40phy_save_papd_calibration_results(phy_info_t *pi)
{
	phytbl_info_t tab;
#if defined(PHYCAL_CACHING)
	ch_calcache_t *ctx = wlc_phy_get_chanctx(pi, pi->radio_chanspec);
	lcnphy_calcache_t *cache;

	if (!ctx)
		return;

	cache = &ctx->u.lcnphy_cache;
#else
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
#endif // endif

	/* Save epsilon table */
	tab.tbl_len = PHY_PAPD_EPS_TBL_SIZE_LCNPHY;
	tab.tbl_id = LCN40PHY_TBL_ID_PAPDCOMPDELTATBL;
	tab.tbl_offset = 0;
	tab.tbl_width = 32;

#if defined(PHYCAL_CACHING)
	tab.tbl_ptr = cache->papd_eps_tbl;
	wlc_lcnphy_read_table(pi, &tab);

	cache->analog_gain_ref =
		phy_utils_read_phyreg(pi, LCN40PHY_papd_tx_analog_gain_ref);
	cache->lut_begin =
		phy_utils_read_phyreg(pi, LCN40PHY_papd_track_pa_lut_begin);
	cache->lut_step  =
		phy_utils_read_phyreg(pi, LCN40PHY_papd_track_pa_lut_step);
	cache->lut_end	 =
		phy_utils_read_phyreg(pi, LCN40PHY_papd_track_pa_lut_end);
	cache->rxcompdbm =
		phy_utils_read_phyreg(pi, LCN40PHY_papd_rx_gain_comp_dbm);
	cache->papdctrl  =
		phy_utils_read_phyreg(pi, LCN40PHY_papd_control);
	cache->sslpnCalibClkEnCtrl =
		phy_utils_read_phyreg(pi, LCN40PHY_sslpnCalibClkEnCtrl);
#else
	tab.tbl_ptr = pi_lcn->lcnphy_cal_results.papd_eps_tbl;
	wlc_lcnphy_read_table(pi, &tab);

	pi_lcn->lcnphy_cal_results.analog_gain_ref =
		phy_utils_read_phyreg(pi, LCN40PHY_papd_tx_analog_gain_ref);
	pi_lcn->lcnphy_cal_results.lut_begin =
		phy_utils_read_phyreg(pi, LCN40PHY_papd_track_pa_lut_begin);
	pi_lcn->lcnphy_cal_results.lut_step  =
		phy_utils_read_phyreg(pi, LCN40PHY_papd_track_pa_lut_step);
	pi_lcn->lcnphy_cal_results.lut_end   =
		phy_utils_read_phyreg(pi, LCN40PHY_papd_track_pa_lut_end);
	pi_lcn->lcnphy_cal_results.rxcompdbm =
		phy_utils_read_phyreg(pi, LCN40PHY_papd_rx_gain_comp_dbm);
	pi_lcn->lcnphy_cal_results.papdctrl  =
		phy_utils_read_phyreg(pi, LCN40PHY_papd_control);
	pi_lcn->lcnphy_cal_results.sslpnCalibClkEnCtrl =
		phy_utils_read_phyreg(pi, LCN40PHY_sslpnCalibClkEnCtrl);
#endif /* PHYCAL_CACHING */
}

static const uint32 papd_buf_20mhz[] = {
	0x7fc00, 0x5a569, 0x1ff, 0xa5d69, 0x80400, 0xa5e97, 0x201, 0x5a697}; /* 5MHz */
static const uint32 papd_buf_40mhz_5m[] = {
	0x7d000, 0x738bf, 0x58962, 0x2fdce, 0x001f4, 0xd05ce, 0xa7962, 0x8c8bf,
	0x83000, 0x8cb41, 0xa7a9e, 0xd0632, 0x0020c, 0x2fe32, 0x58a9e, 0x73b41}; /* 5MHz */
static const uint32 papd_buf_40mhz_15m[] = {
	0x7d000, 0x2fdce, 0xa7962, 0x8cb41, 0x0020c, 0x73b41, 0x58962, 0xd05ce,
	0x83000, 0xd0632, 0x58a9e, 0x738bf, 0x001f4, 0x8c8bf, 0xa7a9e, 0x2fe32}; /* 15MHz */

static void
wlc_lcn40phy_papd_cal_core(
	phy_info_t *pi,
	phy_papd_cal_type_t calType,
	bool rxGnCtrl,
	bool txGnCtrl,
	bool samplecapture,
	bool papd_dbg_mode,
	uint16 num_symbols,
	bool init_papd_lut,
	uint16 papd_bbmult_init,
	uint16 papd_bbmult_step,
	bool lpgn_only,
	bool papd_lpgn_ovr,
	uint16 LPGN_I,
	uint16 LPGN_Q)
{
	phytbl_info_t tab;
	uint32 papdcompdeltatbl_init_val;
	uint32 j;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	/* Reset PAPD Hw to reset register values */
	phy_utils_or_phyreg(pi, LCN40PHY_papd_control2, 0x1);
	phy_utils_and_phyreg(pi, LCN40PHY_papd_control2, ~0x1);
	PHY_REG_MOD(pi, LCN40PHY, papd_control2, papd_loop_gain_cw_ovr, papd_lpgn_ovr);

	/* set PAPD registers to configure the PAPD calibration */
	if (init_papd_lut != 0) {
		/* Load papd comp delta table */
		papdcompdeltatbl_init_val = 0x80000;
		tab.tbl_ptr = &papdcompdeltatbl_init_val; /* ptr to init var */
		tab.tbl_len = 1;        /* # values   */
		tab.tbl_id = LCN40PHY_TBL_ID_PAPDCOMPDELTATBL;         /* papdcompdeltatbl */
		tab.tbl_width = 32;     /* 32 bit wide */
		if (PAPD2LUT == 1)
			tab.tbl_offset = 64; /* tbl offset */
		else
			tab.tbl_offset = 0; /* tbl offset */
		for (j = 0; j < 64; j ++) {
			wlc_lcn40phy_write_table(pi, &tab);
			tab.tbl_offset++;
		}
	}

	/* set PAPD registers to configure PAPD calibration */
	wlc_lcn40phy_papd_cal_setup_cw(pi, lpgn_only);

	/* num_symbols is computed based on corr_norm */
	num_symbols = num_symbols * (PAPD_BLANKING_PROFILE + 1);

	/* override control params */
	phy_utils_write_phyreg(pi, LCN40PHY_papd_loop_gain_ovr_cw_i, LPGN_I);
	phy_utils_write_phyreg(pi, LCN40PHY_papd_loop_gain_ovr_cw_q, LPGN_Q);

	/* papd update */
	phy_utils_write_phyreg(pi, LCN40PHY_papd_track_num_symbols_count, num_symbols);

	/* spb parameters */
	phy_utils_write_phyreg(pi, LCN40PHY_papd_spb_num_vld_symbols_n_dly, 0x60);
	phy_utils_write_phyreg(pi, LCN40PHY_sampleLoopCount, (num_symbols+1)*20-1);
	phy_utils_write_phyreg(pi, LCN40PHY_papd_spb_rd_address, 0);

	/* load the spb */
	if (CHSPEC_IS40(pi->radio_chanspec)) {
		tab.tbl_len = 16;
		if (LCN40REV_LT(pi->pubpi.phy_rev, 4))
			tab.tbl_ptr = &papd_buf_40mhz_15m;
		else
			tab.tbl_ptr = &papd_buf_40mhz_5m;
	} else {
		tab.tbl_len = 8;
		tab.tbl_ptr = &papd_buf_20mhz;
	}
	tab.tbl_id = LCN40PHY_TBL_ID_SAMPLEPLAY;
	tab.tbl_offset = 0;
	tab.tbl_width = 32;
	wlc_lcn40phy_write_table(pi, &tab);

	/* 20MHz v.s. 40MHz block */
	if (CHSPEC_IS40(pi->radio_chanspec))
		phy_utils_write_phyreg(pi, LCN40PHY_sampleDepthCount, 0xf);
	else
		phy_utils_write_phyreg(pi, LCN40PHY_sampleDepthCount, 0x7);

	phy_utils_write_phyreg(pi, LCN40PHY_papd_bbmult_init, papd_bbmult_init);
	phy_utils_write_phyreg(pi, LCN40PHY_papd_bbmult_step, papd_bbmult_step);

	/* BBMULT parameters */
	phy_utils_write_phyreg(pi, LCN40PHY_papd_bbmult_num_symbols, 1-1);

	/* set bbmult to 0 to remove DC current spike after cal */
	wlc_lcn40phy_set_bbmult(pi, 0);

	/* clean up the forward path to avoid large transient due to loopgain-only enabling; */
	/* do this here due to findings from 43143 and 43342 */
	PHY_REG_MOD(pi, LCN40PHY, sslpnCtrl0, txSoftReset, 1);
	PHY_REG_MOD(pi, LCN40PHY, sslpnCtrl0, txSoftReset, 0);

	/* To avoid RxFIFO Overflow which causes 45 degree phase jump in eps LUT */
	/* This causes 43142 to show Rx throughput dip so skip it for now */
	if (LCN40REV_GE(pi->pubpi.phy_rev, 4))
		PHY_REG_MOD(pi, LCN40PHY, RxFeStatus, rx_fifo_reset, 1);

	/* Run PAPD HW Cal */
	/* pdbypass 0, gain0mode 1 */
	phy_utils_write_phyreg(pi, LCN40PHY_papd_rx_sm_iqmm_gain_comp, 0x00);
	phy_utils_write_phyreg(pi, LCN40PHY_papd_control, 0xb8a1);

	/* Wait for completion, around 1s */
	SPINWAIT(phy_utils_read_phyreg(pi, LCN40PHY_papd_control) &
	         LCN40PHY_papd_control_papd_cal_run_MASK,
	         1 * 1000 * 1000);
}

static void
wlc_lcn40phy_papd_cal_setup_cw(
	phy_info_t *pi, bool lpgn_only)
{
	phy_info_lcn40phy_t *pi_lcn40 = pi->u.pi_lcn40phy;
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	/* Set samples/cycle/4 for q delay */
	phy_utils_write_phyreg(pi, LCN40PHY_papd_variable_delay, 3);
	/* Set LUT begin gain, step gain, and size */
	phy_utils_write_phyreg(pi, LCN40PHY_papd_track_pa_lut_begin, pi_lcn40->papd_lut_begin);
	phy_utils_write_phyreg(pi, LCN40PHY_papd_rx_gain_comp_dbm, 0);
	phy_utils_write_phyreg(pi, LCN40PHY_papd_track_pa_lut_step, pi_lcn40->papd_lut_step);

	PHY_REG_LIST_START
		PHY_REG_WRITE_ENTRY(LCN40PHY, papd_track_pa_lut_end, 0x3f)
		/* set papd constants */
		PHY_REG_WRITE_ENTRY(LCN40PHY, papd_dbm_offset, 0x681)
		/* Dc estimation samples */
		PHY_REG_WRITE_ENTRY(LCN40PHY, papd_ofdm_dc_est, 0x49)
	PHY_REG_LIST_EXECUTE(pi);

	phy_utils_write_phyreg(pi, LCN40PHY_papd_blanking_control,
		(pi_lcn40->papd_stop_after_last_update << 12 | PAPD_BLANKING_PROFILE << 9 |
		PAPD_BLANKING_THRESHOLD));

	if (LCN40REV_IS(pi->pubpi.phy_rev, 4) || LCN40REV_IS(pi->pubpi.phy_rev, 5) ||
		LCN40REV_IS(pi->pubpi.phy_rev, 7)) {
		PHY_REG_LIST_START
			PHY_REG_WRITE_ENTRY(LCN40PHY, papd_cw_corr_norm, 1)
			PHY_REG_WRITE_ENTRY(LCN40PHY, papd_spb2papdin_dly, 38)
			PHY_REG_WRITE_ENTRY(LCN40PHY, papd_num_skip_count, 76)
			PHY_REG_WRITE_ENTRY(LCN40PHY, papd_num_samples_count, 127)
			PHY_REG_WRITE_ENTRY(LCN40PHY, papd_sync_count, 239)
		PHY_REG_LIST_EXECUTE(pi);
	} else {
		PHY_REG_LIST_START
			PHY_REG_WRITE_ENTRY(LCN40PHY, papd_cw_corr_norm, 0)
			PHY_REG_WRITE_ENTRY(LCN40PHY, papd_spb2papdin_dly, 35)
			PHY_REG_WRITE_ENTRY(LCN40PHY, papd_num_skip_count, 39)
			PHY_REG_WRITE_ENTRY(LCN40PHY, papd_num_samples_count, 255)
			PHY_REG_WRITE_ENTRY(LCN40PHY, papd_sync_count, 319)
		PHY_REG_LIST_EXECUTE(pi);
	}

	phy_utils_write_phyreg(pi, LCN40PHY_papd_switch_lut, PAPD2LUT);
	PHY_REG_WRITE(pi, LCN40PHY, papd_pa_off_control_1, 0);
	PHY_REG_WRITE(pi, LCN40PHY, papd_only_loop_gain, lpgn_only);
	PHY_REG_LIST_START
		PHY_REG_WRITE_ENTRY(LCN40PHY, papd_pa_off_control_2, 0)

		PHY_REG_WRITE_ENTRY(LCN40PHY, smoothenLut_max_thr, 0x7ff)
		PHY_REG_WRITE_ENTRY(LCN40PHY, papd_dcest_i_ovr, 0x0000)
		PHY_REG_WRITE_ENTRY(LCN40PHY, papd_dcest_q_ovr, 0x0000)
	PHY_REG_LIST_EXECUTE(pi);
}

/* force epa off */
static void
wlc_lcn40phy_epa_pd(phy_info_t *pi, bool disable)
{
	uint16 swctrlovr_mask = 0;
	uint16 swctrlovr_val = 0;

#ifdef WLC_SW_DIVERSITY
	phy_info_lcn40phy_t *pi_lcn40 = pi->u.pi_lcn40phy;
	swctrlovr_mask = pi_lcn40->swdiv_swctrl_mask;
	swctrlovr_val = PHY_REG_READ(pi, LCN40PHY, swctrlOvr_val, swCtrl_ovr_val);
	swctrlovr_val = swctrlovr_val & swctrlovr_mask;
#endif // endif

	if (!disable) {
		PHY_REG_MOD(pi, LCN40PHY, swctrlOvr, swCtrl_ovr, swctrlovr_mask);
	} else {
		PHY_REG_MOD(pi, LCN40PHY, swctrlOvr_val, swCtrl_ovr_val, (0 | swctrlovr_val));
		PHY_REG_MOD(pi, LCN40PHY, swctrlOvr, swCtrl_ovr, (0xff | swctrlovr_mask));
	}
}

/*
* Get Rx IQ Imbalance Estimate from modem
*/
static bool
wlc_lcn40phy_rx_iq_est(phy_info_t *pi,
	uint16 num_samps,
	uint8 wait_time,
	uint8 wait_for_crs,
	phy_iq_est_t *iq_est,
	uint16 timeout_ms)
{
	int wait_count = 0;
	bool result = TRUE;
	phy_info_lcn40phy_t *pi_lcn40 = pi->u.pi_lcn40phy;
	uint16 lcn40phyregs_shm_addr = 2 * wlapi_bmac_read_shm(pi->sh->physhim, M_LCN40PHYREGS_PTR);

	/* Turn on clk to Rx IQ */
	PHY_REG_MOD(pi, LCN40PHY, sslpnCalibClkEnCtrl, iqEstClkEn, 1);
	OSL_DELAY(1);

	/* Disable trigger(may have been enabled by noise cal) and abort measurement */
	PHY_REG_MOD(pi, LCN40PHY, IQEnableWaitTimeAddress, abort, 1);

	if (pi_lcn40->rssi_iqest_en)
		wlapi_bmac_write_shm(pi->sh->physhim,
			lcn40phyregs_shm_addr + M_RSSI_IQEST_PENDING,
			0);

	if (pi_lcn40->noise_iqest_en)
		wlapi_bmac_write_shm(pi->sh->physhim,
			lcn40phyregs_shm_addr + M_NOISE_IQEST_PENDING,
			0);

	PHY_REG_MOD(pi, LCN40PHY, IQTrigger0Address, trig_en, 0);
	PHY_REG_MOD(pi, LCN40PHY, IQEnableWaitTimeAddress, abort, 0);

	/* Force OFDM receiver on */
	PHY_REG_MOD(pi, LCN40PHY, crsgainCtrl, APHYGatingEnable, 0);
	PHY_REG_MOD(pi, LCN40PHY, IQNumSampsAddress, numSamps, num_samps);
	PHY_REG_MOD(pi, LCN40PHY, IQEnableWaitTimeAddress, waittimevalue,
		(uint16)wait_time);

	if (!wait_for_crs)
		PHY_REG_MOD(pi, LCN40PHY, IQEnableWaitTimeAddress, iqmode, 0);

	PHY_REG_MOD(pi, LCN40PHY, IQEnableWaitTimeAddress, iqstart, 1);

	/* Wait for IQ estimation to complete */
	while (phy_utils_read_phyreg(pi, LCN40PHY_IQEnableWaitTimeAddress) &
		LCN40PHY_IQEnableWaitTimeAddress_iqstart_MASK) {
		/* Check for timeout */
		if (wait_count > (10 * timeout_ms)) { /* 500 ms */
			PHY_ERROR(("wl%d: %s: IQ estimation failed to complete\n",
				pi->sh->unit, __FUNCTION__));
			result = FALSE;
			goto cleanup;
		}
		OSL_DELAY(100);
		wait_count++;
	}

	/* Save results */
	iq_est->iq_prod = ((uint32)phy_utils_read_phyreg(pi, LCN40PHY_IQAccHiAddress) << 16) |
		(uint32)phy_utils_read_phyreg(pi, LCN40PHY_IQAccLoAddress);
	iq_est->i_pwr = ((uint32)phy_utils_read_phyreg(pi, LCN40PHY_IQIPWRAccHiAddress) << 16) |
		(uint32)phy_utils_read_phyreg(pi, LCN40PHY_IQIPWRAccLoAddress);
	iq_est->q_pwr = ((uint32)phy_utils_read_phyreg(pi, LCN40PHY_IQQPWRAccHiAddress) << 16) |
		(uint32)phy_utils_read_phyreg(pi, LCN40PHY_IQQPWRAccLoAddress);
	PHY_TMP(("wl%d: %s: IQ estimation completed in %d us,"
		"i_pwr: %d, q_pwr: %d, iq_prod: %d\n",
		pi->sh->unit, __FUNCTION__,
		wait_count * 100, iq_est->i_pwr, iq_est->q_pwr, iq_est->iq_prod));
cleanup:
	PHY_REG_MOD(pi, LCN40PHY, crsgainCtrl, APHYGatingEnable, 1);
	PHY_REG_MOD(pi, LCN40PHY, sslpnCalibClkEnCtrl, iqEstClkEn, 0);
	return result;
}

int16
wlc_lcn40phy_iqest_rssi_tempcorr(phy_info_t *pi, bool mode, uint16 board_atten)
{
		int16 curr_temp, ret;
		phy_info_lcn40phy_t *pi_lcn40 = pi->u.pi_lcn40phy;
		int16 temp_coeff = 0;
		int16 gain_err_temp_adj;
		uint8 channel = (pi->radio_chanspec & 0xff);

		curr_temp = wlc_lcn40phy_tempsense(pi, mode);

		/* temp_coeff in nvram is multiplied by 2^10 */

	switch (wlc_phy_chanspec_bandrange_get(pi, pi->radio_chanspec)) {
		case WL_CHAN_FREQ_RANGE_2G:
			/* dp2/rxsaw/LNA temp adj */
			if (board_atten)
				temp_coeff += pi_lcn40->rssi_eLNAbyp_slope_2g;
			if (channel <= 4) {
				temp_coeff += pi_lcn40->rssi_rxsaw_slope_2g[0];
				temp_coeff += pi_lcn40->rssi_dp2_slope_2g[0];
				if (!board_atten)
					temp_coeff += pi_lcn40->rssi_eLNAon_slope_2g[0];
			} else if (channel <= 9) {
				temp_coeff += pi_lcn40->rssi_rxsaw_slope_2g[1];
				temp_coeff += pi_lcn40->rssi_dp2_slope_2g[1];
				if (!board_atten)
					temp_coeff += pi_lcn40->rssi_eLNAon_slope_2g[1];
			} else {
				temp_coeff += pi_lcn40->rssi_rxsaw_slope_2g[2];
				temp_coeff += pi_lcn40->rssi_dp2_slope_2g[2];
				if (!board_atten)
					temp_coeff += pi_lcn40->rssi_eLNAon_slope_2g[2];
			}
			break;
	#ifdef BAND5G
		case WL_CHAN_FREQ_RANGE_5GL:
			/* 5 GHz low */
			/* LNA temp adj */
			if (board_atten)
				temp_coeff += pi_lcn40->rssi_eLNAbyp_slope_5g;
			else
				temp_coeff += pi_lcn40->rssi_eLNAon_slope_5gl;
			break;

		case WL_CHAN_FREQ_RANGE_5GM:
			/* 5 GHz middle */
			/* LNA temp adj */
			if (board_atten)
				temp_coeff += pi_lcn40->rssi_eLNAbyp_slope_5g;
			else {
				if (channel < 120)
					temp_coeff += pi_lcn40->rssi_eLNAon_slope_5gml;
				else
					temp_coeff += pi_lcn40->rssi_eLNAon_slope_5gmu;
			}
			break;

		case WL_CHAN_FREQ_RANGE_5GH:
			/* 5 GHz high */
			/* LNA temp adj */
			if (board_atten)
				temp_coeff += pi_lcn40->rssi_eLNAbyp_slope_5g;
			else
				temp_coeff += pi_lcn40->rssi_eLNAon_slope_5gh;
			break;
	#endif /* BAND5G */
		default:
			temp_coeff = 0;
			break;
	}

	/* gain_err_temp_adj is on 0.25dB steps */
	gain_err_temp_adj =
		((curr_temp - pi_lcn40->gain_cal_temp) * temp_coeff);
	ret = ABS(gain_err_temp_adj) >> 8;
	if (gain_err_temp_adj < 0)
		ret = -ret;

	return ret;
}

int16
wlc_lcn40phy_rssi_tempcorr(phy_info_t *pi, bool mode)
{
	int16 curr_temp, ret;
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	int16 temp_coeff;
	int16 gain_err_temp_adj;

	curr_temp = wlc_lcn40phy_tempsense(pi, mode);

	/* temp_coeff in nvram is multiplied by 2^10 */

	switch (wlc_phy_chanspec_bandrange_get(pi, pi->radio_chanspec)) {
		case WL_CHAN_FREQ_RANGE_2G:
			temp_coeff = pi_lcn->rxgain_tempadj_2g;
			break;
	#ifdef BAND5G
		case WL_CHAN_FREQ_RANGE_5GL:
			/* 5 GHz low */
			temp_coeff = pi_lcn->rxgain_tempadj_5gl;
			break;

		case WL_CHAN_FREQ_RANGE_5GM:
			/* 5 GHz middle */
			temp_coeff = pi_lcn->rxgain_tempadj_5gm;
			break;

		case WL_CHAN_FREQ_RANGE_5GH:
			/* 5 GHz high */
			temp_coeff = pi_lcn->rxgain_tempadj_5gh;
			break;
	#endif /* BAND5G */
		default:
			temp_coeff = 0;
			break;
	}

	/* gain_err_temp_adj is on 0.25dB steps */
	gain_err_temp_adj =
		((curr_temp - LCN40_RSSI_NOMINAL_TEMP) * temp_coeff);
	ret = ABS(gain_err_temp_adj) >> 8;
	if (gain_err_temp_adj < 0)
		ret = -ret;

	return ret;
}

int16
wlc_lcn40phy_get_rxpath_gain_by_index(phy_info_t *pi, uint8 gain_index, uint16 board_atten)
{
	int16 rxpath_gain;
	bool aci_gaintbl = 0;
	phy_info_lcn40phy_t *pi_lcn40 = pi->u.pi_lcn40phy;
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	int8* rssi_gain_delta;
	uint8 channel = CHSPEC_CHANNEL(pi->radio_chanspec);

	if (gain_index >= 38) {
		gain_index = gain_index - 38;
		aci_gaintbl = 1;
	}

	rxpath_gain = (gain_index * 3) - 18;
	rxpath_gain = rxpath_gain << 2;

	switch (wlc_phy_chanspec_bandrange_get(pi, pi->radio_chanspec)) {
		case WL_CHAN_FREQ_RANGE_2G:
			rxpath_gain += lcn40phy_gain_index_offset_for_rx_gain_2g[gain_index];
			if (channel <= 11)
				rssi_gain_delta = pi_lcn40->rssi_gain_delta_2g;
			else if (channel == 12)
				rssi_gain_delta = pi_lcn40->rssi_gain_delta_2gh;
			else
				rssi_gain_delta = pi_lcn40->rssi_gain_delta_2ghh;

			if (board_atten) {
				rxpath_gain = rxpath_gain - ((pi_lcn->lcnphy_tr_isolation_mid)*12)
				- rssi_gain_delta[8];
			} else {
				uint8 gain_index_map_aci[3] = {35, 34, 17};
				uint8 gain_index_map[3] = {30, 28, 26}, *map;

				if (aci_gaintbl)
					map = gain_index_map_aci;
				else
					map = gain_index_map;

				if (gain_index >= map[0])
				  rxpath_gain -= rssi_gain_delta[0+(4*aci_gaintbl)];
				else if (gain_index >= map[1])
				  rxpath_gain -= rssi_gain_delta[1+(4*aci_gaintbl)];
				else if (gain_index  >= map[2])
				  rxpath_gain -= rssi_gain_delta[2+(4*aci_gaintbl)];
				else
				  rxpath_gain -= rssi_gain_delta[3+(4*aci_gaintbl)];
			}
			break;
	#ifdef BAND5G
		case WL_CHAN_FREQ_RANGE_5GL:
			/* 5 GHz low */
			rxpath_gain += lcn40phy_gain_index_offset_for_rx_gain_5gl[gain_index];
			if (board_atten) {
			  rxpath_gain = rxpath_gain - ((pi_lcn->triso5g[0])*12) -
			  (pi_lcn40->rssi_gain_delta_5gl[3] - pi_lcn40->rssi_gain_delta_5gl[2]);
			}
			if (gain_index >= 20)
				rxpath_gain -= pi_lcn40->rssi_gain_delta_5gl[0];
			else if (gain_index == 19)
				rxpath_gain -= pi_lcn40->rssi_gain_delta_5gl[1];
			else
				rxpath_gain -= pi_lcn40->rssi_gain_delta_5gl[2];
			break;

		case WL_CHAN_FREQ_RANGE_5GM:
			/* 5 GHz middle */
		{
			int8 *rssi_gain_delta_5gm;
			if ((pi->radio_chanspec & 0xff) < 120) {
				rssi_gain_delta_5gm = pi_lcn40->rssi_gain_delta_5gml;
				rxpath_gain +=
					lcn40phy_gain_index_offset_for_rx_gain_5gml[gain_index];
			} else {
				rssi_gain_delta_5gm = pi_lcn40->rssi_gain_delta_5gmu;
				rxpath_gain +=
					lcn40phy_gain_index_offset_for_rx_gain_5gmu[gain_index];
			}
			if (board_atten) {
			  rxpath_gain = rxpath_gain - ((pi_lcn->triso5g[0])*12) -
			  (rssi_gain_delta_5gm[3] - rssi_gain_delta_5gm[2]);
			}
			if (gain_index >= 20)
				rxpath_gain -= rssi_gain_delta_5gm[0];
			else if (gain_index == 19)
				rxpath_gain -= rssi_gain_delta_5gm[1];
			else
				rxpath_gain -= rssi_gain_delta_5gm[2];
			break;
		}
		case WL_CHAN_FREQ_RANGE_5GH:
			/* 5 GHz high */
			rxpath_gain += lcn40phy_gain_index_offset_for_rx_gain_5gh[gain_index];
			if (board_atten) {
			  rxpath_gain = rxpath_gain - ((pi_lcn->triso5g[0])*12) -
			  (pi_lcn40->rssi_gain_delta_5gh[3] - pi_lcn40->rssi_gain_delta_5gh[2]);
			 }
			if (gain_index >= 20)
				rxpath_gain -= pi_lcn40->rssi_gain_delta_5gh[0];
			else if (gain_index == 19)
				rxpath_gain -= pi_lcn40->rssi_gain_delta_5gh[1];
			else
			  rxpath_gain -= pi_lcn40->rssi_gain_delta_5gh[2];
			break;
	#endif /* BAND5G */
		default:
			rxpath_gain = rxpath_gain;
			break;
	}

	return rxpath_gain;
}

#if defined(WLTEST) || defined(SAMPLE_COLLECT)
static void
wlc_lcn40phy_set_gain_by_index(phy_info_t *pi, uint8 gain_index)
{
	phytbl_info_t tab;
	uint32 data;
	uint16 biq2, biq1, digi;
	uint16 trsw, elna, tia, lna2, lna1;
	uint16 digi_offset, slna_byp, slna_rout, lna2_rout;
	uint16 gain_idx_bt, gain_idx;
	bool suspend;
	bool aci_gaintbl = 0;

	suspend = !(R_REG(pi->sh->osh, &pi->regs->maccontrol) & MCTL_EN_MAC);
	if (!suspend)
		wlapi_suspend_mac_and_wait(pi->sh->physhim);

	wlc_lcn40phy_agc_reset(pi);

	if (gain_index >= 38) {
		gain_index = gain_index - 38;
		aci_gaintbl = 1;
	}
	tab.tbl_id = LCN40PHY_TBL_ID_GAIN_IDX;
	tab.tbl_width = 32;
	tab.tbl_ptr = &data;
	tab.tbl_len = 1;
	tab.tbl_offset = gain_index;
	if (aci_gaintbl)
		tab.tbl_offset = gain_index + 38;
	wlc_lcnphy_read_table(pi, &tab);

	gain_idx_bt = data & 0x7f;
	gain_idx = (data >> 7) & 0x7f;
	lna1 = ((data >> 14) & 0x7);
	slna_rout = ((data >> 17) & 0xf);
	slna_byp = ((data >> 21) & 0x1);
	elna = (data >> 22) & 0x1;
	trsw = (data >> 23) & 0x1;

	BCM_REFERENCE(gain_idx_bt);

	tab.tbl_id = LCN40PHY_TBL_ID_GAIN_TBL;
	tab.tbl_width = 32;
	tab.tbl_ptr = &data;
	tab.tbl_len = 1;
	tab.tbl_offset = gain_idx;
	wlc_lcnphy_read_table(pi, &tab);

	lna2 = (data & 0x7);
	lna2_rout = ((data >> 3) & 0xf);
	tia =  ((data >> 7) & 0xf);
	biq1 = ((data >> 11) & 0xf);
	biq2 = ((data >> 15) & 0xf);
	digi = ((data >> 19) & 0xf);
	digi_offset = ((data >> 23) & 0x1);

	PHY_INFORM(("trsw = %d, elna = %d, "
		"slna_byp = %d, slna_rout= %d, lna1= %d, lna2= %d, lna2_rout=%d, tia=%d, "
		"biq1=%d, biq2=%d, digi=%d, digi_offset=%d\n",
		trsw, elna,
		slna_byp, slna_rout, lna1, lna2, lna2_rout, tia,
		biq1, biq2, digi, digi_offset));

	wlc_lcn40phy_set_rx_gain_by_distribution(pi, trsw, elna,
		slna_byp, slna_rout, lna1, lna2, lna2_rout, tia,
		biq1, biq2, digi, digi_offset);

	wlc_lcn40phy_rx_gain_override_enable(pi, TRUE);
	wlc_lcn40phy_agc_fsm_toggle(pi);

	/* Power cycle adc */
	PHY_REG_LIST_START
		PHY_REG_MOD_ENTRY(LCN40PHY, AfeCtrlOvr1, adc_pu_ovr, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, AfeCtrlOvr1Val, adc_pu_ovr_val, 0)
	PHY_REG_LIST_EXECUTE(pi);
	OSL_DELAY(10);
	PHY_REG_LIST_START
		PHY_REG_MOD_ENTRY(LCN40PHY, AfeCtrlOvr1, adc_pu_ovr, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, AfeCtrlOvr1Val, adc_pu_ovr_val, 31)
	PHY_REG_LIST_EXECUTE(pi);
	OSL_DELAY(10);

	if (!suspend)
		wlapi_enable_mac(pi->sh->physhim);
}
#endif /* #if defined(WLTEST) || defined(SAMPLE_COLLECT) */

#if defined(WLTEST)
void
wlc_phy_get_rxgainerr_lcn40phy(phy_info_t *pi, int16 *gainerr)
{
	/* Returns rxgain error (read from srom) for current channel */
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);

	switch (wlc_phy_chanspec_bandrange_get(pi, pi->radio_chanspec)) {
		case WL_CHAN_FREQ_RANGE_2G:
			*gainerr = pi_lcn->srom_rxgainerr_2g;
			break;
	#ifdef BAND5G
		case WL_CHAN_FREQ_RANGE_5GL:
			/* 5 GHz low */
			*gainerr = pi_lcn->srom_rxgainerr_5gl;
			break;

		case WL_CHAN_FREQ_RANGE_5GM:
			/* 5 GHz middle */
			*gainerr = pi_lcn->srom_rxgainerr_5gm;
			break;

		case WL_CHAN_FREQ_RANGE_5GH:
			/* 5 GHz high */
			*gainerr = pi_lcn->srom_rxgainerr_5gh;
			break;
	#endif /* BAND5G */
		default:
			*gainerr = 0;
			break;
	}
}

void
wlc_phy_get_SROMnoiselvl_lcn40phy(phy_info_t *pi, int8 *noiselvl)
{
	/* Returns noise level (read from srom) for current channel */
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);

	switch (wlc_phy_chanspec_bandrange_get(pi, pi->radio_chanspec)) {
		case WL_CHAN_FREQ_RANGE_2G:
			*noiselvl = pi_lcn->srom_noiselvl_2g;
			break;
	#ifdef BAND5G
		case WL_CHAN_FREQ_RANGE_5GL:
			/* 5 GHz low */
			*noiselvl = pi_lcn->srom_noiselvl_5gl;
			break;

		case WL_CHAN_FREQ_RANGE_5GM:
			/* 5 GHz middle */
			*noiselvl = pi_lcn->srom_noiselvl_5gm;
			break;

		case WL_CHAN_FREQ_RANGE_5GH:
			/* 5 GHz high */
			*noiselvl = pi_lcn->srom_noiselvl_5gh;
			break;
	#endif /* BAND5G */
		default:
			*noiselvl = 0;
			break;
	}
}

void
wlc_phy_get_noiseoffset_lcn40phy(phy_info_t *pi, int16 *noiseoff)
{
	/* Returns noise level (read from srom) for current channel */
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	phy_info_lcn40phy_t *pi_lcn40 = pi->u.pi_lcn40phy;

	if (pi_lcn->rxpath_index < 76) {
		*noiseoff = pi_lcn40->rssi_iqest_iov_gain_adj;
		return;
	}

	switch (wlc_phy_chanspec_bandrange_get(pi, pi->radio_chanspec)) {
		case WL_CHAN_FREQ_RANGE_2G:
			*noiseoff = pi_lcn->noise_offset_2g;
			break;
	#ifdef BAND5G
		case WL_CHAN_FREQ_RANGE_5GL:
			/* 5 GHz low */
			*noiseoff = pi_lcn->noise_offset_5gl;
			break;

		case WL_CHAN_FREQ_RANGE_5GM:
			/* 5 GHz middle */
			*noiseoff = pi_lcn->noise_offset_5gm;
			break;

		case WL_CHAN_FREQ_RANGE_5GH:
			/* 5 GHz high */
			*noiseoff = pi_lcn->noise_offset_5gh;
			break;
	#endif /* BAND5G */
		default:
			*noiseoff = 0;
			break;
	}
}

static int8
wlc_lcn40phy_calc_rx_gain(phy_info_t *pi)
{
	uint8 trsw;
	uint8 elna;
	uint8 biq2;
	uint8 biq1;
	uint8 tia;
	uint8 lna2;
	uint8 lna2_rout;
	uint8 slna;
	uint8 slna_byp;
	uint8 slna_rout;
	uint8 digi;

	int8 rxgain = 0;
	int8 tr_gain;
	int8 elna_gain;
	int8 biq2_gain;
	int8 biq1_gain;
	int8 tia_gain;
	int8 lna2_gain;
	int8 slna_gain;
	int8 digi_gain;

	int8 slna_gain_table_2g[] = {-5, 1, 7, 13, 19, 25};
	int8 slna_rout_gain_table_2g[] = {0, -1, -2, -3, -5, -7, -9, -11, -13, -16, -19, -21};
	int8 slna_byp_gain_2g = 10;
#ifdef BAND5G
	int8 slna_gain_table_5g[] = {6, 14, 18, 22, 0, 0};
	int8 slna_rout_gain_table_5g[] = {-6, -4, -2, -1, 0};
	int8 slna_byp_gain_5g = 10;
#endif /* BAND5G */
	int8 tr_gain_table[] = {-3, -28};
	int8 elna_gain_table = 12;
	int8 lna2_rout_gain_2g_5g = 0;
	int8 tia_gain_table_2g_5g[] = {0, 3, 6, 9, 12, 15, 18, 21, 24, 27};
	int8 lna2_gain_table_2g_5g[] = {-5, -2, 1, 4, 7, 10, 13};

	elna = PHY_REG_READ(pi, LCN40PHY, rfoverride2val, ext_lna_gain_ovr_val);
	slna = PHY_REG_READ(pi, LCN40PHY, rfoverride2val, slna_gain_ctrl_ovr_val);
	slna_byp = PHY_REG_READ(pi, LCN40PHY, rfoverride2val, slna_byp_ovr_val);
	slna_rout = PHY_REG_READ(pi, LCN40PHY, rfoverride3_val, slna_rout_ctrl_ovr_val);
	if (LCN40REV_IS(pi->pubpi.phy_rev, 7)) {
		if (RADIOVER(pi->pubpi.radiover) == 0x4) {
#ifdef BAND5G
			if (CHSPEC_IS5G(pi->radio_chanspec))
				trsw = !PHY_REG_READ(pi, LCN40PHY, RFOverrideVal0,
					trsw_rx_pu_ovr_val);
			else
#endif /* BAND5G */
				trsw = !PHY_REG_READ(pi, LCN40PHY, rfoverride4val,
					trsw_rx_pwrup_ovr_val);
		} else {
#ifdef BAND5G
			if (CHSPEC_IS5G(pi->radio_chanspec))
				trsw = !PHY_REG_READ(pi, LCN40PHY, rfoverride8val,
					amode_trsw_rx_pwrup_ovr_val);
			else
#endif /* BAND5G */
				trsw = !PHY_REG_READ(pi, LCN40PHY, rfoverride4val,
					trsw_rx_pwrup_ovr_val);
		}
	} else {
		trsw = !PHY_REG_READ(pi, LCN40PHY, RFOverrideVal0, trsw_rx_pu_ovr_val);
	}
	lna2 = PHY_REG_READ(pi, LCN40PHY, rfoverride5val, rxrf_lna2_gain_ovr_val);
	lna2_rout = PHY_REG_READ(pi, LCN40PHY, rfoverride5val, rxrf_lna2_rout_ovr_val);
	BCM_REFERENCE(lna2_rout);
	tia = PHY_REG_READ(pi, LCN40PHY, rfoverride5val, rxrf_tia_gain_ovr_val);

	biq1 = PHY_REG_READ(pi, LCN40PHY, rfoverride6val, lpf_bq1_gain_ovr_val);
	biq2 = PHY_REG_READ(pi, LCN40PHY, rfoverride6val, lpf_bq2_gain_ovr_val);

	digi = PHY_REG_READ(pi, LCN40PHY, radioCtrl, digi_gain_ovr_val);

	PHY_TMP(("wl%d: %s: tr = %d, elna = %d, biq2= %d, biq1 = %d, tia = %d, "
		"lna2 = %d, lna2_r = %d, slna = %d, slna_r = %d, digi = %d\n",
		pi->sh->unit, __FUNCTION__,
		trsw, elna, biq2, biq1, tia, lna2, lna2_rout, slna, slna_rout, digi));

	biq2_gain = biq2 * 3;
	biq1_gain = biq1 * 3;
	digi_gain = digi * 3;
	tr_gain = tr_gain_table[trsw];
	if (elna)
		elna_gain = elna_gain_table;
	else
		elna_gain = 0;
	tia_gain = tia_gain_table_2g_5g[tia];
	lna2_gain = lna2_gain_table_2g_5g[lna2] + lna2_rout_gain_2g_5g;

#ifdef BAND5G
	if (CHSPEC_IS5G(pi->radio_chanspec)) {
		if (slna_byp)
			slna_gain = slna_byp_gain_5g;
		else
			slna_gain = slna_gain_table_5g[slna] + slna_rout_gain_table_5g[slna_rout];

	} else
#endif /* BAND5G */
	{
		if (slna_byp)
			slna_gain = slna_byp_gain_2g;
		else
			slna_gain = slna_gain_table_2g[slna] + slna_rout_gain_table_2g[slna_rout];
	}

	rxgain = tr_gain +  elna_gain + biq2_gain + biq1_gain
		+ tia_gain + lna2_gain + slna_gain + digi_gain;

	PHY_TMP(("wl%d: %s: tr = %d, elna = %d, biq2= %d,"
		"biq1 = %d, tia = %d, lna2 = %d, slna = %d, digi = %d\n",
		pi->sh->unit, __FUNCTION__,
		tr_gain, elna_gain, biq2_gain, biq1_gain,
		tia_gain, lna2_gain, slna_gain, digi_gain));

	return rxgain;
}

static void
wlc_lcn40phy_set_lna_freq_comp(phy_info_t *pi)
{
}

void
wlc_lcn40phy_get_lna_freq_correction(phy_info_t *pi, int8 *freq_offset_fact)
{
		*freq_offset_fact = 0;
}

int16
wlc_lcn40phy_rxgaincal_tempadj(phy_info_t *pi)
{
		int16 curr_temp;
		phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
		int16 temp_coeff;
		int16 gain_err_temp_adj;

		curr_temp = wlc_lcn40phy_tempsense(pi, 1);

		/* temp_coeff in nvram is multiplied by 2^10 */

	switch (wlc_phy_chanspec_bandrange_get(pi, pi->radio_chanspec)) {
		case WL_CHAN_FREQ_RANGE_2G:
			temp_coeff = pi_lcn->rxgain_tempadj_2g;
			break;
	#ifdef BAND5G
		case WL_CHAN_FREQ_RANGE_5GL:
			/* 5 GHz low */
			temp_coeff = pi_lcn->rxgain_tempadj_5gl;
			break;

		case WL_CHAN_FREQ_RANGE_5GM:
			/* 5 GHz middle */
			temp_coeff = pi_lcn->rxgain_tempadj_5gm;
			break;

		case WL_CHAN_FREQ_RANGE_5GH:
			/* 5 GHz high */
			temp_coeff = pi_lcn->rxgain_tempadj_5gh;
			break;
	#endif /* BAND5G */
		default:
			temp_coeff = 0;
			break;
	}

	/* gain_err_temp_adj is on 0.25dB steps */
	gain_err_temp_adj = (curr_temp * temp_coeff) >> 8;

	return gain_err_temp_adj;
}

void
wlc_lcn40phy_rx_power(phy_info_t *pi, uint16 num_samps, uint8 wait_time,
	uint8 wait_for_crs, phy_iq_est_t* est)
{
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	uint32 rx_pwr = 0, prev_rx_pwr = 0;
	uint16 biq2, biq1, digi;
	uint16 trsw, ext_lna, tia, lna2, lna1;
	uint16 digi_offset, slna_byp, slna_rout, lna2_rout;
	bool rxtestcond, iq_done, loop_cont_cond, threshcond;
	uint16 rf_override_2;
	uint32 adj_min, adj_max;
	uint32 adj_plusdB, adj_mindB;
	uint8 counter = 0, step = 0;
	bool suspend;
	uint8 retry_count;
	/* uint16 lna_freq_tune; */
	uint8 count;
	uint16 reduced_nsamples = 1 << 10;
	uint32 pwr_thresh;
	uint32 sample_adjust;
	uint32 i_pwr_red, q_pwr_red;
	uint8 timeout_count = 8;
	uint8 reduced_timeout = 1;
	uint8 full_timeout = 20;
	uint8 rxpath_steps = pi_lcn->rxpath_steps;

	suspend = !(R_REG(pi->sh->osh, &pi->regs->maccontrol) & MCTL_EN_MAC);
	if (!suspend)
		wlapi_suspend_mac_and_wait(pi->sh->physhim);

	wlc_btcx_override_enable(pi);

	rf_override_2 = phy_utils_read_phyreg(pi, LCN40PHY_rfoverride2);
	/* lna_freq_tune = phy_utils_read_radioreg(pi, RADIO_2064_REG0AD); */

	wlc_lcnphy_deaf_mode(pi, FALSE);

	/* Force the TR switch to Receive */
	wlc_lcn40phy_set_trsw_override(pi, FALSE, TRUE);

	phy_lcn40_radio_switch(pi->u.pi_lcn40phy->radioi, ON);

	wlc_lcn40phy_rx_pu(pi, TRUE);

	PHY_REG_LIST_START
		/* ADC */
		PHY_REG_MOD_ENTRY(LCN40PHY, AfeCtrlOvr1, adc_pu_ovr, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, AfeCtrlOvr1Val, adc_pu_ovr_val, 0)
		PHY_REG_MOD_ENTRY(LCN40PHY, AfeCtrlOvr1, adc_pu_ovr, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, AfeCtrlOvr1Val, adc_pu_ovr_val, 31)

		/* Set the LPF in the Rx mode */
		PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride4val, lpf_sel_tx_rx_ovr_val, 0)
		PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride4, lpf_sel_tx_rx_ovr, 1)

		PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride7val, lpf_hpc_ovr_val, 3)
		PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride7, lpf_hpc_ovr, 0x1)

		PHY_REG_MOD_ENTRY(LCN40PHY, ClkEnCtrl, forcerxfrontendclk, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, AfeCtrlOvr1Val, afe_iqadc_aux_en_ovr_val, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, AfeCtrlOvr1, afe_iqadc_aux_en_ovr, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, sslpnCalibClkEnCtrl, forceTxfiltClkOn, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, sslpnCalibClkEnCtrl, forceaphytxFeclkOn, 1)

		PHY_REG_WRITE_ENTRY(LCN40PHY, lpfgainlutreg, 0)
	PHY_REG_LIST_EXECUTE(pi);

	wlc_lcn40phy_set_lna_freq_comp(pi);

	pi_lcn->rxpath_final_power = 0;
	pi_lcn->rxpath_gainselect_power = 0;
	pi_lcn->rxpath_status = 0;
	est->i_pwr = 0;
	est->q_pwr = 0;

	wlc_lcn40phy_agc_reset(pi);
	PHY_REG_MOD(pi, LCN40PHY, agcControl4, c_agc_fsm_en, 0);

	trsw = 0;
	ext_lna = 1;

	digi_offset = 0;
	slna_byp = 0;

#ifdef BAND5G
	if (CHSPEC_IS5G(pi->radio_chanspec)) {
			tia = 6;
			lna2 = 3;
			lna2_rout = 3;
			lna1 = 3;
			slna_rout = 4;
			retry_count = 2;
			pwr_thresh = pi_lcn->pwr_thresh_5g;

	} else
#endif /* BAND5G */
	{
			tia = 6;
			lna2 = 6;
			lna2_rout = 0;
			lna1 = 5;
			slna_rout = 0;
			retry_count = 2;
			pwr_thresh = pi_lcn->pwr_thresh_2g;
	}

	while (counter < retry_count)
	{
		prev_rx_pwr = 0;
		step = 0;

#ifdef BAND5G
		if (CHSPEC_IS5G(pi->radio_chanspec)) {
				biq2 = 4;
				biq1 = 6;
				digi = 0;
		} else
#endif /* BAND5G */
		{
				biq2 = 3;
				biq1 = 6;
				digi = 0;
		}

		do {
			step++;
			if (pi_lcn->rxpath_index >= 76) {

				wlc_lcn40phy_set_rx_gain_by_distribution(pi, trsw, ext_lna,
					slna_byp, slna_rout, lna1, lna2, lna2_rout, tia,
					biq1, biq2, digi, digi_offset);
				wlc_lcn40phy_rx_gain_override_enable(pi, TRUE);
				wlc_lcn40phy_agc_fsm_toggle(pi);

				/* Toggle ADC here to get rid of random errors */
				PHY_REG_LIST_START
					PHY_REG_MOD_ENTRY(LCN40PHY, AfeCtrlOvr1, adc_pu_ovr, 1)
					PHY_REG_MOD_ENTRY(LCN40PHY, AfeCtrlOvr1Val, adc_pu_ovr_val,
						0)
				PHY_REG_LIST_EXECUTE(pi);

				OSL_DELAY(10);

				PHY_REG_LIST_START
					PHY_REG_MOD_ENTRY(LCN40PHY, AfeCtrlOvr1, adc_pu_ovr, 1)
					PHY_REG_MOD_ENTRY(LCN40PHY, AfeCtrlOvr1Val, adc_pu_ovr_val,
						31)
				PHY_REG_LIST_EXECUTE(pi);
				OSL_DELAY(10);

				pi_lcn->rxpath_gain = (int16) wlc_lcn40phy_calc_rx_gain(pi);
				pi_lcn->rxpath_gain = pi_lcn->rxpath_gain << 2; /* Convert to qdB */
			} else {
				rxpath_steps = 1;

				wlc_lcn40phy_set_gain_by_index(pi, pi_lcn->rxpath_index);

				/* Turn on/off elna */
				if ((CHSPEC_IS2G(pi->radio_chanspec) &&
				(BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA)) ||
				(CHSPEC_IS5G(pi->radio_chanspec) &&
				(BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA_5GHz))) {
					if (!pi_lcn->rxpath_elna) {
						PHY_REG_MOD(pi, LCN40PHY,
							RFOverride0, trsw_rx_pu_ovr, 1);
						PHY_REG_MOD(pi, LCN40PHY,
							RFOverrideVal0, trsw_rx_pu_ovr_val, 0);
					}
				}

				pi_lcn->rxpath_gain = wlc_lcn40phy_get_rxpath_gain_by_index(pi,
					pi_lcn->rxpath_index, !pi_lcn->rxpath_elna);

			}

			iq_done = FALSE;
			for (count = 0; count < timeout_count; count++) {
				iq_done = wlc_lcn40phy_rx_iq_est(pi, reduced_nsamples, wait_time,
					wait_for_crs, est, reduced_timeout);
				if (((est->i_pwr + est->q_pwr) / reduced_nsamples) == 0)
					iq_done = FALSE;
				if (iq_done)
					break;
			}
			if (!iq_done) {
				PHY_ERROR(("wl%d: %s: IQ Estimation Failed\n",
				pi->sh->unit, __FUNCTION__));
				pi_lcn->rxpath_status |= RX_GAIN_CAL_GS_TIMEOUT;
			}

			prev_rx_pwr = rx_pwr;
			rx_pwr = (est->i_pwr + est->q_pwr) / reduced_nsamples;
			i_pwr_red = est->i_pwr;
			q_pwr_red = est->q_pwr;
			pi_lcn->rxpath_gainselect_power = rx_pwr;

			/* if (prev_rx_pwr_dbm - rx_pwr_dbm) > 2.8dBm and < 3.3, break */
			adj_min = (rx_pwr * 122) >> 6;
			adj_max = (rx_pwr * 137) >> 6;

			rxtestcond = 0;
			if (step != 1)
				rxtestcond = ((prev_rx_pwr > adj_min) && (prev_rx_pwr < adj_max));
			if (rxtestcond)
				pi_lcn->rxpath_status |= RX_GAIN_CAL_3DB_DECISION;

			threshcond = 0;
			if (step <= 2)
				threshcond = rx_pwr < pwr_thresh;
			if (threshcond)
				pi_lcn->rxpath_status |= RX_GAIN_CAL_THRESH_DECISION;

			PHY_TMP(("prev_rx_pwr = %d, rxpwr = %d, rxgain = %d, tia = %d, lna2 = %d, "
				"lna1 = %d, biq2 = %d, biq1 = %d, digi = %d\n",
				prev_rx_pwr, rx_pwr, pi_lcn->rxpath_gain,
				tia, lna2, lna1, biq2, biq1, digi));

			if (biq1)
				biq1 = biq1 - 1;
			else if (biq2)
				biq2 = biq2 - 1;
			else
				break;

			if (rxpath_steps)
				loop_cont_cond = step < rxpath_steps;
			else
				loop_cont_cond = (!rxtestcond) && (!threshcond);
		} while (loop_cont_cond);

		if ((rxtestcond) || (threshcond) || (rxpath_steps)) {
			break;
		}
		else {
			tia = tia - 1;
			counter++;
		}
	}

	iq_done = FALSE;
	for (count = 0; count < timeout_count; count++) {
		iq_done = wlc_lcn40phy_rx_iq_est(pi, num_samps, wait_time,
			wait_for_crs, est, full_timeout);
		if (((est->i_pwr + est->q_pwr) / num_samps) == 0)
			iq_done = FALSE;
		if (iq_done)
			break;
	}
	if (!iq_done) {
		PHY_ERROR(("wl%d: %s: Possible Saturation, Final IQ Estimation Failed\n",
		pi->sh->unit, __FUNCTION__));
		sample_adjust = num_samps/reduced_nsamples;
		est->i_pwr = i_pwr_red * sample_adjust;
		est->q_pwr = q_pwr_red * sample_adjust;
		pi_lcn->rxpath_status |= RX_GAIN_CAL_FINAL_TIMEOUT;
	}
	pi_lcn->rxpath_final_power = (est->i_pwr + est->q_pwr) / num_samps;

	/* Check if rxpath_final_power shows more than +-3 dB variaion from rx_pwr */
	adj_plusdB = (rx_pwr * 128) >> 6;
	adj_mindB = (pi_lcn->rxpath_final_power * 128) >> 6;
	if ((pi_lcn->rxpath_final_power > adj_plusdB) || (rx_pwr > adj_mindB)) {
		sample_adjust = num_samps/reduced_nsamples;
		est->i_pwr = i_pwr_red * sample_adjust;
		est->q_pwr = q_pwr_red * sample_adjust;
		pi_lcn->rxpath_final_power = (est->i_pwr + est->q_pwr) / num_samps;
		pi_lcn->rxpath_status |= RX_GAIN_CAL_3DB_ADJUST;
	}

	PHY_REG_LIST_START
		PHY_REG_MOD_ENTRY(LCN40PHY, RFOverride0, internalrfrxpu_ovr, 0)
		PHY_REG_MOD_ENTRY(LCN40PHY, AfeCtrlOvr1, adc_pu_ovr, 0)
		PHY_REG_MOD_ENTRY(LCN40PHY, RFOverride0, trsw_rx_pu_ovr, 0)
	PHY_REG_LIST_EXECUTE(pi);

	wlc_lcn40phy_rx_gain_override_enable(pi, FALSE);
	wlc_lcn40phy_clear_trsw_override(pi);
	phy_utils_write_phyreg(pi, LCN40PHY_rfoverride2, rf_override_2);
	PHY_REG_MOD(pi, LCN40PHY, agcControl4, c_agc_fsm_en, 1);
	/* phy_utils_write_radioreg(pi, RADIO_2064_REG0AD, lna_freq_tune); */

	wlc_phy_btcx_override_disable(pi);

	if (!suspend)
		wlapi_enable_mac(pi->sh->physhim);

}
#endif  /* #if defined(WLTEST) */

#ifdef SAMPLE_COLLECT
static uint32
wlc_lcn40phy_sqrt_32(uint32 data)
{
	uint32 ans = 0;
	uint32 i = 1 << 15;

	while (i) {
		ans = ans + i;
		if ((ans * ans) > data)
			ans = ans - i;
		i = i >> 1;
	}
	PHY_INFORM(("%s: sqrt(%d) = %d\n", __FUNCTION__, data, ans));
	return ans;
}

static void
wlc_lcn40phy_find_max_min_in_window(uint32* data, uint32 win_size,
	uint32* i_maxval, uint32* i_minval, uint32* q_maxval, uint32* q_minval,
	int32* iq_maxval, int32* iq_minval)
{
	uint8 i;
	int16 IData, QData;
	uint32 Ipwr = 0, Qpwr = 0;
	uint32 Iamp, Qamp;

	uint16 wordlength = 14;
	uint16 mask = ((0x1 << wordlength) - 1);
	uint16 wrap = (0x1 << (wordlength - 1));
	uint16 maxd = (0x1 << wordlength);

	for (i = 0; i < win_size; i++) {

		IData = data[i] & mask;
		QData = ((data[i] >> 16) & mask);
	  if (IData >= wrap)
			IData = IData - maxd;
	  if (QData >= wrap)
			QData = QData - maxd;

		Ipwr = Ipwr + IData*IData;
		Qpwr = Qpwr + QData*QData;
	}
	Ipwr = Ipwr / win_size;
	Qpwr = Qpwr / win_size;

	*i_maxval = MAX(*i_maxval, Ipwr);
	*i_minval = MIN(*i_minval, Ipwr);

	*q_maxval = MAX(*q_maxval, Qpwr);
	*q_minval = MIN(*q_minval, Qpwr);

	Iamp = wlc_lcn40phy_sqrt_32(Ipwr);
	Qamp = wlc_lcn40phy_sqrt_32(Qpwr);

	*iq_maxval = MAX(*iq_maxval, ((int32)Iamp-(int32)Qamp));
	*iq_minval = MIN(*iq_minval, ((int32)Iamp-(int32)Qamp));
}

static uint32
wlc_lcn40phy_find_IQ_steady(uint32* data, uint32 win_size)
{
	uint8 i;
	int16 IData, QData;
	uint32 Iamp, Qamp;
	uint32 Ipwr = 0, Qpwr = 0;

	uint16 wordlength = 14;
	uint16 mask = ((0x1 << wordlength) - 1);
	uint16 wrap = (0x1 << (wordlength - 1));
	uint16 maxd = (0x1 << wordlength);

	for (i = 0; i < win_size; i++) {
		IData = data[i] & mask;
		QData = ((data[i] >> 16) & mask);
		if (IData >= wrap)
			IData = IData - maxd;
	  if (QData >= wrap)
			QData = QData - maxd;
		Ipwr = Ipwr + IData*IData;
		Qpwr = Qpwr + QData*QData;
	}
	Ipwr = Ipwr / win_size;
	Qpwr = Qpwr / win_size;

	Iamp = wlc_lcn40phy_sqrt_32(Ipwr);
	Qamp = wlc_lcn40phy_sqrt_32(Qpwr);

	return ((Iamp + Qamp)/2);
}

#define LCN40PHY_SAMPLE_SKIP_COUNT_EVM	240 /* 200 + 40 for PU */
#define LCN40PHY_SAMPLE_SKIP_COUNT_PSD	90  /* 50 + 40 for PU */
#define LCN40PHY_IQMIN	0xFFFFFFFF
#define LCN40PHY_IQMAX	0
#define LCN40PHY_IQMIN_INT32	0x7FFFFFFF
#define LCN40PHY_IQMAX_INT32	0x80000000

int
phy_lcn40_iqimb_check(phy_info_t *pi, uint32 nsamps, uint32 *buf, int32 *metric, int32 *result)
{
	bool suspend;
	uint32 curval_psm;
	uint32 samp_coll_ctl;
	uint8 strt_trig = 0;
	uint8 end_trig = 0;
	uint8 crs_state1 = 0;
	uint8 crs_state2 = 1;
	uint32 strt_timer = 0;
	uint32 end_timer = 4 * nsamps;
	uint8 gpio_mode_sel = 3;
	uint8 gpio_bit_chunk = 0;
	uint8 gpio_clk_rate = 1;
	uint8 strt_trig_dly = 0;
	uint8 end_trig_dly = 0;
	uint8 gpio_trg_dly = 0;
	uint8 sel_lpphy_gpio = 0;
	uint8 data2maconly = 1;
	uint8 mac_phy_xfer_rate = 1;
	uint8 spb_nbits = 0;
	uint8 spb_wrap_around = 1;
	uint8 strt_mac_xfer_mode;
	uint8 end_mac_xfer_mode;
	uint16 strt_timer_low, strt_timer_high, end_timer_low, end_timer_high;
	uint16 dbgctrlword;
	uint32 wait_count = 0;
	uint32 start_ptr, stop_ptr, curr_ptr, nsamps_out, data, cnt;
	uint16 rf_override_2;
	uint16 old_sslpnCalibClkEnCtrl, sslpnCalibClkEnCtrl;
	uint8 module_sel1 = 2;
	uint8 module_sel2 = 6;
	uint32 i_maxval, i_minval, q_maxval, q_minval;
	int32 iq_maxval, iq_minval;
	uint32 psd_metric;
	uint32 I_amp_max, I_amp_min, Q_amp_max, Q_amp_min;
	uint32 err_I, err_Q, evm[PHY_CORE_MAX];
	uint32 *Data_trim;
	uint32 win_size = 50;
	uint32 x;
	uint32 IQsteady;
	uint32 *local_buf = NULL;
	uint16 save_2065_top_spare3, save_2065_ovr7, save_2065_lpfcfg1;
	uint16 SAVE_txpwrctrl;
	uint8 SAVE_bbmult;
	uint16 save_a, save_b, save_didq;
	uint32 scale;
	int8 evm_dB[PHY_CORE_MAX];
	bool pass = 0;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	suspend = !(R_REG(pi->sh->osh, &pi->regs->maccontrol) & MCTL_EN_MAC);
	if (!suspend)
		wlapi_suspend_mac_and_wait(pi->sh->physhim);

	SAVE_txpwrctrl = wlc_lcn40phy_get_tx_pwr_ctrl(pi);
	SAVE_bbmult = wlc_lcn40phy_get_bbmult(pi);
	old_sslpnCalibClkEnCtrl = phy_utils_read_phyreg(pi, LCN40PHY_sslpnCalibClkEnCtrl);
	wlc_lcn40phy_get_tx_iqcc(pi, &save_a, &save_b);
	save_didq = wlc_lcn40phy_get_tx_locc(pi);

	/* Force WLAN antenna */
	wlc_btcx_override_enable(pi);

	rf_override_2 = phy_utils_read_phyreg(pi, LCN40PHY_rfoverride2);
	save_2065_top_spare3 = phy_utils_read_radioreg(pi, RADIO_2065_TOP_SPARE3);
	save_2065_ovr7 = phy_utils_read_radioreg(pi, RADIO_2065_OVR7);
	save_2065_lpfcfg1 = phy_utils_read_radioreg(pi, RADIO_2065_LPF_CFG1);

	wlc_lcnphy_deaf_mode(pi, FALSE);

	/* Force the TR switch to Receive */
	wlc_lcn40phy_set_trsw_override(pi, FALSE, TRUE);

	phy_lcn40_radio_switch(pi->u.pi_lcn40phy->radioi, ON);

	wlc_lcn40phy_rx_pu(pi, TRUE);

	PHY_REG_LIST_START
		/* DAC to ADC loopback */
		/* set reg(RF_top_spare3.lpf_unsel_dac) 0 */
		RADIO_REG_MOD_ENTRY(RADIO_2065_TOP_SPARE3, 0x2, 0<<1)
		/* set reg(RF_top_spare3.lpf_sel_gpio) 0 */
		RADIO_REG_MOD_ENTRY(RADIO_2065_TOP_SPARE3, 0x0, 0)
		/* set reg(RF_OVR7.ovr_lpf_sel_byp_txlpf) 1 */
		RADIO_REG_MOD_ENTRY(RADIO_2065_OVR7, 0x10, 1<<4)
		/* set reg(RF_lpf_cfg1.sel_byp_txlpf) 1 */
		RADIO_REG_MOD_ENTRY(RADIO_2065_LPF_CFG1, 0x200, 1<<9)
		/* set reg(RF_OVR7.ovr_lpf_sel_tx_rx) 1 */
		RADIO_REG_MOD_ENTRY(RADIO_2065_OVR7, 0x4, 1<<2)
		/* set reg(RF_lpf_cfg1.sel_tx_rx) 0x0 */
		RADIO_REG_MOD_ENTRY(RADIO_2065_LPF_CFG1, 0xc00, 0<<10)

		PHY_REG_MOD_ENTRY(LCN40PHY, ClkEnCtrl, forcerxfrontendclk, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, AfeCtrlOvr1Val, afe_iqadc_aux_en_ovr_val, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, AfeCtrlOvr1, afe_iqadc_aux_en_ovr, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, sslpnCalibClkEnCtrl, forceTxfiltClkOn, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, sslpnCalibClkEnCtrl, forceaphytxFeclkOn, 1)
	PHY_REG_LIST_EXECUTE(pi);

	/* Force ADC ON, otherwise lcn40_play_tone will turn ADC off */
	PHY_REG_LIST_START
		PHY_REG_MOD_ENTRY(LCN40PHY, AfeCtrlOvr1, adc_pu_ovr, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, AfeCtrlOvr1Val, adc_pu_ovr_val, 31)
	PHY_REG_LIST_EXECUTE(pi);

	wlc_lcn40phy_set_tx_pwr_ctrl(pi, LCN40PHY_TX_PWR_CTRL_OFF);
	wlc_lcn40phy_set_tx_locc(pi, 0);
	wlc_lcn40phy_set_tx_iqcc(pi, 0, 0);
	wlc_lcn40phy_set_tx_pwr_by_index(pi, 60);
	wlc_lcn40phy_set_bbmult(pi, 100);
	wlc_lcn40phy_start_tx_tone(pi, (4000 * 1000), 511, 0);

	wlc_lcn40phy_agc_reset(pi);
	PHY_REG_MOD(pi, LCN40PHY, agcControl4, c_agc_fsm_en, 0);

	/* Turn OFF the DAC */
	PHY_REG_MOD(pi, LCN40PHY, AfeCtrlOvr1, dac_pu_ovr, 1);
	PHY_REG_MOD(pi, LCN40PHY, AfeCtrlOvr1Val, dac_pu_ovr_val, 0);
	OSL_DELAY(1000);

	/* enabling clocks to sampleplay buffer and debug blocks */
	sslpnCalibClkEnCtrl = phy_utils_read_phyreg(pi, LCN40PHY_sslpnCalibClkEnCtrl);
	PHY_REG_WRITE(pi, LCN40PHY, sslpnCalibClkEnCtrl, (uint32)(sslpnCalibClkEnCtrl | 0x2008));

	if (buf == NULL) {
		if ((local_buf = (uint32*) MALLOC(pi->sh->osh, nsamps * sizeof(uint32))) == NULL) {
			PHY_ERROR(("wl%d: %s: MALLOC failure\n", pi->sh->unit, __FUNCTION__));
			return BCME_ERROR;
		}
		buf = local_buf;
	}

	bzero((uint8 *)buf, nsamps * 4);

	/* switch dot11mac_clk to macphy clock (80MHz) */
	curval_psm = R_REG(pi->sh->osh, &pi->regs->psm_corectlsts);
	W_REG(pi->sh->osh, &pi->regs->psm_corectlsts, ((1 << 6) | curval_psm));

	/* set start_sample_collect (bit 4) and stop_sample_collect (bit 5) bits */
	/* setting bit 5 makes it a NON-circular buffer */
	samp_coll_ctl =
		(R_REG(pi->sh->osh, &pi->regs->psm_phy_hdr_param) & 0xffcf) |
		(1 << 4) |
		(1 << 5);

	/* Configure PHY for debug capture mode */
	strt_mac_xfer_mode = 0;
	end_mac_xfer_mode = 0;

	phy_utils_write_phyreg(pi, LCN40PHY_debugmux, module_sel1);

	if (LCN40REV_LT(pi->pubpi.phy_rev, 4))
		PHY_REG_MOD(pi, LCN40PHY, sslnpny_rxfe_muxsel, dbgSelDbg, module_sel2);
	else
		PHY_REG_MOD(pi, LCN40PHY, sslnpny_rxfe_muxsel, dbgSelGPIO, module_sel2);

	/* Enabling clocks to sampleplaybuffer and debugblocks */
	phy_utils_or_phyreg(pi, LCN40PHY_sslpnCalibClkEnCtrl, 0x2008);

	/* Set the timer values */
	strt_timer_low = strt_timer & 0xffff;
	strt_timer_high = (strt_timer & 0xffff0000) >> 16;
	end_timer_low = end_timer & 0xffff;
	end_timer_high = (end_timer & 0xffff0000) >> 16;

	phy_utils_write_phyreg(pi, LCN40PHY_dbg_samp_coll_start_mac_xfer_trig_timer_15_0,
	                       strt_timer_low);
	phy_utils_write_phyreg(pi, LCN40PHY_dbg_samp_coll_start_mac_xfer_trig_timer_31_16,
	                       strt_timer_high);
	phy_utils_write_phyreg(pi, LCN40PHY_dbg_samp_coll_end_mac_xfer_trig_timer_15_0,
	                       end_timer_low);
	phy_utils_write_phyreg(pi, LCN40PHY_dbg_samp_coll_end_mac_xfer_trig_timer_31_16,
	                       end_timer_high);

	/* Programming the triggers */
	phy_utils_write_phyreg(pi, LCN40PHY_triggerConfiguration,
	                       ((strt_trig << 5) | (end_trig << 10)));

	/* Progrmming crs state trigger parameters */
	phy_utils_and_phyreg(pi, LCN40PHY_crsMiscCtrl0, 0x43ff);
	phy_utils_or_phyreg(pi, LCN40PHY_crsMiscCtrl0, (crs_state2 << 10));

	phy_utils_and_phyreg(pi, LCN40PHY_gpioTriggerConfig, 0x3f);
	phy_utils_or_phyreg(pi, LCN40PHY_gpioTriggerConfig, (crs_state1 << 6));

	/* Programming the trigger delays */
	phy_utils_write_phyreg(pi, LCN40PHY_gpioDlyConfig,
		((strt_trig_dly << 3) | (end_trig_dly << 6) | (gpio_trg_dly << 9)));

	/* Programming sslpnphy_gpio_ctrl */
	phy_utils_write_phyreg(pi, LCN40PHY_sslpnphy_gpio_ctrl,
		(1 | (sel_lpphy_gpio << 1) | (gpio_clk_rate << 2)));

	/* Programming gpio_data_ctrl */
	phy_utils_write_phyreg(pi, LCN40PHY_gpio_data_ctrl,
	                       (gpio_mode_sel | (gpio_bit_chunk << 2)));

	/* Enabling all the gpios */
	if (LCN40REV_LT(pi->pubpi.phy_rev, 4))
		phy_utils_write_phyreg(pi, LCN40PHY_sslpnphy_gpio_out_en_34_32, 0x1);
	else
		phy_utils_write_phyreg(pi, LCN40PHY_sslpnphy_gpio_out_en_15_0, 0xffff);

	/* Programming dbg_samp_coll_ctrl word */
	dbgctrlword = (4 | (strt_mac_xfer_mode << 5) | (end_mac_xfer_mode << 6) |
	            (data2maconly << 8) | (spb_nbits << 9) | (mac_phy_xfer_rate << 10) |
	            (spb_wrap_around << 14));

	phy_utils_write_phyreg(pi, LCN40PHY_dbg_samp_coll_ctrl, dbgctrlword);

	phy_utils_or_phyreg(pi, LCN40PHY_sslpnphy40_soft_reset, 3);

	/* start and stop pointer address */
	W_REG(pi->sh->osh, &pi->regs->PHYREF_SMPL_CLCT_STRPTR, 0);
	W_REG(pi->sh->osh, &pi->regs->PHYREF_SMPL_CLCT_STPPTR, nsamps - 1);

	phy_utils_and_phyreg(pi, LCN40PHY_sslpnphy40_soft_reset, (~3));

	/* Single shot sample capture PHY_CTL = 0x832 */
	W_REG(pi->sh->osh, &pi->regs->psm_phy_hdr_param, samp_coll_ctl);

	PHY_REG_MOD(pi, LCN40PHY, dbg_samp_coll_ctrl, samp_coll_en, 1);

	/* Turn ON the DAC */
	PHY_REG_MOD(pi, LCN40PHY, AfeCtrlOvr1Val, dac_pu_ovr_val, 1);

	/* Wait for Sample Collect to complete */
	while (phy_utils_read_phyreg(pi, LCN40PHY_dbg_samp_coll_ctrl) &
		LCN40PHY_dbg_samp_coll_ctrl_samp_coll_en_MASK) {
		/* Check for timeout */
		if (wait_count > 1000) {
			PHY_ERROR(("wl%d: %s: Sample Capture failed to complete\n",
				pi->sh->unit, __FUNCTION__));
			break;
		}
		OSL_DELAY(10);
		wait_count++;
	}

	/* Reset things for next iteration */
	samp_coll_ctl = (R_REG(pi->sh->osh, &pi->regs->psm_phy_hdr_param) & 0xffcf);
	W_REG(pi->sh->osh, &pi->regs->psm_phy_hdr_param, samp_coll_ctl);

	phy_utils_or_phyreg(pi, LCN40PHY_sslpnphy40_soft_reset, 3);

	/* Stop Sample Capture and Play */
	W_REG(pi->sh->osh, &pi->regs->psm_phy_hdr_param, 0x002);

	/* Copy data into a buffer */
	start_ptr = R_REG(pi->sh->osh, &pi->regs->PHYREF_SMPL_CLCT_STRPTR);
	stop_ptr = R_REG(pi->sh->osh, &pi->regs->PHYREF_SMPL_CLCT_STPPTR);
	curr_ptr = R_REG(pi->sh->osh, &pi->regs->PHYREF_SMPL_CLCT_CURPTR) + 1;
	nsamps_out = nsamps;
	cnt = 0;
	while (nsamps_out > 0) {
		if (curr_ptr > stop_ptr) {
		    curr_ptr = start_ptr;
		}
		wlapi_bmac_templateptr_wreg(pi->sh->physhim, curr_ptr << 2);
		data = wlapi_bmac_templatedata_rreg(pi->sh->physhim);
		buf[cnt++] = data;
		curr_ptr += 1;
		nsamps_out -= 1;
	}

	/* CALCULATE EVM METRIC */
	/* Discard first 240 samples to reach PU region */
	Data_trim = &buf[LCN40PHY_SAMPLE_SKIP_COUNT_EVM];

	i_maxval = LCN40PHY_IQMAX;
	i_minval = LCN40PHY_IQMIN;
	q_maxval = LCN40PHY_IQMAX;
	q_minval = LCN40PHY_IQMIN;
	iq_maxval = LCN40PHY_IQMAX_INT32;
	iq_minval = LCN40PHY_IQMIN_INT32;

	x = nsamps - LCN40PHY_SAMPLE_SKIP_COUNT_EVM - win_size; /* Observation Window */
	for (cnt = 0; cnt < 0+x; cnt++) {
		wlc_lcn40phy_find_max_min_in_window(&Data_trim[cnt], win_size,
			&i_maxval, &i_minval, &q_maxval, &q_minval, &iq_maxval, &iq_minval);
	}

	PHY_INFORM(("wl%d: %s: I_pwr_max = %d, I_pwr_min = %d, Q_pwr_max = %d, Q_pwr_min = %d\n",
		pi->sh->unit, __FUNCTION__, i_maxval,
		i_minval, q_maxval, q_minval));

	scale = 10000;
	I_amp_max = wlc_lcn40phy_sqrt_32(i_maxval);
	I_amp_min = wlc_lcn40phy_sqrt_32(i_minval);
	Q_amp_max = wlc_lcn40phy_sqrt_32(q_maxval);
	Q_amp_min = wlc_lcn40phy_sqrt_32(q_minval);

	err_I = ((I_amp_max - I_amp_min)*scale)/I_amp_max;
	err_Q = ((Q_amp_max - Q_amp_min)*scale)/Q_amp_max;

	evm[0] = (err_I*err_I) + (err_Q *err_Q);

	phy_utils_computedB(evm, evm_dB, PHYCORENUM(pi->pubpi.phy_corenum));
	evm_dB[0] = evm_dB[0] - 80;

	PHY_INFORM(("wl%d: %s: I_amp_max = %d, I_amp_min = %d,"
		"Q_amp_max = %d, Q_amp_min = %d, evm = %d, evm_dB = %d\n",
		pi->sh->unit, __FUNCTION__,
		I_amp_max, I_amp_min, Q_amp_max, Q_amp_min, evm[0], evm_dB[0]));

	/* CALCULATE PSD METRIC */
	/* Discard first 90 samples to reach PU region */
	Data_trim = &buf[LCN40PHY_SAMPLE_SKIP_COUNT_PSD];

	i_maxval = LCN40PHY_IQMAX;
	i_minval = LCN40PHY_IQMIN;
	q_maxval = LCN40PHY_IQMAX;
	q_minval = LCN40PHY_IQMIN;
	iq_maxval = LCN40PHY_IQMAX_INT32;
	iq_minval = LCN40PHY_IQMIN_INT32;

	x = 1024 - LCN40PHY_SAMPLE_SKIP_COUNT_PSD - win_size; /* Observation Window */
	for (cnt = 0; cnt < 0+x; cnt++) {
		wlc_lcn40phy_find_max_min_in_window(&Data_trim[cnt], win_size,
			&i_maxval, &i_minval, &q_maxval, &q_minval, &iq_maxval, &iq_minval);
	}

	psd_metric = iq_maxval - iq_minval;
	IQsteady = wlc_lcn40phy_find_IQ_steady(&Data_trim[x-1], win_size);
	psd_metric = (psd_metric * 7000)/(IQsteady);

	PHY_INFORM(("wl%d: %s: IQ_pwr_max = %d, IQ_pwr_min = %d, IQsteady = %d, psd_metric = %d\n",
		pi->sh->unit, __FUNCTION__, iq_maxval, iq_minval, IQsteady, psd_metric));

	pass = ((psd_metric < 50) && (evm_dB[0] < -41)) || (evm_dB[0] < -45);

	if (result)
		*result = pass;

	if (metric) {
		*metric = ((((int16)psd_metric) << 16) & 0xFFFF0000) |
			((((int8)evm_dB[0]) << 8) & 0xFF00) | (pass & 0xFF);
	}

	wlc_lcn40phy_stop_tx_tone(pi);
	PHY_REG_MOD(pi, LCN40PHY, RFOverride0, internalrfrxpu_ovr, 0);
	PHY_REG_MOD(pi, LCN40PHY, AfeCtrlOvr1, adc_pu_ovr, 0);
	PHY_REG_MOD(pi, LCN40PHY, AfeCtrlOvr1, dac_pu_ovr, 0);
	wlc_lcn40phy_rx_gain_override_enable(pi, FALSE);
	wlc_lcn40phy_clear_trsw_override(pi);
	phy_utils_write_phyreg(pi, LCN40PHY_rfoverride2, rf_override_2);
	PHY_REG_MOD(pi, LCN40PHY, agcControl4, c_agc_fsm_en, 1);
	PHY_REG_WRITE(pi, LCN40PHY, sslpnCalibClkEnCtrl, old_sslpnCalibClkEnCtrl);
	phy_utils_write_radioreg(pi, RADIO_2065_TOP_SPARE3, save_2065_top_spare3);
	phy_utils_write_radioreg(pi, RADIO_2065_OVR7, save_2065_ovr7);
	phy_utils_write_radioreg(pi, RADIO_2065_LPF_CFG1, save_2065_lpfcfg1);
	wlc_lcn40phy_set_tx_locc(pi, save_didq);
	wlc_lcn40phy_set_tx_iqcc(pi, save_a, save_b);
	wlc_lcn40phy_set_bbmult(pi, SAVE_bbmult);
	wlc_lcn40phy_set_tx_pwr_ctrl(pi, SAVE_txpwrctrl);

	if (local_buf)
		MFREE(pi->sh->osh, local_buf, nsamps * sizeof(uint32));

	if (!suspend)
		wlapi_enable_mac(pi->sh->physhim);

	return BCME_OK;
}

int
phy_lcn40_sample_collect(phy_info_t *pi, wl_samplecollect_args_t *collect, uint32 *buf)
{
	bool suspend;
	uint32 curval_psm;
	uint32 samp_coll_ctl;
	uint8 strt_trig = 0;
	uint8 end_trig = 0;
	uint8 crs_state1 = 0;
	uint8 crs_state2 = 1;
	uint32 strt_timer = 0;
	uint32 end_timer = 4 * collect->nsamps;
	uint8 gpio_mode_sel = 3;
	uint8 gpio_bit_chunk = 0;
	uint8 gpio_clk_rate = 1;
	uint8 strt_trig_dly = 0;
	uint8 end_trig_dly = 0;
	uint8 gpio_trg_dly = 0;
	uint8 sel_lpphy_gpio = 0;
	uint8 data2maconly = 1;
	uint8 mac_phy_xfer_rate = 1;
	uint8 spb_nbits = 0;
	uint8 spb_wrap_around = 1;
	uint8 strt_mac_xfer_mode;
	uint8 end_mac_xfer_mode;
	uint16 strt_timer_low, strt_timer_high, end_timer_low, end_timer_high;
	uint16 dbgctrlword;
	uint32 wait_count = 0;
	uint32 start_ptr, stop_ptr, curr_ptr, nsamps_out, data, cnt = 0;
	uint16 rf_override_2;
	uint16 biq2, biq1, digi;
	uint16 trsw, ext_lna, tia, lna2, lna1;
	uint16 digi_offset, slna_byp, slna_rout, lna2_rout;
	uint16 old_sslpnCalibClkEnCtrl, sslpnCalibClkEnCtrl;
	phy_info_lcn40phy_t *pi_lcn40 = pi->u.pi_lcn40phy;
	int8 gainadj = pi_lcn40->sample_collect_gainadj;
	uint8 gainidx = pi_lcn40->sample_collect_gainidx;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	suspend = !(R_REG(pi->sh->osh, &pi->regs->maccontrol) & MCTL_EN_MAC);
	if (!suspend)
		wlapi_suspend_mac_and_wait(pi->sh->physhim);

	old_sslpnCalibClkEnCtrl = phy_utils_read_phyreg(pi, LCN40PHY_sslpnCalibClkEnCtrl);

	/* Force WLAN antenna */
	wlc_btcx_override_enable(pi);

	rf_override_2 = phy_utils_read_phyreg(pi, LCN40PHY_rfoverride2);
	/* lna_freq_tune = phy_utils_read_radioreg(pi, RADIO_2064_REG0AD); */

	wlc_lcnphy_deaf_mode(pi, FALSE);

	/* Force the TR switch to Receive */
	wlc_lcn40phy_set_trsw_override(pi, FALSE, TRUE);

	phy_lcn40_radio_switch(pi->u.pi_lcn40phy->radioi, ON);

	wlc_lcn40phy_rx_pu(pi, TRUE);

	PHY_REG_LIST_START
		/* Set the LPF in the Rx mode */
		PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride4val, lpf_sel_tx_rx_ovr_val, 0)
		PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride4, lpf_sel_tx_rx_ovr, 1)

		PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride7val, lpf_hpc_ovr_val, 3)
		PHY_REG_MOD_ENTRY(LCN40PHY, rfoverride7, lpf_hpc_ovr, 0x1)

		PHY_REG_MOD_ENTRY(LCN40PHY, ClkEnCtrl, forcerxfrontendclk, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, AfeCtrlOvr1Val, afe_iqadc_aux_en_ovr_val, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, AfeCtrlOvr1, afe_iqadc_aux_en_ovr, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, sslpnCalibClkEnCtrl, forceTxfiltClkOn, 1)
		PHY_REG_MOD_ENTRY(LCN40PHY, sslpnCalibClkEnCtrl, forceaphytxFeclkOn, 1)
	PHY_REG_LIST_EXECUTE(pi);

	/* enabling clocks to sampleplay buffer and debug blocks */
	sslpnCalibClkEnCtrl = phy_utils_read_phyreg(pi, LCN40PHY_sslpnCalibClkEnCtrl);
	PHY_REG_WRITE(pi, LCN40PHY, sslpnCalibClkEnCtrl, (uint32)(sslpnCalibClkEnCtrl | 0x2008));

	phy_utils_write_phyreg(pi, LCN40PHY_lpfgainlutreg, 0);

	wlc_lcn40phy_agc_reset(pi);
	PHY_REG_MOD(pi, LCN40PHY, agcControl4, c_agc_fsm_en, 0);

	if (gainidx >= 76) {
		trsw = 0;
		ext_lna = 1;
		digi_offset = 0;
		slna_byp = 0;
	#ifdef BAND5G
		if (CHSPEC_IS5G(pi->radio_chanspec)) {
				tia = 6;
				lna2 = 3;
				lna2_rout = 3;
				lna1 = 3;
				slna_rout = 4;
				biq2 = 4;
				biq1 = 4;
				digi = 0;
		} else
	#endif /* BAND5G */
		{
				tia = 6;
				lna2 = 6;
				lna2_rout = 0;
				lna1 = 5;
				slna_rout = 0;
				biq2 = 3;
				biq1 = 1;
				digi = 0;
		}

		if (gainadj > 0) {
			while (gainadj && ((biq2 < 6)||(biq1 < 6))) {
				if (biq2 < 6) {
					biq2++;
					gainadj--;
				} else if (biq1 < 6) {
					biq1++;
					gainadj--;
				}
			}
		} else {
			while (gainadj && ((biq2 > 0)||(biq1 > 0)||(tia > 0))) {
				if (biq2 > 0) {
					biq2--;
					gainadj++;
				} else if (biq1 > 0) {
					biq1--;
					gainadj++;
				} else if (tia > 0) {
					tia--;
					gainadj++;
				}
			}
		}
		pi_lcn40->sample_collect_gainadj -=  gainadj;

		wlc_lcn40phy_set_rx_gain_by_distribution(pi, trsw, ext_lna,
			slna_byp, slna_rout, lna1, lna2, lna2_rout, tia,
			biq1, biq2, digi, digi_offset);

		wlc_lcn40phy_rx_gain_override_enable(pi, TRUE);

		/* Toggle ADC here to get rid of random errors */
		PHY_REG_LIST_START
			PHY_REG_MOD_ENTRY(LCN40PHY, AfeCtrlOvr1, adc_pu_ovr, 1)
			PHY_REG_MOD_ENTRY(LCN40PHY, AfeCtrlOvr1Val, adc_pu_ovr_val, 0)
		PHY_REG_LIST_EXECUTE(pi);

		OSL_DELAY(10);
		PHY_REG_LIST_START
			PHY_REG_MOD_ENTRY(LCN40PHY, AfeCtrlOvr1, adc_pu_ovr, 1)
			PHY_REG_MOD_ENTRY(LCN40PHY, AfeCtrlOvr1Val, adc_pu_ovr_val, 31)
		PHY_REG_LIST_EXECUTE(pi);
		OSL_DELAY(10);
	} else {
			wlc_lcn40phy_set_gain_by_index(pi, gainidx);
	}

	bzero((uint8 *)buf, collect->nsamps * 4);

	/* switch dot11mac_clk to macphy clock (80MHz) */
	curval_psm = R_REG(pi->sh->osh, &pi->regs->psm_corectlsts);
	W_REG(pi->sh->osh, &pi->regs->psm_corectlsts, ((1 << 6) | curval_psm));

	/* set start_sample_collect (bit 4) and stop_sample_collect (bit 5) bits */
	/* setting bit 5 makes it a NON-circular buffer */
	samp_coll_ctl =
		(R_REG(pi->sh->osh, &pi->regs->psm_phy_hdr_param) & 0xffcf) |
		(1 << 4) |
		(1 << 5);

	/* Configure PHY for debug capture mode */
	strt_mac_xfer_mode = 0;
	end_mac_xfer_mode = 0;

	phy_utils_write_phyreg(pi, LCN40PHY_debugmux, collect->module_sel1);

	if (LCN40REV_LT(pi->pubpi.phy_rev, 4))
		PHY_REG_MOD(pi, LCN40PHY, sslnpny_rxfe_muxsel, dbgSelDbg, collect->module_sel2);
	else
		PHY_REG_MOD(pi, LCN40PHY, sslnpny_rxfe_muxsel, dbgSelGPIO, collect->module_sel2);

	/* Enabling clocks to sampleplaybuffer and debugblocks */
	phy_utils_or_phyreg(pi, LCN40PHY_sslpnCalibClkEnCtrl, 0x2008);

	/* Set the timer values */
	strt_timer_low = strt_timer & 0xffff;
	strt_timer_high = (strt_timer & 0xffff0000) >> 16;
	end_timer_low = end_timer & 0xffff;
	end_timer_high = (end_timer & 0xffff0000) >> 16;

	phy_utils_write_phyreg(pi, LCN40PHY_dbg_samp_coll_start_mac_xfer_trig_timer_15_0,
	                       strt_timer_low);
	phy_utils_write_phyreg(pi, LCN40PHY_dbg_samp_coll_start_mac_xfer_trig_timer_31_16,
	                       strt_timer_high);
	phy_utils_write_phyreg(pi, LCN40PHY_dbg_samp_coll_end_mac_xfer_trig_timer_15_0,
	                       end_timer_low);
	phy_utils_write_phyreg(pi, LCN40PHY_dbg_samp_coll_end_mac_xfer_trig_timer_31_16,
	                       end_timer_high);

	/* Programming the triggers */
	phy_utils_write_phyreg(pi, LCN40PHY_triggerConfiguration,
	                       ((strt_trig << 5) | (end_trig << 10)));

	/* Progrmming crs state trigger parameters */
	phy_utils_and_phyreg(pi, LCN40PHY_crsMiscCtrl0, 0x43ff);
	phy_utils_or_phyreg(pi, LCN40PHY_crsMiscCtrl0, (crs_state2 << 10));

	phy_utils_and_phyreg(pi, LCN40PHY_gpioTriggerConfig, 0x3f);
	phy_utils_or_phyreg(pi, LCN40PHY_gpioTriggerConfig, (crs_state1 << 6));

	/* Programming the trigger delays */
	phy_utils_write_phyreg(pi, LCN40PHY_gpioDlyConfig,
		((strt_trig_dly << 3) | (end_trig_dly << 6) | (gpio_trg_dly << 9)));

	/* Programming sslpnphy_gpio_ctrl */
	phy_utils_write_phyreg(pi, LCN40PHY_sslpnphy_gpio_ctrl,
		(1 | (sel_lpphy_gpio << 1) | (gpio_clk_rate << 2)));

	/* Programming gpio_data_ctrl */
	phy_utils_write_phyreg(pi, LCN40PHY_gpio_data_ctrl,
	                       (gpio_mode_sel | (gpio_bit_chunk << 2)));

	/* Enabling all the gpios */
	if (LCN40REV_LT(pi->pubpi.phy_rev, 4))
		phy_utils_write_phyreg(pi, LCN40PHY_sslpnphy_gpio_out_en_34_32, 0x1);
	else
		phy_utils_write_phyreg(pi, LCN40PHY_sslpnphy_gpio_out_en_15_0, 0xffff);

	/* Programming dbg_samp_coll_ctrl word */
	dbgctrlword = (4 | (strt_mac_xfer_mode << 5) | (end_mac_xfer_mode << 6) |
	            (data2maconly << 8) | (spb_nbits << 9) | (mac_phy_xfer_rate << 10) |
	            (spb_wrap_around << 14));

	phy_utils_write_phyreg(pi, LCN40PHY_dbg_samp_coll_ctrl, dbgctrlword);

	phy_utils_or_phyreg(pi, LCN40PHY_sslpnphy40_soft_reset, 3);

	/* start and stop pointer address */
	W_REG(pi->sh->osh, &pi->regs->PHYREF_SMPL_CLCT_STRPTR, 0);
	W_REG(pi->sh->osh, &pi->regs->PHYREF_SMPL_CLCT_STPPTR, collect->nsamps - 1);

	phy_utils_and_phyreg(pi, LCN40PHY_sslpnphy40_soft_reset, (~3));

	/* Single shot sample capture PHY_CTL = 0x832 */
	W_REG(pi->sh->osh, &pi->regs->psm_phy_hdr_param, samp_coll_ctl);

	PHY_REG_MOD(pi, LCN40PHY, dbg_samp_coll_ctrl, samp_coll_en, 1);

	/* Wait for Sample Collect to complete */
	while (phy_utils_read_phyreg(pi, LCN40PHY_dbg_samp_coll_ctrl) &
		LCN40PHY_dbg_samp_coll_ctrl_samp_coll_en_MASK) {
		/* Check for timeout */
		if (wait_count > collect->timeout) {
			PHY_ERROR(("wl%d: %s: Sample Capture failed to complete\n",
				pi->sh->unit, __FUNCTION__));
			break;
		}
		OSL_DELAY(10);
		wait_count++;
	}

	/* Reset things for next iteration */
	samp_coll_ctl = (R_REG(pi->sh->osh, &pi->regs->psm_phy_hdr_param) & 0xffcf);
	W_REG(pi->sh->osh, &pi->regs->psm_phy_hdr_param, samp_coll_ctl);

	phy_utils_or_phyreg(pi, LCN40PHY_sslpnphy40_soft_reset, 3);

	/* Stop Sample Capture and Play */
	W_REG(pi->sh->osh, &pi->regs->psm_phy_hdr_param, 0x002);

	/* Copy data into a buffer */
	start_ptr = R_REG(pi->sh->osh, &pi->regs->PHYREF_SMPL_CLCT_STRPTR);
	stop_ptr = R_REG(pi->sh->osh, &pi->regs->PHYREF_SMPL_CLCT_STPPTR);
	curr_ptr = R_REG(pi->sh->osh, &pi->regs->PHYREF_SMPL_CLCT_CURPTR) + 1;
	nsamps_out = collect->nsamps;

	while (nsamps_out > 0) {
		if (curr_ptr > stop_ptr) {
		    curr_ptr = start_ptr;
		}

		wlapi_bmac_templateptr_wreg(pi->sh->physhim, curr_ptr << 2);
		data = wlapi_bmac_templatedata_rreg(pi->sh->physhim);
		buf[cnt++] = data;

		curr_ptr += 1;
		nsamps_out -= 1;
	}

	PHY_REG_MOD(pi, LCN40PHY, RFOverride0, internalrfrxpu_ovr, 0);
	PHY_REG_MOD(pi, LCN40PHY, AfeCtrlOvr1, adc_pu_ovr, 0);
	wlc_lcn40phy_rx_gain_override_enable(pi, FALSE);
	wlc_lcn40phy_clear_trsw_override(pi);
	phy_utils_write_phyreg(pi, LCN40PHY_rfoverride2, rf_override_2);
	PHY_REG_MOD(pi, LCN40PHY, agcControl4, c_agc_fsm_en, 1);
	PHY_REG_WRITE(pi, LCN40PHY, sslpnCalibClkEnCtrl, old_sslpnCalibClkEnCtrl);

	if (!suspend)
		wlapi_enable_mac(pi->sh->physhim);

	return BCME_OK;
}
#endif /* #ifdef SAMPLE_COLLECT */

/*
* Compute Rx compensation coeffs
*   -- run IQ est and calculate compensation coefficients
*/
static bool
wlc_lcn40phy_calc_rx_iq_comp(phy_info_t *pi,  uint16 num_samps)
{
#define LCN40PHY_MIN_RXIQ_PWR 2
	bool result;
	uint16 a0_new, b0_new;
	phy_iq_est_t iq_est = {0, 0, 0};
	int32  a, b, temp;
	int16  iq_nbits, qq_nbits, arsh, brsh;
	int32  iq;
	uint32 ii, qq;
#if defined(PHYCAL_CACHING)
	ch_calcache_t *ctx = wlc_phy_get_chanctx(pi, pi->radio_chanspec);
	lcnphy_calcache_t *cache = NULL;

	if (ctx)
		cache = &ctx->u.lcnphy_cache;
#else
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
#endif // endif

	/* Save original c0 & c1 */
	a0_new = ((phy_utils_read_phyreg(pi, LCN40PHY_RxCompcoeffa0) &
	           LCN40PHY_RxCompcoeffa0_a0_MASK) >>
		LCN40PHY_RxCompcoeffa0_a0_SHIFT);
	b0_new = ((phy_utils_read_phyreg(pi, LCN40PHY_RxCompcoeffb0) &
	           LCN40PHY_RxCompcoeffb0_b0_MASK) >>
		LCN40PHY_RxCompcoeffb0_b0_SHIFT);

	PHY_REG_MOD(pi, LCN40PHY, rxfe, bypass_iqcomp, 0);
	PHY_REG_MOD(pi, LCN40PHY, RxIqCoeffCtrl, RxIqComp11bEn, 1);
	/* Zero out comp coeffs and do "one-shot" calibration */
	wlc_lcn40phy_set_rx_iq_comp(pi, 0, 0);

	if (!(result = wlc_lcn40phy_rx_iq_est(pi, num_samps, 32, 0, &iq_est, RXIQEST_TIMEOUT)))
		goto cleanup;

	iq = (int32)iq_est.iq_prod;
	ii = iq_est.i_pwr;
	qq = iq_est.q_pwr;

	/* bounds check estimate info */
	if ((ii + qq) < LCN40PHY_MIN_RXIQ_PWR) {
		PHY_ERROR(("wl%d: %s: RX IQ imbalance estimate power too small\n",
			pi->sh->unit, __FUNCTION__));
		result = FALSE;
		goto cleanup;
	}

	/* Calculate new coeffs */
	iq_nbits = phy_utils_nbits(iq);
	qq_nbits = phy_utils_nbits(qq);

	arsh = 10-(30-iq_nbits);
	if (arsh >= 0) {
		a = (-(iq << (30 - iq_nbits)) + (ii >> (1 + arsh)));
		temp = (int32) (ii >>  arsh);
		if (temp == 0) {
			PHY_ERROR(("Aborting Rx IQCAL! ii=%d, arsh=%d\n", ii, arsh));
			return FALSE;
		}
	} else {
		a = (-(iq << (30 - iq_nbits)) + (ii << (-1 - arsh)));
		temp = (int32) (ii << -arsh);
		if (temp == 0) {
			PHY_ERROR(("Aborting Rx IQCAL! ii=%d, arsh=%d\n", ii, arsh));
			return FALSE;
		}
}
	a /= temp;
	brsh = qq_nbits-31+20;
	if (brsh >= 0) {
		b = (qq << (31-qq_nbits));
		temp = (int32) (ii >>  brsh);
		if (temp == 0) {
			PHY_ERROR(("Aborting Rx IQCAL! ii=%d, brsh=%d\n", ii, brsh));
			return FALSE;
		}
	} else {
		b = (qq << (31-qq_nbits));
		temp = (int32) (ii << -brsh);
		if (temp == 0) {
			PHY_ERROR(("Aborting Rx IQCAL! ii=%d, brsh=%d\n", ii, brsh));
			return FALSE;
		}
	}
	b /= temp;
	b -= a*a;
	b = (int32)phy_utils_sqrt_int((uint32) b);
	b -= (1 << 10);

	if (a > 511)
		a = 511;
	else if (a < -512)
		a = -512;
	a0_new = ((uint16)a & 0x3ff);

	if (b  > 511)
		b = 511;
	else if (b	< -512)
		b = -512;
	b0_new = ((uint16)b & 0x3ff);

cleanup:
	/* Apply new coeffs */
	wlc_lcn40phy_set_rx_iq_comp(pi, a0_new, b0_new);
	/* enabling the hardware override to choose only a0, b0 coeff */
	PHY_REG_MOD(pi, LCN40PHY, RxIqCoeffCtrl, RxIqCrsCoeffOverRide, 1);
	PHY_REG_MOD(pi, LCN40PHY, RxIqCoeffCtrl,
		RxIqCrsCoeffOverRide11b, 1);

#if defined(PHYCAL_CACHING)
	if (ctx) {
		cache->rxiqcal_coeff_a0 = a0_new;
		cache->rxiqcal_coeff_b0 = b0_new;
	}
#else
	pi_lcn->lcnphy_cal_results.rxiqcal_coeff_a0 = a0_new;
	pi_lcn->lcnphy_cal_results.rxiqcal_coeff_b0 = b0_new;
#endif // endif

	return result;

}

/*
 * Play samples from sample play buffer
 */
static void
wlc_lcn40phy_run_samples(phy_info_t *pi,
	uint16 num_samps,
	uint16 num_loops,
	uint16 wait,
	bool iqcalmode)
{
	uint16 playback_status, mask;
	int cnt = 0;

	/* enable clk to txFrontEnd */
	phy_utils_or_phyreg(pi, LCN40PHY_sslpnCalibClkEnCtrl, 0x8080);

	phy_utils_mod_phyreg(pi, LCN40PHY_sampleDepthCount,
		LCN40PHY_sampleDepthCount_DepthCount_MASK,
		(num_samps - 1) << LCN40PHY_sampleDepthCount_DepthCount_SHIFT);

	if (num_loops != 0xffff)
		num_loops--;
	phy_utils_mod_phyreg(pi, LCN40PHY_sampleLoopCount,
		LCN40PHY_sampleLoopCount_LoopCount_MASK,
		num_loops << LCN40PHY_sampleLoopCount_LoopCount_SHIFT);

	phy_utils_mod_phyreg(pi, LCN40PHY_sampleInitWaitCount,
		LCN40PHY_sampleInitWaitCount_InitWaitCount_MASK,
		wait << LCN40PHY_sampleInitWaitCount_InitWaitCount_SHIFT);

	mask = iqcalmode ? LCN40PHY_sampleStatus_iqlocalPlay_MASK :
		LCN40PHY_sampleStatus_NormalPlay_MASK;
	do {
		if (iqcalmode)
			/* Enable calibration */
			PHY_REG_MOD(pi, LCN40PHY, iqloCalCmdGctl, iqlo_cal_en, 1);
		else
			PHY_REG_MOD(pi, LCN40PHY, sampleCmd, start, 1);
		OSL_DELAY(1);
		cnt++;
		playback_status = phy_utils_read_phyreg(pi, LCN40PHY_sampleStatus);
	} while ((cnt < 10) && !(playback_status & mask));

	ASSERT((playback_status & mask));

	if (!iqcalmode)
		wlc_lcn40phy_tx_pu(pi, 1);
}

static uint16 wlc_lcn40phy_papd_index_search(phy_info_t *pi, uint64 final_idx_thresh,
	phy_txcalgains_t *txgains)
{
	uint16 start_index = 0;
	uint16 stop_index = 100;
	uint16 mid_index = 0;
	phytbl_info_t tab;
	uint32 lastval;
	int32 lreal, limag;
	uint64 mag;
	txgains->useindex = 1;

	while (1)
{
		mid_index = (start_index + stop_index) >> 1;
		txgains->index = (uint8) mid_index;
		/* run papd corresponding to the target pwr */
		wlc_lcn40phy_papd_cal(pi, PHY_PAPD_CAL_CW, txgains, 0, 0, 0, 0, 219, 1, 0);

		tab.tbl_id = LCN40PHY_TBL_ID_PAPDCOMPDELTATBL;
		tab.tbl_offset = 63;
		tab.tbl_ptr = &lastval; /* ptr to buf */
		tab.tbl_width = 32;
		tab.tbl_len = 1;        /* # values   */
		wlc_lcnphy_read_table(pi, &tab);
		lreal = lastval & 0x00fff000;
		limag = lastval & 0x00000fff;
		lreal = lreal << 8;
		limag = limag << 20;
		lreal = lreal >> 20;
		limag = limag >> 20;

		mag = (lreal * lreal) + (limag * limag);
		if (mag <= final_idx_thresh)
			stop_index = mid_index;
		else
			start_index = mid_index;
		if ((mag > (final_idx_thresh - 2000)) && (mag < (final_idx_thresh)))
			break;
		if (stop_index - start_index < 2)
			break;
}
	return (mid_index);
}

static void wlc_lcn40phy_papd_calc_capindex(phy_info_t *pi, phy_txcalgains_t *txgains)
{
	uint8 channel;
	uint16 papdAmamThrs = 1680; /* 1.68 *1000 */
	uint16 papdAmamSlope = 10; /* 0.01 *1000 */
	uint16 papdSlopeDivFreq = (papdAmamSlope)/5; /* 2 */
	uint16 lcnphytxindex = 0;
	uint32 Threshold = 0;
	uint32 temp = 0;
	uint64 final_idx_thresh = 0;
	uint freq;
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	freq = phy_utils_channel2freq(CHSPEC_CHANNEL(pi->radio_chanspec));
	channel = CHSPEC_CHANNEL(pi->radio_chanspec);
	/* Papd Based Index Capping calculation */
	if (freq == 2484) {
		Threshold = (128 * (papdAmamThrs + (14 * papdAmamSlope)));
}
	else {
		if (channel == 1)
			Threshold = (128 * papdAmamThrs);
		else
			Threshold = (128 * (papdAmamThrs + ((freq - 2412) * papdSlopeDivFreq)));
	}
	temp =  phy_utils_qdiv_roundup(Threshold, 1000, 0);
	final_idx_thresh = ((uint64)temp*temp);
	lcnphytxindex = wlc_lcn40phy_papd_index_search(pi, final_idx_thresh, txgains);
	if ((lcnphytxindex < 40) || (lcnphytxindex >= 70))
{
		if (lcnphytxindex < 40) {
			Threshold = (128 * (papdAmamThrs -240));
		}
		else {
			if (freq == 2484)
				Threshold = (128 * (papdAmamThrs + 100 + 14*papdAmamSlope));
			else
				Threshold = (128 * (papdAmamThrs + 100 +
					((freq - 2412) * papdSlopeDivFreq)));
}
		temp =  phy_utils_qdiv_roundup(Threshold, 1000, 0);
		final_idx_thresh = ((uint64)temp*temp);
		lcnphytxindex = wlc_lcn40phy_papd_index_search(pi, final_idx_thresh, txgains);

}
	/* cap it to 1dB higher pwr as headroom  */
	pi_lcn->lcnphy_capped_index = lcnphytxindex - 4;

}

static void wlc_lcn40phy_load_txgainwithcappedindex(phy_info_t *pi, bool cap)
{
	uint8 k;
	phytbl_info_t tab;
	uint32 val;
	uint8 indx;
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);

	if (cap) {
		k =  pi_lcn->lcnphy_capped_index;
		tab.tbl_id = LCN40PHY_TBL_ID_TXPWRCTL;
		tab.tbl_width = 32;     /* 32 bit wide  */
		tab.tbl_len = 1;
		tab.tbl_ptr = &val; /* ptr to buf */

		for (indx = 0; indx <=  pi_lcn->lcnphy_capped_index; indx++) {
			/* Cap the GainOffset table */
			tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_GAIN_OFFSET + k;
			wlc_lcn40phy_read_table(pi, &tab);
			tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_GAIN_OFFSET + indx;
			wlc_lcn40phy_write_table(pi, &tab);

			/* Cap the IQOffset table */
			tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_IQ_OFFSET + k;
			wlc_lcn40phy_read_table(pi, &tab);
			tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_IQ_OFFSET + indx;
			wlc_lcn40phy_write_table(pi, &tab);

			/* Cap the RF PWR offset table */
			tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_PWR_OFFSET + k;
			wlc_lcn40phy_read_table(pi, &tab);
			tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_PWR_OFFSET + indx;
			wlc_lcn40phy_write_table(pi, &tab);
		}
	} else {
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			if (LCN40REV_IS(pi->pubpi.phy_rev, 0) ||
				LCN40REV_IS(pi->pubpi.phy_rev, 2) ||
				LCN40REV_IS(pi->pubpi.phy_rev, 4) ||
				LCN40REV_IS(pi->pubpi.phy_rev, 5) ||
				LCN40REV_IS(pi->pubpi.phy_rev, 7))
				wlc_lcn40phy_load_tx_gain_table(pi,
				        dot11lcn40phy_2GHz_gaintable_rev0);
			else if (LCN40REV_IS(pi->pubpi.phy_rev, 1)) {
				if (PHY_EPA_SUPPORT(pi_lcn->ePA)) {
					wlc_lcn40phy_load_tx_gain_table(pi,
					        dot11lcn40phy_2GHz_extPA_gaintable_rev1);
				}
				else {
					wlc_lcn40phy_load_tx_gain_table(pi,
					        dot11lcn40phy_2GHz_gaintable_rev1);
				}
			}
			else if (LCN40REV_IS(pi->pubpi.phy_rev, 3)) {
				if (PHY_EPA_SUPPORT(pi_lcn->ePA)) {
					wlc_lcn40phy_load_tx_gain_table(pi,
					        dot11lcn40phy_2GHz_extPA_gaintable_rev1);
				}
				else {
					wlc_lcn40phy_load_tx_gain_table(pi,
					        dot11lcn40phy_2GHz_gaintable_rev3);
				}
			}
			else if (LCN40REV_IS(pi->pubpi.phy_rev, 6)) {
				wlc_lcn40phy_load_tx_gain_table(pi,
					dot11lcn40phy_2GHz_gaintable_rev6);
			}
		}
		wlc_lcn40phy_load_rfpower(pi);
	}
}

static void
wlc_lcn40phy_adc_init(phy_info_t *pi, phy_adc_mode_t adc_mode, bool cal_mode)
{
	PHY_REG_LIST_START
		RADIO_REG_OR_ENTRY(RADIO_2065_ADC_CFG3, 0x2000)

		PHY_REG_OR_ENTRY(LCN40PHY, AfeCtrlOvr1Val,
			LCN40PHY_AfeCtrlOvr1Val_afe_iqadc_reset_ovr_val_MASK |
			LCN40PHY_AfeCtrlOvr1Val_afe_reset_ov_det_ovr_val_MASK)

		PHY_REG_OR_ENTRY(LCN40PHY, AfeCtrlOvr1,
			LCN40PHY_AfeCtrlOvr1_afe_iqadc_reset_ovr_MASK |
			LCN40PHY_AfeCtrlOvr1_afe_reset_ov_det_ovr_MASK)
	PHY_REG_LIST_EXECUTE(pi);

	OSL_DELAY(100);
	phy_utils_and_phyreg(pi, LCN40PHY_AfeCtrlOvr1Val,
		~(LCN40PHY_AfeCtrlOvr1Val_afe_iqadc_reset_ovr_val_MASK |
		LCN40PHY_AfeCtrlOvr1Val_afe_reset_ov_det_ovr_val_MASK));

	switch (adc_mode) {
	case ADC_20M:
		PHY_REG_LIST_START
			RADIO_REG_MOD_ENTRY(RADIO_2065_ADC_CFG4, 0x400, 0)
			RADIO_REG_WRITE_ENTRY(RADIO_2065_ADC_BIAS, 2)
			PHY_REG_MOD_RAW_ENTRY(LCN40PHY_AfeCtrlOvr1Val,
				LCN40PHY_AfeCtrlOvr1Val_afe_iqadc_lf_order_ovr_val_MASK |
				LCN40PHY_AfeCtrlOvr1Val_afe_iqadc_lf_lowif_dis_ovr_val_MASK |
				LCN40PHY_AfeCtrlOvr1Val_afe_iqadc_wl_lp_ovr_val_MASK |
				LCN40PHY_AfeCtrlOvr1Val_afe_iqadc_flash_only_ovr_val_MASK |
				LCN40PHY_AfeCtrlOvr1Val_adc_pu_ovr_val_MASK,
				0x1f << LCN40PHY_AfeCtrlOvr1Val_adc_pu_ovr_val_SHIFT)
			PHY_REG_OR_ENTRY(LCN40PHY, AfeCtrlOvr1,
				LCN40PHY_AfeCtrlOvr1_afe_iqadc_lf_order_ovr_MASK |
				LCN40PHY_AfeCtrlOvr1_afe_iqadc_lf_lowif_dis_ovr_MASK |
				LCN40PHY_AfeCtrlOvr1_afe_iqadc_wl_lp_ovr_MASK |
				LCN40PHY_AfeCtrlOvr1_afe_iqadc_flash_only_ovr_MASK |
				LCN40PHY_AfeCtrlOvr1_adc_pu_ovr_MASK)
		PHY_REG_LIST_EXECUTE(pi);
		break;
	case ADC_40M:
		PHY_REG_LIST_START
			RADIO_REG_OR_ENTRY(RADIO_2065_ADC_CFG4, 0x400)
			RADIO_REG_WRITE_ENTRY(RADIO_2065_ADC_BIAS, 0)
			PHY_REG_MOD_RAW_ENTRY(LCN40PHY_AfeCtrlOvr1Val,
				LCN40PHY_AfeCtrlOvr1Val_afe_iqadc_lf_order_ovr_val_MASK |
				LCN40PHY_AfeCtrlOvr1Val_afe_iqadc_lf_lowif_dis_ovr_val_MASK |
				LCN40PHY_AfeCtrlOvr1Val_afe_iqadc_wl_lp_ovr_val_MASK |
				LCN40PHY_AfeCtrlOvr1Val_afe_iqadc_flash_only_ovr_val_MASK |
				LCN40PHY_AfeCtrlOvr1Val_adc_pu_ovr_val_MASK,
				0x1f << LCN40PHY_AfeCtrlOvr1Val_adc_pu_ovr_val_SHIFT)
			PHY_REG_OR_ENTRY(LCN40PHY, AfeCtrlOvr1,
				LCN40PHY_AfeCtrlOvr1_afe_iqadc_lf_order_ovr_MASK |
				LCN40PHY_AfeCtrlOvr1_afe_iqadc_lf_lowif_dis_ovr_MASK |
				LCN40PHY_AfeCtrlOvr1_afe_iqadc_wl_lp_ovr_MASK |
				LCN40PHY_AfeCtrlOvr1_afe_iqadc_flash_only_ovr_MASK |
				LCN40PHY_AfeCtrlOvr1_adc_pu_ovr_MASK)
		PHY_REG_LIST_EXECUTE(pi);
		break;
	case ADC_20M_LP:
		PHY_REG_LIST_START
			RADIO_REG_MOD_ENTRY(RADIO_2065_ADC_CFG4, 0x400, 0)
			RADIO_REG_WRITE_ENTRY(RADIO_2065_ADC_BIAS, 2)
			PHY_REG_MOD_RAW_ENTRY(LCN40PHY_AfeCtrlOvr1Val,
			    LCN40PHY_AfeCtrlOvr1Val_afe_iqadc_lf_order_ovr_val_MASK |
			    LCN40PHY_AfeCtrlOvr1Val_afe_iqadc_lf_lowif_dis_ovr_val_MASK |
			    LCN40PHY_AfeCtrlOvr1Val_afe_iqadc_wl_lp_ovr_val_MASK |
			    LCN40PHY_AfeCtrlOvr1Val_afe_iqadc_flash_only_ovr_val_MASK |
			    LCN40PHY_AfeCtrlOvr1Val_adc_pu_ovr_val_MASK,
			    (1 << LCN40PHY_AfeCtrlOvr1Val_afe_iqadc_lf_order_ovr_val_SHIFT) |
			    (1 << LCN40PHY_AfeCtrlOvr1Val_afe_iqadc_lf_lowif_dis_ovr_val_SHIFT) |
			    (1 << LCN40PHY_AfeCtrlOvr1Val_afe_iqadc_wl_lp_ovr_val_SHIFT) |
			    (0x19 << LCN40PHY_AfeCtrlOvr1Val_adc_pu_ovr_val_SHIFT))
			PHY_REG_OR_ENTRY(LCN40PHY, AfeCtrlOvr1,
				LCN40PHY_AfeCtrlOvr1_afe_iqadc_lf_order_ovr_MASK |
				LCN40PHY_AfeCtrlOvr1_afe_iqadc_lf_lowif_dis_ovr_MASK |
				LCN40PHY_AfeCtrlOvr1_afe_iqadc_wl_lp_ovr_MASK |
				LCN40PHY_AfeCtrlOvr1_afe_iqadc_flash_only_ovr_MASK |
				LCN40PHY_AfeCtrlOvr1_adc_pu_ovr_MASK)
		PHY_REG_LIST_EXECUTE(pi);
		break;
	case ADC_40M_LP:
		PHY_REG_LIST_START
			RADIO_REG_OR_ENTRY(RADIO_2065_ADC_CFG4, 0x400)
			RADIO_REG_WRITE_ENTRY(RADIO_2065_ADC_BIAS, 0)
			PHY_REG_MOD_RAW_ENTRY(LCN40PHY_AfeCtrlOvr1Val,
			    LCN40PHY_AfeCtrlOvr1Val_afe_iqadc_lf_order_ovr_val_MASK |
			    LCN40PHY_AfeCtrlOvr1Val_afe_iqadc_lf_lowif_dis_ovr_val_MASK |
			    LCN40PHY_AfeCtrlOvr1Val_afe_iqadc_wl_lp_ovr_val_MASK |
			    LCN40PHY_AfeCtrlOvr1Val_afe_iqadc_flash_only_ovr_val_MASK |
			    LCN40PHY_AfeCtrlOvr1Val_adc_pu_ovr_val_MASK,
			    (1 << LCN40PHY_AfeCtrlOvr1Val_afe_iqadc_lf_order_ovr_val_SHIFT) |
			    (1 << LCN40PHY_AfeCtrlOvr1Val_afe_iqadc_lf_lowif_dis_ovr_val_SHIFT) |
			    (1 << LCN40PHY_AfeCtrlOvr1Val_afe_iqadc_wl_lp_ovr_val_SHIFT) |
			    (0x19 << LCN40PHY_AfeCtrlOvr1Val_adc_pu_ovr_val_SHIFT))
			PHY_REG_OR_ENTRY(LCN40PHY, AfeCtrlOvr1,
				LCN40PHY_AfeCtrlOvr1_afe_iqadc_lf_order_ovr_MASK |
				LCN40PHY_AfeCtrlOvr1_afe_iqadc_lf_lowif_dis_ovr_MASK |
				LCN40PHY_AfeCtrlOvr1_afe_iqadc_wl_lp_ovr_MASK |
				LCN40PHY_AfeCtrlOvr1_afe_iqadc_flash_only_ovr_MASK |
				LCN40PHY_AfeCtrlOvr1_adc_pu_ovr_MASK)
		PHY_REG_LIST_EXECUTE(pi);
		break;
	case ADC_FLASHONLY:
		PHY_REG_LIST_START
			RADIO_REG_OR_ENTRY(RADIO_2065_ADC_CFG4, 0x400)
			RADIO_REG_WRITE_ENTRY(RADIO_2065_ADC_BIAS, 0)
			PHY_REG_MOD_RAW_ENTRY(LCN40PHY_AfeCtrlOvr1Val,
				LCN40PHY_AfeCtrlOvr1Val_afe_iqadc_lf_order_ovr_val_MASK |
				LCN40PHY_AfeCtrlOvr1Val_afe_iqadc_lf_lowif_dis_ovr_val_MASK |
				LCN40PHY_AfeCtrlOvr1Val_afe_iqadc_wl_lp_ovr_val_MASK |
				LCN40PHY_AfeCtrlOvr1Val_afe_iqadc_flash_only_ovr_val_MASK |
				LCN40PHY_AfeCtrlOvr1Val_adc_pu_ovr_val_MASK,
				(1 << LCN40PHY_AfeCtrlOvr1Val_afe_iqadc_flash_only_ovr_val_SHIFT) |
				(1 << LCN40PHY_AfeCtrlOvr1Val_adc_pu_ovr_val_SHIFT))
			PHY_REG_OR_ENTRY(LCN40PHY, AfeCtrlOvr1,
				LCN40PHY_AfeCtrlOvr1_afe_iqadc_lf_order_ovr_MASK |
				LCN40PHY_AfeCtrlOvr1_afe_iqadc_lf_lowif_dis_ovr_MASK |
				LCN40PHY_AfeCtrlOvr1_afe_iqadc_wl_lp_ovr_MASK |
				LCN40PHY_AfeCtrlOvr1_afe_iqadc_flash_only_ovr_MASK |
				LCN40PHY_AfeCtrlOvr1_adc_pu_ovr_MASK)
		PHY_REG_LIST_EXECUTE(pi);
		break;
	default:
		PHY_ERROR(("wrong ADC mode %d\n", adc_mode));
	}

	if (cal_mode) {
		PHY_REG_LIST_START
			RADIO_REG_OR_ENTRY(RADIO_2065_XTAL_CFG1, 0x4000)
			RADIO_REG_OR_ENTRY(RADIO_2065_ADC_CFG3, 0x1000)
			RADIO_REG_MOD_ENTRY(RADIO_2065_ADC_CFG4, 0xff, 0x5f)
			RADIO_REG_OR_ENTRY(RADIO_2065_OVR2, 0x2400)
		PHY_REG_LIST_EXECUTE(pi);
		OSL_DELAY(300);
		PHY_REG_LIST_START
			RADIO_REG_MOD_ENTRY(RADIO_2065_ADC_CFG4, 0xff, 0)
			RADIO_REG_AND_ENTRY(RADIO_2065_OVR2, ~0x2400)
			RADIO_REG_AND_ENTRY(RADIO_2065_XTAL_CFG1, ~0x4000)
			PHY_REG_OR_ENTRY(LCN40PHY, AfeCtrlOvr1,
				LCN40PHY_AfeCtrlOvr1_afe_reset_ov_det_ovr_MASK)
		PHY_REG_LIST_EXECUTE(pi);
		OSL_DELAY(100);
		phy_utils_and_phyreg(pi, LCN40PHY_AfeCtrlOvr1,
			~LCN40PHY_AfeCtrlOvr1_afe_reset_ov_det_ovr_MASK);
	} else {
		phy_utils_and_radioreg(pi, RADIO_2065_ADC_CFG3, ~0x1000);
		phy_utils_mod_radioreg(pi, RADIO_2065_ADC_CFG4, 0x0c, 0);
	}

	phy_utils_and_phyreg(pi, LCN40PHY_AfeCtrlOvr1,
		~(LCN40PHY_AfeCtrlOvr1_afe_iqadc_reset_ovr_MASK |
		LCN40PHY_AfeCtrlOvr1_afe_reset_ov_det_ovr_MASK |
		LCN40PHY_AfeCtrlOvr1_adc_pu_ovr_MASK));
}

#if defined(LP_P2P_SOFTAP) || defined(WL_LPC)
#define LPC_MIN_IDX 38

#define LPC_TOT_IDX (LPC_MIN_IDX + 1)
#define LCN40PHY_TX_PWR_CTRL_MACLUT_MAX_ENTRIES	64
#define LCN40PHY_TX_PWR_CTRL_MACLUT_WIDTH	8

#ifdef WL_LPC
static uint8 lpc_pwr_level[LPC_TOT_IDX] =
	{0, 2, 4, 6, 8, 10,
	12, 14, 16, 18, 20,
	22, 24, 26, 28, 30,
	32, 34, 36, 38, 40,
	42, 44, 46, 48, 50,
	52, 54, 56, 58, 60,
	61, 62, 63, 64, 65,
	66, 67, 68};
#endif /* WL_LPC */

static void
wlc_lcn40phy_lpc_write_maclut(phy_info_t *pi)
{
	phytbl_info_t tab;

#if defined(LP_P2P_SOFTAP)
	uint8 i;
	/* Assign values from 0 to 63 qdB for now */
	for (i = 0; i < LCNPHY_TX_PWR_CTRL_MACLUT_LEN; i++)
		pwr_lvl_qdB[i] = i;
	tab.tbl_ptr = pwr_lvl_qdB;
	tab.tbl_len = LCNPHY_TX_PWR_CTRL_MACLUT_LEN;

#elif defined(WL_LPC)

	/* If not enabled, no need to clear out the table, just quit */
	if (!pi->lpc_algo)
		return;
	tab.tbl_ptr = lpc_pwr_level;
	tab.tbl_len = LPC_TOT_IDX;
#endif /* WL_LPC */

	tab.tbl_id = LCN40PHY_TBL_ID_TXPWRCTL;
	tab.tbl_width = LCN40PHY_TX_PWR_CTRL_MACLUT_WIDTH;
	tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_MAC_OFFSET;

	/* Write to it */
	wlc_lcn40phy_write_table(pi, &tab);
}

#ifdef WL_LPC
uint8
wlc_lcn40phy_lpc_getminidx(void)
{
	return LPC_MIN_IDX;
}

void
wlc_lcn40phy_lpc_setmode(phy_info_t *pi, bool enable)
{
	if (enable)
		wlc_lcn40phy_lpc_write_maclut(pi);
}

uint8
wlc_lcn40phy_lpc_getoffset(uint8 index)
{
	return index;
	/* return lpc_pwr_level[index]; for PHYs which expect the actual offset
	 * for example, HT 4331.
	 */
}

uint8
wlc_lcn40phy_lpc_get_txcpwrval(uint16 phytxctrlword)
{
	return (phytxctrlword & PHY_TXC_PWR_MASK) >> PHY_TXC_PWR_SHIFT;
}

void
wlc_lcn40phy_lpc_set_txcpwrval(uint16 *phytxctrlword, uint8 txcpwrval)
{
	*phytxctrlword = (*phytxctrlword & ~PHY_TXC_PWR_MASK) |
		(txcpwrval << PHY_TXC_PWR_SHIFT);
	return;
}

#ifdef WL_LPC_DEBUG
uint8 *
wlc_lcn40phy_lpc_get_pwrlevelptr(void)
{
	return lpc_pwr_level;
}
#endif // endif
#endif /* WL_LPC */
#endif /* WL_LPC || LP_P2P_SOFTAP */

/* ********************** NOISE CAL ********************************* */

static int8 wlc_lcn40phy_noise_log(uint32 x);

#if PHY_NOISE_DBG_HISTORY > 0

static uint16 wlc_lcn40phy_noise_get_reg(phy_info_t *pi, uint16 reg);

#define PHY_NOISE_DBG_CNT           0
#define PHY_NOISE_DBG_INIT_CNT      1
#define PHY_NOISE_DBG_STARTS        2
#define PHY_NOISE_DBG_CH            3
#define PHY_NOISE_DBG_STATE         4
#define PHY_NOISE_DBG_NOISE         5
#define PHY_NOISE_DBG_NOISE_GAIN    6
#define PHY_NOISE_DBG_NOISE_CNT     7
#define PHY_NOISE_DBG_ADJ           8
#define PHY_NOISE_DBG_REF           9
#define PHY_NOISE_DBG_ADJ_HG       10
#define PHY_NOISE_DBG_ADJ_PO       11
#define PHY_NOISE_DBG_HG_ADJ_CNT   12
#define PHY_NOISE_DBG_HG_ADJ_TIME  13
#define PHY_NOISE_DBG_PO_ADJ_CNT   14
#define PHY_NOISE_DBG_PO_ADJ_TIME  15
#define PHY_NOISE_DBG_CALLBACK_CNT 16
#define PHY_NOISE_DBG_CALLBACK_T   17
#define PHY_NOISE_DBG_TAINT_CNT    18
#define PHY_NOISE_DBG_ADJ_MIN      19
#define PHY_NOISE_DBG_ADJ_MAX      20
#define PHY_NOISE_DBG_ADJ_CNT      21
#define PHY_NOISE_DBG_UCODE_CMD    22
#define PHY_NOISE_DBG_UCODE_RSP    23
#define PHY_NOISE_DBG_ADJ_T0       24
#define PHY_NOISE_DBG_ADJ_T1       25
#define PHY_NOISE_DBG_UCODE_DATA_X 26
#define PHY_NOISE_DBG_UCODE_DATA_C 27
#define PHY_NOISE_DBG_UCODE_DATA_L 28
#define PHY_NOISE_DBG_UCODE_DATA_T 29
#define PHY_NOISE_DBG_UCODE_DATA_0 30
#define PHY_NOISE_DBG_UCODE_DATA_N (PHY_NOISE_DBG_UCODE_DATA_0 +\
	k_noise_cal_ucode_data_size - 1)
#define PHY_NOISE_DBG_UCODE_SMPLS (PHY_NOISE_DBG_UCODE_DATA_N + 1)
#define PHY_NOISE_DBG_LAST   (PHY_NOISE_DBG_UCODE_DATA_N + PHY_NOISE_DBG_UCODE_NUM_SMPLS)

extern int noise_cal_dbg_check[PHY_NOISE_DBG_DATA_LEN - PHY_NOISE_DBG_LAST];

static void
wlc_lcn40phy_noise_log_init(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	int8 idx = pi_lcn->noise.dbg_idx;

	pi_lcn->noise.dbg_info[idx][PHY_NOISE_DBG_INIT_CNT]++;
}

static void
wlc_lcn40phy_noise_log_start(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	int8 idx = pi_lcn->noise.dbg_idx;

	if (pi_lcn->noise.state !=  k_noise_adj_state)
		return;
	pi_lcn->noise.dbg_info[idx][PHY_NOISE_DBG_STARTS]++;
	pi_lcn->noise.dbg_info[idx][PHY_NOISE_DBG_CH] =
	  (uint16)CHSPEC_CHANNEL(pi->radio_chanspec);
	pi_lcn->noise.dbg_info[idx][PHY_NOISE_DBG_REF] = pi_lcn->noise.ref;
	pi_lcn->noise.dbg_info[idx][PHY_NOISE_DBG_CALLBACK_CNT] = 0;
	pi_lcn->noise.dbg_info[idx][PHY_NOISE_DBG_CALLBACK_T] = 0;
	pi_lcn->noise.dbg_info[idx][PHY_NOISE_DBG_TAINT_CNT] = 0;
	pi_lcn->noise.dbg_info[idx][PHY_NOISE_DBG_HG_ADJ_CNT] = 0;
	pi_lcn->noise.dbg_info[idx][PHY_NOISE_DBG_HG_ADJ_TIME] = 0;
	pi_lcn->noise.dbg_info[idx][PHY_NOISE_DBG_PO_ADJ_CNT] = 0;
	pi_lcn->noise.dbg_info[idx][PHY_NOISE_DBG_PO_ADJ_TIME] = 0;
	pi_lcn->noise.dbg_info[idx][PHY_NOISE_DBG_ADJ_CNT] = 0;
	pi_lcn->noise.dbg_info[idx][PHY_NOISE_DBG_NOISE] = 0xdead;
	pi_lcn->noise.dbg_info[idx][PHY_NOISE_DBG_ADJ] = 0xdead;
	pi_lcn->noise.dbg_info[idx][PHY_NOISE_DBG_ADJ_HG] = 0xdead;
	pi_lcn->noise.dbg_info[idx][PHY_NOISE_DBG_ADJ_PO] = 0xdead;
	pi_lcn->noise.start_time =
	  (uint16)((R_REG(pi->sh->osh, &pi->regs->tsf_timerlow) >> 10) & 0xffff);
	pi_lcn->noise.per_start_time = 0;
}

static void
wlc_lcn40phy_noise_log_per_start(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);

	pi_lcn->noise.per_start_time = (uint16)(R_REG(pi->sh->osh, &pi->regs->tsf_timerlow) >> 10)
	        & 0xffff;
}

static void
wlc_lcn40phy_noise_log_adj(phy_info_t *pi, uint16 noise, int16 adj,
	int16 gain, int16 po, bool gain_change, bool po_change)
{
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	int8 idx = pi_lcn->noise.dbg_idx;
	uint16 timestamp, timestamp_ch, timestamp_per;

	if (pi_lcn->noise.state !=  k_noise_adj_state)
		return;
	timestamp = (uint16)(R_REG(pi->sh->osh, &pi->regs->tsf_timerlow) >> 10) & 0xffff;
	timestamp_ch = (timestamp - pi_lcn->noise.start_time);
	if (pi_lcn->noise.per_start_time == 0)
	  timestamp_per = 0;
	else
	  timestamp_per = (timestamp - pi_lcn->noise.per_start_time);
	pi_lcn->noise.dbg_info[idx][PHY_NOISE_DBG_NOISE] = noise;
	pi_lcn->noise.dbg_info[idx][PHY_NOISE_DBG_ADJ] = (uint16)adj;
	pi_lcn->noise.dbg_info[idx][PHY_NOISE_DBG_ADJ_HG] = (uint16)gain;
	pi_lcn->noise.dbg_info[idx][PHY_NOISE_DBG_ADJ_PO] = (uint16)po;
	if (gain_change) {
		pi_lcn->noise.dbg_info[idx][PHY_NOISE_DBG_HG_ADJ_CNT]++;
		pi_lcn->noise.dbg_info[idx][PHY_NOISE_DBG_HG_ADJ_TIME] = timestamp_ch;
	}
	if (po_change) {
		pi_lcn->noise.dbg_info[idx][PHY_NOISE_DBG_PO_ADJ_CNT]++;
		pi_lcn->noise.dbg_info[idx][PHY_NOISE_DBG_PO_ADJ_TIME] = timestamp_ch;
	}
	if (pi_lcn->noise.dbg_info[idx][PHY_NOISE_DBG_ADJ_CNT] > 0) {
	  if (adj < pi_lcn->noise.dbg_adj_min)
	    pi_lcn->noise.dbg_adj_min = adj;
	  if (adj > pi_lcn->noise.dbg_adj_max)
	    pi_lcn->noise.dbg_adj_max = adj;
	} else {
	  pi_lcn->noise.dbg_adj_min = adj;
	  pi_lcn->noise.dbg_adj_max = adj;
	}
	pi_lcn->noise.dbg_info[idx][PHY_NOISE_DBG_ADJ_MIN] = (uint16)pi_lcn->noise.dbg_adj_min;
	pi_lcn->noise.dbg_info[idx][PHY_NOISE_DBG_ADJ_MAX] = (uint16)pi_lcn->noise.dbg_adj_max;
	pi_lcn->noise.dbg_info[idx][PHY_NOISE_DBG_ADJ_CNT]++;
	pi_lcn->noise.dbg_info[idx][PHY_NOISE_DBG_ADJ_T0] = timestamp_ch;
	pi_lcn->noise.dbg_info[idx][PHY_NOISE_DBG_ADJ_T1] = timestamp_per;
}

static void
wlc_lcn40phy_noise_log_callback(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	int8 idx = pi_lcn->noise.dbg_idx;

	if (pi_lcn->noise.state !=  k_noise_adj_state)
		return;
	pi_lcn->noise.dbg_info[idx][PHY_NOISE_DBG_CNT]++;
	pi_lcn->noise.dbg_info[idx][PHY_NOISE_DBG_CALLBACK_CNT]++;
	pi_lcn->noise.dbg_info[idx][PHY_NOISE_DBG_CALLBACK_T] =
		(uint16)((R_REG(pi->sh->osh, &pi->regs->tsf_timerlow) >> 10) & 0xffff) -
	pi_lcn->noise.start_time;
}

static void
wlc_lcn40phy_noise_log_tainted(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	int8 idx = pi_lcn->noise.dbg_idx;

	if (pi_lcn->noise.state !=  k_noise_adj_state)
		return;
	pi_lcn->noise.dbg_info[idx][PHY_NOISE_DBG_TAINT_CNT]++;
}

static void
wlc_lcn40phy_noise_log_ucode_data_reset(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	int8 idx = pi_lcn->noise.dbg_idx;

	if (pi_lcn->noise.state !=  k_noise_adj_state)
		return;

	pi_lcn->noise.dbg_info[idx][PHY_NOISE_DBG_UCODE_DATA_C] = 0;
	pi_lcn->noise.dbg_info[idx][PHY_NOISE_DBG_UCODE_DATA_X] = 0;
}

static void
wlc_lcn40phy_noise_log_ucode_data_ok(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	int8 idx = pi_lcn->noise.dbg_idx;

	if (pi_lcn->noise.state !=  k_noise_adj_state)
		return;
	pi_lcn->noise.dbg_info[idx][PHY_NOISE_DBG_NOISE_GAIN] =
	  PHY_REG_READ(pi, LCN40PHY, agcControl12, crs_gain_high_gain_db_40mhz);
	pi_lcn->noise.dbg_info[idx][PHY_NOISE_DBG_NOISE_CNT] = pi_lcn->noise.ucode_data_ok_cnt;
}

static void
wlc_lcn40phy_noise_log_ucode_data(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	int8 idx = pi_lcn->noise.dbg_idx;
	int8 i, ucode_idx;

	if (pi_lcn->noise.state !=  k_noise_adj_state)
		return;
	pi_lcn->noise.dbg_info[idx][PHY_NOISE_DBG_UCODE_DATA_C]++;
	pi_lcn->noise.dbg_info[idx][PHY_NOISE_DBG_UCODE_DATA_L] = pi_lcn->noise.ucode_data_len;
	ucode_idx = pi_lcn->noise.ucode_data_idx;
	for (i = 0; i < k_noise_cal_ucode_data_size; i++) {
		pi_lcn->noise.dbg_info[idx][PHY_NOISE_DBG_UCODE_DATA_0 + i] =
		  (uint16)wlc_lcn40phy_noise_log(pi_lcn->noise.ucode_data[ucode_idx++]);
		if (ucode_idx >= k_noise_cal_ucode_data_size)
		  ucode_idx = 0;
	}
}

static void
wlc_lcn40phy_noise_log_ucode_samples(phy_info_t *pi)
{
#if PHY_NOISE_DBG_UCODE_NUM_SMPLS > 0
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	int8 i, idx = pi_lcn->noise.dbg_idx;

	pi_lcn->noise.dbg_info[idx][PHY_NOISE_DBG_UCODE_SMPLS] = 0xbeef;
	pi_lcn->noise.dbg_info[idx][PHY_NOISE_DBG_UCODE_SMPLS+1] =
	  (uint16)wlc_lcn40phy_noise_log(pi_lcn->noise.ucode_data[pi_lcn->noise.ucode_data_idx]);
	pi_lcn->noise.dbg_info[idx][PHY_NOISE_DBG_UCODE_SMPLS+2] =
	  wlc_lcn40phy_noise_get_reg(pi, M_55f_REG_VAL);

	for (i = 3; i < PHY_NOISE_DBG_UCODE_NUM_SMPLS; i++) {
		int16 x_i, x_q;

		x_i = pi_lcn->noise.dbg_samples[2*i];
		x_q = pi_lcn->noise.dbg_samples[2*i + 1];
		x_i = x_i >> 2;
		x_q = x_q >> 2;
		if (x_i > 127)
		  x_i = 127;
		else if (x_i < -128)
		  x_i = -128;
		if (x_q > 127)
		  x_q = 127;
		else if (x_q < -128)
		  x_q = -128;
		pi_lcn->noise.dbg_info[idx][PHY_NOISE_DBG_UCODE_SMPLS+i] =
		  (((x_q & 0xff)<<8) | (x_i & 0xff));
	}
#endif /* PHY_NOISE_DBG_UCODE_NUM_SMPLS > 0 */
}

#ifdef NOISE_CAL_CLUSTER_CHECK
static void
wlc_lcn40phy_noise_log_bad_ucode_data(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	int8 idx = pi_lcn->noise.dbg_idx;

	if (pi_lcn->noise.state !=  k_noise_adj_state)
		return;
	pi_lcn->noise.dbg_info[idx][PHY_NOISE_DBG_UCODE_DATA_X]++;
}
#endif // endif

static void
wlc_lcn40phy_noise_log_ucode_data_insert_time(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	int8 idx = pi_lcn->noise.dbg_idx;

	if (pi_lcn->noise.state !=  k_noise_adj_state)
		return;

	pi_lcn->noise.dbg_info[idx][PHY_NOISE_DBG_UCODE_DATA_T] =
	  (uint16)((R_REG(pi->sh->osh, &pi->regs->tsf_timerlow) >> 10) & 0xffff) -
	  pi_lcn->noise.start_time;
}

static void
wlc_lcn40phy_noise_log_state(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	int8 idx = pi_lcn->noise.dbg_idx;

	if (pi_lcn->noise.state !=  k_noise_adj_state)
		return;
	pi_lcn->noise.dbg_info[idx][PHY_NOISE_DBG_STATE] =
	  (uint16)((pi_lcn->noise.enable ? 1 : 0) << 2) |
	  (uint16)((pi_lcn->noise.state & 0xf) << 4) |
	  ((uint16)pi_lcn->noise.update_step << 8);

	pi_lcn->noise.dbg_info[idx][PHY_NOISE_DBG_UCODE_CMD] =
	  (uint16)wlc_lcn40phy_noise_get_reg(pi, M_NOISE_CAL_CMD_LCN40PHY);
	pi_lcn->noise.dbg_info[idx][PHY_NOISE_DBG_UCODE_RSP] =
	  (uint16)wlc_lcn40phy_noise_get_reg(pi, M_NOISE_CAL_RSP_LCN40PHY);
}

static void
wlc_lcn40phy_noise_reset_log(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);

	pi_lcn->noise.dbg_idx = 0;
	pi_lcn->noise.dbg_dump_idx = 0;
	pi_lcn->noise.dbg_dump_sub_idx = 0;
	bzero((void*)pi_lcn->noise.dbg_info, sizeof(pi_lcn->noise.dbg_info));
	pi_lcn->noise.dbg_dump_cmd = 0;
}

static void
wlc_lcn40phy_noise_advance_log(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	int8 idx = pi_lcn->noise.dbg_idx;

	if (pi_lcn->noise.state !=  k_noise_adj_state)
		return;

	wlc_lcn40phy_noise_log_state(pi);

	idx = (idx + 1) % PHY_NOISE_DBG_HISTORY;
	if (idx != pi_lcn->noise.dbg_idx) {
		bcopy((void*)&pi_lcn->noise.dbg_info[pi_lcn->noise.dbg_idx],
			(void*)&pi_lcn->noise.dbg_info[idx],
			(sizeof(pi_lcn->noise.dbg_info)/PHY_NOISE_DBG_HISTORY));
		pi_lcn->noise.dbg_idx = idx;
	}
}

static void
wlc_lcn40phy_noise_dump_log(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	int8 idx = (pi_lcn->noise.dbg_idx + 1) % PHY_NOISE_DBG_HISTORY;
	int8 sub_idx, i, j;
	uint16 dbg[4];

	wlc_lcn40phy_noise_log_state(pi);

	char c = '>';
	j = 0;
	while (j++ < PHY_NOISE_DBG_HISTORY) {
		i = sub_idx = 0;
		while (sub_idx < 4*(((PHY_NOISE_DBG_LAST+1)/4)+1)) {
			if (sub_idx < (PHY_NOISE_DBG_LAST+1))
			  dbg[i] = pi_lcn->noise.dbg_info[idx][sub_idx];
			else
			  dbg[i] = 0xdead;
			if (i++ == 3) {
			  PHY_INFORM(
			  ("wl%d: %s: %c %2d %2d : %5d 0x%4x : %5d 0x%4x : %5d 0x%4x : %5d 0x%4x\n",
			  pi->sh->unit, __FUNCTION__, c, idx, sub_idx-3,
			  dbg[0], dbg[0], dbg[1], dbg[1], dbg[2], dbg[2], dbg[3], dbg[3]));
			  i = 0;
			}
			sub_idx++;
		}
		if (idx & 1)
		  c = '+';
		else
		  c = '-';
		idx = (idx + 1) % PHY_NOISE_DBG_HISTORY;
	}
}

static bool
wlc_lcn40phy_noise_log_dump_active(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);

	return pi_lcn->noise.dbg_dump_cmd;
}

static uint32
wlc_lcn40phy_noise_log_data(phy_info_t *pi)
{
	uint32 datum;
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	int8 idx, sub_idx;

	if (!pi_lcn->noise.dbg_dump_cmd)
		return 0xdeadbeef;

	wlc_lcn40phy_noise_log_state(pi);

	idx = pi_lcn->noise.dbg_dump_idx;
	sub_idx = pi_lcn->noise.dbg_dump_sub_idx;
	if (sub_idx < PHY_NOISE_DBG_DATA_LEN)
	  datum  = (uint32)pi_lcn->noise.dbg_info[idx][sub_idx];
	else
	  datum  = 0xdead;
	datum |= (uint32)(sub_idx << 16);
	datum |= (uint32)(idx << 24);
	sub_idx++;
	if (sub_idx > PHY_NOISE_DBG_LAST)
	{
		sub_idx = 0;
		idx--;
		if (idx < 0)
			idx += PHY_NOISE_DBG_HISTORY;
	}
	pi_lcn->noise.dbg_dump_idx = idx;
	pi_lcn->noise.dbg_dump_sub_idx = sub_idx;

	return datum;
}

static bool
wlc_lcn40phy_noise_log_ioctl(phy_info_t *pi, uint32 flag)
{
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	bool retval = FALSE;

	if (flag & 1)
	{
		pi_lcn->noise.dbg_dump_cmd = 1;
		pi_lcn->noise.dbg_dump_idx = pi_lcn->noise.dbg_idx;
		pi_lcn->noise.dbg_dump_sub_idx = 0;
		retval = TRUE;
	}
	else if (flag & 2)
	{
		wlc_lcn40phy_noise_dump_log(pi);
		retval = TRUE;
	}
	else if (flag & 4)
	{
		pi_lcn->noise.dbg_dump_cmd = 0;
		retval = TRUE;
	}
	return retval;
}

extern void  wlc_lcn40phy_per_start(wlc_phy_t *ppi);
extern void  wlc_lcn40phy_per_stop(wlc_phy_t *ppi);

extern void
wlc_lcn40phy_per_start(wlc_phy_t *ppi)
{
	phy_info_t *pi = (phy_info_t *)ppi;

	wlc_lcn40phy_noise_log_per_start(pi);
}

extern void
wlc_lcn40phy_per_stop(wlc_phy_t *ppi)
{
}

#else /* PHY_NOISE_DBG_HISTORY > 0 */

#define wlc_lcn40phy_noise_log_init(a)
#define wlc_lcn40phy_noise_log_start(a)
#define wlc_lcn40phy_noise_log_ucode_data_ok(a)
#define wlc_lcn40phy_noise_log_adj(a, b, c, d, e, f, g)
#define wlc_lcn40phy_noise_log_callback(a)
#define wlc_lcn40phy_noise_log_tainted(a)
#define wlc_lcn40phy_noise_log_ucode_data_reset(a)
#define wlc_lcn40phy_noise_log_ucode_data(a)
#define wlc_lcn40phy_noise_log_bad_ucode_data(a)
#define wlc_lcn40phy_noise_log_ucode_data_insert_time(a)
#define wlc_lcn40phy_noise_log_state(a)
#define wlc_lcn40phy_noise_reset_log(a)
#define wlc_lcn40phy_noise_advance_log(a)
#define wlc_lcn40phy_noise_dump_log(a)
#define wlc_lcn40phy_noise_log_dump_active(a) FALSE
#define wlc_lcn40phy_noise_log_data(a) 0
#define wlc_lcn40phy_noise_log_ioctl(a, b) ((b))
#define wlc_lcn40phy_noise_log_ucode_samples(a)

#endif /* PHY_NOISE_DBG_HISTORY > 0 */

static void
wlc_lcn40phy_noise_restore_phy_regs(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	PHY_REG_MOD(pi, LCN40PHY, agcControl12, crs_gain_high_gain_db_40mhz,
		pi_lcn->noise.high_gain);
	PHY_REG_MOD(pi, LCN40PHY, SignalBlockConfigTable6_new, crssignalblk_input_pwr_offset_db,
		pi_lcn->noise.input_pwr_offset);
	PHY_REG_MOD(pi, LCN40PHY, SignalBlockConfigTable5_new,
		crssignalblk_input_pwr_offset_db_40mhz,
		pi_lcn->noise.input_pwr_offset_40);

	/* Fix XXX Need to see why we require the below change, this could conflict
	  * with ucode fix for digi gain
	  */
	OSL_DELAY(20); /* To give AGC/radio ctrl chance to initialize */
}

static uint16
wlc_lcn40phy_noise_get_reg(phy_info_t *pi, uint16 reg)
{
	uint16 lcnphy_shm_ptr;

	lcnphy_shm_ptr = wlapi_bmac_read_shm(pi->sh->physhim, M_LCN40PHYREGS_PTR);
	return wlapi_bmac_read_shm(pi->sh->physhim,
		2*(lcnphy_shm_ptr + reg));
}

static void
wlc_lcn40phy_noise_set_reg(phy_info_t *pi, uint16 reg, uint16 val)
{
	uint16 lcnphy_shm_ptr;

	lcnphy_shm_ptr = wlapi_bmac_read_shm(pi->sh->physhim, M_LCN40PHYREGS_PTR);
	wlapi_bmac_write_shm(pi->sh->physhim,
		2*(lcnphy_shm_ptr + reg),
		val);
}

static bool
wlc_lcn40phy_noise_sync_ucode(phy_info_t *pi, int timeout, int* p_timeout)
{
	bool ucode_sync;
	uint16 status, ucode_status, mask, val;

	ucode_sync = FALSE;
	status = wlc_lcn40phy_noise_get_reg(pi, M_NOISE_CAL_CMD_LCN40PHY);
	if (status & k_noise_active_flag) {
		mask = k_noise_sync_mask;
		val = status & mask;
	} else {
		mask = k_noise_active_flag;
		val = 0;
	}
	/* Sync w/ ucode */
	do {
		ucode_status = wlc_lcn40phy_noise_get_reg(pi, M_NOISE_CAL_RSP_LCN40PHY);
		if ((ucode_status & mask) == val) {
		  ucode_sync = TRUE;
		  break;
		}
		if (timeout-- > 0)
		  OSL_DELAY(k_noise_sync_us_per_tick);
	} while (timeout > 0);

	if (p_timeout)
	  *p_timeout = timeout;

	return ucode_sync;
}

static void
wlc_lcn40phy_noise_ucode_ctrl(phy_info_t *pi, bool enable)
{
	uint16 ctrl, timeout;
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);

	ctrl = wlc_lcn40phy_noise_get_reg(pi, M_NOISE_CAL_CMD_LCN40PHY);
	if (enable)
		ctrl = (ctrl | k_noise_active_flag);
	else
		ctrl = (ctrl & ~k_noise_active_flag);
	ctrl += k_noise_sync_flag;
	timeout = (5*pi_lcn->noise.update_ucode_interval[pi_lcn->noise.update_step]);
	if (timeout < 5)
		timeout = 5;
	wlc_lcn40phy_noise_set_reg(pi, M_NOISE_CAL_TIMEOUT_LCN40PHY, timeout);
	wlc_lcn40phy_noise_set_reg(pi, M_NOISE_CAL_CMD_LCN40PHY, ctrl);
}

static void
wlc_lcn40phy_noise_set_input_pwr_offset(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	phy_info_lcn40phy_t *pi_lcn40 = pi->u.pi_lcn40phy;
	int16 input_power_offset_ref;
	int16 curtemp, thrs1, thrs2, thrs3, delta = 0;
	uint32 temp_cal_adj;

	if (((CHSPEC_IS2G(pi->radio_chanspec) && !pi_lcn40->temp_cal_en_2g)) ||
		((CHSPEC_IS5G(pi->radio_chanspec) && !pi_lcn40->temp_cal_en_5g)))
		return;

#ifdef BAND5G
	if (CHSPEC_IS5G(pi->radio_chanspec)) {
		if (pi_lcn->noise.nvram_input_pwr_offset_5g[0] != 0xff)
			input_power_offset_ref =
			pi_lcn->noise.nvram_input_pwr_offset_5g[0];
		else
			input_power_offset_ref = -3;
		curtemp = pi_lcn40->temp_offset_5g;
	} else
#endif /* BAND5G */
	{
		if (pi_lcn->noise.nvram_input_pwr_offset_2g != 0xff)
			input_power_offset_ref =
			pi_lcn->noise.nvram_input_pwr_offset_2g;
		else
			input_power_offset_ref = -3;
		curtemp = pi_lcn40->temp_offset_2g;
	}

	curtemp += wlc_lcn40phy_tempsense(pi, TEMPER_VBAT_TRIGGER_NEW_MEAS);

	if ((curtemp < -75) || (curtemp > 100))
		return;

	if (curtemp < pi_lcn40->pretemp) {
		thrs1 = 0;
		thrs2 = -10;
		thrs3 = 50;
	} else {
		thrs1 = 5;
		thrs2 = -5;
		thrs3 = 55;
	}

	if (CHSPEC_IS2G(pi->radio_chanspec))
		temp_cal_adj = pi_lcn40->temp_cal_adj_2g;
	else
		temp_cal_adj = pi_lcn40->temp_cal_adj_5g;

	if (curtemp < thrs2)
		delta = (((int16)(temp_cal_adj & 0xff)) << 8) >> 8;
	else if (curtemp < thrs1)
		delta = ((int16)(temp_cal_adj & 0xff00)) >> 8;
	else if (curtemp > thrs3)
		delta = ((int16)((temp_cal_adj & 0xff0000) >> 8)) >> 8;

	input_power_offset_ref += delta;

	PHY_REG_MOD(pi, LCN40PHY, SignalBlockConfigTable6_new,
		crssignalblk_input_pwr_offset_db,
		(input_power_offset_ref & 0xff));
	PHY_REG_MOD(pi, LCN40PHY, SignalBlockConfigTable5_new,
		crssignalblk_input_pwr_offset_db_40mhz,
		(input_power_offset_ref & 0xff));
	pi_lcn40->pretemp = curtemp;
}

static void
wlc_lcn40phy_noise_init(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	int nf_substract_val;

	wlc_lcn40phy_noise_log_init(pi);

#ifdef BAND5G
	if (CHSPEC_IS5G(pi->radio_chanspec)) {
		pi_lcn->noise.enable = pi_lcn->noise.nvram_enable_5g;
		nf_substract_val = pi_lcn->noise.nvram_nf_substract_val_5g;
		if (CHSPEC_IS40(pi->radio_chanspec))
			pi_lcn->noise.ref = pi_lcn->noise.nvram_ref_40_5g;
		else
			pi_lcn->noise.ref = pi_lcn->noise.nvram_ref_5g;
	} else
#endif /* BAND5G */
	{
		pi_lcn->noise.enable = pi_lcn->noise.nvram_enable_2g;
		nf_substract_val = pi_lcn->noise.nvram_nf_substract_val_2g;
		if (CHSPEC_IS40(pi->radio_chanspec))
			pi_lcn->noise.ref = pi_lcn->noise.nvram_ref_40_2g;
		else
			pi_lcn->noise.ref = pi_lcn->noise.nvram_ref_2g;
	}

#if PHY_NOISE_TEST_METRIC
	pi_lcn->noise.global_adj_en = FALSE;
#else
	pi_lcn->noise.global_adj_en = pi_lcn->noise.enable;
#endif // endif

	pi_lcn->noise.update_step_interval[k_noise_cal_update_steps-1] = 255;

	pi_lcn->noise.noise_cb = FALSE;
	pi_lcn->lcnphy_aci.init_noise_cal_done = FALSE;

	if (nf_substract_val > 0)
		PHY_REG_MOD(pi, LCN40PHY, nfSubtractVal,
			nfSubVal, nf_substract_val);

	/* Save dflt values */
	if (CHSPEC_IS2G(pi->radio_chanspec))
		pi_lcn->noise.high_gain = (int16)pi_lcn->noise.nvram_high_gain_2g;
#ifdef BAND5G
	else
		pi_lcn->noise.high_gain = (int16)pi_lcn->noise.nvram_high_gain_5g;
#endif /* BAND5G */
	PHY_REG_MOD(pi, LCN40PHY, agcControl12,
		crs_gain_high_gain_db_40mhz, pi_lcn->noise.high_gain);

	pi_lcn->noise.nf_substract_val =
		(uint16)PHY_REG_READ(pi, LCN40PHY, nfSubtractVal, nfSubVal);

	pi_lcn->noise.input_pwr_offset =
		PHY_REG_READ(pi, LCN40PHY, SignalBlockConfigTable6_new,
		crssignalblk_input_pwr_offset_db);
	pi_lcn->noise.input_pwr_offset_40 =
		PHY_REG_READ(pi, LCN40PHY, SignalBlockConfigTable5_new,
		crssignalblk_input_pwr_offset_db_40mhz);
}

static int8
wlc_lcn40phy_noise_log(uint32 x)
{
	uint32 mask;
	int8 step, i, log_x;

	mask = 0xffff0000;
	step = i = 16;
	while (step != 1) {
		step = step >> 1;
		if (mask & x) {
			i += step;
			mask = mask << step;
		} else {
			i -= step;
			mask = mask >> step;
		}
	}
	if (!(mask & x))
		i--;
	log_x = 3 * i;
	if (i > 3) {
		x = (x >> (i - 3));
		if (x > 13)
			log_x += 3;
		else if (x > 10)
			log_x += 2;
		else if (x > 8)
			log_x += 1;
	}
	return log_x;
}

static bool
wlc_lcn40phy_noise_adj(phy_info_t *pi, uint32 metric)
{
#if PHY_NOISE_TEST_METRIC
	int8 log_metric;

	if (metric > 0)
	  log_metric = wlc_lcn40phy_noise_log(metric);
	else
	  log_metric = -1;

	wlc_lcn40phy_noise_log_adj(pi, log_metric, log_metric, 255, 255, TRUE, TRUE);

	return TRUE;
#else
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	int previous_gain, new_gain;
	int delta;
	int16 nvram_gain_tbl_adj;
	phy_info_lcn40phy_t *pi_lcn40 = pi->u.pi_lcn40phy;

	if (pi_lcn->noise.ref <= 0)
	  return TRUE;

	previous_gain = phy_utils_read_phyreg(pi, LCN40PHY_wl_gain_tbl_offset) & 0xff;

#ifdef BAND5G
	if (CHSPEC_IS5G(pi->radio_chanspec)) {
		nvram_gain_tbl_adj = pi_lcn->noise.nvram_gain_tbl_adj_5g;
	} else
#endif /* BAND5G */
	{
		nvram_gain_tbl_adj = pi_lcn->noise.nvram_gain_tbl_adj_2g;
	}

	delta = pi_lcn->noise.ref - wlc_lcn40phy_noise_log(metric) +
		previous_gain - nvram_gain_tbl_adj +
		(phy_utils_read_phyreg(pi, LCN40PHY_agcControl12) & 0xff) - pi_lcn->noise.high_gain;
	delta = delta > pi_lcn40->noise_cal_deltamax ? pi_lcn40->noise_cal_deltamax : delta;
	delta = delta < pi_lcn40->noise_cal_deltamin ? pi_lcn40->noise_cal_deltamin : delta;

	PHY_INFORM(("metric %d, dbm %d, delta %d, gain_offset %d, high gain %d, power offset %x\n",
		metric, wlc_lcn40phy_noise_log(metric), delta,
		phy_utils_read_phyreg(pi, LCN40PHY_wl_gain_tbl_offset) & 0xff,
		phy_utils_read_phyreg(pi, LCN40PHY_agcControl12) & 0xff,
		phy_utils_read_phyreg(pi, LCN40PHY_SignalBlockConfigTable6_new) & 0xff));
	new_gain = previous_gain;
	/*
	if (delta >= 3) {
		new_gain = nvram_gain_tbl_adj + 3;
		delta -= 3;
	} else if (delta <= -3) {
		new_gain = nvram_gain_tbl_adj - 3;
		delta += 3;
	} else
		new_gain = nvram_gain_tbl_adj;

	PHY_REG_MOD(pi, LCN40PHY, wl_gain_tbl_offset, wl_gain_tbl_offset, new_gain);
	*/
	PHY_REG_MOD(pi, LCN40PHY, SignalBlockConfigTable6_new, crssignalblk_input_pwr_offset_db,
		pi_lcn->noise.input_pwr_offset + delta);
	PHY_REG_MOD(pi, LCN40PHY, SignalBlockConfigTable5_new,
		crssignalblk_input_pwr_offset_db_40mhz,
		pi_lcn->noise.input_pwr_offset_40 + delta);

	return (previous_gain == new_gain);
#endif /* PHY_NOISE_TEST_METRIC */
}

static bool
wlc_lcn40phy_noise_metric(phy_info_t* pi, uint32* metric, uint32* power)
{
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);

	*metric = *power = wlc_lcn40phy_noise_get_reg(pi, M_NOISE_CAL_DATA_LCN40PHY);

	if (!pi_lcn->noise.tainted && (*power < 1000)) {
		PHY_INFORM(("wl%d: %s: bogus noise = %d\n",
			pi->sh->unit, __FUNCTION__, (*power)));
#if !PHY_NOISE_TEST_METRIC
		return FALSE;
#endif // endif
	}
	return TRUE;
}

static void
wlc_lcn40phy_noise_reset(phy_info_t *pi, bool restore_regs)
{
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);

	pi_lcn->noise.state = k_noise_end_state;
	wlc_lcn40phy_noise_ucode_ctrl(pi, FALSE);
	if (pi_lcn->noise.adj_en)
		wlc_lcn40phy_noise_restore_phy_regs(pi);
}

static void
wlc_lcn40phy_noise_reset_data(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);

	pi_lcn->noise.ucode_data_len = 0;
	pi_lcn->noise.ucode_data_idx = 0;
	pi_lcn->noise.ucode_data_ok_cnt = 0;
	pi_lcn->noise.update_cnt = 0;
	pi_lcn->noise.update_step = 0;

	wlc_lcn40phy_noise_log_ucode_data(pi);
	wlc_lcn40phy_noise_log_ucode_data_reset(pi);
}

void
wlc_lcn40phy_noise_measure_start(phy_info_t *pi, bool adj_en)
{
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	if (!pi_lcn->noise.enable) {
		if (pi->phynoise_state & PHY_NOISE_STATE_MON)
			pi->phynoise_state &= ~PHY_NOISE_STATE_MON;
		if (pi->phynoise_state & PHY_NOISE_STATE_EXTERNAL)
			pi->phynoise_state &= ~PHY_NOISE_STATE_EXTERNAL;
	  return;
	}

	pi_lcn->noise.noise_cb = pi_lcn->noise.noise_cb || !adj_en;
	if (adj_en) {
		pi_lcn->noise.state = k_noise_adj_state;
		pi_lcn->noise.power = 0;
	} else {
		  return;
	}

	pi_lcn->noise.tainted = FALSE;
	wlc_lcn40phy_noise_measure_stop(pi);
	pi_lcn->noise.adj_en = adj_en && pi_lcn->noise.global_adj_en;
	if (pi_lcn->noise.adj_en)
		wlc_lcn40phy_noise_restore_phy_regs(pi);
	wlc_lcn40phy_noise_reset_data(pi);
	wlc_lcn40phy_noise_ucode_ctrl(pi, TRUE);

	wlc_lcn40phy_noise_log_start(pi);
}

void
wlc_lcn40phy_noise_measure_stop(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);

	if (!pi_lcn->noise.enable)
		return;

	wlc_lcn40phy_noise_ucode_ctrl(pi, FALSE);

	/* This is called from beginning of watchdog. If ucode cant
	 * be stopped before exiting this function the noise measurement
	 * may be invalid b/c of a cal....
	 */
	/* declare tainted noise data due to clear iqstart manually */
	pi_lcn->noise.tainted = TRUE;
}

static void
wlc_lcn40phy_noise_attach(phy_info_t *pi)
{
	wlc_lcn40phy_noise_reset(pi, FALSE);
	wlc_lcn40phy_noise_reset_data(pi);
	wlc_lcn40phy_noise_reset_log(pi);
}

static void
wlc_lcn40phy_noise_cb(phy_info_t *pi, uint32 metric)
{
	int16 noise_dbm;
	uint16 channel;

	/*                       ADC     rx filt          num of samples
	 * phy_scale =  20*log((2^9/0.4) * (2^4) )  + 10*log(  2  *   64 ) = 107
	 * noise_dBm = -174 + 10*log(20*10^6) = -101dBm
	 * 10log(noise_v^2) = noise_dBm + 30 + 10*log(50) = -54
	 * 10log(metric) =  10log(2*noise_v^2) + phy_scale = 56
	 */
	noise_dbm = (wlc_lcn40phy_noise_log(metric) - 56) - 78;

	if (noise_dbm < -98) /* Limit to a reasonable value */
		noise_dbm = -98;
	channel = wlapi_bmac_read_shm(pi->sh->physhim, M_CURCHANNEL);

	wlc_phy_noise_cb(pi, (uint8)channel, (int8)noise_dbm);
}

void
wlc_lcn40phy_noise_measure(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	uint32 metric, power;

	metric = power = 0;

	/* This proc is called from noise measure callback or from watchdog timer */
	if (!(pi_lcn->noise.enable &&
	      /* Only run if not complete */
	      (pi_lcn->noise.state != k_noise_end_state) &&
	      /* No point in running this if ucode is not active */
	      (R_REG(pi->sh->osh, &pi->regs->maccontrol) & MCTL_EN_MAC)))
		return;

	/* wlc_lcn40phy_noise_log_callback(pi); */

	if (!pi_lcn->noise.tainted &&
	    wlc_lcn40phy_noise_sync_ucode(pi, 0, NULL) &&
	    wlc_lcn40phy_noise_metric(pi, &metric, &power)) {
		uint32 tmp, avg_metric;
		int8 i, j;
		bool done;

		wlc_lcn40phy_noise_log_callback(pi);

		pi_lcn->noise.power = power;
		PHY_INFORM(("power %d, ucode_data_idx %d, ucode_data_len %d, gain offset %d\n",
			power, pi_lcn->noise.ucode_data_idx, pi_lcn->noise.ucode_data_len,
			phy_utils_read_phyreg(pi, LCN40PHY_wl_gain_tbl_offset) & 0xff));

		if (pi_lcn->noise.state == k_noise_adj_state) {
			/* Update metric list */
			avg_metric = 0;
			i = 0;
			j = pi_lcn->noise.ucode_data_idx;
			done = FALSE;
			while (!done && (i < pi_lcn->noise.ucode_data_len)) {
				tmp = pi_lcn->noise.ucode_data[j];
				if (metric < tmp) {
					pi_lcn->noise.ucode_data[j] = metric;
					metric = tmp;
					done = TRUE;
					/* wlc_lcn40phy_noise_log_ucode_data_insert_time(pi); */
					if (i == 0) {
						wlc_lcn40phy_noise_log_ucode_data_insert_time(pi);
						wlc_lcn40phy_noise_log_ucode_samples(pi);
					}
				}
				avg_metric += pi_lcn->noise.ucode_data[j];
				i++;
				if (++j >= k_noise_cal_ucode_data_size)
					j = 0;
			}
			while (i < k_noise_cal_ucode_data_size) {
				tmp = pi_lcn->noise.ucode_data[j];
				pi_lcn->noise.ucode_data[j] = metric;
				metric = tmp;
				avg_metric += pi_lcn->noise.ucode_data[j];
				if (i == 0) {
					wlc_lcn40phy_noise_log_ucode_data_insert_time(pi);
					wlc_lcn40phy_noise_log_ucode_samples(pi);
				}
				i++;
				if (++j >= k_noise_cal_ucode_data_size)
					j = 0;
			}
			if (pi_lcn->noise.ucode_data_len < k_noise_cal_ucode_data_size) {
				pi_lcn->noise.ucode_data_ok_cnt = 0;
				pi_lcn->noise.ucode_data_len++;
			}

			wlc_lcn40phy_noise_log_ucode_data(pi);

			if (pi_lcn->noise.ucode_data_len == k_noise_cal_ucode_data_size) {
				uint8 data_int, step_int;

				data_int =
				pi_lcn->noise.update_data_interval[pi_lcn->noise.update_step];
				step_int =
				pi_lcn->noise.update_step_interval[pi_lcn->noise.update_step];

				if (pi_lcn->noise.ucode_data_ok_cnt < 253)
					pi_lcn->noise.ucode_data_ok_cnt++;

				wlc_lcn40phy_noise_log_ucode_data_ok(pi);
				PHY_INFORM(("noise.ucode_data_ok_cnt %d, data_int %d\n",
					pi_lcn->noise.ucode_data_ok_cnt, data_int));

				if (pi_lcn->noise.ucode_data_ok_cnt == data_int) {
					/* Throw away max and min periodically */
					if (++(pi_lcn->noise.ucode_data_idx) >=
						k_noise_cal_ucode_data_size)
						pi_lcn->noise.ucode_data_idx = 0;
					pi_lcn->noise.ucode_data_len -= 2;

					/* Calculate adjustment, only do it with valid noise ref */
					if (!wlc_lcn40phy_noise_adj(pi, avg_metric)) {
					  /* Adjusted listen gain, so
					   * need to throw away old data
					   */
					  wlc_lcn40phy_noise_advance_log(pi);
					  wlc_lcn40phy_noise_reset_data(pi);
					  wlc_lcn40phy_agc_reset(pi);
					} else {
						if (1) {
							if (pi_lcn->noise.update_cnt < 253)
								pi_lcn->noise.update_cnt++;
							if ((pi_lcn->noise.update_cnt
								== step_int) &&
								(pi_lcn->noise.update_step <
								(k_noise_cal_update_steps-1)))
								pi_lcn->noise.update_step++;
						} else {
							pi_lcn->noise.update_step = 0;
							pi_lcn->noise.update_cnt = 0;
						}
					}
					pi_lcn->noise.ucode_data_ok_cnt = 0;
				}
			}
		} /* if ( pi_lcn->noise.state == k_noise_adj_state ) */
	} else {
		if (pi_lcn->noise.tainted) {
			wlc_lcn40phy_noise_log_tainted(pi);
		}
	}

	/* Kick off new measurement if needed */
	pi_lcn->noise.tainted = FALSE;
	if (pi_lcn->noise.state == k_noise_adj_state) {
		wlc_lcn40phy_noise_ucode_ctrl(pi, TRUE);
	} else {
		wlc_lcn40phy_noise_reset(pi, FALSE);
	}

	/* Callback for periodic noise measurement */
	wlc_lcn40phy_noise_cb(pi, pi_lcn->noise.power);
	pi_lcn->noise.noise_cb = FALSE;

	if (!pi_lcn->lcnphy_aci.init_noise_cal_done) {

		pi_lcn->lcnphy_aci.init_noise_cal_done = TRUE;
	}
}

void
wlc_lcn40phy_aci_upd(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	uint8 time_since_bcn;
	uint16 cnt, delta;
	uint16 t;
	uint8 incr = 3;

	/* Read rxcrsglitch count from shared memory */
	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	time_since_bcn = wlapi_bmac_time_since_bcn_get(pi->sh->physhim);
	cnt =  wlapi_bmac_read_shm(pi->sh->physhim, MACSTAT_ADDR(MCSTOFF_RXCRSGLITCH));
	delta = (cnt - pi_lcn->lcnphy_aci.glitch_cnt) & 0xffff;

	pi_lcn->lcnphy_aci.glitch_cnt = cnt;
	if ((!(pi->aci_state & ACI_ACTIVE) && (delta < pi_lcn->lcnphy_aci.on_thresh)) ||
	     ((pi->aci_state & ACI_ACTIVE) && (delta > pi_lcn->lcnphy_aci.off_thresh)))
		pi_lcn->lcnphy_aci.ts = (int)((pi)->sh->now);

	if (pi->aci_state & ACI_ACTIVE) {
		if (delta > pi_lcn->lcnphy_aci.off_thresh)
			pi_lcn->lcnphy_aci.gain_backoff = 1;
		else
			pi_lcn->lcnphy_aci.gain_backoff = 0;
	}
	if (pi_lcn->lcnphy_aci.gain_backoff) {
		/* put your code here */
		/* Fixed 3dB Listen Gain BackOff Which Decided by Auto Aci Logic */
		wlapi_high_update_phy_mode(pi->sh->physhim, PHY_MODE_ACI);
		pi_lcn->lcnphy_aci.Latest_Listen_GaindB_Latch -= incr;

		if (pi_lcn->lcnphy_aci.Latest_Listen_GaindB_Latch <=
			pi_lcn->lcnphy_aci.GaindB_Limit) {
			pi_lcn->lcnphy_aci.Latest_Listen_GaindB_Latch =
				pi_lcn->lcnphy_aci.GaindB_Limit;
		}

		pi_lcn->lcnphy_aci.EdOn_Thresh_Latch += incr;
		if (pi_lcn->lcnphy_aci.EdOn_Thresh_Latch >=
			(pi_lcn->lcnphy_aci.EdOn_Thresh_Limit)) {
			pi_lcn->lcnphy_aci.EdOn_Thresh_Latch =
				pi_lcn->lcnphy_aci.EdOn_Thresh_Limit;
		}

		pi_lcn->lcnphy_aci.SignalBlock_edet1_ofdm_Latch -= incr;
		if (pi_lcn->lcnphy_aci.SignalBlock_edet1_ofdm_Latch <=
			pi_lcn->lcnphy_aci.SignalBlock_edet1_ofdm_Limit) {
			pi_lcn->lcnphy_aci.SignalBlock_edet1_ofdm_Latch =
				pi_lcn->lcnphy_aci.SignalBlock_edet1_ofdm_Limit;
		}

		pi_lcn->lcnphy_aci.SignalBlock_edet1_dsss_Latch -= incr;
		if (pi_lcn->lcnphy_aci.SignalBlock_edet1_dsss_Latch <=
			pi_lcn->lcnphy_aci.SignalBlock_edet1_dsss_Limit) {
			pi_lcn->lcnphy_aci.SignalBlock_edet1_dsss_Latch =
				pi_lcn->lcnphy_aci.SignalBlock_edet1_dsss_Limit;
		}

		PHY_REG_MOD(pi, LCN40PHY, agcControl12, crs_gain_high_gain_db_40mhz,
			pi_lcn->lcnphy_aci.Latest_Listen_GaindB_Latch);
		PHY_REG_MOD(pi, LCN40PHY, SignalBlock_edet1, signalblk_det_thresh_ofdm,
			pi_lcn->lcnphy_aci.SignalBlock_edet1_ofdm_Latch);
		PHY_REG_MOD(pi, LCN40PHY, SignalBlock_edet1, signalblk_det_thresh_dsss,
			pi_lcn->lcnphy_aci.SignalBlock_edet1_dsss_Latch);
		if (CHIPID(pi->sh->chip) == BCM43341_CHIP_ID) {
			if (CHSPEC_IS20(pi->radio_chanspec)) {
				PHY_REG_MOD(pi, LCN40PHY, edthresh20ul, edonthreshold20L,
					pi_lcn->lcnphy_aci.EdOn_Thresh_Latch);
			} else {
				PHY_REG_MOD(pi, LCN40PHY, edthresh20ul, edonthreshold20U,
					pi_lcn->lcnphy_aci.EdOn_Thresh_Latch);
				PHY_REG_MOD(pi, LCN40PHY, edthresh20ul, edonthreshold20L,
					pi_lcn->lcnphy_aci.EdOn_Thresh_Latch);
				PHY_REG_MOD(pi, LCN40PHY, edthresh40, edonthreshold40,
					pi_lcn->lcnphy_aci.EdOn_Thresh_Latch);
			}
		} else {
			PHY_REG_MOD(pi, LCN40PHY, crsedthresh, edonthreshold,
				pi_lcn->lcnphy_aci.EdOn_Thresh_Latch);
		}
		wlc_lcn40phy_agc_reset(pi);
	}

	t = (uint16)((int)((pi)->sh->now) - pi_lcn->lcnphy_aci.ts);

	if (time_since_bcn > pi_lcn->lcnphy_aci.bcn_thresh)
		wlc_lcn40phy_aci(pi, FALSE);
	else if (!(pi->aci_state & ACI_ACTIVE) && (t >= pi_lcn->lcnphy_aci.on_timeout))
		wlc_lcn40phy_aci(pi, TRUE);
	else if ((pi->aci_state & ACI_ACTIVE) && (t >= pi_lcn->lcnphy_aci.off_timeout))
		wlc_lcn40phy_aci(pi, FALSE);

	wlapi_enable_mac(pi->sh->physhim);
	PHY_INFORM(("aci state %d delta %d backoff %d HGain %d Edonthr %d BcnLost %d %2x%2x t %d\n",
		pi->aci_state, delta, pi_lcn->lcnphy_aci.gain_backoff,
		pi_lcn->lcnphy_aci.Latest_Listen_GaindB_Latch,
		pi_lcn->lcnphy_aci.EdOn_Thresh_Latch,
		time_since_bcn,
		pi_lcn->lcnphy_aci.SignalBlock_edet1_ofdm_Latch,
		pi_lcn->lcnphy_aci.SignalBlock_edet1_dsss_Latch, t));
}

void
wlc_lcn40phy_aci_init(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	phy_info_lcn40phy_t *pi_lcn40 = pi->u.pi_lcn40phy;

	PHY_INFORM(("aci init interference_mode %d\n", pi->sh->interference_mode));

	pi->aci_state = 0;
	pi_lcn->lcnphy_aci.gain_backoff = 0;
	pi_lcn->lcnphy_aci.on_thresh = 1000;
	pi_lcn->lcnphy_aci.on_timeout = 3;
	pi_lcn->lcnphy_aci.off_thresh = 500;
	pi_lcn->lcnphy_aci.off_timeout = 20;
	pi_lcn->lcnphy_aci.SignalBlock_edet1_dsss_Limit = 0x41;
	pi_lcn->lcnphy_aci.SignalBlock_edet1_ofdm_Limit = 0x3f;
	pi_lcn->lcnphy_aci.SignalBlock_edet1_ofdm_Back = 0x61;
	pi_lcn->lcnphy_aci.SignalBlock_edet1_dsss_Back = 0x63;

	/* 43341 requires a lower threshold to trigger algorithm */
	if ((CHIPID(pi->sh->chip) == BCM43340_CHIP_ID) ||
		(CHIPID(pi->sh->chip) == BCM43341_CHIP_ID)) {
		pi_lcn->lcnphy_aci.on_thresh = 150;
		pi_lcn->lcnphy_aci.off_thresh = 80;
	}

	pi_lcn->lcnphy_aci.Listen_GaindB_AfrNoiseCal = 0x46;
	pi_lcn->lcnphy_aci.GaindB_Limit = 0x31;

	if (CHIPID(pi->sh->chip) == BCM43142_CHIP_ID)
		pi_lcn->lcnphy_aci.EdOn_Thresh_BASE = -81;
	else {
		/* Initialize ED based on the channel bandwidth */
		PHY_REG_MOD(pi, LCN40PHY, eddisable20ul, crs_ed_asserts_crs, 0);
		if (CHIPID(pi->sh->chip) == BCM43341_CHIP_ID) {
			/* crs_ed_asserts_crs = 1 will enable these ED parameters to be effective */
			PHY_REG_MOD(pi, LCN40PHY, eddisable20ul, crs_ed_asserts_crs, 1);
		}
		if (CHSPEC_IS20(pi->radio_chanspec)) {
			PHY_REG_LIST_START
				PHY_REG_MOD_ENTRY(LCN40PHY, eddisable20ul, crseddisable_20L, 0)
				PHY_REG_MOD_ENTRY(LCN40PHY, eddisable20ul, crseddisable_20U, 1)
				PHY_REG_MOD_ENTRY(LCN40PHY, eddisable20ul, crseddisable_40, 1)
			PHY_REG_LIST_EXECUTE(pi);
		} else {
			PHY_REG_LIST_START
				PHY_REG_MOD_ENTRY(LCN40PHY, eddisable20ul, crseddisable_20L, 0)
				PHY_REG_MOD_ENTRY(LCN40PHY, eddisable20ul, crseddisable_20U, 0)
				PHY_REG_MOD_ENTRY(LCN40PHY, eddisable20ul, crseddisable_40, 0)
			PHY_REG_LIST_EXECUTE(pi);
		}

		if ((CHIPID(pi->sh->chip) == BCM43340_CHIP_ID) ||
			(CHIPID(pi->sh->chip) == BCM43341_CHIP_ID)) {
			if (CHSPEC_IS20(pi->radio_chanspec)) {
				PHY_REG_MOD(pi, LCN40PHY, edthresh40, edonthreshold40, 0xf6);
				PHY_REG_MOD(pi, LCN40PHY, edthresh40, edoffthreshold40, 0xa8);
			}
			else {
				PHY_REG_MOD(pi, LCN40PHY, edthresh40, edonthreshold40, 0xbf);
				PHY_REG_MOD(pi, LCN40PHY, edthresh40, edoffthreshold40, 0xb9);
			}
			pi_lcn->lcnphy_aci.EdOn_Thresh_BASE = 0xbf;
		} else {
			/* Initialize the edon/off thresholds based on NVRAM */
			PHY_REG_MOD(pi, LCN40PHY, edthresh40, edonthreshold40,
				pi_lcn40->edonthreshold40);
			PHY_REG_MOD(pi, LCN40PHY, edthresh40, edoffthreshold40,
				pi_lcn40->edoffthreshold40);
			pi_lcn->lcnphy_aci.EdOn_Thresh_BASE =
			PHY_REG_READ(pi, LCN40PHY, edthresh40, edonthreshold40);
		}
		PHY_REG_MOD(pi, LCN40PHY, crsedthresh, edoffthreshold,
			pi_lcn40->edoffthreshold20UL);
		PHY_REG_MOD(pi, LCN40PHY, edthresh20ul, edonthreshold20U,
			pi_lcn40->edonthreshold20U);
		PHY_REG_MOD(pi, LCN40PHY, edthresh20ul, edonthreshold20L,
			pi_lcn40->edonthreshold20L);
	}
	pi_lcn->lcnphy_aci.EdOn_Thresh_Limit = 0xc4;

	pi_lcn->lcnphy_aci.bcn_thresh = 3;

	pi_lcn->noise.high_gain =
		pi_lcn->lcnphy_aci.Latest_Listen_GaindB_Latch =
		pi_lcn->lcnphy_aci.Listen_GaindB_AfrNoiseCal;
	pi_lcn->lcnphy_aci.SignalBlock_edet1_ofdm_Latch =
		pi_lcn->lcnphy_aci.SignalBlock_edet1_ofdm_Back;
	pi_lcn->lcnphy_aci.SignalBlock_edet1_dsss_Latch =
		pi_lcn->lcnphy_aci.SignalBlock_edet1_dsss_Back;
	pi_lcn->lcnphy_aci.EdOn_Thresh_Latch =
		pi_lcn->lcnphy_aci.EdOn_Thresh_BASE;

	if (CHIPID(pi->sh->chip) == BCM43142_CHIP_ID ||
		CHIPID(pi->sh->chip) == BCM43340_CHIP_ID ||
		CHIPID(pi->sh->chip) == BCM43341_CHIP_ID) {
		PHY_REG_MOD(pi, LCN40PHY, agcControl12, crs_gain_high_gain_db_40mhz,
			(uint16)pi_lcn->lcnphy_aci.Listen_GaindB_AfrNoiseCal);
		PHY_REG_MOD(pi, LCN40PHY, crsedthresh, edonthreshold,
			(uint16)pi_lcn->lcnphy_aci.EdOn_Thresh_BASE);
		PHY_REG_MOD(pi, LCN40PHY, SignalBlock_edet1, signalblk_det_thresh_ofdm,
			pi_lcn->lcnphy_aci.SignalBlock_edet1_ofdm_Back);
		PHY_REG_MOD(pi, LCN40PHY, SignalBlock_edet1, signalblk_det_thresh_dsss,
			pi_lcn->lcnphy_aci.SignalBlock_edet1_dsss_Back);

		wlc_lcn40phy_aci(pi, FALSE);
	}
}

void
wlc_lcn40phy_aci(phy_info_t *pi, bool on)
{
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	uint8 decr = 3;
	if (on && !(pi->aci_state & ACI_ACTIVE)) {
		pi->aci_state |= ACI_ACTIVE;
		/* set the registers for ACI */
		pi_lcn->lcnphy_aci.gain_backoff = 1;
	} else if (!on && (pi->aci_state & ACI_ACTIVE)) {

		pi_lcn->lcnphy_aci.Latest_Listen_GaindB_Latch += decr;
		if (pi_lcn->lcnphy_aci.Latest_Listen_GaindB_Latch >=
			pi_lcn->lcnphy_aci.Listen_GaindB_AfrNoiseCal) {
			pi_lcn->lcnphy_aci.Latest_Listen_GaindB_Latch =
			pi_lcn->lcnphy_aci.Listen_GaindB_AfrNoiseCal;
		}

		pi_lcn->lcnphy_aci.EdOn_Thresh_Latch -= decr;
		if (pi_lcn->lcnphy_aci.EdOn_Thresh_Latch <=
			pi_lcn->lcnphy_aci.EdOn_Thresh_BASE) {
			pi_lcn->lcnphy_aci.EdOn_Thresh_Latch =
			pi_lcn->lcnphy_aci.EdOn_Thresh_BASE;
		}

		pi_lcn->lcnphy_aci.SignalBlock_edet1_ofdm_Latch += decr;
		if (pi_lcn->lcnphy_aci.SignalBlock_edet1_ofdm_Latch >=
			pi_lcn->lcnphy_aci.SignalBlock_edet1_ofdm_Back) {
			pi_lcn->lcnphy_aci.SignalBlock_edet1_ofdm_Latch =
			pi_lcn->lcnphy_aci.SignalBlock_edet1_ofdm_Back;
		}

		pi_lcn->lcnphy_aci.SignalBlock_edet1_dsss_Latch += decr;
		if (pi_lcn->lcnphy_aci.SignalBlock_edet1_dsss_Latch >=
			pi_lcn->lcnphy_aci.SignalBlock_edet1_dsss_Back) {
			pi_lcn->lcnphy_aci.SignalBlock_edet1_dsss_Latch =
			pi_lcn->lcnphy_aci.SignalBlock_edet1_dsss_Back;
		}

		if (pi_lcn->lcnphy_aci.SignalBlock_edet1_dsss_Latch ==
			pi_lcn->lcnphy_aci.SignalBlock_edet1_dsss_Back) {
			/* finnaly relase the ACI_ACTIVE state */
			pi->aci_state &= ~ACI_ACTIVE;
			pi_lcn->lcnphy_aci.gain_backoff = 0;
			wlapi_high_update_phy_mode(pi->sh->physhim, 0);
		}

		PHY_REG_MOD(pi, LCN40PHY, agcControl12, crs_gain_high_gain_db_40mhz,
			(uint16)pi_lcn->lcnphy_aci.Latest_Listen_GaindB_Latch);

		PHY_REG_MOD(pi, LCN40PHY, crsedthresh, edonthreshold,
			(uint16)pi_lcn->lcnphy_aci.EdOn_Thresh_Latch);

		PHY_REG_MOD(pi, LCN40PHY, SignalBlock_edet1, signalblk_det_thresh_ofdm,
			pi_lcn->lcnphy_aci.SignalBlock_edet1_ofdm_Latch);
		PHY_REG_MOD(pi, LCN40PHY, SignalBlock_edet1, signalblk_det_thresh_dsss,
			pi_lcn->lcnphy_aci.SignalBlock_edet1_dsss_Latch);
		/* Reset radio ctrl and crs gain */
		wlc_lcn40phy_agc_reset(pi);
	}

	pi_lcn->lcnphy_aci.ts = (int)((pi)->sh->now);
}

void
wlc_lcn40phy_rev6_aci(phy_info_t *pi, int wanted_mode)
{
	switch (wanted_mode) {
		case WLAN_AUTO:
		case WLAN_AUTO_W_NOISE:
		case NON_WLAN:
			/* Automatic ACI detection */
			PHY_REG_MOD(pi, LCN40PHY, aciContro9,
				aci_detect_aci_det_en, 1);
			PHY_REG_MOD(pi, LCN40PHY, aciContro9,
				aci_detect_aci_det_force, 0);
			break;

		case WLAN_MANUAL:
			/* Force ACI gain table */
			PHY_REG_MOD(pi, LCN40PHY, aciContro9,
				aci_detect_aci_det_force, 1);
			PHY_REG_MOD(pi, LCN40PHY, aciContro9,
				aci_detect_aci_pre_sel, 1);
			break;

		case INTERFERE_NONE:
		default:
			/* Force normal gain table */
			PHY_REG_MOD(pi, LCN40PHY, aciContro9,
				aci_detect_aci_det_force, 1);
			PHY_REG_MOD(pi, LCN40PHY, aciContro9,
				aci_detect_aci_pre_sel, 0);
	}
}

#if defined(WLC_LOWPOWER_BEACON_MODE)
void
wlc_lcn40phy_lowpower_beacon_mode(phy_info_t *pi, int lowpower_beacon_mode)
{
	phy_info_lcn40phy_t *pi_lcn40 = pi->u.pi_lcn40phy;
	int adc_mode, lpf_mode, lna1_mode, lna2_mode, rxmix_mode;
	if (pi_lcn40->lowpower_beacon_mode == lowpower_beacon_mode)
		return;
	switch (lowpower_beacon_mode)
	{
	case 0:
		adc_mode = lpf_mode = lna1_mode = lna2_mode = rxmix_mode = 0;
		if (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA) {
			wlc_lcn40phy_write_table(pi,
				&dot11lcn40phytbl_2G_ext_lna_rx_gain_info_rev4[1]);
			wlc_lcn40phy_write_table(pi,
				&dot11lcn40phytbl_2G_ext_lna_rx_gain_info_rev4[2]);
		} else {
			wlc_lcn40phy_write_table(pi,
				&dot11lcn40phytbl_2G_rx_gain_info_rev4[1]);
			wlc_lcn40phy_write_table(pi,
				&dot11lcn40phytbl_2G_rx_gain_info_rev4[2]);
		}
		break;
	case 1:
		adc_mode = 3;
		lpf_mode = 2;
		lna1_mode = 1;
		lna2_mode = 1;
		rxmix_mode = 1;
		wlc_lcn40phy_write_table(pi,
			&dot11lcn40_2G_ext_lna_rx_gain_tbl_lowpower_c1_rev4[0]);
		wlc_lcn40phy_write_table(pi,
			&dot11lcn40_2G_ext_lna_rx_gain_tbl_lowpower_c1_rev4[1]);
		break;
	case 2:
		adc_mode = 1;
		lpf_mode = 2;
		lna1_mode = 1;
		lna2_mode = 1;
		rxmix_mode = 1;
		wlc_lcn40phy_write_table(pi,
			&dot11lcn40_2G_ext_lna_rx_gain_tbl_lowpower_c2_rev4[0]);
		wlc_lcn40phy_write_table(pi,
			&dot11lcn40_2G_ext_lna_rx_gain_tbl_lowpower_c2_rev4[1]);
		break;
	case 3:
		adc_mode = 1;
		lpf_mode = 1;
		lna1_mode = 1;
		lna2_mode = 1;
		rxmix_mode = 1;
		wlc_lcn40phy_write_table(pi,
			&dot11lcn40_2G_ext_lna_rx_gain_tbl_lowpower_c3_rev4[0]);
		wlc_lcn40phy_write_table(pi,
			&dot11lcn40_2G_ext_lna_rx_gain_tbl_lowpower_c3_rev4[1]);
		break;
	case 4:
		adc_mode = 3;
		lpf_mode = 1;
		lna1_mode = 1;
		lna2_mode = 1;
		rxmix_mode = 1;
		wlc_lcn40phy_write_table(pi,
			&dot11lcn40_2G_ext_lna_rx_gain_tbl_lowpower_c4_rev4[0]);
		wlc_lcn40phy_write_table(pi,
			&dot11lcn40_2G_ext_lna_rx_gain_tbl_lowpower_c4_rev4[1]);
		break;
	default:
		adc_mode = lpf_mode = lna1_mode = lna2_mode = rxmix_mode = 0;
	}

	wlc_lcn40phy_adc_lowpower(pi, adc_mode);
	wlc_lcn40phy_lpf_lowpower(pi, lpf_mode);
	wlc_lcn40phy_lna1_2g_lowpower(pi, lna1_mode);
	wlc_lcn40phy_lna2_2g_lowpower(pi, lna2_mode);
	wlc_lcn40phy_rxmix_2g_lowpower(pi, rxmix_mode);
	pi_lcn40->lowpower_beacon_mode = lowpower_beacon_mode;
}
#endif /* WLC_LOWPOWER_BEACON_MODE */

uint8
wlc_lcn40phy_max_cachedchans(phy_info_t *pi)
{
	return LCN40PHY_MAX_CAL_CACHE;
}

void
/* Conditonal Backoff/Boost Per Rate TX Power Targets */
wlc_lcn40phy_apply_cond_chg(phy_info_lcn40phy_t *pi_lcn40, ppr_t *tx_pwr_target)
{
	ppr_plus_cmn_val(tx_pwr_target,
		(pi_lcn40->cond_offs1 + pi_lcn40->cond_offs2 +
		pi_lcn40->cond_offs3 + pi_lcn40->cond_offs4));
}

/* Temperature/Vbat based Backoff/Boost Per Rate TX Power Targets */
static void
wlc_lcn40phy_update_temp_vbat_backoff_boost(phy_info_t* pi, int16 senseval,
	int8 offs1, int8 offs2, int8* cond_offs1, int8* cond_offs2,
	int8 high_threshold, int8 low_threshold, int8 diff)
{
	if (offs1) {
		if (*cond_offs1) {
			/* If offset is already applied, wait for senseval  */
			/* to go lower than threshold-diff to undo it */
			if (senseval < (high_threshold - diff)) {
				*cond_offs1 = 0;
				PHY_TMP(("Undo offs1, senseval = %d\n", senseval));
				wlapi_high_txpwr_limit_update_req(pi->sh->physhim);
			}
		}
		else if (senseval > high_threshold) {
			*cond_offs1 = offs1;
			PHY_TMP(("Apply offs1, senseval = %d\n", senseval));
			wlapi_high_txpwr_limit_update_req(pi->sh->physhim);
		}
	}

	if (offs2) {
		if (*cond_offs2) {
			/* If offset is already applied, wait for senseval  */
			/* to go higher than threshold+diff to undo it */
			if (senseval > (low_threshold + diff)) {
				*cond_offs2 = 0;
				PHY_TMP(("Undo offs2, senseval = %d\n", senseval));
				wlapi_high_txpwr_limit_update_req(pi->sh->physhim);
			}
		}
		else if (senseval < low_threshold) {
			*cond_offs2 = offs2;
			PHY_TMP(("Apply offs2, senseval = %d\n", senseval));
			wlapi_high_txpwr_limit_update_req(pi->sh->physhim);
		}
	}
}

/* Modify Tx Power Per Rate Targets Based on Envirormental Conditions */
/* such as Temperature and Vbat */
void wlc_lcn40phy_update_cond_backoff_boost(phy_info_t* pi)
{
	int16 temper;
	int8 vbat;
	int8 temp_offs1, temp_offs2;
	int8 vbat_offs1, vbat_offs2;
	phy_info_lcn40phy_t *pi_lcn40 = pi->u.pi_lcn40phy;

#ifdef BAND5G
	if (CHSPEC_IS5G(pi->radio_chanspec)) {
		temp_offs1 = pi_lcn40->temp_offs1_5g;
		temp_offs2 = pi_lcn40->temp_offs2_5g;
		vbat_offs1 = pi_lcn40->vbat_offs1_5g;
		vbat_offs2 = pi_lcn40->vbat_offs2_5g;
	} else
#endif /* BAND5G */
	{
		temp_offs1 = pi_lcn40->temp_offs1_2g;
		temp_offs2 = pi_lcn40->temp_offs2_2g;
		vbat_offs1 = pi_lcn40->vbat_offs1_2g;
		vbat_offs2 = pi_lcn40->vbat_offs2_2g;
	}

	if ((!temp_offs1) && (!temp_offs2) && (!vbat_offs1) && (!vbat_offs2))
		return;

	temper = wlc_lcn40phy_tempsense(pi, !TEMPER_VBAT_TRIGGER_NEW_MEAS);
	vbat =  wlc_lcn40phy_vbatsense(pi, !TEMPER_VBAT_TRIGGER_NEW_MEAS);

	wlc_lcn40phy_update_temp_vbat_backoff_boost(pi, temper,
	temp_offs1, temp_offs2, &(pi_lcn40->cond_offs1), &(pi_lcn40->cond_offs2),
	pi_lcn40->high_temp_threshold, pi_lcn40->low_temp_threshold, pi_lcn40->temp_diff);

	wlc_lcn40phy_update_temp_vbat_backoff_boost(pi, vbat,
	vbat_offs1, vbat_offs2, &(pi_lcn40->cond_offs3), &(pi_lcn40->cond_offs4),
	pi_lcn40->high_vbat_threshold, pi_lcn40->low_vbat_threshold, pi_lcn40->vbat_diff);
}

void wlc_lcn40phy_trigger_noise_iqest(phy_info_t* pi)
{
	phy_info_lcn40phy_t *pi_lcn40 = pi->u.pi_lcn40phy;
	if (pi_lcn40->noise_iqest_en) {
		uint16 lcn40phyregs_shm_addr =
			2 * wlapi_bmac_read_shm(pi->sh->physhim, M_LCN40PHYREGS_PTR);
		wlapi_bmac_write_shm(pi->sh->physhim, lcn40phyregs_shm_addr + M_NOISE_IQEST_EN, 1);
	}
}

int8
wlc_lcn40phy_get_noise_iqest_gainadjust(phy_info_t *pi)
{
	phy_info_lcn40phy_t *pi_lcn40 = pi->u.pi_lcn40phy;
	int8 gain_adjust;
	switch (wlc_phy_chanspec_bandrange_get(pi, pi->radio_chanspec)) {
		case WL_CHAN_FREQ_RANGE_2G:
			gain_adjust = pi_lcn40->noise_iqest_gain_adj_2g;
			break;
	#ifdef BAND5G
		case WL_CHAN_FREQ_RANGE_5GL:
			/* 5 GHz low */
			gain_adjust = pi_lcn40->noise_iqest_gain_adj_5gl;
			break;

		case WL_CHAN_FREQ_RANGE_5GM:
			/* 5 GHz middle */
			gain_adjust = pi_lcn40->noise_iqest_gain_adj_5gm;
			break;

		case WL_CHAN_FREQ_RANGE_5GH:
			/* 5 GHz high */
			gain_adjust = pi_lcn40->noise_iqest_gain_adj_5gh;
			break;
	#endif /* BAND5G */
		default:
			gain_adjust = 0;
			break;
	}
	return gain_adjust;
}
void
BCMATTACHFN(wlc_phy_interference_mode_attach_lcn40phy)(phy_info_t *pi)
{
	if (CHIPID(pi->sh->chip) == BCM43142_CHIP_ID) {
		pi->sh->interference_mode = pi->sh->interference_mode_2G = WLAN_AUTO_W_NOISE;
	}
}

void wlc_phy_adjust_ed_thres_lcn40phy(phy_info_t *pi, int32 *assert_thresh_dbm, bool set_threshold)
{
	/* Set the EDCRS Assert and De-assert Threshold
	* The de-assert threshold is set to 6dB lower then the assert threshold
	* A 6dB offset is observed during measurements between value set in the register
	* and the actual value in dBm i.e. if the desired ED threshold is -65dBm,
	* the Edon register has to be set to -71
	*/
	phy_info_lcn40phy_t *pi_lcn40 = pi->u.pi_lcn40phy;
	int8 assert_thres_val, de_assert_thresh_val;

	if (set_threshold == TRUE) {
		assert_thres_val = *assert_thresh_dbm - 6;
		de_assert_thresh_val = assert_thres_val - 6;
		/* Set the EDCRS Assert Threshold */
		pi_lcn40->edonthreshold40 = assert_thres_val;
		pi_lcn40->edonthreshold20U = assert_thres_val;
		pi_lcn40->edonthreshold20L = assert_thres_val;

		/* Set the EDCRS De-assert Threshold */
		pi_lcn40->edoffthreshold40 = de_assert_thresh_val;
		pi_lcn40->edoffthreshold20UL = de_assert_thresh_val;

		wlc_lcn40phy_set_ed_thres(pi);
	}
	else {
		assert_thres_val = pi_lcn40->edonthreshold20L + 6;
		*assert_thresh_dbm = assert_thres_val;
	}
}

static void
wlc_lcn40phy_set_ed_thres(phy_info_t *pi)
{
	phy_info_lcn40phy_t *pi_lcn40 = pi->u.pi_lcn40phy;

	PHY_REG_MOD(pi, LCN40PHY, edthresh40, edonthreshold40,
		pi_lcn40->edonthreshold40);
	PHY_REG_MOD(pi, LCN40PHY, edthresh40, edoffthreshold40,
		pi_lcn40->edoffthreshold40);
	PHY_REG_MOD(pi, LCN40PHY, edthresh20ul, edonthreshold20U,
		pi_lcn40->edonthreshold20U);
	PHY_REG_MOD(pi, LCN40PHY, edthresh20ul, edonthreshold20L,
		pi_lcn40->edonthreshold20L);
	PHY_REG_MOD(pi, LCN40PHY, crsedthresh, edoffthreshold,
		pi_lcn40->edoffthreshold20UL);
}

#ifdef SAMPLE_COLLECT
int8
phy_lcn40_sample_collect_gainadj(phy_info_t *pi, int8 gainadj, bool set)
{
	phy_info_lcn40phy_t *pi_lcn40 = pi->u.pi_lcn40phy;

	if (set)
		pi_lcn40->sample_collect_gainadj = gainadj;

	return pi_lcn40->sample_collect_gainadj;
}

uint8
phy_lcn40_sample_collect_gainidx(phy_info_t *pi, uint8 gainidx, bool set)
{
	phy_info_lcn40phy_t *pi_lcn40 = pi->u.pi_lcn40phy;

	if (set)
		pi_lcn40->sample_collect_gainidx = gainidx;

	return pi_lcn40->sample_collect_gainidx;
}
#endif /* SAMPLE_COLLECT */
#endif /* LCN40CONF != 0 */
