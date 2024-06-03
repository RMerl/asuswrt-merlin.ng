/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2015 Google, Inc
 */

#ifndef __CONFIG_RK3188_COMMON_H
#define __CONFIG_RK3188_COMMON_H

#define CONFIG_SYS_CACHELINE_SIZE	64

#include <asm/arch-rockchip/hardware.h>
#include "rockchip-common.h"

#define CONFIG_SKIP_LOWLEVEL_INIT_ONLY
#define CONFIG_SYS_MALLOC_LEN		(32 << 20)
#define CONFIG_SYS_CBSIZE		1024

#define CONFIG_SYS_NS16550_MEM32

#ifdef CONFIG_SPL_ROCKCHIP_BACK_TO_BROM
/* Bootrom will load u-boot binary to 0x60000000 once return from SPL */
#endif
#define CONFIG_SYS_INIT_SP_ADDR		0x60100000
#define CONFIG_SYS_LOAD_ADDR		0x60800800

#define CONFIG_ROCKCHIP_MAX_INIT_SIZE	(0x8000 - 0x800)
#define CONFIG_ROCKCHIP_CHIP_TAG	"RK31"

/* spl size 32kb sram - 2kb bootrom */
#define CONFIG_SPL_MAX_SIZE		(0x8000 - 0x800)
#define CONFIG_ROCKCHIP_SERIAL		1

#define CONFIG_SPL_STACK		0x10087fff

#define CONFIG_SYS_SDRAM_BASE		0x60000000
#define SDRAM_BANK_SIZE			(2UL << 30)
#define SDRAM_MAX_SIZE			0x80000000

#ifndef CONFIG_SPL_BUILD
/* usb otg */

/* usb host support */
#define ENV_MEM_LAYOUT_SETTINGS \
	"scriptaddr=0x60000000\0" \
	"pxefile_addr_r=0x60100000\0" \
	"fdt_addr_r=0x61f00000\0" \
	"kernel_addr_r=0x62000000\0" \
	"ramdisk_addr_r=0x64000000\0"

#include <config_distro_bootcmd.h>

/* Linux fails to load the fdt if it's loaded above 256M on a Rock board,
 * so limit the fdt reallocation to that */
#define CONFIG_EXTRA_ENV_SETTINGS \
	"fdtfile=" CONFIG_DEFAULT_FDT_FILE "\0" \
	"fdt_high=0x6fffffff\0" \
	"initrd_high=0x6fffffff\0" \
	"partitions=" PARTS_DEFAULT \
	ENV_MEM_LAYOUT_SETTINGS \
	ROCKCHIP_DEVICE_SETTINGS \
	BOOTENV

#endif /* CONFIG_SPL_BUILD */

#define CONFIG_PREBOOT

#endif
