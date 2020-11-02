/*
 * LCN40PHY ANTennaDIVersity module implementation
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

#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmdevs.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include "phy_type_antdiv.h"
#include <phy_lcn40.h>
#include <phy_lcn40_rstr.h>
#include <phy_lcn40_antdiv.h>

#include <wlc_phyreg_lcn40.h>

#include <phy_utils_reg.h>
#include <phy_utils_var.h>

typedef enum {
	SWDIV_SWCTRL_0,   /* Diversity switch controlled via GPIO */
	SWDIV_SWCTRL_1,   /* Diversity switch controlled via LCN40PHY_swctrlOvr_val */
	SWDIV_SWCTRL_2    /* Diversity switch controlled via LCN40PHY_RFOverrideVal0 */
} wlc_swdiv_swctrl_t;

typedef struct {
	bool swdiv_enable;
	uint8 swdiv_gpio_num;
	wlc_swdiv_swctrl_t swdiv_swctrl_en;
	uint16 swdiv_swctrl_mask;
	uint16 swdiv_swctrl_ant0;
	uint16 swdiv_swctrl_ant1;
} phy_lcn40_swdiv_t;

/* module private states */
struct phy_lcn40_antdiv_info {
	phy_info_t *pi;
	phy_lcn40_info_t *lcn40i;
	phy_antdiv_info_t *di;
	phy_lcn40_swdiv_t *swdiv;
};

/* module private states memory layout */
typedef struct {
	phy_lcn40_antdiv_info_t info;
#ifdef WLC_SW_DIVERSITY
	phy_lcn40_swdiv_t swdiv;
#endif // endif
} phy_lcn40_antdiv_mem_t;

/* local functions */
#ifdef WLC_SW_DIVERSITY
static void phy_lcn40_swdiv_init(phy_type_antdiv_ctx_t *ctx);
static uint8 phy_lcn40_swdiv_get_ant(phy_type_antdiv_ctx_t *ctx);
static void phy_lcn40_swdiv_set_ant(phy_type_antdiv_ctx_t *ctx, uint8 ant);
#endif // endif

#ifdef WLC_SW_DIVERSITY
static void phy_lcn40_swdiv_read_srom(phy_lcn40_antdiv_info_t *info);
#endif // endif

