/*
 * IOCV module implementation - ioctl/iovar table registry.
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
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlioctl.h>
#include <wlc_pub.h>
#include <wlc.h>
#include <wlc_bmac.h>
#include <wl_dbg.h>

#ifndef WLC_HIGH
#error "WLC_HIGH is not defined"
#endif // endif

#include "wlc_iocv_cfg.h"
#include "wlc_iocv_if.h"
#include <wlc_iocv_types.h>
#include <wlc_iocv_high.h>

/* 1. ioctl/iovar registry entry - wlc_io[cv]_ent_t
 *
 * 'rpc procs' - callbacks to fixup iovar/ioctl params before and result after rpc call.
 * Its purpose is to unify the rpc call pack/unpack.
 * One can use this pair of callbacks to:
 *
 *    1. pack IOVT_BUFFER type of iovar/ioctl parameter; unpack IOVT_BUFFER type of result.
 *    2. fix endianness of iovar parameters (h2d); fix endianness of results (d2h).
 *
 * 'dispatch proc' - callback to process the iovar/ioctl params and return the result.
 *
 * 2. ioctl/iovar registry - io[cv]_cnt/io[cv]_max/io[cv]_wlc/io[cv]
 *
 * All entries are stored in the registry. The number of available entries is 'io[cv]_max.
 * The number of used entries is 'io[cv]_cnt (from index 0 in 'io[cv]' array).
 *
 * The 'hdlr' struct in the first N entries (N = 'io[cv]_wlc') are forced to be
 * 'rpc proc' and ioctl/iovar in the registered tables are forwarded to bmac.
 *
 * The 'hdlr' struct in all other entries (from 'io[cv]_wlc' to 'io[cv]_cnt' - 1)
 * are 'dispatch proc' of wlc modules.
 */

/* iovar handler entry */
typedef struct {
	/* table */
	const bcm_iovar_t *iovt;
	union {
#ifdef WLC_HIGH_ONLY
		/* rpc procs */
		struct {
			wlc_iov_cmd_fn_t cmd_fn;	/* iovar command fixup callback */
			wlc_iov_res_fn_t res_fn;	/* iovar result fixup callback */
		} rpc;
#endif // endif
		/* dispatch proc */
		struct {
			wlc_iov_disp_fn_t fn;
			void *ctx;
		} disp;
	} hdlr;
} wlc_iov_high_ent_t;

/* ioctl handler entry */
typedef struct {
	/* table */
	const wlc_ioctl_cmd_t *ioct;
	uint num_cmds;
	/* handlers */
	wlc_ioc_vld_fn_t vld_fn;	/* ioctl validation callback */
	union {
#ifdef WLC_HIGH_ONLY
		/* rpc procs */
		struct {
			wlc_ioc_cmd_fn_t cmd_fn;	/* ioctl command fixup callback */
			wlc_ioc_res_fn_t res_fn;	/* ioctl result fixup callback */
		} rpc;
#endif // endif
		/* dispatch proc */
		struct {
			wlc_ioc_disp_fn_t fn;
			void *ctx;
		} disp;
	} hdlr;
} wlc_ioc_high_ent_t;

/* module private states */
typedef struct {
	wlc_info_t *wlc;

	/* iovar table registry */
	uint16 iov_cnt;
	uint16 iov_max;
	uint16 iov_wlc;		/* the first wlc only table */
	wlc_iov_high_ent_t *iov;

	/* ioctl table registry */
	uint16 ioc_cnt;
	uint16 ioc_max;
	uint16 ioc_wlc;		/* the first wlc only table */
	wlc_ioc_high_ent_t *ioc;
} wlc_iocv_high_t;

/* module private states memory layout */
typedef struct {
	wlc_iocv_high_t high;
	wlc_iov_high_ent_t iov[WLC_IOVT_REG_SZ];
	wlc_ioc_high_ent_t ioc[WLC_IOCT_REG_SZ];
	wlc_iocv_info_t ii;
/* add other variable size variables here at the end */
} wlc_iocv_mem_t;

