/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 Andes Technology Corporation
 * Nobuhiro Lin, Andes Technology Corporation <nobuhiro@andestech.com>
 * Macpaul Lin, Andes Technology Corporation <macpaul@andestech.com>
 */

#ifndef __AE3XX_H
#define __AE3XX_H

/* Hardware register bases */

/* Static Memory Controller (SRAM) */
#define CONFIG_FTSMC020_BASE		0xe0400000
/* DMA Controller */
#define CONFIG_FTDMAC020_BASE		0xf0c00000
/* AHB-to-APB Bridge */
#define CONFIG_FTAPBBRG020S_01_BASE	0xf0000000
/* Reserved */
#define CONFIG_RESERVED_01_BASE		0xe0500000
/* Reserved */
#define CONFIG_RESERVED_02_BASE		0xf0800000
/* Reserved */
#define CONFIG_RESERVED_03_BASE		0xf0900000
/* Ethernet */
#define CONFIG_FTMAC100_BASE		0xe0100000
/* Reserved */
#define CONFIG_RESERVED_04_BASE		0xf1000000

/* APB Device definitions */

/* UART1 */
#define CONFIG_FTUART010_01_BASE	0xf0200000
/* UART2 */
#define CONFIG_FTUART010_02_BASE	0xf0300000
/* Counter/Timers */
#define CONFIG_FTTMR010_BASE		0xf0400000
/* Watchdog Timer */
#define CONFIG_FTWDT010_BASE		0xf0500000
/* Real Time Clock */
#define CONFIG_FTRTC010_BASE		0xf0600000
/* GPIO */
#define CONFIG_FTGPIO010_BASE		0xf0700000
/* I2C */
#define CONFIG_FTIIC010_BASE		0xf0a00000

/* The following address was not defined in Linux */

/* Synchronous Serial Port Controller (SSP) 01 */
#define CONFIG_FTSSP010_01_BASE		0xf0d00000
#endif	/* __AE3XX_H */
