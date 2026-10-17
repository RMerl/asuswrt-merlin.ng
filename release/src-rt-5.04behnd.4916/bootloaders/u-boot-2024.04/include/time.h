/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef _TIME_H
#define _TIME_H

#include <linux/typecheck.h>
#include <linux/types.h>

ulong get_tbclk(void);

unsigned long get_timer(unsigned long base);

/*
 * Return the current value of a monotonically increasing microsecond timer.
 * Granularity may be larger than 1us if hardware does not support this.
 */
unsigned long timer_get_us(void);
uint64_t get_timer_us(uint64_t base);

/**
 * get_timer_us_long() - Get the number of elapsed microseconds
 *
 * This uses 32-bit arithmetic on 32-bit machines, which is enough to handle
 * delays of over an hour. For 64-bit machines it uses a 64-bit value.
 *
 *@base: Base time to consider
 *Return: elapsed time since @base
 */
unsigned long get_timer_us_long(unsigned long base);

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

/* Same as above, but does so with platform independent 64bit types.
 * These must be used when utilizing jiffies_64 (i.e. return value of
 * get_jiffies_64() */
#define time_after64(a,b)	\
	(typecheck(__u64, a) &&	\
	 typecheck(__u64, b) && \
	 ((__s64)((b) - (a)) < 0))
#define time_before64(a,b)	time_after64(b,a)

#define time_after_eq64(a,b)	\
	(typecheck(__u64, a) && \
	 typecheck(__u64, b) && \
	 ((__s64)((a) - (b)) >= 0))
#define time_before_eq64(a,b)	time_after_eq64(b,a)

#define time_in_range64(a, b, c) \
	(time_after_eq64(a, b) && \
	 time_before_eq64(a, c))

/**
 * usec2ticks() - Convert microseconds to internal ticks
 *
 * @usec: Value of microseconds to convert
 * Return: Corresponding internal ticks value, calculated using get_tbclk()
 */
ulong usec2ticks(unsigned long usec);

/**
 * ticks2usec() - Convert internal ticks to microseconds
 *
 * @ticks: Value of ticks to convert
 * Return: Corresponding microseconds value, calculated using get_tbclk()
 */
ulong ticks2usec(unsigned long ticks);

/**
 * wait_ticks() - waits a given number of ticks
 *
 * This is an internal function typically used to implement udelay() and
 * similar. Normally you should use udelay() or mdelay() instead.
 *
 * @ticks: Number of ticks to wait
 */
void wait_ticks(unsigned long ticks);

/**
 * timer_get_us() - Get monotonic microsecond timer
 *
 * Return: value of monotonic microsecond timer
 */
unsigned long timer_get_us(void);

/**
 * get_ticks() - Get the current tick value
 *
 * This is an internal value used by the timer on the system. Ticks increase
 * monotonically at the rate given by get_tbclk().
 *
 * Return: current tick value
 */
uint64_t get_ticks(void);

#endif /* _TIME_H */
