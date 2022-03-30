/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2011 by Vladimir Zapolskiy <vz@mleia.com>
 */

#ifndef _LPC32XX_WDT_H
#define _LPC32XX_WDT_H

#include <asm/types.h>

/* Watchdog Timer Registers */
struct wdt_regs {
	u32 isr;		/* Interrupt Status Register		*/
	u32 ctrl;		/* Control Register			*/
	u32 counter;		/* Counter Value Register		*/
	u32 mctrl;		/* Match Control Register		*/
	u32 match0;		/* Match 0 Register			*/
	u32 emr;		/* External Match Control Register	*/
	u32 pulse;		/* Reset Pulse Length Register		*/
	u32 res;		/* Reset Source Register		*/
};

/* Watchdog Timer Control Register bits */
#define WDTIM_CTRL_PAUSE_EN		(1 << 2)
#define WDTIM_CTRL_RESET_COUNT		(1 << 1)
#define WDTIM_CTRL_COUNT_ENAB		(1 << 0)

/* Watchdog Timer Match Control Register bits */
#define WDTIM_MCTRL_RESFRC2		(1 << 6)
#define WDTIM_MCTRL_RESFRC1		(1 << 5)
#define WDTIM_MCTRL_M_RES2		(1 << 4)
#define WDTIM_MCTRL_M_RES1		(1 << 3)
#define WDTIM_MCTRL_STOP_COUNT0		(1 << 2)
#define WDTIM_MCTRL_RESET_COUNT0	(1 << 1)
#define WDTIM_MCTRL_MR0_INT		(1 << 0)

#endif /* _LPC32XX_WDT_H */
