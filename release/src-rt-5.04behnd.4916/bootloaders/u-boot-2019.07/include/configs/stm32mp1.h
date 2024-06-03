/* SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause */
/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 *
 * Configuration settings for the STM32MP15x CPU
 */

#ifndef __CONFIG_H
#define __CONFIG_H
#include <linux/sizes.h>
#include <asm/arch/stm32.h>

/*
 * Number of clock ticks in 1 sec
 */
#define CONFIG_SYS_HZ				1000

#ifndef CONFIG_STM32MP1_TRUSTED
/* PSCI support */
#define CONFIG_ARMV7_PSCI_1_0
#define CONFIG_ARMV7_SECURE_BASE		STM32_SYSRAM_BASE
#define CONFIG_ARMV7_SECURE_MAX_SIZE		STM32_SYSRAM_SIZE
#endif

/*
 * Configuration of the external SRAM memory used by U-Boot
 */
#define CONFIG_SYS_SDRAM_BASE			STM32_DDR_BASE
#define CONFIG_SYS_INIT_SP_ADDR			CONFIG_SYS_TEXT_BASE

/*
 * Console I/O buffer size
 */
#define CONFIG_SYS_CBSIZE			SZ_1K

/*
 * Needed by "loadb"
 */
#define CONFIG_SYS_LOAD_ADDR			STM32_DDR_BASE

#if defined(CONFIG_ENV_IS_IN_UBI)
#define CONFIG_ENV_UBI_VOLUME_REDUND		"uboot_config_r"
#endif

#if defined(CONFIG_ENV_IS_IN_SPI_FLASH)
#define	CONFIG_ENV_SECT_SIZE			SZ_256K
#define	CONFIG_ENV_OFFSET			0x00280000
#endif

/* ATAGs */
#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG

/* Extend size of kernel image for uncompression */
#define CONFIG_SYS_BOOTM_LEN			SZ_32M

/* SPL support */
#ifdef CONFIG_SPL
/* BOOTROM load address */
/* SPL use DDR */
#define CONFIG_SPL_BSS_START_ADDR	0xC0200000
#define CONFIG_SPL_BSS_MAX_SIZE		0x00100000
#define CONFIG_SYS_SPL_MALLOC_START	0xC0300000
#define CONFIG_SYS_SPL_MALLOC_SIZE	0x00100000

/* limit SYSRAM usage to first 128 KB */
#define CONFIG_SPL_MAX_SIZE		0x00020000
#define CONFIG_SPL_STACK		(STM32_SYSRAM_BASE + \
					 STM32_SYSRAM_SIZE)
#endif /* #ifdef CONFIG_SPL */

#define CONFIG_SYS_MEMTEST_START	STM32_DDR_BASE
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_MEMTEST_START + SZ_64M)
#define CONFIG_SYS_MEMTEST_SCRATCH	(CONFIG_SYS_MEMTEST_END + 4)

/*MMC SD*/
#define CONFIG_SYS_MMC_MAX_DEVICE	3

/* Ethernet need */
#ifdef CONFIG_DWC_ETH_QOS
#define CONFIG_SYS_NONCACHED_MEMORY	(1 * SZ_1M)	/* 1M */
#define CONFIG_SERVERIP                 192.168.1.1
#define CONFIG_BOOTP_SERVERIP
#define CONFIG_SYS_AUTOLOAD		"no"
#endif

/*****************************************************************************/
#ifdef CONFIG_DISTRO_DEFAULTS
/*****************************************************************************/

#if !defined(CONFIG_SPL_BUILD)

/* NAND support */
#define CONFIG_SYS_NAND_ONFI_DETECTION
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 1) \
	func(MMC, mmc, 0) \
	func(MMC, mmc, 2) \
	func(PXE, pxe, na)

/*
 * bootcmd for stm32mp1:
 * for serial/usb: execute the stm32prog command
 * for mmc boot (eMMC, SD card), boot only on the same device
 * for nand boot, boot with on ubifs partition on nand
 * for nor boot, use the default order
 */
#define STM32MP_BOOTCMD "bootcmd_stm32mp=" \
	"echo \"Boot over ${boot_device}${boot_instance}!\";" \
	"if test ${boot_device} = serial || test ${boot_device} = usb;" \
	"then stm32prog ${boot_device} ${boot_instance}; " \
	"else " \
		"if test ${boot_device} = mmc;" \
		"then env set boot_targets \"mmc${boot_instance}\"; fi;" \
		"if test ${boot_device} = nand;" \
		"then env set boot_targets ubifs0; fi;" \
		"run distro_bootcmd;" \
	"fi;\0"

#include <config_distro_bootcmd.h>

#if defined(CONFIG_STM32_QSPI) || defined(CONFIG_NAND_STM32_FMC)
#define CONFIG_SYS_MTDPARTS_RUNTIME
#endif

#define STM32MP_MTDPARTS \
	"mtdparts_nor0=256k(fsbl1),256k(fsbl2),2m(ssbl),256k(u-boot-env),-(nor_user)\0" \
	"mtdparts_nand0=2m(fsbl),2m(ssbl1),2m(ssbl2),-(UBI)\0"

/*
 * memory layout for 32M uncompressed/compressed kernel,
 * 1M fdt, 1M script, 1M pxe and 1M for splashimage
 * and the ramdisk at the end.
 */
#define CONFIG_EXTRA_ENV_SETTINGS \
	"kernel_addr_r=0xc2000000\0" \
	"fdt_addr_r=0xc4000000\0" \
	"scriptaddr=0xc4100000\0" \
	"pxefile_addr_r=0xc4200000\0" \
	"splashimage=0xc4300000\0"  \
	"ramdisk_addr_r=0xc4400000\0" \
	"fdt_high=0xffffffff\0" \
	"initrd_high=0xffffffff\0" \
	STM32MP_BOOTCMD \
	STM32MP_MTDPARTS \
	BOOTENV

#endif /* ifndef CONFIG_SPL_BUILD */
#endif /* ifdef CONFIG_DISTRO_DEFAULTS*/

#endif /* __CONFIG_H */
