// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2008 Freescale Semiconductor, Inc.
 */

#include <common.h>

#include <fsl_ddr_sdram.h>
#include <fsl_ddr_dimm_params.h>

struct board_specific_parameters {
	u32 n_ranks;
	u32 datarate_mhz_high;
	u32 clk_adjust;
	u32 cpo;
	u32 write_data_delay;
	u32 force_2t;
};

/*
 * This table contains all valid speeds we want to override with board
 * specific parameters. datarate_mhz_high values need to be in ascending order
 * for each n_ranks group.
 *
 * For DDR2 DIMM, all combinations of clk_adjust and write_data_delay have been
 * tested. For RDIMM, clk_adjust = 4 and write_data_delay = 3 is optimized for
 * all clocks from 400MT/s to 800MT/s, verified with Kingston KVR800D2D8P6/2G.
 * For UDIMM, clk_adjust = 8 and write_delay = 5 is optimized for all clocks
 * from 400MT/s to 800MT/s, verified with Micron MT18HTF25672AY-800E1.
 *
 * CPO value doesn't matter if workaround for errata 111 and 134 enabled.
 */
static const struct board_specific_parameters udimm0[] = {
	/*
	 * memory controller 0
	 *   num|  hi|  clk| cpo|wrdata|2T
	 * ranks| mhz|adjst|    | delay|
	 */
	{2,  333,    8,   7,    5,  0},
	{2,  400,    8,   9,    5,  0},
	{2,  549,    8,  11,    5,  0},
	{2,  680,    8,  10,    5,  0},
	{2,  850,    8,  12,    5,  1},
	{1,  333,    6,   7,    3,  0},
	{1,  400,    6,   9,    3,  0},
	{1,  549,    6,  11,    3,  0},
	{1,  680,    1,  10,    5,  0},
	{1,  850,    1,  12,    5,  0},
	{}
};

static const struct board_specific_parameters udimm1[] = {
	/*
	 * memory controller 1
	 *   num|  hi|  clk| cpo|wrdata|2T
	 * ranks| mhz|adjst|    | delay|
	 */
	{2,  333,    8,  7,    5,  0},
	{2,  400,    8,  9,    5,  0},
	{2,  549,    8, 11,    5,  0},
	{2,  680,    8, 11,    5,  0},
	{2,  850,    8, 13,    5,  1},
	{1,  333,    6,  7,    3,  0},
	{1,  400,    6,  9,    3,  0},
	{1,  549,    6, 11,    3,  0},
	{1,  680,    1, 11,    6,  0},
	{1,  850,    1, 13,    6,  0},
	{}
};

static const struct board_specific_parameters *udimms[] = {
	udimm0,
	udimm1,
};

static const struct board_specific_parameters rdimm0[] = {
	/*
	 * memory controller 0
	 *   num|  hi|  clk| cpo|wrdata|2T
	 * ranks| mhz|adjst|    | delay|
	 */
	{2,  333,    4,   7,    3,  0},
	{2,  400,    4,   9,    3,  0},
	{2,  549,    4,  11,    3,  0},
	{2,  680,    4,  10,    3,  0},
	{2,  850,    4,  12,    3,  1},
	{}
};

static const struct board_specific_parameters rdimm1[] = {
	/*
	 * memory controller 1
	 *   num|  hi|  clk| cpo|wrdata|2T
	 * ranks| mhz|adjst|    | delay|
	 */
	{2,  333,     4,  7,    3,  0},
	{2,  400,     4,  9,    3,  0},
	{2,  549,     4, 11,    3,  0},
	{2,  680,     4, 11,    3,  0},
	{2,  850,     4, 13,    3,  1},
	{}
};

static const struct board_specific_parameters *rdimms[] = {
	rdimm0,
	rdimm1,
};

void fsl_ddr_board_options(memctl_options_t *popts,
				dimm_params_t *pdimm,
				unsigned int ctrl_num)
{
	const struct board_specific_parameters *pbsp, *pbsp_highest = NULL;
	ulong ddr_freq;

	if (ctrl_num > 1) {
		printf("Wrong parameter for controller number %d", ctrl_num);
		return;
	}
	if (!pdimm->n_ranks)
		return;

	if (popts->registered_dimm_en)
		pbsp = rdimms[ctrl_num];
	else
		pbsp = udimms[ctrl_num];

	/* Get clk_adjust, cpo, write_data_delay,2T, according to the board ddr
	 * freqency and n_banks specified in board_specific_parameters table.
	 */
	ddr_freq = get_ddr_freq(0) / 1000000;
	while (pbsp->datarate_mhz_high) {
		if (pbsp->n_ranks == pdimm->n_ranks) {
			if (ddr_freq <= pbsp->datarate_mhz_high) {
				popts->clk_adjust = pbsp->clk_adjust;
				popts->cpo_override = pbsp->cpo;
				popts->write_data_delay =
					pbsp->write_data_delay;
				popts->twot_en = pbsp->force_2t;
				goto found;
			}
			pbsp_highest = pbsp;
		}
		pbsp++;
	}

	if (pbsp_highest) {
		printf("Error: board specific timing not found "
			"for data rate %lu MT/s!\n"
			"Trying to use the highest speed (%u) parameters\n",
			ddr_freq, pbsp_highest->datarate_mhz_high);
		popts->clk_adjust = pbsp->clk_adjust;
		popts->cpo_override = pbsp->cpo;
		popts->write_data_delay = pbsp->write_data_delay;
		popts->twot_en = pbsp->force_2t;
	} else {
		panic("DIMM is not supported by this board");
	}

found:
	/*
	 * Factors to consider for half-strength driver enable:
	 *	- number of DIMMs installed
	 */
	popts->half_strength_driver_enable = 0;
}
