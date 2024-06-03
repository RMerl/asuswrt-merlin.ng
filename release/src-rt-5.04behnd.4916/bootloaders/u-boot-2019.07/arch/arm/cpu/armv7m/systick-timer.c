// SPDX-License-Identifier: GPL-2.0+
/*
 * ARM Cortex M3/M4/M7 SysTick timer driver
 * (C) Copyright 2017 Renesas Electronics Europe Ltd
 *
 * Based on arch/arm/mach-stm32/stm32f1/timer.c
 * (C) Copyright 2015
 * Kamil Lulko, <kamil.lulko@gmail.com>
 *
 * Copyright 2015 ATS Advanced Telematics Systems GmbH
 * Copyright 2015 Konsulko Group, Matt Porter <mporter@konsulko.com>
 *
 * The SysTick timer is a 24-bit count down timer. The clock can be either the
 * CPU clock or a reference clock. Since the timer will wrap around very quickly
 * when using the CPU clock, and we do not handle the timer interrupts, it is
 * expected that this driver is only ever used with a slow reference clock.
 *
 * The number of reference clock ticks that correspond to 10ms is normally
 * defined in the SysTick Calibration register's TENMS field. However, on some
 * devices this is wrong, so this driver allows the clock rate to be defined
 * using CONFIG_SYS_HZ_CLOCK.
 */

#include <common.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

/* SysTick Base Address - fixed for all Cortex M3, M4 and M7 devices */
#define SYSTICK_BASE		0xE000E010

struct cm3_systick {
	uint32_t ctrl;
	uint32_t reload_val;
	uint32_t current_val;
	uint32_t calibration;
};

#define TIMER_MAX_VAL		0x00FFFFFF
#define SYSTICK_CTRL_EN		BIT(0)
/* Clock source: 0 = Ref clock, 1 = CPU clock */
#define SYSTICK_CTRL_CPU_CLK	BIT(2)
#define SYSTICK_CAL_NOREF	BIT(31)
#define SYSTICK_CAL_SKEW	BIT(30)
#define SYSTICK_CAL_TENMS_MASK	0x00FFFFFF

/* read the 24-bit timer */
static ulong read_timer(void)
{
	struct cm3_systick *systick = (struct cm3_systick *)SYSTICK_BASE;

	/* The timer counts down, therefore convert to an incrementing timer */
	return TIMER_MAX_VAL - readl(&systick->current_val);
}

int timer_init(void)
{
	struct cm3_systick *systick = (struct cm3_systick *)SYSTICK_BASE;
	u32 cal;

	writel(TIMER_MAX_VAL, &systick->reload_val);
	/* Any write to current_val reg clears it to 0 */
	writel(0, &systick->current_val);

	cal = readl(&systick->calibration);
	if (cal & SYSTICK_CAL_NOREF)
		/* Use CPU clock, no interrupts */
		writel(SYSTICK_CTRL_EN | SYSTICK_CTRL_CPU_CLK, &systick->ctrl);
	else
		/* Use external clock, no interrupts */
		writel(SYSTICK_CTRL_EN, &systick->ctrl);

	/*
	 * If the TENMS field is inexact or wrong, specify the clock rate using
	 * CONFIG_SYS_HZ_CLOCK.
	 */
#if defined(CONFIG_SYS_HZ_CLOCK)
	gd->arch.timer_rate_hz = CONFIG_SYS_HZ_CLOCK;
#else
	gd->arch.timer_rate_hz = (cal & SYSTICK_CAL_TENMS_MASK) * 100;
#endif

	gd->arch.tbl = 0;
	gd->arch.tbu = 0;
	gd->arch.lastinc = read_timer();

	return 0;
}

/* return milli-seconds timer value */
ulong get_timer(ulong base)
{
	unsigned long long t = get_ticks() * 1000;

	return (ulong)((t / gd->arch.timer_rate_hz)) - base;
}

unsigned long long get_ticks(void)
{
	u32 now = read_timer();

	if (now >= gd->arch.lastinc)
		gd->arch.tbl += (now - gd->arch.lastinc);
	else
		gd->arch.tbl += (TIMER_MAX_VAL - gd->arch.lastinc) + now;

	gd->arch.lastinc = now;

	return gd->arch.tbl;
}

ulong get_tbclk(void)
{
	return gd->arch.timer_rate_hz;
}
