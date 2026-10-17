/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2016 Rockchip Electronics Co., Ltd
 */
#ifndef __CONFIG_RV1108_COMMON_H
#define __CONFIG_RV1108_COMMON_H

#include <asm/arch-rockchip/hardware.h>
#include "rockchip-common.h"

#define CFG_IRAM_BASE		0x10080000

#define CFG_SYS_TIMER_RATE		(24 * 1000 * 1000)
/* TIMER1,initialized by ddr initialize code */
#define CFG_SYS_TIMER_BASE		0x10350020
#define CFG_SYS_TIMER_COUNTER	(CFG_SYS_TIMER_BASE + 8)

#define CFG_SYS_SDRAM_BASE		0x60000000

#define ENV_MEM_LAYOUT_SETTINGS \
	"scriptaddr=0x60000000\0" \
	"fdt_addr_r=0x61f00000\0" \
	"kernel_addr_r=0x62000000\0" \
	"ramdisk_addr_r=0x64000000\0"

#include <config_distro_bootcmd.h>
#define CFG_EXTRA_ENV_SETTINGS \
	ENV_MEM_LAYOUT_SETTINGS \
	"fdtfile=" CONFIG_DEFAULT_FDT_FILE "\0" \
	"partitions=" PARTS_DEFAULT \
	"boot_targets=" BOOT_TARGETS "\0"

#endif
