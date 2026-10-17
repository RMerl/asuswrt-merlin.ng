/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2009-2014
 * Gerald Kerma <dreagle@doukki.net>
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Siddarth Gore <gores@marvell.com>
 */

#ifndef _CONFIG_GURUPLUG_H
#define _CONFIG_GURUPLUG_H

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

#define CFG_EXTRA_ENV_SETTINGS \
	"console=console=ttyS0,115200\0"				\
	"mtdids=nand0=orion_nand\0"					\
	"kernel=/boot/zImage\0"						\
	"fdt=/boot/guruplug-server-plus.dtb\0"				\
	"bootargs_root=ubi.mtd=2 root=ubi0:rootfs rootfstype=ubifs rw\0"

#endif /* _CONFIG_GURUPLUG_H */
