/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2012 Samsung Electronics
 * Abhilash Kesavan <a.kesavan@samsung.com>
 */

#ifndef __ASM_ARM_ARCH_PINMUX_H
#define __ASM_ARM_ARCH_PINMUX_H

#include "periph.h"

/*
 * Flags for setting specific configarations of peripherals.
 * List will grow with support for more devices getting added.
 */
enum {
	PINMUX_FLAG_NONE	= 0x00000000,

	/* Flags for eMMC */
	PINMUX_FLAG_8BIT_MODE	= 1 << 0,       /* SDMMC 8-bit mode */

	/* Flags for SROM controller */
	PINMUX_FLAG_BANK	= 3 << 0,       /* bank number (0-3) */
	PINMUX_FLAG_16BIT	= 1 << 2,       /* 16-bit width */
};

/**
 * Configures the pinmux for a particular peripheral.
 *
 * Each gpio can be configured in many different ways (4 bits on exynos)
 * such as "input", "output", "special function", "external interrupt"
 * etc. This function will configure the peripheral pinmux along with
 * pull-up/down and drive strength.
 *
 * @param peripheral	peripheral to be configured
 * @param flags		configure flags
 * Return: 0 if ok, -1 on error (e.g. unsupported peripheral)
 */
int exynos_pinmux_config(int peripheral, int flags);

/**
 * Decode the peripheral id using the interrpt numbers.
 *
 * @param blob  Device tree blob
 * @param node  FDT I2C node to find
 * Return: peripheral id if ok, PERIPH_ID_NONE on error
 */
int pinmux_decode_periph_id(const void *blob, int node);
#endif
