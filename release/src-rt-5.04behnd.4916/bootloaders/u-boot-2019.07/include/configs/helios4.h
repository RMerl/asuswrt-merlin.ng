/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2018 Dennis Gilmore <dgilmore@redhat.com>
 */

#ifndef _CONFIG_HELIOS4_H
#define _CONFIG_HELIOS4_H

#include <linux/sizes.h>

/*
 * High Level Configuration Options (easy to change)
 */

/*
 * TEXT_BASE needs to be below 16MiB, since this area is scrubbed
 * for DDR ECC byte filling in the SPL before loading the main
 * U-Boot into it.
 */
#define CONFIG_SYS_TCLK		250000000	/* 250MHz */

/*
 * Commands configuration
 */

/*
 * SDIO/MMC Card Configuration
 */
#define CONFIG_SYS_MMC_BASE		MVEBU_SDIO_BASE

/* USB/EHCI configuration */
#define CONFIG_EHCI_IS_TDI

#define CONFIG_ENV_MIN_ENTRIES		128

/*
 * SATA/SCSI/AHCI configuration
 */
#define CONFIG_SCSI_AHCI_PLAT
#define CONFIG_SYS_SCSI_MAX_SCSI_ID     2
#define CONFIG_SYS_SCSI_MAX_LUN         2
#define CONFIG_SYS_SCSI_MAX_DEVICE      (CONFIG_SYS_SCSI_MAX_SCSI_ID * \
					CONFIG_SYS_SCSI_MAX_LUN)

#ifdef CONFIG_MVEBU_SPL_BOOT_DEVICE_SPI
/* Environment in SPI NOR flash */
#define CONFIG_ENV_SECT_SIZE		SZ_64K
#define CONFIG_ENV_SIZE			CONFIG_ENV_SECT_SIZE
#define CONFIG_ENV_OFFSET		SZ_1M
#endif

#ifdef CONFIG_MVEBU_SPL_BOOT_DEVICE_MMC
/* Environment in MMC */
#define CONFIG_SYS_MMC_ENV_DEV		0
#define CONFIG_ENV_SECT_SIZE		0x200
#define CONFIG_ENV_SIZE			0x2000
/* stay within first 1M */
#define CONFIG_ENV_OFFSET		(SZ_1M - CONFIG_ENV_SIZE)
#define CONFIG_ENV_ADDR			CONFIG_ENV_OFFSET
#endif

#define CONFIG_PHY_MARVELL		/* there is a marvell phy */
#define PHY_ANEG_TIMEOUT	8000	/* PHY needs a longer aneg time */

/* Keep device tree and initrd in lower memory so the kernel can access them */
#define RELOCATION_LIMITS_ENV_SETTINGS	\
	"fdt_high=0x10000000\0"		\
	"initrd_high=0x10000000\0"

/* SPL */
/*
 * Select the boot device here
 *
 * Currently supported are:
 * SPL_BOOT_SPI_NOR_FLASH	- Booting via SPI NOR flash
 * SPL_BOOT_SDIO_MMC_CARD	- Booting via SDIO/MMC card (partition 1)
 */
#define SPL_BOOT_SPI_NOR_FLASH		1
#define SPL_BOOT_SDIO_MMC_CARD		2

#ifdef CONFIG_MVEBU_SPL_BOOT_DEVICE_SPI
#define CONFIG_SPL_BOOT_DEVICE		SPL_BOOT_SPI_NOR_FLASH
#endif
#ifdef CONFIG_MVEBU_SPL_BOOT_DEVICE_MMC
#define CONFIG_SPL_BOOT_DEVICE		SPL_BOOT_SDIO_MMC_CARD
#endif

/* Defines for SPL */
#define CONFIG_SPL_SIZE			(140 << 10)
#define CONFIG_SPL_MAX_SIZE		(CONFIG_SPL_SIZE - 0x0030)

#define CONFIG_SPL_BSS_START_ADDR	(0x40000000 + CONFIG_SPL_SIZE)
#define CONFIG_SPL_BSS_MAX_SIZE		(16 << 10)

