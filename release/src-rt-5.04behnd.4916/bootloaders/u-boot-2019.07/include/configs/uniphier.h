/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2012-2015 Panasonic Corporation
 * Copyright (C) 2015-2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

/* U-Boot - Common settings for UniPhier Family */

#ifndef __CONFIG_UNIPHIER_COMMON_H__
#define __CONFIG_UNIPHIER_COMMON_H__

#ifndef CONFIG_SPL_BUILD
#include <config_distro_bootcmd.h>

#ifdef CONFIG_CMD_MMC
#define BOOT_TARGET_DEVICE_MMC(func)	func(MMC, mmc, 0) func(MMC, mmc, 1)
#else
#define BOOT_TARGET_DEVICE_MMC(func)
#endif

#ifdef CONFIG_CMD_UBIFS
#define BOOT_TARGET_DEVICE_UBIFS(func)	func(UBIFS, ubifs, 0)
#else
#define BOOT_TARGET_DEVICE_UBIFS(func)
#endif

#ifdef CONFIG_CMD_USB
#define BOOT_TARGET_DEVICE_USB(func)	func(USB, usb, 0)
#else
#define BOOT_TARGET_DEVICE_USB(func)
#endif

#define BOOT_TARGET_DEVICES(func)	\
	BOOT_TARGET_DEVICE_MMC(func)	\
	BOOT_TARGET_DEVICE_UBIFS(func)	\
	BOOT_TARGET_DEVICE_USB(func)
#else
#define BOOTENV
#endif

#define CONFIG_ARMV7_PSCI_1_0

/*-----------------------------------------------------------------------
 * MMU and Cache Setting
 *----------------------------------------------------------------------*/

#define CONFIG_SYS_MALLOC_LEN		(4 * 1024 * 1024)

#define CONFIG_TIMESTAMP

/* FLASH related */

#define CONFIG_SYS_MAX_FLASH_SECT	256
#define CONFIG_SYS_MONITOR_BASE		0
#define CONFIG_SYS_MONITOR_LEN		0x000d0000	/* 832KB */
#define CONFIG_SYS_FLASH_BASE		0

/*
 * flash_toggle does not work for our support card.
 * We need to use flash_status_poll.
 */
#define CONFIG_SYS_CFI_FLASH_STATUS_POLL

#define CONFIG_FLASH_SHOW_PROGRESS	45 /* count down from 45/5: 9..1 */

#define CONFIG_SYS_MAX_FLASH_BANKS_DETECT 1

/* serial console configuration */

#define CONFIG_SYS_CBSIZE		1024	/* Console I/O Buffer Size */
/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE		(CONFIG_SYS_CBSIZE)

#define CONFIG_ENV_OFFSET			0x100000
#define CONFIG_ENV_SIZE				0x2000
/* #define CONFIG_ENV_OFFSET_REDUND	(CONFIG_ENV_OFFSET + CONFIG_ENV_SIZE) */

#define CONFIG_SYS_MMC_ENV_DEV		0
#define CONFIG_SYS_MMC_ENV_PART		1

#if !defined(CONFIG_ARM64)
/* Time clock 1MHz */
#define CONFIG_SYS_TIMER_RATE			1000000
#endif

#define CONFIG_SYS_MAX_NAND_DEVICE			1
#define CONFIG_SYS_NAND_ONFI_DETECTION
#define CONFIG_SYS_NAND_REGS_BASE			0x68100000
#define CONFIG_SYS_NAND_DATA_BASE			0x68000000
#define CONFIG_SYS_NAND_BAD_BLOCK_POS			0

/* memtest works on */
#define CONFIG_SYS_MEMTEST_START	CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_SDRAM_BASE + 0x01000000)

/*
 * Network Configuration
 */
#define CONFIG_SERVERIP			192.168.11.1
#define CONFIG_IPADDR			192.168.11.10
#define CONFIG_GATEWAYIP		192.168.11.1
#define CONFIG_NETMASK			255.255.255.0

#define CONFIG_LOADADDR			0x85000000
#define CONFIG_SYS_LOAD_ADDR		CONFIG_LOADADDR
#define CONFIG_SYS_BOOTM_LEN		(32 << 20)

#if defined(CONFIG_ARM64)
/* ARM Trusted Firmware */
#define BOOT_IMAGES \
	"second_image=unph_bl.bin\0" \
	"third_image=fip.bin\0"
#else
#define BOOT_IMAGES \
	"second_image=u-boot-spl.bin\0" \
	"third_image=u-boot.bin\0"
#endif

#define CONFIG_ROOTPATH			"/nfs/root/path"
#define CONFIG_NFSBOOTCOMMAND						\
	"setenv bootargs $bootargs root=/dev/nfs rw "			\
	"nfsroot=$serverip:$rootpath "					\
	"ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:$netdev:off;" \
		"run __nfsboot"

#ifdef CONFIG_FIT
#define CONFIG_BOOTFILE			"fitImage"
#define LINUXBOOT_ENV_SETTINGS \
	"kernel_addr_r=0x85100000\0" \
	"tftpboot=tftpboot $kernel_addr_r $bootfile &&" \
		"bootm $kernel_addr_r\0" \
	"__nfsboot=run tftpboot\0"
