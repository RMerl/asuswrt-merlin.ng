// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012-2014 Panasonic Corporation
 * Copyright (C) 2015-2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include <common.h>
#include <linux/io.h>
#include <asm/secure.h>

#include "sc-regs.h"

/* If PSCI is enabled, this is used for SYSTEM_RESET function */
#ifdef CONFIG_ARMV7_PSCI
#define __SECURE	__secure
#else
#define __SECURE
#endif

void __SECURE reset_cpu(unsigned long ignored)
{
	u32 tmp;

	writel(5, SC_IRQTIMSET); /* default value */

	tmp  = readl(SC_SLFRSTSEL);
	tmp &= ~0x3; /* mask [1:0] */
	tmp |= 0x0;  /* XRST reboot */
	writel(tmp, SC_SLFRSTSEL);

	tmp = readl(SC_SLFRSTCTL);
	tmp |= 0x1;
	writel(tmp, SC_SLFRSTCTL);
}
