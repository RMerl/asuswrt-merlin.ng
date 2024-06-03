// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015, Freescale Semiconductor, Inc.
 */

#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/mc_cgm_regs.h>
#include <asm/arch/mc_me_regs.h>
#include <asm/arch/clock.h>

/*
 * Select the clock reference for required pll.
 * pll - ARM_PLL, PERIPH_PLL, ENET_PLL, DDR_PLL, VIDEO_PLL.
 * refclk_freq - input referece clock frequency (FXOSC - 40 MHZ, FIRC - 48 MHZ)
 */
static int select_pll_source_clk(enum pll_type pll, u32 refclk_freq)
{
	u32 clk_src;
	u32 pll_idx;
	volatile struct src *src = (struct src *)SRC_SOC_BASE_ADDR;

	/* select the pll clock source */
	switch (refclk_freq) {
	case FIRC_CLK_FREQ:
		clk_src = SRC_GPR1_FIRC_CLK_SOURCE;
		break;
	case XOSC_CLK_FREQ:
		clk_src = SRC_GPR1_XOSC_CLK_SOURCE;
		break;
	default:
		/* The clock frequency for the source clock is unknown */
		return -1;
	}
	/*
	 * The hardware definition is not uniform, it has to calculate again
	 * the recurrence formula.
	 */
	switch (pll) {
	case PERIPH_PLL:
		pll_idx = 3;
		break;
	case ENET_PLL:
		pll_idx = 1;
		break;
	case DDR_PLL:
		pll_idx = 2;
		break;
	default:
		pll_idx = pll;
	}

	writel(readl(&src->gpr1) | SRC_GPR1_PLL_SOURCE(pll_idx, clk_src),
	       &src->gpr1);

	return 0;
}

static void entry_to_target_mode(u32 mode)
{
	writel(mode | MC_ME_MCTL_KEY, MC_ME_MCTL);
	writel(mode | MC_ME_MCTL_INVERTEDKEY, MC_ME_MCTL);
	while ((readl(MC_ME_GS) & MC_ME_GS_S_MTRANS) != 0x00000000) ;
}

/*
 * Program the pll according to the input parameters.
 * pll - ARM_PLL, PERIPH_PLL, ENET_PLL, DDR_PLL, VIDEO_PLL.
 * refclk_freq - input reference clock frequency (FXOSC - 40 MHZ, FIRC - 48 MHZ)
 * freq - expected output frequency for PHY0
 * freq1 - expected output frequency for PHY1
 * dfs_nr - number of DFS modules for current PLL
 * dfs - array with the activation dfs field, mfn and mfi
 * plldv_prediv - divider of clkfreq_ref
 * plldv_mfd - loop multiplication factor divider
 * pllfd_mfn - numerator loop multiplication factor divider
 * Please consult the PLLDIG chapter of platform manual
 * before to use this function.
 *)
 */