#else
#ifdef CONFIG_ARM64
#define CONFIG_BOOTFILE			"Image"
#define LINUXBOOT_CMD			"booti"
#define KERNEL_ADDR_R			"kernel_addr_r=0x82080000\0"
#else
#define CONFIG_BOOTFILE			"zImage"
#define LINUXBOOT_CMD			"bootz"
#define KERNEL_ADDR_R			"kernel_addr_r=0x80208000\0"
#endif
#define LINUXBOOT_ENV_SETTINGS \
	"fdt_addr_r=0x85100000\0" \
	KERNEL_ADDR_R \
	"ramdisk_addr_r=0x86000000\0" \
	"ramdisk_file=rootfs.cpio.gz\0" \
	"boot_common=setexpr bootm_low $kernel_addr_r '&' fe000000 && " \
		LINUXBOOT_CMD " $kernel_addr_r $ramdisk_addr_r $fdt_addr_r\0" \
	"tftpboot=tftpboot $kernel_addr_r $bootfile && " \
		"tftpboot $fdt_addr_r $fdtfile &&" \
		"tftpboot $ramdisk_addr_r $ramdisk_file &&" \
		"setenv ramdisk_addr_r $ramdisk_addr_r:$filesize &&" \
		"run boot_common\0" \
	"__nfsboot=tftpboot $kernel_addr_load $bootfile && " \
		"tftpboot $fdt_addr_r $fdtfile &&" \
		"setenv ramdisk_addr_r - &&" \
		"run boot_common\0"
#endif

#define	CONFIG_EXTRA_ENV_SETTINGS				\
	"netdev=eth0\0"						\
	"initrd_high=0xffffffffffffffff\0"			\
	"script=boot.scr\0" \
	"scriptaddr=0x85000000\0"				\
	"nor_base=0x42000000\0"					\
	"emmcboot=mmcsetn && run bootcmd_mmc${mmc_first_dev}\0" \
	"nandboot=run bootcmd_ubifs0\0" \
	"norboot=run tftpboot\0" \
	"usbboot=run bootcmd_usb0\0" \
	"emmcscript=setenv devtype mmc && " \
		"mmcsetn && " \
		"setenv devnum ${mmc_first_dev} && " \
		"run loadscript_fat\0" \
	"nandscript=echo Running ${script} from ubi ... && " \
		"ubi part UBI && " \
		"ubifsmount ubi0:boot && " \
		"ubifsload ${loadaddr} ${script} && " \
		"source\0" \
	"norscript=echo Running ${script} from tftp ... && " \
		"tftpboot ${script} &&" \
		"source\0" \
	"usbscript=usb start && " \
		"setenv devtype usb && " \
		"setenv devnum 0 && " \
		"run loadscript_fat\0" \
	"loadscript_fat=echo Running ${script} from ${devtype}${devnum} ... && " \
		"load ${devtype} ${devnum}:1 ${loadaddr} ${script} && " \
		"source\0" \
	"sramupdate=setexpr tmp_addr $nor_base + 0x50000 &&"	\
		"tftpboot $tmp_addr $second_image && " \
		"setexpr tmp_addr $nor_base + 0x70000 && " \
		"tftpboot $tmp_addr $third_image\0" \
	"emmcupdate=mmcsetn &&"					\
		"mmc dev $mmc_first_dev &&"			\
		"mmc partconf $mmc_first_dev 0 1 1 &&"		\
		"tftpboot $second_image && " \
		"mmc write $loadaddr 0 100 && " \
		"tftpboot $third_image && " \
		"mmc write $loadaddr 100 f00\0" \
	"nandupdate=nand erase 0 0x00100000 &&"			\
		"tftpboot $second_image && " \
		"nand write $loadaddr 0 0x00020000 && " \
		"tftpboot $third_image && " \
		"nand write $loadaddr 0x00020000 0x001e0000\0" \
	"usbupdate=usb start &&" \
		"tftpboot $second_image && " \
		"usb write $loadaddr 0 100 && " \
		"tftpboot $third_image && " \
		"usb write $loadaddr 100 f00\0" \
	BOOT_IMAGES \
	LINUXBOOT_ENV_SETTINGS \
	BOOTENV

#define CONFIG_SYS_BOOTMAPSZ			0x20000000

#define CONFIG_SYS_SDRAM_BASE		0x80000000

#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_TEXT_BASE)

/* only for SPL */
#if defined(CONFIG_ARCH_UNIPHIER_LD4) || \
	defined(CONFIG_ARCH_UNIPHIER_SLD8)
#endif

#define CONFIG_SPL_STACK		(0x00200000)

#define CONFIG_SYS_NAND_U_BOOT_OFFS		0x20000

/* subtract sizeof(struct image_header) */
#define CONFIG_SYS_UBOOT_BASE			(0x130000 - 0x40)

#define CONFIG_SPL_TARGET			"u-boot-with-spl.bin"
#define CONFIG_SPL_MAX_FOOTPRINT		0x10000
#define CONFIG_SPL_MAX_SIZE			0x10000
#define CONFIG_SPL_BSS_MAX_SIZE			0x2000

#define CONFIG_SPL_PAD_TO			0x20000

#endif /* __CONFIG_UNIPHIER_COMMON_H__ */
