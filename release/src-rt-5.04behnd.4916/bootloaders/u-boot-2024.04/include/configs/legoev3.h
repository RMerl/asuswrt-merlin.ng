/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 David Lechner <david@lechnology.com>
 *
 * Based on da850evm.h
 *
 * Copyright (C) 2010 Texas Instruments Incorporated - https://www.ti.com/
 *
 * Based on davinci_dvevm.h. Original Copyrights follow:
 *
 * Copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * SoC Configuration
 */
#define CFG_SYS_EXCEPTION_VECTORS_HIGH
#define CFG_SYS_OSCIN_FREQ		24000000
#define CFG_SYS_TIMERBASE		DAVINCI_TIMER0_BASE
#define CFG_SYS_HZ_CLOCK		clk_get(DAVINCI_AUXCLK_CLKID)

/*
 * Memory Info
 */
#define PHYS_SDRAM_1		DAVINCI_DDR_EMIF_DATA_BASE /* DDR Start */
#define PHYS_SDRAM_1_SIZE	(64 << 20) /* SDRAM size 64MB */
#define CFG_MAX_RAM_BANK_SIZE (512 << 20) /* max size from SPRS586*/

/* memtest start addr */

/* memtest will be run on 16MB */

/*
 * Serial Driver info
 */
#define CFG_SYS_NS16550_CLK	clk_get(DAVINCI_UART2_CLKID)

#define CFG_SYS_SPI_CLK		clk_get(DAVINCI_SPI0_CLKID)

/*
 * U-Boot general configuration
 */

/*
 * Linux Information
 */
#define LINUX_BOOT_PARAM_ADDR	(PHYS_SDRAM_1 + 0x100)
#define CFG_EXTRA_ENV_SETTINGS \
	"bootenvfile=uEnv.txt\0" \
	"fdtfile=da850-lego-ev3.dtb\0" \
	"memsize=64M\0" \
	"filesyssize=10M\0" \
	"verify=n\0" \
	"console=ttyS1,115200n8\0" \
	"bootscraddr=0xC0600000\0" \
	"fdtaddr=0xC0600000\0" \
	"loadaddr=0xC0007FC0\0" \
	"filesysaddr=0xC1180000\0" \
	"fwupdateboot=mw 0xFFFF1FFC 0x5555AAAA; reset\0" \
	"importbootenv=echo Importing environment...; " \
		"env import -t ${loadaddr} ${filesize}\0" \
	"loadbootenv=fatload mmc 0 ${loadaddr} ${bootenvfile}\0" \
	"mmcargs=setenv bootargs console=${console} root=/dev/mmcblk0p2 rw " \
		"rootwait ${optargs}\0" \
	"mmcboot=bootm ${loadaddr}\0" \
	"flashargs=setenv bootargs initrd=${filesysaddr},${filesyssize} " \
		"root=/dev/ram0 rw rootfstype=squashfs console=${console} " \
		"${optargs}\0" \
	"flashboot=sf probe 0; " \
		"sf read ${fdtaddr} 0x40000 0x10000; " \
		"sf read ${loadaddr} 0x50000 0x400000; " \
		"sf read ${filesysaddr} 0x450000 0xA00000; " \
		"run fdtfixup; " \
		"run fdtboot\0" \
	"loadimage=fatload mmc 0 ${loadaddr} uImage\0" \
	"loadfdt=fatload mmc 0 ${fdtaddr} ${fdtfile}\0" \
	"fdtfixup=fdt addr ${fdtaddr}; fdt resize; fdt chosen\0" \
	"fdtboot=bootm ${loadaddr} - ${fdtaddr}\0" \
	"loadbootscr=fatload mmc 0 ${bootscraddr} boot.scr\0" \
	"bootscript=source ${bootscraddr}\0"

/* additions for new relocation code, must added to all boards */
#define CFG_SYS_SDRAM_BASE		0xc0000000

#include <asm/arch/hardware.h>

#endif /* __CONFIG_H */
