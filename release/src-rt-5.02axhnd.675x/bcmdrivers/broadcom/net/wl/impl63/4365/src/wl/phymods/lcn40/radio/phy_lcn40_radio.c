/*
 * LCN40PHY RADIO contorl module implementation
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
#include <phy_radio.h>
#include "phy_type_radio.h"
#include <phy_lcn40.h>
#include <phy_lcn40_radio.h>

#include <wlc_phyreg_lcn40.h>
#include <wlc_phy_radio.h>
#include <phy_utils_reg.h>

/* module private states */
struct phy_lcn40_radio_info {
	phy_info_t *pi;
	phy_lcn40_info_t *lcn40i;
	phy_radio_info_t *ri;
};

/* local functions */
static void _phy_lcn40_radio_switch(phy_type_radio_ctx_t *ctx, bool on);
static void phy_lcn40_radio_on(phy_type_radio_ctx_t *ctx);
static void phy_lcn40_radio_switch_band(phy_type_radio_ctx_t *ctx);
static uint32 _phy_lcn40_radio_query_idcode(phy_type_radio_ctx_t *ctx);

/* Register/unregister LCN40PHY specific implementation to common layer. */
phy_lcn40_radio_info_t *
BCMATTACHFN(phy_lcn40_radio_register_impl)(phy_info_t *pi, phy_lcn40_info_t *lcn40i,
	phy_radio_info_t *ri)
{
	phy_lcn40_radio_info_t *info;
	phy_type_radio_fns_t fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage in once */
	if ((info = phy_malloc(pi, sizeof(phy_lcn40_radio_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	bzero(info, sizeof(phy_lcn40_radio_info_t));
	info->pi = pi;
	info->lcn40i = lcn40i;
	info->ri = ri;

	pi->pubpi.radiooffset = RADIO_2065_READ_OFF;

#ifndef BCM_OL_DEV
	/* make sure the radio is off until we do an "up" */
	_phy_lcn40_radio_switch(info, OFF);
#endif // endif

	/* Register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.ctrl = _phy_lcn40_radio_switch;
	fns.on = phy_lcn40_radio_on;
	fns.bandx = phy_lcn40_radio_switch_band;
	fns.id = _phy_lcn40_radio_query_idcode;
	fns.ctx = info;

	phy_radio_register_impl(ri, &fns);

	return info;

fail:
	if (info != NULL)
		phy_mfree(pi, info, sizeof(phy_lcn40_radio_info_t));
	return NULL;
}

void
BCMATTACHFN(phy_lcn40_radio_unregister_impl)(phy_lcn40_radio_info_t *info)
{
	phy_info_t *pi = info->pi;
	phy_radio_info_t *ri = info->ri;

	phy_radio_unregister_impl(ri);

	phy_mfree(pi, info, sizeof(phy_lcn40_radio_info_t));
}

/* switch radio on/off */
static void
_phy_lcn40_radio_switch(phy_type_radio_ctx_t *ctx, bool on)
{
	phy_lcn40_radio_info_t *info = (phy_lcn40_radio_info_t *)ctx;
	phy_info_t *pi = info->pi;

	PHY_TRACE(("wl%d: %s %d\n", pi->sh->unit, __FUNCTION__, on));

	/* reset rxafe before switching radio */
	/* This reset is removed elsewhere */
	if ((LCN40REV_LE(pi->pubpi.phy_rev, 5)) || (LCN40REV_IS(pi->pubpi.phy_rev, 7)))
		phy_utils_or_phyreg(pi, LCN40PHY_resetCtrl, 0x0047);

	if (on) {
		PHY_REG_LIST_START
			PHY_REG_AND_RAW_ENTRY(LCN40PHY_RFOverride0,
				~(LCN40PHY_RFOverride0_rfpll_pu_ovr_MASK	|
				LCN40PHY_RFOverride0_internalrfrxpu_ovr_MASK 	|
				LCN40PHY_RFOverride0_internalrftxpu_ovr_MASK |
				LCN40PHY_RFOverride0_gmode_tx_pu_ovr_MASK |
				LCN40PHY_RFOverride0_gmode_rx_pu_ovr_MASK |
				LCN40PHY_RFOverride0_amode_tx_pu_ovr_MASK |
				LCN40PHY_RFOverride0_amode_rx_pu_ovr_MASK))

			PHY_REG_AND_RAW_ENTRY(LCN40PHY_rfoverride8,
				~(LCN40PHY_rfoverride8_rxrf_wrssi1_pwrup_ovr_MASK |
				LCN40PHY_rfoverride8_rxrf_wrssi2_pwrup_ovr_MASK |
				LCN40PHY_rfoverride8_lna2_pu_ovr_MASK))

			PHY_REG_AND_RAW_ENTRY(LCN40PHY_rfoverride2,
				~(LCN40PHY_rfoverride2_slna_pu_ovr_MASK))

			PHY_REG_AND_RAW_ENTRY(LCN40PHY_rfoverride3,
				~LCN40PHY_rfoverride3_rfactive_ovr_MASK)
		PHY_REG_LIST_EXECUTE(pi);
	} else {
		/* Reset the fast band switch flag */
		pi->fast_bs = 0;
		PHY_REG_LIST_START
			PHY_REG_AND_RAW_ENTRY(LCN40PHY_RFOverrideVal0,
				~(LCN40PHY_RFOverrideVal0_rfpll_pu_ovr_val_MASK |
				LCN40PHY_RFOverrideVal0_internalrfrxpu_ovr_val_MASK |
				LCN40PHY_RFOverrideVal0_internalrftxpu_ovr_val_MASK |
				LCN40PHY_RFOverrideVal0_gmode_tx_pu_ovr_val_MASK |
				LCN40PHY_RFOverrideVal0_gmode_rx_pu_ovr_val_MASK |
				LCN40PHY_RFOverrideVal0_amode_tx_pu_ovr_val_MASK |
				LCN40PHY_RFOverrideVal0_amode_rx_pu_ovr_val_MASK))
			PHY_REG_OR_RAW_ENTRY(LCN40PHY_RFOverride0,
				LCN40PHY_RFOverride0_rfpll_pu_ovr_MASK	|
				LCN40PHY_RFOverride0_internalrfrxpu_ovr_MASK |
				LCN40PHY_RFOverride0_internalrftxpu_ovr_MASK |
				LCN40PHY_RFOverride0_gmode_tx_pu_ovr_MASK |
				LCN40PHY_RFOverride0_gmode_rx_pu_ovr_MASK |
				LCN40PHY_RFOverride0_amode_tx_pu_ovr_MASK |
				LCN40PHY_RFOverride0_amode_rx_pu_ovr_MASK)

			PHY_REG_AND_RAW_ENTRY(LCN40PHY_rfoverride8val,
				~(LCN40PHY_rfoverride8val_rxrf_wrssi1_pwrup_ovr_val_MASK |
				LCN40PHY_rfoverride8val_rxrf_wrssi2_pwrup_ovr_val_MASK |
				LCN40PHY_rfoverride8val_lna2_pu_ovr_val_MASK))

			PHY_REG_OR_RAW_ENTRY(LCN40PHY_rfoverride8,
				LCN40PHY_rfoverride8_rxrf_wrssi1_pwrup_ovr_MASK |
				LCN40PHY_rfoverride8_rxrf_wrssi2_pwrup_ovr_MASK |
				LCN40PHY_rfoverride8_lna2_pu_ovr_MASK)

			PHY_REG_OR_RAW_ENTRY(LCN40PHY_RFOverride0,
				LCN40PHY_RFOverride0_rfpll_pu_ovr_MASK |
				LCN40PHY_RFOverride0_internalrfrxpu_ovr_MASK |
				LCN40PHY_RFOverride0_internalrftxpu_ovr_MASK |
				LCN40PHY_RFOverride0_gmode_tx_pu_ovr_MASK |
				LCN40PHY_RFOverride0_gmode_rx_pu_ovr_MASK |
				LCN40PHY_RFOverride0_amode_tx_pu_ovr_MASK |
				LCN40PHY_RFOverride0_amode_rx_pu_ovr_MASK)

			PHY_REG_AND_RAW_ENTRY(LCN40PHY_rfoverride2val,
				~(LCN40PHY_rfoverride2val_slna_pu_ovr_val_MASK))
			PHY_REG_OR_RAW_ENTRY(LCN40PHY_rfoverride2,
				LCN40PHY_rfoverride2_slna_pu_ovr_MASK)
			PHY_REG_AND_RAW_ENTRY(LCN40PHY_rfoverride3_val,
				~(LCN40PHY_rfoverride3_val_rfactive_ovr_val_MASK))
			PHY_REG_OR_RAW_ENTRY(LCN40PHY_rfoverride3,
				LCN40PHY_rfoverride3_rfactive_ovr_MASK)
		PHY_REG_LIST_EXECUTE(pi);
	}
}

void
phy_lcn40_radio_switch(phy_lcn40_radio_info_t *info, bool on)
{
	_phy_lcn40_radio_switch(info, on);
}

/* turn radio on */
static void
WLBANDINITFN(phy_lcn40_radio_on)(phy_type_radio_ctx_t *ctx)
{
	_phy_lcn40_radio_switch(ctx, ON);
}

/* switch radio off when changing band */
static void
phy_lcn40_radio_switch_band(phy_type_radio_ctx_t *ctx)
{
	_phy_lcn40_radio_switch(ctx, OFF);
}

/* query radio idcode */
static uint32
_phy_lcn40_radio_query_idcode(phy_type_radio_ctx_t *ctx)
{
	phy_lcn40_radio_info_t *info = (phy_lcn40_radio_info_t *)ctx;
	phy_info_t *pi = info->pi;

	return phy_lcn40_radio_query_idcode(pi);
}

uint32
phy_lcn40_radio_query_idcode(phy_info_t *pi)
{
	uint32 rev, ver;
	uint32 idcode;

	W_REG(pi->sh->osh, &pi->regs->radioregaddr, 0);
	rev = (uint32)R_REG(pi->sh->osh, &pi->regs->radioregdata);
	ver = rev & 0xF;
	rev = (rev >> 4) & 0xF;

	W_REG(pi->sh->osh, &pi->regs->radioregaddr, 1);
	idcode = R_REG(pi->sh->osh, &pi->regs->radioregdata);
	/* The format of all other PHY is like the following:
	 * id = (rev << IDCODE_REV_SHIFT) | (R_REG(pi->sh->osh, &pi->regs->radioregdata)
	 * 	<< IDCODE_ID_SHIFT) | ver;
	 * the LCN40PHY team decided that we should use RADIOVER only, so
	 * it becomes the following format:
	 */
	if (RADIOID(idcode) == BCM2065_ID && RADIOVER(ver) == 2)
		idcode = (rev << IDCODE_REV_SHIFT) | (idcode << IDCODE_ID_SHIFT) | ver;
	else if (rev == 6)
		idcode = (idcode << IDCODE_ID_SHIFT) | ver;
	else
		idcode = (idcode << IDCODE_ID_SHIFT) | rev;

	return idcode;
}
