/*
 * LCNPHY RADIO contorl module implementation
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
#include <phy_lcn.h>
#include <phy_lcn_radio.h>

#include <wlc_phy_radio.h>
#include <wlc_phyreg_lcn.h>

#ifndef ALL_NEW_PHY_MOD
/* < TODO: all these are going away... */
#include <wlc_phy_int.h>
#include <wlc_phy_lcn.h>
/* TODO: all these are going away... > */
#endif // endif

#include <phy_utils_reg.h>

/* module private states */
struct phy_lcn_radio_info {
	phy_info_t *pi;
	phy_lcn_info_t *lcni;
	phy_radio_info_t *ri;
};

/* local functions */
static void phy_lcn_radio_switch(phy_type_radio_ctx_t *ctx, bool on);
static void phy_lcn_radio_on(phy_type_radio_ctx_t *ctx);
static void phy_lcn_radio_switch_band(phy_type_radio_ctx_t *ctx);
static uint32 _phy_lcn_radio_query_idcode(phy_type_radio_ctx_t *ctx);
#if (defined(BCMDBG) || defined(BCMDBG_DUMP)) && defined(DBG_PHY_IOV)
static int phy_lcn_radio_dump(phy_type_radio_ctx_t *ctx, struct bcmstrbuf *b);
#else
#define phy_lcn_radio_dump NULL
#endif // endif