/* helper macros */
#define WLC_IOCV_HIGH(ii) ((wlc_iocv_high_t *)(ii)->obj)

/* debug macros */
#define WL_IOCV(x) WL_NONE(x)

/* local functions */
static int wlc_iocv_high_reg_iovt(wlc_iocv_info_t *ii, wlc_iovt_desc_t *iovd);
static int wlc_iocv_high_reg_ioct(wlc_iocv_info_t *ii, wlc_ioct_desc_t *iocd);
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static int wlc_iocv_high_dump(void *ctx, struct bcmstrbuf *b);
#endif // endif

/* attach/detach */
wlc_iocv_info_t *
BCMATTACHFN(wlc_iocv_high_attach)(wlc_info_t *wlc)
{
	wlc_iocv_high_t *high;
	wlc_iov_high_ent_t *iov;
	wlc_ioc_high_ent_t *ioc;
	wlc_iocv_info_t *ii;

	/* allocate storage */
	if ((high = MALLOC(wlc->osh, sizeof(wlc_iocv_mem_t))) == NULL) {
		WL_ERROR(("%s: MALLOC failed\n", __FUNCTION__));
		return NULL;
	}
	bzero(high, sizeof(wlc_iocv_mem_t));
	high->wlc = wlc;

	/* init the common info struct */
	ii = &((wlc_iocv_mem_t *)high)->ii;
	/* for bmac and phy tables */
	ii->iovt_reg_fn = wlc_iocv_high_reg_iovt;
	ii->ioct_reg_fn = wlc_iocv_high_reg_ioct;
	ii->obj = high;

	/* init the iov registry */
	iov = ((wlc_iocv_mem_t *)high)->iov;
	high->iov_max = WLC_IOVT_REG_SZ;
	high->iov = iov;

	/* init the ioc registry */
	ioc = ((wlc_iocv_mem_t *)high)->ioc;
	high->ioc_max = WLC_IOCT_REG_SZ;
	high->ioc = ioc;

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
	wlc_dump_register(wlc->pub, "iocv", wlc_iocv_high_dump, (void *)high);
#endif // endif

	return ii;
}

void
BCMATTACHFN(wlc_iocv_high_detach)(wlc_iocv_info_t *ii)
{
	wlc_iocv_high_t *high;
	wlc_info_t *wlc;

	ASSERT(ii != NULL);

	high = WLC_IOCV_HIGH(ii);
	ASSERT(high != NULL);

	wlc = high->wlc;

	MFREE(wlc->osh, high, sizeof(wlc_iocv_mem_t));
}

/* register iovar table (for BMAC and PHY) */
static int
BCMATTACHFN(wlc_iocv_high_reg_iovt)(wlc_iocv_info_t *ii, wlc_iovt_desc_t *iovd)
{
	wlc_iocv_high_t *high;
	uint16 idx;

	high = WLC_IOCV_HIGH(ii);
	ASSERT(high != NULL);

	if (high->iov_cnt == high->iov_max) {
		WL_ERROR(("%s: too many iovar tables\n", __FUNCTION__));
		return BCME_NORESOURCE;
	}

	ASSERT(iovd->iovt != NULL);

	idx = high->iov_wlc;

	/* move wlc only tables down towards the end of the table */
	if (idx < high->iov_cnt) {
		memmove(&high->iov[idx + 1], &high->iov[idx],
		        (high->iov_cnt - idx) * sizeof(wlc_iov_high_ent_t));
	}

	high->iov_wlc ++;

	high->iov[idx].iovt = iovd->iovt;
#ifdef WLC_HIGH_ONLY
	high->iov[idx].hdlr.rpc.cmd_fn = iovd->cmd_proc_fn;
	high->iov[idx].hdlr.rpc.res_fn = iovd->res_proc_fn;
#endif // endif

	high->iov_cnt ++;

	return BCME_OK;
}

