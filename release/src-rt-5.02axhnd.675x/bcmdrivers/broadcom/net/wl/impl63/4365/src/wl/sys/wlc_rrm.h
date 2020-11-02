/*
 * 802.11k definitions for
 * Broadcom 802.11abgn Networking Device Driver
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
 * $Id: wlc_rrm.h 729272 2017-10-31 06:46:21Z $
 */

#ifndef _wlc_rrm_h_
#define _wlc_rrm_h_

extern wlc_rrm_info_t *wlc_rrm_attach(wlc_info_t *wlc);
extern void wlc_rrm_detach(wlc_rrm_info_t *rrm_info);
extern void wlc_frameaction_rrm(wlc_rrm_info_t *rrm_info, wlc_bsscfg_t *cfg, struct scb *scb,
	uint action_id, uint8 *body, int body_len, int8 rssi, ratespec_t rspec);
extern void wlc_rrm_pm_pending_complete(wlc_rrm_info_t *rrm_info);
extern void wlc_rrm_terminate(wlc_rrm_info_t *rrm_info);
extern bool wlc_rrm_inprog(wlc_info_t *wlc);
extern void wlc_rrm_stop(wlc_info_t *wlc);
extern bool wlc_rrm_wait_tx_suspend(wlc_info_t *wlc);
extern void wlc_rrm_start_timer(wlc_info_t *wlc);
extern bool wlc_rrm_enabled(wlc_rrm_info_t *rrm_info, wlc_bsscfg_t *cfg);
#ifdef WLSCANCACHE
extern void wlc_rrm_update_cap(wlc_rrm_info_t *rrm_info, wlc_bsscfg_t *bsscfg);
#endif /* WLSCANCACHE */
extern bool wlc_rrm_in_progress(wlc_info_t *wlc);
extern void wlc_rrm_upd_data_activity_ts(wlc_rrm_info_t *ri);
#ifdef WL11K_ALL_MEAS
extern void wlc_rrm_stat_qos_counter(struct scb *scb, int tid, uint cnt_offset);
extern void wlc_rrm_stat_bw_counter(wlc_info_t *wlc, struct scb *scb, bool tx);
extern void wlc_rrm_stat_chanwidthsw_counter(wlc_info_t *wlc, wlc_bsscfg_t *cfg);
extern void wlc_rrm_tscm_upd(struct scb *scb, int tid, uint cnt_offset, uint cnt_val);
extern void wlc_rrm_delay_upd(struct scb *scb, uint8 tid, uint32 delay);
#else
#define wlc_rrm_stat_qos_counter(scb, tid, cnt_offset) do {(void)(tid);} while (0)
#define wlc_rrm_stat_bw_counter(wlc, scb, tx) do {} while (0)
#define wlc_rrm_stat_chanwidthsw_counter(wlc, cfg) do {} while (0)
#define wlc_rrm_tscm_upd(scb, tid, cnt_offset, cnt_val) do {} while (0)
#define wlc_rrm_delay_upd(scb, tid, delay) do {(void)(delay);} while (0)
#endif /* WL11K_ALL_MEAS */
#ifdef WL11K_AP
extern void rrm_add_pilot_timer(wlc_info_t *wlc, wlc_bsscfg_t *cfg);
#endif // endif
#if defined(WL11K_AP) && defined(WL_MBO) && !defined(WL_MBO_DISABLED)
extern void wlc_rrm_update_gasi(wlc_info_t* wlc, void* gasi);
#endif /* WL11K_AP && WL_MBO && !WL_MBO_DISABLED */
#endif	/* _wlc_rrm_h_ */
