/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * K2E: Clock management APIs
 *
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 */

#ifndef __ASM_ARCH_CLOCK_K2E_H
#define __ASM_ARCH_CLOCK_K2E_H

#define PLLSET_CMD_LIST	"<pa|ddr3>"

#define KS2_CLK1_6	sys_clk0_6_clk

#define CORE_PLL_800	{CORE_PLL, 16, 1, 2}
#define CORE_PLL_850	{CORE_PLL, 17, 1, 2}
#define CORE_PLL_1000	{CORE_PLL, 20, 1, 2}
#define CORE_PLL_1200	{CORE_PLL, 24, 1, 2}
#define PASS_PLL_1000	{PASS_PLL, 20, 1, 2}
#define CORE_PLL_1250	{CORE_PLL, 25, 1, 2}
#define CORE_PLL_1350	{CORE_PLL, 27, 1, 2}
#define CORE_PLL_1400	{CORE_PLL, 28, 1, 2}
#define CORE_PLL_1500	{CORE_PLL, 30, 1, 2}
#define DDR3_PLL_200	{DDR3_PLL, 4,  1, 2}
#define DDR3_PLL_400	{DDR3_PLL, 16, 1, 4}
#define DDR3_PLL_800	{DDR3_PLL, 16, 1, 2}
#define DDR3_PLL_333	{DDR3_PLL, 20, 1, 6}

/* k2e DEV supports 800, 850, 1000, 1250, 1350, 1400, 1500 MHz */
#define DEV_SUPPORTED_SPEEDS	0xFFF
#define ARM_SUPPORTED_SPEEDS	0

#endif
