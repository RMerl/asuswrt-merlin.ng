// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright (C) 2013-2017 Altera Corporation <www.altera.com>
 */

#include <common.h>
#include <asm/io.h>
#include <dm.h>
#include <asm/arch/clock_manager.h>
#include <wait_bit.h>

static const struct socfpga_clock_manager *clock_manager_base =
	(struct socfpga_clock_manager *)SOCFPGA_CLKMGR_ADDRESS;

/*
 * function to write the bypass register which requires a poll of the
 * busy bit
 */
static void cm_write_bypass(u32 val)
{
	writel(val, &clock_manager_base->bypass);
	cm_wait_for_fsm();
}

/* function to write the ctrl register which requires a poll of the busy bit */
static void cm_write_ctrl(u32 val)
{
	writel(val, &clock_manager_base->ctrl);
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
		readl(&clock_manager_base->per_pll.en),
		&clock_manager_base->per_pll.en);

	/* DO NOT GATE OFF DEBUG CLOCKS & BRIDGE CLOCKS */
	writel(CLKMGR_MAINPLLGRP_EN_DBGTIMERCLK_MASK |
		CLKMGR_MAINPLLGRP_EN_DBGTRACECLK_MASK |
		CLKMGR_MAINPLLGRP_EN_DBGCLK_MASK |
		CLKMGR_MAINPLLGRP_EN_DBGATCLK_MASK |
		CLKMGR_MAINPLLGRP_EN_S2FUSER0CLK_MASK |
		CLKMGR_MAINPLLGRP_EN_L4MPCLK_MASK,
		&clock_manager_base->main_pll.en);

	writel(0, &clock_manager_base->sdr_pll.en);

	/* now we can gate off the rest of the peripheral clocks */
	writel(0, &clock_manager_base->per_pll.en);

	/* Put all plls in bypass */
	cm_write_bypass(CLKMGR_BYPASS_PERPLL | CLKMGR_BYPASS_SDRPLL |
			CLKMGR_BYPASS_MAINPLL);

	/* Put all plls VCO registers back to reset value. */
	writel(CLKMGR_MAINPLLGRP_VCO_RESET_VALUE &
	       ~CLKMGR_MAINPLLGRP_VCO_REGEXTSEL_MASK,
	       &clock_manager_base->main_pll.vco);
	writel(CLKMGR_PERPLLGRP_VCO_RESET_VALUE &
	       ~CLKMGR_PERPLLGRP_VCO_REGEXTSEL_MASK,
	       &clock_manager_base->per_pll.vco);
	writel(CLKMGR_SDRPLLGRP_VCO_RESET_VALUE &
	       ~CLKMGR_SDRPLLGRP_VCO_REGEXTSEL_MASK,
	       &clock_manager_base->sdr_pll.vco);

	/*
	 * The clocks to the flash devices and the L4_MAIN clocks can
	 * glitch when coming out of safe mode if their source values
	 * are different from their reset value.  So the trick it to
	 * put them back to their reset state, and change input
	 * after exiting safe mode but before ungating the clocks.
	 */
	writel(CLKMGR_PERPLLGRP_SRC_RESET_VALUE,
	       &clock_manager_base->per_pll.src);
	writel(CLKMGR_MAINPLLGRP_L4SRC_RESET_VALUE,
	       &clock_manager_base->main_pll.l4src);

	/* read back for the required 5 us delay. */
	readl(&clock_manager_base->main_pll.vco);
	readl(&clock_manager_base->per_pll.vco);
	readl(&clock_manager_base->sdr_pll.vco);


	/*
	 * We made sure bgpwr down was assert for 5 us. Now deassert BG PWR DN
	 * with numerator and denominator.
	 */
	writel(cfg->main_vco_base, &clock_manager_base->main_pll.vco);
	writel(cfg->peri_vco_base, &clock_manager_base->per_pll.vco);
	writel(cfg->sdram_vco_base, &clock_manager_base->sdr_pll.vco);

	/*
	 * Time starts here. Must wait 7 us from
	 * BGPWRDN_SET(0) to VCO_ENABLE_SET(1).
	 */
	end = timer_get_us() + 7;

	/* main mpu */
	writel(cfg->mpuclk, &clock_manager_base->main_pll.mpuclk);

	/* altera group mpuclk */
	writel(cfg->altera_grp_mpuclk, &clock_manager_base->altera.mpuclk);

	/* main main clock */
	writel(cfg->mainclk, &clock_manager_base->main_pll.mainclk);

	/* main for dbg */
	writel(cfg->dbgatclk, &clock_manager_base->main_pll.dbgatclk);

	/* main for cfgs2fuser0clk */
	writel(cfg->cfg2fuser0clk,
	       &clock_manager_base->main_pll.cfgs2fuser0clk);

	/* Peri emac0 50 MHz default to RMII */
	writel(cfg->emac0clk, &clock_manager_base->per_pll.emac0clk);

	/* Peri emac1 50 MHz default to RMII */
	writel(cfg->emac1clk, &clock_manager_base->per_pll.emac1clk);

	/* Peri QSPI */
	writel(cfg->mainqspiclk, &clock_manager_base->main_pll.mainqspiclk);

	writel(cfg->perqspiclk, &clock_manager_base->per_pll.perqspiclk);

	/* Peri pernandsdmmcclk */
	writel(cfg->mainnandsdmmcclk,
	       &clock_manager_base->main_pll.mainnandsdmmcclk);

	writel(cfg->pernandsdmmcclk,
	       &clock_manager_base->per_pll.pernandsdmmcclk);

	/* Peri perbaseclk */
	writel(cfg->perbaseclk, &clock_manager_base->per_pll.perbaseclk);

	/* Peri s2fuser1clk */
	writel(cfg->s2fuser1clk, &clock_manager_base->per_pll.s2fuser1clk);

	/* 7 us must have elapsed before we can enable the VCO */
	while (timer_get_us() < end)
		;

	/* Enable vco */
	/* main pll vco */
	writel(cfg->main_vco_base | CLKMGR_MAINPLLGRP_VCO_EN,
	       &clock_manager_base->main_pll.vco);

	/* periferal pll */
	writel(cfg->peri_vco_base | CLKMGR_MAINPLLGRP_VCO_EN,
	       &clock_manager_base->per_pll.vco);

	/* sdram pll vco */
	writel(cfg->sdram_vco_base | CLKMGR_MAINPLLGRP_VCO_EN,
	       &clock_manager_base->sdr_pll.vco);

	/* L3 MP and L3 SP */
	writel(cfg->maindiv, &clock_manager_base->main_pll.maindiv);

	writel(cfg->dbgdiv, &clock_manager_base->main_pll.dbgdiv);

	writel(cfg->tracediv, &clock_manager_base->main_pll.tracediv);

	/* L4 MP, L4 SP, can0, and can1 */
	writel(cfg->perdiv, &clock_manager_base->per_pll.div);

	writel(cfg->gpiodiv, &clock_manager_base->per_pll.gpiodiv);

	cm_wait_for_lock(LOCKED_MASK);

	/* write the sdram clock counters before toggling outreset all */
	writel(cfg->ddrdqsclk & CLKMGR_SDRPLLGRP_DDRDQSCLK_CNT_MASK,
	       &clock_manager_base->sdr_pll.ddrdqsclk);

	writel(cfg->ddr2xdqsclk & CLKMGR_SDRPLLGRP_DDR2XDQSCLK_CNT_MASK,
	       &clock_manager_base->sdr_pll.ddr2xdqsclk);

	writel(cfg->ddrdqclk & CLKMGR_SDRPLLGRP_DDRDQCLK_CNT_MASK,
	       &clock_manager_base->sdr_pll.ddrdqclk);

	writel(cfg->s2fuser2clk & CLKMGR_SDRPLLGRP_S2FUSER2CLK_CNT_MASK,
	       &clock_manager_base->sdr_pll.s2fuser2clk);

	/*
	 * after locking, but before taking out of bypass
	 * assert/deassert outresetall
	 */
	u32 mainvco = readl(&clock_manager_base->main_pll.vco);

	/* assert main outresetall */
	writel(mainvco | CLKMGR_MAINPLLGRP_VCO_OUTRESETALL_MASK,
	       &clock_manager_base->main_pll.vco);

	u32 periphvco = readl(&clock_manager_base->per_pll.vco);

	/* assert pheriph outresetall */
	writel(periphvco | CLKMGR_PERPLLGRP_VCO_OUTRESETALL_MASK,
	       &clock_manager_base->per_pll.vco);

	/* assert sdram outresetall */
	writel(cfg->sdram_vco_base | CLKMGR_MAINPLLGRP_VCO_EN|
		CLKMGR_SDRPLLGRP_VCO_OUTRESETALL,
		&clock_manager_base->sdr_pll.vco);

	/* deassert main outresetall */
	writel(mainvco & ~CLKMGR_MAINPLLGRP_VCO_OUTRESETALL_MASK,
	       &clock_manager_base->main_pll.vco);

	/* deassert pheriph outresetall */
	writel(periphvco & ~CLKMGR_PERPLLGRP_VCO_OUTRESETALL_MASK,
	       &clock_manager_base->per_pll.vco);

	/* deassert sdram outresetall */
	writel(cfg->sdram_vco_base | CLKMGR_MAINPLLGRP_VCO_EN,
	       &clock_manager_base->sdr_pll.vco);

	/*
	 * now that we've toggled outreset all, all the clocks
	 * are aligned nicely; so we can change any phase.
	 */
	ret = cm_write_with_phase(cfg->ddrdqsclk,
				  &clock_manager_base->sdr_pll.ddrdqsclk,
				  CLKMGR_SDRPLLGRP_DDRDQSCLK_PHASE_MASK);
	if (ret)
		return ret;

	/* SDRAM DDR2XDQSCLK */
	ret = cm_write_with_phase(cfg->ddr2xdqsclk,
				  &clock_manager_base->sdr_pll.ddr2xdqsclk,
				  CLKMGR_SDRPLLGRP_DDR2XDQSCLK_PHASE_MASK);
	if (ret)
		return ret;

	ret = cm_write_with_phase(cfg->ddrdqclk,
				  &clock_manager_base->sdr_pll.ddrdqclk,
				  CLKMGR_SDRPLLGRP_DDRDQCLK_PHASE_MASK);
	if (ret)
		return ret;

	ret = cm_write_with_phase(cfg->s2fuser2clk,
				  &clock_manager_base->sdr_pll.s2fuser2clk,
				  CLKMGR_SDRPLLGRP_S2FUSER2CLK_PHASE_MASK);
	if (ret)
		return ret;

	/* Take all three PLLs out of bypass when safe mode is cleared. */
	cm_write_bypass(0);

	/* clear safe mode */
	cm_write_ctrl(readl(&clock_manager_base->ctrl) | CLKMGR_CTRL_SAFEMODE);

	/*
	 * now that safe mode is clear with clocks gated
	 * it safe to change the source mux for the flashes the the L4_MAIN
	 */
	writel(cfg->persrc, &clock_manager_base->per_pll.src);
	writel(cfg->l4src, &clock_manager_base->main_pll.l4src);

	/* Now ungate non-hw-managed clocks */
	writel(~0, &clock_manager_base->main_pll.en);
	writel(~0, &clock_manager_base->per_pll.en);
	writel(~0, &clock_manager_base->sdr_pll.en);

	/* Clear the loss of lock bits (write 1 to clear) */
	writel(CLKMGR_INTER_SDRPLLLOST_MASK | CLKMGR_INTER_PERPLLLOST_MASK |
	       CLKMGR_INTER_MAINPLLLOST_MASK,
	       &clock_manager_base->inter);

	return 0;
}

