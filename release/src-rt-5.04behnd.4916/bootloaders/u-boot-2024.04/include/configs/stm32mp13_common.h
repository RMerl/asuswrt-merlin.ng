/* SPDX-License-Identifier: GPL-2.0-or-later OR BSD-3-Clause */
/*
 * Copyright (C) 2022, STMicroelectronics - All Rights Reserved
 *
 * Configuration settings for the STM32MP13x CPU
 */

#ifndef __CONFIG_STM32MP13_COMMMON_H
#define __CONFIG_STM32MP13_COMMMON_H
#include <linux/sizes.h>
#include <asm/arch/stm32.h>

/*
 * Configuration of the external SRAM memory used by U-Boot
 */
#define CFG_SYS_SDRAM_BASE		STM32_DDR_BASE

/*
 * For booting Linux, use the first 256 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_SYS_BOOTMAPSZ		SZ_256M

/* NAND support */

/*****************************************************************************/
#ifdef CONFIG_DISTRO_DEFAULTS
/*****************************************************************************/

#ifdef CONFIG_CMD_MMC
#define BOOT_TARGET_MMC0(func)	func(MMC, mmc, 0)
#define BOOT_TARGET_MMC1(func)	func(MMC, mmc, 1)
#else
#define BOOT_TARGET_MMC0(func)
#define BOOT_TARGET_MMC1(func)
#endif

#ifdef CONFIG_CMD_USB
#define BOOT_TARGET_USB(func)	func(USB, usb, 0)
#else
#define BOOT_TARGET_USB(func)
#endif

#define BOOT_TARGET_DEVICES(func)	\
	BOOT_TARGET_MMC1(func)		\
	BOOT_TARGET_MMC0(func)		\
	BOOT_TARGET_USB(func)

/*
 * default bootcmd for stm32mp13:
 * for serial/usb: execute the stm32prog command
 * for mmc boot (eMMC, SD card), distro boot on the same mmc device
 */
#define STM32MP_BOOTCMD "bootcmd_stm32mp=" \
	"echo \"Boot over ${boot_device}${boot_instance}!\";" \
	"if test ${boot_device} = serial || test ${boot_device} = usb;" \
	"then stm32prog ${boot_device} ${boot_instance}; " \
	"else " \
		"run env_check;" \
		"if test ${boot_device} = mmc;" \
		"then env set boot_targets \"mmc${boot_instance}\"; fi;" \
		"run distro_bootcmd;" \
	"fi;\0"

#define STM32MP_EXTRA \
	"env_check=if env info -p -d -q; then env save; fi\0" \
	"boot_net_usb_start=true\0"

#ifndef STM32MP_BOARD_EXTRA_ENV
#define STM32MP_BOARD_EXTRA_ENV
#endif

#include <config_distro_bootcmd.h>

/*
 * memory layout for 32M uncompressed/compressed kernel,
 * 1M fdt, 1M script, 1M pxe and 1M for overlay
 * and the ramdisk at the end.
 */
#define __KERNEL_ADDR_R     __stringify(0xc2000000)
#define __FDT_ADDR_R        __stringify(0xc4000000)
#define __SCRIPT_ADDR_R     __stringify(0xc4100000)
#define __PXEFILE_ADDR_R    __stringify(0xc4200000)
#define __FDTOVERLAY_ADDR_R __stringify(0xc4300000)
#define __RAMDISK_ADDR_R    __stringify(0xc4400000)

#define STM32MP_MEM_LAYOUT \
	"kernel_addr_r=" __KERNEL_ADDR_R "\0" \
	"fdt_addr_r=" __FDT_ADDR_R "\0" \
	"scriptaddr=" __SCRIPT_ADDR_R "\0" \
	"pxefile_addr_r=" __PXEFILE_ADDR_R "\0" \
	"fdtoverlay_addr_r=" __FDTOVERLAY_ADDR_R "\0" \
	"ramdisk_addr_r=" __RAMDISK_ADDR_R "\0"

#define CFG_EXTRA_ENV_SETTINGS \
	STM32MP_MEM_LAYOUT \
	STM32MP_BOOTCMD \
	BOOTENV \
	STM32MP_EXTRA \
	STM32MP_BOARD_EXTRA_ENV

#endif /* ifdef CONFIG_DISTRO_DEFAULTS*/

#endif /* __CONFIG_STM32MP13_COMMMON_H */
