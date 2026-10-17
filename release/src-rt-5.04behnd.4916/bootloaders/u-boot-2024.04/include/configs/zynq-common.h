/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2012 Michal Simek <monstr@monstr.eu>
 * (C) Copyright 2013 - 2018 Xilinx, Inc.
 *
 * Common configuration options for all Zynq boards.
 */

#ifndef __CONFIG_ZYNQ_COMMON_H
#define __CONFIG_ZYNQ_COMMON_H

/* Cache options */
#ifndef CONFIG_SYS_L2CACHE_OFF
# define CFG_SYS_PL310_BASE		0xf8f02000
#endif

#define ZYNQ_SCUTIMER_BASEADDR		0xF8F00600
#define CFG_SYS_TIMERBASE		ZYNQ_SCUTIMER_BASEADDR
#define CFG_SYS_TIMER_COUNTER	(CFG_SYS_TIMERBASE + 0x4)

/* GUIDs for capsule updatable firmware images */
#define XILINX_BOOT_IMAGE_GUID \
	EFI_GUID(0x1ba29a15, 0x9969, 0x40aa, 0xb4, 0x24, \
		 0xe8, 0x61, 0x21, 0x61, 0x86, 0x64)

#define XILINX_UBOOT_IMAGE_GUID \
	EFI_GUID(0x1a5178f0, 0x87d3, 0x4f36, 0xac, 0x63, \
		 0x3b, 0x31, 0xa2, 0x3b, 0xe3, 0x05)

/* Serial drivers */
/* The following table includes the supported baudrates */
#define CFG_SYS_BAUDRATE_TABLE  \
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400}

/* Ethernet driver */

/* NOR */

#ifdef CONFIG_USB_EHCI_ZYNQ
# define DFU_DEFAULT_POLL_TIMEOUT	300
#endif

/* enable preboot to be loaded before CONFIG_BOOTDELAY */

/* Boot configuration */

#ifdef CONFIG_SPL_BUILD
#define BOOTENV
#else

#ifdef CONFIG_CMD_MMC
#define BOOT_TARGET_DEVICES_MMC(func) func(MMC, mmc, 0) func(MMC, mmc, 1)
#else
#define BOOT_TARGET_DEVICES_MMC(func)
#endif

#ifdef CONFIG_CMD_USB
#define BOOT_TARGET_DEVICES_USB(func) func(USB, usb, 0) func(USB, usb, 1)
#else
#define BOOT_TARGET_DEVICES_USB(func)
#endif

#if defined(CONFIG_CMD_PXE) && defined(CONFIG_CMD_DHCP)
#define BOOT_TARGET_DEVICES_PXE(func) func(PXE, pxe, na)
#else
#define BOOT_TARGET_DEVICES_PXE(func)
#endif

#if defined(CONFIG_CMD_DHCP)
#define BOOT_TARGET_DEVICES_DHCP(func) func(DHCP, dhcp, na)
#else
#define BOOT_TARGET_DEVICES_DHCP(func)
#endif

#if defined(CONFIG_ZYNQ_QSPI)
# define BOOT_TARGET_DEVICES_QSPI(func)	func(QSPI, qspi, na)
#else
# define BOOT_TARGET_DEVICES_QSPI(func)
#endif

#if defined(CONFIG_NAND_ZYNQ)
# define BOOT_TARGET_DEVICES_NAND(func)	func(NAND, nand, na)
#else
# define BOOT_TARGET_DEVICES_NAND(func)
#endif

#if defined(CONFIG_MTD_NOR_FLASH)
# define BOOT_TARGET_DEVICES_NOR(func)	func(NOR, nor, na)
#else
# define BOOT_TARGET_DEVICES_NOR(func)
#endif

#define BOOTENV_DEV_QSPI(devtypeu, devtypel, instance) \
	"bootcmd_qspi=sf probe 0 0 0 && " \
		      "sf read ${scriptaddr} ${script_offset_f} ${script_size_f} && " \
		      "echo QSPI: Trying to boot script at ${scriptaddr} && " \
		      "source ${scriptaddr}; echo QSPI: SCRIPT FAILED: continuing...;\0"

#define BOOTENV_DEV_NAME_QSPI(devtypeu, devtypel, instance) \
	"qspi "

#define BOOTENV_DEV_NAND(devtypeu, devtypel, instance) \
	"bootcmd_nand=nand info && " \
		      "nand read ${scriptaddr} ${script_offset_f} ${script_size_f} && " \
		      "echo NAND: Trying to boot script at ${scriptaddr} && " \
		      "source ${scriptaddr}; echo NAND: SCRIPT FAILED: continuing...;\0"

#define BOOTENV_DEV_NAME_NAND(devtypeu, devtypel, instance) \
	"nand "

#define BOOTENV_DEV_NOR(devtypeu, devtypel, instance) \
	"script_offset_nor=0xE2FC0000\0"        \
	"bootcmd_nor=cp.b ${script_offset_nor} ${scriptaddr} ${script_size_f} && " \
		     "echo NOR: Trying to boot script at ${scriptaddr} && " \
		     "source ${scriptaddr}; echo NOR: SCRIPT FAILED: continuing...;\0"

#define BOOTENV_DEV_NAME_NOR(devtypeu, devtypel, instance) \
	"nor "

#define BOOT_TARGET_DEVICES_JTAG(func)  func(JTAG, jtag, na)

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
	BOOT_TARGET_DEVICES_NOR(func) \
	BOOT_TARGET_DEVICES_USB_DFU(func) \
	BOOT_TARGET_DEVICES_USB_THOR(func) \
	BOOT_TARGET_DEVICES_USB(func) \
	BOOT_TARGET_DEVICES_PXE(func) \
	BOOT_TARGET_DEVICES_DHCP(func)

#include <config_distro_bootcmd.h>
#endif /* CONFIG_SPL_BUILD */

/* Default environment */
#ifndef CFG_EXTRA_ENV_SETTINGS
#define CFG_EXTRA_ENV_SETTINGS	\
	"script_size_f=0x40000\0"	\
	"fdt_addr_r=0x1f00000\0"        \
	"pxefile_addr_r=0x2000000\0"    \
	"kernel_addr_r=0x2000000\0"     \
	"ramdisk_addr_r=0x3100000\0"    \
	BOOTENV
#endif

/* Miscellaneous configurable options */

#define CFG_SYS_INIT_RAM_ADDR	0xFFFF0000
#define CFG_SYS_INIT_RAM_SIZE	0x2000


/* Extend size of kernel image for uncompression */

/* Address in RAM where the parameters must be copied by SPL. */

/* Not using MMC raw mode - just for compilation purpose */

/* qspi mode is working fine */
#ifdef CONFIG_ZYNQ_QSPI
#define CFG_SYS_SPI_ARGS_OFFS	0x200000
#define CFG_SYS_SPI_ARGS_SIZE	0x80000
#define CFG_SYS_SPI_KERNEL_OFFS	(CFG_SYS_SPI_ARGS_OFFS + \
					CFG_SYS_SPI_ARGS_SIZE)
#endif

/* SP location before relocation, must use scratch RAM */

/* 3 * 64kB blocks of OCM - one is on the top because of bootrom */

/* On the top of OCM space */

/*
 * SPL stack position - and stack goes down
 * 0xfffffe00 is used for putting wfi loop.
 * Set it up as limit for now.
 */

#endif /* __CONFIG_ZYNQ_COMMON_H */
