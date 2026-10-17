/* SPDX-License-Identifier: GPL-2.0+
 *
 * Copyright (c) 2021 Linumiz
 * Author: Navin Sankar Velliangiri <navin@linumiz.com>
 */

#ifndef _NPI_IMX6ULL_H
#define _NPI_IMX6ULL_H

#include <linux/sizes.h>
#include "mx6_common.h"

#define CFG_SYS_FSL_USDHC_NUM	1

/* Console configs */
#define CFG_MXC_UART_BASE		UART1_BASE

/* MMC Configs */
#define CFG_SYS_FSL_ESDHC_ADDR	USDHC2_BASE_ADDR

/* Physical Memory Map */
#define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR

#define CFG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CFG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CFG_SYS_INIT_RAM_SIZE	IRAM_SIZE

/* NAND */
#define CFG_SYS_NAND_BASE		0x40000000

/* USB Configs */
#define CFG_MXC_USB_PORTSC		(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CFG_MXC_USB_FLAGS		0

#ifdef CONFIG_CMD_NET
#define CFG_FEC_MXC_PHYADDR		0x1
#endif

#define CFG_FEC_ENET_DEV		1

#define CFG_EXTRA_ENV_SETTINGS \
	"console=ttymxc0,115200n8\0" \
	"image=zImage\0" \
	"fdtfile=imx6ull-seeed-npi-dev-board.dtb\0" \
	"fdt_addr_r=0x82000000\0" \
	"kernel_addr_r=0x81000000\0" \
	"pxefile_addr_r=0x87100000\0" \
	"ramdisk_addr_r=0x82100000\0" \
	"scriptaddr=0x87000000\0" \
	"root=/dev/mmcblk0p2 rootwait\0" \
	BOOTENV

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(UBIFS, ubifs, 0, UBI, boot) \
	func(PXE, pxe, na) \
	func(DHCP, dhcp, na)

#include <config_distro_bootcmd.h>

#endif /* _NPI_IMX6ULL_H */
