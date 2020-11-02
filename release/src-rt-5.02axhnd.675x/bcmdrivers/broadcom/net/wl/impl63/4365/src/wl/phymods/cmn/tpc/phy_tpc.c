/*
 * TxPowerCtrl module implementation.
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
#include <bcmutils.h>
#include <bcmendian.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include <phy_init.h>
#include <phy_rstr.h>
#include "phy_type_tpc.h"
#include <phy_tpc.h>

#include <phy_utils_var.h>

/* module private states */
struct phy_tpc_info {
	phy_info_t *pi;
	phy_type_tpc_fns_t *fns;

	bool user_txpwr_at_rfport;
	uint8 ucode_tssi_limit_en;
};

/* module private states memory layout */
typedef struct {
	phy_tpc_info_t info;
	phy_type_tpc_fns_t fns;
/* add other variable size variables here at the end */
} phy_tpc_mem_t;

/* local function declaration */

/* attach/detach */
phy_tpc_info_t *
BCMATTACHFN(phy_tpc_attach)(phy_info_t *pi)
{
	phy_tpc_info_t *info;
	phy_type_tpc_fns_t *fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate attach info storage */
	if ((info = phy_malloc(pi, sizeof(phy_tpc_mem_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	info->pi = pi;

	fns = &((phy_tpc_mem_t *)info)->fns;
	info->fns = fns;

	/* user specified power is at the ant port */
#ifdef WLNOKIA_NVMEM
	info->user_txpwr_at_rfport = TRUE;
#else
	info->user_txpwr_at_rfport = FALSE;
#endif // endif

	info->ucode_tssi_limit_en = (uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_tssilimucod, 1);

	/* Register callbacks */

	return info;

	/* error */
fail:
	phy_tpc_detach(info);
	return NULL;
}

void
BCMATTACHFN(phy_tpc_detach)(phy_tpc_info_t *info)
{
	phy_info_t *pi;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (info == NULL) {
		PHY_INFORM(("%s: null tpc module\n", __FUNCTION__));
		return;
	}

	pi = info->pi;

	phy_mfree(pi, info, sizeof(phy_tpc_mem_t));
}

/* recalc target txpwr and apply to h/w */
void
phy_tpc_recalc_tgt(phy_tpc_info_t *ti)
{
	phy_type_tpc_fns_t *fns = ti->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	ASSERT(fns->recalc != NULL);
	(fns->recalc)(fns->ctx);
}

/* read SROM for the given bandtype */
int
BCMATTACHFN(phy_tpc_read_srom)(phy_tpc_info_t *ti, int bandtype)
{
	phy_type_tpc_fns_t *fns = ti->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (fns->read_srom == NULL)
		return BCME_OK;

	return (fns->read_srom)(fns->ctx, bandtype);
}

/* check limit? */
void
phy_tpc_check_limit(phy_info_t *pi)
{
	phy_tpc_info_t *ti = pi->tpci;
	phy_type_tpc_fns_t *fns = ti->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (!ti->ucode_tssi_limit_en)
		return;

	if (fns->check == NULL)
		return;

	(fns->check)(fns->ctx);
}

/* register phy type specific implementations */
int
BCMATTACHFN(phy_tpc_register_impl)(phy_tpc_info_t *ti, phy_type_tpc_fns_t *fns)
{
	PHY_TRACE(("%s\n", __FUNCTION__));

	*ti->fns = *fns;

	return BCME_OK;
}

void
BCMATTACHFN(phy_tpc_unregister_impl)(phy_tpc_info_t *ti)
{
	PHY_TRACE(("%s\n", __FUNCTION__));
}
