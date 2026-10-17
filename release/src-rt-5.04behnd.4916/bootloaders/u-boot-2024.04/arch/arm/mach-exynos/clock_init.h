/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Clock initialization routines
 *
 * Copyright (c) 2011 The Chromium OS Authors.
 */

#ifndef __EXYNOS_CLOCK_INIT_H
#define __EXYNOS_CLOCK_INIT_H

enum {
#ifdef CONFIG_EXYNOS5420
	MEM_TIMINGS_MSR_COUNT	= 5,
#else
	MEM_TIMINGS_MSR_COUNT	= 4,
#endif
};

/* These are the ratio's for configuring ARM clock */
struct arm_clk_ratios {
	unsigned arm_freq_mhz;		/* Frequency of ARM core in MHz */

	unsigned apll_mdiv;
	unsigned apll_pdiv;
	unsigned apll_sdiv;

	unsigned arm2_ratio;
	unsigned apll_ratio;
	unsigned pclk_dbg_ratio;
	unsigned atb_ratio;
	unsigned periph_ratio;
	unsigned acp_ratio;
	unsigned cpud_ratio;
	unsigned arm_ratio;
};

/* These are the memory timings for a particular memory type and speed */
struct mem_timings {
	enum mem_manuf mem_manuf;	/* Memory manufacturer */
	enum ddr_mode mem_type;		/* Memory type */
	unsigned frequency_mhz;		/* Frequency of memory in MHz */

	/* Here follow the timing parameters for the selected memory */
	unsigned apll_mdiv;
	unsigned apll_pdiv;
	unsigned apll_sdiv;
	unsigned mpll_mdiv;
	unsigned mpll_pdiv;
	unsigned mpll_sdiv;
	unsigned cpll_mdiv;
	unsigned cpll_pdiv;
	unsigned cpll_sdiv;
	unsigned gpll_mdiv;
	unsigned gpll_pdiv;
	unsigned gpll_sdiv;
	unsigned epll_mdiv;
	unsigned epll_pdiv;
	unsigned epll_sdiv;
	unsigned vpll_mdiv;
	unsigned vpll_pdiv;
	unsigned vpll_sdiv;
	unsigned bpll_mdiv;
	unsigned bpll_pdiv;
	unsigned bpll_sdiv;
	unsigned kpll_mdiv;
	unsigned kpll_pdiv;
	unsigned kpll_sdiv;
	unsigned dpll_mdiv;
	unsigned dpll_pdiv;
	unsigned dpll_sdiv;
	unsigned ipll_mdiv;
	unsigned ipll_pdiv;
	unsigned ipll_sdiv;
	unsigned spll_mdiv;
	unsigned spll_pdiv;
	unsigned spll_sdiv;
	unsigned rpll_mdiv;
	unsigned rpll_pdiv;
	unsigned rpll_sdiv;
	unsigned pclk_cdrex_ratio;
	unsigned direct_cmd_msr[MEM_TIMINGS_MSR_COUNT];

	unsigned timing_ref;
	unsigned timing_row;
	unsigned timing_data;
	unsigned timing_power;

	/* DQS, DQ, DEBUG offsets */
	unsigned phy0_dqs;
	unsigned phy1_dqs;
	unsigned phy0_dq;
	unsigned phy1_dq;
	unsigned phy0_tFS;
	unsigned phy1_tFS;
	unsigned phy0_pulld_dqs;
	unsigned phy1_pulld_dqs;

	unsigned lpddr3_ctrl_phy_reset;
	unsigned ctrl_start_point;
	unsigned ctrl_inc;
	unsigned ctrl_start;
	unsigned ctrl_dll_on;
	unsigned ctrl_ref;

	unsigned ctrl_force;
	unsigned ctrl_rdlat;
	unsigned ctrl_bstlen;

	unsigned fp_resync;
	unsigned iv_size;
	unsigned dfi_init_start;
	unsigned aref_en;

	unsigned rd_fetch;

	unsigned zq_mode_dds;
	unsigned zq_mode_term;
	unsigned zq_mode_noterm;	/* 1 to allow termination disable */

	unsigned memcontrol;
	unsigned memconfig;

	unsigned membaseconfig0;
	unsigned membaseconfig1;
	unsigned prechconfig_tp_cnt;
	unsigned dpwrdn_cyc;
	unsigned dsref_cyc;
	unsigned concontrol;
	/* Channel and Chip Selection */
	uint8_t dmc_channels;		/* number of memory channels */
	uint8_t chips_per_channel;	/* number of chips per channel */
	uint8_t chips_to_configure;	/* number of chips to configure */
	uint8_t send_zq_init;		/* 1 to send this command */
	unsigned impedance;		/* drive strength impedeance */
	uint8_t gate_leveling_enable;	/* check gate leveling is enabled */
	uint8_t read_leveling_enable;	/* check h/w read leveling is enabled */
};

/**
 * Get the correct memory timings for our selected memory type and speed.
 *
 * This function can be called from SPL or the main U-Boot.
 *
 * Return: pointer to the memory timings that we should use
 */
struct mem_timings *clock_get_mem_timings(void);

/*
 * Initialize clock for the device
 */
void system_clock_init(void);

/*
 * Set clock divisor value for booting from EMMC.
 */
void emmc_boot_clk_div_set(void);
#endif
