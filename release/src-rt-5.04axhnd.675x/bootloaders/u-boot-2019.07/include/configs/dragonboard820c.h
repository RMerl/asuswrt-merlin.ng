/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Board configuration file for Dragonboard 410C
 *
 * (C) Copyright 2017 Jorge Ramirez-Ortiz <jorge.ramirez-ortiz@linaro.org>
 */

#ifndef __CONFIGS_DRAGONBOARD820C_H
#define __CONFIGS_DRAGONBOARD820C_H

#include <linux/sizes.h>
#include <asm/arch/sysmap-apq8096.h>

/* Physical Memory Map */

#define PHYS_SDRAM_SIZE			0xC0000000
#define PHYS_SDRAM_1			0x80000000
#define PHYS_SDRAM_1_SIZE		0x60000000
#define PHYS_SDRAM_2			0x100000000
#define PHYS_SDRAM_2_SIZE		0x5ea4ffff

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_SDRAM_BASE + 0x7fff0)
#define CONFIG_SYS_LOAD_ADDR		(CONFIG_SYS_SDRAM_BASE + 0x80000)
#define CONFIG_SYS_BOOTM_LEN		SZ_64M

/* Generic Timer Definitions */
#define COUNTER_FREQUENCY		19000000

/* BOOTP options */
#define CONFIG_BOOTP_BOOTFILESIZE

#ifndef CONFIG_SPL_BUILD
#include <config_distro_bootcmd.h>
#endif

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0)

#define CONFIG_EXTRA_ENV_SETTINGS \
	"loadaddr=0x95000000\0" \
	"fdt_high=0xffffffffffffffff\0" \
	"initrd_high=0xffffffffffffffff\0" \
	"linux_image=uImage\0" \
	"kernel_addr_r=0x95000000\0"\
	"fdtfile=qcom/apq8096-db820c.dtb\0" \
	"fdt_addr_r=0x93000000\0"\
	"ramdisk_addr_r=0x91000000\0"\
	"scriptaddr=0x90000000\0"\
	"pxefile_addr_r=0x90100000\0"\
	BOOTENV

#define CONFIG_ENV_SIZE			0x4000

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + SZ_8M)

/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE		512
#define CONFIG_SYS_MAXARGS		64

#endif
