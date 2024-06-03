/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2013 Siemens Schweiz AG
 * (C) Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * Based on:
 * U-Boot file:/include/configs/am335x_evm.h
 *
 * Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/
 */

#ifndef __CONFIG_ETAMIN_H
#define __CONFIG_ETAMIN_H

#include "siemens-am33x-common.h"
/* NAND specific changes for etamin due to different page size */
#undef CONFIG_SYS_NAND_PAGE_SIZE
#undef CONFIG_SYS_NAND_OOBSIZE
#undef CONFIG_SYS_NAND_BLOCK_SIZE
#undef CONFIG_SYS_NAND_ECCPOS
#undef CONFIG_SYS_NAND_U_BOOT_OFFS
#undef CONFIG_SYS_ENV_SECT_SIZE
#undef CONFIG_ENV_OFFSET
#undef CONFIG_NAND_OMAP_ECCSCHEME
#define CONFIG_NAND_OMAP_ECCSCHEME	OMAP_ECC_BCH16_CODE_HW

#define CONFIG_ENV_OFFSET       0x980000
#define CONFIG_SYS_ENV_SECT_SIZE       (512 << 10)     /* 512 KiB */
#define CONFIG_SYS_NAND_PAGE_SIZE       4096
#define CONFIG_SYS_NAND_OOBSIZE         224
#define CONFIG_SYS_NAND_BLOCK_SIZE      (128 * CONFIG_SYS_NAND_PAGE_SIZE)
#define CONFIG_SYS_NAND_ECCPOS	{ 2, 3, 4, 5, 6, 7, 8, 9, \
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

#undef CONFIG_SYS_NAND_ECCSIZE
#undef CONFIG_SYS_NAND_ECCBYTES
#define CONFIG_SYS_NAND_ECCSIZE 512
#define CONFIG_SYS_NAND_ECCBYTES 26

#define CONFIG_SYS_NAND_U_BOOT_OFFS     0x200000

#define CONFIG_SYS_NAND_MAX_CHIPS       1

#undef CONFIG_SYS_MAX_NAND_DEVICE
#define CONFIG_SYS_MAX_NAND_DEVICE      3
#define CONFIG_SYS_NAND_BASE2           (0x18000000)    /* physical address */
#define CONFIG_SYS_NAND_BASE_LIST       {CONFIG_SYS_NAND_BASE, \
					CONFIG_SYS_NAND_BASE2}

#define CONFIG_SYS_NAND_ONFI_DETECTION
#define DDR_PLL_FREQ	303

/* FWD Button = 27
 * SRV Button = 87 */
#define BOARD_DFU_BUTTON_GPIO	27
#define GPIO_LAN9303_NRST	88	/* GPIO2_24 = gpio88 */
/* In dfu mode keep led1 on */
#define CONFIG_ENV_SETTINGS_BUTTONS_AND_LEDS \
	"button_dfu0=27\0" \
	"button_dfu1=87\0" \
	"led0=3,0,1\0" \
	"led1=4,0,0\0" \
	"led2=5,0,1\0" \
	"led3=87,0,1\0" \
	"led4=60,0,1\0" \
	"led5=63,0,1\0"

/* Physical Memory Map */
#define CONFIG_MAX_RAM_BANK_SIZE       (1024 << 20)    /* 1GB */

/* I2C Configuration */
#define CONFIG_SYS_I2C_SPEED		100000

#define CONFIG_SYS_I2C_EEPROM_ADDR              0x50
#define EEPROM_ADDR_DDR3 0x90
#define EEPROM_ADDR_CHIP 0x120

#define CONFIG_PHY_SMSC

#define CONFIG_FACTORYSET

/* use both define to compile a SPL compliance test  */
/*
#define CONFIG_SPL_CMT
#define CONFIG_SPL_CMT_DEBUG
*/

/* nedded by compliance test in read mode */
#if defined(CONFIG_SPL_CMT)
#define CONFIG_SYS_DCACHE_OFF
#endif

/* Define own nand partitions */
#define CONFIG_ENV_OFFSET_REDUND	0xB80000
#define CONFIG_ENV_SIZE_REDUND		CONFIG_ENV_SIZE
#define CONFIG_ENV_RANGE		(4 * CONFIG_SYS_ENV_SECT_SIZE)



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

#undef CONFIG_ENV_SETTINGS_NAND_V2
#define CONFIG_ENV_SETTINGS_NAND_V2 \
	"nand_active_ubi_vol=rootfs_a\0" \
	"rootfs_name=rootfs\0" \
	"kernel_name=uImage\0"\
	"nand_root_fs_type=ubifs rootwait=1\0" \
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

#ifndef CONFIG_SPL_BUILD

#define CONFIG_NAND_CS_INIT
#define ETAMIN_NAND_GPMC_CONFIG1	0x00000800
#define ETAMIN_NAND_GPMC_CONFIG2	0x001e1e00
#define ETAMIN_NAND_GPMC_CONFIG3	0x001e1e00
#define ETAMIN_NAND_GPMC_CONFIG4	0x16051807
#define ETAMIN_NAND_GPMC_CONFIG5	0x00151e1e
#define ETAMIN_NAND_GPMC_CONFIG6	0x16000f80
#define CONFIG_MTD_CONCAT

/* Default env settings */
#define CONFIG_EXTRA_ENV_SETTINGS \
	"hostname=etamin\0" \
	"ubi_off=4096\0"\
	"nand_img_size=0x400000\0" \
	"optargs=\0" \
	"preboot=draco_led 0\0" \
	CONFIG_ENV_SETTINGS_BUTTONS_AND_LEDS \
	CONFIG_ENV_SETTINGS_V2 \
	CONFIG_ENV_SETTINGS_NAND_V2

#ifndef CONFIG_RESTORE_FLASH

#define CONFIG_BOOTCOMMAND \
"if dfubutton; then " \
	"run dfu_start; " \
	"reset; " \
"fi;" \
"run nand_boot;" \
"run nand_boot_backup;" \
"reset;"


#else
#define CONFIG_BOOTCOMMAND			\
	"setenv autoload no; "			\
	"dhcp; "				\
	"if tftp 80000000 debrick.scr; then "	\
		"source 80000000; "		\
	"fi"
#endif
#endif	/* CONFIG_SPL_BUILD */
#endif	/* ! __CONFIG_ETAMIN_H */
