// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2016-2023 Intel Corporation <www.intel.com>
 *
 */

#include <common.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/clock_manager.h>
#include <asm/arch/handoff_soc64.h>
#include <asm/arch/system_manager.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * function to write the bypass register which requires a poll of the
 * busy bit
 */
static void cm_write_bypass_mainpll(u32 val)
{
	writel(val, socfpga_get_clkmgr_addr() + CLKMGR_S10_MAINPLL_BYPASS);
	cm_wait_for_fsm();
}

static void cm_write_bypass_perpll(u32 val)
{
	writel(val, socfpga_get_clkmgr_addr() + CLKMGR_S10_PERPLL_BYPASS);
	cm_wait_for_fsm();
}

/* function to write the ctrl register which requires a poll of the busy bit */
static void cm_write_ctrl(u32 val)
{
	writel(val, socfpga_get_clkmgr_addr() + CLKMGR_S10_CTRL);
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
		socfpga_get_clkmgr_addr() + CLKMGR_S10_MAINPLL_PLLGLOB);
	writel(cfg->main_pll_fdbck,
	       socfpga_get_clkmgr_addr() + CLKMGR_S10_MAINPLL_FDBCK);
	writel(vcocalib,
	       socfpga_get_clkmgr_addr() + CLKMGR_S10_MAINPLL_VCOCALIB);
	writel(cfg->main_pll_pllc0,
	       socfpga_get_clkmgr_addr() + CLKMGR_S10_MAINPLL_PLLC0);
	writel(cfg->main_pll_pllc1,
	       socfpga_get_clkmgr_addr() + CLKMGR_S10_MAINPLL_PLLC1);
	writel(cfg->main_pll_nocdiv,
	       socfpga_get_clkmgr_addr() + CLKMGR_S10_MAINPLL_NOCDIV);

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
		socfpga_get_clkmgr_addr() + CLKMGR_S10_PERPLL_PLLGLOB);
	writel(cfg->per_pll_fdbck,
	       socfpga_get_clkmgr_addr() + CLKMGR_S10_PERPLL_FDBCK);
	writel(vcocalib,
	       socfpga_get_clkmgr_addr() + CLKMGR_S10_PERPLL_VCOCALIB);
	writel(cfg->per_pll_pllc0,
	       socfpga_get_clkmgr_addr() + CLKMGR_S10_PERPLL_PLLC0);
	writel(cfg->per_pll_pllc1,
	       socfpga_get_clkmgr_addr() + CLKMGR_S10_PERPLL_PLLC1);
	writel(cfg->per_pll_emacctl,
	       socfpga_get_clkmgr_addr() + CLKMGR_S10_PERPLL_EMACCTL);
	writel(cfg->per_pll_gpiodiv,
	       socfpga_get_clkmgr_addr() + CLKMGR_S10_PERPLL_GPIODIV);

	/* Take both PLL out of reset and power up */
	setbits_le32(socfpga_get_clkmgr_addr() + CLKMGR_S10_MAINPLL_PLLGLOB,
		     CLKMGR_PLLGLOB_PD_MASK | CLKMGR_PLLGLOB_RST_MASK);
	setbits_le32(socfpga_get_clkmgr_addr() + CLKMGR_S10_PERPLL_PLLGLOB,
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
	writel(0xff, socfpga_get_clkmgr_addr() + CLKMGR_S10_MAINPLL_MPUCLK);
	writel(0xff, socfpga_get_clkmgr_addr() + CLKMGR_S10_MAINPLL_NOCCLK);
	writel(0xff, socfpga_get_clkmgr_addr() + CLKMGR_S10_MAINPLL_CNTR2CLK);
	writel(0xff, socfpga_get_clkmgr_addr() + CLKMGR_S10_MAINPLL_CNTR3CLK);
	writel(0xff, socfpga_get_clkmgr_addr() + CLKMGR_S10_MAINPLL_CNTR4CLK);
	writel(0xff, socfpga_get_clkmgr_addr() + CLKMGR_S10_MAINPLL_CNTR5CLK);
	writel(0xff, socfpga_get_clkmgr_addr() + CLKMGR_S10_MAINPLL_CNTR6CLK);
	writel(0xff, socfpga_get_clkmgr_addr() + CLKMGR_S10_MAINPLL_CNTR7CLK);
	writel(0xff, socfpga_get_clkmgr_addr() + CLKMGR_S10_MAINPLL_CNTR8CLK);
	writel(0xff, socfpga_get_clkmgr_addr() + CLKMGR_S10_MAINPLL_CNTR9CLK);
	writel(0xff, socfpga_get_clkmgr_addr() + CLKMGR_S10_PERPLL_CNTR2CLK);
	writel(0xff, socfpga_get_clkmgr_addr() + CLKMGR_S10_PERPLL_CNTR3CLK);
	writel(0xff, socfpga_get_clkmgr_addr() + CLKMGR_S10_PERPLL_CNTR4CLK);
	writel(0xff, socfpga_get_clkmgr_addr() + CLKMGR_S10_PERPLL_CNTR5CLK);
	writel(0xff, socfpga_get_clkmgr_addr() + CLKMGR_S10_PERPLL_CNTR6CLK);
	writel(0xff, socfpga_get_clkmgr_addr() + CLKMGR_S10_PERPLL_CNTR7CLK);
	writel(0xff, socfpga_get_clkmgr_addr() + CLKMGR_S10_PERPLL_CNTR8CLK);
	writel(0xff, socfpga_get_clkmgr_addr() + CLKMGR_S10_PERPLL_CNTR9CLK);

	writel(cfg->main_pll_mpuclk,
	       socfpga_get_clkmgr_addr() + CLKMGR_S10_MAINPLL_MPUCLK);
	writel(cfg->main_pll_nocclk,
	       socfpga_get_clkmgr_addr() + CLKMGR_S10_MAINPLL_NOCCLK);
	writel(cfg->main_pll_cntr2clk,
	       socfpga_get_clkmgr_addr() + CLKMGR_S10_MAINPLL_CNTR2CLK);
	writel(cfg->main_pll_cntr3clk,
	       socfpga_get_clkmgr_addr() + CLKMGR_S10_MAINPLL_CNTR3CLK);
	writel(cfg->main_pll_cntr4clk,
	       socfpga_get_clkmgr_addr() + CLKMGR_S10_MAINPLL_CNTR4CLK);
	writel(cfg->main_pll_cntr5clk,
	       socfpga_get_clkmgr_addr() + CLKMGR_S10_MAINPLL_CNTR5CLK);
	writel(cfg->main_pll_cntr6clk,
	       socfpga_get_clkmgr_addr() + CLKMGR_S10_MAINPLL_CNTR6CLK);
	writel(cfg->main_pll_cntr7clk,
	       socfpga_get_clkmgr_addr() + CLKMGR_S10_MAINPLL_CNTR7CLK);
	writel(cfg->main_pll_cntr8clk,
	       socfpga_get_clkmgr_addr() + CLKMGR_S10_MAINPLL_CNTR8CLK);
	writel(cfg->main_pll_cntr9clk,
	       socfpga_get_clkmgr_addr() + CLKMGR_S10_MAINPLL_CNTR9CLK);
	writel(cfg->per_pll_cntr2clk,
	       socfpga_get_clkmgr_addr() + CLKMGR_S10_PERPLL_CNTR2CLK);
	writel(cfg->per_pll_cntr3clk,
	       socfpga_get_clkmgr_addr() + CLKMGR_S10_PERPLL_CNTR3CLK);
	writel(cfg->per_pll_cntr4clk,
	       socfpga_get_clkmgr_addr() + CLKMGR_S10_PERPLL_CNTR4CLK);
	writel(cfg->per_pll_cntr5clk,
	       socfpga_get_clkmgr_addr() + CLKMGR_S10_PERPLL_CNTR5CLK);
	writel(cfg->per_pll_cntr6clk,
	       socfpga_get_clkmgr_addr() + CLKMGR_S10_PERPLL_CNTR6CLK);
	writel(cfg->per_pll_cntr7clk,
	       socfpga_get_clkmgr_addr() + CLKMGR_S10_PERPLL_CNTR7CLK);
	writel(cfg->per_pll_cntr8clk,
	       socfpga_get_clkmgr_addr() + CLKMGR_S10_PERPLL_CNTR8CLK);
	writel(cfg->per_pll_cntr9clk,
	       socfpga_get_clkmgr_addr() + CLKMGR_S10_PERPLL_CNTR9CLK);

	/* Take all PLLs out of bypass */
	cm_write_bypass_mainpll(0);
	cm_write_bypass_perpll(0);

	/* clear safe mode / out of boot mode */
	cm_write_ctrl(readl(socfpga_get_clkmgr_addr() + CLKMGR_S10_CTRL) &
		      ~(CLKMGR_CTRL_SAFEMODE));

	/* Now ungate non-hw-managed clocks */
	writel(~0, socfpga_get_clkmgr_addr() + CLKMGR_S10_MAINPLL_EN);
	writel(~0, socfpga_get_clkmgr_addr() + CLKMGR_S10_PERPLL_EN);

	/* Clear the loss of lock bits (write 1 to clear) */
	writel(CLKMGR_INTER_PERPLLLOST_MASK |
		      CLKMGR_INTER_MAINPLLLOST_MASK,
		      socfpga_get_clkmgr_addr() + CLKMGR_S10_INTRCLR);
}

