/*
 * ACPHY TEMPerature sense module implementation
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
#include "phy_type_temp.h"
#include "phy_temp_st.h"
#include <phy_ac.h>
#include <phy_ac_temp.h>

#include <phy_ac_info.h>
#include <phy_utils_reg.h>
#include <wlc_phy_ac.h>

#include "wlc_phy_radio.h"
#include "wlc_phyreg_ac.h"
#include "wlc_radioreg_20691.h"
#include "wlc_radioreg_20693.h"

#ifdef ATE_BUILD
#include <wl_ate.h>
#endif // endif

/* module private states */
struct phy_ac_temp_info {
	phy_info_t *pi;
	phy_ac_info_t *aci;
	phy_temp_info_t *ti;
	acphy_tempsense_radioregs_t *ac_tempsense_radioregs_orig;
	acphy_tempsense_phyregs_t   *ac_tempsense_phyregs_orig;

/* add other variable size variables here at the end */
};

/* local functions */
static uint8 phy_ac_temp_throttle(phy_type_temp_ctx_t *ctx);
static int phy_ac_temp_get(phy_type_temp_ctx_t *ctx);
static uint8 phy_get_first_actv_core(uint8 coremask);

/* Register/unregister ACPHY specific implementation to common layer. */
phy_ac_temp_info_t *
BCMATTACHFN(phy_ac_temp_register_impl)(phy_info_t *pi, phy_ac_info_t *aci, phy_temp_info_t *ti)
{
	phy_ac_temp_info_t *info;
	phy_type_temp_fns_t fns;
	acphy_tempsense_radioregs_t *ac_tempsense_radioregs_orig = NULL;
	acphy_tempsense_phyregs_t   *ac_tempsense_phyregs_orig = NULL;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage in once */
	if ((info = phy_malloc(pi, sizeof(phy_ac_temp_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}

	if ((ac_tempsense_radioregs_orig =
		phy_malloc(pi, sizeof(acphy_tempsense_radioregs_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc ac_tempsense_radioregs_orig failed\n", __FUNCTION__));
		goto fail;
	}

	if ((ac_tempsense_phyregs_orig =
		phy_malloc(pi, sizeof(acphy_tempsense_phyregs_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc ac_tempsense_phyregs_orig failed\n", __FUNCTION__));
		goto fail;
	}

	info->pi = pi;
	info->aci = aci;
	info->ti = ti;
	info->ac_tempsense_radioregs_orig = ac_tempsense_radioregs_orig;
	info->ac_tempsense_phyregs_orig = ac_tempsense_phyregs_orig;

	/* Register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.throt = phy_ac_temp_throttle;
	fns.get = phy_ac_temp_get;
	fns.ctx = info;

	phy_temp_register_impl(ti, &fns);

	return info;
fail:
	if (ac_tempsense_phyregs_orig != NULL)
		phy_mfree(pi, ac_tempsense_phyregs_orig, sizeof(acphy_tempsense_phyregs_t));
	if (ac_tempsense_radioregs_orig != NULL)
		phy_mfree(pi, ac_tempsense_radioregs_orig, sizeof(acphy_tempsense_radioregs_t));

	if (info != NULL)
		phy_mfree(pi, info, sizeof(phy_ac_temp_info_t));
	return NULL;
}

void
BCMATTACHFN(phy_ac_temp_unregister_impl)(phy_ac_temp_info_t *info)
{
	phy_info_t *pi;
	phy_temp_info_t *ti;

	ASSERT(info);
	pi = info->pi;
	ti = info->ti;

	PHY_TRACE(("%s\n", __FUNCTION__));

	phy_temp_unregister_impl(ti);

	phy_mfree(pi, info->ac_tempsense_phyregs_orig, sizeof(acphy_tempsense_phyregs_t));

	phy_mfree(pi, info->ac_tempsense_radioregs_orig, sizeof(acphy_tempsense_radioregs_t));

	phy_mfree(pi, info, sizeof(phy_ac_temp_info_t));
}

/* XXX Tx-Core Shut-Down to prevent hitting critical junction temperature
 * Assumptions: Code written assuming max txchain = 7 (3 Tx Chain)
 * Output is stored in pi->txcore_temp->bitmap.
 * BitMap returns the active RxChain and TxChain.
 */
static uint8
phy_ac_temp_throttle(phy_type_temp_ctx_t *ctx)
{
	phy_ac_temp_info_t *info = (phy_ac_temp_info_t *)ctx;
	phy_temp_info_t *ti = info->ti;
	phy_info_t *pi = info->pi;
	phy_txcore_temp_t *temp;
	/* XXX Shut-Down All Tx-Core Except One When Hot
	 * When there is only 1 tx-core on, there will be no change
	 * When active coremash is 15 (4366), change to 13
	 */
	uint8 txcore_shutdown_lut[] = {1, 1, 2, 1, 4, 1, 2, 1,
		8, 8, 2, 9, 4, 9, 4, 13};
	uint8 txcore_shutdown_lut_80p80 = 5;
	uint8 phyrxchain = pi->sh->phyrxchain;
	uint8 phytxchain = pi->sh->phytxchain;
	uint8 new_phytxchain;
	int16 currtemp, delta_temp = 0;

	PHY_TRACE(("%s\n", __FUNCTION__));
	/* No need to do tempsense/Throttle/Cal when not associated
	  * and returning hw_txchain to avoid throttling in upper layer
	  */
	if (ACMAJORREV_4(pi->pubpi.phy_rev) && PUB_NOT_ASSOC(pi))
		return pi->sh->hw_phytxchain;

	temp = phy_temp_get_st(ti);
	ASSERT(temp != NULL);

	if (RADIOID(pi->pubpi.radioid) == BCM20691_ID)
		return 0;

	ASSERT(phytxchain);

	currtemp = wlc_phy_tempsense_acphy(pi);
	if (ACMAJORREV_4(pi->pubpi.phy_rev)) {
		/* Checking temperature for every 10sec and
		     Triggering calibration if there is a temperature
		     change greater than phycal_tempdelta.
		     This is needed to address Tx throughput degradation
		     seen on 4349 MIMO mode.
		  */
		delta_temp = ((currtemp > pi->cal_info->last_cal_temp) ?
			(currtemp - pi->cal_info->last_cal_temp) :
			(pi->cal_info->last_cal_temp - currtemp));
		/* Before triggering the cal make sure no cals are pending */
		if ((pi->phycal_tempdelta) && (delta_temp > pi->phycal_tempdelta) &&
			(!PHY_PERICAL_MPHASE_PENDING(pi))) {
			pi->cal_info->last_cal_temp = currtemp;
			wlc_phy_cals_acphy(pi, PHY_CAL_SEARCHMODE_RESTART);
		}
	}
#ifdef BCMDBG
	if (pi->tempsense_override)
		currtemp = pi->tempsense_override;
#endif // endif
	if (phy_get_phymode(pi) == PHYMODE_MIMO) {
		if (currtemp >= temp->disable_temp) {
			if (CHSPEC_IS160(pi->radio_chanspec) ||
				PHY_AS_80P80(pi, pi->radio_chanspec)) {
				new_phytxchain = txcore_shutdown_lut_80p80;
			} else {
				new_phytxchain = txcore_shutdown_lut[phytxchain];
			}
			//temp->heatedup = TRUE;
			temp->bitmap = ((phyrxchain << 4) | new_phytxchain);
		}
		if (currtemp <= temp->enable_temp) {
			new_phytxchain = pi->sh->hw_phytxchain;
			//temp->heatedup = FALSE;
			temp->bitmap = ((phyrxchain << 4) | new_phytxchain);
		}
	}

	return temp->bitmap;
}

/* read the current temperature */
static int
phy_ac_temp_get(phy_type_temp_ctx_t *ctx)
{
	phy_ac_temp_info_t *info = (phy_ac_temp_info_t *)ctx;
	phy_ac_info_t *aci = info->aci;

	return aci->current_temperature;
}

/* ********************************************* */
/*				Internal Definitions					*/
/* ********************************************* */
static void wlc_phy_tempsense_radio_setup_acphy(phy_info_t *pi, uint16 Av, uint16 Vmid);
static void wlc_phy_tempsense_radio_cleanup_acphy(phy_info_t *pi);
static void wlc_phy_tempsense_phy_setup_acphy(phy_info_t *pi);
static void wlc_phy_tempsense_phy_cleanup_acphy(phy_info_t *pi);
static int32 wlc_phy_tempsense_phy_setup_acphy_tiny(phy_info_t *pi, uint8 core);
static int32 wlc_phy_tempsense_radio_setup_acphy_tiny(phy_info_t *pi, uint16 Av,
	uint16 Vmid, uint8 core);
static int32 wlc_phy_tempsense_poll_adc_war_tiny(phy_info_t *pi,
	bool init_adc_inside, int32 *measured_values, uint8 core);
static int32 wlc_phy_tempsense_poll_samps_tiny(phy_info_t *pi,
	uint16 samples, bool init_adc_inside, uint8 core);
static int32 wlc_phy_tempsense_gpiosel_tiny(phy_info_t *pi,
	uint16 sel, uint8 word_swap);
static int32 wlc_phy_tempsense_radio_swap_tiny(phy_info_t *pi,
	acphy_tempsense_cfg_opt_t type, uint8 swap, uint8 core);
static int32 wlc_phy_tempsense_phy_cleanup_acphy_tiny(phy_info_t *pi, uint8 core);
static int32 wlc_phy_tempsense_radio_cleanup_acphy_tiny(phy_info_t *pi, uint8 core);

static void
wlc_phy_tempsense_radio_setup_acphy(phy_info_t *pi, uint16 Av, uint16 Vmid)
{
	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;
	tempsense_radioregs_t *porig =
		&(pi_ac->tempi->ac_tempsense_radioregs_orig->u.acphy_tempsense_radioregs);
	uint8 core;

	ASSERT(RADIOID(pi->pubpi.radioid) == BCM2069_ID);
	FOREACH_ACTV_CORE(pi, pi->sh->phyrxchain, core) {
		/* Reg conflict with 2069 rev 16 */
		if (RADIOMAJORREV(pi) == 0)
			porig->OVR18[core]         = READ_RADIO_REGC(pi, RF, OVR18, core);
		else
			porig->OVR19[core]         = READ_RADIO_REGC(pi, RF, GE16_OVR19, core);
		porig->tempsense_cfg[core] = READ_RADIO_REGC(pi, RF, TEMPSENSE_CFG, core);
		porig->OVR5[core]          = READ_RADIO_REGC(pi, RF, OVR5, core);
		porig->testbuf_cfg1[core]  = READ_RADIO_REGC(pi, RF, TESTBUF_CFG1, core);
		porig->OVR3[core]          = READ_RADIO_REGC(pi, RF, OVR3, core);
		porig->auxpga_cfg1[core]   = READ_RADIO_REGC(pi, RF, AUXPGA_CFG1, core);
		porig->auxpga_vmid[core]   = READ_RADIO_REGC(pi, RF, AUXPGA_VMID, core);

		MOD_RADIO_REGC(pi, OVR5, core, ovr_tempsense_pu, 0x1);
		MOD_RADIO_REGC(pi, TEMPSENSE_CFG, core, pu, 0x1);
		MOD_RADIO_REGC(pi, OVR5, core, ovr_testbuf_PU, 0x1);
		MOD_RADIO_REGC(pi, TESTBUF_CFG1, core, PU, 0x1);
		MOD_RADIO_REGC(pi, TESTBUF_CFG1, core, GPIO_EN, 0x0);
		MOD_RADIO_REGC(pi, OVR3, core, ovr_afe_auxpga_i_sel_vmid, 0x1);
		MOD_RADIO_REGC(pi, AUXPGA_VMID, core, auxpga_i_sel_vmid, Vmid);
		MOD_RADIO_REGC(pi, OVR3, core, ovr_auxpga_i_sel_gain, 0x1);
		MOD_RADIO_REGC(pi, AUXPGA_CFG1, core, auxpga_i_sel_gain, Av);

		if (RADIOMAJORREV(pi) == 1) {
			MOD_RADIO_REGC(pi, AUXPGA_CFG1, core, auxpga_i_vcm_ctrl, 0x0);
			/* This bit is supposed to be controlled by phy direct control line.
			 * Please check: http://jira.broadcom.com/browse/HW11ACRADIO-45
			 */
			MOD_RADIO_REGC(pi, AUXPGA_CFG1, core, auxpga_i_sel_input, 0x1);
		}
	}
}

static void
wlc_phy_tempsense_radio_cleanup_acphy(phy_info_t *pi)
{
	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;
	tempsense_radioregs_t *porig =
		&(pi_ac->tempi->ac_tempsense_radioregs_orig->u.acphy_tempsense_radioregs);
	uint8 core;

	ASSERT(RADIOID(pi->pubpi.radioid) == BCM2069_ID);
	FOREACH_ACTV_CORE(pi, pi->sh->phyrxchain, core) {
		/* Reg conflict with 2069 rev 16 */
		if (RADIOMAJORREV(pi) == 0)
			phy_utils_write_radioreg(pi, RF_2069_OVR18(core), porig->OVR18[core]);
		else
			phy_utils_write_radioreg(pi, RF_2069_GE16_OVR19(core), porig->OVR19[core]);
		phy_utils_write_radioreg(pi, RF_2069_TEMPSENSE_CFG(core),
			porig->tempsense_cfg[core]);
		phy_utils_write_radioreg(pi, RF_2069_OVR5(core), porig->OVR5[core]);
		phy_utils_write_radioreg(pi, RF_2069_TESTBUF_CFG1(core), porig->testbuf_cfg1[core]);
		phy_utils_write_radioreg(pi, RF_2069_OVR3(core), porig->OVR3[core]);
		phy_utils_write_radioreg(pi, RF_2069_AUXPGA_CFG1(core), porig->auxpga_cfg1[core]);
		phy_utils_write_radioreg(pi, RF_2069_AUXPGA_VMID(core), porig->auxpga_vmid[core]);
	}
}

static uint8
phy_get_first_actv_core(uint8 coremask)
{
	/* Get the first active core */
	switch (coremask & (256-coremask)) {
		case 1:
			return 0;
		case 2:
			return 1;
		case 4:
			return 2;
		case 8:
			return 3;
		default:
			return 0;
	}
}

static void
wlc_phy_tempsense_phy_setup_acphy(phy_info_t *pi)
{
	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;
	acphy_tempsense_phyregs_t *porig = (pi_ac->tempi->ac_tempsense_phyregs_orig);
	uint16 sdadc_config, mask;
	uint8 core;

	if (CHSPEC_IS80(pi->radio_chanspec) || PHY_AS_80P80(pi, pi->radio_chanspec)) {
		sdadc_config = sdadc_cfg80;
	} else if (CHSPEC_IS160(pi->radio_chanspec)) {
		sdadc_config = sdadc_cfg80; // FIXME
		ASSERT(0);
	} else if (CHSPEC_IS40(pi->radio_chanspec)) {
		if (pi->sdadc_config_override)
			sdadc_config = sdadc_cfg40hs;
		else
		sdadc_config = sdadc_cfg40;
	} else {
		sdadc_config = sdadc_cfg20;
	}

	porig->RxFeCtrl1 = READ_PHYREG(pi, RxFeCtrl1);
	MOD_PHYREG(pi, RxFeCtrl1, swap_iq0, 0);
	MOD_PHYREG(pi, RxFeCtrl1, swap_iq1, 0);
	MOD_PHYREG(pi, RxFeCtrl1, swap_iq2, 0);

	FOREACH_ACTV_CORE(pi, pi->sh->phyrxchain, core) {
		mask = ACPHY_RfctrlIntc0_override_tr_sw_MASK(rev) |
			ACPHY_RfctrlIntc0_tr_sw_tx_pu_MASK(rev);
		porig->RfctrlIntc[core] = phy_utils_read_phyreg(pi,
			ACPHYREGCE(pi, RfctrlIntc, core));
		phy_utils_write_phyreg(pi, ACPHYREGCE(pi,
			RfctrlIntc, core), mask);  /* elna off */

		porig->RfctrlOverrideAuxTssi[core]  = READ_PHYREGCE(pi, RfctrlOverrideAuxTssi,
		                                                    core);
		porig->RfctrlCoreAuxTssi1[core]     = READ_PHYREGCE(pi, RfctrlCoreAuxTssi1, core);
		porig->RfctrlOverrideRxPus[core]    = READ_PHYREGCE(pi, RfctrlOverrideRxPus, core);
		porig->RfctrlCoreRxPus[core]        = READ_PHYREGCE(pi, RfctrlCoreRxPus, core);
		porig->RfctrlOverrideTxPus[core]    = READ_PHYREGCE(pi, RfctrlOverrideTxPus, core);
		porig->RfctrlCoreTxPus[core]        = READ_PHYREGCE(pi, RfctrlCoreTxPus, core);
		porig->RfctrlOverrideLpfSwtch[core] = READ_PHYREGCE(pi, RfctrlOverrideLpfSwtch,
		                                                    core);
		porig->RfctrlCoreLpfSwtch[core]     = READ_PHYREGCE(pi, RfctrlCoreLpfSwtch, core);
		porig->RfctrlOverrideAfeCfg[core]   = READ_PHYREGCE(pi, RfctrlOverrideAfeCfg, core);
		porig->RfctrlCoreAfeCfg1[core]      = READ_PHYREGCE(pi, RfctrlCoreAfeCfg1, core);
		porig->RfctrlCoreAfeCfg2[core]      = READ_PHYREGCE(pi, RfctrlCoreAfeCfg2, core);
		porig->RfctrlOverrideGains[core]    = READ_PHYREGCE(pi, RfctrlOverrideGains, core);
		porig->RfctrlCoreLpfGain[core]      = READ_PHYREGCE(pi, RfctrlCoreLpfGain, core);

		MOD_PHYREGCE(pi, RfctrlOverrideAuxTssi,  core, amux_sel_port, 1);
		MOD_PHYREGCE(pi, RfctrlCoreAuxTssi1,     core, amux_sel_port, 1);
		MOD_PHYREGCE(pi, RfctrlOverrideAuxTssi,  core, afe_iqadc_aux_en, 1);
		MOD_PHYREGCE(pi, RfctrlCoreAuxTssi1,     core, afe_iqadc_aux_en, 1);
		MOD_PHYREGCE(pi, RfctrlOverrideRxPus,    core, lpf_pu_dc, 1);
		MOD_PHYREGCE(pi, RfctrlCoreRxPus,        core, lpf_pu_dc, 0);
		MOD_PHYREGCE(pi, RfctrlOverrideTxPus,    core, lpf_bq1_pu, 1);
		MOD_PHYREGCE(pi, RfctrlCoreTxPus,        core, lpf_bq1_pu, 1);
		MOD_PHYREGCE(pi, RfctrlOverrideTxPus,    core, lpf_bq2_pu, 1);
		MOD_PHYREGCE(pi, RfctrlCoreTxPus,        core, lpf_bq2_pu, 0);
		MOD_PHYREGCE(pi, RfctrlOverrideTxPus,    core, lpf_pu, 1);
		MOD_PHYREGCE(pi, RfctrlCoreTxPus,        core, lpf_pu, 1);

		WRITE_PHYREGCE(pi, RfctrlCoreLpfSwtch, core,         0x154);
		WRITE_PHYREGCE(pi, RfctrlOverrideLpfSwtch, core,     0x3ff);

		MOD_PHYREGCE(pi, RfctrlCoreAfeCfg2, core, afe_iqadc_mode, sdadc_config & 0x7);
		MOD_PHYREGCE(pi, RfctrlOverrideAfeCfg, core, afe_iqadc_mode, 1);
		MOD_PHYREGCE(pi, RfctrlCoreAfeCfg1, core, afe_iqadc_pwrup,
		             (sdadc_config >> 3) & 0x3f);
		MOD_PHYREGCE(pi, RfctrlOverrideAfeCfg, core, afe_iqadc_pwrup, 1);
		MOD_PHYREGCE(pi, RfctrlCoreAfeCfg2, core, afe_iqadc_flashhspd,
		             (sdadc_config >> 9) & 0x1);
		MOD_PHYREGCE(pi, RfctrlOverrideAfeCfg, core, afe_iqadc_flashhspd, 1);
		MOD_PHYREGCE(pi, RfctrlCoreAfeCfg2, core, afe_ctrl_flash17lvl,
		             (sdadc_config >> 10) & 0x1);
		MOD_PHYREGCE(pi, RfctrlOverrideAfeCfg, core, afe_ctrl_flash17lvl, 1);
		MOD_PHYREGCE(pi, RfctrlCoreAfeCfg2, core, afe_iqadc_adc_bias,
		             (sdadc_config >> 11) & 0x3);
		MOD_PHYREGCE(pi, RfctrlOverrideAfeCfg, core, afe_iqadc_adc_bias,  1);

		MOD_PHYREGCE(pi, RfctrlCoreLpfGain,      core, lpf_bq1_gain, 0);
		MOD_PHYREGCE(pi, RfctrlOverrideGains,    core, lpf_bq1_gain, 1);
	}
}

static void
wlc_phy_tempsense_phy_cleanup_acphy(phy_info_t *pi)
{
	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;
	acphy_tempsense_phyregs_t *porig = (pi_ac->tempi->ac_tempsense_phyregs_orig);
	uint8 core;

	WRITE_PHYREG(pi, RxFeCtrl1, porig->RxFeCtrl1);

	FOREACH_ACTV_CORE(pi, pi->sh->phyrxchain, core) {
		WRITE_PHYREGCE(pi, RfctrlIntc, core, porig->RfctrlIntc[core]);

		WRITE_PHYREGCE(pi, RfctrlOverrideAuxTssi, core, porig->RfctrlOverrideAuxTssi[core]);
		WRITE_PHYREGCE(pi, RfctrlCoreAuxTssi1, core, porig->RfctrlCoreAuxTssi1[core]);
		WRITE_PHYREGCE(pi, RfctrlOverrideRxPus, core, porig->RfctrlOverrideRxPus[core]);
		WRITE_PHYREGCE(pi, RfctrlCoreRxPus, core, porig->RfctrlCoreRxPus[core]);
		WRITE_PHYREGCE(pi, RfctrlOverrideTxPus, core, porig->RfctrlOverrideTxPus[core]);
		WRITE_PHYREGCE(pi, RfctrlCoreTxPus, core, porig->RfctrlCoreTxPus[core]);
		WRITE_PHYREGCE(pi, RfctrlOverrideLpfSwtch, core,
		              porig->RfctrlOverrideLpfSwtch[core]);
		WRITE_PHYREGCE(pi, RfctrlCoreLpfSwtch, core, porig->RfctrlCoreLpfSwtch[core]);
		WRITE_PHYREGCE(pi, RfctrlOverrideAfeCfg, core, porig->RfctrlOverrideAfeCfg[core]);
		WRITE_PHYREGCE(pi, RfctrlCoreAfeCfg1, core, porig->RfctrlCoreAfeCfg1[core]);
		WRITE_PHYREGCE(pi, RfctrlCoreAfeCfg2, core, porig->RfctrlCoreAfeCfg2[core]);
		WRITE_PHYREGCE(pi, RfctrlOverrideGains, core, porig->RfctrlOverrideGains[core]);
		WRITE_PHYREGCE(pi, RfctrlCoreLpfGain, core, porig->RfctrlCoreLpfGain[core]);
	}
}

static int32
wlc_phy_tempsense_phy_setup_acphy_tiny(phy_info_t *pi, uint8 core)
{
	/* # this proc is used to configure the tempsense phy settings. */
	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;
	acphy_tempsense_phyregs_t *porig = (pi_ac->tempi->ac_tempsense_phyregs_orig);

	/* Applicable # foreach core */
	porig->RfctrlOverrideAuxTssi[core] = READ_PHYREGCE(pi, RfctrlOverrideAuxTssi, core);
	porig->RfctrlCoreAuxTssi1[core]    = READ_PHYREGCE(pi, RfctrlCoreAuxTssi1, core);
	porig->RfctrlOverrideRxPus[core]   = READ_PHYREGCE(pi, RfctrlOverrideRxPus, core);
	porig->RfctrlCoreRxPus[core]       = READ_PHYREGCE(pi, RfctrlCoreRxPus, core);
	porig->RxFeCtrl1                   = READ_PHYREG(pi, RxFeCtrl1);
	porig->RfctrlOverrideTxPus[core]   = READ_PHYREGCE(pi, RfctrlOverrideTxPus, core);
	porig->RfctrlCoreTxPus[core]       = READ_PHYREGCE(pi, RfctrlCoreTxPus, core);
	porig->RfctrlOverrideGains[core]   = READ_PHYREGCE(pi, RfctrlOverrideGains, core);
	porig->RfctrlCoreLpfGain[core]     = READ_PHYREGCE(pi, RfctrlCoreLpfGain, core);
	porig->RxSdFeConfig1               = READ_PHYREG(pi, RxSdFeConfig1);
	porig->RxSdFeConfig6               = READ_PHYREG(pi, RxSdFeConfig6);
	porig->RfctrlOverrideLpfSwtch[core] = READ_PHYREGCE(pi, RfctrlOverrideLpfSwtch, core);
	porig->RfctrlCoreLpfSwtch[core]    = READ_PHYREGCE(pi, RfctrlCoreLpfSwtch, core);
	porig->RfctrlOverrideAfeCfg[core]  = READ_PHYREGCE(pi, RfctrlOverrideAfeCfg, core);
	porig->RfctrlCoreAfeCfg1[core]     = READ_PHYREGCE(pi, RfctrlCoreAfeCfg1, core);
	porig->RfctrlCoreAfeCfg2[core]     = READ_PHYREGCE(pi, RfctrlCoreAfeCfg2, core);

	// Additional settings for 4365 core3 GPIO WAR
	if ((ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev)) &&
			(core == 3 || pi->phy_tempsense_option == 1)) {
		MOD_PHYREG(pi, DcFiltAddress,        dcBypass,     1);
		MOD_PHYREGCE(pi, RfctrlCoreRXGAIN1, core,  rxgain_dvga,  0);
		MOD_PHYREGCE(pi, RfctrlCoreLpfGain, core,  lpf_bq2_gain, 0);
		MOD_PHYREGCE(pi, RfctrlOverrideGains, core, rxgain,       1);
		MOD_PHYREGCE(pi, RfctrlOverrideGains, core, lpf_bq2_gain, 1);
	}

	MOD_PHYREGCE(pi, RfctrlOverrideAuxTssi,  core, amux_sel_port, 1);
	MOD_PHYREGCE(pi, RfctrlCoreAuxTssi1,	 core, amux_sel_port, 1);
	MOD_PHYREGCE(pi, RfctrlOverrideAuxTssi,  core, afe_iqadc_aux_en, 1);
	MOD_PHYREGCE(pi, RfctrlCoreAuxTssi1,	 core, afe_iqadc_aux_en, 1);
	MOD_PHYREGCE(pi, RfctrlOverrideRxPus,	 core, lpf_pu_dc, 1);
	MOD_PHYREGCE(pi, RfctrlCoreRxPus,		 core, lpf_pu_dc, 0);

	if (ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev)) {
		WRITE_PHYREGCE(pi, RfctrlCoreLpfSwtch, core,         0x154);
		WRITE_PHYREGCE(pi, RfctrlOverrideLpfSwtch, core,     0x3ff);
	}

	/* Power down LNAs - isolation of TIA power down is insufficient at high signal power */
	MOD_PHYREGCE(pi, RfctrlOverrideRxPus,	 core, rxrf_lna1_pwrup, 1);
	MOD_PHYREGCE(pi, RfctrlOverrideRxPus,	 core, rxrf_lna1_5G_pwrup, 1);
	MOD_PHYREGCE(pi, RfctrlCoreRxPus,	 core, rxrf_lna1_pwrup, 0);
	MOD_PHYREGCE(pi, RfctrlCoreRxPus,	 core, rxrf_lna1_5G_pwrup, 0);
	MOD_PHYREGCE(pi, RfctrlOverrideTxPus,    core, lpf_bq1_pu, 1);
	MOD_PHYREGCE(pi, RfctrlCoreTxPus,        core, lpf_bq1_pu, 1);
	MOD_PHYREGCE(pi, RfctrlOverrideTxPus,    core, lpf_bq2_pu, 1);
	MOD_PHYREGCE(pi, RfctrlCoreTxPus,        core, lpf_bq2_pu, 0);
	MOD_PHYREGCE(pi, RfctrlOverrideTxPus,    core, lpf_pu, 1);
	MOD_PHYREGCE(pi, RfctrlCoreTxPus,        core, lpf_pu, 1);
	MOD_PHYREGCE(pi, RfctrlOverrideGains,    core, lpf_bq1_gain, 1);
	MOD_PHYREGCE(pi, RfctrlCoreLpfGain,      core, lpf_bq1_gain, 0);
	if (ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev)) {
		MOD_PHYREG(pi, RxFeCtrl1, swap_iq0, 0);
		MOD_PHYREG(pi, RxFeCtrl1, swap_iq1, 0);
		MOD_PHYREG(pi, RxFeCtrl1, swap_iq2, 0);
		MOD_PHYREG(pi, RxFeCtrl1, swap_iq3, 0);
	} else {
		MOD_PHYREG(pi, RxFeCtrl1, swap_iq0, 0);
	}
	if (ACMAJORREV_4(pi->pubpi.phy_rev)) {
		MOD_PHYREG(pi, RxSdFeConfig1, farrow_rshift_tx, 1);
	} else if (ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev)) {
		MOD_PHYREG(pi, RxSdFeConfig1, farrow_rshift_tx, 0);
	}
	MOD_PHYREG(pi, RxSdFeConfig1, farrow_rshift_force, 1);
	MOD_PHYREG(pi, RxSdFeConfig6, rx_farrow_rshift_0,
		READ_PHYREGFLD(pi, RxSdFeConfig1, farrow_rshift_tx));

	return BCME_OK;
}

static int32
wlc_phy_tempsense_radio_setup_acphy_tiny(phy_info_t *pi, uint16 Av, uint16 Vmid, uint8 coreidx)
{
	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;
	tempsense_radioregs_tiny_t *porig =
		&(pi_ac->tempi->ac_tempsense_radioregs_orig->u.acphy_tempsense_radioregs_tiny);
	uint8 core, cores;
	if (ACMAJORREV_4(pi->pubpi.phy_rev) || ACMAJORREV_32(pi->pubpi.phy_rev) ||
		ACMAJORREV_33(pi->pubpi.phy_rev))
		cores = PHYCORENUM((pi)->pubpi.phy_corenum);
	else
		cores = 1;

	for (core = 0; core < cores; core++) {
		if ((ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev)) &&
			(core != coreidx))
			continue;

		porig->tempsense_cfg[core]     = READ_RADIO_REG_TINY(pi, TEMPSENSE_CFG, core);
		porig->tempsense_ovr1[core]    = READ_RADIO_REG_TINY(pi, TEMPSENSE_OVR1, core);
		porig->testbuf_cfg1[core]      = READ_RADIO_REG_TINY(pi, TESTBUF_CFG1, core);
		porig->testbuf_ovr1[core]      = READ_RADIO_REG_TINY(pi, TESTBUF_OVR1, core);
		porig->auxpga_cfg1[core]       = READ_RADIO_REG_TINY(pi, AUXPGA_CFG1, core);
		porig->auxpga_vmid[core]       = READ_RADIO_REG_TINY(pi, AUXPGA_VMID, core);
		porig->auxpga_ovr1[core]       = READ_RADIO_REG_TINY(pi, AUXPGA_OVR1, core);
		porig->tia_cfg5[core]          = READ_RADIO_REG_TINY(pi, TIA_CFG5, core);
		porig->tia_cfg7[core]          = READ_RADIO_REG_TINY(pi, TIA_CFG7, core);
		porig->tia_cfg9[core]          = READ_RADIO_REG_TINY(pi, TIA_CFG9, core);
		porig->adc_ovr1[core]          = READ_RADIO_REG_TINY(pi, ADC_OVR1, core);
		porig->adc_cfg10[core]         = READ_RADIO_REG_TINY(pi, ADC_CFG10, core);
		porig->rx_bb_2g_ovr_east[core] = READ_RADIO_REG_TINY(pi, RX_BB_2G_OVR_EAST, core);

		/* # Setup Tempsense */
		MOD_RADIO_REG_TINY(pi, TEMPSENSE_OVR1, core, ovr_tempsense_pu, 1);
		MOD_RADIO_REG_TINY(pi, TEMPSENSE_CFG, core, tempsense_pu, 1);

		/* # Setup Testbuf */
		MOD_RADIO_REG_TINY(pi, TESTBUF_OVR1, core, ovr_testbuf_PU, 1);
		MOD_RADIO_REG_TINY(pi, TESTBUF_CFG1, core, testbuf_PU, 1);
		MOD_RADIO_REG_TINY(pi, TESTBUF_CFG1, core, testbuf_GPIO_EN, 0);
		MOD_RADIO_REG_TINY(pi, TESTBUF_OVR1, core, ovr_testbuf_sel_test_port, 1);
		MOD_RADIO_REG_TINY(pi, TESTBUF_CFG1, core, testbuf_sel_test_port, 1);

		/* # Setup AuxPGA */
		MOD_RADIO_REG_TINY(pi, AUXPGA_OVR1, core, ovr_auxpga_i_pu, 1);
		MOD_RADIO_REG_TINY(pi, AUXPGA_CFG1, core, auxpga_i_pu, 1);
		MOD_RADIO_REG_TINY(pi, AUXPGA_OVR1, core, ovr_auxpga_i_sel_vmid, 1);

		MOD_RADIO_REG_TINY(pi, AUXPGA_VMID, core, auxpga_i_sel_vmid, Vmid);
		MOD_RADIO_REG_TINY(pi, AUXPGA_OVR1, core, ovr_auxpga_i_sel_gain, 1);
		MOD_RADIO_REG_TINY(pi, AUXPGA_CFG1, core, auxpga_i_sel_gain, Av);
		MOD_RADIO_REG_TINY(pi, AUXPGA_CFG1, core, auxpga_i_vcm_ctrl, 0);
		MOD_RADIO_REG_TINY(pi, AUXPGA_OVR1, core, ovr_auxpga_i_sel_input, 1);
		MOD_RADIO_REG_TINY(pi, AUXPGA_CFG1, core, auxpga_i_sel_input, 1);

		/* # Setup Aux Path */
		MOD_RADIO_REG_TINY(pi, TIA_CFG9, core, txbb_dac2adc, 0);
		MOD_RADIO_REG_TINY(pi, ADC_OVR1, core, ovr_adc_in_test, 1);
		MOD_RADIO_REG_TINY(pi, ADC_CFG10, core, adc_in_test, 0x3);
		MOD_RADIO_REG_TINY(pi, TIA_CFG5, core, tia_out_test, 0);

		/* # Turn off TIA otherwise it dominates the ADC input */
		MOD_RADIO_REG_TINY(pi, RX_BB_2G_OVR_EAST, core, ovr_tia_amp1_pwrup, 1);
		MOD_RADIO_REG_TINY(pi, TIA_CFG5, core, tia_amp1_pwrup, 0);
		MOD_RADIO_REG_TINY(pi, RX_BB_2G_OVR_EAST, core, ovr_tia_pwrup_amp2, 1);
		MOD_RADIO_REG_TINY(pi, TIA_CFG7, core, tia_pwrup_amp2, 0);
	}
	return BCME_OK;
}

static int32
wlc_phy_tempsense_poll_adc_war_tiny(phy_info_t *pi, bool init_adc_inside,
	int32 *measured_values, uint8 core)
{
	uint16 nsamp = 200;
	wlc_phy_tempsense_radio_swap_tiny(pi, ACPHY_TEMPSENSE_VBE, 0, core);
	measured_values[0] = wlc_phy_tempsense_poll_samps_tiny(pi, nsamp, init_adc_inside, core);

	wlc_phy_tempsense_radio_swap_tiny(pi, ACPHY_TEMPSENSE_VBG, 0, core);
	measured_values[1] = wlc_phy_tempsense_poll_samps_tiny(pi, nsamp, init_adc_inside, core);

	wlc_phy_tempsense_radio_swap_tiny(pi, ACPHY_TEMPSENSE_VBE, 1, core);
	measured_values[2] = wlc_phy_tempsense_poll_samps_tiny(pi, nsamp, init_adc_inside, core);

	wlc_phy_tempsense_radio_swap_tiny(pi, ACPHY_TEMPSENSE_VBG, 1, core);
	measured_values[3] = wlc_phy_tempsense_poll_samps_tiny(pi, nsamp, init_adc_inside, core);

	return BCME_OK;
}

static int32
wlc_phy_tempsense_poll_samps_tiny(phy_info_t *pi, uint16 samples, bool init_adc_inside, uint8 core)
{
	int32 measured_voltage;
	int32 i_sum = 0;
	int32 i_val = 0;
	int i = 0;
	uint16 mask, signcheck;
	uint32 signextend, discardbits;
	uint16 nsamps = 0;

	/* Need to set the swap bit, otherwise there is a bug */
	if (init_adc_inside) {
		if (!((ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev)) &&
			(core == 3 || pi->phy_tempsense_option == 1))) {
			wlc_phy_tempsense_gpiosel_tiny(pi, 16 + core, 1);
		}
	}
	if (ACMAJORREV_4(pi->pubpi.phy_rev) || ACMAJORREV_32(pi->pubpi.phy_rev) ||
		ACMAJORREV_33(pi->pubpi.phy_rev)) {
		/* 13 bit signed to 16 bit signed conversion. */
		mask = 0x1FFF;
		signcheck = 0x1000;
		signextend = 0xFFFFF000;
		discardbits = 3;
	} else {
		/* 12 bit signed to 16 bit signed conversion. */
		mask = 0x0FFF;
		signcheck = 0x0800;
		signextend = 0xFFFFF800;
		discardbits = 2;
	}

	OSL_DELAY(10);
	if ((ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev)) &&
			(core == 3 || pi->phy_tempsense_option == 1)) {
			phy_iq_est_t rx_iq_est[PHY_CORE_MAX];
			MOD_PHYREG(pi, RxSdFeConfig6, rx_farrow_rshift_0, 1);
			nsamps = 2*(1<<2*discardbits);
			wlc_phy_rx_iq_est_acphy(pi, rx_iq_est,	nsamps, 32, 0, TRUE);
			i_sum = (int32)	phy_utils_sqrt_int((uint32)(rx_iq_est[core].i_pwr)) *
				(ACMAJORREV_33(pi->pubpi.phy_rev)?1:4);
			measured_voltage = 0 - i_sum;
			//MOD_PHYREG(pi, RxSdFeConfig6, rx_farrow_rshift_0, 0);
	} else {
		for (i = 0; i < samples; i++) {
			i_val = READ_PHYREG(pi, gpioHiOut);
			i_val = (i_val & mask);
			if ((i_val & signcheck)) {
				i_val = (i_val | signextend);
			}
			i_sum += i_val;
		}
		measured_voltage = i_sum/samples;
	}
	return (measured_voltage >> discardbits);
}

static int32
wlc_phy_tempsense_gpiosel_tiny(phy_info_t *pi, uint16 sel, uint8 word_swap)
{
	if (!(ACMAJORREV_32(pi->pubpi.phy_rev) ||
			ACMAJORREV_33(pi->pubpi.phy_rev))) {
		W_REG(pi->sh->osh, &pi->regs->psm_gpio_oe, 0x0);
	}
	WRITE_PHYREG(pi, gpioLoOutEn, 0xFFFF);
	WRITE_PHYREG(pi, gpioHiOutEn, 0XFFFF);

	/* set up acphy GPIO sel */
	WRITE_PHYREG(pi, gpioSel, (word_swap<<8) | sel);
	return BCME_OK;
}

static int32
wlc_phy_tempsense_radio_swap_tiny(phy_info_t *pi, acphy_tempsense_cfg_opt_t type,
	uint8 swap, uint8 coreidx)
{
	uint8 core, cores;
	if (ACMAJORREV_4(pi->pubpi.phy_rev) || ACMAJORREV_32(pi->pubpi.phy_rev) ||
		ACMAJORREV_33(pi->pubpi.phy_rev))
		cores = PHYCORENUM((pi)->pubpi.phy_corenum);
	else
		cores = 1;

	for (core = 0; core < cores; core++) {
		if ((ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev)) &&
		(core != coreidx))
			continue;
		/* Enable override */
		MOD_RADIO_REG_TINY(pi, TEMPSENSE_OVR1, core, ovr_tempsense_sel_Vbe_Vbg, 1);
		MOD_RADIO_REG_TINY(pi, TEMPSENSE_OVR1, core, ovr_tempsense_swap_amp, 1);

		if (swap == 0) {
			MOD_RADIO_REG_TINY(pi, TEMPSENSE_CFG, core, tempsense_swap_amp, 0);
		} else if (swap == 1) {
			MOD_RADIO_REG_TINY(pi, TEMPSENSE_CFG, core, tempsense_swap_amp, 1);
		} else {
			PHY_ERROR(("Unsupported, swap should be 0 or 1\n"));
			return BCME_ERROR;
		}
		if (type == ACPHY_TEMPSENSE_VBG) {
			MOD_RADIO_REG_TINY(pi, TEMPSENSE_CFG, core, tempsense_sel_Vbe_Vbg, 0);
		} else if (type == ACPHY_TEMPSENSE_VBE) {
			MOD_RADIO_REG_TINY(pi, TEMPSENSE_CFG, core, tempsense_sel_Vbe_Vbg, 1);
		} else {
			PHY_ERROR(("Unsupported, supported types are"));
			PHY_ERROR((" ACPHY_TEMPSENSE_VBE/ACPHY_TEMPSENSE_VBG\n"));
			return BCME_ERROR;
		}
	}
	return BCME_OK;
}

static int32
wlc_phy_tempsense_phy_cleanup_acphy_tiny(phy_info_t *pi, uint8 core)
{
	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;
	acphy_tempsense_phyregs_t *porig = (pi_ac->tempi->ac_tempsense_phyregs_orig);

	// Additional settings for 4365 core3 GPIO WAR
	if ((ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev)) &&
		(core == 3 || pi->phy_tempsense_option == 1)) {
		MOD_PHYREG(pi, DcFiltAddress,        dcBypass,     0);
		MOD_PHYREGCE(pi, RfctrlOverrideGains, core, rxgain,       0);
		MOD_PHYREGCE(pi, RfctrlOverrideGains, core, lpf_bq2_gain, 0);
	}

	/* # restore T/R and external PA states */
	WRITE_PHYREG(pi, RxFeCtrl1, porig->RxFeCtrl1);
	WRITE_PHYREG(pi, RxSdFeConfig6, porig->RxSdFeConfig6);
	WRITE_PHYREG(pi, RxSdFeConfig1, porig->RxSdFeConfig1);
	WRITE_PHYREGCE(pi, RfctrlIntc, core, porig->RfctrlIntc[core]);
	WRITE_PHYREGCE(pi, RfctrlOverrideAuxTssi, core, porig->RfctrlOverrideAuxTssi[core]);
	WRITE_PHYREGCE(pi, RfctrlCoreAuxTssi1, core, porig->RfctrlCoreAuxTssi1[core]);
	WRITE_PHYREGCE(pi, RfctrlOverrideRxPus, core, porig->RfctrlOverrideRxPus[core]);
	WRITE_PHYREGCE(pi, RfctrlCoreRxPus, core, porig->RfctrlCoreRxPus[core]);
	WRITE_PHYREGCE(pi, RfctrlOverrideTxPus, core, porig->RfctrlOverrideTxPus[core]);
	WRITE_PHYREGCE(pi, RfctrlCoreTxPus, core, porig->RfctrlCoreTxPus[core]);
	WRITE_PHYREGCE(pi, RfctrlOverrideLpfSwtch, core, porig->RfctrlOverrideLpfSwtch[core]);
	WRITE_PHYREGCE(pi, RfctrlCoreLpfSwtch, core, porig->RfctrlCoreLpfSwtch[core]);
	WRITE_PHYREGCE(pi, RfctrlOverrideAfeCfg, core, porig->RfctrlOverrideAfeCfg[core]);
	WRITE_PHYREGCE(pi, RfctrlCoreAfeCfg1, core, porig->RfctrlCoreAfeCfg1[core]);
	WRITE_PHYREGCE(pi, RfctrlCoreAfeCfg2, core, porig->RfctrlCoreAfeCfg2[core]);
	WRITE_PHYREGCE(pi, RfctrlOverrideGains, core, porig->RfctrlOverrideGains[core]);
	WRITE_PHYREGCE(pi, RfctrlCoreLpfGain, core, porig->RfctrlCoreLpfGain[core]);
	return BCME_OK;
}

static int32
wlc_phy_tempsense_radio_cleanup_acphy_tiny(phy_info_t *pi, uint8 coreidx)
{
	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;
	tempsense_radioregs_tiny_t *porig =
		&(pi_ac->tempi->ac_tempsense_radioregs_orig->u.acphy_tempsense_radioregs_tiny);
	uint8 core, cores;
	if (ACMAJORREV_4(pi->pubpi.phy_rev) || ACMAJORREV_32(pi->pubpi.phy_rev) ||
		ACMAJORREV_33(pi->pubpi.phy_rev))
		cores = PHYCORENUM((pi)->pubpi.phy_corenum);
	else
		cores = 1;

	for (core = 0; core < cores; core++) {
		if ((ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev)) &&
			(core != coreidx))
			continue;
		/* # Get back old values */
		phy_utils_write_radioreg(pi, RADIO_REG(pi, TEMPSENSE_OVR1, core),
			porig->tempsense_ovr1[core]);
		phy_utils_write_radioreg(pi, RADIO_REG(pi, TEMPSENSE_CFG, core),
			porig->tempsense_cfg[core]);
		phy_utils_write_radioreg(pi, RADIO_REG(pi, TESTBUF_OVR1, core),
			porig->testbuf_ovr1[core]);
		phy_utils_write_radioreg(pi, RADIO_REG(pi, TESTBUF_CFG1, core),
			porig->testbuf_cfg1[core]);
		phy_utils_write_radioreg(pi, RADIO_REG(pi, AUXPGA_OVR1, core),
			porig->auxpga_ovr1[core]);
		phy_utils_write_radioreg(pi, RADIO_REG(pi, AUXPGA_CFG1, core),
			porig->auxpga_cfg1[core]);
		phy_utils_write_radioreg(pi, RADIO_REG(pi, AUXPGA_VMID, core),
			porig->auxpga_vmid[core]);
		phy_utils_write_radioreg(pi, RADIO_REG(pi, TIA_CFG9, core),
			porig->tia_cfg9[core]);
		phy_utils_write_radioreg(pi, RADIO_REG(pi, ADC_OVR1, core),
			porig->adc_ovr1[core]);
		phy_utils_write_radioreg(pi, RADIO_REG(pi, ADC_CFG10, core),
			porig->adc_cfg10[core]);
		phy_utils_write_radioreg(pi, RADIO_REG(pi, TIA_CFG5, core),
			porig->tia_cfg5[core]);
		phy_utils_write_radioreg(pi, RADIO_REG(pi, RX_BB_2G_OVR_EAST, core),
			porig->rx_bb_2g_ovr_east[core]);
		phy_utils_write_radioreg(pi, RADIO_REG(pi, TIA_CFG7, core),
			porig->tia_cfg7[core]);
		MOD_RADIO_REG_TINY(pi, TEMPSENSE_OVR1, core, ovr_tempsense_pu, 1);
		MOD_RADIO_REG_TINY(pi, TEMPSENSE_CFG, core, tempsense_pu, 0);

	}
	return BCME_OK;
}

