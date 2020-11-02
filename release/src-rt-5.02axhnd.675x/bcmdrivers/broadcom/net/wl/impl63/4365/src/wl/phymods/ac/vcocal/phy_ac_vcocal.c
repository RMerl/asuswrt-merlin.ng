/*
 * ACPHY VCO CAL module implementation
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
#include <phy_dbg.h>
#include <phy_mem.h>
#include <phy_type_vcocal.h>
#include <phy_ac.h>
#include <phy_ac_vcocal.h>

#include <phy_ac_info.h>
#include <wlc_phy_radio.h>
#include <phy_utils_reg.h>
#include <wlc_radioreg_20691.h>
#include <wlc_radioreg_20693.h>

/* module private states */
struct phy_ac_vcocal_info {
	phy_info_t              *pi;
	phy_ac_info_t		*aci;
	phy_vcocal_info_t	*cmn_info;
	uint8                    pll_mode;
/* add other variable size variables here at the end */
};

/* local functions */
static void wlc_phy_radio20693_vco_opt(phy_info_t *pi);

/* register phy type specific implementation */
phy_ac_vcocal_info_t *
BCMATTACHFN(phy_ac_vcocal_register_impl)(phy_info_t *pi, phy_ac_info_t *aci,
	phy_vcocal_info_t *cmn_info)
{
	phy_ac_vcocal_info_t *ac_info;
	phy_type_vcocal_fns_t fns;

	PHY_CAL(("%s\n", __FUNCTION__));

	/* allocate all storage together */
	if ((ac_info = phy_malloc(pi, sizeof(phy_ac_vcocal_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}

	/* Initialize params */
	ac_info->pi = pi;
	ac_info->aci = aci;
	ac_info->cmn_info = cmn_info;

	ac_info->pll_mode = 0;

	/* register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.ctx = ac_info;

	if (phy_vcocal_register_impl(cmn_info, &fns) != BCME_OK) {
		PHY_ERROR(("%s: phy_vcocal_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	return ac_info;

	/* error handling */
fail:
	if (ac_info != NULL)
		phy_mfree(pi, ac_info, sizeof(phy_ac_vcocal_info_t));
	return NULL;
}

void
BCMATTACHFN(phy_ac_vcocal_unregister_impl)(phy_ac_vcocal_info_t *ac_info)
{
	phy_vcocal_info_t *cmn_info;
	phy_info_t *pi;

	ASSERT(ac_info);
	pi = ac_info->pi;
	cmn_info = ac_info->cmn_info;

	PHY_CAL(("%s\n", __FUNCTION__));

	/* unregister from common */
	phy_vcocal_unregister_impl(cmn_info);

	phy_mfree(pi, ac_info, sizeof(phy_ac_vcocal_info_t));
}

/* ********************************************* */
/*				Internal Definitions					*/
/* ********************************************* */
static void
wlc_phy_radio20693_vco_opt(phy_info_t *pi)
{
	uint8 core;
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;

	if (CHSPEC_IS2G(pi->radio_chanspec) &&
		(pi_ac->lpmode_2g != ACPHY_LPMODE_NONE)) {
		PHY_TRACE(("%s\n", __FUNCTION__));
		FOREACH_CORE(pi, core) {
			MOD_RADIO_REG_20693(pi, PLL_VCO3, core, rfpll_vco_en_alc, 0x1);
			MOD_RADIO_REG_20693(pi, PLL_VCO6, core, rfpll_vco_ALC_ref_ctrl, 0xa);
			MOD_RADIO_REG_20693(pi, PLL_HVLDO2, core, ldo_2p5_lowquiescenten_CP, 0x1);
			MOD_RADIO_REG_20693(pi, PLL_HVLDO2, core, ldo_2p5_lowquiescenten_VCO, 0x1);
			MOD_RADIO_REG_20693(pi, PLL_HVLDO4, core, ldo_2p5_static_load_CP, 0x1);
			MOD_RADIO_REG_20693(pi, PLL_HVLDO4, core, ldo_2p5_static_load_VCO, 0x1);
		}
	}
}

/* ********************************************* */
/*				External Functions					*/
/* ********************************************* */
void
wlc_phy_radio_tiny_vcocal(phy_info_t *pi)
{

	uint8 core;
	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;
	acphy_vcocal_radregs_t *porig = &(pi_ac->ac_vcocal_radioregs_orig);

	ASSERT(TINY_RADIO(pi));

	if (RADIOID(pi->pubpi.radioid) == BCM20693_ID) {
		ASSERT(!porig->is_orig);
		porig->is_orig = TRUE;

		/* Save the radio regs for core 0 */
		porig->clk_div_ovr1 = _READ_RADIO_REG(pi, RADIO_REG_20693(pi, CLK_DIV_OVR1, 0));
		porig->clk_div_cfg1 = _READ_RADIO_REG(pi, RADIO_REG_20693(pi, CLK_DIV_CFG1, 0));

		/* In core0, afeclk_6/12g_xxx_mimo_xxx needs to be off */
		MOD_RADIO_REG_20693(pi, CLK_DIV_OVR1, 0, ovr_afeclkdiv_6g_mimo_pu, 1);
		MOD_RADIO_REG_20693(pi, CLK_DIV_OVR1, 0, ovr_afeclkdiv_12g_mimo_div2_pu, 1);
		MOD_RADIO_REG_20693(pi, CLK_DIV_OVR1, 0, ovr_afeclkdiv_12g_mimo_pu, 1);
		MOD_RADIO_REG_20693(pi, CLK_DIV_CFG1, 0, afe_clk_div_6g_mimo_pu, 0);
		MOD_RADIO_REG_20693(pi, CLK_DIV_CFG1, 0, afe_clk_div_12g_mimo_div2_pu, 0);
		MOD_RADIO_REG_20693(pi, CLK_DIV_CFG1, 0, afe_clk_div_12g_mimo_pu, 0);
	}

	FOREACH_CORE(pi, core) {
		/* cal not required for non-zero cores in MIMO bu required for 80p80 */
		if ((phy_get_phymode(pi) == PHYMODE_MIMO) && (core != 0)) {
			break;
		}
		if (RADIOID(pi->pubpi.radioid) == BCM20691_ID) {
			MOD_RADIO_REG_20691(pi, PLL_HVLDO3, core, ldo_2p5_ldo_VCO_vout_sel, 0xf);
			MOD_RADIO_REG_20691(pi, PLL_HVLDO3, core, ldo_2p5_ldo_CP_vout_sel, 0xf);
		}

		if (RADIOID(pi->pubpi.radioid) == BCM20693_ID) {
			wlc_phy_radio20693_vco_opt(pi);
		}

		/* VCO-Cal startup seq */
		/* VCO cal mode selection */
		/* Use legacy mode */
		MOD_RADIO_REG_TINY(pi, PLL_VCOCAL10, core, rfpll_vcocal_ovr_mode, 0);

		/* # TODO: The below registers have direct PHY control in 20691 (unlike 2069?)
		 * so this reset should ideally be done by writing phy registers
		 */
		/* # Reset delta-sigma modulator */
		MOD_RADIO_REG_TINY(pi, PLL_CFG2, core, rfpll_rst_n, 0);
		/* # Reset VCO cal */
		MOD_RADIO_REG_TINY(pi, PLL_VCOCAL13, core, rfpll_vcocal_rst_n, 0);
		/* # Reset start of VCO Cal */
		MOD_RADIO_REG_TINY(pi, PLL_VCOCAL1, core, rfpll_vcocal_cal, 0);
		if (RADIOID(pi->pubpi.radioid) == BCM20691_ID) {
			/* # Disable PHY direct control for vcocal reset */
			MOD_RADIO_REG_20691(pi, RFPLL_OVR1, core, ovr_rfpll_vcocal_rst_n, 1);
			/* # Disable PHY direct control for delta-sigma modulator reset signal */
			MOD_RADIO_REG_20691(pi, RFPLL_OVR1, core, ovr_rfpll_rst_n, 1);
			/* # Disable PHY direct control for vcocal start */
			MOD_RADIO_REG_20691(pi, RFPLL_OVR1, core, ovr_rfpll_vcocal_cal, 1);
		}
	}
	OSL_DELAY(11);
	FOREACH_CORE(pi, core) {
		/* cal not required for non-zero cores in MIMO but required for 80p80 */
		if ((phy_get_phymode(pi) == PHYMODE_MIMO) && (core != 0)) {
			break;
		}
		/* # Release reset */
		MOD_RADIO_REG_TINY(pi, PLL_CFG2, core, rfpll_rst_n, 1);
		/* # Release reset */
		MOD_RADIO_REG_TINY(pi, PLL_VCOCAL13, core, rfpll_vcocal_rst_n, 1);
	}
	OSL_DELAY(1);
	/* # Start VCO Cal */
	FOREACH_CORE(pi, core) {
		/* cal not required for non-zero cores in MIMO but required for 80p80 */
		if ((phy_get_phymode(pi) == PHYMODE_MIMO) && (core != 0)) {
			break;
		}
		MOD_RADIO_REG_TINY(pi, PLL_VCOCAL1, core, rfpll_vcocal_cal, 1);
	}
}

void
wlc_phy_radio_tiny_afe_resynch(phy_info_t *pi, uint8 mode)
{
	uint8 core;

	FOREACH_CORE(pi, core) {
		MOD_RADIO_REG_20693(pi, CLK_DIV_CFG1, core,
		                    sel_common_pu_rst, 0);
		switch (core) {
			case 0: {
				MOD_RADIO_PLLREG_20693(pi, AFECLK_DIV_CFG1, 0,
					adc_bypass_reset_core0, 1);
				MOD_RADIO_PLLREG_20693(pi, AFECLK_DIV_CFG1, 0,
					dac_bypass_reset_core0, 1);
				}
				break;
			case 1: {
				MOD_RADIO_PLLREG_20693(pi, AFECLK_DIV_CFG1, 0,
					adc_bypass_reset_core1, 1);
				MOD_RADIO_PLLREG_20693(pi, AFECLK_DIV_CFG1, 0,
					dac_bypass_reset_core1, 1);
				}
				break;
			case 2: {
				MOD_RADIO_PLLREG_20693(pi, AFECLK_DIV_CFG1, 0,
					adc_bypass_reset_core2, 1);
				MOD_RADIO_PLLREG_20693(pi, AFECLK_DIV_CFG1, 0,
					dac_bypass_reset_core2, 1);
				}
				break;
			case 3: {
				MOD_RADIO_PLLREG_20693(pi, AFECLK_DIV_CFG1, 0,
					adc_bypass_reset_core3, 1);
				MOD_RADIO_PLLREG_20693(pi, AFECLK_DIV_CFG1, 0,
					dac_bypass_reset_core3, 1);
				}
				break;
		}
	}
}

void
wlc_phy_radio_tiny_vcocal_wave2(phy_info_t *pi, uint8 vcocal_mode, uint8 pll_mode,
                                uint8 coupling_mode, bool cache_calcode)
{
	// # VCO cal supports 15 different modes as following: set by vcocal_mode:
	// ## Mode 0.  Cold Init-4
	// ## Mode 1.  Cold Init-3
	// ## Mode 2.  Cold Norm-4
	// ## Mode 3.  Cold Norm-3
	// ## Mode 4.  Cold Norm-2
	// ## Mode 5.  Cold Norm-1
	// ## Mode 6.  Warm Init-4
	// ## Mode 7.  Warm Init-3
	// ## Mode 8.  Warm Norm-4
	// ## Mode 9.  Warm Norm-3
	// ## Mode 10. Warm Norm-2
	// ## Mode 11. Warm Norm-1
	// ## Mode 12. Warm Fast-2
	// ## Mode 13. Warm Fast-1
	// ## Mode 14. Warm Fast-0

	// # PLL/VCO operating modes: set by pll_mode:
	// ## Mode 0. RFP0 non-coupled: e.g. 4x4 MIMO non-1024QAM
	// ## Mode 1. RFP0 coupled: e.g. 4x4 MIMO 1024QAM
	// ## Mode 2. RFP0 non-coupled + RFP1 non-coupled: 2x2 + 2x2 MIMO non-1024QAM
	// ## Mode 3. RFP0 non-coupled + RFP1 coupled: 3x3 + 1x1 scanning in 80MHz mode
	// ## Mode 4. RFP0 coupled + RFP1 non-coupled: 3x3 MIMO 1024QAM + 1x1 scanning in 20MHz mode
	// ## Mode 5. RFP0 coupled + RFP1 coupled: 3x3 MIMO 1024QAM + 1x1 scanning in 80MHz mode
	// ## Mode 6. RFP1 non-coupled
	// ## Mode 7. RFP1 coupled

	// # When coupled VCO mode is ON, there are two modes: set by coupling_mode
	// ## Mode 0. Use Aux cap to calibrate LC mismatch between VCO1 & VCO2
	// ## Mode 1. Use Aux cap to calibrate K error

	uint8 core, start_core = 0, end_core = 0;
	uint16 rfpll_vcocal_FastSwitch_val[] =
		{3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 1, 0};
	uint16 rfpll_vcocal_slopeInOVR_val[] =
		{1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0};
	uint16 rfpll_vcocal_slopeIn_val[] =
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	uint16 rfpll_vcocal_FourthMesEn_val[] =
		{1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0};
	uint16 rfpll_vcocal_FixMSB_val[] =
		{3, 0, 3, 0, 3, 0, 3, 0, 3, 0, 3, 0, 0, 0, 0};

	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;
	pi_ac->vcocali->pll_mode = pll_mode;

	ASSERT(TINY_RADIO(pi) && RADIOID(pi->pubpi.radioid) == BCM20693_ID);

	PHY_INFORM(("%s: vcocal_mode=%d, pll_mode=%d\n",
			__FUNCTION__, vcocal_mode, pll_mode));

	switch (pll_mode) {
	case 0:
	        // intentional fall through
	case 1:
	        start_core = 0;
		end_core   = 0;
		break;
	case 2:
	        // intentional fall through
	case 3:
	        // intentional fall through
	case 4:
	        // intentional fall through
	case 5:
	        start_core = 0;
		end_core   = 1;
		break;
	case 6:
	        // intentional fall through
	case 7:
	        start_core = 1;
		end_core   = 1;
		break;
	default:
		PHY_ERROR(("wl%d: %s: Unsupported PLL/VCO operating mode %d\n",
			pi->sh->unit, __FUNCTION__, pll_mode));
		ASSERT(0);
		return;
	}

	/* Turn on the VCO-cal buffer */
	if (RADIOID(pi->pubpi.radioid) == BCM20693_ID && RADIOMAJORREV(pi) == 3) {
		MOD_RADIO_PLLREG_20693(pi, WL_XTAL_CFG1, 0, wl_xtal_out_pu, 0x3b);
	}

	for (core = start_core; core <= end_core; core++) {
		uint8 ct;
		// Put all regs/fields write/modification in a array
		uint16 pll_regs_bit_vals[][3] = {
			RADIO_PLLREGC_FLD_20693(pi, PLL_VCOCAL1, core, rfpll_vcocal_FastSwitch,
			rfpll_vcocal_FastSwitch_val[vcocal_mode]),
			RADIO_PLLREGC_FLD_20693(pi, PLL_VCOCAL_OVR1, core, ovr_rfpll_vcocal_slopeIn,
			rfpll_vcocal_slopeInOVR_val[vcocal_mode]),
			RADIO_PLLREGC_FLD_20693(pi, PLL_VCOCAL15, core, rfpll_vcocal_slopeIn,
			rfpll_vcocal_slopeIn_val[vcocal_mode]),
			RADIO_PLLREGC_FLD_20693(pi, PLL_VCOCAL1, core, rfpll_vcocal_FourthMesEn,
			rfpll_vcocal_FourthMesEn_val[vcocal_mode]),
			RADIO_PLLREGC_FLD_20693(pi, PLL_VCOCAL1, core, rfpll_vcocal_FixMSB,
			rfpll_vcocal_FixMSB_val[vcocal_mode]),
		};

		// now write/modification to radio regs
		for (ct = 0; ct < ARRAYSIZE(pll_regs_bit_vals); ct++) {
			phy_utils_mod_radioreg(pi, pll_regs_bit_vals[ct][0],
			pll_regs_bit_vals[ct][1], pll_regs_bit_vals[ct][2]);
		}

		if (READ_RADIO_PLLREGFLD_20693(pi, PLL_VCOCAL24, core,
		                               rfpll_vcocal_enableCoupling)) {
			MOD_RADIO_PLLREG_20693(pi, PLL_VCOCAL24, core,
			                       rfpll_vcocal_couplingMode, coupling_mode);
		}

		if (vcocal_mode <= 5) {
			// Cold Start
			MOD_RADIO_PLLREG_20693(pi, PLL_VCOCAL1, core, rfpll_vcocal_enableCal, 1);
			MOD_RADIO_PLLREG_20693(pi, PLL_CFG2,    core, rfpll_rst_n,            0);
			MOD_RADIO_PLLREG_20693(pi, PLL_VCOCAL1, core, rfpll_vcocal_rst_n,     0);
			OSL_DELAY(10);
			MOD_RADIO_PLLREG_20693(pi, PLL_CFG2,    core, rfpll_rst_n,            1);
			MOD_RADIO_PLLREG_20693(pi, PLL_VCOCAL1, core, rfpll_vcocal_rst_n,     1);
		} else {
			// WARM Start
			MOD_RADIO_PLLREG_20693(pi, PLL_VCOCAL1, core, rfpll_vcocal_rst_n,     1);
			MOD_RADIO_PLLREG_20693(pi, PLL_CFG2,    core, rfpll_rst_n,            0);
			MOD_RADIO_PLLREG_20693(pi, PLL_VCOCAL1, core, rfpll_vcocal_enableCal, 0);
			OSL_DELAY(10);
			MOD_RADIO_PLLREG_20693(pi, PLL_CFG2,    core, rfpll_rst_n,            1);
			MOD_RADIO_PLLREG_20693(pi, PLL_VCOCAL1, core, rfpll_vcocal_enableCal, 1);
		}
	}

	wlc_phy_radio2069x_vcocal_isdone(pi, 0, cache_calcode);

}

void
wlc_phy_radio2069_vcocal(phy_info_t *pi)
{
	/* Use legacy mode */
	uint8 legacy_n = 0;

	ASSERT(RADIOID(pi->pubpi.radioid) == BCM2069_ID);

	/* VCO cal mode selection */
	MOD_RADIO_REG(pi, RFP, PLL_VCOCAL10, rfpll_vcocal_ovr_mode, legacy_n);

	/* VCO-Cal startup seq */
	MOD_RADIO_REG(pi, RFP, PLL_CFG2, rfpll_rst_n, 0);
	MOD_RADIO_REG(pi, RFP, PLL_VCOCAL13, rfpll_vcocal_rst_n, 0);
	MOD_RADIO_REG(pi, RFP, PLL_VCOCAL1, rfpll_vcocal_cal, 0);
	OSL_DELAY(10);
	MOD_RADIO_REG(pi, RFP, PLL_CFG2, rfpll_rst_n, 1);
	MOD_RADIO_REG(pi, RFP, PLL_VCOCAL13, rfpll_vcocal_rst_n, 1);
	OSL_DELAY(1);
	MOD_RADIO_REG(pi, RFP, PLL_VCOCAL1, rfpll_vcocal_cal, 1);
}

#define MAX_2069x_VCOCAL_WAITLOOPS 100

/* vcocal should take < 120 us */
void
wlc_phy_radio2069x_vcocal_isdone(phy_info_t *pi, bool set_delay, bool cache_calcode)
{
	/* Use legacy mode */
	uint8 done, itr, core, start_core = 0, end_core = 0;
	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;
	acphy_vcocal_radregs_t *porig = &(pi_ac->ac_vcocal_radioregs_orig);
	uint8 pll_mode = pi_ac->vcocali->pll_mode;
	uint16 maincap = 0, secondcap = 0, auxcap = 0, base_addr;

	switch (pll_mode) {
	case 0:
	        // intentional fall through
	case 1:
	        start_core = 0;
		end_core   = 0;
		break;
	case 2:
	        // intentional fall through
	case 3:
	        // intentional fall through
	case 4:
	        // intentional fall through
	case 5:
	        start_core = 0;
		end_core   = 1;
		break;
	case 6:
	        // intentional fall through
	case 7:
	        start_core = 1;
		end_core   = 1;
		break;
	default:
		PHY_ERROR(("wl%d: %s: Unsupported PLL/VCO operating mode %d\n",
			pi->sh->unit, __FUNCTION__, pll_mode));
		ASSERT(0);
		return;
	}

	/* Wait for vco_cal to be done, max = 100us * 10 = 1ms  */
	done = 0;
	for (itr = 0; itr < MAX_2069x_VCOCAL_WAITLOOPS; itr++) {
		OSL_DELAY(10);
		if (TINY_RADIO(pi)) {
			if (RADIOID(pi->pubpi.radioid) == BCM20693_ID && RADIOMAJORREV(pi) == 3) {
				done = READ_RADIO_PLLREGFLD_20693(pi, PLL_STATUS1, start_core,
					rfpll_vcocal_done_cal);
				if (end_core != start_core) {
					done &= READ_RADIO_PLLREGFLD_20693(pi, PLL_STATUS1,
						end_core, rfpll_vcocal_done_cal);
				}
			} else {
				done = READ_RADIO_REGFLD_TINY(pi, PLL_VCOCAL14, 0,
					rfpll_vcocal_done_cal);
				/* In 80P80 mode, vcocal should be complete on core 0 and core 1 */
				if (phy_get_phymode(pi) == PHYMODE_80P80)
					done &= READ_RADIO_REGFLD_TINY(pi, PLL_VCOCAL14, 1,
						rfpll_vcocal_done_cal);
			}
		} else
			done = READ_RADIO_REGFLD(pi, RFP, PLL_VCOCAL14, rfpll_vcocal_done_cal);

		if (ISSIM_ENAB(pi->sh->sih))
			done = 1;

		if (done == 1)
			break;
	}

	/* Need to wait extra time after vcocal done bit is high for it to settle */
	if (set_delay == TRUE)
		OSL_DELAY(120);

	ASSERT(done & 0x1);

	/* Restore the radio regs for core 0 */
	if (RADIOID(pi->pubpi.radioid) == BCM20693_ID && RADIOMAJORREV(pi) < 3) {
		ASSERT(porig->is_orig);
		porig->is_orig = FALSE;

		phy_utils_write_radioreg(pi, RADIO_REG_20693(pi, CLK_DIV_OVR1, 0),
			porig->clk_div_ovr1);
		phy_utils_write_radioreg(pi, RADIO_REG_20693(pi, CLK_DIV_CFG1, 0),
			porig->clk_div_cfg1);
	}

	if (RADIOID(pi->pubpi.radioid) == BCM20693_ID && RADIOMAJORREV(pi) == 3) {
		/* Clear Slope In Override */
		MOD_RADIO_PLLREG_20693(pi, PLL_VCOCAL_OVR1, start_core,
		                       ovr_rfpll_vcocal_slopeIn, 0x0);
		if (end_core != start_core) {
			MOD_RADIO_PLLREG_20693(pi, PLL_VCOCAL_OVR1, end_core,
			                       ovr_rfpll_vcocal_slopeIn, 0x0);
		}

		/* store the vco calcode into shmem */
		if (cache_calcode) {
			for (core = start_core; core <= end_core; core++) {
				MOD_RADIO_PLLREG_20693(pi, PLL_VCOCAL7, core,
						rfpll_vcocal_CalCapRBMode, 0x2);
				maincap = READ_RADIO_PLLREGFLD_20693(pi, PLL_STATUS2, 0,
					rfpll_vcocal_calCapRB) | (1 << 12);
				MOD_RADIO_PLLREG_20693(pi, PLL_VCOCAL7, core,
						rfpll_vcocal_CalCapRBMode, 0x3);
				secondcap = READ_RADIO_PLLREGFLD_20693(pi, PLL_STATUS2, 0,
					rfpll_vcocal_calCapRB) | (1 << 12);
				MOD_RADIO_PLLREG_20693(pi, PLL_VCOCAL7, core,
						rfpll_vcocal_CalCapRBMode, 0x4);
				auxcap = READ_RADIO_PLLREGFLD_20693(pi, PLL_STATUS2, 0,
					rfpll_vcocal_calCapRB) | (1 << 12);
			}

			base_addr = wlapi_bmac_read_shm(pi->sh->physhim, M_USEQ_PWRUP_PTR);
			wlapi_bmac_write_shm(pi->sh->physhim,
				2*(base_addr + VCOCAL_MAINCAP_OFFSET_20693), maincap);
			wlapi_bmac_write_shm(pi->sh->physhim,
				2*(base_addr + VCOCAL_SECONDCAP_OFFSET_20693), secondcap);
			wlapi_bmac_write_shm(pi->sh->physhim,
				2*(base_addr + VCOCAL_AUXCAP0_OFFSET_20693), auxcap);
			wlapi_bmac_write_shm(pi->sh->physhim,
				2*(base_addr + VCOCAL_AUXCAP1_OFFSET_20693), auxcap);
		}

		wlc_phy_radio_tiny_afe_resynch(pi, 2);

		/* Restore back */
		MOD_RADIO_PLLREG_20693(pi, WL_XTAL_CFG1, 0, wl_xtal_out_pu, 0x3);
	}

	PHY_INFORM(("wl%d: %s vcocal done\n", pi->sh->unit, __FUNCTION__));
}