static unsigned int cm_get_main_vco_clk_hz(void)
{
	u32 reg, clock;

	/* get the main VCO clock */
	reg = readl(&clock_manager_base->main_pll.vco);
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
	reg = readl(&clock_manager_base->per_pll.vco);
	reg = (reg & CLKMGR_PERPLLGRP_VCO_SSRC_MASK) >>
	      CLKMGR_PERPLLGRP_VCO_SSRC_OFFSET;
	if (reg == CLKMGR_VCO_SSRC_EOSC1)
		clock = cm_get_osc_clk_hz(1);
	else if (reg == CLKMGR_VCO_SSRC_EOSC2)
		clock = cm_get_osc_clk_hz(2);
	else if (reg == CLKMGR_VCO_SSRC_F2S)
		clock = cm_get_f2s_per_ref_clk_hz();

	/* get the PER VCO clock */
	reg = readl(&clock_manager_base->per_pll.vco);
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
	reg = readl(&clock_manager_base->altera.mpuclk);
	clock /= (reg + 1);
	reg = readl(&clock_manager_base->main_pll.mpuclk);
	clock /= (reg + 1);
	return clock;
}

unsigned long cm_get_sdram_clk_hz(void)
{
	u32 reg, clock = 0;

	/* identify SDRAM PLL clock source */
	reg = readl(&clock_manager_base->sdr_pll.vco);
	reg = (reg & CLKMGR_SDRPLLGRP_VCO_SSRC_MASK) >>
	      CLKMGR_SDRPLLGRP_VCO_SSRC_OFFSET;
	if (reg == CLKMGR_VCO_SSRC_EOSC1)
		clock = cm_get_osc_clk_hz(1);
	else if (reg == CLKMGR_VCO_SSRC_EOSC2)
		clock = cm_get_osc_clk_hz(2);
	else if (reg == CLKMGR_VCO_SSRC_F2S)
		clock = cm_get_f2s_sdr_ref_clk_hz();

	/* get the SDRAM VCO clock */
	reg = readl(&clock_manager_base->sdr_pll.vco);
	clock /= ((reg & CLKMGR_SDRPLLGRP_VCO_DENOM_MASK) >>
		  CLKMGR_SDRPLLGRP_VCO_DENOM_OFFSET) + 1;
	clock *= ((reg & CLKMGR_SDRPLLGRP_VCO_NUMER_MASK) >>
		  CLKMGR_SDRPLLGRP_VCO_NUMER_OFFSET) + 1;

	/* get the SDRAM (DDR_DQS) clock */
	reg = readl(&clock_manager_base->sdr_pll.ddrdqsclk);
	reg = (reg & CLKMGR_SDRPLLGRP_DDRDQSCLK_CNT_MASK) >>
	      CLKMGR_SDRPLLGRP_DDRDQSCLK_CNT_OFFSET;
	clock /= (reg + 1);

	return clock;
}

