/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration for Xilinx Versal
 * (C) Copyright 2016 - 2018 Xilinx, Inc.
 * Michal Simek <michal.simek@amd.com>
 *
 * Based on Configuration for Xilinx ZynqMP
 */

#ifndef __XILINX_VERSAL_H
#define __XILINX_VERSAL_H

/* Generic Interrupt Controller Definitions */
#define GICD_BASE	0xF9000000
#define GICR_BASE	0xF9080000

/* Serial setup */
#define CFG_SYS_BAUDRATE_TABLE \
	{ 4800, 9600, 19200, 38400, 57600, 115200 }

/* GUID for capsule updatable firmware image */
#define XILINX_BOOT_IMAGE_GUID \
	EFI_GUID(0x20c5fba5, 0x0171, 0x457f, 0xb9, 0xcd, \
		 0xf5, 0x12, 0x9c, 0xd0, 0x72, 0x28)

#if defined(CONFIG_CMD_DFU)
#define DFU_DEFAULT_POLL_TIMEOUT	300
#endif

/* Ethernet driver */
#if defined(CONFIG_ZYNQ_GEM)
# define PHY_ANEG_TIMEOUT       20000
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
# define BOOT_TARGET_DEVICES_XSPI(func)	func(XSPI, xspi, 0)
#else
# define BOOT_TARGET_DEVICES_XSPI(func)
#endif

#define BOOTENV_DEV_XSPI(devtypeu, devtypel, instance) \
	"bootcmd_xspi0=sf probe 0 0 0 && " \
	"sf read $scriptaddr $script_offset_f $script_size_f && " \
	"echo XSPI: Trying to boot script at ${scriptaddr} && " \
	"source ${scriptaddr}; echo XSPI: SCRIPT FAILED: continuing...;\0"

#define BOOTENV_DEV_NAME_XSPI(devtypeu, devtypel, instance) \
	"xspi0 "

#define BOOT_TARGET_DEVICES_JTAG(func)	func(JTAG, jtag, na)

#define BOOTENV_DEV_JTAG(devtypeu, devtypel, instance) \
	"bootcmd_jtag=echo JTAG: Trying to boot script at ${scriptaddr} && " \
		"source ${scriptaddr}; echo JTAG: SCRIPT FAILED: continuing...;\0"

#define BOOTENV_DEV_NAME_JTAG(devtypeu, devtypel, instance) \
	"jtag "

#define BOOT_TARGET_DEVICES_USB_DFU(func) \
	func(USB_DFU, usb_dfu, 0) func(USB_DFU, usb_dfu, 1)

#define BOOTENV_DEV_USB_DFU(devtypeu, devtypel, instance) \
	"bootcmd_" #devtypel #instance "=setenv dfu_alt_info boot.scr ram " \
	"$scriptaddr $script_size_f && " \
	"dfu " #instance " ram " #instance " 60 && " \
	"echo DFU" #instance ": Trying to boot script at ${scriptaddr} && " \
	"source ${scriptaddr}; " \
	"echo DFU" #instance ": SCRIPT FAILED: continuing...;\0"

#define BOOTENV_DEV_NAME_USB_DFU(devtypeu, devtypel, instance) \
	""

#define BOOT_TARGET_DEVICES_USB_THOR(func) \
	func(USB_THOR, usb_thor, 0) func(USB_THOR, usb_thor, 1)

#define BOOTENV_DEV_USB_THOR(devtypeu, devtypel, instance) \
	"bootcmd_" #devtypel #instance "=setenv dfu_alt_info boot.scr ram " \
	"$scriptaddr $script_size_f && " \
	"thordown " #instance " ram " #instance " && " \
	"echo THOR" #instance ": Trying to boot script at ${scriptaddr} && " \
	"source ${scriptaddr}; " \
	"echo THOR" #instance ": SCRIPT FAILED: continuing...;\0"

#define BOOTENV_DEV_NAME_USB_THOR(devtypeu, devtypel, instance) \
	""

#define BOOT_TARGET_DEVICES(func) \
	BOOT_TARGET_DEVICES_JTAG(func) \
	BOOT_TARGET_DEVICES_MMC(func) \
	BOOT_TARGET_DEVICES_XSPI(func) \
	BOOT_TARGET_DEVICES_USB_DFU(func) \
	BOOT_TARGET_DEVICES_USB_THOR(func) \
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
	BOOTENV
#endif

#endif /* __XILINX_VERSAL_H */