/* register ioctl table (for BMAC and PHY) */
static int
BCMATTACHFN(wlc_iocv_high_reg_ioct)(wlc_iocv_info_t *ii, wlc_ioct_desc_t *iocd)
{
	wlc_iocv_high_t *high;
	uint16 idx;

	high = WLC_IOCV_HIGH(ii);
	ASSERT(high != NULL);

	if (high->ioc_cnt == high->ioc_max) {
		WL_ERROR(("%s: too many ioctl tables\n", __FUNCTION__));
		return BCME_NORESOURCE;
	}

	ASSERT(iocd->ioct != NULL);

	idx = high->ioc_wlc;

	/* move wlc only tables down towards the end of the table */
	if (idx < high->ioc_cnt) {
		memmove(&high->ioc[idx + 1], &high->ioc[idx],
		        (high->ioc_cnt - idx) * sizeof(wlc_ioc_high_ent_t));
	}

	high->ioc_wlc ++;

	high->ioc[idx].ioct = iocd->ioct;
	high->ioc[idx].num_cmds = iocd->num_cmds;
	high->ioc[idx].vld_fn = iocd->st_vld_fn;
#ifdef WLC_HIGH_ONLY
	high->ioc[idx].hdlr.rpc.cmd_fn = iocd->cmd_proc_fn;
	high->ioc[idx].hdlr.rpc.res_fn = iocd->res_proc_fn;
#endif // endif

	high->ioc_cnt ++;

	return BCME_OK;
}

/* register iovar table (for WLC) */
int
BCMATTACHFN(wlc_iocv_high_register_iovt)(wlc_iocv_info_t *ii,
	const bcm_iovar_t *iovt,
	wlc_iov_disp_fn_t disp_fn, void *ctx)
{
	wlc_iocv_high_t *high;
	uint16 idx;

	high = WLC_IOCV_HIGH(ii);
	ASSERT(high != NULL);

	if (high->iov_cnt == high->iov_max) {
		WL_ERROR(("%s: too many iovar tables\n", __FUNCTION__));
		return BCME_NORESOURCE;
	}

	ASSERT(iovt != NULL);
	ASSERT(disp_fn != NULL);

	idx = high->iov_cnt;

	high->iov[idx].iovt = iovt;
	high->iov[idx].hdlr.disp.fn = disp_fn;
	high->iov[idx].hdlr.disp.ctx = ctx;

	high->iov_cnt ++;

	return BCME_OK;
}

/* register ioctl table (for WLC) */
int
BCMATTACHFN(wlc_iocv_high_register_ioct)(wlc_iocv_info_t *ii,
	const wlc_ioctl_cmd_t *ioct, uint num_cmds,
	wlc_ioc_disp_fn_t disp_fn, void *ctx)
{
	wlc_iocv_high_t *high;
	uint16 idx;

	high = WLC_IOCV_HIGH(ii);
	ASSERT(high != NULL);

	if (high->ioc_cnt == high->ioc_max) {
		WL_ERROR(("%s: too many ioctl tables\n", __FUNCTION__));
		return BCME_NORESOURCE;
	}

	ASSERT(ioct != NULL);
	ASSERT(disp_fn != NULL);

	idx = high->ioc_cnt;

	high->ioc[idx].ioct = ioct;
	high->ioc[idx].num_cmds = num_cmds;
	high->ioc[idx].hdlr.disp.fn = disp_fn;
	high->ioc[idx].hdlr.disp.ctx = ctx;

	high->ioc_cnt ++;

	return BCME_OK;
}

/* lookup iovar and return iovar entry and table id if found */
const bcm_iovar_t *
wlc_iocv_high_find_iov(wlc_iocv_info_t *ii, const char *name, uint16 *tid)
{
	wlc_iocv_high_t *high;
	const bcm_iovar_t *vi;
	uint16 i;

	high = WLC_IOCV_HIGH(ii);
	ASSERT(high != NULL);

	for (i = 0; i < high->iov_cnt; i ++) {
		if ((vi = wlc_iovar_lookup(high->iov[i].iovt, name)) != NULL) {
			/* found entry in the table */
			*tid = i;
			return vi;
		}
	}

	*tid = WLC_IOCV_TID_INV;
	return NULL;
}

