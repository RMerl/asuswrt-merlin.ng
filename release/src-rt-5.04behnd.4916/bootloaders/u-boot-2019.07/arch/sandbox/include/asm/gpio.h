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

/**
 * Return the simulated value of a GPIO (used only in sandbox test code)
 *
 * @param dev		device to use
 * @param offset	GPIO offset within bank
 * @return -1 on error, 0 if GPIO is low, >0 if high
 */
int sandbox_gpio_get_value(struct udevice *dev, unsigned int offset);

/**
 * Set the simulated value of a GPIO (used only in sandbox test code)
 *
 * @param dev		device to use
 * @param offset	GPIO offset within bank
 * @param value		value to set (0 for low, non-zero for high)
 * @return -1 on error, 0 if ok
 */
int sandbox_gpio_set_value(struct udevice *dev, unsigned int offset, int value);

/**
 * Set or reset the simulated open drain mode of a GPIO (used only in sandbox
 * test code)
 *
 * @param gp	GPIO number
 * @param value	value to set (0 for enabled open drain mode, non-zero for
 * 		disabled)
 * @return -1 on error, 0 if ok
 */
int sandbox_gpio_set_open_drain(struct udevice *dev, unsigned offset, int value);

/**
 * Return the state of the simulated open drain mode of a GPIO (used only in
 * sandbox test code)
 *
 * @param gp	GPIO number
 * @return -1 on error, 0 if GPIO is input, >0 if output
 */
int sandbox_gpio_get_open_drain(struct udevice *dev, unsigned offset);

/**
 * Return the simulated direction of a GPIO (used only in sandbox test code)
 *
 * @param dev		device to use
 * @param offset	GPIO offset within bank
 * @return -1 on error, 0 if GPIO is input, >0 if output
 */
int sandbox_gpio_get_direction(struct udevice *dev, unsigned int offset);

/**
 * Set the simulated direction of a GPIO (used only in sandbox test code)
 *
 * @param dev		device to use
 * @param offset	GPIO offset within bank
 * @param output 	0 to set as input, 1 to set as output
 * @return -1 on error, 0 if ok
 */
int sandbox_gpio_set_direction(struct udevice *dev, unsigned int offset,
			       int output);

#endif
