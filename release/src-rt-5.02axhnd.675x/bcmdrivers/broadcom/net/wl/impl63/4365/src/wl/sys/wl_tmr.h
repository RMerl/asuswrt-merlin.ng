/*
 * tmr module
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
 * $Id: wl_tmr.h 708017 2017-06-29 14:11:45Z $
 */

#ifndef _wl_tmr_h_
#define _wl_tmr_h_

#include <wlc.h>
#include <wl_export.h>

typedef struct wl_tmr {
	wlc_info_t		*wlc;
	struct wl_timer		*timer;
} tmrT;

/* create timer */
#ifndef tmrCreate
#define tmrCreate(wlc, fn, arg, name) \
	wl_init_tmr((wlc), (fn), (arg), (name))
#endif // endif
extern struct wl_tmr * wl_init_tmr(void *w,
	void (*fn)(void *arg), void *arg, const char *name);

/* destroy timer */
#ifndef tmrDestroy
#define tmrDestroy(tmr) \
	do { \
		ASSERT(tmr); \
		wl_free_timer((tmr)->wlc->wl, (tmr)->timer); \
		MFREE((tmr)->wlc->osh, (tmr), sizeof(tmrT)); \
	} while (0);
#endif // endif

/* start timer */
#ifndef tmrStart
#define tmrStart(tmr, ms, is_periodic) \
	do { \
		ASSERT(tmr); \
		wl_add_timer((tmr)->wlc->wl, (tmr)->timer, (ms), (is_periodic)); \
	} while (0);
#endif // endif

/* stop timer */
#ifndef tmrStop
#define tmrStop(tmr) \
	do { \
		ASSERT(tmr); \
		wl_del_timer((tmr)->wlc->wl, (tmr)->timer); \
	} while (0);
#endif // endif

#endif /* _wl_tmr_h_ */
