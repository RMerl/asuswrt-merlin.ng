/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 Freescale Semiconductor, Inc.
 */

#ifndef __ASM_ARCH_MX7ULP_GPIO_H
#define __ASM_ARCH_MX7ULP_GPIO_H

struct gpio_regs {
	u32 gpio_pdor;
	u32 gpio_psor;
	u32 gpio_pcor;
	u32 gpio_ptor;
	u32 gpio_pdir;
	u32 gpio_pddr;
	u32 gpio_gacr;
};

#define IMX_GPIO_NR(port, index)		((((port)-1)*32)+((index)&31))

#endif /* __ASM_ARCH_MX7ULP_GPIO_H */
