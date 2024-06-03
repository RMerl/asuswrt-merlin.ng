/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2011
 * Stefano Babic, DENX Software Engineering, <sbabic@denx.de>
 */


#ifndef __ASM_ARCH_IMX_GPIO_H
#define __ASM_ARCH_IMX_GPIO_H

#if !(defined(__KERNEL_STRICT_NAMES) || defined(__ASSEMBLY__))
/* GPIO registers */
struct gpio_regs {
	u32 gpio_dr;	/* data */
	u32 gpio_dir;	/* direction */
	u32 gpio_psr;	/* pad satus */
};
#endif

#define IMX_GPIO_NR(port, index)		((((port)-1)*32)+((index)&31))

#endif
