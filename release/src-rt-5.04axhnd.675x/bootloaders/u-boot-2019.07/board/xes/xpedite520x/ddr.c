// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2008 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <i2c.h>

#include <fsl_ddr_sdram.h>
#include <fsl_ddr_dimm_params.h>

void get_spd(ddr2_spd_eeprom_t *spd, unsigned char i2c_address)
{
	i2c_read(i2c_address, 0, 1, (uchar *)spd, sizeof(ddr2_spd_eeprom_t));

	/* We use soldered memory, but use an SPD EEPROM to describe it.
	 * The SPD has an unspecified dimm type, but the DDR2 initialization
	 * code requires a specific type to be specified. This sets the type
	 * as a standard unregistered SO-DIMM. */
	if (spd->dimm_type == 0) {
		spd->dimm_type = 0x4;
		((uchar *)spd)[63] += 0x4;
	}
}

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
	popts->clk_adjust = 7;

	/*
	 * Factors to consider for CPO:
	 *	- frequency
	 *	- ddr1 vs. ddr2
	 */
	popts->cpo_override = 9;

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
	popts->write_data_delay = 3;

	/*
	 * Factors to consider for half-strength driver enable:
	 *	- number of DIMMs installed
	 */
	popts->half_strength_driver_enable = 0;
}
