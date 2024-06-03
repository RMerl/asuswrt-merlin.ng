// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2004, 2007-2011 Freescale Semiconductor, Inc.
 *
 * (C) Copyright 2003 Motorola Inc.
 * Xianghua Xiao, (X.Xiao@motorola.com)
 *
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>
#include <ppc_asm.tmpl>
#include <linux/compiler.h>
#include <asm/processor.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;


#ifndef CONFIG_SYS_FSL_NUM_CC_PLLS
#define CONFIG_SYS_FSL_NUM_CC_PLLS	6
#endif
/* --------------------------------------------------------------- */

void get_sys_info(sys_info_t *sys_info)
{
	volatile ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
#ifdef CONFIG_FSL_CORENET
	volatile ccsr_clk_t *clk = (void *)(CONFIG_SYS_FSL_CORENET_CLK_ADDR);
	unsigned int cpu;
#ifdef CONFIG_HETROGENOUS_CLUSTERS
	unsigned int dsp_cpu;
	uint rcw_tmp1, rcw_tmp2;
#endif
#ifdef CONFIG_SYS_FSL_QORIQ_CHASSIS2
	int cc_group[12] = CONFIG_SYS_FSL_CLUSTER_CLOCKS;
#endif
	__maybe_unused u32 svr;

	const u8 core_cplx_PLL[16] = {
		[ 0] = 0,	/* CC1 PPL / 1 */
		[ 1] = 0,	/* CC1 PPL / 2 */
		[ 2] = 0,	/* CC1 PPL / 4 */
		[ 4] = 1,	/* CC2 PPL / 1 */
		[ 5] = 1,	/* CC2 PPL / 2 */
		[ 6] = 1,	/* CC2 PPL / 4 */
		[ 8] = 2,	/* CC3 PPL / 1 */
		[ 9] = 2,	/* CC3 PPL / 2 */
		[10] = 2,	/* CC3 PPL / 4 */
		[12] = 3,	/* CC4 PPL / 1 */
		[13] = 3,	/* CC4 PPL / 2 */
		[14] = 3,	/* CC4 PPL / 4 */
	};

	const u8 core_cplx_pll_div[16] = {
		[ 0] = 1,	/* CC1 PPL / 1 */
		[ 1] = 2,	/* CC1 PPL / 2 */
		[ 2] = 4,	/* CC1 PPL / 4 */
		[ 4] = 1,	/* CC2 PPL / 1 */
		[ 5] = 2,	/* CC2 PPL / 2 */
		[ 6] = 4,	/* CC2 PPL / 4 */
		[ 8] = 1,	/* CC3 PPL / 1 */
		[ 9] = 2,	/* CC3 PPL / 2 */
		[10] = 4,	/* CC3 PPL / 4 */
		[12] = 1,	/* CC4 PPL / 1 */
		[13] = 2,	/* CC4 PPL / 2 */
		[14] = 4,	/* CC4 PPL / 4 */
	};
	uint i, freq_c_pll[CONFIG_SYS_FSL_NUM_CC_PLLS];
#if !defined(CONFIG_FM_PLAT_CLK_DIV) || !defined(CONFIG_PME_PLAT_CLK_DIV) || \
	defined(CONFIG_FSL_ESDHC_USE_PERIPHERAL_CLK)
	uint rcw_tmp;
#endif
	uint ratio[CONFIG_SYS_FSL_NUM_CC_PLLS];
	unsigned long sysclk = CONFIG_SYS_CLK_FREQ;
	uint mem_pll_rat;

	sys_info->freq_systembus = sysclk;
#ifdef CONFIG_SYS_FSL_SINGLE_SOURCE_CLK
	uint ddr_refclk_sel;
	unsigned int porsr1_sys_clk;
	porsr1_sys_clk = in_be32(&gur->porsr1) >> FSL_DCFG_PORSR1_SYSCLK_SHIFT
						& FSL_DCFG_PORSR1_SYSCLK_MASK;
	if (porsr1_sys_clk == FSL_DCFG_PORSR1_SYSCLK_DIFF)
		sys_info->diff_sysclk = 1;
	else
		sys_info->diff_sysclk = 0;

	/*
	 * DDR_REFCLK_SEL rcw bit is used to determine if DDR PLLS
	 * are driven by separate DDR Refclock or single source
	 * differential clock.
	 */
	ddr_refclk_sel = (in_be32(&gur->rcwsr[5]) >>
		      FSL_CORENET2_RCWSR5_DDR_REFCLK_SEL_SHIFT) &
		      FSL_CORENET2_RCWSR5_DDR_REFCLK_SEL_MASK;
	/*
	 * For single source clocking, both ddrclock and sysclock
	 * are driven by differential sysclock.
	 */
	if (ddr_refclk_sel == FSL_CORENET2_RCWSR5_DDR_REFCLK_SINGLE_CLK)
		sys_info->freq_ddrbus = CONFIG_SYS_CLK_FREQ;
	else
#endif
#ifdef CONFIG_DDR_CLK_FREQ
		sys_info->freq_ddrbus = CONFIG_DDR_CLK_FREQ;
#else
		sys_info->freq_ddrbus = sysclk;
#endif

	sys_info->freq_systembus *= (in_be32(&gur->rcwsr[0]) >> 25) & 0x1f;
	mem_pll_rat = (in_be32(&gur->rcwsr[0]) >>
			FSL_CORENET_RCWSR0_MEM_PLL_RAT_SHIFT)
			& FSL_CORENET_RCWSR0_MEM_PLL_RAT_MASK;
#ifdef CONFIG_SYS_FSL_ERRATUM_A007212
	if (mem_pll_rat == 0) {
		mem_pll_rat = (in_be32(&gur->rcwsr[0]) >>
			FSL_CORENET_RCWSR0_MEM_PLL_RAT_RESV_SHIFT) &
			FSL_CORENET_RCWSR0_MEM_PLL_RAT_MASK;
	}
#endif
	/* T4240/T4160 Rev2.0 MEM_PLL_RAT uses a value which is half of
	 * T4240/T4160 Rev1.0. eg. It's 12 in Rev1.0, however, for Rev2.0
	 * it uses 6.
	 * T2080 rev 1.1 and later also use half mem_pll comparing with rev 1.0
	 */
#if defined(CONFIG_ARCH_T4240) || defined(CONFIG_ARCH_T4160) || \
	defined(CONFIG_ARCH_T2080) || defined(CONFIG_ARCH_T2081)
	svr = get_svr();
	switch (SVR_SOC_VER(svr)) {
	case SVR_T4240:
	case SVR_T4160:
	case SVR_T4120:
	case SVR_T4080:
		if (SVR_MAJ(svr) >= 2)
			mem_pll_rat *= 2;
		break;
	case SVR_T2080:
	case SVR_T2081:
		if ((SVR_MAJ(svr) > 1) || (SVR_MIN(svr) >= 1))
			mem_pll_rat *= 2;
		break;
	default:
		break;
	}
#endif
	if (mem_pll_rat > 2)
		sys_info->freq_ddrbus *= mem_pll_rat;
	else
		sys_info->freq_ddrbus = sys_info->freq_systembus * mem_pll_rat;

	for (i = 0; i < CONFIG_SYS_FSL_NUM_CC_PLLS; i++) {
		ratio[i] = (in_be32(&clk->pllcgsr[i].pllcngsr) >> 1) & 0x3f;
		if (ratio[i] > 4)
			freq_c_pll[i] = sysclk * ratio[i];
		else
			freq_c_pll[i] = sys_info->freq_systembus * ratio[i];
	}

#ifdef CONFIG_SYS_FSL_QORIQ_CHASSIS2
	/*
	 * As per CHASSIS2 architeture total 12 clusters are posible and
	 * Each cluster has up to 4 cores, sharing the same PLL selection.
	 * The cluster clock assignment is SoC defined.
	 *
	 * Total 4 clock groups are possible with 3 PLLs each.
	 * as per array indices, clock group A has 0, 1, 2 numbered PLLs &
	 * clock group B has 3, 4, 6 and so on.
	 *
	 * Clock group A having PLL1, PLL2, PLL3, feeding cores of any cluster
	 * depends upon the SoC architeture. Same applies to other
	 * clock groups and clusters.
	 *
	 */
	for_each_cpu(i, cpu, cpu_numcores(), cpu_mask()) {
		int cluster = fsl_qoriq_core_to_cluster(cpu);
		u32 c_pll_sel = (in_be32(&clk->clkcsr[cluster].clkcncsr) >> 27)
				& 0xf;
		u32 cplx_pll = core_cplx_PLL[c_pll_sel];
		cplx_pll += cc_group[cluster] - 1;
		sys_info->freq_processor[cpu] =
			 freq_c_pll[cplx_pll] / core_cplx_pll_div[c_pll_sel];
	}

#ifdef CONFIG_HETROGENOUS_CLUSTERS
	for_each_cpu(i, dsp_cpu, cpu_num_dspcores(), cpu_dsp_mask()) {
		int dsp_cluster = fsl_qoriq_dsp_core_to_cluster(dsp_cpu);
		u32 c_pll_sel = (in_be32
				(&clk->clkcsr[dsp_cluster].clkcncsr) >> 27)
				& 0xf;
		u32 cplx_pll = core_cplx_PLL[c_pll_sel];
		cplx_pll += cc_group[dsp_cluster] - 1;
		sys_info->freq_processor_dsp[dsp_cpu] =
			 freq_c_pll[cplx_pll] / core_cplx_pll_div[c_pll_sel];
	}
#endif

#if defined(CONFIG_ARCH_B4860) || defined(CONFIG_ARCH_B4420) || \
	defined(CONFIG_ARCH_T2080) || defined(CONFIG_ARCH_T2081)
#define FM1_CLK_SEL	0xe0000000
#define FM1_CLK_SHIFT	29
#elif defined(CONFIG_ARCH_T1024) || defined(CONFIG_ARCH_T1023)
#define FM1_CLK_SEL	0x00000007
#define FM1_CLK_SHIFT	0
#else
#define PME_CLK_SEL	0xe0000000
#define PME_CLK_SHIFT	29
#define FM1_CLK_SEL	0x1c000000
#define FM1_CLK_SHIFT	26
#endif
#if !defined(CONFIG_FM_PLAT_CLK_DIV) || !defined(CONFIG_PME_PLAT_CLK_DIV)
#if defined(CONFIG_ARCH_T1024) || defined(CONFIG_ARCH_T1023)
	rcw_tmp = in_be32(&gur->rcwsr[15]) - 4;
#else
	rcw_tmp = in_be32(&gur->rcwsr[7]);
#endif
#endif

#ifdef CONFIG_SYS_DPAA_PME
#ifndef CONFIG_PME_PLAT_CLK_DIV
	switch ((rcw_tmp & PME_CLK_SEL) >> PME_CLK_SHIFT) {
	case 1:
		sys_info->freq_pme = freq_c_pll[CONFIG_SYS_PME_CLK];
		break;
	case 2:
		sys_info->freq_pme = freq_c_pll[CONFIG_SYS_PME_CLK] / 2;
		break;
	case 3:
		sys_info->freq_pme = freq_c_pll[CONFIG_SYS_PME_CLK] / 3;
		break;
	case 4:
		sys_info->freq_pme = freq_c_pll[CONFIG_SYS_PME_CLK] / 4;
		break;
	case 6:
		sys_info->freq_pme = freq_c_pll[CONFIG_SYS_PME_CLK + 1] / 2;
		break;
	case 7:
		sys_info->freq_pme = freq_c_pll[CONFIG_SYS_PME_CLK + 1] / 3;
		break;
	default:
		printf("Error: Unknown PME clock select!\n");
	case 0:
		sys_info->freq_pme = sys_info->freq_systembus / 2;
		break;

	}
#else
	sys_info->freq_pme = sys_info->freq_systembus / CONFIG_SYS_PME_CLK;

#endif
#endif

#ifdef CONFIG_SYS_DPAA_QBMAN
#ifndef CONFIG_QBMAN_CLK_DIV
#define CONFIG_QBMAN_CLK_DIV	2
#endif
	sys_info->freq_qman = sys_info->freq_systembus / CONFIG_QBMAN_CLK_DIV;
#endif

#if defined(CONFIG_SYS_MAPLE)
#define CPRI_CLK_SEL		0x1C000000
#define CPRI_CLK_SHIFT		26
#define CPRI_ALT_CLK_SEL	0x00007000
#define CPRI_ALT_CLK_SHIFT	12

	rcw_tmp1 = in_be32(&gur->rcwsr[7]);	/* Reading RCW bits: 224-255*/
	rcw_tmp2 = in_be32(&gur->rcwsr[15]);	/* Reading RCW bits: 480-511*/
	/* For MAPLE and CPRI frequency */
	switch ((rcw_tmp1 & CPRI_CLK_SEL) >> CPRI_CLK_SHIFT) {
	case 1:
		sys_info->freq_maple = freq_c_pll[CONFIG_SYS_CPRI_CLK];
		sys_info->freq_cpri = freq_c_pll[CONFIG_SYS_CPRI_CLK];
		break;
	case 2:
		sys_info->freq_maple = freq_c_pll[CONFIG_SYS_CPRI_CLK] / 2;
		sys_info->freq_cpri = freq_c_pll[CONFIG_SYS_CPRI_CLK] / 2;
		break;
	case 3:
		sys_info->freq_maple = freq_c_pll[CONFIG_SYS_CPRI_CLK] / 3;
		sys_info->freq_cpri = freq_c_pll[CONFIG_SYS_CPRI_CLK] / 3;
		break;
	case 4:
		sys_info->freq_maple = freq_c_pll[CONFIG_SYS_CPRI_CLK] / 4;
		sys_info->freq_cpri = freq_c_pll[CONFIG_SYS_CPRI_CLK] / 4;
		break;
	case 5:
		if (((rcw_tmp2 & CPRI_ALT_CLK_SEL)
					>> CPRI_ALT_CLK_SHIFT) == 6) {
			sys_info->freq_maple =
				freq_c_pll[CONFIG_SYS_CPRI_CLK - 2] / 2;
			sys_info->freq_cpri =
				freq_c_pll[CONFIG_SYS_CPRI_CLK - 2] / 2;
		}
		if (((rcw_tmp2 & CPRI_ALT_CLK_SEL)
					>> CPRI_ALT_CLK_SHIFT) == 7) {
			sys_info->freq_maple =
				freq_c_pll[CONFIG_SYS_CPRI_CLK - 2] / 3;
			sys_info->freq_cpri =
				freq_c_pll[CONFIG_SYS_CPRI_CLK - 2] / 3;
		}
		break;
	case 6:
		sys_info->freq_maple = freq_c_pll[CONFIG_SYS_CPRI_CLK + 1] / 2;
		sys_info->freq_cpri = freq_c_pll[CONFIG_SYS_CPRI_CLK + 1] / 2;
		break;
	case 7:
		sys_info->freq_maple = freq_c_pll[CONFIG_SYS_CPRI_CLK + 1] / 3;
		sys_info->freq_cpri = freq_c_pll[CONFIG_SYS_CPRI_CLK + 1] / 3;
		break;
	default:
		printf("Error: Unknown MAPLE/CPRI clock select!\n");
	}

	/* For MAPLE ULB and eTVPE frequencies */
#define ULB_CLK_SEL		0x00000038
#define ULB_CLK_SHIFT		3
#define ETVPE_CLK_SEL		0x00000007
#define ETVPE_CLK_SHIFT		0

	switch ((rcw_tmp2 & ULB_CLK_SEL) >> ULB_CLK_SHIFT) {
	case 1:
		sys_info->freq_maple_ulb = freq_c_pll[CONFIG_SYS_ULB_CLK];
		break;
	case 2:
		sys_info->freq_maple_ulb = freq_c_pll[CONFIG_SYS_ULB_CLK] / 2;
		break;
	case 3:
		sys_info->freq_maple_ulb = freq_c_pll[CONFIG_SYS_ULB_CLK] / 3;
		break;
	case 4:
		sys_info->freq_maple_ulb = freq_c_pll[CONFIG_SYS_ULB_CLK] / 4;
		break;
	case 5:
		sys_info->freq_maple_ulb = sys_info->freq_systembus;
		break;
	case 6:
		sys_info->freq_maple_ulb =
			freq_c_pll[CONFIG_SYS_ULB_CLK - 1] / 2;
		break;
	case 7:
		sys_info->freq_maple_ulb =
			freq_c_pll[CONFIG_SYS_ULB_CLK - 1] / 3;
		break;
	default:
		printf("Error: Unknown MAPLE ULB clock select!\n");
	}

	switch ((rcw_tmp2 & ETVPE_CLK_SEL) >> ETVPE_CLK_SHIFT) {
	case 1:
		sys_info->freq_maple_etvpe = freq_c_pll[CONFIG_SYS_ETVPE_CLK];
		break;
	case 2:
		sys_info->freq_maple_etvpe =
			freq_c_pll[CONFIG_SYS_ETVPE_CLK] / 2;
		break;
	case 3:
		sys_info->freq_maple_etvpe =
			freq_c_pll[CONFIG_SYS_ETVPE_CLK] / 3;
		break;
	case 4:
		sys_info->freq_maple_etvpe =
			freq_c_pll[CONFIG_SYS_ETVPE_CLK] / 4;
		break;
	case 5:
		sys_info->freq_maple_etvpe = sys_info->freq_systembus;
		break;
	case 6:
		sys_info->freq_maple_etvpe =
			freq_c_pll[CONFIG_SYS_ETVPE_CLK - 1] / 2;
		break;
	case 7:
		sys_info->freq_maple_etvpe =
			freq_c_pll[CONFIG_SYS_ETVPE_CLK - 1] / 3;
		break;
	default:
		printf("Error: Unknown MAPLE eTVPE clock select!\n");
	}

#endif

#ifdef CONFIG_SYS_DPAA_FMAN
#ifndef CONFIG_FM_PLAT_CLK_DIV
	switch ((rcw_tmp & FM1_CLK_SEL) >> FM1_CLK_SHIFT) {
	case 1:
		sys_info->freq_fman[0] = freq_c_pll[CONFIG_SYS_FM1_CLK];
		break;
	case 2:
		sys_info->freq_fman[0] = freq_c_pll[CONFIG_SYS_FM1_CLK] / 2;
		break;
	case 3:
		sys_info->freq_fman[0] = freq_c_pll[CONFIG_SYS_FM1_CLK] / 3;
		break;
	case 4:
		sys_info->freq_fman[0] = freq_c_pll[CONFIG_SYS_FM1_CLK] / 4;
		break;
	case 5:
		sys_info->freq_fman[0] = sys_info->freq_systembus;
		break;
	case 6:
		sys_info->freq_fman[0] = freq_c_pll[CONFIG_SYS_FM1_CLK + 1] / 2;
		break;
	case 7:
		sys_info->freq_fman[0] = freq_c_pll[CONFIG_SYS_FM1_CLK + 1] / 3;
		break;
	default:
		printf("Error: Unknown FMan1 clock select!\n");
	case 0:
		sys_info->freq_fman[0] = sys_info->freq_systembus / 2;
		break;
	}
#if (CONFIG_SYS_NUM_FMAN) == 2
#ifdef CONFIG_SYS_FM2_CLK
#define FM2_CLK_SEL	0x00000038
#define FM2_CLK_SHIFT	3
	rcw_tmp = in_be32(&gur->rcwsr[15]);
	switch ((rcw_tmp & FM2_CLK_SEL) >> FM2_CLK_SHIFT) {
	case 1:
		sys_info->freq_fman[1] = freq_c_pll[CONFIG_SYS_FM2_CLK + 1];
		break;
	case 2:
		sys_info->freq_fman[1] = freq_c_pll[CONFIG_SYS_FM2_CLK + 1] / 2;
		break;
	case 3:
		sys_info->freq_fman[1] = freq_c_pll[CONFIG_SYS_FM2_CLK + 1] / 3;
		break;
	case 4:
		sys_info->freq_fman[1] = freq_c_pll[CONFIG_SYS_FM2_CLK + 1] / 4;
		break;
	case 5:
		sys_info->freq_fman[1] = sys_info->freq_systembus;
		break;
	case 6:
		sys_info->freq_fman[1] = freq_c_pll[CONFIG_SYS_FM2_CLK] / 2;
		break;
	case 7:
		sys_info->freq_fman[1] = freq_c_pll[CONFIG_SYS_FM2_CLK] / 3;
		break;
	default:
		printf("Error: Unknown FMan2 clock select!\n");
	case 0:
		sys_info->freq_fman[1] = sys_info->freq_systembus / 2;
		break;
	}
#endif
#endif	/* CONFIG_SYS_NUM_FMAN == 2 */
#else
	sys_info->freq_fman[0] = sys_info->freq_systembus / CONFIG_SYS_FM1_CLK;
#endif
#endif

#ifdef CONFIG_FSL_ESDHC_USE_PERIPHERAL_CLK
#if defined(CONFIG_ARCH_T2080)
#define ESDHC_CLK_SEL	0x00000007
#define ESDHC_CLK_SHIFT	0
#define ESDHC_CLK_RCWSR	15
#else	/* Support T1040 T1024 by now */
#define ESDHC_CLK_SEL	0xe0000000
#define ESDHC_CLK_SHIFT	29
#define ESDHC_CLK_RCWSR	7
#endif
	rcw_tmp = in_be32(&gur->rcwsr[ESDHC_CLK_RCWSR]);
	switch ((rcw_tmp & ESDHC_CLK_SEL) >> ESDHC_CLK_SHIFT) {
	case 1:
		sys_info->freq_sdhc = freq_c_pll[CONFIG_SYS_SDHC_CLK];
		break;
	case 2:
		sys_info->freq_sdhc = freq_c_pll[CONFIG_SYS_SDHC_CLK] / 2;
		break;
	case 3:
		sys_info->freq_sdhc = freq_c_pll[CONFIG_SYS_SDHC_CLK] / 3;
		break;
#if defined(CONFIG_SYS_SDHC_CLK_2_PLL)
	case 4:
		sys_info->freq_sdhc = freq_c_pll[CONFIG_SYS_SDHC_CLK] / 4;
		break;
#if defined(CONFIG_ARCH_T2080)
	case 5:
		sys_info->freq_sdhc = freq_c_pll[1 - CONFIG_SYS_SDHC_CLK];
		break;
#endif
	case 6:
		sys_info->freq_sdhc = freq_c_pll[1 - CONFIG_SYS_SDHC_CLK] / 2;
		break;
	case 7:
		sys_info->freq_sdhc = freq_c_pll[1 - CONFIG_SYS_SDHC_CLK] / 3;
		break;
#endif
	default:
		sys_info->freq_sdhc = 0;
		printf("Error: Unknown SDHC peripheral clock select!\n");
	}
#endif
#else /* CONFIG_SYS_FSL_QORIQ_CHASSIS2 */

	for_each_cpu(i, cpu, cpu_numcores(), cpu_mask()) {
		u32 c_pll_sel = (in_be32(&clk->clkcsr[cpu].clkcncsr) >> 27)
				& 0xf;
		u32 cplx_pll = core_cplx_PLL[c_pll_sel];

		sys_info->freq_processor[cpu] =
			 freq_c_pll[cplx_pll] / core_cplx_pll_div[c_pll_sel];
	}
#define PME_CLK_SEL	0x80000000
#define FM1_CLK_SEL	0x40000000
#define FM2_CLK_SEL	0x20000000
#define HWA_ASYNC_DIV	0x04000000
#if (CONFIG_SYS_FSL_NUM_CC_PLLS == 2)
#define HWA_CC_PLL	1
#elif (CONFIG_SYS_FSL_NUM_CC_PLLS == 3)
#define HWA_CC_PLL	2
#elif (CONFIG_SYS_FSL_NUM_CC_PLLS == 4)
#define HWA_CC_PLL	2
#else
#error CONFIG_SYS_FSL_NUM_CC_PLLS not set or unknown case
#endif
	rcw_tmp = in_be32(&gur->rcwsr[7]);

#ifdef CONFIG_SYS_DPAA_PME
	if (rcw_tmp & PME_CLK_SEL) {
		if (rcw_tmp & HWA_ASYNC_DIV)
			sys_info->freq_pme = freq_c_pll[HWA_CC_PLL] / 4;
		else
			sys_info->freq_pme = freq_c_pll[HWA_CC_PLL] / 2;
	} else {
		sys_info->freq_pme = sys_info->freq_systembus / 2;
	}
#endif

#ifdef CONFIG_SYS_DPAA_FMAN
	if (rcw_tmp & FM1_CLK_SEL) {
		if (rcw_tmp & HWA_ASYNC_DIV)
			sys_info->freq_fman[0] = freq_c_pll[HWA_CC_PLL] / 4;
		else
			sys_info->freq_fman[0] = freq_c_pll[HWA_CC_PLL] / 2;
	} else {
		sys_info->freq_fman[0] = sys_info->freq_systembus / 2;
	}
#if (CONFIG_SYS_NUM_FMAN) == 2
	if (rcw_tmp & FM2_CLK_SEL) {
		if (rcw_tmp & HWA_ASYNC_DIV)
			sys_info->freq_fman[1] = freq_c_pll[HWA_CC_PLL] / 4;
		else
			sys_info->freq_fman[1] = freq_c_pll[HWA_CC_PLL] / 2;
	} else {
		sys_info->freq_fman[1] = sys_info->freq_systembus / 2;
	}
#endif
#endif

#ifdef CONFIG_SYS_DPAA_QBMAN
	sys_info->freq_qman = sys_info->freq_systembus / 2;
#endif

#endif /* CONFIG_SYS_FSL_QORIQ_CHASSIS2 */

#ifdef CONFIG_U_QE
	sys_info->freq_qe =  sys_info->freq_systembus / 2;
#endif

#else /* CONFIG_FSL_CORENET */
	uint plat_ratio, e500_ratio, half_freq_systembus;
	int i;
#ifdef CONFIG_QE
	__maybe_unused u32 qe_ratio;
#endif

	plat_ratio = (gur->porpllsr) & 0x0000003e;
	plat_ratio >>= 1;
	sys_info->freq_systembus = plat_ratio * CONFIG_SYS_CLK_FREQ;

	/* Divide before multiply to avoid integer
	 * overflow for processor speeds above 2GHz */
	half_freq_systembus = sys_info->freq_systembus/2;
	for (i = 0; i < cpu_numcores(); i++) {
		e500_ratio = ((gur->porpllsr) >> (i * 8 + 16)) & 0x3f;
		sys_info->freq_processor[i] = e500_ratio * half_freq_systembus;
	}

	/* Note: freq_ddrbus is the MCLK frequency, not the data rate. */
	sys_info->freq_ddrbus = sys_info->freq_systembus;

#ifdef CONFIG_DDR_CLK_FREQ
	{
		u32 ddr_ratio = ((gur->porpllsr) & MPC85xx_PORPLLSR_DDR_RATIO)
			>> MPC85xx_PORPLLSR_DDR_RATIO_SHIFT;
		if (ddr_ratio != 0x7)
			sys_info->freq_ddrbus = ddr_ratio * CONFIG_DDR_CLK_FREQ;
	}
#endif

#ifdef CONFIG_QE
#if defined(CONFIG_ARCH_P1021) || defined(CONFIG_ARCH_P1025)
	sys_info->freq_qe =  sys_info->freq_systembus;
#else
	qe_ratio = ((gur->porpllsr) & MPC85xx_PORPLLSR_QE_RATIO)
			>> MPC85xx_PORPLLSR_QE_RATIO_SHIFT;
	sys_info->freq_qe = qe_ratio * CONFIG_SYS_CLK_FREQ;
#endif
#endif

#ifdef CONFIG_SYS_DPAA_FMAN
		sys_info->freq_fman[0] = sys_info->freq_systembus;
#endif

#endif /* CONFIG_FSL_CORENET */

#if defined(CONFIG_FSL_LBC)
	sys_info->freq_localbus = sys_info->freq_systembus /
						CONFIG_SYS_FSL_LBC_CLK_DIV;
#endif

#if defined(CONFIG_FSL_IFC)
	sys_info->freq_localbus = sys_info->freq_systembus /
						CONFIG_SYS_FSL_IFC_CLK_DIV;
#endif
}


