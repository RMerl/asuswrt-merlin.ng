/*
 * WL (per-port) event queue, for use by dongle offloads that need to process wl events
 * asynchronously. Not to be confused with wlc_eventq, which is used by the common code
 * to send events to the host.
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
 * $Id: wl_eventq.h 708017 2017-06-29 14:11:45Z $
 */

/**
 * XXX Apple specific feature
 * Twiki: [OffloadsEventQueue]
 */

#ifndef _wl_eventq_h_
#define _wl_eventq_h_

#include <wlc_types.h>

/* opaque private context */
typedef struct wl_eventq_info wl_eventq_info_t;

typedef void (*wl_eventq_cb_t)(void *ctx, uint32 event_type,
	wl_event_msg_t *wl_event, uint8 *data, uint32 length);

/*
 * Initialize wl eventq private context.
 * Returns a pointer to the wl eventq private context, NULL on failure.
 */
extern wl_eventq_info_t *wl_eventq_attach(wlc_info_t *wlc);

/* Cleanup wl event queue private context */
extern void wl_eventq_detach(wl_eventq_info_t *wlevtq);

/* register a callback fn to handle events */
extern int wl_eventq_register_event_cb(wl_eventq_info_t *wlevtq, uint32 event[], uint count,
	wl_eventq_cb_t cb, void *arg);

/* unregister a callback fn */
extern void wl_eventq_unregister_event_cb(wl_eventq_info_t *wlevtq, wl_eventq_cb_t cb);

/* duplicate event for wl event queue */
extern void wl_eventq_dup_event(wl_eventq_info_t *wlevtq, wlc_event_t *e);

#endif /* _wl_eventq_h_ */
