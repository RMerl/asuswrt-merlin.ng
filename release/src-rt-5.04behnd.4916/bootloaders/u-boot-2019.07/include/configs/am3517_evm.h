/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * am3517_evm.h - Default configuration for AM3517 EVM board.
 *
 * Author: Vaibhav Hiremath <hvaibhav@ti.com>
 *
 * Based on omap3_evm_config.h
 *
 * Copyright (C) 2010 Texas Instruments Incorporated
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <configs/ti_omap3_common.h>

#undef CONFIG_DM_I2C_COMPAT
#define CONFIG_REVISION_TAG

/* Hardware drivers */

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

/*
 * USB configuration
 * Enable CONFIG_USB_MUSB_HOST for Host functionalities MSC, keyboard
 * Enable CONFIG_USB_MUSB_GADGET for Device functionalities.
 */

#ifdef CONFIG_USB_MUSB_AM35X

#ifdef CONFIG_USB_MUSB_HOST

#ifdef CONFIG_USB_KEYBOARD
#define CONFIG_PREBOOT "usb start"
#endif /* CONFIG_USB_KEYBOARD */

#endif /* CONFIG_USB_MUSB_HOST */

#endif /* CONFIG_USB_MUSB_AM35X */

/* I2C */

/* Ethernet */
#define CONFIG_DRIVER_TI_EMAC_USE_RMII
#define CONFIG_BOOTP_DEFAULT
#define CONFIG_BOOTP_DNS2
#define CONFIG_BOOTP_SEND_HOSTNAME
#define CONFIG_NET_RETRY_COUNT		10

/* Board NAND Info. */
#ifdef CONFIG_NAND
#define CONFIG_SYS_NAND_5_ADDR_CYCLE
#define CONFIG_SYS_NAND_PAGE_COUNT	64
#define CONFIG_SYS_NAND_PAGE_SIZE	2048
#define CONFIG_SYS_NAND_OOBSIZE		64
#define CONFIG_SYS_NAND_BLOCK_SIZE	(128 * 1024)
#define CONFIG_SYS_NAND_BAD_BLOCK_POS	NAND_LARGE_BADBLOCK_POS
#define CONFIG_SYS_NAND_ECCPOS		{ 2,  3,  4,  5,  6,  7,  8,  9, 10, \
					 11, 12, 13, 14, 16, 17, 18, 19, 20, \
					 21, 22, 23, 24, 25, 26, 27, 28, 30, \
					 31, 32, 33, 34, 35, 36, 37, 38, 39, \
					 40, 41, 42, 44, 45, 46, 47, 48, 49, \
					 50, 51, 52, 53, 54, 55, 56 }

#define CONFIG_SYS_NAND_ECCSIZE		512
#define CONFIG_SYS_NAND_ECCBYTES	13
#define CONFIG_NAND_OMAP_ECCSCHEME	OMAP_ECC_BCH8_CODE_HW_DETECTION_SW
#define CONFIG_SYS_NAND_MAX_OOBFREE	2
#define CONFIG_SYS_NAND_MAX_ECCPOS	56
#define CONFIG_SYS_NAND_U_BOOT_START	CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_NAND_U_BOOT_OFFS	0x80000
#define CONFIG_SYS_NAND_SPL_KERNEL_OFFS 0x2a0000
/* NAND block size is 128 KiB.  Synchronize these values with
 * corresponding Device Tree entries in Linux:
 *  MLO(SPL)             4 * NAND_BLOCK_SIZE = 512 KiB  @ 0x000000
 *  U-Boot              15 * NAND_BLOCK_SIZE = 1920 KiB @ 0x080000
 *  U-Boot environment   2 * NAND_BLOCK_SIZE = 256 KiB  @ 0x260000
 *  Kernel              64 * NAND_BLOCK_SIZE = 8 MiB    @ 0x2A0000
 *  DTB                  4 * NAND_BLOCK_SIZE = 512 KiB  @ 0xAA0000
 *  RootFS              Remaining Flash Space           @ 0xB20000
 */
#endif /* CONFIG_NAND */

/* Environment information */

#define CONFIG_BOOTFILE		"uImage"