unsigned int cm_get_l4_sp_clk_hz(void)
{
	u32 reg, clock = 0;

	/* identify the source of L4 SP clock */
	reg = readl(&clock_manager_base->main_pll.l4src);
	reg = (reg & CLKMGR_MAINPLLGRP_L4SRC_L4SP) >>
	      CLKMGR_MAINPLLGRP_L4SRC_L4SP_OFFSET;

	if (reg == CLKMGR_L4_SP_CLK_SRC_MAINPLL) {
		clock = cm_get_main_vco_clk_hz();

		/* get the clock prior L4 SP divider (main clk) */
		reg = readl(&clock_manager_base->altera.mainclk);
		clock /= (reg + 1);
		reg = readl(&clock_manager_base->main_pll.mainclk);
		clock /= (reg + 1);
	} else if (reg == CLKMGR_L4_SP_CLK_SRC_PERPLL) {
		clock = cm_get_per_vco_clk_hz();

		/* get the clock prior L4 SP divider (periph_base_clk) */
		reg = readl(&clock_manager_base->per_pll.perbaseclk);
		clock /= (reg + 1);
	}

	/* get the L4 SP clock which supplied to UART */
	reg = readl(&clock_manager_base->main_pll.maindiv);
	reg = (reg & CLKMGR_MAINPLLGRP_MAINDIV_L4SPCLK_MASK) >>
	      CLKMGR_MAINPLLGRP_MAINDIV_L4SPCLK_OFFSET;
	clock = clock / (1 << reg);

	return clock;
}

