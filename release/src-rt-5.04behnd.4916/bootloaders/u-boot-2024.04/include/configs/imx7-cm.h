/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2021 Ronetix GmbH
 *
 * Configuration settings for the Ronetix's iMX7-CM System-on-Module.
 */

#ifndef __IMX7_CM_CONFIG_H
#define __IMX7_CM_CONFIG_H

#include "mx7_common.h"

#define CFG_MXC_UART_BASE            UART1_IPS_BASE_ADDR

#undef CFG_EXTRA_ENV_SETTINGS

/*
 * Use:
 *		boot-mode=mix
 *		boot-mode=sd
 *		boot-mode=net
 */
#define MY_CONFIG_BOOT_MODE	"boot-mode=sd\0"

#define CFG_EXTRA_ENV_SETTINGS \
	MY_CONFIG_BOOT_MODE \
	"image=zImage\0" \
	"console=ttymxc0\0" \
	"fdt_file=imx7-cm.dtb\0" \
	"fdt_addr=0x83000000\0" \
	"mmcdev="__stringify(CONFIG_SYS_MMC_ENV_DEV)"\0" \
	"mmcpart=1\0" \
	"mmcroot=/dev/mmcblk0p2 rootwait rw\0" \
	"mmcargs=setenv bootargs console=${console},${baudrate} " \
		"root=${mmcroot}\0" \
		"loadimage=fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${image}\0" \
		"loadfdt=fatload mmc ${mmcdev}:${mmcpart} ${fdt_addr} ${fdt_file}\0" \
	\
	"bootsd=" \
		"echo Booting from SD card ...; " \
		"run mmcargs; " \
		"mmc dev ${mmcdev};" \
		"run loadimage; " \
		"run loadfdt; " \
		"bootz ${loadaddr} - ${fdt_addr}; " \
		"\0" \
	\
	"bootmix=" \
		"echo Boot Kernel and FDT from TFTP, RootFs from SD card ...; " \
		"run mmcargs; " \
		"mmc dev ${mmcdev};" \
		"tftp ${fdt_addr} ${fdt_file}; " \
		"tftp ${image}; " \
		"bootz ${loadaddr} - ${fdt_addr}; " \
		"\0" \
	\
	"netargs=setenv bootargs console=${console},${baudrate} " \
		"root=/dev/nfs " \
		"ip=dhcp nfsroot=${serverip}:${nfsroot},v3,tcp" \
		"\0" \
	"bootnet=" \
		"echo Booting from net ...; " \
		"run netargs; " \
		"tftp ${image}; " \
		"tftp ${fdt_addr} ${fdt_file}; " \
		"bootz ${loadaddr} - ${fdt_addr}; " \
		"\0"

/* Physical Memory Map */
#define PHYS_SDRAM					MMDC0_ARB_BASE_ADDR

#define CFG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CFG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CFG_SYS_INIT_RAM_SIZE	IRAM_SIZE

/* MMC Config*/
#define CFG_SYS_FSL_ESDHC_ADDR       USDHC1_BASE_ADDR
#define CFG_SYS_FSL_USDHC_NUM		2


/* USB Configs */
#define CFG_MXC_USB_PORTSC  (PORT_PTS_UTMI | PORT_PTS_PTW)

#endif	/* __CONFIG_H */
