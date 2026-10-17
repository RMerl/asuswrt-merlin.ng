/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * DHCOM DH-iMX6 PDK board configuration
 *
 * Copyright (C) 2017 Marek Vasut <marex@denx.de>
 */

#ifndef __DH_IMX6_CONFIG_H
#define __DH_IMX6_CONFIG_H

#include <asm/arch/imx-regs.h>

#include "mx6_common.h"

/*
 * SPI NOR layout:
 * 0x00_0000-0x00_ffff ... U-Boot SPL
 * 0x01_0000-0x0f_ffff ... U-Boot
 * 0x10_0000-0x10_ffff ... U-Boot env #1
 * 0x11_0000-0x11_ffff ... U-Boot env #2
 * 0x12_0000-0x1f_ffff ... UNUSED
 */

/* Miscellaneous configurable options */

/* MMC Configs */
#define CFG_SYS_FSL_ESDHC_ADDR	0
#define CFG_SYS_FSL_USDHC_NUM	3

/* UART */
#define CFG_MXC_UART_BASE		UART1_BASE

/* USB Configs */
#define CFG_MXC_USB_PORTSC		(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CFG_MXC_USB_FLAGS		0

/* USB Gadget (DFU, UMS) */
#if defined(CONFIG_CMD_DFU) || defined(CONFIG_CMD_USB_MASS_STORAGE)
#define DFU_DEFAULT_POLL_TIMEOUT	300
#endif

#define CFG_EXTRA_ENV_SETTINGS	\
	"console=ttymxc0,115200\0"	\
	"fdt_addr=0x18000000\0"		\
	"fdt_high=0xffffffff\0"		\
	"initrd_high=0xffffffff\0"	\
	"kernel_addr_r=0x10008000\0"	\
	"fdt_addr_r=0x13000000\0"	\
	"ramdisk_addr_r=0x18000000\0"	\
	"scriptaddr=0x14000000\0"	\
	"fdtfile=imx6q-dhcom-pdk2.dtb\0"\
	"update_sf=" /* Erase SPI NOR and install U-Boot from SD */	\
		"load mmc 0:1 ${loadaddr} /boot/u-boot-with-spl.imx && "\
		"sf probe && sf update ${loadaddr} 0x400 ${filesize}\0"	\
	BOOTENV

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(MMC, mmc, 1) \
	func(MMC, mmc, 2) \
	func(USB, usb, 1) \
	func(SATA, sata, 0) \
	func(DHCP, dhcp, na)

#include <config_distro_bootcmd.h>

/* Physical Memory Map */
#define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR

#define CFG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CFG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CFG_SYS_INIT_RAM_SIZE	IRAM_SIZE

/* Environment */

#endif	/* __DH_IMX6_CONFIG_H */
