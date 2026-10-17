/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 NXP Semiconductors
 *
 * Configuration settings for the i.MX7S Warp board.
 */

#ifndef __WARP7_CONFIG_H
#define __WARP7_CONFIG_H

#include "mx7_common.h"
#include <imximage.h>

#define PHYS_SDRAM_SIZE			SZ_512M

/* MMC Config*/
#define CFG_SYS_FSL_ESDHC_ADDR       USDHC3_BASE_ADDR

#define CFG_DFU_ENV_SETTINGS \
	"dfu_alt_info=boot raw 0x2 0x1000 mmcpart 1\0" \

/* When booting with FIT specify the node entry containing boot.scr */
#if defined(CONFIG_FIT)
#define BOOT_SCR_STRING "source ${bootscriptaddr}:${bootscr_fitimage_name}\0"
#else
#define BOOT_SCR_STRING "source ${bootscriptaddr}\0"
#endif

#define CFG_EXTRA_ENV_SETTINGS \
	CFG_DFU_ENV_SETTINGS \
	"script=boot.scr\0" \
	"bootscr_fitimage_name=bootscr\0" \
	"script_signed=boot.scr.imx-signed\0" \
	"bootscriptaddr=0x83200000\0" \
	"image=zImage\0" \
	"console=ttymxc0\0" \
	"ethact=usb_ether\0" \
	"fdt_high=0xffffffff\0" \
	"initrd_high=0xffffffff\0" \
	"fdt_file=imx7s-warp.dtb\0" \
	"fdt_addr=0x83000000\0" \
	"fdtovaddr=0x83100000\0" \
	"boot_fdt=try\0" \
	"ip_dyn=yes\0" \
	"mmcdev="__stringify(CONFIG_SYS_MMC_ENV_DEV)"\0" \
	"mmcpart=1\0" \
	"rootpart=" __stringify(CONFIG_WARP7_ROOT_PART) "\0" \
	"finduuid=part uuid mmc 0:${rootpart} uuid\0" \
	"mmcargs=setenv bootargs console=${console},${baudrate} " \
		"root=PARTUUID=${uuid} rootwait rw\0" \
	"ivt_offset=" __stringify(BOOTROM_IVT_HDR_OFFSET)"\0"\
	"warp7_auth_or_fail=hab_auth_img_or_fail ${hab_ivt_addr} ${filesize} 0;\0" \
	"do_bootscript_hab=" \
		"if test ${hab_enabled} -eq 1; then " \
			"setexpr hab_ivt_addr ${bootscriptaddr} - ${ivt_offset}; " \
			"setenv script ${script_signed}; " \
			"load mmc ${mmcdev}:${mmcpart} ${hab_ivt_addr} ${script}; " \
			"run warp7_auth_or_fail; " \
			"run bootscript; "\
		"fi;\0" \
	"loadbootscript=" \
		"load mmc ${mmcdev}:${mmcpart} ${bootscriptaddr} ${script};\0" \
	"bootscript=echo Running bootscript from mmc ...; " \
		BOOT_SCR_STRING \
	"loadimage=load mmc ${mmcdev}:${mmcpart} ${loadaddr} ${image}\0" \
	"loadfdt=load mmc ${mmcdev}:${mmcpart} ${fdt_addr} ${fdt_file}\0" \
	"mmcboot=echo Booting from mmc ...; " \
		"run finduuid; " \
		"run mmcargs; " \
		"if test ${boot_fdt} = yes || test ${boot_fdt} = try; then " \
			"if run loadfdt; then " \
				"bootz ${loadaddr} - ${fdt_addr}; " \
			"else " \
				"if test ${boot_fdt} = try; then " \
					"bootz; " \
				"else " \
					"echo WARN: Cannot load the DT; " \
				"fi; " \
			"fi; " \
		"else " \
			"bootz; " \
		"fi;\0" \

/* Physical Memory Map */
#define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR

#define CFG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CFG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CFG_SYS_INIT_RAM_SIZE	IRAM_SIZE

/* environment organization */

#define CFG_SYS_FSL_USDHC_NUM	1


#define CFG_MXC_USB_PORTSC		(PORT_PTS_UTMI | PORT_PTS_PTW)

/* USB Device Firmware Update support */
#define DFU_DEFAULT_POLL_TIMEOUT	300

/* Environment variable name to represent HAB enable state */
#define HAB_ENABLED_ENVNAME		"hab_enabled"

#endif
