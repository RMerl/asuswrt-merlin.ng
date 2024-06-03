// SPDX-License-Identifier: GPL-2.0+
/*
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
