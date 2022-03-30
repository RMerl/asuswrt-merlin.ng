// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2011 Freescale Semiconductor, Inc.
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
 */
static const struct board_specific_parameters udimm0[] = {
	/*
	 * memory controller 0
	 *   num|  hi|  clk| cpo|wrdata|2T
	 * ranks| mhz|adjst|    | delay|
	 */
	{2,  300,    4,   4,    2,  0},
	{2,  365,    4,   6,    2,  0},
	{2,  450,    4,   7,    2,  0},
	{2,  850,    4,  31,    2,  0},
	{1,  300,    4,   4,    2,  0},
	{1,  365,    4,   6,    2,  0},
	{1,  450,    4,   7,    2,  0},
	{1,  850,    4,  31,    2,  0},
	{}
};

void fsl_ddr_board_options(memctl_options_t *popts,
				dimm_params_t *pdimm,
				unsigned int ctrl_num)
{
	const struct board_specific_parameters *pbsp, *pbsp_highest = NULL;
	unsigned int i;
	ulong ddr_freq;

	if (ctrl_num != 0)	/* we have only one controller */
		return;
	for (i = 0; i < CONFIG_DIMM_SLOTS_PER_CTLR; i++) {
		if (pdimm[i].n_ranks)
			break;
	}
	if (i >= CONFIG_DIMM_SLOTS_PER_CTLR)	/* no DIMM */
		return;

	pbsp = udimm0;

	/* Get clk_adjust, cpo, write_data_delay,2T, according to the board ddr
	 * freqency and n_banks specified in board_specific_parameters table.
	 */
	ddr_freq = get_ddr_freq(0) / 1000000;
	while (pbsp->datarate_mhz_high) {
		if (pbsp->n_ranks ==  pdimm[i].n_ranks) {
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
		popts->clk_adjust = pbsp_highest->clk_adjust;
		popts->cpo_override = pbsp_highest->cpo;
		popts->write_data_delay = pbsp_highest->write_data_delay;
		popts->twot_en = pbsp_highest->force_2t;
	} else {
		panic("DIMM is not supported by this board");
	}

found:
	/*
	 * Factors to consider for half-strength driver enable:
	 *	- number of DIMMs installed
	 */
	popts->half_strength_driver_enable = 0;
	popts->dqs_config = 0;	/* only true DQS signal is used on board */
}