/* forward iovar to registered module callbacks */
int
wlc_iocv_high_fwd_iov(wlc_iocv_info_t *ii, uint16 tid, uint32 aid, const bcm_iovar_t *vi,
	uint8 *p, uint p_len, uint8 *a, uint a_len, uint var_sz)
{
	wlc_iocv_high_t *high;
	wlc_info_t *wlc;
	int err;

	high = WLC_IOCV_HIGH(ii);
	ASSERT(high != NULL);

	ASSERT(tid < high->iov_cnt);

	wlc = high->wlc;

	/* forward to BMAC (RPC if needed) */
	if (tid < high->iov_wlc) {
		if ((err = wlc_bmac_dispatch_iov(wlc->hw, tid, aid, vi->type,
		                                 p, p_len, a, a_len, var_sz)) != BCME_OK) {
			WL_IOCV(("%s: wlc_bmac_dispatch_iov failure, aid %u\n",
			         __FUNCTION__, aid));
			goto exit;
		}
	}
	/* forward to registered dispatch callback */
	else {
		ASSERT(high->iov[tid].hdlr.disp.fn != NULL);

		if ((err = (high->iov[tid].hdlr.disp.fn)(high->iov[tid].hdlr.disp.ctx, aid,
		                                         p, p_len, a, a_len, var_sz)) != BCME_OK) {
			WL_IOCV(("%s: fn %p failed, aid %u\n",
			         __FUNCTION__, high->iov[tid].hdlr.disp.fn, aid));
			goto exit;
		}
	}

exit:
	return err;
}

/* lookup ioctl and return ioctl entry and table id if found */
const wlc_ioctl_cmd_t *
wlc_iocv_high_find_ioc(wlc_iocv_info_t *ii, uint16 cid, uint16 *tid)
{
	wlc_iocv_high_t *high;
	uint16 i, j;

	high = WLC_IOCV_HIGH(ii);
	ASSERT(high != NULL);

	for (i = 0; i < high->ioc_cnt; i ++) {
		for (j = 0; j < high->ioc[i].num_cmds; j++) {
			if (cid == high->ioc[i].ioct[j].cmd) {
				/* found command 'cid' in the table */
				*tid = i;
				return &high->ioc[i].ioct[j];
			}
		}
	}

	*tid = WLC_IOCV_TID_INV;
	return NULL;
}

/* forward ioctl to registered module callbacks */
int
wlc_iocv_high_fwd_ioc(wlc_iocv_info_t *ii, uint16 tid, const wlc_ioctl_cmd_t *ci,
	uint8 *a, uint a_len)
{
	wlc_iocv_high_t *high;
	wlc_info_t *wlc;
	uint16 cid;
	bool ta_ok = FALSE;
	int err;

	high = WLC_IOCV_HIGH(ii);
	ASSERT(high != NULL);

	ASSERT(tid < high->ioc_cnt);

	wlc = high->wlc;
	cid = ci->cmd;

	if ((err = wlc_iocv_high_vld_ioc(ii, tid, ci, a, a_len)) != BCME_OK) {
		WL_IOCV(("%s: wlc_iocv_high_vld_ioc failed, tid %u cid %u\n",
		         __FUNCTION__, tid, cid));
		goto exit;
	}

	/* forward to BMAC (RPC if needed) */
	if (tid < high->ioc_wlc) {
		if ((err = wlc_bmac_dispatch_ioc(wlc->hw, tid, cid, 0,
		                                 a, a_len, &ta_ok)) != BCME_OK) {
			WL_IOCV(("%s: wlc_bmac_dispatch_ioc failure, cid %u\n",
			         __FUNCTION__, cid));
			goto exit;
		}
	}
	/* forward to registered dispatch callback */
	else {
		ASSERT(high->ioc[tid].hdlr.disp.fn != NULL);

		if ((err = (high->ioc[tid].hdlr.disp.fn)(high->ioc[tid].hdlr.disp.ctx, cid,
		                                         a, a_len, &ta_ok)) != BCME_OK) {
			WL_IOCV(("%s: fn %p failed, cid %u\n",
			         __FUNCTION__, high->ioc[tid].hdlr.disp.fn, cid));
			goto exit;
		}
	}

exit:
	/* The sequence is significant.
	 *   Only if sbclk is TRUE, we can proceed with register access.
	 *   Even though ta_ok is TRUE, we still want to check(and clear) target abort
	 *   si_taclear returns TRUE if there was a target abort, In this case, ta_ok must be TRUE
	 *   to avoid assert
	 *   ASSERT and si_taclear are both under ifdef BCMDBG
	 */
#ifdef WLC_LOW
	/* BMAC_NOTE: for HIGH_ONLY driver, this seems being called after RPC bus failed */
	/* In hw_off condition, IOCTLs that reach here are deemed safe but taclear would
	 * certainly result in getting -1 for register reads. So skip ta_clear altogether
	 */
	if (!(wlc->pub->hw_off))
		ASSERT(wlc_bmac_taclear(wlc->hw, ta_ok) || !ta_ok);
#endif // endif

	return err;
}

