/*
 * packet tx complete callback management module interface
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
 * $Id: wlc_pcb.h 708017 2017-06-29 14:11:45Z $
 */

#ifndef _wlc_pcb_h_
#define _wlc_pcb_h_

/* module entries */
extern wlc_pcb_info_t *wlc_pcb_attach(wlc_info_t *wlc);
extern void wlc_pcb_detach(wlc_pcb_info_t *pcbi);

/* move callbacks from pkt and append them to new_pkt (for AMSDU) */
extern void wlc_pcb_fn_move(wlc_pcb_info_t *pcbi, void *new_pkt, void *pkt);

/* Set packet callback for a given packet.
 * It is called at the time a packet is added to the tx path.
 */
typedef void (*pkcb_fn_t)(wlc_info_t *wlc, uint txs, void *arg);
extern int wlc_pcb_fn_register(wlc_pcb_info_t *pcbi, pkcb_fn_t fn, void *arg, void *pkt);
/* Determine whether a callback <fn,arg> had been registered and optionally
 * cancel all matching callbacks. Returns the number of matching callbacks.
 */
extern int wlc_pcb_fn_find(wlc_pcb_info_t *pcbi, pkcb_fn_t fn, void *arg,
	bool cancel_all);

/* Set packet class callback for a class of packet.
 * It is normally called in the attach function of a module.
 * A packet is classified for a class callback by assigning a number to WLF2_PCBx_MASK bits
 * in flags2 field of the packet tag (or via WLF2_PCBx_REG()/WLF2_PCBx_UNREG() macros).
 * Currently each packet can have maximum 2 class callbacks WLF2_PCB1 and WLF2_PCB2
 * and them are invoked in the order of WLF2_PCB1 and WLF2_PCB2.
 */
typedef void (*wlc_pcb_fn_t)(wlc_info_t *wlc, void *pkt, uint txs);
extern int wlc_pcb_fn_set(wlc_pcb_info_t *pcbi, int tbl, int cls, wlc_pcb_fn_t pcb);

/* Invoke packet callback(s).
 * It is called in the tx path when the tx status is processed.
 */
extern void wlc_pcb_fn_invoke(wlc_pcb_info_t *pcbi, void *pkt, uint txs);

/* move the packet class callback chain attached from 'pkt_from' to 'pkt_to' */
extern void wlc_pcb_cb_move(wlc_pcb_info_t *pcbi, void *pkt_from, void *pkt_to);

/* register or unregister
* callback for all pkts prior to when they are freed
*/
typedef void (*wlc_pcb_pktfree_fn_t)(void *ctxt, void* pkt);

extern int wlc_pcb_pktfree_cb_register(wlc_pcb_info_t *pcbi,
	wlc_pcb_pktfree_fn_t fn, void* ctxt);
extern int wlc_pcb_pktfree_cb_unregister(wlc_pcb_info_t *pcbi,
	wlc_pcb_pktfree_fn_t fn, void* ctxt);

#endif /* _wlc_pcb_h_ */
