// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
 *
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <gj@denx.de>
 *
 * (C) Copyright 2009
 * Ilya Yanok, Emcraft Systems Ltd, <yanok@emcraft.com>
 */

#include <common.h>
#include <div64.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>

/* General purpose timers bitfields */
#define GPTCR_SWR		(1 << 15)	/* Software reset	*/
#define GPTCR_FRR		(1 << 8)	/* Freerun / restart	*/
#define GPTCR_CLKSOURCE_32	(4 << 1)	/* Clock source		*/
#define GPTCR_TEN		1		/* Timer enable		*/

DECLARE_GLOBAL_DATA_PTR;

#define timestamp	(gd->arch.tbl)
#define lastinc		(gd->arch.lastinc)

/*
 * "time" is measured in 1 / CONFIG_SYS_HZ seconds,
 * "tick" is internal timer period
 */
#ifdef CONFIG_MX27_TIMER_HIGH_PRECISION
/* ~0.4% error - measured with stop-watch on 100s boot-delay */
static inline unsigned long long tick_to_time(unsigned long long tick)
{
	tick *= CONFIG_SYS_HZ;
	do_div(tick, CONFIG_MX27_CLK32);
	return tick;
}

static inline unsigned long long time_to_tick(unsigned long long time)
{
	time *= CONFIG_MX27_CLK32;
	do_div(time, CONFIG_SYS_HZ);
	return time;
}

static inline unsigned long long us_to_tick(unsigned long long us)
{
	us = us * CONFIG_MX27_CLK32 + 999999;
	do_div(us, 1000000);
	return us;
}
#else
/* ~2% error */
#define TICK_PER_TIME	((CONFIG_MX27_CLK32 + CONFIG_SYS_HZ / 2) / \
		CONFIG_SYS_HZ)
#define US_PER_TICK	(1000000 / CONFIG_MX27_CLK32)

static inline unsigned long long tick_to_time(unsigned long long tick)
{
	do_div(tick, TICK_PER_TIME);
	return tick;
}

static inline unsigned long long time_to_tick(unsigned long long time)
{
	return time * TICK_PER_TIME;
}

static inline unsigned long long us_to_tick(unsigned long long us)
{
	us += US_PER_TICK - 1;
	do_div(us, US_PER_TICK);
	return us;
}
#endif

/* nothing really to do with interrupts, just starts up a counter. */
/* The 32768Hz 32-bit timer overruns in 131072 seconds */
int timer_init(void)
{
	int i;
	struct gpt_regs *regs = (struct gpt_regs *)IMX_TIM1_BASE;
	struct pll_regs *pll = (struct pll_regs *)IMX_PLL_BASE;

	/* setup GP Timer 1 */
	writel(GPTCR_SWR, &regs->gpt_tctl);

	writel(readl(&pll->pccr0) | PCCR0_GPT1_EN, &pll->pccr0);
	writel(readl(&pll->pccr1) | PCCR1_PERCLK1_EN, &pll->pccr1);

	for (i = 0; i < 100; i++)
		writel(0, &regs->gpt_tctl); /* We have no udelay by now */
	writel(0, &regs->gpt_tprer); /* 32Khz */
	/* Freerun Mode, PERCLK1 input */
	writel(readl(&regs->gpt_tctl) | GPTCR_CLKSOURCE_32 | GPTCR_FRR,
			&regs->gpt_tctl);
	writel(readl(&regs->gpt_tctl) | GPTCR_TEN, &regs->gpt_tctl);

	return 0;
}

unsigned long long get_ticks(void)
{
	struct gpt_regs *regs = (struct gpt_regs *)IMX_TIM1_BASE;
	ulong now = readl(&regs->gpt_tcn); /* current tick value */

	if (now >= lastinc) {
		/*
		 * normal mode (non roll)
		 * move stamp forward with absolut diff ticks
		 */
		timestamp += (now - lastinc);
	} else {
		/* we have rollover of incrementer */
		timestamp += (0xFFFFFFFF - lastinc) + now;
	}
	lastinc = now;
	return timestamp;
}

static ulong get_timer_masked(void)
{
	/*
	 * get_ticks() returns a long long (64 bit), it wraps in
	 * 2^64 / CONFIG_MX27_CLK32 = 2^64 / 2^15 = 2^49 ~ 5 * 10^14 (s) ~
	 * 5 * 10^9 days... and get_ticks() * CONFIG_SYS_HZ wraps in
	 * 5 * 10^6 days - long enough.
	 */
	return tick_to_time(get_ticks());
}

ulong get_timer(ulong base)
{
	return get_timer_masked() - base;
}

/* delay x useconds AND preserve advance timstamp value */
void __udelay(unsigned long usec)
{
	unsigned long long tmp;
	ulong tmo;

	tmo = us_to_tick(usec);
	tmp = get_ticks() + tmo;	/* get current timestamp */

	while (get_ticks() < tmp)	/* loop till event */
		 /*NOP*/;
}

ulong get_tbclk(void)
{
	return CONFIG_MX27_CLK32;
}
