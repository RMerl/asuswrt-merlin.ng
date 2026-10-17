/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2015 Google, Inc
 */

#ifndef __CONFIG_RK3066_COMMON_H
#define __CONFIG_RK3066_COMMON_H

#include <asm/arch-rockchip/hardware.h>
#include "rockchip-common.h"

#define CFG_IRAM_BASE		0x10080000

#define CFG_SYS_SDRAM_BASE		0x60000000
#define SDRAM_BANK_SIZE			(1024UL << 20UL)
#define SDRAM_MAX_SIZE			CONFIG_NR_DRAM_BANKS * SDRAM_BANK_SIZE

#define ENV_MEM_LAYOUT_SETTINGS \
	"scriptaddr=0x60000000\0" \
	"pxefile_addr_r=0x60100000\0" \
	"fdt_addr_r=0x61f00000\0" \
	"kernel_addr_r=0x62000000\0" \
	"ramdisk_addr_r=0x64000000\0"

#define CFG_EXTRA_ENV_SETTINGS \
	"fdt_high=0x6fffffff\0" \
	"initrd_high=0x6fffffff\0" \
	"partitions=" PARTS_DEFAULT \
	ENV_MEM_LAYOUT_SETTINGS \
	ROCKCHIP_DEVICE_SETTINGS \
	"boot_targets=" BOOT_TARGETS "\0"

#endif
