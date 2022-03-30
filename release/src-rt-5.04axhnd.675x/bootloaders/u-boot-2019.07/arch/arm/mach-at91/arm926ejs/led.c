// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian@popies.net>
 * Lead Tech Design <www.leadtechdesign.com>
 */

#include <common.h>
#include <asm/gpio.h>
#include <asm/arch/gpio.h>
#include <status_led.h>

#ifdef CONFIG_RED_LED
void red_led_on(void)
{
	gpio_set_value(CONFIG_RED_LED, 1);
}

void red_led_off(void)
{
	gpio_set_value(CONFIG_RED_LED, 0);
}
#endif

#ifdef CONFIG_GREEN_LED
void green_led_on(void)
{
	gpio_set_value(CONFIG_GREEN_LED, 0);
}

void green_led_off(void)
{
	gpio_set_value(CONFIG_GREEN_LED, 1);
}
#endif

#ifdef CONFIG_YELLOW_LED
void yellow_led_on(void)
{
	gpio_set_value(CONFIG_YELLOW_LED, 0);
}

void yellow_led_off(void)
{
	gpio_set_value(CONFIG_YELLOW_LED, 1);
}
#endif
