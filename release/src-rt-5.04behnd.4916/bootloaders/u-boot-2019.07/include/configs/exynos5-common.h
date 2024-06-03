/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2013 Samsung Electronics
 *
 * Configuration settings for the SAMSUNG EXYNOS5 board.
 */

#ifndef __CONFIG_EXYNOS5_COMMON_H
#define __CONFIG_EXYNOS5_COMMON_H

#define CONFIG_EXYNOS5			/* Exynos5 Family */

#include "exynos-common.h"

#define CONFIG_EXYNOS_SPL

#ifdef FTRACE
#define CONFIG_TRACE
#define CONFIG_TRACE_BUFFER_SIZE	(16 << 20)
#define CONFIG_TRACE_EARLY_SIZE		(8 << 20)
#define CONFIG_TRACE_EARLY
#define CONFIG_TRACE_EARLY_ADDR		0x50000000
#endif

/* Enable ACE acceleration for SHA1 and SHA256 */
#define CONFIG_EXYNOS_ACE_SHA

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

/* Thermal Management Unit */
#define CONFIG_EXYNOS_TMU

/* MMC SPL */
#define COPY_BL2_FNPTR_ADDR	0x02020030

/* specific .lds file */

/* Boot Argument Buffer Size */
/* memtest works on */
#define CONFIG_SYS_MEMTEST_START	CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_SDRAM_BASE + 0x5E00000)
#define CONFIG_SYS_LOAD_ADDR		(CONFIG_SYS_SDRAM_BASE + 0x3E00000)

#define CONFIG_RD_LVL

#define PHYS_SDRAM_1		CONFIG_SYS_SDRAM_BASE
#define PHYS_SDRAM_1_SIZE	SDRAM_BANK_SIZE
#define PHYS_SDRAM_2		(CONFIG_SYS_SDRAM_BASE + SDRAM_BANK_SIZE)
#define PHYS_SDRAM_2_SIZE	SDRAM_BANK_SIZE
#define PHYS_SDRAM_3		(CONFIG_SYS_SDRAM_BASE + (2 * SDRAM_BANK_SIZE))
#define PHYS_SDRAM_3_SIZE	SDRAM_BANK_SIZE
#define PHYS_SDRAM_4		(CONFIG_SYS_SDRAM_BASE + (3 * SDRAM_BANK_SIZE))
#define PHYS_SDRAM_4_SIZE	SDRAM_BANK_SIZE
#define PHYS_SDRAM_5		(CONFIG_SYS_SDRAM_BASE + (4 * SDRAM_BANK_SIZE))
#define PHYS_SDRAM_5_SIZE	SDRAM_BANK_SIZE
#define PHYS_SDRAM_6		(CONFIG_SYS_SDRAM_BASE + (5 * SDRAM_BANK_SIZE))
#define PHYS_SDRAM_6_SIZE	SDRAM_BANK_SIZE
#define PHYS_SDRAM_7		(CONFIG_SYS_SDRAM_BASE + (6 * SDRAM_BANK_SIZE))
#define PHYS_SDRAM_7_SIZE	SDRAM_BANK_SIZE
#define PHYS_SDRAM_8		(CONFIG_SYS_SDRAM_BASE + (7 * SDRAM_BANK_SIZE))
#define PHYS_SDRAM_8_SIZE	SDRAM_BANK_SIZE

#define CONFIG_SYS_MONITOR_BASE	0x00000000

#define CONFIG_SYS_MMC_ENV_DEV		0

#define CONFIG_SECURE_BL1_ONLY

/* Secure FW size configuration */
#ifdef CONFIG_SECURE_BL1_ONLY
#define CONFIG_SEC_FW_SIZE (8 << 10) /* 8KB */
#else
#define CONFIG_SEC_FW_SIZE 0
#endif

/* Configuration of BL1, BL2, ENV Blocks on mmc */
#define CONFIG_RES_BLOCK_SIZE	(512)
#define CONFIG_BL1_SIZE	(16 << 10) /*16 K reserved for BL1*/
#define CONFIG_BL2_SIZE	(512UL << 10UL) /* 512 KB */
#define CONFIG_ENV_SIZE	(16 << 10) /* 16 KB */

#define CONFIG_BL1_OFFSET	(CONFIG_RES_BLOCK_SIZE + CONFIG_SEC_FW_SIZE)
#define CONFIG_BL2_OFFSET	(CONFIG_BL1_OFFSET + CONFIG_BL1_SIZE)

/* U-Boot copy size from boot Media to DRAM.*/
#define BL2_START_OFFSET	(CONFIG_BL2_OFFSET/512)
#define BL2_SIZE_BLOC_COUNT	(CONFIG_BL2_SIZE/512)

#define EXYNOS_COPY_SPI_FNPTR_ADDR	0x02020058
#define SPI_FLASH_UBOOT_POS	(CONFIG_SEC_FW_SIZE + CONFIG_BL1_SIZE)

/* I2C */
#define CONFIG_SYS_I2C_S3C24X0
#define CONFIG_SYS_I2C_S3C24X0_SPEED	100000		/* 100 Kbps */
#define CONFIG_SYS_I2C_S3C24X0_SLAVE    0x0

/* SPI */

#ifdef CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_SECT_SIZE	CONFIG_ENV_SIZE
#endif

/* Ethernet Controllor Driver */
#ifdef CONFIG_CMD_NET
#define CONFIG_ENV_SROM_BANK		1
#endif /*CONFIG_CMD_NET*/

/* Enable Time Command */

/* USB */

/* USB boot mode */
#define CONFIG_USB_BOOTING
#define EXYNOS_COPY_USB_FNPTR_ADDR	0x02020070
#define EXYNOS_USB_SECONDARY_BOOT	0xfeed0002
#define EXYNOS_IRAM_SECONDARY_BASE	0x02020018

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 1) \
	func(MMC, mmc, 0) \
	func(MMC, mmc, 2) \
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

#define CONFIG_EXTRA_ENV_SETTINGS \
	EXYNOS_DEVICE_SETTINGS \
	EXYNOS_FDTFILE_SETTING \
	MEM_LAYOUT_ENV_SETTINGS \
	BOOTENV

#endif	/* __CONFIG_EXYNOS5_COMMON_H */
