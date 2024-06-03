/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2018 NXP
 */

#ifndef __ASM_ARCH_IMX8_GPIO_H
#define __ASM_ARCH_IMX8_GPIO_H

#if !(defined(__KERNEL_STRICT_NAMES) || defined(__ASSEMBLY__))
/* GPIO registers */
struct gpio_regs {
	u32 gpio_dr;	/* data */
	u32 gpio_dir;	/* direction */
	u32 gpio_psr;	/* pad satus */
};
#endif

/* IMX8 the GPIO index is from 0 not 1 */
#define IMX_GPIO_NR(port, index)		(((port) * 32) + ((index) & 31))

#endif /* __ASM_ARCH_IMX8_GPIO_H */
