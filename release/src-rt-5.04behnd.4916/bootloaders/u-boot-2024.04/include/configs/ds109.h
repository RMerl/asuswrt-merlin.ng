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

#include "mv-common.h"

/*
 *  Environment variables configurations
 */

/*
 * max 4k env size is enough, but in case of nand
 * it has to be rounded to sector size
 */

/*
 * Default environment variables
 */

#define CFG_EXTRA_ENV_SETTINGS	\
	"x_bootcmd_ethernet=ping 192.168.1.2\0"	\
	"x_bootcmd_usb=usb start\0"	\
	"x_bootcmd_kernel=fatload usb 0 0x6400000 uImage\0" \
	"x_bootargs=console=ttyS0,115200\0"	\
	"x_bootargs_root=root=/dev/sda2 rootdelay=10\0" \
	"ipaddr=192.168.1.5\0"		\
	"usb0Mode=host\0"

#endif /* _CONFIG_DS109_H */
