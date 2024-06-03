/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015 Linaro
 * Peter Griffin <peter.griffin@linaro.org>
 */

#ifndef _HI6220_GPIO_H_
#define _HI6220_GPIO_H_

#define HI6220_GPIO_BASE(bank)	(((bank < 4) ? 0xf8011000 : \
				0xf7020000 - 0x4000) + (0x1000 * bank))

#define HI6220_GPIO_PER_BANK	8
#define HI6220_GPIO_DIR		0x400

struct gpio_bank {
	u8 *base;	/* address of registers in physical memory */
};

/* Information about a GPIO bank */
struct hikey_gpio_platdata {
	int bank_index;
	ulong base;     /* address of registers in physical memory */
};

#endif /* _HI6220_GPIO_H_ */