/* validate ioctl */
int
wlc_iocv_high_vld_ioc(wlc_iocv_info_t *ii, uint16 tid, const wlc_ioctl_cmd_t *ci,
	void *a, uint a_len)
{
	wlc_iocv_high_t *high;
	wlc_info_t *wlc;
	uint16 cid;

	high = WLC_IOCV_HIGH(ii);
	ASSERT(high != NULL);

	ASSERT(tid < high->ioc_cnt);

	if (high->ioc[tid].vld_fn == NULL)
		return BCME_OK;

	wlc = high->wlc;
	cid = ci->cmd;

	return (high->ioc[tid].vld_fn)(wlc, cid, a, a_len);
}

#ifdef WLC_HIGH_ONLY
/* pack iovar parameters in buffer */
bool
wlc_iocv_high_pack_iov(wlc_iocv_info_t *ii, uint16 tid, uint32 aid,
	void *p, uint p_len, bcm_xdr_buf_t *b)
{
	wlc_iocv_high_t *high;
	wlc_info_t *wlc;

	high = WLC_IOCV_HIGH(ii);
	ASSERT(high != NULL);

	ASSERT(tid < high->iov_wlc);

	wlc = high->wlc;

	/* fixup cmd */
	return high->iov[tid].hdlr.rpc.cmd_fn != NULL &&
	        (high->iov[tid].hdlr.rpc.cmd_fn)(wlc, aid, p, p_len, b);
}

/* unpack iovar results in buffer */
bool
wlc_iocv_high_unpack_iov(wlc_iocv_info_t *ii, uint16 tid, uint32 aid,
	bcm_xdr_buf_t *b, void *a, uint a_len)
{
	wlc_iocv_high_t *high;
	wlc_info_t *wlc;

	high = WLC_IOCV_HIGH(ii);
	ASSERT(high != NULL);

	ASSERT(tid < high->iov_wlc);

	wlc = high->wlc;

	/* fixup result */
	return high->iov[tid].hdlr.rpc.res_fn != NULL &&
	        (high->iov[tid].hdlr.rpc.res_fn)(wlc, aid, b, a, a_len);
}

/* pack ioctl parameters */
bool
wlc_iocv_high_pack_ioc(wlc_iocv_info_t *ii, uint16 tid, uint16 cid,
	void *a, uint a_len, bcm_xdr_buf_t *b)
{
	wlc_iocv_high_t *high;
	wlc_info_t *wlc;

	high = WLC_IOCV_HIGH(ii);
	ASSERT(high != NULL);

	ASSERT(tid < high->ioc_wlc);

	wlc = high->wlc;

	/* fixup cmd */
	return high->ioc[tid].hdlr.rpc.cmd_fn != NULL &&
	        (high->ioc[tid].hdlr.rpc.cmd_fn)(wlc, cid, a, a_len, b);
}

