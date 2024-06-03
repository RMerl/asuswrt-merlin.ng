// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2010 Freescale Semiconductor, Inc.
 * Authors: Srikanth Srinivasan <srikanth.srinivasan@freescale.com>
 *          Timur Tabi <timur@freescale.com>
 */

#include <common.h>
#include <i2c.h>

#include <fsl_ddr_sdram.h>
#include <fsl_ddr_dimm_params.h>

void fsl_ddr_board_options(memctl_options_t *popts, dimm_params_t *pdimm,
			   unsigned int ctrl_num)
{
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

	popts->clk_adjust = 5;
	popts->cpo_override = 0x1f;
	popts->write_data_delay = 2;
	popts->half_strength_driver_enable = 1;

	/* Per AN4039, enable ZQ calibration. */
	popts->zq_en = 1;
}

#ifdef CONFIG_SPD_EEPROM
/*
 * we only have a "fake" SPD-EEPROM here, which has 16 bit addresses
 */
void get_spd(generic_spd_eeprom_t *spd, u8 i2c_address)
{
	int ret = i2c_read(i2c_address, 0, 2, (uchar *)spd,
				sizeof(generic_spd_eeprom_t));

	if (ret) {
		if (i2c_address ==
#ifdef SPD_EEPROM_ADDRESS
				SPD_EEPROM_ADDRESS
#elif defined(SPD_EEPROM_ADDRESS1)
				SPD_EEPROM_ADDRESS1
#endif
				) {
			printf("DDR: failed to read SPD from address %u\n",
			       i2c_address);
		} else {
			debug("DDR: failed to read SPD from address %u\n",
			      i2c_address);
		}
		memset(spd, 0, sizeof(generic_spd_eeprom_t));
	}
}
#endif
