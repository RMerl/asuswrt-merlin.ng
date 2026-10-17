/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Board configuration file for Phytec phyBOARD-i.MX6ULL-Segin SBC
 * Copyright (C) 2019 Parthiban Nallathambi <parthitce@gmail.com>
 *
 * Based on include/configs/xpress.h:
 * Copyright (C) 2015-2016 Stefan Roese <sr@denx.de>
 */
#ifndef __PCL063_ULL_H
#define __PCL063_ULL_H

#include <linux/sizes.h>
#include <linux/stringify.h>
#include "mx6_common.h"

#define CFG_SYS_FSL_USDHC_NUM	2

/* Environment settings */

/* Environment in SD */
#define MMC_ROOTFS_DEV		0
#define MMC_ROOTFS_PART		2

/* Console configs */
#define CFG_MXC_UART_BASE		UART1_BASE

/* MMC Configs */

#define CFG_SYS_FSL_ESDHC_ADDR	USDHC2_BASE_ADDR

/* I2C configs */

/* Miscellaneous configurable options */

/* Physical Memory Map */
#define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR
#define PHYS_SDRAM_SIZE			SZ_256M

#define CFG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CFG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CFG_SYS_INIT_RAM_SIZE	IRAM_SIZE

/* NAND */
#define CFG_SYS_NAND_BASE		0x40000000

/* USB Configs */
#define CFG_MXC_USB_PORTSC		(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CFG_MXC_USB_FLAGS		0

#define ENV_MMC \
	"mmcdev=" __stringify(MMC_ROOTFS_DEV) "\0" \
	"mmcpart=" __stringify(MMC_ROOTFS_PART) "\0" \
	"fitpart=1\0" \
	"bootdelay=3\0" \
	"silent=1\0" \
	"optargs=rw rootwait\0" \
	"mmcautodetect=yes\0" \
	"mmcrootfstype=ext4\0" \
	"mmcfit_name=fitImage\0" \
	"mmcloadfit=fatload mmc ${mmcdev}:${fitpart} ${fit_addr} " \
		    "${mmcfit_name}\0" \
	"mmcargs=setenv bootargs " \
		"root=/dev/mmcblk${mmcdev}p${mmcpart} ${optargs} " \
		"console=${console} rootfstype=${mmcrootfstype}\0" \
	"mmc_mmc_fit=run mmcloadfit;run mmcargs addcon; bootm ${fit_addr}\0" \

/* Default environment */
#define CFG_EXTRA_ENV_SETTINGS \
	"fdt_high=0xffffffff\0" \
	"console=ttymxc0,115200n8\0" \
	"addcon=setenv bootargs ${bootargs} console=${console},${baudrate}\0" \
	"fit_addr=0x82000000\0" \
	ENV_MMC

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(MMC, mmc, 1) \
	func(DHCP, dhcp, na)

#include <config_distro_bootcmd.h>

#endif /* __PCL063_ULL_H */
