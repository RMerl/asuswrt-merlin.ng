// SPDX-License-Identifier: GPL-2.0+
/*
 * clock_ti814x.c
 *
 * Clocks for TI814X based boards
 *
 * Copyright (C) 2013, Texas Instruments, Incorporated
 */

#include <common.h>
#include <asm/arch/cpu.h>
#include <asm/arch/clock.h>
#include <asm/arch/hardware.h>
#include <asm/io.h>

/* PRCM */
#define PRCM_MOD_EN		0x2

/* CLK_SRC */
#define OSC_SRC0		0
#define OSC_SRC1		1

#define L3_OSC_SRC		OSC_SRC0

#define OSC_0_FREQ		20

#define DCO_HS2_MIN		500
#define DCO_HS2_MAX		1000
#define DCO_HS1_MIN		1000
#define DCO_HS1_MAX		2000

#define SELFREQDCO_HS2		0x00000801
#define SELFREQDCO_HS1		0x00001001

#define MPU_N			0x1
#define MPU_M			0x3C
#define MPU_M2			1
#define MPU_CLKCTRL		0x1

#define L3_N			19
#define L3_M			880
#define L3_M2			4
#define L3_CLKCTRL		0x801

#define DDR_N			19
#define DDR_M			666
#define DDR_M2			2
#define DDR_CLKCTRL		0x801

/* ADPLLJ register values */
#define ADPLLJ_CLKCTRL_HS2	0x00000801 /* HS2 mode, TINT2 = 1 */
#define ADPLLJ_CLKCTRL_HS1	0x00001001 /* HS1 mode, TINT2 = 1 */
#define ADPLLJ_CLKCTRL_CLKDCOLDOEN	(1 << 29)
#define ADPLLJ_CLKCTRL_IDLE		(1 << 23)
#define ADPLLJ_CLKCTRL_CLKOUTEN		(1 << 20)
#define ADPLLJ_CLKCTRL_CLKOUTLDOEN	(1 << 19)
#define ADPLLJ_CLKCTRL_CLKDCOLDOPWDNZ	(1 << 17)
#define ADPLLJ_CLKCTRL_LPMODE		(1 << 12)
#define ADPLLJ_CLKCTRL_DRIFTGUARDIAN	(1 << 11)
#define ADPLLJ_CLKCTRL_REGM4XEN		(1 << 10)
#define ADPLLJ_CLKCTRL_TINITZ		(1 << 0)
#define ADPLLJ_CLKCTRL_CLKDCO		(ADPLLJ_CLKCTRL_CLKDCOLDOEN | \
					 ADPLLJ_CLKCTRL_CLKOUTEN | \
					 ADPLLJ_CLKCTRL_CLKOUTLDOEN | \
					 ADPLLJ_CLKCTRL_CLKDCOLDOPWDNZ)

#define ADPLLJ_STATUS_PHASELOCK		(1 << 10)
#define ADPLLJ_STATUS_FREQLOCK		(1 << 9)
#define ADPLLJ_STATUS_PHSFRQLOCK	(ADPLLJ_STATUS_PHASELOCK | \
					 ADPLLJ_STATUS_FREQLOCK)
#define ADPLLJ_STATUS_BYPASSACK		(1 << 8)
#define ADPLLJ_STATUS_BYPASS		(1 << 0)
#define ADPLLJ_STATUS_BYPASSANDACK	(ADPLLJ_STATUS_BYPASSACK | \
					 ADPLLJ_STATUS_BYPASS)

#define ADPLLJ_TENABLE_ENB		(1 << 0)
#define ADPLLJ_TENABLEDIV_ENB		(1 << 0)

#define ADPLLJ_M2NDIV_M2SHIFT		16

#define MPU_PLL_BASE			(PLL_SUBSYS_BASE + 0x048)
#define L3_PLL_BASE			(PLL_SUBSYS_BASE + 0x110)
#define DDR_PLL_BASE			(PLL_SUBSYS_BASE + 0x290)

struct ad_pll {
	unsigned int pwrctrl;
	unsigned int clkctrl;
	unsigned int tenable;
	unsigned int tenablediv;
	unsigned int m2ndiv;
	unsigned int mn2div;
	unsigned int fracdiv;
	unsigned int bwctrl;
	unsigned int fracctrl;
	unsigned int status;
	unsigned int m3div;
	unsigned int rampctrl;
};

