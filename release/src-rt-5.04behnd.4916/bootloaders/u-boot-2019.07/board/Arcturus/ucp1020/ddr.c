// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2013-2015 Arcturus Networks, Inc.
 *           http://www.arcturusnetworks.com/products/ucp1020/
 * based on board/freescale/p1_p2_rdb_pc/spl.c
 * original copyright follows:
 * Copyright 2013 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <asm/mmu.h>
#include <asm/immap_85xx.h>
#include <asm/processor.h>
#include <fsl_ddr_sdram.h>
#include <fsl_ddr_dimm_params.h>
#include <asm/io.h>
#include <asm/fsl_law.h>

#ifdef CONFIG_SYS_DDR_RAW_TIMING
#if defined(CONFIG_UCP1020) || defined(CONFIG_UCP1020T1)
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

#else
#error Missing raw timing data for this board
#endif

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
#endif /* CONFIG_SYS_DDR_RAW_TIMING */

#ifdef CONFIG_SYS_DDR_CS0_BNDS
/* Fixed sdram init -- doesn't use serial presence detect. */
phys_size_t fixed_sdram(void)
{
	sys_info_t sysinfo;
	char buf[32];
	size_t ddr_size;
	fsl_ddr_cfg_regs_t ddr_cfg_regs = {
		.cs[0].bnds = CONFIG_SYS_DDR_CS0_BNDS,
		.cs[0].config = CONFIG_SYS_DDR_CS0_CONFIG,
		.cs[0].config_2 = CONFIG_SYS_DDR_CS0_CONFIG_2,
#if CONFIG_CHIP_SELECTS_PER_CTRL > 1
		.cs[1].bnds = CONFIG_SYS_DDR_CS1_BNDS,
		.cs[1].config = CONFIG_SYS_DDR_CS1_CONFIG,
		.cs[1].config_2 = CONFIG_SYS_DDR_CS1_CONFIG_2,
#endif
		.timing_cfg_3 = CONFIG_SYS_DDR_TIMING_3,
		.timing_cfg_0 = CONFIG_SYS_DDR_TIMING_0,
		.timing_cfg_1 = CONFIG_SYS_DDR_TIMING_1,
		.timing_cfg_2 = CONFIG_SYS_DDR_TIMING_2,
		.ddr_sdram_cfg = CONFIG_SYS_DDR_CONTROL,
		.ddr_sdram_cfg_2 = CONFIG_SYS_DDR_CONTROL_2,
		.ddr_sdram_mode = CONFIG_SYS_DDR_MODE_1,
		.ddr_sdram_mode_2 = CONFIG_SYS_DDR_MODE_2,
		.ddr_sdram_md_cntl = CONFIG_SYS_DDR_MODE_CONTROL,
		.ddr_sdram_interval = CONFIG_SYS_DDR_INTERVAL,
		.ddr_data_init = CONFIG_SYS_DDR_DATA_INIT,
		.ddr_sdram_clk_cntl = CONFIG_SYS_DDR_CLK_CTRL,
		.ddr_init_addr = CONFIG_SYS_DDR_INIT_ADDR,
		.ddr_init_ext_addr = CONFIG_SYS_DDR_INIT_EXT_ADDR,
		.timing_cfg_4 = CONFIG_SYS_DDR_TIMING_4,
		.timing_cfg_5 = CONFIG_SYS_DDR_TIMING_5,
		.ddr_zq_cntl = CONFIG_SYS_DDR_ZQ_CONTROL,
		.ddr_wrlvl_cntl = CONFIG_SYS_DDR_WRLVL_CONTROL,
		.ddr_sr_cntr = CONFIG_SYS_DDR_SR_CNTR,
		.ddr_sdram_rcw_1 = CONFIG_SYS_DDR_RCW_1,
		.ddr_sdram_rcw_2 = CONFIG_SYS_DDR_RCW_2
	};

	get_sys_info(&sysinfo);
	printf("Configuring DDR for %s MT/s data rate\n",
	       strmhz(buf, sysinfo.freq_ddrbus));

	ddr_size = CONFIG_SYS_SDRAM_SIZE * 1024 * 1024;

	fsl_ddr_set_memctl_regs(&ddr_cfg_regs, 0, 0);

	if (set_ddr_laws(CONFIG_SYS_DDR_SDRAM_BASE,
			 ddr_size, LAW_TRGT_IF_DDR_1) < 0) {
		printf("ERROR setting Local Access Windows for DDR\n");
		return 0;
	};

	return ddr_size;
}
#endif

void fsl_ddr_board_options(memctl_options_t *popts,
			   dimm_params_t *pdimm,
			   unsigned int ctrl_num)
{
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

	if (pdimm->primary_sdram_width == 64)
		popts->data_bus_width = 0;
	else if (pdimm->primary_sdram_width == 32)
		popts->data_bus_width = 1;
	else
		printf("Error in DDR bus width configuration!\n");

	for (i = 0; i < CONFIG_CHIP_SELECTS_PER_CTRL; i++) {
		popts->cs_local_opts[i].odt_rd_cfg = FSL_DDR_ODT_NEVER;
		popts->cs_local_opts[i].odt_wr_cfg = FSL_DDR_ODT_CS;
	}
}
