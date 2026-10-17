// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2021 NXP
 */

#include <common.h>
#include <div64.h>
#include <asm/io.h>
#include <errno.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/cgc.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <asm/global_data.h>
#include <linux/delay.h>
#include <hang.h>

DECLARE_GLOBAL_DATA_PTR;

static struct cgc1_regs *cgc1_regs = (struct cgc1_regs *)0x292C0000UL;
static struct cgc2_regs *cgc2_regs = (struct cgc2_regs *)0x2da60000UL;

void cgc1_soscdiv_init(void)
{
	/* Configure SOSC/FRO DIV1 ~ DIV3 */
	clrbits_le32(&cgc1_regs->soscdiv, BIT(7));
	clrbits_le32(&cgc1_regs->soscdiv, BIT(15));
	clrbits_le32(&cgc1_regs->soscdiv, BIT(23));
	clrbits_le32(&cgc1_regs->soscdiv, BIT(31));

	clrbits_le32(&cgc1_regs->frodiv, BIT(7));
}

void cgc1_pll2_init(ulong freq)
{
	u32 reg;

	if (readl(&cgc1_regs->pll2csr) & BIT(23))
		clrbits_le32(&cgc1_regs->pll2csr, BIT(23));

	/* Disable PLL2 */
	clrbits_le32(&cgc1_regs->pll2csr, BIT(0));
	mdelay(1);

	/* wait valid bit false */
	while ((readl(&cgc1_regs->pll2csr) & BIT(24)))
		;

	/* Select SOSC as source */
	reg = (freq / MHZ(24)) << 16;
	writel(reg, &cgc1_regs->pll2cfg);

	/* Enable PLL2 */
	setbits_le32(&cgc1_regs->pll2csr, BIT(0));

	/* Wait for PLL2 clock ready */
	while (!(readl(&cgc1_regs->pll2csr) & BIT(24)))
		;
}

static void cgc1_set_a35_clk(u32 clk_src, u32 div_core)
{
	u32 reg;

	/* ulock */
	if (readl(&cgc1_regs->ca35clk) & BIT(31))
		clrbits_le32(&cgc1_regs->ca35clk, BIT(31));

	reg = readl(&cgc1_regs->ca35clk);
	reg &= ~GENMASK(29, 21);
	reg |= ((clk_src & 0x3) << 28);
	reg |= (((div_core - 1) & 0x3f) << 21);
	writel(reg, &cgc1_regs->ca35clk);

	while (!(readl(&cgc1_regs->ca35clk) & BIT(27)))
		;
}

void cgc1_init_core_clk(ulong freq)
{
	u32 reg = readl(&cgc1_regs->ca35clk);

	/* if already selected to PLL2, switch to FRO firstly */
	if (((reg >> 28) & 0x3) == 0x1)
		cgc1_set_a35_clk(0, 1);

	cgc1_pll2_init(freq);

	/* Set A35 clock to pll2 */
	cgc1_set_a35_clk(1, 1);
}

void cgc1_enet_stamp_sel(u32 clk_src)
{
	writel((clk_src & 0x7) << 24, &cgc1_regs->enetstamp);
}

