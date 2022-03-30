// SPDX-License-Identifier: GPL-2.0+
/*
 * Clock setup for SMDK5250 board based on EXYNOS5
 *
 * Copyright (C) 2012 Samsung Electronics
 */

#include <common.h>
#include <config.h>
#include <asm/io.h>
#include <asm/arch/clk.h>
#include <asm/arch/clock.h>
#include <asm/arch/spl.h>
#include <asm/arch/dwmmc.h>

#include "clock_init.h"
#include "common_setup.h"
#include "exynos5_setup.h"

#define FSYS1_MMC0_DIV_MASK	0xff0f
#define FSYS1_MMC0_DIV_VAL	0x0701

struct arm_clk_ratios arm_clk_ratios[] = {
#ifdef CONFIG_EXYNOS5420
	{
		.arm_freq_mhz = 900,

		.apll_mdiv = 0x96,
		.apll_pdiv = 0x2,
		.apll_sdiv = 0x1,

		.arm2_ratio = 0x0,
		.apll_ratio = 0x3,
		.pclk_dbg_ratio = 0x6,
		.atb_ratio = 0x6,
		.periph_ratio = 0x7,
		.acp_ratio = 0x0,
		.cpud_ratio = 0x2,
		.arm_ratio = 0x0,
	}
#else
	{
		.arm_freq_mhz = 600,

		.apll_mdiv = 0xc8,
		.apll_pdiv = 0x4,
		.apll_sdiv = 0x1,

		.arm2_ratio = 0x0,
		.apll_ratio = 0x1,
		.pclk_dbg_ratio = 0x1,
		.atb_ratio = 0x2,
		.periph_ratio = 0x7,
		.acp_ratio = 0x7,
		.cpud_ratio = 0x1,
		.arm_ratio = 0x0,
	}, {
		.arm_freq_mhz = 800,

		.apll_mdiv = 0x64,
		.apll_pdiv = 0x3,
		.apll_sdiv = 0x0,

		.arm2_ratio = 0x0,
		.apll_ratio = 0x1,
		.pclk_dbg_ratio = 0x1,
		.atb_ratio = 0x3,
		.periph_ratio = 0x7,
		.acp_ratio = 0x7,
		.cpud_ratio = 0x2,
		.arm_ratio = 0x0,
	}, {
		.arm_freq_mhz = 1000,

		.apll_mdiv = 0x7d,
		.apll_pdiv = 0x3,
		.apll_sdiv = 0x0,

		.arm2_ratio = 0x0,
		.apll_ratio = 0x1,
		.pclk_dbg_ratio = 0x1,
		.atb_ratio = 0x4,
		.periph_ratio = 0x7,
		.acp_ratio = 0x7,
		.cpud_ratio = 0x2,
		.arm_ratio = 0x0,
	}, {
		.arm_freq_mhz = 1200,

		.apll_mdiv = 0x96,
		.apll_pdiv = 0x3,
		.apll_sdiv = 0x0,

		.arm2_ratio = 0x0,
		.apll_ratio = 0x3,
		.pclk_dbg_ratio = 0x1,
		.atb_ratio = 0x5,
		.periph_ratio = 0x7,
		.acp_ratio = 0x7,
		.cpud_ratio = 0x3,
		.arm_ratio = 0x0,
	}, {
		.arm_freq_mhz = 1400,

		.apll_mdiv = 0xaf,
		.apll_pdiv = 0x3,
		.apll_sdiv = 0x0,

		.arm2_ratio = 0x0,
		.apll_ratio = 0x3,
		.pclk_dbg_ratio = 0x1,
		.atb_ratio = 0x6,
		.periph_ratio = 0x7,
		.acp_ratio = 0x7,
		.cpud_ratio = 0x3,
		.arm_ratio = 0x0,
	}, {
		.arm_freq_mhz = 1700,

		.apll_mdiv = 0x1a9,
		.apll_pdiv = 0x6,
		.apll_sdiv = 0x0,

		.arm2_ratio = 0x0,
		.apll_ratio = 0x3,
		.pclk_dbg_ratio = 0x1,
		.atb_ratio = 0x6,
		.periph_ratio = 0x7,
		.acp_ratio = 0x7,
		.cpud_ratio = 0x3,
		.arm_ratio = 0x0,
	}
#endif
};

