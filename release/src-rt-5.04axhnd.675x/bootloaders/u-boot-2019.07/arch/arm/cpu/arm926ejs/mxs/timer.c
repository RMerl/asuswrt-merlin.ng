// SPDX-License-Identifier: GPL-2.0+
/*
 * Freescale i.MX28 timer driver
 *
 * Copyright (C) 2011 Marek Vasut <marek.vasut@gmail.com>
 * on behalf of DENX Software Engineering GmbH
 *
 * Based on code from LTIB:
 * (C) Copyright 2009-2010 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/sys_proto.h>

/* Maximum fixed count */
#if defined(CONFIG_MX23)
#define TIMER_LOAD_VAL 0xffff
#elif defined(CONFIG_MX28)
#define TIMER_LOAD_VAL 0xffffffff
#endif

DECLARE_GLOBAL_DATA_PTR;

#define timestamp (gd->arch.tbl)
#define lastdec (gd->arch.lastinc)

/*
 * This driver uses 1kHz clock source.
 */
#define	MXS_INCREMENTER_HZ		1000

static inline unsigned long tick_to_time(unsigned long tick)
{
	return tick / (MXS_INCREMENTER_HZ / CONFIG_SYS_HZ);
}

static inline unsigned long time_to_tick(unsigned long time)
{
	return time * (MXS_INCREMENTER_HZ / CONFIG_SYS_HZ);
}

/* Calculate how many ticks happen in "us" microseconds */
static inline unsigned long us_to_tick(unsigned long us)
{
	return (us * MXS_INCREMENTER_HZ) / 1000000;
}

int timer_init(void)
{
	struct mxs_timrot_regs *timrot_regs =
		(struct mxs_timrot_regs *)MXS_TIMROT_BASE;

	/* Reset Timers and Rotary Encoder module */
	mxs_reset_block(&timrot_regs->hw_timrot_rotctrl_reg);

	/* Set fixed_count to 0 */
#if defined(CONFIG_MX23)
	writel(0, &timrot_regs->hw_timrot_timcount0);
#elif defined(CONFIG_MX28)
	writel(0, &timrot_regs->hw_timrot_fixed_count0);
#endif

	/* Set UPDATE bit and 1Khz frequency */
	writel(TIMROT_TIMCTRLn_UPDATE | TIMROT_TIMCTRLn_RELOAD |
		TIMROT_TIMCTRLn_SELECT_1KHZ_XTAL,
		&timrot_regs->hw_timrot_timctrl0);

	/* Set fixed_count to maximal value */
#if defined(CONFIG_MX23)
	writel(TIMER_LOAD_VAL - 1, &timrot_regs->hw_timrot_timcount0);
#elif defined(CONFIG_MX28)
	writel(TIMER_LOAD_VAL, &timrot_regs->hw_timrot_fixed_count0);
#endif

	return 0;
}

unsigned long long get_ticks(void)
{
	struct mxs_timrot_regs *timrot_regs =
		(struct mxs_timrot_regs *)MXS_TIMROT_BASE;
	uint32_t now;

	/* Current tick value */
#if defined(CONFIG_MX23)
	/* Upper bits are the valid ones. */
	now = readl(&timrot_regs->hw_timrot_timcount0) >>
		TIMROT_RUNNING_COUNTn_RUNNING_COUNT_OFFSET;
#elif defined(CONFIG_MX28)
	now = readl(&timrot_regs->hw_timrot_running_count0);
#else
#error "Don't know how to read timrot_regs"
#endif

	if (lastdec >= now) {
		/*
		 * normal mode (non roll)
		 * move stamp forward with absolut diff ticks
		 */
		timestamp += (lastdec - now);
	} else {
		/* we have rollover of decrementer */
		timestamp += (TIMER_LOAD_VAL - now) + lastdec;

	}
	lastdec = now;

	return timestamp;
}

ulong get_timer(ulong base)
{
	return tick_to_time(get_ticks()) - base;
}

/* We use the HW_DIGCTL_MICROSECONDS register for sub-millisecond timer. */
#define	MXS_HW_DIGCTL_MICROSECONDS	0x8001c0c0

void __udelay(unsigned long usec)
{
	uint32_t old, new, incr;
	uint32_t counter = 0;

	old = readl(MXS_HW_DIGCTL_MICROSECONDS);

	while (counter < usec) {
		new = readl(MXS_HW_DIGCTL_MICROSECONDS);

		/* Check if the timer wrapped. */
		if (new < old) {
			incr = 0xffffffff - old;
			incr += new;
		} else {
			incr = new - old;
		}

		/*
		 * Check if we are close to the maximum time and the counter
		 * would wrap if incremented. If that's the case, break out
		 * from the loop as the requested delay time passed.
		 */
		if (counter + incr < counter)
			break;

		counter += incr;
		old = new;
	}
}

ulong get_tbclk(void)
{
	return MXS_INCREMENTER_HZ;
}
