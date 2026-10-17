// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright (C) 2013-2017 Altera Corporation <www.altera.com>
 */

#include <common.h>
#include <time.h>
#include <asm/io.h>
#include <dm.h>
#include <asm/arch/clock_manager.h>
#include <wait_bit.h>

/*
 * function to write the bypass register which requires a poll of the
 * busy bit
 */
static void cm_write_bypass(u32 val)
{
	writel(val, socfpga_get_clkmgr_addr() + CLKMGR_GEN5_BYPASS);
	cm_wait_for_fsm();
}

/* function to write the ctrl register which requires a poll of the busy bit */
static void cm_write_ctrl(u32 val)
{
	writel(val, socfpga_get_clkmgr_addr() + CLKMGR_GEN5_CTRL);
	cm_wait_for_fsm();
}

/* function to write a clock register that has phase information */
static int cm_write_with_phase(u32 value, const void *reg_address, u32 mask)
{
	int ret;

	/* poll until phase is zero */
	ret = wait_for_bit_le32(reg_address, mask, false, 20000, false);
	if (ret)
		return ret;

	writel(value, reg_address);

	return wait_for_bit_le32(reg_address, mask, false, 20000, false);
}

/*
 * Setup clocks while making no assumptions about previous state of the clocks.
 *
 * Start by being paranoid and gate all sw managed clocks
 * Put all plls in bypass
 * Put all plls VCO registers back to reset value (bandgap power down).
 * Put peripheral and main pll src to reset value to avoid glitch.
 * Delay 5 us.
 * Deassert bandgap power down and set numerator and denominator
 * Start 7 us timer.
 * set internal dividers
 * Wait for 7 us timer.
 * Enable plls
 * Set external dividers while plls are locking
 * Wait for pll lock
 * Assert/deassert outreset all.
 * Take all pll's out of bypass
 * Clear safe mode
 * set source main and peripheral clocks
 * Ungate clocks
 */

