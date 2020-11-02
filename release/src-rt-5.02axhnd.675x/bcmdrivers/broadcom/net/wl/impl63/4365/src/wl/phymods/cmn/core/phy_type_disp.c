/*
 * PHY Core module implementation - connect PHY type specific layer to common layer
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
#include <phy_cfg.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include <phy_api.h>
#include "phy_shared.h"
#include "phy_type.h"
#include "phy_type_disp.h"
#include <phy_utils_reg.h>

#include <wlc_iocv_types.h>

#ifndef ALL_NEW_PHY_MOD
/* TODO: remove these lines... */
#include <wlc_phy_int.h>
#endif // endif

/* ============= PHY type implementation dispatch table ============= */

#if NCONF
#include "phy_type_n.h"
#endif // endif
#if HTCONF
#include "phy_type_ht.h"
#endif // endif
#if LCNCONF
#include "phy_type_lcn.h"
#endif // endif
#if LCN40CONF
#include "phy_type_lcn40.h"
#endif // endif
#if ACCONF || ACCONF2
#include "phy_type_ac.h"
#endif // endif

/* PHY type specific implementation registry */
typedef struct {
	uint8 phy_type;
	phy_type_info_t *(*attach)(phy_info_t *pi, int bandtype);
	void (*detach)(phy_type_info_t *ti);
} phy_type_reg_t;

static phy_type_reg_t BCMATTACHDATA(phy_type_reg_tbl)[] = {
#if NCONF
	{PHY_TYPE_N, phy_n_attach, phy_n_detach},
#endif // endif
#if HTCONF
	{PHY_TYPE_HT, phy_ht_attach, phy_ht_detach},
#endif // endif
#if LCNCONF
	{PHY_TYPE_LCN, phy_lcn_attach, phy_lcn_detach},
#endif // endif
#if LCN40CONF
	{PHY_TYPE_LCN40, phy_lcn40_attach, phy_lcn40_detach},
#endif // endif
#if ACCONF || ACCONF2
	{PHY_TYPE_AC, phy_ac_attach, phy_ac_detach},
#endif // endif
	/* *** ADD NEW PHY TYPE IMPLEMENTATION ENTRIES HERE *** */
};

/* ============= PHY type implementation dispatch glue ============= */

/* PHY type specific implementation dispatch info */
struct phy_type_disp {
	phy_info_t *pi;
	phy_type_reg_t *reg;
	phy_type_fns_t *fns;
	bool impl;
};

/* PHY type specific implementation dispatch info memory layout */
typedef struct {
	phy_type_disp_t info;
	phy_type_fns_t fns;
} phy_type_mem_t;

/* attach/detach PHY type specific implementation dispatch info */
phy_type_disp_t *
BCMATTACHFN(phy_type_disp_attach)(phy_info_t *pi)
{
	uint i;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* Attach PHY type specific implementation to PHY common */
	for (i = 0; i < ARRAYSIZE(phy_type_reg_tbl); i ++) {
		if (pi->pubpi.phy_type == phy_type_reg_tbl[i].phy_type) {
			phy_type_disp_t *disp;

			if ((disp = phy_malloc(pi, sizeof(phy_type_mem_t))) == NULL) {
				PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
				return NULL;
			}
			disp->pi = pi;
			disp->reg = &phy_type_reg_tbl[i];
			disp->fns = &((phy_type_mem_t *)disp)->fns;

			return disp;
		}
	}

	return NULL;
}

void
BCMATTACHFN(phy_type_disp_detach)(phy_type_disp_t *disp)
{
	phy_info_t *pi;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (disp == NULL)
		return;

	pi = disp->pi;

	phy_mfree(pi, disp, sizeof(phy_type_mem_t));
}

/* register/unregister PHY type specific implementation dispatch info */
int
BCMATTACHFN(phy_register_impl)(phy_info_t *pi, phy_type_fns_t *fns)
{
	phy_type_disp_t *disp = pi->typei;

	PHY_TRACE(("%s\n", __FUNCTION__));

	*disp->fns = *fns;
	disp->impl = TRUE;

	return BCME_OK;
}

void
BCMATTACHFN(phy_unregister_impl)(phy_info_t *pi)
{
	phy_type_disp_t *disp = pi->typei;

	PHY_TRACE(("%s\n", __FUNCTION__));

	disp->impl = FALSE;
}

/* ============= PHY type implementation dispatch  ============= */

