/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2019 SiFive, Inc.
 */

#ifndef _GPIO_SIFIVE_H
#define _GPIO_SIFIVE_H

#define GPIO_INPUT_VAL	0x00
#define GPIO_INPUT_EN	0x04
#define GPIO_OUTPUT_EN	0x08
#define GPIO_OUTPUT_VAL	0x0C
#define GPIO_RISE_IE	0x18
#define GPIO_RISE_IP	0x1C
#define GPIO_FALL_IE	0x20
#define GPIO_FALL_IP	0x24
#define GPIO_HIGH_IE	0x28
#define GPIO_HIGH_IP	0x2C
#define GPIO_LOW_IE	0x30
#define GPIO_LOW_IP	0x34
#define GPIO_OUTPUT_XOR	0x40

#define NR_GPIOS	16

enum gpio_state {
	LOW,
	HIGH
};

/* Details about a GPIO bank */
struct sifive_gpio_plat {
	void *base;     /* address of registers in physical memory */
};

#define SIFIVE_GENERIC_GPIO_NR(port, index) \
		(((port) * NR_GPIOS) + ((index) & (NR_GPIOS - 1)))

#endif /* _GPIO_SIFIVE_H */
