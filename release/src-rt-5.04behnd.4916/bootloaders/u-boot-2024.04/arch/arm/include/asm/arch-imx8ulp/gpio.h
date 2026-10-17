/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2020 NXP
 */

#ifndef __ASM_ARCH_IMX8ULP_GPIO_H
#define __ASM_ARCH_IMX8ULP_GPIO_H

struct gpio_regs {
	u32 gpio_pdor;
	u32 gpio_psor;
	u32 gpio_pcor;
	u32 gpio_ptor;
	u32 gpio_pdir;
	u32 gpio_pddr;
	u32 gpio_pidr;
	u8 gpio_pxdr[32];
};

#endif
