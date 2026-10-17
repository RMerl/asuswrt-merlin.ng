/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2012-2015 Panasonic Corporation
 * Copyright (C) 2015-2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

/* U-Boot - Common settings for UniPhier Family */

#ifndef __CONFIG_UNIPHIER_H__
#define __CONFIG_UNIPHIER_H__

#include <config_distro_bootcmd.h>

#ifdef CONFIG_CMD_MMC
#define BOOT_TARGET_DEVICE_MMC(func)	func(MMC, mmc, 0) func(MMC, mmc, 1)
#else
#define BOOT_TARGET_DEVICE_MMC(func)
#endif

#ifdef CONFIG_CMD_UBIFS
#define BOOT_TARGET_DEVICE_UBIFS(func)	func(UBIFS, ubifs, 0, UBI, boot)
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

#if !defined(CONFIG_ARM64)
#define CFG_SYS_HZ_CLOCK			50000000
#endif

#define CFG_SYS_NAND_REGS_BASE			0x68100000
#define CFG_SYS_NAND_DATA_BASE			0x68000000

/*
 * Network Configuration
 */

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

#ifdef CONFIG_FIT
#define KERNEL_ADDR_R_OFFSET		"0x05100000"
#define LINUXBOOT_ENV_SETTINGS \
	"tftpboot=tftpboot $kernel_addr_r $bootfile &&" \
		"bootm $kernel_addr_r\0" \
	"__nfsboot=run tftpboot\0"
#else
#ifdef CONFIG_ARM64
#define LINUXBOOT_CMD			"booti"
#define KERNEL_ADDR_R_OFFSET		"0x02080000"
#else
#define LINUXBOOT_CMD			"bootz"
#define KERNEL_ADDR_R_OFFSET		"0x00208000"
#endif
#define LINUXBOOT_ENV_SETTINGS \
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

#define	CFG_EXTRA_ENV_SETTINGS				\
	"fdt_addr_r_offset=0x05100000\0" \
	"kernel_addr_r_offset=" KERNEL_ADDR_R_OFFSET "\0" \
	"ramdisk_addr_r_offset=0x06000000\0" \
	"ramdisk_file=rootfs.cpio.gz\0" \
	"netdev=eth0\0"						\
	"initrd_high=0xffffffffffffffff\0"			\
	"loadaddr_offset=0x05000000\0" \
	"script=boot.scr\0" \
	"scriptaddr=0x85000000\0"				\
	"nor_base=0x42000000\0"					\
	"emmcboot=mmcsetn && run bootcmd_mmc${mmc_first_dev}\0" \
	"nandboot=run bootcmd_ubifs0\0" \
	"norboot=run tftpboot\0" \
	"sdboot=sdsetn && run bootcmd_mmc${sd_first_dev}\0" \
	"usbboot=run bootcmd_usb0\0" \
	"emmcscript=setenv devtype mmc && " \
		"mmcsetn && " \
		"setenv devnum ${mmc_first_dev} && " \
		"run loadscript_fat\0" \
	"nandscript=echo Running ${script} from ubi ... && " \
		"ubi part UBI && " \
		"ubifsmount ubi0:boot && " \
		"ubifsload ${loadaddr} ${script} && " \
		"source $loadaddr\0" \
	"sdscript=setenv devtype mmc && " \
		"sdsetn && " \
		"setenv devnum ${sd_first_dev} && " \
		"run loadscript_fat\0" \
	"norscript=echo Running ${script} from tftp ... && " \
		"tftpboot ${script} &&" \
		"source $loadaddr\0" \
	"usbscript=usb start && " \
		"setenv devtype usb && " \
		"setenv devnum 0 && " \
		"run loadscript_fat\0" \
	"loadscript_fat=echo Running ${script} from ${devtype}${devnum} ... && " \
		"load ${devtype} ${devnum}:1 ${loadaddr} ${script} && " \
		"source $loadaddr\0" \
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
	"sdupdate=sdsetn &&" \
		"mmc dev $sd_first_dev &&" \
		"tftpboot $second_image && " \
		"mmc write $loadaddr 0 100 && " \
		"tftpboot $third_image && " \
		"mmc write $loadaddr 100 f00\0" \
	"usbupdate=usb start &&" \
		"tftpboot $second_image && " \
		"usb write $loadaddr 0 100 && " \
		"tftpboot $third_image && " \
		"usb write $loadaddr 100 f00\0" \
	BOOT_IMAGES \
	LINUXBOOT_ENV_SETTINGS \
	BOOTENV

#define CFG_SYS_BOOTMAPSZ			0x20000000

/* only for SPL */

/* subtract sizeof(struct legacy_img_hdr) */
#define CFG_SYS_UBOOT_BASE			(0x130000 - 0x40)

#endif /* __CONFIG_UNIPHIER_H__ */
