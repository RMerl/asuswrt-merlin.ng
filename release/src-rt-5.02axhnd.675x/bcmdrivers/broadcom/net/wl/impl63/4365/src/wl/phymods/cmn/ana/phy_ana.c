/*
 * ANAcore module implementation.
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
#include <phy_dbg.h>
#include <phy_mem.h>
#include <phy_init.h>
#include "phy_type_ana.h"
#include <phy_ana_api.h>
#include <phy_ana.h>

/* module private states */
struct phy_ana_info {
	phy_info_t *pi;
	phy_type_ana_fns_t *fns;
};

/* module private states memory layout */
typedef struct {
	phy_ana_info_t info;
	phy_type_ana_fns_t fns;
/* add other variable size variables here at the end */
} phy_ana_mem_t;

/* local function declaration */
static int phy_ana_init(phy_init_ctx_t *ctx);

/* attach/detach */
phy_ana_info_t *
BCMATTACHFN(phy_ana_attach)(phy_info_t *pi)
{
	phy_ana_info_t *info;
	phy_type_ana_fns_t *fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate attach info storage */
	if ((info = phy_malloc(pi, sizeof(phy_ana_mem_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	info->pi = pi;

	fns = &((phy_ana_mem_t *)info)->fns;
	info->fns = fns;

	/* register init fn */
	if (phy_init_add_init_fn(pi->initi, phy_ana_init, info, PHY_INIT_ANA) != BCME_OK) {
		PHY_ERROR(("%s: phy_init_add_init_fn failed\n", __FUNCTION__));
		goto fail;
	}

	/* Register callbacks */

	return info;

	/* error */
fail:
	phy_ana_detach(info);
	return NULL;
}

void
BCMATTACHFN(phy_ana_detach)(phy_ana_info_t *info)
{
	phy_info_t *pi;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (info == NULL) {
		PHY_INFORM(("%s: null ana module\n", __FUNCTION__));
		return;
	}

	pi = info->pi;

	phy_mfree(pi, info, sizeof(phy_ana_mem_t));
}

/* switch on/off anacore */
void
phy_ana_switch(phy_info_t *pi, bool on)
{
	phy_ana_info_t *ani = pi->anai;
	phy_type_ana_fns_t *fns = ani->fns;

	PHY_TRACE(("%s: %d\n", __FUNCTION__, on));

	ASSERT(fns->ctrl != NULL);
	(fns->ctrl)(fns->ctx, on);
}

/* reset anacore */
void
phy_ana_reset(phy_info_t *pi)
{
	phy_ana_info_t *ani = pi->anai;
	phy_type_ana_fns_t *fns = ani->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (fns->reset == NULL)
		return;

	(fns->reset)(fns->ctx);
}

static int
phy_ana_init(phy_init_ctx_t *ctx)
{
	phy_ana_info_t *ani = (phy_ana_info_t *)ctx;
	phy_info_t *pi = ani->pi;

	PHY_TRACE(("%s\n", __FUNCTION__));

	phy_ana_switch(pi, ON);
	return BCME_OK;
}

/* register phy type specific implementations */
int
BCMATTACHFN(phy_ana_register_impl)(phy_ana_info_t *ani, phy_type_ana_fns_t *fns)
{

	PHY_TRACE(("%s\n", __FUNCTION__));

	*ani->fns = *fns;

	return BCME_OK;
}

void
BCMATTACHFN(phy_ana_unregister_impl)(phy_ana_info_t *ani)
{
	PHY_TRACE(("%s\n", __FUNCTION__));
}
