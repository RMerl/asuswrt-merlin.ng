/*
 * IOCV module interface - ioctl/iovar table registration.
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

#ifndef _wlc_iocv_reg_h_
#define _wlc_iocv_reg_h_

#if !defined(WLC_HIGH) && !defined(WLC_LOW)
#error "neither WLC_HIGH nor WLC_LOW is defined"
#endif // endif

#include <wlc_cfg.h>
#include <typedefs.h>

#include <wlc_iocv_types.h>

/* invalid table id */
#define WLC_IOCV_TID_INV ((uint16)(~0))

#ifdef WLC_HIGH
#include <wlc_types.h>
#include <bcmutils.h>
#ifdef WLC_HIGH_ONLY
#include <bcm_xdr.h>
/* RPC pack/unpack and param/result proc. callbacks.
 * Return TRUE to indicate it has been packed.
 */

/* iovar params pack/proc function */
typedef bool (*wlc_iov_cmd_fn_t)(wlc_info_t *wlc, uint32 aid,
	void *p, uint plen, bcm_xdr_buf_t *b);
/* iovar result unpack/proc function */
typedef bool (*wlc_iov_res_fn_t)(wlc_info_t *wlc, uint32 aid,
	bcm_xdr_buf_t *b, void *a, uint alen);

/* ioctl params pack/proc function */
typedef bool (*wlc_ioc_cmd_fn_t)(wlc_info_t *wlc, uint16 cid,
	void *a, uint alen, bcm_xdr_buf_t *b);
/* ioctl result unpack/proc function */
typedef bool (*wlc_ioc_res_fn_t)(wlc_info_t *wlc, uint16 cid,
	bcm_xdr_buf_t *b, void *a, uint alen);

#endif /* WLC_HIGH_ONLY */

/* ioctl state validate function */
typedef int (*wlc_ioc_vld_fn_t)(wlc_info_t *wlc, uint16 cid,
	void *a, uint alen);
#endif /* WLC_HIGH */

/* iovar table descriptor */
typedef struct {
#ifdef WLC_HIGH
	/* table pointer */
	const bcm_iovar_t *iovt;
#ifdef WLC_HIGH_ONLY
	/* proc callbacks */
	wlc_iov_cmd_fn_t cmd_proc_fn;
	wlc_iov_res_fn_t res_proc_fn;
#endif // endif
#endif /* WLC_HIGH */
	/* dispatch callback */
	wlc_iov_disp_fn_t disp_fn;
	void *ctx;
} wlc_iovt_desc_t;

/* ioctl table descriptor */
typedef struct {
#ifdef WLC_HIGH
	/* table pointer */
	const wlc_ioctl_cmd_t *ioct;
	uint num_cmds;
#ifdef WLC_HIGH_ONLY
	/* proc callbacks */
	wlc_ioc_cmd_fn_t cmd_proc_fn;
	wlc_ioc_res_fn_t res_proc_fn;
#endif // endif
	wlc_ioc_vld_fn_t st_vld_fn;
#endif /* WLC_HIGH */
	/* dispatch callback */
	wlc_ioc_disp_fn_t disp_fn;
	void *ctx;
} wlc_ioct_desc_t;

/* init iovar table desc */
#if defined(WLC_HIGH) && defined(WLC_LOW)
#define _wlc_iocv_init_iovd_(_iovt_, _cmdfn_, _resfn_, _dispfn_, _ctx_, _iovd_) do { \
		(_iovd_)->iovt = _iovt_;				\
		(_iovd_)->disp_fn = _dispfn_;				\
		(_iovd_)->ctx = _ctx_;					\
	} while (FALSE)
#elif defined(WLC_HIGH_ONLY)
#define _wlc_iocv_init_iovd_(_iovt_, _cmdfn_, _resfn_, _dispfn_, _ctx_, _iovd_) do { \
		(_iovd_)->iovt = _iovt_;				\
		(_iovd_)->cmd_proc_fn = _cmdfn_;			\
		(_iovd_)->res_proc_fn = _resfn_;			\
		(_iovd_)->disp_fn = NULL;				\
		(_iovd_)->ctx = NULL; (void)_ctx_;			\
	} while (FALSE)
#elif defined(WLC_LOW_ONLY)
#define _wlc_iocv_init_iovd_(_iovt_, _cmdfn_, _resfn_, _dispfn_, _ctx_, _iovd_) do { \
		(_iovd_)->disp_fn = _dispfn_;				\
		(_iovd_)->ctx = _ctx_;					\
	} while (FALSE)
#endif /* WLC_LOW_ONLY */

/* init iovar table descriptor */
#define wlc_iocv_init_iovd(iovt, cmdfn, resfn, dispfn, ctx, iovd) \
	_wlc_iocv_init_iovd_(iovt, cmdfn, resfn, dispfn, ctx, iovd)
/* register bmac/phy iovar table & callbacks */
int wlc_iocv_register_iovt(wlc_iocv_info_t *ii, wlc_iovt_desc_t *iovd);

/* init ioctl table desc */
#if defined(WLC_HIGH) && defined(WLC_LOW)
#define _wlc_iocv_init_iocd_(_ioct_, _sz_, _vldfn_, _cmdfn_, _resfn_, _dispfn_, _ctx_, _iocd_) do {\
		(_iocd_)->ioct = _ioct_;				\
		(_iocd_)->num_cmds = _sz_;				\
		(_iocd_)->st_vld_fn = _vldfn_;				\
		(_iocd_)->disp_fn = _dispfn_;				\
		(_iocd_)->ctx = _ctx_;					\
	} while (FALSE)
#elif defined(WLC_HIGH_ONLY)
#define _wlc_iocv_init_iocd_(_ioct_, _sz_, _vldfn_, _cmdfn_, _resfn_, _dispfn_, _ctx_, _iocd_) do {\
		(_iocd_)->ioct = _ioct_;				\
		(_iocd_)->num_cmds = _sz_;				\
		(_iocd_)->cmd_proc_fn = _cmdfn_;			\
		(_iocd_)->res_proc_fn = _resfn_;			\
		(_iocd_)->st_vld_fn = _vldfn_;				\
		(_iocd_)->disp_fn = NULL;				\
		(_iocd_)->ctx = NULL; (void)_ctx_;			\
	} while (FALSE)
#elif defined(WLC_LOW_ONLY)
#define _wlc_iocv_init_iocd_(_ioct_, _sz_, _vldfn_, _cmdfn_, _resfn_, _dispfn_, _ctx_, _iocd_) do {\
		(_iocd_)->disp_fn = _dispfn_;				\
		(_iocd_)->ctx = _ctx_;					\
	} while (FALSE)
#endif /* WLC_LOW_ONLY */

/* init ioctl table descriptor */
#define wlc_iocv_init_iocd(ioct, sz, vldfn, cmdfn, resfn, dispfn, ctx, iocd) \
	_wlc_iocv_init_iocd_(ioct, sz, vldfn, cmdfn, resfn, dispfn, ctx, iocd)
/* register bmac/phy ioctl table & callbacks */
int wlc_iocv_register_ioct(wlc_iocv_info_t *ii, wlc_ioct_desc_t *iocd);

#endif /* wlc_iocv_reg_h_ */