int cm_basic_init(const struct cm_config * const cfg)
{
	unsigned long end;
	int ret;

	/* Start by being paranoid and gate all sw managed clocks */

	/*
	 * We need to disable nandclk
	 * and then do another apb access before disabling
	 * gatting off the rest of the periperal clocks.
	 */
	writel(~CLKMGR_PERPLLGRP_EN_NANDCLK_MASK &
		readl(socfpga_get_clkmgr_addr() + CLKMGR_GEN5_PERPLL_EN),
		socfpga_get_clkmgr_addr() + CLKMGR_GEN5_PERPLL_EN);

	/* DO NOT GATE OFF DEBUG CLOCKS & BRIDGE CLOCKS */
	writel(CLKMGR_MAINPLLGRP_EN_DBGTIMERCLK_MASK |
		CLKMGR_MAINPLLGRP_EN_DBGTRACECLK_MASK |
		CLKMGR_MAINPLLGRP_EN_DBGCLK_MASK |
		CLKMGR_MAINPLLGRP_EN_DBGATCLK_MASK |
		CLKMGR_MAINPLLGRP_EN_S2FUSER0CLK_MASK |
		CLKMGR_MAINPLLGRP_EN_L4MPCLK_MASK,
		socfpga_get_clkmgr_addr() + CLKMGR_GEN5_MAINPLL_EN);

	writel(0, socfpga_get_clkmgr_addr() + CLKMGR_GEN5_SDRPLL_EN);

	/* now we can gate off the rest of the peripheral clocks */
	writel(0, socfpga_get_clkmgr_addr() + CLKMGR_GEN5_PERPLL_EN);

	/* Put all plls in bypass */
	cm_write_bypass(CLKMGR_BYPASS_PERPLL | CLKMGR_BYPASS_SDRPLL |
			CLKMGR_BYPASS_MAINPLL);

	/* Put all plls VCO registers back to reset value. */
	writel(CLKMGR_MAINPLLGRP_VCO_RESET_VALUE &
	       ~CLKMGR_MAINPLLGRP_VCO_REGEXTSEL_MASK,
	       socfpga_get_clkmgr_addr() + CLKMGR_GEN5_MAINPLL_VCO);
	writel(CLKMGR_PERPLLGRP_VCO_RESET_VALUE &
	       ~CLKMGR_PERPLLGRP_VCO_REGEXTSEL_MASK,
	       socfpga_get_clkmgr_addr() + CLKMGR_GEN5_PERPLL_VCO);
	writel(CLKMGR_SDRPLLGRP_VCO_RESET_VALUE &
	       ~CLKMGR_SDRPLLGRP_VCO_REGEXTSEL_MASK,
	       socfpga_get_clkmgr_addr() + CLKMGR_GEN5_SDRPLL_VCO);

	/*
	 * The clocks to the flash devices and the L4_MAIN clocks can
	 * glitch when coming out of safe mode if their source values
	 * are different from their reset value.  So the trick it to
	 * put them back to their reset state, and change input
	 * after exiting safe mode but before ungating the clocks.
	 */
	writel(CLKMGR_PERPLLGRP_SRC_RESET_VALUE,
		      socfpga_get_clkmgr_addr() + CLKMGR_GEN5_PERPLL_SRC);
	writel(CLKMGR_MAINPLLGRP_L4SRC_RESET_VALUE,
		      socfpga_get_clkmgr_addr() + CLKMGR_GEN5_MAINPLL_L4SRC);

	/* read back for the required 5 us delay. */
	readl(socfpga_get_clkmgr_addr() + CLKMGR_GEN5_MAINPLL_VCO);
	readl(socfpga_get_clkmgr_addr() + CLKMGR_GEN5_PERPLL_VCO);
	readl(socfpga_get_clkmgr_addr() + CLKMGR_GEN5_SDRPLL_VCO);


	/*
	 * We made sure bgpwr down was assert for 5 us. Now deassert BG PWR DN
	 * with numerator and denominator.
	 */
	writel(cfg->main_vco_base,
	       socfpga_get_clkmgr_addr() + CLKMGR_GEN5_MAINPLL_VCO);
	writel(cfg->peri_vco_base,
	       socfpga_get_clkmgr_addr() + CLKMGR_GEN5_PERPLL_VCO);
	writel(cfg->sdram_vco_base,
	       socfpga_get_clkmgr_addr() + CLKMGR_GEN5_SDRPLL_VCO);

	/*
	 * Time starts here. Must wait 7 us from
	 * BGPWRDN_SET(0) to VCO_ENABLE_SET(1).
	 */
	end = timer_get_us() + 7;

	/* main mpu */
	writel(cfg->mpuclk,
	       socfpga_get_clkmgr_addr() + CLKMGR_GEN5_MAINPLL_MPUCLK);

	/* altera group mpuclk */
	writel(cfg->altera_grp_mpuclk,
	       socfpga_get_clkmgr_addr() + CLKMGR_GEN5_ALTR_MPUCLK);

	/* main main clock */
	writel(cfg->mainclk,
	       socfpga_get_clkmgr_addr() + CLKMGR_GEN5_MAINPLL_MAINCLK);

	/* main for dbg */
	writel(cfg->dbgatclk,
	       socfpga_get_clkmgr_addr() + CLKMGR_GEN5_MAINPLL_DBGATCLK);

	/* main for cfgs2fuser0clk */
	writel(cfg->cfg2fuser0clk,
	       socfpga_get_clkmgr_addr() + CLKMGR_GEN5_MAINPLL_CFGS2FUSER0CLK);

	/* Peri emac0 50 MHz default to RMII */
	writel(cfg->emac0clk,
	       socfpga_get_clkmgr_addr() + CLKMGR_GEN5_PERPLL_EMAC0CLK);

	/* Peri emac1 50 MHz default to RMII */
	writel(cfg->emac1clk,
	       socfpga_get_clkmgr_addr() + CLKMGR_GEN5_PERPLL_EMAC1CLK);

	/* Peri QSPI */
	writel(cfg->mainqspiclk,
	       socfpga_get_clkmgr_addr() + CLKMGR_GEN5_MAINPLL_MAINQSPICLK);

	writel(cfg->perqspiclk,
	       socfpga_get_clkmgr_addr() + CLKMGR_GEN5_PERPLL_PERQSPICLK);

	/* Peri pernandsdmmcclk */
	writel(cfg->mainnandsdmmcclk,
	       socfpga_get_clkmgr_addr() +
	       CLKMGR_GEN5_MAINPLL_MAINNANDSDMMCCLK);

	writel(cfg->pernandsdmmcclk,
	       socfpga_get_clkmgr_addr() + CLKMGR_GEN5_PERPLL_PERNANDSDMMCCLK);

	/* Peri perbaseclk */
	writel(cfg->perbaseclk,
	       socfpga_get_clkmgr_addr() + CLKMGR_GEN5_PERPLL_PERBASECLK);

	/* Peri s2fuser1clk */
	writel(cfg->s2fuser1clk,
	       socfpga_get_clkmgr_addr() + CLKMGR_GEN5_PERPLL_S2FUSER1CLK);

	/* 7 us must have elapsed before we can enable the VCO */
	while (timer_get_us() < end)
		;

	/* Enable vco */
	/* main pll vco */
	writel(cfg->main_vco_base | CLKMGR_MAINPLLGRP_VCO_EN,
	       socfpga_get_clkmgr_addr() + CLKMGR_GEN5_MAINPLL_VCO);

	/* periferal pll */
	writel(cfg->peri_vco_base | CLKMGR_MAINPLLGRP_VCO_EN,
	       socfpga_get_clkmgr_addr() + CLKMGR_GEN5_PERPLL_VCO);

	/* sdram pll vco */
	writel(cfg->sdram_vco_base | CLKMGR_MAINPLLGRP_VCO_EN,
	       socfpga_get_clkmgr_addr() + CLKMGR_GEN5_SDRPLL_VCO);

	/* L3 MP and L3 SP */
	writel(cfg->maindiv,
	       socfpga_get_clkmgr_addr() + CLKMGR_GEN5_MAINPLL_MAINDIV);

	writel(cfg->dbgdiv,
	       socfpga_get_clkmgr_addr() + CLKMGR_GEN5_MAINPLL_DBGDIV);

	writel(cfg->tracediv,
	       socfpga_get_clkmgr_addr() + CLKMGR_GEN5_MAINPLL_TRACEDIV);

	/* L4 MP, L4 SP, can0, and can1 */
	writel(cfg->perdiv,
	       socfpga_get_clkmgr_addr() + CLKMGR_GEN5_PERPLL_DIV);

	writel(cfg->gpiodiv,
	       socfpga_get_clkmgr_addr() + CLKMGR_GEN5_PERPLL_GPIODIV);

	cm_wait_for_lock(LOCKED_MASK);

	/* write the sdram clock counters before toggling outreset all */
	writel(cfg->ddrdqsclk & CLKMGR_SDRPLLGRP_DDRDQSCLK_CNT_MASK,
	       socfpga_get_clkmgr_addr() + CLKMGR_GEN5_SDRPLL_DDRDQSCLK);

	writel(cfg->ddr2xdqsclk & CLKMGR_SDRPLLGRP_DDR2XDQSCLK_CNT_MASK,
	       socfpga_get_clkmgr_addr() + CLKMGR_GEN5_SDRPLL_DDR2XDQSCLK);

	writel(cfg->ddrdqclk & CLKMGR_SDRPLLGRP_DDRDQCLK_CNT_MASK,
	       socfpga_get_clkmgr_addr() + CLKMGR_GEN5_SDRPLL_DDRDQCLK);

	writel(cfg->s2fuser2clk & CLKMGR_SDRPLLGRP_S2FUSER2CLK_CNT_MASK,
	       socfpga_get_clkmgr_addr() + CLKMGR_GEN5_SDRPLL_S2FUSER2CLK);

	/*
	 * after locking, but before taking out of bypass
	 * assert/deassert outresetall
	 */
	u32 mainvco = readl(socfpga_get_clkmgr_addr() +
			    CLKMGR_GEN5_MAINPLL_VCO);

	/* assert main outresetall */
	writel(mainvco | CLKMGR_MAINPLLGRP_VCO_OUTRESETALL_MASK,
	       socfpga_get_clkmgr_addr() + CLKMGR_GEN5_MAINPLL_VCO);

	u32 periphvco = readl(socfpga_get_clkmgr_addr() +
			      CLKMGR_GEN5_PERPLL_VCO);

	/* assert pheriph outresetall */
	writel(periphvco | CLKMGR_PERPLLGRP_VCO_OUTRESETALL_MASK,
	       socfpga_get_clkmgr_addr() + CLKMGR_GEN5_PERPLL_VCO);

	/* assert sdram outresetall */
	writel(cfg->sdram_vco_base | CLKMGR_MAINPLLGRP_VCO_EN |
	       CLKMGR_SDRPLLGRP_VCO_OUTRESETALL,
	       socfpga_get_clkmgr_addr() + CLKMGR_GEN5_SDRPLL_VCO);

	/* deassert main outresetall */
	writel(mainvco & ~CLKMGR_MAINPLLGRP_VCO_OUTRESETALL_MASK,
	       socfpga_get_clkmgr_addr() + CLKMGR_GEN5_MAINPLL_VCO);

	/* deassert pheriph outresetall */
	writel(periphvco & ~CLKMGR_PERPLLGRP_VCO_OUTRESETALL_MASK,
	       socfpga_get_clkmgr_addr() + CLKMGR_GEN5_PERPLL_VCO);

	/* deassert sdram outresetall */
	writel(cfg->sdram_vco_base | CLKMGR_MAINPLLGRP_VCO_EN,
	       socfpga_get_clkmgr_addr() + CLKMGR_GEN5_SDRPLL_VCO);

	/*
	 * now that we've toggled outreset all, all the clocks
	 * are aligned nicely; so we can change any phase.
	 */
	ret = cm_write_with_phase(cfg->ddrdqsclk,
				  (const void *)(socfpga_get_clkmgr_addr() +
				  CLKMGR_GEN5_SDRPLL_DDRDQSCLK),
				  CLKMGR_SDRPLLGRP_DDRDQSCLK_PHASE_MASK);
	if (ret)
		return ret;

	/* SDRAM DDR2XDQSCLK */
	ret = cm_write_with_phase(cfg->ddr2xdqsclk,
				  (const void *)(socfpga_get_clkmgr_addr() +
				  CLKMGR_GEN5_SDRPLL_DDR2XDQSCLK),
				  CLKMGR_SDRPLLGRP_DDR2XDQSCLK_PHASE_MASK);
	if (ret)
		return ret;

	ret = cm_write_with_phase(cfg->ddrdqclk,
				  (const void *)(socfpga_get_clkmgr_addr() +
				  CLKMGR_GEN5_SDRPLL_DDRDQCLK),
				  CLKMGR_SDRPLLGRP_DDRDQCLK_PHASE_MASK);
	if (ret)
		return ret;

	ret = cm_write_with_phase(cfg->s2fuser2clk,
				  (const void *)(socfpga_get_clkmgr_addr() +
				  CLKMGR_GEN5_SDRPLL_S2FUSER2CLK),
				  CLKMGR_SDRPLLGRP_S2FUSER2CLK_PHASE_MASK);
	if (ret)
		return ret;

	/* Take all three PLLs out of bypass when safe mode is cleared. */
	cm_write_bypass(0);

	/* clear safe mode */
	cm_write_ctrl(readl(socfpga_get_clkmgr_addr() + CLKMGR_GEN5_CTRL) |
		      CLKMGR_CTRL_SAFEMODE);

	/*
	 * now that safe mode is clear with clocks gated
	 * it safe to change the source mux for the flashes the the L4_MAIN
	 */
	writel(cfg->persrc,
	       socfpga_get_clkmgr_addr() + CLKMGR_GEN5_PERPLL_SRC);
	writel(cfg->l4src,
	       socfpga_get_clkmgr_addr() + CLKMGR_GEN5_MAINPLL_L4SRC);

	/* Now ungate non-hw-managed clocks */
	writel(~0, socfpga_get_clkmgr_addr() + CLKMGR_GEN5_MAINPLL_EN);
	writel(~0, socfpga_get_clkmgr_addr() + CLKMGR_GEN5_PERPLL_EN);
	writel(~0, socfpga_get_clkmgr_addr() + CLKMGR_GEN5_SDRPLL_EN);

	/* Clear the loss of lock bits (write 1 to clear) */
	writel(CLKMGR_INTER_SDRPLLLOST_MASK |
		      CLKMGR_INTER_PERPLLLOST_MASK |
		      CLKMGR_INTER_MAINPLLLOST_MASK,
		      socfpga_get_clkmgr_addr() + CLKMGR_GEN5_INTER);

	return 0;
}

