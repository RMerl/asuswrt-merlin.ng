/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2012 Stefan Roese <sr@denx.de>
 */


#ifndef __ASM_ARCH_SPEAR_GPIO_H
#define __ASM_ARCH_SPEAR_GPIO_H

enum gpio_direction {
	GPIO_DIRECTION_IN,
	GPIO_DIRECTION_OUT,
};

struct gpio_regs {
	u32 gpiodata[0x100];	/* 0x000 ... 0x3fc */
	u32 gpiodir;		/* 0x400 */
};

#define SPEAR_GPIO_COUNT		8
#define DATA_REG_ADDR(gpio)		(1 << (gpio + 2))

#endif	/* __ASM_ARCH_SPEAR_GPIO_H */
