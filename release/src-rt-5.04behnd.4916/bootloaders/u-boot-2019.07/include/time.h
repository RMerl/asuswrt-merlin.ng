/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef _TIME_H
#define _TIME_H

#include <linux/typecheck.h>
#include <linux/types.h>

unsigned long get_timer(unsigned long base);

/*
 * Return the current value of a monotonically increasing microsecond timer.
 * Granularity may be larger than 1us if hardware does not support this.
 */
unsigned long timer_get_us(void);

/*
 * timer_test_add_offset()
 *
 * Allow tests to add to the time reported through lib/time.c functions
 * offset: number of milliseconds to advance the system time
 */
void timer_test_add_offset(unsigned long offset);

/**
 * usec_to_tick() - convert microseconds to clock ticks
 *
 * @usec:	duration in microseconds
 * Return:	duration in clock ticks
 */
uint64_t usec_to_tick(unsigned long usec);

/*
 *	These inlines deal with timer wrapping correctly. You are
 *	strongly encouraged to use them
 *	1. Because people otherwise forget
 *	2. Because if the timer wrap changes in future you won't have to
 *	   alter your driver code.
 *
 * time_after(a,b) returns true if the time a is after time b.
 *
 * Do this with "<0" and ">=0" to only test the sign of the result. A
 * good compiler would generate better code (and a really good compiler
 * wouldn't care). Gcc is currently neither.
 */
#define time_after(a,b)		\
	(typecheck(unsigned long, a) && \
	 typecheck(unsigned long, b) && \
	 ((long)((b) - (a)) < 0))
#define time_before(a,b)	time_after(b,a)

#define time_after_eq(a,b)	\
	(typecheck(unsigned long, a) && \
	 typecheck(unsigned long, b) && \
	 ((long)((a) - (b)) >= 0))
#define time_before_eq(a,b)	time_after_eq(b,a)

/*
 * Calculate whether a is in the range of [b, c].
 */
#define time_in_range(a,b,c) \
	(time_after_eq(a,b) && \
	 time_before_eq(a,c))

/*
 * Calculate whether a is in the range of [b, c).
 */
#define time_in_range_open(a,b,c) \
	(time_after_eq(a,b) && \
	 time_before(a,c))

#endif /* _TIME_H */
