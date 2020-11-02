/*
 * 802.11h module header file (top level and spectrum management, radar avoidance)
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
 * $Id: wlc_11h.h 708017 2017-06-29 14:11:45Z $
*/

#ifndef _wlc_11h_h_
#define _wlc_11h_h_

/* spect_state */
#define NEED_TO_UPDATE_BCN	(1 << 0)	/* Need to decrement counter in outgoing beacon */
#define NEED_TO_SWITCH_CHANNEL	(1 << 1)	/* A channel switch is pending */
#define RADAR_SIM		(1 << 2)	/* Simulate radar detection...for testing */
#define RADAR_SIM_SC		(1 << 3)	/* Simulate radar detection on scan core */

/* APIs */
#ifdef WL11H

/* module */
extern wlc_11h_info_t *wlc_11h_attach(wlc_info_t *wlc);
extern void wlc_11h_detach(wlc_11h_info_t *tpc);

/* TBTT proc */
extern void wlc_11h_tbtt(wlc_11h_info_t *m11h, wlc_bsscfg_t *cfg);

/* spectrum management */
extern void wlc_recv_frameaction_specmgmt(wlc_11h_info_t *m11h, struct dot11_management_header *hdr,
	uint8 *body, int body_len, int8 rssi, ratespec_t rspec);
extern bool wlc_validate_measure_req(wlc_11h_info_t *m11h, wlc_bsscfg_t *cfg,
	struct dot11_management_header *hdr);
extern int wlc_11h_set_spect(wlc_11h_info_t *m11h, uint spect);

/* accessors */
extern uint wlc_11h_get_spect(wlc_11h_info_t *m11h);
extern void wlc_11h_set_spect_state(wlc_11h_info_t *m11h, wlc_bsscfg_t *cfg, uint mask, uint val);
extern uint wlc_11h_get_spect_state(wlc_11h_info_t *m11h, wlc_bsscfg_t *cfg);
extern void wlc_11h_send_basic_report_radar(wlc_info_t *wlc, wlc_bsscfg_t *cfg,
	void (*func_ptr)(wlc_info_t *, uint, void *));

#else /* !WL11H */

#define wlc_11h_attach(wlc) NULL
#define wlc_11h_detach(m11h) do {} while (0)

#define wlc_11h_tbtt(m11h, cfg) do {} while (0)

#define wlc_recv_frameaction_specmgmt(m11h, hdr, body, body_len, rssi, rspec) do {} while (0)
#define wlc_validate_measure_req(m11h, cfg, hdr) FALSE
#define wlc_11h_set_spect(m11h, spect) BCME_ERROR

#define wlc_11h_get_spect(m11h) 0
#define wlc_11h_set_spect_state(m11h, cfg, mask, val) do {} while (0)
#define wlc_11h_get_spect_state(m11h, cfg) 0
#define wlc_11h_send_basic_report_radar(wlc, cfg, func_ptr) do {} while (0)

#endif /* !WL11H */

#endif /* _wlc_11h_h_ */