struct mem_timings mem_timings[] = {
#ifdef CONFIG_EXYNOS5420
	{
		.mem_manuf = MEM_MANUF_SAMSUNG,
		.mem_type = DDR_MODE_DDR3,
		.frequency_mhz = 800,

		/* MPLL @800MHz*/
		.mpll_mdiv = 0xc8,
		.mpll_pdiv = 0x3,
		.mpll_sdiv = 0x1,
		/* CPLL @666MHz */
		.cpll_mdiv = 0xde,
		.cpll_pdiv = 0x4,
		.cpll_sdiv = 0x1,
		/* EPLL @600MHz */
		.epll_mdiv = 0x64,
		.epll_pdiv = 0x2,
		.epll_sdiv = 0x1,
		/* VPLL @430MHz */
		.vpll_mdiv = 0xd7,
		.vpll_pdiv = 0x3,
		.vpll_sdiv = 0x2,
		/* BPLL @800MHz */
		.bpll_mdiv = 0xc8,
		.bpll_pdiv = 0x3,
		.bpll_sdiv = 0x1,
		/* KPLL @600MHz */
		.kpll_mdiv = 0x190,
		.kpll_pdiv = 0x4,
		.kpll_sdiv = 0x2,
		/* DPLL @600MHz */
		.dpll_mdiv = 0x190,
		.dpll_pdiv = 0x4,
		.dpll_sdiv = 0x2,
		/* IPLL @370MHz */
		.ipll_mdiv = 0xb9,
		.ipll_pdiv = 0x3,
		.ipll_sdiv = 0x2,
		/* SPLL @400MHz */
		.spll_mdiv = 0xc8,
		.spll_pdiv = 0x3,
		.spll_sdiv = 0x2,
		/* RPLL @141Mhz */
		.rpll_mdiv = 0x5E,
		.rpll_pdiv = 0x2,
		.rpll_sdiv = 0x3,

		.direct_cmd_msr = {
			0x00020018, 0x00030000, 0x00010046, 0x00000d70,
			0x00000c70
		},
		.timing_ref = 0x000000bb,
		.timing_row = 0x6836650f,
		.timing_data = 0x3630580b,
		.timing_power = 0x41000a26,
		.phy0_dqs = 0x08080808,
		.phy1_dqs = 0x08080808,
		.phy0_dq = 0x08080808,
		.phy1_dq = 0x08080808,
		.phy0_tFS = 0x8,
		.phy1_tFS = 0x8,
		.phy0_pulld_dqs = 0xf,
		.phy1_pulld_dqs = 0xf,

		.lpddr3_ctrl_phy_reset = 0x1,
		.ctrl_start_point = 0x10,
		.ctrl_inc = 0x10,
		.ctrl_start = 0x1,
		.ctrl_dll_on = 0x1,
		.ctrl_ref = 0x8,

		.ctrl_force = 0x1a,
		.ctrl_rdlat = 0x0b,
		.ctrl_bstlen = 0x08,

		.fp_resync = 0x8,
		.iv_size = 0x7,
		.dfi_init_start = 1,
		.aref_en = 1,

		.rd_fetch = 0x3,

		.zq_mode_dds = 0x7,
		.zq_mode_term = 0x1,
		.zq_mode_noterm = 1,

		/*
		* Dynamic Clock: Always Running
		* Memory Burst length: 8
		* Number of chips: 1
		* Memory Bus width: 32 bit
		* Memory Type: DDR3
		* Additional Latancy for PLL: 0 Cycle
		*/
		.memcontrol = DMC_MEMCONTROL_CLK_STOP_DISABLE |
			DMC_MEMCONTROL_DPWRDN_DISABLE |
			DMC_MEMCONTROL_DPWRDN_ACTIVE_PRECHARGE |
			DMC_MEMCONTROL_TP_DISABLE |
			DMC_MEMCONTROL_DSREF_DISABLE |
			DMC_MEMCONTROL_ADD_LAT_PALL_CYCLE(0) |
			DMC_MEMCONTROL_MEM_TYPE_DDR3 |
			DMC_MEMCONTROL_MEM_WIDTH_32BIT |
			DMC_MEMCONTROL_NUM_CHIP_1 |
			DMC_MEMCONTROL_BL_8 |
			DMC_MEMCONTROL_PZQ_DISABLE |
			DMC_MEMCONTROL_MRR_BYTE_7_0,
		.memconfig = DMC_MEMCONFIG_CHIP_MAP_SPLIT |
			DMC_MEMCONFIGX_CHIP_COL_10 |
			DMC_MEMCONFIGX_CHIP_ROW_15 |
			DMC_MEMCONFIGX_CHIP_BANK_8,
		.prechconfig_tp_cnt = 0xff,
		.dpwrdn_cyc = 0xff,
		.dsref_cyc = 0xffff,
		.concontrol = DMC_CONCONTROL_DFI_INIT_START_DISABLE |
			DMC_CONCONTROL_TIMEOUT_LEVEL0 |
			DMC_CONCONTROL_RD_FETCH_DISABLE |
			DMC_CONCONTROL_EMPTY_DISABLE |
			DMC_CONCONTROL_AREF_EN_DISABLE |
			DMC_CONCONTROL_IO_PD_CON_DISABLE,
		.dmc_channels = 1,
		.chips_per_channel = 1,
		.chips_to_configure = 1,
		.send_zq_init = 1,
		.gate_leveling_enable = 1,
		.read_leveling_enable = 0,
	}
#else
	{
		.mem_manuf = MEM_MANUF_ELPIDA,
		.mem_type = DDR_MODE_DDR3,
		.frequency_mhz = 800,
		.mpll_mdiv = 0xc8,
		.mpll_pdiv = 0x3,
		.mpll_sdiv = 0x0,
		.cpll_mdiv = 0xde,
		.cpll_pdiv = 0x4,
		.cpll_sdiv = 0x2,
		.gpll_mdiv = 0x215,
		.gpll_pdiv = 0xc,
		.gpll_sdiv = 0x1,
		.epll_mdiv = 0x60,
		.epll_pdiv = 0x3,
		.epll_sdiv = 0x3,
		.vpll_mdiv = 0x96,
		.vpll_pdiv = 0x3,
		.vpll_sdiv = 0x2,

		.bpll_mdiv = 0x64,
		.bpll_pdiv = 0x3,
		.bpll_sdiv = 0x0,
		.pclk_cdrex_ratio = 0x5,
		.direct_cmd_msr = {
			0x00020018, 0x00030000, 0x00010042, 0x00000d70
		},
		.timing_ref = 0x000000bb,
		.timing_row = 0x8c36650e,
		.timing_data = 0x3630580b,
		.timing_power = 0x41000a44,
		.phy0_dqs = 0x08080808,
		.phy1_dqs = 0x08080808,
		.phy0_dq = 0x08080808,
		.phy1_dq = 0x08080808,
		.phy0_tFS = 0x4,
		.phy1_tFS = 0x4,
		.phy0_pulld_dqs = 0xf,
		.phy1_pulld_dqs = 0xf,

		.lpddr3_ctrl_phy_reset = 0x1,
		.ctrl_start_point = 0x10,
		.ctrl_inc = 0x10,
		.ctrl_start = 0x1,
		.ctrl_dll_on = 0x1,
		.ctrl_ref = 0x8,

		.ctrl_force = 0x1a,
		.ctrl_rdlat = 0x0b,
		.ctrl_bstlen = 0x08,

		.fp_resync = 0x8,
		.iv_size = 0x7,
		.dfi_init_start = 1,
		.aref_en = 1,

		.rd_fetch = 0x3,

		.zq_mode_dds = 0x7,
		.zq_mode_term = 0x1,
		.zq_mode_noterm = 0,

		/*
		* Dynamic Clock: Always Running
		* Memory Burst length: 8
		* Number of chips: 1
		* Memory Bus width: 32 bit
		* Memory Type: DDR3
		* Additional Latancy for PLL: 0 Cycle
		*/
		.memcontrol = DMC_MEMCONTROL_CLK_STOP_DISABLE |
			DMC_MEMCONTROL_DPWRDN_DISABLE |
			DMC_MEMCONTROL_DPWRDN_ACTIVE_PRECHARGE |
			DMC_MEMCONTROL_TP_DISABLE |
			DMC_MEMCONTROL_DSREF_ENABLE |
			DMC_MEMCONTROL_ADD_LAT_PALL_CYCLE(0) |
			DMC_MEMCONTROL_MEM_TYPE_DDR3 |
			DMC_MEMCONTROL_MEM_WIDTH_32BIT |
			DMC_MEMCONTROL_NUM_CHIP_1 |
			DMC_MEMCONTROL_BL_8 |
			DMC_MEMCONTROL_PZQ_DISABLE |
			DMC_MEMCONTROL_MRR_BYTE_7_0,
		.memconfig = DMC_MEMCONFIGX_CHIP_MAP_INTERLEAVED |
			DMC_MEMCONFIGX_CHIP_COL_10 |
			DMC_MEMCONFIGX_CHIP_ROW_15 |
			DMC_MEMCONFIGX_CHIP_BANK_8,
		.membaseconfig0 = DMC_MEMBASECONFIG_VAL(0x40),
		.membaseconfig1 = DMC_MEMBASECONFIG_VAL(0x80),
		.prechconfig_tp_cnt = 0xff,
		.dpwrdn_cyc = 0xff,
		.dsref_cyc = 0xffff,
		.concontrol = DMC_CONCONTROL_DFI_INIT_START_DISABLE |
			DMC_CONCONTROL_TIMEOUT_LEVEL0 |
			DMC_CONCONTROL_RD_FETCH_DISABLE |
			DMC_CONCONTROL_EMPTY_DISABLE |
			DMC_CONCONTROL_AREF_EN_DISABLE |
			DMC_CONCONTROL_IO_PD_CON_DISABLE,
		.dmc_channels = 2,
		.chips_per_channel = 2,
		.chips_to_configure = 1,
		.send_zq_init = 1,
		.impedance = IMP_OUTPUT_DRV_30_OHM,
		.gate_leveling_enable = 0,
	}, {
		.mem_manuf = MEM_MANUF_SAMSUNG,
		.mem_type = DDR_MODE_DDR3,
		.frequency_mhz = 800,
		.mpll_mdiv = 0xc8,
		.mpll_pdiv = 0x3,
		.mpll_sdiv = 0x0,
		.cpll_mdiv = 0xde,
		.cpll_pdiv = 0x4,
		.cpll_sdiv = 0x2,
		.gpll_mdiv = 0x215,
		.gpll_pdiv = 0xc,
		.gpll_sdiv = 0x1,
		.epll_mdiv = 0x60,
		.epll_pdiv = 0x3,
		.epll_sdiv = 0x3,
		.vpll_mdiv = 0x96,
		.vpll_pdiv = 0x3,
		.vpll_sdiv = 0x2,

		.bpll_mdiv = 0x64,
		.bpll_pdiv = 0x3,
		.bpll_sdiv = 0x0,
		.pclk_cdrex_ratio = 0x5,
		.direct_cmd_msr = {
			0x00020018, 0x00030000, 0x00010000, 0x00000d70
		},
		.timing_ref = 0x000000bb,
		.timing_row = 0x8c36650e,
		.timing_data = 0x3630580b,
		.timing_power = 0x41000a44,
		.phy0_dqs = 0x08080808,
		.phy1_dqs = 0x08080808,
		.phy0_dq = 0x08080808,
		.phy1_dq = 0x08080808,
		.phy0_tFS = 0x8,
		.phy1_tFS = 0x8,
		.phy0_pulld_dqs = 0xf,
		.phy1_pulld_dqs = 0xf,

		.lpddr3_ctrl_phy_reset = 0x1,
		.ctrl_start_point = 0x10,
		.ctrl_inc = 0x10,
		.ctrl_start = 0x1,
		.ctrl_dll_on = 0x1,
		.ctrl_ref = 0x8,

		.ctrl_force = 0x1a,
		.ctrl_rdlat = 0x0b,
		.ctrl_bstlen = 0x08,

		.fp_resync = 0x8,
		.iv_size = 0x7,
		.dfi_init_start = 1,
		.aref_en = 1,

		.rd_fetch = 0x3,

		.zq_mode_dds = 0x5,
		.zq_mode_term = 0x1,
		.zq_mode_noterm = 1,

		/*
		* Dynamic Clock: Always Running
		* Memory Burst length: 8
		* Number of chips: 1
		* Memory Bus width: 32 bit
		* Memory Type: DDR3
		* Additional Latancy for PLL: 0 Cycle
		*/
		.memcontrol = DMC_MEMCONTROL_CLK_STOP_DISABLE |
			DMC_MEMCONTROL_DPWRDN_DISABLE |
			DMC_MEMCONTROL_DPWRDN_ACTIVE_PRECHARGE |
			DMC_MEMCONTROL_TP_DISABLE |
			DMC_MEMCONTROL_DSREF_ENABLE |
			DMC_MEMCONTROL_ADD_LAT_PALL_CYCLE(0) |
			DMC_MEMCONTROL_MEM_TYPE_DDR3 |
			DMC_MEMCONTROL_MEM_WIDTH_32BIT |
			DMC_MEMCONTROL_NUM_CHIP_1 |
			DMC_MEMCONTROL_BL_8 |
			DMC_MEMCONTROL_PZQ_DISABLE |
			DMC_MEMCONTROL_MRR_BYTE_7_0,
		.memconfig = DMC_MEMCONFIGX_CHIP_MAP_INTERLEAVED |
			DMC_MEMCONFIGX_CHIP_COL_10 |
			DMC_MEMCONFIGX_CHIP_ROW_15 |
			DMC_MEMCONFIGX_CHIP_BANK_8,
		.membaseconfig0 = DMC_MEMBASECONFIG_VAL(0x40),
		.membaseconfig1 = DMC_MEMBASECONFIG_VAL(0x80),
		.prechconfig_tp_cnt = 0xff,
		.dpwrdn_cyc = 0xff,
		.dsref_cyc = 0xffff,
		.concontrol = DMC_CONCONTROL_DFI_INIT_START_DISABLE |
			DMC_CONCONTROL_TIMEOUT_LEVEL0 |
			DMC_CONCONTROL_RD_FETCH_DISABLE |
			DMC_CONCONTROL_EMPTY_DISABLE |
			DMC_CONCONTROL_AREF_EN_DISABLE |
			DMC_CONCONTROL_IO_PD_CON_DISABLE,
		.dmc_channels = 2,
		.chips_per_channel = 2,
		.chips_to_configure = 1,
		.send_zq_init = 1,
		.impedance = IMP_OUTPUT_DRV_40_OHM,
		.gate_leveling_enable = 1,
	}
#endif
};

