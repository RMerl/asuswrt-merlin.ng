/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration for Xilinx Versal
 * (C) Copyright 2016 - 2018 Xilinx, Inc.
 * Michal Simek <michal.simek@xilinx.com>
 *
 * Based on Configuration for Xilinx ZynqMP
 */

#ifndef __XILINX_VERSAL_H
#define __XILINX_VERSAL_H

#define CONFIG_REMAKE_ELF

/* #define CONFIG_ARMV8_SWITCH_TO_EL1 */

/* Generic Interrupt Controller Definitions */
#define GICD_BASE	0xF9000000
#define GICR_BASE	0xF9080000

#define CONFIG_SYS_MEMTEST_SCRATCH	0xfffc0000

#define CONFIG_SYS_MEMTEST_START	0
#define CONFIG_SYS_MEMTEST_END		1000

#define CONFIG_SYS_INIT_SP_ADDR		CONFIG_SYS_TEXT_BASE

/* Generic Timer Definitions - setup in EL3. Setup by ATF for other cases */
#if CONFIG_COUNTER_FREQUENCY
# define COUNTER_FREQUENCY	CONFIG_COUNTER_FREQUENCY
#endif

/* Serial setup */
#define CONFIG_ARM_DCC
#define CONFIG_CPU_ARMV8

#define CONFIG_SYS_BAUDRATE_TABLE \
	{ 4800, 9600, 19200, 38400, 57600, 115200 }

/* BOOTP options */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_MAY_FAIL

/* Miscellaneous configurable options */
#define CONFIG_SYS_LOAD_ADDR		0x8000000

/* Monitor Command Prompt */
/* Console I/O Buffer Size */
#define CONFIG_SYS_CBSIZE		2048
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE
#define CONFIG_SYS_MAXARGS		64

/* Ethernet driver */
#if defined(CONFIG_ZYNQ_GEM)
# define CONFIG_NET_MULTI
# define CONFIG_SYS_FAULT_ECHO_LINK_DOWN
# define PHY_ANEG_TIMEOUT       20000
#endif

#define CONFIG_SYS_BOOTM_LEN	(60 * 1024 * 1024)

#define CONFIG_CLOCKS

#define ENV_MEM_LAYOUT_SETTINGS \
	"fdt_high=10000000\0" \
	"initrd_high=10000000\0" \
	"fdt_addr_r=0x40000000\0" \
	"pxefile_addr_r=0x10000000\0" \
	"kernel_addr_r=0x18000000\0" \
	"scriptaddr=0x02000000\0" \
	"ramdisk_addr_r=0x02100000\0" \
	"script_offset_f=0x3f80000\0" \
	"script_size_f=0x80000\0"

#if defined(CONFIG_MMC_SDHCI_ZYNQ)
# define BOOT_TARGET_DEVICES_MMC(func)	func(MMC, mmc, 0) func(MMC, mmc, 1)
#else
# define BOOT_TARGET_DEVICES_MMC(func)
#endif

#if defined(CONFIG_ZYNQMP_GQSPI) || defined(CONFIG_CADENCE_OSPI_VERSAL)
# define BOOT_TARGET_DEVICES_XSPI(func)	func(XSPI, xspi, 0)
#else
# define BOOT_TARGET_DEVICES_XSPI(func)
#endif

#define BOOTENV_DEV_XSPI(devtypeu, devtypel, instance) \
	"bootcmd_xspi0=sf probe 0 0 0 && " \
	"sf read $scriptaddr $script_offset_f $script_size_f && " \
	"source ${scriptaddr}; echo SCRIPT FAILED: continuing...;\0"

#define BOOTENV_DEV_NAME_XSPI(devtypeu, devtypel, instance) \
	"xspi "

#define BOOT_TARGET_DEVICES(func) \
	BOOT_TARGET_DEVICES_MMC(func) \
	BOOT_TARGET_DEVICES_XSPI(func) \
	func(PXE, pxe, na) \
	func(DHCP, dhcp, na)

#include <config_distro_bootcmd.h>

/* Initial environment variables */
#ifndef CONFIG_EXTRA_ENV_SETTINGS
#define CONFIG_EXTRA_ENV_SETTINGS \
	ENV_MEM_LAYOUT_SETTINGS \
	BOOTENV
#endif

#endif /* __XILINX_VERSAL_H */
