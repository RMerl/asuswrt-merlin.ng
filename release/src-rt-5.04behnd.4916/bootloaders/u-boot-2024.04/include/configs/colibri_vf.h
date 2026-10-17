/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2015-2019 Toradex, Inc.
 *
 * Configuration settings for the Toradex VF50/VF61 modules.
 *
 * Based on vf610twr.h:
 * Copyright 2013 Freescale Semiconductor, Inc.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <asm/arch/imx-regs.h>
#include <linux/sizes.h>

#define MEM_LAYOUT_ENV_SETTINGS \
	"bootm_size=0x10000000\0" \
	"fdt_addr_r=0x82000000\0" \
	"kernel_addr_r=0x81000000\0" \
	"pxefile_addr_r=0x87100000\0" \
	"ramdisk_addr_r=0x82100000\0" \
	"scriptaddr=0x87000000\0"

#define UBOOT_UPDATE \
	"update_uboot=nand erase.part u-boot && " \
		"nand write ${loadaddr} u-boot ${filesize}\0" \

#define UBI_BOOTCMD \
	"ubiargs=ubi.mtd=ubi root=ubi0:rootfs rootfstype=ubifs " \
	"ubi.fm_autoconvert=1\0" \
	"ubiboot=run setup; " \
	"setenv bootargs ${defargs} ${ubiargs} ${mtdparts} "   \
	"${setupargs} ${vidargs} ${tdxargs}; echo Booting from NAND...; " \
	"ubi part ubi && " \
	"ubi read ${kernel_addr_r} kernel && " \
	"ubi read ${fdt_addr_r} dtb && " \
	"run fdt_fixup && bootz ${kernel_addr_r} - ${fdt_addr_r}\0" \

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(USB, usb, 0) \
	func(DHCP, dhcp, na)
#include <config_distro_bootcmd.h>
#undef BOOTENV_RUN_NET_USB_START
#define BOOTENV_RUN_NET_USB_START ""

#define DFU_ALT_NAND_INFO "vf-bcb part 0,1;u-boot part 0,2;ubi part 0,4"

#define CFG_EXTRA_ENV_SETTINGS \
	BOOTENV \
	MEM_LAYOUT_ENV_SETTINGS \
	UBI_BOOTCMD \
	UBOOT_UPDATE \
	"boot_script_dhcp=boot.scr\0" \
	"console=ttyLP0\0" \
	"defargs=user_debug=30\0" \
	"dfu_alt_info=" DFU_ALT_NAND_INFO "\0" \
	"fdt_board=eval-v3\0" \
	"fdt_fixup=;\0" \
	"kernel_image=zImage\0" \
	"setsdupdate=mmc rescan && set interface mmc && " \
		"fatload ${interface} 0:1 ${loadaddr} flash_blk.img && " \
		"source ${loadaddr}\0" \
	"setup=setenv setupargs console=tty1 console=${console}" \
		",${baudrate}n8 ${memargs}\0" \
	"setupdate=run setsdupdate || run setusbupdate\0" \
	"setusbupdate=usb start && set interface usb && " \
		"fatload ${interface} 0:1 ${loadaddr} flash_blk.img && " \
		"source ${loadaddr}\0" \
	"splashpos=m,m\0" \
	"video-mode=dcufb:640x480-16@60,monitor=lcd\0"

/* Miscellaneous configurable options */

/* Physical memory map */
#define PHYS_SDRAM			(0x80000000)
#define PHYS_SDRAM_SIZE			(256 * SZ_1M)

#define CFG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CFG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CFG_SYS_INIT_RAM_SIZE	IRAM_SIZE

/* USB Host Support */

/* USB DFU */

#endif /* __CONFIG_H */
