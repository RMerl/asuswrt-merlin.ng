/*
 * A few os-independent timer macros considered useful by some.
 *
 * Copyright 1998-1999 Epigram, Inc.
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
 * $Id: epitimers.h 708017 2017-06-29 14:11:45Z $
 */

#ifndef _EPITIMERS_H_
#define _EPITIMERS_H_

/*
 * the basic type used for arq timers, support defs below
 */
#if !SHORT_TIMERS

typedef uint tstamp_t;
typedef int _ststamp_t;

#define NO_TIMER (0)
#define TIMER_WRAP_MASK (0x80000000)
#define IL_INFINITE (TIMER_WRAP_MASK)
#define TSTAMP2UINT(stamp) ((uint)(stamp))

#else /* SHORT_TIMERS */

typedef uint16 tstamp_t;
typedef int16  _ststamp_t;

#define NO_TIMER (0)
#define TIMER_WRAP_MASK (0x8000)
#define IL_INFINITE (0xffff)
#define _STAMP_MASK 0xffff
#define TSTAMP2UINT(stamp) ((uint)((stamp) & _STAMP_MASK))

#endif /* !SHORT_TIMERS */
/*
 * Stuff for timer support
 *
 * A timer is off if its value is 0.
 *   A zero result for (now + interval) is always replaced with a 1
 *   Note that the routine that returns the current time in milliseconds
 *      must never return 0 as a value.
 */

#define NEW_TSTAMP(now, interval) \
(tstamp_t)(((tstamp_t)(now + interval) != NO_TIMER) ? now + interval : NO_TIMER + 1)

/*
 * a simple test to check expired timers, using wrapping tick counter
 *
 * Evaluates to TRUE if first arg is "later than" second.
 * Evaluates to FALSE otherwise, including the cases that the second
 *   argument is NO_TIMER or the timers are equal.
 * First arg should not be NO_TIMER.
 */
#define LATER_THAN(now, timer) \
	((timer != NO_TIMER) && ((((_ststamp_t)timer - (_ststamp_t)now) & TIMER_WRAP_MASK) != 0))

/*
 * a simple test to check for timers, using wrapping tick counter
 *
 * Evaluates to TRUE if first arg is "earlier than" second, including
 *   the case that the second argument is NO_TIMER.
 * Evaluates to FALSE otherwise, including when the timers are equal.
 * First arg should not be NO_TIMER.
 */
#define EARLIER_THAN(new, timer) \
	((timer == NO_TIMER) || ((((_ststamp_t)new - (_ststamp_t)timer) & TIMER_WRAP_MASK) != 0))

#endif /* _EPITIMERS_H_ */
