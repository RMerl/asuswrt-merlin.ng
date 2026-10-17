/* SPDX-License-Identifier: GPL-2.0+
 *
 * Copyright (C) 2020 Linumiz
 * Author: Parthiban Nallathambi <parthiban@linumiz.com>
 */

#ifndef __MYS_6ULX_H
#define __MYS_6ULX_H

#include <linux/sizes.h>
#include "mx6_common.h"

#define CFG_SYS_FSL_USDHC_NUM	1

/* Console configs */
#define CFG_MXC_UART_BASE		UART1_BASE

/* MMC Configs */
#define CFG_SYS_FSL_ESDHC_ADDR	USDHC2_BASE_ADDR

/* Physical Memory Map */
#define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR
#define PHYS_SDRAM_SIZE			SZ_256M

#define CFG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CFG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CFG_SYS_INIT_RAM_SIZE	IRAM_SIZE

/* NAND */
#define CFG_SYS_NAND_BASE		0x40000000

/* USB Configs */
#define CFG_MXC_USB_PORTSC		(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CFG_MXC_USB_FLAGS		0

#define CFG_EXTRA_ENV_SETTINGS \
	"console=ttymxc0,115200n8\0" \
	"fdt_addr_r=0x82000000\0" \
	"fdt_high=0xffffffff\0" \
	"initrd_high=0xffffffff\0" \
	"kernel_addr_r=0x81000000\0" \
	"pxefile_addr_r=0x87100000\0" \
	"ramdisk_addr_r=0x82100000\0" \
	"scriptaddr=0x87000000\0" \
	BOOTENV

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(UBIFS, ubifs, 0, UBI, boot) \
	func(PXE, pxe, na) \
	func(DHCP, dhcp, na)

#include <config_distro_bootcmd.h>

#endif /* __MYS_6ULX_H */
