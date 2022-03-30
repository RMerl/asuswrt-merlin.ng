/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2015 Freescale Semiconductor, Inc.
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
	{2,  1350, 0, 8,     6, 0x0708090B, 0x0C0D0E09,},
	{2,  1666, 0, 10,    9, 0x090A0B0E, 0x0F11110C,},
	{2,  1900, 0, 12,  0xA, 0x0B0C0E11, 0x1214140F,},
	{2,  2300, 0, 12,  0xB, 0x0C0D0F12, 0x14161610,},
	{}
};

/* DP-DDR DIMM */
static const struct board_specific_parameters udimm2[] = {
	/*
	 * memory controller 2
	 *   num|  hi| rank|  clk| wrlvl |   wrlvl   |  wrlvl
	 * ranks| mhz| GB  |adjst| start |   ctl2    |  ctl3
	 */
	{2,  1350, 0, 8,   0xd, 0x0C0A0A00, 0x00000009,},
	{2,  1666, 0, 8,   0xd, 0x0C0A0A00, 0x00000009,},
	{2,  1900, 0, 8,   0xe, 0x0D0C0B00, 0x0000000A,},
	{2,  2200, 0, 8,   0xe, 0x0D0C0B00, 0x0000000A,},
	{}
};

static const struct board_specific_parameters rdimm0[] = {
	/*
	 * memory controller 0
	 *   num|  hi| rank|  clk| wrlvl |   wrlvl   |  wrlvl
	 * ranks| mhz| GB  |adjst| start |   ctl2    |  ctl3
	 */
	{2,  1666, 0, 8,     0x0F, 0x0D0C0A09, 0x0B0C0E08,},
	{2,  1900, 0, 8,     0x10, 0x0F0D0B0A, 0x0B0E0F09,},
	{2,  2200, 0, 8,     0x13, 0x120F0E0B, 0x0D10110B,},
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
	udimm2,	/* DP-DDR doesn't support RDIMM */
};


#endif
