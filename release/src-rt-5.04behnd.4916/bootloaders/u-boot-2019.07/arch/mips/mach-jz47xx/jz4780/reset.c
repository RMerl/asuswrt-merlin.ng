// SPDX-License-Identifier: GPL-2.0+
/*
 * JZ4780 common routines
 *
 * Copyright (c) 2013 Imagination Technologies
 * Author: Paul Burton <paul.burton@imgtec.com>
 */

#include <config.h>
#include <common.h>
#include <asm/io.h>
#include <mach/jz4780.h>

/* WDT */
#define WDT_TDR		0x00
#define WDT_TCER	0x04
#define WDT_TCNT	0x08
#define WDT_TCSR	0x0C

/* Register definition */
#define WDT_TCSR_PRESCALE_BIT	3
#define WDT_TCSR_PRESCALE_MASK	(0x7 << WDT_TCSR_PRESCALE_BIT)
  #define WDT_TCSR_PRESCALE1	(0x0 << WDT_TCSR_PRESCALE_BIT)
  #define WDT_TCSR_PRESCALE4	(0x1 << WDT_TCSR_PRESCALE_BIT)
  #define WDT_TCSR_PRESCALE16	(0x2 << WDT_TCSR_PRESCALE_BIT)
  #define WDT_TCSR_PRESCALE64	(0x3 << WDT_TCSR_PRESCALE_BIT)
  #define WDT_TCSR_PRESCALE256	(0x4 << WDT_TCSR_PRESCALE_BIT)
  #define WDT_TCSR_PRESCALE1024	(0x5 << WDT_TCSR_PRESCALE_BIT)
#define WDT_TCSR_EXT_EN		BIT(2)
#define WDT_TCSR_RTC_EN		BIT(1)
#define WDT_TCSR_PCK_EN		BIT(0)

#define WDT_TCER_TCEN		BIT(0)

void _machine_restart(void)
{
	void __iomem *wdt_regs = (void __iomem *)WDT_BASE;

	/* EXTAL as the timer clock input. */
	writew(WDT_TCSR_PRESCALE1 | WDT_TCSR_EXT_EN, wdt_regs + WDT_TCSR);

	/* Reset the WDT counter and timeout. */
	writew(0, wdt_regs + WDT_TCNT);
	writew(0, wdt_regs + WDT_TDR);

	jz4780_tcu_wdt_start();

	/* WDT start */
	writeb(WDT_TCER_TCEN, wdt_regs + WDT_TCER);

	for (;;)
		;
}