static unsigned int cm_get_main_vco_clk_hz(void)
{
	u32 reg, clock;

	/* get the main VCO clock */
	reg = readl(socfpga_get_clkmgr_addr() + CLKMGR_GEN5_MAINPLL_VCO);
	clock = cm_get_osc_clk_hz(1);
	clock /= ((reg & CLKMGR_MAINPLLGRP_VCO_DENOM_MASK) >>
		  CLKMGR_MAINPLLGRP_VCO_DENOM_OFFSET) + 1;
	clock *= ((reg & CLKMGR_MAINPLLGRP_VCO_NUMER_MASK) >>
		  CLKMGR_MAINPLLGRP_VCO_NUMER_OFFSET) + 1;

	return clock;
}

static unsigned int cm_get_per_vco_clk_hz(void)
{
	u32 reg, clock = 0;

	/* identify PER PLL clock source */
	reg = readl(socfpga_get_clkmgr_addr() + CLKMGR_GEN5_PERPLL_VCO);
	reg = (reg & CLKMGR_PERPLLGRP_VCO_SSRC_MASK) >>
	      CLKMGR_PERPLLGRP_VCO_SSRC_OFFSET;
	if (reg == CLKMGR_VCO_SSRC_EOSC1)
		clock = cm_get_osc_clk_hz(1);
	else if (reg == CLKMGR_VCO_SSRC_EOSC2)
		clock = cm_get_osc_clk_hz(2);
	else if (reg == CLKMGR_VCO_SSRC_F2S)
		clock = cm_get_f2s_per_ref_clk_hz();

	/* get the PER VCO clock */
	reg = readl(socfpga_get_clkmgr_addr() + CLKMGR_GEN5_PERPLL_VCO);
	clock /= ((reg & CLKMGR_PERPLLGRP_VCO_DENOM_MASK) >>
		  CLKMGR_PERPLLGRP_VCO_DENOM_OFFSET) + 1;
	clock *= ((reg & CLKMGR_PERPLLGRP_VCO_NUMER_MASK) >>
		  CLKMGR_PERPLLGRP_VCO_NUMER_OFFSET) + 1;

	return clock;
}

