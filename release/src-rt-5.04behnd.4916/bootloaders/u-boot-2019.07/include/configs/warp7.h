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

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(35 * SZ_1M)

/* MMC Config*/
#define CONFIG_SYS_FSL_ESDHC_ADDR       USDHC3_BASE_ADDR
#define CONFIG_SYS_FSL_ESDHC_HAS_DDR_MODE
#define CONFIG_SYS_MMC_IMG_LOAD_PART	1

/* Switch on SERIAL_TAG */
#define CONFIG_SERIAL_TAG

#define CONFIG_DFU_ENV_SETTINGS \
	"dfu_alt_info=boot raw 0x2 0x1000 mmcpart 1\0" \

#define CONFIG_EXTRA_ENV_SETTINGS \
	CONFIG_DFU_ENV_SETTINGS \
	"script=boot.scr\0" \
	"script_signed=boot.scr.imx-signed\0" \
	"image=zImage\0" \
	"console=ttymxc0\0" \
	"ethact=usb_ether\0" \
	"fdt_high=0xffffffff\0" \
	"initrd_high=0xffffffff\0" \
	"fdt_file=imx7s-warp.dtb\0" \
	"fdt_addr=" __stringify(CONFIG_SYS_FDT_ADDR)"\0" \
	"optee_addr=" __stringify(CONFIG_OPTEE_LOAD_ADDR)"\0" \
	"boot_fdt=try\0" \
	"ip_dyn=yes\0" \
	"mmcdev="__stringify(CONFIG_SYS_MMC_ENV_DEV)"\0" \
	"mmcpart=" __stringify(CONFIG_SYS_MMC_IMG_LOAD_PART) "\0" \
	"rootpart=" __stringify(CONFIG_WARP7_ROOT_PART) "\0" \
	"finduuid=part uuid mmc 0:${rootpart} uuid\0" \
	"mmcargs=setenv bootargs console=${console},${baudrate} " \
		"root=PARTUUID=${uuid} rootwait rw\0" \
	"ivt_offset=" __stringify(BOOTROM_IVT_HDR_OFFSET)"\0"\
	"warp7_auth_or_fail=hab_auth_img_or_fail ${hab_ivt_addr} ${filesize} 0;\0" \
	"do_bootscript_hab=" \
		"if test ${hab_enabled} -eq 1; then " \
			"setexpr hab_ivt_addr ${loadaddr} - ${ivt_offset}; " \
			"setenv script ${script_signed}; " \
			"load mmc ${mmcdev}:${mmcpart} ${hab_ivt_addr} ${script}; " \
			"run warp7_auth_or_fail; " \
			"run bootscript; "\
		"fi;\0" \
	"loadbootscript=" \
		"load mmc ${mmcdev}:${mmcpart} ${loadaddr} ${script};\0" \
	"bootscript=echo Running bootscript from mmc ...; " \
		"source\0" \
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

#define CONFIG_BOOTCOMMAND \
	   "mmc dev ${mmcdev};" \
	   "mmc dev ${mmcdev}; if mmc rescan; then " \
		   "run do_bootscript_hab;" \
		   "if run loadbootscript; then " \
			   "run bootscript; " \
		   "else " \
			   "if run loadimage; then " \
				   "run mmcboot; " \
			   "fi; " \
		   "fi; " \
	   "fi"

#define CONFIG_SYS_MEMTEST_START	0x80000000
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_MEMTEST_START + 0x20000000)

#define CONFIG_SYS_LOAD_ADDR		CONFIG_LOADADDR
#define CONFIG_SYS_HZ			1000

/* Physical Memory Map */
#define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CONFIG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE	IRAM_SIZE

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

/* I2C configs */
#define CONFIG_SYS_I2C_MXC
#define CONFIG_SYS_I2C_SPEED		100000

/* environment organization */
#define CONFIG_ENV_SIZE			SZ_8K

#define CONFIG_ENV_OFFSET		(8 * SZ_64K)
#define CONFIG_SYS_FSL_USDHC_NUM	1

#define CONFIG_SYS_MMC_ENV_DEV		0
#define CONFIG_SYS_MMC_ENV_PART		0

/* USB Configs */
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET

#define CONFIG_MXC_USB_PORTSC		(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CONFIG_MXC_USB_FLAGS		0
#define CONFIG_USB_MAX_CONTROLLER_COUNT 1 /* Only OTG1 port enabled */

#define CONFIG_IMX_THERMAL

#define CONFIG_USBD_HS

/* USB Device Firmware Update support */
#define CONFIG_SYS_DFU_DATA_BUF_SIZE	SZ_16M
#define DFU_DEFAULT_POLL_TIMEOUT	300

#define CONFIG_USBNET_DEV_ADDR		"de:ad:be:af:00:01"

/* Environment variable name to represent HAB enable state */
#define HAB_ENABLED_ENVNAME		"hab_enabled"

#endif
