// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian@popies.net>
 * Lead Tech Design <www.leadtechdesign.com>
 * Ilko Iliev <www.ronetix.at>
 */

#include <common.h>
#include <asm/gpio.h>
#include <asm/arch/clk.h>
#include <asm/arch/gpio.h>

void coloured_LED_init(void)
{
	at91_periph_clk_enable(ATMEL_ID_PIOC);

	gpio_direction_output(CONFIG_RED_LED, 1);
	gpio_direction_output(CONFIG_GREEN_LED, 1);
	gpio_direction_output(CONFIG_YELLOW_LED, 1);

	gpio_set_value(CONFIG_RED_LED, 0);
	gpio_set_value(CONFIG_GREEN_LED, 1);
	gpio_set_value(CONFIG_YELLOW_LED, 1);
}
