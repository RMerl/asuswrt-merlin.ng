/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2016 Andreas Färber
 */

#ifndef __CONFIG_RK3368_COMMON_H
#define __CONFIG_RK3368_COMMON_H

#include "rockchip-common.h"

#include <asm/arch-rockchip/hardware.h>
#include <linux/sizes.h>

#define CFG_SYS_SDRAM_BASE		0
#define SDRAM_MAX_SIZE			0xff000000

#define CFG_IRAM_BASE		0xff8c0000

#define ENV_MEM_LAYOUT_SETTINGS \
	"scriptaddr=0x00500000\0" \
	"pxefile_addr_r=0x00600000\0" \
	"fdt_addr_r=0x5600000\0" \
	"kernel_addr_r=0x280000\0" \
	"ramdisk_addr_r=0x5bf0000\0"

#define CFG_EXTRA_ENV_SETTINGS \
	"fdtfile=" CONFIG_DEFAULT_DEVICE_TREE ".dtb\0" \
	ENV_MEM_LAYOUT_SETTINGS \
	"boot_targets=" BOOT_TARGETS "\0"

#endif