/* attach/detach PHY Core type specific implementation layer to/from common layer */
phy_type_info_t *
BCMATTACHFN(phy_type_attach)(phy_type_disp_t *disp, int bandtype)
{
	phy_type_reg_t *type = disp->reg;
	phy_info_t *pi = disp->pi;

	PHY_TRACE(("%s\n", __FUNCTION__));

	ASSERT(type != NULL);
	ASSERT(type->attach != NULL);
	return (type->attach)(pi, bandtype);
}

void
BCMATTACHFN(phy_type_detach)(phy_type_disp_t *disp, phy_type_info_t *ti)
{
	phy_type_reg_t *type = disp->reg;

	PHY_TRACE(("%s\n", __FUNCTION__));

	ASSERT(type != NULL);
	ASSERT(type->detach != NULL);
	(type->detach)(ti);
}

/* register/unregister PHY modules PHY type specific implementation layer to/from common layer */
int
BCMATTACHFN(phy_type_register_impl)(phy_type_disp_t *disp, int bandtype)
{
	phy_type_fns_t *fns = disp->fns;
	phy_info_t *pi = disp->pi;
	int err;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (!disp->impl)
		return BCME_ERROR;

	ASSERT(fns->reg_impl != NULL);
	if ((err = (fns->reg_impl)(pi, fns->ti, bandtype)) != BCME_OK) {
		PHY_ERROR(("%s: fn %p returns error %d\n",
		           __FUNCTION__, fns->reg_impl, err));
		return err;
	}

	/* Prepare for one-time initializations */
	pi->initialized = FALSE;
	pi->txpwridx = -1;
	pi->phy_spuravoid = SPURAVOID_AUTO;

	/* Reset PHY type specific implementation */
	phy_type_reset_impl(disp);

	return BCME_OK;
}

void
BCMATTACHFN(phy_type_unregister_impl)(phy_type_disp_t *disp)
{
	phy_type_fns_t *fns = disp->fns;
	phy_info_t *pi = disp->pi;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (!disp->impl)
		return;

	ASSERT(fns->unreg_impl != NULL);
	(fns->unreg_impl)(pi, fns->ti);
}

void
BCMATTACHFN(phy_type_reset_impl)(phy_type_disp_t *disp)
{
	phy_type_fns_t *fns = disp->fns;
	phy_info_t *pi = disp->pi;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (fns->reset_impl != NULL)
		(fns->reset_impl)(pi, fns->ti);
}

int
WLBANDINITFN(phy_type_init_impl)(phy_type_disp_t *disp)
{
	phy_type_fns_t *fns = disp->fns;
	phy_info_t *pi = disp->pi;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (fns->init_impl == NULL)
		return BCME_OK;

	return (fns->init_impl)(pi, fns->ti);
}

/* register PHY type specific implementations' iovar tables/handlers */
int
BCMATTACHFN(phy_type_register_iovt)(phy_type_disp_t *disp, wlc_iocv_info_t *ii)
{
	phy_type_fns_t *fns = disp->fns;
	phy_info_t *pi = disp->pi;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (fns->reg_iovt != NULL)
		return (fns->reg_iovt)(pi, fns->ti, ii);

	return BCME_OK;
}

/* register PHY type specific implementations' ioctl tables/handlers */
int
BCMATTACHFN(phy_type_register_ioct)(phy_type_disp_t *disp, wlc_iocv_info_t *ii)
{
	phy_type_fns_t *fns = disp->fns;
	phy_info_t *pi = disp->pi;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (fns->reg_ioct != NULL)
		return (fns->reg_ioct)(pi, fns->ti, ii);

	return BCME_OK;
}

#if ((defined(BCMDBG) || defined(BCMDBG_DUMP)) && defined(DBG_PHY_IOV)) || \
	defined(BCMDBG_PHYDUMP)
/* read phy type specific phy register */
uint16
phy_type_read_phyreg(phy_type_disp_t *disp, uint addr)
{
	phy_info_t *pi = disp->pi;
	phy_type_fns_t *fns = disp->fns;

	if (fns->read_phyreg != NULL)
		return (fns->read_phyreg)(pi, fns->ti, addr);

	return phy_utils_read_phyreg(pi, (uint16)addr);
}

/* dump phy type specific phy registers */
int
phy_type_dump_phyregs(phy_type_disp_t *disp, struct bcmstrbuf *b)
{
	phy_info_t *pi = disp->pi;
	phy_type_fns_t *fns = disp->fns;
	int ret = BCME_UNSUPPORTED;

	if (fns->dump_phyregs != NULL) {
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		phy_utils_phyreg_enter(pi);

		ret = (fns->dump_phyregs)(pi, fns->ti, b);

		phy_utils_phyreg_exit(pi);
		wlapi_enable_mac(pi->sh->physhim);
	}

	return ret;
}
#endif // endif
