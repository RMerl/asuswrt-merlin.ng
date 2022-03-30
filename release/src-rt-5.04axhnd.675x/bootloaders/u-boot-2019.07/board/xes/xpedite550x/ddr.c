// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2010 Extreme Engineering Solutions, Inc.
 * Copyright 2007-2008 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <i2c.h>

#include <fsl_ddr_sdram.h>
#include <fsl_ddr_dimm_params.h>

void get_spd(ddr3_spd_eeprom_t *spd, u8 i2c_address)
{
	i2c_read(i2c_address, SPD_EEPROM_OFFSET, 2, (uchar *)spd,
		 sizeof(ddr3_spd_eeprom_t));
}

/*
 *     There are traditionally three board-specific SDRAM timing parameters
 *     which must be calculated based on the particular PCB artwork.  These are:
 *     1.) CPO (Read Capture Delay)
 *	       - TIMING_CFG_2 register
 *	       Source: Calculation based on board trace lengths and
 *		       chip-specific internal delays.
 *     2.) CLK_ADJUST (Clock and Addr/Cmd alignment control)
 *	       - DDR_SDRAM_CLK_CNTL register
 *	       Source: Signal Integrity Simulations
 *     3.) 2T Timing on Addr/Ctl
 *	       - TIMING_CFG_2 register
 *	       Source: Signal Integrity Simulations
 *	       Usually only needed with heavy load/very high speed (>DDR2-800)
 *
 *     ====== XPedite550x DDR3-800 read delay calculations ======
 *
 *     The P2020 processor provides an autoleveling option. Setting CPO to
 *     0x1f enables this auto configuration.
 */

typedef struct {
	unsigned short datarate_mhz_low;
	unsigned short datarate_mhz_high;
	unsigned char clk_adjust;
	unsigned char cpo;
} board_specific_parameters_t;

const board_specific_parameters_t board_specific_parameters[][20] = {
	{
		/* Controller 0 */
		{
			/* DDR3-600/667 */
			.datarate_mhz_low	= 500,
			.datarate_mhz_high	= 750,
			.clk_adjust		= 5,
			.cpo			= 31,
		},
		{
			/* DDR3-800 */
			.datarate_mhz_low	= 750,
			.datarate_mhz_high	= 850,
			.clk_adjust		= 5,
			.cpo			= 31,
		},
	},
};

void fsl_ddr_board_options(memctl_options_t *popts,
				dimm_params_t *pdimm,
				unsigned int ctrl_num)
{
	const board_specific_parameters_t *pbsp =
				&(board_specific_parameters[ctrl_num][0]);
	u32 num_params = sizeof(board_specific_parameters[ctrl_num]) /
				sizeof(board_specific_parameters[0][0]);
	u32 i;
	ulong ddr_freq;

	/*
	 * Set odt_rd_cfg and odt_wr_cfg. If the there is only one dimm in
	 * that controller, set odt_wr_cfg to 4 for CS0, and 0 to CS1. If
	 * there are two dimms in the controller, set odt_rd_cfg to 3 and
	 * odt_wr_cfg to 3 for the even CS, 0 for the odd CS.
	 */
	for (i = 0; i < CONFIG_CHIP_SELECTS_PER_CTRL; i++) {
		if (i&1) {	/* odd CS */
			popts->cs_local_opts[i].odt_rd_cfg = 0;
			popts->cs_local_opts[i].odt_wr_cfg = 0;
		} else {	/* even CS */
			if (CONFIG_DIMM_SLOTS_PER_CTLR == 1) {
				popts->cs_local_opts[i].odt_rd_cfg = 0;
				popts->cs_local_opts[i].odt_wr_cfg = 4;
			} else if (CONFIG_DIMM_SLOTS_PER_CTLR == 2) {
				popts->cs_local_opts[i].odt_rd_cfg = 3;
				popts->cs_local_opts[i].odt_wr_cfg = 3;
			}
		}
	}

	/*
	 * Get clk_adjust, cpo, write_data_delay,2T, according to the board ddr
	 * freqency and n_banks specified in board_specific_parameters table.
	 */
	ddr_freq = get_ddr_freq(0) / 1000000;

	for (i = 0; i < num_params; i++) {
		if (ddr_freq >= pbsp->datarate_mhz_low &&
		    ddr_freq <= pbsp->datarate_mhz_high) {
			popts->clk_adjust = pbsp->clk_adjust;
			popts->cpo_override = pbsp->cpo;
			popts->twot_en = 0;
			break;
		}
		pbsp++;
	}

	if (i == num_params) {
		printf("Warning: board specific timing not found "
		"for data rate %lu MT/s!\n", ddr_freq);
	}

	/*
	 * Factors to consider for half-strength driver enable:
	 *	- number of DIMMs installed
	 */
	popts->half_strength_driver_enable = 0;

	/*
	 * Enable on-die termination.
	 * From the Micron Technical Node TN-41-04, RTT_Nom should typically
	 * be 30 to 40 ohms, while RTT_WR should be 120 ohms.  Setting RTT_WR
	 * is handled in the Freescale DDR3 driver.  Set RTT_Nom here.
	 */
	popts->rtt_override = 1;
	popts->rtt_override_value = 3;
}
