/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * omap.h
 *
 * Copyright (C) 2011, Texas Instruments, Incorporated - http://www.ti.com/
 *
 * Author:
 *	Chandan Nath <chandan.nath@ti.com>
 *
 * Derived from OMAP4 work by
 *	Aneesh V <aneesh@ti.com>
 */

#ifndef _OMAP_H_
#define _OMAP_H_

#include <linux/sizes.h>

#ifdef CONFIG_AM33XX
#define NON_SECURE_SRAM_START	0x402F0400
#define NON_SECURE_SRAM_END	0x40310000
#define NON_SECURE_SRAM_IMG_END	0x4030B800
#elif defined(CONFIG_TI816X) || defined(CONFIG_TI814X)
#define NON_SECURE_SRAM_START	0x40300000
#define NON_SECURE_SRAM_END	0x40320000
#define NON_SECURE_SRAM_IMG_END	0x4031B800
#elif defined(CONFIG_AM43XX)
#define NON_SECURE_SRAM_START	0x402F0400
#define NON_SECURE_SRAM_END	0x40340000
#define NON_SECURE_SRAM_IMG_END	0x40337DE0
#define QSPI_BASE              0x47900000
#endif
#define SRAM_SCRATCH_SPACE_ADDR	(NON_SECURE_SRAM_IMG_END - SZ_1K)

/* Boot parameters */
#ifndef __ASSEMBLY__
struct omap_boot_parameters {
	unsigned int reserved;
	unsigned int boot_device_descriptor;
	unsigned char boot_device;
	unsigned char reset_reason;
};

#define DEVICE_TYPE_SHIFT		0x8
#define DEVICE_TYPE_MASK		(0x7 << DEVICE_TYPE_SHIFT)
#endif

#endif
