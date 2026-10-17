/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * USB armory MkI board configuration settings
 * http://inversepath.com/usbarmory
 *
 * Copyright (C) 2015, Inverse Path
 * Andrej Rosano <andrej@inversepath.com>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <asm/arch/imx-regs.h>

/* U-Boot environment */

/* U-Boot general configurations */

/* UART */
#define CFG_MXC_UART_BASE	UART1_BASE

/* SD/MMC */
#define CFG_SYS_FSL_ESDHC_ADDR	0

/* USB */
#define CFG_MXC_USB_PORT	1
#define CFG_MXC_USB_PORTSC	(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CFG_MXC_USB_FLAGS	0

/* Linux boot */

#define BOOT_TARGET_DEVICES(func) func(MMC, mmc, 0)

#include <config_distro_bootcmd.h>

#define MEM_LAYOUT_ENV_SETTINGS			\
	"kernel_addr_r=0x70800000\0"		\
	"fdt_addr_r=0x71000000\0"		\
	"scriptaddr=0x70800000\0"		\
	"pxefile_addr_r=0x70800000\0"		\
	"ramdisk_addr_r=0x73000000\0"

#define CFG_EXTRA_ENV_SETTINGS				\
	MEM_LAYOUT_ENV_SETTINGS					\
	"bootargs_default=root=/dev/mmcblk0p1 rootwait rw\0"	\
	"fdtfile=imx53-usbarmory.dtb\0"				\
	"console=ttymxc0,115200\0"				\
	BOOTENV

#ifndef CONFIG_CMDLINE
#define USBARMORY_FIT_PATH	"/boot/usbarmory.itb"
#define USBARMORY_FIT_ADDR	"0x70800000"
#endif

/* Physical Memory Map */
#define PHYS_SDRAM			CSD0_BASE_ADDR
#define PHYS_SDRAM_SIZE			(gd->ram_size)

#define CFG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CFG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CFG_SYS_INIT_RAM_SIZE	IRAM_SIZE

#endif				/* __CONFIG_H */
