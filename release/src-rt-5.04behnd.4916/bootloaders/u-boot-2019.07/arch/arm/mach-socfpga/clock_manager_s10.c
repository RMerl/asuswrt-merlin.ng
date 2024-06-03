// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2016-2018 Intel Corporation <www.intel.com>
 *
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/clock_manager.h>
#include <asm/arch/handoff_s10.h>
#include <asm/arch/system_manager.h>

DECLARE_GLOBAL_DATA_PTR;

static const struct socfpga_clock_manager *clock_manager_base =
	(struct socfpga_clock_manager *)SOCFPGA_CLKMGR_ADDRESS;
static const struct socfpga_system_manager *sysmgr_regs =
		(struct socfpga_system_manager *)SOCFPGA_SYSMGR_ADDRESS;

/*
 * function to write the bypass register which requires a poll of the
 * busy bit
 */
static void cm_write_bypass_mainpll(u32 val)
{
	writel(val, &clock_manager_base->main_pll.bypass);
	cm_wait_for_fsm();
}

static void cm_write_bypass_perpll(u32 val)
{
	writel(val, &clock_manager_base->per_pll.bypass);
	cm_wait_for_fsm();
}

/* function to write the ctrl register which requires a poll of the busy bit */
static void cm_write_ctrl(u32 val)
{
	writel(val, &clock_manager_base->ctrl);
	cm_wait_for_fsm();
}

/*
 * Setup clocks while making no assumptions about previous state of the clocks.
 */
