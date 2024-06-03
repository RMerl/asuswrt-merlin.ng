// SPDX-License-Identifier: GPL-2.0+
/*
 * clock_am33xx.c
 *
 * clocks for AM33XX based boards
 *
 * Copyright (C) 2013, Texas Instruments, Incorporated - http://www.ti.com/
 */

#include <common.h>
#include <asm/arch/cpu.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/clock.h>
#include <asm/arch/hardware.h>
#include <asm/io.h>

#define OSC	(V_OSCK/1000000)

struct cm_perpll *const cmper = (struct cm_perpll *)CM_PER;
struct cm_wkuppll *const cmwkup = (struct cm_wkuppll *)CM_WKUP;
struct cm_dpll *const cmdpll = (struct cm_dpll *)CM_DPLL;
struct cm_rtc *const cmrtc = (struct cm_rtc *)CM_RTC;

const struct dpll_regs dpll_mpu_regs = {
	.cm_clkmode_dpll	= CM_WKUP + 0x88,
	.cm_idlest_dpll		= CM_WKUP + 0x20,
	.cm_clksel_dpll		= CM_WKUP + 0x2C,
	.cm_div_m2_dpll		= CM_WKUP + 0xA8,
};

const struct dpll_regs dpll_core_regs = {
	.cm_clkmode_dpll	= CM_WKUP + 0x90,
	.cm_idlest_dpll		= CM_WKUP + 0x5C,
	.cm_clksel_dpll		= CM_WKUP + 0x68,
	.cm_div_m4_dpll		= CM_WKUP + 0x80,
	.cm_div_m5_dpll		= CM_WKUP + 0x84,
	.cm_div_m6_dpll		= CM_WKUP + 0xD8,
};

const struct dpll_regs dpll_per_regs = {
	.cm_clkmode_dpll	= CM_WKUP + 0x8C,
	.cm_idlest_dpll		= CM_WKUP + 0x70,
	.cm_clksel_dpll		= CM_WKUP + 0x9C,
	.cm_div_m2_dpll		= CM_WKUP + 0xAC,
};

const struct dpll_regs dpll_ddr_regs = {
	.cm_clkmode_dpll	= CM_WKUP + 0x94,
	.cm_idlest_dpll		= CM_WKUP + 0x34,
	.cm_clksel_dpll		= CM_WKUP + 0x40,
	.cm_div_m2_dpll		= CM_WKUP + 0xA0,
};

const struct dpll_regs dpll_disp_regs = {
	.cm_clkmode_dpll	= CM_WKUP + 0x98,
	.cm_idlest_dpll		= CM_WKUP + 0x48,
	.cm_clksel_dpll		= CM_WKUP + 0x54,
	.cm_div_m2_dpll		= CM_WKUP + 0xA4,
};

struct dpll_params dpll_mpu_opp100 = {
		CONFIG_SYS_MPUCLK, OSC-1, 1, -1, -1, -1, -1};
const struct dpll_params dpll_core_opp100 = {
		1000, OSC-1, -1, -1, 10, 8, 4};

const struct dpll_params dpll_mpu_opp[NUM_CRYSTAL_FREQ][NUM_OPPS] = {
	{	/* 19.2 MHz */
		{125, 3, 2, -1, -1, -1, -1},	/* OPP 50 */
		{-1, -1, -1, -1, -1, -1, -1},	/* OPP RESERVED	*/
		{125, 3, 1, -1, -1, -1, -1},	/* OPP 100 */
		{150, 3, 1, -1, -1, -1, -1},	/* OPP 120 */
		{125, 2, 1, -1, -1, -1, -1},	/* OPP TB */
		{625, 11, 1, -1, -1, -1, -1}	/* OPP NT */
	},
	{	/* 24 MHz */
		{25, 0, 2, -1, -1, -1, -1},	/* OPP 50 */
		{-1, -1, -1, -1, -1, -1, -1},	/* OPP RESERVED	*/
		{25, 0, 1, -1, -1, -1, -1},	/* OPP 100 */
		{30, 0, 1, -1, -1, -1, -1},	/* OPP 120 */
		{100, 3, 1, -1, -1, -1, -1},	/* OPP TB */
		{125, 2, 1, -1, -1, -1, -1}	/* OPP NT */
	},
	{	/* 25 MHz */
		{24, 0, 2, -1, -1, -1, -1},	/* OPP 50 */
		{-1, -1, -1, -1, -1, -1, -1},	/* OPP RESERVED	*/
		{24, 0, 1, -1, -1, -1, -1},	/* OPP 100 */
		{144, 4, 1, -1, -1, -1, -1},	/* OPP 120 */
		{32, 0, 1, -1, -1, -1, -1},	/* OPP TB */
		{40, 0, 1, -1, -1, -1, -1}	/* OPP NT */
	},
	{	/* 26 MHz */
		{300, 12, 2, -1, -1, -1, -1},	/* OPP 50 */
		{-1, -1, -1, -1, -1, -1, -1},	/* OPP RESERVED	*/
		{300, 12, 1, -1, -1, -1, -1},	/* OPP 100 */
		{360, 12, 1, -1, -1, -1, -1},	/* OPP 120 */
		{400, 12, 1, -1, -1, -1, -1},	/* OPP TB */
		{500, 12, 1, -1, -1, -1, -1}	/* OPP NT */
	},
};