static int program_pll(enum pll_type pll, u32 refclk_freq, u32 freq0, u32 freq1,
		       u32 dfs_nr, u32 dfs[][DFS_PARAMS_Nr], u32 plldv_prediv,
		       u32 plldv_mfd, u32 pllfd_mfn)
{
	u32 i, rfdphi1, rfdphi, dfs_on = 0, fvco;

	/*
	 * This formula is from platform reference manual (Rev. 1, 6/2015), PLLDIG chapter.
	 */
	fvco =
	    (refclk_freq / plldv_prediv) * (plldv_mfd +
					    pllfd_mfn / (float)20480);

	/*
	 * VCO should have value in [ PLL_MIN_FREQ, PLL_MAX_FREQ ]. Please consult
	 * the platform DataSheet in order to determine the allowed values.
	 */

	if (fvco < PLL_MIN_FREQ || fvco > PLL_MAX_FREQ) {
		return -1;
	}

	if (select_pll_source_clk(pll, refclk_freq) < 0) {
		return -1;
	}

	rfdphi = fvco / freq0;

	rfdphi1 = (freq1 == 0) ? 0 : fvco / freq1;

	writel(PLLDIG_PLLDV_RFDPHI1_SET(rfdphi1) |
	       PLLDIG_PLLDV_RFDPHI_SET(rfdphi) |
	       PLLDIG_PLLDV_PREDIV_SET(plldv_prediv) |
	       PLLDIG_PLLDV_MFD(plldv_mfd), PLLDIG_PLLDV(pll));

	writel(readl(PLLDIG_PLLFD(pll)) | PLLDIG_PLLFD_MFN_SET(pllfd_mfn) |
	       PLLDIG_PLLFD_SMDEN, PLLDIG_PLLFD(pll));

	/* switch on the pll in current mode */
	writel(readl(MC_ME_RUNn_MC(0)) | MC_ME_RUNMODE_MC_PLL(pll),
	       MC_ME_RUNn_MC(0));

	entry_to_target_mode(MC_ME_MCTL_RUN0);

	/* Only ARM_PLL, ENET_PLL and DDR_PLL */
	if ((pll == ARM_PLL) || (pll == ENET_PLL) || (pll == DDR_PLL)) {
		/* DFS clk enable programming */
		writel(DFS_CTRL_DLL_RESET, DFS_CTRL(pll));

		writel(DFS_DLLPRG1_CPICTRL_SET(0x5) |
		       DFS_DLLPRG1_VSETTLCTRL_SET(0x1) |
		       DFS_DLLPRG1_CALBYPEN_SET(0x0) |
		       DFS_DLLPRG1_DACIN_SET(0x1) | DFS_DLLPRG1_LCKWT_SET(0x0) |
		       DFS_DLLPRG1_V2IGC_SET(0x5), DFS_DLLPRG1(pll));

		for (i = 0; i < dfs_nr; i++) {
			if (dfs[i][0]) {
				writel(DFS_DVPORTn_MFI_SET(dfs[i][2]) |
				       DFS_DVPORTn_MFN_SET(dfs[i][1]),
				       DFS_DVPORTn(pll, i));
				dfs_on |= (dfs[i][0] << i);
			}
		}

		writel(readl(DFS_CTRL(pll)) & ~DFS_CTRL_DLL_RESET,
		       DFS_CTRL(pll));
		writel(readl(DFS_PORTRESET(pll)) &
		       ~DFS_PORTRESET_PORTRESET_SET(dfs_on),
		       DFS_PORTRESET(pll));
		while ((readl(DFS_PORTSR(pll)) & dfs_on) != dfs_on) ;
	}

	entry_to_target_mode(MC_ME_MCTL_RUN0);

	return 0;

}

static void aux_source_clk_config(uintptr_t cgm_addr, u8 ac, u32 source)
{
	/* select the clock source */
	writel(MC_CGM_ACn_SEL_SET(source), CGM_ACn_SC(cgm_addr, ac));
}

static void aux_div_clk_config(uintptr_t cgm_addr, u8 ac, u8 dc, u32 divider)
{
	/* set the divider */
	writel(MC_CGM_ACn_DCm_DE | MC_CGM_ACn_DCm_PREDIV(divider),
	       CGM_ACn_DCm(cgm_addr, ac, dc));
}

static void setup_sys_clocks(void)
{

	/* set ARM PLL DFS 1 as SYSCLK */
	writel((readl(MC_ME_RUNn_MC(0)) & ~MC_ME_RUNMODE_MC_SYSCLK_MASK) |
	       MC_ME_RUNMODE_MC_SYSCLK(0x2), MC_ME_RUNn_MC(0));

	entry_to_target_mode(MC_ME_MCTL_RUN0);

	/* select sysclks  ARMPLL, ARMPLLDFS2, ARMPLLDFS3 */
	writel(MC_ME_RUNMODE_SEC_CC_I_SYSCLK
	       (0x2,
		MC_ME_RUNMODE_SEC_CC_I_SYSCLK1_OFFSET) |
	       MC_ME_RUNMODE_SEC_CC_I_SYSCLK(0x2,
					     MC_ME_RUNMODE_SEC_CC_I_SYSCLK2_OFFSET)
	       | MC_ME_RUNMODE_SEC_CC_I_SYSCLK(0x2,
					       MC_ME_RUNMODE_SEC_CC_I_SYSCLK3_OFFSET),
	       MC_ME_RUNn_SEC_CC_I(0));

	/* setup the sys clock divider for CORE_CLK (1000MHz) */
	writel(MC_CGM_SC_DCn_DE | MC_CGM_SC_DCn_PREDIV(0x0),
	       CGM_SC_DCn(MC_CGM1_BASE_ADDR, 0));

	/* setup the sys clock divider for CORE2_CLK (500MHz) */
	writel(MC_CGM_SC_DCn_DE | MC_CGM_SC_DCn_PREDIV(0x1),
	       CGM_SC_DCn(MC_CGM1_BASE_ADDR, 1));
	/* setup the sys clock divider for SYS3_CLK (266 MHz) */
	writel(MC_CGM_SC_DCn_DE | MC_CGM_SC_DCn_PREDIV(0x0),
	       CGM_SC_DCn(MC_CGM0_BASE_ADDR, 0));

	/* setup the sys clock divider for SYS6_CLK (133 Mhz) */
	writel(MC_CGM_SC_DCn_DE | MC_CGM_SC_DCn_PREDIV(0x1),
	       CGM_SC_DCn(MC_CGM0_BASE_ADDR, 1));

	entry_to_target_mode(MC_ME_MCTL_RUN0);

}

