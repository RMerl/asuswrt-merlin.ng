// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2008,2011 Freescale Semiconductor, Inc.
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
};

/*
 * This table contains all valid speeds we want to override with board
 * specific parameters. datarate_mhz_high values need to be in ascending order
 * for each n_ranks group.
 */
const struct board_specific_parameters dimm0[] = {
	/*
	 * memory controller 0
	 *   num|  hi|  clk| cpo|wrdata|2T
	 * ranks| mhz|adjst|    | delay|
	 */
	{4,  333,    7,   7,     3},
	{4,  549,    7,   9,     3},
	{4,  650,    7,  10,     4},
	{2,  333,    7,   7,     3},
	{2,  549,    7,   9,     3},
	{2,  650,    7,  10,     4},
	{1,  333,    7,   7,     3},
	{1,  549,    7,   9,     3},
	{1,  650,    7,  10,     4},
	{}
};

/*
 * The two slots have slightly different timing. The center values are good
 * for both slots. We use identical speed tables for them. In future use, if
 * DIMMs have fewer center values that require two separated tables, copy the
 * udimm0 table to udimm1 and make changes to clk_adjust and wrlvl_start.
 */
const struct board_specific_parameters *dimms[] = {
	dimm0,
	dimm0,
};

void fsl_ddr_board_options(memctl_options_t *popts,
			dimm_params_t *pdimm,
			unsigned int ctrl_num)
{
	const struct board_specific_parameters *pbsp, *pbsp_highest = NULL;
	unsigned int i;
	ulong ddr_freq;

	if (ctrl_num > 1) {
		printf("Wrong parameter for controller number %d", ctrl_num);
		return;
	}
	for (i = 0; i < CONFIG_DIMM_SLOTS_PER_CTLR; i++) {
		if (pdimm[i].n_ranks)
			break;
	}
	if (i >= CONFIG_DIMM_SLOTS_PER_CTLR)    /* no DIMM */
		return;

	pbsp = dimms[ctrl_num];

	/* Get clk_adjust, cpo, write_data_delay, according to the board ddr
	 * freqency and n_banks specified in board_specific_parameters table.
	 */
	ddr_freq = get_ddr_freq(0) / 1000000;
	while (pbsp->datarate_mhz_high) {
		if (pbsp->n_ranks == pdimm[i].n_ranks) {
			if (ddr_freq <= pbsp->datarate_mhz_high) {
				popts->clk_adjust = pbsp->clk_adjust;
				popts->cpo_override = pbsp->cpo;
				popts->write_data_delay =
					pbsp->write_data_delay;
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
	} else {
		panic("DIMM is not supported by this board");
	}

found:
	/* 2T timing enable */
	popts->twot_en = 1;
}