void cgc1_pll3_init(ulong freq)
{
	/* Gate off VCO */
	setbits_le32(&cgc1_regs->pll3div_vco, BIT(7));

	/* Disable PLL3 */
	clrbits_le32(&cgc1_regs->pll3csr, BIT(0));

	/* Gate off PFDxDIV */
	setbits_le32(&cgc1_regs->pll3div_pfd0, BIT(7) | BIT(15) | BIT(23) | BIT(31));
	setbits_le32(&cgc1_regs->pll3div_pfd1, BIT(7) | BIT(15) | BIT(23) | BIT(31));

	/* Gate off PFDx */
	setbits_le32(&cgc1_regs->pll3pfdcfg, BIT(7));
	setbits_le32(&cgc1_regs->pll3pfdcfg, BIT(15));
	setbits_le32(&cgc1_regs->pll3pfdcfg, BIT(23));
	setbits_le32(&cgc1_regs->pll3pfdcfg, BIT(31));

	/* Select SOSC as source */
	clrbits_le32(&cgc1_regs->pll3cfg, BIT(0));

	switch (freq) {
	case 540672000:
		writel(0x16 << 16, &cgc1_regs->pll3cfg);
		writel(0x16e3600, &cgc1_regs->pll3denom);
		writel(0xc15c00, &cgc1_regs->pll3num);
		break;
	default:
		hang();
	}

	/* Enable PLL3 */
	setbits_le32(&cgc1_regs->pll3csr, BIT(0));

	/* Wait for PLL3 clock ready */
	while (!(readl(&cgc1_regs->pll3csr) & BIT(24)))
		;
	/* Gate on VCO */
	clrbits_le32(&cgc1_regs->pll3div_vco, BIT(7));

	clrbits_le32(&cgc1_regs->pll3pfdcfg, 0x3F);
	setbits_le32(&cgc1_regs->pll3pfdcfg, 30 << 0); /* PFD0 324M */
	clrbits_le32(&cgc1_regs->pll3pfdcfg, BIT(7));
	while (!(readl(&cgc1_regs->pll3pfdcfg) & BIT(6)))
		;

	clrbits_le32(&cgc1_regs->pll3pfdcfg, 0x3F << 8);
	setbits_le32(&cgc1_regs->pll3pfdcfg, 25 << 8);	/* PFD1 389M */
	clrbits_le32(&cgc1_regs->pll3pfdcfg, BIT(15));
	while (!(readl(&cgc1_regs->pll3pfdcfg) & BIT(14)))
		;

	clrbits_le32(&cgc1_regs->pll3pfdcfg, 0x3F << 16);
	setbits_le32(&cgc1_regs->pll3pfdcfg, 30 << 16);	/* PFD2 324M */
	clrbits_le32(&cgc1_regs->pll3pfdcfg, BIT(23));
	while (!(readl(&cgc1_regs->pll3pfdcfg) & BIT(22)))
		;

	clrbits_le32(&cgc1_regs->pll3pfdcfg, 0x3F << 24);
	setbits_le32(&cgc1_regs->pll3pfdcfg, 25 << 24);	/* PFD3 389M */
	clrbits_le32(&cgc1_regs->pll3pfdcfg, BIT(31));
	while (!(readl(&cgc1_regs->pll3pfdcfg) & BIT(30)))
		;

	clrbits_le32(&cgc1_regs->pll3div_pfd0, 0x3f3f3f3f);
	if (IS_ENABLED(CONFIG_IMX8ULP_LD_MODE) || IS_ENABLED(CONFIG_IMX8ULP_ND_MODE))
		clrsetbits_le32(&cgc1_regs->pll3div_pfd1, 0x3f3f3f3f, 0x03010000); /* Set PFD3 DIV1 to 194M, PFD3 DIV2 to 97M */
	else
		clrsetbits_le32(&cgc1_regs->pll3div_pfd1, 0x3f3f3f3f, 0x01000000); /* Set PFD3 DIV1 to 389M, PFD3 DIV2 to 194M */
	clrbits_le32(&cgc1_regs->pll3div_pfd0, BIT(7));
	clrbits_le32(&cgc1_regs->pll3div_pfd0, BIT(15));
	clrbits_le32(&cgc1_regs->pll3div_pfd0, BIT(23));
	clrbits_le32(&cgc1_regs->pll3div_pfd0, BIT(31));

	clrbits_le32(&cgc1_regs->pll3div_pfd1, BIT(7));
	clrbits_le32(&cgc1_regs->pll3div_pfd1, BIT(15));
	clrbits_le32(&cgc1_regs->pll3div_pfd1, BIT(23));
	clrbits_le32(&cgc1_regs->pll3div_pfd1, BIT(31));

	/* NIC_AP:
	 * OD source PLL3 PFD0, 324M
	 * ND source FRO192, 192M
	 * LD source FRO192, 96M
	*/
	if (IS_ENABLED(CONFIG_IMX8ULP_LD_MODE)) {
		clrsetbits_le32(&cgc1_regs->nicclk, GENMASK(26, 21), 1 << 21);
	} else {
		clrbits_le32(&cgc1_regs->nicclk, GENMASK(26, 21));
	}

	if (!IS_ENABLED(CONFIG_IMX8ULP_LD_MODE) && !IS_ENABLED(CONFIG_IMX8ULP_ND_MODE)) {
		/* nicclk select pll3 pfd0 */
		clrsetbits_le32(&cgc1_regs->nicclk, GENMASK(29, 28), BIT(28));
		while (!(readl(&cgc1_regs->nicclk) & BIT(27)))
			;
	}
}

