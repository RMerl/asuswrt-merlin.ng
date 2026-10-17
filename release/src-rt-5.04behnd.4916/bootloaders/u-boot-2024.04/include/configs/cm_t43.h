/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * cm_t43.h
 *
 * Copyright (C) 2015 Compulab, Ltd.
 */

#ifndef __CONFIG_CM_T43_H
#define __CONFIG_CM_T43_H

#define CFG_MAX_RAM_BANK_SIZE	(2048 << 20)	/* 2GB */
#define CFG_SYS_TIMERBASE		0x48040000	/* Use Timer2 */

#include <asm/arch/omap.h>

/* Serial support */
#define CFG_SYS_NS16550_CLK		48000000
#define CFG_SYS_NS16550_COM1		0x44e09000

/* NAND support */
#define CFG_SYS_NAND_ECCSIZE		512
#define CFG_SYS_NAND_ECCBYTES	14
#define CFG_SYS_NAND_ECCPOS		{ 2, 3, 4, 5, 6, 7, 8, 9, \
					 10, 11, 12, 13, 14, 15, 16, 17, \
					 18, 19, 20, 21, 22, 23, 24, 25, \
					 26, 27, 28, 29, 30, 31, 32, 33, \
					 34, 35, 36, 37, 38, 39, 40, 41, \
					 42, 43, 44, 45, 46, 47, 48, 49, \
					 50, 51, 52, 53, 54, 55, 56, 57, }

/* Enabling L2 Cache */
#define CFG_SYS_PL310_BASE		0x48242000

/*
 * Since SPL did pll and ddr initialization for us,
 * we don't need to do it twice.
 */

#include <configs/ti_armv7_omap.h>

#define V_OSCK				24000000  /* Clock output from T2 */
#define V_SCLK				(V_OSCK)

#define CFG_EXTRA_ENV_SETTINGS \
	"loadaddr=0x80200000\0" \
	"fdtaddr=0x81200000\0" \
	"bootm_size=0x8000000\0" \
	"console=ttyO0,115200n8\0" \
	"fdtfile=am437x-sb-som-t43.dtb\0" \
	"kernel=zImage-cm-t43\0" \
	"bootscr=bootscr.img\0" \
	"emmcroot=/dev/mmcblk0p2 rw\0" \
	"emmcrootfstype=ext4 rootwait\0" \
	"emmcargs=setenv bootargs console=${console} " \
		"root=${emmcroot} " \
		"rootfstype=${emmcrootfstype}\0" \
	"loadbootscript=load mmc 0 ${loadaddr} ${bootscr}\0" \
	"bootscript=echo Running bootscript from mmc ...; " \
		"source ${loadaddr}\0" \
	"emmcboot=echo Booting from emmc ... && " \
		"run emmcargs && " \
		"load mmc 1 ${loadaddr} ${kernel} && " \
		"load mmc 1 ${fdtaddr} ${fdtfile} && " \
		"bootz ${loadaddr} - ${fdtaddr}\0"

/* SPL defines. */

/* EEPROM */

#endif	/* __CONFIG_CM_T43_H */
