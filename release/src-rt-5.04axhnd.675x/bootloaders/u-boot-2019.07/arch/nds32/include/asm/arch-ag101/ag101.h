/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2011 Andes Technology Corporation
 * Nobuhiro Lin, Andes Technology Corporation <nobuhiro@andestech.com>
 * Macpaul Lin, Andes Technology Corporation <macpaul@andestech.com>
 */

#ifndef __AG101_H
#define __AG101_H

/* Hardware register bases */

/* AHB Controller */
#define CONFIG_FTAHBC020S_BASE		0x90100000
/* Static Memory Controller (SRAM) */
#define CONFIG_FTSMC020_BASE		0x90200000
/* FTSDMC021 SDRAM Controller */
#define CONFIG_FTSDMC021_BASE		0x90300000
/* DMA Controller */
#define CONFIG_FTDMAC020_BASE		0x90400000
/* AHB-to-APB Bridge */
#define CONFIG_FTAPBBRG020S_01_BASE	0x90500000
/* LCD Controller */
#define CONFIG_FTLCDC100_BASE		0x90600000
/* Reserved */
#define CONFIG_RESERVED_01_BASE		0x90700000
/* Reserved */
#define CONFIG_RESERVED_02_BASE		0x90800000
/* Ethernet */
#define CONFIG_FTMAC100_BASE		0x90900000
/* External USB host */
#define CONFIG_EXT_USB_HOST_BASE	0x90A00000
/* USB Device */
#define CONFIG_USB_DEV_BASE		0x90B00000
/* External AHB-to-PCI Bridge (FTPCI100 not exist in ag101) */
#define CONFIG_EXT_AHBPCIBRG_BASE	0x90C00000
/* Reserved */
#define CONFIG_RESERVED_03_BASE		0x90D00000
/* External AHB-to-APB Bridger (FTAPBBRG020S_02) */
#define CONFIG_EXT_AHBAPBBRG_BASE	0x90E00000
/* External AHB slave1 (LCD) */
#define CONFIG_EXT_AHBSLAVE01_BASE	0x90F00000
/* External AHB slave2 (FUSBH200) */
#define CONFIG_EXT_AHBSLAVE02_BASE	0x92000000

/* DEBUG LED */
#define CONFIG_DEBUG_LED		0x902FFFFC

/* APB Device definitions */

/* Power Management Unit */
#define CONFIG_FTPMU010_BASE		0x98100000
/* BT UART 2/IrDA (UART 01 in Linux) */
#define CONFIG_FTUART010_01_BASE	0x98300000
/* Counter/Timers */
#define CONFIG_FTTMR010_BASE		0x98400000
/* Watchdog Timer */
#define CONFIG_FTWDT010_BASE		0x98500000
/* Real Time Clock */
#define CONFIG_FTRTC010_BASE		0x98600000
/* GPIO */
#define CONFIG_FTGPIO010_BASE		0x98700000
/* Interrupt Controller */
#define CONFIG_FTINTC010_BASE		0x98800000
/* I2C */
#define CONFIG_FTIIC010_BASE		0x98A00000
/* Reserved */
#define CONFIG_RESERVED_04_BASE		0x98C00000
/* Compat Flash Controller */
#define CONFIG_FTCFC010_BASE		0x98D00000

/* Synchronous Serial Port Controller (SSP) I2S/AC97 */
#define CONFIG_FTSSP010_02_BASE		0x99400000
/* ST UART ? SSP 02 (UART 02 in Linux) */
#define CONFIG_FTUART010_02_BASE	0x99600000

/* The following address was not defined in Linux */

/* FF UART 3 */
#define CONFIG_FTUART010_03_BASE	0x98200000
/* Synchronous Serial Port Controller (SSP) 01 */
#define CONFIG_FTSSP010_01_BASE		0x98B00000
/* IrDA */
#define CONFIG_IRDA_BASE		0x98900000
/* PWM - Pulse Width Modulator Controller */
#define CONFIG_PMW_BASE			0x99100000

#endif	/* __AG101_H */
