/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * am3517_evm.h - Default configuration for AM3517 EVM board.
 *
 * Author: Vaibhav Hiremath <hvaibhav@ti.com>
 *
 * Based on omap3_evm_config.h
 *
 * Copyright (C) 2010 Texas Instruments Incorporated
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <configs/ti_omap3_common.h>

/* Board NAND Info. */
#ifdef CONFIG_MTD_RAW_NAND
#define CFG_SYS_NAND_ECCPOS		{ 2,  3,  4,  5,  6,  7,  8,  9, 10, \
					 11, 12, 13, 14, 16, 17, 18, 19, 20, \
					 21, 22, 23, 24, 25, 26, 27, 28, 30, \
					 31, 32, 33, 34, 35, 36, 37, 38, 39, \
					 40, 41, 42, 44, 45, 46, 47, 48, 49, \
					 50, 51, 52, 53, 54, 55, 56 }

#define CFG_SYS_NAND_ECCSIZE		512
#define CFG_SYS_NAND_ECCBYTES	13
#define CFG_SYS_NAND_U_BOOT_START	CONFIG_TEXT_BASE
/* NAND block size is 128 KiB.  Synchronize these values with
 * corresponding Device Tree entries in Linux:
 *  MLO(SPL)             4 * NAND_BLOCK_SIZE = 512 KiB  @ 0x000000
 *  U-Boot              15 * NAND_BLOCK_SIZE = 1920 KiB @ 0x080000
 *  U-Boot environment   2 * NAND_BLOCK_SIZE = 256 KiB  @ 0x260000
 *  Kernel              64 * NAND_BLOCK_SIZE = 8 MiB    @ 0x2A0000
 *  DTB                  4 * NAND_BLOCK_SIZE = 512 KiB  @ 0xAA0000
 *  RootFS              Remaining Flash Space           @ 0xB20000
 */
#endif /* CONFIG_MTD_RAW_NAND */

/* Environment information */
#define CFG_EXTRA_ENV_SETTINGS \
	"loadaddr=0x82000000\0" \
	"console=ttyS2,115200n8\0" \
	"fdtfile=am3517-evm.dtb\0" \
	"fdtaddr=0x82C00000\0" \
	"vram=16M\0" \
	"bootenv=uEnv.txt\0" \
	"cmdline=\0" \
	"optargs=\0" \
	"mmcdev=0\0" \
	"mmcpart=1\0" \
	"mmcroot=/dev/mmcblk0p2 rw\0" \
	"mmcrootfstype=ext4 rootwait fixrtc\0" \
	"mmcargs=setenv bootargs console=${console} " \
		"${mtdparts} " \
		"${optargs} " \
		"root=${mmcroot} " \
		"rootfstype=${mmcrootfstype} " \
		"${cmdline}\0" \
	"nandargs=setenv bootargs console=${console} " \
		"${mtdparts} " \
		"${optargs} " \
		"root=ubi0:rootfs rw ubi.mtd=rootfs " \
		"rootfstype=ubifs rootwait " \
		"${cmdline}\0" \
	"loadbootenv=fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${bootenv}\0"\
	"importbootenv=echo Importing environment from mmc ...; " \
		"env import -t ${loadaddr} ${filesize}\0" \
	"bootscript=echo Running bootscript from mmc ...; " \
		"source ${loadaddr}\0" \
	"loadimage=fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${bootfile}\0" \
	"loadfdt=fatload mmc ${mmcdev}:${mmcpart} ${fdtaddr} ${fdtfile}\0" \
	"mmcboot=echo Booting from mmc ...; " \
		"run mmcargs; " \
		"bootz ${loadaddr} - ${fdtaddr}\0" \
	"nandboot=echo Booting from nand ...; " \
		"run nandargs; " \
		"nand read ${loadaddr} 2a0000 800000; " \
		"nand read ${fdtaddr} aa0000 80000; " \
		"bootm ${loadaddr} - ${fdtaddr}\0" \

/* Miscellaneous configurable options */

/* memtest works on */

/* FLASH and environment organization */

/* **** PISMO SUPPORT *** */
						/* on one chip */

#if defined(CONFIG_MTD_RAW_NAND)
#define CFG_SYS_FLASH_BASE		NAND_BASE
#endif

#endif /* __CONFIG_H */
