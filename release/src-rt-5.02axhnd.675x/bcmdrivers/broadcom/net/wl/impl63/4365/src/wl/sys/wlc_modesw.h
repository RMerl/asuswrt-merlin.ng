/*
 * Exposed interfaces of wlc_modesw.c
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
 * $Id: wlc_modesw.h 449355 2014-02-06 21:41:18Z  $
 */

#ifndef _wlc_modesw_h_
#define _wlc_modesw_h_

#define BW_SWITCH_TYPE_UPGRADE	1
#define BW_SWITCH_TYPE_DNGRADE	2
#define MSW_MODE_SWITCH_TIMEOUT 3000

/* Notification call back data */
enum notif_events {
	MODESW_DN_STA_COMPLETE = 1,
	MODESW_UP_STA_COMPLETE = 2,
	MODESW_DN_AP_COMPLETE = 3,
	MODESW_UP_AP_COMPLETE = 4,
	MODESW_PHY_UP_COMPLETE = 5,
	MODESW_PHY_DN_COMPLETE = 6,
	MODESW_ACTION_FAILURE = 7,
	MODESW_STA_TIMEOUT = 8,
	MODESW_PHY_UP_START = 9,
	MODESW_LAST = 10
};

typedef struct {
	wlc_bsscfg_t *cfg;
	uint8 opmode;
	int status;
	int signal;
} wlc_modesw_notif_cb_data_t;

#define MODESW_CTRL_UP_SILENT_UPGRADE			(0x0001)
#define MODESW_CTRL_DN_SILENT_DNGRADE			(0x0002)
#define MODESW_CTRL_UP_ACTION_FRAMES_ONLY		(0x0004)
#define MODESW_CTRL_AP_ACT_FRAMES			(0x0008)
#define MODESW_CTRL_OPMODE_IE_REQD_OVERRIDE		(0x0010) /* if set, oper_mode_enabled alone
								  * decides if opmode IE is required
								  */
#define MODESW_CTRL_SEND_EVENT				(0x0020) /* send event WL_E_MODES_SWITCH */

/* invalid oper mode */
#define WL_OPER_MODE_INVALID 0xFFu

/* Flag to handle all BSSCFGs. for eg: in DYN BWSW
* for PSTA/MBSS cases, we would need a callback from
* modesw for each BSSCFG while normally we would need
* just a single modesw cb. This flag helps to ensure that
* we get a callback for every cfg
*/
#define MODESW_CTRL_HANDLE_ALL_CFGS			(0x0010)
/* Flag to enable disassoc of non-acking STAs/APs
*/
#define MODESW_CTRL_NO_ACK_DISASSOC			(0x0020)

#define MODESW_BSS_ACTIVE(wlc, cfg)			\
	(	/* for assoc STA's... */					\
	((BSSCFG_STA(cfg) && cfg->associated) ||	\
	/* ... or UP AP's ... */						\
	(BSSCFG_AP(cfg) && cfg->up)))

/* Time Measurement Structure   */
typedef struct modesw_time_calc {
/* Normal Downgrade  */
	uint32 ActDN_SeqCnt;
	uint32 DN_start_time;
	uint32 DN_ActionFrame_time;
	uint32 DN_PHY_BW_UPDTime;
	uint32 DN_CompTime;
/* Only PHY Upgrade */
	uint32 PHY_UP_SeqCnt;
	uint32 PHY_UP_start_time;
	uint32 PHY_UP_PM1_time;
	uint32 PHY_UP_BW_UPDTime;
	uint32 PHY_UP_CompTime;
/* Only PHY Downgrade */
	uint32 PHY_DN_SeqCnt;
	uint32 PHY_DN_start_time;
	uint32 PHY_DN_PM1_time;
	uint32 PHY_DN_BW_UPDTime;
	uint32 PHY_DN_CompTime;
/* Action Frame sending only  */
	uint32 ACTFrame_SeqCnt;
	uint32 ACTFrame_start;
	uint32 ACTFrame_complete;
} modesw_time_calc_t;

typedef struct {
	uint32 w_80_sum;
	uint32 w_160_sum;
	uint64 tot_traffic_bytes;
	uint64 additional_bytes;
	//uint8 mu_cap_stas;
	uint8 w_bf_nss[5];
	uint8 init;
} dyn_switch_info_t;