void cm_basic_init(const struct cm_config * const cfg)
{
	u32 mdiv, refclkdiv, mscnt, hscnt, vcocalib;

	if (cfg == 0)
		return;

	/* Put all plls in bypass */
	cm_write_bypass_mainpll(CLKMGR_BYPASS_MAINPLL_ALL);
	cm_write_bypass_perpll(CLKMGR_BYPASS_PERPLL_ALL);

	/* setup main PLL dividers where calculate the vcocalib value */
	mdiv = (cfg->main_pll_fdbck >> CLKMGR_FDBCK_MDIV_OFFSET) &
		CLKMGR_FDBCK_MDIV_MASK;
	refclkdiv = (cfg->main_pll_pllglob >> CLKMGR_PLLGLOB_REFCLKDIV_OFFSET) &
		     CLKMGR_PLLGLOB_REFCLKDIV_MASK;
	mscnt = CLKMGR_MSCNT_CONST / (CLKMGR_MDIV_CONST + mdiv) / refclkdiv;
	hscnt = (mdiv + CLKMGR_MDIV_CONST) * mscnt / refclkdiv -
		CLKMGR_HSCNT_CONST;
	vcocalib = (hscnt & CLKMGR_VCOCALIB_HSCNT_MASK) |
		   ((mscnt & CLKMGR_VCOCALIB_MSCNT_MASK) <<
		   CLKMGR_VCOCALIB_MSCNT_OFFSET);

	writel((cfg->main_pll_pllglob & ~CLKMGR_PLLGLOB_PD_MASK &
		~CLKMGR_PLLGLOB_RST_MASK),
		&clock_manager_base->main_pll.pllglob);
	writel(cfg->main_pll_fdbck, &clock_manager_base->main_pll.fdbck);
	writel(vcocalib, &clock_manager_base->main_pll.vcocalib);
	writel(cfg->main_pll_pllc0, &clock_manager_base->main_pll.pllc0);
	writel(cfg->main_pll_pllc1, &clock_manager_base->main_pll.pllc1);
	writel(cfg->main_pll_nocdiv, &clock_manager_base->main_pll.nocdiv);

	/* setup peripheral PLL dividers */
	/* calculate the vcocalib value */
	mdiv = (cfg->per_pll_fdbck >> CLKMGR_FDBCK_MDIV_OFFSET) &
		CLKMGR_FDBCK_MDIV_MASK;
	refclkdiv = (cfg->per_pll_pllglob >> CLKMGR_PLLGLOB_REFCLKDIV_OFFSET) &
		     CLKMGR_PLLGLOB_REFCLKDIV_MASK;
	mscnt = CLKMGR_MSCNT_CONST / (CLKMGR_MDIV_CONST + mdiv) / refclkdiv;
	hscnt = (mdiv + CLKMGR_MDIV_CONST) * mscnt / refclkdiv -
		CLKMGR_HSCNT_CONST;
	vcocalib = (hscnt & CLKMGR_VCOCALIB_HSCNT_MASK) |
		   ((mscnt & CLKMGR_VCOCALIB_MSCNT_MASK) <<
		   CLKMGR_VCOCALIB_MSCNT_OFFSET);

	writel((cfg->per_pll_pllglob & ~CLKMGR_PLLGLOB_PD_MASK &
		~CLKMGR_PLLGLOB_RST_MASK),
		&clock_manager_base->per_pll.pllglob);
	writel(cfg->per_pll_fdbck, &clock_manager_base->per_pll.fdbck);
	writel(vcocalib, &clock_manager_base->per_pll.vcocalib);
	writel(cfg->per_pll_pllc0, &clock_manager_base->per_pll.pllc0);
	writel(cfg->per_pll_pllc1, &clock_manager_base->per_pll.pllc1);
	writel(cfg->per_pll_emacctl, &clock_manager_base->per_pll.emacctl);
	writel(cfg->per_pll_gpiodiv, &clock_manager_base->per_pll.gpiodiv);

	/* Take both PLL out of reset and power up */
	setbits_le32(&clock_manager_base->main_pll.pllglob,
		     CLKMGR_PLLGLOB_PD_MASK | CLKMGR_PLLGLOB_RST_MASK);
	setbits_le32(&clock_manager_base->per_pll.pllglob,
		     CLKMGR_PLLGLOB_PD_MASK | CLKMGR_PLLGLOB_RST_MASK);

#define LOCKED_MASK \
	(CLKMGR_STAT_MAINPLL_LOCKED | \
	CLKMGR_STAT_PERPLL_LOCKED)

	cm_wait_for_lock(LOCKED_MASK);

	/*
	 * Dividers for C2 to C9 only init after PLLs are lock. As dividers
	 * only take effect upon value change, we shall set a maximum value as
	 * default value.
	 */
	writel(0xff, &clock_manager_base->main_pll.mpuclk);
	writel(0xff, &clock_manager_base->main_pll.nocclk);
	writel(0xff, &clock_manager_base->main_pll.cntr2clk);
	writel(0xff, &clock_manager_base->main_pll.cntr3clk);
	writel(0xff, &clock_manager_base->main_pll.cntr4clk);
	writel(0xff, &clock_manager_base->main_pll.cntr5clk);
	writel(0xff, &clock_manager_base->main_pll.cntr6clk);
	writel(0xff, &clock_manager_base->main_pll.cntr7clk);
	writel(0xff, &clock_manager_base->main_pll.cntr8clk);
	writel(0xff, &clock_manager_base->main_pll.cntr9clk);
	writel(0xff, &clock_manager_base->per_pll.cntr2clk);
	writel(0xff, &clock_manager_base->per_pll.cntr3clk);
	writel(0xff, &clock_manager_base->per_pll.cntr4clk);
	writel(0xff, &clock_manager_base->per_pll.cntr5clk);
	writel(0xff, &clock_manager_base->per_pll.cntr6clk);
	writel(0xff, &clock_manager_base->per_pll.cntr7clk);
	writel(0xff, &clock_manager_base->per_pll.cntr8clk);
	writel(0xff, &clock_manager_base->per_pll.cntr9clk);

	writel(cfg->main_pll_mpuclk, &clock_manager_base->main_pll.mpuclk);
	writel(cfg->main_pll_nocclk, &clock_manager_base->main_pll.nocclk);
	writel(cfg->main_pll_cntr2clk, &clock_manager_base->main_pll.cntr2clk);
	writel(cfg->main_pll_cntr3clk, &clock_manager_base->main_pll.cntr3clk);
	writel(cfg->main_pll_cntr4clk, &clock_manager_base->main_pll.cntr4clk);
	writel(cfg->main_pll_cntr5clk, &clock_manager_base->main_pll.cntr5clk);
	writel(cfg->main_pll_cntr6clk, &clock_manager_base->main_pll.cntr6clk);
	writel(cfg->main_pll_cntr7clk, &clock_manager_base->main_pll.cntr7clk);
	writel(cfg->main_pll_cntr8clk, &clock_manager_base->main_pll.cntr8clk);
	writel(cfg->main_pll_cntr9clk, &clock_manager_base->main_pll.cntr9clk);
	writel(cfg->per_pll_cntr2clk, &clock_manager_base->per_pll.cntr2clk);
	writel(cfg->per_pll_cntr3clk, &clock_manager_base->per_pll.cntr3clk);
	writel(cfg->per_pll_cntr4clk, &clock_manager_base->per_pll.cntr4clk);
	writel(cfg->per_pll_cntr5clk, &clock_manager_base->per_pll.cntr5clk);
	writel(cfg->per_pll_cntr6clk, &clock_manager_base->per_pll.cntr6clk);
	writel(cfg->per_pll_cntr7clk, &clock_manager_base->per_pll.cntr7clk);
	writel(cfg->per_pll_cntr8clk, &clock_manager_base->per_pll.cntr8clk);
	writel(cfg->per_pll_cntr9clk, &clock_manager_base->per_pll.cntr9clk);

	/* Take all PLLs out of bypass */
	cm_write_bypass_mainpll(0);
	cm_write_bypass_perpll(0);

	/* clear safe mode / out of boot mode */
	cm_write_ctrl(readl(&clock_manager_base->ctrl)
			& ~(CLKMGR_CTRL_SAFEMODE));

	/* Now ungate non-hw-managed clocks */
	writel(~0, &clock_manager_base->main_pll.en);
	writel(~0, &clock_manager_base->per_pll.en);

	/* Clear the loss of lock bits (write 1 to clear) */
	writel(CLKMGR_INTER_PERPLLLOST_MASK | CLKMGR_INTER_MAINPLLLOST_MASK,
	       &clock_manager_base->intrclr);
}

