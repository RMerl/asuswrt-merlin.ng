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

#define UNIPHIER_PXS2_UART_CLK		88888888

unsigned int uniphier_pxs2_debug_uart_init(void)
{
	u32 tmp;

	sg_set_iectrl(0);
	sg_set_pinsel(217, 8, 8, 4);	/* TXD0 -> TXD0 */
	sg_set_pinsel(115, 8, 8, 4);	/* TXD1 -> TXD1 */
	sg_set_pinsel(113, 8, 8, 4);	/* TXD2 -> TXD2 */
	sg_set_pinsel(219, 8, 8, 4);	/* TXD3 -> TXD3 */

	tmp = readl(SC_CLKCTRL);
	tmp |= SC_CLKCTRL_CEN_PERI;
	writel(tmp, SC_CLKCTRL);

	return DIV_ROUND_CLOSEST(UNIPHIER_PXS2_UART_CLK, 16 * CONFIG_BAUDRATE);
}
