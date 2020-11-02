/*
 * STA monitor interface
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
 * $Id: wlc_stamon.h 782025 2019-12-09 10:23:05Z $
 */

/** This is an AP/router specific feature. Twiki: [STASniffingModeOnAP] */

#ifndef _WLC_STAMON_H_
#define _WLC_STAMON_H_

#ifdef WL_STA_MONITOR_COMP
#define STA_MONITORING(_wlc_, _ea_) \
	((wlc_stamon_sta_num((_wlc_)->stamon_info) > 0) && \
	(wlc_stamon_sta_find((_wlc_)->stamon_info, (_ea_)) >= 0))
#else
#define STA_MONITORING(_wlc_, _ea_) \
	((wlc_stamon_sta_num((_wlc_)->stamon_info) > 0) && \
	wlc_stamon_sta_find((_wlc_)->stamon_info, (_ea_)))
#endif /* WL_STA_MONITOR_COMP */

/*
 * Initialize sta monitor private context.
 * Returns a pointer to the sta monitor private context, NULL on failure.
 */
extern wlc_stamon_info_t *wlc_stamon_attach(wlc_info_t *wlc);
/* Cleanup sta monitor private context */
extern void wlc_stamon_detach(wlc_stamon_info_t *stamon_ctxt);

extern int wlc_stamon_sta_config(wlc_stamon_info_t *stamon_ctxt,
	wlc_stamon_sta_config_t* cfg);

extern int8 wlc_stamon_sta_find_index(wlc_stamon_info_t *stamon_ctxt, const struct ether_addr *ea);
#ifdef WL_STA_MONITOR_COMP
extern int8 wlc_stamon_sta_find(wlc_stamon_info_t *stamon_ctxt, const struct ether_addr *ea);
#else
extern bool wlc_stamon_sta_find(wlc_stamon_info_t *stamon_ctxt, const struct ether_addr *ea);
#endif /* WL_STA_MONITOR_COMP */
extern int wlc_stamon_sta_sniff_enab(wlc_stamon_info_t *stamon_ctxt,
	struct ether_addr *ea, bool enab);
extern uint16 wlc_stamon_sta_num(wlc_stamon_info_t *stamon_ctxt);
#ifdef WL_STA_MONITOR_COMP
extern void wlc_stamon_rxcounters_update(wlc_info_t *wlc, void *p, bool reset);
#else
extern int wlc_stamon_sta_get(wlc_stamon_info_t *stamon_ctxt, struct ether_addr *ea);
#endif /* WL_STA_MONITOR_COMP */

extern int wlc_stamon_stats_update(wlc_info_t *wlc, const struct ether_addr* ea, int value);
#ifdef ACKSUPR_MAC_FILTER
extern bool wlc_stamon_acksupr_is_duplicate(wlc_info_t *wlc, struct ether_addr *ea);
extern bool wlc_stamon_is_slot_reserved(wlc_info_t *wlc, int idx);
#endif /* ACKSUPR_MAC_FILTER */
int wlc_stamon_rcmta_slots_free(wlc_stamon_info_t *ctxt);
#endif /* _WLC_STAMON_H_ */
