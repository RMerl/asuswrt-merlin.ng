/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2015 Google, Inc
 */

#ifndef __CONFIG_RK3288_COMMON_H
#define __CONFIG_RK3288_COMMON_H

#include <asm/arch-rockchip/hardware.h>
#include "rockchip-common.h"

#define CFG_SYS_HZ_CLOCK		24000000

#define CFG_IRAM_BASE		0xff700000

#define CFG_SYS_SDRAM_BASE		0
#define SDRAM_MAX_SIZE			0xfe000000

#define ENV_MEM_LAYOUT_SETTINGS \
	"scriptaddr=0x00000000\0" \
	"pxefile_addr_r=0x00100000\0" \
	"fdt_addr_r=0x01f00000\0" \
	"kernel_addr_r=0x02000000\0" \
	"ramdisk_addr_r=0x04000000\0"

/* Linux fails to load the fdt if it's loaded above 256M on a Rock 2 board, so
 * limit the fdt reallocation to that */
#define CFG_EXTRA_ENV_SETTINGS \
	"fdt_high=0x0fffffff\0" \
	"initrd_high=0x0fffffff\0" \
	"fdtfile=" CONFIG_DEFAULT_FDT_FILE "\0" \
	"partitions=" PARTS_DEFAULT \
	ENV_MEM_LAYOUT_SETTINGS \
	ROCKCHIP_DEVICE_SETTINGS \
	"boot_targets=" BOOT_TARGETS "\0"

#endif
