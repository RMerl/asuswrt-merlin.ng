/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015, 2021-2023 Tony Dinh <mibodhi@gmail.com>
 * Copyright (C) 2015
 * Gerald Kerma <dreagle@doukki.net>
 * Luka Perkov <luka.perkov@sartura.hr>
 */

#ifndef _CONFIG_NSA310S_H
#define _CONFIG_NSA310S_H

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
	"console=console=ttyS0,115200\0" \
	"kernel=/boot/zImage\0" \
	"fdt=/boot/nsa310s.dtb\0" \
	"bootargs_root=ubi.mtd=3 root=ubi0:rootfs rootfstype=ubifs rw\0" \
	LOAD_ADDRESS_ENV_SETTINGS

#endif /* _CONFIG_NSA310S_H */
