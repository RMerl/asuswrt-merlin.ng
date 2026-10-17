/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2017 Toradex, Inc.
 *
 * Configuration settings for the Toradex Apalis TK1 modules.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <linux/sizes.h>

#include "tegra124-common.h"

/* Board-specific serial config */
#define CFG_SYS_NS16550_COM1		NV_PA_APB_UARTA_BASE

#define FDT_MODULE			"apalis-v1.2"
#define FDT_MODULE_V1_0			"apalis"

/*
 * Custom Distro Boot configuration:
 * 1. 8bit SD port (MMC1)
 * 2. 4bit SD port (MMC2)
 * 3. eMMC (MMC0)
 */
#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 1) \
	func(MMC, mmc, 2) \
	func(MMC, mmc, 0) \
	func(USB, usb, 0) \
	func(PXE, pxe, na) \
	func(DHCP, dhcp, na)

#define DFU_ALT_EMMC_INFO	"apalis-tk1.img raw 0x0 0x500 mmcpart 1; " \
				"boot part 0 1 mmcpart 0; " \
				"rootfs part 0 2 mmcpart 0; " \
				"zImage fat 0 1 mmcpart 0; " \
				"tegra124-apalis-eval.dtb fat 0 1 mmcpart 0"

#define UBOOT_UPDATE \
	"uboot_hwpart=1\0" \
	"uboot_blk=0\0" \
	"set_blkcnt=setexpr blkcnt ${filesize} + 0x1ff && " \
		"setexpr blkcnt ${blkcnt} / 0x200\0" \
	"update_uboot=run set_blkcnt && mmc dev 0 ${uboot_hwpart} && " \
		"mmc write ${loadaddr} ${uboot_blk} ${blkcnt}\0" \

#define BOARD_EXTRA_ENV_SETTINGS \
	"boot_file=zImage\0" \
	"boot_script_dhcp=boot.scr\0" \
	"console=ttyS0\0" \
	"defargs=lp0_vec=2064@0xf46ff000 core_edp_mv=1150 core_edp_ma=4000 " \
		"usb_port_owner_info=2 lane_owner_info=6 emc_max_dvfs=0 " \
		"user_debug=30 pcie_aspm=off\0" \
	"dfu_alt_info=" DFU_ALT_EMMC_INFO "\0" \
	"fdt_board=eval\0" \
	"fdt_fixup=;\0" \
	"fdt_module=" FDT_MODULE "\0" \
	UBOOT_UPDATE \
	"setethupdate=if env exists ethaddr; then; else setenv ethaddr " \
		"00:14:2d:00:00:00; fi; pci enum && tftpboot ${loadaddr} " \
		"flash_eth.img && source ${loadaddr}\0" \
	"setsdupdate=setenv interface mmc; setenv drive 1; mmc rescan; " \
		"load ${interface} ${drive}:1 ${loadaddr} flash_blk.img " \
		"|| setenv drive 2; mmc rescan; load ${interface} ${drive}:1 " \
		"${loadaddr} flash_blk.img && " \
		"source ${loadaddr}\0" \
	"setup=setenv setupargs igb_mac=${ethaddr} " \
		"consoleblank=0 no_console_suspend=1 console=tty1 " \
		"console=${console},${baudrate}n8 debug_uartport=lsport,0 " \
		"${memargs}\0" \
	"setupdate=run setsdupdate || run setusbupdate || run setethupdate\0" \
	"setusbupdate=usb start && setenv interface usb; setenv drive 0; " \
		"load ${interface} ${drive}:1 ${loadaddr} flash_blk.img && " \
		"source ${loadaddr}\0" \
	"vidargs=fbcon=map:1\0"

#include "tegra-common-post.h"

#endif /* __CONFIG_H */
