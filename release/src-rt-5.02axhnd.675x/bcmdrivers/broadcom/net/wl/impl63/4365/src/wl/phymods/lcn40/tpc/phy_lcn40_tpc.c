/*
 * LCN40PHY TxPowerCtrl module implementation
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
#include <phy_tpc.h>
#include "phy_tpc_shared.h"
#include "phy_type_tpc.h"
#include <phy_lcn40.h>
#include <phy_lcn40_tpc.h>

#ifndef ALL_NEW_PHY_MOD
/* < TODO: all these are going away... */
#include <wlc_phy_int.h>
#include <wlc_phy_lcn40.h>
/* TODO: all these are going away... > */
#endif // endif

/* module private states */
struct phy_lcn40_tpc_info {
	phy_info_t *pi;
	phy_lcn40_info_t *lcn40i;
	phy_tpc_info_t *ti;
};

/* local functions */
static int phy_lcn40_tpc_init(phy_type_tpc_ctx_t *ctx);
static void phy_lcn40_tpc_recalc_tgt(phy_type_tpc_ctx_t *ctx);
static int phy_lcn40n_tpc_read_srom(phy_type_tpc_ctx_t *ctx, int bandtype);

/* Register/unregister LCN40PHY specific implementation to common layer. */
phy_lcn40_tpc_info_t *
BCMATTACHFN(phy_lcn40_tpc_register_impl)(phy_info_t *pi, phy_lcn40_info_t *lcn40i,
	phy_tpc_info_t *ti)
{
	phy_lcn40_tpc_info_t *info;
	phy_type_tpc_fns_t fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage in once */
	if ((info = phy_malloc(pi, sizeof(phy_lcn40_tpc_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	bzero(info, sizeof(phy_lcn40_tpc_info_t));
	info->pi = pi;
	info->lcn40i = lcn40i;
	info->ti = ti;

	/* Register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.init = phy_lcn40_tpc_init;
	fns.recalc = phy_lcn40_tpc_recalc_tgt;
	fns.read_srom = phy_lcn40n_tpc_read_srom;
	fns.ctx = info;

	phy_tpc_register_impl(ti, &fns);

	return info;

fail:
	if (info != NULL)
		phy_mfree(pi, info, sizeof(phy_lcn40_tpc_info_t));
	return NULL;
}

void
BCMATTACHFN(phy_lcn40_tpc_unregister_impl)(phy_lcn40_tpc_info_t *info)
{
	phy_info_t *pi = info->pi;
	phy_tpc_info_t *ti = info->ti;

	PHY_TRACE(("%s\n", __FUNCTION__));

	phy_tpc_unregister_impl(ti);

	phy_mfree(pi, info, sizeof(phy_lcn40_tpc_info_t));
}

/* init module and h/w */
static int
WLBANDINITFN(phy_lcn40_tpc_init)(phy_type_tpc_ctx_t *ctx)
{
	phy_lcn40_tpc_info_t *info = (phy_lcn40_tpc_info_t *)ctx;
	phy_lcn40_info_t *lcn40i = info->lcn40i;
	phy_info_t *pi = info->pi;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (lcn40i->lcnphycommon.txpwrindex5g_nvram ||
	    lcn40i->lcnphycommon.txpwrindex_nvram) {
		uint8 txpwrindex;
#ifdef BAND5G
		if (CHSPEC_IS5G(pi->radio_chanspec))
			txpwrindex = lcn40i->lcnphycommon.txpwrindex5g_nvram;
		else
#endif /* BAND5G */
			txpwrindex = lcn40i->lcnphycommon.txpwrindex_nvram;

		if (txpwrindex) {
#if defined(PHYCAL_CACHING)
			ch_calcache_t *ctxTmp = wlc_phy_get_chanctx(pi, pi->last_radio_chanspec);
			wlc_iovar_txpwrindex_set_lcncommon(pi, txpwrindex, ctxTmp);
#else
			wlc_iovar_txpwrindex_set_lcncommon(pi, txpwrindex);
#endif /* defined(PHYCAL_CACHING) */
		}
	}

	return BCME_OK;
}

/* recalc target txpwr and apply to h/w */
static void
phy_lcn40_tpc_recalc_tgt(phy_type_tpc_ctx_t *ctx)
{
	phy_lcn40_tpc_info_t *info = (phy_lcn40_tpc_info_t *)ctx;
	phy_info_t *pi = info->pi;

	PHY_TRACE(("%s\n", __FUNCTION__));

	wlc_phy_txpower_recalc_target_lcn40phy(pi);
}

/* read srom txpwr limits */
static int
BCMATTACHFN(phy_lcn40n_tpc_read_srom)(phy_type_tpc_ctx_t *ctx, int bandtype)
{
	phy_lcn40_tpc_info_t *info = (phy_lcn40_tpc_info_t *)ctx;
	phy_info_t *pi = info->pi;

	PHY_TRACE(("%s: band %d\n", __FUNCTION__, bandtype));

	return wlc_lcn40phy_txpwr_srom_read(pi) ? BCME_OK : BCME_ERROR;
}
