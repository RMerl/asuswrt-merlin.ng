/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * K2HK: Clock management APIs
 *
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 */

#ifndef __ASM_ARCH_CLOCK_K2HK_H
#define __ASM_ARCH_CLOCK_K2HK_H

#define PLLSET_CMD_LIST		"<pa|arm|ddr3a|ddr3b>"

#define KS2_CLK1_6 sys_clk0_6_clk

#define CORE_PLL_799    {CORE_PLL,	13,	1,	2}
#define CORE_PLL_983    {CORE_PLL,	16,	1,	2}
#define CORE_PLL_999	{CORE_PLL,	122,	15,	1}
#define CORE_PLL_1167   {CORE_PLL,	19,	1,	2}
#define CORE_PLL_1228   {CORE_PLL,	20,	1,	2}
#define CORE_PLL_1200	{CORE_PLL,	625,	32,	2}
#define PASS_PLL_1228   {PASS_PLL,	20,	1,	2}
#define PASS_PLL_983    {PASS_PLL,	16,	1,	2}
#define PASS_PLL_1050   {PASS_PLL,	205,    12,	2}
#define TETRIS_PLL_500  {TETRIS_PLL,	8,	1,	2}
#define TETRIS_PLL_750  {TETRIS_PLL,	12,	1,	2}
#define TETRIS_PLL_800	{TETRIS_PLL,	32,	5,	1}
#define TETRIS_PLL_687  {TETRIS_PLL,	11,	1,	2}
#define TETRIS_PLL_625  {TETRIS_PLL,	10,	1,	2}
#define TETRIS_PLL_812  {TETRIS_PLL,	13,	1,	2}
#define TETRIS_PLL_875  {TETRIS_PLL,	14,	1,	2}
#define TETRIS_PLL_1000	{TETRIS_PLL,	40,	5,	1}
#define TETRIS_PLL_1188 {TETRIS_PLL,	19,	2,	1}
#define TETRIS_PLL_1200 {TETRIS_PLL,	48,	5,	1}
#define TETRIS_PLL_1350	{TETRIS_PLL,	54,	5,	1}
#define TETRIS_PLL_1375 {TETRIS_PLL,	22,	2,	1}
#define TETRIS_PLL_1400 {TETRIS_PLL,	56,	5,	1}
#define DDR3_PLL_200(x)	{DDR3##x##_PLL,	4,	1,	2}
#define DDR3_PLL_400(x)	{DDR3##x##_PLL,	16,	1,	4}
#define DDR3_PLL_800(x)	{DDR3##x##_PLL,	16,	1,	2}
#define DDR3_PLL_333(x)	{DDR3##x##_PLL,	20,	1,	6}

/* k2h DEV supports 800, 1000, 1200 MHz */
#define DEV_SUPPORTED_SPEEDS	0x383
/* k2h ARM supportd 800, 1000, 1200, 1350, 1400 MHz */
#define ARM_SUPPORTED_SPEEDS	0x3EF

#endif
