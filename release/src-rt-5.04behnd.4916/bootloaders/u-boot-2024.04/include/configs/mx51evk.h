/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2007, Guennadi Liakhovetski <lg@denx.de>
 *
 * (C) Copyright 2009 Freescale Semiconductor, Inc.
 *
 * Configuration settings for the MX51EVK Board
 */

#ifndef __CONFIG_H
#define __CONFIG_H

 /* High Level Configuration Options */

#include <asm/arch/imx-regs.h>

/*
 * Hardware drivers
 */

#define CFG_MXC_UART_BASE	UART1_BASE

/* PMIC Controller */
#define CFG_FSL_PMIC_BUS	0
#define CFG_FSL_PMIC_CS	0
#define CFG_FSL_PMIC_CLK	2500000
#define CFG_FSL_PMIC_MODE	(SPI_MODE_0 | SPI_CS_HIGH)
#define CFG_FSL_PMIC_BITLEN	32

/*
 * MMC Configs
 * */
#define CFG_SYS_FSL_ESDHC_ADDR	MMC_SDHC1_BASE_ADDR

/* USB Configs */
#define CFG_MXC_USB_PORT	1
#define CFG_MXC_USB_PORTSC	PORT_PTS_ULPI
#define CFG_MXC_USB_FLAGS	MXC_EHCI_POWER_PINS_ENABLED

/* Framebuffer and LCD */

#define CFG_EXTRA_ENV_SETTINGS \
	"script=boot.scr\0" \
	"image=zImage\0" \
	"fdt_file=imx51-babbage.dtb\0" \
	"fdt_addr=0x91000000\0" \
	"boot_fdt=try\0" \
	"ip_dyn=yes\0" \
	"mmcdev=0\0" \
	"mmcpart=1\0" \
	"mmcroot=/dev/mmcblk0p2 rootwait rw\0" \
	"mmcargs=setenv bootargs console=ttymxc0,${baudrate} " \
		"root=${mmcroot}\0" \
	"loadbootscript=" \
		"fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${script};\0" \
	"bootscript=echo Running bootscript from mmc ...; " \
		"source\0" \
	"loadimage=fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${image}\0" \
	"loadfdt=fatload mmc ${mmcdev}:${mmcpart} ${fdt_addr} ${fdt_file}\0" \
	"mmcboot=echo Booting from mmc ...; " \
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
	"netargs=setenv bootargs console=ttymxc0,${baudrate} " \
		"root=/dev/nfs " \
		"ip=dhcp nfsroot=${serverip}:${nfsroot},v3,tcp\0" \
	"netboot=echo Booting from net ...; " \
		"run netargs; " \
		"if test ${ip_dyn} = yes; then " \
			"setenv get_cmd dhcp; " \
		"else " \
			"setenv get_cmd tftp; " \
		"fi; " \
		"${get_cmd} ${image}; " \
		"if test ${boot_fdt} = yes ||  test ${boot_fdt} = try; then " \
			"if ${get_cmd} ${fdt_addr} ${fdt_file}; then " \
				"bootz ${loadaddr} - ${fdt_addr}; " \
			"else " \
				"if test ${boot_fdt} = try; then " \
					"bootz; " \
				"else " \
					"echo ERROR: Cannot load the DT; " \
					"exit; " \
				"fi; " \
			"fi; " \
		"else " \
			"bootz; " \
		"fi;\0"

/*
 * Miscellaneous configurable options
 */

/*-----------------------------------------------------------------------
 * Physical Memory Map
 */
#define PHYS_SDRAM_1		CSD0_BASE_ADDR
#define PHYS_SDRAM_1_SIZE	(512 * 1024 * 1024)

#define CFG_SYS_SDRAM_BASE		(PHYS_SDRAM_1)
#define CFG_SYS_INIT_RAM_ADDR	(IRAM_BASE_ADDR)
#define CFG_SYS_INIT_RAM_SIZE	(IRAM_SIZE)

#define CFG_SYS_DDR_CLKSEL	0
#define CFG_SYS_CLKTL_CBCDR	0x59E35100
#define CFG_SYS_MAIN_PWR_ON

/*-----------------------------------------------------------------------
 * environment organization
 */

#endif
