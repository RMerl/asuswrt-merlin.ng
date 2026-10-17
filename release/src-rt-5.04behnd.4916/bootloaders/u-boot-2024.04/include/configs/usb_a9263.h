/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2007-2013
 * Stelian Pop <stelian.pop@leadtechdesign.com>
 * Lead Tech Design <www.leadtechdesign.com>
 * Thomas Petazzoni, Free Electrons, <thomas.petazzoni@free-electrons.com>
 * Mateusz Kulikowski <mateusz.kulikowski@gmail.com>
 *
 * Settings for Calao USB-A9263 board
 *
 * U-Boot image has to be less than 200704 bytes, otherwise at91bootstrap
 * installed on board will not be able to load it properly.
 */

#ifndef __CONFIG_H
#define __CONFIG_H
#include <asm/hardware.h>

/* ARM asynchronous clock */
#define CFG_SYS_AT91_MAIN_CLOCK	12000000	/* 12 MHz crystal */
#define CFG_SYS_AT91_SLOW_CLOCK	32768

/*
 * Hardware drivers
 */

/* SDRAM */
#define CFG_SYS_SDRAM_BASE		ATMEL_BASE_CS1
#define CFG_SYS_SDRAM_SIZE		0x04000000

#define CFG_SYS_INIT_RAM_SIZE	(16 * 1024)
#define CFG_SYS_INIT_RAM_ADDR	ATMEL_BASE_SRAM1

/* NAND flash */
#ifdef CONFIG_CMD_NAND
#define CFG_SYS_NAND_BASE			ATMEL_BASE_CS3
/* our ALE is AD21 */
#define CFG_SYS_NAND_MASK_ALE		(1 << 21)
/* our CLE is AD22 */
#define CFG_SYS_NAND_MASK_CLE		(1 << 22)
#define CFG_SYS_NAND_ENABLE_PIN		GPIO_PIN_PD(15)
#define CFG_SYS_NAND_READY_PIN		GPIO_PIN_PA(22)
#endif

/* bootstrap + u-boot + env + linux in dataflash on CS0 */
#define CFG_EXTRA_ENV_SETTINGS \

#endif
