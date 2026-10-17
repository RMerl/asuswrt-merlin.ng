/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2014 Eukréa Electromatique
 * Author: Eric Bénard <eric@eukrea.com>
 *
 * Configuration settings for the Embest RIoTboard
 *
 * based on mx6*sabre*.h which are :
 * Copyright (C) 2012 Freescale Semiconductor, Inc.
 */

#ifndef __RIOTBOARD_CONFIG_H
#define __RIOTBOARD_CONFIG_H

#define CFG_MXC_UART_BASE		UART2_BASE
#define CONSOLE_DEV		"ttymxc1"

#define PHYS_SDRAM_SIZE		(1u * 1024 * 1024 * 1024)

/* USB Configs */
#define CFG_MXC_USB_PORTSC	(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CFG_MXC_USB_FLAGS	0

/* MMC Configs */
#define CFG_SYS_FSL_ESDHC_ADDR      0

/* Physical Memory Map */
#define PHYS_SDRAM                     MMDC0_ARB_BASE_ADDR

#define CFG_SYS_SDRAM_BASE          PHYS_SDRAM
#define CFG_SYS_INIT_RAM_ADDR       IRAM_BASE_ADDR
#define CFG_SYS_INIT_RAM_SIZE       IRAM_SIZE

/* Environment organization */

#if defined(CONFIG_ENV_IS_IN_MMC)
/* RiOTboard */
#define FDTFILE	"imx6dl-riotboard.dtb"
#define CFG_SYS_FSL_USDHC_NUM	3
#elif defined(CONFIG_ENV_IS_IN_SPI_FLASH)
/* MarSBoard */
#define FDTFILE	"imx6q-marsboard.dtb"
#define CFG_SYS_FSL_USDHC_NUM	2
#endif

#include "mx6_common.h"

/* 256M RAM (minimum), 32M uncompressed kernel, 16M compressed kernel, 1M fdt,
 * 1M script, 1M pxe and the ramdisk at the end */
#define MEM_LAYOUT_ENV_SETTINGS \
	"bootm_size=0x10000000\0" \
	"kernel_addr_r=0x12000000\0" \
	"fdt_addr_r=0x13000000\0" \
	"scriptaddr=0x13100000\0" \
	"pxefile_addr_r=0x13200000\0" \
	"ramdisk_addr_r=0x13300000\0"

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(MMC, mmc, 1) \
	func(MMC, mmc, 2) \
	func(USB, usb, 0) \
	func(PXE, pxe, na) \
	func(DHCP, dhcp, na)

#include <config_distro_bootcmd.h>

#define CONSOLE_STDIN_SETTINGS \
	"stdin=serial\0"

#define CONSOLE_STDOUT_SETTINGS \
	"stdout=serial\0" \
	"stderr=serial\0"

#define CONSOLE_ENV_SETTINGS \
	CONSOLE_STDIN_SETTINGS \
	CONSOLE_STDOUT_SETTINGS

#define CFG_EXTRA_ENV_SETTINGS \
	CONSOLE_ENV_SETTINGS \
	MEM_LAYOUT_ENV_SETTINGS \
	"fdtfile=" FDTFILE "\0" \
	"finduuid=part uuid mmc 0:1 uuid\0" \
	BOOTENV

#endif                         /* __RIOTBOARD_CONFIG_H */
