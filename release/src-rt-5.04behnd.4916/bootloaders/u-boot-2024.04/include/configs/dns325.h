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

#include "mv-common.h"

/* Remove or override few declarations from mv-common.h */

/*
 * Enable GPI0 support
 */

/*
 * Environment variables configurations
 */

/*
 * Default environment variables
 */

#define CFG_EXTRA_ENV_SETTINGS \
	"stdin=serial\0" \
	"stdout=serial\0" \
	"stderr=serial\0" \
	"loadaddr=0x800000\0" \
	"console=ttyS0,115200\0" \
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

#endif /* _CONFIG_DNS325_H */
