/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2013 Samsung Electronics
 *
 * Configuration settings for the SAMSUNG EXYNOS5 board.
 */

#ifndef __CONFIG_EXYNOS5_COMMON_H
#define __CONFIG_EXYNOS5_COMMON_H

#include "exynos-common.h"

/* Power Down Modes */
#define S5P_CHECK_SLEEP			0x00000BAD
#define S5P_CHECK_DIDLE			0xBAD00000
#define S5P_CHECK_LPA			0xABAD0000

/* Offset for inform registers */
#define INFORM0_OFFSET			0x800
#define INFORM1_OFFSET			0x804
#define INFORM2_OFFSET			0x808
#define INFORM3_OFFSET			0x80c

/* select serial console configuration */
#define EXYNOS5_DEFAULT_UART_OFFSET	0x010000

/* MMC SPL */
#define COPY_BL2_FNPTR_ADDR	0x02020030

#define PHYS_SDRAM_1		CFG_SYS_SDRAM_BASE
#define PHYS_SDRAM_1_SIZE	SDRAM_BANK_SIZE
#define PHYS_SDRAM_2		(CFG_SYS_SDRAM_BASE + SDRAM_BANK_SIZE)
#define PHYS_SDRAM_2_SIZE	SDRAM_BANK_SIZE
#define PHYS_SDRAM_3		(CFG_SYS_SDRAM_BASE + (2 * SDRAM_BANK_SIZE))
#define PHYS_SDRAM_3_SIZE	SDRAM_BANK_SIZE
#define PHYS_SDRAM_4		(CFG_SYS_SDRAM_BASE + (3 * SDRAM_BANK_SIZE))
#define PHYS_SDRAM_4_SIZE	SDRAM_BANK_SIZE
#define PHYS_SDRAM_5		(CFG_SYS_SDRAM_BASE + (4 * SDRAM_BANK_SIZE))
#define PHYS_SDRAM_5_SIZE	SDRAM_BANK_SIZE
#define PHYS_SDRAM_6		(CFG_SYS_SDRAM_BASE + (5 * SDRAM_BANK_SIZE))
#define PHYS_SDRAM_6_SIZE	SDRAM_BANK_SIZE
#define PHYS_SDRAM_7		(CFG_SYS_SDRAM_BASE + (6 * SDRAM_BANK_SIZE))
#define PHYS_SDRAM_7_SIZE	SDRAM_BANK_SIZE
#define PHYS_SDRAM_8		(CFG_SYS_SDRAM_BASE + (7 * SDRAM_BANK_SIZE))
#define PHYS_SDRAM_8_SIZE	SDRAM_BANK_SIZE

/* SPI */

/* Ethernet Controllor Driver */
#ifdef CONFIG_CMD_NET
#define CFG_ENV_SROM_BANK		1
#endif /*CONFIG_CMD_NET*/

/* Enable Time Command */

/* USB */

/* USB boot mode */
#define EXYNOS_COPY_USB_FNPTR_ADDR	0x02020070
#define EXYNOS_USB_SECONDARY_BOOT	0xfeed0002
#define EXYNOS_IRAM_SECONDARY_BASE	0x02020018

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 2) \
	func(MMC, mmc, 1) \
	func(MMC, mmc, 0) \
	func(PXE, pxe, na) \
	func(DHCP, dhcp, na)

#include <config_distro_bootcmd.h>

#ifndef MEM_LAYOUT_ENV_SETTINGS
/* 2GB RAM, bootm size of 256M, load scripts after that */
#define MEM_LAYOUT_ENV_SETTINGS \
	"bootm_size=0x10000000\0" \
	"kernel_addr_r=0x42000000\0" \
	"fdt_addr_r=0x43000000\0" \
	"ramdisk_addr_r=0x43300000\0" \
	"scriptaddr=0x50000000\0" \
	"pxefile_addr_r=0x51000000\0"
#endif

#ifndef EXYNOS_DEVICE_SETTINGS
#define EXYNOS_DEVICE_SETTINGS \
	"stdin=serial\0" \
	"stdout=serial\0" \
	"stderr=serial\0"
#endif

#ifndef EXYNOS_FDTFILE_SETTING
#define EXYNOS_FDTFILE_SETTING
#endif

#define CFG_EXTRA_ENV_SETTINGS \
	EXYNOS_DEVICE_SETTINGS \
	EXYNOS_FDTFILE_SETTING \
	MEM_LAYOUT_ENV_SETTINGS \
	BOOTENV

#endif	/* __CONFIG_EXYNOS5_COMMON_H */