#define OSC_SRC_CTRL			(PLL_SUBSYS_BASE + 0x2C0)

#define ENET_CLKCTRL_CMPL		0x30000

#define SATA_PLL_BASE			(CTRL_BASE + 0x0720)

struct sata_pll {
	unsigned int pllcfg0;
	unsigned int pllcfg1;
	unsigned int pllcfg2;
	unsigned int pllcfg3;
	unsigned int pllcfg4;
	unsigned int pllstatus;
	unsigned int rxstatus;
	unsigned int txstatus;
	unsigned int testcfg;
};

#define SEL_IN_FREQ		(0x1 << 31)
#define DIGCLRZ			(0x1 << 30)
#define ENDIGLDO		(0x1 << 4)
#define APLL_CP_CURR		(0x1 << 3)
#define ENBGSC_REF		(0x1 << 2)
#define ENPLLLDO		(0x1 << 1)
#define ENPLL			(0x1 << 0)

#define SATA_PLLCFG0_1 (SEL_IN_FREQ | ENBGSC_REF)
#define SATA_PLLCFG0_2 (SEL_IN_FREQ | ENDIGLDO | ENBGSC_REF)
#define SATA_PLLCFG0_3 (SEL_IN_FREQ | ENDIGLDO | ENBGSC_REF | ENPLLLDO)
#define SATA_PLLCFG0_4 (SEL_IN_FREQ | DIGCLRZ | ENDIGLDO | ENBGSC_REF | \
			ENPLLLDO | ENPLL)

#define PLL_LOCK		(0x1 << 0)

#define ENSATAMODE		(0x1 << 31)
#define PLLREFSEL		(0x1 << 30)
#define MDIVINT			(0x4b << 18)
#define EN_CLKAUX		(0x1 << 5)
#define EN_CLK125M		(0x1 << 4)
#define EN_CLK100M		(0x1 << 3)
#define EN_CLK50M		(0x1 << 2)

#define SATA_PLLCFG1 (ENSATAMODE |	\
		      PLLREFSEL |	\
		      MDIVINT |		\
		      EN_CLKAUX |	\
		      EN_CLK125M |	\
		      EN_CLK100M |	\
		      EN_CLK50M)

#define DIGLDO_EN_CAPLESSMODE	(0x1 << 22)
#define PLLDO_EN_LDO_STABLE	(0x1 << 11)
#define PLLDO_EN_BUF_CUR	(0x1 << 7)
#define PLLDO_EN_LP		(0x1 << 6)
#define PLLDO_CTRL_TRIM_1_4V	(0x10 << 1)

#define SATA_PLLCFG3 (DIGLDO_EN_CAPLESSMODE |	\
		      PLLDO_EN_LDO_STABLE |	\
		      PLLDO_EN_BUF_CUR |	\
		      PLLDO_EN_LP |		\
		      PLLDO_CTRL_TRIM_1_4V)

const struct cm_alwon *cmalwon = (struct cm_alwon *)CM_ALWON_BASE;
const struct cm_def *cmdef = (struct cm_def *)CM_DEFAULT_BASE;
const struct sata_pll *spll = (struct sata_pll *)SATA_PLL_BASE;

/*
 * Enable the peripheral clock for required peripherals
 */
static void enable_per_clocks(void)
{
	/* HSMMC1 */
	writel(PRCM_MOD_EN, &cmalwon->mmchs1clkctrl);
	while (readl(&cmalwon->mmchs1clkctrl) != PRCM_MOD_EN)
		;

	/* Ethernet */
	writel(PRCM_MOD_EN, &cmalwon->ethclkstctrl);
	writel(PRCM_MOD_EN, &cmalwon->ethernet0clkctrl);
	while ((readl(&cmalwon->ethernet0clkctrl) & ENET_CLKCTRL_CMPL) != 0)
		;
	writel(PRCM_MOD_EN, &cmalwon->ethernet1clkctrl);
	while ((readl(&cmalwon->ethernet1clkctrl) & ENET_CLKCTRL_CMPL) != 0)
		;

	/* RTC clocks */
	writel(PRCM_MOD_EN, &cmalwon->rtcclkstctrl);
	writel(PRCM_MOD_EN, &cmalwon->rtcclkctrl);
	while (readl(&cmalwon->rtcclkctrl) != PRCM_MOD_EN)
		;
}

