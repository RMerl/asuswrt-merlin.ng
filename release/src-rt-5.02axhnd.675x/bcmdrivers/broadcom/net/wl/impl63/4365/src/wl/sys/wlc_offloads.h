/*
 * Broadcom 802.11 host offload module
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
 * $Id: wlc_offloads.h 708017 2017-06-29 14:11:45Z $
 */

#ifndef _WL_OFFLOADS_H_
#define _WL_OFFLOADS_H_

#ifdef WLOFFLD
#include <bcm_ol_msg.h>

#define OL_CFG_MASK         0x1	/* For configuration */
#define OL_SCAN_MASK        0x2	/* For SCANNING */
#define OL_TBTT_MASK        0x4	/* For unaligned TBTT */
#define OL_AWAKEBCN_MASK    0x8	/* Stay Awake for Beacon */
#define OL_CSA_MASK         0x10 /* CSA IE Mask */
#define OL_RM_SCAN_MASK     0x20 /* Rm scan */

/* Whether offload capable or not */
extern bool wlc_ol_cap(wlc_info_t *wlc);

/* Offload module attach */
extern wlc_ol_info_t * wlc_ol_attach(wlc_info_t *wlc);

/* Offload module detach */
extern void wlc_ol_detach(wlc_ol_info_t *ol);
extern int wlc_ol_up(void *hdl);
extern int wlc_ol_down(void *hdl);
extern void wlc_ol_clear(wlc_ol_info_t *ol);
extern void wlc_ol_restart(wlc_ol_info_t *ol);

/* Returns true of the interrupt is from CR4 */
extern bool wlc_ol_intstatus(wlc_ol_info_t *ol);
/* DPC */
extern void wlc_ol_dpc(wlc_ol_info_t *ol);
extern void wlc_ol_mb_poll(wlc_ol_info_t *ol);
extern void wlc_ol_enable(wlc_ol_info_t *ol, wlc_bsscfg_t *cfg);
extern void wlc_ol_disable(wlc_ol_info_t *ol, wlc_bsscfg_t *cfg);
extern void wlc_ol_rx_deferral(wlc_ol_info_t *ol, uint32 mask, uint32 val);
extern bool wlc_ol_bcn_is_enable(wlc_ol_info_t *ol);
extern uint16 wlc_ol_get_state(wlc_ol_info_t *ol);
extern bool wlc_ol_time_since_bcn(wlc_ol_info_t *ol);
extern bool wlc_ol_arp_nd_enabled(wlc_ol_info_t *ol);
extern void wlc_ol_enable_intrs(wlc_ol_info_t *ol, bool enable);
extern bool wlc_ol_chkintstatus(wlc_ol_info_t *ol);
extern void wlc_ol_armtx(wlc_ol_info_t *ol, uint8 txEnable);
extern void wlc_ol_armtxdone(wlc_ol_info_t *ol, void *msg);
extern void wlc_set_tsf_update(wlc_ol_info_t *ol, bool value);
extern bool wlc_get_tsf_update(wlc_ol_info_t *ol);
extern bool wlc_ol_get_arm_txtstatus(wlc_ol_info_t *ol);
extern int wlc_ol_wowl_enable_start(
		    wlc_ol_info_t *ol,
		    wlc_bsscfg_t *cfg,
		    olmsg_wowl_enable_start *wowl_enable_start,
		    uint wowl_enable_len);

extern void wlc_ol_arm_halt(wlc_ol_info_t *ol);
extern bool wlc_ol_is_arm_halted(wlc_ol_info_t *ol);
extern int wlc_ol_wowl_enable_complete(wlc_ol_info_t *ol);
extern int wlc_ol_wowl_disable(wlc_ol_info_t *ol);
extern int wlc_ol_l2keepalive_enable(wlc_ol_info_t *ol);
extern int
wlc_ol_gtk_enable(wlc_ol_info_t *ol,
	rsn_rekey_params *rkey, struct scb *scb, int wpaauth);
void wlc_ol_rscupdate(wlc_ol_info_t *ol, void *rkey, void *msg);

void wlc_ol_update_sec_info(wlc_ol_info_t *ol, wlc_bsscfg_t *cfg,
    scb_t *scb, ol_sec_info *info);

#ifdef WL_LTR
extern void wlc_ol_ltr(wlc_ol_info_t *ol, wlc_ltr_info_t *ltr_info);
#endif /* WL_LTR */

#ifdef UCODE_SEQ
#if defined(WLOFFLD) || defined(SCANOL)
/* This function tell whether the BK tid seq/iv needs to be
 * copied from shared memory (frmShm)
 */
extern void wlc_ol_update_seq_iv(wlc_info_t *wlc, bool frmShm, struct scb *scb);
#endif // endif
#endif /* UCODE_SEQ */

extern int8 wlc_ol_rssi_get_value(wlc_ol_info_t *ol);
extern int8 wlc_ol_rssi_get_ant(wlc_ol_info_t *ol, uint32 ant_idx);
extern void wlc_ol_inc_rssi_cnt_arm(wlc_ol_info_t *ol);
extern void wlc_ol_inc_rssi_cnt_host(wlc_ol_info_t *ol);
extern void wlc_ol_inc_rssi_cnt_events(wlc_ol_info_t *ol);
extern void wlc_ol_curpwr_upd(wlc_ol_info_t *ol, int8 target_max_txpwr, chanspec_t chanspec);
extern void wlc_ol_print_cons(wlc_ol_info_t *ol);

/* Event log mechanism during sleep mode handlers and registration */
typedef void (*wlc_eventlog_print_handler_fn_t)(wlc_ol_info_t *ol, struct bcmstrbuf *b,
	uint8 type, uint32 time, uint32 data);
void	wlc_eventlog_register_print_handler(wlc_ol_info_t *ol, uint8 type,
	wlc_eventlog_print_handler_fn_t fn);

/* Definitions for chip memory space allocation */
#define OL_RAM_BASE_4360        0
#define OL_RAM_BASE_4350        0x180000
#define OL_RAM_BASE_43602       0x180000
#define OL_TEXT_START_4360      0
#define OL_TEXT_START_4350      0x180800
#define OL_TEXT_START_43602     0x180800

#endif /* WLOFFLD */

#endif /* _WL_OFFLOADS_H_ */
