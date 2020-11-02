/*
 * ACPHY ANAcore contorl module implementation
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
#include <phy_ana.h>
#include "phy_type_ana.h"
#include <phy_ac_ana.h>

#include <wlc_phyreg_ac.h>
#include <phy_utils_reg.h>
#include <phy_ac_info.h>
#include <wlc_radioreg_20691.h>
#include <wlc_radioreg_20693.h>
#include <sbchipc.h>

/* module private states */
struct phy_ac_ana_info {
	phy_info_t *pi;
	phy_ac_info_t *aci;
	phy_ana_info_t *ani;
};

/* local functions */
static int phy_ac_ana_switch(phy_type_ana_ctx_t *ctx, bool on);
static void phy_ac_ana_reset(phy_type_ana_ctx_t *ctx);

/* Register/unregister ACPHY specific implementation to common layer. */
phy_ac_ana_info_t *
BCMATTACHFN(phy_ac_ana_register_impl)(phy_info_t *pi, phy_ac_info_t *aci,
	phy_ana_info_t *ani)
{
	phy_ac_ana_info_t *info;
	phy_type_ana_fns_t fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage in once */
	if ((info = phy_malloc(pi, sizeof(phy_ac_ana_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}

	info->pi = pi;
	info->aci = aci;
	info->ani = ani;

#ifndef BCM_OL_DEV
	phy_ac_ana_switch(info, ON);
#endif // endif

	/* Register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.ctrl = phy_ac_ana_switch;
	fns.reset = phy_ac_ana_reset;
	fns.ctx = info;

	phy_ana_register_impl(ani, &fns);

	return info;
fail:
	if (info != NULL)
		phy_mfree(pi, info, sizeof(phy_ac_ana_info_t));
	return NULL;
}

void
BCMATTACHFN(phy_ac_ana_unregister_impl)(phy_ac_ana_info_t *info)
{
	phy_info_t *pi;
	phy_ana_info_t *ani;

	ASSERT(info);
	pi = info->pi;
	ani = info->ani;

	PHY_TRACE(("%s\n", __FUNCTION__));

	phy_ana_unregister_impl(ani);

	phy_mfree(pi, info, sizeof(phy_ac_ana_info_t));
}

/* switch anacore on/off */
static int
phy_ac_ana_switch(phy_type_ana_ctx_t *ctx, bool on)
{
	phy_ac_ana_info_t *info = (phy_ac_ana_info_t *)ctx;
	phy_info_t *pi = info->pi;

	PHY_TRACE(("%s: %d\n", __FUNCTION__, on));

	if (on)
		W_REG(pi->sh->osh, &pi->regs->phyanacore, 0x0);
	else
		W_REG(pi->sh->osh, &pi->regs->phyanacore, 0xF4);

	return BCME_OK;
}

/* reset h/w */
static void
phy_ac_ana_reset(phy_type_ana_ctx_t *ctx)
{
	phy_ac_ana_switch(ctx, ON);
}

/* ********************************************* */
/*				External Functions					*/
/* ********************************************* */
uint16 sdadc_cfg20 = 0xd5eb;
uint16 sdadc_cfg40 = 0x45ea;

/* High SNR 40M */
uint16 sdadc_cfg40hs = 0x43e9;
uint16 sdadc_cfg80 = 0x07f8;

void
wlc_tiny_dc_static_WAR(phy_info_t *pi)
{
	uint8 core;

	FOREACH_CORE(pi, core) {
		if (CHSPEC_IS80(pi->radio_chanspec) ||
				PHY_AS_80P80(pi, pi->radio_chanspec)) {
			MOD_RADIO_REG_TINY(pi, ADC_OVR1, core, ovr_adc_clk_slow_pu, 0);
			MOD_RADIO_REG_TINY(pi, ADC_OVR1, core, ovr_adc_sipo_pu, 0);

			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				if (RADIOID(pi->pubpi.radioid) == BCM20693_ID) {
					MOD_RADIO_REG_20693(pi, RX_TOP_2G_OVR_EAST2, core,
						ovr_rxmix2g_pu, 0);
				} else {
					MOD_RADIO_REG_20691(pi, RX_TOP_2G_OVR_NORTH, core,
						ovr_rxmix2g_pu, 0);
				}
			}
		} else if (CHSPEC_IS160(pi->radio_chanspec)) {
			//FIXME
		} else {
			MOD_RADIO_REG_TINY(pi, ADC_CFG15, core, adc_clk_slow_pu, 1);
			MOD_RADIO_REG_TINY(pi, ADC_OVR1, core, ovr_adc_clk_slow_pu, 1);
			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				MOD_RADIO_REG_TINY(pi, RXMIX2G_CFG1, core, rxmix2g_pu, 1);
				if (RADIOID(pi->pubpi.radioid) == BCM20693_ID) {
					MOD_RADIO_REG_20693(pi, RX_TOP_2G_OVR_EAST2, core,
						ovr_rxmix2g_pu, 1);
				} else {
					MOD_RADIO_REG_20691(pi, RX_TOP_2G_OVR_NORTH, core,
						ovr_rxmix2g_pu, 1);
				}
				MOD_RADIO_REG_TINY(pi, ADC_CFG1, core, adc_sipo_pu, 1);
				MOD_RADIO_REG_TINY(pi, ADC_OVR1, core, ovr_adc_sipo_pu, 1);
			}
		}
	}

	wlc_phy_force_rfseq_acphy(pi, ACPHY_RFSEQ_RX2TX);
	wlc_phy_force_rfseq_acphy(pi, ACPHY_RFSEQ_TX2RX);
}

void
wlc_phy_init_adc_read(phy_info_t* pi, uint16* save_afePuCtrl, uint16* save_gpio,
                      uint32* save_chipc, uint16* fval2g_orig, uint16* fval5g_orig,
                      uint16* fval2g, uint16* fval5g, uint8* stall_val,
                      uint16* save_gpioHiOutEn)
{
	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;
	*stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
	*save_afePuCtrl = READ_PHYREGFLD(pi, AfePuCtrl, tssiSleepEn);
	MOD_PHYREG(pi, AfePuCtrl, tssiSleepEn, 0);
	*save_gpio = READ_PHYREG(pi, gpioSel);
	*save_gpioHiOutEn = READ_PHYREG(pi, gpioHiOutEn);

	if (pi_ac->poll_adc_WAR) {
		ACPHY_DISABLE_STALL(pi);

		MOD_PHYREGCE(pi, RfctrlIntc, 1, ext_2g_papu, 0);
		MOD_PHYREGCE(pi, RfctrlIntc, 1, ext_5g_papu, 0);
		MOD_PHYREGCE(pi, RfctrlIntc, 1, override_ext_pa, 1);

		*save_chipc = si_corereg(pi->sh->sih, SI_CC_IDX,
		                         OFFSETOF(chipcregs_t, chipcontrol), 0, 0);
		si_corereg(pi->sh->sih, SI_CC_IDX, OFFSETOF(chipcregs_t, chipcontrol),
		           0xffffffff, CCTRL4360_EXTRA_FEMCTRL_MODE);

		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_FEMCTRLLUT, 1, 41, 16, fval2g_orig);
		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_FEMCTRLLUT, 1, 57, 16, fval5g_orig);

		*fval2g = (*fval2g_orig & 0xf0) << 1;
		*fval5g = (*fval5g_orig & 0xf);

		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_FEMCTRLLUT, 1, 41, 16, fval2g);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_FEMCTRLLUT, 1, 57, 16, fval5g);

		MOD_PHYREGCE(pi, RfctrlIntc, 1, ext_2g_papu, 0);
		MOD_PHYREGCE(pi, RfctrlIntc, 1, ext_5g_papu, 0);
		MOD_PHYREGCE(pi, RfctrlIntc, 1, override_ext_pa, 0);

		ACPHY_ENABLE_STALL(pi, *stall_val);
	}
}

