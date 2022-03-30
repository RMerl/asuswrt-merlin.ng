/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2011 by Vladimir Zapolskiy <vz@mleia.com>
 */

#ifndef _LPC32XX_TIMER_H
#define _LPC32XX_TIMER_H

#include <asm/types.h>

/* Timer/Counter Registers */
struct timer_regs {
	u32 ir;			/* Interrupt Register		*/
	u32 tcr;		/* Timer Control Register	*/
	u32 tc;			/* Timer Counter		*/
	u32 pr;			/* Prescale Register		*/
	u32 pc;			/* Prescale Counter		*/
	u32 mcr;		/* Match Control Register	*/
	u32 mr[4];		/* Match Registers		*/
	u32 ccr;		/* Capture Control Register	*/
	u32 cr[4];		/* Capture Registers		*/
	u32 emr;		/* External Match Register	*/
	u32 reserved[12];
	u32 ctcr;		/* Count Control Register	*/
};

/* Timer/Counter Interrupt Register bits */
#define TIMER_IR_CR(n)			(1 << ((n) + 4))
#define TIMER_IR_MR(n)			(1 << (n))

/* Timer/Counter Timer Control Register bits */
#define TIMER_TCR_COUNTER_RESET		(1 << 1)
#define TIMER_TCR_COUNTER_ENABLE	(1 << 0)
#define TIMER_TCR_COUNTER_DISABLE	(0 << 0)

/* Timer/Counter Match Control Register bits */
#define TIMER_MCR_STOP(n)		(1 << (3 * (n) + 2))
#define TIMER_MCR_RESET(n)		(1 << (3 * (n) + 1))
#define TIMER_MCR_INTERRUPT(n)		(1 << (3 * (n)))

/* Timer/Counter Capture Control Register bits */
#define TIMER_CCR_INTERRUPT(n)		(1 << (3 * (n) + 2))
#define TIMER_CCR_FALLING_EDGE(n)	(1 << (3 * (n) + 1))
#define TIMER_CCR_RISING_EDGE(n)	(1 << (3 * (n)))

/* Timer/Counter External Match Register bits */
#define TIMER_EMR_EMC_TOGGLE(n)		(0x3 << (2 * (n) + 4))
#define TIMER_EMR_EMC_SET(n)		(0x2 << (2 * (n) + 4))
#define TIMER_EMR_EMC_CLEAR(n)		(0x1 << (2 * (n) + 4))
#define TIMER_EMR_EMC_NOTHING(n)	(0x0 << (2 * (n) + 4))
#define TIMER_EMR_EM(n)			(1 << (n))

/* Timer/Counter Count Control Register bits */
#define TIMER_CTCR_INPUT(n)		((n) << 2)
#define TIMER_CTCR_MODE_COUNTER_BOTH	(0x3 << 0)
#define TIMER_CTCR_MODE_COUNTER_FALLING	(0x2 << 0)
#define TIMER_CTCR_MODE_COUNTER_RISING	(0x1 << 0)
#define TIMER_CTCR_MODE_TIMER		(0x0 << 0)

#endif /* _LPC32XX_TIMER_H */
