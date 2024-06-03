// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015
 * Kamil Lulko, <kamil.lulko@gmail.com>
 */

#include <common.h>
#include <asm-generic/gpio.h>

void coloured_LED_init(void)
{
	gpio_direction_output(CONFIG_RED_LED, 0);
	gpio_direction_output(CONFIG_GREEN_LED, 0);
}

void red_led_off(void)
{
	gpio_set_value(CONFIG_RED_LED, 0);
}

void green_led_off(void)
{
	gpio_set_value(CONFIG_GREEN_LED, 0);
}

void red_led_on(void)
{
	gpio_set_value(CONFIG_RED_LED, 1);
}

void green_led_on(void)
{
	gpio_set_value(CONFIG_GREEN_LED, 1);
}
