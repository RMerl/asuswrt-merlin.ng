/*
 * NPHY ANAcore contorl module implementation
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
#include <phy_ana.h>
#include "phy_type_ana.h"
#include <phy_n.h>
#include <phy_n_ana.h>

#include <wlc_phyreg_n.h>
#include <phy_utils_reg.h>

/* module private states */
struct phy_n_ana_info {
	phy_info_t *pi;
	phy_n_info_t *ni;
	phy_ana_info_t *ani;
};

/* local functions */
static int phy_n_ana_switch(phy_type_ana_ctx_t *ctx, bool on);
static void phy_n_ana_reset(phy_type_ana_ctx_t *ctx);

/* Register/unregister NPHY specific implementation to common layer */
phy_n_ana_info_t *
BCMATTACHFN(phy_n_ana_register_impl)(phy_info_t *pi, phy_n_info_t *ni,
	phy_ana_info_t *ani)
{
	phy_n_ana_info_t *info;
	phy_type_ana_fns_t fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage in once */
	if ((info = phy_malloc(pi, sizeof(phy_n_ana_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	bzero(info, sizeof(phy_n_ana_info_t));
	info->pi = pi;
	info->ni = ni;
	info->ani = ani;

#ifndef BCM_OL_DEV
	phy_n_ana_switch(info, ON);
#endif // endif

	/* Register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.ctrl = phy_n_ana_switch;
	fns.reset = phy_n_ana_reset;
	fns.ctx = info;

	phy_ana_register_impl(ani, &fns);

	return info;
fail:
	if (info != NULL)
		phy_mfree(pi, info, sizeof(phy_n_ana_info_t));
	return NULL;
}

void
BCMATTACHFN(phy_n_ana_unregister_impl)(phy_n_ana_info_t *info)
{
	phy_info_t *pi = info->pi;
	phy_ana_info_t *ani = info->ani;

	phy_ana_unregister_impl(ani);

	phy_mfree(pi, info, sizeof(phy_n_ana_info_t));
}

/* switch anacore on/off */
static int
phy_n_ana_switch(phy_type_ana_ctx_t *ctx, bool on)
{
	phy_n_ana_info_t *info = (phy_n_ana_info_t *)ctx;
	phy_info_t *pi = info->pi;

	PHY_TRACE(("%s: %d\n", __FUNCTION__, on));

	/* Below code causes driver hang randomly for 4324 */
	/* Enable after root causing */
	if (NREV_GE(pi->pubpi.phy_rev, LCNXN_BASEREV + 3))
		return BCME_OK;

	if (on) {
		if (NREV_GE(pi->pubpi.phy_rev, 3)) {
			if (NREV_GE(pi->pubpi.phy_rev, LCNXN_BASEREV + 3)) {
				PHY_REG_LIST_START
					PHY_REG_WRITE_ENTRY(NPHY, AfectrlCore1,     0x0d)
					PHY_REG_WRITE_ENTRY(NPHY, AfectrlOverride1, 0x0)
				        PHY_REG_WRITE_ENTRY(NPHY, AfectrlCore2,     0x0d)
				        PHY_REG_WRITE_ENTRY(NPHY, AfectrlOverride2, 0x0)
				PHY_REG_LIST_EXECUTE(pi);
				wlc_phy_nphy_afectrl_override(pi, NPHY_ADC_PD, 0, 0, 0x3);
				PHY_REG_LIST_START
					PHY_REG_MOD_RAW_ENTRY(NPHY_AfectrlOverride1,
					                      NPHY_REV3_AfectrlOverride_adc_pd_MASK,
					                      NPHY_REV3_AfectrlOverride_adc_pd_MASK)
					PHY_REG_MOD_RAW_ENTRY(NPHY_AfectrlOverride2,
					                      NPHY_REV3_AfectrlOverride_adc_pd_MASK,
					                      NPHY_REV3_AfectrlOverride_adc_pd_MASK)
					PHY_REG_MOD_RAW_ENTRY(NPHY_AfectrlCore1,
					                      NPHY_REV19_AfectrlCore_adc_pd_MASK,
					                      0)
					PHY_REG_MOD_RAW_ENTRY(NPHY_AfectrlCore2,
					                      NPHY_REV19_AfectrlCore_adc_pd_MASK,
					                      0)
				PHY_REG_LIST_EXECUTE(pi);
				/* dac pd */
				wlc_phy_nphy_afectrl_override(pi, NPHY_DAC_PD, 0, 0, 0x3);
				/* aux_en */
				wlc_phy_rfctrl_override_nphy_rev19(pi,
				                NPHY_REV19_RfctrlOverride_aux_en_MASK,
				                0, 0, 1, NPHY_REV19_RFCTRLOVERRIDE_ID4);

			} else {
				PHY_REG_LIST_START
					PHY_REG_WRITE_ENTRY(NPHY, AfectrlCore1,     0x0d)
					PHY_REG_WRITE_ENTRY(NPHY, AfectrlOverride1, 0x0)
					PHY_REG_WRITE_ENTRY(NPHY, AfectrlCore2,     0x0d)
					PHY_REG_WRITE_ENTRY(NPHY, AfectrlOverride2, 0x0)
				PHY_REG_LIST_EXECUTE(pi);
			}
		} else {
			phy_utils_write_phyreg(pi, NPHY_AfectrlOverride, 0x0);
		}
	} else {
		if (NREV_GE(pi->pubpi.phy_rev, 3)) {
			if (NREV_GE(pi->pubpi.phy_rev, LCNXN_BASEREV + 3)) {
				PHY_REG_LIST_START
					PHY_REG_WRITE_ENTRY(NPHY, AfectrlOverride1, 0x07ff)
					PHY_REG_WRITE_ENTRY(NPHY, AfectrlCore1,     0x0fd)
					PHY_REG_WRITE_ENTRY(NPHY, AfectrlOverride2, 0x07ff)
					PHY_REG_WRITE_ENTRY(NPHY, AfectrlCore2,     0x0fd)
				PHY_REG_LIST_EXECUTE(pi);
				wlc_phy_nphy_afectrl_override(pi, NPHY_ADC_PD, 1, 0, 0x3);
				PHY_REG_LIST_START
					PHY_REG_MOD_RAW_ENTRY(NPHY_AfectrlOverride1,
					                      NPHY_REV3_AfectrlOverride_adc_pd_MASK,
					                      NPHY_REV3_AfectrlOverride_adc_pd_MASK)
					PHY_REG_MOD_RAW_ENTRY(NPHY_AfectrlOverride2,
					                      NPHY_REV3_AfectrlOverride_adc_pd_MASK,
					                      NPHY_REV3_AfectrlOverride_adc_pd_MASK)
					PHY_REG_MOD_RAW_ENTRY(NPHY_AfectrlCore1,
					                      NPHY_REV19_AfectrlCore_adc_pd_MASK,
					                      NPHY_REV19_AfectrlCore_adc_pd_MASK)
					PHY_REG_MOD_RAW_ENTRY(NPHY_AfectrlCore2,
					                      NPHY_REV19_AfectrlCore_adc_pd_MASK,
					                      NPHY_REV19_AfectrlCore_adc_pd_MASK)
				PHY_REG_LIST_EXECUTE(pi);
				wlc_phy_nphy_afectrl_override(pi, NPHY_DAC_PD, 1, 0, 0x3);
				/* aux_en */
				wlc_phy_rfctrl_override_nphy_rev19(pi,
				                NPHY_REV19_RfctrlOverride_aux_en_MASK,
				                0, 0, 0, NPHY_REV19_RFCTRLOVERRIDE_ID4);
			} else {

				PHY_REG_LIST_START
					PHY_REG_WRITE_ENTRY(NPHY, AfectrlOverride1, 0x07ff)
					PHY_REG_WRITE_ENTRY(NPHY, AfectrlCore1,     0x0fd)
					PHY_REG_WRITE_ENTRY(NPHY, AfectrlOverride2, 0x07ff)
					PHY_REG_WRITE_ENTRY(NPHY, AfectrlCore2,     0x0fd)
				PHY_REG_LIST_EXECUTE(pi);
			}
		} else {
			phy_utils_write_phyreg(pi, NPHY_AfectrlOverride, 0x7fff);
		}
	}

	return BCME_OK;
}

/* reset h/w */
static void
phy_n_ana_reset(phy_type_ana_ctx_t *ctx)
{
	phy_n_ana_switch(ctx, ON);
}