/* unpack ioctl results */
bool
wlc_iocv_high_unpack_ioc(wlc_iocv_info_t *ii, uint16 tid, uint16 cid,
	bcm_xdr_buf_t *b, void *a, uint a_len)
{
	wlc_iocv_high_t *high;
	wlc_info_t *wlc;

	high = WLC_IOCV_HIGH(ii);
	ASSERT(high != NULL);

	ASSERT(tid < high->ioc_wlc);

	wlc = high->wlc;

	/* fixup result */
	return high->ioc[tid].hdlr.rpc.res_fn != NULL &&
	        (high->ioc[tid].hdlr.rpc.res_fn)(wlc, cid, b, a, a_len);
}
#endif /* WLC_HIGH_ONLY */

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static int
wlc_iocv_high_dump(void *ctx, struct bcmstrbuf *b)
{
	wlc_iocv_high_t *high = (wlc_iocv_high_t *)ctx;
	uint16 tid;

	bcm_bprintf(b, "IOVAR Tables: cnt %u max %u\n", high->iov_cnt, high->iov_max);
	bcm_bprintf(b, "  Entries from LOW: cnt %u\n", high->iov_wlc);
	for (tid = 0; tid < high->iov_wlc; tid ++) {
		bcm_bprintf(b, "    idx %u: tbl %p", tid, high->iov[tid].iovt);
		if (high->iov[tid].iovt[0].name != NULL)
			bcm_bprintf(b, " (%s...)", high->iov[tid].iovt[0].name);
#ifdef WLC_HIGH_ONLY
		bcm_bprintf(b, " cmd %p res %p",
		            high->iov[tid].hdlr.rpc.cmd_fn,
		            high->iov[tid].hdlr.rpc.res_fn);
#endif // endif
		bcm_bprintf(b, "\n");
	}
	bcm_bprintf(b, "  Entries from HIGH: cnt %u\n", high->ioc_cnt - high->ioc_wlc);
	for (; tid < high->iov_cnt; tid ++) {
		bcm_bprintf(b, "    idx %u: tbl %p", tid, high->iov[tid].iovt);
		if (high->iov[tid].iovt[0].name != NULL)
			bcm_bprintf(b, " (%s...)", high->iov[tid].iovt[0].name);
		bcm_bprintf(b, "disp %p ctx %p\n",
		            high->iov[tid].hdlr.disp.fn,
		            high->iov[tid].hdlr.disp.ctx);
	}

	bcm_bprintf(b, "IOCTL Tables: cnt %u max %u\n", high->ioc_cnt, high->ioc_max);
	bcm_bprintf(b, "  Entries from LOW: cnt %u\n", high->ioc_wlc);
	for (tid = 0; tid < high->ioc_wlc; tid ++) {
		bcm_bprintf(b, "    idx %u: tbl %p cnt %u",
		            tid, high->ioc[tid].ioct, high->ioc[tid].num_cmds);
		bcm_bprintf(b, " (%u...)", high->ioc[tid].ioct[0].cmd);
		bcm_bprintf(b, " vld %p", high->ioc[tid].vld_fn);
#ifdef WLC_HIGH_ONLY
		bcm_bprintf(b, " cmd %p res %p",
		            high->ioc[tid].hdlr.rpc.cmd_fn,
		            high->ioc[tid].hdlr.rpc.res_fn);
#endif // endif
		bcm_bprintf(b, "\n");
	}
	bcm_bprintf(b, "  Entries from HIGH: cnt %u\n", high->ioc_cnt - high->ioc_wlc);
	for (; tid < high->ioc_cnt; tid ++) {
		bcm_bprintf(b, "    idx %u: tbl %p cnt %u",
		            tid, high->ioc[tid].ioct, high->ioc[tid].num_cmds);
		bcm_bprintf(b, " (%u...)", high->ioc[tid].ioct[0].cmd);
		bcm_bprintf(b, " vld %p", high->ioc[tid].vld_fn);
		bcm_bprintf(b, "disp %p ctx %p\n",
		            high->ioc[tid].hdlr.disp.fn,
		            high->ioc[tid].hdlr.disp.ctx);
	}

	return BCME_OK;
}
#endif /* BCMDBG || BCMDBG_DUMP */
