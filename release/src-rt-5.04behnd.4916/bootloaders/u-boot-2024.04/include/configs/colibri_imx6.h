/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2013-2019 Toradex, Inc.
 *
 * Configuration settings for the Toradex Colibri iMX6
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <linux/stringify.h>

#include "mx6_common.h"

#include <asm/arch/imx-regs.h>
#include <asm/mach-imx/gpio.h>

#define CFG_MXC_UART_BASE		UART1_BASE

/* MMC Configs */
#define CFG_SYS_FSL_ESDHC_ADDR	0
#define CFG_SYS_FSL_USDHC_NUM	2

/* USB Configs */
/* Host */
#define CFG_MXC_USB_PORTSC		(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CFG_MXC_USB_FLAGS		0

/* Command definition */

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 1) \
	func(MMC, mmc, 0) \
	func(USB, usb, 0) \
	func(DHCP, dhcp, na)
#include <config_distro_bootcmd.h>
#undef BOOTENV_RUN_NET_USB_START
#define BOOTENV_RUN_NET_USB_START ""

#define UBOOT_UPDATE \
	"uboot_hwpart=1\0" \
	"uboot_blk=8a\0" \
	"uboot_spl_blk=2\0" \
	"set_blkcnt=setexpr blkcnt ${filesize} + 0x1ff && " \
		"setexpr blkcnt ${blkcnt} / 0x200\0" \
	"update_uboot=run set_blkcnt && mmc dev 0 ${uboot_hwpart} && " \
		"mmc write ${loadaddr} ${uboot_blk} ${blkcnt}\0" \
	"update_spl=run set_blkcnt && mmc dev 0 ${uboot_hwpart} && " \
		"mmc write ${loadaddr} ${uboot_spl_blk} ${blkcnt}\0"

#define MEM_LAYOUT_ENV_SETTINGS \
	"bootm_size=0x10000000\0" \
	"fdt_addr_r=0x18200000\0" \
	"kernel_addr_r=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
	"pxefile_addr_r=0x18300000\0" \
	"ramdisk_addr_r=0x18400000\0" \
	"scriptaddr=0x18280000\0"

#define CFG_EXTRA_ENV_SETTINGS \
	BOOTENV \
	"boot_file=zImage\0" \
	"boot_script_dhcp=boot.scr\0" \
	"console=ttymxc0\0" \
	"defargs=enable_wait_mode=off galcore.contiguousSize=50331648\0" \
	"fdt_board=eval-v3\0" \
	"fdt_fixup=;\0" \
	MEM_LAYOUT_ENV_SETTINGS \
	UBOOT_UPDATE \
	"setethupdate=if env exists ethaddr; then; else setenv ethaddr " \
		"00:14:2d:00:00:00; fi; tftpboot ${loadaddr} " \
		"flash_eth.img && source ${loadaddr}\0" \
	"setsdupdate=setenv interface mmc; setenv drive 1; mmc rescan; load " \
		"${interface} ${drive}:1 ${loadaddr} flash_blk.img && " \
		"source ${loadaddr}\0" \
	"setup=setenv setupargs fec_mac=${ethaddr} " \
		"consoleblank=0 no_console_suspend=1 console=tty1 " \
		"console=${console},${baudrate}n8\0 " \
	"setupdate=run setsdupdate || run setusbupdate || run setethupdate\0" \
	"setusbupdate=usb start && setenv interface usb; setenv drive 0; " \
		"load ${interface} ${drive}:1 ${loadaddr} flash_blk.img && " \
		"source ${loadaddr}\0" \
	"splashpos=m,m\0" \
	"splashimage=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
	"vidargs=fbmem=8M\0"

/* Miscellaneous configurable options */

/* Physical Memory Map */
#define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR

#define CFG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CFG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CFG_SYS_INIT_RAM_SIZE	IRAM_SIZE

#endif	/* __CONFIG_H */
