/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Bluewater Systems Snapper 9G45 module
 *
 * (C) Copyright 2011 Bluewater Systems
 *   Author: Andre Renaud <andre@bluewatersys.com>
 *   Author: Ryan Mallon <ryan@bluewatersys.com>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* SoC type is defined in boards.cfg */
#include <asm/hardware.h>
#include <linux/sizes.h>

/* ARM asynchronous clock */
#define CFG_SYS_AT91_MAIN_CLOCK      12000000 /* from 12 MHz crystal */
#define CFG_SYS_AT91_SLOW_CLOCK	32768

/* CPU */

/* SDRAM */
#define CFG_SYS_SDRAM_BASE		ATMEL_BASE_CS6
#define CFG_SYS_SDRAM_SIZE		(128 * 1024 * 1024) /* 64MB */
#define CFG_SYS_INIT_RAM_SIZE	0x1000
#define CFG_SYS_INIT_RAM_ADDR	ATMEL_BASE_SRAM

/* Mem test settings */

/* NAND Flash */
#define CFG_SYS_NAND_BASE		ATMEL_BASE_CS3
#define CFG_SYS_NAND_MASK_ALE	(1 << 21) /* AD21 */
#define CFG_SYS_NAND_MASK_CLE	(1 << 22) /* AD22 */
#define CFG_SYS_NAND_ENABLE_PIN	AT91_PIN_PC14
#define CFG_SYS_NAND_READY_PIN	AT91_PIN_PC8

/* UARTs/Serial console */

/* Boot options */

/* Environment settings */

#define	CFG_EXTRA_ENV_SETTINGS	\
	"ethaddr=00:00:00:00:00:00\0" \
	"serial=0\0" \
	"stdout=serial_atmel\0" \
	"stderr=serial_atmel\0" \
	"stdin=serial_atmel\0" \
	"bootlimit=3\0" \
	"loadaddr=0x71000000\0" \
	"board_rev=2\0" \
	"bootfile=/tftpboot/uImage\0" \
	"bootargs_def=console=ttyS0,115200 panic=5 quiet lpj=997376\0" \
	"nfsroot=/export/root\0" \
	"boot_working=setenv bootargs $bootargs_def; nboot $loadaddr 0 0x20c0000 && bootm\0" \
	"boot_safe=setenv bootargs $bootargs_def; nboot $loadaddr 0 0xc0000 && bootm\0" \
	"boot_tftp=setenv bootargs $bootargs_def ip=any nfsroot=$nfsroot; setenv autoload y && bootp && bootm\0" \
	"boot_usb=setenv bootargs $bootargs_def; usb start && usb storage && fatload usb 0:1 $loadaddr dds-xm200.bin && bootm\0" \
	"boot_mmc=setenv bootargs $bootargs_def; mmc rescan && fatload mmc 0:1 $loadaddr dds-xm200.bin && bootm\0" \
	"bootcmd=run boot_mmc ; run boot_usb ; run boot_working ; run boot_safe\0" \
	"altbootcmd=run boot_mmc ; run boot_usb ; run boot_safe ; run boot_working\0"

/* Console settings */

#endif /* __CONFIG_H */
