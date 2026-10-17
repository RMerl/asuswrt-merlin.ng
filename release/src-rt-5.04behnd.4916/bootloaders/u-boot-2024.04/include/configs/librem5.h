/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2017 NXP
 * Copyright 2018 Emcraft Systems
 * Copyright 2022 Purism
 *
 */

#ifndef __LIBREM5_H
#define __LIBREM5_H

/* #define DEBUG */

#include <version.h>
#include <linux/sizes.h>
#include <asm/arch/imx-regs.h>

#define CFG_SYS_FSL_USDHC_NUM	2

#define CONSOLE_ON_UART1

#ifdef CONSOLE_ON_UART1
#define CFG_MXC_UART_BASE		UART1_BASE_ADDR
#define CONSOLE_UART_CLK		0
#define CONSOLE		"ttymxc0"
#elif defined(CONSOLE_ON_UART2)
#define CFG_MXC_UART_BASE		UART2_BASE_ADDR
#define CONSOLE_UART_CLK		1
#define CONSOLE		"ttymxc1"
#elif defined(CONSOLE_ON_UART3)
#define CFG_MXC_UART_BASE		UART3_BASE_ADDR
#define CONSOLE_UART_CLK		2
#define CONSOLE		"ttymxc2"
#elif defined(CONSOLE_ON_UART4)
#define CFG_MXC_UART_BASE		UART4_BASE_ADDR
#define CONSOLE_UART_CLK		3
#define CONSOLE		"ttymxc3"
#else
#define CFG_MXC_UART_BASE		UART1_BASE_ADDR
#define CONSOLE_UART_CLK		0
#define CONSOLE		"ttymxc0"
#endif

#ifndef CONFIG_SPL_BUILD
#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(USB, usb, 0) \
	func(DHCP, dhcp, na)
#include <config_distro_bootcmd.h>
#else
#define BOOTENV
#endif

/* Initial environment variables */
#define CFG_EXTRA_ENV_SETTINGS		\
	"scriptaddr=0x80000000\0" \
	"pxefile_addr_r=0x80100000\0" \
	"kernel_addr_r=0x80800000\0" \
	"kernel_comp_addr_r=0x90000000\0" \
	"kernel_comp_size=0x08000000\0" \
	"fdt_addr_r=0x84800000\0" \
	"ramdisk_addr_r=0x85000000\0" \
	"console=" CONSOLE ",115200\0" \
	"bootargs=u_boot_version=" PLAIN_VERSION "\0" \
	"stdin=usbacm,serial\0" \
	"stdout=usbacm,serial\0" \
	"stderr=usbacm,serial\0" \
	BOOTENV

/* Link Definitions */

#define CFG_SYS_INIT_RAM_ADDR        0x40000000
#define CFG_SYS_INIT_RAM_SIZE        0x80000

#define CFG_SYS_SDRAM_BASE           0x40000000
#define PHYS_SDRAM                      0x40000000
#define PHYS_SDRAM_SIZE			0xc0000000 /* 3GB LPDDR4 one Rank */

/* Monitor Command Prompt */

#define CFG_SYS_FSL_ESDHC_ADDR       0

#endif
