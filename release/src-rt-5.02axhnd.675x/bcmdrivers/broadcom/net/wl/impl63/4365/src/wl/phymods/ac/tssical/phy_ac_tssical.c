/*
 * ACPHY TSSI Cal module implementation
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
#include "phy_type_tssical.h"
#include <phy_ac.h>
#include <phy_ac_tssical.h>

/* ************************ */
/* Modules used by this module */
/* ************************ */
#include <wlc_radioreg_20691.h>
#include <wlc_radioreg_20693.h>
#include <wlc_phy_radio.h>
#include <wlc_phyreg_ac.h>

#include <phy_utils_reg.h>
#include <phy_utils_channel.h>
#include <phy_utils_math.h>
#include <phy_ac_info.h>

/* module private states */
struct phy_ac_tssical_info {
	phy_info_t *pi;
	phy_ac_info_t *aci;
	phy_tssical_info_t *cmn_info;
};

/* local functions */

/* register phy type specific implementation */
phy_ac_tssical_info_t *
BCMATTACHFN(phy_ac_tssical_register_impl)(phy_info_t *pi, phy_ac_info_t *aci,
	phy_tssical_info_t *cmn_info)
{
	phy_ac_tssical_info_t *ac_info;
	phy_type_tssical_fns_t fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage together */
	if ((ac_info = phy_malloc(pi, sizeof(phy_ac_tssical_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	ac_info->pi = pi;
	ac_info->aci = aci;
	ac_info->cmn_info = cmn_info;

	/* register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.ctx = ac_info;

	if (phy_tssical_register_impl(cmn_info, &fns) != BCME_OK) {
		PHY_ERROR(("%s: phy_tssical_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	return ac_info;

	/* error handling */
fail:
	if (ac_info != NULL)
		phy_mfree(pi, ac_info, sizeof(phy_ac_tssical_info_t));
	return NULL;
}

void
BCMATTACHFN(phy_ac_tssical_unregister_impl)(phy_ac_tssical_info_t *ac_info)
{
	phy_info_t *pi = ac_info->pi;
	phy_tssical_info_t *cmn_info = ac_info->cmn_info;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* unregister from common */
	phy_tssical_unregister_impl(cmn_info);

	phy_mfree(pi, ac_info, sizeof(phy_ac_tssical_info_t));
}

/* ************************* */
/*		Internal Functions		*/
/* ************************* */
#ifdef NOT_YET
static void wlc_phy_scanroam_tssical_cal_acphy(phy_info_t *pi, bool set)
#endif // endif

#ifdef NOT_YET
static void
wlc_phy_scanroam_tssical_cal_acphy(phy_info_t *pi, bool set)
{
	uint16 ab_int[2];
	uint8 core;

	ASSERT(RADIOID(pi->pubpi.radioid) == BCM2069_ID);

	/* Prepare Mac and Phregs */
	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	phy_utils_phyreg_enter(pi);

	PHY_TRACE(("wl%d: %s: in scan/roam set %d\n", pi->sh->unit, __FUNCTION__, set));

	if (set) {
		PHY_CAL(("wl%d: %s: save the txcal for scan/roam\n",
			pi->sh->unit, __FUNCTION__));
		/* save the txcal to tssical */
		FOREACH_ACTV_CORE(pi, pi->sh->phyrxchain, core) {
			wlc_phy_cal_txiqlo_coeffs_acphy(pi, CAL_COEFF_READ,
				ab_int, TB_OFDM_COEFFS_AB, core);
			pi->u.pi_acphy->txcal_tssical[core].txa = ab_int[0];
			pi->u.pi_acphy->txcal_tssical[core].txb = ab_int[1];
			wlc_phy_cal_txiqlo_coeffs_acphy(pi, CAL_COEFF_READ,
			&pi->u.pi_acphy->txcal_tssical[core].txd,
				TB_OFDM_COEFFS_D, core);
			pi->u.pi_acphy->txcal_tssical[core].txei =
				(uint8)READ_RADIO_REGC(pi, RF, TXGM_LOFT_FINE_I, core);
			pi->u.pi_acphy->txcal_tssical[core].txeq =
				(uint8)READ_RADIO_REGC(pi, RF, TXGM_LOFT_FINE_Q, core);
			pi->u.pi_acphy->txcal_tssical[core].txfi =
				(uint8)READ_RADIO_REGC(pi, RF, TXGM_LOFT_COARSE_I, core);
			pi->u.pi_acphy->txcal_tssical[core].txfq =
				(uint8)READ_RADIO_REGC(pi, RF, TXGM_LOFT_COARSE_Q, core);
			pi->u.pi_acphy->txcal_tssical[core].rxa =
				READ_PHYREGCE(pi, Core1RxIQCompA, core);
			pi->u.pi_acphy->txcal_tssical[core].rxb =
				READ_PHYREGCE(pi, Core1RxIQCompB, core);
			}

		/* mark the tssical as valid */
		pi->u.pi_acphy->txcal_tssical_cookie = TXCAL_tssical_VALID;
	} else {
		if (pi->u.pi_acphy->txcal_tssical_cookie == TXCAL_tssical_VALID) {
			PHY_CAL(("wl%d: %s: restore the txcal after scan/roam\n",
				pi->sh->unit, __FUNCTION__));
			/* restore the txcal from tssical */
			wlc_phy_cal_coeffs_upd(pi, pi->u.pi_acphy->txcal_tssical);
		}
	}

	phy_utils_phyreg_exit(pi);
	wlapi_enable_mac(pi->sh->physhim);
}
#endif /* NOT_YET */

/* ********************************************* */
/*				External Definitions					*/
/* ********************************************* */
/* measure idle TSSI by sending 0-magnitude tone */
void
wlc_phy_txpwrctrl_idle_tssi_meas_acphy(phy_info_t *pi)
{
	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;
	uint8  core;
	int16  idle_tssi[PHY_CORE_MAX] = {0};
	uint16 orig_RfseqCoreActv2059, orig_RxSdFeConfig6 = 0;
	uint16 hwaci_state = 0;

	if (SCAN_RM_IN_PROGRESS(pi) || PLT_INPROG_PHY(pi) || PHY_MUTED(pi))
		/* skip idle tssi cal */
		return;
	/* prevent crs trigger */
	wlc_phy_stay_in_carriersearch_acphy(pi, TRUE);

	if (ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev)) {
		/* Disable hwaci mitigation */
		hwaci_state = READ_PHYREG(pi, ACI_Mitigation_CTRL);
		MOD_PHYREG(pi, ACI_Mitigation_CTRL, aci_mitigation_hwtrig_disable, 1);
		MOD_PHYREG(pi, ACI_Mitigation_CTRL, aci_mitigation_hw_switching_disable, 1);
	}

	/* we should not need this but just in case */
	phy_ac_tssi_loopback_path_setup(pi, LOOPBACK_FOR_TSSICAL);

	if (TINY_RADIO(pi)) {
		MOD_PHYREG(pi, RxSdFeConfig1, farrow_rshift_force, 1);
		orig_RxSdFeConfig6 = READ_PHYREG(pi, RxSdFeConfig6);
		MOD_PHYREG(pi, RxSdFeConfig6, rx_farrow_rshift_0,
			READ_PHYREGFLD(pi, RxSdFeConfig1, farrow_rshift_tx));
	}

	// Additional settings for 4365 core3 GPIO WAR
	if (ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev)) {
		MOD_PHYREG(pi, DcFiltAddress,        dcBypass,     1);
		MOD_PHYREG(pi, RfctrlCoreRXGAIN13,   rxgain_dvga,  0);
		MOD_PHYREG(pi, RfctrlCoreLpfGain3,   lpf_bq2_gain, 0);
		MOD_PHYREG(pi, RfctrlOverrideGains3, rxgain,       1);
		MOD_PHYREG(pi, RfctrlOverrideGains3, lpf_bq2_gain, 1);
	}

	/* force all TX cores on */
	orig_RfseqCoreActv2059 = READ_PHYREG(pi, RfseqCoreActv2059);
	MOD_PHYREG(pi, RfseqCoreActv2059, EnTx,  pi->sh->hw_phyrxchain);
	MOD_PHYREG(pi, RfseqCoreActv2059, DisRx, pi->sh->hw_phyrxchain);

	FOREACH_ACTV_CORE(pi, pi->sh->hw_phyrxchain, core) {
		wlc_phy_poll_samps_WAR_acphy(pi, idle_tssi, TRUE, TRUE, NULL,
		                             FALSE, TRUE, core, 0);
		pi_ac->idle_tssi[core] = idle_tssi[core];
		wlc_phy_txpwrctrl_set_idle_tssi_acphy(pi, idle_tssi[core], core);
		PHY_TRACE(("wl%d: %s: idle_tssi core%d: %d\n",
		           pi->sh->unit, __FUNCTION__, core, pi_ac->idle_tssi[core]));
	}

	WRITE_PHYREG(pi, RfseqCoreActv2059, orig_RfseqCoreActv2059);

	if (TINY_RADIO(pi)) {
		MOD_PHYREG(pi, RxSdFeConfig1, farrow_rshift_force, 0);
		WRITE_PHYREG(pi, RxSdFeConfig6, orig_RxSdFeConfig6);
	}

	if (ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev)) {
		/* Enable hwaci mitigation */
		WRITE_PHYREG(pi, ACI_Mitigation_CTRL, hwaci_state);
	}

	// Additional settings for 4365 core3 GPIO WAR
	if (ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev)) {
		MOD_PHYREG(pi, DcFiltAddress,        dcBypass,     0);
		MOD_PHYREG(pi, RfctrlOverrideGains3, rxgain,       0);
		MOD_PHYREG(pi, RfctrlOverrideGains3, lpf_bq2_gain, 0);
	}

	/* prevent crs trigger */
	wlc_phy_stay_in_carriersearch_acphy(pi, FALSE);
}

