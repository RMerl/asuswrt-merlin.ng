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

#ifndef __ASM_ARCH_MX85XX_GPIO_H
#define __ASM_ARCH_MX85XX_GPIO_H

#ifndef CONFIG_MPC85XX_GPIO
#include <asm/mpc85xx_gpio.h>
#endif

struct mpc8xxx_gpio_plat {
	phys_addr_t addr;
	unsigned long size;
	uint ngpios;
};

#endif
