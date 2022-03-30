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

#define UNIPHIER_LD6B_UART_CLK		88888888

unsigned int uniphier_ld6b_debug_uart_init(void)
{
	u32 tmp;

	sg_set_iectrl(0);
	sg_set_pinsel(135, 3, 8, 4);	/* PORT10 -> TXD0 */
	sg_set_pinsel(115, 0, 8, 4);	/* TXD1 -> TXD1 */
	sg_set_pinsel(113, 2, 8, 4);	/* SBO0 -> TXD2 */

	tmp = readl(SC_CLKCTRL);
	tmp |= SC_CLKCTRL_CEN_PERI;
	writel(tmp, SC_CLKCTRL);

	return DIV_ROUND_CLOSEST(UNIPHIER_LD6B_UART_CLK, 16 * CONFIG_BAUDRATE);
}