static unsigned long cm_get_main_vco_clk_hz(void)
{
	 unsigned long fref, refdiv, mdiv, reg, vco;

	reg = readl(&clock_manager_base->main_pll.pllglob);

	fref = (reg >> CLKMGR_PLLGLOB_VCO_PSRC_OFFSET) &
		CLKMGR_PLLGLOB_VCO_PSRC_MASK;
	switch (fref) {
	case CLKMGR_VCO_PSRC_EOSC1:
		fref = cm_get_osc_clk_hz();
		break;
	case CLKMGR_VCO_PSRC_INTOSC:
		fref = cm_get_intosc_clk_hz();
		break;
	case CLKMGR_VCO_PSRC_F2S:
		fref = cm_get_fpga_clk_hz();
		break;
	}

	refdiv = (reg >> CLKMGR_PLLGLOB_REFCLKDIV_OFFSET) &
		  CLKMGR_PLLGLOB_REFCLKDIV_MASK;

	reg = readl(&clock_manager_base->main_pll.fdbck);
	mdiv = (reg >> CLKMGR_FDBCK_MDIV_OFFSET) & CLKMGR_FDBCK_MDIV_MASK;

	vco = fref / refdiv;
	vco = vco * (CLKMGR_MDIV_CONST + mdiv);
	return vco;
}

static unsigned long cm_get_per_vco_clk_hz(void)
{
	unsigned long fref, refdiv, mdiv, reg, vco;

	reg = readl(&clock_manager_base->per_pll.pllglob);

	fref = (reg >> CLKMGR_PLLGLOB_VCO_PSRC_OFFSET) &
		CLKMGR_PLLGLOB_VCO_PSRC_MASK;
	switch (fref) {
	case CLKMGR_VCO_PSRC_EOSC1:
		fref = cm_get_osc_clk_hz();
		break;
	case CLKMGR_VCO_PSRC_INTOSC:
		fref = cm_get_intosc_clk_hz();
		break;
	case CLKMGR_VCO_PSRC_F2S:
		fref = cm_get_fpga_clk_hz();
		break;
	}

	refdiv = (reg >> CLKMGR_PLLGLOB_REFCLKDIV_OFFSET) &
		  CLKMGR_PLLGLOB_REFCLKDIV_MASK;

	reg = readl(&clock_manager_base->per_pll.fdbck);
	mdiv = (reg >> CLKMGR_FDBCK_MDIV_OFFSET) & CLKMGR_FDBCK_MDIV_MASK;

	vco = fref / refdiv;
	vco = vco * (CLKMGR_MDIV_CONST + mdiv);
	return vco;
}

unsigned long cm_get_mpu_clk_hz(void)
{
	unsigned long clock = readl(&clock_manager_base->main_pll.mpuclk);

	clock = (clock >> CLKMGR_CLKSRC_OFFSET) & CLKMGR_CLKSRC_MASK;

	switch (clock) {
	case CLKMGR_CLKSRC_MAIN:
		clock = cm_get_main_vco_clk_hz();
		clock /= (readl(&clock_manager_base->main_pll.pllc0) &
			  CLKMGR_PLLC0_DIV_MASK);
		break;

	case CLKMGR_CLKSRC_PER:
		clock = cm_get_per_vco_clk_hz();
		clock /= (readl(&clock_manager_base->per_pll.pllc0) &
			  CLKMGR_CLKCNT_MSK);
		break;

	case CLKMGR_CLKSRC_OSC1:
		clock = cm_get_osc_clk_hz();
		break;

	case CLKMGR_CLKSRC_INTOSC:
		clock = cm_get_intosc_clk_hz();
		break;

	case CLKMGR_CLKSRC_FPGA:
		clock = cm_get_fpga_clk_hz();
		break;
	}

	clock /= 1 + (readl(&clock_manager_base->main_pll.mpuclk) &
		CLKMGR_CLKCNT_MSK);
	return clock;
}

