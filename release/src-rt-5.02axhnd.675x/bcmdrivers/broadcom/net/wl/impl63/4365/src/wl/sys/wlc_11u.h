/*
 * 802.11u module header file (interworking protocol)
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
 * $Id: wlc_11u.h 708017 2017-06-29 14:11:45Z $
*/

/**
 * XXX
 * Network discovery and selection (preassociation information on e.g. private/public network,
 * roaming consortium, venue information). Uses GAS and ANQP to accomplish this.
 */

#ifndef _wlc_11u_h_
#define _wlc_11u_h_

#include <bcmutils.h>
/* APIs */
#ifdef WL11U
/* 802.11u Interworking(IW) IE size */
#define IW_IE_MAX_SIZE 				11
/* 802.11u IW Advertisement Protocol IE size */
#define IWAP_IE_MAX_SIZE 			4
/* Access network type offset in IW IE */
#define IW_ANT_OFFSET				0
#define IW_ANT_WILDCARD			0xF
#define IW_LEN						9
#define IW_HESSID_OFFSET			3
#define SHORT_IW_HESSID_OFFSET	1
#define IWAP_QUERY_INFO_SIZE		1

/* module */
extern bool wlc_11u_check_probe_req_iw(void *handle, wlc_bsscfg_t *cfg,
	wlc_d11rxhdr_t *wrxh, uint8 *plcp, struct dot11_management_header *hdr,
	uint8 *body, int body_len, bool *psendProbeResp);

extern wlc_11u_info_t *wlc_11u_attach(wlc_info_t *wlc);
extern void wlc_11u_detach(wlc_11u_info_t *m11u);
extern uint8 *wlc_11u_get_ie(wlc_11u_info_t *m11u, wlc_bsscfg_t *cfg, uint8 ie_type);
extern int wlc_11u_set_ie(wlc_11u_info_t *m11u, wlc_bsscfg_t *cfg, uint8 *ie_data,
	bool *bcn_upd, bool *prbresp_upd);
extern int wlc_11u_set_rx_qos_map_ie(wlc_11u_info_t *m11u, wlc_bsscfg_t *cfg,
	bcm_tlv_t *ie, int ie_len);
extern bool wlc_11u_is_11u_ie(wlc_11u_info_t *m11u, uint8 ie_type);
extern void wlc_11u_set_pkt_prio(wlc_11u_info_t *m11u, wlc_bsscfg_t *cfg, void *pkt);

#else /* !WL11U */
static INLINE wlc_11u_info_t *wlc_11u_attach(wlc_info_t *wlc)
{
	return NULL;
}
static INLINE void wlc_11u_detach(wlc_11u_info_t *m11u)
{
	return;
}
static INLINE bool wlc_11u_iw_activated(wlc_11u_info_t *m11u, wlc_bsscfg_t *cfg)
{
	return FALSE;
}
static INLINE uint8 *wlc_11u_get_ie(wlc_11u_info_t *m11u, wlc_bsscfg_t *cfg, uint8 ie_type)
{
	return NULL;
}
static INLINE int wlc_11u_set_ie(wlc_11u_info_t *m11u, wlc_bsscfg_t *cfg, uint8 *ie_data,
	bool *bcn_upd, bool *prbresp_upd)
{
	return BCME_UNSUPPORTED;
}
static INLINE bool wlc_11u_is_11u_ie(wlc_11u_info_t *m11u, uint8 ie_type)
{
	return FALSE;
}

/* do nothing */
#define wlc_11u_set_pkt_prio(m11u, cfg, pkt)
#endif /* !WL11U */

#endif /* _wlc_11u_h_ */