unsigned long cm_get_mpu_clk_hz(void)
{
	u32 reg, clock;

	clock = cm_get_main_vco_clk_hz();

	/* get the MPU clock */
	reg = readl(socfpga_get_clkmgr_addr() + CLKMGR_GEN5_ALTR_MPUCLK);
	clock /= (reg + 1);
	reg = readl(socfpga_get_clkmgr_addr() + CLKMGR_GEN5_MAINPLL_MPUCLK);
	clock /= (reg + 1);
	return clock;
}

unsigned long cm_get_sdram_clk_hz(void)
{
	u32 reg, clock = 0;

	/* identify SDRAM PLL clock source */
	reg = readl(socfpga_get_clkmgr_addr() + CLKMGR_GEN5_SDRPLL_VCO);
	reg = (reg & CLKMGR_SDRPLLGRP_VCO_SSRC_MASK) >>
	      CLKMGR_SDRPLLGRP_VCO_SSRC_OFFSET;
	if (reg == CLKMGR_VCO_SSRC_EOSC1)
		clock = cm_get_osc_clk_hz(1);
	else if (reg == CLKMGR_VCO_SSRC_EOSC2)
		clock = cm_get_osc_clk_hz(2);
	else if (reg == CLKMGR_VCO_SSRC_F2S)
		clock = cm_get_f2s_sdr_ref_clk_hz();

	/* get the SDRAM VCO clock */
	reg = readl(socfpga_get_clkmgr_addr() + CLKMGR_GEN5_SDRPLL_VCO);
	clock /= ((reg & CLKMGR_SDRPLLGRP_VCO_DENOM_MASK) >>
		  CLKMGR_SDRPLLGRP_VCO_DENOM_OFFSET) + 1;
	clock *= ((reg & CLKMGR_SDRPLLGRP_VCO_NUMER_MASK) >>
		  CLKMGR_SDRPLLGRP_VCO_NUMER_OFFSET) + 1;

	/* get the SDRAM (DDR_DQS) clock */
	reg = readl(socfpga_get_clkmgr_addr() + CLKMGR_GEN5_SDRPLL_DDRDQSCLK);
	reg = (reg & CLKMGR_SDRPLLGRP_DDRDQSCLK_CNT_MASK) >>
	      CLKMGR_SDRPLLGRP_DDRDQSCLK_CNT_OFFSET;
	clock /= (reg + 1);

	return clock;
}

