/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2015 Linaro
 *
 * Peter Griffin <peter.griffin@linaro.org>
 *
 * Configuration for HiKey 96boards CE. Parts were derived from other ARM
 * configurations.
 */

#ifndef __HIKEY_H
#define __HIKEY_H

#include <linux/sizes.h>

/* Physical Memory Map */

/* CONFIG_TEXT_BASE needs to align with where ATF loads bl33.bin */

#define PHYS_SDRAM_1			0x00000000

/* 1008 MB (the last 16Mb are secured for TrustZone by ATF*/
#define PHYS_SDRAM_1_SIZE		0x3EFFFFFF

#define CFG_SYS_SDRAM_BASE		PHYS_SDRAM_1

#define CFG_SYS_INIT_RAM_SIZE	0x1000

/* Generic Interrupt Controller Definitions */
#define GICD_BASE			0xf6801000
#define GICC_BASE			0xf6802000

/* Initial environment variables */

/*
 * Defines where the kernel and FDT will be put in RAM
 */

#define BOOT_TARGET_DEVICES(func) \
	func(USB, usb, 0) \
	func(MMC, mmc, 1) \
	func(DHCP, dhcp, na)
#include <config_distro_bootcmd.h>

#define CFG_EXTRA_ENV_SETTINGS	\
				"kernel_name=Image\0"	\
				"kernel_addr_r=0x00080000\0" \
				"fdtfile=hi6220-hikey.dtb\0" \
				"fdt_addr_r=0x02000000\0" \
				"fdt_high=0xffffffffffffffff\0" \
				"initrd_high=0xffffffffffffffff\0" \
				BOOTENV

/* Preserve environment on eMMC */

#endif /* __HIKEY_H */
