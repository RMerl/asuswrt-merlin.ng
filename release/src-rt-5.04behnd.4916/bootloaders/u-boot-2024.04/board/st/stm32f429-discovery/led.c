// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015
 * Kamil Lulko, <kamil.lulko@gmail.com>
 */

#include <common.h>
#include <status_led.h>
#include <asm-generic/gpio.h>

#define RED_LED			110
#define GREEN_LED		109

void coloured_LED_init(void)
{
	gpio_request(RED_LED, "red led");
	gpio_direction_output(RED_LED, 0);
	gpio_request(GREEN_LED, "green led");
	gpio_direction_output(GREEN_LED, 0);
}

void red_led_off(void)
{
	gpio_set_value(RED_LED, 0);
}

void green_led_off(void)
{
	gpio_set_value(GREEN_LED, 0);
}

void red_led_on(void)
{
	gpio_set_value(RED_LED, 1);
}

void green_led_on(void)
{
	gpio_set_value(GREEN_LED, 1);
}
