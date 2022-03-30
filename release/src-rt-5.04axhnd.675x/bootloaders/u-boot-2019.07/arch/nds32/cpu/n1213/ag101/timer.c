// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009 Faraday Technology
 * Po-Yu Chuang <ratbert@faraday-tech.com>
 *
 * Copyright (C) 2011 Andes Technology Corporation
 * Shawn Lin, Andes Technology Corporation <nobuhiro@andestech.com>
 * Macpaul Lin, Andes Technology Corporation <macpaul@andestech.com>
 */
#ifndef CONFIG_TIMER
#include <common.h>
#include <asm/io.h>
#include <faraday/fttmr010.h>

static ulong timestamp;
static ulong lastdec;

int timer_init(void)
{
	struct fttmr010 *tmr = (struct fttmr010 *)CONFIG_FTTMR010_BASE;
	unsigned int cr;

	debug("%s()\n", __func__);

	/* disable timers */
	writel(0, &tmr->cr);

#ifdef CONFIG_FTTMR010_EXT_CLK
	/* use 32768Hz oscillator for RTC, WDT, TIMER */
	ftpmu010_32768osc_enable();
#endif

	/* setup timer */
	writel(TIMER_LOAD_VAL, &tmr->timer3_load);
	writel(TIMER_LOAD_VAL, &tmr->timer3_counter);
	writel(0, &tmr->timer3_match1);
	writel(0, &tmr->timer3_match2);

	/* we don't want timer to issue interrupts */
	writel(FTTMR010_TM3_MATCH1 |
	       FTTMR010_TM3_MATCH2 |
	       FTTMR010_TM3_OVERFLOW,
	       &tmr->interrupt_mask);

	cr = readl(&tmr->cr);
#ifdef CONFIG_FTTMR010_EXT_CLK
	cr |= FTTMR010_TM3_CLOCK;	/* use external clock */
#endif
	cr |= FTTMR010_TM3_ENABLE;
	writel(cr, &tmr->cr);

	/* init the timestamp and lastdec value */
	reset_timer_masked();

	return 0;
}

/*
 * timer without interrupts
 */

/*
 * reset time
 */
void reset_timer_masked(void)
{
	struct fttmr010 *tmr = (struct fttmr010 *)CONFIG_FTTMR010_BASE;

	/* capure current decrementer value time */
#ifdef CONFIG_FTTMR010_EXT_CLK
	lastdec = readl(&tmr->timer3_counter) / (TIMER_CLOCK / CONFIG_SYS_HZ);
#else
	lastdec = readl(&tmr->timer3_counter) /
			(CONFIG_SYS_CLK_FREQ / 2 / CONFIG_SYS_HZ);
#endif
	timestamp = 0;		/* start "advancing" time stamp from 0 */

	debug("%s(): lastdec = %lx\n", __func__, lastdec);
}

void reset_timer(void)
{
	debug("%s()\n", __func__);
	reset_timer_masked();
}

/*
 * return timer ticks
 */
ulong get_timer_masked(void)
{
	struct fttmr010 *tmr = (struct fttmr010 *)CONFIG_FTTMR010_BASE;

	/* current tick value */
#ifdef CONFIG_FTTMR010_EXT_CLK
	ulong now = readl(&tmr->timer3_counter) / (TIMER_CLOCK / CONFIG_SYS_HZ);
#else
	ulong now = readl(&tmr->timer3_counter) /
			(CONFIG_SYS_CLK_FREQ / 2 / CONFIG_SYS_HZ);
#endif

	debug("%s(): now = %lx, lastdec = %lx\n", __func__, now, lastdec);

	if (lastdec >= now) {
		/*
		 * normal mode (non roll)
		 * move stamp fordward with absoulte diff ticks
		 */
		timestamp += lastdec - now;
	} else {
		/*
		 * we have overflow of the count down timer
		 *
		 * nts = ts + ld + (TLV - now)
		 * ts=old stamp, ld=time that passed before passing through -1
		 * (TLV-now) amount of time after passing though -1
		 * nts = new "advancing time stamp"...it could also roll and
		 * cause problems.
		 */
		timestamp += lastdec + TIMER_LOAD_VAL - now;
	}

	lastdec = now;

	debug("%s() returns %lx\n", __func__, timestamp);

	return timestamp;
}

/*
 * return difference between timer ticks and base
 */
ulong get_timer(ulong base)
{
	debug("%s(%lx)\n", __func__, base);
	return get_timer_masked() - base;
}

void set_timer(ulong t)
{
	debug("%s(%lx)\n", __func__, t);
	timestamp = t;
}

/* delay x useconds AND preserve advance timestamp value */
void __udelay(unsigned long usec)
{
	struct fttmr010 *tmr = (struct fttmr010 *)CONFIG_FTTMR010_BASE;

#ifdef CONFIG_FTTMR010_EXT_CLK
	long tmo = usec * (TIMER_CLOCK / 1000) / 1000;
#else
	long tmo = usec * ((CONFIG_SYS_CLK_FREQ / 2) / 1000) / 1000;
#endif
	unsigned long now, last = readl(&tmr->timer3_counter);

	debug("%s(%lu)\n", __func__, usec);
	while (tmo > 0) {
		now = readl(&tmr->timer3_counter);
		if (now > last) /* count down timer overflow */
			tmo -= TIMER_LOAD_VAL + last - now;
		else
			tmo -= last - now;
		last = now;
	}
}

/*
 * This function is derived from PowerPC code (read timebase as long long).
 * On ARM it just returns the timer value.
 */
unsigned long long get_ticks(void)
{
	debug("%s()\n", __func__);
	return get_timer(0);
}

/*
 * This function is derived from PowerPC code (timebase clock frequency).
 * On ARM it returns the number of timer ticks per second.
 */
ulong get_tbclk(void)
{
	debug("%s()\n", __func__);
#ifdef CONFIG_FTTMR010_EXT_CLK
	return CONFIG_SYS_HZ;
#else
	return CONFIG_SYS_CLK_FREQ;
#endif
}
#endif /* CONFIG_TIMER */
