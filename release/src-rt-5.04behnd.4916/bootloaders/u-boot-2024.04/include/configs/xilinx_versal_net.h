/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Configuration for Xilinx Versal NET
 * Copyright (C) 2016 - 2022, Xilinx, Inc.
 * Copyright (C) 2022, Advanced Micro Devices, Inc.
 *
 * Michal Simek <michal.simek@amd.com>
 *
 * Based on Configuration for Xilinx ZynqMP
 */

#ifndef __XILINX_VERSAL_NET_H
#define __XILINX_VERSAL_NET_H

/* FIXME this is causing issue at least on IPP */
/* #define CONFIG_ARMV8_SWITCH_TO_EL1 */

/* Generic Interrupt Controller Definitions */
#define GICD_BASE	0xe2000000
#define GICR_BASE	0xe2060000

/* Serial setup */
#define CFG_SYS_BAUDRATE_TABLE \
	{ 4800, 9600, 19200, 38400, 57600, 115200 }

#if defined(CONFIG_CMD_DFU)
#define DFU_DEFAULT_POLL_TIMEOUT	300
#define DFU_ALT_INFO_RAM \
	"dfu_ram_info=" \
	"setenv dfu_alt_info " \
	"Image ram 80000 $kernel_size_r\\\\;" \
	"system.dtb ram $fdt_addr_r $fdt_size_r\0" \
	"dfu_ram=run dfu_ram_info && dfu 0 ram 0\0" \
	"thor_ram=run dfu_ram_info && thordown 0 ram 0\0"

#define DFU_ALT_INFO  \
		DFU_ALT_INFO_RAM
#endif

#if !defined(DFU_ALT_INFO)
# define DFU_ALT_INFO
#endif

/* Ethernet driver */
#if defined(CONFIG_ZYNQ_GEM)
# define PHY_ANEG_TIMEOUT	20000
#endif

#define ENV_MEM_LAYOUT_SETTINGS \
	"fdt_addr_r=0x40000000\0" \
	"fdt_size_r=0x400000\0" \
	"pxefile_addr_r=0x10000000\0" \
	"kernel_addr_r=0x18000000\0" \
	"kernel_size_r=0x10000000\0" \
	"kernel_comp_addr_r=0x30000000\0" \
	"kernel_comp_size=0x3C00000\0" \
	"ramdisk_addr_r=0x02100000\0" \
	"script_size_f=0x80000\0"

#if defined(CONFIG_DISTRO_DEFAULTS)

#if defined(CONFIG_MMC_SDHCI_ZYNQ)
# define BOOT_TARGET_DEVICES_MMC(func)	func(MMC, mmc, 0) func(MMC, mmc, 1)
#else
# define BOOT_TARGET_DEVICES_MMC(func)
#endif

#if defined(CONFIG_CMD_PXE) && defined(CONFIG_CMD_DHCP)
# define BOOT_TARGET_DEVICES_PXE(func)	func(PXE, pxe, na)
#else
# define BOOT_TARGET_DEVICES_PXE(func)
#endif

#if defined(CONFIG_CMD_DHCP)
# define BOOT_TARGET_DEVICES_DHCP(func)	func(DHCP, dhcp, na)
#else
# define BOOT_TARGET_DEVICES_DHCP(func)
#endif

#if defined(CONFIG_ZYNQMP_GQSPI) || defined(CONFIG_CADENCE_OSPI_VERSAL)
# define BOOT_TARGET_DEVICES_XSPI(func)	func(XSPI, xspi, 0) func(XSPI, xspi, 1)
# define BOOTENV_DEV_SHARED_XSPI \
	"xspi_boot=sf probe $devnum_xspi:0 0 0 && " \
	"sf read $scriptaddr $script_offset_f $script_size_f && " \
	"echo XSPI: Trying to boot script at ${scriptaddr} && " \
	"source ${scriptaddr}; echo XSPI: SCRIPT FAILED: continuing...;\0"
#else
# define BOOT_TARGET_DEVICES_XSPI(func)
# define BOOTENV_DEV_SHARED_XSPI
#endif

#define BOOTENV_DEV_XSPI(devtypeu, devtypel, instance) \
	"bootcmd_" #devtypel #instance "=" \
	"devnum_xspi=" #instance "; run " #devtypel "_boot\0" \

#define BOOTENV_DEV_NAME_XSPI(devtypeu, devtypel, instance) \
	""

#define BOOT_TARGET_DEVICES_JTAG(func)	func(JTAG, jtag, na)

#define BOOTENV_DEV_JTAG(devtypeu, devtypel, instance) \
	"bootcmd_jtag=echo JTAG: Trying to boot script at ${scriptaddr} && " \
		"source ${scriptaddr}; echo JTAG: SCRIPT FAILED: continuing...;\0"

#define BOOTENV_DEV_NAME_JTAG(devtypeu, devtypel, instance) \
	"jtag "

#define BOOT_TARGET_DEVICES_DFU_USB(func)  func(DFU_USB, dfu_usb, 0)

#define BOOTENV_DEV_DFU_USB(devtypeu, devtypel, instance) \
	"bootcmd_dfu_usb=setenv dfu_alt_info boot.scr ram $scriptaddr " \
	"$script_size_f; dfu 0 ram 0 && " \
	"echo DFU: Trying to boot script at ${scriptaddr} && " \
	"source ${scriptaddr}; " \
	"echo DFU: SCRIPT FAILED: continuing...;\0"

#define BOOTENV_DEV_NAME_DFU_USB(devtypeu, devtypel, instance) \
	""

#define BOOT_TARGET_DEVICES(func) \
	BOOT_TARGET_DEVICES_JTAG(func) \
	BOOT_TARGET_DEVICES_MMC(func) \
	BOOT_TARGET_DEVICES_XSPI(func) \
	BOOT_TARGET_DEVICES_DFU_USB(func) \
	BOOT_TARGET_DEVICES_PXE(func) \
	BOOT_TARGET_DEVICES_DHCP(func)

#include <config_distro_bootcmd.h>

#else /* CONFIG_DISTRO_DEFAULTS */
# define BOOTENV
#endif /* CONFIG_DISTRO_DEFAULTS */

/* Initial environment variables */
#ifndef CFG_EXTRA_ENV_SETTINGS
#define CFG_EXTRA_ENV_SETTINGS \
	ENV_MEM_LAYOUT_SETTINGS \
	BOOTENV \
	BOOTENV_DEV_SHARED_XSPI \
	DFU_ALT_INFO
#endif

#endif /* __XILINX_VERSAL_NET_H */
