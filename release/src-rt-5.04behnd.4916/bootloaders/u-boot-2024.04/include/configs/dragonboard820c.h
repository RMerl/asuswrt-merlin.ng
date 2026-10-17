/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Board configuration file for Dragonboard 410C
 *
 * (C) Copyright 2017 Jorge Ramirez-Ortiz <jorge.ramirez-ortiz@linaro.org>
 */

#ifndef __CONFIGS_DRAGONBOARD820C_H
#define __CONFIGS_DRAGONBOARD820C_H

#include <linux/sizes.h>

/* Physical Memory Map */

#define PHYS_SDRAM_SIZE			0xC0000000
#define PHYS_SDRAM_1			0x80000000
#define PHYS_SDRAM_1_SIZE		0x60000000
#define PHYS_SDRAM_2			0x100000000
#define PHYS_SDRAM_2_SIZE		0x5ea4ffff

#define CFG_SYS_SDRAM_BASE		PHYS_SDRAM_1

#include <config_distro_bootcmd.h>

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0)

#define CFG_EXTRA_ENV_SETTINGS \
	"loadaddr=0x95000000\0" \
	"fdt_high=0xffffffffffffffff\0" \
	"initrd_high=0xffffffffffffffff\0" \
	"linux_image=uImage\0" \
	"kernel_addr_r=0x95000000\0"\
	"fdtfile=qcom/apq8096-db820c.dtb\0" \
	"fdt_addr_r=0x93000000\0"\
	"ramdisk_addr_r=0x91000000\0"\
	"scriptaddr=0x90000000\0"\
	"pxefile_addr_r=0x90100000\0"\
	BOOTENV

#endif