void
wlc_phy_tssi_phy_setup_acphy(phy_info_t *pi, uint8 for_iqcal)
{
	uint8 core;
	bool flag2rangeon =
		((CHSPEC_IS2G(pi->radio_chanspec) && pi->u.pi_acphy->srom_tworangetssi2g) ||
		(CHSPEC_IS5G(pi->radio_chanspec) && pi->u.pi_acphy->srom_tworangetssi5g)) &&
		PHY_IPA(pi);
	bool flaglowrangeon =
		((CHSPEC_IS2G(pi->radio_chanspec) && pi->u.pi_acphy->srom_lowpowerrange2g) ||
		(CHSPEC_IS5G(pi->radio_chanspec) && pi->u.pi_acphy->srom_lowpowerrange5g)) &&
		PHY_IPA(pi);

	FOREACH_ACTV_CORE(pi, pi->sh->hw_phyrxchain, core) {
		if (ACREV_IS(pi->pubpi.phy_rev, 4) && CHSPEC_IS2G(pi->radio_chanspec))
			MOD_PHYREG(pi, TSSIMode, tssiADCSel, 0);
		else
			MOD_PHYREG(pi, TSSIMode, tssiADCSel, 1);

		if (TINY_RADIO(pi)) {
			if (!PHY_IPA(pi)) {
				if (ACREV_IS(pi->pubpi.phy_rev, 4)) {
					MOD_PHYREG(pi, RxSdFeConfig1, farrow_rshift_tx, 0x0);
				} else if (ACMAJORREV_32(pi->pubpi.phy_rev) ||
					ACMAJORREV_33(pi->pubpi.phy_rev)) {
					MOD_PHYREG(pi, RxSdFeConfig1, farrow_rshift_tx, 0x1);
				} else {
					MOD_PHYREG(pi, RxSdFeConfig1, farrow_rshift_tx, 0x2);
				}
			} else {
				if (ACMAJORREV_4(pi->pubpi.phy_rev))
					MOD_PHYREG(pi, RxSdFeConfig1, farrow_rshift_tx, 0x1);
				else
					MOD_PHYREG(pi, RxSdFeConfig1, farrow_rshift_tx, 0x0);
			}

			if (pi->iboard) {
				MOD_PHYREG(pi, RxSdFeConfig1, farrow_rshift_tx, 0x1);
			}
		}
		if (ACMAJORREV_4(pi->pubpi.phy_rev)) {
			/* PingPongComp.sdadcloopback_sat = 0 would drop one lsb
			 * bit 1 would saturate 1 msb bit.
			 * Setting this to 0 to drop 1 bit.
			 */
			MOD_PHYREG(pi, PingPongComp, sdadcloopback_sat, 0x0);
			MOD_PHYREG(pi, RxSdFeConfig1, farrow_rshift_tx, 0x1);

		} else if (ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev)) {
			MOD_PHYREG(pi, PingPongComp, sdadcloopback_sat, 0x0);
			MOD_PHYREG(pi, RxSdFeConfig1, farrow_rshift_tx, 0x0);
		}

		if (!(ACMAJORREV_4(pi->pubpi.phy_rev))) {
			MOD_PHYREGCE(pi, RfctrlOverrideAuxTssi, core, tssi_pu, 1);
		}

		if (!PHY_IPA(pi)) {
			MOD_PHYREGCE(pi, RfctrlCoreAuxTssi1, core, tssi_pu, for_iqcal);
		} else {
			if (!(ACMAJORREV_4(pi->pubpi.phy_rev))) {
				MOD_PHYREGCE(pi, RfctrlCoreAuxTssi1, core, tssi_pu, 1);
			}
			if (flag2rangeon) {
				MOD_PHYREGCE(pi, RfctrlOverrideAuxTssi,  core, tssi_range, 0);
				MOD_PHYREGCE(pi, RfctrlCoreAuxTssi1,     core, tssi_range, 0);
			} else if (flaglowrangeon) {
				MOD_PHYREGCE(pi, RfctrlOverrideAuxTssi,  core, tssi_range, 1);
				MOD_PHYREGCE(pi, RfctrlCoreAuxTssi1,     core, tssi_range, 0);
			} else {
				MOD_PHYREGCE(pi, RfctrlOverrideAuxTssi,  core, tssi_range, 1);
				MOD_PHYREGCE(pi, RfctrlCoreAuxTssi1,     core, tssi_range,
					~(for_iqcal));
			}
		}
	}
}

