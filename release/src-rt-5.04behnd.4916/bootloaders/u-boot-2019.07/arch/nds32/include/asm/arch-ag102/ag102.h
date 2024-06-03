/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2011 Andes Technology Corporation
 * Macpaul Lin, Andes Technology Corporation <macpaul@andestech.com>
 */

#ifndef __AG102_H
#define __AG102_H

/*
 * Hardware register bases
 */

/* PCI Controller */
#define CONFIG_FTPCI100_BASE		0x90000000
/* LPC Controller */
#define CONFIG_LPC_IO_BASE		0x90100000
/* LPC Controller */
#define CONFIG_LPC_BASE			0x90200000

/* NDS32 Data Local Memory 01 */
#define CONFIG_NDS_DLM1_BASE		0x90300000
/* NDS32 Data Local Memory 02 */
#define CONFIG_NDS_DLM2_BASE		0x90400000

/* Synopsys DWC DDR2/1 Controller */
#define CONFIG_DWCDDR21MCTL_BASE	0x90500000
/* DMA Controller */
#define CONFIG_FTDMAC020_BASE		0x90600000
/* FTIDE020_S IDE (ATA) Controller */
#define CONFIG_FTIDE020S_BASE		0x90700000
/* USB OTG Controller */
#define CONFIG_FZOTG266HD0A_BASE	0x90800000
/* Andes L2 Cache Controller */
#define CONFIG_NCEL2C100_BASE		0x90900000
/* XGI XG22 GPU */
#define CONFIG_XGI_XG22_BASE		0x90A00000
/* GMAC Ethernet Controller */
#define CONFIG_FTGMAC100_BASE		0x90B00000
/* AHB Controller */
#define CONFIG_FTAHBC020S_BASE		0x90C00000
/* AHB-to-APB Bridge Controller */
#define CONFIG_FTAPBBRG020S_01_BASE	0x90D00000
/* External AHB2AHB Controller */
#define CONFIG_EXT_AHB2AHB_BASE		0x90E00000
/* Andes Multi-core Interrupt Controller */
#define CONFIG_NCEMIC100_BASE		0x90F00000

/*
 * APB Device definitions
 */
/* Compat Flash Controller */
#define CONFIG_FTCFC010_BASE		0x94000000
/* APB - SSP (SPI) (without AC97) Controller */
#define CONFIG_FTSSP010_01_BASE		0x94100000
/* UART1 - APB STUART Controller (UART0 in Linux) */
#define CONFIG_FTUART010_01_BASE	0x94200000
/* APB - SSP with HDA/AC97 Controller */
#define CONFIG_FTSSP010_02_BASE		0x94500000
/* UART2 - APB STUART Controller (UART1 in Linux) */
#define CONFIG_FTUART010_02_BASE	0x94600000
/* PCU Controller */
#define CONFIG_ANDES_PCU_BASE		0x94800000
/* FTTMR010 Timer */
#define CONFIG_FTTMR010_BASE		0x94900000
/* Watch Dog Controller */
#define CONFIG_FTWDT010_BASE		0x94A00000
/* FTRTC010 Real Time Clock */
#define CONFIG_FTRTC010_BASE		0x98B00000
/* GPIO Controller */
#define CONFIG_FTGPIO010_BASE		0x94C00000
/* I2C Controller */
#define CONFIG_FTIIC010_BASE		0x94E00000
/* PWM - Pulse Width Modulator Controller */
#define CONFIG_FTPWM010_BASE		0x94F00000

/* Debug LED */
#define CONFIG_DEBUG_LED		0x902FFFFC
/* Power Management Unit */
#define CONFIG_FTPMU010_BASE		0x98100000

#endif	/* __AG102_H */
