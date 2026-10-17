/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2021 Amarula Solutions B.V.
 *
 */
#ifndef __IMX6ULZ_SMM_M2_CONFIG_H
#define __IMX6ULZ_SMM_M2_CONFIG_H

#include "mx6_common.h"

#include <asm/arch/imx-regs.h>
#include <linux/sizes.h>
#include <linux/stringify.h>

#define CFG_MXC_UART_BASE		UART4_BASE

#ifndef CONFIG_SPL_BUILD

#define BOOT_TARGET_DEVICES(func) \
	func(NAND, nand, 0) \

#include <config_distro_bootcmd.h>

#endif /* !CONFIG_SPL_BUILD */

#define MEM_LAYOUT_ENV_SETTINGS \
	"scriptaddr=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
	"kernel_addr_r=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
	"fdt_addr_r=0x81000000\0" \
	"fdtfile=" CONFIG_DEFAULT_FDT_FILE "\0" \
	"bootcmd_mfg=echo Running fastboot mode; fastboot usb 0\0" \

#define NANDARGS \
	"mtdids=" CONFIG_MTDIDS_DEFAULT "\0" \
	CONFIG_MTDPARTS_DEFAULT "\0" \
	"nandargs=setenv bootargs " \
		"${optargs} " \
		"mtdparts=${mtdparts} " \
		"root=${nandroot} " \
		"rootfstype=${nandrootfstype}\0" \
	"nandroot=ubi0:root rw ubi.mtd=rootfs\0" \
	"nandrootfstype=ubifs rootwait=1\0" \
	"nandboot=echo Booting from nand ...; " \
		"run nandargs; " \
		"nand read ${fdt_addr_r} nanddtb; " \
		"nand read ${loadaddr} kernel; " \
		"bootz ${loadaddr} - ${fdt_addr_r}\0"

#define BOOTENV_DEV_NAND(devtypeu, devtypel, instance) \
	"bootcmd_" #devtypel #instance "=" \
	"run nandboot\0"

#define BOOTENV_DEV_NAME_NAND(devtypeu, devtypel, instance) \
	#devtypel #instance " "

/* Initial environment variables */
#define CFG_EXTRA_ENV_SETTINGS \
	MEM_LAYOUT_ENV_SETTINGS \
	NANDARGS \
	BOOTENV

/* Physical Memory Map */
#define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR
#define PHYS_SDRAM_SIZE			SZ_128M

#define CFG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CFG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CFG_SYS_INIT_RAM_SIZE	IRAM_SIZE

/* NAND */

#define CFG_SYS_NAND_BASE		0x20000000

#endif