int get_clocks (void)
{
	sys_info_t sys_info;
#ifdef CONFIG_ARCH_MPC8544
	volatile ccsr_gur_t *gur = (void *) CONFIG_SYS_MPC85xx_GUTS_ADDR;
#endif
#if defined(CONFIG_CPM2)
	volatile ccsr_cpm_t *cpm = (ccsr_cpm_t *)CONFIG_SYS_MPC85xx_CPM_ADDR;
	uint sccr, dfbrg;

	/* set VCO = 4 * BRG */
	cpm->im_cpm_intctl.sccr &= 0xfffffffc;
	sccr = cpm->im_cpm_intctl.sccr;
	dfbrg = (sccr & SCCR_DFBRG_MSK) >> SCCR_DFBRG_SHIFT;
#endif
	get_sys_info (&sys_info);
	gd->cpu_clk = sys_info.freq_processor[0];
	gd->bus_clk = sys_info.freq_systembus;
	gd->mem_clk = sys_info.freq_ddrbus;
	gd->arch.lbc_clk = sys_info.freq_localbus;

#ifdef CONFIG_QE
	gd->arch.qe_clk = sys_info.freq_qe;
	gd->arch.brg_clk = gd->arch.qe_clk / 2;
#endif
	/*
	 * The base clock for I2C depends on the actual SOC.  Unfortunately,
	 * there is no pattern that can be used to determine the frequency, so
	 * the only choice is to look up the actual SOC number and use the value
	 * for that SOC. This information is taken from application note
	 * AN2919.
	 */
#if defined(CONFIG_ARCH_MPC8540) || defined(CONFIG_ARCH_MPC8541) || \
	defined(CONFIG_ARCH_MPC8560) || defined(CONFIG_ARCH_MPC8555) || \
	defined(CONFIG_ARCH_P1022)
	gd->arch.i2c1_clk = sys_info.freq_systembus;
#elif defined(CONFIG_ARCH_MPC8544)
	/*
	 * On the 8544, the I2C clock is the same as the SEC clock.  This can be
	 * either CCB/2 or CCB/3, depending on the value of cfg_sec_freq. See
	 * 4.4.3.3 of the 8544 RM.  Note that this might actually work for all
	 * 85xx, but only the 8544 has cfg_sec_freq, so it's unknown if the
	 * PORDEVSR2_SEC_CFG bit is 0 on all 85xx boards that are not an 8544.
	 */
	if (gur->pordevsr2 & MPC85xx_PORDEVSR2_SEC_CFG)
		gd->arch.i2c1_clk = sys_info.freq_systembus / 3;
	else
		gd->arch.i2c1_clk = sys_info.freq_systembus / 2;
#else
	/* Most 85xx SOCs use CCB/2, so this is the default behavior. */
	gd->arch.i2c1_clk = sys_info.freq_systembus / 2;
#endif
	gd->arch.i2c2_clk = gd->arch.i2c1_clk;

#if defined(CONFIG_FSL_ESDHC)
#ifdef CONFIG_FSL_ESDHC_USE_PERIPHERAL_CLK
	gd->arch.sdhc_clk = sys_info.freq_sdhc / 2;
#else
#if defined(CONFIG_ARCH_MPC8569) || defined(CONFIG_ARCH_P1010)
	gd->arch.sdhc_clk = gd->bus_clk;
#else
	gd->arch.sdhc_clk = gd->bus_clk / 2;
#endif
#endif
#endif /* defined(CONFIG_FSL_ESDHC) */

#if defined(CONFIG_CPM2)
	gd->arch.vco_out = 2*sys_info.freq_systembus;
	gd->arch.cpm_clk = gd->arch.vco_out / 2;
	gd->arch.scc_clk = gd->arch.vco_out / 4;
	gd->arch.brg_clk = gd->arch.vco_out / (1 << (2 * (dfbrg + 1)));
#endif

	if(gd->cpu_clk != 0) return (0);
	else return (1);
}


/********************************************
 * get_bus_freq
 * return system bus freq in Hz
 *********************************************/
ulong get_bus_freq (ulong dummy)
{
	return gd->bus_clk;
}

/********************************************
 * get_ddr_freq
 * return ddr bus freq in Hz
 *********************************************/
ulong get_ddr_freq (ulong dummy)
{
	return gd->mem_clk;
}
