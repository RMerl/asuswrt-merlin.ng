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
 * $Id: wlc_phy_lcn.c 573258 2015-07-22 07:56:00Z $
 */

/* XXX WARNING: phy structure has been changed, read this first

 *
 * This submodule is for LCN phy only. It depends on the common submodule wlc_phy_cmn.c
 *   This file was created(split out of wlc_phy.c) on 3/5/2009.
 *   The full cvs history inherited from old wlc_phy.c. cvs annorate -r 1.2084 wlc_phy_lcn.c
 *   Refer to wlc_phy_cmn.c for new structure
 *
 * Future Improvement: cleanup code, restructuring to share some functions
 *              clean up ifdef, if LCNCONF
 *              refactor with wlc_phy_cmn.c
 */
#include <wlc_cfg.h>

#if ((LCNCONF != 0) || (LCN40CONF != 0))
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
#include <wlc_phy_lcn.h>
#include <sbchipc.h>

#include <phy_utils_channel.h>
#include <phy_utils_math.h>
#include <phy_utils_var.h>
#include <phy_utils_reg.h>

#include <wlc_phyreg_lcn.h>
#include <wlc_phytbl_lcn.h>

/* contains settings from BCM2064_JTAG.xls */
#define PLL_2064_NDIV		90
#define PLL_2064_LOW_END_VCO 	3000
#define PLL_2064_LOW_END_KVCO 	27
#define PLL_2064_HIGH_END_VCO	4200
#define PLL_2064_HIGH_END_KVCO	68
#define PLL_2064_LOOP_BW_DOUBLER	200
#define PLL_2064_D30_DOUBLER		10500	/* desired value of PLL_lf_r1(ohm) */
#define PLL_2064_LOOP_BW	260
#define PLL_2064_D30		8000	/* desired value of PLL_lf_r1(ohm) */
#define PLL_2064_CAL_REF_TO	8 	/* PLL cal ref timeout */
#define PLL_2064_MHZ		1000000
#define PLL_2064_OPEN_LOOP_DELAY	5

#define TEMPSENSE 			1
#define VBATSENSE           2

#define PAPD_BLANKING_PROFILE 		3
#define PAPD2LUT			0
#define PAPD_CORR_NORM 			0
#define PAPD_NUM_SKIP_COUNT 52
#define PAPD_BLANKING_THRESHOLD 	0

#define LCN_TARGET_TSSI  30
#define LCN_TARGET_PWR  60

#define LCN_VBAT_SCALE_NOM  53	/* 3.3 * (2 ^ 4) */
#define LCN_VBAT_SCALE_DEN  432

#define LCN_TEMPSENSE_OFFSET  80812 /* 78.918 << 10 */
#define LCN_TEMPSENSE_DEN  2647	/* 2.5847 << 10 */

#define IDLE_TSSI_PER_CAL_EN 1

#define TWO_POWER_RANGE_TXPWR_CTRL 1

#define LCN_BCK_TSSI_UCODE_POST 1
#define LCN_BCK_TSSI_DEBUG_LUT_DIRECT !LCN_BCK_TSSI_UCODE_POST
#if LCN_BCK_TSSI_DEBUG_LUT_DIRECT
#define LCN_BCK_TSSI_DEBUG_VIA_DUMMY 0
#endif // endif
#define LCN_BCK_TSSI_DEBUG 0

#define LCNPHY_TSSI_SET_MAX_LIMIT 1
#define LCNPHY_TSSI_SET_MIN_LIMIT 2
#define LCNPHY_TSSI_SET_MIN_MAX_LIMIT 3

#define RXIQEST_TIMEOUT 500 /* Timeout in ms */

#define DOT11LCNPHY_TX_GAIN_TABLE_SZ 128

/* %%%%%% LCNPHY macros/structure */

#define LCNPHY_txgainctrlovrval1_pagain_ovr_val1_SHIFT \
	(LCNPHY_txgainctrlovrval1_txgainctrl_ovr_val1_SHIFT + 8)
#define LCNPHY_txgainctrlovrval1_pagain_ovr_val1_MASK \
	(0x7f << LCNPHY_txgainctrlovrval1_pagain_ovr_val1_SHIFT)

#define LCNPHY_stxtxgainctrlovrval1_pagain_ovr_val1_SHIFT \
	(LCNPHY_stxtxgainctrlovrval1_stxtxgainctrl_ovr_val1_SHIFT + 8)
#define LCNPHY_stxtxgainctrlovrval1_pagain_ovr_val1_MASK \
	(0x7f << LCNPHY_stxtxgainctrlovrval1_pagain_ovr_val1_SHIFT)

#define wlc_lcnphy_enable_tx_gain_override(pi) \
	wlc_lcnphy_set_tx_gain_override(pi, TRUE)
#define wlc_lcnphy_disable_tx_gain_override(pi) \
	wlc_lcnphy_set_tx_gain_override(pi, FALSE)

/* Turn off all the crs signals to the MAC */
#define wlc_lcnphy_iqcal_active(pi)	\
	(phy_utils_read_phyreg((pi), LCNPHY_iqloCalCmd) & \
	(LCNPHY_iqloCalCmd_iqloCalCmd_MASK | LCNPHY_iqloCalCmd_iqloCalDFTCmd_MASK))

#define txpwrctrl_off(pi) (0x3 != ((phy_utils_read_phyreg(pi, LCNPHY_TxPwrCtrlCmd) & 0xC000) >> 14))

#if defined(PHY_TEMPPWRCTRL_ENABLE)
#define wlc_lcnphy_tempsense_based_pwr_ctrl_enabled(pi)		PHY_TEMPPWRCTRL_ENABLE
#else
#define wlc_lcnphy_tempsense_based_pwr_ctrl_enabled(pi) \
	(pi->u.pi_lcnphy->temppwrctrl_capable)
#endif // endif

#define wlc_lcnphy_tssi_based_pwr_ctrl_enabled(pi) \
	(pi->hwpwrctrl_capable)

/* Max Num of registers for Save/Restore LUTs for Fast TSSI */
/* Multiplied by 2 because LUTs store address and value */
#define MAX_NUM_PHY_REGS_FAST_TSSI_UCODE 	(13 * 2)
#define MAX_NUM_RADIO_REGS_FAST_TSSI_UCODE  (15 * 2)
#define MAX_NUM_PHY_REGS_FAST_TSSI			(20 * 2)
#define MAX_NUM_RADIO_REGS_FAST_TSSI		(20 * 2)

#define SWCTRL_BT_TX		0x18
#define SWCTRL_OVR_DISABLE	0x40

#define	AFE_CLK_INIT_MODE_TXRX2X	1
#define	AFE_CLK_INIT_MODE_PAPD		0

#define LCNPHY_TBL_ID_IQLOCAL			0x00
/* #define LCNPHY_TBL_ID_TXPWRCTL 		0x07 move to wlc_phy_int.h */
#define LCNPHY_TBL_ID_RFSEQ         0x08
#define LCNPHY_TBL_ID_GAIN_IDX		0x0d
#define LCNPHY_TBL_ID_AUX_GAIN_IDX		0x0e
#define LCNPHY_TBL_ID_SW_CTRL			0x0f
#define LCNPHY_TBL_ID_NF_CTRL			0x10
#define LCNPHY_TBL_ID_GAIN_TBL		0x12
#define LCNPHY_TBL_ID_GAIN_VAL_TBL	17

#define LCNPHY_TBL_ID_SPUR			0x14
#define LCNPHY_TBL_ID_SAMPLEPLAY		0x15
#define LCNPHY_TBL_ID_SAMPLEPLAY1		0x16

#define LCNPHY_TX_PWR_CTRL_RATE_OFFSET 	832
#define LCNPHY_TX_PWR_CTRL_MAC_OFFSET 	128
#define LCNPHY_TX_PWR_CTRL_GAIN_OFFSET 	192
#define LCNPHY_TX_PWR_CTRL_IQ_OFFSET		320
#define LCNPHY_TX_PWR_CTRL_LO_OFFSET		448
#define LCNPHY_TX_PWR_CTRL_PWR_OFFSET		576
#define LCNPHY_TX_PWR_CTRL_EST_PWR_OFFSET	704

#define LCNPHY_TX_PWR_CTRL_START_NPT		1
#define LCNPHY_TX_PWR_CTRL_MAX_NPT		2

#define PHY_NOISE_SAMPLES_DEFAULT 5000

#define LCNPHY_ACI_DETECT_START      1
#define LCNPHY_ACI_DETECT_PROGRESS   2
#define LCNPHY_ACI_DETECT_STOP       3

#define LCNPHY_ACI_CRSHIFRMLO_TRSH 100
#define LCNPHY_ACI_GLITCH_TRSH 2000
#define	LCNPHY_ACI_TMOUT 250		/* Time for CRS HI and FRM LO (in micro seconds) */
#define LCNPHY_ACI_DETECT_TIMEOUT  2	/* in  seconds */
#define LCNPHY_ACI_START_DELAY 0

#define LCNPHY_MAX_CAL_CACHE	   2	/* Max number of cal cache contexts reqd */

#define LCNPHY_TX_PWR_CTRL_START_INDEX_2G	90
#define LCNPHY_TX_PWR_CTRL_START_INDEX_5G	80

#define wlc_lcnphy_tx_gain_override_enabled(pi) \
	((phy_utils_read_phyreg((pi), LCNPHY_AfeCtrlOvr) & \
	  LCNPHY_AfeCtrlOvr_dacattctrl_ovr_MASK) != 0)

typedef struct {
	uint16 gm_gain;
	uint16 pga_gain;
	uint16 pad_gain;
	uint16 dac_gain;
} lcnphy_txgains_t;

typedef enum {
	LCNPHY_CAL_FULL,
	LCNPHY_CAL_RECAL,
	LCNPHY_CAL_CURRECAL,
	LCNPHY_CAL_DIGCAL,
	LCNPHY_CAL_GCTRL,
	LCNPHY_CAL_IQ_RECAL,
	LCNPHY_CAL_TXPWRCTRL,
	LCNPHY_CAL_DLO_RECAL
} lcnphy_cal_mode_t;

typedef struct {
	lcnphy_txgains_t gains;
	bool useindex;
	uint8 index;
} lcnphy_txcalgains_t;

typedef struct {
	uint8 chan;
	int16 a;
	int16 b;
} lcnphy_rx_iqcomp_t;

typedef struct {
	int16 re;
	int16 im;
} lcnphy_spb_tone_t;

typedef struct {
	uint16 re;
	uint16 im;
} lcnphy_unsign16_struct;

/* LCNPHY IQCAL parameters for various Tx gain settings */
/* table format: */
/*	target, gm, pga, pad, ncorr for each of 5 cal types */
typedef uint16 iqcal_gain_params_lcnphy[9];

static const iqcal_gain_params_lcnphy tbl_iqcal_gainparams_lcnphy_2G[] = {
	{0, 0, 0, 0, 0, 0, 0, 0, 0},
	};

#ifdef BAND5G
static const iqcal_gain_params_lcnphy tbl_iqcal_gainparams_lcnphy_5G[] = {
	{0, 7, 14, 14, 0, 0, 0, 0, 0},
	};
static const iqcal_gain_params_lcnphy *tbl_iqcal_gainparams_lcnphy[2] = {
	tbl_iqcal_gainparams_lcnphy_2G,
	tbl_iqcal_gainparams_lcnphy_5G
	};
static const uint16 iqcal_gainparams_numgains_lcnphy[2] = {
	sizeof(tbl_iqcal_gainparams_lcnphy_2G) / sizeof(*tbl_iqcal_gainparams_lcnphy_2G),
	sizeof(tbl_iqcal_gainparams_lcnphy_5G) / sizeof(*tbl_iqcal_gainparams_lcnphy_5G)
	};
#else
static const iqcal_gain_params_lcnphy *tbl_iqcal_gainparams_lcnphy[1] = {
	tbl_iqcal_gainparams_lcnphy_2G,
	};

static const uint16 iqcal_gainparams_numgains_lcnphy[1] = {
	sizeof(tbl_iqcal_gainparams_lcnphy_2G) / sizeof(*tbl_iqcal_gainparams_lcnphy_2G),
	};
#endif /* BAND5G */

/* LO Comp Gain ladder. Format: {m genv} */
static const
uint16 lcnphy_iqcal_loft_gainladder[]  = {
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
uint16 lcnphy_iqcal_ir_gainladder[] = {
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

/* Image Rejection Gain ladder. Format: {m genv} */
static const
uint16 lcnphy_iqcal_common_gainladder[] = {
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

/* reference waveform autogenerated , freq 3.75 MHz, amplitude 88 */
static const
lcnphy_spb_tone_t lcnphy_spb_tone_3750[] = {
	{88, 0	 },
	{73, 49	 },
	{34, 81	 },
	{-17, 86 },
	{-62, 62 },
	{-86, 17 },
	{-81, -34 },
	{-49, -73 },
	{0, -88 },
	{49, -73 },
	{81, -34 },
	{86, 17 },
	{62, 62 },
	{17, 86 },
	{-34, 81 },
	{-73, 49 },
	{-88, 0 },
	{-73, -49 },
	{-34, -81 },
	{17, -86 },
	{62, -62 },
	{86, -17 },
	{81, 34	 },
	{49, 73	 },
	{0, 88 },
	{-49, 73 },
	{-81, 34 },
	{-86, -17 },
	{-62, -62 },
	{-17, -86 },
	{34, -81 },
	{73, -49 },
	};

/* these rf registers need to be restored after iqlo_soft_cal_full */
static const
uint16 iqlo_loopback_rf_regs[] = {
	RADIO_2064_REG036,
	RADIO_2064_REG11A,
	RADIO_2064_REG03A,
	RADIO_2064_REG025,
	RADIO_2064_REG028,
	RADIO_2064_REG005,
	RADIO_2064_REG112,
	RADIO_2064_REG0FF,
	RADIO_2064_REG11F,
	RADIO_2064_REG00B,
	RADIO_2064_REG113,
	RADIO_2064_REG007,
	RADIO_2064_REG0FC,
	RADIO_2064_REG0FD,
	RADIO_2064_REG0FF,
	RADIO_2064_REG0C0,
	RADIO_2064_REG0CA,
	RADIO_2064_REG0C5,
	RADIO_2064_REG012,
	RADIO_2064_REG057,
	RADIO_2064_REG059,
	RADIO_2064_REG05C,
	RADIO_2064_REG078,
	RADIO_2064_REG092,
	};

static const
uint16 tempsense_phy_regs[] = {
	LCNPHY_auxadcCtrl,
	LCNPHY_TxPwrCtrlCmd,
	LCNPHY_TxPwrCtrlRangeCmd,
	LCNPHY_TxPwrCtrlRfCtrl0,
	LCNPHY_TxPwrCtrlRfCtrl1,
	LCNPHY_TxPwrCtrlIdleTssi,
	LCNPHY_rfoverride4,
	LCNPHY_rfoverride4val,
	LCNPHY_TxPwrCtrlRfCtrlOverride1,
	LCNPHY_TxPwrCtrlRangeCmd,
	LCNPHY_TxPwrCtrlRfCtrlOverride0,
	LCNPHY_TxPwrCtrlNnum,
	LCNPHY_TxPwrCtrlNum_Vbat,
	LCNPHY_TxPwrCtrlNum_temp,
	};

/* these rf registers need to be restored after rxiq_cal */
static const
uint16 rxiq_cal_rf_reg[] = {
	RADIO_2064_REG098,
	RADIO_2064_REG116,
	RADIO_2064_REG12C,
	RADIO_2064_REG06A,
	RADIO_2064_REG00B,
	RADIO_2064_REG01B,
	RADIO_2064_REG113,
	RADIO_2064_REG01D,
	RADIO_2064_REG114,
	RADIO_2064_REG02E,
	RADIO_2064_REG12A,
	RADIO_2064_REG112,
#ifdef BAND5G
	RADIO_2064_REG0ED,
	RADIO_2064_REG0EC,
#endif // endif
	};
/* These rf registers need to be restored after tone power calculation */
static const
uint16 tone_power_rf_reg[] = {
	RADIO_2064_REG007,
	RADIO_2064_REG0FF,
	RADIO_2064_REG11F,
	RADIO_2064_REG03A,
	RADIO_2064_REG11A,
	RADIO_2064_REG005,
	RADIO_2064_REG082,
	RADIO_2064_REG086,
	RADIO_2064_REG12E,
	RADIO_2064_REG113,
	RADIO_2064_REG07D,
	RADIO_2064_REG028,
#ifdef BAND5G
	RADIO_2064_REG0CA,
	RADIO_2064_REG126,
	RADIO_2064_REG0EB,
	RADIO_2064_REG0C0,
#endif // endif
	};
/* Autogenerated by 2064_chantbl_tcl2c.tcl */
static const
lcnphy_rx_iqcomp_t lcnphy_rx_iqcomp_table_rev0[] = {
	{ 1, 0, 0 },
	{ 2, 0, 0 },
	{ 3, 0, 0 },
	{ 4, 0, 0 },
	{ 5, 0, 0 },
	{ 6, 0, 0 },
	{ 7, 0, 0 },
	{ 8, 0, 0 },
	{ 9, 0, 0 },
	{ 10, 0, 0 },
	{ 11, 0, 0 },
	{ 12, 0, 0 },
	{ 13, 0, 0 },
	{ 14, 0, 0 },
	{ 34, 0, 0 },
	{ 38, 0, 0 },
	{ 42, 0, 0 },
	{ 46, 0, 0 },
	{ 36, 0, 0 },
	{ 40, 0, 0 },
	{ 44, 0, 0 },
	{ 48, 0, 0 },
	{ 52, 0, 0 },
	{ 56, 0, 0 },
	{ 60, 0, 0 },
	{ 64, 0, 0 },
	{ 100, 0, 0 },
	{ 104, 0, 0 },
	{ 108, 0, 0 },
	{ 112, 0, 0 },
	{ 116, 0, 0 },
	{ 120, 0, 0 },
	{ 124, 0, 0 },
	{ 128, 0, 0 },
	{ 132, 0, 0 },
	{ 136, 0, 0 },
	{ 140, 0, 0 },
	{ 149, 0, 0 },
	{ 153, 0, 0 },
	{ 157, 0, 0 },
	{ 161, 0, 0 },
	{ 165, 0, 0 },
	{ 184, 0, 0 },
	{ 188, 0, 0 },
	{ 192, 0, 0 },
	{ 196, 0, 0 },
	{ 200, 0, 0 },
	{ 204, 0, 0 },
	{ 208, 0, 0 },
	{ 212, 0, 0 },
	{ 216, 0, 0 },
	};

/* The 23bit gaincode has been constructed like this - */
/* 22(extlna),21(tr),(20:2)19bit gain code, (1-0) lna1 */
static const uint32 lcnphy_23bitgaincode_table[] = {
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000100,
	0x00000001,
	0x00000101,
	0x00000201,
	0x00000011,
	0x00000111,
	0x00000211,
	0x00000311,
	0x00001311,
	0x00002311,
	0x00000313,
	0x00001313,
	0x00002313,
	0x00003313,
	0x00001333,
	0x00002333,
	0x00003333,
	0x00003433,
	0x00004433,
	0x00014433,
	0x00024433,
	0x00025433,
	0x00035433,
	0x00045433,
	0x00046433,
	0x00146433,
	0x00246433,
	0x00346433,
	0x00446433
};

static const int8 lcnphy_gain_table[] = {
	-16,
	-13,
	10,
	7,
	4,
	0,
	3,
	6,
	9,
	12,
	15,
	18,
	21,
	24,
	27,
	30,
	33,
	36,
	39,
	42,
	45,
	48,
	50,
	53,
	56,
	59,
	62,
	65,
	68,
	71,
	74,
	77,
	80,
	83,
	86,
	89,
	92,
	};

static const int8 lcnphy_gain_index_offset_for_rssi[] = {
	7,	/* 0 */
	7,	/* 1 */
	7,	/* 2 */
	7,	/* 3 */
	7,	/* 4 */
	7,	/* 5 */
	7,	/* 6 */
	8,	/* 7 */
	7,	/* 8 */
	7,	/* 9 */
	6,	/* 10 */
	7,	/* 11 */
	7,	/* 12 */
	4,	/* 13 */
	4,	/* 14 */
	4,	/* 15 */
	4,	/* 16 */
	4,	/* 17 */
	4,	/* 18 */
	4,	/* 19 */
	4,	/* 20 */
	3,	/* 21 */
	3,	/* 22 */
	3,	/* 23 */
	3,	/* 24 */
	3,	/* 25 */
	3,	/* 26 */
	4,	/* 27 */
	2,	/* 28 */
	2,	/* 29 */
	2,	/* 30 */
	2,	/* 31 */
	2,	/* 32 */
	2,	/* 33 */
	-1,	/* 34 */
	-2,	/* 35 */
	-2,	/* 36 */
	-2	/* 37 */
};

/* channel info type for 2064 radio used in lcnphy */
typedef struct _chan_info_2064_lcnphy {
	uint   chan;            /* channel number */
	uint   freq;            /* in Mhz */
	uint8 logen_buftune;
	uint8 logen_rccr_tx;
	uint8 txrf_mix_tune_ctrl;
	uint8 pa_input_tune_g;
	uint8 logen_rccr_rx;
	uint8 pa_rxrf_lna1_freq_tune;
	uint8 pa_rxrf_lna2_freq_tune;
	uint8 rxrf_rxrf_spare1;
} chan_info_2064_lcnphy_t;

/* Autogenerated by 2064_chantbl_tcl2c.tcl */
static chan_info_2064_lcnphy_t chan_info_2064_lcnphy[] = {
{   1, 2412, 0x0B, 0x0A, 0x00, 0x07, 0x0A, 0x88, 0x88, 0x80 },
{   2, 2417, 0x0B, 0x0A, 0x00, 0x07, 0x0A, 0x88, 0x88, 0x80 },
{   3, 2422, 0x0B, 0x0A, 0x00, 0x07, 0x0A, 0x88, 0x88, 0x80 },
{   4, 2427, 0x0B, 0x0A, 0x00, 0x07, 0x0A, 0x88, 0x88, 0x80 },
{   5, 2432, 0x0B, 0x0A, 0x00, 0x07, 0x0A, 0x88, 0x88, 0x80 },
{   6, 2437, 0x0B, 0x0A, 0x00, 0x07, 0x0A, 0x88, 0x88, 0x80 },
{   7, 2442, 0x0B, 0x0A, 0x00, 0x07, 0x0A, 0x88, 0x88, 0x80 },
{   8, 2447, 0x0B, 0x0A, 0x00, 0x07, 0x0A, 0x88, 0x88, 0x80 },
{   9, 2452, 0x0B, 0x0A, 0x00, 0x07, 0x0A, 0x88, 0x88, 0x80 },
{  10, 2457, 0x0B, 0x0A, 0x00, 0x07, 0x0A, 0x88, 0x88, 0x80 },
{  11, 2462, 0x0B, 0x0A, 0x00, 0x07, 0x0A, 0x88, 0x88, 0x80 },
{  12, 2467, 0x0B, 0x0A, 0x00, 0x07, 0x0A, 0x88, 0x88, 0x80 },
{  13, 2472, 0x0B, 0x0A, 0x00, 0x07, 0x0A, 0x88, 0x88, 0x80 },
{  14, 2484, 0x0B, 0x0A, 0x00, 0x07, 0x0A, 0x88, 0x88, 0x80 },
};

/* channel info type for 2066 radio used in lcnphy */
typedef struct _chan_info_2066_lcnphy {
	uint   chan;            /* channel number */
	uint   freq;            /* in Mhz */
	uint8 logen_buftune;
	uint8 logen_rccr_tx;
	uint8 txrf_mix_tune_ctrl;
	uint8 pa_input_tune_g;
	uint8 pa_rxrf_lna1_freq_tune;
	uint8 rxrf_rxrf_spare1;
	uint8 pad;
	uint8 pad1;
} chan_info_2066_lcnphy_t;

/* a band channel info type for 2066 radio used in lcnphy */
typedef struct _chan_info_2066_a_lcnphy {
	uint   chan;            /* channel number */
	uint   freq;            /* in Mhz */
	uint8 logen_buftune;
	uint8 ctune;
	uint8 logen_rccr;
	uint8 mix_tune_5g;
	uint8 pga_tune_ctrl;
	uint8 pa_input_tune_5g;
	uint8 pa_rxrf_lna1_freq_tune;
	uint8 rxrf_rxrf_spare1;
} chan_info_2066_a_lcnphy_t;

/* Autogenerated by 2066_chantbl_tcl2c.tcl */
static chan_info_2066_lcnphy_t chan_info_2066_lcnphy[] = {
{   1, 2412, 0x01, 0x0A, 0x00, 0x07, 0x0F, 0x01, 0x00, 0x00 },
{   2, 2417, 0x01, 0x0A, 0x00, 0x07, 0x0F, 0x01, 0x00, 0x00 },
{   3, 2422, 0x01, 0x0A, 0x00, 0x07, 0x0F, 0x01, 0x00, 0x00 },
{   4, 2427, 0x01, 0x0A, 0x00, 0x07, 0x0F, 0x01, 0x00, 0x00 },
{   5, 2432, 0x01, 0x0A, 0x00, 0x07, 0x0F, 0x01, 0x00, 0x00 },
{   6, 2437, 0x01, 0x0A, 0x00, 0x07, 0x0F, 0x01, 0x00, 0x00 },
{   7, 2442, 0x01, 0x0A, 0x00, 0x07, 0x0F, 0x01, 0x00, 0x00 },
{   8, 2447, 0x01, 0x0A, 0x00, 0x07, 0x0F, 0x01, 0x00, 0x00 },
{   9, 2452, 0x01, 0x0A, 0x00, 0x07, 0x0F, 0x01, 0x00, 0x00 },
{  10, 2457, 0x01, 0x0A, 0x00, 0x07, 0x0F, 0x01, 0x00, 0x00 },
{  11, 2462, 0x01, 0x0A, 0x00, 0x07, 0x0F, 0x01, 0x00, 0x00 },
{  12, 2467, 0x01, 0x0A, 0x00, 0x07, 0x0F, 0x01, 0x00, 0x00 },
{  13, 2472, 0x01, 0x0A, 0x00, 0x07, 0x0F, 0x01, 0x00, 0x00 },
{  14, 2484, 0x01, 0x0A, 0x00, 0x07, 0x0F, 0x01, 0x00, 0x00 },
#if defined(BAND5G)
/* New tunning for reg 0xAD */
{  34, 5170, 0x09, 0x98, 0x99, 0x0D, 0x0F, 0x0F, 0xED, 0x0F  },
{  38, 5190, 0x09, 0x98, 0x99, 0x0D, 0x0F, 0x0F, 0xFF, 0x0F  },
{  42, 5210, 0x09, 0x87, 0x99, 0x0A, 0x0F, 0x0C, 0xFE, 0x0F  },
{  46, 5230, 0x08, 0x87, 0x99, 0x0A, 0x0F, 0x0C, 0xFE, 0x0F  },
{  36, 5180, 0x09, 0x98, 0x99, 0x0D, 0x0F, 0x0F, 0xFF, 0x0F  },
{  40, 5200, 0x09, 0x87, 0x99, 0x0A, 0x0F, 0x0C, 0xFF, 0x0F  },
{  44, 5220, 0x08, 0x87, 0x99, 0x0A, 0x0F, 0x0C, 0xFE, 0x0F  },
{  48, 5240, 0x08, 0x87, 0x99, 0x0A, 0x0F, 0x0C, 0xFE, 0x0F  },
{  52, 5260, 0x08, 0x87, 0x99, 0x0A, 0x0F, 0x0C, 0xFE, 0x0F  },
{  56, 5280, 0x08, 0x77, 0x99, 0x0A, 0x0F, 0x0C, 0xFD, 0x0E  },
{  60, 5300, 0x07, 0x77, 0x99, 0x0A, 0x0F, 0x0C, 0xFD, 0x0E  },
{  64, 5320, 0x07, 0x77, 0x99, 0x0A, 0x0F, 0x0C, 0xFC, 0x0E  },
{ 100, 5500, 0x05, 0x66, 0xAA, 0x05, 0x08, 0x06, 0x97, 0x0B  },
{ 104, 5520, 0x04, 0x66, 0xAA, 0x05, 0x08, 0x06, 0x97, 0x0B  },
{ 108, 5540, 0x04, 0x66, 0xAA, 0x05, 0x08, 0x06, 0x76, 0x0B  },
{ 112, 5560, 0x04, 0x66, 0xAA, 0x05, 0x08, 0x06, 0x65, 0x0A  },
{ 116, 5580, 0x04, 0x66, 0xAA, 0x05, 0x08, 0x06, 0x64, 0x0A  },
{ 120, 5600, 0x04, 0x66, 0xAA, 0x05, 0x08, 0x06, 0x53, 0x0A  },
{ 124, 5620, 0x04, 0x55, 0xAA, 0x05, 0x08, 0x06, 0x42, 0x09  },
{ 128, 5640, 0x03, 0x55, 0xAA, 0x05, 0x08, 0x06, 0x31, 0x09  },
{ 132, 5660, 0x03, 0x55, 0xBB, 0x05, 0x08, 0x06, 0x20, 0x09  },
{ 136, 5680, 0x03, 0x55, 0xBB, 0x05, 0x08, 0x06, 0x20, 0x09  },
{ 140, 5700, 0x03, 0x55, 0xBB, 0x02, 0x04, 0x03, 0x10, 0x08  },
{ 149, 5745, 0x02, 0x44, 0xBB, 0x02, 0x04, 0x03, 0x00, 0x07  },
{ 153, 5765, 0x02, 0x44, 0xBB, 0x02, 0x04, 0x03, 0x00, 0x07  },
{ 157, 5785, 0x02, 0x44, 0xBB, 0x02, 0x04, 0x03, 0x00, 0x06  },
{ 161, 5805, 0x02, 0x44, 0xBB, 0x00, 0x01, 0x00, 0x00, 0x06  },
{ 165, 5825, 0x02, 0x33, 0xBB, 0x00, 0x01, 0x00, 0x00, 0x05  },
{ 184, 4920, 0x0E, 0xCB, 0x88, 0x0F, 0x0F, 0x0F, 0xFF, 0x0F  },
{ 188, 4940, 0x0E, 0xCB, 0x88, 0x0F, 0x0F, 0x0F, 0xFF, 0x0F  },
{ 192, 4960, 0x0E, 0xCB, 0x88, 0x0F, 0x0F, 0x0F, 0xFF, 0x0F  },
{ 196, 4980, 0x0D, 0xBA, 0x88, 0x0F, 0x0F, 0x0F, 0xFF, 0x0F  },
{ 200, 5000, 0x0D, 0xBA, 0x88, 0x0F, 0x0F, 0x0F, 0xFF, 0x0F  },
{ 204, 5020, 0x0D, 0xBA, 0x99, 0x0F, 0x0F, 0x0F, 0xFF, 0x0F  },
{ 208, 5040, 0x0C, 0xA9, 0x99, 0x0F, 0x0F, 0x0F, 0xFF, 0x0F  },
{ 212, 5060, 0x0C, 0xA9, 0x99, 0x0D, 0x0F, 0x0F, 0xED, 0x0F  },
{ 216, 5080, 0x0B, 0xA9, 0x99, 0x0D, 0x0F, 0x0F, 0xED, 0x0F  },
#endif /* BAND5G : New tunning for reg 0xAD */
};

#if defined(WLTEST)
typedef struct _chan_info_2066_lcnphy_lna_corr {
	uint8   chan;            /* channel number */
	int8   corr_qdBm;        /* Correction in qdBm */
} chan_info_2066_lcnphy_lna_corr_t;

static chan_info_2066_lcnphy_lna_corr_t chan_info_2066_lcnphy_lna_corr[] = {
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

/* Init values for 2066 regs (autogenerated by 2066_regs_tcl2c.tcl)
 *   Entries: addr, init value A, init value G, do_init A, do_init G.
 *   Last line (addr FF is dummy as delimiter. This table has dual use
 *   between dumping and initializing.
 */
lcnphy_radio_regs_t WLBANDINITDATA(lcnphy_radio_regs_2066)[] = {
	{ 0x05,             0,           0x8,   0,   1  },
	{ 0x0A,          0x37,          0x77,   0,   1  },
	{ 0x0C,          0x55,          0xaa,   0,   1  },
	{ 0x1D,           0x1,           0x3,   0,   1  },
	{ 0x1E,          0x12,           0x2,   0,   1  },
	{ 0x2A,           0xb,           0xf,   0,   1  },
	{ 0x2C,           0x3,             0,   0,   1  },
	{ 0x38,           0x3,           0x7,   0,   1  },
	{ 0x5E,          0x88,           0xf,   0,   1  },
	{ 0x5F,          0x6c,          0xac,   0,   1  },
	{ 0x6B,          0x7f,             0,   0,   1  },
	{ 0x73,             0,           0x1,   0,   1  },
	{ 0x91,           0x5,           0x4,   0,   1  },
	{ 0xAC,          0x57,           0xf,   0,   1  },
	{ 0xB2,          0x1b,          0x3b,   0,   1  },
	{ 0xB4,          0x24,          0x27,   0,   1  },
	{ 0xB6,          0x1b,          0x3b,   0,   1  },
	{ 0xB7,          0x24,          0x27,   0,   1  },
	{ 0xB8,           0x3,           0x7,   0,   1  },
	{ 0xD8,           0x8,          0x7f,   0,   1  },
	{ 0xD9,           0x8,          0x7f,   0,   1  },
	{ 0xDA,           0x8,          0x7f,   0,   1  },
	{ 0xEC,          0x22,          0x11,   0,   1  },
	{ 0x105,            0,           0x8,   0,   1  },
	{ 0xFFFF,           0,             0,   0,   0  },
};

/* Init values for 2066 regs (autogenerated by 2066_regs_tcl2c.tcl)
 *   Entries: addr, init value A, init value G, do_init A, do_init G.
 *   Last line (addr FF is dummy as delimiter. This table has dual use
 *   between dumping and initializing.
 */
#if defined(BCMDBG) || defined(WLTEST)
lcnphy_radio_regs_t WLBANDINITDATA(lcnphy_radio_regs_2064)[] = {
	{ 0x00,             0,             0,   0,   0  },
	{ 0x01,          0x64,          0x64,   0,   0  },
	{ 0x02,          0x20,          0x20,   0,   0  },
	{ 0x03,          0x66,          0x66,   0,   0  },
	{ 0x04,          0xf8,          0xf8,   0,   0  },
	{ 0x05,             0,             0,   0,   0  },
	{ 0x06,          0x10,          0x10,   0,   0  },
	{ 0x07,             0,             0,   0,   0  },
	{ 0x08,             0,             0,   0,   0  },
	{ 0x09,             0,             0,   0,   0  },
	{ 0x0A,          0x37,          0x37,   0,   0  },
	{ 0x0B,           0x6,           0x6,   0,   0  },
	{ 0x0C,          0x55,          0x55,   0,   0  },
	{ 0x0D,          0x8b,          0x8b,   0,   0  },
	{ 0x0E,             0,             0,   0,   0  },
	{ 0x0F,           0x5,           0x5,   0,   0  },
	{ 0x10,             0,             0,   0,   0  },
	{ 0x11,           0xe,           0xe,   0,   0  },
	{ 0x12,             0,             0,   0,   0  },
	{ 0x13,           0xb,           0xb,   0,   0  },
	{ 0x14,           0x2,           0x2,   0,   0  },
	{ 0x15,          0x12,          0x12,   0,   0  },
	{ 0x16,          0x12,          0x12,   0,   0  },
	{ 0x17,           0xc,           0xc,   0,   0  },
	{ 0x18,           0xc,           0xc,   0,   0  },
	{ 0x19,           0xc,           0xc,   0,   0  },
	{ 0x1A,           0x8,           0x8,   0,   0  },
	{ 0x1B,           0x2,           0x2,   0,   0  },
	{ 0x1C,             0,             0,   0,   0  },
	{ 0x1D,           0x1,           0x1,   0,   0  },
	{ 0x1E,          0x12,          0x12,   0,   0  },
	{ 0x1F,          0x6e,          0x6e,   0,   0  },
	{ 0x20,           0x2,           0x2,   0,   0  },
	{ 0x21,          0x23,          0x23,   0,   0  },
	{ 0x22,           0x8,           0x8,   0,   0  },
	{ 0x23,             0,             0,   0,   0  },
	{ 0x24,             0,             0,   0,   0  },
	{ 0x25,           0xc,           0xc,   0,   0  },
	{ 0x26,          0x33,          0x33,   0,   0  },
	{ 0x27,          0x55,          0x55,   0,   0  },
	{ 0x28,             0,             0,   0,   0  },
	{ 0x29,          0x30,          0x30,   0,   0  },
	{ 0x2A,           0xb,           0xb,   0,   0  },
	{ 0x2B,          0x1b,          0x1b,   0,   0  },
	{ 0x2C,           0x3,           0x3,   0,   0  },
	{ 0x2D,          0x1b,          0x1b,   0,   0  },
	{ 0x2E,             0,             0,   0,   0  },
	{ 0x2F,          0x20,          0x20,   0,   0  },
	{ 0x30,           0xa,           0xa,   0,   0  },
	{ 0x31,             0,             0,   0,   0  },
	{ 0x32,          0x62,          0x62,   0,   0  },
	{ 0x33,          0x19,          0x19,   0,   0  },
	{ 0x34,          0x33,          0x33,   0,   0  },
	{ 0x35,          0x77,          0x77,   0,   0  },
	{ 0x36,             0,             0,   0,   0  },
	{ 0x37,          0x70,          0x70,   0,   0  },
	{ 0x38,           0x3,           0x3,   0,   0  },
	{ 0x39,           0xf,           0xf,   0,   0  },
	{ 0x3A,           0x6,           0x6,   0,   0  },
	{ 0x3B,          0xcf,          0xcf,   0,   0  },
	{ 0x3C,          0x1a,          0x1a,   0,   0  },
	{ 0x3D,           0x6,           0x6,   0,   0  },
	{ 0x3E,          0x42,          0x42,   0,   0  },
	{ 0x3F,             0,             0,   0,   0  },
	{ 0x40,          0xfb,          0xfb,   0,   0  },
	{ 0x41,          0x9a,          0x9a,   0,   0  },
	{ 0x42,          0x7a,          0x7a,   0,   0  },
	{ 0x43,          0x29,          0x29,   0,   0  },
	{ 0x44,             0,             0,   0,   0  },
	{ 0x45,           0x8,           0x8,   0,   0  },
	{ 0x46,          0xce,          0xce,   0,   0  },
	{ 0x47,          0x27,          0x27,   0,   0  },
	{ 0x48,          0x62,          0x62,   0,   0  },
	{ 0x49,           0x6,           0x6,   0,   0  },
	{ 0x4A,          0x58,          0x58,   0,   0  },
	{ 0x4B,          0xf7,          0xf7,   0,   0  },
	{ 0x4C,             0,             0,   0,   0  },
	{ 0x4D,          0xb3,          0xb3,   0,   0  },
	{ 0x4E,             0,             0,   0,   0  },
	{ 0x4F,           0x2,           0x2,   0,   0  },
	{ 0x50,             0,             0,   0,   0  },
	{ 0x51,           0x9,           0x9,   0,   0  },
	{ 0x52,           0x5,           0x5,   0,   0  },
	{ 0x53,          0x17,          0x17,   0,   0  },
	{ 0x54,          0x38,          0x38,   0,   0  },
	{ 0x55,             0,             0,   0,   0  },
	{ 0x56,             0,             0,   0,   0  },
	{ 0x57,           0xb,           0xb,   0,   0  },
	{ 0x58,             0,             0,   0,   0  },
	{ 0x59,             0,             0,   0,   0  },
	{ 0x5A,             0,             0,   0,   0  },
	{ 0x5B,             0,             0,   0,   0  },
	{ 0x5C,             0,             0,   0,   0  },
	{ 0x5D,             0,             0,   0,   0  },
	{ 0x5E,          0x88,          0x88,   0,   0  },
	{ 0x5F,          0xcc,          0xcc,   0,   0  },
	{ 0x60,          0x74,          0x74,   0,   0  },
	{ 0x61,          0x74,          0x74,   0,   0  },
	{ 0x62,          0x74,          0x74,   0,   0  },
	{ 0x63,          0x44,          0x44,   0,   0  },
	{ 0x64,          0x77,          0x77,   0,   0  },
	{ 0x65,          0x44,          0x44,   0,   0  },
	{ 0x66,          0x77,          0x77,   0,   0  },
	{ 0x67,          0x55,          0x55,   0,   0  },
	{ 0x68,          0x77,          0x77,   0,   0  },
	{ 0x69,          0x77,          0x77,   0,   0  },
	{ 0x6A,             0,             0,   0,   0  },
	{ 0x6B,          0x7f,          0x7f,   0,   0  },
	{ 0x6C,           0x8,           0x8,   0,   0  },
	{ 0x6D,             0,             0,   0,   0  },
	{ 0x6E,          0x88,          0x88,   0,   0  },
	{ 0x6F,          0x66,          0x66,   0,   0  },
	{ 0x70,          0x66,          0x66,   0,   0  },
	{ 0x71,          0x28,          0x28,   0,   0  },
	{ 0x72,          0x55,          0x55,   0,   0  },
	{ 0x73,           0x4,           0x4,   0,   0  },
	{ 0x74,             0,             0,   0,   0  },
	{ 0x75,             0,             0,   0,   0  },
	{ 0x76,             0,             0,   0,   0  },
	{ 0x77,           0x1,           0x1,   0,   0  },
	{ 0x78,          0xd6,          0xd6,   0,   0  },
	{ 0x79,             0,             0,   0,   0  },
	{ 0x7A,             0,             0,   0,   0  },
	{ 0x7B,             0,             0,   0,   0  },
	{ 0x7D,             0,             0,   0,   0  },
	{ 0x7E,           0x1,           0x1,   0,   0  },
	{ 0x7F,             0,             0,   0,   0  },
	{ 0x80,             0,             0,   0,   0  },
	{ 0x81,             0,             0,   0,   0  },
	{ 0x82,             0,             0,   0,   0  },
	{ 0x83,          0xb4,          0xb4,   0,   0  },
	{ 0x84,           0x1,           0x1,   0,   0  },
	{ 0x85,          0x20,          0x20,   0,   0  },
	{ 0x86,           0x5,           0x5,   0,   0  },
	{ 0x87,          0xff,          0xff,   0,   0  },
	{ 0x88,           0x7,           0x7,   0,   0  },
	{ 0x89,          0x77,          0x77,   0,   0  },
	{ 0x8A,          0x77,          0x77,   0,   0  },
	{ 0x8B,          0x77,          0x77,   0,   0  },
	{ 0x8C,          0x77,          0x77,   0,   0  },
	{ 0x8D,           0x8,           0x8,   0,   0  },
	{ 0x8E,           0xa,           0xa,   0,   0  },
	{ 0x8F,           0x8,           0x8,   0,   0  },
	{ 0x90,          0x18,          0x18,   0,   0  },
	{ 0x91,           0x5,           0x5,   0,   0  },
	{ 0x92,          0x1f,          0x1f,   0,   0  },
	{ 0x93,          0x10,          0x10,   0,   0  },
	{ 0x94,           0x3,           0x3,   0,   0  },
	{ 0x95,             0,             0,   0,   0  },
	{ 0x96,             0,             0,   0,   0  },
	{ 0x97,          0xaa,          0xaa,   0,   0  },
	{ 0x98,             0,             0,   0,   0  },
	{ 0x99,          0x23,          0x23,   0,   0  },
	{ 0x9A,           0x7,           0x7,   0,   0  },
	{ 0x9B,           0xf,           0xf,   0,   0  },
	{ 0x9C,          0x10,          0x10,   0,   0  },
	{ 0x9D,           0x3,           0x3,   0,   0  },
	{ 0x9E,           0x4,           0x4,   0,   0  },
	{ 0x9F,          0x20,          0x20,   0,   0  },
	{ 0xA0,             0,             0,   0,   0  },
	{ 0xA1,             0,             0,   0,   0  },
	{ 0xA2,             0,             0,   0,   0  },
	{ 0xA3,             0,             0,   0,   0  },
	{ 0xA4,           0x1,           0x1,   0,   0  },
	{ 0xA5,          0x77,          0x77,   0,   0  },
	{ 0xA6,          0x77,          0x77,   0,   0  },
	{ 0xA7,          0x77,          0x77,   0,   0  },
	{ 0xA8,          0x77,          0x77,   0,   0  },
	{ 0xA9,          0x8c,          0x8c,   0,   0  },
	{ 0xAA,          0x88,          0x88,   0,   0  },
	{ 0xAB,          0x78,          0x78,   0,   0  },
	{ 0xAC,          0x57,          0x57,   0,   0  },
	{ 0xAD,          0x88,          0x88,   0,   0  },
	{ 0xAE,             0,             0,   0,   0  },
	{ 0xAF,           0x8,           0x8,   0,   0  },
	{ 0xB0,          0x88,          0x88,   0,   0  },
	{ 0xB1,             0,             0,   0,   0  },
	{ 0xB2,          0x1b,          0x1b,   0,   0  },
	{ 0xB3,           0x3,           0x3,   0,   0  },
	{ 0xB4,          0x24,          0x24,   0,   0  },
	{ 0xB5,           0x3,           0x3,   0,   0  },
	{ 0xB6,          0x1b,          0x1b,   0,   0  },
	{ 0xB7,          0x24,          0x24,   0,   0  },
	{ 0xB8,           0x3,           0x3,   0,   0  },
	{ 0xB9,             0,             0,   0,   0  },
	{ 0xBA,          0xaa,          0xaa,   0,   0  },
	{ 0xBB,             0,             0,   0,   0  },
	{ 0xBC,           0x4,           0x4,   0,   0  },
	{ 0xBD,             0,             0,   0,   0  },
	{ 0xBE,           0x8,           0x8,   0,   0  },
	{ 0xBF,          0x11,          0x11,   0,   0  },
	{ 0xC0,             0,             0,   0,   0  },
	{ 0xC1,             0,             0,   0,   0  },
	{ 0xC2,          0x62,          0x62,   0,   0  },
	{ 0xC3,          0x1e,          0x1e,   0,   0  },
	{ 0xC4,          0x33,          0x33,   0,   0  },
	{ 0xC5,          0x37,          0x37,   0,   0  },
	{ 0xC6,             0,             0,   0,   0  },
	{ 0xC7,          0x70,          0x70,   0,   0  },
	{ 0xC8,          0x1e,          0x1e,   0,   0  },
	{ 0xC9,           0x6,           0x6,   0,   0  },
	{ 0xCA,           0x4,           0x4,   0,   0  },
	{ 0xCB,          0x2f,          0x2f,   0,   0  },
	{ 0xCC,           0xf,           0xf,   0,   0  },
	{ 0xCD,             0,             0,   0,   0  },
	{ 0xCE,          0xff,          0xff,   0,   0  },
	{ 0xCF,           0x8,           0x8,   0,   0  },
	{ 0xD0,          0x3f,          0x3f,   0,   0  },
	{ 0xD1,          0x3f,          0x3f,   0,   0  },
	{ 0xD2,          0x3f,          0x3f,   0,   0  },
	{ 0xD3,             0,             0,   0,   0  },
	{ 0xD4,             0,             0,   0,   0  },
	{ 0xD5,             0,             0,   0,   0  },
	{ 0xD6,          0xcc,          0xcc,   0,   0  },
	{ 0xD7,             0,             0,   0,   0  },
	{ 0xD8,           0x8,           0x8,   0,   0  },
	{ 0xD9,           0x8,           0x8,   0,   0  },
	{ 0xDA,           0x8,           0x8,   0,   0  },
	{ 0xDB,          0x11,          0x11,   0,   0  },
	{ 0xDC,             0,             0,   0,   0  },
	{ 0xDD,          0x87,          0x87,   0,   0  },
	{ 0xDE,          0x88,          0x88,   0,   0  },
	{ 0xDF,           0x8,           0x8,   0,   0  },
	{ 0xE0,           0x8,           0x8,   0,   0  },
	{ 0xE1,           0x8,           0x8,   0,   0  },
	{ 0xE2,             0,             0,   0,   0  },
	{ 0xE3,             0,             0,   0,   0  },
	{ 0xE4,             0,             0,   0,   0  },
	{ 0xE5,          0xf5,          0xf5,   0,   0  },
	{ 0xE6,          0x30,          0x30,   0,   0  },
	{ 0xE7,           0x1,           0x1,   0,   0  },
	{ 0xE8,             0,             0,   0,   0  },
	{ 0xE9,          0xff,          0xff,   0,   0  },
	{ 0xEA,             0,             0,   0,   0  },
	{ 0xEB,             0,             0,   0,   0  },
	{ 0xEC,          0x22,          0x22,   0,   0  },
	{ 0xED,             0,             0,   0,   0  },
	{ 0xEE,             0,             0,   0,   0  },
	{ 0xEF,             0,             0,   0,   0  },
	{ 0xF0,           0x3,           0x3,   0,   0  },
	{ 0xF1,           0x1,           0x1,   0,   0  },
	{ 0xF2,             0,             0,   0,   0  },
	{ 0xF3,             0,             0,   0,   0  },
	{ 0xF4,             0,             0,   0,   0  },
	{ 0xF5,             0,             0,   0,   0  },
	{ 0xF6,             0,             0,   0,   0  },
	{ 0xF7,           0x6,           0x6,   0,   0  },
	{ 0xF8,             0,             0,   0,   0  },
	{ 0xF9,             0,             0,   0,   0  },
	{ 0xFA,          0x40,          0x40,   0,   0  },
	{ 0xFB,             0,             0,   0,   0  },
	{ 0xFC,           0x1,           0x1,   0,   0  },
	{ 0xFD,          0x80,          0x80,   0,   0  },
	{ 0xFE,           0x2,           0x2,   0,   0  },
	{ 0xFF,          0x10,          0x10,   0,   0  },
	{ 0x100,           0x2,           0x2,   0,   0  },
	{ 0x101,          0x1e,          0x1e,   0,   0  },
	{ 0x102,          0x1e,          0x1e,   0,   0  },
	{ 0x103,             0,             0,   0,   0  },
	{ 0x104,          0x1f,          0x1f,   0,   0  },
	{ 0x105,             0,           0x8,   0,   1  },
	{ 0x106,          0x2a,          0x2a,   0,   0  },
	{ 0x107,           0xf,           0xf,   0,   0  },
	{ 0x108,             0,             0,   0,   0  },
	{ 0x109,             0,             0,   0,   0  },
	{ 0x10A,             0,             0,   0,   0  },
	{ 0x10B,             0,             0,   0,   0  },
	{ 0x10C,             0,             0,   0,   0  },
	{ 0x10D,             0,             0,   0,   0  },
	{ 0x10E,             0,             0,   0,   0  },
	{ 0x10F,             0,             0,   0,   0  },
	{ 0x110,             0,             0,   0,   0  },
	{ 0x111,             0,             0,   0,   0  },
	{ 0x112,             0,             0,   0,   0  },
	{ 0x113,             0,             0,   0,   0  },
	{ 0x114,             0,             0,   0,   0  },
	{ 0x115,             0,             0,   0,   0  },
	{ 0x116,             0,             0,   0,   0  },
	{ 0x117,             0,             0,   0,   0  },
	{ 0x118,             0,             0,   0,   0  },
	{ 0x119,             0,             0,   0,   0  },
	{ 0x11A,             0,             0,   0,   0  },
	{ 0x11B,             0,             0,   0,   0  },
	{ 0x11C,           0x1,           0x1,   0,   0  },
	{ 0x11D,             0,             0,   0,   0  },
	{ 0x11E,             0,             0,   0,   0  },
	{ 0x11F,             0,             0,   0,   0  },
	{ 0x120,             0,             0,   0,   0  },
	{ 0x121,             0,             0,   0,   0  },
	{ 0x122,          0x80,          0x80,   0,   0  },
	{ 0x123,             0,             0,   0,   0  },
	{ 0x124,          0xf8,          0xf8,   0,   0  },
	{ 0x125,             0,             0,   0,   0  },
	{ 0x126,             0,             0,   0,   0  },
	{ 0x127,             0,             0,   0,   0  },
	{ 0x128,             0,             0,   0,   0  },
	{ 0x129,             0,             0,   0,   0  },
	{ 0x12A,             0,             0,   0,   0  },
	{ 0x12B,             0,             0,   0,   0  },
	{ 0x12C,             0,             0,   0,   0  },
	{ 0x12D,             0,             0,   0,   0  },
	{ 0x12E,             0,             0,   0,   0  },
	{ 0x12F,             0,             0,   0,   0  },
	{ 0x130,             0,             0,   0,   0  },
	{ 0xFFFF,             0,             0,   0,   0  }
};
#else
lcnphy_radio_regs_t WLBANDINITDATA(lcnphy_radio_regs_2064)[] = {
	{ 0x105,             0,           0x8,   0,   1  },
	{ 0xFFFF,             0,             0,   0,   0  }
};
#endif /* defined(BCMDBG) || defined(WLTEST) */

#define LCNPHY_NUM_DIG_FILT_COEFFS 17
#define LCNPHY_NUM_TX_DIG_FILTERS_CCK 14
/* filter id, followed by coefficients */
uint16 LCNPHY_txdigfiltcoeffs_cck[LCNPHY_NUM_TX_DIG_FILTERS_CCK][1+LCNPHY_NUM_DIG_FILT_COEFFS] = {
	{ 0, 1, 415, 1874, 64, 128, 64, 792, 1656, 64, 128, 64, 778, 1582, 64, 128, 64, 8},
	{ 1, 1, 402, 1847, 259, 59, 259, 671, 1794, 68, 54, 68, 608, 1863, 93, 167, 93, 8},
	{ 2, 1, 415, 1874, 64, 128, 64, 792, 1656, 192, 384, 192, 778, 1582, 64, 128, 64, 8},
	{ 3, 1, 302, 1841, 129, 258, 129, 658, 1720, 205, 410, 205, 754, 1760, 170, 340, 170, 8},
	{ 20, 1, 360, -164, 242, -314, 242, 752, -328, -205, 203, -205, 767, -288, -253, -183,
	-253, 8},
	{ 21, 1, 360, -164, 149, -174, 149, 752, -328, -205, 164, -205, 767, -288, -256, -273,
	-256, 8},
	{ 22, 1, 360, -164, 98, -100, 98, 752, -328, -205, 124, -205, 767, -288, -256, -352,
	-256, 8},
	{ 23, 1, 350, -164, -116, 82, -116, 752, -328, -205, 40, -205, 767, -288, -129, -235,
	-129, 8},
	{ 24, 1, 325, -164, -32, -40, -32, 756, -328, -256, -471, -256, 766, -288, -262, 170,
	-262, 8},
	{ 25, 1, 299, -164, -51, -64, -51, 736, -328, -256, -471, -256, 765, -288, -262, 170,
	-262, 8},
	{ 26, 1, 277, -105, -39, -117, -88, 637, -210, -64, -192, -144, 614, -184, -128, -384,
	-288, 8},
	{ 27, 1, 245, -105, -49, -147, -110, 626, -210, -162, -485, -363, 613, -184, -62, -186,
	-139, 8},
	{ 30, 1, 302, -207, -61, -122, -61, 658, -328, -205, -410, -205, 754, -288, -170, -340,
	-170, 8},
	{ 40, 1, 360, -164, 242, -314, 242, 752, -328, 205, -203, 205, 767, -288, 511, 370, 511,
	8},
	};

#define LCNPHY_NUM_TX_DIG_FILTERS_CCK_160 10
uint16 LCNPHY_txdigfiltcoeffs_cck_160
	[LCNPHY_NUM_TX_DIG_FILTERS_CCK_160][1+LCNPHY_NUM_DIG_FILT_COEFFS] = {
	{ 20, 1, 447, -207, 242, -439, 242, 903, -415, -205, 354, -205, 896, -392, -294, 233,
	-294, 8},
	{ 21, 1, 447, -207, 280, -498, 280, 903, -415, -234, 380, -234, 896, -392, -113, 147,
	-113, 9},
	{ 22, 1, 447, -207, 98, -171, 98, 903, -415, -225, 363, -225, 889, -386, -268, 93, -268, 8},
	{ 23, 1, 444, -207, 116, -190, 116, 903, -415, -174, 258, -174, 896, -392, -129, -15,
	-129, 8},
	{ 24, 1, 436, -207, -33, 29, -33, 904, -415, -121, -112, -121, 895, -392, -307, 500,
	-307, 8},
	{ 25, 1, 429, -207, -72, 62, -72, 898, -415, -86, -79, -86, 895, -392, -313, 510, -313, 8},
	{ 26, 1, 403, -172, -39, 12, -39, 795, -313, -180, 56, -180, 819, -328, -128, 40, -128, 8},
	{ 27, 1, 393, -172, -49, 19, -49, 791, -313, -256, 80, -256, 814, -323, -117, 37, -117, 8},
	{ 30, 1, 443, -231, -61, 29, -61, 875, -415, -154, 72, -154, 892, -392, -170, 79, -170, 8},
	{ 44, 1, 447, -207, 224, -399, 224, 903, -415, -117, 196, -117, 896, -392, -334, 192,
	-334, 8},
	};

#define LCNPHY_NUM_TX_DIG_FILTERS_OFDM 5
uint16 LCNPHY_txdigfiltcoeffs_ofdm[LCNPHY_NUM_TX_DIG_FILTERS_OFDM][1+LCNPHY_NUM_DIG_FILT_COEFFS] = {
	{0, 0, 162, 0, -256, -256, 0, 0, 0, -256, 0, 0, 632, -352, -128, -256, -128, 8},
	{1, 0, 374, -135, -16, -32, -16, 799, -396, -50, -32, -50, 750, -469, -212, 50, -212, 8},
	{2, 0, 375, -234, -37, -76, -37, 799, -396, -32, -20, -32, 748, -270, -128, 30, -128, 8},
	{3, 0, 375, -234, -37, -76, -37, 799, -396, -32, -20, -32, 748, -270, -148, 35, -148, 8},
	{4, 0, 307, -82, -53, -106, -53, 779, -379, -53, 10, -53, 765, -469, -212, 202, -212, 8},
};

#define LCNPHY_NUM_TX_DIG_FILTERS_OFDM_160 3
uint16 LCNPHY_txdigfiltcoeffs_ofdm_160
	[LCNPHY_NUM_TX_DIG_FILTERS_OFDM_160][1+LCNPHY_NUM_DIG_FILT_COEFFS] = {
	{0, 0, 453, -227, -32, -64, -32, 0, 0, -256, 0, 0, 875, -382, -25, -52, -25, 8},
	{2, 0, 471, -245, -18, -36, -18, 935, -452, -20, 23, -20, 882, -379, -338, 504, -338, 7},
	{4, 0, 408, -160, -53, -106, -53, 924, -443, 37, -63, 37, 945, -490, -284, 421, -284, 7},
	};

#define wlc_lcnphy_common_read_table(pi, tbl_id, tbl_ptr, tbl_len, tbl_width, tbl_offset) \
	phy_utils_read_common_phytable(pi, tbl_id, tbl_ptr, tbl_len, tbl_width, tbl_offset, \
	wlc_lcnphy_read_table)

#define wlc_lcnphy_common_write_table(pi, tbl_id, tbl_ptr, tbl_len, tbl_width, tbl_offset) \
	phy_utils_write_common_phytable(pi, tbl_id, tbl_ptr, tbl_len, tbl_width, tbl_offset, \
	wlc_lcnphy_write_table)

#define wlc_lcnphy_set_start_tx_pwr_idx(pi, idx) \
	PHY_REG_MOD(pi, LCNPHY, TxPwrCtrlCmd, pwrIndex_init, \
	idx*2/(1+pi->u.pi_lcnphy->virtual_p25_tx_gain_step))

#define wlc_lcnphy_set_tx_pwr_npt(pi, npt) \
	PHY_REG_MOD(pi, LCNPHY, TxPwrCtrlNnum, Npt_intg_log2, npt)

#define wlc_lcnphy_get_tx_pwr_npt(pi) \
	((phy_utils_read_phyreg(pi, LCNPHY_TxPwrCtrlNnum) & \
		LCNPHY_TxPwrCtrlNnum_Npt_intg_log2_MASK) >> \
		LCNPHY_TxPwrCtrlNnum_Npt_intg_log2_SHIFT)

/* the bitsize of the register is 9 bits for lcnphy */
#define wlc_lcnphy_get_current_tx_pwr_idx_if_pwrctrl_on(pi) \
	(phy_utils_read_phyreg(pi, LCNPHY_TxPwrCtrlStatusExt) & 0x1ff)

#define wlc_lcnphy_get_target_tx_pwr(pi) \
	((phy_utils_read_phyreg(pi, LCNPHY_TxPwrCtrlTargetPwr) & \
		LCNPHY_TxPwrCtrlTargetPwr_targetPwr0_MASK) >> \
		LCNPHY_TxPwrCtrlTargetPwr_targetPwr0_SHIFT)

#define wlc_lcnphy_set_target_tx_pwr(pi, target) \
	PHY_REG_MOD(pi, LCNPHY, TxPwrCtrlTargetPwr, targetPwr0, \
		(uint16)MAX(pi->u.pi_lcnphy->tssi_minpwr_limit, \
		(MIN(pi->u.pi_lcnphy->tssi_maxpwr_limit, \
		(uint16)(target)))))

#define wlc_radio_2064_rc_cal_done(pi) \
	((phy_utils_read_radioreg(pi, RADIO_2064_REG10A) & 0x01) != 0)
#define wlc_radio_2064_rcal_done(pi) \
	((phy_utils_read_radioreg(pi, RADIO_2064_REG05C) & 0x20) != 0)
#define tempsense_done(pi) \
	((phy_utils_read_phyreg(pi, LCNPHY_TxPwrCtrlStatusTemp) & 0x8000) == 0x8000)

#define LCNPHY_IQLOCC_READ(val) ((uint8)(-(int8)(((val) & 0xf0) >> 4) + (int8)((val) & 0x0f)))
#define FIXED_TXPWR 78
#define LCNPHY_TEMPSENSE(val) ((int16)((val > 255)?(val - 512):val))

/* %%%%%% LCNPHY function declaration */
static uint32 wlc_lcnphy_qdiv_roundup(uint32 dividend, uint32 divisor, uint8 precision);
static void wlc_lcnphy_set_rx_gain_by_distribution(phy_info_t *pi, uint16 trsw,
	uint16 ext_lna,	uint16 biq2, uint16 biq1, uint16 tia,
	uint16 lna2, uint16 lna1, uint16 digi);
static void wlc_lcnphy_set_pa_gain(phy_info_t *pi, uint16 gain);
static void wlc_lcnphy_epa_pd(phy_info_t *pi, bool disable);
static void wlc_lcnphy_set_trsw_override(phy_info_t *pi, bool tx, bool rx);
static void wlc_lcnphy_set_bbmult(phy_info_t *pi, uint8 m0);
static void wlc_lcnphy_get_tx_gain(phy_info_t *pi,  lcnphy_txgains_t *gains);
static void wlc_lcnphy_set_tx_gain_override(phy_info_t *pi, bool bEnable);
static void wlc_lcnphy_toggle_afe_pwdn(phy_info_t *pi);
static void wlc_lcnphy_rx_gain_override_enable(phy_info_t *pi, bool enable);
static void wlc_lcnphy_set_tx_gain(phy_info_t *pi,  lcnphy_txgains_t *target_gains);
static void wlc_lcnphy_GetpapdMaxMinIdxupdt(phy_info_t *pi,
	int8 *maxUpdtIdx, int8 *minUpdtIdx);
static void wlc_lcnphy_rx_pu(phy_info_t *pi, bool bEnable);
static bool wlc_lcnphy_calc_rx_iq_comp(phy_info_t *pi,  uint16 num_samps);
static uint16 wlc_lcnphy_get_pa_gain(phy_info_t *pi);
static void wlc_lcnphy_afe_clk_init(phy_info_t *pi, uint8 mode);
extern void wlc_lcnphy_tx_pwr_ctrl_init(wlc_phy_t *ppi);
extern void wlc_lcnphy_dummytx(wlc_phy_t *ppi, uint16 nframes, uint16 wait_delay);
static void wlc_lcnphy_radio_2064_channel_tune(phy_info_t *pi, uint8 channel);
static void wlc_lcnphy_radio_2064_channel_tune_4313(phy_info_t *pi, uint8 channel);

static void wlc_lcnphy_load_tx_gain_table(phy_info_t *pi, const lcnphy_tx_gain_tbl_entry *g);
static void wlc_lcnphy_force_pwr_index(phy_info_t *pi, int indx);
static void wlc_lcnphy_decode_aa2g(wlc_phy_t *ppi, uint8 val);
static int8 wlc_lcnphy_tempcompensated_txpwrctrl(phy_info_t *pi, bool calc_cap_index);
static void wlc_lcnphy_load_txgainwithcappedindex(phy_info_t *pi, bool cap);
#if defined(WLTEST)
static int8 wlc_lcnphy_calc_rx_gain(phy_info_t *pi);
#endif // endif

static bool wlc_lcnphy_rx_iq_est(phy_info_t *pi, uint16 num_samps, uint8 wait_time,
	uint8 wait_for_crs, phy_iq_est_t *iq_est, uint16 timeout_ms);

/* added for iqlo soft cal */
static void wlc_lcnphy_samp_cap(phy_info_t *pi, int clip_detect_algo, uint16 thresh,
	int16* ptr, int mode);
static int wlc_lcnphy_calc_floor(int16 coeff, int type);
static void wlc_lcnphy_tx_iqlo_loopback(phy_info_t *pi, uint16 *values_to_save);
static void wlc_lcnphy_tx_iqlo_loopback_cleanup(phy_info_t *pi, uint16 *values_to_save);
static void wlc_lcnphy_set_cc(phy_info_t *pi, int cal_type, int16 coeff_x, int16 coeff_y);
static lcnphy_unsign16_struct wlc_lcnphy_get_cc(phy_info_t *pi, int cal_type);
static void wlc_lcnphy_tx_iqlo_soft_cal(phy_info_t *pi, int cal_type,
	int num_levels, int step_size_lg2);
static void wlc_lcnphy_tx_iqlo_soft_cal_full(phy_info_t *pi);

static void wlc_lcnphy_set_chanspec_tweaks(phy_info_t *pi, chanspec_t chanspec);
static void wlc_lcnphy_agc_temp_init(phy_info_t *pi);
static void wlc_lcnphy_temp_adj(phy_info_t *pi);
static void wlc_lcnphy_clear_papd_comptable(phy_info_t *pi);
static void wlc_lcnphy_baseband_init(phy_info_t *pi);
static void wlc_lcnphy_radio_init(phy_info_t *pi);
static void wlc_lcnphy_rc_cal(phy_info_t *pi);
static void wlc_lcnphy_rcal(phy_info_t *pi);
static void wlc_lcnphy_reset_radio_loft(phy_info_t *pi);
static void wlc_lcnphy_txrx_spur_avoidance_mode(phy_info_t *pi, bool enable);
static int wlc_lcnphy_load_tx_iir_filter(phy_info_t *pi, bool is_ofdm, int16 filt_type);
static void wlc_lcnphy_restore_calibration_results(phy_info_t *pi);
static void wlc_lcnphy_set_rx_iq_comp(phy_info_t *pi, uint16 a, uint16 b);
static void wlc_lcnphy_set_genv(phy_info_t *pi, uint16 genv);
static uint16 wlc_lcnphy_get_genv(phy_info_t *pi);
static uint16 wlc_lcnphy_calc_papd_rf_pwr_offset(phy_info_t *pi, uint16 indx, int8
papd_rf_pwr_scale);
static void wlc_lcnphy_load_rfpower(phy_info_t *pi);
static uint8 wlc_lcnphy_get_papd_rfpwr_scale(phy_info_t *pi);
static uint16 wlc_lcnphy_papd_index_search(phy_info_t *pi, uint64 final_idx_thresh,
lcnphy_txcalgains_t *txgains);
static void wlc_lcnphy_papd_calc_capindex(phy_info_t *pi, lcnphy_txcalgains_t *txgains);
static void wlc_lcnphy_papd_idx_sel(phy_info_t *pi, uint32 amam_cmp, lcnphy_txcalgains_t *txgains);
static void wlc_lcnphy_papd_cal_txpwr(phy_info_t *pi);
static void wlc_lcnphy_rfpower_lut_2papdlut(phy_info_t *pi);

static void wlc_lcnphy_papd_cal(phy_info_t *pi, phy_papd_cal_type_t cal_type,
	lcnphy_txcalgains_t *txgains, bool frcRxGnCtrl, bool txGnCtrl, bool samplecapture,
	bool papd_dbg_mode, uint8 num_symbols, uint8 init_papd_lut, uint16 bbmult_step);

static void wlc_lcnphy_reset_iir_filter(phy_info_t *pi);
static void wlc_lcnphy_noise_init(phy_info_t *pi);
static void wlc_lcnphy_noise_attach(phy_info_t *pi);

static void wlc_lcnphy_noise_save_phy_regs(phy_info_t *pi);
static void wlc_lcnphy_pll_reset_war(phy_info_t *pi);

static void wlc_lcnphy_set_estPwrLUT(phy_info_t *pi, int32 lut_num);

static void wlc_lcnphy_set_tssi_pwr_limit(phy_info_t *pi, uint8 mode);
static void wlc_lcnphy_set_tx_idx_raw(phy_info_t *pi, uint16 indx);

#if defined(BCMDBG)
void wlc_lcnphy_iovar_cw_tx_pwr_ctrl(phy_info_t *pi, int32 targetpwr, int32 *ret, bool set);
#endif // endif
static int32 wlc_lcnphy_cw_tx_pwr_ctrl(phy_info_t *pi, int32 targetpwr);
static int32 wlc_lcnphy_tone_power(phy_info_t *pi, bool tssi_setup);
static void wlc_lcnphy_tssi_loopback(phy_info_t *pi, uint16 *values_to_save);
static void wlc_lcnphy_tssi_loopback_cleanup(phy_info_t *pi, uint16 *values_to_save);

static void wlc_lcnphy_tssi_ucode_setup(phy_info_t *pi, uint8 update_lut);
static bool wlc_lcnphy_cal_reqd(phy_info_t *pi);
static void wlc_lcnphy_btc_adjust(phy_info_t *pi, bool btactive);
static void wlc_phy_watchdog_lcnphy(phy_info_t *pi);
static uint16 wlc_lcnphy_get_tx_pwr_ctrl(phy_info_t *pi);
static uint16 wlc_lcnphy_tssi_cal_sweep(phy_info_t *pi, int8 *, uint8 *);
static void wlc_lcnphy_set_tr_attn(phy_info_t *pi, bool override_en, uint8 override_val);
static bool wlc_lcnphy_frmTypBasedIqLoCoeff(phy_info_t *pi);
static void wlc_lcnphy_adjust_frame_based_lo_coeffs(phy_info_t *pi);
static int16 wlc_lcnphy_tx_dig_iir_dc_val(phy_info_t *pi, uint8 is_ofdm, int16 filt_num);
static bool wlc_lcnphy_tx_dig_iir_dc_err_comp(phy_info_t *pi);
static int8 wlc_lcnphy_noise_log(uint32 x);
static void wlc_lcnphy_noise_set_input_pwr_offset(phy_info_t *pi);
static void wlc_lcnphy_agc_reset(phy_info_t *pi);

static void wlc_lcnphy_tx_gain_tbl_init(phy_info_t *pi);
static bool BCMATTACHFN(wlc_lcnphy_tx_gain_tbl_select)(phy_info_t *pi);
static void BCMATTACHFN(wlc_lcnphy_tx_gain_tbl_copy)(lcnphy_tx_gain_tbl_entry *gain_table_dst,
	const lcnphy_tx_gain_tbl_entry * gain_table, uint32 num_elem);
static void wlc_lcnphy_tx_gain_tbl_free(phy_info_t *pi);

static void wlc_lcnphy_rx_gain_tbl_init(phy_info_t *pi);
static bool BCMATTACHFN(wlc_lcnphy_rx_gain_tbl_select)(phy_info_t *pi);
static bool BCMATTACHFN(wlc_lcnphy_rx_gain_tbl_copy)(phy_info_t *pi, bool tbl2g);
static void wlc_lcnphy_rx_gain_tbl_free(phy_info_t *pi);
static void wlc_lcnphy_rx_gain_tbl_free_entry(phy_info_t *pi, bool tbl2g);
static void wlc_lcnphy_calc_papd_rf_pwr_offset_arr(phy_info_t *pi,
	int8 papd_rf_pwr_scale, uint32 *val_arr);

static void wlc_phy_txpwr_sromlcn_read_2g_ppr_parameters(phy_info_t *pi);
static void wlc_phy_txpwr_apply_sromlcn(phy_info_t *pi, uint8 band, ppr_t *tx_srom_max_pwr);

#ifdef LP_P2P_SOFTAP
static void wlc_lcnphy_lpc_write_maclut(phy_info_t *pi);
#endif /* LP_P2P_SOFTAP */

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
/*  function implementation   					*/
/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

void
wlc_lcnphy_write_table(phy_info_t *pi, const phytbl_info_t *pti)
{
	phy_utils_write_phytable(pi, pti, LCNPHY_TableAddress,
		LCNPHY_TabledataHi, LCNPHY_TabledataLo);
}

void
wlc_lcnphy_read_table(phy_info_t *pi, phytbl_info_t *pti)
{
	phy_utils_read_phytable(pi, pti, LCNPHY_TableAddress,
	   LCNPHY_TabledataHi, LCNPHY_TabledataLo);
}

static uint16
wlc_lcnphy_get_tx_pwr_ctrl(phy_info_t *pi)
{
	return phy_utils_read_phyreg(pi, LCNPHY_TxPwrCtrlCmd) &
		(LCNPHY_TxPwrCtrlCmd_txPwrCtrl_en_MASK |
		LCNPHY_TxPwrCtrlCmd_hwtxPwrCtrl_en_MASK);
}

static uint32
wlc_lcnphy_qdiv_roundup(uint32 dividend, uint32 divisor, uint8 precision)
{
	uint32 quotient, remainder, roundup, rbit;

	ASSERT(divisor);

	quotient = dividend / divisor;
	remainder = dividend % divisor;
	rbit = divisor & 1;
	roundup = (divisor >> 1) + rbit;

	while (precision--) {
		quotient <<= 1;
		if (remainder >= roundup) {
			quotient++;
			remainder = ((remainder - roundup) << 1) + rbit;
		} else {
			remainder <<= 1;
		}
	}

	/* Final rounding */
	if (remainder >= roundup)
		quotient++;

	return quotient;
}

static int
wlc_lcnphy_calc_floor(int16 coeff_x, int type)
{
	int k;
	k = 0;
	if (type == 0) {
		if (coeff_x < 0) {
			k = (coeff_x - 1)/2;
		} else {
			k = coeff_x/2;
		}
	}
	if (type == 1) {
		if ((coeff_x + 1) < 0)
			k = (coeff_x)/2;
		else
			k = (coeff_x + 1)/ 2;
	}
	return k;
}

int8
wlc_lcnphy_get_current_tx_pwr_idx(phy_info_t *pi)
{
	int8 indx;
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	/* for txpwrctrl_off and tempsense_based pwrctrl, return current_index */
	if (txpwrctrl_off(pi))
		indx = pi_lcn->lcnphy_current_index;
	else if (wlc_lcnphy_tssi_based_pwr_ctrl_enabled(pi))
		indx = (int8)(wlc_lcnphy_get_current_tx_pwr_idx_if_pwrctrl_on(pi)
						*(1+pi_lcn->virtual_p25_tx_gain_step)/2);
	else
		indx = pi_lcn->lcnphy_current_index;
	return indx;
}

static uint32
wlc_lcnphy_measure_digital_power(phy_info_t *pi, uint16 nsamples)
{
	phy_iq_est_t iq_est = {0, 0, 0};

	if (!wlc_lcnphy_rx_iq_est(pi, nsamples, 32, 0, &iq_est, RXIQEST_TIMEOUT))
		return 0;
	return (iq_est.i_pwr + iq_est.q_pwr) / nsamples;
}

/* %%%%%% LCNPHY functions */

void
wlc_lcnphy_crsuprs(phy_info_t *pi, int channel)
{
	uint16 afectrlovr, afectrlovrval;
	afectrlovr = phy_utils_read_phyreg(pi, LCNPHY_AfeCtrlOvr);
	afectrlovrval = phy_utils_read_phyreg(pi, LCNPHY_AfeCtrlOvrVal);
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	if (channel != 0) {
		PHY_REG_LIST_START
			PHY_REG_MOD_ENTRY(LCNPHY, AfeCtrlOvr, pwdn_dac_ovr, 1)
			PHY_REG_MOD_ENTRY(LCNPHY, AfeCtrlOvrVal, pwdn_dac_ovr_val, 0)
			PHY_REG_MOD_ENTRY(LCNPHY, AfeCtrlOvr, dac_clk_disable_ovr, 1)
			PHY_REG_MOD_ENTRY(LCNPHY, AfeCtrlOvrVal, dac_clk_disable_ovr_val, 0)
			PHY_REG_WRITE_ENTRY(LCNPHY, ClkEnCtrl, 0xffff)
		PHY_REG_LIST_EXECUTE(pi);

		wlc_lcnphy_tx_pu(pi, 1);

		PHY_REG_LIST_START
			/* Turn ON bphyframe */
			PHY_REG_MOD_ENTRY(LCNPHY, BphyControl3, bphyFrmStartCntValue, 0)
			/* Turn on Tx Front End clks */
			PHY_REG_OR_ENTRY(LCNPHY, sslpnCalibClkEnCtrl, 0x0080)
			/* start the rfcs signal */
			PHY_REG_OR_ENTRY(LCNPHY, bphyTest, 0x228)
		PHY_REG_LIST_EXECUTE(pi);
	} else {
		PHY_REG_LIST_START
			PHY_REG_AND_ENTRY(LCNPHY, bphyTest, ~(0x228))
			/* disable clk to txFrontEnd */
			PHY_REG_AND_ENTRY(LCNPHY, sslpnCalibClkEnCtrl, 0xFF7F)
		PHY_REG_LIST_EXECUTE(pi);

		PHY_REG_WRITE(pi, LCNPHY, AfeCtrlOvr, afectrlovr);
		PHY_REG_WRITE(pi, LCNPHY, AfeCtrlOvrVal, afectrlovrval);
	}
}

static void
wlc_lcnphy_toggle_afe_pwdn(phy_info_t *pi)
{
	uint16 save_AfeCtrlOvrVal, save_AfeCtrlOvr;

	save_AfeCtrlOvrVal = phy_utils_read_phyreg(pi, LCNPHY_AfeCtrlOvrVal);
	save_AfeCtrlOvr = phy_utils_read_phyreg(pi, LCNPHY_AfeCtrlOvr);

	PHY_REG_WRITE(pi, LCNPHY, AfeCtrlOvrVal, save_AfeCtrlOvrVal | 0x1);
	PHY_REG_WRITE(pi, LCNPHY, AfeCtrlOvr, save_AfeCtrlOvr | 0x1);

	PHY_REG_WRITE(pi, LCNPHY, AfeCtrlOvrVal, save_AfeCtrlOvrVal & 0xfffe);
	PHY_REG_WRITE(pi, LCNPHY, AfeCtrlOvr, save_AfeCtrlOvr & 0xfffe);

	PHY_REG_WRITE(pi, LCNPHY, AfeCtrlOvrVal, save_AfeCtrlOvrVal);
	PHY_REG_WRITE(pi, LCNPHY, AfeCtrlOvr, save_AfeCtrlOvr);
}

static void
wlc_lcnphy_txrx_spur_avoidance_mode(phy_info_t *pi, bool enable)
{
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	if (enable) {
		PHY_REG_LIST_START
			PHY_REG_WRITE_ENTRY(LCNPHY, spur_canceller1, ((1 << 13) + 23))
			PHY_REG_WRITE_ENTRY(LCNPHY, spur_canceller2, ((1 << 13) + 1989))
		PHY_REG_LIST_EXECUTE(pi);

		if (LCNREV_GE(pi->pubpi.phy_rev, 3)) {
			if (pi->u.pi_lcnphy->dacrate == 160)
				PHY_REG_WRITE(pi, LCNPHY, lcnphy_clk_muxsel3, (79 + (2 << 7)));
			else
				PHY_REG_WRITE(pi, LCNPHY, lcnphy_clk_muxsel3, (39 + (1 << 7)));
		}
		PHY_REG_LIST_START
			PHY_REG_WRITE_ENTRY(LCNPHY, lcnphy_clk_muxsel1, 0xf)
			PHY_REG_WRITE_ENTRY(LCNPHY, lcnphy_clk_muxsel1, 0x0)
			PHY_REG_WRITE_ENTRY(LCNPHY, lcnphy_clk_muxsel1, 0xf)
			/* do a soft reset */
			PHY_REG_WRITE_ENTRY(LCNPHY, resetCtrl, 0x084)
			PHY_REG_WRITE_ENTRY(LCNPHY, resetCtrl, 0x080)
			PHY_REG_WRITE_ENTRY(LCNPHY, sslpnCtrl0, 0x2222)
			PHY_REG_WRITE_ENTRY(LCNPHY, sslpnCtrl0, 0x2220)
		PHY_REG_LIST_EXECUTE(pi);
	} else {
		PHY_REG_LIST_START
			PHY_REG_WRITE_ENTRY(LCNPHY, lcnphy_clk_muxsel1, 0x0)
			PHY_REG_WRITE_ENTRY(LCNPHY, lcnphy_clk_muxsel1, 0xf)
			PHY_REG_WRITE_ENTRY(LCNPHY, lcnphy_clk_muxsel1, 0x0)
			PHY_REG_WRITE_ENTRY(LCNPHY, spur_canceller1, ((0 << 13) + 23))
			PHY_REG_WRITE_ENTRY(LCNPHY, spur_canceller2, ((0 << 13) + 1989))
		PHY_REG_LIST_EXECUTE(pi);
	}
	wlapi_switch_macfreq(pi->sh->physhim, (enable ? WL_SPURAVOID_ON1 : WL_SPURAVOID_OFF));
}

/* Enable and restore calibrations or disable calibration */
static void
wlc_phy_papd_txiqlo_rxiq_enable(phy_info_t *pi, bool enable, chanspec_t chanspec)
{
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;

	if (enable) {
		wlc_lcnphy_restore_calibration_results(pi);
		PHY_REG_MOD(pi, LCNPHY, rxfe, bypass_iqcomp, 0);
		if (PAPD_4336_MODE(pi_lcn) && !EPA(pi_lcn))
			PHY_REG_MOD(pi, LCNPHY, papd_control, papdCompEn, 1);
	} else {
		PHY_REG_MOD(pi, LCNPHY, rxfe, bypass_iqcomp, 1);
		if (PAPD_4336_MODE(pi_lcn) && !EPA(pi_lcn))
			PHY_REG_MOD(pi, LCNPHY, papd_control, papdCompEn, 0);
		wlc_lcnphy_set_tx_locc(pi, 0);
		wlc_lcnphy_set_tx_iqcc(pi, 0, 0);
	}
}

void
wlc_phy_chanspec_set_lcnphy(phy_info_t *pi, chanspec_t chanspec)
{
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	uint8 channel = CHSPEC_CHANNEL(chanspec); /* see wlioctl.h */
	/* uint16 SAVE_txpwrctrl = wlc_lcnphy_get_tx_pwr_ctrl(pi); */
	uint32 ptcentreTs20, ptcentreFactor;
	uint32 freq = phy_utils_channel2freq(channel);

#if defined(PHYCAL_CACHING)
	ch_calcache_t *ctx = wlc_phy_get_chanctx(pi, chanspec);
#endif // endif

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	wlc_phy_chanspec_radio_set((wlc_phy_t *)pi, chanspec);

	wlc_lcnphy_set_chanspec_tweaks(pi, pi->radio_chanspec);

	PHY_REG_LIST_START
		/* lcnphy_agc_reset */
		PHY_REG_OR_ENTRY(LCNPHY, resetCtrl, 0x44)
		PHY_REG_WRITE_ENTRY(LCNPHY, resetCtrl, 0x80)
	PHY_REG_LIST_EXECUTE(pi);

	/* Tune radio for the channel */
	if (!NORADIO_ENAB(pi->pubpi)) {
		/* mapped to lcnphy_set_rf_pll_2064 proc */
		/* doubler enabled for 4313A0 */
		if (CHIPID(pi->sh->chip) == BCM4313_CHIP_ID)
			wlc_lcnphy_radio_2064_channel_tune_4313(pi, channel);
		else
			wlc_lcnphy_radio_2064_channel_tune(pi, channel);
		OSL_DELAY(10);
	}

#ifdef BAND5G
	if (CHSPEC_IS5G(pi->radio_chanspec)) {
		PHY_REG_MOD(pi, LCNPHY, lpphyCtrl, muxGmode, 0);
	} else
#endif // endif
	{
		PHY_REG_MOD(pi, LCNPHY, lpphyCtrl, muxGmode, 1);
	}

	/* toggle the afe whenever we move to a new channel */
	wlc_lcnphy_toggle_afe_pwdn(pi);

	/* Sampling frequency offset */
	ptcentreTs20 = wlc_lcnphy_qdiv_roundup(freq * 2, 5, 0);
	ptcentreFactor = wlc_lcnphy_qdiv_roundup(2621440, freq, 0);
	PHY_REG_WRITE(pi, LCNPHY, ptcentreTs20, (uint16)ptcentreTs20);
	PHY_REG_WRITE(pi, LCNPHY, ptcentreFactor, (uint16)ptcentreFactor);

	/* wlapi_bmac_write_shm(pi->sh->physhim, M_MAXRXFRM_LEN, 5000); */

	if (CHIPID(pi->sh->chip) == BCM4313_CHIP_ID) {

		/* 4313: channel based cck filter coeffs */
		if (CHSPEC_CHANNEL(pi->radio_chanspec) == 14) {
		    PHY_REG_MOD(pi, LCNPHY, lpphyCtrl, txfiltSelect, 2);
			wlc_lcnphy_load_tx_iir_filter(pi, FALSE, 3);
		} else {
			PHY_REG_MOD(pi, LCNPHY, lpphyCtrl, txfiltSelect, 1);
			wlc_lcnphy_load_tx_iir_filter(pi, FALSE, 25);
		}
		/* 4313: ofdm filter coeffs */
		if (!EPA(pi_lcn))
			wlc_lcnphy_load_tx_iir_filter(pi, TRUE, 3);
		else
			wlc_lcnphy_load_tx_iir_filter(pi, TRUE, 0);

		PHY_REG_MOD(pi, LCNPHY, lpfbwlutreg1, lpf_cck_tx_bw, 1);

	} else {

		int16 cck_dig_filt_type;
		int16 ofdm_dig_filt_type;

		PHY_REG_MOD(pi, LCNPHY, lpphyCtrl, txfiltSelect, 2);

		if (CHSPEC_CHANNEL(pi->radio_chanspec) == 14) {
			cck_dig_filt_type = 30;
		} else if (pi_lcn->lcnphy_cck_dig_filt_type != -1) {
			cck_dig_filt_type = pi_lcn->lcnphy_cck_dig_filt_type;
		} else {
			cck_dig_filt_type = 21;
		}

		pi_lcn->lcnphy_cck_dig_filt_type_curr = cck_dig_filt_type;

		if (wlc_lcnphy_load_tx_iir_filter(pi, FALSE, cck_dig_filt_type) != 0)
			wlc_lcnphy_load_tx_iir_filter(pi, FALSE, 20);

		PHY_REG_MOD(pi, LCNPHY, lpfbwlutreg1, lpf_cck_tx_bw,
			(CHSPEC_CHANNEL(pi->radio_chanspec) == 14) ? 3 : 0);

		/* 4336: ofdm filter coeffs */
		if (CHSPEC_IS2G(pi->radio_chanspec))
			ofdm_dig_filt_type = pi_lcn->lcnphy_ofdm_dig_filt_type_2g;
		else
			ofdm_dig_filt_type = pi_lcn->lcnphy_ofdm_dig_filt_type_5g;

		if (ofdm_dig_filt_type == -1)
			ofdm_dig_filt_type = 2;

		pi_lcn->lcnphy_ofdm_dig_filt_type_curr = ofdm_dig_filt_type;

		wlc_lcnphy_load_tx_iir_filter(pi, TRUE, ofdm_dig_filt_type);
	}

	/* Power cycle ADC, DAC to fix occasional tx iqlo and rx iq cal failures */
	PHY_REG_LIST_START
		/* ADC */
		PHY_REG_MOD_ENTRY(LCNPHY, AfeCtrlOvr, pwdn_adc_ovr, 1)
		PHY_REG_MOD_ENTRY(LCNPHY, AfeCtrlOvrVal, pwdn_adc_ovr_val, 1)
		PHY_REG_MOD_ENTRY(LCNPHY, AfeCtrlOvrVal, pwdn_adc_ovr_val, 0)
		PHY_REG_MOD_ENTRY(LCNPHY, AfeCtrlOvr, pwdn_adc_ovr, 0)
		/* DAC */
		PHY_REG_MOD_ENTRY(LCNPHY, AfeCtrlOvr, pwdn_dac_ovr, 1)
		PHY_REG_MOD_ENTRY(LCNPHY, AfeCtrlOvrVal, pwdn_dac_ovr_val, 1)
		PHY_REG_MOD_ENTRY(LCNPHY, AfeCtrlOvrVal, pwdn_dac_ovr_val, 0)
		PHY_REG_MOD_ENTRY(LCNPHY, AfeCtrlOvr, pwdn_dac_ovr, 0)
	PHY_REG_LIST_EXECUTE(pi);

	/* Perform Tx IQ, Rx IQ and PAPD cal only if no scan in progress */
	if (wlc_phy_no_cal_possible(pi)) {
		/* If they are in same band, apply the stored cal resutls and enable cals,
		other wise disable cals
		*/
#if defined(PHYCAL_CACHING)
		if (ctx) {
			wlc_phy_papd_txiqlo_rxiq_enable(pi, ctx->valid, chanspec);
		}
#else
		bool enab;
		enab = (((pi_lcn->lcnphy_full_cal_channel > 14) &&
			(CHSPEC_CHANNEL(pi->radio_chanspec) > 14)) ||
			((pi_lcn->lcnphy_full_cal_channel <= 14) &&
			(CHSPEC_CHANNEL(pi->radio_chanspec) <= 14)));
		if (!enab || (pi_lcn->lcnphy_full_cal_channel)) {
			wlc_phy_papd_txiqlo_rxiq_enable(pi, enab, chanspec);
		}
#endif /* PHYCAL_CACHING */
		return;
	}

#if defined(PHYCAL_CACHING)
	/* Fresh calibration or restoration required */
	if (!ctx) {
		if (LCNPHY_MAX_CAL_CACHE <= pi->phy_calcache_num) {
			/* Already max num ctx exist, reuse oldest */
			ctx = wlc_phy_get_chanctx_oldest(pi);
			ASSERT(ctx);
			wlc_phy_reinit_chanctx(pi, ctx, chanspec);
		} else {
			/* Prepare a fresh calibration context */
			if (BCME_OK == wlc_phy_create_chanctx((wlc_phy_t *)pi,
				pi->radio_chanspec)) {
				ctx = pi->phy_calcache;
				/* This increment is moved to wlc_create_chanctx() */
				/* pi->phy_calcache_num++; */
			}
			else
				ASSERT(ctx);
		}
	}
#endif /* PHYCAL_CACHING */

	/* 4313B0 is facing stability issues with per-channel cal */
	if (CHIPID(pi->sh->chip) != BCM4313_CHIP_ID) {
#if defined(PHYCAL_CACHING)
#if	defined(WLDSTA)
		/* We cannot afford to recal in VSDB */
		if (!ctx->valid)
#else
		/* FIXME : The below fix for channel 1 is done wrt Nokia issue,
		Doesn't seem required in other modules, require some more investigation
		*/
		if ((!ctx->valid) || (CHSPEC_CHANNEL(pi->radio_chanspec) == 1))
#endif /* defined(WLDSTA) */
#else
		if ((pi_lcn->lcnphy_full_cal_channel != CHSPEC_CHANNEL(pi->radio_chanspec)) ||
			(CHSPEC_CHANNEL(pi->radio_chanspec) == 1))
#endif /* defined(PHYCAL_CACHING) */
			wlc_lcnphy_calib_modes(pi, PHY_FULLCAL);
		else
			wlc_lcnphy_restore_calibration_results(pi);
	}
	if ((CHIPID(pi->sh->chip) == BCM4313_CHIP_ID) &&
	    pi_lcn->lcnphy_papd_cal_done_at_init && !EPA(pi_lcn)) {
		/* Before a channel change we would might already have
		 * capped the tx gain table to the capped index,
		 * before PAPD cal it is necessary to restore the gain
		 * index table and then call PAPD. Subsequently we
		 * will again calculate the cap index for the channel
		 * change
		 */
		pi_lcn->lcnphy_capped_index = 0;
		wlc_lcnphy_load_txgainwithcappedindex(pi, 0);
		wlc_lcnphy_calib_modes(pi, PHY_PAPDCAL);
		if (pi_lcn->lcnphy_CalcPapdCapEnable == 0) {
			pi_lcn->lcnphy_capped_index =
				wlc_lcnphy_tempcompensated_txpwrctrl(pi, TRUE);
			wlc_lcnphy_load_txgainwithcappedindex(pi, 1);
		}

	}
	else if ((CHIPID(pi->sh->chip) == BCM4313_CHIP_ID) && EPA(pi_lcn)) {
#ifndef PHYCAL_CACHING
		pi_lcn->lcnphy_full_cal_channel = CHSPEC_CHANNEL(pi->radio_chanspec);
#endif // endif
	}

	if (CHIPID(pi->sh->chip) != BCM4313_CHIP_ID) {
		wlc_lcnphy_set_tr_attn(pi, FALSE, 0);
	}

	wlc_lcnphy_noise_set_input_pwr_offset(pi);

	wlc_lcnphy_noise_measure_start(pi, TRUE);

	if (pi->hwpwrctrl_capable)
		wlc_lcnphy_set_tssi_pwr_limit(pi, LCNPHY_TSSI_SET_MIN_MAX_LIMIT);
}

static void
wlc_lcnphy_set_dac_gain(phy_info_t *pi, uint16 dac_gain)
{
	uint16 dac_ctrl;

	dac_ctrl =
	        (phy_utils_read_phyreg(pi, LCNPHY_AfeDACCtrl) >> LCNPHY_AfeDACCtrl_dac_ctrl_SHIFT);
	dac_ctrl = dac_ctrl & 0xc7f;
	dac_ctrl = dac_ctrl | (dac_gain << 7);
	PHY_REG_MOD(pi, LCNPHY, AfeDACCtrl, dac_ctrl, dac_ctrl);
}

static void
wlc_lcnphy_set_tx_gain_override(phy_info_t *pi, bool bEnable)
{
	uint16 bit = bEnable ? 1 : 0;

	PHY_REG_MOD(pi, LCNPHY, rfoverride2, txgainctrl_ovr, bit);
	PHY_REG_MOD(pi, LCNPHY, rfoverride2, stxtxgainctrl_ovr, bit);
	PHY_REG_MOD(pi, LCNPHY, AfeCtrlOvr, dacattctrl_ovr, bit);
}

static uint16
wlc_lcnphy_get_pa_gain(phy_info_t *pi)
{
	uint16 pa_gain;

	pa_gain = (phy_utils_read_phyreg(pi, LCNPHY_txgainctrlovrval1) &
		LCNPHY_txgainctrlovrval1_pagain_ovr_val1_MASK) >>
		LCNPHY_txgainctrlovrval1_pagain_ovr_val1_SHIFT;

	return pa_gain;
}

static void
wlc_lcnphy_set_tx_gain(phy_info_t *pi,  lcnphy_txgains_t *target_gains)
{
	uint16 pa_gain = wlc_lcnphy_get_pa_gain(pi);

	PHY_REG_MOD(pi, LCNPHY, txgainctrlovrval0, txgainctrl_ovr_val0,
		target_gains->gm_gain | (target_gains->pga_gain << 8));
	PHY_REG_MOD(pi, LCNPHY, txgainctrlovrval1, txgainctrl_ovr_val1,
		target_gains->pad_gain | (pa_gain << 8));
	PHY_REG_MOD(pi, LCNPHY, stxtxgainctrlovrval0, stxtxgainctrl_ovr_val0,
		target_gains->gm_gain | (target_gains->pga_gain << 8));
	PHY_REG_MOD(pi, LCNPHY, stxtxgainctrlovrval1, stxtxgainctrl_ovr_val1,
		target_gains->pad_gain | (pa_gain << 8));

	wlc_lcnphy_set_dac_gain(pi, target_gains->dac_gain);

	/* Enable gain overrides */
	wlc_lcnphy_enable_tx_gain_override(pi);
}

static void
wlc_lcnphy_set_bbmult(phy_info_t *pi, uint8 m0)
{
	uint16 m0m1 = (uint16)m0 << 8;
	phytbl_info_t tab;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	tab.tbl_ptr = &m0m1; /* ptr to buf */
	tab.tbl_len = 1;        /* # values   */
	tab.tbl_id = LCNPHY_TBL_ID_IQLOCAL;         /* iqloCaltbl      */
	tab.tbl_offset = 87; /* tbl offset */
	tab.tbl_width = 16;     /* 16 bit wide */
	wlc_lcnphy_write_table(pi, &tab);
}

void
wlc_lcnphy_clear_tx_power_offsets(phy_info_t *pi)
{
	uint32 data_buf[64];
	phytbl_info_t tab;
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;

	/* Clear out buffer */
	bzero(data_buf, sizeof(data_buf));

	/* Preset txPwrCtrltbl */
	tab.tbl_id = LCNPHY_TBL_ID_TXPWRCTL;
	tab.tbl_width = 32;	/* 32 bit wide	*/
	tab.tbl_ptr = data_buf; /* ptr to buf */
	/* Since 4313A0 uses the rate offset table to do tx pwr ctrl for cck, */
	/* we shouldn't be clearing the rate offset table */

	if (!pi_lcn->lcnphy_uses_rate_offset_table) {
		/* Per rate power offset */
		tab.tbl_len = 30; /* # values   */
		tab.tbl_offset = LCNPHY_TX_PWR_CTRL_RATE_OFFSET;
		wlc_lcnphy_write_table(pi, &tab);
	}
	/* Per index power offset */
	tab.tbl_len = 64; /* # values   */
	tab.tbl_offset = LCNPHY_TX_PWR_CTRL_MAC_OFFSET;
	wlc_lcnphy_write_table(pi, &tab);
}

#ifdef WL_LPC
#define LPC_MIN_IDX 33 /* Synch with same macro in wlc_rate_sel.c ??? */
#define LPC_TOT_IDX (LPC_MIN_IDX + 1)
#define LCNPHY_TX_PWR_CTRL_MACLUT_MAX_ENTRIES	64
#define LCNPHY_TX_PWR_CTRL_MACLUT_WIDTH	8

static uint8 lpc_pwr_level[LPC_TOT_IDX] =
	{0, 2, 4, 6, 8, 10,
	12, 14, 16, 18, 20,
	22, 24, 26, 28, 30,
	32, 34, 36, 38, 40,
	42, 44, 46, 48, 50,
	52, 54, 56, 58, 60,
	61, 62, 63};

void
wlc_lcnphy_lpc_mode(wlc_phy_t *ppi, bool enable)
{
	/* Nothing needs to be done here */
}

void
wlc_lcnphy_lpc_write_maclut(wlc_phy_t *ppi)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	phytbl_info_t tab;

	/* If not enabled, no need to clear out the table, just quit */
	if (!pi->lpc_algo)
		return;

	tab.tbl_id = LCNPHY_TBL_ID_TXPWRCTL;
	tab.tbl_width = LCNPHY_TX_PWR_CTRL_MACLUT_WIDTH;
	tab.tbl_offset = LCNPHY_TX_PWR_CTRL_MAC_OFFSET;
	tab.tbl_ptr = lpc_pwr_level;
	tab.tbl_len = LPC_TOT_IDX;
	wlc_lcnphy_write_table(pi, &tab);
}
#endif /* WL_LPC */

typedef enum {
	LCNPHY_TSSI_PRE_PA,
	LCNPHY_TSSI_POST_PA,
	LCNPHY_TSSI_EXT
} lcnphy_tssi_mode_t;

static void
wlc_lcnphy_set_tssi_mux(phy_info_t *pi, lcnphy_tssi_mode_t pos)
{
	/* Set TSSI/RSSI mux */
	if (LCNPHY_TSSI_POST_PA == pos) {
		PHY_REG_LIST_START
			PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlRfCtrl0, tssiSelVal0, 0)
			PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlRfCtrl0, tssiSelVal1, 1)
		PHY_REG_LIST_EXECUTE(pi);
		if (RADIOID(pi->pubpi.radioid) == BCM2064_ID) {
			PHY_REG_LIST_START
				RADIO_REG_MOD_ENTRY(RADIO_2064_REG03A, 1, 0x1)
				RADIO_REG_MOD_ENTRY(RADIO_2064_REG11A, 0x8, 0x8)
				RADIO_REG_WRITE_ENTRY(RADIO_2064_REG07F, 0)
			PHY_REG_LIST_EXECUTE(pi);
		} else {
			phy_utils_mod_radioreg(pi, RADIO_2064_REG086, 0x4, 0x4);
			phy_utils_write_radioreg(pi, RADIO_2064_REG07D, 0);
		}
		if (CHIPID(pi->sh->chip) == BCM4313_CHIP_ID) {
			PHY_REG_LIST_START
				/* TSSI1 selection ( PA out ) */
				RADIO_REG_MOD_ENTRY(RADIO_2064_REG028, 0x1, 0x0)
				/* TSSI Range */
				RADIO_REG_MOD_ENTRY(RADIO_2064_REG11A, 0x4, 1<<2)
				RADIO_REG_MOD_ENTRY(RADIO_2064_REG036, 0x10, 0x0)
				/* Sel PA out */
				RADIO_REG_MOD_ENTRY(RADIO_2064_REG11A, 0x10, 1<<4)
				RADIO_REG_MOD_ENTRY(RADIO_2064_REG036, 0x3, 0x0)
				/* env det bias current */
				RADIO_REG_MOD_ENTRY(RADIO_2064_REG035, 0xff, 0x77)
				/* select G tssi */
				RADIO_REG_MOD_ENTRY(RADIO_2064_REG028, 0x1e, 0xe<<1)
				/* select tssi at amux */
				RADIO_REG_MOD_ENTRY(RADIO_2064_REG112, 0x80, 1<<7)
				RADIO_REG_MOD_ENTRY(RADIO_2064_REG005, 0x7, 1<<1)
				/* select tssi bias */
				RADIO_REG_MOD_ENTRY(RADIO_2064_REG029, 0xf0, 0<<4)
			PHY_REG_LIST_EXECUTE(pi);
		}

	} else if (LCNPHY_TSSI_PRE_PA == pos) {
		PHY_REG_LIST_START
			PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlRfCtrl0, tssiSelVal0, 0x1)
			PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlRfCtrl0, tssiSelVal1, 0)
		PHY_REG_LIST_EXECUTE(pi);
		if (RADIOID(pi->pubpi.radioid) == BCM2064_ID) {
			phy_utils_mod_radioreg(pi, RADIO_2064_REG03A, 1, 0);
			phy_utils_mod_radioreg(pi, RADIO_2064_REG11A, 0x8, 0x8);
		} else {
			phy_utils_mod_radioreg(pi, RADIO_2064_REG086, 0x4, 0x4);
		}
	} else if (LCNPHY_TSSI_EXT == pos) {
		if (CHIPID(pi->sh->chip) == BCM4313_CHIP_ID) {
			phy_utils_write_radioreg(pi, RADIO_2064_REG07F, 1);
			phy_utils_mod_radioreg(pi, RADIO_2064_REG028, 0x1f, 3);
		} else {				/* this is for 4330 */
			if (CHSPEC_IS2G(pi->radio_chanspec))
				phy_utils_write_radioreg(pi, RADIO_2064_REG07D, 3);
			else
				phy_utils_write_radioreg(pi, RADIO_2064_REG07D, 2);
			phy_utils_mod_radioreg(pi, RADIO_2064_REG028, 0xf, 0x1);
		}
		phy_utils_mod_radioreg(pi, RADIO_2064_REG112, 0x80, 0x1 << 7);
		phy_utils_mod_radioreg(pi, RADIO_2064_REG005, 0x7, 0x2);
	}
	PHY_REG_MOD(pi, LCNPHY, TxPwrCtrlCmdNew, txPwrCtrlScheme, 0);
}

static uint16
wlc_lcnphy_rfseq_tbl_adc_pwrup(phy_info_t *pi)
{
	uint16 N1, N2, N3, N4, N5, N6, N;
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;

	if (CHSPEC_IS5G(pi->radio_chanspec)) {
		/* tune adc power down carefully,
		1600: has problems with 160 MHz spur
		600: EVM problems for legacy ofdm
		400: power detector still settling but overall good compromise
		*/
		if (pi_lcn->adcrfseq_5g != -1)
			return pi_lcn->adcrfseq_5g;
		else {
			if (EPA(pi_lcn))
				return 600;
			else
				return 400;
		}
	} else {
		if (pi_lcn->adcrfseq_2g != -1)
			return pi_lcn->adcrfseq_2g;
	}

	N1 = PHY_REG_READ(pi, LCNPHY, TxPwrCtrlNnum, Ntssi_delay);
	N2 = 1 << PHY_REG_READ(pi, LCNPHY, TxPwrCtrlNnum, Ntssi_intg_log2);
	N3 = PHY_REG_READ(pi, LCNPHY, TxPwrCtrlNum_Vbat, Nvbat_delay);
	N4 = 1 << PHY_REG_READ(pi, LCNPHY, TxPwrCtrlNum_Vbat, Nvbat_intg_log2);
	N5 = PHY_REG_READ(pi, LCNPHY, TxPwrCtrlNum_temp, Ntemp_delay);
	N6 = 1 << PHY_REG_READ(pi, LCNPHY, TxPwrCtrlNum_temp, Ntemp_intg_log2);
	N = 2 * (N1 + N2 + N3 + N4 + 2 *(N5 + N6)) + 80;
	if (N < 1600)
		N = 1600; /* min 20 us to avoid tx evm degradation */

	return N;
}

void
wlc_lcnphy_4313war(phy_info_t *pi)
{
	phytbl_info_t tab;
	uint16 rfseq;
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	/* initialize the delay value for keeping ADC powered up during Tx */
	rfseq = wlc_lcnphy_rfseq_tbl_adc_pwrup(pi);
	tab.tbl_id = LCNPHY_TBL_ID_RFSEQ;
	tab.tbl_width = 16;	 /* 12 bit wide  */
	tab.tbl_len = 1;		 /* # values   */
	tab.tbl_ptr = &rfseq; /* ptr to buf */
	tab.tbl_offset = 6;
	wlc_lcnphy_write_table(pi, &tab);
	if (!EPA(pi_lcn)) {
		rfseq = 150;
		tab.tbl_offset = 0;
		wlc_lcnphy_write_table(pi, &tab);
		rfseq = 220;
		tab.tbl_offset = 1;
		wlc_lcnphy_write_table(pi, &tab);
	}
}

static void
wlc_lcnphy_pwrctrl_rssiparams(phy_info_t *pi)
{
	uint16 auxpga_vmid, auxpga_vmid_temp, auxpga_gain_temp;
	uint16 auxpga_vmid_vbat, auxpga_gain_vbat;
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	int16 auxpga_gain = 0;

#ifdef BAND5G
	if (CHSPEC_IS5G(pi->radio_chanspec)) {
		auxpga_vmid = (2 << 8) | (pi_lcn->rssismc5g << 4) | pi_lcn->rssismf5g;
		auxpga_gain = pi_lcn->rssisav5g;
	} else
#endif // endif
	{
		auxpga_gain = pi_lcn->lcnphy_rssi_gs;
		auxpga_vmid = (2 << 8) | (pi_lcn->lcnphy_rssi_vc << 4) | pi_lcn->lcnphy_rssi_vf;
	}
	auxpga_vmid_temp = (2 << 8) | (pi_lcn->lcnphy_temp_vc << 4) | pi_lcn->lcnphy_temp_vf;
	auxpga_gain_temp = pi_lcn->lcnphy_temp_gs;

	auxpga_vmid_vbat = (2 << 8) | (pi_lcn->lcnphy_vbat_vc << 4) | pi_lcn->lcnphy_vbat_vf;
	auxpga_gain_vbat = pi_lcn->lcnphy_vbat_gs;

	PHY_REG_LIST_START
		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlRfCtrlOverride1, afeAuxpgaSelVmidOverride, 0)
		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlRfCtrlOverride1, afeAuxpgaSelGainOverride, 0)
		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlRfCtrlOverride0, amuxSelPortOverride, 0)
	PHY_REG_LIST_EXECUTE(pi);

	PHY_REG_MOD2(pi, LCNPHY, TxPwrCtrlRfCtrl2, afeAuxpgaSelVmidVal0, afeAuxpgaSelGainVal0,
		auxpga_vmid, auxpga_gain);
	PHY_REG_MOD2(pi, LCNPHY, TxPwrCtrlRfCtrl3, afeAuxpgaSelVmidVal1, afeAuxpgaSelGainVal1,
		auxpga_vmid, auxpga_gain);
	PHY_REG_MOD2(pi, LCNPHY, TxPwrCtrlRfCtrl4, afeAuxpgaSelVmidVal2, afeAuxpgaSelGainVal2,
		auxpga_vmid, auxpga_gain);
	PHY_REG_MOD2(pi, LCNPHY, TxPwrCtrlRfCtrl5, afeAuxpgaSelVmidVal3, afeAuxpgaSelGainVal3,
		auxpga_vmid_vbat, auxpga_gain_vbat);
	PHY_REG_MOD2(pi, LCNPHY, TxPwrCtrlRfCtrl6, afeAuxpgaSelVmidVal4, afeAuxpgaSelGainVal4,
		auxpga_vmid_temp, auxpga_gain_temp);

	/* for tempsense */
	if (RADIOID(pi->pubpi.radioid) != BCM2064_ID)
		phy_utils_mod_radioreg(pi, RADIO_2064_REG082, (1 << 1), (1 << 1));
	else
		phy_utils_mod_radioreg(pi, RADIO_2064_REG082, (1 << 5), (1 << 5));

	/* for vbat */
	if (RADIOID(pi->pubpi.radioid) != BCM2064_ID)
		phy_utils_mod_radioreg(pi, RADIO_2064_REG07F, (1 << 4), (1 << 4));
	else
		phy_utils_mod_radioreg(pi, RADIO_2064_REG07C, (1 << 0), (1 << 0));
}

static void
wlc_lcnphy_tssi_ucode_setup(phy_info_t *pi, uint8 update_lut)
{
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	uint16 auxpga_vmid, auxpga_gs;
	uint16 tssi_shm_addr;
	uint16 phy_reg_restore_blk, phy_reg_setup_blk;
	uint16 radio_reg_restore_blk, radio_reg_setup_blk;

	uint16 phy_restore_LUT[MAX_NUM_PHY_REGS_FAST_TSSI];
	uint16 phy_setup_LUT[MAX_NUM_PHY_REGS_FAST_TSSI];
	int phy_count = 0;

	uint16 radio_restore_LUT[MAX_NUM_RADIO_REGS_FAST_TSSI];
	uint16 radio_setup_LUT[MAX_NUM_RADIO_REGS_FAST_TSSI];
	int radio_count = 0;
	int i;
	bool update_restore_LUT = 1;

#if LCN_BCK_TSSI_DEBUG_LUT_DIRECT && LCN_BCK_TSSI_DEBUG_VIA_DUMMY
	bool tx_gain_override_old;
	lcnphy_txgains_t old_gains;
	uint16 SAVE_txpwrctrl = wlc_lcnphy_get_tx_pwr_ctrl(pi);
#endif // endif

#if LCN_BCK_TSSI_DEBUG_LUT_DIRECT
	bool suspend;
	suspend = !(R_REG(pi->sh->osh, &((phy_info_t *)pi)->regs->maccontrol) & MCTL_EN_MAC);
	if (!suspend)
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
	wlc_btcx_override_enable(pi);
#endif // endif

#if LCN_BCK_TSSI_DEBUG_LUT_DIRECT && LCN_BCK_TSSI_DEBUG_VIA_DUMMY
	/* Save old tx gains if needed */
	tx_gain_override_old = wlc_lcnphy_tx_gain_override_enabled(pi);
	wlc_lcnphy_get_tx_gain(pi, &old_gains);
#endif // endif

	if (!wlc_lcnphy_tssi_based_pwr_ctrl_enabled(pi))
		return;

	/* Get pointer to the TSSI shm block */
	if ((tssi_shm_addr = 2 * wlapi_bmac_read_shm(pi->sh->physhim, M_TSSI_MSMT_BLK_PTR)) == 0)
		return;

	if (LCNPHY_TX_PWR_CTRL_OFF == wlc_lcnphy_get_tx_pwr_ctrl(pi)) {
		wlapi_bmac_write_shm(pi->sh->physhim, tssi_shm_addr + M_LCNPHY_TSSICAL_EN, 0);
		return;
	}

	/* Check if LUT is set up already */
	if (wlapi_bmac_read_shm(pi->sh->physhim, tssi_shm_addr + M_LCNPHY_TSSICAL_EN) == 1)
		return;

	phy_reg_restore_blk = tssi_shm_addr + M_LUT_PHY_REG_RESTORE_BLK;
	phy_reg_setup_blk = tssi_shm_addr + M_LUT_PHY_REG_SETUP_BLK;
	radio_reg_restore_blk = tssi_shm_addr + M_LUT_RADIO_REG_RESTORE_BLK;
	radio_reg_setup_blk = tssi_shm_addr + M_LUT_RADIO_REG_SETUP_BLK;

	/* Fill up LUT with setup values */
		if (update_lut || LCN_BCK_TSSI_DEBUG_LUT_DIRECT) {

		/* Check if Radio override is set */
		if (PHY_REG_READ(pi, LCNPHY, rfoverride3, rfactive_ovr) == 1)
			return;

		ASSERT(phy_count < (MAX_NUM_PHY_REGS_FAST_TSSI - 2));
		phy_utils_gen_phyreg(pi, LCNPHY_TxPwrCtrlStatusTemp,
			LCNPHY_TxPwrCtrlStatusTemp_avgTemp_MASK,
			0 << LCNPHY_TxPwrCtrlStatusTemp_avgTemp_SHIFT,
			&phy_restore_LUT[phy_count], &phy_restore_LUT[phy_count+1],
			&phy_setup_LUT[phy_count], &phy_setup_LUT[phy_count+1]);
		phy_count += 2;

		ASSERT(phy_count < (MAX_NUM_PHY_REGS_FAST_TSSI - 2));
		phy_utils_gen_phyreg(pi, LCNPHY_TxPwrCtrlStatusTemp1,
			LCNPHY_TxPwrCtrlStatusTemp1_avgTemp2_MASK,
			0 << LCNPHY_TxPwrCtrlStatusTemp1_avgTemp2_SHIFT,
			&phy_restore_LUT[phy_count], &phy_restore_LUT[phy_count+1],
			&phy_setup_LUT[phy_count], &phy_setup_LUT[phy_count+1]);
		phy_count += 2;

		ASSERT(phy_count < (MAX_NUM_PHY_REGS_FAST_TSSI - 2));
		phy_utils_gen_phyreg(pi, LCNPHY_TxPwrCtrlPwrIndex,
			LCNPHY_TxPwrCtrlPwrIndex_uC_pwrIndex0_MASK |
			LCNPHY_TxPwrCtrlPwrIndex_loadPwrIndex_MASK,
			127 << LCNPHY_TxPwrCtrlPwrIndex_uC_pwrIndex0_SHIFT |
			1 << LCNPHY_TxPwrCtrlPwrIndex_loadPwrIndex_SHIFT,
			&phy_restore_LUT[phy_count], &phy_restore_LUT[phy_count+1],
			&phy_setup_LUT[phy_count], &phy_setup_LUT[phy_count+1]);
		phy_count += 2;

		/* Power Up Tx Chain */
		/* Force on DAC */
		ASSERT(phy_count < (MAX_NUM_PHY_REGS_FAST_TSSI - 2));
		phy_utils_gen_phyreg(pi, LCNPHY_AfeCtrlOvr,
			LCNPHY_AfeCtrlOvr_pwdn_dac_ovr_MASK |
			LCNPHY_AfeCtrlOvr_dac_clk_disable_ovr_MASK |
			LCNPHY_AfeCtrlOvr_pwdn_rssi_ovr_MASK,
			1 << LCNPHY_AfeCtrlOvr_pwdn_dac_ovr_SHIFT |
			1 << LCNPHY_AfeCtrlOvr_dac_clk_disable_ovr_SHIFT |
			1 << LCNPHY_AfeCtrlOvr_pwdn_rssi_ovr_SHIFT,
			&phy_restore_LUT[phy_count], &phy_restore_LUT[phy_count+1],
			&phy_setup_LUT[phy_count], &phy_setup_LUT[phy_count+1]);
		phy_count += 2;

		ASSERT(phy_count < (MAX_NUM_PHY_REGS_FAST_TSSI - 2));
		phy_utils_gen_phyreg(pi, LCNPHY_AfeCtrlOvrVal,
			LCNPHY_AfeCtrlOvrVal_pwdn_dac_ovr_val_MASK |
			LCNPHY_AfeCtrlOvrVal_dac_clk_disable_ovr_val_MASK |
			LCNPHY_AfeCtrlOvrVal_pwdn_rssi_ovr_val_MASK,
			0 << LCNPHY_AfeCtrlOvrVal_pwdn_dac_ovr_val_SHIFT |
			0 << LCNPHY_AfeCtrlOvrVal_dac_clk_disable_ovr_val_SHIFT |
			0 << LCNPHY_AfeCtrlOvrVal_pwdn_rssi_ovr_val_SHIFT,
			&phy_restore_LUT[phy_count], &phy_restore_LUT[phy_count+1],
			&phy_setup_LUT[phy_count], &phy_setup_LUT[phy_count+1]);
		phy_count += 2;

		/* Force on the transmit chain */
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			/* Force on the Gband-ePA and force off the Aband-ePA */
			ASSERT(phy_count < (MAX_NUM_PHY_REGS_FAST_TSSI - 2));
			phy_utils_gen_phyreg(pi, LCNPHY_RFOverride0,
				LCNPHY_RFOverride0_internalrftxpu_ovr_MASK |
				LCNPHY_RFOverride0_gmode_tx_pu_ovr_MASK |
				LCNPHY_RFOverride0_amode_tx_pu_ovr_MASK |
				LCNPHY_RFOverride0_trsw_tx_pu_ovr_MASK |
				LCNPHY_RFOverride0_trsw_rx_pu_ovr_MASK,
				1 << LCNPHY_RFOverride0_internalrftxpu_ovr_SHIFT |
				1 << LCNPHY_RFOverride0_gmode_tx_pu_ovr_SHIFT |
				1 << LCNPHY_RFOverride0_amode_tx_pu_ovr_SHIFT |
				1 << LCNPHY_RFOverride0_trsw_tx_pu_ovr_SHIFT |
				1 << LCNPHY_RFOverride0_trsw_rx_pu_ovr_SHIFT,
				&phy_restore_LUT[phy_count], &phy_restore_LUT[phy_count+1],
				&phy_setup_LUT[phy_count], &phy_setup_LUT[phy_count+1]);
			phy_count += 2;

			ASSERT(phy_count < (MAX_NUM_PHY_REGS_FAST_TSSI - 2));
			phy_utils_gen_phyreg(pi, LCNPHY_RFOverrideVal0,
				LCNPHY_RFOverrideVal0_internalrftxpu_ovr_val_MASK |
				LCNPHY_RFOverrideVal0_gmode_tx_pu_ovr_val_MASK |
				LCNPHY_RFOverrideVal0_amode_tx_pu_ovr_val_MASK |
				LCNPHY_RFOverrideVal0_trsw_tx_pu_ovr_val_MASK |
				LCNPHY_RFOverrideVal0_trsw_rx_pu_ovr_val_MASK,
				1 << LCNPHY_RFOverrideVal0_internalrftxpu_ovr_val_SHIFT |
				1 << LCNPHY_RFOverrideVal0_gmode_tx_pu_ovr_val_SHIFT |
				0 << LCNPHY_RFOverrideVal0_amode_tx_pu_ovr_val_SHIFT |
				1 << LCNPHY_RFOverrideVal0_trsw_tx_pu_ovr_val_SHIFT |
				0 << LCNPHY_RFOverrideVal0_trsw_rx_pu_ovr_val_SHIFT,
				&phy_restore_LUT[phy_count], &phy_restore_LUT[phy_count+1],
				&phy_setup_LUT[phy_count], &phy_setup_LUT[phy_count+1]);
			phy_count += 2;

			/* PAD PU, PGA PU and PA PU */
			ASSERT(phy_count < (MAX_NUM_PHY_REGS_FAST_TSSI - 2));
			phy_utils_gen_phyreg(pi, LCNPHY_rfoverride3,
				LCNPHY_rfoverride3_stxpadpu2g_ovr_MASK |
				LCNPHY_rfoverride3_stxpapu2g_ovr_MASK |
				LCNPHY_rfoverride3_stxpapu_ovr_MASK,
				1 << LCNPHY_rfoverride3_stxpadpu2g_ovr_SHIFT |
				1 << LCNPHY_rfoverride3_stxpapu2g_ovr_SHIFT |
				1 << LCNPHY_rfoverride3_stxpapu_ovr_SHIFT,
				&phy_restore_LUT[phy_count], &phy_restore_LUT[phy_count+1],
				&phy_setup_LUT[phy_count], &phy_setup_LUT[phy_count+1]);
			phy_count += 2;

			ASSERT(phy_count < (MAX_NUM_PHY_REGS_FAST_TSSI - 2));
			phy_utils_gen_phyreg(pi, LCNPHY_rfoverride3_val,
				LCNPHY_rfoverride3_val_stxpadpu2g_ovr_val_MASK |
				LCNPHY_rfoverride3_val_stxpapu2g_ovr_val_MASK |
				LCNPHY_rfoverride3_val_stxpapu_ovr_val_MASK,
				1 << LCNPHY_rfoverride3_val_stxpadpu2g_ovr_val_SHIFT |
				1 << LCNPHY_rfoverride3_val_stxpapu2g_ovr_val_SHIFT |
				1 << LCNPHY_rfoverride3_val_stxpapu_ovr_val_SHIFT,
				&phy_restore_LUT[phy_count], &phy_restore_LUT[phy_count+1],
				&phy_setup_LUT[phy_count], &phy_setup_LUT[phy_count+1]);
			phy_count += 2;

		} else {
			/* Force off the Gband-ePA and force on the Aband-ePA */
			ASSERT(phy_count < (MAX_NUM_PHY_REGS_FAST_TSSI - 2));
			phy_utils_gen_phyreg(pi, LCNPHY_RFOverride0,
				LCNPHY_RFOverride0_internalrftxpu_ovr_MASK |
				LCNPHY_RFOverride0_gmode_tx_pu_ovr_MASK |
				LCNPHY_RFOverride0_amode_tx_pu_ovr_MASK |
				LCNPHY_RFOverride0_trsw_tx_pu_ovr_MASK |
				LCNPHY_RFOverride0_trsw_rx_pu_ovr_MASK,
				1 << LCNPHY_RFOverride0_internalrftxpu_ovr_SHIFT |
				1 << LCNPHY_RFOverride0_gmode_tx_pu_ovr_SHIFT |
				1 << LCNPHY_RFOverride0_amode_tx_pu_ovr_SHIFT |
				1 << LCNPHY_RFOverride0_trsw_tx_pu_ovr_SHIFT |
				1 << LCNPHY_RFOverride0_trsw_rx_pu_ovr_SHIFT,
				&phy_restore_LUT[phy_count], &phy_restore_LUT[phy_count+1],
				&phy_setup_LUT[phy_count], &phy_setup_LUT[phy_count+1]);
			phy_count += 2;

			ASSERT(phy_count < (MAX_NUM_PHY_REGS_FAST_TSSI - 2));
			phy_utils_gen_phyreg(pi, LCNPHY_RFOverrideVal0,
				LCNPHY_RFOverrideVal0_internalrftxpu_ovr_val_MASK |
				LCNPHY_RFOverrideVal0_gmode_tx_pu_ovr_val_MASK |
				LCNPHY_RFOverrideVal0_amode_tx_pu_ovr_val_MASK |
				LCNPHY_RFOverrideVal0_trsw_tx_pu_ovr_val_MASK |
				LCNPHY_RFOverrideVal0_trsw_rx_pu_ovr_val_MASK,
				1 << LCNPHY_RFOverrideVal0_internalrftxpu_ovr_val_SHIFT |
				0 << LCNPHY_RFOverrideVal0_gmode_tx_pu_ovr_val_SHIFT |
				1 << LCNPHY_RFOverrideVal0_amode_tx_pu_ovr_val_SHIFT |
				1 << LCNPHY_RFOverrideVal0_trsw_tx_pu_ovr_val_SHIFT |
				0 << LCNPHY_RFOverrideVal0_trsw_rx_pu_ovr_val_SHIFT,
				&phy_restore_LUT[phy_count], &phy_restore_LUT[phy_count+1],
				&phy_setup_LUT[phy_count], &phy_setup_LUT[phy_count+1]);
			phy_count += 2;

			/* PAD PU, PGA PU and PA PU */
			ASSERT(phy_count < (MAX_NUM_PHY_REGS_FAST_TSSI - 2));
			phy_utils_gen_phyreg(pi, LCNPHY_rfoverride3,
				LCNPHY_rfoverride3_stxpadpu2g_ovr_MASK |
				LCNPHY_rfoverride3_stxpapu2g_ovr_MASK |
				LCNPHY_rfoverride3_stxpapu_ovr_MASK,
				1 << LCNPHY_rfoverride3_stxpadpu2g_ovr_SHIFT |
				1 << LCNPHY_rfoverride3_stxpapu2g_ovr_SHIFT |
				1 << LCNPHY_rfoverride3_stxpapu_ovr_SHIFT,
				&phy_restore_LUT[phy_count], &phy_restore_LUT[phy_count+1],
				&phy_setup_LUT[phy_count], &phy_setup_LUT[phy_count+1]);
			phy_count += 2;

			ASSERT(phy_count < (MAX_NUM_PHY_REGS_FAST_TSSI - 2));
			phy_utils_gen_phyreg(pi, LCNPHY_rfoverride3_val,
				LCNPHY_rfoverride3_val_stxpadpu2g_ovr_val_MASK |
				LCNPHY_rfoverride3_val_stxpapu2g_ovr_val_MASK |
				LCNPHY_rfoverride3_val_stxpapu_ovr_val_MASK,
				0 << LCNPHY_rfoverride3_val_stxpadpu2g_ovr_val_SHIFT |
				0 << LCNPHY_rfoverride3_val_stxpapu2g_ovr_val_SHIFT |
				0 << LCNPHY_rfoverride3_val_stxpapu_ovr_val_SHIFT,
				&phy_restore_LUT[phy_count], &phy_restore_LUT[phy_count+1],
				&phy_setup_LUT[phy_count], &phy_setup_LUT[phy_count+1]);
			phy_count += 2;

		}

		/* jtag_bb_afe_switch, jtag_auxpga, iqadc_aux_en */
		ASSERT(radio_count < (MAX_NUM_RADIO_REGS_FAST_TSSI - 2));
		phy_utils_gen_radioreg(pi, RADIO_2064_REG007, 0x1, 1,
			&radio_restore_LUT[radio_count], &radio_restore_LUT[radio_count+1],
			&radio_setup_LUT[radio_count], &radio_setup_LUT[radio_count+1]);
		radio_count += 2;

		ASSERT(radio_count < (MAX_NUM_RADIO_REGS_FAST_TSSI - 2));
		phy_utils_gen_radioreg(pi, RADIO_2064_REG0FF, 0x10, 1 << 4,
			&radio_restore_LUT[radio_count], &radio_restore_LUT[radio_count+1],
			&radio_setup_LUT[radio_count], &radio_setup_LUT[radio_count+1]);
		radio_count += 2;

		ASSERT(radio_count < (MAX_NUM_RADIO_REGS_FAST_TSSI - 2));
		phy_utils_gen_radioreg(pi, RADIO_2064_REG11F, 0x4, 1 << 2,
			&radio_restore_LUT[radio_count], &radio_restore_LUT[radio_count+1],
			&radio_setup_LUT[radio_count], &radio_setup_LUT[radio_count+1]);
		radio_count += 2;

		/* Switch on the PA and set Mux control for signals going to ADC. */
		/* The enable signals are not mutually exclusive, enable Rx Buf */
		ASSERT(phy_count < (MAX_NUM_PHY_REGS_FAST_TSSI - 2));
		phy_utils_gen_phyreg(pi, LCNPHY_rfoverride4,
			LCNPHY_rfoverride4_lpf_byp_rx_ovr_MASK |
			LCNPHY_rfoverride4_lpf_buf_pwrup_ovr_MASK |
			LCNPHY_rfoverride4_papu_ovr_MASK,
			1 << LCNPHY_rfoverride4_lpf_byp_rx_ovr_SHIFT |
			1 << LCNPHY_rfoverride4_lpf_buf_pwrup_ovr_SHIFT |
			1 << LCNPHY_rfoverride4_papu_ovr_SHIFT,
			&phy_restore_LUT[phy_count], &phy_restore_LUT[phy_count+1],
			&phy_setup_LUT[phy_count], &phy_setup_LUT[phy_count+1]);
		phy_count += 2;

		ASSERT(phy_count < (MAX_NUM_PHY_REGS_FAST_TSSI - 2));
		phy_utils_gen_phyreg(pi, LCNPHY_rfoverride4val,
			LCNPHY_rfoverride4val_lpf_byp_rx_ovr_val_MASK |
			LCNPHY_rfoverride4val_lpf_buf_pwrup_ovr_val_MASK |
			LCNPHY_rfoverride4val_papu_ovr_val_MASK,
			0 << LCNPHY_rfoverride4val_lpf_byp_rx_ovr_val_SHIFT |
			1 << LCNPHY_rfoverride4val_lpf_buf_pwrup_ovr_val_SHIFT |
			1 << LCNPHY_rfoverride4val_papu_ovr_val_SHIFT,
			&phy_restore_LUT[phy_count], &phy_restore_LUT[phy_count+1],
			&phy_setup_LUT[phy_count], &phy_setup_LUT[phy_count+1]);
		phy_count += 2;

		/* Mux Setting for TSSI, amux sel Port, and PA Pwrup */
		if (RADIOID(pi->pubpi.radioid) != BCM2064_ID) {
			ASSERT(radio_count < (MAX_NUM_RADIO_REGS_FAST_TSSI - 2));
			phy_utils_gen_radioreg(pi, RADIO_2064_REG131, 1<<0, 1<<0,
				&radio_restore_LUT[radio_count], &radio_restore_LUT[radio_count+1],
				&radio_setup_LUT[radio_count], &radio_setup_LUT[radio_count+1]);
			radio_count += 2;

			if (EPA(pi_lcn)) {
				if (CHSPEC_IS2G(pi->radio_chanspec)) {
					ASSERT(radio_count < (MAX_NUM_RADIO_REGS_FAST_TSSI - 2));
					phy_utils_gen_radioreg(pi, RADIO_2064_REG07D, 0x3, 3,
						&radio_restore_LUT[radio_count],
						&radio_restore_LUT[radio_count+1],
						&radio_setup_LUT[radio_count],
						&radio_setup_LUT[radio_count+1]);
					radio_count += 2;
				}
				else {
					ASSERT(radio_count < (MAX_NUM_RADIO_REGS_FAST_TSSI - 2));
					phy_utils_gen_radioreg(pi, RADIO_2064_REG07D, 0x3, 2,
						&radio_restore_LUT[radio_count],
						&radio_restore_LUT[radio_count+1],
						&radio_setup_LUT[radio_count],
						&radio_setup_LUT[radio_count+1]);
					radio_count += 2;
				}

				ASSERT(radio_count < (MAX_NUM_RADIO_REGS_FAST_TSSI - 2));
				phy_utils_gen_radioreg(pi, RADIO_2064_REG028,
				                       ((1<<5) | 0xf), ((1<<5) | 1),
					&radio_restore_LUT[radio_count],
					&radio_restore_LUT[radio_count+1],
					&radio_setup_LUT[radio_count],
					&radio_setup_LUT[radio_count+1]);
				radio_count += 2;

			}
			else {
				uint8 tssi_sel;
				if (CHSPEC_IS2G(pi->radio_chanspec))
					tssi_sel = 0xe;
				else
					tssi_sel = 0xc;

				ASSERT(radio_count < (MAX_NUM_RADIO_REGS_FAST_TSSI - 2));
				phy_utils_gen_radioreg(pi, RADIO_2064_REG028,
				((1<<5) | 0xf), ((1<<5) | tssi_sel),
				&radio_restore_LUT[radio_count], &radio_restore_LUT[radio_count+1],
				&radio_setup_LUT[radio_count], &radio_setup_LUT[radio_count+1]);
				radio_count += 2;
			}

			ASSERT(radio_count < (MAX_NUM_RADIO_REGS_FAST_TSSI - 2));
			phy_utils_gen_radioreg(pi, RADIO_2064_REG03A, 0x3, 0x3,
				&radio_restore_LUT[radio_count], &radio_restore_LUT[radio_count+1],
				&radio_setup_LUT[radio_count], &radio_setup_LUT[radio_count+1]);
			radio_count += 2;

			ASSERT(radio_count < (MAX_NUM_RADIO_REGS_FAST_TSSI - 2));
			phy_utils_gen_radioreg(pi, RADIO_2064_REG11A, 0x1, 1 << 0,
				&radio_restore_LUT[radio_count], &radio_restore_LUT[radio_count+1],
				&radio_setup_LUT[radio_count], &radio_setup_LUT[radio_count+1]);
			radio_count += 2;

			/* Power off TX Mixer because of leakage seen in Uno3 */
			ASSERT(radio_count < (MAX_NUM_RADIO_REGS_FAST_TSSI - 2));
			phy_utils_gen_radioreg(pi, RADIO_2064_REG12E, 1 << 2, 1 << 2,
				&radio_restore_LUT[radio_count], &radio_restore_LUT[radio_count+1],
				&radio_setup_LUT[radio_count], &radio_setup_LUT[radio_count+1]);
			radio_count += 2;

			ASSERT(radio_count < (MAX_NUM_RADIO_REGS_FAST_TSSI - 2));
			phy_utils_gen_radioreg(pi, RADIO_2064_REG090, 0xff, 0,
				&radio_restore_LUT[radio_count], &radio_restore_LUT[radio_count+1],
				&radio_setup_LUT[radio_count], &radio_setup_LUT[radio_count+1]);
			radio_count += 2;
		}
		else {
			ASSERT(radio_count < (MAX_NUM_RADIO_REGS_FAST_TSSI - 2));
			phy_utils_gen_radioreg(pi, RADIO_2064_REG03A, 1, 0x1,
				&radio_restore_LUT[radio_count], &radio_restore_LUT[radio_count+1],
				&radio_setup_LUT[radio_count], &radio_setup_LUT[radio_count+1]);
			radio_count += 2;

			ASSERT(radio_count < (MAX_NUM_RADIO_REGS_FAST_TSSI - 2));
			phy_utils_gen_radioreg(pi, RADIO_2064_REG11A, 0x8, 0x8,
				&radio_restore_LUT[radio_count], &radio_restore_LUT[radio_count+1],
				&radio_setup_LUT[radio_count], &radio_setup_LUT[radio_count+1]);
			radio_count += 2;

			ASSERT(radio_count < (MAX_NUM_RADIO_REGS_FAST_TSSI - 2));
			phy_utils_gen_radioreg(pi, RADIO_2064_REG005, 0x8, 1 << 3,
				&radio_restore_LUT[radio_count], &radio_restore_LUT[radio_count+1],
				&radio_setup_LUT[radio_count], &radio_setup_LUT[radio_count+1]);
			radio_count += 2;

			ASSERT(radio_count < (MAX_NUM_RADIO_REGS_FAST_TSSI - 2));
			phy_utils_gen_radioreg(pi, RADIO_2064_REG03A, 0x4, 1 << 2,
				&radio_restore_LUT[radio_count], &radio_restore_LUT[radio_count+1],
				&radio_setup_LUT[radio_count], &radio_setup_LUT[radio_count+1]);
			radio_count += 2;

			ASSERT(radio_count < (MAX_NUM_RADIO_REGS_FAST_TSSI - 2));
			phy_utils_gen_radioreg(pi, RADIO_2064_REG11A, 0x1, 1 << 0,
				&radio_restore_LUT[radio_count], &radio_restore_LUT[radio_count+1],
				&radio_setup_LUT[radio_count], &radio_setup_LUT[radio_count+1]);
			radio_count += 2;
		}

		/* tssiRangeOverrideVal: 0 - Bypass attenuator */
		ASSERT(phy_count < (MAX_NUM_PHY_REGS_FAST_TSSI - 2));
		phy_utils_gen_phyreg(pi, LCNPHY_TxPwrCtrlRfCtrlOverride0,
			LCNPHY_TxPwrCtrlRfCtrlOverride0_tssiSelOverride_MASK |
			LCNPHY_TxPwrCtrlRfCtrlOverride0_tssiSelOverrideVal_MASK |
			LCNPHY_TxPwrCtrlRfCtrlOverride0_tssiRangeOverride_MASK |
			LCNPHY_TxPwrCtrlRfCtrlOverride0_tssiRangeOverrideVal_MASK |
			LCNPHY_TxPwrCtrlRfCtrlOverride0_amuxSelPortOverride_MASK |
			LCNPHY_TxPwrCtrlRfCtrlOverride0_amuxSelPortOverrideVal_MASK,
			1 << LCNPHY_TxPwrCtrlRfCtrlOverride0_tssiSelOverride_SHIFT |
			0 << LCNPHY_TxPwrCtrlRfCtrlOverride0_tssiSelOverrideVal_SHIFT |
			1 << LCNPHY_TxPwrCtrlRfCtrlOverride0_tssiRangeOverride_SHIFT |
			1 << LCNPHY_TxPwrCtrlRfCtrlOverride0_tssiRangeOverrideVal_SHIFT |
			1 << LCNPHY_TxPwrCtrlRfCtrlOverride0_amuxSelPortOverride_SHIFT |
			2 << LCNPHY_TxPwrCtrlRfCtrlOverride0_amuxSelPortOverrideVal_SHIFT,
			&phy_restore_LUT[phy_count], &phy_restore_LUT[phy_count+1],
			&phy_setup_LUT[phy_count], &phy_setup_LUT[phy_count+1]);
		phy_count += 2;

		/* for tempsense o_wrf_jtag_tempsense_pwrup */
		if (RADIOID(pi->pubpi.radioid) != BCM2064_ID) {
			ASSERT(radio_count < (MAX_NUM_RADIO_REGS_FAST_TSSI - 2));
			phy_utils_gen_radioreg(pi, RADIO_2064_REG082, (1 << 1), (1 << 1),
				&radio_restore_LUT[radio_count], &radio_restore_LUT[radio_count+1],
				&radio_setup_LUT[radio_count], &radio_setup_LUT[radio_count+1]);
			radio_count += 2;
		} else {
			ASSERT(radio_count < (MAX_NUM_RADIO_REGS_FAST_TSSI - 2));
			phy_utils_gen_radioreg(pi, RADIO_2064_REG082, (1 << 5), (1 << 5),
				&radio_restore_LUT[radio_count], &radio_restore_LUT[radio_count+1],
				&radio_setup_LUT[radio_count], &radio_setup_LUT[radio_count+1]);
			radio_count += 2;
		}

	#ifdef BAND5G
		if (CHSPEC_IS5G(pi->radio_chanspec)) {
			auxpga_vmid = ((2 << 8) | (pi_lcn->rssismc5g << 4) |
			               pi_lcn->rssismf5g);
			auxpga_gs = pi_lcn->rssisav5g;
		} else
	#endif
		{
			auxpga_vmid = ((2 << 8) | (pi_lcn->lcnphy_rssi_vc << 4) |
			               pi_lcn->lcnphy_rssi_vf);
			auxpga_gs = pi_lcn->lcnphy_rssi_gs;
		}

		ASSERT(phy_count < (MAX_NUM_PHY_REGS_FAST_TSSI - 2));
		phy_utils_gen_phyreg(pi, LCNPHY_TxPwrCtrlRfCtrlOverride1,
			LCNPHY_TxPwrCtrlRfCtrlOverride1_afeAuxpgaSelVmidOverride_MASK |
			LCNPHY_TxPwrCtrlRfCtrlOverride1_afeAuxpgaSelVmidOverrideVal_MASK |
			LCNPHY_TxPwrCtrlRfCtrlOverride1_afeAuxpgaSelGainOverride_MASK |
			LCNPHY_TxPwrCtrlRfCtrlOverride1_afeAuxpgaSelGainOverrideVal_MASK,
			1 << LCNPHY_TxPwrCtrlRfCtrlOverride1_afeAuxpgaSelVmidOverride_SHIFT |
			auxpga_vmid <<
			LCNPHY_TxPwrCtrlRfCtrlOverride1_afeAuxpgaSelVmidOverrideVal_SHIFT |
			1 << LCNPHY_TxPwrCtrlRfCtrlOverride1_afeAuxpgaSelGainOverride_SHIFT |
			auxpga_gs <<
			LCNPHY_TxPwrCtrlRfCtrlOverride1_afeAuxpgaSelGainOverrideVal_SHIFT,
			&phy_restore_LUT[phy_count], &phy_restore_LUT[phy_count+1],
			&phy_setup_LUT[phy_count], &phy_setup_LUT[phy_count+1]);
		phy_count += 2;

#if LCN_BCK_TSSI_UCODE_POST
		ASSERT(phy_count < MAX_NUM_PHY_REGS_FAST_TSSI_UCODE);
		ASSERT(radio_count < MAX_NUM_RADIO_REGS_FAST_TSSI_UCODE);
#endif // endif
		/* Fill up Shared Memory LUT with Restore and Setup values */
		for (i = 0; i < phy_count; i++) {
			wlapi_bmac_write_shm(pi->sh->physhim,
				(phy_reg_restore_blk + 2*i), phy_restore_LUT[i]); /* Reg Addr */
			wlapi_bmac_write_shm(pi->sh->physhim,
				(phy_reg_setup_blk + 2*i), phy_setup_LUT[i]); /* Reg Addr */
			i++;
			/* NOTE: ucode will overwrite the Restore LUTs
			to avoid Race Condition irrespective of this
			*/
			if (update_restore_LUT)
				wlapi_bmac_write_shm(pi->sh->physhim,
					(phy_reg_restore_blk + 2*i), phy_restore_LUT[i]);
			wlapi_bmac_write_shm(pi->sh->physhim,
				(phy_reg_setup_blk + 2*i), phy_setup_LUT[i]);
		}

		for (i = 0; i < radio_count; i++) {
			wlapi_bmac_write_shm(pi->sh->physhim,
				(radio_reg_restore_blk + 2*i), radio_restore_LUT[i]); /* Reg Addr */
			wlapi_bmac_write_shm(pi->sh->physhim,
				(radio_reg_setup_blk + 2*i), radio_setup_LUT[i]); /* Reg Addr */
			i++;
			/* NOTE: ucode will overwrite the Restore LUTs
			to avoid Race Condition irrespective of this
			*/
			if (update_restore_LUT)
				wlapi_bmac_write_shm(pi->sh->physhim,
					(radio_reg_restore_blk + 2*i), radio_restore_LUT[i]);
			wlapi_bmac_write_shm(pi->sh->physhim,
				(radio_reg_setup_blk + 2*i), radio_setup_LUT[i]);
		}

		wlapi_bmac_write_shm(pi->sh->physhim,
			tssi_shm_addr + M_PHY_REG_LUT_CNT, (uint16) phy_count>>1);
		wlapi_bmac_write_shm(pi->sh->physhim,
			tssi_shm_addr + M_RADIO_REG_LUT_CNT, radio_count>>1);
	}
	pi_lcn->lcnphy_last_tssical = pi->sh->now;

#if LCN_BCK_TSSI_DEBUG
	printf("\n\nPHY LUT: Setup and Restore\n");
	phy_count =
		wlapi_bmac_read_shm(pi->sh->physhim, tssi_shm_addr + M_PHY_REG_LUT_CNT) << 1;
	for (i = 0; i < phy_count; i++) {
		printf("%x, %x, %x, %x\n",
			wlapi_bmac_read_shm(pi->sh->physhim, (phy_reg_setup_blk + 2*i)),
			wlapi_bmac_read_shm(pi->sh->physhim, (phy_reg_setup_blk + 2*(i+1))),
			wlapi_bmac_read_shm(pi->sh->physhim, (phy_reg_restore_blk + 2*i)),
			wlapi_bmac_read_shm(pi->sh->physhim, (phy_reg_restore_blk + 2*(i+1))));

		i = i+1;
	}

	printf("\n\nRADIO LUT: Setup and Restore\n");
	radio_count =
		wlapi_bmac_read_shm(pi->sh->physhim, tssi_shm_addr + M_RADIO_REG_LUT_CNT) << 1;
	for (i = 0; i < radio_count; i++) {
		printf("%x, %x, %x, %x\n",
			wlapi_bmac_read_shm(pi->sh->physhim, (radio_reg_setup_blk + 2*i)),
			wlapi_bmac_read_shm(pi->sh->physhim, (radio_reg_setup_blk + 2*(i+1))),
			wlapi_bmac_read_shm(pi->sh->physhim, (radio_reg_restore_blk + 2*i)),
			wlapi_bmac_read_shm(pi->sh->physhim, (radio_reg_restore_blk + 2*(i+1))));

		i = i+1;
	}
#endif /* LCN_BCK_TSSI_DEBUG */

#if LCN_BCK_TSSI_UCODE_POST

	wlapi_bmac_write_shm(pi->sh->physhim, tssi_shm_addr + M_LCNPHY_TSSICAL_DELAY,
		(uint16) pi_lcn->lcnphy_tssical_txdelay);

	/* Indicate to ucode to start idleTSSI measurement */
	wlapi_bmac_write_shm(pi->sh->physhim, tssi_shm_addr + M_LCNPHY_TSSICAL_EN, 1);

#elif LCN_BCK_TSSI_DEBUG_LUT_DIRECT
	{
		uint16 tempsenseval1, tempsenseval2, vbat;

		for (i = 0; i < phy_count; i++) {
			phy_utils_write_phyreg(pi, phy_setup_LUT[i], phy_setup_LUT[i+1]);
			i = i+1;
		}

		for (i = 0; i < radio_count; i++) {
			phy_utils_write_radioreg(pi, radio_setup_LUT[i], radio_setup_LUT[i+1]);
			i = i+1;
		}

	#if LCN_BCK_TSSI_DEBUG_VIA_DUMMY
		{
		uint16 idleTssi0_2C;

		wlc_lcnphy_set_tx_pwr_ctrl(pi, LCNPHY_TX_PWR_CTRL_OFF);
		wlc_lcnphy_enable_tx_gain_override(pi);
		wlc_lcnphy_set_tx_pwr_by_index(pi, 127);
		wlc_lcnphy_set_bbmult(pi, 0x0);

		wlc_phy_do_dummy_tx(pi, TRUE, OFF);
		idleTssi0_2C = PHY_REG_READ(pi, LCNPHY, TxPwrCtrlStatusNew4, avgTssi);
		tempsenseval1 = phy_utils_read_phyreg(pi, LCNPHY_TxPwrCtrlStatusTemp);
		tempsenseval2 = phy_utils_read_phyreg(pi, LCNPHY_TxPwrCtrlStatusTemp1);
		vbat = phy_utils_read_phyreg(pi, LCNPHY_TxPwrCtrlStatusVbat);
		PHY_REG_MOD(pi, LCNPHY, TxPwrCtrlIdleTssi, idleTssi0, idleTssi0_2C);
		printf("Dummy: idleTssi0_2C = %d, tempsenval1 = %d, tempsenval2 = %d, vbat = %d\n",
			idleTssi0_2C, tempsenseval1 & 0x1FF, tempsenseval2 & 0x1FF, vbat & 0x1FF);
		}
	#else
		OSL_DELAY(100);
		PHY_REG_MOD(pi, LCNPHY, TxPwrCtrlRangeCmd, force_vbatTemp, 1);
		OSL_DELAY(100);
		tempsenseval1 = phy_utils_read_phyreg(pi, LCNPHY_TxPwrCtrlStatusTemp);
		tempsenseval2 = phy_utils_read_phyreg(pi, LCNPHY_TxPwrCtrlStatusTemp1);
		vbat = phy_utils_read_phyreg(pi, LCNPHY_TxPwrCtrlStatusVbat);
		printf("Force_Vbat: tempsenval1 = %d, tempsenval2 = %d, vbat = %d\n",
			tempsenseval1 & 0x1FF, tempsenseval2 & 0x1FF, vbat & 0x1FF);

		PHY_REG_MOD(pi, LCNPHY, TxPwrCtrlIdleTssi, rawTssiOffsetBinFormat, 1);
		PHY_REG_MOD(pi, LCNPHY, TxPwrCtrlIdleTssi, idleTssi0, tempsenseval1 & 0x1FF);

		/* Get tempsense back */
		PHY_REG_LIST_START
			PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlRfCtrlOverride0, amuxSelPortOverride, 0)
			PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlRfCtrlOverride1,
				afeAuxpgaSelVmidOverride, 0)
			PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlRfCtrlOverride1,
				afeAuxpgaSelGainOverride, 0)
		PHY_REG_LIST_EXECUTE(pi);

		OSL_DELAY(3);
		PHY_REG_MOD(pi, LCNPHY, TxPwrCtrlRangeCmd, force_vbatTemp, 1);
		OSL_DELAY(6);
		tempsenseval1 = phy_utils_read_phyreg(pi, LCNPHY_TxPwrCtrlStatusTemp);
		tempsenseval2 = phy_utils_read_phyreg(pi, LCNPHY_TxPwrCtrlStatusTemp1);
		printf("Tempsense vals: tempsenval1 = %d, tempsenval2 = %d\n",
			tempsenseval1 & 0x1FF, tempsenseval2 & 0x1FF);

	#endif /* LCN_BCK_TSSI_DEBUG_VIA_DUMMY */

		for (i = 0; i < phy_count; i++) {
			phy_utils_write_phyreg(pi, phy_restore_LUT[i], phy_restore_LUT[i+1]);
			i = i+1;
		}

		for (i = 0; i < radio_count; i++) {
			phy_utils_write_radioreg(pi, radio_restore_LUT[i], radio_restore_LUT[i+1]);
			i = i+1;
		}

	#if LCN_BCK_TSSI_DEBUG_VIA_DUMMY
		/* restore txgain override */
		wlc_lcnphy_set_tx_gain_override(pi, tx_gain_override_old);
		wlc_lcnphy_set_tx_gain(pi, &old_gains);
		wlc_lcnphy_set_tx_pwr_ctrl(pi, SAVE_txpwrctrl);
	#endif
	}
#endif /* LCN_BCK_TSSI_UCODE_POST */

#if LCN_BCK_TSSI_DEBUG_LUT_DIRECT
	if (!suspend)
		wlapi_enable_mac(pi->sh->physhim);
#endif // endif

}

static void
wlc_lcnphy_tssi_setup(phy_info_t *pi)
{
	phytbl_info_t tab;
	uint32 ind;
	uint16 rfseq;
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	uint8 tssi_sel;

	/* Setup estPwrLuts for measuring idle TSSI */
	tab.tbl_id = LCNPHY_TBL_ID_TXPWRCTL;
	tab.tbl_width = 32;	/* 32 bit wide	*/
	tab.tbl_ptr = &ind; /* ptr to buf */
	tab.tbl_len = 1;        /* # values   */
	tab.tbl_offset = 0;
	for (ind = 0; ind < 128; ind++) {
		wlc_lcnphy_write_table(pi,  &tab);
		tab.tbl_offset++;
	}
	tab.tbl_offset = LCNPHY_TX_PWR_CTRL_EST_PWR_OFFSET;
	for (ind = 0; ind < 128; ind++) {
		wlc_lcnphy_write_table(pi,  &tab);
		tab.tbl_offset++;
	}

	PHY_REG_LIST_START
		PHY_REG_MOD_ENTRY(LCNPHY, auxadcCtrl, rssifiltEn, 0)
		PHY_REG_MOD_ENTRY(LCNPHY, auxadcCtrl, rssiFormatConvEn, 0)
		PHY_REG_MOD_ENTRY(LCNPHY, auxadcCtrl, txpwrctrlEn, 1)
	PHY_REG_LIST_EXECUTE(pi);

	if (!EPA(pi_lcn)) {
		wlc_lcnphy_set_tssi_mux(pi, LCNPHY_TSSI_POST_PA);
	} else {
		wlc_lcnphy_set_tssi_mux(pi, LCNPHY_TSSI_EXT);
	}

	PHY_REG_LIST_START
		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlCmd, hwtxPwrCtrl_en, 0)
		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlCmd, txPwrCtrl_en, 1)
		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlRangeCmd, force_vbatTemp, 0)
		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlCmd, pwrIndex_init, 0)
		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlNnum, Ntssi_delay, 255)
		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlNnum, Ntssi_intg_log2, 5)
		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlNnum, Npt_intg_log2, 0)
		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlNum_Vbat, Nvbat_delay, 64)
		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlNum_Vbat, Nvbat_intg_log2, 4)
		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlNum_temp, Ntemp_delay, 64)
		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlNum_temp, Ntemp_intg_log2, 4)
		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlDeltaPwrLimit, DeltaPwrLimit, 0x1)
	PHY_REG_LIST_EXECUTE(pi);

	/* Reduce tssi delay for short adc rfseq power down */
	PHY_REG_MOD(pi, LCNPHY, TxPwrCtrlNnum, Ntssi_delay, MIN(255,
		wlc_lcnphy_rfseq_tbl_adc_pwrup(pi)/2 + 25));

	wlc_lcnphy_clear_tx_power_offsets(pi);

	PHY_REG_MOD(pi, LCNPHY, TxPwrCtrlRangeCmd, cckPwrOffset,
		pi_lcn->cckPwrOffset + (pi_lcn->cckPwrIdxCorr<<1));

	PHY_REG_LIST_START
		/*  Set idleTssi to (2^9-1) in OB format = (2^9-1-2^8) = 0xff in 2C format */
		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlIdleTssi, rawTssiOffsetBinFormat, 1)
		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlIdleTssi, idleTssi0, 0xff)
		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlIdleTssi1, idleTssi1, 0xff)
	PHY_REG_LIST_EXECUTE(pi);

	/* Set TSSI sel */
	if (EPA(pi_lcn))
		tssi_sel = 0x1;
	else {
		if (CHSPEC_IS2G(pi->radio_chanspec))
		tssi_sel = 0xe;
		else
			tssi_sel = 0xc;
	}
	if (RADIOID(pi->pubpi.radioid) == BCM2064_ID) {
		phy_utils_mod_radioreg(pi, RADIO_2064_REG028, 0x1e, tssi_sel << 1);
		phy_utils_mod_radioreg(pi, RADIO_2064_REG03A, 0x1, 1);
		phy_utils_mod_radioreg(pi, RADIO_2064_REG11A, 0x8, 1 << 3);
	} else {
		phy_utils_mod_radioreg(pi, RADIO_2064_REG028, 0xf, tssi_sel);
		phy_utils_mod_radioreg(pi, RADIO_2064_REG086, 0x4, 0x4);
	}

	/* disable iqlocal */
	phy_utils_write_radioreg(pi, RADIO_2064_REG025, 0xc);
	/* sel g tssi */
	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		if (RADIOID(pi->pubpi.radioid) == BCM2064_ID)
			phy_utils_mod_radioreg(pi, RADIO_2064_REG03A, 0x2, 1 << 1);
		else
			phy_utils_mod_radioreg(pi, RADIO_2064_REG03A, 0x1, 1);
	} else {
		if (RADIOID(pi->pubpi.radioid) == BCM2064_ID)
			phy_utils_mod_radioreg(pi, RADIO_2064_REG03A, 0x2, 0 << 1);
		else
			phy_utils_mod_radioreg(pi, RADIO_2064_REG03A, 0x1, 1);
	}
/* pa pwrup */

	PHY_REG_LIST_START
		/* amux sel port */
		RADIO_REG_MOD_ENTRY(RADIO_2064_REG005, 0x8, 1 << 3)

		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlRfCtrlOverride0, amuxSelPortOverride, 0)
		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlRfCtrlOverride0, amuxSelPortOverrideVal, 2)
	PHY_REG_LIST_EXECUTE(pi);

	/* no need for tssi setup for 4313A0 */
	if (!wlc_lcnphy_tempsense_based_pwr_ctrl_enabled(pi)) {
		PHY_REG_MOD2(pi, LCNPHY, TxPwrCtrlRfCtrlOverride0, amuxSelPortOverride,
			amuxSelPortOverrideVal, 0, 2);
	}

	rfseq = wlc_lcnphy_rfseq_tbl_adc_pwrup(pi);
	tab.tbl_id = LCNPHY_TBL_ID_RFSEQ;
	tab.tbl_width = 16;	/* 12 bit wide	*/
	tab.tbl_ptr = &rfseq;
	tab.tbl_len = 1;
	tab.tbl_offset = 6;
	wlc_lcnphy_write_table(pi,  &tab);

	PHY_REG_LIST_START
		/* enable Rx Buf */
		PHY_REG_MOD_ENTRY(LCNPHY, rfoverride4, lpf_buf_pwrup_ovr, 1)
		PHY_REG_MOD_ENTRY(LCNPHY, rfoverride4val, lpf_buf_pwrup_ovr_val, 1)
		/* swap iq */
		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlCmd, txPwrCtrl_swap_iq, 1)
		/* increase envelope detector gain */
		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlRfCtrlOverride0, paCtrlTssiOverride, 1)
		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlRfCtrlOverride0, paCtrlTssiOverrideVal, 0)
	PHY_REG_LIST_EXECUTE(pi);

	if (RADIOID(pi->pubpi.radioid) == BCM2064_ID) {
		/* env det bias current */
		phy_utils_mod_radioreg(pi, RADIO_2064_REG035, 0xff, 0x0);
		phy_utils_mod_radioreg(pi, RADIO_2064_REG036, 0x3, 0x0);
	} else {
		PHY_REG_LIST_START
			RADIO_REG_MOD_ENTRY(RADIO_2064_REG035, 0xff, 0x0)
			RADIO_REG_MOD_ENTRY(RADIO_2064_REG036, 0x3, 0x0)
			RADIO_REG_MOD_ENTRY(RADIO_2064_REG0C5, 0xff, 0x0)
			RADIO_REG_MOD_ENTRY(RADIO_2064_REG0C6, 0x3, 0x0)
		PHY_REG_LIST_EXECUTE(pi);
		if (CHSPEC_IS5G(pi->radio_chanspec))
			phy_utils_mod_radioreg(pi, RADIO_2064_REG0CA, 0x2, 0x2);
		else
			phy_utils_mod_radioreg(pi, RADIO_2064_REG0CA, 0x2, 0x0);
	}
	phy_utils_mod_radioreg(pi, RADIO_2064_REG11A, 0x8, 0x8);

	if (LCNREV_GE(pi->pubpi.phy_rev, 2))
		PHY_REG_MOD(pi, LCNPHY, TxPwrCtrlCmd, invertTssiSamples, 1);

	wlc_lcnphy_pwrctrl_rssiparams(pi);
}

void
wlc_lcnphy_tx_pwr_update_npt(phy_info_t *pi)
{
	uint16 tx_cnt, tx_total, npt;
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;

	if (LCNPHY_TX_PWR_CTRL_HW != wlc_lcnphy_get_tx_pwr_ctrl((pi)))
		return;

	tx_total = PHY_TOTAL_TX_FRAMES(pi);
	tx_cnt = tx_total - pi_lcn->lcnphy_tssi_tx_cnt;
	npt = wlc_lcnphy_get_tx_pwr_npt(pi);

	if (tx_cnt > (1 << npt)) {
		/* Reset frame counter */
		pi_lcn->lcnphy_tssi_tx_cnt = tx_total;

		/* Set new NPT */
		if (npt < pi_lcn->tssi_max_npt) {
			npt++;
			wlc_lcnphy_set_tx_pwr_npt(pi, npt);
		}

		/* Update cached power index & NPT */
		pi_lcn->lcnphy_tssi_idx = wlc_lcnphy_get_current_tx_pwr_idx(pi);
		if (CHSPEC_IS2G(pi->radio_chanspec))
			pi_lcn->init_txpwrindex_2g = (uint8) pi_lcn->lcnphy_tssi_idx;
		else
			pi_lcn->init_txpwrindex_5g = (uint8) pi_lcn->lcnphy_tssi_idx;
		pi_lcn->lcnphy_tssi_npt = npt;

		/* Also update starting index as an additional safeguard */
		wlc_lcnphy_set_start_tx_pwr_idx(pi, pi_lcn->lcnphy_tssi_idx);

		PHY_INFORM(("wl%d: %s: Index: %d, NPT: %d, TxCount: %d\n",
			pi->sh->unit, __FUNCTION__, pi_lcn->lcnphy_tssi_idx, npt, tx_cnt));
	}
}

int32
wlc_lcnphy_tssi2dbm(int32 tssi, int32 a1, int32 b0, int32 b1)
{
	int32 a, b, p;
	/* On lcnphy, estPwrLuts0/1 table entries are in S6.3 format */
	a = 32768 + (a1 * tssi);
	b = (1024 * b0) + (64 * b1 * tssi);
	p = ((2 * b) + a) / (2 * a);

	return p;
}

static void
wlc_lcnphy_txpower_reset_npt(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	pi_lcn->lcnphy_tssi_npt = LCNPHY_TX_PWR_CTRL_START_NPT;
}

extern void
wlc_lcnphy_modify_max_txpower(phy_info_t *pi, ppr_t *maxtxpwr)
{
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	if (!EPA(pi_lcn))
		ppr_minus_cmn_val(maxtxpwr, 4);
	else
		ppr_minus_cmn_val(maxtxpwr, 6);
}

extern void
wlc_lcnphy_modify_rate_power_offsets(phy_info_t *pi)
{
	uint32 rate_table[WL_RATESET_SZ_DSSS];
	ppr_dsss_rateset_t dsss;
	uint32 cck_rate_offset_iPA[WL_RATESET_SZ_DSSS] = {14, 14, 14, 14};
	uint32 cck_rate_offset_ePA[WL_RATESET_SZ_DSSS] = {6, 6, 6, 6};
	uint32 *cck_rate_offset_ptr;

	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	int i;

	if (!EPA(pi_lcn))
		cck_rate_offset_ptr = cck_rate_offset_iPA;
	else
		cck_rate_offset_ptr = cck_rate_offset_ePA;

	ppr_get_dsss(pi->tx_power_offset, WL_TX_BW_20, WL_TX_CHAINS_1, &dsss);

	for (i = 0; i < (int)ARRAYSIZE(rate_table); i++) {
		dsss.pwr[i] -= (int32)cck_rate_offset_ptr[i];
	}

	ppr_set_dsss(pi->tx_power_offset, WL_TX_BW_20, WL_TX_CHAINS_1, &dsss);
}

static int32
wlc_lcnphy_get_tssi_pwr(phy_info_t *pi,
	int32 a1, int32 b0, int32 b1,
	uint8 maxlimit, uint8 offset)
{
	int32 tssi, pwr, prev_pwr;
	int32 lcnphy_tssi_pwr_limit;
	uint8 tssi_ladder_cnt = 0;

	if (maxlimit) {
		prev_pwr = 0x7fffffff;
		lcnphy_tssi_pwr_limit = 0x7fffffff;
		for (tssi = 0; tssi < 128; tssi++) {
			pwr = wlc_lcnphy_tssi2dbm(tssi, a1, b0, b1);
			if (pwr < prev_pwr) {
				prev_pwr = pwr;
				if (++tssi_ladder_cnt == offset) {
					lcnphy_tssi_pwr_limit = pwr;
					break;
				}
			}
		}
	} else {
		prev_pwr = 0xffffffff;
		lcnphy_tssi_pwr_limit = 0xffffffff;
		for (tssi = 127; tssi >= 0; tssi--) {
			pwr = wlc_lcnphy_tssi2dbm(tssi, a1, b0, b1);
			if (pwr > prev_pwr) {
				prev_pwr = pwr;
				if (++tssi_ladder_cnt == offset) {
					lcnphy_tssi_pwr_limit = pwr;
					break;
				}
			}
		}
	}

	return lcnphy_tssi_pwr_limit;
}

static void
wlc_lcnphy_get_tssi_offset(phy_info_t *pi, uint8 *offset_maxpwr, uint8 *offset_minpwr)
{
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;

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
		*offset_maxpwr = 0;
		*offset_minpwr = 0;
		ASSERT(FALSE);
		break;
	}
}

static void
wlc_lcnphy_set_tssi_pwr_limit(phy_info_t *pi, uint8 mode)
{
	int32 a1 = 0, b0 = 0, b1 = 0;
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	int32 lcnphy_tssi_maxpwr_limit = 0x7fffffff;
	int32 lcnphy_tssi_minpwr_limit = 0xffffffff;
	uint8 tssi_ladder_offset_maxpwr, tssi_ladder_offset_minpwr;

#if TWO_POWER_RANGE_TXPWR_CTRL
	int32 lcnphy_tssi_maxpwr_limit_second = 0x7fffffff;
	int32 lcnphy_tssi_minpwr_limit_second = 0xffffffff;
#endif // endif

	wlc_phy_get_paparams_for_band(pi, &a1, &b0, &b1);
	wlc_lcnphy_get_tssi_offset(pi, &tssi_ladder_offset_maxpwr, &tssi_ladder_offset_minpwr);
	lcnphy_tssi_maxpwr_limit =
		wlc_lcnphy_get_tssi_pwr(pi, a1, b0, b1, 1, tssi_ladder_offset_maxpwr);
	lcnphy_tssi_minpwr_limit =
		wlc_lcnphy_get_tssi_pwr(pi, a1, b0, b1, 0, tssi_ladder_offset_minpwr);

#if TWO_POWER_RANGE_TXPWR_CTRL
	if (pi_lcn->lcnphy_twopwr_txpwrctrl_en) {
		b0 = pi->txpa_2g_lo[0];
		b1 = pi->txpa_2g_lo[1];
		a1 = pi->txpa_2g_lo[2];
		lcnphy_tssi_maxpwr_limit_second = wlc_lcnphy_get_tssi_pwr(pi, a1, b0, b1, 1,
			tssi_ladder_offset_maxpwr);
		lcnphy_tssi_minpwr_limit_second = wlc_lcnphy_get_tssi_pwr(pi, a1, b0, b1, 0,
			tssi_ladder_offset_minpwr);

		lcnphy_tssi_maxpwr_limit =
			MAX(lcnphy_tssi_maxpwr_limit, lcnphy_tssi_maxpwr_limit_second);
		lcnphy_tssi_minpwr_limit =
			MIN(lcnphy_tssi_minpwr_limit, lcnphy_tssi_minpwr_limit_second);
	}
#endif /* #if TWO_POWER_RANGE_TXPWR_CTRL */

	if ((mode == LCNPHY_TSSI_SET_MAX_LIMIT) || (mode == LCNPHY_TSSI_SET_MIN_MAX_LIMIT))
		pi_lcn->tssi_maxpwr_limit = lcnphy_tssi_maxpwr_limit >> 1;

	if ((mode == LCNPHY_TSSI_SET_MIN_LIMIT) || (mode == LCNPHY_TSSI_SET_MIN_MAX_LIMIT))
		pi_lcn->tssi_minpwr_limit = lcnphy_tssi_minpwr_limit >> 1;
}

void
wlc_lcnphy_tx_pwr_limit_check(phy_info_t *pi)
{
	int8 txpwrindex;
	int32 margin_qdBm = 4;
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	int32 tssi_maxpwr_limit, tssi_minpwr_limit;
	uint8 wait_for_pwrctrl = 5;
	int32 a1 = 0, b0 = 0, b1 = 0;
	uint16 avgTssi_2C, idleTssi0_2C, idleTssi0_OB, avgTssi_OB, adjTssi;

	if (!pi_lcn->dynamic_pwr_limit_en)
		return;

	txpwrindex = wlc_lcnphy_get_current_tx_pwr_idx(pi);

	if ((txpwrindex != 0) && (txpwrindex != 127))
		return;

	wlc_phy_get_paparams_for_band(pi, &a1, &b0, &b1);
	avgTssi_2C = PHY_REG_READ(pi, LCNPHY, TxPwrCtrlStatusNew4, avgTssi);
	idleTssi0_2C = PHY_REG_READ(pi, LCNPHY, TxPwrCtrlIdleTssi, idleTssi0);

	if (idleTssi0_2C >= 256)
		idleTssi0_OB = idleTssi0_2C - 256;
	else
		idleTssi0_OB = idleTssi0_2C + 256;

	if (avgTssi_2C >= 256)
		avgTssi_OB = avgTssi_2C - 256;
	else
		avgTssi_OB = avgTssi_2C + 256;

	adjTssi = (avgTssi_OB + (511-idleTssi0_OB))>>2;

	if (txpwrindex == 0) {
		pi_lcn->idx0cnt++;
		if (pi_lcn->idx0cnt < wait_for_pwrctrl)
			return;
		pi_lcn->idx0cnt = 0;
		tssi_maxpwr_limit = (wlc_lcnphy_tssi2dbm(adjTssi, a1, b0, b1) >> 1) - margin_qdBm;
		if (tssi_maxpwr_limit >= pi_lcn->tssi_minpwr_limit)
			pi_lcn->tssi_maxpwr_limit = tssi_maxpwr_limit;
		else
			pi_lcn->tssi_maxpwr_limit = pi_lcn->tssi_minpwr_limit;
		wlc_lcnphy_set_target_tx_pwr(pi, pi_lcn->tssi_maxpwr_limit);

	} else if (txpwrindex == 127) {
		pi_lcn->idx127cnt++;
		if (pi_lcn->idx127cnt < wait_for_pwrctrl)
			return;
		pi_lcn->idx127cnt = 0;
		tssi_minpwr_limit = (wlc_lcnphy_tssi2dbm(adjTssi, a1, b0, b1) >> 1) + margin_qdBm;
		if (tssi_minpwr_limit <= pi_lcn->tssi_maxpwr_limit)
			pi_lcn->tssi_minpwr_limit = tssi_minpwr_limit;
		else
			pi_lcn->tssi_minpwr_limit = pi_lcn->tssi_maxpwr_limit;
		wlc_lcnphy_set_target_tx_pwr(pi, pi_lcn->tssi_minpwr_limit);
	}
}

void
wlc_lcnphy_txpower_recalc_target(phy_info_t *pi)
{
	phytbl_info_t tab;
	ppr_dsss_rateset_t dsss_limits;
	ppr_ofdm_rateset_t ofdm_limits;
	ppr_ht_mcs_rateset_t mcs_limits;
	uint32 rate_table[WL_RATESET_SZ_DSSS + WL_RATESET_SZ_OFDM + WL_RATESET_SZ_HT_MCS];
	uint j;
	uint i;
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
#if TWO_POWER_RANGE_TXPWR_CTRL
	int8 pmin_range;
	int8 diff;
#endif // endif

	if (wlc_lcnphy_tempsense_based_pwr_ctrl_enabled(pi))
		return;

	/* Adjust rate based power offset */
	if (pi->tx_power_offset == NULL)
		return;

	ppr_get_dsss(pi->tx_power_offset, WL_TX_BW_20, WL_TX_CHAINS_1, &dsss_limits);
	ppr_get_ofdm(pi->tx_power_offset, WL_TX_BW_20, WL_TX_MODE_NONE, WL_TX_CHAINS_1,
		&ofdm_limits);
	ppr_get_ht_mcs(pi->tx_power_offset, WL_TX_BW_20, WL_TX_NSS_1,
		WL_TX_MODE_NONE, WL_TX_CHAINS_1, &mcs_limits);

	j = 0;
	for (i = 0; i < WL_RATESET_SZ_DSSS; i++, j++) {
		rate_table[j] = (uint32)((int32)(-dsss_limits.pwr[i]));
		PHY_TMP((" Rate %d, offset %d\n", j, rate_table[j]));
	}

	for (i = 0; i < WL_RATESET_SZ_OFDM; i++, j++) {
		rate_table[j] = (uint32)((int32)(-ofdm_limits.pwr[i]));
		PHY_TMP((" Rate %d, offset %d\n", j, rate_table[j]));
	}

	for (i = 0; i < WL_RATESET_SZ_HT_MCS; i++, j++) {
		rate_table[j] = (uint32)((int32)(-mcs_limits.pwr[i]));
		PHY_TMP((" Rate %d, offset %d\n", j, rate_table[j]));
	}

	if (!pi_lcn->lcnphy_uses_rate_offset_table) {
		/* Preset txPwrCtrltbl */
		tab.tbl_id = LCNPHY_TBL_ID_TXPWRCTL;
		tab.tbl_width = 32;	/* 32 bit wide	*/
		tab.tbl_len = ARRAYSIZE(rate_table); /* # values   */
		tab.tbl_ptr = rate_table; /* ptr to buf */
		tab.tbl_offset = LCNPHY_TX_PWR_CTRL_RATE_OFFSET;
		wlc_lcnphy_write_table(pi, &tab);
	}

#ifdef LP_P2P_SOFTAP
	/* Update the MACAddr LUT which is cleared when doing recal */
	if (pi_lcn->pwr_offset_val)
		wlc_lcnphy_lpc_write_maclut(pi);
#endif /* LP_P2P_SOFTAP */

#ifdef WL_LPC
	/* update the MACAddr LUT also */
	if (pi->lpc_algo)
	wlc_lcnphy_lpc_write_maclut((wlc_phy_t *)pi);
#endif /* WL_LPC */

	/* Set new target power */
	wlc_lcnphy_set_target_tx_pwr(pi, wlc_phy_txpower_get_target_min((wlc_phy_t*)pi));
#if TWO_POWER_RANGE_TXPWR_CTRL
		if (pi_lcn->lcnphy_twopwr_txpwrctrl_en) {
			uint8 tx_power_min = wlc_phy_txpower_get_target_min((wlc_phy_t*)pi);

			/* RTL calculates intended TXPwr as targetPwr + RateOffset */
			/* While actual intended power is targetPower - RateOffset */
			ASSERT(pi_lcn->cckPwrIdxCorr <= 0);
			PHY_REG_MOD(pi, LCNPHY, TxPwrCtrlPwrRange2,
				pwrMax_range2, tx_power_min - pi_lcn->cckPwrIdxCorr);
			diff = pi_lcn->pmax - tx_power_min;
			pmin_range = tx_power_min - diff - pi_lcn->cckPwrIdxCorr;
			PHY_REG_MOD(pi, LCNPHY, TxPwrCtrlPwrRange2, pwrMin_range2, pmin_range);
		}
#endif /* #if TWO_POWER_RANGE_TXPWR_CTRL */

	wlc_lcnphy_txpower_reset_npt(pi);
}

static void
wlc_lcnphy_set_tx_pwr_soft_ctrl(phy_info_t *pi, int8 indx)
{
	uint32 cck_rate_offset[WL_RATESET_SZ_DSSS] = {6, 6, 6, 6};
	uint32 ofdm_offset;
	int i;
	uint16 index2;
	phytbl_info_t tab;
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;

	if (wlc_lcnphy_tssi_based_pwr_ctrl_enabled(pi) &&
		(!pi_lcn->openlp_pwrctrl))
		return;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	PHY_REG_LIST_START
		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlCmd, hwtxPwrCtrl_en, 0x1)
		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlCmd, hwtxPwrCtrl_en, 0x0)
		PHY_REG_OR_ENTRY(LCNPHY, sslpnCalibClkEnCtrl, 0x0040)
	PHY_REG_LIST_EXECUTE(pi);

	tab.tbl_id = LCNPHY_TBL_ID_TXPWRCTL;
	tab.tbl_width = 32;     /* 32 bit wide  */
	tab.tbl_len = 4;        /* # values   */
	tab.tbl_ptr = cck_rate_offset; /* ptr to buf */
	tab.tbl_offset = LCNPHY_TX_PWR_CTRL_RATE_OFFSET;
	pi_lcn->lcnphy_uses_rate_offset_table = TRUE;
	wlc_lcnphy_write_table(pi, &tab);

	ofdm_offset = 0;
	tab.tbl_len = 1;
	tab.tbl_ptr = &ofdm_offset;
	for (i = 836; i < 862; i++) {
		tab.tbl_offset = i;
		wlc_lcnphy_write_table(pi, &tab);
	}

	PHY_REG_LIST_START
		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlCmd, txPwrCtrl_en, 0x1)
		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlCmd, hwtxPwrCtrl_en, 0x1)
		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlCmd, use_txPwrCtrlCoefs, 0x1)
		PHY_REG_MOD_ENTRY(LCNPHY, rfoverride2, txgainctrl_ovr, 0)
		PHY_REG_MOD_ENTRY(LCNPHY, AfeCtrlOvr, dacattctrl_ovr, 0)
		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlBaseIndex, loadBaseIndex, 1)
	PHY_REG_LIST_EXECUTE(pi);

	index2 = (uint16)(indx * 2);
	PHY_REG_MOD(pi, LCNPHY, TxPwrCtrlBaseIndex, uC_baseIndex0, index2);
	PHY_REG_MOD(pi, LCNPHY, papd_control2, papd_analog_gain_ovr, 0);
}

static int8
wlc_lcnphy_openloop_txpwrctrl(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	int8 idx;
	int8 ref_idx = 0;
	uint8 channel = CHSPEC_CHANNEL(pi->radio_chanspec); /* see wlioctl.h */

	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		ref_idx = pi_lcn->openlp_gainidx_b[channel-1];
	} else {
		switch (channel) {
			case 36: ref_idx = pi_lcn->openlp_gainidx_a36; break;
			case 40: ref_idx = pi_lcn->openlp_gainidx_a40; break;
			case 44: ref_idx = pi_lcn->openlp_gainidx_a44; break;
			case 48: ref_idx = pi_lcn->openlp_gainidx_a48; break;
			case 52: ref_idx = pi_lcn->openlp_gainidx_a52; break;
			case 56: ref_idx = pi_lcn->openlp_gainidx_a56; break;
			case 60: ref_idx = pi_lcn->openlp_gainidx_a60; break;
			case 64: ref_idx = pi_lcn->openlp_gainidx_a64; break;
			case 100: ref_idx = pi_lcn->openlp_gainidx_a100; break;
			case 104: ref_idx = pi_lcn->openlp_gainidx_a104; break;
			case 108: ref_idx = pi_lcn->openlp_gainidx_a108; break;
			case 112: ref_idx = pi_lcn->openlp_gainidx_a112; break;
			case 116: ref_idx = pi_lcn->openlp_gainidx_a116; break;
			case 120: ref_idx = pi_lcn->openlp_gainidx_a120; break;
			case 124: ref_idx = pi_lcn->openlp_gainidx_a124; break;
			case 128: ref_idx = pi_lcn->openlp_gainidx_a128; break;
			case 132: ref_idx = pi_lcn->openlp_gainidx_a132; break;
			case 136: ref_idx = pi_lcn->openlp_gainidx_a136; break;
			case 140: ref_idx = pi_lcn->openlp_gainidx_a140; break;
			case 149: ref_idx = pi_lcn->openlp_gainidx_a149; break;
			case 153: ref_idx = pi_lcn->openlp_gainidx_a153; break;
			case 157: ref_idx = pi_lcn->openlp_gainidx_a157; break;
			case 161: ref_idx = pi_lcn->openlp_gainidx_a161; break;
			case 165: ref_idx = pi_lcn->openlp_gainidx_a165; break;
		}
	}

	idx = ref_idx + (pi_lcn->openlp_refpwrqdB - pi->openlp_tx_power_min);
	return idx;
}

/* In the following function the calc_cap_index is used to handle the case
 * of using mode1 when using a tssi based power control. The same flag is also
 * used for determine if bandedge correction is to be applied. For functions
 * called in tssi based power control this flag is set to zero so that the
 * function behaves in the original manner
 */
static int8
wlc_lcnphy_tempcompensated_txpwrctrl(phy_info_t *pi, bool calc_cap_index)
{
	int8 indx, delta_brd, delta_temp, new_index, tempcorrx;
	int16 manp, meas_temp, temp_diff;
	bool neg = 0;
	bool skip_chpower_meas = 0;
	uint16 temp, chMeasPower = 0;
	uint8 channel = CHSPEC_CHANNEL(pi->radio_chanspec); /* see wlioctl.h */
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	indx = FIXED_TXPWR; /* for 4313A0 FEM boards */

	if (NORADIO_ENAB(pi->pubpi))
		return indx;

	/* This condition is to handle the case when we do not want any
	 * capping to be implemented
	 */
	if ((pi_lcn->lcnphy_measPower == 0) && (pi_lcn->lcnphy_measPower1 == 0) &&
	    (pi_lcn->lcnphy_measPower2 == 0)) {
		return 0;
	}
	else if ((pi_lcn->lcnphy_measPower1 == 0) && (pi_lcn->lcnphy_measPower2 == 0) &&
	         wlc_lcnphy_tssi_based_pwr_ctrl_enabled(pi)) {
		skip_chpower_meas = TRUE;
	}
	/* This condition is to handle the case that in older board we will
	 * not have meas power1 and meas power 2 values
	 */
	else if ((pi_lcn->lcnphy_measPower1 == 0) && (pi_lcn->lcnphy_measPower2 == 0) &&
	         wlc_lcnphy_tempsense_based_pwr_ctrl_enabled(pi)) {
		skip_chpower_meas = TRUE;
	}

	if (pi_lcn->lcnphy_tempsense_slope == 0) {
		if (wlc_lcnphy_tssi_based_pwr_ctrl_enabled(pi)) {
			/* To support older revision of TSSI (<= P209) based brds
			 * which are having tempsense_slope = 0
			 */
			return 0;
		} else {
			PHY_ERROR(("wl%d: %s: Correct nvram.txt is not taken\n",
			pi->sh->unit, __FUNCTION__));
			return indx;
		}

	}
	temp = (uint16)wlc_lcnphy_tempsense(pi, calc_cap_index);
	meas_temp = LCNPHY_TEMPSENSE(temp);
	if (skip_chpower_meas == FALSE) {
		if (channel == 1)
			chMeasPower = pi_lcn->lcnphy_measPower;
		else if (channel > 1 && channel < 7)
			chMeasPower = pi_lcn->lcnphy_measPower + wlc_lcnphy_qdiv_roundup
				((uint32)((pi_lcn->lcnphy_measPower1 -
					pi_lcn->lcnphy_measPower) * channel), 6, 0);
		else if (channel == 7)
			chMeasPower = pi_lcn->lcnphy_measPower1;
		else if (channel > 7 && channel < 13)
			chMeasPower = pi_lcn->lcnphy_measPower1 + wlc_lcnphy_qdiv_roundup
				((uint32)((pi_lcn->lcnphy_measPower2 -
					pi_lcn->lcnphy_measPower1) * channel), 6, 0);
		else if (channel == 13)
			chMeasPower = pi_lcn->lcnphy_measPower2;
		else if (channel == 14)
			chMeasPower = pi_lcn->lcnphy_measPower2 + wlc_lcnphy_qdiv_roundup
					((uint32)((pi_lcn->lcnphy_measPower2 -
						pi_lcn->lcnphy_measPower1) * channel), 6, 0);
	} else {
		chMeasPower = pi_lcn->lcnphy_measPower;
	}

	/* delta_brd = round((Precorded - Ptarget)*4) */
	/* for country specific regulatory limits */
	/* this check had to be put as on driver init, pi->tx_power_min comes up as 0 */
	if (wlc_phy_txpower_get_target_min((wlc_phy_t*)pi) != 0) {
		delta_brd = (chMeasPower - wlc_phy_txpower_get_target_min((wlc_phy_t*)pi));
	} else {
		delta_brd = 0;
	}
	/* delta_temp = round((Trecorded - Tmeas)/Tslope*0.075*4) */
	/* tempsense_slope is multiplied with a scalar of 64 */
	manp = LCNPHY_TEMPSENSE(pi_lcn->lcnphy_rawtempsense);
	temp_diff = manp - meas_temp;
	if (temp_diff < 0) {
		/* delta_temp is negative */
		neg = 1;
		/* taking only the magnitude */
		temp_diff = -temp_diff;
	}

	/* 192 is coming from 0.075 * 4 * 64 * 10 */
	delta_temp = (int8)wlc_lcnphy_qdiv_roundup((uint32)(temp_diff*192),
		(uint32)(pi_lcn->lcnphy_tempsense_slope*10), 0);
	if (neg)
		delta_temp = -delta_temp;
	/* functionality of tempsense_option OTP param has been changed */
	/* value of 3 is for 4313B0 tssi based pwrctrl			*/
	if (pi_lcn->lcnphy_tempsense_option == 3 && LCNREV_IS(pi->pubpi.phy_rev, 0))
		delta_temp = 0;

	if (pi_lcn->lcnphy_tempcorrx > 31)
		tempcorrx = (int8)(pi_lcn->lcnphy_tempcorrx - 64);
	else
		tempcorrx = (int8)pi_lcn->lcnphy_tempcorrx;

	if (EPA(pi_lcn)) {
		tempcorrx = 4; /* temporarily For B0 */
	} else {
		/* The tempcorrx needs to be tuned based on the channel selected. */
		/* The assumption is that the measpower will be calibrated on Ch10 */
		/* Based on this, the rest of the channels are applied a correction factor */
		if (channel <= 5)
			tempcorrx = tempcorrx - 6;
		else if (channel > 5 && channel <= 8)
			tempcorrx = tempcorrx - 4;
		else if (channel > 8 && channel < 11)
			tempcorrx = tempcorrx - 2;
		else
			tempcorrx = tempcorrx + 0;
	}

	if (calc_cap_index) {
		/* While capping index, do not apply band edge correction */
		new_index = indx + delta_brd + delta_temp;
	} else
		new_index = indx + delta_brd + delta_temp - pi_lcn->lcnphy_bandedge_corr;

	new_index += tempcorrx;
	/* B0: ceiling the tx pwr index to 127 if it is out of bound */
	if (LCNREV_IS(pi->pubpi.phy_rev, 1))
		indx = 127;
	if (new_index < 0 || new_index > 126) {
		PHY_ERROR(("wl%d: %s: Tempcompensated tx index out of bound\n",
			pi->sh->unit, __FUNCTION__));
		return indx;
	}
	PHY_INFORM(("wl%d: %s Settled to tx pwr index: %d\n",
		pi->sh->unit, __FUNCTION__, new_index));
	return new_index;
}

static uint16
wlc_lcnphy_set_tx_pwr_ctrl_mode(phy_info_t *pi, uint16 mode)
{
	/* the abstraction is for wlc_phy_cmn.c routines, tempsense based and tssi based */
	/* is LCNPHY_TX_PWR_CTRL_HW . internally, we route it through the correct path , */
	/* LCNPHY_TX_PWR_CTRL_TEMPBASED = 0xE001 so that TxPwrCtrlCmd gets set correctly */

	uint16 current_mode = mode;
	if (CHIPID(pi->sh->chip) == BCM4313_CHIP_ID) {
		if (wlc_lcnphy_tempsense_based_pwr_ctrl_enabled(pi) &&
			mode == LCNPHY_TX_PWR_CTRL_HW)
				current_mode = LCNPHY_TX_PWR_CTRL_TEMPBASED;
		if (wlc_lcnphy_tssi_based_pwr_ctrl_enabled(pi) &&
			mode == LCNPHY_TX_PWR_CTRL_TEMPBASED)
				current_mode = LCNPHY_TX_PWR_CTRL_HW;
	}

	return current_mode;
}

void
wlc_lcnphy_set_tx_pwr_ctrl(phy_info_t *pi, uint16 mode)
{
	uint16 old_mode = wlc_lcnphy_get_tx_pwr_ctrl(pi);
	int8 indx;
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;

	ASSERT(
		(LCNPHY_TX_PWR_CTRL_OFF == mode) ||
		(LCNPHY_TX_PWR_CTRL_SW == mode) ||
		(LCNPHY_TX_PWR_CTRL_HW == mode) ||
		(LCNPHY_TX_PWR_CTRL_TEMPBASED == mode));

	mode = wlc_lcnphy_set_tx_pwr_ctrl_mode(pi, mode);
	old_mode = wlc_lcnphy_set_tx_pwr_ctrl_mode(pi, old_mode);

#if defined(WLTEST)
	/* Override power index if NVRAM says so */
	if ((pi_lcn->txpwrindex_nvram || pi_lcn->txpwrindex5g_nvram) &&
		(LCNPHY_TX_PWR_CTRL_HW == mode)) {
		if (CHSPEC_IS2G(pi->radio_chanspec))
			wlc_lcnphy_force_pwr_index(pi, pi_lcn->txpwrindex_nvram);
		else
			wlc_lcnphy_force_pwr_index(pi, pi_lcn->txpwrindex5g_nvram);
		mode = LCNPHY_TX_PWR_CTRL_OFF;
	}
#endif // endif

	/* Setting txfront end clock also along with hwpwr control */
	PHY_REG_MOD(pi, LCNPHY, sslpnCalibClkEnCtrl, txFrontEndCalibClkEn,
		(LCNPHY_TX_PWR_CTRL_HW == mode) ? 1 : 0);
	/* Feed back RF power level to PAPD block */
	PHY_REG_MOD(pi, LCNPHY, papd_control2, papd_analog_gain_ovr,
		(LCNPHY_TX_PWR_CTRL_HW == mode) ? 0 : 1);
	/* for phy rev2 and above, we need to set this register */
	if (LCNREV_GE(pi->pubpi.phy_rev, 2))
		PHY_REG_MOD(pi, LCNPHY, BBmultCoeffSel,  use_txPwrCtrlCoeffforBBMult,
			(LCNPHY_TX_PWR_CTRL_HW == mode) ? 1 : 0);

	if (old_mode != mode) {
		if (LCNPHY_TX_PWR_CTRL_HW == old_mode) {
			/* Clear out all power offsets */
			wlc_lcnphy_clear_tx_power_offsets(pi);
		}
		if (LCNPHY_TX_PWR_CTRL_HW == mode) {
			/* Recalculate target power to restore power offsets */
			wlc_lcnphy_txpower_recalc_target(pi);
			/* Set starting index & NPT to best known values for that target */
			wlc_lcnphy_set_start_tx_pwr_idx(pi, pi_lcn->lcnphy_tssi_idx);
			wlc_lcnphy_set_tx_pwr_npt(pi, pi_lcn->lcnphy_tssi_npt);
			phy_utils_mod_radioreg(pi, RADIO_2064_REG11F, 0x4, 0);
			/* Reset frame counter for NPT calculations */
			pi_lcn->lcnphy_tssi_tx_cnt = PHY_TOTAL_TX_FRAMES(pi);
			/* Disable any gain overrides */
			wlc_lcnphy_disable_tx_gain_override(pi);
			pi_lcn->lcnphy_tx_power_idx_override = -1;
			PHY_REG_MOD(pi, LCNPHY, TxPwrCtrlBaseIndex, loadBaseIndex, 0);
		}
		else
			wlc_lcnphy_enable_tx_gain_override(pi);

		/* Set requested tx power control mode */
		phy_utils_mod_phyreg(pi, LCNPHY_TxPwrCtrlCmd,
			(LCNPHY_TxPwrCtrlCmd_txPwrCtrl_en_MASK |
			LCNPHY_TxPwrCtrlCmd_hwtxPwrCtrl_en_MASK |
			LCNPHY_TxPwrCtrlCmd_use_txPwrCtrlCoefs_MASK),
			mode);

		if (LCNPHY_TX_PWR_CTRL_HW == mode)
			PHY_REG_MOD(pi, LCNPHY, TxPwrCtrlCmd,
			use_txPwrCtrlCoefs,
			!wlc_lcnphy_frmTypBasedIqLoCoeff(pi));

		if (mode == LCNPHY_TX_PWR_CTRL_TEMPBASED) {
			if (CHIPID(pi->sh->chip) == BCM4313_CHIP_ID) {
				indx = wlc_lcnphy_tempcompensated_txpwrctrl(pi, FALSE);

			} else {
				indx = wlc_lcnphy_openloop_txpwrctrl(pi);
			}
			wlc_lcnphy_set_tx_pwr_soft_ctrl(pi, indx);
			pi_lcn->lcnphy_current_index = (int8)
				((phy_utils_read_phyreg(pi, LCNPHY_TxPwrCtrlBaseIndex) & 0xFF)/2);
		}

		if (pi_lcn->papd_num_lut_used > 1)
			PHY_REG_MOD(pi, LCNPHY, PapdDupLutCtrl, gainbasedSelect,
				PHY_REG_READ(pi, LCNPHY, TxPwrCtrlCmd, hwtxPwrCtrl_en));
		else
			PHY_REG_MOD(pi, LCNPHY, PapdDupLutCtrl, gainbasedSelect, 0);

		if (!EPA(pi_lcn) && (CHIPID(pi->sh->chip) == BCM4313_CHIP_ID)) {
			if ((mode == LCNPHY_TX_PWR_CTRL_HW)&& (pi_lcn->lcnphy_calreqd == 1)) {
				pi_lcn->lcnphy_calreqd = 0;
				if (pi_lcn->lcnphy_CalcPapdCapEnable == 0) {
					pi_lcn->lcnphy_capped_index =
						wlc_lcnphy_tempcompensated_txpwrctrl(pi, TRUE);
					wlc_lcnphy_load_txgainwithcappedindex(pi, 1);
				}

			}
		}
		PHY_INFORM(("wl%d: %s: %s \n", pi->sh->unit, __FUNCTION__,
			mode ? ((LCNPHY_TX_PWR_CTRL_HW == mode) ? "Auto" : "Manual") : "Off"));
	}
}

#if defined(BCMDBG)
void
wlc_lcnphy_iovar_cw_tx_pwr_ctrl(phy_info_t *pi, int32 targetpwr, int32 *ret, bool set)
{
	bool suspend;

	if (set) {
		suspend = !(R_REG(pi->sh->osh, &pi->regs->maccontrol) & MCTL_EN_MAC);
		if (!suspend) {
			/* Set non-zero duration for CTS-to-self */
			wlapi_bmac_write_shm(pi->sh->physhim, M_CTS_DURATION, 10000);
			wlapi_suspend_mac_and_wait(pi->sh->physhim);
		}
		/* Turn off automatic power control */
		wlc_lcnphy_set_tx_pwr_ctrl(pi, LCNPHY_TX_PWR_CTRL_OFF);

		wlc_lcnphy_cw_tx_pwr_ctrl(pi, targetpwr);

		if (!suspend)
			wlapi_enable_mac(pi->sh->physhim);
	}
	else {
		*ret = pi->u.pi_lcnphy->lcnphy_current_index;
	}
}
#endif // endif

static int32
wlc_lcnphy_cw_tx_pwr_ctrl(phy_info_t *pi, int32 targetpwr)
{
	/* Read tone/spb power with tssi and automatically adjust gain index
	 * to reach target power
	 */
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	uint16 save_TxPwrCtrlRfCtrlOverride0 = 0;
	int32 tone_pwr, step_size;
	int count = 0;
	int tmp_tx_idx;
	targetpwr = targetpwr*16;
	tmp_tx_idx = wlc_lcnphy_get_current_tx_pwr_idx(pi);
	save_TxPwrCtrlRfCtrlOverride0 = phy_utils_read_phyreg(pi, LCNPHY_TxPwrCtrlRfCtrlOverride0);

	if (pi_lcn->pmax != -1) {
		if (targetpwr < pi_lcn->pmax/4)
			PHY_REG_MOD(pi, LCNPHY, TxPwrCtrlRfCtrlOverride0, tssiRangeOverrideVal, 0);
		else
			PHY_REG_MOD(pi, LCNPHY, TxPwrCtrlRfCtrlOverride0, tssiRangeOverrideVal, 1);
	}

	if (pi->phy_tx_tone_freq) {
		/* if tone is already played out with iq cal mode zero then
		 * stop the tone and re-play with iq cal mode 1.
		 */
		wlc_lcnphy_stop_tx_tone(pi);
		OSL_DELAY(5);
		wlc_lcnphy_start_tx_tone(pi, (4000 * 1000), 112, 0);
	} else {
		wlc_lcnphy_start_tx_tone(pi, (4000 * 1000), 112, 0);
	}

	wlc_lcnphy_set_tx_pwr_by_index(pi, tmp_tx_idx);
	while (1) {
		tone_pwr = wlc_lcnphy_tone_power(pi, 1);
		step_size = ((tone_pwr - targetpwr) / 4) - 1;
		tmp_tx_idx +=  step_size;
		if (tmp_tx_idx < 0)
			tmp_tx_idx = 0;
		else if (tmp_tx_idx > 127)
			tmp_tx_idx = 127;
		wlc_lcnphy_set_tx_pwr_by_index(pi, tmp_tx_idx);
		if (((ABS(tone_pwr - targetpwr)) <= 4) || (count >= 10))
			break;
		count++;
	}
	wlc_lcnphy_stop_tx_tone(pi);
	PHY_REG_WRITE(pi, LCNPHY, TxPwrCtrlRfCtrlOverride0,	save_TxPwrCtrlRfCtrlOverride0);
	return (wlc_lcnphy_get_current_tx_pwr_idx(pi));
}

static int32
wlc_lcnphy_tone_power(phy_info_t *pi, bool tssi_setup)
{
	/* Read power in dBm of tone/spb signal using tssi caller is responsible for starting tone
	 * Currently works only for radio version 2064; support for 2066 will be added soon
	 */
	uint16 values_to_save[ARRAYSIZE(tone_power_rf_reg)];
	int32 tone_tssi, idle_tssi, tssi_idx;
	int32 a1, b0, b1, estPower;
	uint16 save_TxPwrCtrlRfCtrlOverride0 = 0;

	/* set path and enable tx_pwr_ctrl clock for tone power measurement */
	if (tssi_setup) {
		/* cache tssirangeoverride */
		save_TxPwrCtrlRfCtrlOverride0 =
		        phy_utils_read_phyreg(pi, LCNPHY_TxPwrCtrlRfCtrlOverride0);
		wlc_lcnphy_tssi_loopback(pi, (uint16 *)&values_to_save);
	}

	/* force vbatTemp measurement */
	PHY_REG_MOD(pi, LCNPHY, TxPwrCtrlRangeCmd, force_vbatTemp, 1);
	OSL_DELAY(100);
	tone_tssi = PHY_REG_READ(pi, LCNPHY, TxPwrCtrlStatusTemp, avgTemp) ^ 0x100;
	idle_tssi = PHY_REG_READ(pi, LCNPHY, TxPwrCtrlIdleTssi, idleTssi0) ^ 0x100;
	tssi_idx = (tone_tssi - idle_tssi + 511) >> 2;
	/*
	Future work: To improve accuracy of tssi2dbm for powers < 10 dBm
	If tssi_idx > val:
	(where "val" corresponds to tssi for 10 dBm power;
	Note: larger the power smaller the tssi register value)
		then :
			set tssi_range=1
			use tssi2 lut for tssi2dbm
			OR
			use paparams from nvram (yet to added) and compute tssi2dbm value
		else:
			continue as below
	*/
	wlc_phy_get_paparams_for_band(pi, &a1, &b0, &b1);
	estPower = wlc_lcnphy_tssi2dbm(tssi_idx, a1, b0, b1) << 1;
	/* restore state of path */
	if (tssi_setup) {
		wlc_lcnphy_tssi_loopback_cleanup(pi, (uint16 *)&values_to_save);
		PHY_REG_WRITE(pi, LCNPHY, TxPwrCtrlRfCtrlOverride0,
			save_TxPwrCtrlRfCtrlOverride0);
	}
	return estPower;
}

static void
wlc_lcnphy_tssi_loopback(phy_info_t *pi, uint16 *values_to_save)
{
	/* Enable/force tssi loopback path,
	 * (especially for use with non-packets, spb etc.)
	 */
	uint8 i;
	uint16 auxpga_vmid = 0;
	int16 auxpga_gain = 0;

	PHY_REG_LIST_START
		PHY_REG_MOD_ENTRY(LCNPHY, AfeCtrlOvr, pwdn_rssi_ovr, 1)
		PHY_REG_MOD_ENTRY(LCNPHY, AfeCtrlOvrVal, pwdn_rssi_ovr_val, 0)
	PHY_REG_LIST_EXECUTE(pi);

	/* save tone power rf register states */
	for (i = 0; i < ARRAYSIZE(tone_power_rf_reg); i++) {
		values_to_save[i] = phy_utils_read_radioreg(pi, tone_power_rf_reg[i]);
	}

	PHY_REG_LIST_START
		RADIO_REG_WRITE_ENTRY(RADIO_2064_REG007, 1)
		RADIO_REG_MOD_ENTRY(RADIO_2064_REG0FF, 0x10, 1<<4)
		RADIO_REG_MOD_ENTRY(RADIO_2064_REG11F, 0x4, 1<<2)
		PHY_REG_MOD_ENTRY(LCNPHY, rfoverride4, lpf_byp_rx_ovr, 1)
		PHY_REG_MOD_ENTRY(LCNPHY, rfoverride4, lpf_buf_pwrup_ovr, 1)
		PHY_REG_MOD_ENTRY(LCNPHY, rfoverride4val, lpf_byp_rx_ovr_val, 0)
		PHY_REG_MOD_ENTRY(LCNPHY, rfoverride4val, lpf_buf_pwrup_ovr_val, 1)
	PHY_REG_LIST_EXECUTE(pi);

	if (RADIOID(pi->pubpi.radioid) == BCM2064_ID) {
		PHY_REG_LIST_START
			RADIO_REG_MOD_ENTRY(RADIO_2064_REG03A, 1, 1)
			RADIO_REG_MOD_ENTRY(RADIO_2064_REG03A, 4, 1<<2)
			RADIO_REG_MOD_ENTRY(RADIO_2064_REG11A, 8, 1<<3)
		PHY_REG_LIST_EXECUTE(pi);
	} else {
		phy_utils_mod_radioreg(pi, RADIO_2064_REG113, 0x80, 0x80);
		if (CHSPEC_IS5G(pi->radio_chanspec)) {
			if (!pi->u.pi_lcnphy->ePA) {
				phy_utils_mod_radioreg(pi, RADIO_2064_REG07D, 0x3, 0);
				phy_utils_mod_radioreg(pi, RADIO_2064_REG028, 0xf, 0xc);
			} else {
				phy_utils_mod_radioreg(pi, RADIO_2064_REG07D, 0x3, 0x2);
				phy_utils_mod_radioreg(pi, RADIO_2064_REG028, 0xf, 0x1);
			}
			PHY_REG_LIST_START
				RADIO_REG_MOD_ENTRY(RADIO_2064_REG11A, 0x8, 0x8)
				RADIO_REG_MOD_ENTRY(RADIO_2064_REG0CA, 0x2, 0x2)
				RADIO_REG_MOD_ENTRY(RADIO_2064_REG0C0, 0x1, 0x1)
				RADIO_REG_MOD_ENTRY(RADIO_2064_REG126, 0x40, 0x40)
				RADIO_REG_MOD_ENTRY(RADIO_2064_REG0EB, 0x1, 0x1)
			PHY_REG_LIST_EXECUTE(pi);
		} else {
			if (!pi->u.pi_lcnphy->ePA) {
				phy_utils_mod_radioreg(pi, RADIO_2064_REG07D, 0x3, 0);
				phy_utils_mod_radioreg(pi, RADIO_2064_REG028, 0xf, 0xe);
			} else {
				phy_utils_mod_radioreg(pi, RADIO_2064_REG07D, 0x3, 0x3);
				phy_utils_mod_radioreg(pi, RADIO_2064_REG028, 0xf, 0x1);
			}
			PHY_REG_LIST_START
				RADIO_REG_MOD_ENTRY(RADIO_2064_REG03A, 0x2, 0x2)
				RADIO_REG_MOD_ENTRY(RADIO_2064_REG12E, 0x10, 0x10)
				RADIO_REG_MOD_ENTRY(RADIO_2064_REG086, 0x4, 0x4)
			PHY_REG_LIST_EXECUTE(pi);
		}
	}

	PHY_REG_LIST_START
		RADIO_REG_MOD_ENTRY(RADIO_2064_REG005, 8, 1<<3)
		RADIO_REG_MOD_ENTRY(RADIO_2064_REG11A, 1, 1)

		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlRfCtrlOverride0, tssiSelOverride, 1)
		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlRfCtrlOverride0, tssiSelOverrideVal, 0)
		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlRfCtrlOverride0, tssiRangeOverride, 1)
		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlRfCtrlOverride0, tssiRangeOverrideVal, 1)
		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlRfCtrlOverride0, amuxSelPortOverride, 1)
		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlRfCtrlOverride0, amuxSelPortOverrideVal, 2)

		RADIO_REG_MOD_ENTRY(RADIO_2064_REG082, 0x20, 1<<5)
	PHY_REG_LIST_EXECUTE(pi);

	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		auxpga_vmid = (0x200 |  (pi->u.pi_lcnphy->lcnphy_rssi_vc << 4)
			| pi->u.pi_lcnphy->lcnphy_rssi_vf);
		auxpga_gain = pi->u.pi_lcnphy->lcnphy_rssi_gs;
	}
#ifdef BAND5G
	else {
		auxpga_vmid = (0x200 |  (pi->u.pi_lcnphy->rssismc5g << 4)
			| pi->u.pi_lcnphy->rssismf5g);
		auxpga_gain = pi->u.pi_lcnphy->rssisav5g;
	}
#endif /* BAND5G */

	PHY_REG_MOD(pi, LCNPHY, TxPwrCtrlRfCtrlOverride1, afeAuxpgaSelVmidOverride, 1);
	PHY_REG_MOD(pi, LCNPHY, TxPwrCtrlRfCtrlOverride1, afeAuxpgaSelVmidOverrideVal,
		auxpga_vmid);
	PHY_REG_MOD(pi, LCNPHY, TxPwrCtrlRfCtrlOverride1, afeAuxpgaSelGainOverride, 1);
	PHY_REG_MOD(pi, LCNPHY, TxPwrCtrlRfCtrlOverride1, afeAuxpgaSelGainOverrideVal,
		auxpga_gain);

	PHY_REG_LIST_START
		/* set tx_pwr_ctrl clock */
		PHY_REG_MOD_ENTRY(LCNPHY, sslpnCalibClkEnCtrl, txFrontEndCalibClkEn, 1)
		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlCmd, use_txPwrCtrlCoefs, 0)
	PHY_REG_LIST_EXECUTE(pi);
}

static void
wlc_lcnphy_tssi_loopback_cleanup(phy_info_t *pi, uint16 *values_to_save)
{
	uint8 i;

	/* Restore RF Registers */
	for (i = 0; i < ARRAYSIZE(tone_power_rf_reg); i++) {
		phy_utils_write_radioreg(pi, tone_power_rf_reg[i], values_to_save[i]);
	}

	PHY_REG_LIST_START
		PHY_REG_MOD_ENTRY(LCNPHY, AfeCtrlOvr, pwdn_rssi_ovr, 0)
		PHY_REG_MOD_ENTRY(LCNPHY, rfoverride4, lpf_byp_rx_ovr, 0)
		PHY_REG_MOD_ENTRY(LCNPHY, rfoverride4, lpf_buf_pwrup_ovr, 0)
		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlRfCtrlOverride0, tssiSelOverride, 0)
		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlRfCtrlOverride0, tssiRangeOverride, 0)
		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlRfCtrlOverride0, amuxSelPortOverride, 0)
		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlRfCtrlOverride1, afeAuxpgaSelVmidOverride, 0)
		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlRfCtrlOverride1, afeAuxpgaSelGainOverride, 0)
	PHY_REG_LIST_EXECUTE(pi);
}

static bool
wlc_lcnphy_iqcal_wait(phy_info_t *pi)
{
	uint delay_count = 0;

	while (wlc_lcnphy_iqcal_active(pi)) {
		OSL_DELAY(100);
		delay_count++;

		if (delay_count > (10 * 500)) /* 500 ms */
			break;
	}

	PHY_TMP(("wl%d: %s: %u us\n", pi->sh->unit, __FUNCTION__, delay_count * 100));

	return (wlc_lcnphy_iqcal_active(pi) == 0);
}

static void
wlc_lcnphy_tx_iqlo_cal(
	phy_info_t *pi,
	lcnphy_txgains_t *target_gains,
	lcnphy_cal_mode_t cal_mode,
	bool keep_tone)
{
	/* starting values used in full cal
	 * -- can fill non-zero vals based on lab campaign (e.g., per channel)
	 * -- format: a0,b0,a1,b1,ci0_cq0_ci1_cq1,di0_dq0,di1_dq1,ei0_eq0,ei1_eq1,fi0_fq0,fi1_fq1
	 */
	lcnphy_txgains_t cal_gains, temp_gains;
	uint16 hash;
	uint8 band_idx;
	int j;
	uint16 ncorr_override[5];
	uint16 syst_coeffs[] =
		{0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000};

	/* cal commands full cal and recal */
	uint16 commands_fullcal[] =  { 0x8421, 0x8423, 0x8334, 0x8084, 0x8267, 0x8056, 0x8234 };

	/* do the recal with full cal cmds for now, re-visit if time becomes
	 * an issue.
	 */
	/* uint16 commands_recal[] =  { 0x8312, 0x8055, 0x8212 }; */
	uint16 commands_recal[] =  { 0x8421, 0x8423, 0x8334, 0x8084, 0x8267, 0x8056, 0x8234 };
	uint16 commands_iq_recal[] =  { 0x8067 };
	/* dig lo cal only */
	uint16 commands_dlo_recal[] = { 0x8245 };

	/* for populating tx power control table
	 * (like full cal but skip radio regs, since they don't exist in tx pwr ctrl table)
	 */
	uint16 commands_txpwrctrl[] = { 0x8084, 0x8267, 0x8056, 0x8234 };

	/* calCmdNum register: log2 of settle/measure times for search/gain-ctrl, 4 bits each */
	uint16 command_nums_fullcal[] = { 0x7a97, 0x7a97, 0x7a97, 0x7a97, 0x7a87, 0x7a87, 0x7b97 };
	uint16 command_nums_txpwrctrl[] = { 0x7a97, 0x7a87, 0x7a87, 0x7b97 };

	/* do the recal with full cal cmds for now, re-visit if time becomes
	 * an issue.
	 */
	/* uint16 command_nums_recal[] = {  0x7997, 0x7987, 0x7a97 }; */
	uint16 command_nums_recal[] = { 0x7a97, 0x7a97, 0x7a97, 0x7a97, 0x7a87, 0x7a87, 0x7b97 };
	uint16 *command_nums = command_nums_fullcal;
	uint16 command_nums_iq_recal[] = { 0x7b97 };
	uint16 command_nums_dlo_recal[] = { 0x7b97 };

	uint16 *start_coeffs = NULL, *cal_cmds = NULL, cal_type, diq_start;
	uint16 tx_pwr_ctrl_old, save_txpwrctrlrfctrl2;
	uint16 save_sslpnCalibClkEnCtrl, save_sslpnRxFeClkEnCtrl;
	bool tx_gain_override_old;
	lcnphy_txgains_t old_gains = {0, 0, 0, 0};
	uint i, n_cal_cmds = 0, n_cal_start = 0;
	uint16 *values_to_save;
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	int16 tone_freq;
	uint8 ei0, eq0, fi0, fq0;
	uint8 coarse_loft_gain_offset;
	int8 cal_type_prev;
	uint8 default_gain_index;
	uint16 save_TxPwrCtrlRfCtrlOverride0 = 0;
#if defined(PHYCAL_CACHING)
	lcnphy_calcache_t *cache;
	ch_calcache_t *ctx = wlc_phy_get_chanctx(pi, pi->radio_chanspec);
	if (ctx == NULL) {
		PHY_ERROR(("%s: NULL ctx - bail\n", __FUNCTION__));
		return;
	}

	cache = &ctx->u.lcnphy_cache;
#endif // endif

	if (NORADIO_ENAB(pi->pubpi))
		return;

	values_to_save = MALLOC(pi->sh->osh, sizeof(uint16) * ARRAYSIZE(iqlo_loopback_rf_regs));
	if (values_to_save == NULL) {
		PHY_ERROR(("wl%d: %s: MALLOC failure\n", pi->sh->unit, __FUNCTION__));
		return;
	}

	save_sslpnRxFeClkEnCtrl = phy_utils_read_phyreg(pi, LCNPHY_sslpnRxFeClkEnCtrl);
	save_sslpnCalibClkEnCtrl = phy_utils_read_phyreg(pi, LCNPHY_sslpnCalibClkEnCtrl);

	PHY_REG_LIST_START
		/* turn on clk to iqlo block */
		PHY_REG_OR_ENTRY(LCNPHY, sslpnCalibClkEnCtrl, 0x40)
		PHY_REG_OR_ENTRY(LCNPHY, sslpnRxFeClkEnCtrl, 0x3)
	PHY_REG_LIST_EXECUTE(pi);

	/* Get desired start coeffs and select calibration command sequence */

	switch (cal_mode) {
		case LCNPHY_CAL_FULL:
			start_coeffs = syst_coeffs;
			cal_cmds = commands_fullcal;
			n_cal_cmds = ARRAYSIZE(commands_fullcal);
			wlc_lcnphy_set_radio_loft(pi, 0, 0, 0, 0);
			break;

		case LCNPHY_CAL_RECAL:
#if defined(PHYCAL_CACHING)
			ASSERT(cache->txiqlocal_bestcoeffs_valid);
#else
			ASSERT(pi_lcn->lcnphy_cal_results.txiqlocal_bestcoeffs_valid);
#endif // endif

			/* start_coeffs = pi_lcn->lcnphy_cal_results.txiqlocal_bestcoeffs; */

			/* since re-cal is same as full cal */
			start_coeffs = syst_coeffs;

			cal_cmds = commands_recal;
			n_cal_cmds = ARRAYSIZE(commands_recal);
			command_nums = command_nums_recal;
			break;

		case LCNPHY_CAL_IQ_RECAL:

#if defined(PHYCAL_CACHING)
			ASSERT(cache->txiqlocal_bestcoeffs_valid);
			start_coeffs = cache->txiqlocal_bestcoeffs;
#else
			ASSERT(pi_lcn->lcnphy_cal_results.txiqlocal_bestcoeffs_valid);
			start_coeffs = pi_lcn->lcnphy_cal_results.txiqlocal_bestcoeffs;
#endif // endif

			cal_cmds = commands_iq_recal;
			n_cal_cmds = ARRAYSIZE(commands_iq_recal);
			command_nums = command_nums_iq_recal;
			break;

		case LCNPHY_CAL_DLO_RECAL:

#if defined(PHYCAL_CACHING)
			ASSERT(cache->txiqlocal_bestcoeffs_valid);
			start_coeffs = cache->txiqlocal_bestcoeffs;
#else
			ASSERT(pi_lcn->lcnphy_cal_results.txiqlocal_bestcoeffs_valid);
			start_coeffs = pi_lcn->lcnphy_cal_results.txiqlocal_bestcoeffs;
#endif // endif

			cal_cmds = commands_dlo_recal;
			n_cal_cmds = ARRAYSIZE(commands_dlo_recal);
			command_nums = command_nums_dlo_recal;
			break;

		case LCNPHY_CAL_TXPWRCTRL:

			wlc_lcnphy_get_radio_loft(pi, &ei0, &eq0, &fi0, &fq0);
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
	wlc_lcnphy_common_write_table(pi, LCNPHY_TBL_ID_IQLOCAL,
		start_coeffs, 11, 16, 64);

	PHY_REG_LIST_START
		PHY_REG_WRITE_ENTRY(LCNPHY, sslpnCalibClkEnCtrl, 0xffff)
		PHY_REG_MOD_ENTRY(LCNPHY, auxadcCtrl, iqlocalEn, 1)
	PHY_REG_LIST_EXECUTE(pi);

	/* Save original tx power control mode */
	tx_pwr_ctrl_old = wlc_lcnphy_get_tx_pwr_ctrl(pi);

	PHY_REG_MOD(pi, LCNPHY, TxPwrCtrlCmd, txPwrCtrl_swap_iq, 1);

	/* Disable tx power control */
	wlc_lcnphy_set_tx_pwr_ctrl(pi, LCNPHY_TX_PWR_CTRL_OFF);

	save_txpwrctrlrfctrl2 = phy_utils_read_phyreg(pi, LCNPHY_TxPwrCtrlRfCtrl2);

	PHY_REG_LIST_START
		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlRfCtrl2, afeAuxpgaSelVmidVal0, 0x2a6)
		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlRfCtrl2, afeAuxpgaSelGainVal0, 2)
	PHY_REG_LIST_EXECUTE(pi);

	if (LCNREV_GE(pi->pubpi.phy_rev, 3)) {
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			PHY_REG_LIST_START
				PHY_REG_MOD_ENTRY(LCNPHY, iqlocalRadioRegAddr0,
					txrf_iqcal_gain_waddr, 0x26)
				PHY_REG_MOD_ENTRY(LCNPHY, iqlocalRadioRegAdd1,
					tx_vos_mxi_waddr, 0x89)
				PHY_REG_MOD_ENTRY(LCNPHY, iqlocalRadioRegAddr2,
					tx_vos_mxq_waddr, 0x8a)
				PHY_REG_MOD_ENTRY(LCNPHY, iqlocalRadioRegAddr2,
					tx_idac_lo_rfi_waddr9to4, 0x8b >> 4)
				PHY_REG_MOD_ENTRY(LCNPHY, iqlocalRadioRegAdd3,
					tx_idac_lo_rfi_waddr3to0, 0x8b & 0xf)
				PHY_REG_MOD_ENTRY(LCNPHY, iqlocalRadioRegAdd3,
					tx_idac_lo_rfq_waddr, 0x8c)
			PHY_REG_LIST_EXECUTE(pi);
		} else {
			PHY_REG_LIST_START
				PHY_REG_MOD_ENTRY(LCNPHY, iqlocalRadioRegAddr0,
					txrf_iqcal_gain_waddr, 0x26)
				PHY_REG_MOD_ENTRY(LCNPHY, iqlocalRadioRegAdd1,
					tx_vos_mxi_waddr, 0xA5)
				PHY_REG_MOD_ENTRY(LCNPHY, iqlocalRadioRegAddr2,
					tx_vos_mxq_waddr, 0xA6)
				PHY_REG_MOD_ENTRY(LCNPHY, iqlocalRadioRegAddr2,
					tx_idac_lo_rfi_waddr9to4, 0xA7 >> 4)
				PHY_REG_MOD_ENTRY(LCNPHY, iqlocalRadioRegAdd3,
					tx_idac_lo_rfi_waddr3to0, 0xA7 & 0xf)
				PHY_REG_MOD_ENTRY(LCNPHY, iqlocalRadioRegAdd3,
					tx_idac_lo_rfq_waddr, 0xA8)
			PHY_REG_LIST_EXECUTE(pi);
		}
	}

	/* setup tx iq loopback path */
	/* save/restore tssiRangeOverride */
	save_TxPwrCtrlRfCtrlOverride0 = phy_utils_read_phyreg(pi, LCNPHY_TxPwrCtrlRfCtrlOverride0);
	wlc_lcnphy_tx_iqlo_loopback(pi, values_to_save);

	/* Save old and apply new tx gains if needed */
	tx_gain_override_old = wlc_lcnphy_tx_gain_override_enabled(pi);
	if (tx_gain_override_old)
		wlc_lcnphy_get_tx_gain(pi, &old_gains);

	if (!target_gains) {
		if (!tx_gain_override_old)
			wlc_lcnphy_set_tx_pwr_by_index(pi, pi_lcn->lcnphy_tssi_idx);
		wlc_lcnphy_get_tx_gain(pi, &temp_gains);
		target_gains = &temp_gains;
	}

	hash = (target_gains->gm_gain << 8) |
		(target_gains->pga_gain << 4) |
		(target_gains->pad_gain);

	band_idx = (CHSPEC_IS5G(pi->radio_chanspec) ? 1 : 0);

	cal_gains = *target_gains;
	bzero(ncorr_override, sizeof(ncorr_override));
	for (j = 0; j < iqcal_gainparams_numgains_lcnphy[band_idx]; j++) {
		if (hash == tbl_iqcal_gainparams_lcnphy[band_idx][j][0]) {
			cal_gains.gm_gain = tbl_iqcal_gainparams_lcnphy[band_idx][j][1];
			cal_gains.pga_gain = tbl_iqcal_gainparams_lcnphy[band_idx][j][2];
			cal_gains.pad_gain = tbl_iqcal_gainparams_lcnphy[band_idx][j][3];
			bcopy(&tbl_iqcal_gainparams_lcnphy[band_idx][j][3], ncorr_override,
				sizeof(ncorr_override));
			break;
		}
	}
	/* apply cal gains */
	wlc_lcnphy_set_tx_gain(pi, &cal_gains);

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

	PHY_REG_LIST_START
		/* set gain control parameters */
		PHY_REG_WRITE_ENTRY(LCNPHY, iqloCalCmdGctl, 0xaa9)
		PHY_REG_WRITE_ENTRY(LCNPHY, iqloCalGainThreshD2, 0xc0)
	PHY_REG_LIST_EXECUTE(pi);

	if (CHIPID(pi->sh->chip) == BCM4313_CHIP_ID) {
		/* Load the LO compensation gain table */
		wlc_lcnphy_common_write_table(pi, LCNPHY_TBL_ID_IQLOCAL,
			lcnphy_iqcal_loft_gainladder, ARRAYSIZE(lcnphy_iqcal_loft_gainladder),
			16, 0);
		/* Load the IQ calibration gain table */
		wlc_lcnphy_common_write_table(pi, LCNPHY_TBL_ID_IQLOCAL,
			lcnphy_iqcal_ir_gainladder, ARRAYSIZE(lcnphy_iqcal_ir_gainladder),
			16, 32);
	} else {
		/* Load the LO compensation gain table */
		wlc_lcnphy_common_write_table(pi, LCNPHY_TBL_ID_IQLOCAL,
			lcnphy_iqcal_common_gainladder, ARRAYSIZE(lcnphy_iqcal_common_gainladder),
			16, 0);
		/* Load the IQ calibration gain table */
		wlc_lcnphy_common_write_table(pi, LCNPHY_TBL_ID_IQLOCAL,
			lcnphy_iqcal_common_gainladder, ARRAYSIZE(lcnphy_iqcal_common_gainladder),
			16, 32);

		PHY_REG_WRITE(pi, LCNPHY, iqloCalCmdGctl, 0x7a4);
	}

	if (pi_lcn->lcnphy_tx_iqlo_tone_freq_ovr_val == 0)
		tone_freq = 3750;
	else
		tone_freq = pi_lcn->lcnphy_tx_iqlo_tone_freq_ovr_val;

	if (cal_mode == LCNPHY_CAL_IQ_RECAL)
		tone_freq = -tone_freq;

	/* Send out calibration tone */
	if (pi->phy_tx_tone_freq) {
		/* if tone is already played out with iq cal mode zero then
		 * stop the tone and re-play with iq cal mode 1.
		 */
		wlc_lcnphy_stop_tx_tone(pi);
		OSL_DELAY(5);
		wlc_lcnphy_start_tx_tone(pi, (tone_freq * 1000), 88, 1);
	} else {
		wlc_lcnphy_start_tx_tone(pi, (tone_freq * 1000), 88, 1);
	}

	/* FIX ME: re-enable all the phy clks. */
	PHY_REG_WRITE(pi, LCNPHY, sslpnCalibClkEnCtrl, 0xffff);

	if (CHSPEC_IS2G(pi->radio_chanspec))
		coarse_loft_gain_offset = pi_lcn->iqlocalst1off_2g;
	else
		coarse_loft_gain_offset = pi_lcn->iqlocalst1off_5g;

	cal_type_prev = -1;
	default_gain_index = wlc_lcnphy_get_current_tx_pwr_idx(pi);

	/*
	 * Cal Steps
	 */
	for (i = n_cal_start; i < n_cal_cmds; i++) {
		uint16 zero_diq = 0;
		uint16 best_coeffs[11];
		uint16 command_num;

		cal_type = (cal_cmds[i] & 0x0f00) >> 8;

		/* run coarse LOFT cal at lower power to improve stability */
		if (coarse_loft_gain_offset) {
			uint8 gain_index;
			if ((cal_type == 4) || (cal_type_prev == 4)) {
				if (cal_type == 4) {
					gain_index = default_gain_index + coarse_loft_gain_offset;
					if (gain_index > 127)
						gain_index = 127;
				} else {
					gain_index = default_gain_index;
				}
				wlc_lcnphy_set_tx_idx_raw(pi, gain_index);
			}
			cal_type_prev = (int8)cal_type;
		}

		/* get & set intervals */
		command_num = command_nums[i];
		if (ncorr_override[cal_type])
			command_num = ncorr_override[cal_type] << 8 | (command_num & 0xff);

		PHY_REG_WRITE(pi, LCNPHY, iqloCalCmdNnum, command_num);

		PHY_TMP(("wl%d: %s: running cmd: %x, cmd_num: %x\n",
			pi->sh->unit, __FUNCTION__, cal_cmds[i], command_nums[i]));

		/* need to set di/dq to zero if analog LO cal */
		if ((cal_type == 3) || (cal_type == 4)) {
			/* store given dig LOFT comp vals */
			wlc_lcnphy_common_read_table(pi, LCNPHY_TBL_ID_IQLOCAL,
				&diq_start, 1, 16, 69);
			/* Set to zero during analog LO cal */
			wlc_lcnphy_common_write_table(pi, LCNPHY_TBL_ID_IQLOCAL, &zero_diq,
				1, 16, 69);
		}

		/* Issue cal command */
		PHY_REG_WRITE(pi, LCNPHY, iqloCalCmd, cal_cmds[i]);

		/* Wait until cal command finished */
		if (!wlc_lcnphy_iqcal_wait(pi)) {
			PHY_ERROR(("wl%d: %s: tx iqlo cal failed to complete\n",
				pi->sh->unit, __FUNCTION__));
			/* No point to continue */
			goto cleanup;
		}

		/* Copy best coefficients to start coefficients */
		wlc_lcnphy_common_read_table(pi, LCNPHY_TBL_ID_IQLOCAL,
			best_coeffs, ARRAYSIZE(best_coeffs), 16, 96);
		wlc_lcnphy_common_write_table(pi, LCNPHY_TBL_ID_IQLOCAL, best_coeffs,
			ARRAYSIZE(best_coeffs), 16, 64);
		/* restore di/dq in case of analog LO cal */
		if ((cal_type == 3) || (cal_type == 4)) {
			wlc_lcnphy_common_write_table(pi, LCNPHY_TBL_ID_IQLOCAL,
				&diq_start, 1, 16, 69);
		}
#if defined(PHYCAL_CACHING)
		wlc_lcnphy_common_read_table(pi, LCNPHY_TBL_ID_IQLOCAL,
			cache->txiqlocal_bestcoeffs,
			ARRAYSIZE(cache->txiqlocal_bestcoeffs), 16, 96);
#else
		wlc_lcnphy_common_read_table(pi, LCNPHY_TBL_ID_IQLOCAL,
			pi_lcn->lcnphy_cal_results.txiqlocal_bestcoeffs,
			ARRAYSIZE(pi_lcn->lcnphy_cal_results.txiqlocal_bestcoeffs), 16, 96);
#endif /* PHYCAL_CACHING */
	}

	/*
	 * Apply Results
	 */

	/* Save calibration results */
#if defined(PHYCAL_CACHING)
	wlc_lcnphy_common_read_table(pi, LCNPHY_TBL_ID_IQLOCAL,
		cache->txiqlocal_bestcoeffs,
		ARRAYSIZE(cache->txiqlocal_bestcoeffs), 16, 96);
	cache->txiqlocal_bestcoeffs_valid = TRUE;

	/* Apply IQ Cal Results */
	wlc_lcnphy_common_write_table(pi, LCNPHY_TBL_ID_IQLOCAL,
		&cache->txiqlocal_bestcoeffs[0], 4, 16, 80);
	/* Apply Digital LOFT Comp */
	wlc_lcnphy_common_write_table(pi, LCNPHY_TBL_ID_IQLOCAL,
		&cache->txiqlocal_bestcoeffs[5], 2, 16, 85);
#else
	wlc_lcnphy_common_read_table(pi, LCNPHY_TBL_ID_IQLOCAL,
		pi_lcn->lcnphy_cal_results.txiqlocal_bestcoeffs,
		ARRAYSIZE(pi_lcn->lcnphy_cal_results.txiqlocal_bestcoeffs), 16, 96);
	pi_lcn->lcnphy_cal_results.txiqlocal_bestcoeffs_valid = TRUE;

	/* Apply IQ Cal Results */
	wlc_lcnphy_common_write_table(pi, LCNPHY_TBL_ID_IQLOCAL,
		&pi_lcn->lcnphy_cal_results.txiqlocal_bestcoeffs[0], 4, 16, 80);
	/* Apply Digital LOFT Comp */
	wlc_lcnphy_common_write_table(pi, LCNPHY_TBL_ID_IQLOCAL,
		&pi_lcn->lcnphy_cal_results.txiqlocal_bestcoeffs[5], 2, 16, 85);
	/* Apply per modulation coeffs */
	if (wlc_lcnphy_frmTypBasedIqLoCoeff(pi)) {
		uint16 a, b, didq;
		wlc_lcnphy_get_tx_iqcc(pi, &a, &b);
		didq = wlc_lcnphy_get_tx_locc(pi);
		wlc_lcnphy_set_tx_iqcc(pi, a, b);
		wlc_lcnphy_set_tx_locc(pi, didq);
	}
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
		wlc_lcnphy_stop_tx_tone(pi);

	wlc_lcnphy_tx_iqlo_loopback_cleanup(pi, values_to_save);
	PHY_REG_WRITE(pi, LCNPHY, TxPwrCtrlRfCtrlOverride0, save_TxPwrCtrlRfCtrlOverride0);
	MFREE(pi->sh->osh, values_to_save, ARRAYSIZE(iqlo_loopback_rf_regs) * sizeof(uint16));

	/* restore tx power and reenable tx power control */
	PHY_REG_WRITE(pi, LCNPHY, TxPwrCtrlRfCtrl2, save_txpwrctrlrfctrl2);

	/* Reset calibration  command register */
	PHY_REG_WRITE(pi, LCNPHY, iqloCalCmdGctl, 0);

	/* Restore tx power and reenable tx power control */
	if (tx_gain_override_old)
		wlc_lcnphy_set_tx_gain(pi, &old_gains);
	wlc_lcnphy_set_tx_pwr_ctrl(pi, tx_pwr_ctrl_old);

	/* Restoration RxFE clk */
	PHY_REG_WRITE(pi, LCNPHY, sslpnCalibClkEnCtrl, save_sslpnCalibClkEnCtrl);
	PHY_REG_WRITE(pi, LCNPHY, sslpnRxFeClkEnCtrl, save_sslpnRxFeClkEnCtrl);

}

static void
wlc_lcnphy_idle_tssi_est(wlc_phy_t *ppi)
{
	bool suspend, tx_gain_override_old;
	lcnphy_txgains_t old_gains;
	phy_info_t *pi = (phy_info_t *)ppi;
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	int diff;
	uint8 SAVE_bbmult;
	uint16 idleTssi, idleTssi0_2C, idleTssi0_OB, idleTssi0_regvalue_OB, idleTssi0_regvalue_2C;
#if TWO_POWER_RANGE_TXPWR_CTRL
	uint16 idleTssi1, idleTssi1_2C, idleTssi1_OB, idleTssi1_regvalue_OB, idleTssi1_regvalue_2C;
#endif // endif
	uint16 SAVE_txpwrctrl = wlc_lcnphy_get_tx_pwr_ctrl(pi);
	uint8 SAVE_indx = wlc_lcnphy_get_current_tx_pwr_idx(pi);
	uint16 SAVE_lpfgain = phy_utils_read_radioreg(pi, RADIO_2064_REG112);
	uint16 SAVE_jtag_bb_afe_switch = phy_utils_read_radioreg(pi, RADIO_2064_REG007) & 1;
	uint16 SAVE_jtag_auxpga = phy_utils_read_radioreg(pi, RADIO_2064_REG0FF) & 0x10;
	uint16 SAVE_iqadc_aux_en = phy_utils_read_radioreg(pi, RADIO_2064_REG11F) & 4;
	idleTssi = phy_utils_read_phyreg(pi, LCNPHY_TxPwrCtrlStatus);
	suspend = !(R_REG(pi->sh->osh, &((phy_info_t *)pi)->regs->maccontrol) & MCTL_EN_MAC);
	if (!suspend)
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
	wlc_lcnphy_set_tx_pwr_ctrl(pi, LCNPHY_TX_PWR_CTRL_OFF);
	wlc_btcx_override_enable(pi);
	/* Save old tx gains if needed */
	tx_gain_override_old = wlc_lcnphy_tx_gain_override_enabled(pi);
	wlc_lcnphy_get_tx_gain(pi, &old_gains);
	/* set txgain override */
	wlc_lcnphy_enable_tx_gain_override(pi);
	wlc_lcnphy_set_tx_pwr_by_index(pi, 127);
	if (LCNREV_LT(pi->pubpi.phy_rev, 1)) {
		phy_utils_write_radioreg(pi, RADIO_2064_REG112, 0x6);
	}

	PHY_REG_LIST_START
		RADIO_REG_MOD_ENTRY(RADIO_2064_REG007, 0x1, 1)
		RADIO_REG_MOD_ENTRY(RADIO_2064_REG0FF, 0x10, 1 << 4)
		RADIO_REG_MOD_ENTRY(RADIO_2064_REG11F, 0x4, 1 << 2)
	PHY_REG_LIST_EXECUTE(pi);

	wlc_lcnphy_tssi_setup(pi);

	PHY_REG_LIST_START
		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlRfCtrlOverride0, tssiRangeOverride, 1)
		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlRfCtrlOverride0, tssiRangeOverrideVal, 1)
	PHY_REG_LIST_EXECUTE(pi);

	SAVE_bbmult = wlc_lcnphy_get_bbmult(pi);
	wlc_lcnphy_set_bbmult(pi, 0x0);
	wlc_phy_do_dummy_tx(pi, TRUE, OFF);
	idleTssi = PHY_REG_READ(pi, LCNPHY, TxPwrCtrlStatus, estPwr);
	/* avgTssi value is in 2C (S9.0) format */
	idleTssi0_2C = PHY_REG_READ(pi, LCNPHY, TxPwrCtrlStatusNew4, avgTssi);
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

	if ((CHIPID(pi->sh->chip) == BCM4336_CHIP_ID) && LCNREV_LT(pi->pubpi.phy_rev, 1)) {
		diff = (LCN_TARGET_TSSI * 4 - idleTssi0_OB) > 0 ?
			LCN_TARGET_TSSI * 4 - idleTssi0_OB : 0;
		idleTssi0_regvalue_OB = 511 - diff;
	} else
		idleTssi0_regvalue_OB = idleTssi0_OB;

	if (idleTssi0_regvalue_OB >= 256)
		idleTssi0_regvalue_2C = idleTssi0_regvalue_OB - 256;
	else
		idleTssi0_regvalue_2C = idleTssi0_regvalue_OB + 256;

#if TWO_POWER_RANGE_TXPWR_CTRL
	if (pi_lcn->lcnphy_twopwr_txpwrctrl_en) {
		PHY_REG_LIST_START
			PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlRfCtrlOverride0,
				tssiRangeOverride, 1)
			PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlRfCtrlOverride0,
				tssiRangeOverrideVal, 0)
			PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlCmdNew, txPwrCtrlScheme, 2)
		PHY_REG_LIST_EXECUTE(pi);

		wlc_phy_do_dummy_tx(pi, TRUE, OFF);

		/* idletssi1 is calculated from path 0 after attenuator setting */
		/* since HW uses path 0 always when txpower is set by index */
		idleTssi1 = PHY_REG_READ(pi, LCNPHY, TxPwrCtrlStatus, estPwr);
		/* avgTssi value is in 2C (S9.0) format */
		idleTssi1_2C = PHY_REG_READ(pi, LCNPHY, TxPwrCtrlStatusNew4, avgTssi);
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

		PHY_REG_MOD(pi, LCNPHY, TxPwrCtrlIdleTssi1, idleTssi1, idleTssi1_regvalue_2C);

		PHY_REG_LIST_START
			/* Clear tssiRangeOverride */
			PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlRfCtrlOverride0, tssiRangeOverride, 0)
			PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlRfCtrl0, tssiRangeVal0, 1)
			PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlRfCtrl0, tssiRangeVal1, 0)
		PHY_REG_LIST_EXECUTE(pi);
	}
#endif  /* #if TWO_POWER_RANGE_TXPWR_CTRL */

	/* Write after idletssi1 is calculated since it depends on idleTssi0 set to 0xFF */
	PHY_REG_MOD(pi, LCNPHY, TxPwrCtrlIdleTssi, idleTssi0, idleTssi0_regvalue_2C);

	/* Clear tx PU override */
	PHY_REG_MOD(pi, LCNPHY, RFOverride0, internalrftxpu_ovr, 0);
	wlc_lcnphy_set_bbmult(pi, SAVE_bbmult);
	/* restore txgain override */
	wlc_lcnphy_set_tx_gain_override(pi, tx_gain_override_old);
	wlc_lcnphy_set_tx_gain(pi, &old_gains);
	wlc_lcnphy_set_tx_pwr_by_index(pi, SAVE_indx);
	wlc_lcnphy_set_tx_pwr_ctrl(pi, SAVE_txpwrctrl);
	/* restore radio registers */
	phy_utils_write_radioreg(pi, RADIO_2064_REG112, SAVE_lpfgain);
	phy_utils_mod_radioreg(pi, RADIO_2064_REG007, 0x1, SAVE_jtag_bb_afe_switch);
	phy_utils_mod_radioreg(pi, RADIO_2064_REG0FF, 0x10, SAVE_jtag_auxpga);
	phy_utils_mod_radioreg(pi, RADIO_2064_REG11F, 0x4, SAVE_iqadc_aux_en);
	/* making amux_sel_port to be always 1 for 4313 tx pwr ctrl */
	if (CHIPID(pi->sh->chip) == BCM4313_CHIP_ID)
		phy_utils_mod_radioreg(pi, RADIO_2064_REG112, 0x80, 1 << 7);
	if (!suspend)
		wlapi_enable_mac(pi->sh->physhim);
}

int
wlc_lcnphy_idle_tssi_est_iovar(phy_info_t *pi, bool type)
{
	/* type = 0 will just do idle_tssi_est */
	/* type = 1 will do full tx_pwr_ctrl_init */

	if (!type)
		wlc_lcnphy_idle_tssi_est((wlc_phy_t *)pi);
	else
		wlc_lcnphy_tx_pwr_ctrl_init((wlc_phy_t *)pi);

	wlc_lcnphy_set_tx_pwr_ctrl(pi, LCNPHY_TX_PWR_CTRL_OFF);
	wlc_lcnphy_set_tx_pwr_ctrl(pi, LCNPHY_TX_PWR_CTRL_HW);

	return (phy_utils_read_phyreg(pi, LCNPHY_TxPwrCtrlIdleTssi));
}

static void
wlc_lcnphy_vbat_temp_sense_setup(phy_info_t *pi, uint8 mode)
{
	bool suspend;
	uint16 save_txpwrCtrlEn;
	uint8 auxpga_vmidcourse, auxpga_vmidfine, auxpga_gain;
	uint16 auxpga_vmid;
	phytbl_info_t tab;
	uint16 val;
	uint8 save_reg007, save_reg0FF, save_reg11F, save_reg005, save_reg025, save_reg112;
	uint16 values_to_save[ARRAYSIZE(tempsense_phy_regs)];
	int8 indx;
	uint i;
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	OSL_DELAY(999);

	/* save older states of registers */
	/* radio */
	save_reg007 = (uint8)phy_utils_read_radioreg(pi, RADIO_2064_REG007);
	save_reg0FF = (uint8)phy_utils_read_radioreg(pi, RADIO_2064_REG0FF);
	save_reg11F = (uint8)phy_utils_read_radioreg(pi, RADIO_2064_REG11F);
	save_reg005 = (uint8)phy_utils_read_radioreg(pi, RADIO_2064_REG005);
	save_reg025 = (uint8)phy_utils_read_radioreg(pi, RADIO_2064_REG025);
	save_reg112 = (uint8)phy_utils_read_radioreg(pi, RADIO_2064_REG112);
	/* phy */
	for (i = 0; i < ARRAYSIZE(tempsense_phy_regs); i++)
		values_to_save[i] = phy_utils_read_phyreg(pi, tempsense_phy_regs[i]);
	suspend = !(R_REG(pi->sh->osh, &pi->regs->maccontrol) & MCTL_EN_MAC);
	if (!suspend)
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
	save_txpwrCtrlEn = phy_utils_read_radioreg(pi, LCNPHY_TxPwrCtrlCmd);
	/* disable tx power control measurement */
	wlc_lcnphy_set_tx_pwr_ctrl(pi, LCNPHY_TX_PWR_CTRL_OFF);
	indx = pi_lcn->lcnphy_current_index;
	wlc_lcnphy_set_tx_pwr_by_index(pi, 127);
	PHY_REG_LIST_START
		RADIO_REG_MOD_ENTRY(RADIO_2064_REG007, 0x1, 0x1)
		RADIO_REG_MOD_ENTRY(RADIO_2064_REG0FF, 0x10, 0x1 << 4)
		RADIO_REG_MOD_ENTRY(RADIO_2064_REG11F, 0x4, 0x1 << 2)

		PHY_REG_MOD_ENTRY(LCNPHY, auxadcCtrl, rssifiltEn, 0)
		PHY_REG_MOD_ENTRY(LCNPHY, auxadcCtrl, rssiFormatConvEn, 0)
		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlCmd, hwtxPwrCtrl_en, 0)
		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlCmd, txPwrCtrl_en, 0)
		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlRangeCmd, force_vbatTemp, 0)
		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlNnum, Ntssi_delay, 255)
		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlNnum, Ntssi_intg_log2, 5)
		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlNnum, Npt_intg_log2, 0)
		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlNum_Vbat, Nvbat_delay, 64)
		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlNum_Vbat, Nvbat_intg_log2, 6)
		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlNum_temp, Ntemp_delay, 64)
		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlNum_temp, Ntemp_intg_log2, 6)
		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlRfCtrl0, amuxSelPortVal0, 2)
		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlRfCtrl0, amuxSelPortVal1, 3)
		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlRfCtrl0, amuxSelPortVal2, 1)
		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlRfCtrl1, tempsenseSwapVal0, 0)
		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlRfCtrl1, tempsenseSwapVal1, 1)
		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlIdleTssi, rawTssiOffsetBinFormat, 1)
	PHY_REG_LIST_EXECUTE(pi);

	/* Reduce tssi delay for short adc rfseq power down */
	PHY_REG_MOD(pi, LCNPHY, TxPwrCtrlNnum, Ntssi_delay, MIN(255,
		wlc_lcnphy_rfseq_tbl_adc_pwrup(pi)/2 + 25));

	PHY_REG_LIST_START
		/* disable iqlo cal */
		RADIO_REG_WRITE_ENTRY(RADIO_2064_REG025, 0xC)
		/* amux sel port */
		RADIO_REG_MOD_ENTRY(RADIO_2064_REG005, 0x8, 0x1 << 3)

		/* enable Rx buf */
		PHY_REG_MOD_ENTRY(LCNPHY, rfoverride4, lpf_buf_pwrup_ovr, 1)
		PHY_REG_MOD_ENTRY(LCNPHY, rfoverride4val, lpf_buf_pwrup_ovr_val, 1)
		/* swap iq */
		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlCmd, txPwrCtrl_swap_iq, 1)
	PHY_REG_LIST_EXECUTE(pi);

	/* initialize the delay value for keeping ADC powered up during Tx */
	val = wlc_lcnphy_rfseq_tbl_adc_pwrup(pi);
	tab.tbl_id = LCNPHY_TBL_ID_RFSEQ;
	tab.tbl_width = 16;     /* 12 bit wide  */
	tab.tbl_len = 1;        /* # values   */
	tab.tbl_ptr = &val; /* ptr to buf */
	tab.tbl_offset = 6;
	wlc_lcnphy_write_table(pi, &tab);
	if (mode == TEMPSENSE) {
		PHY_REG_LIST_START
			PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlRfCtrlOverride0,
				amuxSelPortOverride, 1)
			PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlRfCtrlOverride0,
				amuxSelPortOverrideVal, 1)
		PHY_REG_LIST_EXECUTE(pi);
		auxpga_vmidcourse = 8;
		auxpga_vmidfine = 0x4;
		auxpga_gain = 2;
		phy_utils_mod_radioreg(pi, RADIO_2064_REG082, 0x20, 1 << 5);
	} else {
		PHY_REG_LIST_START
			PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlRfCtrlOverride0,
				amuxSelPortOverride, 1)
			PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlRfCtrlOverride0,
				amuxSelPortOverrideVal, 3)
		PHY_REG_LIST_EXECUTE(pi);
		auxpga_vmidcourse = 7;
		auxpga_vmidfine = 0xa;
		auxpga_gain = 2;
	}
	auxpga_vmid = (uint16)((2 << 8) | (auxpga_vmidcourse << 4) | auxpga_vmidfine);

	PHY_REG_MOD(pi, LCNPHY, TxPwrCtrlRfCtrlOverride1, afeAuxpgaSelVmidOverride, 1);
	PHY_REG_MOD(pi, LCNPHY, TxPwrCtrlRfCtrlOverride1, afeAuxpgaSelVmidOverrideVal,
		auxpga_vmid);
	PHY_REG_MOD(pi, LCNPHY, TxPwrCtrlRfCtrlOverride1, afeAuxpgaSelGainOverride, 1);
	PHY_REG_MOD(pi, LCNPHY, TxPwrCtrlRfCtrlOverride1, afeAuxpgaSelGainOverrideVal,
		auxpga_gain);

	/* trigger Vbat and Temp sense measurements */
	PHY_REG_MOD(pi, LCNPHY, TxPwrCtrlRangeCmd, force_vbatTemp, 1);

	/* to put lpf in Tx */
	if (LCNREV_LT(pi->pubpi.phy_rev, 1)) {
		phy_utils_or_radioreg(pi, RADIO_2064_REG112, 0x6);
	}
	/* do dummy tx */
	wlc_phy_do_dummy_tx(pi, TRUE, OFF);
	if (!tempsense_done(pi))
		OSL_DELAY(10);
	/* clean up */
	phy_utils_write_radioreg(pi, RADIO_2064_REG007, (uint16)save_reg007);
	phy_utils_write_radioreg(pi, RADIO_2064_REG0FF, (uint16)save_reg0FF);
	phy_utils_write_radioreg(pi, RADIO_2064_REG11F, (uint16)save_reg11F);
	phy_utils_write_radioreg(pi, RADIO_2064_REG005, (uint16)save_reg005);
	phy_utils_write_radioreg(pi, RADIO_2064_REG025, (uint16)save_reg025);
	phy_utils_write_radioreg(pi, RADIO_2064_REG112, (uint16)save_reg112);
	for (i = 0; i < ARRAYSIZE(tempsense_phy_regs); i++)
		phy_utils_write_phyreg(pi, tempsense_phy_regs[i], values_to_save[i]);
	wlc_lcnphy_set_tx_pwr_by_index(pi, (int)indx);
	/* restore the state of txpwrctrl */
	phy_utils_write_radioreg(pi, LCNPHY_TxPwrCtrlCmd, save_txpwrCtrlEn);
	if (!suspend)
		wlapi_enable_mac(pi->sh->physhim);
	OSL_DELAY(999);
}

/* Convert tssi to power LUT */
static void
wlc_lcnphy_set_estPwrLUT(phy_info_t *pi, int32 lut_num)
{
	phytbl_info_t tab;
	int32 tssi, pwr;
	int32 a1 = 0, b0 = 0, b1 = 0;
	int32 maxtargetpwr, mintargetpwr;

	tab.tbl_id = LCNPHY_TBL_ID_TXPWRCTL;
	if (lut_num == 0)
		tab.tbl_offset = 0; /* estPwrLuts */
	else
		tab.tbl_offset = LCNPHY_TX_PWR_CTRL_EST_PWR_OFFSET; /* estPwrLuts1 */
	tab.tbl_width = 32;	/* 32 bit wide	*/
	tab.tbl_ptr = &pwr; /* ptr to buf */
	tab.tbl_len = 1;        /* # values   */

	if (lut_num == 0) {
		/* Get the PA params for the particular channel we are in */
		wlc_phy_get_paparams_for_band(pi, &a1, &b0, &b1);

		if (CHIPID(pi->sh->chip) != BCM4313_CHIP_ID) {
			if (LCNREV_GE(pi->pubpi.phy_rev, 1)) {
				maxtargetpwr = wlc_lcnphy_tssi2dbm(1, a1, b0, b1);
				mintargetpwr = 0;
			} else {
				maxtargetpwr = wlc_lcnphy_tssi2dbm(127, a1, b0, b1);
				mintargetpwr = wlc_lcnphy_tssi2dbm(LCN_TARGET_TSSI, a1, b0, b1);
			}
		} else {
			maxtargetpwr = wlc_lcnphy_tssi2dbm(10, a1, b0, b1);
			mintargetpwr = wlc_lcnphy_tssi2dbm(125, a1, b0, b1);
		}

	}
	else {
		b0 = pi->txpa_2g_lo[0];
		b1 = pi->txpa_2g_lo[1];
		a1 = pi->txpa_2g_lo[2];
		maxtargetpwr = wlc_lcnphy_tssi2dbm(1, a1, b0, b1);
		mintargetpwr = 0;
	}

	for (tssi = 0; tssi < 128; tssi++) {
		pwr = wlc_lcnphy_tssi2dbm(tssi, a1, b0, b1);
		if (LCNREV_LT(pi->pubpi.phy_rev, 2))
			pwr = (pwr < mintargetpwr) ? mintargetpwr : pwr;
		wlc_lcnphy_write_table(pi,  &tab);
		tab.tbl_offset++;
	}
	BCM_REFERENCE(maxtargetpwr);
}

void
WLBANDINITFN(wlc_lcnphy_tx_pwr_ctrl_init)(wlc_phy_t *ppi)
{
	lcnphy_txgains_t tx_gains;
	uint8 bbmult;
	bool suspend;
	phy_info_t *pi = (phy_info_t *)ppi;
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;

	suspend = !(R_REG(pi->sh->osh, &pi->regs->maccontrol) & MCTL_EN_MAC);
	if (!suspend)
		wlapi_suspend_mac_and_wait(pi->sh->physhim);

	if (NORADIO_ENAB(pi->pubpi)) {
		wlc_lcnphy_set_bbmult(pi, 0x30);
		if (!suspend)
			wlapi_enable_mac(pi->sh->physhim);
		return;
	}

	if (!pi->hwpwrctrl_capable) {
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			tx_gains.gm_gain = 4;
			tx_gains.pga_gain = 12;
			tx_gains.pad_gain = 12;
			tx_gains.dac_gain = 0;

			bbmult = 150;
		} else {
			tx_gains.gm_gain = 255;
			tx_gains.pga_gain = 255;
			tx_gains.pad_gain = 0xf0;
			tx_gains.dac_gain = 0;

			bbmult = 150;
		}
		wlc_lcnphy_set_tx_gain(pi, &tx_gains);
		wlc_lcnphy_set_bbmult(pi, bbmult);
		wlc_lcnphy_vbat_temp_sense_setup(pi, TEMPSENSE);
	} else {

		wlc_lcnphy_idle_tssi_est(ppi);

		/* Clear out all power offsets */
		wlc_lcnphy_clear_tx_power_offsets(pi);

		/* Convert tssi to power LUT */
		wlc_lcnphy_set_estPwrLUT(pi, 0);

		PHY_REG_LIST_START_WLBANDINITDATA
			PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlRangeCmd, pwrMinMaxEnable, 0)
			PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlPwrMinMaxVal, pwrMinVal, 0)
			PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlPwrMinMaxVal, pwrMaxVal, 0)
			PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlRangeCmd, txGainTable_mode, 0)
			PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlRangeCmd, interpol_en, 0)
		PHY_REG_LIST_EXECUTE(pi);

#if TWO_POWER_RANGE_TXPWR_CTRL
		if (pi_lcn->lcnphy_twopwr_txpwrctrl_en) {
			wlc_lcnphy_set_estPwrLUT(pi, 1);

			PHY_REG_MOD(pi, LCNPHY, TxPwrCtrlPwrRange2, pwrMin_range2, pi_lcn->pmin);
			PHY_REG_MOD(pi, LCNPHY, TxPwrCtrlPwrRange2, pwrMax_range2, pi_lcn->pmax);

			PHY_REG_LIST_START_WLBANDINITDATA
				PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlRangeCmd, pwrMinMaxEnable2, 0)
				PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlPwrMinMaxVal2, pwrMinVal2, 0)
				PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlPwrMinMaxVal2, pwrMaxVal2, 0)
				PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlRangeCmd, interpol_en1, 0)
			PHY_REG_LIST_EXECUTE(pi);
		}
#endif /* TWO_POWER_RANGE_TXPWR_CTRL */

		PHY_REG_LIST_START_WLBANDINITDATA
			PHY_REG_MOD_ENTRY(LCNPHY, crsgainCtrl, crseddisable, 0)
			PHY_REG_WRITE_ENTRY(LCNPHY, TxPwrCtrlDeltaPwrLimit, 10)
		PHY_REG_LIST_EXECUTE(pi);

		wlc_lcnphy_set_target_tx_pwr(pi, LCN_TARGET_PWR);

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
			PHY_REG_MOD(pi, LCNPHY, TxPwrCtrlRangeCmd, cckPwrOffset, diff);

			/* program the tempsense curr in reg 0x50c */
			if (ofdmoffset >= 0)
				diff = ofdmoffset;
			else
				diff = 0x100 | ofdmoffset;
			PHY_REG_MOD(pi, LCNPHY, TempSenseCorrection, tempsenseCorr, diff);
		}
#endif /* WLNOKIA_NVMEM */

		if (CHSPEC_IS2G(pi->radio_chanspec))
			pi_lcn->lcnphy_tssi_idx = pi_lcn->init_txpwrindex_2g;
		else
			pi_lcn->lcnphy_tssi_idx = pi_lcn->init_txpwrindex_5g;

		ASSERT(pi_lcn->lcnphy_tssi_idx > 0);

		/* Enable hardware power control */
		wlc_lcnphy_set_tx_pwr_ctrl(pi, LCNPHY_TX_PWR_CTRL_HW);
	}

	pi_lcn->tempsensereg1 = PHY_REG_READ(pi, LCNPHY, TxPwrCtrlStatusTemp, avgTemp);
	pi_lcn->tempsensereg2 = PHY_REG_READ(pi, LCNPHY, TxPwrCtrlStatusTemp1, avgTemp2);

	if (pi_lcn->lcnphy_tssical_time)
		wlc_lcnphy_tssi_ucode_setup(pi, 1);

	if (!suspend)
		wlapi_enable_mac(pi->sh->physhim);
}

/* Save/Restore digital filter state OFDM filter settings */
static void
wlc_lcnphy_save_restore_dig_filt_state(phy_info_t *pi, bool save, uint16 *filtcoeffs)
{
	int j;

	uint16 addr_ofdm[] = {
		LCNPHY_txfilt20Stg1Shft,
		LCNPHY_txfilt20CoeffStg0A1,
		LCNPHY_txfilt20CoeffStg0A2,
		LCNPHY_txfilt20CoeffStg0B1,
		LCNPHY_txfilt20CoeffStg0B2,
		LCNPHY_txfilt20CoeffStg0B3,
		LCNPHY_txfilt20CoeffStg1A1,
		LCNPHY_txfilt20CoeffStg1A2,
		LCNPHY_txfilt20CoeffStg1B1,
		LCNPHY_txfilt20CoeffStg1B2,
		LCNPHY_txfilt20CoeffStg1B3,
		LCNPHY_txfilt20CoeffStg2A1,
		LCNPHY_txfilt20CoeffStg2A2,
		LCNPHY_txfilt20CoeffStg2B1,
		LCNPHY_txfilt20CoeffStg2B2,
		LCNPHY_txfilt20CoeffStg2B3,
		LCNPHY_txfilt20CoeffStg0_leftshift /* This coeff is specific to DAC160 */
		};
	/* Assume 80 MHz Digital filter by default */
	uint8 max_filter_coeffs = LCNPHY_NUM_DIG_FILT_COEFFS - 1;
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;

	/* If DAC160 then program the Stg0 Leftshift coeff as well, otherwise ignore that */
	if (pi_lcn->dacrate == 160)
		max_filter_coeffs = LCNPHY_NUM_DIG_FILT_COEFFS;

	if (save) {
		for (j = 0; j < max_filter_coeffs; j++)
			filtcoeffs[j] = phy_utils_read_phyreg(pi, addr_ofdm[j]);
	} else {
		for (j = 0; j < max_filter_coeffs; j++)
			phy_utils_write_phyreg(pi, addr_ofdm[j], filtcoeffs[j]);
	}

	if (wlc_lcnphy_tx_dig_iir_dc_err_comp(pi))
		wlc_lcnphy_adjust_frame_based_lo_coeffs(pi);
	if (!save) {
		/* Reset the iir filter after setting the coefficients */
		wlc_lcnphy_reset_iir_filter(pi);
	}
}

uint8
wlc_lcnphy_get_bbmult_from_index(phy_info_t *pi, int indx)
{
	phytbl_info_t tab;
	uint8 bb_mult;
	uint32 bbmultiqcomp;

	ASSERT(indx <= LCNPHY_MAX_TX_POWER_INDEX);

	/* Preset txPwrCtrltbl */
	tab.tbl_id = LCNPHY_TBL_ID_TXPWRCTL;
	tab.tbl_width = 32;	/* 32 bit wide	*/
	tab.tbl_len = 1;        /* # values   */

	/* Read index based bb_mult, a, b from the table */
	tab.tbl_offset = LCNPHY_TX_PWR_CTRL_IQ_OFFSET + indx; /* iqCoefLuts */
	tab.tbl_ptr = &bbmultiqcomp; /* ptr to buf */
	wlc_lcnphy_read_table(pi,  &tab);

	/* Apply bb_mult */
	bb_mult = (uint8)((bbmultiqcomp >> 20) & 0xff);

	return bb_mult;
}

uint8
wlc_lcnphy_get_bbmult(phy_info_t *pi)
{
	uint16 m0m1;
	phytbl_info_t tab;

	tab.tbl_ptr = &m0m1; /* ptr to buf */
	tab.tbl_len = 1;        /* # values   */
	tab.tbl_id = LCNPHY_TBL_ID_IQLOCAL;         /* iqloCaltbl      */
	tab.tbl_offset = 87; /* tbl offset */
	tab.tbl_width = 16;     /* 16 bit wide */
	wlc_lcnphy_read_table(pi, &tab);

	return (uint8)((m0m1 & 0xff00) >> 8);
}

static void
wlc_lcnphy_set_pa_gain(phy_info_t *pi, uint16 gain)
{
	PHY_REG_MOD(pi, LCNPHY, txgainctrlovrval1, pagain_ovr_val1, gain);
	PHY_REG_MOD(pi, LCNPHY, stxtxgainctrlovrval1, pagain_ovr_val1, gain);
}

/* force epa off */
static void
wlc_lcnphy_epa_pd(phy_info_t *pi, bool disable)
{
	if (!disable) {
		PHY_REG_MOD(pi, LCNPHY, swctrlOvr, swCtrl_ovr, 0);
	} else {
		PHY_REG_LIST_START
			PHY_REG_MOD_ENTRY(LCNPHY, swctrlOvr_val, swCtrl_ovr_val, 0)
			PHY_REG_MOD_ENTRY(LCNPHY, swctrlOvr, swCtrl_ovr, 0xff)
		PHY_REG_LIST_EXECUTE(pi);
	}
}

#define RADIO_REG_EI(pi) (CHSPEC_IS2G(pi->radio_chanspec)? RADIO_2064_REG089 : RADIO_2064_REG0A5)
#define RADIO_REG_EQ(pi) (CHSPEC_IS2G(pi->radio_chanspec)? RADIO_2064_REG08A : RADIO_2064_REG0A6)
#define RADIO_REG_FI(pi) (CHSPEC_IS2G(pi->radio_chanspec)? RADIO_2064_REG08B : RADIO_2064_REG0A7)
#define RADIO_REG_FQ(pi) (CHSPEC_IS2G(pi->radio_chanspec)? RADIO_2064_REG08C : RADIO_2064_REG0A8)

void
wlc_lcnphy_get_radio_loft(phy_info_t *pi,
	uint8 *ei0,
	uint8 *eq0,
	uint8 *fi0,
	uint8 *fq0)
{
	*ei0 = LCNPHY_IQLOCC_READ(
		phy_utils_read_radioreg(pi, RADIO_REG_EI(pi)));
	*eq0 = LCNPHY_IQLOCC_READ(
		phy_utils_read_radioreg(pi, RADIO_REG_EQ(pi)));
	*fi0 = LCNPHY_IQLOCC_READ(
		phy_utils_read_radioreg(pi, RADIO_REG_FI(pi)));
	*fq0 = LCNPHY_IQLOCC_READ(
		phy_utils_read_radioreg(pi, RADIO_REG_FQ(pi)));
}

static uint16
lcnphy_iqlocc_write(phy_info_t *pi, uint8 data)
{
	int32 data32 = (int8)data;
	int32 rf_data32;
	int32 ip, in;
	ip = 8 + (data32 >> 1);
	in = 8 - ((data32+1) >> 1);
	rf_data32 = (in << 4) | ip;
	return (uint16)(rf_data32);
}

void
wlc_lcnphy_set_radio_loft(phy_info_t *pi,
	uint8 ei0,
	uint8 eq0,
	uint8 fi0,
	uint8 fq0)
{
	phy_utils_write_radioreg(pi, RADIO_REG_EI(pi), lcnphy_iqlocc_write(pi, ei0));
	phy_utils_write_radioreg(pi, RADIO_REG_EQ(pi), lcnphy_iqlocc_write(pi, eq0));
	phy_utils_write_radioreg(pi, RADIO_REG_FI(pi), lcnphy_iqlocc_write(pi, fi0));
	phy_utils_write_radioreg(pi, RADIO_REG_FQ(pi), lcnphy_iqlocc_write(pi, fq0));
}

static void
wlc_lcnphy_get_tx_gain(phy_info_t *pi,  lcnphy_txgains_t *gains)
{
	uint16 dac_gain;

	dac_gain = phy_utils_read_phyreg(pi, LCNPHY_AfeDACCtrl) >>
		LCNPHY_AfeDACCtrl_dac_ctrl_SHIFT;
	gains->dac_gain = (dac_gain & 0x380) >> 7;

	{
		uint16 rfgain0, rfgain1;

		rfgain0 = (phy_utils_read_phyreg(pi, LCNPHY_txgainctrlovrval0) &
			LCNPHY_txgainctrlovrval0_txgainctrl_ovr_val0_MASK) >>
			LCNPHY_txgainctrlovrval0_txgainctrl_ovr_val0_SHIFT;
		rfgain1 = (phy_utils_read_phyreg(pi, LCNPHY_txgainctrlovrval1) &
			LCNPHY_txgainctrlovrval1_txgainctrl_ovr_val1_MASK) >>
			LCNPHY_txgainctrlovrval1_txgainctrl_ovr_val1_SHIFT;

		gains->gm_gain = rfgain0 & 0xff;
		gains->pga_gain = (rfgain0 >> 8) & 0xff;
		gains->pad_gain = rfgain1 & 0xff;
	}
}

/* enable per modulation LOFT compensation for dc introduced by tx digital IIR rounding error  */
static bool
wlc_lcnphy_tx_dig_iir_dc_err_comp(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;

	if (LCNREV_GE(pi->pubpi.phy_rev, 2) && CHSPEC_IS2G(pi->radio_chanspec)) {
		if (pi_lcn->loccmode1 != -1)
			return pi_lcn->loccmode1;
		else if (EPA(pi_lcn))
			return TRUE;
		else
			return FALSE;
	} else
		return FALSE;
}

/* Enabler to use per modulation (ie. ofdm vs. ht vs. cck) iq and lo coefficients */
static bool
wlc_lcnphy_frmTypBasedIqLoCoeff(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	/*
	need to enable frame based iqlo comp if:
		we're not using multiple gm gains in table, ie. gaintbl=1
		we're compensating for dig irr dc error or
		we're bypassing alpf differently for ofdm and cck
	*/
	if ((wlc_lcnphy_tx_dig_iir_dc_err_comp(pi) || pi_lcn->use_per_modulation_loft_cal) &&
	    (TXGAINTBL(pi_lcn) != 1) && (CHSPEC_IS2G(pi->radio_chanspec)))
		return TRUE;
	else
		return FALSE;
}

/* DC error introduced in tx digital iir due to rounding error */
#define LCNPHY_NUM_DC_VALS 50
static int16
wlc_lcnphy_tx_dig_iir_dc_val(phy_info_t *pi, uint8 is_ofdm, int16 filt_num)
{
	int8 dc[LCNPHY_NUM_DC_VALS], i;
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;

	/* Initializing the dc vals to zero */
	for (i = 0; i < LCNPHY_NUM_DC_VALS; i++)
		dc[i] = 0;

	if (filt_num == -1) {
		if (is_ofdm)
			filt_num = pi_lcn->lcnphy_ofdm_dig_filt_type_curr;
		else
			filt_num = pi_lcn->lcnphy_cck_dig_filt_type_curr;
	}

	if (is_ofdm) {
		if (pi_lcn->dacrate != 160) {
			dc[0] = 0;
			dc[1] = 0;
			dc[2] = 0;
			dc[3] = 0;
			dc[4] = 0;
		} else {
			dc[0] = 0;
			dc[2] = 0;
			dc[4] = 0;
		}
	} else {
		if (pi_lcn->dacrate != 160) {
			dc[20] = 1;
			dc[21] = 1;
			dc[22] = 0;
			dc[23] = 0;
			dc[24] = -2;
			dc[25] = -2;
			dc[26] = 0;
			dc[27] = -1;
			dc[30] = 0;
		} else {
			dc[20] = 2;
			dc[21] = 0;
			dc[22] = 0;
			dc[23] = 0;
			dc[24] = -4;
			dc[25] = -2;
			dc[26] = -2;
			dc[27] = -2;
			dc[30] = 0;
			dc[44] = 4;
		}
	}
	return dc[filt_num];
}

static void
wlc_lcnphy_adjust_frame_based_lo_coeffs(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	int16 di_ofdm, dq_ofdm, di_cck, dq_cck, ofdm_lo_offset, cck_lo_offset;
	int16 ofdm_dig_filt_type;

	if (!pi_lcn->use_per_modulation_loft_cal) {
		uint16 didq;
		int16 di, dq;
		didq = wlc_lcnphy_get_tx_locc(pi);
		di = (((didq & 0xff00) << 16) >> 24);
		dq = (((didq & 0x00ff) << 24) >> 24);

		di_ofdm = di;
		dq_ofdm = dq;
		di_cck = di;
		dq_cck = dq;
	} else {
		di_ofdm = pi_lcn->di_ofdm;
		dq_ofdm = pi_lcn->dq_ofdm;
		di_cck = pi_lcn->di_cck;
		dq_cck = pi_lcn->dq_cck;
	}

	if (CHSPEC_IS2G(pi->radio_chanspec))
		ofdm_dig_filt_type = pi_lcn->lcnphy_ofdm_dig_filt_type_2g;
	else
		ofdm_dig_filt_type = pi_lcn->lcnphy_ofdm_dig_filt_type_5g;

	ofdm_lo_offset = wlc_lcnphy_tx_dig_iir_dc_val(pi, TRUE, -1) -
		wlc_lcnphy_tx_dig_iir_dc_val(pi, TRUE, ofdm_dig_filt_type);
	cck_lo_offset =  wlc_lcnphy_tx_dig_iir_dc_val(pi, FALSE, -1) -
		wlc_lcnphy_tx_dig_iir_dc_val(pi, TRUE, ofdm_dig_filt_type);

	PHY_REG_MOD(pi, LCNPHY, dlo_coeff_di_ofdm, dlo_coeff_di_ofdm, (di_ofdm - ofdm_lo_offset));
	PHY_REG_MOD(pi, LCNPHY, dlo_coeff_dq_ofdm, dlo_coeff_dq_ofdm, (dq_ofdm - ofdm_lo_offset));

	PHY_REG_MOD(pi, LCNPHY, dlo_coeff_di_ht, dlo_coeff_di_ht, (di_ofdm - ofdm_lo_offset));
	PHY_REG_MOD(pi, LCNPHY, dlo_coeff_dq_ht, dlo_coeff_dq_ht, (dq_ofdm - ofdm_lo_offset));

	PHY_REG_MOD(pi, LCNPHY, dlo_coeff_di_cck, dlo_coeff_di_cck, (di_cck - cck_lo_offset));
	PHY_REG_MOD(pi, LCNPHY, dlo_coeff_dq_cck, dlo_coeff_dq_cck, (dq_cck - cck_lo_offset));
}

void
wlc_lcnphy_set_tx_iqcc(phy_info_t *pi, uint16 a, uint16 b)
{
	phytbl_info_t tab;
	uint16 iqcc[2];

	/* Fill buffer with coeffs */
	iqcc[0] = a;
	iqcc[1] = b;
	/* Update iqloCaltbl */
	tab.tbl_id = LCNPHY_TBL_ID_IQLOCAL;			/* iqloCaltbl	*/
	tab.tbl_width = 16;	/* 16 bit wide	*/
	tab.tbl_ptr = iqcc;
	tab.tbl_len = 2;
	tab.tbl_offset = 80;
	wlc_lcnphy_write_table(pi, &tab);

	if (wlc_lcnphy_frmTypBasedIqLoCoeff(pi)) {
		PHY_REG_MOD(pi, LCNPHY, iq_coeff_a_cck, iq_coeff_a_cck, a);
		PHY_REG_MOD(pi, LCNPHY, iq_coeff_b_cck, iq_coeff_b_cck, b);

		PHY_REG_MOD(pi, LCNPHY, iq_coeff_a_ofdm, iq_coeff_a_ofdm, a);
		PHY_REG_MOD(pi, LCNPHY, iq_coeff_b_ofdm, iq_coeff_b_ofdm, b);

		PHY_REG_MOD(pi, LCNPHY, iq_coeff_a_ht, iq_coeff_a_ht, a);
		PHY_REG_MOD(pi, LCNPHY, iq_coeff_b_ht, iq_coeff_b_ht, b);
	}
}

void
wlc_lcnphy_set_tx_locc(phy_info_t *pi, uint16 didq)
{
	phytbl_info_t tab;

	/* Update iqloCaltbl */
	tab.tbl_id = LCNPHY_TBL_ID_IQLOCAL;			/* iqloCaltbl	*/
	tab.tbl_width = 16;	/* 16 bit wide	*/
	tab.tbl_ptr = &didq;
	tab.tbl_len = 1;
	tab.tbl_offset = 85;
	wlc_lcnphy_write_table(pi, &tab);

	if (wlc_lcnphy_tx_dig_iir_dc_err_comp(pi))
		wlc_lcnphy_adjust_frame_based_lo_coeffs(pi);
}

void
wlc_lcnphy_set_tx_pwr_by_index(phy_info_t *pi, int indx)
{
	/* Turn off automatic power control */
	wlc_lcnphy_set_tx_pwr_ctrl(pi, LCNPHY_TX_PWR_CTRL_OFF);

	/* Force tx power from the index */
	wlc_lcnphy_force_pwr_index(pi, indx);
}

static void
wlc_lcnphy_force_pwr_index(phy_info_t *pi, int indx)
{
	phytbl_info_t tab;
	uint16 a, b;
	uint8 bb_mult, sw_index, papdlut;
	uint32 bbmultiqcomp, txgain, locoeffs, rfpower;
	lcnphy_txgains_t gains;
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;

	ASSERT(indx <= LCNPHY_MAX_TX_POWER_INDEX);
	sw_index = (uint8) indx;
	if (pi_lcn->virtual_p25_tx_gain_step) {
		if (indx % 2 == 1)
			PHY_REG_MOD(pi, LCNPHY, bbShiftCtrl, bbshift, 2);
		else
			PHY_REG_MOD(pi, LCNPHY, bbShiftCtrl, bbshift, 0);
		indx = indx/2;
	}

	pi_lcn->lcnphy_tx_power_idx_override = sw_index;
	pi_lcn->lcnphy_current_index = sw_index;

	/* Preset txPwrCtrltbl */
	tab.tbl_id = LCNPHY_TBL_ID_TXPWRCTL;
	tab.tbl_width = 32;	/* 32 bit wide	*/
	tab.tbl_len = 1;        /* # values   */

	/* Read index based bb_mult, a, b from the table */
	tab.tbl_offset = LCNPHY_TX_PWR_CTRL_IQ_OFFSET + indx; /* iqCoefLuts */
	tab.tbl_ptr = &bbmultiqcomp; /* ptr to buf */
	wlc_lcnphy_read_table(pi,  &tab);

	/* Read index based tx gain from the table */
	tab.tbl_offset = LCNPHY_TX_PWR_CTRL_GAIN_OFFSET + indx; /* gainCtrlLuts */
	tab.tbl_width = 32;
	tab.tbl_ptr = &txgain; /* ptr to buf */
	wlc_lcnphy_read_table(pi,  &tab);
	/* Apply tx gain */
	gains.gm_gain = (uint16)(txgain & 0xff);
	gains.pga_gain = (uint16)(txgain >> 8) & 0xff;
	gains.pad_gain = (uint16)(txgain >> 16) & 0xff;
	gains.dac_gain = (uint16)(bbmultiqcomp >> 28) & 0x07;
	wlc_lcnphy_set_tx_gain(pi, &gains);
	wlc_lcnphy_set_pa_gain(pi,  (uint16)(txgain >> 24) & 0x7f);
	/* Apply bb_mult */
	bb_mult = (uint8)((bbmultiqcomp >> 20) & 0xff);
	wlc_lcnphy_set_bbmult(pi, bb_mult);
	/* Enable gain overrides */
	wlc_lcnphy_enable_tx_gain_override(pi);
	/* the reading and applying lo, iqcc coefficients is not getting done for 4313A0 */
	/* to be fixed */

	if (!wlc_lcnphy_tempsense_based_pwr_ctrl_enabled(pi)) {
		/* Apply iqcc */
		a = (uint16)((bbmultiqcomp >> 10) & 0x3ff);
		b = (uint16)(bbmultiqcomp & 0x3ff);
		wlc_lcnphy_set_tx_iqcc(pi, a, b);
		/* Read index based di & dq from the table */
		tab.tbl_offset = LCNPHY_TX_PWR_CTRL_LO_OFFSET + indx; /* loftCoefLuts */
		tab.tbl_ptr = &locoeffs; /* ptr to buf */
		wlc_lcnphy_read_table(pi,  &tab);
		/* Apply locc */
		wlc_lcnphy_set_tx_locc(pi, (uint16)locoeffs);
		/* Apply PAPD rf power correction */
		tab.tbl_offset = LCNPHY_TX_PWR_CTRL_PWR_OFFSET + indx;
		tab.tbl_ptr = &rfpower; /* ptr to buf */
		wlc_lcnphy_read_table(pi,  &tab);
		PHY_REG_MOD(pi, LCNPHY, papd_analog_gain_ovr_val,
			papd_analog_gain_ovr_val, rfpower * 8);
	}

	/* PAPD lut sel */
	if (pi_lcn->papd_num_lut_used > 1) {
		if (indx > PHY_REG_READ(pi, LCNPHY, PapdDupLutCtrl, txgainidxThreshold))
			papdlut = 1;
		else
			papdlut = 0;
		PHY_REG_MOD(pi, LCNPHY, PapdDupLutCtrl, PapdLutSel0, papdlut);
		PHY_REG_MOD(pi, LCNPHY, PapdDupLutCtrl, PapdLutSel1, papdlut);
		PHY_REG_MOD(pi, LCNPHY, PapdDupLutCtrl, PapdLutSel2, papdlut);
		PHY_REG_MOD(pi, LCNPHY, PapdDupLutCtrl, PapdLutSel3, papdlut);
	}
}

static void
wlc_lcnphy_set_trsw_override(phy_info_t *pi, bool tx, bool rx)
{
	/* Set TR switch */
	PHY_REG_MOD2(pi, LCNPHY, RFOverrideVal0, trsw_tx_pu_ovr_val, trsw_rx_pu_ovr_val,
		(tx ? 1 : 0), (rx ? 1 : 0));

	/* Enable overrides */
	PHY_REG_OR(pi, LCNPHY, RFOverride0,
		LCNPHY_RFOverride0_trsw_tx_pu_ovr_MASK |
		LCNPHY_RFOverride0_trsw_rx_pu_ovr_MASK);
}

void
wlc_lcnphy_read_papdepstbl(phy_info_t *pi, struct bcmstrbuf *b)
{
	phytbl_info_t tab;
	uint32 val, j;
	int32 eps_real, eps_imag;
	/* Save epsilon table */
	tab.tbl_id = LCNPHY_TBL_ID_PAPDCOMPDELTATBL;
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
wlc_lcnphy_clear_papd_comptable(phy_info_t *pi)
{
	phytbl_info_t tab;
	uint8 j;
	uint32 papdcompdeltatbl_init_val;
	tab.tbl_ptr = &papdcompdeltatbl_init_val; /* ptr to buf */
	tab.tbl_len = 1;        /* # values   */
	tab.tbl_id = LCNPHY_TBL_ID_PAPDCOMPDELTATBL;         /* papdcompdeltatbl */
	tab.tbl_width = 32;     /* 32 bit wide */
	tab.tbl_offset = 0; /* tbl offset */

	papdcompdeltatbl_init_val = 0x80000; /* lut init val */

	for (j = 0; j < 128; j++) {
	    wlc_lcnphy_write_table(pi, &tab);
	    tab.tbl_offset++;
	}
	return;
}

static void
wlc_lcnphy_set_rx_gain_by_distribution(phy_info_t *pi,
	uint16 trsw,
	uint16 ext_lna,
	uint16 biq2,
	uint16 biq1,
	uint16 tia,
	uint16 lna2,
	uint16 lna1,
	uint16 digi)
{
	uint16 gain0_15, gain16_19;

	gain16_19 = biq2 & 0xf;
	gain0_15 = ((biq1 & 0xf) << 12) |
		((tia & 0xf) << 8) |
		((lna2 & 0x3) << 6) |
		((lna2 & 0x3) << 4) |
		((lna1 & 0x3) << 2) |
		((lna1 & 0x3) << 0);

	PHY_REG_MOD(pi, LCNPHY, rxgainctrl0ovrval, rxgainctrl_ovr_val0,	gain0_15);
	PHY_REG_MOD(pi, LCNPHY, rxlnaandgainctrl1ovrval, rxgainctrl_ovr_val1, gain16_19);
	if (LCNREV_LT(pi->pubpi.phy_rev, 2)) {
		PHY_REG_MOD(pi, LCNPHY, rfoverride2val, slna_gain_ctrl_ovr_val, lna1);
	} else {
		PHY_REG_MOD(pi, LCNPHY, rfoverride2val, slna_gain_ctrl_ovr_val_rev_ge2, lna1);
	}

	if (LCNREV_LT(pi->pubpi.phy_rev, 2)) {
		PHY_REG_MOD(pi, LCNPHY, rfoverride2val, gmode_ext_lna_gain_ovr_val, ext_lna);
		PHY_REG_MOD(pi, LCNPHY, rfoverride2val,	amode_ext_lna_gain_ovr_val, ext_lna);
	}
	else {
		PHY_REG_LIST_START
			PHY_REG_MOD_ENTRY(LCNPHY, rfoverride2val, slna_mode_ovr_val, 0)
			PHY_REG_MOD_ENTRY(LCNPHY, rfoverride2val, slna_byp_ovr_val, 0)
		PHY_REG_LIST_EXECUTE(pi);
		PHY_REG_MOD(pi, LCNPHY, RxRadioControl, digi_gain_ovr_val, digi);
		PHY_REG_MOD(pi, LCNPHY, rfoverride2val, ext_lna_gain_ovr_val, ext_lna);
	}

	PHY_REG_MOD(pi, LCNPHY, RFOverrideVal0,	trsw_rx_pu_ovr_val, (!trsw));

	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		PHY_REG_MOD(pi, LCNPHY, rfoverride2val, slna_gain_ctrl_ovr_val, lna1);
		PHY_REG_MOD(pi, LCNPHY, RFinputOverrideVal, wlslnagainctrl_ovr_val, lna1);
	}

	PHY_REG_MOD(pi, LCNPHY, RxRadioControl, digi_gain_ovr_val, digi);

}

static void
wlc_lcnphy_rx_gain_override_enable(phy_info_t *pi, bool enable)
{
	uint16 ebit = enable ? 1 : 0;

	PHY_REG_MOD(pi, LCNPHY, rfoverride2, rxgainctrl_ovr, ebit);
	PHY_REG_MOD(pi, LCNPHY, RFOverride0, trsw_rx_pu_ovr, ebit);

	if (LCNREV_LT(pi->pubpi.phy_rev, 2)) {
		PHY_REG_MOD(pi, LCNPHY, RFOverride0, gmode_rx_pu_ovr, ebit);
		PHY_REG_MOD(pi, LCNPHY, RFOverride0, amode_rx_pu_ovr, ebit);
		PHY_REG_MOD(pi, LCNPHY, rfoverride2, gmode_ext_lna_gain_ovr, ebit);
		PHY_REG_MOD(pi, LCNPHY, rfoverride2, amode_ext_lna_gain_ovr, ebit);
	}
	else {
		PHY_REG_MOD(pi, LCNPHY, rfoverride2, slna_byp_ovr, ebit);
		PHY_REG_MOD(pi, LCNPHY, rfoverride2, slna_mode_ovr, ebit);
		PHY_REG_MOD(pi, LCNPHY, rfoverride2, ext_lna_gain_ovr, ebit);
		PHY_REG_MOD(pi, LCNPHY, RxRadioControl, digi_gain_ovr, ebit);

	}

	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		PHY_REG_MOD(pi, LCNPHY, rfoverride2, slna_gain_ctrl_ovr, ebit);
		PHY_REG_MOD(pi, LCNPHY, RFinputOverride, wlslnagainctrl_ovr, ebit);
	}
}

static void
wlc_lcnphy_rx_pu(phy_info_t *pi, bool bEnable)
{
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	if (!bEnable) {
		PHY_REG_LIST_START
			PHY_REG_MOD_ENTRY(LCNPHY, RFOverride0, internalrfrxpu_ovr, 1)
			PHY_REG_MOD_ENTRY(LCNPHY, RFOverrideVal0, internalrfrxpu_ovr_val, 0)
			PHY_REG_MOD_ENTRY(LCNPHY, rfoverride2, rxgainctrl_ovr, 1)
			PHY_REG_WRITE_ENTRY(LCNPHY, rxgainctrl0ovrval, 0xffff)
			PHY_REG_MOD_ENTRY(LCNPHY, rxlnaandgainctrl1ovrval,
				rxgainctrl_ovr_val1, 0xf)
		PHY_REG_LIST_EXECUTE(pi);
	} else {
		/* Force on the receive chain */
		PHY_REG_LIST_START
			PHY_REG_MOD_ENTRY(LCNPHY, RFOverride0, internalrfrxpu_ovr, 1)
			PHY_REG_MOD_ENTRY(LCNPHY, RFOverrideVal0, internalrfrxpu_ovr_val, 1)
			PHY_REG_MOD_ENTRY(LCNPHY, rfoverride2, rxgainctrl_ovr, 1)
		PHY_REG_LIST_EXECUTE(pi);

		wlc_lcnphy_set_rx_gain_by_distribution(pi, 0, 0, 4, 6, 4, 3, 3, 0);
		wlc_lcnphy_rx_gain_override_enable(pi, TRUE);
	}
}

void
wlc_lcnphy_tx_pu(phy_info_t *pi, bool bEnable)
{
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	if (!bEnable) {
		PHY_REG_LIST_START
			/* Clear overrides */
			PHY_REG_AND_ENTRY(LCNPHY, AfeCtrlOvr, ~(uint16)
				(LCNPHY_AfeCtrlOvr_pwdn_dac_ovr_MASK |
				 LCNPHY_AfeCtrlOvr_dac_clk_disable_ovr_MASK))
			PHY_REG_MOD_ENTRY(LCNPHY, AfeCtrlOvrVal, pwdn_dac_ovr_val, 1)
			PHY_REG_AND_ENTRY(LCNPHY, RFOverride0,
				~(uint16)(LCNPHY_RFOverride0_gmode_tx_pu_ovr_MASK |
				LCNPHY_RFOverride0_amode_tx_pu_ovr_MASK |
				LCNPHY_RFOverride0_internalrftxpu_ovr_MASK |
				LCNPHY_RFOverride0_trsw_rx_pu_ovr_MASK |
				LCNPHY_RFOverride0_trsw_tx_pu_ovr_MASK |
				LCNPHY_RFOverride0_ant_selp_ovr_MASK))
			PHY_REG_AND_ENTRY(LCNPHY, RFOverrideVal0,
				~(uint16)(LCNPHY_RFOverrideVal0_gmode_tx_pu_ovr_val_MASK |
				LCNPHY_RFOverrideVal0_amode_tx_pu_ovr_val_MASK |
				LCNPHY_RFOverrideVal0_internalrftxpu_ovr_val_MASK))
			PHY_REG_MOD_ENTRY(LCNPHY, RFOverrideVal0, ant_selp_ovr_val, 1)
			/* Set TR switch */
			PHY_REG_MOD2_ENTRY(LCNPHY, RFOverrideVal0,
				trsw_tx_pu_ovr_val, trsw_rx_pu_ovr_val,
				0, 1)
			PHY_REG_AND_ENTRY(LCNPHY, rfoverride3,
				~(uint16)(LCNPHY_rfoverride3_stxpapu_ovr_MASK |
				LCNPHY_rfoverride3_stxpadpu2g_ovr_MASK |
				LCNPHY_rfoverride3_stxpapu2g_ovr_MASK))
			PHY_REG_AND_ENTRY(LCNPHY, rfoverride3_val,
				~(uint16)(LCNPHY_rfoverride3_val_stxpapu_ovr_val_MASK |
				LCNPHY_rfoverride3_val_stxpadpu2g_ovr_val_MASK |
				LCNPHY_rfoverride3_val_stxpapu2g_ovr_val_MASK))
		PHY_REG_LIST_EXECUTE(pi);
	} else {
		PHY_REG_LIST_START
			/* Force on DAC */
			PHY_REG_MOD_ENTRY(LCNPHY, AfeCtrlOvr, pwdn_dac_ovr, 1)
			PHY_REG_MOD_ENTRY(LCNPHY, AfeCtrlOvrVal, pwdn_dac_ovr_val, 0)
			PHY_REG_MOD_ENTRY(LCNPHY, AfeCtrlOvr, dac_clk_disable_ovr, 1)
			PHY_REG_MOD_ENTRY(LCNPHY, AfeCtrlOvrVal, dac_clk_disable_ovr_val, 0)
			/* Force on the transmit chain */
			PHY_REG_MOD_ENTRY(LCNPHY, RFOverride0, internalrftxpu_ovr, 1)
			PHY_REG_MOD_ENTRY(LCNPHY, RFOverrideVal0, internalrftxpu_ovr_val, 1)
		PHY_REG_LIST_EXECUTE(pi);

		/* Force the TR switch to transmit */
		wlc_lcnphy_set_trsw_override(pi, TRUE, FALSE);

		PHY_REG_LIST_START
			/* Force antenna  0 */
			PHY_REG_MOD_ENTRY(LCNPHY, RFOverrideVal0, ant_selp_ovr_val, 0)
			PHY_REG_MOD_ENTRY(LCNPHY, RFOverride0, ant_selp_ovr, 1)
		PHY_REG_LIST_EXECUTE(pi);

		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			PHY_REG_LIST_START
				/* Force on the Gband-ePA */
				PHY_REG_MOD_ENTRY(LCNPHY, RFOverride0, gmode_tx_pu_ovr, 1)
				PHY_REG_MOD_ENTRY(LCNPHY, RFOverrideVal0, gmode_tx_pu_ovr_val, 1)
				/* force off the Aband-ePA */
				PHY_REG_MOD_ENTRY(LCNPHY, RFOverride0, amode_tx_pu_ovr, 1)
				PHY_REG_MOD_ENTRY(LCNPHY, RFOverrideVal0, amode_tx_pu_ovr_val, 0)
				/* PAD PU */
				PHY_REG_MOD_ENTRY(LCNPHY, rfoverride3, stxpadpu2g_ovr, 1)
				PHY_REG_MOD_ENTRY(LCNPHY, rfoverride3_val, stxpadpu2g_ovr_val, 1)
				/* PGA PU */
				PHY_REG_MOD_ENTRY(LCNPHY, rfoverride3, stxpapu2g_ovr, 1)
				PHY_REG_MOD_ENTRY(LCNPHY, rfoverride3_val, stxpapu2g_ovr_val, 1)
				/* PA PU */
				PHY_REG_MOD_ENTRY(LCNPHY, rfoverride3, stxpapu_ovr, 1)
				PHY_REG_MOD_ENTRY(LCNPHY, rfoverride3_val, stxpapu_ovr_val, 1)
			PHY_REG_LIST_EXECUTE(pi);
		} else {
			PHY_REG_LIST_START
				/* Force off the Gband-ePA */
				PHY_REG_MOD_ENTRY(LCNPHY, RFOverride0, gmode_tx_pu_ovr, 1)
				PHY_REG_MOD_ENTRY(LCNPHY, RFOverrideVal0, gmode_tx_pu_ovr_val, 0)
				/* force off the Aband-ePA */
				PHY_REG_MOD_ENTRY(LCNPHY, RFOverride0, amode_tx_pu_ovr, 1)
				PHY_REG_MOD_ENTRY(LCNPHY, RFOverrideVal0, amode_tx_pu_ovr_val, 1)
				/* PAP PU */
				PHY_REG_MOD_ENTRY(LCNPHY, rfoverride3, stxpadpu2g_ovr, 1)
				PHY_REG_MOD_ENTRY(LCNPHY, rfoverride3_val, stxpadpu2g_ovr_val, 0)
				/* PGA PU */
				PHY_REG_MOD_ENTRY(LCNPHY, rfoverride3, stxpapu2g_ovr, 1)
				PHY_REG_MOD_ENTRY(LCNPHY, rfoverride3_val, stxpapu2g_ovr_val, 0)
				/* PA PU */
				PHY_REG_MOD_ENTRY(LCNPHY, rfoverride3, stxpapu_ovr, 1)
				PHY_REG_MOD_ENTRY(LCNPHY, rfoverride3_val, stxpapu_ovr_val, 0)
			PHY_REG_LIST_EXECUTE(pi);
		}
	}
}

/*
 * Play samples from sample play buffer
 */
static void
wlc_lcnphy_run_samples(phy_info_t *pi,
	uint16 num_samps,
	uint16 num_loops,
	uint16 wait,
	bool iqcalmode)
{
	/* enable clk to txFrontEnd */
	PHY_REG_OR(pi, LCNPHY, sslpnCalibClkEnCtrl, 0x8080);

	if (wlc_lcnphy_frmTypBasedIqLoCoeff(pi)) {
		/* in hardware spb uses cck coeffs, but ofdm digital filter;
		     Use ofdm for both to be consistent
		*/
		PHY_REG_MOD(pi, LCNPHY, dlo_coeff_di_cck, dlo_coeff_di_cck,
			(phy_utils_read_phyreg(pi, LCNPHY_dlo_coeff_di_ofdm) & 0xff));
		PHY_REG_MOD(pi, LCNPHY, dlo_coeff_dq_cck, dlo_coeff_dq_cck,
			(phy_utils_read_phyreg(pi, LCNPHY_dlo_coeff_dq_ofdm) & 0xff));
	}

	/* rtl uses cck setting for alpf bypass, but ofdm dig filter; use ofdm setting for
	    both to be consistent
	*/
	if (pi->u.pi_lcnphy->use_per_modulation_loft_cal)
		PHY_REG_MOD(pi, LCNPHY, lpfbwlutreg1, lpf_cck_tx_byp,
			PHY_REG_READ(pi, LCNPHY, lpfbwlutreg1, lpf_ofdm_tx_byp));

	PHY_REG_MOD(pi, LCNPHY, sampleDepthCount, DepthCount, (num_samps - 1));
	if (num_loops != 0xffff)
		num_loops--;

		PHY_REG_MOD(pi, LCNPHY, sampleLoopCount, LoopCount, num_loops);
	PHY_REG_MOD(pi, LCNPHY, sampleInitWaitCount, InitWaitCount, wait);

	if (iqcalmode) {
		/* Enable calibration */
		PHY_REG_LIST_START
			PHY_REG_AND_ENTRY(LCNPHY, iqloCalCmdGctl,
				(uint16)~LCNPHY_iqloCalCmdGctl_iqlo_cal_en_MASK)
			PHY_REG_OR_ENTRY(LCNPHY, iqloCalCmdGctl,
				LCNPHY_iqloCalCmdGctl_iqlo_cal_en_MASK)
		PHY_REG_LIST_EXECUTE(pi);
	} else {
		PHY_REG_WRITE(pi, LCNPHY, sampleCmd, 1);
		wlc_lcnphy_tx_pu(pi, 1);
	}
	/* enable filter override */
	if (LCNREV_LT(pi->pubpi.phy_rev, 1)) {
		phy_utils_or_radioreg(pi, RADIO_2064_REG112, 0x6);
	}
}

void
wlc_lcnphy_deaf_mode(phy_info_t *pi, bool mode)
{

	uint8 phybw40;
	phybw40 = CHSPEC_IS40(pi->radio_chanspec);

	phy_utils_phyreg_enter(pi);

	if (LCNREV_LT(pi->pubpi.phy_rev, 2)) {
		PHY_REG_MOD(pi, LCNPHY, rfoverride2, gmode_ext_lna_gain_ovr, mode);
		PHY_REG_MOD(pi, LCNPHY, rfoverride2val,	gmode_ext_lna_gain_ovr_val, 0);
	}
	else
	{
		PHY_REG_MOD(pi, LCNPHY, rfoverride2, ext_lna_gain_ovr, mode);
		PHY_REG_MOD(pi, LCNPHY, rfoverride2val, ext_lna_gain_ovr_val, 0);
	}

	if (phybw40 == 0) {
		PHY_REG_MOD2(pi, LCNPHY, crsgainCtrl, DSSSDetectionEnable, OFDMDetectionEnable,
			((CHSPEC_IS2G(pi->radio_chanspec)) ? !mode : 0), !mode);
		PHY_REG_MOD(pi, LCNPHY, crsgainCtrl, crseddisable, mode);
	}

	phy_utils_phyreg_exit(pi);
}

/*
* Given a test tone frequency, continuously play the samples. Ensure that num_periods
* specifies the number of periods of the underlying analog signal over which the
* digital samples are periodic
*/
/* equivalent to lcnphy_play_tone */
void
wlc_lcnphy_start_tx_tone(phy_info_t *pi, int32 f_Hz, uint16 max_val, bool iqcalmode)
{
	uint8 phy_bw;
	uint16 num_samps, t, k;
	uint32 bw;
	math_fixed theta = 0, rot = 0;
	math_cint32 tone_samp;
	uint32 *data_buf;
	uint16 i_samp, q_samp;
	phytbl_info_t tab;
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;

	if ((data_buf = MALLOC(pi->sh->osh, sizeof(uint32) * 256)) == NULL) {
		PHY_ERROR(("wl%d: %s: MALLOC failure\n", pi->sh->unit, __FUNCTION__));
		return;
	}

	/* Save active tone frequency */
	pi->phy_tx_tone_freq = f_Hz;

	/* Turn off all the crs signals to the MAC */
	wlc_lcnphy_deaf_mode(pi, TRUE);

	phy_bw = 40;
	if (pi_lcn->lcnphy_spurmod) {
			PHY_REG_LIST_START
				PHY_REG_WRITE_ENTRY(LCNPHY, lcnphy_clk_muxsel1, 0x2)
				PHY_REG_WRITE_ENTRY(LCNPHY, spur_canceller1, 0x0)
				PHY_REG_WRITE_ENTRY(LCNPHY, spur_canceller2, 0x0)
			PHY_REG_LIST_EXECUTE(pi);

			wlc_lcnphy_txrx_spur_avoidance_mode(pi, FALSE);
	}
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

	PHY_INFORM(("wl%d: %s: %d Hz, %d samples\n",
		pi->sh->unit, __FUNCTION__,
		f_Hz, num_samps));

	if (num_samps > 256) {
		PHY_ERROR(("wl%d: %s: Too many samples to fit in SPB\n",
			pi->sh->unit, __FUNCTION__));
		MFREE(pi->sh->osh, data_buf, 256 * sizeof(uint32));
		return;
	}

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

	PHY_REG_LIST_START
		/* in LCNPHY, we need to bring SPB out of standby before using it */
		PHY_REG_MOD_ENTRY(LCNPHY, sslpnCtrl3, sram_stby, 0)
		PHY_REG_MOD_ENTRY(LCNPHY, sslpnCalibClkEnCtrl, samplePlayClkEn, 1)
	PHY_REG_LIST_EXECUTE(pi);

	/* lcnphy_load_sample_table */
	tab.tbl_ptr = data_buf;
	tab.tbl_len = num_samps;
	tab.tbl_id = LCNPHY_TBL_ID_SAMPLEPLAY;
	tab.tbl_offset = 0;
	tab.tbl_width = 32;
	wlc_lcnphy_write_table(pi, &tab);
	/* play samples from the sample play buffer */
	wlc_lcnphy_run_samples(pi, num_samps, 0xffff, 0, iqcalmode);
	MFREE(pi->sh->osh, data_buf, 256 * sizeof(uint32));
}

void
wlc_lcnphy_stop_tx_tone(phy_info_t *pi)
{
	int16 playback_status;
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;

	pi->phy_tx_tone_freq = 0;
	if (pi_lcn->lcnphy_spurmod) {
		PHY_REG_LIST_START
			PHY_REG_WRITE_ENTRY(LCNPHY, lcnphy_clk_muxsel1, 0x7)
			PHY_REG_WRITE_ENTRY(LCNPHY, spur_canceller1, 0x2017)
			PHY_REG_WRITE_ENTRY(LCNPHY, spur_canceller2, 0x27c5)
		PHY_REG_LIST_EXECUTE(pi);

		wlc_lcnphy_txrx_spur_avoidance_mode(pi, TRUE);
	}
	/* Stop sample buffer playback */
	playback_status = phy_utils_read_phyreg(pi, LCNPHY_sampleStatus);
	if (playback_status & LCNPHY_sampleStatus_NormalPlay_MASK) {
		wlc_lcnphy_tx_pu(pi, 0);
		PHY_REG_MOD(pi, LCNPHY, sampleCmd, stop, 1);
	} else if (playback_status & LCNPHY_sampleStatus_iqlocalPlay_MASK) {
		PHY_REG_MOD(pi, LCNPHY, iqloCalCmdGctl, iqlo_cal_en, 0);
	}

	PHY_REG_LIST_START
		/* put back SPB into standby */
		PHY_REG_MOD_ENTRY(LCNPHY, sslpnCtrl3, sram_stby, 1)
		/* disable clokc to spb */
		PHY_REG_MOD_ENTRY(LCNPHY, sslpnCalibClkEnCtrl, samplePlayClkEn, 0)
		/* disable clock to txFrontEnd */
		PHY_REG_MOD_ENTRY(LCNPHY, sslpnCalibClkEnCtrl, forceaphytxFeclkOn, 0)
	PHY_REG_LIST_EXECUTE(pi);

	/* disable filter override */
	if (LCNREV_LT(pi->pubpi.phy_rev, 1)) {
		phy_utils_and_radioreg(pi, RADIO_2064_REG112, 0xFFF9);
	}
	/* Restore all the crs signals to the MAC */
	wlc_lcnphy_deaf_mode(pi, FALSE);

	/* Restore per modulation lo coeffs */
	if (wlc_lcnphy_frmTypBasedIqLoCoeff(pi)) {
		uint16 didq;
		didq = wlc_lcnphy_get_tx_locc(pi);
		wlc_lcnphy_set_tx_locc(pi, didq);
	}

	PHY_REG_MOD(pi, LCNPHY, lpfbwlutreg1, lpf_cck_tx_byp,
		pi_lcn->lpf_cck_tx_byp);

	if (pi_lcn->di_cck) {
		PHY_REG_MOD(pi, LCNPHY, dlo_coeff_di_cck, dlo_coeff_di_cck,
			pi_lcn->di_cck);
		PHY_REG_MOD(pi, LCNPHY, dlo_coeff_dq_cck, dlo_coeff_dq_cck,
			pi_lcn->dq_cck);
	}
}

void
wlc_lcnphy_set_tx_tone_and_gain_idx(phy_info_t *pi)
{
	int8 curr_pwr_idx_val;

	/* Force WLAN antenna */
	wlc_btcx_override_enable(pi);

	if (LCNPHY_TX_PWR_CTRL_OFF != wlc_lcnphy_get_tx_pwr_ctrl(pi)) {
		curr_pwr_idx_val = wlc_lcnphy_get_current_tx_pwr_idx(pi);
		wlc_lcnphy_set_tx_pwr_by_index(pi, (int)curr_pwr_idx_val);
	} else {
		/* if tx power ctrl was disabled, then gain index already set */
	}

	PHY_REG_WRITE(pi, LCNPHY, sslpnCalibClkEnCtrl, 0xffff);
	wlc_lcnphy_start_tx_tone(pi, pi->phy_tx_tone_freq, 120, 1); /* play tone */
}

static void
wlc_lcnphy_clear_trsw_override(phy_info_t *pi)
{
	/* Clear overrides */
	PHY_REG_AND(pi, LCNPHY, RFOverride0, (uint16)~(LCNPHY_RFOverride0_trsw_tx_pu_ovr_MASK |
		LCNPHY_RFOverride0_trsw_rx_pu_ovr_MASK));
}

void
wlc_lcnphy_get_tx_iqcc(phy_info_t *pi, uint16 *a, uint16 *b)
{
	uint16 iqcc[2];
	phytbl_info_t tab;

	tab.tbl_ptr = iqcc; /* ptr to buf */
	tab.tbl_len = 2;        /* # values   */
	tab.tbl_id = 0;         /* iqloCaltbl      */
	tab.tbl_offset = 80; /* tbl offset */
	tab.tbl_width = 16;     /* 16 bit wide */
	wlc_lcnphy_read_table(pi, &tab);

	*a = iqcc[0];
	*b = iqcc[1];
}

uint16
wlc_lcnphy_get_tx_locc(phy_info_t *pi)
{
	phytbl_info_t tab;
	uint16 didq;

	/* Update iqloCaltbl */
	tab.tbl_id = 0;			/* iqloCaltbl		*/
	tab.tbl_width = 16;	/* 16 bit wide	*/
	tab.tbl_ptr = &didq;
	tab.tbl_len = 1;
	tab.tbl_offset = 85;
	wlc_lcnphy_read_table(pi, &tab);

	return didq;
}
static void
wlc_lcnphy_iqlo_set_srom_pwridx(phy_info_t *pi, int8 iqlocalidx, int16 iqlocalpwr, int txpwrindex,
	int8 iqlocalidxoffs)
{
	if (iqlocalidx != -1) {
		txpwrindex = (int)iqlocalidx;
		txpwrindex += iqlocalidxoffs;
	}
	else {
		if (iqlocalpwr == 0)
			txpwrindex = (int)wlc_lcnphy_cw_tx_pwr_ctrl(pi,
				(int32)wlc_phy_txpower_get_target_min((wlc_phy_t*)pi));

		if (iqlocalpwr != -1) {
			txpwrindex = (int) wlc_lcnphy_cw_tx_pwr_ctrl(pi, (int32)iqlocalpwr);
			txpwrindex += iqlocalidxoffs;
		}
	}
	wlc_lcnphy_set_tx_pwr_by_index(pi, txpwrindex);
}
static void
wlc_lcnphy_load_iqlo_txpwr_table(phy_info_t *pi, uint16 startidx, uint16 stopidx, uint8 index)
{
	uint16 a, b, didq;
	uint32 val, bbmult_a_b = 0;
	uint8 ei0, eq0, fi0, fq0;
	phytbl_info_t tab;
	uint idx;
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;

#if defined(PHYCAL_CACHING)
	lcnphy_calcache_t *cache;
	ch_calcache_t *ctx = wlc_phy_get_chanctx(pi, pi->radio_chanspec);
	if (ctx == NULL) {
		PHY_ERROR(("%s: NULL ctx - bail\n", __FUNCTION__));
		return;
	}
	cache = &ctx->u.lcnphy_cache;
#endif /* defined(PHYCAL_CACHING) */

	wlc_lcnphy_get_radio_loft(pi, &ei0, &eq0, &fi0, &fq0);
	/* Get calibration results */
	wlc_lcnphy_get_tx_iqcc(pi, &a, &b);
	PHY_INFORM(("TXIQCal: %d %d\n", a, b));

	didq = wlc_lcnphy_get_tx_locc(pi);

	/* Populate tx power control table with coeffs */
	tab.tbl_id = LCNPHY_TBL_ID_TXPWRCTL;
	tab.tbl_width = 32;	/* 32 bit wide	*/
	tab.tbl_ptr = &val; /* ptr to buf */

	/* Per rate power offset */
	tab.tbl_len = 1; /* # values   */
	tab.tbl_offset = LCNPHY_TX_PWR_CTRL_RATE_OFFSET;

	for (idx = startidx; idx <= stopidx; idx++) {
		if (pi_lcn->virtual_p25_tx_gain_step && (idx % 2 == 1))
			continue;
		/* iq */
		tab.tbl_offset = LCNPHY_TX_PWR_CTRL_IQ_OFFSET
			+ idx/(1+pi_lcn->virtual_p25_tx_gain_step);
		wlc_lcnphy_read_table(pi, &tab);
		val = (val & 0xfff00000) |
			((uint32)(a & 0x3FF) << 10) | (b & 0x3ff);
		bbmult_a_b = val;
		wlc_lcnphy_write_table(pi, &tab);

		/* loft */
		val = didq;
		tab.tbl_offset = LCNPHY_TX_PWR_CTRL_LO_OFFSET
			+ idx/(1+pi_lcn->virtual_p25_tx_gain_step);
		wlc_lcnphy_write_table(pi, &tab);
	}
	if ((pi_lcn->virtual_p25_tx_gain_step) && (stopidx == 127)) {
		for (idx = 64; idx < 128; idx++) {
			/* iq */
			tab.tbl_offset = LCNPHY_TX_PWR_CTRL_IQ_OFFSET + idx;
			val = bbmult_a_b;
			wlc_lcnphy_write_table(pi, &tab);

			/* loft */
			val = didq;
			tab.tbl_offset = LCNPHY_TX_PWR_CTRL_LO_OFFSET + idx;
			wlc_lcnphy_write_table(pi, &tab);
		}
	}

	/* Save Cal Results */
#if defined(PHYCAL_CACHING)
	cache->txiqlocal_a[index] = a;
	cache->txiqlocal_b[index] = b;
	cache->txiqlocal_didq[index] = didq;
	cache->txiqlocal_ei0 = ei0;
	cache->txiqlocal_eq0 = eq0;
	cache->txiqlocal_fi0 = fi0;
	cache->txiqlocal_fq0 = fq0;
#else
	pi_lcn->lcnphy_cal_results.txiqlocal_a[index] = a;
	pi_lcn->lcnphy_cal_results.txiqlocal_b[index] = b;
	pi_lcn->lcnphy_cal_results.txiqlocal_didq[index] = didq;
	pi_lcn->lcnphy_cal_results.txiqlocal_ei0 = ei0;
	pi_lcn->lcnphy_cal_results.txiqlocal_eq0 = eq0;
	pi_lcn->lcnphy_cal_results.txiqlocal_fi0 = fi0;
	pi_lcn->lcnphy_cal_results.txiqlocal_fq0 = fq0;
#endif /* PHYCAL_CACHING */
}

/* Run iqlo cal and populate iqlo portion of tx power control table */
static void
wlc_lcnphy_txpwrtbl_iqlo_cal(phy_info_t *pi)
{

	lcnphy_txgains_t old_gains;
	uint8 save_bb_mult;
	uint16 save_pa_gain = 0;
	uint SAVE_txpwrindex = 0xFF;
	uint16 save_TxPwrCtrlRfCtrlOverride0 = 0;
	uint16 SAVE_txpwrctrl = wlc_lcnphy_get_tx_pwr_ctrl(pi);
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	int txpwrindex = 0xFF;
	int8 iqlocalidx;
	int16 iqlocalpwr;
	int8 iqlocalidxoffs = 0;

	wlc_lcnphy_get_tx_gain(pi, &old_gains);
	save_pa_gain = wlc_lcnphy_get_pa_gain(pi);
	if (LCNREV_GE(pi->pubpi.phy_rev, 2))
		save_TxPwrCtrlRfCtrlOverride0 =
		        phy_utils_read_phyreg(pi, LCNPHY_TxPwrCtrlRfCtrlOverride0);

	/* Store state */
	save_bb_mult = wlc_lcnphy_get_bbmult(pi);

	if (SAVE_txpwrctrl == LCNPHY_TX_PWR_CTRL_OFF)
		SAVE_txpwrindex = wlc_lcnphy_get_current_tx_pwr_idx(pi);

	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		iqlocalidx = pi_lcn->iqlocalidx_2g;
		iqlocalpwr = pi_lcn->iqlocalpwr_2g;
		iqlocalidxoffs = pi_lcn->iqlocalidx2goffs;
	}
	else {
		iqlocalidx = pi_lcn->iqlocalidx_5g;
		iqlocalpwr = pi_lcn->iqlocalpwr_5g;
		iqlocalidxoffs = pi_lcn->iqlocalidx5goffs;
	}
	if (((CHIPID(pi->sh->chip) == BCM4313_CHIP_ID) && LCNREV_IS(pi->pubpi.phy_rev, 1)) ||
		pi_lcn->lcnphy_hw_iqcal_en || LCNREV_GE(pi->pubpi.phy_rev, 1)) {
		if (CHIPID(pi->sh->chip) == BCM4336_CHIP_ID)
			txpwrindex = 12;
		else if (CHIPID(pi->sh->chip) == BCM43362_CHIP_ID) {
			txpwrindex = 60;
			iqlocalpwr = 15;
			iqlocalidxoffs = 42;
		} else if (CHIPID(pi->sh->chip) == BCM4313_CHIP_ID) {
			txpwrindex = 30;
		} else {	/* 4330 */
			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				if (EPA(pi_lcn)) {
					txpwrindex = 44;
				} else {
					if (pi->sh->chippkg == BCM4330_WLBGA_PKG_ID) {
						txpwrindex = 60;
					} else {
						txpwrindex = 30;
					}
				}
			} else {	/* 5G */
				if (EPA(pi_lcn))
					txpwrindex = 32;
				else
					txpwrindex = 73;
			}
		}

		wlc_lcnphy_iqlo_set_srom_pwridx(pi, iqlocalidx, iqlocalpwr, txpwrindex,
			iqlocalidxoffs);

		if (EPA(pi_lcn) && (CHIPID(pi->sh->chip) != BCM4313_CHIP_ID)) {
			int16 apos, bpos, aneg, bneg, amean, bmean;

			/* Check if the idx or txpwr is passed via NVRAM */
			wlc_lcnphy_iqlo_set_srom_pwridx(pi, iqlocalidx, iqlocalpwr, txpwrindex,
				iqlocalidxoffs);

			wlc_lcnphy_tx_iqlo_cal(pi, NULL, (pi_lcn->lcnphy_recal ?
				LCNPHY_CAL_RECAL : LCNPHY_CAL_FULL), FALSE);

			wlc_lcnphy_get_tx_iqcc(pi, (uint16*)&apos, (uint16*)&bpos);

			wlc_lcnphy_tx_iqlo_cal(pi, NULL, LCNPHY_CAL_IQ_RECAL, FALSE);
			wlc_lcnphy_get_tx_iqcc(pi, (uint16*)&aneg, (uint16*)&bneg);

			amean = (apos + aneg)/2;
			bmean = (bpos + bneg)/2;

			wlc_lcnphy_set_tx_iqcc(pi, (uint16)amean, (uint16)bmean);

			PHY_INFORM(("pos: %d %d neg: %d %d, mean: %d %d\n",
				apos, bpos, aneg, bneg, amean, bmean));

		}
		else if (CHSPEC_IS2G(pi->radio_chanspec) && (TXGAINTBL(pi_lcn) == 1)) {
			int8 tx_idx;
			lcnphy_txgains_t temp_gains;

			tx_idx = wlc_lcnphy_get_current_tx_pwr_idx(pi);

			/* gm_gain 15 */
			wlc_lcnphy_tx_iqlo_cal(pi, NULL, (pi_lcn->lcnphy_recal ?
				LCNPHY_CAL_RECAL : LCNPHY_CAL_FULL), FALSE);
			wlc_lcnphy_load_iqlo_txpwr_table(pi, 0, 60, 0);

			/* gm_gain 7 */
			/* add ~1 dB to account for gm delta */
			wlc_lcnphy_set_tx_pwr_by_index(pi, (tx_idx - 4));
			wlc_lcnphy_get_tx_gain(pi, &temp_gains);
			temp_gains.gm_gain = 7;
			wlc_lcnphy_set_tx_gain(pi, &temp_gains);
			wlc_lcnphy_tx_iqlo_cal(pi, NULL, LCNPHY_CAL_TXPWRCTRL, FALSE);
			wlc_lcnphy_load_iqlo_txpwr_table(pi, 61, 109, 1);

			/* gm_gain 3 */
			/* add ~3 dB to account for gm delta */
			wlc_lcnphy_set_tx_pwr_by_index(pi, (tx_idx - 12));
			wlc_lcnphy_get_tx_gain(pi, &temp_gains);
			temp_gains.gm_gain = 3;
			wlc_lcnphy_set_tx_gain(pi, &temp_gains);
			wlc_lcnphy_tx_iqlo_cal(pi, NULL, LCNPHY_CAL_TXPWRCTRL, FALSE);
			wlc_lcnphy_load_iqlo_txpwr_table(pi, 110, 127, 2);
		}
		else if (pi_lcn->use_per_modulation_loft_cal) {
			uint16 didq;
			/* OFDM coeffs */
			phy_utils_mod_radioreg(pi, RADIO_2064_REG012, 0x1,
			PHY_REG_READ(pi, LCNPHY, lpfbwlutreg1, lpf_ofdm_tx_byp));
			wlc_lcnphy_tx_iqlo_cal(pi, NULL, (pi_lcn->lcnphy_recal ?
				LCNPHY_CAL_RECAL : LCNPHY_CAL_FULL), FALSE);
			didq = wlc_lcnphy_get_tx_locc(pi);
			pi_lcn->di_ofdm = (((didq & 0xff00) << 16) >> 24);
			pi_lcn->dq_ofdm = (((didq & 0x00ff) << 24) >> 24);

			/* CCK coeffs */
			PHY_REG_MOD(pi, LCNPHY, lpfbwlutreg1, lpf_ofdm_tx_byp,
			PHY_REG_READ(pi, LCNPHY, lpfbwlutreg1, lpf_cck_tx_byp));
			wlc_lcnphy_tx_iqlo_cal(pi, NULL, LCNPHY_CAL_DLO_RECAL, FALSE);

			didq = wlc_lcnphy_get_tx_locc(pi);
			pi_lcn->di_cck = (((didq & 0xff00) << 16) >> 24);
			pi_lcn->dq_cck = (((didq & 0x00ff) << 24) >> 24);

			PHY_REG_MOD(pi, LCNPHY, dlo_coeff_di_ofdm, dlo_coeff_di_ofdm,
				pi_lcn->di_ofdm);
			PHY_REG_MOD(pi, LCNPHY, dlo_coeff_dq_ofdm, dlo_coeff_dq_ofdm,
				pi_lcn->dq_ofdm);

			PHY_REG_MOD(pi, LCNPHY, dlo_coeff_di_ht, dlo_coeff_di_ht,
				pi_lcn->di_ofdm);
			PHY_REG_MOD(pi, LCNPHY, dlo_coeff_dq_ht, dlo_coeff_dq_ht,
				pi_lcn->dq_ofdm);

			PHY_REG_MOD(pi, LCNPHY, dlo_coeff_di_cck, dlo_coeff_di_cck,
				pi_lcn->di_cck);
			PHY_REG_MOD(pi, LCNPHY, dlo_coeff_dq_cck, dlo_coeff_dq_cck,
				pi_lcn->dq_cck);

			didq = (((pi_lcn->di_ofdm & 0xff)<< 8) | (pi_lcn->dq_ofdm & 0xff));
			wlc_lcnphy_set_tx_locc(pi, didq);
			PHY_REG_MOD(pi, LCNPHY, lpfbwlutreg1, lpf_cck_tx_byp,
				pi_lcn->lpf_cck_tx_byp);
			PHY_REG_MOD(pi, LCNPHY, lpfbwlutreg1, lpf_ofdm_tx_byp,
				pi_lcn->lpf_ofdm_tx_byp);
		}
		else {
			/* Check if the idx or txpwr is passed via NVRAM */
			wlc_lcnphy_iqlo_set_srom_pwridx(pi, iqlocalidx, iqlocalpwr, txpwrindex,
				iqlocalidxoffs);
			wlc_lcnphy_tx_iqlo_cal(pi, NULL, (pi_lcn->lcnphy_recal ?
			LCNPHY_CAL_RECAL : LCNPHY_CAL_FULL), FALSE);
		}
	} else {
		txpwrindex = 12;
		/* Check if the idx or txpwr is passed via NVRAM */
		wlc_lcnphy_iqlo_set_srom_pwridx(pi, iqlocalidx, iqlocalpwr, txpwrindex,
			iqlocalidxoffs);
		/* only 4313a0 or lcn phy rev 0 requires sw tx iqlo cal */
		wlc_lcnphy_tx_iqlo_soft_cal_full(pi);
	}

	if (wlc_lcnphy_tx_dig_iir_dc_err_comp(pi))
		wlc_lcnphy_adjust_frame_based_lo_coeffs(pi);

	if (!CHSPEC_IS2G(pi->radio_chanspec) || (TXGAINTBL(pi_lcn) != 1))
		wlc_lcnphy_load_iqlo_txpwr_table(pi, 0, 127, 0);

	/* Restore state */
	wlc_lcnphy_set_bbmult(pi, save_bb_mult);
	wlc_lcnphy_set_pa_gain(pi, save_pa_gain);
	wlc_lcnphy_set_tx_gain(pi, &old_gains);
	if (LCNREV_GE(pi->pubpi.phy_rev, 2))
		PHY_REG_WRITE(pi, LCNPHY, TxPwrCtrlRfCtrlOverride0, save_TxPwrCtrlRfCtrlOverride0);

	if (SAVE_txpwrctrl != LCNPHY_TX_PWR_CTRL_OFF)
		wlc_lcnphy_set_tx_pwr_ctrl(pi, SAVE_txpwrctrl);
	else
		wlc_lcnphy_set_tx_pwr_by_index(pi, SAVE_txpwrindex);
}

static void
wlc_lcnphy_papd_cal_setup_cw(
	phy_info_t *pi)
{
	uint16 papd_num_skip_count;
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	PHY_REG_MOD(pi, LCNPHY, papd_blanking_control, papd_stop_after_last_update, 0);
	/* Tune the hardware delay */
	if (pi->u.pi_lcnphy->papd_corr_norm > 0) {
		if (pi->u.pi_lcnphy->dacrate == 160)
			PHY_REG_WRITE(pi, LCNPHY, papd_spb2papdin_dly, 0x23);
		else
			PHY_REG_WRITE(pi, LCNPHY, papd_spb2papdin_dly, 0x30);
		if (pi->u.pi_lcnphy->papd_corr_norm == 2)
			PHY_REG_MOD(pi, LCNPHY, papd_blanking_control, papd_stop_after_last_update,
			 1);
	}
	else
		PHY_REG_WRITE(pi, LCNPHY, papd_spb2papdin_dly, 0x23);

	PHY_REG_LIST_START
		/* Set samples/cycle/4 for q delay */
		PHY_REG_WRITE_ENTRY(LCNPHY, papd_variable_delay, 3)
		/* Set LUT begin gain, step gain, and size (Reset values, remove if possible) */
		PHY_REG_WRITE_ENTRY(LCNPHY, papd_track_pa_lut_begin, 6000)
	PHY_REG_LIST_EXECUTE(pi);

	/* peak_curr_mode dependent */
	if (CHIPID(pi->sh->chip) == BCM4313_CHIP_ID)
	    PHY_REG_WRITE(pi, LCNPHY, papd_rx_gain_comp_dbm, 200);
	else
	    PHY_REG_WRITE(pi, LCNPHY, papd_rx_gain_comp_dbm, 0);

	PHY_REG_LIST_START
		PHY_REG_WRITE_ENTRY(LCNPHY, papd_track_pa_lut_step, 0x444)
		PHY_REG_WRITE_ENTRY(LCNPHY, papd_track_pa_lut_end, 0x3f)
		/* set papd constants */
		PHY_REG_WRITE_ENTRY(LCNPHY, papd_dbm_offset, 0x681)
		/* Dc estimation samples */
		PHY_REG_WRITE_ENTRY(LCNPHY, papd_ofdm_dc_est, 0x49)
	PHY_REG_LIST_EXECUTE(pi);

	/* lcnphy - newly added registers */
	PHY_REG_WRITE(pi, LCNPHY, papd_cw_corr_norm, pi->u.pi_lcnphy->papd_corr_norm);

	PHY_REG_LIST_START
		PHY_REG_WRITE_ENTRY(LCNPHY, papd_blanking_control,
			(PAPD_BLANKING_PROFILE << 9 | PAPD_BLANKING_THRESHOLD))
		PHY_REG_WRITE_ENTRY(LCNPHY, papd_num_samples_count, 255)
		PHY_REG_WRITE_ENTRY(LCNPHY, papd_sync_count, 319)
	PHY_REG_LIST_EXECUTE(pi);

	/* Adjust the number of samples to be processed depending on the corr_norm */
	PHY_REG_WRITE(pi, LCNPHY, papd_num_samples_count,
		(((255+1)/(1<<(pi->u.pi_lcnphy->papd_corr_norm)))-1));
	PHY_REG_WRITE(pi, LCNPHY, papd_sync_count,
		(((319+1)/(1<<(pi->u.pi_lcnphy->papd_corr_norm)))-1));

	papd_num_skip_count = PAPD_NUM_SKIP_COUNT;
	switch (pi->u.pi_lcnphy->papd_corr_norm) {
		case 0:
			papd_num_skip_count = MIN(PAPD_NUM_SKIP_COUNT, 52);
			break;
		case 1:
			papd_num_skip_count = MIN(PAPD_NUM_SKIP_COUNT, 24);
			break;
		case 2:
			papd_num_skip_count = MIN(PAPD_NUM_SKIP_COUNT, 8);
			break;
	}
	PHY_REG_WRITE(pi, LCNPHY, papd_num_skip_count, papd_num_skip_count);

	PHY_REG_LIST_START
		PHY_REG_WRITE_ENTRY(LCNPHY, papd_switch_lut, PAPD2LUT)
		PHY_REG_WRITE_ENTRY(LCNPHY, papd_pa_off_control_1, 0)
		PHY_REG_WRITE_ENTRY(LCNPHY, papd_only_loop_gain, 0)
		PHY_REG_WRITE_ENTRY(LCNPHY, papd_pa_off_control_2, 0)
		PHY_REG_WRITE_ENTRY(LCNPHY, smoothenLut_max_thr, 0x7ff)
		PHY_REG_WRITE_ENTRY(LCNPHY, papd_dcest_i_ovr, 0x0000)
		PHY_REG_WRITE_ENTRY(LCNPHY, papd_dcest_q_ovr, 0x0000)
	PHY_REG_LIST_EXECUTE(pi);
}

static void
wlc_lcnphy_papd_cal_core(
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
	bool papd_lpgn_ovr,
	uint16 LPGN_I,
	uint16 LPGN_Q)
{
	phytbl_info_t tab;
	uint32 papdcompdeltatbl_init_val;
	uint32 j;
	uint32 papd_buf[] = {0x7fc00, 0x5a569, 0x1ff, 0xa5d69, 0x80400, 0xa5e97, 0x201, 0x5a697};

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	PHY_REG_LIST_START
		/* Reset PAPD Hw to reset register values */
		PHY_REG_OR_ENTRY(LCNPHY, papd_control2, 0x1)
		PHY_REG_AND_ENTRY(LCNPHY, papd_control2, ~0x1)
	PHY_REG_LIST_EXECUTE(pi);

	PHY_REG_MOD(pi, LCNPHY, papd_control2, papd_loop_gain_cw_ovr, papd_lpgn_ovr);

	/* set PAPD registers to configure the PAPD calibration */
	if (init_papd_lut != 0) {
		/* Load papd comp delta table */
		papdcompdeltatbl_init_val = 0x80000;
		tab.tbl_ptr = &papdcompdeltatbl_init_val; /* ptr to init var */
		tab.tbl_len = 1;        /* # values   */
		tab.tbl_id = LCNPHY_TBL_ID_PAPDCOMPDELTATBL;         /* papdcompdeltatbl */
		tab.tbl_width = 32;     /* 32 bit wide */
		if (PAPD2LUT == 1)
			tab.tbl_offset = 64; /* tbl offset */
		else
			tab.tbl_offset = 0; /* tbl offset */
		for (j = 0; j < 64; j ++) {
			wlc_lcnphy_write_table(pi, &tab);
			tab.tbl_offset++;
		}
	}

	/* set PAPD registers to configure PAPD calibration */
	wlc_lcnphy_papd_cal_setup_cw(pi);

	/* num_symbols is computed based on corr_norm */
	num_symbols = num_symbols * PAPD_BLANKING_PROFILE + 1;

	/* override control params */
	PHY_REG_WRITE(pi, LCNPHY, papd_loop_gain_ovr_cw_i, LPGN_I);
	PHY_REG_WRITE(pi, LCNPHY, papd_loop_gain_ovr_cw_q, LPGN_Q);

	/* papd update */
	PHY_REG_WRITE(pi, LCNPHY, papd_track_num_symbols_count, num_symbols);

	PHY_REG_LIST_START
		/* spb parameters */
		PHY_REG_WRITE_ENTRY(LCNPHY, papd_spb_num_vld_symbols_n_dly, 0x60)
		PHY_REG_WRITE_ENTRY(LCNPHY, sampleDepthCount, 7)
	PHY_REG_LIST_EXECUTE(pi);

	PHY_REG_WRITE(pi, LCNPHY, sampleLoopCount, (num_symbols+1)*20-1);
	PHY_REG_WRITE(pi, LCNPHY, papd_spb_rd_address, 0);

	/* load the spb */
	tab.tbl_len = 8;
	tab.tbl_id = LCNPHY_TBL_ID_SAMPLEPLAY;
	tab.tbl_offset = 0;
	tab.tbl_width = 32;
	tab.tbl_ptr = &papd_buf;
	wlc_lcnphy_write_table(pi, &tab);

	/* BBMULT parameters */
	PHY_REG_WRITE(pi, LCNPHY, papd_bbmult_init, papd_bbmult_init);
	PHY_REG_WRITE(pi, LCNPHY, papd_bbmult_step, papd_bbmult_step);
	PHY_REG_WRITE(pi, LCNPHY, papd_bbmult_num_symbols, 1-1);

	/* set bbmult to 0 to remove DC current spike after cal */
	wlc_lcnphy_set_bbmult(pi, 0);
	/* Run PAPD HW Cal */

	/*
	 * LCNPHY_papd_rx_sm_iqmm_gain_comp:
		GAIN0MODE==0: 0x100; GAIN0MODE==1: 0;
	 * LCNPHY_papd_control:
		PDBYPASS==0&&GAIN0MODE==1: 0xb8a1;
		PDBYPASS==0&&GAIN0MODE==0: 0xb0a1;
		PDBYPASS==1&&GAIN0MODE==1: 0x8821;
		PDBYPASS==1&&GAIN0MODE==0: 0x8021;
	*/
	#define PDBYPASS	0	/* PD output(0) or input(1)based training */
	#define GAIN0MODE	1	/* 0 for using both I and Q , 1 use only I */
	PHY_REG_LIST_START
		PHY_REG_WRITE_ENTRY(LCNPHY, papd_rx_sm_iqmm_gain_comp, (GAIN0MODE == 0 ? 0x100 : 0))
		PHY_REG_WRITE_ENTRY(LCNPHY, papd_control, 0x8021|(PDBYPASS == 0 ? 0X3080 : 0)|
			(GAIN0MODE == 1 ? 0X800 : 0))
	PHY_REG_LIST_EXECUTE(pi);

	/* Wait for completion, around 1s */
	SPINWAIT(phy_utils_read_phyreg(pi, LCNPHY_papd_control) &
	         LCNPHY_papd_control_papd_cal_run_MASK,
		1 * 1000 * 1000);
}

static int16
wlc_lcnphy_temp_sense_vbatTemp_on(phy_info_t *pi, int mode)
{
	uint8 save_reg007, save_reg0FF, save_reg11F;
	uint16 save_afectrlovr, save_afectrlovrval;
	uint16 save_TxPwrCtrlRfCtrlOverride0, save_TxPwrCtrlRfCtrlOverride1;
	uint16 tempsenseval1, tempsenseval2;
	uint16 vbat;
	int16 temp2, temp1;
	int16 avg = 0;
	bool suspend;
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;

	wlc_lcnphy_tx_pwr_update_npt(pi);

	suspend = !(R_REG(pi->sh->osh, &pi->regs->maccontrol) & MCTL_EN_MAC);
	if (!suspend)
		wlapi_suspend_mac_and_wait(pi->sh->physhim);

	save_reg007 = (uint8)phy_utils_read_radioreg(pi, RADIO_2064_REG007);
	save_reg0FF = (uint8)phy_utils_read_radioreg(pi, RADIO_2064_REG0FF);
	save_reg11F = (uint8)phy_utils_read_radioreg(pi, RADIO_2064_REG11F);
	save_afectrlovr = phy_utils_read_phyreg(pi, LCNPHY_AfeCtrlOvr);
	save_afectrlovrval = phy_utils_read_phyreg(pi, LCNPHY_AfeCtrlOvrVal);
	save_TxPwrCtrlRfCtrlOverride0 = phy_utils_read_phyreg(pi, LCNPHY_TxPwrCtrlRfCtrlOverride0);
	save_TxPwrCtrlRfCtrlOverride1 = phy_utils_read_phyreg(pi, LCNPHY_TxPwrCtrlRfCtrlOverride1);

	PHY_REG_LIST_START
		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlRfCtrlOverride0, amuxSelPortOverride, 0)
		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlRfCtrlOverride1, afeAuxpgaSelVmidOverride, 0)
		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlRfCtrlOverride1, afeAuxpgaSelGainOverride, 0)
		PHY_REG_MOD_ENTRY(LCNPHY, AfeCtrlOvr, pwdn_rssi_ovr, 1)
		PHY_REG_MOD_ENTRY(LCNPHY, AfeCtrlOvrVal, pwdn_rssi_ovr_val, 0)

		RADIO_REG_MOD_ENTRY(RADIO_2064_REG007, 0x1, (1<<0))
		RADIO_REG_MOD_ENTRY(RADIO_2064_REG0FF, 0x10, (1<<4))
		RADIO_REG_MOD_ENTRY(RADIO_2064_REG11F, 0x4, (1<<2))
	PHY_REG_LIST_EXECUTE(pi);

	OSL_DELAY(10);
	PHY_REG_MOD(pi, LCNPHY, TxPwrCtrlRangeCmd, force_vbatTemp, 1);
	OSL_DELAY(10);

	if (mode == TEMPSENSE) {
		tempsenseval1 = phy_utils_read_phyreg(pi, LCNPHY_TxPwrCtrlStatusTemp) & 0x1FF;
		tempsenseval2 = phy_utils_read_phyreg(pi, LCNPHY_TxPwrCtrlStatusTemp1) & 0x1FF;

		pi_lcn->tempsensereg1 = tempsenseval1;
		pi_lcn->tempsensereg2 = tempsenseval2;

		if (tempsenseval2 > 255)
			temp2 = (int16)(tempsenseval2 - 512);
		else
			temp2 = (int16)tempsenseval2;

		if (tempsenseval1 > 255)
			temp1 = (int16)(tempsenseval1 - 512);
		else
			temp1 = (int16)tempsenseval1;

		avg = temp2 - temp1;
	}
	else {
		vbat = phy_utils_read_phyreg(pi, LCNPHY_TxPwrCtrlStatusVbat) & 0x1FF;
		if (vbat > 255)
			avg = (int16)(vbat - 512);
		else
			avg = (int16)vbat;
	}

	phy_utils_write_radioreg(pi, RADIO_2064_REG007, save_reg007);
	phy_utils_write_radioreg(pi, RADIO_2064_REG0FF, save_reg0FF);
	phy_utils_write_radioreg(pi, RADIO_2064_REG11F, save_reg11F);
	PHY_REG_WRITE(pi, LCNPHY, AfeCtrlOvr, save_afectrlovr);
	PHY_REG_WRITE(pi, LCNPHY, AfeCtrlOvrVal, save_afectrlovrval);
	PHY_REG_WRITE(pi, LCNPHY, TxPwrCtrlRfCtrlOverride0, save_TxPwrCtrlRfCtrlOverride0);
	PHY_REG_WRITE(pi, LCNPHY, TxPwrCtrlRfCtrlOverride1, save_TxPwrCtrlRfCtrlOverride1);

	if (!suspend)
		wlapi_enable_mac(pi->sh->physhim);

	return avg;
}

/* wlc_lcnphy_tempsense_new: use mode 1 if tempsense registers
 *	might be used for other purposes such as background idle tssi measurement
 */
int16
wlc_lcnphy_tempsense_new(phy_info_t *pi, bool mode)
{
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	uint16 tempsenseval1, tempsenseval2;
	int16 temp2, temp1;
	int16 avg = 0;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	if (NORADIO_ENAB(pi->pubpi))
		return -1;

	/* mode 1 will use force_vbat and do the measurement */
	if (mode == 1) {
		avg = wlc_lcnphy_temp_sense_vbatTemp_on(pi, TEMPSENSE);
		return avg;
	}

	tempsenseval1 = pi_lcn->tempsensereg1;
	tempsenseval2 = pi_lcn->tempsensereg2;

	if (tempsenseval2 > 255)
		temp2 = (int16)(tempsenseval2 - 512);
	else
		temp2 = (int16)tempsenseval2;

	if (tempsenseval1 > 255)
		temp1 = (int16)(tempsenseval1 - 512);
	else
		temp1 = (int16)tempsenseval1;

	avg = temp2 - temp1;
	return avg;
}

uint16
wlc_lcnphy_tempsense(phy_info_t *pi, bool mode)
{
	uint16 tempsenseval1, tempsenseval2;
	int32 avg = 0;
	bool suspend = FALSE;
	uint16 SAVE_txpwrctrl = wlc_lcnphy_get_tx_pwr_ctrl(pi);
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	if (NORADIO_ENAB(pi->pubpi))
		return -1;

	/* mode 1 will set-up and do the measurement */
	if (mode == 1) {
		suspend = !(R_REG(pi->sh->osh, &pi->regs->maccontrol) & MCTL_EN_MAC);
		if (!suspend)
			wlapi_suspend_mac_and_wait(pi->sh->physhim);
		wlc_lcnphy_vbat_temp_sense_setup(pi, TEMPSENSE);
	}
	tempsenseval1 = phy_utils_read_phyreg(pi, LCNPHY_TxPwrCtrlStatusTemp) & 0x1FF;
	tempsenseval2 = phy_utils_read_phyreg(pi, LCNPHY_TxPwrCtrlStatusTemp1) & 0x1FF;

	if (tempsenseval1 > 255)
		avg = (int)(tempsenseval1 - 512);
	else
		avg = (int)tempsenseval1;

	if (pi_lcn->lcnphy_tempsense_option == 1 ||
		(pi->hwpwrctrl_capable && CHIPID(pi->sh->chip) == BCM4313_CHIP_ID)) {
		if (tempsenseval2 > 255)
			avg = (int)(avg - tempsenseval2 + 512);
		else
			avg = (int)(avg - tempsenseval2);
	} else {
		if (tempsenseval2 > 255)
			avg = (int)(avg + tempsenseval2 - 512);
		else
			avg = (int)(avg + tempsenseval2);
		avg = avg/2;
	}
	if (avg < 0)
		avg = avg + 512;

	if (pi_lcn->lcnphy_tempsense_option == 2)
		avg = tempsenseval1;

	if (mode)
		wlc_lcnphy_set_tx_pwr_ctrl(pi, SAVE_txpwrctrl);

	if (mode == 1) {
		/* reset CCA */
		PHY_REG_MOD(pi, LCNPHY, lpphyCtrl, resetCCA, 1);
		OSL_DELAY(100);
		PHY_REG_MOD(pi, LCNPHY, lpphyCtrl, resetCCA, 0);
		if (!suspend)
			wlapi_enable_mac(pi->sh->physhim);
	}
	return (uint16)avg;
}

int8
wlc_lcnphy_tempsense_degree(phy_info_t *pi, bool mode)
{
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;

	int32 b = pi_lcn->temp_add;
	int32 a = pi_lcn->temp_mult;
	int32 q = pi_lcn->temp_q;

	int32 degree = wlc_lcnphy_tempsense_new(pi, mode);

	if (LCNREV_GE(pi->pubpi.phy_rev, 1)) {
	    /* Temp in deg = (temp_add - (T2-T1)*temp_mult)>>temp_q; */
		degree = ((b - (degree * a)) + (1 << (q-1))) >> q;
	}
	else {
		degree = ((degree << 10) + LCN_TEMPSENSE_OFFSET + (LCN_TEMPSENSE_DEN >> 1))
				/ LCN_TEMPSENSE_DEN;
	}

	return (int8)degree;
}

/* return value is in volt Q4.4 */
int8
wlc_lcnphy_vbatsense(phy_info_t *pi, bool mode)
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
	if (CHIPID(pi->sh->chip) == BCM4313_CHIP_ID) {
		if (mode == 1) {
			wlc_lcnphy_vbat_temp_sense_setup(pi, VBATSENSE);
		}

		vbatsenseval = phy_utils_read_phyreg(pi, LCNPHY_TxPwrCtrlStatusVbat) & 0x1FF;
		if (vbatsenseval > 255)
			avg =  (int32)(vbatsenseval - 512);
		else
			avg = (int32)vbatsenseval;

		avg = (avg * LCN_VBAT_SCALE_NOM + (LCN_VBAT_SCALE_DEN >> 1))
			/ LCN_VBAT_SCALE_DEN;

		return (int8)avg;

	}
	else if (mode == 1) {
		avg = wlc_lcnphy_temp_sense_vbatTemp_on(pi, VBATSENSE);
	}
	else {
		vbatsenseval = phy_utils_read_phyreg(pi, LCNPHY_TxPwrCtrlStatusVbat) & 0x1FF;

		if (vbatsenseval > 255)
			avg =  (int32)(vbatsenseval - 512);
		else
			avg = (int32)vbatsenseval;
	}

	/* Voltage = (vbat_add - (vbat_reading)*vbat_mult)>>vbat_q;  */
	avg = ((b - (a * avg)) + (1 << (q-5))) >> (q - 4);

	return (int8)avg;
}

static uint32
wlc_lcnphy_papd_rxGnCtrl(
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
	int32  cwLpGn2_min = 8192, cwLpGn2_max = 16384;
	uint8  bsCnt;
	int16  lgI, lgQ;
	int32  cwLpGn2;
	uint8  num_symbols4lpgn;
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	/* frcRxGnCtrl conditional missing */
	for (bsCnt = 0; bsCnt < bsDepth; bsCnt++) {
		/* Running PAPD for Tx gain index:CurTxGain */
		/* Rx gain index : tia gain : rxGnInit */
		PHY_PAPD(("Running PAPD for Tx Gain Idx : %d ,Rx Gain Index %d\n",
			CurTxGain, rxGnInit));
		if (rxGnInit > 15)
			rxGnInit = 15; /* out-of-range correction */
		wlc_lcnphy_set_rx_gain_by_distribution(pi, 1, 0, 0, 0, (uint16)rxGnInit, 0, 0, 0);

		num_symbols4lpgn = 219;
		wlc_lcnphy_papd_cal_core(pi, 0,
		                         TRUE,
		                         0,
		                         0,
		                         0,
		                         num_symbols4lpgn,
		                         1,
		                         1400,
		                         16640,
		                         0,
		                         128,
		                         0);
		if (cal_type == PHY_PAPD_CAL_CW) {
			lgI = ((int16) phy_utils_read_phyreg(pi, LCNPHY_papd_loop_gain_cw_i)) << 6;
			lgI = lgI >> 6;
			lgQ = ((int16) phy_utils_read_phyreg(pi, LCNPHY_papd_loop_gain_cw_q)) << 6;
			lgQ = lgQ >> 6;

			cwLpGn2 = (lgI * lgI) + (lgQ * lgQ);

			PHY_PAPD(("LCNPHY_papd_loop_gain_cw_i %x LCNPHY_papd_loop_gain_cw_q %x\n",
				phy_utils_read_phyreg(pi, LCNPHY_papd_loop_gain_cw_i),
				phy_utils_read_phyreg(pi, LCNPHY_papd_loop_gain_cw_q)));
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
	}
	if (rxGnInit > 9)
		rxGnInit = 9;
	if (rxGnInit < 0)
		rxGnInit = 0; /* out-of-range correction */

	pi_lcn->lcnphy_papdRxGnIdx = rxGnInit;
	PHY_PAPD(("wl%d: %s Settled to rxGnInit: %d\n",
		pi->sh->unit, __FUNCTION__, rxGnInit));
	return rxGnInit;
}

static void
wlc_lcnphy_afe_clk_init(phy_info_t *pi, uint8 mode)
{
	uint8 phybw40;
	phybw40 = CHSPEC_IS40(pi->radio_chanspec);
	/* IQ Swap at adc */
	PHY_REG_MOD(pi, LCNPHY, rxfe, swap_rxfiltout_iq, 1);
	/* Setting adc in 2x mode */
	if (((mode == AFE_CLK_INIT_MODE_PAPD) && (phybw40 == 0)) ||
		(mode == AFE_CLK_INIT_MODE_TXRX2X))
		PHY_REG_WRITE(pi, LCNPHY, adc_2x, 0xf);

	wlc_lcnphy_toggle_afe_pwdn(pi);
}

static void
wlc_lcnphy_GetpapdMaxMinIdxupdt(phy_info_t *pi,
	int8 *maxUpdtIdx,
	int8 *minUpdtIdx)
{
	uint16 papd_lut_index_updt_63_48, papd_lut_index_updt_47_32;
	uint16 papd_lut_index_updt_31_16, papd_lut_index_updt_15_0;
	int8 MaxIdx, MinIdx;
	uint8 MaxIdxUpdated, MinIdxUpdated;
	uint8 i;

	papd_lut_index_updt_63_48 = phy_utils_read_phyreg(pi, LCNPHY_papd_lut_index_updated_63_48);
	papd_lut_index_updt_47_32 = phy_utils_read_phyreg(pi, LCNPHY_papd_lut_index_updated_47_32);
	papd_lut_index_updt_31_16 = phy_utils_read_phyreg(pi, LCNPHY_papd_lut_index_updated_31_16);
	papd_lut_index_updt_15_0  = phy_utils_read_phyreg(pi, LCNPHY_papd_lut_index_updated_15_0);

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
wlc_lcnphy_save_papd_calibration_results(phy_info_t *pi)
{
	phytbl_info_t tab;
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;

#if defined(PHYCAL_CACHING)
	lcnphy_calcache_t *cache;
	ch_calcache_t *ctx = wlc_phy_get_chanctx(pi, pi->radio_chanspec);
	if (ctx == NULL) {
		PHY_ERROR(("%s: NULL ctx - bail\n", __FUNCTION__));
		return;
	}
	cache = &ctx->u.lcnphy_cache;
#endif // endif

	/* Save epsilon table */
	if (LCNREV_GE(pi->pubpi.phy_rev, 3))
		tab.tbl_len = PHY_PAPD_EPS_TBL_SIZE_LCNPHY*2;
	else
		tab.tbl_len = PHY_PAPD_EPS_TBL_SIZE_LCNPHY;
	tab.tbl_id = LCNPHY_TBL_ID_PAPDCOMPDELTATBL;
	tab.tbl_offset = 0;
	tab.tbl_width = 32;
	/* To make sure the papd comp is enabled as we are saving this reg. state */
	PHY_REG_MOD(pi, LCNPHY, papd_control, papdCompEn, 1);

#if defined(PHYCAL_CACHING)
	tab.tbl_ptr = cache->papd_eps_tbl;
	wlc_lcnphy_read_table(pi, &tab);

	cache->analog_gain_ref =
		phy_utils_read_phyreg(pi, LCNPHY_papd_tx_analog_gain_ref);
	cache->lut_begin =
		phy_utils_read_phyreg(pi, LCNPHY_papd_track_pa_lut_begin);
	cache->lut_step  =
		phy_utils_read_phyreg(pi, LCNPHY_papd_track_pa_lut_step);
	cache->lut_end	 =
		phy_utils_read_phyreg(pi, LCNPHY_papd_track_pa_lut_end);
	cache->rxcompdbm =
		phy_utils_read_phyreg(pi, LCNPHY_papd_rx_gain_comp_dbm);
	cache->papdctrl  =
		phy_utils_read_phyreg(pi, LCNPHY_papd_control);
	cache->sslpnCalibClkEnCtrl =
		phy_utils_read_phyreg(pi, LCNPHY_sslpnCalibClkEnCtrl);
	if (LCNREV_GE(pi->pubpi.phy_rev, 3)) {
		cache->papd_dup_lut_ctrl =
			phy_utils_read_phyreg(pi, LCNPHY_PapdDupLutCtrl);
		cache->papd_num_lut_used = pi_lcn->papd_num_lut_used;
		cache->bbshift_ctrl =
			phy_utils_read_phyreg(pi, LCNPHY_bbShiftCtrl);
		cache->tx_pwr_ctrl_range_cmd =
			phy_utils_read_phyreg(pi, LCNPHY_TxPwrCtrlRangeCmd);
		cache->papd_lut1_global_disable =
			PHY_REG_READ(pi, LCNPHY, RadioRegWriteAdd4, GlobalDisable);
		cache->papd_lut1_thres = pi_lcn->papd_lut1_thres;
		cache->papd_lut0_cal_idx = pi_lcn->papd_lut0_cal_idx;
		cache->papd_lut1_cal_idx = pi_lcn->papd_lut1_cal_idx;
	}

#else
	tab.tbl_ptr = pi_lcn->lcnphy_cal_results.papd_eps_tbl;
	wlc_lcnphy_read_table(pi, &tab);

	pi_lcn->lcnphy_cal_results.analog_gain_ref =
		phy_utils_read_phyreg(pi, LCNPHY_papd_tx_analog_gain_ref);
	pi_lcn->lcnphy_cal_results.lut_begin =
		phy_utils_read_phyreg(pi, LCNPHY_papd_track_pa_lut_begin);
	pi_lcn->lcnphy_cal_results.lut_step  =
		phy_utils_read_phyreg(pi, LCNPHY_papd_track_pa_lut_step);
	pi_lcn->lcnphy_cal_results.lut_end   =
		phy_utils_read_phyreg(pi, LCNPHY_papd_track_pa_lut_end);
	pi_lcn->lcnphy_cal_results.rxcompdbm =
		phy_utils_read_phyreg(pi, LCNPHY_papd_rx_gain_comp_dbm);
	pi_lcn->lcnphy_cal_results.papdctrl  =
		phy_utils_read_phyreg(pi, LCNPHY_papd_control);
	pi_lcn->lcnphy_cal_results.sslpnCalibClkEnCtrl =
		phy_utils_read_phyreg(pi, LCNPHY_sslpnCalibClkEnCtrl);
	if (LCNREV_GE(pi->pubpi.phy_rev, 3)) {
		pi_lcn->lcnphy_cal_results.papd_dup_lut_ctrl =
			phy_utils_read_phyreg(pi, LCNPHY_PapdDupLutCtrl);
		pi_lcn->lcnphy_cal_results.papd_num_lut_used =
			pi_lcn->papd_num_lut_used;
		pi_lcn->lcnphy_cal_results.bbshift_ctrl =
			phy_utils_read_phyreg(pi, LCNPHY_bbShiftCtrl);
		pi_lcn->lcnphy_cal_results.tx_pwr_ctrl_range_cmd =
			phy_utils_read_phyreg(pi, LCNPHY_TxPwrCtrlRangeCmd);
		pi_lcn->lcnphy_cal_results.papd_lut1_global_disable =
			PHY_REG_READ(pi, LCNPHY, RadioRegWriteAdd4, GlobalDisable);
		pi_lcn->lcnphy_cal_results.papd_lut1_thres =
			pi_lcn->papd_lut1_thres;
		pi_lcn->lcnphy_cal_results.papd_lut0_cal_idx =
			pi_lcn->papd_lut0_cal_idx;
		pi_lcn->lcnphy_cal_results.papd_lut1_cal_idx =
			pi_lcn->papd_lut1_cal_idx;
	}
#endif /* PHYCAL_CACHING */
}

static void
wlc_lcnphy_restore_papd_calibration_results(phy_info_t *pi)
{
	phytbl_info_t tab;
#if defined(PHYCAL_CACHING)
	ch_calcache_t *ctx = wlc_phy_get_chanctx(pi, pi->radio_chanspec);
	lcnphy_calcache_t *cache = &ctx->u.lcnphy_cache;
#endif // endif
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;

	/* write eps table */
	tab.tbl_id = LCNPHY_TBL_ID_PAPDCOMPDELTATBL;
	tab.tbl_width = 32;
	if (LCNREV_GE(pi->pubpi.phy_rev, 3))
		tab.tbl_len = PHY_PAPD_EPS_TBL_SIZE_LCNPHY*2;
	else
		tab.tbl_len = PHY_PAPD_EPS_TBL_SIZE_LCNPHY;
	tab.tbl_offset = 0;
#if defined(PHYCAL_CACHING)
	tab.tbl_ptr = cache->papd_eps_tbl;
	wlc_lcnphy_write_table(pi, &tab);

	PHY_REG_WRITE(pi, LCNPHY, papd_tx_analog_gain_ref,
		cache->analog_gain_ref);
	PHY_REG_WRITE(pi, LCNPHY, papd_track_pa_lut_begin,
		cache->lut_begin);
	PHY_REG_WRITE(pi, LCNPHY, papd_track_pa_lut_step,
		cache->lut_step);
	PHY_REG_WRITE(pi, LCNPHY, papd_track_pa_lut_end,
		cache->lut_end);
	PHY_REG_WRITE(pi, LCNPHY, papd_rx_gain_comp_dbm,
		cache->rxcompdbm);
	PHY_REG_WRITE(pi, LCNPHY, papd_control,
		cache->papdctrl);
	PHY_REG_WRITE(pi, LCNPHY, sslpnCalibClkEnCtrl,
		cache->sslpnCalibClkEnCtrl);
	if (LCNREV_GE(pi->pubpi.phy_rev, 3)) {
		PHY_REG_WRITE(pi, LCNPHY, PapdDupLutCtrl,
			cache->papd_dup_lut_ctrl);
		PHY_REG_WRITE(pi, LCNPHY, bbShiftCtrl,
			cache->bbshift_ctrl);
		PHY_REG_WRITE(pi, LCNPHY, TxPwrCtrlRangeCmd,
			cache->tx_pwr_ctrl_range_cmd);
		PHY_REG_MOD(pi, LCNPHY, RadioRegWriteAdd4,
			GlobalDisable, cache->papd_lut1_global_disable);
		pi_lcn->papd_num_lut_used = cache->papd_num_lut_used;
		pi_lcn->papd_lut1_thres = cache->papd_lut1_thres;
		pi_lcn->papd_lut0_cal_idx = cache->papd_lut0_cal_idx;
		pi_lcn->papd_lut1_cal_idx = cache->papd_lut1_cal_idx;
	}
	wlc_lcnphy_rfpower_lut_2papdlut(pi);
	/* Restore the last gain index for this channel */
	wlc_lcnphy_set_tx_pwr_by_index(pi, cache->lcnphy_gain_index_at_last_cal);
#else
	tab.tbl_ptr = pi_lcn->lcnphy_cal_results.papd_eps_tbl;
	wlc_lcnphy_write_table(pi, &tab);

	PHY_REG_WRITE(pi, LCNPHY, papd_tx_analog_gain_ref,
		pi_lcn->lcnphy_cal_results.analog_gain_ref);
	PHY_REG_WRITE(pi, LCNPHY, papd_track_pa_lut_begin,
		pi_lcn->lcnphy_cal_results.lut_begin);
	PHY_REG_WRITE(pi, LCNPHY, papd_track_pa_lut_step,
		pi_lcn->lcnphy_cal_results.lut_step);
	PHY_REG_WRITE(pi, LCNPHY, papd_track_pa_lut_end,
		pi_lcn->lcnphy_cal_results.lut_end);
	PHY_REG_WRITE(pi, LCNPHY, papd_rx_gain_comp_dbm,
		pi_lcn->lcnphy_cal_results.rxcompdbm);
	PHY_REG_WRITE(pi, LCNPHY, papd_control,
		pi_lcn->lcnphy_cal_results.papdctrl);
	PHY_REG_WRITE(pi, LCNPHY, sslpnCalibClkEnCtrl,
		pi_lcn->lcnphy_cal_results.sslpnCalibClkEnCtrl);
	if (LCNREV_GE(pi->pubpi.phy_rev, 3)) {
		PHY_REG_WRITE(pi, LCNPHY, PapdDupLutCtrl,
			pi_lcn->lcnphy_cal_results.papd_dup_lut_ctrl);
		PHY_REG_WRITE(pi, LCNPHY, bbShiftCtrl,
			pi_lcn->lcnphy_cal_results.bbshift_ctrl);
		PHY_REG_WRITE(pi, LCNPHY, TxPwrCtrlRangeCmd,
			pi_lcn->lcnphy_cal_results.tx_pwr_ctrl_range_cmd);
		PHY_REG_MOD(pi, LCNPHY, RadioRegWriteAdd4,
			GlobalDisable, pi_lcn->lcnphy_cal_results.papd_lut1_global_disable);
		pi_lcn->papd_num_lut_used = pi_lcn->lcnphy_cal_results.papd_num_lut_used;
		pi_lcn->papd_lut1_thres = pi_lcn->lcnphy_cal_results.papd_lut1_thres;
		pi_lcn->papd_lut0_cal_idx = pi_lcn->lcnphy_cal_results.papd_lut0_cal_idx;
		pi_lcn->papd_lut1_cal_idx = pi_lcn->lcnphy_cal_results.papd_lut1_cal_idx;
	}
	wlc_lcnphy_rfpower_lut_2papdlut(pi);
#endif /* PHYCAL_CACHING */
}

static void
wlc_lcnphy_papd_calc_capindex(phy_info_t *pi, lcnphy_txcalgains_t *txgains)
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
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;

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
	temp =  wlc_lcnphy_qdiv_roundup(Threshold, 1000, 0);
	final_idx_thresh = ((uint64)temp*temp);
	lcnphytxindex = wlc_lcnphy_papd_index_search(pi, final_idx_thresh, txgains);
	if ((lcnphytxindex < 40) || (lcnphytxindex >= 70)) {
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
		temp =  wlc_lcnphy_qdiv_roundup(Threshold, 1000, 0);
		final_idx_thresh = ((uint64)temp*temp);
		lcnphytxindex = wlc_lcnphy_papd_index_search(pi, final_idx_thresh, txgains);

	}
	/* cap it to 1dB higher pwr as headroom  */
	pi_lcn->lcnphy_capped_index = lcnphytxindex - 4;

}

static uint16
wlc_lcnphy_papd_index_search(phy_info_t *pi, uint64 final_idx_thresh,
	lcnphy_txcalgains_t *txgains)
{
	uint16 start_index = 0;
	uint16 stop_index = 100;
	uint16 mid_index = 0;
	phytbl_info_t tab;
	uint32 lastval;
	int32 lreal, limag;
	uint64 mag;
	txgains->useindex = 1;

	while (1) {
		mid_index = (start_index + stop_index) >> 1;
		txgains->index = (uint8) mid_index;
		/* run papd corresponding to the target pwr */
		wlc_lcnphy_papd_cal(pi, PHY_PAPD_CAL_CW, txgains, 0, 0, 0, 0, 219, 1, 0);
		tab.tbl_id = LCNPHY_TBL_ID_PAPDCOMPDELTATBL;
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

static void
wlc_lcnphy_papd_cal(
	phy_info_t *pi,
	phy_papd_cal_type_t cal_type,
	lcnphy_txcalgains_t *txgains,
	bool frcRxGnCtrl,
	bool txGnCtrl,
	bool samplecapture,
	bool papd_dbg_mode,
	uint8 num_symbols,
	uint8 init_papd_lut,
	uint16 bbmult_step)
{
	uint8 bb_mult_old;
	uint16 AphyControl_old, lcnCtrl3_old;
	uint16 Core1TxControl_old;
	uint16 save_amuxSelPortOverride, save_amuxSelPortOverrideVal;
	uint32 rxGnIdx, val1;
	phytbl_info_t tab, tab1;
	uint16 lcnCalibClkEnCtrl_old;
	uint32 tmpVar;
	uint32 refTxAnGn;
	uint16 lpfbwlut0, lpfbwlut1;
	uint8 CurTxGain;
	lcnphy_txgains_t old_gains;
	uint16 lpf_ofdm_tx_bw;
	uint8 papd_peak_curr_mode = 0;
	int8 maxUpdtIdx, minUpdtIdx, j;
	int32 lut_i;
	bool suspend;
	uint16 save_filtcoeffs[LCNPHY_NUM_DIG_FILT_COEFFS+1];

	uint32 min_papd_lut_val;
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;

	const uint16 savereg2g_addr[] = {
		RADIO_2064_REG116, RADIO_2064_REG12D, RADIO_2064_REG12C, RADIO_2064_REG06A,
		RADIO_2064_REG098, RADIO_2064_REG097, RADIO_2064_REG12F, RADIO_2064_REG00B,
		RADIO_2064_REG113, RADIO_2064_REG01D, RADIO_2064_REG114, RADIO_2064_REG02E,
		RADIO_2064_REG12A, RADIO_2064_REG009, RADIO_2064_REG11F, RADIO_2064_REG007,
		RADIO_2064_REG0FF, RADIO_2064_REG005,
	};
	uint8 savereg2g_data[ARRAYSIZE(savereg2g_addr)];

#ifdef BAND5G
	const uint16 savereg5g_addr[] = {
		RADIO_2064_REG128, RADIO_2064_REG0E7, RADIO_2064_REG0ED, RADIO_2064_REG0EC,
		RADIO_2064_REG0D8, RADIO_2064_REG0DA, RADIO_2064_REG0DC, RADIO_2064_REG127,
		RADIO_2064_REG004, RADIO_2064_REG121, RADIO_2064_REG0D7, RADIO_2064_REG11A,
	};
	uint8 savereg5g_data[ARRAYSIZE(savereg5g_addr)];
#endif // endif

	suspend = FALSE;
	maxUpdtIdx = 1;
	minUpdtIdx = 1;
	ASSERT((cal_type == PHY_PAPD_CAL_CW));

	tab1.tbl_id = LCNPHY_TBL_ID_PAPDCOMPDELTATBL;
	tab1.tbl_ptr = &val1; /* ptr to buf */
	tab1.tbl_width = 32;
	tab1.tbl_len = 1;
	tab1.tbl_offset = 1;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	PHY_PAPD(("Running papd cal, channel: %d cal type: %d\n",
		CHSPEC_CHANNEL(pi->radio_chanspec),
		cal_type));

	bb_mult_old = wlc_lcnphy_get_bbmult(pi); /* PAPD cal can modify this value */

	/* AMUX SEL logic */
	save_amuxSelPortOverride =
		PHY_REG_READ(pi, LCNPHY, TxPwrCtrlRfCtrlOverride0, amuxSelPortOverride);
	save_amuxSelPortOverrideVal =
		PHY_REG_READ(pi, LCNPHY, TxPwrCtrlRfCtrlOverride0, amuxSelPortOverrideVal);

	if (PAPD_4336_MODE(pi_lcn)) {
		PHY_REG_LIST_START
			PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlRfCtrlOverride0,
				amuxSelPortOverride, 1)
			PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlRfCtrlOverride0,
				amuxSelPortOverrideVal, 2)
		PHY_REG_LIST_EXECUTE(pi);
	}
	else {
		PHY_REG_LIST_START
			PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlRfCtrlOverride0,
				amuxSelPortOverride, 1)
			PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlRfCtrlOverride0,
				amuxSelPortOverrideVal, 0)
		PHY_REG_LIST_EXECUTE(pi);
	}

	PHY_REG_LIST_START
		/* Disable filters, turn off PAPD */
		PHY_REG_MOD_ENTRY(LCNPHY, papd_control, papdCompEn, 0)
		PHY_REG_MOD_ENTRY(LCNPHY, sslpnCalibClkEnCtrl, papdTxClkEn, 0)
		PHY_REG_MOD_ENTRY(LCNPHY, txfefilterconfig, cck_papden, 0)
		PHY_REG_MOD_ENTRY(LCNPHY, txfefilterconfig, ofdm_papden, 0)
		PHY_REG_MOD_ENTRY(LCNPHY, txfefilterconfig, ht_papden, 0)
	PHY_REG_LIST_EXECUTE(pi);

	/* Save Digital Filter and set to OFDM Coeffs */
	wlc_lcnphy_save_restore_dig_filt_state(pi, TRUE, save_filtcoeffs);
	/* Different filter is not required , can be removed after verifying in 4336 */
	if (PAPD_4336_MODE(pi_lcn)) {
	    wlc_lcnphy_load_tx_iir_filter(pi, TRUE, 0);
	}

	if (!txGnCtrl) {
		/* Disable CRS/Rx, MAC/Tx and BT */
		wlc_lcnphy_deaf_mode(pi, TRUE);
		/* suspend the mac if it is not already suspended */
		suspend = !(R_REG(pi->sh->osh, &pi->regs->maccontrol) & MCTL_EN_MAC);
		if (!suspend) {
			/* Set non-zero duration for CTS-to-self */
			wlapi_bmac_write_shm(pi->sh->physhim, M_CTS_DURATION, 10000);
			wlapi_suspend_mac_and_wait(pi->sh->physhim);
		}
	}

	/* Set Rx gain override */
	wlc_lcnphy_rx_gain_override_enable(pi, TRUE);

	wlc_lcnphy_tx_pu(pi, TRUE);
	wlc_lcnphy_rx_pu(pi, TRUE);

	/* Save lcnphy filt bw */
	lpfbwlut0 = phy_utils_read_phyreg(pi, LCNPHY_lpfbwlutreg0);
	lpfbwlut1 = phy_utils_read_phyreg(pi, LCNPHY_lpfbwlutreg1);

	/* Widen tx filter */
	/* set filter bandwidth in lpphy_rev0_rf_init, so it's common b/n cal and packets tx */
	/* tones use cck setting, we want to cal with ofdm filter setting */
	if ((CHIPID(pi->sh->chip) == BCM4313_CHIP_ID) && !EPA(pi_lcn)) {
		PHY_REG_LIST_START
			PHY_REG_MOD_ENTRY(LCNPHY, lpfbwlutreg1, lpf_ofdm_tx_bw, 3)
			PHY_REG_MOD_ENTRY(LCNPHY, lpfbwlutreg1, lpf_cck_tx_bw, 3)
		PHY_REG_LIST_EXECUTE(pi);
	} else {
		lpf_ofdm_tx_bw = PHY_REG_READ(pi, LCNPHY, lpfbwlutreg1, lpf_ofdm_tx_bw);
		PHY_REG_MOD(pi, LCNPHY, lpfbwlutreg1, lpf_cck_tx_bw, lpf_ofdm_tx_bw);
	}

	CurTxGain = pi_lcn->lcnphy_current_index;
	wlc_lcnphy_get_tx_gain(pi, &old_gains);

	/* Set tx gain */
	if (txgains) {
		if (txgains->useindex) {
			wlc_lcnphy_set_tx_pwr_by_index(pi, txgains->index);
			CurTxGain = txgains->index;
			PHY_PAPD(("txgainIndex = %d\n", CurTxGain));
			PHY_PAPD(("papd_analog_gain_ovr_val = %d\n",
				PHY_REG_READ(pi, LCNPHY, papd_analog_gain_ovr_val,
				papd_analog_gain_ovr_val)));
		} else {
			wlc_lcnphy_set_tx_gain(pi, &txgains->gains);
		}
	}

	AphyControl_old = phy_utils_read_phyreg(pi, LCNPHY_AphyControlAddr);
	Core1TxControl_old = phy_utils_read_phyreg(pi, LCNPHY_Core1TxControl);
	lcnCtrl3_old = phy_utils_read_phyreg(pi, LCNPHY_sslpnCtrl3);
	lcnCalibClkEnCtrl_old = phy_utils_read_phyreg(pi, LCNPHY_sslpnCalibClkEnCtrl);

	wlc_lcnphy_afe_clk_init(pi, AFE_CLK_INIT_MODE_PAPD);

	PHY_REG_LIST_START
		/* loft comp , iqmm comp enable */
		PHY_REG_OR_ENTRY(LCNPHY, Core1TxControl, 0x0015)
		/* we need to bring SPB out of standby before using it */
		PHY_REG_MOD_ENTRY(LCNPHY, sslpnCtrl3, sram_stby, 0)
		/* enable clk to SPB, PAPD blks and cal */
		PHY_REG_OR_ENTRY(LCNPHY, sslpnCalibClkEnCtrl, 0x8f)
	PHY_REG_LIST_EXECUTE(pi);

	/* Set PAPD reference analog gain */
	tab.tbl_ptr = &refTxAnGn; /* ptr to buf */
	tab.tbl_len = 1;        /* # values   */
	tab.tbl_id = LCNPHY_TBL_ID_TXPWRCTL;
	if (txgains != NULL) {
		tab.tbl_offset = LCNPHY_TX_PWR_CTRL_PWR_OFFSET +
			txgains->index/(1+pi_lcn->virtual_p25_tx_gain_step); /* tbl offset */
		tab.tbl_width = 32;     /* 32 bit wide */
		wlc_lcnphy_read_table(pi, &tab);
	}
	/* format change from x.4 to x.7 */
	refTxAnGn = refTxAnGn * 8;
	PHY_REG_WRITE(pi, LCNPHY, papd_tx_analog_gain_ref, (uint16)refTxAnGn);
	PHY_PAPD(("refTxAnGn = %d\n", refTxAnGn));

	PHY_REG_LIST_START
		/* Force ADC on */
		PHY_REG_MOD_ENTRY(LCNPHY, AfeCtrlOvr, pwdn_adc_ovr, 1)
		PHY_REG_MOD_ENTRY(LCNPHY, AfeCtrlOvrVal, pwdn_adc_ovr_val, 0)
		/* Switch on the PA */
		PHY_REG_MOD_ENTRY(LCNPHY, rfoverride4, papu_ovr, 1)
		PHY_REG_MOD_ENTRY(LCNPHY, rfoverride4val, papu_ovr_val, 1)
	PHY_REG_LIST_EXECUTE(pi);

	/* Save the contents of RF Registers */
	for (j = 0; j < (int8)ARRAYSIZE(savereg2g_addr); j++)
		savereg2g_data[j] = (uint8)phy_utils_read_radioreg(pi, savereg2g_addr[j]);

#ifdef BAND5G
	if (CHSPEC_IS5G(pi->radio_chanspec)) {
		for (j = 0; j < (int8)ARRAYSIZE(savereg5g_addr); j++)
			savereg5g_data[j] = (uint8)phy_utils_read_radioreg(pi, savereg5g_addr[j]);

		/* a band LNA series supply switch */
		phy_utils_mod_radioreg(pi, RADIO_2064_REG121, 0x8, (1<<3));
		phy_utils_mod_radioreg(pi, RADIO_2064_REG0D7, 0x8, (0<<3));
	}

#endif // endif

	/* RF overrides */
	phy_utils_write_radioreg(pi, RADIO_2064_REG116, 0x7); /* rxrf , lna1 , lna2 pwrup */
	if (CHSPEC_IS2G(pi->radio_chanspec))
		phy_utils_write_radioreg(pi, RADIO_2064_REG12D, 0x1); /* tr sw rx pwrup8 */
	/* rx rf pwrup1 ,rf pwrup2,rf pwrup7 */
	phy_utils_write_radioreg(pi, RADIO_2064_REG12C, 0x7);

	/* set up loopback */
	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		/* TIA pwrup,G band mixer up,G band Rx global pwrup,  G band TR switch in Rx mode */
		PHY_REG_LIST_START
			RADIO_REG_WRITE_ENTRY(RADIO_2064_REG06A, 0xc2)
			/* PAPD cal path pwrup */
			RADIO_REG_WRITE_ENTRY(RADIO_2064_REG098, 0xc)
			/* override for papd cal path pwrup */
			RADIO_REG_WRITE_ENTRY(RADIO_2064_REG12F, 0x3)
		PHY_REG_LIST_EXECUTE(pi);

		if (PAPD_4336_MODE(pi_lcn)) {
			if (CHIPID(pi->sh->chip) != BCM43362_CHIP_ID)
				/* atten for papd cal path */
				phy_utils_write_radioreg(pi, RADIO_2064_REG097, 0xf0);
			else
				phy_utils_write_radioreg(pi, RADIO_2064_REG097, 0xf9);
		}
		else
			/* atten for papd cal path */
			phy_utils_write_radioreg(pi, RADIO_2064_REG097, 0xfa);
	} else {
#ifdef BAND5G
		PHY_REG_LIST_START
			/* rx chain pwr ups */
			RADIO_REG_WRITE_ENTRY(RADIO_2064_REG06A, 0x80)
			RADIO_REG_MOD_ENTRY(RADIO_2064_REG0D8, 0x80, (0<<7))
			RADIO_REG_MOD_ENTRY(RADIO_2064_REG0DA, 0x80, (0<<7))
			RADIO_REG_MOD_ENTRY(RADIO_2064_REG128, 0x8, (1<<3))
			RADIO_REG_MOD_ENTRY(RADIO_2064_REG0E7, 0x1, (0<<0))

			RADIO_REG_MOD_ENTRY(RADIO_2064_REG0DC, 0x1, (1<<0))
			RADIO_REG_MOD_ENTRY(RADIO_2064_REG127, 0x10, (1<<4))
			RADIO_REG_MOD_ENTRY(RADIO_2064_REG0E2, 0x1, (1<<0))
			RADIO_REG_MOD_ENTRY(RADIO_2064_REG127, 0x8, (1<<3))

			/* loopback: */
			RADIO_REG_WRITE_ENTRY(RADIO_2064_REG0ED, 0xc)
			RADIO_REG_MOD_ENTRY(RADIO_2064_REG128, 0x2, (1<<1))
			RADIO_REG_MOD_ENTRY(RADIO_2064_REG128, 0x1, (1<<0))

			/* don't force PA off */
			RADIO_REG_MOD_ENTRY(RADIO_2064_REG11A, 0x1, (0<<0))
		PHY_REG_LIST_EXECUTE(pi);
#endif /* BAND5G */
	}

	PHY_REG_LIST_START
		RADIO_REG_WRITE_ENTRY(RADIO_2064_REG00B, 0x7)  /* pu for rx buffer,bw set 25Mhz */
		RADIO_REG_WRITE_ENTRY(RADIO_2064_REG113, 0x10) /* override for pu rx buffer */
		RADIO_REG_WRITE_ENTRY(RADIO_2064_REG01D, 0x1)  /* Pu for Tx buffer */
		RADIO_REG_WRITE_ENTRY(RADIO_2064_REG114, 0x1)  /* override for pu Tx buffer */
	PHY_REG_LIST_EXECUTE(pi);

	if (CHSPEC_IS2G(pi->radio_chanspec))
		phy_utils_write_radioreg(pi, RADIO_2064_REG02E, 0x10); /* logen Rx path enable */
#ifdef BAND5G
	else
		phy_utils_mod_radioreg(pi, RADIO_2064_REG004, 0x4, (1<<2));
#endif // endif

	PHY_REG_LIST_START
		/* override for logen Rx path enable */
		RADIO_REG_WRITE_ENTRY(RADIO_2064_REG12A, 0x8)
		/* enable papd loopback path */
		RADIO_REG_WRITE_ENTRY(RADIO_2064_REG009, 0x2)

		RADIO_REG_MOD_ENTRY(RADIO_2064_REG11F, 0x4, (1<<2))
		RADIO_REG_MOD_ENTRY(RADIO_2064_REG0FF, 0x10, (0<<4))
		RADIO_REG_MOD_ENTRY(RADIO_2064_REG007, 0x1, (0<<0))
		RADIO_REG_MOD_ENTRY(RADIO_2064_REG005, 0x8, (0<<3))
	PHY_REG_LIST_EXECUTE(pi);

	if (RADIOID(pi->pubpi.radioid) != BCM2064_ID) {
#ifdef BAND5G
		if (CHSPEC_IS5G(pi->radio_chanspec)) {
			PHY_REG_LIST_START
				RADIO_REG_MOD_ENTRY(RADIO_2064_REG121, 0x8, (1<<3))
				RADIO_REG_MOD_ENTRY(RADIO_2064_REG0D7, 0x8, (0<<3))
				RADIO_REG_MOD_ENTRY(RADIO_2064_REG0D7, 0x2, (1<<1))
			PHY_REG_LIST_EXECUTE(pi);
		}
		else
#endif // endif
		{
			phy_utils_mod_radioreg(pi, RADIO_2064_REG120, 0x40, (1<<6));
		}
	}
	if (LCNREV_LT(pi->pubpi.phy_rev, 2))
		PHY_REG_WRITE(pi, LCNPHY, AphyControlAddr, 0x0001);
	else
		PHY_REG_MOD(pi, LCNPHY, sslpnCalibClkEnCtrl, forceTxfiltClkOn, 1);

	/* Do Rx Gain Control */
	rxGnIdx = wlc_lcnphy_papd_rxGnCtrl(pi, cal_type, frcRxGnCtrl, CurTxGain);

	/* Set Rx Gain */
	wlc_lcnphy_set_rx_gain_by_distribution(pi, 1, 0, 0, 0, (uint16)rxGnIdx, 0, 0, 0);

	if (bbmult_step == 0)
		bbmult_step = 16640;
	/* Do PAPD Operation - All symbols in one go */
	wlc_lcnphy_papd_cal_core(pi, cal_type,
		FALSE,
		txGnCtrl,
		samplecapture,
		papd_dbg_mode,
		num_symbols,
		init_papd_lut,
		1400,
		bbmult_step,
		0,
		128,
		0);

	/* CSP 536770 */
	/* If AM/AM comp at index 0 is very large reduce TIA gain and recal, */
	/* especially for 43362 Nikon */
	if (pi_lcn->pacalfcd1) {
		while (rxGnIdx > 0) {
			wlc_lcnphy_read_table(pi, &tab1);
			lut_i = (val1 & 0x00fff000) << 8;
			lut_i = lut_i >> 20;
			if (lut_i > 135)
				rxGnIdx--;
			else
				break;
			wlc_lcnphy_set_rx_gain_by_distribution(pi, 1, 0, 0,
				0, (uint16)rxGnIdx, 0, 0, 0);
			wlc_lcnphy_papd_cal_core(pi, cal_type,
				FALSE,
				txGnCtrl,
				samplecapture,
				papd_dbg_mode,
				num_symbols,
				init_papd_lut,
				1400,
				bbmult_step,
				0,
				128,
				0);
		}
	}

	wlc_lcnphy_GetpapdMaxMinIdxupdt(pi, &maxUpdtIdx, &minUpdtIdx);

	PHY_PAPD(("wl%d: %s max: %d, min: %d\n",
		pi->sh->unit, __FUNCTION__, maxUpdtIdx, minUpdtIdx));

	if ((minUpdtIdx >= 0) && (minUpdtIdx < PHY_PAPD_EPS_TBL_SIZE_LCNPHY) &&
		(maxUpdtIdx >= 0) && (maxUpdtIdx < PHY_PAPD_EPS_TBL_SIZE_LCNPHY) &&
		(minUpdtIdx <= maxUpdtIdx)) {

		if (cal_type == PHY_PAPD_CAL_CW) {
			tab.tbl_id = LCNPHY_TBL_ID_PAPDCOMPDELTATBL;
			tab.tbl_offset = minUpdtIdx;
			tab.tbl_ptr = &tmpVar; /* ptr to buf */
			wlc_lcnphy_read_table(pi, &tab);
			min_papd_lut_val = tmpVar;
			tab.tbl_offset = maxUpdtIdx;
			wlc_lcnphy_read_table(pi, &tab);
			for (j = 0; j < minUpdtIdx; j++) {
				tmpVar = min_papd_lut_val;
				tab.tbl_offset = j;
				tab.tbl_ptr = &tmpVar;
				wlc_lcnphy_write_table(pi, &tab);
			}
		}

		PHY_PAPD(("wl%d: %s: PAPD cal completed\n", pi->sh->unit, __FUNCTION__));
		if (papd_peak_curr_mode == 0) {
			tab.tbl_id = LCNPHY_TBL_ID_PAPDCOMPDELTATBL;
			tab.tbl_offset = 62;
			tab.tbl_ptr = &tmpVar;
			wlc_lcnphy_read_table(pi, &tab);
			tab.tbl_offset = 63;
			wlc_lcnphy_write_table(pi, &tab);
			tab.tbl_offset = 1;
			wlc_lcnphy_read_table(pi, &tab);
			tab.tbl_offset = 0;
			wlc_lcnphy_write_table(pi, &tab);
		}

		wlc_lcnphy_save_papd_calibration_results(pi);
	}
	else
		PHY_PAPD(("Error in PAPD Cal. Exiting... \n"));

	/* restore saved registers */
	PHY_REG_WRITE(pi, LCNPHY, lpfbwlutreg0, lpfbwlut0);
	PHY_REG_WRITE(pi, LCNPHY, lpfbwlutreg1, lpfbwlut1);

	PHY_REG_LIST_START
		RADIO_REG_MOD_ENTRY(RADIO_2064_REG0D7, 0x2, (0<<1))
		RADIO_REG_MOD_ENTRY(RADIO_2064_REG0D7, 0x8, (1<<3))
		RADIO_REG_MOD_ENTRY(RADIO_2064_REG128, 0x8, (0<<3))
		RADIO_REG_WRITE_ENTRY(RADIO_2064_REG116, 0x0)
	PHY_REG_LIST_EXECUTE(pi);

	PHY_REG_WRITE(pi, LCNPHY, AphyControlAddr, AphyControl_old);
	PHY_REG_WRITE(pi, LCNPHY, Core1TxControl, Core1TxControl_old);
	PHY_REG_WRITE(pi, LCNPHY, sslpnCtrl3, lcnCtrl3_old);

	/* restore calib ctrl clk */
	/* switch on PAPD clk */
	PHY_REG_WRITE(pi, LCNPHY, sslpnCalibClkEnCtrl, lcnCalibClkEnCtrl_old | 0x1);

	wlc_lcnphy_afe_clk_init(pi, AFE_CLK_INIT_MODE_TXRX2X);

	/* Restore AMUX sel */
	PHY_REG_MOD(pi, LCNPHY, TxPwrCtrlRfCtrlOverride0,
		amuxSelPortOverride, save_amuxSelPortOverride);
	PHY_REG_MOD(pi, LCNPHY, TxPwrCtrlRfCtrlOverride0,
		amuxSelPortOverrideVal, save_amuxSelPortOverrideVal);

	/* TR switch */
	wlc_lcnphy_clear_trsw_override(pi);

	/* Restore rx path mux and turn off PAPD mixer, bias filter settings */
	for (j = 0; j < (int8)ARRAYSIZE(savereg2g_addr); j++)
		phy_utils_write_radioreg(pi, savereg2g_addr[j], savereg2g_data[j]);

#ifdef BAND5G
	if (CHSPEC_IS5G(pi->radio_chanspec)) {
	  for (j = 0; j < (int8)ARRAYSIZE(savereg5g_addr); j++)
			phy_utils_write_radioreg(pi, savereg5g_addr[j], savereg5g_data[j]);
	}
#endif // endif

	/* Clear rx PU override */
	PHY_REG_MOD(pi, LCNPHY, RFOverride0, internalrfrxpu_ovr, 0);

	wlc_lcnphy_tx_pu(pi, FALSE);

	/* Clear rx gain override */
	wlc_lcnphy_rx_gain_override_enable(pi, FALSE);

	/* Clear ADC override */
	PHY_REG_MOD(pi, LCNPHY, AfeCtrlOvr, pwdn_adc_ovr, 0);
	/* Clear PA override */
	PHY_REG_MOD(pi, LCNPHY, rfoverride4, papu_ovr, 0);

	/* Restore CRS */
	wlc_lcnphy_deaf_mode(pi, FALSE);

	/* Restore Digital Filter */
	wlc_lcnphy_save_restore_dig_filt_state(pi, FALSE, save_filtcoeffs);

	PHY_REG_LIST_START
		/* Enable PAPD */
		PHY_REG_MOD_ENTRY(LCNPHY, papd_control, papdCompEn, 1)
		PHY_REG_MOD_ENTRY(LCNPHY, sslpnCalibClkEnCtrl, papdTxClkEn, 1)
		PHY_REG_MOD_ENTRY(LCNPHY, txfefilterconfig, cck_papden, 1)
		PHY_REG_MOD_ENTRY(LCNPHY, txfefilterconfig, ofdm_papden, 1)
		PHY_REG_MOD_ENTRY(LCNPHY, txfefilterconfig, ht_papden, 1)
	PHY_REG_LIST_EXECUTE(pi);

	if (!suspend)
		wlapi_enable_mac(pi->sh->physhim);

	/* restore bbmult */
	wlc_lcnphy_set_bbmult(pi, (uint8)bb_mult_old);
}

static void
wlc_lcnphy_reset_iir_filter(phy_info_t *pi)
{
	PHY_REG_LIST_START
		PHY_REG_MOD_ENTRY(LCNPHY, sslpnCalibClkEnCtrl, forceTxfiltClkOn, 1)
		PHY_REG_MOD_ENTRY(LCNPHY, sslpnCtrl0, txSoftReset, 1)
		PHY_REG_MOD_ENTRY(LCNPHY, sslpnCtrl0, txSoftReset, 0)
		PHY_REG_MOD_ENTRY(LCNPHY, sslpnCalibClkEnCtrl, forceTxfiltClkOn, 0)
	PHY_REG_LIST_EXECUTE(pi);
}

#if defined(WLTEST)
void
wlc_phy_get_rxgainerr_lcnphy(phy_info_t *pi, int16 *gainerr)
{
	/* Returns rxgain error (read from srom) for current channel */
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;

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
wlc_phy_get_SROMnoiselvl_lcnphy(phy_info_t *pi, int8 *noiselvl)
{
	/* Returns noise level (read from srom) for current channel */
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;

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
wlc_phy_get_noiseoffset_lcnphy(phy_info_t *pi, int16 *noiseoff)
{
	/* Returns noise level (read from srom) for current channel */
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;

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
#endif  /* #if defined(WLTEST) */

/*
* Get Rx IQ Imbalance Estimate from modem
*/
static bool
wlc_lcnphy_rx_iq_est(phy_info_t *pi,
	uint16 num_samps,
	uint8 wait_time,
	uint8 wait_for_crs,
	phy_iq_est_t *iq_est,
	uint16 timeout_ms)
{
	int wait_count = 0;
	bool result = TRUE;

	PHY_REG_LIST_START
		/* Turn on clk to Rx IQ */
		PHY_REG_MOD_ENTRY(LCNPHY, sslpnCalibClkEnCtrl, iqEstClkEn, 1)
		/* Force OFDM receiver on */
		PHY_REG_MOD_ENTRY(LCNPHY, crsgainCtrl, APHYGatingEnable, 0)
	PHY_REG_LIST_EXECUTE(pi);

	PHY_REG_MOD(pi, LCNPHY, IQNumSampsAddress, numSamps, num_samps);
	PHY_REG_MOD(pi, LCNPHY, IQEnableWaitTimeAddress, waittimevalue,
		(uint16)wait_time);

	if (!wait_for_crs)
		PHY_REG_MOD(pi, LCNPHY, IQEnableWaitTimeAddress, iqmode, 0);

	PHY_REG_MOD(pi, LCNPHY, IQEnableWaitTimeAddress, iqstart, 1);

	/* Wait for IQ estimation to complete */
	while (phy_utils_read_phyreg(pi, LCNPHY_IQEnableWaitTimeAddress) &
		LCNPHY_IQEnableWaitTimeAddress_iqstart_MASK) {
		/* Check for timeout */
		if (wait_count > (10 * timeout_ms)) { /* timeout in ms */
			PHY_ERROR(("wl%d: %s: IQ estimation failed to complete\n",
				pi->sh->unit, __FUNCTION__));
			result = FALSE;
			goto cleanup;
		}
		OSL_DELAY(100);
		wait_count++;
	}

	/* Save results */
	iq_est->iq_prod = ((uint32)phy_utils_read_phyreg(pi, LCNPHY_IQAccHiAddress) << 16) |
		(uint32)phy_utils_read_phyreg(pi, LCNPHY_IQAccLoAddress);
	iq_est->i_pwr = ((uint32)phy_utils_read_phyreg(pi, LCNPHY_IQIPWRAccHiAddress) << 16) |
		(uint32)phy_utils_read_phyreg(pi, LCNPHY_IQIPWRAccLoAddress);
	iq_est->q_pwr = ((uint32)phy_utils_read_phyreg(pi, LCNPHY_IQQPWRAccHiAddress) << 16) |
		(uint32)phy_utils_read_phyreg(pi, LCNPHY_IQQPWRAccLoAddress);
	PHY_TMP(("wl%d: %s: IQ estimation completed in %d us,"
		"i_pwr: %d, q_pwr: %d, iq_prod: %d\n",
		pi->sh->unit, __FUNCTION__,
		wait_count * 100, iq_est->i_pwr, iq_est->q_pwr, iq_est->iq_prod));
cleanup:
	PHY_REG_LIST_START
		PHY_REG_MOD_ENTRY(LCNPHY, crsgainCtrl, APHYGatingEnable, 1)
		PHY_REG_MOD_ENTRY(LCNPHY, sslpnCalibClkEnCtrl, iqEstClkEn, 0)
	PHY_REG_LIST_EXECUTE(pi);

	return result;
}

#if defined(WLTEST)

static void
wlc_lcnphy_set_lna_freq_comp(phy_info_t *pi)
{
	uint8 lna_freq_tune;

	switch (wlc_phy_chanspec_bandrange_get(pi, pi->radio_chanspec)) {
		case WL_CHAN_FREQ_RANGE_2G:
			lna_freq_tune = 0x00;
			break;
	#ifdef BAND5G
		case WL_CHAN_FREQ_RANGE_5GL:
			/* 5 GHz low */
			lna_freq_tune = 0xDB;
			break;

		case WL_CHAN_FREQ_RANGE_5GM:
			/* 5 GHz middle */
			lna_freq_tune = 0x00;
			break;

		case WL_CHAN_FREQ_RANGE_5GH:
			/* 5 GHz high */
			lna_freq_tune = 0x32;
			break;
	#endif /* BAND5G */
		default:
			lna_freq_tune = 0x00;
			break;
	}
	phy_utils_write_radioreg(pi, RADIO_2064_REG0AD, lna_freq_tune);
}

void
wlc_lcnphy_get_lna_freq_correction(phy_info_t *pi, int8 *freq_offset_fact)
{
	uint i;
	uint8 channel = CHSPEC_CHANNEL(pi->radio_chanspec);

	if (RADIOID(pi->pubpi.radioid) == BCM2066_ID) {
		for (i = 0; i < ARRAYSIZE(chan_info_2066_lcnphy_lna_corr); i++)
			if (chan_info_2066_lcnphy_lna_corr[i].chan == channel)
				break;

		if (i >= ARRAYSIZE(chan_info_2066_lcnphy_lna_corr)) {
			PHY_ERROR(("wl%d: %s: channel %d not found in channel table\n",
				pi->sh->unit, __FUNCTION__, channel));
			return;
		}
		*freq_offset_fact = chan_info_2066_lcnphy_lna_corr[i].corr_qdBm;
	} else
		*freq_offset_fact = 0;
}

void
wlc_lcnphy_rx_power(phy_info_t *pi, uint16 num_samps, uint8 wait_time,
	uint8 wait_for_crs, phy_iq_est_t* est)
{
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	uint32 rx_pwr = 0, prev_rx_pwr = 0;
	uint16 biq2, biq1, digi;
	uint16 trsw, ext_lna, tia, lna2, lna1;
	bool rxtestcond, iq_done, loop_cont_cond, threshcond;
	uint16 rf_override_2;
	uint32 adj_min, adj_max;
	uint32 adj_plusdB, adj_mindB;
	uint8 counter = 0, step = 0;
	bool suspend;
	uint8 retry_count;
	uint16 lna_freq_tune;
	uint8 count;
	uint16 reduced_nsamples = 1 << 10;
	uint32 pwr_thresh;
	uint32 sample_adjust;
	uint32 i_pwr_red, q_pwr_red;
	uint8 timeout_count = 8;
	uint8 reduced_timeout = 1;
	uint8 full_timeout = 20;

	suspend = !(R_REG(pi->sh->osh, &pi->regs->maccontrol) & MCTL_EN_MAC);
	if (!suspend)
		wlapi_suspend_mac_and_wait(pi->sh->physhim);

	pi_lcn->noise.tainted = TRUE;

	rf_override_2 = phy_utils_read_phyreg(pi, LCNPHY_rfoverride2);
	lna_freq_tune = phy_utils_read_radioreg(pi, RADIO_2064_REG0AD);

	wlc_lcnphy_deaf_mode(pi, FALSE);

	/* Force the TR switch to Receive */
	wlc_lcnphy_set_trsw_override(pi, FALSE, TRUE);

	wlc_lcnphy_rx_pu(pi, TRUE);

	PHY_REG_LIST_START
		/* ADC */
		PHY_REG_MOD_ENTRY(LCNPHY, AfeCtrlOvr, pwdn_adc_ovr, 1)
		PHY_REG_MOD_ENTRY(LCNPHY, AfeCtrlOvrVal, pwdn_adc_ovr_val, 1)
		PHY_REG_MOD_ENTRY(LCNPHY, AfeCtrlOvrVal, pwdn_adc_ovr_val, 0)
		PHY_REG_MOD_ENTRY(LCNPHY, AfeCtrlOvr, pwdn_adc_ovr, 1)

		PHY_REG_MOD_ENTRY(LCNPHY, rfoverride2val, hpf1_ctrl_ovr_val, 7)
		PHY_REG_MOD_ENTRY(LCNPHY, rfoverride2val, hpf2_ctrl_ovr_val, 7)
		PHY_REG_MOD_ENTRY(LCNPHY, rfoverride2val, lpf_lq_ovr_val, 0)
		PHY_REG_MOD_ENTRY(LCNPHY, rfoverride2, hpf1_ctrl_ovr, 1)
		PHY_REG_MOD_ENTRY(LCNPHY, rfoverride2, hpf2_ctrl_ovr, 1)
		PHY_REG_MOD_ENTRY(LCNPHY, rfoverride2, lpf_lq_ovr, 1)
	PHY_REG_LIST_EXECUTE(pi);

	wlc_lcnphy_set_lna_freq_comp(pi);

	pi_lcn->rxpath_final_power = 0;
	pi_lcn->rxpath_gainselect_power = 0;
	pi_lcn->rxpath_status = 0;
	est->i_pwr = 0;
	est->q_pwr = 0;

	trsw = 0;
	ext_lna = 1;

	if (CHSPEC_IS5G(pi->radio_chanspec)) {
			tia = 1;
			lna2 = 2;
			lna1 = 3;
			retry_count = 2;
			pwr_thresh = pi_lcn->pwr_thresh_5g;

	} else {
			tia = 3;
			lna2 = 3;
			lna1 = 5;
			retry_count = 2;
			pwr_thresh = pi_lcn->pwr_thresh_2g;
	}

	while (counter < retry_count)
	{
		prev_rx_pwr = 0;
		step = 0;

		if (CHSPEC_IS5G(pi->radio_chanspec)) {
				biq2 = 4;
				biq1 = 6;
				digi = 6;
		} else {
				biq2 = 4;
				biq1 = 6;
				digi = 4;
		}

		do {
			step++;

			wlc_lcnphy_set_rx_gain_by_distribution(pi, trsw, ext_lna,
				biq2, biq1, tia, lna2, lna1, digi);

			wlc_lcnphy_rx_gain_override_enable(pi, TRUE);

			wlc_lcnphy_agc_reset(pi);

			/* Toggle ADC here to get rid of random errors */
			PHY_REG_LIST_START
				PHY_REG_MOD_ENTRY(LCNPHY, AfeCtrlOvr, pwdn_adc_ovr, 1)
				PHY_REG_MOD_ENTRY(LCNPHY, AfeCtrlOvrVal, pwdn_adc_ovr_val, 1)
			PHY_REG_LIST_EXECUTE(pi);

			OSL_DELAY(10);
			PHY_REG_LIST_START
				PHY_REG_MOD_ENTRY(LCNPHY, AfeCtrlOvrVal, pwdn_adc_ovr_val, 0)
				PHY_REG_MOD_ENTRY(LCNPHY, AfeCtrlOvr, pwdn_adc_ovr, 1)
			PHY_REG_LIST_EXECUTE(pi);
			OSL_DELAY(10);

			iq_done = FALSE;
			for (count = 0; count < timeout_count; count++) {
				iq_done = wlc_lcnphy_rx_iq_est(pi, reduced_nsamples, wait_time,
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
			pi_lcn->rxpath_gain = (int16) wlc_lcnphy_calc_rx_gain(pi) + 3*digi;

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

			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				if (biq1)
					biq1 = biq1 - 1;
				else if (biq2)
					biq2 = biq2 - 1;
				else
					break;
			} else {
				if (digi)
					digi = digi - 1;
				else
					break;
			}

			if (pi_lcn->rxpath_steps)
				loop_cont_cond = step < pi_lcn->rxpath_steps;
			else
				loop_cont_cond = (!rxtestcond) && (!threshcond);
		} while (loop_cont_cond);

		if ((rxtestcond) || (threshcond) || (pi_lcn->rxpath_steps)) {
			break;
		}
		else {
			tia = tia - 1;
			counter++;
		}
	}

	iq_done = FALSE;
	for (count = 0; count < timeout_count; count++) {
		iq_done = wlc_lcnphy_rx_iq_est(pi, num_samps, wait_time,
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

	PHY_REG_MOD(pi, LCNPHY, RFOverride0, internalrfrxpu_ovr, 0);
	PHY_REG_MOD(pi, LCNPHY, AfeCtrlOvr, pwdn_adc_ovr, 0);
	wlc_lcnphy_rx_gain_override_enable(pi, FALSE);
	wlc_lcnphy_clear_trsw_override(pi);
	phy_utils_write_phyreg(pi, LCNPHY_rfoverride2, rf_override_2);
	phy_utils_write_radioreg(pi, RADIO_2064_REG0AD, lna_freq_tune);

	if (!suspend)
		wlapi_enable_mac(pi->sh->physhim);

}
#endif  /* #if defined(WLTEST) */

/*
* Compute Rx compensation coeffs
*   -- run IQ est and calculate compensation coefficients
*/
static bool
wlc_lcnphy_calc_rx_iq_comp(phy_info_t *pi,  uint16 num_samps)
{
#define LCNPHY_MIN_RXIQ_PWR 2
	bool result;
	uint16 a0_new, b0_new;
	phy_iq_est_t iq_est = {0, 0, 0};
	int32  a, b, temp;
	int16  iq_nbits, qq_nbits, arsh, brsh;
	int32  iq;
	uint32 ii, qq;
#if defined(PHYCAL_CACHING)
	lcnphy_calcache_t *cache;
	ch_calcache_t *ctx = wlc_phy_get_chanctx(pi, pi->radio_chanspec);
	if (ctx == NULL) {
		PHY_ERROR(("%s: NULL ctx - bail\n", __FUNCTION__));
		return FALSE;
	}
	cache = &ctx->u.lcnphy_cache;
#else
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
#endif // endif

	/* Save original c0 & c1 */
	a0_new = ((phy_utils_read_phyreg(pi, LCNPHY_RxCompcoeffa0) &
	           LCNPHY_RxCompcoeffa0_a0_MASK) >>
		LCNPHY_RxCompcoeffa0_a0_SHIFT);
	b0_new = ((phy_utils_read_phyreg(pi, LCNPHY_RxCompcoeffb0) &
	           LCNPHY_RxCompcoeffb0_b0_MASK) >>
		LCNPHY_RxCompcoeffb0_b0_SHIFT);

	PHY_REG_LIST_START
		PHY_REG_MOD_ENTRY(LCNPHY, rxfe, bypass_iqcomp, 0)
		PHY_REG_MOD_ENTRY(LCNPHY, RxIqCoeffCtrl, RxIqComp11bEn, 1)
	PHY_REG_LIST_EXECUTE(pi);

	/* Zero out comp coeffs and do "one-shot" calibration */
	wlc_lcnphy_set_rx_iq_comp(pi, 0, 0);

	if (!(result = wlc_lcnphy_rx_iq_est(pi, num_samps, 32, 0, &iq_est, RXIQEST_TIMEOUT)))
		goto cleanup;

	iq = (int32)iq_est.iq_prod;
	ii = iq_est.i_pwr;
	qq = iq_est.q_pwr;

	/* bounds check estimate info */
	if ((ii + qq) < LCNPHY_MIN_RXIQ_PWR) {
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
	a0_new = (uint16)(a & 0x3ff);
	b0_new = (uint16)(b & 0x3ff);

cleanup:
	/* Apply new coeffs */
	wlc_lcnphy_set_rx_iq_comp(pi, a0_new, b0_new);
	PHY_REG_LIST_START
		/* enabling the hardware override to choose only a0, b0 coeff */
		PHY_REG_MOD_ENTRY(LCNPHY, RxIqCoeffCtrl, RxIqCrsCoeffOverRide, 1)
		PHY_REG_MOD_ENTRY(LCNPHY, RxIqCoeffCtrl, RxIqCrsCoeffOverRide11b, 1)
	PHY_REG_LIST_EXECUTE(pi);

#if defined(PHYCAL_CACHING)
	cache->rxiqcal_coeff_a0 = a0_new;
	cache->rxiqcal_coeff_b0 = b0_new;
#else
	pi_lcn->lcnphy_cal_results.rxiqcal_coeff_a0 = a0_new;
	pi_lcn->lcnphy_cal_results.rxiqcal_coeff_b0 = b0_new;
#endif // endif

	return result;
}

/*
* RX IQ Calibration
*/
static bool
wlc_lcnphy_rx_iq_cal(phy_info_t *pi, const lcnphy_rx_iqcomp_t *iqcomp, int iqcomp_sz,
	int module, int tx_gain_idx)
{
	lcnphy_txgains_t old_gains;
	uint16 tx_pwr_ctrl;
	uint8 tx_gain_index_old = 0;
	bool result = FALSE, tx_gain_override_old = FALSE;
	uint16 i, Core1TxControl_old, RFOverride0_old,
	RFOverrideVal0_old, rfoverride2_old, rfoverride2val_old,
	rfoverride3_old, rfoverride3val_old, rfoverride4_old,
	old_rxlnaandgainctrl1ovrval,
	rfoverride4val_old, afectrlovr_old, afectrlovrval_old;
	int tia_gain, lna2_gain, biq1_gain, biq2_gain;
	uint16 old_sslpnCalibClkEnCtrl, old_sslpnRxFeClkEnCtrl;
	uint16 values_to_save[ARRAYSIZE(rxiq_cal_rf_reg)];
	int16 *ptr;
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	bool set_gain = FALSE;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	if ((ptr = MALLOC(pi->sh->osh, sizeof(int16) * 131)) == NULL) {
		PHY_ERROR(("wl%d: %s: MALLOC failure\n", pi->sh->unit, __FUNCTION__));
		return FALSE;
	}

	if (EPA(pi->u.pi_lcnphy))
		wlc_lcnphy_epa_pd(pi, 1);

	if (module == 2) {
		ASSERT(iqcomp_sz);

		while (iqcomp_sz--) {
			if (iqcomp[iqcomp_sz].chan == CHSPEC_CHANNEL(pi->radio_chanspec)) {
				/* Apply new coeffs */
				wlc_lcnphy_set_rx_iq_comp(pi, (uint16)iqcomp[iqcomp_sz].a,
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
		tx_pwr_ctrl = wlc_lcnphy_get_tx_pwr_ctrl(pi);
		wlc_lcnphy_set_tx_pwr_ctrl(pi, LCNPHY_TX_PWR_CTRL_OFF);
		/* turn off papd */
		/* save rf register states */
		for (i = 0; i < ARRAYSIZE(rxiq_cal_rf_reg); i++) {
			values_to_save[i] = phy_utils_read_radioreg(pi, rxiq_cal_rf_reg[i]);
		}
		Core1TxControl_old = phy_utils_read_phyreg(pi, LCNPHY_Core1TxControl);
		/* loft comp , iqmm comp enable */
		PHY_REG_OR(pi, LCNPHY, Core1TxControl, 0x0015);
		/* store old values to be restored */
		RFOverride0_old = phy_utils_read_phyreg(pi, LCNPHY_RFOverride0);
		RFOverrideVal0_old = phy_utils_read_phyreg(pi, LCNPHY_RFOverrideVal0);
		rfoverride2_old = phy_utils_read_phyreg(pi, LCNPHY_rfoverride2);
		rfoverride2val_old = phy_utils_read_phyreg(pi, LCNPHY_rfoverride2val);
		rfoverride3_old = phy_utils_read_phyreg(pi, LCNPHY_rfoverride3);
		rfoverride3val_old = phy_utils_read_phyreg(pi, LCNPHY_rfoverride3_val);
		rfoverride4_old = phy_utils_read_phyreg(pi, LCNPHY_rfoverride4);
		rfoverride4val_old = phy_utils_read_phyreg(pi, LCNPHY_rfoverride4val);
		afectrlovr_old = phy_utils_read_phyreg(pi, LCNPHY_AfeCtrlOvr);
		afectrlovrval_old = phy_utils_read_phyreg(pi, LCNPHY_AfeCtrlOvrVal);
		old_sslpnCalibClkEnCtrl = phy_utils_read_phyreg(pi, LCNPHY_sslpnCalibClkEnCtrl);
		old_sslpnRxFeClkEnCtrl = phy_utils_read_phyreg(pi, LCNPHY_sslpnRxFeClkEnCtrl);
		old_rxlnaandgainctrl1ovrval =
		        phy_utils_read_phyreg(pi, LCNPHY_rxlnaandgainctrl1ovrval);
		/* Save old tx gain settings */
		tx_gain_override_old = wlc_lcnphy_tx_gain_override_enabled(pi);
		if (tx_gain_override_old) {
			wlc_lcnphy_get_tx_gain(pi, &old_gains);
			tx_gain_index_old = pi_lcn->lcnphy_current_index;
		}
		/* Apply new tx gain */
		wlc_lcnphy_set_tx_pwr_by_index(pi, tx_gain_idx);

		PHY_REG_LIST_START
			/* PA override */
			PHY_REG_MOD_ENTRY(LCNPHY, rfoverride3, stxpapu_ovr, 1)
			PHY_REG_MOD_ENTRY(LCNPHY, rfoverride3_val, stxpapu_ovr_val, 0)
		PHY_REG_LIST_EXECUTE(pi);

		/* radio regs */
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			phy_utils_write_radioreg(pi, RADIO_2064_REG098, 0x03);
		}
#ifdef BAND5G
		else {
			/* set up Rx IQCal loopback */
			phy_utils_write_radioreg(pi, RADIO_2064_REG0ED, 0x3);
			phy_utils_write_radioreg(pi, RADIO_2064_REG0EC, (0 << 4));
		}
#endif // endif

		PHY_REG_LIST_START
			/* Force RX/TX pu */
			PHY_REG_MOD2_ENTRY(LCNPHY, RFOverride0, internalrfrxpu_ovr,
				internalrftxpu_ovr, 1, 1)
			PHY_REG_MOD2_ENTRY(LCNPHY, RFOverrideVal0, internalrfrxpu_ovr_val,
				internalrftxpu_ovr_val, 1, 1)
			/* Filter programming as per the Rx IQCAL Mode A (LPF in Rx mode) */
			PHY_REG_MOD_RAW_ENTRY(LCNPHY_rfoverride4,
				(LCNPHY_rfoverride4_lpf_sel_tx_rx_ovr_MASK |
				LCNPHY_rfoverride4_lpf_pwrup_ovr_MASK |
				LCNPHY_rfoverride4_lpf_buf_pwrup_ovr_MASK |
				LCNPHY_rfoverride4_lpf_tx_buf_pwrup_ovr_MASK |
				LCNPHY_rfoverride4_lpf_dc1_pwrup_ovr_MASK |
				LCNPHY_rfoverride4_lpf_byp_dc_ovr_MASK |
				LCNPHY_rfoverride4_lpf_byp_rx_ovr_MASK |
				LCNPHY_rfoverride4_lpf_byp_tx_ovr_MASK |
				LCNPHY_rfoverride4_lpf_byp_dac_buf_ovr_MASK),
				((1 << LCNPHY_rfoverride4_lpf_sel_tx_rx_ovr_SHIFT) |
				(1 << LCNPHY_rfoverride4_lpf_pwrup_ovr_SHIFT) |
				(1 << LCNPHY_rfoverride4_lpf_buf_pwrup_ovr_SHIFT) |
				(1 << LCNPHY_rfoverride4_lpf_tx_buf_pwrup_ovr_SHIFT) |
				(1 << LCNPHY_rfoverride4_lpf_dc1_pwrup_ovr_SHIFT) |
				(1 << LCNPHY_rfoverride4_lpf_byp_dc_ovr_SHIFT) |
				(1 << LCNPHY_rfoverride4_lpf_byp_rx_ovr_SHIFT) |
				(1 << LCNPHY_rfoverride4_lpf_byp_tx_ovr_SHIFT) |
				(1 << LCNPHY_rfoverride4_lpf_byp_dac_buf_ovr_SHIFT)))
			PHY_REG_MOD_RAW_ENTRY(LCNPHY_rfoverride4val,
				(LCNPHY_rfoverride4val_lpf_sel_tx_rx_ovr_val_MASK |
				LCNPHY_rfoverride4val_lpf_pwrup_override_val_MASK |
				LCNPHY_rfoverride4val_lpf_buf_pwrup_ovr_val_MASK |
				LCNPHY_rfoverride4val_lpf_tx_buf_pwrup_ovr_val_MASK |
				LCNPHY_rfoverride4val_lpf_dc1_pwrup_ovr_val_MASK |
				LCNPHY_rfoverride4val_lpf_byp_dc_ovr_val_MASK |
				LCNPHY_rfoverride4val_lpf_byp_rx_ovr_val_MASK |
				LCNPHY_rfoverride4val_lpf_byp_tx_ovr_val_MASK |
				LCNPHY_rfoverride4val_lpf_byp_dac_buf_ovr_val_MASK),
				((0 << LCNPHY_rfoverride4val_lpf_sel_tx_rx_ovr_val_SHIFT) |
				(1 << LCNPHY_rfoverride4val_lpf_pwrup_override_val_SHIFT) |
				(1 << LCNPHY_rfoverride4val_lpf_buf_pwrup_ovr_val_SHIFT) |
				(1 << LCNPHY_rfoverride4val_lpf_tx_buf_pwrup_ovr_val_SHIFT) |
				(1 << LCNPHY_rfoverride4val_lpf_dc1_pwrup_ovr_val_SHIFT) |
				(0 << LCNPHY_rfoverride4val_lpf_byp_dc_ovr_val_SHIFT) |
				(0 << LCNPHY_rfoverride4val_lpf_byp_rx_ovr_val_SHIFT) |
				(1 << LCNPHY_rfoverride4val_lpf_byp_tx_ovr_val_SHIFT) |
				(0 << LCNPHY_rfoverride4val_lpf_byp_dac_buf_ovr_val_SHIFT)))

			/* Force DAC/ADC on */
			PHY_REG_MOD2_ENTRY(LCNPHY, AfeCtrlOvr, pwdn_adc_ovr,
				pwdn_dac_ovr, 1, 1)
			PHY_REG_MOD2_ENTRY(LCNPHY, AfeCtrlOvrVal, pwdn_adc_ovr_val,
				pwdn_dac_ovr_val, 0, 0)

			/* Run calibration */
			PHY_REG_WRITE_ENTRY(LCNPHY, sslpnCalibClkEnCtrl, 0xffff)
			PHY_REG_OR_ENTRY(LCNPHY, sslpnRxFeClkEnCtrl, 0x3)
			/* adjust rx power */
			PHY_REG_MOD_ENTRY(LCNPHY, rxfe, bypass_iqcomp, 1)
		PHY_REG_LIST_EXECUTE(pi);

		#define k_lcnphy_rx_iq_est_tone_amplitude 120
		#define k_lcnphy_rx_iq_est_n_samps 1024

		/* Power down lna1 and turn on kill switch */
		PHY_REG_LIST_START
			PHY_REG_MOD_ENTRY(LCNPHY, rxlnaandgainctrl1ovrval, lnapuovr_Val, 0x6c)
			PHY_REG_MOD_ENTRY(LCNPHY, rfoverride2val, slna_pu_ovr_val, 0)
			PHY_REG_MOD_ENTRY(LCNPHY, rfoverride4val, trsw_rx_pwrup_ovr_val, 0)
			PHY_REG_MOD_ENTRY(LCNPHY, rfoverride4, trsw_rx_pwrup_ovr, 1)
			PHY_REG_MOD_ENTRY(LCNPHY, rfoverride2, lna_pu_ovr, 1)
			PHY_REG_MOD_ENTRY(LCNPHY, rfoverride2, slna_pu_ovr, 1)
			PHY_REG_MOD_ENTRY(LCNPHY, rfoverride4val, papu_ovr_val, 0)
			PHY_REG_MOD_ENTRY(LCNPHY, rfoverride4, papu_ovr, 1)
		PHY_REG_LIST_EXECUTE(pi);

		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			biq2_gain = 0;
		} else {
			biq2_gain = 4;
		}

		do {

		  lna2_gain = 3;
		  while ((lna2_gain >= 0) && !set_gain) {

		    tia_gain = 4;
		    while ((tia_gain >= 0) && !set_gain) {

		      biq1_gain = 6;
		      while ((biq1_gain >= 0) && !set_gain) {
			phy_iq_est_t iq_est_h;
			phy_iq_est_t iq_est_l;
			uint32 i_thresh_l, i_thresh_h, q_thresh_l, q_thresh_h;

			wlc_lcnphy_set_rx_gain_by_distribution(pi,
				0, 0,
				(uint16)biq2_gain, (uint16)biq1_gain,
				(uint16)tia_gain, (uint16)lna2_gain,
				0, 0);
			wlc_lcnphy_rx_gain_override_enable(pi, TRUE);

			wlc_lcnphy_start_tx_tone(pi, (2000 * 1000),
				(k_lcnphy_rx_iq_est_tone_amplitude >> 1), 0);
			phy_utils_write_radioreg(pi, RADIO_2064_REG112, 0);
			if (!wlc_lcnphy_rx_iq_est(pi, k_lcnphy_rx_iq_est_n_samps, 32, 0,
				&iq_est_l, RXIQEST_TIMEOUT))
			{
				printf("Error getting low iq est\n");
				break;
			}

			wlc_lcnphy_start_tx_tone(pi, (2000 * 1000),
				k_lcnphy_rx_iq_est_tone_amplitude, 0);
			phy_utils_write_radioreg(pi, RADIO_2064_REG112, 0);
			if (!wlc_lcnphy_rx_iq_est(pi, k_lcnphy_rx_iq_est_n_samps, 32, 0,
				&iq_est_h, RXIQEST_TIMEOUT))
			{
				printf("Error getting high iq est\n");
				break;
			}

			i_thresh_l = (iq_est_l.i_pwr << 1) /* + iq_est_l.i_pwr */;
			i_thresh_h = (iq_est_l.i_pwr << 2) + iq_est_l.i_pwr;

			q_thresh_l = (iq_est_l.q_pwr << 1) /* + iq_est_l.q_pwr */;
			q_thresh_h = (iq_est_l.q_pwr << 2) + iq_est_l.q_pwr;
			/* Check that rx power drops by 6dB after pwr of dac input drops by 6dB */
			if ((iq_est_h.i_pwr > i_thresh_l) &&
			    (iq_est_h.i_pwr < i_thresh_h) &&
			    (iq_est_h.q_pwr > q_thresh_l) &&
			    (iq_est_h.q_pwr < q_thresh_h)) {
				set_gain = TRUE;

				PHY_INFORM(("wl%d: %s gains are lna2=%d tia=%d biq1=%d biq2=0",
				pi->sh->unit, __FUNCTION__, lna2_gain, tia_gain, biq1_gain));
			}
			biq1_gain--;
		      }
		      tia_gain--;
		    }
		    lna2_gain--;
		  }
		} while (0);

		PHY_REG_MOD(pi, LCNPHY, rxfe, bypass_iqcomp, 0);

		if (set_gain)
		  result = wlc_lcnphy_calc_rx_iq_comp(pi, k_lcnphy_rx_iq_est_n_samps);
		else
		  result = FALSE;

		/* clean up */
		wlc_lcnphy_stop_tx_tone(pi);
		/* restore papd state */
		/* restore Core1TxControl */
		PHY_REG_WRITE(pi, LCNPHY, Core1TxControl, Core1TxControl_old);
		/* save the older override settings */
		PHY_REG_WRITE(pi, LCNPHY, RFOverride0, RFOverride0_old);
		PHY_REG_WRITE(pi, LCNPHY, RFOverrideVal0, RFOverrideVal0_old);
		PHY_REG_WRITE(pi, LCNPHY, rfoverride2, rfoverride2_old);
		PHY_REG_WRITE(pi, LCNPHY, rfoverride2val, rfoverride2val_old);
		PHY_REG_WRITE(pi, LCNPHY, rfoverride3, rfoverride3_old);
		PHY_REG_WRITE(pi, LCNPHY, rfoverride3_val, rfoverride3val_old);
		PHY_REG_WRITE(pi, LCNPHY, rfoverride4, rfoverride4_old);
		PHY_REG_WRITE(pi, LCNPHY, rfoverride4val, rfoverride4val_old);
		PHY_REG_WRITE(pi, LCNPHY, AfeCtrlOvr, afectrlovr_old);
		PHY_REG_WRITE(pi, LCNPHY, AfeCtrlOvrVal, afectrlovrval_old);
		PHY_REG_WRITE(pi, LCNPHY, sslpnCalibClkEnCtrl, old_sslpnCalibClkEnCtrl);
		PHY_REG_WRITE(pi, LCNPHY, sslpnRxFeClkEnCtrl, old_sslpnRxFeClkEnCtrl);
		PHY_REG_WRITE(pi, LCNPHY, rxlnaandgainctrl1ovrval, old_rxlnaandgainctrl1ovrval);

		/* temporary fix as RFOverrideVal0 entering into rx_iq_cal */
		/* is seen to have ant_selp_ovr_val turned on */
		PHY_REG_MOD(pi, LCNPHY, RFOverride0, ant_selp_ovr, 0);
		/* Restore RF Registers */
		for (i = 0; i < ARRAYSIZE(rxiq_cal_rf_reg); i++) {
			phy_utils_write_radioreg(pi, rxiq_cal_rf_reg[i], values_to_save[i]);
		}
		/* Restore Tx gain */
		if (tx_gain_override_old) {
			wlc_lcnphy_set_tx_pwr_by_index(pi, tx_gain_index_old);
		} else
			wlc_lcnphy_disable_tx_gain_override(pi);
		wlc_lcnphy_set_tx_pwr_ctrl(pi, tx_pwr_ctrl);

		/* Clear various overrides */
		wlc_lcnphy_rx_gain_override_enable(pi, FALSE);
	}

cal_done:
	if (EPA(pi->u.pi_lcnphy))
		wlc_lcnphy_epa_pd(pi, 0);

	MFREE(pi->sh->osh, ptr, 131 * sizeof(int16));
	PHY_INFORM(("wl%d: %s: Rx IQ cal complete, coeffs: A0: %d, B0: %d\n",
		pi->sh->unit, __FUNCTION__,
		(int16)((phy_utils_read_phyreg(pi, LCNPHY_RxCompcoeffa0) &
		LCNPHY_RxCompcoeffa0_a0_MASK)
		>> LCNPHY_RxCompcoeffa0_a0_SHIFT),
		(int16)((phy_utils_read_phyreg(pi, LCNPHY_RxCompcoeffb0) &
		LCNPHY_RxCompcoeffb0_b0_MASK)
		>> LCNPHY_RxCompcoeffb0_b0_SHIFT)));
	return result;
}

static void
wlc_lcnphy_temp_adj(phy_info_t *pi)
{
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	if (NORADIO_ENAB(pi->pubpi))
		return;
}
static void
wlc_lcnphy_glacial_timer_based_cal(phy_info_t* pi)
{
	bool suspend;
	int8 indx;
	uint16 SAVE_pwrctrl = wlc_lcnphy_get_tx_pwr_ctrl(pi);
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
#if defined(PHYCAL_CACHING)
	ch_calcache_t *ctx = wlc_phy_get_chanctx(pi, pi->radio_chanspec);
	if (ctx == NULL) {
		PHY_ERROR(("%s: NULL ctx - bail\n", __FUNCTION__));
		return;
	}
#endif // endif
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	suspend = !(R_REG(pi->sh->osh, &pi->regs->maccontrol) & MCTL_EN_MAC);
	if (!suspend) {
		/* Set non-zero duration for CTS-to-self */
		wlapi_bmac_write_shm(pi->sh->physhim, M_CTS_DURATION, 10000);
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
	}
	wlc_lcnphy_deaf_mode(pi, TRUE);

#if defined(PHYCAL_CACHING)
	ctx->cal_info.last_cal_time = pi->sh->now;
#else
	pi->phy_lastcal = pi->sh->now;
#endif // endif
	pi->phy_forcecal = FALSE;
	indx = wlc_lcnphy_get_current_tx_pwr_idx(pi);
	/* do a full h/w iqlo cal during periodic cal */
	wlc_lcnphy_txpwrtbl_iqlo_cal(pi);
	wlc_2064_vco_cal(pi);

#if IDLE_TSSI_PER_CAL_EN
	if (wlc_lcnphy_tssi_based_pwr_ctrl_enabled(pi)) {
		/* Idle TSSI Estimate */
		wlc_lcnphy_idle_tssi_est((wlc_phy_t *) pi);

		/* Convert tssi to power LUT */
		wlc_lcnphy_set_estPwrLUT(pi, 0);

#if TWO_POWER_RANGE_TXPWR_CTRL
		if (pi_lcn->lcnphy_twopwr_txpwrctrl_en) {
			wlc_lcnphy_set_estPwrLUT(pi, 1);
		}
#endif /* TWO_POWER_RANGE_TXPWR_CTRL */
	}
#endif /* #if IDLE_TSSI_PER_CAL_EN */

	/* Check for temperature drift and recalculate the capping index */
	if ((CHIPID(pi->sh->chip) == BCM4313_CHIP_ID) && !EPA(pi_lcn)) {
		pi_lcn->lcnphy_capped_index = 0;
		wlc_lcnphy_load_txgainwithcappedindex(pi, 0);
		if (pi_lcn->lcnphy_CalcPapdCapEnable == 0) {
			pi_lcn->lcnphy_capped_index =
				wlc_lcnphy_tempcompensated_txpwrctrl(pi, TRUE);
			wlc_lcnphy_load_txgainwithcappedindex(pi, 1);
		}

	}
	/* cleanup */
	wlc_lcnphy_set_tx_pwr_by_index(pi, indx);
	wlc_lcnphy_set_tx_pwr_ctrl(pi, SAVE_pwrctrl);
	wlc_lcnphy_deaf_mode(pi, FALSE);
	if (!suspend)
		wlapi_enable_mac(pi->sh->physhim);
}

static void
wlc_phy_watchdog_lcnphy(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	int8 txpwrindex;
	uint16 tssi_shm_addr;

	if (pi->phy_forcecal || wlc_lcnphy_cal_reqd(pi)) {
		if (!(SCAN_RM_IN_PROGRESS(pi) || ASSOC_INPROG_PHY(pi) ||
			pi->disable_percal))
			wlc_lcnphy_calib_modes(pi, LCNPHY_PERICAL_TEMPBASED_TXPWRCTRL);

		if (!(SCAN_RM_IN_PROGRESS(pi) || PLT_INPROG_PHY(pi) ||
			ASSOC_INPROG_PHY(pi) || pi->carrier_suppr_disable ||
			(pi->measure_hold & PHY_HOLD_FOR_PKT_ENG) || pi->disable_percal))
			wlc_lcnphy_calib_modes(pi, PHY_PERICAL_WATCHDOG);
	}

	if (!(SCAN_RM_IN_PROGRESS(pi) ||
		PLT_INPROG_PHY(pi)))
		wlc_lcnphy_tx_pwr_update_npt(pi);

	if (!(SCAN_RM_IN_PROGRESS(pi))) {
		if ((pi_lcn->lcnphy_tssical_time) &&
			((pi->sh->now - pi_lcn->lcnphy_last_tssical)
			 >= pi_lcn->lcnphy_tssical_time)) {

			tssi_shm_addr =
				2 * wlapi_bmac_read_shm(pi->sh->physhim, M_TSSI_MSMT_BLK_PTR);
			if (tssi_shm_addr) {
				if (wlapi_bmac_read_shm(pi->sh->physhim,
					tssi_shm_addr + M_LCNPHY_TSSICAL_EN) == 0) {
					pi_lcn->tempsensereg1 =
					PHY_REG_READ(pi, LCNPHY, TxPwrCtrlStatusTemp, avgTemp);
					pi_lcn->tempsensereg2 =
					PHY_REG_READ(pi, LCNPHY, TxPwrCtrlStatusTemp1, avgTemp2);
				}
			}
			wlc_lcnphy_tssi_ucode_setup(pi, 0);
		} else if (!pi_lcn->lcnphy_tssical_time) {
			pi_lcn->tempsensereg1 =
				PHY_REG_READ(pi, LCNPHY, TxPwrCtrlStatusTemp, avgTemp);
			pi_lcn->tempsensereg2 =
				PHY_REG_READ(pi, LCNPHY, TxPwrCtrlStatusTemp1, avgTemp2);

		}
	}

	if (LCNPHY_TX_PWR_CTRL_HW != wlc_lcnphy_get_tx_pwr_ctrl((pi))) {
		txpwrindex = wlc_lcnphy_get_current_tx_pwr_idx(pi);
		if (txpwrindex != 0)
			pi_lcn->idx0cnt = 0;
		if (txpwrindex != 127)
			pi_lcn->idx127cnt = 0;
	}

	if (!(SCAN_RM_IN_PROGRESS(pi) || PLT_INPROG_PHY(pi) || ASSOC_INPROG_PHY(pi)))
		if (CHIPID(pi->sh->chip) != BCM4313_CHIP_ID)
			wlc_lcnphy_noise_measure(pi);

#ifdef MULTICHAN_4313
	/* Reset papd tx power index and tx power control for all the channels */
	if (CHIPID(pi->sh->chip) == BCM4313_CHIP_ID) {
		if ((pi->sh->now - pi->cal_info->last_cal_time) >= pi->sh->glacial_timer)
		{
			wlc_lcnphy_tx_pwr_idx_reset(pi);
			pi->cal_info->last_cal_time = pi->sh->now;
		}
	}
#endif // endif
}

static void
wlc_lcnphy_aci_upd(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	int cnt, delta, t;
	cnt = (int)phy_utils_read_phyreg(pi, LCNPHY_crsDetFailCtr);
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

	t = (int)((pi)->sh->now) - pi_lcn->lcnphy_aci.ts;
	if (!(pi->aci_state & ACI_ACTIVE) && (t >= pi_lcn->lcnphy_aci.on_timeout))
		wlc_lcnphy_aci(pi, TRUE);
	else if ((pi->aci_state & ACI_ACTIVE) && (t >= pi_lcn->lcnphy_aci.off_timeout))
		wlc_lcnphy_aci(pi, FALSE);

}

void
wlc_lcnphy_aci_init(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	PHY_REG_OR(pi, LCNPHY, failCtrCtrl, 0x02);
	pi->aci_state = 0;
	pi_lcn->lcnphy_aci.gain_backoff = 0;
	pi_lcn->lcnphy_aci.on_thresh = 800;
	pi_lcn->lcnphy_aci.on_timeout = 1;
	pi_lcn->lcnphy_aci.off_thresh = 500;
	pi_lcn->lcnphy_aci.off_timeout = 20;
	wlc_lcnphy_aci(pi, FALSE);
}

void
wlc_lcnphy_aci(phy_info_t *pi, bool on)
{
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;

	if (on && !(pi->aci_state & ACI_ACTIVE)) {
		pi->aci_state |= ACI_ACTIVE;
		/* set the registers for ACI */
		pi_lcn->lcnphy_aci.gain_backoff = 1;
	} else if (!on && (pi->aci_state & ACI_ACTIVE)) {

		pi->aci_state &= ~ACI_ACTIVE;
		pi_lcn->lcnphy_aci.gain_backoff = 0;
			PHY_REG_MOD(pi, LCNPHY, HiGainDB, HiGainDB,
				pi_lcn->lcnphy_aci.Listen_GaindB_AfrNoiseCal);
			PHY_REG_MOD(pi, LCNPHY, crsedthresh, edonthreshold,
				pi_lcn->lcnphy_aci.EdOn_Thresh_BASE);

		/* Reset radio ctrl and crs gain */
		PHY_REG_LIST_START
			PHY_REG_OR_ENTRY(LCNPHY, resetCtrl, 0x44)
			PHY_REG_WRITE_ENTRY(LCNPHY, resetCtrl, 0x80)
		PHY_REG_LIST_EXECUTE(pi);
	}

	pi_lcn->lcnphy_aci.ts = (pi)->sh->now;
	PHY_REG_OR(pi, LCNPHY, failCtrCtrl, 0x01);
	pi_lcn->lcnphy_aci.glitch_cnt = 0;
	pi_lcn->lcnphy_aci.ts = (int)((pi)->sh->now);
}

void wlc_lcnphy_force_adj_gain(phy_info_t *pi, bool on, int mode)
{
	/* ACI forced mitigation */
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	if (on) {
		uint16 higain, pwroff;
		int16 nfs_adj;
		int8 ed_thresh;

		if (mode == WLAN_MANUAL) {
			higain = pi_lcn->noise.high_gain - 6;
			pwroff = pi_lcn->noise.input_pwr_offset - 3;
			nfs_adj = pi_lcn->noise.nf_substract_val - 66;
			ed_thresh = pi_lcn->lcnphy_aci.EdOn_Thresh_BASE + 19;
		} else {
			higain = pi_lcn->noise.high_gain - 3;
			pwroff = pi_lcn->noise.input_pwr_offset - 2;
			nfs_adj = pi_lcn->noise.nf_substract_val - 33;
			ed_thresh = pi_lcn->lcnphy_aci.EdOn_Thresh_BASE + 3;
		}
			PHY_REG_MOD(pi, LCNPHY, HiGainDB, HiGainDB, higain);
			PHY_REG_MOD(pi, LCNPHY, InputPowerDB, inputpwroffsetdb, pwroff);
			PHY_REG_WRITE(pi, LCNPHY, nfSubtractVal, nfs_adj);
			PHY_REG_MOD(pi, LCNPHY, crsedthresh, edonthreshold, ed_thresh);

	} else if (!on) {
			PHY_REG_MOD(pi, LCNPHY, HiGainDB, HiGainDB, pi_lcn->noise.high_gain);
			PHY_REG_MOD(pi, LCNPHY, InputPowerDB, inputpwroffsetdb,
				pi_lcn->noise.input_pwr_offset);
			PHY_REG_WRITE(pi, LCNPHY, nfSubtractVal, pi_lcn->noise.nf_substract_val);
			PHY_REG_MOD(pi, LCNPHY, crsedthresh, edonthreshold,
				pi_lcn->lcnphy_aci.EdOn_Thresh_BASE);
	}
}

/* periodic cal does tx iqlo cal, rx iq cal, (tempcompensated txpwrctrl for P200 4313A0 board) */
static void
wlc_lcnphy_periodic_cal(phy_info_t *pi)
{
	bool suspend, full_cal;
	uint16 SAVE_pwrctrl = wlc_lcnphy_get_tx_pwr_ctrl(pi);
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;

#if defined(PHYCAL_CACHING)
	lcnphy_calcache_t *cache;
	ch_calcache_t *ctx = wlc_phy_get_chanctx(pi, pi->radio_chanspec);
	if (ctx == NULL) {
		PHY_ERROR(("%s: NULL ctx - bail\n", __FUNCTION__));
		return;
	}
	cache = &ctx->u.lcnphy_cache;
#endif // endif

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	if (NORADIO_ENAB(pi->pubpi))
		return;

#if defined(PHYCAL_CACHING)
	ctx->cal_info.last_cal_time = pi->sh->now;
	ctx->cal_info.last_cal_temp = pi_lcn->lcnphy_rawtempsense;
	full_cal = !(ctx->valid);
#else
	pi->phy_lastcal = pi->sh->now;
	full_cal = (pi_lcn->lcnphy_full_cal_channel != CHSPEC_CHANNEL(pi->radio_chanspec));
	pi_lcn->lcnphy_full_cal_channel = CHSPEC_CHANNEL(pi->radio_chanspec);
#endif // endif
	BCM_REFERENCE(full_cal);

	pi->phy_forcecal = FALSE;
	suspend = !(R_REG(pi->sh->osh, &pi->regs->maccontrol) & MCTL_EN_MAC);
	if (!suspend) {
		/* Set non-zero duration for CTS-to-self */
		wlapi_bmac_write_shm(pi->sh->physhim, M_CTS_DURATION, 10000);
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
	}
	wlc_lcnphy_deaf_mode(pi, TRUE);

	PHY_REG_LIST_START
		/* Disable filters, turn off PAPD */
		PHY_REG_MOD_ENTRY(LCNPHY, papd_control, papdCompEn, 0)
		PHY_REG_MOD_ENTRY(LCNPHY, sslpnCalibClkEnCtrl, papdTxClkEn, 0)
		PHY_REG_MOD_ENTRY(LCNPHY, txfefilterconfig, cck_papden, 0)
		PHY_REG_MOD_ENTRY(LCNPHY, txfefilterconfig, ofdm_papden, 0)
		PHY_REG_MOD_ENTRY(LCNPHY, txfefilterconfig, ht_papden, 0)
	PHY_REG_LIST_EXECUTE(pi);
	wlc_lcnphy_reset_radio_loft(pi);
	wlc_lcnphy_set_tx_locc(pi, 0);
	wlc_lcnphy_set_tx_iqcc(pi, 0, 0);

	wlc_btcx_override_enable(pi);
#if IDLE_TSSI_PER_CAL_EN
	if (wlc_lcnphy_tssi_based_pwr_ctrl_enabled(pi)) {
		/* Idle TSSI Estimate */
		wlc_lcnphy_idle_tssi_est((wlc_phy_t *) pi);

		/* Convert tssi to power LUT */
		wlc_lcnphy_set_estPwrLUT(pi, 0);

#if TWO_POWER_RANGE_TXPWR_CTRL
		if (pi_lcn->lcnphy_twopwr_txpwrctrl_en) {
			wlc_lcnphy_set_estPwrLUT(pi, 1);
		}
#endif /* TWO_POWER_RANGE_TXPWR_CTRL */
	}
#endif /* #if IDLE_TSSI_PER_CAL_EN */
	wlc_lcnphy_txpwrtbl_iqlo_cal(pi);

	/* XXX FIXME: unused?
	 * const lcnphy_rx_iqcomp_t *rx_iqcomp = lcnphy_rx_iqcomp_table_rev0;
	 * int rx_iqcomp_sz = ARRAYSIZE(lcnphy_rx_iqcomp_table_rev0);
	 */
	/* For 4313B0, Tx index at which rxiqcal is done, has to be 40 */
	/* to get good SNR in the loopback path */
	if ((CHIPID(pi->sh->chip) == BCM4313_CHIP_ID) && LCNREV_IS(pi->pubpi.phy_rev, 1))
		wlc_lcnphy_rx_iq_cal(pi, NULL, 0, 1, 40);
	else
		wlc_lcnphy_rx_iq_cal(pi, NULL, 0, 1, 127);

	pi_lcn->papd_lut0_cal_idx = -1;
	pi_lcn->papd_lut1_cal_idx = -1;

	if (PAPD_4336_MODE(pi_lcn) && !EPA(pi_lcn)) {
		wlc_lcnphy_papd_cal_txpwr(pi);
	}

	if (CHIPID(pi->sh->chip) == BCM4313_CHIP_ID && !EPA(pi_lcn)) {
	    wlc_lcnphy_calib_modes(pi, PHY_PAPDCAL);
	    pi_lcn->lcnphy_papd_cal_done_at_init = TRUE;
	}
	wlc_lcnphy_set_tx_pwr_ctrl(pi, SAVE_pwrctrl);
	wlc_lcnphy_deaf_mode(pi, FALSE);

	if (!suspend)
		wlapi_enable_mac(pi->sh->physhim);

#if defined(PHYCAL_CACHING)
	/* Cache the power index used for this channel */
	cache->lcnphy_gain_index_at_last_cal = wlc_lcnphy_get_current_tx_pwr_idx(pi);
	/* Already cached the Tx, Rx IQ and PAPD Cal results */
	ctx->valid = TRUE;

#if defined(BCMDBG)
	/* Print the params to be cached */
	wlc_phy_cal_cache_dbg_lcnphy(ctx);
#endif // endif

#endif /* PHYCAL_CACHING */
}

/* Populates the rfpower lut for papd w/ 1 or 2 luts */
static void
wlc_lcnphy_rfpower_lut_2papdlut(phy_info_t *pi)
{
	phytbl_info_t tab;
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	int32 lut0_refpwr, lut1_refpwr, lut1_refpwr_corr, rfpower, rfpower_lut1_thres,
		offset;
	uint8 index, index1, gain_tbl_idx = 0;
	uint32 *rfpower_arr = NULL, len = 0;

	lut0_refpwr = wlc_lcnphy_calc_papd_rf_pwr_offset(pi,
		pi_lcn->papd_lut0_cal_idx, -1);

	if (pi_lcn->papd_num_lut_used == 2)
		lut1_refpwr = wlc_lcnphy_calc_papd_rf_pwr_offset(pi,
			pi_lcn->papd_lut1_cal_idx, -1);
	else
		lut1_refpwr = 0;
	lut1_refpwr_corr = lut0_refpwr - lut1_refpwr;

	tab.tbl_id = LCNPHY_TBL_ID_TXPWRCTL;
	tab.tbl_width = 32;     /* 32 bit wide  */
	tab.tbl_len = 1;
	tab.tbl_ptr = &rfpower; /* ptr to buf */
	rfpower_lut1_thres = wlc_lcnphy_calc_papd_rf_pwr_offset(pi,
		pi_lcn->papd_lut1_thres, -1);

	pi_lcn->papd_analog_gain_ref_offset = 0;
	if (rfpower_lut1_thres + lut1_refpwr_corr < 0)
		pi_lcn->papd_analog_gain_ref_offset = rfpower_lut1_thres + lut1_refpwr_corr;
	offset = 0;
	/* Write the rfpower lut */
	if (CHIPID(pi->sh->chip) == BCM4330_CHIP_ID) {
		if (pi_lcn->virtual_p25_tx_gain_step) {
			len = 64;
		} else {
			len = 128;
		}
		rfpower_arr = (uint32 *)MALLOC(pi->sh->osh, len* sizeof(uint32));
	}
	if ((CHIPID(pi->sh->chip) != BCM4330_CHIP_ID) || (rfpower_arr == NULL)) {
		for (index = 0; index < 128; index++) {
			if ((pi_lcn->virtual_p25_tx_gain_step) && (index % 2 == 1))
				continue;
			gain_tbl_idx = index/(1+pi_lcn->virtual_p25_tx_gain_step);
			tab.tbl_offset = LCNPHY_TX_PWR_CTRL_PWR_OFFSET + gain_tbl_idx;
			offset = pi_lcn->papd_analog_gain_ref_offset;
			if (index > pi_lcn->papd_lut1_thres)
				offset -= lut1_refpwr_corr;

			rfpower = wlc_lcnphy_calc_papd_rf_pwr_offset(pi, index, -1)
				- offset;
			wlc_lcnphy_write_table(pi, &tab);
		}
	} else {
		tab.tbl_len = len;
		tab.tbl_offset = LCNPHY_TX_PWR_CTRL_PWR_OFFSET;
		tab.tbl_ptr = rfpower_arr;
		wlc_lcnphy_calc_papd_rf_pwr_offset_arr(pi, -1, rfpower_arr);
		for (index = 0; index < 128; index ++) {
			if ((pi_lcn->virtual_p25_tx_gain_step) && (index % 2 == 1))
			continue;
			gain_tbl_idx = index/(1+pi_lcn->virtual_p25_tx_gain_step);
			offset = pi_lcn->papd_analog_gain_ref_offset;
			if (index > pi_lcn->papd_lut1_thres)
				offset -= lut1_refpwr_corr;
			*(rfpower_arr+gain_tbl_idx) -= offset;
		}
		rfpower = *(rfpower_arr + gain_tbl_idx);
		wlc_lcnphy_write_table(pi, &tab);
	}

	if (pi_lcn->virtual_p25_tx_gain_step)  {
		tab.tbl_len = 1;
		tab.tbl_ptr = &rfpower;
		for (index1 = 64; index1 < 128; index1++) {
			tab.tbl_offset = LCNPHY_TX_PWR_CTRL_PWR_OFFSET
				+ index1;
			wlc_lcnphy_write_table(pi, &tab);
		}
	}
	if (rfpower_arr != NULL)
		MFREE(pi->sh->osh, rfpower_arr, len * sizeof(uint32));
}

/* Search for gain index with PAPD AM/AM magnitude less than specified threshold */
static void
wlc_lcnphy_papd_idx_sel(phy_info_t *pi, uint32 amam_cmp, lcnphy_txcalgains_t *txgains)
{
	phytbl_info_t tab;
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	uint32 val;
	int8 step, step_size;
	uint8 num_symbols = 6;
	uint16 bbmult_step = 32767;
	int32 lut_i;
	int32 lut_j;
	uint32 amam;
	int16 tx_idx;
	int8 pacalalim;
	uint8 gain_incr;
	step = 5;
	tab.tbl_id = LCNPHY_TBL_ID_PAPDCOMPDELTATBL;
	tab.tbl_ptr = &val; /* ptr to buf */
	tab.tbl_width = 32;
	tab.tbl_len = 1;
	tab.tbl_offset = 63;

	if (pi_lcn->pacalalim == -1)
		if (CHIPID(pi->sh->chip) == BCM43362_CHIP_ID)
			pacalalim = 1;
		else
			pacalalim = 0;
	else
		pacalalim = pi_lcn->pacalalim;

	amam_cmp *= amam_cmp;
	for (step_size = 5; step_size > 0; step_size /= 2) {
		wlc_lcnphy_papd_cal(pi, PHY_PAPD_CAL_CW, txgains, 0, 0, 0, 0, num_symbols,
			1, bbmult_step);
		wlc_lcnphy_read_table(pi, &tab);
		lut_i = (val & 0x00fff000) << 8;
		lut_j = (val & 0x00000fff) << 20;
		lut_i = lut_i >> 20;
		lut_j = lut_j >> 20;
		amam = ((lut_i*lut_i) + (lut_j*lut_j));
		tx_idx = txgains->index;
		gain_incr = 0;
		step = step_size;
		if (amam < amam_cmp) {
			if (pacalalim)
				break;
			step = -step;
			gain_incr = 1;
		}
		while (1) {
			if ((gain_incr == 1) && (amam > amam_cmp)) {
				txgains->index = (uint8)(tx_idx - step);
				break;
			} else if ((gain_incr == 0) && (amam < amam_cmp)) {
				break;
			}
			tx_idx += step;
			if (tx_idx > 127) {
				tx_idx = 127;
				break;
			} else if (tx_idx < 0) {
				tx_idx = 0;
				break;
			}
			txgains->index = (uint8)tx_idx;
			wlc_lcnphy_papd_cal(pi, PHY_PAPD_CAL_CW, txgains, 0, 0, 0, 0, num_symbols,
				1, bbmult_step);
			wlc_lcnphy_read_table(pi, &tab);
			lut_i = (val & 0x00fff000) << 8;
			lut_j = (val & 0x00000fff) << 20;
			lut_i = lut_i >> 20;
			lut_j = lut_j >> 20;
			amam = ((lut_i*lut_i) + (lut_j*lut_j));
		}
	}
}

/* PAPD tx gain control + PAPD cal for 4330/4336/43362 */
static void
wlc_lcnphy_papd_cal_txpwr(phy_info_t *pi)
{
	phytbl_info_t tab;
	uint8 indx, j;
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	int8 pacalidx = -1, pacalidx1 = -1, temp;
	int16 pacalpwr = -1, pacalpwr1 = -1;
	int16 pacalamamth = -1, pacalamamth1 = -1;
	uint16 papd_analog_gain_ref_new;
	lcnphy_txcalgains_t txgains;

	indx = wlc_lcnphy_get_current_tx_pwr_idx(pi);

	/* PAPD cal */
	txgains.index = 0;

	/* PAPD tx index per chip/band defaults */
	if ((CHIPID(pi->sh->chip) == BCM4336_CHIP_ID) &&
		(pi->sh->chippkg == BCM4336_WLBGA_PKG_ID))
		txgains.index = 40;
	else if (CHIPID(pi->sh->chip) == BCM4330_CHIP_ID) {
		if (pi->sh->chiprev == 0)
			txgains.index = 60;
		else if (pi->sh->chiprev == 1)
			txgains.index = 40;
		else
			txgains.index = 55;
	}
	else if (CHIPID(pi->sh->chip) == BCM43362_CHIP_ID)
		txgains.index = 28;
	else
		txgains.index = 14;
	txgains.useindex = 1;

	if (CHIPID(pi->sh->chip) == BCM43362_CHIP_ID)
		pacalamamth = 290;
	else if (!EPA(pi->u.pi_lcnphy) && CHSPEC_IS5G(pi->radio_chanspec)) {
		if (CHSPEC_CHANNEL(pi->radio_chanspec) <= 64)
			pacalamamth = 192;
		else if (CHSPEC_CHANNEL(pi->radio_chanspec) <= 140)
			pacalamamth = 192;
		else
			pacalamamth = 217;
	}

	if (CHSPEC_IS5G(pi->radio_chanspec)) {
		if (CHSPEC_CHANNEL(pi->radio_chanspec) <= 64)
			txgains.index = 27;
		else if (CHSPEC_CHANNEL(pi->radio_chanspec) <= 140)
			txgains.index = 45;
		else
			txgains.index = 45;
	}

	/* PAPD tx index, apply nvram overrides */
	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		/* g band lut0 */
		if (pi_lcn->pacalpwr_2g != -1) {
			if (pi_lcn->pacalpwr_2g != 0)
				pacalpwr = pi_lcn->pacalpwr_2g;
			else
				pacalpwr = -1;
		}
		if (pi_lcn->pacalamamth_2g != -1) {
			if (pi_lcn->pacalamamth_2g != 0)
				pacalamamth = pi_lcn->pacalamamth_2g;
			else
				pacalamamth = -1;
		}
		if (pi_lcn->pacalidx_2g != -1)
			pacalidx = pi_lcn->pacalidx_2g;
		/* g band lut1 */
		if (pi_lcn->pacalpwr_2g1 != -1) {
			if (pi_lcn->pacalpwr_2g1 != 0)
				pacalpwr1 = pi_lcn->pacalpwr_2g1;
			else
				pacalpwr1 = -1;
		}
		if (pi_lcn->pacalamamth_2g1 != -1) {
			if (pi_lcn->pacalamamth_2g1 != 0)
				pacalamamth1 = pi_lcn->pacalamamth_2g1;
			else
				pacalamamth1 = -1;
		}
		if (pi_lcn->pacalidx_2g1 != -1)
			pacalidx1 = pi_lcn->pacalidx_2g1;
	} else if (CHSPEC_CHANNEL(pi->radio_chanspec) <= 64) {
		/* a low-band lut0 */
		if (pi_lcn->pacalpwr_5glo != -1) {
			if (pi_lcn->pacalpwr_5glo != 0)
				pacalpwr = pi_lcn->pacalpwr_5glo;
			else
				pacalpwr = -1;
		}
		if (pi_lcn->pacalamamth_5glo != -1) {
			if (pi_lcn->pacalamamth_5glo != 0)
				pacalamamth = pi_lcn->pacalamamth_5glo;
			else
				pacalamamth = -1;
		}
		if (pi_lcn->pacalidx_5glo != -1)
			pacalidx = pi_lcn->pacalidx_5glo;
		/* a low-band lut1 */
		if (pi_lcn->pacalpwr_5glo1 != -1) {
			if (pi_lcn->pacalpwr_5glo1 != 0)
				pacalpwr1 = pi_lcn->pacalpwr_5glo1;
			else
				pacalpwr1 = -1;
		}
		if (pi_lcn->pacalamamth_5glo1 != -1) {
			if (pi_lcn->pacalamamth_5glo1 != 0)
				pacalamamth1 = pi_lcn->pacalamamth_5glo1;
			else
				pacalamamth1 = -1;
		}
		if (pi_lcn->pacalidx_5glo1 != -1)
			pacalidx1 = pi_lcn->pacalidx_5glo1;
	} else if (CHSPEC_CHANNEL(pi->radio_chanspec) <= 140) {
		/* a mid-band lut0 */
		if (pi_lcn->pacalpwr_5g != -1) {
			if (pi_lcn->pacalpwr_5g != 0)
				pacalpwr = pi_lcn->pacalpwr_5g;
			else
				pacalpwr = -1;
		}
		if (pi_lcn->pacalamamth_5g != -1) {
			if (pi_lcn->pacalamamth_5g != 0)
				pacalamamth = pi_lcn->pacalamamth_5g;
			else
				pacalamamth = -1;
		}
		if (pi_lcn->pacalidx_5g != -1)
			pacalidx = pi_lcn->pacalidx_5g;
		/* a mid-band lut1 */
		if (pi_lcn->pacalpwr_5g1 != -1) {
			if (pi_lcn->pacalpwr_5g1 != 0)
				pacalpwr1 = pi_lcn->pacalpwr_5g1;
			else
				pacalpwr1 = -1;
		}
		if (pi_lcn->pacalamamth_5g1 != -1) {
			if (pi_lcn->pacalamamth_5g1 != 0)
				pacalamamth1 = pi_lcn->pacalamamth_5g1;
			else
				pacalamamth1 = -1;
		}
		if (pi_lcn->pacalidx_5g1 != -1)
			pacalidx1 = pi_lcn->pacalidx_5g1;
	} else {
		/* a high-band lut0 */
		if (pi_lcn->pacalpwr_5ghi != -1) {
			if (pi_lcn->pacalpwr_5ghi != 0)
				pacalpwr = pi_lcn->pacalpwr_5ghi;
			else
				pacalpwr = -1;
		}
		if (pi_lcn->pacalamamth_5ghi != -1) {
			if (pi_lcn->pacalamamth_5ghi != 0)
				pacalamamth = pi_lcn->pacalamamth_5ghi;
			else
				pacalamamth = -1;
		}
		if (pi_lcn->pacalidx_5ghi != -1)
			pacalidx = pi_lcn->pacalidx_5ghi;
		/* a high-band lut1 */
		if (pi_lcn->pacalpwr_5ghi1 != -1) {
			if (pi_lcn->pacalpwr_5ghi1 != 0)
				pacalpwr1 = pi_lcn->pacalpwr_5ghi1;
			else
				pacalpwr1 = -1;
		}
		if (pi_lcn->pacalamamth_5ghi1 != -1) {
			if (pi_lcn->pacalamamth_5ghi1 != 0)
				pacalamamth1 = pi_lcn->pacalamamth_5ghi1;
			else
				pacalamamth1 = -1;
		}
		if (pi_lcn->pacalidx_5ghi1 != -1)
			pacalidx1 = pi_lcn->pacalidx_5ghi1;
	}

	/* Run PAPD tx gain control for lut0 */
	if (pacalpwr != -1) {
		if (pacalpwr == 0)
			pi_lcn->papd_lut0_cal_idx =
			(uint8)wlc_lcnphy_cw_tx_pwr_ctrl(pi,
				(int32)wlc_phy_txpower_get_target_min((wlc_phy_t*)pi));
		else
			pi_lcn->papd_lut0_cal_idx =
				(uint8) wlc_lcnphy_cw_tx_pwr_ctrl(pi, (int32)pacalpwr);
	} else if (pacalamamth != -1) {
		wlc_lcnphy_papd_idx_sel(pi, pacalamamth, &txgains);
		pi_lcn->papd_lut0_cal_idx = txgains.index;
	} else if (pacalidx != -1)
		pi_lcn->papd_lut0_cal_idx = (uint8)pacalidx;
	else if (txgains.index != 0)
		pi_lcn->papd_lut0_cal_idx = txgains.index;

	/* Run PAPD tx gain control for lut1 */
	if (pacalpwr1 != -1) {
		if (pacalpwr1 == 0)
			pi_lcn->papd_lut1_cal_idx =
			(uint8)wlc_lcnphy_cw_tx_pwr_ctrl(pi,
				(int32)wlc_phy_txpower_get_target_min((wlc_phy_t*)pi));
		else
			pi_lcn->papd_lut1_cal_idx =
				(uint8) wlc_lcnphy_cw_tx_pwr_ctrl(pi, (int32)pacalpwr1);
	} else if (pacalamamth1 != -1) {
		wlc_lcnphy_papd_idx_sel(pi, pacalamamth1, &txgains);
		pi_lcn->papd_lut1_cal_idx = txgains.index;
	} else if (pacalidx1 != -1)
		pi_lcn->papd_lut1_cal_idx = (uint8)pacalidx1;

	/* To force the papd cal index via iovar */
	if (pi_lcn->pacalidx != -1)
		pi_lcn->papd_lut1_cal_idx = -1;

	/* run papd cal for second LUT */
	if (pi_lcn->papd_lut1_cal_idx != -1) {
		if (pi_lcn->papd_lut1_cal_idx <= pi_lcn->papd_lut0_cal_idx) {
			temp = pi_lcn->papd_lut0_cal_idx;
			pi_lcn->papd_lut0_cal_idx = pi_lcn->papd_lut1_cal_idx;
			pi_lcn->papd_lut1_cal_idx = temp;
		}
		txgains.index = pi_lcn->papd_lut1_cal_idx;
		wlc_lcnphy_papd_cal(pi, PHY_PAPD_CAL_CW, &txgains, 0, 0, 0, 0, 219, 1, 0);

		/* copy lut0 to lut1 */
		tab.tbl_id = LCNPHY_TBL_ID_PAPDCOMPDELTATBL;
		tab.tbl_width = 32;
		tab.tbl_len = 1;
		for (j = 0; j < 64; j++) {
			int32 val;
			tab.tbl_ptr = &val;
			tab.tbl_offset = j;
			wlc_lcnphy_read_table(pi, &tab);
			tab.tbl_offset = j+64;
			wlc_lcnphy_write_table(pi, &tab);
		}

		/* enable second lut */
		pi_lcn->papd_lut1_thres = (pi_lcn->papd_lut0_cal_idx
			+ pi_lcn->papd_lut1_cal_idx)/2;
		if (CHSPEC_IS2G(pi->radio_chanspec) && (pi_lcn->pacalidx2g1th != 0))
			pi_lcn->papd_lut1_thres = pi_lcn->pacalidx2g1th;
		else if ((CHSPEC_CHANNEL(pi->radio_chanspec) <= 64) &&
			(pi_lcn->pacalidx5glo1th != 0))
			pi_lcn->papd_lut1_thres = pi_lcn->pacalidx5glo1th;
		else if ((CHSPEC_CHANNEL(pi->radio_chanspec) <= 140) &&
			(pi_lcn->pacalidx5g1th != 0))
			pi_lcn->papd_lut1_thres = pi_lcn->pacalidx5g1th;
		else if (pi_lcn->pacalidx5ghi1th != 0)
			pi_lcn->papd_lut1_thres = pi_lcn->pacalidx5ghi1th;
		PHY_REG_MOD(pi, LCNPHY, RadioRegWriteAdd4, GlobalDisable, 0);
		PHY_REG_MOD(pi, LCNPHY, PapdDupLutCtrl, txgainidxThreshold,
			pi_lcn->papd_lut1_thres/(1+pi_lcn->virtual_p25_tx_gain_step));
		pi_lcn->papd_num_lut_used = 2;

	} else {
		pi_lcn->papd_num_lut_used = 1;
		PHY_REG_LIST_START
			PHY_REG_MOD_ENTRY(LCNPHY, PapdDupLutCtrl, txgainidxThreshold, 127)
			PHY_REG_MOD_ENTRY(LCNPHY, PapdDupLutCtrl, PapdLutSel0, 0)
			PHY_REG_MOD_ENTRY(LCNPHY, PapdDupLutCtrl, PapdLutSel1, 0)
			PHY_REG_MOD_ENTRY(LCNPHY, PapdDupLutCtrl, PapdLutSel2, 0)
			PHY_REG_MOD_ENTRY(LCNPHY, PapdDupLutCtrl, PapdLutSel3, 0)
		PHY_REG_LIST_EXECUTE(pi);
	}

	pi_lcn->papd_analog_gain_ref_offset = 0;
	pi_lcn->papd_lut1_thres = (1+pi_lcn->virtual_p25_tx_gain_step)*PHY_REG_READ(pi,
		LCNPHY, PapdDupLutCtrl, txgainidxThreshold);

	/* current band uses dual lut for at least some channels, rewrite rfpwrlut */
	if (pi_lcn->virtual_p25_tx_gain_step)
		wlc_lcnphy_rfpower_lut_2papdlut(pi);

	/* papd cal the first table */
	if (pi_lcn->pacalidx != -1)
		txgains.index = pi_lcn->pacalidx;
	else
		txgains.index = pi_lcn->papd_lut0_cal_idx;

	wlc_lcnphy_papd_cal(pi, PHY_PAPD_CAL_CW, &txgains, 0, 0, 0, 0, 219, 1, 0);
	wlc_lcnphy_set_tx_pwr_by_index(pi, indx);
	papd_analog_gain_ref_new = phy_utils_read_phyreg(pi, LCNPHY_papd_tx_analog_gain_ref)
		- pi_lcn->papd_analog_gain_ref_offset*8;
	PHY_REG_WRITE(pi, LCNPHY, papd_tx_analog_gain_ref,
		(uint16)papd_analog_gain_ref_new);
	wlc_lcnphy_save_papd_calibration_results(pi);
}

void
wlc_lcnphy_calib_modes(phy_info_t *pi, uint mode)
{
	uint16 temp_new;
	int temp1, temp2, temp_diff;
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
#if defined(PHYCAL_CACHING)
	ch_calcache_t *ctx = wlc_phy_get_chanctx(pi, pi->radio_chanspec);
	if (ctx == NULL) {
		/* Fresh calibration or restoration required */
		if (LCNPHY_MAX_CAL_CACHE <= pi->phy_calcache_num) {
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
	if (!ctx) {
		PHY_ERROR(("%s: unable to create cal ctx\n", __FUNCTION__));
		return;
	}
#endif /* PHYCAL_CACHING */

	switch (mode) {
	case PHY_PERICAL_CHAN:
		/* right now, no channel based calibration */
		break;
	case PHY_FULLCAL:
		wlc_lcnphy_periodic_cal(pi);
		break;
	case PHY_PERICAL_WATCHDOG: {
		if ((CHIPID(pi->sh->chip) == BCM4313_CHIP_ID) &&
			wlc_lcnphy_tempsense_based_pwr_ctrl_enabled(pi)) {
		temp_new = wlc_lcnphy_tempsense(pi, 0);
			temp1 = LCNPHY_TEMPSENSE(temp_new);
#if defined(PHYCAL_CACHING)
			temp2 = LCNPHY_TEMPSENSE(ctx->cal_info.last_cal_temp);
#else
			temp2 = LCNPHY_TEMPSENSE(pi_lcn->lcnphy_cal_temper);
#endif // endif
			temp_diff = temp1 - temp2;
			if ((pi_lcn->lcnphy_cal_counter > 90) ||
				(temp_diff > 60) || (temp_diff < -60)) {
				wlc_lcnphy_glacial_timer_based_cal(pi);
#if defined(PHYCAL_CACHING)
				ctx->cal_info.last_cal_temp = temp_new;
#else
				pi_lcn->lcnphy_cal_temper = temp_new;
#endif // endif
				pi_lcn->lcnphy_cal_counter = 0;
			} else {
				pi_lcn->lcnphy_cal_counter++;
#if defined(PHYCAL_CACHING)
				ctx->cal_info.last_cal_time = pi->sh->now;
#else
				pi->phy_lastcal = pi->sh->now;
#endif // endif
			}
		}
		else if ((CHIPID(pi->sh->chip) == BCM4313_CHIP_ID) &&
			wlc_lcnphy_tssi_based_pwr_ctrl_enabled(pi)) {
			wlc_lcnphy_glacial_timer_based_cal(pi);
		} else if (CHIPID(pi->sh->chip) == BCM4336_CHIP_ID ||
			CHIPID(pi->sh->chip) == BCM43362_CHIP_ID ||
			CHIPID(pi->sh->chip) == BCM4330_CHIP_ID)
			wlc_lcnphy_periodic_cal(pi);
		else
			PHY_INFORM(("No periodic cal for tssi based tx pwr ctrl supported chip"));
		break;
	}
	case LCNPHY_PERICAL_TEMPBASED_TXPWRCTRL:
		if (wlc_lcnphy_tempsense_based_pwr_ctrl_enabled(pi))
			wlc_lcnphy_tx_power_adjustment((wlc_phy_t *)pi);
		break;
	case PHY_PAPDCAL:
		wlc_lcnphy_papd_recal(pi);
		break;
	default:
		ASSERT(0);
		break;
	}
}

static bool
wlc_lcnphy_cal_reqd(phy_info_t *pi)
{
#if defined(PHYCAL_CACHING)
	uint time_since_last_cal;
	ch_calcache_t *ctx = wlc_phy_get_chanctx(pi, pi->radio_chanspec);

	if (ctx == NULL) {
		return TRUE;
	}
	time_since_last_cal = (pi->sh->now >= ctx->cal_info.last_cal_time)?
		(pi->sh->now - ctx->cal_info.last_cal_time):
		(((uint)~0) - ctx->cal_info.last_cal_time + pi->sh->now);

	if (ctx->valid)
#else
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	uint time_since_last_cal = (pi->sh->now >= pi->phy_lastcal)?
		(pi->sh->now - pi->phy_lastcal):
		(((uint)~0) - pi->phy_lastcal + pi->sh->now);

	if (pi_lcn->lcnphy_full_cal_channel == CHSPEC_CHANNEL(pi->radio_chanspec))
#endif /* PHYCAL_CACHING */
		return (time_since_last_cal >= pi->sh->glacial_timer);
	return TRUE;
}

static void
wlc_lcnphy_pll_reset_war(phy_info_t *pi)
{
	bool suspend;
	PHY_TRACE(("Executing PLL reset\n"));
	suspend = !(R_REG(pi->sh->osh, &((phy_info_t *)pi)->regs->maccontrol) & MCTL_EN_MAC);
	if (!suspend)
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
	si_pll_reset(pi->sh->sih);
	/* enable rfseqSoftReset bit */
	PHY_REG_WRITE(pi, LCNPHY, resetCtrl, 0x088);
	OSL_DELAY(5);
	/* disable rfseqSoftReset bit and write default value 0x80  */
	PHY_REG_WRITE(pi, LCNPHY, resetCtrl, 0x080);
	OSL_DELAY(5);
	wlc_lcnphy_4313war(pi);
	if (!suspend)
		wlapi_enable_mac(pi->sh->physhim);
}

void
wlc_lcnphy_check_pllcorruption(phy_info_t *pi)
{
	uint16 iqcc0re, iqcc0im;
	uint16 rfseqtbl[3];
	uint16 resamplertbl[3];
	phytbl_info_t tab;
	uint idx;
	int32 a, b;
	uint16 di0dq0;
	int8 loccValues[6];
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;

	/* Read and store iqcc and locc values */
	wlc_lcnphy_get_tx_iqcc(pi, &iqcc0re, &iqcc0im);
	a = (int16)iqcc0re;
	b = (int16)iqcc0im;
	/* sign extend a, b from 10 bit signed value to 32 bit signed value */
	a = ((a << 22) >> 22);
	b = ((b << 22) >> 22);

	/* Read and store the locc values */
	di0dq0 = wlc_lcnphy_get_tx_locc(pi);
	loccValues[0] = (uint8)(di0dq0 >> 8);
	loccValues[1] = (uint8)(di0dq0 & 0xff);

	wlc_lcnphy_get_radio_loft(pi, (uint8 *)&loccValues[2], (uint8 *)&loccValues[3],
		(uint8 *)&loccValues[4], (uint8 *)&loccValues[5]);

	/* sign extend the loccValues */
	loccValues[2] = (loccValues[2] << 3) >> 3;
	loccValues[3] = (loccValues[3] << 3) >> 3;
	loccValues[4] = (loccValues[4] << 3) >> 3;
	loccValues[5] = (loccValues[5] << 3) >> 3;

	/* Read and store RF seq table values */
	tab.tbl_id = LCNPHY_TBL_ID_RFSEQ;
	tab.tbl_width = 16;	/* 12 bit wide	*/
	tab.tbl_len = 1;
	for (idx = 0; idx < 3; idx++) {
		tab.tbl_ptr = &rfseqtbl[idx];
		tab.tbl_offset = idx;
		wlc_lcnphy_read_table(pi,  &tab);
	}
	/* if RF seq table has changed from init then
	 * it is corrupted and hence force a pll reset
	 */
	for (idx = 0; idx < 3; idx++) {
		if (rfseqtbl[idx] != pi_lcn->rfseqtbl[idx])
		{
			PHY_TRACE(("Executing PLL reset due to RF seq corruption\n"));
			wlc_lcnphy_pll_reset_war(pi);
			break;
		}
	}

	/* Read and store resampler table values */
	tab.tbl_id = 26;
	tab.tbl_width = 16;	/* 9 bit wide	*/
	tab.tbl_len = 1;
	for (idx = 0; idx < 3; idx++) {
		tab.tbl_ptr = &resamplertbl[idx];
		tab.tbl_offset = idx;
		wlc_lcnphy_read_table(pi,  &tab);
	}
	/* if resampler table has changed from init then
	 * it is corrupted and hence force a pll reset
	 */
	for (idx = 0; idx < 3; idx++) {
		if (resamplertbl[idx] != pi_lcn->resamplertbl[idx])
		{
			PHY_TRACE(("Executing PLL reset due to Resampler Tbl corruption\n"));
			wlc_lcnphy_pll_reset_war(pi);
			break;
		}

	}
	/* if RSSI is positive then force a pll reset */
	if (pi_lcn->Rssi > 0) {
		PHY_TRACE(("Executing PLL reset due to positive RSSI\n"));
		pi_lcn->Rssi = 0;
		wlc_lcnphy_pll_reset_war(pi);
	}
}

void
wlc_lcnphy_get_tssi(phy_info_t *pi, int8 *ofdm_pwr, int8 *cck_pwr)
{
	int8 cck_offset;
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	*cck_pwr = 0;
	*ofdm_pwr = 0;

	if (wlc_lcnphy_tssi_based_pwr_ctrl_enabled(pi))	{
		if (phy_utils_read_phyreg(pi, LCNPHY_TxPwrCtrlStatus)
			& LCNPHY_TxPwrCtrlStatus_estPwrValid_MASK)
			*ofdm_pwr =
				(int8)(PHY_REG_READ(pi, LCNPHY, TxPwrCtrlStatus, estPwr) >> 1);
		else if (phy_utils_read_phyreg(pi, LCNPHY_TxPwrCtrlStatusNew2)
			& LCNPHY_TxPwrCtrlStatusNew2_estPwrValid1_MASK)
			*ofdm_pwr =
				(int8)(PHY_REG_READ(pi, LCNPHY, TxPwrCtrlStatusNew2, estPwr1) >> 1);

		if (wlc_phy_tpc_isenabled_lcnphy(pi)) {
			ppr_dsss_rateset_t dsss;
			ppr_get_dsss(pi->tx_power_offset, WL_TX_BW_20, WL_TX_CHAINS_1, &dsss);
			cck_offset = dsss.pwr[0];
		} else
			cck_offset = 0;
		/* change to 6.2 */
		*cck_pwr = *ofdm_pwr + cck_offset;
	}
}

void
WLBANDINITFN(wlc_phy_cal_init_lcnphy)(phy_info_t *pi)
{
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
}

static void
wlc_lcnphy_set_chanspec_tweaks(phy_info_t *pi, chanspec_t chanspec)
{
	uint8 channel = CHSPEC_CHANNEL(chanspec);
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;

#ifdef BAND5G
	phytbl_info_t tab;
	CONST uint32 *gain_tbl_5G;
#endif // endif

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	if (NORADIO_ENAB(pi->pubpi))
		return;

	PHY_REG_LIST_START
		PHY_REG_MOD_ENTRY(LCNPHY, gmShiftReg, goToHiGainInResumeSrch, 0x1)
		PHY_REG_MOD_ENTRY(LCNPHY, gmShiftReg, goToHiGainAfterRifsTmOut, 0x1)
	PHY_REG_LIST_EXECUTE(pi);

	if (CHIPID(pi->sh->chip) == BCM4313_CHIP_ID) {
		if (channel == 14) {
			PHY_REG_MOD(pi, LCNPHY, lpphyCtrl, txfiltSelect, 2);
		} else {
			PHY_REG_MOD(pi, LCNPHY, lpphyCtrl, txfiltSelect, 1);
		}
		pi_lcn->lcnphy_bandedge_corr = 2;
		if (channel == 1)
			pi_lcn->lcnphy_bandedge_corr = 4;
		/* spur/non-spur channel */
		if (channel == 1 || channel == 2 || channel == 3 ||
		    channel == 4 || channel == 9 ||
		    channel == 10 || channel == 11 || channel == 12) {
			si_pmu_pllcontrol(pi->sh->sih, 0x2, 0xffffffff, 0x03000c04);
			si_pmu_pllcontrol(pi->sh->sih, 0x3, 0xffffff, 0x0);
			si_pmu_pllcontrol(pi->sh->sih, 0x4, 0xffffffff, 0x200005c0);
			/* do a pll update */
			si_pmu_pllupd(pi->sh->sih);
			PHY_REG_WRITE(pi, LCNPHY, lcnphy_clk_muxsel1, 0);
			wlc_lcnphy_txrx_spur_avoidance_mode(pi, FALSE);
			pi_lcn->lcnphy_spurmod = 0;
			if (!(BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_FEM_BT) &&
			    EPA(pi_lcn) &&
			    (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA)) {
				PHY_REG_LIST_START
					/* For NON COMBO, EPA boards with EXT LNA */
					PHY_REG_MOD_ENTRY(LCNPHY, LowGainDB, MedLowGainDB, 0x25)
					PHY_REG_WRITE_ENTRY(LCNPHY, VeryLowGainDB, 0x5918)
				PHY_REG_LIST_EXECUTE(pi);
			} else {
				PHY_REG_LIST_START
					PHY_REG_MOD_ENTRY(LCNPHY, LowGainDB, MedLowGainDB, 0x1b)
					PHY_REG_WRITE_ENTRY(LCNPHY, VeryLowGainDB, 0x5907)
				PHY_REG_LIST_EXECUTE(pi);
			}
		} else {
			si_pmu_pllcontrol(pi->sh->sih, 0x2, 0xffffffff, 0x03140c04);
			si_pmu_pllcontrol(pi->sh->sih, 0x3, 0xffffff, 0x333333);
			si_pmu_pllcontrol(pi->sh->sih, 0x4, 0xffffffff, 0x202c2820);
			/* do a pll update */
			si_pmu_pllupd(pi->sh->sih);
			PHY_REG_WRITE(pi, LCNPHY, lcnphy_clk_muxsel1, 0);
			wlc_lcnphy_txrx_spur_avoidance_mode(pi, TRUE);
			/* will become 1 when the functionality has to be brought in */
			pi_lcn->lcnphy_spurmod = 0;
			PHY_REG_MOD(pi, LCNPHY, LowGainDB, MedLowGainDB, 0x25);
			if (!(BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_FEM_BT) &&
			    EPA(pi_lcn) &&
			    (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA)) {
				PHY_REG_WRITE(pi, LCNPHY, VeryLowGainDB, 0x5918);
			} else {
				PHY_REG_WRITE(pi, LCNPHY, VeryLowGainDB, 0x590f);
			}
			PHY_REG_LIST_START
				PHY_REG_MOD_ENTRY(LCNPHY, MinPwrLevel, dsssMinPwrLevel, -99)
				PHY_REG_MOD_ENTRY(LCNPHY, ClipCtrThresh, clipCtrThreshLoGain, 0x13)
			PHY_REG_LIST_EXECUTE(pi);
		}
		if (channel == 13) {
			PHY_REG_MOD(pi, LCNPHY, InputPowerDB, inputpwroffsetdb, -5);
		}

	} else if ((CHIPID(pi->sh->chip) == BCM4336_CHIP_ID) ||
	           (CHIPID(pi->sh->chip) == BCM43362_CHIP_ID)) {
		uint16 medLowGain, veryLowGain;
		bool spur_enable;

		if (channel >= 13) {
			spur_enable = (pi->phy_spuravoid != SPURAVOID_DISABLE);
			medLowGain = (27 + 9);
			veryLowGain = (9 + 6);
		} else {
			spur_enable = FALSE;
			medLowGain = 27;
			veryLowGain = 9;
		}
		if (CHIPID(pi->sh->chip) == BCM4336_CHIP_ID) {
			PHY_REG_LIST_START
				PHY_REG_MOD_ENTRY(LCNPHY, MinPwrLevel, ofdmMinPwrLevel, -92)
				PHY_REG_MOD_ENTRY(LCNPHY, MinPwrLevel, dsssMinPwrLevel, -97)
				PHY_REG_MOD_ENTRY(LCNPHY, HiGainDB, MedHiGainDB, 42)
			PHY_REG_LIST_EXECUTE(pi);

			PHY_REG_MOD(pi, LCNPHY, LowGainDB, MedLowGainDB, medLowGain);
			PHY_REG_MOD(pi, LCNPHY, VeryLowGainDB, veryLowGainDB, veryLowGain);

			PHY_REG_LIST_START
				PHY_REG_MOD_ENTRY(LCNPHY, ClipCtrThresh, ClipCtrThreshHiGain, 10)
				PHY_REG_MOD_ENTRY(LCNPHY, InputPowerDB, transientfreeThresh, 1)
			PHY_REG_LIST_EXECUTE(pi);
		}

		if ((CHIPID(pi->sh->chip) == BCM43362_CHIP_ID) && pi_lcn->spuravoid_2g) {
			int16 clk_muxsel1;
			bool force_reset = FALSE;

			clk_muxsel1 = phy_utils_read_phyreg(pi, LCNPHY_lcnphy_clk_muxsel1);

			/* lmac/plt driver likes to dump reinitalize phy_info_t state,
			   make sure register state matches expected value
			*/
			if ((spur_enable && clk_muxsel1 != 0xf) ||
				(!spur_enable && clk_muxsel1 != 0)) {
				force_reset = TRUE;
			}

			if (!SCAN_RM_IN_PROGRESS(pi) &&
				((spur_enable != pi_lcn->lcnphy_spurmod) || force_reset)) {
				if (spur_enable) {
					si_pmu_spuravoid(pi->sh->sih, pi->sh->osh, 1);
					wlc_lcnphy_txrx_spur_avoidance_mode(pi, TRUE);
					pi_lcn->lcnphy_spurmod = 1;
				} else {
					si_pmu_spuravoid(pi->sh->sih, pi->sh->osh, 0);
					wlc_lcnphy_txrx_spur_avoidance_mode(pi, FALSE);
					pi_lcn->lcnphy_spurmod = 0;
				}

				PHY_REG_LIST_START
					PHY_REG_MOD_ENTRY(LCNPHY, AfeCtrlOvr, pwdn_adc_ovr, 1)
					PHY_REG_MOD_ENTRY(LCNPHY, AfeCtrlOvrVal, pwdn_adc_ovr_val,
						1)
					PHY_REG_MOD_ENTRY(LCNPHY, AfeCtrlOvrVal, pwdn_adc_ovr_val,
						0)
					PHY_REG_MOD_ENTRY(LCNPHY, AfeCtrlOvr, pwdn_adc_ovr, 0)
				PHY_REG_LIST_EXECUTE(pi);
			}
		}

	}
	else if (CHIPID(pi->sh->chip) == BCM4330_CHIP_ID) {
#ifdef BAND5G
		bool spur_enable;
		if (CHSPEC_IS5G(pi->radio_chanspec) && pi_lcn->spuravoid_5g &&
			!SCAN_RM_IN_PROGRESS(pi)) {
			if (((channel == 56) || (channel == 104) || (channel == 128))) {
				spur_enable = TRUE;
			} else {
				spur_enable = FALSE;
			}

			if (spur_enable && !pi_lcn->lcnphy_spurmod) {
				si_pmu_spuravoid(pi->sh->sih, pi->sh->osh, 1);
				wlc_lcnphy_txrx_spur_avoidance_mode(pi, TRUE);
				pi_lcn->lcnphy_spurmod = 1;
			} else if (!spur_enable && pi_lcn->lcnphy_spurmod) {
				si_pmu_spuravoid(pi->sh->sih, pi->sh->osh, 0);
				wlc_lcnphy_txrx_spur_avoidance_mode(pi, FALSE);
				pi_lcn->lcnphy_spurmod = 0;
			}
		}
		/* Modify some entries in gain table to fix per issues w/ 4330 wlbga on 5G */
		if (CHSPEC_IS5G(pi->radio_chanspec) &&
		    (pi->sh->chippkg == BCM4330_WLBGA_PKG_ID)) {
			if (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) &  BFL_EXTLNA) {
			  int blocker_en = 0;
			  if (wlc_lcnphy_tempsense_degree(pi, 1) < 10) {
			    /* At cold switch in gain table w/ lna1 backed off to fix rx per humps
			       due to gain transitions. Also adjust some agc params to improve per
			       at high input powers
			     */
			    blocker_en = 1;
			    PHY_REG_LIST_START
			        PHY_REG_MOD_ENTRY(LCNPHY, HiGainDB, MedHiGainDB, 40)
			        PHY_REG_MOD_ENTRY(LCNPHY, LowGainDB, MedLowGainDB, 31)
			        PHY_REG_MOD_ENTRY(LCNPHY, ClipCtrThresh, clipCtrThreshLoGain, 15)
			        PHY_REG_MOD_ENTRY(LCNPHY, gaindirectMismatch, medGainGmShftVal, 2)
			        PHY_REG_MOD_ENTRY(LCNPHY, gmShiftReg, medGain2GmShftVal, 2)
			        PHY_REG_MOD_ENTRY(LCNPHY, gmShiftReg, medGain3GmShftVal, 2)
			    PHY_REG_LIST_EXECUTE(pi);
			  } else {
			    PHY_REG_LIST_START
			        PHY_REG_MOD_ENTRY(LCNPHY, HiGainDB, MedHiGainDB, 46)
			        PHY_REG_MOD_ENTRY(LCNPHY, LowGainDB, MedLowGainDB, 37)
			        PHY_REG_MOD_ENTRY(LCNPHY, ClipCtrThresh, clipCtrThreshLoGain, 20)
			        PHY_REG_MOD_ENTRY(LCNPHY, gaindirectMismatch, medGainGmShftVal, 3)
			        PHY_REG_MOD_ENTRY(LCNPHY, gmShiftReg, medGain2GmShftVal, 3)
			        PHY_REG_MOD_ENTRY(LCNPHY, gmShiftReg, medGain3GmShftVal, 3)
			    PHY_REG_LIST_EXECUTE(pi);
			  }
			  PHY_REG_MOD(pi, LCNPHY, radioCtrl, blockerEn, blocker_en);
			    /* Change point at which superbypass mode is applied to fix
			       rx per humps
			     */
			  if (wlc_lcnphy_tempsense_degree(pi, 1) > 45)
			    PHY_REG_MOD(pi, LCNPHY, radioTRCtrlCrs1, trGainThresh, 32);
			  else
			    PHY_REG_MOD(pi, LCNPHY, radioTRCtrlCrs1, trGainThresh, 28);

			} else {
			  int blocker_en = 0;
			   /* At cold switch in gain table w/ lna1 backed off to fix rx per humps
			       due to gain transitions
			   */
			  if ((wlc_lcnphy_tempsense_degree(pi, 1) < 0) && (channel >= 100))
			    blocker_en = 1;

			  PHY_REG_MOD(pi, LCNPHY, radioCtrl, blockerEn, blocker_en);
			}
		}
#endif /* BAND5G */
	}
#ifdef BAND5G
	if ((CHSPEC_IS5G(pi->radio_chanspec)) && (pi_lcn->alt_gaintbl_5g) &&
		(!(BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA_5GHz))) {
		if (((channel >= 54) && (channel <= 64)) ||
			((channel >= 100) && (channel <= 140)) ||
			(channel == 144)) {
			gain_tbl_5G = dot11lcn_gain_tbl_5G_rev2_alt;
		} else {
			gain_tbl_5G = dot11lcn_gain_tbl_5G_rev2;
		}
		tab.tbl_ptr = gain_tbl_5G;
		tab.tbl_len = 96;
		tab.tbl_id = LCNPHY_TBL_ID_GAIN_TBL;
		tab.tbl_offset = 0;
		tab.tbl_width = 32;
		wlc_lcnphy_write_table(pi, &tab);
	}
#endif /* BAND5G */

	PHY_REG_LIST_START
		/* lcnphy_agc_reset */
		PHY_REG_OR_ENTRY(LCNPHY, resetCtrl, 0x44)
		PHY_REG_WRITE_ENTRY(LCNPHY, resetCtrl, 0x80)
	PHY_REG_LIST_EXECUTE(pi);
}

void
wlc_lcnphy_tx_power_adjustment(wlc_phy_t *ppi)
{
	int8 indx;
	uint16 index2;
	phy_info_t *pi = (phy_info_t*)ppi;
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	uint16 SAVE_txpwrctrl = wlc_lcnphy_get_tx_pwr_ctrl(pi);
	if (wlc_lcnphy_tempsense_based_pwr_ctrl_enabled(pi) && SAVE_txpwrctrl) {
		indx = wlc_lcnphy_tempcompensated_txpwrctrl(pi, FALSE);
		index2 = (uint16)(indx * 2);
		PHY_REG_MOD(pi, LCNPHY, TxPwrCtrlBaseIndex, uC_baseIndex0, index2);
		pi_lcn->lcnphy_current_index = (int8)
			((phy_utils_read_phyreg(pi, LCNPHY_TxPwrCtrlBaseIndex) & 0xFF)/2);
	}
}

static void
wlc_lcnphy_restore_txiqlo_calibration_results(phy_info_t *pi, uint16 startidx,
	uint16 stopidx, uint8 index)
{
	phytbl_info_t tab;
	uint16 a, b;
	uint16 didq;
	uint32 val, bbmult_a_b = 0;
	uint idx;
	uint8 ei0, eq0, fi0, fq0;
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
#if defined(PHYCAL_CACHING)
	ch_calcache_t *ctx = wlc_phy_get_chanctx(pi, pi->radio_chanspec);
	lcnphy_calcache_t *cache = &ctx->u.lcnphy_cache;
	a = cache->txiqlocal_a[index];
	b = cache->txiqlocal_b[index];
	didq = cache->txiqlocal_didq[index];
#else
	a = pi_lcn->lcnphy_cal_results.txiqlocal_a[index];
	b = pi_lcn->lcnphy_cal_results.txiqlocal_b[index];
	didq = pi_lcn->lcnphy_cal_results.txiqlocal_didq[index];
#endif // endif

	wlc_lcnphy_set_tx_iqcc(pi, a, b);
	wlc_lcnphy_set_tx_locc(pi, didq);

	/* restore iqlo portion of tx power control tables */
	/* remaining element */
	tab.tbl_id = LCNPHY_TBL_ID_TXPWRCTL;
	tab.tbl_width = 32;	/* 32 bit wide	*/
	tab.tbl_len = 1;        /* # values   */
	tab.tbl_ptr = &val; /* ptr to buf */
	for (idx = startidx; idx <= stopidx; idx++) {
		if (pi_lcn->virtual_p25_tx_gain_step && (idx % 2 == 1))
			continue;
		/* iq */
		tab.tbl_offset = LCNPHY_TX_PWR_CTRL_IQ_OFFSET
			+ idx/(1+pi_lcn->virtual_p25_tx_gain_step);
		wlc_lcnphy_read_table(pi,  &tab);
		val = (val & 0xfff00000) |
			((uint32)(a & 0x3FF) << 10) | (b & 0x3ff);
		bbmult_a_b = val;
		wlc_lcnphy_write_table(pi,  &tab);
		/* loft */
		tab.tbl_offset = LCNPHY_TX_PWR_CTRL_LO_OFFSET
			+ idx/(1+pi_lcn->virtual_p25_tx_gain_step);
		val = didq;
		wlc_lcnphy_write_table(pi,  &tab);
	}
	if ((pi_lcn->virtual_p25_tx_gain_step) && (stopidx == 127)) {
		for (idx = 64; idx < 128; idx++) {
			/* iq */
			tab.tbl_offset = LCNPHY_TX_PWR_CTRL_IQ_OFFSET + idx;
			val = bbmult_a_b;
			wlc_lcnphy_write_table(pi, &tab);

			/* loft */
			val = didq;
			tab.tbl_offset = LCNPHY_TX_PWR_CTRL_LO_OFFSET + idx;
			wlc_lcnphy_write_table(pi, &tab);
		}
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
	wlc_lcnphy_set_radio_loft(pi, ei0, eq0, fi0, fq0);
}

static void
wlc_lcnphy_set_rx_iq_comp(phy_info_t *pi, uint16 a, uint16 b)
{
	PHY_REG_MOD(pi, LCNPHY, RxCompcoeffa0, a0, a);
	PHY_REG_MOD(pi, LCNPHY, RxCompcoeffb0, b0, b);

	PHY_REG_MOD(pi, LCNPHY, RxCompcoeffa1, a1, a);
	PHY_REG_MOD(pi, LCNPHY, RxCompcoeffb1, b1, b);

	PHY_REG_MOD(pi, LCNPHY, RxCompcoeffa2, a2, a);
	PHY_REG_MOD(pi, LCNPHY, RxCompcoeffb2, b2, b);
}

static void
wlc_lcnphy_restore_calibration_results(phy_info_t *pi)
{
#if defined(PHYCAL_CACHING)
	ch_calcache_t *ctx = wlc_phy_get_chanctx(pi, pi->radio_chanspec);
	lcnphy_calcache_t *cache = &ctx->u.lcnphy_cache;
#endif // endif
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;

#if defined(PHYCAL_CACHING) && defined(BCMDBG)
	/* Print the params to be restored */
	wlc_phy_cal_cache_dbg_lcnphy(ctx);
#endif // endif
	/* restore tx iq cal results */
	if (CHSPEC_IS2G(pi->radio_chanspec) && (TXGAINTBL(pi_lcn) == 1)) {
		wlc_lcnphy_restore_txiqlo_calibration_results(pi, 0, 60, 0);
		wlc_lcnphy_restore_txiqlo_calibration_results(pi, 61, 109, 1);
		wlc_lcnphy_restore_txiqlo_calibration_results(pi, 110, 127, 2);
	} else
		wlc_lcnphy_restore_txiqlo_calibration_results(pi, 0, 127, 0);

	/* restore PAPD cal results */
	if (PAPD_4336_MODE(pi_lcn) && !EPA(pi_lcn)) {
		wlc_lcnphy_restore_papd_calibration_results(pi);
	}

	/* restore rx iq cal results */
#if defined(PHYCAL_CACHING)
	wlc_lcnphy_set_rx_iq_comp(pi, cache->rxiqcal_coeff_a0,
		cache->rxiqcal_coeff_b0);
#else
	wlc_lcnphy_set_rx_iq_comp(pi, pi_lcn->lcnphy_cal_results.rxiqcal_coeff_a0,
		pi_lcn->lcnphy_cal_results.rxiqcal_coeff_b0);
#endif // endif
	PHY_REG_MOD(pi, LCNPHY, rxfe, bypass_iqcomp, 0);
}

void
WLBANDINITFN(wlc_phy_init_lcnphy)(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;

	PHY_TRACE(("%s:\n", __FUNCTION__));

	if (CHIPID(pi->sh->chip) != BCM4313_CHIP_ID)
		pi_lcn->lcnphy_papd_4336_mode = TRUE;
	else
		pi_lcn->lcnphy_papd_4336_mode = FALSE;

	if ((BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_FEM) &&
		(CHIPID(pi->sh->chip) == BCM4313_CHIP_ID))
		pi_lcn->ePA = 1;

	if (PAPD_4336_MODE(pi_lcn)) {
		pi_lcn->ePA = 0;
		if ((CHSPEC_IS2G(pi->radio_chanspec) &&
			pi_lcn->extpagain2g == 2) ||
			(CHSPEC_IS5G(pi->radio_chanspec) &&
			pi_lcn->extpagain5g == 2))
			pi_lcn->ePA = 1;
	}

	ASSERT(EPA(pi_lcn) == pi_lcn->ePA);
	ASSERT(PAPD_4336_MODE(pi_lcn) == pi_lcn->lcnphy_papd_4336_mode);

	if (CHSPEC_IS5G(pi->radio_chanspec))
		pi_lcn->dacrate = pi_lcn->dacrate_5g;
	else
		pi_lcn->dacrate = pi_lcn->dacrate_2g;

	pi_lcn->lcnphy_cal_counter = 0;
	pi_lcn->lcnphy_capped_index = 0;
	pi_lcn->lcnphy_calreqd = 0;
	pi_lcn->lcnphy_CalcPapdCapEnable = 0;
#if !defined(PHYCAL_CACHING)
	pi_lcn->lcnphy_cal_temper = pi_lcn->lcnphy_rawtempsense;
#endif // endif
	pi_lcn->papd_num_lut_used = 1;

	PHY_REG_LIST_START_WLBANDINITDATA
		/* reset the radio */
		PHY_REG_OR_ENTRY(LCNPHY, resetCtrl, 0x80)
		PHY_REG_AND_ENTRY(LCNPHY, resetCtrl, 0x7f)
	PHY_REG_LIST_EXECUTE(pi);

	/* initializing the adc-presync and auxadc-presync for 2x sampling */
	wlc_lcnphy_afe_clk_init(pi, AFE_CLK_INIT_MODE_TXRX2X);

	PHY_REG_LIST_START_WLBANDINITDATA
		/* txrealframe delay increased to ~2us */
		PHY_REG_WRITE_ENTRY(LCNPHY, TxRealFrameDelay, 160)
		/* phy crs delay adj for spikes */
		PHY_REG_WRITE_ENTRY(LCNPHY, ccktx_phycrsDelayValue, 25)
	PHY_REG_LIST_EXECUTE(pi);

	/* PR82977: Fix the rifsSttimeout to 4usec (default 100 --> 5usec) to
	 * get the SIFS to be 16usec. For 4313, it is observed that ack was
	 * sent at ~17usec.
	 */
	if (CHIPID(pi->sh->chip) == BCM4313_CHIP_ID) {
		PHY_REG_MOD(pi, LCNPHY, rifsSttimeout, rifsStTimeout, 80);
	}

	/* Initialize baseband : bu_tweaks is a placeholder */
	wlc_lcnphy_baseband_init(pi);

	/* Initialize radio */
	wlc_lcnphy_radio_init(pi);

	/* Initialize power control */
	wlc_lcnphy_tx_pwr_ctrl_init((wlc_phy_t *) pi);

	/* Initialize spur avoidance mode */
	pi_lcn->lcnphy_spurmod = 0;

	/* Tune to the current channel */
	/* mapped to lcnphy_set_chan_raw minus the agc_temp_init, txpwrctrl init */
	wlc_phy_chanspec_set((wlc_phy_t*)pi, pi->radio_chanspec);

	if (CHIPID(pi->sh->chip) == BCM4313_CHIP_ID) {
		/* set ldo voltage to 1.2 V */
		si_pmu_regcontrol(pi->sh->sih, 0, 0xf, 0x9);
		/* reduce crystal's drive strength */
		si_pmu_chipcontrol(pi->sh->sih, 0, 0xffffffff, 0x03CDDDDD);
	}

	/* here we are forcing the index to 78 */
	if (EPA(pi_lcn) && wlc_lcnphy_tempsense_based_pwr_ctrl_enabled(pi))
		wlc_lcnphy_set_tx_pwr_by_index(pi, FIXED_TXPWR);

	/* save default params for AGC temp adjustments */
	wlc_lcnphy_agc_temp_init(pi);

	wlc_lcnphy_temp_adj(pi);

	PHY_REG_MOD(pi, LCNPHY, lpphyCtrl, resetCCA, 1);
	OSL_DELAY(100);
	PHY_REG_MOD(pi, LCNPHY, lpphyCtrl, resetCCA, 0);
	wlc_lcnphy_set_tx_pwr_ctrl(pi, LCNPHY_TX_PWR_CTRL_HW);
	pi_lcn->lcnphy_noise_samples = PHY_NOISE_SAMPLES_DEFAULT;

	/* Only 4313 does an init time full cal */
	if (CHIPID(pi->sh->chip) == BCM4313_CHIP_ID) {
		wlc_lcnphy_calib_modes(pi, PHY_FULLCAL);
		if (!EPA(pi_lcn))
			pi_lcn->lcnphy_calreqd = 1;
	}
	if (CHIPID(pi->sh->chip) == BCM4313_CHIP_ID) {
		/* Disable any RF overrides since ucode checks if rf overides
		 * are set. If they are set then ucode exits noise cal
		 * routines without measurements
		 */
		wlc_lcnphy_disable_tx_gain_override(pi);
	}
	pi_lcn->startup_init = 1;
}

static void
wlc_lcnphy_tx_iqlo_loopback(phy_info_t *pi, uint16 *values_to_save)
{
	uint16 vmid;
	uint i;
	for (i = 0; i < ARRAYSIZE(iqlo_loopback_rf_regs); i++) {
		values_to_save[i] = phy_utils_read_radioreg(pi, iqlo_loopback_rf_regs[i]);
	}
	PHY_REG_LIST_START
		/* force tx on, rx off , force ADC on */
		PHY_REG_MOD_ENTRY(LCNPHY, RFOverride0, internalrftxpu_ovr, 1)
		PHY_REG_MOD_ENTRY(LCNPHY, RFOverrideVal0, internalrftxpu_ovr_val, 1)
		PHY_REG_MOD_ENTRY(LCNPHY, RFOverride0, internalrfrxpu_ovr, 1)
		PHY_REG_MOD_ENTRY(LCNPHY, RFOverrideVal0, internalrfrxpu_ovr_val, 0)
		PHY_REG_MOD_ENTRY(LCNPHY, AfeCtrlOvr, pwdn_dac_ovr, 1)
		PHY_REG_MOD_ENTRY(LCNPHY, AfeCtrlOvrVal, pwdn_dac_ovr_val, 0)
		PHY_REG_MOD_ENTRY(LCNPHY, AfeCtrlOvr, pwdn_adc_ovr, 1)
		PHY_REG_MOD_ENTRY(LCNPHY, AfeCtrlOvrVal, pwdn_adc_ovr_val, 0)
	PHY_REG_LIST_EXECUTE(pi);

	/* PA override */
	if (RADIOID(pi->pubpi.radioid) != BCM2064_ID) {
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			phy_utils_and_radioreg(pi, RADIO_2064_REG03A, 0xFD);
		} else {
			phy_utils_or_radioreg(pi, RADIO_2064_REG0C0, 1 << 0);
			phy_utils_or_radioreg(pi, RADIO_2064_REG0CA, 1 << 0);
			wlc_lcnphy_set_pa_gain(pi, 0x10);
		}
	}
	else
		phy_utils_and_radioreg(pi, RADIO_2064_REG03A, 0xFB);

	if (CHIPID(pi->sh->chip) == BCM4313_CHIP_ID)
		phy_utils_and_radioreg(pi, RADIO_2064_REG03A, 0xF9);

	phy_utils_or_radioreg(pi, RADIO_2064_REG11A, 0x1);
	/* envelope detector */
	if (RADIOID(pi->pubpi.radioid) == BCM2064_ID) {
		phy_utils_or_radioreg(pi, RADIO_2064_REG036, 0x01);
		phy_utils_or_radioreg(pi, RADIO_2064_REG11A, 0x10);
	}
	else {
		PHY_REG_LIST_START
			RADIO_REG_OR_ENTRY(RADIO_2064_REG036, 0x01)
			RADIO_REG_OR_ENTRY(RADIO_2064_REG0C5, 0x01)
			RADIO_REG_OR_ENTRY(RADIO_2064_REG11A, 0x08)
		PHY_REG_LIST_EXECUTE(pi);
	}
	OSL_DELAY(20);

	if (CHSPEC_IS2G(pi->radio_chanspec) && EPA(pi->u.pi_lcnphy) &&
		(CHIPID(pi->sh->chip) != BCM4313_CHIP_ID)) {
		phy_utils_mod_radioreg(pi, RADIO_2064_REG036, 1<<0, 0);	/* tap at PA output */
		phy_utils_mod_radioreg(pi, RADIO_2064_REG03A, 1<<1, 1<<1);
		if (pi->u.pi_lcnphy->txiqlopag_2g == -1)
			wlc_lcnphy_set_pa_gain(pi, 0x40);
		else
			wlc_lcnphy_set_pa_gain(pi, pi->u.pi_lcnphy->txiqlopag_2g);

		if (pi->u.pi_lcnphy->txiqlopapu_2g == -1)
			wlc_lcnphy_epa_pd(pi, 1);
		else
			wlc_lcnphy_epa_pd(pi, !pi->u.pi_lcnphy->txiqlopapu_2g);
	}
#ifdef BAND5G
	else if (CHSPEC_IS5G(pi->radio_chanspec) && EPA(pi->u.pi_lcnphy)) {
		if (pi->u.pi_lcnphy->txiqlopag_5g == -1)
			wlc_lcnphy_set_pa_gain(pi, 0x30);
		else
			wlc_lcnphy_set_pa_gain(pi, pi->u.pi_lcnphy->txiqlopag_5g);

		if (pi->u.pi_lcnphy->txiqlopapu_5g == -1)
			wlc_lcnphy_epa_pd(pi, 1);
		else
			wlc_lcnphy_epa_pd(pi, !pi->u.pi_lcnphy->txiqlopapu_5g);
	}
#endif // endif

	/* turn on TSSI, pick band */
	if (RADIOID(pi->pubpi.radioid) == BCM2064_ID) {
		phy_utils_or_radioreg(pi, RADIO_2064_REG03A, 1);
		if (CHIPID(pi->sh->chip) == BCM4313_CHIP_ID)
			phy_utils_or_radioreg(pi, RADIO_2064_REG03A, 0x3);
	}
	else if (CHSPEC_IS5G(pi->radio_chanspec))
		phy_utils_or_radioreg(pi, RADIO_2064_REG0CA, 1 << 1);

	OSL_DELAY(20);

	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		phy_utils_mod_radioreg(pi, RADIO_2064_REG03A, 0x1, 1 << 0);
	} else {
		phy_utils_mod_radioreg(pi, RADIO_2064_REG03A, 0x1, 0 << 0);
		phy_utils_mod_radioreg(pi, RADIO_2064_REG0CA, 0x4, 1 << 2);
	}
	if (RADIOID(pi->pubpi.radioid) == BCM2064_ID)
		phy_utils_or_radioreg(pi, RADIO_2064_REG11A, 0x8);
	/* iqcal block */
	phy_utils_write_radioreg(pi, RADIO_2064_REG025, 0xE);
	phy_utils_or_radioreg(pi, RADIO_2064_REG025, 1 << 0);

	if (RADIOID(pi->pubpi.radioid) == BCM2066_ID) {
		if (CHSPEC_IS5G(pi->radio_chanspec))
			phy_utils_mod_radioreg(pi, RADIO_2064_REG028, 0xF, 0x4);
		else
			phy_utils_mod_radioreg(pi, RADIO_2064_REG028, 0xF, 0x6);
	} else {
		if (CHSPEC_IS5G(pi->radio_chanspec))
			phy_utils_mod_radioreg(pi, RADIO_2064_REG028, 0x1e, 0x4 << 1);
		else
			phy_utils_mod_radioreg(pi, RADIO_2064_REG028, 0x1e, 0x6 << 1);
	}

	OSL_DELAY(20);
	/* set testmux to iqcal */
	phy_utils_write_radioreg(pi, RADIO_2064_REG005, 0x8);
	phy_utils_or_radioreg(pi, RADIO_2064_REG112, 0x80);
	OSL_DELAY(20);
	/* enable aux adc */
	phy_utils_or_radioreg(pi, RADIO_2064_REG0FF, 0x10);
	phy_utils_or_radioreg(pi, RADIO_2064_REG11F, 0x44);
	OSL_DELAY(20);
	/* enable 'Rx Buff' */
	phy_utils_or_radioreg(pi, RADIO_2064_REG00B, 0x7);
	phy_utils_or_radioreg(pi, RADIO_2064_REG113, 0x10);
	OSL_DELAY(20);
	/* BB */
	phy_utils_write_radioreg(pi, RADIO_2064_REG007, 0x1);
	OSL_DELAY(20);
	/* ADC Vmid , range etc. */
	vmid = 0x2A6;
	phy_utils_mod_radioreg(pi, RADIO_2064_REG0FC, 0x3 << 0, (vmid >> 8) & 0x3);
	phy_utils_write_radioreg(pi, RADIO_2064_REG0FD, (vmid & 0xff));
	phy_utils_or_radioreg(pi, RADIO_2064_REG11F, 0x44);
	OSL_DELAY(20);
	/* power up aux pga */
	phy_utils_or_radioreg(pi, RADIO_2064_REG0FF, 0x10);
	OSL_DELAY(20);
	/* tssi range */
	if (CHSPEC_IS5G(pi->radio_chanspec)) {
		PHY_REG_LIST_START
			PHY_REG_WRITE_ENTRY(LCNPHY, TxPwrCtrlRfCtrlOverride0, 0)
			PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlRfCtrlOverride0,
				tssiRangeOverride, 1)
			PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlRfCtrlOverride0,
				tssiRangeOverrideVal, 0)
		PHY_REG_LIST_EXECUTE(pi);
	} else {
		PHY_REG_LIST_START
			PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlRfCtrlOverride0,
				tssiRangeOverride, 1)
			PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlRfCtrlOverride0,
				tssiRangeOverrideVal, 1)
		PHY_REG_LIST_EXECUTE(pi);
	}

	/* phy_utils_write_radioreg(pi, RADIO_2064_REG012, 0x02); */
	if (LCNREV_LT(pi->pubpi.phy_rev, 1)) {
		phy_utils_or_radioreg(pi, RADIO_2064_REG112, 0x06);
	}
	PHY_REG_LIST_START
		RADIO_REG_WRITE_ENTRY(RADIO_2064_REG059, 0xcc)
		RADIO_REG_WRITE_ENTRY(RADIO_2064_REG05C, 0x2e)
		RADIO_REG_WRITE_ENTRY(RADIO_2064_REG078, 0xd7)
		RADIO_REG_WRITE_ENTRY(RADIO_2064_REG092, 0x15)
	PHY_REG_LIST_EXECUTE(pi);
}

static void
wlc_lcnphy_samp_cap(phy_info_t *pi, int clip_detect_algo, uint16 thresh, int16* ptr, int mode)
{
	uint32 curval1, curval2, stpptr, curptr, strptr, val;
	uint16 sslpnCalibClkEnCtrl, timer;
	uint16 old_sslpnCalibClkEnCtrl;
	int16 imag, real;
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	timer = 0;
	old_sslpnCalibClkEnCtrl = phy_utils_read_phyreg(pi, LCNPHY_sslpnCalibClkEnCtrl);
	/* ********************samp capture ******************** */
	/* switch dot11mac_clk to macphy clock (80 MHz) */
	curval1 = R_REG(pi->sh->osh, &pi->regs->psm_corectlsts);
	ptr[130] = 0;
	W_REG(pi->sh->osh, &pi->regs->psm_corectlsts, ((1 << 6) | curval1));
	/* using the last 512 locations of the Tx fifo for sample capture */
	W_REG(pi->sh->osh, &pi->regs->PHYREF_SMPL_CLCT_STRPTR, 0x7E00);
	W_REG(pi->sh->osh, &pi->regs->PHYREF_SMPL_CLCT_STPPTR, 0x8000);
	OSL_DELAY(20);
	curval2 = R_REG(pi->sh->osh, &pi->regs->psm_phy_hdr_param);
	W_REG(pi->sh->osh, &pi->regs->psm_phy_hdr_param, curval2 | 0x30);

	PHY_REG_LIST_START
		/* configure phy to do a timer based capture of mode 2(adc o/p) */
		PHY_REG_WRITE_ENTRY(LCNPHY, triggerConfiguration, 0x0)
		PHY_REG_WRITE_ENTRY(LCNPHY, sslpnphy_gpio_ctrl, 0x5)
	PHY_REG_LIST_EXECUTE(pi);

	/* if dac has to be captured, the module sel will be 19 */
	PHY_REG_WRITE(pi, LCNPHY, sslpnphy_gpio_sel, (uint16)(mode | mode << 6));
	PHY_REG_LIST_START
		PHY_REG_WRITE_ENTRY(LCNPHY, gpio_data_ctrl, 3)
		PHY_REG_WRITE_ENTRY(LCNPHY, sslpnphy_gpio_out_en_34_32, 0x3)
		PHY_REG_WRITE_ENTRY(LCNPHY, dbg_samp_coll_start_mac_xfer_trig_timer_15_0, 0x0)
		PHY_REG_WRITE_ENTRY(LCNPHY, dbg_samp_coll_start_mac_xfer_trig_timer_31_16, 0x0)
		PHY_REG_WRITE_ENTRY(LCNPHY, dbg_samp_coll_end_mac_xfer_trig_timer_15_0, 0x0fff)
		PHY_REG_WRITE_ENTRY(LCNPHY, dbg_samp_coll_end_mac_xfer_trig_timer_31_16, 0x0000)
		/* programming dbg_samp_coll_ctrl_word */
		PHY_REG_WRITE_ENTRY(LCNPHY, dbg_samp_coll_ctrl, 0x4501)
	PHY_REG_LIST_EXECUTE(pi);

	/* enabling clocks to sampleplay buffer and debug blocks */
	sslpnCalibClkEnCtrl = phy_utils_read_phyreg(pi, LCNPHY_sslpnCalibClkEnCtrl);
	PHY_REG_WRITE(pi, LCNPHY, sslpnCalibClkEnCtrl, (uint32)(sslpnCalibClkEnCtrl | 0x2008));
	stpptr = R_REG(pi->sh->osh, &pi->regs->PHYREF_SMPL_CLCT_STPPTR);
	curptr = R_REG(pi->sh->osh, &pi->regs->PHYREF_SMPL_CLCT_CURPTR);
	do {
		OSL_DELAY(10);
		curptr = R_REG(pi->sh->osh, &pi->regs->PHYREF_SMPL_CLCT_CURPTR);
		timer++;
	} while ((curptr != stpptr) && (timer < 500));
	/* PHY_CTL to 2 */
	W_REG(pi->sh->osh, &pi->regs->psm_phy_hdr_param, 0x2);
	strptr = 0x7E00;
	wlapi_bmac_templateptr_wreg(pi->sh->physhim, strptr);
	while (strptr < 0x8000) {
		val = wlapi_bmac_templatedata_rreg(pi->sh->physhim);
		imag = ((val >> 16) & 0x3ff);
		real = ((val) & 0x3ff);
		if (imag > 511) {
			imag -= 1024;
		}
		if (real > 511) {
			real -= 1024;
		}
		if (pi_lcn->lcnphy_iqcal_swp_dis)
			ptr[(strptr-0x7E00)/4] = real;
		else
			ptr[(strptr-0x7E00)/4] = imag;
		if (clip_detect_algo) {
			if (imag > thresh || imag < -thresh) {
				strptr = 0x8000;
				ptr[130] = 1;
			}
		}
		strptr += 4;
	}
	/* ********************samp capture ends**************** */
	PHY_REG_WRITE(pi, LCNPHY, sslpnCalibClkEnCtrl, old_sslpnCalibClkEnCtrl);
	W_REG(pi->sh->osh, &pi->regs->psm_phy_hdr_param, curval2);
	W_REG(pi->sh->osh, &pi->regs->psm_corectlsts, curval1);
}

/* run soft(samp capture based) tx iqlo cal for all sets of tx iq lo coeffs */
static void
wlc_lcnphy_tx_iqlo_soft_cal_full(phy_info_t *pi)
{
	lcnphy_unsign16_struct iqcc0, locc2, locc3, locc4;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	wlc_lcnphy_set_cc(pi, 0, 0, 0);
	wlc_lcnphy_set_cc(pi, 2, 0, 0);
	wlc_lcnphy_set_cc(pi, 3, 0, 0);
	wlc_lcnphy_set_cc(pi, 4, 0, 0);

	wlc_lcnphy_set_genv(pi, 7);
	/* calls for the iqlo calibration */
	wlc_lcnphy_tx_iqlo_soft_cal(pi, 4, 0, 0);
	wlc_lcnphy_tx_iqlo_soft_cal(pi, 3, 0, 0);
	wlc_lcnphy_tx_iqlo_soft_cal(pi, 2, 3, 2);
	wlc_lcnphy_tx_iqlo_soft_cal(pi, 0, 5, 8);
	wlc_lcnphy_tx_iqlo_soft_cal(pi, 2, 2, 1);
	wlc_lcnphy_tx_iqlo_soft_cal(pi, 0, 4, 3);
	/* get the coeffs */
	iqcc0 = wlc_lcnphy_get_cc(pi, 0);
	locc2 = wlc_lcnphy_get_cc(pi, 2);
	locc3 = wlc_lcnphy_get_cc(pi, 3);
	locc4 = wlc_lcnphy_get_cc(pi, 4);
	PHY_INFORM(("wl%d: %s: Tx IQLO cal complete, coeffs: lcnphy_tx_iqcc: %d, %d\n",
		pi->sh->unit, __FUNCTION__, iqcc0.re, iqcc0.im));
	PHY_INFORM(("wl%d: %s coeffs: lcnphy_tx_locc: %d, %d, %d, %d, %d, %d\n",
		pi->sh->unit, __FUNCTION__, locc2.re, locc2.im,
		locc3.re, locc3.im, locc4.re, locc4.im));
	BCM_REFERENCE(iqcc0);
	BCM_REFERENCE(locc2);
	BCM_REFERENCE(locc3);
	BCM_REFERENCE(locc4);
}

static void
wlc_lcnphy_set_cc(phy_info_t *pi, int cal_type, int16 coeff_x, int16 coeff_y)
{
	uint16 di0dq0;
	uint16 x, y, data_rf;
	int k;
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	switch (cal_type) {
		case 0:
			wlc_lcnphy_set_tx_iqcc(pi, coeff_x, coeff_y);
			break;
		case 2:
			di0dq0 = (coeff_x & 0xff) << 8 | (coeff_y & 0xff);
			wlc_lcnphy_set_tx_locc(pi, di0dq0);
			break;
		case 3:
			k = wlc_lcnphy_calc_floor(coeff_x, 0);
			y = 8 + k;
			k = wlc_lcnphy_calc_floor(coeff_x, 1);
			x = 8 - k;
			data_rf = (x * 16 + y);
			phy_utils_write_radioreg(pi, RADIO_REG_EI(pi), data_rf);
			k = wlc_lcnphy_calc_floor(coeff_y, 0);
			y = 8 + k;
			k = wlc_lcnphy_calc_floor(coeff_y, 1);
			x = 8 - k;
			data_rf = (x * 16 + y);
			phy_utils_write_radioreg(pi, RADIO_REG_EQ(pi), data_rf);
			break;
		case 4:
			k = wlc_lcnphy_calc_floor(coeff_x, 0);
			y = 8 + k;
			k = wlc_lcnphy_calc_floor(coeff_x, 1);
			x = 8 - k;
			data_rf = (x * 16 + y);
			phy_utils_write_radioreg(pi, RADIO_REG_FI(pi), data_rf);
			k = wlc_lcnphy_calc_floor(coeff_y, 0);
			y = 8 + k;
			k = wlc_lcnphy_calc_floor(coeff_y, 1);
			x = 8 - k;
			data_rf = (x * 16 + y);
			phy_utils_write_radioreg(pi, RADIO_REG_FQ(pi), data_rf);
			break;
	}
}

static lcnphy_unsign16_struct
wlc_lcnphy_get_cc(phy_info_t *pi, int cal_type)
{
	uint16 a, b, didq;
	uint8 di0, dq0, ei, eq, fi, fq;
	lcnphy_unsign16_struct cc;
	cc.re = 0;
	cc.im = 0;
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	switch (cal_type) {
		case 0:
			wlc_lcnphy_get_tx_iqcc(pi, &a, &b);
			cc.re = a;
			cc.im = b;
			break;
		case 2:
			didq = wlc_lcnphy_get_tx_locc(pi);
			di0 = (((didq & 0xff00) << 16) >> 24);
			dq0 = (((didq & 0x00ff) << 24) >> 24);
			cc.re = (uint16)di0;
			cc.im = (uint16)dq0;
			break;
		case 3:
			wlc_lcnphy_get_radio_loft(pi, &ei, &eq, &fi, &fq);
			cc.re = (uint16)ei;
			cc.im = (uint16)eq;
			break;
		case 4:
			wlc_lcnphy_get_radio_loft(pi, &ei, &eq, &fi, &fq);
			cc.re = (uint16)fi;
			cc.im = (uint16)fq;
			break;
	}
	return cc;
}
/* envelop detector gain */
static uint16
wlc_lcnphy_get_genv(phy_info_t *pi)
{
	return (phy_utils_read_radioreg(pi, RADIO_2064_REG026) & 0x70) >> 4;
}

static void
wlc_lcnphy_set_genv(phy_info_t *pi, uint16 genv)
{
	phy_utils_write_radioreg(pi, RADIO_2064_REG026, (genv & 0x7) | ((genv & 0x7) << 4));
}

/* minimal code to set the tx gain index */
/* sets bbmult, dac, rf gain, but not pa gain, iq coeffs, etc. */
static void
wlc_lcnphy_set_tx_idx_raw(phy_info_t *pi, uint16 indx)
{
	phytbl_info_t tab;
	uint8 bb_mult;
	uint32 bbmultiqcomp, txgain;
	lcnphy_txgains_t gains;
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;

	ASSERT(indx <= LCNPHY_MAX_TX_POWER_INDEX);
	/* Save forced index */
	pi_lcn->lcnphy_tx_power_idx_override = (int8)indx;
	pi_lcn->lcnphy_current_index = (uint8)indx;

	if (pi_lcn->virtual_p25_tx_gain_step) {
		if (indx % 2 == 1)
			PHY_REG_MOD(pi, LCNPHY, bbShiftCtrl, bbshift, 2);
		else
			PHY_REG_MOD(pi, LCNPHY, bbShiftCtrl, bbshift, 0);
		indx = indx/2;
	}

	/* Preset txPwrCtrltbl */
	tab.tbl_id = LCNPHY_TBL_ID_TXPWRCTL;
	tab.tbl_width = 32;	/* 32 bit wide	*/
	tab.tbl_len = 1;        /* # values   */

	/* Turn off automatic power control */
	/* wlc_lcnphy_set_tx_pwr_ctrl(pi, LCNPHY_TX_PWR_CTRL_OFF); */

	/* Read index based bb_mult, a, b from the table */
	tab.tbl_offset = LCNPHY_TX_PWR_CTRL_IQ_OFFSET + indx; /* iqCoefLuts */
	tab.tbl_ptr = &bbmultiqcomp; /* ptr to buf */
	wlc_lcnphy_read_table(pi,  &tab);

	/* Read index based tx gain from the table */
	tab.tbl_offset = LCNPHY_TX_PWR_CTRL_GAIN_OFFSET + indx; /* gainCtrlLuts */
	tab.tbl_width = 32;
	tab.tbl_ptr = &txgain; /* ptr to buf */
	wlc_lcnphy_read_table(pi,  &tab);
	/* Apply tx gain */
	gains.gm_gain = (uint16)(txgain & 0xff);
	gains.pga_gain = (uint16)(txgain >> 8) & 0xff;
	gains.pad_gain = (uint16)(txgain >> 16) & 0xff;
	gains.dac_gain = (uint16)(bbmultiqcomp >> 28) & 0x07;
	wlc_lcnphy_set_tx_gain(pi, &gains);
	/* Apply bb_mult */
	bb_mult = (uint8)((bbmultiqcomp >> 20) & 0xff);
	wlc_lcnphy_set_bbmult(pi, bb_mult);
	/* Enable gain overrides */
	wlc_lcnphy_enable_tx_gain_override(pi);
}

/* sample capture based tx iqlo cal */
/* cal type : 0 - IQ, 2 -dLO, 3 -eLO, 4 -fLO */
static void
wlc_lcnphy_tx_iqlo_soft_cal(phy_info_t *pi, int cal_type, int num_levels, int step_size_lg2)
{
	const lcnphy_spb_tone_t *tone_out;
	lcnphy_spb_tone_t spb_samp;
	lcnphy_unsign16_struct best;
	int tone_out_sz, genv, k, l, j, spb_index;
	uint16 grid_size, level, thresh, start_tx_idx;
	int16 coeff_max, coeff_x, coeff_y, best_x1, best_y1, best_x, best_y, tx_idx;
	int16 *ptr, samp;
	int32 ripple_re, ripple_im;
	uint32 ripple_abs, ripple_min;
	bool clipped, first_point, prev_clipped, first_pass;
	uint16 save_sslpnCalibClkEnCtrl, save_sslpnRxFeClkEnCtrl;
	uint16 save_rfoverride4, save_txpwrctrlrfctrloverride0, save_txpwrctrlrfctrloverride1;
	uint16 save_reg026;
	uint16 *values_to_save;
	ripple_min = 0;
	coeff_max = best_x1 = best_y1 = level = 0;
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	if ((ptr = MALLOC(pi->sh->osh, sizeof(int16) * 131)) == NULL) {
		PHY_ERROR(("wl%d: %s: MALLOC failure\n", pi->sh->unit, __FUNCTION__));
		return;
	}
	/* initial setup for tx iqlo cal */
	values_to_save = MALLOC(pi->sh->osh, sizeof(uint16) * ARRAYSIZE(iqlo_loopback_rf_regs));
	if (values_to_save == NULL) {
		PHY_ERROR(("wl%d: %s: MALLOC failure\n", pi->sh->unit, __FUNCTION__));
		if (ptr)
			MFREE(pi->sh->osh, ptr, 131 * sizeof(int16));
		return;
	}
	start_tx_idx = tx_idx = wlc_lcnphy_get_current_tx_pwr_idx(pi);
	save_sslpnCalibClkEnCtrl = phy_utils_read_phyreg(pi, LCNPHY_sslpnCalibClkEnCtrl);
	save_sslpnRxFeClkEnCtrl = phy_utils_read_phyreg(pi, LCNPHY_sslpnRxFeClkEnCtrl);
	save_reg026 = phy_utils_read_radioreg(pi, RADIO_2064_REG026);
	PHY_REG_WRITE(pi, LCNPHY, iqloCalGainThreshD2, 0xC0);
	/* start tx tone, load a precalculated tone of same freq into tone_out */
	wlc_lcnphy_start_tx_tone(pi, (3750 * 1000), 88, 0);
	PHY_REG_LIST_START
		PHY_REG_WRITE_ENTRY(LCNPHY, sslpnCalibClkEnCtrl, 0xffff)
		PHY_REG_OR_ENTRY(LCNPHY, sslpnRxFeClkEnCtrl, 0x3)
	PHY_REG_LIST_EXECUTE(pi);
	/* save regs */
	wlc_lcnphy_tx_iqlo_loopback(pi, values_to_save);
	OSL_DELAY(500);
	save_rfoverride4 = phy_utils_read_phyreg(pi, LCNPHY_rfoverride4);
	save_txpwrctrlrfctrloverride0 = phy_utils_read_phyreg(pi, LCNPHY_TxPwrCtrlRfCtrlOverride0);
	save_txpwrctrlrfctrloverride1 = phy_utils_read_phyreg(pi, LCNPHY_TxPwrCtrlRfCtrlOverride1);

	PHY_REG_LIST_START
		PHY_REG_OR_ENTRY(LCNPHY, rfoverride4,
			0x1 << LCNPHY_rfoverride4_lpf_buf_pwrup_ovr_SHIFT)
		PHY_REG_OR_ENTRY(LCNPHY, TxPwrCtrlRfCtrlOverride0,
			(0x1 << LCNPHY_TxPwrCtrlRfCtrlOverride0_paCtrlTssiOverride_SHIFT) |
			(0x1 << LCNPHY_TxPwrCtrlRfCtrlOverride0_amuxSelPortOverride_SHIFT))
		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlRfCtrlOverride0, amuxSelPortOverrideVal, 0x2)
		PHY_REG_OR_ENTRY(LCNPHY, TxPwrCtrlRfCtrlOverride1,
			(1 << LCNPHY_TxPwrCtrlRfCtrlOverride1_afeAuxpgaSelVmidOverride_SHIFT) |
			(1 << LCNPHY_TxPwrCtrlRfCtrlOverride1_afeAuxpgaSelGainOverride_SHIFT))
		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlRfCtrlOverride1,
			afeAuxpgaSelVmidOverrideVal, 0x2A6)
		PHY_REG_MOD_ENTRY(LCNPHY, TxPwrCtrlRfCtrlOverride1,
			afeAuxpgaSelGainOverrideVal, 0x2)
	PHY_REG_LIST_EXECUTE(pi);

	tone_out = &lcnphy_spb_tone_3750[0];
	tone_out_sz = 32;
	/* Grid search */
	if (num_levels == 0) {
		if (cal_type != 0) {
			num_levels = 4;
		} else {
			num_levels = 9;
		}
	}
	if (step_size_lg2 == 0) {
		if (cal_type != 0) {
			step_size_lg2 = 3;
		} else {
			step_size_lg2 = 8;
		}
	}
	/* Grid search inputs */
	grid_size = (1 << step_size_lg2);
	best = wlc_lcnphy_get_cc(pi, cal_type);
	best_x = (int16)best.re;
	best_y = (int16)best.im;
	if (cal_type == 2) {
		if (best.re > 127)
			best_x = best.re - 256;
		if (best.im > 127)
			best_y = best.im - 256;
	}
	wlc_lcnphy_set_cc(pi, cal_type, best_x, best_y);
	OSL_DELAY(20);

	genv = wlc_lcnphy_get_genv(pi);

	for (level = 0; grid_size != 0 && level < num_levels; level++) {
		first_point = 1;
		clipped = 0;
		switch (cal_type) {
			case 0:
				coeff_max = 511;
				break;
			case 2:
				coeff_max = 127;
				break;
			case 3:
				coeff_max = 15;
				break;
			case 4:
				coeff_max = 15;
				break;
		}
		/* gain control */
		thresh = phy_utils_read_phyreg(pi, LCNPHY_iqloCalGainThreshD2);
		thresh = 2*thresh;
		prev_clipped = 0;

		first_pass = 1;
		while (1) {
			wlc_lcnphy_set_genv(pi, (uint16)genv);
			OSL_DELAY(50);
			wlc_lcnphy_set_tx_idx_raw(pi, (uint16)tx_idx);
			clipped = 0;
			ptr[130] = 0;
			wlc_lcnphy_samp_cap(pi, 1, thresh, &ptr[0], 2);
			if (ptr[130] == 1)
				clipped = 1;
			if (clipped) {
				if (tx_idx < start_tx_idx || genv <= 0)
					tx_idx += 12;
				else
					genv -= 1;
			}
			if ((clipped != prev_clipped) && (!first_pass))
				break;
			if (!clipped) {
				if (tx_idx > start_tx_idx || genv >= 7)
					tx_idx -= 12;
				else
					genv += 1;
			}

			if ((genv <= 0 && tx_idx >= 127) || (genv >= 7 && tx_idx <= 0))
				break;
			prev_clipped = clipped;
			first_pass = 0;
			/* limiting genv to be within 0 to 7 */
			if (genv < 0)
				genv = 0;
			else if (genv > 7)
				genv = 7;

			if (tx_idx < 0)
				tx_idx = 0;
			else if (tx_idx > 127)
			tx_idx = 127;
		}
		/* limiting genv to be within 0 to 7 */
		if (genv < 0)
			genv = 0;
		else if (genv > 7)
			genv = 7;

		if (tx_idx < 0)
			tx_idx = 0;
		else if (tx_idx > 127)
			tx_idx = 127;
		/* printf("Best.re , grid size % d %d %d",best.re, best.im,grid_size); */
		for (k = -grid_size; k <= grid_size; k += grid_size) {
			for (l = -grid_size; l <= grid_size; l += grid_size) {
				coeff_x = best_x + k;
				coeff_y = best_y + l;
				/* limiting the coeffs */
				if (coeff_x < -coeff_max)
					coeff_x = -coeff_max;
				else if (coeff_x > coeff_max)
					coeff_x = coeff_max;
				if (coeff_y < -coeff_max)
					coeff_y = -coeff_max;
				else if (coeff_y > coeff_max)
					coeff_y = coeff_max;
				wlc_lcnphy_set_cc(pi, cal_type, coeff_x, coeff_y);
				OSL_DELAY(20);
				wlc_lcnphy_samp_cap(pi, 0, 0, ptr, 2);
				/* run correlator */
				ripple_re = 0;
				ripple_im = 0;
				for (j = 0; j < 128; j++) {
					if (cal_type != 0) {
						spb_index = j % tone_out_sz;
					} else {
						spb_index = (2*j) % tone_out_sz;
					}
					spb_samp.re = tone_out[spb_index].re;
					spb_samp.im = tone_out[spb_index].im;
					samp = ptr[j];
					ripple_re = ripple_re + samp*spb_samp.re;
					ripple_im = ripple_im + samp*spb_samp.im;
				}
				/* this is done to make sure squares of ripple_re and ripple_im */
				/* don't cross the limits of uint32 */
				ripple_re = ripple_re>>10;
				ripple_im = ripple_im>>10;
				ripple_abs = ((ripple_re*ripple_re)+(ripple_im*ripple_im));
				/* ripple_abs = (uint32)phy_utils_sqrt_int((uint32) ripple_abs); */
				/* keep track of minimum ripple bin, best coeffs */
				if (first_point || ripple_abs < ripple_min) {
					ripple_min = ripple_abs;
					best_x1 = coeff_x;
					best_y1 = coeff_y;
				}
				first_point = 0;
			}
		}
		first_point = 1;
		best_x = best_x1;
		best_y = best_y1;
		grid_size = grid_size >> 1;
		wlc_lcnphy_set_cc(pi, cal_type, best_x, best_y);
		OSL_DELAY(20);
	}
	goto cleanup;
cleanup:
	wlc_lcnphy_tx_iqlo_loopback_cleanup(pi, values_to_save);
	wlc_lcnphy_stop_tx_tone(pi);
	PHY_REG_WRITE(pi, LCNPHY, sslpnCalibClkEnCtrl, save_sslpnCalibClkEnCtrl);
	PHY_REG_WRITE(pi, LCNPHY, sslpnRxFeClkEnCtrl, save_sslpnRxFeClkEnCtrl);
	PHY_REG_WRITE(pi, LCNPHY, rfoverride4, save_rfoverride4);
	PHY_REG_WRITE(pi, LCNPHY, TxPwrCtrlRfCtrlOverride0, save_txpwrctrlrfctrloverride0);
	PHY_REG_WRITE(pi, LCNPHY, TxPwrCtrlRfCtrlOverride1, save_txpwrctrlrfctrloverride1);
	phy_utils_write_radioreg(pi, RADIO_2064_REG026, save_reg026);
	/* Free allocated buffer */
	MFREE(pi->sh->osh, values_to_save, ARRAYSIZE(iqlo_loopback_rf_regs) * sizeof(uint16));
	MFREE(pi->sh->osh, ptr, 131 * sizeof(int16));
	wlc_lcnphy_set_tx_idx_raw(pi, start_tx_idx);
}

static void
wlc_lcnphy_tx_iqlo_loopback_cleanup(phy_info_t *pi, uint16 *values_to_save)
{
	uint i;
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	PHY_REG_LIST_START
		/* tx, rx PU */
		PHY_REG_AND_ENTRY(LCNPHY, RFOverride0,
			(uint16)~(LCNPHY_RFOverride0_internalrfrxpu_ovr_MASK |
			LCNPHY_RFOverride0_internalrftxpu_ovr_MASK))
		/* ADC , DAC PU */
		PHY_REG_AND_ENTRY(LCNPHY, AfeCtrlOvr, 0xC)
	PHY_REG_LIST_EXECUTE(pi);

	/* restore the state of radio regs */
	for (i = 0; i < ARRAYSIZE(iqlo_loopback_rf_regs); i++) {
		phy_utils_write_radioreg(pi, iqlo_loopback_rf_regs[i], values_to_save[i]);
	}

	if (EPA(pi->u.pi_lcnphy) && (CHIPID(pi->sh->chip) != BCM4313_CHIP_ID)) {
		wlc_lcnphy_epa_pd(pi, 0);
	}

}

static void
wlc_lcnphy_load_tx_gain_table(
	phy_info_t *pi,
	const lcnphy_tx_gain_tbl_entry * gain_table)
{
	uint32 j, k;
	phytbl_info_t tab;
	uint32 val;
	uint16 pa_gain;
	uint16 gm_gain = 0;
	int16 dac = 0;
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;

	if (!EPA(pi_lcn)) {
		pa_gain = 0x70;

		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			if (CHIPID(pi->sh->chip) == BCM4330_CHIP_ID) {
				if (LCNREV_LE(pi->pubpi.phy_rev, 2))
					pa_gain = 0x50;
				else
					pa_gain = 0x60;
			}
		}

		if (CHIPID(pi->sh->chip) == BCM4313_CHIP_ID)
			pa_gain = 0x60;
	} else
		pa_gain = 0x10;

	if (CHSPEC_IS2G(pi->radio_chanspec) && (pi_lcn->pa_gain_ovr_val_2g != -1))
		pa_gain = pi_lcn->pa_gain_ovr_val_2g;
	else if (CHSPEC_IS5G(pi->radio_chanspec) && (pi_lcn->pa_gain_ovr_val_5g != -1))
		pa_gain = pi_lcn->pa_gain_ovr_val_5g;

	tab.tbl_id = LCNPHY_TBL_ID_TXPWRCTL;
	tab.tbl_width = 32;     /* 32 bit wide  */
	tab.tbl_len = 1;        /* # values   */
	tab.tbl_ptr = &val; /* ptr to buf */

	for (j = 0; j < 128; j++) {
		dac = gain_table[j].dac;
		if ((j % 2 == 1) && pi_lcn->virtual_p25_tx_gain_step)
			continue;
		if (PAPD_4336_MODE(pi_lcn) && !EPA(pi_lcn) &&
			CHSPEC_IS2G(pi->radio_chanspec) && (TXGAINTBL(pi_lcn) != 1))
			gm_gain = 15;
		else if ((CHIPID(pi->sh->chip) == BCM4313_CHIP_ID) && !EPA(pi_lcn))
			gm_gain = 15;
		else if (EPA(pi_lcn) &&	(CHSPEC_IS5G(pi->radio_chanspec)))
			gm_gain = 7;
		else
			gm_gain = gain_table[j].gm;

		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			if (pi_lcn->gmgc2g != -1)
				gm_gain = pi_lcn->gmgc2g;
			if (pi_lcn->dacgc2g != -1)
				dac = pi_lcn->dacgc2g;
		}
#ifdef BAND5G
		else {
			if (pi_lcn->gmgc5g != -1)
				gm_gain = pi_lcn->gmgc5g;
			if (pi_lcn->dacgc5g != -1)
				dac = pi_lcn->dacgc5g;
		}
#endif // endif
		val = (((uint32)pa_gain << 24) |
			(gain_table[j].pad << 16) |
			(gain_table[j].pga << 8) | gm_gain);

		tab.tbl_offset = LCNPHY_TX_PWR_CTRL_GAIN_OFFSET +
			j/(1+pi_lcn->virtual_p25_tx_gain_step);
		wlc_lcnphy_write_table(pi, &tab);

		tab.tbl_ptr = &val; /* ptr to buf */
		tab.tbl_offset = LCNPHY_TX_PWR_CTRL_IQ_OFFSET +
			j/(1+pi_lcn->virtual_p25_tx_gain_step);
		wlc_lcnphy_read_table(pi, &tab);
		val = val & (0xFFFFF);
		val |= (dac << 28) |
			(gain_table[j].bb_mult << 20);
		tab.tbl_offset = LCNPHY_TX_PWR_CTRL_IQ_OFFSET +
			j/(1+pi_lcn->virtual_p25_tx_gain_step);
		wlc_lcnphy_write_table(pi, &tab);
	}

	if (pi_lcn->virtual_p25_tx_gain_step) {

		for (k = 64; k < 128; k++) {
			val = (((uint32)pa_gain << 24) |
			(gain_table[k].pad << 16) |
			(gain_table[k].pga << 8) | gm_gain);

			tab.tbl_offset = LCNPHY_TX_PWR_CTRL_GAIN_OFFSET + k;
			wlc_lcnphy_write_table(pi, &tab);

			tab.tbl_ptr = &val; /* ptr to buf */
			tab.tbl_offset = LCNPHY_TX_PWR_CTRL_IQ_OFFSET + k;
			wlc_lcnphy_read_table(pi, &tab);
			val = val & (0xFFFFF);
			val |= (dac << 28) |
			(gain_table[k].bb_mult << 20);
			tab.tbl_offset = LCNPHY_TX_PWR_CTRL_IQ_OFFSET + k;
			wlc_lcnphy_write_table(pi, &tab);
		}
	}
}

static uint8
wlc_lcnphy_get_papd_rfpwr_scale(phy_info_t *pi)
{
	uint8 papd_rf_pwr_scale; /* Q4, scale papd rf power scale adjustment */
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;

	if (pi_lcn->papd_rf_pwr_scale != 0) {
		papd_rf_pwr_scale = pi_lcn->papd_rf_pwr_scale;
	} else {
		if (CHIPID(pi->sh->chip) == BCM43362_CHIP_ID) {
			papd_rf_pwr_scale = 16;
		} else {
			papd_rf_pwr_scale = 8;
			if (CHSPEC_IS2G(pi->radio_chanspec) && (TXGAINTBL(pi_lcn) == 1))
				papd_rf_pwr_scale = 12;
		}
	}
	return papd_rf_pwr_scale;
}

static uint16
wlc_lcnphy_calc_papd_rf_pwr_offset(phy_info_t *pi, uint16 indx, int8 papd_rf_pwr_scale)
{
	phytbl_info_t tab;
	uint32 val, bbmult, rfgain;
	int16 temp, temp1, temp2, qQ, qQ1, qQ2, shift;
	uint8 scale_factor = 1, j;
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;

	if (papd_rf_pwr_scale == -1) {
		papd_rf_pwr_scale = wlc_lcnphy_get_papd_rfpwr_scale(pi);
	}

	j = (uint8) indx/(1+pi_lcn->virtual_p25_tx_gain_step);
	tab.tbl_id = LCNPHY_TBL_ID_TXPWRCTL;
	tab.tbl_width = 32;     /* 32 bit wide  */
	tab.tbl_len = 1;        /* # values   */

	tab.tbl_ptr = &bbmult; /* ptr to buf */
	tab.tbl_offset = LCNPHY_TX_PWR_CTRL_IQ_OFFSET + j;
	wlc_lcnphy_read_table(pi, &tab);
	bbmult = (bbmult >> 20) & 0xff;

	tab.tbl_ptr = &rfgain; /* ptr to buf */
	tab.tbl_offset = LCNPHY_TX_PWR_CTRL_GAIN_OFFSET + j;
	wlc_lcnphy_read_table(pi, &tab);

	qm_log10((int32)(bbmult), 0, &temp1, &qQ1);
	qm_log10((int32)(1<<6), 0, &temp2, &qQ2);

	if (qQ1 < qQ2) {
		temp2 = qm_shr16(temp2, qQ2-qQ1);
		qQ = qQ1;
	}
	else {
		temp1 = qm_shr16(temp1, qQ1-qQ2);
		qQ = qQ2;
	}
	temp = qm_sub16(temp1, temp2);

	if (qQ >= 4)
		shift = qQ-4;
	else
		shift = 4-qQ;

	val = (((indx << shift) + (5*temp) +
		(1<<(scale_factor+shift-3)))>>(scale_factor+shift-2));

	/* papd_rf_pwr_scale, q4 */
	/* factor of 2 implicit in previous */
	/* calculation */
	val = val*papd_rf_pwr_scale/8;
	PHY_INFORM(("idx = %d, bb: %d, tmp = %d, qQ = %d, sh = %d, val = %d, rfgain = %x\n",
		indx, bbmult, temp, qQ, shift, val, rfgain));

	return (uint16) val;
}

static void
wlc_lcnphy_calc_papd_rf_pwr_offset_arr(phy_info_t *pi, int8 papd_rf_pwr_scale, uint32 *val_arr)
{
	phytbl_info_t tab;
	uint32 val, bbmult_val, bbmult_val_old = 0xffff;
	uint32 *bbmult;
	int16 temp, temp1, temp2, temp1_old = 0, qQ, qQ1, qQ2, qQ1_old = 0, shift;
	uint8 scale_factor = 1, j;
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	uint32 len;

	if (papd_rf_pwr_scale == -1) {
		papd_rf_pwr_scale = wlc_lcnphy_get_papd_rfpwr_scale(pi);
	}
	if (pi_lcn->virtual_p25_tx_gain_step) {
		len = 64;
	} else {
		len = 128;
	}

	bbmult = (uint32 *)MALLOC(pi->sh->osh, sizeof(uint32)*len);
	qm_log10((int32)(1<<6), 0, &temp2, &qQ2);
	tab.tbl_id = LCNPHY_TBL_ID_TXPWRCTL;
	tab.tbl_width = 32;     /* 32 bit wide  */

	if (bbmult == NULL) {
		tab.tbl_len = 1;        /* # values   */
	} else {
		tab.tbl_len = len;
		tab.tbl_ptr = bbmult; /* ptr to buf */
		tab.tbl_offset = LCNPHY_TX_PWR_CTRL_IQ_OFFSET;
		wlc_lcnphy_read_table(pi, &tab);
	}

	for (j = 0; j < len; j++) {
		if (bbmult == NULL) {
			tab.tbl_ptr = &bbmult_val; /* ptr to buf */
			tab.tbl_offset = LCNPHY_TX_PWR_CTRL_IQ_OFFSET+j;
			wlc_lcnphy_read_table(pi, &tab);
			bbmult_val = (bbmult_val >> 20) & 0xff;
		} else {
			bbmult_val = (*(bbmult+j) >> 20) & 0xff;
		}

		if (bbmult_val != bbmult_val_old) {
			qm_log10((int32)(bbmult_val), 0, &temp1, &qQ1);

			bbmult_val_old = bbmult_val;
			qQ1_old = qQ1;
			temp1_old = temp1;
		} else {
			qQ1 = qQ1_old;
			temp1 = temp1_old;
		}
		if (qQ1 < qQ2) {
			temp2 = qm_shr16(temp2, qQ2-qQ1);
			qQ = qQ1;
		}
		else {
			temp1 = qm_shr16(temp1, qQ1-qQ2);
			qQ = qQ2;
		}
		temp = qm_sub16(temp1, temp2);

		if (qQ >= 4)
			shift = qQ-4;
		else
			shift = 4-qQ;

		val = (((j*(1+pi_lcn->virtual_p25_tx_gain_step) << shift) + (5*temp) +
			(1<<(scale_factor+shift-3)))>>(scale_factor+shift-2));

		/* papd_rf_pwr_scale, q4 */
		/* factor of 2 implicit in previous */
		/* calculation */
		val = val*papd_rf_pwr_scale/8;
		*(val_arr+j) = val;
		PHY_INFORM(("idx = %d, bb: %d, tmp = %d, qQ = %d, sh = %d, val = %d\n",
			j, bbmult_val, temp, qQ, shift, val));
	}
	if (bbmult != NULL)
		MFREE(pi->sh->osh, bbmult, len * sizeof(uint32));
	return;
}

static void
wlc_lcnphy_load_rfpower(phy_info_t *pi)
{
	phytbl_info_t tab;
	uint32 val = 0, rfgain, len = 0;
	uint8 indx, gain_tbl_idx;
	uint8 papd_rf_pwr_scale; /* Q4, scale papd rf power scale adjustment */
	uint8 pad, padOld = 32;
	uint16 rfPwrOffset, rfPwrOffsetOld = 0;
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	uint32 *val_arr = NULL;
	papd_rf_pwr_scale = wlc_lcnphy_get_papd_rfpwr_scale(pi);

	tab.tbl_id = LCNPHY_TBL_ID_TXPWRCTL;
	tab.tbl_width = 32;     /* 32 bit wide  */
	tab.tbl_len = 1;        /* # values   */

	if (CHIPID(pi->sh->chip) == BCM4330_CHIP_ID) {
		if (pi_lcn->virtual_p25_tx_gain_step) {
			len = 64;
		} else {
			len = 128;
		}
		val_arr = (uint32 *)MALLOC(pi->sh->osh, sizeof(uint32)*len);
	}

	if ((CHIPID(pi->sh->chip) != BCM4330_CHIP_ID) || (val_arr == NULL)) {
		for (indx = 0; indx < 128; indx++) {
			if (CHIPID(pi->sh->chip) != BCM4313_CHIP_ID) {
				if (pi_lcn->virtual_p25_tx_gain_step) {
					if (indx % 2) {
						continue;
					}
					gain_tbl_idx = indx/2;
				} else {
					gain_tbl_idx = indx;
				}

				val = wlc_lcnphy_calc_papd_rf_pwr_offset
					(pi, indx, papd_rf_pwr_scale);
			} else {
				tab.tbl_ptr = &rfgain;
				tab.tbl_offset = LCNPHY_TX_PWR_CTRL_GAIN_OFFSET + indx;
				wlc_lcnphy_read_table(pi, &tab);

				pad = (rfgain & 0x01f0000) >> 16;
				if (padOld == pad)
				   rfPwrOffset = rfPwrOffsetOld;
				else
				   rfPwrOffset = (uint16)wlc_lcnphy_qdiv_roundup(((padOld -
				                 pad) * 32 * indx), 10, 0);
				padOld = pad;
				rfPwrOffsetOld = rfPwrOffset;
				val = rfPwrOffset;
				gain_tbl_idx = indx;
			}

			tab.tbl_ptr = &val;
			tab.tbl_offset = LCNPHY_TX_PWR_CTRL_PWR_OFFSET + gain_tbl_idx;
			wlc_lcnphy_write_table(pi, &tab);
		}
	} else {
		wlc_lcnphy_calc_papd_rf_pwr_offset_arr(pi, papd_rf_pwr_scale, val_arr);
		tab.tbl_ptr = val_arr;
		tab.tbl_offset = LCNPHY_TX_PWR_CTRL_PWR_OFFSET;
		tab.tbl_len = len;
		val = *(val_arr + len - 1);
		wlc_lcnphy_write_table(pi, &tab);
	}
	if ((CHIPID(pi->sh->chip) != BCM4313_CHIP_ID) && pi_lcn->virtual_p25_tx_gain_step) {
		tab.tbl_len = 1;
		for (indx = 64; indx < 128; indx++) {
			tab.tbl_ptr = &val;
			tab.tbl_offset = LCNPHY_TX_PWR_CTRL_PWR_OFFSET + indx;
			wlc_lcnphy_write_table(pi, &tab);
		}
	}
	if (val_arr != NULL) {
		MFREE(pi->sh->osh, val_arr, sizeof(uint32)*len);
	}
}

static void
wlc_lcnphy_load_txgainwithcappedindex(phy_info_t *pi, bool cap)
{
	uint8 k;
	phytbl_info_t tab;
	uint32 val;
	uint8 indx;
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;

	if (cap) {
		k =  pi_lcn->lcnphy_capped_index;
		tab.tbl_id = LCNPHY_TBL_ID_TXPWRCTL;
		tab.tbl_width = 32;     /* 32 bit wide  */
		tab.tbl_len = 1;
		tab.tbl_ptr = &val; /* ptr to buf */

		for (indx = 0; indx <= pi_lcn->lcnphy_capped_index; indx++) {
			/* Cap the GainOffset table */
			tab.tbl_offset = LCNPHY_TX_PWR_CTRL_GAIN_OFFSET + k;
			wlc_lcnphy_read_table(pi, &tab);
			tab.tbl_offset = LCNPHY_TX_PWR_CTRL_GAIN_OFFSET + indx;
			wlc_lcnphy_write_table(pi, &tab);

			/* Cap the IQOffset table */
			tab.tbl_offset = LCNPHY_TX_PWR_CTRL_IQ_OFFSET + k;
			wlc_lcnphy_read_table(pi, &tab);
			tab.tbl_offset = LCNPHY_TX_PWR_CTRL_IQ_OFFSET + indx;
			wlc_lcnphy_write_table(pi, &tab);

			/* Cap the RF PWR offset table */
			tab.tbl_offset = LCNPHY_TX_PWR_CTRL_PWR_OFFSET + k;
			wlc_lcnphy_read_table(pi, &tab);
			tab.tbl_offset = LCNPHY_TX_PWR_CTRL_PWR_OFFSET + indx;
			wlc_lcnphy_write_table(pi, &tab);
		}
	} else {
		if (CHSPEC_IS2G(pi->radio_chanspec))
			wlc_lcnphy_load_tx_gain_table(pi, dot11lcnphy_2GHz_gaintable_rev0);
		wlc_lcnphy_load_rfpower(pi);
	}

}
/* initialize all the tables defined in auto-generated lcnphytbls.c,
 * see lcnphyprocs.tcl, proc lcnphy_tbl_init
 */

#define LCNPHY_I_WL_TX 0 /* PA1 PA0 Tx1 TX0 */
#define LCNPHY_I_WL_RX 1 /* eLNARx1 eLNARx0 Rx1 RX0 */
#define LCNPHY_I_WL_RX_ATTN 2 /* eLNAAttnRx1 eLNAAttnRx0 AttnRx1 AttnRx0 */
#define LCNPHY_I_BT 3 /* Tx eLNARx Rx */
#define LCNPHY_I_WL_MASK 4 /* ant(1 bit) ovr_en(1 bit) tdm(1 bit) mask(8 bits) */

#define LCNPHY_MASK_BT_RX	0xff
#define LCNPHY_MASK_BT_ELNARX	0xff00
#define LCNPHY_SHIFT_BT_ELNARX	8
#define LCNPHY_MASK_BT_TX	0xff0000
#define LCNPHY_SHIFT_BT_TX	16

#define LCNPHY_MASK_WL_MASK	0xff
#define LCNPHY_MASK_TDM	0x100
#define LCNPHY_MASK_OVR_EN	0x200
#define LCNPHY_MASK_ANT	0x400
#define LCNPHY_SHIFT_ANT	10

#define LCNPHY_SW_CTRL_MAP_ANT 0x1
#define LCNPHY_SW_CTRL_MAP_WL_RX 0x2
#define LCNPHY_SW_CTRL_MAP_WL_TX 0x4
#define LCNPHY_SW_CTRL_MAP_BT_TX 0x8
#define LCNPHY_SW_CTRL_MAP_BT_PRIO 0x10
#define LCNPHY_SW_CTRL_MAP_ELNA 0x20

#define LCNPHY_SW_CTRL_TBL_LENGTH	64
#define LCNPHY_SW_CTRL_TBL_WIDTH	16

#define LCNPHY_DIV_BASED_INIT_H(ant, swidx) \
	(ant)?(swctrlmap[swidx] >> 24):((swctrlmap[swidx] & 0xff0000) >> 16)

#define LCNPHY_DIV_BASED_INIT_L(ant, swidx) \
	(ant)?((swctrlmap[swidx] & 0xff00) >> 8):(swctrlmap[swidx] & 0xff)

static void
WLBANDINITFN(wlc_lcnphy_sw_ctrl_tbl_init)(phy_info_t *pi)
{
	phytbl_info_t tab;
	uint16 *tbl_ptr;
	uint16 tblsz;
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	uint8 idx;
	uint32 *swctrlmap;
	uint16 tdm, ovr_en;

	tblsz = LCNPHY_SW_CTRL_TBL_LENGTH * (LCNPHY_SW_CTRL_TBL_WIDTH >> 3);

	if ((tbl_ptr = MALLOC(pi->sh->osh, tblsz)) == NULL) {
		PHY_ERROR(("wl%d: %s: MALLOC failure\n",
			pi->sh->unit, __FUNCTION__));
		return;
	}

	swctrlmap = pi_lcn->swctrlmap_2g;
#ifdef BAND5G
	if (CHSPEC_IS5G(pi->radio_chanspec))
		swctrlmap = pi_lcn->swctrlmap_5g;
#endif // endif

	tdm = (swctrlmap[LCNPHY_I_WL_MASK] & LCNPHY_MASK_TDM);
	ovr_en = (swctrlmap[LCNPHY_I_WL_MASK] & LCNPHY_MASK_OVR_EN);

	for (idx = 0; idx < LCNPHY_SW_CTRL_TBL_LENGTH; idx++) {
		uint8	bt_pri = idx & LCNPHY_SW_CTRL_MAP_BT_PRIO;
		uint16	ant = idx & LCNPHY_SW_CTRL_MAP_ANT;

		tbl_ptr[idx] = 0;
		/* BT Prio */
		if (bt_pri) {
			/* Disable diversity in WL to ensure
			both BT and WL receive from the same ant
			*/
			if (ovr_en)
				ant = (swctrlmap[LCNPHY_I_WL_MASK] & LCNPHY_MASK_ANT)
						>> LCNPHY_SHIFT_ANT;

			if (idx & LCNPHY_SW_CTRL_MAP_BT_TX)
				/* BT Tx */
				tbl_ptr[idx] |=
				(swctrlmap[LCNPHY_I_BT] & LCNPHY_MASK_BT_TX) >> LCNPHY_SHIFT_BT_TX;
			else {
				/* BT Rx */
				if (idx & LCNPHY_SW_CTRL_MAP_ELNA)
					tbl_ptr[idx] |=
					(swctrlmap[LCNPHY_I_BT] & LCNPHY_MASK_BT_ELNARX)
					>> LCNPHY_SHIFT_BT_ELNARX;
				else
					tbl_ptr[idx] |=
					swctrlmap[LCNPHY_I_BT] & LCNPHY_MASK_BT_RX;
			}
		}
		/* WL Tx/Rx */
		if (!tdm || !bt_pri) {
			if (idx & LCNPHY_SW_CTRL_MAP_WL_TX) {
				/* PA on */
				if (idx & LCNPHY_SW_CTRL_MAP_WL_RX)
					/* Rx with PA on */
					tbl_ptr[idx] |=
					LCNPHY_DIV_BASED_INIT_H(ant, LCNPHY_I_WL_TX);
				else
					/* WL Tx with PA on */
					tbl_ptr[idx] |=
					LCNPHY_DIV_BASED_INIT_L(ant, LCNPHY_I_WL_TX);
			} else {
				if (idx & LCNPHY_SW_CTRL_MAP_ELNA) {
					if (idx & LCNPHY_SW_CTRL_MAP_WL_RX)
						/* WL Rx eLNA */
						tbl_ptr[idx] |=
						LCNPHY_DIV_BASED_INIT_H(ant, LCNPHY_I_WL_RX);
					else
						/* WL Rx Attn eLNA */
						tbl_ptr[idx] |=
						LCNPHY_DIV_BASED_INIT_H
						(ant, LCNPHY_I_WL_RX_ATTN);
				} else { /* Without eLNA */
					if (idx & LCNPHY_SW_CTRL_MAP_WL_RX)
						/* WL Rx */
						tbl_ptr[idx] |=
						LCNPHY_DIV_BASED_INIT_L(ant, LCNPHY_I_WL_RX);
					else
						/* WL Rx Attn */
						tbl_ptr[idx] |=
						LCNPHY_DIV_BASED_INIT_L(ant, LCNPHY_I_WL_RX_ATTN);
				}
			}
		}
	}
	/* Writing the following fields into the LCNPHY_swctrlconfig register
	  * (swctrlmap[LCNPHY_I_WL_MASK] & 0ff) to sw_ctrl_mask
	  * 0 to swCtrl_input_mux_ext_lna_gain
	  * 0 to swCtrl_input_mux_ercx_prisel
	 */
	if (CHIPID(pi->sh->chip) == BCM4330_CHIP_ID) {
		uint16 mask = (uint16)(swctrlmap[LCNPHY_I_WL_MASK] & LCNPHY_MASK_WL_MASK)
			<< LCNPHY_swctrlconfig_swctrlmask_SHIFT;

		phy_utils_mod_phyreg(pi, LCNPHY_swctrlconfig,
			(LCNPHY_swctrlconfig_swctrlmask_MASK |
			LCNPHY_swctrlconfig_swCtrlinputmuxextlnagain_MASK |
			LCNPHY_swctrlconfig_swCtrlinputmuxercxprisel_MASK),
			mask);
	}

	/* Write the populated sw ctrl table to the default switch ctrl table location */
	tab.tbl_len = LCNPHY_SW_CTRL_TBL_LENGTH;
	tab.tbl_id = LCNPHY_TBL_ID_SW_CTRL;
	tab.tbl_offset = 0;
	tab.tbl_width = LCNPHY_SW_CTRL_TBL_WIDTH;
	tab.tbl_ptr = tbl_ptr;
	wlc_lcnphy_write_table(pi, &tab);

	MFREE(pi->sh->osh, tbl_ptr, tblsz);
}

static void
wlc_lcnphy_rx_gain_tbl_init(phy_info_t *pi)
{
	uint idx;
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;

	if (LCNREV_GE(pi->pubpi.phy_rev, 2)) {
		if (RADIOID(pi->pubpi.radioid) == BCM2064_ID) {
			/* BCM43362 */
			dot11lcnphytbl_info_t *lcnphytbl =
				(phytbl_info_t *)pi_lcn->gain_table_rx_2g;
			for (idx = 0; idx < pi_lcn->num_gain_table_rx_elem_2g; idx++)
					wlc_lcnphy_write_table(pi, &lcnphytbl[idx]);
		} else {
			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				dot11lcnphytbl_info_t *lcnphytbl =
					(phytbl_info_t *)pi_lcn->gain_table_rx_2g;

				for (idx = 0; idx < pi_lcn->num_gain_table_rx_elem_2g; idx++)
					wlc_lcnphy_write_table(pi, &lcnphytbl[idx]);
			}
#ifdef BAND5G
			else {
				dot11lcnphytbl_info_t *lcnphytbl =
					(phytbl_info_t *)pi_lcn->gain_table_rx_5g;
				for (idx = 0; idx < pi_lcn->num_gain_table_rx_elem_5g; idx++)
					wlc_lcnphy_write_table(pi, &lcnphytbl[idx]);
			}
#endif /* BAND5G */
		}
	} else {
		if (LCNREV_IS(pi->pubpi.phy_rev, 1)) {
			if (CHIPID(pi->sh->chip) == BCM4313_CHIP_ID) {
				if (!(BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_FEM_BT) &&
					EPA(pi_lcn) &&
					(BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) &
					BFL_EXTLNA)) {
					dot11lcnphytbl_info_t *lcnphytbl =
						(phytbl_info_t *)pi_lcn->gain_table_rx_2g;
					for (idx = 0; idx <	pi_lcn->num_gain_table_rx_elem_2g;
						idx++) {
						wlc_lcnphy_write_table(pi, &lcnphytbl[idx]);
					}
				}
			}
		}
	}

	/* Using A0's Rx gain table for B0 */
}

static bool
BCMATTACHFN(wlc_lcnphy_rx_gain_tbl_select)(phy_info_t *pi)
{
	uint idx;
	dot11lcnphytbl_info_t *lcnphytbl = NULL;
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	pi_lcn->gain_table_rx_2g = NULL;
#ifdef BAND5G
	pi_lcn->gain_table_rx_5g = NULL;
#endif // endif

	if (LCNREV_GE(pi->pubpi.phy_rev, 2)) {

		if (RADIOID(pi->pubpi.radioid) == BCM2064_ID) {
			/* BCM43362 */
			pi_lcn->num_gain_table_rx_elem_2g =
				dot11lcnphytbl_radio2064_rx_gain_info_sz_rev3;
			pi_lcn->gain_table_rx_2g = MALLOC(pi->sh->osh,
				pi_lcn->num_gain_table_rx_elem_2g * sizeof(dot11lcnphytbl_info_t));
			if (!pi_lcn->gain_table_rx_2g) {
				PHY_ERROR(("wl%d: %s: MALLOC failure\n", pi->sh->unit,
				__FUNCTION__));
				return FALSE;
			}

			lcnphytbl = (dot11lcnphytbl_info_t *) pi_lcn->gain_table_rx_2g;

			for (idx = 0; idx < pi_lcn->num_gain_table_rx_elem_2g; idx++)
				if (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA)
					lcnphytbl[idx] =
					dot11lcnphytbl_radio2064_ext_lna_rx_gain_info_rev3[idx];
				else
					lcnphytbl[idx] =
						dot11lcnphytbl_radio2064_rx_gain_info_rev3[idx];

				/* Preserve the tables */
				if (!wlc_lcnphy_rx_gain_tbl_copy(pi, TRUE))
					return FALSE;
		} else {
			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				pi_lcn->num_gain_table_rx_elem_2g =
					dot11lcnphytbl_2G_wlcsp_rx_gain_info_sz_rev2;
				pi_lcn->gain_table_rx_2g = MALLOC(pi->sh->osh,
				pi_lcn->num_gain_table_rx_elem_2g * sizeof(dot11lcnphytbl_info_t));
				if (!pi_lcn->gain_table_rx_2g) {
					PHY_ERROR(("wl%d: %s: MALLOC failure\n", pi->sh->unit,
					__FUNCTION__));
					return FALSE;
				}
				lcnphytbl = (dot11lcnphytbl_info_t *) pi_lcn->gain_table_rx_2g;

				for (idx = 0; idx < pi_lcn->num_gain_table_rx_elem_2g; idx++) {
				if (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) &
					BFL_EXTLNA) {
					if (pi_lcn->rxgaintbl_elna_100) {
						lcnphytbl[idx] =
						dot11lcnphytbl_2G_ext_lna_100_rx_gain_info_rev2
						[idx];
					} else if (pi->sh->chippkg ==
						BCM4330_WLBGA_PKG_ID) {
						if (pi_lcn->rxgaintbl_wlbga_elna_aci) {
						lcnphytbl[idx] =
						dot11lcnphytbl_2G_ext_lna_wlbga_1_rx_gain_info_rev2
						[idx];
						} else {
						lcnphytbl[idx] =
						dot11lcnphytbl_2G_ext_lna_wlbga_rx_gain_info_rev2
						[idx];
						}
					} else if (pi_lcn->mux_gain_table) {
						lcnphytbl[idx] =
						dot11lcnphytbl_2G_ext_lna_lin_rx_gain_info_rev2
						[idx];
					} else {
						lcnphytbl[idx] =
						dot11lcnphytbl_2G_ext_lna_rx_gain_info_rev2[idx];
					}
				} else {
					if (pi->sh->chippkg == BCM4330_WLBGA_PKG_ID) {
						if (pi_lcn->rxgaintbl_wlbga_aci == 2)
							lcnphytbl[idx] =
							dot11lcnphytbl_2G_wlbga_2_rx_gain_info_rev2
							[idx];
						else if (pi_lcn->rxgaintbl_wlbga_aci == 1)
							lcnphytbl[idx] =
							dot11lcnphytbl_2G_wlcsp_rx_gain_info_rev2
							[idx];
						else
							lcnphytbl[idx] =
							dot11lcnphytbl_2G_wlbga_rx_gain_info_rev2
							[idx];
					} else {
						if (pi_lcn->rxgaintbl_fcbga)
							lcnphytbl[idx] =
							dot11lcnphytbl_2G_fcbga_rx_gain_info_rev2
							[idx];
						else if (pi_lcn->rxgaintbl_wlcsp)
							lcnphytbl[idx] =
							dot11lcnphytbl_2G_wlcsp_1_rx_gain_info_rev2
							[idx];
						else if  (pi_lcn->rxgaintbl_wlcsp == 2)
							lcnphytbl[idx] =
							dot11lcnphytbl_2G_wlcsp_2_rx_gain_info_rev2
							[idx];
						else
							lcnphytbl[idx] =
							dot11lcnphytbl_2G_wlcsp_rx_gain_info_rev2
							[idx];
					}
				}
				}
				/* Preserve the tables */
				if (!wlc_lcnphy_rx_gain_tbl_copy(pi, TRUE))
					return FALSE;
			}
		}
	} else {
		if (LCNREV_IS(pi->pubpi.phy_rev, 1)) {
			if (CHIPID(pi->sh->chip) == BCM4313_CHIP_ID) {
				if (!(BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_FEM_BT) &&
					EPA(pi_lcn) &&
					(BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) &
					BFL_EXTLNA)) {
					pi_lcn->num_gain_table_rx_elem_2g =
						dot11lcnphytbl_2G_ext_lna_rx_gain_info_sz_rev1;
					pi_lcn->gain_table_rx_2g = MALLOC(pi->sh->osh,
						pi_lcn->num_gain_table_rx_elem_2g *
						sizeof(dot11lcnphytbl_info_t));
					if (!pi_lcn->gain_table_rx_2g) {
						PHY_ERROR(("wl%d: %s: MALLOC failure\n",
						pi->sh->unit, __FUNCTION__));
						return FALSE;
					}
					lcnphytbl =
						(dot11lcnphytbl_info_t *) pi_lcn->gain_table_rx_2g;

					for (idx = 0; idx <	pi_lcn->num_gain_table_rx_elem_2g;
						idx++) {
						lcnphytbl[idx] =
						dot11lcnphytbl_2G_ext_lna_rx_gain_info_rev1[idx];
					}
				}
			}
		}

		/* Preserve the tables */
		if (!wlc_lcnphy_rx_gain_tbl_copy(pi, TRUE))
			return FALSE;
	}

#ifdef BAND5G
	pi_lcn->num_gain_table_rx_elem_5g = dot11lcnphytbl_5G_rx_gain_info_sz_rev2;
	pi_lcn->gain_table_rx_5g = MALLOC(pi->sh->osh,
		pi_lcn->num_gain_table_rx_elem_5g * sizeof(dot11lcnphytbl_info_t));
	if (!pi_lcn->gain_table_rx_5g) {
		PHY_ERROR(("wl%d: %s: MALLOC failure\n", pi->sh->unit, __FUNCTION__));
		return FALSE;
	}
	lcnphytbl = (dot11lcnphytbl_info_t *) pi_lcn->gain_table_rx_5g;

	for (idx = 0; idx < pi_lcn->num_gain_table_rx_elem_5g; idx++) {
		if (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) &
			BFL_EXTLNA_5GHz) {
			lcnphytbl[idx] = dot11lcnphytbl_5G_ext_lna_rx_gain_info_rev2[idx];
		}
		else {
			lcnphytbl[idx] = dot11lcnphytbl_5G_rx_gain_info_rev2[idx];
		}
	}

	/* Preserve the tables */
	if (!wlc_lcnphy_rx_gain_tbl_copy(pi, FALSE))
		return FALSE;
#endif /* BAND5G */

	return TRUE;
}

static bool
BCMATTACHFN(wlc_lcnphy_rx_gain_tbl_copy)(phy_info_t *pi, bool tbl2g)
{
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	void *tbl_ptr = NULL;
	uint8 idx;
#ifdef BAND5G
	dot11lcnphytbl_info_t *lcnphy_tbl_ptr = (tbl2g) ? pi_lcn->gain_table_rx_2g :
		pi_lcn->gain_table_rx_5g;
	uint32 numtbl = (tbl2g) ? pi_lcn->num_gain_table_rx_elem_2g :
		pi_lcn->num_gain_table_rx_elem_5g;
#else
	dot11lcnphytbl_info_t *lcnphy_tbl_ptr = pi_lcn->gain_table_rx_2g;
	uint32 numtbl = pi_lcn->num_gain_table_rx_elem_2g;
#endif // endif

	for (idx = 0; idx < numtbl; idx++) {
		uint32 num_bytes = lcnphy_tbl_ptr[idx].tbl_len *
			lcnphy_tbl_ptr[idx].tbl_width / 8;
		tbl_ptr = (void *) lcnphy_tbl_ptr[idx].tbl_ptr;
		lcnphy_tbl_ptr[idx].tbl_ptr = MALLOC(pi->sh->osh, num_bytes);
		if (!lcnphy_tbl_ptr[idx].tbl_ptr) {
			PHY_ERROR(("wl%d: %s: MALLOC failure\n", pi->sh->unit, __FUNCTION__));
			return FALSE;
		}
		memcpy((void *) lcnphy_tbl_ptr[idx].tbl_ptr, tbl_ptr, num_bytes);
	}

	return TRUE;
}

static void
wlc_lcnphy_rx_gain_tbl_free(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;

	if (pi_lcn->gain_table_rx_2g) {
		/* free up the individual entries first */
		wlc_lcnphy_rx_gain_tbl_free_entry(pi, TRUE);

		/* free up the info structure now */
		MFREE(pi->sh->osh, pi_lcn->gain_table_rx_2g,
			pi_lcn->num_gain_table_rx_elem_2g *	sizeof(dot11lcnphytbl_info_t));
	}
#ifdef BAND5G
	if (pi_lcn->gain_table_rx_5g) {
		/* free up the individual entries first */
		wlc_lcnphy_rx_gain_tbl_free_entry(pi, FALSE);

		/* free up the info structure now */
		MFREE(pi->sh->osh, pi_lcn->gain_table_rx_5g,
			pi_lcn->num_gain_table_rx_elem_5g *	sizeof(dot11lcnphytbl_info_t));
	}
#endif // endif
}

static void
wlc_lcnphy_rx_gain_tbl_free_entry(phy_info_t *pi, bool tbl2g)
{
	uint8 idx;
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
#ifdef BAND5G
	dot11lcnphytbl_info_t *lcnphy_tbl_ptr = (tbl2g) ? pi_lcn->gain_table_rx_2g :
		pi_lcn->gain_table_rx_5g;
	uint32 numtbl = (tbl2g) ? pi_lcn->num_gain_table_rx_elem_2g :
		pi_lcn->num_gain_table_rx_elem_5g;
#else
	dot11lcnphytbl_info_t *lcnphy_tbl_ptr = pi_lcn->gain_table_rx_2g;
	uint32 numtbl = pi_lcn->num_gain_table_rx_elem_2g;
#endif // endif

	for (idx = 0; idx < numtbl; idx++) {
		uint32 num_bytes = lcnphy_tbl_ptr[idx].tbl_len *
			lcnphy_tbl_ptr[idx].tbl_width / 8;
		MFREE(pi->sh->osh, (void *)lcnphy_tbl_ptr[idx].tbl_ptr, num_bytes);
	}
}

static void
wlc_lcnphy_tx_gain_tbl_init(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		ASSERT(pi_lcn->gain_table_tx_2g);
		wlc_lcnphy_load_tx_gain_table(pi,
			(lcnphy_tx_gain_tbl_entry *) pi_lcn->gain_table_tx_2g);
	}
#ifdef BAND5G
	else {
		ASSERT(pi_lcn->gain_table_tx_5g);
		wlc_lcnphy_load_tx_gain_table(pi,
			(lcnphy_tx_gain_tbl_entry *) pi_lcn->gain_table_tx_5g);
	}
#endif /* BAND5G */
}

static bool
BCMATTACHFN(wlc_lcnphy_tx_gain_tbl_select)(phy_info_t *pi)
{
	lcnphy_tx_gain_tbl_entry *gain_table_ptr = NULL;
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	pi_lcn->gain_table_tx_2g = NULL;
#ifdef BAND5G
	pi_lcn->gain_table_tx_5g = NULL;
#endif // endif

	/* ePA board needs to use extPA gain table */
	if (((BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_FEM) &&
		(CHIPID(pi->sh->chip) == BCM4313_CHIP_ID))) {
		gain_table_ptr = (lcnphy_tx_gain_tbl_entry *)
			&dot11lcnphy_2GHz_extPA_4313_gaintable_rev0;
	}
	else if (pi_lcn->extpagain2g == 2) {
		gain_table_ptr = (lcnphy_tx_gain_tbl_entry *)
			&dot11lcnphy_2GHz_extPA_gaintable_rev0;
	}
	else if (TXGAINTBL(pi_lcn) != 1) {
		gain_table_ptr = (lcnphy_tx_gain_tbl_entry *)
			&dot11lcnphy_2GHz_gaintable_rev0;
	}
	else {
		gain_table_ptr = (lcnphy_tx_gain_tbl_entry *)
			&dot11lcnphy_2GHz_gaintable_1_rev0;
	}

	/* preserve the 2g gain table for later use */
	pi_lcn->gain_table_tx_2g = MALLOC(pi->sh->osh, sizeof(lcnphy_tx_gain_tbl_entry) *
		DOT11LCNPHY_TX_GAIN_TABLE_SZ);
	if (!pi_lcn->gain_table_tx_2g) {
		PHY_ERROR(("wl%d: %s: MALLOC failure\n", pi->sh->unit, __FUNCTION__));
		return FALSE;
	}
	wlc_lcnphy_tx_gain_tbl_copy((lcnphy_tx_gain_tbl_entry *) pi_lcn->gain_table_tx_2g,
		gain_table_ptr, DOT11LCNPHY_TX_GAIN_TABLE_SZ);

	if (pi_lcn->txgaintbl == 2) {
		gain_table_ptr = (lcnphy_tx_gain_tbl_entry *)
			&dot11lcnphy_2GHz_gaintable_2_rev0;
		wlc_lcnphy_tx_gain_tbl_copy(((lcnphy_tx_gain_tbl_entry *)
			(pi_lcn->gain_table_tx_2g)) + 32, gain_table_ptr, 21);
	}

#ifdef BAND5G
	if (pi_lcn->extpagain5g != 2) {
		gain_table_ptr = (lcnphy_tx_gain_tbl_entry *)
			&dot11lcnphy_5GHz_gaintable_rev0;
	}
	else if (TXGAINTBL5G(pi_lcn)) {
		gain_table_ptr = (lcnphy_tx_gain_tbl_entry *)
			&dot11lcnphy_5GHz_extPA_gaintable_1_rev0;
	}
	else {
		gain_table_ptr = (lcnphy_tx_gain_tbl_entry *)
			&dot11lcnphy_5GHz_extPA_gaintable_rev0;
	}

	/* preserve the 5g gain table for later use */
	pi_lcn->gain_table_tx_5g = MALLOC(pi->sh->osh, sizeof(lcnphy_tx_gain_tbl_entry) *
		DOT11LCNPHY_TX_GAIN_TABLE_SZ);
	if (!pi_lcn->gain_table_tx_5g) {
		PHY_ERROR(("wl%d: %s: MALLOC failure\n", pi->sh->unit, __FUNCTION__));
		return FALSE;
	}
	wlc_lcnphy_tx_gain_tbl_copy((lcnphy_tx_gain_tbl_entry *) pi_lcn->gain_table_tx_5g,
		gain_table_ptr, DOT11LCNPHY_TX_GAIN_TABLE_SZ);
#endif /* BAND5G */

	return TRUE;
}

static void
BCMATTACHFN(wlc_lcnphy_tx_gain_tbl_copy)(lcnphy_tx_gain_tbl_entry *gain_table_dst,
	const lcnphy_tx_gain_tbl_entry * gain_table, uint32 num_elem)
{
	uint j;
	for (j = 0; j < num_elem; j++) {
		gain_table_dst[j] = gain_table[j];
	}
}

static void
wlc_lcnphy_tx_gain_tbl_free(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;

	if (pi_lcn->gain_table_tx_2g)
		MFREE(pi->sh->osh, pi_lcn->gain_table_tx_2g, sizeof(lcnphy_tx_gain_tbl_entry) *
			DOT11LCNPHY_TX_GAIN_TABLE_SZ);
#ifdef BAND5G
	if (pi_lcn->gain_table_tx_5g)
		MFREE(pi->sh->osh, pi_lcn->gain_table_tx_5g, sizeof(lcnphy_tx_gain_tbl_entry) *
			DOT11LCNPHY_TX_GAIN_TABLE_SZ);
#endif // endif
}

static void
WLBANDINITFN(wlc_lcnphy_tbl_init)(phy_info_t *pi)
{
	uint idx;
	phytbl_info_t tab;
	uint16 val;
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	if ((CHIPID(pi->sh->chip) == BCM4313_CHIP_ID) || !pi_lcn->startup_init) {
		for (idx = 0; idx < dot11lcnphytbl_info_sz_rev0; idx++) {
			wlc_lcnphy_write_table(pi, &dot11lcnphytbl_info_rev0[idx]);
		}
		/* Replace the default nf table for rev 2 */
		if (LCNREV_GE(pi->pubpi.phy_rev, 2)) {
			tab.tbl_len = dot11lcnphytbl_nf_table_info_rev2[0].tbl_len;
			tab.tbl_id = LCNPHY_TBL_ID_NF_CTRL;
			tab.tbl_offset = 0;
			tab.tbl_width = 8;
			tab.tbl_ptr = dot11lcnphytbl_nf_table_info_rev2[0].tbl_ptr;
			wlc_lcnphy_write_table(pi, &tab);
		}

		/* Replace default gain table with 4336 specific gain table */
		if ((CHIPID(pi->sh->chip) == BCM4336_CHIP_ID ||
		     CHIPID(pi->sh->chip) == BCM43362_CHIP_ID) &&
			(pi->sh->chippkg == BCM4336_WLBGA_PKG_ID)) {
			/* Back off lna2 gain */
			tab.tbl_len = dot11lcnphytbl_4336wlbga_rx_gain_info_rev0[0].tbl_len;
			tab.tbl_id = LCNPHY_TBL_ID_GAIN_TBL;
			tab.tbl_offset = 0;
			tab.tbl_width = 32;
			tab.tbl_ptr = dot11lcnphytbl_4336wlbga_rx_gain_info_rev0[0].tbl_ptr;
			wlc_lcnphy_write_table(pi, &tab);
		}
	}

	/* delaying the switching on of ePA after Tx start which */
	/* is done by switch control for BT boards */
	if ((CHIPID(pi->sh->chip) == BCM4313_CHIP_ID) && EPA(pi_lcn) &&
		(BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_FEM_BT)) {
		tab.tbl_id = LCNPHY_TBL_ID_RFSEQ;
		tab.tbl_width = 16;	/* 12 bit wide	*/
		tab.tbl_ptr = &val;
		tab.tbl_len = 1;
		val = 200;
		tab.tbl_offset = 4;
		wlc_lcnphy_write_table(pi,  &tab);
	}

	/* tx_ofdm_delay and tx_cck_delay have been increased for 4313 iPA boards */
	/* is done to remove the precursor which is hitting the throughput in high SNR regions */
	if ((CHIPID(pi->sh->chip) == BCM4313_CHIP_ID) && !EPA(pi_lcn)) {
		tab.tbl_id = LCNPHY_TBL_ID_RFSEQ;
		tab.tbl_width = 16;	/* 12 bit wide	*/
		tab.tbl_ptr = &val;
		tab.tbl_len = 1;

		val = 150;
		tab.tbl_offset = 0;
		wlc_lcnphy_write_table(pi,  &tab);

		val = 220;
		tab.tbl_offset = 1;
		wlc_lcnphy_write_table(pi,  &tab);
	}
	/* time domain spikes at beginning of packets related to   */
	/* shared lpf switching between tx and rx observed in 4336 */
	/* need to be looked at for 4313, until then making it 4336 specific */

	if (CHIPID(pi->sh->chip) == BCM4336_CHIP_ID ||
	    CHIPID(pi->sh->chip) == BCM43362_CHIP_ID ||
	    CHIPID(pi->sh->chip) == BCM4330_CHIP_ID) {
		/* RF seq */
		tab.tbl_id = LCNPHY_TBL_ID_RFSEQ;
		tab.tbl_width = 16;	/* 12 bit wide	*/
		tab.tbl_ptr = &val;
		tab.tbl_len = 1;

		val = 114;
		tab.tbl_offset = 0;
		wlc_lcnphy_write_table(pi,  &tab);

		val = 130;
		tab.tbl_offset = 1;
		wlc_lcnphy_write_table(pi,  &tab);

		val = 6;
		tab.tbl_offset = 8;
		wlc_lcnphy_write_table(pi,  &tab);
	}

	/* Load tx gain tables */
	wlc_lcnphy_tx_gain_tbl_init(pi);

	/* Load rx gain tables */
	wlc_lcnphy_rx_gain_tbl_init(pi);

	/* Change switch table if necessary based on chip and board type */
	wlc_lcnphy_sw_ctrl_tbl_init(pi);

	wlc_lcnphy_load_rfpower(pi);

	/* clear our PAPD Compensation table */
	wlc_lcnphy_clear_papd_comptable(pi);

}

void
wlc_phy_lcn_updatemac_rssi(phy_info_t *pi, int8 rssi, int8 antenna)
{
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	int32 rssi_z;
#if defined(WLTEST)
	uint16 x;
	uint16 n[2];
#endif // endif

	if (CHIPID(pi->sh->chip) == BCM4313_CHIP_ID) {
		if (rssi > 10)
			pi_lcn->Rssi = rssi;
	}

	/* window avg of 4 last rssi values / antenna */
	rssi_z = (pi_lcn->rssi_dly >> 24) & 0xff;
	pi_lcn->rssi_dly = ((pi_lcn->rssi_dly << 8) & 0xffffff00) | ((uint32)rssi & 0xff);
	if (rssi_z > 127)
		rssi_z -= 256;
	pi_lcn->rssi_acc += ((int32)rssi - rssi_z);
	pi_lcn->rssi_avg = pi_lcn->rssi_acc >> 2;

#if defined(WLTEST)
	/* Re-use these phy regs which are unused */
	PHY_REG_WRITE(pi, LCNPHY, sslpnphy_gpio_test_reg_17_2, (uint16)(-pi_lcn->rssi_avg));
	x = phy_utils_read_phyreg(pi, LCNPHY_sslpnphy_gpio_test_reg_33_18);
	n[1] = (x >> 8) & 0xff;
	n[0] = (x & 0xff);
	n[antenna & 1]++;
	x = ((n[1] & 0xff) << 8) | (n[0] & 0xff);
	PHY_REG_WRITE(pi, LCNPHY, sslpnphy_gpio_test_reg_33_18, x);
#endif // endif
}

/* mapped to lcnphy_rev0_reg_init */
static void
WLBANDINITFN(wlc_lcnphy_rev0_baseband_init)(phy_info_t *pi)
{
	uint16 afectrl1;
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	int16 bphy_scale;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	/* Program for 160MHz digital filter, if required */
	if (pi_lcn->dacrate == 160) {
		PHY_REG_LIST_START_WLBANDINITDATA
			/* Enable DAC Clk edge */
			PHY_REG_OR_ENTRY(LCNPHY, sslpnCtrl4,
				LCNPHY_sslpnCtrl4_flip_dacclk_edge_MASK)
			/* Enable DAC160 mode and CCK Rx Clk */
			PHY_REG_OR_ENTRY(LCNPHY, Core1TxControl,
				(LCNPHY_Core1TxControl_DAC160ModeEn_MASK |
				LCNPHY_Core1TxControl_bphycckRxclk_div2_MASK))
		PHY_REG_LIST_EXECUTE(pi);
	}

	PHY_REG_LIST_START_WLBANDINITDATA
		/* disable internal VCO LDO PU override */
		RADIO_REG_WRITE_ENTRY(RADIO_2064_REG11C, 0x0)

		/* Enable DAC/ADC and disable rf overrides */
		PHY_REG_WRITE_ENTRY(LCNPHY, AfeCtrlOvr, 0x0)
		PHY_REG_WRITE_ENTRY(LCNPHY, AfeCtrlOvrVal, 0x0)
		PHY_REG_WRITE_ENTRY(LCNPHY, RFOverride0, 0x0)
		PHY_REG_WRITE_ENTRY(LCNPHY, RFinputOverrideVal, 0x0)
		PHY_REG_WRITE_ENTRY(LCNPHY, rfoverride3, 0x0)
		PHY_REG_WRITE_ENTRY(LCNPHY, rfoverride2, 0x0)
		PHY_REG_WRITE_ENTRY(LCNPHY, rfoverride4, 0x0)
		PHY_REG_WRITE_ENTRY(LCNPHY, rfoverride2, 0x0)
		PHY_REG_WRITE_ENTRY(LCNPHY, swctrlOvr, 0)
		/* enabling nominal state timeout */
		PHY_REG_OR_ENTRY(LCNPHY, nomStTimeOutParams, 0x03)
		/* Reset radio ctrl and crs gain */
		PHY_REG_OR_ENTRY(LCNPHY, resetCtrl, 0x44)
		PHY_REG_WRITE_ENTRY(LCNPHY, resetCtrl, 0x80)
	PHY_REG_LIST_EXECUTE(pi);

	/* tx gain init for 4313A0 boards without the ext PA */
	if (!(BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_FEM) &&
		(CHIPID(pi->sh->chip) == BCM4313_CHIP_ID))
		wlc_lcnphy_set_tx_pwr_by_index(pi, 52);
	/* initialize rssi gains and vmids */
	/* rssismf2g is 3:0, rssismc2g is 7:4 and rssisav2g is 12:10 of 0x43e, * AfeRSSICtrl1 */
	/* not enabled in tcl */
	if (0) {
		afectrl1 = 0;
		afectrl1 = (uint16)((pi_lcn->lcnphy_rssi_vf) |
		                    (pi_lcn->lcnphy_rssi_vc << 4) | (pi_lcn->lcnphy_rssi_gs << 10));
		PHY_REG_WRITE(pi, LCNPHY, AfeRSSICtrl1, afectrl1);
	}
	/* bphy scale fine tuning to make cck dac ac pwr same as ofdm dac */
	if (pi->bphy_scale == 0)
		bphy_scale = 0xc;
	else
		bphy_scale = pi->bphy_scale;
	PHY_REG_MOD(pi, LCNPHY, BphyControl3, bphyScale, bphy_scale);
	if ((BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_FEM) &&
		(CHIPID(pi->sh->chip) == BCM4313_CHIP_ID)) {
		PHY_REG_LIST_START_WLBANDINITDATA
			PHY_REG_MOD_ENTRY(LCNPHY, BphyControl3, bphyScale, 0xA)
			/* load iir filter type 1 for cck */
			/* middle stage biquad shift for cck */
			PHY_REG_WRITE_ENTRY(LCNPHY, ccktxfilt20Stg1Shft, 0x1)
		PHY_REG_LIST_EXECUTE(pi);
	}

	PHY_REG_LIST_START_WLBANDINITDATA
		/* bphy filter selection , for channel 14 it is 2 */
		PHY_REG_MOD_ENTRY(LCNPHY, lpphyCtrl, txfiltSelect, 1)
		PHY_REG_MOD_ENTRY(LCNPHY, TxMacIfHoldOff, holdoffval, 0x17)
		PHY_REG_MOD_ENTRY(LCNPHY, TxMacDelay, macdelay, 0x3EA)
	PHY_REG_LIST_EXECUTE(pi);

	/* adc swap and crs stuck state is moved to bu_tweaks */
	/* Program ELNA related settings for 4313 */
	if (CHIPID(pi->sh->chip) == BCM4313_CHIP_ID) {
		uint32 ePA = BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_FEM;
		if (!(BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_FEM_BT)) {
		/* non combo boards */
			if (ePA) {
				if (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA) {
					PHY_REG_LIST_START_WLBANDINITDATA
						/* Enable EXT LNA */
						PHY_REG_MOD_ENTRY(LCNPHY, radioCtrl, extlnaen, 1)
						PHY_REG_MOD_ENTRY(LCNPHY, extlnagainvalue0,
							extlnagain0, 0x7d)
						PHY_REG_MOD_ENTRY(LCNPHY, extlnagainvalue0,
							extlnagain1, 0xe)
					PHY_REG_LIST_EXECUTE(pi);
				}
			}
		}
	}
}

/* mapped to lcnphy_rev2_reg_init */
static void
WLBANDINITFN(wlc_lcnphy_rev2_baseband_init)(phy_info_t *pi)
{
#ifdef BAND5G
	if (CHSPEC_IS5G(pi->radio_chanspec)) {
		PHY_REG_LIST_START_WLBANDINITDATA
			PHY_REG_MOD_ENTRY(LCNPHY, MinPwrLevel, ofdmMinPwrLevel, 80)
			PHY_REG_MOD_ENTRY(LCNPHY, MinPwrLevel, dsssMinPwrLevel, 80)
		PHY_REG_LIST_EXECUTE(pi);
	}
#endif // endif
}

static void
WLBANDINITFN(wlc_lcnphy_agc_temp_init)(phy_info_t *pi)
{
	int16 temp;
	phytbl_info_t tab;
	uint32 tableBuffer[2];
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	if (NORADIO_ENAB(pi->pubpi))
		return;
	/* reference ofdm gain index table offset */
	temp = (int16) phy_utils_read_phyreg(pi, LCNPHY_gainidxoffset);
	pi_lcn->lcnphy_ofdmgainidxtableoffset =
		(temp & LCNPHY_gainidxoffset_ofdmgainidxtableoffset_MASK) >>
		LCNPHY_gainidxoffset_ofdmgainidxtableoffset_SHIFT;

	if (pi_lcn->lcnphy_ofdmgainidxtableoffset > 127)
		pi_lcn->lcnphy_ofdmgainidxtableoffset -= 256;

	/* reference dsss gain index table offset */
	pi_lcn->lcnphy_dsssgainidxtableoffset =
		(temp & LCNPHY_gainidxoffset_dsssgainidxtableoffset_MASK) >>
		LCNPHY_gainidxoffset_dsssgainidxtableoffset_SHIFT;

	if (pi_lcn->lcnphy_dsssgainidxtableoffset > 127)
		pi_lcn->lcnphy_dsssgainidxtableoffset -= 256;

	tab.tbl_ptr = tableBuffer;	/* ptr to buf */
	tab.tbl_len = 2;			/* # values   */
	tab.tbl_id = 17;			/* gain_val_tbl_rev3 */
	tab.tbl_offset = 59;		/* tbl offset */
	tab.tbl_width = 32;			/* 32 bit wide */
	wlc_lcnphy_read_table(pi, &tab);

	/* reference value of gain_val_tbl at index 59 */
	if (tableBuffer[0] > 63)
		tableBuffer[0] -= 128;
	pi_lcn->lcnphy_tr_R_gain_val = tableBuffer[0];

	/* reference value of gain_val_tbl at index 60 */
	if (tableBuffer[1] > 63)
		tableBuffer[1] -= 128;
	pi_lcn->lcnphy_tr_T_gain_val = tableBuffer[1];

	temp = (int16)(phy_utils_read_phyreg(pi, LCNPHY_InputPowerDB)
			& LCNPHY_InputPowerDB_inputpwroffsetdb_MASK);
	if (temp > 127)
		temp -= 256;
	pi_lcn->lcnphy_input_pwr_offset_db = (int8)temp;

	pi_lcn->lcnphy_Med_Low_Gain_db = (phy_utils_read_phyreg(pi, LCNPHY_LowGainDB)
					& LCNPHY_LowGainDB_MedLowGainDB_MASK)
					>> LCNPHY_LowGainDB_MedLowGainDB_SHIFT;
	pi_lcn->lcnphy_Very_Low_Gain_db = (phy_utils_read_phyreg(pi, LCNPHY_VeryLowGainDB)
					& LCNPHY_VeryLowGainDB_veryLowGainDB_MASK)
					>> LCNPHY_VeryLowGainDB_veryLowGainDB_SHIFT;

	tab.tbl_ptr = tableBuffer;	/* ptr to buf */
	tab.tbl_len = 2;			/* # values   */
	tab.tbl_id = LCNPHY_TBL_ID_GAIN_IDX;			/* gain_val_tbl_rev3 */
	tab.tbl_offset = 28;		/* tbl offset */
	tab.tbl_width = 32;			/* 32 bit wide */
	wlc_lcnphy_read_table(pi, &tab);

	pi_lcn->lcnphy_gain_idx_14_lowword = tableBuffer[0];
	pi_lcn->lcnphy_gain_idx_14_hiword = tableBuffer[1];
	/* Added To Increase The 1Mbps Sense for Temps @Around */
	/* -15C Temp With CmRxAciGainTbl */
}

static void
wlc_lcnphy_agc_reset(phy_info_t *pi)
{
	uint16 reset_ctrl;

	reset_ctrl = phy_utils_read_phyreg(pi, LCNPHY_resetCtrl);
	reset_ctrl |= (LCNPHY_resetCtrl_radioctrlSoftReset_MASK |
		LCNPHY_resetCtrl_rxfrontendSoftReset_MASK);
	phy_utils_write_phyreg(pi, LCNPHY_resetCtrl, reset_ctrl);
	reset_ctrl &= ~LCNPHY_resetCtrl_radioctrlSoftReset_MASK;
	phy_utils_write_phyreg(pi, LCNPHY_resetCtrl, reset_ctrl);
	OSL_DELAY(4);
	reset_ctrl &= ~LCNPHY_resetCtrl_rxfrontendSoftReset_MASK;
	phy_utils_write_phyreg(pi, LCNPHY_resetCtrl, reset_ctrl);
}

#define k_max_tr_idx 16

static void
wlc_lcnphy_set_tr_attn(phy_info_t *pi, bool override_en, uint8 override_val)
{
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	phytbl_info_t tab;
	uint8 wlRxGain, wlRxGainAttn, tr_attn_idx;
	uint8 trAttnDflt = 0, trAttn = 0xff;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	if (NORADIO_ENAB(pi->pubpi))
		return;

	wlapi_bmac_mctrl(pi->sh->physhim, MCTL_TBTT_HOLD,  MCTL_TBTT_HOLD);
	OSL_DELAY(5);

	PHY_REG_MOD(pi, LCNPHY, lpphyCtrl, resetCCA, 0);

	PHY_REG_MOD(pi, LCNPHY, radioTRCtrl, gainrequestTRAttnOnEn, 1);

	/* Account for difference between wl_rx and wl_rx_attn in swctrlmap_{2,5}G */
	tab.tbl_ptr = &wlRxGain;	/* ptr to buf */
	tab.tbl_len = 1;		/* # values   */
	tab.tbl_id = LCNPHY_TBL_ID_GAIN_VAL_TBL; /* gain_val_tbl */
	if (LCNREV_GE(pi->pubpi.phy_rev, 2))
		tab.tbl_offset = 15;	/* tbl offset */
	else
		tab.tbl_offset = 56;	/* tbl offset */
	tab.tbl_width = 8;		/* 8 bit wide */
	wlc_lcnphy_read_table(pi, &tab);

	if (override_en) {
		trAttnDflt = override_val;
	} else {
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			trAttn = pi_lcn->triso2g;
			if (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA)
				trAttnDflt = 6;
			else
				trAttnDflt = 8;
		}
#ifdef BAND5G
		else {
		  uint8 channel = CHSPEC_CHANNEL(pi->radio_chanspec);
		  if  (channel >= 149)
		    trAttn = pi_lcn->triso5g[2];
		  else if (channel >= 100)
		    trAttn = pi_lcn->triso5g[1];
		  else
		    trAttn = pi_lcn->triso5g[0];

			if (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA_5GHz)
				trAttnDflt = 6;
			else
				trAttnDflt = 8;
		}
#endif /* #ifdef BAND5G */
	}
	if (trAttn == 0xff)
		trAttn = trAttnDflt;
	wlRxGainAttn = wlRxGain - (3*trAttn);

	tab.tbl_ptr = &wlRxGainAttn;
	if (LCNREV_GE(pi->pubpi.phy_rev, 2))
		tab.tbl_offset = 16;
	else
		tab.tbl_offset = 57;
	wlc_lcnphy_write_table(pi, &tab);

	PHY_REG_MOD(pi, LCNPHY, radioTRCtrl, gainrequestTRAttnOnOffset, trAttn);

	tr_attn_idx = pi_lcn->tridx2g;
#ifdef BAND5G
	if (CHSPEC_IS5G(pi->radio_chanspec)) {
		tr_attn_idx = pi_lcn->tridx5g;
	}
#endif // endif
	if (tr_attn_idx != 0xff) {
		if (tr_attn_idx > 0) {
			uint32 tbl_data[2*(k_max_tr_idx+1)];
			uint16 tbl_data_16[k_max_tr_idx+1];
			uint32 tr_mask;
			uint offset = 0;
			int8 tr_attn_offset, tbl_len, tbl_offset;
			uint8 i;

			if (tr_attn_idx > k_max_tr_idx)
			tr_attn_idx = k_max_tr_idx;
			tr_attn_offset = 37 - tr_attn_idx  - 1; /* max value given table size */
			if ((int8)trAttn <= tr_attn_offset)
			tr_attn_offset = (int8)trAttn;
			if (tr_attn_offset == 0)
			tr_attn_offset = 1;
			if (tr_attn_offset <= tr_attn_idx)
			tbl_len = tr_attn_offset;
			else
			tbl_len = tr_attn_idx + 1;
			tbl_offset = tr_attn_idx + 1 - tbl_len;
			if (LCNREV_GE(pi->pubpi.phy_rev, 2))
				tr_mask = (1<<7);
			else
				tr_mask = (1<<6);
			/* Modify gain idx table */
			tab.tbl_id = LCNPHY_TBL_ID_GAIN_IDX;
			tab.tbl_width = 32;
			while (offset < 74) {
				tab.tbl_ptr = &tbl_data[2*tbl_offset];
				tab.tbl_len = 2*tbl_len;
				if (tr_attn_offset <= tr_attn_idx)
					tab.tbl_offset = tr_attn_idx + 1;
				else
					tab.tbl_offset = tr_attn_offset;
				tab.tbl_offset += offset;
				tab.tbl_offset *= 2;
				wlc_lcnphy_read_table(pi, &tab);
				for (i = (2*tbl_offset+1); i <= (2*tr_attn_idx + 1); i += 2) {
					tbl_data[i] |= tr_mask;
				}
				for (i = 0; i < (2*tbl_offset); i += 2) {
				tbl_data[i] = tbl_data[2*tbl_offset];
				tbl_data[i+1] = tbl_data[2*tbl_offset + 1];
				}
				tab.tbl_len = 2*(tr_attn_idx + 1);
				tab.tbl_ptr = tbl_data;
				tab.tbl_offset = 2*offset;
				wlc_lcnphy_write_table(pi, &tab);
				offset += 37;
			}
			/* Modify aux gain idx table */
			tr_mask = (1<<11);
			tab.tbl_id = LCNPHY_TBL_ID_AUX_GAIN_IDX;
			tab.tbl_width = 16;
			tab.tbl_ptr = &tbl_data_16[tbl_offset];
			tab.tbl_len = tbl_len;
			if (tr_attn_offset <= tr_attn_idx)
				tab.tbl_offset = tr_attn_idx + 1;
			else
				tab.tbl_offset = tr_attn_offset;
			wlc_lcnphy_read_table(pi, &tab);
			for (i = tbl_offset; i <= tr_attn_idx; i++) {
				tbl_data_16[i] |= tr_mask;
			}
			for (i = 0; i < tbl_offset; i++) {
				tbl_data_16[i] = tbl_data_16[tbl_offset];
			}
			tab.tbl_len = tr_attn_idx + 1;
			tab.tbl_ptr = tbl_data_16;
			tab.tbl_offset = 0;
			wlc_lcnphy_write_table(pi, &tab);
		}
		PHY_REG_MOD(pi, LCNPHY, radioTRCtrlCrs1, gainReqTrAttOnEnByCrs, 1);
		PHY_REG_MOD(pi, LCNPHY, radioTRCtrlCrs1, trTransAddrLmtBlocker, tr_attn_idx);
		PHY_REG_MOD(pi, LCNPHY, radioTRCtrlCrs2, trTransAddrLmtOfdm, tr_attn_idx);
		PHY_REG_MOD(pi, LCNPHY, radioTRCtrlCrs2, trTransAddrLmtDsss, tr_attn_idx);
	} else {
		PHY_REG_LIST_START
			PHY_REG_MOD_ENTRY(LCNPHY, radioTRCtrlCrs1, gainReqTrAttOnEnByCrs, 0)
			PHY_REG_MOD_ENTRY(LCNPHY, radioTRCtrlCrs1, trTransAddrLmtBlocker, 0)
			PHY_REG_MOD_ENTRY(LCNPHY, radioTRCtrlCrs2, trTransAddrLmtOfdm, 0)
			PHY_REG_MOD_ENTRY(LCNPHY, radioTRCtrlCrs2, trTransAddrLmtDsss, 0)
		PHY_REG_LIST_EXECUTE(pi);
	}

	wlapi_bmac_mctrl(pi->sh->physhim, MCTL_TBTT_HOLD,  0);

	/* Reset radio ctrl and agc after changing table */
	wlc_lcnphy_agc_reset(pi);
}

#if defined(WLTEST)
static int8
wlc_lcnphy_get_rx_gain(phy_info_t *pi, uint8 offset, uint8 num_reading)
{
	phytbl_info_t tab;
	uint8 wlRxGain;
	int8 rxgain;

	tab.tbl_ptr = &wlRxGain;	/* ptr to buf */
	tab.tbl_len = 1;		/* # values   */
	tab.tbl_id = LCNPHY_TBL_ID_GAIN_VAL_TBL; /* gain_val_tbl */
	tab.tbl_width = 8;		/* 8 bit wide */
	tab.tbl_offset = offset + num_reading;	/* tbl offset */
	wlc_lcnphy_read_table(pi, &tab);
	if (wlRxGain >= 64)
		rxgain = wlRxGain - 128;
	else
		rxgain = wlRxGain;
	return rxgain;
}

static int8
wlc_lcnphy_calc_rx_gain(phy_info_t *pi)
{
	uint8 trsw;
	uint8 elna;
	uint8 biq2;
	uint8 biq1;
	uint8 tia;
	uint8 lna2;
	uint8 lna1;

	int8 rxgain = 0;
	int8 tr_gain;
	int8 elna_gain;
	int8 biq2_gain;
	int8 biq1_gain;
	int8 tia_gain;
	int8 lna2_gain;
	int8 lna1_gain;
	uint16 gain0_15, gain16_19;

	gain0_15 = PHY_REG_READ(pi, LCNPHY, rxgainctrl0ovrval, rxgainctrl_ovr_val0);
	gain16_19 = PHY_REG_READ(pi, LCNPHY, rxlnaandgainctrl1ovrval, rxgainctrl_ovr_val1);

	lna1 = (gain0_15 >> 0) & 0x3;
	lna2 = (gain0_15 >> 4) & 0x3;
	tia =  (gain0_15 >> 8) & 0xf;
	biq1 = (gain0_15 >> 12) & 0xf;
	biq2 = gain16_19 & 0xf;

	if (LCNREV_LT(pi->pubpi.phy_rev, 2)) {
		elna = PHY_REG_READ(pi, LCNPHY, rfoverride2val, gmode_ext_lna_gain_ovr_val);
		lna1 = PHY_REG_READ(pi, LCNPHY, rfoverride2val, slna_gain_ctrl_ovr_val);
	} else {
		elna = PHY_REG_READ(pi, LCNPHY, rfoverride2val, ext_lna_gain_ovr_val);
		lna1 = PHY_REG_READ(pi, LCNPHY, rfoverride2val, slna_gain_ctrl_ovr_val_rev_ge2);
	}

	trsw = !PHY_REG_READ(pi, LCNPHY, RFOverrideVal0, trsw_rx_pu_ovr_val);

	PHY_TMP(("wl%d: %s: tr = %d, elna = %d, biq2= %d,"
		"biq1 = %d, tia = %d, lna2 = %d, lna1 = %d\n",
		pi->sh->unit, __FUNCTION__,
		trsw, elna, biq2, biq1, tia, lna2, lna1));

	if (LCNREV_GE(pi->pubpi.phy_rev, 2)) {
		tr_gain = wlc_lcnphy_get_rx_gain(pi, 15, trsw);
		elna_gain = wlc_lcnphy_get_rx_gain(pi, 13, elna);
		biq2_gain = wlc_lcnphy_get_rx_gain(pi, 53, biq2);
		biq1_gain = wlc_lcnphy_get_rx_gain(pi, 37, biq1);
		tia_gain = wlc_lcnphy_get_rx_gain(pi, 21, tia);
		lna2_gain = wlc_lcnphy_get_rx_gain(pi, 17, lna2);
		lna1_gain = wlc_lcnphy_get_rx_gain(pi, 0, lna1);
		rxgain = tr_gain +  elna_gain + biq2_gain +
			biq1_gain + tia_gain + lna2_gain + lna1_gain;

		PHY_TMP(("wl%d: %s: tr = %d, elna = %d, biq2= %d,"
			"biq1 = %d, tia = %d, lna2 = %d, lna1 = %d\n",
			pi->sh->unit, __FUNCTION__,
			tr_gain, elna_gain, biq2_gain, biq1_gain, tia_gain, lna2_gain, lna1_gain));

	}
	return rxgain;
}
#endif  /* #if defined(WLTEST) */

static void
WLBANDINITFN(wlc_lcnphy_bu_tweaks)(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	if (NORADIO_ENAB(pi->pubpi))
		return;

	PHY_REG_LIST_START_WLBANDINITDATA
		/* dmdStExitEn */
		PHY_REG_OR_ENTRY(LCNPHY, pktfsmctrl, 0x1)
		/* DSSS threshold tightening to avoid cross detections */
		PHY_REG_MOD_ENTRY(LCNPHY, DSSSConfirmCnt, DSSSConfirmCntHiGain, 0x5)
	PHY_REG_LIST_EXECUTE(pi);

	if (LCNREV_GE(pi->pubpi.phy_rev, 2))
		PHY_REG_MOD(pi, LCNPHY, SyncPeakCnt, Kthresh, 0x1F);

	PHY_REG_LIST_START_WLBANDINITDATA
		PHY_REG_MOD_ENTRY(LCNPHY, SyncPeakCnt, MaxPeakCntM1, 0x6)
		PHY_REG_WRITE_ENTRY(LCNPHY, dsssPwrThresh0, 0x1e10)
		PHY_REG_WRITE_ENTRY(LCNPHY, dsssPwrThresh1, 0x0640)

		/* cck gain table tweak to shift the gain table to give uptil */
		/* 98 dBm rx fe power */
		PHY_REG_MOD_ENTRY(LCNPHY, gainidxoffset, dsssgainidxtableoffset, -9)
	PHY_REG_LIST_EXECUTE(pi);

	/* lcnphy_agc_reset */
	wlc_lcnphy_agc_reset(pi);

	if (CHIPID(pi->sh->chip) == BCM4313_CHIP_ID) {
		PHY_REG_LIST_START_WLBANDINITDATA
			PHY_REG_MOD_ENTRY(LCNPHY, InputPowerDB, inputpwroffsetdb, 0xFD)
			PHY_REG_MOD_ENTRY(LCNPHY, ClipCtrThresh, ClipCtrThreshHiGain, 16)

			/* xtal swcap is needed for all 4313 rev boards */
			RADIO_REG_MOD_ENTRY(RADIO_2064_REG09B, 0xF0, 0xF0)

			/* fixes for diversity performance enhancement */
			PHY_REG_WRITE_ENTRY(LCNPHY, diversityReg, 0x0902)
			PHY_REG_MOD_ENTRY(LCNPHY, PwrThresh1, LargeGainMismatchThresh, 0x9)
			PHY_REG_MOD_ENTRY(LCNPHY, PwrThresh1, LoPwrMismatchThresh, 0xe)
		PHY_REG_LIST_EXECUTE(pi);

		if (LCNREV_IS(pi->pubpi.phy_rev, 1)) {
			PHY_REG_LIST_START_WLBANDINITDATA
				PHY_REG_MOD_ENTRY(LCNPHY, HiGainDB, HiGainDB, 0x46)
				PHY_REG_MOD_ENTRY(LCNPHY, ofdmPwrThresh0, ofdmPwrThresh0, 1)
				PHY_REG_MOD_ENTRY(LCNPHY, InputPowerDB, inputpwroffsetdb, 0xFC)
				PHY_REG_MOD_ENTRY(LCNPHY, MinPwrLevel, ofdmMinPwrLevel, -92)
				PHY_REG_MOD_ENTRY(LCNPHY, MinPwrLevel, dsssMinPwrLevel, -101)
				PHY_REG_MOD_ENTRY(LCNPHY, SgiprgReg, sgiPrg, 2)
				PHY_REG_MOD_ENTRY(LCNPHY, RFOverrideVal0, ant_selp_ovr_val, 1)
				/* fix for ADC clip issue */
				RADIO_REG_MOD_ENTRY(RADIO_2064_REG0F7, 0x4, 0x4)
				RADIO_REG_MOD_ENTRY(RADIO_2064_REG0F1, 0x3, 0)
				RADIO_REG_MOD_ENTRY(RADIO_2064_REG0F2, 0xF8, 0x90)
				RADIO_REG_MOD_ENTRY(RADIO_2064_REG0F3, 0x3, 0x2)
				RADIO_REG_MOD_ENTRY(RADIO_2064_REG0F3, 0xf0, 0xa0)
				/* enabling override for o_wrf_jtag_afe_tadj_iqadc_ib20u_1 */
				RADIO_REG_MOD_ENTRY(RADIO_2064_REG11F, 0x2, 0x2)
			PHY_REG_LIST_EXECUTE(pi);
			/* to be revisited, mac offsets of txpwrctrl table */
			/* are getting written with junk values */
			wlc_lcnphy_clear_tx_power_offsets(pi);
			/* Radio register tweaks for reducing Phase Noise */
			phy_utils_write_radioreg(pi, RADIO_2064_REG09C, 0x20);
			if (!EPA(pi_lcn)) {
				pi_lcn->cckPwrOffset = -8;
				if (BOARDTYPE(pi->sh->boardtype) == BCM94313HMG_BOARD)
					PHY_REG_MOD(pi, LCNPHY, InputPowerDB,
						inputpwroffsetdb, 0xFD);
			}
		}
	}
	else if (CHIPID(pi->sh->chip) == BCM43362_CHIP_ID) {
		/*
		BCM43362 -- the 2G lna pu port in radio got connected to 5G lna pu port in phy.
		The 5G lna pu port will always be 0 since BCM43362 only supports 5G.
		The lna1_pwrup_lut bit is used to invert polarity of this signal so setting
		this bit to 1 will force 5G  lna1 pu port in phy to 1. Probably better than using
		override since it's less likely anyone is touching this stuff.
		*/
		PHY_REG_LIST_START_WLBANDINITDATA
			PHY_REG_MOD_ENTRY(LCNPHY, lprfsignallut2, lna1_pwrup_lut, 1)
			/* Stuff ported over from 4336 */
			PHY_REG_MOD_ENTRY(LCNPHY, HiGainDB, HiGainDB, 73)
			PHY_REG_MOD_ENTRY(LCNPHY, InputPowerDB, inputpwroffsetdb, -4)
			PHY_REG_MOD_ENTRY(LCNPHY, MinPwrLevel, ofdmMinPwrLevel, -92)
			PHY_REG_MOD_ENTRY(LCNPHY, MinPwrLevel, dsssMinPwrLevel, -97)
			PHY_REG_MOD_ENTRY(LCNPHY, LowGainDB, MedLowGainDB, 42)
			PHY_REG_MOD_ENTRY(LCNPHY, HiGainDB, MedHiGainDB, 45)
			PHY_REG_MOD_ENTRY(LCNPHY, VeryLowGainDB, veryLowGainDB, 15)
			PHY_REG_MOD_ENTRY(LCNPHY, veryLowGainEx, veryLowGain_2DB, 15)
			PHY_REG_MOD_ENTRY(LCNPHY, veryLowGainEx, veryLowGain_3DB, 15)
			PHY_REG_MOD_ENTRY(LCNPHY, ClipCtrThresh, ClipCtrThreshHiGain, 10)
			PHY_REG_MOD_ENTRY(LCNPHY, InputPowerDB, transientfreeThresh, 1)
			PHY_REG_MOD_ENTRY(LCNPHY, gainMismatchMedGainEx,
				medHiGainDirectMismatchOFDMDet, 6)
		PHY_REG_LIST_EXECUTE(pi);
	}
	else if (CHIPID(pi->sh->chip) == BCM4330_CHIP_ID) {

	  PHY_REG_LIST_START_WLBANDINITDATA
	    PHY_REG_MOD_ENTRY(LCNPHY, InputPowerDB, inputpwroffsetdb, 0)
	    PHY_REG_MOD_ENTRY(LCNPHY, MinPwrLevel, ofdmMinPwrLevel, -95)
	    PHY_REG_MOD_ENTRY(LCNPHY, MinPwrLevel, dsssMinPwrLevel, -100)
	    /* BCM4330 non-agc regs */
	    PHY_REG_WRITE_ENTRY(LCNPHY, slna_tbl_sel, 0)
	    PHY_REG_MOD_ENTRY(LCNPHY, nftrAdj, bt_gain_tbl_offset, -10)
	  PHY_REG_LIST_EXECUTE(pi);

	  PHY_REG_MOD(pi, LCNPHY, crsedthresh, edonthreshold, pi_lcn->edonthreshold);
	  PHY_REG_MOD(pi, LCNPHY, crsedthresh, edoffthreshold, pi_lcn->edoffthreshold);
	  /* Disable slna noise scaling */
	  PHY_REG_LIST_START_WLBANDINITDATA
	     PHY_REG_WRITE_ENTRY(LCNPHY, slnanoisetblreg0, 0x4210)
	     PHY_REG_WRITE_ENTRY(LCNPHY, slnanoisetblreg1, 0x4210)
	     PHY_REG_WRITE_ENTRY(LCNPHY, slnanoisetblreg2, 0x0210)
	  PHY_REG_LIST_EXECUTE(pi);

	  if (CHSPEC_IS2G(pi->radio_chanspec)) {
	    PHY_REG_LIST_START_WLBANDINITDATA
	      PHY_REG_WRITE_ENTRY(LCNPHY, nfSubtractVal, 392)
	      PHY_REG_MOD_ENTRY(LCNPHY, ClipCtrThresh, ClipCtrThreshHiGain, 10)
	      PHY_REG_MOD_ENTRY(LCNPHY, gaindirectMismatch, medGainGmShftVal, 2)
	      PHY_REG_MOD_ENTRY(LCNPHY, gainMismatch, GainmisMatchPktRx, 9)
	      PHY_REG_MOD_ENTRY(LCNPHY, ofdmPwrThresh0, ofdmPwrThresh0, 16)
	      PHY_REG_MOD_ENTRY(LCNPHY, ofdmPwrThresh0, ofdmPwrThresh1, 3)
	      PHY_REG_MOD_ENTRY(LCNPHY, crsMiscCtrl0, usePreFiltPwr, 0)
	      PHY_REG_MOD_ENTRY(LCNPHY, RxRadioControlFltrState, rx_flt_high_start_state, 1)
	      PHY_REG_MOD_ENTRY(LCNPHY, RxRadioControlFltrState, rx_flt_high_hold_state, 2)
	      PHY_REG_MOD_ENTRY(LCNPHY, RxRadioControlFltrState, rx_flt_low_start_state, 3)
	    PHY_REG_LIST_EXECUTE(pi);
	    PHY_REG_MOD(pi, LCNPHY, gainBackOffVal, genGainBkOff, pi_lcn->rxgain_backoff_val);

	    if (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA) {
		PHY_REG_LIST_START_WLBANDINITDATA
		  PHY_REG_MOD_ENTRY(LCNPHY, radioCtrl, extlnaen, 1)
		  PHY_REG_MOD_ENTRY(LCNPHY, HiGainDB, HiGainDB, 76)
		  PHY_REG_MOD_ENTRY(LCNPHY, ClipCtrThresh, clipCtrThreshLoGain, 10)
		  PHY_REG_MOD_ENTRY(LCNPHY, gaindirectMismatch, MedHigainDirectMismatch, 15)
		  PHY_REG_MOD_ENTRY(LCNPHY, LowGainDB, MedLowGainDB, 40)
		  PHY_REG_MOD_ENTRY(LCNPHY, HiGainDB, MedHiGainDB, 58)
		  PHY_REG_MOD_ENTRY(LCNPHY, VeryLowGainDB, veryLowGainDB, 22)
		  PHY_REG_MOD_ENTRY(LCNPHY, veryLowGainEx, veryLowGain_2DB, 22)
		  PHY_REG_MOD_ENTRY(LCNPHY, veryLowGainEx, veryLowGain_3DB, 22)
		PHY_REG_LIST_EXECUTE(pi);
		if (pi_lcn->mux_gain_table) {
			PHY_REG_LIST_START_WLBANDINITDATA
			  PHY_REG_WRITE_ENTRY(LCNPHY, nfSubtractVal, 385)
			  PHY_REG_MOD_ENTRY(LCNPHY, HiGainDB, MedHiGainDB, 64)
			  PHY_REG_MOD_ENTRY(LCNPHY, VeryLowGainDB, veryLowGainDB, 19)
			  PHY_REG_MOD_ENTRY(LCNPHY, veryLowGainEx, veryLowGain_2DB, 19)
			  PHY_REG_MOD_ENTRY(LCNPHY, veryLowGainEx, veryLowGain_3DB, 25)
			  PHY_REG_MOD_ENTRY(LCNPHY, gaindirectMismatch, medGainGmShftVal, 1)
			  PHY_REG_MOD_ENTRY(LCNPHY, radioTRCtrlCrs1, trGainThresh, 22)
			  PHY_REG_MOD_ENTRY(LCNPHY, ClipCtrThresh, clipCtrThreshLoGain, 15)
			  PHY_REG_MOD_ENTRY(LCNPHY, ofdmPwrThresh0, ofdmPwrThresh0, 2)
			  PHY_REG_MOD_ENTRY(LCNPHY, ofdmPwrThresh0, ofdmPwrThresh1, 6)
			  PHY_REG_MOD_ENTRY(LCNPHY, ofdmSyncThresh1, ofdmSyncThresh2, 2)
			PHY_REG_LIST_EXECUTE(pi);

		}
	    } else {
	      PHY_REG_LIST_START_WLBANDINITDATA
	        PHY_REG_MOD_ENTRY(LCNPHY, HiGainDB, HiGainDB, 73)
	        PHY_REG_MOD_ENTRY(LCNPHY, ClipCtrThresh, clipCtrThreshLoGain, 10)
	        PHY_REG_MOD_ENTRY(LCNPHY, gaindirectMismatch, MedHigainDirectMismatch, 15)
	        PHY_REG_MOD_ENTRY(LCNPHY, LowGainDB, MedLowGainDB, 40)
		PHY_REG_MOD_ENTRY(LCNPHY, HiGainDB, MedHiGainDB, 58)
		PHY_REG_MOD_ENTRY(LCNPHY, gaindirectMismatch, GainmisMatchMedGain, 6)
	        PHY_REG_MOD_ENTRY(LCNPHY, VeryLowGainDB, veryLowGainDB, 13)
	        PHY_REG_MOD_ENTRY(LCNPHY, veryLowGainEx, veryLowGain_2DB, 13)
	        PHY_REG_MOD_ENTRY(LCNPHY, veryLowGainEx, veryLowGain_3DB, 13)
		PHY_REG_MOD_ENTRY(LCNPHY, MinPwrLevel, ofdmMinPwrLevel, -92)
		PHY_REG_MOD_ENTRY(LCNPHY, MinPwrLevel, dsssMinPwrLevel, -97)
	      PHY_REG_LIST_EXECUTE(pi);
	    }

	    if (pi->aa2g == 3) {
	      PHY_REG_LIST_START_WLBANDINITDATA
	        PHY_REG_MOD_ENTRY(LCNPHY, gainMismatchMedGainEx, medHiGainDirectMismatchOFDMDet, 15)
	        PHY_REG_MOD_ENTRY(LCNPHY, ofdmPwrThresh0, ofdmPwrThresh0, 2)
	        PHY_REG_MOD_ENTRY(LCNPHY, ofdmPwrThresh0, ofdmPwrThresh1, 6)
	      PHY_REG_LIST_EXECUTE(pi);
	    }

	  } else {
#ifdef BAND5G
	    if (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA_5GHz) {
		/* WAR to the Higher A-band Channels Rxper Hump @-60 to -70dBm Signal Levels
		   From Anritsu8860C Tester.
		   FIXME: there seem to be some side-effects for ilna board.
		*/
		phy_utils_mod_phyreg(pi, LCNPHY_crsMiscCtrl0, LCNPHY_crsMiscCtrl0_cfoCalcEn_MASK,
			0 << LCNPHY_crsMiscCtrl0_cfoCalcEn_SHIFT);

		if ((pi_lcn->tridx5g != 0) && (pi_lcn->tridx5g != 0xff)) {
		/* New settins to improve things at high input power */
		PHY_REG_LIST_START_WLBANDINITDATA
		 PHY_REG_MOD_ENTRY(LCNPHY, radioCtrl, extlnaen, 1)
		 PHY_REG_MOD_ENTRY(LCNPHY, HiGainDB, HiGainDB, 70)
		 PHY_REG_WRITE_ENTRY(LCNPHY, nfSubtractVal, 392)
		 PHY_REG_MOD_ENTRY(LCNPHY, HiGainDB, MedHiGainDB, 46)
		 PHY_REG_MOD_ENTRY(LCNPHY, LowGainDB, MedLowGainDB, 37)
		 PHY_REG_MOD_ENTRY(LCNPHY, VeryLowGainDB, veryLowGainDB, 16)
		 PHY_REG_MOD_ENTRY(LCNPHY, veryLowGainEx, veryLowGain_2DB, 28)
		 PHY_REG_MOD_ENTRY(LCNPHY, veryLowGainEx, veryLowGain_3DB, 28)
		 PHY_REG_MOD_ENTRY(LCNPHY, ClipThresh, ClipThresh, 70)
		 PHY_REG_MOD_ENTRY(LCNPHY, ClipCtrThresh, ClipCtrThreshHiGain, 15)
		 PHY_REG_MOD_ENTRY(LCNPHY, ClipCtrThresh, clipCtrThreshLoGain, 20)
		 PHY_REG_MOD_ENTRY(LCNPHY, PwrThresh1, LargeGainMismatchThresh, 6)
		 PHY_REG_MOD_ENTRY(LCNPHY, gainMismatchMedGainEx, medHiGainDirectMismatchOFDMDet, 6)
		 PHY_REG_MOD_ENTRY(LCNPHY, gaindirectMismatch, MedHigainDirectMismatch, 7)
		 PHY_REG_MOD_ENTRY(LCNPHY, radioTRCtrlCrs1, trGainThresh, 28)
		 PHY_REG_MOD_ENTRY(LCNPHY, gaindirectMismatch, medGainGmShftVal, 3)
		 PHY_REG_MOD_ENTRY(LCNPHY, gmShiftReg, medGain2GmShftVal, 3)
		 PHY_REG_MOD_ENTRY(LCNPHY, gmShiftReg, medGain3GmShftVal, 3)
		 PHY_REG_MOD_ENTRY(LCNPHY, RxRadioControlFltrState, rx_flt_high_start_state, 1)
		 PHY_REG_MOD_ENTRY(LCNPHY, RxRadioControlFltrState, rx_flt_high_hold_state, 2)
		 PHY_REG_MOD_ENTRY(LCNPHY, RxRadioControlFltrState, rx_flt_low_start_state, 3)
		 PHY_REG_MOD_ENTRY(LCNPHY, gainMismatchLimit, gainmismatchlimit, 30)
		 PHY_REG_MOD_ENTRY(LCNPHY, MinPwrLevel, ofdmMinPwrLevel, -95)
		 PHY_REG_MOD_ENTRY(LCNPHY, ClipCtrDefThresh, clipCtrThresh, 15)
		PHY_REG_LIST_EXECUTE(pi);
		} else {
		PHY_REG_LIST_START_WLBANDINITDATA
		 PHY_REG_MOD_ENTRY(LCNPHY, radioCtrl, extlnaen, 1)
		 PHY_REG_MOD_ENTRY(LCNPHY, HiGainDB, HiGainDB, 73)
		 PHY_REG_WRITE_ENTRY(LCNPHY, nfSubtractVal, 392)
		 PHY_REG_MOD_ENTRY(LCNPHY, HiGainDB, MedHiGainDB, 61)
		 PHY_REG_MOD_ENTRY(LCNPHY, LowGainDB, MedLowGainDB, 43)
		 PHY_REG_MOD_ENTRY(LCNPHY, VeryLowGainDB, veryLowGainDB, 26)
		 PHY_REG_MOD_ENTRY(LCNPHY, veryLowGainEx, veryLowGain_2DB, 23)
		 PHY_REG_MOD_ENTRY(LCNPHY, veryLowGainEx, veryLowGain_3DB, 23)
		 PHY_REG_MOD_ENTRY(LCNPHY, ClipThresh, ClipThresh, 70)
		 PHY_REG_MOD_ENTRY(LCNPHY, ClipCtrThresh, ClipCtrThreshHiGain, 15)
		 PHY_REG_MOD_ENTRY(LCNPHY, ClipCtrThresh, clipCtrThreshLoGain, 15)
		 PHY_REG_MOD_ENTRY(LCNPHY, PwrThresh1, LargeGainMismatchThresh, 6)
		 PHY_REG_MOD_ENTRY(LCNPHY, gainMismatchMedGainEx, medHiGainDirectMismatchOFDMDet, 6)
		 PHY_REG_MOD_ENTRY(LCNPHY, gaindirectMismatch, MedHigainDirectMismatch, 12)
		 PHY_REG_MOD_ENTRY(LCNPHY, gainMismatch, GainmisMatchPktRx, 7)
		 PHY_REG_MOD_ENTRY(LCNPHY, PwrThresh1, PktRxSignalDropThresh, 15)
		PHY_REG_LIST_EXECUTE(pi);
		}
	    } else {
		/* Settings for 5G wlbga internal lna board */
		PHY_REG_LIST_START_WLBANDINITDATA
		 PHY_REG_MOD_ENTRY(LCNPHY, HiGainDB, HiGainDB, 67)
		 PHY_REG_MOD_ENTRY(LCNPHY, HiGainDB, MedHiGainDB, 49)
		 PHY_REG_MOD_ENTRY(LCNPHY, LowGainDB, MedLowGainDB, 37)
		 PHY_REG_MOD_ENTRY(LCNPHY, ClipCtrThresh, ClipCtrThreshHiGain, 18)
		 PHY_REG_MOD_ENTRY(LCNPHY, clipCtrThreshLowGainEx, clipCtrThreshLoGain3, 15)
		 PHY_REG_MOD_ENTRY(LCNPHY, clipCtrThreshLowGainEx, clipCtrThreshLoGain2, 15)
		 PHY_REG_MOD_ENTRY(LCNPHY, ClipCtrThresh, clipCtrThreshLoGain, 15)
		 PHY_REG_MOD_ENTRY(LCNPHY, veryLowGainEx, veryLowGain_3DB, 13)
		 PHY_REG_MOD_ENTRY(LCNPHY, veryLowGainEx, veryLowGain_2DB, 13)
		 PHY_REG_MOD_ENTRY(LCNPHY, VeryLowGainDB, veryLowGainDB, 13)
		 PHY_REG_MOD_ENTRY(LCNPHY, gaindirectMismatch, medGainGmShftVal, 0)
		 PHY_REG_MOD_ENTRY(LCNPHY, gainMismatch, GainMismatchHigain, 12)
		 PHY_REG_MOD_ENTRY(LCNPHY, gainMismatch, ofdmGainMismatchLogain, 12)
		 PHY_REG_MOD_ENTRY(LCNPHY, gainMismatch, GainMismatchNomgain, 12)
		 PHY_REG_MOD_ENTRY(LCNPHY, gaindirectMismatch, MedHigainDirectMismatch, 8)
		 PHY_REG_MOD_ENTRY(LCNPHY, gainMismatchMedGainEx, medHiGainDirectMismatchOFDMDet, 7)
		 PHY_REG_MOD_ENTRY(LCNPHY, gainBackOffVal, genGainBkOff, 3)
		 PHY_REG_MOD_ENTRY(LCNPHY, gainMismatch, GainmisMatchPktRx, 9)
		 PHY_REG_MOD_ENTRY(LCNPHY, MinPwrLevel, ofdmMinPwrLevel, -90)
		PHY_REG_LIST_EXECUTE(pi);
	    }

	    if (pi->aa2g == 3) {
	       PHY_REG_MOD(pi, LCNPHY, gainMismatchMedGainEx, medHiGainDirectMismatchOFDMDet, 15);
	    }

#endif /* BAND5G */
	  }
	} else if (CHIPID(pi->sh->chip) == BCM4336_CHIP_ID) {
		PHY_REG_MOD(pi, LCNPHY, HiGainDB, HiGainDB, 73);
		PHY_REG_MOD(pi, LCNPHY, InputPowerDB, inputpwroffsetdb, -5);
	}
}

static void
WLBANDINITFN(wlc_lcnphy_baseband_init)(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;

	PHY_TRACE(("%s:***CHECK***\n", __FUNCTION__));

	/* use .5 dB step in hardware power control */
	if ((CHSPEC_IS2G(pi->radio_chanspec) && ((pi_lcn->pacalidx_2g1 != -1) ||
		(pi_lcn->pacalamamth_2g1 != -1) || (pi_lcn->pacalpwr_2g1 != -1))) ||
		(CHSPEC_IS5G(pi->radio_chanspec) && ((pi_lcn->pacalidx_5glo1 != -1) ||
		(pi_lcn->pacalidx_5g1 != -1) || (pi_lcn->pacalidx_5ghi1 != -1) ||
		(pi_lcn->pacalamamth_5glo1 != -1) || (pi_lcn->pacalpwr_5glo1 != -1) ||
		(pi_lcn->pacalamamth_5g1 != -1) || (pi_lcn->pacalpwr_5g1 != -1) ||
		(pi_lcn->pacalamamth_5ghi1 != -1) || (pi_lcn->pacalpwr_5ghi1 != -1)))) {
		pi_lcn->virtual_p25_tx_gain_step = 1;
		PHY_REG_MOD(pi, LCNPHY, TxPwrCtrlRangeCmd, txGainTable_mode, 1);
		PHY_REG_MOD(pi, LCNPHY, bbShiftCtrl, bbshift_mode, 1);
	} else {
		pi_lcn->virtual_p25_tx_gain_step = 0;
		PHY_REG_MOD(pi, LCNPHY, TxPwrCtrlRangeCmd, txGainTable_mode, 0);
		PHY_REG_MOD(pi, LCNPHY, bbShiftCtrl, bbshift_mode, 0);
	}

	/* Initialize LCNPHY tables */
	wlc_lcnphy_tbl_init(pi);
	wlc_lcnphy_rev0_baseband_init(pi);
	if (LCNREV_GE(pi->pubpi.phy_rev, 2))
		wlc_lcnphy_rev2_baseband_init(pi);
	wlc_lcnphy_bu_tweaks(pi);
	wlc_lcnphy_noise_init(pi);
}

/* mapped onto lcnphy_rev0_rf_init proc */
static void
WLBANDINITFN(wlc_radio_2064_init)(phy_info_t *pi)
{
	uint32 i;
	uint16 lpfbwlut;
	uint16 shm_addr; /* Addr of SHM blk for LCNPHY params */
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	lcnphy_radio_regs_t *lcnphyregs = NULL;
	int8 ofdm_tx_lpf_bw;
	uint16 tx_alpf_bypass;
	int8 tx_biq1_gain;
	int8 tx_biq2_gain;

	PHY_INFORM(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	/* init radio regs */
	if (RADIOID(pi->pubpi.radioid) == BCM2066_ID)
		lcnphyregs = lcnphy_radio_regs_2066;
	else
		lcnphyregs = lcnphy_radio_regs_2064;

	for (i = 0; lcnphyregs[i].address != 0xffff; i++)
		if (CHSPEC_IS5G(pi->radio_chanspec) &&
			lcnphyregs[i].do_init_a)
			phy_utils_write_radioreg(pi,
				((lcnphyregs[i].address & 0x3fff) |
				RADIO_DEFAULT_CORE),
				(uint16)lcnphyregs[i].init_a);
		else if (lcnphyregs[i].do_init_g)
			phy_utils_write_radioreg(pi,
				((lcnphyregs[i].address & 0x3fff) |
				RADIO_DEFAULT_CORE),
				(uint16)lcnphyregs[i].init_g);

	if ((RADIOID(pi->pubpi.radioid) == BCM2064_ID) &&
		(CHIPID(pi->sh->chip) != BCM4313_CHIP_ID)) {
		/* disable dac gpio */
		phy_utils_write_radioreg(pi, RADIO_2064_REG073, 0);
	}

	/* pa class setting */
#ifdef BAND5G
	if (CHSPEC_IS5G(pi->radio_chanspec)) {
		if (CHIPID(pi->sh->chip) == BCM4313_CHIP_ID)
			phy_utils_write_radioreg(pi, RADIO_2064_REG032, 0x65);
		else
			phy_utils_write_radioreg(pi, RADIO_2064_REG032, 0x6a);
	} else
#endif // endif
		if (PAPD_4336_MODE(pi_lcn))
			phy_utils_write_radioreg(pi, RADIO_2064_REG032, 0x6a);
		else
			phy_utils_write_radioreg(pi, RADIO_2064_REG032, 0x62);
	phy_utils_write_radioreg(pi, RADIO_2064_REG033, 0x19);

	if (pi_lcn->rfreg033)
		phy_utils_write_radioreg(pi, RADIO_2064_REG033, pi_lcn->rfreg033);

	PHY_REG_LIST_START_WLBANDINITDATA
		RADIO_REG_WRITE_ENTRY(RADIO_2064_REG0C2, 0x6f)

		/* TX Tuning */
		RADIO_REG_WRITE_ENTRY(RADIO_2064_REG090, 0x10)

		/* set LPF gain to 0 dB let ucode enable/disable override register based on tx/rx */
		RADIO_REG_WRITE_ENTRY(RADIO_2064_REG010, 0x00)
	PHY_REG_LIST_EXECUTE(pi);

	if ((CHIPID(pi->sh->chip) == BCM4313_CHIP_ID) && LCNREV_IS(pi->pubpi.phy_rev, 1)) {
		/* adjust lna1,lna2 current for B0 */
		PHY_REG_LIST_START_WLBANDINITDATA
			RADIO_REG_WRITE_ENTRY(RADIO_2064_REG060, 0x7f)
			RADIO_REG_WRITE_ENTRY(RADIO_2064_REG061, 0x72)
			RADIO_REG_WRITE_ENTRY(RADIO_2064_REG062, 0x7f)
		PHY_REG_LIST_EXECUTE(pi);
	}
	if (CHIPID(pi->sh->chip) == BCM4336_CHIP_ID ||
	    CHIPID(pi->sh->chip) == BCM43362_CHIP_ID)
		phy_utils_write_radioreg(pi, RADIO_2064_REG09B, 0x7);

	phy_utils_write_radioreg(pi, RADIO_2064_REG01D, 0x02);
	phy_utils_write_radioreg(pi, RADIO_2064_REG01E, 0x06);
	/* lpfbwlutregx = x where x = 0:4 */
	lpfbwlut = (0 << LCNPHY_lpfbwlutreg0_lpfbwlut0_SHIFT) |
		(1 << LCNPHY_lpfbwlutreg0_lpfbwlut1_SHIFT) |
		(2 << LCNPHY_lpfbwlutreg0_lpfbwlut2_SHIFT) |
		(3 << LCNPHY_lpfbwlutreg0_lpfbwlut3_SHIFT) |
		(4 << LCNPHY_lpfbwlutreg0_lpfbwlut4_SHIFT);

	PHY_REG_WRITE(pi, LCNPHY, lpfbwlutreg0, lpfbwlut);

	/* setting lpf_ofdm_tx_bw, lpf_cck_tx_bw and lpf_rx_bw */
	if ((CHIPID(pi->sh->chip) == BCM4313_CHIP_ID) && !EPA(pi_lcn))
		ofdm_tx_lpf_bw = 3;
	else {
		if (CHSPEC_IS2G(pi->radio_chanspec) && (pi_lcn->dacrate == 160) && !EPA(pi_lcn))
			ofdm_tx_lpf_bw = 4;
		else
			ofdm_tx_lpf_bw = 2;
	}

	if (CHSPEC_IS2G(pi->radio_chanspec) && (pi->ofdm_analog_filt_bw_override_2g != -1))
		ofdm_tx_lpf_bw = (uint8)pi->ofdm_analog_filt_bw_override_2g;
	else if (CHSPEC_IS5G(pi->radio_chanspec) && (pi->ofdm_analog_filt_bw_override_5g != -1))
		ofdm_tx_lpf_bw = (uint8)pi->ofdm_analog_filt_bw_override_5g;

	PHY_REG_MOD(pi, LCNPHY, lpfbwlutreg1, lpf_ofdm_tx_bw, ofdm_tx_lpf_bw);
	if (LCNREV_GE(pi->pubpi.phy_rev, 2)) {
		if (CHSPEC_IS2G(pi->radio_chanspec) || (CHSPEC_IS5G(pi->radio_chanspec) &&
		                                        !TXGAINTBL5G(pi_lcn))) {
			tx_biq1_gain = 0;
			tx_biq2_gain = 0;
		} else {
			tx_biq1_gain = 1;
			tx_biq2_gain = 1;
		}
		PHY_REG_MOD(pi, LCNPHY, lpfofdmhtlutreg, lpf_ofdm_ht_tx_biq1_gain, tx_biq1_gain);
		PHY_REG_MOD(pi, LCNPHY, lpfofdmhtlutreg, lpf_ofdm_ht_tx_biq2_gain, tx_biq2_gain);
		PHY_REG_MOD(pi, LCNPHY, lpfgainlutreg, lpf_ofdm_tx_biq1_gain, tx_biq1_gain);
		PHY_REG_MOD(pi, LCNPHY, lpfgainlutreg, lpf_ofdm_tx_biq2_gain, tx_biq2_gain);
	}
	if (LCNREV_GE(pi->pubpi.phy_rev, 2))
		PHY_REG_MOD(pi, LCNPHY, lpfofdmhtlutreg, lpf_ofdm_tx_ht_bw, ofdm_tx_lpf_bw);

	if (CHSPEC_IS5G(pi->radio_chanspec))
		tx_alpf_bypass = pi->tx_alpf_bypass_5g;
	else
		tx_alpf_bypass = pi->tx_alpf_bypass_2g;

	PHY_REG_MOD(pi, LCNPHY, lpfbwlutreg1, lpf_ofdm_tx_byp, tx_alpf_bypass);

	if (CHSPEC_IS5G(pi->radio_chanspec))
		PHY_REG_MOD(pi, LCNPHY, lpfbwlutreg1, lpf_cck_tx_byp, tx_alpf_bypass);
	else {
		if (pi->tx_alpf_bypass_cck_2g != -1)
			PHY_REG_MOD(pi, LCNPHY, lpfbwlutreg1, lpf_cck_tx_byp,
			pi->tx_alpf_bypass_cck_2g);
		else
			PHY_REG_MOD(pi, LCNPHY, lpfbwlutreg1, lpf_cck_tx_byp, tx_alpf_bypass);
	}

	pi->u.pi_lcnphy->lpf_cck_tx_byp = PHY_REG_READ(pi, LCNPHY, lpfbwlutreg1, lpf_cck_tx_byp);
	pi->u.pi_lcnphy->lpf_ofdm_tx_byp = PHY_REG_READ(pi, LCNPHY, lpfbwlutreg1, lpf_ofdm_tx_byp);

	pi->u.pi_lcnphy->use_per_modulation_loft_cal =
		(pi->u.pi_lcnphy->lpf_cck_tx_byp != pi->u.pi_lcnphy->lpf_ofdm_tx_byp) ? 1 : 0;

	if (LCNREV_GE(pi->pubpi.phy_rev, 2))
		PHY_REG_MOD(pi, LCNPHY, iq_coeff_a_cck,
			frmTypBasedIqLoCoeff,
			wlc_lcnphy_frmTypBasedIqLoCoeff(pi));

	/* code to set lpf_cck_tx_bw moved to wlc_phy_chanspec_set_lcnphy() */
	PHY_REG_LIST_START_WLBANDINITDATA
		/* rx setting */
		PHY_REG_MOD_ENTRY(LCNPHY, lpfbwlutreg1, lpf_rx_bw, 0)
		PHY_REG_MOD_ENTRY(LCNPHY, ccktx_phycrsDelayValue, ccktx_phycrsDelayValue, 25)
	PHY_REG_LIST_EXECUTE(pi);

	if (pi->btclock_tune)
		phy_utils_mod_radioreg(pi, RADIO_2064_REG077, 0x0F, 0x0F);

	/* setting tx lo coefficients to 0 */
	wlc_lcnphy_set_tx_locc(pi, 0);

	if (!pi_lcn->startup_init) {
		/* run rcal */
		wlc_lcnphy_rcal(pi);

		/* run rc_cal */
		wlc_lcnphy_rc_cal(pi);
	}

	/* 4313 IPA tuning */
	if ((CHIPID(pi->sh->chip) == BCM4313_CHIP_ID) && !EPA(pi_lcn)) {
		PHY_REG_LIST_START_WLBANDINITDATA
			RADIO_REG_WRITE_ENTRY(RADIO_2064_REG02C, 0x7)
			RADIO_REG_WRITE_ENTRY(RADIO_2064_REG032, 0x6f)
			RADIO_REG_WRITE_ENTRY(RADIO_2064_REG08F, 0xc)
			RADIO_REG_WRITE_ENTRY(RADIO_2064_REG033, 0x39)
			RADIO_REG_WRITE_ENTRY(RADIO_2064_REG003, 0x66)
			RADIO_REG_WRITE_ENTRY(RADIO_2064_REG00D, 0x0)
			RADIO_REG_WRITE_ENTRY(RADIO_2064_REG00C, 0x55)
			RADIO_REG_WRITE_ENTRY(RADIO_2064_REG00A, 0x73)
			RADIO_REG_WRITE_ENTRY(RADIO_2064_REG008, 0xff)
			RADIO_REG_WRITE_ENTRY(RADIO_2064_REG01F, 0x0)
			RADIO_REG_WRITE_ENTRY(RADIO_2064_REG01D, 0x7)
			RADIO_REG_MOD_ENTRY(RADIO_2064_REG01E, 0x1c, 0x0)

			RADIO_REG_WRITE_ENTRY(RADIO_2064_REG032, 0x6f)
			RADIO_REG_WRITE_ENTRY(RADIO_2064_REG033, 0x19)
			RADIO_REG_WRITE_ENTRY(RADIO_2064_REG039, 0xe)
		PHY_REG_LIST_EXECUTE(pi);
	}

	shm_addr = 2*wlapi_bmac_read_shm(pi->sh->physhim, M_LCNPHY_BLK_PTR);
	if (pi_lcn->rfreg033) {
		if (shm_addr)
			wlapi_bmac_write_shm(pi->sh->physhim,
			     shm_addr + 2*M_LCNPHY_PABIAS_OFDM_OFFSET, pi_lcn->rfreg033);
	}
	if (pi_lcn->rfreg033_cck) {
		if (shm_addr)
			wlapi_bmac_write_shm(pi->sh->physhim,
			     shm_addr + 2*M_LCNPHY_PABIAS_CCK_OFFSET, pi_lcn->rfreg033_cck);
	}
	if (pi_lcn->xtal_mode[0]) {
		phy_utils_mod_radioreg(pi, RADIO_2064_REG11D, 0x04, 0x04);
		phy_utils_mod_radioreg(pi, RADIO_2064_REG09F, 0x3f, pi_lcn->xtal_mode[1]);
		phy_utils_mod_radioreg(pi, RADIO_2064_REG09E, 0x3f, pi_lcn->xtal_mode[2]);
		phy_utils_mod_radioreg(pi, RADIO_2064_REG077, 0x0f, pi_lcn->xtal_mode[3]);
	}
	if (CHSPEC_IS5G(pi->radio_chanspec)) {

		/* radio register settings to improve VCO leakage at ~7.3 GHz
		spur suppression (and risk of side effects esp. at temperature)
		increases with increases with value of loidacmode5g
		*/
		if (pi_lcn->loidacmode_5g != 0) {
			PHY_REG_LIST_START_WLBANDINITDATA
				RADIO_REG_WRITE_ENTRY(RADIO_2064_REG0B4, 0x24)
				RADIO_REG_WRITE_ENTRY(RADIO_2064_REG0B7, 0x24)
				RADIO_REG_WRITE_ENTRY(RADIO_2064_REG0B8, 0x3)
			PHY_REG_LIST_EXECUTE(pi);
			if (pi_lcn->loidacmode_5g == 2) {
				phy_utils_write_radioreg(pi, RADIO_2064_REG0B8, 0x2);
				phy_utils_write_radioreg(pi, RADIO_2064_REG0B5, 0x1);
			} else if (pi_lcn->loidacmode_5g == 3) {
				phy_utils_write_radioreg(pi, RADIO_2064_REG0B8, 0x2);
				phy_utils_write_radioreg(pi, RADIO_2064_REG0B5, 0x0);
			}
		}
	}
}

static void
WLBANDINITFN(wlc_lcnphy_radio_init)(phy_info_t *pi)
{
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	if (NORADIO_ENAB(pi->pubpi))
		return;
	/* Initialize 2064 radio */
	wlc_radio_2064_init(pi);
}

static void
wlc_lcnphy_rcal(phy_info_t *pi)
{
	uint8 rcal_value;
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	if (NORADIO_ENAB(pi->pubpi))
		return;

	PHY_REG_LIST_START
		/* power down RCAL */
		RADIO_REG_AND_ENTRY(RADIO_2064_REG05B, 0xfD)
		/* enable pwrsw */
		RADIO_REG_OR_ENTRY(RADIO_2064_REG004, 0x40)
		RADIO_REG_OR_ENTRY(RADIO_2064_REG120, 0x10)
		/* enable bandgap */
		RADIO_REG_OR_ENTRY(RADIO_2064_REG078, 0x80)
		RADIO_REG_OR_ENTRY(RADIO_2064_REG129, 0x02)
		/* enable RCAL clock */
		RADIO_REG_OR_ENTRY(RADIO_2064_REG057, 0x01)
		/* power up RCAL */
		RADIO_REG_OR_ENTRY(RADIO_2064_REG05B, 0x02)
	PHY_REG_LIST_EXECUTE(pi);

	OSL_DELAY(5000);
	SPINWAIT(!wlc_radio_2064_rcal_done(pi), 10 * 1000 * 1000);
	/* wait for RCAL valid bit to be set */
	if (wlc_radio_2064_rcal_done(pi)) {
		rcal_value = (uint8) phy_utils_read_radioreg(pi, RADIO_2064_REG05C);
		rcal_value = rcal_value & 0x1f;
		PHY_INFORM(("wl%d: %s:  Rx RCAL completed, code: %x\n",
		pi->sh->unit, __FUNCTION__, rcal_value));
	} else {
		PHY_ERROR(("wl%d: %s: RCAL failed\n", pi->sh->unit, __FUNCTION__));
	}
	PHY_REG_LIST_START
		/* power down RCAL */
		RADIO_REG_AND_ENTRY(RADIO_2064_REG05B, 0xfD)
		/* disable RCAL clock */
		RADIO_REG_AND_ENTRY(RADIO_2064_REG057, 0xFE)

		/* Clear the bandgap override */
		RADIO_REG_AND_ENTRY(RADIO_2064_REG129, 0xFD)
	PHY_REG_LIST_EXECUTE(pi);
}

static void
wlc_lcnphy_rc_cal(phy_info_t *pi)
{
	/* Bringing in code from lcnphyprocs.tcl */
	uint8 old_big_cap, old_small_cap;
	uint8 save_pll_jtag_pll_xtal, save_bb_rccal_hpc;
	uint8 dflt_rc_cal_val, hpc_code, hpc_offset, b_cap_rc_code_raw, s_cap_rc_code_raw;
	uint16 flt_val, trc;
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	hpc_code = 0xb;
	b_cap_rc_code_raw = 0xb;
	s_cap_rc_code_raw = 0x9;

	if (NORADIO_ENAB(pi->pubpi))
		return;

	if (CHIPID(pi->sh->chip) == BCM4313_CHIP_ID) {
		dflt_rc_cal_val = 7;
		if (LCNREV_IS(pi->pubpi.phy_rev, 1))
			dflt_rc_cal_val = 11;
		flt_val = (dflt_rc_cal_val << 10) | (dflt_rc_cal_val << 5) | (dflt_rc_cal_val);
		PHY_REG_WRITE(pi, LCNPHY, lpf_rccal_tbl_1, flt_val);
		PHY_REG_WRITE(pi, LCNPHY, lpf_rccal_tbl_2, flt_val);
		PHY_REG_WRITE(pi, LCNPHY, lpf_rccal_tbl_3, flt_val);
		PHY_REG_WRITE(pi, LCNPHY, lpf_rccal_tbl_4, flt_val);
		PHY_REG_WRITE(pi, LCNPHY, lpf_rccal_tbl_5, (flt_val & 0x1FF));
		phy_utils_and_radioreg(pi, RADIO_2064_REG057, 0xFD);
		return;
	}
	/* 4330 */
	if (RADIOID(pi->pubpi.radioid) == BCM2064_ID) {
		/* 4336 WLBGA */
		if ((CHIPID(pi->sh->chip) == BCM4336_CHIP_ID) &&
			(pi->sh->chippkg == BCM4336_WLBGA_PKG_ID)) {
			if (PHY_XTALFREQ(pi->xtalfreq) / 1000000 == 26) {
				trc = 0xFA;
				hpc_offset = 0;
			} else {
				PHY_INFORM(("wl%d: %s:  RCCAL not run, xtal "
					"%d not known\n",
					pi->sh->unit, __FUNCTION__, PHY_XTALFREQ(pi->xtalfreq)));
				return;
			}
		} else if ((CHIPID(pi->sh->chip) == BCM4336_CHIP_ID) &&
			(LCNREV_IS(pi->pubpi.phy_rev, 1))) {
			trc = PHY_XTALFREQ(pi->xtalfreq) / 1000000 * 254 / 26;
			hpc_offset = 0;
		}
		else {
			trc = PHY_XTALFREQ(pi->xtalfreq) / 1000000 * 264 / 26;
			hpc_offset = 2;
		}
	} else {
		if (PHY_XTALFREQ(pi->xtalfreq) / 100000 == 374) {
			trc = 0x169;
			hpc_offset = 0;
		} else {
			PHY_INFORM(("wl%d: %s:  RCCAL not run, xtal "
				"%d not known\n",
				pi->sh->unit, __FUNCTION__, PHY_XTALFREQ(pi->xtalfreq)));
			return;
		}
	}

	/* Save old value of pll_xtal_1 */
	save_pll_jtag_pll_xtal = (uint8) phy_utils_read_radioreg(pi, RADIO_2064_REG057);
	/* Save old HPC value in case RCCal fails */
	save_bb_rccal_hpc = (uint8) phy_utils_read_radioreg(pi, RADIO_2064_REG017);
	BCM_REFERENCE(save_bb_rccal_hpc);
	/* Save old big cap value in case RCCal fails */
	old_big_cap = (uint8)phy_utils_read_radioreg(pi, RADIO_2064_REG018);

	PHY_REG_LIST_START
		/* Power down RC CAL */
		RADIO_REG_AND_ENTRY(RADIO_2064_REG105, (uint8)~(0x04))

		/*	enable Pwrsw	*/
		RADIO_REG_OR_ENTRY(RADIO_2064_REG004, 0x40)
		RADIO_REG_OR_ENTRY(RADIO_2064_REG120, 0x10)
		/* Enable RCCal Clock */
		RADIO_REG_OR_ENTRY(RADIO_2064_REG057, 0x02)
		/* Power up RC Cal */
		RADIO_REG_OR_ENTRY(RADIO_2064_REG105, 0x04)
		/* setup to run Rx RC Cal and setup R1/Q1/P1 */
		RADIO_REG_WRITE_ENTRY(RADIO_2064_REG106, 0x2A)
		/* set X1<7:0> */
		RADIO_REG_WRITE_ENTRY(RADIO_2064_REG107, 0x6E)
	PHY_REG_LIST_EXECUTE(pi);

	/* set Trc <7:0> */
	phy_utils_write_radioreg(pi, RADIO_2064_REG108, trc & 0xFF);
	/* set Trc <12:8> */
	phy_utils_write_radioreg(pi, RADIO_2064_REG109, (trc >> 8) & 0x1F);
	/* RCCAL on big cap first */
	phy_utils_and_radioreg(pi, RADIO_2064_REG105, (uint8)~(0x02));
	/* Start RCCAL */
	phy_utils_or_radioreg(pi, RADIO_2064_REG106, 0x01);
	/* check to see if RC CAL is done */
	OSL_DELAY(50);
	SPINWAIT(!wlc_radio_2064_rc_cal_done(pi), 10 * 1000 * 1000);
	if (!wlc_radio_2064_rc_cal_done(pi)) {
		PHY_ERROR(("wl%d: %s: Big Cap RC Cal failed\n", pi->sh->unit, __FUNCTION__));
		phy_utils_or_radioreg(pi, RADIO_2064_REG018, old_big_cap);
	} else {
		/* RCCAL successful */
		PHY_INFORM(("wl%d: %s:  Rx RC Cal completed for "
		            "cap: N0: %x%x, N1: %x%x, code: %x\n",
		            pi->sh->unit, __FUNCTION__,
		            phy_utils_read_radioreg(pi, RADIO_2064_REG10C),
		            phy_utils_read_radioreg(pi, RADIO_2064_REG10B),
		            phy_utils_read_radioreg(pi, RADIO_2064_REG10E),
		            phy_utils_read_radioreg(pi, RADIO_2064_REG10D),
		            phy_utils_read_radioreg(pi, RADIO_2064_REG10F) & 0x1f));
		b_cap_rc_code_raw = phy_utils_read_radioreg(pi, RADIO_2064_REG10F) & 0x1f;
		if (b_cap_rc_code_raw < 0x1E)
			hpc_code = b_cap_rc_code_raw + hpc_offset;
	}
	/* RCCAL on small cap */
	/* Save old small cap value in case RCCal fails */
	old_small_cap = (uint8)phy_utils_read_radioreg(pi, RADIO_2064_REG019);

	PHY_REG_LIST_START
		/* Stop RCCAL */
		RADIO_REG_AND_ENTRY(RADIO_2064_REG106, (uint8)~(0x01))
		/* Power down RC CAL */
		RADIO_REG_AND_ENTRY(RADIO_2064_REG105, (uint8)~(0x04))
		/* Power up RC Cal */
		RADIO_REG_OR_ENTRY(RADIO_2064_REG105, 0x04)
		/* RCCAL on small Cap */
		RADIO_REG_OR_ENTRY(RADIO_2064_REG105, 0x02)
		/* Start RCCAL */
		RADIO_REG_OR_ENTRY(RADIO_2064_REG106, 0x01)
	PHY_REG_LIST_EXECUTE(pi);

		/* check to see if RC CAL is done */
	OSL_DELAY(50);
	SPINWAIT(!wlc_radio_2064_rc_cal_done(pi), 10 * 1000 * 1000);
	if (!wlc_radio_2064_rc_cal_done(pi)) {
		PHY_ERROR(("wl%d: %s: Small Cap RC Cal failed\n", pi->sh->unit, __FUNCTION__));
		phy_utils_write_radioreg(pi, RADIO_2064_REG019, old_small_cap);
	} else {
		/* RCCAL successful */
		PHY_INFORM(("wl%d: %s:  Rx RC Cal completed for cap: "
		            "N0: %x%x, N1: %x%x, code: %x\n",
		            pi->sh->unit, __FUNCTION__,
		            phy_utils_read_radioreg(pi, RADIO_2064_REG10C),
		            phy_utils_read_radioreg(pi, RADIO_2064_REG10B),
		            phy_utils_read_radioreg(pi, RADIO_2064_REG10E),
		            phy_utils_read_radioreg(pi, RADIO_2064_REG10D),
		            phy_utils_read_radioreg(pi, RADIO_2064_REG110) & 0x1f));
		s_cap_rc_code_raw = phy_utils_read_radioreg(pi, RADIO_2064_REG110) & 0x1f;
	}

	/* Stop RC Cal */
	phy_utils_and_radioreg(pi, RADIO_2064_REG106, (uint8)~(0x01));
	/* Power down RC CAL */
	phy_utils_and_radioreg(pi, RADIO_2064_REG105, (uint8)~(0x04));
	/* Restore old value of pll_xtal_1 */
	phy_utils_write_radioreg(pi, RADIO_2064_REG057, save_pll_jtag_pll_xtal);

	flt_val =
		(s_cap_rc_code_raw << 10) | (s_cap_rc_code_raw << 5) | (s_cap_rc_code_raw);
	PHY_REG_WRITE(pi, LCNPHY, lpf_rccal_tbl_1, flt_val);
	PHY_REG_WRITE(pi, LCNPHY, lpf_rccal_tbl_2, flt_val);

	flt_val =
		(b_cap_rc_code_raw << 10) | (b_cap_rc_code_raw << 5) | (s_cap_rc_code_raw);
	PHY_REG_WRITE(pi, LCNPHY, lpf_rccal_tbl_3, flt_val);

	flt_val =
		(b_cap_rc_code_raw << 10) | (b_cap_rc_code_raw << 5) | (b_cap_rc_code_raw);
	PHY_REG_WRITE(pi, LCNPHY, lpf_rccal_tbl_4, flt_val);

	flt_val = (hpc_code << 5) | (hpc_code);
	PHY_REG_WRITE(pi, LCNPHY, lpf_rccal_tbl_5, (flt_val & 0x1FF));

	/* narrow 9 MHz tx filter to improve CCK SM */
	if (0 && PAPD_4336_MODE(pi_lcn) && LCNREV_GE(pi->pubpi.phy_rev, 2)) {
		PHY_REG_LIST_START
			PHY_REG_MOD_ENTRY(LCNPHY, lpf_rccal_tbl_1,  small_tx_9, 0x1F)
			PHY_REG_MOD_ENTRY(LCNPHY, lpf_rccal_tbl_3,  big_tx_9, 0x1F)
		PHY_REG_LIST_EXECUTE(pi);
	}
}

static const char BCMATTACHDATA(rstr_triso2g)[] = "triso2g";
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
static const char BCMATTACHDATA(rstr_tssitxdelay)[] = "tssitxdelay";
static const char BCMATTACHDATA(rstr_initxidx2g)[] = "initxidx2g";
static const char BCMATTACHDATA(rstr_initxidx5g)[] = "initxidx5g";
static const char BCMATTACHDATA(rstr_tssioffsetmax)[] = "tssioffsetmax";
static const char BCMATTACHDATA(rstr_tssioffsetmin)[] = "tssioffsetmin";
static const char BCMATTACHDATA(rstr_tssioffsetmax5gl)[] = "tssioffsetmax5gl";
static const char BCMATTACHDATA(rstr_tssioffsetmin5gl)[] = "tssioffsetmin5gl";
static const char BCMATTACHDATA(rstr_tssioffsetmax5gm)[] = "tssioffsetmax5gm";
static const char BCMATTACHDATA(rstr_tssioffsetmin5gm)[] = "tssioffsetmin5gm";
static const char BCMATTACHDATA(rstr_tssioffsetmax5gh)[] = "tssioffsetmax5gh";
static const char BCMATTACHDATA(rstr_tssioffsetmin5gh)[] = "tssioffsetmin5gh";
static const char BCMATTACHDATA(rstr_dynpwrlimen)[] = "dynpwrlimen";
static const char BCMATTACHDATA(rstr_tssimaxnpt)[] = "tssimaxnpt";
static const char BCMATTACHDATA(rstr_vbatsmf)[] = "vbatsmf";
static const char BCMATTACHDATA(rstr_vbatsmc)[] = "vbatsmc";
static const char BCMATTACHDATA(rstr_vbatsav)[] = "vbatsav";
static const char BCMATTACHDATA(rstr_tempsmf)[] = "tempsmf";
static const char BCMATTACHDATA(rstr_tempsmc)[] = "tempsmc";
static const char BCMATTACHDATA(rstr_tempsav)[] = "tempsav";
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
static const char BCMATTACHDATA(rstr_rxgainerr5gl)[] = "rxgainerr5gl";
static const char BCMATTACHDATA(rstr_rxgainerr5gm)[] = "rxgainerr5gm";
static const char BCMATTACHDATA(rstr_rxgainerr5gh)[] = "rxgainerr5gh";
static const char BCMATTACHDATA(rstr_noiselvl2g)[] = "noiselvl2g";
static const char BCMATTACHDATA(rstr_noiselvl5gl)[] = "noiselvl5gl";
static const char BCMATTACHDATA(rstr_noiselvl5gm)[] = "noiselvl5gm";
static const char BCMATTACHDATA(rstr_noiselvl5gh)[] = "noiselvl5gh";
static const char BCMATTACHDATA(rstr_noiseoff2g)[] = "noiseoff2g";
static const char BCMATTACHDATA(rstr_noiseoff5gl)[] = "noiseoff5gl";
static const char BCMATTACHDATA(rstr_noiseoff5gm)[] = "noiseoff5gm";
static const char BCMATTACHDATA(rstr_noiseoff5gh)[] = "noiseoff5gh";
static const char BCMATTACHDATA(rstr_pwrthresh2g)[] = "pwrthresh2g";
static const char BCMATTACHDATA(rstr_pwrthresh5g)[] = "pwrthresh5g";
static const char BCMATTACHDATA(rstr_maxp2ga0)[] = "maxp2ga0";
static const char BCMATTACHDATA(rstr_cck2gpo)[] = "cck2gpo";
static const char BCMATTACHDATA(rstr_ofdm2gpo)[] = "ofdm2gpo";
static const char BCMATTACHDATA(rstr_mcs2gpo1)[] = "mcs2gpo1";
static const char BCMATTACHDATA(rstr_mcs2gpo0)[] = "mcs2gpo0";
static const char BCMATTACHDATA(rstr_maxp5ga0)[] = "maxp5ga0";
static const char BCMATTACHDATA(rstr_ofdm5gpo)[] = "ofdm5gpo";
static const char BCMATTACHDATA(rstr_mcs5gpo1)[] = "mcs5gpo1";
static const char BCMATTACHDATA(rstr_mcs5gpo0)[] = "mcs5gpo0";
static const char BCMATTACHDATA(rstr_maxp5gla0)[] = "maxp5gla0";
static const char BCMATTACHDATA(rstr_ofdm5glpo)[] = "ofdm5glpo";
static const char BCMATTACHDATA(rstr_mcs5glpo1)[] = "mcs5glpo1";
static const char BCMATTACHDATA(rstr_mcs5glpo0)[] = "mcs5glpo0";
static const char BCMATTACHDATA(rstr_maxp5gha0)[] = "maxp5gha0";
static const char BCMATTACHDATA(rstr_ofdm5ghpo)[] = "ofdm5ghpo";
static const char BCMATTACHDATA(rstr_mcs5ghpo1)[] = "mcs5ghpo1";
static const char BCMATTACHDATA(rstr_mcs5ghpo0)[] = "mcs5ghpo0";
static const char BCMATTACHDATA(rstr_rawtempsense)[] = "rawtempsense";
static const char BCMATTACHDATA(rstr_measpower)[] = "measpower";
static const char BCMATTACHDATA(rstr_measpower1)[] = "measpower1";
static const char BCMATTACHDATA(rstr_measpower2)[] = "measpower2";
static const char BCMATTACHDATA(rstr_tempsense_slope)[] = "tempsense_slope";
static const char BCMATTACHDATA(rstr_tempcorrx)[] = "tempcorrx";
static const char BCMATTACHDATA(rstr_tempsense_option)[] = "tempsense_option";
static const char BCMATTACHDATA(rstr_openlppwrctrl)[] = "openlppwrctrl";
static const char BCMATTACHDATA(rstr_openlprefpwr)[] = "openlprefpwr";
static const char BCMATTACHDATA(rstr_openlpgainidxbN)[] = "openlpgainidxb%d";
static const char BCMATTACHDATA(rstr_openlpgainidxa36)[] = "openlpgainidxa36";
static const char BCMATTACHDATA(rstr_openlpgainidxa40)[] = "openlpgainidxa40";
static const char BCMATTACHDATA(rstr_openlpgainidxa44)[] = "openlpgainidxa44";
static const char BCMATTACHDATA(rstr_openlpgainidxa48)[] = "openlpgainidxa48";
static const char BCMATTACHDATA(rstr_openlpgainidxa52)[] = "openlpgainidxa52";
static const char BCMATTACHDATA(rstr_openlpgainidxa56)[] = "openlpgainidxa56";
static const char BCMATTACHDATA(rstr_openlpgainidxa60)[] = "openlpgainidxa60";
static const char BCMATTACHDATA(rstr_openlpgainidxa64)[] = "openlpgainidxa64";
static const char BCMATTACHDATA(rstr_openlpgainidxa100)[] = "openlpgainidxa100";
static const char BCMATTACHDATA(rstr_openlpgainidxa104)[] = "openlpgainidxa104";
static const char BCMATTACHDATA(rstr_openlpgainidxa108)[] = "openlpgainidxa108";
static const char BCMATTACHDATA(rstr_openlpgainidxa112)[] = "openlpgainidxa112";
static const char BCMATTACHDATA(rstr_openlpgainidxa116)[] = "openlpgainidxa116";
static const char BCMATTACHDATA(rstr_openlpgainidxa120)[] = "openlpgainidxa120";
static const char BCMATTACHDATA(rstr_openlpgainidxa124)[] = "openlpgainidxa124";
static const char BCMATTACHDATA(rstr_openlpgainidxa128)[] = "openlpgainidxa128";
static const char BCMATTACHDATA(rstr_openlpgainidxa132)[] = "openlpgainidxa132";
static const char BCMATTACHDATA(rstr_openlpgainidxa136)[] = "openlpgainidxa136";
static const char BCMATTACHDATA(rstr_openlpgainidxa140)[] = "openlpgainidxa140";
static const char BCMATTACHDATA(rstr_openlpgainidxa149)[] = "openlpgainidxa149";
static const char BCMATTACHDATA(rstr_openlpgainidxa153)[] = "openlpgainidxa153";
static const char BCMATTACHDATA(rstr_openlpgainidxa157)[] = "openlpgainidxa157";
static const char BCMATTACHDATA(rstr_openlpgainidxa161)[] = "openlpgainidxa161";
static const char BCMATTACHDATA(rstr_openlpgainidxa165)[] = "openlpgainidxa165";
static const char BCMATTACHDATA(rstr_openlptempcorr)[] = "openlptempcorr";
static const char BCMATTACHDATA(rstr_openlpvoltcorr)[] = "openlpvoltcorr";
static const char BCMATTACHDATA(rstr_openlppwrlim)[] = "openlppwrlim";
static const char BCMATTACHDATA(rstr_hw_iqcal_en)[] = "hw_iqcal_en";
static const char BCMATTACHDATA(rstr_iqcal_swp_dis)[] = "iqcal_swp_dis";
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
static const char BCMATTACHDATA(rstr_cckPwrIdxCorr)[] = "cckPwrIdxCorr";
static const char BCMATTACHDATA(rstr_dacrate2g)[] = "dacrate2g";
static const char BCMATTACHDATA(rstr_rfreg033)[] = "rfreg033";
static const char BCMATTACHDATA(rstr_rfreg033_cck)[] = "rfreg033_cck";
static const char BCMATTACHDATA(rstr_rfreg038)[] = "rfreg038";
static const char BCMATTACHDATA(rstr_pacalidx2g)[] = "pacalidx2g";
static const char BCMATTACHDATA(rstr_pacalidx2g1)[] = "pacalidx2g1";
static const char BCMATTACHDATA(rstr_pacalalim)[] = "pacalalim";
static const char BCMATTACHDATA(rstr_pacalath2g)[] = "pacalath2g";
static const char BCMATTACHDATA(rstr_pacalath2g1)[] = "pacalath2g1";
static const char BCMATTACHDATA(rstr_pacalath5glo)[] = "pacalath5glo";
static const char BCMATTACHDATA(rstr_pacalath5g)[] = "pacalath5g";
static const char BCMATTACHDATA(rstr_pacalath5ghi)[] = "pacalath5ghi";
static const char BCMATTACHDATA(rstr_pacalath5glo1)[] = "pacalath5glo1";
static const char BCMATTACHDATA(rstr_pacalath5g1)[] = "pacalath5g1";
static const char BCMATTACHDATA(rstr_pacalath5ghi1)[] = "pacalath5ghi1";
static const char BCMATTACHDATA(rstr_pacalidx5g)[] = "pacalidx5g";
static const char BCMATTACHDATA(rstr_pacalidx5g1)[] = "pacalidx5g1";
static const char BCMATTACHDATA(rstr_pacalidx2g1th)[] = "pacalidx2g1th";
static const char BCMATTACHDATA(rstr_pacalidx5g1th)[] = "pacalidx5g1th";
static const char BCMATTACHDATA(rstr_pacalidx5glo1th)[] = "pacalidx5glo1th";
static const char BCMATTACHDATA(rstr_pacalidx5ghi1th)[] = "pacalidx5ghi1th";
static const char BCMATTACHDATA(rstr_pacalfcd1)[] = "pacalfcd1";
static const char BCMATTACHDATA(rstr_parfps)[] = "parfps";
static const char BCMATTACHDATA(rstr_swctrlmap_2g)[] = "swctrlmap_2g";
static const char BCMATTACHDATA(rstr_swctrlmap_5g)[] = "swctrlmap_5g";
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
static const char BCMATTACHDATA(rstr_txalpfbyp5g)[] = "txalpfbyp5g";
static const char BCMATTACHDATA(rstr_dacrate5g)[] = "dacrate5g";
static const char BCMATTACHDATA(rstr_ofdmanalogfiltbw5g)[] = "ofdmanalogfiltbw5g";
static const char BCMATTACHDATA(rstr_dacgc2g)[] = "dacgc2g";
static const char BCMATTACHDATA(rstr_gmgc2g)[] = "gmgc2g";
static const char BCMATTACHDATA(rstr_ofdmanalogfiltbw2g)[] = "ofdmanalogfiltbw2g";
static const char BCMATTACHDATA(rstr_txalpfbyp2g)[] = "txalpfbyp2g";
static const char BCMATTACHDATA(rstr_txalpfbyp2g_cck)[] = "txalpfbyp2g_cck";
static const char BCMATTACHDATA(rstr_bphyscale)[] = "bphyscale";
static const char BCMATTACHDATA(rstr_noise_cal_dbg)[] = "noise_cal_dbg";
static const char BCMATTACHDATA(rstr_noise_cal_high_gain)[] = "noise_cal_high_gain";
static const char BCMATTACHDATA(rstr_noise_cal_nf_substract_val)[] = "noise_cal_nf_substract_val";
static const char BCMATTACHDATA(rstr_noise_cal_update)[] = "noise_cal_update";
static const char BCMATTACHDATA(rstr_noise_cal_po_2g)[] = "noise_cal_po_2g";
static const char BCMATTACHDATA(rstr_noise_cal_po_5g)[] = "noise_cal_po_5g";
static const char BCMATTACHDATA(rstr_noise_cal_enable_2g)[] = "noise_cal_enable_2g";
static const char BCMATTACHDATA(rstr_noise_cal_po_bias_2g)[] = "noise_cal_po_bias_2g";
static const char BCMATTACHDATA(rstr_noise_cal_enable_5g)[] = "noise_cal_enable_5g";
static const char BCMATTACHDATA(rstr_noise_cal_po_bias_5g)[] = "noise_cal_po_bias_5g";
static const char BCMATTACHDATA(rstr_noise_cal_ref_2g)[] = "noise_cal_ref_2g";
static const char BCMATTACHDATA(rstr_noise_cal_ref_5g)[] = "noise_cal_ref_5g";
static const char BCMATTACHDATA(rstr_noise_cal_adj_2g)[] = "noise_cal_adj_2g";
static const char BCMATTACHDATA(rstr_noise_cal_adj_5g)[] = "noise_cal_adj_5g";
static const char BCMATTACHDATA(rstr_xtalmode)[] = "xtalmode";
static const char BCMATTACHDATA(rstr_tridx2g)[] = "tridx2g";
static const char BCMATTACHDATA(rstr_triso5g)[] = "triso5g";
static const char BCMATTACHDATA(rstr_tridx5g)[] = "tridx5g";
static const char BCMATTACHDATA(rstr_pacalpulsewidth)[] = "pacalpulsewidth";
static const char BCMATTACHDATA(rstr_plldoubler_enable2g)[] = "plldoubler_enable2g";
static const char BCMATTACHDATA(rstr_plldoubler_enable5g)[] = "plldoubler_enable5g";
static const char BCMATTACHDATA(rstr_spuravoid_enable2g)[] = "spuravoid_enable2g";
static const char BCMATTACHDATA(rstr_spuravoid_enable5g)[] = "spuravoid_enable5g";
static const char BCMATTACHDATA(rstr_iqlocalidx2goffs)[] = "iqlocalidx2goffs";
static const char BCMATTACHDATA(rstr_iqlocalidx5goffs)[] = "iqlocalidx5goffs";
static const char BCMATTACHDATA(rstr_iqlocalidx2g)[] = "iqlocalidx2g";
static const char BCMATTACHDATA(rstr_iqlocalidx5g)[] = "iqlocalidx5g";
static const char BCMATTACHDATA(rstr_iqlocalpwr2g)[] = "iqlocalpwr2g";
static const char BCMATTACHDATA(rstr_iqlocalpwr5g)[] = "iqlocalpwr5g";
static const char BCMATTACHDATA(rstr_iqlost1off2g)[] = "iqlost1off2g";
static const char BCMATTACHDATA(rstr_iqlost1off5g)[] = "iqlost1off5g";
static const char BCMATTACHDATA(rstr_plldoubler_mode2g)[] = "plldoubler_mode2g";
static const char BCMATTACHDATA(rstr_plldoubler_mode5g)[] = "plldoubler_mode5g";
static const char BCMATTACHDATA(rstr_pacalpwr2g)[] = "pacalpwr2g";
static const char BCMATTACHDATA(rstr_pacalpwr2g1)[] = "pacalpwr2g1";
static const char BCMATTACHDATA(rstr_pacalpwr5glo)[] = "pacalpwr5glo";
static const char BCMATTACHDATA(rstr_pacalpwr5glo1)[] = "pacalpwr5glo1";
static const char BCMATTACHDATA(rstr_pacalpwr5g)[] = "pacalpwr5g";
static const char BCMATTACHDATA(rstr_pacalpwr5g1)[] = "pacalpwr5g1";
static const char BCMATTACHDATA(rstr_pacalpwr5ghi)[] = "pacalpwr5ghi";
static const char BCMATTACHDATA(rstr_pacalpwr5ghi1)[] = "pacalpwr5ghi1";
static const char BCMATTACHDATA(rstr_txgaintbl)[] = "txgaintbl";
static const char BCMATTACHDATA(rstr_txgaintbl5g)[] = "txgaintbl5g";
static const char BCMATTACHDATA(rstr_loccmode1)[] = "loccmode1";
static const char BCMATTACHDATA(rstr_txiqlopapu2g)[] = "txiqlopapu2g";
static const char BCMATTACHDATA(rstr_txiqlopag2g)[] = "txiqlopag2g";
static const char BCMATTACHDATA(rstr_txiqlopapu5g)[] = "txiqlopapu5g";
static const char BCMATTACHDATA(rstr_txiqlopag5g)[] = "txiqlopag5g";
static const char BCMATTACHDATA(rstr_logen_mode)[] = "logen_mode";
static const char BCMATTACHDATA(rstr_logen_mode5gx)[] = "logen_mode5g%d";
static const char BCMATTACHDATA(rstr_loidacmode5g)[] = "loidacmode5g";
static const char BCMATTACHDATA(rstr_pacalidx5glo)[] = "pacalidx5glo";
static const char BCMATTACHDATA(rstr_pacalidx5ghi)[] = "pacalidx5ghi";
static const char BCMATTACHDATA(rstr_pacalidx5glo1)[] = "pacalidx5glo1";
static const char BCMATTACHDATA(rstr_pacalidx5ghi1)[] = "pacalidx5ghi1";
static const char BCMATTACHDATA(rstr_rxgaintbl100)[] = "rxgaintbl100";
static const char BCMATTACHDATA(rstr_rxgaintblwlbga)[] = "rxgaintblwlbga";
static const char BCMATTACHDATA(rstr_rxgaintblfcbga)[] = "rxgaintblfcbga";
static const char BCMATTACHDATA(rstr_rxgaintblwlcsp)[] = "rxgaintblwlcsp";
static const char BCMATTACHDATA(rstr_rxgaintblwlbgaelna)[] = "rxgaintblwlbgaelna";
static const char BCMATTACHDATA(rstr_rxgainbackoffval)[] = "rxgainbackoffval";
static const char BCMATTACHDATA(rstr_rfreg088)[] = "rfreg088";
static const char BCMATTACHDATA(rstr_rxgaintempcorr2g)[] = "rxgaintempcorr2g";
static const char BCMATTACHDATA(rstr_rxgaintempcorr5gh)[] = "rxgaintempcorr5gh";
static const char BCMATTACHDATA(rstr_rxgaintempcorr5gm)[] = "rxgaintempcorr5gm";
static const char BCMATTACHDATA(rstr_rxgaintempcorr5gl)[] = "rxgaintempcorr5gl";
static const char BCMATTACHDATA(rstr_unmod_rssi_offset)[] = "unmod_rssi_offset";
static const char BCMATTACHDATA(rstr_adcrfseq2g)[] = "adcrfseq2g";
static const char BCMATTACHDATA(rstr_adcrfseq5g)[] = "adcrfseq5g";
static const char BCMATTACHDATA(rstr_mux_gain_table)[] = "mux_gain_table";
static const char BCMATTACHDATA(rstr_lpoff)[] = "lpoff";
static const char BCMATTACHDATA(rstr_edonthd)[] = "edonthd";
static const char BCMATTACHDATA(rstr_edoffthd)[] = "edoffthd";
static const char BCMATTACHDATA(rstr_tx_tone_power_index)[] = "tx_tone_power_index";
static const char BCMATTACHDATA(rstr_rxgainfreqcorrNd)[] = "rxgainfreqcorr%d";
static const char BCMATTACHDATA(rstr_offtgpwr)[] = "offtgpwr";
static const char BCMATTACHDATA(rstr_alt_gaintbl_5g)[] = "alt_gaintbl_5g";

/* Read band specific data from the SROM */
bool
BCMATTACHFN(wlc_phy_txpwr_srom_read_lcnphy)(phy_info_t *pi, int bandtype)
{
	int i;
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	char str1[20];
	char *val;

#if defined(WLTEST)
	chan_info_2066_lcnphy_lna_corr_t *freq_corr;
#endif // endif

	/* TR switch isolation */
	pi_lcn->lcnphy_tr_isolation_mid = (uint8)PHY_GETINTVAR(pi, rstr_triso2g);

	/* Input power offset */
	pi_lcn->lcnphy_rx_power_offset = (uint8)PHY_GETINTVAR(pi, rstr_rxpo2g);
	/* pa0b0 */
	pi->txpa_2g[0] = (int16)PHY_GETINTVAR(pi, rstr_pa0b0);
	pi->txpa_2g[1] = (int16)PHY_GETINTVAR(pi, rstr_pa0b1);
	pi->txpa_2g[2] = (int16)PHY_GETINTVAR(pi, rstr_pa0b2);

#if TWO_POWER_RANGE_TXPWR_CTRL
	if (PHY_GETVAR(pi, rstr_pa0b0_lo) && PHY_GETINTVAR(pi, rstr_pa0b0_lo))
		pi_lcn->lcnphy_twopwr_txpwrctrl_en = 1;
	if (pi_lcn->lcnphy_twopwr_txpwrctrl_en) {
		pi->txpa_2g_lo[0] = (int16)PHY_GETINTVAR(pi, rstr_pa0b0_lo);
		pi->txpa_2g_lo[1] = (int16)PHY_GETINTVAR(pi, rstr_pa0b1_lo);
		pi->txpa_2g_lo[2] = (int16)PHY_GETINTVAR(pi, rstr_pa0b2_lo);
		pi_lcn->pmin = (int8)PHY_GETINTVAR(pi, rstr_pmin);
		pi_lcn->pmax = (int8)PHY_GETINTVAR(pi, rstr_pmax);
		pi->min_txpower = 0;
	}
#endif // endif

	pi_lcn->lcnphy_tssical_time = (uint32)PHY_GETINTVAR(pi, rstr_tssitime);
	pi_lcn->lcnphy_tssical_txdelay = (uint32)PHY_GETINTVAR_DEFAULT(pi, rstr_tssitxdelay, 20);
	pi_lcn->init_txpwrindex_2g = (uint32)PHY_GETINTVAR_DEFAULT(pi, rstr_initxidx2g,
		LCNPHY_TX_PWR_CTRL_START_INDEX_2G);
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

	pi_lcn->dynamic_pwr_limit_en = (uint32)PHY_GETINTVAR(pi, rstr_dynpwrlimen);
	pi_lcn->tssi_max_npt = (uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_tssimaxnpt,
		LCNPHY_TX_PWR_CTRL_MAX_NPT);

	/* VBATSENSE */
	pi_lcn->lcnphy_vbat_vf = (uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_vbatsmf, 0);

	pi_lcn->lcnphy_vbat_vc = (uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_vbatsmc, 8);

	pi_lcn->lcnphy_vbat_gs = (uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_vbatsav, 2);

	/* TEMPSENSE */
	pi_lcn->lcnphy_temp_vf = (uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_tempsmf, 4);

	pi_lcn->lcnphy_temp_vc = (uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_tempsmc, 8);

	pi_lcn->lcnphy_temp_gs = (uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_tempsav, 2);

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

#if defined(WLTEST)
	pi_lcn->srom_rxgainerr_2g  = (int16)PHY_GETINTVAR(pi, rstr_rxgainerr2g);
	pi_lcn->srom_rxgainerr_5gl = (int16)PHY_GETINTVAR(pi, rstr_rxgainerr5gl);
	pi_lcn->srom_rxgainerr_5gm = (int16)PHY_GETINTVAR(pi, rstr_rxgainerr5gm);
	pi_lcn->srom_rxgainerr_5gh = (int16)PHY_GETINTVAR(pi, rstr_rxgainerr5gh);

	pi_lcn->srom_noiselvl_2g  = (int8)PHY_GETINTVAR(pi, rstr_noiselvl2g);
	pi_lcn->srom_noiselvl_5gl = (int8)PHY_GETINTVAR(pi, rstr_noiselvl5gl);
	pi_lcn->srom_noiselvl_5gm = (int8)PHY_GETINTVAR(pi, rstr_noiselvl5gm);
	pi_lcn->srom_noiselvl_5gh = (int8)PHY_GETINTVAR(pi, rstr_noiselvl5gh);

	pi_lcn->noise_offset_2g = (int16)PHY_GETINTVAR_DEFAULT(pi, rstr_noiseoff2g, -32);
	pi_lcn->noise_offset_5gl = (int16)PHY_GETINTVAR_DEFAULT(pi, rstr_noiseoff5gl, -27);
	pi_lcn->noise_offset_5gm = (int16)PHY_GETINTVAR_DEFAULT(pi, rstr_noiseoff5gm, -27);
	pi_lcn->noise_offset_5gh = (int16)PHY_GETINTVAR_DEFAULT(pi, rstr_noiseoff5gh, -33);

	pi_lcn->pwr_thresh_2g = (uint32)PHY_GETINTVAR_DEFAULT(pi, rstr_pwrthresh2g, 8000);
	pi_lcn->pwr_thresh_5g = (uint32)PHY_GETINTVAR_DEFAULT(pi, rstr_pwrthresh5g, 1000);

	freq_corr = chan_info_2066_lcnphy_lna_corr;
	for (i = 0; i < ARRAYSIZE(chan_info_2066_lcnphy_lna_corr); i++) {
		snprintf(str1, sizeof(str1),
		         rstr_rxgainfreqcorrNd, chan_info_2066_lcnphy_lna_corr[i].chan);
		freq_corr[i].corr_qdBm =
			(int8)PHY_GETINTVAR_DEFAULT(pi, str1, freq_corr[i].corr_qdBm);
	}
#endif /* #if defined(WLTEST) */

	for (i = 0; i < PWRTBL_NUM_COEFF; i++) {
		pi->txpa_2g_low_temp[i] = pi->txpa_2g[i];
		pi->txpa_2g_high_temp[i] = pi->txpa_2g[i];
	}

	wlc_phy_txpwr_sromlcn_read_2g_ppr_parameters(pi);

	/* for tempcompensated tx power control */
	pi_lcn->lcnphy_rawtempsense = (uint16)PHY_GETINTVAR(pi, rstr_rawtempsense);
	pi_lcn->lcnphy_measPower = (uint8)PHY_GETINTVAR(pi, rstr_measpower);
	pi_lcn->lcnphy_measPower1 = (uint8)PHY_GETINTVAR(pi, rstr_measpower1);
	pi_lcn->lcnphy_measPower2 = (uint8)PHY_GETINTVAR(pi, rstr_measpower2);
	pi_lcn->lcnphy_tempsense_slope = (uint8)PHY_GETINTVAR(pi, rstr_tempsense_slope);
	pi_lcn->lcnphy_tempcorrx = (int8)PHY_GETINTVAR(pi, rstr_tempcorrx);
	pi_lcn->lcnphy_tempsense_option = (uint8)PHY_GETINTVAR(pi, rstr_tempsense_option);

	pi_lcn->openlp_pwrctrl  = (uint8)PHY_GETINTVAR(pi, rstr_openlppwrctrl);
	pi_lcn->openlp_refpwrqdB  = (int32)PHY_GETINTVAR(pi, rstr_openlprefpwr);

	for (i = 1; i <= CH_MAX_2G_CHANNEL; i++) {
		snprintf(str1, sizeof(str1), rstr_openlpgainidxbN, i);
		val = getvar(pi->vars, str1);
		if (val)
			pi_lcn->openlp_gainidx_b[i-1] = (uint8)bcm_strtoul(val, NULL, 0);
	}

	pi_lcn->openlp_gainidx_a36      = (uint8)PHY_GETINTVAR(pi, rstr_openlpgainidxa36);
	pi_lcn->openlp_gainidx_a40      = (uint8)PHY_GETINTVAR(pi, rstr_openlpgainidxa40);
	pi_lcn->openlp_gainidx_a44      = (uint8)PHY_GETINTVAR(pi, rstr_openlpgainidxa44);
	pi_lcn->openlp_gainidx_a48      = (uint8)PHY_GETINTVAR(pi, rstr_openlpgainidxa48);
	pi_lcn->openlp_gainidx_a52      = (uint8)PHY_GETINTVAR(pi, rstr_openlpgainidxa52);
	pi_lcn->openlp_gainidx_a56      = (uint8)PHY_GETINTVAR(pi, rstr_openlpgainidxa56);
	pi_lcn->openlp_gainidx_a60      = (uint8)PHY_GETINTVAR(pi, rstr_openlpgainidxa60);
	pi_lcn->openlp_gainidx_a64      = (uint8)PHY_GETINTVAR(pi, rstr_openlpgainidxa64);
	pi_lcn->openlp_gainidx_a100     = (uint8)PHY_GETINTVAR(pi, rstr_openlpgainidxa100);
	pi_lcn->openlp_gainidx_a104     = (uint8)PHY_GETINTVAR(pi, rstr_openlpgainidxa104);
	pi_lcn->openlp_gainidx_a108     = (uint8)PHY_GETINTVAR(pi, rstr_openlpgainidxa108);
	pi_lcn->openlp_gainidx_a112     = (uint8)PHY_GETINTVAR(pi, rstr_openlpgainidxa112);
	pi_lcn->openlp_gainidx_a116     = (uint8)PHY_GETINTVAR(pi, rstr_openlpgainidxa116);
	pi_lcn->openlp_gainidx_a120     = (uint8)PHY_GETINTVAR(pi, rstr_openlpgainidxa120);
	pi_lcn->openlp_gainidx_a124     = (uint8)PHY_GETINTVAR(pi, rstr_openlpgainidxa124);
	pi_lcn->openlp_gainidx_a128     = (uint8)PHY_GETINTVAR(pi, rstr_openlpgainidxa128);
	pi_lcn->openlp_gainidx_a132     = (uint8)PHY_GETINTVAR(pi, rstr_openlpgainidxa132);
	pi_lcn->openlp_gainidx_a136     = (uint8)PHY_GETINTVAR(pi, rstr_openlpgainidxa136);
	pi_lcn->openlp_gainidx_a140     = (uint8)PHY_GETINTVAR(pi, rstr_openlpgainidxa140);
	pi_lcn->openlp_gainidx_a149     = (uint8)PHY_GETINTVAR(pi, rstr_openlpgainidxa149);
	pi_lcn->openlp_gainidx_a153     = (uint8)PHY_GETINTVAR(pi, rstr_openlpgainidxa153);
	pi_lcn->openlp_gainidx_a157     = (uint8)PHY_GETINTVAR(pi, rstr_openlpgainidxa157);
	pi_lcn->openlp_gainidx_a161     = (uint8)PHY_GETINTVAR(pi, rstr_openlpgainidxa161);
	pi_lcn->openlp_gainidx_a165     = (uint8)PHY_GETINTVAR(pi, rstr_openlpgainidxa165);

	pi_lcn->openlp_tempcorr = (uint16) PHY_GETINTVAR(pi, rstr_openlptempcorr);
	pi_lcn->openlp_voltcorr = (uint16) PHY_GETINTVAR(pi, rstr_openlpvoltcorr);

	pi_lcn->openlp_pwrlimqdB = (int32) PHY_GETINTVAR_DEFAULT(pi, rstr_openlppwrlim, 1<<31);

	pi_lcn->lcnphy_hw_iqcal_en = (bool)PHY_GETINTVAR(pi, rstr_hw_iqcal_en);
	pi_lcn->lcnphy_iqcal_swp_dis = (bool)PHY_GETINTVAR(pi, rstr_iqcal_swp_dis);

	pi_lcn->lcnphy_freqoffset_corr = (uint8)PHY_GETINTVAR(pi, rstr_freqoffset_corr);
	pi->aa2g = (uint8) PHY_GETINTVAR(pi, rstr_aa2g);
	pi_lcn->extpagain2g = (uint8)PHY_GETINTVAR(pi, rstr_extpagain2g);
	pi_lcn->extpagain5g = (uint8)PHY_GETINTVAR(pi, rstr_extpagain5g);
	pi_lcn->txpwrindex_nvram = (uint8)PHY_GETINTVAR(pi, rstr_txpwrindex);

	if (pi->aa2g >= 1)
		wlc_lcnphy_decode_aa2g((wlc_phy_t *)pi, pi->aa2g);

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

	/* pa gain override */
	pi_lcn->pa_gain_ovr_val_2g = (int16)PHY_GETINTVAR_DEFAULT(pi, rstr_pagc2g, -1);
	pi_lcn->pa_gain_ovr_val_5g = (int16)PHY_GETINTVAR_DEFAULT(pi, rstr_pagc5g, -1);
	pi_lcn->lcnphy_tx_iqlo_tone_freq_ovr_val =  (int16)PHY_GETINTVAR(pi, rstr_txiqlotf);

	/* Coefficients for Temperature Conversion to Centigrade */
	pi_lcn->temp_mult =
	        (int32)PHY_GETINTVAR_DEFAULT(pi, rstr_temp_mult, 414); /* .4043 << 10 */
	pi_lcn->temp_add =
	        (int32)PHY_GETINTVAR_DEFAULT(pi, rstr_temp_add, 32410); /*  31.657 << 10 */
	pi_lcn->temp_q =
	        (int32)PHY_GETINTVAR_DEFAULT(pi, rstr_temp_q, 10);

	/* Coefficients for vbat conversion to Volts */
	pi_lcn->vbat_mult =
	        (int32)PHY_GETINTVAR_DEFAULT(pi, rstr_vbat_mult, 8);  /* .008 << 10 */
	pi_lcn->vbat_add =
	        (int32)PHY_GETINTVAR_DEFAULT(pi, rstr_vbat_add, 4206); /*  4.1072 << 10 */
	pi_lcn->vbat_q =
	        (int32)PHY_GETINTVAR_DEFAULT(pi, rstr_vbat_q, 10);

	/* Offset for the CCK power detector */
	pi_lcn->cckPwrOffset = (int16)PHY_GETINTVAR(pi, rstr_cckPwrOffset);

	/* CCK Power Index Correction */
	pi_lcn->cckPwrIdxCorr = (int16)PHY_GETINTVAR(pi, rstr_cckPwrIdxCorr);
#if TWO_POWER_RANGE_TXPWR_CTRL
	if ((pi_lcn->lcnphy_twopwr_txpwrctrl_en) && (pi_lcn->cckPwrIdxCorr > 0))
		pi_lcn->cckPwrIdxCorr = 0;
#endif // endif

	/* DAC rate */
	pi_lcn->dacrate_2g = (uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_dacrate2g, 80);

	/* rfreg033 value */
	pi_lcn->rfreg033 = (uint8)PHY_GETINTVAR(pi, rstr_rfreg033);

	pi_lcn->rfreg033_cck = (uint8)PHY_GETINTVAR(pi, rstr_rfreg033_cck);

	/* rfreg038 value */
	pi_lcn->rfreg038 = (uint8)PHY_GETINTVAR(pi, rstr_rfreg038);

	/* PA Cal Idx 2g */
	pi_lcn->pacalidx_2g = (int8)PHY_GETINTVAR_DEFAULT(pi, rstr_pacalidx2g, -1);

	/* PA Cal Idx 2g1 */
	pi_lcn->pacalidx_2g1 = (int8)PHY_GETINTVAR_DEFAULT(pi, rstr_pacalidx2g1, -1);

	/* Default value for forced papd cal index */
	pi_lcn->pacalidx = -1;

	/* PA Cal Idx 5g */
	pi_lcn->pacalidx_5g = (int8)PHY_GETINTVAR_DEFAULT(pi, rstr_pacalidx5g, -1);

	/* PA Cal Idx 5g for lower band */
	pi_lcn->pacalidx_5glo = (int8)PHY_GETINTVAR_DEFAULT(pi, rstr_pacalidx5glo, -1);

	/* PA Cal Idx 5g for higher band */
	pi_lcn->pacalidx_5ghi = (int8)PHY_GETINTVAR_DEFAULT(pi, rstr_pacalidx5ghi, -1);

	/* PA Cal Idx 5g1 */
	pi_lcn->pacalidx_5g1 = (int8)PHY_GETINTVAR_DEFAULT(pi, rstr_pacalidx5g1, -1);

	/* PA Cal Idx 5g for lower band */
	pi_lcn->pacalidx_5glo1 = (int8)PHY_GETINTVAR_DEFAULT(pi, rstr_pacalidx5glo1, -1);

	/* PA Cal Idx 5g for higher band */
	pi_lcn->pacalidx_5ghi1 = (int8)PHY_GETINTVAR_DEFAULT(pi, rstr_pacalidx5ghi1, -1);

	/* If pacalalim==1, only reduce gain for papd cal, but never increase it */
	pi_lcn->pacalalim = (int8)PHY_GETINTVAR_DEFAULT(pi, rstr_pacalalim, -1);

	/* Backoff rx gain if AMAM is above threshold */
	pi_lcn->pacalfcd1 = (int8)PHY_GETINTVAR_DEFAULT(pi, rstr_pacalfcd1, 0);

	/* PA cal amam threshold 2g */
	pi_lcn->pacalamamth_2g = (int16)PHY_GETINTVAR_DEFAULT(pi, rstr_pacalath2g, -1);

	/* PA cal amam threshold 2g1 */
	pi_lcn->pacalamamth_2g1 = (int16)PHY_GETINTVAR_DEFAULT(pi, rstr_pacalath2g1, -1);

	/* PA cal amam threshold 5g lo */
	pi_lcn->pacalamamth_5glo = (int16)PHY_GETINTVAR_DEFAULT(pi, rstr_pacalath5glo, -1);

	/* PA cal amam threshold 5g */
	pi_lcn->pacalamamth_5g = (int16)PHY_GETINTVAR_DEFAULT(pi, rstr_pacalath5g, -1);

	/* PA cal amam threshold 5g hi */
	pi_lcn->pacalamamth_5ghi = (int16)PHY_GETINTVAR_DEFAULT(pi, rstr_pacalath5ghi, -1);

	/* PA cal amam threshold 5g lo1 */
	pi_lcn->pacalamamth_5glo1 = (int16)PHY_GETINTVAR_DEFAULT(pi, rstr_pacalath5glo1, -1);

	/* PA cal amam threshold 5g1 */
	pi_lcn->pacalamamth_5g1 = (int16)PHY_GETINTVAR_DEFAULT(pi, rstr_pacalath5g1, -1);

	/* PA cal amam threshold 5g hi1 */
	pi_lcn->pacalamamth_5ghi1 = (int16)PHY_GETINTVAR_DEFAULT(pi, rstr_pacalath5ghi1, -1);

	/* dual lut threshold 2g */
	pi_lcn->pacalidx2g1th = (int16)PHY_GETINTVAR_DEFAULT(pi, rstr_pacalidx2g1th, 0);

	/* dual lut threshold 5glo */
	pi_lcn->pacalidx5glo1th = (int16)PHY_GETINTVAR_DEFAULT(pi, rstr_pacalidx5glo1th, 0);

	/* dual lut threshold 5g */
	pi_lcn->pacalidx5g1th = (int16)PHY_GETINTVAR_DEFAULT(pi, rstr_pacalidx5g1th, 0);

	/* dual lut threshold 5ghi */
	pi_lcn->pacalidx5ghi1th = (int16)PHY_GETINTVAR_DEFAULT(pi, rstr_pacalidx5ghi1th, 0);

	pi_lcn->papd_rf_pwr_scale = (uint8)PHY_GETINTVAR(pi, rstr_parfps);

	/* Switch control params */
	if (CHIPID(pi->sh->chip) == BCM4313_CHIP_ID) {
		uint32 ePA = BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_FEM;
		if (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_FEM_BT) { /* combo boards */
			if (!ePA) {
				pi_lcn->swctrlmap_2g[LCNPHY_I_WL_TX] = 0x06050605;
				pi_lcn->swctrlmap_2g[LCNPHY_I_WL_RX] = 0x0a090a09;
				pi_lcn->swctrlmap_2g[LCNPHY_I_WL_RX_ATTN] = 0x06050605;
				pi_lcn->swctrlmap_2g[LCNPHY_I_BT] = 0;
				pi_lcn->swctrlmap_2g[LCNPHY_I_WL_MASK] = 0x2ff;
			} else {
				if (pi->sh->boardrev < 0x1250) {
					pi_lcn->swctrlmap_2g[LCNPHY_I_WL_TX] = 0x05070507;
					pi_lcn->swctrlmap_2g[LCNPHY_I_WL_RX] = 0x04060406;
					pi_lcn->swctrlmap_2g[LCNPHY_I_WL_RX_ATTN] = 0x05070507;
					pi_lcn->swctrlmap_2g[LCNPHY_I_BT] = 0;
					pi_lcn->swctrlmap_2g[LCNPHY_I_WL_MASK] = 0x2ff;
				} else if (pi->aa2g > 2) {
					pi_lcn->swctrlmap_2g[LCNPHY_I_WL_TX] = 0x05070507;
					pi_lcn->swctrlmap_2g[LCNPHY_I_WL_RX] = 0x00020002;
					pi_lcn->swctrlmap_2g[LCNPHY_I_WL_RX_ATTN] = 0x05070507;
					pi_lcn->swctrlmap_2g[LCNPHY_I_BT] = 0x0;
					pi_lcn->swctrlmap_2g[LCNPHY_I_WL_MASK] = 0x2ff;
				} else {
					pi_lcn->swctrlmap_2g[LCNPHY_I_WL_TX] = 0x05050505;
					pi_lcn->swctrlmap_2g[LCNPHY_I_WL_RX] = 0;
					pi_lcn->swctrlmap_2g[LCNPHY_I_WL_RX_ATTN] = 0x05050505;
					pi_lcn->swctrlmap_2g[LCNPHY_I_BT] = 0;
					pi_lcn->swctrlmap_2g[LCNPHY_I_WL_MASK] = 0x0ff;
				}
			}
		} else { /* non-combo boards */
			if (!ePA) {
				pi_lcn->swctrlmap_2g[LCNPHY_I_WL_TX] = 0x0a090a09;
				pi_lcn->swctrlmap_2g[LCNPHY_I_WL_RX] = 0x06050605;
				pi_lcn->swctrlmap_2g[LCNPHY_I_WL_RX_ATTN] = 0x0a090a09;
				pi_lcn->swctrlmap_2g[LCNPHY_I_BT] = 0;
				pi_lcn->swctrlmap_2g[LCNPHY_I_WL_MASK] = 0x0ff;
			} else {
				if (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA) {
					/* board has ext LNA in 2.4Ghz band */
					pi_lcn->swctrlmap_2g[LCNPHY_I_WL_TX] = 0x06060606;
					pi_lcn->swctrlmap_2g[LCNPHY_I_WL_RX] = 0x01010101;
					pi_lcn->swctrlmap_2g[LCNPHY_I_WL_RX_ATTN] = 0x02020202;
					pi_lcn->swctrlmap_2g[LCNPHY_I_BT] = 0;
					pi_lcn->swctrlmap_2g[LCNPHY_I_WL_MASK] = 0x0ff;
				}
				else
				{
					pi_lcn->swctrlmap_2g[LCNPHY_I_WL_TX] = 0x08020802;
					pi_lcn->swctrlmap_2g[LCNPHY_I_WL_RX] = 0x01040104;
					pi_lcn->swctrlmap_2g[LCNPHY_I_WL_RX_ATTN] = 0x08020802;
					pi_lcn->swctrlmap_2g[LCNPHY_I_BT] = 0;
					pi_lcn->swctrlmap_2g[LCNPHY_I_WL_MASK] = 0x0ff;
				}
			}
		}
	} else {
		if (bandtype == WLC_BAND_2G) {
			if (PHY_GETVAR(pi, rstr_swctrlmap_2g)) {
				for (i = 0; i < LCNPHY_SWCTRL_NVRAM_PARAMS; i++) {
					pi_lcn->swctrlmap_2g[i] =
						(uint32)PHY_GETINTVAR_ARRAY(pi,
					                                    rstr_swctrlmap_2g, i);
				}
			} else {
				PHY_ERROR(("Switch control map(swctrlmap_2g) is NOT found"
					"in the NVRAM file %s\n", __FUNCTION__));
				return FALSE;
			}
		} else if (bandtype == WLC_BAND_5G) {
			if (PHY_GETVAR(pi, rstr_swctrlmap_5g)) {
				for (i = 0; i < LCNPHY_SWCTRL_NVRAM_PARAMS; i++) {
					pi_lcn->swctrlmap_5g[i] =
						(uint32)PHY_GETINTVAR_ARRAY(pi,
					                                    rstr_swctrlmap_5g, i);
				}
			} else {
				PHY_ERROR(("Switch control map(swctrlmap_5g) is NOT found"
					"in the NVRAM file %s\n", __FUNCTION__));
				return FALSE;
			}
		}
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
	/* The below logic assumes that "extpagain5g" parameter is already parsed */
	pi->tx_alpf_bypass_5g = (int16) PHY_GETINTVAR_DEFAULT(pi, rstr_txalpfbyp5g,
		!(pi_lcn->extpagain5g == 2));

	/* DAC rate */
	pi_lcn->dacrate_5g = (uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_dacrate5g, pi_lcn->dacrate_2g);
	if (pi_lcn->dacrate_5g != pi_lcn->dacrate_2g) {
		PHY_ERROR(("wl%d: %s: Unsupported 5G DAC rate, using %d MHz\n",
			pi->sh->unit, __FUNCTION__, pi_lcn->dacrate_2g));
		pi_lcn->dacrate_5g = pi_lcn->dacrate_2g;
	}
	pi_lcn->dacrate = pi_lcn->dacrate_2g;

	/* ofdm analog filt bw */
	pi->ofdm_analog_filt_bw_override_5g =
		(int16)PHY_GETINTVAR_DEFAULT(pi, rstr_ofdmanalogfiltbw5g, -1);
	pi_lcn->init_txpwrindex_5g = (uint32)PHY_GETINTVAR_DEFAULT(pi, rstr_initxidx5g,
		LCNPHY_TX_PWR_CTRL_START_INDEX_5G);
#endif /* BAND5G */

	pi_lcn->dacgc2g = (int16)PHY_GETINTVAR_DEFAULT(pi, rstr_dacgc2g, -1);
	pi_lcn->gmgc2g = (int16)PHY_GETINTVAR_DEFAULT(pi, rstr_gmgc2g, -1);

	/* ofdm analog filt bw */
	pi->ofdm_analog_filt_bw_override_2g =
		(int16)PHY_GETINTVAR_DEFAULT(pi, rstr_ofdmanalogfiltbw2g, -1);

	if (PHY_GETVAR(pi, rstr_txalpfbyp2g)) {
		pi->tx_alpf_bypass_2g = (int16)PHY_GETINTVAR(pi, rstr_txalpfbyp2g);
	}

	pi->tx_alpf_bypass_cck_2g =
		(int16)PHY_GETINTVAR_DEFAULT(pi, rstr_txalpfbyp2g_cck, -1);

	if (PHY_GETVAR(pi, rstr_bphyscale)) {
		pi->bphy_scale = (int16)PHY_GETINTVAR(pi, rstr_bphyscale);
	}

	pi_lcn->noise.nvram_dbg_noise = (bool)PHY_GETINTVAR_DEFAULT(pi, rstr_noise_cal_dbg, FALSE);

	if (pi_lcn->noise.nvram_dbg_noise) {
	  pi_lcn->noise.nvram_high_gain = (int)PHY_GETINTVAR(pi, rstr_noise_cal_high_gain);
	  pi_lcn->noise.nvram_nf_substract_val =
	    (int)PHY_GETINTVAR(pi, rstr_noise_cal_nf_substract_val);
	}

	if (PHY_GETVAR(pi, rstr_noise_cal_update) &&
		(getintvararraysize(pi->vars, rstr_noise_cal_update) ==
		(3*k_noise_cal_update_steps))) {
		for (i = 0; i < (k_noise_cal_update_steps); i++) {
			pi_lcn->noise.update_ucode_interval[i] =
				(uint8) getintvararray(pi->vars, rstr_noise_cal_update, 3*i);
			pi_lcn->noise.update_data_interval[i] =
				(uint8) getintvararray(pi->vars, rstr_noise_cal_update, (3*i+1));
			pi_lcn->noise.update_step_interval[i] =
				(uint8) getintvararray(pi->vars, rstr_noise_cal_update, (3*i+2));
		}
	} else {
		pi_lcn->noise.update_ucode_interval[0] = 1;
		pi_lcn->noise.update_ucode_interval[1] = 50;
		pi_lcn->noise.update_data_interval[0] = 64;
		pi_lcn->noise.update_data_interval[1] = 64;
		pi_lcn->noise.update_step_interval[0] = 4;
		pi_lcn->noise.update_step_interval[1] = 255;
	}
	pi_lcn->noise.nvram_input_pwr_offset_2g =
		(int16)PHY_GETINTVAR_DEFAULT(pi, rstr_noise_cal_po_2g, 0xff);
	pi_lcn->noise.nvram_input_pwr_offset_5g[0] = 0xff;
	pi_lcn->noise.nvram_input_pwr_offset_5g[1] = 0xff;
	pi_lcn->noise.nvram_input_pwr_offset_5g[2] = 0xff;
	if (PHY_GETVAR(pi, rstr_noise_cal_po_5g)) {
		int k = getintvararraysize(pi->vars, rstr_noise_cal_po_5g);
		i = 0;
		if (k > 3)
			k = 3;
		while (i < k) {
			pi_lcn->noise.nvram_input_pwr_offset_5g[i] =
			        (int16) PHY_GETINTVAR_ARRAY(pi, rstr_noise_cal_po_5g, i);
			i++;
		}
		while (i < 3) {
			pi_lcn->noise.nvram_input_pwr_offset_5g[i++] =
			        pi_lcn->noise.nvram_input_pwr_offset_5g[0];
		}
	}
	pi_lcn->noise.nvram_enable_2g = (bool)PHY_GETINTVAR_DEFAULT(pi, rstr_noise_cal_enable_2g,
		TRUE);
	pi_lcn->noise.nvram_po_bias_2g = (int8)PHY_GETINTVAR(pi, rstr_noise_cal_po_bias_2g);
	pi_lcn->noise.nvram_enable_5g = (bool)PHY_GETINTVAR_DEFAULT(pi, rstr_noise_cal_enable_5g,
		TRUE);
	pi_lcn->noise.nvram_po_bias_5g = (int8)PHY_GETINTVAR(pi, rstr_noise_cal_po_bias_5g);
	pi_lcn->noise.nvram_ref_2g = (int8)PHY_GETINTVAR_DEFAULT(pi, rstr_noise_cal_ref_2g, -1);
	pi_lcn->noise.nvram_ref_5g = (int8)PHY_GETINTVAR_DEFAULT(pi, rstr_noise_cal_ref_5g, -1);
	pi_lcn->noise.nvram_gain_tbl_adj_2g =
		(int8)PHY_GETINTVAR_DEFAULT(pi, rstr_noise_cal_adj_2g, -1);
	pi_lcn->noise.nvram_gain_tbl_adj_5g =
		(int8)PHY_GETINTVAR_DEFAULT(pi, rstr_noise_cal_adj_5g, -1);

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

	pi_lcn->triso2g = (uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_triso2g, 0xff);
	pi_lcn->tridx2g = (uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_tridx2g, 0xff);
#ifdef BAND5G
	pi_lcn->triso5g[0] = 0xff;
	pi_lcn->triso5g[1] = 0xff;
	pi_lcn->triso5g[2] = 0xff;
	if (PHY_GETVAR(pi, rstr_triso5g)) {
		int k = getintvararraysize(pi->vars, rstr_triso5g);
		i = 0;
		if (k > 3)
			k = 3;
		while (i < k) {
			pi_lcn->triso5g[i] = (uint8) PHY_GETINTVAR_ARRAY(pi, rstr_triso5g, i);
			i++;
		}
		while (i < 3) {
			pi_lcn->triso5g[i++] = pi_lcn->triso5g[0];
		}
	}
	pi_lcn->tridx5g = (uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_tridx5g, 0xff);
#endif /* #ifdef BAND5G */

	pi_lcn->papd_corr_norm = (uint16)PHY_GETINTVAR(pi, rstr_pacalpulsewidth);
	pi_lcn->rfpll_doubler_2g =
	        (bool) PHY_GETINTVAR_DEFAULT(pi, rstr_plldoubler_enable2g, FALSE);

#ifdef BAND5G
	pi_lcn->rfpll_doubler_5g =
	        (bool) PHY_GETINTVAR_DEFAULT(pi, rstr_plldoubler_enable5g, TRUE);
#endif // endif

	pi_lcn->spuravoid_2g = (bool) PHY_GETINTVAR_DEFAULT(pi, rstr_spuravoid_enable2g, FALSE);
	if (!CST4330_CHIPMODE_SDIOD(pi->sh->sih->chipst)) {
		if (pi_lcn->spuravoid_2g) {
			PHY_ERROR(("wl%d: %s: 2G spur avoidance not supported\n",
				pi->sh->unit, __FUNCTION__));
			pi_lcn->spuravoid_2g = FALSE;
		}
	}

#ifdef BAND5G
	pi_lcn->spuravoid_5g = (bool) PHY_GETINTVAR_DEFAULT(pi, rstr_spuravoid_enable5g, FALSE);
	if (!CST4330_CHIPMODE_SDIOD(pi->sh->sih->chipst)) {
		if (pi_lcn->spuravoid_5g) {
			PHY_ERROR(("wl%d: %s: 5G spur avoidance not supported\n",
				pi->sh->unit, __FUNCTION__));
			pi_lcn->spuravoid_5g = FALSE;
		}
	}
#endif // endif

	/* IQLO Cal Idx Backoff 2g */
	pi_lcn->iqlocalidx2goffs = (int8)PHY_GETINTVAR_DEFAULT(pi, rstr_iqlocalidx2goffs, 0);

	/* IQLO Cal Idx Backoff 5g */
	pi_lcn->iqlocalidx5goffs = (int8)PHY_GETINTVAR_DEFAULT(pi, rstr_iqlocalidx5goffs, 0);

	/* IQLO Cal Idx 2g */
	pi_lcn->iqlocalidx_2g = (int8)PHY_GETINTVAR_DEFAULT(pi, rstr_iqlocalidx2g, -1);

	/* tx iqlo cal coarse loft cal gain index offset 2g */
	pi_lcn->iqlocalst1off_2g = (int8)PHY_GETINTVAR_DEFAULT(pi, rstr_iqlost1off2g, 0);

#ifdef BAND5G
	/* IQLO Cal Idx 5g */
	pi_lcn->iqlocalidx_5g = (int8)PHY_GETINTVAR_DEFAULT(pi, rstr_iqlocalidx5g, -1);
#endif // endif

	/* IQLO Cal power 2g */
	pi_lcn->iqlocalpwr_2g = (int16)PHY_GETINTVAR_DEFAULT(pi, rstr_iqlocalpwr2g, -1);

#ifdef BAND5G
	/* IQLO Cal power 5g */
	pi_lcn->iqlocalpwr_5g = (int16)PHY_GETINTVAR_DEFAULT(pi, rstr_iqlocalpwr5g, -1);

	/* tx iqlo cal coarse loft cal gain index offset 5g */
	pi_lcn->iqlocalst1off_5g = (int8)PHY_GETINTVAR_DEFAULT(pi, rstr_iqlost1off5g, 0);
#endif // endif

	pi_lcn->rfpll_doubler_mode2g =
	        (bool) PHY_GETINTVAR_DEFAULT(pi, rstr_plldoubler_mode2g, TRUE);

#ifdef BAND5G
	pi_lcn->rfpll_doubler_mode5g =
	        (bool) PHY_GETINTVAR_DEFAULT(pi, rstr_plldoubler_mode5g, TRUE);
#endif // endif

	/* PA Cal power 2g */
	pi_lcn->pacalpwr_2g = (int16)PHY_GETINTVAR_DEFAULT(pi, rstr_pacalpwr2g, -1);

	pi_lcn->pacalpwr_2g1 = (int16)PHY_GETINTVAR_DEFAULT(pi, rstr_pacalpwr2g1, -1);
#ifdef BAND5G
	/* PA Cal power 5g */
	pi_lcn->pacalpwr_5glo = (int16)PHY_GETINTVAR_DEFAULT(pi, rstr_pacalpwr5glo, -1);
	pi_lcn->pacalpwr_5glo1 = (int16)PHY_GETINTVAR_DEFAULT(pi, rstr_pacalpwr5glo1, -1);
	pi_lcn->pacalpwr_5g = (int16)PHY_GETINTVAR_DEFAULT(pi, rstr_pacalpwr5g, -1);
	pi_lcn->pacalpwr_5g1 = (int16)PHY_GETINTVAR_DEFAULT(pi, rstr_pacalpwr5g1, -1);
	pi_lcn->pacalpwr_5ghi = (int16)PHY_GETINTVAR_DEFAULT(pi, rstr_pacalpwr5ghi, -1);
	pi_lcn->pacalpwr_5ghi1 = (int16)PHY_GETINTVAR_DEFAULT(pi, rstr_pacalpwr5ghi1, -1);
#endif // endif

	pi_lcn->txgaintbl = (uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_txgaintbl, 0);
	ASSERT(TXGAINTBL(pi_lcn) == pi_lcn->txgaintbl);

	pi_lcn->txgaintbl5g = (bool)PHY_GETINTVAR_DEFAULT(pi, rstr_txgaintbl5g, FALSE);
	ASSERT(TXGAINTBL5G(pi_lcn) == pi_lcn->txgaintbl5g);

	pi_lcn->loccmode1 = (int8)PHY_GETINTVAR_DEFAULT(pi, rstr_loccmode1, -1);

	if (PHY_GETVAR(pi, rstr_txiqlopapu2g))
		pi_lcn->txiqlopapu_2g = (bool)PHY_GETINTVAR(pi, rstr_txiqlopapu2g);
	else
		pi_lcn->txiqlopapu_2g = -1;

	if (PHY_GETVAR(pi, rstr_txiqlopag2g))
		pi_lcn->txiqlopag_2g = (uint16)PHY_GETINTVAR(pi, rstr_txiqlopag2g);
	else
		pi_lcn->txiqlopag_2g = -1;

	pi_lcn->adcrfseq_2g = (int16)PHY_GETINTVAR_DEFAULT(pi, rstr_adcrfseq2g, -1);
#ifdef BAND5G
	if (PHY_GETVAR(pi, rstr_txiqlopapu5g))
		pi_lcn->txiqlopapu_5g = (bool)PHY_GETINTVAR(pi, rstr_txiqlopapu5g);
	else
		pi_lcn->txiqlopapu_5g = -1;

	if (PHY_GETVAR(pi, rstr_txiqlopag5g))
		pi_lcn->txiqlopag_5g = (uint16)PHY_GETINTVAR(pi, rstr_txiqlopag5g);
	else
		pi_lcn->txiqlopag_5g = -1;

	pi_lcn->adcrfseq_5g = (int16)PHY_GETINTVAR_DEFAULT(pi, rstr_adcrfseq5g, -1);
#endif // endif

	if (PHY_GETVAR(pi, rstr_logen_mode)) {
		pi_lcn->logen_mode[0] = 1;
		pi_lcn->logen_mode[1] =
			(uint8) PHY_GETINTVAR_ARRAY(pi, rstr_logen_mode, 0);
		pi_lcn->logen_mode[2] =
			(uint8) PHY_GETINTVAR_ARRAY(pi, rstr_logen_mode, 1);
		pi_lcn->logen_mode[3] =
			(uint8) PHY_GETINTVAR_ARRAY(pi, rstr_logen_mode, 2);
		pi_lcn->logen_mode[4] =
			(uint8) PHY_GETINTVAR_ARRAY(pi, rstr_logen_mode, 3);
		pi_lcn->logen_mode[5] =
			(uint8) PHY_GETINTVAR_ARRAY(pi, rstr_logen_mode, 4);
	} else {
		pi_lcn->logen_mode[0] = 0;
	}

#ifdef BAND5G
	for (i = 0; i <= 4; i++) {
		sprintf(str1, rstr_logen_mode5gx, i);
		if (PHY_GETVAR(pi, str1)) {
			pi_lcn->logen_mode5g[i][0] = 1;
			pi_lcn->logen_mode5g[i][1] =
				(uint8) PHY_GETINTVAR_ARRAY(pi, str1, 0);
			pi_lcn->logen_mode5g[i][2] =
				(uint8) PHY_GETINTVAR_ARRAY(pi, str1, 1);
			pi_lcn->logen_mode5g[i][3] =
				(uint8) PHY_GETINTVAR_ARRAY(pi, str1, 2);
			pi_lcn->logen_mode5g[i][4] =
				(uint8) PHY_GETINTVAR_ARRAY(pi, str1, 3);
		} else {
			pi_lcn->logen_mode5g[i][0] = 0;
		}
	}
	pi_lcn->alt_gaintbl_5g = (uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_alt_gaintbl_5g, 0);
#endif /* BAND5G */

	pi_lcn->loidacmode_5g = (int8)PHY_GETINTVAR_DEFAULT(pi, rstr_loidacmode5g, -1);

	/* PA Cal Idx 5g for lower band */
	pi_lcn->pacalidx_5glo = (int8)PHY_GETINTVAR_DEFAULT(pi, rstr_pacalidx5glo, -1);
	/* PA Cal Idx 5g for higher band */
	pi_lcn->pacalidx_5ghi = (int8)PHY_GETINTVAR_DEFAULT(pi, rstr_pacalidx5ghi, -1);

	/* Select option for elna rx gain tbl 100 */
	pi_lcn->rxgaintbl_elna_100 = (bool)PHY_GETINTVAR_DEFAULT(pi, rstr_rxgaintbl100, 0);

	/* Select option for ilna rx gain tbl wlbga aci */
	pi_lcn->rxgaintbl_wlbga_aci = (uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_rxgaintblwlbga, 0);

	/* Select option for ilna rx gain tbl FCBGA or CSP */
	pi_lcn->rxgaintbl_fcbga = (uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_rxgaintblfcbga, 0);

	/* Select option for ilna rx CSP gain tbl 1 */
	pi_lcn->rxgaintbl_wlcsp = (uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_rxgaintblwlcsp, 0);

	/* Select option for elna ACI optimized WLBGA gain table */
	pi_lcn->rxgaintbl_wlbga_elna_aci = (uint8)PHY_GETINTVAR_DEFAULT(pi,
		rstr_rxgaintblwlbgaelna, 0);
	/* genGainBkOff value */
	pi_lcn->rxgain_backoff_val = (uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_rxgainbackoffval, 3);

	/* rfreg088 value */
	pi_lcn->rfreg088 = (uint8)PHY_GETINTVAR(pi, rstr_rfreg088);

	/* Below is the rx gain change rate per degree C multiplied by 1000 */
	pi_lcn->rxgain_tempcorr_2g = (uint16)PHY_GETINTVAR(pi, rstr_rxgaintempcorr2g);
	pi_lcn->rxgain_tempcorr_5gh = (uint16)PHY_GETINTVAR(pi, rstr_rxgaintempcorr5gh);
	pi_lcn->rxgain_tempcorr_5gm = (uint16)PHY_GETINTVAR(pi, rstr_rxgaintempcorr5gm);
	pi_lcn->rxgain_tempcorr_5gl = (uint16)PHY_GETINTVAR(pi, rstr_rxgaintempcorr5gl);

	pi_lcn->unmod_rssi_offset = (int32)PHY_GETINTVAR(pi, rstr_unmod_rssi_offset);

	pi_lcn->mux_gain_table = (bool)PHY_GETINTVAR_DEFAULT(pi, rstr_mux_gain_table, FALSE);

#ifdef LP_P2P_SOFTAP
	pi_lcn->pwr_offset_val = (int16)PHY_GETINTVAR_DEFAULT(pi, rstr_lpoff, 0);
#endif // endif
	pi_lcn->edonthreshold = (bool)PHY_GETINTVAR_DEFAULT(pi, rstr_edonthd, -70);
	pi_lcn->edoffthreshold = (bool)PHY_GETINTVAR_DEFAULT(pi, rstr_edoffthd, -76);

	return TRUE;
}

void
wlc_phy_txpower_sromlimit_get_lcnphy(phy_info_t *pi, uint channel, ppr_t *max_pwr, uint8 core)
{
	srom_pwrdet_t *pwrdet  = pi->pwrdet;
	uint8 band;

	band = wlc_phy_get_band_from_channel(pi, channel);

	wlc_phy_txpwr_apply_sromlcn(pi, band, max_pwr);

	switch (band) {
	case WL_CHAN_FREQ_RANGE_2G:
	case WL_CHAN_FREQ_RANGE_5G_BAND0:
	case WL_CHAN_FREQ_RANGE_5G_BAND1:
	case WL_CHAN_FREQ_RANGE_5G_BAND2:
	case WL_CHAN_FREQ_RANGE_5G_BAND3:
		ppr_apply_max(max_pwr, pwrdet->max_pwr[core][band]);
		break;
	}

	if (0) {
		PHY_ERROR(("####band #%d######\n", band));
		ppr_dsss_printf(max_pwr);
		ppr_ofdm_printf(max_pwr);
		ppr_mcs_printf(max_pwr);
	}
}

static void
wlc_phy_txpwr_sromlcn_read_2g_ppr_parameters(phy_info_t *pi)
{
	srom_pwrdet_t *pwrdet  = pi->pwrdet;

	/* Max tx power */
	pi->tx_srom_max_2g = (int8)PHY_GETINTVAR(pi, rstr_maxp2ga0);
	pwrdet->max_pwr[PHY_CORE_0][WL_CHAN_FREQ_RANGE_2G] = pi->tx_srom_max_2g;

	pi->ppr.sr_lcn.cck2gpo = (uint16)PHY_GETINTVAR(pi, rstr_cck2gpo);

	/* Extract offsets for 8 OFDM rates */
	pi->ppr.sr_lcn.ofdm = (uint32)PHY_GETINTVAR(pi, rstr_ofdm2gpo);

	/* Extract offsets for 8 MCS rates */
	/* mcs2gpo(x) are 16 bit numbers */
	pi->ppr.sr_lcn.mcs = ((uint16)PHY_GETINTVAR(pi, rstr_mcs2gpo1) << 16) |
		(uint16)PHY_GETINTVAR(pi, rstr_mcs2gpo0);

	/* Read 5G parameters from srom */
	/* Because there are currently no chips that use the 5GHz band on the LCN PHY,
	 * and because there are no plans to do this, the 5GHz functionality has been
	 * dropped for PPR enabled builds as there are no test targets.
	 * It can be added here if needed.
	 */
}

void
wlc_phy_txpwr_apply_sromlcn(phy_info_t *pi, uint8 band, ppr_t *tx_srom_max_pwr)
{
	srom_lcn_ppr_t *sr_lcn = &pi->ppr.sr_lcn;

	int8 txpwr = 0;

	ppr_dsss_rateset_t ppr_dsss;
	ppr_ofdm_rateset_t ppr_ofdm;
	ppr_ht_mcs_rateset_t ppr_mcs;

	switch (band)
	{
		case WL_CHAN_FREQ_RANGE_2G:
		{
			/* Max tx power */
			txpwr = pi->tx_srom_max_2g;

			/* 2G - CCK */
			wlc_phy_txpwr_srom_convert_cck(sr_lcn->cck2gpo, txpwr, &ppr_dsss);
			ppr_set_dsss(tx_srom_max_pwr, WL_TX_BW_20, WL_TX_CHAINS_1, &ppr_dsss);

			/* 2G - OFDM_20 */
			wlc_phy_txpwr_srom_convert_ofdm(sr_lcn->ofdm, txpwr, &ppr_ofdm);
			ppr_set_ofdm(tx_srom_max_pwr, WL_TX_BW_20, WL_TX_MODE_NONE, WL_TX_CHAINS_1,
				&ppr_ofdm);

			/* 2G - MCS_20 */
			wlc_phy_txpwr_srom_convert_mcs(sr_lcn->mcs, txpwr, &ppr_mcs);
			ppr_set_ht_mcs(tx_srom_max_pwr, WL_TX_BW_20, WL_TX_NSS_1, WL_TX_MODE_NONE,
				WL_TX_CHAINS_1, &ppr_mcs);

			break;
		}
		/* Apply 5G parameters from srom */
		/* Because there are currently no chips that use the 5GHz band on the LCN PHY,
		 * and because there are no plans to do this, the 5GHz functionality has been
		 * dropped for PPR enabled builds as there are no test targets.
		 * It can be added here if needed.
		 */
	}
}

static void
BCMATTACHFN(wlc_lcnphy_decode_aa2g)(wlc_phy_t *ppi, uint8 val)
{
	phy_info_t *pi = (phy_info_t *)ppi;

	switch (val)
	{
		case 1:
			PHY_REG_LIST_START_BCMATTACHDATA
				PHY_REG_MOD_ENTRY(LCNPHY, crsgainCtrl, DiversityChkEnable, 0x00)
				PHY_REG_MOD_ENTRY(LCNPHY, crsgainCtrl, DefaultAntenna, 0x00)
			PHY_REG_LIST_EXECUTE(pi);
			pi->sh->rx_antdiv = 0;
		break;
		case 2:
			PHY_REG_LIST_START_BCMATTACHDATA
				PHY_REG_MOD_ENTRY(LCNPHY, crsgainCtrl, DiversityChkEnable, 0x00)
				PHY_REG_MOD_ENTRY(LCNPHY, crsgainCtrl, DefaultAntenna, 0x01)
			PHY_REG_LIST_EXECUTE(pi);
			pi->sh->rx_antdiv = 1;
		break;
		case 3:
			PHY_REG_LIST_START_BCMATTACHDATA
				PHY_REG_MOD_ENTRY(LCNPHY, crsgainCtrl, DiversityChkEnable, 0x01)
				PHY_REG_MOD_ENTRY(LCNPHY, crsgainCtrl, DefaultAntenna, 0x00)
			PHY_REG_LIST_EXECUTE(pi);
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

void
wlc_2064_vco_cal(phy_info_t *pi)
{
	uint8 calnrst;

	phy_utils_mod_radioreg(pi, RADIO_2064_REG057, 1 << 3, 1 << 3);
	calnrst = (uint8)phy_utils_read_radioreg(pi, RADIO_2064_REG056) & 0xf8;
	phy_utils_write_radioreg(pi, RADIO_2064_REG056, calnrst);
	OSL_DELAY(1);
	phy_utils_write_radioreg(pi, RADIO_2064_REG056, calnrst | 0x03);
	OSL_DELAY(1);
	phy_utils_write_radioreg(pi, RADIO_2064_REG056, calnrst | 0x07);
	OSL_DELAY(300);
	phy_utils_mod_radioreg(pi, RADIO_2064_REG057, 1 << 3, 0);
}

/* 4313A0 now has doubler option enabled, have recoded the RF registers's equations          */
/* Plan is to unify the 4336 channel tune function with 4313's once the new code is stable */
static void
wlc_lcnphy_radio_2064_channel_tune_4313(phy_info_t *pi, uint8 channel)
{
	uint i;
	const chan_info_2064_lcnphy_t *ci;
	uint8 rfpll_doubler = 0;
	uint8 pll_pwrup, pll_pwrup_ovr;
	math_fixed qFxtal, qFref, qFvco, qFcal;
	uint8  d15, d16, f16, e44, e45;
	uint32 div_int, div_frac, fvco3, fpfd, fref3, fcal_div;
	uint16 loop_bw, d30, setCount;
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	if (NORADIO_ENAB(pi->pubpi))
		return;
	ci = &chan_info_2064_lcnphy[0];
	rfpll_doubler = 1;
	/* doubler enable */
	phy_utils_mod_radioreg(pi, RADIO_2064_REG09D, 0x4, 0x1 << 2);
	/* reduce crystal buffer strength */
	phy_utils_write_radioreg(pi, RADIO_2064_REG09E, 0xf);
	if (!rfpll_doubler) {
		loop_bw = PLL_2064_LOOP_BW;
		d30 = PLL_2064_D30;
	} else {
		loop_bw = PLL_2064_LOOP_BW_DOUBLER;
		d30 = PLL_2064_D30_DOUBLER;
	}
	/* lookup radio-chip-specific channel code */
	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		for (i = 0; i < ARRAYSIZE(chan_info_2064_lcnphy); i++)
			if (chan_info_2064_lcnphy[i].chan == channel)
				break;

		if (i >= ARRAYSIZE(chan_info_2064_lcnphy)) {
			PHY_ERROR(("wl%d: %s: channel %d not found in channel table\n",
				pi->sh->unit, __FUNCTION__, channel));
			return;
		}

		ci = &chan_info_2064_lcnphy[i];
	}
	/* Radio tunables */
	/* <4:3> bits of REG02A are for logen_buftune */
	phy_utils_write_radioreg(pi, RADIO_2064_REG02A, ci->logen_buftune);
	/* <1:0> bits of REG030 are for logen_rccr_tx */
	phy_utils_mod_radioreg(pi, RADIO_2064_REG030, 0x3, ci->logen_rccr_tx);
	/* <1:0> bits of REG091 are for txrf_mix_tune_ctrl */
	phy_utils_mod_radioreg(pi, RADIO_2064_REG091, 0x3, ci->txrf_mix_tune_ctrl);
	/* <3:0> bits of REG038 are for pa_INPUT_TUNE_G */
	phy_utils_mod_radioreg(pi, RADIO_2064_REG038, 0xf, ci->pa_input_tune_g);
	/* <3:2> bits of REG030 are for logen_rccr_rx */
	phy_utils_mod_radioreg(pi, RADIO_2064_REG030, 0x3 << 2, (ci->logen_rccr_rx) << 2);
	/* <3:0> bits of REG05E are for rxrf_lna1_freq_tune */
	phy_utils_mod_radioreg(pi, RADIO_2064_REG05E, 0xf, ci->pa_rxrf_lna1_freq_tune);
	/* <7:4> bits of REG05E are for rxrf_lna2_freq_tune */
	phy_utils_mod_radioreg(pi, RADIO_2064_REG05E, (0xf)<<4, (ci->pa_rxrf_lna2_freq_tune)<<4);
	/* <7:0> bits of REG06C are for rxrf_rxrf_spare1:trsw tune */
	phy_utils_write_radioreg(pi, RADIO_2064_REG06C, ci->rxrf_rxrf_spare1);

	/* Turn on PLL power supplies */
	/* to be used later to restore states */
	pll_pwrup = (uint8)phy_utils_read_radioreg(pi, RADIO_2064_REG044);
	pll_pwrup_ovr = (uint8)phy_utils_read_radioreg(pi, RADIO_2064_REG12B);
	/* <2:0> bits of REG044 are pll monitor,synth and vco pwrup */
	phy_utils_or_radioreg(pi, RADIO_2064_REG044, 0x07);
	/* <3:1> bits of REG12B are for the overrides of these pwrups */
	phy_utils_or_radioreg(pi, RADIO_2064_REG12B, (0x07)<<1);
	e44 = 0;
	e45 = 0;
	/* Calculate various input frequencies */
	fpfd = rfpll_doubler ? (PHY_XTALFREQ(pi->xtalfreq) << 1) : (PHY_XTALFREQ(pi->xtalfreq));
	if (PHY_XTALFREQ(pi->xtalfreq) > 26000000)
		e44 = 1;
	if (PHY_XTALFREQ(pi->xtalfreq) > 52000000)
		e45 = 1;
	if (e44 == 0)
		fcal_div = 1;
	else if (e45 == 0)
		fcal_div = 2;
	else
		fcal_div = 4;
	fvco3 = (ci->freq * 3); /* VCO frequency is 1.5 times the LO freq */
	fref3 = 2 * fpfd;
	/* Convert into Q16 MHz */
	qFxtal = wlc_lcnphy_qdiv_roundup(PHY_XTALFREQ(pi->xtalfreq), PLL_2064_MHZ, 16);
	qFref =  wlc_lcnphy_qdiv_roundup(fpfd, PLL_2064_MHZ, 16);
	qFcal = PHY_XTALFREQ(pi->xtalfreq) * fcal_div/PLL_2064_MHZ;
	qFvco = wlc_lcnphy_qdiv_roundup(fvco3, 2, 16);
	BCM_REFERENCE(qFref);
	BCM_REFERENCE(qFxtal);
	BCM_REFERENCE(qFvco);

	/* PLL_delayBeforeOpenLoop */
	phy_utils_write_radioreg(pi, RADIO_2064_REG04F, 0x02);

	/* PLL_enableTimeout */
	d15 = (PHY_XTALFREQ(pi->xtalfreq) * fcal_div* 4/5)/PLL_2064_MHZ - 1;
	phy_utils_write_radioreg(pi, RADIO_2064_REG052, (0x07 & (d15 >> 2)));
	phy_utils_write_radioreg(pi, RADIO_2064_REG053, (d15 & 0x3) << 5);
	/* PLL_cal_ref_timeout */
	d16 = (qFcal * 8 /(d15 + 1)) - 1;
	phy_utils_write_radioreg(pi, RADIO_2064_REG051, d16);

	/* PLL_calSetCount */
	f16 = ((d16 + 1)* (d15 + 1))/qFcal;
	setCount = f16 * 3 * (ci->freq)/32 - 1;
	phy_utils_mod_radioreg(pi, RADIO_2064_REG053, (0x0f << 0), (uint8)(setCount >> 8));
	/* 4th bit is on if using VCO cal */
	phy_utils_or_radioreg(pi, RADIO_2064_REG053, 0x10);
	phy_utils_write_radioreg(pi, RADIO_2064_REG054, (uint8)(setCount & 0xff));
	/* Divider, integer bits */
	div_int = ((fvco3 * (PLL_2064_MHZ >> 4)) / fref3) << 4;

	/* Divider, fractional bits */
	div_frac = ((fvco3 * (PLL_2064_MHZ >> 4)) % fref3) << 4;
	while (div_frac >= fref3) {
		div_int++;
		div_frac -= fref3;
	}
	div_frac = wlc_lcnphy_qdiv_roundup(div_frac, fref3, 20);
	/* Program PLL */
	phy_utils_mod_radioreg(pi, RADIO_2064_REG045, (0x1f << 0), (uint8)(div_int >> 4));
	phy_utils_mod_radioreg(pi, RADIO_2064_REG046, (0x1f << 4), (uint8)(div_int << 4));
	phy_utils_mod_radioreg(pi, RADIO_2064_REG046, (0x0f << 0), (uint8)(div_frac >> 16));
	phy_utils_write_radioreg(pi, RADIO_2064_REG047, (uint8)(div_frac >> 8) & 0xff);
	phy_utils_write_radioreg(pi, RADIO_2064_REG048, (uint8)div_frac & 0xff);

	PHY_REG_LIST_START
		/* Fixed RC values */
		/* REG040 <7:4> is PLL_lf_c1, <3:0> is PLL_lf_c2 */
		RADIO_REG_WRITE_ENTRY(RADIO_2064_REG040, 0xfb)
		/* REG041 <7:4> is PLL_lf_c3, <3:0> is PLL_lf_c4 */
		RADIO_REG_WRITE_ENTRY(RADIO_2064_REG041, 0x9A)
		RADIO_REG_WRITE_ENTRY(RADIO_2064_REG042, 0xA3)
		RADIO_REG_WRITE_ENTRY(RADIO_2064_REG043, 0x0C)
	PHY_REG_LIST_EXECUTE(pi);
	/* PLL_cp_current */
	{
		uint8 h29, h23, c28, d29, h28_ten, e30, h30_ten, cp_current;
		uint16 c29, c38, c30, g30, d28;
		c29 = loop_bw;
		d29 = 200; /* desired loop bw value */
		c38 = 1250;
		h29 = d29/c29; /* change this logic if loop bw is > 200 */
		h23 = 1; /* Ndiv ratio */
		c28 = 30; /* Kvco MHz/V */
		d28 = (((PLL_2064_HIGH_END_KVCO - PLL_2064_LOW_END_KVCO)*
			(fvco3/2 - PLL_2064_LOW_END_VCO))/
			(PLL_2064_HIGH_END_VCO - PLL_2064_LOW_END_VCO))
			+ PLL_2064_LOW_END_KVCO;
		h28_ten = (d28*10)/c28;
		c30 = 2640;
		e30 = (d30 - 680)/490;
		g30 = 680 + (e30 * 490);
		h30_ten = (g30*10)/c30;
		cp_current = ((c38*h29*h23*100)/h28_ten)/h30_ten;
		phy_utils_mod_radioreg(pi, RADIO_2064_REG03C, 0x3f, cp_current);
	}
	if (channel >= 1 && channel <= 5)
		phy_utils_write_radioreg(pi, RADIO_2064_REG03C, 0x8);
	else
		phy_utils_write_radioreg(pi, RADIO_2064_REG03C, 0x7);

	/* Radio reg values set for reducing phase noise */
	if (channel < 4)
		phy_utils_write_radioreg(pi, RADIO_2064_REG03D, 0xa);
	else if (channel < 8)
		phy_utils_write_radioreg(pi, RADIO_2064_REG03D, 0x9);
	else
		phy_utils_write_radioreg(pi, RADIO_2064_REG03D, 0x8);

	/* o_wrf_jtag_pll_cp_reset and o_wrf_jtag_pll_vco_reset (4:3) */
	phy_utils_mod_radioreg(pi, RADIO_2064_REG044, 0x0c, 0x0c);
	OSL_DELAY(1);

	if ((CHIPID(pi->sh->chip) == BCM4313_CHIP_ID) &&
		BOARDFLAGS2(GENERIC_PHY_INFO(pi)->boardflags2) & BFL2_4313_RADIOREG) {
		phy_utils_write_radioreg(pi, RADIO_2064_REG03C, 0x27);
		phy_utils_write_radioreg(pi, RADIO_2064_REG03D, 0x51);
	}

	/* Force VCO cal */
	wlc_2064_vco_cal(pi);
	/* Restore state */
	phy_utils_write_radioreg(pi, RADIO_2064_REG044, pll_pwrup);
	phy_utils_write_radioreg(pi, RADIO_2064_REG12B, pll_pwrup_ovr);
	if (LCNREV_IS(pi->pubpi.phy_rev, 1)) {
		phy_utils_write_radioreg(pi, RADIO_2064_REG038, 0);
		phy_utils_write_radioreg(pi, RADIO_2064_REG091, 7);
	}

	/* 4313 IPA tuning */
	if ((CHIPID(pi->sh->chip) == BCM4313_CHIP_ID) && !EPA(pi_lcn)) {
		uint8 reg038[14] = {0xd, 0xe, 0xd, 0xd, 0xd, 0xc,
			0xa, 0xb, 0xb, 0x3, 0x3, 0x2, 0x0, 0x0};

		PHY_REG_LIST_START
			RADIO_REG_WRITE_ENTRY(RADIO_2064_REG02A, 0xf)
			RADIO_REG_WRITE_ENTRY(RADIO_2064_REG091, 0x3)
			RADIO_REG_WRITE_ENTRY(RADIO_2064_REG038, 0x3)
		PHY_REG_LIST_EXECUTE(pi);

		if (BOARDTYPE(pi->sh->boardtype) == BCM94313HMG_BOARD)
			phy_utils_write_radioreg(pi, RADIO_2064_REG038, reg038[channel - 1]);

	}
}

static void
wlc_lcnphy_radio_2064_channel_tune(phy_info_t *pi, uint8 channel)
{
	uint i;
	void *chi;
	uint8 pll_pwrup, pll_pwrup_ovr;
	math_fixed qFxtal, qFref, qFvco = 0, qFcal, qVco, qVal;
	uint8  to, refTo, cp_current, kpd_scale, ioff_scale, offset_current, e44, e45;
	uint32 setCount, div_int, div_frac, iVal, fvco3 = 0, fref, fref3 = 0, fcal_div;
	uint8 reg_h5e_val = 0;
	uint8 reg_h5e_val_tbl[15] = {0xff, 0xee, 0xdd, 0xcc, 0xbb, 0x99, 0x88, 0x77,
		0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00};
	uint8 rfpll_doubler = 0;
	uint16 loop_bw, d30;
	uint32 c29, c30, e30, h29, h30, temp, temp1;
	uint32 half_div_frac, fpfd, tfref3;
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
#ifdef BAND5G
	uint8 val_REG0B0 = 0, val_REG0AF = 0;
	bool logen_5g_valid;
#endif // endif

	if (NORADIO_ENAB(pi->pubpi))
		return;

	/* lookup radio-chip-specific channel code */
	if (RADIOID(pi->pubpi.radioid) == BCM2066_ID) {
		for (i = 0; i < ARRAYSIZE(chan_info_2066_lcnphy); i++)
			if (chan_info_2066_lcnphy[i].chan == channel)
				break;

		if (i >= ARRAYSIZE(chan_info_2066_lcnphy)) {
			PHY_ERROR(("wl%d: %s: channel %d not found in channel table\n",
				pi->sh->unit, __FUNCTION__, channel));
			return;
		}
		chi = &chan_info_2066_lcnphy[i];
	} else {
		for (i = 0; i < ARRAYSIZE(chan_info_2064_lcnphy); i++)
			if (chan_info_2064_lcnphy[i].chan == channel)
				break;

		if (i >= ARRAYSIZE(chan_info_2064_lcnphy)) {
			PHY_ERROR(("wl%d: %s: channel %d not found in channel table\n",
				pi->sh->unit, __FUNCTION__, channel));
			return;
		}
		chi = &chan_info_2064_lcnphy[i];
	}

	/* Tx Tuning */
	phy_utils_write_radioreg(pi, RADIO_2064_REG091, 0);
	phy_utils_write_radioreg(pi, RADIO_2064_REG038, 7);
	if (CHIPID(pi->sh->chip) != BCM4313_CHIP_ID)
		phy_utils_write_radioreg(pi, RADIO_2064_REG088, 0xa);

	if (pi->u.pi_lcnphy->rfreg088)
		phy_utils_write_radioreg(pi, RADIO_2064_REG088, pi->u.pi_lcnphy->rfreg088);

	if (CHIPID(pi->sh->chip) == BCM4330_CHIP_ID) {
		if (!EPA(pi->u.pi_lcnphy) && CHSPEC_IS5G(pi->radio_chanspec)) {
			/* Tx lpf buffer gain */
			phy_utils_write_radioreg(pi, RADIO_2064_REG011, 0x60);
			/* PGA bias */
			phy_utils_write_radioreg(pi, RADIO_2064_REG0AB, 0x7f);
		}
		else {
			/* Default settings */
			/* Tx lpf buffer gain */
			phy_utils_write_radioreg(pi, RADIO_2064_REG011, 0x0);
			/* PGA bias */
			phy_utils_write_radioreg(pi, RADIO_2064_REG0AB, 0x78);
		}
	}
	if (RADIOID(pi->pubpi.radioid) == BCM2066_ID) {
		if (CHSPEC_IS5G(pi->radio_chanspec)) {
			/* 5G Radio tunables */
			chan_info_2066_a_lcnphy_t *ci = (chan_info_2066_a_lcnphy_t *)chi;
			/* <0:3> bits of REG0AF are for logen_buftune */
			phy_utils_mod_radioreg(pi, RADIO_2064_REG0AF, 0xf, ci->logen_buftune);
			/* REG0B0 are for ctune */
			phy_utils_write_radioreg(pi, RADIO_2064_REG0B0, ci->ctune);
			/* REG0BA are for rccr */
			phy_utils_write_radioreg(pi, RADIO_2064_REG0BA, ci->logen_rccr);
			/* <3:0> bits of REG0BE are for mix_tune_5g */
			phy_utils_mod_radioreg(pi, RADIO_2064_REG0BE, 0xf, ci->mix_tune_5g);
			/* <3:0> bits of REG0CF are for pga_INPUT_TUNE_G */
			phy_utils_mod_radioreg(pi, RADIO_2064_REG0CF, 0xf, ci->pga_tune_ctrl);
			/* <3:0> bits of REG0C9 are for pa_INPUT_TUNE_G */
			phy_utils_mod_radioreg(pi, RADIO_2064_REG0C9, 0xf, ci->pa_input_tune_5g);
			/* <3:0> bits of REG05E are for rxrf_lna1_freq_tune */
			phy_utils_write_radioreg(pi, RADIO_2064_REG0AD, ci->pa_rxrf_lna1_freq_tune);
			/* <7:0> bits of REG06C are for rxrf_rxrf_spare1:trsw tune */
			phy_utils_mod_radioreg(pi, RADIO_2064_REG0D4, 0xff, ci->rxrf_rxrf_spare1);
			phy_utils_write_radioreg(pi, RADIO_2064_REG0D3, 1);
		} else {
			/* Radio tunables */
			chan_info_2066_lcnphy_t *ci = (chan_info_2066_lcnphy_t *)chi;
			/* <4:3> bits of REG02A are for logen_buftune */
			phy_utils_mod_radioreg(pi, RADIO_2064_REG02A, 0x18,
			                       (ci->logen_buftune) << 3);
			/* <1:0> bits of REG030 are for logen_rccr_tx */
			phy_utils_write_radioreg(pi, RADIO_2064_REG030, ci->logen_rccr_tx);
			/* <1:0> bits of REG091 are for txrf_mix_tune_ctrl */
			phy_utils_mod_radioreg(pi, RADIO_2064_REG091, 0x3, ci->txrf_mix_tune_ctrl);
			/* <3:0> bits of REG038 are for pa_INPUT_TUNE_G */
			phy_utils_mod_radioreg(pi, RADIO_2064_REG038, 0xf, ci->pa_input_tune_g);
			/* <3:0> bits of REG05E are for rxrf_lna1_freq_tune */
			phy_utils_write_radioreg(pi, RADIO_2064_REG05E, ci->pa_rxrf_lna1_freq_tune);
			/* <7:0> bits of REG06C are for rxrf_rxrf_spare1:trsw tune */
			phy_utils_write_radioreg(pi, RADIO_2064_REG06C, ci->rxrf_rxrf_spare1);
		}
		if (pi->sh->chiprev == 0) {
			/* mixer and PAD tuning, to match TCL */
			phy_utils_write_radioreg(pi, RADIO_2064_REG038, 0);
			phy_utils_write_radioreg(pi, RADIO_2064_REG091, 3);
		}
		if (pi->fabid == 2)
			phy_utils_write_radioreg(pi, RADIO_2064_REG091, 2);

	} else {
		chan_info_2064_lcnphy_t *ci = (chan_info_2064_lcnphy_t *)chi;
		/* Radio tunables */
		/* <4:3> bits of REG02A are for logen_buftune */
		phy_utils_write_radioreg(pi, RADIO_2064_REG02A, ci->logen_buftune);
		/* <1:0> bits of REG030 are for logen_rccr_tx */
		phy_utils_mod_radioreg(pi, RADIO_2064_REG030, 0x3, ci->logen_rccr_tx);
		/* <1:0> bits of REG091 are for txrf_mix_tune_ctrl */
		phy_utils_mod_radioreg(pi, RADIO_2064_REG091, 0x3, ci->txrf_mix_tune_ctrl);
		/* <3:0> bits of REG038 are for pa_INPUT_TUNE_G */
		phy_utils_mod_radioreg(pi, RADIO_2064_REG038, 0xf, ci->pa_input_tune_g);

		/* Tx tuning - Lookup table does not have correct values for these */
		phy_utils_write_radioreg(pi, RADIO_2064_REG091, 0x00); /* Mixer Tuning */
		phy_utils_write_radioreg(pi, RADIO_2064_REG038, 0x07); /* PAD tuning */

		/* <3:2> bits of REG030 are for logen_rccr_rx */
		phy_utils_mod_radioreg(pi, RADIO_2064_REG030, 0x3 << 2, (ci->logen_rccr_rx) << 2);
		/* <3:0> bits of REG05E are for rxrf_lna1_freq_tune */
		phy_utils_mod_radioreg(pi, RADIO_2064_REG05E, 0xf, ci->pa_rxrf_lna1_freq_tune);
		/* <7:4> bits of REG05E are for rxrf_lna2_freq_tune */
		phy_utils_mod_radioreg(pi, RADIO_2064_REG05E, (0xf)<<4,
		                       (ci->pa_rxrf_lna2_freq_tune)<<4);
		/* <7:0> bits of REG06C are for rxrf_rxrf_spare1:trsw tune */
		phy_utils_write_radioreg(pi, RADIO_2064_REG06C, ci->rxrf_rxrf_spare1);

		/* 4336 wlbga, esp A0Vx tx tuning */
		if (((CHIPID(pi->sh->chip) == BCM4336_CHIP_ID) &&
			(pi->sh->chippkg == BCM4336_WLBGA_PKG_ID))||
			((CHIPID(pi->sh->chip) == BCM4330_CHIP_ID) &&
			LCNREV_IS(pi->pubpi.phy_rev, 0)) ||
			(CHIPID(pi->sh->chip) == BCM43362_CHIP_ID)) {

			if (CHIPID(pi->sh->chip) != BCM43362_CHIP_ID)
				phy_utils_write_radioreg(pi, RADIO_2064_REG038, 0);
			else
				phy_utils_write_radioreg(pi, RADIO_2064_REG038, 0x8);

			phy_utils_write_radioreg(pi, RADIO_2064_REG091, 3);

			reg_h5e_val = reg_h5e_val_tbl[channel-1];
			phy_utils_write_radioreg(pi, RADIO_2064_REG05E, reg_h5e_val);
		}
		/* internal trsw programming for 4336A0Vx */
		phy_utils_write_radioreg(pi, RADIO_2064_REG07E, 1);
	}

	if (pi_lcn->rfreg038)
		phy_utils_write_radioreg(pi, RADIO_2064_REG038, pi_lcn->rfreg038);

	/* <2:0> bits of REG02A are for o_wrf_jtag_logen_idac_gm */
	phy_utils_mod_radioreg(pi, RADIO_2064_REG02A, 0x7, 7);
	/* logen tuning */
	phy_utils_write_radioreg(pi, RADIO_2064_REG02C, 0);

	if ((LCNREV_GE(pi->pubpi.phy_rev, 1) &&
		(CHIPID(pi->sh->chip) == BCM4336_CHIP_ID)) ||
		(CHIPID(pi->sh->chip) == BCM43362_CHIP_ID)) {
		phy_utils_write_radioreg(pi, RADIO_2064_REG02A, 0xc);
		phy_utils_write_radioreg(pi, RADIO_2064_REG02C, 0x1);
	}

	if (pi->u.pi_lcnphy->logen_mode[0]) {
		phy_utils_write_radioreg(pi, RADIO_2064_REG05E, pi->u.pi_lcnphy->logen_mode[1]);
		if (RADIOID(pi->pubpi.radioid) == BCM2066_ID) {
			PHY_REG_MOD(pi, LCNPHY, extslnactrl0, slna_freq_tune,
				(pi->u.pi_lcnphy->logen_mode[1] & 0xf));
		}
		phy_utils_write_radioreg(pi, RADIO_2064_REG02A, pi->u.pi_lcnphy->logen_mode[2]);
		phy_utils_write_radioreg(pi, RADIO_2064_REG02B, pi->u.pi_lcnphy->logen_mode[3]);
		phy_utils_write_radioreg(pi, RADIO_2064_REG02C, pi->u.pi_lcnphy->logen_mode[4]);
		phy_utils_write_radioreg(pi, RADIO_2064_REG02D, pi->u.pi_lcnphy->logen_mode[5]);
	}

#ifdef BAND5G
	for (i = 0; i <= 4; i++) {
		if ((channel >= pi_lcn->logen_mode5g[i][1]) &&
			(channel <= pi_lcn->logen_mode5g[i][2])) {
			val_REG0B0 = pi_lcn->logen_mode5g[i][3];
			val_REG0AF = pi_lcn->logen_mode5g[i][4];
			logen_5g_valid = pi_lcn->logen_mode5g[i][0];
			break;
		} else {
			logen_5g_valid = FALSE;
		}
	}
	if (logen_5g_valid) {
		phy_utils_mod_radioreg(pi, RADIO_2064_REG0B0, 0xf0, val_REG0B0 << 4);
		phy_utils_write_radioreg(pi, RADIO_2064_REG0AF, val_REG0AF);
	}
#endif /* BAND5G */

#ifdef BAND5G
	if (CHSPEC_IS5G(pi->radio_chanspec))
		phy_utils_mod_radioreg(pi, RADIO_2064_REG0B2, 0x38, 7 << 3);
#endif // endif

	/* a band ipa tuning */
	if (!EPA(pi->u.pi_lcnphy) && CHSPEC_IS5G(pi->radio_chanspec))
		phy_utils_write_radioreg(pi, RADIO_2064_REG0BF, 0xee);

	/* increase current to ADC to prevent problem with clipping */
	/* <3:2> bits of REG0F3 are for o_wrf_jtag_afe_iqadc_f1_ctl */
	/* new ADC clipping WAR */
	if (1) {
		PHY_REG_LIST_START
			RADIO_REG_MOD_ENTRY(RADIO_2064_REG11F, 0x2, 1 << 1)
			RADIO_REG_MOD_ENTRY(RADIO_2064_REG0F7, 0x4, 1 << 2)
			RADIO_REG_MOD_ENTRY(RADIO_2064_REG0F1, 0x3, 0)
			RADIO_REG_MOD_ENTRY(RADIO_2064_REG0F2, 0xF8, (2 << 3) | (2 << 6))
			RADIO_REG_MOD_ENTRY(RADIO_2064_REG0F3, 0xFF, (2 << 6) | (2 << 4) | 2)
		PHY_REG_LIST_EXECUTE(pi);
	} else
		phy_utils_mod_radioreg(pi, RADIO_2064_REG0F3, 0x3 << 2, 1 << 2);

	/* Turn on PLL power supplies */
	if (((CHSPEC_IS2G(pi->radio_chanspec)) && (pi->u.pi_lcnphy->rfpll_doubler_2g == TRUE)) ||
		((CHSPEC_IS5G(pi->radio_chanspec)) && (pi->u.pi_lcnphy->rfpll_doubler_5g == TRUE)))
		rfpll_doubler = 1;

	/* doubler enable */
	if (rfpll_doubler)
		phy_utils_mod_radioreg(pi, RADIO_2064_REG09D, 0x4, 0x1 << 2);
	else
		phy_utils_mod_radioreg(pi, RADIO_2064_REG09D, 0x4, 0x0 << 2);

	/* to be used later to restore states */
	pll_pwrup = (uint8)phy_utils_read_radioreg(pi, RADIO_2064_REG044);
	pll_pwrup_ovr = (uint8)phy_utils_read_radioreg(pi, RADIO_2064_REG12B);
	/* <2:0> bits of REG044 are pll monitor,synth and vco pwrup */
	phy_utils_or_radioreg(pi, RADIO_2064_REG044, 0x07);
	/* <3:1> bits of REG12B are for the overrides of these pwrups */
	phy_utils_or_radioreg(pi, RADIO_2064_REG12B, (0x07)<<1);
	e44 = 0;
	e45 = 0;
	/* Calculate various input frequencies */
	fref = PHY_XTALFREQ(pi->xtalfreq);
	/* Calculate various input frequencies */
	fpfd = rfpll_doubler ? (fref << 1) : (fref);
	if (fref > 26000000)
		e44 = 1;
	if (fref > 52000000)
		e45 = 1;
	if (e44 == 0)
		fcal_div = 1;
	else if (e45 == 0)
		fcal_div = 2;
	else
		fcal_div = 4;

	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		fvco3 = ((chan_info_2064_lcnphy_t *)chi)->freq * 3;
		fref3 = 2 * fpfd;
	}
#ifdef BAND5G
	else {
		fvco3 = (((chan_info_2064_lcnphy_t *)chi)->freq * 4);
		fref3 = 6 * fpfd;
	}
#endif // endif

	/* Convert into Q16 MHz */
	qFxtal = wlc_lcnphy_qdiv_roundup(fref, PLL_2064_MHZ, 16);
	BCM_REFERENCE(qFxtal);
	qFref =  wlc_lcnphy_qdiv_roundup(fpfd, PLL_2064_MHZ, 16);
	qFcal = wlc_lcnphy_qdiv_roundup(fref, fcal_div * PLL_2064_MHZ, 16);
	if (CHSPEC_IS2G(pi->radio_chanspec))
		qFvco = wlc_lcnphy_qdiv_roundup(fvco3, 2, 16);
#ifdef BAND5G
	else
		qFvco = wlc_lcnphy_qdiv_roundup(fvco3, 6, 16);
#endif // endif
	/* PLL_delayBeforeOpenLoop */
	phy_utils_write_radioreg(pi, RADIO_2064_REG04F, 0x02);
	/* PLL_enableTimeout */
	to = (uint8)((((fref * PLL_2064_CAL_REF_TO) /
		(PLL_2064_OPEN_LOOP_DELAY * fcal_div * PLL_2064_MHZ)) + 1) >> 1) - 1;
	phy_utils_mod_radioreg(pi, RADIO_2064_REG052, (0x07 << 0), to >> 2);
	phy_utils_mod_radioreg(pi, RADIO_2064_REG053, (0x03 << 5), to << 5);

	/* PLL_cal_ref_timeout */
	refTo = (uint8)((((fref * PLL_2064_CAL_REF_TO) / (fcal_div * (to + 1))) +
		(PLL_2064_MHZ - 1)) / PLL_2064_MHZ) - 1;
	phy_utils_write_radioreg(pi, RADIO_2064_REG051, refTo);

	/* PLL_calSetCount */
	setCount = (uint32)FLOAT(
		(math_fixed)wlc_lcnphy_qdiv_roundup(qFvco, qFcal * 16, 16) *
		(refTo + 1) * (to + 1)) - 1;
	phy_utils_mod_radioreg(pi, RADIO_2064_REG053, (0x0f << 0), (uint8)(setCount >> 8));
	phy_utils_write_radioreg(pi, RADIO_2064_REG054, (uint8)(setCount & 0xff));

	/* Divider, integer bits */
	tfref3 = fref3/100000;
	div_int = ((fvco3 * 10) / tfref3);

	/* Divider, fractional bits */
	div_frac = ((fvco3 * 10) % tfref3);
	half_div_frac = (((div_frac >> 1) * 1048576) + (tfref3/4))/(tfref3>>1);
	div_frac = (half_div_frac) + (((div_frac & 1) * 1048576) + (tfref3/2))/(tfref3);

	/* Program PLL */
	phy_utils_mod_radioreg(pi, RADIO_2064_REG045, (0x1f << 0), (uint8)(div_int >> 4));
	phy_utils_mod_radioreg(pi, RADIO_2064_REG046, (0x1f << 4), (uint8)(div_int << 4));
	phy_utils_mod_radioreg(pi, RADIO_2064_REG046, (0x0f << 0), (uint8)(div_frac >> 16));
	phy_utils_write_radioreg(pi, RADIO_2064_REG047, (uint8)(div_frac >> 8) & 0xff);
	phy_utils_write_radioreg(pi, RADIO_2064_REG048, (uint8)div_frac & 0xff);

#define QFACTOR	9
	c29 = 270;
	c30 = 2640;

	if (rfpll_doubler) {
		loop_bw = 450;
		d30 = 4500;
	} else {
		loop_bw = PLL_2064_LOOP_BW;
		d30 = PLL_2064_D30;
	}

	e30 = ((d30-680) + 245)/490;
	h29 = (loop_bw << QFACTOR)/c29;
	h30 = ((680+(e30*490)) << QFACTOR)/c30;

	/* PLL_lf_r1 */
	temp = (((d30 - 680) + 245)/490);
	/* PLL_lf_r2 and PLL_lf_r3 */
	temp1 = (((1660*h30 - (680 << QFACTOR))+(245 << QFACTOR))/490) >> QFACTOR;
	phy_utils_write_radioreg(pi, RADIO_2064_REG042, ((temp << 3) | ((temp1 & 0x1c) >> 2)));
	phy_utils_write_radioreg(pi, RADIO_2064_REG043, ((temp1 & 0x1f) | ((temp1 & 3) << 5)));

	/* PLL_lf_c1 */
	temp = (((10465 << (2*QFACTOR))/((h30*h29) >> QFACTOR)) - (1700 << QFACTOR)
		+ (128 << (QFACTOR-1)))/(128 << QFACTOR);
	/* PLL_lf_c2 */
	temp1 = (((6170 << (2*QFACTOR))/((h30*h29) >> QFACTOR)) - (1300 << QFACTOR)
		+ (73 << (QFACTOR-1)))/(73 << QFACTOR);
	phy_utils_write_radioreg(pi, RADIO_2064_REG040, ((temp << 4) | (temp1 & 0xf)));

	/* PLL_lf_c3 */
	temp = (((2700 << (2*QFACTOR))/((h30*h29) >> QFACTOR)) - (500 << QFACTOR)
		+ (46 << (QFACTOR-1)))/(46 << QFACTOR);
	/* PLL_lf_c4 */
	temp1 = (((2640 << (2*QFACTOR))/((h30*h29) >> QFACTOR)) - (420 << QFACTOR)
		+ (46 << (QFACTOR-1)))/(46 << QFACTOR);
	phy_utils_write_radioreg(pi, RADIO_2064_REG041, ((temp << 4) | (temp1 & 0xf)));

	/* PLL_cp_current */
	qVco = ((PLL_2064_HIGH_END_KVCO - PLL_2064_LOW_END_KVCO) *
	((qFvco - FIXED(PLL_2064_LOW_END_VCO)) /
	(PLL_2064_HIGH_END_VCO - PLL_2064_LOW_END_VCO))) +
	FIXED(PLL_2064_LOW_END_KVCO);
	iVal = ((d30 - 680)  + (490 >> 1))/ 490;
	qVal = wlc_lcnphy_qdiv_roundup(
		880 * loop_bw * div_int,
		27 * (68 + (iVal * 49)), 16);

	cp_current = (qVal + (qVco >> 1))/ qVco;

	kpd_scale = cp_current > 60 ? 1 : 0;
	if (kpd_scale)
		cp_current = (cp_current >> 1) - 4;
	else
		cp_current = cp_current - 4;

	phy_utils_mod_radioreg(pi, RADIO_2064_REG03C, 0x3f, cp_current);
	/*  PLL_Kpd_scale2 */
	phy_utils_mod_radioreg(pi, RADIO_2064_REG03C, 1 << 6, (kpd_scale << 6));

	/*  PLL_offset_current */
	if (((CHSPEC_IS2G(pi->radio_chanspec)) &&
		(pi->u.pi_lcnphy->rfpll_doubler_mode2g == TRUE)) ||
		((CHSPEC_IS5G(pi->radio_chanspec)) &&
		(pi->u.pi_lcnphy->rfpll_doubler_mode5g == TRUE))) {
		qVal = wlc_lcnphy_qdiv_roundup(325 * qFref, qFvco, 16)
				* (cp_current + 4) * (kpd_scale + 1);
		phy_utils_mod_radioreg(pi, RADIO_2064_REG09E, 0xc0, 0x1<<6);
	}
	else
		qVal = wlc_lcnphy_qdiv_roundup(150 * qFref, qFvco, 16)
				* (cp_current + 4) * (kpd_scale + 1);

	ioff_scale = (qVal > FIXED(150)) ? 1 : 0;
	qVal = (qVal / (3 * (ioff_scale + 1))) - FIXED(2);
	if (qVal < 0)
		offset_current = 0;
	else
		offset_current = FLOAT(qVal + (FIXED(1) >> 1));

	phy_utils_mod_radioreg(pi, RADIO_2064_REG03D, 0x3f, offset_current);

	/*  PLL_ioff_scale2 */
	phy_utils_mod_radioreg(pi, RADIO_2064_REG03D, 1 << 6, ioff_scale << 6);

	/* PLL_cal_xt_endiv */
	if (fref > 26000000)
		phy_utils_mod_radioreg(pi, RADIO_2064_REG057, 1 << 5, 1 << 5);
	else
		phy_utils_mod_radioreg(pi, RADIO_2064_REG057, 1 << 5, 0 << 5);

	/* PLL_cal_xt_sdiv */
	if (fref > 52000000)
		phy_utils_mod_radioreg(pi, RADIO_2064_REG057, 1 << 4, 1 << 4);
	else
		phy_utils_mod_radioreg(pi, RADIO_2064_REG057, 1 << 4, 0 << 4);

	/* PLL_sel_short */
	if (qFref > FIXED(45))
		phy_utils_or_radioreg(pi, RADIO_2064_REG04A, 0x02);
	else
		phy_utils_and_radioreg(pi, RADIO_2064_REG04A, 0xfd);

	/* o_wrf_jtag_pll_cp_reset and o_wrf_jtag_pll_vco_reset (4:3) */
	phy_utils_mod_radioreg(pi, RADIO_2064_REG044, 0x0c, 0x0c);
	OSL_DELAY(1);

	/* Force VCO cal */
	wlc_2064_vco_cal(pi);

	/* Restore state */
	phy_utils_write_radioreg(pi, RADIO_2064_REG044, pll_pwrup);
	phy_utils_write_radioreg(pi, RADIO_2064_REG12B, pll_pwrup_ovr);
}

bool
wlc_phy_tpc_isenabled_lcnphy(phy_info_t *pi)
{
	if (wlc_lcnphy_tempsense_based_pwr_ctrl_enabled(pi))
		return 0;
	else
		return (LCNPHY_TX_PWR_CTRL_HW == wlc_lcnphy_get_tx_pwr_ctrl((pi)));
}

bool
wlc_phy_tpc_iovar_isenabled_lcnphy(phy_info_t *pi)
{
	if (CHIPID(pi->sh->chip) != BCM4313_CHIP_ID)
		return wlc_phy_tpc_isenabled_lcnphy(pi);
	else
		return 	((phy_utils_read_phyreg((pi), LCNPHY_TxPwrCtrlCmd) &
			(LCNPHY_TxPwrCtrlCmd_txPwrCtrl_en_MASK |
			LCNPHY_TxPwrCtrlCmd_hwtxPwrCtrl_en_MASK))
			== LCNPHY_TX_PWR_CTRL_HW);
}

void
wlc_lcnphy_iovar_txpwrctrl(phy_info_t *pi, int32 int_val, int32 *ret_int_ptr)
{
	uint16 pwrctrl;
#if defined(PHYCAL_CACHING)
	ch_calcache_t *ctx = wlc_phy_get_chanctx(pi, pi->radio_chanspec);
	if (ctx == NULL) {
		PHY_ERROR(("%s: NULL ctx - bail\n", __FUNCTION__));
		return;
	}
#else
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
#endif // endif
	if (CHIPID(pi->sh->chip) == BCM4313_CHIP_ID)
		pwrctrl = int_val ?
			LCNPHY_TX_PWR_CTRL_TEMPBASED : LCNPHY_TX_PWR_CTRL_OFF;
	else
		pwrctrl = int_val ?
			LCNPHY_TX_PWR_CTRL_HW : LCNPHY_TX_PWR_CTRL_OFF;

	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	phy_utils_phyreg_enter(pi);
	if (!int_val)
		wlc_lcnphy_set_tx_pwr_by_index(pi, LCNPHY_MAX_TX_POWER_INDEX);

	wlc_lcnphy_set_tx_pwr_ctrl(pi, pwrctrl);

#if defined(PHYCAL_CACHING)
	ctx->valid = FALSE;
#else
	pi_lcn->lcnphy_full_cal_channel = 0;
#endif // endif
	pi->phy_forcecal = TRUE;
	phy_utils_phyreg_exit(pi);
	wlapi_enable_mac(pi->sh->physhim);
}

/* %%%%%% major flow operations */
void
wlc_phy_txpower_recalc_target_lcnphy(phy_info_t *pi)
{
	uint16 pwr_ctrl;
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;

	pi->openlp_tx_power_on = 0;

	/* FIX ME: The below is a hack for setting negative dBm power 	*/
	if (pi_lcn->offset_targetpwr) {
		PHY_REG_MOD(pi, LCNPHY, TxPwrCtrlTargetPwr,
			targetPwr0, (wlc_phy_txpower_get_target_min((wlc_phy_t*)pi) -
			(pi_lcn->offset_targetpwr * 4)));
		return;
	}

	if ((pi_lcn->openlp_pwrctrl) &&
		((pi->openlp_tx_power_min < pi_lcn->tssi_minpwr_limit) ||
		(pi->openlp_tx_power_min < pi_lcn->openlp_pwrlimqdB))) {
		wlc_lcnphy_set_tx_pwr_ctrl(pi, LCNPHY_TX_PWR_CTRL_TEMPBASED);
		pi->openlp_tx_power_on = 1;
	}
	else if (wlc_lcnphy_tempsense_based_pwr_ctrl_enabled(pi)) {
		wlc_lcnphy_calib_modes(pi, LCNPHY_PERICAL_TEMPBASED_TXPWRCTRL);
	} else if (wlc_lcnphy_tssi_based_pwr_ctrl_enabled(pi)) {
		/* Temporary disable power control to update settings */
		pwr_ctrl = wlc_lcnphy_get_tx_pwr_ctrl(pi);
		wlc_lcnphy_set_tx_pwr_ctrl(pi, LCNPHY_TX_PWR_CTRL_OFF);
		wlc_lcnphy_txpower_recalc_target(pi);
		/* Restore power control */
		wlc_lcnphy_set_tx_pwr_ctrl(pi, pwr_ctrl);
	} else
		return;
}

void
wlc_phy_detach_lcnphy(phy_info_t *pi)
{
	if (pi->ptssi_cal)
		MFREE(pi->sh->osh, pi->ptssi_cal, sizeof(tssi_cal_info_t));
	/* Free up the memory used for the Tx and Rx gain tables */
	wlc_lcnphy_tx_gain_tbl_free(pi);
	wlc_lcnphy_rx_gain_tbl_free(pi);

	MFREE(pi->sh->osh, pi->u.pi_lcnphy, sizeof(phy_info_lcnphy_t));
}

bool
BCMATTACHFN(wlc_phy_attach_lcnphy)(phy_info_t *pi, int bandtype)
{
	phy_info_lcnphy_t *pi_lcn;

	pi->u.pi_lcnphy = (phy_info_lcnphy_t*)MALLOC(pi->sh->osh, sizeof(phy_info_lcnphy_t));
	if (pi->u.pi_lcnphy == NULL) {
		PHY_ERROR(("wl%d: %s: MALLOC failure\n", pi->sh->unit, __FUNCTION__));
		return FALSE;
	}
	bzero((char *)pi->u.pi_lcnphy, sizeof(phy_info_lcnphy_t));

	pi_lcn = pi->u.pi_lcnphy;

#if defined(PHYCAL_CACHING)
	/* Reset the var as no cal cache context should exist yet */
	pi->phy_calcache_num = 0;
#endif // endif

	if (((BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_NOPA) == 0) &&
	    !NORADIO_ENAB(pi->pubpi)) {
		pi->hwpwrctrl = TRUE;
		pi->hwpwrctrl_capable = TRUE;
	}

	/* Get xtal frequency from PMU */
#if !defined(XTAL_FREQ)
	pi->xtalfreq = si_alp_clock(pi->sh->sih);
#endif // endif
	ASSERT((PHY_XTALFREQ(pi->xtalfreq) % 1000) == 0);

	/* set papd_rxGnCtrl_init to 0 */
	pi_lcn->lcnphy_papd_rxGnCtrl_init = 0;

	PHY_INFORM(("wl%d: %s: using %d.%d MHz xtalfreq for RF PLL\n",
		pi->sh->unit, __FUNCTION__,
		PHY_XTALFREQ(pi->xtalfreq) / 1000000, PHY_XTALFREQ(pi->xtalfreq) % 1000000));

	pi->pi_fptr.calinit = wlc_phy_cal_init_lcnphy;
	pi->pi_fptr.chanset = wlc_phy_chanspec_set_lcnphy;
	pi->pi_fptr.settxpwrctrl = wlc_lcnphy_set_tx_pwr_ctrl;
	pi->pi_fptr.gettxpwrctrl = wlc_lcnphy_get_tx_pwr_ctrl;
	pi->pi_fptr.tssicalsweep = wlc_lcnphy_tssi_cal_sweep;
	pi->pi_fptr.settxpwrbyindex = wlc_lcnphy_set_tx_pwr_by_index;
#if defined(BCMDBG) || defined(WLTEST)
	pi->pi_fptr.longtrn = wlc_phy_lcnphy_long_train;
#endif // endif
	pi->pi_fptr.txiqccget = wlc_lcnphy_get_tx_iqcc;
	pi->pi_fptr.txiqccset = wlc_lcnphy_set_tx_iqcc;
	pi->pi_fptr.txloccget = wlc_lcnphy_get_tx_locc;
	pi->pi_fptr.radioloftget = wlc_lcnphy_get_radio_loft;
	pi->pi_fptr.radioloftset = wlc_lcnphy_set_radio_loft;
	pi->pi_fptr.phywatchdog = wlc_phy_watchdog_lcnphy;
	pi->pi_fptr.phybtcadjust = wlc_lcnphy_btc_adjust;
#if defined(WLTEST)
	pi->pi_fptr.carrsuppr = wlc_phy_carrier_suppress_lcnphy;
	pi->pi_fptr.rxsigpwr = wlc_lcnphy_rx_signal_power;
#endif // endif
	pi->pi_fptr.calibmodes = wlc_lcnphy_calib_modes;

	if (!wlc_phy_txpwr_srom_read_lcnphy(pi, bandtype))
		return FALSE;

	/* Now tssi based pwr ctrl is supported in iPA */
	/* Tx Pwr Ctrl */
	if ((CHIPID(pi->sh->chip) == BCM4313_CHIP_ID) && LCNREV_IS(pi->pubpi.phy_rev, 1)) {
		if (pi_lcn->lcnphy_tempsense_option == 3) {
			pi->hwpwrctrl = TRUE;
			pi->hwpwrctrl_capable = TRUE;
			pi_lcn->temppwrctrl_capable = FALSE;
			PHY_INFORM(("TSSI based power control is enabled\n"));
		} else {
			pi->hwpwrctrl = FALSE;
			pi->hwpwrctrl_capable = FALSE;
			pi_lcn->temppwrctrl_capable = TRUE;
			PHY_INFORM(("Tempsense based power control is enabled\n"));
		}
	}

	wlc_lcnphy_noise_attach(pi);

	/* Select the TX gain table */
	if (!wlc_lcnphy_tx_gain_tbl_select(pi))
		return FALSE;

	/* Select the RX gain table */
	if (!wlc_lcnphy_rx_gain_tbl_select(pi))
		return FALSE;

#ifdef LP_P2P_SOFTAP
	/* Update the MACAddr LUT */
	if (pi_lcn->pwr_offset_val)
		wlc_lcnphy_lpc_write_maclut(pi);
#endif /* LP_P2P_SOFTAP */

	/* These function are not defined for this phy. */
	pi->pi_fptr.txswctrlmapsetptr = NULL;
	pi->pi_fptr.txswctrlmapgetptr = NULL;

#if defined(WLTEST) || defined(BCMDBG)
	pi->pi_fptr.epadpdsetptr = NULL;
#endif // endif

	return TRUE;
}

/* %%%%%% testing */
#if defined(BCMDBG) || defined(WLTEST)
int
wlc_phy_lcnphy_long_train(phy_info_t *pi, int channel)
{
	uint16 num_samps;
	phytbl_info_t tab;

	/* stop any test in progress */
	wlc_phy_test_stop(pi);

	/* channel 0 means restore original contents and end the test */
	if (channel == 0) {
		wlc_lcnphy_stop_tx_tone(pi);
		wlc_lcnphy_deaf_mode(pi, FALSE);
		return 0;
	}

	if (wlc_phy_test_init(pi, channel, TRUE)) {
		return 1;
	}

	wlc_lcnphy_deaf_mode(pi, TRUE);

	num_samps = sizeof(ltrn_list)/sizeof(*ltrn_list);
	/* load sample table */
	tab.tbl_ptr = ltrn_list;
	tab.tbl_len = num_samps;
	tab.tbl_id = LCNPHY_TBL_ID_SAMPLEPLAY;
	tab.tbl_offset = 0;
	tab.tbl_width = 16;
	wlc_lcnphy_write_table(pi, &tab);

	wlc_lcnphy_run_samples(pi, num_samps, 0xffff, 0, 0);

	return 0;
}

void
wlc_phy_init_test_lcnphy(phy_info_t *pi)
{
	/* Force WLAN antenna */
	wlc_btcx_override_enable(pi);
	/* Disable tx power control */
	wlc_lcnphy_set_tx_pwr_ctrl(pi, LCNPHY_TX_PWR_CTRL_OFF);
	/* Recalibrate for this channel */
	wlc_lcnphy_calib_modes(pi, PHY_FULLCAL);
}
#endif // endif

#if defined(WLTEST)
void
wlc_phy_carrier_suppress_lcnphy(phy_info_t *pi)
{
	if (ISLCNPHY(pi)) {
		wlc_lcnphy_reset_radio_loft(pi);
		if (wlc_phy_tpc_isenabled_lcnphy(pi))
			wlc_lcnphy_clear_tx_power_offsets(pi);
		else
			wlc_lcnphy_set_tx_locc(pi, 0);
	} else
		ASSERT(0);

}

#endif // endif

static void
wlc_lcnphy_reset_radio_loft(phy_info_t *pi)
{
	phy_utils_write_radioreg(pi, RADIO_REG_EI(pi), 0x88);
	phy_utils_write_radioreg(pi, RADIO_REG_EQ(pi), 0x88);
	phy_utils_write_radioreg(pi, RADIO_REG_FI(pi), 0x88);
	phy_utils_write_radioreg(pi, RADIO_REG_FQ(pi), 0x88);
}

static int8
wlc_lcnphy_set_gain_by_index(phy_info_t *pi, uint8 gain_index, uint8 digi_gain_boost)
{
	phytbl_info_t tab;
	uint32 data[2];
	uint8 offset[5];
	int8 gain_data, k, tr, elna, lna1, lna2, tia, biq1, biq2, digi, gain_val;

	wlapi_bmac_mctrl(pi->sh->physhim, MCTL_TBTT_HOLD,  MCTL_TBTT_HOLD);
	OSL_DELAY(5);
	PHY_REG_MOD(pi, LCNPHY, lpphyCtrl, resetCCA, 0);

	if (gain_index > 36) {
		gain_index = 36;
	}
	tab.tbl_id = LCNPHY_TBL_ID_GAIN_IDX;
	tab.tbl_width = 32;
	tab.tbl_ptr = data;
	tab.tbl_len = 2;
	tab.tbl_offset = 2*gain_index;
	wlc_lcnphy_read_table(pi, &tab);
	if (LCNREV_GE(pi->pubpi.phy_rev, 2)) {
		lna1 = (data[1] >> 3) & 7;
		elna = (data[1] >> 6) & 1;
		tr = (data[1] >> 7) & 1;
	} else {
		lna1 = (data[1] >> 3) & 3;
		elna = (data[1] >> 6) & 1;
		tr = (data[1] >> 7) & 1;
	}
	tab.tbl_id = LCNPHY_TBL_ID_GAIN_TBL;
	tab.tbl_width = 32;
	tab.tbl_ptr = data;
	tab.tbl_len = 1;
	tab.tbl_offset = ((data[0] >> 28) & 0xf) | ((data[1] & 0x7) << 4);
	wlc_lcnphy_read_table(pi, &tab);
	lna2 = data[0] & 3;
	tia = (data[0] >> 2) & 0xf;
	biq1 = (data[0] >> 6) & 0xf;
	biq2 = (data[0] >> 10) & 0xf;
	if (LCNREV_GE(pi->pubpi.phy_rev, 2)) {
		offset[0] = 15; /* T/R */
		offset[1] = 13; /* ELNA */
		offset[2] = 0; /* LNA1 */
		offset[3] = 17; /* LNA2 */
		offset[4] = 21; /* TIA */
		digi = digi_gain_boost + ((data[0] >> 14) & 0xf);
	} else {
		offset[0] = 56; /* T/R */
		offset[1] = 58; /* ELNA */
		offset[2] = 0; /* LNA1 */
		offset[3] = 4; /* LNA2 */
		offset[4] = 8; /* TIA */
		digi = 0; /* Override not available in 4336/13 */
	}

	gain_val = 3*(biq1 + biq2 + digi);
	offset[0] += tr;
	offset[1] += elna;
	offset[2] += lna1;
	offset[3] += lna2;
	offset[4] += tia;
	tab.tbl_id = LCNPHY_TBL_ID_GAIN_VAL_TBL;
	tab.tbl_width = 8;
	tab.tbl_ptr = &gain_data;
	tab.tbl_len = 1;
	for (k = 0; k < 5; k++) {
		tab.tbl_offset = offset[k];
		wlc_lcnphy_read_table(pi, &tab);
		gain_data = gain_data & 0x7f;
		if (gain_data > 63)
			gain_val += (gain_data - 128);
		else
			gain_val += gain_data;
	}

	wlc_lcnphy_set_rx_gain_by_distribution(pi,
					tr,   /* TR   */
					elna, /* ELNA */
					biq2, /* BIQ2 */
					biq1, /* BIQ1 */
					tia,  /* TIA  */
					lna2, /* LNA2 */
					lna1, /* LNA1 */
					digi); /* DIGI */
	wlc_lcnphy_rx_gain_override_enable(pi, TRUE);

	/* Power cycle adc */
	PHY_REG_MOD(pi, LCNPHY, AfeCtrlOvrVal, pwdn_adc_ovr_val, 1);
	PHY_REG_MOD(pi, LCNPHY, AfeCtrlOvr, pwdn_adc_ovr, 1);
	OSL_DELAY(10);
	PHY_REG_MOD(pi, LCNPHY, AfeCtrlOvrVal, pwdn_adc_ovr_val, 0);

	wlapi_bmac_mctrl(pi->sh->physhim, MCTL_TBTT_HOLD,  0);

	return gain_val;
}

static uint32
wlc_lcnphy_get_receive_power(phy_info_t *pi, int8* gain_val)
{
	uint32 received_power = 0;
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	uint8 gain_idx = 0;
	uint8 digi_gain_boost = 0;
	uint8 gain_idx_last;
	uint16 rf_override_2;

	rf_override_2 = phy_utils_read_phyreg(pi, LCNPHY_rfoverride2);
	PHY_REG_LIST_START
		PHY_REG_MOD_ENTRY(LCNPHY, rfoverride2val, hpf1_ctrl_ovr_val, 5)
		PHY_REG_MOD_ENTRY(LCNPHY, rfoverride2val, hpf2_ctrl_ovr_val, 5)
		PHY_REG_MOD_ENTRY(LCNPHY, rfoverride2val, lpf_lq_ovr_val, 1)
		PHY_REG_MOD_ENTRY(LCNPHY, rfoverride2, hpf1_ctrl_ovr, 1)
		PHY_REG_MOD_ENTRY(LCNPHY, rfoverride2, hpf2_ctrl_ovr, 1)
		PHY_REG_MOD_ENTRY(LCNPHY, rfoverride2, lpf_lq_ovr, 1)
	PHY_REG_LIST_EXECUTE(pi);

	while ((gain_idx < 37) && (received_power < 1000)) {
		if (gain_idx == 36)
			digi_gain_boost = 3;
		else
			digi_gain_boost = 0;
		*gain_val = wlc_lcnphy_set_gain_by_index(pi, gain_idx, digi_gain_boost);
		gain_idx_last = gain_idx;
		received_power =
			wlc_lcnphy_measure_digital_power(pi, pi_lcn->lcnphy_noise_samples);
		gain_idx++;
	}

/* printf("GAIN_IDX: %d GAIN_VAL: %d PWR: %d\n", gain_idx_last, (*gain_val), received_power); */
	BCM_REFERENCE(gain_idx_last);

	wlc_lcnphy_rx_gain_override_enable(pi, FALSE);
	phy_utils_write_phyreg(pi, LCNPHY_rfoverride2, rf_override_2);
	PHY_REG_MOD(pi, LCNPHY, AfeCtrlOvr, pwdn_adc_ovr, 0);

	return received_power;
}

int32
wlc_lcnphy_rx_signal_power(phy_info_t *pi, int32 gain_index)
{
	int8 gain = 0;
	int32 nominal_power_db;
	int32 log_val, gain_mismatch, desired_gain, input_power_offset_db, input_power_db;
	int32 received_power;

	received_power = wlc_lcnphy_get_receive_power(pi, &gain);

	nominal_power_db = phy_utils_read_phyreg(pi, LCNPHY_VeryLowGainDB) >>
						   LCNPHY_VeryLowGainDB_NominalPwrDB_SHIFT;
	log_val = (int32)wlc_lcnphy_noise_log((uint32)received_power * 16);
	gain_mismatch = (nominal_power_db/2) - (log_val);

	desired_gain = (int32)gain + gain_mismatch;

	input_power_offset_db = pi->u.pi_lcnphy->unmod_rssi_offset;

	input_power_db = input_power_offset_db - desired_gain;
	if (input_power_db < -128)
		input_power_db = -128;

	return input_power_db;
}

/* set tx digital filter coefficients */
static int
wlc_lcnphy_load_tx_iir_filter(phy_info_t *pi, bool is_ofdm, int16 filt_type)
{
	int16 filt_index = -1, j;
	uint16 (*dac_coeffs_table)[LCNPHY_NUM_DIG_FILT_COEFFS+1];
	uint8 max_filter_type, max_filter_coeffs;
	uint16 *addr_coeff;
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;

	uint16 addr_cck[] = {
		LCNPHY_ccktxfilt20Stg1Shft,
		LCNPHY_ccktxfilt20CoeffStg0A1,
		LCNPHY_ccktxfilt20CoeffStg0A2,
		LCNPHY_ccktxfilt20CoeffStg0B1,
		LCNPHY_ccktxfilt20CoeffStg0B2,
		LCNPHY_ccktxfilt20CoeffStg0B3,
		LCNPHY_ccktxfilt20CoeffStg1A1,
		LCNPHY_ccktxfilt20CoeffStg1A2,
		LCNPHY_ccktxfilt20CoeffStg1B1,
		LCNPHY_ccktxfilt20CoeffStg1B2,
		LCNPHY_ccktxfilt20CoeffStg1B3,
		LCNPHY_ccktxfilt20CoeffStg2A1,
		LCNPHY_ccktxfilt20CoeffStg2A2,
		LCNPHY_ccktxfilt20CoeffStg2B1,
		LCNPHY_ccktxfilt20CoeffStg2B2,
		LCNPHY_ccktxfilt20CoeffStg2B3,
		LCNPHY_ccktxfilt20CoeffStg0_leftshift
		};

	uint16 addr_ofdm[] = {
		LCNPHY_txfilt20Stg1Shft,
		LCNPHY_txfilt20CoeffStg0A1,
		LCNPHY_txfilt20CoeffStg0A2,
		LCNPHY_txfilt20CoeffStg0B1,
		LCNPHY_txfilt20CoeffStg0B2,
		LCNPHY_txfilt20CoeffStg0B3,
		LCNPHY_txfilt20CoeffStg1A1,
		LCNPHY_txfilt20CoeffStg1A2,
		LCNPHY_txfilt20CoeffStg1B1,
		LCNPHY_txfilt20CoeffStg1B2,
		LCNPHY_txfilt20CoeffStg1B3,
		LCNPHY_txfilt20CoeffStg2A1,
		LCNPHY_txfilt20CoeffStg2A2,
		LCNPHY_txfilt20CoeffStg2B1,
		LCNPHY_txfilt20CoeffStg2B2,
		LCNPHY_txfilt20CoeffStg2B3,
		LCNPHY_txfilt20CoeffStg0_leftshift
		};

	if (is_ofdm) {
		PHY_REG_WRITE(pi, LCNPHY, txfilt20Stg1Shft, 0); /* If 0 Bypass */

		addr_coeff = (uint16 *)addr_ofdm;
		if (pi_lcn->dacrate == 160) {
			dac_coeffs_table = LCNPHY_txdigfiltcoeffs_ofdm_160;
			max_filter_type = LCNPHY_NUM_TX_DIG_FILTERS_OFDM_160;
			max_filter_coeffs = LCNPHY_NUM_DIG_FILT_COEFFS;
		} else {
			dac_coeffs_table = LCNPHY_txdigfiltcoeffs_ofdm;
			max_filter_type = LCNPHY_NUM_TX_DIG_FILTERS_OFDM;
			max_filter_coeffs = LCNPHY_NUM_DIG_FILT_COEFFS - 1;
		}
	} else {
		PHY_REG_WRITE(pi, LCNPHY, ccktxfilt20Stg1Shft, 1); /* If 0 Bypass */

		addr_coeff = (uint16 *)addr_cck;
		if (pi_lcn->dacrate == 160) {
			dac_coeffs_table = LCNPHY_txdigfiltcoeffs_cck_160;
			max_filter_type = LCNPHY_NUM_TX_DIG_FILTERS_CCK_160;
			max_filter_coeffs = LCNPHY_NUM_DIG_FILT_COEFFS;
		} else {
			dac_coeffs_table = LCNPHY_txdigfiltcoeffs_cck;
			max_filter_type = LCNPHY_NUM_TX_DIG_FILTERS_CCK;
			max_filter_coeffs = LCNPHY_NUM_DIG_FILT_COEFFS - 1;
		}
	}

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
	}

	if (wlc_lcnphy_tx_dig_iir_dc_err_comp(pi))
		wlc_lcnphy_adjust_frame_based_lo_coeffs(pi);

	/* Reset the iir filter after setting the coefficients */
	wlc_lcnphy_reset_iir_filter(pi);

	return (filt_index != -1) ? 0 : -1;
}

#undef  NOISE_CAL_CLUSTER_CHECK
#define NOISE_CAL_BYPASS_FILTER
#define PHY_NOISE_TEST_METRIC 0
#define NOISE_CAL_SHM 0

#define M_NOISE_CAL_DATA 42
#define M_NOISE_CAL_TIMEOUT 38

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

static uint16 wlc_lcnphy_noise_get_reg(phy_info_t *pi, uint16 reg);

#if PHY_NOISE_DBG_HISTORY > 0

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
wlc_lcnphy_noise_log_init(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	int8 idx = pi_lcn->noise.dbg_idx;

	pi_lcn->noise.dbg_info[idx][PHY_NOISE_DBG_INIT_CNT]++;
}

static void
wlc_lcnphy_noise_log_start(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
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
wlc_lcnphy_noise_log_adj(phy_info_t *pi, uint16 noise, int16 adj,
	int16 gain, int16 po, bool gain_change, bool po_change)
{
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
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
wlc_lcnphy_noise_log_callback(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
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
wlc_lcnphy_noise_log_tainted(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	int8 idx = pi_lcn->noise.dbg_idx;

	if (pi_lcn->noise.state !=  k_noise_adj_state)
		return;
	pi_lcn->noise.dbg_info[idx][PHY_NOISE_DBG_TAINT_CNT]++;
}

static void
wlc_lcnphy_noise_log_ucode_data_reset(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	int8 idx = pi_lcn->noise.dbg_idx;

	if (pi_lcn->noise.state !=  k_noise_adj_state)
		return;

	pi_lcn->noise.dbg_info[idx][PHY_NOISE_DBG_UCODE_DATA_C] = 0;
	pi_lcn->noise.dbg_info[idx][PHY_NOISE_DBG_UCODE_DATA_X] = 0;
}

static void
wlc_lcnphy_noise_log_ucode_data_ok(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	int8 idx = pi_lcn->noise.dbg_idx;

	if (pi_lcn->noise.state !=  k_noise_adj_state)
		return;
	pi_lcn->noise.dbg_info[idx][PHY_NOISE_DBG_NOISE_GAIN] =
		((phy_utils_read_phyreg(pi, LCNPHY_HiGainDB) & LCNPHY_HiGainDB_HiGainDB_MASK) >>
		LCNPHY_HiGainDB_HiGainDB_SHIFT);
	pi_lcn->noise.dbg_info[idx][PHY_NOISE_DBG_NOISE_CNT] = pi_lcn->noise.ucode_data_ok_cnt;
}

static void
wlc_lcnphy_noise_log_ucode_data(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	int8 idx = pi_lcn->noise.dbg_idx;
	int8 i, ucode_idx;

	if (pi_lcn->noise.state !=  k_noise_adj_state)
		return;
	pi_lcn->noise.dbg_info[idx][PHY_NOISE_DBG_UCODE_DATA_C]++;
	pi_lcn->noise.dbg_info[idx][PHY_NOISE_DBG_UCODE_DATA_L] = pi_lcn->noise.ucode_data_len;
	ucode_idx = pi_lcn->noise.ucode_data_idx;
	for (i = 0; i < k_noise_cal_ucode_data_size; i++) {
		pi_lcn->noise.dbg_info[idx][PHY_NOISE_DBG_UCODE_DATA_0 + i] =
		  (uint16)wlc_lcnphy_noise_log(pi_lcn->noise.ucode_data[ucode_idx++]);
		if (ucode_idx >= k_noise_cal_ucode_data_size)
		  ucode_idx = 0;
	}
}

static void
wlc_lcnphy_noise_log_ucode_samples(phy_info_t *pi)
{
#if PHY_NOISE_DBG_UCODE_NUM_SMPLS > 0
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	int8 i, idx = pi_lcn->noise.dbg_idx;

	pi_lcn->noise.dbg_info[idx][PHY_NOISE_DBG_UCODE_SMPLS] = 0xbeef;
	pi_lcn->noise.dbg_info[idx][PHY_NOISE_DBG_UCODE_SMPLS+1] =
	  (uint16)wlc_lcnphy_noise_log(pi_lcn->noise.ucode_data[pi_lcn->noise.ucode_data_idx]);
	pi_lcn->noise.dbg_info[idx][PHY_NOISE_DBG_UCODE_SMPLS+2] =
	  wlc_lcnphy_noise_get_reg(pi, M_55f_REG_VAL);

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
wlc_lcnphy_noise_log_bad_ucode_data(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	int8 idx = pi_lcn->noise.dbg_idx;

	if (pi_lcn->noise.state !=  k_noise_adj_state)
		return;
	pi_lcn->noise.dbg_info[idx][PHY_NOISE_DBG_UCODE_DATA_X]++;
}
#endif // endif

static void
wlc_lcnphy_noise_log_ucode_data_insert_time(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	int8 idx = pi_lcn->noise.dbg_idx;

	if (pi_lcn->noise.state !=  k_noise_adj_state)
		return;

	pi_lcn->noise.dbg_info[idx][PHY_NOISE_DBG_UCODE_DATA_T] =
	  (uint16)((R_REG(pi->sh->osh, &pi->regs->tsf_timerlow) >> 10) & 0xffff) -
	  pi_lcn->noise.start_time;
}

static void
wlc_lcnphy_noise_log_state(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	int8 idx = pi_lcn->noise.dbg_idx;

	if (pi_lcn->noise.state !=  k_noise_adj_state)
		return;
	pi_lcn->noise.dbg_info[idx][PHY_NOISE_DBG_STATE] =
	  (uint16)((phy_utils_read_phyreg(pi, LCNPHY_sslpnCalibClkEnCtrl) &
	    LCNPHY_sslpnCalibClkEnCtrl_iqEstClkEn_MASK) >>
	  LCNPHY_sslpnCalibClkEnCtrl_iqEstClkEn_SHIFT) |
	  (uint16)(((phy_utils_read_phyreg(pi, LCNPHY_crsgainCtrl) &
	    LCNPHY_crsgainCtrl_APHYGatingEnable_MASK) >>
	  LCNPHY_crsgainCtrl_APHYGatingEnable_SHIFT) << 1) |
	  (uint16)((pi_lcn->noise.enable ? 1 : 0) << 2) |
	  (uint16)((pi_lcn->noise.state & 0xf) << 4) |
	  ((uint16)pi_lcn->noise.update_step << 8);

	pi_lcn->noise.dbg_info[idx][PHY_NOISE_DBG_UCODE_CMD] =
	  (uint16)wlc_lcnphy_noise_get_reg(pi, M_NOISE_CAL_CMD);
	pi_lcn->noise.dbg_info[idx][PHY_NOISE_DBG_UCODE_RSP] =
	  (uint16)wlc_lcnphy_noise_get_reg(pi, M_NOISE_CAL_RSP);
}

static void
wlc_lcnphy_noise_reset_log(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;

	pi_lcn->noise.dbg_idx = 0;
	pi_lcn->noise.dbg_dump_idx = 0;
	pi_lcn->noise.dbg_dump_sub_idx = 0;
	bzero((void*)pi_lcn->noise.dbg_info, sizeof(pi_lcn->noise.dbg_info));
	pi_lcn->noise.dbg_dump_cmd = 0;
}

static void
wlc_lcnphy_noise_advance_log(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	int8 idx = pi_lcn->noise.dbg_idx;

	if (pi_lcn->noise.state !=  k_noise_adj_state)
		return;

	wlc_lcnphy_noise_log_state(pi);

	idx = (idx + 1) % PHY_NOISE_DBG_HISTORY;
	if (idx != pi_lcn->noise.dbg_idx) {
		bcopy((void*)&pi_lcn->noise.dbg_info[pi_lcn->noise.dbg_idx],
			(void*)&pi_lcn->noise.dbg_info[idx],
			(sizeof(pi_lcn->noise.dbg_info)/PHY_NOISE_DBG_HISTORY));
		pi_lcn->noise.dbg_idx = idx;
	}
}

static void
wlc_lcnphy_noise_dump_log(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	int8 idx = (pi_lcn->noise.dbg_idx + 1) % PHY_NOISE_DBG_HISTORY;
	int8 sub_idx, i, j;
	uint16 dbg[4];

	wlc_lcnphy_noise_log_state(pi);

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
			  printf("%c %2d %2d : %5d 0x%4x : %5d 0x%4x : %5d 0x%4x : %5d 0x%4x\n",
			    c, idx, sub_idx-3,
			    dbg[0], dbg[0], dbg[1], dbg[1], dbg[2], dbg[2], dbg[3], dbg[3]);
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
wlc_lcnphy_noise_log_dump_active(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;

	return pi_lcn->noise.dbg_dump_cmd;
}

static uint32
wlc_lcnphy_noise_log_data(phy_info_t *pi)
{
	uint32 datum;
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	int8 idx, sub_idx;

	if (!pi_lcn->noise.dbg_dump_cmd)
		return 0xdeadbeef;

	wlc_lcnphy_noise_log_state(pi);

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
wlc_lcnphy_noise_log_ioctl(phy_info_t *pi, uint32 flag)
{
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
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
		wlc_lcnphy_noise_dump_log(pi);
		retval = TRUE;
	}
	else if (flag & 4)
	{
		pi_lcn->noise.dbg_dump_cmd = 0;
		retval = TRUE;
	}
	return retval;
}

#else /* PHY_NOISE_DBG_HISTORY > 0 */

#define wlc_lcnphy_noise_log_init(a)
#define wlc_lcnphy_noise_log_start(a)
#define wlc_lcnphy_noise_log_ucode_data_ok(a)
#define wlc_lcnphy_noise_log_adj(a, b, c, d, e, f, g)
#define wlc_lcnphy_noise_log_callback(a)
#define wlc_lcnphy_noise_log_tainted(a)
#define wlc_lcnphy_noise_log_ucode_data_reset(a)
#define wlc_lcnphy_noise_log_ucode_data(a)
#define wlc_lcnphy_noise_log_bad_ucode_data(a)
#define wlc_lcnphy_noise_log_ucode_data_insert_time(a)
#define wlc_lcnphy_noise_log_state(a)
#define wlc_lcnphy_noise_reset_log(a)
#define wlc_lcnphy_noise_advance_log(a)
#define wlc_lcnphy_noise_dump_log(a)
#define wlc_lcnphy_noise_log_dump_active(a) FALSE
#define wlc_lcnphy_noise_log_data(a) 0
#define wlc_lcnphy_noise_log_ioctl(a, b) ((b))
#define wlc_lcnphy_noise_log_ucode_samples(a)
#define wlc_lcnphy_noise_log_per_start(a)

#endif /* PHY_NOISE_DBG_HISTORY > 0 */

static uint16
wlc_lcnphy_noise_get_reg(phy_info_t *pi, uint16 reg)
{
	uint16 lcnphy_shm_ptr;

	lcnphy_shm_ptr = wlapi_bmac_read_shm(pi->sh->physhim, M_SSLPNPHYREGS_PTR);
	return wlapi_bmac_read_shm(pi->sh->physhim,
		2*(lcnphy_shm_ptr + reg));
}

static void
wlc_lcnphy_noise_set_reg(phy_info_t *pi, uint16 reg, uint16 val)
{
	uint16 lcnphy_shm_ptr;

	lcnphy_shm_ptr = wlapi_bmac_read_shm(pi->sh->physhim, M_SSLPNPHYREGS_PTR);
	wlapi_bmac_write_shm(pi->sh->physhim,
		2*(lcnphy_shm_ptr + reg),
		val);
}

static bool
wlc_lcnphy_noise_sync_ucode(phy_info_t *pi, int timeout, int* p_timeout)
{
	bool ucode_sync;
	uint16 status, ucode_status, mask, val;

	ucode_sync = FALSE;
	status = wlc_lcnphy_noise_get_reg(pi, M_NOISE_CAL_CMD);
	if (status & k_noise_active_flag) {
		mask = k_noise_sync_mask;
		val = status & mask;
	} else {
		mask = k_noise_active_flag;
		val = 0;
	}
	/* Sync w/ ucode */
	do {
		ucode_status = wlc_lcnphy_noise_get_reg(pi, M_NOISE_CAL_RSP);
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
wlc_lcnphy_noise_ucode_ctrl(phy_info_t *pi, bool enable)
{
	uint16 ctrl, timeout;
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	if (enable) {
		PHY_REG_LIST_START
			PHY_REG_OR_ENTRY(LCNPHY, sslpnCalibClkEnCtrl, 0x2008)
			PHY_REG_WRITE_ENTRY(LCNPHY, sslpnphy_gpio_ctrl, 0x5)
			PHY_REG_WRITE_ENTRY(LCNPHY, sslpnphy_gpio_sel, 0xc3)
			PHY_REG_WRITE_ENTRY(LCNPHY, sslpnphy_gpio_out_en_34_32, 0x7)
			PHY_REG_WRITE_ENTRY(LCNPHY, dbg_samp_coll_start_mac_xfer_trig_timer_15_0,
				0x320)
			PHY_REG_WRITE_ENTRY(LCNPHY, dbg_samp_coll_start_mac_xfer_trig_timer_31_16,
				0x0)
			PHY_REG_WRITE_ENTRY(LCNPHY, dbg_samp_coll_end_mac_xfer_trig_timer_15_0,
				2*256)
			PHY_REG_WRITE_ENTRY(LCNPHY, dbg_samp_coll_end_mac_xfer_trig_timer_31_16,
				0x0)
		PHY_REG_LIST_EXECUTE(pi);
	}

	ctrl = wlc_lcnphy_noise_get_reg(pi, M_NOISE_CAL_CMD);
	if (enable)
		ctrl = (ctrl | k_noise_active_flag);
	else
		ctrl = (ctrl & ~k_noise_active_flag);
	ctrl += k_noise_sync_flag;
	timeout = (5*pi_lcn->noise.update_ucode_interval[pi_lcn->noise.update_step]);
	if (timeout < 5)
		timeout = 5;
	wlc_lcnphy_noise_set_reg(pi, M_NOISE_CAL_TIMEOUT, timeout);
	wlc_lcnphy_noise_set_reg(pi, M_NOISE_CAL_CMD, ctrl);
}

static void
wlc_lcnphy_noise_save_phy_regs(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	int16 xx;

	/* Save values set in baseband_init */
	pi_lcn->noise.high_gain =
	((phy_utils_read_phyreg(pi, LCNPHY_HiGainDB) &
		LCNPHY_HiGainDB_HiGainDB_MASK) >> LCNPHY_HiGainDB_HiGainDB_SHIFT);
	pi_lcn->noise.nf_substract_val =
		(phy_utils_read_phyreg(pi, LCNPHY_nfSubtractVal) & 0x3ff);
	pi_lcn->noise.input_pwr_offset =
		(phy_utils_read_phyreg(pi, LCNPHY_InputPowerDB)
		& LCNPHY_InputPowerDB_inputpwroffsetdb_MASK);
	if (pi_lcn->noise.input_pwr_offset > 127)
		pi_lcn->noise.input_pwr_offset -= 256;
	xx = (int16)(phy_utils_read_phyreg(pi, LCNPHY_crsedthresh) &
		LCNPHY_crsedthresh_edonthreshold_MASK) >>
		LCNPHY_crsedthresh_edonthreshold_SHIFT;
	if (xx > 127)
		xx -= 256;
	pi_lcn->lcnphy_aci.EdOn_Thresh_BASE = (int8)xx;
}

static void
wlc_lcnphy_noise_restore_phy_regs(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;

	PHY_REG_MOD(pi, LCNPHY, InputPowerDB, inputpwroffsetdb, pi_lcn->noise.input_pwr_offset);
	PHY_REG_MOD(pi, LCNPHY, HiGainDB, HiGainDB, (pi_lcn->noise.high_gain));
	PHY_REG_WRITE(pi, LCNPHY, nfSubtractVal, pi_lcn->noise.nf_substract_val);

	PHY_REG_LIST_START
		/* Reset radio ctrl and crs gain */
		PHY_REG_OR_ENTRY(LCNPHY, resetCtrl, 0x44)
		PHY_REG_WRITE_ENTRY(LCNPHY, resetCtrl, 0x80)
	PHY_REG_LIST_EXECUTE(pi);

	OSL_DELAY(20); /* To give AGC/radio ctrl chance to initialize */
}

#ifdef NOISE_CAL_BYPASS_FILTER

#define k_noise_cal_ref_2g_elna 52
#define k_noise_cal_ref_2g 48
#define k_noise_cal_ref_5g_elna 52
#define k_noise_cal_ref_5g 48

#else

#define k_noise_cal_ref_2g_elna 57
#define k_noise_cal_ref_2g 53
#define k_noise_cal_ref_5g_elna 57
#define k_noise_cal_ref_5g 53

#endif /* NOISE_CAL_BYPASS_FILTER */

static void
wlc_lcnphy_noise_set_input_pwr_offset(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	int16 input_pwr_offset, rxgain_tempcorr;
	uint8 channel = CHSPEC_CHANNEL(pi->radio_chanspec);
	uint8 i = 0;

	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		input_pwr_offset = pi_lcn->noise.nvram_input_pwr_offset_2g;
		rxgain_tempcorr = pi_lcn->rxgain_tempcorr_2g;
	} else {
		if	(channel >= 149) {
			i = 2;
			rxgain_tempcorr = pi_lcn->rxgain_tempcorr_5gh;
		}
		else if (channel >= 100) {
			i = 1;
			rxgain_tempcorr = pi_lcn->rxgain_tempcorr_5gm;
		}
		else {
			i = 0;
			rxgain_tempcorr = pi_lcn->rxgain_tempcorr_5gl;
		}

		input_pwr_offset = pi_lcn->noise.nvram_input_pwr_offset_5g[i];
	}
	if (input_pwr_offset != 0xff) {
		int8 curtemp;
		int16 po_corr;

		curtemp = wlc_lcnphy_tempsense_degree(pi, 1);
		po_corr = (curtemp - 25) * (rxgain_tempcorr);
		/* rounding and scaling it back */
		if (po_corr > 0)
			po_corr = (po_corr + 500)/1000;
		else
			po_corr = (po_corr - 500)/1000;

		input_pwr_offset = input_pwr_offset + po_corr;

		/* Input pwr offset set by nvram param */
		phy_utils_mod_phyreg(pi, LCNPHY_InputPowerDB,
			LCNPHY_InputPowerDB_inputpwroffsetdb_MASK,
			(input_pwr_offset << LCNPHY_InputPowerDB_inputpwroffsetdb_SHIFT));
		pi_lcn->noise.input_pwr_offset = input_pwr_offset; /* save starting value */
	}
}

static void
wlc_lcnphy_noise_init(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	int8 gain_tbl_adj;

	wlc_lcnphy_noise_log_init(pi);

	if (CHIPID(pi->sh->chip) == BCM4313_CHIP_ID) {
		pi_lcn->noise.enable = TRUE;
		pi_lcn->noise.global_adj_en = TRUE;
	} else {
		if (CHSPEC_IS2G(pi->radio_chanspec))
			pi_lcn->noise.enable = pi_lcn->noise.nvram_enable_2g;
		else
			pi_lcn->noise.enable = pi_lcn->noise.nvram_enable_5g;
#if PHY_NOISE_TEST_METRIC
		pi_lcn->noise.global_adj_en = FALSE;
#else
		pi_lcn->noise.global_adj_en = pi_lcn->noise.enable;
#endif // endif
	}

	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		/* Set ref level for 2G band */
		if (pi_lcn->noise.nvram_ref_2g > 0) {
			pi_lcn->noise.ref = pi_lcn->noise.nvram_ref_2g;
		} else {
			if (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA) {
				pi_lcn->noise.ref = k_noise_cal_ref_2g_elna;
			} else {
				pi_lcn->noise.ref = k_noise_cal_ref_2g;
			}
		}
		gain_tbl_adj = pi_lcn->noise.nvram_gain_tbl_adj_2g;
	} else {
		/* Set ref level for 5G band */
		if (pi_lcn->noise.nvram_ref_5g > 0) {
			pi_lcn->noise.ref = pi_lcn->noise.nvram_ref_5g;
		} else {
			if (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA_5GHz) {
				pi_lcn->noise.ref = k_noise_cal_ref_5g_elna;
			} else {
				pi_lcn->noise.ref = k_noise_cal_ref_5g;
			}
		}
		gain_tbl_adj = pi_lcn->noise.nvram_gain_tbl_adj_5g;
	}

	/* Account for fixed(i.e not varying w/ temperature/channel) gain differences */
	if (gain_tbl_adj > 0) {
		int16 tbl_offset, tbl_offset_ofdm, tbl_offset_dsss;
		int8 tbl_data[2];
		phytbl_info_t tab;

		if (gain_tbl_adj > 2)
			gain_tbl_adj = 2;
		gain_tbl_adj = gain_tbl_adj * 3;

		/* Modify gain idx table offset */
		tbl_offset = phy_utils_read_phyreg(pi, LCNPHY_gainidxoffset);
		tbl_offset_ofdm = (tbl_offset & 0xff);
		tbl_offset_dsss = (tbl_offset >> 8) & 0xff;
		if (tbl_offset_ofdm > 127)
			tbl_offset_ofdm -= 256;
		if (tbl_offset_dsss > 127)
			tbl_offset_dsss -= 256;
		tbl_offset_ofdm -= (int16)gain_tbl_adj;
		tbl_offset_dsss -= (int16)gain_tbl_adj;
		tbl_offset = (tbl_offset_ofdm & 0xff) |
			((tbl_offset_dsss & 0xff) << 8);
		phy_utils_write_phyreg(pi, LCNPHY_gainidxoffset, tbl_offset);

		/* Modify gain value table */
		tab.tbl_ptr = tbl_data; /* ptr to buf */
		tab.tbl_len = 2;
		tab.tbl_id = LCNPHY_TBL_ID_GAIN_VAL_TBL; /* gain_val_tbl */
		if (LCNREV_GE(pi->pubpi.phy_rev, 2))
			tab.tbl_offset = 15; /* tbl offset */
		else
			tab.tbl_offset = 56; /* tbl offset */
		tab.tbl_width = 8; /* 8 bit wide */
		wlc_lcnphy_read_table(pi, &tab);
		tbl_data[0] -= gain_tbl_adj;
		tbl_data[1] -= gain_tbl_adj;
		wlc_lcnphy_write_table(pi, &tab);

		/* Reset radio ctrl and agc after changing table */
		wlc_lcnphy_agc_reset(pi);
	}

	pi_lcn->noise.update_step_interval[k_noise_cal_update_steps-1] = 255;

	pi_lcn->noise.noise_cb = FALSE;
	pi_lcn->lcnphy_aci.init_noise_cal_done = FALSE;

	if (pi_lcn->noise.nvram_dbg_noise) {
		/* To force gain to a particular value */
#if !PHY_NOISE_TEST_METRIC
		pi_lcn->noise.enable = FALSE;
		pi_lcn->noise.global_adj_en = FALSE;
#endif /* PHY_NOISE_TEST_METRIC */

		PHY_REG_MOD(pi, LCNPHY, HiGainDB, HiGainDB,
		            pi_lcn->noise.nvram_high_gain);
		PHY_REG_WRITE(pi, LCNPHY, nfSubtractVal,
		              (uint16)pi_lcn->noise.nvram_nf_substract_val);

		PHY_REG_LIST_START
		        /* Reset radio ctrl and crs gain */
		        PHY_REG_OR_ENTRY(LCNPHY, resetCtrl, 0x44)
		        PHY_REG_WRITE_ENTRY(LCNPHY, resetCtrl, 0x80)
		PHY_REG_LIST_EXECUTE(pi);
	}

	wlc_lcnphy_noise_save_phy_regs(pi);
	if (CHIPID(pi->sh->chip) == BCM4313_CHIP_ID) {
		wlc_lcnphy_aci_init(pi);
	}
}

static int8
wlc_lcnphy_noise_log(uint32 x)
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
wlc_lcnphy_noise_adj(phy_info_t *pi, uint32 metric)
{
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	int16 high_gain, high_gain_old, input_pwr_offset, max_neg_po_adj;
	int8 log_metric, adj;
	bool done = TRUE;
	bool adj_en = pi_lcn->noise.adj_en;

	max_neg_po_adj = k_noise_cal_max_negative_po_adj;
	/* Increase range of adjustment in boards w/o eLNA */
	if (!(BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA))
		max_neg_po_adj += 3;

	/* Get current settings */
	high_gain = ((phy_utils_read_phyreg(pi, LCNPHY_HiGainDB) &
		LCNPHY_HiGainDB_HiGainDB_MASK) >> LCNPHY_HiGainDB_HiGainDB_SHIFT);
	high_gain_old = high_gain;
	input_pwr_offset = ((phy_utils_read_phyreg(pi, LCNPHY_InputPowerDB) &
		LCNPHY_InputPowerDB_inputpwroffsetdb_MASK) >>
		LCNPHY_InputPowerDB_inputpwroffsetdb_SHIFT);
	if (input_pwr_offset > 127)
		input_pwr_offset -= 256;

	/* Calculate adjustment */
	log_metric = wlc_lcnphy_noise_log(metric);
	adj = (log_metric - pi_lcn->noise.ref)
		/* Account for different gain setting used to measure noise */
		- (high_gain - pi_lcn->noise.high_gain);

	if (!pi_lcn->noise.adj_en) {
		wlc_lcnphy_noise_log_adj(pi, log_metric, adj, 255, 255, TRUE, TRUE);
		return TRUE;
	}

	if (adj < 0) {
		int16 nf_adj, nf_scale, nf_max;

		nf_adj = -adj;
		if (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA) {
		  nf_scale = 4;
		  nf_max = 3;
		} else {
		  nf_scale = 11;
		  nf_max = 8;
		}
		if (nf_adj > nf_max)
		  nf_adj = nf_max;
		  PHY_REG_WRITE(pi, LCNPHY, nfSubtractVal,
		    (pi_lcn->noise.nf_substract_val + nf_scale*nf_adj));
	}

	/* Adjust high gain */
	if (adj < k_noise_cal_gain_adj_increase_threshold)
		high_gain += 3;
	else if (adj > k_noise_cal_gain_adj_decrease_threshold)
		high_gain -= 3;

	if (high_gain > (pi_lcn->noise.high_gain + k_noise_cal_max_gain_delta))
		high_gain = (pi_lcn->noise.high_gain + k_noise_cal_max_gain_delta);
	else if (high_gain < (pi_lcn->noise.high_gain - k_noise_cal_min_gain_delta))
		high_gain = (pi_lcn->noise.high_gain - k_noise_cal_min_gain_delta);

	if (adj_en && (high_gain != high_gain_old)) {
		PHY_REG_MOD(pi, LCNPHY, HiGainDB, HiGainDB, high_gain);
		PHY_REG_LIST_START
			/* Reset radio ctrl and crs gain */
			PHY_REG_OR_ENTRY(LCNPHY, resetCtrl, 0x44)
			PHY_REG_WRITE_ENTRY(LCNPHY, resetCtrl, 0x80)
		PHY_REG_LIST_EXECUTE(pi);

		OSL_DELAY(20); /* To give AGC/radio ctrl chance to initialize */

		done = FALSE;
	}

	/* Adjust input pwr offset */
	input_pwr_offset = -(adj);
	if (input_pwr_offset < (pi_lcn->noise.input_pwr_offset - max_neg_po_adj))
	  input_pwr_offset = (pi_lcn->noise.input_pwr_offset - max_neg_po_adj);
	if (input_pwr_offset > (pi_lcn->noise.input_pwr_offset + k_noise_cal_max_positive_po_adj))
	  input_pwr_offset = (pi_lcn->noise.input_pwr_offset + k_noise_cal_max_positive_po_adj);
	if (CHSPEC_IS2G(pi->radio_chanspec))
		input_pwr_offset += pi_lcn->noise.nvram_po_bias_2g;
	else
		input_pwr_offset += pi_lcn->noise.nvram_po_bias_5g;

	if (adj_en) {
		PHY_REG_MOD(pi, LCNPHY, InputPowerDB, inputpwroffsetdb, input_pwr_offset);
	}

	wlc_lcnphy_noise_log_adj(pi,
		log_metric,
		adj,
		high_gain,
		input_pwr_offset,
		!done,
		adj_en);

	pi_lcn->lcnphy_aci.Listen_GaindB_AfrNoiseCal =
	        ((phy_utils_read_phyreg(pi, LCNPHY_HiGainDB) &
		LCNPHY_HiGainDB_HiGainDB_MASK) >> LCNPHY_HiGainDB_HiGainDB_SHIFT);
	pi_lcn->lcnphy_aci.Latest_Listen_GaindB_Latch =
		pi_lcn->lcnphy_aci.Listen_GaindB_AfrNoiseCal;
	pi_lcn->lcnphy_aci.EdOn_Thresh_Latch = pi_lcn->lcnphy_aci.EdOn_Thresh_BASE;
	return done;
}

static bool
wlc_lcnphy_noise_metric(phy_info_t* pi, uint32* metric, uint32* power)
{
	int16 data[(2 << k_noise_log2_nsamps)];
	uint16 s;
	int32 dc[2];
	uint32 m_l, m_u, m_p;
	int data_offset, dc_offset, i;

#if NOISE_CAL_SHM
	/* Read i/q data from shared memmory */
	dc[0] = dc[1] = 0;
	s = data_offset = dc_offset = 0;
	for (i = 0; i < (2 << k_noise_log2_nsamps); i++) {
		int16 x;

		x = wlc_lcnphy_noise_get_reg(pi, (M_NOISE_CAL_DATA + i));
		s = (s << 4) | ((x >> 12) & 0xc);
		x = (x & 0x3fff);
		if (x > 8191) /* rx filt output is 14 bit integer */
			x -= 16384;
		if ((x > 2048) || (x < -2048))
			break; /* Avoid overflows */
#if (PHY_NOISE_DBG_HISTORY > 0) && (PHY_NOISE_DBG_UCODE_NUM_SMPLS > 0)
		pi->u.pi_lcnphy->noise.dbg_samples[i] = x;
#endif // endif
		data[data_offset] = x;
		dc[dc_offset] += x; /* pre-dc canceller data :( */
		data_offset ^= (1<<k_noise_log2_nsamps);
		dc_offset ^= 1;
		if (i & 1) {
			if ((s & 0xff) != 0x40) {
				break;
			}
			data_offset++;
		}
	}
#else
	/* Read i/q data from spb table */
#define k_noise_spb_pad_samples 8
	int n = (k_noise_spb_samples << 1);
	int n_max_pad;
	int n_pad;

	dc[0] = dc[1] = 0;
	s = data_offset = dc_offset = 0;

	phy_utils_phyreg_enter(pi);

	wlapi_bmac_mctrl(pi->sh->physhim, MCTL_TBTT_HOLD,  MCTL_TBTT_HOLD);
	OSL_DELAY(5);

	PHY_REG_LIST_START
		PHY_REG_MOD_ENTRY(LCNPHY, lpphyCtrl, resetCCA, 0)
		PHY_REG_WRITE_ENTRY(LCNPHY, TableAddress, 0x5400)
	PHY_REG_LIST_EXECUTE(pi);

	if ((phy_utils_read_phyreg(pi, LCNPHY_TabledataLo) & 0xf) == 5)
	  phy_utils_write_phyreg(pi, LCNPHY_TableAddress, 0x5400);
	else {
	  phy_utils_write_phyreg(pi, LCNPHY_TableAddress, 0x5401);
	  n -= 2;
	}
	/* n is total number of available samples in SPB */
	n_max_pad = (n - (2<<k_noise_log2_nsamps))>>1;
	if (n_max_pad > (k_noise_spb_pad_samples << 1))
		n_max_pad = (k_noise_spb_pad_samples << 1);
	else if (n_max_pad < 0)
		n_max_pad = 0;
	/* n_max_pad is number of samples at begining/end where state will be checked
	 * not used for metric
	*/
	n = (n_max_pad << 1) + (2<<k_noise_log2_nsamps);
	/* n is number of samples read from SPB */
	i = 0;
	n_pad = n - n_max_pad;
	/* n_pad is used to know when to start saving data */
	while (n > 0) {
		int32 x;

		x = ((int32)phy_utils_read_phyreg(pi, LCNPHY_TabledataLo) >> 4) & 0xfff;
		x |= ((int32)phy_utils_read_phyreg(pi, LCNPHY_TabledataHi) << 12);
		s = (s << 4) | (((uint16)x >> 12) & 0xc);
		if ((n <= n_pad) && (i < (2<<k_noise_log2_nsamps))) {
			x = x & 0x3fff;
			if (x > 8192) /* rx filt output is 14 bit integer */
				x -= 16384;
			if ((x > 2048) || (x < -2048))
				break; /* Avoid overflows */
			data[data_offset] = (int16)x;
			dc[dc_offset] += x;
			data_offset += dc_offset;
			data_offset ^= (1<<k_noise_log2_nsamps);
			dc_offset ^= 1;
			i++;
		}
		if ((n & 1) && ((s & 0xff) != 0x40))
			break; /* State is not 1  -- don't use data */
		n--;
	}

	wlapi_bmac_mctrl(pi->sh->physhim, MCTL_TBTT_HOLD,  0);

	phy_utils_phyreg_exit(pi);

#endif /* NOISE_CAL_SHM */
	dc[0] >>= k_noise_log2_nsamps;
	dc[1] >>= k_noise_log2_nsamps;

	if (i != (2<<k_noise_log2_nsamps))
		return FALSE;

	/* Calculate power in upper/lower subband */
	m_p = m_l = m_u = 0;
	for (i = 0; i < 2; i++) {
		int32 x;
		int j;
#ifndef NOISE_CAL_BYPASS_FILTER
		int32 t, y, y0, y1, y_l, y_u, d[6+1];
		int k = 0;

		bzero((void*)d, sizeof(d));
#endif // endif
		for (j = 0; j < (1<<k_noise_log2_nsamps); j++) {
			x  = (int32)data[(i<<k_noise_log2_nsamps) | j] - dc[i];
#ifndef NOISE_CAL_BYPASS_FILTER
			t  = (x << k_noise_ap0_Q) - ((d[k] + 1)>>1);
			y  = (x << k_noise_ap0_Q) + ((t+1)>>1) + d[k];
			d[k] = t;
			t  = (y << k_noise_ap1_Q) - ((d[k | 2] + 2) >> 2);
			y0 = ((t + 2) >> 2) + d[k | 2];
			d[k | 2] = t;
			t  = (d[6] << k_noise_ap1_Q) - ((d[k | 4] + 2) >> 2) -
				((d[k | 4] + 1) >> 1);
			y1 = ((t + 2) >> 2) + ((t + 1) >> 1) + d[k | 4];
			d[k | 4] = t;
			d[6] = y;
			k = k ^ 1;
			y_l = ((y0 + y1 + k_noise_rnd) >> k_noise_shft);
			y_u = ((y0 - y1 + k_noise_rnd) >> k_noise_shft);
			m_l += (y_l * y_l);
			m_u += (y_u * y_u);
#endif /* NOISE_CAL_BYPASS_FILTER */
			m_p += (x * x);
		}
	}
	*power = m_p;
#ifdef NOISE_CAL_BYPASS_FILTER
	*metric = (m_p >> k_noise_log2_nsamps);
#else
	m_l = m_l >> k_noise_log2_nsamps;
	/* Scaling is to account for LPF response so that noise metric is the same without spurs */
	m_u = (13*m_u) >> (k_noise_log2_nsamps + 3);

	if (m_l < m_u)
		*metric = m_l;
	else
		*metric = m_u;
#endif // endif

	if ((*metric < k_noise_min_metric) || (*metric > k_noise_max_metric))
		return FALSE;

	return TRUE;
}

static void
wlc_lcnphy_noise_reset(phy_info_t *pi, bool restore_regs)
{
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;

	pi_lcn->noise.state = k_noise_end_state;
	wlc_lcnphy_noise_ucode_ctrl(pi, FALSE);
	if (restore_regs)
		wlc_lcnphy_noise_restore_phy_regs(pi);

	PHY_REG_MOD2(pi, LCNPHY, sslpnphy40_soft_reset,
		samp_coll_soft_reset, gpio_soft_reset, 1, 1);
	PHY_REG_MOD2(pi, LCNPHY, sslpnCalibClkEnCtrl,
		samplePlayClkEn, debugClkEn, 0, 0);
}

static void
wlc_lcnphy_noise_reset_data(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;

	pi_lcn->noise.ucode_data_len = 0;
	pi_lcn->noise.ucode_data_idx = 0;
	pi_lcn->noise.ucode_data_ok_cnt = 0;
	pi_lcn->noise.update_cnt = 0;
	pi_lcn->noise.update_step = 0;

	wlc_lcnphy_noise_log_ucode_data(pi);
	wlc_lcnphy_noise_log_ucode_data_reset(pi);
}

void
wlc_lcnphy_aci_noise_measure(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;

	if (!pi_lcn->noise.enable)
	  return;

	if (NORADIO_ENAB(pi->pubpi))
		return;

	if ((pi->cur_interference_mode != WLAN_AUTO_W_NOISE) &&
		(pi->cur_interference_mode != WLAN_AUTO)) {
		return;
	}

	if (pi_lcn->lcnphy_aci.init_noise_cal_done &&
		(pi->cur_interference_mode == WLAN_AUTO_W_NOISE))
		wlc_lcnphy_aci_upd(pi);

	if (!(pi->aci_state & ACI_ACTIVE))  {
		if (!(PLT_INPROG_PHY(pi) || ASSOC_INPROG_PHY(pi)))
			wlc_lcnphy_noise_measure(pi);
	} else if ((pi_lcn->lcnphy_aci.init_noise_cal_done == 1) && (pi->aci_state & ACI_ACTIVE)) {
		if (pi_lcn->lcnphy_aci.gain_backoff == 1) {
			/* Fixed 3dB Listen Gain BackOff Which Decided by Auto Aci Logic */
			pi_lcn->lcnphy_aci.Latest_Listen_GaindB_Latch =
				pi_lcn->lcnphy_aci.Latest_Listen_GaindB_Latch - 3;
			if (pi_lcn->lcnphy_aci.Latest_Listen_GaindB_Latch <=
				(pi_lcn->noise.high_gain - 21)) {
				pi_lcn->lcnphy_aci.Latest_Listen_GaindB_Latch =
					pi_lcn->noise.high_gain - 21;
			}
			pi_lcn->lcnphy_aci.EdOn_Thresh_Latch =
				pi_lcn->lcnphy_aci.EdOn_Thresh_Latch + 3;
			if (pi_lcn->lcnphy_aci.EdOn_Thresh_Latch >=
				(pi_lcn->lcnphy_aci.EdOn_Thresh_BASE + 21)) {
				pi_lcn->lcnphy_aci.EdOn_Thresh_Latch =
					pi_lcn->lcnphy_aci.EdOn_Thresh_BASE + 21;
			}
			PHY_REG_MOD(pi, LCNPHY, HiGainDB, HiGainDB,
				pi_lcn->lcnphy_aci.Latest_Listen_GaindB_Latch);
			PHY_REG_MOD(pi, LCNPHY, crsedthresh, edonthreshold,
				pi_lcn->lcnphy_aci.EdOn_Thresh_Latch);
			/* Reset radio ctrl and crs gain */
			PHY_REG_LIST_START
				PHY_REG_OR_ENTRY(LCNPHY, resetCtrl, 0x44)
				PHY_REG_WRITE_ENTRY(LCNPHY, resetCtrl, 0x80)
			PHY_REG_LIST_EXECUTE(pi);
		}
	}

}

void
wlc_lcnphy_noise_measure_start(phy_info_t *pi, bool adj_en)
{
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;

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
	wlc_lcnphy_noise_measure_stop(pi);
	pi_lcn->noise.adj_en = adj_en && pi_lcn->noise.global_adj_en;
	if (pi_lcn->noise.adj_en)
		wlc_lcnphy_noise_restore_phy_regs(pi);
	wlc_lcnphy_noise_reset_data(pi);
	wlc_lcnphy_noise_ucode_ctrl(pi, TRUE);

	wlc_lcnphy_noise_log_start(pi);
}

void
wlc_lcnphy_noise_measure_stop(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;

	if (!pi_lcn->noise.enable)
		return;

	wlc_lcnphy_noise_ucode_ctrl(pi, FALSE);
	/* This is called from beginning of watchdog. If ucode cant
	 * be stopped before exiting this function the noise measurement
	 * may be invalid b/c of a cal....
	 */
	if (!wlc_lcnphy_noise_sync_ucode(pi, k_noise_sync_stop_timeout, NULL))
		pi_lcn->noise.tainted = TRUE;
}

static void
wlc_lcnphy_noise_attach(phy_info_t *pi)
{

	wlc_lcnphy_noise_reset(pi, FALSE);
	wlc_lcnphy_noise_reset_data(pi);
	wlc_lcnphy_noise_reset_log(pi);
}

static void
wlc_lcnphy_noise_cb(phy_info_t *pi, uint32 metric)
{
	int16 noise_dbm;
	uint16 channel;

	/*                       ADC     rx filt          num of samples
	 * phy_scale =  20*log((2^9/0.4) * (2^4) )  + 10*log(  2  *   64 ) = 107
	 * noise_dBm = -174 + 10*log(20*10^6) = -101dBm
	 * 10log(noise_v^2) = noise_dBm + 30 + 10*log(50) = -54
	 * 10log(metric) =  10log(2*noise_v^2) + phy_scale = 56
	 */
	noise_dbm = (wlc_lcnphy_noise_log(metric) - 56) - 101;
	if (noise_dbm < -98) /* Limit to a reasonable value */
		noise_dbm = -98;
	channel = CHSPEC_CHANNEL(pi->radio_chanspec);

	wlc_phy_noise_cb(pi, (uint8)channel, (int8)noise_dbm);
}

void
wlc_lcnphy_noise_measure(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	uint32 metric, power;

	metric = power = 0;

	/* This proc is called from noise measure callback or from watchdog timer */
	if (!(pi_lcn->noise.enable &&
	      /* Only run if not complete */
	      (pi_lcn->noise.state != k_noise_end_state) &&
	      /* No point in running this if ucode is not active */
	      (R_REG(pi->sh->osh, &pi->regs->maccontrol) & MCTL_EN_MAC)))
		return;

	/* wlc_lcnphy_noise_log_callback(pi); */

	if (!pi_lcn->noise.tainted &&
	    wlc_lcnphy_noise_sync_ucode(pi, 0, NULL) &&
#if PHY_NOISE_TEST_METRIC
	    (pi_lcn->noise.update_data_interval[pi_lcn->noise.update_step] != 255) &&
#endif // endif
	    wlc_lcnphy_noise_metric(pi, &metric, &power)) {
		uint32 tmp, avg_metric;
		int8 i, j;
		bool done;

		wlc_lcnphy_noise_log_callback(pi);

		pi_lcn->noise.power = power;

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
					/* wlc_lcnphy_noise_log_ucode_data_insert_time(pi); */
					if (i == 0) {
						wlc_lcnphy_noise_log_ucode_data_insert_time(pi);
						wlc_lcnphy_noise_log_ucode_samples(pi);
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
					wlc_lcnphy_noise_log_ucode_data_insert_time(pi);
					wlc_lcnphy_noise_log_ucode_samples(pi);
				}
				i++;
				if (++j >= k_noise_cal_ucode_data_size)
					j = 0;
			}
			if (pi_lcn->noise.ucode_data_len < k_noise_cal_ucode_data_size) {
				pi_lcn->noise.ucode_data_ok_cnt = 0;
				pi_lcn->noise.ucode_data_len++;
			}

			wlc_lcnphy_noise_log_ucode_data(pi);

			if (pi_lcn->noise.ucode_data_len == k_noise_cal_ucode_data_size) {
				uint8 data_int, step_int;

				data_int =
				pi_lcn->noise.update_data_interval[pi_lcn->noise.update_step];
				step_int =
				pi_lcn->noise.update_step_interval[pi_lcn->noise.update_step];

				if (pi_lcn->noise.ucode_data_ok_cnt < 253)
					pi_lcn->noise.ucode_data_ok_cnt++;

				wlc_lcnphy_noise_log_ucode_data_ok(pi);

				if (pi_lcn->noise.ucode_data_ok_cnt == data_int) {
					/* Throw away max and min periodically */
					if (++(pi_lcn->noise.ucode_data_idx) >=
						k_noise_cal_ucode_data_size)
						pi_lcn->noise.ucode_data_idx = 0;
					pi_lcn->noise.ucode_data_len -= 2;

					/* Calculate adjustment */
					if (!wlc_lcnphy_noise_adj(pi, avg_metric)) {
						/* Adjusted listen gain, so
						 * need to throw away old data
						 */
						wlc_lcnphy_noise_advance_log(pi);
						wlc_lcnphy_noise_reset_data(pi);
					} else {
						if (pi_lcn->noise.update_cnt < 253)
							pi_lcn->noise.update_cnt++;
						if ((pi_lcn->noise.update_cnt == step_int) &&
							(pi_lcn->noise.update_step <
								(k_noise_cal_update_steps-1)))
							pi_lcn->noise.update_step++;
					}
					pi_lcn->noise.ucode_data_ok_cnt = 0;
				}
			}
		} /* if ( pi_lcn->noise.state == k_noise_adj_state ) */
	} else {
		if (pi_lcn->noise.tainted) {
			wlc_lcnphy_noise_log_tainted(pi);
		}
	}

	/* Kick off new measurement if needed */
	pi_lcn->noise.tainted = FALSE;
	if (pi_lcn->noise.state == k_noise_adj_state) {
		wlc_lcnphy_noise_ucode_ctrl(pi, TRUE);
	} else {
		wlc_lcnphy_noise_reset(pi, FALSE);
	}

	/* Callback for periodic noise measurement */
	wlc_lcnphy_noise_cb(pi, pi_lcn->noise.power);
	pi_lcn->noise.noise_cb = FALSE;

	if (!pi_lcn->lcnphy_aci.init_noise_cal_done) {

		pi_lcn->lcnphy_aci.init_noise_cal_done = TRUE;
	}
}

void
wlc_lcnphy_dummytx(wlc_phy_t *ppi, uint16 nframes, uint16 wait_delay)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	uint8 counter = 0;
	uint16 max_pwr_idx = 0;
	uint16 min_pwr_idx = 127;
	uint16 current_txidx = 0;
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;

	wlc_btcx_override_enable(pi);
	wlc_lcnphy_deaf_mode(pi, TRUE);

	for (counter = 0; counter < nframes; counter ++) {
		wlc_phy_do_dummy_tx(pi, TRUE, OFF);
		OSL_DELAY(wait_delay);
		current_txidx = wlc_lcnphy_get_current_tx_pwr_idx(pi);
		if (current_txidx > max_pwr_idx)
			max_pwr_idx = current_txidx;
		if (current_txidx < min_pwr_idx)
			min_pwr_idx = current_txidx;
	}

	wlc_lcnphy_deaf_mode(pi, FALSE);

	pi_lcn->lcnphy_start_idx = (uint8)current_txidx; 	/* debug information */
}

void
wlc_lcnphy_papd_recal(phy_info_t *pi)
{
	uint16 tx_pwr_ctrl;
	bool suspend;
	uint16 current_txidx = 0;
	lcnphy_txcalgains_t txgains;
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	uint16 nframes;
	uint16 wait_delay;
	uint8 npt;

	nframes = 50;
	wait_delay = 10*100;
#ifndef PHYCAL_CACHING
	pi_lcn->lcnphy_full_cal_channel = CHSPEC_CHANNEL(pi->radio_chanspec);
#endif // endif
	suspend = !(R_REG(pi->sh->osh, &pi->regs->maccontrol) & MCTL_EN_MAC);
	if (!suspend) {
		/* Set non-zero duration for CTS-to-self */
		wlapi_bmac_write_shm(pi->sh->physhim, M_CTS_DURATION, 10000);
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
	}
#ifdef MULTICHAN_4313
	if (!pi_lcn->tx_pwr_idx[CHSPEC_CHANNEL(pi->radio_chanspec)] &&
		(CHIPID(pi->sh->chip) == BCM4313_CHIP_ID) && !pi_lcn->ePA)
	{
#endif /* MULTICHAN_4313 */
		/* Save npt */
		npt = wlc_lcnphy_get_tx_pwr_npt(pi);

		/* Save tx power control mode */
		tx_pwr_ctrl = wlc_lcnphy_get_tx_pwr_ctrl(pi);

		/* Disable tx power control */
		wlc_lcnphy_set_tx_pwr_ctrl(pi, LCNPHY_TX_PWR_CTRL_OFF);

		/* Enable pwr ctrl */
		wlc_lcnphy_set_tx_pwr_ctrl(pi, LCNPHY_TX_PWR_CTRL_HW);

		/* clear all offsets */
		wlc_lcnphy_clear_tx_power_offsets(pi);

		/* set target pwr for papd */
		wlc_lcnphy_set_target_tx_pwr(pi, 64);

		/* Setting npt to 0 for index settling with 30 frames */
		wlc_lcnphy_set_tx_pwr_npt(pi, 0);

		PHY_TRACE(("dummy TX Start Called\n"));

		/* transmit dummy tx pkts and find the txidx */
		wlc_lcnphy_dummytx((wlc_phy_t *)pi, nframes, wait_delay);

		current_txidx = pi_lcn->lcnphy_start_idx; 	/* debug information */

#ifdef MULTICHAN_4313
		pi_lcn->tx_pwr_ctrl[CHSPEC_CHANNEL(pi->radio_chanspec)] = tx_pwr_ctrl;
		pi_lcn->tx_pwr_idx[CHSPEC_CHANNEL(pi->radio_chanspec)] = pi_lcn->lcnphy_start_idx;
#endif  /* MULTICHAN_4313 */

		wlc_lcnphy_deaf_mode(pi, TRUE);

		wlc_btcx_override_enable(pi);

		/* Restore npt */
		wlc_lcnphy_set_tx_pwr_npt(pi, npt);

		/* Disable tx power control */
		wlc_lcnphy_set_tx_pwr_ctrl(pi, LCNPHY_TX_PWR_CTRL_OFF);

		if (pi_lcn->lcnphy_CalcPapdCapEnable == 1) {
			wlc_lcnphy_papd_calc_capindex(pi, &txgains);
			wlc_lcnphy_load_txgainwithcappedindex(pi, 1);
			/* set target pwr for papd = 15.5 dbm */
			wlc_lcnphy_set_target_tx_pwr(pi, 62);

			/* Setting npt to 0 for index settling with 30 frames */
			wlc_lcnphy_set_tx_pwr_npt(pi, 0);

			PHY_TRACE(("dummy TX Start Called\n"));

			/* transmit dummy tx pkts and find the txidx */
			wlc_lcnphy_dummytx((wlc_phy_t *)pi, nframes, wait_delay);

			current_txidx = pi_lcn->lcnphy_start_idx; 	/* debug information */
			if (current_txidx == 0) {
				pi_lcn->lcnphy_capped_index += 4;
				wlc_lcnphy_load_txgainwithcappedindex(pi, 1);

			}
		} else {
			txgains.index = (uint8) current_txidx;
			txgains.useindex = 1;
			/* run papd corresponding to the target pwr */
			wlc_lcnphy_papd_cal(pi, PHY_PAPD_CAL_CW, &txgains, 0, 0, 0, 0, 219, 1, 0);
		}
#ifdef MULTICHAN_4313
	}
	else
	{
		txgains.index =
			(uint8) pi_lcn->tx_pwr_idx[CHSPEC_CHANNEL(pi->radio_chanspec)];
		pi_lcn->lcnphy_start_idx = txgains.index;
		 txgains.useindex = 1;
		wlc_lcnphy_papd_cal(pi, PHY_PAPD_CAL_CW, &txgains, 0, 0, 0, 0, 219, 1, 0);
	}
	/* Restore tx power control */
	wlc_lcnphy_set_tx_pwr_ctrl(pi,
		pi_lcn->tx_pwr_ctrl[CHSPEC_CHANNEL(pi->radio_chanspec)]);
#else
	/* Restore tx power control */
	wlc_lcnphy_set_tx_pwr_ctrl(pi, tx_pwr_ctrl);
#endif /* MULTICHAN_4313 */

	PHY_REG_LIST_START
		/* Reset radio ctrl and crs gain */
		PHY_REG_OR_ENTRY(LCNPHY, resetCtrl, 0x44)
		PHY_REG_WRITE_ENTRY(LCNPHY, resetCtrl, 0x80)
	PHY_REG_LIST_EXECUTE(pi);

	if (!suspend)
		wlapi_enable_mac(pi->sh->physhim);

	wlc_lcnphy_deaf_mode(pi, FALSE);

}

static void
wlc_lcnphy_btc_adjust(phy_info_t *pi, bool btactive)
{
	if ((CHIPID(pi->sh->chip) == BCM4313_CHIP_ID) &&
		(BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_FEM_BT)) {
		int16 high_gain;
		int btc_mode = wlapi_bmac_btc_mode_get(pi->sh->physhim);
		bool suspend = !(R_REG(pi->sh->osh, &((phy_info_t *)pi)->regs->maccontrol)
		                 & MCTL_EN_MAC);
		if (!suspend)
			wlapi_suspend_mac_and_wait(pi->sh->physhim);

		if (btactive && !pi->bt_active) {
			wlc_lcnphy_write_table(pi, &dot11lcnphytbl_gain_idx_bt_tbl_info_rev0[0]);
			wlc_lcnphy_write_table(pi, &dot11lcnphytbl_gain_bt_tbl_info_rev0[0]);
			PHY_REG_LIST_START
				RADIO_REG_WRITE_ENTRY(RADIO_2064_REG060, 0x70)
				RADIO_REG_WRITE_ENTRY(RADIO_2064_REG061, 0x7f)
				RADIO_REG_WRITE_ENTRY(RADIO_2064_REG062, 0x75)
			PHY_REG_LIST_EXECUTE(pi);

			/* If BT is detected then reduce the high gain by 3dB */
			high_gain =
			        ((phy_utils_read_phyreg(pi, LCNPHY_HiGainDB) &
				LCNPHY_HiGainDB_HiGainDB_MASK) >> LCNPHY_HiGainDB_HiGainDB_SHIFT);
			high_gain = high_gain - 3;
			PHY_REG_MOD(pi, LCNPHY, HiGainDB, HiGainDB, high_gain);

		} else if (!btactive && pi->bt_active) {
			wlc_lcnphy_write_table(pi, &dot11lcnphytbl_gain_idx_tbl_info_rev0[0]);
			wlc_lcnphy_write_table(pi, &dot11lcnphytbl_gain_tbl_info_rev0[0]);
			PHY_REG_LIST_START
				RADIO_REG_WRITE_ENTRY(RADIO_2064_REG060, 0x7f)
				RADIO_REG_WRITE_ENTRY(RADIO_2064_REG061, 0x72)
				RADIO_REG_WRITE_ENTRY(RADIO_2064_REG062, 0x7f)
			PHY_REG_LIST_EXECUTE(pi);

			/* If no BT is detected then restore the high gain by 3dB */
			high_gain =
			((phy_utils_read_phyreg(pi, LCNPHY_HiGainDB) &
				LCNPHY_HiGainDB_HiGainDB_MASK) >> LCNPHY_HiGainDB_HiGainDB_SHIFT);
			high_gain = high_gain + 3;
			PHY_REG_MOD(pi, LCNPHY, HiGainDB, HiGainDB, high_gain);
		}
		if ((btc_mode == WL_BTC_PARALLEL) || (btc_mode == WL_BTC_LITE)) {
			if (btactive) {
				wlc_lcnphy_set_tx_pwr_by_index(pi, 85);
			} else {
				wlc_lcnphy_set_tx_pwr_ctrl(pi, pi->hwpwrctrl_capable ?
					LCNPHY_TX_PWR_CTRL_HW : LCNPHY_TX_PWR_CTRL_TEMPBASED);
			}
		}
		if (!suspend)
			wlapi_enable_mac(pi->sh->physhim);
	}
}

#if defined(PHYCAL_CACHING) && defined(BCMDBG)
void
wlc_phy_cal_cache_dbg_lcnphy(ch_calcache_t *ctx)
{
	uint i;
	lcnphy_calcache_t *cache = &ctx->u.lcnphy_cache;

	/* Generic parameters */
	PHY_INFORM(("_____Generic params_____\n"));
	PHY_INFORM(("%d\n", CHSPEC_CHANNEL(ctx->chanspec)));
	PHY_INFORM(("last_cal_time: %d\n", ctx->cal_info.last_papd_cal_time));
	PHY_INFORM(("last_cal_temp: %d\n", ctx->cal_info.last_cal_temp));
	PHY_INFORM(("lcnphy_gain_index_at_last_cal: %d\n", cache->lcnphy_gain_index_at_last_cal));

	/* TX IQ LO cal results */
	PHY_INFORM(("_____TX IQ LO cal results_____\n"));
	for (i = 0; i < 3; i++) {
		PHY_INFORM(("\ntxiqlocal_a[%d]: %d\n", i, cache->txiqlocal_a[i]));
		PHY_INFORM(("\ntxiqlocal_b[%d]: %d\n", i, cache->txiqlocal_b[i]));
		PHY_INFORM(("\ntxiqlocal_didq[%d]: %d\n", i, cache->txiqlocal_didq[i]));
	}
	PHY_INFORM(("\ntxiqlocal_ei0: %d\n", cache->txiqlocal_ei0));
	PHY_INFORM(("\ntxiqlocal_eq0: %d\n", cache->txiqlocal_eq0));
	PHY_INFORM(("\ntxiqlocal_fi0: %d\n", cache->txiqlocal_fi0));
	PHY_INFORM(("\ntxiqlocal_fq0: %d\n", cache->txiqlocal_fq0));
	PHY_INFORM(("txiqlocal_bestcoeffs: \n"));
	for (i = 0; i < 11; i++) {
		PHY_INFORM(("[%u]:0x%x\n", i, cache->txiqlocal_bestcoeffs[i]));
	}
	PHY_INFORM(("\ntxiqlocal_bestcoeffs_valid: %d\n", cache->txiqlocal_bestcoeffs_valid));

	/* PAPD results */
	PHY_INFORM(("_____PAPD results_____\n"));
	PHY_INFORM(("papd_eps_tbl: \n"));
	for (i = 0; i < PHY_PAPD_EPS_TBL_SIZE_LCNPHY; i++) {
		PHY_INFORM(("[%u]:0x%x\n", i, cache->papd_eps_tbl[i]));
	}
	PHY_INFORM(("\nanalog_gain_ref: %d\n", cache->analog_gain_ref));
	PHY_INFORM(("\nlut_begin: %d\n", cache->lut_begin));
	PHY_INFORM(("\nlut_end: %d\n", cache->lut_end));
	PHY_INFORM(("\nlut_step: %d\n", cache->lut_step));
	PHY_INFORM(("\nrxcompdbm: %d\n", cache->rxcompdbm));
	PHY_INFORM(("\npapdctrl: %d\n", cache->papdctrl));
	PHY_INFORM(("\nsslpnCalibClkEnCtrl: %d\n", cache->sslpnCalibClkEnCtrl));

	/* RX IQ cal results */
	PHY_INFORM(("_____RX IQ cal results_____\n"));
	PHY_INFORM(("rxiqcal_coeff_a0: %d\n", cache->rxiqcal_coeff_a0));
	PHY_INFORM(("rxiqcal_coeff_b0: %d\n", cache->rxiqcal_coeff_b0));

	cache = NULL;
}
#endif /* PHYCAL_CACHING && BCMDBG */

static uint16
wlc_lcnphy_tssi_cal_sweep(phy_info_t *pi, int8 *des_pwr, uint8 *adj_tssi)
{
	uint16 count = 0;
	uint16 tssi_cnt = 0;
	uint8 bbmult_sweep;
	uint16 num_anchor, total_anchors = 0;
	uint16 idleTssi0_2C, idleTssi0_OB, avgTssi0_2C, avgTssi0_OB, tssi;
	uint32 sum_avgTssi0_2C = 0;
	int pow_change;
	int16 temp, temp1, temp2, qQ, qQ1, qQ2, shift;
	uint8 adjusted_tssi;
	int16 target_tssi = 127;

	int delay_count = 0;

	uint16 SAVE_txpwrctrl;
	bool tx_gain_override_old;
	lcnphy_txgains_t old_gains;

	SAVE_txpwrctrl = wlc_lcnphy_get_tx_pwr_ctrl(pi);
	tx_gain_override_old = wlc_lcnphy_tx_gain_override_enabled(pi);
	wlc_lcnphy_get_tx_gain(pi, &old_gains);

	while (delay_count < 2) {
		OSL_DELAY(1000);
		delay_count++;
	}

#if PHY_TSSI_CAL_DBG_EN
	for (num_anchor = 0; num_anchor < MAX_NUM_ANCHORS; num_anchor++)
		printf("Anchor: TargPwr: %d, MeasPwr: %d, BBmult: %d, TxIdx: %d, TSSI: %d, \n",
		pi->ptssi_cal->target_pwr_qdBm[num_anchor],
		pi->ptssi_cal->measured_pwr_qdBm[num_anchor],
		pi->ptssi_cal->anchor_bbmult[num_anchor],
		pi->ptssi_cal->anchor_txidx[num_anchor],
		pi->ptssi_cal->anchor_tssi[num_anchor]);
#endif // endif

	for (num_anchor = 0; num_anchor < MAX_NUM_ANCHORS; num_anchor++) {
		if (pi->ptssi_cal->anchor_bbmult[num_anchor])
			total_anchors++;
	}

	for (bbmult_sweep = 20; bbmult_sweep <= 100; bbmult_sweep++) {
		for (num_anchor = 0; num_anchor < total_anchors; num_anchor++) {

			bool suspend = !(R_REG(pi->sh->osh, &pi->regs->maccontrol) & MCTL_EN_MAC);
			if (!suspend)
				wlapi_suspend_mac_and_wait(pi->sh->physhim);
			wlc_btcx_override_enable(pi);
			wlc_lcnphy_set_tx_pwr_by_index(pi,
				pi->ptssi_cal->anchor_txidx[num_anchor]);
			wlc_lcnphy_set_bbmult(pi, bbmult_sweep);

			if (!suspend)
				wlapi_enable_mac(pi->sh->physhim);

			PHY_REG_MOD(pi, LCNPHY, TxPwrCtrlCmd, txPwrCtrl_en, 1);

			delay_count = 0;
			while (delay_count < 3) {
				OSL_DELAY(5000);
				delay_count++;
			}

			idleTssi0_2C = PHY_REG_READ(pi, LCNPHY, TxPwrCtrlIdleTssi, idleTssi0);
			if (idleTssi0_2C >= 256)
				idleTssi0_OB = idleTssi0_2C - 256;
			else
				idleTssi0_OB = idleTssi0_2C + 256;

			sum_avgTssi0_2C = 0;
			for (tssi_cnt = 0; tssi_cnt < 16; tssi_cnt++) {
				tssi = PHY_REG_READ(pi, LCNPHY, TxPwrCtrlStatusNew4, avgTssi);
				sum_avgTssi0_2C += tssi;
			}

			/* avgTssi value is in 2C (S9.0) format */
			avgTssi0_2C = sum_avgTssi0_2C >> 4;
			/* Convert from 2C to OB format by toggling MSB OB value */
			if (avgTssi0_2C >= 256)
				avgTssi0_OB = avgTssi0_2C - 256;
			else
				avgTssi0_OB = avgTssi0_2C + 256;

			adjusted_tssi = target_tssi + (avgTssi0_OB >> 2) - (idleTssi0_OB >> 2);

			/* pow_change = 4*20*log10(bbmult_sweep/anchor_bbmult) in qdbm */
			qm_log10((int32)(bbmult_sweep), 0, &temp1, &qQ1);
			qm_log10((int32)(pi->ptssi_cal->anchor_bbmult[num_anchor]),
				0, &temp2, &qQ2);

			if (qQ1 < qQ2) {
				temp2 = qm_shr16(temp2, qQ2-qQ1);
				qQ = qQ1;
			}
			else {
				temp1 = qm_shr16(temp1, qQ1-qQ2);
				qQ = qQ2;
			}
			temp = qm_sub16(temp1, temp2);
			if (qQ >= 4)
				shift = qQ-4;
			else
				shift = 4-qQ;

			pow_change = ((5*temp) + (1 << (shift-1))) >> shift;

			if ((pow_change <= 12) && (pow_change >= -12)) {  /* +- 3dB from anchor */
				des_pwr[count] =
					pi->ptssi_cal->measured_pwr_qdBm[num_anchor] + pow_change;
				adj_tssi[count] = adjusted_tssi;

				#if PHY_TSSI_CAL_DBG_EN
				printf("%d %d %d %d %d\n", wlc_lcnphy_get_bbmult(pi), avgTssi0_2C,
					idleTssi0_2C, adj_tssi[count], des_pwr[count]);
				#endif

				count++;
			}
		}
	}

	/* Restore txgain override */
	wlc_lcnphy_set_tx_gain_override(pi, tx_gain_override_old);
	wlc_lcnphy_set_tx_gain(pi, &old_gains);
	wlc_lcnphy_set_tx_pwr_ctrl(pi, SAVE_txpwrctrl);
	return count;
}

#ifdef WLTEST

static int
wlc_lncphy_iq_samp_cap(phy_info_t *pi, int n, uint32* ptr, int mode)
{
	uint32 val;
	uint16 timer;
	uint16 old_sslpnCalibClkEnCtrl;
	int16 imag, real;
	int n_samps_out = 0;

	if (n > 128)
		n = 128;

	old_sslpnCalibClkEnCtrl = phy_utils_read_phyreg(pi, LCNPHY_sslpnCalibClkEnCtrl);

	/* configure phy to do a timer based capture of mode 2(adc o/p) */
	phy_utils_write_phyreg(pi, LCNPHY_sslpnphy40_soft_reset, 3);
	phy_utils_write_phyreg(pi, LCNPHY_sslpnCalibClkEnCtrl,
	                       (uint32)(old_sslpnCalibClkEnCtrl | 0x2008));
	phy_utils_write_phyreg(pi, LCNPHY_sslpnphy_gpio_ctrl, 5);
	phy_utils_write_phyreg(pi, LCNPHY_sslpnphy_gpio_sel, (uint16)(mode | mode << 6));
	PHY_REG_LIST_START
		PHY_REG_WRITE_ENTRY(LCNPHY, sslpnphy_gpio_out_en_34_32, 7)
		PHY_REG_WRITE_ENTRY(LCNPHY, dbg_samp_coll_start_mac_xfer_trig_timer_15_0, 0)
		PHY_REG_WRITE_ENTRY(LCNPHY, dbg_samp_coll_start_mac_xfer_trig_timer_31_16, 0)
	PHY_REG_LIST_EXECUTE(pi);

	phy_utils_write_phyreg(pi, LCNPHY_dbg_samp_coll_end_mac_xfer_trig_timer_15_0, (2*n));
	PHY_REG_LIST_START
		PHY_REG_WRITE_ENTRY(LCNPHY, dbg_samp_coll_end_mac_xfer_trig_timer_31_16, 0)
		PHY_REG_WRITE_ENTRY(LCNPHY, dbg_samp_coll_ctrl, 0x201)

		PHY_REG_WRITE_ENTRY(LCNPHY, sslpnphy40_soft_reset, 0)
		PHY_REG_WRITE_ENTRY(LCNPHY, sslpnphy_gpio_test_reg_17_2,  0xdead)
		PHY_REG_WRITE_ENTRY(LCNPHY, sslpnphy_gpio_test_reg_33_18, 0xbeef)
	PHY_REG_LIST_EXECUTE(pi);

	/* wait for collection to finish */
	timer = 0;
	do {
		OSL_DELAY(10);
		timer++;
	} while ((phy_utils_read_phyreg(pi, LCNPHY_dbg_samp_coll_ctrl) & 1) && (timer < 500));

	if (phy_utils_read_phyreg(pi, LCNPHY_dbg_samp_coll_ctrl) & 1) {
		return 0;
	}

	/* get data from spb */
	phy_utils_write_phyreg(pi, LCNPHY_sslpnphy_gpio_test_reg_17_2,
	                       phy_utils_read_phyreg(pi, 0x593));
	phy_utils_write_phyreg(pi, LCNPHY_sslpnphy_gpio_test_reg_33_18,
	                       phy_utils_read_phyreg(pi, 0x580));
	n_samps_out = 0;
	phy_utils_write_phyreg(pi, LCNPHY_TableAddress, 0x5400);
	if ((phy_utils_read_phyreg(pi, LCNPHY_TabledataLo) & 0xf) == 5)
		phy_utils_write_phyreg(pi, LCNPHY_TableAddress, 0x5400);
	else
		phy_utils_write_phyreg(pi, LCNPHY_TableAddress, 0x5401);
	while (n > 0) {
		uint16 lo, hi;

		lo = phy_utils_read_phyreg(pi, LCNPHY_TabledataLo);
		hi = phy_utils_read_phyreg(pi, LCNPHY_TabledataHi);
		val = (uint32)(hi << 16) | (uint32)(lo);
		real = (val >> 4) & 0x3ff;

		lo = phy_utils_read_phyreg(pi, LCNPHY_TabledataLo);
		hi = phy_utils_read_phyreg(pi, LCNPHY_TabledataHi);
		val = (uint32)(hi << 16) | (uint32)(lo);
		imag = (val >> 4) & 0x3ff;

		if (imag > 511) {
			imag -= 1024;
		}
		if (real > 511) {
			real -= 1024;
		}
		ptr[n_samps_out] = ((imag & 0x3ff)<<16) | (real & 0x3ff);
		n--;
		n_samps_out++;
	}

	phy_utils_write_phyreg(pi, LCNPHY_sslpnCalibClkEnCtrl, old_sslpnCalibClkEnCtrl);

	return n_samps_out;
}

void
wlc_lcnphy_iovar_samp_cap(wlc_phy_t *ppi, int32 gain, int32 *ret)
{
	phy_info_t *pi = (phy_info_t*)ppi;
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	uint16 swctrl_override, rf_override_0, rf_override_2, rf_override_3, rf_override_4;

	if (ret == NULL) {
		bool suspend;
		int16 hpc, q;

		suspend = !(R_REG(pi->sh->osh, &pi->regs->maccontrol) & MCTL_EN_MAC);
		if (!suspend) {
			wlapi_suspend_mac_and_wait(pi->sh->physhim);
			wlc_btcx_override_enable(pi);
		}

		PHY_REG_MOD(pi, LCNPHY, lpphyCtrl, resetCCA, 1);
		/* Save and clear all rf overrides which could affect rx chain */
		swctrl_override = phy_utils_read_phyreg(pi, LCNPHY_swctrlOvr);
		rf_override_0 = phy_utils_read_phyreg(pi, LCNPHY_RFOverride0);
		rf_override_2 = phy_utils_read_phyreg(pi, LCNPHY_rfoverride2);
		rf_override_3 = phy_utils_read_phyreg(pi, LCNPHY_rfoverride3);
		rf_override_4 = phy_utils_read_phyreg(pi, LCNPHY_rfoverride4);
		PHY_REG_LIST_START
			PHY_REG_WRITE_ENTRY(LCNPHY, swctrlOvr, 0)
			PHY_REG_WRITE_ENTRY(LCNPHY, RFOverride0, 0)
			PHY_REG_WRITE_ENTRY(LCNPHY, rfoverride2, 0)
			PHY_REG_WRITE_ENTRY(LCNPHY, rfoverride3, 0)
			PHY_REG_WRITE_ENTRY(LCNPHY, rfoverride4, 0)
		PHY_REG_LIST_EXECUTE(pi);

		wlc_lcnphy_deaf_mode(pi, TRUE);
		/* argument specifies gain code and bw(each field is 4 bits):
		 * <bw><biq1><biq0><tia><lna2><lna1><elna><tr>
		 */
		wlc_lcnphy_set_rx_gain_by_distribution(pi,
					(gain & 0xf),         /* TR */
					((gain >>  4) & 0xf), /* ELNA */
					((gain >> 24) & 0xf), /* BIQ1 */
					((gain >> 20) & 0xf), /* BIQ0 */
					((gain >> 16) & 0xf), /* TIA */
					((gain >> 12) & 0xf), /* LNA2 */
					((gain >>  8) & 0xf), /* LNA1 */
					0);  /* DIGI */

		wlc_lcnphy_rx_gain_override_enable(pi, TRUE);

		if (gain & 0xf0000000) {
			hpc = 1;
			q = 1;
		} else {
			hpc = 7;
			q = 0;
		}
		PHY_REG_MOD(pi, LCNPHY, rfoverride2val, hpf1_ctrl_ovr_val, hpc);
		PHY_REG_MOD(pi, LCNPHY, rfoverride2val, hpf2_ctrl_ovr_val, hpc);
		PHY_REG_MOD(pi, LCNPHY, rfoverride2val, lpf_lq_ovr_val, q);
		PHY_REG_LIST_START
			PHY_REG_MOD_ENTRY(LCNPHY, rfoverride2, hpf1_ctrl_ovr, 1)
			PHY_REG_MOD_ENTRY(LCNPHY, rfoverride2, hpf2_ctrl_ovr, 1)
			PHY_REG_MOD_ENTRY(LCNPHY, rfoverride2, lpf_lq_ovr, 1)

			PHY_REG_MOD_ENTRY(LCNPHY, lpphyCtrl, resetCCA, 0)
		PHY_REG_LIST_EXECUTE(pi);

		OSL_DELAY(100);

		/* Everything should be configured for rx now */

		pi_lcn->samp_cap_r_idx = 0;
		pi_lcn->samp_cap_w_idx = (uint16)wlc_lncphy_iq_samp_cap(pi,
			LCNPHY_MAX_SAMP_CAP_DATA,
			pi_lcn->samp_cap_data,
			2);

		/* Restore all overrides */
		PHY_REG_MOD(pi, LCNPHY, lpphyCtrl, resetCCA, 1);

		phy_utils_write_phyreg(pi, LCNPHY_swctrlOvr, swctrl_override);
		phy_utils_write_phyreg(pi, LCNPHY_RFOverride0, rf_override_0);
		phy_utils_write_phyreg(pi, LCNPHY_rfoverride2, rf_override_2);
		phy_utils_write_phyreg(pi, LCNPHY_rfoverride3, rf_override_3);
		phy_utils_write_phyreg(pi, LCNPHY_rfoverride4, rf_override_4);

		wlc_lcnphy_deaf_mode(pi, FALSE);

		PHY_REG_MOD(pi, LCNPHY, lpphyCtrl, resetCCA, 0);
		OSL_DELAY(100);

		if (!suspend)
			wlapi_enable_mac(pi->sh->physhim);
	} else {
		*ret = pi_lcn->samp_cap_data[pi_lcn->samp_cap_r_idx];
		pi_lcn->samp_cap_r_idx++;
		if (pi_lcn->samp_cap_r_idx >= pi_lcn->samp_cap_w_idx)
			pi_lcn->samp_cap_r_idx = 0;
	}
}

#endif /* WLTEST */

/* These eventually should move to lcncommon.c */

#if defined(PHYCAL_CACHING)
int
wlc_iovar_txpwrindex_set_lcncommon(phy_info_t *pi, int8 siso_int_val, ch_calcache_t *ctx)
#else
int
wlc_iovar_txpwrindex_set_lcncommon(phy_info_t *pi, int8 siso_int_val)
#endif // endif
{
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	int err = BCME_OK;
	/* Override power index if NVRAM says so */
	if (pi_lcn->txpwrindex5g_nvram && (CHSPEC_IS5G(pi->radio_chanspec)))
		siso_int_val = pi_lcn->txpwrindex5g_nvram;

	if (pi_lcn->txpwrindex_nvram && (CHSPEC_IS2G(pi->radio_chanspec)))
		siso_int_val = pi_lcn->txpwrindex_nvram;

	if (siso_int_val == -1) {
		/* obsolete.  replaced with txpwrctrl */
		/* obsolete.  replaced with txpwrctrl */
		if (pi->pi_fptr.settxpwrctrl)
			pi->pi_fptr.settxpwrctrl(pi, wlc_txpwrctrl_lcncommon(pi));
		/* Reset calibration */
#if defined(PHYCAL_CACHING)
		ASSERT(ctx);
		ctx->valid = FALSE;
#else
		pi_lcn->lcnphy_full_cal_channel = 0;
#endif // endif
		pi->phy_forcecal = TRUE;
	} else if (siso_int_val >= 0) {
		if (pi->pi_fptr.settxpwrbyindex)
			pi->pi_fptr.settxpwrbyindex(pi, siso_int_val);
	} else {
		err = BCME_RANGE;
	}
	return err;
}

#ifdef LP_P2P_SOFTAP
static void
wlc_lcnphy_lpc_write_maclut(phy_info_t *pi)
{
	phytbl_info_t tab;
	uint8 i;

	/* Assign values from 0 to 63 qdB for now */
	for (i = 0; i < LCNPHY_TX_PWR_CTRL_MACLUT_LEN; i++)
		pwr_lvl_qdB[i] = i;

	/* Prepare table structure */
	tab.tbl_ptr = pwr_lvl_qdB;
	tab.tbl_len = LCNPHY_TX_PWR_CTRL_MACLUT_LEN;
	tab.tbl_id = LCNPHY_TBL_ID_TXPWRCTL;
	tab.tbl_offset = LCNPHY_TX_PWR_CTRL_MAC_OFFSET;
	tab.tbl_width = LCNPHY_TX_PWR_CTRL_MACLUT_WIDTH;

	/* Write to it */
	wlc_lcnphy_write_table(pi, &tab);
}
#endif /* LP_P2P_SOFTAP */

#ifdef MULTICHAN_4313
/* num of channels supported in lcn phy for 20MHz is 14 */
#define NUM_CHANNELS_2G_20MHZ   14
void
wlc_lcnphy_tx_pwr_idx_reset(phy_info_t *pi)
{
	uint8 i;
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;

	/* reset the tx power index and tx power control values for
	 * all the 14 channels of 2G
	 */
	for (i = 1; i <= NUM_CHANNELS_2G_20MHZ; i++)
	{
		pi_lcn->tx_pwr_idx[i] = 0;
		pi_lcn->tx_pwr_ctrl[i] = 0;
	}
}
#endif /* MULTICHAN_4313 */

void
BCMATTACHFN(wlc_phy_interference_mode_attach_lcnphy)(phy_info_t *pi)
{
	if (CHIPID(pi->sh->chip) == BCM4313_CHIP_ID) {
		pi->sh->interference_mode_2G = WLAN_AUTO;
		pi->sh->interference_mode = WLAN_AUTO;
	}
}
#endif /* LCNCONF != 0 || LCN40CONF != 0 */
