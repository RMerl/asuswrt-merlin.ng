/*
 * pcm051.h
 *
 * Phytec phyCORE-AM335x (pcm051) boards information header
 *
 * Copyright (C) 2013 Lemonage Software GmbH
 * Author Lars Poeschel <poeschel@lemonage.de>
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

#ifndef __CONFIG_PCM051_H
#define __CONFIG_PCM051_H

#include <configs/ti_am335x_common.h>

#define CONFIG_ENV_SIZE			(128 << 10)	/* 128 KiB */
#define CONFIG_MACH_TYPE		MACH_TYPE_PCM051

/* set to negative value for no autoboot */
#define BOOTENV_DEV_LEGACY_MMC(devtypeu, devtypel, instance) \
	"bootcmd_" #devtypel #instance "=" \
	"setenv mmcdev " #instance"; "\
	"setenv bootpart " #instance":2 ; "\
	"run mmcboot\0"

#define BOOTENV_DEV_NAME_LEGACY_MMC(devtypeu, devtypel, instance) \
	#devtypel #instance " "

#define BOOTENV_DEV_NAND(devtypeu, devtypel, instance) \
	"bootcmd_" #devtypel "=" \
	"run nandboot\0"

#define BOOTENV_DEV_NAME_NAND(devtypeu, devtypel, instance) \
	#devtypel #instance " "

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(LEGACY_MMC, legacy_mmc, 0) \
	func(MMC, mmc, 1) \
	func(LEGACY_MMC, legacy_mmc, 1) \
	func(NAND, nand, 0)

#define CONFIG_BOOTCOMMAND \
	"run distro_bootcmd"

#include <config_distro_bootcmd.h>

#include <environment/ti/dfu.h>
#include <environment/ti/mmc.h>

#define CONFIG_EXTRA_ENV_SETTINGS \
	DEFAULT_LINUX_BOOT_ENV \
	DEFAULT_MMC_TI_ARGS \
	"bootfile=uImage\0" \
	"fdtfile=am335x-wega-rdk.dtb\0" \
	"console=ttyO0,115200n8\0" \
	"optargs=\0" \
	"ramroot=/dev/ram0 rw ramdisk_size=65536 initrd=${rdaddr},64M\0" \
	"ramrootfstype=ext2\0" \
	"bootenv=uEnv.txt\0" \
	"loadbootscript=load mmc ${mmcdev} ${loadaddr} boot.scr\0" \
	"bootscript=echo Running bootscript from mmc${mmcdev} ...; " \
		"source ${loadaddr}\0" \
	"loadbootenv=fatload mmc ${mmcdev} ${loadaddr} ${bootenv}\0" \
	"importbootenv=echo Importing environment from mmc ...; " \
		"env import -t $loadaddr $filesize\0" \
	"ramargs=setenv bootargs console=${console} " \
		"${optargs} " \
		"root=${ramroot} " \
		"rootfstype=${ramrootfstype}\0" \
	"loadramdisk=fatload mmc ${mmcdev} ${rdaddr} ramdisk.gz\0" \
	"loaduimagefat=fatload mmc ${mmcdev} ${loadaddr} ${bootfile}\0" \
	"loaduimage=ext2load mmc ${mmcdev}:2 ${loadaddr} ${bootfile}\0" \
	"mmcboot=echo Booting from mmc ...; " \
		"run args_mmc; " \
		"bootm ${loadaddr}\0" \
	"ramboot=echo Booting from ramdisk ...; " \
		"run ramargs; " \
		"bootm ${loadaddr}\0" \
	BOOTENV

/* Clock Defines */
#define V_OSCK				25000000  /* Clock output from T2 */
#define V_SCLK				(V_OSCK)

/*
 * memtest works on 8 MB in DRAM after skipping 32MB from
 * start addr of ram disk
 */
#define CONFIG_SYS_MEMTEST_START	(CONFIG_SYS_SDRAM_BASE + (64 << 20))
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_MEMTEST_START \
					+ (8 * 1024 * 1024))

/* NS16550 Configuration */
#define CONFIG_SYS_NS16550_COM1		0x44e09000	/* Base EVM has UART0 */
#define CONFIG_SYS_NS16550_COM2		0x48022000	/* UART1 */
#define CONFIG_SYS_NS16550_COM3		0x48024000	/* UART2 */
#define CONFIG_SYS_NS16550_COM4		0x481a6000	/* UART3 */
#define CONFIG_SYS_NS16550_COM5		0x481a8000	/* UART4 */
#define CONFIG_SYS_NS16550_COM6		0x481aa000	/* UART5 */

/* I2C Configuration */
#define CONFIG_ENV_EEPROM_IS_ON_I2C
#define CONFIG_SYS_I2C_EEPROM_ADDR	0x50	/* Main EEPROM */
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	2

#define CONFIG_SYS_BAUDRATE_TABLE	{ 110, 300, 600, 1200, 2400, \
4800, 9600, 14400, 19200, 28800, 38400, 56000, 57600, 115200 }

/* CPU */

#ifdef CONFIG_SPI_BOOT
#define CONFIG_SYS_SPI_U_BOOT_OFFS	0x20000
#define CONFIG_SYS_SPI_U_BOOT_SIZE	0x40000
#endif

/*
 * USB configuration
 */
#define CONFIG_AM335X_USB0
#define CONFIG_AM335X_USB0_MODE	MUSB_PERIPHERAL
#define CONFIG_AM335X_USB1
#define CONFIG_AM335X_USB1_MODE MUSB_HOST

#define CONFIG_PHY_SMSC

#endif	/* ! __CONFIG_PCM051_H */
