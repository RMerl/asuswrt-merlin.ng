/*
 * Proxy STA interface
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
 * $Id: wlc_psta.h 708017 2017-06-29 14:11:45Z $
 */

#ifndef _WLC_PSTA_H_
#define _WLC_PSTA_H_

#define	PSTA_IS_PROXY(wlc)	((wlc)->pub->_psta == PSTA_MODE_PROXY)
#define	PSTA_IS_REPEATER(wlc)	((wlc)->pub->_psta == PSTA_MODE_REPEATER)

/* Max PSTA assoc limits exclude the default primary assoc. One entry in
 * bottom half of the skl and key tables is accounted for the primary's
 * pairwise keys and one in top half is used for storing the broadcast key.
 */
#ifndef MAXVSLAVEDEVS
#define MAXVSLAVEDEVS	0
#endif // endif

#if MAXVSLAVEDEVS > 0 && MAXVSLAVEDEVS < (RCMTA_SIZE / 2)
#define	PSTA_PROXY_MAX_ASSOC	(MAXVSLAVEDEVS)
#define	PSTA_RPT_MAX_ASSOC	(MAXVSLAVEDEVS - 1) /* reserve for AP(wlx.1) mac */
#else
#define	PSTA_PROXY_MAX_ASSOC	((RCMTA_SIZE / 2) - 1)
#define	PSTA_RPT_MAX_ASSOC	((RCMTA_SIZE / 2) - 1)
#endif /* MAXVSLAVEDEVS > 0 */

#define PSTA_MAX_ASSOC(wlc)	(PSTA_IS_REPEATER(wlc) ? PSTA_RPT_MAX_ASSOC : \
				                         PSTA_PROXY_MAX_ASSOC)

/* RCMTA engine can have a maximum of 50 entries. Reserve slot 25 for the
 * primary. Slots 26 to 49 are available for use by PSTA RAs. Slots 0 to 24
 * are available for use for the downstream STAs TAs.
 */
#define PSTA_TA_STRT_INDX	0
#define PSTA_RA_PRIM_INDX	(RCMTA_SIZE / 2)
#define PSTA_RA_STRT_INDX	((RCMTA_SIZE / 2) + 1)

typedef enum {
	/* Store current PSTA mode and switch
	 * to DWDS in case if the
	 * corresponding conditions are
	 * satisfied.
	 */
	PSTA_MODE_UPDATE_ACTION_SAVE = 0,
	/* If there is previously saved mode
	 * then restore it
	 */
	PSTA_MODE_UPDATE_ACTION_RESTORE = 1
} psta_mode_update_action_t;

/* Initialize psta private context.It returns a pointer to the
 * psta private context if succeeded. Otherwise it returns NULL.
 */
extern wlc_psta_info_t *wlc_psta_attach(wlc_info_t *wlc);

extern int32 wlc_psta_init(wlc_psta_info_t *psta, wlc_bsscfg_t *pcfg);

/* Cleanup psta private context */
extern void wlc_psta_detach(wlc_psta_info_t *psta);

/* Process frames in transmit direction */
extern int32 wlc_psta_send_proc(wlc_psta_info_t *psta, void **p, wlc_bsscfg_t **cfg);

/* Process frames in receive direction */
extern void wlc_psta_recv_proc(wlc_psta_info_t *psta, void *p, struct ether_header *eh,
	wlc_bsscfg_t **cfg);

extern wlc_bsscfg_t *wlc_psta_find(wlc_psta_info_t *psta, uint8 *mac);

/* Disassociate / re-associate all Proxy STAs (used for roaming) */
extern void wlc_psta_disassoc_all(wlc_psta_info_t *psta);
extern void wlc_psta_reassoc_all(wlc_psta_info_t *psta, wlc_bsscfg_t *pcfg);

extern void wlc_psta_disable(wlc_psta_info_t *psta, wlc_bsscfg_t *cfg);

extern void wlc_psta_disable_all(wlc_psta_info_t *psta);

extern void wlc_psta_deauth_client(wlc_psta_info_t *psta, struct ether_addr *addr);

#ifdef BCMDBG
extern int wlc_psta_dump(wlc_psta_info_t *psta, struct bcmstrbuf *b);
#endif /* BCMDBG */

extern uint8 wlc_psta_rcmta_idx(wlc_psta_info_t *psta, const wlc_bsscfg_t *cfg);

extern void wlc_psta_mode_update(wlc_psta_info_t *psta, wlc_bsscfg_t *cfg, struct ether_addr *addr,
	psta_mode_update_action_t action);

#ifdef DPSTA
extern bool wlc_psta_is_ds_sta(void *psta, struct ether_addr *mac);
extern bool wlc_psta_authorized(wlc_bsscfg_t *cfg);
extern wlc_bsscfg_t *wlc_psta_find_dpsta(void *psta, uint8 *mac);
extern void *wlc_psta_get_psta(wlc_info_t *wlc);
#endif /* DPSTA */
#endif	/* _WLC_PSTA_H_ */
