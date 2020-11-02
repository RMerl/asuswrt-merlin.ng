/*
 * 802.11h TPC and wl power control module header file
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
 * $Id: wlc_tpc.h 708017 2017-06-29 14:11:45Z $
*/

#ifndef _wlc_tpc_h_
#define _wlc_tpc_h_

/* APIs */
#ifdef WLTPC

/* module */
extern wlc_tpc_info_t *wlc_tpc_attach(wlc_info_t *wlc);
extern void wlc_tpc_detach(wlc_tpc_info_t *tpc);

/* utilities */
extern void wlc_tpc_rep_build(wlc_info_t *wlc, int8 rssi, ratespec_t rspec,
	dot11_tpc_rep_t *rpt);

/* action frame recv/send */
extern void wlc_recv_tpc_request(wlc_tpc_info_t *tpc, wlc_bsscfg_t *cfg,
	struct dot11_management_header *hdr, uint8 *body, int body_len,
	int8 rssi, ratespec_t rspec);
extern void wlc_recv_tpc_report(wlc_tpc_info_t *tpc, wlc_bsscfg_t *cfg,
	struct dot11_management_header *hdr, uint8 *body, int body_len,
	int8 rssi, ratespec_t rspec);

extern void wlc_send_tpc_request(wlc_tpc_info_t *tpc, wlc_bsscfg_t *cfg, struct ether_addr *da);
extern void wlc_send_tpc_report(wlc_tpc_info_t *tpc, wlc_bsscfg_t *cfg, struct ether_addr *da,
	uint8 token, int8 rssi, ratespec_t rspec);

#ifdef WL_AP_TPC
extern void wlc_ap_tpc_assoc_reset(wlc_tpc_info_t *tpc, struct scb *scb);
extern void wlc_ap_bss_tpc_setup(wlc_tpc_info_t *tpc, wlc_bsscfg_t *cfg);
#else
#define wlc_ap_tpc_assoc_reset(tpc, scb) do {} while (0)
#define wlc_ap_bss_tpc_setup(tpc, cfg) do {} while (0)
#endif // endif

/* power management */
extern void wlc_tpc_reset_all(wlc_tpc_info_t *tpc);
extern uint8 wlc_tpc_get_local_constraint_qdbm(wlc_tpc_info_t *tpc);

/* accessors */
extern void wlc_tpc_set_local_max(wlc_tpc_info_t *tpc, uint8 pwr);

#else /* !WLTPC */

#define wlc_tpc_attach(wlc) NULL
#define wlc_tpc_detach(tpc) do {} while (0)

#define wlc_tpc_rep_build(wlc, rssi, rspec, tpc_rep) do {} while (0)

#define wlc_recv_tpc_request(tpc, cfg, hdr, body, body_len, rssi, rspec) do {} while (0)
#define wlc_recv_tpc_report(tpc, cfg, hdr, body, body_len, rssi, rspec) do {} while (0)

#define wlc_send_tpc_request(tpc, cfg, da) do {} while (0)
#define wlc_send_tpc_report(tpc, cfg, da, token, rssi, rspec) do {} while (0)

#define wlc_ap_tpc_assoc_reset(tpc, scb) do {} while (0)

#define wlc_tpc_reset_all(tpc) do {} while (0)
#define wlc_tpc_set_local_constraint(tpc, pwr) do {} while (0)
#define wlc_tpc_get_local_constraint_qdbm(tpc) 0

#define wlc_tpc_set_local_max(tpc, pwr) do {} while (0)

#endif /* !WLTPC */

#endif /* _wlc_tpc_h_ */
