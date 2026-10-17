/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2013 Freescale Semiconductor, Inc.
 *
 * Configuration settings for the pico-imx6 board.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include "mx6_common.h"

#define CFG_MXC_UART_BASE		UART1_BASE

/* MMC Configuration */
#define CFG_SYS_FSL_ESDHC_ADDR	USDHC3_BASE_ADDR

/* USB Configs */
#define CFG_MXC_USB_PORTSC		(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CFG_MXC_USB_FLAGS		0

#define DFU_DEFAULT_POLL_TIMEOUT 300

#define CFG_DFU_ENV_SETTINGS \
	"dfu_alt_info=" \
		"spl raw 0x2 0x400;" \
		"u-boot raw 0x8a 0x1000;" \
		"/boot/zImage ext4 0 1;" \
		"rootfs part 0 1\0" \

#define BOOTMENU_ENV \
	"bootmenu_0=Boot using PICO-Hobbit baseboard=" \
		"setenv baseboard hobbit; saveenv; run base_boot\0" \
	"bootmenu_1=Boot using PICO-Pi baseboard=" \
		"setenv baseboard pi; saveenv; run base_boot\0" \
	"bootmenu_2=Boot using PICO-Dwarf baseboard=" \
		"setenv baseboard dwarf; saveenv; run base_boot\0" \
	"bootmenu_3=Boot using PICO-Nymph baseboard=" \
		"setenv baseboard nymph; saveenv; run base_boot\0" \

#define CFG_EXTRA_ENV_SETTINGS \
	"console=ttymxc0\0" \
	"fdtfile=" CONFIG_DEFAULT_FDT_FILE "\0" \
	BOOTMENU_ENV \
	"fdt_high=0xffffffff\0" \
	"initrd_high=0xffffffff\0" \
	"fdt_addr_r=0x18000000\0" \
	"fdt_addr=0x18000000\0" \
	"mmcdev=" __stringify(CONFIG_SYS_MMC_ENV_DEV) "\0" \
	CFG_DFU_ENV_SETTINGS \
	"finduuid=part uuid mmc 0:1 uuid\0" \
	"findfdt="\
		"if test $baseboard = hobbit && test $board_rev = MX6Q ; then " \
			"setenv fdtfile imx6q-pico-hobbit.dtb; fi; " \
		"if test $baseboard = pi && test $board_rev = MX6Q ; then " \
			"setenv fdtfile imx6q-pico-pi.dtb; fi; " \
		"if test $baseboard = dwarf && test $board_rev = MX6Q ; then " \
			"setenv fdtfile imx6q-pico-dwarf.dtb; fi; " \
		"if test $baseboard = nymph && test $board_rev = MX6Q ; then " \
			"setenv fdtfile imx6q-pico-nymph.dtb; fi; " \
		"if test $baseboard = hobbit && test $board_rev = MX6DL ; then " \
			"setenv fdtfile imx6dl-pico-hobbit.dtb; fi; " \
		"if test $baseboard = pi && test $board_rev = MX6DL ; then " \
			"setenv fdtfile imx6dl-pico-pi.dtb; fi; " \
		"if test $baseboard = dwarf && test $board_rev = MX6DL ; then " \
			"setenv fdtfile imx6dl-pico-dwarf.dtb; fi; " \
		"if test $baseboard = nymph && test $board_rev = MX6DL ; then " \
			"setenv fdtfile imx6dl-pico-nymph.dtb; fi; " \
		"if test $fdtfile = ask; then " \
			"echo WARNING: Could not determine dtb to use; fi; \0" \
	"default_boot=" \
		"if test $baseboard = ask ; then " \
			"bootmenu -1; " \
		"else " \
			"run base_boot;" \
		"fi; \0" \
	"base_boot=run findfdt; run finduuid; run distro_bootcmd\0" \
	"kernel_addr_r=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
	"pxefile_addr_r=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
	"ramdisk_addr_r=0x13000000\0" \
	"ramdiskaddr=0x13000000\0" \
	"scriptaddr=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
	BOOTENV

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(USB, usb, 0)

#include <config_distro_bootcmd.h>

/* Physical Memory Map */
#define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR

#define CFG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CFG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CFG_SYS_INIT_RAM_SIZE	IRAM_SIZE

/* Environment organization */

/* Ethernet Configuration */
#define CFG_FEC_MXC_PHYADDR		1

#endif			       /* __CONFIG_H * */
