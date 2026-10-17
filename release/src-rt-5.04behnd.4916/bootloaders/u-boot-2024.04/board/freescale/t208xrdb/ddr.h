/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2014 Freescale Semiconductor, Inc.
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
	 *   num|  hi| rank|  clk| wrlvl |   wrlvl   |  wrlvl |
	 * ranks| mhz| GB  |adjst| start |   ctl2    |  ctl3  |
	 */
	{2,  1200, 2, 10,     7, 0x0808090a, 0x0b0c0c0a},
	{2,  1500, 2, 10,     6, 0x07070809, 0x0a0b0b09},
	{2,  1600, 2, 10,     8, 0x0808070b, 0x0c0d0e0a},
	{2,  1700, 2,  8,     7, 0x080a0a0c, 0x0c0d0e0a},
	{2,  1900, 0, 10,     7, 0x0808080c, 0x0b0c0c09},
	{1,  1200, 2, 10,     7, 0x0808090a, 0x0b0c0c0a},
	{1,  1500, 2, 10,     6, 0x07070809, 0x0a0b0b09},
	{1,  1600, 2, 10,     8, 0x0808070b, 0x0c0d0e0a},
	{1,  1700, 2,  8,     7, 0x080a0a0c, 0x0c0d0e0a},
	{1,  1900, 0, 10,     7, 0x0808080c, 0x0b0c0c09},
	{}
};

static const struct board_specific_parameters *udimms[] = {
	udimm0,
};
#endif
