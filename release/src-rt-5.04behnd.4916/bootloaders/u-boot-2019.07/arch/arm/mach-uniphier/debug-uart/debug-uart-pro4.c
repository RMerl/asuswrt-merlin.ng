// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include <config.h>
#include <linux/kernel.h>
#include <linux/io.h>

#include "../sc-regs.h"
#include "../sg-regs.h"
#include "debug-uart.h"

#define UNIPHIER_PRO4_UART_CLK		73728000

unsigned int uniphier_pro4_debug_uart_init(void)
{
	u32 tmp;

	sg_set_iectrl(0);
	sg_set_pinsel(128, 0, 4, 8);	/* TXD0 -> TXD0 */

	writel(1, SG_LOADPINCTRL);

	tmp = readl(SC_CLKCTRL);
	tmp |= SC_CLKCTRL_CEN_PERI;
	writel(tmp, SC_CLKCTRL);

	return DIV_ROUND_CLOSEST(UNIPHIER_PRO4_UART_CLK, 16 * CONFIG_BAUDRATE);
}
