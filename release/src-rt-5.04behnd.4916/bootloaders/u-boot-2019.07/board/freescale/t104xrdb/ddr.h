/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2013 Freescale Semiconductor, Inc.
 */

#ifndef __DDR_H__
#define __DDR_H__
struct board_specific_parameters {
	u32 n_ranks;
	u32 datarate_mhz_high;
	u32 rank_gb;
	u32 clk_adjust;
	u32 wrlvl_start;
	u32 wrlvl_ctl_2;
	u32 wrlvl_ctl_3;
};

/*
 * These tables contain all valid speeds we want to override with board
 * specific parameters. datarate_mhz_high values need to be in ascending order
 * for each n_ranks group.
 */

static const struct board_specific_parameters udimm0[] = {
	/*
	 * memory controller 0
	 *   num|  hi| rank|  clk| wrlvl |   wrlvl
	 * ranks| mhz| GB  |adjst| start |   ctl2
	 */
#ifdef CONFIG_SYS_FSL_DDR4
	{2,  1600, 4, 8,     6, 0x07090A0c, 0x0e0f100a},
	{1,  1600, 4, 8,     5, 0x0607080B, 0x0C0C0D09},
#elif defined(CONFIG_SYS_FSL_DDR3)
	{2,  833,  4, 8,     6, 0x06060607, 0x08080807},
	{2,  833,  0, 8,     6, 0x06060607, 0x08080807},
	{2,  1350, 4, 8,     7, 0x0708080A, 0x0A0B0C09},
	{2,  1350, 0, 8,     7, 0x0708080A, 0x0A0B0C09},
	{2,  1666, 4, 8,     7, 0x0808090B, 0x0C0D0E0A},
	{2,  1666, 0, 8,     7, 0x0808090B, 0x0C0D0E0A},
	{1,  833,  4, 8,     6, 0x06060607, 0x08080807},
	{1,  833,  0, 8,     6, 0x06060607, 0x08080807},
	{1,  1350, 4, 8,     7, 0x0708080A, 0x0A0B0C09},
	{1,  1350, 0, 8,     7, 0x0708080A, 0x0A0B0C09},
	{1,  1666, 4, 8,     7, 0x0808090B, 0x0C0D0E0A},
	{1,  1666, 0, 8,     7, 0x0808090B, 0x0C0D0E0A},
#else
#error DDR type not defined
#endif
	{}
};

#endif

static const struct board_specific_parameters *udimms[] = {
	udimm0,
};
