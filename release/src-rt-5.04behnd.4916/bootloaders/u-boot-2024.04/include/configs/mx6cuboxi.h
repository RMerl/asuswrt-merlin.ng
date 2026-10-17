/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015 Freescale Semiconductor, Inc.
 *
 * Configuration settings for the SolidRun mx6 based boards
 */
#ifndef __MX6CUBOXI_CONFIG_H
#define __MX6CUBOXI_CONFIG_H

#include <linux/stringify.h>

#include "mx6_common.h"

/* MMC Configs */
#define CFG_SYS_FSL_ESDHC_ADDR	USDHC2_BASE_ADDR

/* USB */
#define CFG_MXC_USB_PORTSC		(PORT_PTS_UTMI | PORT_PTS_PTW)

/* Command definition */

#define CFG_MXC_UART_BASE	UART1_BASE

#define CFG_EXTRA_ENV_SETTINGS \
	"som_rev=undefined\0" \
	"has_emmc=undefined\0" \
	"fdtfile=undefined\0" \
	"fdt_addr_r=0x18000000\0" \
	"fdt_addr=0x18000000\0" \
	"kernel_addr_r=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0"  \
	"pxefile_addr_r=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
	"scriptaddr=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
	"ramdisk_addr_r=0x13000000\0" \
	"ramdiskaddr=0x13000000\0" \
	"initrd_high=0xffffffff\0" \
	"ip_dyn=yes\0" \
	"console=ttymxc0\0" \
	"bootm_size=0x10000000\0" \
	"mmcdev=" __stringify(CONFIG_SYS_MMC_ENV_DEV) "\0" \
	"finduuid=part uuid mmc 1:1 uuid\0" \
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
		"if test ${board_rev} = MX6Q; then " \
			"setenv fdtprefix imx6q; fi; " \
		"if test ${board_rev} = MX6DL; then " \
			"setenv fdtprefix imx6dl; fi; " \
		"if test ${som_rev} = V15; then " \
			"setenv fdtsuffix -som-v15; fi; " \
		"if test ${has_emmc} = yes; then " \
			"setenv emmcsuffix -emmc; fi; " \
		"if test ${board_name} = HUMMINGBOARD2 ; then " \
			"setenv fdtfile ${fdtprefix}-hummingboard2${emmcsuffix}${fdtsuffix}.dtb; fi; " \
		"if test ${board_name} = HUMMINGBOARD ; then " \
			"setenv fdtfile ${fdtprefix}-hummingboard${emmcsuffix}${fdtsuffix}.dtb; fi; " \
		"if test ${board_name} = CUBOXI ; then " \
			"setenv fdtfile ${fdtprefix}-cubox-i${emmcsuffix}${fdtsuffix}.dtb; fi; " \
		"if test ${fdtfile} = undefined; then " \
			"echo WARNING: Could not determine dtb to use; fi; \0" \
	BOOTENV

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 1) \
	func(MMC, mmc, 2) \
	func(SATA, sata, 0) \
	func(USB, usb, 0) \
	func(PXE, pxe, na) \
	func(DHCP, dhcp, na)

#include <config_distro_bootcmd.h>

/* Physical Memory Map */
#define CFG_SYS_SDRAM_BASE          MMDC0_ARB_BASE_ADDR
#define CFG_SYS_INIT_RAM_ADDR       IRAM_BASE_ADDR
#define CFG_SYS_INIT_RAM_SIZE       IRAM_SIZE

/* Environment organization */

#endif                         /* __MX6CUBOXI_CONFIG_H */
