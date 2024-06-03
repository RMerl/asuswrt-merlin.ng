// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2010 Freescale Semiconductor, Inc.
 * Authors: Srikanth Srinivasan <srikanth.srinivasan@freescale.com>
 *          Timur Tabi <timur@freescale.com>
 */

#include <common.h>

#include <fsl_ddr_sdram.h>
#include <fsl_ddr_dimm_params.h>

struct board_specific_parameters {
	u32 n_ranks;
	u32 datarate_mhz_high;
	u32 clk_adjust;		/* Range: 0-8 */
	u32 cpo;		/* Range: 2-31 */
	u32 write_data_delay;	/* Range: 0-6 */
	u32 force_2t;
};

/*
 * This table contains all valid speeds we want to override with board
 * specific parameters. datarate_mhz_high values need to be in ascending order
 * for each n_ranks group.
 */
static const struct board_specific_parameters dimm0[] = {
	/*
	 * memory controller 0
	 *   num|  hi|  clk| cpo|wrdata|2T
	 * ranks| mhz|adjst|    | delay|
	 */
	{1,  549,    5,  31,     3, 0},
	{1,  850,    5,  31,     5, 0},
	{2,  549,    5,  31,     3, 0},
	{2,  850,    5,  31,     5, 0},
	{}
};

void fsl_ddr_board_options(memctl_options_t *popts, dimm_params_t *pdimm,
			   unsigned int ctrl_num)
{
	const struct board_specific_parameters *pbsp, *pbsp_highest = NULL;
	unsigned long ddr_freq;
	unsigned int i;


	if (ctrl_num) {
		printf("Wrong parameter for controller number %d", ctrl_num);
		return;
	}
	if (!pdimm->n_ranks)
		return;

	/* set odt_rd_cfg and odt_wr_cfg. */
	for (i = 0; i < CONFIG_CHIP_SELECTS_PER_CTRL; i++) {
		popts->cs_local_opts[i].odt_rd_cfg = 0;
		popts->cs_local_opts[i].odt_wr_cfg = 1;
	}

	pbsp = dimm0;
	/*
	 * Get clk_adjust, cpo, write_data_delay,2T, according to the board ddr
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
	popts->half_strength_driver_enable = 1;

	/* Per AN4039, enable ZQ calibration. */
	popts->zq_en = 1;

	/*
	 * For wake-up on ARP, we need auto self refresh enabled
	 */
	popts->auto_self_refresh_en = 1;
	popts->sr_it = 0xb;
}