unsigned int cm_get_l4_sp_clk_hz(void)
{
	u32 reg, clock = 0;

	/* identify the source of L4 SP clock */
	reg = readl(socfpga_get_clkmgr_addr() + CLKMGR_GEN5_MAINPLL_L4SRC);
	reg = (reg & CLKMGR_MAINPLLGRP_L4SRC_L4SP) >>
	      CLKMGR_MAINPLLGRP_L4SRC_L4SP_OFFSET;

	if (reg == CLKMGR_L4_SP_CLK_SRC_MAINPLL) {
		clock = cm_get_main_vco_clk_hz();

		/* get the clock prior L4 SP divider (main clk) */
		reg = readl(socfpga_get_clkmgr_addr() +
			    CLKMGR_GEN5_ALTR_MAINCLK);
		clock /= (reg + 1);
		reg = readl(socfpga_get_clkmgr_addr() +
			    CLKMGR_GEN5_MAINPLL_MAINCLK);
		clock /= (reg + 1);
	} else if (reg == CLKMGR_L4_SP_CLK_SRC_PERPLL) {
		clock = cm_get_per_vco_clk_hz();

		/* get the clock prior L4 SP divider (periph_base_clk) */
		reg = readl(socfpga_get_clkmgr_addr() +
			    CLKMGR_GEN5_PERPLL_PERBASECLK);
		clock /= (reg + 1);
	}

	/* get the L4 SP clock which supplied to UART */
	reg = readl(socfpga_get_clkmgr_addr() + CLKMGR_GEN5_MAINPLL_MAINDIV);
	reg = (reg & CLKMGR_MAINPLLGRP_MAINDIV_L4SPCLK_MASK) >>
	      CLKMGR_MAINPLLGRP_MAINDIV_L4SPCLK_OFFSET;
	clock = clock / (1 << reg);

	return clock;
}