static void setup_aux_clocks(void)
{
	/*
	 * setup the aux clock divider for PERI_CLK
	 * (source: PERIPH_PLL_PHI_0/5, PERI_CLK - 80 MHz)
	 */
	aux_source_clk_config(MC_CGM0_BASE_ADDR, 5, MC_CGM_ACn_SEL_PERPLLDIVX);
	aux_div_clk_config(MC_CGM0_BASE_ADDR, 5, 0, 4);

	/* setup the aux clock divider for LIN_CLK (40MHz) */
	aux_source_clk_config(MC_CGM0_BASE_ADDR, 3, MC_CGM_ACn_SEL_PERPLLDIVX);
	aux_div_clk_config(MC_CGM0_BASE_ADDR, 3, 0, 1);

	/* setup the aux clock divider for ENET_TIME_CLK (50MHz) */
	aux_source_clk_config(MC_CGM0_BASE_ADDR, 7, MC_CGM_ACn_SEL_ENETPLL);
	aux_div_clk_config(MC_CGM0_BASE_ADDR, 7, 1, 9);

	/* setup the aux clock divider for ENET_CLK (50MHz) */
	aux_source_clk_config(MC_CGM2_BASE_ADDR, 2, MC_CGM_ACn_SEL_ENETPLL);
	aux_div_clk_config(MC_CGM2_BASE_ADDR, 2, 0, 9);

	/* setup the aux clock divider for SDHC_CLK (50 MHz). */
	aux_source_clk_config(MC_CGM0_BASE_ADDR, 15, MC_CGM_ACn_SEL_ENETPLL);
	aux_div_clk_config(MC_CGM0_BASE_ADDR, 15, 0, 9);

	/* setup the aux clock divider for DDR_CLK (533MHz) and APEX_SYS_CLK (266MHz) */
	aux_source_clk_config(MC_CGM0_BASE_ADDR, 8, MC_CGM_ACn_SEL_DDRPLL);
	aux_div_clk_config(MC_CGM0_BASE_ADDR, 8, 0, 0);
	/* setup the aux clock divider for DDR4_CLK (133,25MHz) */
	aux_div_clk_config(MC_CGM0_BASE_ADDR, 8, 1, 3);

	entry_to_target_mode(MC_ME_MCTL_RUN0);

}

static void enable_modules_clock(void)
{
	/* PIT0 */
	writeb(MC_ME_PCTLn_RUNPCm(0), MC_ME_PCTL58);
	/* PIT1 */
	writeb(MC_ME_PCTLn_RUNPCm(0), MC_ME_PCTL170);
	/* LINFLEX0 */
	writeb(MC_ME_PCTLn_RUNPCm(0), MC_ME_PCTL83);
	/* LINFLEX1 */
	writeb(MC_ME_PCTLn_RUNPCm(0), MC_ME_PCTL188);
	/* ENET */
	writeb(MC_ME_PCTLn_RUNPCm(0), MC_ME_PCTL50);
	/* SDHC */
	writeb(MC_ME_PCTLn_RUNPCm(0), MC_ME_PCTL93);
	/* IIC0 */
	writeb(MC_ME_PCTLn_RUNPCm(0), MC_ME_PCTL81);
	/* IIC1 */
	writeb(MC_ME_PCTLn_RUNPCm(0), MC_ME_PCTL184);
	/* IIC2 */
	writeb(MC_ME_PCTLn_RUNPCm(0), MC_ME_PCTL186);
	/* MMDC0 */
	writeb(MC_ME_PCTLn_RUNPCm(0), MC_ME_PCTL54);
	/* MMDC1 */
	writeb(MC_ME_PCTLn_RUNPCm(0), MC_ME_PCTL162);

	entry_to_target_mode(MC_ME_MCTL_RUN0);
}

