/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2014-2015 Freescale Semiconductor, Inc.
 * Copyright Jasbir Matharu
 * Copyright 2015 UDOO Team
 *
 * Configuration settings for the UDOO NEO board.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include "mx6_common.h"

/* MMC Configuration */
#define CFG_SYS_FSL_ESDHC_ADDR	USDHC2_BASE_ADDR

/* Command definition */
#define CFG_MXC_UART_BASE		UART1_BASE

/* Linux only */
#define CFG_EXTRA_ENV_SETTINGS \
	"console=ttymxc0,115200\0" \
	"fdt_high=0xffffffff\0" \
	"initrd_high=0xffffffff\0" \
	"fdtfile=undefined\0" \
	"fdt_addr=0x83000000\0" \
	"fdt_addr_r=0x83000000\0" \
	"ip_dyn=yes\0" \
	"mmcdev=1\0" \
	"mmcrootfstype=ext4\0" \
	"findfdt="\
		"if test $board_name = BASIC; then " \
			"setenv fdtfile imx6sx-udoo-neo-basic.dtb; fi; " \
		"if test $board_name = BASICKS; then " \
			"setenv fdtfile imx6sx-udoo-neo-basic.dtb; fi; " \
		"if test $board_name = FULL; then " \
			"setenv fdtfile imx6sx-udoo-neo-full.dtb; fi; " \
		"if test $board_name = EXTENDED; then " \
			"setenv fdtfile imx6sx-udoo-neo-extended.dtb; fi; " \
		"if test $fdtfile = UNDEFINED; then " \
			"echo WARNING: Could not determine dtb to use; fi\0" \
	"kernel_addr_r=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
	"pxefile_addr_r=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
	"ramdisk_addr_r=0x84000000\0" \
	"scriptaddr=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
	BOOTENV

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 1) \
	func(USB, usb, 0) \
	func(DHCP, dhcp, na)

#include <config_distro_bootcmd.h>

/* Physical Memory Map */
#define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR
#define CFG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CFG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CFG_SYS_INIT_RAM_SIZE	IRAM_SIZE

#endif				/* __CONFIG_H */