unsigned int cm_get_mmc_controller_clk_hz(void)
{
	u32 reg, clock = 0;

	/* identify the source of MMC clock */
	reg = readl(&clock_manager_base->per_pll.src);
	reg = (reg & CLKMGR_PERPLLGRP_SRC_SDMMC_MASK) >>
	      CLKMGR_PERPLLGRP_SRC_SDMMC_OFFSET;

	if (reg == CLKMGR_SDMMC_CLK_SRC_F2S) {
		clock = cm_get_f2s_per_ref_clk_hz();
	} else if (reg == CLKMGR_SDMMC_CLK_SRC_MAIN) {
		clock = cm_get_main_vco_clk_hz();

		/* get the SDMMC clock */
		reg = readl(&clock_manager_base->main_pll.mainnandsdmmcclk);
		clock /= (reg + 1);
	} else if (reg == CLKMGR_SDMMC_CLK_SRC_PER) {
		clock = cm_get_per_vco_clk_hz();

		/* get the SDMMC clock */
		reg = readl(&clock_manager_base->per_pll.pernandsdmmcclk);
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
	reg = readl(&clock_manager_base->per_pll.src);
	reg = (reg & CLKMGR_PERPLLGRP_SRC_QSPI_MASK) >>
	      CLKMGR_PERPLLGRP_SRC_QSPI_OFFSET;

	if (reg == CLKMGR_QSPI_CLK_SRC_F2S) {
		clock = cm_get_f2s_per_ref_clk_hz();
	} else if (reg == CLKMGR_QSPI_CLK_SRC_MAIN) {
		clock = cm_get_main_vco_clk_hz();

		/* get the qspi clock */
		reg = readl(&clock_manager_base->main_pll.mainqspiclk);
		clock /= (reg + 1);
	} else if (reg == CLKMGR_QSPI_CLK_SRC_PER) {
		clock = cm_get_per_vco_clk_hz();

		/* get the qspi clock */
		reg = readl(&clock_manager_base->per_pll.perqspiclk);
		clock /= (reg + 1);
	}

	return clock;
}

unsigned int cm_get_spi_controller_clk_hz(void)
{
	u32 reg, clock = 0;

	clock = cm_get_per_vco_clk_hz();

	/* get the clock prior L4 SP divider (periph_base_clk) */
	reg = readl(&clock_manager_base->per_pll.perbaseclk);
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
