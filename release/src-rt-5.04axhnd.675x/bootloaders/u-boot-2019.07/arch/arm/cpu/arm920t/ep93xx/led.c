// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2010, 2009 Matthias Kaehlcke <matthias@kaehlcke.net>
 */

#include <asm/io.h>
#include <asm/arch/ep93xx.h>
#include <config.h>
#include <status_led.h>

static uint8_t saved_state[2] = {CONFIG_LED_STATUS_OFF, CONFIG_LED_STATUS_OFF};
static uint32_t gpio_pin[2] = {1 << CONFIG_LED_STATUS_GREEN,
			       1 << CONFIG_LED_STATUS_RED};

static inline void switch_LED_on(uint8_t led)
{
	register struct gpio_regs *gpio = (struct gpio_regs *)GPIO_BASE;

	writel(readl(&gpio->pedr) | gpio_pin[led], &gpio->pedr);
	saved_state[led] = CONFIG_LED_STATUS_ON;
}

static inline void switch_LED_off(uint8_t led)
{
	register struct gpio_regs *gpio = (struct gpio_regs *)GPIO_BASE;

	writel(readl(&gpio->pedr) & ~gpio_pin[led], &gpio->pedr);
	saved_state[led] = CONFIG_LED_STATUS_OFF;
}

void red_led_on(void)
{
	switch_LED_on(CONFIG_LED_STATUS_RED);
}

void red_led_off(void)
{
	switch_LED_off(CONFIG_LED_STATUS_RED);
}

void green_led_on(void)
{
	switch_LED_on(CONFIG_LED_STATUS_GREEN);
}

void green_led_off(void)
{
	switch_LED_off(CONFIG_LED_STATUS_GREEN);
}

void __led_init(led_id_t mask, int state)
{
	__led_set(mask, state);
}

void __led_toggle(led_id_t mask)
{
	if (CONFIG_LED_STATUS_RED == mask) {
		if (CONFIG_LED_STATUS_ON == saved_state[CONFIG_LED_STATUS_RED])
			red_led_off();
		else
			red_led_on();
	} else if (CONFIG_LED_STATUS_GREEN == mask) {
		if (CONFIG_LED_STATUS_ON ==
		    saved_state[CONFIG_LED_STATUS_GREEN])
			green_led_off();
		else
			green_led_on();
	}
}

void __led_set(led_id_t mask, int state)
{
	if (CONFIG_LED_STATUS_RED == mask) {
		if (CONFIG_LED_STATUS_ON == state)
			red_led_on();
		else
			red_led_off();
	} else if (CONFIG_LED_STATUS_GREEN == mask) {
		if (CONFIG_LED_STATUS_ON == state)
			green_led_on();
		else
			green_led_off();
	}
}
