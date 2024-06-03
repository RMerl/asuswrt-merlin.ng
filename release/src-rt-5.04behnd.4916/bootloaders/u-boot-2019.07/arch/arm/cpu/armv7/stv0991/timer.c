// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2014, STMicroelectronics - All Rights Reserved
 * Author(s): Vikas Manocha, <vikas.manocha@st.com> for STMicroelectronics.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch-stv0991/hardware.h>
#include <asm/arch-stv0991/stv0991_cgu.h>
#include <asm/arch-stv0991/stv0991_gpt.h>

static struct stv0991_cgu_regs *const stv0991_cgu_regs = \
				(struct stv0991_cgu_regs *) (CGU_BASE_ADDR);

#define READ_TIMER()	(readl(&gpt1_regs_ptr->cnt) & GPT_FREE_RUNNING)
#define GPT_RESOLUTION	(CONFIG_STV0991_HZ_CLOCK / CONFIG_STV0991_HZ)

DECLARE_GLOBAL_DATA_PTR;

#define timestamp gd->arch.tbl
#define lastdec gd->arch.lastinc

static ulong get_timer_masked(void);

int timer_init(void)
{
	/* Timer1 clock configuration */
	writel(TIMER1_CLK_CFG, &stv0991_cgu_regs->tim_freq);
	writel(readl(&stv0991_cgu_regs->cgu_enable_2) |
			TIMER1_CLK_EN, &stv0991_cgu_regs->cgu_enable_2);

	/* Stop the timer */
	writel(readl(&gpt1_regs_ptr->cr1) & ~GPT_CR1_CEN, &gpt1_regs_ptr->cr1);
	writel(GPT_PRESCALER_128, &gpt1_regs_ptr->psc);
	/* Configure timer for auto-reload */
	writel(readl(&gpt1_regs_ptr->cr1) | GPT_MODE_AUTO_RELOAD,
			&gpt1_regs_ptr->cr1);

	/* load value for free running */
	writel(GPT_FREE_RUNNING, &gpt1_regs_ptr->arr);

	/* start timer */
	writel(readl(&gpt1_regs_ptr->cr1) | GPT_CR1_CEN,
			&gpt1_regs_ptr->cr1);

	/* Reset the timer */
	lastdec = READ_TIMER();
	timestamp = 0;

	return 0;
}

/*
 * timer without interrupts
 */
ulong get_timer(ulong base)
{
	return (get_timer_masked() / GPT_RESOLUTION) - base;
}

void __udelay(unsigned long usec)
{
	ulong tmo;
	ulong start = get_timer_masked();
	ulong tenudelcnt = CONFIG_STV0991_HZ_CLOCK / (1000 * 100);
	ulong rndoff;

	rndoff = (usec % 10) ? 1 : 0;

	/* tenudelcnt timer tick gives 10 microsecconds delay */
	tmo = ((usec / 10) + rndoff) * tenudelcnt;

	while ((ulong) (get_timer_masked() - start) < tmo)
		;
}

static ulong get_timer_masked(void)
{
	ulong now = READ_TIMER();

	if (now >= lastdec) {
		/* normal mode */
		timestamp += now - lastdec;
	} else {
		/* we have an overflow ... */
		timestamp += now + GPT_FREE_RUNNING - lastdec;
	}
	lastdec = now;

	return timestamp;
}

/*
 * This function is derived from PowerPC code (read timebase as long long).
 * On ARM it just returns the timer value.
 */
unsigned long long get_ticks(void)
{
	return get_timer(0);
}

/*
 * This function is derived from PowerPC code (timebase clock frequency).
 * On ARM it returns the number of timer ticks per second.
 */
ulong get_tbclk(void)
{
	return CONFIG_STV0991_HZ;
}
