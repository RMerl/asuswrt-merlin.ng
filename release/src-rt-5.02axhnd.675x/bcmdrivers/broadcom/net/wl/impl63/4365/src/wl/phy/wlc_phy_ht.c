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
 * $Id: wlc_phy_ht.c 573258 2015-07-22 07:56:00Z $
 *
 */

/* XXX WARNING: phy structure has been changed, read this first
 *
 * This submodule is for HTPHY phy only. It depends on the common submodule wlc_phy_cmn.c
 */

#include <wlc_cfg.h>

#if HTCONF != 0
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
#include <sbchipc.h>
#include <hndpmu.h>
#include <bcmsrom_fmt.h>
#include <sbsprom.h>

#include <wlc_phy_hal.h>
#include <wlc_phy_int.h>
#include <wlc_phy_ht.h>
#include <wlc_phyreg_ht.h>
#include <wlc_phytbl_ht.h>
#include <phy_rxgcrs_api.h>

#include <phy_utils_math.h>
#include <phy_utils_var.h>
#include <phy_utils_reg.h>

#include <phy_ht_radar.h>
#include <phy_ht_rssi.h>

/* Interference mitigation related constant/parameter defs */

#define HTPHY_ACI_MAX_UNDETECT_WINDOW_SZ 40 /* max window of aci detect */
#define HTPHY_ACI_40MHZ_CHANNEL_DELTA 6
#define HTPHY_ACI_40MHZ_CHANNEL_SKIP 5
#define HTPHY_ACI_40MHZ_CHANNEL_DELTA 6
#define HTPHY_ACI_40MHZ_CHANNEL_SKIP 5
#define HTPHY_ACI_CHANNEL_DELTA 4   /* How far a signal can bleed */
#define HTPHY_ACI_CHANNEL_SKIP 3	   /* Num of immediately surrounding channels to skip */

/* noise immunity raise/lowered using crsminpwr or init gain value */
/* before assoc, consec crs glitch before raising noise immunity */
#define HTPHY_NOISE_NOASSOC_GLITCH_TH_UP 1
#define HTPHY_BPHYNOISE_NOASSOC_GLITCH_TH_UP 1

/* before assoc, consec crs glitch before lowering noise immunity */
#define HTPHY_NOISE_NOASSOC_GLITCH_TH_DN 2
#define HTPHY_BPHYNOISE_NOASSOC_GLITCH_TH_DN 4

/* after assoc, no aci, consec crs glitch before raising noise immunity */
#define HTPHY_NOISE_ASSOC_GLITCH_TH_UP 1
#define HTPHY_BPHYNOISE_ASSOC_GLITCH_TH_UP 1

/* after assoc, no aci, consec crs glitch before lowering noise immunity */
#define HTPHY_NOISE_ASSOC_GLITCH_TH_DN 2
#define HTPHY_BPHYNOISE_ASSOC_UNDESENSE_WAIT 4
#define HTPHY_BPHYNOISE_ASSOC_UNDESENSE_WINDOW 8

/* after assoc, aci on, consec crs glitch before raising noise immunity */
#define HTPHY_NOISE_ASSOC_ACI_GLITCH_TH_UP 1

/* after assoc, aci on, consec crs glitch before lowering noise immunity */
#define HTPHY_NOISE_ASSOC_ACI_GLITCH_TH_DN 2

/* not associated, threshold for noise ma to raise inband immunity */
/* compared against rx crs glitches and bad plcps (only ofdm) */
#define HTPHY_NOISE_NOASSOC_ENTER_TH  500

/* associated, threshold for noise ma to raise inband immunity */
/* compared against rx crs glitches and bad plcps (only ofdm) */
#define HTPHY_NOISE_ASSOC_ENTER_TH  500
#define HTPHY_BPHYNOISE_ASSOC_ENTER_TH  125
#define HTPHY_BPHYNOISE_ASSOC_HIGH_TH  500

/* associated, threshold for noise ma to raise inband immunity */
/* compared against rx crs glitches and bad plcps (for both ofdm and bphy) */
#define HTPHY_NOISE_ASSOC_RX_GLITCH_BADPLCP_ENTER_TH  500

/* wl interference 1, 4, no assoc, crsminpwr index increment, 0.375dB steps */
#define HTPHY_NOISE_NOASSOC_CRSIDX_INCR 11

/* wl interference 1, 4, assoc, crsminpwr index increment, 0.375dB steps */
#define HTPHY_NOISE_ASSOC_CRSIDX_INCR 6

/* wl interference 1, 4, crsminpwr index decr, 0.375dB steps */
#define HTPHY_NOISE_CRSIDX_DECR   1

/* wl interference 1, (only assoc), bphy desense index increment, 1dB steps */
#define HTPHY_BPHYNOISE_CRSIDX_INCR_HI 4
#define HTPHY_BPHYNOISE_CRSIDX_INCR_LO 2

/* wl interference 1, bphy desense index decr, 1dB steps */
#define HTPHY_BPHYNOISE_CRSIDX_DECR   1

/* wl interference 1, 4, max crsminpwr index */
#define HTPHY_NOISE_MAX_CRSIDX   120

/* wl interference 1, 4, min crsminpwr index */
#define HTPHY_NOISE_MIN_CRSIDX   32

#define HTPHY_RSSICAL_W1_TARGET 25
#define HTPHY_RSSICAL_W2_TARGET HTPHY_RSSICAL_W1_TARGET

/* XXX
 * Asus boardtype added as a last choice because 4706nrh has
 * the same gain as Asus but we do not want to change anything for asus
 * as it has had its pilot production already
 */
#define BCM94331_4706_ASUS 0x0F5B2

/* %%%%%% shorthand macros */

#define MOD_PHYREG(pi, reg, field, value)				\
	phy_utils_mod_phyreg(pi, reg,						\
	            reg##_##field##_MASK, (value) << reg##_##field##_##SHIFT);

#define MOD_PHYREGC(pi, reg, core, field, value)			\
	phy_utils_mod_phyreg(pi, (core == 0) ? \
	reg##_CR0 : ((core == 1) ? reg##_CR1 : reg##_CR2), \
	            reg##_##field##_MASK, (value) << reg##_##field##_##SHIFT);

#define READ_PHYREG(pi, reg, field)				\
	((phy_utils_read_phyreg(pi, reg)					\
	  & reg##_##field##_##MASK) >> reg##_##field##_##SHIFT)

#define READ_PHYREGC(pi, reg, core, field)				\
	((phy_utils_read_phyreg(pi, (core == 0) ? \
	 reg##_CR0 : ((core == 1) ? reg##_CR1 : reg##_CR2)) \
	  & reg##_##field##_##MASK) >> reg##_##field##_##SHIFT)

#define RADIO2059_RCAL_RCCAL_CACHE_VALID(pi)		\
	(pi->u.pi_htphy->rcal_rccal_cache.cache_valid)
#define RADIO2059_VALIDATE_RCAL_RCCAL_CACHE(pi, val)		\
	(pi->u.pi_htphy->rcal_rccal_cache.cache_valid = val)

/* spectrum shaping filter (cck) */
#define TXFILT_SHAPING_CCK      0
#define TXFILT_SHAPING_OFDM40   1
#define TXFILT_SHAPING_OFDM20_WAR	2
#define TXFILT_SHAPING_OFDM20_DEF	3
#define HTPHY_NUM_DIG_FILT_COEFFS 15
/* CCK ePA/iPA */
uint16 HTPHY_txdigi_filtcoeffs[][HTPHY_NUM_DIG_FILT_COEFFS] = {
	{-360, 164, -376, 164, -1533, 576, 308, -314, 308, 121, -73, 121, 91, 124, 91},
	{-77, 20, -98, 49, -93, 60, 56, 111, 56, 26, -5, 26, 34, -32, 34},
	{-307,82,-389,189,-1529,938,256,511,256,102,-20,102,273,-260,273},	/* OFDM20 WAR */
	{-295,200,-363,142,-1391,826,151,301,151,151,301,151,602,-752,602}	/* OFDM20 DEFAULT */
};
/* %%%%%% channel/radio */
#define TUNING_REG(reg, core)  (((core == 0) ? reg##0 : ((core == 1) ? reg##1 : reg##2)))

/* channel info structure for htphy */
typedef struct _chan_info_htphy_radio2059 {
	uint16 chan;            /* channel number */
	uint16 freq;            /* in Mhz */
	uint8  RF_vcocal_countval0;
	uint8  RF_vcocal_countval1;
	uint8  RF_rfpll_refmaster_sparextalsize;
	uint8  RF_rfpll_loopfilter_r1;
	uint8  RF_rfpll_loopfilter_c2;
	uint8  RF_rfpll_loopfilter_c1;
	uint8  RF_cp_kpd_idac;
	uint8  RF_rfpll_mmd0;
	uint8  RF_rfpll_mmd1;
	uint8  RF_vcobuf_tune;
	uint8  RF_logen_mx2g_tune;
	uint8  RF_logen_mx5g_tune;
	uint8  RF_logen_indbuf2g_tune;
	uint8  RF_logen_indbuf5g_tune_core0;
	uint8  RF_txmix2g_tune_boost_pu_core0;
	uint8  RF_pad2g_tune_pus_core0;
	uint8  RF_pga_boost_tune_core0;
	uint8  RF_txmix5g_boost_tune_core0;
	uint8  RF_pad5g_tune_misc_pus_core0;
	uint8  RF_lna2g_tune_core0;
	uint8  RF_lna5g_tune_core0;
	uint8  RF_logen_indbuf5g_tune_core1;
	uint8  RF_txmix2g_tune_boost_pu_core1;
	uint8  RF_pad2g_tune_pus_core1;
	uint8  RF_pga_boost_tune_core1;
	uint8  RF_txmix5g_boost_tune_core1;
	uint8  RF_pad5g_tune_misc_pus_core1;
	uint8  RF_lna2g_tune_core1;
	uint8  RF_lna5g_tune_core1;
	uint8  RF_logen_indbuf5g_tune_core2;
	uint8  RF_txmix2g_tune_boost_pu_core2;
	uint8  RF_pad2g_tune_pus_core2;
	uint8  RF_pga_boost_tune_core2;
	uint8  RF_txmix5g_boost_tune_core2;
	uint8  RF_pad5g_tune_misc_pus_core2;
	uint8  RF_lna2g_tune_core2;
	uint8  RF_lna5g_tune_core2;
	uint16 PHY_BW1a;
	uint16 PHY_BW2;
	uint16 PHY_BW3;
	uint16 PHY_BW4;
	uint16 PHY_BW5;
	uint16 PHY_BW6;
} chan_info_htphy_radio2059_t;

typedef struct htphy_sfo_cfg {
	uint16 PHY_BW1a;
	uint16 PHY_BW2;
	uint16 PHY_BW3;
	uint16 PHY_BW4;
	uint16 PHY_BW5;
	uint16 PHY_BW6;
} htphy_sfo_cfg_t;

/* Bphy desense settings in ~1dB steps, starting from 0dB */
bphy_desense_info_t HTPHY_bphy_desense_lut_eLNA[] = {
	{0x4477, 0x10, 0}, {0x4402, 0x10, 0}, {0x4402, 0x90, 0}, {0x4403, 0x10, 0},
	{0x4403, 0x40, 0}, {0x4403, 0x50, 0}, {0x4404, 0x20, 0}, {0x4404, 0x28, 0},
	{0x4404, 0x30, 0}, {0x4404, 0x38, 0}, {0x4404, 0x40, 0}, {0x4404, 0x34, 1},
	{0x4404, 0x38, 1}, {0x4404, 0x40, 1}, {0x4404, 0x30, 2}, {0x4404, 0x38, 2},
	{0x4404, 0x40, 2}, {0x4404, 0x30, 3}, {0x4404, 0x38, 3}, {0x4404, 0x40, 3},
	{0x4404, 0x30, 4}, {0x4404, 0x3c, 4}, {0x4404, 0x40, 4}, {0x4404, 0x36, 5},
	{0x4404, 0x3a, 5}
	/* XXX Capping desense to -76dBm; more desense can be got with the following:
	 * {0x4404, 0x40, 5}, {0x4404, 0x36, 6}, {0x4404, 0x3a, 6}, {0x4404, 0x40, 6}
	 */
};
#define HTPHY_BPHY_MIN_SENSITIVITY_ELNA (-100)

bphy_desense_info_t HTPHY_bphy_desense_lut_iLNA_acioff[] = {
	{0x4477, 0x10, 0}, {0x4403, 0x10, 0}, {0x4403, 0x40, 0}, {0x4404, 0x08, 0},
	{0x4404, 0x22, 0}, {0x4404, 0x30, 0}, {0x4404, 0x38, 0}, {0x4404, 0x40, 0},
	{0x4404, 0x2c, 1}, {0x4404, 0x38, 1}, {0x4404, 0x40, 1}, {0x4404, 0x38, 2},
	{0x4404, 0x40, 2}, {0x4404, 0x30, 3}, {0x4404, 0x38, 3}, {0x4404, 0x40, 3},
	{0x4404, 0x2c, 4}, {0x4404, 0x30, 4}, {0x4404, 0x38, 4}, {0x4404, 0x40, 4},
	{0x4404, 0x34, 5}
	/* XXX Capping desense to -76dBm; more desense can be got with the following:
	 * {0x4404, 0x38, 5}, {0x4404, 0x40, 5}, {0x4404, 0x38, 6}, {0x4404, 0x3c, 6},
	 * {0x4404, 0x40, 6}
	*/
};
#define HTPHY_BPHY_MIN_SENSITIVITY_ILNA_ACIOFF (-96)

bphy_desense_info_t HTPHY_bphy_desense_lut_iLNA_acion[] = {
	{0x4477, 0x10, 0}, {0x4477, 0x40, 0}, {0x4403, 0x30, 0}, {0x4403, 0x10, 1},
	{0x4404, 0x08, 0}, {0x4404, 0x28, 0}, {0x4404, 0x10, 1}, {0x4404, 0x28, 1},
	{0x4404, 0x30, 1}, {0x4404, 0x38, 1}, {0x4404, 0x40, 1}, {0x4404, 0x30, 2},
	{0x4404, 0x38, 2}, {0x4404, 0x40, 2}, {0x4404, 0x30, 3}, {0x4404, 0x24, 4}
	/* XXX Capping desense to -76dBm; more desense can be got with the following:
	 * {0x4404, 0x28, 4}, {0x4404, 0x2c, 4}, {0x4404, 0x30, 4}, {0x4404, 0x3a, 4},
	 * {0x4404, 0x40, 4}
	*/
};
#define HTPHY_BPHY_MIN_SENSITIVITY_ILNA_ACION (-91)

bphy_desense_info_t HTPHY_bphy_desense_lut_X29B_BTON[] = {
	{0x4477, 0x10, 0}, {0x4402, 0x40, 2}, {0x4403, 0x30, 1}, {0x4403, 0x20, 2},
	{0x4403, 0x30, 2}, {0x4403, 0x38, 2}, {0x4403, 0x40, 2}, {0x4403, 0x30, 3},
	{0x4403, 0x38, 3}, {0x4403, 0x40, 3}, {0x4404, 0x20, 3}
	/* XXX Capping desense to -76dBm; more desense can be got with the following:
	 * {0x4404, 0x24, 3}, {0x4404, 0x28, 3}, {0x4404, 0x30, 3}, {0x4404, 0x34, 3},
	 * {0x4404, 0x38, 3}, {0x4404, 0x40, 3}
	*/
};
#define HTPHY_BPHY_MIN_SENSITIVITY_X29B_BTON (-86)

/* channel info table for htphyrev0 (autogenerated by gen_tune_2059.tcl) */
static chan_info_htphy_radio2059_t chan_info_htphyrev0_2059rev0[] = {
	{
		184,   4920,  0x68,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0xec,  0x01,  0x0f,
		0x00,  0x0f,  0x00,  0x0f,  0x00,  0xd3,  0x0f,  0x4f,  0xd3,  0x00,  0xff,  0x0f,
		0x00,  0x00,  0x0f,  0x4f,  0xd3,  0x00,  0xff,  0x0f,  0x00,  0x00,  0x0f,  0x4f,
		0xd3,  0x00,  0xff,  0x07b4,  0x07b0,  0x07ac,  0x0214,  0x0215,  0x0216
	},
	{
		186,   4930,  0x6b,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0xed,  0x01,  0x0f,
		0x00,  0x0f,  0x00,  0x0f,  0x00,  0xd3,  0x0f,  0x4f,  0xd3,  0x00,  0xff,  0x0f,
		0x00,  0x00,  0x0f,  0x4f,  0xd3,  0x00,  0xff,  0x0f,  0x00,  0x00,  0x0f,  0x4f,
		0xd3,  0x00,  0xff,  0x07b8,  0x07b4,  0x07b0,  0x0213,  0x0214,  0x0215
	},
	{
		188,   4940,  0x6e,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0xee,  0x01,  0x0f,
		0x00,  0x0f,  0x00,  0x0f,  0x00,  0xd3,  0x0f,  0x4f,  0xd3,  0x00,  0xff,  0x0f,
		0x00,  0x00,  0x0f,  0x4f,  0xd3,  0x00,  0xff,  0x0f,  0x00,  0x00,  0x0f,  0x4f,
		0xd3,  0x00,  0xff,  0x07bc,  0x07b8,  0x07b4,  0x0212,  0x0213,  0x0214
	},
	{
		190,   4950,  0x72,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0xef,  0x01,  0x0f,
		0x00,  0x0f,  0x00,  0x0f,  0x00,  0x00,  0x0f,  0x4f,  0xd3,  0x00,  0xff,  0x0f,
		0x00,  0x00,  0x0f,  0x4f,  0xd3,  0x00,  0xff,  0x0f,  0x00,  0x00,  0x0f,  0x4f,
		0xd3,  0x00,  0xff,  0x07c0,  0x07bc,  0x07b8,  0x0211,  0x0212,  0x0213
	},
	{
		192,   4960,  0x75,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0xf0,  0x01,  0x0f,
		0x00,  0x0f,  0x00,  0x0f,  0x00,  0x00,  0x0f,  0x4f,  0xd3,  0x00,  0xff,  0x0f,
		0x00,  0x00,  0x0f,  0x4f,  0xd3,  0x00,  0xff,  0x0f,  0x00,  0x00,  0x0f,  0x4f,
		0xd3,  0x00,  0xff,  0x07c4,  0x07c0,  0x07bc,  0x020f,  0x0211,  0x0212
	},
	{
		194,   4970,  0x78,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0xf1,  0x01,  0x0f,
		0x00,  0x0f,  0x00,  0x0f,  0x00,  0x00,  0x0f,  0x4f,  0xd3,  0x00,  0xff,  0x0f,
		0x00,  0x00,  0x0f,  0x4f,  0xd3,  0x00,  0xff,  0x0f,  0x00,  0x00,  0x0f,  0x4f,
		0xd3,  0x00,  0xff,  0x07c8,  0x07c4,  0x07c0,  0x020e,  0x020f,  0x0211
	},
	{
		196,   4980,  0x7c,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0xf2,  0x01,  0x0f,
		0x00,  0x0f,  0x00,  0x0f,  0x00,  0x00,  0x0f,  0x4f,  0xd3,  0x00,  0xff,  0x0f,
		0x00,  0x00,  0x0f,  0x4f,  0xd3,  0x00,  0xff,  0x0f,  0x00,  0x00,  0x0f,  0x4f,
		0xd3,  0x00,  0xff,  0x07cc,  0x07c8,  0x07c4,  0x020d,  0x020e,  0x020f
	},
	{
		198,   4990,  0x7f,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0xf3,  0x01,  0x0f,
		0x00,  0x0f,  0x00,  0x0f,  0x00,  0x00,  0x0f,  0x4f,  0xd3,  0x00,  0xff,  0x0f,
		0x00,  0x00,  0x0f,  0x4f,  0xd3,  0x00,  0xff,  0x0f,  0x00,  0x00,  0x0f,  0x4f,
		0xd3,  0x00,  0xff,  0x07d0,  0x07cc,  0x07c8,  0x020c,  0x020d,  0x020e
	},
	{
		200,   5000,  0x82,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0xf4,  0x01,  0x0f,
		0x00,  0x0f,  0x00,  0x0f,  0x00,  0x00,  0x0f,  0x4f,  0xb3,  0x00,  0xff,  0x0f,
		0x00,  0x00,  0x0f,  0x4f,  0xb3,  0x00,  0xff,  0x0f,  0x00,  0x00,  0x0f,  0x4f,
		0xb3,  0x00,  0xff,  0x07d4,  0x07d0,  0x07cc,  0x020b,  0x020c,  0x020d
	},
	{
		202,   5010,  0x86,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0xf5,  0x01,  0x0f,
		0x00,  0x0f,  0x00,  0x0f,  0x00,  0x00,  0x0f,  0x4f,  0xb3,  0x00,  0xff,  0x0f,
		0x00,  0x00,  0x0f,  0x4f,  0xb3,  0x00,  0xff,  0x0f,  0x00,  0x00,  0x0f,  0x4f,
		0xb3,  0x00,  0xff,  0x07d8,  0x07d4,  0x07d0,  0x020a,  0x020b,  0x020c
	},
	{
		204,   5020,  0x89,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0xf6,  0x01,  0x0e,
		0x00,  0x0e,  0x00,  0x0e,  0x00,  0x00,  0x0f,  0x4f,  0xb3,  0x00,  0xff,  0x0e,
		0x00,  0x00,  0x0f,  0x4f,  0xb3,  0x00,  0xff,  0x0e,  0x00,  0x00,  0x0f,  0x4f,
		0xb3,  0x00,  0xff,  0x07dc,  0x07d8,  0x07d4,  0x0209,  0x020a,  0x020b
	},
	{
		206,   5030,  0x8c,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0xf7,  0x01,  0x0e,
		0x00,  0x0e,  0x00,  0x0e,  0x00,  0x00,  0x0f,  0x4f,  0xb3,  0x00,  0xff,  0x0e,
		0x00,  0x00,  0x0f,  0x4f,  0xb3,  0x00,  0xff,  0x0e,  0x00,  0x00,  0x0f,  0x4f,
		0xb3,  0x00,  0xff,  0x07e0,  0x07dc,  0x07d8,  0x0208,  0x0209,  0x020a
	},
	{
		208,   5040,  0x90,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0xf8,  0x01,  0x0e,
		0x00,  0x0e,  0x00,  0x0e,  0x00,  0x00,  0x0f,  0x4f,  0xb3,  0x00,  0xff,  0x0e,
		0x00,  0x00,  0x0f,  0x4f,  0xb3,  0x00,  0xff,  0x0e,  0x00,  0x00,  0x0f,  0x4f,
		0xb3,  0x00,  0xff,  0x07e4,  0x07e0,  0x07dc,  0x0207,  0x0208,  0x0209
	},
	{
		210,   5050,  0x93,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0xf9,  0x01,  0x0e,
		0x00,  0x0e,  0x00,  0x0e,  0x00,  0x00,  0x0f,  0x4f,  0xb3,  0x00,  0xff,  0x0e,
		0x00,  0x00,  0x0f,  0x4f,  0xb3,  0x00,  0xff,  0x0e,  0x00,  0x00,  0x0f,  0x4f,
		0xb3,  0x00,  0xff,  0x07e8,  0x07e4,  0x07e0,  0x0206,  0x0207,  0x0208
	},
	{
		212,   5060,  0x96,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0xfa,  0x01,  0x0e,
		0x00,  0x0e,  0x00,  0x0e,  0x00,  0x00,  0x0f,  0x4f,  0xb3,  0x00,  0xff,  0x0e,
		0x00,  0x00,  0x0f,  0x4f,  0xb3,  0x00,  0xff,  0x0e,  0x00,  0x00,  0x0f,  0x4f,
		0xb3,  0x00,  0xff,  0x07ec,  0x07e8,  0x07e4,  0x0205,  0x0206,  0x0207
	},
	{
		214,   5070,  0x9a,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0xfb,  0x01,  0x0e,
		0x00,  0x0e,  0x00,  0x0e,  0x00,  0x00,  0x0f,  0x4f,  0xb3,  0x00,  0xff,  0x0e,
		0x00,  0x00,  0x0f,  0x4f,  0xb3,  0x00,  0xff,  0x0e,  0x00,  0x00,  0x0f,  0x4f,
		0xb3,  0x00,  0xff,  0x07f0,  0x07ec,  0x07e8,  0x0204,  0x0205,  0x0206
	},
	{
		216,   5080,  0x9d,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0xfc,  0x01,  0x0e,
		0x00,  0x0e,  0x00,  0x0e,  0x00,  0x00,  0x0f,  0x4f,  0xb3,  0x00,  0xff,  0x0e,
		0x00,  0x00,  0x0f,  0x4f,  0xb3,  0x00,  0xff,  0x0e,  0x00,  0x00,  0x0f,  0x4f,
		0xb3,  0x00,  0xff,  0x07f4,  0x07f0,  0x07ec,  0x0203,  0x0204,  0x0205
	},
	{
		218,   5090,  0xa0,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0xfd,  0x01,  0x0e,
		0x00,  0x0e,  0x00,  0x0e,  0x00,  0x00,  0x0f,  0x4f,  0xb3,  0x00,  0xff,  0x0e,
		0x00,  0x00,  0x0f,  0x4f,  0xb3,  0x00,  0xff,  0x0e,  0x00,  0x00,  0x0f,  0x4f,
		0xb3,  0x00,  0xff,  0x07f8,  0x07f4,  0x07f0,  0x0202,  0x0203,  0x0204
	},
	{
		220,   5100,  0xa4,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0xfe,  0x01,  0x0d,
		0x00,  0x0d,  0x00,  0x0d,  0x00,  0x00,  0x0f,  0x4f,  0xa3,  0x00,  0xfc,  0x0d,
		0x00,  0x00,  0x0f,  0x4f,  0xa3,  0x00,  0xfc,  0x0d,  0x00,  0x00,  0x0f,  0x4f,
		0xa3,  0x00,  0xfc,  0x07fc,  0x07f8,  0x07f4,  0x0201,  0x0202,  0x0203
	},
	{
		222,   5110,  0xa7,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0xff,  0x01,  0x0d,
		0x00,  0x0d,  0x00,  0x0d,  0x00,  0x00,  0x0f,  0x4f,  0xa3,  0x00,  0xfc,  0x0d,
		0x00,  0x00,  0x0f,  0x4f,  0xa3,  0x00,  0xfc,  0x0d,  0x00,  0x00,  0x0f,  0x4f,
		0xa3,  0x00,  0xfc,  0x0800,  0x07fc,  0x07f8,  0x0200,  0x0201,  0x0202
	},
	{
		224,   5120,  0xaa,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x00,  0x02,  0x0d,
		0x00,  0x0d,  0x00,  0x0d,  0x00,  0x00,  0x0f,  0x4f,  0xa3,  0x00,  0xfc,  0x0d,
		0x00,  0x00,  0x0f,  0x4f,  0xa3,  0x00,  0xfc,  0x0d,  0x00,  0x00,  0x0f,  0x4f,
		0xa3,  0x00,  0xfc,  0x0804,  0x0800,  0x07fc,  0x01ff,  0x0200,  0x0201
	},
	{
		226,   5130,  0xae,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x01,  0x02,  0x0d,
		0x00,  0x0d,  0x00,  0x0d,  0x00,  0x00,  0x0f,  0x4f,  0xa3,  0x00,  0xfc,  0x0d,
		0x00,  0x00,  0x0f,  0x4f,  0xa3,  0x00,  0xfc,  0x0d,  0x00,  0x00,  0x0f,  0x4f,
		0xa3,  0x00,  0xfc,  0x0808,  0x0804,  0x0800,  0x01fe,  0x01ff,  0x0200
	},
	{
		228,   5140,  0xb1,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x02,  0x02,  0x0d,
		0x00,  0x0d,  0x00,  0x0d,  0x00,  0x00,  0x0f,  0x4f,  0xa3,  0x00,  0xfc,  0x0d,
		0x00,  0x00,  0x0f,  0x4f,  0xa3,  0x00,  0xfc,  0x0d,  0x00,  0x00,  0x0f,  0x4f,
		0xa3,  0x00,  0xfc,  0x080c,  0x0808,  0x0804,  0x01fd,  0x01fe,  0x01ff
	},
	{
		32,   5160,  0xb8,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x04,  0x02,  0x0d,
		0x00,  0x0d,  0x00,  0x0d,  0x00,  0x00,  0x0f,  0x4f,  0xa3,  0x00,  0xfc,  0x0d,
		0x00,  0x00,  0x0f,  0x4f,  0xa3,  0x00,  0xfc,  0x0d,  0x00,  0x00,  0x0f,  0x4f,
		0xa3,  0x00,  0xfc,  0x0814,  0x0810,  0x080c,  0x01fb,  0x01fc,  0x01fd
	},
	{
		34,   5170,  0xbb,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x05,  0x02,  0x0d,
		0x00,  0x0d,  0x00,  0x0d,  0x00,  0x00,  0x0f,  0x4f,  0xa3,  0x00,  0xfc,  0x0d,
		0x00,  0x00,  0x0f,  0x4f,  0xa3,  0x00,  0xfc,  0x0d,  0x00,  0x00,  0x0f,  0x4f,
		0xa3,  0x00,  0xfc,  0x0818,  0x0814,  0x0810,  0x01fa,  0x01fb,  0x01fc
	},
	{
		36,   5180,  0xbe,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x06,  0x02,  0x0c,
		0x00,  0x0c,  0x00,  0x0c,  0x00,  0x00,  0x0f,  0x4f,  0xa3,  0x00,  0xfc,  0x0c,
		0x00,  0x00,  0x0f,  0x4f,  0xa3,  0x00,  0xfc,  0x0c,  0x00,  0x00,  0x0f,  0x4f,
		0xa3,  0x00,  0xfc,  0x081c,  0x0818,  0x0814,  0x01f9,  0x01fa,  0x01fb
	},
	{
		38,   5190,  0xc2,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x07,  0x02,  0x0c,
		0x00,  0x0c,  0x00,  0x0c,  0x00,  0x00,  0x0f,  0x4f,  0xa3,  0x00,  0xfc,  0x0c,
		0x00,  0x00,  0x0f,  0x4f,  0xa3,  0x00,  0xfc,  0x0c,  0x00,  0x00,  0x0f,  0x4f,
		0xa3,  0x00,  0xfc,  0x0820,  0x081c,  0x0818,  0x01f8,  0x01f9,  0x01fa
	},
	{
		40,   5200,  0xc5,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x08,  0x02,  0x0c,
		0x00,  0x0c,  0x00,  0x0c,  0x00,  0x00,  0x0f,  0x4f,  0x93,  0x00,  0xf8,  0x0c,
		0x00,  0x00,  0x0f,  0x4f,  0x93,  0x00,  0xf8,  0x0c,  0x00,  0x00,  0x0f,  0x4f,
		0x93,  0x00,  0xf8,  0x0824,  0x0820,  0x081c,  0x01f7,  0x01f8,  0x01f9
	},
	{
		42,   5210,  0xc8,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x09,  0x02,  0x0c,
		0x00,  0x0c,  0x00,  0x0c,  0x00,  0x00,  0x0f,  0x4f,  0x93,  0x00,  0xf8,  0x0c,
		0x00,  0x00,  0x0f,  0x4f,  0x93,  0x00,  0xf8,  0x0c,  0x00,  0x00,  0x0f,  0x4f,
		0x93,  0x00,  0xf8,  0x0828,  0x0824,  0x0820,  0x01f6,  0x01f7,  0x01f8
	},
	{
		44,   5220,  0xcc,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x0a,  0x02,  0x0c,
		0x00,  0x0c,  0x00,  0x0c,  0x00,  0x00,  0x0f,  0x4f,  0x93,  0x00,  0xf8,  0x0c,
		0x00,  0x00,  0x0f,  0x4f,  0x93,  0x00,  0xf8,  0x0c,  0x00,  0x00,  0x0f,  0x4f,
		0x93,  0x00,  0xf8,  0x082c,  0x0828,  0x0824,  0x01f5,  0x01f6,  0x01f7
	},
	{
		46,   5230,  0xcf,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x0b,  0x02,  0x0c,
		0x00,  0x0c,  0x00,  0x0c,  0x00,  0x00,  0x0f,  0x4f,  0x93,  0x00,  0xf8,  0x0c,
		0x00,  0x00,  0x0f,  0x4f,  0x93,  0x00,  0xf8,  0x0c,  0x00,  0x00,  0x0f,  0x4f,
		0x93,  0x00,  0xf8,  0x0830,  0x082c,  0x0828,  0x01f4,  0x01f5,  0x01f6
	},
	{
		48,   5240,  0xd2,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x0c,  0x02,  0x0c,
		0x00,  0x0c,  0x00,  0x0c,  0x00,  0x00,  0x0f,  0x4f,  0x93,  0x00,  0xf8,  0x0c,
		0x00,  0x00,  0x0f,  0x4f,  0x93,  0x00,  0xf8,  0x0c,  0x00,  0x00,  0x0f,  0x4f,
		0x93,  0x00,  0xf8,  0x0834,  0x0830,  0x082c,  0x01f3,  0x01f4,  0x01f5
	},
	{
		50,   5250,  0xd6,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x0d,  0x02,  0x0c,
		0x00,  0x0c,  0x00,  0x0c,  0x00,  0x00,  0x0f,  0x4f,  0x93,  0x00,  0xf8,  0x0c,
		0x00,  0x00,  0x0f,  0x4f,  0x93,  0x00,  0xf8,  0x0c,  0x00,  0x00,  0x0f,  0x4f,
		0x93,  0x00,  0xf8,  0x0838,  0x0834,  0x0830,  0x01f2,  0x01f3,  0x01f4
	},
	{
		52,   5260,  0xd9,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x0e,  0x02,  0x0b,
		0x00,  0x0b,  0x00,  0x0b,  0x00,  0x00,  0x0f,  0x4f,  0x93,  0x00,  0xf8,  0x0b,
		0x00,  0x00,  0x0f,  0x4f,  0x93,  0x00,  0xf8,  0x0b,  0x00,  0x00,  0x0f,  0x4f,
		0x93,  0x00,  0xf8,  0x083c,  0x0838,  0x0834,  0x01f1,  0x01f2,  0x01f3
	},
	{
		54,   5270,  0xdc,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x0f,  0x02,  0x0b,
		0x00,  0x0b,  0x00,  0x0b,  0x00,  0x00,  0x0f,  0x4f,  0x93,  0x00,  0xf8,  0x0b,
		0x00,  0x00,  0x0f,  0x4f,  0x93,  0x00,  0xf8,  0x0b,  0x00,  0x00,  0x0f,  0x4f,
		0x93,  0x00,  0xf8,  0x0840,  0x083c,  0x0838,  0x01f0,  0x01f1,  0x01f2
	},
	{
		56,   5280,  0xe0,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x10,  0x02,  0x0b,
		0x00,  0x0b,  0x00,  0x0b,  0x00,  0x00,  0x0f,  0x4f,  0x93,  0x00,  0xf8,  0x0b,
		0x00,  0x00,  0x0f,  0x4f,  0x93,  0x00,  0xf8,  0x0b,  0x00,  0x00,  0x0f,  0x4f,
		0x93,  0x00,  0xf8,  0x0844,  0x0840,  0x083c,  0x01f0,  0x01f0,  0x01f1
	},
	{
		58,   5290,  0xe3,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x11,  0x02,  0x0b,
		0x00,  0x0b,  0x00,  0x0b,  0x00,  0x00,  0x0f,  0x4f,  0x93,  0x00,  0xf8,  0x0b,
		0x00,  0x00,  0x0f,  0x4f,  0x93,  0x00,  0xf8,  0x0b,  0x00,  0x00,  0x0f,  0x4f,
		0x93,  0x00,  0xf8,  0x0848,  0x0844,  0x0840,  0x01ef,  0x01f0,  0x01f0
	},
	{
		60,   5300,  0xe6,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x12,  0x02,  0x0b,
		0x00,  0x0b,  0x00,  0x0b,  0x00,  0x00,  0x0f,  0x4c,  0x83,  0x00,  0xf5,  0x0b,
		0x00,  0x00,  0x0f,  0x4c,  0x83,  0x00,  0xf5,  0x0b,  0x00,  0x00,  0x0f,  0x4c,
		0x83,  0x00,  0xf5,  0x084c,  0x0848,  0x0844,  0x01ee,  0x01ef,  0x01f0
	},
	{
		62,   5310,  0xea,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x13,  0x02,  0x0b,
		0x00,  0x0b,  0x00,  0x0b,  0x00,  0x00,  0x0f,  0x4c,  0x83,  0x00,  0xf5,  0x0b,
		0x00,  0x00,  0x0f,  0x4c,  0x83,  0x00,  0xf5,  0x0b,  0x00,  0x00,  0x0f,  0x4c,
		0x83,  0x00,  0xf5,  0x0850,  0x084c,  0x0848,  0x01ed,  0x01ee,  0x01ef
	},
	{
		64,   5320,  0xed,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x14,  0x02,  0x0b,
		0x00,  0x0b,  0x00,  0x0b,  0x00,  0x00,  0x0f,  0x4c,  0x83,  0x00,  0xf5,  0x0b,
		0x00,  0x00,  0x0f,  0x4c,  0x83,  0x00,  0xf5,  0x0b,  0x00,  0x00,  0x0f,  0x4c,
		0x83,  0x00,  0xf5,  0x0854,  0x0850,  0x084c,  0x01ec,  0x01ed,  0x01ee
	},
	{
		66,   5330,  0xf0,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x15,  0x02,  0x0b,
		0x00,  0x0b,  0x00,  0x0b,  0x00,  0x00,  0x0f,  0x4c,  0x83,  0x00,  0xf5,  0x0b,
		0x00,  0x00,  0x0f,  0x4c,  0x83,  0x00,  0xf5,  0x0b,  0x00,  0x00,  0x0f,  0x4c,
		0x83,  0x00,  0xf5,  0x0858,  0x0854,  0x0850,  0x01eb,  0x01ec,  0x01ed
	},
	{
		68,   5340,  0xf4,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x16,  0x02,  0x0a,
		0x00,  0x0a,  0x00,  0x0a,  0x00,  0x00,  0x0f,  0x4c,  0x83,  0x00,  0xf5,  0x0a,
		0x00,  0x00,  0x0f,  0x4c,  0x83,  0x00,  0xf5,  0x0a,  0x00,  0x00,  0x0f,  0x4c,
		0x83,  0x00,  0xf5,  0x085c,  0x0858,  0x0854,  0x01ea,  0x01eb,  0x01ec
	},
	{
		70,   5350,  0xf7,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x17,  0x02,  0x0a,
		0x00,  0x0a,  0x00,  0x0a,  0x00,  0x00,  0x0f,  0x4c,  0x83,  0x00,  0xf5,  0x0a,
		0x00,  0x00,  0x0f,  0x4c,  0x83,  0x00,  0xf5,  0x0a,  0x00,  0x00,  0x0f,  0x4c,
		0x83,  0x00,  0xf5,  0x0860,  0x085c,  0x0858,  0x01e9,  0x01ea,  0x01eb
	},
	{
		72,   5360,  0xfa,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x18,  0x02,  0x0a,
		0x00,  0x0a,  0x00,  0x0a,  0x00,  0x00,  0x0f,  0x4c,  0x83,  0x00,  0xf5,  0x0a,
		0x00,  0x00,  0x0f,  0x4c,  0x83,  0x00,  0xf5,  0x0a,  0x00,  0x00,  0x0f,  0x4c,
		0x83,  0x00,  0xf5,  0x0864,  0x0860,  0x085c,  0x01e8,  0x01e9,  0x01ea
	},
	{
		74,   5370,  0xfe,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x19,  0x02,  0x0a,
		0x00,  0x0a,  0x00,  0x0a,  0x00,  0x00,  0x0f,  0x4c,  0x83,  0x00,  0xf5,  0x0a,
		0x00,  0x00,  0x0f,  0x4c,  0x83,  0x00,  0xf5,  0x0a,  0x00,  0x00,  0x0f,  0x4c,
		0x83,  0x00,  0xf5,  0x0868,  0x0864,  0x0860,  0x01e7,  0x01e8,  0x01e9
	},
	{
		76,   5380,  0x01,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x1a,  0x02,  0x0a,
		0x00,  0x0a,  0x00,  0x0a,  0x00,  0x00,  0x0f,  0x4c,  0x83,  0x00,  0xf5,  0x0a,
		0x00,  0x00,  0x0f,  0x4c,  0x83,  0x00,  0xf5,  0x0a,  0x00,  0x00,  0x0f,  0x4c,
		0x83,  0x00,  0xf5,  0x086c,  0x0868,  0x0864,  0x01e6,  0x01e7,  0x01e8
	},
	{
		78,   5390,  0x04,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x1b,  0x02,  0x0a,
		0x00,  0x0a,  0x00,  0x0a,  0x00,  0x00,  0x0f,  0x4c,  0x83,  0x00,  0xf5,  0x0a,
		0x00,  0x00,  0x0f,  0x4c,  0x83,  0x00,  0xf5,  0x0a,  0x00,  0x00,  0x0f,  0x4c,
		0x83,  0x00,  0xf5,  0x0870,  0x086c,  0x0868,  0x01e5,  0x01e6,  0x01e7
	},
	{
		80,   5400,  0x08,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x1c,  0x02,  0x0a,
		0x00,  0x0a,  0x00,  0x0a,  0x00,  0x00,  0x0d,  0x49,  0x53,  0x00,  0xb1,  0x0a,
		0x00,  0x00,  0x0d,  0x49,  0x53,  0x00,  0xb1,  0x0a,  0x00,  0x00,  0x0d,  0x49,
		0x53,  0x00,  0xb1,  0x0874,  0x0870,  0x086c,  0x01e5,  0x01e5,  0x01e6
	},
	{
		82,   5410,  0x0b,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x1d,  0x02,  0x0a,
		0x00,  0x0a,  0x00,  0x0a,  0x00,  0x00,  0x0d,  0x49,  0x53,  0x00,  0xb1,  0x0a,
		0x00,  0x00,  0x0d,  0x49,  0x53,  0x00,  0xb1,  0x0a,  0x00,  0x00,  0x0d,  0x49,
		0x53,  0x00,  0xb1,  0x0878,  0x0874,  0x0870,  0x01e4,  0x01e5,  0x01e5
	},
	{
		84,   5420,  0x0e,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x1e,  0x02,  0x09,
		0x00,  0x09,  0x00,  0x09,  0x00,  0x00,  0x0d,  0x49,  0x53,  0x00,  0xb1,  0x09,
		0x00,  0x00,  0x0d,  0x49,  0x53,  0x00,  0xb1,  0x09,  0x00,  0x00,  0x0d,  0x49,
		0x53,  0x00,  0xb1,  0x087c,  0x0878,  0x0874,  0x01e3,  0x01e4,  0x01e5
	},
	{
		86,   5430,  0x12,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x1f,  0x02,  0x09,
		0x00,  0x09,  0x00,  0x09,  0x00,  0x00,  0x0d,  0x49,  0x53,  0x00,  0xb1,  0x09,
		0x00,  0x00,  0x0d,  0x49,  0x53,  0x00,  0xb1,  0x09,  0x00,  0x00,  0x0d,  0x49,
		0x53,  0x00,  0xb1,  0x0880,  0x087c,  0x0878,  0x01e2,  0x01e3,  0x01e4
	},
	{
		88,   5440,  0x15,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x20,  0x02,  0x09,
		0x00,  0x09,  0x00,  0x09,  0x00,  0x00,  0x0d,  0x49,  0x53,  0x00,  0xb1,  0x09,
		0x00,  0x00,  0x0d,  0x49,  0x53,  0x00,  0xb1,  0x09,  0x00,  0x00,  0x0d,  0x49,
		0x53,  0x00,  0xb1,  0x0884,  0x0880,  0x087c,  0x01e1,  0x01e2,  0x01e3
	},
	{
		90,   5450,  0x18,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x21,  0x02,  0x09,
		0x00,  0x09,  0x00,  0x09,  0x00,  0x00,  0x0d,  0x49,  0x53,  0x00,  0xb1,  0x09,
		0x00,  0x00,  0x0d,  0x49,  0x53,  0x00,  0xb1,  0x09,  0x00,  0x00,  0x0d,  0x49,
		0x53,  0x00,  0xb1,  0x0888,  0x0884,  0x0880,  0x01e0,  0x01e1,  0x01e2
	},
	{
		92,   5460,  0x1c,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x22,  0x02,  0x09,
		0x00,  0x09,  0x00,  0x09,  0x00,  0x00,  0x0d,  0x49,  0x53,  0x00,  0xb1,  0x09,
		0x00,  0x00,  0x0d,  0x49,  0x53,  0x00,  0xb1,  0x09,  0x00,  0x00,  0x0d,  0x49,
		0x53,  0x00,  0xb1,  0x088c,  0x0888,  0x0884,  0x01df,  0x01e0,  0x01e1
	},
	{
		94,   5470,  0x1f,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x23,  0x02,  0x09,
		0x00,  0x09,  0x00,  0x09,  0x00,  0x00,  0x0d,  0x49,  0x53,  0x00,  0xb1,  0x09,
		0x00,  0x00,  0x0d,  0x49,  0x53,  0x00,  0xb1,  0x09,  0x00,  0x00,  0x0d,  0x49,
		0x53,  0x00,  0xb1,  0x0890,  0x088c,  0x0888,  0x01de,  0x01df,  0x01e0
	},
	{
		96,   5480,  0x22,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x24,  0x02,  0x09,
		0x00,  0x09,  0x00,  0x09,  0x00,  0x00,  0x0d,  0x49,  0x53,  0x00,  0xb1,  0x09,
		0x00,  0x00,  0x0d,  0x49,  0x53,  0x00,  0xb1,  0x09,  0x00,  0x00,  0x0d,  0x49,
		0x53,  0x00,  0xb1,  0x0894,  0x0890,  0x088c,  0x01dd,  0x01de,  0x01df
	},
	{
		98,   5490,  0x26,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x25,  0x02,  0x09,
		0x00,  0x09,  0x00,  0x09,  0x00,  0x00,  0x0d,  0x49,  0x53,  0x00,  0xb1,  0x09,
		0x00,  0x00,  0x0d,  0x49,  0x53,  0x00,  0xb1,  0x09,  0x00,  0x00,  0x0d,  0x49,
		0x53,  0x00,  0xb1,  0x0898,  0x0894,  0x0890,  0x01dd,  0x01dd,  0x01de
	},
	{
		100,   5500,  0x29,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x26,  0x02,  0x09,
		0x00,  0x09,  0x00,  0x09,  0x00,  0x00,  0x0a,  0x46,  0x43,  0x00,  0x80,  0x09,
		0x00,  0x00,  0x0a,  0x46,  0x43,  0x00,  0x80,  0x09,  0x00,  0x00,  0x0a,  0x46,
		0x43,  0x00,  0x80,  0x089c,  0x0898,  0x0894,  0x01dc,  0x01dd,  0x01dd
	},
	{
		102,   5510,  0x2c,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x27,  0x02,  0x09,
		0x00,  0x09,  0x00,  0x09,  0x00,  0x00,  0x0a,  0x46,  0x43,  0x00,  0x80,  0x09,
		0x00,  0x00,  0x0a,  0x46,  0x43,  0x00,  0x80,  0x09,  0x00,  0x00,  0x0a,  0x46,
		0x43,  0x00,  0x80,  0x08a0,  0x089c,  0x0898,  0x01db,  0x01dc,  0x01dd
	},
	{
		104,   5520,  0x30,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x28,  0x02,  0x08,
		0x00,  0x08,  0x00,  0x08,  0x00,  0x00,  0x0a,  0x46,  0x43,  0x00,  0x80,  0x08,
		0x00,  0x00,  0x0a,  0x46,  0x43,  0x00,  0x80,  0x08,  0x00,  0x00,  0x0a,  0x46,
		0x43,  0x00,  0x80,  0x08a4,  0x08a0,  0x089c,  0x01da,  0x01db,  0x01dc
	},
	{
		106,   5530,  0x33,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x29,  0x02,  0x08,
		0x00,  0x08,  0x00,  0x08,  0x00,  0x00,  0x0a,  0x46,  0x43,  0x00,  0x80,  0x08,
		0x00,  0x00,  0x0a,  0x46,  0x43,  0x00,  0x80,  0x08,  0x00,  0x00,  0x0a,  0x46,
		0x43,  0x00,  0x80,  0x08a8,  0x08a4,  0x08a0,  0x01d9,  0x01da,  0x01db
	},
	{
		108,   5540,  0x36,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x2a,  0x02,  0x08,
		0x00,  0x08,  0x00,  0x08,  0x00,  0x00,  0x0a,  0x46,  0x43,  0x00,  0x80,  0x08,
		0x00,  0x00,  0x0a,  0x46,  0x43,  0x00,  0x80,  0x08,  0x00,  0x00,  0x0a,  0x46,
		0x43,  0x00,  0x80,  0x08ac,  0x08a8,  0x08a4,  0x01d8,  0x01d9,  0x01da
	},
	{
		110,   5550,  0x3a,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x2b,  0x02,  0x08,
		0x00,  0x08,  0x00,  0x08,  0x00,  0x00,  0x0a,  0x46,  0x43,  0x00,  0x80,  0x08,
		0x00,  0x00,  0x0a,  0x46,  0x43,  0x00,  0x80,  0x08,  0x00,  0x00,  0x0a,  0x46,
		0x43,  0x00,  0x80,  0x08b0,  0x08ac,  0x08a8,  0x01d7,  0x01d8,  0x01d9
	},
	{
		112,   5560,  0x3d,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x2c,  0x02,  0x08,
		0x00,  0x08,  0x00,  0x08,  0x00,  0x00,  0x0a,  0x46,  0x43,  0x00,  0x80,  0x08,
		0x00,  0x00,  0x0a,  0x46,  0x43,  0x00,  0x80,  0x08,  0x00,  0x00,  0x0a,  0x46,
		0x43,  0x00,  0x80,  0x08b4,  0x08b0,  0x08ac,  0x01d7,  0x01d7,  0x01d8
	},
	{
		114,   5570,  0x40,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x2d,  0x02,  0x08,
		0x00,  0x08,  0x00,  0x08,  0x00,  0x00,  0x0a,  0x46,  0x43,  0x00,  0x80,  0x08,
		0x00,  0x00,  0x0a,  0x46,  0x43,  0x00,  0x80,  0x08,  0x00,  0x00,  0x0a,  0x46,
		0x43,  0x00,  0x80,  0x08b8,  0x08b4,  0x08b0,  0x01d6,  0x01d7,  0x01d7
	},
	{
		116,   5580,  0x44,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x2e,  0x02,  0x08,
		0x00,  0x08,  0x00,  0x08,  0x00,  0x00,  0x0a,  0x46,  0x43,  0x00,  0x80,  0x08,
		0x00,  0x00,  0x0a,  0x46,  0x43,  0x00,  0x80,  0x08,  0x00,  0x00,  0x0a,  0x46,
		0x43,  0x00,  0x80,  0x08bc,  0x08b8,  0x08b4,  0x01d5,  0x01d6,  0x01d7
	},
	{
		118,   5590,  0x47,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x2f,  0x02,  0x08,
		0x00,  0x08,  0x00,  0x08,  0x00,  0x00,  0x0a,  0x46,  0x43,  0x00,  0x80,  0x08,
		0x00,  0x00,  0x0a,  0x46,  0x43,  0x00,  0x80,  0x08,  0x00,  0x00,  0x0a,  0x46,
		0x43,  0x00,  0x80,  0x08c0,  0x08bc,  0x08b8,  0x01d4,  0x01d5,  0x01d6
	},
	{
		120,   5600,  0x4a,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x30,  0x02,  0x08,
		0x00,  0x08,  0x00,  0x08,  0x00,  0x00,  0x09,  0x44,  0x23,  0x00,  0x60,  0x08,
		0x00,  0x00,  0x09,  0x44,  0x23,  0x00,  0x60,  0x08,  0x00,  0x00,  0x09,  0x44,
		0x23,  0x00,  0x60,  0x08c4,  0x08c0,  0x08bc,  0x01d3,  0x01d4,  0x01d5
	},
	{
		122,   5610,  0x4e,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x31,  0x02,  0x08,
		0x00,  0x08,  0x00,  0x08,  0x00,  0x00,  0x09,  0x44,  0x23,  0x00,  0x60,  0x08,
		0x00,  0x00,  0x09,  0x44,  0x23,  0x00,  0x60,  0x08,  0x00,  0x00,  0x09,  0x44,
		0x23,  0x00,  0x60,  0x08c8,  0x08c4,  0x08c0,  0x01d2,  0x01d3,  0x01d4
	},
	{
		124,   5620,  0x51,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x32,  0x02,  0x07,
		0x00,  0x07,  0x00,  0x07,  0x00,  0x00,  0x09,  0x44,  0x23,  0x00,  0x60,  0x07,
		0x00,  0x00,  0x09,  0x44,  0x23,  0x00,  0x60,  0x07,  0x00,  0x00,  0x09,  0x44,
		0x23,  0x00,  0x60,  0x08cc,  0x08c8,  0x08c4,  0x01d2,  0x01d2,  0x01d3
	},
	{
		126,   5630,  0x54,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x33,  0x02,  0x07,
		0x00,  0x07,  0x00,  0x07,  0x00,  0x00,  0x09,  0x44,  0x23,  0x00,  0x60,  0x07,
		0x00,  0x00,  0x09,  0x44,  0x23,  0x00,  0x60,  0x07,  0x00,  0x00,  0x09,  0x44,
		0x23,  0x00,  0x60,  0x08d0,  0x08cc,  0x08c8,  0x01d1,  0x01d2,  0x01d2
	},
	{
		128,   5640,  0x58,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x34,  0x02,  0x07,
		0x00,  0x07,  0x00,  0x07,  0x00,  0x00,  0x09,  0x44,  0x23,  0x00,  0x60,  0x07,
		0x00,  0x00,  0x09,  0x44,  0x23,  0x00,  0x60,  0x07,  0x00,  0x00,  0x09,  0x44,
		0x23,  0x00,  0x60,  0x08d4,  0x08d0,  0x08cc,  0x01d0,  0x01d1,  0x01d2
	},
	{
		130,   5650,  0x5b,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x35,  0x02,  0x07,
		0x00,  0x07,  0x00,  0x07,  0x00,  0x00,  0x09,  0x43,  0x23,  0x00,  0x60,  0x07,
		0x00,  0x00,  0x09,  0x43,  0x23,  0x00,  0x60,  0x07,  0x00,  0x00,  0x09,  0x43,
		0x23,  0x00,  0x60,  0x08d8,  0x08d4,  0x08d0,  0x01cf,  0x01d0,  0x01d1
	},
	{
		132,   5660,  0x5e,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x36,  0x02,  0x07,
		0x00,  0x07,  0x00,  0x07,  0x00,  0x00,  0x09,  0x43,  0x23,  0x00,  0x60,  0x07,
		0x00,  0x00,  0x09,  0x43,  0x23,  0x00,  0x60,  0x07,  0x00,  0x00,  0x09,  0x43,
		0x23,  0x00,  0x60,  0x08dc,  0x08d8,  0x08d4,  0x01ce,  0x01cf,  0x01d0
	},
	{
		134,   5670,  0x62,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x37,  0x02,  0x07,
		0x00,  0x07,  0x00,  0x07,  0x00,  0x00,  0x09,  0x43,  0x23,  0x00,  0x60,  0x07,
		0x00,  0x00,  0x09,  0x43,  0x23,  0x00,  0x60,  0x07,  0x00,  0x00,  0x09,  0x43,
		0x23,  0x00,  0x60,  0x08e0,  0x08dc,  0x08d8,  0x01ce,  0x01ce,  0x01cf
	},
	{
		136,   5680,  0x65,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x38,  0x02,  0x07,
		0x00,  0x07,  0x00,  0x07,  0x00,  0x00,  0x09,  0x42,  0x23,  0x00,  0x60,  0x07,
		0x00,  0x00,  0x09,  0x42,  0x23,  0x00,  0x60,  0x07,  0x00,  0x00,  0x09,  0x42,
		0x23,  0x00,  0x60,  0x08e4,  0x08e0,  0x08dc,  0x01cd,  0x01ce,  0x01ce
	},
	{
		138,   5690,  0x68,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x39,  0x02,  0x07,
		0x00,  0x07,  0x00,  0x07,  0x00,  0x00,  0x09,  0x42,  0x23,  0x00,  0x60,  0x07,
		0x00,  0x00,  0x09,  0x42,  0x23,  0x00,  0x60,  0x07,  0x00,  0x00,  0x09,  0x42,
		0x23,  0x00,  0x60,  0x08e8,  0x08e4,  0x08e0,  0x01cc,  0x01cd,  0x01ce
	},
	{
		140,   5700,  0x6c,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x3a,  0x02,  0x07,
		0x00,  0x07,  0x00,  0x07,  0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x30,  0x07,
		0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x30,  0x07,  0x00,  0x00,  0x08,  0x42,
		0x13,  0x00,  0x30,  0x08ec,  0x08e8,  0x08e4,  0x01cb,  0x01cc,  0x01cd
	},
	{
		142,   5710,  0x6f,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x3b,  0x02,  0x07,
		0x00,  0x07,  0x00,  0x07,  0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x30,  0x07,
		0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x30,  0x07,  0x00,  0x00,  0x08,  0x42,
		0x13,  0x00,  0x30,  0x08f0,  0x08ec,  0x08e8,  0x01ca,  0x01cb,  0x01cc
	},
	{
		144,   5720,  0x72,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x3c,  0x02,  0x07,
		0x00,  0x07,  0x00,  0x07,  0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x30,  0x07,
		0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x30,  0x07,  0x00,  0x00,  0x08,  0x42,
		0x13,  0x00,  0x30,  0x08f4,  0x08f0,  0x08ec,  0x01c9,  0x01ca,  0x01cb
	},
	{
		145,   5725,  0x74,  0x17,  0x20,  0x1f,  0x08,  0x08,  0x3f,  0x79,  0x04,  0x06,
		0x00,  0x06,  0x00,  0x06,  0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x30,  0x06,
		0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x30,  0x06,  0x00,  0x00,  0x08,  0x42,
		0x13,  0x00,  0x30,  0x08f6,  0x08f2,  0x08ee,  0x01c9,  0x01ca,  0x01cb
	},
	{
		146,   5730,  0x76,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x3d,  0x02,  0x06,
		0x00,  0x06,  0x00,  0x06,  0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x30,  0x06,
		0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x30,  0x06,  0x00,  0x00,  0x08,  0x42,
		0x13,  0x00,  0x30,  0x08f8,  0x08f4,  0x08f0,  0x01c9,  0x01c9,  0x01ca
	},
	{
		147,   5735,  0x77,  0x17,  0x20,  0x1f,  0x08,  0x08,  0x3f,  0x7b,  0x04,  0x06,
		0x00,  0x06,  0x00,  0x06,  0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x30,  0x06,
		0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x30,  0x06,  0x00,  0x00,  0x08,  0x42,
		0x13,  0x00,  0x30,  0x08fa,  0x08f6,  0x08f2,  0x01c8,  0x01c9,  0x01ca
	},
	{
		148,   5740,  0x79,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x3e,  0x02,  0x06,
		0x00,  0x06,  0x00,  0x06,  0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x30,  0x06,
		0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x30,  0x06,  0x00,  0x00,  0x08,  0x42,
		0x13,  0x00,  0x30,  0x08fc,  0x08f8,  0x08f4,  0x01c8,  0x01c9,  0x01c9
	},
	{
		149,   5745,  0x7b,  0x17,  0x20,  0x1f,  0x08,  0x08,  0x3f,  0x7d,  0x04,  0x06,
		0x00,  0x06,  0x00,  0x06,  0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x30,  0x06,
		0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x30,  0x06,  0x00,  0x00,  0x08,  0x42,
		0x13,  0x00,  0x30,  0x08fe,  0x08fa,  0x08f6,  0x01c8,  0x01c8,  0x01c9
	},
	{
		150,   5750,  0x7c,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x3f,  0x02,  0x06,
		0x00,  0x06,  0x00,  0x06,  0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x00,  0x06,
		0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x00,  0x06,  0x00,  0x00,  0x08,  0x42,
		0x13,  0x00,  0x00,  0x0900,  0x08fc,  0x08f8,  0x01c7,  0x01c8,  0x01c9
	},
	{
		151,   5755,  0x7e,  0x17,  0x20,  0x1f,  0x08,  0x08,  0x3f,  0x7f,  0x04,  0x06,
		0x00,  0x06,  0x00,  0x06,  0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x00,  0x06,
		0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x00,  0x06,  0x00,  0x00,  0x08,  0x42,
		0x13,  0x00,  0x00,  0x0902,  0x08fe,  0x08fa,  0x01c7,  0x01c8,  0x01c8
	},
	{
		152,   5760,  0x80,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x40,  0x02,  0x06,
		0x00,  0x06,  0x00,  0x06,  0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x00,  0x06,
		0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x00,  0x06,  0x00,  0x00,  0x08,  0x42,
		0x13,  0x00,  0x00,  0x0904,  0x0900,  0x08fc,  0x01c6,  0x01c7,  0x01c8
	},
	{
		153,   5765,  0x81,  0x17,  0x20,  0x1f,  0x08,  0x08,  0x3f,  0x81,  0x04,  0x06,
		0x00,  0x06,  0x00,  0x06,  0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x00,  0x06,
		0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x00,  0x06,  0x00,  0x00,  0x08,  0x42,
		0x13,  0x00,  0x00,  0x0906,  0x0902,  0x08fe,  0x01c6,  0x01c7,  0x01c8
	},
	{
		154,   5770,  0x83,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x41,  0x02,  0x06,
		0x00,  0x06,  0x00,  0x06,  0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x00,  0x06,
		0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x00,  0x06,  0x00,  0x00,  0x08,  0x42,
		0x13,  0x00,  0x00,  0x0908,  0x0904,  0x0900,  0x01c6,  0x01c6,  0x01c7
	},
	{
		155,   5775,  0x85,  0x17,  0x20,  0x1f,  0x08,  0x08,  0x3f,  0x83,  0x04,  0x06,
		0x00,  0x06,  0x00,  0x06,  0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x00,  0x06,
		0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x00,  0x06,  0x00,  0x00,  0x08,  0x42,
		0x13,  0x00,  0x00,  0x090a,  0x0906,  0x0902,  0x01c5,  0x01c6,  0x01c7
	},
	{
		156,   5780,  0x86,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x42,  0x02,  0x06,
		0x00,  0x06,  0x00,  0x06,  0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x00,  0x06,
		0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x00,  0x06,  0x00,  0x00,  0x08,  0x42,
		0x13,  0x00,  0x00,  0x090c,  0x0908,  0x0904,  0x01c5,  0x01c6,  0x01c6
	},
	{
		157,   5785,  0x88,  0x17,  0x20,  0x1f,  0x08,  0x08,  0x3f,  0x85,  0x04,  0x05,
		0x00,  0x05,  0x00,  0x05,  0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x00,  0x05,
		0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x00,  0x05,  0x00,  0x00,  0x08,  0x42,
		0x13,  0x00,  0x00,  0x090e,  0x090a,  0x0906,  0x01c4,  0x01c5,  0x01c6
	},
	{
		158,   5790,  0x8a,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x43,  0x02,  0x05,
		0x00,  0x05,  0x00,  0x05,  0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x00,  0x05,
		0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x00,  0x05,  0x00,  0x00,  0x08,  0x42,
		0x13,  0x00,  0x00,  0x0910,  0x090c,  0x0908,  0x01c4,  0x01c5,  0x01c6
	},
	{
		159,   5795,  0x8b,  0x17,  0x20,  0x1f,  0x08,  0x08,  0x3f,  0x87,  0x04,  0x05,
		0x00,  0x05,  0x00,  0x05,  0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x00,  0x05,
		0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x00,  0x05,  0x00,  0x00,  0x08,  0x42,
		0x13,  0x00,  0x00,  0x0912,  0x090e,  0x090a,  0x01c4,  0x01c4,  0x01c5
	},
	{
		160,   5800,  0x8d,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x44,  0x02,  0x05,
		0x00,  0x05,  0x00,  0x05,  0x00,  0x00,  0x08,  0x41,  0x03,  0x00,  0x00,  0x05,
		0x00,  0x00,  0x08,  0x41,  0x03,  0x00,  0x00,  0x05,  0x00,  0x00,  0x08,  0x41,
		0x03,  0x00,  0x00,  0x0914,  0x0910,  0x090c,  0x01c3,  0x01c4,  0x01c5
	},
	{
		161,   5805,  0x8f,  0x17,  0x20,  0x1f,  0x08,  0x08,  0x3f,  0x89,  0x04,  0x05,
		0x00,  0x05,  0x00,  0x05,  0x00,  0x00,  0x06,  0x41,  0x03,  0x00,  0x00,  0x05,
		0x00,  0x00,  0x06,  0x41,  0x03,  0x00,  0x00,  0x05,  0x00,  0x00,  0x06,  0x41,
		0x03,  0x00,  0x00,  0x0916,  0x0912,  0x090e,  0x01c3,  0x01c4,  0x01c4
	},
	{
		162,   5810,  0x90,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x45,  0x02,  0x05,
		0x00,  0x05,  0x00,  0x05,  0x00,  0x00,  0x06,  0x41,  0x03,  0x00,  0x00,  0x05,
		0x00,  0x00,  0x06,  0x41,  0x03,  0x00,  0x00,  0x05,  0x00,  0x00,  0x06,  0x41,
		0x03,  0x00,  0x00,  0x0918,  0x0914,  0x0910,  0x01c2,  0x01c3,  0x01c4
	},
	{
		163,   5815,  0x92,  0x17,  0x20,  0x1f,  0x08,  0x08,  0x3f,  0x8b,  0x04,  0x05,
		0x00,  0x05,  0x00,  0x05,  0x00,  0x00,  0x06,  0x41,  0x03,  0x00,  0x00,  0x05,
		0x00,  0x00,  0x06,  0x41,  0x03,  0x00,  0x00,  0x05,  0x00,  0x00,  0x06,  0x41,
		0x03,  0x00,  0x00,  0x091a,  0x0916,  0x0912,  0x01c2,  0x01c3,  0x01c4
	},
	{
		164,   5820,  0x94,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x46,  0x02,  0x05,
		0x00,  0x05,  0x00,  0x05,  0x00,  0x00,  0x06,  0x41,  0x03,  0x00,  0x00,  0x05,
		0x00,  0x00,  0x06,  0x41,  0x03,  0x00,  0x00,  0x05,  0x00,  0x00,  0x06,  0x41,
		0x03,  0x00,  0x00,  0x091c,  0x0918,  0x0914,  0x01c2,  0x01c2,  0x01c3
	},
	{
		165,   5825,  0x95,  0x17,  0x20,  0x1f,  0x08,  0x08,  0x3f,  0x8d,  0x04,  0x05,
		0x00,  0x05,  0x00,  0x05,  0x00,  0x00,  0x06,  0x41,  0x03,  0x00,  0x00,  0x05,
		0x00,  0x00,  0x06,  0x41,  0x03,  0x00,  0x00,  0x05,  0x00,  0x00,  0x06,  0x41,
		0x03,  0x00,  0x00,  0x091e,  0x091a,  0x0916,  0x01c1,  0x01c2,  0x01c3
	},
	{
		166,   5830,  0x97,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x47,  0x02,  0x05,
		0x00,  0x05,  0x00,  0x05,  0x00,  0x00,  0x06,  0x41,  0x03,  0x00,  0x00,  0x05,
		0x00,  0x00,  0x06,  0x41,  0x03,  0x00,  0x00,  0x05,  0x00,  0x00,  0x06,  0x41,
		0x03,  0x00,  0x00,  0x0920,  0x091c,  0x0918,  0x01c1,  0x01c2,  0x01c2
	},
	{
		168,   5840,  0x9a,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x48,  0x02,  0x05,
		0x00,  0x05,  0x00,  0x05,  0x00,  0x00,  0x06,  0x41,  0x03,  0x00,  0x00,  0x05,
		0x00,  0x00,  0x06,  0x41,  0x03,  0x00,  0x00,  0x05,  0x00,  0x00,  0x06,  0x41,
		0x03,  0x00,  0x00,  0x0924,  0x0920,  0x091c,  0x01c0,  0x01c1,  0x01c2
	},
	{
		170,   5850,  0x9e,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x49,  0x02,  0x04,
		0x00,  0x04,  0x00,  0x04,  0x00,  0x00,  0x06,  0x41,  0x03,  0x00,  0x00,  0x04,
		0x00,  0x00,  0x06,  0x41,  0x03,  0x00,  0x00,  0x04,  0x00,  0x00,  0x06,  0x41,
		0x03,  0x00,  0x00,  0x0928,  0x0924,  0x0920,  0x01bf,  0x01c0,  0x01c1
	},
	{
		172,   5860,  0xa1,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x4a,  0x02,  0x04,
		0x00,  0x04,  0x00,  0x04,  0x00,  0x00,  0x06,  0x41,  0x03,  0x00,  0x00,  0x04,
		0x00,  0x00,  0x06,  0x41,  0x03,  0x00,  0x00,  0x04,  0x00,  0x00,  0x06,  0x41,
		0x03,  0x00,  0x00,  0x092c,  0x0928,  0x0924,  0x01bf,  0x01bf,  0x01c0
	},
	{
		174,   5870,  0xa4,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x4b,  0x02,  0x04,
		0x00,  0x04,  0x00,  0x04,  0x00,  0x00,  0x06,  0x41,  0x03,  0x00,  0x00,  0x04,
		0x00,  0x00,  0x06,  0x41,  0x03,  0x00,  0x00,  0x04,  0x00,  0x00,  0x06,  0x41,
		0x03,  0x00,  0x00,  0x0930,  0x092c,  0x0928,  0x01be,  0x01bf,  0x01bf
	},
	{
		176,   5880,  0xa8,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x4c,  0x02,  0x03,
		0x00,  0x03,  0x00,  0x03,  0x00,  0x00,  0x06,  0x41,  0x03,  0x00,  0x00,  0x03,
		0x00,  0x00,  0x06,  0x41,  0x03,  0x00,  0x00,  0x03,  0x00,  0x00,  0x06,  0x41,
		0x03,  0x00,  0x00,  0x0934,  0x0930,  0x092c,  0x01bd,  0x01be,  0x01bf
	},
	{
		178,   5890,  0xab,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x4d,  0x02,  0x03,
		0x00,  0x03,  0x00,  0x03,  0x00,  0x00,  0x06,  0x41,  0x03,  0x00,  0x00,  0x03,
		0x00,  0x00,  0x06,  0x41,  0x03,  0x00,  0x00,  0x03,  0x00,  0x00,  0x06,  0x41,
		0x03,  0x00,  0x00,  0x0938,  0x0934,  0x0930,  0x01bc,  0x01bd,  0x01be
	},
	{
		180,   5900,  0xae,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x4e,  0x02,  0x03,
		0x00,  0x03,  0x00,  0x03,  0x00,  0x00,  0x06,  0x41,  0x03,  0x00,  0x00,  0x03,
		0x00,  0x00,  0x06,  0x41,  0x03,  0x00,  0x00,  0x03,  0x00,  0x00,  0x06,  0x41,
		0x03,  0x00,  0x00,  0x093c,  0x0938,  0x0934,  0x01bc,  0x01bc,  0x01bd
	},
	{
		1,   2412,  0x48,  0x16,  0x30,  0x1b,  0x0a,  0x0a,  0x30,  0x6c,  0x09,  0x0f,
		0x0a,  0x00,  0x0a,  0x00,  0x61,  0x03,  0x00,  0x00,  0x00,  0xf0,  0x00,  0x00,
		0x61,  0x03,  0x00,  0x00,  0x00,  0xf0,  0x00,  0x00,  0x61,  0x03,  0x00,  0x00,
		0x00,  0xf0,  0x00,  0x03c9,  0x03c5,  0x03c1,  0x043a,  0x043f,  0x0443
	},
	{
		2,   2417,  0x4b,  0x16,  0x30,  0x1b,  0x0a,  0x0a,  0x30,  0x71,  0x09,  0x0f,
		0x0a,  0x00,  0x0a,  0x00,  0x61,  0x03,  0x00,  0x00,  0x00,  0xf0,  0x00,  0x00,
		0x61,  0x03,  0x00,  0x00,  0x00,  0xf0,  0x00,  0x00,  0x61,  0x03,  0x00,  0x00,
		0x00,  0xf0,  0x00,  0x03cb,  0x03c7,  0x03c3,  0x0438,  0x043d,  0x0441
	},
	{
		3,   2422,  0x4e,  0x16,  0x30,  0x1b,  0x0a,  0x0a,  0x30,  0x76,  0x09,  0x0f,
		0x09,  0x00,  0x09,  0x00,  0x61,  0x03,  0x00,  0x00,  0x00,  0xf0,  0x00,  0x00,
		0x61,  0x03,  0x00,  0x00,  0x00,  0xf0,  0x00,  0x00,  0x61,  0x03,  0x00,  0x00,
		0x00,  0xf0,  0x00,  0x03cd,  0x03c9,  0x03c5,  0x0436,  0x043a,  0x043f
	},
	{
		4,   2427,  0x52,  0x16,  0x30,  0x1b,  0x0a,  0x0a,  0x30,  0x7b,  0x09,  0x0f,
		0x09,  0x00,  0x09,  0x00,  0x61,  0x03,  0x00,  0x00,  0x00,  0xf0,  0x00,  0x00,
		0x61,  0x03,  0x00,  0x00,  0x00,  0xf0,  0x00,  0x00,  0x61,  0x03,  0x00,  0x00,
		0x00,  0xf0,  0x00,  0x03cf,  0x03cb,  0x03c7,  0x0434,  0x0438,  0x043d
	},
	{
		5,   2432,  0x55,  0x16,  0x30,  0x1b,  0x0a,  0x0a,  0x30,  0x80,  0x09,  0x0f,
		0x08,  0x00,  0x08,  0x00,  0x61,  0x03,  0x00,  0x00,  0x00,  0xf0,  0x00,  0x00,
		0x61,  0x03,  0x00,  0x00,  0x00,  0xf0,  0x00,  0x00,  0x61,  0x03,  0x00,  0x00,
		0x00,  0xf0,  0x00,  0x03d1,  0x03cd,  0x03c9,  0x0431,  0x0436,  0x043a
	},
	{
		6,   2437,  0x58,  0x16,  0x30,  0x1b,  0x0a,  0x0a,  0x30,  0x85,  0x09,  0x0f,
		0x08,  0x00,  0x08,  0x00,  0x61,  0x03,  0x00,  0x00,  0x00,  0xf0,  0x00,  0x00,
		0x61,  0x03,  0x00,  0x00,  0x00,  0xf0,  0x00,  0x00,  0x61,  0x03,  0x00,  0x00,
		0x00,  0xf0,  0x00,  0x03d3,  0x03cf,  0x03cb,  0x042f,  0x0434,  0x0438
	},
	{
		7,   2442,  0x5c,  0x16,  0x30,  0x1b,  0x0a,  0x0a,  0x30,  0x8a,  0x09,  0x0f,
		0x07,  0x00,  0x07,  0x00,  0x61,  0x03,  0x00,  0x00,  0x00,  0xf0,  0x00,  0x00,
		0x61,  0x03,  0x00,  0x00,  0x00,  0xf0,  0x00,  0x00,  0x61,  0x03,  0x00,  0x00,
		0x00,  0xf0,  0x00,  0x03d5,  0x03d1,  0x03cd,  0x042d,  0x0431,  0x0436
	},
	{
		8,   2447,  0x5f,  0x16,  0x30,  0x1b,  0x0a,  0x0a,  0x30,  0x8f,  0x09,  0x0f,
		0x07,  0x00,  0x07,  0x00,  0x61,  0x03,  0x00,  0x00,  0x00,  0xf0,  0x00,  0x00,
		0x61,  0x03,  0x00,  0x00,  0x00,  0xf0,  0x00,  0x00,  0x61,  0x03,  0x00,  0x00,
		0x00,  0xf0,  0x00,  0x03d7,  0x03d3,  0x03cf,  0x042b,  0x042f,  0x0434
	},
	{
		9,   2452,  0x62,  0x16,  0x30,  0x1b,  0x0a,  0x0a,  0x30,  0x94,  0x09,  0x0f,
		0x07,  0x00,  0x07,  0x00,  0x61,  0x03,  0x00,  0x00,  0x00,  0xf0,  0x00,  0x00,
		0x61,  0x03,  0x00,  0x00,  0x00,  0xf0,  0x00,  0x00,  0x61,  0x03,  0x00,  0x00,
		0x00,  0xf0,  0x00,  0x03d9,  0x03d5,  0x03d1,  0x0429,  0x042d,  0x0431
	},
	{
		10,   2457,  0x66,  0x16,  0x30,  0x1b,  0x0a,  0x0a,  0x30,  0x99,  0x09,  0x0f,
		0x06,  0x00,  0x06,  0x00,  0x61,  0x03,  0x00,  0x00,  0x00,  0xf0,  0x00,  0x00,
		0x61,  0x03,  0x00,  0x00,  0x00,  0xf0,  0x00,  0x00,  0x61,  0x03,  0x00,  0x00,
		0x00,  0xf0,  0x00,  0x03db,  0x03d7,  0x03d3,  0x0427,  0x042b,  0x042f
	},
	{
		11,   2462,  0x69,  0x16,  0x30,  0x1b,  0x0a,  0x0a,  0x30,  0x9e,  0x09,  0x0f,
		0x06,  0x00,  0x06,  0x00,  0x61,  0x03,  0x00,  0x00,  0x00,  0xf0,  0x00,  0x00,
		0x61,  0x03,  0x00,  0x00,  0x00,  0xf0,  0x00,  0x00,  0x61,  0x03,  0x00,  0x00,
		0x00,  0xf0,  0x00,  0x03dd,  0x03d9,  0x03d5,  0x0424,  0x0429,  0x042d
	},
	{
		12,   2467,  0x6c,  0x16,  0x30,  0x1b,  0x0a,  0x0a,  0x30,  0xa3,  0x09,  0x0f,
		0x05,  0x00,  0x05,  0x00,  0x61,  0x03,  0x00,  0x00,  0x00,  0xf0,  0x00,  0x00,
		0x61,  0x03,  0x00,  0x00,  0x00,  0xf0,  0x00,  0x00,  0x61,  0x03,  0x00,  0x00,
		0x00,  0xf0,  0x00,  0x03df,  0x03db,  0x03d7,  0x0422,  0x0427,  0x042b
	},
	{
		13,   2472,  0x70,  0x16,  0x30,  0x1b,  0x0a,  0x0a,  0x30,  0xa8,  0x09,  0x0f,
		0x05,  0x00,  0x05,  0x00,  0x61,  0x03,  0x00,  0x00,  0x00,  0xf0,  0x00,  0x00,
		0x61,  0x03,  0x00,  0x00,  0x00,  0xf0,  0x00,  0x00,  0x61,  0x03,  0x00,  0x00,
		0x00,  0xf0,  0x00,  0x03e1,  0x03dd,  0x03d9,  0x0420,  0x0424,  0x0429
	},
	{
		14,   2484,  0x78,  0x16,  0x30,  0x1b,  0x0a,  0x0a,  0x30,  0xb4,  0x09,  0x0f,
		0x04,  0x00,  0x04,  0x00,  0x61,  0x03,  0x00,  0x00,  0x00,  0xe0,  0x00,  0x00,
		0x61,  0x03,  0x00,  0x00,  0x00,  0xe0,  0x00,  0x00,  0x61,  0x03,  0x00,  0x00,
		0x00,  0xe0,  0x00,  0x03e6,  0x03e2,  0x03de,  0x041b,  0x041f,  0x0424
	}
};

/* channel info table for htphyrev1 (autogenerated by gen_tune_2059.tcl) */
static chan_info_htphy_radio2059_t chan_info_htphyrev1_2059rev0v1[] = {
	{
		184,   4920,  0x68,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0xec,  0x01,  0x0f,
		0x00,  0x0f,  0x00,  0x0f,  0x00,  0xd3,  0x0f,  0x4f,  0xd3,  0x00,  0xff,  0x0f,
		0x00,  0x00,  0x0f,  0x4f,  0xd3,  0x00,  0xff,  0x0f,  0x00,  0x00,  0x0f,  0x4f,
		0xd3,  0x00,  0xff,  0x07b4,  0x07b0,  0x07ac,  0x0214,  0x0215,  0x0216
	},
	{
		186,   4930,  0x6b,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0xed,  0x01,  0x0f,
		0x00,  0x0f,  0x00,  0x0f,  0x00,  0xd3,  0x0f,  0x4f,  0xd3,  0x00,  0xff,  0x0f,
		0x00,  0x00,  0x0f,  0x4f,  0xd3,  0x00,  0xff,  0x0f,  0x00,  0x00,  0x0f,  0x4f,
		0xd3,  0x00,  0xff,  0x07b8,  0x07b4,  0x07b0,  0x0213,  0x0214,  0x0215
	},
	{
		188,   4940,  0x6e,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0xee,  0x01,  0x0f,
		0x00,  0x0f,  0x00,  0x0f,  0x00,  0xd3,  0x0f,  0x4f,  0xd3,  0x00,  0xff,  0x0f,
		0x00,  0x00,  0x0f,  0x4f,  0xd3,  0x00,  0xff,  0x0f,  0x00,  0x00,  0x0f,  0x4f,
		0xd3,  0x00,  0xff,  0x07bc,  0x07b8,  0x07b4,  0x0212,  0x0213,  0x0214
	},
	{
		190,   4950,  0x72,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0xef,  0x01,  0x0f,
		0x00,  0x0f,  0x00,  0x0f,  0x00,  0x00,  0x0f,  0x4f,  0xd3,  0x00,  0xff,  0x0f,
		0x00,  0x00,  0x0f,  0x4f,  0xd3,  0x00,  0xff,  0x0f,  0x00,  0x00,  0x0f,  0x4f,
		0xd3,  0x00,  0xff,  0x07c0,  0x07bc,  0x07b8,  0x0211,  0x0212,  0x0213
	},
	{
		192,   4960,  0x75,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0xf0,  0x01,  0x0f,
		0x00,  0x0f,  0x00,  0x0f,  0x00,  0x00,  0x0f,  0x4f,  0xd3,  0x00,  0xff,  0x0f,
		0x00,  0x00,  0x0f,  0x4f,  0xd3,  0x00,  0xff,  0x0f,  0x00,  0x00,  0x0f,  0x4f,
		0xd3,  0x00,  0xff,  0x07c4,  0x07c0,  0x07bc,  0x020f,  0x0211,  0x0212
	},
	{
		194,   4970,  0x78,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0xf1,  0x01,  0x0f,
		0x00,  0x0f,  0x00,  0x0f,  0x00,  0x00,  0x0f,  0x4f,  0xd3,  0x00,  0xff,  0x0f,
		0x00,  0x00,  0x0f,  0x4f,  0xd3,  0x00,  0xff,  0x0f,  0x00,  0x00,  0x0f,  0x4f,
		0xd3,  0x00,  0xff,  0x07c8,  0x07c4,  0x07c0,  0x020e,  0x020f,  0x0211
	},
	{
		196,   4980,  0x7c,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0xf2,  0x01,  0x0f,
		0x00,  0x0f,  0x00,  0x0f,  0x00,  0x00,  0x0f,  0x4f,  0xd3,  0x00,  0xff,  0x0f,
		0x00,  0x00,  0x0f,  0x4f,  0xd3,  0x00,  0xff,  0x0f,  0x00,  0x00,  0x0f,  0x4f,
		0xd3,  0x00,  0xff,  0x07cc,  0x07c8,  0x07c4,  0x020d,  0x020e,  0x020f
	},
	{
		198,   4990,  0x7f,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0xf3,  0x01,  0x0f,
		0x00,  0x0f,  0x00,  0x0f,  0x00,  0x00,  0x0f,  0x4f,  0xd3,  0x00,  0xff,  0x0f,
		0x00,  0x00,  0x0f,  0x4f,  0xd3,  0x00,  0xff,  0x0f,  0x00,  0x00,  0x0f,  0x4f,
		0xd3,  0x00,  0xff,  0x07d0,  0x07cc,  0x07c8,  0x020c,  0x020d,  0x020e
	},
	{
		200,   5000,  0x82,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0xf4,  0x01,  0x0f,
		0x00,  0x0f,  0x00,  0x0f,  0x00,  0x00,  0x0f,  0x4f,  0xb3,  0x00,  0xff,  0x0f,
		0x00,  0x00,  0x0f,  0x4f,  0xb3,  0x00,  0xff,  0x0f,  0x00,  0x00,  0x0f,  0x4f,
		0xb3,  0x00,  0xff,  0x07d4,  0x07d0,  0x07cc,  0x020b,  0x020c,  0x020d
	},
	{
		202,   5010,  0x86,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0xf5,  0x01,  0x0f,
		0x00,  0x0f,  0x00,  0x0f,  0x00,  0x00,  0x0f,  0x4f,  0xb3,  0x00,  0xff,  0x0f,
		0x00,  0x00,  0x0f,  0x4f,  0xb3,  0x00,  0xff,  0x0f,  0x00,  0x00,  0x0f,  0x4f,
		0xb3,  0x00,  0xff,  0x07d8,  0x07d4,  0x07d0,  0x020a,  0x020b,  0x020c
	},
	{
		204,   5020,  0x89,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0xf6,  0x01,  0x0e,
		0x00,  0x0e,  0x00,  0x0e,  0x00,  0x00,  0x0f,  0x4f,  0xb3,  0x00,  0xff,  0x0e,
		0x00,  0x00,  0x0f,  0x4f,  0xb3,  0x00,  0xff,  0x0e,  0x00,  0x00,  0x0f,  0x4f,
		0xb3,  0x00,  0xff,  0x07dc,  0x07d8,  0x07d4,  0x0209,  0x020a,  0x020b
	},
	{
		206,   5030,  0x8c,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0xf7,  0x01,  0x0e,
		0x00,  0x0e,  0x00,  0x0e,  0x00,  0x00,  0x0f,  0x4f,  0xb3,  0x00,  0xff,  0x0e,
		0x00,  0x00,  0x0f,  0x4f,  0xb3,  0x00,  0xff,  0x0e,  0x00,  0x00,  0x0f,  0x4f,
		0xb3,  0x00,  0xff,  0x07e0,  0x07dc,  0x07d8,  0x0208,  0x0209,  0x020a
	},
	{
		208,   5040,  0x90,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0xf8,  0x01,  0x0e,
		0x00,  0x0e,  0x00,  0x0e,  0x00,  0x00,  0x0f,  0x4f,  0xb3,  0x00,  0xff,  0x0e,
		0x00,  0x00,  0x0f,  0x4f,  0xb3,  0x00,  0xff,  0x0e,  0x00,  0x00,  0x0f,  0x4f,
		0xb3,  0x00,  0xff,  0x07e4,  0x07e0,  0x07dc,  0x0207,  0x0208,  0x0209
	},
	{
		210,   5050,  0x93,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0xf9,  0x01,  0x0e,
		0x00,  0x0e,  0x00,  0x0e,  0x00,  0x00,  0x0f,  0x4f,  0xb3,  0x00,  0xff,  0x0e,
		0x00,  0x00,  0x0f,  0x4f,  0xb3,  0x00,  0xff,  0x0e,  0x00,  0x00,  0x0f,  0x4f,
		0xb3,  0x00,  0xff,  0x07e8,  0x07e4,  0x07e0,  0x0206,  0x0207,  0x0208
	},
	{
		212,   5060,  0x96,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0xfa,  0x01,  0x0e,
		0x00,  0x0e,  0x00,  0x0e,  0x00,  0x00,  0x0f,  0x4f,  0xb3,  0x00,  0xff,  0x0e,
		0x00,  0x00,  0x0f,  0x4f,  0xb3,  0x00,  0xff,  0x0e,  0x00,  0x00,  0x0f,  0x4f,
		0xb3,  0x00,  0xff,  0x07ec,  0x07e8,  0x07e4,  0x0205,  0x0206,  0x0207
	},
	{
		214,   5070,  0x9a,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0xfb,  0x01,  0x0e,
		0x00,  0x0e,  0x00,  0x0e,  0x00,  0x00,  0x0f,  0x4f,  0xb3,  0x00,  0xff,  0x0e,
		0x00,  0x00,  0x0f,  0x4f,  0xb3,  0x00,  0xff,  0x0e,  0x00,  0x00,  0x0f,  0x4f,
		0xb3,  0x00,  0xff,  0x07f0,  0x07ec,  0x07e8,  0x0204,  0x0205,  0x0206
	},
	{
		216,   5080,  0x9d,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0xfc,  0x01,  0x0e,
		0x00,  0x0e,  0x00,  0x0e,  0x00,  0x00,  0x0f,  0x4f,  0xb3,  0x00,  0xff,  0x0e,
		0x00,  0x00,  0x0f,  0x4f,  0xb3,  0x00,  0xff,  0x0e,  0x00,  0x00,  0x0f,  0x4f,
		0xb3,  0x00,  0xff,  0x07f4,  0x07f0,  0x07ec,  0x0203,  0x0204,  0x0205
	},
	{
		218,   5090,  0xa0,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0xfd,  0x01,  0x0e,
		0x00,  0x0e,  0x00,  0x0e,  0x00,  0x00,  0x0f,  0x4f,  0xb3,  0x00,  0xff,  0x0e,
		0x00,  0x00,  0x0f,  0x4f,  0xb3,  0x00,  0xff,  0x0e,  0x00,  0x00,  0x0f,  0x4f,
		0xb3,  0x00,  0xff,  0x07f8,  0x07f4,  0x07f0,  0x0202,  0x0203,  0x0204
	},
	{
		220,   5100,  0xa4,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0xfe,  0x01,  0x0d,
		0x00,  0x0d,  0x00,  0x0d,  0x00,  0x00,  0x0f,  0x4f,  0xa3,  0x00,  0xff,  0x0d,
		0x00,  0x00,  0x0f,  0x4f,  0xa3,  0x00,  0xff,  0x0d,  0x00,  0x00,  0x0f,  0x4f,
		0xa3,  0x00,  0xff,  0x07fc,  0x07f8,  0x07f4,  0x0201,  0x0202,  0x0203
	},
	{
		222,   5110,  0xa7,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0xff,  0x01,  0x0d,
		0x00,  0x0d,  0x00,  0x0d,  0x00,  0x00,  0x0f,  0x4f,  0xa3,  0x00,  0xff,  0x0d,
		0x00,  0x00,  0x0f,  0x4f,  0xa3,  0x00,  0xff,  0x0d,  0x00,  0x00,  0x0f,  0x4f,
		0xa3,  0x00,  0xff,  0x0800,  0x07fc,  0x07f8,  0x0200,  0x0201,  0x0202
	},
	{
		224,   5120,  0xaa,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x00,  0x02,  0x0d,
		0x00,  0x0d,  0x00,  0x0d,  0x00,  0x00,  0x0f,  0x4f,  0xa3,  0x00,  0xff,  0x0d,
		0x00,  0x00,  0x0f,  0x4f,  0xa3,  0x00,  0xff,  0x0d,  0x00,  0x00,  0x0f,  0x4f,
		0xa3,  0x00,  0xff,  0x0804,  0x0800,  0x07fc,  0x01ff,  0x0200,  0x0201
	},
	{
		226,   5130,  0xae,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x01,  0x02,  0x0d,
		0x00,  0x0d,  0x00,  0x0d,  0x00,  0x00,  0x0f,  0x4f,  0xa3,  0x00,  0xfc,  0x0d,
		0x00,  0x00,  0x0f,  0x4f,  0xa3,  0x00,  0xfc,  0x0d,  0x00,  0x00,  0x0f,  0x4f,
		0xa3,  0x00,  0xfc,  0x0808,  0x0804,  0x0800,  0x01fe,  0x01ff,  0x0200
	},
	{
		228,   5140,  0xb1,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x02,  0x02,  0x0d,
		0x00,  0x0d,  0x00,  0x0d,  0x00,  0x00,  0x0f,  0x4f,  0xa3,  0x00,  0xfc,  0x0d,
		0x00,  0x00,  0x0f,  0x4f,  0xa3,  0x00,  0xfc,  0x0d,  0x00,  0x00,  0x0f,  0x4f,
		0xa3,  0x00,  0xfc,  0x080c,  0x0808,  0x0804,  0x01fd,  0x01fe,  0x01ff
	},
	{
		32,   5160,  0xb8,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x04,  0x02,  0x0d,
		0x00,  0x0d,  0x00,  0x0d,  0x00,  0x00,  0x0f,  0x4f,  0xa3,  0x00,  0xfc,  0x0d,
		0x00,  0x00,  0x0f,  0x4f,  0xa3,  0x00,  0xfc,  0x0d,  0x00,  0x00,  0x0f,  0x4f,
		0xa3,  0x00,  0xfc,  0x0814,  0x0810,  0x080c,  0x01fb,  0x01fc,  0x01fd
	},
	{
		34,   5170,  0xbb,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x05,  0x02,  0x0d,
		0x00,  0x0d,  0x00,  0x0d,  0x00,  0x00,  0x0f,  0x4f,  0xa3,  0x00,  0xfc,  0x0d,
		0x00,  0x00,  0x0f,  0x4f,  0xa3,  0x00,  0xfc,  0x0d,  0x00,  0x00,  0x0f,  0x4f,
		0xa3,  0x00,  0xfc,  0x0818,  0x0814,  0x0810,  0x01fa,  0x01fb,  0x01fc
	},
	{
		36,   5180,  0xbe,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x06,  0x02,  0x0c,
		0x00,  0x0c,  0x00,  0x0c,  0x00,  0x00,  0x0f,  0x4f,  0xa3,  0x00,  0xfc,  0x0c,
		0x00,  0x00,  0x0f,  0x4f,  0xa3,  0x00,  0xfc,  0x0c,  0x00,  0x00,  0x0f,  0x4f,
		0xa3,  0x00,  0xfc,  0x081c,  0x0818,  0x0814,  0x01f9,  0x01fa,  0x01fb
	},
	{
		38,   5190,  0xc2,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x07,  0x02,  0x0c,
		0x00,  0x0c,  0x00,  0x0c,  0x00,  0x00,  0x0f,  0x4f,  0xa3,  0x00,  0xfb,  0x0c,
		0x00,  0x00,  0x0f,  0x4f,  0xa3,  0x00,  0xfb,  0x0c,  0x00,  0x00,  0x0f,  0x4f,
		0xa3,  0x00,  0xfb,  0x0820,  0x081c,  0x0818,  0x01f8,  0x01f9,  0x01fa
	},
	{
		40,   5200,  0xc5,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x08,  0x02,  0x0c,
		0x00,  0x0c,  0x00,  0x0c,  0x00,  0x00,  0x0f,  0x4f,  0x93,  0x00,  0xfb,  0x0c,
		0x00,  0x00,  0x0f,  0x4f,  0x93,  0x00,  0xfb,  0x0c,  0x00,  0x00,  0x0f,  0x4f,
		0x93,  0x00,  0xfb,  0x0824,  0x0820,  0x081c,  0x01f7,  0x01f8,  0x01f9
	},
	{
		42,   5210,  0xc8,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x09,  0x02,  0x0c,
		0x00,  0x0c,  0x00,  0x0c,  0x00,  0x00,  0x0f,  0x4f,  0x93,  0x00,  0xea,  0x0c,
		0x00,  0x00,  0x0f,  0x4f,  0x93,  0x00,  0xea,  0x0c,  0x00,  0x00,  0x0f,  0x4f,
		0x93,  0x00,  0xea,  0x0828,  0x0824,  0x0820,  0x01f6,  0x01f7,  0x01f8
	},
	{
		44,   5220,  0xcc,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x0a,  0x02,  0x0c,
		0x00,  0x0c,  0x00,  0x0c,  0x00,  0x00,  0x0f,  0x4f,  0x93,  0x00,  0xea,  0x0c,
		0x00,  0x00,  0x0f,  0x4f,  0x93,  0x00,  0xea,  0x0c,  0x00,  0x00,  0x0f,  0x4f,
		0x93,  0x00,  0xea,  0x082c,  0x0828,  0x0824,  0x01f5,  0x01f6,  0x01f7
	},
	{
		46,   5230,  0xcf,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x0b,  0x02,  0x0c,
		0x00,  0x0c,  0x00,  0x0c,  0x00,  0x00,  0x0f,  0x4f,  0x93,  0x00,  0xda,  0x0c,
		0x00,  0x00,  0x0f,  0x4f,  0x93,  0x00,  0xda,  0x0c,  0x00,  0x00,  0x0f,  0x4f,
		0x93,  0x00,  0xda,  0x0830,  0x082c,  0x0828,  0x01f4,  0x01f5,  0x01f6
	},
	{
		48,   5240,  0xd2,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x0c,  0x02,  0x0c,
		0x00,  0x0c,  0x00,  0x0c,  0x00,  0x00,  0x0f,  0x4f,  0x93,  0x00,  0xda,  0x0c,
		0x00,  0x00,  0x0f,  0x4f,  0x93,  0x00,  0xda,  0x0c,  0x00,  0x00,  0x0f,  0x4f,
		0x93,  0x00,  0xda,  0x0834,  0x0830,  0x082c,  0x01f3,  0x01f4,  0x01f5
	},
	{
		50,   5250,  0xd6,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x0d,  0x02,  0x0c,
		0x00,  0x0c,  0x00,  0x0c,  0x00,  0x00,  0x0f,  0x4f,  0x93,  0x00,  0xca,  0x0c,
		0x00,  0x00,  0x0f,  0x4f,  0x93,  0x00,  0xca,  0x0c,  0x00,  0x00,  0x0f,  0x4f,
		0x93,  0x00,  0xca,  0x0838,  0x0834,  0x0830,  0x01f2,  0x01f3,  0x01f4
	},
	{
		52,   5260,  0xd9,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x0e,  0x02,  0x0b,
		0x00,  0x0b,  0x00,  0x0b,  0x00,  0x00,  0x0f,  0x4f,  0x93,  0x00,  0xca,  0x0b,
		0x00,  0x00,  0x0f,  0x4f,  0x93,  0x00,  0xca,  0x0b,  0x00,  0x00,  0x0f,  0x4f,
		0x93,  0x00,  0xca,  0x083c,  0x0838,  0x0834,  0x01f1,  0x01f2,  0x01f3
	},
	{
		54,   5270,  0xdc,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x0f,  0x02,  0x0b,
		0x00,  0x0b,  0x00,  0x0b,  0x00,  0x00,  0x0f,  0x4f,  0x93,  0x00,  0xb9,  0x0b,
		0x00,  0x00,  0x0f,  0x4f,  0x93,  0x00,  0xb9,  0x0b,  0x00,  0x00,  0x0f,  0x4f,
		0x93,  0x00,  0xb9,  0x0840,  0x083c,  0x0838,  0x01f0,  0x01f1,  0x01f2
	},
	{
		56,   5280,  0xe0,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x10,  0x02,  0x0b,
		0x00,  0x0b,  0x00,  0x0b,  0x00,  0x00,  0x0f,  0x4f,  0x93,  0x00,  0xb9,  0x0b,
		0x00,  0x00,  0x0f,  0x4f,  0x93,  0x00,  0xb9,  0x0b,  0x00,  0x00,  0x0f,  0x4f,
		0x93,  0x00,  0xb9,  0x0844,  0x0840,  0x083c,  0x01f0,  0x01f0,  0x01f1
	},
	{
		58,   5290,  0xe3,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x11,  0x02,  0x0b,
		0x00,  0x0b,  0x00,  0x0b,  0x00,  0x00,  0x0f,  0x4f,  0x93,  0x00,  0xb8,  0x0b,
		0x00,  0x00,  0x0f,  0x4f,  0x93,  0x00,  0xb8,  0x0b,  0x00,  0x00,  0x0f,  0x4f,
		0x93,  0x00,  0xb8,  0x0848,  0x0844,  0x0840,  0x01ef,  0x01f0,  0x01f0
	},
	{
		60,   5300,  0xe6,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x12,  0x02,  0x0b,
		0x00,  0x0b,  0x00,  0x0b,  0x00,  0x00,  0x0f,  0x4c,  0x83,  0x00,  0xb8,  0x0b,
		0x00,  0x00,  0x0f,  0x4c,  0x83,  0x00,  0xb8,  0x0b,  0x00,  0x00,  0x0f,  0x4c,
		0x83,  0x00,  0xb8,  0x084c,  0x0848,  0x0844,  0x01ee,  0x01ef,  0x01f0
	},
	{
		62,   5310,  0xea,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x13,  0x02,  0x0b,
		0x00,  0x0b,  0x00,  0x0b,  0x00,  0x00,  0x0f,  0x4c,  0x83,  0x00,  0xa8,  0x0b,
		0x00,  0x00,  0x0f,  0x4c,  0x83,  0x00,  0xa8,  0x0b,  0x00,  0x00,  0x0f,  0x4c,
		0x83,  0x00,  0xa8,  0x0850,  0x084c,  0x0848,  0x01ed,  0x01ee,  0x01ef
	},
	{
		64,   5320,  0xed,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x14,  0x02,  0x0b,
		0x00,  0x0b,  0x00,  0x0b,  0x00,  0x00,  0x0f,  0x4c,  0x83,  0x00,  0xa8,  0x0b,
		0x00,  0x00,  0x0f,  0x4c,  0x83,  0x00,  0xa8,  0x0b,  0x00,  0x00,  0x0f,  0x4c,
		0x83,  0x00,  0xa8,  0x0854,  0x0850,  0x084c,  0x01ec,  0x01ed,  0x01ee
	},
	{
		66,   5330,  0xf0,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x15,  0x02,  0x0b,
		0x00,  0x0b,  0x00,  0x0b,  0x00,  0x00,  0x0f,  0x4c,  0x83,  0x00,  0xa8,  0x0b,
		0x00,  0x00,  0x0f,  0x4c,  0x83,  0x00,  0xa8,  0x0b,  0x00,  0x00,  0x0f,  0x4c,
		0x83,  0x00,  0xa8,  0x0858,  0x0854,  0x0850,  0x01eb,  0x01ec,  0x01ed
	},
	{
		68,   5340,  0xf4,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x16,  0x02,  0x0a,
		0x00,  0x0a,  0x00,  0x0a,  0x00,  0x00,  0x0f,  0x4c,  0x83,  0x00,  0x87,  0x0a,
		0x00,  0x00,  0x0f,  0x4c,  0x83,  0x00,  0x87,  0x0a,  0x00,  0x00,  0x0f,  0x4c,
		0x83,  0x00,  0x87,  0x085c,  0x0858,  0x0854,  0x01ea,  0x01eb,  0x01ec
	},
	{
		70,   5350,  0xf7,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x17,  0x02,  0x0a,
		0x00,  0x0a,  0x00,  0x0a,  0x00,  0x00,  0x0f,  0x4c,  0x83,  0x00,  0x87,  0x0a,
		0x00,  0x00,  0x0f,  0x4c,  0x83,  0x00,  0x87,  0x0a,  0x00,  0x00,  0x0f,  0x4c,
		0x83,  0x00,  0x87,  0x0860,  0x085c,  0x0858,  0x01e9,  0x01ea,  0x01eb
	},
	{
		72,   5360,  0xfa,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x18,  0x02,  0x0a,
		0x00,  0x0a,  0x00,  0x0a,  0x00,  0x00,  0x0f,  0x4c,  0x83,  0x00,  0x87,  0x0a,
		0x00,  0x00,  0x0f,  0x4c,  0x83,  0x00,  0x87,  0x0a,  0x00,  0x00,  0x0f,  0x4c,
		0x83,  0x00,  0x87,  0x0864,  0x0860,  0x085c,  0x01e8,  0x01e9,  0x01ea
	},
	{
		74,   5370,  0xfe,  0x16,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x19,  0x02,  0x0a,
		0x00,  0x0a,  0x00,  0x0a,  0x00,  0x00,  0x0f,  0x4c,  0x83,  0x00,  0x87,  0x0a,
		0x00,  0x00,  0x0f,  0x4c,  0x83,  0x00,  0x87,  0x0a,  0x00,  0x00,  0x0f,  0x4c,
		0x83,  0x00,  0x87,  0x0868,  0x0864,  0x0860,  0x01e7,  0x01e8,  0x01e9
	},
	{
		76,   5380,  0x01,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x1a,  0x02,  0x0a,
		0x00,  0x0a,  0x00,  0x0a,  0x00,  0x00,  0x0f,  0x4c,  0x83,  0x00,  0x87,  0x0a,
		0x00,  0x00,  0x0f,  0x4c,  0x83,  0x00,  0x87,  0x0a,  0x00,  0x00,  0x0f,  0x4c,
		0x83,  0x00,  0x87,  0x086c,  0x0868,  0x0864,  0x01e6,  0x01e7,  0x01e8
	},
	{
		78,   5390,  0x04,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x1b,  0x02,  0x0a,
		0x00,  0x0a,  0x00,  0x0a,  0x00,  0x00,  0x0f,  0x4c,  0x83,  0x00,  0x87,  0x0a,
		0x00,  0x00,  0x0f,  0x4c,  0x83,  0x00,  0x87,  0x0a,  0x00,  0x00,  0x0f,  0x4c,
		0x83,  0x00,  0x87,  0x0870,  0x086c,  0x0868,  0x01e5,  0x01e6,  0x01e7
	},
	{
		80,   5400,  0x08,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x1c,  0x02,  0x0a,
		0x00,  0x0a,  0x00,  0x0a,  0x00,  0x00,  0x0d,  0x49,  0x53,  0x00,  0x87,  0x0a,
		0x00,  0x00,  0x0d,  0x49,  0x53,  0x00,  0x87,  0x0a,  0x00,  0x00,  0x0d,  0x49,
		0x53,  0x00,  0x87,  0x0874,  0x0870,  0x086c,  0x01e5,  0x01e5,  0x01e6
	},
	{
		82,   5410,  0x0b,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x1d,  0x02,  0x0a,
		0x00,  0x0a,  0x00,  0x0a,  0x00,  0x00,  0x0d,  0x49,  0x53,  0x00,  0x86,  0x0a,
		0x00,  0x00,  0x0d,  0x49,  0x53,  0x00,  0x86,  0x0a,  0x00,  0x00,  0x0d,  0x49,
		0x53,  0x00,  0x86,  0x0878,  0x0874,  0x0870,  0x01e4,  0x01e5,  0x01e5
	},
	{
		84,   5420,  0x0e,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x1e,  0x02,  0x09,
		0x00,  0x09,  0x00,  0x09,  0x00,  0x00,  0x0d,  0x49,  0x53,  0x00,  0x86,  0x09,
		0x00,  0x00,  0x0d,  0x49,  0x53,  0x00,  0x86,  0x09,  0x00,  0x00,  0x0d,  0x49,
		0x53,  0x00,  0x86,  0x087c,  0x0878,  0x0874,  0x01e3,  0x01e4,  0x01e5
	},
	{
		86,   5430,  0x12,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x1f,  0x02,  0x09,
		0x00,  0x09,  0x00,  0x09,  0x00,  0x00,  0x0d,  0x49,  0x53,  0x00,  0x86,  0x09,
		0x00,  0x00,  0x0d,  0x49,  0x53,  0x00,  0x86,  0x09,  0x00,  0x00,  0x0d,  0x49,
		0x53,  0x00,  0x86,  0x0880,  0x087c,  0x0878,  0x01e2,  0x01e3,  0x01e4
	},
	{
		88,   5440,  0x15,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x20,  0x02,  0x09,
		0x00,  0x09,  0x00,  0x09,  0x00,  0x00,  0x0d,  0x49,  0x53,  0x00,  0x86,  0x09,
		0x00,  0x00,  0x0d,  0x49,  0x53,  0x00,  0x86,  0x09,  0x00,  0x00,  0x0d,  0x49,
		0x53,  0x00,  0x86,  0x0884,  0x0880,  0x087c,  0x01e1,  0x01e2,  0x01e3
	},
	{
		90,   5450,  0x18,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x21,  0x02,  0x09,
		0x00,  0x09,  0x00,  0x09,  0x00,  0x00,  0x0d,  0x49,  0x53,  0x00,  0x86,  0x09,
		0x00,  0x00,  0x0d,  0x49,  0x53,  0x00,  0x86,  0x09,  0x00,  0x00,  0x0d,  0x49,
		0x53,  0x00,  0x86,  0x0888,  0x0884,  0x0880,  0x01e0,  0x01e1,  0x01e2
	},
	{
		92,   5460,  0x1c,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x22,  0x02,  0x09,
		0x00,  0x09,  0x00,  0x09,  0x00,  0x00,  0x0d,  0x49,  0x53,  0x00,  0x75,  0x09,
		0x00,  0x00,  0x0d,  0x49,  0x53,  0x00,  0x75,  0x09,  0x00,  0x00,  0x0d,  0x49,
		0x53,  0x00,  0x75,  0x088c,  0x0888,  0x0884,  0x01df,  0x01e0,  0x01e1
	},
	{
		94,   5470,  0x1f,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x23,  0x02,  0x09,
		0x00,  0x09,  0x00,  0x09,  0x00,  0x00,  0x0d,  0x49,  0x53,  0x00,  0x75,  0x09,
		0x00,  0x00,  0x0d,  0x49,  0x53,  0x00,  0x75,  0x09,  0x00,  0x00,  0x0d,  0x49,
		0x53,  0x00,  0x75,  0x0890,  0x088c,  0x0888,  0x01de,  0x01df,  0x01e0
	},
	{
		96,   5480,  0x22,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x24,  0x02,  0x09,
		0x00,  0x09,  0x00,  0x09,  0x00,  0x00,  0x0d,  0x49,  0x53,  0x00,  0x75,  0x09,
		0x00,  0x00,  0x0d,  0x49,  0x53,  0x00,  0x75,  0x09,  0x00,  0x00,  0x0d,  0x49,
		0x53,  0x00,  0x75,  0x0894,  0x0890,  0x088c,  0x01dd,  0x01de,  0x01df
	},
	{
		98,   5490,  0x26,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x25,  0x02,  0x09,
		0x00,  0x09,  0x00,  0x09,  0x00,  0x00,  0x0d,  0x49,  0x53,  0x00,  0x75,  0x09,
		0x00,  0x00,  0x0d,  0x49,  0x53,  0x00,  0x75,  0x09,  0x00,  0x00,  0x0d,  0x49,
		0x53,  0x00,  0x75,  0x0898,  0x0894,  0x0890,  0x01dd,  0x01dd,  0x01de
	},
	{
		100,   5500,  0x29,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x26,  0x02,  0x09,
		0x00,  0x09,  0x00,  0x09,  0x00,  0x00,  0x0a,  0x46,  0x43,  0x00,  0x75,  0x09,
		0x00,  0x00,  0x0a,  0x46,  0x43,  0x00,  0x75,  0x09,  0x00,  0x00,  0x0a,  0x46,
		0x43,  0x00,  0x75,  0x089c,  0x0898,  0x0894,  0x01dc,  0x01dd,  0x01dd
	},
	{
		102,   5510,  0x2c,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x27,  0x02,  0x09,
		0x00,  0x09,  0x00,  0x09,  0x00,  0x00,  0x0a,  0x46,  0x43,  0x00,  0x75,  0x09,
		0x00,  0x00,  0x0a,  0x46,  0x43,  0x00,  0x75,  0x09,  0x00,  0x00,  0x0a,  0x46,
		0x43,  0x00,  0x75,  0x08a0,  0x089c,  0x0898,  0x01db,  0x01dc,  0x01dd
	},
	{
		104,   5520,  0x30,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x28,  0x02,  0x08,
		0x00,  0x08,  0x00,  0x08,  0x00,  0x00,  0x0a,  0x46,  0x43,  0x00,  0x75,  0x08,
		0x00,  0x00,  0x0a,  0x46,  0x43,  0x00,  0x75,  0x08,  0x00,  0x00,  0x0a,  0x46,
		0x43,  0x00,  0x75,  0x08a4,  0x08a0,  0x089c,  0x01da,  0x01db,  0x01dc
	},
	{
		106,   5530,  0x33,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x29,  0x02,  0x08,
		0x00,  0x08,  0x00,  0x08,  0x00,  0x00,  0x0a,  0x46,  0x43,  0x00,  0x75,  0x08,
		0x00,  0x00,  0x0a,  0x46,  0x43,  0x00,  0x75,  0x08,  0x00,  0x00,  0x0a,  0x46,
		0x43,  0x00,  0x75,  0x08a8,  0x08a4,  0x08a0,  0x01d9,  0x01da,  0x01db
	},
	{
		108,   5540,  0x36,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x2a,  0x02,  0x08,
		0x00,  0x08,  0x00,  0x08,  0x00,  0x00,  0x0a,  0x46,  0x43,  0x00,  0x75,  0x08,
		0x00,  0x00,  0x0a,  0x46,  0x43,  0x00,  0x75,  0x08,  0x00,  0x00,  0x0a,  0x46,
		0x43,  0x00,  0x75,  0x08ac,  0x08a8,  0x08a4,  0x01d8,  0x01d9,  0x01da
	},
	{
		110,   5550,  0x3a,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x2b,  0x02,  0x08,
		0x00,  0x08,  0x00,  0x08,  0x00,  0x00,  0x0a,  0x46,  0x43,  0x00,  0x75,  0x08,
		0x00,  0x00,  0x0a,  0x46,  0x43,  0x00,  0x75,  0x08,  0x00,  0x00,  0x0a,  0x46,
		0x43,  0x00,  0x75,  0x08b0,  0x08ac,  0x08a8,  0x01d7,  0x01d8,  0x01d9
	},
	{
		112,   5560,  0x3d,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x2c,  0x02,  0x08,
		0x00,  0x08,  0x00,  0x08,  0x00,  0x00,  0x0a,  0x46,  0x43,  0x00,  0x75,  0x08,
		0x00,  0x00,  0x0a,  0x46,  0x43,  0x00,  0x75,  0x08,  0x00,  0x00,  0x0a,  0x46,
		0x43,  0x00,  0x75,  0x08b4,  0x08b0,  0x08ac,  0x01d7,  0x01d7,  0x01d8
	},
	{
		114,   5570,  0x40,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x2d,  0x02,  0x08,
		0x00,  0x08,  0x00,  0x08,  0x00,  0x00,  0x0a,  0x46,  0x43,  0x00,  0x74,  0x08,
		0x00,  0x00,  0x0a,  0x46,  0x43,  0x00,  0x74,  0x08,  0x00,  0x00,  0x0a,  0x46,
		0x43,  0x00,  0x74,  0x08b8,  0x08b4,  0x08b0,  0x01d6,  0x01d7,  0x01d7
	},
	{
		116,   5580,  0x44,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x2e,  0x02,  0x08,
		0x00,  0x08,  0x00,  0x08,  0x00,  0x00,  0x0a,  0x46,  0x43,  0x00,  0x74,  0x08,
		0x00,  0x00,  0x0a,  0x46,  0x43,  0x00,  0x74,  0x08,  0x00,  0x00,  0x0a,  0x46,
		0x43,  0x00,  0x74,  0x08bc,  0x08b8,  0x08b4,  0x01d5,  0x01d6,  0x01d7
	},
	{
		118,   5590,  0x47,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x2f,  0x02,  0x08,
		0x00,  0x08,  0x00,  0x08,  0x00,  0x00,  0x0a,  0x46,  0x43,  0x00,  0x54,  0x08,
		0x00,  0x00,  0x0a,  0x46,  0x43,  0x00,  0x54,  0x08,  0x00,  0x00,  0x0a,  0x46,
		0x43,  0x00,  0x54,  0x08c0,  0x08bc,  0x08b8,  0x01d4,  0x01d5,  0x01d6
	},
	{
		120,   5600,  0x4a,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x30,  0x02,  0x08,
		0x00,  0x08,  0x00,  0x08,  0x00,  0x00,  0x09,  0x44,  0x23,  0x00,  0x54,  0x08,
		0x00,  0x00,  0x09,  0x44,  0x23,  0x00,  0x54,  0x08,  0x00,  0x00,  0x09,  0x44,
		0x23,  0x00,  0x54,  0x08c4,  0x08c0,  0x08bc,  0x01d3,  0x01d4,  0x01d5
	},
	{
		122,   5610,  0x4e,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x31,  0x02,  0x08,
		0x00,  0x08,  0x00,  0x08,  0x00,  0x00,  0x09,  0x44,  0x23,  0x00,  0x54,  0x08,
		0x00,  0x00,  0x09,  0x44,  0x23,  0x00,  0x54,  0x08,  0x00,  0x00,  0x09,  0x44,
		0x23,  0x00,  0x54,  0x08c8,  0x08c4,  0x08c0,  0x01d2,  0x01d3,  0x01d4
	},
	{
		124,   5620,  0x51,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x32,  0x02,  0x07,
		0x00,  0x07,  0x00,  0x07,  0x00,  0x00,  0x09,  0x44,  0x23,  0x00,  0x54,  0x07,
		0x00,  0x00,  0x09,  0x44,  0x23,  0x00,  0x54,  0x07,  0x00,  0x00,  0x09,  0x44,
		0x23,  0x00,  0x54,  0x08cc,  0x08c8,  0x08c4,  0x01d2,  0x01d2,  0x01d3
	},
	{
		126,   5630,  0x54,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x33,  0x02,  0x07,
		0x00,  0x07,  0x00,  0x07,  0x00,  0x00,  0x09,  0x44,  0x23,  0x00,  0x43,  0x07,
		0x00,  0x00,  0x09,  0x44,  0x23,  0x00,  0x43,  0x07,  0x00,  0x00,  0x09,  0x44,
		0x23,  0x00,  0x43,  0x08d0,  0x08cc,  0x08c8,  0x01d1,  0x01d2,  0x01d2
	},
	{
		128,   5640,  0x58,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x34,  0x02,  0x07,
		0x00,  0x07,  0x00,  0x07,  0x00,  0x00,  0x09,  0x44,  0x23,  0x00,  0x43,  0x07,
		0x00,  0x00,  0x09,  0x44,  0x23,  0x00,  0x43,  0x07,  0x00,  0x00,  0x09,  0x44,
		0x23,  0x00,  0x43,  0x08d4,  0x08d0,  0x08cc,  0x01d0,  0x01d1,  0x01d2
	},
	{
		130,   5650,  0x5b,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x35,  0x02,  0x07,
		0x00,  0x07,  0x00,  0x07,  0x00,  0x00,  0x09,  0x43,  0x23,  0x00,  0x43,  0x07,
		0x00,  0x00,  0x09,  0x43,  0x23,  0x00,  0x43,  0x07,  0x00,  0x00,  0x09,  0x43,
		0x23,  0x00,  0x43,  0x08d8,  0x08d4,  0x08d0,  0x01cf,  0x01d0,  0x01d1
	},
	{
		132,   5660,  0x5e,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x36,  0x02,  0x07,
		0x00,  0x07,  0x00,  0x07,  0x00,  0x00,  0x09,  0x43,  0x23,  0x00,  0x43,  0x07,
		0x00,  0x00,  0x09,  0x43,  0x23,  0x00,  0x43,  0x07,  0x00,  0x00,  0x09,  0x43,
		0x23,  0x00,  0x43,  0x08dc,  0x08d8,  0x08d4,  0x01ce,  0x01cf,  0x01d0
	},
	{
		134,   5670,  0x62,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x37,  0x02,  0x07,
		0x00,  0x07,  0x00,  0x07,  0x00,  0x00,  0x09,  0x43,  0x23,  0x00,  0x43,  0x07,
		0x00,  0x00,  0x09,  0x43,  0x23,  0x00,  0x43,  0x07,  0x00,  0x00,  0x09,  0x43,
		0x23,  0x00,  0x43,  0x08e0,  0x08dc,  0x08d8,  0x01ce,  0x01ce,  0x01cf
	},
	{
		136,   5680,  0x65,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x38,  0x02,  0x07,
		0x00,  0x07,  0x00,  0x07,  0x00,  0x00,  0x09,  0x42,  0x23,  0x00,  0x43,  0x07,
		0x00,  0x00,  0x09,  0x42,  0x23,  0x00,  0x43,  0x07,  0x00,  0x00,  0x09,  0x42,
		0x23,  0x00,  0x43,  0x08e4,  0x08e0,  0x08dc,  0x01cd,  0x01ce,  0x01ce
	},
	{
		138,   5690,  0x68,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x39,  0x02,  0x07,
		0x00,  0x07,  0x00,  0x07,  0x00,  0x00,  0x09,  0x42,  0x23,  0x00,  0x32,  0x07,
		0x00,  0x00,  0x09,  0x42,  0x23,  0x00,  0x32,  0x07,  0x00,  0x00,  0x09,  0x42,
		0x23,  0x00,  0x32,  0x08e8,  0x08e4,  0x08e0,  0x01cc,  0x01cd,  0x01ce
	},
	{
		140,   5700,  0x6c,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x3a,  0x02,  0x07,
		0x00,  0x07,  0x00,  0x07,  0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x32,  0x07,
		0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x32,  0x07,  0x00,  0x00,  0x08,  0x42,
		0x13,  0x00,  0x32,  0x08ec,  0x08e8,  0x08e4,  0x01cb,  0x01cc,  0x01cd
	},
	{
		142,   5710,  0x6f,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x3b,  0x02,  0x07,
		0x00,  0x07,  0x00,  0x07,  0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x32,  0x07,
		0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x32,  0x07,  0x00,  0x00,  0x08,  0x42,
		0x13,  0x00,  0x32,  0x08f0,  0x08ec,  0x08e8,  0x01ca,  0x01cb,  0x01cc
	},
	{
		144,   5720,  0x72,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x3c,  0x02,  0x07,
		0x00,  0x07,  0x00,  0x07,  0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x32,  0x07,
		0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x32,  0x07,  0x00,  0x00,  0x08,  0x42,
		0x13,  0x00,  0x32,  0x08f4,  0x08f0,  0x08ec,  0x01c9,  0x01ca,  0x01cb
	},
	{
		145,   5725,  0x74,  0x17,  0x20,  0x1f,  0x08,  0x08,  0x3f,  0x79,  0x04,  0x06,
		0x00,  0x06,  0x00,  0x06,  0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x21,  0x06,
		0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x21,  0x06,  0x00,  0x00,  0x08,  0x42,
		0x13,  0x00,  0x21,  0x08f6,  0x08f2,  0x08ee,  0x01c9,  0x01ca,  0x01cb
	},
	{
		146,   5730,  0x76,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x3d,  0x02,  0x06,
		0x00,  0x06,  0x00,  0x06,  0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x21,  0x06,
		0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x21,  0x06,  0x00,  0x00,  0x08,  0x42,
		0x13,  0x00,  0x21,  0x08f8,  0x08f4,  0x08f0,  0x01c9,  0x01c9,  0x01ca
	},
	{
		147,   5735,  0x77,  0x17,  0x20,  0x1f,  0x08,  0x08,  0x3f,  0x7b,  0x04,  0x06,
		0x00,  0x06,  0x00,  0x06,  0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x21,  0x06,
		0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x21,  0x06,  0x00,  0x00,  0x08,  0x42,
		0x13,  0x00,  0x21,  0x08fa,  0x08f6,  0x08f2,  0x01c8,  0x01c9,  0x01ca
	},
	{
		148,   5740,  0x79,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x3e,  0x02,  0x06,
		0x00,  0x06,  0x00,  0x06,  0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x21,  0x06,
		0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x21,  0x06,  0x00,  0x00,  0x08,  0x42,
		0x13,  0x00,  0x21,  0x08fc,  0x08f8,  0x08f4,  0x01c8,  0x01c9,  0x01c9
	},
	{
		149,   5745,  0x7b,  0x17,  0x20,  0x1f,  0x08,  0x08,  0x3f,  0x7d,  0x04,  0x06,
		0x00,  0x06,  0x00,  0x06,  0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x21,  0x06,
		0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x21,  0x06,  0x00,  0x00,  0x08,  0x42,
		0x13,  0x00,  0x21,  0x08fe,  0x08fa,  0x08f6,  0x01c8,  0x01c8,  0x01c9
	},
	{
		150,   5750,  0x7c,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x3f,  0x02,  0x06,
		0x00,  0x06,  0x00,  0x06,  0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x11,  0x06,
		0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x11,  0x06,  0x00,  0x00,  0x08,  0x42,
		0x13,  0x00,  0x11,  0x0900,  0x08fc,  0x08f8,  0x01c7,  0x01c8,  0x01c9
	},
	{
		151,   5755,  0x7e,  0x17,  0x20,  0x1f,  0x08,  0x08,  0x3f,  0x7f,  0x04,  0x06,
		0x00,  0x06,  0x00,  0x06,  0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x11,  0x06,
		0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x11,  0x06,  0x00,  0x00,  0x08,  0x42,
		0x13,  0x00,  0x11,  0x0902,  0x08fe,  0x08fa,  0x01c7,  0x01c8,  0x01c8
	},
	{
		152,   5760,  0x80,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x40,  0x02,  0x06,
		0x00,  0x06,  0x00,  0x06,  0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x11,  0x06,
		0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x11,  0x06,  0x00,  0x00,  0x08,  0x42,
		0x13,  0x00,  0x11,  0x0904,  0x0900,  0x08fc,  0x01c6,  0x01c7,  0x01c8
	},
	{
		153,   5765,  0x81,  0x17,  0x20,  0x1f,  0x08,  0x08,  0x3f,  0x81,  0x04,  0x06,
		0x00,  0x06,  0x00,  0x06,  0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x11,  0x06,
		0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x11,  0x06,  0x00,  0x00,  0x08,  0x42,
		0x13,  0x00,  0x11,  0x0906,  0x0902,  0x08fe,  0x01c6,  0x01c7,  0x01c8
	},
	{
		154,   5770,  0x83,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x41,  0x02,  0x06,
		0x00,  0x06,  0x00,  0x06,  0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x00,  0x06,
		0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x00,  0x06,  0x00,  0x00,  0x08,  0x42,
		0x13,  0x00,  0x00,  0x0908,  0x0904,  0x0900,  0x01c6,  0x01c6,  0x01c7
	},
	{
		155,   5775,  0x85,  0x17,  0x20,  0x1f,  0x08,  0x08,  0x3f,  0x83,  0x04,  0x06,
		0x00,  0x06,  0x00,  0x06,  0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x00,  0x06,
		0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x00,  0x06,  0x00,  0x00,  0x08,  0x42,
		0x13,  0x00,  0x00,  0x090a,  0x0906,  0x0902,  0x01c5,  0x01c6,  0x01c7
	},
	{
		156,   5780,  0x86,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x42,  0x02,  0x06,
		0x00,  0x06,  0x00,  0x06,  0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x00,  0x06,
		0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x00,  0x06,  0x00,  0x00,  0x08,  0x42,
		0x13,  0x00,  0x00,  0x090c,  0x0908,  0x0904,  0x01c5,  0x01c6,  0x01c6
	},
	{
		157,   5785,  0x88,  0x17,  0x20,  0x1f,  0x08,  0x08,  0x3f,  0x85,  0x04,  0x05,
		0x00,  0x05,  0x00,  0x05,  0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x00,  0x05,
		0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x00,  0x05,  0x00,  0x00,  0x08,  0x42,
		0x13,  0x00,  0x00,  0x090e,  0x090a,  0x0906,  0x01c4,  0x01c5,  0x01c6
	},
	{
		158,   5790,  0x8a,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x43,  0x02,  0x05,
		0x00,  0x05,  0x00,  0x05,  0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x00,  0x05,
		0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x00,  0x05,  0x00,  0x00,  0x08,  0x42,
		0x13,  0x00,  0x00,  0x0910,  0x090c,  0x0908,  0x01c4,  0x01c5,  0x01c6
	},
	{
		159,   5795,  0x8b,  0x17,  0x20,  0x1f,  0x08,  0x08,  0x3f,  0x87,  0x04,  0x05,
		0x00,  0x05,  0x00,  0x05,  0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x00,  0x05,
		0x00,  0x00,  0x08,  0x42,  0x13,  0x00,  0x00,  0x05,  0x00,  0x00,  0x08,  0x42,
		0x13,  0x00,  0x00,  0x0912,  0x090e,  0x090a,  0x01c4,  0x01c4,  0x01c5
	},
	{
		160,   5800,  0x8d,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x44,  0x02,  0x05,
		0x00,  0x05,  0x00,  0x05,  0x00,  0x00,  0x08,  0x41,  0x03,  0x00,  0x00,  0x05,
		0x00,  0x00,  0x08,  0x41,  0x03,  0x00,  0x00,  0x05,  0x00,  0x00,  0x08,  0x41,
		0x03,  0x00,  0x00,  0x0914,  0x0910,  0x090c,  0x01c3,  0x01c4,  0x01c5
	},
	{
		161,   5805,  0x8f,  0x17,  0x20,  0x1f,  0x08,  0x08,  0x3f,  0x89,  0x04,  0x05,
		0x00,  0x05,  0x00,  0x05,  0x00,  0x00,  0x06,  0x41,  0x03,  0x00,  0x00,  0x05,
		0x00,  0x00,  0x06,  0x41,  0x03,  0x00,  0x00,  0x05,  0x00,  0x00,  0x06,  0x41,
		0x03,  0x00,  0x00,  0x0916,  0x0912,  0x090e,  0x01c3,  0x01c4,  0x01c4
	},
	{
		162,   5810,  0x90,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x45,  0x02,  0x05,
		0x00,  0x05,  0x00,  0x05,  0x00,  0x00,  0x06,  0x41,  0x03,  0x00,  0x00,  0x05,
		0x00,  0x00,  0x06,  0x41,  0x03,  0x00,  0x00,  0x05,  0x00,  0x00,  0x06,  0x41,
		0x03,  0x00,  0x00,  0x0918,  0x0914,  0x0910,  0x01c2,  0x01c3,  0x01c4
	},
	{
		163,   5815,  0x92,  0x17,  0x20,  0x1f,  0x08,  0x08,  0x3f,  0x8b,  0x04,  0x05,
		0x00,  0x05,  0x00,  0x05,  0x00,  0x00,  0x06,  0x41,  0x03,  0x00,  0x00,  0x05,
		0x00,  0x00,  0x06,  0x41,  0x03,  0x00,  0x00,  0x05,  0x00,  0x00,  0x06,  0x41,
		0x03,  0x00,  0x00,  0x091a,  0x0916,  0x0912,  0x01c2,  0x01c3,  0x01c4
	},
	{
		164,   5820,  0x94,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x46,  0x02,  0x05,
		0x00,  0x05,  0x00,  0x05,  0x00,  0x00,  0x06,  0x41,  0x03,  0x00,  0x00,  0x05,
		0x00,  0x00,  0x06,  0x41,  0x03,  0x00,  0x00,  0x05,  0x00,  0x00,  0x06,  0x41,
		0x03,  0x00,  0x00,  0x091c,  0x0918,  0x0914,  0x01c2,  0x01c2,  0x01c3
	},
	{
		165,   5825,  0x95,  0x17,  0x20,  0x1f,  0x08,  0x08,  0x3f,  0x8d,  0x04,  0x05,
		0x00,  0x05,  0x00,  0x05,  0x00,  0x00,  0x06,  0x41,  0x03,  0x00,  0x00,  0x05,
		0x00,  0x00,  0x06,  0x41,  0x03,  0x00,  0x00,  0x05,  0x00,  0x00,  0x06,  0x41,
		0x03,  0x00,  0x00,  0x091e,  0x091a,  0x0916,  0x01c1,  0x01c2,  0x01c3
	},
	{
		166,   5830,  0x97,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x47,  0x02,  0x05,
		0x00,  0x05,  0x00,  0x05,  0x00,  0x00,  0x06,  0x41,  0x03,  0x00,  0x00,  0x05,
		0x00,  0x00,  0x06,  0x41,  0x03,  0x00,  0x00,  0x05,  0x00,  0x00,  0x06,  0x41,
		0x03,  0x00,  0x00,  0x0920,  0x091c,  0x0918,  0x01c1,  0x01c2,  0x01c2
	},
	{
		168,   5840,  0x9a,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x48,  0x02,  0x05,
		0x00,  0x05,  0x00,  0x05,  0x00,  0x00,  0x06,  0x41,  0x03,  0x00,  0x00,  0x05,
		0x00,  0x00,  0x06,  0x41,  0x03,  0x00,  0x00,  0x05,  0x00,  0x00,  0x06,  0x41,
		0x03,  0x00,  0x00,  0x0924,  0x0920,  0x091c,  0x01c0,  0x01c1,  0x01c2
	},
	{
		170,   5850,  0x9e,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x49,  0x02,  0x04,
		0x00,  0x04,  0x00,  0x04,  0x00,  0x00,  0x06,  0x41,  0x03,  0x00,  0x00,  0x04,
		0x00,  0x00,  0x06,  0x41,  0x03,  0x00,  0x00,  0x04,  0x00,  0x00,  0x06,  0x41,
		0x03,  0x00,  0x00,  0x0928,  0x0924,  0x0920,  0x01bf,  0x01c0,  0x01c1
	},
	{
		172,   5860,  0xa1,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x4a,  0x02,  0x04,
		0x00,  0x04,  0x00,  0x04,  0x00,  0x00,  0x06,  0x41,  0x03,  0x00,  0x00,  0x04,
		0x00,  0x00,  0x06,  0x41,  0x03,  0x00,  0x00,  0x04,  0x00,  0x00,  0x06,  0x41,
		0x03,  0x00,  0x00,  0x092c,  0x0928,  0x0924,  0x01bf,  0x01bf,  0x01c0
	},
	{
		174,   5870,  0xa4,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x4b,  0x02,  0x04,
		0x00,  0x04,  0x00,  0x04,  0x00,  0x00,  0x06,  0x41,  0x03,  0x00,  0x00,  0x04,
		0x00,  0x00,  0x06,  0x41,  0x03,  0x00,  0x00,  0x04,  0x00,  0x00,  0x06,  0x41,
		0x03,  0x00,  0x00,  0x0930,  0x092c,  0x0928,  0x01be,  0x01bf,  0x01bf
	},
	{
		176,   5880,  0xa8,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x4c,  0x02,  0x03,
		0x00,  0x03,  0x00,  0x03,  0x00,  0x00,  0x06,  0x41,  0x03,  0x00,  0x00,  0x03,
		0x00,  0x00,  0x06,  0x41,  0x03,  0x00,  0x00,  0x03,  0x00,  0x00,  0x06,  0x41,
		0x03,  0x00,  0x00,  0x0934,  0x0930,  0x092c,  0x01bd,  0x01be,  0x01bf
	},
	{
		178,   5890,  0xab,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x4d,  0x02,  0x03,
		0x00,  0x03,  0x00,  0x03,  0x00,  0x00,  0x06,  0x41,  0x03,  0x00,  0x00,  0x03,
		0x00,  0x00,  0x06,  0x41,  0x03,  0x00,  0x00,  0x03,  0x00,  0x00,  0x06,  0x41,
		0x03,  0x00,  0x00,  0x0938,  0x0934,  0x0930,  0x01bc,  0x01bd,  0x01be
	},
	{
		180,   5900,  0xae,  0x17,  0x10,  0x1f,  0x08,  0x08,  0x3f,  0x4e,  0x02,  0x03,
		0x00,  0x03,  0x00,  0x03,  0x00,  0x00,  0x06,  0x41,  0x03,  0x00,  0x00,  0x03,
		0x00,  0x00,  0x06,  0x41,  0x03,  0x00,  0x00,  0x03,  0x00,  0x00,  0x06,  0x41,
		0x03,  0x00,  0x00,  0x093c,  0x0938,  0x0934,  0x01bc,  0x01bc,  0x01bd
	},
	{
		1,   2412,  0x48,  0x16,  0x30,  0x1b,  0x0a,  0x0a,  0x30,  0x6c,  0x09,  0x0f,
		0x0a,  0x00,  0x0a,  0x00,  0x61,  0x73,  0x00,  0x00,  0x00,  0xd0,  0x00,  0x00,
		0x61,  0x73,  0x00,  0x00,  0x00,  0xd0,  0x00,  0x00,  0x61,  0x73,  0x00,  0x00,
		0x00,  0xd0,  0x00,  0x03c9,  0x03c5,  0x03c1,  0x043a,  0x043f,  0x0443
	},
	{
		2,   2417,  0x4b,  0x16,  0x30,  0x1b,  0x0a,  0x0a,  0x30,  0x71,  0x09,  0x0f,
		0x0a,  0x00,  0x0a,  0x00,  0x61,  0x73,  0x00,  0x00,  0x00,  0xd0,  0x00,  0x00,
		0x61,  0x73,  0x00,  0x00,  0x00,  0xd0,  0x00,  0x00,  0x61,  0x73,  0x00,  0x00,
		0x00,  0xd0,  0x00,  0x03cb,  0x03c7,  0x03c3,  0x0438,  0x043d,  0x0441
	},
	{
		3,   2422,  0x4e,  0x16,  0x30,  0x1b,  0x0a,  0x0a,  0x30,  0x76,  0x09,  0x0f,
		0x09,  0x00,  0x09,  0x00,  0x61,  0x73,  0x00,  0x00,  0x00,  0xd0,  0x00,  0x00,
		0x61,  0x73,  0x00,  0x00,  0x00,  0xd0,  0x00,  0x00,  0x61,  0x73,  0x00,  0x00,
		0x00,  0xd0,  0x00,  0x03cd,  0x03c9,  0x03c5,  0x0436,  0x043a,  0x043f
	},
	{
		4,   2427,  0x52,  0x16,  0x30,  0x1b,  0x0a,  0x0a,  0x30,  0x7b,  0x09,  0x0f,
		0x09,  0x00,  0x09,  0x00,  0x61,  0x73,  0x00,  0x00,  0x00,  0xa0,  0x00,  0x00,
		0x61,  0x73,  0x00,  0x00,  0x00,  0xa0,  0x00,  0x00,  0x61,  0x73,  0x00,  0x00,
		0x00,  0xa0,  0x00,  0x03cf,  0x03cb,  0x03c7,  0x0434,  0x0438,  0x043d
	},
	{
		5,   2432,  0x55,  0x16,  0x30,  0x1b,  0x0a,  0x0a,  0x30,  0x80,  0x09,  0x0f,
		0x08,  0x00,  0x08,  0x00,  0x61,  0x73,  0x00,  0x00,  0x00,  0xa0,  0x00,  0x00,
		0x61,  0x73,  0x00,  0x00,  0x00,  0xa0,  0x00,  0x00,  0x61,  0x73,  0x00,  0x00,
		0x00,  0xa0,  0x00,  0x03d1,  0x03cd,  0x03c9,  0x0431,  0x0436,  0x043a
	},
	{
		6,   2437,  0x58,  0x16,  0x30,  0x1b,  0x0a,  0x0a,  0x30,  0x85,  0x09,  0x0f,
		0x08,  0x00,  0x08,  0x00,  0x61,  0x73,  0x00,  0x00,  0x00,  0xa0,  0x00,  0x00,
		0x61,  0x73,  0x00,  0x00,  0x00,  0xa0,  0x00,  0x00,  0x61,  0x73,  0x00,  0x00,
		0x00,  0xa0,  0x00,  0x03d3,  0x03cf,  0x03cb,  0x042f,  0x0434,  0x0438
	},
	{
		7,   2442,  0x5c,  0x16,  0x30,  0x1b,  0x0a,  0x0a,  0x30,  0x8a,  0x09,  0x0f,
		0x07,  0x00,  0x07,  0x00,  0x61,  0x73,  0x00,  0x00,  0x00,  0x80,  0x00,  0x00,
		0x61,  0x73,  0x00,  0x00,  0x00,  0x80,  0x00,  0x00,  0x61,  0x73,  0x00,  0x00,
		0x00,  0x80,  0x00,  0x03d5,  0x03d1,  0x03cd,  0x042d,  0x0431,  0x0436
	},
	{
		8,   2447,  0x5f,  0x16,  0x30,  0x1b,  0x0a,  0x0a,  0x30,  0x8f,  0x09,  0x0f,
		0x07,  0x00,  0x07,  0x00,  0x61,  0x73,  0x00,  0x00,  0x00,  0x80,  0x00,  0x00,
		0x61,  0x73,  0x00,  0x00,  0x00,  0x80,  0x00,  0x00,  0x61,  0x73,  0x00,  0x00,
		0x00,  0x80,  0x00,  0x03d7,  0x03d3,  0x03cf,  0x042b,  0x042f,  0x0434
	},
	{
		9,   2452,  0x62,  0x16,  0x30,  0x1b,  0x0a,  0x0a,  0x30,  0x94,  0x09,  0x0f,
		0x07,  0x00,  0x07,  0x00,  0x61,  0x73,  0x00,  0x00,  0x00,  0x80,  0x00,  0x00,
		0x61,  0x73,  0x00,  0x00,  0x00,  0x80,  0x00,  0x00,  0x61,  0x73,  0x00,  0x00,
		0x00,  0x80,  0x00,  0x03d9,  0x03d5,  0x03d1,  0x0429,  0x042d,  0x0431
	},
	{
		10,   2457,  0x66,  0x16,  0x30,  0x1b,  0x0a,  0x0a,  0x30,  0x99,  0x09,  0x0f,
		0x06,  0x00,  0x06,  0x00,  0x61,  0x73,  0x00,  0x00,  0x00,  0x60,  0x00,  0x00,
		0x61,  0x73,  0x00,  0x00,  0x00,  0x60,  0x00,  0x00,  0x61,  0x73,  0x00,  0x00,
		0x00,  0x60,  0x00,  0x03db,  0x03d7,  0x03d3,  0x0427,  0x042b,  0x042f
	},
	{
		11,   2462,  0x69,  0x16,  0x30,  0x1b,  0x0a,  0x0a,  0x30,  0x9e,  0x09,  0x0f,
		0x06,  0x00,  0x06,  0x00,  0x61,  0x73,  0x00,  0x00,  0x00,  0x60,  0x00,  0x00,
		0x61,  0x73,  0x00,  0x00,  0x00,  0x60,  0x00,  0x00,  0x61,  0x73,  0x00,  0x00,
		0x00,  0x60,  0x00,  0x03dd,  0x03d9,  0x03d5,  0x0424,  0x0429,  0x042d
	},
	{
		12,   2467,  0x6c,  0x16,  0x30,  0x1b,  0x0a,  0x0a,  0x30,  0xa3,  0x09,  0x0f,
		0x05,  0x00,  0x05,  0x00,  0x61,  0x73,  0x00,  0x00,  0x00,  0x40,  0x00,  0x00,
		0x61,  0x73,  0x00,  0x00,  0x00,  0x40,  0x00,  0x00,  0x61,  0x73,  0x00,  0x00,
		0x00,  0x40,  0x00,  0x03df,  0x03db,  0x03d7,  0x0422,  0x0427,  0x042b
	},
	{
		13,   2472,  0x70,  0x16,  0x30,  0x1b,  0x0a,  0x0a,  0x30,  0xa8,  0x09,  0x0f,
		0x05,  0x00,  0x05,  0x00,  0x61,  0x73,  0x00,  0x00,  0x00,  0x40,  0x00,  0x00,
		0x61,  0x73,  0x00,  0x00,  0x00,  0x40,  0x00,  0x00,  0x61,  0x73,  0x00,  0x00,
		0x00,  0x40,  0x00,  0x03e1,  0x03dd,  0x03d9,  0x0420,  0x0424,  0x0429
	},
	{
		14,   2484,  0x78,  0x16,  0x30,  0x1b,  0x0a,  0x0a,  0x30,  0xb4,  0x09,  0x0f,
		0x04,  0x00,  0x04,  0x00,  0x61,  0x73,  0x00,  0x00,  0x00,  0x40,  0x00,  0x00,
		0x61,  0x73,  0x00,  0x00,  0x00,  0x40,  0x00,  0x00,  0x61,  0x73,  0x00,  0x00,
		0x00,  0x40,  0x00,  0x03e6,  0x03e2,  0x03de,  0x041b,  0x041f,  0x0424
	}
};

/* %%%%%% radio reg */

/* 2059 rev0 register initialization tables */
radio_20xx_regs_t regs_2059_rev0[] = {
	{ 0x0000 | JTAG_2059_ALL,           0,  0 },
	{ 0x0001 | JTAG_2059_ALL,        0x59,  0 },
	{ 0x0002 | JTAG_2059_ALL,        0x20,  0 },
	{ 0x0003 | JTAG_2059_CR0,        0x1f,  0 },
	{ 0x0004 | JTAG_2059_CR2,         0x4,  0 },
	{ 0x0005 | JTAG_2059_CR2,         0x2,  0 },
	{ 0x0006 | JTAG_2059_CR2,         0x1,  0 },
	{ 0x0007 | JTAG_2059_CR2,         0x1,  0 },
	{ 0x0008 | JTAG_2059_CR0,         0x1,  0 },
	{ 0x0009 | JTAG_2059_CR0,        0x69,  0 },
	{ 0x000A | JTAG_2059_CR0,        0x66,  0 },
	{ 0x000B | JTAG_2059_CR0,         0x6,  0 },
	{ 0x000C | JTAG_2059_CR0,           0,  0 },
	{ 0x000D | JTAG_2059_CR0,         0x3,  0 },
	{ 0x000E | JTAG_2059_CR0,        0x20,  0 },
	{ 0x000F | JTAG_2059_CR0,        0x20,  0 },
	{ 0x0010 | JTAG_2059_CR0,           0,  0 },
	{ 0x0011 | JTAG_2059_CR0,        0x7c,  0 },
	{ 0x0012 | JTAG_2059_CR0,        0x42,  0 },
	{ 0x0013 | JTAG_2059_CR0,        0xbd,  0 },
	{ 0x0014 | JTAG_2059_CR0,         0x7,  0 },
	{ 0x0015 | JTAG_2059_CR0,        0x87,  0 },
	{ 0x0016 | JTAG_2059_CR0,         0x8,  0 },
	{ 0x0017 | JTAG_2059_CR0,        0x17,  0 },
	{ 0x0018 | JTAG_2059_CR0,         0x7,  0 },
	{ 0x0019 | JTAG_2059_CR0,           0,  0 },
	{ 0x001A | JTAG_2059_CR0,         0x2,  0 },
	{ 0x001B | JTAG_2059_CR0,        0x13,  0 },
	{ 0x001C | JTAG_2059_CR0,        0x3e,  0 },
	{ 0x001D | JTAG_2059_CR0,        0x3e,  0 },
	{ 0x001E | JTAG_2059_CR0,        0x96,  0 },
	{ 0x001F | JTAG_2059_CR0,         0x4,  0 },
	{ 0x0020 | JTAG_2059_CR0,           0,  0 },
	{ 0x0021 | JTAG_2059_CR0,           0,  0 },
	{ 0x0022 | JTAG_2059_CR0,        0x17,  0 },
	{ 0x0023 | JTAG_2059_CR0,         0x6,  0 },
	{ 0x0024 | JTAG_2059_CR0,         0x1,  0 },
	{ 0x0025 | JTAG_2059_CR0,         0x6,  0 },
	{ 0x0026 | JTAG_2059_CR0,         0x4,  0 },
	{ 0x0027 | JTAG_2059_CR0,         0xd,  0 },
	{ 0x0028 | JTAG_2059_CR0,         0xd,  0 },
	{ 0x0029 | JTAG_2059_CR0,        0x30,  0 },
	{ 0x002A | JTAG_2059_CR0,        0x32,  0 },
	{ 0x002B | JTAG_2059_CR0,         0x8,  0 },
	{ 0x002C | JTAG_2059_CR0,        0x1c,  0 },
	{ 0x002D | JTAG_2059_CR0,         0x2,  0 },
	{ 0x002E | JTAG_2059_CR0,         0x4,  0 },
	{ 0x002F | JTAG_2059_CR0,        0x7f,  0 },
	{ 0x0030 | JTAG_2059_CR0,        0x27,  0 },
	{ 0x0031 | JTAG_2059_CR0,           0,  0 },
	{ 0x0032 | JTAG_2059_CR0,           0,  0 },
	{ 0x0033 | JTAG_2059_CR0,           0,  0 },
	{ 0x0034 | JTAG_2059_CR0,           0,  0 },
	{ 0x0035 | JTAG_2059_CR0,        0x20,  0 },
	{ 0x0036 | JTAG_2059_CR0,        0x18,  0 },
	{ 0x0037 | JTAG_2059_CR0,         0x7,  0 },
	{ 0x0038 | JTAG_2059_ALL,         0x8,  0 },
	{ 0x0039 | JTAG_2059_ALL,         0x8,  0 },
	{ 0x003A | JTAG_2059_ALL,         0x8,  0 },
	{ 0x003B | JTAG_2059_ALL,         0x8,  0 },
	{ 0x003C | JTAG_2059_ALL,         0x8,  0 },
	{ 0x003D | JTAG_2059_ALL,         0x8,  0 },
	{ 0x003E | JTAG_2059_ALL,         0x8,  0 },
	{ 0x003F | JTAG_2059_ALL,         0x8,  0 },
	{ 0x0040 | JTAG_2059_CR0,        0x16,  0 },
	{ 0x0041 | JTAG_2059_CR0,         0x7,  0 },
	{ 0x0042 | JTAG_2059_CR0,        0x19,  0 },
	{ 0x0043 | JTAG_2059_CR0,         0x7,  0 },
	{ 0x0044 | JTAG_2059_CR0,         0x6,  0 },
	{ 0x0045 | JTAG_2059_CR0,         0x3,  0 },
	{ 0x0046 | JTAG_2059_CR0,         0x1,  0 },
	{ 0x0047 | JTAG_2059_CR0,         0x7,  0 },
	{ 0x0048 | JTAG_2059_ALL,         0x8,  0 },
	{ 0x0049 | JTAG_2059_ALL,         0x5,  0 },
	{ 0x004A | JTAG_2059_ALL,         0x8,  0 },
	{ 0x004B | JTAG_2059_ALL,         0x8,  0 },
	{ 0x004C | JTAG_2059_ALL,         0x8,  0 },
	{ 0x004D | JTAG_2059_CR0,           0,  0 },
	{ 0x004E | JTAG_2059_CR0,         0x4,  0 },
	{ 0x004F | JTAG_2059_ALL,         0xc,  0 },
	{ 0x0050 | JTAG_2059_ALL,           0,  0 },
	{ 0x0051 | JTAG_2059_ALL,        0x70,  1 },
	{ 0x0052 | JTAG_2059_ALL,         0x7,  0 },
	{ 0x0053 | JTAG_2059_ALL,           0,  0 },
	{ 0x0054 | JTAG_2059_ALL,           0,  0 },
	{ 0x0055 | JTAG_2059_ALL,        0x88,  0 },
	{ 0x0056 | JTAG_2059_ALL,           0,  0 },
	{ 0x0057 | JTAG_2059_ALL,        0x1f,  0 },
	{ 0x0058 | JTAG_2059_ALL,        0x20,  0 },
	{ 0x0059 | JTAG_2059_ALL,         0x1,  0 },
	{ 0x005A | JTAG_2059_ALL,         0x3,  1 },
	{ 0x005B | JTAG_2059_ALL,        0x70,  0 },
	{ 0x005C | JTAG_2059_ALL,           0,  0 },
	{ 0x005D | JTAG_2059_ALL,           0,  0 },
	{ 0x005E | JTAG_2059_ALL,        0x33,  0 },
	{ 0x005F | JTAG_2059_ALL,         0xf,  0 },
	{ 0x0060 | JTAG_2059_ALL,         0xf,  0 },
	{ 0x0061 | JTAG_2059_ALL,           0,  0 },
	{ 0x0062 | JTAG_2059_ALL,        0x11,  0 },
	{ 0x0063 | JTAG_2059_ALL,           0,  0 },
	{ 0x0064 | JTAG_2059_ALL,        0x7e,  0 },
	{ 0x0065 | JTAG_2059_ALL,        0x3f,  0 },
	{ 0x0066 | JTAG_2059_ALL,        0x7f,  0 },
	{ 0x0067 | JTAG_2059_ALL,        0x78,  0 },
	{ 0x0068 | JTAG_2059_ALL,        0x58,  0 },
	{ 0x0069 | JTAG_2059_ALL,        0x88,  0 },
	{ 0x006A | JTAG_2059_ALL,         0x8,  0 },
	{ 0x006B | JTAG_2059_ALL,         0xf,  0 },
	{ 0x006C | JTAG_2059_ALL,        0xbc,  0 },
	{ 0x006D | JTAG_2059_ALL,         0x8,  0 },
	{ 0x006E | JTAG_2059_ALL,        0x60,  0 },
	{ 0x006F | JTAG_2059_ALL,        0x13,  0 },
	{ 0x0070 | JTAG_2059_ALL,        0x70,  0 },
	{ 0x0071 | JTAG_2059_ALL,           0,  0 },
	{ 0x0072 | JTAG_2059_ALL,           0,  0 },
	{ 0x0073 | JTAG_2059_ALL,           0,  0 },
	{ 0x0074 | JTAG_2059_ALL,        0x33,  0 },
	{ 0x0075 | JTAG_2059_ALL,        0x13,  0 },
	{ 0x0076 | JTAG_2059_ALL,         0xf,  0 },
	{ 0x0077 | JTAG_2059_ALL,        0x11,  0 },
	{ 0x0078 | JTAG_2059_ALL,        0x3c,  0 },
	{ 0x0079 | JTAG_2059_ALL,         0x1,  1 },
	{ 0x007A | JTAG_2059_ALL,         0xa,  0 },
	{ 0x007B | JTAG_2059_ALL,        0x9d,  0 },
	{ 0x007C | JTAG_2059_ALL,         0xa,  0 },
	{ 0x007D | JTAG_2059_ALL,           0,  0 },
	{ 0x007E | JTAG_2059_ALL,        0x40,  0 },
	{ 0x007F | JTAG_2059_ALL,        0x40,  0 },
	{ 0x0080 | JTAG_2059_ALL,        0x88,  0 },
	{ 0x0081 | JTAG_2059_ALL,        0x10,  0 },
	{ 0x0082 | JTAG_2059_ALL,        0x70,  1 },
	{ 0x0083 | JTAG_2059_ALL,           0,  1 },
	{ 0x0084 | JTAG_2059_ALL,        0x70,  1 },
	{ 0x0085 | JTAG_2059_ALL,           0,  0 },
	{ 0x0086 | JTAG_2059_ALL,           0,  0 },
	{ 0x0087 | JTAG_2059_ALL,        0x10,  0 },
	{ 0x0088 | JTAG_2059_ALL,        0x55,  0 },
	{ 0x0089 | JTAG_2059_ALL,        0x3f,  0 },
	{ 0x008A | JTAG_2059_ALL,        0x36,  0 },
	{ 0x008B | JTAG_2059_ALL,           0,  0 },
	{ 0x008C | JTAG_2059_ALL,           0,  0 },
	{ 0x008D | JTAG_2059_ALL,           0,  0 },
	{ 0x008E | JTAG_2059_ALL,        0x87,  0 },
	{ 0x008F | JTAG_2059_ALL,        0x11,  0 },
	{ 0x0090 | JTAG_2059_ALL,           0,  0 },
	{ 0x0091 | JTAG_2059_ALL,        0x33,  0 },
	{ 0x0092 | JTAG_2059_ALL,        0x88,  0 },
	{ 0x0093 | JTAG_2059_ALL,           0,  0 },
	{ 0x0094 | JTAG_2059_ALL,        0x87,  0 },
	{ 0x0095 | JTAG_2059_ALL,        0x11,  0 },
	{ 0x0096 | JTAG_2059_ALL,           0,  0 },
	{ 0x0097 | JTAG_2059_ALL,        0x33,  0 },
	{ 0x0098 | JTAG_2059_ALL,        0x88,  0 },
	{ 0x0099 | JTAG_2059_ALL,        0x20,  0 },
	{ 0x009A | JTAG_2059_ALL,        0x7f,  1 },
	{ 0x009B | JTAG_2059_ALL,        0x44,  0 },
	{ 0x009C | JTAG_2059_ALL,        0x8c,  0 },
	{ 0x009D | JTAG_2059_ALL,        0x6c,  0 },
	{ 0x009E | JTAG_2059_ALL,        0x22,  0 },
	{ 0x009F | JTAG_2059_ALL,        0xbe,  0 },
	{ 0x00A0 | JTAG_2059_ALL,        0x55,  0 },
	{ 0x00A1 | JTAG_2059_ALL,         0xc,  0 },
	{ 0x00A2 | JTAG_2059_ALL,        0xaa,  0 },
	{ 0x00A3 | JTAG_2059_ALL,         0x2,  0 },
	{ 0x00A4 | JTAG_2059_ALL,           0,  0 },
	{ 0x00A5 | JTAG_2059_ALL,        0x10,  0 },
	{ 0x00A6 | JTAG_2059_ALL,         0x1,  0 },
	{ 0x00A7 | JTAG_2059_ALL,           0,  0 },
	{ 0x00A8 | JTAG_2059_ALL,           0,  0 },
	{ 0x00A9 | JTAG_2059_ALL,        0x80,  0 },
	{ 0x00AA | JTAG_2059_ALL,        0x60,  0 },
	{ 0x00AB | JTAG_2059_ALL,        0x44,  0 },
	{ 0x00AC | JTAG_2059_ALL,        0x55,  0 },
	{ 0x00AD | JTAG_2059_ALL,         0x1,  0 },
	{ 0x00AE | JTAG_2059_ALL,        0x55,  0 },
	{ 0x00AF | JTAG_2059_ALL,         0x1,  0 },
	{ 0x00B0 | JTAG_2059_ALL,         0x5,  0 },
	{ 0x00B1 | JTAG_2059_ALL,        0x55,  0 },
	{ 0x00B2 | JTAG_2059_ALL,        0x55,  0 },
	{ 0x00B3 | JTAG_2059_ALL,           0,  0 },
	{ 0x00B4 | JTAG_2059_ALL,           0,  0 },
	{ 0x00B5 | JTAG_2059_ALL,           0,  0 },
	{ 0x00B6 | JTAG_2059_ALL,           0,  0 },
	{ 0x00B7 | JTAG_2059_ALL,           0,  0 },
	{ 0x00B8 | JTAG_2059_ALL,           0,  0 },
	{ 0x00B9 | JTAG_2059_ALL,           0,  0 },
	{ 0x00BA | JTAG_2059_ALL,           0,  0 },
	{ 0x00BB | JTAG_2059_ALL,           0,  0 },
	{ 0x00BC | JTAG_2059_ALL,           0,  0 },
	{ 0x00BD | JTAG_2059_ALL,           0,  0 },
	{ 0x00BE | JTAG_2059_ALL,           0,  0 },
	{ 0x00BF | JTAG_2059_CR2,           0,  0 },
	{ 0x00C0 | JTAG_2059_CR0,        0x5e,  0 },
	{ 0x00C1 | JTAG_2059_ALL,         0xc,  0 },
	{ 0x00C2 | JTAG_2059_ALL,         0xc,  0 },
	{ 0x00C3 | JTAG_2059_ALL,         0xc,  0 },
	{ 0x00C4 | JTAG_2059_ALL,           0,  0 },
	{ 0x00C5 | JTAG_2059_ALL,        0x2b,  0 },
	{ 0x013C | JTAG_2059_CR0,        0x15,  0 },
	{ 0x013D | JTAG_2059_CR0,         0xf,  0 },
	{ 0x013E | JTAG_2059_CR0,           0,  0 },
	{ 0x013F | JTAG_2059_CR0,           0,  0 },
	{ 0x0140 | JTAG_2059_CR0,           0,  0 },
	{ 0x0141 | JTAG_2059_CR0,           0,  0 },
	{ 0x0142 | JTAG_2059_CR0,           0,  0 },
	{ 0x0143 | JTAG_2059_CR0,           0,  0 },
	{ 0x0144 | JTAG_2059_CR0,           0,  0 },
	{ 0x0145 | JTAG_2059_CR2,           0,  0 },
	{ 0x0146 | JTAG_2059_ALL,           0,  0 },
	{ 0x0147 | JTAG_2059_ALL,           0,  0 },
	{ 0x0148 | JTAG_2059_ALL,           0,  0 },
	{ 0x0149 | JTAG_2059_ALL,           0,  0 },
	{ 0x014A | JTAG_2059_ALL,           0,  0 },
	{ 0x014B | JTAG_2059_ALL,           0,  0 },
	{ 0x014C | JTAG_2059_CR0,           0,  0 },
	{ 0x014D | JTAG_2059_CR0,           0,  0 },
	{ 0x014E | JTAG_2059_CR0,           0,  0 },
	{ 0x014F | JTAG_2059_ALL,           0,  0 },
	{ 0x0150 | JTAG_2059_ALL,           0,  0 },
	{ 0x0151 | JTAG_2059_ALL,        0x77,  0 },
	{ 0x0152 | JTAG_2059_ALL,        0x77,  0 },
	{ 0x0153 | JTAG_2059_ALL,        0x77,  0 },
	{ 0x0154 | JTAG_2059_ALL,        0x77,  0 },
	{ 0x0155 | JTAG_2059_ALL,           0,  0 },
	{ 0x0156 | JTAG_2059_ALL,         0x3,  0 },
	{ 0x0157 | JTAG_2059_ALL,        0x37,  0 },
	{ 0x0158 | JTAG_2059_ALL,         0x3,  0 },
	{ 0x0159 | JTAG_2059_ALL,           0,  0 },
	{ 0x015A | JTAG_2059_ALL,        0x21,  0 },
	{ 0x015B | JTAG_2059_ALL,        0x21,  0 },
	{ 0x015C | JTAG_2059_ALL,           0,  0 },
	{ 0x015D | JTAG_2059_ALL,        0xaa,  0 },
	{ 0x015E | JTAG_2059_ALL,           0,  0 },
	{ 0x015F | JTAG_2059_ALL,        0xaa,  0 },
	{ 0x0160 | JTAG_2059_ALL,           0,  0 },
	{ 0x0172 | JTAG_2059_ALL,         0x2,  0 },
	{ 0x0173 | JTAG_2059_ALL,         0xf,  0 },
	{ 0x0174 | JTAG_2059_ALL,         0xf,  0 },
	{ 0x0175 | JTAG_2059_ALL,           0,  0 },
	{ 0x0176 | JTAG_2059_ALL,           0,  0 },
	{ 0x0177 | JTAG_2059_ALL,           0,  0 },
	{ 0x017E | JTAG_2059_ALL,        0x84,  0 },
	{ 0x017F | JTAG_2059_CR0,        0x60,  0 },
	{ 0x0180 | JTAG_2059_ALL,        0x47,  0 },
	{ 0x0182 | JTAG_2059_ALL,           0,  0 },
	{ 0x0183 | JTAG_2059_ALL,           0,  0 },
	{ 0x0184 | JTAG_2059_ALL,           0,  0 },
	{ 0x0185 | JTAG_2059_ALL,           0,  0 },
	{ 0x0186 | JTAG_2059_ALL,           0,  0 },
	{ 0x0187 | JTAG_2059_ALL,           0,  0 },
	{ 0x0188 | JTAG_2059_ALL,         0x5,  1 },
	{ 0x0189 | JTAG_2059_ALL,           0,  0 },
	{ 0x018A | JTAG_2059_ALL,           0,  0 },
	{ 0x018B | JTAG_2059_ALL,           0,  0 },
	{ 0x018C | JTAG_2059_ALL,           0,  0 },
	{ 0x018D | JTAG_2059_ALL,           0,  0 },
	{ 0x018E | JTAG_2059_ALL,           0,  0 },
	{ 0x018F | JTAG_2059_ALL,           0,  0 },
	{ 0x019A | JTAG_2059_ALL,           0,  0 },
	{ 0x019B | JTAG_2059_CR2,           0,  0 },
	{ 0x019C | JTAG_2059_ALL,           0,  0 },
	{ 0x019D | JTAG_2059_ALL,         0x2,  0 },
	{ 0x019E | JTAG_2059_ALL,         0x1,  0 },
	{ 0x019F | JTAG_2059_ALL,           0,  0 },
	{ 0x01A0 | JTAG_2059_ALL,         0x1,  0 },
	{ 0xFFFF,                           0,  0 }
};

/* 2059 rev0v1 register initialization tables */
radio_20xx_regs_t regs_2059_rev0v1[] = {
	{ 0x0000 | JTAG_2059_ALL,        0x10,  0 },
	{ 0x0001 | JTAG_2059_ALL,        0x59,  0 },
	{ 0x0002 | JTAG_2059_ALL,        0x20,  0 },
	{ 0x0003 | JTAG_2059_CR0,        0x1f,  0 },
	{ 0x0004 | JTAG_2059_CR2,         0x4,  0 },
	{ 0x0005 | JTAG_2059_CR2,         0x2,  0 },
	{ 0x0006 | JTAG_2059_CR2,         0x1,  0 },
	{ 0x0007 | JTAG_2059_CR2,         0x1,  0 },
	{ 0x0008 | JTAG_2059_CR0,         0x1,  0 },
	{ 0x0009 | JTAG_2059_CR0,        0x69,  0 },
	{ 0x000A | JTAG_2059_CR0,        0x66,  0 },
	{ 0x000B | JTAG_2059_CR0,         0x6,  0 },
	{ 0x000C | JTAG_2059_CR0,           0,  0 },
	{ 0x000D | JTAG_2059_CR0,         0x3,  0 },
	{ 0x000E | JTAG_2059_CR0,        0x20,  0 },
	{ 0x000F | JTAG_2059_CR0,        0x20,  0 },
	{ 0x0010 | JTAG_2059_CR0,           0,  0 },
	{ 0x0011 | JTAG_2059_CR0,        0x7c,  0 },
	{ 0x0012 | JTAG_2059_CR0,        0x42,  0 },
	{ 0x0013 | JTAG_2059_CR0,        0xbd,  0 },
	{ 0x0014 | JTAG_2059_CR0,         0x7,  0 },
	{ 0x0015 | JTAG_2059_CR0,        0x87,  0 },
	{ 0x0016 | JTAG_2059_CR0,         0x8,  0 },
	{ 0x0017 | JTAG_2059_CR0,        0x17,  0 },
	{ 0x0018 | JTAG_2059_CR0,         0x7,  0 },
	{ 0x0019 | JTAG_2059_CR0,           0,  0 },
	{ 0x001A | JTAG_2059_CR0,         0x2,  0 },
	{ 0x001B | JTAG_2059_CR0,        0x13,  0 },
	{ 0x001C | JTAG_2059_CR0,        0x3e,  0 },
	{ 0x001D | JTAG_2059_CR0,        0x3e,  0 },
	{ 0x001E | JTAG_2059_CR0,        0x96,  0 },
	{ 0x001F | JTAG_2059_CR0,         0x4,  0 },
	{ 0x0020 | JTAG_2059_CR0,           0,  0 },
	{ 0x0021 | JTAG_2059_CR0,           0,  0 },
	{ 0x0022 | JTAG_2059_CR0,        0x17,  0 },
	{ 0x0023 | JTAG_2059_CR0,         0x6,  0 },
	{ 0x0024 | JTAG_2059_CR0,         0x1,  0 },
	{ 0x0025 | JTAG_2059_CR0,         0x6,  0 },
	{ 0x0026 | JTAG_2059_CR0,         0x4,  0 },
	{ 0x0027 | JTAG_2059_CR0,         0xd,  0 },
	{ 0x0028 | JTAG_2059_CR0,         0xd,  0 },
	{ 0x0029 | JTAG_2059_CR0,        0x30,  0 },
	{ 0x002A | JTAG_2059_CR0,        0x32,  0 },
	{ 0x002B | JTAG_2059_CR0,         0x8,  0 },
	{ 0x002C | JTAG_2059_CR0,        0x1c,  0 },
	{ 0x002D | JTAG_2059_CR0,         0x2,  0 },
	{ 0x002E | JTAG_2059_CR0,         0x4,  0 },
	{ 0x002F | JTAG_2059_CR0,        0x7f,  0 },
	{ 0x0030 | JTAG_2059_CR0,        0x27,  0 },
	{ 0x0031 | JTAG_2059_CR0,           0,  0 },
	{ 0x0032 | JTAG_2059_CR0,           0,  0 },
	{ 0x0033 | JTAG_2059_CR0,           0,  0 },
	{ 0x0034 | JTAG_2059_CR0,           0,  0 },
	{ 0x0035 | JTAG_2059_CR0,        0x20,  0 },
	{ 0x0036 | JTAG_2059_CR0,        0x18,  0 },
	{ 0x0037 | JTAG_2059_CR0,         0x7,  0 },
	{ 0x0038 | JTAG_2059_ALL,         0x8,  0 },
	{ 0x0039 | JTAG_2059_ALL,         0x8,  0 },
	{ 0x003A | JTAG_2059_ALL,         0x8,  0 },
	{ 0x003B | JTAG_2059_ALL,         0x8,  0 },
	{ 0x003C | JTAG_2059_ALL,         0x8,  0 },
	{ 0x003D | JTAG_2059_ALL,         0x8,  0 },
	{ 0x003E | JTAG_2059_ALL,         0x8,  0 },
	{ 0x003F | JTAG_2059_ALL,         0x8,  0 },
	{ 0x0040 | JTAG_2059_CR0,        0x16,  0 },
	{ 0x0041 | JTAG_2059_CR0,         0x7,  0 },
	{ 0x0042 | JTAG_2059_CR0,        0x19,  0 },
	{ 0x0043 | JTAG_2059_CR0,         0x7,  0 },
	{ 0x0044 | JTAG_2059_CR0,         0x6,  0 },
	{ 0x0045 | JTAG_2059_CR0,         0x3,  0 },
	{ 0x0046 | JTAG_2059_CR0,         0x1,  0 },
	{ 0x0047 | JTAG_2059_CR0,         0x7,  0 },
	{ 0x0048 | JTAG_2059_ALL,         0x8,  0 },
	{ 0x0049 | JTAG_2059_ALL,         0x5,  0 },
	{ 0x004A | JTAG_2059_ALL,         0x8,  0 },
	{ 0x004B | JTAG_2059_ALL,         0x8,  0 },
	{ 0x004C | JTAG_2059_ALL,         0x8,  0 },
	{ 0x004D | JTAG_2059_CR0,           0,  0 },
	{ 0x004E | JTAG_2059_CR0,         0x4,  0 },
	{ 0x004F | JTAG_2059_ALL,         0xc,  0 },
	{ 0x0050 | JTAG_2059_ALL,           0,  0 },
	{ 0x0051 | JTAG_2059_ALL,        0x70,  1 },
	{ 0x0052 | JTAG_2059_ALL,         0x7,  0 },
	{ 0x0053 | JTAG_2059_ALL,           0,  0 },
	{ 0x0054 | JTAG_2059_ALL,           0,  0 },
	{ 0x0055 | JTAG_2059_ALL,        0x88,  0 },
	{ 0x0056 | JTAG_2059_ALL,           0,  0 },
	{ 0x0057 | JTAG_2059_ALL,        0x1f,  0 },
	{ 0x0058 | JTAG_2059_ALL,        0x20,  0 },
	{ 0x0059 | JTAG_2059_ALL,         0x1,  0 },
	{ 0x005A | JTAG_2059_ALL,         0x3,  1 },
	{ 0x005B | JTAG_2059_ALL,        0x70,  0 },
	{ 0x005C | JTAG_2059_ALL,           0,  0 },
	{ 0x005D | JTAG_2059_ALL,           0,  0 },
	{ 0x005E | JTAG_2059_ALL,        0x33,  0 },
	{ 0x005F | JTAG_2059_ALL,         0xf,  0 },
	{ 0x0060 | JTAG_2059_ALL,         0xf,  0 },
	{ 0x0061 | JTAG_2059_ALL,           0,  0 },
	{ 0x0062 | JTAG_2059_ALL,        0x11,  0 },
	{ 0x0063 | JTAG_2059_ALL,           0,  0 },
	{ 0x0064 | JTAG_2059_ALL,        0x7e,  0 },
	{ 0x0065 | JTAG_2059_ALL,        0x3f,  0 },
	{ 0x0066 | JTAG_2059_ALL,        0x7f,  0 },
	{ 0x0067 | JTAG_2059_ALL,        0x78,  0 },
	{ 0x0068 | JTAG_2059_ALL,        0x58,  0 },
	{ 0x0069 | JTAG_2059_ALL,        0x88,  0 },
	{ 0x006A | JTAG_2059_ALL,         0x8,  0 },
	{ 0x006B | JTAG_2059_ALL,         0xf,  0 },
	{ 0x006C | JTAG_2059_ALL,        0xbc,  0 },
	{ 0x006D | JTAG_2059_ALL,         0x8,  0 },
	{ 0x006E | JTAG_2059_ALL,        0x60,  0 },
	{ 0x006F | JTAG_2059_ALL,        0x13,  0 },
	{ 0x0070 | JTAG_2059_ALL,        0x70,  0 },
	{ 0x0071 | JTAG_2059_ALL,           0,  0 },
	{ 0x0072 | JTAG_2059_ALL,           0,  0 },
	{ 0x0073 | JTAG_2059_ALL,           0,  0 },
	{ 0x0074 | JTAG_2059_ALL,        0x33,  0 },
	{ 0x0075 | JTAG_2059_ALL,        0x13,  0 },
	{ 0x0076 | JTAG_2059_ALL,         0xf,  0 },
	{ 0x0077 | JTAG_2059_ALL,        0x11,  0 },
	{ 0x0078 | JTAG_2059_ALL,        0x3c,  0 },
	{ 0x0079 | JTAG_2059_ALL,         0x1,  1 },
	{ 0x007A | JTAG_2059_ALL,         0xa,  0 },
	{ 0x007B | JTAG_2059_ALL,        0x9d,  0 },
	{ 0x007C | JTAG_2059_ALL,         0xa,  0 },
	{ 0x007D | JTAG_2059_ALL,           0,  0 },
	{ 0x007E | JTAG_2059_ALL,        0x40,  0 },
	{ 0x007F | JTAG_2059_ALL,        0x40,  0 },
	{ 0x0080 | JTAG_2059_ALL,        0x88,  0 },
	{ 0x0081 | JTAG_2059_ALL,        0x10,  0 },
	{ 0x0082 | JTAG_2059_ALL,        0x70,  1 },
	{ 0x0083 | JTAG_2059_ALL,           0,  1 },
	{ 0x0084 | JTAG_2059_ALL,        0x70,  1 },
	{ 0x0085 | JTAG_2059_ALL,           0,  0 },
	{ 0x0086 | JTAG_2059_ALL,           0,  0 },
	{ 0x0087 | JTAG_2059_ALL,        0x10,  0 },
	{ 0x0088 | JTAG_2059_ALL,        0x55,  0 },
	{ 0x0089 | JTAG_2059_ALL,        0x3f,  0 },
	{ 0x008A | JTAG_2059_ALL,        0x36,  0 },
	{ 0x008B | JTAG_2059_ALL,           0,  0 },
	{ 0x008C | JTAG_2059_ALL,           0,  0 },
	{ 0x008D | JTAG_2059_ALL,           0,  0 },
	{ 0x008E | JTAG_2059_ALL,        0x87,  0 },
	{ 0x008F | JTAG_2059_ALL,        0x11,  0 },
	{ 0x0090 | JTAG_2059_ALL,           0,  0 },
	{ 0x0091 | JTAG_2059_ALL,        0x33,  0 },
	{ 0x0092 | JTAG_2059_ALL,        0x88,  0 },
	{ 0x0093 | JTAG_2059_ALL,           0,  0 },
	{ 0x0094 | JTAG_2059_ALL,        0x87,  0 },
	{ 0x0095 | JTAG_2059_ALL,        0x11,  0 },
	{ 0x0096 | JTAG_2059_ALL,           0,  0 },
	{ 0x0097 | JTAG_2059_ALL,        0x33,  0 },
	{ 0x0098 | JTAG_2059_ALL,        0x88,  0 },
	{ 0x0099 | JTAG_2059_ALL,        0x21,  0 },
	{ 0x009A | JTAG_2059_ALL,        0x7f,  1 },
	{ 0x009B | JTAG_2059_ALL,        0x44,  0 },
	{ 0x009C | JTAG_2059_ALL,        0x8c,  0 },
	{ 0x009D | JTAG_2059_ALL,        0x6c,  0 },
	{ 0x009E | JTAG_2059_ALL,        0x22,  0 },
	{ 0x009F | JTAG_2059_ALL,        0xbe,  0 },
	{ 0x00A0 | JTAG_2059_ALL,        0x55,  0 },
	{ 0x00A1 | JTAG_2059_ALL,         0xc,  0 },
	{ 0x00A2 | JTAG_2059_ALL,        0xaa,  0 },
	{ 0x00A3 | JTAG_2059_ALL,         0x2,  0 },
	{ 0x00A4 | JTAG_2059_ALL,           0,  0 },
	{ 0x00A5 | JTAG_2059_ALL,        0x10,  0 },
	{ 0x00A6 | JTAG_2059_ALL,         0x1,  0 },
	{ 0x00A7 | JTAG_2059_ALL,           0,  0 },
	{ 0x00A8 | JTAG_2059_ALL,           0,  0 },
	{ 0x00A9 | JTAG_2059_ALL,        0x80,  0 },
	{ 0x00AA | JTAG_2059_ALL,        0x60,  0 },
	{ 0x00AB | JTAG_2059_ALL,        0x44,  0 },
	{ 0x00AC | JTAG_2059_ALL,        0x55,  0 },
	{ 0x00AD | JTAG_2059_ALL,         0x1,  0 },
	{ 0x00AE | JTAG_2059_ALL,        0x55,  0 },
	{ 0x00AF | JTAG_2059_ALL,         0x1,  0 },
	{ 0x00B0 | JTAG_2059_ALL,         0x5,  0 },
	{ 0x00B1 | JTAG_2059_ALL,        0x55,  0 },
	{ 0x00B2 | JTAG_2059_ALL,        0x55,  0 },
	{ 0x00B3 | JTAG_2059_ALL,           0,  0 },
	{ 0x00B4 | JTAG_2059_ALL,           0,  0 },
	{ 0x00B5 | JTAG_2059_ALL,           0,  0 },
	{ 0x00B6 | JTAG_2059_ALL,        0x10,  1 },
	{ 0x00B7 | JTAG_2059_ALL,           0,  0 },
	{ 0x00B8 | JTAG_2059_ALL,           0,  0 },
	{ 0x00B9 | JTAG_2059_ALL,           0,  0 },
	{ 0x00BA | JTAG_2059_ALL,           0,  0 },
	{ 0x00BB | JTAG_2059_ALL,           0,  0 },
	{ 0x00BC | JTAG_2059_ALL,           0,  0 },
	{ 0x00BD | JTAG_2059_ALL,           0,  0 },
	{ 0x00BE | JTAG_2059_ALL,           0,  0 },
	{ 0x00BF | JTAG_2059_CR2,           0,  0 },
	{ 0x00C0 | JTAG_2059_CR0,        0x5e,  0 },
	{ 0x00C1 | JTAG_2059_ALL,         0xc,  0 },
	{ 0x00C2 | JTAG_2059_ALL,         0xc,  0 },
	{ 0x00C3 | JTAG_2059_ALL,         0xc,  0 },
	{ 0x00C4 | JTAG_2059_ALL,           0,  0 },
	{ 0x00C5 | JTAG_2059_ALL,        0x2b,  0 },
	{ 0x013C | JTAG_2059_CR0,        0x15,  0 },
	{ 0x013D | JTAG_2059_CR0,         0xf,  0 },
	{ 0x013E | JTAG_2059_CR0,           0,  0 },
	{ 0x013F | JTAG_2059_CR0,           0,  0 },
	{ 0x0140 | JTAG_2059_CR0,           0,  0 },
	{ 0x0141 | JTAG_2059_CR0,           0,  0 },
	{ 0x0142 | JTAG_2059_CR0,           0,  0 },
	{ 0x0143 | JTAG_2059_CR0,           0,  0 },
	{ 0x0144 | JTAG_2059_CR0,           0,  0 },
	{ 0x0145 | JTAG_2059_CR2,           0,  0 },
	{ 0x0146 | JTAG_2059_ALL,           0,  0 },
	{ 0x0147 | JTAG_2059_ALL,           0,  0 },
	{ 0x0148 | JTAG_2059_ALL,           0,  0 },
	{ 0x0149 | JTAG_2059_ALL,           0,  0 },
	{ 0x014A | JTAG_2059_ALL,           0,  0 },
	{ 0x014B | JTAG_2059_ALL,           0,  0 },
	{ 0x014C | JTAG_2059_CR0,           0,  0 },
	{ 0x014D | JTAG_2059_CR0,           0,  0 },
	{ 0x014E | JTAG_2059_CR0,           0,  0 },
	{ 0x014F | JTAG_2059_ALL,           0,  0 },
	{ 0x0150 | JTAG_2059_ALL,           0,  0 },
	{ 0x0151 | JTAG_2059_ALL,        0x77,  0 },
	{ 0x0152 | JTAG_2059_ALL,        0x77,  0 },
	{ 0x0153 | JTAG_2059_ALL,        0x77,  0 },
	{ 0x0154 | JTAG_2059_ALL,        0x77,  0 },
	{ 0x0155 | JTAG_2059_ALL,           0,  0 },
	{ 0x0156 | JTAG_2059_ALL,         0x3,  0 },
	{ 0x0157 | JTAG_2059_ALL,        0x37,  0 },
	{ 0x0158 | JTAG_2059_ALL,         0x3,  0 },
	{ 0x0159 | JTAG_2059_ALL,           0,  0 },
	{ 0x015A | JTAG_2059_ALL,        0x21,  0 },
	{ 0x015B | JTAG_2059_ALL,        0x21,  0 },
	{ 0x015C | JTAG_2059_ALL,           0,  0 },
	{ 0x015D | JTAG_2059_ALL,        0xaa,  0 },
	{ 0x015E | JTAG_2059_ALL,           0,  0 },
	{ 0x015F | JTAG_2059_ALL,        0xaa,  0 },
	{ 0x0160 | JTAG_2059_ALL,           0,  0 },
	{ 0x0172 | JTAG_2059_ALL,         0x2,  0 },
	{ 0x0173 | JTAG_2059_ALL,         0xf,  0 },
	{ 0x0174 | JTAG_2059_ALL,         0xf,  0 },
	{ 0x0175 | JTAG_2059_ALL,           0,  0 },
	{ 0x0176 | JTAG_2059_ALL,           0,  0 },
	{ 0x0177 | JTAG_2059_ALL,           0,  0 },
	{ 0x017E | JTAG_2059_ALL,        0x84,  0 },
	{ 0x017F | JTAG_2059_CR0,        0x60,  0 },
	{ 0x0180 | JTAG_2059_ALL,        0x47,  0 },
	{ 0x0182 | JTAG_2059_ALL,           0,  0 },
	{ 0x0183 | JTAG_2059_ALL,           0,  0 },
	{ 0x0184 | JTAG_2059_ALL,           0,  0 },
	{ 0x0185 | JTAG_2059_ALL,           0,  0 },
	{ 0x0186 | JTAG_2059_ALL,           0,  0 },
	{ 0x0187 | JTAG_2059_ALL,           0,  0 },
	{ 0x0188 | JTAG_2059_ALL,         0x5,  1 },
	{ 0x0189 | JTAG_2059_ALL,           0,  0 },
	{ 0x018A | JTAG_2059_ALL,           0,  0 },
	{ 0x018B | JTAG_2059_ALL,           0,  0 },
	{ 0x018C | JTAG_2059_ALL,           0,  0 },
	{ 0x018D | JTAG_2059_ALL,           0,  0 },
	{ 0x018E | JTAG_2059_ALL,           0,  0 },
	{ 0x018F | JTAG_2059_ALL,           0,  0 },
	{ 0x019A | JTAG_2059_ALL,           0,  0 },
	{ 0x019B | JTAG_2059_CR2,           0,  0 },
	{ 0x019C | JTAG_2059_ALL,           0,  0 },
	{ 0x019D | JTAG_2059_ALL,         0x2,  0 },
	{ 0x019E | JTAG_2059_ALL,         0x1,  0 },
	{ 0x019F | JTAG_2059_ALL,           0,  0 },
	{ 0x01A0 | JTAG_2059_ALL,         0x1,  0 },
	{ 0xFFFF,                           0,  0 }
};

/* %%%%%% tx gain table */

static uint32 tpc_txgain_htphy_rev0_2g[] = {
	0x10f90040, 0x10e10040, 0x10e1003c, 0x10c9003d,
	0x10b9003c, 0x10a9003d, 0x10a1003c, 0x1099003b,
	0x1091003b, 0x1089003a, 0x1081003a, 0x10790039,
	0x10710039, 0x1069003a, 0x1061003b, 0x1059003d,
	0x1051003f, 0x10490042, 0x1049003e, 0x1049003b,
	0x1041003e, 0x1041003b, 0x1039003e, 0x1039003b,
	0x10390038, 0x10390035, 0x1031003a, 0x10310036,
	0x10310033, 0x1029003a, 0x10290037, 0x10290034,
	0x10290031, 0x10210039, 0x10210036, 0x10210033,
	0x10210030, 0x1019003c, 0x10190039, 0x10190036,
	0x10190033, 0x10190030, 0x1019002d, 0x1019002b,
	0x10190028, 0x1011003a, 0x10110036, 0x10110033,
	0x10110030, 0x1011002e, 0x1011002b, 0x10110029,
	0x10110027, 0x10110024, 0x10110022, 0x10110020,
	0x1011001f, 0x1011001d, 0x1009003a, 0x10090037,
	0x10090034, 0x10090031, 0x1009002e, 0x1009002c,
	0x10090029, 0x10090027, 0x10090025, 0x10090023,
	0x10090021, 0x1009001f, 0x1009001d, 0x1009001b,
	0x1009001a, 0x10090018, 0x10090017, 0x10090016,
	0x10090015, 0x10090013, 0x10090012, 0x10090011,
	0x10090010, 0x1009000f, 0x1009000f, 0x1009000e,
	0x1009000d, 0x1009000c, 0x1009000c, 0x1009000b,
	0x1009000a, 0x1009000a, 0x10090009, 0x10090009,
	0x10090008, 0x10090008, 0x10090007, 0x10090007,
	0x10090007, 0x10090006, 0x10090006, 0x10090005,
	0x10090005, 0x10090005, 0x10090005, 0x10090004,
	0x10090004, 0x10090004, 0x10090004, 0x10090003,
	0x10090003, 0x10090003, 0x10090003, 0x10090003,
	0x10090003, 0x10090002, 0x10090002, 0x10090002,
	0x10090002, 0x10090002, 0x10090002, 0x10090002,
	0x10090002, 0x10090002, 0x10090001, 0x10090001,
	0x10090001, 0x10090001, 0x10090001, 0x10090001
};

static uint32 tpc_txgain_htphy_rev0_5g[] = {
	0x2f690044, 0x2f690040, 0x2f69003c, 0x2f690039,
	0x2f690036, 0x2f690033, 0x2f690030, 0x2e690039,
	0x2e690036, 0x2e690033, 0x2d69003a, 0x2d690037,
	0x2d690034, 0x2d690031, 0x2c69003b, 0x2c690038,
	0x2c690035, 0x2b690039, 0x2b690036, 0x2b690033,
	0x2b690030, 0x2a690039, 0x2a690036, 0x2a690033,
	0x2969003c, 0x29690038, 0x29690035, 0x29690032,
	0x2869003a, 0x28690036, 0x28690033, 0x28690030,
	0x28590035, 0x28590032, 0x2859002f, 0x28490035,
	0x28490032, 0x2849002f, 0x28390032, 0x2839002f,
	0x2839002d, 0x2839002a, 0x28290035, 0x28290032,
	0x2829002f, 0x2829002c, 0x2829002a, 0x28290027,
	0x28290025, 0x28190035, 0x28190032, 0x2819002f,
	0x2819002d, 0x2819002a, 0x28190028, 0x28190026,
	0x28190024, 0x28190022, 0x28190020, 0x2819001e,
	0x28090036, 0x28090033, 0x28090031, 0x2809002e,
	0x2809002b, 0x28090029, 0x28090027, 0x28090024,
	0x28090022, 0x28090020, 0x2809001f, 0x2809001d,
	0x2809001b, 0x2809001a, 0x28090018, 0x28090017,
	0x28090016, 0x28090014, 0x28090013, 0x28090012,
	0x28090011, 0x28090010, 0x2809000f, 0x2809000e,
	0x2809000e, 0x2809000d, 0x2809000c, 0x2809000c,
	0x2809000b, 0x2809000a, 0x2809000a, 0x28090009,
	0x28090009, 0x28090008, 0x28090008, 0x28090007,
	0x28090007, 0x28090006, 0x28090006, 0x28090006,
	0x28090005, 0x28090005, 0x28090005, 0x28090005,
	0x28090004, 0x28090004, 0x28090004, 0x28090004,
	0x28090003, 0x28090003, 0x28090003, 0x28090003,
	0x28090003, 0x28090003, 0x28090002, 0x28090002,
	0x28090002, 0x28090002, 0x28090002, 0x28090002,
	0x28090002, 0x28090002, 0x28090002, 0x28090001,
	0x28090001, 0x28090001, 0x28090001, 0x28090001
};

static uint32 tpc_txgain_htphy_rev0_5g_hpa[] = {
	0x2f3c003d, 0x2e3c003d, 0x2e3c0037, 0x2e3c0031,
	0x2e3c002d, 0x2d3c0031, 0x2d3c002e, 0x2d3c002b,
	0x2d2c0033, 0x2c2c003b, 0x2c2c0038, 0x2c2c0035,
	0x2c2c0032, 0x2c2c002f, 0x2c2c002c, 0x2c1c003c,
	0x2c1c0039, 0x2c1c0036, 0x2c1c0034, 0x2c1c0030,
	0x2c1c002e, 0x2c1c002b, 0x2c1c0029, 0x2b1c002d,
	0x2b1c002b, 0x2b1b0032, 0x2b1b002f, 0x2b1b002d,
	0x2a1b0035, 0x2a1b0032, 0x2a1b002f, 0x2a1b002d,
	0x2a1a0036, 0x2a1a0033, 0x2a1a0030, 0x2a1a002d,
	0x2a1a002b, 0x291a0032, 0x291a002f, 0x291a002d,
	0x291a002b, 0x2919003d, 0x29190039, 0x29190036,
	0x29190033, 0x2919002f, 0x28190038, 0x28190035,
	0x28190032, 0x2819002f, 0x2819002d, 0x2819002a,
	0x28190028, 0x28190026, 0x28190024, 0x28190022,
	0x28190020, 0x2819001e, 0x28090036, 0x28090033,
	0x28090031, 0x2809002e, 0x2809002b, 0x28090029,
	0x28090027, 0x28090024, 0x28090022, 0x28090020,
	0x2809001f, 0x2809001d, 0x2809001b, 0x2809001a,
	0x28090018, 0x28090017, 0x28090016, 0x28090014,
	0x28090013, 0x28090012, 0x28090011, 0x28090010,
	0x2809000f, 0x2809000e, 0x2809000e, 0x2809000d,
	0x2809000c, 0x2809000c, 0x2809000b, 0x2809000a,
	0x2809000a, 0x28090009, 0x28090009, 0x28090008,
	0x28090008, 0x28090007, 0x28090007, 0x28090006,
	0x28090006, 0x28090006, 0x28090005, 0x28090005,
	0x28090005, 0x28090005, 0x28090004, 0x28090004,
	0x28090004, 0x28090004, 0x28090003, 0x28090003,
	0x28090003, 0x28090003, 0x28090003, 0x28090003,
	0x28090002, 0x28090002, 0x28090002, 0x28090002,
	0x28090002, 0x28090002, 0x28090002, 0x28090002,
	0x28090002, 0x28090001, 0x28090001, 0x28090001,
	0x28090001, 0x28090001, 0x28090001, 0x28090001
};

static const char BCMATTACHDATA(rstr_eu_edthresh2g)[] = "eu_edthresh2g";
static const char BCMATTACHDATA(rstr_eu_edthresh5g)[] = "eu_edthresh5g";

typedef struct {
	uint8 percent;
	uint8 g_env;
} htphy_txiqcal_ladder_t;

typedef struct {
	uint8 nwords;
	uint8 offs;
	uint8 boffs;
} htphy_coeff_access_t;

/* defs for iqlo cal */
enum {  /* mode selection for reading/writing tx iqlo cal coefficients */
	TB_START_COEFFS_AB, TB_START_COEFFS_D, TB_START_COEFFS_E, TB_START_COEFFS_F,
	TB_BEST_COEFFS_AB,  TB_BEST_COEFFS_D,  TB_BEST_COEFFS_E,  TB_BEST_COEFFS_F,
	TB_OFDM_COEFFS_AB,  TB_OFDM_COEFFS_D,  TB_BPHY_COEFFS_AB,  TB_BPHY_COEFFS_D,
	PI_INTER_COEFFS_AB, PI_INTER_COEFFS_D, PI_INTER_COEFFS_E, PI_INTER_COEFFS_F,
	PI_FINAL_COEFFS_AB, PI_FINAL_COEFFS_D, PI_FINAL_COEFFS_E, PI_FINAL_COEFFS_F
};

#define CAL_TYPE_IQ                 0
#define CAL_TYPE_LOFT_DIG           2
#define CAL_TYPE_LOFT_ANA_FINE      3
#define CAL_TYPE_LOFT_ANA_COARSE    4

#define CAL_COEFF_READ    0
#define CAL_COEFF_WRITE   1
#define MPHASE_TXCAL_CMDS_PER_PHASE  2 /* number of tx iqlo cal commands per phase in mphase cal */

#define IQTBL_CACHE_COOKIE_OFFSET	95
#define TXCAL_CACHE_VALID		0xACDC

/* %%%%%% function declaration */

static void wlc_phy_watchdog_htphy(phy_info_t *pi);
static void wlc_phy_get_txgain_settings_by_index(phy_info_t *pi, txgain_setting_t *txgain_settings,
	int8 txpwrindex);
static void wlc_phy_write_txmacreg_htphy(wlc_phy_t *ppi, uint16 holdoff, uint16 delay);
static void wlc_phy_txpwrctrl_config_htphy(phy_info_t *pi);
static void wlc_phy_resetcca_htphy(phy_info_t *pi);
static void wlc_phy_workarounds_htphy(phy_info_t *pi);
static void wlc_phy_subband_cust_htphy(phy_info_t *pi);
static void wlc_phy_extlna_war_htphy(phy_info_t *pi);
static void wlc_phy_subband_cust_rev1_pciedual_old_htphy(phy_info_t *pi);
static void wlc_phy_subband_cust_rev1_pciedual_old_2g_htphy(phy_info_t *pi, uint16 fc);
static void wlc_phy_subband_cust_rev1_pciedual_old_5g_htphy(phy_info_t *pi, uint16 fc);

static void wlc_phy_subband_cust_rev1_pciedual_htphy(phy_info_t *pi);
static void wlc_phy_subband_cust_rev1_pciedual_2g_htphy(phy_info_t *pi, uint16 fc);
static void wlc_phy_subband_cust_rev1_pciedual_5g_htphy(phy_info_t *pi, uint16 fc);

static void wlc_phy_subband_cust_rev1_htphy(phy_info_t *pi);
static void wlc_phy_subband_cust_rev1_2g_htphy(phy_info_t *pi, uint16 fc);
static void wlc_phy_subband_cust_rev1_5g_htphy(phy_info_t *pi, uint16 fc);

static void wlc_phy_subband_cust_rev0_htphy(phy_info_t *pi);
static void wlc_phy_subband_cust_rev0_2g_htphy(phy_info_t *pi, uint16 fc);
static void wlc_phy_subband_cust_rev0_5g_htphy(phy_info_t *pi, uint16 fc);

static void wlc_phy_subband_cust_rev1_mch5_htphy(phy_info_t *pi);
static void wlc_phy_subband_cust_rev1_mch5_2g_htphy(phy_info_t *pi, uint16 fc);
static void wlc_phy_subband_cust_rev1_mch5_5g_htphy(phy_info_t *pi, uint16 fc);

static void wlc_phy_subband_cust_rev1_cs_htphy(phy_info_t *pi);
static void wlc_phy_subband_cust_rev1_cs_2g_htphy(phy_info_t *pi, uint16 fc);
static void wlc_phy_subband_cust_rev1_cs_5g_htphy(phy_info_t *pi, uint16 fc);

static void wlc_phy_subband_cust_rev1_x33_htphy(phy_info_t *pi);
static void wlc_phy_subband_cust_rev1_x33_2g_htphy(phy_info_t *pi, uint16 fc);
static void wlc_phy_subband_cust_rev1_x33_5g_htphy(phy_info_t *pi, uint16 fc);

static void wlc_phy_subband_cust_rev1_asus_htphy(phy_info_t *pi);
static void wlc_phy_subband_cust_rev1_asus_2g_htphy(phy_info_t *pi, uint16 fc);
static void wlc_phy_subband_cust_rev1_asus_5g_htphy(phy_info_t *pi, uint16 fc);

static void wlc_phy_subband_cust_rev1_4706nrh_htphy(phy_info_t *pi);
static void wlc_phy_subband_cust_rev1_4706nrh_2g_htphy(phy_info_t *pi, uint16 fc);
static void wlc_phy_subband_cust_rev1_4706nrh_5g_htphy(phy_info_t *pi, uint16 fc);

static void wlc_phy_pcieingress_war_htphy(phy_info_t *pi);
static void wlc_phy_radio2059_reset(phy_info_t *pi);
static void wlc_phy_radio2059_vcocal(phy_info_t *pi);
static void wlc_phy_radio2059_save_rcal_cache(phy_info_t *pi, uint16 rval);
static void wlc_phy_radio2059_save_rccal_cache(phy_info_t *pi, uint16 bcap,
                                               uint16 scap, uint16 hpc);
static void wlc_phy_radio2059_restore_rcal_rccal_cache(phy_info_t *pi);
static void wlc_phy_radio2059_rcal(phy_info_t *pi);
static void wlc_phy_radio2059_rccal(phy_info_t *pi);
static void wlc_phy_radio2059_init(phy_info_t *pi);
static bool wlc_phy_chan2freq_htphy(phy_info_t *pi, uint channel, int *f,
                                    chan_info_htphy_radio2059_t **t);
static void wlc_phy_chanspec_setup_htphy(phy_info_t *pi, chanspec_t chans,
                                         const htphy_sfo_cfg_t *c);

static void wlc_phy_txlpfbw_htphy(phy_info_t *pi);
static void wlc_phy_spurwar_htphy(phy_info_t *pi);

static int  wlc_phy_cal_txiqlo_htphy(phy_info_t *pi, uint8 searchmode, uint8 mphase);
static void wlc_phy_cal_txiqlo_coeffs_htphy(phy_info_t *pi,
	uint8 rd_wr, uint16 *coeffs, uint8 select, uint8 core);
static void wlc_phy_precal_txgain_htphy(phy_info_t *pi, txgain_setting_t *target_gains);
static void wlc_phy_txcal_txgain_setup_htphy(phy_info_t *pi, txgain_setting_t *txcal_txgain,
	txgain_setting_t *orig_txgain);
static void wlc_phy_txcal_txgain_cleanup_htphy(phy_info_t *pi, txgain_setting_t *orig_txgain);
static void wlc_phy_txcal_radio_setup_htphy(phy_info_t *pi);
static void wlc_phy_txcal_radio_cleanup_htphy(phy_info_t *pi);
static void wlc_phy_txcal_phy_setup_htphy(phy_info_t *pi);
static void wlc_phy_txcal_phy_cleanup_htphy(phy_info_t *pi);
static void wlc_phy_cal_txiqlo_update_ladder_htphy(phy_info_t *pi, uint16 bbmult);

static void wlc_phy_extpa_set_tx_digi_filts_htphy(phy_info_t *pi);
static void wlc_phy_tx_digi_filts_htphy_war(phy_info_t *pi, bool init);
static void wlc_phy_clip_det_htphy(phy_info_t *pi, uint8 write, uint16 *vals);

static bool wlc_phy_srom_read_htphy(phy_info_t *pi);
static int wlc_phy_txpower_core_offset_set_htphy(phy_info_t *pi,
        struct phy_txcore_pwr_offsets *offsets);
static int wlc_phy_txpower_core_offset_get_htphy(phy_info_t *pi,
	struct phy_txcore_pwr_offsets *offsets);
static void wlc_phy_txpwr_fixpower_htphy(phy_info_t *pi);
static void wlc_phy_tssi_radio_setup_htphy(phy_info_t *pi, uint8 core_mask);
static void wlc_phy_tssi_pdetrange_cust_htphy(phy_info_t *pi);
static void wlc_phy_txpwrctrl_idle_tssi_poll_auxadc_htphy(phy_info_t *pi, int32 *auxadc_buf,
	uint8 nsamps, uint8 mode);
static void wlc_phy_txpwrctrl_idle_tssi_meas_htphy(phy_info_t *pi);
static void wlc_phy_txpwrctrl_pwr_setup_htphy(phy_info_t *pi);
static void wlc_phy_get_txgain_settings_by_index(phy_info_t *pi, txgain_setting_t *txgain_settings,
                                                 int8 txpwrindex);
static bool wlc_phy_txpwrctrl_ison_htphy(phy_info_t *pi);
static void wlc_phy_txpwrctrl_set_idle_tssi_htphy(phy_info_t *pi, int8 idle_tssi, uint8 core);
static void wlc_phy_txpower_recalc_idle_tssi_htphy(phy_info_t *pi);
#if defined(PHYCAL_CACHING)
static bool wlc_phy_txpower_idle_tssi_cache_restore_htphy(phy_info_t *pi);
#endif // endif
static void wlc_phy_txpwrctrl_set_target_htphy(phy_info_t *pi, uint8 pwr_qtrdbm, uint8 core);
static uint8 wlc_phy_txpwrctrl_get_cur_index_htphy(phy_info_t *pi, uint8 core);
static void wlc_phy_txpwrctrl_set_cur_index_htphy(phy_info_t *pi, uint8 idx, uint8 core);

static uint16 wlc_phy_gen_load_samples_htphy(phy_info_t *pi, uint32 f_kHz, uint16 max_val,
                                             uint8 dac_test_mode);
static void wlc_phy_loadsampletable_htphy(phy_info_t *pi, math_cint32 *tone_buf, uint16 num_samps);
static void wlc_phy_init_lpf_sampleplay(phy_info_t *pi);
static void wlc_phy_runsamples_htphy(phy_info_t *pi, uint16 n, uint16 lps, uint16 wait, uint8 iq,
                                     uint8 dac_test_mode);

static void wlc_phy_rx_iq_comp_htphy(phy_info_t *pi, uint8 write, phy_iq_comp_t *pcomp, uint8);
static void wlc_phy_cal_dac_adc_decouple_war_htphy(phy_info_t *pi, bool do_not_undo);
static void wlc_phy_rx_iq_est_loopback_htphy(phy_info_t *pi, phy_iq_est_t *est, uint16 num_samps,
                                             uint8 wait_time, uint8 wait_for_crs);
static int  wlc_phy_calc_ab_from_iq_htphy(phy_iq_est_t *est, phy_iq_comp_t *comp);
static void wlc_phy_calc_rx_iq_comp_htphy(phy_info_t *pi, uint16 num_samps, uint8 core_mask);
static void wlc_phy_rxcal_radio_config_htphy(phy_info_t *pi, bool setup_not_cleanup, uint8 rx_core);
static void wlc_phy_rxcal_phy_config_htphy(phy_info_t *pi, bool setup_not_cleanup, uint8 rx_core);
static void wlc_phy_rxcal_gainctrl_htphy(phy_info_t *pi, uint8 rx_core);
static int  wlc_phy_cal_rxiq_htphy(phy_info_t *pi, bool debug);

static int wlc_phy_aci_scan_htphy(phy_info_t *pi);
static int wlc_phy_aci_scan_iqbased_htphy(phy_info_t *pi);
static void wlc_phy_aci_pwr_upd_htphy(phy_info_t *pi, int aci_pwr);

static void wlc_phy_aci_noise_shared_reset_htphy(phy_info_t *pi);

static void wlc_phy_noisemode_set_htphy(phy_info_t *pi, bool raise);
static void wlc_phy_bphynoisemode_set_htphy(phy_info_t *pi, bool raise);
static void wlc_phy_bphynoise_update_lut_htphy(phy_info_t *pi, bphy_desense_info_t *new_lut,
                                               uint16 new_lut_size, int16 new_min_sensitivity,
                                               uint16 *new_initgain);

static void wlc_phy_aci_sw_set_htphy(phy_info_t *pi, bool enable);
static void wlc_phy_noise_sw_set_htphy(phy_info_t *pi);

static void wlc_phy_aci_hw_set_htphy(phy_info_t *pi, bool enable, int aci_pwr);
static void wlc_phy_aci_noise_shared_hw_set_htphy(phy_info_t *pi, bool enable,
        bool from_aci_call);
static void wlc_phy_bphynoise_hw_set_htphy(phy_info_t *pi, uint16 new_index, bool lut_changed);

static void wlc_phy_noise_adj_thresholds_htphy(phy_info_t *pi, bool raise);
static void wlc_phy_noise_update_crsminpwr_htphy(phy_info_t *pi);
static int16 wlc_phy_bphynoise_adj_thresholds_htphy(phy_info_t *pi, bool raise);
static void wlc_phy_noise_limit_crsmin_htphy(phy_info_t *pi);
static void wlc_phy_bphynoise_limit_crsmin_htphy(phy_info_t *pi, int16 *desense_index);
static void wlc_phy_aci_pwr_upd_htphy(phy_info_t *pi, int aci_pwr);
static void wlc_phy_noisemode_glitch_chk_adj_htphy(phy_info_t *pi, uint16 noise_enter_th,
        uint16 noise_glitch_th_up, uint16 noise_glitch_th_dn);
static void wlc_phy_bphynoisemode_glitch_chk_adj_htphy(phy_info_t *pi);

static void wlc_phy_aci_noise_store_values_htphy(phy_info_t *pi);

static void wlc_phy_enable_extlna_extpa_htphy(phy_info_t *pi);

static int16 wlc_phy_rxgaincode_to_dB_htphy(phy_info_t *pi, uint16 gain_code);
static void wlc_phy_BT_SwControl_setup_htphy(phy_info_t *pi, chanspec_t chanspec);

static void
wlc_phy_poll_auxadc_htphy(phy_info_t *pi, uint8 signal_type, int32 *auxadc_buf,
	uint8 nsamps, uint8 mode);

int8 wlc_phy_txpwr_paparam_adj_htphy(phy_info_t *pi, uint8 core, uint32 idx);
#if defined(PHYCAL_CACHING)
static void wlc_phy_cal_cache_htphy(wlc_phy_t *pih);
static void wlc_phy_cal_cache_dbg_htphy(wlc_phy_t *pih, ch_calcache_t *ctx);
#endif // endif
static void wlc_phy_set_srom_eu_edthresh_htphy(phy_info_t *pi);

/* %%%%%% function implementation */

/* Start phy init code from d11procs.tcl */
/* Initialize the bphy in an htphy */
static void
WLBANDINITFN(wlc_phy_bphy_init_htphy)(phy_info_t *pi)
{
	uint16	addr, val;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	ASSERT(ISHTPHY(pi));

	/* RSSI LUT is now a memory and must therefore be initialized */
	val = 0x1e1f;
	for (addr = (HTPHY_TO_BPHY_OFF + BPHY_RSSI_LUT);
	     addr <= (HTPHY_TO_BPHY_OFF + BPHY_RSSI_LUT_END); addr++) {
		phy_utils_write_phyreg(pi, addr, val);
		if (addr == (HTPHY_TO_BPHY_OFF + 0x97))
			val = 0x3e3f;
		else
			val -= 0x0202;
	}

	if (ISSIM_ENAB(pi->sh->sih)) {
		/* only for use on QT */

		/* CRS thresholds */
		phy_utils_write_phyreg(pi, HTPHY_TO_BPHY_OFF + BPHY_PHYCRSTH, 0x3206);

		/* RSSI thresholds */
		phy_utils_write_phyreg(pi, HTPHY_TO_BPHY_OFF + BPHY_RSSI_TRESH, 0x281e);

		/* LNA gain range */
		phy_utils_or_phyreg(pi, HTPHY_TO_BPHY_OFF + BPHY_LNA_GAIN_RANGE, 0x1a);
	} else	{
		/* kimmer - add change from 0x667 to x668 very slight improvement */
		phy_utils_write_phyreg(pi, HTPHY_TO_BPHY_OFF + BPHY_STEP, 0x668);
	}
}

void
wlc_phy_scanroam_cache_cal_htphy(phy_info_t *pi, bool set)
{
	uint16 tbl_cookie;
	uint16 ab_int[2];
	uint8 core;

	PHY_TRACE(("wl%d: %s: in scan/roam set %d\n", pi->sh->unit, __FUNCTION__, set));
	wlc_phy_table_read_htphy(pi, HTPHY_TBL_ID_IQLOCAL, 1,
		IQTBL_CACHE_COOKIE_OFFSET, 16, &tbl_cookie);

	if (set) {
		if (tbl_cookie == TXCAL_CACHE_VALID) {
			PHY_CAL(("wl%d: %s: save the txcal for scan/roam\n",
				pi->sh->unit, __FUNCTION__));
			/* save the txcal to cache */
			FOREACH_CORE(pi, core) {
				wlc_phy_cal_txiqlo_coeffs_htphy(pi, CAL_COEFF_READ,
					ab_int, TB_OFDM_COEFFS_AB, core);
				pi->u.pi_htphy->txcal_cache[core].txa = ab_int[0];
				pi->u.pi_htphy->txcal_cache[core].txb = ab_int[1];
				wlc_phy_cal_txiqlo_coeffs_htphy(pi, CAL_COEFF_READ,
					&pi->u.pi_htphy->txcal_cache[core].txd,
					TB_OFDM_COEFFS_D, core);
				pi->u.pi_htphy->txcal_cache[core].txei =
				        (uint8)phy_utils_read_radioreg(pi,
					RADIO_2059_TX_LOFT_FINE_I(core));
				pi->u.pi_htphy->txcal_cache[core].txeq =
				        (uint8)phy_utils_read_radioreg(pi,
					RADIO_2059_TX_LOFT_FINE_Q(core));
				pi->u.pi_htphy->txcal_cache[core].txfi =
				        (uint8)phy_utils_read_radioreg(pi,
					RADIO_2059_TX_LOFT_COARSE_I(core));
				pi->u.pi_htphy->txcal_cache[core].txfq =
				        (uint8)phy_utils_read_radioreg(pi,
					RADIO_2059_TX_LOFT_COARSE_Q(core));
				pi->u.pi_htphy->txcal_cache[core].rxa =
					phy_utils_read_phyreg(pi, HTPHY_RxIQCompA(core));
				pi->u.pi_htphy->txcal_cache[core].rxb =
					phy_utils_read_phyreg(pi, HTPHY_RxIQCompB(core));
			}
			/* mark the cache as valid */
			pi->u.pi_htphy->txcal_cache_cookie = tbl_cookie;
		}
	} else {
		if (pi->u.pi_htphy->txcal_cache_cookie == TXCAL_CACHE_VALID &&
		   tbl_cookie != pi->u.pi_htphy->txcal_cache_cookie) {
			PHY_CAL(("wl%d: %s: restore the txcal after scan/roam\n",
				pi->sh->unit, __FUNCTION__));
			/* restore the txcal from cache */
			FOREACH_CORE(pi, core) {
				ab_int[0] = pi->u.pi_htphy->txcal_cache[core].txa;
				ab_int[1] = pi->u.pi_htphy->txcal_cache[core].txb;
				wlc_phy_cal_txiqlo_coeffs_htphy(pi, CAL_COEFF_WRITE,
					ab_int, TB_OFDM_COEFFS_AB, core);
				wlc_phy_cal_txiqlo_coeffs_htphy(pi, CAL_COEFF_WRITE,
					&pi->u.pi_htphy->txcal_cache[core].txd,
					TB_OFDM_COEFFS_D, core);
				phy_utils_write_radioreg(pi, RADIO_2059_TX_LOFT_FINE_I(core),
					pi->u.pi_htphy->txcal_cache[core].txei);
				phy_utils_write_radioreg(pi, RADIO_2059_TX_LOFT_FINE_Q(core),
					pi->u.pi_htphy->txcal_cache[core].txeq);
				phy_utils_write_radioreg(pi, RADIO_2059_TX_LOFT_COARSE_I(core),
					pi->u.pi_htphy->txcal_cache[core].txfi);
				phy_utils_write_radioreg(pi, RADIO_2059_TX_LOFT_COARSE_Q(core),
					pi->u.pi_htphy->txcal_cache[core].txfq);
				phy_utils_write_phyreg(pi, HTPHY_RxIQCompA(core),
					pi->u.pi_htphy->txcal_cache[core].rxa);
				phy_utils_write_phyreg(pi, HTPHY_RxIQCompB(core),
					pi->u.pi_htphy->txcal_cache[core].rxb);
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_IQLOCAL, 1,
					IQTBL_CACHE_COOKIE_OFFSET, 16,
					&pi->u.pi_htphy->txcal_cache_cookie);
			}
			pi->u.pi_htphy->txcal_cache_cookie = 0;
		}
	}
}

void
wlc_phy_table_write_htphy(phy_info_t *pi, uint32 id, uint32 len, uint32 offset, uint32 width,
                          const void *data)
{
	htphytbl_info_t tbl;
	/*
	 * PHY_TRACE(("wlc_phy_table_write_htphy, id %d, len %d, offset %d, width %d\n",
	 * 	id, len, offset, width));
	*/
	tbl.tbl_id = id;
	tbl.tbl_len = len;
	tbl.tbl_offset = offset;
	tbl.tbl_width = width;
	tbl.tbl_ptr = data;
	wlc_phy_write_table_htphy(pi, &tbl);
}

void
wlc_phy_table_read_htphy(phy_info_t *pi, uint32 id, uint32 len, uint32 offset, uint32 width,
                         void *data)
{
	htphytbl_info_t tbl;
	/*	PHY_TRACE(("wlc_phy_table_read_htphy, id %d, len %d, offset %d, width %d\n",
	 *	id, len, offset, width));
	 */
	tbl.tbl_id = id;
	tbl.tbl_len = len;
	tbl.tbl_offset = offset;
	tbl.tbl_width = width;
	tbl.tbl_ptr = data;
	wlc_phy_read_table_htphy(pi, &tbl);
}

/* XXX
 * Query if the input addr is valid for RFSEQ PHY Table
 * (ID = HTPHY_TBL_ID_RFSEQ)
 * REF:
 * http://www.sj.broadcom.com/projects/BCM4331/gallery/design/dot11nphy/doc/RF_AFE_2059_Tables.xls
 */
bool
wlc_phy_rfseqtbl_valid_addr_htphy(phy_info_t *pi, uint16 addr)
{
	/* avoid dumping out holes in the RFSEQ Table */
	if ((addr > 0x05F && addr < 0x080) || (addr > 0x0DF && addr < 0x100) || addr > 0x16F)
		return FALSE;
	if (addr == 0x105 || addr == 0x109 || addr == 0x10D)
		return FALSE;
	if (addr == 0x113 || addr == 0x117 || addr == 0x11E || addr == 0x11F)
		return FALSE;
	if (addr >= 0x128 && addr <= 0x12F)
		return FALSE;
	if (addr == 0x139)
		return FALSE;
	if (addr >= 0x13C && addr <= 0x13F)
		return FALSE;
	if (addr == 0x14B || addr == 0x15B || addr == 0x16B)
		return FALSE;

	return TRUE;
}

/* initialize the static tables defined in auto-generated wlc_phytbl_ht.c,
 * see htphyprocs.tcl, proc htphy_init_tbls
 * After called in the attach stage, all the static phy tables are reclaimed.
 */
static void
WLBANDINITFN(wlc_phy_static_table_download_htphy)(phy_info_t *pi)
{
	uint idx;

	/* these tables are not affected by phy reset, only power down */
	for (idx = 0; idx < htphytbl_info_sz_rev0; idx++) {
		wlc_phy_write_table_htphy(pi, &htphytbl_info_rev0[idx]);
	}
}

/* initialize all the tables defined in auto-generated wlc_phytbl_ht.c,
 * see htphyprocs.tcl, proc htphy_init_tbls
 *  skip static one after first up
 */
static void
WLBANDINITFN(wlc_phy_tbl_init_htphy)(phy_info_t *pi)
{
	PHY_TRACE(("wl%d: %s, dnld tables = %d\n", pi->sh->unit,
	          __FUNCTION__, pi->phy_init_por));

	/* these tables are not affected by phy reset, only power down */
	if (pi->phy_init_por)
		wlc_phy_static_table_download_htphy(pi);

	/* init volatile tables */

	/*  combo shared (X29) */
	if ((CHSPEC_IS2G(pi->radio_chanspec) && (pi->fem2g->antswctrllut == 2)) ||
	    (CHSPEC_IS5G(pi->radio_chanspec) && (pi->fem5g->antswctrllut == 2)))  {
		/*  TR/Switch lines  */
		uint8 antswctrllut_fem5515[]  = {0, 0, 2, 0, 1, 1, 3, 1, 0, 0, 2, 0, 1, 1, 3, 1};
		uint8 antswctrllut_discrete[] = {0, 0, 2, 0, 0, 0, 2, 0, 0, 0, 2, 0, 0, 0, 2, 0};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_ANTSWCTRLLUT,
		                          16, 0,  8, antswctrllut_fem5515); /* fem 5515 */
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_ANTSWCTRLLUT,
		                          16, 16, 8, antswctrllut_discrete); /* discrete */
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_ANTSWCTRLLUT,
		                          16, 32, 8, antswctrllut_fem5515); /* fem 5515 */
	/*   X19C   */
	} else if ((CHSPEC_IS2G(pi->radio_chanspec) && (pi->fem2g->antswctrllut == 3)) ||
	           (CHSPEC_IS5G(pi->radio_chanspec) && (pi->fem5g->antswctrllut == 3)))  {
		/*  TR/Switch lines  */
		uint8 antswctrllut_fem5515[]  = {0, 0, 2, 0, 1, 1, 3, 1, 0, 0, 2, 0, 1, 1, 3, 1};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_ANTSWCTRLLUT,
		                          16, 0,  8, antswctrllut_fem5515); /* fem 5515 */
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_ANTSWCTRLLUT,
		                          16, 16, 8, antswctrllut_fem5515); /* fem 5515 */
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_ANTSWCTRLLUT,
		                          16, 32, 8, antswctrllut_fem5515); /* fem 5515 */

	/*  combo shared (X29 proto 2+) */
	} else if ((CHSPEC_IS2G(pi->radio_chanspec) && (pi->fem2g->antswctrllut == 4)) ||
	           (CHSPEC_IS5G(pi->radio_chanspec) && (pi->fem5g->antswctrllut == 4)))  {
		/*  TR/Switch lines  */
		uint8 antswctrllut_fem5515[]  = {0, 0, 2, 0, 1, 1, 3, 1, 0, 0, 2, 0, 1, 1, 3, 1};
		uint8 antswctrllut_discrete[] = {0, 1, 3, 0, 0, 1, 3, 0, 0, 1, 3, 0, 0, 1, 3, 0};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_ANTSWCTRLLUT,
		                          16, 0,  8, antswctrllut_fem5515); /* fem 5515 */
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_ANTSWCTRLLUT,
		                          16, 16, 8, antswctrllut_discrete); /* discrete */
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_ANTSWCTRLLUT,
		                          16, 32, 8, antswctrllut_fem5515); /* fem 5515 */

		/* XXX PR106608, X29B:
		 * Bypass 2G elna on cores 0,2 as part of rxgain restaging (for btcoex)
		 */
		if (IS_X29B_BOARDTYPE(pi) && CHSPEC_IS2G(pi->radio_chanspec) &&
		    pi->u.pi_htphy->btc_restage_rxgain) {
			uint8 antswctrl_bypelna = 0;
			wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_ANTSWCTRLLUT,
			                          1, 2,  8, &antswctrl_bypelna);
			wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_ANTSWCTRLLUT,
			                          1, 10,  8, &antswctrl_bypelna);
			wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_ANTSWCTRLLUT,
			                          1, 34,  8, &antswctrl_bypelna);
			wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_ANTSWCTRLLUT,
			                          1, 42,  8, &antswctrl_bypelna);
		}
	}
}

void
wlc_phy_write_txmacreg_htphy(wlc_phy_t *ppi, uint16 holdoff, uint16 delay_val)
{
	phy_info_t *pi = (phy_info_t *)ppi;

	phy_utils_write_phyreg(pi, HTPHY_TxMacIfHoldOff, holdoff);
	phy_utils_write_phyreg(pi, HTPHY_TxMacDelay, delay_val);
}

bool
BCMATTACHFN(wlc_phy_attach_htphy)(phy_info_t *pi)
{
	phy_info_htphy_t *pi_ht;
	shared_phy_t *sh;
	uint16 core;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	ASSERT(pi);
	if (!pi) {
		PHY_ERROR(("wl: wlc_phy_attach_htphy: NULL pi\n"));
		return FALSE;
	}

	sh = pi->sh;

	if ((pi->u.pi_htphy = (phy_info_htphy_t *) MALLOC(sh->osh,
		sizeof(phy_info_htphy_t))) == NULL) {
		PHY_ERROR(("wl%d: wlc_phy_attach_htphy: out of memory, malloced %d bytes\n",
			sh->unit, MALLOCED(sh->osh)));
		return FALSE;
	}
	bzero((char*)pi->u.pi_htphy, sizeof(phy_info_htphy_t));
	pi_ht = pi->u.pi_htphy;

	/* pre_init to ON, register POR default setting */
	pi_ht->ht_rxldpc_override = ON;

	pi->n_preamble_override = WLC_N_PREAMBLE_GF_BRCM;

	pi->phy_scraminit = AUTO;

	pi->phy_cal_mode = PHY_PERICAL_MPHASE;
	pi->phy_cal_delay = PHY_PERICAL_DELAY_DEFAULT;

	pi->radio_is_on = FALSE;

	FOREACH_CORE(pi, core) {
		pi_ht->txpwrindex_hw_save[core] = 128;
	}

	if ((PHY_GETVAR(pi, "subband5gver")) != NULL)
		pi->sromi->subband5Gver = (uint)PHY_GETINTVAR(pi, "subband5gver");
	else
		pi->sromi->subband5Gver = PHY_SUBBAND_3BAND_JAPAN;
	if (PHY_GETVAR(pi, "pcieingress_war") != NULL)
	       pi->pcieingress_war = (uint)PHY_GETINTVAR(pi, "pcieingress_war");
	else
	       pi->pcieingress_war = 0xf;

	wlc_phy_txpwrctrl_config_htphy(pi);

	pi_ht->btc_restage_rxgain = FALSE;

	if (!wlc_phy_srom_read_htphy(pi))
		goto exit;

	/* Initialize Dynamic TSSI cal and TSSI/Vmid cache & restore */
	pi->itssical = ((pi->sh->boardflags2 & BFL2_DYNAMIC_VMID) != 0 ||
		pi->fem5g->pdetrange == 9);
	pi->itssi_cap_low5g = FALSE;

	/* setup function pointers */
	pi->pi_fptr.calinit = wlc_phy_cal_init_htphy;
	pi->pi_fptr.chanset = wlc_phy_chanspec_set_htphy;
	pi->pi_fptr.phywatchdog = wlc_phy_watchdog_htphy;
#if defined(BCMDBG) || defined(WLTEST)
	pi->pi_fptr.longtrn = NULL;
#endif // endif
	pi->pi_fptr.txiqccget = NULL;
	pi->pi_fptr.txiqccset = NULL;
	pi->pi_fptr.txloccget = NULL;
	pi->pi_fptr.radioloftget = NULL;
	pi->pi_fptr.carrsuppr = NULL;
#if defined(WLTEST)
	pi->pi_fptr.rxsigpwr = NULL;
#endif // endif
	pi->pi_fptr.txcorepwroffsetset = wlc_phy_txpower_core_offset_set_htphy;
	pi->pi_fptr.txcorepwroffsetget = wlc_phy_txpower_core_offset_get_htphy;

#ifdef ATE_BUILD
	pi->pi_fptr.gpaioconfigptr = NULL;
#endif // endif
	/* PA Mode change not supported by HT PHYs */
	pi->pi_fptr.txswctrlmapsetptr = NULL;
	pi->pi_fptr.txswctrlmapgetptr = NULL;

#if defined(WLTEST) || defined(BCMDBG)
	pi->pi_fptr.epadpdsetptr = NULL;
#endif // endif

	return TRUE;

exit:
	if (pi->u.pi_htphy != NULL) {
		MFREE(pi->sh->osh, pi->u.pi_htphy, sizeof(phy_info_htphy_t));
		pi->u.pi_htphy = NULL;
	}
	return FALSE;
}

static void
BCMATTACHFN(wlc_phy_txpwrctrl_config_htphy)(phy_info_t *pi)
{
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	pi->hwpwrctrl_capable = TRUE;
	pi->txpwrctrl = PHY_TPC_HW_ON;

	pi->phy_5g_pwrgain = TRUE;
}

void
WLBANDINITFN(wlc_phy_init_htphy)(phy_info_t *pi)
{
	phy_info_htphy_t *pi_ht = (phy_info_htphy_t *)pi->u.pi_htphy;
	uint16 val;
	uint16 clip1_ths[PHY_CORE_MAX];
	uint8 tx_pwr_ctrl_state = PHY_TPC_HW_OFF;
	uint16 core;
	uint32 *tx_pwrctrl_tbl = NULL;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	/* init PUB_NOT_ASSOC */
	if (!(pi->measure_hold & PHY_HOLD_FOR_SCAN) &&
		!(pi->interf->aci.nphy.detection_in_progress)) {
		pi->measure_hold |= PHY_HOLD_FOR_NOT_ASSOC;
	}

	pi_ht->deaf_count = 0;

	/* Step 0, initialize tables */
	wlc_phy_tbl_init_htphy(pi);

	/* Load preferred values */
	MOD_PHYREG(pi, HTPHY_BphyControl1, flipiq_adccompout, 0);

	/* Reset Ch11 40 MHz spur avoidance WAR flags */

	/***********************************
	 * Rfctrl, Rfseq, and Afectrl setup
	 */

	/* Step 1, power up and reset radio
	 * Step 2, clear force reset to radio
	 * NOT DONE HERE -- already done in wlc_phy_radio2059_reset
	 */

	/* Step 3, write 0x0 to rfctrloverride regs
	 * remove rfctrl signal override
	 */
	FOREACH_CORE(pi, core) {
		phy_utils_write_phyreg(pi, HTPHY_RfctrlOverride(core), 0);
		phy_utils_write_phyreg(pi, HTPHY_RfctrlOverrideAux(core), 0);
		phy_utils_write_phyreg(pi, HTPHY_RfctrlOverrideLpfCT(core), 0);
		phy_utils_write_phyreg(pi, HTPHY_RfctrlOverrideLpfPU(core), 0);
	}

	/* Step 4, clear bit in RfctrlIntcX to give the control to rfseq
	 * for the antenna for coreX
	 */
	FOREACH_CORE(pi, core) {
		phy_utils_write_phyreg(pi, HTPHY_RfctrlIntc(core), 0);
	}

	/* Step 8, Write 0x0 to RfseqMode to turn off both CoreActv_override
	 * (to give control to Tx control word) and Trigger_override (to give
	 * control to rfseq)
	 *
	 * Now you are done with all rfseq INIT.
	 */
	phy_utils_and_phyreg(pi, HTPHY_RfseqMode, ~3);

	/* Step 9, write 0x0 to AfectrlOverride to give control to Auto
	 * control mode.
	 */
	FOREACH_CORE(pi, core) {
		phy_utils_write_phyreg(pi, HTPHY_AfectrlOverride(core), 0);
	}

	/* gdk: not sure what these do but they are used in wmac */
	phy_utils_write_phyreg(pi, HTPHY_AfeseqTx2RxPwrUpDownDly20M, 32);
	phy_utils_write_phyreg(pi, HTPHY_AfeseqTx2RxPwrUpDownDly40M, 32);

	/* FIXME: review comparative DAC output timing for OFDM and 11b TX
	 * Delay htphy start to 2.3 usec to match BPHY
	 */
	phy_utils_write_phyreg(pi, HTPHY_TxRealFrameDelay, 184);

	/* Turn on TxCRS extension.
	 * (Need to eventually make the 1.0 be native TxCRSOff (1.0us))
	 */
	phy_utils_write_phyreg(pi, HTPHY_mimophycrsTxExtension, 200);

	/* FIXME: review RX CRS, RXFRAME, RXSTART timing
	 * Adjust PayloadCRS extension for RX.
	 * (This should have some better definition of native RxCRSoff (4.0us))
	 */
	phy_utils_write_phyreg(pi, HTPHY_payloadcrsExtensionLen, 80);

	/* This number combined with MAC RIFS results in 2.0us RIFS air time */
	phy_utils_write_phyreg(pi, HTPHY_TxRifsFrameDelay, 48);

	/* LDPC RX settings */
	wlc_phy_update_rxldpc_htphy(pi, ON);

	/* set tx/rx chain */

	/* For ExtPA, use sharp tx digital filter for BPHY transmission same as
	 * in the IntPA case to improve spectral-mask margin as well as EVM.
	 * PR 66521.
	 */
	wlc_phy_extpa_set_tx_digi_filts_htphy(pi);

	wlc_phy_workarounds_htphy(pi);

	/* Configure Sample Play Buffer's LPF-BW's in RfSeq */
	wlc_phy_init_lpf_sampleplay(pi);

	/* Pulse reset_cca after initing all the tables */
	wlapi_bmac_phyclk_fgc(pi->sh->physhim, ON);

	val = phy_utils_read_phyreg(pi, HTPHY_BBConfig);
	phy_utils_write_phyreg(pi, HTPHY_BBConfig, val | BBCFG_RESETCCA);
	phy_utils_write_phyreg(pi, HTPHY_BBConfig, val & (~BBCFG_RESETCCA));
	wlapi_bmac_phyclk_fgc(pi->sh->physhim, OFF);

	/* Temporarily Force PHY gated clock on to avoid weird PHY hang issue.
	 * This hangs issue ties closely with MAC AES wedge issue, we commonly
	 * believe it is due to some corner bad PLCP cases cause the clock control
	 * logic out of order. Not reproduced yet, in future we hope to move this
	 * work around into ucode to do it on frame by frame basis
	 */
	wlapi_bmac_macphyclk_set(pi->sh->physhim, ON);

	/* example calls to avoid compiler warnings, not used elsewhere yet: */
	wlc_phy_classifier_htphy(pi, 0, 0);
	wlc_phy_clip_det_htphy(pi, 0, clip1_ths);

	/* Initialize the bphy part */
	if (CHSPEC_IS2G(pi->radio_chanspec))
		wlc_phy_bphy_init_htphy(pi);

	/* Load tx gain table */
	if (CHSPEC_IS5G(pi->radio_chanspec)) {
		tx_pwrctrl_tbl = tpc_txgain_htphy_rev0_5g;
		/* XXX
		 * pdetrange == 6 means 5004 PA
		 * pdetrange == 8 means 5003 design for Asus board
		 * pdetrange == 12 means 5003 design on BCM94706LAPH5
		 * extpagain == 3 means high power PA
		 * if the above two are true use a different gain table
		 */
		if ((pi->fem5g->extpagain == 3) &&
		    ((pi->fem5g->pdetrange == 6) || (pi->fem5g->pdetrange == 8) ||
		    (pi->fem5g->pdetrange == 12)))
			tx_pwrctrl_tbl = tpc_txgain_htphy_rev0_5g_hpa;
	} else {
		tx_pwrctrl_tbl = tpc_txgain_htphy_rev0_2g;
	}
	wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_TXPWRCTL(0), 128, 192, 32, tx_pwrctrl_tbl);

	/* set txgain in case txpwrctrl is disabled */
	wlc_phy_txpwr_fixpower_htphy(pi);

	/* ensure power control is off before starting cals */
	tx_pwr_ctrl_state = pi->txpwrctrl;
	wlc_phy_txpwrctrl_enable_htphy(pi, PHY_TPC_HW_OFF);

	/* Idle TSSI measurement and pwrctrl setup */
	wlc_phy_txpwrctrl_idle_tssi_meas_htphy(pi);
	wlc_phy_txpwrctrl_pwr_setup_htphy(pi);

	/* Set up radio muxes for TSSI
	 * Note: Idle TSSI measurement above may return early w/o
	 *       setting up the muxes for TSSI, see
	 *       wlc_phy_txpwrctrl_idle_tssi_meas_htphy().
	 */
	wlc_phy_tssi_radio_setup_htphy(pi, 7);

	/* If any rx cores were disabled before htphy_init,
	 *  disable them again since htphy_init enables all rx cores
	 */
	if (pi->sh->phyrxchain != 0x7) {
		wlc_phy_rxcore_setstate_htphy((wlc_phy_t *)pi, pi->sh->phyrxchain);
	}

	/* XXX FIXME: add various cal stuff here
	 * FIXME: lots of conditioning here (Scan etc.) in Nphy
	 * FIXME: simply schedule a multiphase calibration
	 * FIXME: unclear whether we need a cal during init to begin with
	 *        maybe only in 2G band, where coeffs are very similar b/w
	 *        channels so can use for STA's initial SCAN
	 * reset mphase calibration
	 *	if (PHY_PERICAL_MPHASE_PENDING(pi)) {
	 *		wlc_phy_cal_perical_mphase_restart(pi);
	 *	}
	 */

	/* re-enable (if necessary) tx power control */
	wlc_phy_txpwrctrl_enable_htphy(pi, tx_pwr_ctrl_state);

	/* Set the analog TX_LPF Bandwidth */
	wlc_phy_txlpfbw_htphy(pi);

	/* ACI mode inits */
	if (pi->phy_init_por) {
		pi->interf->hw_aci_mitig_on = FALSE;
		wlc_phy_aci_noise_store_values_htphy(pi);
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			wlc_phy_acimode_reset_htphy(pi);
		}
		wlc_phy_noisemode_reset_htphy(pi);

		/* curr_home_channel may have changed from attached value */
		pi->interf->curr_home_channel = CHSPEC_CHANNEL(pi->radio_chanspec);
	}

	wlc_phy_enable_extlna_extpa_htphy(pi);

	/* Setup control for BT/WLAN switch: */
	wlc_phy_BT_SwControl_setup_htphy(pi, pi->radio_chanspec);

	phy_utils_mod_phyreg(pi, HTPHY_mluA, HTPHY_mluA_mluA1_MASK,
		(4 << HTPHY_mluA_mluA1_SHIFT));
	phy_utils_mod_phyreg(pi, HTPHY_mluA, HTPHY_mluA_mluA2_MASK,
		(4 << HTPHY_mluA_mluA2_SHIFT));
}

/* enable/disable receiving of LDPC frame */
void
wlc_phy_update_rxldpc_htphy(phy_info_t *pi, bool ldpc)
{
	phy_info_htphy_t *pi_ht;
	uint16 val, bit;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	pi_ht = (phy_info_htphy_t *)pi->u.pi_htphy;
	if (ldpc == pi_ht->ht_rxldpc_override)
		return;

	pi_ht->ht_rxldpc_override = ldpc;
	val = phy_utils_read_phyreg(pi, HTPHY_HTSigTones);

	bit = HTPHY_HTSigTones_support_ldpc_MASK;
	if (ldpc)
		val |= bit;
	else
		val &= ~bit;
	phy_utils_write_phyreg(pi, HTPHY_HTSigTones, val);
}

static void
wlc_phy_resetcca_htphy(phy_info_t *pi)
{
	uint16 val;

	/* MAC should be suspended before calling this function */
	ASSERT(!(R_REG(pi->sh->osh, &pi->regs->maccontrol) & MCTL_EN_MAC));

	wlapi_bmac_phyclk_fgc(pi->sh->physhim, ON);

	val = phy_utils_read_phyreg(pi, HTPHY_BBConfig);
	phy_utils_write_phyreg(pi, HTPHY_BBConfig, val | BBCFG_RESETCCA);
	OSL_DELAY(1);
	phy_utils_write_phyreg(pi, HTPHY_BBConfig, val & (~BBCFG_RESETCCA));

	wlapi_bmac_phyclk_fgc(pi->sh->physhim, OFF);

	wlc_phy_force_rfseq_htphy(pi, HTPHY_RFSEQ_RESET2RX);
}

void
wlc_phy_pa_override_htphy(phy_info_t *pi, bool en)
{
	phy_info_htphy_t *pi_ht = (phy_info_htphy_t *)pi->u.pi_htphy;
	uint8 core;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	FOREACH_CORE(pi, core) {
		if (!en) {
			/* switch Off both PA's */
			pi_ht->rfctrlIntc_save[core] =
			        phy_utils_read_phyreg(pi, HTPHY_RfctrlIntc(core));
			phy_utils_write_phyreg(pi, HTPHY_RfctrlIntc(core), 0x400);
		} else {
			/* restore Rfctrl override settings */
			phy_utils_write_phyreg(pi, HTPHY_RfctrlIntc(core),
			                       pi_ht->rfctrlIntc_save[core]);
		}
	}
}

void
wlc_phy_rxcore_setstate_htphy(wlc_phy_t *pih, uint8 rxcore_bitmask)
{
	phy_info_t *pi = (phy_info_t*)pih;
	uint16 rfseqCoreActv_DisRx_save, rfseqCoreActv_EnTx_save;
	uint16 rfseqMode_save, sampleDepthCount_save, sampleLoopCount_save;
	uint16 sampleInitWaitCount_save, sampleCmd_save;

	ASSERT((rxcore_bitmask > 0) && (rxcore_bitmask <= 7));

	pi->sh->phyrxchain = rxcore_bitmask;

	if (!pi->sh->clk)
		return;

	wlapi_suspend_mac_and_wait(pi->sh->physhim);

	/* Save Registers */
	rfseqCoreActv_DisRx_save = READ_PHYREG(pi, HTPHY_RfseqCoreActv2059, DisRx);
	rfseqCoreActv_EnTx_save = READ_PHYREG(pi, HTPHY_RfseqCoreActv2059, EnTx);
	rfseqMode_save = phy_utils_read_phyreg(pi, HTPHY_RfseqMode);
	sampleDepthCount_save = phy_utils_read_phyreg(pi, HTPHY_sampleDepthCount);
	sampleLoopCount_save = phy_utils_read_phyreg(pi, HTPHY_sampleLoopCount);
	sampleInitWaitCount_save = phy_utils_read_phyreg(pi, HTPHY_sampleInitWaitCount);
	sampleCmd_save = phy_utils_read_phyreg(pi, HTPHY_sampleCmd);

	/* Indicate to PHY of the Inactive Core */
	MOD_PHYREG(pi, HTPHY_CoreConfig, CoreMask, rxcore_bitmask);
	/* Indicate to RFSeq of the Inactive Core */
	MOD_PHYREG(pi, HTPHY_RfseqCoreActv2059, EnRx, rxcore_bitmask);
	/* Make sure Rx Chain gets shut off in Rx2Tx Sequence */
	MOD_PHYREG(pi, HTPHY_RfseqCoreActv2059, DisRx, 7);
	/* Make sure Tx Chain doesn't get turned off during this function */
	MOD_PHYREG(pi, HTPHY_RfseqCoreActv2059, EnTx, 0);
	MOD_PHYREG(pi, HTPHY_RfseqMode, CoreActv_override, 1);

	wlc_phy_stay_in_carriersearch_htphy(pi, TRUE);

	/* To Turn off the AFE:
	 * TxFrame needs to toggle on and off.
	 * Accomplished by A) turning sample play ON and OFF
	 * Rx2Tx and Tx2Rx Rfseq is executed during A)
	 * To Turn Off the Radio (except LPF) - Do a Rx2Tx
	 * To Turn off the LPF - Do a Tx2Rx
	 */
	phy_utils_write_phyreg(pi, HTPHY_sampleDepthCount, 0);
	phy_utils_write_phyreg(pi, HTPHY_sampleLoopCount, 0xffff);
	phy_utils_write_phyreg(pi, HTPHY_sampleInitWaitCount, 0);
	MOD_PHYREG(pi, HTPHY_sampleCmd, DisTxFrameInSampleplay, 0);
	MOD_PHYREG(pi, HTPHY_sampleCmd, start, 1);

	/* Allow Time For Rfseq to start */
	OSL_DELAY(1);
	/* Allow Time For Rfseq to stop - 1ms timeout */
	SPINWAIT(((phy_utils_read_phyreg(pi, HTPHY_RfseqStatus0) & 0x1) == 1),
	         HTPHY_SPINWAIT_RFSEQ_STOP);

	wlc_phy_stay_in_carriersearch_htphy(pi, FALSE);

	MOD_PHYREG(pi, HTPHY_sampleCmd, stop, 1);

	/* Restore Register */
	MOD_PHYREG(pi, HTPHY_RfseqCoreActv2059, DisRx, rfseqCoreActv_DisRx_save);
	MOD_PHYREG(pi, HTPHY_RfseqCoreActv2059, EnTx, rfseqCoreActv_EnTx_save);
	phy_utils_write_phyreg(pi, HTPHY_RfseqMode, rfseqMode_save);
	phy_utils_write_phyreg(pi, HTPHY_sampleDepthCount, sampleDepthCount_save);
	phy_utils_write_phyreg(pi, HTPHY_sampleLoopCount, sampleLoopCount_save);
	phy_utils_write_phyreg(pi, HTPHY_sampleInitWaitCount, sampleInitWaitCount_save);
	phy_utils_write_phyreg(pi, HTPHY_sampleCmd, sampleCmd_save);

	wlapi_enable_mac(pi->sh->physhim);
}

uint8
wlc_phy_rxcore_getstate_htphy(wlc_phy_t *pih)
{
	uint16 rxen_bits;
	phy_info_t *pi = (phy_info_t*)pih;

	rxen_bits = READ_PHYREG(pi, HTPHY_RfseqCoreActv2059, EnRx);

	ASSERT(pi->sh->phyrxchain == rxen_bits);

	return ((uint8) rxen_bits);
}

static void
wlc_phy_workarounds_htphy(phy_info_t *pi)
{
	uint16 rfseq_rx2tx_dacbufpu_rev0[] = {0x10f, 0x10f};
	uint16 dac_control = 0x0002;
	uint16 afectrl_adc_lp_bw20 = 0x1;
	uint16 afectrl_adc_lp_bw40 = 0x0;
	uint16 afectrl_adc_ctrl0_bw20 = 0xA840;
	uint16 afectrl_adc_ctrl1_bw20 = 0x8;
	uint16 afectrl_adc_ctrl0_bw40 = 0xFC60;
	uint16 afectrl_adc_ctrl1_bw40 = 0xC;
	uint16 pktgn_lpf_hml_hpc[] = {0x777, 0x111, 0x111,
	                              0x777, 0x111, 0x111,
	                              0x777, 0x111, 0x111};
	/* XXX backout before PR84023 is fixed
	uint16 rx2tx_lpf_rc_lut_tx[] = {0x0, 0x0, 0x0,
	                                0x2, 0x2, 0x2, 0x2, 0x2, 0x2};
	*/
	uint16 tx2rx_lpf_h_hpc = 0x777;
	uint16 rx2tx_lpf_h_hpc = 0x777;
	uint16 core;
	uint16 tone_jammer_war_blk;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	/* XXX IMPORTANT NOTE:
	 * ---------------
	 * Setting in here may depend on 2G vs 5G and/or bw20 vs bw40, but they
	 * MAY NOT depend on specific channels (or sub-bands). This is because
	 * wlc_phy_workarounds_htphy() is only called during phy init, i.e. on
	 * a band or bw change.
	 * Any channel (or sub-band) specific settings need to go into
	 * wlc_phy_chanspec_setup_htphy() !!! PR82700.
	 */

	if (HTREV_GE(pi->pubpi.phy_rev, 1)) {
		phy_utils_write_phyreg(pi, HTPHY_LDPCControl, 0);
	}

	if (CHSPEC_IS5G(pi->radio_chanspec)) {
		wlc_phy_classifier_htphy(pi, HTPHY_ClassifierCtrl_cck_en, 0);
	} else {
		wlc_phy_classifier_htphy(pi, HTPHY_ClassifierCtrl_cck_en, 1);
	}

	/* XXX REV7 PR73285 if an IQ swap is present on the radio before the signals
	 *    reach the AFE, flip the I and Q samples after ADC
	 */
	if (!ISSIM_ENAB(pi->sh->sih)) {
		phy_utils_or_phyreg(pi, HTPHY_IQFlip, HTPHY_IQFlip_adc1_MASK |
		           HTPHY_IQFlip_adc2_MASK | HTPHY_IQFlip_adc3_MASK);
	}

	/* XXX 2. enable PingPongComp "comp swap" to alter which rail gets
	 *    half-sample delay compensation; as of nphy, QT DAC/ADC
	 *    hookup employs same rail delay as true ADC, so comp swap
	 *    no longer conditioned on is_qt.
	 */
	phy_utils_write_phyreg(pi, HTPHY_PingPongComp, HTPHY_PingPongComp_comp_enable_MASK |
	              HTPHY_PingPongComp_comp_iqswap_MASK);

	/* XXX PR50520: need to increase PHY holdoff time to avoid tx underflow in TKIP+WME
	 * PR78138: can't increase holdoff too much, or will corrupt TX OFDM SIG
	 */
	wlc_phy_write_txmacreg_htphy((wlc_phy_t *)pi, 0x10, 0x258);

	/* XXX FIXME: PR77432 - Tx shouldn't turn off before all the samples are on the air.
	 * (this workaround is particularly true for Resampler On + PAPD On)
	 */
	phy_utils_write_phyreg(pi, HTPHY_txTailCountValue, 114);

	/* XXX FIXME: need to check the table offsets for htphy rev0 and add 3rd core
	 * Need to power up dacbuf in the rx2tx RF sequence
	 */
	FOREACH_CORE(pi, core) {
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_RFSEQ,
		                          2, 0x14e + 0x10*core, 16, rfseq_rx2tx_dacbufpu_rev0);
	}

	FOREACH_CORE(pi, core) {
		MOD_PHYREGC(pi, HTPHY_Afectrl, core, adc_pd, 1);
		MOD_PHYREGC(pi, HTPHY_AfectrlOverride, core, adc_pd, 1);
		if (CHSPEC_IS20(pi->radio_chanspec)) {
			MOD_PHYREGC(pi, HTPHY_Afectrl, core, adc_lp, afectrl_adc_lp_bw20);
			MOD_PHYREGC(pi, HTPHY_AfectrlOverride, core, adc_lp, 1);
			wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL, 1, 0x04 + 0x10*core, 16,
			                          &afectrl_adc_ctrl0_bw20);
			wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL, 1, 0x05 + 0x10*core, 16,
			                          &afectrl_adc_ctrl1_bw20);
		} else {
			MOD_PHYREGC(pi, HTPHY_Afectrl, core, adc_lp, afectrl_adc_lp_bw40);
			MOD_PHYREGC(pi, HTPHY_AfectrlOverride, core, adc_lp, 1);
			wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL, 1, 0x04 + 0x10*core, 16,
			                          &afectrl_adc_ctrl0_bw40);
			wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL, 1, 0x05 + 0x10*core, 16,
			                          &afectrl_adc_ctrl1_bw40);
		}
		MOD_PHYREGC(pi, HTPHY_AfectrlOverride, core, adc_pd, 0);
	}

	/* XXX Higher LPF HPC "H" (7) for better settling during gain-settling, better DC Rejection
	 * Lower  LPF HPC "L" (1) for better inner tone EVM during payload decoding;
	 */
	wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_RFSEQ, 9, 0x130, 16, pktgn_lpf_hml_hpc);
	wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_RFSEQ, 1, 0x120, 16, &tx2rx_lpf_h_hpc);
	wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_RFSEQ, 1, 0x124, 16, &rx2tx_lpf_h_hpc);

	/* Use 9/18MHz analog LPF BW settings for TX of 20/40MHz frames, respectively. */
	/* XXX backout before PR84023 is fixed
	FOREACH_CORE(pi, core) {
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_RFSEQ, 9, 0x142 + 0x10*core, 16,
		                          rx2tx_lpf_rc_lut_tx);
	}
	*/

	/* XXX PR37069 -- Prevent 40MHz BW glitching at DAC (160MHz)
	 *             by getting AFE to sample on clk rising edge val offset
	 */
	FOREACH_CORE(pi, core) {
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL, 1, 0x00 + 0x10*core, 16,
		                          &dac_control);
	}

	/* SET Vmid and Av for {TSSI} based on ID pdetrangeXg FROM SROM */
	wlc_phy_tssi_pdetrange_cust_htphy(pi);

	if (HTREV_IS(pi->pubpi.phy_rev, 0)) {
		phy_utils_mod_radioreg(pi, RADIO_2059_OVR_REG7(0), 0x8, 0x8);
		phy_utils_mod_radioreg(pi, RADIO_2059_LOGEN_PTAT_RESETS, 0x2, 0x2);
	}

	/* XXX Upon Reception of a High Tone/Tx Spur,
	 * the default 40MHz MF settings causes ton of glitches.
	 * Set the MF settings similar to 20MHz uniformly
	 * Provides Robustness for tones
	 * (on-chip, on-platform, accidential loft coming from other devices)
	 */
	MOD_PHYREG(pi, HTPHY_crsControll, mfLessAve, 0);
	MOD_PHYREG(pi, HTPHY_crsThreshold2l, peakThresh, 85);
	MOD_PHYREG(pi, HTPHY_crsControlu, mfLessAve, 0);
	MOD_PHYREG(pi, HTPHY_crsThreshold2u, peakThresh, 85);

#ifdef NPHYREV7_HTPHY_DFS_WAR
	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		/* XXX In event of high power spurs/interference that causes crs-glitches,
		   stay in WAIT_ENERGY_DROP for 1 clk20 instead of default 1 ms.
		   This way, we get back to CARRIER_SEARCH quickly and will less likely to miss
		   actual packets. PS: this is actually one settings for ACI
		*/
		phy_reg_write(pi, HTPHY_energydroptimeoutLen, 0x2);

		/* XXX Fine timing modificiation to have more overlap (~10dB)
		 * between low and high SNR regimes
		 */
		MOD_PHYREG(pi, HTPHY_FSTRMetricTh, hiPwr_min_metric_th, 0xf);
	} else {
		/* use 1ms WAIT_ENERGY_DROP for DFS detection in 5G */
		phy_reg_write(pi, HTPHY_energydroptimeoutLen, 0x9c40);
		/* change to 0x8 to prevent the radar to trigger fine timing */
		MOD_PHYREG(pi, HTPHY_FSTRMetricTh, hiPwr_min_metric_th, 0x8);
	}
#else
	/* XXX In event of high power spurs/interference that causes crs-glitches, stay
	 * in WAIT_ENERGY_DROP for 1 clk20 instead of default 1 ms. This way, we
	 * get back to CARRIER_SEARCH quickly and will less likely miss actual
	 * packets. PS: this is actually one settings for ACI
	 */
	/* XXX In ACI mitigation code, HTPHY_energydroptimeoutLen = 2 is assumed
	 * for proper interference modes 3 and 4 operation. If this value is
	 * changed, make sure the ACI code is reviewed for proper operation
	 */
	phy_utils_write_phyreg(pi, HTPHY_energydroptimeoutLen, 0x2);

	/* XXX Fine timing modificiation to have more overlap (~10dB)
	 * between low and high SNR regimes
	 */
	MOD_PHYREG(pi, HTPHY_FSTRMetricTh, hiPwr_min_metric_th, 0xf);
#endif	/* NPHYREV7_HTPHY_DFS_WAR */

	FOREACH_CORE(pi, core) {
		MOD_PHYREGC(pi, HTPHY_computeGainInfo, core, disableClip2detect, 1);
	}

	tone_jammer_war_blk = wlapi_bmac_read_shm(pi->sh->physhim, M_AFEOVR_PTR);
	FOREACH_CORE(pi, core) {
		wlapi_bmac_write_shm(pi->sh->physhim, (tone_jammer_war_blk +  2 * core + 0) * 2,
			HTPHY_AfectrlOverride(core));
		wlapi_bmac_write_shm(pi->sh->physhim, (tone_jammer_war_blk +  2 * core + 1) * 2,
			HTPHY_Afectrl(core));
	}
	/* enable the war by default */
		wlapi_bmac_mhf(pi->sh->physhim, MHF5, MHF5_TONEJAMMER_WAR,
		MHF5_TONEJAMMER_WAR, WLC_BAND_ALL);
	/* XXX PR100453 LOFT suppression improvements (LOGEN Overrides)
	 * Second piece of this WAR happens dynamically in
	 *  the ucode and writes RADIO_2059_OVR_REG6 to 0x4
	 */
	if (CHSPEC_IS5G(pi->radio_chanspec)) {
		phy_utils_write_radioreg(pi, RADIO_2059_LOGEN_PUS1(3), 0x2);
		/* enable the ucode war in 5G only */
		wlapi_bmac_mhf(pi->sh->physhim, MHF5, MHF5_TXLOFT_WAR,
		               MHF5_TXLOFT_WAR, WLC_BAND_5G);
	}

}

static void
wlc_phy_subband_cust_htphy(phy_info_t *pi)
{
	bool elna_present = 0;
	bool call_extlna_war = 0;
	void (*subbandcust_pointer)(phy_info_t *pi) = 0;

	if  ((BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA) ||
	     (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA_5GHz)) {
		elna_present = 1;
	}
	if (HTREV_GE(pi->pubpi.phy_rev, 1) && ((CHIPID(pi->sh->chip) == BCM4331_CHIP_ID) ||
	                                       (CHIPID(pi->sh->chip) == BCM43431_CHIP_ID))) {
		/* XXX
		 * KEEP ALL THE CONDITIONING
		 * BASED ON BOARDTYPE ON THE TOP
		 *
		 */

		if (pi->sh->boardtype == BCM94331MCH5_SSID) {
			/* XXX
			 * MCH5
			 */
			subbandcust_pointer = wlc_phy_subband_cust_rev1_mch5_htphy;
		} else if (IS_X29B_BOARDTYPE(pi)) {
			subbandcust_pointer = wlc_phy_subband_cust_rev1_cs_htphy;
		} else if ((CHSPEC_IS2G(pi->radio_chanspec) &&
			(pi->fem2g->antswctrllut == 3)) ||
			(CHSPEC_IS5G(pi->radio_chanspec) &&
			(pi->fem5g->antswctrllut == 3))) {
			subbandcust_pointer = wlc_phy_subband_cust_rev1_x33_htphy;
		} else if (IS_X12_BOARDTYPE(pi) || (elna_present &&
		                                    ((CHSPEC_IS2G(pi->radio_chanspec) &&
		                                      (pi->u.pi_htphy->elna2g == 1)) ||
		                                     (CHSPEC_IS5G(pi->radio_chanspec) &&
		                                      (pi->u.pi_htphy->elna5g == 0))))) {
			/* XXX
			 * X12 or any board with similar rx frontend in 2G or 5G
			 * gains and charcetrisitic
			 * use the meausured gain for elna2g and elna5g
			 * gain = 2*elna2(5)g + 8;
			 */
			if ((IS_X12_BOARDTYPE(pi) &&
			     (pi->sh->boardrev >= 0x1603)) || !(IS_X12_BOARDTYPE(pi))) {
				/* XXX 2G Absorptive T/R-switch with high isolation,
				 * Skyworks extLNA.
				 * for non X12 boardtypes use this
				 */
				subbandcust_pointer = wlc_phy_subband_cust_rev1_pciedual_htphy;
				/* XXX
				 * There was a separate subband_cust setting for X12
				 * boardrevs >=0x1400 && <0x1603, which is no longer
				 * needed as the production boardrev for X12 is 0x1610.
				 * (It is still available in tcl; check proc
				 * htphy_subband_cust in htphyprocs.tcl)
				 */
			} else {
				/* XXX RFMD Ext-LNA used (boardrevs < 0x1400)
				 * NOTE:
				 * This setting is also picked up by WNDR4500 which
				 * shares the same boardtype as X12 on the 2G interface:
				 * http://home.sj.broadcom.com/doc/hwlab/4706/bcm94706nrhwndr4500/
				 */
				subbandcust_pointer = wlc_phy_subband_cust_rev1_pciedual_old_htphy;
			}
		}  else if (pi->sh->boardtype == BCM94331_4706_ASUS) {
			/*
			 * XXX ASUS or any board with similar rx frontend in 2G or 5G
			 * use the meausured gain for elna2g and elna5g
			 */
			subbandcust_pointer = wlc_phy_subband_cust_rev1_asus_htphy;
		} else if (elna_present &&
		           ((CHSPEC_IS5G(pi->radio_chanspec) &&
		             (pi->u.pi_htphy->elna5g == 3)) ||
		            ((CHSPEC_IS2G(pi->radio_chanspec) &&
		              (pi->u.pi_htphy->elna2g == 2))))) {
			/*
			 * XXX 4706nrh or any board with similar rx frontend in 5G = 14.5 dB
			 * use the meausured elna5g
			 */
			subbandcust_pointer = wlc_phy_subband_cust_rev1_4706nrh_htphy;
		} else {
			subbandcust_pointer = wlc_phy_subband_cust_rev1_htphy;
			/* XXX
			 * If external LNA is used and does not have
			 * a seperate subband cust
			 * for further optimization define
			 * a new subband cust
			 */
			call_extlna_war = 1;
		}
	} else {
		subbandcust_pointer = wlc_phy_subband_cust_rev0_htphy;
		/*
		 * If external LNA is used
		 * and a seperate subband cust
		 * is not defined
		 * for further optimization define
		 * a new subband cust
		 */
		call_extlna_war = 1;
	}

	ASSERT(subbandcust_pointer);
	if (subbandcust_pointer)
		subbandcust_pointer(pi);

	if (call_extlna_war)
		wlc_phy_extlna_war_htphy(pi);

	wlc_phy_pcieingress_war_htphy(pi);
	if (IS_X29D_BOARDTYPE(pi)) {
		/* Subband Customization for X29D */
		/* Increased clip lo gain to regain the loss of clip lo */
		/* low sensitivity due to 5G TRSW loss */
		/* Centered 2G, HT20 W1 threshold to center it in clip lo region */
		/* Increased mid 5G CRSmin by 1 tick WRT X19C */
		uint16 fc;
		if (CHSPEC_CHANNEL(pi->radio_chanspec) > 14) {
			fc = CHAN5G_FREQ(CHSPEC_CHANNEL(pi->radio_chanspec));
		} else {
			fc = CHAN2G_FREQ(CHSPEC_CHANNEL(pi->radio_chanspec));
		}
		if (CHSPEC_IS5G(pi->radio_chanspec)) {
			MOD_PHYREG(pi, HTPHY_Core1clipwbThreshold2059, clip1wbThreshold, 0x12);
			if  ((fc >= 5240 && fc <= 5550)) {
				phy_utils_write_phyreg(pi, HTPHY_Core1cliploGainCodeA2059, 0x008c);
			} else if ((fc >= 5560 && fc <= 5825)) {
				phy_utils_write_phyreg(pi, HTPHY_Core1cliploGainCodeA2059, 0x008e);
			}
			if (CHSPEC_IS20(pi->radio_chanspec) == 1) {
				if  ((fc >= 5240 && fc <= 5550)) {
					MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 64);
					MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 64);
				}
			}
		}
	}
}

static void
wlc_phy_extlna_war_htphy(phy_info_t *pi)
{
	uint8 core;
	int16 new_biq1_gain[PHY_CORE_MAX];
	uint16 biq1_gain[PHY_CORE_MAX], currgain_init[PHY_CORE_MAX];
	uint16 elna_gain_db[2], gain_applied, val, back_off_val;

	if  (!((BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_ELNA_GAINDEF) &&
	       ((BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA) ||
	        (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA_5GHz)))) {
		return;
	}

	val = 2*((CHSPEC_IS2G(pi->radio_chanspec) ?
	          pi->u.pi_htphy->elna2g : pi->u.pi_htphy->elna5g)) + 8;

	if (val == 0)
		return;

	(void)memset(currgain_init, 0, sizeof(currgain_init));

	/*
	 * Backing off initgain
	 */
	currgain_init[0] = (uint16) phy_utils_read_phyreg(pi, HTPHY_Core0InitGainCodeB2059);
	biq1_gain[0] = (uint16) ((currgain_init[0] &
	                          HTPHY_Core0InitGainCodeB2059_initBiQ1Index_MASK)
	                         >> HTPHY_Core0InitGainCodeB2059_initBiQ1Index_SHIFT);

	if (PHYCORENUM(pi->pubpi.phy_corenum) > 1) {
		currgain_init[1] = (uint16) phy_utils_read_phyreg(pi, HTPHY_Core1InitGainCodeB2059);
		biq1_gain[1] = (uint16) ((currgain_init[1] &
			HTPHY_Core0InitGainCodeB2059_initBiQ1Index_MASK)
			>> HTPHY_Core0InitGainCodeB2059_initBiQ1Index_SHIFT);
	}

	if (PHYCORENUM(pi->pubpi.phy_corenum) > 2) {
		currgain_init[2] = (uint16) phy_utils_read_phyreg(pi, HTPHY_Core2InitGainCodeB2059);
		biq1_gain[2] = (uint16) ((currgain_init[2] &
			HTPHY_Core0InitGainCodeB2059_initBiQ1Index_MASK)
			>> HTPHY_Core0InitGainCodeB2059_initBiQ1Index_SHIFT);
	}

	back_off_val = val/3;
	FOREACH_CORE(pi, core) {
		new_biq1_gain[core] = biq1_gain[core] - back_off_val;
		if (new_biq1_gain[core] < 0) {
			new_biq1_gain[core] = 0;
		}
	}

	gain_applied = (((uint16)(new_biq1_gain[0] <<
	                          HTPHY_Core0InitGainCodeB2059_initBiQ1Index_SHIFT))
	                | (currgain_init[0] & 0xff));
	phy_utils_write_phyreg(pi, HTPHY_Core0InitGainCodeB2059, gain_applied);

	if (PHYCORENUM(pi->pubpi.phy_corenum) > 1) {
		gain_applied = (((uint16)(new_biq1_gain[1] <<
					HTPHY_Core0InitGainCodeB2059_initBiQ1Index_SHIFT))
				| (currgain_init[1] & 0xff));
		phy_utils_write_phyreg(pi, HTPHY_Core1InitGainCodeB2059, gain_applied);
	}
	if (PHYCORENUM(pi->pubpi.phy_corenum) > 2) {
		gain_applied = (((uint16)(new_biq1_gain[2] <<
					HTPHY_Core0InitGainCodeB2059_initBiQ1Index_SHIFT))
				| (currgain_init[2] & 0xff));
		phy_utils_write_phyreg(pi, HTPHY_Core2InitGainCodeB2059, gain_applied);
	}

	/*
	 * Writing the external LNA gain in the gain table
	 */

	elna_gain_db[0] = val;
	elna_gain_db[1] = val;
	wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1, 2, 0x0, 8, elna_gain_db);
	wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2, 2, 0x0, 8, elna_gain_db);
	wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3, 2, 0x0, 8, elna_gain_db);
}

/*
 * AUTOGENERATED from htphy/subband_cust_4331B1_4706nrh.xls
 * DO NOT EDIT
 */

static void
wlc_phy_subband_cust_rev1_4706nrh_htphy(phy_info_t *pi)
{
	void (*subbandcust_bandx_pointer)(phy_info_t *pi, uint16 fc) = 0;
	uint16 fc;
	if (CHSPEC_CHANNEL(pi->radio_chanspec) > 14) {
		fc = CHAN5G_FREQ(CHSPEC_CHANNEL(pi->radio_chanspec));
	} else {
		fc = CHAN2G_FREQ(CHSPEC_CHANNEL(pi->radio_chanspec));
	}

	/* 2G Band Customizations */
	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		subbandcust_bandx_pointer = wlc_phy_subband_cust_rev1_4706nrh_2g_htphy;
	}

	/* 5G Band Customizations */
	if (CHSPEC_IS5G(pi->radio_chanspec)) {
		subbandcust_bandx_pointer = wlc_phy_subband_cust_rev1_4706nrh_5g_htphy;
	}
	ASSERT(subbandcust_bandx_pointer);
	if (subbandcust_bandx_pointer)
		subbandcust_bandx_pointer(pi, fc);
}
static void
wlc_phy_subband_cust_rev1_4706nrh_2g_htphy(phy_info_t *pi, uint16 fc)
{

	/* Default customizations */
	/* CRS MinPwr */
	MOD_PHYREG(pi, HTPHY_bphycrsminpower0, bphycrsminpower0, 0x46);
	MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 0x3a);
	MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 0x3a);
	/* Clip1 WB Thresh */
	MOD_PHYREG(pi, HTPHY_Core0clipwbThreshold2059, clip1wbThreshold, 0x12);
	MOD_PHYREG(pi, HTPHY_Core1clipwbThreshold2059, clip1wbThreshold, 0x12);
	MOD_PHYREG(pi, HTPHY_Core2clipwbThreshold2059, clip1wbThreshold, 0x12);
	/* NB Clip  */
	phy_utils_write_phyreg(pi, HTPHY_Core0nbClipThreshold, 0xbd);
	phy_utils_write_phyreg(pi, HTPHY_Core1nbClipThreshold, 0xbd);
	phy_utils_write_phyreg(pi, HTPHY_Core2nbClipThreshold, 0xbd);
	/* Hi Gain  */
	phy_utils_write_phyreg(pi, HTPHY_Core0clipHiGainCodeA2059, 0x4c);
	phy_utils_write_phyreg(pi, HTPHY_Core1clipHiGainCodeA2059, 0x4c);
	phy_utils_write_phyreg(pi, HTPHY_Core2clipHiGainCodeA2059, 0x4c);
	phy_utils_write_phyreg(pi, HTPHY_Core0clipHiGainCodeB2059, 0x014);
	phy_utils_write_phyreg(pi, HTPHY_Core1clipHiGainCodeB2059, 0x014);
	phy_utils_write_phyreg(pi, HTPHY_Core2clipHiGainCodeB2059, 0x014);
	/* MD Gain  */
	phy_utils_write_phyreg(pi, HTPHY_Core0clipmdGainCodeA2059, 0x22);
	phy_utils_write_phyreg(pi, HTPHY_Core1clipmdGainCodeA2059, 0x22);
	phy_utils_write_phyreg(pi, HTPHY_Core2clipmdGainCodeA2059, 0x22);
	phy_utils_write_phyreg(pi, HTPHY_Core0clipmdGainCodeB2059, 0x004);
	phy_utils_write_phyreg(pi, HTPHY_Core1clipmdGainCodeB2059, 0x004);
	phy_utils_write_phyreg(pi, HTPHY_Core2clipmdGainCodeB2059, 0x004);
	/* LO Gain  */
	phy_utils_write_phyreg(pi, HTPHY_Core0cliploGainCodeA2059, 0x4c);
	phy_utils_write_phyreg(pi, HTPHY_Core1cliploGainCodeA2059, 0x4c);
	phy_utils_write_phyreg(pi, HTPHY_Core2cliploGainCodeA2059, 0x6c);
	phy_utils_write_phyreg(pi, HTPHY_Core0cliploGainCodeB2059, 0x018);
	phy_utils_write_phyreg(pi, HTPHY_Core1cliploGainCodeB2059, 0x018);
	phy_utils_write_phyreg(pi, HTPHY_Core2cliploGainCodeB2059, 0x018);
	/* InitGCode */
	phy_utils_write_phyreg(pi, HTPHY_Core0InitGainCodeA2059, 0x6c);
	phy_utils_write_phyreg(pi, HTPHY_Core1InitGainCodeA2059, 0x6c);
	phy_utils_write_phyreg(pi, HTPHY_Core2InitGainCodeA2059, 0x6c);
	phy_utils_write_phyreg(pi, HTPHY_Core0InitGainCodeB2059, 0x0614);
	phy_utils_write_phyreg(pi, HTPHY_Core1InitGainCodeB2059, 0x0614);
	phy_utils_write_phyreg(pi, HTPHY_Core2InitGainCodeB2059, 0x0614);
	/* InitGain */
	{
		const uint16 temp[] = {0x6136, 0x6136, 0x6136};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_RFSEQ,
		                          3, 0x106, 16, temp);
	}
	/* EXTLNA Gains (dB) */
	{
		const int8 temp[] = {13, 13};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
		                          2, 0x0, 8, temp);
	}
	{
		const int8 temp[] = {13, 13};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
		                          2, 0x0, 8, temp);
	}
	{
		const int8 temp[] = {13, 13};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
		                          2, 0x0, 8, temp);
	}
	/* LNA1 Gains [dB] */
	{
		const int8 temp[] = {9, 15, 19, 19};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
		                          4, 0x8, 8, temp);
	}
	{
		const int8 temp[] = {9, 15, 19, 19};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
		                          4, 0x8, 8, temp);
	}
	{
		const int8 temp[] = {9, 15, 19, 19};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
		                          4, 0x8, 8, temp);
	}
	{
		const int8 temp[] = {0, 1, 2, 2};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS1,
		                          4, 0x8, 8, temp);
	}
	{
		const int8 temp[] = {0, 1, 2, 2};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS2,
		                          4, 0x8, 8, temp);
	}
	{
		const int8 temp[] = {0, 1, 2, 2};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS3,
		                          4, 0x8, 8, temp);
	}
	/* LNA2 Gains [dB] */
	{
		const int8 temp[] = {-3, 7, 7, 7};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
		                          4, 0x10, 8, temp);
	}
	{
		const int8 temp[] = {-3, 7, 7, 7};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
		                          4, 0x10, 8, temp);
	}
	{
		const int8 temp[] = {-3, 7, 7, 7};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
		                          4, 0x10, 8, temp);
	}
	{
		const int8 temp[] = {0, 1, 1, 1};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS1,
		                          4, 0x10, 8, temp);
	}
	{
		const int8 temp[] = {0, 1, 1, 1};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS2,
		                          4, 0x10, 8, temp);
	}
	{
		const int8 temp[] = {0, 1, 1, 1};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS3,
		                          4, 0x10, 8, temp);
	}
	/* MixTIA Gains */
	{
		const int8 temp[] = {-1, -1, 2, 5, 5, 5, 5, 5, 5, 5};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
		                          10, 0x20, 8, temp);
	}
	{
		const int8 temp[] = {-1, -1, 2, 5, 5, 5, 5, 5, 5, 5};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
		                          10, 0x20, 8, temp);
	}
	{
		const int8 temp[] = {-1, -1, 2, 5, 5, 5, 5, 5, 5, 5};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
		                          10, 0x20, 8, temp);
	}
	{
		const int8 temp[] = {1, 1, 2, 3, 3, 3, 3, 3, 3, 3};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS1,
		                          10, 0x20, 8, temp);
	}
	{
		const int8 temp[] = {1, 1, 2, 3, 3, 3, 3, 3, 3, 3};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS2,
		                          10, 0x20, 8, temp);
	}
	{
		const int8 temp[] = {1, 1, 2, 3, 3, 3, 3, 3, 3, 3};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS3,
		                          10, 0x20, 8, temp);
	}
	/* Vmid and Av for {RSSI invalid PWRDET} */
	{
		const uint16 temp[] = {0x8e, 0x96, 0x96};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x08, 16, temp);
	}
	{
		const uint16 temp[] = {0x8f, 0x9f, 0x9f};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x18, 16, temp);
	}
	{
		const uint16 temp[] = {0x8f, 0x9f, 0x9f};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x28, 16, temp);
	}
	{
		const uint16 temp[] = {0x02, 0x02, 0x02};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x0c, 16, temp);
	}
	{
		const uint16 temp[] = {0x02, 0x02, 0x02};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x1c, 16, temp);
	}
	{
		const uint16 temp[] = {0x02, 0x02, 0x02};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x2c, 16, temp);
	}

	/* BW20 customizations */
	if (CHSPEC_IS20(pi->radio_chanspec) == 1) {

		if ((fc >= 2412 && fc <= 2484)) {
		}

		/* BW40 customizations */
	} else {

		if ((fc >= 2412 && fc <= 2437)) {
		} else if ((fc >= 2442 && fc <= 2484)) {
		}
	}
}
static void
wlc_phy_subband_cust_rev1_4706nrh_5g_htphy(phy_info_t *pi, uint16 fc)
{

	/* Default customizations */
	/* Hi Gain  */
	phy_utils_write_phyreg(pi, HTPHY_Core0clipHiGainCodeA2059, 0x004d);
	phy_utils_write_phyreg(pi, HTPHY_Core1clipHiGainCodeA2059, 0x004d);
	phy_utils_write_phyreg(pi, HTPHY_Core2clipHiGainCodeA2059, 0x004d);
	phy_utils_write_phyreg(pi, HTPHY_Core0clipHiGainCodeB2059, 0x4);
	phy_utils_write_phyreg(pi, HTPHY_Core1clipHiGainCodeB2059, 0x4);
	phy_utils_write_phyreg(pi, HTPHY_Core2clipHiGainCodeB2059, 0x4);
	/* MD Gain  */
	phy_utils_write_phyreg(pi, HTPHY_Core0clipmdGainCodeA2059, 0x0043);
	phy_utils_write_phyreg(pi, HTPHY_Core1clipmdGainCodeA2059, 0x0043);
	phy_utils_write_phyreg(pi, HTPHY_Core2clipmdGainCodeA2059, 0x0043);
	phy_utils_write_phyreg(pi, HTPHY_Core0clipmdGainCodeB2059, 0x4);
	phy_utils_write_phyreg(pi, HTPHY_Core1clipmdGainCodeB2059, 0x4);
	phy_utils_write_phyreg(pi, HTPHY_Core2clipmdGainCodeB2059, 0x4);
	/* LO Gain  */
	phy_utils_write_phyreg(pi, HTPHY_Core0cliploGainCodeA2059, 0x002d);
	phy_utils_write_phyreg(pi, HTPHY_Core1cliploGainCodeA2059, 0x002d);
	phy_utils_write_phyreg(pi, HTPHY_Core2cliploGainCodeA2059, 0x002d);
	phy_utils_write_phyreg(pi, HTPHY_Core0cliploGainCodeB2059, 0x0128);
	phy_utils_write_phyreg(pi, HTPHY_Core1cliploGainCodeB2059, 0x0128);
	phy_utils_write_phyreg(pi, HTPHY_Core2cliploGainCodeB2059, 0x0128);
	/* InitGCode */
	phy_utils_write_phyreg(pi, HTPHY_Core0InitGainCodeA2059, 0x004d);
	phy_utils_write_phyreg(pi, HTPHY_Core1InitGainCodeA2059, 0x004d);
	phy_utils_write_phyreg(pi, HTPHY_Core2InitGainCodeA2059, 0x004d);
	/* InitGain */
	{
		const uint16 temp[] = {0x6226, 0x6226, 0x6226};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_RFSEQ,
		                          3, 0x106, 16, temp);
	}
	/* EXTLNA Gains (dB) */
	{
		const int8 temp[] = {15, 15};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
		                          2, 0x0, 8, temp);
	}
	{
		const int8 temp[] = {15, 15};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
		                          2, 0x0, 8, temp);
	}
	{
		const int8 temp[] = {15, 15};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
		                          2, 0x0, 8, temp);
	}
	/* MixTIA Gains */
	{
		const int8 temp[] = {-10, -7, -4, -1, 3, 3, 3, 3, 3, 3};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
		                          10, 0x20, 8, temp);
	}
	{
		const int8 temp[] = {-10, -7, -4, -1, 3, 3, 3, 3, 3, 3};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
		                          10, 0x20, 8, temp);
	}
	{
		const int8 temp[] = {-10, -7, -4, -1, 3, 3, 3, 3, 3, 3};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
		                          10, 0x20, 8, temp);
	}
	{
		const int8 temp[] = {0, 1, 2, 3, 4, 4, 4, 4, 4, 4};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS1,
		                          10, 0x20, 8, temp);
	}
	{
		const int8 temp[] = {0, 1, 2, 3, 4, 4, 4, 4, 4, 4};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS2,
		                          10, 0x20, 8, temp);
	}
	{
		const int8 temp[] = {0, 1, 2, 3, 4, 4, 4, 4, 4, 4};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS3,
		                          10, 0x20, 8, temp);
	}
	/* Vmid and Av for {RSSI invalid PWRDET} */
	{
		const uint16 temp[] = {0x8e, 0x96, 0x96};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x08, 16, temp);
	}
	{
		const uint16 temp[] = {0x8f, 0x9f, 0x9f};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x18, 16, temp);
	}
	{
		const uint16 temp[] = {0x8f, 0x9f, 0x9f};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x28, 16, temp);
	}
	{
		const uint16 temp[] = {0x02, 0x02, 0x02};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x0c, 16, temp);
	}
	{
		const uint16 temp[] = {0x02, 0x02, 0x02};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x1c, 16, temp);
	}
	{
		const uint16 temp[] = {0x02, 0x02, 0x02};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x2c, 16, temp);
	}

	/* BW20 customizations */
	if (CHSPEC_IS20(pi->radio_chanspec) == 1) {

		if ((fc >= 4920 && fc <= 5230)) {
			/* CRS MinPwr */
			MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 53);
			MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 53);
			/* RadioNF [1/4dB] */
			MOD_PHYREG(pi, HTPHY_nvcfg0, noisevar_nf_radio_qdb_core0, 28);
			MOD_PHYREG(pi, HTPHY_nvcfg0, noisevar_nf_radio_qdb_core1, 28);
			MOD_PHYREG(pi, HTPHY_nvcfg1, noisevar_nf_radio_qdb_core2, 28);
			/* Clip1 WB Thresh */
			MOD_PHYREG(pi, HTPHY_Core0clipwbThreshold2059, clip1wbThreshold, 0x1a);
			MOD_PHYREG(pi, HTPHY_Core1clipwbThreshold2059, clip1wbThreshold, 0x1a);
			MOD_PHYREG(pi, HTPHY_Core2clipwbThreshold2059, clip1wbThreshold, 0x1a);
			/* NB Clip  */
			phy_utils_write_phyreg(pi, HTPHY_Core0nbClipThreshold, 0x90);
			phy_utils_write_phyreg(pi, HTPHY_Core1nbClipThreshold, 0x90);
			phy_utils_write_phyreg(pi, HTPHY_Core2nbClipThreshold, 0x90);
			/* LNA1 Gains [dB] */
			{
				const int8 temp[] = {11, 17, 21, 25};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {11, 17, 21, 25};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {8, 14, 18, 22};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x8, 8, temp);
			}
			/* LNA2 Gains [dB] */
			{
				const int8 temp[] = {-1, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {-1, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {-1, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x10, 8, temp);
			}

		} else if ((fc >= 5240 && fc <= 5550)) {
			/* CRS MinPwr */
			MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 52);
			MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 52);
			/* RadioNF [1/4dB] */
			MOD_PHYREG(pi, HTPHY_nvcfg0, noisevar_nf_radio_qdb_core0, 28);
			MOD_PHYREG(pi, HTPHY_nvcfg0, noisevar_nf_radio_qdb_core1, 28);
			MOD_PHYREG(pi, HTPHY_nvcfg1, noisevar_nf_radio_qdb_core2, 28);
			/* Clip1 WB Thresh */
			MOD_PHYREG(pi, HTPHY_Core0clipwbThreshold2059, clip1wbThreshold, 0x1a);
			MOD_PHYREG(pi, HTPHY_Core1clipwbThreshold2059, clip1wbThreshold, 0x1a);
			MOD_PHYREG(pi, HTPHY_Core2clipwbThreshold2059, clip1wbThreshold, 0x1a);
			/* NB Clip  */
			phy_utils_write_phyreg(pi, HTPHY_Core0nbClipThreshold, 0x90);
			phy_utils_write_phyreg(pi, HTPHY_Core1nbClipThreshold, 0x90);
			phy_utils_write_phyreg(pi, HTPHY_Core2nbClipThreshold, 0x90);
			/* LNA1 Gains [dB] */
			{
				const int8 temp[] = {11, 17, 21, 25};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {9, 15, 19, 23};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {8, 14, 18, 22};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x8, 8, temp);
			}
			/* LNA2 Gains [dB] */
			{
				const int8 temp[] = {-1, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {-1, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {-1, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x10, 8, temp);
			}

		} else if ((fc >= 5560 && fc <= 5825)) {
			/* CRS MinPwr */
			MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 50);
			MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 50);
			/* RadioNF [1/4dB] */
			MOD_PHYREG(pi, HTPHY_nvcfg0, noisevar_nf_radio_qdb_core0, 28);
			MOD_PHYREG(pi, HTPHY_nvcfg0, noisevar_nf_radio_qdb_core1, 28);
			MOD_PHYREG(pi, HTPHY_nvcfg1, noisevar_nf_radio_qdb_core2, 28);
			/* Clip1 WB Thresh */
			MOD_PHYREG(pi, HTPHY_Core0clipwbThreshold2059, clip1wbThreshold, 0x1a);
			MOD_PHYREG(pi, HTPHY_Core1clipwbThreshold2059, clip1wbThreshold, 0x1a);
			MOD_PHYREG(pi, HTPHY_Core2clipwbThreshold2059, clip1wbThreshold, 0x1a);
			/* NB Clip  */
			phy_utils_write_phyreg(pi, HTPHY_Core0nbClipThreshold, 0x90);
			phy_utils_write_phyreg(pi, HTPHY_Core1nbClipThreshold, 0x90);
			phy_utils_write_phyreg(pi, HTPHY_Core2nbClipThreshold, 0x90);
			/* LNA1 Gains [dB] */
			{
				const int8 temp[] = {11, 17, 21, 25};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {9, 15, 19, 23};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {8, 14, 18, 22};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x8, 8, temp);
			}
			/* LNA2 Gains [dB] */
			{
				const int8 temp[] = {-1, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {-1, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {-1, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x10, 8, temp);
			}

		}

		/* BW40 customizations */
	} else {

		if ((fc >= 4920 && fc <= 5230)) {
			/* CRS MinPwr */
			MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 52);
			MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 52);
			/* Clip1 WB Thresh */
			MOD_PHYREG(pi, HTPHY_Core0clipwbThreshold2059, clip1wbThreshold, 0x1a);
			MOD_PHYREG(pi, HTPHY_Core1clipwbThreshold2059, clip1wbThreshold, 0x1a);
			MOD_PHYREG(pi, HTPHY_Core2clipwbThreshold2059, clip1wbThreshold, 0x1a);
			/* NB Clip  */
			phy_utils_write_phyreg(pi, HTPHY_Core0nbClipThreshold, 0x90);
			phy_utils_write_phyreg(pi, HTPHY_Core1nbClipThreshold, 0x90);
			phy_utils_write_phyreg(pi, HTPHY_Core2nbClipThreshold, 0x90);
			/* LNA1 Gains [dB] */
			{
				const int8 temp[] = {7, 14, 18, 23};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {9, 16, 20, 25};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {7, 14, 18, 23};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x8, 8, temp);
			}
			/* LNA2 Gains [dB] */
			{
				const int8 temp[] = {2, 10, 14, 18};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {2, 10, 14, 18};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {2, 10, 14, 18};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x10, 8, temp);
			}

		} else if ((fc >= 5240 && fc <= 5550)) {
			/* CRS MinPwr */
			MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 50);
			MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 50);
			/* Clip1 WB Thresh */
			MOD_PHYREG(pi, HTPHY_Core0clipwbThreshold2059, clip1wbThreshold, 0x1a);
			MOD_PHYREG(pi, HTPHY_Core1clipwbThreshold2059, clip1wbThreshold, 0x1a);
			MOD_PHYREG(pi, HTPHY_Core2clipwbThreshold2059, clip1wbThreshold, 0x1a);
			/* NB Clip  */
			phy_utils_write_phyreg(pi, HTPHY_Core0nbClipThreshold, 0x90);
			phy_utils_write_phyreg(pi, HTPHY_Core1nbClipThreshold, 0x90);
			phy_utils_write_phyreg(pi, HTPHY_Core2nbClipThreshold, 0x90);
			/* LNA1 Gains [dB] */
			{
				const int8 temp[] = {10, 17, 21, 26};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {9, 16, 20, 25};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {7, 14, 18, 23};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x8, 8, temp);
			}
			/* LNA2 Gains [dB] */
			{
				const int8 temp[] = {2, 10, 14, 18};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {2, 10, 14, 18};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {2, 10, 14, 18};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x10, 8, temp);
			}

		} else if ((fc >= 5560 && fc <= 5825)) {
			/* CRS MinPwr */
			MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 49);
			MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 49);
			/* Clip1 WB Thresh */
			MOD_PHYREG(pi, HTPHY_Core0clipwbThreshold2059, clip1wbThreshold, 0x1a);
			MOD_PHYREG(pi, HTPHY_Core1clipwbThreshold2059, clip1wbThreshold, 0x1a);
			MOD_PHYREG(pi, HTPHY_Core2clipwbThreshold2059, clip1wbThreshold, 0x1a);
			/* NB Clip  */
			phy_utils_write_phyreg(pi, HTPHY_Core0nbClipThreshold, 0x90);
			phy_utils_write_phyreg(pi, HTPHY_Core1nbClipThreshold, 0x90);
			phy_utils_write_phyreg(pi, HTPHY_Core2nbClipThreshold, 0x90);
			/* LNA1 Gains [dB] */
			{
				const int8 temp[] = {10, 17, 21, 26};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {8, 15, 19, 24};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {7, 14, 18, 23};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x8, 8, temp);
			}
			/* LNA2 Gains [dB] */
			{
				const int8 temp[] = {2, 10, 14, 18};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {2, 10, 14, 18};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {2, 10, 14, 18};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x10, 8, temp);
			}

		}
	}
}
/*
 * AUTOGENERATED from htphy/subband_cust_4331B1_asus.xls
 * DO NOT EDIT
 */

static void
wlc_phy_subband_cust_rev1_asus_htphy(phy_info_t *pi)
{
	void (*subbandcust_bandx_pointer)(phy_info_t *pi, uint16 fc) = 0;
	uint16 fc;
	if (CHSPEC_CHANNEL(pi->radio_chanspec) > 14) {
		fc = CHAN5G_FREQ(CHSPEC_CHANNEL(pi->radio_chanspec));
	} else {
		fc = CHAN2G_FREQ(CHSPEC_CHANNEL(pi->radio_chanspec));
	}

	/* 2G Band Customizations */
	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		subbandcust_bandx_pointer = wlc_phy_subband_cust_rev1_asus_2g_htphy;
	}

	/* 5G Band Customizations */
	if (CHSPEC_IS5G(pi->radio_chanspec)) {
		subbandcust_bandx_pointer = wlc_phy_subband_cust_rev1_asus_5g_htphy;
	}
	ASSERT(subbandcust_bandx_pointer);
	if (subbandcust_bandx_pointer)
		subbandcust_bandx_pointer(pi, fc);
}
static void
wlc_phy_subband_cust_rev1_asus_2g_htphy(phy_info_t *pi, uint16 fc)
{

	/* Default customizations */
	/* CRS MinPwr */
	MOD_PHYREG(pi, HTPHY_bphycrsminpower0, bphycrsminpower0, 0x46);
	/* Hi Gain  */
	phy_utils_write_phyreg(pi, HTPHY_Core0clipHiGainCodeA2059, 0x6c);
	phy_utils_write_phyreg(pi, HTPHY_Core1clipHiGainCodeA2059, 0x6c);
	phy_utils_write_phyreg(pi, HTPHY_Core2clipHiGainCodeA2059, 0x6c);
	/* MD Gain  */
	phy_utils_write_phyreg(pi, HTPHY_Core0clipmdGainCodeA2059, 0x60);
	phy_utils_write_phyreg(pi, HTPHY_Core1clipmdGainCodeA2059, 0x60);
	phy_utils_write_phyreg(pi, HTPHY_Core2clipmdGainCodeA2059, 0x60);
	phy_utils_write_phyreg(pi, HTPHY_Core0clipmdGainCodeB2059, 0x24);
	phy_utils_write_phyreg(pi, HTPHY_Core1clipmdGainCodeB2059, 0x24);
	phy_utils_write_phyreg(pi, HTPHY_Core2clipmdGainCodeB2059, 0x24);
	/* LO Gain  */
	phy_utils_write_phyreg(pi, HTPHY_Core0cliploGainCodeA2059, 0x66);
	phy_utils_write_phyreg(pi, HTPHY_Core1cliploGainCodeA2059, 0x66);
	phy_utils_write_phyreg(pi, HTPHY_Core2cliploGainCodeA2059, 0x66);
	phy_utils_write_phyreg(pi, HTPHY_Core0cliploGainCodeB2059, 0x318);
	phy_utils_write_phyreg(pi, HTPHY_Core1cliploGainCodeB2059, 0x318);
	phy_utils_write_phyreg(pi, HTPHY_Core2cliploGainCodeB2059, 0x318);
	/* InitGCode */
	phy_utils_write_phyreg(pi, HTPHY_Core0InitGainCodeA2059, 0x6e);
	phy_utils_write_phyreg(pi, HTPHY_Core1InitGainCodeA2059, 0x6e);
	phy_utils_write_phyreg(pi, HTPHY_Core2InitGainCodeA2059, 0x6e);
	/* InitGCode */
	phy_utils_write_phyreg(pi, HTPHY_Core0InitGainCodeB2059, 0x0614);
	phy_utils_write_phyreg(pi, HTPHY_Core1InitGainCodeB2059, 0x0614);
	phy_utils_write_phyreg(pi, HTPHY_Core2InitGainCodeB2059, 0x0614);
	/* InitGain */
	{
		const uint16 temp[] = {0x6137, 0x6137, 0x6137};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_RFSEQ,
		                          3, 0x106, 16, temp);
	}
	/* EXTLNA Gains (dB) */
	{
		const int8 temp[] = {12, 12};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
		                          2, 0x0, 8, temp);
	}
	{
		const int8 temp[] = {12, 12};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
		                          2, 0x0, 8, temp);
	}
	{
		const int8 temp[] = {12, 12};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
		                          2, 0x0, 8, temp);
	}
	/* LNA2 Gains [dB] */
	{
		const int8 temp[] = {-3, 7, 7, 7};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
		                          4, 0x10, 8, temp);
	}
	{
		const int8 temp[] = {-3, 7, 7, 7};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
		                          4, 0x10, 8, temp);
	}
	{
		const int8 temp[] = {-3, 7, 7, 7};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
		                          4, 0x10, 8, temp);
	}
	/* LNA2 GainBits */
	{
		const int8 temp[] = {0, 1, 1, 1};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS1,
		                          4, 0x10, 8, temp);
	}
	{
		const int8 temp[] = {0, 1, 1, 1};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS2,
		                          4, 0x10, 8, temp);
	}
	{
		const int8 temp[] = {0, 1, 1, 1};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS3,
		                          4, 0x10, 8, temp);
	}
	/* Vmid and Av for {RSSI invalid PWRDET} */
	{
		const uint16 temp[] = {0x8e, 0x96, 0x96};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x08, 16, temp);
	}
	{
		const uint16 temp[] = {0x8f, 0x9f, 0x9f};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x18, 16, temp);
	}
	{
		const uint16 temp[] = {0x8f, 0x9f, 0x9f};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x28, 16, temp);
	}
	{
		const uint16 temp[] = {0x02, 0x02, 0x02};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x0c, 16, temp);
	}
	{
		const uint16 temp[] = {0x02, 0x02, 0x02};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x1c, 16, temp);
	}
	{
		const uint16 temp[] = {0x02, 0x02, 0x02};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x2c, 16, temp);
	}

	/* BW20 customizations */
	if (CHSPEC_IS20(pi->radio_chanspec) == 1) {

		if ((fc >= 2412 && fc <= 2484)) {
			/* CRS MinPwr */
			MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 72);
			MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 72);
			/* Clip1 WB Thresh */
			MOD_PHYREG(pi, HTPHY_Core0clipwbThreshold2059, clip1wbThreshold, 4);
			MOD_PHYREG(pi, HTPHY_Core1clipwbThreshold2059, clip1wbThreshold, 4);
			MOD_PHYREG(pi, HTPHY_Core2clipwbThreshold2059, clip1wbThreshold, 4);
			/* LNA1 Gains [dB] */
			{
				const int8 temp[] = {9, 15, 19, 24};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {9, 15, 19, 24};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {9, 15, 19, 24};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x8, 8, temp);
			}

		}

		/* BW40 customizations */
	} else {

		if ((fc >= 2412 && fc <= 2437)) {
			/* CRS MinPwr */
			MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 69);
			MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 69);
			/* LNA1 Gains [dB] */
			{
				const int8 temp[] = {7, 13, 17, 22};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {7, 13, 17, 22};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {7, 13, 17, 22};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x8, 8, temp);
			}

		} else if ((fc >= 2442 && fc <= 2484)) {
			/* CRS MinPwr */
			MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 69);
			MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 69);
			/* LNA1 Gains [dB] */
			{
				const int8 temp[] = {8, 13, 17, 22};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {8, 13, 17, 22};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {8, 13, 17, 22};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x8, 8, temp);
			}

		}
	}
}
static void
wlc_phy_subband_cust_rev1_asus_5g_htphy(phy_info_t *pi, uint16 fc)
{

	/* Default customizations */
	/* Hi Gain  */
	phy_utils_write_phyreg(pi, HTPHY_Core0clipHiGainCodeA2059, 0x96);
	phy_utils_write_phyreg(pi, HTPHY_Core1clipHiGainCodeA2059, 0x96);
	phy_utils_write_phyreg(pi, HTPHY_Core2clipHiGainCodeA2059, 0x96);
	phy_utils_write_phyreg(pi, HTPHY_Core0clipHiGainCodeB2059, 0x4);
	phy_utils_write_phyreg(pi, HTPHY_Core1clipHiGainCodeB2059, 0x4);
	phy_utils_write_phyreg(pi, HTPHY_Core2clipHiGainCodeB2059, 0x4);
	/* MD Gain  */
	phy_utils_write_phyreg(pi, HTPHY_Core0clipmdGainCodeA2059, 0x80);
	phy_utils_write_phyreg(pi, HTPHY_Core1clipmdGainCodeA2059, 0x80);
	phy_utils_write_phyreg(pi, HTPHY_Core2clipmdGainCodeA2059, 0x80);
	phy_utils_write_phyreg(pi, HTPHY_Core0clipmdGainCodeB2059, 0x14);
	phy_utils_write_phyreg(pi, HTPHY_Core1clipmdGainCodeB2059, 0x14);
	phy_utils_write_phyreg(pi, HTPHY_Core2clipmdGainCodeB2059, 0x14);
	/* LO Gain  */
	phy_utils_write_phyreg(pi, HTPHY_Core0cliploGainCodeA2059, 0x0082);
	phy_utils_write_phyreg(pi, HTPHY_Core1cliploGainCodeA2059, 0x0082);
	phy_utils_write_phyreg(pi, HTPHY_Core2cliploGainCodeA2059, 0x0082);
	phy_utils_write_phyreg(pi, HTPHY_Core0cliploGainCodeB2059, 0x308);
	phy_utils_write_phyreg(pi, HTPHY_Core1cliploGainCodeB2059, 0x308);
	phy_utils_write_phyreg(pi, HTPHY_Core2cliploGainCodeB2059, 0x308);
	/* InitGCode */
	phy_utils_write_phyreg(pi, HTPHY_Core0InitGainCodeA2059, 0x8e);
	phy_utils_write_phyreg(pi, HTPHY_Core1InitGainCodeA2059, 0x8e);
	phy_utils_write_phyreg(pi, HTPHY_Core2InitGainCodeA2059, 0x8e);
	/* InitGain */
	{
		const uint16 temp[] = {0x6247, 0x6247, 0x6247};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_RFSEQ,
		                          3, 0x106, 16, temp);
	}
	/* EXTLNA Gains (dB) */
	{
		const int8 temp[] = {12, 12};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
		                          2, 0x0, 8, temp);
	}
	{
		const int8 temp[] = {12, 12};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
		                          2, 0x0, 8, temp);
	}
	{
		const int8 temp[] = {12, 12};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
		                          2, 0x0, 8, temp);
	}
	/* MixTIA Gains */
	{
		const int8 temp[] = {-10, -7, -4, -1, 3, 3, 3, 3, 3, 3};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
		                          10, 32, 8, temp);
	}
	{
		const int8 temp[] = {-10, -7, -4, -1, 3, 3, 3, 3, 3, 3};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
		                          10, 32, 8, temp);
	}
	{
		const int8 temp[] = {-10, -7, -4, -1, 3, 3, 3, 3, 3, 3};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
		                          10, 32, 8, temp);
	}
	/* MixGainBits */
	{
		const int8 temp[] = {0, 1, 2, 3, 4, 4, 4, 4, 4, 4};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS1,
		                          10, 0x20, 8, temp);
	}
	{
		const int8 temp[] = {0, 1, 2, 3, 4, 4, 4, 4, 4, 4};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS2,
		                          10, 0x20, 8, temp);
	}
	{
		const int8 temp[] = {0, 1, 2, 3, 4, 4, 4, 4, 4, 4};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS3,
		                          10, 0x20, 8, temp);
	}
	/* Vmid and Av for {RSSI invalid PWRDET} */
	{
		const uint16 temp[] = {0x8e, 0x96, 0x96};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x08, 16, temp);
	}
	{
		const uint16 temp[] = {0x8f, 0x9f, 0x9f};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x18, 16, temp);
	}
	{
		const uint16 temp[] = {0x8f, 0x9f, 0x9f};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x28, 16, temp);
	}
	{
		const uint16 temp[] = {0x02, 0x02, 0x02};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x0c, 16, temp);
	}
	{
		const uint16 temp[] = {0x02, 0x02, 0x02};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x1c, 16, temp);
	}
	{
		const uint16 temp[] = {0x02, 0x02, 0x02};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x2c, 16, temp);
	}

	/* BW20 customizations */
	if (CHSPEC_IS20(pi->radio_chanspec) == 1) {

		if ((fc >= 4920 && fc <= 5230)) {
			/* CRS MinPwr */
			MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 79);
			MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 79);
			/* RadioNF [1/4dB] */
			MOD_PHYREG(pi, HTPHY_nvcfg0, noisevar_nf_radio_qdb_core0, 28);
			MOD_PHYREG(pi, HTPHY_nvcfg0, noisevar_nf_radio_qdb_core1, 28);
			MOD_PHYREG(pi, HTPHY_nvcfg1, noisevar_nf_radio_qdb_core2, 28);
			/* Clip1 WB Thresh */
			MOD_PHYREG(pi, HTPHY_Core0clipwbThreshold2059, clip1wbThreshold, 0xa);
			MOD_PHYREG(pi, HTPHY_Core1clipwbThreshold2059, clip1wbThreshold, 0xa);
			MOD_PHYREG(pi, HTPHY_Core2clipwbThreshold2059, clip1wbThreshold, 0xa);
			/* NB Clip  */
			phy_utils_write_phyreg(pi, HTPHY_Core0nbClipThreshold, 0xbc);
			phy_utils_write_phyreg(pi, HTPHY_Core1nbClipThreshold, 0xbc);
			phy_utils_write_phyreg(pi, HTPHY_Core2nbClipThreshold, 0xbc);
			/* LNA1 Gains [dB] */
			{
				const int8 temp[] = {11, 17, 21, 25};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {11, 17, 21, 25};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {8, 14, 18, 22};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x8, 8, temp);
			}
			/* LNA2 Gains [dB] */
			{
				const int8 temp[] = {-1, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {-1, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {-1, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x10, 8, temp);
			}

		} else if ((fc >= 5240 && fc <= 5550)) {
			/* CRS MinPwr */
			MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 75);
			MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 75);
			/* RadioNF [1/4dB] */
			MOD_PHYREG(pi, HTPHY_nvcfg0, noisevar_nf_radio_qdb_core0, 28);
			MOD_PHYREG(pi, HTPHY_nvcfg0, noisevar_nf_radio_qdb_core1, 28);
			MOD_PHYREG(pi, HTPHY_nvcfg1, noisevar_nf_radio_qdb_core2, 28);
			/* Clip1 WB Thresh */
			MOD_PHYREG(pi, HTPHY_Core0clipwbThreshold2059, clip1wbThreshold, 0xa);
			MOD_PHYREG(pi, HTPHY_Core1clipwbThreshold2059, clip1wbThreshold, 0xa);
			MOD_PHYREG(pi, HTPHY_Core2clipwbThreshold2059, clip1wbThreshold, 0xa);
			/* NB Clip  */
			phy_utils_write_phyreg(pi, HTPHY_Core0nbClipThreshold, 0xbc);
			phy_utils_write_phyreg(pi, HTPHY_Core1nbClipThreshold, 0xbc);
			phy_utils_write_phyreg(pi, HTPHY_Core2nbClipThreshold, 0xbc);
			/* LNA1 Gains [dB] */
			{
				const int8 temp[] = {11, 17, 21, 25};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {9, 15, 19, 23};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {8, 14, 18, 22};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x8, 8, temp);
			}
			/* LNA2 Gains [dB] */
			{
				const int8 temp[] = {-1, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {-1, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {-1, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x10, 8, temp);
			}

		} else if ((fc >= 5560 && fc <= 5825)) {
			/* CRS MinPwr */
			MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 75);
			MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 75);
			/* RadioNF [1/4dB] */
			MOD_PHYREG(pi, HTPHY_nvcfg0, noisevar_nf_radio_qdb_core0, 28);
			MOD_PHYREG(pi, HTPHY_nvcfg0, noisevar_nf_radio_qdb_core1, 28);
			MOD_PHYREG(pi, HTPHY_nvcfg1, noisevar_nf_radio_qdb_core2, 28);
			/* Clip1 WB Thresh */
			MOD_PHYREG(pi, HTPHY_Core0clipwbThreshold2059, clip1wbThreshold, 0x13);
			MOD_PHYREG(pi, HTPHY_Core1clipwbThreshold2059, clip1wbThreshold, 0x13);
			MOD_PHYREG(pi, HTPHY_Core2clipwbThreshold2059, clip1wbThreshold, 0x13);
			/* NB Clip  */
			phy_utils_write_phyreg(pi, HTPHY_Core0nbClipThreshold, 0xbc);
			phy_utils_write_phyreg(pi, HTPHY_Core1nbClipThreshold, 0xbc);
			phy_utils_write_phyreg(pi, HTPHY_Core2nbClipThreshold, 0xbc);
			/* LNA1 Gains [dB] */
			{
				const int8 temp[] = {11, 17, 21, 25};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {9, 15, 19, 23};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {8, 14, 18, 22};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x8, 8, temp);
			}
			/* LNA2 Gains [dB] */
			{
				const int8 temp[] = {-1, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {-1, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {-1, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x10, 8, temp);
			}

		}

		/* BW40 customizations */
	} else {

		if ((fc >= 4920 && fc <= 5230)) {
			/* CRS MinPwr */
			MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 75);
			MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 75);
			/* Clip1 WB Thresh */
			MOD_PHYREG(pi, HTPHY_Core0clipwbThreshold2059, clip1wbThreshold, 0xa);
			MOD_PHYREG(pi, HTPHY_Core1clipwbThreshold2059, clip1wbThreshold, 0xa);
			MOD_PHYREG(pi, HTPHY_Core2clipwbThreshold2059, clip1wbThreshold, 0xa);
			/* NB Clip  */
			phy_utils_write_phyreg(pi, HTPHY_Core0nbClipThreshold, 0xb4);
			phy_utils_write_phyreg(pi, HTPHY_Core1nbClipThreshold, 0xb4);
			phy_utils_write_phyreg(pi, HTPHY_Core2nbClipThreshold, 0xb4);
			/* LNA1 Gains [dB] */
			{
				const int8 temp[] = {7, 14, 18, 23};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {9, 16, 20, 25};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {7, 14, 18, 23};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x8, 8, temp);
			}
			/* LNA2 Gains [dB] */
			{
				const int8 temp[] = {2, 10, 14, 18};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {2, 10, 14, 18};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {2, 10, 14, 18};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x10, 8, temp);
			}

		} else if ((fc >= 5240 && fc <= 5550)) {
			/* CRS MinPwr */
			MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 72);
			MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 72);
			/* Clip1 WB Thresh */
			MOD_PHYREG(pi, HTPHY_Core0clipwbThreshold2059, clip1wbThreshold, 0xa);
			MOD_PHYREG(pi, HTPHY_Core1clipwbThreshold2059, clip1wbThreshold, 0xa);
			MOD_PHYREG(pi, HTPHY_Core2clipwbThreshold2059, clip1wbThreshold, 0xa);
			/* NB Clip  */
			phy_utils_write_phyreg(pi, HTPHY_Core0nbClipThreshold, 0xb4);
			phy_utils_write_phyreg(pi, HTPHY_Core1nbClipThreshold, 0xb4);
			phy_utils_write_phyreg(pi, HTPHY_Core2nbClipThreshold, 0xb4);
			/* LNA1 Gains [dB] */
			{
				const int8 temp[] = {10, 17, 21, 26};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {9, 16, 20, 25};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {7, 14, 18, 23};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x8, 8, temp);
			}
			/* LNA2 Gains [dB] */
			{
				const int8 temp[] = {2, 10, 14, 18};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {2, 10, 14, 18};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {2, 10, 14, 18};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x10, 8, temp);
			}

		} else if ((fc >= 5560 && fc <= 5825)) {
			/* CRS MinPwr */
			MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 70);
			MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 70);
			/* Clip1 WB Thresh */
			MOD_PHYREG(pi, HTPHY_Core0clipwbThreshold2059, clip1wbThreshold, 0x13);
			MOD_PHYREG(pi, HTPHY_Core1clipwbThreshold2059, clip1wbThreshold, 0x13);
			MOD_PHYREG(pi, HTPHY_Core2clipwbThreshold2059, clip1wbThreshold, 0x13);
			/* NB Clip  */
			phy_utils_write_phyreg(pi, HTPHY_Core0nbClipThreshold, 0xb4);
			phy_utils_write_phyreg(pi, HTPHY_Core1nbClipThreshold, 0xb4);
			phy_utils_write_phyreg(pi, HTPHY_Core2nbClipThreshold, 0xb4);
			/* LNA1 Gains [dB] */
			{
				const int8 temp[] = {10, 17, 21, 26};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {8, 15, 19, 24};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {7, 14, 18, 23};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x8, 8, temp);
			}
			/* LNA2 Gains [dB] */
			{
				const int8 temp[] = {2, 10, 14, 18};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {2, 10, 14, 18};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {2, 10, 14, 18};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x10, 8, temp);
			}

		}
	}
}

/*
 * AUTOGENERATED from htphy/subband_cust_4331B0_mch5.xls
 * DO NOT EDIT
 */

static void
wlc_phy_subband_cust_rev1_mch5_htphy(phy_info_t *pi)
{
	void (*subbandcust_bandx_pointer)(phy_info_t *pi, uint16 fc) = 0;
	uint16 fc;
	if (CHSPEC_CHANNEL(pi->radio_chanspec) > 14) {
		fc = CHAN5G_FREQ(CHSPEC_CHANNEL(pi->radio_chanspec));
	} else {
		fc = CHAN2G_FREQ(CHSPEC_CHANNEL(pi->radio_chanspec));
	}

	/* 2G Band Customizations */
	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		subbandcust_bandx_pointer = wlc_phy_subband_cust_rev1_mch5_2g_htphy;
	}

	/* 5G Band Customizations */
	if (CHSPEC_IS5G(pi->radio_chanspec)) {
		subbandcust_bandx_pointer = wlc_phy_subband_cust_rev1_mch5_5g_htphy;
	}
	ASSERT(subbandcust_bandx_pointer);
	if (subbandcust_bandx_pointer)
		subbandcust_bandx_pointer(pi, fc);
}
static void
wlc_phy_subband_cust_rev1_mch5_2g_htphy(phy_info_t *pi, uint16 fc)
{

	/* Default customizations */
	/* CRS MinPwr */
	MOD_PHYREG(pi, HTPHY_bphycrsminpower0, bphycrsminpower0, 0x64);
	/* Hi Gain  */
	phy_utils_write_phyreg(pi, HTPHY_Core0clipHiGainCodeA2059, 0x6c);
	phy_utils_write_phyreg(pi, HTPHY_Core1clipHiGainCodeA2059, 0x6c);
	phy_utils_write_phyreg(pi, HTPHY_Core2clipHiGainCodeA2059, 0x6c);
	/* MD Gain  */
	phy_utils_write_phyreg(pi, HTPHY_Core0clipmdGainCodeA2059, 0x60);
	phy_utils_write_phyreg(pi, HTPHY_Core1clipmdGainCodeA2059, 0x60);
	phy_utils_write_phyreg(pi, HTPHY_Core2clipmdGainCodeA2059, 0x60);
	phy_utils_write_phyreg(pi, HTPHY_Core0clipmdGainCodeB2059, 0x24);
	phy_utils_write_phyreg(pi, HTPHY_Core1clipmdGainCodeB2059, 0x24);
	phy_utils_write_phyreg(pi, HTPHY_Core2clipmdGainCodeB2059, 0x24);
	/* LO Gain  */
	phy_utils_write_phyreg(pi, HTPHY_Core0cliploGainCodeA2059, 0x62);
	phy_utils_write_phyreg(pi, HTPHY_Core1cliploGainCodeA2059, 0x62);
	phy_utils_write_phyreg(pi, HTPHY_Core2cliploGainCodeA2059, 0x62);
	phy_utils_write_phyreg(pi, HTPHY_Core0cliploGainCodeB2059, 0x18);
	phy_utils_write_phyreg(pi, HTPHY_Core1cliploGainCodeB2059, 0x18);
	phy_utils_write_phyreg(pi, HTPHY_Core2cliploGainCodeB2059, 0x18);
	/* InitGCode */
	phy_utils_write_phyreg(pi, HTPHY_Core0InitGainCodeA2059, 0x6e);
	phy_utils_write_phyreg(pi, HTPHY_Core1InitGainCodeA2059, 0x6e);
	phy_utils_write_phyreg(pi, HTPHY_Core2InitGainCodeA2059, 0x6e);
	/* InitGCode */
	phy_utils_write_phyreg(pi, HTPHY_Core0InitGainCodeB2059, 0x0514);
	phy_utils_write_phyreg(pi, HTPHY_Core1InitGainCodeB2059, 0x0514);
	phy_utils_write_phyreg(pi, HTPHY_Core2InitGainCodeB2059, 0x0514);
	/* InitGain */
	{
		const uint16 temp[] = {0x5137, 0x5137, 0x5137};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_RFSEQ,
		                          3, 0x106, 16, temp);
	}
	/* EXTLNA Gains (dB) */
	{
		const int8 temp[] = {14, 14};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
		                          2, 0x0, 8, temp);
	}
	{
		const int8 temp[] = {14, 14};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
		                          2, 0x0, 8, temp);
	}
	{
		const int8 temp[] = {14, 14};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
		                          2, 0x0, 8, temp);
	}
	/* Vmid and Av for {RSSI invalid PWRDET} */
	{
		const uint16 temp[] = {0x8e, 0x96, 0x96};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x08, 16, temp);
	}
	{
		const uint16 temp[] = {0x8f, 0x9f, 0x9f};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x18, 16, temp);
	}
	{
		const uint16 temp[] = {0x8f, 0x9f, 0x9f};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x28, 16, temp);
	}
	{
		const uint16 temp[] = {0x02, 0x02, 0x02};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x0c, 16, temp);
	}
	{
		const uint16 temp[] = {0x02, 0x02, 0x02};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x1c, 16, temp);
	}
	{
		const uint16 temp[] = {0x02, 0x02, 0x02};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x2c, 16, temp);
	}

	/* BW20 customizations */
	if (CHSPEC_IS20(pi->radio_chanspec) == 1) {

		if ((fc >= 2412 && fc <= 2484)) {
			/* CRS MinPwr */
			MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 57);
			MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 57);
			/* Clip1 WB Thresh */
			MOD_PHYREG(pi, HTPHY_Core0clipwbThreshold2059, clip1wbThreshold, 13);
			MOD_PHYREG(pi, HTPHY_Core1clipwbThreshold2059, clip1wbThreshold, 13);
			MOD_PHYREG(pi, HTPHY_Core2clipwbThreshold2059, clip1wbThreshold, 13);
			/* LNA1 Gains [dB] */
			{
				const int8 temp[] = {9, 15, 19, 24};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {9, 15, 19, 24};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {9, 15, 19, 24};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x8, 8, temp);
			}
			/* LNA2 Gains [dB] */
			{
				const int8 temp[] = {-3, 7, 11, 15};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {-3, 7, 11, 15};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {-3, 7, 11, 15};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x10, 8, temp);
			}

		}

		/* BW40 customizations */
	} else {

		if ((fc >= 2412 && fc <= 2437)) {
			/* CRS MinPwr */
			MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 57);
			MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 57);
			/* LNA1 Gains [dB] */
			{
				const int8 temp[] = {7, 13, 17, 22};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {7, 13, 17, 22};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {7, 13, 17, 22};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x8, 8, temp);
			}
			/* LNA2 Gains [dB] */
			{
				const int8 temp[] = {-3, 7, 11, 15};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {-3, 7, 11, 15};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {-3, 7, 11, 15};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x10, 8, temp);
			}

		} else if ((fc >= 2442 && fc <= 2484)) {
			/* CRS MinPwr */
			MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 57);
			MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 57);
			/* LNA1 Gains [dB] */
			{
				const int8 temp[] = {8, 13, 17, 22};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {8, 13, 17, 22};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {8, 13, 17, 22};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x8, 8, temp);
			}
			/* LNA2 Gains [dB] */
			{
				const int8 temp[] = {-3, 7, 11, 15};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {-3, 7, 11, 15};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {-3, 7, 11, 15};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x10, 8, temp);
			}

		}
	}
}
static void
wlc_phy_subband_cust_rev1_mch5_5g_htphy(phy_info_t *pi, uint16 fc)
{

	/* Default customizations */
	/* NB Clip  */
	phy_utils_write_phyreg(pi, HTPHY_Core0nbClipThreshold, 0xff);
	phy_utils_write_phyreg(pi, HTPHY_Core1nbClipThreshold, 0xff);
	phy_utils_write_phyreg(pi, HTPHY_Core2nbClipThreshold, 0xff);
	/* Hi Gain  */
	phy_utils_write_phyreg(pi, HTPHY_Core0clipHiGainCodeA2059, 0x94);
	phy_utils_write_phyreg(pi, HTPHY_Core1clipHiGainCodeA2059, 0x94);
	phy_utils_write_phyreg(pi, HTPHY_Core2clipHiGainCodeA2059, 0x94);
	/* MD Gain  */
	phy_utils_write_phyreg(pi, HTPHY_Core0clipmdGainCodeA2059, 0x80);
	phy_utils_write_phyreg(pi, HTPHY_Core1clipmdGainCodeA2059, 0x80);
	phy_utils_write_phyreg(pi, HTPHY_Core2clipmdGainCodeA2059, 0x80);
	phy_utils_write_phyreg(pi, HTPHY_Core0clipmdGainCodeB2059, 0x4);
	phy_utils_write_phyreg(pi, HTPHY_Core1clipmdGainCodeB2059, 0x4);
	phy_utils_write_phyreg(pi, HTPHY_Core2clipmdGainCodeB2059, 0x4);
	/* LO Gain  */
	phy_utils_write_phyreg(pi, HTPHY_Core0cliploGainCodeA2059, 0x0082);
	phy_utils_write_phyreg(pi, HTPHY_Core1cliploGainCodeA2059, 0x0082);
	phy_utils_write_phyreg(pi, HTPHY_Core2cliploGainCodeA2059, 0x0082);
	phy_utils_write_phyreg(pi, HTPHY_Core0cliploGainCodeB2059, 0x8);
	phy_utils_write_phyreg(pi, HTPHY_Core1cliploGainCodeB2059, 0x8);
	phy_utils_write_phyreg(pi, HTPHY_Core2cliploGainCodeB2059, 0x8);
	/* InitGCode */
	phy_utils_write_phyreg(pi, HTPHY_Core0InitGainCodeA2059, 0x94);
	phy_utils_write_phyreg(pi, HTPHY_Core1InitGainCodeA2059, 0x94);
	phy_utils_write_phyreg(pi, HTPHY_Core2InitGainCodeA2059, 0x94);
	/* InitGain */
	{
		const uint16 temp[] = {0x624a, 0x624a, 0x624a};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_RFSEQ,
		                          3, 0x106, 16, temp);
	}
	/* EXTLNA Gains (dB) */
	{
		const int8 temp[] = {10, 10};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
		                          2, 0x0, 8, temp);
	}
	{
		const int8 temp[] = {10, 10};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
		                          2, 0x0, 8, temp);
	}
	{
		const int8 temp[] = {10, 10};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
		                          2, 0x0, 8, temp);
	}
	/* MixTIA Gains */
	{
		const int8 temp[] = {3, 3, 3, 3, 3, 3, 3, 3, 3, 3};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
		                          10, 32, 8, temp);
	}
	{
		const int8 temp[] = {3, 3, 3, 3, 3, 3, 3, 3, 3, 3};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
		                          10, 32, 8, temp);
	}
	{
		const int8 temp[] = {3, 3, 3, 3, 3, 3, 3, 3, 3, 3};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
		                          10, 32, 8, temp);
	}
	/* GainBits */
	{
		const int8 temp[] = {4, 4, 4, 4, 4, 4, 4, 4, 4, 4};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS1,
		                          10, 0x20, 8, temp);
	}
	{
		const int8 temp[] = {4, 4, 4, 4, 4, 4, 4, 4, 4, 4};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS2,
		                          10, 0x20, 8, temp);
	}
	{
		const int8 temp[] = {4, 4, 4, 4, 4, 4, 4, 4, 4, 4};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS3,
		                          10, 0x20, 8, temp);
	}
	/* Vmid and Av for {RSSI invalid PWRDET} */
	{
		const uint16 temp[] = {0x8e, 0x96, 0x96};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x08, 16, temp);
	}
	{
		const uint16 temp[] = {0x8f, 0x9f, 0x9f};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x18, 16, temp);
	}
	{
		const uint16 temp[] = {0x8f, 0x9f, 0x9f};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x28, 16, temp);
	}
	{
		const uint16 temp[] = {0x02, 0x02, 0x02};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x0c, 16, temp);
	}
	{
		const uint16 temp[] = {0x02, 0x02, 0x02};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x1c, 16, temp);
	}
	{
		const uint16 temp[] = {0x02, 0x02, 0x02};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x2c, 16, temp);
	}

	/* BW20 customizations */
	if (CHSPEC_IS20(pi->radio_chanspec) == 1) {

		if ((fc >= 4920 && fc <= 5230)) {
			/* CRS MinPwr */
			MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 67);
			MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 67);
			/* RadioNF [1/4dB] */
			MOD_PHYREG(pi, HTPHY_nvcfg0, noisevar_nf_radio_qdb_core0, 28);
			MOD_PHYREG(pi, HTPHY_nvcfg0, noisevar_nf_radio_qdb_core1, 28);
			MOD_PHYREG(pi, HTPHY_nvcfg1, noisevar_nf_radio_qdb_core2, 28);
			/* Clip1 WB Thresh */
			MOD_PHYREG(pi, HTPHY_Core0clipwbThreshold2059, clip1wbThreshold, 0x10);
			MOD_PHYREG(pi, HTPHY_Core1clipwbThreshold2059, clip1wbThreshold, 0x10);
			MOD_PHYREG(pi, HTPHY_Core2clipwbThreshold2059, clip1wbThreshold, 0x10);
			/* LNA1 Gains [dB] */
			{
				const int8 temp[] = {11, 17, 21, 25};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {11, 17, 21, 25};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {11, 17, 21, 25};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x8, 8, temp);
			}
			/* LNA2 Gains [dB] */
			{
				const int8 temp[] = {0, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {0, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {0, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x10, 8, temp);
			}

		} else if ((fc >= 5240 && fc <= 5550)) {
			/* CRS MinPwr */
			MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 69);
			MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 69);
			/* RadioNF [1/4dB] */
			MOD_PHYREG(pi, HTPHY_nvcfg0, noisevar_nf_radio_qdb_core0, 28);
			MOD_PHYREG(pi, HTPHY_nvcfg0, noisevar_nf_radio_qdb_core1, 28);
			MOD_PHYREG(pi, HTPHY_nvcfg1, noisevar_nf_radio_qdb_core2, 28);
			/* Clip1 WB Thresh */
			MOD_PHYREG(pi, HTPHY_Core0clipwbThreshold2059, clip1wbThreshold, 0x10);
			MOD_PHYREG(pi, HTPHY_Core1clipwbThreshold2059, clip1wbThreshold, 0x10);
			MOD_PHYREG(pi, HTPHY_Core2clipwbThreshold2059, clip1wbThreshold, 0x10);
			/* LNA1 Gains [dB] */
			{
				const int8 temp[] = {10, 16, 20, 24};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {10, 16, 20, 24};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {10, 16, 20, 24};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x8, 8, temp);
			}
			/* LNA2 Gains [dB] */
			{
				const int8 temp[] = {0, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {0, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {0, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x10, 8, temp);
			}

		} else if ((fc >= 5560 && fc <= 5825)) {
			/* CRS MinPwr */
			MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 70);
			MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 70);
			/* RadioNF [1/4dB] */
			MOD_PHYREG(pi, HTPHY_nvcfg0, noisevar_nf_radio_qdb_core0, 28);
			MOD_PHYREG(pi, HTPHY_nvcfg0, noisevar_nf_radio_qdb_core1, 28);
			MOD_PHYREG(pi, HTPHY_nvcfg1, noisevar_nf_radio_qdb_core2, 28);
			/* Clip1 WB Thresh */
			MOD_PHYREG(pi, HTPHY_Core0clipwbThreshold2059, clip1wbThreshold, 0x10);
			MOD_PHYREG(pi, HTPHY_Core1clipwbThreshold2059, clip1wbThreshold, 0x10);
			MOD_PHYREG(pi, HTPHY_Core2clipwbThreshold2059, clip1wbThreshold, 0x10);
			/* LNA1 Gains [dB] */
			{
				const int8 temp[] = {10, 15, 19, 23};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {10, 15, 19, 23};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {10, 15, 19, 23};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x8, 8, temp);
			}
			/* LNA2 Gains [dB] */
			{
				const int8 temp[] = {2, 9, 13, 16};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {2, 9, 13, 16};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {2, 9, 13, 16};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x10, 8, temp);
			}

		}

		/* BW40 customizations */
	} else {

		if ((fc >= 4920 && fc <= 5230)) {
			/* CRS MinPwr */
			MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 63);
			MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 63);
			/* Clip1 WB Thresh */
			MOD_PHYREG(pi, HTPHY_Core0clipwbThreshold2059, clip1wbThreshold, 0x10);
			MOD_PHYREG(pi, HTPHY_Core1clipwbThreshold2059, clip1wbThreshold, 0x10);
			MOD_PHYREG(pi, HTPHY_Core2clipwbThreshold2059, clip1wbThreshold, 0x10);
			/* LNA1 Gains [dB] */
			{
				const int8 temp[] = {10, 16, 20, 24};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {10, 16, 20, 24};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {10, 16, 20, 24};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x8, 8, temp);
			}
			/* LNA2 Gains [dB] */
			{
				const int8 temp[] = {0, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {0, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {0, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x10, 8, temp);
			}

		} else if ((fc >= 5240 && fc <= 5550)) {
			/* CRS MinPwr */
			MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 67);
			MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 67);
			/* Clip1 WB Thresh */
			MOD_PHYREG(pi, HTPHY_Core0clipwbThreshold2059, clip1wbThreshold, 0x10);
			MOD_PHYREG(pi, HTPHY_Core1clipwbThreshold2059, clip1wbThreshold, 0x10);
			MOD_PHYREG(pi, HTPHY_Core2clipwbThreshold2059, clip1wbThreshold, 0x10);
			/* LNA1 Gains [dB] */
			{
				const int8 temp[] = {9, 15, 20, 23};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {9, 15, 20, 23};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {9, 15, 20, 23};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x8, 8, temp);
			}
			/* LNA2 Gains [dB] */
			{
				const int8 temp[] = {0, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {0, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {0, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x10, 8, temp);
			}

		} else if ((fc >= 5560 && fc <= 5825)) {
			/* CRS MinPwr */
			MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 67);
			MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 67);
			/* Clip1 WB Thresh */
			MOD_PHYREG(pi, HTPHY_Core0clipwbThreshold2059, clip1wbThreshold, 0x10);
			MOD_PHYREG(pi, HTPHY_Core1clipwbThreshold2059, clip1wbThreshold, 0x10);
			MOD_PHYREG(pi, HTPHY_Core2clipwbThreshold2059, clip1wbThreshold, 0x10);
			/* LNA1 Gains [dB] */
			{
				const int8 temp[] = {9, 14, 17, 21};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {9, 14, 17, 21};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {9, 14, 17, 21};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x8, 8, temp);
			}
			/* LNA2 Gains [dB] */
			{
				const int8 temp[] = {2, 9, 13, 16};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {2, 9, 13, 16};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {2, 9, 13, 16};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x10, 8, temp);
			}

		}
	}
}

/*
 * AUTOGENERATED from htphy/subband_cust_4331B1_pciedual.xls
 * DO NOT EDIT
 */

static void
wlc_phy_subband_cust_rev1_pciedual_htphy(phy_info_t *pi)
{
	void (*subbandcust_bandx_pointer)(phy_info_t *pi, uint16 fc) = 0;
	uint16 fc;
	if (CHSPEC_CHANNEL(pi->radio_chanspec) > 14) {
		fc = CHAN5G_FREQ(CHSPEC_CHANNEL(pi->radio_chanspec));
	} else {
		fc = CHAN2G_FREQ(CHSPEC_CHANNEL(pi->radio_chanspec));
	}

	/* 2G Band Customizations */
	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		subbandcust_bandx_pointer = wlc_phy_subband_cust_rev1_pciedual_2g_htphy;
	}

	/* 5G Band Customizations */
	if (CHSPEC_IS5G(pi->radio_chanspec)) {
		subbandcust_bandx_pointer = wlc_phy_subband_cust_rev1_pciedual_5g_htphy;
	}
	ASSERT(subbandcust_bandx_pointer);
	if (subbandcust_bandx_pointer)
		subbandcust_bandx_pointer(pi, fc);

}
static void
wlc_phy_subband_cust_rev1_pciedual_2g_htphy(phy_info_t *pi, uint16 fc)
{

	/* Default customizations */
	/* CRS MinPwr */
	MOD_PHYREG(pi, HTPHY_bphycrsminpower0, bphycrsminpower0, 0x46);
	/* Hi Gain  */
	phy_utils_write_phyreg(pi, HTPHY_Core0clipHiGainCodeA2059, 0x6c);
	phy_utils_write_phyreg(pi, HTPHY_Core1clipHiGainCodeA2059, 0x6c);
	phy_utils_write_phyreg(pi, HTPHY_Core2clipHiGainCodeA2059, 0x6c);
	/* MD Gain  */
	phy_utils_write_phyreg(pi, HTPHY_Core0clipmdGainCodeA2059, 0x60);
	phy_utils_write_phyreg(pi, HTPHY_Core1clipmdGainCodeA2059, 0x60);
	phy_utils_write_phyreg(pi, HTPHY_Core2clipmdGainCodeA2059, 0x60);
	phy_utils_write_phyreg(pi, HTPHY_Core0clipmdGainCodeB2059, 0x24);
	phy_utils_write_phyreg(pi, HTPHY_Core1clipmdGainCodeB2059, 0x24);
	phy_utils_write_phyreg(pi, HTPHY_Core2clipmdGainCodeB2059, 0x24);
	/* LO Gain  */
	phy_utils_write_phyreg(pi, HTPHY_Core0cliploGainCodeA2059, 0x66);
	phy_utils_write_phyreg(pi, HTPHY_Core1cliploGainCodeA2059, 0x66);
	phy_utils_write_phyreg(pi, HTPHY_Core2cliploGainCodeA2059, 0x66);
	phy_utils_write_phyreg(pi, HTPHY_Core0cliploGainCodeB2059, 0x318);
	phy_utils_write_phyreg(pi, HTPHY_Core1cliploGainCodeB2059, 0x318);
	phy_utils_write_phyreg(pi, HTPHY_Core2cliploGainCodeB2059, 0x318);
	/* InitGCode */
	phy_utils_write_phyreg(pi, HTPHY_Core0InitGainCodeA2059, 0x6e);
	phy_utils_write_phyreg(pi, HTPHY_Core1InitGainCodeA2059, 0x6e);
	phy_utils_write_phyreg(pi, HTPHY_Core2InitGainCodeA2059, 0x6e);
	/* InitGCode */
	phy_utils_write_phyreg(pi, HTPHY_Core0InitGainCodeB2059, 0x0614);
	phy_utils_write_phyreg(pi, HTPHY_Core1InitGainCodeB2059, 0x0614);
	phy_utils_write_phyreg(pi, HTPHY_Core2InitGainCodeB2059, 0x0614);
	/* InitGain */
	{
		const uint16 temp[] = {0x6137, 0x6137, 0x6137};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_RFSEQ,
		                          3, 0x106, 16, temp);
	}
	/* EXTLNA Gains (dB) */
	{
		const int8 temp[] = {10, 10};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
		                          2, 0x0, 8, temp);
	}
	{
		const int8 temp[] = {10, 10};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
		                          2, 0x0, 8, temp);
	}
	{
		const int8 temp[] = {10, 10};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
		                          2, 0x0, 8, temp);
	}
	/* LNA2 GainBits */
	{
		const int8 temp[] = {0, 1, 1, 1};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS1,
		                          4, 0x10, 8, temp);
	}
	{
		const int8 temp[] = {0, 1, 1, 1};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS2,
		                          4, 0x10, 8, temp);
	}
	{
		const int8 temp[] = {0, 1, 1, 1};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS3,
		                          4, 0x10, 8, temp);
	}
	/* Vmid and Av for {RSSI invalid PWRDET} */
	{
		const uint16 temp[] = {0x8e, 0x96, 0x96};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x08, 16, temp);
	}
	{
		const uint16 temp[] = {0x8f, 0x9f, 0x9f};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x18, 16, temp);
	}
	{
		const uint16 temp[] = {0x8f, 0x9f, 0x9f};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x28, 16, temp);
	}
	{
		const uint16 temp[] = {0x02, 0x02, 0x02};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x0c, 16, temp);
	}
	{
		const uint16 temp[] = {0x02, 0x02, 0x02};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x1c, 16, temp);
	}
	{
		const uint16 temp[] = {0x02, 0x02, 0x02};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x2c, 16, temp);
	}

	/* BW20 customizations */
	if (CHSPEC_IS20(pi->radio_chanspec) == 1) {

		if ((fc >= 2412 && fc <= 2484)) {
			/* CRS MinPwr */
			MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 59);
			MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 59);
			/* Clip1 WB Thresh */
			MOD_PHYREG(pi, HTPHY_Core0clipwbThreshold2059, clip1wbThreshold, 4);
			MOD_PHYREG(pi, HTPHY_Core1clipwbThreshold2059, clip1wbThreshold, 4);
			MOD_PHYREG(pi, HTPHY_Core2clipwbThreshold2059, clip1wbThreshold, 4);
			/* LNA1 Gains [dB] */
			{
				const int8 temp[] = {9, 15, 19, 24};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {9, 15, 19, 24};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {9, 15, 19, 24};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x8, 8, temp);
			}
			/* LNA2 Gains [dB] */
			{
				const int8 temp[] = {-3, 7, 11, 15};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {-3, 7, 11, 15};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {-3, 7, 11, 15};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x10, 8, temp);
			}

		}

		/* BW40 customizations */
	} else {

		if ((fc >= 2412 && fc <= 2437)) {
			/* CRS MinPwr */
			MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 56);
			MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 56);
			/* LNA1 Gains [dB] */
			{
				const int8 temp[] = {7, 13, 17, 22};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {7, 13, 17, 22};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {7, 13, 17, 22};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x8, 8, temp);
			}
			/* LNA2 Gains [dB] */
			{
				const int8 temp[] = {-3, 7, 11, 15};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {-3, 7, 11, 15};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {-3, 7, 11, 15};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x10, 8, temp);
			}

		} else if ((fc >= 2442 && fc <= 2484)) {
			/* CRS MinPwr */
			MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 56);
			MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 56);
			/* LNA1 Gains [dB] */
			{
				const int8 temp[] = {8, 13, 17, 22};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {8, 13, 17, 22};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {8, 13, 17, 22};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x8, 8, temp);
			}
			/* LNA2 Gains [dB] */
			{
				const int8 temp[] = {-3, 7, 11, 15};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {-3, 7, 11, 15};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {-3, 7, 11, 15};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x10, 8, temp);
			}

		}
	}
}
static void
wlc_phy_subband_cust_rev1_pciedual_5g_htphy(phy_info_t *pi, uint16 fc)
{

	/* Default customizations */
	/* Hi Gain  */
	phy_utils_write_phyreg(pi, HTPHY_Core0clipHiGainCodeA2059, 0x96);
	phy_utils_write_phyreg(pi, HTPHY_Core1clipHiGainCodeA2059, 0x96);
	phy_utils_write_phyreg(pi, HTPHY_Core2clipHiGainCodeA2059, 0x96);
	phy_utils_write_phyreg(pi, HTPHY_Core0clipHiGainCodeB2059, 0x4);
	phy_utils_write_phyreg(pi, HTPHY_Core1clipHiGainCodeB2059, 0x4);
	phy_utils_write_phyreg(pi, HTPHY_Core2clipHiGainCodeB2059, 0x4);
	/* MD Gain  */
	phy_utils_write_phyreg(pi, HTPHY_Core0clipmdGainCodeA2059, 0x80);
	phy_utils_write_phyreg(pi, HTPHY_Core1clipmdGainCodeA2059, 0x80);
	phy_utils_write_phyreg(pi, HTPHY_Core2clipmdGainCodeA2059, 0x80);
	phy_utils_write_phyreg(pi, HTPHY_Core0clipmdGainCodeB2059, 0x14);
	phy_utils_write_phyreg(pi, HTPHY_Core1clipmdGainCodeB2059, 0x14);
	phy_utils_write_phyreg(pi, HTPHY_Core2clipmdGainCodeB2059, 0x14);
	/* LO Gain  */
	phy_utils_write_phyreg(pi, HTPHY_Core0cliploGainCodeA2059, 0x0082);
	phy_utils_write_phyreg(pi, HTPHY_Core1cliploGainCodeA2059, 0x0082);
	phy_utils_write_phyreg(pi, HTPHY_Core2cliploGainCodeA2059, 0x0082);
	phy_utils_write_phyreg(pi, HTPHY_Core0cliploGainCodeB2059, 0x308);
	phy_utils_write_phyreg(pi, HTPHY_Core1cliploGainCodeB2059, 0x308);
	phy_utils_write_phyreg(pi, HTPHY_Core2cliploGainCodeB2059, 0x308);
	/* InitGCode */
	phy_utils_write_phyreg(pi, HTPHY_Core0InitGainCodeA2059, 0x8e);
	phy_utils_write_phyreg(pi, HTPHY_Core1InitGainCodeA2059, 0x8e);
	phy_utils_write_phyreg(pi, HTPHY_Core2InitGainCodeA2059, 0x8e);
	/* InitGain */
	{
		const uint16 temp[] = {0x6247, 0x6247, 0x6247};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_RFSEQ,
		                          3, 0x106, 16, temp);
	}
	/* EXTLNA Gains (dB) */
	{
		const int8 temp[] = {8, 8};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
		                          2, 0x0, 8, temp);
	}
	{
		const int8 temp[] = {8, 8};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
		                          2, 0x0, 8, temp);
	}
	{
		const int8 temp[] = {8, 8};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
		                          2, 0x0, 8, temp);
	}
	/* MixTIA Gains */
	{
		const int8 temp[] = {-10, -7, -4, -1, 3, 3, 3, 3, 3, 3};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
		                          10, 32, 8, temp);
	}
	{
		const int8 temp[] = {-10, -7, -4, -1, 3, 3, 3, 3, 3, 3};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
		                          10, 32, 8, temp);
	}
	{
		const int8 temp[] = {-10, -7, -4, -1, 3, 3, 3, 3, 3, 3};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
		                          10, 32, 8, temp);
	}
	/* MixGainBits */
	{
		const int8 temp[] = {0, 1, 2, 3, 4, 4, 4, 4, 4, 4};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS1,
		                          10, 0x20, 8, temp);
	}
	{
		const int8 temp[] = {0, 1, 2, 3, 4, 4, 4, 4, 4, 4};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS2,
		                          10, 0x20, 8, temp);
	}
	{
		const int8 temp[] = {0, 1, 2, 3, 4, 4, 4, 4, 4, 4};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS3,
		                          10, 0x20, 8, temp);
	}
	/* Vmid and Av for {RSSI invalid PWRDET} */
	{
		const uint16 temp[] = {0x8e, 0x96, 0x96};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x08, 16, temp);
	}
	{
		const uint16 temp[] = {0x8f, 0x9f, 0x9f};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x18, 16, temp);
	}
	{
		const uint16 temp[] = {0x8f, 0x9f, 0x9f};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x28, 16, temp);
	}
	{
		const uint16 temp[] = {0x02, 0x02, 0x02};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x0c, 16, temp);
	}
	{
		const uint16 temp[] = {0x02, 0x02, 0x02};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x1c, 16, temp);
	}
	{
		const uint16 temp[] = {0x02, 0x02, 0x02};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x2c, 16, temp);
	}

	/* BW20 customizations */
	if (CHSPEC_IS20(pi->radio_chanspec) == 1) {

		if ((fc >= 4920 && fc <= 5230)) {
			/* CRS MinPwr */
			MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 63);
			MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 63);
			/* RadioNF [1/4dB] */
			MOD_PHYREG(pi, HTPHY_nvcfg0, noisevar_nf_radio_qdb_core0, 28);
			MOD_PHYREG(pi, HTPHY_nvcfg0, noisevar_nf_radio_qdb_core1, 28);
			MOD_PHYREG(pi, HTPHY_nvcfg1, noisevar_nf_radio_qdb_core2, 28);
			/* Clip1 WB Thresh */
			MOD_PHYREG(pi, HTPHY_Core0clipwbThreshold2059, clip1wbThreshold, 0xa);
			MOD_PHYREG(pi, HTPHY_Core1clipwbThreshold2059, clip1wbThreshold, 0xa);
			MOD_PHYREG(pi, HTPHY_Core2clipwbThreshold2059, clip1wbThreshold, 0xa);
			/* NB Clip  */
			phy_utils_write_phyreg(pi, HTPHY_Core0nbClipThreshold, 0xbc);
			phy_utils_write_phyreg(pi, HTPHY_Core1nbClipThreshold, 0xbc);
			phy_utils_write_phyreg(pi, HTPHY_Core2nbClipThreshold, 0xbc);
			/* LNA1 Gains [dB] */
			{
				const int8 temp[] = {11, 17, 21, 25};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {11, 17, 21, 25};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {8, 14, 18, 22};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x8, 8, temp);
			}
			/* LNA2 Gains [dB] */
			{
				const int8 temp[] = {-1, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {-1, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {-1, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x10, 8, temp);
			}

		} else if ((fc >= 5240 && fc <= 5550)) {
			/* CRS MinPwr */
			MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 61);
			MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 61);
			/* RadioNF [1/4dB] */
			MOD_PHYREG(pi, HTPHY_nvcfg0, noisevar_nf_radio_qdb_core0, 28);
			MOD_PHYREG(pi, HTPHY_nvcfg0, noisevar_nf_radio_qdb_core1, 28);
			MOD_PHYREG(pi, HTPHY_nvcfg1, noisevar_nf_radio_qdb_core2, 28);
			/* Clip1 WB Thresh */
			MOD_PHYREG(pi, HTPHY_Core0clipwbThreshold2059, clip1wbThreshold, 0xa);
			MOD_PHYREG(pi, HTPHY_Core1clipwbThreshold2059, clip1wbThreshold, 0xa);
			MOD_PHYREG(pi, HTPHY_Core2clipwbThreshold2059, clip1wbThreshold, 0xa);
			/* NB Clip  */
			phy_utils_write_phyreg(pi, HTPHY_Core0nbClipThreshold, 0xbc);
			phy_utils_write_phyreg(pi, HTPHY_Core1nbClipThreshold, 0xbc);
			phy_utils_write_phyreg(pi, HTPHY_Core2nbClipThreshold, 0xbc);
			/* LNA1 Gains [dB] */
			{
				const int8 temp[] = {11, 17, 21, 25};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {9, 15, 19, 23};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {8, 14, 18, 22};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x8, 8, temp);
			}
			/* LNA2 Gains [dB] */
			{
				const int8 temp[] = {-1, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {-1, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {-1, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x10, 8, temp);
			}

		} else if ((fc >= 5560 && fc <= 5825)) {
			/* CRS MinPwr */
			MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 60);
			MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 60);
			/* RadioNF [1/4dB] */
			MOD_PHYREG(pi, HTPHY_nvcfg0, noisevar_nf_radio_qdb_core0, 28);
			MOD_PHYREG(pi, HTPHY_nvcfg0, noisevar_nf_radio_qdb_core1, 28);
			MOD_PHYREG(pi, HTPHY_nvcfg1, noisevar_nf_radio_qdb_core2, 28);
			/* Clip1 WB Thresh */
			MOD_PHYREG(pi, HTPHY_Core0clipwbThreshold2059, clip1wbThreshold, 0x13);
			MOD_PHYREG(pi, HTPHY_Core1clipwbThreshold2059, clip1wbThreshold, 0x13);
			MOD_PHYREG(pi, HTPHY_Core2clipwbThreshold2059, clip1wbThreshold, 0x13);
			/* NB Clip  */
			phy_utils_write_phyreg(pi, HTPHY_Core0nbClipThreshold, 0xbc);
			phy_utils_write_phyreg(pi, HTPHY_Core1nbClipThreshold, 0xbc);
			phy_utils_write_phyreg(pi, HTPHY_Core2nbClipThreshold, 0xbc);
			/* LNA1 Gains [dB] */
			{
				const int8 temp[] = {11, 17, 21, 25};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {9, 15, 19, 23};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {8, 14, 18, 22};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x8, 8, temp);
			}
			/* LNA2 Gains [dB] */
			{
				const int8 temp[] = {-1, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {-1, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {-1, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x10, 8, temp);
			}

		}

		/* BW40 customizations */
	} else {

		if ((fc >= 4920 && fc <= 5230)) {
			/* CRS MinPwr */
			MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 58);
			MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 58);
			/* Clip1 WB Thresh */
			MOD_PHYREG(pi, HTPHY_Core0clipwbThreshold2059, clip1wbThreshold, 0xa);
			MOD_PHYREG(pi, HTPHY_Core1clipwbThreshold2059, clip1wbThreshold, 0xa);
			MOD_PHYREG(pi, HTPHY_Core2clipwbThreshold2059, clip1wbThreshold, 0xa);
			/* NB Clip  */
			phy_utils_write_phyreg(pi, HTPHY_Core0nbClipThreshold, 0xb4);
			phy_utils_write_phyreg(pi, HTPHY_Core1nbClipThreshold, 0xb4);
			phy_utils_write_phyreg(pi, HTPHY_Core2nbClipThreshold, 0xb4);
			/* LNA1 Gains [dB] */
			{
				const int8 temp[] = {7, 14, 18, 23};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {9, 16, 20, 25};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {7, 14, 18, 23};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x8, 8, temp);
			}
			/* LNA2 Gains [dB] */
			{
				const int8 temp[] = {2, 10, 14, 18};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {2, 10, 14, 18};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {2, 10, 14, 18};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x10, 8, temp);
			}

		} else if ((fc >= 5240 && fc <= 5550)) {
			/* CRS MinPwr */
			MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 56);
			MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 56);
			/* Clip1 WB Thresh */
			MOD_PHYREG(pi, HTPHY_Core0clipwbThreshold2059, clip1wbThreshold, 0xa);
			MOD_PHYREG(pi, HTPHY_Core1clipwbThreshold2059, clip1wbThreshold, 0xa);
			MOD_PHYREG(pi, HTPHY_Core2clipwbThreshold2059, clip1wbThreshold, 0xa);
			/* NB Clip  */
			phy_utils_write_phyreg(pi, HTPHY_Core0nbClipThreshold, 0xb4);
			phy_utils_write_phyreg(pi, HTPHY_Core1nbClipThreshold, 0xb4);
			phy_utils_write_phyreg(pi, HTPHY_Core2nbClipThreshold, 0xb4);
			/* LNA1 Gains [dB] */
			{
				const int8 temp[] = {10, 17, 21, 26};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {9, 16, 20, 25};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {7, 14, 18, 23};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x8, 8, temp);
			}
			/* LNA2 Gains [dB] */
			{
				const int8 temp[] = {2, 10, 14, 18};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {2, 10, 14, 18};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {2, 10, 14, 18};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x10, 8, temp);
			}

		} else if ((fc >= 5560 && fc <= 5825)) {
			/* CRS MinPwr */
			MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 58);
			MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 58);
			/* Clip1 WB Thresh */
			MOD_PHYREG(pi, HTPHY_Core0clipwbThreshold2059, clip1wbThreshold, 0x13);
			MOD_PHYREG(pi, HTPHY_Core1clipwbThreshold2059, clip1wbThreshold, 0x13);
			MOD_PHYREG(pi, HTPHY_Core2clipwbThreshold2059, clip1wbThreshold, 0x13);
			/* NB Clip  */
			phy_utils_write_phyreg(pi, HTPHY_Core0nbClipThreshold, 0xb4);
			phy_utils_write_phyreg(pi, HTPHY_Core1nbClipThreshold, 0xb4);
			phy_utils_write_phyreg(pi, HTPHY_Core2nbClipThreshold, 0xb4);
			/* LNA1 Gains [dB] */
			{
				const int8 temp[] = {10, 17, 21, 26};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {8, 15, 19, 24};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {7, 14, 18, 23};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x8, 8, temp);
			}
			/* LNA2 Gains [dB] */
			{
				const int8 temp[] = {2, 10, 14, 18};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {2, 10, 14, 18};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {2, 10, 14, 18};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x10, 8, temp);
			}

		}
	}
}
/*
 * AUTOGENERATED from htphy/subband_cust_4331B0.xls
 * DO NOT EDIT
 */

static void
wlc_phy_subband_cust_rev1_htphy(phy_info_t *pi)
{
	void (*subbandcust_bandx_pointer)(phy_info_t *pi, uint16 fc) = 0;
	uint16 fc;
	if (CHSPEC_CHANNEL(pi->radio_chanspec) > 14) {
		fc = CHAN5G_FREQ(CHSPEC_CHANNEL(pi->radio_chanspec));
	} else {
		fc = CHAN2G_FREQ(CHSPEC_CHANNEL(pi->radio_chanspec));
	}

	/* 2G Band Customizations */
	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		subbandcust_bandx_pointer = wlc_phy_subband_cust_rev1_2g_htphy;
	}

	/* 5G Band Customizations */
	if (CHSPEC_IS5G(pi->radio_chanspec)) {
		subbandcust_bandx_pointer = wlc_phy_subband_cust_rev1_5g_htphy;
	}

	ASSERT(subbandcust_bandx_pointer);
	if (subbandcust_bandx_pointer)
		subbandcust_bandx_pointer(pi, fc);
}
static void
wlc_phy_subband_cust_rev1_2g_htphy(phy_info_t *pi, uint16 fc)
{

	/* Default customizations */
	/* CRS MinPwr */
	MOD_PHYREG(pi, HTPHY_bphycrsminpower0, bphycrsminpower0, 0x46);
	/* InitGCode */
	phy_utils_write_phyreg(pi, HTPHY_Core0InitGainCodeA2059, 0x7e);
	phy_utils_write_phyreg(pi, HTPHY_Core1InitGainCodeA2059, 0x7e);
	phy_utils_write_phyreg(pi, HTPHY_Core2InitGainCodeA2059, 0x7e);
	/* InitGCode */
	phy_utils_write_phyreg(pi, HTPHY_Core0InitGainCodeB2059, 0x0614);
	phy_utils_write_phyreg(pi, HTPHY_Core1InitGainCodeB2059, 0x0614);
	phy_utils_write_phyreg(pi, HTPHY_Core2InitGainCodeB2059, 0x0614);
	/* InitGain */
	{
		const uint16 temp[] = {0x613f, 0x613f, 0x613f};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_RFSEQ,
		                          3, 0x106, 16, temp);
	}
	/* Vmid and Av for {RSSI invalid PWRDET} */
	{
		const uint16 temp[] = {0x8e, 0x96, 0x96};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x08, 16, temp);
	}
	{
		const uint16 temp[] = {0x8f, 0x9f, 0x9f};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x18, 16, temp);
	}
	{
		const uint16 temp[] = {0x8f, 0x9f, 0x9f};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x28, 16, temp);
	}
	{
		const uint16 temp[] = {0x02, 0x02, 0x02};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x0c, 16, temp);
	}
	{
		const uint16 temp[] = {0x02, 0x02, 0x02};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x1c, 16, temp);
	}
	{
		const uint16 temp[] = {0x02, 0x02, 0x02};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x2c, 16, temp);
	}

	/* BW20 customizations */
	if (CHSPEC_IS20(pi->radio_chanspec) == 1) {

		if ((fc >= 2412 && fc <= 2484)) {
			/* CRS MinPwr */
			MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 63);
			MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 63);
			/* Clip1 WB Thresh */
			MOD_PHYREG(pi, HTPHY_Core0clipwbThreshold2059, clip1wbThreshold, 13);
			MOD_PHYREG(pi, HTPHY_Core1clipwbThreshold2059, clip1wbThreshold, 13);
			MOD_PHYREG(pi, HTPHY_Core2clipwbThreshold2059, clip1wbThreshold, 13);
			/* LO Gain  */
			phy_utils_write_phyreg(pi, HTPHY_Core0cliploGainCodeB2059, 0x28);
			phy_utils_write_phyreg(pi, HTPHY_Core1cliploGainCodeB2059, 0x28);
			phy_utils_write_phyreg(pi, HTPHY_Core2cliploGainCodeB2059, 0x28);
			/* LNA1 Gains [dB] */
			{
				const int8 temp[] = {9, 15, 19, 24};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {9, 15, 19, 24};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {9, 15, 19, 24};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x8, 8, temp);
			}
			/* LNA2 Gains [dB] */
			{
				const int8 temp[] = {-3, 7, 11, 15};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {-3, 7, 11, 15};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {-3, 7, 11, 15};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x10, 8, temp);
			}

		}

		/* BW40 customizations */
	} else {

		if ((fc >= 2412 && fc <= 2437)) {
			/* CRS MinPwr */
			MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 58);
			MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 58);
			/* LNA1 Gains [dB] */
			{
				const int8 temp[] = {7, 13, 17, 22};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {7, 13, 17, 22};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {7, 13, 17, 22};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x8, 8, temp);
			}
			/* LNA2 Gains [dB] */
			{
				const int8 temp[] = {-3, 7, 11, 15};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {-3, 7, 11, 15};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {-3, 7, 11, 15};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x10, 8, temp);
			}

		} else if ((fc >= 2442 && fc <= 2484)) {
			/* CRS MinPwr */
			MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 58);
			MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 58);
			/* LNA1 Gains [dB] */
			{
				const int8 temp[] = {8, 13, 17, 22};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {8, 13, 17, 22};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {8, 13, 17, 22};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x8, 8, temp);
			}
			/* LNA2 Gains [dB] */
			{
				const int8 temp[] = {-3, 7, 11, 15};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {-3, 7, 11, 15};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {-3, 7, 11, 15};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x10, 8, temp);
			}

		}
	}
}
static void
wlc_phy_subband_cust_rev1_5g_htphy(phy_info_t *pi, uint16 fc)
{

	/* Default customizations */
	/* NB Clip  */
	phy_utils_write_phyreg(pi, HTPHY_Core0nbClipThreshold, 0xb4);
	phy_utils_write_phyreg(pi, HTPHY_Core1nbClipThreshold, 0xb4);
	phy_utils_write_phyreg(pi, HTPHY_Core2nbClipThreshold, 0xb4);
	/* Hi Gain  */
	phy_utils_write_phyreg(pi, HTPHY_Core0clipHiGainCodeA2059, 0x9e);
	phy_utils_write_phyreg(pi, HTPHY_Core1clipHiGainCodeA2059, 0x9e);
	phy_utils_write_phyreg(pi, HTPHY_Core2clipHiGainCodeA2059, 0x9e);
	/* MD Gain  */
	phy_utils_write_phyreg(pi, HTPHY_Core0clipmdGainCodeA2059, 0x82);
	phy_utils_write_phyreg(pi, HTPHY_Core1clipmdGainCodeA2059, 0x82);
	phy_utils_write_phyreg(pi, HTPHY_Core2clipmdGainCodeA2059, 0x82);
	phy_utils_write_phyreg(pi, HTPHY_Core0clipmdGainCodeB2059, 0x24);
	phy_utils_write_phyreg(pi, HTPHY_Core1clipmdGainCodeB2059, 0x24);
	phy_utils_write_phyreg(pi, HTPHY_Core2clipmdGainCodeB2059, 0x24);
	/* LO Gain  */
	phy_utils_write_phyreg(pi, HTPHY_Core0cliploGainCodeA2059, 0x008a);
	phy_utils_write_phyreg(pi, HTPHY_Core1cliploGainCodeA2059, 0x008a);
	phy_utils_write_phyreg(pi, HTPHY_Core2cliploGainCodeA2059, 0x008a);
	phy_utils_write_phyreg(pi, HTPHY_Core0cliploGainCodeB2059, 0x18);
	phy_utils_write_phyreg(pi, HTPHY_Core1cliploGainCodeB2059, 0x18);
	phy_utils_write_phyreg(pi, HTPHY_Core2cliploGainCodeB2059, 0x18);
	/* InitGCode */
	phy_utils_write_phyreg(pi, HTPHY_Core0InitGainCodeA2059, 0x9e);
	phy_utils_write_phyreg(pi, HTPHY_Core1InitGainCodeA2059, 0x9e);
	phy_utils_write_phyreg(pi, HTPHY_Core2InitGainCodeA2059, 0x9e);
	/* InitGCode */
	phy_utils_write_phyreg(pi, HTPHY_Core0InitGainCodeB2059, 0x0624);
	phy_utils_write_phyreg(pi, HTPHY_Core1InitGainCodeB2059, 0x0624);
	phy_utils_write_phyreg(pi, HTPHY_Core2InitGainCodeB2059, 0x0624);
	/* InitGain */
	{
		const uint16 temp[] = {0x624f, 0x624f, 0x624f};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_RFSEQ,
		                          3, 0x106, 16, temp);
	}
	/* MixTIA Gains */
	{
		const int8 temp[] = {0, 0, 0, 0, 3, 3, 3, 3, 3, 3};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
		                          10, 32, 8, temp);
	}
	{
		const int8 temp[] = {0, 0, 0, 0, 3, 3, 3, 3, 3, 3};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
		                          10, 32, 8, temp);
	}
	{
		const int8 temp[] = {0, 0, 0, 0, 3, 3, 3, 3, 3, 3};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
		                          10, 32, 8, temp);
	}
	/* GainBits */
	{
		const int8 temp[] = {3, 3, 3, 3, 4, 4, 4, 4, 4, 4};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS1,
		                          10, 0x20, 8, temp);
	}
	{
		const int8 temp[] = {3, 3, 3, 3, 4, 4, 4, 4, 4, 4};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS2,
		                          10, 0x20, 8, temp);
	}
	{
		const int8 temp[] = {3, 3, 3, 3, 4, 4, 4, 4, 4, 4};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS3,
		                          10, 0x20, 8, temp);
	}
	/* Vmid and Av for {RSSI invalid PWRDET} */
	{
		const uint16 temp[] = {0x8e, 0x96, 0x96};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x08, 16, temp);
	}
	{
		const uint16 temp[] = {0x8f, 0x9f, 0x9f};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x18, 16, temp);
	}
	{
		const uint16 temp[] = {0x8f, 0x9f, 0x9f};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x28, 16, temp);
	}
	{
		const uint16 temp[] = {0x02, 0x02, 0x02};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x0c, 16, temp);
	}
	{
		const uint16 temp[] = {0x02, 0x02, 0x02};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x1c, 16, temp);
	}
	{
		const uint16 temp[] = {0x02, 0x02, 0x02};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x2c, 16, temp);
	}

	/* BW20 customizations */
	if (CHSPEC_IS20(pi->radio_chanspec) == 1) {

		if ((fc >= 4920 && fc <= 5230)) {
			/* CRS MinPwr */
			MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 63);
			MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 63);
			/* RadioNF [1/4dB] */
			MOD_PHYREG(pi, HTPHY_nvcfg0, noisevar_nf_radio_qdb_core0, 28);
			MOD_PHYREG(pi, HTPHY_nvcfg0, noisevar_nf_radio_qdb_core1, 28);
			MOD_PHYREG(pi, HTPHY_nvcfg1, noisevar_nf_radio_qdb_core2, 28);
			/* Clip1 WB Thresh */
			MOD_PHYREG(pi, HTPHY_Core0clipwbThreshold2059, clip1wbThreshold, 0x19);
			MOD_PHYREG(pi, HTPHY_Core1clipwbThreshold2059, clip1wbThreshold, 0x19);
			MOD_PHYREG(pi, HTPHY_Core2clipwbThreshold2059, clip1wbThreshold, 0x19);
			/* LNA1 Gains [dB] */
			{
				const int8 temp[] = {11, 17, 21, 25};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {11, 17, 21, 25};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {11, 17, 21, 25};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x8, 8, temp);
			}
			/* LNA2 Gains [dB] */
			{
				const int8 temp[] = {0, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {0, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {0, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x10, 8, temp);
			}

		} else if ((fc >= 5240 && fc <= 5550)) {
			/* CRS MinPwr */
			MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 63);
			MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 63);
			/* RadioNF [1/4dB] */
			MOD_PHYREG(pi, HTPHY_nvcfg0, noisevar_nf_radio_qdb_core0, 28);
			MOD_PHYREG(pi, HTPHY_nvcfg0, noisevar_nf_radio_qdb_core1, 28);
			MOD_PHYREG(pi, HTPHY_nvcfg1, noisevar_nf_radio_qdb_core2, 28);
			/* Clip1 WB Thresh */
			MOD_PHYREG(pi, HTPHY_Core0clipwbThreshold2059, clip1wbThreshold, 0x19);
			MOD_PHYREG(pi, HTPHY_Core1clipwbThreshold2059, clip1wbThreshold, 0x19);
			MOD_PHYREG(pi, HTPHY_Core2clipwbThreshold2059, clip1wbThreshold, 0x19);
			/* LNA1 Gains [dB] */
			{
				const int8 temp[] = {10, 16, 20, 24};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {10, 16, 20, 24};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {10, 16, 20, 24};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x8, 8, temp);
			}
			/* LNA2 Gains [dB] */
			{
				const int8 temp[] = {0, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {0, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {0, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x10, 8, temp);
			}

		} else if ((fc >= 5560 && fc <= 5825)) {
			/* CRS MinPwr */
			MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 65);
			MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 65);
			/* RadioNF [1/4dB] */
			MOD_PHYREG(pi, HTPHY_nvcfg0, noisevar_nf_radio_qdb_core0, 28);
			MOD_PHYREG(pi, HTPHY_nvcfg0, noisevar_nf_radio_qdb_core1, 28);
			MOD_PHYREG(pi, HTPHY_nvcfg1, noisevar_nf_radio_qdb_core2, 28);
			/* Clip1 WB Thresh */
			MOD_PHYREG(pi, HTPHY_Core0clipwbThreshold2059, clip1wbThreshold, 0x19);
			MOD_PHYREG(pi, HTPHY_Core1clipwbThreshold2059, clip1wbThreshold, 0x19);
			MOD_PHYREG(pi, HTPHY_Core2clipwbThreshold2059, clip1wbThreshold, 0x19);
			/* LNA1 Gains [dB] */
			{
				const int8 temp[] = {10, 15, 19, 23};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {10, 15, 19, 23};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {10, 15, 19, 23};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x8, 8, temp);
			}
			/* LNA2 Gains [dB] */
			{
				const int8 temp[] = {2, 9, 13, 16};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {2, 9, 13, 16};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {2, 9, 13, 16};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x10, 8, temp);
			}

		}

		/* BW40 customizations */
	} else {

		if ((fc >= 4920 && fc <= 5230)) {
			/* CRS MinPwr */
			MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 60);
			MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 60);
			/* Clip1 WB Thresh */
			MOD_PHYREG(pi, HTPHY_Core0clipwbThreshold2059, clip1wbThreshold, 0x18);
			MOD_PHYREG(pi, HTPHY_Core1clipwbThreshold2059, clip1wbThreshold, 0x18);
			MOD_PHYREG(pi, HTPHY_Core2clipwbThreshold2059, clip1wbThreshold, 0x18);
			/* LNA1 Gains [dB] */
			{
				const int8 temp[] = {10, 16, 20, 24};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {10, 16, 20, 24};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {10, 16, 20, 24};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x8, 8, temp);
			}
			/* LNA2 Gains [dB] */
			{
				const int8 temp[] = {0, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {0, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {0, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x10, 8, temp);
			}

		} else if ((fc >= 5240 && fc <= 5550)) {
			/* CRS MinPwr */
			MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 60);
			MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 60);
			/* Clip1 WB Thresh */
			MOD_PHYREG(pi, HTPHY_Core0clipwbThreshold2059, clip1wbThreshold, 0x18);
			MOD_PHYREG(pi, HTPHY_Core1clipwbThreshold2059, clip1wbThreshold, 0x18);
			MOD_PHYREG(pi, HTPHY_Core2clipwbThreshold2059, clip1wbThreshold, 0x18);
			/* LNA1 Gains [dB] */
			{
				const int8 temp[] = {9, 15, 20, 23};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {9, 15, 20, 23};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {9, 15, 20, 23};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x8, 8, temp);
			}
			/* LNA2 Gains [dB] */
			{
				const int8 temp[] = {0, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {0, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {0, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x10, 8, temp);
			}

		} else if ((fc >= 5560 && fc <= 5825)) {
			/* CRS MinPwr */
			MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 63);
			MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 63);
			/* Clip1 WB Thresh */
			MOD_PHYREG(pi, HTPHY_Core0clipwbThreshold2059, clip1wbThreshold, 0x18);
			MOD_PHYREG(pi, HTPHY_Core1clipwbThreshold2059, clip1wbThreshold, 0x18);
			MOD_PHYREG(pi, HTPHY_Core2clipwbThreshold2059, clip1wbThreshold, 0x18);
			/* LNA1 Gains [dB] */
			{
				const int8 temp[] = {9, 14, 17, 21};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {9, 14, 17, 21};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {9, 14, 17, 21};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x8, 8, temp);
			}
			/* LNA2 Gains [dB] */
			{
				const int8 temp[] = {2, 9, 13, 16};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {2, 9, 13, 16};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {2, 9, 13, 16};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x10, 8, temp);
			}

		}
	}
}

/*
 * AUTOGENERATED from htphy/subband_cust_4331A0.xls
 * DO NOT EDIT
 */

static void
wlc_phy_subband_cust_rev0_htphy(phy_info_t *pi)
{
	void (*subbandcust_bandx_pointer)(phy_info_t *pi, uint16 fc) = 0;
	uint16 fc;
	if (CHSPEC_CHANNEL(pi->radio_chanspec) > 14) {
		fc = CHAN5G_FREQ(CHSPEC_CHANNEL(pi->radio_chanspec));
	} else {
		fc = CHAN2G_FREQ(CHSPEC_CHANNEL(pi->radio_chanspec));
	}

	/* 2G Band Customizations */
	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		subbandcust_bandx_pointer = wlc_phy_subband_cust_rev0_2g_htphy;
	}

	/* 5G Band Customizations */
	if (CHSPEC_IS5G(pi->radio_chanspec)) {
		subbandcust_bandx_pointer = wlc_phy_subband_cust_rev0_5g_htphy;
	}
	ASSERT(subbandcust_bandx_pointer);
	if (subbandcust_bandx_pointer)
		subbandcust_bandx_pointer(pi, fc);
}
static void
wlc_phy_subband_cust_rev0_2g_htphy(phy_info_t *pi, uint16 fc)
{

	/* Default customizations */
	/* CRS MinPwr */

	MOD_PHYREG(pi, HTPHY_bphycrsminpower0, bphycrsminpower0, 0x46);
	/* InitGCode */

	phy_utils_write_phyreg(pi, HTPHY_Core0InitGainCodeB2059, 0x0614);
	phy_utils_write_phyreg(pi, HTPHY_Core1InitGainCodeB2059, 0x0614);
	phy_utils_write_phyreg(pi, HTPHY_Core2InitGainCodeB2059, 0x0614);

	/* InitGain */
	{
		const uint16 temp[] = {0x613f, 0x613f, 0x613f};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_RFSEQ,
		                          3, 0x106, 16, temp);
	}

	/* Vmid and Av for {RSSI invalid PWRDET} */
	{
		const uint16 temp[] = {0x8e, 0x96, 0x96};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x08, 16, temp);
	}
	{
		const uint16 temp[] = {0x8f, 0x9f, 0x9f};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x18, 16, temp);
	}
	{
		const uint16 temp[] = {0x8f, 0x9f, 0x9f};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x28, 16, temp);
	}
	{
		const uint16 temp[] = {0x02, 0x02, 0x02};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x0c, 16, temp);
	}
	{
		const uint16 temp[] = {0x02, 0x02, 0x02};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x1c, 16, temp);
	}
	{
		const uint16 temp[] = {0x02, 0x02, 0x02};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x2c, 16, temp);
	}

	/* BW20 customizations */
	if (CHSPEC_IS20(pi->radio_chanspec) == 1) {

		if ((fc >= 2412 && fc <= 2484)) {
			/* CRS MinPwr */
			MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 62);
			MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 62);
			/* Clip1 WB Thresh */
			MOD_PHYREG(pi, HTPHY_Core0clipwbThreshold2059, clip1wbThreshold, 13);
			MOD_PHYREG(pi, HTPHY_Core1clipwbThreshold2059, clip1wbThreshold, 13);
			MOD_PHYREG(pi, HTPHY_Core2clipwbThreshold2059, clip1wbThreshold, 13);
			/* LNA1 Gains [dB] */
			{
				const int8 temp[] = {12, 16, 20, 26};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {12, 16, 20, 26};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {12, 16, 20, 26};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x8, 8, temp);
			}
			/* LNA2 Gains [dB] */
			{
				const int8 temp[] = {-3, 6, 11, 15};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {-3, 6, 11, 15};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {-3, 6, 11, 15};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x10, 8, temp);
			}

		}

		/* BW40 customizations */
	} else {

		if ((fc >= 2412 && fc <= 2484)) {
			/* CRS MinPwr */
			MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 60);
			MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 60);
			/* LNA1 Gains [dB] */
			{
				const int8 temp[] = {10, 14, 18, 24};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {10, 14, 18, 24};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {10, 14, 18, 24};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x8, 8, temp);
			}
			/* LNA2 Gains [dB] */
			{
				const int8 temp[] = {-3, 6, 11, 15};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {-3, 6, 11, 15};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {-3, 6, 11, 15};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x10, 8, temp);
			}

		}
	}
}
static void
wlc_phy_subband_cust_rev0_5g_htphy(phy_info_t *pi, uint16 fc)
{

	/* Default customizations */
	/* NB Clip  */
	phy_utils_write_phyreg(pi, HTPHY_Core0nbClipThreshold, 0xff);
	phy_utils_write_phyreg(pi, HTPHY_Core1nbClipThreshold, 0xff);
	phy_utils_write_phyreg(pi, HTPHY_Core2nbClipThreshold, 0xff);
	/* Hi Gain  */
	phy_utils_write_phyreg(pi, HTPHY_Core0clipHiGainCodeA2059, 0x9e);
	phy_utils_write_phyreg(pi, HTPHY_Core1clipHiGainCodeA2059, 0x9e);
	phy_utils_write_phyreg(pi, HTPHY_Core2clipHiGainCodeA2059, 0x9e);
	/* MD Gain  */
	phy_utils_write_phyreg(pi, HTPHY_Core0clipmdGainCodeA2059, 0x82);
	phy_utils_write_phyreg(pi, HTPHY_Core1clipmdGainCodeA2059, 0x82);
	phy_utils_write_phyreg(pi, HTPHY_Core2clipmdGainCodeA2059, 0x82);
	phy_utils_write_phyreg(pi, HTPHY_Core0clipmdGainCodeB2059, 0x24);
	phy_utils_write_phyreg(pi, HTPHY_Core1clipmdGainCodeB2059, 0x24);
	phy_utils_write_phyreg(pi, HTPHY_Core2clipmdGainCodeB2059, 0x24);
	/* LO Gain  */
	phy_utils_write_phyreg(pi, HTPHY_Core0cliploGainCodeA2059, 0x008a);
	phy_utils_write_phyreg(pi, HTPHY_Core1cliploGainCodeA2059, 0x008a);
	phy_utils_write_phyreg(pi, HTPHY_Core2cliploGainCodeA2059, 0x008a);
	phy_utils_write_phyreg(pi, HTPHY_Core0cliploGainCodeB2059, 0x8);
	phy_utils_write_phyreg(pi, HTPHY_Core1cliploGainCodeB2059, 0x8);
	phy_utils_write_phyreg(pi, HTPHY_Core2cliploGainCodeB2059, 0x8);
	/* InitGCode */
	phy_utils_write_phyreg(pi, HTPHY_Core0InitGainCodeA2059, 0x9e);
	phy_utils_write_phyreg(pi, HTPHY_Core1InitGainCodeA2059, 0x9e);
	phy_utils_write_phyreg(pi, HTPHY_Core2InitGainCodeA2059, 0x9e);
	/* InitGain */
	{
		const uint16 temp[] = {0x624f, 0x624f, 0x624f};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_RFSEQ,
		                          3, 0x106, 16, temp);
	}
	/* MixTIA Gains */
	{
		const int8 temp[] = {3, 3, 3, 3, 3, 3, 3, 3, 3, 3};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
		                          10, 32, 8, temp);
	}
	{
		const int8 temp[] = {3, 3, 3, 3, 3, 3, 3, 3, 3, 3};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
		                          10, 32, 8, temp);
	}
	{
		const int8 temp[] = {3, 3, 3, 3, 3, 3, 3, 3, 3, 3};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
		                          10, 32, 8, temp);
	}
	/* GainBits */
	{
		const int8 temp[] = {4, 4, 4, 4, 4, 4, 4, 4, 4, 4};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS1,
		                          10, 0x20, 8, temp);
	}
	{
		const int8 temp[] = {4, 4, 4, 4, 4, 4, 4, 4, 4, 4};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS2,
		                          10, 0x20, 8, temp);
	}
	{
		const int8 temp[] = {4, 4, 4, 4, 4, 4, 4, 4, 4, 4};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS3,
		                          10, 0x20, 8, temp);
	}
	/* Vmid and Av for {RSSI invalid PWRDET} */
	{
		const uint16 temp[] = {0x8e, 0x96, 0x96};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x08, 16, temp);
	}
	{
		const uint16 temp[] = {0x8f, 0x9f, 0x9f};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x18, 16, temp);
	}
	{
		const uint16 temp[] = {0x8f, 0x9f, 0x9f};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x28, 16, temp);
	}
	{
		const uint16 temp[] = {0x02, 0x02, 0x02};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x0c, 16, temp);
	}
	{
		const uint16 temp[] = {0x02, 0x02, 0x02};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x1c, 16, temp);
	}
	{
		const uint16 temp[] = {0x02, 0x02, 0x02};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x2c, 16, temp);
	}

	/* BW20 customizations */
	if (CHSPEC_IS20(pi->radio_chanspec) == 1) {

		if ((fc >= 4920 && fc <= 5230)) {
			/* CRS MinPwr */
			MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 62);
			MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 62);
			/* RadioNF [1/4dB] */
			MOD_PHYREG(pi, HTPHY_nvcfg0, noisevar_nf_radio_qdb_core0, 28);
			MOD_PHYREG(pi, HTPHY_nvcfg0, noisevar_nf_radio_qdb_core1, 28);
			MOD_PHYREG(pi, HTPHY_nvcfg1, noisevar_nf_radio_qdb_core2, 28);
			/* Clip1 WB Thresh */
			MOD_PHYREG(pi, HTPHY_Core0clipwbThreshold2059, clip1wbThreshold, 0x19);
			MOD_PHYREG(pi, HTPHY_Core1clipwbThreshold2059, clip1wbThreshold, 0x19);
			MOD_PHYREG(pi, HTPHY_Core2clipwbThreshold2059, clip1wbThreshold, 0x19);
			/* LNA1 Gains [dB] */
			{
				const int8 temp[] = {14, 19, 24, 28};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {14, 19, 24, 28};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {14, 19, 24, 28};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x8, 8, temp);
			}
			/* LNA2 Gains [dB] */
			{
				const int8 temp[] = {0, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {0, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {0, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x10, 8, temp);
			}

		} else if ((fc >= 5240 && fc <= 5550)) {
			/* CRS MinPwr */
			MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 63);
			MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 63);
			/* RadioNF [1/4dB] */
			MOD_PHYREG(pi, HTPHY_nvcfg0, noisevar_nf_radio_qdb_core0, 28);
			MOD_PHYREG(pi, HTPHY_nvcfg0, noisevar_nf_radio_qdb_core1, 28);
			MOD_PHYREG(pi, HTPHY_nvcfg1, noisevar_nf_radio_qdb_core2, 28);
			/* Clip1 WB Thresh */
			MOD_PHYREG(pi, HTPHY_Core0clipwbThreshold2059, clip1wbThreshold, 0x19);
			MOD_PHYREG(pi, HTPHY_Core1clipwbThreshold2059, clip1wbThreshold, 0x19);
			MOD_PHYREG(pi, HTPHY_Core2clipwbThreshold2059, clip1wbThreshold, 0x19);
			/* LNA1 Gains [dB] */
			{
				const int8 temp[] = {15, 20, 24, 28};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {15, 20, 24, 28};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {15, 20, 24, 28};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x8, 8, temp);
			}
			/* LNA2 Gains [dB] */
			{
				const int8 temp[] = {0, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {0, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {0, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x10, 8, temp);
			}

		} else if ((fc >= 5560 && fc <= 5825)) {
			/* CRS MinPwr */
			MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 67);
			MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 67);
			/* RadioNF [1/4dB] */
			MOD_PHYREG(pi, HTPHY_nvcfg0, noisevar_nf_radio_qdb_core0, 28);
			MOD_PHYREG(pi, HTPHY_nvcfg0, noisevar_nf_radio_qdb_core1, 28);
			MOD_PHYREG(pi, HTPHY_nvcfg1, noisevar_nf_radio_qdb_core2, 28);
			/* Clip1 WB Thresh */
			MOD_PHYREG(pi, HTPHY_Core0clipwbThreshold2059, clip1wbThreshold, 0x19);
			MOD_PHYREG(pi, HTPHY_Core1clipwbThreshold2059, clip1wbThreshold, 0x19);
			MOD_PHYREG(pi, HTPHY_Core2clipwbThreshold2059, clip1wbThreshold, 0x19);
			/* LNA1 Gains [dB] */
			{
				const int8 temp[] = {14, 19, 22, 26};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {14, 19, 22, 26};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {14, 19, 22, 26};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x8, 8, temp);
			}
			/* LNA2 Gains [dB] */
			{
				const int8 temp[] = {2, 9, 13, 16};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {2, 9, 13, 16};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {2, 9, 13, 16};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x10, 8, temp);
			}

		}

		/* BW40 customizations */
	} else {

		if ((fc >= 4920 && fc <= 5230)) {
			/* CRS MinPwr */
			MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 59);
			MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 59);
			/* Clip1 WB Thresh */
			MOD_PHYREG(pi, HTPHY_Core0clipwbThreshold2059, clip1wbThreshold, 0x14);
			MOD_PHYREG(pi, HTPHY_Core1clipwbThreshold2059, clip1wbThreshold, 0x14);
			MOD_PHYREG(pi, HTPHY_Core2clipwbThreshold2059, clip1wbThreshold, 0x14);
			/* LNA1 Gains [dB] */
			{
				const int8 temp[] = {13, 18, 22, 26};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {13, 18, 22, 26};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {13, 18, 22, 26};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x8, 8, temp);
			}
			/* LNA2 Gains [dB] */
			{
				const int8 temp[] = {0, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {0, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {0, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x10, 8, temp);
			}

		} else if ((fc >= 5240 && fc <= 5550)) {
			/* CRS MinPwr */
			MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 61);
			MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 61);
			/* Clip1 WB Thresh */
			MOD_PHYREG(pi, HTPHY_Core0clipwbThreshold2059, clip1wbThreshold, 0x14);
			MOD_PHYREG(pi, HTPHY_Core1clipwbThreshold2059, clip1wbThreshold, 0x14);
			MOD_PHYREG(pi, HTPHY_Core2clipwbThreshold2059, clip1wbThreshold, 0x14);
			/* LNA1 Gains [dB] */
			{
				const int8 temp[] = {13, 18, 22, 26};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {13, 18, 22, 26};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {13, 18, 22, 26};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x8, 8, temp);
			}
			/* LNA2 Gains [dB] */
			{
				const int8 temp[] = {0, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {0, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {0, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x10, 8, temp);
			}

		} else if ((fc >= 5560 && fc <= 5825)) {
			/* CRS MinPwr */
			MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 64);
			MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 64);
			/* Clip1 WB Thresh */
			MOD_PHYREG(pi, HTPHY_Core0clipwbThreshold2059, clip1wbThreshold, 0x14);
			MOD_PHYREG(pi, HTPHY_Core1clipwbThreshold2059, clip1wbThreshold, 0x14);
			MOD_PHYREG(pi, HTPHY_Core2clipwbThreshold2059, clip1wbThreshold, 0x14);
			/* LNA1 Gains [dB] */
			{
				const int8 temp[] = {12, 16, 20, 24};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {12, 16, 20, 24};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {12, 16, 20, 24};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x8, 8, temp);
			}
			/* LNA2 Gains [dB] */
			{
				const int8 temp[] = {2, 9, 13, 16};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {2, 9, 13, 16};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {2, 9, 13, 16};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x10, 8, temp);
			}

		}
	}
}

/*
 * AUTOGENERATED from htphy/subband_cust_4331B1_x33.xls
 * DO NOT EDIT
 */

static void
wlc_phy_subband_cust_rev1_x33_htphy(phy_info_t *pi)
{
	void (*subbandcust_bandx_pointer)(phy_info_t *pi, uint16 fc) = 0;
	uint16 fc;
	if (CHSPEC_CHANNEL(pi->radio_chanspec) > 14) {
		fc = CHAN5G_FREQ(CHSPEC_CHANNEL(pi->radio_chanspec));
	} else {
		fc = CHAN2G_FREQ(CHSPEC_CHANNEL(pi->radio_chanspec));
	}

	/* 2G Band Customizations */
	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		subbandcust_bandx_pointer = wlc_phy_subband_cust_rev1_x33_2g_htphy;
	}

	/* 5G Band Customizations */
	if (CHSPEC_IS5G(pi->radio_chanspec)) {
		subbandcust_bandx_pointer = wlc_phy_subband_cust_rev1_x33_5g_htphy;
	}
	ASSERT(subbandcust_bandx_pointer);
	if (subbandcust_bandx_pointer)
		subbandcust_bandx_pointer(pi, fc);
}
static void
wlc_phy_subband_cust_rev1_x33_2g_htphy(phy_info_t *pi, uint16 fc)
{

	/* Default customizations */
	/* CRS MinPwr */
	MOD_PHYREG(pi, HTPHY_bphycrsminpower0, bphycrsminpower0, 0x46);
	MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 56);
	MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 56);
	/* RadioNF [1/4dB] */
	MOD_PHYREG(pi, HTPHY_nvcfg0, noisevar_nf_radio_qdb_core0, 0xe);
	MOD_PHYREG(pi, HTPHY_nvcfg0, noisevar_nf_radio_qdb_core1, 0xe);
	MOD_PHYREG(pi, HTPHY_nvcfg1, noisevar_nf_radio_qdb_core2, 0xe);
	/* Clip1 WB Thresh */
	MOD_PHYREG(pi, HTPHY_Core0clipwbThreshold2059, clip1wbThreshold, 0x04);
	MOD_PHYREG(pi, HTPHY_Core1clipwbThreshold2059, clip1wbThreshold, 0x04);
	MOD_PHYREG(pi, HTPHY_Core2clipwbThreshold2059, clip1wbThreshold, 0x04);
	/* NB Clip  */
	phy_utils_write_phyreg(pi, HTPHY_Core0nbClipThreshold, 0x84);
	phy_utils_write_phyreg(pi, HTPHY_Core1nbClipThreshold, 0x84);
	phy_utils_write_phyreg(pi, HTPHY_Core2nbClipThreshold, 0x84);
	/* Hi Gain  */
	phy_utils_write_phyreg(pi, HTPHY_Core0clipHiGainCodeA2059, 0x2e);
	phy_utils_write_phyreg(pi, HTPHY_Core1clipHiGainCodeA2059, 0x2e);
	phy_utils_write_phyreg(pi, HTPHY_Core2clipHiGainCodeA2059, 0x2e);
	phy_utils_write_phyreg(pi, HTPHY_Core0clipHiGainCodeB2059, 0x004);
	phy_utils_write_phyreg(pi, HTPHY_Core1clipHiGainCodeB2059, 0x004);
	phy_utils_write_phyreg(pi, HTPHY_Core2clipHiGainCodeB2059, 0x004);
	/* MD Gain  */
	phy_utils_write_phyreg(pi, HTPHY_Core0clipmdGainCodeA2059, 0x20);
	phy_utils_write_phyreg(pi, HTPHY_Core1clipmdGainCodeA2059, 0x20);
	phy_utils_write_phyreg(pi, HTPHY_Core2clipmdGainCodeA2059, 0x20);
	phy_utils_write_phyreg(pi, HTPHY_Core0clipmdGainCodeB2059, 0x304);
	phy_utils_write_phyreg(pi, HTPHY_Core1clipmdGainCodeB2059, 0x304);
	phy_utils_write_phyreg(pi, HTPHY_Core2clipmdGainCodeB2059, 0x304);
	/* LO Gain  */
	phy_utils_write_phyreg(pi, HTPHY_Core0cliploGainCodeA2059, 0x24);
	phy_utils_write_phyreg(pi, HTPHY_Core1cliploGainCodeA2059, 0x24);
	phy_utils_write_phyreg(pi, HTPHY_Core2cliploGainCodeA2059, 0x24);
	phy_utils_write_phyreg(pi, HTPHY_Core0cliploGainCodeB2059, 0x208);
	phy_utils_write_phyreg(pi, HTPHY_Core1cliploGainCodeB2059, 0x208);
	phy_utils_write_phyreg(pi, HTPHY_Core2cliploGainCodeB2059, 0x208);
	/* InitGCode */
	phy_utils_write_phyreg(pi, HTPHY_Core0InitGainCodeA2059, 0x2e);
	phy_utils_write_phyreg(pi, HTPHY_Core1InitGainCodeA2059, 0x2e);
	phy_utils_write_phyreg(pi, HTPHY_Core2InitGainCodeA2059, 0x2e);
	phy_utils_write_phyreg(pi, HTPHY_Core0InitGainCodeB2059, 0x524);
	phy_utils_write_phyreg(pi, HTPHY_Core1InitGainCodeB2059, 0x524);
	phy_utils_write_phyreg(pi, HTPHY_Core2InitGainCodeB2059, 0x524);
	/* InitGain */
	{
		const uint16 temp[] = {0x5217, 0x5217, 0x5217};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_RFSEQ,
		                          3, 0x106, 16, temp);
	}
	/* EXTLNA Gains (dB) */
	{
		const int8 temp[] = {16, 16};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
		                          2, 0x0, 8, temp);
	}
	{
		const int8 temp[] = {16, 16};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
		                          2, 0x0, 8, temp);
	}
	{
		const int8 temp[] = {16, 16};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
		                          2, 0x0, 8, temp);
	}
	/* LNA1 Gains [dB] */
	{
		const int8 temp[] = {11, 17, 21, 26};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
		                          4, 0x8, 8, temp);
	}
	{
		const int8 temp[] = {11, 17, 21, 26};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
		                          4, 0x8, 8, temp);
	}
	{
		const int8 temp[] = {11, 17, 21, 26};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
		                          4, 0x8, 8, temp);
	}
	/* LNA2 Gains [dB] */
	{
		const int8 temp[] = {-3, 7, 7, 7};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
		                          4, 0x10, 8, temp);
	}
	{
		const int8 temp[] = {-3, 7, 7, 7};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
		                          4, 0x10, 8, temp);
	}
	{
		const int8 temp[] = {-3, 7, 7, 7};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
		                          4, 0x10, 8, temp);
	}
	/* MixTIA Gains */
	{
		const int8 temp[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
		                          10, 32, 8, temp);
	}
	{
		const int8 temp[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
		                          10, 32, 8, temp);
	}
	{
		const int8 temp[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
		                          10, 32, 8, temp);
	}
	/* Lna2GainBits */
	{
		const int8 temp[] = {0, 1, 1, 1};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS1,
		                          4, 0x10, 8, temp);
	}
	{
		const int8 temp[] = {0, 1, 1, 1};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS2,
		                          4, 0x10, 8, temp);
	}
	{
		const int8 temp[] = {0, 1, 1, 1};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS3,
		                          4, 0x10, 8, temp);
	}
	/* MixGainBits */
	{
		const int8 temp[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS1,
		                          10, 0x20, 8, temp);
	}
	{
		const int8 temp[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS2,
		                          10, 0x20, 8, temp);
	}
	{
		const int8 temp[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS3,
		                          10, 0x20, 8, temp);
	}
	/* Vmid and Av for {RSSI invalid PWRDET} */
	{
		const uint16 temp[] = {0x8e, 0x96, 0x96};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x08, 16, temp);
	}
	{
		const uint16 temp[] = {0x8f, 0x9f, 0x9f};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x18, 16, temp);
	}
	{
		const uint16 temp[] = {0x8f, 0x9f, 0x9f};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x28, 16, temp);
	}
	{
		const uint16 temp[] = {0x2, 0x2, 0x2};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x0c, 16, temp);
	}
	{
		const uint16 temp[] = {0x2, 0x2, 0x2};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x1c, 16, temp);
	}
	{
		const uint16 temp[] = {0x2, 0x2, 0x2};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x2c, 16, temp);
	}

	/* BW20 customizations */
	if (CHSPEC_IS20(pi->radio_chanspec) == 1) {

		if ((fc >= 2412 && fc <= 2484)) {
		}

		/* BW40 customizations */
	} else {

		if ((fc >= 2412 && fc <= 2437)) {
		}
	}
}
static void
wlc_phy_subband_cust_rev1_x33_5g_htphy(phy_info_t *pi, uint16 fc)
{

	/* Default customizations */
	/* CRS MinPwr */
	MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 60);
	MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 60);
	/* RadioNF [1/4dB] */
	MOD_PHYREG(pi, HTPHY_nvcfg0, noisevar_nf_radio_qdb_core0, 0x1c);
	MOD_PHYREG(pi, HTPHY_nvcfg0, noisevar_nf_radio_qdb_core1, 0x1c);
	MOD_PHYREG(pi, HTPHY_nvcfg1, noisevar_nf_radio_qdb_core2, 0x1c);
	/* Clip1 WB Thresh */
	MOD_PHYREG(pi, HTPHY_Core0clipwbThreshold2059, clip1wbThreshold, 0x0F);
	MOD_PHYREG(pi, HTPHY_Core1clipwbThreshold2059, clip1wbThreshold, 0x11);
	MOD_PHYREG(pi, HTPHY_Core2clipwbThreshold2059, clip1wbThreshold, 0x08);
	/* NB Clip  */
	phy_utils_write_phyreg(pi, HTPHY_Core0nbClipThreshold, 0xa1);
	phy_utils_write_phyreg(pi, HTPHY_Core1nbClipThreshold, 0xa1);
	phy_utils_write_phyreg(pi, HTPHY_Core2nbClipThreshold, 0xa1);
	/* Hi Gain  */
	phy_utils_write_phyreg(pi, HTPHY_Core0clipHiGainCodeA2059, 0x56);
	phy_utils_write_phyreg(pi, HTPHY_Core1clipHiGainCodeA2059, 0x56);
	phy_utils_write_phyreg(pi, HTPHY_Core2clipHiGainCodeA2059, 0x56);
	phy_utils_write_phyreg(pi, HTPHY_Core0clipHiGainCodeB2059, 0x004);
	phy_utils_write_phyreg(pi, HTPHY_Core1clipHiGainCodeB2059, 0x004);
	phy_utils_write_phyreg(pi, HTPHY_Core2clipHiGainCodeB2059, 0x004);
	/* MD Gain  */
	phy_utils_write_phyreg(pi, HTPHY_Core0clipmdGainCodeA2059, 0x42);
	phy_utils_write_phyreg(pi, HTPHY_Core1clipmdGainCodeA2059, 0x42);
	phy_utils_write_phyreg(pi, HTPHY_Core2clipmdGainCodeA2059, 0x42);
	phy_utils_write_phyreg(pi, HTPHY_Core0clipmdGainCodeB2059, 0x204);
	phy_utils_write_phyreg(pi, HTPHY_Core1clipmdGainCodeB2059, 0x204);
	phy_utils_write_phyreg(pi, HTPHY_Core2clipmdGainCodeB2059, 0x204);
	/* LO Gain  */
	phy_utils_write_phyreg(pi, HTPHY_Core0cliploGainCodeA2059, 0x56);
	phy_utils_write_phyreg(pi, HTPHY_Core1cliploGainCodeA2059, 0x56);
	phy_utils_write_phyreg(pi, HTPHY_Core2cliploGainCodeA2059, 0x56);
	phy_utils_write_phyreg(pi, HTPHY_Core0cliploGainCodeB2059, 0x008);
	phy_utils_write_phyreg(pi, HTPHY_Core1cliploGainCodeB2059, 0x008);
	phy_utils_write_phyreg(pi, HTPHY_Core2cliploGainCodeB2059, 0x008);
	/* InitGCode */
	phy_utils_write_phyreg(pi, HTPHY_Core0InitGainCodeA2059, 0x56);
	phy_utils_write_phyreg(pi, HTPHY_Core1InitGainCodeA2059, 0x56);
	phy_utils_write_phyreg(pi, HTPHY_Core2InitGainCodeA2059, 0x56);
	phy_utils_write_phyreg(pi, HTPHY_Core0InitGainCodeB2059, 0x524);
	phy_utils_write_phyreg(pi, HTPHY_Core1InitGainCodeB2059, 0x524);
	phy_utils_write_phyreg(pi, HTPHY_Core2InitGainCodeB2059, 0x524);
	/* InitGain */
	{
		const uint16 temp[] = {0x522b, 0x522b, 0x522b};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_RFSEQ,
		                          3, 0x106, 16, temp);
	}
	/* EXTLNA Gains (dB) */
	{
		const int8 temp[] = {15, 15};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
		                          2, 0x0, 8, temp);
	}
	{
		const int8 temp[] = {15, 15};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
		                          2, 0x0, 8, temp);
	}
	{
		const int8 temp[] = {15, 15};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
		                          2, 0x0, 8, temp);
	}
	/* LNA1 Gains [dB] */
	{
		const int8 temp[] = {12, 18, 22, 26};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
		                          4, 0x8, 8, temp);
	}
	{
		const int8 temp[] = {12, 18, 22, 26};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
		                          4, 0x8, 8, temp);
	}
	{
		const int8 temp[] = {12, 18, 22, 26};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
		                          4, 0x8, 8, temp);
	}
	/* LNA2 Gains [dB] */
	{
		const int8 temp[] = {-1, 7, 11, 11};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
		                          4, 0x10, 8, temp);
	}
	{
		const int8 temp[] = {-1, 7, 11, 11};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
		                          4, 0x10, 8, temp);
	}
	{
		const int8 temp[] = {-1, 7, 11, 11};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
		                          4, 0x10, 8, temp);
	}
	/* MixTIA Gains */
	{
		const int8 temp[] = {-6, -6, -3, -3, -3, -3, -3, -3, -3, -3, -3};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
		                          11, 32, 8, temp);
	}
	{
		const int8 temp[] = {-6, -6, -3, -3, -3, -3, -3, -3, -3, -3, -3};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
		                          11, 32, 8, temp);
	}
	{
		const int8 temp[] = {-6, -6, -3, -3, -3, -3, -3, -3, -3, -3, -3};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
		                          11, 32, 8, temp);
	}
	/* Lna2GainBits */
	{
		const int8 temp[] = {0, 1, 2, 2};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS1,
		                          4, 0x10, 8, temp);
	}
	{
		const int8 temp[] = {0, 1, 2, 2};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS2,
		                          4, 0x10, 8, temp);
	}
	{
		const int8 temp[] = {0, 1, 2, 2};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS3,
		                          4, 0x10, 8, temp);
	}
	/* MixGainBits */
	{
		const int8 temp[] = {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS1,
		                          11, 0x20, 8, temp);
	}
	{
		const int8 temp[] = {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS2,
		                          11, 0x20, 8, temp);
	}
	{
		const int8 temp[] = {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS3,
		                          11, 0x20, 8, temp);
	}
	/* Vmid and Av for {RSSI invalid PWRDET} */
	{
		const uint16 temp[] = {0x8e, 0x96, 0x96};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x08, 16, temp);
	}
	{
		const uint16 temp[] = {0x8f, 0x9f, 0x9f};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x18, 16, temp);
	}
	{
		const uint16 temp[] = {0x8f, 0x9f, 0x9f};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x28, 16, temp);
	}
	{
		const uint16 temp[] = {0x2, 0x2, 0x2};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x0c, 16, temp);
	}
	{
		const uint16 temp[] = {0x2, 0x2, 0x2};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x1c, 16, temp);
	}
	{
		const uint16 temp[] = {0x2, 0x2, 0x2};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x2c, 16, temp);
	}

	/* BW20 customizations */
	if (CHSPEC_IS20(pi->radio_chanspec) == 1) {

		if ((fc >= 4920 && fc <= 5320)) {
			/* EXTLNA Gains (dB) */
			{
				const int8 temp[] = {13, 13};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          2, 0x0, 8, temp);
			}
			{
				const int8 temp[] = {13, 13};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          2, 0x0, 8, temp);
			}
			{
				const int8 temp[] = {13, 13};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          2, 0x0, 8, temp);
			}
			/* LNA1 Gains [dB] */
			{
				const int8 temp[] = {14, 20, 24, 28};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {14, 20, 24, 28};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {14, 20, 24, 28};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x8, 8, temp);
			}

		}

		/* BW40 customizations */
	} else {

		if ((fc >= 4920 && fc <= 5320)) {
			/* Clip1 WB Thresh */
			MOD_PHYREG(pi, HTPHY_Core0clipwbThreshold2059, clip1wbThreshold, 0x0B);
			MOD_PHYREG(pi, HTPHY_Core1clipwbThreshold2059, clip1wbThreshold, 0x10);
			MOD_PHYREG(pi, HTPHY_Core2clipwbThreshold2059, clip1wbThreshold, 0x05);
			/* EXTLNA Gains (dB) */
			{
				const int8 temp[] = {13, 13};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          2, 0x0, 8, temp);
			}
			{
				const int8 temp[] = {13, 13};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          2, 0x0, 8, temp);
			}
			{
				const int8 temp[] = {13, 13};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          2, 0x0, 8, temp);
			}
			/* LNA1 Gains [dB] */
			{
				const int8 temp[] = {14, 20, 24, 28};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {14, 20, 24, 28};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {14, 20, 24, 28};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x8, 8, temp);
			}

		} else if ((fc >= 5330 && fc <= 5825)) {
			/* Clip1 WB Thresh */
			MOD_PHYREG(pi, HTPHY_Core0clipwbThreshold2059, clip1wbThreshold, 0x0B);
			MOD_PHYREG(pi, HTPHY_Core1clipwbThreshold2059, clip1wbThreshold, 0x10);
			MOD_PHYREG(pi, HTPHY_Core2clipwbThreshold2059, clip1wbThreshold, 0x05);

		}
	}
}
/*
 * AUTOGENERATED from htphy/subband_cust_4331B1_cs.xls
 * DO NOT EDIT
 */

static void
wlc_phy_subband_cust_rev1_cs_htphy(phy_info_t *pi)
{
	void (*subbandcust_bandx_pointer)(phy_info_t *pi, uint16 fc) = 0;
	uint16 fc;
	if (CHSPEC_CHANNEL(pi->radio_chanspec) > 14) {
		fc = CHAN5G_FREQ(CHSPEC_CHANNEL(pi->radio_chanspec));
	} else {
		fc = CHAN2G_FREQ(CHSPEC_CHANNEL(pi->radio_chanspec));
	}

	/* 2G Band Customizations */
	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		subbandcust_bandx_pointer = wlc_phy_subband_cust_rev1_cs_2g_htphy;
	}

	/* 5G Band Customizations */
	if (CHSPEC_IS5G(pi->radio_chanspec)) {
		subbandcust_bandx_pointer = wlc_phy_subband_cust_rev1_cs_5g_htphy;
	}
	ASSERT(subbandcust_bandx_pointer);
	if (subbandcust_bandx_pointer)
		subbandcust_bandx_pointer(pi, fc);
}
static void
wlc_phy_subband_cust_rev1_cs_2g_htphy(phy_info_t *pi, uint16 fc)
{

	/* Default customizations */
	/* CRS MinPwr */
	MOD_PHYREG(pi, HTPHY_bphycrsminpower0, bphycrsminpower0, 0x46);
	MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 59);
	MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 59);
	/* RadioNF [1/4dB] */
	MOD_PHYREG(pi, HTPHY_nvcfg0, noisevar_nf_radio_qdb_core0, 0xe);
	MOD_PHYREG(pi, HTPHY_nvcfg0, noisevar_nf_radio_qdb_core1, 0xe);
	MOD_PHYREG(pi, HTPHY_nvcfg1, noisevar_nf_radio_qdb_core2, 0xe);
	/* Clip1 WB Thresh */
	MOD_PHYREG(pi, HTPHY_Core0clipwbThreshold2059, clip1wbThreshold, 0x2);
	MOD_PHYREG(pi, HTPHY_Core1clipwbThreshold2059, clip1wbThreshold, 0x0);
	MOD_PHYREG(pi, HTPHY_Core2clipwbThreshold2059, clip1wbThreshold, 0x2);
	/* NB Clip  */
	phy_utils_write_phyreg(pi, HTPHY_Core0nbClipThreshold, 0x9b);
	phy_utils_write_phyreg(pi, HTPHY_Core1nbClipThreshold, 0x70);
	phy_utils_write_phyreg(pi, HTPHY_Core2nbClipThreshold, 0x8b);
	/* Hi Gain  */
	phy_utils_write_phyreg(pi, HTPHY_Core0clipHiGainCodeA2059, 0x2e);
	phy_utils_write_phyreg(pi, HTPHY_Core1clipHiGainCodeA2059, 0x2e);
	phy_utils_write_phyreg(pi, HTPHY_Core2clipHiGainCodeA2059, 0x2e);
	phy_utils_write_phyreg(pi, HTPHY_Core0clipHiGainCodeB2059, 0x004);
	phy_utils_write_phyreg(pi, HTPHY_Core1clipHiGainCodeB2059, 0x004);
	phy_utils_write_phyreg(pi, HTPHY_Core2clipHiGainCodeB2059, 0x004);
	/* MD Gain  */
	phy_utils_write_phyreg(pi, HTPHY_Core0clipmdGainCodeA2059, 0x20);
	phy_utils_write_phyreg(pi, HTPHY_Core1clipmdGainCodeA2059, 0x22);
	phy_utils_write_phyreg(pi, HTPHY_Core2clipmdGainCodeA2059, 0x20);
	phy_utils_write_phyreg(pi, HTPHY_Core0clipmdGainCodeB2059, 0x304);
	phy_utils_write_phyreg(pi, HTPHY_Core1clipmdGainCodeB2059, 0x304);
	phy_utils_write_phyreg(pi, HTPHY_Core2clipmdGainCodeB2059, 0x304);
	/* LO Gain  */
	phy_utils_write_phyreg(pi, HTPHY_Core0cliploGainCodeA2059, 0x24);
	phy_utils_write_phyreg(pi, HTPHY_Core1cliploGainCodeA2059, 0x2e);
	phy_utils_write_phyreg(pi, HTPHY_Core2cliploGainCodeA2059, 0x24);
	phy_utils_write_phyreg(pi, HTPHY_Core0cliploGainCodeB2059, 0x208);
	phy_utils_write_phyreg(pi, HTPHY_Core1cliploGainCodeB2059, 0x008);
	phy_utils_write_phyreg(pi, HTPHY_Core2cliploGainCodeB2059, 0x208);
	/* InitGCode */
	phy_utils_write_phyreg(pi, HTPHY_Core0InitGainCodeA2059, 0x2e);
	phy_utils_write_phyreg(pi, HTPHY_Core1InitGainCodeA2059, 0x2e);
	phy_utils_write_phyreg(pi, HTPHY_Core2InitGainCodeA2059, 0x2e);
	phy_utils_write_phyreg(pi, HTPHY_Core0InitGainCodeB2059, 0x524);
	phy_utils_write_phyreg(pi, HTPHY_Core1InitGainCodeB2059, 0x624);
	phy_utils_write_phyreg(pi, HTPHY_Core2InitGainCodeB2059, 0x524);
	/* InitGain */
	{
		const uint16 temp[] = {0x5217, 0x6217, 0x5217};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_RFSEQ,
		                          3, 0x106, 16, temp);
	}
	/* EXTLNA Gains (dB) */
	{
		const int8 temp[] = {16, 16};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
		                          2, 0x0, 8, temp);
	}
	{
		const int8 temp[] = {12, 12};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
		                          2, 0x0, 8, temp);
	}
	{
		const int8 temp[] = {16, 16};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
		                          2, 0x0, 8, temp);
	}
	/* LNA1 Gains [dB] */
	{
		const int8 temp[] = {11, 17, 21, 26};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
		                          4, 0x8, 8, temp);
	}
	{
		const int8 temp[] = {11, 17, 21, 26};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
		                          4, 0x8, 8, temp);
	}
	{
		const int8 temp[] = {11, 17, 21, 26};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
		                          4, 0x8, 8, temp);
	}
	/* LNA2 Gains [dB] */
	{
		const int8 temp[] = {-3, 7, 7, 7};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
		                          4, 0x10, 8, temp);
	}
	{
		const int8 temp[] = {-3, 7, 7, 7};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
		                          4, 0x10, 8, temp);
	}
	{
		const int8 temp[] = {-3, 7, 7, 7};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
		                          4, 0x10, 8, temp);
	}
	/* MixTIA Gains */
	{
		const int8 temp[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
		                          10, 32, 8, temp);
	}
	{
		const int8 temp[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
		                          10, 32, 8, temp);
	}
	{
		const int8 temp[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
		                          10, 32, 8, temp);
	}
	/* Lna2GainBits */
	{
		const int8 temp[] = {0, 1, 1, 1};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS1,
		                          4, 0x10, 8, temp);
	}
	{
		const int8 temp[] = {0, 1, 1, 1};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS2,
		                          4, 0x10, 8, temp);
	}
	{
		const int8 temp[] = {0, 1, 1, 1};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS3,
		                          4, 0x10, 8, temp);
	}
	/* MixGainBits */
	{
		const int8 temp[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS1,
		                          10, 0x20, 8, temp);
	}
	{
		const int8 temp[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS2,
		                          10, 0x20, 8, temp);
	}
	{
		const int8 temp[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS3,
		                          10, 0x20, 8, temp);
	}
	/* Vmid and Av for {RSSI invalid PWRDET} */
	{
		const uint16 temp[] = {0x8e, 0x96, 0x96};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x08, 16, temp);
	}
	{
		const uint16 temp[] = {0x8f, 0x9f, 0x9f};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x18, 16, temp);
	}
	{
		const uint16 temp[] = {0x8f, 0x9f, 0x9f};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x28, 16, temp);
	}
	{
		const uint16 temp[] = {0x2, 0x2, 0x2};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x0c, 16, temp);
	}
	{
		const uint16 temp[] = {0x2, 0x2, 0x2};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x1c, 16, temp);
	}
	{
		const uint16 temp[] = {0x2, 0x2, 0x2};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x2c, 16, temp);
	}

	/* BW20 customizations */
	if (CHSPEC_IS20(pi->radio_chanspec) == 1) {

		if ((fc >= 2412 && fc <= 2484)) {
		}

		/* BW40 customizations */
	} else {

		if ((fc >= 2412 && fc <= 2437)) {
		}
	}
}
static void
wlc_phy_subband_cust_rev1_cs_5g_htphy(phy_info_t *pi, uint16 fc)
{

	/* Default customizations */
	/* CRS MinPwr */
	MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 0x37);
	MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 0x37);
	/* RadioNF [1/4dB] */
	MOD_PHYREG(pi, HTPHY_nvcfg0, noisevar_nf_radio_qdb_core0, 0x1c);
	MOD_PHYREG(pi, HTPHY_nvcfg0, noisevar_nf_radio_qdb_core1, 0x1c);
	MOD_PHYREG(pi, HTPHY_nvcfg1, noisevar_nf_radio_qdb_core2, 0x1c);
	/* Clip1 WB Thresh */
	MOD_PHYREG(pi, HTPHY_Core0clipwbThreshold2059, clip1wbThreshold, 0x09);
	MOD_PHYREG(pi, HTPHY_Core1clipwbThreshold2059, clip1wbThreshold, 0x34);
	MOD_PHYREG(pi, HTPHY_Core2clipwbThreshold2059, clip1wbThreshold, 0x0c);
	/* NB Clip  */
	phy_utils_write_phyreg(pi, HTPHY_Core0nbClipThreshold, 0xa1);
	phy_utils_write_phyreg(pi, HTPHY_Core1nbClipThreshold, 0xa1);
	phy_utils_write_phyreg(pi, HTPHY_Core2nbClipThreshold, 0xa1);
	/* Hi Gain  */
	phy_utils_write_phyreg(pi, HTPHY_Core0clipHiGainCodeA2059, 0x56);
	phy_utils_write_phyreg(pi, HTPHY_Core1clipHiGainCodeA2059, 0x56);
	phy_utils_write_phyreg(pi, HTPHY_Core2clipHiGainCodeA2059, 0x56);
	phy_utils_write_phyreg(pi, HTPHY_Core0clipHiGainCodeB2059, 0x004);
	phy_utils_write_phyreg(pi, HTPHY_Core1clipHiGainCodeB2059, 0x004);
	phy_utils_write_phyreg(pi, HTPHY_Core2clipHiGainCodeB2059, 0x004);
	/* MD Gain  */
	phy_utils_write_phyreg(pi, HTPHY_Core0clipmdGainCodeA2059, 0x42);
	phy_utils_write_phyreg(pi, HTPHY_Core1clipmdGainCodeA2059, 0x42);
	phy_utils_write_phyreg(pi, HTPHY_Core2clipmdGainCodeA2059, 0x42);
	phy_utils_write_phyreg(pi, HTPHY_Core0clipmdGainCodeB2059, 0x204);
	phy_utils_write_phyreg(pi, HTPHY_Core1clipmdGainCodeB2059, 0x204);
	phy_utils_write_phyreg(pi, HTPHY_Core2clipmdGainCodeB2059, 0x204);
	/* LO Gain  */
	phy_utils_write_phyreg(pi, HTPHY_Core0cliploGainCodeA2059, 0x36);
	phy_utils_write_phyreg(pi, HTPHY_Core1cliploGainCodeA2059, 0x56);
	phy_utils_write_phyreg(pi, HTPHY_Core2cliploGainCodeA2059, 0x36);
	phy_utils_write_phyreg(pi, HTPHY_Core0cliploGainCodeB2059, 0x008);
	phy_utils_write_phyreg(pi, HTPHY_Core1cliploGainCodeB2059, 0x18);
	phy_utils_write_phyreg(pi, HTPHY_Core2cliploGainCodeB2059, 0x008);
	/* InitGCode */
	phy_utils_write_phyreg(pi, HTPHY_Core0InitGainCodeA2059, 0x56);
	phy_utils_write_phyreg(pi, HTPHY_Core1InitGainCodeA2059, 0x56);
	phy_utils_write_phyreg(pi, HTPHY_Core2InitGainCodeA2059, 0x56);
	phy_utils_write_phyreg(pi, HTPHY_Core0InitGainCodeB2059, 0x524);
	phy_utils_write_phyreg(pi, HTPHY_Core1InitGainCodeB2059, 0x524);
	phy_utils_write_phyreg(pi, HTPHY_Core2InitGainCodeB2059, 0x524);
	/* InitGain */
	{
		const uint16 temp[] = {0x522b, 0x522b, 0x522b};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_RFSEQ,
		                          3, 0x106, 16, temp);
	}
	/* EXTLNA Gains (dB) */
	{
		const int8 temp[] = {15, 15};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
		                          2, 0x0, 8, temp);
	}
	{
		const int8 temp[] = {13, 13};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
		                          2, 0x0, 8, temp);
	}
	{
		const int8 temp[] = {15, 15};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
		                          2, 0x0, 8, temp);
	}
	/* LNA1 Gains [dB] */
	{
		const int8 temp[] = {12, 18, 22, 26};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
		                          4, 0x8, 8, temp);
	}
	{
		const int8 temp[] = {12, 18, 22, 26};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
		                          4, 0x8, 8, temp);
	}
	{
		const int8 temp[] = {12, 18, 22, 26};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
		                          4, 0x8, 8, temp);
	}
	/* LNA2 Gains [dB] */
	{
		const int8 temp[] = {-1, 7, 11, 11};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
		                          4, 0x10, 8, temp);
	}
	{
		const int8 temp[] = {-1, 7, 11, 11};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
		                          4, 0x10, 8, temp);
	}
	{
		const int8 temp[] = {-1, 7, 11, 11};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
		                          4, 0x10, 8, temp);
	}
	/* MixTIA Gains */
	{
		const int8 temp[] = {-6, -6, -3, -3, -3, -3, -3, -3, -3, -3, -3};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
		                          11, 32, 8, temp);
	}
	{
		const int8 temp[] = {-6, -6, -3, -3, -3, -3, -3, -3, -3, -3, -3};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
		                          11, 32, 8, temp);
	}
	{
		const int8 temp[] = {-6, -6, -3, -3, -3, -3, -3, -3, -3, -3, -3};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
		                          11, 32, 8, temp);
	}
	/* Lna2GainBits */
	{
		const int8 temp[] = {0, 1, 2, 2};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS1,
		                          4, 0x10, 8, temp);
	}
	{
		const int8 temp[] = {0, 1, 2, 2};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS2,
		                          4, 0x10, 8, temp);
	}
	{
		const int8 temp[] = {0, 1, 2, 2};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS3,
		                          4, 0x10, 8, temp);
	}
	/* MixGainBits */
	{
		const int8 temp[] = {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS1,
		                          11, 0x20, 8, temp);
	}
	{
		const int8 temp[] = {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS2,
		                          11, 0x20, 8, temp);
	}
	{
		const int8 temp[] = {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS3,
		                          11, 0x20, 8, temp);
	}
	/* Vmid and Av for {RSSI invalid PWRDET} */
	{
		const uint16 temp[] = {0x8e, 0x96, 0x96};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x08, 16, temp);
	}
	{
		const uint16 temp[] = {0x8f, 0x9f, 0x9f};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x18, 16, temp);
	}
	{
		const uint16 temp[] = {0x8f, 0x9f, 0x9f};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x28, 16, temp);
	}
	{
		const uint16 temp[] = {0x2, 0x2, 0x2};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x0c, 16, temp);
	}
	{
		const uint16 temp[] = {0x2, 0x2, 0x2};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x1c, 16, temp);
	}
	{
		const uint16 temp[] = {0x2, 0x2, 0x2};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x2c, 16, temp);
	}

	/* BW20 customizations */
	if (CHSPEC_IS20(pi->radio_chanspec) == 1) {

		if ((fc >= 4920 && fc <= 5320)) {
			/* CRS MinPwr */
			MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 0x3c);
			MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 0x3c);
			/* Clip1 WB Thresh */
			MOD_PHYREG(pi, HTPHY_Core1clipwbThreshold2059, clip1wbThreshold, 0x0);
			/* LO Gain  */
			phy_utils_write_phyreg(pi, HTPHY_Core1cliploGainCodeA2059, 0x36);
			phy_utils_write_phyreg(pi, HTPHY_Core1cliploGainCodeB2059, 0x008);
			/* EXTLNA Gains (dB) */
			{
				const int8 temp[] = {13, 13};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          2, 0x0, 8, temp);
			}
			{
				const int8 temp[] = {13, 13};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          2, 0x0, 8, temp);
			}
			{
				const int8 temp[] = {13, 13};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          2, 0x0, 8, temp);
			}
			/* LNA1 Gains [dB] */
			{
				const int8 temp[] = {14, 20, 24, 28};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {14, 20, 24, 28};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {14, 20, 24, 28};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x8, 8, temp);
			}

		} else if ((fc >= 5500 && fc <= 5640)) {
			/* CRS MinPwr */
			MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 0x39);
			MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 0x39);
			/* Clip1 WB Thresh */
			MOD_PHYREG(pi, HTPHY_Core1clipwbThreshold2059, clip1wbThreshold, 0x38);
			/* LO Gain  */
			phy_utils_write_phyreg(pi, HTPHY_Core1cliploGainCodeA2059, 0x56);
			phy_utils_write_phyreg(pi, HTPHY_Core1cliploGainCodeB2059, 0x008);
			/* EXTLNA Gains (dB) */
			{
				const int8 temp[] = {15, 15};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          2, 0x0, 8, temp);
			}
			{
				const int8 temp[] = {13, 13};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          2, 0x0, 8, temp);
			}
			{
				const int8 temp[] = {15, 15};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          2, 0x0, 8, temp);
			}
			/* LNA1 Gains [dB] */
			{
				const int8 temp[] = {12, 18, 22, 26};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {12, 18, 22, 26};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {12, 18, 22, 26};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x8, 8, temp);
			}

		} else if ((fc >= 5650 && fc <= 5825)) {
			/* CRS MinPwr */
			MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 0x39);
			MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 0x39);
			/* Clip1 WB Thresh */
			MOD_PHYREG(pi, HTPHY_Core1clipwbThreshold2059, clip1wbThreshold, 0x34);
			/* LO Gain  */
			phy_utils_write_phyreg(pi, HTPHY_Core1cliploGainCodeA2059, 0x56);
			phy_utils_write_phyreg(pi, HTPHY_Core1cliploGainCodeB2059, 0x18);
			/* EXTLNA Gains (dB) */
			{
				const int8 temp[] = {15, 15};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          2, 0x0, 8, temp);
			}
			{
				const int8 temp[] = {13, 13};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          2, 0x0, 8, temp);
			}
			{
				const int8 temp[] = {15, 15};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          2, 0x0, 8, temp);
			}
			/* LNA1 Gains [dB] */
			{
				const int8 temp[] = {12, 18, 22, 26};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {12, 18, 22, 26};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {12, 18, 22, 26};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x8, 8, temp);
			}

		}

		/* BW40 customizations */
	} else {

		if ((fc >= 4920 && fc <= 5320)) {
			/* CRS MinPwr */
			MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 0x38);
			MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 0x38);
			/* Clip1 WB Thresh */
			MOD_PHYREG(pi, HTPHY_Core1clipwbThreshold2059, clip1wbThreshold, 0x0);
			/* LO Gain  */
			phy_utils_write_phyreg(pi, HTPHY_Core1cliploGainCodeA2059, 0x36);
			phy_utils_write_phyreg(pi, HTPHY_Core1cliploGainCodeB2059, 0x008);
			/* EXTLNA Gains (dB) */
			{
				const int8 temp[] = {13, 13};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          2, 0x0, 8, temp);
			}
			{
				const int8 temp[] = {13, 13};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          2, 0x0, 8, temp);
			}
			{
				const int8 temp[] = {13, 13};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          2, 0x0, 8, temp);
			}
			/* LNA1 Gains [dB] */
			{
				const int8 temp[] = {14, 20, 24, 28};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {14, 20, 24, 28};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {14, 20, 24, 28};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x8, 8, temp);
			}

		} else if ((fc >= 5500 && fc <= 5640)) {
			/* CRS MinPwr */
			MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 0x35);
			MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 0x35);
			/* Clip1 WB Thresh */
			MOD_PHYREG(pi, HTPHY_Core1clipwbThreshold2059, clip1wbThreshold, 0x3c);
			/* LO Gain  */
			phy_utils_write_phyreg(pi, HTPHY_Core1cliploGainCodeA2059, 0x56);
			phy_utils_write_phyreg(pi, HTPHY_Core1cliploGainCodeB2059, 0x008);
			/* EXTLNA Gains (dB) */
			{
				const int8 temp[] = {15, 15};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          2, 0x0, 8, temp);
			}
			{
				const int8 temp[] = {13, 13};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          2, 0x0, 8, temp);
			}
			{
				const int8 temp[] = {15, 15};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          2, 0x0, 8, temp);
			}
			/* LNA1 Gains [dB] */
			{
				const int8 temp[] = {12, 18, 22, 26};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {12, 18, 22, 26};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {12, 18, 22, 26};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x8, 8, temp);
			}

		} else if ((fc >= 5650 && fc <= 5825)) {
			/* CRS MinPwr */
			MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 0x34);
			MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 0x34);
			/* Clip1 WB Thresh */
			MOD_PHYREG(pi, HTPHY_Core1clipwbThreshold2059, clip1wbThreshold, 0x34);
			/* LO Gain  */
			phy_utils_write_phyreg(pi, HTPHY_Core1cliploGainCodeA2059, 0x56);
			phy_utils_write_phyreg(pi, HTPHY_Core1cliploGainCodeB2059, 0x18);
			/* EXTLNA Gains (dB) */
			{
				const int8 temp[] = {15, 15};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          2, 0x0, 8, temp);
			}
			{
				const int8 temp[] = {13, 13};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          2, 0x0, 8, temp);
			}
			{
				const int8 temp[] = {15, 15};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          2, 0x0, 8, temp);
			}
			/* LNA1 Gains [dB] */
			{
				const int8 temp[] = {12, 18, 22, 26};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {12, 18, 22, 26};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {12, 18, 22, 26};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x8, 8, temp);
			}

		}
	}
}

/*
 * AUTOGENERATED from htphy/subband_cust_4331B1_pciedual_old.xls
 * DO NOT EDIT
 */

static void
wlc_phy_subband_cust_rev1_pciedual_old_htphy(phy_info_t *pi)
{
	void (*subbandcust_bandx_pointer)(phy_info_t *pi, uint16 fc) = 0;
	uint16 fc;
	if (CHSPEC_CHANNEL(pi->radio_chanspec) > 14) {
		fc = CHAN5G_FREQ(CHSPEC_CHANNEL(pi->radio_chanspec));
	} else {
		fc = CHAN2G_FREQ(CHSPEC_CHANNEL(pi->radio_chanspec));
	}

	/* 2G Band Customizations */
	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		subbandcust_bandx_pointer = wlc_phy_subband_cust_rev1_pciedual_old_2g_htphy;
	}

	/* 5G Band Customizations */
	if (CHSPEC_IS5G(pi->radio_chanspec)) {
		subbandcust_bandx_pointer = wlc_phy_subband_cust_rev1_pciedual_old_5g_htphy;
	}
	ASSERT(subbandcust_bandx_pointer);
	if (subbandcust_bandx_pointer)
		subbandcust_bandx_pointer(pi, fc);
}
static void
wlc_phy_subband_cust_rev1_pciedual_old_2g_htphy(phy_info_t *pi, uint16 fc)
{

	/* Default customizations */
	/* CRS MinPwr */
	MOD_PHYREG(pi, HTPHY_bphycrsminpower0, bphycrsminpower0, 0x46);
	/* Hi Gain  */
	phy_utils_write_phyreg(pi, HTPHY_Core0clipHiGainCodeA2059, 0x6c);
	phy_utils_write_phyreg(pi, HTPHY_Core1clipHiGainCodeA2059, 0x6c);
	phy_utils_write_phyreg(pi, HTPHY_Core2clipHiGainCodeA2059, 0x6c);
	/* MD Gain  */
	phy_utils_write_phyreg(pi, HTPHY_Core0clipmdGainCodeA2059, 0x60);
	phy_utils_write_phyreg(pi, HTPHY_Core1clipmdGainCodeA2059, 0x60);
	phy_utils_write_phyreg(pi, HTPHY_Core2clipmdGainCodeA2059, 0x60);
	phy_utils_write_phyreg(pi, HTPHY_Core0clipmdGainCodeB2059, 0x24);
	phy_utils_write_phyreg(pi, HTPHY_Core1clipmdGainCodeB2059, 0x24);
	phy_utils_write_phyreg(pi, HTPHY_Core2clipmdGainCodeB2059, 0x24);
	/* LO Gain  */
	phy_utils_write_phyreg(pi, HTPHY_Core0cliploGainCodeA2059, 0x62);
	phy_utils_write_phyreg(pi, HTPHY_Core1cliploGainCodeA2059, 0x62);
	phy_utils_write_phyreg(pi, HTPHY_Core2cliploGainCodeA2059, 0x62);
	phy_utils_write_phyreg(pi, HTPHY_Core0cliploGainCodeB2059, 0x318);
	phy_utils_write_phyreg(pi, HTPHY_Core1cliploGainCodeB2059, 0x318);
	phy_utils_write_phyreg(pi, HTPHY_Core2cliploGainCodeB2059, 0x318);
	/* InitGCode */
	phy_utils_write_phyreg(pi, HTPHY_Core0InitGainCodeA2059, 0x6e);
	phy_utils_write_phyreg(pi, HTPHY_Core1InitGainCodeA2059, 0x6e);
	phy_utils_write_phyreg(pi, HTPHY_Core2InitGainCodeA2059, 0x6e);
	/* InitGCode */
	phy_utils_write_phyreg(pi, HTPHY_Core0InitGainCodeB2059, 0x0514);
	phy_utils_write_phyreg(pi, HTPHY_Core1InitGainCodeB2059, 0x0514);
	phy_utils_write_phyreg(pi, HTPHY_Core2InitGainCodeB2059, 0x0514);
	/* InitGain */
	{
		const uint16 temp[] = {0x5137, 0x5137, 0x5137};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_RFSEQ,
		                          3, 0x106, 16, temp);
	}
	/* EXTLNA Gains (dB) */
	{
		const int8 temp[] = {14, 14};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
		                          2, 0x0, 8, temp);
	}
	{
		const int8 temp[] = {14, 14};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
		                          2, 0x0, 8, temp);
	}
	{
		const int8 temp[] = {14, 14};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
		                          2, 0x0, 8, temp);
	}
	/* Vmid and Av for {RSSI invalid PWRDET} */
	{
		const uint16 temp[] = {0x8e, 0x96, 0x96};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x08, 16, temp);
	}
	{
		const uint16 temp[] = {0x8f, 0x9f, 0x9f};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x18, 16, temp);
	}
	{
		const uint16 temp[] = {0x8f, 0x9f, 0x9f};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x28, 16, temp);
	}
	{
		const uint16 temp[] = {0x02, 0x02, 0x02};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x0c, 16, temp);
	}
	{
		const uint16 temp[] = {0x02, 0x02, 0x02};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x1c, 16, temp);
	}
	{
		const uint16 temp[] = {0x02, 0x02, 0x02};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x2c, 16, temp);
	}

	/* BW20 customizations */
	if (CHSPEC_IS20(pi->radio_chanspec) == 1) {

		if ((fc >= 2412 && fc <= 2484)) {
			/* CRS MinPwr */
			MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 57);
			MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 57);
			/* Clip1 WB Thresh */
			MOD_PHYREG(pi, HTPHY_Core0clipwbThreshold2059, clip1wbThreshold, 13);
			MOD_PHYREG(pi, HTPHY_Core1clipwbThreshold2059, clip1wbThreshold, 13);
			MOD_PHYREG(pi, HTPHY_Core2clipwbThreshold2059, clip1wbThreshold, 13);
			/* LNA1 Gains [dB] */
			{
				const int8 temp[] = {9, 15, 19, 24};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {9, 15, 19, 24};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {9, 15, 19, 24};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x8, 8, temp);
			}
			/* LNA2 Gains [dB] */
			{
				const int8 temp[] = {-3, 7, 11, 15};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {-3, 7, 11, 15};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {-3, 7, 11, 15};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x10, 8, temp);
			}

		}

		/* BW40 customizations */
	} else {

		if ((fc >= 2412 && fc <= 2437)) {
			/* CRS MinPwr */
			MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 57);
			MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 57);
			/* LNA1 Gains [dB] */
			{
				const int8 temp[] = {7, 13, 17, 22};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {7, 13, 17, 22};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {7, 13, 17, 22};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x8, 8, temp);
			}
			/* LNA2 Gains [dB] */
			{
				const int8 temp[] = {-3, 7, 11, 15};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {-3, 7, 11, 15};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {-3, 7, 11, 15};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x10, 8, temp);
			}

		} else if ((fc >= 2442 && fc <= 2484)) {
			/* CRS MinPwr */
			MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 57);
			MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 57);
			/* LNA1 Gains [dB] */
			{
				const int8 temp[] = {8, 13, 17, 22};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {8, 13, 17, 22};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {8, 13, 17, 22};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x8, 8, temp);
			}
			/* LNA2 Gains [dB] */
			{
				const int8 temp[] = {-3, 7, 11, 15};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {-3, 7, 11, 15};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {-3, 7, 11, 15};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x10, 8, temp);
			}

		}
	}
}
static void
wlc_phy_subband_cust_rev1_pciedual_old_5g_htphy(phy_info_t *pi, uint16 fc)
{

	/* Default customizations */
	/* NB Clip  */
	phy_utils_write_phyreg(pi, HTPHY_Core0nbClipThreshold, 0xff);
	phy_utils_write_phyreg(pi, HTPHY_Core1nbClipThreshold, 0xff);
	phy_utils_write_phyreg(pi, HTPHY_Core2nbClipThreshold, 0xff);
	/* Hi Gain  */
	phy_utils_write_phyreg(pi, HTPHY_Core0clipHiGainCodeA2059, 0x9e);
	phy_utils_write_phyreg(pi, HTPHY_Core1clipHiGainCodeA2059, 0x9e);
	phy_utils_write_phyreg(pi, HTPHY_Core2clipHiGainCodeA2059, 0x9e);
	/* MD Gain  */
	phy_utils_write_phyreg(pi, HTPHY_Core0clipmdGainCodeA2059, 0x80);
	phy_utils_write_phyreg(pi, HTPHY_Core1clipmdGainCodeA2059, 0x80);
	phy_utils_write_phyreg(pi, HTPHY_Core2clipmdGainCodeA2059, 0x80);
	phy_utils_write_phyreg(pi, HTPHY_Core0clipmdGainCodeB2059, 0x14);
	phy_utils_write_phyreg(pi, HTPHY_Core1clipmdGainCodeB2059, 0x14);
	phy_utils_write_phyreg(pi, HTPHY_Core2clipmdGainCodeB2059, 0x14);
	/* LO Gain  */
	phy_utils_write_phyreg(pi, HTPHY_Core0cliploGainCodeA2059, 0x0082);
	phy_utils_write_phyreg(pi, HTPHY_Core1cliploGainCodeA2059, 0x0082);
	phy_utils_write_phyreg(pi, HTPHY_Core2cliploGainCodeA2059, 0x0082);
	phy_utils_write_phyreg(pi, HTPHY_Core0cliploGainCodeB2059, 0x308);
	phy_utils_write_phyreg(pi, HTPHY_Core1cliploGainCodeB2059, 0x308);
	phy_utils_write_phyreg(pi, HTPHY_Core2cliploGainCodeB2059, 0x308);
	/* InitGCode */
	phy_utils_write_phyreg(pi, HTPHY_Core0InitGainCodeA2059, 0x8e);
	phy_utils_write_phyreg(pi, HTPHY_Core1InitGainCodeA2059, 0x8e);
	phy_utils_write_phyreg(pi, HTPHY_Core2InitGainCodeA2059, 0x8e);
	/* InitGain */
	{
		const uint16 temp[] = {0x6247, 0x6247, 0x6247};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_RFSEQ,
		                          3, 0x106, 16, temp);
	}
	/* EXTLNA Gains (dB) */
	{
		const int8 temp[] = {8, 8};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
		                          2, 0x0, 8, temp);
	}
	{
		const int8 temp[] = {8, 8};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
		                          2, 0x0, 8, temp);
	}
	{
		const int8 temp[] = {8, 8};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
		                          2, 0x0, 8, temp);
	}
	/* MixTIA Gains */
	{
		const int8 temp[] = {3, 3, 3, 3, 3, 3, 3, 3, 3, 3};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
		                          10, 32, 8, temp);
	}
	{
		const int8 temp[] = {3, 3, 3, 3, 3, 3, 3, 3, 3, 3};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
		                          10, 32, 8, temp);
	}
	{
		const int8 temp[] = {3, 3, 3, 3, 3, 3, 3, 3, 3, 3};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
		                          10, 32, 8, temp);
	}
	/* GainBits */
	{
		const int8 temp[] = {4, 4, 4, 4, 4, 4, 4, 4, 4, 4};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS1,
		                          10, 0x20, 8, temp);
	}
	{
		const int8 temp[] = {4, 4, 4, 4, 4, 4, 4, 4, 4, 4};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS2,
		                          10, 0x20, 8, temp);
	}
	{
		const int8 temp[] = {4, 4, 4, 4, 4, 4, 4, 4, 4, 4};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS3,
		                          10, 0x20, 8, temp);
	}
	/* Vmid and Av for {RSSI invalid PWRDET} */
	{
		const uint16 temp[] = {0x8e, 0x96, 0x96};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x08, 16, temp);
	}
	{
		const uint16 temp[] = {0x8f, 0x9f, 0x9f};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x18, 16, temp);
	}
	{
		const uint16 temp[] = {0x8f, 0x9f, 0x9f};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x28, 16, temp);
	}
	{
		const uint16 temp[] = {0x02, 0x02, 0x02};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x0c, 16, temp);
	}
	{
		const uint16 temp[] = {0x02, 0x02, 0x02};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x1c, 16, temp);
	}
	{
		const uint16 temp[] = {0x02, 0x02, 0x02};
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL,
		                          3, 0x2c, 16, temp);
	}

	/* BW20 customizations */
	if (CHSPEC_IS20(pi->radio_chanspec) == 1) {

		if ((fc >= 4920 && fc <= 5230)) {
			/* CRS MinPwr */
			MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 64);
			MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 64);
			/* RadioNF [1/4dB] */
			MOD_PHYREG(pi, HTPHY_nvcfg0, noisevar_nf_radio_qdb_core0, 28);
			MOD_PHYREG(pi, HTPHY_nvcfg0, noisevar_nf_radio_qdb_core1, 28);
			MOD_PHYREG(pi, HTPHY_nvcfg1, noisevar_nf_radio_qdb_core2, 28);
			/* Clip1 WB Thresh */
			MOD_PHYREG(pi, HTPHY_Core0clipwbThreshold2059, clip1wbThreshold, 0x19);
			MOD_PHYREG(pi, HTPHY_Core1clipwbThreshold2059, clip1wbThreshold, 0x19);
			MOD_PHYREG(pi, HTPHY_Core2clipwbThreshold2059, clip1wbThreshold, 0x19);
			/* LNA1 Gains [dB] */
			{
				const int8 temp[] = {11, 17, 21, 25};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {11, 17, 21, 25};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {11, 17, 21, 25};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x8, 8, temp);
			}
			/* LNA2 Gains [dB] */
			{
				const int8 temp[] = {0, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {0, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {0, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x10, 8, temp);
			}

		} else if ((fc >= 5240 && fc <= 5550)) {
			/* CRS MinPwr */
			MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 64);
			MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 64);
			/* RadioNF [1/4dB] */
			MOD_PHYREG(pi, HTPHY_nvcfg0, noisevar_nf_radio_qdb_core0, 28);
			MOD_PHYREG(pi, HTPHY_nvcfg0, noisevar_nf_radio_qdb_core1, 28);
			MOD_PHYREG(pi, HTPHY_nvcfg1, noisevar_nf_radio_qdb_core2, 28);
			/* Clip1 WB Thresh */
			MOD_PHYREG(pi, HTPHY_Core0clipwbThreshold2059, clip1wbThreshold, 0x19);
			MOD_PHYREG(pi, HTPHY_Core1clipwbThreshold2059, clip1wbThreshold, 0x19);
			MOD_PHYREG(pi, HTPHY_Core2clipwbThreshold2059, clip1wbThreshold, 0x19);
			/* LNA1 Gains [dB] */
			{
				const int8 temp[] = {10, 16, 20, 24};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {10, 16, 20, 24};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {10, 16, 20, 24};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x8, 8, temp);
			}
			/* LNA2 Gains [dB] */
			{
				const int8 temp[] = {0, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {0, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {0, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x10, 8, temp);
			}

		} else if ((fc >= 5560 && fc <= 5825)) {
			/* CRS MinPwr */
			MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 64);
			MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 64);
			/* RadioNF [1/4dB] */
			MOD_PHYREG(pi, HTPHY_nvcfg0, noisevar_nf_radio_qdb_core0, 28);
			MOD_PHYREG(pi, HTPHY_nvcfg0, noisevar_nf_radio_qdb_core1, 28);
			MOD_PHYREG(pi, HTPHY_nvcfg1, noisevar_nf_radio_qdb_core2, 28);
			/* Clip1 WB Thresh */
			MOD_PHYREG(pi, HTPHY_Core0clipwbThreshold2059, clip1wbThreshold, 0x19);
			MOD_PHYREG(pi, HTPHY_Core1clipwbThreshold2059, clip1wbThreshold, 0x19);
			MOD_PHYREG(pi, HTPHY_Core2clipwbThreshold2059, clip1wbThreshold, 0x19);
			/* LNA1 Gains [dB] */
			{
				const int8 temp[] = {10, 15, 19, 23};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {10, 15, 19, 23};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {10, 15, 19, 23};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x8, 8, temp);
			}
			/* LNA2 Gains [dB] */
			{
				const int8 temp[] = {2, 9, 13, 16};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {2, 9, 13, 16};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {2, 9, 13, 16};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x10, 8, temp);
			}

		}

		/* BW40 customizations */
	} else {

		if ((fc >= 4920 && fc <= 5230)) {
			/* CRS MinPwr */
			MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 61);
			MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 61);
			/* Clip1 WB Thresh */
			MOD_PHYREG(pi, HTPHY_Core0clipwbThreshold2059, clip1wbThreshold, 0x14);
			MOD_PHYREG(pi, HTPHY_Core1clipwbThreshold2059, clip1wbThreshold, 0x14);
			MOD_PHYREG(pi, HTPHY_Core2clipwbThreshold2059, clip1wbThreshold, 0x14);
			/* LNA1 Gains [dB] */
			{
				const int8 temp[] = {10, 16, 20, 24};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {10, 16, 20, 24};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {10, 16, 20, 24};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x8, 8, temp);
			}
			/* LNA2 Gains [dB] */
			{
				const int8 temp[] = {0, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {0, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {0, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x10, 8, temp);
			}

		} else if ((fc >= 5240 && fc <= 5550)) {
			/* CRS MinPwr */
			MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 61);
			MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 61);
			/* Clip1 WB Thresh */
			MOD_PHYREG(pi, HTPHY_Core0clipwbThreshold2059, clip1wbThreshold, 0x14);
			MOD_PHYREG(pi, HTPHY_Core1clipwbThreshold2059, clip1wbThreshold, 0x14);
			MOD_PHYREG(pi, HTPHY_Core2clipwbThreshold2059, clip1wbThreshold, 0x14);
			/* LNA1 Gains [dB] */
			{
				const int8 temp[] = {9, 15, 20, 23};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {9, 15, 20, 23};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {9, 15, 20, 23};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x8, 8, temp);
			}
			/* LNA2 Gains [dB] */
			{
				const int8 temp[] = {0, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {0, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {0, 7, 11, 14};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x10, 8, temp);
			}

		} else if ((fc >= 5560 && fc <= 5825)) {
			/* CRS MinPwr */
			MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, 62);
			MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, 62);
			/* Clip1 WB Thresh */
			MOD_PHYREG(pi, HTPHY_Core0clipwbThreshold2059, clip1wbThreshold, 0x14);
			MOD_PHYREG(pi, HTPHY_Core1clipwbThreshold2059, clip1wbThreshold, 0x14);
			MOD_PHYREG(pi, HTPHY_Core2clipwbThreshold2059, clip1wbThreshold, 0x14);
			/* LNA1 Gains [dB] */
			{
				const int8 temp[] = {9, 14, 17, 21};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {9, 14, 17, 21};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x8, 8, temp);
			}
			{
				const int8 temp[] = {9, 14, 17, 21};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x8, 8, temp);
			}
			/* LNA2 Gains [dB] */
			{
				const int8 temp[] = {2, 9, 13, 16};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {2, 9, 13, 16};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN2,
				                          4, 0x10, 8, temp);
			}
			{
				const int8 temp[] = {2, 9, 13, 16};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3,
				                          4, 0x10, 8, temp);
			}

		}
	}
}

void
wlc_phy_switch_radio_htphy(phy_info_t *pi, bool on)
{
	PHY_TRACE(("wl%d: %s %s\n", pi->sh->unit, __FUNCTION__, on ? "ON" : "OFF"));

	if (on) {
		if (! pi->radio_is_on) {
			wlc_phy_radio2059_init(pi);

			/* !!! it could change bw inside */
			wlc_phy_chanspec_set((wlc_phy_t*)pi, pi->radio_chanspec);
		}
		pi->radio_is_on = TRUE;
	} else {
		phy_utils_and_phyreg(pi, HTPHY_RfctrlCmd, ~HTPHY_RfctrlCmd_chip_pu_MASK);
		pi->radio_is_on = FALSE;
	}
}

static void
wlc_phy_ht_radio2059_war(phy_info_t *pi)
{
	uint16 reg;

	PHY_INFORM(("wl%d: %s: Apply Radio 2059 WAR\n", pi->sh->unit, __FUNCTION__));

	/* PR 98699 / Radar 9424581
	 * Use four-wire interface to touch a register on each radio to initialize
	 * the jtag tap of each core.  Then reset the radio again to put all radio
	 * registers back to power-on-reset values.
	 */

	/* Reset jtag (through por_force toggle) to "reset" radio */
	/* NOTE: por_force is active low and level sensitive! */
	phy_utils_and_phyreg(pi, HTPHY_RfctrlCmd, ~HTPHY_RfctrlCmd_chip_pu_MASK);
	phy_utils_or_phyreg(pi, HTPHY_RfctrlCmd, HTPHY_RfctrlCmd_por_force_MASK);

	phy_utils_and_phyreg(pi, HTPHY_RfctrlCmd, ~HTPHY_RfctrlCmd_por_force_MASK);
	phy_utils_or_phyreg(pi, HTPHY_RfctrlCmd, HTPHY_RfctrlCmd_chip_pu_MASK);

	/* Set radio reg 0x39 on Core 0 to it's POR value */
	W_REG(pi->sh->osh, &pi->regs->phy4waddr, 0x039);
	reg = R_REG(pi->sh->osh, &pi->regs->phy4waddr);
	W_REG(pi->sh->osh, &pi->regs->phy4wdatalo, 0x08);

	/* Set radio reg 0x39 on Core 1 to it's POR value */
	W_REG(pi->sh->osh, &pi->regs->phy4waddr, 0x1039);
	reg = R_REG(pi->sh->osh, &pi->regs->phy4waddr);
	W_REG(pi->sh->osh, &pi->regs->phy4wdatalo, 0x08);

	/* Set radio reg 0x39 on Core 2 to it's POR value */
	W_REG(pi->sh->osh, &pi->regs->phy4waddr, 0x2039);
	reg = R_REG(pi->sh->osh, &pi->regs->phy4waddr);
	W_REG(pi->sh->osh, &pi->regs->phy4wdatalo, 0x08);
	BCM_REFERENCE(reg);
}

static void
wlc_phy_radio2059_reset(phy_info_t *pi)
{
	if (pi->sh->clk && !pi->sh->up)
		wlc_phy_ht_radio2059_war(pi);

	/* Reset jtag (through por_force toggle) to "reset" radio */
	/* NOTE: por_force is active low and level sensitive! */
	phy_utils_and_phyreg(pi, HTPHY_RfctrlCmd, ~HTPHY_RfctrlCmd_chip_pu_MASK);
	phy_utils_or_phyreg(pi, HTPHY_RfctrlCmd, HTPHY_RfctrlCmd_por_force_MASK);
	phy_utils_and_phyreg(pi, HTPHY_RfctrlCmd, ~HTPHY_RfctrlCmd_por_force_MASK);
	phy_utils_or_phyreg(pi, HTPHY_RfctrlCmd, HTPHY_RfctrlCmd_chip_pu_MASK);
}

static void
wlc_phy_radio2059_vcocal(phy_info_t *pi)
{

	int freq;
	chan_info_htphy_radio2059_t *ci = NULL;
	uint channel;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	phy_utils_mod_radioreg(pi, RADIO_2059_RFPLL_MISC_EN,         0x01, 0x0);
	phy_utils_mod_radioreg(pi, RADIO_2059_RFPLL_MISC_CAL_RESETN, 0x04, 0x0);
	phy_utils_mod_radioreg(pi, RADIO_2059_RFPLL_MISC_CAL_RESETN, 0x04, (1 << 2));
	phy_utils_mod_radioreg(pi, RADIO_2059_RFPLL_MISC_EN,         0x01, 0x01);

	/* XXX:
	 * PR90571: Reduce VCO-Cal Static Wait Time
	 * Wait for open loop cal completion and settling
	 */
	channel = CHSPEC_CHANNEL(pi->radio_chanspec);
	if (!wlc_phy_chan2freq_htphy(pi, channel, &freq, &ci)) {
		PHY_ERROR(("wl%d: %s: channel invalid\n", pi->sh->unit, __FUNCTION__));
		ASSERT(0);
	}

	if (ci == NULL) {
		PHY_ERROR(("wl%d: %s: Tuning Table Retrieved is Empty.\n",
		           pi->sh->unit, __FUNCTION__));
		ASSERT(0);
		OSL_DELAY(272);
	} else {
		/* XXX:
		 * Determine Fref from tuning table value
		 * instead of Radio Reg (Save Time)
		 */

		uint8 rfpll_refmaster =
		        ci->RF_rfpll_refmaster_sparextalsize & 0x30;
		if (rfpll_refmaster == 0x10) {
			OSL_DELAY(142);
		} else if (rfpll_refmaster == 0x20) {
			OSL_DELAY(177);
		} else if (rfpll_refmaster == 0x30) {
			OSL_DELAY(272);
		} else {
			PHY_ERROR(("wl%d: %s: RFPLL_REFMASTER = %d is not supported.\n",
			           pi->sh->unit, __FUNCTION__, rfpll_refmaster));
			ASSERT(0);
			OSL_DELAY(272);
		}
	}
}

static void
wlc_phy_radio2059_save_rcal_cache(phy_info_t *pi, uint16 rval)
{
	phy_info_htphy_t *pi_ht = (phy_info_htphy_t *)pi->u.pi_htphy;
	pi_ht->rcal_rccal_cache.rcal_val = rval;
}

static void
wlc_phy_radio2059_save_rccal_cache(phy_info_t *pi, uint16 bcap, uint16 scap, uint16 hpc)
{
	phy_info_htphy_t *pi_ht = (phy_info_htphy_t *)pi->u.pi_htphy;
	pi_ht->rcal_rccal_cache.rccal_bcap_val = bcap;
	pi_ht->rcal_rccal_cache.rccal_scap_val = scap;
	pi_ht->rcal_rccal_cache.rccal_hpc_val = hpc;
}
static void
wlc_phy_radio2059_restore_rcal_rccal_cache(phy_info_t *pi)
{
	phy_info_htphy_t *pi_ht = (phy_info_htphy_t *)pi->u.pi_htphy;
	htphy_rcal_rccal_cache *rcache = &pi_ht->rcal_rccal_cache;
	uint16 rcal_val = rcache->rcal_val;
	uint16 rccal_bcap_val = rcache->rccal_bcap_val;
	uint16 rccal_scap_val = rcache->rccal_scap_val;
	uint16 rccal_hpc_val = rcache->rccal_hpc_val;

	if (!RADIO2059_RCAL_RCCAL_CACHE_VALID(pi)) {
		PHY_ERROR(("wl%d: %s: rccal,rcal cache not valid, do nothing\n",
		           pi->sh->unit, __FUNCTION__));
		ASSERT(RADIO2059_RCAL_RCCAL_CACHE_VALID(pi));
		return;
	}

	/* XXX Apply R-CAL Override. Ignore the LSB of rcal_val as it's not used.
	 * Override Value - i_wrf_jtag_bandgap_B
	 * Override En - ovr_i_wrf_jtag_bandgap_B
	 */
	phy_utils_write_radioreg(pi, RADIO_2059_BANDGAP_RCAL_TRIM, (rcal_val << 3) | 0x6);
	phy_utils_write_radioreg(pi, RADIO_2059_OVR_REG5(0), 1 << 6);

	/* XXX Apply RCCAP HPC Overrides
	 * Override Value - i_wrf_jtag_rxbb_RCCAL_HPC
	 * Override En - ovr_i_wrf_jtag_rxbb_RCCAL_HPC
	 */
	phy_utils_write_radioreg(pi, RADIO_2059_RXBB_RCCAL_HPC(3), rccal_hpc_val);
	phy_utils_write_radioreg(pi, RADIO_2059_OVR_REG15(3), 1 << 6);

	/* XXX Apply RCCAP BCAP/SCAP Overrides
	 * Override Value - i_wrf_jtag_RCCAL_TX
	 * Override En - ovr_rccal_bcap, ovr_rccal_scap
	 */
	phy_utils_write_radioreg(pi, RADIO_2059_RCCAL_OVERRIDES(3), 0x3);
	phy_utils_write_radioreg(pi, RADIO_2059_TXLPF_RCCAL(3), rccal_scap_val);
	phy_utils_write_radioreg(pi, RADIO_2059_RXBB_GPAIOSEL_RXLPF_RCCAL(3), rccal_bcap_val);
}

#define MAX_2059_RCAL_WAITLOOPS 10000

static void
wlc_phy_radio2059_rcal(phy_info_t *pi)
{
	uint16 rcal_reg = 0, rcal_val = 0;
	int i;

	if (ISSIM_ENAB(pi->sh->sih))
		return;

	phy_utils_mod_radioreg(pi, RADIO_2059_RCAL_CONFIG, 0x1, 0x1);
	OSL_DELAY(10);

	phy_utils_mod_radioreg(pi, RADIO_2059_IQTEST_SEL_PU,  0x1, 0x1);
	phy_utils_mod_radioreg(pi, RADIO_2059_IQTEST_SEL_PU2, 0x3, 0x2);

	phy_utils_mod_radioreg(pi, RADIO_2059_RCAL_CONFIG, 0x2, 0x2);
	OSL_DELAY(100);
	phy_utils_mod_radioreg(pi, RADIO_2059_RCAL_CONFIG, 0x2, 0x0);

	for (i = 0; i < MAX_2059_RCAL_WAITLOOPS; i++) {
		rcal_reg = phy_utils_read_radioreg(pi, RADIO_2059_RCAL_STATUS);
		if (rcal_reg & 0x1) {
			break;
		}
		OSL_DELAY(100);
	}
	ASSERT(rcal_reg & 0x1);

	rcal_reg = phy_utils_read_radioreg(pi, RADIO_2059_RCAL_STATUS) & 0x3e;
	rcal_val = rcal_reg >> 1;

	phy_utils_mod_radioreg(pi, RADIO_2059_RCAL_CONFIG, 0x1, 0x0);

	phy_utils_mod_radioreg(pi, RADIO_2059_BANDGAP_RCAL_TRIM, 0xf0, rcal_val << 3);

	wlc_phy_radio2059_save_rcal_cache(pi, rcal_val);

	PHY_INFORM(("wl%d: %s rcal=%d\n", pi->sh->unit, __FUNCTION__, rcal_val));
}

#define MAX_2059_RCCAL_WAITLOOPS 10000
#define NUM_2059_RCCAL_CAPS 3

static void
wlc_phy_radio2059_rccal(phy_info_t *pi)
{
	uint8 master_val[] = {0x61, 0x69, 0x73};
	uint8 x1_val[]     = {0x6e, 0x6e, 0x6e};
	uint8 trc0_val[]   = {0xe9, 0xd5, 0x99};
	uint16 rccal_valid, bcap, scap, hpc;
	int i, cal;

	if (ISSIM_ENAB(pi->sh->sih))
		return;

	for (cal = 0; cal < NUM_2059_RCCAL_CAPS; cal++) {
		rccal_valid = 0;
		phy_utils_write_radioreg(pi, RADIO_2059_RCCAL_MASTER, master_val[cal]);
		phy_utils_write_radioreg(pi, RADIO_2059_RCCAL_X1, x1_val[cal]);
		phy_utils_write_radioreg(pi, RADIO_2059_RCCAL_TRC0, trc0_val[cal]);

		OSL_DELAY(35);

		phy_utils_write_radioreg(pi, RADIO_2059_RCCAL_START_R1_Q1_P1, 0x55);

		OSL_DELAY(70);

		for (i = 0; i < MAX_2059_RCCAL_WAITLOOPS; i++) {
			rccal_valid = phy_utils_read_radioreg(pi, RADIO_2059_RCCAL_DONE_OSCCAP);
			if (rccal_valid & 0x2) {
				break;
			}
			OSL_DELAY(25);
		}
		ASSERT(rccal_valid & 0x2);

		phy_utils_write_radioreg(pi, RADIO_2059_RCCAL_START_R1_Q1_P1, 0x15);
	}

	phy_utils_mod_radioreg(pi, RADIO_2059_RCCAL_MASTER, 1, 0);

	scap = phy_utils_read_radioreg(pi, RADIO_2059_RCCAL_SCAP_VAL);
	bcap = phy_utils_read_radioreg(pi, RADIO_2059_RCCAL_BCAP_VAL);
	hpc  = phy_utils_read_radioreg(pi, RADIO_2059_RCCAL_HPC_VAL);

	wlc_phy_radio2059_save_rccal_cache(pi, bcap, scap, hpc);

	PHY_INFORM(("wl%d: %s rccal bcap=%d scap=%d hpc=%d\n",
	            pi->sh->unit, __FUNCTION__, bcap, scap, hpc));

}

static void
wlc_phy_radio2059_init(phy_info_t *pi)
{
	radio_20xx_regs_t *regs_2059_ptr = NULL;
	uint16 core;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	if (HTREV_GE(pi->pubpi.phy_rev, 1)) {
		regs_2059_ptr = regs_2059_rev0v1;
	} else {
		regs_2059_ptr = regs_2059_rev0;
	}

	wlc_phy_radio2059_reset(pi);

	wlc_phy_init_radio_regs_allbands(pi, regs_2059_ptr);

	FOREACH_CORE(pi, core) {
		/* XXX Enable PHY direct control ("pin control")
		 * Enable the xtal_pu_ovr. Thereby the settings in the RF_xtal_config1
		 * and RF_xtal_config2 regss will be used instead of the xtal being
		 * controlled by radio_xtal_pu
		 */
		phy_utils_mod_radioreg(pi, RADIO_2059_XTALPUOVR_PINCTRL(core), 0x3, 0x3);
	}

	phy_utils_mod_radioreg(pi, RADIO_2059_RFPLL_MISC_CAL_RESETN, 0x78, 0x78);
	phy_utils_mod_radioreg(pi, RADIO_2059_XTAL_CONFIG2,          0x80, 0x80);
	OSL_DELAY(20);
	phy_utils_mod_radioreg(pi, RADIO_2059_RFPLL_MISC_CAL_RESETN, 0x78, 0x0);
	phy_utils_mod_radioreg(pi, RADIO_2059_XTAL_CONFIG2,          0x80, 0x0);

	/* XXX PR89866: R-cal and RC-cal Required on Driver Attach
	 * Cache Cal Values and apply the values via overrides
	 * Overrides are necessary as everytime the radio is reset,
	 * the latches are reset
	 */
	if (!RADIO2059_RCAL_RCCAL_CACHE_VALID(pi)) {
		wlc_phy_radio2059_rcal(pi);
		wlc_phy_radio2059_rccal(pi);
		RADIO2059_VALIDATE_RCAL_RCCAL_CACHE(pi, TRUE);
	}
	wlc_phy_radio2059_restore_rcal_rccal_cache(pi);

	phy_utils_mod_radioreg(pi, RADIO_2059_RFPLL_MASTER, 0x8, 0x0);
}

/*  lookup radio-chip-specific channel code */
static bool
wlc_phy_chan2freq_htphy(phy_info_t *pi, uint channel, int *f, chan_info_htphy_radio2059_t **t)
{
	uint i;
	chan_info_htphy_radio2059_t *chan_info_tbl = NULL;
	uint32 tbl_len = 0;

	int freq = 0;

	if (HTREV_GE(pi->pubpi.phy_rev, 1)) {
		chan_info_tbl = chan_info_htphyrev1_2059rev0v1;
		tbl_len = ARRAYSIZE(chan_info_htphyrev1_2059rev0v1);
	} else {
		chan_info_tbl = chan_info_htphyrev0_2059rev0;
		tbl_len = ARRAYSIZE(chan_info_htphyrev0_2059rev0);
	}

	for (i = 0; i < tbl_len; i++) {
		if (chan_info_tbl[i].chan == channel)
			break;
	}

	if (i >= tbl_len) {
		PHY_ERROR(("wl%d: %s: channel %d not found in channel table\n",
		           pi->sh->unit, __FUNCTION__, channel));
		ASSERT(i < tbl_len);
		goto fail;
	}
	*t = &chan_info_tbl[i];
	freq = chan_info_tbl[i].freq;

	*f = freq;
	return TRUE;

fail:
	*f = WL_CHAN_FREQ_RANGE_2G;
	return FALSE;
}

/* get the complex freq. if chan==0, use default radio channel */
uint8
wlc_phy_get_chan_freq_range_htphy(phy_info_t *pi, uint channel)
{
	int freq;
	chan_info_htphy_radio2059_t *t = NULL;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	if (channel == 0)
		channel = CHSPEC_CHANNEL(pi->radio_chanspec);

	if (!wlc_phy_chan2freq_htphy(pi, channel, &freq, &t)) {
		PHY_ERROR(("wl%d: %s: channel invalid\n", pi->sh->unit, __FUNCTION__));
		ASSERT(0);
	}

	if (channel <= CH_MAX_2G_CHANNEL)
		return WL_CHAN_FREQ_RANGE_2G;
	else {
		if (pi->sromi->subband5Gver == PHY_SUBBAND_4BAND) {
			if ((freq >= PHY_SUBBAND_4BAND_BAND0) &&
				(freq < PHY_SUBBAND_4BAND_BAND1))
				return WL_CHAN_FREQ_RANGE_5G_BAND0;
			else if ((freq >= PHY_SUBBAND_4BAND_BAND1) &&
				(freq < PHY_SUBBAND_4BAND_BAND2))
				return WL_CHAN_FREQ_RANGE_5G_BAND1;
			else if ((freq >= PHY_SUBBAND_4BAND_BAND2) &&
				(freq < PHY_SUBBAND_4BAND_BAND3))
				return WL_CHAN_FREQ_RANGE_5G_BAND2;
			else
				return WL_CHAN_FREQ_RANGE_5G_BAND3;
		} else if (pi->sromi->subband5Gver == PHY_SUBBAND_3BAND_EMBDDED) {
			if ((freq >= EMBEDDED_LOW_5G_CHAN) && (freq < EMBEDDED_MID_5G_CHAN)) {
				return WL_CHAN_FREQ_RANGE_5G_BAND0;
			} else if ((freq >= EMBEDDED_MID_5G_CHAN) &&
			           (freq < EMBEDDED_HIGH_5G_CHAN)) {
				return WL_CHAN_FREQ_RANGE_5G_BAND1;
			} else {
				return WL_CHAN_FREQ_RANGE_5G_BAND2;
			}
		} else if (pi->sromi->subband5Gver == PHY_SUBBAND_3BAND_HIGHPWR) {
			if ((freq >= HIGHPWR_LOW_5G_CHAN) && (freq < HIGHPWR_MID_5G_CHAN)) {
				return WL_CHAN_FREQ_RANGE_5G_BAND0;
			} else if ((freq >= HIGHPWR_MID_5G_CHAN) && (freq < HIGHPWR_HIGH_5G_CHAN)) {
				return WL_CHAN_FREQ_RANGE_5G_BAND1;
			} else {
				return WL_CHAN_FREQ_RANGE_5G_BAND2;
			}
		} else { /* Default PPR Subband subband5Gver = 7 */
			if ((freq >= JAPAN_LOW_5G_CHAN) && (freq < JAPAN_MID_5G_CHAN)) {
				return WL_CHAN_FREQ_RANGE_5G_BAND0;
			} else if ((freq >= JAPAN_MID_5G_CHAN) && (freq < JAPAN_HIGH_5G_CHAN)) {
				return WL_CHAN_FREQ_RANGE_5G_BAND1;
			} else {
				return WL_CHAN_FREQ_RANGE_5G_BAND2;
			}
		}
	}

}

static void
wlc_phy_chanspec_radio2059_setup(phy_info_t *pi, const chan_info_htphy_radio2059_t *ci)
{
	uint8 core;

	/* Tuning */
	phy_utils_write_radioreg(pi,
	                RADIO_2059_VCOCAL_COUNTVAL0, ci->RF_vcocal_countval0);
	phy_utils_write_radioreg(pi,
	                RADIO_2059_VCOCAL_COUNTVAL1, ci->RF_vcocal_countval1);
	phy_utils_write_radioreg(pi,
	                RADIO_2059_RFPLL_REFMASTER_SPAREXTALSIZE,
	                ci->RF_rfpll_refmaster_sparextalsize);
	phy_utils_write_radioreg(pi,
	                RADIO_2059_RFPLL_LOOPFILTER_R1, ci->RF_rfpll_loopfilter_r1);
	phy_utils_write_radioreg(pi,
	                RADIO_2059_RFPLL_LOOPFILTER_C2, ci->RF_rfpll_loopfilter_c2);
	phy_utils_write_radioreg(pi,
	                RADIO_2059_RFPLL_LOOPFILTER_C1, ci->RF_rfpll_loopfilter_c1);
	phy_utils_write_radioreg(pi,
	                RADIO_2059_CP_KPD_IDAC, ci->RF_cp_kpd_idac);
	phy_utils_write_radioreg(pi,
	                RADIO_2059_RFPLL_MMD0, ci->RF_rfpll_mmd0);
	phy_utils_write_radioreg(pi,
	                RADIO_2059_RFPLL_MMD1, ci->RF_rfpll_mmd1);
	phy_utils_write_radioreg(pi,
	                RADIO_2059_VCOBUF_TUNE, ci->RF_vcobuf_tune);
	phy_utils_write_radioreg(pi,
	                RADIO_2059_LOGEN_MX2G_TUNE, ci->RF_logen_mx2g_tune);
	phy_utils_write_radioreg(pi,
	                RADIO_2059_LOGEN_MX5G_TUNE, ci->RF_logen_mx5g_tune);
	phy_utils_write_radioreg(pi,
	                RADIO_2059_LOGEN_INDBUF2G_TUNE, ci->RF_logen_indbuf2g_tune);

	FOREACH_CORE(pi, core) {
		phy_utils_write_radioreg(pi, RADIO_2059_LOGEN_INDBUF5G_TUNE(core),
		                TUNING_REG(ci->RF_logen_indbuf5g_tune_core, core));
		phy_utils_write_radioreg(pi, RADIO_2059_TXMIX2G_TUNE_BOOST_PU(core),
		                TUNING_REG(ci->RF_txmix2g_tune_boost_pu_core, core));
		phy_utils_write_radioreg(pi, RADIO_2059_PAD2G_TUNE_PUS(core),
		                TUNING_REG(ci->RF_pad2g_tune_pus_core, core));
		phy_utils_write_radioreg(pi, RADIO_2059_PGA_BOOST_TUNE(core),
		                TUNING_REG(ci->RF_pga_boost_tune_core, core));
		phy_utils_write_radioreg(pi, RADIO_2059_TXMIX5G_BOOST_TUNE(core),
		                TUNING_REG(ci->RF_txmix5g_boost_tune_core, core));
		phy_utils_write_radioreg(pi, RADIO_2059_PAD5G_TUNE_MISC_PUS(core),
		                TUNING_REG(ci->RF_pad5g_tune_misc_pus_core, core));
		phy_utils_write_radioreg(pi, RADIO_2059_LNA2G_TUNE(core),
		                TUNING_REG(ci->RF_lna2g_tune_core, core));
		phy_utils_write_radioreg(pi, RADIO_2059_LNA5G_TUNE(core),
		                TUNING_REG(ci->RF_lna5g_tune_core, core));
	}

	/* Do a VCO cal after writing the tuning table regs */
	wlc_phy_radio2059_vcocal(pi);
}

static void
wlc_phy_chanspec_setup_htphy(phy_info_t *pi, chanspec_t chanspec, const htphy_sfo_cfg_t *ci)
{
	uint16 val;
	uint8 spuravoid = WL_SPURAVOID_OFF;
	uint16 chan = CHSPEC_CHANNEL(chanspec);
	uint8 region_group = wlc_phy_get_locale(pi->rxgcrsi);
	int32 eu_edthresh;

	/* Any channel (or sub-band) specific settings go in here */

	/* Set phy Band bit, required to ensure correct band's tx/rx board
	 * level controls are being driven, 0 = 2.4G, 1 = 5G.
	 * Enable/disable BPHY if channel is in 2G/5G band, respectively.
	 */
	if (CHSPEC_IS5G(chanspec)) { /* we are in 5G */
		/* switch BandControl to 2G to make BPHY regs accessible */
		phy_utils_and_phyreg(pi, HTPHY_BandControl, ~HTPHY_BandControl_currentBand_MASK);
		/* enable force gated clock on */
		val = R_REG(pi->sh->osh, &pi->regs->psm_phy_hdr_param);
		W_REG(pi->sh->osh, &pi->regs->psm_phy_hdr_param, (val | MAC_PHY_FORCE_CLK));
		/* enable bphy resetCCA and put bphy receiver in reset */
		phy_utils_or_phyreg(pi, (HTPHY_TO_BPHY_OFF + BPHY_BB_CONFIG),
		           (BBCFG_RESETCCA | BBCFG_RESETRX));
		/* restore force gated clock */
		W_REG(pi->sh->osh, &pi->regs->psm_phy_hdr_param, val);
		/* switch BandControl to 5G */
		phy_utils_or_phyreg(pi, HTPHY_BandControl, HTPHY_BandControl_currentBand_MASK);
	} else { /* we are in 2G */
		/* switch BandControl to 2G */
		phy_utils_and_phyreg(pi, HTPHY_BandControl, ~HTPHY_BandControl_currentBand_MASK);
		/* enable force gated clock on */
		val = R_REG(pi->sh->osh, &pi->regs->psm_phy_hdr_param);
		W_REG(pi->sh->osh, &pi->regs->psm_phy_hdr_param, (val | MAC_PHY_FORCE_CLK));
		/* disable bphy resetCCA and take bphy receiver out of reset */
		phy_utils_and_phyreg(pi, (HTPHY_TO_BPHY_OFF + BPHY_BB_CONFIG),
		            (uint16)(~(BBCFG_RESETCCA | BBCFG_RESETRX)));
		/* restore force gated clock */
		W_REG(pi->sh->osh, &pi->regs->psm_phy_hdr_param, val);
	}

	/* set SFO parameters
	 * sfo_chan_center_Ts20 = round([fc-10e6 fc fc+10e6] / 20e6 * 8), fc in Hz
	 *                      = round([$channel-10 $channel $channel+10] * 0.4),
	 *                              $channel in MHz
	 */
	phy_utils_write_phyreg(pi, HTPHY_BW1a, ci->PHY_BW1a);
	phy_utils_write_phyreg(pi, HTPHY_BW2, ci->PHY_BW2);
	phy_utils_write_phyreg(pi, HTPHY_BW3, ci->PHY_BW3);

	/* sfo_chan_center_factor = round(2^17./([fc-10e6 fc fc+10e6]/20e6)), fc in Hz
	 *                        = round(2621440./[$channel-10 $channel $channel+10]),
	 *                                $channel in MHz
	 */
	phy_utils_write_phyreg(pi, HTPHY_BW4, ci->PHY_BW4);
	phy_utils_write_phyreg(pi, HTPHY_BW5, ci->PHY_BW5);
	phy_utils_write_phyreg(pi, HTPHY_BW6, ci->PHY_BW6);

	/* PR 41942 WAR
	 *     - turn off OFDM classification for channel 14
	 *     - this affects _all_ countries though needed only for locale c/a2
	 */
	if (chan == 14) {
		wlc_phy_classifier_htphy(pi, HTPHY_ClassifierCtrl_ofdm_en, 0);
		/* Bit 11 and 6 of BPHY testRegister to '10' */
		phy_utils_or_phyreg(pi, HTPHY_TO_BPHY_OFF + BPHY_TEST, 0x800);
	} else {
		wlc_phy_classifier_htphy(pi, HTPHY_ClassifierCtrl_ofdm_en,
			HTPHY_ClassifierCtrl_ofdm_en);
		/* Bit 11 and 6 of BPHY testRegister to '00' */
		if (CHSPEC_IS2G(chanspec))
			phy_utils_and_phyreg(pi, HTPHY_TO_BPHY_OFF + BPHY_TEST, ~0x840);
	}

	/* set txgain in case txpwrctrl is disabled */
	wlc_phy_txpwr_fixpower_htphy(pi);

	/* Set the analog TX_LPF Bandwidth */
	wlc_phy_txlpfbw_htphy(pi);

	/* dynamic spur avoidance, user can override the behavior */
	if (!CHSPEC_IS40(chanspec)) {
		if ((chan == 13) || (chan == 14) || (chan == 153)) {
			spuravoid = WL_SPURAVOID_ON1;
		}
	} else { /* 40 MHz b/w */
		/* XXX PR 87046 WAR, Avoid using spur avoidance for 40MHz
		 *  channels for 4331B0/B1 due to high RX PER floor ( >10%)
		 */
		if (HTREV_IS(pi->pubpi.phy_rev, 0)) {
			if (chan == 54) {
				spuravoid = WL_SPURAVOID_ON1;
			} else if ((chan == 118) || (chan == 151)) {
				spuravoid = WL_SPURAVOID_ON2;
			}
		}
	}

	if (pi->phy_spuravoid == SPURAVOID_DISABLE) {
		spuravoid = WL_SPURAVOID_OFF;
	} else if (pi->phy_spuravoid == SPURAVOID_FORCEON) {
		spuravoid = WL_SPURAVOID_ON1;
	} else if (pi->phy_spuravoid == SPURAVOID_FORCEON2) {
		spuravoid = WL_SPURAVOID_ON2;
	}

	/* XXX FIXME:
	 * To improve channel switching time, the following PLL/clk settings
	 * should only be executed if really necessary - The PLL may take up
	 * to 0.4ms to lock. Necessary conditions are:
	 * - spur avoidance mode changed
	 * - anything else ? Resampler setup needs refresh after PHY reset
	 */

	/* PLL parameter changes */
	wlapi_bmac_core_phypll_ctl(pi->sh->physhim, FALSE);
	si_pmu_spuravoid(pi->sh->sih, pi->sh->osh, spuravoid);
	wlapi_bmac_core_phypll_ctl(pi->sh->physhim, TRUE);

	/* MAX TSF clock setup for chips where PHY & MAC share same PLL */
	wlapi_switch_macfreq(pi->sh->physhim, spuravoid);

	/* Do a soft dreset of the PLL */
	wlapi_bmac_core_phypll_reset(pi->sh->physhim);

	/* Setup resampler */
	MOD_PHYREG(pi, HTPHY_BBConfig, resample_clk160,
	           (spuravoid == WL_SPURAVOID_OFF) ? 0 : 1);
	if (spuravoid != WL_SPURAVOID_OFF) {
		MOD_PHYREG(pi, HTPHY_BBConfig, resamplerFreq,
		           (spuravoid - WL_SPURAVOID_ON1));
	}

	/* Clean up */
	wlc_phy_resetcca_htphy(pi);

	PHY_TRACE(("wl%d: %s spuravoid=%d\n", pi->sh->unit, __FUNCTION__, spuravoid));

	wlc_phy_subband_cust_htphy(pi);

	/* XXX Spurwar backs off initgain on core 0 for 5G SPUR channels (PR87046)
	 * - Causes large error in core 0 rssi
	 * WAR: Do rssi gainerror calculation AFTER subband_cust, BEFORE spurwar
	 */
	phy_ht_rssi_init_gain_err(pi->u.pi_htphy->rssii);

	wlc_phy_tx_digi_filts_htphy_war(pi, FALSE);

	wlc_phy_spurwar_htphy(pi);

	/* SET Vmid and Av for {TSSI} based on ID pdetrangeXg FROM SROM */
	wlc_phy_tssi_pdetrange_cust_htphy(pi);

	/* Restore/Re-Cal Idle TSSI & Vmid */
	if (pi->itssical || (pi->itssi_cap_low5g && chan <= 48 && chan >= 36))
		wlc_phy_txpower_recalc_idle_tssi_htphy(pi);

	if (CHSPEC_IS5G(chanspec)) {
		/* 5G LNA1 bias boost changes;
		 * these two register changes apply to all cores
		 */
		phy_utils_write_radioreg(pi, RADIO_2059_RXRFBIAS_IBOOST_PU(3), 0xFE);
		phy_utils_write_radioreg(pi, RADIO_2059_RXRF_IABAND_RXGM_IMAIN_PTAT(3), 0xC0);
	}

	/* XXX Activate TX Power Savings (Only 5G for now) based on boardflag:
	 *  BFL2_PWR_NOMINAL -> 1 -> No power saving measures adopted
	 * Dropping iPA bias currents if BFL2_PWR_NOMINAL -> 0
	 */
	if (!(BOARDFLAGS2(GENERIC_PHY_INFO(pi)->boardflags2) & BFL2_PWR_NOMINAL) &&
	    (CHSPEC_IS5G(chanspec))) {
		/* Tx 5G iPA settings */
		phy_utils_write_radioreg(pi, RADIO_2059_IPA5G_IAUX(3), 0x9);
		phy_utils_write_radioreg(pi, RADIO_2059_IPA5G_IMAIN(3), 0xA);
	}

	/* XXX 5G LOGEN bias current change required for regulatory
	 * the first register applies only to core1,
	 * the second applies to both cores
	 */
	if (CHSPEC_IS5G(chanspec)) {
		phy_utils_write_radioreg(pi, RADIO_2059_LOGEN_INDBUF5G_IDAC(1), 0xD);
		phy_utils_write_radioreg(pi, RADIO_2059_LOGEN_MX5G_IDACS, 0x3C);
	}

	/* ED thresh */
	if (region_group == REGION_EU) {
		if  (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA) {
			/* 2g: Make default to be -69dBm to pass EU */
			eu_edthresh = CHSPEC_IS2G(chanspec) ? -69 : -62;
			wlc_phy_adjust_ed_thres_htphy(pi, &eu_edthresh, TRUE);
		}

		/* Override from SROM if SROM has an entry */
		wlc_phy_set_srom_eu_edthresh_htphy(pi);
	}
}

void
wlc_phy_get_rxiqest_gain_htphy(phy_info_t *pi, int16 *rxiqest_gain)
{
	if (IS_X12_BOARDTYPE(pi)) {
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			*rxiqest_gain = (int16)(HTPHY_NOISE_INITGAIN_X12_2G);
		} else {
			*rxiqest_gain = (int16)(HTPHY_NOISE_INITGAIN_X12_5G);
		}
	} else if (IS_X29B_BOARDTYPE(pi) || (IS_X33_BOARDTYPE(pi) &&
	                                     (pi->sh->boardrev >= 0x1602))) {
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			*rxiqest_gain = (int16)(HTPHY_NOISE_INITGAIN_X29_2G);
		} else {
			*rxiqest_gain = (int16)(HTPHY_NOISE_INITGAIN_X29_5G);
		}
	} else {
		*rxiqest_gain = (int16)(HTPHY_NOISE_INITGAIN);
	}
	return;
}

static void
wlc_phy_BT_SwControl_setup_htphy(phy_info_t *pi, chanspec_t chanspec)
{
	if (IS_X28_BOARDTYPE(pi)) {
		/* Setup LUT: */
		/* XXX
		 * Second entry of bt_en_lut0/1 is for bt_shd1, which is unused here.
		 * Nonetheless, this entry is correctly programmed to be the
		 * complement of the first entry, used for bt_shd0.
		 */
		MOD_PHYREG(pi, HTPHY_BT_SwControl, bt_en_lut0, 0x1);
		MOD_PHYREG(pi, HTPHY_BT_SwControl, bt_en_lut1, 0x2);
	}
}

void
wlc_phy_chanspec_set_htphy(phy_info_t *pi, chanspec_t chanspec)
{
	int freq;
	chan_info_htphy_radio2059_t *t = NULL;

	bool suspend = FALSE;

	PHY_TRACE(("wl%d: %s chan = %d\n", pi->sh->unit, __FUNCTION__, CHSPEC_CHANNEL(chanspec)));

	if (!wlc_phy_chan2freq_htphy(pi, CHSPEC_CHANNEL(chanspec), &freq, &t))
		return;

	wlc_phy_chanspec_radio_set((wlc_phy_t *)pi, chanspec);

	/* XXX
	 * 1. PR109265
	 *    On a channel change, first restore aci mitigation-OFF register settings, followed by
	 *    chanspec_setup/subband_cust, and then followed by restoring aci mitigation-ON
	 *    values on home channel if ACI state was active. This fixes the problem of aci
	 *    parameters getting clobbered due to channel scan.
	 * 2. Do the above for bphy desense as well: first remove desense, then run subband_cust,
	 *    then reapply desense settings if on home channel.
	 */

	if ((pi->interf->curr_home_channel != CHSPEC_CHANNEL(pi->radio_chanspec)) &&
	    (pi->interf->bphy_desense_index != 0)) {
		/* Save current bphy desense index */
		pi->interf->bphy_desense_index_scan_restore = pi->interf->bphy_desense_index;
		/* Set bphy desense to OFF */
		wlc_phy_bphynoise_hw_set_htphy(pi, 0, FALSE);
	}

	/* Restore ACI mitigation OFF parameters */
	if ((((pi->aci_state & ACI_ACTIVE)) &&
		(pi->interf->curr_home_channel !=
		CHSPEC_CHANNEL(pi->radio_chanspec))) ||
		(pi->sh->interference_mode == WLAN_MANUAL)) {
		wlc_phy_acimode_set_htphy(pi, FALSE, PHY_ACI_PWR_NOTPRESENT);
	}

	/* Set the phy bandwidth as dictated by the chanspec */
	if (CHSPEC_BW(chanspec) != pi->bw)
		wlapi_bmac_bw_set(pi->sh->physhim, CHSPEC_BW(chanspec));

	/* Set the correct sideband if in 40MHz mode */
	if (CHSPEC_IS40(chanspec)) {
		if (CHSPEC_SB_UPPER(chanspec)) {
			phy_utils_or_phyreg(pi, HTPHY_RxControl,
			                    HTPHY_RxControl_bphy_band_sel_MASK);
			phy_utils_or_phyreg(pi, HTPHY_ClassifierCtrl2,
			           HTPHY_ClassifierCtrl2_prim_sel_MASK);
		} else {
			phy_utils_and_phyreg(pi, HTPHY_RxControl,
			                     ~HTPHY_RxControl_bphy_band_sel_MASK);
			phy_utils_and_phyreg(pi, HTPHY_ClassifierCtrl2,
			            (~HTPHY_ClassifierCtrl2_prim_sel_MASK & 0xffff));
		}
	}

	/* band specific 2059 radio inits */

	/* do chanspec setup for radio and phy */
	wlc_phy_chanspec_radio2059_setup(pi, t);
	wlc_phy_chanspec_setup_htphy(pi, chanspec, (const htphy_sfo_cfg_t *)&(t->PHY_BW1a));
	/* store aci mitigation off values */
	wlc_phy_aci_noise_store_values_htphy(pi);

	if (pi->u.pi_htphy->btc_restage_rxgain)
		wlc_phy_btc_restage_rxgain_htphy(pi, TRUE);

	/* XXX
	 * PR114196: Set high MF threshold for all RSSIs in 2G if interference mode is not 0
	 */
	wlc_phy_noise_raise_MFthresh_htphy(pi, (CHSPEC_IS2G(pi->radio_chanspec) &&
	                                        (pi->sh->interference_mode != INTERFERE_NONE)));

	if ((CHSPEC_CHANNEL(chanspec) == pi->interf->curr_home_channel) ||
		(pi->sh->interference_mode == WLAN_MANUAL)) {
		/* back in home channel */
		/* if aci mitigation was on, turn it back on */
		if (CHSPEC_IS2G(chanspec) &&
			(((pi->sh->interference_mode == WLAN_AUTO_W_NOISE ||
			pi->sh->interference_mode == WLAN_AUTO) &&
			(pi->aci_state & ACI_ACTIVE)) ||
			pi->sh->interference_mode == WLAN_MANUAL)) {
				if (CHSPEC_IS2G(pi->radio_chanspec)) {
					wlc_phy_acimode_set_htphy(pi,
						TRUE, PHY_ACI_PWR_HIGH);
				}
		}

		if ((pi->sh->interference_mode == WLAN_AUTO_W_NOISE ||
			pi->sh->interference_mode == NON_WLAN) &&
			(pi->phy_init_por == FALSE)) {

			/* suspend mac if haven't done so */
			suspend = !(R_REG(pi->sh->osh, &pi->regs->maccontrol) & MCTL_EN_MAC);
			if (!suspend) {
				wlapi_suspend_mac_and_wait(pi->sh->physhim);
			}
			/* Restore ofdm desense settings */
			wlc_phy_noise_update_crsminpwr_htphy(pi);
			/* Restore bphy desense settings */
			if (CHSPEC_IS2G(chanspec) &&
			    (pi->interf->bphy_desense_index_scan_restore != 0)) {
				wlc_phy_bphynoise_hw_set_htphy(pi,
				        pi->interf->bphy_desense_index_scan_restore, FALSE);
				pi->interf->bphy_desense_index_scan_restore = 0;
			}
			/* unsuspend mac */
			if (!suspend) {
				wlapi_enable_mac(pi->sh->physhim);
			}
		}
	}

#if defined(AP) && defined(RADAR)
	/* update radar detect mode specific params
	 * based on new chanspec
	 */
	phy_ht_radar_upd(pi->u.pi_htphy->radari);
#endif /* defined(AP) && defined(RADAR) */

}

static void
wlc_phy_txlpfbw_htphy(phy_info_t *pi)
{
}

void
wlc_phy_btc_restage_rxgain_htphy(phy_info_t *pi, bool enable)
{
	phy_info_htphy_t *pi_ht = (phy_info_htphy_t *)pi->u.pi_htphy;
	bool suspend;

	/* suspend mac if haven't done so */
	suspend = !(R_REG(pi->sh->osh, &pi->regs->maccontrol) & MCTL_EN_MAC);
	if (!suspend) {
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
	}

	if (IS_X29B_BOARDTYPE(pi)) {
		if (enable) {
			uint8 antswctrl_byp_elna2G = 0;
			wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_ANTSWCTRLLUT,
			                          1, 2,  8, &antswctrl_byp_elna2G);
			wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_ANTSWCTRLLUT,
			                          1, 10,  8, &antswctrl_byp_elna2G);
			wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_ANTSWCTRLLUT,
			                          1, 34,  8, &antswctrl_byp_elna2G);
			wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_ANTSWCTRLLUT,
			                          1, 42,  8, &antswctrl_byp_elna2G);

			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				uint16 newinitgain = 0x9117;
				int8 elnabyp_gain[] = {-4, -4};

				if (pi->interf->bphy_desense_index == 0) {
					/* XXX
					 * if bphy desense is off, then backup from hw directly
					 */
					wlc_phy_table_read_htphy(pi, HTPHY_TBL_ID_RFSEQ, 3, 0x106,
					                         16, &pi_ht->btc_saved_init_rfseq);
					pi_ht->btc_saved_init_regs[0][0] =
					        phy_utils_read_phyreg(pi,
					                              HTPHY_Core0InitGainCodeA2059);
					pi_ht->btc_saved_init_regs[0][1] =
					        phy_utils_read_phyreg(pi,
					                              HTPHY_Core0InitGainCodeB2059);
					pi_ht->btc_saved_init_regs[1][0] =
					        phy_utils_read_phyreg(pi,
					                              HTPHY_Core1InitGainCodeA2059);
					pi_ht->btc_saved_init_regs[1][1] =
					        phy_utils_read_phyreg(pi,
					                              HTPHY_Core1InitGainCodeB2059);
					pi_ht->btc_saved_init_regs[2][0] =
					        phy_utils_read_phyreg(pi,
					                              HTPHY_Core2InitGainCodeA2059);
					pi_ht->btc_saved_init_regs[2][1] =
					        phy_utils_read_phyreg(pi,
					                              HTPHY_Core2InitGainCodeB2059);
				} else {
					/* XXX
					 * bphy desense may have modified initgain, so backup from
					 * aci/noise mitigation save/restore values.
					 */
					uint8 core;
					FOREACH_CORE(pi, core) {
						pi_ht->btc_saved_init_rfseq[core] =
						        pi->interf->init_gain_table_stored[core];
					}
					pi_ht->btc_saved_init_regs[0][0] =
					        pi->interf->init_gain_code_core0_stored;
					pi_ht->btc_saved_init_regs[0][1] =
					        pi->interf->init_gain_codeb_core0_stored;
					pi_ht->btc_saved_init_regs[1][0] =
					        pi->interf->init_gain_code_core1_stored;
					pi_ht->btc_saved_init_regs[1][1] =
					        pi->interf->init_gain_codeb_core1_stored;
					pi_ht->btc_saved_init_regs[2][0] =
					        pi->interf->init_gain_code_core2_stored;
					pi_ht->btc_saved_init_regs[2][1] =
					        pi->interf->init_gain_codeb_core2_stored;
				}
				pi_ht->btc_saved_nbclip[0] =
				        phy_utils_read_phyreg(pi, HTPHY_Core0nbClipThreshold);
				pi_ht->btc_saved_nbclip[1] =
				        phy_utils_read_phyreg(pi, HTPHY_Core1nbClipThreshold);
				pi_ht->btc_saved_nbclip[2] =
				        phy_utils_read_phyreg(pi, HTPHY_Core2nbClipThreshold);
				wlc_phy_table_read_htphy(pi, HTPHY_TBL_ID_GAIN1, 2, 0, 8,
				                         pi_ht->btc_saved_elnagain[0]);
				wlc_phy_table_read_htphy(pi, HTPHY_TBL_ID_GAIN2, 2, 0, 8,
				                         pi_ht->btc_saved_elnagain[1]);
				wlc_phy_table_read_htphy(pi, HTPHY_TBL_ID_GAIN3, 2, 0, 8,
				                         pi_ht->btc_saved_elnagain[2]);

				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_RFSEQ, 1, 0x106, 16,
				                          &newinitgain);
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_RFSEQ, 1, 0x108, 16,
				                          &newinitgain);
				phy_utils_write_phyreg(pi, HTPHY_Core0InitGainCodeA2059, 0x2e);
				phy_utils_write_phyreg(pi, HTPHY_Core2InitGainCodeA2059, 0x2e);
				phy_utils_write_phyreg(pi, HTPHY_Core0InitGainCodeB2059, 0x914);
				phy_utils_write_phyreg(pi, HTPHY_Core2InitGainCodeB2059, 0x914);
				phy_utils_write_phyreg(pi, HTPHY_Core0nbClipThreshold, 0x4b);
				phy_utils_write_phyreg(pi, HTPHY_Core2nbClipThreshold, 0x4b);

				/* XXX Update gain table to reflect elna gain(loss) in bypass -
				 * Needed for accurate RSSI reporting.
				 */
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1, 2, 0,
				                          8, elnabyp_gain);
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3, 2, 0,
				                          8, elnabyp_gain);

				if ((pi->sh->interference_mode == NON_WLAN) &&
				    (pi->interf->bphy_desense_lut !=
				     HTPHY_bphy_desense_lut_X29B_BTON)) {
					uint16 new_initgain[PHY_CORE_MAX];
					new_initgain[0] = 0x9117;
					new_initgain[1] = pi->interf->init_gain_table_stored[1];
					new_initgain[2] = 0x9117;
					wlc_phy_bphynoise_update_lut_htphy(
						pi, HTPHY_bphy_desense_lut_X29B_BTON,
						sizeof(HTPHY_bphy_desense_lut_X29B_BTON)/
						sizeof(bphy_desense_info_t),
						HTPHY_BPHY_MIN_SENSITIVITY_X29B_BTON,
						new_initgain);
				}
			} else {
				uint16 newinitgain = 0x8226;
				int8 new_lna1gains[4], new_lna2gains[4], idx;

				/* lna1 tables */
				wlc_phy_table_read_htphy(pi, HTPHY_TBL_ID_GAINBITS1, 4, 0x8, 8,
				                         pi_ht->btc_saved_lna1gainbits[0]);
				wlc_phy_table_read_htphy(pi, HTPHY_TBL_ID_GAINBITS3, 4, 0x8, 8,
				                         pi_ht->btc_saved_lna1gainbits[2]);
				wlc_phy_table_read_htphy(pi, HTPHY_TBL_ID_GAIN1, 4, 0x8, 8,
				                         pi_ht->btc_saved_lna1gains[0]);
				wlc_phy_table_read_htphy(pi, HTPHY_TBL_ID_GAIN3, 4, 0x8, 8,
				                         pi_ht->btc_saved_lna1gains[2]);
				/* lna2 tables */
				wlc_phy_table_read_htphy(pi, HTPHY_TBL_ID_GAINBITS1, 4, 0x10, 8,
				                         pi_ht->btc_saved_lna2gainbits[0]);
				wlc_phy_table_read_htphy(pi, HTPHY_TBL_ID_GAINBITS3, 4, 0x10, 8,
				                         pi_ht->btc_saved_lna2gainbits[2]);
				wlc_phy_table_read_htphy(pi, HTPHY_TBL_ID_GAIN1, 4, 0x10, 8,
				                         pi_ht->btc_saved_lna2gains[0]);
				wlc_phy_table_read_htphy(pi, HTPHY_TBL_ID_GAIN3, 4, 0x10, 8,
				                         pi_ht->btc_saved_lna2gains[2]);
				/* init gain */
				wlc_phy_table_read_htphy(pi, HTPHY_TBL_ID_RFSEQ, 1, 0x106, 16,
				                         &pi_ht->btc_saved_init_rfseq[0]);
				wlc_phy_table_read_htphy(pi, HTPHY_TBL_ID_RFSEQ, 1, 0x108, 16,
				                         &pi_ht->btc_saved_init_rfseq[2]);
				pi_ht->btc_saved_init_regs[0][0] =
				        phy_utils_read_phyreg(pi, HTPHY_Core0InitGainCodeA2059);
				pi_ht->btc_saved_init_regs[0][1] =
				        phy_utils_read_phyreg(pi, HTPHY_Core0InitGainCodeB2059);
				pi_ht->btc_saved_init_regs[2][0] =
				        phy_utils_read_phyreg(pi, HTPHY_Core2InitGainCodeA2059);
				pi_ht->btc_saved_init_regs[2][1] =
				        phy_utils_read_phyreg(pi, HTPHY_Core2InitGainCodeB2059);
				/* nbclip */
				pi_ht->btc_saved_nbclip[0] =
				        phy_utils_read_phyreg(pi, HTPHY_Core0nbClipThreshold);
				pi_ht->btc_saved_nbclip[2] =
				        phy_utils_read_phyreg(pi, HTPHY_Core2nbClipThreshold);
				/* cliphi */
				pi_ht->btc_saved_cliphi[0][0] =
				        phy_utils_read_phyreg(pi, HTPHY_Core0clipHiGainCodeA2059);
				pi_ht->btc_saved_cliphi[0][1] =
				        phy_utils_read_phyreg(pi, HTPHY_Core0clipHiGainCodeB2059);
				pi_ht->btc_saved_cliphi[2][0] =
				        phy_utils_read_phyreg(pi, HTPHY_Core2clipHiGainCodeA2059);
				pi_ht->btc_saved_cliphi[2][1] =
				        phy_utils_read_phyreg(pi, HTPHY_Core2clipHiGainCodeB2059);
				/* cliplo */
				pi_ht->btc_saved_cliplo[0][0] =
				        phy_utils_read_phyreg(pi, HTPHY_Core0cliploGainCodeA2059);
				pi_ht->btc_saved_cliplo[0][1] =
				        phy_utils_read_phyreg(pi, HTPHY_Core0cliploGainCodeB2059);
				pi_ht->btc_saved_cliplo[2][0] =
				        phy_utils_read_phyreg(pi, HTPHY_Core2cliploGainCodeA2059);
				pi_ht->btc_saved_cliplo[2][1] =
				        phy_utils_read_phyreg(pi, HTPHY_Core2cliploGainCodeB2059);

				/* limit lna1, lna2 on core0 */
				{
					int8 new_lna1gainbits[] = {0, 1, 2, 2};
					int8 new_lna2gainbits[] = {0, 1, 1, 1};
					for (idx = 0; idx < 4; idx++) {
						new_lna1gains[idx] = pi_ht->btc_saved_lna1gains[0]
						        [new_lna1gainbits[idx]];
						new_lna2gains[idx] = pi_ht->btc_saved_lna2gains[0]
						        [new_lna2gainbits[idx]];
					}
					wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS1, 4,
					                          0x8, 8, new_lna1gainbits);
					wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1, 4, 0x8,
					                          8, new_lna1gains);
					wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS1, 4,
					                          0x10, 8, new_lna2gainbits);
					wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1, 4, 0x10,
					                          8, new_lna2gains);
				}
				/* limit lna1, lna2 on core2 */
				{
					int8 new_lna1gainbits[] = {0, 1, 2, 2};
					int8 new_lna2gainbits[] = {0, 1, 1, 1};
					for (idx = 0; idx < 4; idx++) {
						new_lna1gains[idx] = pi_ht->btc_saved_lna1gains[2]
						        [new_lna1gainbits[idx]];
						new_lna2gains[idx] = pi_ht->btc_saved_lna2gains[2]
						        [new_lna2gainbits[idx]];
					}
					wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS3, 4,
					                          0x8, 8, new_lna1gainbits);
					wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3, 4, 0x8,
					                          8, new_lna1gains);
					wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS3, 4,
					                          0x10, 8, new_lna2gainbits);
					wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3, 4, 0x10,
					                          8, new_lna2gains);
				}
				/* init gain */
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_RFSEQ, 1, 0x106, 16,
				                          &newinitgain);
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_RFSEQ, 1, 0x108, 16,
				                          &newinitgain);
				phy_utils_write_phyreg(pi, HTPHY_Core0InitGainCodeA2059, 0x4c);
				phy_utils_write_phyreg(pi, HTPHY_Core2InitGainCodeA2059, 0x4c);
				phy_utils_write_phyreg(pi, HTPHY_Core0InitGainCodeB2059, 0x824);
				phy_utils_write_phyreg(pi, HTPHY_Core2InitGainCodeB2059, 0x824);
				/* nbclip */
				phy_utils_write_phyreg(pi, HTPHY_Core0nbClipThreshold, 0x51);
				phy_utils_write_phyreg(pi, HTPHY_Core2nbClipThreshold, 0x51);
				/* cliphi */
				phy_utils_write_phyreg(pi, HTPHY_Core0clipHiGainCodeA2059, 0x4c);
				phy_utils_write_phyreg(pi, HTPHY_Core2clipHiGainCodeA2059, 0x4c);
				phy_utils_write_phyreg(pi, HTPHY_Core0clipHiGainCodeB2059, 0x34);
				phy_utils_write_phyreg(pi, HTPHY_Core2clipHiGainCodeB2059, 0x34);
				/* cliplo */
				phy_utils_write_phyreg(pi, HTPHY_Core0cliploGainCodeA2059, 0x2c);
				phy_utils_write_phyreg(pi, HTPHY_Core2cliploGainCodeA2059, 0x2c);
				phy_utils_write_phyreg(pi, HTPHY_Core0cliploGainCodeB2059, 0x38);
				phy_utils_write_phyreg(pi, HTPHY_Core2cliploGainCodeB2059, 0x38);
			}
			pi_ht->btc_restage_rxgain = TRUE;
		} else {
			uint8 antswctrl_enab_elna2G = 0x2;
			wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_ANTSWCTRLLUT, 1, 2, 8,
			                          &antswctrl_enab_elna2G);
			wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_ANTSWCTRLLUT, 1, 10, 8,
			                          &antswctrl_enab_elna2G);
			wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_ANTSWCTRLLUT, 1, 34, 8,
			                          &antswctrl_enab_elna2G);
			wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_ANTSWCTRLLUT, 1, 42, 8,
			                          &antswctrl_enab_elna2G);
			/* XXX
			 * Restore original rxgain staging
			 */
			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_RFSEQ, 1, 0x106, 16,
				                          &pi_ht->btc_saved_init_rfseq[0]);
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_RFSEQ, 1, 0x108, 16,
				                          &pi_ht->btc_saved_init_rfseq[2]);
				phy_utils_write_phyreg(pi, HTPHY_Core0InitGainCodeA2059,
				              pi_ht->btc_saved_init_regs[0][0]);
				phy_utils_write_phyreg(pi, HTPHY_Core0InitGainCodeB2059,
				              pi_ht->btc_saved_init_regs[0][1]);
				phy_utils_write_phyreg(pi, HTPHY_Core2InitGainCodeA2059,
				              pi_ht->btc_saved_init_regs[2][0]);
				phy_utils_write_phyreg(pi, HTPHY_Core2InitGainCodeB2059,
				              pi_ht->btc_saved_init_regs[2][1]);
				phy_utils_write_phyreg(pi, HTPHY_Core0nbClipThreshold,
				              pi_ht->btc_saved_nbclip[0]);
				phy_utils_write_phyreg(pi, HTPHY_Core2nbClipThreshold,
				              pi_ht->btc_saved_nbclip[2]);
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1, 2, 0, 8,
				                          pi_ht->btc_saved_elnagain[0]);
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3, 2, 0, 8,
				                          pi_ht->btc_saved_elnagain[2]);

				if (pi->sh->interference_mode == NON_WLAN) {
					wlc_phy_bphynoise_update_lut_htphy(
						pi, HTPHY_bphy_desense_lut_eLNA,
						sizeof(HTPHY_bphy_desense_lut_eLNA)/
						sizeof(bphy_desense_info_t),
						HTPHY_BPHY_MIN_SENSITIVITY_ELNA,
						pi->interf->init_gain_table_stored);
				}
			} else {
				/* lna1 tables */
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS1, 4, 0x8, 8,
				                          pi_ht->btc_saved_lna1gainbits[0]);
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS3, 4, 0x8, 8,
				                          pi_ht->btc_saved_lna1gainbits[2]);
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1, 4, 0x8, 8,
				                          pi_ht->btc_saved_lna1gains[0]);
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3, 4, 0x8, 8,
				                          pi_ht->btc_saved_lna1gains[2]);
				/* lna2 tables */
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS1, 4, 0x10, 8,
				                          pi_ht->btc_saved_lna2gainbits[0]);
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINBITS3, 4, 0x10, 8,
				                          pi_ht->btc_saved_lna2gainbits[2]);
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN1, 4, 0x10, 8,
				                          pi_ht->btc_saved_lna2gains[0]);
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAIN3, 4, 0x10, 8,
				                          pi_ht->btc_saved_lna2gains[2]);
				/* init */
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_RFSEQ, 1, 0x106, 16,
				                          &pi_ht->btc_saved_init_rfseq[0]);
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_RFSEQ, 1, 0x108, 16,
				                          &pi_ht->btc_saved_init_rfseq[2]);
				phy_utils_write_phyreg(pi, HTPHY_Core0InitGainCodeA2059,
				              pi_ht->btc_saved_init_regs[0][0]);
				phy_utils_write_phyreg(pi, HTPHY_Core0InitGainCodeB2059,
				              pi_ht->btc_saved_init_regs[0][1]);
				phy_utils_write_phyreg(pi, HTPHY_Core2InitGainCodeA2059,
				              pi_ht->btc_saved_init_regs[2][0]);
				phy_utils_write_phyreg(pi, HTPHY_Core2InitGainCodeB2059,
				              pi_ht->btc_saved_init_regs[2][1]);
				/* nbclip */
				phy_utils_write_phyreg(pi, HTPHY_Core0nbClipThreshold,
				              pi_ht->btc_saved_nbclip[0]);
				phy_utils_write_phyreg(pi, HTPHY_Core2nbClipThreshold,
				              pi_ht->btc_saved_nbclip[2]);
				/* cliphi */
				phy_utils_write_phyreg(pi, HTPHY_Core0clipHiGainCodeA2059,
				              pi_ht->btc_saved_cliphi[0][0]);
				phy_utils_write_phyreg(pi, HTPHY_Core0clipHiGainCodeB2059,
				              pi_ht->btc_saved_cliphi[0][1]);
				phy_utils_write_phyreg(pi, HTPHY_Core2clipHiGainCodeA2059,
				              pi_ht->btc_saved_cliphi[2][0]);
				phy_utils_write_phyreg(pi, HTPHY_Core2clipHiGainCodeB2059,
				              pi_ht->btc_saved_cliphi[2][1]);
				/* cliplo */
				phy_utils_write_phyreg(pi, HTPHY_Core0cliploGainCodeA2059,
				              pi_ht->btc_saved_cliplo[0][0]);
				phy_utils_write_phyreg(pi, HTPHY_Core0cliploGainCodeB2059,
				              pi_ht->btc_saved_cliplo[0][1]);
				phy_utils_write_phyreg(pi, HTPHY_Core2cliploGainCodeA2059,
				              pi_ht->btc_saved_cliplo[2][0]);
				phy_utils_write_phyreg(pi, HTPHY_Core2cliploGainCodeB2059,
				              pi_ht->btc_saved_cliplo[2][1]);
			}
			pi_ht->btc_restage_rxgain = FALSE;
		}
	} else if (IS_X29D_BOARDTYPE(pi)) {
		if (enable) {
			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				uint8 lna1_limit_idx3 = 0x7f;
				uint8 lna2_limit_idx123[] = {0x7f, 0x7f, 0x7f};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINLIMIT, 1, 0xb,
				                          8, &lna1_limit_idx3);
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINLIMIT, 3, 0x11,
				                          8, &lna2_limit_idx123);
				pi_ht->btc_restage_rxgain = TRUE;
			}
		} else {
			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				/* XXX
				 * Un-limit OFDM pktgains for LNAs - restore defaults.
				 * HACK:
				 * Default values are hard-coded here rather than retrieved from
				 * saved values. This is done because ACI mitigation might also
				 * write to these table entries, potentially putting them in an
				 * unknown state when saving.
				 */
				uint8 lna1_idx3_default = 0xf;
				uint8 lna2_idx123_default[] = {0x0, 0x0, 0x3};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINLIMIT, 1, 0xb,
				                          8, &lna1_idx3_default);
				if (pi->aci_state & ACI_ACTIVE) {
					/* XXX
					 * If ACI mitigation is active, DO NOT restore lna2 idx3
					 * entry of GainLimit table, as ACI mitigation needs
					 * this entry to be 0x7f.
					 */
					wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINLIMIT,
					                          2, 0x11, 8, &lna2_idx123_default);
				} else {
					wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINLIMIT,
					                          3, 0x11, 8, &lna2_idx123_default);
				}
			}
			pi_ht->btc_restage_rxgain = FALSE;
		}
	} else if (IS_X33_BOARDTYPE(pi)) {
		if (enable) {
			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				uint8 lna1_limit_idx3 = 0x7f;
				uint8 lna2_limit_idx123[] = {0x7f, 0x7f, 0x7f};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINLIMIT,
				                          1, 0xb, 8, &lna1_limit_idx3);
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINLIMIT,
				                          3, 0x11, 8, &lna2_limit_idx123);
				pi_ht->btc_restage_rxgain = TRUE;
			}
		} else {
			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				/* XXX
				 * Un-limit OFDM pktgains for LNAs - restore defaults.
				 * HACK:
				 * Default values are hard-coded here rather than retrieved from
				 * saved values. This is done to avoid BT COEX algorithm picking
				 * incorrect defaults settings if
				 * future driver code changes (ACI mode from 0 to 1,2,3)
				 * access and modify saved
				 * values and if they are differnt from the defaults.
				 */
				uint8 lna1_idx3_default = 0xf;
				uint8 lna2_idx123_default[] = {0x0, 0x0, 0x3};
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINLIMIT,
				                          1, 0xb, 8, &lna1_idx3_default);
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINLIMIT,
				                          3, 0x11, 8, &lna2_idx123_default);
			}
			pi_ht->btc_restage_rxgain = FALSE;
		}
	}

	/* unsuspend mac */
	if (!suspend) {
		wlapi_enable_mac(pi->sh->physhim);
	}
}

/* Backoff per-core init gain in 3dB steps specified by backoff_val[core].
 * Currently, only biq1 index is reduced.
 */
static void
wlc_phy_backoff_initgain_htphy(phy_info_t *pi, const uint8 *backoff_val)
{
	uint16 initgain[PHY_CORE_MAX], biq1idx[PHY_CORE_MAX];
	uint8 core;
	wlc_phy_table_read_htphy(pi, HTPHY_TBL_ID_RFSEQ, 3, 0x106, 16,
	                         initgain);
	FOREACH_CORE(pi, core) {
		biq1idx[core] = (initgain[core] & 0xf000) >> 12;
		if (biq1idx[core] < backoff_val[core]) {
			PHY_ERROR(("%s: Backoff index %d > current biq1 index %d on core %d\n",
			           __FUNCTION__, backoff_val[core], biq1idx[core], core));
			ASSERT(0);
			return;
		}
		biq1idx[core] -= backoff_val[core];
		initgain[core] = (biq1idx[core] << 12) | (initgain[core] & 0xfff);
	}

	wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_RFSEQ, 3, 0x106, 16,
	                         initgain);
	MOD_PHYREG(pi, HTPHY_Core0InitGainCodeB2059, initBiQ1Index, biq1idx[0]);
	if (PHYCORENUM(pi->pubpi.phy_corenum) > 1)
		MOD_PHYREG(pi, HTPHY_Core1InitGainCodeB2059, InitBiQ1Index, biq1idx[1]);
	if (PHYCORENUM(pi->pubpi.phy_corenum) > 2)
		MOD_PHYREG(pi, HTPHY_Core2InitGainCodeB2059, InitBiQ1Index, biq1idx[2]);
}

/*
 * Define any new set of spurs below
 * Then go in wlc_phy_spurwar_htphy and
 * define or modify the appropriate conditinal
 * for it to be used
 */
typedef struct {
	uint16 spur_freq;
	int16 nvar[3];
} htphy_spurinfo_t;

static const htphy_spurinfo_t spurs2G_xtal[] = {
	{2420, {10, 0, 0}}, {2440, {10, 0, 0}}, {2460, {10, 0, 0}},
	{2480, {10, 0, 0}}
};

/* XXX LLR-deweighting for 5G 40MHz spurs (non-eLNA boards):
 * PR 87046, PR 100886, PR 104311, PR112795
 */
static const htphy_spurinfo_t spurs5G_40M[] = {
	{5200, {10, 0, 0}}, {5280, {17, 10, 7}}, {5600, {13, 10, 7}},
	{5640, {10, 0, 0}}, {5680, {10, 0, 0}}, {5760, {9, 0, 0}}
};

/* XXX LLR-deweighting for 5G 20MHz spurs (non-eLNA boards):
 * PR 104311
 */
static const htphy_spurinfo_t spurs5G_20M[] = {
	{5800, {10, 0, 0}}
};

typedef struct {
	uint16 fc;
	uint8 val[3];
} htphy_initbackoff_t;

/* XXX Initgain backoff for certain 5G 40MHz spur channels (non-eLNA boards):
 * PR 87046: Reduce core0 initgain by 6dB on 40MHz channels with fc=5270,5590,5755
 *           (adc clk harmonic spur)
 */
static const htphy_initbackoff_t initbackoff5G_40M[] = {
	{5270, {2, 0, 0}}, {5590, {2, 0, 0}}, {5755, {2, 0, 0}}
};

/*
 * Currently there is only one set
 * of spurs per band. If there are
 * two or more the following
 * number must change
 */
#define  MAX_SPURS_BAND   1

/* XXX
 * Spur workaround comprises one/both of the following for impacted cores:
 *      1) Deweighting LLRs for tones affected by spur (improves sensitivity)
 *      2) Backing off initgain (reduces PER floor)
 * This function MUST be called ONLY AFTER subband_cust to avoid
 * potential override of initgain by subband_cust.
 */
static void
wlc_phy_spurwar_htphy(phy_info_t *pi)
{
	uint16 fc, bw, n_tones;
	int16 idx;
	/* n_spur_pnt is defaulted to zero
	 * the value is changed if
	 * spurs de-weighting is used
	 */
	uint16 n_spur_pnt = 0;

	/* The array num_pnt [MAX_SPURS_BAND]
	 * carries the number of
	 * pointers for each set of spurs.
	 * Up to MAX_SPURS_BAND set of spurs can be used for
	 * each band  at the same time.
	 */
	uint16 num_pnt[MAX_SPURS_BAND];
	const htphy_spurinfo_t *spurs[MAX_SPURS_BAND];

	/* Noise Variance Table Modification Monitor */
	phy_info_htphy_t *pi_ht = (phy_info_htphy_t *)pi->u.pi_htphy;
	htphy_nshapetbl_mon_t* nshapetbl_mon =
	        &pi_ht->nshapetbl_mon;
	uint8* nshapetbl_start_addr =
	        nshapetbl_mon->start_addr;
	uint8* nshapetbl_mod_length =
	        nshapetbl_mon->mod_length;
	uint32 nshapetbl_wr_data[NTONES_BW40];
	uint8 mon_idx;

	/* XXX The following are
	 * initialized to avoid warnings
	 * in some builds
	 */
	for (idx = 0; idx < MAX_SPURS_BAND; idx++) {
		num_pnt[idx] = 0;
		spurs[idx] = spurs2G_xtal;
	}
	if (CHSPEC_CHANNEL(pi->radio_chanspec) > 14) {
		fc = CHAN5G_FREQ(CHSPEC_CHANNEL(pi->radio_chanspec));
	} else {
		fc = CHAN2G_FREQ(CHSPEC_CHANNEL(pi->radio_chanspec));
	}

	bw = (CHSPEC_IS20(pi->radio_chanspec) == 1)? 20 : 40;
	n_tones = (bw == 20)? NTONES_BW20 : NTONES_BW40;

	/* reset nvar shaping table
	 * PR90360: Clear only the elements that has been set previously
	 */
	for (mon_idx = 0; mon_idx < nshapetbl_mon->length; mon_idx++) {
		uint32 start_addr = (uint32) nshapetbl_start_addr[mon_idx];
		uint32 mod_length = (uint32) nshapetbl_mod_length[mon_idx];
		uint32 tbl_wr_data[2];
		/* Increase tbl_wr_data if this asserts */
		ASSERT(sizeof(tbl_wr_data)/sizeof(tbl_wr_data[0]) >= mod_length);
		bzero((uint32 *)tbl_wr_data, mod_length * sizeof(uint32));
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_NVNOISESHAPINGTBL,
		                          mod_length, start_addr, 32, &tbl_wr_data);
	}
	/* Invalidate the monitor upon reseting the nvar shaping table */
	nshapetbl_mon->length = 0;

	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		spurs[n_spur_pnt] = spurs2G_xtal;
		num_pnt[n_spur_pnt] = sizeof(spurs2G_xtal)/sizeof(htphy_spurinfo_t);
		n_spur_pnt += 1;
	} else {
		/* XXX 5G BAND Customizations
		 * Note: With extLNA, the spurs levels are low. This WAR is not required.
		 * Ref: http://hwnbu-twiki.broadcom.com/bin/view/Mwgroup/BCM4331B1LabNotebook56
		 */
		bool extLNA5G = ((pi->sh->boardflags & BFL_EXTLNA_5GHz) != 0);
		if (HTREV_GT(pi->pubpi.phy_rev, 0) && !extLNA5G) {
			bool is40MHz = CHSPEC_IS40(pi->radio_chanspec);
			if (is40MHz) {
				uint8 num_chan;

				spurs[n_spur_pnt] = spurs5G_40M;
				num_pnt[n_spur_pnt] = sizeof(spurs5G_40M)/sizeof(htphy_spurinfo_t);
				n_spur_pnt += 1;

				num_chan = sizeof(initbackoff5G_40M)/sizeof(htphy_initbackoff_t);
				for (idx = 0; idx < num_chan; idx++) {
					if (fc == initbackoff5G_40M[idx].fc) break;
				}
				if (idx < num_chan) {
					wlc_phy_backoff_initgain_htphy(pi,
					        (const uint8*)initbackoff5G_40M[idx].val);
				}
			} else {
				spurs[n_spur_pnt] = spurs5G_20M;
				num_pnt[n_spur_pnt] = sizeof(spurs5G_20M)/sizeof(htphy_spurinfo_t);
				n_spur_pnt += 1;
			}
		}
	}

	ASSERT(n_spur_pnt <= MAX_SPURS_BAND);
	for (idx = 0; idx < n_spur_pnt; idx++) {
		int16 spur_idx;
		for (spur_idx = 0; spur_idx < num_pnt[idx]; spur_idx++) {
			int16 f_delta = spurs[idx][spur_idx].spur_freq - fc;
			if ((f_delta > -bw/2) && (f_delta < bw/2)) {
				/*
				 * XXX Calculating tone_idx based on
				 * the f_delta
				 */
				uint32 nvarshaping;
				int16 tone_idx, core;
				/* current monitor index */
				mon_idx = nshapetbl_mon->length;

				tone_idx  =  (f_delta << 6)/20;
				PHY_INFORM(("SPUR: fc = %d, delta_spur=%d, tone_idx =%d \n",
				          fc, f_delta, tone_idx));
				tone_idx += (tone_idx > 0)? 0 : n_tones;
				nvarshaping = 0;
				FOREACH_CORE(pi, core) {
					int16 nvar_tmp  = spurs[idx][spur_idx].nvar[core];
					nvar_tmp += (nvar_tmp < 0)? 256 : 0;
					nvarshaping += ((nvar_tmp & 0xFF) << (8*core));
				}

				/*
				 * for frequencies that the spur
				 * lands on the center of subcarrier
				 * only one subcarrier is de-weighted
				 */
				nshapetbl_start_addr[mon_idx] = (uint8) tone_idx;
				nshapetbl_wr_data[mon_idx] = nvarshaping;
				nshapetbl_mon->length++;
				if ((ABS(f_delta) == 5) || (ABS(f_delta) == 10) ||
				    (ABS(f_delta) == 15)) {
					nshapetbl_mod_length[mon_idx] = 1;
				} else {
					nshapetbl_mod_length[mon_idx] = 2;
				}
			} /* fi ((f_delta >= -bw/2) && (f_delta <= bw/2) */
		} /* for (spur_idx = 0; spur_idx <= num_pnt[idx]; spur_idx++) */
	} /* for (idx = 0; idx < n_spur_pnt; idx++) */

	/* Write Data into Noise Shaping Table  */
	for (mon_idx = 0; mon_idx < nshapetbl_mon->length; mon_idx++) {
		uint32 start_addr = (uint32) nshapetbl_start_addr[mon_idx];
		uint32 mod_length = (uint32) nshapetbl_mod_length[mon_idx];
		uint32 wr_idx;
		uint32 tbl_wr_data[2];
		/* Increase tbl_wr_data if this asserts */
		ASSERT(sizeof(tbl_wr_data)/sizeof(tbl_wr_data[0]) >= mod_length);
		for (wr_idx = 0; wr_idx < mod_length; wr_idx++) {
			tbl_wr_data[wr_idx] = nshapetbl_wr_data[mon_idx];
		}
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_NVNOISESHAPINGTBL,
		                          mod_length, start_addr, 32, &tbl_wr_data);
	}
}

uint16
wlc_phy_classifier_htphy(phy_info_t *pi, uint16 mask, uint16 val)
{
	uint16 curr_ctl, new_ctl;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	/* Turn on/off classification (bphy, ofdm, and wait_ed), mask and
	 * val are bit fields, bit 0: bphy, bit 1: ofdm, bit 2: wait_ed;
	 * for types corresponding to bits set in mask, apply on/off state
	 * from bits set in val; if no bits set in mask, simply returns
	 * current on/off state.
	 */
	curr_ctl = phy_utils_read_phyreg(pi, HTPHY_ClassifierCtrl) &
		HTPHY_ClassifierCtrl_classifierSel_MASK;

	new_ctl = (curr_ctl & (~mask)) | (val & mask);

	MOD_PHYREG(pi, HTPHY_ClassifierCtrl, classifierSel, new_ctl);

	return new_ctl;
}

static void
wlc_phy_clip_det_htphy(phy_info_t *pi, uint8 write, uint16 *vals)
{
	uint16 core;

	/* Make clip detection difficult (impossible?) */
	FOREACH_CORE(pi, core) {
		if (write == 0) {
			vals[core] = phy_utils_read_phyreg(pi, HTPHY_Clip1Threshold(core));
		} else {
			phy_utils_write_phyreg(pi, HTPHY_Clip1Threshold(core), vals[core]);
		}
	}
}

void
wlc_phy_force_rfseq_htphy(phy_info_t *pi, uint8 cmd)
{
	uint16 trigger_mask, status_mask;
	uint16 orig_RfseqCoreActv;

	switch (cmd) {
	case HTPHY_RFSEQ_RX2TX:
		trigger_mask = HTPHY_RfseqTrigger_rx2tx_MASK;
		status_mask = HTPHY_RfseqStatus0_rx2tx_MASK;
		break;
	case HTPHY_RFSEQ_TX2RX:
		trigger_mask = HTPHY_RfseqTrigger_tx2rx_MASK;
		status_mask = HTPHY_RfseqStatus0_tx2rx_MASK;
		break;
	case HTPHY_RFSEQ_RESET2RX:
		trigger_mask = HTPHY_RfseqTrigger_reset2rx_MASK;
		status_mask = HTPHY_RfseqStatus0_reset2rx_MASK;
		break;
	case HTPHY_RFSEQ_UPDATEGAINH:
		trigger_mask = HTPHY_RfseqTrigger_updategainh_MASK;
		status_mask = HTPHY_RfseqStatus0_updategainh_MASK;
		break;
	case HTPHY_RFSEQ_UPDATEGAINL:
		trigger_mask = HTPHY_RfseqTrigger_updategainl_MASK;
		status_mask = HTPHY_RfseqStatus0_updategainl_MASK;
		break;
	case HTPHY_RFSEQ_UPDATEGAINU:
		trigger_mask = HTPHY_RfseqTrigger_updategainu_MASK;
		status_mask = HTPHY_RfseqStatus0_updategainu_MASK;
		break;
	default:
		PHY_ERROR(("wlc_phy_force_rfseq_htphy: unrecognized command.\n"));
		return;
	}

	orig_RfseqCoreActv = phy_utils_read_phyreg(pi, HTPHY_RfseqMode);
	phy_utils_or_phyreg(pi, HTPHY_RfseqMode,
	           (HTPHY_RfseqMode_CoreActv_override_MASK |
	            HTPHY_RfseqMode_Trigger_override_MASK));
	phy_utils_or_phyreg(pi, HTPHY_RfseqTrigger, trigger_mask);
	SPINWAIT((phy_utils_read_phyreg(pi, HTPHY_RfseqStatus0) & status_mask),
	         HTPHY_SPINWAIT_RFSEQ_FORCE);
	phy_utils_write_phyreg(pi, HTPHY_RfseqMode, orig_RfseqCoreActv);

	ASSERT((phy_utils_read_phyreg(pi, HTPHY_RfseqStatus0) & status_mask) == 0);
}

static void
wlc_phy_tssi_radio_setup_htphy(phy_info_t *pi, uint8 core_mask)
{
	uint8  core;

	FOREACH_ACTV_CORE(pi, core_mask, core) {

		/* activate IQ testpins and route through external tssi */
		phy_utils_mod_radioreg(pi, RADIO_2059_IQTEST_SEL_PU,  0x1, 0x1);
		phy_utils_write_radioreg(pi, RADIO_2059_TX_TX_SSI_MUX(core), 0x11);
	}
}

static void
wlc_phy_tssi_pdetrange_cust_htphy(phy_info_t *pi)
{
	uint8  IS5G = CHSPEC_IS5G(pi->radio_chanspec);
	uint8  pdetrange;
	uint16 core;
	uint16 *Av, *Vmid;
	uint16 pdetrange4_Av[] = {0, 0, 0};
	uint16 pdetrange6_Av_UNI1[] = {6, 6, 6};
	uint16 pdetrange4_Vmid2G[] = {0x96, 0x96, 0x96};
	uint16 pdetrange4_Vmid5G[] = {0x91, 0x91, 0x91};
	uint16 pdetrange5_Av2G[]   = {0, 2, 0};
	uint16 pdetrange5_Vmid2G[] = {0x96, 0x5a, 0x96};
	uint16 pdetrange5_Av5G[]   = {0, 5, 0};
	uint16 pdetrange5_Vmid5G[] = {0x91, 0x15c, 0x91};
	uint16 pdetrange6_Vmid5G_Not_UNII1[] = {0x72, 0x72, 0x72};
	uint16 pdetrange6_Vmid5G_UNII1[] = {0x15, 0x15, 0x15};
	uint16 pdetrange7_Vmid5G[] = {0x80, 0x80, 0x80};

	uint16 pdetrange9_Av5G_subband0[]   = {4,    5,    4};
	uint16 pdetrange9_Av5G_subband1[]   = {1,    2,    1};
	uint16 pdetrange9_Av5G_subband2[]   = {1,    2,    1};
	uint16 pdetrange9_Vmid5G_subband0[] = {96, 104, 96};
	uint16 pdetrange9_Vmid5G_subband1[] = {132, 128, 132};
	uint16 pdetrange9_Vmid5G_subband2[] = {132, 128, 132};
	uint16 pdetrange9_Av2G[]            = {3,    2,    3};
	uint16 pdetrange9_Vmid2G[]          = {0x68, 0x70, 0x68};

	uint16 pdetrange10_Av5G_subband0[]   = {4,    4,    4};
	uint16 pdetrange10_Av5G_subband1[]   = {1,    1,    1};
	uint16 pdetrange10_Av5G_subband2[]   = {1,    1,    1};
	uint16 pdetrange10_Vmid5G_subband0[] = {96, 96, 96};
	uint16 pdetrange10_Vmid5G_subband1[] = {132, 132, 132};
	uint16 pdetrange10_Vmid5G_subband2[] = {132, 132, 132};
	uint16 pdetrange10_Av2G[]            = {3,    3,    3};
	uint16 pdetrange10_Vmid2G[]          = {0x68, 0x68, 0x68};

	uint16 pdetrange11_Av5G_subband0[]   = {4,    4,    4};
	uint16 pdetrange11_Av5G_subband_rest[]   = {0,    0,    0};
	uint16 pdetrange11_Vmid5G_subband0[] = {96, 96, 96};
	uint16 pdetrange11_Vmid5G_subband_rest[] = {144, 144, 144};

	uint16 pdetrange12_Av_UNII1[] = {2, 2, 2};
	uint16 pdetrange12_Vmid5G_Not_UNII1[] = {110, 110, 110};
	uint16 pdetrange12_Vmid5G_UNII1[] = {70, 70, 70};

	uint16 pdetrange13_Av[] = {0, 0, 0};
	uint16 pdetrange13_Vmid[] = {0x32, 0x32, 0x32};

	uint16 fc;

	/* initializing to avoid compiler error */
	Av = pdetrange4_Av;
	Vmid = (IS5G)? pdetrange4_Vmid5G: pdetrange4_Vmid2G;

	if (CHSPEC_CHANNEL(pi->radio_chanspec) > 14) {
		fc = CHAN5G_FREQ(CHSPEC_CHANNEL(pi->radio_chanspec));
	} else {
		fc = CHAN2G_FREQ(CHSPEC_CHANNEL(pi->radio_chanspec));
	}

	pdetrange =
	        (IS5G) ? pi->fem5g->pdetrange :
	        pi->fem2g->pdetrange;

	switch (pdetrange) {
	case 3:
		if (IS5G) {
			ASSERT(0);
		}
		/* Fall back to case 4 to pick the pdetrange in 2G */
	case 4:
		/* Eiffel Dual Band FEM with external PA - SE5503A */
		Av = pdetrange4_Av;
		Vmid = (IS5G)? pdetrange4_Vmid5G: pdetrange4_Vmid2G;
		break;
	case 5:
		/* X28: Discrete FEM on Ant1, Eiffel FEM on Ant0/2 */
		Av = (IS5G)? pdetrange5_Av5G: pdetrange5_Av2G;
		Vmid = (IS5G)? pdetrange5_Vmid5G: pdetrange5_Vmid2G;
		break;
	case 6:
		/* MCH5: High power PA - 5G only -SE5004L */
		if (IS5G) {
			if ((fc <= 5240) && (fc >= 5180)) {
				Av = pdetrange6_Av_UNI1;
				Vmid = pdetrange6_Vmid5G_UNII1;
			} else {
				Av = pdetrange4_Av;
				Vmid = pdetrange6_Vmid5G_Not_UNII1;
			}
		} else {
			/* No g band for mch5. */
			ASSERT(0);
		}
		break;
	case 7:
		/* X12 5G: P414 and above */
		if (IS5G) {
			Av = pdetrange4_Av;
			Vmid = pdetrange7_Vmid5G;
		} else {
			/* No g band for this pdetrange */
			ASSERT(0);
		}
		break;
	case 8:
		/* Asus 5G: Se5003L PA */
		if (IS5G) {
			Av = pdetrange4_Av;
			Vmid = pdetrange6_Vmid5G_Not_UNII1;
		} else {
			/* No g band for this pdetrange */
			ASSERT(0);
		}
		break;

	case 9:
		/* X29: Discrete FEM on Ant1, FEM 5515 on Ant0/2 */
		if (IS5G) {
			if ((fc <= 5240) && (fc >= 5180)) {
				Av   = pdetrange9_Av5G_subband0;
				Vmid = pdetrange9_Vmid5G_subband0;
			} else if ((fc < 5500) && (fc >= 5260)) {
				Av   = pdetrange9_Av5G_subband1;
				Vmid = pdetrange9_Vmid5G_subband1;
			} else {
				Av   = pdetrange9_Av5G_subband2;
				Vmid = pdetrange9_Vmid5G_subband2;
			}
		} else {
			Av   = pdetrange9_Av2G;
			Vmid = pdetrange9_Vmid2G;
		}
		break;
	case 10:
		/* X19C: FEM 5515 on Ant0/1/2 */
		if (IS5G) {
			if ((fc <= 5240) && (fc >= 5180)) {
				Av   = pdetrange10_Av5G_subband0;
				Vmid = pdetrange10_Vmid5G_subband0;
			} else if ((fc < 5500) && (fc >= 5260)) {
				Av   = pdetrange10_Av5G_subband1;
				Vmid = pdetrange10_Vmid5G_subband1;
			} else {
				Av   = pdetrange10_Av5G_subband2;
				Vmid = pdetrange10_Vmid5G_subband2;
			}
		} else {
			Av   = pdetrange10_Av2G;
			Vmid = pdetrange10_Vmid2G;
		}
		break;
	case 11:
		/* SiGe Se5005L PAs on Ant0 Ant1 and Ant2 */
		if (IS5G) {
			if ((fc <= 5240) && (fc >= 5180)) {
				Av   = pdetrange11_Av5G_subband0;
				Vmid = pdetrange11_Vmid5G_subband0;
			} else {
				Av   = pdetrange11_Av5G_subband_rest;
				Vmid = pdetrange11_Vmid5G_subband_rest;
			}
		} else {
			/* NO 2G for this PA */
			ASSERT(0);
		}
		break;
	case 12:
		/* BCM94706LAPH5: High power PA - 5G only -SE5003L */
		if (IS5G) {
			if ((fc <= 5240) && (fc >= 5180)) {
				Av = pdetrange12_Av_UNII1;
				Vmid = pdetrange12_Vmid5G_UNII1;
			} else {
				Av = pdetrange4_Av;
				Vmid = pdetrange12_Vmid5G_Not_UNII1;
			}
		} else {
			/* No g band for BCM94706LAPH5. */
			ASSERT(0);
		}
		break;
	case 13:
		/* BCM94706NR2HMC WITH SE2604L PA */
		if (IS5G) {
			/* No a band */
			ASSERT(0);
		} else {
			Av = pdetrange13_Av;
			Vmid = pdetrange13_Vmid;
		}
		break;
	default:
		/* Use pdetrange = 4 */
		Av = pdetrange4_Av;
		Vmid = (IS5G)? pdetrange4_Vmid5G: pdetrange4_Vmid2G;
		PHY_ERROR(("invalid fem%s.pdetrange %d\n",
		           (IS5G) ? "5g" : "2g",
		           pdetrange));
		break;
	}

	/* Write To the AFECtrl Table */
	FOREACH_CORE(pi, core) {
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL, 1, 0x0b + 0x10*core, 16,
		                          &Vmid[core]);
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL, 1, 0x0f + 0x10*core, 16,
		                          &Av[core]);
	}
}

static void
wlc_phy_auxadc_sel_htphy(phy_info_t *pi, uint8 core_mask, uint8 signal_type)
{
	uint8  core;
	uint16 reset_mask = HTPHY_RfctrlAux_rssi_wb1a_sel_MASK |
	                    HTPHY_RfctrlAux_rssi_wb1g_sel_MASK |
	                    HTPHY_RfctrlAux_rssi_wb2_sel_MASK |
	                    HTPHY_RfctrlAux_rssi_nb_sel_MASK;

	/* signal_type is HTPHY_AUXADC_SEL_[W1,W2,NB,IQ,OFF] */
	FOREACH_ACTV_CORE(pi, core_mask, core) {

		if (signal_type == HTPHY_AUXADC_SEL_OFF) {
			MOD_PHYREGC(pi, HTPHY_AfectrlOverride, core, rssi_select_i, 0);
			MOD_PHYREGC(pi, HTPHY_AfectrlOverride, core, rssi_select_q, 0);

			MOD_PHYREGC(pi, HTPHY_RfctrlOverrideAux, core, rssi_ctrl, 0);

		} else if ((signal_type == HTPHY_AUXADC_SEL_W1) ||
		           (signal_type == HTPHY_AUXADC_SEL_W2) ||
		           (signal_type == HTPHY_AUXADC_SEL_NB)) {
			/* force AFE rssi mux sel to rssi inputs */
			MOD_PHYREGC(pi, HTPHY_Afectrl, core, rssi_select_i, 0);
			MOD_PHYREGC(pi, HTPHY_Afectrl, core, rssi_select_q, 0);
			MOD_PHYREGC(pi, HTPHY_AfectrlOverride, core, rssi_select_i, 1);
			MOD_PHYREGC(pi, HTPHY_AfectrlOverride, core, rssi_select_q, 1);

			phy_utils_mod_phyreg(pi, HTPHY_RfctrlAux(core), reset_mask, 0);
			if (signal_type == HTPHY_AUXADC_SEL_W1) {
				if (CHSPEC_IS5G(pi->radio_chanspec)) {
					MOD_PHYREGC(pi, HTPHY_RfctrlAux, core, rssi_wb1a_sel, 1);
				} else {
					MOD_PHYREGC(pi, HTPHY_RfctrlAux, core, rssi_wb1g_sel, 1);
				}
			} else if (signal_type == HTPHY_AUXADC_SEL_W2) {
				MOD_PHYREGC(pi, HTPHY_RfctrlAux, core, rssi_wb2_sel, 1);
			} else /* (signal_type == HTPHY_AUXADC_SEL_NB) */ {
				MOD_PHYREGC(pi, HTPHY_RfctrlAux, core, rssi_nb_sel, 1);
			}
			MOD_PHYREGC(pi, HTPHY_RfctrlOverrideAux, core, rssi_ctrl, 1);

		} else if (signal_type == HTPHY_AUXADC_SEL_TSSI) {
			/* set mux at the afe input level (tssi) */
			MOD_PHYREGC(pi, HTPHY_Afectrl, core, rssi_select_i, 3);
			MOD_PHYREGC(pi, HTPHY_Afectrl, core, rssi_select_q, 3);
			MOD_PHYREGC(pi, HTPHY_AfectrlOverride, core, rssi_select_i, 1);
			MOD_PHYREGC(pi, HTPHY_AfectrlOverride, core, rssi_select_q, 1);
			/* set up radio muxes (already done in init) */
			wlc_phy_tssi_radio_setup_htphy(pi, 1 << core);

		} else if ((signal_type == HTPHY_AUXADC_SEL_IQ) ||
			(signal_type == HTPHY_AUXADC_SEL_TEMPSENSE)) {
			/* force AFE rssi mux sel to iq/pwr_det/temp_sense */
			MOD_PHYREGC(pi, HTPHY_Afectrl, core, rssi_select_i, 2);
			MOD_PHYREGC(pi, HTPHY_Afectrl, core, rssi_select_q, 2);
			MOD_PHYREGC(pi, HTPHY_AfectrlOverride, core, rssi_select_i, 1);
			MOD_PHYREGC(pi, HTPHY_AfectrlOverride, core, rssi_select_q, 1);
		}
	}
}

static void
wlc_phy_poll_auxadc_htphy(phy_info_t *pi, uint8 signal_type, int32 *auxadc_buf,
	uint8 nsamps, uint8 mode)
{
	uint core;
	uint16 afectrl_save[PHY_CORE_MAX];
	uint16 afectrlOverride_save[PHY_CORE_MAX];
	uint16 rfctrlAux_save[PHY_CORE_MAX];
	uint16 rfctrlOverrideAux_save[PHY_CORE_MAX];
	uint8 samp = 0;
	uint16 rssi = 0;

	/* Save afectrl/rfctrl registers touched in auxadc_sel */
	FOREACH_CORE(pi, core) {
		afectrl_save[core] = phy_utils_read_phyreg(pi, HTPHY_Afectrl(core));
		afectrlOverride_save[core] = phy_utils_read_phyreg(pi, HTPHY_AfectrlOverride(core));
		rfctrlAux_save[core] = phy_utils_read_phyreg(pi, HTPHY_RfctrlAux(core));
		rfctrlOverrideAux_save[core] =
		        phy_utils_read_phyreg(pi, HTPHY_RfctrlOverrideAux(core));
	}

	wlc_phy_auxadc_sel_htphy(pi, 7, signal_type);

	FOREACH_CORE(pi, core) {
		auxadc_buf[2*core] = 0;
		auxadc_buf[2*core+1] = 0;
	}

	/* for tempsense, only core0 need to be polled */
	/* do not poll other cores to reduce MAC halt time */
	if (signal_type == HTPHY_AUXADC_SEL_TEMPSENSE) {
		for (samp = 0; samp < nsamps; samp++) {
			core = 0;
			rssi = phy_utils_read_phyreg(pi, HTPHY_RSSIVal(core));
			/* sign extension 6 to 8 bit */
			auxadc_buf[2*core] += ((int8)((rssi & 0x3f) << 2)) >> 2;
			auxadc_buf[2*core+1] += ((int8)(((rssi >> 8) & 0x3f) << 2)) >> 2;
		}
	}
	else {
		for (samp = 0; samp < nsamps; samp++) {
			FOREACH_CORE(pi, core) {
				rssi = phy_utils_read_phyreg(pi, HTPHY_RSSIVal(core));
				/* sign extension 6 to 8 bit */
				auxadc_buf[2*core] += ((int8)((rssi & 0x3f) << 2)) >> 2;
				auxadc_buf[2*core+1] += ((int8)(((rssi >> 8) & 0x3f) << 2)) >> 2;
			}
		}
	}

	/* Restore the saved registers */
	FOREACH_CORE(pi, core) {
		if (mode == 1) {
			auxadc_buf[2*core] = auxadc_buf[2*core]/nsamps;
			auxadc_buf[2*core+1] = auxadc_buf[2*core+1]/nsamps;
		}
		phy_utils_write_phyreg(pi, HTPHY_Afectrl(core), afectrl_save[core]);
		phy_utils_write_phyreg(pi, HTPHY_AfectrlOverride(core), afectrlOverride_save[core]);
		phy_utils_write_phyreg(pi, HTPHY_RfctrlAux(core), rfctrlAux_save[core]);
		phy_utils_write_phyreg(pi, HTPHY_RfctrlOverrideAux(core),
		                       rfctrlOverrideAux_save[core]);
	}
}

int16
wlc_phy_tempsense_htphy(phy_info_t *pi)
{
	int32 radio_temp;
	int32 vref1[2*PHY_CORE_MAX], vref2[2*PHY_CORE_MAX];
	int32 vptat1[2*PHY_CORE_MAX], vptat2[2*PHY_CORE_MAX];
	int32 t_offset, t_slope, t_scale;
	uint16 syn_tempprocsense_save;
	uint16 AfectrlCore0_save, AfectrlOverride0_save, RSSIMultCoef0QPowerDet_save;
	int16 offset = (int16) pi->phy_tempsense_offset;
	uint16 auxADC_Vmid, auxADC_Av, auxADC_Vmid_save, auxADC_Av_save;
	uint16 auxADC_ctrlL, auxADC_ctrlH, auxADC_ctrlL_save, auxADC_ctrlH_save;
	bool suspend;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	/* XXX If mac is suspended, leave it suspended and don't touch the state of the MAC
	 * If not, suspend at the beginning of tempsense and resume it at the end.
	 * Suspending is required
	 * - as we are reading via muxes that are pin-contolled during normal RX & TX.
	 */

	suspend = !(R_REG(pi->sh->osh, &pi->regs->maccontrol) & MCTL_EN_MAC);
	if (!suspend) {
		/* suspend mac */
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
	}

	/* XXX Save
	 * a) radio registers
	 * b) current Aux ADC Settings
	 * c) phy register
	 */
	syn_tempprocsense_save = phy_utils_read_radioreg(pi, RADIO_2059_TEMPSENSE_CONFIG);
	wlc_phy_table_read_htphy(pi, HTPHY_TBL_ID_AFECTRL, 1, 0xa, 16,
	                         &auxADC_Vmid_save);
	wlc_phy_table_read_htphy(pi, HTPHY_TBL_ID_AFECTRL, 1, 0xe, 16,
	                         &auxADC_Av_save);
	wlc_phy_table_read_htphy(pi, HTPHY_TBL_ID_AFECTRL, 1, 0x2, 16,
	                         &auxADC_ctrlL_save);
	wlc_phy_table_read_htphy(pi, HTPHY_TBL_ID_AFECTRL, 1, 0x3, 16,
	                         &auxADC_ctrlH_save);
	AfectrlCore0_save = phy_utils_read_phyreg(pi, HTPHY_AfectrlCore0);
	AfectrlOverride0_save = phy_utils_read_phyreg(pi, HTPHY_AfectrlOverride0);
	RSSIMultCoef0QPowerDet_save = phy_utils_read_phyreg(pi, HTPHY_RSSIMultCoef0QPowerDet);

	/* XXX Set the Vmid and Av for Core0
	 * FIXME: 4331 can mux between Core0 and Core2. Default at Core0.
	 *        Set registers to ensure that it's at Core0
	 */
	auxADC_Vmid = 0xce;
	auxADC_Av   = 0x3;
	wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL, 1, 0xa, 16,
	                         &auxADC_Vmid);
	wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL, 1, 0xe, 16,
	                         &auxADC_Av);

	auxADC_ctrlL = 0x0;
	auxADC_ctrlH = 0x20;
	wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL, 1, 0x2, 16,
	                         &auxADC_ctrlL);
	wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL, 1, 0x3, 16,
	                         &auxADC_ctrlH);

	phy_utils_write_phyreg(pi, HTPHY_RSSIMultCoef0QPowerDet, 0);

	MOD_PHYREG(pi, HTPHY_AfectrlCore0, bg_pd, 0);
	MOD_PHYREG(pi, HTPHY_AfectrlOverride0, bg_pd, 1);
	MOD_PHYREG(pi, HTPHY_AfectrlCore0, rssi_pd, 0);
	MOD_PHYREG(pi, HTPHY_AfectrlOverride0, rssi_pd, 1);

	/* XXX Config and make first Vref measurement
	 * Then toggle config bit and make a 2nd Vref measurement
	 */
	phy_utils_write_radioreg(pi, RADIO_2059_TEMPSENSE_CONFIG, 0x01);
	OSL_DELAY(5);
	wlc_phy_poll_auxadc_htphy(pi, HTPHY_AUXADC_SEL_TEMPSENSE, vref1, 8, 0);
	phy_utils_write_radioreg(pi, RADIO_2059_TEMPSENSE_CONFIG, 0x03);
	OSL_DELAY(5);
	wlc_phy_poll_auxadc_htphy(pi, HTPHY_AUXADC_SEL_TEMPSENSE, vref2, 8, 0);

	/* XXX Config temp sense readout; tempsense pu, clear flip (swap) bit
	 * Do the first temperature measurement
	 * Then toggle the config bit and make a second temperature measurement
	 */
	phy_utils_write_radioreg(pi, RADIO_2059_TEMPSENSE_CONFIG, 0x05);
	OSL_DELAY(5);
	wlc_phy_poll_auxadc_htphy(pi, HTPHY_AUXADC_SEL_TEMPSENSE, vptat1, 8, 0);
	phy_utils_write_radioreg(pi, RADIO_2059_TEMPSENSE_CONFIG, 0x07);
	OSL_DELAY(5);
	wlc_phy_poll_auxadc_htphy(pi, HTPHY_AUXADC_SEL_TEMPSENSE, vptat2, 8, 0);

	/* XXX Average readings w/o scaling, then use line-fit
	 * Convert ADC values to deg C
	 * REF: http://hwnbu-twiki.broadcom.com/bin/view/Mwgroup/BCM4331B0LabNotebook20
	 */
	t_slope = 95538;
	t_offset = 2234337;
	t_scale = 65536;
	/* the vptat and vref now holds the sum of 8 times of measurement result */
	radio_temp = ((t_slope*(vptat1[1] + vptat2[1] - vref1[1] - vref2[1]) + t_offset*8)
			/(4*t_scale)+1)/2;

	PHY_THERMAL(("Tempsense\n\tAuxADC0 Av,Vmid = 0x%x,0x%x\n",
	             auxADC_Av, auxADC_Vmid));
	PHY_THERMAL(("\tVref1,Vref2,Vptat1,Vptat2 =%d,%d,%d,%d\n",
	             vref1[1], vref2[1], vptat1[1], vptat2[1]));
	PHY_THERMAL(("\t^C Formula: (%d * (Vptat1+Vptat2-Vref1-Vref2) + %d)/%d\n",
	             t_slope, t_offset, t_scale));
	PHY_THERMAL(("\t^C = %d, applied offset = %d\n",
	             radio_temp, offset));

	/* XXX Restore
	 * a) radio registers
	 * b) current Aux ADC Settings
	 * c) phy registers
	 */
	phy_utils_write_radioreg(pi, RADIO_2059_TEMPSENSE_CONFIG, syn_tempprocsense_save);
	wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL, 1, 0xa, 16,
	                         &auxADC_Vmid_save);
	wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL, 1, 0xe, 16,
	                         &auxADC_Av_save);
	wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL, 1, 0x2, 16,
	                         &auxADC_ctrlL_save);
	wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL, 1, 0x3, 16,
	                         &auxADC_ctrlH_save);
	phy_utils_write_phyreg(pi, HTPHY_AfectrlOverride0, AfectrlOverride0_save);
	phy_utils_write_phyreg(pi, HTPHY_AfectrlCore0, AfectrlCore0_save);
	phy_utils_write_phyreg(pi, HTPHY_RSSIMultCoef0QPowerDet, RSSIMultCoef0QPowerDet_save);

	if (!suspend) {
		wlapi_enable_mac(pi->sh->physhim);
	}

	/* Store temperature and return value */
	pi->u.pi_htphy->current_temperature = (int16) radio_temp + offset;
	return ((int16) radio_temp + offset);
}

static void
wlc_phy_get_tx_bbmult(phy_info_t *pi, uint16 *bb_mult, uint16 core)
{
	uint16 tbl_ofdm_offset[] = { 99, 103, 107, 111};

	wlc_phy_table_read_htphy(pi, HTPHY_TBL_ID_IQLOCAL, 1,
	                         tbl_ofdm_offset[core], 16,
	                         bb_mult);
}

static void
wlc_phy_set_tx_bbmult(phy_info_t *pi, uint16 *bb_mult, uint16 core)
{
	uint16 tbl_ofdm_offset[] = { 99, 103, 107, 111};
	uint16 tbl_bphy_offset[] = {115, 119, 123, 127};

	wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_IQLOCAL, 1,
	                          tbl_ofdm_offset[core], 16,
	                          bb_mult);
	wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_IQLOCAL, 1,
	                          tbl_bphy_offset[core], 16,
	                          bb_mult);
}

static uint16
wlc_phy_gen_load_samples_htphy(phy_info_t *pi, uint32 f_kHz, uint16 max_val, uint8 dac_test_mode)
{
	uint8 phy_bw, is_phybw40;
	uint16 num_samps, t, bbc;
	math_fixed theta = 0, rot = 0;
	uint32 tbl_len;
	math_cint32* tone_buf = NULL;

	/* check phy_bw */
	is_phybw40 = CHSPEC_IS40(pi->radio_chanspec);
	phy_bw     = (is_phybw40 == 1)? 40 : 20;
	tbl_len    = (phy_bw << 3);

	if (dac_test_mode == 1) {
		bbc = phy_utils_read_phyreg(pi, HTPHY_BBConfig);
		if (bbc & HTPHY_BBConfig_resample_clk160_MASK) {
			phy_bw = ((bbc >> HTPHY_BBConfig_resamplerFreq_SHIFT) & 1) ? 84 : 82;
		} else {
			phy_bw = 80;
		}
		phy_bw = (is_phybw40 == 1) ? (phy_bw << 1) : phy_bw;
		/* use smaller num_samps to prevent overflow the buffer length */
		tbl_len = (phy_bw << 1);
	}

	/* allocate buffer */
	if ((tone_buf = MALLOC(pi->sh->osh, sizeof(math_cint32) * tbl_len)) == NULL) {
		PHY_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n", pi->sh->unit,
		          __FUNCTION__, MALLOCED(pi->sh->osh)));
		return 0;
	}

	/* set up params to generate tone */
	num_samps  = (uint16)tbl_len;
	rot = FIXED((f_kHz * 36)/phy_bw) / 100; /* 2*pi*f/bw/1000  Note: f in KHz */
	theta = 0;			/* start angle 0 */

	/* tone freq = f_c MHz ; phy_bw = phy_bw MHz ; # samples = phy_bw (1us) ; max_val = 151 */
	for (t = 0; t < num_samps; t++) {
		/* compute phasor */
		phy_utils_cordic(theta, &tone_buf[t]);
		/* update rotation angle */
		theta += rot;
		/* produce sample values for play buffer */
		tone_buf[t].q = (int32)FLOAT(tone_buf[t].q * max_val);
		tone_buf[t].i = (int32)FLOAT(tone_buf[t].i * max_val);
	}

	/* load sample table */
	wlc_phy_loadsampletable_htphy(pi, tone_buf, num_samps);

	if (tone_buf != NULL)
		MFREE(pi->sh->osh, tone_buf, sizeof(math_cint32) * tbl_len);

	return num_samps;
}

int
wlc_phy_tx_tone_htphy(phy_info_t *pi, uint32 f_kHz, uint16 max_val, uint8 iqmode,
                      uint8 dac_test_mode, bool modify_bbmult)
{
	uint8 core;
	uint16 num_samps;
	uint16 bb_mult;
	uint16 loops = 0xffff;
	uint16 wait = 0;
	phy_info_htphy_t *pi_ht = (phy_info_htphy_t *)pi->u.pi_htphy;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	if (max_val == 0) {
		/* XXX PR90390: For a zero ampltitude signal, bypass loading samples
		 * and set bbmult = 0
		 */
		num_samps = 1;
	} else if ((num_samps
	            = wlc_phy_gen_load_samples_htphy(pi, f_kHz, max_val, dac_test_mode)) == 0) {
		return BCME_ERROR;
	}

	if (pi_ht->bb_mult_save_valid == 0) {
		FOREACH_CORE(pi, core) {
			wlc_phy_get_tx_bbmult(pi, &pi_ht->bb_mult_save[core], core);
		}
		pi_ht->bb_mult_save_valid = 1;
	}

	/* XXX  max_val = 0, set bbmult = 0
	 * elseif modify_bbmult = 1,
	 * set samp_play -> DAC_out loss to 0dB by setting bb_mult (2.6 format) to
	 * 100/64 for bw = 20MHz
	 *  71/64 for bw = 40Mhz
	 */
	if (max_val == 0 || modify_bbmult) {
		if (max_val == 0) {
			bb_mult = 0;
		} else {
			bb_mult = (CHSPEC_IS20(pi->radio_chanspec)? 100 : 71);
		}
		FOREACH_CORE(pi, core) {
			wlc_phy_set_tx_bbmult(pi, &bb_mult, core);
		}
	}

	wlc_phy_runsamples_htphy(pi, num_samps, loops, wait, iqmode, dac_test_mode);

	return BCME_OK;
}

static void
wlc_phy_loadsampletable_htphy(phy_info_t *pi, math_cint32 *tone_buf, uint16 num_samps)
{
	uint16 t;
	uint32* data_buf = NULL;

	/* allocate buffer */
	if ((data_buf = (uint32 *)MALLOC(pi->sh->osh, sizeof(uint32) * num_samps)) == NULL) {
		PHY_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n", pi->sh->unit,
		           __FUNCTION__, MALLOCED(pi->sh->osh)));
		return;
	}

	/* load samples into sample play buffer */
	for (t = 0; t < num_samps; t++) {
		data_buf[t] = ((((unsigned int)tone_buf[t].i) & 0x3ff) << 10) |
		               (((unsigned int)tone_buf[t].q) & 0x3ff);
	}
	wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_SAMPLEPLAY, num_samps, 0, 32, data_buf);

	if (data_buf != NULL)
		MFREE(pi->sh->osh, data_buf, sizeof(uint32) * num_samps);
}

static void
wlc_phy_init_lpf_sampleplay(phy_info_t *pi)
{
	/* copy BW20/40 entry from rx2tx_lpf_rc_lut to sampleplay */
	uint16 rfseq_splay_offsets[]     = {0x14A, 0x15A, 0x16A};
	uint16 rfseq_lpf_rc_offsets_40[] = {0x149, 0x159, 0x169};
	uint16 rfseq_lpf_rc_offsets_20[] = {0x144, 0x154, 0x164};
	uint16 *rfseq_lpf_rc_offsets = NULL;
	uint16 core, val;

	if (CHSPEC_IS40(pi->radio_chanspec)) {
		rfseq_lpf_rc_offsets = rfseq_lpf_rc_offsets_40;
	} else {
		rfseq_lpf_rc_offsets = rfseq_lpf_rc_offsets_20;
	}
	FOREACH_CORE(pi, core) {
		wlc_phy_table_read_htphy(pi, HTPHY_TBL_ID_RFSEQ,
		                         1, rfseq_lpf_rc_offsets[core], 16, &val);
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_RFSEQ,
		                          1, rfseq_splay_offsets[core], 16, &val);
	}
}

static void
wlc_phy_runsamples_htphy(phy_info_t *pi, uint16 num_samps, uint16 loops, uint16 wait, uint8 iqmode,
                         uint8 dac_test_mode)
{
	uint8  sample_cmd;
	uint16 orig_RfseqCoreActv;

	wlc_phy_stay_in_carriersearch_htphy(pi, TRUE);

	phy_utils_write_phyreg(pi, HTPHY_sampleDepthCount, num_samps-1);

	if (loops != 0xffff) {
		phy_utils_write_phyreg(pi, HTPHY_sampleLoopCount, loops - 1);
	} else {
		phy_utils_write_phyreg(pi, HTPHY_sampleLoopCount, loops);
	}
	phy_utils_write_phyreg(pi, HTPHY_sampleInitWaitCount, wait);

	orig_RfseqCoreActv = phy_utils_read_phyreg(pi, HTPHY_RfseqMode);
	phy_utils_or_phyreg(pi, HTPHY_RfseqMode, HTPHY_RfseqMode_CoreActv_override_MASK);
	phy_utils_and_phyreg(pi, HTPHY_sampleCmd, ~HTPHY_sampleCmd_DacTestMode_MASK);
	phy_utils_and_phyreg(pi, HTPHY_sampleCmd, ~HTPHY_sampleCmd_start_MASK);
	phy_utils_and_phyreg(pi, HTPHY_iqloCalCmdGctl, 0x3FFF);
	if (iqmode) {
		phy_utils_or_phyreg(pi, HTPHY_iqloCalCmdGctl, 0x8000);
	} else {
		sample_cmd = HTPHY_sampleCmd_start_MASK;
		sample_cmd |= (dac_test_mode == 1 ? HTPHY_sampleCmd_DacTestMode_MASK : 0);
		phy_utils_or_phyreg(pi, HTPHY_sampleCmd, sample_cmd);
	}

	SPINWAIT(((phy_utils_read_phyreg(pi, HTPHY_RfseqStatus0) & 0x1) == 1),
	         HTPHY_SPINWAIT_RUNSAMPLE);

	phy_utils_write_phyreg(pi, HTPHY_RfseqMode, orig_RfseqCoreActv);

	wlc_phy_stay_in_carriersearch_htphy(pi, FALSE);
}

void
wlc_phy_stopplayback_htphy(phy_info_t *pi)
{
	phy_info_htphy_t *pi_ht = (phy_info_htphy_t *)pi->u.pi_htphy;
	uint16 playback_status;
	uint8 core;

	/* check status register */
	playback_status = phy_utils_read_phyreg(pi, HTPHY_sampleStatus);
	if (playback_status & 0x1) {
		phy_utils_or_phyreg(pi, HTPHY_sampleCmd, HTPHY_sampleCmd_stop_MASK);
	} else if (playback_status & 0x2) {
		phy_utils_and_phyreg(pi, HTPHY_iqloCalCmdGctl,
		            (uint16)~HTPHY_iqloCalCmdGctl_iqlo_cal_en_MASK);
	} else {
		PHY_CAL(("wlc_phy_stopplayback_htphy: already disabled\n"));
	}
	/* disable the dac_test mode */
	phy_utils_and_phyreg(pi, HTPHY_sampleCmd, ~HTPHY_sampleCmd_DacTestMode_MASK);

	/* if bb_mult_save does exist, restore bb_mult and undef bb_mult_save */
	if (pi_ht->bb_mult_save_valid != 0) {
		FOREACH_CORE(pi, core) {
			wlc_phy_set_tx_bbmult(pi, &pi_ht->bb_mult_save[core], core);
		}
		pi_ht->bb_mult_save_valid = 0;
	}

	wlc_phy_resetcca_htphy(pi);
}

static void
wlc_phy_txcal_radio_setup_htphy(phy_info_t *pi)
{
	phy_info_htphy_t *pi_ht = (phy_info_htphy_t *)pi->u.pi_htphy;
	htphy_txcal_radioregs_t *porig = &(pi_ht->htphy_txcal_radioregs_orig);
	uint16 core;

	/* SETUP: set 2059 into iq/lo cal state while saving off orig state */
	FOREACH_CORE(pi, core) {
		/* save off orig */
		porig->RF_TX_tx_ssi_master[core] =
			phy_utils_read_radioreg(pi, RADIO_2059_TX_TX_SSI_MASTER(core));
		porig->RF_TX_tx_ssi_mux[core] =
			phy_utils_read_radioreg(pi, RADIO_2059_TX_TX_SSI_MUX(core));
		porig->RF_TX_tssia[core] =
			phy_utils_read_radioreg(pi, RADIO_2059_TX_TSSIA(core));
		porig->RF_TX_tssig[core] =
			phy_utils_read_radioreg(pi, RADIO_2059_TX_TSSIG(core));
		porig->RF_TX_iqcal_vcm_hg[core] =
			phy_utils_read_radioreg(pi, RADIO_2059_TX_IQCAL_VCM_HG(core));
		porig->RF_TX_iqcal_idac[core] =
			phy_utils_read_radioreg(pi, RADIO_2059_TX_IQCAL_IDAC(core));
		porig->RF_TX_tssi_misc1[core] =
			phy_utils_read_radioreg(pi, RADIO_2059_TX_TSSI_MISC1(core));
		porig->RF_TX_tssi_vcm[core] =
			phy_utils_read_radioreg(pi, RADIO_2059_TX_TSSI_VCM(core));

		/* now write desired values */
		phy_utils_write_radioreg(pi, RADIO_2059_TX_IQCAL_VCM_HG(core), 0x43);
		phy_utils_write_radioreg(pi, RADIO_2059_TX_IQCAL_IDAC(core), 0x55);
		phy_utils_write_radioreg(pi, RADIO_2059_TX_TSSI_VCM(core), 0x00);
		phy_utils_write_radioreg(pi, RADIO_2059_TX_TSSI_MISC1(core), 0x00);

		if (CHSPEC_IS5G(pi->radio_chanspec)) {
			phy_utils_write_radioreg(pi, RADIO_2059_TX_TX_SSI_MASTER(core), 0x0a);
			phy_utils_write_radioreg(pi, RADIO_2059_TX_TSSIG(core), 0x00);
			phy_utils_write_radioreg(pi, RADIO_2059_TX_TX_SSI_MUX(core), 0x4);
			phy_utils_write_radioreg(pi, RADIO_2059_TX_TSSIA(core), 0x31); /* pad */
			/* phy_utils_write_radioreg(pi, RADIO_2059_TX_TSSIA, 0x21); */ /* ipa */
		} else {
			phy_utils_write_radioreg(pi, RADIO_2059_TX_TX_SSI_MASTER(core), 0x06);
			phy_utils_write_radioreg(pi, RADIO_2059_TX_TSSIA(core), 0x00);
			phy_utils_write_radioreg(pi, RADIO_2059_TX_TX_SSI_MUX(core), 0x06);
			 /* pad tapoff */
			phy_utils_write_radioreg(pi, RADIO_2059_TX_TSSIG(core), 0x31);
			/* phy_utils_write_radioreg(pi, RADIO_2059_TX_TSSIG, 0x21); */ /* ipa */
		}
	} /* for core */
}

static void
wlc_phy_txcal_radio_cleanup_htphy(phy_info_t *pi)
{
	phy_info_htphy_t *pi_ht = (phy_info_htphy_t *)pi->u.pi_htphy;
	htphy_txcal_radioregs_t *porig = &(pi_ht->htphy_txcal_radioregs_orig);
	uint16 core;

	/* CLEANUP: restore reg values */
	FOREACH_CORE(pi, core) {
		phy_utils_write_radioreg(pi, RADIO_2059_TX_TX_SSI_MASTER(core),
		                porig->RF_TX_tx_ssi_master[core]);
		phy_utils_write_radioreg(pi, RADIO_2059_TX_TX_SSI_MUX(core),
		                porig->RF_TX_tx_ssi_mux[core]);
		phy_utils_write_radioreg(pi, RADIO_2059_TX_TSSIA(core),
		                porig->RF_TX_tssia[core]);
		phy_utils_write_radioreg(pi, RADIO_2059_TX_TSSIG(core),
		                porig->RF_TX_tssig[core]);
		phy_utils_write_radioreg(pi, RADIO_2059_TX_IQCAL_VCM_HG(core),
		                porig->RF_TX_iqcal_vcm_hg[core]);
		phy_utils_write_radioreg(pi, RADIO_2059_TX_IQCAL_IDAC(core),
		                porig->RF_TX_iqcal_idac[core]);
		phy_utils_write_radioreg(pi, RADIO_2059_TX_TSSI_MISC1(core),
		                porig->RF_TX_tssi_misc1[core]);
		phy_utils_write_radioreg(pi, RADIO_2059_TX_TSSI_VCM(core),
		                porig->RF_TX_tssi_vcm[core]);
	} /* for core */
}

static void
wlc_phy_txcal_phy_setup_htphy(phy_info_t *pi)
{
	phy_info_htphy_t *pi_ht = (phy_info_htphy_t *)pi->u.pi_htphy;
	htphy_txcal_phyregs_t *porig = &(pi_ht->ht_txcal_phyregs_orig);
	uint16 val;
	uint8  core;

	/*  SETUP: save off orig reg values and configure for cal  */
	FOREACH_CORE(pi, core) {

		/* AUX-ADC ("rssi") selection (could use aux_adc_sel function instead) */
		porig->Afectrl[core] = phy_utils_read_phyreg(pi, HTPHY_Afectrl(core));
		porig->AfectrlOverride[core] =
		        phy_utils_read_phyreg(pi, HTPHY_AfectrlOverride(core));
		MOD_PHYREGC(pi, HTPHY_Afectrl,         core, rssi_select_i, 2);
		MOD_PHYREGC(pi, HTPHY_Afectrl,         core, rssi_select_q, 2);
		MOD_PHYREGC(pi, HTPHY_AfectrlOverride, core, rssi_select_i, 1);
		MOD_PHYREGC(pi, HTPHY_AfectrlOverride, core, rssi_select_q, 1);

		/* Aux-ADC Offset-binary mode (bypass conversion to 2's comp) */
		wlc_phy_table_read_htphy(pi, HTPHY_TBL_ID_AFECTRL, 1, core*0x10 +3, 16,
		                         &porig->Afectrl_AuxADCmode[core]);
		val = 0;   /* 0 means "no conversion", i.e., use offset binary as is */
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL, 1, core*0x10 +3, 16, &val);

		/* Power Down External PA (simply always do 2G & 5G), and set T/R to R */
		porig->RfctrlIntc[core] = phy_utils_read_phyreg(pi, HTPHY_RfctrlIntc(core));
		MOD_PHYREGC(pi, HTPHY_RfctrlIntc, core, ext_2g_papu, 0);
		MOD_PHYREGC(pi, HTPHY_RfctrlIntc, core, ext_5g_papu, 0);
		if ((CHSPEC_IS2G(pi->radio_chanspec) && ((pi->fem2g->antswctrllut == 3) ||
		                                         (pi->fem2g->antswctrllut == 4))) ||
		    (CHSPEC_IS5G(pi->radio_chanspec) && ((pi->fem5g->antswctrllut == 3) ||
		                                         (pi->fem5g->antswctrllut == 4)))) {
			MOD_PHYREGC(pi, HTPHY_RfctrlIntc, core, tr_sw_tx_pu, 1);
			MOD_PHYREGC(pi, HTPHY_RfctrlIntc, core, tr_sw_rx_pu, 0);
		} else {
			MOD_PHYREGC(pi, HTPHY_RfctrlIntc, core, tr_sw_tx_pu, 0);
			MOD_PHYREGC(pi, HTPHY_RfctrlIntc, core, tr_sw_rx_pu, 1);
		}

		MOD_PHYREGC(pi, HTPHY_RfctrlIntc, core, override_ext_pa, 1);
		MOD_PHYREGC(pi, HTPHY_RfctrlIntc, core, override_tr_sw, 1);

		/* Power Down Internal PA */
		porig->RfctrlPU[core] = phy_utils_read_phyreg(pi, HTPHY_RfctrlPU(core));
		porig->RfctrlOverride[core] = phy_utils_read_phyreg(pi, HTPHY_RfctrlOverride(core));
		MOD_PHYREGC(pi, HTPHY_RfctrlPU,       core, intpa_pu, 0);
		MOD_PHYREGC(pi, HTPHY_RfctrlOverride, core, intpa_pu, 1);

		/* disable PAPD (if enabled)
		 * FIXME: not supported (and not needed) yet
		 * porig->PapdEnable[core] = phy_utils_read_phyreg(pi, NPHY_PapdEnable(core));
		 * MOD_PHYREGC(pi, HTPHY_PapdEnable, core, compEnable, 0);
		 */

	} /* for core */

	/* Disable the re-sampler (in case spur avoidance is on) */
	porig->BBConfig = phy_utils_read_phyreg(pi, HTPHY_BBConfig);
	MOD_PHYREG(pi, HTPHY_BBConfig, resample_clk160, 0);
}

static void
wlc_phy_txcal_phy_cleanup_htphy(phy_info_t *pi)
{
	phy_info_htphy_t *pi_ht = (phy_info_htphy_t *)pi->u.pi_htphy;
	htphy_txcal_phyregs_t *porig = &(pi_ht->ht_txcal_phyregs_orig);
	uint8  core;

	/*  CLEANUP: Restore Original Values  */
	FOREACH_CORE(pi, core) {

		/* Restore AUX-ADC Select */
		phy_utils_write_phyreg(pi, HTPHY_Afectrl(core), porig->Afectrl[core]);
		phy_utils_write_phyreg(pi, HTPHY_AfectrlOverride(core),
		                       porig->AfectrlOverride[core]);

		/* Restore AUX-ADC format */
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL, 1, core*0x10 + 3,
		                          16, &porig->Afectrl_AuxADCmode[core]);

		/* restore ExtPA PU & TR */
		phy_utils_write_phyreg(pi, HTPHY_RfctrlIntc(core), porig->RfctrlIntc[core]);

		/* restore IntPA PU */
		phy_utils_write_phyreg(pi, HTPHY_RfctrlPU(core), porig->RfctrlPU[core]);
		phy_utils_write_phyreg(pi, HTPHY_RfctrlOverride(core), porig->RfctrlOverride[core]);

		/* restore PAPD Enable
		 * FIXME: not supported (and not needed) yet
		 * phy_utils_write_phyreg(pi, NPHY_PapdEnable(core), porig->PapdEnable[core]);
		 */

	} /* for core */

	/* Re-enable re-sampler (in case spur avoidance is on) */
	phy_utils_write_phyreg(pi, HTPHY_BBConfig, porig->BBConfig);

	/* XXX PR85342 When entering into Spur-Avoidance Mode (On or On2)
	 * cleanup to avoid PHY hang in PAYDECODE
	 */
	wlc_phy_resetcca_htphy(pi);
}

void
wlc_phy_cals_htphy(phy_info_t *pi, uint8 searchmode)
{
	/* XXX FIXME:
	 * ToDo:
	 *       - CTS-to-self for Rx cal / all cals? see nphy
	 *       - radar protection ? covered already?
	 */

	uint8 core;
	uint8 tx_pwr_ctrl_state;
	uint8 phase_id = pi->cal_info->cal_phase_id;
	uint16 tbl_cookie = TXCAL_CACHE_VALID;
	htphy_cal_result_t *htcal = &pi->cal_info->u.htcal;
#if defined(PHYCAL_CACHING)
	ch_calcache_t *ctx = wlc_phy_get_chanctx(pi, pi->radio_chanspec);
#endif // endif

	PHY_TRACE(("wl%d: Running HTPHY periodic calibration\n", pi->sh->unit));

	/* -----------------
	 *  Initializations
	 * -----------------
	 */

	/* Exit immediately if we are running on Quickturn */
	if (ISSIM_ENAB(pi->sh->sih)) {
		wlc_phy_cal_perical_mphase_reset(pi);
		return;
	}

	/* skip cal if phy is muted */
	if (PHY_MUTED(pi)) {
		return;
	}

	/*
	 * Search-Mode Sanity Check for Tx-iqlo-Cal
	 *
	 * Notes: - "RESTART" means: start with 0-coeffs and use large search radius
	 *        - "REFINE"  means: start with latest coeffs and only search
	 *                    around that (faster)
	 *        - here, if channel has changed or no previous valid coefficients
	 *          are available, enforce RESTART search mode (this shouldn't happen
	 *          unless cal driver code is work-in-progress, so this is merely a safety net)
	 */
	if ((pi->radio_chanspec != htcal->chanspec) ||
	    (htcal->txiqlocal_coeffsvalid == 0)) {
		searchmode = PHY_CAL_SEARCHMODE_RESTART;
	}

	/*
	 * If previous phase of multiphase cal was on different channel,
	 * then restart multiphase cal on current channel (again, safety net)
	 */
	if ((phase_id > MPHASE_CAL_STATE_INIT)) {
		if (htcal->chanspec != pi->radio_chanspec) {
			wlc_phy_cal_perical_mphase_restart(pi);
		}
	}

	/* Make the ucode send a CTS-to-self packet with duration set to 10ms. This
	 *  prevents packets from other STAs/AP from interfering with Rx IQcal
	 */
	/* XXX FIXME
	 *	if ((pi->mphase_cal_phase_id == MPHASE_CAL_STATE_RXCAL)) {
	 *		wlapi_bmac_write_shm(pi->sh->physhim, M_CTS_DURATION, 10000);
	 *	}
	 */

	/* Disable Power control */
	tx_pwr_ctrl_state = pi->txpwrctrl;
	wlc_phy_txpwrctrl_enable_htphy(pi, PHY_TPC_HW_OFF);

	/* Prepare Mac and Phregs */
	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	phy_utils_phyreg_enter(pi);

	/* -------------------
	 *  Calibration Calls
	 * -------------------
	 */

	PHY_NONE(("wlc_phy_cals_htphy: Time=%d, LastTi=%d, SrchMd=%d, PhIdx=%d,"
		" Chan=%d, LastCh=%d, First=%d, vld=%d\n",
		pi->sh->now, pi->last_cal_time, searchmode, phase_id,
		pi->radio_chanspec, htcal->chanspec,
		pi->first_cal_after_assoc, htcal->txiqlocal_coeffsvalid));

	if (phase_id == MPHASE_CAL_STATE_IDLE) {

		/*
		 * SINGLE-SHOT Calibrations
		 *
		 *    Call all Cals one after another
		 *
		 *    Notes:
		 *    - if this proc is called with the phase state in IDLE,
		 *      we know that this proc was called directly rather
		 *      than via the mphase scheduler (the latter puts us into
		 *      INIT state); under those circumstances, perform immediate
		 *      execution over all cal tasks
		 *    - for better code structure, we would use the below mphase code for
		 *      sphase case, too, by utilizing an appropriate outer for-loop
		 */

		/* TO-DO: Ensure that all inits and cleanups happen here */

		/* carry out all phases "en bloc", for comments see the various phases below */
		pi->cal_info->last_cal_time     = pi->sh->now;
		htcal->chanspec = pi->radio_chanspec;
		/* Idle TSSI determination, we do not want to do
		 * it in channel switch to avoid extra delay,
		 thus we do it here
		*/
		wlc_phy_txpwrctrl_idle_tssi_meas_htphy(pi);

		wlc_phy_txpwrctrl_pwr_setup_htphy(pi); /* to write idle tssi */

		wlc_phy_precal_txgain_htphy(pi, htcal->txcal_txgain);
		wlc_phy_cal_txiqlo_htphy(pi, searchmode, FALSE); /* request "Sphase" */
		wlc_phy_cal_rxiq_htphy(pi, FALSE);
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_IQLOCAL, 1,
		                          IQTBL_CACHE_COOKIE_OFFSET, 16, &tbl_cookie);
		pi->u.pi_htphy->txcal_cache_cookie = 0;
		pi->first_cal_after_assoc = FALSE;

#if defined(PHYCAL_CACHING)
		if (ctx)
			wlc_phy_cal_cache_htphy((wlc_phy_t *)pi);
#endif // endif

	} else {

		/*
		 * MULTI-PHASE CAL
		 *
		 *   Carry out next step in multi-phase execution of cal tasks
		 *
		 */

		switch (phase_id) {
		case CAL_PHASE_INIT:

			/*
			 *   Housekeeping & Pre-Txcal Tx Gain Adjustment
			 */

			/* remember time and channel of this cal event */
			pi->cal_info->last_cal_time     = pi->sh->now;
			htcal->chanspec = pi->radio_chanspec;

			/* PreCal Gain Ctrl:
			 * get tx gain settings (radio, dac, bbmult) to be used throughout
			 * tx iqlo cal; also resets "ladder_updated" flags in pi to 0
			 */
			wlc_phy_precal_txgain_htphy(pi, htcal->txcal_txgain);

			/* move on */
			pi->cal_info->cal_phase_id++;
			break;

		case CAL_PHASE_TX0:
		case CAL_PHASE_TX1:
		case CAL_PHASE_TX2:
		case CAL_PHASE_TX3:
		case CAL_PHASE_TX4:
		case CAL_PHASE_TX5:
		case CAL_PHASE_TX6:
		case CAL_PHASE_TX7:
		case CAL_PHASE_TX_LAST:

			/*
			 *   Tx-IQLO-Cal
			 */

			/* to ensure radar detect is skipped during cals */
			if ((pi->radar_percal_mask & 0x10) != 0) {
				pi->u.pi_htphy->radar_cal_active = TRUE;
			}

			/* execute txiqlo cal's next phase */
			if (wlc_phy_cal_txiqlo_htphy(pi, searchmode, TRUE) != BCME_OK) {
				/* rare case, just reset */
				PHY_ERROR(("wlc_phy_cal_txiqlo_htphy failed\n"));
				wlc_phy_cal_perical_mphase_reset(pi);
				break;
			}

			/* move on */
			if ((pi->cal_info->cal_phase_id == CAL_PHASE_TX_LAST) &&
			    (!PHY_IPA(pi))) {
				pi->cal_info->cal_phase_id++; /* so we skip papd cal for non-ipa */
			}
			pi->cal_info->cal_phase_id++;
			break;

		case CAL_PHASE_PAPDCAL:

			/*
			 *   PAPD Cal -- not supported yet. XXX FIXME
			 */

			/* move on */
			pi->cal_info->cal_phase_id++;
			break;

		case CAL_PHASE_RXCAL:
			/*
			 *   Rx IQ Cal
			 */

			if ((pi->radar_percal_mask & 0x1) != 0) {
				pi->u.pi_htphy->radar_cal_active = TRUE;
			}

			wlc_phy_cal_rxiq_htphy(pi, FALSE);
			wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_IQLOCAL, 1,
				IQTBL_CACHE_COOKIE_OFFSET, 16, &tbl_cookie);
			pi->u.pi_htphy->txcal_cache_cookie = 0;

#if defined(PHYCAL_CACHING)
			if (ctx)
				wlc_phy_cal_cache_htphy((wlc_phy_t *)pi);
#endif // endif
			/* move on */
			pi->cal_info->cal_phase_id++;
			break;

		case CAL_PHASE_RSSICAL:

			/*
			 *     RSSI Cal & VCO Cal
			 */

			if ((pi->radar_percal_mask & 0x4) != 0) {
			    pi->u.pi_htphy->radar_cal_active = TRUE;
			}

			/* RSSI & VCO cal (prevents VCO/PLL from losing lock with temp delta) */
			/* wlc_phy_rssi_cal_htphy(pi); */
			wlc_phy_radio2059_vcocal(pi);

			/* If this is the first calibration after association then we
			 * still have to do calibrate the idle-tssi, otherrwise done
			 */
			if (pi->first_cal_after_assoc || pi->itssical) {
				pi->cal_info->cal_phase_id++;
			} else {
				wlc_phy_cal_perical_mphase_reset(pi);
			}
			break;

		case CAL_PHASE_IDLETSSI:

			/*
			 *     Idle TSSI & TSSI-to-dBm Mapping Setup
			 */

			/* XXX FIXME:
			 * shouldn't idle-tssi cal before the other cals? for precal power control
			 * (if using actual gain iterations rather than fixed selection, we need
			 * to know the idle tssi)
			 */

			if ((pi->radar_percal_mask & 0x8) != 0)
				pi->u.pi_htphy->radar_cal_active = TRUE;

			/* Idle TSSI determination once right after join/up/assoc */
			wlc_phy_txpwrctrl_idle_tssi_meas_htphy(pi);
			wlc_phy_txpwrctrl_pwr_setup_htphy(pi); /* to write idle tssi */

			/* done with multi-phase cal, reset phase */
			pi->first_cal_after_assoc = FALSE;
			wlc_phy_cal_perical_mphase_reset(pi);
			break;

		default:
			PHY_ERROR(("wlc_phy_cals_phy: Invalid calibration phase %d\n", phase_id));
			ASSERT(0);
			wlc_phy_cal_perical_mphase_reset(pi);
			break;
		}
	}

	/* ----------
	 *  Cleanups
	 * ----------
	 */

	wlc_phy_txpwrctrl_enable_htphy(pi, tx_pwr_ctrl_state);
	/* restore the old txpwr index if they are valid */
	FOREACH_CORE(pi, core) {
		if (pi->u.pi_htphy->txpwrindex_hw_save[core] != 128) {
			wlc_phy_txpwrctrl_set_cur_index_htphy(pi,
				pi->u.pi_htphy->txpwrindex_hw_save[core], core);
		}
	}

	phy_utils_phyreg_exit(pi);
	wlapi_enable_mac(pi->sh->physhim);

}

static int
wlc_phy_cal_txiqlo_htphy(phy_info_t *pi, uint8 searchmode, uint8 mphase)
{
	uint8  phy_bw, bw_idx, rd_select = 0, wr_select1 = 0, wr_select2 = 0;
	uint16 tone_ampl;
	uint16 tone_freq;
	int    bcmerror = BCME_OK;
	uint8  num_cmds_total, num_cmds_per_core;
	uint8  cmd_idx, cmd_stop_idx, core, cal_type;
	uint16 *cmds;
	uint16 cmd;
	uint16 coeffs[2];
	uint16 *coeff_ptr;
	uint16 zero = 0;
	uint8  cr, k, num_cores;
	txgain_setting_t orig_txgain[4];
	htphy_cal_result_t *htcal = &pi->cal_info->u.htcal;

	/* -----------
	 *  Constants
	 * -----------
	 */

	/* Table of commands for RESTART & REFINE search-modes
	 *
	 *     This uses the following format (three hex nibbles left to right)
	 *      1. cal_type: 0 = IQ (a/b),   1 = deprecated
	 *                   2 = LOFT digital (di/dq)
	 *                   3 = LOFT analog, fine,   injected at mixer      (ei/eq)
	 *                   4 = LOFT analog, coarse, injected at mixer, too (fi/fq)
	 *      2. initial stepsize (in log2)
	 *      3. number of cal precision "levels"
	 *
	 *     Notes: - functions assumes that order of LOFT cal cmds will be f => e => d,
	 *              where it's ok to have multiple cmds (say interrupted by IQ) of
	 *              the same type; this is due to zeroing out of e and/or d that happens
	 *              even during REFINE cal to avoid a coefficient "divergence" (increasing
	 *              LOFT comp over time of different types that cancel each other)
	 *            - final cal cmd should NOT be analog LOFT cal (otherwise have to manually
	 *              pick up analog LOFT settings from best_coeffs and write to radio)
	 */
	uint16 cmds_RESTART[] = { 0x434, 0x334, 0x084, 0x267, 0x056, 0x234};
	uint16 cmds_REFINE[] = { 0x423, 0x334, 0x073, 0x267, 0x045, 0x234};

	/* zeros start coeffs (a,b,di/dq,ei/eq,fi/fq for each core) */
	uint16 start_coeffs_RESTART[] = {0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0};

	/* interval lengths for gain control and correlation segments
	 *   (top/bottom nibbles are for guard and measurement intlvs, resp., in log 2 # samples)
	 */
	uint8 nsamp_gctrl[] = {0x76, 0x87};
	uint8 nsamp_corrs[] = {0x68, 0x79};

	/* -------
	 *  Inits
	 * -------
	 */

	num_cores = pi->pubpi.phy_corenum;

	/* prevent crs trigger */
	wlc_phy_stay_in_carriersearch_htphy(pi, TRUE);

	/* phy_bw */
	if (CHSPEC_IS40(pi->radio_chanspec)) {
		phy_bw = 40;
		bw_idx = 1;
	} else {
		phy_bw = 20;
		bw_idx = 0;
	}

	/* Put the radio and phy into TX iqlo cal state, including tx gains */
	wlc_phy_txcal_radio_setup_htphy(pi);
	wlc_phy_txcal_phy_setup_htphy(pi);
	wlc_phy_txcal_txgain_setup_htphy(pi, &htcal->txcal_txgain[0], &orig_txgain[0]);

	/* Set IQLO Cal Engine Gain Control Parameters including engine Enable
	 *   iqlocal_en<15> / start_index / thresh_d2 / ladder_length_d2
	 */
	phy_utils_write_phyreg(pi, HTPHY_iqloCalCmdGctl, 0x8ad9);

	/*
	 *   Retrieve and set Start Coeffs
	 */
	if (pi->cal_info->cal_phase_id > CAL_PHASE_TX0) {
		/* mphase cal and have done at least 1 Tx phase already */
		coeff_ptr = htcal->txiqlocal_interm_coeffs; /* use results from previous phase */
	} else {
		/* single-phase cal or first phase of mphase cal */
		if (searchmode == PHY_CAL_SEARCHMODE_REFINE) {
			/* recal ("refine") */
			coeff_ptr = htcal->txiqlocal_coeffs; /* use previous cal's final results */
		} else {
			/* start from zero coeffs ("restart") */
			coeff_ptr = start_coeffs_RESTART; /* zero coeffs */
		}
		/* copy start coeffs to intermediate coeffs, for pairwise update from here on
		 *    (after all cmds/phases have filled this with latest values, this
		 *    will be copied to OFDM/BPHY coeffs and to htcal->txiqlocal_coeffs
		 *    for use by possible REFINE cal next time around)
		 */
		for (k = 0; k < 5*num_cores; k++) {
			htcal->txiqlocal_interm_coeffs[k] = coeff_ptr[k];
		}
	}
	FOREACH_CORE(pi, core) {
		wlc_phy_cal_txiqlo_coeffs_htphy(pi, CAL_COEFF_WRITE, coeff_ptr + 5*core + 0,
		                                TB_START_COEFFS_AB, core);
		wlc_phy_cal_txiqlo_coeffs_htphy(pi, CAL_COEFF_WRITE, coeff_ptr + 5*core + 2,
		                                TB_START_COEFFS_D,  core);
		wlc_phy_cal_txiqlo_coeffs_htphy(pi, CAL_COEFF_WRITE, coeff_ptr + 5*core + 3,
		                                TB_START_COEFFS_E,  core);
		wlc_phy_cal_txiqlo_coeffs_htphy(pi, CAL_COEFF_WRITE, coeff_ptr + 5*core + 4,
		                                TB_START_COEFFS_F,  core);
	}

	/*
	 *   Choose Cal Commands for this Phase
	 */
	if (searchmode == PHY_CAL_SEARCHMODE_RESTART) {
		cmds = cmds_RESTART;
		num_cmds_per_core = ARRAYSIZE(cmds_RESTART);
		num_cmds_total    = num_cores * num_cmds_per_core;
	} else {
		cmds = cmds_REFINE;
		num_cmds_per_core = ARRAYSIZE(cmds_REFINE);
		num_cmds_total    = num_cores * num_cmds_per_core;
	}
	if (mphase) {
		/* multi-phase: get next subset of commands (first & last index) */
		cmd_idx = (pi->cal_info->cal_phase_id - CAL_PHASE_TX0) *
			MPHASE_TXCAL_CMDS_PER_PHASE; /* first cmd index in this phase */
		if ((cmd_idx + MPHASE_TXCAL_CMDS_PER_PHASE - 1) < num_cmds_total) {
			cmd_stop_idx = cmd_idx + MPHASE_TXCAL_CMDS_PER_PHASE - 1;
		} else {
			cmd_stop_idx = num_cmds_total - 1;
		}
	} else {
		/* single-phase: execute all commands for all cores */
		cmd_idx = 0;
		cmd_stop_idx = num_cmds_total - 1;
	}

	/* turn on test tone */
	tone_ampl = 250;
	tone_freq = (phy_bw == 20) ? 2500 : 5000;
	if (pi->cal_info->cal_phase_id > CAL_PHASE_TX0) {
		wlc_phy_runsamples_htphy(pi, phy_bw * 8, 0xffff, 0, 1, 0);
		bcmerror = BCME_OK;
	} else {
		bcmerror = wlc_phy_tx_tone_htphy(pi, tone_freq, tone_ampl, 1, 0, FALSE);
	}

	PHY_NONE(("wlc_phy_cal_txiqlo_htphy (after inits): SearchMd=%d, MPhase=%d,"
		" CmdIds=(%d to %d)\n",
		searchmode, mphase, cmd_idx, cmd_stop_idx));

	/* ---------------
	 *  Cmd Execution
	 * ---------------
	 */

	if (bcmerror == BCME_OK) { /* in case tone doesn't start (still needed?) */

		/* loop over commands in this cal phase */
		for (; cmd_idx <= cmd_stop_idx; cmd_idx++) {

			/* get command, cal_type, and core */
			core     = cmd_idx / num_cmds_per_core; /* integer divide */
			cmd      = cmds[cmd_idx % num_cmds_per_core] | 0x8000 | (core << 12);
			cal_type = ((cmd & 0x0F00) >> 8);

			/* PHY_CAL(("wlc_phy_cal_txiqlo_htphy: Cmds => cmd_idx=%2d, Cmd=0x%04x,
			 *          cal_type=%d, core=%d\n", cmd_idx, cmd, cal_type, core));
			 */

			/* set up scaled ladders for desired bbmult of current core */
			if (!htcal->txiqlocal_ladder_updated[core]) {
				wlc_phy_cal_txiqlo_update_ladder_htphy(pi,
					htcal->txcal_txgain[core].bbmult);
				htcal->txiqlocal_ladder_updated[core] = TRUE;
			}

			/* set intervals settling and measurement intervals */
			phy_utils_write_phyreg(pi, HTPHY_iqloCalCmdNnum,
			              (nsamp_corrs[bw_idx] << 8) | nsamp_gctrl[bw_idx]);

			/* if coarse-analog-LOFT cal (fi/fq), always zero out ei/eq and di/dq;
			 * if fine-analog-LOFT   cal (ei/dq), always zero out di/dq
			 *   - even do this with search-type REFINE, to prevent a "drift"
			 *   - assumes that order of LOFT cal cmds will be f => e => d, where it's
			 *     ok to have multiple cmds (say interrupted by IQ cal) of the same type
			 */
			if ((cal_type == CAL_TYPE_LOFT_ANA_COARSE) ||
			    (cal_type == CAL_TYPE_LOFT_ANA_FINE)) {
				wlc_phy_cal_txiqlo_coeffs_htphy(pi, CAL_COEFF_WRITE,
					&zero, TB_START_COEFFS_D, core);
			}
			if (cal_type == CAL_TYPE_LOFT_ANA_COARSE) {
				wlc_phy_cal_txiqlo_coeffs_htphy(pi, CAL_COEFF_WRITE,
					&zero, TB_START_COEFFS_E, core);
			}

			/* now execute this command and wait max of ~20ms */
			phy_utils_write_phyreg(pi, HTPHY_iqloCalCmd, cmd);
			SPINWAIT(((phy_utils_read_phyreg(pi, HTPHY_iqloCalCmd) & 0xc000) != 0),
				HTPHY_SPINWAIT_TXIQLO);
			ASSERT((phy_utils_read_phyreg(pi, HTPHY_iqloCalCmd) & 0xc000) == 0);

			/* copy coeffs best-to-start and to
			 * "intermediate" coeffs in pi state; in mphase,
			 * the latter is also used as starting point
			 * when coming back for next phase, and
			 * we always use the "intermediate" coeffs at
			 * the very end to apply to OFDM/BPHY,
			 * see below;
			 * (copy step only done for coeff pair that
			 * changed, thereby also covering ei/eq swap
			 * per PR 79353)
			 */
			switch (cal_type) {
			case CAL_TYPE_IQ:
				rd_select  = TB_BEST_COEFFS_AB;
				wr_select1 = TB_START_COEFFS_AB;
				wr_select2 = PI_INTER_COEFFS_AB;
				break;
			case CAL_TYPE_LOFT_DIG:
				rd_select  = TB_BEST_COEFFS_D;
				wr_select1 = TB_START_COEFFS_D;
				wr_select2 = PI_INTER_COEFFS_D;
				break;
			case CAL_TYPE_LOFT_ANA_FINE:
				rd_select  = TB_BEST_COEFFS_E;
				wr_select1 = TB_START_COEFFS_E;
				wr_select2 = PI_INTER_COEFFS_E;
				break;
			case CAL_TYPE_LOFT_ANA_COARSE:
				rd_select  = TB_BEST_COEFFS_F;
				wr_select1 = TB_START_COEFFS_F;
				wr_select2 = PI_INTER_COEFFS_F;
				break;
			default:
				ASSERT(0);
			}
			wlc_phy_cal_txiqlo_coeffs_htphy(pi, CAL_COEFF_READ,
				coeffs, rd_select,  core);
			wlc_phy_cal_txiqlo_coeffs_htphy(pi, CAL_COEFF_WRITE,
				coeffs, wr_select1, core);
			wlc_phy_cal_txiqlo_coeffs_htphy(pi, CAL_COEFF_WRITE,
				coeffs, wr_select2, core);

		} /* command loop */

		/* single phase or last tx stage in multiphase cal: apply & store overall results */
		if ((mphase == 0) || (pi->cal_info->cal_phase_id == CAL_PHASE_TX_LAST)) {

			PHY_CAL(("wlc_phy_cal_txiqlo_htphy (mphase = %d, refine = %d):\n",
			         mphase, searchmode == PHY_CAL_SEARCHMODE_REFINE));
			for (cr = 0; cr < num_cores; cr++) {
				/* Save and Apply IQ Cal Results */
				wlc_phy_cal_txiqlo_coeffs_htphy(pi, CAL_COEFF_READ,
					coeffs, PI_INTER_COEFFS_AB, cr);
				wlc_phy_cal_txiqlo_coeffs_htphy(pi, CAL_COEFF_WRITE,
					coeffs, PI_FINAL_COEFFS_AB, cr);
				wlc_phy_cal_txiqlo_coeffs_htphy(pi, CAL_COEFF_WRITE,
					coeffs, TB_OFDM_COEFFS_AB,  cr);
				wlc_phy_cal_txiqlo_coeffs_htphy(pi, CAL_COEFF_WRITE,
					coeffs, TB_BPHY_COEFFS_AB,  cr);

				/* Save and Apply Dig LOFT Cal Results */
				wlc_phy_cal_txiqlo_coeffs_htphy(pi, CAL_COEFF_READ,
					coeffs, PI_INTER_COEFFS_D, cr);
				wlc_phy_cal_txiqlo_coeffs_htphy(pi, CAL_COEFF_WRITE,
					coeffs, PI_FINAL_COEFFS_D, cr);
				wlc_phy_cal_txiqlo_coeffs_htphy(pi, CAL_COEFF_WRITE,
					coeffs, TB_OFDM_COEFFS_D,  cr);
				wlc_phy_cal_txiqlo_coeffs_htphy(pi, CAL_COEFF_WRITE,
					coeffs, TB_BPHY_COEFFS_D,  cr);

				/* Apply Analog LOFT Comp
				 * - unncessary if final command on each core is digital
				 * LOFT-cal or IQ-cal
				 * - then the loft comp coeffs were applied to radio
				 * at the beginning of final command per core
				 * - this is assumed to be the case, so nothing done here
				 */

				/* Save Analog LOFT Comp in PI State */
				wlc_phy_cal_txiqlo_coeffs_htphy(pi, CAL_COEFF_READ,
					coeffs, PI_INTER_COEFFS_E, cr);
				wlc_phy_cal_txiqlo_coeffs_htphy(pi, CAL_COEFF_WRITE,
					coeffs, PI_FINAL_COEFFS_E, cr);
				wlc_phy_cal_txiqlo_coeffs_htphy(pi, CAL_COEFF_READ,
					coeffs, PI_INTER_COEFFS_F, cr);
				wlc_phy_cal_txiqlo_coeffs_htphy(pi, CAL_COEFF_WRITE,
					coeffs, PI_FINAL_COEFFS_F, cr);

				/* Print out Results */
				PHY_CAL(("\tcore-%d: a/b = (%4d,%4d), d = (%4d,%4d),"
					" e = (%4d,%4d), f = (%4d,%4d)\n", cr,
					htcal->txiqlocal_coeffs[cr*5+0],  /* a */
					htcal->txiqlocal_coeffs[cr*5+1],  /* b */
					(htcal->txiqlocal_coeffs[cr*5+2] & 0xFF00) >> 8, /* di */
					(htcal->txiqlocal_coeffs[cr*5+2] & 0x00FF),      /* dq */
					(htcal->txiqlocal_coeffs[cr*5+3] & 0xFF00) >> 8, /* ei */
					(htcal->txiqlocal_coeffs[cr*5+3] & 0x00FF),      /* eq */
					(htcal->txiqlocal_coeffs[cr*5+4] & 0xFF00) >> 8, /* fi */
					(htcal->txiqlocal_coeffs[cr*5+4] & 0x00FF)));   /* fq */
			} /* for cr */

			/* validate availability of results and store off channel */
			htcal->txiqlocal_coeffsvalid = TRUE;
			htcal->chanspec = pi->radio_chanspec;
		} /* writing of results */

		/* Switch off test tone */
		wlc_phy_stopplayback_htphy(pi);	/* mimophy_stop_playback */

		/* disable IQ/LO cal */
		phy_utils_write_phyreg(pi, HTPHY_iqloCalCmdGctl, 0x0000);
	} /* if BCME_OK */

	/* clean Up PHY and radio */
	wlc_phy_txcal_txgain_cleanup_htphy(pi, &orig_txgain[0]);
	wlc_phy_txcal_phy_cleanup_htphy(pi);
	wlc_phy_txcal_radio_cleanup_htphy(pi);

	/* XXX FIXME: May consider saving off LOFT comp before 1st phase and
	 *  restoring LOFT comp  after each phase except for last phase
	 */

	/*
	 *-----------*
	 *  Cleanup  *
	 *-----------
	 */

	/* prevent crs trigger */
	wlc_phy_stay_in_carriersearch_htphy(pi, FALSE);

	return bcmerror;
}

static void
wlc_phy_cal_txiqlo_coeffs_htphy(phy_info_t *pi, uint8 rd_wr, uint16 *coeff_vals,
                                uint8 select, uint8 core) {

	/* handles IQLOCAL coefficients access (read/write from/to
	 * iqloCaltbl and pi State)
	 *
	 * not sure if reading/writing the pi state coeffs via this appraoch
	 * is a bit of an overkill
	 */

	/* {num of 16b words to r/w, start offset (ie address), core-to-core block offset} */
	htphy_coeff_access_t coeff_access_info[] = {
		{2, 64, 8},  /* TB_START_COEFFS_AB   */
		{1, 67, 8},  /* TB_START_COEFFS_D    */
		{1, 68, 8},  /* TB_START_COEFFS_E    */
		{1, 69, 8},  /* TB_START_COEFFS_F    */
		{2, 128, 7}, /*   TB_BEST_COEFFS_AB  */
		{1, 131, 7}, /*   TB_BEST_COEFFS_D   */
		{1, 132, 7}, /*   TB_BEST_COEFFS_E   */
		{1, 133, 7}, /*   TB_BEST_COEFFS_F   */
		{2, 96,  4}, /* TB_OFDM_COEFFS_AB    */
		{1, 98,  4}, /* TB_OFDM_COEFFS_D     */
		{2, 112, 4}, /* TB_BPHY_COEFFS_AB    */
		{1, 114, 4}, /* TB_BPHY_COEFFS_D     */
		{2, 0, 5},   /*   PI_INTER_COEFFS_AB */
		{1, 2, 5},   /*   PI_INTER_COEFFS_D  */
		{1, 3, 5},   /*   PI_INTER_COEFFS_E  */
		{1, 4, 5},   /*   PI_INTER_COEFFS_F  */
		{2, 0, 5},   /* PI_FINAL_COEFFS_AB   */
		{1, 2, 5},   /* PI_FINAL_COEFFS_D    */
		{1, 3, 5},   /* PI_FINAL_COEFFS_E    */
		{1, 4, 5}    /* PI_FINAL_COEFFS_F    */
	};
	htphy_cal_result_t *htcal = &pi->cal_info->u.htcal;

	uint8 nwords, offs, boffs, k;

	/* get access info for desired choice */
	nwords = coeff_access_info[select].nwords;
	offs   = coeff_access_info[select].offs;
	boffs  = coeff_access_info[select].boffs;

	/* read or write given coeffs */
	if (select <= TB_BPHY_COEFFS_D) { /* START and BEST coeffs in Table */
		if (rd_wr == CAL_COEFF_READ) { /* READ */
			wlc_phy_table_read_htphy(pi, HTPHY_TBL_ID_IQLOCAL, nwords,
				offs + boffs*core, 16, coeff_vals);
		} else { /* WRITE */
			wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_IQLOCAL, nwords,
				offs + boffs*core, 16, coeff_vals);
		}
	} else if (select <= PI_INTER_COEFFS_F) { /* PI state intermediate coeffs */
		for (k = 0; k < nwords; k++) {
			if (rd_wr == CAL_COEFF_READ) { /* READ */
				coeff_vals[k] = htcal->txiqlocal_interm_coeffs[offs +
				                                               boffs*core + k];
			} else { /* WRITE */
				htcal->txiqlocal_interm_coeffs[offs +
				                               boffs*core + k] = coeff_vals[k];
			}
		}
	} else { /* PI state final coeffs */
		for (k = 0; k < nwords; k++) { /* PI state final coeffs */
			if (rd_wr == CAL_COEFF_READ) { /* READ */
				coeff_vals[k] = htcal->txiqlocal_coeffs[offs + boffs*core + k];
			} else { /* WRITE */
				htcal->txiqlocal_coeffs[offs + boffs*core + k] = coeff_vals[k];
			}
		}
	}
}

static void
wlc_phy_cal_txiqlo_update_ladder_htphy(phy_info_t *pi, uint16 bbmult)
{
	uint8  indx;
	uint32 bbmult_scaled;
	uint16 tblentry;

	htphy_txiqcal_ladder_t ladder_lo[] = {
	{3, 0}, {4, 0}, {6, 0}, {9, 0}, {13, 0}, {18, 0},
	{25, 0}, {25, 1}, {25, 2}, {25, 3}, {25, 4}, {25, 5},
	{25, 6}, {25, 7}, {35, 7}, {50, 7}, {71, 7}, {100, 7}};

	htphy_txiqcal_ladder_t ladder_iq[] = {
	{3, 0}, {4, 0}, {6, 0}, {9, 0}, {13, 0}, {18, 0},
	{25, 0}, {35, 0}, {50, 0}, {71, 0}, {100, 0}, {100, 1},
	{100, 2}, {100, 3}, {100, 4}, {100, 5}, {100, 6}, {100, 7}};

	for (indx = 0; indx < 18; indx++) {

		/* calculate and write LO cal gain ladder */
		bbmult_scaled = ladder_lo[indx].percent * bbmult;
		bbmult_scaled /= 100;
		tblentry = ((bbmult_scaled & 0xff) << 8) | ladder_lo[indx].g_env;
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_IQLOCAL, 1, indx, 16, &tblentry);

		/* calculate and write IQ cal gain ladder */
		bbmult_scaled = ladder_iq[indx].percent * bbmult;
		bbmult_scaled /= 100;
		tblentry = ((bbmult_scaled & 0xff) << 8) | ladder_iq[indx].g_env;
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_IQLOCAL, 1, indx+32, 16, &tblentry);
	}
}

static void
wlc_phy_txcal_txgain_setup_htphy(phy_info_t *pi, txgain_setting_t *txcal_txgain,
txgain_setting_t *orig_txgain)
{
	uint16 core;
	uint16 tmp;

	FOREACH_CORE(pi, core) {
		/* store off orig and set new tx radio gain */
		wlc_phy_table_read_htphy(pi, HTPHY_TBL_ID_RFSEQ, 1, (0x110 + core),
			16, &(orig_txgain[core].rad_gain));
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_RFSEQ, 1, (0x110 + core),
			16, &txcal_txgain[core].rad_gain);

		PHY_NONE(("\n radio gain = 0x%x, bbm=%d, dacgn = %d  \n",
			txcal_txgain[core].rad_gain,
			txcal_txgain[core].bbmult,
			txcal_txgain[core].dac_gain));

		/* store off orig and set new dac gain */
		tmp = phy_utils_read_phyreg(pi, HTPHY_AfeseqInitDACgain);
		switch (core) {
		case 0:
			orig_txgain[0].dac_gain =
				(tmp & HTPHY_AfeseqInitDACgain_InitDACgain0_MASK) >>
				HTPHY_AfeseqInitDACgain_InitDACgain0_SHIFT;
			MOD_PHYREG(pi, HTPHY_AfeseqInitDACgain, InitDACgain0,
			           txcal_txgain[0].dac_gain);
			break;
		case 1:
			orig_txgain[1].dac_gain =
				(tmp & HTPHY_AfeseqInitDACgain_InitDACgain1_MASK) >>
				HTPHY_AfeseqInitDACgain_InitDACgain1_SHIFT;
			MOD_PHYREG(pi, HTPHY_AfeseqInitDACgain, InitDACgain1,
			           txcal_txgain[1].dac_gain);
			break;
		case 2:
			orig_txgain[2].dac_gain =
				(tmp & HTPHY_AfeseqInitDACgain_InitDACgain2_MASK) >>
				HTPHY_AfeseqInitDACgain_InitDACgain2_SHIFT;
			MOD_PHYREG(pi, HTPHY_AfeseqInitDACgain, InitDACgain2,
			           txcal_txgain[2].dac_gain);
			break;
		}

		/* store off orig and set new bbmult gain */
		wlc_phy_get_tx_bbmult(pi, &(orig_txgain[core].bbmult),  core);
		wlc_phy_set_tx_bbmult(pi, &(txcal_txgain[core].bbmult), core);
	}
}

static void
wlc_phy_txcal_txgain_cleanup_htphy(phy_info_t *pi, txgain_setting_t *orig_txgain)
{
	uint8 core;

	FOREACH_CORE(pi, core) {
		/* restore gains: DAC, Radio and BBmult */
		switch (core) {
		case 0:
			MOD_PHYREG(pi, HTPHY_AfeseqInitDACgain, InitDACgain0,
			           orig_txgain[0].dac_gain);
			break;
		case 1:
			MOD_PHYREG(pi, HTPHY_AfeseqInitDACgain, InitDACgain1,
			           orig_txgain[1].dac_gain);
			break;
		case 2:
			MOD_PHYREG(pi, HTPHY_AfeseqInitDACgain, InitDACgain2,
			           orig_txgain[2].dac_gain);
			break;
		}
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_RFSEQ, 1, (0x110 + core),
			16, &(orig_txgain[core].rad_gain));
		wlc_phy_set_tx_bbmult(pi, &(orig_txgain[core].bbmult), core);
	}
}

static void
wlc_phy_precal_txgain_htphy(phy_info_t *pi, txgain_setting_t *target_gains)
{
	/*   This function determines the tx gain settings to be
	 *   used during tx iqlo calibration; that is, it sends back
	 *   the following settings for each core:
	 *       - radio gain
	 *       - dac gain
	 *       - bbmult
	 *   This is accomplished by choosing a predefined power-index, or by
	 *   setting gain elements explicitly to predefined values, or by
	 *   doing actual "pre-cal gain control". Either way, the idea is
	 *   to get a stable setting for which the swing going into the
	 *   envelope detectors is large enough for good "envelope ripple"
	 *   while avoiding distortion or EnvDet overdrive during the cal.
	 *
	 *   Note:
	 *       - this function and the calling infrastructure is set up
	 *         in a way not to leave behind any modified state; this
	 *         is in contrast to mimophy ("nphy"); in htphy, only the
	 *         desired gain quantities are set/found and set back
	 */

	uint8 core;
	htphy_cal_result_t *htcal = &pi->cal_info->u.htcal;

	/* reset ladder_updated flags so tx-iqlo-cal ensures appropriate recalculation */
	FOREACH_CORE(pi, core) {
		htcal->txiqlocal_ladder_updated[core] = 0;
	}

	/* get target tx gain settings */
	FOREACH_CORE(pi, core) {
		if (1) {
			int8 target_pwr_idx;
			/* XXX PR100453 LOFT suppression improvements (cal index change in 5G)
			 * specify tx gain by index (reads from tx power table)
			 */
			if (CHSPEC_IS5G(pi->radio_chanspec)) {
				target_pwr_idx = 25;
			} else {
				target_pwr_idx = 0;
			}
			wlc_phy_get_txgain_settings_by_index(
				pi, &(target_gains[core]), target_pwr_idx);
		} else if (0) {
			/* specify tx gain as hardcoded explicit gain */
			target_gains[core].rad_gain  = 0x000;
			target_gains[core].dac_gain  = 0;
			target_gains[core].bbmult = 64;
		} else {
			/* actual precal gain control */
		}
	}
}

/* see also: proc htphy_rx_iq_comp { {vals {}} {cores {}}} */
static void
wlc_phy_rx_iq_comp_htphy(phy_info_t *pi, uint8 write, phy_iq_comp_t *pcomp, uint8 rx_core)
{
	/* write: 0 - fetch values from phyregs into *pcomp
	 *        1 - deposit values from *pcomp into phyregs
	 *        2 - set all coeff phyregs to 0
	 *
	 * rx_core: specify which core to fetch/deposit
	 */

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	ASSERT(write <= 2);

	/* write values */
	if (write == 0) {
		pcomp->a = phy_utils_read_phyreg(pi, HTPHY_RxIQCompA(rx_core));
		pcomp->b = phy_utils_read_phyreg(pi, HTPHY_RxIQCompB(rx_core));
	} else if (write == 1) {
		phy_utils_write_phyreg(pi, HTPHY_RxIQCompA(rx_core), pcomp->a);
		phy_utils_write_phyreg(pi, HTPHY_RxIQCompB(rx_core), pcomp->b);
	} else {
		phy_utils_write_phyreg(pi, HTPHY_RxIQCompA(rx_core), 0);
		phy_utils_write_phyreg(pi, HTPHY_RxIQCompB(rx_core), 0);
	}
}

/* see also: proc htphy_rx_iq_est { {num_samps 2000} {wait_time ""} } */
void
wlc_phy_rx_iq_est_htphy(phy_info_t *pi, phy_iq_est_t *est, uint16 num_samps,
                        uint8 wait_time, uint8 wait_for_crs)
{
	uint8 core;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	/* Get Rx IQ Imbalance Estimate from modem */
	phy_utils_write_phyreg(pi, HTPHY_IqestSampleCount, num_samps);
	MOD_PHYREG(pi, HTPHY_IqestWaitTime, waitTime, wait_time);
	MOD_PHYREG(pi, HTPHY_IqestCmd, iqMode, wait_for_crs);

	MOD_PHYREG(pi, HTPHY_IqestCmd, iqstart, 1);

	/* wait for estimate */
	SPINWAIT(((phy_utils_read_phyreg(pi, HTPHY_IqestCmd) & HTPHY_IqestCmd_iqstart_MASK) != 0),
		HTPHY_SPINWAIT_IQEST);
	ASSERT((phy_utils_read_phyreg(pi, HTPHY_IqestCmd) & HTPHY_IqestCmd_iqstart_MASK) == 0);

	if ((phy_utils_read_phyreg(pi, HTPHY_IqestCmd) & HTPHY_IqestCmd_iqstart_MASK) == 0) {
		ASSERT(pi->pubpi.phy_corenum <= PHY_CORE_MAX);
		FOREACH_CORE(pi, core) {
			est[core].i_pwr =
			        (phy_utils_read_phyreg(pi, HTPHY_IqestipwrAccHi(core)) << 16) |
			        phy_utils_read_phyreg(pi, HTPHY_IqestipwrAccLo(core));
			est[core].q_pwr =
			        (phy_utils_read_phyreg(pi, HTPHY_IqestqpwrAccHi(core)) << 16) |
				phy_utils_read_phyreg(pi, HTPHY_IqestqpwrAccLo(core));
			est[core].iq_prod =
			        (phy_utils_read_phyreg(pi, HTPHY_IqestIqAccHi(core)) << 16) |
				phy_utils_read_phyreg(pi, HTPHY_IqestIqAccLo(core));
			PHY_NONE(("wlc_phy_rx_iq_est_htphy: core%d "
			         "i_pwr = %u, q_pwr = %u, iq_prod = %d\n",
			         core, est[core].i_pwr, est[core].q_pwr, est[core].iq_prod));
		}
	} else {
		PHY_ERROR(("wlc_phy_rx_iq_est_htphy: IQ measurement timed out\n"));
	}
}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
/* dump calibration regs/info */
void
wlc_phy_cal_dump_htphy(phy_info_t *pi, struct bcmstrbuf *b)
{

	uint8 core;
	int16  a_reg, b_reg, a_int, b_int;
	uint16 ab_int[2], d_reg, eir, eqr, fir, fqr;

	bcm_bprintf(b, "Tx-IQ/LOFT-Cal:\n");
	FOREACH_CORE(pi, core) {
		wlc_phy_cal_txiqlo_coeffs_htphy(pi, CAL_COEFF_READ, ab_int,
			TB_OFDM_COEFFS_AB, core);
		wlc_phy_cal_txiqlo_coeffs_htphy(pi, CAL_COEFF_READ, &d_reg,
			TB_OFDM_COEFFS_D, core);
		eir = phy_utils_read_radioreg(pi, RADIO_2059_TX_LOFT_FINE_I(core));
		eqr = phy_utils_read_radioreg(pi, RADIO_2059_TX_LOFT_FINE_Q(core));
		fir = phy_utils_read_radioreg(pi, RADIO_2059_TX_LOFT_COARSE_I(core));
		fqr = phy_utils_read_radioreg(pi, RADIO_2059_TX_LOFT_COARSE_Q(core));
		bcm_bprintf(b, "   core-%d: a/b: (%4d,%4d), d: (%3d,%3d),"
			" e: (%3d,%3d), f: (%3d,%3d)\n",
			core, (int16) ab_int[0], (int16) ab_int[1],
			(int8)((d_reg & 0xFF00) >> 8), /* di */
			(int8)((d_reg & 0x00FF)),      /* dq */
			(int8)(-((eir & 0xF0) >> 4) + ((eir & 0xF))), /* ei */
			(int8)(-((eqr & 0xF0) >> 4) + ((eqr & 0xF))), /* eq */
			(int8)(-((fir & 0xF0) >> 4) + ((fir & 0xF))), /* fi */
			(int8)(-((fqr & 0xF0) >> 4) + ((fqr & 0xF))));  /* fq */
	}
	bcm_bprintf(b, "Rx-IQ-Cal:\n");
	FOREACH_CORE(pi, core) {
		a_reg = (int16) phy_utils_read_phyreg(pi, HTPHY_RxIQCompA(core));
		b_reg = (int16) phy_utils_read_phyreg(pi, HTPHY_RxIQCompB(core));
		a_int = (a_reg >= 512) ? a_reg - 1024 : a_reg; /* s9 format */
		b_int = (b_reg >= 512) ? b_reg - 1024 : b_reg;
		bcm_bprintf(b, "   core-%d: a/b = (%4d,%4d)\n", core, a_int, b_int);
	}
	return;
}
#endif	/*  defined(BCMDBG) || defined(BCMDBG_DUMP) */

#if defined(DBG_BCN_LOSS)
void
wlc_phy_cal_dump_htphy_rx_min(phy_info_t *pi, struct bcmstrbuf *b)
{
	uint8 core;
	int16  a_reg, b_reg, a_int, b_int;
	bcm_bprintf(b, "Rx-IQ-Cal:\n");
	FOREACH_CORE(pi, core) {
		a_reg = (int16) phy_utils_read_phyreg(pi, HTPHY_RxIQCompA(core));
		b_reg = (int16) phy_utils_read_phyreg(pi, HTPHY_RxIQCompB(core));
		a_int = (a_reg >= 512) ? a_reg - 1024 : a_reg; /* s9 format */
		b_int = (b_reg >= 512) ? b_reg - 1024 : b_reg;
		bcm_bprintf(b, "   core-%d: a/b = (%4d,%4d)\n", core, a_int, b_int);
	}
}
#endif	/* DBG_BCN_LOSS */

/* see also: proc htphy_dac_adc_decouple_WAR80827 {cmd} */
static void
wlc_phy_cal_dac_adc_decouple_war_htphy(phy_info_t *pi, bool do_not_undo)
{
	/* do_not_undo: TRUE  - do/initiate the workaround
	 *              FALSE - undo/stop the workaround
	 *
	 * NOTE: attempting to initiate the WAR twice in succession
	 *       without stopping it will throw an assertion.
	 */

	phy_info_htphy_t *pi_ht = (phy_info_htphy_t *)pi->u.pi_htphy;
	htphy_dac_adc_decouple_war_t *pwar = &(pi_ht->ht_dac_adc_decouple_war_info);
	uint8 core;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	/* if not rev0, return immediately, WAR not applicable */
	if (!(HTREV_IS(pi->pubpi.phy_rev, 0))) return;

	if (do_not_undo) {
		/* start WAR */

		/* sanity check to avoid multiple calling to this proc */
		ASSERT(!pwar->is_on);
		pwar->is_on = TRUE;

		/* Disable PAPD Control bits */
		ASSERT(pi->pubpi.phy_corenum <= PHY_CORE_MAX);
		FOREACH_CORE(pi, core) {
			/* shift & enable */
			pwar->PapdCalShifts[core] =
			        phy_utils_read_phyreg(pi, HTPHY_PapdCalShifts(core));
			pwar->PapdEnable[core] = phy_utils_read_phyreg(pi, HTPHY_PapdEnable(core));
			phy_utils_write_phyreg(pi, HTPHY_PapdCalShifts(core), 0);
			phy_utils_write_phyreg(pi, HTPHY_PapdEnable(core), 0);
		}

		/* correlate, epsilon, cal-settle */
		pwar->PapdCalCorrelate = phy_utils_read_phyreg(pi, HTPHY_PapdCalCorrelate);
		pwar->PapdEpsilonUpdateIterations =
			phy_utils_read_phyreg(pi, HTPHY_PapdEpsilonUpdateIterations);
		pwar->PapdCalSettle = phy_utils_read_phyreg(pi, HTPHY_PapdCalSettle);
		phy_utils_write_phyreg(pi, HTPHY_PapdCalCorrelate, 0xfff);
		phy_utils_write_phyreg(pi, HTPHY_PapdEpsilonUpdateIterations, 0x1ff);
		phy_utils_write_phyreg(pi, HTPHY_PapdCalSettle, 0xfff);

		/* engage */
		phy_utils_write_phyreg(pi, HTPHY_PapdCalStart, 1);

	} else {
		/* stop WAR */
		uint8 sample_play_was_active = 0;

		/* sanity check to avoid multiple calling to this proc */
		ASSERT(pwar->is_on);
		pwar->is_on = FALSE;

		ASSERT(pi->pubpi.phy_corenum <= PHY_CORE_MAX);
		FOREACH_CORE(pi, core) {
			/* shift, enable */
			phy_utils_write_phyreg(pi, HTPHY_PapdCalShifts(core),
			                       pwar->PapdCalShifts[core]);
			phy_utils_write_phyreg(pi, HTPHY_PapdEnable(core), pwar->PapdEnable[core]);
		}
		/* correlate, epsilon, cal-settle, start */
		phy_utils_write_phyreg(pi, HTPHY_PapdCalCorrelate, pwar->PapdCalCorrelate);
		phy_utils_write_phyreg(pi, HTPHY_PapdEpsilonUpdateIterations,
			pwar->PapdEpsilonUpdateIterations);
		phy_utils_write_phyreg(pi, HTPHY_PapdCalSettle, pwar->PapdCalSettle);

		/* ensure that Sample Play is running (already or forcing it here);
		 * that way, the PAPD-cal clk is forced "on" and we can issue a
		 * successful reset
		 */
		if (READ_PHYREG(pi, HTPHY_sampleStatus, NormalPlay) == 0) {
			wlc_phy_tx_tone_htphy(pi, 0, 0, 0, 0, TRUE);
			sample_play_was_active = 0;
		} else {
			sample_play_was_active = 1;
		}

		/* do papd reset */
		MOD_PHYREG(pi, HTPHY_PapdCalAddress, papdReset, 1);
		MOD_PHYREG(pi, HTPHY_PapdCalAddress, papdReset, 0);

		/* turn tone off if we turned it on here */
		if (sample_play_was_active == 0) {
			wlc_phy_stopplayback_htphy(pi);
		}

		/* squash start bit (needed?) */
		phy_utils_write_phyreg(pi, HTPHY_PapdCalStart, 0);

	}

}

/* see also: proc htphy_rx_iq_est_loopback { {num_samps 2000} {wait_time ""} } */
static void
wlc_phy_rx_iq_est_loopback_htphy(phy_info_t *pi, phy_iq_est_t *est, uint16 num_samps,
                                 uint8 wait_time, uint8 wait_for_crs)
{
	/* same as rx iq-est, but checks if this is rev0 and enables hack
	 * for PR 80827 (ie, we decouple the bogus digital loopback from DAC
	 * to ADC, so we can see the actual desired RF loopback)
	 *
	 * -- note that by taking the resampler out of the Rx path with this PR's WAR,
	 *    we also disable possible I/Q flipping, so we need to do this is SW here
	 */

	uint16 flip_val = phy_utils_read_phyreg(pi, HTPHY_IQFlip);
	uint8 core;

	static const uint16 flip_masks[] = {
		HTPHY_IQFlip_adc1_MASK,
		HTPHY_IQFlip_adc2_MASK,
		HTPHY_IQFlip_adc3_MASK
	};
	static const uint16 flip_shifts[] = {
		HTPHY_IQFlip_adc1_SHIFT,
		HTPHY_IQFlip_adc2_SHIFT,
		HTPHY_IQFlip_adc3_SHIFT
	};

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	wlc_phy_cal_dac_adc_decouple_war_htphy(pi, TRUE);
	wlc_phy_rx_iq_est_htphy(pi, est, num_samps, wait_time, wait_for_crs);
	wlc_phy_cal_dac_adc_decouple_war_htphy(pi, FALSE);

	ASSERT(pi->pubpi.phy_corenum <= PHY_CORE_MAX);
	FOREACH_CORE(pi, core) {
		if (((flip_val & flip_masks[core]) >> flip_shifts[core]) == 1) {
			uint32 true_qq = est[core].i_pwr;
			est[core].i_pwr = est[core].q_pwr;
			est[core].q_pwr = true_qq;
		}
	}
}

static int
wlc_phy_calc_ab_from_iq_htphy(phy_iq_est_t *est, phy_iq_comp_t *comp)
{
	int16  iq_nbits, qq_nbits, brsh, arsh;
	int32  iq = est->iq_prod;
	uint32 ii = est->i_pwr;
	uint32 qq = est->q_pwr;
	int32  a, b, temp;

	/* for each core, implement the following floating point
	 * operations quantized to 10 fractional bits:
	 *   a = -iq/ii
	 *   b = -1 + sqrt(qq/ii - a*a)
	 */

	/* ---- BEGIN a,b fixed point computation ---- */

	iq_nbits = phy_utils_nbits(iq);
	qq_nbits = phy_utils_nbits(qq);

	/*   a = -iq/ii */
	arsh = 10-(30-iq_nbits);
	if (arsh >= 0) {
		a = (-(iq << (30 - iq_nbits)) + (ii >> (1 + arsh)));
		temp = (int32) (ii >>  arsh);
		if (temp == 0) {
			PHY_ERROR(("Aborting Rx IQCAL! ii=%d, arsh=%d\n", ii, arsh));
			return BCME_ERROR;
		}
	} else {
		a = (-(iq << (30 - iq_nbits)) + (ii << (-1 - arsh)));
		temp = (int32) (ii << -arsh);
		if (temp == 0) {
			PHY_ERROR(("Aborting Rx IQCAL! ii=%d, arsh=%d\n", ii, arsh));
			return BCME_ERROR;
		}
	}
	a /= temp;

	/*   b = -1 + sqrt(qq/ii - a*a) */
	brsh = qq_nbits-31+20;
	if (brsh >= 0) {
		b = (qq << (31-qq_nbits));
		temp = (int32) (ii >>  brsh);
		if (temp == 0) {
			PHY_ERROR(("Aborting Rx IQCAL! ii=%d, brsh=%d\n", ii, brsh));
			return BCME_ERROR;
		}
	} else {
		b = (qq << (31-qq_nbits));
		temp = (int32) (ii << -brsh);
		if (temp == 0) {
			PHY_ERROR(("Aborting Rx IQCAL! ii=%d, brsh=%d\n", ii, brsh));
			return BCME_ERROR;
		}
	}
	b /= temp;
	b -= a*a;
	b = (int32) phy_utils_sqrt_int((uint32) b);
	b -= (1 << 10);

	comp->a = a & 0x3ff;
	comp->b = b & 0x3ff;

	/* ---- END a,b fixed point computation ---- */

	return BCME_OK;
}

/* see also: proc htphy_rx_iq_cal_calc_coeffs { {num_samps 0x4000} {cores} } */
static void
wlc_phy_calc_rx_iq_comp_htphy(phy_info_t *pi, uint16 num_samps, uint8 core_mask)
{
	phy_iq_comp_t coeffs[PHY_CORE_MAX];
	phy_iq_est_t  rx_imb_vals[PHY_CORE_MAX];
#if defined(BCMDBG)
	phy_iq_est_t  noise_vals[PHY_CORE_MAX];
	uint16 bbmult_orig[PHY_CORE_MAX], bbmult_zero = 0;
#endif // endif
	uint8 core;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	/* save coefficients */
	ASSERT(pi->pubpi.phy_corenum <= PHY_CORE_MAX);
	FOREACH_CORE(pi, core) {
		wlc_phy_rx_iq_comp_htphy(pi, 0, &(coeffs[core]), core);
	}

	/* get iq, ii, qq measurements from iq_est */
	wlc_phy_rx_iq_est_loopback_htphy(pi, rx_imb_vals, num_samps, 32, 0);

#if defined(BCMDBG)
	/* take noise measurement (for SNR calc, for information purposes only) */
	FOREACH_CORE(pi, core) {
		wlc_phy_get_tx_bbmult(pi, &(bbmult_orig[core]), core);
		wlc_phy_set_tx_bbmult(pi, &bbmult_zero, core);
	}
	wlc_phy_rx_iq_est_loopback_htphy(pi, noise_vals, num_samps, 32, 0);
	FOREACH_CORE(pi, core) {
		wlc_phy_set_tx_bbmult(pi, &(bbmult_orig[core]), core);
	}
#endif // endif

	/* calculate coeffs on requested cores */
	FOREACH_ACTV_CORE(pi, core_mask, core) {

		/* reset coeffs */
		wlc_phy_rx_iq_comp_htphy(pi, 2, NULL, core);

		/* bounds check estimate info */

		/* calculate coefficients, map to 10 bit and apply */
		wlc_phy_calc_ab_from_iq_htphy(&(rx_imb_vals[core]), &(coeffs[core]));

#if defined(BCMDBG)
		/* Calculate SNR for debug */

		/* FIXME: need to port a convenient lin-to-dB operation;
		 * barring that, just report unnorm sig power, noise power,
		 * and samp count.
		 */

		PHY_NONE(("wlc_phy_calc_rx_iq_comp_htphy: core%d => "
			"(S =%9d,  N =%9d,  K =%d)\n",
			core,
			rx_imb_vals[core].i_pwr + rx_imb_vals[core].q_pwr,
			noise_vals[core].i_pwr + noise_vals[core].q_pwr,
			num_samps));
#endif // endif
	}

	/* apply coeffs */
	FOREACH_CORE(pi, core) {
		wlc_phy_rx_iq_comp_htphy(pi, 1, &(coeffs[core]), core);
	}

}

static void
wlc_phy_rxcal_radio_config_htphy(phy_info_t *pi, bool setup_not_cleanup, uint8 rx_core)
{
	/* XXX nowadays have setup and cleanup in one proc, to ease
	 * consistency between the two
	 */

	phy_info_htphy_t *pi_ht = (phy_info_htphy_t *)pi->u.pi_htphy;
	htphy_rxcal_radioregs_t *porig = &(pi_ht->ht_rxcal_radioregs_orig);

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	ASSERT(rx_core < PHY_CORE_MAX);

	if (setup_not_cleanup) {
		/* XXX
		 *-------------
		 * Radio Setup
		 *-------------
		 */

		ASSERT(!porig->is_orig);
		porig->is_orig = TRUE;

		porig->RF_TX_txrxcouple_2g_pwrup[rx_core] =
			phy_utils_read_radioreg(pi, RADIO_2059_TX_TXRXCOUPLE_2G_PWRUP(rx_core));
		porig->RF_TX_txrxcouple_2g_atten[rx_core] =
			phy_utils_read_radioreg(pi, RADIO_2059_TX_TXRXCOUPLE_2G_ATTEN(rx_core));
		porig->RF_TX_txrxcouple_5g_pwrup[rx_core] =
			phy_utils_read_radioreg(pi, RADIO_2059_TX_TXRXCOUPLE_5G_PWRUP(rx_core));
		porig->RF_TX_txrxcouple_5g_atten[rx_core] =
			phy_utils_read_radioreg(pi, RADIO_2059_TX_TXRXCOUPLE_5G_ATTEN(rx_core));
		porig->RF_afe_vcm_cal_master[rx_core] =
			phy_utils_read_radioreg(pi, RADIO_2059_AFE_VCM_CAL_MASTER(rx_core));
		porig->RF_afe_set_vcm_i[rx_core] =
			phy_utils_read_radioreg(pi, RADIO_2059_AFE_SET_VCM_I(rx_core));
		porig->RF_afe_set_vcm_q[rx_core] =
			phy_utils_read_radioreg(pi, RADIO_2059_AFE_SET_VCM_Q(rx_core));
		porig->RF_rxbb_vgabuf_idacs[rx_core] =
			phy_utils_read_radioreg(pi, RADIO_2059_RXBB_VGABUF_IDACS(rx_core));
		porig->RF_rxbuf_degen[rx_core] =
			phy_utils_read_radioreg(pi, RADIO_2059_RXBUF_DEGEN(rx_core));

		/* XXX FIXME (from mimophy rev7):
		 *   - set the IQCAL loopback attenuation to 7 (JH: in 2G band? and in 5G?)
		 *   - will need to revisit once Tx chain is stable
		 *   - consider changing the preferred value in .xls
		 * 0. LPF is set in Rx mode in the phy_setup
		 * 1. LNA1 has been turned off via rfctrl override in phy_setup
		 * 2. turn Tx--Rx coupling ON
		 */
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			phy_utils_write_radioreg(pi, RADIO_2059_TX_TXRXCOUPLE_2G_PWRUP(rx_core),
			                         0x03);
			phy_utils_write_radioreg(pi, RADIO_2059_TX_TXRXCOUPLE_2G_ATTEN(rx_core),
			                         0x7f);
		} else {
			phy_utils_write_radioreg(pi, RADIO_2059_TX_TXRXCOUPLE_5G_PWRUP(rx_core),
			                         0x03);
			phy_utils_write_radioreg(pi, RADIO_2059_TX_TXRXCOUPLE_5G_ATTEN(rx_core),
			                         0x0f);
		}

		/* XXX WAR to overcome BB oscillation (PR 81998)
		 *   see also corresponding section in the rx cal phy config!
		 *   - see http://hwnbu-twiki.broadcom.com/bin/view/Mwgroup/BCM4331A0LabNotebook51)
		 *   - FIXME: condition this on radio rev some day??
		 */

		phy_utils_mod_radioreg(pi, RADIO_2059_AFE_VCM_CAL_MASTER(rx_core), 0x2, 0x0);
		phy_utils_write_radioreg(pi, RADIO_2059_AFE_SET_VCM_I(rx_core), 0x27);
		phy_utils_write_radioreg(pi, RADIO_2059_AFE_SET_VCM_Q(rx_core), 0x27);

		phy_utils_write_radioreg(pi, RADIO_2059_RXBB_VGABUF_IDACS(rx_core), 0x24);

		phy_utils_write_radioreg(pi, RADIO_2059_RXBUF_DEGEN(rx_core), 0x2);

	} else {
		/* XXX
		 *---------------
		 * Radio Cleanup
		 *---------------
		 */

		ASSERT(porig->is_orig);
		porig->is_orig = FALSE;

		phy_utils_write_radioreg(pi, RADIO_2059_TX_TXRXCOUPLE_2G_PWRUP(rx_core),
		                porig->RF_TX_txrxcouple_2g_pwrup[rx_core]);
		phy_utils_write_radioreg(pi, RADIO_2059_TX_TXRXCOUPLE_2G_ATTEN(rx_core),
		                porig->RF_TX_txrxcouple_2g_atten[rx_core]);
		phy_utils_write_radioreg(pi, RADIO_2059_TX_TXRXCOUPLE_5G_PWRUP(rx_core),
		                porig->RF_TX_txrxcouple_5g_pwrup[rx_core]);
		phy_utils_write_radioreg(pi, RADIO_2059_TX_TXRXCOUPLE_5G_ATTEN(rx_core),
		                porig->RF_TX_txrxcouple_5g_atten[rx_core]);
		phy_utils_write_radioreg(pi, RADIO_2059_AFE_VCM_CAL_MASTER(rx_core),
		                porig->RF_afe_vcm_cal_master[rx_core]);
		phy_utils_write_radioreg(pi, RADIO_2059_AFE_SET_VCM_I(rx_core),
		                porig->RF_afe_set_vcm_i[rx_core]);
		phy_utils_write_radioreg(pi, RADIO_2059_AFE_SET_VCM_Q(rx_core),
		                porig->RF_afe_set_vcm_q[rx_core]);
		phy_utils_write_radioreg(pi, RADIO_2059_RXBB_VGABUF_IDACS(rx_core),
		                porig->RF_rxbb_vgabuf_idacs[rx_core]);
		phy_utils_write_radioreg(pi, RADIO_2059_RXBUF_DEGEN(rx_core),
		                porig->RF_rxbuf_degen[rx_core]);
	}

}

static void
wlc_phy_rxcal_phy_config_htphy(phy_info_t *pi, bool setup_not_cleanup, uint8 rx_core)
{
	/* XXX Notes:
	 *
	 * (1) nowadays have setup & cleanup in one proc to ease consistency b/w the two
	 *
	 * (2) Reminder on why we turn off the resampler
	 *   - quick reminder of the motivation (on 4322): Turning on SamplePlay
	 *     does/did not automatically enable the resampler clocks in Spur Channels
	 *     (this was a HW bug); therefore, we have two choices: either,
	 *         (A) disable resampling and accept that the tone freqs will be slightly off
	 *     from what the tone_freq arguments indicate (who cares)
	 *         (B) turn on forcegatedclock when SamplePlay is active to enforce the
	 *    Tx resampler to be active despite the HW bug
	 *    => Currently, we use (A). On chips >4322, supposedly SamplePlay now enables
	 *    the right clocks, but we maintain the old approach regardless for now
	 *
	 *   - also note that in the driver we do a resetCCA after this to be on the safe
	 *     side; may want to revisit this here, too, in case we run into issues
	 */

	phy_info_htphy_t *pi_ht = (phy_info_htphy_t *)pi->u.pi_htphy;
	htphy_rxcal_phyregs_t *porig = &(pi_ht->ht_rxcal_phyregs_orig);
	uint8 core;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	ASSERT(rx_core < PHY_CORE_MAX);

	if (setup_not_cleanup) {
		/* XXX
		 *-------------
		 * Phy "Setup"
		 *-------------
		 */

		ASSERT(!porig->is_orig);
		porig->is_orig = TRUE;

		/* XXX Disable the Tx and Rx re-samplers (if enabled)
		 *   - relevant only if spur avoidance is ON;
		 *   - see also comment outsourced to the end of this proc
		 */
		porig->BBConfig = phy_utils_read_phyreg(pi, HTPHY_BBConfig);
		MOD_PHYREG(pi, HTPHY_BBConfig, resample_clk160, 0);

		/* XXX FIXME: even though this is called before calibrating each individual core, we
		 * must store all N cores tx gain state for later restoration, because
		 * wlc_phy_rxcal_gainctrl_htphy() is sloppy in modifying all 3 cores' tx gain info
		 * when calibrating any one core.  clean this up when we can clean up the gainctrl
		 * function.
		 */
		FOREACH_CORE(pi, core) {
			wlc_phy_get_tx_bbmult(pi, &(porig->bbmult[core]), core);
		}
		wlc_phy_table_read_htphy(pi, HTPHY_TBL_ID_RFSEQ, pi->pubpi.phy_corenum, 0x110, 16,
		                         porig->rfseq_txgain);

		porig->Afectrl[rx_core] = phy_utils_read_phyreg(pi, HTPHY_Afectrl(rx_core));
		porig->AfectrlOverride[rx_core] =
		        phy_utils_read_phyreg(pi, HTPHY_AfectrlOverride(rx_core));
		MOD_PHYREGC(pi, HTPHY_Afectrl, rx_core, adc_pd, 0);
		MOD_PHYREGC(pi, HTPHY_AfectrlOverride, rx_core, adc_pd, 1);

		/* XXX WAR to overcome BB oscillation (PR 81998)
		 * power down aux-adc to "save current" (reduce IR drop)
		 * - see also corresponding section in the rx cal radio config!
		 * - see http://hwnbu-twiki.broadcom.com/bin/view/Mwgroup/BCM4331A0LabNotebook51)
		 * - state storing is taken care of, because override Enable/Value states for both
		 * AFE and RFctrl suitably stored above and cleaned up in cleanup section
		 */
		MOD_PHYREGC(pi, HTPHY_Afectrl, rx_core, rssi_pd, 1);
		MOD_PHYREGC(pi, HTPHY_AfectrlOverride, rx_core, rssi_pd, 1);

		/* XXX Core Activate/Deactivate
		 *   - for now, keep all rx's enabled for most realistic rx conditions
		 *   - may revisit later JH FIXME
		 */
		porig->RfseqCoreActv2059 = phy_utils_read_phyreg(pi, HTPHY_RfseqCoreActv2059);
		MOD_PHYREG(pi, HTPHY_RfseqCoreActv2059, DisRx, 0);
		MOD_PHYREG(pi, HTPHY_RfseqCoreActv2059, EnTx, (1 << rx_core));

		/* XXX RF External Settings
		 *   - ext_pa off if applic,
		 *   - T/R on T to protect against interference
		 *   - FIXME: make this more efficient by combining all this into one regw???
		 */
		porig->RfctrlIntc[rx_core] = phy_utils_read_phyreg(pi, HTPHY_RfctrlIntc(rx_core));
		MOD_PHYREGC(pi, HTPHY_RfctrlIntc, rx_core, ext_2g_papu, 0);
		MOD_PHYREGC(pi, HTPHY_RfctrlIntc, rx_core, ext_5g_papu, 0);
		MOD_PHYREGC(pi, HTPHY_RfctrlIntc, rx_core, override_ext_pa, 1);
		MOD_PHYREGC(pi, HTPHY_RfctrlIntc, rx_core, tr_sw_tx_pu, 1);
		MOD_PHYREGC(pi, HTPHY_RfctrlIntc, rx_core, tr_sw_rx_pu, 0);
		MOD_PHYREGC(pi, HTPHY_RfctrlIntc, rx_core, override_tr_sw, 1);

		/* XXX RfCtrl
		 *   - turn off Internal PA
		 *   - turn off LNA1 to protect against interference and reduce thermal noise
		 *   - force LPF to Rx Chain
		 *   - force LPF bw
		 *   - NOTE: this also saves off state of possible Tx/Rx gain override states
		 */

		porig->RfctrlCorePU[rx_core] =
			phy_utils_read_phyreg(pi, HTPHY_RfctrlCorePU(rx_core));
		porig->RfctrlOverride[rx_core] =
			phy_utils_read_phyreg(pi, HTPHY_RfctrlOverride(rx_core));
		porig->RfctrlCoreLpfCT[rx_core] =
			phy_utils_read_phyreg(pi, HTPHY_RfctrlCoreLpfCT(rx_core));
		porig->RfctrlOverrideLpfCT[rx_core] =
			phy_utils_read_phyreg(pi, HTPHY_RfctrlOverrideLpfCT(rx_core));
		porig->RfctrlCoreLpfPU[rx_core] =
			phy_utils_read_phyreg(pi, HTPHY_RfctrlCoreLpfPU(rx_core));
		porig->RfctrlOverrideLpfPU[rx_core] =
			phy_utils_read_phyreg(pi, HTPHY_RfctrlOverrideLpfPU(rx_core));

		MOD_PHYREGC(pi, HTPHY_RfctrlCorePU, rx_core, intpa_pu, 0);
		MOD_PHYREGC(pi, HTPHY_RfctrlCorePU, rx_core, lna1_pu, 0);
		MOD_PHYREGC(pi, HTPHY_RfctrlOverride, rx_core, intpa_pu, 1);
		MOD_PHYREGC(pi, HTPHY_RfctrlOverride, rx_core, lna1_pu, 1);

		MOD_PHYREGC(pi, HTPHY_RfctrlCoreLpfCT, rx_core, lpf_byp_rx, 0);
		MOD_PHYREGC(pi, HTPHY_RfctrlCoreLpfCT, rx_core, lpf_byp_tx, 1);
		MOD_PHYREGC(pi, HTPHY_RfctrlCoreLpfCT, rx_core, lpf_sel_txrx, 0);
		MOD_PHYREGC(pi, HTPHY_RfctrlCoreLpfCT, rx_core, lpf_bw_ctl,
		            (CHSPEC_IS40(pi->radio_chanspec) ? 2 : 0));
		MOD_PHYREGC(pi, HTPHY_RfctrlOverrideLpfCT, rx_core, lpf_byp_rx, 1);
		MOD_PHYREGC(pi, HTPHY_RfctrlOverrideLpfCT, rx_core, lpf_byp_tx, 1);
		MOD_PHYREGC(pi, HTPHY_RfctrlOverrideLpfCT, rx_core, lpf_sel_txrx, 1);
		MOD_PHYREGC(pi, HTPHY_RfctrlOverrideLpfCT, rx_core, lpf_bw_ctl, 1);

		MOD_PHYREGC(pi, HTPHY_RfctrlCoreLpfPU, rx_core, dc_loop_pu, 1);
		MOD_PHYREGC(pi, HTPHY_RfctrlCoreLpfPU, rx_core, lpf_rx_buf_pu, 1);
		MOD_PHYREGC(pi, HTPHY_RfctrlOverrideLpfPU, rx_core, dc_loop_pu, 1);
		MOD_PHYREGC(pi, HTPHY_RfctrlOverrideLpfPU, rx_core, lpf_rx_buf_pu, 1);

		porig->PapdEnable[rx_core] = phy_utils_read_phyreg(pi, HTPHY_PapdEnable(rx_core));
		MOD_PHYREGC(pi, HTPHY_PapdEnable, rx_core, papd_compEnb0, 0);

	} else {
		/* XXX
		 *---------------
		 * Phy "Cleanup"
		 *---------------
		 */

		ASSERT(porig->is_orig);
		porig->is_orig = FALSE;

		phy_utils_write_phyreg(pi, HTPHY_BBConfig, porig->BBConfig);

		/* XXX PR85342 When entering into Spur-Avoidance Mode (On or On2)
		 * cleanup to avoid PHY hang in PAYDECODE
		 */
		wlc_phy_resetcca_htphy(pi);

		FOREACH_CORE(pi, core) {
			wlc_phy_set_tx_bbmult(pi, &(porig->bbmult[core]), core);
		}
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_RFSEQ, pi->pubpi.phy_corenum, 0x110, 16,
		                         porig->rfseq_txgain);
		phy_utils_write_phyreg(pi, HTPHY_Afectrl(rx_core), porig->Afectrl[rx_core]);
		phy_utils_write_phyreg(pi, HTPHY_AfectrlOverride(rx_core),
		                       porig->AfectrlOverride[rx_core]);
		phy_utils_write_phyreg(pi, HTPHY_RfseqCoreActv2059, porig->RfseqCoreActv2059);
		phy_utils_write_phyreg(pi, HTPHY_RfctrlIntc(rx_core), porig->RfctrlIntc[rx_core]);
		phy_utils_write_phyreg(pi, HTPHY_RfctrlCorePU(rx_core),
		                       porig->RfctrlCorePU[rx_core]);
		phy_utils_write_phyreg(pi, HTPHY_RfctrlOverride(rx_core),
		                       porig->RfctrlOverride[rx_core]);
		phy_utils_write_phyreg(pi, HTPHY_RfctrlCoreLpfCT(rx_core),
		                       porig->RfctrlCoreLpfCT[rx_core]);
		phy_utils_write_phyreg(pi, HTPHY_RfctrlOverrideLpfCT(rx_core),
			porig->RfctrlOverrideLpfCT[rx_core]);
		phy_utils_write_phyreg(pi, HTPHY_RfctrlCoreLpfPU(rx_core),
		                       porig->RfctrlCoreLpfPU[rx_core]);
		phy_utils_write_phyreg(pi, HTPHY_RfctrlOverrideLpfPU(rx_core),
			porig->RfctrlOverrideLpfPU[rx_core]);
		phy_utils_write_phyreg(pi, HTPHY_PapdEnable(rx_core), porig->PapdEnable[rx_core]);

		wlc_phy_force_rfseq_htphy(pi, HTPHY_RFSEQ_RESET2RX);
	}
}

#define HTPHY_RXCAL_TONEAMP 181
#define HTPHY_RXCAL_TONEFREQ_40MHz 4000
#define HTPHY_RXCAL_TONEFREQ_20MHz 2000
#define HTPHY_RXCAL_NUMGAINS 6

typedef struct _htphy_rxcal_txrxgain {
	uint16 lpf_biq1;
	uint16 lpf_biq0;
	uint16 lna2;
	int8 txpwrindex;
} htphy_rxcal_txrxgain_t;

enum {
	HTPHY_RXCAL_GAIN_INIT = 0,
	HTPHY_RXCAL_GAIN_UP,
	HTPHY_RXCAL_GAIN_DOWN
};
static void
wlc_phy_set_rfseq_htphy(phy_info_t *pi, uint8 cmd, uint8 *events, uint8 *dlys, uint8 len)
{
	uint32 t1_offset, t2_offset;
	uint8 ctr;
	uint8 end_event = HTPHY_RFSEQ_CMD_END;
	uint8 end_dly   = 1;

	ASSERT(len <= 16);

	t1_offset = cmd << 4;
	wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_RFSEQ, len, t1_offset, 8, events);
	t2_offset = t1_offset + 0x080;
	wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_RFSEQ, len, t2_offset, 8, dlys);

	for (ctr = len; ctr < 16; ctr++) {
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_RFSEQ, 1,
		                          t1_offset + ctr, 8, &end_event);
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_RFSEQ, 1,
		                          t2_offset + ctr, 8, &end_dly);
	}

}
/* see also: proc htphy_rx_iq_cal_txrxgain_setup { core } */
static void
wlc_phy_rxcal_gainctrl_htphy(phy_info_t *pi, uint8 rx_core)
{
	/*
	 * joint tx-rx gain control for Rx IQ calibration
	 */

	/* gain candidates tables,
	 * columns are: B1 B0 L2 Tx-Pwr-Idx
	 * rows are monotonically increasing gain
	 */
	htphy_rxcal_txrxgain_t gaintbl_5G[HTPHY_RXCAL_NUMGAINS] =
		{
		{0, 0, 0, 100},
		{0, 0, 0,  50},
		{0, 0, 0,  -1},
		{0, 0, 3,  -1},
		{0, 3, 3,  -1},
		{0, 5, 3,  -1}
		};
	htphy_rxcal_txrxgain_t gaintbl_2G[HTPHY_RXCAL_NUMGAINS] =
	        {
		{0, 0, 0,  10},
		{0, 0, 1,  10},
		{0, 1, 2,  10},
		{0, 1, 3,  10},
		{0, 4, 3,  10},
		{0, 6, 3,  10}
		};
	uint32 gain_bits_tbl_ids[] =
	        {HTPHY_TBL_ID_GAINBITS1, HTPHY_TBL_ID_GAINBITS2, HTPHY_TBL_ID_GAINBITS3};
	uint16 num_samps = 1024;
	phy_iq_est_t est[PHY_CORE_MAX];
	/* threshold for "too high power"(313 mVpk, where clip = 400mVpk in 4322) */
	uint32 i_pwr, q_pwr, curr_pwr, optim_pwr = 0, prev_pwr = 0, thresh_pwr = 10000;
	/* desired_log2_pwr for gain fine adjustment in the end */
	int16 desired_log2_pwr = 13, actual_log2_pwr, delta_pwr;
	bool gainctrl_done = FALSE;
	/* max A-band Tx gain code (does not contain TxGm bits - injected below) */
	uint16 mix_tia_gain_idx, mix_tia_gain = 0;
	int8 optim_gaintbl_index = 0, prev_gaintbl_index = 0;
	int8 curr_gaintbl_index = 3;
	uint8 gainctrl_dirn = HTPHY_RXCAL_GAIN_INIT;
	htphy_rxcal_txrxgain_t *gaintbl;
	uint16 lpf_biq1_gain, lpf_biq0_gain, lna2_gain;
	int16 fine_gain_idx;
	int8 txpwrindex;
	uint16 txgain_max_a = 0x0ff8;
	txgain_setting_t txgain_setting_tmp;
	uint16 txgain_txgm_val, txgain_code;
	uint8 core;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	ASSERT(rx_core < PHY_CORE_MAX);

	/* FIXME: save / zero-out and later restore rx_iq_comp coeffs??;
	 * done in mimophy driver but not in tcl
	 */

	/* retrieve Rx Mixer/TIA gain from InitGain and via GainBits table */
	mix_tia_gain_idx = READ_PHYREG(pi, HTPHY_Core0InitGainCodeA2059, initmixergainIndex);
	wlc_phy_table_read_htphy(pi, gain_bits_tbl_ids[rx_core], 1, (0x20 + mix_tia_gain_idx), 16,
	                         &mix_tia_gain);

	/* Retrieve Tx Mixer/Gm gain (so can inject below since this needs to be
	 * invariant) -- (extract from Tx gain table)
	 */
	wlc_phy_get_txgain_settings_by_index(pi, &txgain_setting_tmp, 50);
	/* this is the bit-shifted TxGm gain code */
	txgain_txgm_val = txgain_setting_tmp.rad_gain & 0x7000;

	/* gain candidates
	 *
	 * -- intPA assumed to be turned off earlier in calibration setups
	 * FIXME-->
	 * -- invoking tx_tone will reset bb_mult to default values dep on BW (20/40),
	 *      ie change whatever pwr_by_index chose; this is known and o.k. at this point;
	 *      the index based approach is used to get valid Tx gain codes and is fairly
	 *      coarse to begin with
	 * -- default tx index for G-band was chosen such that row#3 (start index)
	 *      gave up to (close to) 1000 in pwr in "typical lab scenario" (really? FIXME)
	 * -- do we want to add low-end entries in 2G that crank up Tx-index, to be on safe side?
	 */
	if (CHSPEC_IS5G(pi->radio_chanspec)) {
		gaintbl = gaintbl_5G;
	} else {
		gaintbl = gaintbl_2G;
	}

	/*
	 * COARSE ADJUSTMENT: Gain Search Loop
	 */
	do {
		/* set coarse gains */

		/* biq1 should be zero for REV7+ (see above) since it's used for fine AGC */
		/* lna1 is powered down, so code shouldn't matter but implicitly set to 0. */
		lpf_biq1_gain   = gaintbl[curr_gaintbl_index].lpf_biq1;
		lpf_biq0_gain   = gaintbl[curr_gaintbl_index].lpf_biq0;
		lna2_gain       = gaintbl[curr_gaintbl_index].lna2;
		txpwrindex      = gaintbl[curr_gaintbl_index].txpwrindex;

		/* rx */
		phy_utils_write_phyreg(pi, HTPHY_RfctrlRXGAIN(rx_core),
			(mix_tia_gain << 4) | (lna2_gain << 2));
		phy_utils_write_phyreg(pi, HTPHY_Rfctrl_lpf_gain(rx_core),
			(lpf_biq1_gain << 4) | lpf_biq0_gain);
		MOD_PHYREGC(pi, HTPHY_RfctrlOverride, rx_core, rxgain, 1);
		MOD_PHYREGC(pi, HTPHY_RfctrlOverride, rx_core, lpf_gain_biq0, 1);
		MOD_PHYREGC(pi, HTPHY_RfctrlOverride, rx_core, lpf_gain_biq1, 1);

		/* tx */
		FOREACH_CORE(pi, core) {
			if (txpwrindex == -1) {
				/* inject default TxGm value from above */
				txgain_code = txgain_max_a | txgain_txgm_val;
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_RFSEQ,
					1, 0x110 + core, 16, &txgain_code);
			} else {
				wlc_phy_txpwr_by_index_htphy(pi, (1 << core), txpwrindex);
			}
		}

		/* turn on testtone (this will override bbmult, but that's ok) */
		wlc_phy_tx_tone_htphy(pi, (CHSPEC_IS40(pi->radio_chanspec)) ?
		                      HTPHY_RXCAL_TONEFREQ_40MHz : HTPHY_RXCAL_TONEFREQ_20MHz,
		                      HTPHY_RXCAL_TONEAMP, 0, 0, FALSE);

		/* estimate digital power using rx_iq_est (using loopbk version in case
		 * PR 80827 needs to be worked around)
		 */
		wlc_phy_rx_iq_est_loopback_htphy(pi, est, num_samps, 32, 0);
		i_pwr = (est[rx_core].i_pwr + num_samps / 2) / num_samps;
		q_pwr = (est[rx_core].q_pwr + num_samps / 2) / num_samps;
		curr_pwr = i_pwr + q_pwr;
		PHY_NONE(("core %u (gain idx %d): i_pwr = %u, q_pwr = %u, curr_pwr = %d\n",
		         rx_core, curr_gaintbl_index, i_pwr, q_pwr, curr_pwr));

		switch (gainctrl_dirn) {
		case HTPHY_RXCAL_GAIN_INIT:
			if (curr_pwr > thresh_pwr) {
				gainctrl_dirn = HTPHY_RXCAL_GAIN_DOWN;
				prev_gaintbl_index = curr_gaintbl_index;
				curr_gaintbl_index--;
			} else {
				gainctrl_dirn = HTPHY_RXCAL_GAIN_UP;
				prev_gaintbl_index = curr_gaintbl_index;
				curr_gaintbl_index++;
			}
			break;

		case HTPHY_RXCAL_GAIN_UP:
			if (curr_pwr > thresh_pwr) {
				gainctrl_done = TRUE;
				optim_pwr = prev_pwr;
				optim_gaintbl_index = prev_gaintbl_index;
			} else {
				prev_gaintbl_index = curr_gaintbl_index;
				curr_gaintbl_index++;
			}
			break;

		case HTPHY_RXCAL_GAIN_DOWN:
			if (curr_pwr > thresh_pwr) {
				prev_gaintbl_index = curr_gaintbl_index;
				curr_gaintbl_index--;
			} else {
				gainctrl_done = TRUE;
				optim_pwr = curr_pwr;
				optim_gaintbl_index = curr_gaintbl_index;
			}
			break;

		default:
			PHY_ERROR(("Invalid gaintable direction id %d\n", gainctrl_dirn));
			ASSERT(0);
		}

		if ((curr_gaintbl_index < 0) ||
		    (curr_gaintbl_index >= HTPHY_RXCAL_NUMGAINS)) {
			gainctrl_done = TRUE;
			optim_pwr = curr_pwr;
			optim_gaintbl_index = prev_gaintbl_index;
		} else {
			prev_pwr = curr_pwr;
		}

		/* Turn off the tone */
		wlc_phy_stopplayback_htphy(pi);

	} while (!gainctrl_done);

	/* fetch back the chosen gain values resulting from the coarse gain search */
	lpf_biq1_gain   = gaintbl[optim_gaintbl_index].lpf_biq1;
	lpf_biq0_gain   = gaintbl[optim_gaintbl_index].lpf_biq0;
	lna2_gain       = gaintbl[optim_gaintbl_index].lna2;
	txpwrindex      = gaintbl[optim_gaintbl_index].txpwrindex;

	/*
	 * FINE ADJUSTMENT: adjust Biq1 to get on target
	 */

	/* get measured power in log2 domain
	 * (log domain value is larger or equal to lin value)
	 * Compute power delta in log domain
	 */
	actual_log2_pwr = phy_utils_nbits(optim_pwr);
	delta_pwr = desired_log2_pwr - actual_log2_pwr;

	fine_gain_idx = (int)lpf_biq1_gain + delta_pwr;
	/* Limit Total LPF To 30 dB */
	if (fine_gain_idx + (int)lpf_biq0_gain > 10) {
		lpf_biq1_gain = 10 - lpf_biq0_gain;
	} else {
		lpf_biq1_gain = (uint16) MAX(fine_gain_idx, 0);
	}

	/* set fine gains */

	/* rx */
	phy_utils_write_phyreg(pi, HTPHY_RfctrlRXGAIN(rx_core),
	                       (mix_tia_gain << 4) | (lna2_gain << 2));
	phy_utils_write_phyreg(pi, HTPHY_Rfctrl_lpf_gain(rx_core),
	                       (lpf_biq1_gain << 4) | lpf_biq0_gain);
	MOD_PHYREGC(pi, HTPHY_RfctrlOverride, rx_core, rxgain, 1);
	MOD_PHYREGC(pi, HTPHY_RfctrlOverride, rx_core, lpf_gain_biq0, 1);
	MOD_PHYREGC(pi, HTPHY_RfctrlOverride, rx_core, lpf_gain_biq1, 1);

	/* tx */
	FOREACH_CORE(pi, core) {
		if (txpwrindex == -1) {
			/* inject default TxGm value from above */
			txgain_code = txgain_max_a | txgain_txgm_val;
			wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_RFSEQ, 1, 0x110 + core, 16,
			                          &txgain_code);
		} else {
			wlc_phy_txpwr_by_index_htphy(pi, (1 << core), txpwrindex);
		}
	}

	PHY_NONE(("FINAL: gainIdx=%3d, lna1=OFF, lna2=%3d, mix_tia=%3d, "
	         "lpf0=%3d, lpf1=%3d, txpwridx=%3d\n",
	         optim_gaintbl_index, lna2_gain, mix_tia_gain,
	         lpf_biq0_gain, lpf_biq1_gain, txpwrindex));
}

/* see also: proc htphy_rx_iq_cal {} */
static int
wlc_phy_cal_rxiq_htphy(phy_info_t *pi, bool debug)
{
	uint8 core;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	/* Make sure we start in RX state and are "deaf" */
	wlc_phy_stay_in_carriersearch_htphy(pi, TRUE);
	wlc_phy_force_rfseq_htphy(pi, HTPHY_RFSEQ_RESET2RX);

	/* Zero Out coefficients */
	FOREACH_CORE(pi, core) {
		wlc_phy_rx_iq_comp_htphy(pi, 2, NULL, core);
	}

	/* Master Loop */
	FOREACH_CORE(pi, core) {
		wlc_phy_rxcal_phy_config_htphy(pi, TRUE, core);
		wlc_phy_rxcal_radio_config_htphy(pi, TRUE, core);
		wlc_phy_rxcal_gainctrl_htphy(pi, core);
		wlc_phy_tx_tone_htphy(pi, (CHSPEC_IS40(pi->radio_chanspec)) ?
		                      HTPHY_RXCAL_TONEFREQ_40MHz : HTPHY_RXCAL_TONEFREQ_20MHz,
		                      HTPHY_RXCAL_TONEAMP, 0, 0, FALSE);
		wlc_phy_calc_rx_iq_comp_htphy(pi, 0x4000, (1 << core));
		wlc_phy_stopplayback_htphy(pi);
		wlc_phy_rxcal_radio_config_htphy(pi, FALSE, core);
		wlc_phy_rxcal_phy_config_htphy(pi, FALSE, core);
	}

	/* reactivate carrier search */
	wlc_phy_stay_in_carriersearch_htphy(pi, FALSE);

	return BCME_OK;
}

void
wlc_phy_set_filt_war_htphy(phy_info_t *pi, bool war)
{
	phy_info_htphy_t *pi_ht;
	pi_ht = (phy_info_htphy_t *)pi->u.pi_htphy;
	if (pi_ht->ht_ofdm20_filt_war_req != war) {
		pi_ht->ht_ofdm20_filt_war_req = war;
		if (pi->sh->up)
			wlc_phy_tx_digi_filts_htphy_war(pi, TRUE);
	}
}

bool
wlc_phy_get_filt_war_htphy(phy_info_t *pi)
{
	phy_info_htphy_t *pi_ht;
	pi_ht = (phy_info_htphy_t *)pi->u.pi_htphy;
	return pi_ht->ht_ofdm20_filt_war_req;
}

/* XXX configure tx filter for EU band edge shaping in ExtPA case
 * Assumptions:
 * 1. Post Reset, the filter coefficients are initialized once in the init path
 *    filter coefficients state is stored in pi_ht->ht_ofdm20_filt_war_active
 *    (Input: init = 1; Function wlc_phy_init_htphy)
 * 2. Outside of Init e.g. phy_chanspec, use init=0 to avoid
 *    making redundant PHYREG calls
 */
static void
wlc_phy_tx_digi_filts_htphy_war(phy_info_t *pi, bool init)
{
	int j, type;
	uint16 addr_offset = HTPHY_txfilt20CoeffStg0A1;
	phy_info_htphy_t *pi_ht;
	bool ofdm20_filt_war_active, ofdm20_filt_war_req;

	pi_ht = (phy_info_htphy_t *)pi->u.pi_htphy;
	ofdm20_filt_war_active = pi_ht->ht_ofdm20_filt_war_active;
	ofdm20_filt_war_req = pi_ht->ht_ofdm20_filt_war_req;

	if (init || (!init && ofdm20_filt_war_active != ofdm20_filt_war_req)) {
		/* XXX:
		 * Always write to the registers during init sequence (HW has been reset prior)
		 * During chanspec, check if the register needs to be updated.
		 * (to save Bandwidth Time)
		 */
		type = ofdm20_filt_war_req ? TXFILT_SHAPING_OFDM20_WAR : TXFILT_SHAPING_OFDM20_DEF;
		pi_ht->ht_ofdm20_filt_war_active = ofdm20_filt_war_req;

	        for (j = 0; j < HTPHY_NUM_DIG_FILT_COEFFS; j++) {
		        phy_utils_write_phyreg(pi, addr_offset+j, HTPHY_txdigi_filtcoeffs[type][j]);
		}
	}
}

/* configure tx filter for SM shaping in ExtPA case */
static void
wlc_phy_extpa_set_tx_digi_filts_htphy(phy_info_t *pi)
{
	int j, type = TXFILT_SHAPING_CCK;
	uint16 addr_offset = HTPHY_ccktxfilt20CoeffStg0A1;

	for (j = 0; j < HTPHY_NUM_DIG_FILT_COEFFS; j++) {
		phy_utils_write_phyreg(pi, addr_offset+j,
		              HTPHY_txdigi_filtcoeffs[type][j]);
	}

	type = TXFILT_SHAPING_OFDM40;
	addr_offset = HTPHY_txfilt40CoeffStg0A1;

	for (j = 0; j < HTPHY_NUM_DIG_FILT_COEFFS; j++) {
		phy_utils_write_phyreg(pi, addr_offset+j,
		              HTPHY_txdigi_filtcoeffs[type][j]);
	}

	wlc_phy_tx_digi_filts_htphy_war(pi, TRUE);
}

/* set txgain in case txpwrctrl is disabled (fixed power) */
static void
wlc_phy_txpwr_fixpower_htphy(phy_info_t *pi)
{
	uint16 core;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	FOREACH_CORE(pi, core) {
		wlc_phy_txpwr_by_index_htphy(pi, (1 << core), pi->u.pi_htphy->txpwrindex[core]);
	}
}

static void
BCMATTACHFN(wlc_phy_srom_read_rxgainerr_htphy)(phy_info_t *pi)
{
	/* read and uncompress gain-error values for rx power reporting */

	int8 tmp[PHY_CORE_NUM_3];
	int16 tmp2;

	(void)memset(tmp, -1, sizeof(tmp));

	/* read in temperature at calibration time */
	if (PHY_GETVAR(pi, "rawtempsense") != NULL)
		tmp2 = (int16) (((int16)PHY_GETINTVAR(pi, "rawtempsense"))  << 7) >> 7;
	else
		tmp2 = -1;

	if (tmp2 == -1) {
		/* set to some bogus value, since nothing was written to SROM */
		pi->srom_rawtempsense = 255;
	} else {
		pi->srom_rawtempsense = tmp2;
	}
	pi->u.pi_htphy->current_temperature = pi->srom_rawtempsense;

	/* 2G: */
	/* read and sign-extend */
	if (PHY_GETVAR(pi, "rxgainerr2ga0") != NULL)
		tmp[0] = (int8)(((int8)PHY_GETINTVAR(pi, "rxgainerr2ga0")) << 2) >> 2;

	if (PHYCORENUM(pi->pubpi.phy_corenum) > 1 && PHY_GETVAR(pi, "rxgainerr2ga1") != NULL)
		tmp[1] = (int8)(((int8)PHY_GETINTVAR(pi, "rxgainerr2ga1")) << 3) >> 3;

	if (PHYCORENUM(pi->pubpi.phy_corenum) > 2 && PHY_GETVAR(pi, "rxgainerr2ga2") != NULL)
		tmp[2] = (int8)(((int8)PHY_GETINTVAR(pi, "rxgainerr2ga2")) << 3) >> 3;

	if ((tmp[0] == -1) && (tmp[1] == -1) && (tmp[2] == -1)) {
		/* If all srom values are -1, then possibly
		 * no gainerror info was written to srom
		 */
		tmp[0] = 0; tmp[1] = 0; tmp[2] = 0;
		pi->rxgainerr2g_isempty = TRUE;
	} else {
		pi->rxgainerr2g_isempty = FALSE;
	}
	/* gain errors for cores 1 and 2 are stored in srom as deltas relative to core 0: */
	pi->rxgainerr_2g[0] = tmp[0];
	if (PHYCORENUM(pi->pubpi.phy_corenum) > 1)
		pi->rxgainerr_2g[1] = tmp[0] + tmp[1];
	if (PHYCORENUM(pi->pubpi.phy_corenum) > 2)
		pi->rxgainerr_2g[2] = tmp[0] + tmp[2];

	/* 5G low: */
	/* read and sign-extend */
	(void)memset(tmp, -1, sizeof(tmp));

	if (PHY_GETVAR(pi, "rxgainerr5gla0") != NULL)
		tmp[0] = (int8)(((int8)PHY_GETINTVAR(pi, "rxgainerr5gla0")) << 2) >> 2;

	if (PHYCORENUM(pi->pubpi.phy_corenum) > 1 && PHY_GETVAR(pi, "rxgainerr5gla1") != NULL)
		tmp[1] = (int8)(((int8)PHY_GETINTVAR(pi, "rxgainerr5gla1")) << 3) >> 3;

	if (PHYCORENUM(pi->pubpi.phy_corenum) > 2 && PHY_GETVAR(pi, "rxgainerr5gla2") != NULL)
		tmp[2] = (int8)(((int8)PHY_GETINTVAR(pi, "rxgainerr5gla2")) << 3) >> 3;

	if ((tmp[0] == -1) && (tmp[1] == -1) && (tmp[2] == -1)) {
		/* If all srom values are -1, then possibly
		 * no gainerror info was written to srom
		 */
		tmp[0] = 0; tmp[1] = 0; tmp[2] = 0;
		pi->rxgainerr5gl_isempty = TRUE;
	} else {
		pi->rxgainerr5gl_isempty = FALSE;
	}
	/* gain errors for cores 1 and 2 are stored in srom as deltas relative to core 0: */
	pi->rxgainerr_5gl[0] = tmp[0];
	if (PHYCORENUM(pi->pubpi.phy_corenum) > 1)
		pi->rxgainerr_5gl[1] = tmp[0] + tmp[1];
	if (PHYCORENUM(pi->pubpi.phy_corenum) > 2)
		pi->rxgainerr_5gl[2] = tmp[0] + tmp[2];

	/* 5G mid: */
	/* read and sign-extend */
	(void)memset(tmp, -1, sizeof(tmp));

	if (PHY_GETVAR(pi, "rxgainerr5gma0") != NULL)
		tmp[0] = (int8)(((int8)PHY_GETINTVAR(pi, "rxgainerr5gma0")) << 2) >> 2;

	if (PHYCORENUM(pi->pubpi.phy_corenum) > 1 && PHY_GETVAR(pi, "rxgainerr5gma1") != NULL)
		tmp[1] = (int8)(((int8)PHY_GETINTVAR(pi, "rxgainerr5gma1")) << 3) >> 3;

	if (PHYCORENUM(pi->pubpi.phy_corenum) > 2 && PHY_GETVAR(pi, "rxgainerr5gma2") != NULL)
		tmp[2] = (int8)(((int8)PHY_GETINTVAR(pi, "rxgainerr5gma2")) << 3) >> 3;

	if ((tmp[0] == -1) && (tmp[1] == -1) && (tmp[2] == -1)) {
		/* If all srom values are -1, then possibly
		 * no gainerror info was written to srom
		 */
		tmp[0] = 0; tmp[1] = 0; tmp[2] = 0;
		pi->rxgainerr5gm_isempty = TRUE;
	} else {
		pi->rxgainerr5gm_isempty = FALSE;
	}
	/* gain errors for cores 1 and 2 are stored in srom as deltas relative to core 0: */
	pi->rxgainerr_5gm[0] = tmp[0];
	if (PHYCORENUM(pi->pubpi.phy_corenum) > 1)
		pi->rxgainerr_5gm[1] = tmp[0] + tmp[1];
	if (PHYCORENUM(pi->pubpi.phy_corenum) > 2)
		pi->rxgainerr_5gm[2] = tmp[0] + tmp[2];

	/* 5G high: */
	/* read and sign-extend */
	(void)memset(tmp, -1, sizeof(tmp));
	if (PHY_GETVAR(pi, "rxgainerr5gha0") != NULL)
		tmp[0] = (int8)(((int8)PHY_GETINTVAR(pi, "rxgainerr5gha0")) << 2) >> 2;

	if (PHYCORENUM(pi->pubpi.phy_corenum) > 1 && PHY_GETVAR(pi, "rxgainerr5gha1") != NULL)
		tmp[1] = (int8)(((int8)PHY_GETINTVAR(pi, "rxgainerr5gha1")) << 3) >> 3;

	if (PHYCORENUM(pi->pubpi.phy_corenum) > 2 && PHY_GETVAR(pi, "rxgainerr5gha2") != NULL)
		tmp[2] = (int8)(((int8)PHY_GETINTVAR(pi, "rxgainerr5gha2")) << 3) >> 3;

	if ((tmp[0] == -1) && (tmp[1] == -1) && (tmp[2] == -1)) {
		/* If all srom values are -1, then possibly
		 * no gainerror info was written to srom
		 */
		tmp[0] = 0; tmp[1] = 0; tmp[2] = 0;
		pi->rxgainerr5gh_isempty = TRUE;
	} else {
		pi->rxgainerr5gh_isempty = FALSE;
	}
	/* gain errors for cores 1 and 2 are stored in srom as deltas relative to core 0: */
	pi->rxgainerr_5gh[0] = tmp[0];
	if (PHYCORENUM(pi->pubpi.phy_corenum) > 1)
		pi->rxgainerr_5gh[1] = tmp[0] + tmp[1];
	if (PHYCORENUM(pi->pubpi.phy_corenum) > 2)
		pi->rxgainerr_5gh[2] = tmp[0] + tmp[2];

	/* 5G upper: */
	/* read and sign-extend */
	(void)memset(tmp, -1, sizeof(tmp));

	if (PHY_GETVAR(pi, "rxgainerr5gua0") != NULL)
		tmp[0] = (int8)(((int8)PHY_GETINTVAR(pi, "rxgainerr5gua0")) << 2) >> 2;

	if (PHYCORENUM(pi->pubpi.phy_corenum) > 1 && PHY_GETVAR(pi, "rxgainerr5gua1") != NULL)
		tmp[1] = (int8)(((int8)PHY_GETINTVAR(pi, "rxgainerr5gua1")) << 3) >> 3;

	if (PHYCORENUM(pi->pubpi.phy_corenum) > 2 && PHY_GETVAR(pi, "rxgainerr5gua2") != NULL)
		tmp[2] = (int8)(((int8)PHY_GETINTVAR(pi, "rxgainerr5gua2")) << 3) >> 3;

	if ((tmp[0] == -1) && (tmp[1] == -1) && (tmp[2] == -1)) {
		/* If all srom values are -1, then possibly
		 * no gainerror info was written to srom
		 */
		tmp[0] = 0; tmp[1] = 0; tmp[2] = 0;
		pi->rxgainerr5gu_isempty = TRUE;
	} else {
		pi->rxgainerr5gu_isempty = FALSE;
	}
	/* gain errors for cores 1 and 2 are stored in srom as deltas relative to core 0: */
	pi->rxgainerr_5gu[0] = tmp[0];
	if (PHYCORENUM(pi->pubpi.phy_corenum) > 1)
		pi->rxgainerr_5gu[1] = tmp[0] + tmp[1];
	if (PHYCORENUM(pi->pubpi.phy_corenum) > 2)
		pi->rxgainerr_5gu[2] = tmp[0] + tmp[2];
}

#define HTPHY_SROM_NOISELVL_OFFSET (-70)

static void
BCMATTACHFN(wlc_phy_srom_read_noiselvl_htphy)(phy_info_t *pi)
{
	/* read noise levels from SROM */
	uint8 core;
	char phy_var_name[20];

	FOREACH_CORE(pi, core) {
		/* 2G: */
		(void)snprintf(phy_var_name, sizeof(phy_var_name), "noiselvl2ga%d", core);
		pi->noiselvl_2g[core] = HTPHY_SROM_NOISELVL_OFFSET -
		                             (uint8)PHY_GETINTVAR(pi, phy_var_name);

		/* 5G low: */
		(void)snprintf(phy_var_name, sizeof(phy_var_name), "noiselvl5gla%d", core);
		pi->noiselvl_5gl[core] = HTPHY_SROM_NOISELVL_OFFSET -
		                             (uint8)PHY_GETINTVAR(pi, phy_var_name);

		/* 5G mid: */
		(void)snprintf(phy_var_name, sizeof(phy_var_name), "noiselvl5gma%d", core);
		pi->noiselvl_5gm[core] = HTPHY_SROM_NOISELVL_OFFSET -
		                             (uint8)PHY_GETINTVAR(pi, phy_var_name);

		/* 5G high: */
		(void)snprintf(phy_var_name, sizeof(phy_var_name), "noiselvl5gha%d", core);
		pi->noiselvl_5gh[core] = HTPHY_SROM_NOISELVL_OFFSET -
		                              (uint8)PHY_GETINTVAR(pi, phy_var_name);

		/* 5G upper: */
		(void)snprintf(phy_var_name, sizeof(phy_var_name), "noiselvl5gua%d", core);
		pi->noiselvl_5gu[core] = HTPHY_SROM_NOISELVL_OFFSET -
		                              (uint8)PHY_GETINTVAR(pi, phy_var_name);
	}
}

static bool
BCMATTACHFN(wlc_phy_srom_read_htphy)(phy_info_t *pi)
{
	if (!wlc_phy_txpwr_srom9_read(pi))
		return FALSE;
	/* read and uncompress gain-error values for rx power reporting */
	wlc_phy_srom_read_rxgainerr_htphy(pi);
	/* read noise levels from SROM */
	wlc_phy_srom_read_noiselvl_htphy(pi);

	/* XXX  is done to get the  2.5+ 0.01*parefldovoltage according
	 * to the LD04331_PAR_SPAC
	 */
	if (getvar(pi->vars, "parefldovoltage") != NULL) {
		pi->ldo_voltage = (uint8)PHY_GETINTVAR(pi, "parefldovoltage");
	} else {
		/* Default if not present in NVRAM */
		pi->ldo_voltage = 0x23;	 /* 2.85 V */
	}

	if ((getvar(pi->vars, "elna2g")) != NULL)
		pi->u.pi_htphy->elna2g = (uint)PHY_GETINTVAR(pi, "elna2g");
	else
		pi->u.pi_htphy->elna2g = 0;

	if ((getvar(pi->vars, "elna5g")) != NULL)
		pi->u.pi_htphy->elna5g = (uint)PHY_GETINTVAR(pi, "elna5g");
	else
		pi->u.pi_htphy->elna5g = 0;

	if ((getvar(pi->vars, "eu_edthresh2g")) != NULL) {
		pi->srom_eu_edthresh2g = (int8)PHY_GETINTVAR(pi, "eu_edthresh2g");
	} else {
		pi->srom_eu_edthresh2g = 0;
	}

	if ((getvar(pi->vars, "eu_edthresh5g")) != NULL) {
		pi->srom_eu_edthresh5g = (int8)PHY_GETINTVAR(pi, "eu_edthresh5g");
	} else {
		pi->srom_eu_edthresh5g = 0;
	}

	return TRUE;
}

static int
wlc_phy_txpower_core_offset_set_htphy(phy_info_t *pi, struct phy_txcore_pwr_offsets *offsets)
{
	phy_info_htphy_t *pi_ht = (phy_info_htphy_t *)pi->u.pi_htphy;
	int8 core_offset;
	int core;
	int offset_changed = FALSE;

	FOREACH_CORE(pi, core) {
		core_offset = offsets->offset[core];
		if (core_offset != 0 && core > pi->pubpi.phy_corenum) {
			return BCME_BADARG;
		}

		if (pi_ht->txpwr_offset[core] != core_offset) {
			offset_changed = TRUE;
			pi_ht->txpwr_offset[core] = core_offset;
		}
	}

	/* Apply the new per-core targets to the hw */
	if (pi->sh->clk && offset_changed) {
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		wlc_phy_txpower_recalc_target_htphy(pi);
		wlapi_enable_mac(pi->sh->physhim);
	}

	return BCME_OK;
}

static int
wlc_phy_txpower_core_offset_get_htphy(phy_info_t *pi, struct phy_txcore_pwr_offsets *offsets)
{
	phy_info_htphy_t *pi_ht = (phy_info_htphy_t *)pi->u.pi_htphy;
	int core;

	memset(offsets, 0, sizeof(struct phy_txcore_pwr_offsets));

	FOREACH_CORE(pi, core) {
		offsets->offset[core] = pi_ht->txpwr_offset[core];
	}

	return BCME_OK;
}

void
wlc_phy_txpower_recalc_target_htphy(phy_info_t *pi)
{
	wlapi_high_update_txppr_offset(pi->sh->physhim, pi->tx_power_offset);

	/* recalc targets -- turns hwpwrctrl off */
	wlc_phy_txpwrctrl_pwr_setup_htphy(pi);

	/* restore power control */
	wlc_phy_txpwrctrl_enable_htphy(pi, pi->txpwrctrl);
}

static void
wlc_phy_txpower_recalc_idle_tssi_htphy(phy_info_t *pi)
{
	phy_info_htphy_t *pi_ht = (phy_info_htphy_t *)pi->u.pi_htphy;
	bool cache_hit = FALSE;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	/* If phy cal caching is enabled try restoring
	 * else do the idle tssi cal if the channel changed
	 * except if in the middle of a scan
	 */
#if defined(PHYCAL_CACHING)
	cache_hit = wlc_phy_txpower_idle_tssi_cache_restore_htphy(pi);
#endif // endif
	if (!cache_hit) {
		uint8 core;

		/* Cal and write idle tssi value to the registers */
		PHY_TXPWR(("wl%d: %s: No cache hit. Recal-ing IdleTSSI & Vmid for chanspec 0x%x\n",
		           pi->sh->unit, __FUNCTION__, pi->radio_chanspec));
		wlc_phy_txpwrctrl_idle_tssi_meas_htphy(pi);

		FOREACH_CORE(pi, core) {
			wlc_phy_txpwrctrl_set_idle_tssi_htphy(pi, pi_ht->idle_tssi[core], core);
		}
	}
}

#if defined(PHYCAL_CACHING)
static bool
wlc_phy_txpower_idle_tssi_cache_restore_htphy(phy_info_t *pi)
{
	ch_calcache_t *ctx;
	htphy_calcache_t *cache = NULL;
	uint8 core;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	ctx = wlc_phy_get_chanctx(pi, pi->radio_chanspec);

	if ((ctx) && (ctx->valid)) {
		PHY_TXPWR(("wl%d: %s: Restoring IdleTSSI & Vmid cache for chanspec 0x%x\n",
		         pi->sh->unit, __FUNCTION__, pi->radio_chanspec));
		cache = &ctx->u.htphy_cache;
		FOREACH_CORE(pi, core) {
			/* Restore Idle TSSI & Vmid values */
			wlc_phy_txpwrctrl_set_idle_tssi_htphy(pi, cache->idle_tssi[core], core);
			wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL, 1, 0x0b + 0x10*core,
			                          16, &cache->Vmid[core]);
		}
		return TRUE;
	}
	return FALSE;
}
#endif /* PHYCAL_CACHING */

static void
wlc_phy_txpwrctrl_idle_tssi_poll_auxadc_htphy(phy_info_t *pi, int32 *auxadc_buf, uint8 nsamps,
	uint8 mode)
{
	int32 itssi_d1, itssi_d2, itssi_d3;
	int8 itssi_meas_cnt = 0;

	wlc_phy_poll_auxadc_htphy(pi, HTPHY_AUXADC_SEL_TSSI, auxadc_buf, nsamps, mode);
	itssi_d1 = ABS(auxadc_buf[0] - auxadc_buf[2]);
	itssi_d2 = ABS(auxadc_buf[0] - auxadc_buf[4]);
	itssi_d3 = ABS(auxadc_buf[4] - auxadc_buf[2]);
	while (((auxadc_buf[0] > -18) || (auxadc_buf[2] > -18) || (auxadc_buf[4] > -18) ||
		(itssi_d1 > 12) || (itssi_d2 > 12) || (itssi_d3 > 12)) &&
		(itssi_meas_cnt < 4)) {
		itssi_meas_cnt++;
		PHY_TXPWR(("wl%d: %s: idle tssi meas off. remeas %d\n",
			pi->sh->unit, __FUNCTION__, itssi_meas_cnt));
		wlc_phy_stopplayback_htphy(pi);
		wlc_phy_tx_tone_htphy(pi, 4000, 0, 0, 0, FALSE);
		OSL_DELAY(20);
		wlc_phy_poll_auxadc_htphy(pi, HTPHY_AUXADC_SEL_TSSI, auxadc_buf, nsamps, mode);
		itssi_d1 = ABS(auxadc_buf[0] - auxadc_buf[2]);
		itssi_d2 = ABS(auxadc_buf[0] - auxadc_buf[4]);
		itssi_d3 = ABS(auxadc_buf[4] - auxadc_buf[2]);
	}
	if (itssi_meas_cnt != 0) pi->itssi_war_cnt++;

}

/* measure idle TSSI by sending 0-magnitude tone */
static void
wlc_phy_txpwrctrl_idle_tssi_meas_htphy(phy_info_t *pi)
{
	phy_info_htphy_t *pi_ht = (phy_info_htphy_t *)pi->u.pi_htphy;
	uint16 txgain_save[PHY_CORE_MAX];
	uint16 lpf_gain_save[PHY_CORE_MAX];
	uint16 override_save[PHY_CORE_MAX];
	int32 auxadc_buf[2*PHY_CORE_NUM_3];
	uint16 core;

	/* Dynamic Vmid cal variables */
	int8 target_itssi = -30;
	int8 delta_itssi, Vmid_correction;
	uint8 itr;
	uint16 Av[PHY_CORE_MAX];
	uint16 Vmid[PHY_CORE_MAX];
	int32 num, denom;
	uint8 extra_meas = 0;
	uint16 chan = CHSPEC_CHANNEL(pi->radio_chanspec);

	/* Rf and Rin are in Ohms */
	uint16 Rin  = 26500;
	uint16 Rf[8] = {9375, 11250, 13125, 15000, 18750, 22500, 26500, 30000};

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	/* BMAC_NOTE: Why is the idle tssi skipped? Is it avoiding doing a cal
	 * on a temporary channel (this is why I would think it would check
	 * SCAN_RM_IN_PROGRESS(pi) || PLT_INPROG_PHY(pi)), or is it preventing
	 * any energy emission (this is why I would think it would check
	 * PHY_MUTED()), or both?
	 */
	if (SCAN_RM_IN_PROGRESS(pi) || PLT_INPROG_PHY(pi) || PHY_MUTED(pi))
		/* skip idle tssi cal */
		return;

	/* Set TX gain to 0, so that LO leakage does not affect IDLE TSSI */
	FOREACH_CORE(pi, core) {
		txgain_save[core] = phy_utils_read_phyreg(pi, HTPHY_RfctrlTXGAIN(core));
		lpf_gain_save[core] = phy_utils_read_phyreg(pi, HTPHY_Rfctrl_lpf_gain(core));
		override_save[core] = phy_utils_read_phyreg(pi, HTPHY_RfctrlOverride(core));

		phy_utils_write_phyreg(pi, HTPHY_RfctrlTXGAIN(core), 0);
		MOD_PHYREGC(pi, HTPHY_Rfctrl_lpf_gain, core, lpf_gain_biq0, 0);
		MOD_PHYREGC(pi, HTPHY_RfctrlOverride, core, txgain, 1);
		MOD_PHYREGC(pi, HTPHY_RfctrlOverride, core, lpf_gain_biq0, 1);
	}

	(void)memset(auxadc_buf, 0, sizeof(auxadc_buf));
	if (!pi->itssical && PHYCORENUM(pi->pubpi.phy_corenum) > 1) {
		wlc_phy_tx_tone_htphy(pi, 4000, 0, 0, 0, FALSE);
		OSL_DELAY(20);
		wlc_phy_txpwrctrl_idle_tssi_poll_auxadc_htphy(pi, auxadc_buf, 8, 1);

		/* XXX PR106714 Need to provision for more margin for idle TSSI
		* for core1 PA(SE5019) at low 5G subband. By detecting idle TSSI
		* capped at low 5G core1, the Vmid value will be lowered.
		*/
		if (auxadc_buf[2] == -32 && pi->fem5g->pdetrange == 9 &&
			chan <= 48 && chan >= 36) {
			pi->itssi_cap_low5g = 1;
			wlc_phy_table_read_htphy(pi, HTPHY_TBL_ID_AFECTRL, 1,
			                         0x0b + 0x10*1, 16, &Vmid[1]);
			Vmid[1] = Vmid[1]-24;
			wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL, 1,
			                          0x0b + 0x10*1, 16, &Vmid[1]);
			wlc_phy_txpwrctrl_idle_tssi_poll_auxadc_htphy(pi, auxadc_buf, 8, 1);
		}
		wlc_phy_stopplayback_htphy(pi);

		wlc_phy_table_read_htphy(pi, HTPHY_TBL_ID_AFECTRL, 1, 0x0b + 0x10*1, 16, &Vmid[1]);
		PHY_TXPWR(("wl%d: %s core1: ch = %d itssi_cap=%d idle tssi = %d, Vmid=%d \n",
			pi->sh->unit, __FUNCTION__, CHSPEC_CHANNEL(pi->radio_chanspec),
			pi->itssi_cap_low5g, auxadc_buf[2], Vmid[1]));
	}

	if (pi->itssical) {
		FOREACH_CORE(pi, core) {
			/* Read Av and Vmid from AFECTRL table */
			wlc_phy_table_read_htphy(pi, HTPHY_TBL_ID_AFECTRL, 1, 0x0b + 0x10*core,
				16, &Vmid[core]);
			wlc_phy_table_read_htphy(pi, HTPHY_TBL_ID_AFECTRL, 1, 0x0f + 0x10*core,
				16, &Av[core]);
			PHY_TXPWR(("wl%d: %s: core%d: Original Av: %d Vmid:%d\n",
				pi->sh->unit, __FUNCTION__, core, Av[core], Vmid[core]));
		}
		wlc_phy_tx_tone_htphy(pi, 4000, 0, 0, 0, FALSE);
		OSL_DELAY(20);
		wlc_phy_txpwrctrl_idle_tssi_poll_auxadc_htphy(pi, auxadc_buf, 8, 1);

		extra_meas = 0;
		FOREACH_CORE(pi, core) {
			if (auxadc_buf[2*core] == -32) {
				extra_meas = 1;
				Vmid[core] = Vmid[core] - 32;
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL, 1,
					0x0b + 0x10*core, 16, &Vmid[core]);
				PHY_TXPWR(("wl%d: %s core%d: Extra meas: Vmid=%d\n",
					pi->sh->unit, __FUNCTION__, core, Vmid[core]));
			}
		}
		if (extra_meas == 1) {
			wlc_phy_txpwrctrl_idle_tssi_poll_auxadc_htphy(pi, auxadc_buf, 8, 1);
			FOREACH_CORE(pi, core) {
				PHY_TXPWR(("wl%d: %s core%d: Extra meas: itssi=%d\n",
					pi->sh->unit, __FUNCTION__, core, auxadc_buf[2*core]));
			}
		}

		/* Compute Vmid adjustment and converge to target idletssi */
		for (itr = 0; itr < 4; itr++) {
			extra_meas = 0;
			FOREACH_CORE(pi, core) {
				delta_itssi = (int8)auxadc_buf[2*core] - target_itssi;
				PHY_TXPWR(("wl%d: %s: core%d: delta_itssi:%d\n",
					pi->sh->unit, __FUNCTION__, core, delta_itssi));
				if (delta_itssi != 0) {
					/* XXX PR101494 Vmid adjustment computation happens here
					* the computation computes the Vmid adjustment
					* required to move from current TSSI to target idle TSSI
					* based on AUXADC Av Vmid formulas in the AFE worksheet
					* deltaVmidLin = ($deltaIT*4.68)/(1+$Av_gain)
					* Vmid correction quantized is deltaVmidLin/0.6 as
					* Each Vmid tick corresponds to 0.6 mv (150 mv / 255)
					*/
					extra_meas = 1;
					num = delta_itssi * 468 * Rin;
					denom = 60 * (Rin + Rf[Av[core]]);
					Vmid_correction = (num > 0 ? (2*num + denom)/(2*denom)
						: (2*num - denom)/(2*denom));
					Vmid[core] += Vmid_correction;
					wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL, 1,
						0x0b + 0x10*core, 16, &Vmid[core]);
					PHY_TXPWR(("wl%d: %s: core%d: numer:%d denom:%d",
						pi->sh->unit, __FUNCTION__, core, num, denom));
					PHY_TXPWR(("Vmid_cort:%d Vmid:%d\n",
						Vmid_correction, Vmid[core]));
				}
			}
			if (extra_meas == 0)
				break;
			wlc_phy_txpwrctrl_idle_tssi_poll_auxadc_htphy(pi, auxadc_buf, 8, 1);
			FOREACH_CORE(pi, core) {
				PHY_TXPWR(("wl%d: %s: core%d: Itr%d meas: itssi=%d\n",
					pi->sh->unit, __FUNCTION__, core, itr, auxadc_buf[2*core]));
			}
		}
		/* Turn off tone and remove TX gain override */
		wlc_phy_stopplayback_htphy(pi);
	}

	/* TSSI value is available only on the I component of AUX ADC out */
	FOREACH_CORE(pi, core) {
		phy_utils_write_phyreg(pi, HTPHY_RfctrlOverride(core), override_save[core]);
		phy_utils_write_phyreg(pi, HTPHY_RfctrlTXGAIN(core), txgain_save[core]);
		phy_utils_write_phyreg(pi, HTPHY_Rfctrl_lpf_gain(core), lpf_gain_save[core]);
		/* TSSI value is available only on the I component of AUX ADC out */
		pi_ht->idle_tssi[core] = (int8)auxadc_buf[2*core];
		PHY_TXPWR(("wl%d: %s: core%d: itssi=%d\n",
		           pi->sh->unit, __FUNCTION__, core, pi_ht->idle_tssi[core]));
	}
}
static void
wlc_phy_txpwrctrl_pwr_setup_htphy(phy_info_t *pi)
{
	phy_info_htphy_t *pi_ht = (phy_info_htphy_t *)pi->u.pi_htphy;
	srom_pwrdet_t	*pwrdet  = pi->pwrdet;
	int8   target_pwr_qtrdbm[PHY_CORE_MAX];
	int8   target_min_limit;
	int16  a1[PHY_CORE_MAX], b0[PHY_CORE_MAX], b1[PHY_CORE_MAX];
	uint8  chan_freq_range;
	uint8  iidx = 25;
	uint8  core;
	int32  num, den, pwr_est;
	uint32 tbl_len, tbl_offset, idx;
	uint8 regval[64];
	int8   pwradj_qdbm;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	/* enable TSSI */
	MOD_PHYREG(pi, HTPHY_TSSIMode, tssiEn, 1);
	MOD_PHYREG(pi, HTPHY_TxPwrCtrlCmd, txPwrCtrl_en, 0);

	/* Get pwrdet params from SROM for current subband */
	chan_freq_range = wlc_phy_get_chan_freq_range_htphy(pi, 0);

	FOREACH_CORE(pi, core) {
		a1[core] =  pwrdet->pwrdet_a1[core][chan_freq_range];
		b0[core] =  pwrdet->pwrdet_b0[core][chan_freq_range];
		b1[core] =  pwrdet->pwrdet_b1[core][chan_freq_range];
		PHY_TXPWR(("wl%d: %s: pwrdet core%d: a1=%d b0=%d b1=%d\n",
			pi->sh->unit, __FUNCTION__, core, a1[core], b0[core], b1[core]));
	}
	/* target power */
	target_min_limit = pi->min_txpower * WLC_TXPWR_DB_FACTOR;
	FOREACH_CORE(pi, core) {
		int8 txpwr = (int8)pi->tx_power_max_per_core[core];
#ifdef WL_SARLIMIT
#if defined(BCMDBG) || defined(WLTEST)
		if (pi->txpwr_max_percore_override[core] != 0) {
			txpwr = (int8)pi->txpwr_max_percore_override[core];
		}
#endif /* BCMDBG || WLTEST */
#endif /* WL_SARLIMIT */
		target_pwr_qtrdbm[core] = txpwr + pi_ht->txpwr_offset[core];
		/* never target below the min threshold */
		if (target_pwr_qtrdbm[core] < target_min_limit)
			target_pwr_qtrdbm[core] = target_min_limit;
	}

	/* determine pos/neg TSSI slope */
	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		MOD_PHYREG(pi, HTPHY_TxPwrCtrlIdleTssi, tssiPosSlope, pi->fem2g->tssipos);
	} else {
		MOD_PHYREG(pi, HTPHY_TxPwrCtrlIdleTssi, tssiPosSlope, pi->fem5g->tssipos);
	}

	/* set power index initial condition */
	FOREACH_CORE(pi, core) {
		wlc_phy_txpwrctrl_set_cur_index_htphy(pi, iidx, core);
		/* lower the starting txpwr index for A-Band */
		if (CHSPEC_IS5G(pi->radio_chanspec)) {
			wlc_phy_txpwrctrl_set_cur_index_htphy(pi, 0x20, core);
		}
	}

	/* expect aux-adc putting out 2's comp format */
	MOD_PHYREG(pi, HTPHY_TxPwrCtrlIdleTssi, rawTssiOffsetBinFormat, 1);

	/* set idle TSSI in 2s complement format (max is 0x1f) */
	FOREACH_CORE(pi, core) {
		wlc_phy_txpwrctrl_set_idle_tssi_htphy(pi, pi_ht->idle_tssi[core], core);
	}

	/* sample TSSI at 7.5us */
	MOD_PHYREG(pi, HTPHY_TxPwrCtrlNnum, Ntssi_delay, 150);

	/* average over 8 = 2^3 packets */
	MOD_PHYREG(pi, HTPHY_TxPwrCtrlNnum, Npt_intg_log2, 3);

	/* decouple IQ comp and LOFT comp from Power Control */
	MOD_PHYREG(pi, HTPHY_TxPwrCtrlCmd, use_txPwrCtrlCoefsIQ, 0);
	MOD_PHYREG(pi, HTPHY_TxPwrCtrlCmd, use_txPwrCtrlCoefsLO, 0);

	/* set target powers in 6.2 format (in dBs) */
	FOREACH_CORE(pi, core) {
		wlc_phy_txpwrctrl_set_target_htphy(pi, target_pwr_qtrdbm[core], core);
		PHY_TXPWR(("wl%d: %s: txpwrctl[%d]: %d \n",
			pi->sh->unit, __FUNCTION__, core, target_pwr_qtrdbm[core]));
	}

	/* load estimated power tables (maps TSSI to power in dBm)
	 *    entries in tx power table 0000xxxxxx
	 */
	tbl_len = 64;
	tbl_offset = 0;
	FOREACH_CORE(pi, core) {
		for (idx = 0; idx < tbl_len; idx++) {
			num = 8 * (16 * b0[core] + b1[core] * idx);
			den = 32768 + a1[core] * idx;
			/* XXX
			 * Applying an offset for 40MHz channels to accomodate the
			 * 20MHz vs 40MHz offset in TSSI Curves
			 */
			/* XXX
			 * Adding driver specific PA param adjust
			 */
			pwradj_qdbm = wlc_phy_txpwr_paparam_adj_htphy(pi, core, idx);
			if (CHSPEC_IS40(pi->radio_chanspec))
				pwr_est = MAX(((4 * num + den/2)/den) + pwradj_qdbm
				              + pwrdet->pwr_offset40[core][chan_freq_range], -8);
			else
				pwr_est = MAX(((4 * num + den/2)/den) + pwradj_qdbm, -8);

			pwr_est = MIN(pwr_est, 0x7F);
			regval[idx] = (uint8)pwr_est;
		}
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_TXPWRCTL(core), tbl_len,
		                          tbl_offset, 8, regval);
	}
}

int8
wlc_phy_txpwr_paparam_adj_htphy(phy_info_t *pi, uint8 core, uint32 idx)
{
	int8 pwradj_qdbm = 0;
	uint16 chan = CHSPEC_CHANNEL(pi->radio_chanspec);
	uint8 IS5G = CHSPEC_IS5G(pi->radio_chanspec);
	uint8 pdetrange = (IS5G) ? pi->fem5g->pdetrange :
	        pi->fem2g->pdetrange;

	/* XXX
	 * driver specific PA param adjust for X28
	 */
	if (pdetrange == 5) {
		if (chan >= 36 && chan < 48) pwradj_qdbm = 1;
		else if (chan > 48 && chan <= 64) pwradj_qdbm = 1;
		else if (chan >= 100 && chan <= 124) pwradj_qdbm = 2;
		else if (chan > 124 && chan < 140) pwradj_qdbm = 1;
		if (CHSPEC_IS20(pi->radio_chanspec) != 1) {
			if ((core == 0) || (core == 2))	pwradj_qdbm += 1;
			else if (core == 1)  pwradj_qdbm += 3;
		}
	}
	return pwradj_qdbm;
}

/* report maximum estimated power to limit power control run away */
uint8
wlc_phy_txpwr_max_est_pwr_get_htphy(phy_info_t *pi)
{
	uint8 core = 0;
	uint8 min_pwr;
	uint8 maxpwr[PHY_CORE_MAX];

	FOREACH_CORE(pi, core) {
		wlc_phy_table_read_htphy(pi, HTPHY_TBL_ID_TXPWRCTL(core), 1, 2, 8, &maxpwr[core]);
	}

	min_pwr = 0xFF;
	FOREACH_CORE(pi, core) {
		min_pwr = MIN(min_pwr, maxpwr[core]);
	}

	PHY_TXPWR(("wl%d: %s: channel(%d): %d\n", pi->sh->unit, __FUNCTION__,
	           pi->radio_chanspec & 0xff, min_pwr));
	return min_pwr;
}

/* report estimated power and adjusted estimated power in quarter dBms */
void
wlc_phy_txpwr_est_pwr_htphy(phy_info_t *pi, uint8 *Pout, uint8 *Pout_act)
{
	uint8 core;
	uint16 val;

	FOREACH_CORE(pi, core) {
		/* Read the Actual Estimated Powers without adjustment */
		val = phy_utils_read_phyreg(pi, HTPHY_EstPower(core));

		Pout[core] = 0;
		if ((val & HTPHY_EstPower_estPowerValid_MASK)) {
			Pout[core] = (uint8) ((val & HTPHY_EstPower_estPower_MASK)
			                      >> HTPHY_EstPower_estPower_SHIFT);
		}
		/* Read the Adjusted Estimated Powers */
		val = phy_utils_read_phyreg(pi, HTPHY_TxPwrCtrlStatus(core));

		Pout_act[core] = 0;
		if ((val & HTPHY_TxPwrCtrlStatus_estPwrAdjValid_MASK)) {
			Pout_act[core] = (uint8) ((val & HTPHY_TxPwrCtrlStatus_estPwr_adj_MASK)
			                          >> HTPHY_TxPwrCtrlStatus_estPwr_adj_SHIFT);
		}
	}
}

static void
wlc_phy_get_txgain_settings_by_index(phy_info_t *pi, txgain_setting_t *txgain_settings,
                                     int8 txpwrindex)
{
	uint32 txgain;

	wlc_phy_table_read_htphy(pi, HTPHY_TBL_ID_TXPWRCTL(0), 1,
	                         (192 + txpwrindex), 32, &txgain);

	txgain_settings->rad_gain = (txgain >> 16) & ((1<<(32-16+1))-1);
	txgain_settings->dac_gain = (txgain >>  8) & ((1<<(13- 8+1))-1);
	txgain_settings->bbmult   = (txgain >>  0) & ((1<<(7 - 0+1))-1);
}

static bool
wlc_phy_txpwrctrl_ison_htphy(phy_info_t *pi)
{
	uint16 mask = (HTPHY_TxPwrCtrlCmd_txPwrCtrl_en_MASK |
	               HTPHY_TxPwrCtrlCmd_hwtxPwrCtrl_en_MASK |
	               HTPHY_TxPwrCtrlCmd_use_txPwrCtrlCoefs_MASK);

	return ((phy_utils_read_phyreg((pi), HTPHY_TxPwrCtrlCmd) & mask) == mask);
}

static void
wlc_phy_txpwrctrl_set_idle_tssi_htphy(phy_info_t *pi, int8 idle_tssi, uint8 core)
{
	/* set idle TSSI in 2s complement format (max is 0x1f) */
	switch (core) {
	case 0:
		MOD_PHYREG(pi, HTPHY_TxPwrCtrlIdleTssi, idleTssi0, idle_tssi);
		break;
	case 1:
		MOD_PHYREG(pi, HTPHY_TxPwrCtrlIdleTssi, idleTssi1, idle_tssi);
		break;
	case 2:
		MOD_PHYREG(pi, HTPHY_TxPwrCtrlIdleTssi1, idleTssi2, idle_tssi);
		break;
	}
}

static void
wlc_phy_txpwrctrl_set_target_htphy(phy_info_t *pi, uint8 pwr_qtrdbm, uint8 core)
{
	/* set target powers in 6.2 format (in dBs) */
	switch (core) {
	case 0:
		MOD_PHYREG(pi, HTPHY_TxPwrCtrlTargetPwr, targetPwr0, pwr_qtrdbm);
		break;
	case 1:
		MOD_PHYREG(pi, HTPHY_TxPwrCtrlTargetPwr, targetPwr1, pwr_qtrdbm);
		break;
	case 2:
		MOD_PHYREG(pi, HTPHY_TxPwrCtrlTargetPwr1, targetPwr2, pwr_qtrdbm);
		break;
	}
}

static uint8
wlc_phy_txpwrctrl_get_cur_index_htphy(phy_info_t *pi, uint8 core)
{
	uint16 tmp;

	tmp = READ_PHYREGC(pi, HTPHY_TxPwrCtrlStatus, core, baseIndex);

	return (uint8)tmp;
}

static void
wlc_phy_txpwrctrl_set_cur_index_htphy(phy_info_t *pi, uint8 idx, uint8 core)
{
	switch (core) {
	case 0:
		MOD_PHYREG(pi, HTPHY_TxPwrCtrlCmd, pwrIndex_init, idx);
		break;
	case 1:
		MOD_PHYREG(pi, HTPHY_TxPwrCtrlInit, pwrIndex_init1, idx);
		break;
	case 2:
		MOD_PHYREG(pi, HTPHY_TxPwrCtrlInit1, pwrIndex_init2, idx);
		break;
	}
}

uint32
wlc_phy_idletssi_get_htphy(phy_info_t *pi)
{
	uint32 tmp1, tmp2, tmp3, tmp4;
	uint16 val;
	uint32 r;

	val = phy_utils_read_phyreg(pi, HTPHY_TxPwrCtrlIdleTssi);
	tmp1 = ((int8)((val & 0x3f)<<2))>>2 & 0xff;
	tmp2 = ((int8)(((val>>8) & 0x3f)<<2))>>2 & 0xff;
	val = phy_utils_read_phyreg(pi, HTPHY_TxPwrCtrlIdleTssi1);
	tmp3 = ((int8)((val & 0x3f)<<2))>>2 & 0xff;
	tmp4 = pi->itssi_war_cnt & 0xff;
	r = (tmp4<<24) | (tmp3<<16) | (tmp2<<8) | tmp1;

	return r;
}

uint32
wlc_phy_txpwr_idx_get_htphy(phy_info_t *pi)
{
	uint8 core;
	uint32 pwr_idx[] = {0, 0, 0, 0};
	uint32 tmp = 0;

	if (wlc_phy_txpwrctrl_ison_htphy(pi)) {
		FOREACH_CORE(pi, core) {
			pwr_idx[core] = wlc_phy_txpwrctrl_get_cur_index_htphy(pi, core);
		}
	} else {
		FOREACH_CORE(pi, core) {
			pwr_idx[core] = (pi->u.pi_htphy->txpwrindex[core] & 0xff);
		}
	}
	tmp = (pwr_idx[3] << 24) | (pwr_idx[2] << 16) | (pwr_idx[1] << 8) | pwr_idx[0];

	return tmp;
}

void
wlc_phy_txpwrctrl_enable_htphy(phy_info_t *pi, uint8 ctrl_type)
{
	phy_info_htphy_t *pi_ht = (phy_info_htphy_t *)pi->u.pi_htphy;
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
		if (wlc_phy_txpwrctrl_ison_htphy(pi)) {
			FOREACH_CORE(pi, core) {
				pi_ht->txpwrindex_hw_save[core] =
					wlc_phy_txpwrctrl_get_cur_index_htphy(pi, (uint8)core);
			}
		}

		/* Disable hw txpwrctrl */
		mask = HTPHY_TxPwrCtrlCmd_txPwrCtrl_en_MASK |
		       HTPHY_TxPwrCtrlCmd_hwtxPwrCtrl_en_MASK |
		       HTPHY_TxPwrCtrlCmd_use_txPwrCtrlCoefs_MASK;
		phy_utils_mod_phyreg(pi, HTPHY_TxPwrCtrlCmd, mask, 0);

	} else {
		/* Enable hw txpwrctrl */
		mask = HTPHY_TxPwrCtrlCmd_txPwrCtrl_en_MASK |
		       HTPHY_TxPwrCtrlCmd_hwtxPwrCtrl_en_MASK |
		       HTPHY_TxPwrCtrlCmd_use_txPwrCtrlCoefs_MASK;
		phy_utils_mod_phyreg(pi, HTPHY_TxPwrCtrlCmd, mask, mask);

	}
}

void
wlc_phy_txpwr_by_index_htphy(phy_info_t *pi, uint8 core_mask, int8 txpwrindex)
{
	uint16 core;
	txgain_setting_t txgain_settings;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	/* Set tx power based on an input "index"
	 * (Emulate what HW power control would use for a given table index)
	 */

	FOREACH_ACTV_CORE(pi, core_mask, core) {

		/* Check txprindex >= 0 */
		if (txpwrindex < 0)
			ASSERT(0); /* negative index not supported */

		/* Read tx gain table */
		wlc_phy_get_txgain_settings_by_index(pi, &txgain_settings, txpwrindex);

		/* Override gains: DAC, Radio and BBmult */
		switch (core) {
		case 0:
			MOD_PHYREG(pi, HTPHY_AfeseqInitDACgain, InitDACgain0,
			           txgain_settings.dac_gain);
			break;
		case 1:
			MOD_PHYREG(pi, HTPHY_AfeseqInitDACgain, InitDACgain1,
			           txgain_settings.dac_gain);
			break;
		case 2:
			MOD_PHYREG(pi, HTPHY_AfeseqInitDACgain, InitDACgain2,
			           txgain_settings.dac_gain);
			break;
		}

		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_RFSEQ, 1, (0x110 + core), 16,
		                          &txgain_settings.rad_gain);

		wlc_phy_set_tx_bbmult(pi, &txgain_settings.bbmult, core);

		PHY_TXPWR(("wl%d: %s: Fixed txpwrindex for core%d is %d\n",
		          pi->sh->unit, __FUNCTION__, core, txpwrindex));

		/* Update the per-core state of power index */
		pi->u.pi_htphy->txpwrindex[core] = txpwrindex;
	}
}

void
wlc_phy_txpower_sromlimit_get_htphy(phy_info_t *pi, chanspec_t chanspec, ppr_t *max_pwr, uint8 core)
{
	srom_pwrdet_t	*pwrdet  = pi->pwrdet;
	uint8 chan_freq_range;
	uint8 tmp_max_pwr = 0;
	uint8 chan = CHSPEC_CHANNEL(chanspec);

	ASSERT(core < PHY_CORE_MAX);
	ASSERT(max_pwr);
	chan_freq_range = wlc_phy_get_chan_freq_range_htphy(pi, chan);

	tmp_max_pwr = pwrdet->max_pwr[0][chan_freq_range];
	if (PHYCORENUM(pi->pubpi.phy_corenum) > 1)
		tmp_max_pwr = MIN(tmp_max_pwr, pwrdet->max_pwr[1][chan_freq_range]);
	if (PHYCORENUM(pi->pubpi.phy_corenum) > 2)
		tmp_max_pwr = MIN(tmp_max_pwr, pwrdet->max_pwr[2][chan_freq_range]);

	wlc_phy_txpwr_apply_srom9(pi, chan_freq_range, chanspec, tmp_max_pwr, max_pwr);

	if (pwrdet->max_pwr[core][chan_freq_range] > tmp_max_pwr)
		ppr_plus_cmn_val(max_pwr, (pwrdet->max_pwr[core][chan_freq_range] - tmp_max_pwr));

	switch (chan_freq_range) {
	case WL_CHAN_FREQ_RANGE_2G:
	case WL_CHAN_FREQ_RANGE_5G_BAND0:
	case WL_CHAN_FREQ_RANGE_5G_BAND1:
	case WL_CHAN_FREQ_RANGE_5G_BAND2:
	case WL_CHAN_FREQ_RANGE_5G_BAND3:
		ppr_apply_max(max_pwr, pwrdet->max_pwr[core][chan_freq_range]);
		break;
	}

	/*
	  PHY_ERROR(("####band #%d###pwrdet->max_pwr[core][chan_freq_range]
	  = %d ###\n",
	           chan_freq_range, pwrdet->max_pwr[core][chan_freq_range]));
	ppr_dsss_printf(max_pwr);
	ppr_ofdm_printf(max_pwr);
	ppr_mcs_printf(max_pwr);
	*/

	/*
	ppr_dsss_printf(pi->tx_srom_max_pwr[chan_freq_range]);
	ppr_ofdm_printf(pi->tx_srom_max_pwr[chan_freq_range]);
	ppr_mcs_printf(pi->tx_srom_max_pwr[chan_freq_range]);
	*/

}

void
wlc_phy_stay_in_carriersearch_htphy(phy_info_t *pi, bool enable)
{
	phy_info_htphy_t *pi_ht = (phy_info_htphy_t *)pi->u.pi_htphy;
	uint16 clip_off[] = {0xffff, 0xffff, 0xffff, 0xffff};

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	/* MAC should be suspended before calling this function */
	ASSERT(!(R_REG(pi->sh->osh, &pi->regs->maccontrol) & MCTL_EN_MAC));

	if (enable) {
		if (pi_ht->deaf_count == 0) {
			pi_ht->classifier_state = wlc_phy_classifier_htphy(pi, 0, 0);
			wlc_phy_classifier_htphy(pi, HTPHY_ClassifierCtrl_classifierSel_MASK, 4);
			wlc_phy_clip_det_htphy(pi, 0, pi_ht->clip_state);
			wlc_phy_clip_det_htphy(pi, 1, clip_off);
		}

		pi_ht->deaf_count++;

		wlc_phy_resetcca_htphy(pi);

	} else {
		ASSERT(pi_ht->deaf_count > 0);

		pi_ht->deaf_count--;

		if (pi_ht->deaf_count == 0) {
			wlc_phy_classifier_htphy(pi, HTPHY_ClassifierCtrl_classifierSel_MASK,
			pi_ht->classifier_state);
			wlc_phy_clip_det_htphy(pi, 1, pi_ht->clip_state);
		}
	}
}

void
wlc_phy_deaf_htphy(phy_info_t *pi, bool mode)
{
	phy_info_htphy_t *pi_ht = (phy_info_htphy_t *)pi->u.pi_htphy;

	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	if (mode) {
		if (pi_ht->deaf_count == 0)
			wlc_phy_stay_in_carriersearch_htphy(pi, TRUE);
		else
			PHY_ERROR(("%s: Deafness already set\n", __FUNCTION__));
	}
	else {
		if (pi_ht->deaf_count > 0)
			wlc_phy_stay_in_carriersearch_htphy(pi, FALSE);
		else
			PHY_ERROR(("%s: Deafness already cleared\n", __FUNCTION__));
	}
	wlapi_enable_mac(pi->sh->physhim);
}

bool
wlc_phy_get_deaf_htphy(phy_info_t *pi)
{
	uint16 curr_classifctl;
	uint16 curr_clipdet[PHY_CORE_MAX];
	int core;
	bool isDeaf = TRUE;

	/* Get current classifier and clip_detect settings */
	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	curr_classifctl = phy_utils_read_phyreg(pi, HTPHY_ClassifierCtrl) &
		HTPHY_ClassifierCtrl_classifierSel_MASK;
	wlc_phy_clip_det_htphy(pi, 0, curr_clipdet);
	wlapi_enable_mac(pi->sh->physhim);

	/* XXX
	 * For deafness to be set, ofdm and cck classifiers must be disabled,
	 * AND adc_clip thresholds must be set to max (0xffff)
	 */
	if (curr_classifctl != 4) {
		isDeaf = FALSE;
	} else {
		FOREACH_CORE(pi, core) {
			if (curr_clipdet[core] != 0xffff) {
				isDeaf = FALSE;
				break;
			}
		}
	}
	return isDeaf;
}

/* Override/Restore routine for Rx Digital LPF:
 * 1) Override: Save digital LPF config and set new LPF configuration
 * 2) Restore: Restore digital LPF config
 */
void
wlc_phy_dig_lpf_override_htphy(phy_info_t *pi, uint8 dig_lpf_ht)
{
	if ((dig_lpf_ht > 0) && !pi->phy_rx_diglpf_default_coeffs_valid) {

		pi->phy_rx_diglpf_default_coeffs[0] =
		        phy_utils_read_phyreg(pi, HTPHY_RxStrnFilt40Num00);
		pi->phy_rx_diglpf_default_coeffs[1] =
		        phy_utils_read_phyreg(pi, HTPHY_RxStrnFilt40Num01);
		pi->phy_rx_diglpf_default_coeffs[2] =
		        phy_utils_read_phyreg(pi, HTPHY_RxStrnFilt40Num02);
		pi->phy_rx_diglpf_default_coeffs[3] =
		        phy_utils_read_phyreg(pi, HTPHY_RxStrnFilt40Den00);
		pi->phy_rx_diglpf_default_coeffs[4] =
		        phy_utils_read_phyreg(pi, HTPHY_RxStrnFilt40Den01);
		pi->phy_rx_diglpf_default_coeffs[5] =
		        phy_utils_read_phyreg(pi, HTPHY_RxStrnFilt40Num10);
		pi->phy_rx_diglpf_default_coeffs[6] =
		        phy_utils_read_phyreg(pi, HTPHY_RxStrnFilt40Num11);
		pi->phy_rx_diglpf_default_coeffs[7] =
		        phy_utils_read_phyreg(pi, HTPHY_RxStrnFilt40Num12);
		pi->phy_rx_diglpf_default_coeffs[8] =
		        phy_utils_read_phyreg(pi, HTPHY_RxStrnFilt40Den10);
		pi->phy_rx_diglpf_default_coeffs[9] =
		        phy_utils_read_phyreg(pi, HTPHY_RxStrnFilt40Den11);
		pi->phy_rx_diglpf_default_coeffs_valid = TRUE;

	}

	switch (dig_lpf_ht) {
	case 0:  /* restore rx dig lpf */

		/* ASSERT(pi->phy_rx_diglpf_default_coeffs_valid); */
		if (!pi->phy_rx_diglpf_default_coeffs_valid) {
			break;
		}
		phy_utils_write_phyreg(pi, HTPHY_RxStrnFilt40Num00,
		                       pi->phy_rx_diglpf_default_coeffs[0]);
		phy_utils_write_phyreg(pi, HTPHY_RxStrnFilt40Num01,
		                       pi->phy_rx_diglpf_default_coeffs[1]);
		phy_utils_write_phyreg(pi, HTPHY_RxStrnFilt40Num02,
		                       pi->phy_rx_diglpf_default_coeffs[2]);
		phy_utils_write_phyreg(pi, HTPHY_RxStrnFilt40Den00,
		                       pi->phy_rx_diglpf_default_coeffs[3]);
		phy_utils_write_phyreg(pi, HTPHY_RxStrnFilt40Den01,
		                       pi->phy_rx_diglpf_default_coeffs[4]);
		phy_utils_write_phyreg(pi, HTPHY_RxStrnFilt40Num10,
		                       pi->phy_rx_diglpf_default_coeffs[5]);
		phy_utils_write_phyreg(pi, HTPHY_RxStrnFilt40Num11,
		                       pi->phy_rx_diglpf_default_coeffs[6]);
		phy_utils_write_phyreg(pi, HTPHY_RxStrnFilt40Num12,
		                       pi->phy_rx_diglpf_default_coeffs[7]);
		phy_utils_write_phyreg(pi, HTPHY_RxStrnFilt40Den10,
		                       pi->phy_rx_diglpf_default_coeffs[8]);
		phy_utils_write_phyreg(pi, HTPHY_RxStrnFilt40Den11,
		                       pi->phy_rx_diglpf_default_coeffs[9]);

		pi->phy_rx_diglpf_default_coeffs_valid = FALSE;
		break;
	case 1:  /* set rx dig lpf to ltrn-lpf mode */

		phy_utils_write_phyreg(pi, HTPHY_RxStrnFilt40Num00,
		                       phy_utils_read_phyreg(pi, HTPHY_RxFilt40Num00));
		phy_utils_write_phyreg(pi, HTPHY_RxStrnFilt40Num01,
		                       phy_utils_read_phyreg(pi, HTPHY_RxFilt40Num01));
		phy_utils_write_phyreg(pi, HTPHY_RxStrnFilt40Num02,
		                       phy_utils_read_phyreg(pi, HTPHY_RxFilt40Num02));
		phy_utils_write_phyreg(pi, HTPHY_RxStrnFilt40Num10,
		                       phy_utils_read_phyreg(pi, HTPHY_RxFilt40Num10));
		phy_utils_write_phyreg(pi, HTPHY_RxStrnFilt40Num11,
		                       phy_utils_read_phyreg(pi, HTPHY_RxFilt40Num11));
		phy_utils_write_phyreg(pi, HTPHY_RxStrnFilt40Num12,
		                       phy_utils_read_phyreg(pi, HTPHY_RxFilt40Num12));
		phy_utils_write_phyreg(pi, HTPHY_RxStrnFilt40Den00,
		                       phy_utils_read_phyreg(pi, HTPHY_RxFilt40Den00));
		phy_utils_write_phyreg(pi, HTPHY_RxStrnFilt40Den01,
		                       phy_utils_read_phyreg(pi, HTPHY_RxFilt40Den01));
		phy_utils_write_phyreg(pi, HTPHY_RxStrnFilt40Den10,
		                       phy_utils_read_phyreg(pi, HTPHY_RxFilt40Den10));
		phy_utils_write_phyreg(pi, HTPHY_RxStrnFilt40Den11,
		                       phy_utils_read_phyreg(pi, HTPHY_RxFilt40Den11));

		break;
	case 2:  /* bypass rx dig lpf */
		/* 0x2d4 = sqrt(2) * 512 */
		phy_utils_write_phyreg(pi, HTPHY_RxStrnFilt40Num00, 0x2d4);
		phy_utils_write_phyreg(pi, HTPHY_RxStrnFilt40Num01, 0);
		phy_utils_write_phyreg(pi, HTPHY_RxStrnFilt40Num02, 0);
		phy_utils_write_phyreg(pi, HTPHY_RxStrnFilt40Den00, 0);
		phy_utils_write_phyreg(pi, HTPHY_RxStrnFilt40Den01, 0);
		phy_utils_write_phyreg(pi, HTPHY_RxStrnFilt40Num10, 0x2d4);
		phy_utils_write_phyreg(pi, HTPHY_RxStrnFilt40Num11, 0);
		phy_utils_write_phyreg(pi, HTPHY_RxStrnFilt40Num12, 0);
		phy_utils_write_phyreg(pi, HTPHY_RxStrnFilt40Den10, 0);
		phy_utils_write_phyreg(pi, HTPHY_RxStrnFilt40Den11, 0);

		break;

	default:
		ASSERT((dig_lpf_ht == 2) || (dig_lpf_ht == 1) || (dig_lpf_ht == 0));
		break;
	}
}

/* Setup/Cleanup routine for high-pass corner (HPC) of LPF:
 * 1) Setup: Save LPF config and set HPC to lowest value (0x1)
 * 2) Cleanup: Restore HPC config
 */
void
wlc_phy_lpf_hpc_override_htphy(phy_info_t *pi, bool setup_not_cleanup)
{
	phy_info_htphy_t *pi_ht = (phy_info_htphy_t *)pi->u.pi_htphy;
	htphy_lpfCT_phyregs_t *porig = &(pi_ht->ht_lpfCT_phyregs_orig);
	uint8 core;

	if (setup_not_cleanup) {
		/* XXX
		 *-------------
		 * Phy "Setup"
		 *-------------
		 */

		ASSERT(!porig->is_orig);
		porig->is_orig = TRUE;

		FOREACH_CORE(pi, core) {
			porig->RfctrlOverrideLpfCT[core] =
				phy_utils_read_phyreg(pi, HTPHY_RfctrlOverrideLpfCT(core));
			porig->RfctrlCoreLpfCT[core] =
				phy_utils_read_phyreg(pi, HTPHY_RfctrlCoreLpfCT(core));

			MOD_PHYREGC(pi, HTPHY_RfctrlOverrideLpfCT, core, lpf_hpc, 1);
			MOD_PHYREGC(pi, HTPHY_RfctrlCoreLpfCT, core, lpf_hpc, 1);
		}
	} else {
		/* XXX
		 *-------------
		 * Phy "Cleanup"
		 *-------------
		 */

		ASSERT(porig->is_orig);
		porig->is_orig = FALSE;

		FOREACH_CORE(pi, core) {
			phy_utils_write_phyreg(pi, HTPHY_RfctrlOverrideLpfCT(core),
				porig->RfctrlOverrideLpfCT[core]);
			phy_utils_write_phyreg(pi, HTPHY_RfctrlCoreLpfCT(core),
				porig->RfctrlCoreLpfCT[core]);
		}
	}
}

void
wlc_phy_get_initgain_dB_htphy(phy_info_t *pi, int16 *initgain_dB)
{
	uint16 initgain_code[PHY_CORE_MAX];
	uint8 core;

	/* Read initgain code from phy rfseq table */
	wlc_phy_table_read_htphy(pi, HTPHY_TBL_ID_RFSEQ, 3, 0x106, 16, initgain_code);

	FOREACH_CORE(pi, core) {
		initgain_dB[core] = wlc_phy_rxgaincode_to_dB_htphy(pi, initgain_code[core]);
	}

	/* Add extLNA gain if extLNA is present */
	if (((BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA) &&
	     (CHSPEC_IS2G(pi->radio_chanspec))) ||
	    ((BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA_5GHz) &&
	     (CHSPEC_IS5G(pi->radio_chanspec)))) {
		uint8 indx;
		int8 gain[PHY_CORE_MAX];

		indx = READ_PHYREG(pi, HTPHY_Core0InitGainCodeA2059, initExtLnaIndex);
		wlc_phy_table_read_htphy(pi, HTPHY_TBL_ID_GAIN1, 1, (0x0 + indx), 8, &(gain[0]));
		if (PHYCORENUM(pi->pubpi.phy_corenum) > 1) {
			indx = READ_PHYREG(pi, HTPHY_Core1InitGainCodeA2059, initExtLnaIndex);
			wlc_phy_table_read_htphy(pi, HTPHY_TBL_ID_GAIN2, 1, (0x0 + indx), 8,
			                         &(gain[1]));
		}
		if (PHYCORENUM(pi->pubpi.phy_corenum) > 2) {
			indx = READ_PHYREG(pi, HTPHY_Core2InitGainCodeA2059, initExtLnaIndex);
			wlc_phy_table_read_htphy(pi, HTPHY_TBL_ID_GAIN3, 1, (0x0 + indx), 8,
			                         &(gain[2]));
		}

		FOREACH_CORE(pi, core) {
			initgain_dB[core] += gain[core];
		}
	}
}

static int16
wlc_phy_rxgaincode_to_dB_htphy(phy_info_t *pi, uint16 gain_code)
{
	int8 lna1_code, lna2_code, mixtia_code, biq0_code, biq1_code;
	int8 lna1_gain, lna2_gain, mixtia_gain, biq0_gain, biq1_gain;
	uint16 TR_loss;
	int16 total_gain;

	/* Extract gain codes for each gain element from overall gain code: */
	lna1_code = gain_code & 0x3;
	lna2_code = (gain_code >> 2) & 0x3;
	mixtia_code = (gain_code >> 4) & 0xf;
	biq0_code = (gain_code >> 8) & 0xf;
	biq1_code = (gain_code >> 12) & 0xf;

	/* XXX
	* FIXME:
	* Need to deduce true offset indices into gain table
	* from the matching entry in gainbits table
	*/

	/* Look up gains for lna1, lna2 and mixtia from indices: */
	wlc_phy_table_read_htphy(pi, HTPHY_TBL_ID_GAIN1, 1, (0x8 + lna1_code), 8, &lna1_gain);
	wlc_phy_table_read_htphy(pi, HTPHY_TBL_ID_GAIN1, 1, (0x10 + lna2_code), 8, &lna2_gain);
	wlc_phy_table_read_htphy(pi, HTPHY_TBL_ID_GAIN1, 1, (0x20 + mixtia_code), 8, &mixtia_gain);

	/* Biquad gains: */
	biq0_gain = 3 * biq0_code;
	biq1_gain = 3 * biq1_code;

	/* Need to subtract out TR_loss in Rx mode: */
	TR_loss = (phy_utils_read_phyreg(pi, HTPHY_TRLossValue)) & 0x7f;

	/* Total gain: */
	total_gain = lna1_gain + lna2_gain + mixtia_gain + biq0_gain +  biq1_gain - TR_loss;

	return total_gain;
}

#ifdef SAMPLE_COLLECT
/* channel to frequency conversion */
static int
wlc_phy_chan2fc(uint channel)
{
	/* go from channel number (such as 6) to carrier freq (such as 2442) */
	if (channel >= 184 && channel <= 228)
		return (channel*5 + 4000);
	else if (channel >= 32 && channel <= 180)
		return (channel*5 + 5000);
	else if (channel >= 1 && channel <= 13)
		return (channel*5 + 2407);
	else if (channel == 14)
		return (2484);
	else
		return -1;
}

/* Save/Restore rxgain settings for lna1, lna2, mix_tia, biq0 and biq1 */
static void
wlc_phy_agc_rxgain_config_htphy(phy_info_t *pi, bool push_not_pop)
{
	phy_info_htphy_t *pi_ht = (phy_info_htphy_t *)pi->u.pi_htphy;
	htphy_rxgain_phyregs_t *porig = &(pi_ht->ht_rxgain_phyregs_orig);
	uint8 core;

	if (push_not_pop) {
		/* XXX
		 *-------------
		 * Phy "Setup"
		 *-------------
		 */

		ASSERT(!porig->is_orig);
		porig->is_orig = TRUE;

		/* Save Rx gain state */

		FOREACH_CORE(pi, core) {
			porig->RfctrlOverride[core] =
				phy_utils_read_phyreg(pi, HTPHY_RfctrlOverride(core));
			porig->RfctrlRXGAIN[core] =
				phy_utils_read_phyreg(pi, HTPHY_RfctrlRXGAIN(core));
			porig->Rfctrl_lpf_gain[core] =
				phy_utils_read_phyreg(pi, HTPHY_Rfctrl_lpf_gain(core));
		}
	}

	else {
		/* XXX
		 *-------------
		 * Phy "Cleanup"
		 *-------------
		 */

		ASSERT(porig->is_orig);
		porig->is_orig = FALSE;

		/* Restore saved RX gain state */
		FOREACH_CORE(pi, core) {
			phy_utils_write_phyreg(pi, HTPHY_RfctrlOverride(core),
			                       porig->RfctrlOverride[core]);
			phy_utils_write_phyreg(pi, HTPHY_RfctrlRXGAIN(core),
			                       porig->RfctrlRXGAIN[core]);
			phy_utils_write_phyreg(pi, HTPHY_Rfctrl_lpf_gain(core),
				porig->Rfctrl_lpf_gain[core]);
		}
	}
}

#define HTPHY_NUM_BANDS 2
#define HTPHY_BAND_B 0
#define HTPHY_BAND_A 1
/* "AGC" routine:
 * 1) Determines gain code for non-clipping i/p to ADC
 * 2) Output is a 16-bit gain code for each core (applied_gain[core]):
 *    lower-byte written to phyreg RfctrlRXGAIN(core), upper-byte
 *    written to Rfctrl_lpf_gain(core)
 */
static int
wlc_phy_agc_htphy(phy_info_t *pi, int16 *applied_gain)
{
	int16 *core_gain = applied_gain;  /* local copy */
	uint16 nsamps = 0x2000;
	uint32 iqpwr;
	int i, core;
	int bcmerror = BCME_OK;

	/* For storing rx_iq estimates of all cores */
	phy_iq_est_t est[PHY_CORE_MAX];

	/* XXX
	 * Gain codes for different RX power regimes:
	 * 1) Maximum non-clipping  per-core rxpowers for the gains
	 *    below are {-75, -63, -51, -42, -29} dBm
	 * 2) gains need fine-tuning
	 */
	uint16 rfctrl_gain[][HTPHY_NUM_BANDS] = { {0x733f, 0x734f},
	                                          {0x513f, 0x514f},
	                                          {0x113f, 0x114f},
	                                          {0x3b, 0x4b},
	                                          {0x33, 0x43} };

	int num_gain_levels = sizeof(rfctrl_gain) / sizeof(rfctrl_gain[0]);
	uint8 band = CHSPEC_IS2G(pi->radio_chanspec) ? HTPHY_BAND_B : HTPHY_BAND_A;

	/* Set the clip threshold for avg pwr reported by rx_iq_est,
	 * assuming PAPR = 6dB:
	 */
	uint16 iqsamples_clip_thresh = 4000;

	core = 0;

	FOREACH_CORE(pi, core) {

		MOD_PHYREGC(pi, HTPHY_RfctrlOverride, core, rxgain, 1);
		MOD_PHYREGC(pi, HTPHY_RfctrlOverride, core, lpf_gain_biq0, 1);
		MOD_PHYREGC(pi, HTPHY_RfctrlOverride, core, lpf_gain_biq1, 1);

		/* loop over gain levels and choose first non-clipping gain */

		for (i = 0; i < num_gain_levels; i++) {
			/* Lower byte of gain code applied to lna1, lna2 and mix_tia */
			phy_utils_write_phyreg(pi, HTPHY_RfctrlRXGAIN(core),
				(uint16)(rfctrl_gain[i][band] & 0xff));
			/* Upper byte applied to biq0 and biq1 */
			phy_utils_write_phyreg(pi, HTPHY_Rfctrl_lpf_gain(core),
				(uint16)((rfctrl_gain[i][band] & 0xff00) >> 8));

			/* RX_IQ_EST args: wait_time = 32, wait_for_crs = 0 */
			wlc_phy_rx_iq_est_htphy(pi, est, nsamps, 32, 0);

			/* Calculate total I+Q power: */
			iqpwr = (est[core].i_pwr + est[core].q_pwr) / nsamps;
			if (iqpwr < iqsamples_clip_thresh)
				break;
		}

		if (i == num_gain_levels) {
			PHY_ERROR(("%s: Could not find a non-clipping gain for core %d\n",
				__FUNCTION__, core));
			bcmerror = BCME_ERROR;
			i = num_gain_levels - 1;
		}

		/* Report the dB gain applied on this core */
		core_gain[core] = wlc_phy_rxgaincode_to_dB_htphy(pi, rfctrl_gain[i][band]);
	}

	return bcmerror;
}

#define FILE_HDR_LEN 20 /* words */
/* (FIFO memory is 176kB = 45056 x 32bit) */

int
phy_ht_sample_collect(phy_info_t *pi, wl_samplecollect_args_t *collect, uint32 *buf)
{
	uint32 machwcap;
	uint32 tx_fifo_length;
	uint32 phy_ctl, timer, cnt;
	uint16 val;
	uint8 words_per_us_BW20 = 80;
	uint8 words_per_us;
	uint16 fc = (uint16)wlc_phy_chan2fc(CHSPEC_CHANNEL(pi->radio_chanspec));
	uint8 gpio_collection = 0;
	uint32 *ptr;
	phy_info_htphy_t *pi_ht;
	wl_sampledata_t *sample_data;
	int16 agc_gain[PHY_CORE_NUM_4] = {0};
	/* subtract from ADC sample power to obtain true analog power in dBm */
	uint16 dBsample_to_dBm_sub;
	uint8 sample_rate, sample_bitwidth;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	ASSERT(pi);
	pi_ht = (phy_info_htphy_t *)pi->u.pi_htphy;
	if (!pi_ht)
		return BCME_ERROR;

	/* initial return info pointers */
	sample_data = (wl_sampledata_t *)buf;
	ptr = (uint32 *)&sample_data[1];
	bzero((uint8 *)sample_data, sizeof(wl_sampledata_t));
	sample_data->version = htol16(WL_SAMPLEDATA_T_VERSION);
	sample_data->size = htol16(sizeof(wl_sampledata_t));

	/* get TX FIFO length in words */
	machwcap = R_REG(pi->sh->osh, &pi->regs->machwcap);
	tx_fifo_length = (((machwcap >> 3) & 0x1ff) << 7);

	/* compute applicable words_per_us */
	words_per_us = (CHSPEC_IS40(pi->radio_chanspec) ?
		(words_per_us_BW20 << 1) : words_per_us_BW20);

	/* duration(s): length sanity check and mapping from "max" to usec values */
	if (((collect->pre_dur + collect->post_dur) * words_per_us) > tx_fifo_length) {
		PHY_ERROR(("wl%d: %s Error: Bad Duration Option\n", pi->sh->unit, __FUNCTION__));
		return BCME_RANGE;
	}

	/* set phyreg into desired collect mode and enable */
	if (collect->mode == 0) { /* ADC */
		phy_utils_write_phyreg(pi, HTPHY_AdcDataCollect, 0);
		MOD_PHYREG(pi, HTPHY_AdcDataCollect, adc_sel, collect->mode);
		MOD_PHYREG(pi, HTPHY_AdcDataCollect, adcDataCollectEn, 1);
	} else if (collect->mode < 4) { /* ADC+RSSI */
		phy_utils_write_phyreg(pi, HTPHY_AdcDataCollect, 0);
		MOD_PHYREG(pi, HTPHY_AdcDataCollect, adc_sel, collect->mode);
		MOD_PHYREG(pi, HTPHY_AdcDataCollect, adcDataCollectEn, 1);
	} else if (collect->mode == 4) { /* GPIO */
		phy_utils_write_phyreg(pi, HTPHY_AdcDataCollect, 0);
		MOD_PHYREG(pi, HTPHY_AdcDataCollect, gpioSel, 1);
		MOD_PHYREG(pi, HTPHY_gpioSel, gpioSel, collect->gpio_sel);
	} else {
		PHY_ERROR(("wl%d: %s Error: Unsupported Mode\n", pi->sh->unit, __FUNCTION__));
		return BCME_ERROR;
	}
	MOD_PHYREG(pi, HTPHY_AdcDataCollect, downSample, collect->downsamp);

	/* be deaf if requested (e.g. for spur measurement) */
	if (collect->be_deaf) {
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		wlc_phy_stay_in_carriersearch_htphy(pi, TRUE);
	}

	/* perform AGC if requested */
	if (collect->agc) {
		/* Backup gain config prior to gain control */
		wlc_phy_agc_rxgain_config_htphy(pi, TRUE);
		wlc_phy_agc_htphy(pi, agc_gain);
	} else {
		/* Set reported agc gains to init_gain with gain_error correction */
		int16 gainerr[PHY_CORE_MAX];
		uint8 core;

		wlc_phy_get_rxgainerr_phy(pi, gainerr);
		FOREACH_CORE(pi, core) {
			/* gainerr is in 0.5dB steps; needs to be rounded to nearest dB */
			int16 tmp = gainerr[core];
			tmp = ((tmp >= 0) ? ((tmp + 1) >> 1) : -1*((-1*tmp + 1) >> 1));
			agc_gain[core] = HTPHY_NOISE_INITGAIN + tmp;
		}
	}

	/* Apply filter settings if requested */
	if (collect->filter) {
		/* Override the LPF high pass corners to their lowest values (0x1) */
		wlc_phy_lpf_hpc_override_htphy(pi, TRUE);
	}

	/* set Tx-FIFO collect start pointer to 0 */
	pi_ht->pstart = 0;
	W_REG(pi->sh->osh, &pi->regs->PHYREF_SMPL_CLCT_STRPTR, pi_ht->pstart);

	PHY_TRACE(("wl%d: %s Start capture, trigger = %d\n", pi->sh->unit, __FUNCTION__,
		collect->trigger));

	timer = collect->timeout;
	/* immediate trigger */
	if (collect->trigger == TRIGGER_NOW) {
		uint32 curptr;

		/* compute and set stop pointer */
		pi_ht->pstop = (collect->pre_dur + collect->post_dur) * words_per_us;
		if (pi_ht->pstop >= tx_fifo_length-1)
			pi_ht->pstop = tx_fifo_length-1;
		W_REG(pi->sh->osh, &pi->regs->PHYREF_SMPL_CLCT_STPPTR, pi_ht->pstop);

		/* set Stop bit and Start bit (start capture) */
		phy_ctl = R_REG(pi->sh->osh, &pi->regs->psm_phy_hdr_param);
		W_REG(pi->sh->osh, &pi->regs->psm_phy_hdr_param, phy_ctl | (1 << 4) | (1 << 5));

		/* wait until done */
		do {
			OSL_DELAY(10);
			curptr = R_REG(pi->sh->osh, &pi->regs->PHYREF_SMPL_CLCT_CURPTR);
			timer--;
		} while ((curptr != pi_ht->pstop) && (timer > 0));

		/* clear start/stop bits */
		W_REG(pi->sh->osh, &pi->regs->psm_phy_hdr_param, phy_ctl & 0xFFCF);

		/* set start/stop pointers for readout */
		pi_ht->pfirst = 0;
		pi_ht->plast = R_REG(pi->sh->osh, &pi->regs->PHYREF_SMPL_CLCT_CURPTR);
	} else {
		uint32 mac_ctl, dur_1_8th_us;
		uint16 tmp_phyreg;

		tmp_phyreg = phy_utils_read_phyreg(pi, HTPHY_AdcDataCollect);
		phy_utils_write_phyreg(pi, HTPHY_AdcDataCollect, 0);

		/* enable mac and run psm */
		mac_ctl = R_REG(pi->sh->osh, &pi->regs->maccontrol);
		W_REG(pi->sh->osh, &pi->regs->maccontrol, mac_ctl | MCTL_PSM_RUN | MCTL_EN_MAC);

		/* set stop pointer */
		pi_ht->pstop = tx_fifo_length-1;
		W_REG(pi->sh->osh, &pi->regs->PHYREF_SMPL_CLCT_STPPTR, pi_ht->pstop);

		/* set up post-trigger duration (expected by ucode in units of 1/8 us) */
		dur_1_8th_us = collect->post_dur << 3;
		W_REG(pi->sh->osh, &pi->regs->PHYREF_TSF_GPT2_CTR_L, dur_1_8th_us & 0x0000FFFF);
		W_REG(pi->sh->osh, &pi->regs->PHYREF_TSF_GPT2_CTR_H, dur_1_8th_us >> 16);

		/* start ucode trigger-based sample collect procedure */
		wlapi_bmac_write_shm(pi->sh->physhim, M_SMPL_COL_BMP, 0x0);

		if (ISSIM_ENAB(pi->sh->sih)) {
			OSL_DELAY(1000*collect->pre_dur);
		} else {
			OSL_DELAY(collect->pre_dur);
		}
		wlapi_bmac_write_shm(pi->sh->physhim, M_SMPL_COL_BMP, (int8)collect->trigger);
		wlapi_bmac_write_shm(pi->sh->physhim, M_SMPL_COL_CTL, tmp_phyreg);

		PHY_TRACE(("wl%d: %s Wait for trigger ...\n", pi->sh->unit, __FUNCTION__));

		/* wait until start bit has been cleared - or we time out */
		do {
			OSL_DELAY(10);
			val = R_REG(pi->sh->osh, &pi->regs->psm_phy_hdr_param) & 0x30;
			timer--;
		} while ((val != 0) && (timer > 0));

		/* set first and last pointer indices for readout */
		pi_ht->plast = R_REG(pi->sh->osh, &pi->regs->PHYREF_SMPL_CLCT_CURPTR);
		if (pi_ht->plast >= (collect->pre_dur + collect->post_dur) * words_per_us) {
			pi_ht->pfirst =
				pi_ht->plast - (collect->pre_dur + collect->post_dur)*words_per_us;
		} else {
			pi_ht->pfirst = (pi_ht->pstop - pi_ht->pstart + 1) +
				pi_ht->plast - (collect->pre_dur + collect->post_dur)*words_per_us;
		}

		/* erase trigger setup */
		/* restore mac_ctl */
		W_REG(pi->sh->osh, &pi->regs->maccontrol, mac_ctl);
	}

	/* CLEAN UP: */
	/* return from deaf if requested */
	if (collect->be_deaf) {
		wlc_phy_stay_in_carriersearch_htphy(pi, FALSE);
		wlapi_enable_mac(pi->sh->physhim);
	}
	/* revert to original gains if AGC was applied */
	if (collect->agc) {
		wlc_phy_agc_rxgain_config_htphy(pi, FALSE);
	}
	/* Restore filter settings if changed */
	if (collect->filter) {
		/* Restore LPF high pass corners to their original values */
		wlc_phy_lpf_hpc_override_htphy(pi, FALSE);
	}
	/* turn off sample collect config in PHY & MAC */
	phy_utils_write_phyreg(pi, HTPHY_AdcDataCollect, 0);
	W_REG(pi->sh->osh, &pi->regs->psm_phy_hdr_param, 0x2);

	/* abort if timeout ocurred */
	if (timer == 0) {
		PHY_ERROR(("wl%d: %s Error: Timeout\n", pi->sh->unit, __FUNCTION__));
		return BCME_ERROR;
	}

	PHY_TRACE(("wl%d: %s: Capture successful\n", pi->sh->unit, __FUNCTION__));

	if (pi_ht->pfirst > pi_ht->plast) {
		cnt = pi_ht->pstop - pi_ht->pfirst + 1;
		cnt += pi_ht->plast;
	} else {
		cnt = pi_ht->plast - pi_ht->pfirst;
	}

	sample_data->tag = htol16(WL_SAMPLEDATA_HEADER_TYPE);
	sample_data->length = htol16((WL_SAMPLEDATA_HEADER_SIZE));
	sample_data->flag = 0;		/* first sequence */
	sample_data->flag |= htol32(WL_SAMPLEDATA_MORE_DATA);

	sample_rate = CHSPEC_IS40(pi->radio_chanspec) ? 80 : 40;
	sample_bitwidth = 10;
	/* Hack in conversion factor for subtracting from adc sample
	 * power to obtain true analog power in dBm:
	 */
	dBsample_to_dBm_sub = 49;

	/* store header to buf */
	ptr[0] = htol32(0xACDC2009);
	ptr[1] = htol32(0xFFFF0000 | (FILE_HDR_LEN<<8));
	ptr[2] = htol32(cnt);
	ptr[3] = htol32(0xFFFF0000 | (pi->pubpi.phy_rev<<8) | pi->pubpi.phy_type);
	ptr[4] = htol32(0xFFFFFFFF);
	ptr[5] = htol32(((fc / 100) << 24) | ((fc % 100) << 16) | (3 << 8) |
	         (CHSPEC_IS40(pi->radio_chanspec) ? 40 : 20));
	if (collect->mode == 4)
		gpio_collection = 1;
	ptr[6] = htol32((collect->gpio_sel << 24) | (collect->mode << 8) | gpio_collection);
	ptr[7] = htol32(0xFFFF0000 | (collect->downsamp << 8) | collect->trigger);
	ptr[8] = htol32(0xFFFFFFFF);
	ptr[9] = htol32((collect->post_dur << 16) | collect->pre_dur);
	ptr[10] = htol32((phy_utils_read_phyreg(pi, HTPHY_RxIQCompA(0))) |
	          (phy_utils_read_phyreg(pi, HTPHY_RxIQCompB(0)) << 16));
	ptr[11] = htol32((phy_utils_read_phyreg(pi, HTPHY_RxIQCompA(1))) |
	          (phy_utils_read_phyreg(pi, HTPHY_RxIQCompB(1)) << 16));
	ptr[12] = htol32((phy_utils_read_phyreg(pi, HTPHY_RxIQCompA(2))) |
	          (phy_utils_read_phyreg(pi, HTPHY_RxIQCompB(2)) << 16));
	ptr[13] = htol32(((collect->filter ? 1 : 0) << 24) | ((collect->agc ? 1 : 0) << 16) |
		(sample_rate << 8) | sample_bitwidth);
	ptr[14] = htol32(((dBsample_to_dBm_sub << 16) | agc_gain[0]));
	ptr[15] = htol32(((agc_gain[2] << 16) | agc_gain[1]));
	ptr[16] = htol32(0xFFFFFFFF);
	ptr[17] = htol32(0xFFFFFFFF);
	ptr[18] = htol32(0xFFFFFFFF);
	ptr[19] = htol32(0xFFFFFFFF);
	PHY_TRACE(("wl%d: %s: pfirst 0x%x plast 0x%x pstart 0x%x pstop 0x%x\n", pi->sh->unit,
		__FUNCTION__, pi_ht->pfirst, pi_ht->plast, pi_ht->pstart, pi_ht->pstop));
	PHY_TRACE(("wl%d: %s Capture length = %d words\n", pi->sh->unit, __FUNCTION__, cnt));
	return BCME_OK;
}

int
phy_ht_sample_data(phy_info_t *pi, wl_sampledata_t *sample_data, void *b)
{
	uint32 data, cnt, bufsize, seq;
	phy_info_htphy_t *pi_ht = (phy_info_htphy_t *)pi->u.pi_htphy;
	uint8* head = (uint8 *)b;
	uint32* buf = (uint32 *)(head + sizeof(wl_sampledata_t));
	/* buf2 is used when sampledata is unpacked for
	 * version WL_SAMPLEDATA_T_VERSION_SPEC_AN
	 */
	int16* buf2 = (int16 *)(head + sizeof(wl_sampledata_t));
	int i;

	bufsize = ltoh32(sample_data->length) - sizeof(wl_sampledata_t);

	if (sample_data->version == (WL_SAMPLEDATA_T_VERSION_SPEC_AN)) {
		/* convert to # of (4*num_cores)--byte  words */
		bufsize = bufsize / (pi->pubpi.phy_corenum * 4);
	} else {
		/* convert to # of 4--byte words */
		bufsize = bufsize >> 2;
	}

	/* get the last sequence number */
	seq = ltoh32(sample_data->flag) & 0xff;

	/* Saturate sequence number to 0xff */
	seq = (seq < 0xff) ? (seq + 1) : 0xff;

	/* write back to data struct */
	sample_data->flag = htol32(seq);

	PHY_TRACE(("wl%d: %s: bufsize(words) %d flag 0x%x\n", pi->sh->unit, __FUNCTION__,
		bufsize, sample_data->flag));

	wlapi_bmac_templateptr_wreg(pi->sh->physhim, pi_ht->pfirst << 2);

	/* Currently only 3 cores (and collect mode 0) are supported
	 * for version WL_SAMPLEDATA_T_VERSION_SPEC_AN
	 */
	if ((sample_data->version == WL_SAMPLEDATA_T_VERSION_SPEC_AN) &&
	    (pi->pubpi.phy_corenum != 3))  {
		/* No more data to read */
		sample_data->flag = sample_data->flag & 0xff;

		/* No bytes were read */
		sample_data->length = 0;

		bcopy((uint8 *)sample_data, head, sizeof(wl_sampledata_t));
		PHY_ERROR(("wl%d: %s: Number of cores not equal to 3! \n",
			pi->sh->unit, __FUNCTION__));
		return BCME_ERROR;
	}

	/* Initialization for version WL_SAMPLEDATA_T_VERSION_SPEC_AN: */
	if ((sample_data->version == WL_SAMPLEDATA_T_VERSION_SPEC_AN) && (seq == 1)) {
		bool capture_start = FALSE;

		/* Search for and sync to a sample with a valid 2-bit '00' alignment pattern */
		while ((!capture_start) && (pi_ht->pfirst != pi_ht->plast)) {
			data = wlapi_bmac_templatedata_rreg(pi->sh->physhim);
			/* wrap around end of fifo if necessary */
			if (pi_ht->pfirst == pi_ht->pstop) {
				wlapi_bmac_templateptr_wreg(pi->sh->physhim, pi_ht->pstart << 2);
				pi_ht->pfirst = pi_ht->pstart;
				PHY_TRACE(("wl%d: %s TXFIFO wrap around\n",
					pi->sh->unit, __FUNCTION__));
			} else {
				pi_ht->pfirst++;
			}

			/* Check for alignment pattern 0x3 in the captured word */
			if (((data >> 30) & 0x3) == 0x3) {
				/* Read and discard one 32-bit word to
				 * move to where the next sample
				 * (with alignment pattern '00') starts
				 */
				data = wlapi_bmac_templatedata_rreg(pi->sh->physhim);

				/* wrap around end of fifo if necessary */
				if (pi_ht->pfirst == pi_ht->pstop) {
					wlapi_bmac_templateptr_wreg(pi->sh->physhim,
					      pi_ht->pstart << 2);
					pi_ht->pfirst = pi_ht->pstart;
					PHY_TRACE(("wl%d: %s TXFIFO wrap around\n",
						pi->sh->unit, __FUNCTION__));
				} else {
					pi_ht->pfirst++;
				}

				if (pi_ht->pfirst != pi_ht->plast) {
					capture_start = TRUE;
				}
			}
		}

		if (capture_start == FALSE) {
			/* ERROR: No starting pattern was found! */
			/* No more data to read */
			sample_data->flag = sample_data->flag & 0xff;

			/* No bytes were read */
			sample_data->length = 0;

			bcopy((uint8 *)sample_data, head, sizeof(wl_sampledata_t));
			PHY_ERROR(("wl%d: %s: Starting pattern not found! \n",
				pi->sh->unit, __FUNCTION__));
			return BCME_ERROR;
		}
	}

	/* start writing samples to buffer */
	cnt = 0;

	while ((cnt < bufsize) && (pi_ht->pfirst != pi_ht->plast)) {
		if (sample_data->version == WL_SAMPLEDATA_T_VERSION_SPEC_AN) {
			/* Unpack collected samples and write to buffer */
			uint32 data1[2];
			int16 isample[PHY_CORE_MAX];
			int16 qsample[PHY_CORE_MAX];

			for (i = 0; ((i < 2) && (pi_ht->pfirst != pi_ht->plast)); i++) {
				data1[i] = wlapi_bmac_templatedata_rreg(pi->sh->physhim);
				/* wrap around end of fifo if necessary */
				if (pi_ht->pfirst == pi_ht->pstop) {
					wlapi_bmac_templateptr_wreg(pi->sh->physhim,
					      pi_ht->pstart << 2);
					pi_ht->pfirst = pi_ht->pstart;
					PHY_TRACE(("wl%d: %s TXFIFO wrap around\n",
						pi->sh->unit, __FUNCTION__));
				} else {
					pi_ht->pfirst++;
				}
			}

			/* Unpack samples only if two 32-bit words have
			 * been successfully read from TX FIFO
			 */
			if (i == 2) {
				/* Unpack and perform sign extension: */
				uint16 temp;

				/* Core 0: */
				temp = (uint16)(data1[0] & 0x3ff);
				isample[0] = (temp & 0x200) ? (int16)(temp | 0xfc00) : temp;
				temp = (uint16)((data1[0] >> 10) & 0x3ff);
				qsample[0] = (temp & 0x200) ? (int16)(temp | 0xfc00) : temp;

				/* Core 1: */
				temp = (uint16)((data1[0] >> 20) & 0x3ff);
				isample[1] = (temp & 0x200) ? (int16)(temp | 0xfc00) : temp;
				temp = (uint16)(data1[1] & 0x3ff);
				qsample[1] = (temp & 0x200) ? (int16)(temp | 0xfc00) : temp;

				/* Core 2: */
				temp = (uint16)((data1[1] >> 10) & 0x3ff);
				isample[2] = (temp & 0x200) ? (int16)(temp | 0xfc00) : temp;
				temp = (uint16)((data1[1] >> 20) & 0x3ff);
				qsample[2] = (temp & 0x200) ? (int16)(temp | 0xfc00) : temp;

				/* Write to buffer in 2-byte words */
				buf2[6*cnt]     = isample[0];
				buf2[6*cnt + 1] = qsample[0];
				buf2[6*cnt + 2] = isample[1];
				buf2[6*cnt + 3] = qsample[1];
				buf2[6*cnt + 4] = isample[2];
				buf2[6*cnt + 5] = qsample[2];

				cnt++;
			}

		} else {
			/* Write collected samples as-is to buffer */

			data = wlapi_bmac_templatedata_rreg(pi->sh->physhim);
			/* write one 4-byte word */
			buf[cnt++] = htol32(data);
			/* wrap around end of fifo if necessary */
			if (pi_ht->pfirst == pi_ht->pstop) {
				wlapi_bmac_templateptr_wreg(pi->sh->physhim, pi_ht->pstart << 2);
				pi_ht->pfirst = pi_ht->pstart;
				PHY_TRACE(("wl%d: %s TXFIFO wrap around\n",
					pi->sh->unit, __FUNCTION__));
			} else {
				pi_ht->pfirst++;
			}
		}
	}

	PHY_TRACE(("wl%d: %s: Data fragment completed (pfirst %d plast %d)\n",
		pi->sh->unit, __FUNCTION__, pi_ht->pfirst, pi_ht->plast));
	if (pi_ht->pfirst != pi_ht->plast) {
		sample_data->flag |= htol32(WL_SAMPLEDATA_MORE_DATA);
	}

	/* update to # of bytes read */
	if (sample_data->version == WL_SAMPLEDATA_T_VERSION_SPEC_AN) {
		sample_data->length = htol16((cnt * 4 * pi->pubpi.phy_corenum));
	} else {
		sample_data->length = htol16((cnt << 2));
	}

	bcopy((uint8 *)sample_data, head, sizeof(wl_sampledata_t));
	PHY_TRACE(("wl%d: %s: Capture length = %d words\n", pi->sh->unit, __FUNCTION__, cnt));
	return BCME_OK;
}
#endif /* SAMPLE_COLLECT */

#if defined(WLTEST)

void
wlc_phy_bphy_testpattern_htphy(phy_info_t *pi, uint8 testpattern, uint16 rate_reg, bool enable)
{
	uint16 clip_off[] = {0xffff, 0xffff, 0xffff, 0xffff};
	uint16 core;

	if (enable == TRUE) {
		/* Save existing clip_detect state */
		wlc_phy_clip_det_htphy(pi, 0, pi->phy_clip_state);
		/* Turn OFF all clip detections */
		wlc_phy_clip_det_htphy(pi, 1, clip_off);
		/* Save existing classifier state */
		pi->phy_classifier_state = wlc_phy_classifier_htphy(pi, 0, 0);
		/* Turn OFF all classifcation */
		wlc_phy_classifier_htphy(pi, HTPHY_ClassifierCtrl_classifierSel_MASK, 4);
		/* Trigger SamplePlay Start */
		phy_utils_write_phyreg(pi,  HTPHY_sampleCmd, 0x1);
		/* Save BPHY test and testcontrol registers */
		pi->old_bphy_test = phy_utils_read_phyreg(pi, (HTPHY_TO_BPHY_OFF + BPHY_TEST));
		pi->old_bphy_testcontrol = phy_utils_read_phyreg(pi, HTPHY_bphytestcontrol);
		if (testpattern == HTPHY_TESTPATTERN_BPHY_EVM) {
			phy_utils_and_phyreg(pi, (HTPHY_TO_BPHY_OFF + BPHY_TEST),
			            ~(TST_TXTEST_ENABLE|TST_TXTEST_RATE|TST_TXTEST_PHASE));
			phy_utils_or_phyreg(pi, (HTPHY_TO_BPHY_OFF + BPHY_TEST),
			           TST_TXTEST_ENABLE | rate_reg);

		} else {
			phy_utils_and_phyreg(pi, (HTPHY_TO_BPHY_OFF + BPHY_TEST), 0xfc00);
			phy_utils_or_phyreg(pi, (HTPHY_TO_BPHY_OFF + BPHY_TEST), 0x0228);
		}
		/* Configure htphy's bphy testcontrol */
		phy_utils_write_phyreg(pi, HTPHY_bphytestcontrol, 0xF);
		/* Force Rx2Tx */
		wlc_phy_force_rfseq_htphy(pi, HTPHY_RFSEQ_RX2TX);

		FOREACH_CORE(pi, core) {
			MOD_PHYREGC(pi, HTPHY_AfectrlOverride, core, dac_pd, 1);
			MOD_PHYREGC(pi, HTPHY_Afectrl, core, dac_pd, 0);
		}

		PHY_CAL(("wl%d: %s: Turning  ON: TestPattern = %3d  "
		         "en = %3d",
		         pi->sh->unit, __FUNCTION__, testpattern, enable));

	} else if (enable == FALSE) {
		/* Turn OFF BPHY testpattern */
		/* Turn off AFE Override */

		FOREACH_CORE(pi, core) {
			MOD_PHYREGC(pi, HTPHY_AfectrlOverride, core, dac_pd, 0);
			MOD_PHYREGC(pi, HTPHY_Afectrl, core, dac_pd, 1);
		}

		/* Force Tx2Rx */
		wlc_phy_force_rfseq_htphy(pi, HTPHY_RFSEQ_TX2RX);
		/* Restore BPHY test and testcontrol registers */
		phy_utils_write_phyreg(pi, HTPHY_bphytestcontrol, pi->old_bphy_testcontrol);
		phy_utils_write_phyreg(pi, (HTPHY_TO_BPHY_OFF + BPHY_TEST), pi->old_bphy_test);
		/* Turn ON receive packet activity */
		wlc_phy_clip_det_htphy(pi, 1, pi->phy_clip_state);
		wlc_phy_classifier_htphy(pi, HTPHY_ClassifierCtrl_classifierSel_MASK,
		                        pi->phy_classifier_state);

		/* Trigger samplePlay Stop */
		phy_utils_write_phyreg(pi,  HTPHY_sampleCmd, 0x2);

		PHY_CAL(("wl%d: %s: Turning OFF: TestPattern = %3d  "
		         "en = %3d",
		         pi->sh->unit, __FUNCTION__, testpattern, enable));
	}
}

void
wlc_phy_test_scraminit_htphy(phy_info_t *pi, int8 init)
{
	uint16 mask, value;

	/* PR 38226: routine to allow special testmodes where the scrambler is
	 * forced to a fixed init value, hence, the same bit sequence into
	 * the MAC produces the same constellation point sequence every
	 * time
	 */

	if (init < 0) {
		/* auto: clear Mode bit so that scrambler LFSR will be free
		 * running.  ok to leave scramindexctlEn and initState in
		 * whatever current condition, since their contents are unused
		 * when free running, but for ease of reg diffs, just write
		 * 0x7f to them for repeatability.
		 */
		mask = (HTPHY_ScramSigCtrl_scramCtrlMode_MASK |
		        HTPHY_ScramSigCtrl_scramindexctlEn_MASK |
		        HTPHY_ScramSigCtrl_initStateValue_MASK);
		value = ((0 << HTPHY_ScramSigCtrl_scramCtrlMode_SHIFT) |
		         HTPHY_ScramSigCtrl_initStateValue_MASK);
		phy_utils_mod_phyreg(pi, HTPHY_ScramSigCtrl, mask, value);
	} else {
		/* fixed init: set Mode bit, clear scramindexctlEn, and write
		 * init to initState, so that scrambler LFSR will be
		 * initialized with specified value for each transmission.
		 */
		mask = (HTPHY_ScramSigCtrl_scramCtrlMode_MASK |
		        HTPHY_ScramSigCtrl_scramindexctlEn_MASK |
		        HTPHY_ScramSigCtrl_initStateValue_MASK);
		value = (HTPHY_ScramSigCtrl_scramCtrlMode_MASK |
		         (HTPHY_ScramSigCtrl_initStateValue_MASK &
		          (init << HTPHY_ScramSigCtrl_initStateValue_SHIFT)));
		phy_utils_mod_phyreg(pi, HTPHY_ScramSigCtrl, mask, value);
	}
}

int8
wlc_phy_test_tssi_htphy(phy_info_t *pi, int8 ctrl_type, int8 pwr_offs)
{
	int8 tssi;

	switch (ctrl_type & 0x3) {
	case 0:
	case 1:
	case 2:
		tssi = phy_utils_read_phyreg(pi, HTPHY_TSSIBiasVal((ctrl_type & 0x3))) & 0xff;
		break;
	default:
		tssi = -127;
	}
	return (tssi);
}

void
wlc_phy_gpiosel_htphy(phy_info_t *pi, uint16 sel)
{
	uint32 mc;

	/* kill OutEn */
	phy_utils_write_phyreg(pi, HTPHY_gpioLoOutEn, 0x0);
	phy_utils_write_phyreg(pi, HTPHY_gpioHiOutEn, 0x0);

	/* take over gpio control from cc */
	si_gpiocontrol(pi->sh->sih, 0xffff, 0xffff, GPIO_DRV_PRIORITY);

	/* clear the mac selects, disable mac oe */
	mc = R_REG(pi->sh->osh, &pi->regs->maccontrol);
	mc &= ~MCTL_GPOUT_SEL_MASK;
	W_REG(pi->sh->osh, &pi->regs->maccontrol, mc);
	W_REG(pi->sh->osh, &pi->regs->psm_gpio_oe, 0x0);

	/* set up htphy GPIO sel */
	phy_utils_write_phyreg(pi, HTPHY_gpioSel, sel);
	phy_utils_write_phyreg(pi, HTPHY_gpioLoOutEn, 0xffff);
	phy_utils_write_phyreg(pi, HTPHY_gpioHiOutEn, 0xffff);
}
void
wlc_phy_pavars_get_htphy(phy_info_t *pi, uint16 *buf, uint16 band, uint16 core)
{
	srom_pwrdet_t	*pwrdet  = pi->pwrdet;

	ASSERT(PHY_CORE_MAX > core);
	ASSERT((CH_2G_GROUP + CH_5G_GROUP_EXT) > band);

	buf[0] = pwrdet->pwrdet_a1[core][band];
	buf[1] = pwrdet->pwrdet_b0[core][band];
	buf[2] = pwrdet->pwrdet_b1[core][band];
}

void
wlc_phy_pavars_set_htphy(phy_info_t *pi, uint16 *buf, uint16 band, uint16 core)
{
	srom_pwrdet_t	*pwrdet  = pi->pwrdet;

	pwrdet->pwrdet_a1[core][band] = buf[0];
	pwrdet->pwrdet_b0[core][band] = buf[1];
	pwrdet->pwrdet_b1[core][band] = buf[2];
}
#endif // endif

#if defined(BCMDBG) || defined(WLTEST)
int
wlc_phy_freq_accuracy_htphy(phy_info_t *pi, int channel)
{
	phy_info_htphy_t *pi_ht = (phy_info_htphy_t *)pi->u.pi_htphy;
	int bcmerror = BCME_OK;

	if (channel == 0) {
		wlc_phy_stopplayback_htphy(pi);
		/* restore the old BBconfig, to restore resampler setting */
		phy_utils_write_phyreg(pi, HTPHY_BBConfig, pi_ht->saved_bbconf);
		wlc_phy_resetcca_htphy(pi);
	} else {
		/* Disable the re-sampler (in case we are in spur avoidance mode) */
		pi_ht->saved_bbconf = phy_utils_read_phyreg(pi, HTPHY_BBConfig);
		MOD_PHYREG(pi, HTPHY_BBConfig, resample_clk160, 0);
		/* use 151 since that should correspond to nominal tx output power */
		bcmerror = wlc_phy_tx_tone_htphy(pi, 0, 151, 0, 0, TRUE);
	}
	return bcmerror;
}
#endif // endif

/* reset both aci and/or noise states, hardware and software */
void
wlc_phy_aci_noise_reset_htphy(phy_info_t *pi, uint channel, bool clear_aci_state,
	bool clear_noise_state, bool disassoc)
{
	bool suspend;

	if ((pi->interf->curr_home_channel == (uint8) channel) &&
		(disassoc == FALSE)) {
		/* same channel, not a disassoc, do not reset */
		PHY_ACI(("same home channel = %d, so do not reset aci/noise state\n",
			pi->interf->curr_home_channel));
		return;
	}

	PHY_ACI(("wlc_phy_acimode_noisemode_reset: reset aci state = %d,"
		" reset noise state = %d\n",
		clear_aci_state, clear_noise_state));

	suspend = !(R_REG(pi->sh->osh, &pi->regs->maccontrol) & MCTL_EN_MAC);
	if (!suspend) {
		/* suspend mac */
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
	}

	if (clear_aci_state) {
		wlc_phy_acimode_reset_htphy(pi);
	}

	if (clear_noise_state) {
		wlc_phy_noisemode_reset_htphy(pi);
	}

	pi->interf->curr_home_channel = (uint8) channel;

	/* unsuspend mac */
	if (!suspend) {
		wlapi_enable_mac(pi->sh->physhim);
	}
}

void
wlc_phy_noisemode_upd_htphy(phy_info_t *pi)
{
	uint16 noise_noassoc_enter_th;
	uint16 noise_assoc_enter_th;
	bool suspend;

	/* suspend mac if haven't done so */
	suspend = !(R_REG(pi->sh->osh, &pi->regs->maccontrol) & MCTL_EN_MAC);
	if (!suspend) {
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
	}

	if (pi->interf->scanroamtimer != 0) {
		/* unsuspend mac */
		if (!suspend) {
			wlapi_enable_mac(pi->sh->physhim);
		}

		/* have not updated moving averages */
		return;
	}

	noise_assoc_enter_th = pi->interf->noise.nphy_noise_assoc_enter_th;
	noise_noassoc_enter_th = pi->interf->noise.nphy_noise_noassoc_enter_th;

	if (ASSOC_INPROG_PHY(pi) || PUB_NOT_ASSOC(pi)) {
		/* not associated.  check for noise inband */
		wlc_phy_noisemode_glitch_chk_adj_htphy(pi,
			noise_noassoc_enter_th,
			pi->interf->noise.nphy_noise_noassoc_glitch_th_up,
			pi->interf->noise.nphy_noise_noassoc_glitch_th_dn);
	} else {
		if (PUB_NOT_ASSOC(pi)) {
			/* unsuspend mac */
			if (!suspend) {
				wlapi_enable_mac(pi->sh->physhim);
			}
			return;
		}
		/* only desense bphy if associated and in 2G and on home channel */
		if (CHSPEC_IS2G(pi->radio_chanspec) &&
		    (CHSPEC_CHANNEL(pi->radio_chanspec) == pi->interf->curr_home_channel))
			wlc_phy_bphynoisemode_glitch_chk_adj_htphy(pi);

		if ((pi->sh->now % NPHY_NOISE_CHECK_PERIOD == 0)) {
			wlc_phy_noisemode_glitch_chk_adj_htphy(pi,
				noise_assoc_enter_th,
				pi->interf->noise.nphy_noise_assoc_glitch_th_up,
				pi->interf->noise.nphy_noise_assoc_glitch_th_dn);
		}

	}
	PHY_ACI(("wlc_noise_reduction:  aci ma is %d, ofdm_ma = %d, badplcp_ma = %d,"
	         " crsminpwrl0 = 0x%x, crsminpwru0 = 0x%x, crsminpwr index = %d,"
	         " bphy_desense_index = %d, init gain = 0x%x, current channel = %d,"
	         " home channel = %d \n", pi->interf->aci.glitch_ma,
	         pi->interf->noise.ofdm_glitch_ma, pi->interf->badplcp_ma,
	         pi->interf->crsminpwrl0, pi->interf->crsminpwru0, pi->interf->crsminpwr_index,
	         pi->interf->bphy_desense_index, pi->interf->init_gain_core1,
	         CHSPEC_CHANNEL(pi->radio_chanspec), pi->interf->curr_home_channel));

	/* unsuspend mac */
	if (!suspend) {
		wlapi_enable_mac(pi->sh->physhim);
	}
}

void
wlc_phy_aci_noise_upd_htphy(phy_info_t *pi)
{
	uint16 noise_assoc_rx_glitch_badplcp_enter_th;
	uint16 noise_noassoc_enter_th;
	uint16 noise_assoc_enter_th;
	bool suspend;

	/* suspend mac if haven't done so */
	suspend = !(R_REG(pi->sh->osh, &pi->regs->maccontrol) & MCTL_EN_MAC);
	if (!suspend) {
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
	}

	noise_assoc_enter_th = pi->interf->noise.nphy_noise_assoc_enter_th;
	noise_noassoc_enter_th = pi->interf->noise.nphy_noise_noassoc_enter_th;
	noise_assoc_rx_glitch_badplcp_enter_th =
		pi->interf->noise.nphy_noise_assoc_rx_glitch_badplcp_enter_th;

	if (pi->interf->scanroamtimer != 0) {
		/* unsuspend mac */
		if (!suspend) {
			wlapi_enable_mac(pi->sh->physhim);
		}

		/* have not updated moving averages */
		return;
	}

	if (ASSOC_INPROG_PHY(pi) || PUB_NOT_ASSOC(pi)) {
		/* not associated.  check for noise inband */
		wlc_phy_noisemode_glitch_chk_adj_htphy(pi,
			noise_noassoc_enter_th,
			pi->interf->noise.nphy_noise_noassoc_glitch_th_up,
			pi->interf->noise.nphy_noise_noassoc_glitch_th_dn);
	} else {
		/* currently associated */
		if (!(pi->aci_state & ACI_ACTIVE)) {
			/* not in ACI mitigation mode */
			if (pi->sh->now % NPHY_ACI_CHECK_PERIOD == 0) {
				uint16 default_crsmin = MAX(pi->interf->crsminpwrthld_20U_stored,
				                            pi->interf->crsminpwrthld_20L_stored);
				bool ofdm_desense_active = (pi->interf->crsminpwr_index !=
				                            default_crsmin);
				bool bphy_desense_active = (pi->interf->bphy_desense_index > 0);
				PHY_ACI(("Interf Mode 4, pi->interf->aci.glitch_ma = %d,"
					" pi->interf->badplcp_ma = %d, sum = %d,"
					" crs_min_pwr = u 0x%x, l 0x%x\n",
					pi->interf->aci.glitch_ma, pi->interf->badplcp_ma,
					pi->interf->aci.glitch_ma + pi->interf->badplcp_ma,
					phy_utils_read_phyreg(pi, HTPHY_crsminpoweru0),
						phy_utils_read_phyreg(pi, HTPHY_crsminpowerl0)));
				if (((pi->interf->aci.glitch_ma + pi->interf->badplcp_ma) >=
				     noise_assoc_rx_glitch_badplcp_enter_th) ||
				    ofdm_desense_active || bphy_desense_active) {
					/* XXX
					 * check for aci if either glitch count is high or if
					 * bphy/ofdm desense has kicked in (for the latter case,
					 * glitch count is not a reliable metric as it can be
					 * low on account of desense).
					 */
					wlc_phy_acimode_upd_htphy(pi);
				}
			}

			if (pi->interf->aci.detect_total == 0) {
				/* aci scan didn't indicate aci present */
				/* so, check for excessive crs glitch */

				/* only desense bphy if associated and in 2G and on home channel */
				if (CHSPEC_IS2G(pi->radio_chanspec) &&
				    (CHSPEC_CHANNEL(pi->radio_chanspec) ==
				        pi->interf->curr_home_channel))
					wlc_phy_bphynoisemode_glitch_chk_adj_htphy(pi);

				wlc_phy_noisemode_glitch_chk_adj_htphy(pi,
					noise_assoc_enter_th,
					pi->interf->noise.nphy_noise_assoc_glitch_th_up,
					pi->interf->noise.nphy_noise_assoc_glitch_th_dn);

			} else {
				/* ACI mitigation just occurred. reset state */
				pi->interf->noise.noise_glitch_high_detect_total = 0;
				pi->interf->noise.noise_glitch_low_detect_total = 0;
				pi->interf->noise.bphy_noise_glitch_low_detect_total = 0;
				pi->interf->noise.bphy_noise_glitch_high_detect_total = 0;
			}
		} else {
			/* already active in ACI mitigation mode, check to get out */
			if (((pi->sh->now - pi->aci_start_time)
				% pi->aci_exit_check_period) == 0) {
				wlc_phy_acimode_upd_htphy(pi);
			}

			if (pi->interf->aci.detect_total >= 0) {
				/* in ACI mitigation, done transitioning,
				 * check for inband noise
				 */

				/* only desense bphy if associated and in 2G and on home channel */
				if (CHSPEC_IS2G(pi->radio_chanspec) &&
				    (CHSPEC_CHANNEL(pi->radio_chanspec) ==
				        pi->interf->curr_home_channel))
					wlc_phy_bphynoisemode_glitch_chk_adj_htphy(pi);

				wlc_phy_noisemode_glitch_chk_adj_htphy(pi,
					noise_assoc_enter_th,
					pi->interf->noise.nphy_noise_assoc_aci_glitch_th_up,
					pi->interf->noise.nphy_noise_assoc_aci_glitch_th_dn);
			}
		}
	}

	/* unsuspend mac */
	if (!suspend) {
		wlapi_enable_mac(pi->sh->physhim);
	}
}

static void
wlc_phy_bphynoisemode_glitch_chk_adj_htphy(phy_info_t *pi)
{
	uint16 total_glitch_badplcp = pi->interf->noise.bphy_glitch_ma +
	        pi->interf->noise.bphy_badplcp_ma;

	if (total_glitch_badplcp > pi->interf->noise.nphy_bphynoise_assoc_enter_th) {
		/* glitch count is high, could be due to inband noise */
		pi->interf->noise.bphy_noise_glitch_high_detect_total++;
		pi->interf->noise.bphy_noise_glitch_low_detect_total = 0;
	} else {
		/* glitch count not high */
		pi->interf->noise.bphy_noise_glitch_high_detect_total = 0;
		pi->interf->noise.bphy_noise_glitch_low_detect_total++;
	}

	if (pi->interf->noise.bphy_noise_glitch_high_detect_total >=
		pi->interf->noise.nphy_bphynoise_assoc_glitch_th_up) {
		/* we have more than glitch_th_up bphy
		 * glitches in a row. so, let's try raising the
		 * inband noise immunity
		 */
		if (total_glitch_badplcp < pi->interf->noise.nphy_bphynoise_assoc_high_th) {
			/* Desense by less */
			pi->interf->noise.nphy_bphynoise_crsidx_incr =
			        HTPHY_BPHYNOISE_CRSIDX_INCR_LO;
		} else {
			/* Desense by more */
			pi->interf->noise.nphy_bphynoise_crsidx_incr =
			        HTPHY_BPHYNOISE_CRSIDX_INCR_HI;
		}
		wlc_phy_bphynoisemode_set_htphy(pi, 1);
		pi->interf->noise.bphy_noise_glitch_high_detect_total = 0;
	} else if (pi->interf->noise.bphy_noise_glitch_low_detect_total > 0) {
		/* check to see if we can lower noise immunity */
		uint16 low_detect_total, undesense_wait, undesense_window;

		low_detect_total = pi->interf->noise.bphy_noise_glitch_low_detect_total;
		undesense_wait = (uint16) HTPHY_BPHYNOISE_ASSOC_UNDESENSE_WAIT;
		undesense_window = (uint16) HTPHY_BPHYNOISE_ASSOC_UNDESENSE_WINDOW;

		/* Reduce the wait time to undesense if glitch count has been low for longer */
		while (undesense_wait > 1) {
			if (low_detect_total <=  undesense_window) {
				break;
			}
			low_detect_total -= undesense_window;
			/* Halve the wait time */
			undesense_wait /= 2;
		}

		if ((low_detect_total % undesense_wait) == 0) {
			/* Undesense */
			wlc_phy_bphynoisemode_set_htphy(pi, 0);
		}
	}
}

static void
wlc_phy_noisemode_glitch_chk_adj_htphy(phy_info_t *pi, uint16 noise_enter_th,
	uint16 noise_glitch_th_up, uint16 noise_glitch_th_dn)
{
	if ((pi->interf->noise.ofdm_glitch_ma + pi->interf->noise.ofdm_badplcp_ma) >
		noise_enter_th) {
		/* glitch count is high, could be due to inband noise */
		pi->interf->noise.noise_glitch_high_detect_total++;
		pi->interf->noise.noise_glitch_low_detect_total = 0;
	} else {
		/* glitch count not high */
		pi->interf->noise.noise_glitch_high_detect_total = 0;
		pi->interf->noise.noise_glitch_low_detect_total++;
	}

	if (pi->interf->noise.noise_glitch_high_detect_total >=
		noise_glitch_th_up) {
		/* we have more than noise_glitch_th_up ofdm
		 * glitches in a row. so, let's try raising the
		 * inband noise immunity
		 */
		wlc_phy_noisemode_set_htphy(pi, 1);
		pi->interf->noise.noise_glitch_high_detect_total = 0;
	} else {
		/* check to see if we can lower noise immunity */
		if (pi->interf->noise.noise_glitch_low_detect_total >=
			noise_glitch_th_dn) {
			/* we have more than noise_glitch_th_dn
			 * non detects in a row.  try lowering noise threshold
			 */
			wlc_phy_noisemode_set_htphy(pi, 0);
		}
	}

}

static void
wlc_phy_aci_noise_store_values_htphy(phy_info_t *pi)
{
	bool suspend;

	/* suspend mac if haven't done so */
	suspend = !(R_REG(pi->sh->osh, &pi->regs->maccontrol) & MCTL_EN_MAC);
	if (!suspend) {
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
	}
	/* store current chanspec */
	pi->interf->radio_chanspec_stored = pi->radio_chanspec;

	/* store off orig values for aci/noise mitigation */
	pi->interf->init_gain_code_core0_stored =
		phy_utils_read_phyreg(pi, HTPHY_Core0InitGainCodeA2059);
	pi->interf->init_gain_code_core1_stored =
		phy_utils_read_phyreg(pi, HTPHY_Core1InitGainCodeA2059);
	pi->interf->init_gain_code_core2_stored =
		phy_utils_read_phyreg(pi, HTPHY_Core2InitGainCodeA2059);
	pi->interf->init_gain_codeb_core0_stored =
		phy_utils_read_phyreg(pi, HTPHY_Core0InitGainCodeB2059);
	pi->interf->init_gain_codeb_core1_stored =
		phy_utils_read_phyreg(pi, HTPHY_Core1InitGainCodeB2059);
	pi->interf->init_gain_codeb_core2_stored =
		phy_utils_read_phyreg(pi, HTPHY_Core2InitGainCodeB2059);

	pi->interf->crsminpwrthld_20L_stored =
		(uint16) (phy_utils_read_phyreg(pi, HTPHY_crsminpowerl0) & 0xff);
	pi->interf->crsminpwrthld_20U_stored =
		(uint16) (phy_utils_read_phyreg(pi, HTPHY_crsminpoweru0) & 0xff);

	pi->interf->digigainlimit0_stored = phy_utils_read_phyreg(pi, HTPHY_DigiGainLimit0);
	if (CHSPEC_IS2G(pi->radio_chanspec))
		pi->interf->peakenergyl_stored = phy_utils_read_phyreg(pi, (HTPHY_TO_BPHY_OFF
		                                                 + BPHY_PEAK_ENERGY_LO));

	wlc_phy_table_read_htphy(pi, HTPHY_TBL_ID_RFSEQ, pi->pubpi.phy_corenum, 0x106, 16,
		&(pi->interf->init_gain_table_stored[0]));

	pi->interf->clip1_hi_gain_code_core0_stored =
		phy_utils_read_phyreg(pi, HTPHY_Core0clipHiGainCodeA2059);
	pi->interf->clip1_hi_gain_code_core1_stored =
		phy_utils_read_phyreg(pi, HTPHY_Core1clipHiGainCodeA2059);
	pi->interf->clip1_hi_gain_code_core2_stored =
		phy_utils_read_phyreg(pi, HTPHY_Core2clipHiGainCodeA2059);
	pi->interf->clip1_hi_gain_codeb_core0_stored =
		phy_utils_read_phyreg(pi, HTPHY_Core0clipHiGainCodeB2059);
	pi->interf->clip1_hi_gain_codeb_core1_stored =
		phy_utils_read_phyreg(pi, HTPHY_Core1clipHiGainCodeB2059);
	pi->interf->clip1_hi_gain_codeb_core2_stored =
		phy_utils_read_phyreg(pi, HTPHY_Core2clipHiGainCodeB2059);

	pi->interf->nb_clip_thresh_core0_stored =
		phy_utils_read_phyreg(pi, HTPHY_Core0nbClipThreshold);
	pi->interf->nb_clip_thresh_core1_stored =
		phy_utils_read_phyreg(pi, HTPHY_Core1nbClipThreshold);
	pi->interf->nb_clip_thresh_core2_stored =
		phy_utils_read_phyreg(pi, HTPHY_Core2nbClipThreshold);

	wlc_phy_table_read_htphy(pi, 4, 4, 0x10, 16,
		pi->interf->init_ofdmlna2gainchange_stored);
	wlc_phy_table_read_htphy(pi, 4, 4, 0x50, 16,
		pi->interf->init_ccklna2gainchange_stored);

	/* clipLO gain */
	pi->interf->clip1_lo_gain_code_core0_stored =
		phy_utils_read_phyreg(pi, HTPHY_Core0cliploGainCodeA2059);
	pi->interf->clip1_lo_gain_code_core1_stored =
		phy_utils_read_phyreg(pi, HTPHY_Core1cliploGainCodeA2059);
	pi->interf->clip1_lo_gain_code_core2_stored =
		phy_utils_read_phyreg(pi, HTPHY_Core2cliploGainCodeA2059);
	pi->interf->clip1_lo_gain_codeb_core0_stored =
		phy_utils_read_phyreg(pi, HTPHY_Core0cliploGainCodeB2059);
	pi->interf->clip1_lo_gain_codeb_core1_stored =
		phy_utils_read_phyreg(pi, HTPHY_Core1cliploGainCodeB2059);
	pi->interf->clip1_lo_gain_codeb_core2_stored =
		phy_utils_read_phyreg(pi, HTPHY_Core2cliploGainCodeB2059);

	pi->interf->w1_clip_thresh_core0_stored =
		phy_utils_read_phyreg(pi, HTPHY_Core0clipwbThreshold2059);
	pi->interf->w1_clip_thresh_core1_stored =
		phy_utils_read_phyreg(pi, HTPHY_Core1clipwbThreshold2059);
	pi->interf->w1_clip_thresh_core2_stored =
		phy_utils_read_phyreg(pi, HTPHY_Core2clipwbThreshold2059);

	pi->interf->radio_2057_core0_rssi_nb_gc_stored =
		phy_utils_read_radioreg(pi, RADIO_2059_NB_MASTER(0)) & 0x60;
	pi->interf->radio_2057_core1_rssi_nb_gc_stored =
		phy_utils_read_radioreg(pi, RADIO_2059_NB_MASTER(1)) & 0x60;
	pi->interf->radio_2057_core2_rssi_nb_gc_stored =
		phy_utils_read_radioreg(pi, RADIO_2059_NB_MASTER(2)) & 0x60;
	pi->interf->radio_2057_core0_rssi_wb1a_gc_stored =
		phy_utils_read_radioreg(pi, RADIO_2059_RSSI_GPAIOSEL_W1_IDACS(0)) & 0xc;
	pi->interf->radio_2057_core1_rssi_wb1a_gc_stored =
		phy_utils_read_radioreg(pi, RADIO_2059_RSSI_GPAIOSEL_W1_IDACS(1)) & 0xc;
	pi->interf->radio_2057_core2_rssi_wb1a_gc_stored =
		phy_utils_read_radioreg(pi, RADIO_2059_RSSI_GPAIOSEL_W1_IDACS(2)) & 0xc;
	pi->interf->radio_2057_core0_rssi_wb1g_gc_stored =
		phy_utils_read_radioreg(pi, RADIO_2059_RSSI_GPAIOSEL_W1_IDACS(0)) & 0x3;
	pi->interf->radio_2057_core1_rssi_wb1g_gc_stored =
		phy_utils_read_radioreg(pi, RADIO_2059_RSSI_GPAIOSEL_W1_IDACS(1)) & 0x3;
	pi->interf->radio_2057_core2_rssi_wb1g_gc_stored =
		phy_utils_read_radioreg(pi, RADIO_2059_RSSI_GPAIOSEL_W1_IDACS(2)) & 0x3;
	pi->interf->radio_2057_core0_rssi_wb2_gc_stored =
		phy_utils_read_radioreg(pi, RADIO_2059_W2_MASTER(0)) & 0xc0;
	pi->interf->radio_2057_core1_rssi_wb2_gc_stored =
		phy_utils_read_radioreg(pi, RADIO_2059_W2_MASTER(1)) & 0xc0;
	pi->interf->radio_2057_core2_rssi_wb2_gc_stored =
		phy_utils_read_radioreg(pi, RADIO_2059_W2_MASTER(2)) & 0xc0;
	/*
		pi->interf->energy_drop_timeout_len_stored =
			phy_utils_read_phyreg(pi, HTPHY_energydroptimeoutLen);
	*/
	pi->interf->ed_crs20L_assertthld0_stored=
		phy_utils_read_phyreg(pi, HTPHY_ed_crs20LAssertThresh0);
	pi->interf->ed_crs20L_assertthld1_stored=
		phy_utils_read_phyreg(pi, HTPHY_ed_crs20LAssertThresh1);
	pi->interf->ed_crs20L_deassertthld0_stored=
		phy_utils_read_phyreg(pi, HTPHY_ed_crs20LDeassertThresh0);
	pi->interf->ed_crs20L_deassertthld1_stored=
		phy_utils_read_phyreg(pi, HTPHY_ed_crs20LDeassertThresh1);

	pi->interf->ed_crs20U_assertthld0_stored=
		phy_utils_read_phyreg(pi, HTPHY_ed_crs20UAssertThresh0);
	pi->interf->ed_crs20U_assertthld1_stored=
		phy_utils_read_phyreg(pi, HTPHY_ed_crs20UAssertThresh1);
	pi->interf->ed_crs20U_deassertthld0_stored=
		phy_utils_read_phyreg(pi, HTPHY_ed_crs20UDeassertThresh0);
	pi->interf->ed_crs20U_deassertthld1_stored=
		phy_utils_read_phyreg(pi, HTPHY_ed_crs20UDeassertThresh1);

	/* unsuspend mac */
	if (!suspend) {
		wlapi_enable_mac(pi->sh->physhim);
	}
}

void
wlc_phy_noisemode_reset_htphy(phy_info_t *pi)
{
	bool suspend;

	/* suspend mac if haven't done so */
	suspend = !(R_REG(pi->sh->osh, &pi->regs->maccontrol) & MCTL_EN_MAC);
	if (!suspend) {
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
	}

	pi->interf->noise.noise_glitch_low_detect_total = 0;
	pi->interf->noise.noise_glitch_high_detect_total = 0;

	pi->interf->noise.nphy_noise_noassoc_glitch_th_up =
		HTPHY_NOISE_NOASSOC_GLITCH_TH_UP;
	pi->interf->noise.nphy_noise_noassoc_glitch_th_dn =
		HTPHY_NOISE_NOASSOC_GLITCH_TH_DN;
	pi->interf->noise.nphy_noise_assoc_glitch_th_up =
		HTPHY_NOISE_ASSOC_GLITCH_TH_UP;
	pi->interf->noise.nphy_noise_assoc_glitch_th_dn =
		HTPHY_NOISE_ASSOC_GLITCH_TH_DN;
	pi->interf->noise.nphy_bphynoise_noassoc_glitch_th_up =
		HTPHY_BPHYNOISE_NOASSOC_GLITCH_TH_UP;
	pi->interf->noise.nphy_bphynoise_noassoc_glitch_th_dn =
		HTPHY_BPHYNOISE_NOASSOC_GLITCH_TH_DN;
	pi->interf->noise.nphy_bphynoise_assoc_glitch_th_up =
		HTPHY_BPHYNOISE_ASSOC_GLITCH_TH_UP;

	pi->interf->noise.nphy_noise_assoc_aci_glitch_th_up =
		HTPHY_NOISE_ASSOC_ACI_GLITCH_TH_UP;
	pi->interf->noise.nphy_noise_assoc_aci_glitch_th_dn =
		HTPHY_NOISE_ASSOC_ACI_GLITCH_TH_DN;
	pi->interf->noise.nphy_noise_assoc_enter_th =
		HTPHY_NOISE_ASSOC_ENTER_TH;
	pi->interf->noise.nphy_noise_noassoc_enter_th =
		HTPHY_NOISE_NOASSOC_ENTER_TH;
	pi->interf->noise.nphy_noise_assoc_rx_glitch_badplcp_enter_th =
		HTPHY_NOISE_ASSOC_RX_GLITCH_BADPLCP_ENTER_TH;
	pi->interf->noise.nphy_noise_noassoc_crsidx_incr =
		HTPHY_NOISE_NOASSOC_CRSIDX_INCR;
	pi->interf->noise.nphy_noise_assoc_crsidx_incr =
		HTPHY_NOISE_ASSOC_CRSIDX_INCR;
	pi->interf->noise.nphy_noise_crsidx_decr =
		HTPHY_NOISE_CRSIDX_DECR;

	pi->interf->noise.nphy_bphynoise_assoc_enter_th =
	        HTPHY_BPHYNOISE_ASSOC_ENTER_TH;
	pi->interf->noise.nphy_bphynoise_assoc_high_th =
	        HTPHY_BPHYNOISE_ASSOC_HIGH_TH;
	pi->interf->noise.nphy_bphynoise_crsidx_incr =
		HTPHY_BPHYNOISE_CRSIDX_INCR_HI;
	pi->interf->noise.nphy_bphynoise_crsidx_decr =
		HTPHY_BPHYNOISE_CRSIDX_DECR;

	wlc_phy_aci_noise_shared_reset_htphy(pi);

	/* unsuspend mac */
	if (!suspend) {
		wlapi_enable_mac(pi->sh->physhim);
	}
}

/* noise mitigation mode, hardware and software change */
static void
wlc_phy_noisemode_set_htphy(phy_info_t *pi, bool raise)
{

	/* the following function only determines
	   the updated crsminpower
	 */
	wlc_phy_noise_adj_thresholds_htphy(pi, raise);
	/* the following function applies the updated crsminpower */
	wlc_phy_aci_noise_shared_hw_set_htphy(pi, FALSE, FALSE);
	wlc_phy_noise_sw_set_htphy(pi);

	PHY_ACI(("wlc_phy_noisemode_set_htphy: raise = %d, crsminpwr index = %d,"
	         " crsminpwrl0 = 0x%x, crsminpwru0 = 0x%x \n", raise, pi->interf->crsminpwr_index,
	         pi->interf->crsminpwrl0, pi->interf->crsminpwru0));
}

/* bphy noise mitigation mode, hardware and software change */
static void
wlc_phy_bphynoisemode_set_htphy(phy_info_t *pi, bool raise)
{
	int16 new_index;
	/* Bail if not in 2G */
	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		/* Currently mode 4 not supported with elna */
		if ((BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA) &&
		    (pi->sh->interference_mode == WLAN_AUTO_W_NOISE))
			return;
		new_index = wlc_phy_bphynoise_adj_thresholds_htphy(pi, raise);
		if (new_index != pi->interf->bphy_desense_index) {
			/* Apply new desense settings */
			wlc_phy_bphynoise_hw_set_htphy(pi, new_index, FALSE);
		}
		PHY_ACI(("%s: raise = %d, bphy_desense_index = %d, digigainlimit0 = 0x%x, "
		         "peakenergyL = 0x%x, initgain0 = 0x%x, initgain1 = 0x%x, "
		         "initgain2 = 0x%x \n", __FUNCTION__, raise, pi->interf->bphy_desense_index,
		         pi->interf->digigainlimit0, pi->interf->peakenergyl,
		         pi->interf->init_gain_rfseq[0], pi->interf->init_gain_rfseq[1],
		         pi->interf->init_gain_rfseq[2]));
	}
}

/* Determine new bphy desense index */
static int16
wlc_phy_bphynoise_adj_thresholds_htphy(phy_info_t *pi, bool raise)
{
	bphy_desense_info_t *bphy_desense_lut = pi->interf->bphy_desense_lut;
	uint16 bphy_desense_max_index = 0;
	int16 new_index = pi->interf->bphy_desense_index;
	int8 core;

	if (raise == 1) {
		/* increase the index (only if associated) */
		if (!PUB_NOT_ASSOC(pi)) {
			/* desense by 2 or 4 ticks (1 tick = 1dB) */
			new_index += pi->interf->noise.nphy_bphynoise_crsidx_incr;
		}
	} else if (raise == 0) {
		/* un-desense by 1 tick (1 tick = 1dB) */
		new_index -= pi->interf->noise.nphy_bphynoise_crsidx_decr;
	}

	/* limit index based on rssi value */
	wlc_phy_bphynoise_limit_crsmin_htphy(pi, &new_index);

	/* XXX FIXME:
	 * Should the following computation be moved to a common location?
	 * (to avoid repeating it on each call to this function)
	 */
	{
		/* determine max possible index based on init gain */
		uint8 min_biq1 = 10;
		uint16 i;
		FOREACH_CORE(pi, core) {
			min_biq1 = MIN(min_biq1, ((pi->interf->bphy_desense_base_initgain[core]
			                           & 0xf000) >> 12));
		}
		for (i = 1; i < pi->interf->bphy_desense_lut_size; i++) {
			if (bphy_desense_lut[i].drop_initBiq1 > min_biq1) {
				break;
			}
			bphy_desense_max_index = i;
		}
	}
	/* Bound bphy desense index */
	new_index = MIN(new_index, bphy_desense_max_index);
	new_index = MAX(new_index, 0);

	return new_index;
}

/* Update bphy desense LUT and apply new settings */
static void
wlc_phy_bphynoise_update_lut_htphy(phy_info_t *pi, bphy_desense_info_t *new_lut,
                                   uint16 new_lut_size, int16 new_min_sensitivity,
                                   uint16 *new_initgain)
{
	int8 new_index, core;

	/* Compute new bphy desense index */
	new_index = pi->interf->bphy_desense_index + pi->interf->bphy_min_sensitivity -
	        new_min_sensitivity;
	new_index = MAX(new_index, 0);

	PHY_ACI(("%s: old_min_sensitivity = %d, old_desense_index = %d, new_min_sensitivity = %d,"
	         " new_desense_index = %d\n", __FUNCTION__, pi->interf->bphy_min_sensitivity,
	         pi->interf->bphy_desense_index, new_min_sensitivity, new_index));

	/* Update sw settings */
	pi->interf->bphy_desense_lut = new_lut;
	pi->interf->bphy_desense_lut_size = new_lut_size;
	pi->interf->bphy_min_sensitivity = new_min_sensitivity;
	FOREACH_CORE(pi, core) {
		pi->interf->bphy_desense_base_initgain[core] = new_initgain[core];
	}

	/* Update hw settings */
	wlc_phy_bphynoise_hw_set_htphy(pi, new_index, TRUE);
}

/* bphy noise mitigation hardware modifications */
static void
wlc_phy_bphynoise_hw_set_htphy(phy_info_t *pi, uint16 new_index, bool lut_changed)
{
	bphy_desense_info_t *bphy_desense_lut = pi->interf->bphy_desense_lut;
	uint16 old_index = pi->interf->bphy_desense_index;
	int8 core;
	uint8 previous_channel = CHSPEC_CHANNEL(pi->interf->radio_chanspec_stored);
	uint8 current_channel = CHSPEC_CHANNEL(pi->radio_chanspec);
	bool suspend;

	/* XXX
	 * Currently aci mitigation not supported with elna in 2G; only modes 0,1 are supported.
	 * So bail if in mode 4 (bphy desense is not part of mode 3, so no need to check for it).
	 */
	if (CHSPEC_IS2G(pi->radio_chanspec) && (pi->sh->interference_mode == WLAN_AUTO_W_NOISE) &&
	    (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA))
		return;

	/* Handle any channel changes */
	if (previous_channel != current_channel) {
		/* XXX
		 * Shouldn't attempt to write to hw as the channel has changed but the hw defaults
		 * (i.e., desense-OFF values) for the new channel haven't been saved yet - this is
		 * indicated by radio_chanspec_stored being out-of-sync with the new channel.
		 *
		 * EXCEPTION TO THE ABOVE:
		 * May need to REMOVE any existing desense that was applied previously on the home
		 * channel. In that case, make sure that band/bw has not changed before restoring
		 * the default (desense OFF) values to phyregs/tables. Any channel-specific hw
		 * defaults for initgain,etc. are applied as part of subband_cust which gets called
		 * later - see chanspec_setup_htphy for code flow.
		 *
		 * EXCEPTION TO EXCEPTION:
		 * If band/bw did change, then a phy-reset has happened or is going to happen. This
		 * will reset all phyregs and tables to appropriate defaults for the new band/bw.
		 * So, do NOT restore old hw defaults stored from old channel/band/bw.
		 */
		if (new_index == 0) {
			/* removing desense */
			bool band_switch, bw_switch;
			band_switch = ((CHSPEC_IS2G(pi->radio_chanspec) &&
			                !CHSPEC_IS2G(pi->interf->radio_chanspec_stored)) ||
			               (!CHSPEC_IS2G(pi->radio_chanspec) &&
			                CHSPEC_IS2G(pi->interf->radio_chanspec_stored)));
			bw_switch = ((CHSPEC_IS20(pi->interf->radio_chanspec_stored) &&
			             !CHSPEC_IS20(pi->radio_chanspec)) ||
			             (!CHSPEC_IS20(pi->interf->radio_chanspec_stored) &&
			              CHSPEC_IS20(pi->radio_chanspec)));
			if (bw_switch || band_switch) {
				/* XXX
				 * Either a bw/band switch is going to happen or it has already
				 * happened. This will reset phyregs, so don't tamper with them.
				 */
				pi->interf->bphy_desense_index = 0;
				pi->interf->crsminpwr_offset_for_bphydesense = 0;
				return;
			}
		} else {
			PHY_ERROR(("%s: Not ready to desense on new channel %d ! Current "
			           "default state saved on channel %d\n", __FUNCTION__,
			           current_channel, previous_channel));
			ASSERT(0);
		}
	}

	/* suspend mac if haven't done so */
	suspend = !(R_REG(pi->sh->osh, &pi->regs->maccontrol) & MCTL_EN_MAC);
	if (!suspend) {
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
	}

	/* Update index */
	pi->interf->bphy_desense_index = new_index;

	/* Update digigainlimit and peakenergyl */
	phy_utils_mod_phyreg(pi, HTPHY_DigiGainLimit0,
	                     0xffff, bphy_desense_lut[new_index].DigiGainLimit);
	pi->interf->digigainlimit0 = bphy_desense_lut[new_index].DigiGainLimit;
	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		phy_utils_mod_phyreg(pi, (HTPHY_TO_BPHY_OFF+BPHY_PEAK_ENERGY_LO), 0xffff,
		            bphy_desense_lut[new_index].PeakEnergyL);
		pi->interf->peakenergyl = bphy_desense_lut[new_index].PeakEnergyL;
	}

	/* XXX
	 * Update initgain, crsmin if target biq1 index has changed from before or
	 * if bphy desense lut was changed
	 */
	if (lut_changed || (bphy_desense_lut[new_index].drop_initBiq1 !=
	                    bphy_desense_lut[old_index].drop_initBiq1)) {
		uint8 biq1idx[PHY_CORE_MAX];
		uint16 initgain[PHY_CORE_MAX];
		uint8 drop_biq1 = bphy_desense_lut[new_index].drop_initBiq1;

		/* Drop InitGain Biq1 index by the required amount */
		FOREACH_CORE(pi, core) {
			initgain[core] = pi->interf->bphy_desense_base_initgain[core];
			biq1idx[core] = (initgain[core] & 0xf000) >> 12;
			biq1idx[core] -= drop_biq1;
			initgain[core] = (biq1idx[core] << 12) | (initgain[core] & 0xfff);
			pi->interf->init_gain_rfseq[core] = initgain[core];
		}
		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_RFSEQ, 3, 0x106, 16, initgain);
		MOD_PHYREG(pi, HTPHY_Core0InitGainCodeB2059, initBiQ1Index, biq1idx[0]);
		if (PHYCORENUM(pi->pubpi.phy_corenum) > 1)
			MOD_PHYREG(pi, HTPHY_Core1InitGainCodeB2059, InitBiQ1Index, biq1idx[1]);
		if (PHYCORENUM(pi->pubpi.phy_corenum) > 2)
			MOD_PHYREG(pi, HTPHY_Core2InitGainCodeB2059, InitBiQ1Index, biq1idx[2]);

		/* Store sw state for init gain */
		pi->interf->init_gain_core0 =
			phy_utils_read_phyreg(pi, HTPHY_Core0InitGainCodeA2059);
		pi->interf->init_gain_core1 =
			phy_utils_read_phyreg(pi, HTPHY_Core1InitGainCodeA2059);
		pi->interf->init_gain_core2 =
			phy_utils_read_phyreg(pi, HTPHY_Core2InitGainCodeA2059);
		pi->interf->init_gainb_core0 =
			phy_utils_read_phyreg(pi, HTPHY_Core0InitGainCodeB2059);
		pi->interf->init_gainb_core1 =
			phy_utils_read_phyreg(pi, HTPHY_Core1InitGainCodeB2059);
		pi->interf->init_gainb_core2 =
			phy_utils_read_phyreg(pi, HTPHY_Core2InitGainCodeB2059);

		if (lut_changed || (bphy_desense_lut[new_index].drop_initBiq1 <
		                    bphy_desense_lut[old_index].drop_initBiq1)) {
			/* ensure initgain settings take effect */
			wlc_phy_force_rfseq_htphy(pi, HTPHY_RFSEQ_RESET2RX);
		}

		/* Adjust ofdm crsminpower accordingly */
		pi->interf->crsminpwr_offset_for_bphydesense = 8*drop_biq1;
		wlc_phy_noise_update_crsminpwr_htphy(pi);
	}

	/* unsuspend mac */
	if (!suspend) {
		wlapi_enable_mac(pi->sh->physhim);
	}
}

static void
wlc_phy_noise_update_crsminpwr_htphy(phy_info_t *pi)
{
	int16 newcrsminpwr;
	bool suspend;

	/* suspend mac if haven't done so */
	suspend = !(R_REG(pi->sh->osh, &pi->regs->maccontrol) & MCTL_EN_MAC);
	if (!suspend) {
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
	}

	/* Allow for some slack due to initgain backoff in ACI/btcoex desense/bphy desense */
	newcrsminpwr = MAX(pi->interf->crsminpwr_index -
	                   pi->interf->crsminpwr_offset_for_bphydesense -
	                   pi->interf->crsminpwr_offset_for_aci_bt, 0);
	MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, newcrsminpwr);
	MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, newcrsminpwr);
	pi->interf->crsminpwrl0 = newcrsminpwr;
	pi->interf->crsminpwru0 = newcrsminpwr;

	/* unsuspend mac */
	if (!suspend) {
		wlapi_enable_mac(pi->sh->physhim);
	}
}

/* noise mitigation hardware modifications */
static void
wlc_phy_noise_adj_thresholds_htphy(phy_info_t *pi, bool raise)
{
	/* Formula: round(2^3*log2((10.^(([-98:0.25:-69]-3+68-30)/10)*50)*(2^9/0.4)^2)) */
	/* The above formula is previous method formula using table. Now increment */
	/* crsminpower directly with 0.375dB resolution */

	uint16 max_stored_crsminpwr;

	if ((pi->sh->interference_mode == WLAN_AUTO_W_NOISE)|| (pi->sh->interference_mode
		== NON_WLAN)) {
		/* to reduce crs glitches, we will try to use crs min
		 * pwr threshold for ofdm in 0.375 dbm steps, up to a max
		 * value (-69 dbm?)
		 */
		if (raise == 1) {
			/* increase the index, make sure it's in bounds */
			if (PUB_NOT_ASSOC(pi)) {
				/* raise by 4.125 dB (0.375 dB resolution) */
				pi->interf->crsminpwr_index +=
				pi->interf->noise.nphy_noise_noassoc_crsidx_incr;
			} else {
				/* raise by 2.25 dB (0.375 dB resolution) */
				pi->interf->crsminpwr_index +=
				pi->interf->noise.nphy_noise_assoc_crsidx_incr;
			}

			/* changes crsminpwr only, limit the
			 * change based on rssi value
			 *
			 * concern:  bphy doesn't get limited the same way
			 * XXX PR 84999
			 */
			if (!(PUB_NOT_ASSOC(pi))) {
				wlc_phy_noise_limit_crsmin_htphy(pi);
			}
		} else if (raise == 0) {
			/* lower by 0.375dB per step
			 */
			pi->interf->crsminpwr_index -= pi->interf->noise.nphy_noise_crsidx_decr;
		}

		/* set register
		 * check to make sure that the maximum value is not exceeded
		 * or the smallest value isn't smaller than
		 * the agreed upon value from workarounds
		 */
		if (pi->interf->crsminpwr_index > HTPHY_NOISE_MAX_CRSIDX) {
			pi->interf->crsminpwr_index = HTPHY_NOISE_MAX_CRSIDX;
		}
		max_stored_crsminpwr = MAX(pi->interf->crsminpwrthld_20U_stored,
			pi->interf->crsminpwrthld_20L_stored);
		if (pi->interf->crsminpwr_index < max_stored_crsminpwr) {
			pi->interf->crsminpwr_index = max_stored_crsminpwr;
		}
	} else {
		/* currently no changes for other revs */
	}
}

static void
wlc_phy_bphynoise_limit_crsmin_htphy(phy_info_t *pi, int16 *desense_index)
{
	/* XXX
	 * max_desense_rssi corresponds to max allowed bphy desense;
	 * min_desense_rssi corresponds to no bphy desense.
	 */
	int16 min_desense_rssi = pi->interf->bphy_min_sensitivity;
	int16 max_desense_rssi = PHY_NOISE_DESENSE_RSSI_MAX;
	int16 desense_margin = PHY_NOISE_DESENSE_RSSI_MARGIN;
	int16 desense_cap;

	if (pi->interf->rssi == WLC_RSSI_INVALID) {
		/* XXX
		 * Do not desense if no valid rssi info exists yet.
		 *
		 * Unfortunately, this check is overloaded: if true rssi is numerically equal
		 * to WLC_RSSI_INVALID, then no desense will be applied, even though desense might
		 * be required. However, this is unlikely to be a persistent problem in real/
		 * over-the-air environments where the rssi will constantly fluctuate and not
		 * remain stuck at one value.
		 */
		*desense_index = 0;
		PHY_ACI(("%s: RSSI invalid! desense_index = %d \n", __FUNCTION__, *desense_index));
		return;
	}

	/* XXX
	 * Cap the max amount of desense assuming 1dB steps;
	 * Normalize so that min_desense_rssi corresponds to desense_cap = 0
	 */
	desense_cap = MIN(pi->interf->rssi, max_desense_rssi) - min_desense_rssi;
	desense_cap -= desense_margin;
	desense_cap = MAX(0, desense_cap);
	*desense_index = MIN(*desense_index, desense_cap);

	PHY_ACI(("%s rssi = %d, desense_index = %d \n", __FUNCTION__, pi->interf->rssi,
	         *desense_index));
}

static void
wlc_phy_noise_limit_crsmin_htphy(phy_info_t *pi)
{
	/* formula 2059: round(2^3*log2((10.^(([-92-6:0.25:-69]-3+68-30)/10)*50)*(2^9/0.4)^2)) */
	/* if this formula changes, then the constants here need to change too */
	/* for future chips */

	if (pi->interf->rssi < -88) {
		if (pi->interf->crsminpwr_index > 71)
			pi->interf->crsminpwr_index = 71;
	} else if (pi->interf->rssi < -85) {
		if (pi->interf->crsminpwr_index > 78)
			pi->interf->crsminpwr_index = 78;
	} else if (pi->interf->rssi < -80) {
		if (pi->interf->crsminpwr_index > 92)
			pi->interf->crsminpwr_index = 92;
	} else if (pi->interf->rssi < -75) {
		if (pi->interf->crsminpwr_index > 105)
			pi->interf->crsminpwr_index = 105;
	} else if (pi->interf->rssi < -70) {
		if (pi->interf->crsminpwr_index > 119)
			pi->interf->crsminpwr_index = 119;
	} else if (pi->interf->rssi < -65) {
		if (pi->interf->crsminpwr_index > 132)
			pi->interf->crsminpwr_index = 132;
	} else {
	}
}

/* noise mitigation software modification */
static void
wlc_phy_noise_sw_set_htphy(phy_info_t *pi)
{
	if (pi->sh->interference_mode == WLAN_AUTO_W_NOISE ||
		pi->sh->interference_mode == NON_WLAN) {
		pi->interf->noise.noise_glitch_high_detect_total = 0;
		pi->interf->noise.noise_glitch_low_detect_total = 0;
	}
}

/* initialize aci parameters */
void
wlc_phy_aci_init_htphy(phy_info_t *pi)
{

	/* Can be changed via ioctl/iovar */
	pi->interf->aci.nphy.detect_repeat_ctr = 2;
	pi->interf->aci.nphy.detect_num_samples = 50;
	pi->interf->aci.enter_thresh = 100;
	pi->interf->aci.nphy.adcpwr_enter_thresh = 500;
	pi->interf->aci.nphy.adcpwr_exit_thresh = 500;
	pi->interf->aci.nphy.undetect_window_sz = 5;

	/* Phyreg 0xc33(bphy eneregy thresh values for ACI pwr levels(lo, md, hi) */
	pi->interf->aci.nphy.b_energy_lo_aci = 0x40;
	pi->interf->aci.nphy.b_energy_md_aci = 0x80;
	pi->interf->aci.nphy.b_energy_hi_aci = 0xc0;
}

/* aci mode reset hardware and software states */
void
wlc_phy_acimode_reset_htphy(phy_info_t *pi)
{
	bool suspend;

	/* suspend mac if haven't done so */
	suspend = !(R_REG(pi->sh->osh, &pi->regs->maccontrol) & MCTL_EN_MAC);
	if (!suspend) {
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
	}

	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		wlc_phy_aci_init_htphy(pi);
		wlc_phy_acimode_set_htphy(pi, FALSE, PHY_ACI_PWR_NOTPRESENT);
		wlc_phy_aci_sw_reset_htphy(pi);
		wlc_phy_aci_noise_shared_reset_htphy(pi);
	}

	/* unsuspend mac */
	if (!suspend) {
		wlapi_enable_mac(pi->sh->physhim);
	}
}

/* aci software state reset */
void
wlc_phy_aci_sw_reset_htphy(phy_info_t *pi)
{
	int i;
	pi->interf->aci.detect_index = 0;
	pi->interf->aci.detect_total = 0;
	pi->interf->aci.nphy.detection_in_progress = FALSE;
	for (i = 0; i < HTPHY_ACI_MAX_UNDETECT_WINDOW_SZ; i++) {
		pi->interf->aci.detect_list[i] = 0;
		pi->interf->aci.detect_acipwr_lt_list[i] = -100;
	}
	pi->interf->aci.detect_acipwr_max = PHY_ACI_PWR_NOTPRESENT;

	pi->aci_state &= ~ACI_ACTIVE;
	wlapi_high_update_phy_mode(pi->sh->physhim, 0);
}

void
wlc_phy_acimode_upd_htphy(phy_info_t *pi)
{
	int aci_pwr;
	uint8 aci_detect = 0;
	uint i;

	/* !! suspend MAC first */
	wlapi_suspend_mac_and_wait(pi->sh->physhim);

	aci_pwr = wlc_phy_aci_scan_htphy(pi);
	if (aci_pwr != PHY_ACI_PWR_NOTPRESENT) {
		aci_detect = 1;
	}

	/* evict old value */
	pi->interf->aci.detect_total -= pi->interf->aci.detect_list[pi->interf->aci.detect_index];

	/* admit new value */
	pi->interf->aci.detect_total += aci_detect;

	pi->interf->aci.detect_list[pi->interf->aci.detect_index] = aci_detect;
	pi->interf->aci.detect_acipwr_lt_list[pi->interf->aci.detect_index] = aci_pwr;
	pi->interf->aci.detect_index++;

	if (pi->interf->aci.detect_index >= pi->interf->aci.nphy.undetect_window_sz)
		pi->interf->aci.detect_index = 0;

	for (i = 0; i < pi->interf->aci.nphy.undetect_window_sz; i++) {
		if ((pi->interf->aci.detect_list[i] == 1) &&
		    (pi->interf->aci.detect_acipwr_lt_list[i] > aci_pwr))
			aci_pwr = pi->interf->aci.detect_acipwr_lt_list[i];
	}

	if (pi->aci_state & ACI_ACTIVE) {
		if (pi->interf->aci.detect_total == 0) {
			/* disable ACI mitigation */
			wlc_phy_acimode_set_htphy(pi, FALSE, PHY_ACI_PWR_NOTPRESENT);
		} else if (aci_pwr != pi->interf->aci.detect_acipwr_max) {
			wlc_phy_aci_pwr_upd_htphy(pi, aci_pwr);
		} else {
			/* do nothing */
		}
	} else {
		if ((pi->interf->aci.detect_total >= 1) && (CHSPEC_IS2G(pi->radio_chanspec))) {
			/* enable ACI mitigation */
			wlc_phy_acimode_set_htphy(pi, TRUE, aci_pwr);
		}
	}

	wlapi_enable_mac(pi->sh->physhim);

	pi->interf->aci.detect_acipwr_max = aci_pwr;
	PHY_ACI(("wlc_phy_acimode_upd_htphy: aci_state = %d, detect_total = %d\n",
		pi->aci_state, pi->interf->aci.detect_total));
}

int
wlc_phy_aci_scan_htphy(phy_info_t *pi)
{
	int aci_pwr;

	pi->interf->aci.nphy.detection_in_progress = TRUE;
	aci_pwr = wlc_phy_aci_scan_iqbased_htphy(pi);
	pi->interf->aci.nphy.detection_in_progress = FALSE;

	return aci_pwr;
}

/* Return whether or not ACI is present
 * WARNING: this fcn change/restore channel inside, tested on 2G only
 */
static int
wlc_phy_aci_scan_iqbased_htphy(phy_info_t *pi)
{
	phy_info_htphy_t *pi_htphy = pi->u.pi_htphy;
	uint16 repeat_ctr = pi->interf->aci.nphy.detect_repeat_ctr;
	uint16 nsamps = pi->interf->aci.nphy.detect_num_samples;
	uint16 count_thresh = nsamps/5;
	int num_adc_ranges = 3;	/* ADC power ranges & Valid W2s in that power range */
	uint32 adc_pwrs[] = {pi->interf->aci.nphy.adcpwr_enter_thresh, 7000, 12000, 18000};
	int8 valid_w2s[] = {32, 22, -32};
	int16 adc_code;
	uint16 adc_code_thresh;
	uint32 avg_adcpwr, adcpwrL, adcpwrR;
	uint32 pwr_ch, pwr = 0;
	bool adcpwr_val;
	int8 w2;
	int avgw2, w2_ch;
	uint16 i, ctr, core = 0, samp, count;
	int chan, start, end;
	chanspec_t orig_chanspec;
	uint8 orig_channel;
	uint16 classifier_state;	/* register to be saved/restored */
	uint16 clip_state[3];
	uint16 clip_off[3] = {0xffff, 0xffff, 0xffff};
	uint16 gpiosel, gpio;
	uint16 gpioLoOutEn, gpioHiOutEn;
	int chan_delta, chan_skip;
	uint8 lna1 = 0, lna2 = 0, mix_tia_gain = 0, lpf_biq0 = 0;
	uint8 lpf_biq1 = 0;
	uint8 tx_pwr_ctrl_state;
	uint8 saved_pwr_idx[3];
	uint16 saved_crsminpwrl0, saved_crsminpwru0;
	uint16 saved_initgaincode_core0 = 0, saved_initgaincode_core1 = 0,
		saved_initgaincode_core2 = 0;
	uint16 saved_initgaincodeb_core0 = 0, saved_initgaincodeb_core1 = 0,
		saved_initgaincodeb_core2 = 0;
	uint16 saved_initgaincode_rfseq[4];

	ASSERT(nsamps > 0);
	/* save the original chanspec */
	orig_chanspec = pi->radio_chanspec;
	orig_channel = CHSPEC_CHANNEL(orig_chanspec);

	adc_code_thresh = (uint16) phy_utils_sqrt_int(adc_pwrs[0]/2);

	/* Save registers that are going to be changed (start) */
	classifier_state = wlc_phy_classifier_htphy(pi, 0, 0);
	wlc_phy_clip_det_htphy(pi, 0, clip_state);

	/* this for restoring the tx pwr index when we return to orig channel */
	saved_pwr_idx[0] = wlc_phy_txpwrctrl_get_cur_index_htphy(pi, PHY_CORE_0);
	saved_pwr_idx[1] = wlc_phy_txpwrctrl_get_cur_index_htphy(pi, PHY_CORE_1);
	saved_pwr_idx[2] = wlc_phy_txpwrctrl_get_cur_index_htphy(pi, PHY_CORE_2);

	/* save crs min pwr and init gain values */
	saved_crsminpwrl0 = (uint16) (phy_utils_read_phyreg(pi, HTPHY_crsminpowerl0) & 0xff);
	saved_crsminpwru0 = (uint16) (phy_utils_read_phyreg(pi, HTPHY_crsminpoweru0) & 0xff);
	saved_initgaincode_core0 =  phy_utils_read_phyreg(pi, HTPHY_Core0InitGainCodeA2059);
	saved_initgaincode_core1 =  phy_utils_read_phyreg(pi, HTPHY_Core1InitGainCodeA2059);
	saved_initgaincode_core2 =  phy_utils_read_phyreg(pi, HTPHY_Core2InitGainCodeA2059);
	saved_initgaincodeb_core0 =  phy_utils_read_phyreg(pi, HTPHY_Core0InitGainCodeB2059);
	saved_initgaincodeb_core1 =  phy_utils_read_phyreg(pi, HTPHY_Core1InitGainCodeB2059);
	saved_initgaincodeb_core2 =  phy_utils_read_phyreg(pi, HTPHY_Core2InitGainCodeB2059);
	wlc_phy_table_read_htphy(pi, HTPHY_TBL_ID_RFSEQ,
		pi->pubpi.phy_corenum, 0x106, 16, saved_initgaincode_rfseq);

	gpiosel = phy_utils_read_phyreg(pi, HTPHY_gpioSel);
	gpioLoOutEn = phy_utils_read_phyreg(pi, HTPHY_gpioLoOutEn);
	gpioHiOutEn = phy_utils_read_phyreg(pi, HTPHY_gpioHiOutEn);

	/* ************** (end) ************ */

	/* Based on phy bw, set the aci scan delta & skip */
	if (IS40MHZ(pi)) {
		chan_delta = HTPHY_ACI_40MHZ_CHANNEL_DELTA;
		chan_skip = HTPHY_ACI_40MHZ_CHANNEL_SKIP;
	} else {
		chan_delta = HTPHY_ACI_CHANNEL_DELTA;
		chan_skip = HTPHY_ACI_CHANNEL_SKIP;
	}

	/* Channels scan range */
	start = MAX(ACI_FIRST_CHAN, orig_channel - chan_delta);
	end = MIN(ACI_LAST_CHAN, orig_channel + chan_delta);

	/* Algorithm:
	*  1. Scan channels(for power) that are least 4 apart
	*  2. Average adc_pwr & W2 for adc_codes greater than some threshold
	*  3. High power signal can leak in 4 channel apart, so qualify that
	*     with W2. For low adc_pwr W2 will be low, if its coming from adjacent channel
	*/
	for (chan = start; chan <= end; chan++) {
		if ((chan < (orig_channel - chan_skip)) ||
			(chan > (orig_channel + chan_skip))) {

			/* because this function can potentially reinit the phy */
			/* save off the current crs min power and init gain values */
			/* and restore after this function */

			wlc_phy_chanspec_set((wlc_phy_t*)pi, CH20MHZ_CHSPEC(chan));

			/* ** Change phy registers needed for scanning (start) ** */
			/* Classifier off,clip det off, set appropriate gain */
			wlc_phy_classifier_htphy(pi,
				HTPHY_ClassifierCtrl_classifierSel_MASK, 4);
			wlc_phy_clip_det_htphy(pi, 1, clip_off);

			/* use overall gain 15 db less than init gain here */
			/* 51dB gain */
			lna1 = 3; lna2 = 3;  mix_tia_gain = 4;
			lpf_biq0 = 2; lpf_biq1 = 0;
			/* lna1=25dB, lna2=15, mix=5, lpf_biq0=6, lpf_bq1=0
			*/
			FOREACH_CORE(pi, core) {
				phy_utils_write_phyreg(pi, HTPHY_RfctrlRXGAIN(core),
					(mix_tia_gain << 4) | (lna2 << 2) | lna1);
				phy_utils_write_phyreg(pi, HTPHY_Rfctrl_lpf_gain(core),
					(lpf_biq1 << 4) | lpf_biq0);
				MOD_PHYREGC(pi, HTPHY_RfctrlOverride, core, rxgain, 1);
				MOD_PHYREGC(pi, HTPHY_RfctrlOverride, core, lpf_gain_biq0, 1);
				MOD_PHYREGC(pi, HTPHY_RfctrlOverride, core, lpf_gain_biq1, 1);
			}

			/* Enable the gpio's */
			phy_utils_write_phyreg(pi, HTPHY_gpioLoOutEn, 0xffff);
			phy_utils_write_phyreg(pi, HTPHY_gpioHiOutEn, 0xffff);

			/* Select W2 Rssi */
			wlc_phy_auxadc_sel_htphy(pi, pi->sh->phyrxchain, HTPHY_AUXADC_SEL_W2);

			/* ** end (changing registers) ** */

			pwr_ch = 0;
			w2_ch = 32;
			/* Alternate i-rail, q-rail and use only one rail for each */
			/* core to save time */
			FOREACH_CORE(pi, core) {
				phy_utils_write_phyreg(pi, HTPHY_gpioSel, 8+core);
				for (ctr = 0; ctr < repeat_ctr; ctr++) {
					avg_adcpwr = 0;
					avgw2 = 0;
					count = 0;
					for (samp = 0; samp < nsamps; samp++) {
						if (samp % 2 == 0)
							gpio = phy_utils_read_phyreg(pi,
								HTPHY_gpioLoOut);
						else
							gpio = phy_utils_read_phyreg(pi,
								HTPHY_gpioHiOut);

						/* MSB 8bits of ADC are enough */
						adc_code = ((int16)((gpio & 0x3ff) << 6)) >> 8;
						w2 = ((int8)(((gpio >> 10) & 0x3f) << 2)) >> 2;
						if (ABS(adc_code) > adc_code_thresh) {
							count++;
							avg_adcpwr += (adc_code * adc_code);
							avgw2 += w2;
						}
					}
					if (count > count_thresh) {
						avg_adcpwr = avg_adcpwr / count;
						if (avg_adcpwr > pwr_ch) {
							avgw2 = avgw2 / count;
							adcpwr_val = TRUE;  /* preset */
							for (i = 0; i < num_adc_ranges; i++) {
								adcpwrL = adc_pwrs[i];
								adcpwrR = adc_pwrs[i+1];
								if ((avg_adcpwr >= adcpwrL) &&
									(avg_adcpwr < adcpwrR)) {
									if (avgw2 < valid_w2s[i])
										adcpwr_val = FALSE;
									break;
								}
							}
							PHY_ACI(("wlc_phy_aci_scan_iqbased_htphy:"
								" chan=%d avg_adc=%d avgw2=%d"
								" cnt=%d core=%d ctr=%d\n",
								chan, avg_adcpwr,
								avgw2, count, core, ctr));
							if (adcpwr_val) {
								pwr_ch = avg_adcpwr;
								w2_ch = avgw2;
							}
						}
					}

					OSL_DELAY(10);
				}
				pwr = MAX(pwr, pwr_ch);
			}
			PHY_ACI(("%s:  pwr=%d, chan=%d, pwr_ch=%d, w2_ch=%d\n",
				__FUNCTION__, pwr, chan, pwr_ch, w2_ch));
			BCM_REFERENCE(w2_ch);
		}
	}

	/* ************* RESTORE REGISTERS ************* */
	phy_utils_write_phyreg(pi, HTPHY_gpioSel, gpiosel);
	phy_utils_write_phyreg(pi, HTPHY_gpioLoOutEn, gpioLoOutEn);
	phy_utils_write_phyreg(pi, HTPHY_gpioHiOutEn, gpioHiOutEn);
	wlc_phy_auxadc_sel_htphy(pi, pi->sh->phyrxchain, HTPHY_AUXADC_SEL_OFF);

	/* turn off rxgain, lpf_gain_biq0, lpf_gain_biq1 overrides */
	FOREACH_CORE(pi, core) {
		MOD_PHYREGC(pi, HTPHY_RfctrlOverride, core, rxgain, 0);
		MOD_PHYREGC(pi, HTPHY_RfctrlOverride, core, lpf_gain_biq0, 0);
		MOD_PHYREGC(pi, HTPHY_RfctrlOverride, core, lpf_gain_biq1, 0);
	}

	/* revert to the original chanspec */
	wlc_phy_chanspec_set((wlc_phy_t*)pi, orig_chanspec);

	wlc_phy_clip_det_htphy(pi, 1, clip_state);
	wlc_phy_classifier_htphy(pi, HTPHY_ClassifierCtrl_classifierSel_MASK,
		classifier_state);

	/* restore tx pwr index to original power index */
	tx_pwr_ctrl_state = pi->txpwrctrl;
	wlc_phy_txpwrctrl_enable_htphy(pi, PHY_TPC_HW_OFF);
	FOREACH_CORE(pi, core) {
		pi_htphy->txpwrindex_hw_save[core] = saved_pwr_idx[core];
	}
	wlc_phy_txpwrctrl_enable_htphy(pi, tx_pwr_ctrl_state);

	/* restore the crsminpwr and init gains */
	phy_utils_mod_phyreg(pi, HTPHY_crsminpowerl0, HTPHY_crsminpowerl0_crsminpower0_MASK,
		saved_crsminpwrl0);
	phy_utils_mod_phyreg(pi, HTPHY_crsminpoweru0, HTPHY_crsminpoweru0_crsminpower0_MASK,
		saved_crsminpwru0);

	phy_utils_write_phyreg(pi, HTPHY_Core0InitGainCodeA2059,
		saved_initgaincode_core0);
	phy_utils_write_phyreg(pi, HTPHY_Core1InitGainCodeA2059,
		saved_initgaincode_core1);
	phy_utils_write_phyreg(pi, HTPHY_Core2InitGainCodeA2059,
		saved_initgaincode_core2);
	phy_utils_write_phyreg(pi, HTPHY_Core0InitGainCodeB2059,
		saved_initgaincodeb_core0);
	phy_utils_write_phyreg(pi, HTPHY_Core1InitGainCodeB2059,
		saved_initgaincodeb_core1);
	phy_utils_write_phyreg(pi, HTPHY_Core2InitGainCodeB2059,
		saved_initgaincodeb_core2);

	wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_RFSEQ, pi->pubpi.phy_corenum, 0x106,
		16, saved_initgaincode_rfseq);

	/* Based on adc power level decide what is the ACI power level */
	PHY_ACI(("pwr = %d, pwr > adc_pwrs[1] = %d\n",
	pwr, (pwr > adc_pwrs[1])));
	if (pwr > adc_pwrs[1])
		return PHY_ACI_PWR_HIGH;
	return PHY_ACI_PWR_NOTPRESENT;
}

/* Don't use this feature right now, until the values are correct */
static void
wlc_phy_aci_pwr_upd_htphy(phy_info_t *pi, int aci_pwr)
{
	/* XXX
	 * The code in this function is no longer needed, as the new bphy desense feature
	 * replicates the functionality below based on glitch count instead of aci pwr.
	 * However, retaining this function as a placeholder if we ever need to add
	 * multiple levels of aci mitigation (front-end desense) based on aci pwr.
	 *
	 * ORIGINAL CODE:
	 *
	 * uint16 b_energy_thresh = pi->interf->aci.nphy.b_energy_hi_aci;
	 *
	 * if (aci_pwr == PHY_ACI_PWR_LOW) {
	 *	b_energy_thresh = pi->interf->aci.nphy.b_energy_lo_aci;
	 * } else if (aci_pwr == PHY_ACI_PWR_MED) {
	 *	b_energy_thresh = pi->interf->aci.nphy.b_energy_md_aci;
	 * } else if (aci_pwr == PHY_ACI_PWR_HIGH) {
	 *	b_energy_thresh = pi->interf->aci.nphy.b_energy_hi_aci;
	 * }
	 *
	 * phy_utils_write_phyreg(pi, (HTPHY_TO_BPHY_OFF+BPHY_PEAK_ENERGY_LO), b_energy_thresh);
	 */

	BCM_REFERENCE(pi);
	BCM_REFERENCE(aci_pwr);
}

static void
wlc_phy_aci_sw_set_htphy(phy_info_t *pi, bool enable)
{

	if (enable) {
		if (!CHSPEC_IS2G(pi->radio_chanspec)) {
			/* not 2 ghz, so do not do this */
			PHY_ERROR(("wlc_phy_aci_sw_set_htphy: do not enable ACI for 5GHz!\n"));
			return;
		}

		pi->aci_state |= ACI_ACTIVE;
		pi->aci_start_time = pi->sh->now;
		wlapi_high_update_phy_mode(pi->sh->physhim, PHY_MODE_ACI);
	} else {
		if (CHSPEC_CHANNEL(pi->radio_chanspec) == pi->interf->curr_home_channel) {
			/* on home channel, i wanted to disable ACI, so do so */
			pi->aci_state &= ~ACI_ACTIVE;
			wlapi_high_update_phy_mode(pi->sh->physhim, 0);
		}
	}

}

static void
wlc_phy_aci_noise_shared_reset_htphy(phy_info_t *pi)
{
	uint8 core;

		/* software state reset */
		pi->interf->init_gain_core0 =
			pi->interf->init_gain_code_core0_stored;
		pi->interf->init_gain_core1 =
			pi->interf->init_gain_code_core1_stored;
		pi->interf->init_gain_core2 =
			pi->interf->init_gain_code_core2_stored;
		pi->interf->init_gainb_core0 =
			pi->interf->init_gain_codeb_core0_stored;
		pi->interf->init_gainb_core1 =
			pi->interf->init_gain_codeb_core1_stored;
		pi->interf->init_gainb_core2 =
			pi->interf->init_gain_codeb_core2_stored;
		FOREACH_CORE(pi, core) {
			pi->interf->init_gain_rfseq[core] =
				pi->interf->init_gain_table_stored[core];
		}

		pi->interf->crsminpwrl0 = pi->interf->crsminpwrthld_20L_stored;
		pi->interf->crsminpwru0 = pi->interf->crsminpwrthld_20U_stored;

		pi->interf->digigainlimit0 = pi->interf->digigainlimit0_stored;
		pi->interf->peakenergyl = pi->interf->peakenergyl_stored;

		/* set base crsminpwr for noise mitigation */
		pi->interf->crsminpwr_index = MAX(pi->interf->crsminpwrthld_20L_stored,
		                                 pi->interf->crsminpwrthld_20U_stored);

		/* set base bphy desense index for noise mitigation */
		pi->interf->bphy_desense_index = 0;
		pi->interf->bphy_desense_index_scan_restore = pi->interf->bphy_desense_index;
		/* set appropriate LUT for bphy desense */
		if (IS_X29B_BOARDTYPE(pi) && pi->u.pi_htphy->btc_restage_rxgain) {
			pi->interf->bphy_desense_lut = HTPHY_bphy_desense_lut_X29B_BTON;
			pi->interf->bphy_desense_lut_size =
				sizeof(HTPHY_bphy_desense_lut_X29B_BTON)/
				sizeof(bphy_desense_info_t);
			pi->interf->bphy_min_sensitivity = HTPHY_BPHY_MIN_SENSITIVITY_X29B_BTON;
		} else if (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA) {
			/* Use LUT for elna */
			pi->interf->bphy_desense_lut = HTPHY_bphy_desense_lut_eLNA;
			pi->interf->bphy_desense_lut_size = sizeof(HTPHY_bphy_desense_lut_eLNA)/
			        sizeof(bphy_desense_info_t);
			pi->interf->bphy_min_sensitivity = HTPHY_BPHY_MIN_SENSITIVITY_ELNA;
		} else {
			/* Use LUT for ilna, aci off */
			pi->interf->bphy_desense_lut = HTPHY_bphy_desense_lut_iLNA_acioff;
			pi->interf->bphy_min_sensitivity = HTPHY_BPHY_MIN_SENSITIVITY_ILNA_ACIOFF;
			pi->interf->bphy_desense_lut_size =
			        sizeof(HTPHY_bphy_desense_lut_iLNA_acioff)/
			        sizeof(bphy_desense_info_t);
		}

		/* define appropriate base initgain table for bphy desense */
		if (IS_X29B_BOARDTYPE(pi) && pi->u.pi_htphy->btc_restage_rxgain) {
			pi->interf->bphy_desense_base_initgain[0] = 0x9117;
			pi->interf->bphy_desense_base_initgain[1] = pi->interf->
			        init_gain_table_stored[1];
			pi->interf->bphy_desense_base_initgain[2] = 0x9117;
		} else {
			FOREACH_CORE(pi, core) {
				pi->interf->bphy_desense_base_initgain[core] = pi->interf->
				        init_gain_table_stored[core];
			}
		}

		pi->interf->crsminpwr_offset_for_bphydesense = 0;
		pi->interf->crsminpwr_offset_for_aci_bt = 0;

		/* hardware state reset */
		phy_utils_write_phyreg(pi, HTPHY_Core0InitGainCodeA2059,
			pi->interf->init_gain_code_core0_stored);
		phy_utils_write_phyreg(pi, HTPHY_Core1InitGainCodeA2059,
			pi->interf->init_gain_code_core1_stored);
		phy_utils_write_phyreg(pi, HTPHY_Core2InitGainCodeA2059,
			pi->interf->init_gain_code_core2_stored);
		phy_utils_write_phyreg(pi, HTPHY_Core0InitGainCodeB2059,
			pi->interf->init_gain_codeb_core0_stored);
		phy_utils_write_phyreg(pi, HTPHY_Core1InitGainCodeB2059,
			pi->interf->init_gain_codeb_core1_stored);
		phy_utils_write_phyreg(pi, HTPHY_Core2InitGainCodeB2059,
			pi->interf->init_gain_codeb_core2_stored);

		wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_RFSEQ, pi->pubpi.phy_corenum, 0x106,
			16, pi->interf->init_gain_table_stored);

		/* reset crsminpwr threshold to original value */
		phy_utils_mod_phyreg(pi, HTPHY_crsminpowerl0, HTPHY_crsminpowerl0_crsminpower0_MASK,
			pi->interf->crsminpwrthld_20L_stored);
		phy_utils_mod_phyreg(pi, HTPHY_crsminpoweru0, HTPHY_crsminpoweru0_crsminpower0_MASK,
			pi->interf->crsminpwrthld_20U_stored);

		/* reset digigainlimit and peakenergyl to default values */
		if (CHSPEC_IS2G(pi->radio_chanspec))
			phy_utils_write_phyreg(pi, (HTPHY_TO_BPHY_OFF+BPHY_PEAK_ENERGY_LO),
			              pi->interf->peakenergyl_stored);
		phy_utils_write_phyreg(pi, HTPHY_DigiGainLimit0, pi->interf->digigainlimit0_stored);
}

/* XXX
 * PR114196 fix: Bphy ACI/CCI causes OFDM crs glitches at low powers due to MF
 */
void
wlc_phy_noise_raise_MFthresh_htphy(phy_info_t *pi, bool raise)
{
	uint8 high2lowpowThresh, low2highpowThresh;
	bool suspend;

	/* suspend mac if haven't done so */
	suspend = !(R_REG(pi->sh->osh, &pi->regs->maccontrol) & MCTL_EN_MAC);
	if (!suspend) {
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
	}

	/* Modify gain thresholds at which MF detection threshold becomes more/less stringent */
	if (raise && CHSPEC_IS2G(pi->radio_chanspec)) {
		/* Choose gain thresholds so that MF detection threshold is always high */
		high2lowpowThresh = 0x62;
		low2highpowThresh = 0x5f;
	} else {
		/* Set back to default values */
		high2lowpowThresh = 0x51;
		low2highpowThresh = 0x4e;
	}
	/* Apply chosen thresholds */
	MOD_PHYREG(pi, HTPHY_crshighlowpowThreshold, high2lowpowThresh, high2lowpowThresh);
	MOD_PHYREG(pi, HTPHY_crshighlowpowThreshold, low2highpowThresh, low2highpowThresh);
	MOD_PHYREG(pi, HTPHY_crshighlowpowThresholdl, high2lowpowThresh, high2lowpowThresh);
	MOD_PHYREG(pi, HTPHY_crshighlowpowThresholdl, low2highpowThresh, low2highpowThresh);
	MOD_PHYREG(pi, HTPHY_crshighlowpowThresholdu, high2lowpowThresh, high2lowpowThresh);
	MOD_PHYREG(pi, HTPHY_crshighlowpowThresholdu, low2highpowThresh, low2highpowThresh);

	/* unsuspend mac */
	if (!suspend) {
		wlapi_enable_mac(pi->sh->physhim);
	}
}

static void
wlc_phy_aci_noise_shared_hw_set_htphy(phy_info_t *pi, bool aci_miti_enable,
	bool from_aci_call)
{

	uint16 aci_present_init_gaincode;
	uint16 aci_present_init_gaincodeb = 0;
	uint16 aci_present_rfseq_init_gain[] = {0x9136, 0x9136, 0x9136, 0x9136};
	uint16 aci_present_rfseq_init_gain_elna[] = {0x5136, 0x5136, 0x5136, 0x5136};
	bool band_switch;
	bool bw_switch;

	if (from_aci_call) {
		if (aci_miti_enable) {
			/* reduce init gain and also redistribute gains
			 * with less lna gain
			 */

			/* XXX
			 * use these values:
			 * Core1InitGainCodeA2059 =
			 * 0000 0000 0110 1100 = 0x6c
			 * Core1InitGainCodeB2059 =
			 * 0000 1001 0001 0000 = 0x910
			 * lna1=0x2, lna2=1, mixer=0x3,
			 * lpf-b0=0x1, lpf-b1=0x9
			 * lna1=19dB, lna2=10dB, mixer=5dB,
			 * lpf-b0=3dB, lpf-b1=27dB
			 */
			if ((BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA) &&
				(CHSPEC_IS2G(pi->radio_chanspec))) {
				/* Drop INIT_GAIN in HPVGA gain by 12 dB for ext-LNA
				 * to improve false detection
				 */
				aci_present_init_gaincode = 0x6c;
				aci_present_init_gaincodeb = 0x510 |
					(phy_utils_read_phyreg(pi,
					HTPHY_Core1InitGainCodeB2059) & 0xf);
			} else {
				aci_present_init_gaincode = 0x6c;
				aci_present_init_gaincodeb = 0x910 |
					(phy_utils_read_phyreg(pi,
					HTPHY_Core1InitGainCodeB2059) & 0xf);
			}

			phy_utils_write_phyreg(pi, HTPHY_Core0InitGainCodeA2059,
				aci_present_init_gaincode);
			phy_utils_write_phyreg(pi, HTPHY_Core1InitGainCodeA2059,
				aci_present_init_gaincode);
			phy_utils_write_phyreg(pi, HTPHY_Core2InitGainCodeA2059,
				aci_present_init_gaincode);
			phy_utils_write_phyreg(pi, HTPHY_Core0InitGainCodeB2059,
				aci_present_init_gaincodeb);
			phy_utils_write_phyreg(pi, HTPHY_Core1InitGainCodeB2059,
				aci_present_init_gaincodeb);
			phy_utils_write_phyreg(pi, HTPHY_Core2InitGainCodeB2059,
				aci_present_init_gaincodeb);

			if ((BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA) &&
				(CHSPEC_IS2G(pi->radio_chanspec))) {
				/* Drop INIT_GAIN in HPVGA gain by 12 dB for ext-LNA
				 * to improve false detection
				 */
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_RFSEQ,
					pi->pubpi.phy_corenum, 0x106,
					16, aci_present_rfseq_init_gain_elna);
			} else {
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_RFSEQ,
					pi->pubpi.phy_corenum, 0x106,
					16, aci_present_rfseq_init_gain);
				/* apply correction to crsminpower due to initgain drop = 4dB */
				pi->interf->crsminpwr_offset_for_aci_bt = 11;
				wlc_phy_noise_update_crsminpwr_htphy(pi);
				/* update bphy desense to account for initgain/sensitivity delta */
				if (pi->sh->interference_mode == WLAN_AUTO_W_NOISE) {
					wlc_phy_bphynoise_update_lut_htphy(
						pi, HTPHY_bphy_desense_lut_iLNA_acion,
						sizeof(HTPHY_bphy_desense_lut_iLNA_acion)/
						sizeof(bphy_desense_info_t),
						HTPHY_BPHY_MIN_SENSITIVITY_ILNA_ACION,
						aci_present_rfseq_init_gain);
				}
			}
		} else {
			band_switch = (CHSPEC_CHANNEL(pi->interf->radio_chanspec_stored) > 14 &&
				CHSPEC_CHANNEL(pi->radio_chanspec) <= 14) ||
				((CHSPEC_CHANNEL(pi->interf->radio_chanspec_stored) <= 14) &&
				(CHSPEC_CHANNEL(pi->radio_chanspec) > 14));

			bw_switch = ((CHSPEC_IS20(pi->interf->radio_chanspec_stored) == 1) &&
				(CHSPEC_IS20(pi->radio_chanspec) == 0)) ||
				((CHSPEC_IS20(pi->interf->radio_chanspec_stored) == 0) &&
				(CHSPEC_IS20(pi->radio_chanspec) == 1));

			if ((pi->interf->init_gain_code_core0_stored == 0) ||
			    (pi->interf->init_gain_code_core1_stored == 0) ||
			    (pi->interf->init_gain_code_core2_stored == 0) ||
			    (pi->interf->init_gain_codeb_core0_stored == 0) ||
			    (pi->interf->init_gain_codeb_core1_stored == 0) ||
			    (pi->interf->init_gain_codeb_core2_stored == 0)) {
				return;
			}

			if (!(band_switch || bw_switch)) {
				/* set intigain phyregs/tables to default values */
				phy_utils_write_phyreg(pi, HTPHY_Core0InitGainCodeA2059,
				              pi->interf->init_gain_code_core0_stored);
				phy_utils_write_phyreg(pi, HTPHY_Core1InitGainCodeA2059,
				              pi->interf->init_gain_code_core1_stored);
				phy_utils_write_phyreg(pi, HTPHY_Core2InitGainCodeA2059,
				              pi->interf->init_gain_code_core2_stored);
				phy_utils_write_phyreg(pi, HTPHY_Core0InitGainCodeB2059,
				              pi->interf->init_gain_codeb_core0_stored);
				phy_utils_write_phyreg(pi, HTPHY_Core1InitGainCodeB2059,
				              pi->interf->init_gain_codeb_core1_stored);
				phy_utils_write_phyreg(pi, HTPHY_Core2InitGainCodeB2059,
				              pi->interf->init_gain_codeb_core2_stored);
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_RFSEQ,
				                          pi->pubpi.phy_corenum, 0x106,
				                          16, pi->interf->init_gain_table_stored);
			}

			if ((CHSPEC_CHANNEL(pi->radio_chanspec) ==
				pi->interf->curr_home_channel) &&
				(pi->sh->interference_mode == WLAN_AUTO_W_NOISE)) {
				/* XXX
				 * in home channel, adjust crsminpwr to account for
				 * initgain change
				 */
				pi->interf->crsminpwr_offset_for_aci_bt = 0;
				wlc_phy_noise_update_crsminpwr_htphy(pi);

				/* update bphy desense to account for initgain/sensitivity delta */
				wlc_phy_bphynoise_update_lut_htphy(
					pi, HTPHY_bphy_desense_lut_iLNA_acioff,
					sizeof(HTPHY_bphy_desense_lut_iLNA_acioff)/
					sizeof(bphy_desense_info_t),
					HTPHY_BPHY_MIN_SENSITIVITY_ILNA_ACIOFF,
					pi->interf->init_gain_table_stored);
			} else {
				/* not in home channel and/or not wl interference 4,
				 * so restore last stored default values.
				 */
				if (!(band_switch || bw_switch)) {
					phy_utils_mod_phyreg(pi, HTPHY_crsminpowerl0,
					            HTPHY_crsminpowerl0_crsminpower0_MASK,
					            pi->interf->crsminpwrthld_20L_stored);
					phy_utils_mod_phyreg(pi, HTPHY_crsminpoweru0,
					            HTPHY_crsminpoweru0_crsminpower0_MASK,
					            pi->interf->crsminpwrthld_20U_stored);
				}
			}
		}
	} else {
		/* called from noise mitigation */
		wlc_phy_noise_update_crsminpwr_htphy(pi);
	}
}

void
wlc_phy_acimode_set_htphy(phy_info_t *pi, bool aci_miti_enable, int aci_pwr)
{
	bool suspend;

	/* suspend mac if haven't done so */
	suspend = !(R_REG(pi->sh->osh, &pi->regs->maccontrol) & MCTL_EN_MAC);
	if (!suspend) {
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
	}

	if ((CHSPEC_IS2G(pi->radio_chanspec) && aci_miti_enable &&
		!pi->interf->hw_aci_mitig_on) ||
		(!aci_miti_enable && pi->interf->hw_aci_mitig_on)) {
		wlc_phy_aci_hw_set_htphy(pi, aci_miti_enable, aci_pwr);
		wlc_phy_aci_noise_shared_hw_set_htphy(pi, aci_miti_enable, TRUE);
		wlc_phy_aci_sw_set_htphy(pi, aci_miti_enable);
		if (aci_miti_enable) {
			pi->interf->hw_aci_mitig_on = TRUE;
		} else {
			pi->interf->hw_aci_mitig_on = FALSE;
		}
	}

	/* unsuspend mac */
	if (!suspend) {
		wlapi_enable_mac(pi->sh->physhim);
	}
}

static void
wlc_phy_aci_hw_set_htphy(phy_info_t *pi, bool enable, int aci_pwr)
{
	uint16 aci_prsnt_clip1hi_gaincode;
	uint16 aci_prsnt_clip1hi_gaincodeb;
	uint16 aci_prsnt_nbclip_threshold;

	int w1_thresh;
	uint16 regval[PHY_CORE_MAX];
	bool band_switch;
	bool bw_switch;

	if (enable) {
		if (!CHSPEC_IS2G(pi->radio_chanspec)) {
			/* not 2 ghz, so do not do this */
			PHY_ERROR(("wlc_phy_aci_hw_set_htphy: do not enable ACI for 5GHz!\n"));
			return;
		}

		PHY_ACI(("wlc_phy_aci_hw_set_htphy: Enable ACI mitigation, channel is %d\n",
			CHSPEC_CHANNEL(pi->radio_chanspec)));

		/* set energy drop timeout len to small value, return to
		 * search asap after previous detection fails
		 */
		/* XXX. The following is not needed since HTPHY_energydroptimeoutLen
		 * is set to 2 in wlc_phy_workarounds_htphy
		 */
		/*
			pi->interf->energy_drop_timeout_len_stored =
				phy_utils_read_phyreg(pi, HTPHY_energydroptimeoutLen);
			phy_utils_write_phyreg(pi, HTPHY_energydroptimeoutLen, 0x2);
		*/

		/* clip hi gain: since overall init gain reduced, */
		/* need to reduce clip hi gain and */
		/* since lna gains reduced, we need to also reduce */
		/* nbclip thresholds */
		pi->interf->clip1_hi_gain_code_core0_stored =
			phy_utils_read_phyreg(pi, HTPHY_Core0clipHiGainCodeA2059);
		pi->interf->clip1_hi_gain_code_core1_stored =
			phy_utils_read_phyreg(pi, HTPHY_Core1clipHiGainCodeA2059);
		pi->interf->clip1_hi_gain_code_core2_stored =
			phy_utils_read_phyreg(pi, HTPHY_Core2clipHiGainCodeA2059);
		pi->interf->clip1_hi_gain_codeb_core0_stored =
			phy_utils_read_phyreg(pi, HTPHY_Core0clipHiGainCodeB2059);
		pi->interf->clip1_hi_gain_codeb_core1_stored =
			phy_utils_read_phyreg(pi, HTPHY_Core1clipHiGainCodeB2059);
		pi->interf->clip1_hi_gain_codeb_core2_stored =
			phy_utils_read_phyreg(pi, HTPHY_Core2clipHiGainCodeB2059);

		/* use lna1 = 0x2, lna2 = 0x1, mixer = 1, */
		/* lpf-b0 = 2, lpf-b1=2  */
		aci_prsnt_clip1hi_gaincode = 0x2c;
		phy_utils_write_phyreg(pi, HTPHY_Core0clipHiGainCodeA2059,
			aci_prsnt_clip1hi_gaincode);
		phy_utils_write_phyreg(pi, HTPHY_Core1clipHiGainCodeA2059,
			aci_prsnt_clip1hi_gaincode);
		phy_utils_write_phyreg(pi, HTPHY_Core2clipHiGainCodeA2059,
			aci_prsnt_clip1hi_gaincode);
		aci_prsnt_clip1hi_gaincodeb = 0x220 |
			(phy_utils_read_phyreg(pi, HTPHY_Core0clipHiGainCodeB2059) & 0xf);
		phy_utils_write_phyreg(pi, HTPHY_Core0clipHiGainCodeB2059,
			aci_prsnt_clip1hi_gaincodeb);
		aci_prsnt_clip1hi_gaincodeb = 0x220 |
			(phy_utils_read_phyreg(pi, HTPHY_Core1clipHiGainCodeB2059) & 0xf);
		phy_utils_write_phyreg(pi, HTPHY_Core1clipHiGainCodeB2059,
			aci_prsnt_clip1hi_gaincodeb);
		aci_prsnt_clip1hi_gaincodeb = 0x220 |
			(phy_utils_read_phyreg(pi, HTPHY_Core2clipHiGainCodeB2059) & 0xf);
		phy_utils_write_phyreg(pi, HTPHY_Core2clipHiGainCodeB2059,
			aci_prsnt_clip1hi_gaincodeb);

		/* nb threshold changes */
		pi->interf->nb_clip_thresh_core0_stored =
			phy_utils_read_phyreg(pi, HTPHY_Core0nbClipThreshold);
		pi->interf->nb_clip_thresh_core1_stored =
			phy_utils_read_phyreg(pi, HTPHY_Core1nbClipThreshold);
		pi->interf->nb_clip_thresh_core2_stored =
			phy_utils_read_phyreg(pi, HTPHY_Core2nbClipThreshold);

		aci_prsnt_nbclip_threshold = 0x32;
		phy_utils_write_phyreg(pi, HTPHY_Core0nbClipThreshold,
		aci_prsnt_nbclip_threshold);
		phy_utils_write_phyreg(pi, HTPHY_Core1nbClipThreshold,
			aci_prsnt_nbclip_threshold);
		phy_utils_write_phyreg(pi, HTPHY_Core2nbClipThreshold,
			aci_prsnt_nbclip_threshold);

		/* gain limit */
		/* change lna2gainchange value for ofdm and cck */
		wlc_phy_table_read_htphy(pi, 4, 4, 0x10, 16,
		   pi->interf->init_ofdmlna2gainchange_stored);
		wlc_phy_table_read_htphy(pi, 4, 4, 0x50, 16,
		   pi->interf->init_ccklna2gainchange_stored);

		/* change only the 19th and 83rd values */
		regval[0] = 0x7f;
		wlc_phy_table_write_htphy(pi, 4, 1, 0x13, 16, &regval[0]);
		wlc_phy_table_write_htphy(pi, 4, 1, 0x53, 16, &regval[0]);

		/* clipLO gain */
		pi->interf->clip1_lo_gain_code_core0_stored =
			phy_utils_read_phyreg(pi, HTPHY_Core0cliploGainCodeA2059);
		pi->interf->clip1_lo_gain_code_core1_stored =
			phy_utils_read_phyreg(pi, HTPHY_Core1cliploGainCodeA2059);
		pi->interf->clip1_lo_gain_code_core2_stored =
			phy_utils_read_phyreg(pi, HTPHY_Core2cliploGainCodeA2059);
		phy_utils_write_phyreg(pi, HTPHY_Core0cliploGainCodeA2059, 0x24);
		phy_utils_write_phyreg(pi, HTPHY_Core1cliploGainCodeA2059, 0x24);
		phy_utils_write_phyreg(pi, HTPHY_Core2cliploGainCodeA2059, 0x24);
		pi->interf->clip1_lo_gain_codeb_core0_stored =
			phy_utils_read_phyreg(pi, HTPHY_Core0cliploGainCodeB2059);
		pi->interf->clip1_lo_gain_codeb_core1_stored =
			phy_utils_read_phyreg(pi, HTPHY_Core1cliploGainCodeB2059);
		pi->interf->clip1_lo_gain_codeb_core2_stored =
			phy_utils_read_phyreg(pi, HTPHY_Core2cliploGainCodeB2059);
		phy_utils_write_phyreg(pi, HTPHY_Core0cliploGainCodeB2059, 0x310 |
			(phy_utils_read_phyreg(pi, HTPHY_Core0cliploGainCodeB2059) & 0xf));
		phy_utils_write_phyreg(pi, HTPHY_Core1cliploGainCodeB2059, 0x310 |
			(phy_utils_read_phyreg(pi, HTPHY_Core1cliploGainCodeB2059) & 0xf));
		phy_utils_write_phyreg(pi, HTPHY_Core2cliploGainCodeB2059, 0x310 |
			(phy_utils_read_phyreg(pi, HTPHY_Core2cliploGainCodeB2059) & 0xf));

		/* RF rssi gain */
		pi->interf->radio_2057_core0_rssi_nb_gc_stored =
			phy_utils_read_radioreg(pi, RADIO_2059_NB_MASTER(0)) & 0x60;
		pi->interf->radio_2057_core1_rssi_nb_gc_stored =
			phy_utils_read_radioreg(pi, RADIO_2059_NB_MASTER(1)) & 0x60;
		pi->interf->radio_2057_core2_rssi_nb_gc_stored =
			phy_utils_read_radioreg(pi, RADIO_2059_NB_MASTER(2)) & 0x60;
		pi->interf->radio_2057_core0_rssi_wb1a_gc_stored =
			phy_utils_read_radioreg(pi,
			RADIO_2059_RSSI_GPAIOSEL_W1_IDACS(0)) & 0xc;
		pi->interf->radio_2057_core1_rssi_wb1a_gc_stored =
			phy_utils_read_radioreg(pi,
			RADIO_2059_RSSI_GPAIOSEL_W1_IDACS(1)) & 0xc;
		pi->interf->radio_2057_core2_rssi_wb1a_gc_stored =
			phy_utils_read_radioreg(pi,
			RADIO_2059_RSSI_GPAIOSEL_W1_IDACS(2)) & 0xc;
		pi->interf->radio_2057_core0_rssi_wb1g_gc_stored =
			phy_utils_read_radioreg(pi,
			RADIO_2059_RSSI_GPAIOSEL_W1_IDACS(0)) & 0x3;
		pi->interf->radio_2057_core1_rssi_wb1g_gc_stored =
			phy_utils_read_radioreg(pi,
			RADIO_2059_RSSI_GPAIOSEL_W1_IDACS(1)) & 0x3;
		pi->interf->radio_2057_core2_rssi_wb1g_gc_stored =
			phy_utils_read_radioreg(pi,
			RADIO_2059_RSSI_GPAIOSEL_W1_IDACS(2)) & 0x3;
		pi->interf->radio_2057_core0_rssi_wb2_gc_stored =
			phy_utils_read_radioreg(pi,
			RADIO_2059_W2_MASTER(0)) & 0xc0;
		pi->interf->radio_2057_core1_rssi_wb2_gc_stored =
			phy_utils_read_radioreg(pi,
			RADIO_2059_W2_MASTER(1)) & 0xc0;
		pi->interf->radio_2057_core2_rssi_wb2_gc_stored =
			phy_utils_read_radioreg(pi,
			RADIO_2059_W2_MASTER(2)) & 0xc0;

		phy_utils_write_radioreg(pi, RADIO_2059_NB_MASTER(0), 0x40 |
			(phy_utils_read_radioreg(pi, RADIO_2059_NB_MASTER(0)) & 0x1f));
		phy_utils_write_radioreg(pi, RADIO_2059_NB_MASTER(1), 0x40 |
			(phy_utils_read_radioreg(pi, RADIO_2059_NB_MASTER(1)) & 0x1f));
		phy_utils_write_radioreg(pi, RADIO_2059_NB_MASTER(2), 0x40 |
			(phy_utils_read_radioreg(pi, RADIO_2059_NB_MASTER(2)) & 0x1f));
		phy_utils_write_radioreg(pi, RADIO_2059_RSSI_GPAIOSEL_W1_IDACS(0), 0x0 |
			(phy_utils_read_radioreg(pi, RADIO_2059_RSSI_GPAIOSEL_W1_IDACS(0))
			& 0x33));
		phy_utils_write_radioreg(pi, RADIO_2059_RSSI_GPAIOSEL_W1_IDACS(1), 0x0 |
			(phy_utils_read_radioreg(pi, RADIO_2059_RSSI_GPAIOSEL_W1_IDACS(1))
			& 0x33));
		phy_utils_write_radioreg(pi, RADIO_2059_RSSI_GPAIOSEL_W1_IDACS(2), 0x0 |
			(phy_utils_read_radioreg(pi, RADIO_2059_RSSI_GPAIOSEL_W1_IDACS(2))
			& 0x33));
		phy_utils_write_radioreg(pi, RADIO_2059_RSSI_GPAIOSEL_W1_IDACS(0), 0x0 |
			(phy_utils_read_radioreg(pi, RADIO_2059_RSSI_GPAIOSEL_W1_IDACS(0))
			& 0x3c));
		phy_utils_write_radioreg(pi, RADIO_2059_RSSI_GPAIOSEL_W1_IDACS(1), 0x0 |
			(phy_utils_read_radioreg(pi, RADIO_2059_RSSI_GPAIOSEL_W1_IDACS(1))
			& 0x3c));
		phy_utils_write_radioreg(pi, RADIO_2059_RSSI_GPAIOSEL_W1_IDACS(2), 0x0 |
			(phy_utils_read_radioreg(pi, RADIO_2059_RSSI_GPAIOSEL_W1_IDACS(2))
			& 0x3c));
		phy_utils_write_radioreg(pi, RADIO_2059_W2_MASTER(0), 0x40 |
			(phy_utils_read_radioreg(pi, RADIO_2059_W2_MASTER(0)) & 0x3f));
		phy_utils_write_radioreg(pi, RADIO_2059_W2_MASTER(1), 0x40 |
			(phy_utils_read_radioreg(pi, RADIO_2059_W2_MASTER(1)) & 0x3f));
		phy_utils_write_radioreg(pi, RADIO_2059_W2_MASTER(2), 0x40 |
			(phy_utils_read_radioreg(pi, RADIO_2059_W2_MASTER(2)) & 0x3f));

		/* w1 threshold */
		/* Adjust W1 Clip thresh based on gainctrl changes */
		pi->interf->w1_clip_thresh_core0_stored =
			phy_utils_read_phyreg(pi, HTPHY_Core0clipwbThreshold2059);
		pi->interf->w1_clip_thresh_core1_stored =
			phy_utils_read_phyreg(pi, HTPHY_Core1clipwbThreshold2059);
		pi->interf->w1_clip_thresh_core2_stored =
			phy_utils_read_phyreg(pi, HTPHY_Core2clipwbThreshold2059);
		w1_thresh = HTPHY_RSSICAL_W1_TARGET - 4;
		phy_utils_mod_phyreg(pi, HTPHY_Core0clipwbThreshold2059,
			HTPHY_Core0clipwbThreshold2059_clip1wbThreshold_MASK,
			(w1_thresh <<
			HTPHY_Core0clipwbThreshold2059_clip1wbThreshold_SHIFT));
		phy_utils_mod_phyreg(pi, HTPHY_Core1clipwbThreshold2059,
			HTPHY_Core1clipwbThreshold2059_clip1wbThreshold_MASK,
			(w1_thresh <<
			HTPHY_Core1clipwbThreshold2059_clip1wbThreshold_SHIFT));
		phy_utils_mod_phyreg(pi, HTPHY_Core2clipwbThreshold2059,
			HTPHY_Core2clipwbThreshold2059_clip1wbThreshold_MASK,
			(w1_thresh <<
			HTPHY_Core2clipwbThreshold2059_clip1wbThreshold_SHIFT));

		/* ED CRS changes */
		if (!pi->edcrs_threshold_lock) {
			phy_utils_write_phyreg(pi, HTPHY_ed_crs20LAssertThresh0, 0x42b);
			phy_utils_write_phyreg(pi, HTPHY_ed_crs20LAssertThresh1, 0x42b);
			phy_utils_write_phyreg(pi, HTPHY_ed_crs20LDeassertThresh0, 0x381);
			phy_utils_write_phyreg(pi, HTPHY_ed_crs20LDeassertThresh1, 0x381);
			phy_utils_write_phyreg(pi, HTPHY_ed_crs20UAssertThresh0, 0x42b);
			phy_utils_write_phyreg(pi, HTPHY_ed_crs20UAssertThresh1, 0x42b);
			phy_utils_write_phyreg(pi, HTPHY_ed_crs20UDeassertThresh0, 0x381);
			phy_utils_write_phyreg(pi, HTPHY_ed_crs20UDeassertThresh1, 0x381);
		}

		wlc_phy_aci_pwr_upd_htphy(pi, aci_pwr);

	} else {
		/* Disable ACI mitigation */

		PHY_ACI(("Disable ACI mitigation, on channel %d\n",
			CHSPEC_CHANNEL(pi->radio_chanspec)));

		if ((pi->interf->init_gain_code_core0_stored == 0) ||
			(pi->interf->init_gain_code_core1_stored == 0) ||
			(pi->interf->init_gain_code_core2_stored == 0) ||
			(pi->interf->init_gain_codeb_core0_stored == 0) ||
			(pi->interf->init_gain_codeb_core1_stored == 0) ||
			(pi->interf->init_gain_codeb_core2_stored == 0)) {
			return;
		}

		band_switch = (CHSPEC_CHANNEL(pi->interf->radio_chanspec_stored) > 14 &&
			CHSPEC_CHANNEL(pi->radio_chanspec) <= 14) ||
			((CHSPEC_CHANNEL(pi->interf->radio_chanspec_stored) <= 14) &&
			(CHSPEC_CHANNEL(pi->radio_chanspec) > 14));

		bw_switch = ((CHSPEC_IS20(pi->interf->radio_chanspec_stored) == 1) &&
			(CHSPEC_IS20(pi->radio_chanspec) == 0)) ||
			((CHSPEC_IS20(pi->interf->radio_chanspec_stored) == 0) &&
			(CHSPEC_IS20(pi->radio_chanspec) == 1));

		/* get back to detection, as soon as a previous detection fails */

		if (!band_switch && !bw_switch) {
		/* set ACI phyregs to default values */

		/*
			phy_utils_write_phyreg(pi, HTPHY_energydroptimeoutLen,
				pi->interf->energy_drop_timeout_len_stored);
		*/

		/* restore clip hi gains */
		phy_utils_write_phyreg(pi, HTPHY_Core0clipHiGainCodeA2059,
			pi->interf->clip1_hi_gain_code_core0_stored);
		phy_utils_write_phyreg(pi, HTPHY_Core1clipHiGainCodeA2059,
			pi->interf->clip1_hi_gain_code_core1_stored);
		phy_utils_write_phyreg(pi, HTPHY_Core2clipHiGainCodeA2059,
			pi->interf->clip1_hi_gain_code_core2_stored);
		phy_utils_write_phyreg(pi, HTPHY_Core0clipHiGainCodeB2059,
			pi->interf->clip1_hi_gain_codeb_core0_stored);
		phy_utils_write_phyreg(pi, HTPHY_Core1clipHiGainCodeB2059,
			pi->interf->clip1_hi_gain_codeb_core1_stored);
		phy_utils_write_phyreg(pi, HTPHY_Core2clipHiGainCodeB2059,
			pi->interf->clip1_hi_gain_codeb_core2_stored);

		/* restore nb clip thresholds */
		phy_utils_write_phyreg(pi, HTPHY_Core0nbClipThreshold,
			pi->interf->nb_clip_thresh_core0_stored);
		phy_utils_write_phyreg(pi, HTPHY_Core1nbClipThreshold,
			pi->interf->nb_clip_thresh_core1_stored);
		phy_utils_write_phyreg(pi, HTPHY_Core2nbClipThreshold,
			pi->interf->nb_clip_thresh_core2_stored);

		/* restore gainlimits */
		if (IS_X29D_BOARDTYPE(pi) && CHSPEC_IS2G(pi->radio_chanspec)) {
			/* XXX
			 * PR108447
			 * Hack to account for X29D BT-coex desense: restore only if
			 * btcoex desense is no longer active.
			 */
			if (!pi->u.pi_htphy->btc_restage_rxgain) {
				/* XXX
				 * Don't use aci sw-stored values to restore, as these might
				 * be incorrect if btcoex-desense was active at the time
				 * of saving. Instead, use hard-coded values.
				 *
				 * ASSUMPTION:
				 * - ACI mitigation only writes to lna2 index3 entry of table.
				 * - The default value of lna2 index3 entry is 0x3.
				 */
				uint8 lna2idx3_default = 3;
				wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_GAINLIMIT, 1, 0x13, 8,
				                          &lna2idx3_default);
			}
		} else {
			wlc_phy_table_write_htphy(pi, 4, 4, 0x10, 16,
			                          pi->interf->init_ofdmlna2gainchange_stored);
		}
		wlc_phy_table_write_htphy(pi, 4, 4, 0x50, 16,
		                          pi->interf->init_ccklna2gainchange_stored);

		/* restore clip lo gain */
		phy_utils_write_phyreg(pi, HTPHY_Core0cliploGainCodeA2059,
			pi->interf->clip1_lo_gain_code_core0_stored);
		phy_utils_write_phyreg(pi, HTPHY_Core1cliploGainCodeA2059,
			pi->interf->clip1_lo_gain_code_core1_stored);
		phy_utils_write_phyreg(pi, HTPHY_Core2cliploGainCodeA2059,
			pi->interf->clip1_lo_gain_code_core2_stored);
		phy_utils_write_phyreg(pi, HTPHY_Core0cliploGainCodeB2059,
			pi->interf->clip1_lo_gain_codeb_core0_stored);
		phy_utils_write_phyreg(pi, HTPHY_Core1cliploGainCodeB2059,
			pi->interf->clip1_lo_gain_codeb_core1_stored);
		phy_utils_write_phyreg(pi, HTPHY_Core2cliploGainCodeB2059,
			pi->interf->clip1_lo_gain_codeb_core2_stored);

		/* Adjust W1 Clip thresh based on gainctrl changes */
		phy_utils_write_phyreg(pi, HTPHY_Core0clipwbThreshold2059,
			pi->interf->w1_clip_thresh_core0_stored);
		phy_utils_write_phyreg(pi, HTPHY_Core1clipwbThreshold2059,
			pi->interf->w1_clip_thresh_core1_stored);
		phy_utils_write_phyreg(pi, HTPHY_Core2clipwbThreshold2059,
			pi->interf->w1_clip_thresh_core2_stored);

		/* restore ed values */
		if (!pi->edcrs_threshold_lock) {
			phy_utils_write_phyreg(pi, HTPHY_ed_crs20LAssertThresh0,
				pi->interf->ed_crs20L_assertthld0_stored);
			phy_utils_write_phyreg(pi, HTPHY_ed_crs20LAssertThresh1,
				pi->interf->ed_crs20L_assertthld1_stored);
			phy_utils_write_phyreg(pi, HTPHY_ed_crs20LDeassertThresh0,
				pi->interf->ed_crs20L_deassertthld0_stored);
			phy_utils_write_phyreg(pi, HTPHY_ed_crs20LDeassertThresh1,
				pi->interf->ed_crs20L_deassertthld1_stored);
			phy_utils_write_phyreg(pi, HTPHY_ed_crs20UAssertThresh0,
				pi->interf->ed_crs20U_assertthld0_stored);
			phy_utils_write_phyreg(pi, HTPHY_ed_crs20UAssertThresh1,
				pi->interf->ed_crs20U_assertthld1_stored);
			phy_utils_write_phyreg(pi, HTPHY_ed_crs20UDeassertThresh0,
			pi->interf->ed_crs20U_deassertthld0_stored);
			phy_utils_write_phyreg(pi, HTPHY_ed_crs20UDeassertThresh1,
				pi->interf->ed_crs20U_deassertthld1_stored);
			}
		}

		if (!band_switch) {
			/* set ACI radioregs to default values */
			/* restore rssi gain */
			phy_utils_write_radioreg(pi, RADIO_2059_NB_MASTER(0),
				pi->interf->radio_2057_core0_rssi_nb_gc_stored |
				(phy_utils_read_radioreg(pi, RADIO_2059_NB_MASTER(0)) & 0x1f));
			phy_utils_write_radioreg(pi, RADIO_2059_NB_MASTER(1),
				pi->interf->radio_2057_core1_rssi_nb_gc_stored |
				(phy_utils_read_radioreg(pi, RADIO_2059_NB_MASTER(1)) & 0x1f));
			phy_utils_write_radioreg(pi, RADIO_2059_NB_MASTER(2),
				pi->interf->radio_2057_core2_rssi_nb_gc_stored |
				(phy_utils_read_radioreg(pi, RADIO_2059_NB_MASTER(2)) & 0x1f));
			phy_utils_write_radioreg(pi, RADIO_2059_RSSI_GPAIOSEL_W1_IDACS(0),
				pi->interf->radio_2057_core0_rssi_wb1a_gc_stored |
				(phy_utils_read_radioreg(pi, RADIO_2059_RSSI_GPAIOSEL_W1_IDACS(0))
				& 0x33));
			phy_utils_write_radioreg(pi, RADIO_2059_RSSI_GPAIOSEL_W1_IDACS(1),
				pi->interf->radio_2057_core1_rssi_wb1a_gc_stored |
				(phy_utils_read_radioreg(pi, RADIO_2059_RSSI_GPAIOSEL_W1_IDACS(1))
				& 0x33));
			phy_utils_write_radioreg(pi, RADIO_2059_RSSI_GPAIOSEL_W1_IDACS(2),
				pi->interf->radio_2057_core2_rssi_wb1a_gc_stored |
				(phy_utils_read_radioreg(pi, RADIO_2059_RSSI_GPAIOSEL_W1_IDACS(2))
				& 0x33));
			phy_utils_write_radioreg(pi, RADIO_2059_RSSI_GPAIOSEL_W1_IDACS(0),
				pi->interf->radio_2057_core0_rssi_wb1g_gc_stored |
				(phy_utils_read_radioreg(pi, RADIO_2059_RSSI_GPAIOSEL_W1_IDACS(0))
				& 0x3c));
			phy_utils_write_radioreg(pi, RADIO_2059_RSSI_GPAIOSEL_W1_IDACS(1),
				pi->interf->radio_2057_core1_rssi_wb1g_gc_stored |
				(phy_utils_read_radioreg(pi, RADIO_2059_RSSI_GPAIOSEL_W1_IDACS(1))
				& 0x3c));
			phy_utils_write_radioreg(pi, RADIO_2059_RSSI_GPAIOSEL_W1_IDACS(2),
				pi->interf->radio_2057_core2_rssi_wb1g_gc_stored |
				(phy_utils_read_radioreg(pi, RADIO_2059_RSSI_GPAIOSEL_W1_IDACS(2))
				& 0x3c));
			phy_utils_write_radioreg(pi, RADIO_2059_W2_MASTER(0),
				pi->interf->radio_2057_core0_rssi_wb2_gc_stored |
				(phy_utils_read_radioreg(pi, RADIO_2059_W2_MASTER(0)) & 0x3f));
			phy_utils_write_radioreg(pi, RADIO_2059_W2_MASTER(1),
				pi->interf->radio_2057_core1_rssi_wb2_gc_stored |
				(phy_utils_read_radioreg(pi, RADIO_2059_W2_MASTER(1)) & 0x3f));
			phy_utils_write_radioreg(pi, RADIO_2059_W2_MASTER(2),
				pi->interf->radio_2057_core2_rssi_wb2_gc_stored |
			(phy_utils_read_radioreg(pi, RADIO_2059_W2_MASTER(2)) & 0x3f));

		}
	}
}

void
wlc_phy_cal_init_htphy(phy_info_t *pi)
{
	wlc_phy_aci_init_htphy(pi);
	wlc_phy_aci_sw_reset_htphy(pi);
}

static void
wlc_phy_pcieingress_war_htphy(phy_info_t *pi)
{
	int16 fc;

	if (CHSPEC_CHANNEL(pi->radio_chanspec) > 14) {
		fc = CHAN5G_FREQ(CHSPEC_CHANNEL(pi->radio_chanspec));
	} else {
		fc = CHAN2G_FREQ(CHSPEC_CHANNEL(pi->radio_chanspec));
	}
	PHY_INFORM(("PCIEINGRESS_WAR= %d", pi->pcieingress_war));

	if (pi->pcieingress_war == 0xf)
		return;
	if (pi->pcieingress_war == 0x0) {
		if (CHSPEC_IS5G(pi->radio_chanspec)) {
			if (CHSPEC_IS20(pi->radio_chanspec) == 1) {
				if ((fc >= 4920 && fc <= 5230)) {
					uint16 crsminu  = 72;
					uint16 crsminl  = 72;
					MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, crsminu);
					MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, crsminl);
				} else if ((fc >= 5240 && fc <= 5550)) {
					uint16 crsminu  = 72;
					uint16 crsminl  = 72;
					MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, crsminu);
					MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, crsminl);

				} else if ((fc >= 5560 && fc <= 5825)) {
					uint16 crsminu  = 66;
					uint16 crsminl  = 66;
					MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, crsminu);
					MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, crsminl);
				}
			} else {
				if ((fc >= 4920 && fc <= 5230)) {
					uint16 crsminu  = 72;
					uint16 crsminl  = 72;
					MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, crsminu);
					MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, crsminl);
				} else if ((fc >= 5240 && fc <= 5550)) {
					uint16 crsminu  = 68;
					uint16 crsminl  = 68;
					MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, crsminu);
					MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, crsminl);
				} else if ((fc >= 5560 && fc <= 5825)) {
					uint16 crsminu  = 64;
					uint16 crsminl  = 64;
					MOD_PHYREG(pi, HTPHY_crsminpoweru0, crsminpower0, crsminu);
					MOD_PHYREG(pi, HTPHY_crsminpowerl0, crsminpower0, crsminl);
				}
			}
		}
	} else {
		PHY_ERROR(("PCIEINGRESS_WAR= %d is not defined yet\n",
			pi->pcieingress_war));
	}
}

/* Function to enable external LNA and/or external PA Control Lines */
static void wlc_phy_enable_extlna_extpa_htphy(phy_info_t *pi)
{
	uint32 chipc_mask_on = 0;
	uint32 chipc_mask_off = 0;
	bool ext_lna_2g_flag = ((BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) &
		BFL_EXTLNA) != 0);
	bool ext_lna_5g_flag = ((BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) &
		BFL_EXTLNA_5GHz) != 0);
	bool ext_pa_ana_2g =  ((BOARDFLAGS2(GENERIC_PHY_INFO(pi)->boardflags2) &
		BFL2_ANAPACTRL_2G) != 0);
	bool ext_pa_ana_5g =  ((BOARDFLAGS2(GENERIC_PHY_INFO(pi)->boardflags2) &
		BFL2_ANAPACTRL_5G) != 0);
	uint8 extpagain2g = pi->fem2g->extpagain;
	uint8 extpagain5g = pi->fem5g->extpagain;
	bool  digital_2g_pa = (pi->sh->did != BCM4331_D11N5G_ID) &&
	        (ext_pa_ana_2g == 0) && (extpagain2g != 2);
	bool  digital_5g_pa = (pi->sh->did != BCM4331_D11N2G_ID) &&
	        (ext_pa_ana_5g == 0) && (extpagain5g != 2);

	uint16 HTPHY_Rfctrl_2Gx5G_Dup_ExtLna_coreALL_MASK;
	bool DUP_Turn_ON = 0;

	HTPHY_Rfctrl_2Gx5G_Dup_ExtLna_coreALL_MASK =
	        (HTPHY_Rfctrl_2Gx5G_Dup_ExtLna_core0_MASK) |
	        (HTPHY_Rfctrl_2Gx5G_Dup_ExtLna_core1_MASK) |
	        (HTPHY_Rfctrl_2Gx5G_Dup_ExtLna_core2_MASK);
	if ((CHSPEC_IS2G(pi->radio_chanspec) && ext_pa_ana_2g) ||
	    (CHSPEC_IS5G(pi->radio_chanspec) && ext_pa_ana_5g)) {
		uint8 rfseq_tx2rx_dlys[] = {70, 4, 2, 2, 4, 4, 6, 1};
		uint8 rfseq_tx2rx_events[] = {
			HTPHY_RFSEQ_CMD_EXT_PA,
			HTPHY_RFSEQ_CMD_INT_PA_PU,
			HTPHY_RFSEQ_CMD_TX_GAIN,
			HTPHY_RFSEQ_CMD_RXPD_TXPD,
			HTPHY_RFSEQ_CMD_TR_SWITCH,
			HTPHY_RFSEQ_CMD_RXG_FBW,
			HTPHY_RFSEQ_CMD_CLR_HIQ_DIS,
			HTPHY_RFSEQ_CMD_END,
			HTPHY_RFSEQ_CMD_END
		};
		wlc_phy_set_rfseq_htphy(pi, HTPHY_RFSEQ_TX2RX, rfseq_tx2rx_events, rfseq_tx2rx_dlys,
		                        sizeof(rfseq_tx2rx_events)/sizeof(rfseq_tx2rx_events[0]));
		chipc_mask_on = chipc_mask_on | CCTRL4331_EXTPA_ANA_EN;
		si_pmu_set_ldo_voltage(pi->sh->sih, pi->sh->osh, SET_LDO_VOLTAGE_PAREF,
		                       pi->ldo_voltage/5+9);
	} else {
		chipc_mask_off = chipc_mask_off | CCTRL4331_EXTPA_ANA_EN;
	}
	if ((pi->sh->chippkg == BCM4331TN_PKG_ID) || (pi->sh->chippkg == BCM4331TNA0_PKG_ID)) {
		/* 12x9 Code starts here */

		if (digital_2g_pa) {
			chipc_mask_on = chipc_mask_on |  CCTRL4331_EXTPA_EN;
			chipc_mask_off = chipc_mask_off | CCTRL4331_EXT_LNA_G;
		} else {
			chipc_mask_off = chipc_mask_off | CCTRL4331_EXTPA_EN;
			if (ext_lna_2g_flag || ext_lna_5g_flag) {
				chipc_mask_on = chipc_mask_on | CCTRL4331_EXT_LNA_G;
			} else {
				chipc_mask_off = chipc_mask_off | CCTRL4331_EXT_LNA_G;
			}
		}

		if (digital_5g_pa) {
			chipc_mask_on =  chipc_mask_on | CCTRL4331_EXTPA_ON_GPIO2_5;
		} else {
			chipc_mask_off =  chipc_mask_off | CCTRL4331_EXTPA_ON_GPIO2_5;
		}

		if (ext_lna_5g_flag) {
				DUP_Turn_ON = 1;
		}

		/* 12x9 Code ends here */
	} else {

		/* 12x12 Code starts here */
		if (digital_2g_pa) {
			chipc_mask_on = chipc_mask_on |  CCTRL4331_EXTPA_EN;
		} else {
			chipc_mask_off = chipc_mask_off | CCTRL4331_EXTPA_EN;
		}

		if (digital_5g_pa) {
			chipc_mask_on = chipc_mask_on | CCTRL4331_EXTPA_EN2;

		} else {
			chipc_mask_off = chipc_mask_off | CCTRL4331_EXTPA_EN2;
		}

		if (ext_lna_2g_flag) {
			if (digital_2g_pa) {
				if (!digital_5g_pa) {
					/* Use Ext LNA/PA 5G lines */
					chipc_mask_on = chipc_mask_on | CCTRL4331_EXT_LNA_A;
					chipc_mask_off = chipc_mask_off | CCTRL4331_EXT_LNA_G;
					DUP_Turn_ON = 1;
				} else {
					PHY_ERROR(("illegal combo for chipcontrol \n "));
					ASSERT(0);

				}
			} else {
				/* Use Ext LNA 2G Lines */
				chipc_mask_on = chipc_mask_on | CCTRL4331_EXT_LNA_G;
			}
		}
		if (ext_lna_5g_flag) {
			if (digital_5g_pa) {
				if (!digital_2g_pa) {
					/* Use Ext LNA/PA 2G lines */
					chipc_mask_on = chipc_mask_on | CCTRL4331_EXT_LNA_G;
					chipc_mask_off = chipc_mask_off | CCTRL4331_EXT_LNA_A;
					DUP_Turn_ON = 1;
				} else {
					PHY_ERROR(("illegal combo for chipcontrol \n"));
					ASSERT(0);
				}
			} else {
				/* Use Ext LNA 5G Lines */
				chipc_mask_on = chipc_mask_on | CCTRL4331_EXT_LNA_A;
			}
		}

		/* 12x12 Code ends here */
	}
	/* Apply the DUP Register Setting */
	if (DUP_Turn_ON) {

		HTPHY_Rfctrl_2Gx5G_Dup_ExtLna_coreALL_MASK =
		        (HTPHY_Rfctrl_2Gx5G_Dup_ExtLna_core0_MASK) |
		        (HTPHY_Rfctrl_2Gx5G_Dup_ExtLna_core1_MASK) |
		        (HTPHY_Rfctrl_2Gx5G_Dup_ExtLna_core2_MASK);

		phy_utils_mod_phyreg(pi, HTPHY_Rfctrl_2Gx5G,
		            HTPHY_Rfctrl_2Gx5G_Dup_ExtLna_coreALL_MASK,
		            HTPHY_Rfctrl_2Gx5G_Dup_ExtLna_coreALL_MASK);
	}
	/* Apply the ChipControl Register Setting */
	si_corereg(pi->sh->sih, SI_CC_IDX, OFFSETOF(chipcregs_t, chipcontrol),
	           chipc_mask_on | chipc_mask_off, chipc_mask_on);
}

#if defined(PHYCAL_CACHING)
static void wlc_phy_cal_cache_htphy(wlc_phy_t *pih)
{
	phy_info_t *pi = (phy_info_t *) pih;
	ch_calcache_t *ctx;
	htphy_calcache_t *cache;
	uint8 core;
	uint16 tbl_cookie;
	uint16 chan = CHSPEC_CHANNEL(pi->radio_chanspec);

	ctx = wlc_phy_get_chanctx(pi, pi->radio_chanspec);

	/* A context must have been created before reaching here */
	ASSERT(ctx != NULL);
	if (ctx == NULL)
		return;

	/* Ensure that the Callibration Results are valid */
	wlc_phy_table_read_htphy(pi, HTPHY_TBL_ID_IQLOCAL, 1,
		IQTBL_CACHE_COOKIE_OFFSET, 16, &tbl_cookie);
	if (tbl_cookie != TXCAL_CACHE_VALID) {
		return;
	}

	ctx->valid = TRUE;

	cache = &ctx->u.htphy_cache;

	/* save the callibration to cache */
	FOREACH_CORE(pi, core) {
		uint16 ab_int[2];
		/* Save OFDM Tx IQ Imb Coeffs A,B and Digital Loft Comp Coeffs */
		wlc_phy_cal_txiqlo_coeffs_htphy(pi, CAL_COEFF_READ,
		                                ab_int, TB_OFDM_COEFFS_AB, core);
		cache->ofdm_txa[core] = ab_int[0];
		cache->ofdm_txb[core] = ab_int[1];
		wlc_phy_cal_txiqlo_coeffs_htphy(pi, CAL_COEFF_READ,
		                                &cache->ofdm_txd[core], TB_OFDM_COEFFS_D, core);
		/* Save OFDM Tx IQ Imb Coeffs A,B and Digital Loft Comp Coeffs */
		wlc_phy_cal_txiqlo_coeffs_htphy(pi, CAL_COEFF_READ,
		                                ab_int, TB_BPHY_COEFFS_AB, core);
		cache->bphy_txa[core] = ab_int[0];
		cache->bphy_txb[core] = ab_int[1];
		wlc_phy_cal_txiqlo_coeffs_htphy(pi, CAL_COEFF_READ,
		                                &cache->bphy_txd[core], TB_BPHY_COEFFS_D, core);
		/* Save Analog Tx Loft Comp Coeffs */
		cache->txei[core] = (uint8)phy_utils_read_radioreg(pi,
			RADIO_2059_TX_LOFT_FINE_I(core));
		cache->txeq[core] = (uint8)phy_utils_read_radioreg(pi,
			RADIO_2059_TX_LOFT_FINE_Q(core));
		cache->txfi[core] = (uint8)phy_utils_read_radioreg(pi,
			RADIO_2059_TX_LOFT_COARSE_I(core));
		cache->txfq[core] = (uint8)phy_utils_read_radioreg(pi,
			RADIO_2059_TX_LOFT_COARSE_Q(core));
		/* Save Rx IQ Imb Coeffs */
		cache->rxa[core] = phy_utils_read_phyreg(pi, HTPHY_RxIQCompA(core));
		cache->rxb[core] = phy_utils_read_phyreg(pi, HTPHY_RxIQCompB(core));

		if (pi->itssical || (pi->itssi_cap_low5g && chan <= 48 && chan >= 36)) {
			/* Save Idle TSSI & Vmid values */
			/* Read idle TSSI in 2s complement format (max is 0x1f) */
			switch (core) {
			case 0:
				cache->idle_tssi[core] =
				        READ_PHYREG(pi, HTPHY_TxPwrCtrlIdleTssi, idleTssi0);
				break;
			case 1:
				cache->idle_tssi[core] =
				        READ_PHYREG(pi, HTPHY_TxPwrCtrlIdleTssi, idleTssi1);
				break;
			case 2:
				cache->idle_tssi[core] =
				        READ_PHYREG(pi, HTPHY_TxPwrCtrlIdleTssi1, idleTssi2);
				break;
			default:
				ASSERT(0);
			}

			/* Save Vmid values */
			wlc_phy_table_read_htphy(pi, HTPHY_TBL_ID_AFECTRL, 1, 0x0b + 0x10*core,
			                         16, &cache->Vmid[core]);
		}
	}

	PHY_CAL(("wl%d: %s: Cached cal values for chanspec 0x%x are:\n",
		pi->sh->unit, __FUNCTION__,  ctx->chanspec));
	wlc_phy_cal_cache_dbg_htphy(pih, ctx);
}

static void
wlc_phy_cal_cache_dbg_htphy(wlc_phy_t *pih, ch_calcache_t *ctx)
{
	uint i;
	htphy_calcache_t *cache = NULL;
	phy_info_t *pi = (phy_info_t *) pih;
	uint16 chan = CHSPEC_CHANNEL(pi->radio_chanspec);

	if (ISHTPHY(pi)) {
		cache = &ctx->u.htphy_cache;
	} else
		return;

	BCM_REFERENCE(cache);
	FOREACH_CORE(pi, i) {
		PHY_CAL(("CORE %d:\n", i));
		PHY_CAL(("\tofdm_txa:0x%x  ofdm_txb:0x%x  ofdm_txd:0x%x\n",
		            cache->ofdm_txa[i], cache->ofdm_txb[i], cache->ofdm_txd[i]));
		PHY_CAL(("\tbphy_txa:0x%x  bphy_txb:0x%x  bphy_txd:0x%x\n",
		            cache->bphy_txa[i], cache->bphy_txb[i], cache->bphy_txd[i]));
		PHY_CAL(("\ttxei:0x%x  txeq:0x%x\n", cache->txei[i], cache->txeq[i]));
		PHY_CAL(("\ttxfi:0x%x  txfq:0x%x\n", cache->txfi[i], cache->txfq[i]));
		PHY_CAL(("\trxa:0x%x  rxb:0x%x\n", cache->rxa[i], cache->rxb[i]));
		if (pi->itssical || (pi->itssi_cap_low5g && chan <= 48 && chan >= 36)) {
			PHY_CAL(("\tidletssi:0x%x\n", cache->idle_tssi[i]));
			PHY_CAL(("\tvmid:0x%x\n", cache->Vmid[i]));
		}
	}
}

#ifdef BCMDBG
void
wlc_phydump_cal_cache_htphy(phy_info_t *pi, ch_calcache_t *ctx, struct bcmstrbuf *b)
{
	uint i;
	htphy_calcache_t *cache = NULL;
	uint16 chan = CHSPEC_CHANNEL(pi->radio_chanspec);

	if (ISHTPHY(pi)) {
		cache = &ctx->u.htphy_cache;
	} else
		return;

	FOREACH_CORE(pi, i) {
		bcm_bprintf(b, "CORE %d:\n", i);
		bcm_bprintf(b, "\tofdm_txa:0x%x  ofdm_txb:0x%x  ofdm_txd:0x%x\n",
		            cache->ofdm_txa[i], cache->ofdm_txb[i], cache->ofdm_txd[i]);
		bcm_bprintf(b, "\tbphy_txa:0x%x  bphy_txb:0x%x  bphy_txd:0x%x\n",
		            cache->bphy_txa[i], cache->bphy_txb[i], cache->bphy_txd[i]);
		bcm_bprintf(b, "\ttxei:0x%x  txeq:0x%x\n", cache->txei[i], cache->txeq[i]);
		bcm_bprintf(b, "\ttxfi:0x%x  txfq:0x%x\n", cache->txfi[i], cache->txfq[i]);
		bcm_bprintf(b, "\trxa:0x%x  rxb:0x%x\n", cache->rxa[i], cache->rxb[i]);
		if (pi->itssical || (pi->itssi_cap_low5g && chan <= 48 && chan >= 36)) {
			bcm_bprintf(b, "\tidletssi:0x%x\n", cache->idle_tssi[i]);
			bcm_bprintf(b, "\tvmid:0x%x\n", cache->Vmid[i]);
		}
	}
}
#endif /* BCMDBG */

int
wlc_phy_cal_cache_restore_htphy(phy_info_t *pi)
{
	ch_calcache_t *ctx;
	htphy_calcache_t *cache = NULL;
	bool suspend;
	uint8 core;
	uint16 tbl_cookie = TXCAL_CACHE_VALID;
	uint16 chan = CHSPEC_CHANNEL(pi->radio_chanspec);

	ctx = wlc_phy_get_chanctx(pi, pi->radio_chanspec);

	if (!ctx) {
		PHY_ERROR(("wl%d: %s: Chanspec 0x%x not found in calibration cache\n",
		           pi->sh->unit, __FUNCTION__, pi->radio_chanspec));
		return BCME_ERROR;
	}

	if (!ctx->valid) {
		PHY_ERROR(("wl%d: %s: Chanspec 0x%x found, but not valid in phycal cache\n",
		           pi->sh->unit, __FUNCTION__, ctx->chanspec));
		return BCME_ERROR;
	}

	PHY_CAL(("wl%d: %s: Restoring all cal coeffs from calibration cache for chanspec 0x%x\n",
	           pi->sh->unit, __FUNCTION__, pi->radio_chanspec));

	cache = &ctx->u.htphy_cache;

	suspend = !(R_REG(pi->sh->osh, &pi->regs->maccontrol) & MCTL_EN_MAC);
	if (!suspend) {
		/* suspend mac */
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
	}
	phy_utils_phyreg_enter(pi);

	/* restore the txcal from cache */
	FOREACH_CORE(pi, core) {
		uint16 ab_int[2];
		/* Restore OFDM Tx IQ Imb Coeffs A,B and Digital Loft Comp Coeffs */
		ab_int[0] = cache->ofdm_txa[core];
		ab_int[1] = cache->ofdm_txb[core];
		wlc_phy_cal_txiqlo_coeffs_htphy(pi, CAL_COEFF_WRITE,
		                                ab_int, TB_OFDM_COEFFS_AB, core);
		wlc_phy_cal_txiqlo_coeffs_htphy(pi, CAL_COEFF_WRITE,
		                                &cache->ofdm_txd[core], TB_OFDM_COEFFS_D, core);
		/* Restore BPHY Tx IQ Imb Coeffs A,B and Digital Loft Comp Coeffs */
		ab_int[0] = cache->bphy_txa[core];
		ab_int[1] = cache->bphy_txb[core];
		wlc_phy_cal_txiqlo_coeffs_htphy(pi, CAL_COEFF_WRITE,
		                                ab_int, TB_BPHY_COEFFS_AB, core);
		wlc_phy_cal_txiqlo_coeffs_htphy(pi, CAL_COEFF_WRITE,
		                                &cache->bphy_txd[core], TB_BPHY_COEFFS_D, core);
		/* Restore Analog Tx Loft Comp Coeffs */
		phy_utils_write_radioreg(pi, RADIO_2059_TX_LOFT_FINE_I(core),
		                cache->txei[core]);
		phy_utils_write_radioreg(pi, RADIO_2059_TX_LOFT_FINE_Q(core),
		                cache->txeq[core]);
		phy_utils_write_radioreg(pi, RADIO_2059_TX_LOFT_COARSE_I(core),
		                cache->txfi[core]);
		phy_utils_write_radioreg(pi, RADIO_2059_TX_LOFT_COARSE_Q(core),
		                cache->txfq[core]);
		/* Restore Rx IQ Imb Coeffs */
		phy_utils_write_phyreg(pi, HTPHY_RxIQCompA(core), cache->rxa[core]);
		phy_utils_write_phyreg(pi, HTPHY_RxIQCompB(core), cache->rxb[core]);
		if (pi->itssical || (pi->itssi_cap_low5g && chan <= 48 && chan >= 36)) {
			/* Restore Idle TSSI & Vmid values */
			wlc_phy_txpwrctrl_set_idle_tssi_htphy(pi, cache->idle_tssi[core], core);
			wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_AFECTRL, 1, 0x0b + 0x10*core,
			                          16, &cache->Vmid[core]);
		}
	}

	/* Validate the Calibration Results */
	wlc_phy_table_write_htphy(pi, HTPHY_TBL_ID_IQLOCAL, 1,
	                          IQTBL_CACHE_COOKIE_OFFSET, 16, &tbl_cookie);

	phy_utils_phyreg_exit(pi);

	/* unsuspend mac */
	if (!suspend) {
		wlapi_enable_mac(pi->sh->physhim);
	}

	PHY_CAL(("wl%d: %s: Restored values for chanspec 0x%x are:\n", pi->sh->unit,
	           __FUNCTION__, ctx->chanspec));
	wlc_phy_cal_cache_dbg_htphy((wlc_phy_t *)pi, ctx);
	return BCME_OK;
}
#endif /* PHYCAL_CACHING */

void wlc_phy_rfctrl_override_rxgain_htphy(phy_info_t *pi, uint8 restore,
                                           rxgain_t rxgain[], rxgain_ovrd_t rxgain_ovrd[])
{
	uint8 core;
	uint16 reg_rxgain, reg_lpfgain;

	if (restore == 1) {
		/* restore the stored values */
		FOREACH_CORE(pi, core) {
			phy_utils_write_phyreg(pi, HTPHY_RfctrlOverride(core),
			                       rxgain_ovrd[core].rfctrlovrd);
			phy_utils_write_phyreg(pi, HTPHY_RfctrlRXGAIN(core),
			                       rxgain_ovrd[core].rxgain);
			phy_utils_write_phyreg(pi, HTPHY_Rfctrl_lpf_gain(core),
			                       rxgain_ovrd[core].lpfgain);
			PHY_INFORM(("%s, Restoring RfctrlOverride(rxgain) values\n", __FUNCTION__));
		}
	} else {
		FOREACH_CORE(pi, core) {
			/* Save the original values */
			rxgain_ovrd[core].rfctrlovrd =
			        phy_utils_read_phyreg(pi, HTPHY_RfctrlOverride(core));
			rxgain_ovrd[core].rxgain =
			        phy_utils_read_phyreg(pi, HTPHY_RfctrlRXGAIN(core));
			rxgain_ovrd[core].lpfgain =
			        phy_utils_read_phyreg(pi, HTPHY_Rfctrl_lpf_gain(core));

			/* Write the rxgain override registers */
			phy_utils_write_phyreg(pi, HTPHY_RfctrlRXGAIN(core),
			              (rxgain[core].dvga << 8) | (rxgain[core].mix << 4) |
			              (rxgain[core].lna2 << 2) | rxgain[core].lna1);
			phy_utils_write_phyreg(pi, HTPHY_Rfctrl_lpf_gain(core),
			              (rxgain[core].lpf1 << 4) | rxgain[core].lpf0);
			MOD_PHYREGC(pi, HTPHY_RfctrlOverride, core, rxgain, 1);
			MOD_PHYREGC(pi, HTPHY_RfctrlOverride, core, lpf_gain_biq0, 1);
			MOD_PHYREGC(pi, HTPHY_RfctrlOverride, core, lpf_gain_biq1, 1);

			reg_rxgain = phy_utils_read_phyreg(pi, HTPHY_RfctrlRXGAIN(core));
			reg_lpfgain = phy_utils_read_phyreg(pi, HTPHY_Rfctrl_lpf_gain(core));
			PHY_INFORM(("%s, core %d. rxgain_ovrd = 0x%x, lpf_ovrd = 0x%x\n",
			            __FUNCTION__, core, reg_rxgain, reg_lpfgain));
			BCM_REFERENCE(reg_rxgain);
			BCM_REFERENCE(reg_lpfgain);
		}
	}
}

void
BCMATTACHFN(wlc_phy_interference_mode_attach_htphy)(phy_info_t *pi)
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

static void
wlc_phy_watchdog_htphy(phy_info_t *pi)
{
	phy_info_htphy_t *pi_htphy = pi->u.pi_htphy;
	ASSERT(pi_htphy != NULL);
	BCM_REFERENCE(pi_htphy);

	/* do we really want to look at disable_percal? we have an enable flag,
	 * so isn't this redundant? (nphy does this, but seems weird)
	 */
	if (!pi->disable_percal) {
		if (!(IS_WFD_PHY_LL_ENABLE(pi) && DCS_INPROG_PHY(pi))) {
			/* check to see if a cal needs to be run */
			if ((pi->phy_cal_mode != PHY_PERICAL_DISABLE) &&
				(pi->phy_cal_mode != PHY_PERICAL_MANUAL) &&
				(pi->cal_info->cal_suppress_count == 0) &&
				((pi->sh->now - pi->cal_info->last_cal_time) >=
				pi->sh->glacial_timer)) {
					PHY_CAL(("wlc_phy_watchdog: watchdog fired (en=%d, now=%d,"
					"prev_time=%d, glac=%d)\n",
					pi->phy_cal_mode, pi->sh->now,
					pi->cal_info->last_cal_time,  pi->sh->glacial_timer));

					wlc_phy_cal_perical((wlc_phy_t *)pi, PHY_PERICAL_WATCHDOG);
			}
		}
	}

	/* Read and (implicitly) store current temperature */
	if (pi->sh->now % HTPHY_TEMPSENSE_TIMER == 0)
		wlc_phy_tempsense_htphy(pi);
}

static void
wlc_phy_set_srom_eu_edthresh_htphy(phy_info_t *pi)
{
	int32 eu_edthresh;

	eu_edthresh = CHSPEC_IS2G(pi->radio_chanspec) ? pi->srom_eu_edthresh2g :
	        pi->srom_eu_edthresh5g;
	/* edthresh = 0 & 0xff(-1) are invalid values */
	if (eu_edthresh < -10)
		wlc_phy_adjust_ed_thres_htphy(pi, &eu_edthresh, TRUE);
}

void
wlc_phy_adjust_ed_thres_htphy(phy_info_t *pi, int32 *assert_thresh_dbm, bool set_threshold)
{
	/* Set the EDCRS Assert and De-assert Threshold
	The de-assert threshold is set to 6dB lower then the assert threshold
	Accurate Formula:64*log2(round((10.^((THRESHOLD_dBm +65-30)./10).*50).*(2^9./0.4).^2))
	Simplified Accurate Formula: 64*(THRESHOLD_dBm + 75)/(10*log10(2)) + 832;
	Implemented Approximate Formula: 640000*(THRESHOLD_dBm + 75)/30103 + 832;
	*/
	int32 assert_thres_val, de_assert_thresh_val;

	if (set_threshold == TRUE) {
		assert_thres_val = (640000*(*assert_thresh_dbm + 75) + 25045696)/30103;
		de_assert_thresh_val = (640000*(*assert_thresh_dbm + 69) + 25045696)/30103;

		phy_utils_write_phyreg(pi, HTPHY_ed_crs20LAssertThresh0, (uint16)assert_thres_val);
		phy_utils_write_phyreg(pi, HTPHY_ed_crs20LAssertThresh1, (uint16)assert_thres_val);
		phy_utils_write_phyreg(pi, HTPHY_ed_crs20UAssertThresh0, (uint16)assert_thres_val);
		phy_utils_write_phyreg(pi, HTPHY_ed_crs20UAssertThresh1, (uint16)assert_thres_val);
		phy_utils_write_phyreg(pi, HTPHY_ed_crs20LDeassertThresh0,
		                       (uint16)de_assert_thresh_val);
		phy_utils_write_phyreg(pi, HTPHY_ed_crs20LDeassertThresh1,
		                       (uint16)de_assert_thresh_val);
		phy_utils_write_phyreg(pi, HTPHY_ed_crs20UDeassertThresh0,
		                       (uint16)de_assert_thresh_val);
		phy_utils_write_phyreg(pi, HTPHY_ed_crs20UDeassertThresh1,
		                       (uint16)de_assert_thresh_val);
	} else {
		assert_thres_val = phy_utils_read_phyreg(pi, HTPHY_ed_crs20LAssertThresh0);
		*assert_thresh_dbm = ((((assert_thres_val - 832)*30103)) - 48000000)/640000;
	}
}

#endif /* HTCONF != 0 */