void
wlc_phy_tssi_radio_setup_acphy_tiny(phy_info_t *pi, uint8 core_mask, uint8 for_iqcal)
{
	uint8 core, pupd = 0;

	ASSERT(TINY_RADIO(pi));

	/* 20691_tssi_radio_setup */
	/* # Powerdown rcal otherwise it won't let any other test point go through */
	if ((RADIOID(pi->pubpi.radioid) == BCM20691_ID) ||
			(RADIOID(pi->pubpi.radioid) == BCM20693_ID && RADIOMAJORREV(pi) == 3)) {
		/* 20691_gpaio # Powerup gpaio block, powerdown rcal,
		 * clear all test point selection
		 */
		MOD_RADIO_REG_TINY(pi, GPAIO_SEL2, 0, gpaio_pu, 1);

		MOD_RADIO_REG_TINY(pi, GPAIO_SEL0, 0, gpaio_sel_0to15_port, 0x0);
		MOD_RADIO_REG_TINY(pi, GPAIO_SEL1, 0, gpaio_sel_16to31_port, 0x0);

		MOD_RADIO_REG_TINY(pi, GPAIO_SEL0, 0, gpaio_sel_0to15_port, 0x0);
		MOD_RADIO_REG_TINY(pi, GPAIO_SEL1, 0, gpaio_sel_16to31_port, 0x0);
	}
	FOREACH_ACTV_CORE(pi, core_mask, core) {
		if (PHY_IPA(pi) || pi->iboard) {
			/* #
			 *  # INT TSSI setup
			 *  #
			 */
			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				MOD_RADIO_REG_TINY(pi, IQCAL_CFG1, core, iqcal_sel_sw, 0x0);
				/* # B3: (1 = iqcal, 0 = tssi);
				 * # B2: (1 = ext-tssi, 0 = int-tssi)
				 * # B1: (1 = 5g, 0 = 2g)
				 * # B0: (1 = wo filter, 0 = w filter for ext-tssi)
				 */
				 /* # Select PA output (and not PA input) */
				MOD_RADIO_REG_TINY(pi, PA2G_CFG1, core, pa2g_tssi_ctrl_sel, 0);
				pupd = 1;
			} else {
				pupd = 0;
				MOD_RADIO_REG_TINY(pi, IQCAL_CFG1, core, iqcal_sel_sw, 0x2);
				/* # Select PA output (and not PA input) */
				MOD_RADIO_REG_TINY(pi, TX5G_MISC_CFG1, core,
					pa5g_tssi_ctrl_sel, 0);
				MOD_RADIO_REG_TINY(pi, TX_TOP_5G_OVR1, core,
					ovr_pa5g_tssi_ctrl_sel, 1);
			}
			MOD_RADIO_REG_20693(pi, TX5G_MISC_CFG1, core,
				pa5g_tssi_ctrl_range, (1-pupd));
			MOD_RADIO_REG_20693(pi, TX_TOP_5G_OVR1, core,
				ovr_pa5g_tssi_ctrl_range, (1-pupd));
			MOD_RADIO_REG_20693(pi, TX2G_MISC_CFG1, core, pa2g_tssi_ctrl_range, pupd);
			MOD_RADIO_REG_20693(pi, TX_TOP_2G_OVR_EAST,
			core, ovr_pa2g_tssi_ctrl_range, pupd);

			/* # int-tssi select */
			MOD_RADIO_REG_TINY(pi, IQCAL_CFG1, core, iqcal_sel_ext_tssi, 0x0);
		} else {
			/* #
			 * # EPA TSSI setup
			 * #
			 */
			/* # Enabling and Muxing per band */

			if (CHSPEC_IS5G(pi->radio_chanspec)) {
			    MOD_RADIO_REG_TINY(pi, IQCAL_CFG1, core, iqcal_sel_sw, 0x3);
			} else {
				/* ### gband */
			    MOD_RADIO_REG_TINY(pi, IQCAL_CFG1, core, iqcal_sel_sw, 0x1);
			}
			/* # ext-tssi select */
			MOD_RADIO_REG_TINY(pi, IQCAL_CFG1, core, iqcal_sel_ext_tssi, 0x1);
			if (!(RADIOID(pi->pubpi.radioid) == BCM20693_ID &&
					RADIOMAJORREV(pi) == 3)) {
				MOD_RADIO_REG_TINY(pi, IQCAL_OVR1, core, ovr_iqcal_PU_tssi, 0x1);
				/* # power on tssi */
				MOD_RADIO_REG_TINY(pi, IQCAL_CFG1, core, iqcal_PU_tssi, 0x1);
			}
		}

		if (!(RADIOID(pi->pubpi.radioid) == BCM20693_ID && RADIOMAJORREV(pi) == 3)) {
			MOD_RADIO_REG_TINY(pi, IQCAL_OVR1, core, ovr_iqcal_PU_iqcal, 0x1);
			/* # power off iqlocal */
			MOD_RADIO_REG_TINY(pi, IQCAL_CFG1, core, iqcal_PU_iqcal, 0x0);
			MOD_RADIO_REG_TINY(pi, IQCAL_CFG1, core, iqcal_PU_tssi, 1);
			MOD_RADIO_REG_TINY(pi, IQCAL_OVR1, core, ovr_iqcal_PU_tssi, 1);
		} else {
			/* # power on tssi/iqcal */
			MOD_RADIO_REG_TINY(pi, IQCAL_CFG1, core, iqcal_PU_iqcal, 0x1);
		}

		if (RADIOID(pi->pubpi.radioid) == BCM20693_ID && RADIOMAJORREV(pi) == 3) {
			MOD_RADIO_REG_TINY(pi, TIA_CFG9, core, txbb_dac2adc, 0x0);
			MOD_RADIO_REG_TINY(pi, TIA_CFG5, core, tia_out_test, 0x0);

			MOD_RADIO_REG_TINY(pi, AUXPGA_OVR1, core, ovr_auxpga_i_pu, 0x1);
			MOD_RADIO_REG_TINY(pi, AUXPGA_CFG1, core, auxpga_i_pu, 0x1);
		}
		/* MOD_RADIO_REG_20691(pi, ADC_CFG10, 0, adc_in_test, 0xF); */

		MOD_RADIO_REG_TINY(pi, AUXPGA_OVR1, core, ovr_auxpga_i_sel_input, 0x1);
		MOD_RADIO_REG_TINY(pi, AUXPGA_CFG1, core, auxpga_i_sel_input, 0x2);
	}
}