/**
 * Get the required memory type and speed (SPL version).
 *
 * In SPL we have no device tree, so we use the machine parameters
 *
 * @param mem_type	Returns memory type
 * @param frequency_mhz	Returns memory speed in MHz
 * @param arm_freq	Returns ARM clock speed in MHz
 * @param mem_manuf	Return Memory Manufacturer name
 */
static void clock_get_mem_selection(enum ddr_mode *mem_type,
		unsigned *frequency_mhz, unsigned *arm_freq,
		enum mem_manuf *mem_manuf)
{
	struct spl_machine_param *params;

	params = spl_get_machine_params();
	*mem_type = params->mem_type;
	*frequency_mhz = params->frequency_mhz;
	*arm_freq = params->arm_freq_mhz;
	*mem_manuf = params->mem_manuf;
}

/* Get the ratios for setting ARM clock */
struct arm_clk_ratios *get_arm_ratios(void)
{
	struct arm_clk_ratios *arm_ratio;
	enum ddr_mode mem_type;
	enum mem_manuf mem_manuf;
	unsigned frequency_mhz, arm_freq;
	int i;

	clock_get_mem_selection(&mem_type, &frequency_mhz,
				&arm_freq, &mem_manuf);

	for (i = 0, arm_ratio = arm_clk_ratios; i < ARRAY_SIZE(arm_clk_ratios);
		i++, arm_ratio++) {
		if (arm_ratio->arm_freq_mhz == arm_freq)
			return arm_ratio;
	}

