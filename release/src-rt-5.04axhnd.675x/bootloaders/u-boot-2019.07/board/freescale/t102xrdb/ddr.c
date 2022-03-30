// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <i2c.h>
#include <hwconfig.h>
#include <asm/mmu.h>
#include <fsl_ddr_sdram.h>
#include <fsl_ddr_dimm_params.h>
#include <asm/fsl_law.h>
#include <asm/mpc85xx_gpio.h>

DECLARE_GLOBAL_DATA_PTR;

struct board_specific_parameters {
	u32 n_ranks;
	u32 datarate_mhz_high;
	u32 rank_gb;
	u32 clk_adjust;
	u32 wrlvl_start;
	u32 wrlvl_ctl_2;
	u32 wrlvl_ctl_3;
};

/*
 * datarate_mhz_high values need to be in ascending order
 */
static const struct board_specific_parameters udimm0[] = {
	/*
	 * memory controller 0
	 *   num|  hi| rank|  clk| wrlvl |   wrlvl   |  wrlvl |
	 * ranks| mhz| GB  |adjst| start |   ctl2    |  ctl3  |
	 */
	{2,  833,   0,  8,  6,  0x06060607,  0x08080807,},
	{2,  1350,  0,  8,  7,  0x0708080A,  0x0A0B0C09,},
	{2,  1666,  0,  8,  7,  0x0808090B,  0x0C0D0E0A,},
	{1,  833,   0,  8,  6,  0x06060607,  0x08080807,},
	{1,  1350,  0,  8,  7,  0x0708080A,  0x0A0B0C09,},
	{1,  1666,  0,  8,  7,  0x0808090B,  0x0C0D0E0A,},
	{}
};

static const struct board_specific_parameters *udimms[] = {
	udimm0,
};

void fsl_ddr_board_options(memctl_options_t *popts,
			   dimm_params_t *pdimm,
			   unsigned int ctrl_num)
{
	const struct board_specific_parameters *pbsp, *pbsp_highest = NULL;
	ulong ddr_freq;
	struct cpu_type *cpu = gd->arch.cpu;

	if (ctrl_num > 1) {
		printf("Not supported controller number %d\n", ctrl_num);
		return;
	}
	if (!pdimm->n_ranks)
		return;

	pbsp = udimms[0];

	/* Get clk_adjust according to the board ddr freqency and n_banks
	 * specified in board_specific_parameters table.
	 */
	ddr_freq = get_ddr_freq(0) / 1000000;
	while (pbsp->datarate_mhz_high) {
		if (pbsp->n_ranks == pdimm->n_ranks &&
		    (pdimm->rank_density >> 30) >= pbsp->rank_gb) {
			if (ddr_freq <= pbsp->datarate_mhz_high) {
				popts->clk_adjust = pbsp->clk_adjust;
				popts->wrlvl_start = pbsp->wrlvl_start;
				popts->wrlvl_ctl_2 = pbsp->wrlvl_ctl_2;
				popts->wrlvl_ctl_3 = pbsp->wrlvl_ctl_3;
				goto found;
			}
			pbsp_highest = pbsp;
		}
		pbsp++;
	}

	if (pbsp_highest) {
		printf("Error: board specific timing not found\n");
		printf("for data rate %lu MT/s\n", ddr_freq);
		printf("Trying to use the highest speed (%u) parameters\n",
		       pbsp_highest->datarate_mhz_high);
		popts->clk_adjust = pbsp_highest->clk_adjust;
		popts->wrlvl_start = pbsp_highest->wrlvl_start;
		popts->wrlvl_ctl_2 = pbsp->wrlvl_ctl_2;
		popts->wrlvl_ctl_3 = pbsp->wrlvl_ctl_3;
	} else {
		panic("DIMM is not supported by this board");
	}
found:
	debug("Found timing match: n_ranks %d, data rate %d, rank_gb %d\n",
	      pbsp->n_ranks, pbsp->datarate_mhz_high, pbsp->rank_gb);
	debug("\tclk_adjust %d, wrlvl_start %d, wrlvl_ctrl_2 0x%x, ",
	      pbsp->clk_adjust, pbsp->wrlvl_start, pbsp->wrlvl_ctl_2);
	debug("wrlvl_ctrl_3 0x%x\n", pbsp->wrlvl_ctl_3);

	/*
	 * Factors to consider for half-strength driver enable:
	 *	- number of DIMMs installed
	 */
	popts->half_strength_driver_enable = 0;
	/*
	 * Write leveling override
	 */
	popts->wrlvl_override = 1;
	popts->wrlvl_sample = 0xf;

	/*
	 * rtt and rtt_wr override
	 */
	popts->rtt_override = 0;

	/* Enable ZQ calibration */
	popts->zq_en = 1;

	/* DHC_EN =1, ODT = 75 Ohm */
	popts->ddr_cdr1 = DDR_CDR1_DHC_EN | DDR_CDR1_ODT(DDR_CDR_ODT_OFF);
	popts->ddr_cdr2 = DDR_CDR2_ODT(DDR_CDR_ODT_OFF);

	/* T1023 supports max DDR bus 32bit width, T1024 supports DDR 64bit,
	 * force DDR bus width to 32bit for T1023
	 */
	if (cpu->soc_ver == SVR_T1023)
		popts->data_bus_width = DDR_DATA_BUS_WIDTH_32;

#ifdef CONFIG_FORCE_DDR_DATA_BUS_WIDTH_32
	/* for DDR bus 32bit test on T1024 */
	popts->data_bus_width = DDR_DATA_BUS_WIDTH_32;
#endif

#ifdef CONFIG_TARGET_T1023RDB
	popts->wrlvl_ctl_2 = 0x07070606;
	popts->half_strength_driver_enable = 1;
	popts->cpo_sample = 0x43;
#elif defined(CONFIG_TARGET_T1024RDB)
	/* optimize cpo for erratum A-009942 */
	popts->cpo_sample = 0x52;
#endif
}

