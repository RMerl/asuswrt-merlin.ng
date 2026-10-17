/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2010-2011 Freescale Semiconductor, Inc.
 *
 * Configuration settings for the Boundary Devices Nitrogen6X
 * and Freescale i.MX6Q Sabre Lite boards.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include "mx6_common.h"

#define CFG_MXC_UART_BASE	       UART2_BASE

/* MMC Configs */
#define CFG_SYS_FSL_ESDHC_ADDR      0
#define CFG_SYS_FSL_USDHC_NUM       2

#define IMX_FEC_BASE			ENET_BASE_ADDR
#define CFG_FEC_MXC_PHYADDR		6

/* USB Configs */
#define CFG_MXC_USB_PORTSC	(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CFG_MXC_USB_FLAGS	0

#ifdef CONFIG_CMD_MMC
#define DISTRO_BOOT_DEV_MMC(func) func(MMC, mmc, 0) func(MMC, mmc, 1)
#else
#define DISTRO_BOOT_DEV_MMC(func)
#endif

#ifdef CONFIG_CMD_SATA
#define DISTRO_BOOT_DEV_SATA(func) func(SATA, sata, 0)
#else
#define DISTRO_BOOT_DEV_SATA(func)
#endif

#ifdef CONFIG_USB_STORAGE
#define DISTRO_BOOT_DEV_USB(func) func(USB, usb, 0)
#else
#define DISTRO_BOOT_DEV_USB(func)
#endif

#ifdef CONFIG_CMD_PXE
#define DISTRO_BOOT_DEV_PXE(func) func(PXE, pxe, na)
#else
#define DISTRO_BOOT_DEV_PXE(func)
#endif

#ifdef CONFIG_CMD_DHCP
#define DISTRO_BOOT_DEV_DHCP(func) func(DHCP, dhcp, na)
#else
#define DISTRO_BOOT_DEV_DHCP(func)
#endif

#define BOOT_TARGET_DEVICES(func) \
	DISTRO_BOOT_DEV_MMC(func) \
	DISTRO_BOOT_DEV_SATA(func) \
	DISTRO_BOOT_DEV_USB(func) \
	DISTRO_BOOT_DEV_PXE(func) \
	DISTRO_BOOT_DEV_DHCP(func)

#include <config_distro_bootcmd.h>
#include <linux/stringify.h>

#define CFG_EXTRA_ENV_SETTINGS \
	"console=ttymxc1\0" \
	"fdt_high=0xffffffff\0" \
	"initrd_high=0xffffffff\0" \
	"fdt_addr_r=0x18000000\0" \
	"fdtfile=" CONFIG_DEFAULT_DEVICE_TREE ".dtb\0" \
	"kernel_addr_r=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0"  \
	"pxefile_addr_r=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
	"scriptaddr=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
	"ramdisk_addr_r=0x13000000\0" \
	"ramdiskaddr=0x13000000\0" \
	"ip_dyn=yes\0" \
	"usb_pgood_delay=2000\0" \
	BOOTENV

/* Miscellaneous configurable options */

/* Physical Memory Map */
#define PHYS_SDRAM		       MMDC0_ARB_BASE_ADDR

#define CFG_SYS_SDRAM_BASE	       PHYS_SDRAM
#define CFG_SYS_INIT_RAM_ADDR       IRAM_BASE_ADDR
#define CFG_SYS_INIT_RAM_SIZE       IRAM_SIZE

/* Environment organization */

#endif	       /* __CONFIG_H */
