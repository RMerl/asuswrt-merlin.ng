/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2011
 * Stefan Herbrechtsmeier <stefan@herbrechtsmeier.net>
 *
 * Based on Kirkwood support:
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 */

#ifndef _CONFIG_DNS325_H
#define _CONFIG_DNS325_H

/*
 * Machine number definition
 */
#define CONFIG_MACH_TYPE		MACH_TYPE_DNS325

/*
 * High Level Configuration Options (easy to change)
 */
#define CONFIG_FEROCEON_88FR131		/* CPU Core subversion */
#define CONFIG_KW88F6281		/* SOC Name */
#define CONFIG_SKIP_LOWLEVEL_INIT	/* disable board lowlevel_init */

/*
 * Commands configuration
 */

/*
 * mv-common.h should be defined after CMD configs since it used them
 * to enable certain macros
 */
#include "mv-common.h"

/* Remove or override few declarations from mv-common.h */

/*
 * Ethernet Driver configuration
 */
#ifdef CONFIG_CMD_NET
#define CONFIG_MVGBE_PORTS		{1, 0} /* enable port 0 only */
#define CONFIG_NETCONSOLE
#endif

/*
 * SATA Driver configuration
 */
#ifdef CONFIG_MVSATA_IDE
#define CONFIG_SYS_ATA_IDE0_OFFSET      MV_SATA_PORT0_OFFSET
#define CONFIG_SYS_ATA_IDE1_OFFSET      MV_SATA_PORT1_OFFSET
#endif

/*
 * Enable GPI0 support
 */
#define CONFIG_KIRKWOOD_GPIO

/*
 * Environment variables configurations
 */
#ifdef CONFIG_CMD_NAND
#define CONFIG_ENV_SECT_SIZE		0x20000	/* 128KB */
#endif

#define CONFIG_ENV_SIZE			0x20000	/* 128KB */
#define CONFIG_ENV_ADDR			0xe0000
#define CONFIG_ENV_OFFSET		0xe0000	/* env starts here */

/*
 * Default environment variables
 */

#define CONFIG_EXTRA_ENV_SETTINGS \
	"stdin=serial\0" \
	"stdout=serial\0" \
	"stderr=serial\0" \
	"loadaddr=0x800000\0" \
	"autoload=no\0" \
	"console=ttyS0,115200\0" \
	"mtdparts="CONFIG_MTDPARTS_DEFAULT \
	"optargs=\0" \
	"bootenv=uEnv.txt\0" \
	"importbootenv=echo Importing environment ...; " \
		"env import -t ${loadaddr} ${filesize}\0" \
	"loadbootenv=fatload usb 0 ${loadaddr} ${bootenv}\0" \
	"setbootargs=setenv bootargs console=${console} " \
		"${optargs} " \
		"${mtdparts} " \
		"root=${bootenvroot} " \
		"rootfstype=${bootenvrootfstype}\0" \
	"subbootcmd=run setbootargs; " \
		"if run bootenvloadimage; then " \
			"bootm ${loadaddr};" \
		"fi;\0" \
	"nandroot=ubi0:rootfs ubi.mtd=rootfs\0" \
	"nandrootfstype=ubifs\0" \
	"nandloadimage=nand read ${loadaddr} kernel\0" \
	"setnandbootenv=echo Booting from nand ...; " \
		"setenv bootenvroot ${nandroot}; " \
		"setenv bootenvrootfstype ${nandrootfstype}; " \
		"setenv bootenvloadimage ${nandloadimage}\0"

#define CONFIG_BOOTCOMMAND \
	"if test -n ${bootenv} && usb start; then " \
		"if run loadbootenv; then " \
			"echo Loaded environment ${bootenv} from usb;" \
			"run importbootenv;" \
		"fi;" \
		"if test -n ${bootenvcmd}; then " \
			"echo Running bootenvcmd ...;" \
			"run bootenvcmd;" \
		"fi;" \
	"fi;" \
	"run setnandbootenv subbootcmd;"

#endif /* _CONFIG_DNS325_H */
