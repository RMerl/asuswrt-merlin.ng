// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2013
 * David Feng <fenghua@phytium.com.cn>
 */

#include <common.h>
#include <command.h>
#include <asm/system.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Generic timer implementation of get_tbclk()
 */
unsigned long get_tbclk(void)
{
	unsigned long cntfrq;
	asm volatile("mrs %0, cntfrq_el0" : "=r" (cntfrq));
	return cntfrq;
}

#ifdef CONFIG_SYS_FSL_ERRATUM_A008585
/*
 * FSL erratum A-008585 says that the ARM generic timer counter "has the
 * potential to contain an erroneous value for a small number of core
 * clock cycles every time the timer value changes".
 * This sometimes leads to a consecutive counter read returning a lower
 * value than the previous one, thus reporting the time to go backwards.
 * The workaround is to read the counter twice and only return when the value
 * was the same in both reads.
 * Assumes that the CPU runs in much higher frequency than the timer.
 */
unsigned long timer_read_counter(void)
{
	unsigned long cntpct;
	unsigned long temp;

	isb();
	asm volatile("mrs %0, cntpct_el0" : "=r" (cntpct));
	asm volatile("mrs %0, cntpct_el0" : "=r" (temp));
	while (temp != cntpct) {
		asm volatile("mrs %0, cntpct_el0" : "=r" (cntpct));
		asm volatile("mrs %0, cntpct_el0" : "=r" (temp));
	}

	return cntpct;
}
#elif CONFIG_SUNXI_A64_TIMER_ERRATUM
/*
 * This erratum sometimes flips the lower 11 bits of the counter value
 * to all 0's or all 1's, leading to jumps forwards or backwards.
 * Backwards jumps might be interpreted all roll-overs and be treated as
 * huge jumps forward.
 * The workaround is to check whether the lower 11 bits of the counter are
 * all 0 or all 1, then discard this value and read again.
 * This occasionally discards valid values, but will catch all erroneous
 * reads and fixes the problem reliably. Also this mostly requires only a
 * single read, so does not have any significant overhead.
 * The algorithm was conceived by Samuel Holland.
 */
unsigned long timer_read_counter(void)
{
	unsigned long cntpct;

	isb();
	do {
		asm volatile("mrs %0, cntpct_el0" : "=r" (cntpct));
	} while (((cntpct + 1) & GENMASK(10, 0)) <= 1);

	return cntpct;
}
#else
/*
 * timer_read_counter() using the Arm Generic Timer (aka arch timer).
 */
unsigned long timer_read_counter(void)
{
	unsigned long cntpct;

	isb();
	asm volatile("mrs %0, cntpct_el0" : "=r" (cntpct));

	return cntpct;
}
#endif

uint64_t get_ticks(void)
{
	unsigned long ticks = timer_read_counter();

	gd->arch.tbl = ticks;

	return ticks;
}

unsigned long usec2ticks(unsigned long usec)
{
	ulong ticks;
	if (usec < 1000)
		ticks = ((usec * (get_tbclk()/1000)) + 500) / 1000;
	else
		ticks = ((usec / 10) * (get_tbclk() / 100000));

	return ticks;
}

ulong timer_get_boot_us(void)
{
	u64 val = get_ticks() * 1000000;

	return val / get_tbclk();
}
