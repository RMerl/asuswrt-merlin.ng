// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <asm/io.h>
#include <div64.h>
#include <asm/arch/immap_ls102xa.h>
#include <asm/arch/clock.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * This function is intended for SHORT delays only.
 * It will overflow at around 10 seconds @ 400MHz,
 * or 20 seconds @ 200MHz.
 */
unsigned long usec2ticks(unsigned long usec)
{
	ulong ticks;

	if (usec < 1000)
		ticks = ((usec * (get_tbclk()/1000)) + 500) / 1000;
	else
		ticks = ((usec / 10) * (get_tbclk() / 100000));

	return ticks;
}

static inline unsigned long long tick_to_time(unsigned long long tick)
{
	unsigned long freq;

	asm volatile("mrc p15, 0, %0, c14, c0, 0" : "=r" (freq));

	tick *= CONFIG_SYS_HZ;
	do_div(tick, freq);

	return tick;
}

static inline unsigned long long us_to_tick(unsigned long long usec)
{
	unsigned long freq;

	asm volatile("mrc p15, 0, %0, c14, c0, 0" : "=r" (freq));

	usec = usec * freq  + 999999;
	do_div(usec, 1000000);

	return usec;
}

int timer_init(void)
{
	struct sctr_regs *sctr = (struct sctr_regs *)SCTR_BASE_ADDR;
	unsigned long ctrl, freq;
	unsigned long long val;

	/* Enable System Counter */
	writel(SYS_COUNTER_CTRL_ENABLE, &sctr->cntcr);

	freq = COUNTER_FREQUENCY;
	asm("mcr p15, 0, %0, c14, c0, 0" : : "r" (freq));

	/* Set PL1 Physical Timer Ctrl */
	ctrl = ARCH_TIMER_CTRL_ENABLE;
	asm("mcr p15, 0, %0, c14, c2, 1" : : "r" (ctrl));

	/* Set PL1 Physical Comp Value */
	val = TIMER_COMP_VAL;
	asm("mcrr p15, 2, %Q0, %R0, c14" : : "r" (val));

	gd->arch.tbl = 0;
	gd->arch.tbu = 0;

	return 0;
}

unsigned long long get_ticks(void)
{
	unsigned long long now;

	asm("mrrc p15, 0, %Q0, %R0, c14" : "=r" (now));

	gd->arch.tbl = (unsigned long)(now & 0xffffffff);
	gd->arch.tbu = (unsigned long)(now >> 32);

	return now;
}

unsigned long get_timer(ulong base)
{
	return tick_to_time(get_ticks()) - base;
}

/* delay x useconds and preserve advance timstamp value */
void __udelay(unsigned long usec)
{
	unsigned long long start;
	unsigned long tmo;

	start = get_ticks();			/* get current timestamp */
	tmo = us_to_tick(usec);			/* convert usecs to ticks */

	while ((get_ticks() - start) < tmo)
		;				/* loop till time has passed */
}

/*
 * This function is derived from PowerPC code (timebase clock frequency).
 * On ARM it returns the number of timer ticks per second.
 */
unsigned long get_tbclk(void)
{
	unsigned long freq;

	asm volatile("mrc p15, 0, %0, c14, c0, 0" : "=r" (freq));

	return freq;
}
