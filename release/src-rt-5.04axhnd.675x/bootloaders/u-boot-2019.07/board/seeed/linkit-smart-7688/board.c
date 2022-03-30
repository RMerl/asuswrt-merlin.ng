// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Stefan Roese <sr@denx.de>
 */

#include <common.h>
#include <asm/io.h>

#define MT76XX_GPIO1_MODE	0x10000060

void board_debug_uart_init(void)
{
	void __iomem *gpio_mode;

	/* Select UART2 mode instead of GPIO mode (default) */
	gpio_mode = ioremap_nocache(MT76XX_GPIO1_MODE, 0x100);
	clrbits_le32(gpio_mode, GENMASK(27, 26));
}

int board_early_init_f(void)
{
	/*
	 * The pin muxing of UART2 also needs to be done, if debug uart
	 * is not enabled. So we need to call this function here as well.
	 */
	board_debug_uart_init();

	return 0;
}
