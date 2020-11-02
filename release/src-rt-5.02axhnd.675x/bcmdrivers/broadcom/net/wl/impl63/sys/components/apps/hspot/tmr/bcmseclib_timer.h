/*
 * bcmseclib_timer.h -- timer library interface
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
 * $Id: bcmseclib_timer.h,v 1.2 2010-12-11 00:06:34 $
 */

#ifndef _bcmseclib_timer_h_
#define _bcmseclib_timer_h_

#include <typedefs.h>

/* PRIVATE struct - only used internally within the timer module.
 *
 * Time structure with sec and usec units.
 */
typedef struct bcmseclib_time_t
{
	long sec;
	long usec;
} bcmseclib_time_t;

/* Opaque type for timer manager. */
typedef struct bcmseclib_timer_mgr bcmseclib_timer_mgr_t;

typedef struct bcmseclib_time_t exp_time_t;

typedef
struct bcmseclib_timer {
	struct bcmseclib_timer *next;
	void (*fn)(void *);
	void *arg; /* argument to fn */
	uint ms;
	bool periodic;
	bool set;
#ifdef BCMDBG
	char* name; /* Description of the timer */
#endif // endif
	exp_time_t expiry_time;	/* time to expiry */
	bcmseclib_timer_mgr_t *mgr; /* timer manager */
} bcmseclib_timer_t;

/* Activate a [previously created] timer with specified parms */
void
bcmseclib_add_timer(bcmseclib_timer_t *t, uint ms, bool periodic);

/* De-activate timer but don't delete it */
bool
bcmseclib_del_timer(bcmseclib_timer_t *t);

/* Remove from timer list and free allocated memory */
void
bcmseclib_free_timer(bcmseclib_timer_t *t);

/* Create the data structures, fill in callback args,
 * but do NOT activate
 */
#define bcmseclib_init_timer(cb, arg, name) bcmseclib_init_timer_ex(NULL, (cb), (arg), (name))
bcmseclib_timer_t *
bcmseclib_init_timer_ex(bcmseclib_timer_mgr_t *mgr, void (*fn)(void *arg),
	void *arg, const char *name);

#define bcmseclib_init_timer_utilities(n) bcmseclib_init_timer_utilities_ex((n), NULL)
int
bcmseclib_init_timer_utilities_ex(int ntimers, bcmseclib_timer_mgr_t **mgr);

#define bcmseclib_deinit_timer_utilities() bcmseclib_deinit_timer_utilities_ex(NULL)
int
bcmseclib_deinit_timer_utilities_ex(bcmseclib_timer_mgr_t *mgr);

#define bcmseclib_get_timeout(t) bcmseclib_get_timeout_ex(NULL, (t))
bool
bcmseclib_get_timeout_ex(bcmseclib_timer_mgr_t *mgr, exp_time_t *t);

#define bcmseclib_process_timer_expiry() bcmseclib_process_timer_expiry_ex(NULL)
void
bcmseclib_process_timer_expiry_ex(bcmseclib_timer_mgr_t *mgr);

#endif /* _bcmseclib_timer_h_ */
