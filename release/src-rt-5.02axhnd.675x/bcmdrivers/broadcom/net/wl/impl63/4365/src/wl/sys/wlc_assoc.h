/*
 * Association/Roam related routines
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
 * $Id: wlc_assoc.h 782619 2019-12-27 08:38:47Z $
 */

#ifndef __wlc_assoc_h__
#define __wlc_assoc_h__

#ifdef STA
extern int wlc_join(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, uint8 *SSID, int len,
	wl_join_scan_params_t *scan_params,
	wl_join_assoc_params_t *assoc_params, int assoc_params_len);
extern void wlc_join_recreate(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg);

extern void wlc_join_complete(wlc_bsscfg_t *cfg, wlc_d11rxhdr_t *wrxh, uint8 *plcp,
	struct dot11_bcn_prb *bcn, int bcn_len);
extern void wlc_join_recreate_complete(wlc_bsscfg_t *cfg, wlc_d11rxhdr_t *wrxh, uint8 *plcp,
	struct dot11_bcn_prb *bcn, int bcn_len);

extern int wlc_join_pref_parse(wlc_bsscfg_t *cfg, uint8 *pref, int len);
extern int wlc_join_pref_build(wlc_bsscfg_t *cfg, uint8 *pref, int len);
extern void wlc_join_pref_reset(wlc_bsscfg_t *cfg);
extern void wlc_join_pref_band_upd(wlc_bsscfg_t *cfg);

extern int wlc_reassoc(wlc_bsscfg_t *cfg, wl_reassoc_params_t *reassoc_params);

extern void wlc_roam_complete(wlc_bsscfg_t *cfg, uint status,
                              struct ether_addr *addr, uint bss_type);
extern int wlc_roam_scan(wlc_bsscfg_t *cfg, uint reason, chanspec_t *list, uint32 channum);
extern void wlc_roamscan_start(wlc_bsscfg_t *cfg, uint roam_reason);
extern void wlc_assoc_roam(wlc_bsscfg_t *cfg);
extern void wlc_txrate_roam(wlc_info_t *wlc, struct scb *scb, tx_status_t *txs, bool pkt_sent,
	bool pkt_max_retries, uint8 ac);
extern void wlc_build_roam_cache(wlc_bsscfg_t *cfg, wlc_bss_list_t *candidates);
extern void wlc_roam_motion_detect(wlc_bsscfg_t *cfg);
extern void wlc_roam_bcns_lost(wlc_bsscfg_t *cfg);
extern int wlc_roam_trigger_logical_dbm(wlc_info_t *wlc, wlcband_t *band, int val);
extern bool wlc_roam_scan_islazy(wlc_info_t *wlc, wlc_bsscfg_t *cfg, bool roam_scan_isactive);
extern bool wlc_lazy_roam_scan_suspend(wlc_info_t *wlc, wlc_bsscfg_t *cfg);
extern bool wlc_lazy_roam_scan_sync_dtim(wlc_info_t *wlc, wlc_bsscfg_t *cfg);
extern void wlc_roam_prof_update(wlc_info_t *wlc, wlc_bsscfg_t *cfg, bool reset);
extern void wlc_roam_prof_update_default(wlc_info_t *wlc, wlc_bsscfg_t *cfg);

extern int wlc_disassociate_client(wlc_bsscfg_t *cfg, bool send_disassociate);

extern int wlc_assoc_abort(wlc_bsscfg_t *cfg);
extern void wlc_assoc_timeout(void *cfg);
extern void wlc_assoc_change_state(wlc_bsscfg_t *cfg, uint newstate);
extern void wlc_authresp_client(wlc_bsscfg_t *cfg,
	struct dot11_management_header *hdr, uint8 *body, uint body_len, bool short_preamble);
extern void wlc_assocresp_client(wlc_bsscfg_t *cfg, struct scb *scb,
	struct dot11_management_header *hdr, uint8 *body, uint body_len);
extern void wlc_process_assocresp_decision(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg,
	assoc_decision_t * dc);