void cgc2_pll4_init(bool pll4_reset)
{
	/* Check the NICLPAV first to ensure not from PLL4 PFD1 clock */
	if ((readl(&cgc2_regs->niclpavclk) & GENMASK(29, 28)) == BIT(28)) {
		/* switch to FRO 192 first */
		clrbits_le32(&cgc2_regs->niclpavclk, GENMASK(29, 28));
		while (!(readl(&cgc2_regs->niclpavclk) & BIT(27)))
			;
	}

	/* Disable PFD DIV and clear DIV */
	writel(0x80808080, &cgc2_regs->pll4div_pfd0);
	writel(0x80808080, &cgc2_regs->pll4div_pfd1);

	/* Gate off and clear PFD  */
	writel(0x80808080, &cgc2_regs->pll4pfdcfg);

	if (pll4_reset) {
		/* Disable PLL4 */
		writel(0x0, &cgc2_regs->pll4csr);

		/* Configure PLL4 to 528Mhz and clock source from SOSC */
		writel(22 << 16, &cgc2_regs->pll4cfg);
		writel(0x1, &cgc2_regs->pll4csr);

		/* wait for PLL4 output valid */
		while (!(readl(&cgc2_regs->pll4csr) & BIT(24)))
			;
	}

	/* Enable all 4 PFDs */
	setbits_le32(&cgc2_regs->pll4pfdcfg, 18 << 0); /* 528 */
	setbits_le32(&cgc2_regs->pll4pfdcfg, 30 << 8); /* 316.8Mhz for NIC_LPAV */
	setbits_le32(&cgc2_regs->pll4pfdcfg, 30 << 16); /* 316.8Mhz */
	setbits_le32(&cgc2_regs->pll4pfdcfg, 24 << 24); /* 396Mhz */

	clrbits_le32(&cgc2_regs->pll4pfdcfg, BIT(7) | BIT(15) | BIT(23) | BIT(31));

	while ((readl(&cgc2_regs->pll4pfdcfg) & (BIT(30) | BIT(22) | BIT(14) | BIT(6)))
		!= (BIT(30) | BIT(22) | BIT(14) | BIT(6)))
		;

	/* Enable PFD DIV */
	clrbits_le32(&cgc2_regs->pll4div_pfd0, BIT(7) | BIT(15) | BIT(23) | BIT(31));
	clrbits_le32(&cgc2_regs->pll4div_pfd1, BIT(7) | BIT(15) | BIT(23) | BIT(31));

	/* NIC_LPAV:
	 * OD source PLL4 PFD1, 316.8M
	 * ND source FRO192, 192M
	 * LD source FRO192, 96M
	*/
	if (IS_ENABLED(CONFIG_IMX8ULP_LD_MODE)) {
		clrsetbits_le32(&cgc2_regs->niclpavclk, GENMASK(26, 21), 1 << 21);
	} else {
		clrbits_le32(&cgc2_regs->niclpavclk, GENMASK(26, 21));
	}

	if (!IS_ENABLED(CONFIG_IMX8ULP_LD_MODE) && !IS_ENABLED(CONFIG_IMX8ULP_ND_MODE)) {
		clrsetbits_le32(&cgc2_regs->niclpavclk, GENMASK(29, 28), BIT(28));
		while (!(readl(&cgc2_regs->niclpavclk) & BIT(27)))
			;
	}
}

