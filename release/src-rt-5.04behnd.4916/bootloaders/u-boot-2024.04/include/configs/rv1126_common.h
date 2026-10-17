/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2019 Rockchip Electronics Co., Ltd
 * Copyright (c) 2022 Edgeble AI Technologies Pvt. Ltd.
 */

#ifndef __CONFIG_RV1126_COMMON_H
#define __CONFIG_RV1126_COMMON_H

#include "rockchip-common.h"

#define CFG_SYS_HZ_CLOCK		24000000

#define CFG_IRAM_BASE		0xff700000

#define GICD_BASE			0xfeff1000
#define GICC_BASE			0xfeff2000

#define CFG_SYS_SDRAM_BASE		0
#define SDRAM_MAX_SIZE			0xfd000000

/* memory size > 128MB */
#define ENV_MEM_LAYOUT_SETTINGS	 \
	"scriptaddr=0x00000000\0" \
	"pxefile_addr_r=0x00100000\0" \
	"fdt_addr_r=0x08300000\0" \
	"fdtoverlay_addr_r=0x02000000\0" \
	"kernel_addr_r=0x02008000\0" \
	"ramdisk_addr_r=0x0a400000\0"

#define CFG_EXTRA_ENV_SETTINGS \
	"fdt_high=0x0fffffff\0" \
	"initrd_high=0x0fffffff\0" \
	"fdtfile=" CONFIG_DEFAULT_FDT_FILE "\0" \
	"partitions=" PARTS_DEFAULT \
	ENV_MEM_LAYOUT_SETTINGS	 \
	ROCKCHIP_DEVICE_SETTINGS \
	"boot_targets=" BOOT_TARGETS "\0"

#endif /* __CONFIG_RV1126_COMMON_H */
