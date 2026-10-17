// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2010 Texas Instruments, Inc.
 * Jason Kridner <jkridner@beagleboard.org>
 */
#include <common.h>
#include <status_led.h>
#include <asm/arch/cpu.h>
#include <asm/io.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>

/* GPIO pins for the LEDs */
#define BEAGLE_LED_USR0	150
#define BEAGLE_LED_USR1	149

#ifdef CONFIG_LED_STATUS_GREEN
void green_led_off(void)
{
	__led_set(CONFIG_LED_STATUS_GREEN, 0);
}

void green_led_on(void)
{
	__led_set(CONFIG_LED_STATUS_GREEN, 1);
}
#endif

static int get_led_gpio(led_id_t mask)
{
#ifdef CONFIG_LED_STATUS0
	if (CONFIG_LED_STATUS_BIT & mask)
		return BEAGLE_LED_USR0;
#endif
#ifdef CONFIG_LED_STATUS1
	if (CONFIG_LED_STATUS_BIT1 & mask)
		return BEAGLE_LED_USR1;
#endif

	return 0;
}

void __led_init (led_id_t mask, int state)
{
	int toggle_gpio;

	toggle_gpio = get_led_gpio(mask);

	if (toggle_gpio && !gpio_request(toggle_gpio, "led"))
		__led_set(mask, state);
}

void __led_toggle (led_id_t mask)
{
	int state, toggle_gpio;

	toggle_gpio = get_led_gpio(mask);
	if (toggle_gpio) {
		state = gpio_get_value(toggle_gpio);
		gpio_direction_output(toggle_gpio, !state);
	}
}

void __led_set (led_id_t mask, int state)
{
	int toggle_gpio;

	toggle_gpio = get_led_gpio(mask);
	if (toggle_gpio)
		gpio_direction_output(toggle_gpio, state);
}
