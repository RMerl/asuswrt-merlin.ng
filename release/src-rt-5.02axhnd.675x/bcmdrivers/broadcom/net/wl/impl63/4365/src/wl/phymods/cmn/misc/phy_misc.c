/*
 * Miscellaneous module implementation.
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
#include "phy_type_misc.h"
#include <phy_rstr.h>
#include <phy_misc.h>
#include <phy_misc_api.h>

/* forward declaration */
typedef struct phy_misc_mem phy_misc_mem_t;

/* module private states */
struct phy_misc_info {
	phy_info_t			*pi;	/* PHY info ptr */
	phy_type_misc_fns_t	*fns;	/* PHY specific function ptrs */
	phy_misc_mem_t		*mem;	/* Memory layout ptr */
};

/* module private states memory layout */
struct phy_misc_mem {
	phy_misc_info_t cmn_info;
	phy_type_misc_fns_t fns;
/* add other variable size variables here at the end */
};

/* local function declaration */

/* attach/detach */
phy_misc_info_t *
BCMATTACHFN(phy_misc_attach)(phy_info_t *pi)
{
	phy_misc_mem_t	*mem = NULL;
	phy_misc_info_t	*cmn_info = NULL;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate attach info storage */
	if ((mem = phy_malloc(pi, sizeof(phy_misc_mem_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}

	/* Initialize infra params */
	cmn_info = &(mem->cmn_info);
	cmn_info->pi = pi;
	cmn_info->fns = &(mem->fns);
	cmn_info->mem = mem;

	/* Initialize misc params */

	/* Register callbacks */

	return cmn_info;

	/* error */
fail:
	phy_misc_detach(cmn_info);
	return NULL;
}

void
BCMATTACHFN(phy_misc_detach)(phy_misc_info_t *cmn_info)
{
	phy_misc_mem_t	*mem;

	PHY_CAL(("%s\n", __FUNCTION__));

	/* Clean up module related states */

	/* Clean up infra related states */
	/* Malloc has failed. No cleanup is necessary here. */
	if (!cmn_info)
		return;

	/* Freeup memory associated with cmn_info */
	mem = cmn_info->mem;

	if (mem == NULL) {
		PHY_INFORM(("%s: null misc module\n", __FUNCTION__));
		return;
	}

	phy_mfree(cmn_info->pi, mem, sizeof(phy_misc_mem_t));
}

/* register phy type specific implementations */
int
BCMATTACHFN(phy_misc_register_impl)(phy_misc_info_t *mi, phy_type_misc_fns_t *fns)
{
	PHY_TRACE(("%s\n", __FUNCTION__));

	*mi->fns = *fns;

	return BCME_OK;
}

void
BCMATTACHFN(phy_misc_unregister_impl)(phy_misc_info_t *mi)
{
	PHY_TRACE(("%s\n", __FUNCTION__));
}

/* driver down processing */
int
phy_misc_down(phy_misc_info_t *mi)
{
	int callbacks = 0;

	PHY_TRACE(("%s\n", __FUNCTION__));

	return callbacks;
}

/*
 * Return vasip version, -1 if not present.
 */
uint8
phy_misc_get_vasip_ver(phy_info_t *pi)
{
	phy_type_misc_fns_t *fns = pi->misci->fns;

	if (fns->phy_type_vasip_get_ver != NULL)
		return fns->phy_type_vasip_get_ver(fns->ctx);
	else
		return VASIP_NOVERSION;
}

/*
 * reset/activate vasip.
 */
void
phy_misc_vasip_proc_reset(phy_info_t *pi, int reset)
{
	phy_type_misc_fns_t *fns = pi->misci->fns;

	if (fns->phy_type_vasip_proc_reset != NULL)
		fns->phy_type_vasip_proc_reset(fns->ctx, reset);
}

void
phy_misc_vasip_clk_set(phy_info_t *pi, bool val)
{
	phy_type_misc_fns_t *fns = pi->misci->fns;

	if (fns->phy_type_vasip_clk_set != NULL)
		fns->phy_type_vasip_clk_set(fns->ctx, val);
}

void
phy_misc_vasip_bin_write(phy_info_t *pi, const uint32 vasip_code[], const uint nbytes)
{
	phy_type_misc_fns_t *fns = pi->misci->fns;

	if (fns->phy_type_vasip_bin_write != NULL) {
		fns->phy_type_vasip_bin_write(fns->ctx, vasip_code, nbytes);
	}
}

#ifdef VASIP_SPECTRUM_ANALYSIS
void
phy_misc_vasip_spectrum_tbl_write(phy_info_t *pi,
        const uint32 vasip_spectrum_tbl[], const uint nbytes_tbl)
{
	phy_type_misc_fns_t *fns = pi->misci->fns;

	if (fns->phy_type_vasip_spectrum_tbl_write != NULL) {
		fns->phy_type_vasip_spectrum_tbl_write(fns->ctx, vasip_spectrum_tbl, nbytes_tbl);
	}
}
#endif // endif

void
phy_misc_vasip_svmp_write(phy_info_t *pi, uint32 offset, uint16 val)
{
	phy_type_misc_fns_t *fns = pi->misci->fns;

	if (fns->phy_type_vasip_svmp_write != NULL)
		fns->phy_type_vasip_svmp_write(fns->ctx, offset, val);
}

uint16
phy_misc_vasip_svmp_read(phy_info_t *pi, uint32 offset)
{
	phy_type_misc_fns_t *fns = pi->misci->fns;

	return fns->phy_type_vasip_svmp_read(fns->ctx, offset);
}

void
phy_misc_vasip_svmp_write_blk(phy_info_t *pi, uint32 offset, uint16 len, uint16 *val)
{
	phy_type_misc_fns_t *fns = pi->misci->fns;

	if (fns->phy_type_vasip_svmp_write_blk != NULL)
		fns->phy_type_vasip_svmp_write_blk(fns->ctx, offset, len, val);
}

void
phy_misc_vasip_svmp_read_blk(phy_info_t *pi, uint32 offset, uint16 len, uint16 *val)
{
	phy_type_misc_fns_t *fns = pi->misci->fns;

	if (fns->phy_type_vasip_svmp_read_blk != NULL)
		fns->phy_type_vasip_svmp_read_blk(fns->ctx, offset, len, val);
}
