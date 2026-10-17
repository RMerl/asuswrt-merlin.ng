/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * am335x_sl50.h
 *
 * Copyright (C) 2015 Toby Churchill Ltd - http://www.toby-churchill.com/
 */

#ifndef __CONFIG_AM335X_EVM_H
#define __CONFIG_AM335X_EVM_H

#include <configs/ti_am335x_common.h>

/* Clock Defines */
#define V_OSCK				24000000  /* Clock output from T2 */
#define V_SCLK				(V_OSCK)

#define MEM_LAYOUT_ENV_SETTINGS \
	"scriptaddr=0x80000000\0" \
	"pxefile_addr_r=0x80100000\0" \
	"kernel_addr_r=0x82000000\0" \
	"fdt_addr_r=0x88000000\0" \
	"ramdisk_addr_r=0x88080000\0" \

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(MMC, mmc, 1)

#define AM335XX_BOARD_FDTFILE \
	"fdtfile=am335x-sl50.dtb\0" \

#include <config_distro_bootcmd.h>

#define CFG_EXTRA_ENV_SETTINGS \
	AM335XX_BOARD_FDTFILE \
	MEM_LAYOUT_ENV_SETTINGS \
	BOOTENV

/* NS16550 Configuration */
#define CFG_SYS_NS16550_COM1		0x44e09000	/* Base EVM has UART0 */
#define CFG_SYS_NS16550_COM2		0x48022000	/* UART1 */
#define CFG_SYS_NS16550_COM3		0x48024000	/* UART2 */
#define CFG_SYS_NS16550_COM4		0x481a6000	/* UART3 */
#define CFG_SYS_NS16550_COM5		0x481a8000	/* UART4 */
#define CFG_SYS_NS16550_COM6		0x481aa000	/* UART5 */

#endif	/* ! __CONFIG_AM335X_SL50_H */
