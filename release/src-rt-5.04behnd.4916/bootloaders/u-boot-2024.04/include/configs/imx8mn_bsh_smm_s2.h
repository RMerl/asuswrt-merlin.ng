/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2021 Collabora Ltd.
 */

#ifndef __IMX8MN_BSH_SMM_S2_H
#define __IMX8MN_BSH_SMM_S2_H

#include <configs/imx8mn_bsh_smm_s2_common.h>

#define BOOT_TARGET_DEVICES(func) \
	func(NAND, nand, 0) \

#include <config_distro_bootcmd.h>

#define NANDARGS \
	"nandargs=setenv bootargs " \
		"${optargs} " \
		"mtdparts=${mtdparts} " \
		"root=${nandroot} " \
		"rootfstype=${nandrootfstype}\0" \
	"nandroot=ubi0:root rw ubi.mtd=nandrootfs\0" \
	"nandrootfstype=ubifs rootwait\0" \
	"nandboot=echo Booting from nand ...; " \
		"run nandargs; " \
		"nand read ${fdt_addr_r} nanddtb; " \
		"nand read ${loadaddr} nandkernel; " \
		"booti ${loadaddr} - ${fdt_addr_r}\0"

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

#define PHYS_SDRAM_SIZE			SZ_256M

/* NAND */

#define CFG_SYS_NAND_BASE		0x20000000

#endif /* __IMX8MN_BSH_SMM_S2_H */
