/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015  Beckhoff Automation GmbH & Co. KG
 * Patrick Bruenn <p.bruenn@beckhoff.com>
 *
 * Configuration settings for Beckhoff CX9020.
 *
 * Based on Freescale's Linux i.MX mx53loco.h file:
 * Copyright (C) 2010-2011 Freescale Semiconductor.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <asm/arch/imx-regs.h>

#define CFG_MXC_UART_BASE UART2_BASE

/* MMC Configs */
#define CFG_SYS_FSL_ESDHC_ADDR	0

/* bootz: zImage/initrd.img support */


/* USB Configs */
#define CFG_MXC_USB_PORT	1
#define CFG_MXC_USB_PORTSC	(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CFG_MXC_USB_FLAGS	0

/* Command definition */

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(MMC, mmc, 1) \
	func(USB, usb, 0) \
	func(PXE, pxe, na)

#include <config_distro_bootcmd.h>

#define CFG_EXTRA_ENV_SETTINGS \
	"fdt_addr_r=0x75000000\0" \
	"pxefile_addr_r=0x73000000\0" \
	"scriptaddr=0x74000000\0" \
	"ramdisk_addr_r=0x80000000\0" \
	"kernel_addr_r=0x72000000\0"  \
	"fdt_high=0xffffffff\0" \
	"console=ttymxc1,115200\0" \
	"stdin=serial\0" \
	"stdout=serial,vidconsole\0" \
	"stderr=serial,vidconsole\0" \
	"fdtfile=imx53-cx9020.dtb\0" \
	BOOTENV

/* Miscellaneous configurable options */

/* Physical Memory Map */
#define PHYS_SDRAM_1			CSD0_BASE_ADDR
#define PHYS_SDRAM_1_SIZE		(gd->bd->bi_dram[0].size)
#define PHYS_SDRAM_2			CSD1_BASE_ADDR
#define PHYS_SDRAM_2_SIZE		(gd->bd->bi_dram[1].size)
#define PHYS_SDRAM_SIZE			(gd->ram_size)

#define CFG_SYS_SDRAM_BASE		(PHYS_SDRAM_1)
#define CFG_SYS_INIT_RAM_ADDR	(IRAM_BASE_ADDR)
#define CFG_SYS_INIT_RAM_SIZE	(IRAM_SIZE)

/* environment organization */

#endif /* __CONFIG_H */