const struct dpll_params dpll_core_1000MHz[NUM_CRYSTAL_FREQ] = {
		{625, 11, -1, -1, 10, 8, 4},	/* 19.2 MHz */
		{125, 2, -1, -1, 10, 8, 4},	/* 24 MHz */
		{40, 0, -1, -1, 10, 8, 4},	/* 25 MHz */
		{500, 12, -1, -1, 10, 8, 4}	/* 26 MHz */
};

const struct dpll_params dpll_per_192MHz[NUM_CRYSTAL_FREQ] = {
		{400, 7, 5, -1, -1, -1, -1},	/* 19.2 MHz */
		{400, 9, 5, -1, -1, -1, -1},	/* 24 MHz */
		{384, 9, 5, -1, -1, -1, -1},	/* 25 MHz */
		{480, 12, 5, -1, -1, -1, -1}	/* 26 MHz */
};

const struct dpll_params dpll_ddr3_303MHz[NUM_CRYSTAL_FREQ] = {
		{505, 15, 2, -1, -1, -1, -1}, /*19.2*/
		{101, 3, 2, -1, -1, -1, -1}, /* 24 MHz */
		{303, 24, 1, -1, -1, -1, -1}, /* 25 MHz */
		{303, 12, 2, -1, -1, -1, -1}  /* 26 MHz */
};

const struct dpll_params dpll_ddr3_400MHz[NUM_CRYSTAL_FREQ] = {
		{125, 5, 1, -1, -1, -1, -1}, /*19.2*/
		{50, 2, 1, -1, -1, -1, -1}, /* 24 MHz */
		{16, 0, 1, -1, -1, -1, -1}, /* 25 MHz */
		{200, 12, 1, -1, -1, -1, -1}  /* 26 MHz */
};

const struct dpll_params dpll_ddr2_266MHz[NUM_CRYSTAL_FREQ] = {
		{665, 47, 1, -1, -1, -1, -1}, /*19.2*/
		{133, 11, 1, -1, -1, -1, -1}, /* 24 MHz */
		{266, 24, 1, -1, -1, -1, -1}, /* 25 MHz */
		{133, 12, 1, -1, -1, -1, -1}  /* 26 MHz */
};

__weak const struct dpll_params *get_dpll_mpu_params(void)
{
	return &dpll_mpu_opp100;
}

const struct dpll_params *get_dpll_core_params(void)
{
	int ind = get_sys_clk_index();

	return &dpll_core_1000MHz[ind];
}

const struct dpll_params *get_dpll_per_params(void)
{
	int ind = get_sys_clk_index();

	return &dpll_per_192MHz[ind];
}

void setup_clocks_for_console(void)
{
	clrsetbits_le32(&cmwkup->wkclkstctrl, CD_CLKCTRL_CLKTRCTRL_MASK,
			CD_CLKCTRL_CLKTRCTRL_SW_WKUP <<
			CD_CLKCTRL_CLKTRCTRL_SHIFT);

	clrsetbits_le32(&cmper->l4hsclkstctrl, CD_CLKCTRL_CLKTRCTRL_MASK,
			CD_CLKCTRL_CLKTRCTRL_SW_WKUP <<
			CD_CLKCTRL_CLKTRCTRL_SHIFT);

	clrsetbits_le32(&cmwkup->wkup_uart0ctrl,
			MODULE_CLKCTRL_MODULEMODE_MASK,
			MODULE_CLKCTRL_MODULEMODE_SW_EXPLICIT_EN <<
			MODULE_CLKCTRL_MODULEMODE_SHIFT);
	clrsetbits_le32(&cmper->uart1clkctrl,
			MODULE_CLKCTRL_MODULEMODE_MASK,
			MODULE_CLKCTRL_MODULEMODE_SW_EXPLICIT_EN <<
			MODULE_CLKCTRL_MODULEMODE_SHIFT);
	clrsetbits_le32(&cmper->uart2clkctrl,
			MODULE_CLKCTRL_MODULEMODE_MASK,
			MODULE_CLKCTRL_MODULEMODE_SW_EXPLICIT_EN <<
			MODULE_CLKCTRL_MODULEMODE_SHIFT);
	clrsetbits_le32(&cmper->uart3clkctrl,
			MODULE_CLKCTRL_MODULEMODE_MASK,
			MODULE_CLKCTRL_MODULEMODE_SW_EXPLICIT_EN <<
			MODULE_CLKCTRL_MODULEMODE_SHIFT);
	clrsetbits_le32(&cmper->uart4clkctrl,
			MODULE_CLKCTRL_MODULEMODE_MASK,
			MODULE_CLKCTRL_MODULEMODE_SW_EXPLICIT_EN <<
			MODULE_CLKCTRL_MODULEMODE_SHIFT);
	clrsetbits_le32(&cmper->uart5clkctrl,
			MODULE_CLKCTRL_MODULEMODE_MASK,
			MODULE_CLKCTRL_MODULEMODE_SW_EXPLICIT_EN <<
			MODULE_CLKCTRL_MODULEMODE_SHIFT);
}