#define CONFIG_EXTRA_ENV_SETTINGS \
	"loadaddr=0x82000000\0" \
	"console=ttyS2,115200n8\0" \
	"fdtfile=am3517-evm.dtb\0" \
	"fdtaddr=0x82C00000\0" \
	"vram=16M\0" \
	"bootenv=uEnv.txt\0" \
	"cmdline=\0" \
	"optargs=\0" \
	"mtdids=" CONFIG_MTDIDS_DEFAULT "\0" \
	"mtdparts=" CONFIG_MTDPARTS_DEFAULT "\0" \
	"mmcdev=0\0" \
	"mmcpart=1\0" \
	"mmcroot=/dev/mmcblk0p2 rw\0" \
	"mmcrootfstype=ext4 rootwait fixrtc\0" \
	"mmcargs=setenv bootargs console=${console} " \
		"${mtdparts} " \
		"${optargs} " \
		"root=${mmcroot} " \
		"rootfstype=${mmcrootfstype} " \
		"${cmdline}\0" \
	"nandargs=setenv bootargs console=${console} " \
		"${mtdparts} " \
		"${optargs} " \
		"root=ubi0:rootfs rw ubi.mtd=rootfs " \
		"rootfstype=ubifs rootwait " \
		"${cmdline}\0" \
	"loadbootenv=fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${bootenv}\0"\
	"importbootenv=echo Importing environment from mmc ...; " \
		"env import -t ${loadaddr} ${filesize}\0" \
	"bootscript=echo Running bootscript from mmc ...; " \
		"source ${loadaddr}\0" \
	"loadimage=fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${bootfile}\0" \
	"loadfdt=fatload mmc ${mmcdev}:${mmcpart} ${fdtaddr} ${fdtfile}\0" \
	"mmcboot=echo Booting from mmc ...; " \
		"run mmcargs; " \
		"bootz ${loadaddr} - ${fdtaddr}\0" \
	"nandboot=echo Booting from nand ...; " \
		"run nandargs; " \
		"nand read ${loadaddr} 2a0000 800000; " \
		"nand read ${fdtaddr} aa0000 80000; " \
		"bootm ${loadaddr} - ${fdtaddr}\0" \

#define CONFIG_BOOTCOMMAND \
	"mmc dev ${mmcdev}; if mmc rescan; then " \
		"echo SD/MMC found on device $mmcdev; " \
		"if run loadbootenv; then " \
			"run importbootenv; " \
		"fi; " \
		"echo Checking if uenvcmd is set ...; " \
		"if test -n $uenvcmd; then " \
			"echo Running uenvcmd ...; " \
			"run uenvcmd; " \
		"fi; " \
		"echo Running default loadimage ...; " \
		"setenv bootfile zImage; " \
		"if run loadimage; then " \
			"run loadfdt; " \
			"run mmcboot; " \
		"fi; " \
	"else run nandboot; fi"

/* Miscellaneous configurable options */

/* We set the max number of command args high to avoid HUSH bugs. */
#define CONFIG_SYS_MAXARGS		64

/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE \
					+ sizeof(CONFIG_SYS_PROMPT) + 16)
/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE

/* memtest works on */
#define CONFIG_SYS_MEMTEST_START	(OMAP34XX_SDRC_CS0)
#define CONFIG_SYS_MEMTEST_END		(OMAP34XX_SDRC_CS0 + \
					0x01F00000) /* 31MB */

/* Physical Memory Map */
#define CONFIG_SYS_CS0_SIZE		(256 * 1024 * 1024)

/* FLASH and environment organization */

/* **** PISMO SUPPORT *** */
#define CONFIG_SYS_MAX_FLASH_SECT	520	/* max number of sectors */
						/* on one chip */
#define CONFIG_SYS_MAX_FLASH_BANKS	2	/* max number of flash banks */
#define CONFIG_SYS_MONITOR_LEN		(256 << 10)	/* Reserve 2 sectors */

#if defined(CONFIG_NAND)
#define CONFIG_SYS_FLASH_BASE		NAND_BASE
#endif

/* Monitor at start of flash */
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_FLASH_BASE

#define CONFIG_SYS_ENV_SECT_SIZE	(128 << 10)	/* 128 KiB */
#define CONFIG_ENV_SIZE			CONFIG_SYS_ENV_SECT_SIZE
#define CONFIG_ENV_OFFSET		0x260000
#define CONFIG_ENV_ADDR			0x260000

/* Defines for SPL */

#define CONFIG_SYS_MMCSD_FS_BOOT_PARTITION	1
#define CONFIG_SPL_FS_LOAD_PAYLOAD_NAME		"u-boot.img"

#define CONFIG_SPL_NAND_BASE
#define CONFIG_SPL_NAND_DRIVERS
#define CONFIG_SPL_NAND_ECC

#endif /* __CONFIG_H */