/*
 * select the HS1 or HS2 for DCO Freq
 * return : CLKCTRL
 */
static u32 pll_dco_freq_sel(u32 clkout_dco)
{
	if (clkout_dco >= DCO_HS2_MIN && clkout_dco < DCO_HS2_MAX)
		return SELFREQDCO_HS2;
	else if (clkout_dco >= DCO_HS1_MIN && clkout_dco < DCO_HS1_MAX)
		return SELFREQDCO_HS1;
	else
		return -1;
}

/*
 * select the sigma delta config
 * return: sigma delta val
 */
static u32 pll_sigma_delta_val(u32 clkout_dco)
{
	u32 sig_val = 0;

	sig_val = (clkout_dco + 225) / 250;
	sig_val = sig_val << 24;

	return sig_val;
}

/*
 * configure individual ADPLLJ
 */
static void pll_config(u32 base, u32 n, u32 m, u32 m2,
		       u32 clkctrl_val, int adpllj)
{
	const struct ad_pll *adpll = (struct ad_pll *)base;
	u32 m2nval, mn2val, read_clkctrl = 0, clkout_dco = 0;
	u32 sig_val = 0, hs_mod = 0;

	m2nval = (m2 << ADPLLJ_M2NDIV_M2SHIFT) | n;
	mn2val = m;

	/* calculate clkout_dco */
	clkout_dco = ((OSC_0_FREQ / (n+1)) * m);

	/* sigma delta & Hs mode selection skip for ADPLLS*/
	if (adpllj) {
		sig_val = pll_sigma_delta_val(clkout_dco);
		hs_mod = pll_dco_freq_sel(clkout_dco);
	}

	/* by-pass pll */
	read_clkctrl = readl(&adpll->clkctrl);
	writel((read_clkctrl | ADPLLJ_CLKCTRL_IDLE), &adpll->clkctrl);
	while ((readl(&adpll->status) & ADPLLJ_STATUS_BYPASSANDACK)
		!= ADPLLJ_STATUS_BYPASSANDACK)
		;

	/* clear TINITZ */
	read_clkctrl = readl(&adpll->clkctrl);
	writel((read_clkctrl & ~ADPLLJ_CLKCTRL_TINITZ), &adpll->clkctrl);

	/*
	 * ref_clk = 20/(n + 1);
	 * clkout_dco = ref_clk * m;
	 * clk_out = clkout_dco/m2;
	*/
	read_clkctrl = readl(&adpll->clkctrl) &
			     ~(ADPLLJ_CLKCTRL_LPMODE |
			     ADPLLJ_CLKCTRL_DRIFTGUARDIAN |
			     ADPLLJ_CLKCTRL_REGM4XEN);
	writel(m2nval, &adpll->m2ndiv);
	writel(mn2val, &adpll->mn2div);

	/* Skip for modena(ADPLLS) */
	if (adpllj) {
		writel(sig_val, &adpll->fracdiv);
		writel((read_clkctrl | hs_mod), &adpll->clkctrl);
	}

	/* Load M2, N2 dividers of ADPLL */
	writel(ADPLLJ_TENABLEDIV_ENB, &adpll->tenablediv);
	writel(~ADPLLJ_TENABLEDIV_ENB, &adpll->tenablediv);

	/* Load M, N dividers of ADPLL */
	writel(ADPLLJ_TENABLE_ENB, &adpll->tenable);
	writel(~ADPLLJ_TENABLE_ENB, &adpll->tenable);

	/* Configure CLKDCOLDOEN,CLKOUTLDOEN,CLKOUT Enable BITS */
	read_clkctrl = readl(&adpll->clkctrl) & ~ADPLLJ_CLKCTRL_CLKDCO;
	if (adpllj)
		writel((read_clkctrl | ADPLLJ_CLKCTRL_CLKDCO),
						&adpll->clkctrl);

	/* Enable TINTZ and disable IDLE(PLL in Active & Locked Mode */
	read_clkctrl = readl(&adpll->clkctrl) & ~ADPLLJ_CLKCTRL_IDLE;
	writel((read_clkctrl | ADPLLJ_CLKCTRL_TINITZ), &adpll->clkctrl);

	/* Wait for phase and freq lock */
	while ((readl(&adpll->status) & ADPLLJ_STATUS_PHSFRQLOCK) !=
	       ADPLLJ_STATUS_PHSFRQLOCK)
		;
}

