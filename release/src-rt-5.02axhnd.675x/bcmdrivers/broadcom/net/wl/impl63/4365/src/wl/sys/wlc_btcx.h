/*
 * BT Coex module interface
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
 * $Id: wlc_btcx.h 708017 2017-06-29 14:11:45Z $
 */

#ifndef _wlc_btcx_h_
#define _wlc_btcx_h_

#define BT_AMPDU_THRESH		10000	/* if BT period < this threshold, turn off ampdu */

#define BTC_RXGAIN_FORCE_OFF            0
#define BTC_RXGAIN_FORCE_2G_MASK        0x1
#define BTC_RXGAIN_FORCE_2G_ON          0x1
#define BTC_RXGAIN_FORCE_5G_MASK        0x2
#define BTC_RXGAIN_FORCE_5G_ON          0x2

#ifdef WLRSDB
/* COEX IO_MASK block */
typedef enum {
	COEX_IOMASK_TXCONF_POS = 0,
	COEX_IOMASK_PRISEL_POS = 1,
	COEX_IOMASK_WLPRIO_POS = 4,
	COEX_IOMASK_WLTXON_POS = 5
} coex_io_mask_t;
#endif /* WLRSDB */

extern wlc_btc_info_t *wlc_btc_attach(wlc_info_t *wlc);
extern void wlc_btc_detach(wlc_btc_info_t *btc);
extern int wlc_btc_wire_get(wlc_info_t *wlc);
extern int wlc_btc_mode_get(wlc_info_t *wlc);
extern void wlc_enable_btc_ps_protection(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, bool protect);
extern uint wlc_btc_frag_threshold(wlc_info_t *wlc, struct scb *scb);
extern void wlc_btc_mode_sync(wlc_info_t *wlc);
extern uint8 wlc_btc_save_host_requested_pm(wlc_info_t *wlc, uint8 val);
extern bool wlc_btc_get_bth_active(wlc_info_t *wlc);
extern uint16 wlc_btc_get_bth_period(wlc_info_t *wlc);
extern void wlc_btc_4313_gpioctrl_init(wlc_info_t *wlc);
extern void wlc_btcx_read_btc_params(wlc_info_t *wlc);
extern bool wlc_btc_turnoff_aggr(wlc_info_t *wlc);
extern bool wlc_btc_mode_not_parallel(int btc_mode);
extern bool wlc_btc_active(wlc_info_t *wlc);
#ifdef WLRSDB
extern void wlc_btcx_update_coex_iomask(wlc_info_t *wlc);
#endif /* WLRSDB */
extern int wlc_btc_siso_ack_set(wlc_info_t *wlc, int16 int_val, bool force);
extern void wlc_btc_hflg(wlc_info_t *wlc, bool set, uint16 val);

#ifdef WLBTCPROF
#define WL_BTCPROF WL_INFORM

/* BTCOEX profile data structures */
#define BTC_SUPPORT_BANDS	2

#define BTC_PROFILE_2G		0
#define BTC_PROFILE_5G		1

#define BTC_PROFILE_OFF		0
#define BTC_PROFILE_DISABLE 1
#define BTC_PROFILE_ENABLE	2

struct wlc_btc_profile {
	uint32 mode;
	uint32 desense;
	int desense_level;
	int desense_thresh_high;
	int desense_thresh_low;
	uint32 num_chains;
	uint32 chain_ack[WL_NUM_TXCHAIN_MAX];
	int chain_power_offset[WL_NUM_TXCHAIN_MAX];
};
typedef struct wlc_btc_profile wlc_btc_profile_t;

struct wlc_btc_prev_connect {
	int prev_band;
	int prev_2G_mode;
	int prev_5G_mode;
	struct wlc_btc_profile prev_2G_profile;
	struct wlc_btc_profile prev_5G_profile;
};
typedef struct wlc_btc_prev_connect wlc_btc_prev_connect_t;

struct wlc_btc_select_profile {
	int enable;
	struct wlc_btc_profile select_profile;
};
typedef struct wlc_btc_select_profile wlc_btc_select_profile_t;

extern int wlc_btcx_select_profile_set(wlc_info_t *wlc, uint8 *pref, int len);
extern int wlc_btcx_select_profile_get(wlc_info_t *wlc, uint8 *pref, int len);
extern int wlc_btcx_set_btc_profile_param(struct wlc_info *wlc, chanspec_t chanspec, bool force);
#else
#define WL_BTCPROF WL_NONE
#endif /* WLBTCPROF */

#define BTC_BTRSSI_THRESH  60 /* actual thresh = -1 x (thresh + 10) e.g., 60 -> -70dBm */
#define BTC_BTRSSI_SIZE    16 /* maximum number of btrssi samples for moving average */
#define BTC_BTRSSI_MIN_NUM  4 /* minimum number of btrssi samples for moving average */
#define BTC_SISOACK_CORES_MASK    0x00FF
#define BTC_SISOACK_TXPWR_MASK    0xFF00
#endif /* _wlc_btcx_h_ */
