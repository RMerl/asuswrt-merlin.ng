/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2014 Soeren Moch <smoch@web.de>
 *
 * Configuration settings for the TBS2910 MatrixARM board.
 */

#ifndef __TBS2910_CONFIG_H
#define __TBS2910_CONFIG_H

#include "mx6_common.h"

/* General configuration */

#define CONFIG_MACH_TYPE		3980

#define CONFIG_SYS_HZ			1000

#define CONFIG_IMX_THERMAL

/* Physical Memory Map */
#define CONFIG_SYS_SDRAM_BASE		MMDC0_ARB_BASE_ADDR

#define CONFIG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE	IRAM_SIZE
#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

#define CONFIG_SYS_MALLOC_LEN		(128 * 1024 * 1024)

#define CONFIG_SYS_MEMTEST_START	CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_MEMTEST_END \
	(CONFIG_SYS_MEMTEST_START + 500 * 1024 * 1024)

#define CONFIG_SYS_BOOTMAPSZ		0x10000000

/* Serial console */
#define CONFIG_MXC_UART
#define CONFIG_MXC_UART_BASE		UART1_BASE /* select UART1/UART2 */

/* Ethernet */
#define CONFIG_FEC_MXC
#define CONFIG_FEC_MXC
#define IMX_FEC_BASE			ENET_BASE_ADDR
#define CONFIG_FEC_XCV_TYPE		RGMII
#define CONFIG_ETHPRIME			"FEC"
#define CONFIG_FEC_MXC_PHYADDR		4
#define CONFIG_PHY_ATHEROS

/* Framebuffer */
#ifdef CONFIG_VIDEO
#define CONFIG_VIDEO_BMP_RLE8
#define CONFIG_IMX_HDMI
#define CONFIG_IMX_VIDEO_SKIP
#endif

/* PCI */
#ifdef CONFIG_CMD_PCI
#define CONFIG_PCI_SCAN_SHOW
#define CONFIG_PCIE_IMX
#define CONFIG_PCIE_IMX_PERST_GPIO	IMX_GPIO_NR(7, 12)
#endif

/* SATA */
#ifdef CONFIG_CMD_SATA
#define CONFIG_SYS_SATA_MAX_DEVICE	1
#define CONFIG_DWC_AHSATA_PORT_ID	0
#define CONFIG_DWC_AHSATA_BASE_ADDR	SATA_ARB_BASE_ADDR
#define CONFIG_LBA48
#define CONFIG_SYS_64BIT_LBA
#endif

/* USB */
#ifdef CONFIG_CMD_USB
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET
#define CONFIG_MXC_USB_PORTSC		(PORT_PTS_UTMI | PORT_PTS_PTW)
#ifdef CONFIG_CMD_USB_MASS_STORAGE
#define CONFIG_USBD_HS
#endif /* CONFIG_CMD_USB_MASS_STORAGE */
#ifdef CONFIG_USB_KEYBOARD
#define CONFIG_PREBOOT \
	"usb start; " \
	"if hdmidet; then " \
		"run set_con_hdmi; " \
	"else " \
		"run set_con_serial; " \
	"fi"
#endif /* CONFIG_USB_KEYBOARD */
#endif /* CONFIG_CMD_USB      */

/* Environment organization */
#define CONFIG_SYS_MMC_ENV_DEV		2 /* overwritten on SD boot */
#define CONFIG_SYS_MMC_ENV_PART		1 /* overwritten on SD boot */
#define CONFIG_ENV_SIZE			(8 * 1024)
#define CONFIG_ENV_OFFSET		(384 * 1024)
#define CONFIG_ENV_OVERWRITE

#define CONFIG_BOARD_SIZE_LIMIT		392192 /* (CONFIG_ENV_OFFSET - 1024) */

#define CONFIG_EXTRA_ENV_SETTINGS \
	"bootargs_mmc1=console=ttymxc0,115200 di0_primary console=tty1\0" \
	"bootargs_mmc2=video=mxcfb0:dev=hdmi,1920x1080M@60 " \
			"video=mxcfb1:off video=mxcfb2:off fbmem=28M\0" \
	"bootargs_mmc3=root=/dev/mmcblk0p1 rootwait consoleblank=0 quiet\0" \
	"bootargs_mmc=setenv bootargs ${bootargs_mmc1} ${bootargs_mmc2} " \
			"${bootargs_mmc3}\0" \
	"bootargs_upd=setenv bootargs console=ttymxc0,115200 " \
			"rdinit=/sbin/init enable_wait_mode=off\0" \
	"bootcmd_mmc=run bootargs_mmc; mmc dev 2; " \
			"mmc read 0x10800000 0x800 0x4000; bootm 0x10800000\0" \
	"bootcmd_up1=load mmc 1 0x10800000 uImage\0" \
	"bootcmd_up2=load mmc 1 0x10d00000 uramdisk.img; " \
			"run bootargs_upd; " \
			"bootm 0x10800000 0x10d00000\0" \
	"console=ttymxc0\0" \
	"fan=gpio set 92\0" \
	"set_con_serial=setenv stdout serial; " \
			"setenv stderr serial\0" \
	"set_con_hdmi=setenv stdout serial,vga; " \
			"setenv stderr serial,vga\0" \
	"stderr=serial,vga\0" \
	"stdin=serial,usbkbd\0" \
	"stdout=serial,vga\0"

#define CONFIG_BOOTCOMMAND \
	"mmc rescan; " \
	"if run bootcmd_up1; then " \
		"run bootcmd_up2; " \
	"else " \
		"run bootcmd_mmc; " \
	"fi"

#endif			       /* __TBS2910_CONFIG_H * */
