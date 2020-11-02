/**
 * Required functions exported by the wlc_aibss.c to common (os-independent) driver code.
 *
 * Advanced IBSS mode
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
 * $Id: wlc_aibss.h 708017 2017-06-29 14:11:45Z $
 */

#ifndef _WLC_AIBSS_H_
#define _WLC_AIBSS_H_

#ifdef	WLAIBSS

typedef struct aibss_scb_info {
	int32	tx_noack_count;
	uint16	no_bcn_counter;
	uint16	bcn_count;
	bool	pkt_pend;
	bool	atim_acked;
	bool	atim_rcvd;
	uint16	atim_failure_count;
} aibss_scb_info_t;

struct wlc_aibss_info {
	int32 scb_handle;		/* SCB CUBBY OFFSET */
};

enum aibss_peer_txfail {
	AIBSS_TX_FAILURE = 0,
	AIBSS_BCN_FAILURE = 1,
	AIBSS_ATIM_FAILURE = 2
};

/* access the variables via a macro */
#define WLC_AIBSS_INFO_SCB_HDL(a) ((a)->scb_handle)

extern wlc_aibss_info_t *wlc_aibss_attach(wlc_info_t *wlc);
extern void wlc_aibss_detach(wlc_aibss_info_t *aibss);
extern void wlc_aibss_check_txfail(wlc_aibss_info_t *aibss, wlc_bsscfg_t *cfg, struct scb *scb);
extern void wlc_aibss_tbtt(wlc_aibss_info_t *aibss);
extern bool wlc_aibss_sendpmnotif(wlc_aibss_info_t *aibss, wlc_bsscfg_t *cfg,
	ratespec_t rate_override, int prio, bool track);
extern void wlc_aibss_atim_window_end(wlc_info_t *wlc);
extern void wlc_aibss_ps_start(wlc_info_t *wlc, wlc_bsscfg_t *cfg);
extern void wlc_aibss_ps_stop(wlc_info_t *wlc, wlc_bsscfg_t *cfg);
extern void wlc_aibss_ps_process_atim(wlc_info_t *wlc, struct ether_addr *ea);
#else
#define wlc_aibss_atim_window_end(a)	do {} while (0)
#define wlc_aibss_ps_start(a, b)	do {} while (0)
#define wlc_aibss_ps_stop(a, b)	do {} while (0)
#define wlc_aibss_ps_process_atim(a, b)	do {} while (0)
#endif /* WLAIBSS */
#endif /* _WLC_AIBSS_H_ */
