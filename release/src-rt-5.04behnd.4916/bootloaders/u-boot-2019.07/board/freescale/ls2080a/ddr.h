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
	 *   num|  hi| rank|  clk| wrlvl |   wrlvl   |  wrlvl
	 * ranks| mhz| GB  |adjst| start |   ctl2    |  ctl3
	 */
	{2,  2140, 0, 4,     4, 0x0, 0x0},
	{1,  2140, 0, 4,     4, 0x0, 0x0},
	{}
};

/* DP-DDR DIMM */
static const struct board_specific_parameters udimm2[] = {
	/*
	 * memory controller 2
	 *   num|  hi| rank|  clk| wrlvl |   wrlvl   |  wrlvl
	 * ranks| mhz| GB  |adjst| start |   ctl2    |  ctl3
	 */
	{2,  2140, 0, 4,     4, 0x0, 0x0},
	{1,  2140, 0, 4,     4, 0x0, 0x0},
	{}
};

static const struct board_specific_parameters rdimm0[] = {
	/*
	 * memory controller 0
	 *   num|  hi| rank|  clk| wrlvl |   wrlvl   |  wrlvl
	 * ranks| mhz| GB  |adjst| start |   ctl2    |  ctl3
	 */
	{4,  2140, 0, 5,     4, 0x0, 0x0},
	{2,  2140, 0, 5,     4, 0x0, 0x0},
	{1,  2140, 0, 4,     4, 0x0, 0x0},
	{}
};

/* DP-DDR DIMM */
static const struct board_specific_parameters rdimm2[] = {
	/*
	 * memory controller 2
	 *   num|  hi| rank|  clk| wrlvl |   wrlvl   |  wrlvl
	 * ranks| mhz| GB  |adjst| start |   ctl2    |  ctl3
	 */
	{4,  2140, 0, 5,     4, 0x0, 0x0},
	{2,  2140, 0, 5,     4, 0x0, 0x0},
	{1,  2140, 0, 4,     4, 0x0, 0x0},
	{}
};

static const struct board_specific_parameters *udimms[] = {
	udimm0,
	udimm0,
	udimm2,
};

static const struct board_specific_parameters *rdimms[] = {
	rdimm0,
	rdimm0,
	rdimm2,
};


#endif
