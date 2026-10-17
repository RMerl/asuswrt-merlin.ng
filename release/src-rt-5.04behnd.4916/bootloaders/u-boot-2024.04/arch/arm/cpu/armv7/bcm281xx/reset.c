// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2013 Broadcom Corporation.
 */

#include <common.h>
#include <cpu_func.h>
#include <asm/io.h>
#include <asm/arch/sysmap.h>

#define EN_MASK		0x08000000	/* Enable timer */
#define SRSTEN_MASK	0x04000000	/* Enable soft reset */
#define CLKS_SHIFT	20		/* Clock period shift */
#define LD_SHIFT	0		/* Reload value shift */

void reset_cpu(void)
{
	/*
	 * Set WD enable, RST enable,
	 * 3.9 msec clock period (8), reload value (8*3.9ms)
	 */
	u32 reg = EN_MASK + SRSTEN_MASK + (8 << CLKS_SHIFT) + (8 << LD_SHIFT);
	writel(reg, SECWD2_BASE_ADDR);

	while (1)
		;	/* loop forever till reset */
}
