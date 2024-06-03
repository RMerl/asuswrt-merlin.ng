/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2017 NXP
 */

#ifndef __LS1088AQDS_QIXIS_H__
#define __LS1088AQDS_QIXIS_H__

/* Definitions of QIXIS Registers for LS1088AQDS */

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

/* BRDCFG2 - SD clock*/
#define QIXIS_SDCLK1_100		0x0
#define QIXIS_SDCLK1_125		0x1
#define QIXIS_SDCLK1_165		0x2
#define QIXIS_SDCLK1_100_SP		0x3

#define BRDCFG4_EMISEL_MASK		0xE0
#define BRDCFG4_EMISEL_SHIFT		5
#define BRDCFG9_SFPTX_MASK		0x10
#define BRDCFG9_SFPTX_SHIFT		4

/* Definitions of QIXIS Registers for LS1088ARDB */

/* BRDCFG5 */
#define BRDCFG5_SPISDHC_MASK		0x0C
#define BRDCFG5_FORCE_SD		0x08

#endif
