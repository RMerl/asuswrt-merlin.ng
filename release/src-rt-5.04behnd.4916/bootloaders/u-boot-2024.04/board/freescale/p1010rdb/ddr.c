// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2010-2011 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <vsprintf.h>
#include <asm/global_data.h>
#include <asm/mmu.h>
#include <asm/immap_85xx.h>
#include <asm/processor.h>
#include <fsl_ddr_sdram.h>
#include <fsl_ddr_dimm_params.h>
#include <asm/io.h>
#include <asm/fsl_law.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Samsung K4B2G0846C-HCF8
 * The following timing are for "downshift"
 * i.e. to use CL9 part as CL7
 * otherwise, tAA, tRCD, tRP will be 13500ps
 * and tRC will be 49500ps
 */
dimm_params_t ddr_raw_timing = {
	.n_ranks = 1,
	.rank_density = 1073741824u,
	.capacity = 1073741824u,
	.primary_sdram_width = 32,
	.ec_sdram_width = 0,
	.registered_dimm = 0,
	.mirrored_dimm = 0,
	.n_row_addr = 15,
	.n_col_addr = 10,
	.n_banks_per_sdram_device = 8,
	.edc_config = 0,
	.burst_lengths_bitmask = 0x0c,

	.tckmin_x_ps = 1875,
	.caslat_x = 0x1e << 4,	/* 5,6,7,8 */
	.taa_ps = 13125,
	.twr_ps = 15000,
	.trcd_ps = 13125,
	.trrd_ps = 7500,
	.trp_ps = 13125,
	.tras_ps = 37500,
	.trc_ps = 50625,
	.trfc_ps = 160000,
	.twtr_ps = 7500,
	.trtp_ps = 7500,
	.refresh_rate_ps = 7800000,
	.tfaw_ps = 37500,
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
	struct cpu_type *cpu;
	int i;
	popts->clk_adjust = 6;
	popts->cpo_override = 0x1f;
	popts->write_data_delay = 2;
	popts->half_strength_driver_enable = 1;
	/* Write leveling override */
	popts->wrlvl_en = 1;
	popts->wrlvl_override = 1;
	popts->wrlvl_sample = 0xf;
	popts->wrlvl_start = 0x8;
	popts->trwt_override = 1;
	popts->trwt = 0;

	cpu = gd->arch.cpu;
	/* P1014 and it's derivatives support max 16it DDR width */
	if (cpu->soc_ver == SVR_P1014)
		popts->data_bus_width = DDR_DATA_BUS_WIDTH_16;

	for (i = 0; i < CONFIG_CHIP_SELECTS_PER_CTRL; i++) {
		popts->cs_local_opts[i].odt_rd_cfg = FSL_DDR_ODT_NEVER;
		popts->cs_local_opts[i].odt_wr_cfg = FSL_DDR_ODT_CS;
	}
}
