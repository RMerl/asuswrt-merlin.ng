/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2014 Freescale Semiconductor, Inc.
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
	u32 cpo_override;
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
#ifdef CONFIG_SYS_FSL_DDR4
	{2,  1666, 0, 8,     7, 0x0808090B, 0x0C0D0E0A,},
	{2,  1900, 0, 8,     6, 0x08080A0C, 0x0D0E0F0A,},
	{1,  1666, 0, 8,     8, 0x090A0B0B, 0x0C0D0E0C,},
	{1,  1900, 0, 8,     9, 0x0A0B0C0B, 0x0D0E0F0D,},
	{1,  2200, 0, 8,    10, 0x0B0C0D0C, 0x0E0F110E,},
#elif defined(CONFIG_SYS_FSL_DDR3)
	{1,  833,  1, 12,     8, 0x06060607, 0x08080807,   0x1f,    2,  0},
	{1,  1350, 1, 12,     8, 0x0708080A, 0x0A0B0C09,   0x1f,    2,  0},
	{1,  833,  2, 12,     8, 0x06060607, 0x08080807,   0x1f,    2,  0},
	{1,  1350, 2, 12,     8, 0x0708080A, 0x0A0B0C09,   0x1f,    2,  0},
	{2,  833,  4, 12,     8, 0x06060607, 0x08080807,   0x1f,    2,  0},
	{2,  1350, 4, 12,     8, 0x0708080A, 0x0A0B0C09,   0x1f,    2,  0},
	{2,  1350, 0, 12,     8, 0x0708080A, 0x0A0B0C09,   0x1f,    2,  0},
	{2,  1666, 4, 8,    0xa, 0x0B08090C, 0x0B0E0D0A,   0x1f,    2,  0},
	{2,  1666, 0, 8,    0xa, 0x0B08090C, 0x0B0E0D0A,   0x1f,    2,  0},
#else
#error DDR type not defined
#endif
	{}
};

static const struct board_specific_parameters *udimms[] = {
	udimm0,
};

#endif