#ifdef CONFIG_SYS_DDR_RAW_TIMING
/* 2GB discrete DDR4 MT40A512M8HX on T1023RDB */
dimm_params_t ddr_raw_timing = {
	.n_ranks = 1,
	.rank_density = 0x80000000,
	.capacity = 0x80000000,
	.primary_sdram_width = 32,
	.ec_sdram_width = 8,
	.registered_dimm = 0,
	.mirrored_dimm = 0,
	.n_row_addr = 15,
	.n_col_addr = 10,
	.bank_addr_bits = 2,
	.bank_group_bits = 2,
	.edc_config = 0,
	.burst_lengths_bitmask = 0x0c,
	.tckmin_x_ps = 938,
	.tckmax_ps = 1500,
	.caslat_x = 0x000DFA00,
	.taa_ps = 13500,
	.trcd_ps = 13500,
	.trp_ps = 13500,
	.tras_ps = 33000,
	.trc_ps = 46500,
	.trfc1_ps = 260000,
	.trfc2_ps = 160000,
	.trfc4_ps = 110000,
	.tfaw_ps = 25000,
	.trrds_ps = 3700,
	.trrdl_ps = 5300,
	.tccdl_ps = 5355,
	.refresh_rate_ps = 7800000,
	.dq_mapping[0] = 0x0,
	.dq_mapping[1] = 0x0,
	.dq_mapping[2] = 0x0,
	.dq_mapping[3] = 0x0,
	.dq_mapping[4] = 0x0,
	.dq_mapping[5] = 0x0,
	.dq_mapping[6] = 0x0,
	.dq_mapping[7] = 0x0,
	.dq_mapping[8] = 0x0,
	.dq_mapping[9] = 0x0,
	.dq_mapping[10] = 0x0,
	.dq_mapping[11] = 0x0,
	.dq_mapping[12] = 0x0,
	.dq_mapping[13] = 0x0,
	.dq_mapping[14] = 0x0,
	.dq_mapping[15] = 0x0,
	.dq_mapping[16] = 0x0,
	.dq_mapping[17] = 0x0,
	.dq_mapping_ors = 1,
};

int fsl_ddr_get_dimm_params(dimm_params_t *pdimm,
		unsigned int controller_number,
		unsigned int dimm_number)
{
	const char dimm_model[] = "Fixed DDR4 on board";

	if (((controller_number == 0) && (dimm_number == 0)) ||
	    ((controller_number == 1) && (dimm_number == 0))) {
		memcpy(pdimm, &ddr_raw_timing, sizeof(dimm_params_t));
		memset(pdimm->mpart, 0, sizeof(pdimm->mpart));
		memcpy(pdimm->mpart, dimm_model, sizeof(dimm_model) - 1);
	}

	return 0;
}
#endif

#if defined(CONFIG_DEEP_SLEEP)
void board_mem_sleep_setup(void)
{
	void __iomem *cpld_base = (void *)CONFIG_SYS_CPLD_BASE;

	/* does not provide HW signals for power management */
	clrbits_8(cpld_base + 0x17, 0x40);
	/* Disable MCKE isolation */
	gpio_set_value(2, 0);
	udelay(1);
}
#endif

int dram_init(void)
{
	phys_size_t dram_size;

#if defined(CONFIG_SPL_BUILD) || !defined(CONFIG_RAMBOOT_PBL)
#ifndef CONFIG_SYS_DDR_RAW_TIMING
	puts("Initializing....using SPD\n");
#endif
	dram_size = fsl_ddr_sdram();
#else
	/* DDR has been initialised by first stage boot loader */
	dram_size =  fsl_ddr_sdram_size();
#endif
	dram_size = setup_ddr_tlbs(dram_size / 0x100000);
	dram_size *= 0x100000;

#if defined(CONFIG_DEEP_SLEEP) && !defined(CONFIG_SPL_BUILD)
	fsl_dp_resume();
#endif

	gd->ram_size = dram_size;

	return 0;
}
