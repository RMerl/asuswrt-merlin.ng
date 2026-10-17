/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2022 Tony Dinh <mibodhi@gmail.com>
 * (C) Copyright 2009-2014
 * Gerald Kerma <dreagle@doukki.net>
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 */

#ifndef _CONFIG_SHEEVAPLUG_H
#define _CONFIG_SHEEVAPLUG_H

#include "mv-common.h"

/*
 *  Environment variables configurations
 */
#define CFG_EXTRA_ENV_SETTINGS	"x_bootargs=console"	\
	"=ttyS0,115200 mtdparts=" CONFIG_MTDPARTS_DEFAULT	\
	"x_bootcmd_kernel=nand read 0x6400000 0x100000 0x400000\0" \
	"x_bootcmd_usb=usb start\0" \
	"x_bootargs_root=root=/dev/mtdblock3 rw rootfstype=jffs2\0"

#endif /* _CONFIG_SHEEVAPLUG_H */