void cgc2_pll4_pfd_config(enum cgc_clk pllpfd, u32 pfd)
{
	void __iomem *reg = &cgc2_regs->pll4div_pfd0;
	u32 halt_mask = BIT(7) | BIT(15);
	u32 pfd_shift = (pllpfd - PLL4_PFD0) * 8;
	u32 val;

	if (pllpfd < PLL4_PFD0 || pllpfd > PLL4_PFD3)
		return;

	if ((pllpfd - PLL4_PFD0) >> 1)
		reg = &cgc2_regs->pll4div_pfd1;

	halt_mask = halt_mask << (((pllpfd - PLL4_PFD0) & 0x1) * 16);

	/* halt pfd div */
	setbits_le32(reg, halt_mask);

	/* gate pfd */
	setbits_le32(&cgc2_regs->pll4pfdcfg, BIT(7) << pfd_shift);

	val = readl(&cgc2_regs->pll4pfdcfg);
	val &= ~(0x3f << pfd_shift);
	val |= (pfd << pfd_shift);
	writel(val, &cgc2_regs->pll4pfdcfg);

	/* ungate */
	clrbits_le32(&cgc2_regs->pll4pfdcfg, BIT(7) << pfd_shift);

	/* Wait stable */
	while ((readl(&cgc2_regs->pll4pfdcfg) & (BIT(6) << pfd_shift))
		!= (BIT(6) << pfd_shift))
		;

	/* enable pfd div */
	clrbits_le32(reg, halt_mask);
}

void cgc2_pll4_pfddiv_config(enum cgc_clk pllpfddiv, u32 div)
{
	void __iomem *reg = &cgc2_regs->pll4div_pfd0;
	u32 shift = ((pllpfddiv - PLL4_PFD0_DIV1) & 0x3) * 8;

	if (pllpfddiv < PLL4_PFD0_DIV1 || pllpfddiv > PLL4_PFD3_DIV2)
		return;

	if ((pllpfddiv - PLL4_PFD0_DIV1) >> 2)
		reg = &cgc2_regs->pll4div_pfd1;

	/* Halt pfd div */
	setbits_le32(reg, BIT(7) << shift);

	/* Clear div */
	clrbits_le32(reg, 0x3f << shift);

	/* Set div*/
	setbits_le32(reg, div << shift);

	/* Enable pfd div */
	clrbits_le32(reg, BIT(7) << shift);
}

void cgc2_ddrclk_config(u32 src, u32 div)
{
	/* If reg lock is set, wait until unlock by HW */
	/* This lock is triggered by div updating and ddrclk halt status change, */
	while ((readl(&cgc2_regs->ddrclk) & BIT(31)))
		;

	writel((src << 28) | (div << 21), &cgc2_regs->ddrclk);
	/* wait for DDRCLK switching done */
	while (!(readl(&cgc2_regs->ddrclk) & BIT(27)))
		;
}

void cgc2_ddrclk_wait_unlock(void)
{
	while ((readl(&cgc2_regs->ddrclk) & BIT(31)))
		;
}

void cgc2_lpav_init(enum cgc_clk clk)
{
	u32 i, scs, reg;
	const enum cgc_clk src[] = {FRO, PLL4_PFD1, SOSC, LVDS};

	reg = readl(&cgc2_regs->niclpavclk);
	scs = (reg >> 28) & 0x3;

	for (i = 0; i < 4; i++) {
		if (clk == src[i]) {
			if (scs == i)
				return;

			reg &= ~(0x3 << 28);
			reg |= (i << 28);

			writel(reg, &cgc2_regs->niclpavclk);
			break;
		}
	}

	if (i == 4)
		printf("Invalid clock source [%u] for LPAV\n", clk);
}

