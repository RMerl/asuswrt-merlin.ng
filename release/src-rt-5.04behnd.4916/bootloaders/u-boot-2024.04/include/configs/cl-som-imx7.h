/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015 CompuLab, Ltd.
 *
 * Configuration settings for the CompuLab CL-SOM-iMX7 System-on-Module.
 */

#ifndef __CL_SOM_IMX7_CONFIG_H
#define __CL_SOM_IMX7_CONFIG_H

#include "mx7_common.h"

#define CFG_MXC_UART_BASE            UART1_IPS_BASE_ADDR

/* Network */
#define CFG_FEC_MXC_PHYADDR          0

/* ENET1 */
#define IMX_FEC_BASE			ENET_IPS_BASE_ADDR

/* PMIC */
#define CFG_POWER_PFUZE3000_I2C_ADDR	0x08

#define CFG_SYS_I2C_PCA953X_ADDR	0x20
#define CFG_SYS_I2C_PCA953X_WIDTH	{ {0x20, 16} }

#undef CFG_EXTRA_ENV_SETTINGS

#define CFG_EXTRA_ENV_SETTINGS \
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
	"mmcpart=1\0" \
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

/* Physical Memory Map */
#define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR

#define CFG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CFG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CFG_SYS_INIT_RAM_SIZE	IRAM_SIZE

/* SPI Flash support */

/* FLASH and environment organization */

/* MMC Config*/
#ifdef CONFIG_FSL_USDHC
#define CFG_SYS_FSL_ESDHC_ADDR       USDHC1_BASE_ADDR

#define CFG_SYS_FSL_USDHC_NUM	2
#endif

/* USB Configs */
#define CFG_MXC_USB_PORTSC  (PORT_PTS_UTMI | PORT_PTS_PTW)
#define CFG_MXC_USB_FLAGS   0

#endif	/* __CONFIG_H */
