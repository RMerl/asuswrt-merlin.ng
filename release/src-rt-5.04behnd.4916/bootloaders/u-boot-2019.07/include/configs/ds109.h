/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2011
 * Jason Cooper <u-boot@lakedaemon.net>
 *
 * Based on work by:
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Siddarth Gore <gores@marvell.com>
 */

#ifndef _CONFIG_DS109_H
#define _CONFIG_DS109_H

/* Provide the MACH_TYPE value that the vendor kernel requires. */
#define CONFIG_MACH_TYPE		527

/*
 * High Level Configuration Options (easy to change)
 */
#define CONFIG_SHEEVA_88SV131	1	/* CPU Core subversion */

/*
 * Commands configuration
 */
#define CONFIG_CMD_EXT2

/*
 * mv-plug-common.h should be defined after CMD configs since it used them
 * to enable certain macros
 */
#include "mv-plug-common.h"

/*
 *  Environment variables configurations
 */
#ifdef CONFIG_SPI_FLASH
#define CONFIG_ENV_SECT_SIZE		0x10000	/* 64k */
#endif

/*
 * max 4k env size is enough, but in case of nand
 * it has to be rounded to sector size
 */
#define CONFIG_ENV_SIZE			0x10000
#define CONFIG_ENV_ADDR			0x3d0000
#define CONFIG_ENV_OFFSET		0x3d0000 /* env starts here */

/*
 * Default environment variables
 */
#define CONFIG_BOOTCOMMAND		"setenv ethact egiga0; " \
	"${x_bootcmd_ethernet}; ${x_bootcmd_usb}; ${x_bootcmd_kernel}; "\
	"setenv bootargs ${x_bootargs} ${x_bootargs_root}; "	\
	"bootm 0x6400000;"

#define CONFIG_EXTRA_ENV_SETTINGS	\
	"x_bootcmd_ethernet=ping 192.168.1.2\0"	\
	"x_bootcmd_usb=usb start\0"	\
	"x_bootcmd_kernel=fatload usb 0 0x6400000 uImage\0" \
	"x_bootargs=console=ttyS0,115200\0"	\
	"x_bootargs_root=root=/dev/sda2 rootdelay=10\0" \
	"ipaddr=192.168.1.5\0"

/*
 * Ethernet Driver configuration
 */
#ifdef CONFIG_CMD_NET
#define CONFIG_MVGBE_PORTS	{1, 0}	/* enable one port */
#define CONFIG_PHY_BASE_ADR	8
#endif /* CONFIG_CMD_NET */

/*
 * SATA Driver configuration
 */
#ifdef CONFIG_MVSATA_IDE
#define CONFIG_SYS_ATA_IDE0_OFFSET	MV_SATA_PORT0_OFFSET
#endif /*CONFIG_MVSATA_IDE*/

#endif /* _CONFIG_DS109_H */
