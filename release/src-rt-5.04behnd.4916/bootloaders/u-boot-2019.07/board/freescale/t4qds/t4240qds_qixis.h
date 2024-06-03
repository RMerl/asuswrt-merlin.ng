/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2012 Freescale Semiconductor, Inc.
 */

#ifndef __T4020QDS_QIXIS_H__
#define __T4020QDS_QIXIS_H__

/* Definitions of QIXIS Registers for T4020QDS */

/* BRDCFG4[4:7]] select EC1 and EC2 as a pair */
#define BRDCFG4_EMISEL_MASK		0xE0
#define BRDCFG4_EMISEL_SHIFT		5

/* SYSCLK */
#define QIXIS_SYSCLK_66			0x0
#define QIXIS_SYSCLK_83			0x1
#define QIXIS_SYSCLK_100		0x2
#define QIXIS_SYSCLK_125		0x3
#define QIXIS_SYSCLK_133		0x4
#define QIXIS_SYSCLK_150		0x5
#define QIXIS_SYSCLK_160		0x6
#define QIXIS_SYSCLK_166		0x7

/* DDRCLK */
#define QIXIS_DDRCLK_66			0x0
#define QIXIS_DDRCLK_100		0x1
#define QIXIS_DDRCLK_125		0x2
#define QIXIS_DDRCLK_133		0x3

#define BRDCFG5_IRE			0x20	/* i2c Remote i2c1 enable */

#define BRDCFG12_SD3EN_MASK		0x20
#define BRDCFG12_SD3MX_MASK		0x08
#define BRDCFG12_SD3MX_SLOT5		0x08
#define BRDCFG12_SD3MX_SLOT6		0x00
#define BRDCFG12_SD4EN_MASK		0x04
#define BRDCFG12_SD4MX_MASK		0x03
#define BRDCFG12_SD4MX_SLOT7		0x02
#define BRDCFG12_SD4MX_SLOT8		0x01
#define BRDCFG12_SD4MX_AURO_SATA	0x00
#endif
