/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2015 Linaro
 *
 * Peter Griffin <peter.griffin@linaro.org>
 *
 * Configuration for HiKey 96boards CE. Parts were derived from other ARM
 * configurations.
 */

#ifndef __HIKEY_H
#define __HIKEY_H

#include <linux/sizes.h>

#define CONFIG_POWER
#define CONFIG_POWER_HI6553

#define CONFIG_REMAKE_ELF

#define CONFIG_SYS_BOOTM_LEN		SZ_64M

/* Physical Memory Map */

/* CONFIG_SYS_TEXT_BASE needs to align with where ATF loads bl33.bin */

#define PHYS_SDRAM_1			0x00000000

/* 1008 MB (the last 16Mb are secured for TrustZone by ATF*/
#define PHYS_SDRAM_1_SIZE		0x3EFFFFFF

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1

#define CONFIG_SYS_INIT_RAM_SIZE	0x1000

#define CONFIG_SYS_INIT_SP_ADDR         (CONFIG_SYS_SDRAM_BASE + 0x7fff0)

#define CONFIG_SYS_LOAD_ADDR		(CONFIG_SYS_SDRAM_BASE + 0x80000)

/* Generic Timer Definitions */
#define COUNTER_FREQUENCY		19000000

/* Generic Interrupt Controller Definitions */
#define GICD_BASE			0xf6801000
#define GICC_BASE			0xf6802000

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + SZ_8M)

#ifdef CONFIG_CMD_USB
#define CONFIG_USB_DWC2_REG_ADDR 0xF72C0000
/*#define CONFIG_DWC2_DFLT_SPEED_FULL*/
#define CONFIG_DWC2_ENABLE_DYNAMIC_FIFO
#endif

#define CONFIG_HIKEY_GPIO

/* Command line configuration */

/* BOOTP options */
#define CONFIG_BOOTP_BOOTFILESIZE

/* Initial environment variables */

/*
 * Defines where the kernel and FDT will be put in RAM
 */

#define BOOT_TARGET_DEVICES(func) \
	func(USB, usb, 0) \
	func(MMC, mmc, 1) \
	func(DHCP, dhcp, na)
#include <config_distro_bootcmd.h>

#define CONFIG_EXTRA_ENV_SETTINGS	\
				"kernel_name=Image\0"	\
				"kernel_addr_r=0x00080000\0" \
				"fdtfile=hi6220-hikey.dtb\0" \
				"fdt_addr_r=0x02000000\0" \
				"fdt_high=0xffffffffffffffff\0" \
				"initrd_high=0xffffffffffffffff\0" \
				BOOTENV

/* Preserve environment on eMMC */
#define CONFIG_ENV_SIZE			0x1000
#define CONFIG_SYS_MMC_ENV_DEV		0	/* Use eMMC */
#define CONFIG_SYS_MMC_ENV_PART		2	/* Use Boot1 partition */

/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE		512	/* Console I/O Buffer Size */
#define CONFIG_SYS_MAXARGS		64	/* max command args */

#endif /* __HIKEY_H */
