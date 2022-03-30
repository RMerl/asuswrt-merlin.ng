// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian@popies.net>
 * Lead Tech Design <www.leadtechdesign.com>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/at91_pit.h>
#include <asm/arch/clk.h>
#include <div64.h>

#if !defined(CONFIG_AT91FAMILY)
# error You need to define CONFIG_AT91FAMILY in your board config!
#endif

DECLARE_GLOBAL_DATA_PTR;

/*
 * We're using the AT91CAP9/SAM9 PITC in 32 bit mode, by
 * setting the 20 bit counter period to its maximum (0xfffff).
 * (See the relevant data sheets to understand that this really works)
 *
 * We do also mimic the typical powerpc way of incrementing
 * two 32 bit registers called tbl and tbu.
 *
 * Those registers increment at 1/16 the main clock rate.
 */

#define TIMER_LOAD_VAL	0xfffff

/*
 * Use the PITC in full 32 bit incrementing mode
 */
int timer_init(void)
{
	at91_pit_t *pit = (at91_pit_t *) ATMEL_BASE_PIT;

	at91_periph_clk_enable(ATMEL_ID_SYS);

	/* Enable PITC */
	writel(TIMER_LOAD_VAL | AT91_PIT_MR_EN , &pit->mr);

	gd->arch.timer_rate_hz = gd->arch.mck_rate_hz / 16;

	return 0;
}

/*
 * Return the number of timer ticks per second.
 */
ulong get_tbclk(void)
{
	return gd->arch.timer_rate_hz;
}
