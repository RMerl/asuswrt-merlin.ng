// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2007
 * Sascha Hauer, Pengutronix
 */

#include <common.h>
#include <asm/arch/imx-regs.h>
#include <asm/io.h>

#define TIMER_BASE 0x53f90000 /* General purpose timer 1 */

/* General purpose timers registers */
#define GPTCR	__REG(TIMER_BASE)		/* Control register	*/
#define GPTPR	__REG(TIMER_BASE + 0x4)		/* Prescaler register	*/
#define GPTSR	__REG(TIMER_BASE + 0x8)		/* Status register	*/
#define GPTCNT	__REG(TIMER_BASE + 0x24)	/* Counter register	*/

/* General purpose timers bitfields */
#define GPTCR_SWR		(1 << 15)	/* Software reset	*/
#define GPTCR_FRR		(1 << 9)	/* Freerun / restart	*/
#define GPTCR_CLKSOURCE_32	(4 << 6)	/* Clock source		*/
#define GPTCR_TEN		1		/* Timer enable		*/

/* The 32768Hz 32-bit timer overruns in 131072 seconds */
int timer_init(void)
{
	int i;

	/* setup GP Timer 1 */
	GPTCR = GPTCR_SWR;
	for (i = 0; i < 100; i++)
		GPTCR = 0; /* We have no udelay by now */
	GPTPR = 0; /* 32Khz */
	/* Freerun Mode, PERCLK1 input */
	GPTCR |= GPTCR_CLKSOURCE_32 | GPTCR_TEN;

	return 0;
}

unsigned long timer_read_counter(void)
{
	return GPTCNT;
}