#define chspec_to_rspec(chspec_bw)	\
		(((uint32)(((((uint16)(chspec_bw)) & WL_CHANSPEC_BW_MASK) >> \
			WL_CHANSPEC_BW_SHIFT) - 1)) << RSPEC_BW_SHIFT)

/* Call back registration */
typedef void (*wlc_modesw_notif_cb_fn_t)(void *ctx, wlc_modesw_notif_cb_data_t *notif_data);
extern int wlc_modesw_notif_cb_register(wlc_modesw_info_t *modeswinfo,
	wlc_modesw_notif_cb_fn_t cb, void *arg);
extern int wlc_modesw_notif_cb_unregister(wlc_modesw_info_t *modeswinfo,
	wlc_modesw_notif_cb_fn_t cb, void *arg);
extern wlc_modesw_info_t * wlc_modesw_attach(wlc_info_t *wlc);
extern void wlc_modesw_detach(wlc_modesw_info_t *modesw_info);

extern int wlc_modesw_set_max_chanspec(wlc_modesw_info_t *modesw_info, wlc_bsscfg_t *cfg,
	chanspec_t chanspec);

extern bool wlc_modesw_is_req_valid(wlc_modesw_info_t *modesw_info, wlc_bsscfg_t *cfg);

extern void wlc_modesw_bss_tbtt(wlc_modesw_info_t *modesw_info, wlc_bsscfg_t *cfg);

extern void wlc_modesw_pm_pending_complete(wlc_modesw_info_t *modesw_info, wlc_bsscfg_t *cfg);

extern uint8 wlc_modesw_get_oper_bw(chanspec_t chanspec);

extern chanspec_t wlc_modesw_ht_chanspec_override(wlc_modesw_info_t *modesw_info, wlc_bsscfg_t *cfg,
	chanspec_t beacon_chanspec);

extern uint8 wlc_modesw_derive_opermode(wlc_modesw_info_t *modesw_info,
	chanspec_t chanspec, wlc_bsscfg_t *bsscfg, uint8 rxstreams);

extern bool wlc_modesw_is_downgrade(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, uint8 oper_mode_old,
	uint8 oper_mode_new);

extern uint16 wlc_modesw_get_bw_from_opermode(uint8 oper_mode, chanspec_t chspec);

extern int wlc_modesw_handle_oper_mode_notif_request(wlc_modesw_info_t *modesw_info,
	wlc_bsscfg_t *bsscfg, uint8 oper_mode, uint8 enabled, uint32 ctrl_flags);

extern bool wlc_modesw_is_obss_active(wlc_modesw_info_t *modesw_info);

extern void wlc_modesw_clear_context(wlc_modesw_info_t *modesw_info, wlc_bsscfg_t *cfg);

extern int wlc_modesw_bw_switch(wlc_modesw_info_t *modesw_info,	chanspec_t chanspec,
	uint8 switch_type,	wlc_bsscfg_t *cfg, uint32 ctrl_flags);

extern bool
wlc_modesw_is_connection_vht(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg);
extern bool wlc_modesw_opmode_ie_reqd(wlc_modesw_info_t *modesw_info, wlc_bsscfg_t *bsscfg);
extern void wlc_modesw_bsscfg_complete_downgrade(wlc_modesw_info_t *modesw_info);
extern void wlc_modesw_resume_opmode_change(wlc_modesw_info_t *modesw_info, wlc_bsscfg_t *cfg);
extern void
wlc_modesw_dynbw_tx_bw_override(wlc_modesw_info_t *modesw_info,
	wlc_bsscfg_t *bsscfg, uint32 *rspec_bw);

extern int wlc_modesw_init_max_rateset(wlc_modesw_info_t *modesw_info, uint8 streams);
extern void wlc_modesw_deinit_max_rateset(wlc_modesw_info_t *modesw_info);

extern int wlc_modesw_mutx_state_upd_register(wlc_info_t *wlc);
extern int wlc_modesw_mutx_state_upd_unregister(wlc_info_t *wlc);

#if defined(DYN160)
int wlc_modesw_dyn_switch_calc(wlc_modesw_info_t *modesw_info, wlc_bsscfg_t *bsscfg);
void wlc_modesw_dyn_switch_th_set(wlc_modesw_info_t *modesw_info);
#endif /* DYN160 */

#endif  /* _WLC_MODESW_H_ */