void
wlc_phy_restore_after_adc_read(phy_info_t *pi, uint16* save_afePuCtrl, uint16 *save_gpio,
                               uint32* save_chipc, uint16* fval2g_orig, uint16* fval5g_orig,
                               uint16* fval2g, uint16* fval5g, uint8* stall_val,
                               uint16* save_gpioHiOutEn)
{
	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;
	ACPHY_DISABLE_STALL(pi);
	WRITE_PHYREG(pi, gpioSel, *save_gpio);
	WRITE_PHYREG(pi, gpioHiOutEn, *save_gpioHiOutEn);
	if (pi_ac->poll_adc_WAR) {
		MOD_PHYREGCE(pi, RfctrlIntc, 1, ext_2g_papu, 0);
		MOD_PHYREGCE(pi, RfctrlIntc, 1, ext_5g_papu, 0);
		MOD_PHYREGCE(pi, RfctrlIntc, 1, override_ext_pa, 1);

		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_FEMCTRLLUT, 1, 41, 16, fval2g_orig);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_FEMCTRLLUT, 1, 57, 16, fval5g_orig);

		si_corereg(pi->sh->sih, SI_CC_IDX, OFFSETOF(chipcregs_t, chipcontrol),
		           0xffffffff, *save_chipc);

		MOD_PHYREGCE(pi, RfctrlIntc, 1, ext_2g_papu, 0);
		MOD_PHYREGCE(pi, RfctrlIntc, 1, ext_5g_papu, 0);
		MOD_PHYREGCE(pi, RfctrlIntc, 1, override_ext_pa, 0);
	}

	MOD_PHYREG(pi, AfePuCtrl, tssiSleepEn, *save_afePuCtrl);

	ACPHY_ENABLE_STALL(pi, *stall_val);
}