void
wlc_phy_tssi_radio_setup_acphy(phy_info_t *pi, uint8 core_mask, uint8 for_iqcal)
{
	uint8 core;

	ASSERT(RADIOID(pi->pubpi.radioid) == BCM2069_ID);

	/* 2069_gpaio(clear) to pwr up the GPAIO and clean up al lthe otehr test pins */
	if (RADIOMAJORREV(pi) == 2) {
		FOREACH_ACTV_CORE(pi, core_mask, core) {
			/* first powerup the CGPAIO block */
			MOD_RADIO_REGC(pi, GE32_CGPAIO_CFG1, core, cgpaio_pu, 1);
		}
		/* turn off all test points in cgpaio block to avoid conflict,disable tp0 to tp15 */
		phy_utils_write_radioreg(pi, RFX_2069_GE32_CGPAIO_CFG2, 0);
		/* disable tp16 to tp31 */
		phy_utils_write_radioreg(pi, RFX_2069_GE32_CGPAIO_CFG3, 0);
		/* Disable muxsel0 and muxsel1 test points */
		phy_utils_write_radioreg(pi, RFX_2069_GE32_CGPAIO_CFG4, 0);
		/* Disable muxsel2 and muxselgpaio test points */
		phy_utils_write_radioreg(pi, RFX_2069_GE32_CGPAIO_CFG5, 0);
	} else {
		/* first powerup the CGPAIO block */
		MOD_RADIO_REG(pi, RF2, CGPAIO_CFG1, cgpaio_pu, 1);
		/* turn off all test points in cgpaio block to avoid conflict,disable tp0 to tp15 */
		phy_utils_write_radioreg(pi, RF2_2069_CGPAIO_CFG2, 0);
		/* disable tp16 to tp31 */
		phy_utils_write_radioreg(pi, RF2_2069_CGPAIO_CFG3, 0);
		/* Disable muxsel0 and muxsel1 test points */
		phy_utils_write_radioreg(pi, RF2_2069_CGPAIO_CFG4, 0);
		/* Disable muxsel2 and muxselgpaio test points */
		phy_utils_write_radioreg(pi, RF2_2069_CGPAIO_CFG5, 0);
	}
	/* Powerdown rcal. This is one of the enable pins to AND gate in cgpaio block */
	MOD_RADIO_REG(pi, RF2, RCAL_CFG, pu, 0);

	FOREACH_ACTV_CORE(pi, core_mask, core) {

		if (for_iqcal == 0) {
			if (PHY_IPA(pi)) {
				if (CHSPEC_IS5G(pi->radio_chanspec)) {
					MOD_RADIO_REGC(pi, IQCAL_CFG1, core, sel_sw, 0x2);
					MOD_RADIO_REGC(pi, IQCAL_CFG1, core, sel_ext_tssi, 0x0);
					MOD_RADIO_REGC(pi, GE16_OVR21, core,
						ovr_pa5g_ctrl_tssi_sel, 0x1);
					MOD_RADIO_REGC(pi, TX5G_TSSI, core,
						pa5g_ctrl_tssi_sel, 0x0);
				} else {
					MOD_RADIO_REGC(pi, IQCAL_CFG1, core, sel_sw, 0x0);
					MOD_RADIO_REGC(pi, IQCAL_CFG1, core, sel_ext_tssi, 0x0);
					MOD_RADIO_REGC(pi, GE16_OVR21, core,
						ovr_pa2g_ctrl_tssi_sel, 0x1);
					MOD_RADIO_REGC(pi, PA2G_TSSI, core,
						pa2g_ctrl_tssi_sel, 0x0);
				}
			} else  {
				if ((RADIOMAJORREV(pi) > 0) &&
				    CHSPEC_IS5G(pi->radio_chanspec)) {
					MOD_RADIO_REGC(pi, IQCAL_CFG1, core, sel_sw, 0x3);
				} else {
					MOD_RADIO_REGC(pi, IQCAL_CFG1, core, sel_sw, 0x1);
				}
			}
		} else {
			MOD_RADIO_REGC(pi, IQCAL_CFG1, core, sel_sw,
			               (CHSPEC_IS5G(pi->radio_chanspec)) ? 0x2 : 0x0);
		}
		if (!PHY_IPA(pi)) {
			MOD_RADIO_REGC(pi, IQCAL_CFG1, core, sel_ext_tssi, for_iqcal == 0);

			switch (core) {
			case 0:
				MOD_RADIO_REG(pi, RF2, CGPAIO_CFG4, cgpaio_tssi_muxsel0, 0x1);
				break;
			case 1:
				MOD_RADIO_REG(pi, RF2, CGPAIO_CFG4, cgpaio_tssi_muxsel1, 0x1);
				break;
			case 2:
				MOD_RADIO_REG(pi, RF2, CGPAIO_CFG5, cgpaio_tssi_muxsel2, 0x1);
				break;
			case 3:
				MOD_RADIO_REG(pi, RF2, CGPAIO_CFG5, cgpaio_tssi_muxsel2, 0x1);
				break;
			}
		}

		MOD_RADIO_REGC(pi, IQCAL_CFG1, core, tssi_GPIO_ctrl, 0);
		MOD_RADIO_REGC(pi, TESTBUF_CFG1, core, GPIO_EN, 0);

		/* Reg conflict with 2069 rev 16 */
		if (RADIOMAJORREV(pi) == 0) {
			MOD_RADIO_REGC(pi, TX5G_TSSI, core, pa5g_ctrl_tssi_sel, for_iqcal);
			MOD_RADIO_REGC(pi, OVR20, core, ovr_pa5g_ctrl_tssi_sel, 1);
		} else if ((RADIOMAJORREV(pi) == 1) ||
			(RADIOMAJORREV(pi) == 2)) {
			MOD_RADIO_REGC(pi, TX5G_TSSI, core, pa5g_ctrl_tssi_sel, for_iqcal);
			MOD_RADIO_REGC(pi, GE16_OVR21, core, ovr_pa5g_ctrl_tssi_sel, 1);
			MOD_RADIO_REGC(pi, PA2G_TSSI, core, pa2g_ctrl_tssi_sel, for_iqcal);
			MOD_RADIO_REGC(pi, GE16_OVR21, core, ovr_pa2g_ctrl_tssi_sel, 1);
		}

		if (RADIOMAJORREV(pi) == 1) {
			MOD_RADIO_REGC(pi, AUXPGA_CFG1, core, auxpga_i_vcm_ctrl, 0x0);
			/* This bit is supposed to be controlled by phy direct control line.
			 * Please check: http://jira.broadcom.com/browse/HW11ACRADIO-45
			 */
			MOD_RADIO_REGC(pi, AUXPGA_CFG1, core, auxpga_i_sel_input, 0x2);
		}

	}
}
void
phy_ac_tssi_loopback_path_setup(phy_info_t *pi, uint8 for_iqcal)
{
	wlc_phy_tssi_phy_setup_acphy(pi, for_iqcal);
	if (TINY_RADIO(pi))
		wlc_phy_tssi_radio_setup_acphy_tiny(pi, pi->sh->hw_phyrxchain, for_iqcal);
	else
		wlc_phy_tssi_radio_setup_acphy(pi, pi->sh->hw_phyrxchain, for_iqcal);
}
void
wlc_phy_txpwrctrl_set_idle_tssi_acphy(phy_info_t *pi, int16 idle_tssi, uint8 core)
{
	bool flag2rangeon =
		((CHSPEC_IS2G(pi->radio_chanspec) && pi->u.pi_acphy->srom_tworangetssi2g) ||
		(CHSPEC_IS5G(pi->radio_chanspec) && pi->u.pi_acphy->srom_tworangetssi5g)) &&
		PHY_IPA(pi);

	/* set idle TSSI in 2s complement format (max is 0x1ff) */
	switch (core) {
	case 0:
		MOD_PHYREG(pi, TxPwrCtrlIdleTssi_path0, idleTssi0, idle_tssi);
		break;
	case 1:
		MOD_PHYREG(pi, TxPwrCtrlIdleTssi_path1, idleTssi1, idle_tssi);
		break;
	case 2:
		MOD_PHYREG(pi, TxPwrCtrlIdleTssi_path2, idleTssi2, idle_tssi);
		break;
	case 3:
		MOD_PHYREG(pi, TxPwrCtrlIdleTssi_path3, idleTssi3, idle_tssi);
		break;
	}

	/* Only 4335 and 4350 has 2nd idle-tssi */
	if (((ACMAJORREV_1(pi->pubpi.phy_rev) || ACMAJORREV_2(pi->pubpi.phy_rev) ||
	      TINY_RADIO(pi)) && BF3_TSSI_DIV_WAR(pi->u.pi_acphy)) || flag2rangeon) {
		switch (core) {
		case 0:
			MOD_PHYREG(pi, TxPwrCtrlIdleTssi_second_path0,
			           idleTssi_second0, idle_tssi);
			break;
		case 1:
			MOD_PHYREG(pi, TxPwrCtrlIdleTssi_second_path1,
			           idleTssi_second1, idle_tssi);
			break;
		}
	} else if (ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev)) {
		switch (core) {
		case 0:
			MOD_PHYREG(pi, TxPwrCtrlIdleTssi_second_path0, idleTssi_second0, idle_tssi);
			MOD_PHYREG(pi, TxPwrCtrlIdleTssi_third_path0, idleTssi_third0, idle_tssi);
			break;
		case 1:
			MOD_PHYREG(pi, TxPwrCtrlIdleTssi_second_path1, idleTssi_second1, idle_tssi);
			MOD_PHYREG(pi, TxPwrCtrlIdleTssi_third_path1, idleTssi_third1, idle_tssi);
			break;
		case 2:
			MOD_PHYREG(pi, TxPwrCtrlIdleTssi_second_path2, idleTssi_second2, idle_tssi);
			MOD_PHYREG(pi, TxPwrCtrlIdleTssi_third_path2, idleTssi_third2, idle_tssi);
			break;
		case 3:
			MOD_PHYREG(pi, TxPwrCtrlIdleTssi_second_path3, idleTssi_second3, idle_tssi);
			MOD_PHYREG(pi, TxPwrCtrlIdleTssi_third_path3, idleTssi_third3, idle_tssi);
			break;
		}
	}
}

