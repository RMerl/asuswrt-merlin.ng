/*
 * ACPHY ANTennaDIVersity module implementation
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
#include <phy_ac.h>
#include <phy_ac_antdiv.h>
#include <wlc_phy_ac.h>
#include <phy_utils_reg.h>
#include <sbchipc.h>
#include <wlc_phyreg_ac.h>

/* module private states */
struct phy_ac_antdiv_info {
	phy_info_t *pi;
	phy_ac_info_t *aci;
	phy_antdiv_info_t *di;
};

/* local functions */
static void phy_ac_antdiv_set_rx(phy_type_antdiv_ctx_t *ctx, uint8 ant);
static uint32 si_gci_chipstatus_acphy(si_t *sih, uint reg);

/* register phy type specific implementation */
phy_ac_antdiv_info_t *
BCMATTACHFN(phy_ac_antdiv_register_impl)(phy_info_t *pi, phy_ac_info_t *aci,
	phy_antdiv_info_t *di)
{
	phy_ac_antdiv_info_t *info;
	phy_type_antdiv_fns_t fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage together */
	if ((info = phy_malloc(pi, sizeof(phy_ac_antdiv_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	info->pi = pi;
	info->aci = aci;
	info->di = di;

	/* register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.setrx = phy_ac_antdiv_set_rx;
	fns.ctx = info;

	phy_antdiv_register_impl(di, &fns);

	return info;

	/* error handling */
fail:
	if (info != NULL)
		phy_mfree(pi, info, sizeof(phy_ac_antdiv_info_t));
	return NULL;
}

void
BCMATTACHFN(phy_ac_antdiv_unregister_impl)(phy_ac_antdiv_info_t *info)
{
	phy_info_t *pi;
	phy_antdiv_info_t *di;

	ASSERT(info);
	pi = info->pi;
	di = info->di;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* unregister from common */
	phy_antdiv_unregister_impl(di);

	phy_mfree(pi, info, sizeof(phy_ac_antdiv_info_t));
}

/* Setup */
static void
phy_ac_antdiv_set_rx(phy_type_antdiv_ctx_t *ctx, uint8 ant)
{
	phy_ac_antdiv_info_t *info = (phy_ac_antdiv_info_t *)ctx;
	phy_info_t *pi = info->pi;

	PHY_TRACE(("%s: ant 0x%x\n", __FUNCTION__, ant));

	if (!wlc_phy_check_antdiv_enable_acphy(pi))
		return;

	if (ant > ANT_RX_DIV_FORCE_1)
		wlapi_bmac_mhf(pi->sh->physhim, MHF1, MHF1_ANTDIV, MHF1_ANTDIV, WLC_BAND_ALL);
	else
		wlapi_bmac_mhf(pi->sh->physhim, MHF1, MHF1_ANTDIV, 0, WLC_BAND_ALL);

	wlc_phy_antdiv_acphy(pi, ant);
}

/* Read the gci chip status register indexed by 'reg' */
static uint32
si_gci_chipstatus_acphy(si_t *sih, uint reg)
{
	/* because NFLASH and GCI clashes in 0xC00 */
#ifndef NFLASH_SUPPORT
	si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, gci_indirect_addr), ~0, reg);
	/* setting mask and value to '0' to use si_corereg for read only purpose */
	return si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, gci_chipsts), 0, 0);
#else /* NFLASH_SUPPORT */
	ASSERT(0);
	return 0xFFFFFFFF;
#endif // endif
}

bool
wlc_phy_check_antdiv_enable_acphy(phy_info_t *pi)
{
	if ((ACMAJORREV_1(pi->pubpi.phy_rev) && ACMINORREV_2(pi))) {
		return ((si_gci_chipstatus_acphy(pi->sh->sih, 8) >> 2) & 0x1);
	} else {
		return (ACMAJORREV_3(pi->pubpi.phy_rev));
	}
}

void
wlc_phy_antdiv_acphy(phy_info_t *pi, uint8 val)
{
	if (val > ANT_RX_DIV_FORCE_1) {
		MOD_PHYREG(pi, AntDivConfig2059, board_switch_div0, 1); /* enable diversity */
		if (val == ANT_RX_DIV_START_1) {
			MOD_PHYREG(pi, AntDivConfig2059, CoreStartAntPos0, 1);
		} else {
			MOD_PHYREG(pi, AntDivConfig2059, CoreStartAntPos0, 0);
		}

		MOD_PHYREG(pi, AntennaDivDwellTime, DivDwellTime, 64); /* 1p6us dwell time */
		MOD_PHYREG(pi, DivEnableClipGain, AntDivEnClipGains_Md, 0);
		MOD_PHYREG(pi, DivEnableClipGain, AntDivEnClipGains_Lo, 0);
		MOD_PHYREG(pi, DivEnableClipGain, AntDivEnClipGains_Clip2, 0);
		MOD_PHYREG(pi, DivEnableClipGain, AntDivEnClipGainBphy, 0);
		MOD_PHYREG(pi, DivGainThreshold_OFDM, Div_GainThresh_OFDM, 85);
		MOD_PHYREG(pi, DivGainThreshold_BPHY, Div_GainThresh_BPHY, 95);
		MOD_PHYREG(pi, AntennaDivBackOffGain, BackoffGain, 6);
		MOD_PHYREG(pi, AntennaDivMinGain, cckBackoffGain, 0);
		/* MOD_PHYREG(pi, defer_setClip1_CtrLen, defer_setclip1gain_len, 20); */

		if (CHSPEC_IS5G(pi->radio_chanspec)) {
			if (CHSPEC_IS80(pi->radio_chanspec) ||
					PHY_AS_80P80(pi, pi->radio_chanspec)) {
				MOD_PHYREG(pi, DivEnableClipGain, AntDivEnClipGains_Hi, 1);
			} else if (CHSPEC_IS160(pi->radio_chanspec)) {
				ASSERT(0); //FIXME
			} else {
				MOD_PHYREG(pi, DivEnableClipGain, AntDivEnClipGains_Hi, 0);
			}
		} else {
			MOD_PHYREG(pi, DivEnableClipGain, AntDivEnClipGains_Hi, 0);
		}
	} else {
		/* disable HW antsel */
		MOD_PHYREG(pi, AntDivConfig2059, board_switch_div0, 0);
		if (val == ANT_RX_DIV_FORCE_1) {
			MOD_PHYREG(pi, AntDivConfig2059, CoreStartAntPos0, 1);
		} else {
			MOD_PHYREG(pi, AntDivConfig2059, CoreStartAntPos0, 0);
		}
	}
}
