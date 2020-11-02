/*
 * Exposed interfaces of wlc_fbt.c
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
 * $Id: wlc_fbt.h 708017 2017-06-29 14:11:45Z $
 */

#ifndef _wlc_fbt_h_
#include <proto/eapol.h>

#define _wlc_fbt_h_

typedef struct wlc_fbt_pub {
	int cfgh;			/* bsscfg cubby handle */
} wlc_fbt_pub_t;

typedef struct bss_fbt_pub {
	bool ini_fbt;		/* initial ft handshake */
} bss_fbt_pub_t;

#define WLC_FBT_INFO_CFGH(fbt_info) (((wlc_fbt_pub_t *)(fbt_info))->cfgh)

#define BSS_FBT_INFO(fbt_info, cfg) \
	(*(bss_fbt_pub_t **)BSSCFG_CUBBY(cfg, WLC_FBT_INFO_CFGH(fbt_info)))
#define BSS_FBT_INI_FBT(fbt_info, cfg) (BSS_FBT_INFO(fbt_info, cfg)->ini_fbt)

extern wlc_fbt_info_t * BCMATTACHFN(wlc_fbt_attach)(wlc_info_t *wlc);
extern void BCMATTACHFN(wlc_fbt_detach)(wlc_fbt_info_t *fbt_info);

#if defined(STA) && defined(FBT_STA)
extern int wlc_fbt_set_pmk(wlc_fbt_info_t *fbt_info, struct wlc_bsscfg *cfg,
	wsec_pmk_t *pmk, bool assoc);
/* verify with current mdid */
extern void wlc_fbt_clear_ies(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg);
extern bool wlc_fbt_is_cur_mdid(wlc_fbt_info_t *fbt_info, struct wlc_bsscfg *cfg,
	wlc_bss_info_t *bi);
extern bool wlc_fbt_is_fast_reassoc(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg,
	wlc_bss_info_t *bi);
extern bool wlc_fbt_akm_match(wlc_fbt_info_t *fbt_info, const wlc_bsscfg_t *cfg,
	const wlc_bss_info_t *bi);
extern uint8 *
wlc_fbt_get_pmkr1name(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg);
extern void
wlc_fbt_calc_fbt_ptk(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg);
extern void
wlc_fbt_addies(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg, eapol_wpa_key_header_t *wpa_key);
extern uint16 wlc_fbt_getlen_eapol(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg);
extern void wlc_fbt_recv_overds_resp(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg,
	struct dot11_management_header *hdr, uint8 *body, uint body_len);
extern void *wlc_fbt_send_overds_req(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg,
	struct ether_addr *ea, struct scb *scb, bool short_preamble);
extern void wlc_fbt_save_current_akm(wlc_fbt_info_t *fbt_info, const wlc_bsscfg_t *cfg,
	const wlc_bss_info_t *bi);
extern void wlc_fbt_reset_current_akm(wlc_fbt_info_t *fbt_info, const wlc_bsscfg_t *cfg);
extern void wlc_fbt_get_kck_kek(wlc_fbt_info_t *fbt_info, struct wlc_bsscfg *cfg, uint8 *key);

#else /* STA && FBT_STA */
#define wlc_fbt_set_pmk(f, c, p, a)			0
#define wlc_fbt_clear_ies(f, c)				do {} while (0)
#define wlc_fbt_is_cur_mdid(f, c, b)			FALSE
#define wlc_fbt_is_fast_reassoc(f, c, b)		FALSE
#define wlc_fbt_akm_match(f, c, b)			FALSE
#define wlc_fbt_get_pmkr1name(f, c)			NULL
#define wlc_fbt_calc_fbt_ptk(f, b)			do {} while (0)
#define wlc_fbt_addies(f, c, w)				do {} while (0)
#define wlc_fbt_getlen_eapol(f, c)			0
#define wlc_fbt_recv_overds_resp(f, c, h, b, l)		do {} while (0)
#define wlc_fbt_send_overds_req(f, c, e, s, p)		NULL
#define wlc_fbt_save_current_akm(f, c, b)		do {} while (0)
#define wlc_fbt_reset_current_akm(f, c)			do {} while (0)
#define wlc_fbt_get_kck_kek(f, c, k)			do {} while (0)
#endif /* STA && FBT_STA */

extern void
wlc_fbt_set_ea(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg, struct ether_addr *ea);

extern bool wlc_fbt_enabled(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg);
#ifdef AP
extern void wlc_fbt_handle_auth(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg,
	struct dot11_management_header *hdr, uint8 *body, int body_len);
extern void wlc_fbt_recv_overds_req(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *bsscfg,
	struct dot11_management_header *hdr, void *body, uint body_len);
#endif /* AP */
#endif	/* _wlc_fbt_h_ */