extern void wlc_auth_tx_complete(wlc_info_t *wlc, uint txstatus, void *arg);
extern void wlc_auth_complete(wlc_bsscfg_t *cfg, uint status, struct ether_addr *addr,
	uint auth_status, uint auth_type);
extern void wlc_assocreq_complete(wlc_info_t *wlc, uint txstatus, void *arg);

extern void wlc_sta_assoc_upd(wlc_bsscfg_t *cfg, bool state);

extern void wlc_clear_hw_association(wlc_bsscfg_t *cfg, bool mute_mode);
#ifdef ROBUST_DISASSOC_TX
extern void wlc_disassoc_tx(wlc_bsscfg_t *cfg, bool send_disassociate);
#endif /* ROBUST_DISASSOC_TX */
extern void wlc_roam_timer_expiry(void *arg);
#if defined(WLTEST)
extern void wlc_assoc_auth_txstatus(wlc_info_t * wlc, wlc_bsscfg_t * cfg, uint16 fc);
#endif // endif

#endif /* STA */

extern int wlc_mac_request_entry(wlc_info_t *wlc, wlc_bsscfg_t *cfg, int req);

extern void wlc_roam_defaults(wlc_info_t *wlc, wlcband_t *band, int *roam_trigger, uint *rm_delta);

extern void wlc_disassoc_complete(wlc_bsscfg_t *cfg, uint status, struct ether_addr *addr,
	uint disassoc_reason, uint bss_type);
extern void wlc_deauth_complete(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, uint status,
	const struct ether_addr *addr, uint deauth_reason, uint bss_type);
typedef struct wlc_deauth_send_cbargs {
	struct ether_addr	ea;
	int8			_idx;
	void                    *pkt;
} wlc_deauth_send_cbargs_t;
extern void wlc_deauth_sendcomplete(wlc_info_t *wlc, uint txstatus, void *arg);
extern void wlc_disassoc_ind_complete(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, uint status,
	struct ether_addr *addr, uint disassoc_reason, uint bss_type,
	uint8 *body, int body_len);
extern void wlc_deauth_ind_complete(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, uint status,
	struct ether_addr *addr, uint deauth_reason, uint bss_type,
	uint8 *body, int body_len);

extern void wlc_assoc_bcn_mon_off(wlc_bsscfg_t *cfg, bool off, uint user);

extern void wlc_join_adopt_ibss_params(wlc_info_t *wlc, wlc_bsscfg_t *cfg);
extern int wlc_join_start_ibss(wlc_info_t *wlc, wlc_bsscfg_t *cfg);
extern int wlc_assoc_iswpaenab(wlc_bsscfg_t *cfg, bool wpa);

extern void wlc_join_start_prep(wlc_info_t *wlc, wlc_bsscfg_t *cfg);
extern void wlc_assoc_init(wlc_bsscfg_t *cfg, uint type);
extern bool wlc_assoc_check_aplost_ok(wlc_info_t *wlc, wlc_bsscfg_t *cfg);
extern void wlc_adopt_dtim_period(wlc_bsscfg_t *cfg, uint8 dtim_period);
#ifdef WLAWDL
extern void wlc_join_awdl_bsscfg(wlc_bsscfg_t *cfg);
#endif // endif
extern uint32 wlc_bss_pref_score(wlc_bsscfg_t *cfg, wlc_bss_info_t *bi, bool band_rssi_boost,
	uint32 *prssi);
extern int wlc_remove_assoc_req(wlc_info_t *wlc, wlc_bsscfg_t *cfg);
extern void wlc_try_join_start(wlc_bsscfg_t *cfg, wl_join_scan_params_t *scan_params,
	wl_join_assoc_params_t *assoc_params);
extern int8 wlc_assoc_get_as_state(wlc_bsscfg_t *cfg);
#endif /* __wlc_assoc_h__ */
extern int wlc_ext_auth_tx_complete(wlc_info_t *wlc, wlc_bsscfg_t *cfg, void *pkt,
        void *arg);

extern void wlc_assoc_continue_post_auth1(wlc_bsscfg_t *cfg, struct scb *scb);
extern int wlc_start_assoc(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, const struct ether_addr *ea);
