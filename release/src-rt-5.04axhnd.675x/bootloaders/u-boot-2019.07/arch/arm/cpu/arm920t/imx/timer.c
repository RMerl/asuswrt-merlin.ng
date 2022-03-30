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
 * Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
 */

#include <common.h>
#if defined (CONFIG_IMX)

#include <asm/arch/imx-regs.h>

int timer_init (void)
{
	int i;
	/* setup GP Timer 1 */
	TCTL1 = TCTL_SWR;
	for ( i=0; i<100; i++) TCTL1 = 0; /* We have no udelay by now */
	TPRER1 = get_PERCLK1() / 1000000; /* 1 MHz */
	TCTL1 |= TCTL_FRR | (1<<1); /* Freerun Mode, PERCLK1 input */

	/* Reset the timer */
	TCTL1 &= ~TCTL_TEN;
	TCTL1 |= TCTL_TEN; /* Enable timer */

	return (0);
}

/*
 * timer without interrupts
 */
static ulong get_timer_masked (void)
{
	return TCN1;
}

ulong get_timer (ulong base)
{
	return get_timer_masked() - base;
}

void __udelay (unsigned long usec)
{
	ulong endtime = get_timer_masked() + usec;
	signed long diff;

	do {
		ulong now = get_timer_masked ();
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
ulong get_tbclk (void)
{
	return CONFIG_SYS_HZ;
}

/*
 * Reset the cpu by setting up the watchdog timer and let him time out
 */
void reset_cpu (ulong ignored)
{
	/* Disable watchdog and set Time-Out field to 0 */
	WCR = 0x00000000;

	/* Write Service Sequence */
	WSR = 0x00005555;
	WSR = 0x0000AAAA;

	/* Enable watchdog */
	WCR = 0x00000001;

	while (1);
	/*NOTREACHED*/
}

#endif /* defined (CONFIG_IMX) */
