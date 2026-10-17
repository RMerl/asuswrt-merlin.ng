// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014-2015, Freescale Semiconductor, Inc.
 * Copyright 2019-2020 NXP
 *
 * Derived from arch/power/cpu/mpc85xx/speed.c
 */

#include <common.h>
#include <clock_legacy.h>
#include <cpu_func.h>
#include <asm/global_data.h>
#include <linux/compiler.h>
#include <fsl_ifc.h>
#include <asm/processor.h>
#include <asm/io.h>
#include <asm/arch-fsl-layerscape/immap_lsch3.h>
#include <asm/arch/clock.h>
#include <asm/arch/soc.h>
#include "cpu.h"

DECLARE_GLOBAL_DATA_PTR;

void get_sys_info(struct sys_info *sys_info)
{
	struct ccsr_gur __iomem *gur = (void *)(CFG_SYS_FSL_GUTS_ADDR);
	struct ccsr_clk_cluster_group __iomem *clk_grp[2] = {
		(void *)(CFG_SYS_FSL_CH3_CLK_GRPA_ADDR),
		(void *)(CFG_SYS_FSL_CH3_CLK_GRPB_ADDR)
	};
	struct ccsr_clk_ctrl __iomem *clk_ctrl =
		(void *)(CFG_SYS_FSL_CH3_CLK_CTRL_ADDR);
	unsigned int cpu;
	const u8 core_cplx_pll[16] = {
		[0] = 0,	/* CC1 PPL / 1 */
		[1] = 0,	/* CC1 PPL / 2 */
		[2] = 0,	/* CC1 PPL / 4 */
		[4] = 1,	/* CC2 PPL / 1 */
		[5] = 1,	/* CC2 PPL / 2 */
		[6] = 1,	/* CC2 PPL / 4 */
		[8] = 2,	/* CC3 PPL / 1 */
		[9] = 2,	/* CC3 PPL / 2 */
		[10] = 2,	/* CC3 PPL / 4 */
		[12] = 3,	/* CC4 PPL / 1 */
		[13] = 3,	/* CC4 PPL / 2 */
		[14] = 3,	/* CC4 PPL / 4 */
	};

	const u8 core_cplx_pll_div[16] = {
		[0] = 1,	/* CC1 PPL / 1 */
		[1] = 2,	/* CC1 PPL / 2 */
		[2] = 4,	/* CC1 PPL / 4 */
		[4] = 1,	/* CC2 PPL / 1 */
		[5] = 2,	/* CC2 PPL / 2 */
		[6] = 4,	/* CC2 PPL / 4 */
		[8] = 1,	/* CC3 PPL / 1 */
		[9] = 2,	/* CC3 PPL / 2 */
		[10] = 4,	/* CC3 PPL / 4 */
		[12] = 1,	/* CC4 PPL / 1 */
		[13] = 2,	/* CC4 PPL / 2 */
		[14] = 4,	/* CC4 PPL / 4 */
	};

	uint i, cluster;
#if defined(CONFIG_ARCH_LS1028A) || defined(CONFIG_ARCH_LS1088A)
	uint rcw_tmp;
#endif
	uint freq_c_pll[CONFIG_SYS_FSL_NUM_CC_PLLS];
	uint ratio[CONFIG_SYS_FSL_NUM_CC_PLLS];
	unsigned long sysclk = get_board_sys_clk();
	int cc_group[12] = CFG_SYS_FSL_CLUSTER_CLOCKS;
	u32 c_pll_sel, cplx_pll;
	void *offset;

	sys_info->freq_systembus = sysclk;
#if defined(CONFIG_DYNAMIC_DDR_CLK_FREQ) || defined(CONFIG_STATIC_DDR_CLK_FREQ)
	sys_info->freq_ddrbus = get_board_ddr_clk();
#ifdef CONFIG_SYS_FSL_HAS_DP_DDR
	sys_info->freq_ddrbus2 = get_board_ddr_clk();
#endif
#else
	sys_info->freq_ddrbus = sysclk;
#ifdef CONFIG_SYS_FSL_HAS_DP_DDR
	sys_info->freq_ddrbus2 = sysclk;
#endif
#endif

	/* The freq_systembus is used to record frequency of platform PLL */
	sys_info->freq_systembus *= (gur_in32(&gur->rcwsr[0]) >>
			FSL_CHASSIS3_RCWSR0_SYS_PLL_RAT_SHIFT) &
			FSL_CHASSIS3_RCWSR0_SYS_PLL_RAT_MASK;
	sys_info->freq_ddrbus *= (gur_in32(&gur->rcwsr[0]) >>
			FSL_CHASSIS3_RCWSR0_MEM_PLL_RAT_SHIFT) &
			FSL_CHASSIS3_RCWSR0_MEM_PLL_RAT_MASK;
#ifdef CONFIG_SYS_FSL_HAS_DP_DDR
	if (soc_has_dp_ddr()) {
		sys_info->freq_ddrbus2 *= (gur_in32(&gur->rcwsr[0]) >>
			FSL_CHASSIS3_RCWSR0_MEM2_PLL_RAT_SHIFT) &
			FSL_CHASSIS3_RCWSR0_MEM2_PLL_RAT_MASK;
	} else {
		sys_info->freq_ddrbus2 = 0;
	}
#endif

	for (i = 0; i < CONFIG_SYS_FSL_NUM_CC_PLLS; i++) {
		/*
		 * fixme: prefer to combine the following into one line, but
		 * cannot pass compiling without warning about in_le32.
		 */
		offset = (void *)((size_t)clk_grp[i/3] +
			 offsetof(struct ccsr_clk_cluster_group,
				  pllngsr[i%3].gsr));
		ratio[i] = (in_le32(offset) >> 1) & 0x3f;
		freq_c_pll[i] = sysclk * ratio[i];
	}

	for_each_cpu(i, cpu, cpu_numcores(), cpu_mask()) {
		cluster = fsl_qoriq_core_to_cluster(cpu);
		c_pll_sel = (in_le32(&clk_ctrl->clkcncsr[cluster].csr) >> 27)
			    & 0xf;
		cplx_pll = core_cplx_pll[c_pll_sel];
		cplx_pll += cc_group[cluster] - 1;
		sys_info->freq_processor[cpu] =
			freq_c_pll[cplx_pll] / core_cplx_pll_div[c_pll_sel];
	}

#if defined(CONFIG_FSL_IFC)
	sys_info->freq_localbus = sys_info->freq_systembus /
						CONFIG_SYS_FSL_IFC_CLK_DIV;
#endif

#if defined(CONFIG_ARCH_LS1028A) || defined(CONFIG_ARCH_LS1088A)
#define HWA_CGA_M2_CLK_SEL      0x00380000
#define HWA_CGA_M2_CLK_SHIFT    19
	rcw_tmp = in_le32(&gur->rcwsr[5]);
	switch ((rcw_tmp & HWA_CGA_M2_CLK_SEL) >> HWA_CGA_M2_CLK_SHIFT) {
	case 1:
		sys_info->freq_cga_m2 = freq_c_pll[1];
		break;
	case 2:
		sys_info->freq_cga_m2 = freq_c_pll[1] / 2;
		break;
	case 3:
		sys_info->freq_cga_m2 = freq_c_pll[1] / 3;
		break;
	case 4:
		sys_info->freq_cga_m2 = freq_c_pll[1] / 4;
		break;
	case 6:
		sys_info->freq_cga_m2 = freq_c_pll[0] / 2;
		break;
	case 7:
		sys_info->freq_cga_m2 = freq_c_pll[0] / 3;
		break;
	default:
		printf("Error: Unknown peripheral clock select!\n");
		break;
	}
#endif
}

