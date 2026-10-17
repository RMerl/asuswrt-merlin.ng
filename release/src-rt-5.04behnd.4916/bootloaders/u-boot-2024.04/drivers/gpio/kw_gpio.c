// SPDX-License-Identifier: GPL-2.0+
/*
 * arch/arm/plat-orion/gpio.c
 *
 * Marvell Orion SoC GPIO handling.
 */

/*
 * Based on (mostly copied from) plat-orion based Linux 2.6 kernel driver.
 * Removed orion_gpiochip struct and kernel level irq handling.
 *
 * Dieter Kiermaier dk-arm-linux@gmx.de
 */

#include <common.h>
#include <linux/bitops.h>
#include <asm/io.h>
#include <asm/arch/soc.h>
#include <asm/arch/gpio.h>

static unsigned long gpio_valid_input[BITS_TO_LONGS(GPIO_MAX)];
static unsigned long gpio_valid_output[BITS_TO_LONGS(GPIO_MAX)];

void __set_direction(unsigned pin, int input)
{
	u32 u;

	u = readl(GPIO_IO_CONF(pin));
	if (input)
		u |= 1 << (pin & 31);
	else
		u &= ~(1 << (pin & 31));
	writel(u, GPIO_IO_CONF(pin));

	u = readl(GPIO_IO_CONF(pin));
}

static void __set_level(unsigned pin, int high)
{
	u32 u;

	u = readl(GPIO_OUT(pin));
	if (high)
		u |= 1 << (pin & 31);
	else
		u &= ~(1 << (pin & 31));
	writel(u, GPIO_OUT(pin));
}

static void __set_blinking(unsigned pin, int blink)
{
	u32 u;

	u = readl(GPIO_BLINK_EN(pin));
	if (blink)
		u |= 1 << (pin & 31);
	else
		u &= ~(1 << (pin & 31));
	writel(u, GPIO_BLINK_EN(pin));
}

int kw_gpio_is_valid(unsigned pin, int mode)
{
	if (pin < GPIO_MAX) {
		if ((mode & GPIO_INPUT_OK) && !test_bit(pin, gpio_valid_input))
			goto err_out;

		if ((mode & GPIO_OUTPUT_OK) && !test_bit(pin, gpio_valid_output))
			goto err_out;
		return 0;
	}

err_out:
		printf("%s: invalid GPIO %d\n", __func__, pin);
	return 1;
}

void kw_gpio_set_valid(unsigned pin, int mode)
{
	if (mode == 1)
		mode = GPIO_INPUT_OK | GPIO_OUTPUT_OK;
	if (mode & GPIO_INPUT_OK)
		__set_bit(pin, gpio_valid_input);
	else
		__clear_bit(pin, gpio_valid_input);
	if (mode & GPIO_OUTPUT_OK)
		__set_bit(pin, gpio_valid_output);
	else
		__clear_bit(pin, gpio_valid_output);
}
/*
 * GENERIC_GPIO primitives.
 */
int kw_gpio_direction_input(unsigned pin)
{
	if (kw_gpio_is_valid(pin, GPIO_INPUT_OK) != 0)
		return 1;

	/* Configure GPIO direction. */
	__set_direction(pin, 1);

	return 0;
}

int kw_gpio_direction_output(unsigned pin, int value)
{
	if (kw_gpio_is_valid(pin, GPIO_OUTPUT_OK) != 0)
	{
		printf("%s: invalid GPIO %d\n", __func__, pin);
		return 1;
	}

	__set_blinking(pin, 0);

	/* Configure GPIO output value. */
	__set_level(pin, value);

	/* Configure GPIO direction. */
	__set_direction(pin, 0);

	return 0;
}

int kw_gpio_get_value(unsigned pin)
{
	int val;

	if (readl(GPIO_IO_CONF(pin)) & (1 << (pin & 31)))
		val = readl(GPIO_DATA_IN(pin)) ^ readl(GPIO_IN_POL(pin));
	else
		val = readl(GPIO_OUT(pin));

	return (val >> (pin & 31)) & 1;
}

void kw_gpio_set_value(unsigned pin, int value)
{
	/* Configure GPIO output value. */
	__set_level(pin, value);
}

void kw_gpio_set_blink(unsigned pin, int blink)
{
	/* Set output value to zero. */
	__set_level(pin, 0);

	/* Set blinking. */
	__set_blinking(pin, blink);
}
