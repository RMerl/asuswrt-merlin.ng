/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2013 Freescale Semiconductor, Inc.
 *
 * Configuration settings for Udoo board.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include "mx6_common.h"

#define CFG_MXC_UART_BASE		UART2_BASE

/* MMC Configuration */
#define CFG_SYS_FSL_ESDHC_ADDR	0

#define CFG_EXTRA_ENV_SETTINGS \
	"console=ttymxc1,115200\0" \
	"fdt_high=0xffffffff\0" \
	"initrd_high=0xffffffff\0" \
	"fdtfile=undefined\0" \
	"fdt_addr=0x18000000\0" \
	"fdt_addr_r=0x18000000\0" \
	"ip_dyn=yes\0" \
	"mmcdev=2\0" \
	"mmcrootfstype=ext4\0" \
	"findfdt="\
		"if test ${board_rev} = MX6Q; then " \
			"setenv fdtfile imx6q-udoo.dtb; fi; " \
		"if test ${board_rev} = MX6DL; then " \
			"setenv fdtfile imx6dl-udoo.dtb; fi; " \
		"if test ${fdtfile} = undefined; then " \
			"echo WARNING: Could not determine dtb to use; fi\0" \
	"kernel_addr_r=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
	"pxefile_addr_r=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
	"ramdisk_addr_r=0x13000000\0" \
	"scriptaddr=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
	BOOTENV

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 2) \
	func(SATA, sata, 0) \
	func(DHCP, dhcp, na)

#include <config_distro_bootcmd.h>
#include <linux/stringify.h>

/* Physical Memory Map */
#define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR

#define CFG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CFG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CFG_SYS_INIT_RAM_SIZE	IRAM_SIZE

/* Environment organization */

#endif			       /* __CONFIG_H * */
