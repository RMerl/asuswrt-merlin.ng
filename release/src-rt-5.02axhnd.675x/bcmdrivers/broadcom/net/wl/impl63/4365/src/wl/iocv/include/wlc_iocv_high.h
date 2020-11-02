/*
 * IOCV module interface.
 * For WLC.
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

#ifndef _wlc_iocv_high_h_
#define _wlc_iocv_high_h_

#ifndef WLC_HIGH
#error "WLC_HIGH is not defined"
#endif // endif

#include <typedefs.h>
#include <wlc_types.h>
#include <wlc_iocv_types.h>

/* attach/detach */
wlc_iocv_info_t *wlc_iocv_high_attach(wlc_info_t *wlc);
void wlc_iocv_high_detach(wlc_iocv_info_t *ii);

/* register wlc iovar table & dispatcher */
int wlc_iocv_high_register_iovt(wlc_iocv_info_t *ii, const bcm_iovar_t *iovt,
	wlc_iov_disp_fn_t disp_fn, void *ctx);
/* register wlc ioctl table & dispatcher */
int wlc_iocv_high_register_ioct(wlc_iocv_info_t *ii, const wlc_ioctl_cmd_t *ioct, uint num_cmds,
	wlc_ioc_disp_fn_t disp_fn, void *ctx);

/*
 * TODO: Remove unnecessary interface
 * once integrated with existing iovar table handling.
 */

/* lookup iovar and return iovar entry and table id if found */
const bcm_iovar_t *wlc_iocv_high_find_iov(wlc_iocv_info_t *ii, const char *name, uint16 *tid);

/* forward iovar to registered table dispatcher */
int wlc_iocv_high_fwd_iov(wlc_iocv_info_t *ii, uint16 tid, uint32 aid, const bcm_iovar_t *vi,
	uint8 *p, uint p_len, uint8 *a, uint a_len, uint var_sz);

/* lookup ioctl and return ioctl entry and table id if found */
const wlc_ioctl_cmd_t *wlc_iocv_high_find_ioc(wlc_iocv_info_t *ii, uint16 cid, uint16 *tid);

/* forward ioctl to registered table dispatcher */
int wlc_iocv_high_fwd_ioc(wlc_iocv_info_t *ii, uint16 tid, const wlc_ioctl_cmd_t *ci,
	uint8 *a, uint a_len);

/* validate ioctl */
int wlc_iocv_high_vld_ioc(wlc_iocv_info_t *ii, uint16 tid, const wlc_ioctl_cmd_t *ci,
	void *a, uint a_len);

#ifdef WLC_HIGH_ONLY
#include <bcm_xdr.h>

/* pack iovar parameters into buffer */
bool wlc_iocv_high_pack_iov(wlc_iocv_info_t *ii, uint16 tid, uint32 aid,
	void *p, uint p_len, bcm_xdr_buf_t *b);
/* unpack iovar result from buffer */
bool wlc_iocv_high_unpack_iov(wlc_iocv_info_t *ii, uint16 tid, uint32 aid,
	bcm_xdr_buf_t *b, void *a, uint a_len);

/* pack ioctl parameters into buffer */
bool wlc_iocv_high_pack_ioc(wlc_iocv_info_t *ii, uint16 tid, uint16 cid,
	void *a, uint a_len, bcm_xdr_buf_t *b);
/* unpack ioctl result from buffer */
bool wlc_iocv_high_unpack_ioc(wlc_iocv_info_t *ii, uint16 tid, uint16 cid,
	bcm_xdr_buf_t *b, void *a, uint a_len);
#endif /* WLC_HIGH_ONLY */

#endif /* _wlc_iocv_high_h_ */
