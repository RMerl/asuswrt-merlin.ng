// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include <config.h>
#include <linux/kernel.h>

#include "../sg-regs.h"
#include "debug-uart.h"

#define UNIPHIER_SLD8_UART_CLK		80000000

unsigned int uniphier_sld8_debug_uart_init(void)
{
	sg_set_iectrl(0);
	sg_set_pinsel(70, 3, 8, 4);	/* HSDOUT6 -> TXD0 */

	return DIV_ROUND_CLOSEST(UNIPHIER_SLD8_UART_CLK, 16 * CONFIG_BAUDRATE);
}
