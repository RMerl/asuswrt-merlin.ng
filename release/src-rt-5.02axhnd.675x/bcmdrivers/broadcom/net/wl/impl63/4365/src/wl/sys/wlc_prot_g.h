/*
 * 11g protection module APIs
 * Broadcom 802.11abg Networking Device Driver
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
 * $Id: wlc_prot_g.h 708017 2017-06-29 14:11:45Z $
 */

#ifndef _wlc_prot_g_h_
#define _wlc_prot_g_h_

/* data APIs */
/* module public states - read only */
struct wlc_prot_g_info {
	int cfgh;
};
#define WLC_PROT_G_INFO_CFGH(prot)	((prot)->cfgh)

/* bsscfg specific states - read only */
typedef	struct {
	bool _g;
} wlc_prot_g_cfg_t;
#define WLC_PROT_G_CFG(prot, cfg)	(*(wlc_prot_g_cfg_t **) \
					 BSSCFG_CUBBY(cfg, WLC_PROT_G_INFO_CFGH(prot)))
#define WLC_PROT_G_CFG_G(prot, cfg)	(WLC_PROT_G_CFG(prot, cfg)->_g)

/* function APIs */
/* module entries */
extern wlc_prot_g_info_t *wlc_prot_g_attach(wlc_info_t *wlc);
extern void wlc_prot_g_detach(wlc_prot_g_info_t *prot);

/* configuration change */
extern void wlc_prot_g_cfg_set(wlc_prot_g_info_t *prot, uint idx, int val);
/* wlc_prot_g_cfg_set() idx */
#define	WLC_PROT_G_USER		1	/* gmode specified by user */
extern void wlc_prot_g_cfg_init(wlc_prot_g_info_t *prot, wlc_bsscfg_t *cfg);

/* condition change */
extern void wlc_prot_g_cond_upd(wlc_prot_g_info_t *prot, struct scb *scb);

/* timeout change */
extern void wlc_prot_g_ovlp_upd(wlc_prot_g_info_t *prot, chanspec_t chspec,
	uint8 *erp, int erp_len, bool is_erp,
	bool short_cap);
extern void wlc_prot_g_to_upd(wlc_prot_g_info_t *prot, wlc_bsscfg_t *cfg,
	uint8 *erp, int erp_len, bool is_erp, bool shortpreamble);

/* gmode change */
extern void wlc_prot_g_mode_reset(wlc_prot_g_info_t *prot);
extern bool wlc_prot_g_mode_upd(wlc_prot_g_info_t *prot, wlc_bsscfg_t *cfg);

/* protection init */
extern void wlc_prot_g_init(wlc_prot_g_info_t *prot, wlc_bsscfg_t *cfg);

#endif /* _wlc_prot_g_h_ */
