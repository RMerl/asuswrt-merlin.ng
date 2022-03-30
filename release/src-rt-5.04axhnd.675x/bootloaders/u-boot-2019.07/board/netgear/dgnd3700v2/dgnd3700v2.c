// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Ãlvaro FernÃ¡ndez Rojas <noltari@gmail.com>
 */

#include <common.h>
#include <asm/io.h>

#define GPIO_BASE_6362			0x10000080

#define GPIO_MODE_6362_REG		0x18
#define GPIO_MODE_6362_SERIAL_LED_DATA	BIT(2)
#define GPIO_MODE_6362_SERIAL_LED_CLK	BIT(3)

#ifdef CONFIG_BOARD_EARLY_INIT_F
int board_early_init_f(void)
{
	void __iomem *gpio_regs = map_physmem(GPIO_BASE_6362, 0, MAP_NOCACHE);

	/* Enable Serial LEDs */
	setbits_be32(gpio_regs + GPIO_MODE_6362_REG,
		     GPIO_MODE_6362_SERIAL_LED_DATA |
		     GPIO_MODE_6362_SERIAL_LED_CLK);

	return 0;
}
#endif
