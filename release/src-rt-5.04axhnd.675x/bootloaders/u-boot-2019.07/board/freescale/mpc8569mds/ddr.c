// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2009 Freescale Semiconductor, Inc.
 */

#include <common.h>

#include <fsl_ddr_sdram.h>
#include <fsl_ddr_dimm_params.h>

void fsl_ddr_board_options(memctl_options_t *popts,
				dimm_params_t *pdimm,
				unsigned int ctrl_num)
{
	/*
	 * Factors to consider for clock adjust:
	 *	- number of chips on bus
	 *	- position of slot
	 *	- DDR1 vs. DDR2?
	 *	- ???
	 *
	 * This needs to be determined on a board-by-board basis.
	 *	0110	3/4 cycle late
	 *	0111	7/8 cycle late
	 */
	popts->clk_adjust = 4;

	/*
	 * Factors to consider for CPO:
	 *	- frequency
	 *	- ddr1 vs. ddr2
	 */
	popts->cpo_override = 0xff;

	/*
	 * Factors to consider for write data delay:
	 *	- number of DIMMs
	 *
	 * 1 = 1/4 clock delay
	 * 2 = 1/2 clock delay
	 * 3 = 3/4 clock delay
	 * 4 = 1   clock delay
	 * 5 = 5/4 clock delay
	 * 6 = 3/2 clock delay
	 */
	popts->write_data_delay = 2;

	/*
	 * Enable half drive strength
	 */
	popts->half_strength_driver_enable = 1;

	/* Write leveling override */
	popts->wrlvl_en = 1;
	popts->wrlvl_override = 1;
	popts->wrlvl_sample = 0xa;
	popts->wrlvl_start = 0x4;

	/* Rtt and Rtt_W override */
	popts->rtt_override = 1;
	popts->rtt_override_value = DDR3_RTT_60_OHM;
	popts->rtt_wr_override_value = 0; /* Rtt_WR= dynamic ODT off */
}
