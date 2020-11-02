/*
 * LCNPHY ANTennaDIVersity module implementation
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
#include "phy_type_antdiv.h"
#include <phy_lcn.h>
#include <phy_lcn_antdiv.h>

#include <wlc_phyreg_lcn.h>
#include <phy_utils_reg.h>

/* module private states */
struct phy_lcn_antdiv_info {
	phy_info_t *pi;
	phy_lcn_info_t *lcni;
	phy_antdiv_info_t *di;
};

/* local functions */
static void phy_lcn_antdiv_set_rx(phy_type_antdiv_ctx_t *ctx, uint8 ant);

/* register phy type specific implementation */
phy_lcn_antdiv_info_t *
BCMATTACHFN(phy_lcn_antdiv_register_impl)(phy_info_t *pi, phy_lcn_info_t *lcni,
	phy_antdiv_info_t *di)
{
	phy_lcn_antdiv_info_t *info;
	phy_type_antdiv_fns_t fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage together */
	if ((info = phy_malloc(pi, sizeof(phy_lcn_antdiv_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	bzero(info, sizeof(phy_lcn_antdiv_info_t));
	info->pi = pi;
	info->lcni = lcni;
	info->di = di;

	/* register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.setrx = phy_lcn_antdiv_set_rx;
	fns.ctx = info;

	phy_antdiv_register_impl(di, &fns);

	return info;

	/* error handling */
fail:
	if (info != NULL)
		phy_mfree(pi, info, sizeof(phy_lcn_antdiv_info_t));
	return NULL;
}

void
BCMATTACHFN(phy_lcn_antdiv_unregister_impl)(phy_lcn_antdiv_info_t *info)
{
	phy_info_t *pi = info->pi;
	phy_antdiv_info_t *di = info->di;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* unregister from common */
	phy_antdiv_unregister_impl(di);

	phy_mfree(pi, info, sizeof(phy_lcn_antdiv_info_t));
}

/* Setup */
static void
phy_lcn_antdiv_set_rx(phy_type_antdiv_ctx_t *ctx, uint8 ant)
{
	phy_lcn_antdiv_info_t *info = (phy_lcn_antdiv_info_t *)ctx;
	phy_info_t *pi = info->pi;

	PHY_TRACE(("%s: ant 0x%x\n", __FUNCTION__, ant));

	if (ant > ANT_RX_DIV_FORCE_1)
		wlapi_bmac_mhf(pi->sh->physhim, MHF1, MHF1_ANTDIV, MHF1_ANTDIV, WLC_BAND_ALL);
	else
		wlapi_bmac_mhf(pi->sh->physhim, MHF1, MHF1_ANTDIV, 0, WLC_BAND_ALL);

	if (ant > ANT_RX_DIV_FORCE_1) {
		phy_utils_mod_phyreg(pi, LCNPHY_crsgainCtrl,
		                     LCNPHY_crsgainCtrl_DiversityChkEnable_MASK,
		                     0x01 << LCNPHY_crsgainCtrl_DiversityChkEnable_SHIFT);
		phy_utils_mod_phyreg(pi, LCNPHY_crsgainCtrl,
		                     LCNPHY_crsgainCtrl_DefaultAntenna_MASK,
		                     ((ANT_RX_DIV_START_1 == ant) ? 1 : 0) <<
		                     LCNPHY_crsgainCtrl_DefaultAntenna_SHIFT);
	} else {
		phy_utils_mod_phyreg(pi, LCNPHY_crsgainCtrl,
		                     LCNPHY_crsgainCtrl_DiversityChkEnable_MASK,
		                     0x00 << LCNPHY_crsgainCtrl_DiversityChkEnable_SHIFT);
		phy_utils_mod_phyreg(pi, LCNPHY_crsgainCtrl,
		                     LCNPHY_crsgainCtrl_DefaultAntenna_MASK,
		                     (uint16)ant << LCNPHY_crsgainCtrl_DefaultAntenna_SHIFT);
	}
}
