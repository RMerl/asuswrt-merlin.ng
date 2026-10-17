/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * This is the interface to the sandbox GPIO driver for test code which
 * wants to change the GPIO values reported to U-Boot.
 *
 * Copyright (c) 2011 The Chromium OS Authors.
 */

#ifndef __ASM_SANDBOX_GPIO_H
#define __ASM_SANDBOX_GPIO_H

/*
 * We use the generic interface, and add a back-channel.
 *
 * The back-channel functions are declared in this file. They should not be used
 * except in test code.
 *
 * Test code can, for example, call sandbox_gpio_set_value() to set the value of
 * a simulated GPIO. From then on, normal code in U-Boot will see this new
 * value when it calls gpio_get_value().
 *
 * NOTE: DO NOT use the functions in this file except in test code!
 */
#include <asm-generic/gpio.h>

/* Our own private GPIO flags, which musn't conflict with GPIOD_... */
#define GPIOD_EXT_HIGH		BIT(31)	/* external source is high (else low) */
#define GPIOD_EXT_DRIVEN	BIT(30)	/* external source is driven */
#define GPIOD_EXT_PULL_UP	BIT(29)	/* GPIO has external pull-up */
#define GPIOD_EXT_PULL_DOWN	BIT(28)	/* GPIO has external pull-down */

#define GPIOD_EXT_PULL		(BIT(28) | BIT(29))
#define GPIOD_SANDBOX_MASK	GENMASK(31, 28)

/**
 * Return the simulated value of a GPIO (used only in sandbox test code)
 *
 * @param dev		device to use
 * @param offset	GPIO offset within bank
 * Return: -1 on error, 0 if GPIO is low, >0 if high
 */
int sandbox_gpio_get_value(struct udevice *dev, unsigned int offset);

/**
 * Set the simulated value of a GPIO (used only in sandbox test code)
 *
 * @param dev		device to use
 * @param offset	GPIO offset within bank
 * @param value		value to set (0 for low, non-zero for high)
 * Return: -1 on error, 0 if ok
 */
int sandbox_gpio_set_value(struct udevice *dev, unsigned int offset, int value);

/**
 * Return the simulated direction of a GPIO (used only in sandbox test code)
 *
 * @param dev		device to use
 * @param offset	GPIO offset within bank
 * Return: -1 on error, 0 if GPIO is input, >0 if output
 */
int sandbox_gpio_get_direction(struct udevice *dev, unsigned int offset);

/**
 * Set the simulated direction of a GPIO (used only in sandbox test code)
 *
 * @param dev		device to use
 * @param offset	GPIO offset within bank
 * @param output	0 to set as input, 1 to set as output
 * Return: -1 on error, 0 if ok
 */
int sandbox_gpio_set_direction(struct udevice *dev, unsigned int offset,
			       int output);

/**
 * Return the simulated flags of a GPIO (used only in sandbox test code)
 *
 * @param dev		device to use
 * @param offset	GPIO offset within bank
 * Return: dir_flags: bitfield accesses by GPIOD_ defines
 */
ulong sandbox_gpio_get_flags(struct udevice *dev, unsigned int offset);

/**
 * Set the simulated flags of a GPIO (used only in sandbox test code)
 *
 * @param dev		device to use
 * @param offset	GPIO offset within bank
 * @param flags		bitfield accesses by GPIOD_ defines
 * Return: -1 on error, 0 if ok
 */
int sandbox_gpio_set_flags(struct udevice *dev, unsigned int offset,
			   ulong flags);

#endif
