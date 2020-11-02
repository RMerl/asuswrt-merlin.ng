/*
 * Common OS-independent driver header for open-loop power calibration engine.
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
 * $Id: wlc_olpc_engine.h,v 1.70 2012/01/28 04:59:00 Exp $
 */
#ifndef _wlc_olpc_engine_h_
#define _wlc_olpc_engine_h_
#ifdef WLOLPC
/* get open loop pwr ctl rate limit for current channel */
#define WLC_OLPC_NO_LIMIT	0
#define WLC_OLPC_SISO_LIMIT	1

/* Event handlers */
/* EVENT: channel changes */
extern int wlc_olpc_eng_hdl_chan_update(wlc_olpc_eng_info_t *olpc);

/* EVENT: txchain changes */
extern int wlc_olpc_eng_hdl_txchain_update(wlc_olpc_eng_info_t *olpc);

/* EVENT: ccrev changes - wipe out saved channel data and try to cal on current channel */
extern int wlc_olpc_eng_reset(wlc_olpc_eng_info_t *olpc);

/* API: Force new open loop phy cal on current channel (call after channel change done) */
/* Will only calibrate if assoc is present */
extern int wlc_olpc_eng_recal(wlc_olpc_eng_info_t *olpc);

/* API: Module-system interfaces */
/* module attach */
extern wlc_olpc_eng_info_t * wlc_olpc_eng_attach(wlc_info_t *wlc);

/* module detach, up, down */
extern void wlc_olpc_eng_detach(wlc_olpc_eng_info_t *olpc);

/* return true if a calibration is in progress */
extern bool wlc_olpc_eng_has_active_cal(wlc_olpc_eng_info_t *olpc);

extern bool wlc_olpc_eng_tx_cal_pkts(wlc_olpc_eng_info_t *olpc);

/* get coremask of calibrated cores for current channel; -1 if not on olpc channel */
extern int wlc_olpc_get_cal_state(wlc_olpc_eng_info_t *olpc);

#endif /* WLOLPC */
#endif /* _wlc_olpc_engine_h_ */
