// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/immap_ls102xa.h>
#include <asm/arch/clock.h>
#include <fsl_ifc.h>

DECLARE_GLOBAL_DATA_PTR;

#ifndef CONFIG_SYS_FSL_NUM_CC_PLLS
#define CONFIG_SYS_FSL_NUM_CC_PLLS      2
#endif

void get_sys_info(struct sys_info *sys_info)
{
	struct ccsr_gur __iomem *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	struct ccsr_clk *clk = (void *)(CONFIG_SYS_FSL_LS1_CLK_ADDR);
	unsigned int cpu;
	const u8 core_cplx_pll[6] = {
		[0] = 0,	/* CC1 PPL / 1 */
		[1] = 0,	/* CC1 PPL / 2 */
		[4] = 1,	/* CC2 PPL / 1 */
		[5] = 1,	/* CC2 PPL / 2 */
	};

	const u8 core_cplx_pll_div[6] = {
		[0] = 1,	/* CC1 PPL / 1 */
		[1] = 2,	/* CC1 PPL / 2 */
		[4] = 1,	/* CC2 PPL / 1 */
		[5] = 2,	/* CC2 PPL / 2 */
	};

	uint i;
	uint freq_c_pll[CONFIG_SYS_FSL_NUM_CC_PLLS];
	uint ratio[CONFIG_SYS_FSL_NUM_CC_PLLS];
	unsigned long sysclk = CONFIG_SYS_CLK_FREQ;

	sys_info->freq_systembus = sysclk;
#ifdef CONFIG_DDR_CLK_FREQ
	sys_info->freq_ddrbus = CONFIG_DDR_CLK_FREQ;
#else
	sys_info->freq_ddrbus = sysclk;
#endif

	sys_info->freq_systembus *= (in_be32(&gur->rcwsr[0]) >>
		RCWSR0_SYS_PLL_RAT_SHIFT) & RCWSR0_SYS_PLL_RAT_MASK;
	sys_info->freq_ddrbus *= (in_be32(&gur->rcwsr[0]) >>
		RCWSR0_MEM_PLL_RAT_SHIFT) & RCWSR0_MEM_PLL_RAT_MASK;

	for (i = 0; i < CONFIG_SYS_FSL_NUM_CC_PLLS; i++) {
		ratio[i] = (in_be32(&clk->pllcgsr[i].pllcngsr) >> 1) & 0x3f;
		if (ratio[i] > 4)
			freq_c_pll[i] = sysclk * ratio[i];
		else
			freq_c_pll[i] = sys_info->freq_systembus * ratio[i];
	}

	for (cpu = 0; cpu < CONFIG_MAX_CPUS; cpu++) {
		u32 c_pll_sel = (in_be32(&clk->clkcsr[cpu].clkcncsr) >> 27)
				& 0xf;
		u32 cplx_pll = core_cplx_pll[c_pll_sel];

		sys_info->freq_processor[cpu] =
			freq_c_pll[cplx_pll] / core_cplx_pll_div[c_pll_sel];
	}

#if defined(CONFIG_FSL_IFC)
	sys_info->freq_localbus = sys_info->freq_systembus;
#endif
}

int get_clocks(void)
{
	struct sys_info sys_info;

	get_sys_info(&sys_info);
	gd->cpu_clk = sys_info.freq_processor[0];
	gd->bus_clk = sys_info.freq_systembus;
	gd->mem_clk = sys_info.freq_ddrbus * 2;

#if defined(CONFIG_FSL_ESDHC)
	gd->arch.sdhc_clk = gd->bus_clk;
#endif

	return 0;
}

ulong get_bus_freq(ulong dummy)
{
	return gd->bus_clk;
}

ulong get_ddr_freq(ulong dummy)
{
	return gd->mem_clk;
}

int get_serial_clock(void)
{
	return gd->bus_clk / 2;
}

unsigned int mxc_get_clock(enum mxc_clock clk)
{
	switch (clk) {
	case MXC_I2C_CLK:
		return get_bus_freq(0) / 2;
	case MXC_ESDHC_CLK:
		return get_bus_freq(0);
	case MXC_DSPI_CLK:
		return get_bus_freq(0) / 2;
	case MXC_UART_CLK:
		return get_bus_freq(0) / 2;
	default:
		printf("Unsupported clock\n");
	}
	return 0;
}