static unsigned long cm_get_main_vco_clk_hz(void)
{
	 unsigned long fref, refdiv, mdiv, reg, vco;

	reg = readl(socfpga_get_clkmgr_addr() + CLKMGR_S10_MAINPLL_PLLGLOB);

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

	reg = readl(socfpga_get_clkmgr_addr() + CLKMGR_S10_MAINPLL_FDBCK);
	mdiv = (reg >> CLKMGR_FDBCK_MDIV_OFFSET) & CLKMGR_FDBCK_MDIV_MASK;

	vco = fref / refdiv;
	vco = vco * (CLKMGR_MDIV_CONST + mdiv);
	return vco;
}

static unsigned long cm_get_per_vco_clk_hz(void)
{
	unsigned long fref, refdiv, mdiv, reg, vco;

	reg = readl(socfpga_get_clkmgr_addr() + CLKMGR_S10_PERPLL_PLLGLOB);

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

	reg = readl(socfpga_get_clkmgr_addr() + CLKMGR_S10_PERPLL_FDBCK);
	mdiv = (reg >> CLKMGR_FDBCK_MDIV_OFFSET) & CLKMGR_FDBCK_MDIV_MASK;

	vco = fref / refdiv;
	vco = vco * (CLKMGR_MDIV_CONST + mdiv);
	return vco;
}

