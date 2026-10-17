/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 */

/*
 * Dummy header file to enable CONFIG_OF_CONTROL.
 * If CONFIG_OF_CONTROL is enabled, lib/fdtdec.c is compiled.
 * It includes <asm/arch/gpio.h> via <asm/gpio.h>, so those SoCs that enable
 * OF_CONTROL must have arch/gpio.h.
 */

#ifndef __ASM_ARCH_LS102XA_GPIO_H_
#define __ASM_ARCH_LS102XA_GPIO_H_

struct ccsr_gpio {
	u32	gpdir;
	u32	gpodr;
	u32	gpdat;
	u32	gpier;
	u32	gpimr;
	u32	gpicr;
	u32	gpibe;
};

struct mpc8xxx_gpio_plat {
	ulong addr;
	ulong size;
	uint ngpios;
};

#endif
