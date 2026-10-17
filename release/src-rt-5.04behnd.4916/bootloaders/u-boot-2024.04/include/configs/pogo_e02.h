/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2012
 * David Purdy <david.c.purdy@gmail.com>
 *
 * Based on Kirkwood support:
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 */

#ifndef _CONFIG_POGO_E02_H
#define _CONFIG_POGO_E02_H

#include "mv-common.h"

/*
 * Default environment variables
 */

#define CFG_EXTRA_ENV_SETTINGS \
	"bootargs_console=console=ttyS0,115200\0" \
	"bootcmd_usb=usb start; ext2load usb 0:1 0x00800000 /uImage; " \
	"ext2load usb 0:1 0x01100000 /uInitrd\0"

#endif /* _CONFIG_POGO_E02_H */
