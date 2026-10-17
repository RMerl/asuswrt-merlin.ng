/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2013 Siemens Schweiz AG
 * (C) Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * Based on:
 * U-Boot file:/include/configs/am335x_evm.h
 *
 * Copyright (C) 2011 Texas Instruments Incorporated - https://www.ti.com/
 */

#ifndef __CONFIG_ETAMIN_H
#define __CONFIG_ETAMIN_H

#include "siemens-am33x-common.h"
/* NAND specific changes for etamin due to different page size */
#undef CFG_SYS_NAND_ECCPOS

#define CFG_SYS_ENV_SECT_SIZE       (512 << 10)     /* 512 KiB */
#define CFG_SYS_NAND_ECCPOS	{ 2, 3, 4, 5, 6, 7, 8, 9, \
				10, 11, 12, 13, 14, 15, 16, 17, 18, 19, \
				20, 21, 22, 23, 24, 25, 26, 27, 28, 29, \
				30, 31, 32, 33, 34, 35, 36, 37, 38, 39, \
				40, 41, 42, 43, 44, 45, 46, 47, 48, 49, \
				50, 51, 52, 53, 54, 55, 56, 57, 58, 59, \
				60, 61, 62, 63, 64, 65, 66, 67, 68, 69, \
				70, 71, 72, 73, 74, 75, 76, 77, 78, 79, \
				80, 81, 82, 83, 84, 85, 86, 87, 88, 89, \
				90, 91, 92, 93, 94, 95, 96, 97, 98, 99, \
			100, 101, 102, 103, 104, 105, 106, 107, 108, 109, \
			110, 111, 112, 113, 114, 115, 116, 117, 118, 119, \
			120, 121, 122, 123, 124, 125, 126, 127, 128, 129, \
			130, 131, 132, 133, 134, 135, 136, 137, 138, 139, \
			140, 141, 142, 143, 144, 145, 146, 147, 148, 149, \
			150, 151, 152, 153, 154, 155, 156, 157, 158, 159, \
			160, 161, 162, 163, 164, 165, 166, 167, 168, 169, \
			170, 171, 172, 173, 174, 175, 176, 177, 178, 179, \
			180, 181, 182, 183, 184, 185, 186, 187, 188, 189, \
			190, 191, 192, 193, 194, 195, 196, 197, 198, 199, \
			200, 201, 202, 203, 204, 205, 206, 207, 208, 209, \
			}

#undef CFG_SYS_NAND_ECCSIZE
#undef CFG_SYS_NAND_ECCBYTES
#define CFG_SYS_NAND_ECCSIZE 512
#define CFG_SYS_NAND_ECCBYTES 26

#define CFG_SYS_NAND_BASE2           (0x18000000)    /* physical address */
#define CFG_SYS_NAND_BASE_LIST       {CFG_SYS_NAND_BASE, \
					CFG_SYS_NAND_BASE2}

#define DDR_PLL_FREQ	303

/* FWD Button = 27
 * SRV Button = 87 */
#define BOARD_DFU_BUTTON_GPIO	27
#define GPIO_LAN9303_NRST	88	/* GPIO2_24 = gpio88 */
/* In dfu mode keep led1 on */
#define CFG_ENV_SETTINGS_BUTTONS_AND_LEDS \
	"button_dfu0=27\0" \
	"button_dfu1=87\0" \
	"led0=3,0,1\0" \
	"led1=4,0,0\0" \
	"led2=5,0,1\0" \
	"led3=87,0,1\0" \
	"led4=60,0,1\0" \
	"led5=63,0,1\0"

/* Physical Memory Map */
#define CFG_MAX_RAM_BANK_SIZE       (1024 << 20)    /* 1GB */

/* nedded by compliance test in read mode */

#undef COMMON_ENV_DFU_ARGS
#define COMMON_ENV_DFU_ARGS	"dfu_args=run bootargs_defaults;" \
				"setenv bootargs ${bootargs};" \
				"mtdparts default;" \
				"draco_led 1;" \
				"dfu 0 mtd 0;" \
				"draco_led 0;\0" \

#undef DFU_ALT_INFO_NAND_V2
#define DFU_ALT_INFO_NAND_V2 \
	"spl mtddev;" \
	"spl.backup1 mtddev;" \
	"spl.backup2 mtddev;" \
	"spl.backup3 mtddev;" \
	"u-boot mtddev;" \
	"u-boot.env0 mtddev;" \
	"u-boot.env1 mtddev;" \
	"rootfs mtddevubi" \

#undef CFG_ENV_SETTINGS_NAND_V2
#define CFG_ENV_SETTINGS_NAND_V2 \
	"nand_active_ubi_vol=rootfs_a\0" \
	"rootfs_name=rootfs\0" \
	"kernel_name=uImage\0"\
	"nand_root_fs_type=ubifs rootwait\0" \
	"nand_args=run bootargs_defaults;" \
		"mtdparts default;" \
		"setenv ${partitionset_active} true;" \
		"if test -n ${A}; then " \
			"setenv nand_active_ubi_vol ${rootfs_name}_a;" \
		"fi;" \
		"if test -n ${B}; then " \
			"setenv nand_active_ubi_vol ${rootfs_name}_b;" \
		"fi;" \
		"setenv nand_root ubi0:${nand_active_ubi_vol} rw " \
		"ubi.mtd=rootfs,${ubi_off};" \
		"setenv bootargs ${bootargs} " \
		"root=${nand_root} noinitrd ${mtdparts} " \
		"rootfstype=${nand_root_fs_type} ip=${ip_method} " \
		"console=ttyMTD,mtdoops console=ttyO0,115200n8 mtdoops.mtddev" \
		"=mtdoops\0" \
	COMMON_ENV_DFU_ARGS \
		"dfu_alt_info=" DFU_ALT_INFO_NAND_V2 "\0" \
	COMMON_ENV_NAND_BOOT \
		"ubi part rootfs ${ubi_off};" \
		"ubifsmount ubi0:${nand_active_ubi_vol};" \
		"ubifsload ${kloadaddr} boot/${kernel_name};" \
		"ubifsload ${loadaddr} boot/${dtb_name}.dtb;" \
		"bootm ${kloadaddr} - ${loadaddr}\0" \
	"nand_boot_backup=ubifsload ${loadaddr} boot/am335x-draco.dtb;" \
		"bootm ${kloadaddr} - ${loadaddr}\0" \
	COMMON_ENV_NAND_CMDS

/* Default env settings */
#define CFG_EXTRA_ENV_SETTINGS \
	"hostname=etamin\0" \
	"ubi_off=4096\0"\
	"nand_img_size=0x400000\0" \
	"optargs=\0" \
	"preboot=draco_led 0\0" \
	CFG_ENV_SETTINGS_BUTTONS_AND_LEDS \
	CFG_ENV_SETTINGS_V2 \
	CFG_ENV_SETTINGS_NAND_V2

#endif	/* ! __CONFIG_ETAMIN_H */
