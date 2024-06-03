// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian@popies.net>
 * Lead Tech Design <www.leadtechdesign.com>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/at91sam9g45.h>
#include <asm/arch/clk.h>
#include <asm/arch/gpio.h>

void coloured_LED_init(void)
{
	at91_periph_clk_enable(ATMEL_ID_PIODE);

	at91_set_gpio_output(CONFIG_RED_LED, 1);
	at91_set_gpio_output(CONFIG_GREEN_LED, 1);

	at91_set_gpio_value(CONFIG_RED_LED, 0);
	at91_set_gpio_value(CONFIG_GREEN_LED, 1);
}
