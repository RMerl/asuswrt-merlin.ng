/*
 * Copyright (C) 2013, ISEE 2007 SL - http://www.isee.biz/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __CONFIG_IGEP003X_H
#define __CONFIG_IGEP003X_H

#include <configs/ti_am335x_common.h>

/* Clock defines */
#define V_OSCK				24000000  /* Clock output from T2 */
#define V_SCLK				(V_OSCK)

#define CONFIG_ENV_SIZE			(96 << 10)	/*  96 KiB */

#ifndef CONFIG_SPL_BUILD
#define CONFIG_EXTRA_ENV_SETTINGS \
	DEFAULT_LINUX_BOOT_ENV \
	"bootdir=/boot\0" \
	"bootfile=zImage\0" \
	"console=ttyO0,115200n8\0" \
	"mmcdev=0\0" \
	"mmcroot=/dev/mmcblk0p2 rw\0" \
	"mmcrootfstype=ext4 rootwait\0" \
	"mmcargs=setenv bootargs console=${console} " \
		"${optargs} " \
		"root=${mmcroot} " \
		"rootfstype=${mmcrootfstype}\0" \
		"bootenv=uEnv.txt\0" \
	"loadbootenv=load mmc ${mmcdev} ${loadaddr} ${bootenv}\0" \
	"importbootenv=echo Importing environment from mmc ...; " \
		"env import -t ${loadaddr} ${filesize}\0" \
	"mmcload=load mmc ${mmcdev}:2 ${loadaddr} ${bootdir}/${bootfile}; " \
		"load mmc ${mmcdev}:2 ${fdtaddr} ${bootdir}/${fdtfile}\0" \
	"mmcboot=mmc dev ${mmcdev}; " \
		"if mmc rescan; then " \
			"echo SD/MMC found on device ${mmcdev};" \
			"if run loadbootenv; then " \
				"echo Loaded environment from ${bootenv};" \
				"run importbootenv;" \
			"fi;" \
			"if test -n $uenvcmd; then " \
				"echo Running uenvcmd ...;" \
				"run uenvcmd;" \
			"fi;" \
			"if run mmcload; then " \
				"run mmcargs; " \
				"bootz ${loadaddr} - ${fdtaddr};" \
			"fi;" \
		"fi;\0" \
	"mtdids=" CONFIG_MTDIDS_DEFAULT "\0" \
	"mtdparts=" CONFIG_MTDPARTS_DEFAULT "\0" \
	"nandroot=ubi0:rootfs rw ubi.mtd=1\0" \
	"nandrootfstype=ubifs rootwait\0" \
	"nandload=ubi part UBI; " \
		"ubi read ${loadaddr} kernel; " \
		"ubi read ${fdtaddr} dtb \0" \
	"nandargs=setenv bootargs console=${console} " \
		"${optargs} " \
		"root=${nandroot} " \
		"rootfstype=${nandrootfstype} \0" \
	"nandboot=echo Booting from nand ...; " \
		"run nandargs; " \
		"run nandload; " \
		"bootz ${loadaddr} - ${fdtaddr} \0" \
	"netload=tftpboot ${loadaddr} ${bootfile}; " \
		"tftpboot ${fdtaddr} ${fdtfile} \0" \
	"netargs=setenv bootargs console=${console} " \
		"${optargs} " \
		"root=/dev/nfs " \
		"ip=${ipaddr} nfsroot=${serverip}:${rootnfs},v3,tcp \0" \
	"netboot=echo Booting from net ...; " \
		"run netargs; " \
		"run netload; " \
		"bootz ${loadaddr} - ${fdtaddr} \0" \
	"findfdt="\
		"if test ${board_name} = igep0033; then " \
			"setenv fdtfile am335x-igep-base0033.dtb; fi; " \
		"if test ${board_name} = igep0034; then " \
			"setenv fdtfile am335x-igep-base0040.dtb; fi; " \
		"if test ${board_name} = igep0034-lite; then " \
			"setenv fdtfile am335x-igep-base0040-lite.dtb; fi; " \
		"if test ${fdtfile} = ''; then " \
			"echo WARNING: Could not determine device tree to use; fi; \0"
#endif

#define CONFIG_BOOTCOMMAND \
	"run findfdt;" \
	"run mmcboot;" \
	"run nandboot;" \
	"run netboot;"

/* NS16550 Configuration */
#define CONFIG_SYS_NS16550_COM1		0x44e09000	/* UART0 */

/* Ethernet support */
#define CONFIG_PHY_SMSC

/* NAND support */
#define CONFIG_SYS_NAND_ONFI_DETECTION	1

/* SPL */

/* UBI configuration */
#define CONFIG_SPL_UBI			1
#define CONFIG_SPL_UBI_MAX_VOL_LEBS	256
#define CONFIG_SPL_UBI_MAX_PEB_SIZE	(256*1024)
#define CONFIG_SPL_UBI_MAX_PEBS		4096
#define CONFIG_SPL_UBI_VOL_IDS		8
#define CONFIG_SPL_UBI_LOAD_MONITOR_ID	0
#define CONFIG_SPL_UBI_LOAD_KERNEL_ID	3
#define CONFIG_SPL_UBI_LOAD_ARGS_ID	4
#define CONFIG_SPL_UBI_PEB_OFFSET	4
#define CONFIG_SPL_UBI_VID_OFFSET	512
#define CONFIG_SPL_UBI_LEB_START	2048
#define CONFIG_SPL_UBI_INFO_ADDR	0x88080000

/* environment organization */
#define CONFIG_ENV_UBI_PART		"UBI"
#define CONFIG_ENV_UBI_VOLUME		"config"
#define CONFIG_ENV_UBI_VOLUME_REDUND	"config_r"

/* NAND config */
#define CONFIG_SYS_NAND_5_ADDR_CYCLE
#define CONFIG_SYS_NAND_PAGE_COUNT	(CONFIG_SYS_NAND_BLOCK_SIZE / \
					 CONFIG_SYS_NAND_PAGE_SIZE)
#define CONFIG_SYS_NAND_PAGE_SIZE	2048
#define CONFIG_SYS_NAND_OOBSIZE		64
#define CONFIG_SYS_NAND_BLOCK_SIZE	(128*1024)
#define CONFIG_SYS_NAND_BAD_BLOCK_POS	NAND_LARGE_BADBLOCK_POS
#define CONFIG_SYS_NAND_ECCPOS		{ 2, 3, 4, 5, 6, 7, 8, 9, \
					 10, 11, 12, 13, 14, 15, 16, 17, \
					 18, 19, 20, 21, 22, 23, 24, 25, \
					 26, 27, 28, 29, 30, 31, 32, 33, \
					 34, 35, 36, 37, 38, 39, 40, 41, \
					 42, 43, 44, 45, 46, 47, 48, 49, \
					 50, 51, 52, 53, 54, 55, 56, 57, }

#define CONFIG_SYS_NAND_ECCSIZE		512
#define CONFIG_SYS_NAND_ECCBYTES	14
#define CONFIG_NAND_OMAP_ECCSCHEME	OMAP_ECC_BCH8_CODE_HW

#endif	/* ! __CONFIG_IGEP003X_H */