unsigned int cm_get_mmc_controller_clk_hz(void)
{
	u32 reg, clock = 0;

	/* identify the source of MMC clock */
	reg = readl(socfpga_get_clkmgr_addr() + CLKMGR_GEN5_PERPLL_SRC);
	reg = (reg & CLKMGR_PERPLLGRP_SRC_SDMMC_MASK) >>
	      CLKMGR_PERPLLGRP_SRC_SDMMC_OFFSET;

	if (reg == CLKMGR_SDMMC_CLK_SRC_F2S) {
		clock = cm_get_f2s_per_ref_clk_hz();
	} else if (reg == CLKMGR_SDMMC_CLK_SRC_MAIN) {
		clock = cm_get_main_vco_clk_hz();

		/* get the SDMMC clock */
		reg = readl(socfpga_get_clkmgr_addr() +
			    CLKMGR_GEN5_MAINPLL_MAINNANDSDMMCCLK);
		clock /= (reg + 1);
	} else if (reg == CLKMGR_SDMMC_CLK_SRC_PER) {
		clock = cm_get_per_vco_clk_hz();

		/* get the SDMMC clock */
		reg = readl(socfpga_get_clkmgr_addr() +
			    CLKMGR_GEN5_PERPLL_PERNANDSDMMCCLK);
		clock /= (reg + 1);
	}

	/* further divide by 4 as we have fixed divider at wrapper */
	clock /= 4;
	return clock;
}

