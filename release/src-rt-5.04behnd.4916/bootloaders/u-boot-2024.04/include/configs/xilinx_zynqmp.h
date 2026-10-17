/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration for Xilinx ZynqMP
 * (C) Copyright 2014 - 2015 Xilinx, Inc.
 * Michal Simek <michal.simek@amd.com>
 *
 * Based on Configuration for Versatile Express
 */

#ifndef __XILINX_ZYNQMP_H
#define __XILINX_ZYNQMP_H

/* Generic Interrupt Controller Definitions */
#define GICD_BASE	0xF9010000
#define GICC_BASE	0xF9020000

/* Serial setup */
#define CFG_SYS_BAUDRATE_TABLE \
	{ 4800, 9600, 19200, 38400, 57600, 115200 }

/* GUIDs for capsule updatable firmware images */
#define XILINX_BOOT_IMAGE_GUID \
	EFI_GUID(0xde6066e8, 0x0256, 0x4fad, 0x82, 0x38, \
		 0xe4, 0x06, 0xe2, 0x74, 0xc4, 0xcf)

#define XILINX_UBOOT_IMAGE_GUID \
	EFI_GUID(0xcf9ecfd4, 0x938b, 0x41c5, 0x85, 0x51, \
		 0x1f, 0x88, 0x3a, 0xb7, 0xdc, 0x18)

/* Miscellaneous configurable options */

#if defined(CONFIG_USB_STORAGE)
#define DFU_DEFAULT_POLL_TIMEOUT	300

# define PARTS_DEFAULT \
	"partitions=uuid_disk=${uuid_gpt_disk};" \
	"name=""boot"",size=16M,uuid=${uuid_gpt_boot};" \
	"name=""Linux"",size=-M,uuid=${uuid_gpt_Linux}\0"
#endif

#if !defined(PARTS_DEFAULT)
# define PARTS_DEFAULT
#endif

/* Console I/O Buffer Size */

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
	"script_size_f=0x80000\0" \
	"stdin=serial\0" \
	"stdout=serial,vidconsole\0" \
	"stderr=serial,vidconsole\0" \

#if defined(CONFIG_DISTRO_DEFAULTS)

#if defined(CONFIG_MMC_SDHCI_ZYNQ)
# define BOOT_TARGET_DEVICES_MMC(func)	func(MMC, mmc, 0) func(MMC, mmc, 1)
#else
# define BOOT_TARGET_DEVICES_MMC(func)
#endif

#if defined(CONFIG_SATA_CEVA)
# define BOOT_TARGET_DEVICES_SCSI(func)	func(SCSI, scsi, 0)
#else
# define BOOT_TARGET_DEVICES_SCSI(func)
#endif

#if defined(CONFIG_USB_STORAGE)
# define BOOT_TARGET_DEVICES_USB(func)	func(USB, usb, 0) func(USB, usb, 1)
#else
# define BOOT_TARGET_DEVICES_USB(func)
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

#if defined(CONFIG_ZYNQMP_GQSPI)
# define BOOT_TARGET_DEVICES_QSPI(func)	func(QSPI, qspi, 0)
#else
# define BOOT_TARGET_DEVICES_QSPI(func)
#endif

#if defined(CONFIG_NAND_ARASAN)
# define BOOT_TARGET_DEVICES_NAND(func)	func(NAND, nand, 0)
#else
# define BOOT_TARGET_DEVICES_NAND(func)
#endif

#define BOOTENV_DEV_QSPI(devtypeu, devtypel, instance) \
	"bootcmd_" #devtypel #instance "=sf probe " #instance " 0 0 && " \
		       "sf read $scriptaddr $script_offset_f $script_size_f && " \
		       "echo QSPI: Trying to boot script at ${scriptaddr} && " \
		       "source ${scriptaddr}; echo QSPI: SCRIPT FAILED: continuing...;\0"

#define BOOTENV_DEV_NAME_QSPI(devtypeu, devtypel, instance) \
	#devtypel #instance " "

#define BOOTENV_DEV_NAND(devtypeu, devtypel, instance) \
	"bootcmd_" #devtypel #instance "= nand info && " \
		       "nand read $scriptaddr $script_offset_f $script_size_f && " \
		       "echo NAND: Trying to boot script at ${scriptaddr} && " \
		       "source ${scriptaddr}; echo NAND: SCRIPT FAILED: continuing...;\0"

#define BOOTENV_DEV_NAME_NAND(devtypeu, devtypel, instance) \
	#devtypel #instance " "

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
	BOOT_TARGET_DEVICES_QSPI(func) \
	BOOT_TARGET_DEVICES_NAND(func) \
	BOOT_TARGET_DEVICES_USB_DFU(func) \
	BOOT_TARGET_DEVICES_USB_THOR(func) \
	BOOT_TARGET_DEVICES_USB(func) \
	BOOT_TARGET_DEVICES_SCSI(func) \
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

/* SPL can't handle all huge variables - define just DFU */
#if defined(CONFIG_SPL_BUILD) && defined(CONFIG_SPL_DFU)
#undef CFG_EXTRA_ENV_SETTINGS
# define CFG_EXTRA_ENV_SETTINGS \
	"dfu_alt_info_ram=uboot.bin ram 0x8000000 0x1000000;" \
			  "atf-uboot.ub ram 0x10000000 0x1000000;" \
			  "Image ram 0x80000 0x3f80000;" \
			  "system.dtb ram 0x4000000 0x100000\0" \
	"dfu_bufsiz=0x1000\0"
#endif

#if defined(CONFIG_SPL_SPI_FLASH_SUPPORT)
# define CFG_SYS_SPI_KERNEL_OFFS	0x80000
# define CFG_SYS_SPI_ARGS_OFFS	0xa0000
# define CFG_SYS_SPI_ARGS_SIZE	0xa0000
#endif

/* u-boot is like dtb */

/* ATF is my kernel image */

#ifdef CONFIG_SPL_SYS_MALLOC_SIMPLE
# error "Disable CONFIG_SPL_SYS_MALLOC_SIMPLE. Full malloc needs to be used"
#endif

#endif /* __XILINX_ZYNQMP_H */
