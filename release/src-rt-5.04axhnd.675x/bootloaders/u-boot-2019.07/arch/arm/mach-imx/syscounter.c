// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Freescale Semiconductor, Inc.
 *
 * The file use ls102xa/timer.c as a reference.
 */

#include <common.h>
#include <asm/io.h>
#include <div64.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/syscounter.h>

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

#ifndef CONFIG_SKIP_LOWLEVEL_INIT
int timer_init(void)
{
	struct sctr_regs *sctr = (struct sctr_regs *)SCTR_BASE_ADDR;
	unsigned long val, freq;

	freq = CONFIG_SC_TIMER_CLK;
	asm volatile("mcr p15, 0, %0, c14, c0, 0" : : "r" (freq));

	writel(freq, &sctr->cntfid0);

	/* Enable system counter */
	val = readl(&sctr->cntcr);
	val &= ~(SC_CNTCR_FREQ0 | SC_CNTCR_FREQ1);
	val |= SC_CNTCR_FREQ0 | SC_CNTCR_ENABLE | SC_CNTCR_HDBG;
	writel(val, &sctr->cntcr);

	gd->arch.tbl = 0;
	gd->arch.tbu = 0;

	return 0;
}
#endif

unsigned long long get_ticks(void)
{
	unsigned long long now;

	asm volatile("mrrc p15, 0, %Q0, %R0, c14" : "=r" (now));

	gd->arch.tbl = (unsigned long)(now & 0xffffffff);
	gd->arch.tbu = (unsigned long)(now >> 32);

	return now;
}

ulong get_timer(ulong base)
{
	return tick_to_time(get_ticks()) - base;
}

void __udelay(unsigned long usec)
{
	unsigned long long tmp;
	ulong tmo;

	tmo = us_to_tick(usec);
	tmp = get_ticks() + tmo;	/* get current timestamp */

	while (get_ticks() < tmp)	/* loop till event */
		 /*NOP*/;
}

/*
 * This function is derived from PowerPC code (timebase clock frequency).
 * On ARM it returns the number of timer ticks per second.
 */
ulong get_tbclk(void)
{
	unsigned long freq;

	asm volatile("mrc p15, 0, %0, c14, c0, 0" : "=r" (freq));

	return freq;
}
