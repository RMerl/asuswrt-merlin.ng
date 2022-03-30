/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2012 Freescale Semiconductor, Inc.
 */

#ifndef __B4860QDS_QIXIS_H__
#define __B4860QDS_QIXIS_H__

/* Definitions of QIXIS Registers for B4860QDS */

/* BRDCFG4[4:7]] select EC1 and EC2 as a pair */
#define BRDCFG4_EMISEL_MASK		0xE0
#define BRDCFG4_EMISEL_SHIFT		5

/* CLK */
#define QIXIS_CLK_66		0x0
#define QIXIS_CLK_100		0x1
#define QIXIS_CLK_125		0x2
#define QIXIS_CLK_133		0x3

#define QIXIS_SRDS1CLK_122		0x5a
#define QIXIS_SRDS1CLK_125		0x5e

/* SGMII */
#define PHY_BASE_ADDR		0x18
#define PORT_NUM		0x04
#define REGNUM			0x00
#endif