unsigned long cm_get_mpu_clk_hz(void)
{
	unsigned long clock = readl(socfpga_get_clkmgr_addr() +
				    CLKMGR_S10_MAINPLL_MPUCLK);

	clock = (clock >> CLKMGR_CLKSRC_OFFSET) & CLKMGR_CLKSRC_MASK;

	switch (clock) {
	case CLKMGR_CLKSRC_MAIN:
		clock = cm_get_main_vco_clk_hz();
		clock /= (readl(socfpga_get_clkmgr_addr() +
				CLKMGR_S10_MAINPLL_PLLC0) &
			  CLKMGR_PLLC0_DIV_MASK);
		break;

	case CLKMGR_CLKSRC_PER:
		clock = cm_get_per_vco_clk_hz();
		clock /= (readl(socfpga_get_clkmgr_addr() +
				CLKMGR_S10_PERPLL_PLLC0) &
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

	clock /= 1 + (readl(socfpga_get_clkmgr_addr() +
			    CLKMGR_S10_MAINPLL_MPUCLK) & CLKMGR_CLKCNT_MSK);
	return clock;
}

unsigned int cm_get_l3_main_clk_hz(void)
{
	u32 clock = readl(socfpga_get_clkmgr_addr() +
			  CLKMGR_S10_MAINPLL_NOCCLK);

	clock = (clock >> CLKMGR_CLKSRC_OFFSET) & CLKMGR_CLKSRC_MASK;

	switch (clock) {
	case CLKMGR_CLKSRC_MAIN:
		clock = cm_get_main_vco_clk_hz();
		clock /= (readl(socfpga_get_clkmgr_addr() +
				CLKMGR_S10_MAINPLL_PLLC1) &
			  CLKMGR_PLLC0_DIV_MASK);
		break;

	case CLKMGR_CLKSRC_PER:
		clock = cm_get_per_vco_clk_hz();
		clock /= (readl(socfpga_get_clkmgr_addr() +
			  CLKMGR_S10_PERPLL_PLLC1) & CLKMGR_CLKCNT_MSK);
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

	clock /= 1 + (readl(socfpga_get_clkmgr_addr() +
		      CLKMGR_S10_MAINPLL_NOCCLK) & CLKMGR_CLKCNT_MSK);
	return clock;
}

unsigned int cm_get_mmc_controller_clk_hz(void)
{
	u32 clock = readl(socfpga_get_clkmgr_addr() +
			  CLKMGR_S10_PERPLL_CNTR6CLK);

	clock = (clock >> CLKMGR_CLKSRC_OFFSET) & CLKMGR_CLKSRC_MASK;

	switch (clock) {
	case CLKMGR_CLKSRC_MAIN:
		clock = cm_get_l3_main_clk_hz();
		clock /= 1 + (readl(socfpga_get_clkmgr_addr() +
				    CLKMGR_S10_MAINPLL_CNTR6CLK) &
			      CLKMGR_CLKCNT_MSK);
		break;

	case CLKMGR_CLKSRC_PER:
		clock = cm_get_l3_main_clk_hz();
		clock /= 1 + (readl(socfpga_get_clkmgr_addr() +
				    CLKMGR_S10_PERPLL_CNTR6CLK) &
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

	clock /= (1 << ((readl(socfpga_get_clkmgr_addr() +
			       CLKMGR_S10_MAINPLL_NOCDIV) >>
			 CLKMGR_NOCDIV_L4SPCLK_OFFSET) & CLKMGR_CLKCNT_MSK));
	return clock;
}

unsigned int cm_get_spi_controller_clk_hz(void)
{
	u32 clock = cm_get_l3_main_clk_hz();

	clock /= (1 << ((readl(socfpga_get_clkmgr_addr() +
			       CLKMGR_S10_MAINPLL_NOCDIV) >>
			 CLKMGR_NOCDIV_L4MAIN_OFFSET) & CLKMGR_CLKCNT_MSK));
	return clock;
}

unsigned int cm_get_l4_sys_free_clk_hz(void)
{
	return cm_get_l3_main_clk_hz() / 4;
}

/*
 * Override weak dw_spi_get_clk implementation in designware_spi.c driver
 */

int dw_spi_get_clk(struct udevice *bus, ulong *rate)
{
	*rate = cm_get_spi_controller_clk_hz();
	if (!*rate) {
		printf("SPI: clock rate is zero");
		return -EINVAL;
	}

	return 0;
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
