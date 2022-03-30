/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015 CompuLab, Ltd.
 *
 * Configuration settings for the CompuLab CL-SOM-iMX7 System-on-Module.
 */

#ifndef __CL_SOM_IMX7_CONFIG_H
#define __CL_SOM_IMX7_CONFIG_H

#include "mx7_common.h"

#define CONFIG_MXC_UART_BASE            UART1_IPS_BASE_ADDR

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(32 * SZ_1M)

#define CONFIG_BOARD_LATE_INIT

/* Uncomment to enable secure boot support */
/* #define CONFIG_SECURE_BOOT */
#define CONFIG_CSF_SIZE			0x4000

/* Network */
#define CONFIG_FEC_MXC
#define CONFIG_FEC_XCV_TYPE             RGMII
#define CONFIG_ETHPRIME                 "FEC"
#define CONFIG_FEC_MXC_PHYADDR          0

#define CONFIG_PHYLIB
#define CONFIG_PHY_ATHEROS
/* ENET1 */
#define IMX_FEC_BASE			ENET_IPS_BASE_ADDR

/* PMIC */
#define CONFIG_POWER
#define CONFIG_POWER_I2C
#define CONFIG_POWER_PFUZE3000
#define CONFIG_POWER_PFUZE3000_I2C_ADDR	0x08

#undef CONFIG_BOOTM_NETBSD
#undef CONFIG_BOOTM_PLAN9
#undef CONFIG_BOOTM_RTEMS

/* I2C configs */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_MXC
#define CONFIG_SYS_I2C_MXC_I2C2		/* Enable I2C bus 2 */
#define CONFIG_SYS_I2C_SPEED		100000
#define SYS_I2C_BUS_SOM			0

#define CONFIG_SYS_I2C_EEPROM_ADDR	0x50
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	1
#define CONFIG_SYS_I2C_EEPROM_BUS	SYS_I2C_BUS_SOM

#define CONFIG_PCA953X
#define CONFIG_CMD_PCA953X
#define CONFIG_SYS_I2C_PCA953X_ADDR	0x20
#define CONFIG_SYS_I2C_PCA953X_WIDTH	{ {0x20, 16} }

#undef CONFIG_SYS_AUTOLOAD
#undef CONFIG_EXTRA_ENV_SETTINGS
#undef CONFIG_BOOTCOMMAND

#define CONFIG_SYS_AUTOLOAD		"no"

#define CONFIG_EXTRA_ENV_SETTINGS \
	"autoload=off\0" \
	"script=boot.scr\0" \
	"loadscript=load ${storagetype} ${storagedev} ${loadaddr} ${script};\0" \
	"loadkernel=load ${storagetype} ${storagedev} ${loadaddr} ${kernel};\0" \
	"loadfdt=load ${storagetype} ${storagedev} ${fdtaddr} ${fdtfile};\0" \
	"bootscript=echo Running bootscript from ${storagetype} ...; source ${loadaddr};\0" \
	"storagebootcmd=echo Booting from ${storagetype} ...; run ${storagetype}args; run doboot;\0" \
	"kernel=zImage\0" \
	"console=ttymxc0\0" \
	"fdt_high=0xffffffff\0" \
	"initrd_high=0xffffffff\0" \
	"fdtfile=imx7d-sbc-imx7.dtb\0" \
	"fdtaddr=0x83000000\0" \
	"mmcdev_def="__stringify(CONFIG_SYS_MMC_DEV)"\0" \
	"usbdev_def="__stringify(CONFIG_SYS_USB_DEV)"\0" \
	"mmcpart=" __stringify(CONFIG_SYS_MMC_IMG_LOAD_PART) "\0" \
	"usbpart=" __stringify(CONFIG_SYS_USB_IMG_LOAD_PART) "\0" \
	"doboot=bootz ${loadaddr} - ${fdtaddr}\0" \
	"mmc_config=mmc dev ${mmcdev}; mmc rescan\0" \
	"mmcargs=setenv bootargs console=${console},${baudrate} " \
		"root=/dev/mmcblk${mmcblk}p2 rootwait rw\0" \
	"mmcbootscript=" \
		"if run mmc_config; then " \
			"setenv storagetype mmc;" \
			"setenv storagedev ${mmcdev}:${mmcpart};" \
			"if run loadscript; then " \
				"run bootscript; " \
			"fi; " \
		"fi;\0" \
	"mmcboot=" \
		"if run mmc_config; then " \
			"setenv storagetype mmc;" \
			"setenv storagedev ${mmcdev}:${mmcpart};" \
			"if run loadkernel; then " \
				"if run loadfdt; then " \
					"run storagebootcmd;" \
				"fi; " \
			"fi; " \
		"fi;\0" \
	"sdbootscript=setenv mmcdev ${mmcdev_def}; setenv mmcblk 0; " \
		"run mmcbootscript\0" \
	"usbbootscript=setenv usbdev ${usbdev_def}; " \
		"setenv storagetype usb;" \
		"setenv storagedev ${usbdev}:${usbpart};" \
		"if run loadscript; then " \
			"run bootscript; " \
		"fi; " \
	"sdboot=setenv mmcdev ${mmcdev_def}; setenv mmcblk 0; run mmcboot\0" \
	"emmcbootscript=setenv mmcdev 1; setenv mmcblk 2; run mmcbootscript\0" \
	"emmcboot=setenv mmcdev 1; setenv mmcblk 2; run mmcboot\0" \

#define CONFIG_BOOTCOMMAND \
	"echo SD boot attempt ...; run sdbootscript; run sdboot; " \
	"echo eMMC boot attempt ...; run emmcbootscript; run emmcboot; " \
	"echo USB boot attempt ...; run usbbootscript; "

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

/* SPI Flash support */

/* FLASH and environment organization */
#define CONFIG_ENV_SIZE			SZ_8K
#define CONFIG_ENV_OFFSET		(768 * 1024)
#define CONFIG_ENV_SECT_SIZE		(64 * 1024)

/* MMC Config*/
#define CONFIG_FSL_USDHC
#ifdef CONFIG_FSL_USDHC
#define CONFIG_SYS_FSL_ESDHC_ADDR       USDHC1_BASE_ADDR

#define CONFIG_SYS_FSL_USDHC_NUM	2
#define CONFIG_MMCROOT			"/dev/mmcblk0p2" /* USDHC1 */
#endif

/* USB Configs */
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET
#define CONFIG_MXC_USB_PORTSC  (PORT_PTS_UTMI | PORT_PTS_PTW)
#define CONFIG_MXC_USB_FLAGS   0
#define CONFIG_USB_MAX_CONTROLLER_COUNT 2

/* Uncomment to enable iMX thermal driver support */
/*#define CONFIG_IMX_THERMAL*/

/* SPL */
#include "imx7_spl.h"
#ifdef CONFIG_SPL_BUILD
#define CONFIG_SYS_SPI_U_BOOT_OFFS	(64 * 1024)
#endif /* CONFIG_SPL_BUILD */

#endif	/* __CONFIG_H */
