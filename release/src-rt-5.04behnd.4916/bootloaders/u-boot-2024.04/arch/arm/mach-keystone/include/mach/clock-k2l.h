/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * K2L: Clock management APIs
 *
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 */

#ifndef __ASM_ARCH_CLOCK_K2L_H
#define __ASM_ARCH_CLOCK_K2L_H

#define PLLSET_CMD_LIST	"<pa|arm|ddr3>"

#define KS2_CLK1_6	sys_clk0_6_clk

#define CORE_PLL_799	{CORE_PLL, 13, 1, 2}
#define CORE_PLL_983	{CORE_PLL, 16, 1, 2}
#define CORE_PLL_1000	{CORE_PLL, 114, 7, 2}
#define CORE_PLL_1167	{CORE_PLL, 19, 1, 2}
#define CORE_PLL_1198	{CORE_PLL, 39, 2, 2}
#define CORE_PLL_1228	{CORE_PLL, 20, 1, 2}
#define PASS_PLL_1228	{PASS_PLL, 20, 1, 2}
#define PASS_PLL_983	{PASS_PLL, 16, 1, 2}
#define PASS_PLL_1050	{PASS_PLL, 205, 12, 2}
#define TETRIS_PLL_491	{TETRIS_PLL, 8, 1, 2}
#define TETRIS_PLL_737	{TETRIS_PLL, 12, 1, 2}
#define TETRIS_PLL_799	{TETRIS_PLL, 13, 1, 2}
#define TETRIS_PLL_983	{TETRIS_PLL, 16, 1, 2}
#define TETRIS_PLL_1000	{TETRIS_PLL, 114, 7, 2}
#define TETRIS_PLL_1167	{TETRIS_PLL, 19, 1, 2}
#define TETRIS_PLL_1198	{TETRIS_PLL, 39, 2, 2}
#define TETRIS_PLL_1228	{TETRIS_PLL, 20, 1, 2}
#define TETRIS_PLL_1352	{TETRIS_PLL, 22, 1, 2}
#define TETRIS_PLL_1401	{TETRIS_PLL, 114, 5, 2}
#define DDR3_PLL_200	{DDR3_PLL, 4, 1, 2}
#define DDR3_PLL_400	{DDR3_PLL, 16, 1, 4}
#define DDR3_PLL_800	{DDR3_PLL, 16, 1, 2}
#define DDR3_PLL_333	{DDR3_PLL, 20, 1, 6}

/* k2l DEV supports 800, 1000, 1200 MHz */
#define DEV_SUPPORTED_SPEEDS	0x383
/* k2l ARM supportd 800, 1000, 1200, 1350, 1400 MHz */
#define ARM_SUPPORTED_SPEEDS	0x3ef

#endif