/* Register/unregister LCNPHY specific implementation to common layer. */
phy_lcn_radio_info_t *
BCMATTACHFN(phy_lcn_radio_register_impl)(phy_info_t *pi, phy_lcn_info_t *lcni,
	phy_radio_info_t *ri)
{
	phy_lcn_radio_info_t *info;
	phy_type_radio_fns_t fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage in once */
	if ((info = phy_malloc(pi, sizeof(phy_lcn_radio_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	bzero(info, sizeof(phy_lcn_radio_info_t));
	info->pi = pi;
	info->lcni = lcni;
	info->ri = ri;

	pi->pubpi.radiooffset = RADIO_2064_READ_OFF;

#ifndef BCM_OL_DEV
	/* make sure the radio is off until we do an "up" */
	phy_lcn_radio_switch(info, OFF);
#endif // endif

	/* Register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.ctrl = phy_lcn_radio_switch;
	fns.on = phy_lcn_radio_on;
	fns.bandx = phy_lcn_radio_switch_band;
	fns.id = _phy_lcn_radio_query_idcode;
	fns.dump = phy_lcn_radio_dump;
	fns.ctx = info;

	phy_radio_register_impl(ri, &fns);

	return info;

fail:
	if (info != NULL)
		phy_mfree(pi, info, sizeof(phy_lcn_radio_info_t));
	return NULL;
}

void
BCMATTACHFN(phy_lcn_radio_unregister_impl)(phy_lcn_radio_info_t *info)
{
	phy_info_t *pi = info->pi;
	phy_radio_info_t *ri = info->ri;

	PHY_TRACE(("%s\n", __FUNCTION__));

	phy_radio_unregister_impl(ri);

	phy_mfree(pi, info, sizeof(phy_lcn_radio_info_t));
}

/* switch radio on/off */
static void
phy_lcn_radio_switch(phy_type_radio_ctx_t *ctx, bool on)
{
	phy_lcn_radio_info_t *info = (phy_lcn_radio_info_t *)ctx;
	phy_info_t *pi = info->pi;

	PHY_TRACE(("wl%d: %s %d\n", pi->sh->unit, __FUNCTION__, on));

	if (on) {
		PHY_REG_LIST_START
			PHY_REG_AND_ENTRY(LCNPHY, RFOverride0,
				~(LCNPHY_RFOverride0_rfpll_pu_ovr_MASK	|
				LCNPHY_RFOverride0_wrssi_pu_ovr_MASK 		|
				LCNPHY_RFOverride0_nrssi_pu_ovr_MASK 		|
				LCNPHY_RFOverride0_internalrfrxpu_ovr_MASK 	|
				LCNPHY_RFOverride0_internalrftxpu_ovr_MASK))
			PHY_REG_AND_ENTRY(LCNPHY, rfoverride2,
				~(LCNPHY_rfoverride2_lna_pu_ovr_MASK |
				LCNPHY_rfoverride2_slna_pu_ovr_MASK))
			PHY_REG_AND_ENTRY(LCNPHY, rfoverride3,
				~LCNPHY_rfoverride3_rfactive_ovr_MASK)
		PHY_REG_LIST_EXECUTE(pi);
		if (pi->u.pi_lcnphy->lcnphy_spurmod == 1) {
			PHY_REG_LIST_START
				PHY_REG_WRITE_ENTRY(LCNPHY,
					spur_canceller1, ((1 << 13) + 23))
				PHY_REG_WRITE_ENTRY(LCNPHY,
					spur_canceller2, ((1 << 13) + 1989))
			PHY_REG_LIST_EXECUTE(pi);
		}
	} else {
		PHY_REG_LIST_START
			PHY_REG_AND_ENTRY(LCNPHY, RFOverrideVal0,
				~(LCNPHY_RFOverrideVal0_rfpll_pu_ovr_val_MASK |
				LCNPHY_RFOverrideVal0_wrssi_pu_ovr_val_MASK 	|
				LCNPHY_RFOverrideVal0_nrssi_pu_ovr_val_MASK 	|
				LCNPHY_RFOverrideVal0_internalrfrxpu_ovr_val_MASK |
				LCNPHY_RFOverrideVal0_internalrftxpu_ovr_val_MASK))
			PHY_REG_OR_ENTRY(LCNPHY, RFOverride0,
				LCNPHY_RFOverride0_rfpll_pu_ovr_MASK 		|
				LCNPHY_RFOverride0_wrssi_pu_ovr_MASK 		|
				LCNPHY_RFOverride0_nrssi_pu_ovr_MASK 		|
				LCNPHY_RFOverride0_internalrfrxpu_ovr_MASK 	|
				LCNPHY_RFOverride0_internalrftxpu_ovr_MASK)
			PHY_REG_AND_ENTRY(LCNPHY, rxlnaandgainctrl1ovrval,
				~(LCNPHY_rxlnaandgainctrl1ovrval_lnapuovr_Val_MASK))
			PHY_REG_AND_ENTRY(LCNPHY, rfoverride2val,
				~(LCNPHY_rfoverride2val_slna_pu_ovr_val_MASK))
			PHY_REG_OR_ENTRY(LCNPHY, rfoverride2,
				LCNPHY_rfoverride2_lna_pu_ovr_MASK |
				LCNPHY_rfoverride2_slna_pu_ovr_MASK)
			PHY_REG_AND_ENTRY(LCNPHY, rfoverride3_val,
				~(LCNPHY_rfoverride3_val_rfactive_ovr_val_MASK))
			PHY_REG_OR_ENTRY(LCNPHY, rfoverride3,
				LCNPHY_rfoverride3_rfactive_ovr_MASK)
		PHY_REG_LIST_EXECUTE(pi);
	}
}

/* turn radio on */
static void
WLBANDINITFN(phy_lcn_radio_on)(phy_type_radio_ctx_t *ctx)
{
	phy_lcn_radio_switch(ctx, ON);
}

/* switch radio off when changing band */
static void
phy_lcn_radio_switch_band(phy_type_radio_ctx_t *ctx)
{
	phy_lcn_radio_switch(ctx, OFF);
}

/* query radio idcode */
static uint32
_phy_lcn_radio_query_idcode(phy_type_radio_ctx_t *ctx)
{
	phy_lcn_radio_info_t *info = (phy_lcn_radio_info_t *)ctx;
	phy_info_t *pi = info->pi;

	return phy_lcn_radio_query_idcode(pi);
}

uint32
phy_lcn_radio_query_idcode(phy_info_t *pi)
{
	uint32 b0, b1, b2;

	W_REG(pi->sh->osh, &pi->regs->radioregaddr, 0);
#ifdef __mips__
	(void)R_REG(pi->sh->osh, &pi->regs->radioregaddr);
#endif // endif
	b0 = (uint32)R_REG(pi->sh->osh, &pi->regs->radioregdata);
	W_REG(pi->sh->osh, &pi->regs->radioregaddr, 1);
#ifdef __mips__
	(void)R_REG(pi->sh->osh, &pi->regs->radioregaddr);
#endif // endif
	b1 = (uint32)R_REG(pi->sh->osh, &pi->regs->radioregdata);
	W_REG(pi->sh->osh, &pi->regs->radioregaddr, 2);
#ifdef __mips__
	(void)R_REG(pi->sh->osh, &pi->regs->radioregaddr);
#endif // endif
	b2 = (uint32)R_REG(pi->sh->osh, &pi->regs->radioregdata);

	return ((b0  & 0xf) << 28) | (((b2 << 8) | b1) << 12) | ((b0 >> 4) & 0xf);
}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
#if defined(DBG_PHY_IOV)
static int
phy_lcn_radio_dump(phy_type_radio_ctx_t *ctx, struct bcmstrbuf *b)
{
	phy_lcn_radio_info_t *gi = (phy_lcn_radio_info_t *)ctx;
	phy_info_t *pi = gi->pi;
	const char *name = NULL;
	int i, jtag_core;
	uint16 addr = 0;
	lcnphy_radio_regs_t *lcnphyregs = NULL;

	if (LCNREV_IS(pi->pubpi.phy_rev, 2)) {
		lcnphyregs = lcnphy_radio_regs_2066;
		name = "2066 Radio";
	} else {
		lcnphyregs = lcnphy_radio_regs_2064;
		name = "2064 Radio";
	}

	bcm_bprintf(b, "----- %08s -----\n", name);
	bcm_bprintf(b, "Add Value\n");

	i = 0;
	while (TRUE) {
		addr = lcnphyregs[i].address;

		if (addr == 0xffff)
			break;

		jtag_core = 0;

		bcm_bprintf(b, "%03x %04x\n", addr, phy_utils_read_radioreg(pi, addr | jtag_core));
		i++;
	}

	return BCME_OK;
}
#endif // endif
#endif /* BCMDBG || BCMDBG_DUMP */