#ifdef CONFIG_SPL_BUILD
#define CONFIG_SYS_MALLOC_SIMPLE
#endif

#define CONFIG_SPL_STACK		(0x40000000 + ((192 - 16) << 10))
#define CONFIG_SPL_BOOTROM_SAVE		(CONFIG_SPL_STACK + 4)

#if CONFIG_SPL_BOOT_DEVICE == SPL_BOOT_SPI_NOR_FLASH
/* SPL related SPI defines */
#define CONFIG_SPL_SPI_FLASH_SUPPORT
#define CONFIG_SPL_SPI_LOAD
#define CONFIG_SPL_SPI_SUPPORT
#define CONFIG_SYS_SPI_U_BOOT_OFFS	0x30000
#define CONFIG_SYS_U_BOOT_OFFS		CONFIG_SYS_SPI_U_BOOT_OFFS
#endif

#if CONFIG_SPL_BOOT_DEVICE == SPL_BOOT_SDIO_MMC_CARD
/* SPL related MMC defines */
#define CONFIG_SPL_MMC_SUPPORT
#define CONFIG_SYS_MMC_U_BOOT_OFFS		(160 << 10)
#define CONFIG_SYS_U_BOOT_OFFS			CONFIG_SYS_MMC_U_BOOT_OFFS
#ifdef CONFIG_SPL_BUILD
#define CONFIG_FIXED_SDHCI_ALIGNED_BUFFER	0x00180000	/* in SDRAM */
#endif
#endif
/*
 * mv-common.h should be defined after CMD configs since it used them
 * to enable certain macros
 */
#include "mv-common.h"

/* Include the common distro boot environment */
#ifndef CONFIG_SPL_BUILD

#ifdef CONFIG_MMC
#define BOOT_TARGET_DEVICES_MMC(func) func(MMC, mmc, 0)
#else
#define BOOT_TARGET_DEVICES_MMC(func)
#endif

#ifdef CONFIG_USB_STORAGE
#define BOOT_TARGET_DEVICES_USB(func) func(USB, usb, 0)
#else
#define BOOT_TARGET_DEVICES_USB(func)
#endif

#ifdef CONFIG_SATA
#define BOOT_TARGET_DEVICES_SATA(func) func(SATA, sata, 0)
#else
#define BOOT_TARGET_DEVICES_SATA(func)
#endif

#define BOOT_TARGET_DEVICES(func) \
	BOOT_TARGET_DEVICES_MMC(func) \
	BOOT_TARGET_DEVICES_USB(func) \
	BOOT_TARGET_DEVICES_SATA(func) \
	func(PXE, pxe, na) \
	func(DHCP, dhcp, na)

#define KERNEL_ADDR_R	__stringify(0x800000)
#define FDT_ADDR_R	__stringify(0x100000)
#define RAMDISK_ADDR_R	__stringify(0x1800000)
#define SCRIPT_ADDR_R	__stringify(0x200000)
#define PXEFILE_ADDR_R	__stringify(0x300000)

#define LOAD_ADDRESS_ENV_SETTINGS \
	"kernel_addr_r=" KERNEL_ADDR_R "\0" \
	"fdt_addr_r=" FDT_ADDR_R "\0" \
	"ramdisk_addr_r=" RAMDISK_ADDR_R "\0" \
	"scriptaddr=" SCRIPT_ADDR_R "\0" \
	"pxefile_addr_r=" PXEFILE_ADDR_R "\0"

#include <config_distro_bootcmd.h>

#define CONFIG_EXTRA_ENV_SETTINGS \
	RELOCATION_LIMITS_ENV_SETTINGS \
	LOAD_ADDRESS_ENV_SETTINGS \
	"fdtfile=" CONFIG_DEFAULT_DEVICE_TREE ".dtb\0" \
	"console=ttyS0,115200\0" \
	BOOTENV

#endif /* CONFIG_SPL_BUILD */

#endif /* _CONFIG_HELIOS4_H */
