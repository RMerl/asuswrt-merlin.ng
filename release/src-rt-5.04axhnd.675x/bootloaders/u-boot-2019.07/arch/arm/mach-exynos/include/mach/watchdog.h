/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2011 Samsung Electronics
 * Heungjun Kim <riverful.kim@samsung.com>
 */

#ifndef __ASM_ARM_ARCH_WATCHDOG_H_
#define __ASM_ARM_ARCH_WATCHDOG_H_

#define WTCON_RESET_OFFSET	0
#define WTCON_INTEN_OFFSET	2
#define WTCON_CLKSEL_OFFSET	3
#define WTCON_EN_OFFSET		5
#define WTCON_PRE_OFFSET	8

#define WTCON_CLK_16		0x0
#define WTCON_CLK_32		0x1
#define WTCON_CLK_64		0x2
#define WTCON_CLK_128		0x3

#define WTCON_CLK(x)		((x & 0x3) << WTCON_CLKSEL_OFFSET)
#define WTCON_PRESCALER(x)	((x) << WTCON_PRE_OFFSET)
#define WTCON_EN		(0x1 << WTCON_EN_OFFSET)
#define WTCON_RESET		(0x1 << WTCON_RESET_OFFSET)
#define WTCON_INT		(0x1 << WTCON_INTEN_OFFSET)

#ifndef __ASSEMBLY__
struct s5p_watchdog {
	unsigned int wtcon;
	unsigned int wtdat;
	unsigned int wtcnt;
	unsigned int wtclrint;
};

/* functions */
void wdt_stop(void);
void wdt_start(unsigned int timeout);
#endif	/* __ASSEMBLY__ */

#endif
