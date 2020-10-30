/*
 * Copyright (C) 2019, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * Low resolution timer interface. Timer handlers may be called
 * in a deferred manner in a different task context after the
 * timer expires or in the task context from which the timer
 * was created, depending on the implementation.
 *
 * $Id: bcmtimer.h 774063 2019-04-09 06:11:02Z $
 */
#ifndef __bcmtimer_h__
#define __bcmtimer_h__

/* ANSI headers */
#include <time.h>
#ifdef RTCONFIG_HND_ROUTER_AX
#include <inttypes.h>
#endif

/* timer ID */
typedef unsigned long bcm_timer_module_id;
typedef unsigned long bcm_timer_id;

/* timer callback */
typedef void (*bcm_timer_cb)(bcm_timer_id id, unsigned int data);

/* OS-independant interfaces, applications should call these functions only */
int bcm_timer_module_init(int timer_entries, bcm_timer_module_id *module_id);
int bcm_timer_module_cleanup(bcm_timer_module_id module_id);
int bcm_timer_module_enable(bcm_timer_module_id module_id, int enable);
int bcm_timer_create(bcm_timer_module_id module_id, bcm_timer_id *timer_id);
int bcm_timer_delete(bcm_timer_id timer_id);
int bcm_timer_gettime(bcm_timer_id timer_id, struct itimerspec *value);
int bcm_timer_settime(bcm_timer_id timer_id, const struct itimerspec *value);
#ifdef RTCONFIG_HND_ROUTER_AX
int bcm_timer_connect(bcm_timer_id timer_id, bcm_timer_cb func, uintptr_t data);
#else
int bcm_timer_connect(bcm_timer_id timer_id, bcm_timer_cb func, int data);
#endif
int bcm_timer_cancel(bcm_timer_id timer_id);
int bcm_timer_change_expirytime(bcm_timer_id timer_id, const struct itimerspec *timer_spec);

#endif	/* #ifndef __bcmtimer_h__ */