int8
wlc_phy_tssivisible_thresh_acphy(phy_info_t *pi)
{
	int8 visi_thresh_qdbm;
	uint16 channel = CHSPEC_CHANNEL(pi->radio_chanspec);

	if (ACMAJORREV_0(pi->pubpi.phy_rev)) {
		/* J28 */
		if ((BFCTL(pi->u.pi_acphy) == 3) && (BF3_FEMCTRL_SUB(pi->u.pi_acphy) == 2)) {
			visi_thresh_qdbm = 30;  /* 7.5*4 */
		} else {
			if (IS_X52C_BOARDTYPE(pi))
				visi_thresh_qdbm = 5*4;
			else if (IS_X29C_BOARDTYPE(pi) && (channel >= 149))
				visi_thresh_qdbm = 22; /* 5.5 dBm */
			else
				visi_thresh_qdbm = 6*4;
		}
	}
	else if (ACMAJORREV_2(pi->pubpi.phy_rev)) {
			visi_thresh_qdbm = 7*4;
	}
	else if (ACMAJORREV_5(pi->pubpi.phy_rev)) {
	  if (pi->u.pi_acphy->srom_2g_pdrange_id == 24)
	    visi_thresh_qdbm = 4*4;
	  else
	    visi_thresh_qdbm = 7*4;
	}
	else visi_thresh_qdbm = WL_RATE_DISABLED;

	return visi_thresh_qdbm;
}