u32 cgc2_nic_get_rate(enum cgc_clk clk)
{
	u32 reg, rate;
	u32 scs, lpav_axi_clk, lpav_ahb_clk, lpav_bus_clk;
	const enum cgc_clk src[] = {FRO, PLL4_PFD1, SOSC, LVDS};

	reg = readl(&cgc2_regs->niclpavclk);
	scs = (reg >> 28) & 0x3;
	lpav_axi_clk = ((reg >> 21) & 0x3f) + 1;
	lpav_ahb_clk = ((reg >> 14) & 0x3f) + 1;
	lpav_bus_clk = ((reg >> 7) & 0x3f) + 1;

	rate = cgc_clk_get_rate(src[scs]);

	switch (clk) {
	case LPAV_AXICLK:
		rate = rate / lpav_axi_clk;
		break;
	case LPAV_AHBCLK:
		rate = rate / (lpav_axi_clk * lpav_ahb_clk);
		break;
	case LPAV_BUSCLK:
		rate = rate / (lpav_axi_clk * lpav_bus_clk);
		break;
	default:
		return 0;
	}

	return rate;
}

u32 decode_pll(enum cgc_clk pll)
{
	u32 reg, infreq, mult;
	u32 num, denom;

	infreq = 24000000U;
	/*
	 * Alought there are four choices for the bypass src,
	 * we choose SOSC 24M which is the default set in ROM.
	 * TODO: check more the comments
	 */
	switch (pll) {
	case PLL2:
		reg = readl(&cgc1_regs->pll2csr);
		if (!(reg & BIT(24)))
			return 0;

		reg = readl(&cgc1_regs->pll2cfg);
		mult = (reg >> 16) & 0x7F;
		denom = readl(&cgc1_regs->pll2denom) & 0x3FFFFFFF;
		num = readl(&cgc1_regs->pll2num) & 0x3FFFFFFF;

		return (u64)infreq * mult + (u64)infreq * num / denom;
	case PLL3:
		reg = readl(&cgc1_regs->pll3csr);
		if (!(reg & BIT(24)))
			return 0;

		reg = readl(&cgc1_regs->pll3cfg);
		mult = (reg >> 16) & 0x7F;
		denom = readl(&cgc1_regs->pll3denom) & 0x3FFFFFFF;
		num = readl(&cgc1_regs->pll3num) & 0x3FFFFFFF;

		return (u64)infreq * mult + (u64)infreq * num / denom;
	case PLL4:
		reg = readl(&cgc2_regs->pll4csr);
		if (!(reg & BIT(24)))
			return 0;

		reg = readl(&cgc2_regs->pll4cfg);
		mult = (reg >> 16) & 0x7F;
		denom = readl(&cgc2_regs->pll4denom) & 0x3FFFFFFF;
		num = readl(&cgc2_regs->pll4num) & 0x3FFFFFFF;

		return (u64)infreq * mult + (u64)infreq * num / denom;
	default:
		printf("Unsupported pll clocks %d\n", pll);
		break;
	}

	return 0;
}

u32 cgc_pll_vcodiv_rate(enum cgc_clk clk)
{
	u32 reg, gate, div;
	void __iomem *plldiv_vco;
	enum cgc_clk pll;

	if (clk == PLL3_VCODIV) {
		plldiv_vco = &cgc1_regs->pll3div_vco;
		pll = PLL3;
	} else {
		plldiv_vco = &cgc2_regs->pll4div_vco;
		pll = PLL4;
	}

	reg = readl(plldiv_vco);
	gate = BIT(7) & reg;
	div = reg & 0x3F;

	return gate ? 0 : decode_pll(pll) / (div + 1);
}

u32 cgc_pll_pfd_rate(enum cgc_clk clk)
{
	u32 index, gate, vld, reg;
	void __iomem *pllpfdcfg;
	enum cgc_clk pll;

	switch (clk) {
	case PLL3_PFD0:
	case PLL3_PFD1:
	case PLL3_PFD2:
	case PLL3_PFD3:
		index = clk - PLL3_PFD0;
		pllpfdcfg = &cgc1_regs->pll3pfdcfg;
		pll = PLL3;
		break;
	case PLL4_PFD0:
	case PLL4_PFD1:
	case PLL4_PFD2:
	case PLL4_PFD3:
		index = clk - PLL4_PFD0;
		pllpfdcfg = &cgc2_regs->pll4pfdcfg;
		pll = PLL4;
		break;
	default:
		return 0;
	}

	reg = readl(pllpfdcfg);
	gate = reg & (BIT(7) << (index * 8));
	vld = reg & (BIT(6) << (index * 8));

	if (gate || !vld)
		return 0;

	return (u64)decode_pll(pll) * 18 / ((reg >> (index * 8)) & 0x3F);
}

