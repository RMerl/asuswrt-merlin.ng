/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2013 Freescale Semiconductor, Inc.
 *
 * Configuration settings for the Wandboard.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include "mx6_common.h"

#define CFG_MXC_UART_BASE		UART1_BASE

/* MMC Configuration */
#define CFG_SYS_FSL_USDHC_NUM	2
#define CFG_SYS_FSL_ESDHC_ADDR	0

/* USB Configs */
#define CFG_MXC_USB_PORTSC		(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CFG_MXC_USB_FLAGS		0

#define CFG_EXTRA_ENV_SETTINGS \
	"console=ttymxc0\0" \
	"splashpos=m,m\0" \
	"splashimage=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
	"fdtfile=undefined\0" \
	"fdt_high=0xffffffff\0" \
	"initrd_high=0xffffffff\0" \
	"fdt_addr_r=0x18000000\0" \
	"fdt_addr=0x18000000\0" \
	"ip_dyn=yes\0" \
	"mmcdev=" __stringify(CONFIG_SYS_MMC_ENV_DEV) "\0" \
	"finduuid=part uuid mmc 0:1 uuid\0" \
	"update_sd_firmware_filename=u-boot.imx\0" \
	"update_sd_firmware=" \
		"if test ${ip_dyn} = yes; then " \
			"setenv get_cmd dhcp; " \
		"else " \
			"setenv get_cmd tftp; " \
		"fi; " \
		"if mmc dev ${mmcdev}; then "	\
			"if ${get_cmd} ${update_sd_firmware_filename}; then " \
				"setexpr fw_sz ${filesize} / 0x200; " \
				"setexpr fw_sz ${fw_sz} + 1; "	\
				"mmc write ${loadaddr} 0x2 ${fw_sz}; " \
			"fi; "	\
		"fi\0" \
	"findfdt="\
		"if test $board_name = D1 && test $board_rev = MX6QP ; then " \
			"setenv fdtfile imx6qp-wandboard-revd1.dtb; fi; " \
		"if test $board_name = D1 && test $board_rev = MX6Q ; then " \
			"setenv fdtfile imx6q-wandboard-revd1.dtb; fi; " \
		"if test $board_name = D1 && test $board_rev = MX6DL ; then " \
			"setenv fdtfile imx6dl-wandboard-revd1.dtb; fi; " \
		"if test $board_name = C1 && test $board_rev = MX6Q ; then " \
			"setenv fdtfile imx6q-wandboard.dtb; fi; " \
		"if test $board_name = C1 && test $board_rev = MX6DL ; then " \
			"setenv fdtfile imx6dl-wandboard.dtb; fi; " \
		"if test $board_name = B1 && test $board_rev = MX6Q ; then " \
			"setenv fdtfile imx6q-wandboard-revb1.dtb; fi; " \
		"if test $board_name = B1 && test $board_rev = MX6DL ; then " \
			"setenv fdtfile imx6dl-wandboard-revb1.dtb; fi; " \
		"if test $fdtfile = undefined; then " \
			"echo WARNING: Could not determine dtb to use; fi; \0" \
	"kernel_addr_r=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
	"pxefile_addr_r=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
	"ramdisk_addr_r=0x13000000\0" \
	"ramdiskaddr=0x13000000\0" \
	"scriptaddr=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
	BOOTENV

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(MMC, mmc, 1) \
	func(MMC, mmc, 2) \
	func(SATA, sata, 0) \
	func(USB, usb, 0) \
	func(PXE, pxe, na) \
	func(DHCP, dhcp, na)

#include <config_distro_bootcmd.h>
#include <linux/stringify.h>

/* Physical Memory Map */
#define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR

#define CFG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CFG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CFG_SYS_INIT_RAM_SIZE	IRAM_SIZE

/* Environment organization */

#endif			       /* __CONFIG_H * */
