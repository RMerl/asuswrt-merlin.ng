// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2013 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <i2c.h>
#include <asm/fsl_law.h>
#include <fsl_ddr_sdram.h>
#include <fsl_ddr_dimm_params.h>

#include "cpld.h"

#define C29XPCIE_HARDWARE_REVA	0x40
/*
 * Micron MT41J128M16HA-15E
 * */
dimm_params_t ddr_raw_timing = {
	.n_ranks = 1,
	.rank_density = 536870912u,
	.capacity = 536870912u,
	.primary_sdram_width = 32,
	.ec_sdram_width = 8,
	.registered_dimm = 0,
	.mirrored_dimm = 0,
	.n_row_addr = 14,
	.n_col_addr = 10,
	.n_banks_per_sdram_device = 8,
	.edc_config = 2,
	.burst_lengths_bitmask = 0x0c,

	.tckmin_x_ps = 1650,
	.caslat_x = 0x7e << 4,	/* 5,6,7,8,9,10 */
	.taa_ps = 14050,
	.twr_ps = 15000,
	.trcd_ps = 13500,
	.trrd_ps = 75000,
	.trp_ps = 13500,
	.tras_ps = 40000,
	.trc_ps = 49500,
	.trfc_ps = 160000,
	.twtr_ps = 75000,
	.trtp_ps = 75000,
	.refresh_rate_ps = 7800000,
	.tfaw_ps = 30000,
};

int fsl_ddr_get_dimm_params(dimm_params_t *pdimm,
		unsigned int controller_number,
		unsigned int dimm_number)
{
	const char dimm_model[] = "Fixed DDR on board";

	if ((controller_number == 0) && (dimm_number == 0)) {
		memcpy(pdimm, &ddr_raw_timing, sizeof(dimm_params_t));
		memset(pdimm->mpart, 0, sizeof(pdimm->mpart));
		memcpy(pdimm->mpart, dimm_model, sizeof(dimm_model) - 1);
	}

	return 0;
}

void fsl_ddr_board_options(memctl_options_t *popts,
				dimm_params_t *pdimm,
				unsigned int ctrl_num)
{
	struct cpld_data *cpld_data = (void *)(CONFIG_SYS_CPLD_BASE);
	int i;

	popts->clk_adjust = 4;
	popts->cpo_override = 0x1f;
	popts->write_data_delay = 4;
	popts->half_strength_driver_enable = 1;
	popts->bstopre = 0x3cf;
	popts->quad_rank_present = 1;
	popts->rtt_override = 1;
	popts->rtt_override_value = 1;
	popts->dynamic_power = 1;
	/* Write leveling override */
	popts->wrlvl_en = 1;
	popts->wrlvl_override = 1;
	popts->wrlvl_sample = 0xf;
	popts->wrlvl_start = 0x4;
	popts->trwt_override = 1;
	popts->trwt = 0;

	if (in_8(&cpld_data->hwver) == C29XPCIE_HARDWARE_REVA)
		popts->ecc_mode = 0;

	for (i = 0; i < CONFIG_CHIP_SELECTS_PER_CTRL; i++) {
		popts->cs_local_opts[i].odt_rd_cfg = FSL_DDR_ODT_NEVER;
		popts->cs_local_opts[i].odt_wr_cfg = FSL_DDR_ODT_CS;
	}
}

void get_spd(generic_spd_eeprom_t *spd, u8 i2c_address)
{
	int ret = i2c_read(i2c_address, 0, 2, (uint8_t *)spd,
				sizeof(generic_spd_eeprom_t));

	if (ret) {
		printf("DDR: failed to read SPD from address %u\n",
				i2c_address);
		memset(spd, 0, sizeof(generic_spd_eeprom_t));
	}
}
