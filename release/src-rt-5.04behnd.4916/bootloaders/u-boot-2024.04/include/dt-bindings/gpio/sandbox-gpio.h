/* SPDX-License-Identifier: GPL-2.0 */
/*
 * This header provides constants for binding sandbox,gpio
 *
 */
#ifndef _DT_BINDINGS_GPIO_SANDBOX_GPIO_H
#define _DT_BINDINGS_GPIO_SANDBOX_GPIO_H

/*
 * Add a specific binding for sandbox gpio.
 * The value need to be after the generic defines of
 * dt-bindings/gpio/gpio.h
 */

/* Bit 16 express GPIO input mode */
#define GPIO_IN			0x10000

/* Bit 17 express GPIO output mode */
#define GPIO_OUT		0x20000

/* Bit 18 express GPIO output is active */
#define GPIO_OUT_ACTIVE		0x40000

/* Bit 19 express GPIO set as alternate function */
#define GPIO_AF			0x80000

#endif
