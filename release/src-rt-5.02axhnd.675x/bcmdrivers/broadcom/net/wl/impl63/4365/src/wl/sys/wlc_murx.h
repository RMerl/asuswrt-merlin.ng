/*
 * MU-MIMO receive module for Broadcom 802.11 Networking Adapter Device Drivers
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
 * $Id: wlc_murx.h 537259 2015-02-25 13:29:29Z $
 */

#ifndef _wlc_murx_h_
#define _wlc_murx_h_

#include <typedefs.h>
#include <wlc_types.h>
#include <wlc_mumimo.h>

#ifdef WLCNT
#define WLC_UPDATE_MURX_INPROG(wlc, bval) wlc_murx_update_murx_inprog((wlc)->murx, bval)
#define WLC_GET_MURX_INPROG(wlc) wlc_murx_get_murx_inprog((wlc)->murx)
#endif /* WLCNT */

#define WLC_MURX_BI_MU_BFR_CAP(wlc, bi) (MU_RX_ENAB(wlc) && \
	wlc_murx_is_bi_mu_bfr_cap((wlc)->murx, (bi)))

wlc_murx_info_t *(wlc_murx_attach)(wlc_info_t *wlc);
void (wlc_murx_detach)(wlc_murx_info_t *mu_info);
void wlc_murx_filter_bfe_cap(wlc_murx_info_t *mu_info, wlc_bsscfg_t *bsscfg, uint32 *cap);
bool wlc_murx_is_bi_mu_bfr_cap(wlc_murx_info_t *mu_info, wlc_bss_info_t *bi);
int wlc_murx_gid_update(wlc_info_t *wlc, struct scb *scb,
                        uint8 *membership_status, uint8 *user_position);
#if defined(WLCNT) && (defined(BCMDBG) || defined(WLDUMP) || defined(BCMDBG_MU) || \
	defined(BCMDBG_DUMP))
void wlc_murx_update_rxcounters(wlc_murx_info_t *mu_info, uint32 ft, struct scb *scb,
	struct dot11_header *h);
#endif // endif
bool wlc_murx_active(wlc_murx_info_t *mu_info);
#ifdef WLRSDB
bool wlc_murx_anymurx_active(wlc_murx_info_t *mu_info);
#endif /* WLRSDB */
#ifdef WL_MODESW
void wlc_murx_sync_oper_mode(wlc_murx_info_t *mu_info, wlc_bsscfg_t *bsscfg, wlc_bss_info_t *bi);
#endif /* WL_MODESW */
#ifdef WLCNT
void wlc_murx_update_murx_inprog(wlc_murx_info_t *mu_info, bool bval);
bool wlc_murx_get_murx_inprog(wlc_murx_info_t *mu_info);
#endif /* WLCNT */
#endif /* _wlc_murx_h_ */