	/* will hang if failed to find clock ratio */
	while (1)
		;

	return NULL;
}

struct mem_timings *clock_get_mem_timings(void)
{
	struct mem_timings *mem;
	enum ddr_mode mem_type;
	enum mem_manuf mem_manuf;
	unsigned frequency_mhz, arm_freq;
	int i;

	clock_get_mem_selection(&mem_type, &frequency_mhz,
				&arm_freq, &mem_manuf);
	for (i = 0, mem = mem_timings; i < ARRAY_SIZE(mem_timings);
	     i++, mem++) {
		if (mem->mem_type == mem_type &&
		    mem->frequency_mhz == frequency_mhz &&
		    mem->mem_manuf == mem_manuf)
			return mem;
	}

	/* will hang if failed to find memory timings */
	while (1)
		;

	return NULL;
}

static void exynos5250_system_clock_init(void)
{
	struct exynos5_clock *clk =
		(struct exynos5_clock *)samsung_get_base_clock();
	struct mem_timings *mem;
	struct arm_clk_ratios *arm_clk_ratio;
	u32 val, tmp;

	mem = clock_get_mem_timings();
	arm_clk_ratio = get_arm_ratios();

	clrbits_le32(&clk->src_cpu, MUX_APLL_SEL_MASK);
	do {
		val = readl(&clk->mux_stat_cpu);
	} while ((val | MUX_APLL_SEL_MASK) != val);

	clrbits_le32(&clk->src_core1, MUX_MPLL_SEL_MASK);
	do {
		val = readl(&clk->mux_stat_core1);
	} while ((val | MUX_MPLL_SEL_MASK) != val);

	clrbits_le32(&clk->src_top2, MUX_CPLL_SEL_MASK);
	clrbits_le32(&clk->src_top2, MUX_EPLL_SEL_MASK);
	clrbits_le32(&clk->src_top2, MUX_VPLL_SEL_MASK);
	clrbits_le32(&clk->src_top2, MUX_GPLL_SEL_MASK);
	tmp = MUX_CPLL_SEL_MASK | MUX_EPLL_SEL_MASK | MUX_VPLL_SEL_MASK
		| MUX_GPLL_SEL_MASK;
	do {
		val = readl(&clk->mux_stat_top2);
	} while ((val | tmp) != val);

	clrbits_le32(&clk->src_cdrex, MUX_BPLL_SEL_MASK);
	do {
		val = readl(&clk->mux_stat_cdrex);
	} while ((val | MUX_BPLL_SEL_MASK) != val);

	/* PLL locktime */
	writel(mem->apll_pdiv * PLL_LOCK_FACTOR, &clk->apll_lock);
	writel(mem->mpll_pdiv * PLL_LOCK_FACTOR, &clk->mpll_lock);
	writel(mem->bpll_pdiv * PLL_LOCK_FACTOR, &clk->bpll_lock);
	writel(mem->cpll_pdiv * PLL_LOCK_FACTOR, &clk->cpll_lock);
	writel(mem->gpll_pdiv * PLL_X_LOCK_FACTOR, &clk->gpll_lock);
	writel(mem->epll_pdiv * PLL_X_LOCK_FACTOR, &clk->epll_lock);
	writel(mem->vpll_pdiv * PLL_X_LOCK_FACTOR, &clk->vpll_lock);

	writel(CLK_REG_DISABLE, &clk->pll_div2_sel);

	writel(MUX_HPM_SEL_MASK, &clk->src_cpu);
	do {
		val = readl(&clk->mux_stat_cpu);
	} while ((val | HPM_SEL_SCLK_MPLL) != val);

	val = arm_clk_ratio->arm2_ratio << 28
		| arm_clk_ratio->apll_ratio << 24
		| arm_clk_ratio->pclk_dbg_ratio << 20
		| arm_clk_ratio->atb_ratio << 16
		| arm_clk_ratio->periph_ratio << 12
		| arm_clk_ratio->acp_ratio << 8
		| arm_clk_ratio->cpud_ratio << 4
		| arm_clk_ratio->arm_ratio;
	writel(val, &clk->div_cpu0);
	do {
		val = readl(&clk->div_stat_cpu0);
	} while (0 != val);

	writel(CLK_DIV_CPU1_VAL, &clk->div_cpu1);
	do {
		val = readl(&clk->div_stat_cpu1);
	} while (0 != val);

	/* Set APLL */
	writel(APLL_CON1_VAL, &clk->apll_con1);
	val = set_pll(arm_clk_ratio->apll_mdiv, arm_clk_ratio->apll_pdiv,
			arm_clk_ratio->apll_sdiv);
	writel(val, &clk->apll_con0);
	while ((readl(&clk->apll_con0) & APLL_CON0_LOCKED) == 0)
		;

	/* Set MPLL */
	writel(MPLL_CON1_VAL, &clk->mpll_con1);
	val = set_pll(mem->mpll_mdiv, mem->mpll_pdiv, mem->mpll_sdiv);
	writel(val, &clk->mpll_con0);
	while ((readl(&clk->mpll_con0) & MPLL_CON0_LOCKED) == 0)
		;

	/* Set BPLL */
	writel(BPLL_CON1_VAL, &clk->bpll_con1);
	val = set_pll(mem->bpll_mdiv, mem->bpll_pdiv, mem->bpll_sdiv);
	writel(val, &clk->bpll_con0);
	while ((readl(&clk->bpll_con0) & BPLL_CON0_LOCKED) == 0)
		;

	/* Set CPLL */
	writel(CPLL_CON1_VAL, &clk->cpll_con1);
	val = set_pll(mem->cpll_mdiv, mem->cpll_pdiv, mem->cpll_sdiv);
	writel(val, &clk->cpll_con0);
	while ((readl(&clk->cpll_con0) & CPLL_CON0_LOCKED) == 0)
		;

	/* Set GPLL */
	writel(GPLL_CON1_VAL, &clk->gpll_con1);
	val = set_pll(mem->gpll_mdiv, mem->gpll_pdiv, mem->gpll_sdiv);
	writel(val, &clk->gpll_con0);
	while ((readl(&clk->gpll_con0) & GPLL_CON0_LOCKED) == 0)
		;

	/* Set EPLL */
	writel(EPLL_CON2_VAL, &clk->epll_con2);
	writel(EPLL_CON1_VAL, &clk->epll_con1);
	val = set_pll(mem->epll_mdiv, mem->epll_pdiv, mem->epll_sdiv);
	writel(val, &clk->epll_con0);
	while ((readl(&clk->epll_con0) & EPLL_CON0_LOCKED) == 0)
		;

	/* Set VPLL */
	writel(VPLL_CON2_VAL, &clk->vpll_con2);
	writel(VPLL_CON1_VAL, &clk->vpll_con1);
	val = set_pll(mem->vpll_mdiv, mem->vpll_pdiv, mem->vpll_sdiv);
	writel(val, &clk->vpll_con0);
	while ((readl(&clk->vpll_con0) & VPLL_CON0_LOCKED) == 0)
		;

	writel(CLK_SRC_CORE0_VAL, &clk->src_core0);
	writel(CLK_DIV_CORE0_VAL, &clk->div_core0);
	while (readl(&clk->div_stat_core0) != 0)
		;

	writel(CLK_DIV_CORE1_VAL, &clk->div_core1);
	while (readl(&clk->div_stat_core1) != 0)
		;

	writel(CLK_DIV_SYSRGT_VAL, &clk->div_sysrgt);
	while (readl(&clk->div_stat_sysrgt) != 0)
		;

	writel(CLK_DIV_ACP_VAL, &clk->div_acp);
	while (readl(&clk->div_stat_acp) != 0)
		;

	writel(CLK_DIV_SYSLFT_VAL, &clk->div_syslft);
	while (readl(&clk->div_stat_syslft) != 0)
		;

	writel(CLK_SRC_TOP0_VAL, &clk->src_top0);
	writel(CLK_SRC_TOP1_VAL, &clk->src_top1);
	writel(TOP2_VAL, &clk->src_top2);
	writel(CLK_SRC_TOP3_VAL, &clk->src_top3);

	writel(CLK_DIV_TOP0_VAL, &clk->div_top0);
	while (readl(&clk->div_stat_top0))
		;

	writel(CLK_DIV_TOP1_VAL, &clk->div_top1);
	while (readl(&clk->div_stat_top1))
		;

	writel(CLK_SRC_LEX_VAL, &clk->src_lex);
	while (1) {
		val = readl(&clk->mux_stat_lex);
		if (val == (val | 1))
			break;
	}

	writel(CLK_DIV_LEX_VAL, &clk->div_lex);
	while (readl(&clk->div_stat_lex))
		;

	writel(CLK_DIV_R0X_VAL, &clk->div_r0x);
	while (readl(&clk->div_stat_r0x))
		;

	writel(CLK_DIV_R0X_VAL, &clk->div_r0x);
	while (readl(&clk->div_stat_r0x))
		;

	writel(CLK_DIV_R1X_VAL, &clk->div_r1x);
	while (readl(&clk->div_stat_r1x))
		;

	writel(CLK_REG_DISABLE, &clk->src_cdrex);

	writel(CLK_DIV_CDREX_VAL, &clk->div_cdrex);
	while (readl(&clk->div_stat_cdrex))
		;

	val = readl(&clk->src_cpu);
	val |= CLK_SRC_CPU_VAL;
	writel(val, &clk->src_cpu);

	val = readl(&clk->src_top2);
	val |= CLK_SRC_TOP2_VAL;
	writel(val, &clk->src_top2);

	val = readl(&clk->src_core1);
	val |= CLK_SRC_CORE1_VAL;
	writel(val, &clk->src_core1);

	writel(CLK_SRC_FSYS0_VAL, &clk->src_fsys);
	writel(CLK_DIV_FSYS0_VAL, &clk->div_fsys0);
	while (readl(&clk->div_stat_fsys0))
		;

	writel(CLK_REG_DISABLE, &clk->clkout_cmu_cpu);
	writel(CLK_REG_DISABLE, &clk->clkout_cmu_core);
	writel(CLK_REG_DISABLE, &clk->clkout_cmu_acp);
	writel(CLK_REG_DISABLE, &clk->clkout_cmu_top);
	writel(CLK_REG_DISABLE, &clk->clkout_cmu_lex);
	writel(CLK_REG_DISABLE, &clk->clkout_cmu_r0x);
	writel(CLK_REG_DISABLE, &clk->clkout_cmu_r1x);
	writel(CLK_REG_DISABLE, &clk->clkout_cmu_cdrex);

	writel(CLK_SRC_PERIC0_VAL, &clk->src_peric0);
	writel(CLK_DIV_PERIC0_VAL, &clk->div_peric0);

	writel(CLK_SRC_PERIC1_VAL, &clk->src_peric1);
	writel(CLK_DIV_PERIC1_VAL, &clk->div_peric1);
	writel(CLK_DIV_PERIC2_VAL, &clk->div_peric2);
	writel(CLK_DIV_PERIC3_VAL, &clk->div_peric3);

	writel(SCLK_SRC_ISP_VAL, &clk->sclk_src_isp);
	writel(SCLK_DIV_ISP_VAL, &clk->sclk_div_isp);
	writel(CLK_DIV_ISP0_VAL, &clk->div_isp0);
	writel(CLK_DIV_ISP1_VAL, &clk->div_isp1);
	writel(CLK_DIV_ISP2_VAL, &clk->div_isp2);

	/* FIMD1 SRC CLK SELECTION */
	writel(CLK_SRC_DISP1_0_VAL, &clk->src_disp1_0);

	val = MMC2_PRE_RATIO_VAL << MMC2_PRE_RATIO_OFFSET
		| MMC2_RATIO_VAL << MMC2_RATIO_OFFSET
		| MMC3_PRE_RATIO_VAL << MMC3_PRE_RATIO_OFFSET
		| MMC3_RATIO_VAL << MMC3_RATIO_OFFSET;
	writel(val, &clk->div_fsys2);
}