unsigned int cm_get_l3_main_clk_hz(void)
{
	u32 clock = readl(&clock_manager_base->main_pll.nocclk);

	clock = (clock >> CLKMGR_CLKSRC_OFFSET) & CLKMGR_CLKSRC_MASK;

	switch (clock) {
	case CLKMGR_CLKSRC_MAIN:
		clock = cm_get_main_vco_clk_hz();
		clock /= (readl(&clock_manager_base->main_pll.pllc1) &
			  CLKMGR_PLLC0_DIV_MASK);
		break;

	case CLKMGR_CLKSRC_PER:
		clock = cm_get_per_vco_clk_hz();
		clock /= (readl(&clock_manager_base->per_pll.pllc1) &
			  CLKMGR_CLKCNT_MSK);
		break;

	case CLKMGR_CLKSRC_OSC1:
		clock = cm_get_osc_clk_hz();
		break;

	case CLKMGR_CLKSRC_INTOSC:
		clock = cm_get_intosc_clk_hz();
		break;

	case CLKMGR_CLKSRC_FPGA:
		clock = cm_get_fpga_clk_hz();
		break;
	}

	clock /= 1 + (readl(&clock_manager_base->main_pll.nocclk) &
		CLKMGR_CLKCNT_MSK);
	return clock;
}

unsigned int cm_get_mmc_controller_clk_hz(void)
{
	u32 clock = readl(&clock_manager_base->per_pll.cntr6clk);

	clock = (clock >> CLKMGR_CLKSRC_OFFSET) & CLKMGR_CLKSRC_MASK;

	switch (clock) {
	case CLKMGR_CLKSRC_MAIN:
		clock = cm_get_l3_main_clk_hz();
		clock /= 1 + (readl(&clock_manager_base->main_pll.cntr6clk) &
			CLKMGR_CLKCNT_MSK);
		break;

	case CLKMGR_CLKSRC_PER:
		clock = cm_get_l3_main_clk_hz();
		clock /= 1 + (readl(&clock_manager_base->per_pll.cntr6clk) &
			CLKMGR_CLKCNT_MSK);
		break;

	case CLKMGR_CLKSRC_OSC1:
		clock = cm_get_osc_clk_hz();
		break;

	case CLKMGR_CLKSRC_INTOSC:
		clock = cm_get_intosc_clk_hz();
		break;

	case CLKMGR_CLKSRC_FPGA:
		clock = cm_get_fpga_clk_hz();
		break;
	}
	return clock / 4;
}

unsigned int cm_get_l4_sp_clk_hz(void)
{
	u32 clock = cm_get_l3_main_clk_hz();

	clock /= (1 << ((readl(&clock_manager_base->main_pll.nocdiv) >>
		  CLKMGR_NOCDIV_L4SPCLK_OFFSET) & CLKMGR_CLKCNT_MSK));
	return clock;
}

unsigned int cm_get_qspi_controller_clk_hz(void)
{
	return readl(&sysmgr_regs->boot_scratch_cold0);
}

unsigned int cm_get_spi_controller_clk_hz(void)
{
	u32 clock = cm_get_l3_main_clk_hz();

	clock /= (1 << ((readl(&clock_manager_base->main_pll.nocdiv) >>
		  CLKMGR_NOCDIV_L4MAIN_OFFSET) & CLKMGR_CLKCNT_MSK));
	return clock;
}

unsigned int cm_get_l4_sys_free_clk_hz(void)
{
	return cm_get_l3_main_clk_hz() / 4;
}

void cm_print_clock_quick_summary(void)
{
	printf("MPU         %d kHz\n", (u32)(cm_get_mpu_clk_hz() / 1000));
	printf("L3 main     %d kHz\n", cm_get_l3_main_clk_hz() / 1000);
	printf("Main VCO    %d kHz\n", (u32)(cm_get_main_vco_clk_hz() / 1000));
	printf("Per VCO     %d kHz\n", (u32)(cm_get_per_vco_clk_hz() / 1000));
	printf("EOSC1       %d kHz\n", cm_get_osc_clk_hz() / 1000);
	printf("HPS MMC     %d kHz\n", cm_get_mmc_controller_clk_hz() / 1000);
	printf("UART        %d kHz\n", cm_get_l4_sp_clk_hz() / 1000);
}