/* register phy type specific implementation */
phy_lcn40_antdiv_info_t *
BCMATTACHFN(phy_lcn40_antdiv_register_impl)(phy_info_t *pi, phy_lcn40_info_t *lcn40i,
	phy_antdiv_info_t *di)
{
	phy_lcn40_antdiv_info_t *info;
	phy_type_antdiv_fns_t fns;
#ifdef WLC_SW_DIVERSITY
	phy_lcn40_swdiv_t *swdiv;
#endif // endif

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage together */
	if ((info = phy_malloc(pi, sizeof(phy_lcn40_antdiv_mem_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	bzero(info, sizeof(phy_lcn40_antdiv_mem_t));
	info->pi = pi;
	info->lcn40i = lcn40i;
	info->di = di;

#ifdef WLC_SW_DIVERSITY
	swdiv = &((phy_lcn40_antdiv_mem_t *)info)->swdiv;
	info->swdiv = swdiv;

	phy_lcn40_swdiv_read_srom(info);
#endif /* WLC_SW_DIVERSITY */

	/* register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
#ifdef WLC_SW_DIVERSITY
	fns.init = phy_lcn40_swdiv_init;
	fns.getsw = phy_lcn40_swdiv_get_ant;
	fns.setsw = phy_lcn40_swdiv_set_ant;
#endif // endif
	fns.setrx = (phy_type_antdiv_set_rx_fn_t)phy_lcn40_antdiv_set_rx;
	fns.ctx = info;

	phy_antdiv_register_impl(di, &fns);

	return info;

	/* error handling */
fail:
	return NULL;
}

void
BCMATTACHFN(phy_lcn40_antdiv_unregister_impl)(phy_lcn40_antdiv_info_t *info)
{
	phy_info_t *pi = info->pi;
	phy_antdiv_info_t *di = info->di;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* unregister from common */
	phy_antdiv_unregister_impl(di);

	phy_mfree(pi, info, sizeof(phy_lcn40_antdiv_mem_t));
}

/* Setup */
void
phy_lcn40_antdiv_set_rx(phy_lcn40_antdiv_info_t *info, uint8 ant)
{
#ifdef WLC_SW_DIVERSITY
	phy_lcn40_swdiv_t *swdiv = info->swdiv;
#endif // endif
	phy_info_t *pi = info->pi;

	PHY_TRACE(("%s: ant 0x%x\n", __FUNCTION__, ant));

#ifdef WLC_SW_DIVERSITY
	if (swdiv->swdiv_enable)
		return;
#endif // endif

	if (ant > ANT_RX_DIV_FORCE_1) {
		wlapi_bmac_mhf(pi->sh->physhim, MHF1, MHF1_ANTDIV, MHF1_ANTDIV,
			WLC_BAND_ALL);
		/* and M_PHY_ANTDIV_MASK to 0x200 */
		if (CHIPID(pi->sh->chip) == BCM4314_CHIP_ID ||
		    CHIPID(pi->sh->chip) == BCM43142_CHIP_ID) {
			wlapi_bmac_write_shm(pi->sh->physhim,
				M_PHY_ANTDIV_REG_4314, 0x4b1);
			wlapi_bmac_write_shm(pi->sh->physhim,
				M_PHY_ANTDIV_MASK_4314, 0x200);
		}
	} else
		wlapi_bmac_mhf(pi->sh->physhim, MHF1, MHF1_ANTDIV, 0, WLC_BAND_ALL);

#ifdef WLC_SW_DIVERSITY
	if (swdiv->swdiv_swctrl_en)
		return;
#endif // endif

	if (ant > ANT_RX_DIV_FORCE_1) {
		phy_utils_mod_phyreg(pi, LCN40PHY_crsgainCtrl,
		                     LCN40PHY_crsgainCtrl_DiversityChkEnable_MASK,
		                     0x01 << LCN40PHY_crsgainCtrl_DiversityChkEnable_SHIFT);
		phy_utils_mod_phyreg(pi, LCN40PHY_crsgainCtrl,
		                     LCN40PHY_crsgainCtrl_DefaultAntenna_MASK,
		                     ((ANT_RX_DIV_START_1 == ant) ? 1 : 0) <<
		                     LCN40PHY_crsgainCtrl_DefaultAntenna_SHIFT);

		if (CHIPID(pi->sh->chip) == BCM4314_CHIP_ID ||
		    CHIPID(pi->sh->chip) == BCM43142_CHIP_ID) {
			if (CHSPEC_IS40(pi->radio_chanspec)) {
				PHY_REG_LIST_START
					PHY_REG_WRITE_ENTRY(LCN40PHY, agcControl8, 0x0741)
					PHY_REG_WRITE_ENTRY(LCN40PHY, diversityReg, 0x1f02)
					PHY_REG_MOD_ENTRY(LCN40PHY, GainStableThr_new,
						crsgainstablethr20, 0x3)
					PHY_REG_MOD_ENTRY(LCN40PHY, GainStableThr_new,
						crsgainstablethr40, 0x2)
					PHY_REG_MOD_ENTRY(LCN40PHY, SignalBlockConfigTable_new,
						crssignalblock_tf_20L, 0)
					PHY_REG_MOD_ENTRY(LCN40PHY, SignalBlockConfigTable_new,
						crssignalblock_tf_20U, 0)
					PHY_REG_MOD_ENTRY(LCN40PHY, PwrThresh1_new,
						SlowPwrLoThresh40, 0x03)
					PHY_REG_MOD_ENTRY(LCN40PHY, agcControl11,
						gain_settle_dly_cnt, 0x2)
					PHY_REG_MOD_ENTRY(LCN40PHY, ofdmSyncTimerOffset0_new,
						OFDMPreambleSyncTimeOut20, 0x0)
					PHY_REG_MOD_ENTRY(LCN40PHY, ofdmSyncTimerOffset1_new,
						OFDMPreambleSyncTimeOut20L, 0x0)
					PHY_REG_MOD_ENTRY(LCN40PHY, ofdmSyncTimerOffset1_new,
						OFDMPreambleSyncTimeOut20U, 0x0)
					PHY_REG_MOD_ENTRY(LCN40PHY, agcControl6,
						c_agc_phase_2_5, 0x6)
					PHY_REG_MOD_ENTRY(LCN40PHY, ReducedDetectorDly_new,
						reducedDetectorDlyThresh, 0xfa)
					PHY_REG_MOD_ENTRY(LCN40PHY, agcControl13,
						hg_ofdm_detect_cnt, 0x1)
					PHY_REG_MOD_ENTRY(LCN40PHY, agcControl5, c_ng_ofdm_cfo_dly,
						0x0)
					PHY_REG_MOD_ENTRY(LCN40PHY, CFOblkConfig,
						c_crs_cfo_calc_en_40mhz, 0x0)
				PHY_REG_LIST_EXECUTE(pi);
			} else {
				PHY_REG_LIST_START
					PHY_REG_WRITE_ENTRY(LCN40PHY, agcControl8, 0x0741)
					PHY_REG_WRITE_ENTRY(LCN40PHY, diversityReg, 0x0f02)
					PHY_REG_MOD_ENTRY(LCN40PHY, GainStableThr_new,
						crsgainstablethr20, 0x3)
					PHY_REG_MOD_ENTRY(LCN40PHY, SignalBlockConfigTable_new,
						crssignalblock_tf_20U, 0x2)
					PHY_REG_MOD_ENTRY(LCN40PHY, SignalBlockConfigTable_new,
						crssignalblock_tf_20L, 0x1)
					PHY_REG_MOD_ENTRY(LCN40PHY, PwrThresh1_new,
						SlowPwrLoThresh40, 0x07)
					PHY_REG_MOD_ENTRY(LCN40PHY, agcControl11,
						gain_settle_dly_cnt, 0x2)
					PHY_REG_MOD_ENTRY(LCN40PHY, ofdmSyncTimerOffset0_new,
						OFDMPreambleSyncTimeOut20, 0x0)
					PHY_REG_MOD_ENTRY(LCN40PHY, ofdmSyncTimerOffset1_new,
						OFDMPreambleSyncTimeOut20L, 0x7)
					PHY_REG_MOD_ENTRY(LCN40PHY, ofdmSyncTimerOffset1_new,
						OFDMPreambleSyncTimeOut20U, 0x7)
					PHY_REG_MOD_ENTRY(LCN40PHY, agcControl6, c_agc_phase_2_5,
						0x6)
					PHY_REG_MOD_ENTRY(LCN40PHY, ReducedDetectorDly_new,
						reducedDetectorDlyThresh, 0xfa)
					PHY_REG_MOD_ENTRY(LCN40PHY, agcControl13,
						hg_ofdm_detect_cnt, 0x0)
					PHY_REG_MOD_ENTRY(LCN40PHY, agcControl5, c_ng_ofdm_cfo_dly,
						0x0)
					PHY_REG_MOD_ENTRY(LCN40PHY, CFOblkConfig,
						c_crs_cfo_calc_en_40mhz, 0x0)
				PHY_REG_LIST_EXECUTE(pi);
			}
			if (LCN40REV_LT(pi->pubpi.phy_rev, 4)) {
				PHY_REG_MOD(pi, LCN40PHY, ReducedDetectorDly,
				            reducedDetectorDlyThresh, 0xfa);
				PHY_REG_MOD(pi, LCN40PHY, DetectorDlyAdjust,
				            ofdmfiltDlyAdjustment, -1);
			}
			/* Enable lcn40phy antdiv WAR in ucode */
			wlapi_bmac_mhf(pi->sh->physhim, MHF5, MHF5_LCN40PHY_ANTDIV_WAR,
			               MHF5_LCN40PHY_ANTDIV_WAR, WLC_BAND_2G);
		}
	} else {
		phy_utils_mod_phyreg(pi, LCN40PHY_crsgainCtrl,
		                     LCN40PHY_crsgainCtrl_DiversityChkEnable_MASK,
		                     0x00 << LCN40PHY_crsgainCtrl_DiversityChkEnable_SHIFT);
		phy_utils_mod_phyreg(pi, LCN40PHY_crsgainCtrl,
		                     LCN40PHY_crsgainCtrl_DefaultAntenna_MASK,
		                     (uint16)ant << LCN40PHY_crsgainCtrl_DefaultAntenna_SHIFT);

		if (CHIPID(pi->sh->chip) == BCM4314_CHIP_ID ||
		    CHIPID(pi->sh->chip) == BCM43142_CHIP_ID) {
			PHY_REG_LIST_START
				PHY_REG_WRITE_ENTRY(LCN40PHY, agcControl8, 0x0750)
				PHY_REG_WRITE_ENTRY(LCN40PHY, diversityReg, 0x0802)
				PHY_REG_MOD_ENTRY(LCN40PHY, GainStableThr_new,
					crsgainstablethr20, 0x1)
				PHY_REG_MOD_ENTRY(LCN40PHY, SignalBlockConfigTable_new,
					crssignalblock_tf_20U, 0x2)
				PHY_REG_MOD_ENTRY(LCN40PHY, SignalBlockConfigTable_new,
					crssignalblock_tf_20L, 0x2)
				PHY_REG_MOD_ENTRY(LCN40PHY, PwrThresh1_new, SlowPwrLoThresh40,
					0x07)
			PHY_REG_LIST_EXECUTE(pi);
			if (CHSPEC_IS40(pi->radio_chanspec)) {
				PHY_REG_MOD(pi, LCN40PHY, agcControl11, gain_settle_dly_cnt, 0x3);
			} else {
				PHY_REG_MOD(pi, LCN40PHY, agcControl11, gain_settle_dly_cnt, 0x5);
			}
			PHY_REG_LIST_START
				PHY_REG_MOD_ENTRY(LCN40PHY, ofdmSyncTimerOffset0_new,
					OFDMPreambleSyncTimeOut20, 0x7)
				PHY_REG_MOD_ENTRY(LCN40PHY, ofdmSyncTimerOffset1_new,
					OFDMPreambleSyncTimeOut20L, 0x7)
				PHY_REG_MOD_ENTRY(LCN40PHY, ofdmSyncTimerOffset1_new,
					OFDMPreambleSyncTimeOut20U, 0x7)
				PHY_REG_MOD_ENTRY(LCN40PHY, agcControl6, c_agc_phase_2_5, 0x4)
			PHY_REG_LIST_EXECUTE(pi);
			if (LCN40REV_LT(pi->pubpi.phy_rev, 4)) {
				PHY_REG_MOD(pi, LCN40PHY, ReducedDetectorDly,
					reducedDetectorDlyThresh, 0xfa);
			}
			PHY_REG_LIST_START
				PHY_REG_MOD_ENTRY(LCN40PHY, ReducedDetectorDly_new,
					reducedDetectorDlyThresh, 0xfa)
				PHY_REG_MOD_ENTRY(LCN40PHY, agcControl13, hg_ofdm_detect_cnt, 0x1)
				PHY_REG_MOD_ENTRY(LCN40PHY, agcControl5, c_ng_ofdm_cfo_dly, 0x1)
			PHY_REG_LIST_EXECUTE(pi);
			if (LCN40REV_LT(pi->pubpi.phy_rev, 4)) {
				PHY_REG_MOD(pi, LCN40PHY, DetectorDlyAdjust,
					ofdmfiltDlyAdjustment, 0);
			}
			PHY_REG_MOD(pi, LCN40PHY, CFOblkConfig, c_crs_cfo_calc_en_40mhz, 0x1);
			/* Disable lcn40phy antdiv WAR in ucode */
			wlapi_bmac_mhf(pi->sh->physhim, MHF5, MHF5_LCN40PHY_ANTDIV_WAR,
			        0, WLC_BAND_2G);
		}
	}

	if ((CHIPID(pi->sh->chip) == BCM4314_CHIP_ID) ||
	    (CHIPID(pi->sh->chip) == BCM43142_CHIP_ID)) {
		if ((ANT_RX_DIV_START_1 == ant) || (ANT_RX_DIV_FORCE_1 == ant)) {
			/* Ant1 is chosen */
			PHY_REG_MOD(pi, LCN40PHY, rfoverride2, ext_lna_gain_ovr, 0x1);
			PHY_REG_MOD(pi, LCN40PHY, rfoverride2val, ext_lna_gain_ovr_val, 0x1);
		} else {
			/* Ant0 is chosen */
			PHY_REG_MOD(pi, LCN40PHY, rfoverride2, ext_lna_gain_ovr, 0x1);
			PHY_REG_MOD(pi, LCN40PHY, rfoverride2val, ext_lna_gain_ovr_val,	0x0);
		}
	}
}

#ifdef WLC_SW_DIVERSITY
static void
BCMATTACHFN(phy_lcn40_swdiv_read_srom)(phy_lcn40_antdiv_info_t *info)
{
	phy_lcn40_swdiv_t *swdiv = info->swdiv;
	phy_info_t *pi = info->pi;

	/* Note: following swdiv parameters are also read and stored independently in wlc.c */
	/*       (not intended to be used as dynamic parameters) */
	swdiv->swdiv_enable      = (bool)PHY_GETINTVAR(pi, rstr_swdiv_en);
	swdiv->swdiv_gpio_num    = (uint8)PHY_GETINTVAR(pi, rstr_swdiv_gpio);
	swdiv->swdiv_swctrl_en   = (wlc_swdiv_swctrl_t)PHY_GETINTVAR(pi, rstr_swdiv_swctrl_en);
	swdiv->swdiv_swctrl_mask = (uint16)PHY_GETINTVAR(pi, rstr_swdiv_swctrl_mask);
	swdiv->swdiv_swctrl_ant0 = (uint16)PHY_GETINTVAR(pi, rstr_swdiv_swctrl_ant0);
	swdiv->swdiv_swctrl_ant1 = (uint16)PHY_GETINTVAR(pi, rstr_swdiv_swctrl_ant1);
}

#define M_SWDIV_SWCTRL_REG 0

static void
phy_lcn40_swdiv_init(phy_type_antdiv_ctx_t *ctx)
{
	phy_lcn40_antdiv_info_t *info = (phy_lcn40_antdiv_info_t *)ctx;
	phy_lcn40_swdiv_t *swdiv = info->swdiv;
	phy_info_t *pi = info->pi;
	bool suspend;
	uint16 lcn40phyregs_shm_addr;

	if (!swdiv->swdiv_enable)
		return;

	phy_lcn40_swdiv_set_ant(ctx, 0);

	if (swdiv->swdiv_swctrl_en == SWDIV_SWCTRL_0)
		return;

	suspend = !(R_REG(pi->sh->osh, &pi->regs->maccontrol) & MCTL_EN_MAC);
	if (!suspend)
		wlapi_suspend_mac_and_wait(pi->sh->physhim);

	lcn40phyregs_shm_addr =
		2 * wlapi_bmac_read_shm(pi->sh->physhim, M_LCN40PHYREGS_PTR);
	switch (swdiv->swdiv_swctrl_en) {
	case SWDIV_SWCTRL_1:
		phy_utils_or_phyreg(pi, LCN40PHY_sw_ctrl_config,
		                    swdiv->swdiv_swctrl_mask <<
		                    LCN40PHY_sw_ctrl_config_sw_ctrl_mask_SHIFT);
		phy_utils_or_phyreg(pi, LCN40PHY_swctrlOvr,
		                    swdiv->swdiv_swctrl_mask <<
		                    LCN40PHY_swctrlOvr_swCtrl_ovr_SHIFT);
		wlapi_bmac_write_shm(pi->sh->physhim,
		                     lcn40phyregs_shm_addr + M_SWDIV_SWCTRL_REG,
		                     LCN40PHY_swctrlOvr_val);
		break;
	case SWDIV_SWCTRL_2:
		phy_utils_or_phyreg(pi, LCN40PHY_sw_ctrl_config,
		                    swdiv->swdiv_swctrl_mask <<
		                    LCN40PHY_sw_ctrl_config_sw_ctrl_mask_SHIFT);
		phy_utils_or_phyreg(pi, LCN40PHY_RFOverride0, 1 << swdiv->swdiv_gpio_num);
		wlapi_bmac_write_shm(pi->sh->physhim,
		                     lcn40phyregs_shm_addr + M_SWDIV_SWCTRL_REG,
		                     LCN40PHY_RFOverrideVal0);
		break;
	case SWDIV_SWCTRL_0:
	default:
		ASSERT(0);
	}

	if (!suspend)
		wlapi_enable_mac(pi->sh->physhim);
}

static void
phy_lcn40_swdiv_set_ant(phy_type_antdiv_ctx_t *ctx, uint8 new_ant)
{
	phy_lcn40_antdiv_info_t *info = (phy_lcn40_antdiv_info_t *)ctx;
	phy_lcn40_swdiv_t *swdiv = info->swdiv;
	phy_info_t *pi = info->pi;
	bool suspend;
	uint16 mask, val;

	suspend = !(R_REG(pi->sh->osh, &pi->regs->maccontrol) & MCTL_EN_MAC);
	switch (swdiv->swdiv_swctrl_en) {
	case SWDIV_SWCTRL_0:
		val = 1 << swdiv->swdiv_gpio_num;
		if (new_ant == 1)
			OR_REG(pi->sh->osh, &pi->regs->psm_gpio_out, val);
		else
			AND_REG(pi->sh->osh, &pi->regs->psm_gpio_out, ~val);
		break;
	case SWDIV_SWCTRL_1:
		val = (new_ant == 1) ?
			swdiv->swdiv_swctrl_ant1 : swdiv->swdiv_swctrl_ant0;
		mask = swdiv->swdiv_swctrl_mask;
		if (!suspend)
			wlapi_suspend_mac_and_wait(pi->sh->physhim);
		phy_utils_mod_phyreg(pi, LCN40PHY_swctrlOvr_val,
		                     mask << LCN40PHY_sw_ctrl_config_sw_ctrl_mask_SHIFT,
		                     val  << LCN40PHY_sw_ctrl_config_sw_ctrl_mask_SHIFT);
		if (!suspend)
			wlapi_enable_mac(pi->sh->physhim);
		break;
	case SWDIV_SWCTRL_2:
		val = 1 << swdiv->swdiv_gpio_num;
		if (!suspend)
			wlapi_suspend_mac_and_wait(pi->sh->physhim);
		if (new_ant == 1)
			PHY_REG_OR(pi, LCN40PHY, RFOverrideVal0, val);
		else
			PHY_REG_AND(pi, LCN40PHY, RFOverrideVal0, ~val);
		if (!suspend)
			wlapi_enable_mac(pi->sh->physhim);
		break;
	default:
		ASSERT(0);
	}
}

static uint8
phy_lcn40_swdiv_get_ant(phy_type_antdiv_ctx_t *ctx)
{
	phy_lcn40_antdiv_info_t *info = (phy_lcn40_antdiv_info_t *)ctx;
	phy_lcn40_swdiv_t *swdiv = info->swdiv;
	phy_info_t *pi = info->pi;
	uint16 val;
	uint16 mask;
	bool suspend;

	suspend = !(R_REG(pi->sh->osh, &pi->regs->maccontrol) & MCTL_EN_MAC);
	switch (swdiv->swdiv_swctrl_en) {
	case SWDIV_SWCTRL_0:
		val = R_REG(pi->sh->osh, &pi->regs->psm_gpio_out);
		mask = 1 << swdiv->swdiv_gpio_num;
		return (val & mask) ? 1:0;
	case SWDIV_SWCTRL_1:
		if (!suspend)
			wlapi_suspend_mac_and_wait(pi->sh->physhim);
		val = PHY_REG_READ(pi, LCN40PHY, swctrlOvr_val, swCtrl_ovr_val);
		if (!suspend)
			wlapi_enable_mac(pi->sh->physhim);
		mask = swdiv->swdiv_swctrl_mask;
		return ((val & mask) == swdiv->swdiv_swctrl_ant1) ? 1:0;
	case SWDIV_SWCTRL_2:
		if (!suspend)
			wlapi_suspend_mac_and_wait(pi->sh->physhim);
		val = phy_utils_read_phyreg(pi, LCN40PHY_RFOverrideVal0);
		if (!suspend)
			wlapi_enable_mac(pi->sh->physhim);
		mask = 1 << swdiv->swdiv_gpio_num;
		return (val & mask) ? 1:0;
	default:
		ASSERT(0);
		return 0;
	}
}

void
phy_lcn40_swdiv_epa_pd(phy_lcn40_antdiv_info_t *di, bool disable)
{
	phy_lcn40_swdiv_t *swdiv = di->swdiv;
	phy_info_t *pi = di->pi;
	uint16 swctrlovr_mask = 0;
	uint16 swctrlovr_val = 0;

	swctrlovr_mask = swdiv->swdiv_swctrl_mask;
	swctrlovr_val = PHY_REG_READ(pi, LCN40PHY, swctrlOvr_val, swCtrl_ovr_val);
	swctrlovr_val = swctrlovr_val & swctrlovr_mask;

	if (!disable) {
		PHY_REG_MOD(pi, LCN40PHY, swctrlOvr, swCtrl_ovr, swctrlovr_mask);
	} else {
		PHY_REG_MOD(pi, LCN40PHY, swctrlOvr_val, swCtrl_ovr_val, swctrlovr_val);
		PHY_REG_MOD(pi, LCN40PHY, swctrlOvr, swCtrl_ovr, (0xff | swctrlovr_mask));
	}
}
#endif /* WLC_SW_DIVERSITY */
