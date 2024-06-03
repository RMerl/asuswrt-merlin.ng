/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Qualcomm APQ8096 sysmap
 *
 * (C) Copyright 2017 Jorge Ramirez-Ortiz <jorge.ramirez-ortiz@linaro.org>
 */
#ifndef _MACH_SYSMAP_APQ8096_H
#define _MACH_SYSMAP_APQ8096_H

#define TLMM_BASE_ADDR			(0x1010000)

/* Strength (sdc1) */
#define SDC1_HDRV_PULL_CTL_REG		(TLMM_BASE_ADDR + 0x0012D000)

/* Clocks: (from CLK_CTL_BASE)  */
#define GPLL0_STATUS			(0x0000)
#define APCS_GPLL_ENA_VOTE		(0x52000)
#define APCS_CLOCK_BRANCH_ENA_VOTE	(0x52004)

#define SDCC2_BCR			(0x14000) /* block reset */
#define SDCC2_APPS_CBCR			(0x14004) /* branch control */
#define SDCC2_AHB_CBCR			(0x14008)
#define SDCC2_CMD_RCGR			(0x14010)
#define SDCC2_CFG_RCGR			(0x14014)
#define SDCC2_M				(0x14018)
#define SDCC2_N				(0x1401C)
#define SDCC2_D				(0x14020)

#define BLSP2_AHB_CBCR			(0x25004)
#define BLSP2_UART2_APPS_CBCR		(0x29004)
#define BLSP2_UART2_APPS_CMD_RCGR	(0x2900C)
#define BLSP2_UART2_APPS_CFG_RCGR	(0x29010)
#define BLSP2_UART2_APPS_M		(0x29014)
#define BLSP2_UART2_APPS_N		(0x29018)
#define BLSP2_UART2_APPS_D		(0x2901C)

#endif
