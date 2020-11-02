/*
 * Power Management Mode PM_FAST (PM2) interface
 *
 *   Copyright 2020 Broadcom
 *
 *   This program is the proprietary software of Broadcom and/or
 *   its licensors, and may only be used, duplicated, modified or distributed
 *   pursuant to the terms and conditions of a separate, written license
 *   agreement executed between you and Broadcom (an "Authorized License").
 *   Except as set forth in an Authorized License, Broadcom grants no license
 *   (express or implied), right to use, or waiver of any kind with respect to
 *   the Software, and Broadcom expressly reserves all rights in and to the
 *   Software and all intellectual property rights therein.  IF YOU HAVE NO
 *   AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 *   WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 *   THE SOFTWARE.
 *
 *   Except as expressly set forth in the Authorized License,
 *
 *   1. This program, including its structure, sequence and organization,
 *   constitutes the valuable trade secrets of Broadcom, and you shall use
 *   all reasonable efforts to protect the confidentiality thereof, and to
 *   use this information only in connection with your use of Broadcom
 *   integrated circuit products.
 *
 *   2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *   "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 *   REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 *   OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 *   DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 *   NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 *   ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 *   CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 *   OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *   3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 *   BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 *   SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 *   IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 *   IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 *   ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 *   OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 *   NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 *   $Id: wlc_pm.h 708017 2017-06-29 14:11:45Z $
 */

/** Twiki: [WlDriverPowerSave] */

#ifndef _wlc_pm_h_
#define _wlc_pm_h_

#ifdef STA

/* PM2 Fast Return to Sleep */
extern void wlc_pm2_enter_ps(wlc_bsscfg_t *cfg);
extern void wlc_pm2_sleep_ret_timeout_cb(void *arg);
extern void wlc_pm2_sleep_ret_timeout(wlc_bsscfg_t *cfg);
extern void wlc_pm2_ret_upd_last_wake_time(wlc_bsscfg_t *cfg, uint32* tsf_l);
extern void wlc_pm2_sleep_ret_timer_start(wlc_bsscfg_t *cfg);
extern void wlc_pm2_sleep_ret_timer_stop(wlc_bsscfg_t *cfg);
extern void wlc_pm2_sleep_ret_timeout_cb(void *arg);

/* Dynamic PM2 Fast Return To Sleep */
extern void wlc_dfrts_reset_counters(wlc_bsscfg_t *bsscfg);
extern void wlc_update_sleep_ret(wlc_bsscfg_t *bsscfg, bool inc_rx, bool inc_tx,
	uint rxbytes, uint txbytes);

/* PM2 Receive Throttle Duty Cycle */
#if defined(WL_PM2_RCV_DUR_LIMIT)
extern void wlc_pm2_rcv_timeout_cb(void *arg);
extern void wlc_pm2_rcv_timeout(wlc_bsscfg_t *cfg);
extern void wlc_pm2_rcv_timer_start(wlc_bsscfg_t *cfg);
extern void wlc_pm2_rcv_timer_stop(wlc_bsscfg_t *cfg);
#else
#define wlc_pm2_rcv_timer_stop(cfg)
#define wlc_pm2_rcv_timer_start(cfg)
#endif /* WL_PM2_RCV_DUR_LIMIT */

#ifdef WL_EXCESS_PMWAKE
extern uint32 wlc_get_roam_ms(wlc_info_t *wlc);
extern uint32 wlc_get_pfn_ms(wlc_info_t *wlc);
extern void wlc_generate_pm_alert_event(wlc_info_t *wlc, uint32 reason, void *data, uint32 datalen);
extern void wlc_check_roam_alert_thresh(wlc_info_t *wlc);
extern void wlc_check_excess_pm_awake(wlc_info_t *wlc);
extern void wlc_epm_roam_time_upd(wlc_info_t *wlc, uint32 connect_dur);
extern void wlc_reset_epm_ca(wlc_info_t *wlc);
extern void wlc_reset_epm_dur(wlc_info_t *wlc);
#endif /* WL_EXCESS_PMWAKE */
#endif	/* STA */

#endif	/* _wlc_pm_h_ */
