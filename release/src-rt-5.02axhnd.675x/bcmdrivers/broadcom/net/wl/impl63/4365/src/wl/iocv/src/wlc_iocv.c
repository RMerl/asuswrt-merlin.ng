/*
 * IOCV module implementation - ioctl/iovar dispatcher registry
 * For BMAC/PHY.
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

/* XXX: Define wlc_cfg.h to be the first header file included as some builds
 * get their feature flags thru this file.
 */
#include <wlc_cfg.h>

#ifdef WLC_LOW

#include <typedefs.h>
#include <bcmutils.h>
#include <wlioctl.h>
#include <wl_dbg.h>
#include <wlc_types.h>
#include <siutils.h>
#include <sbhnddma.h>
#include <hnddma.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc.h>
#include <wlc_hw_priv.h>

#include "wlc_iocv_cfg.h"
#include "wlc_iocv_if.h"
#include <wlc_iocv_types.h>
#include <wlc_iocv.h>

/* iovar handler entry */
typedef struct {
	wlc_iov_disp_fn_t disp_fn;	/* iovar dispatch callback */
	void *ctx;		/* context passed to callback */
} wlc_iov_ent_t;

/* ioctl handler entry */
typedef struct {
	wlc_ioc_disp_fn_t disp_fn;	/* ioctl dispatch callback */
	void *ctx;		/* context passed to callback */
} wlc_ioc_ent_t;

/* module private states */
typedef struct {
	wlc_hw_info_t *hw;

	/* iovar table registry */
	uint16 iov_cnt;
	uint16 iov_max;
	wlc_iov_ent_t *iov;

	/* ioctl table registry */
	uint16 ioc_cnt;
	uint16 ioc_max;
	wlc_ioc_ent_t *ioc;
} wlc_iocv_low_t;

/* module private states memory layout */
typedef struct {
	wlc_iocv_low_t low;
	wlc_iov_ent_t iov[WLC_IOVT_REG_SZ];
	wlc_ioc_ent_t ioc[WLC_IOCT_REG_SZ];
	wlc_iocv_info_t ii;
/* add other variable size variables here at the end */
} wlc_iocv_mem_t;

/* helper macros */
#define WLC_IOCV_LOW(ii) ((wlc_iocv_low_t *)(ii)->obj)

/* local functions */
static int wlc_iocv_reg_iovt(wlc_iocv_info_t *ii, wlc_iovt_desc_t *iovd);
static int wlc_iocv_reg_ioct(wlc_iocv_info_t *ii, wlc_ioct_desc_t *iocd);

/* attach/detach */
wlc_iocv_info_t *
BCMATTACHFN(wlc_iocv_attach)(wlc_hw_info_t *hw)
{
	wlc_iocv_low_t *low;
	wlc_iov_ent_t *iov;
	wlc_ioc_ent_t *ioc;
	wlc_iocv_info_t *ii;

	/* allocate storage */
	if ((low = MALLOC(hw->osh, sizeof(wlc_iocv_mem_t))) == NULL) {
		WL_ERROR(("%s: MALLOC failed\n", __FUNCTION__));
		return NULL;
	}
	bzero(low, sizeof(wlc_iocv_mem_t));
	low->hw = hw;

	/* init the common info struct */
	ii = &((wlc_iocv_mem_t *)low)->ii;
	ii->iovt_reg_fn = wlc_iocv_reg_iovt;
	ii->ioct_reg_fn = wlc_iocv_reg_ioct;
	ii->obj = low;

	/* init the iov registry */
	iov = ((wlc_iocv_mem_t *)low)->iov;
	low->iov_max = WLC_IOVT_REG_SZ;
	low->iov = iov;

	/* init the ioc registry */
	ioc = ((wlc_iocv_mem_t *)low)->ioc;
	low->ioc_max = WLC_IOCT_REG_SZ;
	low->ioc = ioc;

	return ii;
}

void
BCMATTACHFN(wlc_iocv_detach)(wlc_iocv_info_t *ii)
{
	wlc_iocv_low_t *low;
	wlc_hw_info_t *hw;

	ASSERT(ii != NULL);

	low = WLC_IOCV_LOW(ii);
	ASSERT(low != NULL);

	hw = low->hw;

	MFREE(hw->osh, low, sizeof(wlc_iocv_mem_t));
}

/* register a single iovar table & callbacks */
static int
BCMATTACHFN(wlc_iocv_reg_iovt)(wlc_iocv_info_t *ii, wlc_iovt_desc_t *iovd)
{
	wlc_iocv_low_t *low;

	low = WLC_IOCV_LOW(ii);
	ASSERT(low != NULL);

	if (low->iov_cnt == low->iov_max) {
		WL_ERROR(("%s: too many iovar tables\n", __FUNCTION__));
		return BCME_NORESOURCE;
	}

	low->iov[low->iov_cnt].disp_fn = iovd->disp_fn;
	low->iov[low->iov_cnt].ctx = iovd->ctx;

	low->iov_cnt ++;

	return BCME_OK;
}

/* register a single ioctl table & callbacks */
static int
BCMATTACHFN(wlc_iocv_reg_ioct)(wlc_iocv_info_t *ii, wlc_ioct_desc_t *iocd)
{
	wlc_iocv_low_t *low;

	low = WLC_IOCV_LOW(ii);
	ASSERT(low != NULL);

	if (low->ioc_cnt == low->ioc_max) {
		WL_ERROR(("%s: too many ioctl tables\n", __FUNCTION__));
		return BCME_NORESOURCE;
	}

	low->ioc[low->ioc_cnt].disp_fn = iocd->disp_fn;
	low->ioc[low->ioc_cnt].ctx = iocd->ctx;

	low->ioc_cnt ++;

	return BCME_OK;
}

/* dispatch iovar to registered module dispatch callback */
int
wlc_iocv_dispatch_iov(wlc_iocv_info_t *ii, uint16 tid, uint32 aid,
	uint8 *p, uint p_len, uint8 *a, uint a_len, uint var_sz)
{
	wlc_iocv_low_t *low;

	low = WLC_IOCV_LOW(ii);
	ASSERT(low != NULL);

	ASSERT(tid < low->iov_cnt);
	ASSERT(low->iov[tid].disp_fn != NULL);

	return (low->iov[tid].disp_fn)(low->iov[tid].ctx, aid, p, p_len, a, a_len, var_sz);
}

/* dispatch ioctl to registered module dispatch callback */
int
wlc_iocv_dispatch_ioc(wlc_iocv_info_t *ii, uint16 tid, uint16 cid, uint8 *a, uint a_len, bool *ta)
{
	wlc_iocv_low_t *low;

	low = WLC_IOCV_LOW(ii);
	ASSERT(low != NULL);

	ASSERT(tid < low->ioc_cnt);
	ASSERT(low->ioc[tid].disp_fn != NULL);

	return (low->ioc[tid].disp_fn)(low->ioc[tid].ctx, cid, a, a_len, ta);
}

#endif /* WLC_LOW */