unsigned int cm_get_qspi_controller_clk_hz(void)
{
	u32 reg, clock = 0;

	/* identify the source of QSPI clock */
	reg = readl(socfpga_get_clkmgr_addr() + CLKMGR_GEN5_PERPLL_SRC);
	reg = (reg & CLKMGR_PERPLLGRP_SRC_QSPI_MASK) >>
	      CLKMGR_PERPLLGRP_SRC_QSPI_OFFSET;

	if (reg == CLKMGR_QSPI_CLK_SRC_F2S) {
		clock = cm_get_f2s_per_ref_clk_hz();
	} else if (reg == CLKMGR_QSPI_CLK_SRC_MAIN) {
		clock = cm_get_main_vco_clk_hz();

		/* get the qspi clock */
		reg = readl(socfpga_get_clkmgr_addr() +
			    CLKMGR_GEN5_MAINPLL_MAINQSPICLK);
		clock /= (reg + 1);
	} else if (reg == CLKMGR_QSPI_CLK_SRC_PER) {
		clock = cm_get_per_vco_clk_hz();

		/* get the qspi clock */
		reg = readl(socfpga_get_clkmgr_addr() +
			    CLKMGR_GEN5_PERPLL_PERQSPICLK);
		clock /= (reg + 1);
	}

	return clock;
}

unsigned int cm_get_spi_controller_clk_hz(void)
{
	u32 reg, clock = 0;

	clock = cm_get_per_vco_clk_hz();

	/* get the clock prior L4 SP divider (periph_base_clk) */
	reg = readl(socfpga_get_clkmgr_addr() + CLKMGR_GEN5_PERPLL_PERBASECLK);
	clock /= (reg + 1);

	return clock;
}

/* Override weak dw_spi_get_clk implementation in designware_spi.c driver */
int dw_spi_get_clk(struct udevice *bus, ulong *rate)
{
	*rate = cm_get_spi_controller_clk_hz();

	return 0;
}

void cm_print_clock_quick_summary(void)
{
	printf("MPU       %10ld kHz\n", cm_get_mpu_clk_hz() / 1000);
	printf("DDR       %10ld kHz\n", cm_get_sdram_clk_hz() / 1000);
	printf("EOSC1       %8d kHz\n", cm_get_osc_clk_hz(1) / 1000);
	printf("EOSC2       %8d kHz\n", cm_get_osc_clk_hz(2) / 1000);
	printf("F2S_SDR_REF %8d kHz\n", cm_get_f2s_sdr_ref_clk_hz() / 1000);
	printf("F2S_PER_REF %8d kHz\n", cm_get_f2s_per_ref_clk_hz() / 1000);
	printf("MMC         %8d kHz\n", cm_get_mmc_controller_clk_hz() / 1000);
	printf("QSPI        %8d kHz\n", cm_get_qspi_controller_clk_hz() / 1000);
	printf("UART        %8d kHz\n", cm_get_l4_sp_clk_hz() / 1000);
	printf("SPI         %8d kHz\n", cm_get_spi_controller_clk_hz() / 1000);
}