void
wlc_phy_pulse_adc_reset_acphy(phy_info_t *pi)
{
	uint8 core;

	struct _reg_vals {
		uint16 regval;
		uint16 regaddr;
	} cur_reg_val[PHY_CORE_MAX*3]; /* save 3 register values for each core */

	uint core_count = 0;

	/* Set clamp using rfctrl override */
	FOREACH_CORE(pi, core) {
		cur_reg_val[core_count].regval = READ_PHYREGCE(pi, RfctrlCoreAfeCfg1, core);
		cur_reg_val[core_count].regaddr = ACPHYREGCE(pi, RfctrlCoreAfeCfg1, core);
		WRITE_PHYREGCE(pi, RfctrlCoreAfeCfg1, core, cur_reg_val[core_count].regval |
			ACPHY_RfctrlCoreAfeCfg10_afe_iqadc_reset_MASK(pi->pubpi.phy_rev));
		++core_count;

		cur_reg_val[core_count].regval = READ_PHYREGCE(pi, RfctrlCoreAfeCfg2, core);
		cur_reg_val[core_count].regaddr = ACPHYREGCE(pi, RfctrlCoreAfeCfg2, core);
		WRITE_PHYREGCE(pi, RfctrlCoreAfeCfg2, core, cur_reg_val[core_count].regval |
		        ACPHY_RfctrlCoreAfeCfg20_afe_iqadc_clamp_en_MASK(pi->pubpi.phy_rev));
		++core_count;

		cur_reg_val[core_count].regval = READ_PHYREGCE(pi, RfctrlOverrideAfeCfg, core);
		cur_reg_val[core_count].regaddr = ACPHYREGCE(pi, RfctrlOverrideAfeCfg, core);
		WRITE_PHYREGCE(pi, RfctrlOverrideAfeCfg, core, cur_reg_val[core_count].regval |
			ACPHY_RfctrlOverrideAfeCfg0_afe_iqadc_clamp_en_MASK(pi->pubpi.phy_rev) |
			ACPHY_RfctrlOverrideAfeCfg0_afe_iqadc_reset_MASK(pi->pubpi.phy_rev));
		++core_count;
	}

	/* Wait for 1 us */
	OSL_DELAY(1);

	/* Restore values */
	while (core_count > 0) {
		--core_count;
		phy_utils_write_phyreg(pi, cur_reg_val[core_count].regaddr,
		                       cur_reg_val[core_count].regval);
	}
	/* Wait for 1 us */
	OSL_DELAY(1);
}
