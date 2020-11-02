/*
 * API to generic high resolution timer abstraction layer for multiplexing h/w timer
 * Broadcom 802.11abgn Networking Device Driver
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
 * $Id: wlc_hrt.h 708017 2017-06-29 14:11:45Z $
 */

#ifndef _wlc_hrt_h_
#define _wlc_hrt_h_

/* module attach/detach */
extern wlc_hrt_info_t *wlc_hrt_attach(wlc_info_t *wlc);
extern void wlc_hrt_detach(wlc_hrt_info_t *hrti);

/* h/w timer access routine registration
 * get_time: returns the current value of a free running counter h/w
 * set_timer: programs a timeout to the count-up/count-down timer h/w
 *            the timer shall generate an interrupt when the timeout expires
 * ack_timer: deserts the timer interrupt
 * max_timer_val: maximum value of the the free running counter
 */
typedef uint (*wlc_hrt_get_time_fn)(void *ctx);
typedef void (*wlc_hrt_set_timer_fn)(void *ctx, uint timeout);
typedef void (*wlc_hrt_ack_timer_fn)(void *ctx);
extern int wlc_hrt_hai_register(wlc_hrt_info_t *hrti,
	wlc_hrt_get_time_fn get_time, wlc_hrt_set_timer_fn set_timer,
	wlc_hrt_ack_timer_fn ack_timer,
	void *ctx, uint max_timer_val);

/* isr/dpc */
extern void wlc_hrt_isr(wlc_hrt_info_t *hrti);

/* timer object creation/deletion */
extern wlc_hrt_to_t *wlc_hrt_alloc_timeout(wlc_hrt_info_t *hrti);
extern void wlc_hrt_free_timeout(wlc_hrt_to_t *to);

/* timer arming/canceling */
typedef void (*wlc_hrt_to_cb_fn)(void *arg);
extern bool wlc_hrt_add_timeout(wlc_hrt_to_t *to, uint timeout, wlc_hrt_to_cb_fn fun, void *arg);
extern void wlc_hrt_del_timeout(wlc_hrt_to_t *to);

/* time accessors */
extern uint wlc_hrt_gettime(wlc_hrt_info_t *hrti);
extern uint wlc_hrt_getdelta(wlc_hrt_info_t *hrti, uint *last);

/* deprecated */
extern void wlc_hrt_gptimer_abort(wlc_info_t *wlc);
extern void wlc_hrt_gptimer_cb(wlc_info_t *wlc);

/* gptimer set/get */
extern void wlc_hrt_gptimer_set(wlc_info_t *wlc, uint us);
extern uint32 wlc_hrt_gptimer_get(wlc_info_t *wlc);

#endif /* !_wlc_hrt_h_ */
