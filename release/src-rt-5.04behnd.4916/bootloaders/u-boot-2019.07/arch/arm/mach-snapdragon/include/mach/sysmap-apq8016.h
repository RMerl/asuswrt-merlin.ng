/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Qualcomm APQ8916 sysmap
 *
 * (C) Copyright 2015 Mateusz Kulikowski <mateusz.kulikowski@gmail.com>
 */
#ifndef _MACH_SYSMAP_APQ8016_H
#define _MACH_SYSMAP_APQ8016_H

#define GICD_BASE			(0x0b000000)
#define GICC_BASE			(0x0a20c000)

/* Clocks: (from CLK_CTL_BASE)  */
#define GPLL0_STATUS			(0x2101C)
#define APCS_GPLL_ENA_VOTE		(0x45000)
#define APCS_CLOCK_BRANCH_ENA_VOTE (0x45004)

#define SDCC_BCR(n)			((n * 0x1000) + 0x41000)
#define SDCC_CMD_RCGR(n)		((n * 0x1000) + 0x41004)
#define SDCC_CFG_RCGR(n)		((n * 0x1000) + 0x41008)
#define SDCC_M(n)			((n * 0x1000) + 0x4100C)
#define SDCC_N(n)			((n * 0x1000) + 0x41010)
#define SDCC_D(n)			((n * 0x1000) + 0x41014)
#define SDCC_APPS_CBCR(n)		((n * 0x1000) + 0x41018)
#define SDCC_AHB_CBCR(n)		((n * 0x1000) + 0x4101C)

/* BLSP1 AHB clock (root clock for BLSP) */
#define BLSP1_AHB_CBCR			0x1008

/* Uart clock control registers */
#define BLSP1_UART2_BCR			(0x3028)
#define BLSP1_UART2_APPS_CBCR		(0x302C)
#define BLSP1_UART2_APPS_CMD_RCGR	(0x3034)
#define BLSP1_UART2_APPS_CFG_RCGR	(0x3038)
#define BLSP1_UART2_APPS_M		(0x303C)
#define BLSP1_UART2_APPS_N		(0x3040)
#define BLSP1_UART2_APPS_D		(0x3044)

#endif
