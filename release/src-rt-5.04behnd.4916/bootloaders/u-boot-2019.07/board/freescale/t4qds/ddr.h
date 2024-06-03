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
	u32 cpo;
	u32 write_data_delay;
	u32 force_2t;
};

/*
 * These tables contain all valid speeds we want to override with board
 * specific parameters. datarate_mhz_high values need to be in ascending order
 * for each n_ranks group.
 */

static const struct board_specific_parameters udimm0[] = {
	/*
	 * memory controller 0
	 *   num|  hi| rank|  clk| wrlvl |   wrlvl   |  wrlvl | cpo  |wrdata|2T
	 * ranks| mhz| GB  |adjst| start |   ctl2    |  ctl3  |      |delay |
	 */
	{2,  1350, 4,  8,     8, 0x0809090b, 0x0c0c0d0a,   0xff,    2,  0},
	{2,  1350, 0, 10,     7, 0x0709090b, 0x0c0c0d09,   0xff,    2,  0},
	{2,  1666, 4,  8,     8, 0x080a0a0d, 0x0d10100b,   0xff,    2,  0},
	{2,  1666, 0, 10,     7, 0x080a0a0c, 0x0d0d0e0a,   0xff,    2,  0},
	{2,  1900, 0,  8,     8, 0x090a0b0e, 0x0f11120c,   0xff,    2,  0},
	{2,  2140, 0,  8,     8, 0x090a0b0e, 0x0f11120c,   0xff,    2,  0},
	{1,  1350, 0, 10,     8, 0x0809090b, 0x0c0c0d0a,   0xff,    2,  0},
	{1,  1700, 0, 10,     8, 0x080a0a0c, 0x0c0d0e0a,   0xff,    2,  0},
	{1,  1900, 0,  8,     8, 0x080a0a0c, 0x0e0e0f0a,   0xff,    2,  0},
	{1,  2140, 0,  8,     8, 0x090a0b0c, 0x0e0f100b,   0xff,    2,  0},
	{}
};

static const struct board_specific_parameters rdimm0[] = {
	/*
	 * memory controller 0
	 *   num|  hi| rank|  clk| wrlvl |   wrlvl   |  wrlvl | cpo  |wrdata|2T
	 * ranks| mhz| GB  |adjst| start |   ctl2    |  ctl3  |      |delay |
	 */
	{4,  1350, 0, 10,     9, 0x08070605, 0x06070806,   0xff,    2,  0},
	{4,  1666, 0, 10,    11, 0x0a080706, 0x07090906,   0xff,    2,  0},
	{4,  2140, 0, 10,    12, 0x0b090807, 0x080a0b07,   0xff,    2,  0},
	{2,  1350, 0, 10,     9, 0x08070605, 0x06070806,   0xff,    2,  0},
	{2,  1666, 0, 10,    11, 0x0a090806, 0x08090a06,   0xff,    2,  0},
	{2,  2140, 0, 10,    12, 0x0b090807, 0x080a0b07,   0xff,    2,  0},
	{1,  1350, 0, 10,     9, 0x08070605, 0x06070806,   0xff,    2,  0},
	{1,  1666, 0, 10,    11, 0x0a090806, 0x08090a06,   0xff,    2,  0},
	{1,  2140, 0,  8,    12, 0x0b090807, 0x080a0b07,   0xff,    2,  0},
	{}
};

/*
 * The three slots have slightly different timing. The center values are good
 * for all slots. We use identical speed tables for them. In future use, if
 * DIMMs require separated tables, make more entries as needed.
 */
static const struct board_specific_parameters *udimms[] = {
	udimm0,
};

/*
 * The three slots have slightly different timing. See comments above.
 */
static const struct board_specific_parameters *rdimms[] = {
	rdimm0,
};


#endif