#if defined(WLTEST)
int16
wlc_phy_test_tssi_acphy(phy_info_t *pi, int8 ctrl_type, int8 pwr_offs)
{
	int16 tssi = 0;
	int16 temp = 0;
	int Npt, Npt_log2, i;

	Npt_log2 = READ_PHYREGFLD(pi, TxPwrCtrlNnum, Npt_intg_log2);
	Npt = 1 << Npt_log2;

	switch (ctrl_type & 0x7) {
	case 0:
	case 1:
	case 2:
	case 3:
		for (i = 0; i < Npt; i++) {
			OSL_DELAY(10);
			temp = READ_PHYREGCE(pi, TssiVal_path, ctrl_type) & 0x3ff;
			temp -= (temp >= 512) ? 1024 : 0;
			tssi += temp;
		}
		tssi = tssi >> Npt_log2;
		break;
	default:
		tssi = -1024;
	}
	return (tssi);
}

int16
wlc_phy_test_idletssi_acphy(phy_info_t *pi, int8 ctrl_type)
{
	int16 idletssi = INVALID_IDLETSSI_VAL;

	switch (ctrl_type & 0x7) {
	case 0:
	case 1:
	case 2:
	case 3:
		idletssi = READ_PHYREGCE(pi, TxPwrCtrlIdleTssi_path, ctrl_type) & 0x3ff;
		idletssi -= (idletssi >= 512) ? 1024 : 0;
		break;
	default:
		idletssi = INVALID_IDLETSSI_VAL;
	}

	return (idletssi);
}
#endif // endif

void
wlc_phy_get_tssisens_min_acphy(phy_info_t *pi, int8 *tssiSensMinPwr)
{
	tssiSensMinPwr[0] = READ_PHYREGFLD(pi, TxPwrCtrlCore0TSSISensLmt, tssiSensMinPwr);
	if (PHYCORENUM(pi->pubpi.phy_corenum) > 1) {
		tssiSensMinPwr[1] = READ_PHYREGFLD(pi, TxPwrCtrlCore1TSSISensLmt, tssiSensMinPwr);
		if (PHYCORENUM(pi->pubpi.phy_corenum) > 2)
			tssiSensMinPwr[2] = READ_PHYREGFLD(pi, TxPwrCtrlCore2TSSISensLmt,
				tssiSensMinPwr);
	}
}
