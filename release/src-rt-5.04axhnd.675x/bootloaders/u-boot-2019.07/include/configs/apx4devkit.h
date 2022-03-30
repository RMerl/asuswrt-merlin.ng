/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2012 Bluegiga Technologies Oy
 *
 * Authors:
 * Veli-Pekka Peltola <veli-pekka.peltola@bluegiga.com>
 * Lauri Hintsala <lauri.hintsala@bluegiga.com>
 *
 * Based on m28evk.h:
 * Copyright (C) 2011 Marek Vasut <marek.vasut@gmail.com>
 * on behalf of DENX Software Engineering GmbH
 */
#ifndef __CONFIGS_APX4DEVKIT_H__
#define __CONFIGS_APX4DEVKIT_H__

/* System configurations */
#define CONFIG_MACH_TYPE	MACH_TYPE_APX4DEVKIT

/* Memory configuration */
#define PHYS_SDRAM_1			0x40000000	/* Base address */
#define PHYS_SDRAM_1_SIZE		0x20000000	/* Max 512 MB RAM */
#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1

/* Environment */
#define CONFIG_ENV_OVERWRITE

/* Environment is in MMC */
#if defined(CONFIG_CMD_MMC) && defined(CONFIG_ENV_IS_IN_MMC)
#define CONFIG_ENV_OFFSET		(256 * 1024)
#define CONFIG_ENV_SIZE			(16 * 1024)
#define CONFIG_SYS_MMC_ENV_DEV		0
#endif

/* Environment is in NAND */
#if defined(CONFIG_CMD_NAND) && defined(CONFIG_ENV_IS_IN_NAND)
#define CONFIG_ENV_SECT_SIZE		(128 * 1024)
#define CONFIG_ENV_SIZE			(128 * 1024)
#define CONFIG_ENV_SIZE_REDUND		CONFIG_ENV_SIZE
#define CONFIG_ENV_RANGE		(384 * 1024)
#define CONFIG_ENV_OFFSET		0x120000
#define CONFIG_ENV_OFFSET_REDUND	\
		(CONFIG_ENV_OFFSET + CONFIG_ENV_RANGE)
#endif

/* UBI and NAND partitioning */

/* FEC Ethernet on SoC */
#ifdef CONFIG_CMD_NET
#define CONFIG_FEC_MXC
#define CONFIG_ETHPRIME			"FEC"
#define CONFIG_FEC_MXC_PHYADDR		0
#define IMX_FEC_BASE			MXS_ENET0_BASE
#endif

/* USB */
#ifdef CONFIG_CMD_USB
#define CONFIG_EHCI_MXS_PORT1
#define CONFIG_USB_MAX_CONTROLLER_COUNT	1
#endif

/* Boot Linux */
#define CONFIG_BOOTFILE			"uImage"
#define CONFIG_BOOTCOMMAND		"run bootcmd_nand"
#define CONFIG_LOADADDR			0x41000000
#define CONFIG_SYS_LOAD_ADDR		CONFIG_LOADADDR
#define CONFIG_SERIAL_TAG
#define CONFIG_REVISION_TAG

/* Extra Environments */
#define CONFIG_EXTRA_ENV_SETTINGS \
	"mtdparts=" CONFIG_MTDPARTS_DEFAULT "\0" \
	"verify=no\0" \
	"bootcmd=run bootcmd_nand\0" \
	"kernelargs=console=tty0 console=ttyAMA0,115200 consoleblank=0\0" \
	"bootargs_nand=" \
		"setenv bootargs ${kernelargs} ubi.mtd=3,2048 " \
		"root=ubi0:rootfs rootfstype=ubifs ${mtdparts} rw\0" \
	"bootcmd_nand=" \
		"run bootargs_nand && ubi part root 2048 && " \
		"ubifsmount ubi:rootfs && ubifsload 41000000 boot/uImage && " \
		"bootm 41000000\0" \
	"bootargs_mmc=" \
		"setenv bootargs ${kernelargs} " \
		"root=/dev/mmcblk0p2 rootwait ${mtdparts} rw\0" \
	"bootcmd_mmc=" \
		"run bootargs_mmc && mmc rescan && " \
		"ext2load mmc 0:2 41000000 boot/uImage && bootm 41000000\0" \
""

/* The rest of the configuration is shared */
#include <configs/mxs.h>

#endif /* __CONFIGS_APX4DEVKIT_H__ */
