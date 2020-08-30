/*
 * EVENT_LOG system definitions
 *
 * Copyright (C) 2020, Broadcom. All Rights Reserved.
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
 * $Id: event_log_set.h 692821 2017-03-29 21:32:05Z $
 */

#ifndef _EVENT_LOG_SET_H_
#define _EVENT_LOG_SET_H_

/* Set a maximum number of sets here.  It is not dynamic for
 *  efficiency of the EVENT_LOG calls.
 */
#define NUM_EVENT_LOG_SETS 8

/* Define new event log sets here */
#define EVENT_LOG_SET_BUS	0
#define EVENT_LOG_SET_WL	1
#define EVENT_LOG_SET_PSM	2
#define EVENT_LOG_SET_ERROR	3
#define EVENT_LOG_SET_MEM_API	4
/* Share the set with MEM_API for now to limit ROM invalidation.
 * The above set is used in dingo only
 * On trunk, MSCH should move to a different set.
 */
#define EVENT_LOG_SET_MSCH_PROFILER	4
#define EVENT_LOG_SET_ECOUNTERS 5	/* Host to instantiate this for ecounters. */
#define EVENT_LOG_SET_6	6	/* Instantiated by host for channel switch logs */
#define EVENT_LOG_SET_7	7	/* Instantiated by host for AMPDU stats */

#endif /* _EVENT_LOG_SET_H_ */
