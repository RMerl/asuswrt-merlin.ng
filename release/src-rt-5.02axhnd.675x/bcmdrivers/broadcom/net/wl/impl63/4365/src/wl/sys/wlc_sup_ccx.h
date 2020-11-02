/*
 * Exposed interfaces of wlc_sup_ccx.c
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
 * $Id: wlc_sup_ccx.h 708017 2017-06-29 14:11:45Z $
 */

#ifndef _wlc_ccxsup_h_
#define _wlc_ccxsup_h_

typedef struct wlc_ccxsup_pub {
	int cfgh;			/* bsscfg cubby handle */
} wlc_ccxsup_pub_t;

#define WLC_CCXSUP_INFO_CFGH(ccxsup_info) (((wlc_ccxsup_pub_t *)(ccxsup_info))->cfgh)
extern wlc_ccxsup_info_t * wlc_ccxsup_attach(wlc_info_t *wlc);
extern void wlc_ccxsup_detach(wlc_ccxsup_info_t *ccxsup_info);

/* Initiate supplicant private context */
extern int wlc_ccxsup_init(void *ctx, sup_init_event_data_t *evt);

/* Remove supplicant private context */
extern void wlc_ccxsup_deinit(void *ctx, wlc_bsscfg_t *cfg);

/* Return whether the given SSID matches on in the LEAP list. */
extern bool wlc_ccx_leap_ssid(struct wlc_ccxsup_info *ccxsup_info, struct wlc_bsscfg *cfg,
	uchar SSID[], int len);

/* Time-out  LEAP authentication and presume the AP is a rogue */
extern void wlc_ccx_rogue_timer(struct wlc_ccxsup_info *ccxsup_info, struct wlc_bsscfg *cfg,
	struct ether_addr *ap_mac);

/* Register a rogue AP report */
extern void wlc_ccx_rogueap_update(struct wlc_ccxsup_info *ccxsup_info, struct wlc_bsscfg *cfg,
	uint16 reason, struct ether_addr *ap_mac);

/* Return whether the supplicant state indicates successful authentication */
extern bool wlc_ccx_authenticated(struct wlc_ccxsup_info *ccxsup_info, struct wlc_bsscfg *cfg);

#if defined(BCMSUP_PSK) || !defined(BCMINTSUP)
/* Populate the CCKM reassoc req IE */
extern void wlc_cckm_gen_reassocreq_IE(struct wlc_ccxsup_info *ccxsup_info, struct wlc_bsscfg *cfg,
	cckm_reassoc_req_ie_t *cckmie, uint32 tsf_h, uint32 tsf_l, struct ether_addr *bssid,
	wpa_ie_fixed_t *rsnie);

/* Check for, validate, and process the CCKM reassoc resp IE */
extern bool wlc_cckm_reassoc_resp(struct wlc_ccxsup_info *ccxsup_info, struct wlc_bsscfg *cfg);
#endif /* BCMSUP_PSK || !BCMINTSUP */

extern void wlc_ccx_sup_init(struct wlc_ccxsup_info *ccxsup_info,
	struct wlc_bsscfg *cfg, int sup_type);

extern bool
wlc_sup_getleapauthpend(wlc_ccxsup_info_t *ccxsup_info, wlc_bsscfg_t *cfg);

#if defined BCMEXTCCX
/* For external supplicant */
extern void
wlc_cckm_set_assoc_resp(wlc_ccxsup_info_t *ccxsup_info, wlc_bsscfg_t *cfg,
	uint8 *assoc_resp, int len);
extern void
wlc_cckm_set_rn(wlc_ccxsup_info_t *ccxsup_info, wlc_bsscfg_t *cfg, int rn);
#endif // endif

extern sup_auth_status_t wlc_ccxsup_get_auth_status(wlc_ccxsup_info_t *ccxsup_info,
	wlc_bsscfg_t *cfg);
extern uint16 wlc_ccxsup_get_cipher(wlc_ccxsup_info_t *ccxsup_info, wlc_bsscfg_t *cfg,
	wpapsk_t *wpa, uint16 key_info, uint16 key_len);
extern uint16 wlc_ccxsup_handle_joinstart(wlc_ccxsup_info_t *ccxsup_info,
	wlc_bsscfg_t *cfg, uint16 sup_type);
extern void wlc_ccxsup_handle_wpa_eapol_msg1(wlc_ccxsup_info_t *ccxsup_info,
	wlc_bsscfg_t *cfg, uint16 key_info);
extern void wlc_ccxsup_send_leap_rogue_report(wlc_ccxsup_info_t *ccxsup_info, wlc_bsscfg_t *cfg);
extern void wlc_ccxsup_set_leap_state_keyed(wlc_ccxsup_info_t *ccxsup_info, wlc_bsscfg_t *cfg);
extern void wlc_ccxsup_init_cckm_rn(wlc_ccxsup_info_t *ccxsup_info, wlc_bsscfg_t *cfg);
extern void wlc_ccxsup_start_negotimer(wlc_ccxsup_info_t *ccxsup_info, wlc_bsscfg_t *cfg);
extern bool wlc_ccx_leapsup(wlc_ccxsup_info_t *ccxsup_info, wlc_bsscfg_t *cfg,
	eapol_header_t *rx_hdr);
/* Initiate supplicant LEAP authentication. */
extern bool wlc_leapsup_start(wlc_ccxsup_info_t *ccxsup_info, wlc_bsscfg_t *cfg);
#endif	/* _wlc_ccxsup_h_ */