void enable_basic_clocks(void)
{
	u32 *const clk_domains[] = {
		&cmper->l3clkstctrl,
		&cmper->l4fwclkstctrl,
		&cmper->l3sclkstctrl,
		&cmper->l4lsclkstctrl,
		&cmwkup->wkclkstctrl,
		&cmper->emiffwclkctrl,
		&cmrtc->clkstctrl,
		0
	};

	u32 *const clk_modules_explicit_en[] = {
		&cmper->l3clkctrl,
		&cmper->l4lsclkctrl,
		&cmper->l4fwclkctrl,
		&cmwkup->wkl4wkclkctrl,
		&cmper->l3instrclkctrl,
		&cmper->l4hsclkctrl,
		&cmwkup->wkgpio0clkctrl,
		&cmwkup->wkctrlclkctrl,
		&cmper->timer2clkctrl,
		&cmper->gpmcclkctrl,
		&cmper->elmclkctrl,
		&cmper->mmc0clkctrl,
		&cmper->mmc1clkctrl,
		&cmwkup->wkup_i2c0ctrl,
		&cmper->gpio1clkctrl,
		&cmper->gpio2clkctrl,
		&cmper->gpio3clkctrl,
		&cmper->i2c1clkctrl,
		&cmper->cpgmac0clkctrl,
		&cmper->spi0clkctrl,
		&cmrtc->rtcclkctrl,
		&cmper->usb0clkctrl,
		&cmper->emiffwclkctrl,
		&cmper->emifclkctrl,
		0
	};

	do_enable_clocks(clk_domains, clk_modules_explicit_en, 1);

	/* Select the Master osc 24 MHZ as Timer2 clock source */
	writel(0x1, &cmdpll->clktimer2clk);
}

/*
 * Enable Spread Spectrum for the MPU by calculating the required
 * values and setting the registers accordingly.
 * @param permille The spreading in permille (10th of a percent)
 */
void set_mpu_spreadspectrum(int permille)
{
	u32 multiplier_m;
	u32 predivider_n;
	u32 cm_clksel_dpll_mpu;
	u32 cm_clkmode_dpll_mpu;
	u32 ref_clock;
	u32 pll_bandwidth;
	u32 mod_freq_divider;
	u32 exponent;
	u32 mantissa;
	u32 delta_m_step;

	printf("Enabling Spread Spectrum of %d permille for MPU\n",
	       permille);

	/* Read PLL parameter m and n */
	cm_clksel_dpll_mpu = readl(&cmwkup->clkseldpllmpu);
	multiplier_m = (cm_clksel_dpll_mpu >> 8) & 0x3FF;
	predivider_n = cm_clksel_dpll_mpu & 0x7F;

	/*
	 * Calculate reference clock (clock after pre-divider),
	 * its max. PLL bandwidth,
	 * and resulting mod_freq_divider
	 */
	ref_clock = V_OSCK / (predivider_n + 1);
	pll_bandwidth = ref_clock / 70;
	mod_freq_divider = ref_clock / (4 * pll_bandwidth);

	/* Calculate Mantissa/Exponent */
	exponent = 0;
	mantissa = mod_freq_divider;
	while ((mantissa > 127) && (exponent < 7)) {
		exponent++;
		mantissa /= 2;
	}
	if (mantissa > 127)
		mantissa = 127;

	mod_freq_divider = mantissa << exponent;

	/*
	 * Calculate Modulation steps
	 * As we use Downspread only, the spread is twice the value of
	 * permille, so Div2!
	 * As it takes the value in percent, divide by ten!
	 */
	delta_m_step = ((u32)((multiplier_m * permille) / 10 / 2)) << 18;
	delta_m_step /= 100;
	delta_m_step /= mod_freq_divider;
	if (delta_m_step > 0xFFFFF)
		delta_m_step = 0xFFFFF;

	/* Setup Spread Spectrum */
	writel(delta_m_step, &cmwkup->sscdeltamstepdllmpu);
	writel((exponent << 8) | mantissa, &cmwkup->sscmodfreqdivdpllmpu);
	cm_clkmode_dpll_mpu = readl(&cmwkup->clkmoddpllmpu);
	/* clear all SSC flags */
	cm_clkmode_dpll_mpu &= ~(0xF << CM_CLKMODE_DPLL_SSC_EN_SHIFT);
	/* enable SSC with Downspread only */
	cm_clkmode_dpll_mpu |=  CM_CLKMODE_DPLL_SSC_EN_MASK |
				CM_CLKMODE_DPLL_SSC_DOWNSPREAD_MASK;
	writel(cm_clkmode_dpll_mpu, &cmwkup->clkmoddpllmpu);
	while (!(readl(&cmwkup->clkmoddpllmpu) & 0x2000))
		;
}
