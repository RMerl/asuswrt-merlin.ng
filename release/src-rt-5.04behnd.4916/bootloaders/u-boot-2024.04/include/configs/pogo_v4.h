/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2014-2023 Tony Dinh <mibodhi@gmail.com>
 *
 * Based on
 * Copyright (C) 2012
 * David Purdy <david.c.purdy@gmail.com>
 *
 * Based on Kirkwood support:
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 */

#ifndef _CONFIG_POGO_V4_H
#define _CONFIG_POGO_V4_H

/*
 * mv-common.h should be defined after CMD configs since it used them
 * to enable certain macros
 */
#include "mv-common.h"

#define KERNEL_ADDR_R	__stringify(0x800000)
#define FDT_ADDR_R	__stringify(0x2c00000)
#define RAMDISK_ADDR_R	__stringify(0x01100000)
#define SCRIPT_ADDR_R	__stringify(0x200000)

#define LOAD_ADDRESS_ENV_SETTINGS \
	"kernel_addr_r=" KERNEL_ADDR_R "\0" \
	"fdt_addr_r=" FDT_ADDR_R "\0" \
	"ramdisk_addr_r=" RAMDISK_ADDR_R "\0" \
	"scriptaddr=" SCRIPT_ADDR_R "\0"

#define CFG_EXTRA_ENV_SETTINGS \
	LOAD_ADDRESS_ENV_SETTINGS \
	"fdtfile=" CONFIG_DEFAULT_DEVICE_TREE ".dtb\0" \
	"mtdparts=" CONFIG_MTDPARTS_DEFAULT "\0" \
	"console=ttyS0,115200\0"

#endif /* _CONFIG_POGO_V4_H */
