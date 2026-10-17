/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2018-2021 Toradex AG
 *
 * Configuration settings for the Colibri iMX6ULL module.
 *
 * based on colibri_imx7.h
 */

#ifndef __COLIBRI_IMX6ULL_CONFIG_H
#define __COLIBRI_IMX6ULL_CONFIG_H

#include "mx6_common.h"

#define PHYS_SDRAM_SIZE			SZ_1G

/* ENET1 */

/* MMC Config */
#define CFG_SYS_FSL_ESDHC_ADDR	0
#define CFG_SYS_FSL_USDHC_NUM	1

#if defined(CONFIG_TARGET_COLIBRI_IMX6ULL_EMMC)
#define UBOOT_UPDATE \
	"uboot_hwpart=1\0" \
	"uboot_blk=2\0" \
	"set_blkcnt=setexpr blkcnt ${filesize} + 0x1ff && " \
		"setexpr blkcnt ${blkcnt} / 0x200\0" \
	"update_uboot=run set_blkcnt && mmc dev 0 ${uboot_hwpart} && " \
		"mmc write ${loadaddr} ${uboot_blk} ${blkcnt}\0"
#elif defined(CONFIG_TARGET_COLIBRI_IMX6ULL_NAND)
#define UBOOT_UPDATE \
	"update_uboot=nand erase.part u-boot1 && " \
		"nand write ${loadaddr} u-boot1 ${filesize} && " \
		"nand erase.part u-boot2 && " \
		"nand write ${loadaddr} u-boot2 ${filesize}\0"
#endif

#define MEM_LAYOUT_ENV_SETTINGS \
	"bootm_size=0x10000000\0" \
	"fdt_addr_r=0x88200000\0" \
	"kernel_addr_r=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
	"pxefile_addr_r=0x88300000\0" \
	"ramdisk_addr_r=0x88400000\0" \
	"scriptaddr=0x88280000\0"

#define UBI_BOOTCMD \
	"ubiargs=ubi.mtd=ubi root=ubi0:rootfs rw rootfstype=ubifs " \
		"ubi.fm_autoconvert=1\0" \
	"ubiboot=run setup; " \
		"setenv bootargs ${defargs} ${ubiargs} " \
		"${setupargs} ${vidargs} ${tdxargs}; echo Booting from NAND...; " \
		"ubi part ubi &&" \
		"ubi read ${kernel_addr_r} kernel && " \
		"ubi read ${fdt_addr_r} dtb && " \
		"run fdt_fixup && bootz ${kernel_addr_r} - ${fdt_addr_r}\0" \

#if defined(CONFIG_TARGET_COLIBRI_IMX6ULL_NAND)
/* Run Distro Boot script if ubiboot fails */
#define DFU_ALT_NAND_INFO "imx6ull-bcb part 0,1;u-boot1 part 0,2;u-boot2 part 0,3;u-boot-env part 0,4;ubi partubi 0,5"
#define MODULE_EXTRA_ENV_SETTINGS \
	"dfu_alt_info=" DFU_ALT_NAND_INFO "\0" \
	UBI_BOOTCMD
#else
#define	MODULE_EXTRA_ENV_SETTINGS ""
#endif

#if defined(CONFIG_TARGET_COLIBRI_IMX6ULL_NAND)
#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(USB, usb, 0) \
	func(DHCP, dhcp, na)
#elif defined(CONFIG_TARGET_COLIBRI_IMX6ULL_EMMC)
#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 1) \
	func(MMC, mmc, 0) \
	func(USB, usb, 0) \
	func(DHCP, dhcp, na)
#endif
#include <config_distro_bootcmd.h>

#define CFG_EXTRA_ENV_SETTINGS \
	BOOTENV \
	MEM_LAYOUT_ENV_SETTINGS \
	UBI_BOOTCMD \
	UBOOT_UPDATE \
	"boot_script_dhcp=boot.scr\0" \
	"console=ttymxc0\0" \
	"defargs=user_debug=30\0" \
	"fdt_board=eval-v3\0" \
	"fdt_fixup=;\0" \
	"ip_dyn=yes\0" \
	"kernel_file=zImage\0" \
	"setethupdate=if env exists ethaddr; then; else setenv ethaddr " \
		"00:14:2d:00:00:00; fi; tftpboot ${loadaddr} " \
		"${board}/flash_eth.img && source ${loadaddr}\0" \
	"setsdupdate=mmc rescan && setenv interface mmc && " \
		"fatload ${interface} 0:1 ${loadaddr} " \
		"${board}/flash_blk.img && source ${loadaddr}\0" \
	"setup=setenv setupargs " \
		"console=tty1 console=${console}" \
		",${baudrate}n8 ${memargs} ${mtdparts} consoleblank=0\0" \
	"setupdate=run setsdupdate || run setusbupdate || run setethupdate\0" \
	"setusbupdate=usb start && setenv interface usb && " \
		"fatload ${interface} 0:1 ${loadaddr} " \
		"${board}/flash_blk.img && source ${loadaddr}\0" \
	"splashpos=m,m\0" \
	"splashimage=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
	"videomode=video=ctfb:x:640,y:480,depth:18,pclk:39722,le:48,ri:16,up:33,lo:10,hs:96,vs:2,sync:0,vmode:0\0"

/* Physical Memory Map */
#define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR

#define CFG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CFG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CFG_SYS_INIT_RAM_SIZE	IRAM_SIZE

#ifdef CONFIG_TARGET_COLIBRI_IMX6ULL_NAND
/* NAND stuff */
/* used to initialize CFG_SYS_NAND_BASE_LIST which is unused */
#define CFG_SYS_NAND_BASE		-1
#endif

/* USB Configs */

#define CFG_MXC_USB_PORTSC		(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CFG_MXC_USB_FLAGS		0

/* USB Device Firmware Update support */
#define DFU_DEFAULT_POLL_TIMEOUT	300

#if defined(CONFIG_VIDEO)
#define MXS_LCDIF_BASE MX6UL_LCDIF1_BASE_ADDR
#endif

#endif /* __COLIBRI_IMX6ULL_CONFIG_H */