/* ********************************************* */
/*				External Definitions					*/
/* ********************************************* */
int16
wlc_phy_tempsense_acphy(phy_info_t *pi)
{
	uint8 core, core_cnt = 0;
	uint8 sel_Vb, swap_amp;
	int8 idx;
	uint16 auxPGA_Av = 0x3, auxPGA_Vmid = 0x91;
	int16 V[4][3] = {{0, 0}}, Vout[4] = {0, 0, 0, 0};
	int16 offset = (int16) pi->phy_tempsense_offset;
	int32 radio_temp = 0;
	int32 t_scale = 16384;
	int32 t_slope = 8766;
	int32 t_offset = 1902100;
	int32 avbq_scale = 256;
	int32 avbq_slope[3] = {527, 521, 522};
	uint16 save_afePuCtrl = 0, save_gpio = 0, save_gpioHiOutEn = 0;
	uint16 fval2g_orig, fval5g_orig, fval2g, fval5g;
	uint32 save_chipc = 0;
	uint8  stall_val = 0, log2_nsamps = 3;
	int32 k, tmp_samp, samp_accum;

	/* return dummy temp */
	if (NORADIO_ENAB(pi->pubpi)) {
		return 25;
	}

	if (TINY_RADIO(pi)) {
		wlc_phy_tempsense_acphy_tiny(pi);
		return pi->u.pi_acphy->current_temperature;
	}

	/* Prepare Mac and Phregs */
	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	phy_utils_phyreg_enter(pi);

	if (ACMAJORREV_1(pi->pubpi.phy_rev)) {
		auxPGA_Av = 0x3;
		auxPGA_Vmid = 0x91;
	}

	wlc_phy_tempsense_phy_setup_acphy(pi);
	wlc_phy_tempsense_radio_setup_acphy(pi, auxPGA_Av, auxPGA_Vmid);
		wlc_phy_init_adc_read(pi, &save_afePuCtrl, &save_gpio,
			&save_chipc, &fval2g_orig, &fval5g_orig,
			&fval2g, &fval5g, &stall_val, &save_gpioHiOutEn);
	wlc_phy_pulse_adc_reset_acphy(pi);

	FOREACH_ACTV_CORE(pi, pi->sh->phyrxchain, core) {
	  wlc_phy_gpiosel_acphy(pi, 16+core, 1);
	  core_cnt++;
	  for (idx = 0; idx < 4; idx++) {
	    switch (idx) {
	    case 0:
	      sel_Vb = 1;
	      swap_amp = 0;
		      break;
	    case 1:
	      sel_Vb = 0;
	      swap_amp = 0;
	      break;
	    case 2:
	      sel_Vb = 1;
	      swap_amp = 1;
	      break;
	    case 3:
	      sel_Vb = 0;
	      swap_amp = 1;
	      break;
	    default:
	      sel_Vb = 0;
	      swap_amp = 0;
	      break;
	    }

	    /* Reg conflict with 2069 rev 16 */
	    if (RADIOMAJORREV(pi) == 0) {
		MOD_RADIO_REGC(pi, OVR18, core, ovr_tempsense_sel_Vbe_Vbg, 0x1);
	    } else {
		MOD_RADIO_REGC(pi, GE16_OVR19, core, ovr_tempsense_sel_Vbe_Vbg, 0x1);
	    }
	    MOD_RADIO_REGC(pi, TEMPSENSE_CFG, core, tempsense_sel_Vbe_Vbg, sel_Vb);

	    /* Reg conflict with 2069 rev 16 */
	    if (RADIOMAJORREV(pi) == 0) {
		MOD_RADIO_REGC(pi, OVR18, core, ovr_tempsense_swap_amp, 0x1);
	    } else {
		MOD_RADIO_REGC(pi, GE16_OVR19, core, ovr_tempsense_swap_amp, 0x1);
	    }
	    MOD_RADIO_REGC(pi, TEMPSENSE_CFG, core, swap_amp, swap_amp);

	    OSL_DELAY(10);
	    samp_accum = 0;
	    for (k = 0; k < (1 << log2_nsamps); k++) {
	      /* read out the i-value */
	      tmp_samp = READ_PHYREG(pi, gpioHiOut) >> 2;
	      tmp_samp -= (tmp_samp < 512) ? 0 : 1024;
	      samp_accum += tmp_samp;
	    }
	    V[idx][core] = (int16) (samp_accum >> log2_nsamps);
	  }
	}

	if ((RADIOMAJORREV(pi) == 1) ||
	    (RADIOMAJORREV(pi) == 2)) {

		if (RADIOMAJORREV(pi) == 1) {
			t_slope 		= -5453;
			t_offset		= 1748881;
		} else if (RADIOMAJORREV(pi) == 2) {
			t_slope 		= -5549;
			t_offset		= 1697118;
		}

		avbq_scale 		= 800;
		avbq_slope[0]	        = 1024;
		avbq_slope[1] = 1024;
		t_scale			= 16384;

		FOREACH_ACTV_CORE(pi, pi->sh->phyrxchain, core) {
	        radio_temp += (int32)(((V[0][core] + V[2][core] - V[1][core] - V[3][core]) / 2))
		                      * ((t_slope * avbq_scale) / (avbq_slope[core] * core_cnt));
			Vout[0] += V[0][core] / core_cnt;
			Vout[1] += V[1][core] / core_cnt;
			Vout[2] += V[2][core] / core_cnt;
			Vout[3] += V[3][core] / core_cnt;

		}
		radio_temp = (radio_temp + t_offset)/t_scale;

		/* Forcing offset to zero as this is not characterized yet for 4335 */
		offset = 0;

	} else {
		FOREACH_ACTV_CORE(pi, pi->sh->phyrxchain, core) {
			radio_temp += (int32)(V[1][core] + V[3][core] - V[0][core] - V[2][core])
					  * t_slope / 2 * avbq_scale / avbq_slope[core] / core_cnt;
			Vout[0] += V[0][core] / core_cnt;
			Vout[1] += V[1][core] / core_cnt;
			Vout[2] += V[2][core] / core_cnt;
			Vout[3] += V[3][core] / core_cnt;
		}
		radio_temp = (radio_temp + t_offset) / t_scale;
	}

	PHY_THERMAL(("Tempsense\n\tAuxADC0 Av,Vmid = 0x%x,0x%x\n",
	             auxPGA_Av, auxPGA_Vmid));
	PHY_THERMAL(("\tVref1,Vref2,Vctat1,Vctat2 =%d,%d,%d,%d\n",
	             Vout[0], Vout[2], Vout[1], Vout[3]));
	PHY_THERMAL(("\t^C Formula: (%d*(Vctat1+Vctat2-Vref1-Vref2)/2*800/1024+%d)/%d\n",
	             t_slope, t_offset, t_scale));
	PHY_THERMAL(("\t^C = %d, applied offset = %d\n",
	             radio_temp, offset));

	wlc_phy_tempsense_phy_cleanup_acphy(pi);
	wlc_phy_tempsense_radio_cleanup_acphy(pi);
		wlc_phy_restore_after_adc_read(pi,  &save_afePuCtrl, &save_gpio,
			&save_chipc,  &fval2g_orig,  &fval5g_orig,
			&fval2g,  &fval5g, &stall_val, &save_gpioHiOutEn);
	phy_utils_phyreg_exit(pi);
	wlapi_enable_mac(pi->sh->physhim);

	/* Store temperature and return value */
	pi->u.pi_acphy->current_temperature = (int16) radio_temp + offset;

#ifdef ATE_BUILD
	ate_buffer_regval.curr_radio_temp = pi->u.pi_acphy->current_temperature;
#endif // endif

	return ((int16) radio_temp + offset);
}