u32 cgc_pll_pfd_div(enum cgc_clk clk)
{
	void __iomem *base;
	u32 pfd, index, gate, reg;

	switch (clk) {
	case PLL3_PFD0_DIV1:
	case PLL3_PFD0_DIV2:
		base = &cgc1_regs->pll3div_pfd0;
		pfd = PLL3_PFD0;
		index = clk - PLL3_PFD0_DIV1;
		break;
	case PLL3_PFD1_DIV1:
	case PLL3_PFD1_DIV2:
		base = &cgc1_regs->pll3div_pfd0;
		pfd = PLL3_PFD1;
		index = clk - PLL3_PFD0_DIV1;
		break;
	case PLL3_PFD2_DIV1:
	case PLL3_PFD2_DIV2:
		base = &cgc1_regs->pll3div_pfd1;
		pfd = PLL3_PFD2;
		index = clk - PLL3_PFD2_DIV1;
		break;
	case PLL3_PFD3_DIV1:
	case PLL3_PFD3_DIV2:
		base = &cgc1_regs->pll3div_pfd1;
		pfd = PLL3_PFD3;
		index = clk - PLL3_PFD2_DIV1;
		break;
	case PLL4_PFD0_DIV1:
	case PLL4_PFD0_DIV2:
		base = &cgc2_regs->pll4div_pfd0;
		pfd = PLL4_PFD0;
		index = clk - PLL4_PFD0_DIV1;
		break;
	case PLL4_PFD1_DIV1:
	case PLL4_PFD1_DIV2:
		base = &cgc2_regs->pll4div_pfd0;
		pfd = PLL4_PFD1;
		index = clk - PLL4_PFD0_DIV1;
		break;
	case PLL4_PFD2_DIV1:
	case PLL4_PFD2_DIV2:
		base = &cgc2_regs->pll4div_pfd1;
		pfd = PLL4_PFD2;
		index = clk - PLL4_PFD2_DIV1;
		break;
	case PLL4_PFD3_DIV1:
	case PLL4_PFD3_DIV2:
		base = &cgc2_regs->pll4div_pfd1;
		pfd = PLL4_PFD3;
		index = clk - PLL4_PFD2_DIV1;
		break;
	default:
		return 0;
	}

	reg = readl(base);
	gate = reg & (BIT(7) << (index * 8));

	if (gate)
		return 0;

	return cgc_pll_pfd_rate(pfd) / (((reg >> (index * 8)) & 0x3F) + 1);
}

u32 cgc1_nic_get_rate(enum cgc_clk clk)
{
	u32 reg, rate;
	u32 scs, nic_ad_divplat, nic_per_divplat;
	u32 xbar_ad_divplat, xbar_divbus, ad_slow;
	const enum cgc_clk src[] = {FRO, PLL3_PFD0, SOSC, LVDS};

	reg = readl(&cgc1_regs->nicclk);
	scs = (reg >> 28) & 0x3;
	nic_ad_divplat = ((reg >> 21) & 0x3f) + 1;
	nic_per_divplat = ((reg >> 14) & 0x3f) + 1;

	reg = readl(&cgc1_regs->xbarclk);
	xbar_ad_divplat = ((reg >> 14) & 0x3f) + 1;
	xbar_divbus = ((reg >> 7) & 0x3f) + 1;
	ad_slow = (reg & 0x3f) + 1;

	rate = cgc_clk_get_rate(src[scs]);

	switch (clk) {
	case NIC_APCLK:
		rate = rate / nic_ad_divplat;
		break;
	case NIC_PERCLK:
		rate = rate / (nic_ad_divplat * nic_per_divplat);
		break;
	case XBAR_APCLK:
		rate = rate / (nic_ad_divplat * xbar_ad_divplat);
		break;
	case XBAR_BUSCLK:
		rate = rate / (nic_ad_divplat * xbar_ad_divplat * xbar_divbus);
		break;
	case AD_SLOWCLK:
		rate = rate / (nic_ad_divplat * xbar_ad_divplat * ad_slow);
		break;
	default:
		return 0;
	}

	return rate;
}

