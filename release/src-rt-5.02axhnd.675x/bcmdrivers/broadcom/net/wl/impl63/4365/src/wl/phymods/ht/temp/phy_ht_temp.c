/*
 * HTPHY TEMPerature sense module implementation
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
#include <phy_ht.h>
#include <phy_ht_temp.h>

#ifndef ALL_NEW_PHY_MOD
/* < TODO: all these are going away... */
#include <wlc_phy_int.h>
#include <wlc_phy_ht.h>
/* TODO: all these are going away... > */
#endif // endif

/* module private states */
struct phy_ht_temp_info {
	phy_info_t *pi;
	phy_ht_info_t *hti;
	phy_temp_info_t *ti;
};

/* local functions */
static uint8 phy_ht_temp_throttle(phy_type_temp_ctx_t *ctx);
static int phy_ht_temp_get(phy_type_temp_ctx_t *ctx);

/* Register/unregister HTPHY specific implementation to common layer */
phy_ht_temp_info_t *
BCMATTACHFN(phy_ht_temp_register_impl)(phy_info_t *pi, phy_ht_info_t *hti, phy_temp_info_t *ti)
{
	phy_ht_temp_info_t *info;
	phy_type_temp_fns_t fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage in once */
	if ((info = phy_malloc(pi, sizeof(phy_ht_temp_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	bzero(info, sizeof(phy_ht_temp_info_t));
	info->pi = pi;
	info->hti = hti;
	info->ti = ti;

	/* Register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.throt = phy_ht_temp_throttle;
	fns.get = phy_ht_temp_get;
	fns.ctx = info;

	phy_temp_register_impl(ti, &fns);

	return info;
fail:
	if (info != NULL)
		phy_mfree(pi, info, sizeof(phy_ht_temp_info_t));
	return NULL;
}

void
BCMATTACHFN(phy_ht_temp_unregister_impl)(phy_ht_temp_info_t *info)
{
	phy_info_t *pi = info->pi;
	phy_temp_info_t *ti = info->ti;

	PHY_TRACE(("%s\n", __FUNCTION__));

	phy_temp_unregister_impl(ti);

	phy_mfree(pi, info, sizeof(phy_ht_temp_info_t));
}

/* XXX Tx-Core Shut-Down to prevent hitting critical junction temperature
 * Assumptions: Code written assuming max txchain = 7 (3 Tx Chain)
 * Output is stored in pi->txcore_temp->bitmap.
 * BitMap returns the active RxChain and TxChain.
 */
static uint8
phy_ht_temp_throttle(phy_type_temp_ctx_t *ctx)
{
	phy_ht_temp_info_t *info = (phy_ht_temp_info_t *)ctx;
	phy_temp_info_t *ti = info->ti;
	phy_info_t *pi = info->pi;
	phy_txcore_temp_t *temp;
	/* XXX Shut-Down All Tx-Core Except One When Hot
	 * When there is only 1 tx-core on, there will be no change
	 */
	uint8 txcore_shutdown_lut[] = {1, 1, 2, 1, 4, 1, 2, 5};
	uint8 phyrxchain = pi->sh->phyrxchain;
	uint8 phytxchain = pi->sh->phytxchain;
	uint8 new_phytxchain;
	int16 currtemp;

	PHY_TRACE(("%s\n", __FUNCTION__));

	ASSERT(phytxchain);

	temp = phy_temp_get_st(ti);
	ASSERT(temp != NULL);

	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	currtemp = wlc_phy_tempsense_htphy(pi);
	wlapi_enable_mac(pi->sh->physhim);

#ifdef BCMDBG
	if (pi->tempsense_override)
		currtemp = pi->tempsense_override;
#endif // endif

	if (!temp->heatedup) {
		if (currtemp >= temp->disable_temp) {
			new_phytxchain = txcore_shutdown_lut[phytxchain];
			temp->heatedup = TRUE;
			temp->bitmap = ((phyrxchain << 4) | new_phytxchain);
		}
	} else {
		if (currtemp <= temp->enable_temp) {
			new_phytxchain = pi->sh->hw_phytxchain;
			temp->heatedup = FALSE;
			temp->bitmap = ((phyrxchain << 4) | new_phytxchain);
		}
	}

	return temp->bitmap;
}

/* read the current temperature */
static int
phy_ht_temp_get(phy_type_temp_ctx_t *ctx)
{
	phy_ht_temp_info_t *info = (phy_ht_temp_info_t *)ctx;
	phy_ht_info_t *hti = info->hti;

	return hti->current_temperature;
}
