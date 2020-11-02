/*
 * Event mechanism
 *
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
 * $Id: wlc_event.h 708017 2017-06-29 14:11:45Z $
 */

#ifndef _WLC_EVENT_H_
#define _WLC_EVENT_H_

#include <typedefs.h>

typedef struct wlc_eventq wlc_eventq_t;

typedef void (*wlc_eventq_cb_t)(void *arg);

extern wlc_eventq_t *wlc_eventq_attach(wlc_pub_t *pub, struct wlc_info *wlc,
	void *wl, wlc_eventq_cb_t cb);
extern int wlc_eventq_detach(wlc_eventq_t *eq);
extern int wlc_eventq_down(wlc_eventq_t *eq);
extern void wlc_event_free(wlc_eventq_t *eq, wlc_event_t *e);
extern wlc_event_t *wlc_eventq_next(wlc_eventq_t *eq, wlc_event_t *e);
extern int wlc_eventq_cnt(wlc_eventq_t *eq);
extern bool wlc_eventq_avail(wlc_eventq_t *eq);
extern wlc_event_t *wlc_eventq_deq(wlc_eventq_t *eq);
extern void wlc_eventq_enq(wlc_eventq_t *eq, wlc_event_t *e);
extern wlc_event_t *wlc_event_alloc(wlc_eventq_t *eq);

#ifdef WLNOEIND
#define wlc_eventq_register_ind_ext(a, b, c) 0
#define wlc_eventq_query_ind_ext(a, b, c, d) 0
#define wlc_eventq_test_ind(a, b) FALSE
#define wlc_eventq_handle_ind(a, b) do {} while (0)
#define wlc_eventq_set_ind(a, b, c) do {} while (0)
#define wlc_eventq_flush(eq) do {} while (0)
#define wlc_assign_event_msg(a, b, c, d, e) do {} while (0)
#else /* WLNOEIND */
extern int wlc_eventq_register_ind_ext(wlc_eventq_t *eq, eventmsgs_ext_t* iovar_msg, uint8 *mask);
extern int wlc_eventq_query_ind_ext(wlc_eventq_t *eq, eventmsgs_ext_t* in_iovar_msg,
	eventmsgs_ext_t* out_iovar_msg, uint8 *mask);
extern int wlc_eventq_test_ind(wlc_eventq_t *eq, int et);
extern int wlc_eventq_handle_ind(wlc_eventq_t* eq, wlc_event_t *e);
extern int wlc_eventq_set_ind(wlc_eventq_t* eq, uint et, bool on);
extern void wlc_eventq_flush(wlc_eventq_t *eq);
extern void wlc_assign_event_msg(wlc_info_t *wlc, wl_event_msg_t *msg, const wlc_event_t *e,
	uint8 *data, uint32 len);

#endif /* WLNOEIND */

#if defined(MSGTRACE) || defined(LOGTRACE)
#include <rte_dev.h>
extern void wlc_event_sendup_trace(struct wlc_info * wlc, hnd_dev_t * bus, uint8* hdr,
                                   uint16 hdrlen, uint8 *buf, uint16 buflen);
#endif // endif

#endif  /* _WLC_EVENT_H_ */