u32 cgc1_sosc_div(enum cgc_clk clk)
{
	u32 reg, gate, index;

	switch (clk) {
	case SOSC:
		return 24000000;
	case SOSC_DIV1:
		index = 0;
		break;
	case SOSC_DIV2:
		index = 1;
		break;
	case SOSC_DIV3:
		index = 2;
		break;
	default:
		return 0;
	}

	reg = readl(&cgc1_regs->soscdiv);
	gate = reg & (BIT(7) << (index * 8));

	if (gate)
		return 0;

	return 24000000 / (((reg >> (index * 8)) & 0x3F) + 1);
}

u32 cgc1_fro_div(enum cgc_clk clk)
{
	u32 reg, gate, vld, index;

	switch (clk) {
	case FRO:
		return 192000000;
	case FRO_DIV1:
		index = 0;
		break;
	case FRO_DIV2:
		index = 1;
		break;
	case FRO_DIV3:
		index = 2;
		break;
	default:
		return 0;
	}

	reg = readl(&cgc1_regs->frodiv);
	gate = reg & (BIT(7) << (index * 8));
	vld = reg & (BIT(6) << (index * 8));

	if (gate || !vld)
		return 0;

	return 24000000 / (((reg >> (index * 8)) & 0x3F) + 1);
}

u32 cgc_clk_get_rate(enum cgc_clk clk)
{
	switch (clk) {
	case LVDS:
		return 0; /* No external LVDS clock used */
	case SOSC:
	case SOSC_DIV1:
	case SOSC_DIV2:
	case SOSC_DIV3:
		return cgc1_sosc_div(clk);
	case FRO:
	case FRO_DIV1:
	case FRO_DIV2:
	case FRO_DIV3:
		return cgc1_fro_div(clk);
	case PLL2:
	case PLL3:
	case PLL4:
		return decode_pll(clk);
	case PLL3_VCODIV:
	case PLL4_VCODIV:
		return cgc_pll_vcodiv_rate(clk);
	case PLL3_PFD0:
	case PLL3_PFD1:
	case PLL3_PFD2:
	case PLL3_PFD3:
	case PLL4_PFD0:
	case PLL4_PFD1:
	case PLL4_PFD2:
	case PLL4_PFD3:
		return cgc_pll_pfd_rate(clk);
	case PLL3_PFD0_DIV1:
	case PLL3_PFD0_DIV2:
	case PLL3_PFD1_DIV1:
	case PLL3_PFD1_DIV2:
	case PLL3_PFD2_DIV1:
	case PLL3_PFD2_DIV2:
	case PLL3_PFD3_DIV1:
	case PLL3_PFD3_DIV2:
	case PLL4_PFD0_DIV1:
	case PLL4_PFD0_DIV2:
	case PLL4_PFD1_DIV1:
	case PLL4_PFD1_DIV2:
	case PLL4_PFD2_DIV1:
	case PLL4_PFD2_DIV2:
	case PLL4_PFD3_DIV1:
	case PLL4_PFD3_DIV2:
		return cgc_pll_pfd_div(clk);
	case NIC_APCLK:
	case NIC_PERCLK:
	case XBAR_APCLK:
	case XBAR_BUSCLK:
	case AD_SLOWCLK:
		return cgc1_nic_get_rate(clk);
	case LPAV_AXICLK:
	case LPAV_AHBCLK:
	case LPAV_BUSCLK:
		return cgc2_nic_get_rate(clk);
	default:
		printf("Unsupported cgc clock: %d\n", clk);
		return 0;
	}
}
