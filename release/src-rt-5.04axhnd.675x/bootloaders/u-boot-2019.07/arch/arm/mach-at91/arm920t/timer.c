// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2002
 * Lineo, Inc. <www.lineo.com>
 * Bernhard Kuhn <bkuhn@lineo.com>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
 */

#include <common.h>

#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/at91_tc.h>
#include <asm/arch/clk.h>

DECLARE_GLOBAL_DATA_PTR;

/* the number of clocks per CONFIG_SYS_HZ */
#define TIMER_LOAD_VAL (CONFIG_SYS_HZ_CLOCK/CONFIG_SYS_HZ)

int timer_init(void)
{
	at91_tc_t *tc = (at91_tc_t *) ATMEL_BASE_TC;

	at91_periph_clk_enable(ATMEL_ID_TC0);

	writel(0, &tc->bcr);
	writel(AT91_TC_BMR_TC0XC0S_NONE | AT91_TC_BMR_TC1XC1S_NONE |
		AT91_TC_BMR_TC2XC2S_NONE , &tc->bmr);

	writel(AT91_TC_CCR_CLKDIS, &tc->tc[0].ccr);
	/* set to MCLK/2 and restart the timer
	when the value in TC_RC is reached */
	writel(AT91_TC_CMR_TCCLKS_CLOCK1 | AT91_TC_CMR_CPCTRG, &tc->tc[0].cmr);

	writel(0xFFFFFFFF, &tc->tc[0].idr); /* disable interrupts */
	writel(TIMER_LOAD_VAL, &tc->tc[0].rc);

	writel(AT91_TC_CCR_SWTRG | AT91_TC_CCR_CLKEN, &tc->tc[0].ccr);
	gd->arch.lastinc = 0;
	gd->arch.tbl = 0;

	return 0;
}

/*
 * timer without interrupts
 */
ulong get_timer_raw(void)
{
	at91_tc_t *tc = (at91_tc_t *) ATMEL_BASE_TC;
	u32 now;

	now = readl(&tc->tc[0].cv) & 0x0000ffff;

	if (now >= gd->arch.lastinc) {
		/* normal mode */
		gd->arch.tbl += now - gd->arch.lastinc;
	} else {
		/* we have an overflow ... */
		gd->arch.tbl += now + TIMER_LOAD_VAL - gd->arch.lastinc;
	}
	gd->arch.lastinc = now;

	return gd->arch.tbl;
}

static ulong get_timer_masked(void)
{
	return get_timer_raw()/TIMER_LOAD_VAL;
}

ulong get_timer(ulong base)
{
	return get_timer_masked() - base;
}

void __udelay(unsigned long usec)
{
	u32 tmo;
	u32 endtime;
	signed long diff;

	tmo = CONFIG_SYS_HZ_CLOCK / 1000;
	tmo *= usec;
	tmo /= 1000;

	endtime = get_timer_raw() + tmo;

	do {
		u32 now = get_timer_raw();
		diff = endtime - now;
	} while (diff >= 0);
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
	return CONFIG_SYS_HZ;
}
