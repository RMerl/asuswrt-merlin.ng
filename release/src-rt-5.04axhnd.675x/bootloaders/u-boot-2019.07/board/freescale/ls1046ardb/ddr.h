/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2016 Freescale Semiconductor, Inc.
 */

#ifndef __DDR_H__
#define __DDR_H__

void erratum_a008850_post(void);

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
	{2,  1666, 0, 8,     7, 0x08090A0C, 0x0D0F100B,},
	{2,  1900, 0, 8,     7, 0x09090B0D, 0x0E10120B,},
	{2,  2300, 0, 8,     9, 0x0A0B0C10, 0x1213140E,},
	{}
};

static const struct board_specific_parameters *udimms[] = {
	udimm0,
};

static const struct board_specific_parameters rdimm0[] = {
	/*
	 * memory controller 0
	 *   num|  hi| rank|  clk| wrlvl |   wrlvl   |  wrlvl
	 * ranks| mhz| GB  |adjst| start |   ctl2    |  ctl3
	 */
	{2,  1666, 0, 0x8,     0x0D, 0x0C0B0A08, 0x0A0B0C08,},
	{2,  1900, 0, 0x8,     0x0E, 0x0D0C0B09, 0x0B0C0D09,},
	{2,  2300, 0, 0xa,     0x12, 0x100F0D0C, 0x0E0F100C,},
	{1,  1666, 0, 0x8,     0x0D, 0x0C0B0A08, 0x0A0B0C08,},
	{1,  1900, 0, 0x8,     0x0E, 0x0D0C0B09, 0x0B0C0D09,},
	{1,  2300, 0, 0xa,     0x12, 0x100F0D0C, 0x0E0F100C,},
	{}
};

static const struct board_specific_parameters *rdimms[] = {
	rdimm0,
};

#endif