static void unlock_pll_control_mmr(void)
{
	/* TRM 2.10.1.4 and 3.2.7-3.2.11 */
	writel(0x1EDA4C3D, 0x481C5040);
	writel(0x2FF1AC2B, 0x48140060);
	writel(0xF757FDC0, 0x48140064);
	writel(0xE2BC3A6D, 0x48140068);
	writel(0x1EBF131D, 0x4814006c);
	writel(0x6F361E05, 0x48140070);
}

static void mpu_pll_config(void)
{
	pll_config(MPU_PLL_BASE, MPU_N, MPU_M, MPU_M2, MPU_CLKCTRL, 0);
}

static void l3_pll_config(void)
{
	u32 l3_osc_src, rd_osc_src = 0;

	l3_osc_src = L3_OSC_SRC;
	rd_osc_src = readl(OSC_SRC_CTRL);

	if (OSC_SRC0 == l3_osc_src)
		writel((rd_osc_src & 0xfffffffe)|0x0, OSC_SRC_CTRL);
	else
		writel((rd_osc_src & 0xfffffffe)|0x1, OSC_SRC_CTRL);

	pll_config(L3_PLL_BASE, L3_N, L3_M, L3_M2, L3_CLKCTRL, 1);
}

void ddr_pll_config(unsigned int ddrpll_m)
{
	pll_config(DDR_PLL_BASE, DDR_N, DDR_M, DDR_M2, DDR_CLKCTRL, 1);
}

void sata_pll_config(void)
{
	/*
	 * This sequence for configuring the SATA PLL
	 * resident in the control module is documented
	 * in TI8148 TRM section 21.3.1
	 */
	writel(SATA_PLLCFG1, &spll->pllcfg1);
	udelay(50);

	writel(SATA_PLLCFG3, &spll->pllcfg3);
	udelay(50);

	writel(SATA_PLLCFG0_1, &spll->pllcfg0);
	udelay(50);

	writel(SATA_PLLCFG0_2, &spll->pllcfg0);
	udelay(50);

	writel(SATA_PLLCFG0_3, &spll->pllcfg0);
	udelay(50);

	writel(SATA_PLLCFG0_4, &spll->pllcfg0);
	udelay(50);

	while (((readl(&spll->pllstatus) & PLL_LOCK) == 0))
		;
}

void enable_dmm_clocks(void)
{
	writel(PRCM_MOD_EN, &cmdef->fwclkctrl);
	writel(PRCM_MOD_EN, &cmdef->l3fastclkstctrl);
	writel(PRCM_MOD_EN, &cmdef->emif0clkctrl);
	while ((readl(&cmdef->emif0clkctrl)) != PRCM_MOD_EN)
		;
	writel(PRCM_MOD_EN, &cmdef->emif1clkctrl);
	while ((readl(&cmdef->emif1clkctrl)) != PRCM_MOD_EN)
		;
	while ((readl(&cmdef->l3fastclkstctrl) & 0x300) != 0x300)
		;
	writel(PRCM_MOD_EN, &cmdef->dmmclkctrl);
	while ((readl(&cmdef->dmmclkctrl)) != PRCM_MOD_EN)
		;
	writel(PRCM_MOD_EN, &cmalwon->l3slowclkstctrl);
	while ((readl(&cmalwon->l3slowclkstctrl) & 0x2100) != 0x2100)
		;
}

void setup_clocks_for_console(void)
{
	unlock_pll_control_mmr();
	/* UART0 */
	writel(PRCM_MOD_EN, &cmalwon->uart0clkctrl);
	while (readl(&cmalwon->uart0clkctrl) != PRCM_MOD_EN)
		;
}

void setup_early_clocks(void)
{
	setup_clocks_for_console();
}

/*
 * Configure the PLL/PRCM for necessary peripherals
 */
void prcm_init(void)
{
	/* Enable the control module */
	writel(PRCM_MOD_EN, &cmalwon->controlclkctrl);

	/* Configure PLLs */
	mpu_pll_config();
	l3_pll_config();
	sata_pll_config();

	/* Enable the required peripherals */
	enable_per_clocks();
}