void clock_init(void)
{
	unsigned int arm_dfs[ARM_PLL_PHI1_DFS_Nr][DFS_PARAMS_Nr] = {
		{ARM_PLL_PHI1_DFS1_EN, ARM_PLL_PHI1_DFS1_MFN,
		 ARM_PLL_PHI1_DFS1_MFI},
		{ARM_PLL_PHI1_DFS2_EN, ARM_PLL_PHI1_DFS2_MFN,
		 ARM_PLL_PHI1_DFS2_MFI},
		{ARM_PLL_PHI1_DFS3_EN, ARM_PLL_PHI1_DFS3_MFN,
		 ARM_PLL_PHI1_DFS3_MFI}
	};

	unsigned int enet_dfs[ENET_PLL_PHI1_DFS_Nr][DFS_PARAMS_Nr] = {
		{ENET_PLL_PHI1_DFS1_EN, ENET_PLL_PHI1_DFS1_MFN,
		 ENET_PLL_PHI1_DFS1_MFI},
		{ENET_PLL_PHI1_DFS2_EN, ENET_PLL_PHI1_DFS2_MFN,
		 ENET_PLL_PHI1_DFS2_MFI},
		{ENET_PLL_PHI1_DFS3_EN, ENET_PLL_PHI1_DFS3_MFN,
		 ENET_PLL_PHI1_DFS3_MFI},
		{ENET_PLL_PHI1_DFS4_EN, ENET_PLL_PHI1_DFS4_MFN,
		 ENET_PLL_PHI1_DFS4_MFI}
	};

	unsigned int ddr_dfs[DDR_PLL_PHI1_DFS_Nr][DFS_PARAMS_Nr] = {
		{DDR_PLL_PHI1_DFS1_EN, DDR_PLL_PHI1_DFS1_MFN,
		 DDR_PLL_PHI1_DFS1_MFI},
		{DDR_PLL_PHI1_DFS2_EN, DDR_PLL_PHI1_DFS2_MFN,
		 DDR_PLL_PHI1_DFS2_MFI},
		{DDR_PLL_PHI1_DFS3_EN, DDR_PLL_PHI1_DFS3_MFN,
		 DDR_PLL_PHI1_DFS3_MFI}
	};

	writel(MC_ME_RUN_PCn_DRUN | MC_ME_RUN_PCn_RUN0 | MC_ME_RUN_PCn_RUN1 |
	       MC_ME_RUN_PCn_RUN2 | MC_ME_RUN_PCn_RUN3, MC_ME_RUN_PCn(0));

	/* turn on FXOSC */
	writel(MC_ME_RUNMODE_MC_MVRON | MC_ME_RUNMODE_MC_XOSCON |
	       MC_ME_RUNMODE_MC_FIRCON | MC_ME_RUNMODE_MC_SYSCLK(0x1),
	       MC_ME_RUNn_MC(0));

	entry_to_target_mode(MC_ME_MCTL_RUN0);

	program_pll(ARM_PLL, XOSC_CLK_FREQ, ARM_PLL_PHI0_FREQ,
		    ARM_PLL_PHI1_FREQ, ARM_PLL_PHI1_DFS_Nr, arm_dfs,
		    ARM_PLL_PLLDV_PREDIV, ARM_PLL_PLLDV_MFD, ARM_PLL_PLLDV_MFN);

	setup_sys_clocks();

	program_pll(PERIPH_PLL, XOSC_CLK_FREQ, PERIPH_PLL_PHI0_FREQ,
		    PERIPH_PLL_PHI1_FREQ, PERIPH_PLL_PHI1_DFS_Nr, NULL,
		    PERIPH_PLL_PLLDV_PREDIV, PERIPH_PLL_PLLDV_MFD,
		    PERIPH_PLL_PLLDV_MFN);

	program_pll(ENET_PLL, XOSC_CLK_FREQ, ENET_PLL_PHI0_FREQ,
		    ENET_PLL_PHI1_FREQ, ENET_PLL_PHI1_DFS_Nr, enet_dfs,
		    ENET_PLL_PLLDV_PREDIV, ENET_PLL_PLLDV_MFD,
		    ENET_PLL_PLLDV_MFN);

	program_pll(DDR_PLL, XOSC_CLK_FREQ, DDR_PLL_PHI0_FREQ,
		    DDR_PLL_PHI1_FREQ, DDR_PLL_PHI1_DFS_Nr, ddr_dfs,
		    DDR_PLL_PLLDV_PREDIV, DDR_PLL_PLLDV_MFD, DDR_PLL_PLLDV_MFN);

	program_pll(VIDEO_PLL, XOSC_CLK_FREQ, VIDEO_PLL_PHI0_FREQ,
		    VIDEO_PLL_PHI1_FREQ, VIDEO_PLL_PHI1_DFS_Nr, NULL,
		    VIDEO_PLL_PLLDV_PREDIV, VIDEO_PLL_PLLDV_MFD,
		    VIDEO_PLL_PLLDV_MFN);

	setup_aux_clocks();

	enable_modules_clock();

}
