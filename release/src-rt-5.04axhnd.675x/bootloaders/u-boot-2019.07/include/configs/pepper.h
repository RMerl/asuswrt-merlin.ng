/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2013 Gumstix, Inc. - http://www.gumstix.com/
 */

#ifndef __CONFIG_PEPPER_H
#define __CONFIG_PEPPER_H

#include <configs/ti_am335x_common.h>

/* Clock defines */
#define V_OSCK				24000000  /* Clock output from T2 */
#define V_SCLK				(V_OSCK)

#define CONFIG_SYS_I2C_EEPROM_ADDR	0x50

/* Mach type */
#define CONFIG_MACH_TYPE		MACH_TYPE_PEPPER

#define CONFIG_ENV_SIZE			(128 << 10)	/* 128 KiB */

#define CONFIG_EXTRA_ENV_SETTINGS \
	DEFAULT_LINUX_BOOT_ENV \
	"bootdir=/boot\0" \
	"bootfile=zImage\0" \
	"fdtfile=am335x-pepper.dtb\0" \
	"console=ttyO0,115200n8\0" \
	"optargs=\0" \
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
	"loaduimage=fatload mmc ${mmcdev}:1 ${loadaddr} uImage\0" \
	"uimageboot=echo Booting from mmc${mmcdev} ...; " \
		"run mmcargs; " \
		"bootm ${loadaddr}\0" \
	"mmcboot=echo Booting from mmc ...; " \
		"run mmcargs; " \
		"bootz ${loadaddr} - ${fdtaddr}\0" \
	"ubiboot=echo Booting from nand (ubifs) ...; " \
		"run ubiargs; run ubiload; " \
		"bootz ${loadaddr} - ${fdtaddr}\0" \

#define CONFIG_BOOTCOMMAND \
	"mmc dev ${mmcdev}; if mmc rescan; then " \
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
			"run mmcboot;" \
		"fi;" \
		"if run loaduimage; then " \
			"run uimageboot;" \
		"fi;" \
	"fi;" \

/* Serial console configuration */
#define CONFIG_SYS_NS16550_COM1		0x44e09000

/* Ethernet support */
#define CONFIG_PHY_RESET_DELAY 1000

/* SPL */

#endif /* __CONFIG_PEPPER_H */