int get_clocks(void)
{
	struct sys_info sys_info;
#ifdef CONFIG_FSL_ESDHC
	u32 clock = 0;
#endif
	get_sys_info(&sys_info);
	gd->cpu_clk = sys_info.freq_processor[0];
	gd->bus_clk = sys_info.freq_systembus / CONFIG_SYS_FSL_PCLK_DIV;
	gd->mem_clk = sys_info.freq_ddrbus;
#ifdef CONFIG_SYS_FSL_HAS_DP_DDR
	gd->arch.mem2_clk = sys_info.freq_ddrbus2;
#endif

#ifdef CONFIG_FSL_ESDHC
#if defined(CONFIG_ARCH_LS1028A) || defined(CONFIG_ARCH_LS1088A)
	clock = sys_info.freq_cga_m2;
#elif defined(CONFIG_ARCH_LX2160A) || defined(CONFIG_ARCH_LS2080A) || defined(CONFIG_ARCH_LX2162A)
	clock = sys_info.freq_systembus;
#endif
	gd->arch.sdhc_per_clk = clock / CONFIG_SYS_FSL_SDHC_CLK_DIV;
	gd->arch.sdhc_clk = gd->bus_clk / CONFIG_SYS_FSL_SDHC_CLK_DIV;
#endif

	if (gd->cpu_clk != 0)
		return 0;
	else
		return 1;
}

/********************************************
 * get_bus_freq
 * return platform clock in Hz
 *********************************************/
ulong get_bus_freq(ulong dummy)
{
	if (!gd->bus_clk)
		get_clocks();

	return gd->bus_clk;
}

/********************************************
 * get_ddr_freq
 * return ddr bus freq in Hz
 *********************************************/
ulong get_ddr_freq(ulong ctrl_num)
{
	if (!gd->mem_clk)
		get_clocks();

	/*
	 * DDR controller 0 & 1 are on memory complex 0
	 * DDR controller 2 is on memory complext 1
	 */
#ifdef CONFIG_SYS_FSL_HAS_DP_DDR
	if (ctrl_num >= 2)
		return gd->arch.mem2_clk;
#endif

	return gd->mem_clk;
}

int get_i2c_freq(ulong dummy)
{
	return get_bus_freq(0) / CONFIG_SYS_FSL_I2C_CLK_DIV;
}

int get_dspi_freq(ulong dummy)
{
	return get_bus_freq(0) / CONFIG_SYS_FSL_DSPI_CLK_DIV;
}

int get_serial_clock(void)
{
	return get_bus_freq(0) / CONFIG_SYS_FSL_DUART_CLK_DIV;
}

unsigned int mxc_get_clock(enum mxc_clock clk)
{
	switch (clk) {
	case MXC_I2C_CLK:
		return get_i2c_freq(0);
	case MXC_DSPI_CLK:
		return get_dspi_freq(0);
	default:
		printf("Unsupported clock\n");
	}
	return 0;
}