static void exynos5420_system_clock_init(void)
{
	struct exynos5420_clock *clk =
		(struct exynos5420_clock *)samsung_get_base_clock();
	struct mem_timings *mem;
	struct arm_clk_ratios *arm_clk_ratio;
	u32 val;

	mem = clock_get_mem_timings();
	arm_clk_ratio = get_arm_ratios();

	/* PLL locktime */
	writel(arm_clk_ratio->apll_pdiv * PLL_LOCK_FACTOR, &clk->apll_lock);
	writel(mem->mpll_pdiv * PLL_LOCK_FACTOR, &clk->mpll_lock);
	writel(mem->bpll_pdiv * PLL_LOCK_FACTOR, &clk->bpll_lock);
	writel(mem->cpll_pdiv * PLL_LOCK_FACTOR, &clk->cpll_lock);
	writel(mem->dpll_pdiv * PLL_LOCK_FACTOR, &clk->dpll_lock);
	writel(mem->epll_pdiv * PLL_X_LOCK_FACTOR, &clk->epll_lock);
	writel(mem->vpll_pdiv * PLL_LOCK_FACTOR, &clk->vpll_lock);
	writel(mem->ipll_pdiv * PLL_LOCK_FACTOR, &clk->ipll_lock);
	writel(mem->spll_pdiv * PLL_LOCK_FACTOR, &clk->spll_lock);
	writel(mem->kpll_pdiv * PLL_LOCK_FACTOR, &clk->kpll_lock);
	writel(mem->rpll_pdiv * PLL_X_LOCK_FACTOR, &clk->rpll_lock);

	setbits_le32(&clk->src_cpu, MUX_HPM_SEL_MASK);

	writel(0, &clk->src_top6);

	writel(0, &clk->src_cdrex);
	writel(SRC_KFC_HPM_SEL, &clk->src_kfc);
	writel(HPM_RATIO,  &clk->div_cpu1);
	writel(CLK_DIV_CPU0_VAL,  &clk->div_cpu0);

	/* switch A15 clock source to OSC clock before changing APLL */
	clrbits_le32(&clk->src_cpu, APLL_FOUT);

	/* Set APLL */
	writel(APLL_CON1_VAL, &clk->apll_con1);
	val = set_pll(arm_clk_ratio->apll_mdiv,
		      arm_clk_ratio->apll_pdiv,
		      arm_clk_ratio->apll_sdiv);
	writel(val, &clk->apll_con0);
	while ((readl(&clk->apll_con0) & PLL_LOCKED) == 0)
		;

	/* now it is safe to switch to APLL */
	setbits_le32(&clk->src_cpu, APLL_FOUT);

	writel(SRC_KFC_HPM_SEL, &clk->src_kfc);
	writel(CLK_DIV_KFC_VAL, &clk->div_kfc0);

	/* switch A7 clock source to OSC clock before changing KPLL */
	clrbits_le32(&clk->src_kfc, KPLL_FOUT);

	/* Set KPLL*/
	writel(KPLL_CON1_VAL, &clk->kpll_con1);
	val = set_pll(mem->kpll_mdiv, mem->kpll_pdiv, mem->kpll_sdiv);
	writel(val, &clk->kpll_con0);
	while ((readl(&clk->kpll_con0) & PLL_LOCKED) == 0)
		;

	/* now it is safe to switch to KPLL */
	setbits_le32(&clk->src_kfc, KPLL_FOUT);

	/* Set MPLL */
	writel(MPLL_CON1_VAL, &clk->mpll_con1);
	val = set_pll(mem->mpll_mdiv, mem->mpll_pdiv, mem->mpll_sdiv);
	writel(val, &clk->mpll_con0);
	while ((readl(&clk->mpll_con0) & PLL_LOCKED) == 0)
		;

	/* Set DPLL */
	writel(DPLL_CON1_VAL, &clk->dpll_con1);
	val = set_pll(mem->dpll_mdiv, mem->dpll_pdiv, mem->dpll_sdiv);
	writel(val, &clk->dpll_con0);
	while ((readl(&clk->dpll_con0) & PLL_LOCKED) == 0)
		;

	/* Set EPLL */
	writel(EPLL_CON2_VAL, &clk->epll_con2);
	writel(EPLL_CON1_VAL, &clk->epll_con1);
	val = set_pll(mem->epll_mdiv, mem->epll_pdiv, mem->epll_sdiv);
	writel(val, &clk->epll_con0);
	while ((readl(&clk->epll_con0) & PLL_LOCKED) == 0)
		;

	/* Set CPLL */
	writel(CPLL_CON1_VAL, &clk->cpll_con1);
	val = set_pll(mem->cpll_mdiv, mem->cpll_pdiv, mem->cpll_sdiv);
	writel(val, &clk->cpll_con0);
	while ((readl(&clk->cpll_con0) & PLL_LOCKED) == 0)
		;

	/* Set IPLL */
	writel(IPLL_CON1_VAL, &clk->ipll_con1);
	val = set_pll(mem->ipll_mdiv, mem->ipll_pdiv, mem->ipll_sdiv);
	writel(val, &clk->ipll_con0);
	while ((readl(&clk->ipll_con0) & PLL_LOCKED) == 0)
		;

	/* Set VPLL */
	writel(VPLL_CON1_VAL, &clk->vpll_con1);
	val = set_pll(mem->vpll_mdiv, mem->vpll_pdiv, mem->vpll_sdiv);
	writel(val, &clk->vpll_con0);
	while ((readl(&clk->vpll_con0) & PLL_LOCKED) == 0)
		;

	/* Set BPLL */
	writel(BPLL_CON1_VAL, &clk->bpll_con1);
	val = set_pll(mem->bpll_mdiv, mem->bpll_pdiv, mem->bpll_sdiv);
	writel(val, &clk->bpll_con0);
	while ((readl(&clk->bpll_con0) & PLL_LOCKED) == 0)
		;

	/* Set SPLL */
	writel(SPLL_CON1_VAL, &clk->spll_con1);
	val = set_pll(mem->spll_mdiv, mem->spll_pdiv, mem->spll_sdiv);
	writel(val, &clk->spll_con0);
	while ((readl(&clk->spll_con0) & PLL_LOCKED) == 0)
		;

	/* Set RPLL */
	writel(RPLL_CON2_VAL, &clk->rpll_con2);
	writel(RPLL_CON1_VAL, &clk->rpll_con1);
	val = set_pll(mem->rpll_mdiv, mem->rpll_pdiv, mem->rpll_sdiv);
	writel(val, &clk->rpll_con0);
	while ((readl(&clk->rpll_con0) & PLL_LOCKED) == 0)
		;

	writel(CLK_DIV_CDREX0_VAL, &clk->div_cdrex0);
	writel(CLK_DIV_CDREX1_VAL, &clk->div_cdrex1);

	writel(CLK_SRC_TOP0_VAL, &clk->src_top0);
	writel(CLK_SRC_TOP1_VAL, &clk->src_top1);
	writel(CLK_SRC_TOP2_VAL, &clk->src_top2);
	writel(CLK_SRC_TOP7_VAL, &clk->src_top7);

	writel(CLK_DIV_TOP0_VAL, &clk->div_top0);
	writel(CLK_DIV_TOP1_VAL, &clk->div_top1);
	writel(CLK_DIV_TOP2_VAL, &clk->div_top2);

	writel(0, &clk->src_top10);
	writel(0, &clk->src_top11);
	writel(0, &clk->src_top12);

	writel(CLK_SRC_TOP3_VAL, &clk->src_top3);
	writel(CLK_SRC_TOP4_VAL, &clk->src_top4);
	writel(CLK_SRC_TOP5_VAL, &clk->src_top5);

	/* DISP1 BLK CLK SELECTION */
	writel(CLK_SRC_DISP1_0_VAL, &clk->src_disp10);
	writel(CLK_DIV_DISP1_0_VAL, &clk->div_disp10);

	/* AUDIO BLK */
	writel(AUDIO0_SEL_EPLL, &clk->src_mau);
	writel(DIV_MAU_VAL, &clk->div_mau);

	/* FSYS */
	writel(CLK_SRC_FSYS0_VAL, &clk->src_fsys);
	writel(CLK_DIV_FSYS0_VAL, &clk->div_fsys0);
	writel(CLK_DIV_FSYS1_VAL, &clk->div_fsys1);
	writel(CLK_DIV_FSYS2_VAL, &clk->div_fsys2);

	writel(CLK_SRC_ISP_VAL, &clk->src_isp);
	writel(CLK_DIV_ISP0_VAL, &clk->div_isp0);
	writel(CLK_DIV_ISP1_VAL, &clk->div_isp1);

	writel(CLK_SRC_PERIC0_VAL, &clk->src_peric0);
	writel(CLK_SRC_PERIC1_VAL, &clk->src_peric1);

	writel(CLK_DIV_PERIC0_VAL, &clk->div_peric0);
	writel(CLK_DIV_PERIC1_VAL, &clk->div_peric1);
	writel(CLK_DIV_PERIC2_VAL, &clk->div_peric2);
	writel(CLK_DIV_PERIC3_VAL, &clk->div_peric3);
	writel(CLK_DIV_PERIC4_VAL, &clk->div_peric4);

	writel(CLK_DIV_CPERI1_VAL, &clk->div_cperi1);

	writel(CLK_DIV2_RATIO, &clk->clkdiv2_ratio);
	writel(CLK_DIV4_RATIO, &clk->clkdiv4_ratio);
	writel(CLK_DIV_G2D, &clk->div_g2d);

	writel(CLK_SRC_TOP6_VAL, &clk->src_top6);
	writel(CLK_SRC_CDREX_VAL, &clk->src_cdrex);
	writel(CLK_SRC_KFC_VAL, &clk->src_kfc);
}

void system_clock_init(void)
{
	if (proid_is_exynos542x())
		exynos5420_system_clock_init();
	else
		exynos5250_system_clock_init();
}

void clock_init_dp_clock(void)
{
	struct exynos5_clock *clk =
		(struct exynos5_clock *)samsung_get_base_clock();

	/* DP clock enable */
	setbits_le32(&clk->gate_ip_disp1, CLK_GATE_DP1_ALLOW);

	/* We run DP at 267 Mhz */
	setbits_le32(&clk->div_disp1_0, CLK_DIV_DISP1_0_FIMD1);
}

/*
 * Set clock divisor value for booting from EMMC.
 * Set DWMMC channel-0 clk div to operate mmc0 device at 50MHz.
 */
void emmc_boot_clk_div_set(void)
{
	struct exynos5_clock *clk =
		(struct exynos5_clock *)samsung_get_base_clock();
	unsigned int div_mmc;

	div_mmc = readl((unsigned int) &clk->div_fsys1) & ~FSYS1_MMC0_DIV_MASK;
	div_mmc |= FSYS1_MMC0_DIV_VAL;
	writel(div_mmc, (unsigned int) &clk->div_fsys1);
}
