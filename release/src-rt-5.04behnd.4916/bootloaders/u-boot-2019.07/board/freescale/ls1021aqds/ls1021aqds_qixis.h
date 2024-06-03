/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 */

#ifndef __LS1021AQDS_QIXIS_H__
#define __LS1021AQDS_QIXIS_H__

/* Definitions of QIXIS Registers for LS1021AQDS */

/* BRDCFG4[4:7]] select EC1 and EC2 as a pair */
#define BRDCFG4_EMISEL_MASK		0xe0
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
#define QIXIS_SYSCLK_64			0x8

/* DDRCLK */
#define QIXIS_DDRCLK_66			0x0
#define QIXIS_DDRCLK_100		0x1
#define QIXIS_DDRCLK_125		0x2
#define QIXIS_DDRCLK_133		0x3

#define QIXIS_SRDS1CLK_100		0x0

#define QIXIS_DCU_BRDCFG5		0x55

#endif
