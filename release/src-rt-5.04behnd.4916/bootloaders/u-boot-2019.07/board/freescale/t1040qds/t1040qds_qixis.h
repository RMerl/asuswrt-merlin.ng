/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2013 Freescale Semiconductor, Inc.
 */

#ifndef __T1040QDS_QIXIS_H__
#define __T1040QDS_QIXIS_H__

/* Definitions of QIXIS Registers for T1040QDS */

/* BRDCFG4[4:7]] select EC1 and EC2 as a pair */
#define BRDCFG4_EMISEL_MASK		0xE0
#define BRDCFG4_EMISEL_SHIFT		5

/* BRDCFG5[0:1] controls routing and use of I2C3 & I2C4 ports*/
#define BRDCFG5_IMX_MASK		0xC0
#define BRDCFG5_IMX_DIU			0x80

/* BRDCFG9[2] controls EPHY2 Clock */
#define BRDCFG9_EPHY2_MASK              0x20
#define BRDCFG9_EPHY2_VAL               0x00

/* BRDCFG15[3] controls LCD Panel Powerdown*/
#define BRDCFG15_LCDPD_MASK		0x10
#define BRDCFG15_LCDPD_ENABLED		0x00

/* BRDCFG15[6:7] controls DIU MUX selction*/
#define BRDCFG15_DIUSEL_MASK		0x03
#define BRDCFG15_DIUSEL_HDMI		0x00

/* SYSCLK */
#define QIXIS_SYSCLK_66			0x0
#define QIXIS_SYSCLK_83			0x1
#define QIXIS_SYSCLK_100		0x2
#define QIXIS_SYSCLK_125		0x3
#define QIXIS_SYSCLK_133		0x4
#define QIXIS_SYSCLK_150		0x5
#define QIXIS_SYSCLK_160		0x6
#define QIXIS_SYSCLK_166		0x7
#define QIXIS_SYSCLK_64			0x8

/* DDRCLK */
#define QIXIS_DDRCLK_66			0x0
#define QIXIS_DDRCLK_100		0x1
#define QIXIS_DDRCLK_125		0x2
#define QIXIS_DDRCLK_133		0x3


#define QIXIS_SRDS1CLK_122		0x5a
#define QIXIS_SRDS1CLK_125		0x5e
#endif