int16
wlc_phy_tempsense_acphy_tiny(phy_info_t *pi)
{
	/*
	 * # Description of mode = single:
	 * #	- used to digitally poll the radio's temperature sensors
	 * #	- does an absolute measurement by toggling the flip bit
	 * #	- saves and restores previous register values
	 * #	- returns per core values in degrees Celsius
	 * #
	 * # Description of mode = "scope": *** FIXME: not implemented ***
	 * #	- used to monitor the radio's temperature sensor on the scope by
	 * #	  means of the analog pwrdet signal
	 * #	- changes the state of the muxes and powers up the sensor
	 * # --------------------------------------------------------------------
	 */

	uint16 auxPGA_Av = 0x3, auxPGA_Vmid = 0x91;
	int64 radio_temp = 0;
	int32 t_scale = 16384;
	int32 t_slope = -10246;
	int32 t_offset = 1765868;
	int32 avbq_scale = 800;
	int32 avbq_slope = 1024;
	uint16 save_afePuCtrl = 0, save_gpio = 0, save_gpioHiOutEn = 0;
	uint16 fval2g_orig, fval5g_orig, fval2g, fval5g;
	uint32 save_chipc = 0;
	int32 measured_voltage[4] = {0};
	uint8  stall_val = 0;
	uint8 core = 0;

	if (ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev)) {
		avbq_scale = 1200;
		core = phy_get_first_actv_core(pi->sh->phyrxchain);
		ASSERT(core < PHY_CORE_MAX);
		if (core < 3 && pi->phy_tempsense_option != 1) {
			auxPGA_Av = 0x5, auxPGA_Vmid = 0x7D;
			t_slope = -3038; t_offset = 1621378;
		} else if (pi->phy_tempsense_option == 1) {
			auxPGA_Av = 0x1, auxPGA_Vmid = 0xEB;
			if (pi->fabid == BCM4366_TSMC_MIL12_AU_ID) {
				t_slope = -5530; t_offset = 1765879;
			} else {
				t_slope = -6574; t_offset = 1610716;
			}
		} else {
			auxPGA_Av = 0x5, auxPGA_Vmid = 0x9B;
			t_slope = -3795; t_offset = 1606261+2*t_scale;
		}
	}
	/*
	 * # If mac is suspended, leave it suspended and don't touch the state of the MAC
	 * # If not, suspend at the beginning of tempsense and resume it at the end.
	 * # (Suspending is required - as we arereading via muxes that are pin-contolled
	 * # during normal RX & TX.)
	 */
	/* Prepare Mac and Phregs */
	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	phy_utils_phyreg_enter(pi);

	wlc_phy_tempsense_phy_setup_acphy_tiny(pi, core);
	wlc_phy_tempsense_radio_setup_acphy_tiny(pi, auxPGA_Av, auxPGA_Vmid, core);
	wlc_phy_init_adc_read(pi, &save_afePuCtrl, &save_gpio,
	                          &save_chipc, &fval2g_orig, &fval5g_orig,
	                          &fval2g, &fval5g, &stall_val, &save_gpioHiOutEn);
	wlc_phy_tempsense_poll_adc_war_tiny(pi, TRUE, measured_voltage, core);

	radio_temp += (int64)(((measured_voltage[0] + measured_voltage[2]
		- measured_voltage[1] - measured_voltage[3]) / 2)
		* (int64) t_slope * avbq_scale) / avbq_slope;
	radio_temp = (radio_temp + t_offset)/t_scale;

	wlc_phy_tempsense_phy_cleanup_acphy_tiny(pi, core);
	wlc_phy_tempsense_radio_cleanup_acphy_tiny(pi, core);
	wlc_phy_restore_after_adc_read(pi,	&save_afePuCtrl, &save_gpio,
		&save_chipc,  &fval2g_orig,  &fval5g_orig,
		&fval2g,  &fval5g, &stall_val, &save_gpioHiOutEn);
	phy_utils_phyreg_exit(pi);

	/* # RESUME MAC as soon as we are done reading/writing regs and muxes
	 * # -----------------------------------------------------------------
	 */
	wlapi_enable_mac(pi->sh->physhim);

	/* Store temperature and return value */
	pi->u.pi_acphy->current_temperature = (int16) radio_temp;

#ifdef ATE_BUILD
	ate_buffer_regval.curr_radio_temp = (int16) radio_temp;
#endif // endif

	return ((int16)radio_temp);
}
void
phy_ac_update_tempsense_bitmap(phy_info_t *pi)
{
	phy_info_t *other_pi = phy_get_other_pi(pi);
	phy_temp_info_t *tempi = pi->tempi;
	phy_txcore_temp_t *temp_status = phy_temp_get_st(tempi);

	temp_status->bitmap = pi->pubpi.phy_coremask;
	temp_status->bitmap |= (temp_status->bitmap << 4);

	if (phy_get_phymode(pi) == PHYMODE_RSDB) {
		tempi = other_pi->tempi;
		temp_status = phy_temp_get_st(tempi);
		temp_status->bitmap = other_pi->pubpi.phy_coremask;
		temp_status->bitmap |= (temp_status->bitmap << 4);
	}
}
