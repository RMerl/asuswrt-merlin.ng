// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <div64.h>
#include <log.h>
#include <asm/io.h>
#include <errno.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/pcc.h>
#include <asm/arch/sys_proto.h>
#include <linux/delay.h>

scg_p scg1_regs = (scg_p)SCG1_RBASE;

static u32 scg_src_get_rate(enum scg_clk clksrc)
{
	u32 reg;

	switch (clksrc) {
	case SCG_SOSC_CLK:
		reg = readl(&scg1_regs->sosccsr);
		if (!(reg & SCG_SOSC_CSR_SOSCVLD_MASK))
			return 0;

		return 24000000;
	case SCG_FIRC_CLK:
		reg = readl(&scg1_regs->firccsr);
		if (!(reg & SCG_FIRC_CSR_FIRCVLD_MASK))
			return 0;

		return 48000000;
	case SCG_SIRC_CLK:
		reg = readl(&scg1_regs->sirccsr);
		if (!(reg & SCG_SIRC_CSR_SIRCVLD_MASK))
			return 0;

		return 16000000;
	case SCG_ROSC_CLK:
		reg = readl(&scg1_regs->rtccsr);
		if (!(reg & SCG_ROSC_CSR_ROSCVLD_MASK))
			return 0;

		return 32768;
	default:
		break;
	}

	return 0;
}

static u32 scg_sircdiv_get_rate(enum scg_clk clk)
{
	u32 reg, val, rate;
	u32 shift, mask;

	switch (clk) {
	case SCG_SIRC_DIV1_CLK:
		mask = SCG_SIRCDIV_DIV1_MASK;
		shift = SCG_SIRCDIV_DIV1_SHIFT;
		break;
	case SCG_SIRC_DIV2_CLK:
		mask = SCG_SIRCDIV_DIV2_MASK;
		shift = SCG_SIRCDIV_DIV2_SHIFT;
		break;
	case SCG_SIRC_DIV3_CLK:
		mask = SCG_SIRCDIV_DIV3_MASK;
		shift = SCG_SIRCDIV_DIV3_SHIFT;
		break;
	default:
		return 0;
	}

	reg = readl(&scg1_regs->sirccsr);
	if (!(reg & SCG_SIRC_CSR_SIRCVLD_MASK))
		return 0;

	reg = readl(&scg1_regs->sircdiv);
	val = (reg & mask) >> shift;

	if (!val) /*clock disabled*/
		return 0;

	rate = scg_src_get_rate(SCG_SIRC_CLK);
	rate = rate / (1 << (val - 1));

	return rate;
}

static u32 scg_fircdiv_get_rate(enum scg_clk clk)
{
	u32 reg, val, rate;
	u32 shift, mask;

	switch (clk) {
	case SCG_FIRC_DIV1_CLK:
		mask = SCG_FIRCDIV_DIV1_MASK;
		shift = SCG_FIRCDIV_DIV1_SHIFT;
		break;
	case SCG_FIRC_DIV2_CLK:
		mask = SCG_FIRCDIV_DIV2_MASK;
		shift = SCG_FIRCDIV_DIV2_SHIFT;
		break;
	case SCG_FIRC_DIV3_CLK:
		mask = SCG_FIRCDIV_DIV3_MASK;
		shift = SCG_FIRCDIV_DIV3_SHIFT;
		break;
	default:
		return 0;
	}

	reg = readl(&scg1_regs->firccsr);
	if (!(reg & SCG_FIRC_CSR_FIRCVLD_MASK))
		return 0;

	reg = readl(&scg1_regs->fircdiv);
	val = (reg & mask) >> shift;

	if (!val) /*clock disabled*/
		return 0;

	rate = scg_src_get_rate(SCG_FIRC_CLK);
	rate = rate / (1 << (val - 1));

	return rate;
}

static u32 scg_soscdiv_get_rate(enum scg_clk clk)
{
	u32 reg, val, rate;
	u32 shift, mask;

	switch (clk) {
	case SCG_SOSC_DIV1_CLK:
		mask = SCG_SOSCDIV_DIV1_MASK;
		shift = SCG_SOSCDIV_DIV1_SHIFT;
		break;
	case SCG_SOSC_DIV2_CLK:
		mask = SCG_SOSCDIV_DIV2_MASK;
		shift = SCG_SOSCDIV_DIV2_SHIFT;
		break;
	case SCG_SOSC_DIV3_CLK:
		mask = SCG_SOSCDIV_DIV3_MASK;
		shift = SCG_SOSCDIV_DIV3_SHIFT;
		break;
	default:
		return 0;
	}

	reg = readl(&scg1_regs->sosccsr);
	if (!(reg & SCG_SOSC_CSR_SOSCVLD_MASK))
		return 0;

	reg = readl(&scg1_regs->soscdiv);
	val = (reg & mask) >> shift;

	if (!val) /*clock disabled*/
		return 0;

	rate = scg_src_get_rate(SCG_SOSC_CLK);
	rate = rate / (1 << (val - 1));

	return rate;
}

static u32 scg_apll_pfd_get_rate(enum scg_clk clk)
{
	u32 reg, val, rate;
	u32 shift, mask, gate, valid;

	switch (clk) {
	case SCG_APLL_PFD0_CLK:
		gate = SCG_PLL_PFD0_GATE_MASK;
		valid = SCG_PLL_PFD0_VALID_MASK;
		mask = SCG_PLL_PFD0_FRAC_MASK;
		shift = SCG_PLL_PFD0_FRAC_SHIFT;
		break;
	case SCG_APLL_PFD1_CLK:
		gate = SCG_PLL_PFD1_GATE_MASK;
		valid = SCG_PLL_PFD1_VALID_MASK;
		mask = SCG_PLL_PFD1_FRAC_MASK;
		shift = SCG_PLL_PFD1_FRAC_SHIFT;
		break;
	case SCG_APLL_PFD2_CLK:
		gate = SCG_PLL_PFD2_GATE_MASK;
		valid = SCG_PLL_PFD2_VALID_MASK;
		mask = SCG_PLL_PFD2_FRAC_MASK;
		shift = SCG_PLL_PFD2_FRAC_SHIFT;
		break;
	case SCG_APLL_PFD3_CLK:
		gate = SCG_PLL_PFD3_GATE_MASK;
		valid = SCG_PLL_PFD3_VALID_MASK;
		mask = SCG_PLL_PFD3_FRAC_MASK;
		shift = SCG_PLL_PFD3_FRAC_SHIFT;
		break;
	default:
		return 0;
	}

	reg = readl(&scg1_regs->apllpfd);
	if (reg & gate || !(reg & valid))
		return 0;

	clk_debug("scg_apll_pfd_get_rate reg 0x%x\n", reg);

	val = (reg & mask) >> shift;
	rate = decode_pll(PLL_A7_APLL);

	rate = rate / val * 18;

	clk_debug("scg_apll_pfd_get_rate rate %u\n", rate);

	return rate;
}

static u32 scg_spll_pfd_get_rate(enum scg_clk clk)
{
	u32 reg, val, rate;
	u32 shift, mask, gate, valid;

	switch (clk) {
	case SCG_SPLL_PFD0_CLK:
		gate = SCG_PLL_PFD0_GATE_MASK;
		valid = SCG_PLL_PFD0_VALID_MASK;
		mask = SCG_PLL_PFD0_FRAC_MASK;
		shift = SCG_PLL_PFD0_FRAC_SHIFT;
		break;
	case SCG_SPLL_PFD1_CLK:
		gate = SCG_PLL_PFD1_GATE_MASK;
		valid = SCG_PLL_PFD1_VALID_MASK;
		mask = SCG_PLL_PFD1_FRAC_MASK;
		shift = SCG_PLL_PFD1_FRAC_SHIFT;
		break;
	case SCG_SPLL_PFD2_CLK:
		gate = SCG_PLL_PFD2_GATE_MASK;
		valid = SCG_PLL_PFD2_VALID_MASK;
		mask = SCG_PLL_PFD2_FRAC_MASK;
		shift = SCG_PLL_PFD2_FRAC_SHIFT;
		break;
	case SCG_SPLL_PFD3_CLK:
		gate = SCG_PLL_PFD3_GATE_MASK;
		valid = SCG_PLL_PFD3_VALID_MASK;
		mask = SCG_PLL_PFD3_FRAC_MASK;
		shift = SCG_PLL_PFD3_FRAC_SHIFT;
		break;
	default:
		return 0;
	}

	reg = readl(&scg1_regs->spllpfd);
	if (reg & gate || !(reg & valid))
		return 0;

	clk_debug("scg_spll_pfd_get_rate reg 0x%x\n", reg);

	val = (reg & mask) >> shift;
	rate = decode_pll(PLL_A7_SPLL);

	rate = rate / val * 18;

	clk_debug("scg_spll_pfd_get_rate rate %u\n", rate);

	return rate;
}

static u32 scg_apll_get_rate(void)
{
	u32 reg, val, rate;

	reg = readl(&scg1_regs->apllcfg);
	val = (reg & SCG_PLL_CFG_PLLSEL_MASK) >> SCG_PLL_CFG_PLLSEL_SHIFT;

	if (!val) {
		/* APLL clock after two dividers */
		rate = decode_pll(PLL_A7_APLL);

		val = (reg & SCG_PLL_CFG_POSTDIV1_MASK) >>
			SCG_PLL_CFG_POSTDIV1_SHIFT;
		rate = rate / (val + 1);

		val = (reg & SCG_PLL_CFG_POSTDIV2_MASK) >>
			SCG_PLL_CFG_POSTDIV2_SHIFT;
		rate = rate / (val + 1);
	} else {
		/* APLL PFD clock */
		val = (reg & SCG_PLL_CFG_PFDSEL_MASK) >>
			SCG_PLL_CFG_PFDSEL_SHIFT;
		rate = scg_apll_pfd_get_rate(SCG_APLL_PFD0_CLK + val);
	}

	return rate;
}

static u32 scg_spll_get_rate(void)
{
	u32 reg, val, rate;

	reg = readl(&scg1_regs->spllcfg);
	val = (reg & SCG_PLL_CFG_PLLSEL_MASK) >> SCG_PLL_CFG_PLLSEL_SHIFT;

	clk_debug("scg_spll_get_rate reg 0x%x\n", reg);

	if (!val) {
		/* APLL clock after two dividers */
		rate = decode_pll(PLL_A7_SPLL);

		val = (reg & SCG_PLL_CFG_POSTDIV1_MASK) >>
			SCG_PLL_CFG_POSTDIV1_SHIFT;
		rate = rate / (val + 1);

		val = (reg & SCG_PLL_CFG_POSTDIV2_MASK) >>
			SCG_PLL_CFG_POSTDIV2_SHIFT;
		rate = rate / (val + 1);

		clk_debug("scg_spll_get_rate SPLL %u\n", rate);

	} else {
		/* APLL PFD clock */
		val = (reg & SCG_PLL_CFG_PFDSEL_MASK) >>
			SCG_PLL_CFG_PFDSEL_SHIFT;
		rate = scg_spll_pfd_get_rate(SCG_SPLL_PFD0_CLK + val);

		clk_debug("scg_spll_get_rate PFD %u\n", rate);
	}

	return rate;
}

static u32 scg_ddr_get_rate(void)
{
	u32 reg, val, rate, div;

	reg = readl(&scg1_regs->ddrccr);
	val = (reg & SCG_DDRCCR_DDRCS_MASK) >> SCG_DDRCCR_DDRCS_SHIFT;
	div = (reg & SCG_DDRCCR_DDRDIV_MASK) >> SCG_DDRCCR_DDRDIV_SHIFT;

	if (!div)
		return 0;

	if (!val) {
		reg = readl(&scg1_regs->apllcfg);
		val = (reg & SCG_PLL_CFG_PFDSEL_MASK) >>
			SCG_PLL_CFG_PFDSEL_SHIFT;
		rate = scg_apll_pfd_get_rate(SCG_APLL_PFD0_CLK + val);
	} else {
		rate = decode_pll(PLL_USB);
	}

	rate = rate / (1 << (div - 1));
	return rate;
}

static u32 scg_nic_get_rate(enum scg_clk clk)
{
	u32 reg, val, rate, nic0_rate;
	u32 shift, mask;

	reg = readl(&scg1_regs->niccsr);
	val = (reg & SCG_NICCSR_NICCS_MASK) >> SCG_NICCSR_NICCS_SHIFT;

	clk_debug("scg_nic_get_rate niccsr 0x%x\n", reg);

	if (!val)
		rate = scg_src_get_rate(SCG_FIRC_CLK);
	else
		rate = scg_ddr_get_rate();

	clk_debug("scg_nic_get_rate parent rate %u\n", rate);

	val = (reg & SCG_NICCSR_NIC0DIV_MASK) >> SCG_NICCSR_NIC0DIV_SHIFT;

	rate = rate / (val + 1);
	nic0_rate = rate;

	clk_debug("scg_nic_get_rate NIC0 rate %u\n", rate);

	switch (clk) {
	case SCG_NIC0_CLK:
		return rate;
	case SCG_GPU_CLK:
		mask = SCG_NICCSR_GPUDIV_MASK;
		shift = SCG_NICCSR_GPUDIV_SHIFT;
		break;
	case SCG_NIC1_EXT_CLK:
	case SCG_NIC1_BUS_CLK:
	case SCG_NIC1_CLK:
		mask = SCG_NICCSR_NIC1DIV_MASK;
		shift = SCG_NICCSR_NIC1DIV_SHIFT;
		break;
	default:
		return 0;
	}

	val = (reg & mask) >> shift;
	rate = rate / (val + 1);

	clk_debug("scg_nic_get_rate NIC1 rate %u\n", rate);

	switch (clk) {
	case SCG_GPU_CLK:
	case SCG_NIC1_CLK:
		return rate;
	case SCG_NIC1_EXT_CLK:
		mask = SCG_NICCSR_NIC1EXTDIV_MASK;
		shift = SCG_NICCSR_NIC1EXTDIV_SHIFT;
		break;
	case SCG_NIC1_BUS_CLK:
		mask = SCG_NICCSR_NIC1BUSDIV_MASK;
		shift = SCG_NICCSR_NIC1BUSDIV_SHIFT;
		break;
	default:
		return 0;
	}

	/*
	 * On RevB, the nic_bus and nic_ext dividers are parallel
	 * not chained with nic div
	 */
	if (soc_rev() >= CHIP_REV_2_0)
		rate = nic0_rate;

	val = (reg & mask) >> shift;
	rate = rate / (val + 1);

	clk_debug("scg_nic_get_rate NIC1 bus rate %u\n", rate);
	return rate;
}


static enum scg_clk scg_scs_array[4] = {
	SCG_SOSC_CLK, SCG_SIRC_CLK, SCG_FIRC_CLK, SCG_ROSC_CLK,
};

static u32 scg_sys_get_rate(enum scg_clk clk)
{
	u32 reg, val, rate;

	if (clk != SCG_CORE_CLK && clk != SCG_BUS_CLK)
		return 0;

	reg = readl(&scg1_regs->csr);
	val = (reg & SCG_CCR_SCS_MASK) >> SCG_CCR_SCS_SHIFT;

	clk_debug("scg_sys_get_rate reg 0x%x\n", reg);

	switch (val) {
	case SCG_SCS_SYS_OSC:
	case SCG_SCS_SLOW_IRC:
	case SCG_SCS_FAST_IRC:
	case SCG_SCS_RTC_OSC:
		rate = scg_src_get_rate(scg_scs_array[val - 1]);
		break;
	case 5:
		rate = scg_apll_get_rate();
		break;
	case 6:
		rate = scg_spll_get_rate();
		break;
	default:
		return 0;
	}

	clk_debug("scg_sys_get_rate parent rate %u\n", rate);

	val = (reg & SCG_CCR_DIVCORE_MASK) >> SCG_CCR_DIVCORE_SHIFT;

	rate = rate / (val + 1);

	if (clk == SCG_BUS_CLK) {
		val = (reg & SCG_CCR_DIVBUS_MASK) >> SCG_CCR_DIVBUS_SHIFT;
		rate = rate / (val + 1);
	}

	return rate;
}

u32 decode_pll(enum pll_clocks pll)
{
	u32 reg,  pre_div, infreq, mult;
	u32 num, denom;

	/*
	 * Alought there are four choices for the bypass src,
	 * we choose OSC_24M which is the default set in ROM.
	 */
	switch (pll) {
	case PLL_A7_SPLL:
		reg = readl(&scg1_regs->spllcsr);

		if (!(reg & SCG_SPLL_CSR_SPLLVLD_MASK))
			return 0;

		reg = readl(&scg1_regs->spllcfg);

		pre_div = (reg & SCG_PLL_CFG_PREDIV_MASK) >>
			   SCG_PLL_CFG_PREDIV_SHIFT;
		pre_div += 1;

		mult = (reg & SCG1_SPLL_CFG_MULT_MASK) >>
			   SCG_PLL_CFG_MULT_SHIFT;

		infreq = (reg & SCG_PLL_CFG_CLKSRC_MASK) >>
			   SCG_PLL_CFG_CLKSRC_SHIFT;
		if (!infreq)
			infreq = scg_src_get_rate(SCG_SOSC_CLK);
		else
			infreq = scg_src_get_rate(SCG_FIRC_CLK);

		num = readl(&scg1_regs->spllnum);
		denom = readl(&scg1_regs->splldenom);

		infreq = infreq / pre_div;

		if (denom)
			return infreq * mult + infreq * num / denom;
		else
			return infreq * mult;

	case PLL_A7_APLL:
		reg = readl(&scg1_regs->apllcsr);

		if (!(reg & SCG_APLL_CSR_APLLVLD_MASK))
			return 0;

		reg = readl(&scg1_regs->apllcfg);

		pre_div = (reg & SCG_PLL_CFG_PREDIV_MASK) >>
			   SCG_PLL_CFG_PREDIV_SHIFT;
		pre_div += 1;

		mult = (reg & SCG_APLL_CFG_MULT_MASK) >>
			   SCG_PLL_CFG_MULT_SHIFT;

		infreq = (reg & SCG_PLL_CFG_CLKSRC_MASK) >>
			   SCG_PLL_CFG_CLKSRC_SHIFT;
		if (!infreq)
			infreq = scg_src_get_rate(SCG_SOSC_CLK);
		else
			infreq = scg_src_get_rate(SCG_FIRC_CLK);

		num = readl(&scg1_regs->apllnum);
		denom = readl(&scg1_regs->aplldenom);

		infreq = infreq / pre_div;

		if (denom)
			return infreq * mult + infreq * num / denom;
		else
			return infreq * mult;

	case PLL_USB:
		reg = readl(&scg1_regs->upllcsr);

		if (!(reg & SCG_UPLL_CSR_UPLLVLD_MASK))
			return 0;

		return 480000000u;

	case PLL_MIPI:
		return 480000000u;
	default:
		printf("Unsupported pll clocks %d\n", pll);
		break;
	}

	return 0;
}

u32 scg_clk_get_rate(enum scg_clk clk)
{
	switch (clk) {
	case SCG_SIRC_DIV1_CLK:
	case SCG_SIRC_DIV2_CLK:
	case SCG_SIRC_DIV3_CLK:
		return scg_sircdiv_get_rate(clk);

	case SCG_FIRC_DIV1_CLK:
	case SCG_FIRC_DIV2_CLK:
	case SCG_FIRC_DIV3_CLK:
		return scg_fircdiv_get_rate(clk);

	case SCG_SOSC_DIV1_CLK:
	case SCG_SOSC_DIV2_CLK:
	case SCG_SOSC_DIV3_CLK:
		return scg_soscdiv_get_rate(clk);

	case SCG_CORE_CLK:
	case SCG_BUS_CLK:
		return scg_sys_get_rate(clk);

	case SCG_SPLL_PFD0_CLK:
	case SCG_SPLL_PFD1_CLK:
	case SCG_SPLL_PFD2_CLK:
	case SCG_SPLL_PFD3_CLK:
		return scg_spll_pfd_get_rate(clk);

	case SCG_APLL_PFD0_CLK:
	case SCG_APLL_PFD1_CLK:
	case SCG_APLL_PFD2_CLK:
	case SCG_APLL_PFD3_CLK:
		return scg_apll_pfd_get_rate(clk);

	case SCG_DDR_CLK:
		return scg_ddr_get_rate();

	case SCG_NIC0_CLK:
	case SCG_GPU_CLK:
	case SCG_NIC1_CLK:
	case SCG_NIC1_BUS_CLK:
	case SCG_NIC1_EXT_CLK:
		return scg_nic_get_rate(clk);

	case USB_PLL_OUT:
		return decode_pll(PLL_USB);

	case MIPI_PLL_OUT:
		return decode_pll(PLL_MIPI);

	case SCG_SOSC_CLK:
	case SCG_FIRC_CLK:
	case SCG_SIRC_CLK:
	case SCG_ROSC_CLK:
		return scg_src_get_rate(clk);
	default:
		return 0;
	}
}

int scg_enable_pll_pfd(enum scg_clk clk, u32 frac)
{
	u32 reg;
	u32 shift, mask, gate, valid;
	u32 addr;

	if (frac < 12 || frac > 35)
		return -EINVAL;

	switch (clk) {
	case SCG_SPLL_PFD0_CLK:
	case SCG_APLL_PFD0_CLK:
		gate = SCG_PLL_PFD0_GATE_MASK;
		valid = SCG_PLL_PFD0_VALID_MASK;
		mask = SCG_PLL_PFD0_FRAC_MASK;
		shift = SCG_PLL_PFD0_FRAC_SHIFT;

		if (clk == SCG_SPLL_PFD0_CLK)
			addr = (u32)(&scg1_regs->spllpfd);
		else
			addr = (u32)(&scg1_regs->apllpfd);
		break;
	case SCG_SPLL_PFD1_CLK:
	case SCG_APLL_PFD1_CLK:
		gate = SCG_PLL_PFD1_GATE_MASK;
		valid = SCG_PLL_PFD1_VALID_MASK;
		mask = SCG_PLL_PFD1_FRAC_MASK;
		shift = SCG_PLL_PFD1_FRAC_SHIFT;

		if (clk == SCG_SPLL_PFD1_CLK)
			addr = (u32)(&scg1_regs->spllpfd);
		else
			addr = (u32)(&scg1_regs->apllpfd);
		break;
	case SCG_SPLL_PFD2_CLK:
	case SCG_APLL_PFD2_CLK:
		gate = SCG_PLL_PFD2_GATE_MASK;
		valid = SCG_PLL_PFD2_VALID_MASK;
		mask = SCG_PLL_PFD2_FRAC_MASK;
		shift = SCG_PLL_PFD2_FRAC_SHIFT;

		if (clk == SCG_SPLL_PFD2_CLK)
			addr = (u32)(&scg1_regs->spllpfd);
		else
			addr = (u32)(&scg1_regs->apllpfd);
		break;
	case SCG_SPLL_PFD3_CLK:
	case SCG_APLL_PFD3_CLK:
		gate = SCG_PLL_PFD3_GATE_MASK;
		valid = SCG_PLL_PFD3_VALID_MASK;
		mask = SCG_PLL_PFD3_FRAC_MASK;
		shift = SCG_PLL_PFD3_FRAC_SHIFT;

		if (clk == SCG_SPLL_PFD3_CLK)
			addr = (u32)(&scg1_regs->spllpfd);
		else
			addr = (u32)(&scg1_regs->apllpfd);
		break;
	default:
		return -EINVAL;
	}

	/* Gate the PFD */
	reg = readl(addr);
	reg |= gate;
	writel(reg, addr);

	/* Write Frac divider */
	reg &= ~mask;
	reg |= (frac << shift) & mask;
	writel(reg, addr);

	/*
	 * Un-gate the PFD
	 * (Need un-gate before checking valid, not align with RM)
	 */
	reg &= ~gate;
	writel(reg, addr);

	/* Wait for PFD clock being valid */
	do {
		reg = readl(addr);
	} while (!(reg & valid));

	return 0;
}

#define SIM_MISC_CTRL0_USB_PLL_EN_MASK (0x1 << 2)
int scg_enable_usb_pll(bool usb_control)
{
	u32 sosc_rate;
	s32 timeout = 1000000;
	u32 reg;

	struct usbphy_regs *usbphy =
		(struct usbphy_regs *)USBPHY_RBASE;

	sosc_rate = scg_src_get_rate(SCG_SOSC_CLK);
	if (!sosc_rate)
		return -EPERM;

	reg = readl(SIM0_RBASE + 0x3C);
	if (usb_control)
		reg &= ~SIM_MISC_CTRL0_USB_PLL_EN_MASK;
	else
		reg |= SIM_MISC_CTRL0_USB_PLL_EN_MASK;
	writel(reg, SIM0_RBASE + 0x3C);

	if (!(readl(&usbphy->usb1_pll_480_ctrl) & PLL_USB_LOCK_MASK)) {
		writel(0x1c00000, &usbphy->usb1_pll_480_ctrl_clr);

		switch (sosc_rate) {
		case 24000000:
			writel(0xc00000, &usbphy->usb1_pll_480_ctrl_set);
			break;

		case 30000000:
			writel(0x800000, &usbphy->usb1_pll_480_ctrl_set);
			break;

		case 19200000:
			writel(0x1400000, &usbphy->usb1_pll_480_ctrl_set);
			break;

		default:
			writel(0xc00000, &usbphy->usb1_pll_480_ctrl_set);
			break;
		}

		/* Enable the regulator first */
		writel(PLL_USB_REG_ENABLE_MASK,
		       &usbphy->usb1_pll_480_ctrl_set);

		/* Wait at least 15us */
		udelay(15);

		/* Enable the power */
		writel(PLL_USB_PWR_MASK, &usbphy->usb1_pll_480_ctrl_set);

		/* Wait lock */
		while (timeout--) {
			if (readl(&usbphy->usb1_pll_480_ctrl) &
			    PLL_USB_LOCK_MASK)
				break;
		}

		if (timeout <= 0) {
			/* If timeout, we power down the pll */
			writel(PLL_USB_PWR_MASK,
			       &usbphy->usb1_pll_480_ctrl_clr);
			return -ETIME;
		}
	}

	/* Clear the bypass */
	writel(PLL_USB_BYPASS_MASK, &usbphy->usb1_pll_480_ctrl_clr);

	/* Enable the PLL clock out to USB */
	writel((PLL_USB_EN_USB_CLKS_MASK | PLL_USB_ENABLE_MASK),
	       &usbphy->usb1_pll_480_ctrl_set);

	if (!usb_control) {
		while (timeout--) {
			if (readl(&scg1_regs->upllcsr) &
			    SCG_UPLL_CSR_UPLLVLD_MASK)
				break;
		}

		if (timeout <= 0) {
			reg = readl(SIM0_RBASE + 0x3C);
			reg &= ~SIM_MISC_CTRL0_USB_PLL_EN_MASK;
			writel(reg, SIM0_RBASE + 0x3C);
			return -ETIME;
		}
	}

	return 0;
}


/* A7 domain system clock source is SPLL */
#define SCG1_RCCR_SCS_NUM	((SCG_SCS_SYS_PLL) << SCG_CCR_SCS_SHIFT)

/* A7 Core clck = SPLL PFD0 / 1 = 500MHz / 1 = 500MHz */
#define SCG1_RCCR_DIVCORE_NUM	((0x0)  << SCG_CCR_DIVCORE_SHIFT)
#define SCG1_RCCR_CFG_MASK	(SCG_CCR_SCS_MASK | SCG_CCR_DIVBUS_MASK)

/* A7 Plat clck = A7 Core Clock / 2 = 250MHz / 1 = 250MHz */
#define SCG1_RCCR_DIVBUS_NUM	((0x1)  << SCG_CCR_DIVBUS_SHIFT)
#define SCG1_RCCR_CFG_NUM	(SCG1_RCCR_SCS_NUM | SCG1_RCCR_DIVBUS_NUM)

void scg_a7_rccr_init(void)
{
	u32 rccr_reg_val = 0;

	rccr_reg_val = readl(&scg1_regs->rccr);

	rccr_reg_val &= (~SCG1_RCCR_CFG_MASK);
	rccr_reg_val |= (SCG1_RCCR_CFG_NUM);

	writel(rccr_reg_val, &scg1_regs->rccr);
}

/* POSTDIV2 = 1 */
#define SCG1_SPLL_CFG_POSTDIV2_NUM	((0x0)  << SCG_PLL_CFG_POSTDIV2_SHIFT)
/* POSTDIV1 = 1 */
#define SCG1_SPLL_CFG_POSTDIV1_NUM	((0x0)  << SCG_PLL_CFG_POSTDIV1_SHIFT)

/* MULT = 22 */
#define SCG1_SPLL_CFG_MULT_NUM		((22)   << SCG_PLL_CFG_MULT_SHIFT)

/* PFD0 output clock selected */
#define SCG1_SPLL_CFG_PFDSEL_NUM	((0) << SCG_PLL_CFG_PFDSEL_SHIFT)
/* PREDIV = 1 */
#define SCG1_SPLL_CFG_PREDIV_NUM	((0x0)  << SCG_PLL_CFG_PREDIV_SHIFT)
/* SPLL output clocks (including PFD outputs) selected */
#define SCG1_SPLL_CFG_BYPASS_NUM	((0x0)  << SCG_PLL_CFG_BYPASS_SHIFT)
/* SPLL PFD output clock selected */
#define SCG1_SPLL_CFG_PLLSEL_NUM	((0x1)  << SCG_PLL_CFG_PLLSEL_SHIFT)
/* Clock source is System OSC */
#define SCG1_SPLL_CFG_CLKSRC_NUM	((0x0)  << SCG_PLL_CFG_CLKSRC_SHIFT)
#define SCG1_SPLL_CFG_NUM_24M_OSC	(SCG1_SPLL_CFG_POSTDIV2_NUM	| \
					 SCG1_SPLL_CFG_POSTDIV1_NUM     | \
					 (22 << SCG_PLL_CFG_MULT_SHIFT) | \
					 SCG1_SPLL_CFG_PFDSEL_NUM       | \
					 SCG1_SPLL_CFG_PREDIV_NUM       | \
					 SCG1_SPLL_CFG_BYPASS_NUM       | \
					 SCG1_SPLL_CFG_PLLSEL_NUM       | \
					 SCG1_SPLL_CFG_CLKSRC_NUM)
/*413Mhz = A7 SPLL(528MHz) * 18/23 */
#define SCG1_SPLL_PFD0_FRAC_NUM		((23) << SCG_PLL_PFD0_FRAC_SHIFT)

void scg_a7_spll_init(void)
{
	u32 val = 0;

	/* Disable A7 System PLL */
	val = readl(&scg1_regs->spllcsr);
	val &= ~SCG_SPLL_CSR_SPLLEN_MASK;
	writel(val, &scg1_regs->spllcsr);

	/*
	 * Per block guide,
	 * "When changing PFD values, it is recommneded PFDx clock
	 * gets gated first by writing a value of 1 to PFDx_CLKGATE register,
	 * then program the new PFD value, then poll the PFDx_VALID
	 * flag to set before writing a value of 0 to PFDx_CLKGATE
	 * to ungate the PFDx clock and allow PFDx clock to run"
	 */

	/* Gate off A7 SPLL PFD0 ~ PDF4  */
	val = readl(&scg1_regs->spllpfd);
	val |= (SCG_PLL_PFD3_GATE_MASK |
			SCG_PLL_PFD2_GATE_MASK |
			SCG_PLL_PFD1_GATE_MASK |
			SCG_PLL_PFD0_GATE_MASK);
	writel(val, &scg1_regs->spllpfd);

	/* ================ A7 SPLL Configuration Start ============== */

	/* Configure A7 System PLL */
	writel(SCG1_SPLL_CFG_NUM_24M_OSC, &scg1_regs->spllcfg);

	/* Enable A7 System PLL */
	val = readl(&scg1_regs->spllcsr);
	val |= SCG_SPLL_CSR_SPLLEN_MASK;
	writel(val, &scg1_regs->spllcsr);

	/* Wait for A7 SPLL clock ready */
	while (!(readl(&scg1_regs->spllcsr) & SCG_SPLL_CSR_SPLLVLD_MASK))
		;

	/* Configure A7 SPLL PFD0 */
	val = readl(&scg1_regs->spllpfd);
	val &= ~SCG_PLL_PFD0_FRAC_MASK;
	val |= SCG1_SPLL_PFD0_FRAC_NUM;
	writel(val, &scg1_regs->spllpfd);

	/* Un-gate A7 SPLL PFD0 */
	val = readl(&scg1_regs->spllpfd);
	val &= ~SCG_PLL_PFD0_GATE_MASK;
	writel(val, &scg1_regs->spllpfd);

	/* Wait for A7 SPLL PFD0 clock being valid */
	while (!(readl(&scg1_regs->spllpfd) & SCG_PLL_PFD0_VALID_MASK))
		;

	/* ================ A7 SPLL Configuration End ============== */
}

/* DDR clock source is APLL PFD0 (396MHz) */
#define SCG1_DDRCCR_DDRCS_NUM		((0x0) << SCG_DDRCCR_DDRCS_SHIFT)
/* DDR clock = APLL PFD0 / 1 = 396MHz / 1 = 396MHz */
#define SCG1_DDRCCR_DDRDIV_NUM		((0x1) << SCG_DDRCCR_DDRDIV_SHIFT)
/* DDR clock = APLL PFD0 / 2 = 396MHz / 2 = 198MHz */
#define SCG1_DDRCCR_DDRDIV_LF_NUM	((0x2) << SCG_DDRCCR_DDRDIV_SHIFT)
#define SCG1_DDRCCR_CFG_NUM		(SCG1_DDRCCR_DDRCS_NUM  | \
					 SCG1_DDRCCR_DDRDIV_NUM)
#define SCG1_DDRCCR_CFG_LF_NUM		(SCG1_DDRCCR_DDRCS_NUM  | \
					 SCG1_DDRCCR_DDRDIV_LF_NUM)
void scg_a7_ddrclk_init(void)
{
	writel(SCG1_DDRCCR_CFG_NUM, &scg1_regs->ddrccr);
}

/* SCG1(A7) APLLCFG configurations */
/* divide by 1 <<28 */
#define SCG1_APLL_CFG_POSTDIV2_NUM      ((0x0) << SCG_PLL_CFG_POSTDIV2_SHIFT)
/* divide by 1 <<24 */
#define SCG1_APLL_CFG_POSTDIV1_NUM      ((0x0) << SCG_PLL_CFG_POSTDIV1_SHIFT)
/* MULT is 22  <<16 */
#define SCG1_APLL_CFG_MULT_NUM          ((22)  << SCG_PLL_CFG_MULT_SHIFT)
/* PFD0 output clock selected  <<14 */
#define SCG1_APLL_CFG_PFDSEL_NUM        ((0) << SCG_PLL_CFG_PFDSEL_SHIFT)
/* PREDIV = 1	<<8 */
#define SCG1_APLL_CFG_PREDIV_NUM        ((0x0) << SCG_PLL_CFG_PREDIV_SHIFT)
/* APLL output clocks (including PFD outputs) selected	<<2 */
#define SCG1_APLL_CFG_BYPASS_NUM        ((0x0) << SCG_PLL_CFG_BYPASS_SHIFT)
/* APLL PFD output clock selected <<1 */
#define SCG1_APLL_CFG_PLLSEL_NUM        ((0x0) << SCG_PLL_CFG_PLLSEL_SHIFT)
/* Clock source is System OSC <<0 */
#define SCG1_APLL_CFG_CLKSRC_NUM        ((0x0) << SCG_PLL_CFG_CLKSRC_SHIFT)

/* SCG1(A7) FIRC DIV configurations */
/* Disable FIRC DIV3 */
#define SCG1_FIRCDIV_DIV3_NUM           ((0x0) << SCG_FIRCDIV_DIV3_SHIFT)
/* FIRC DIV2 = 48MHz / 1 = 48MHz */
#define SCG1_FIRCDIV_DIV2_NUM           ((0x1) << SCG_FIRCDIV_DIV2_SHIFT)
/* Disable FIRC DIV1 */
#define SCG1_FIRCDIV_DIV1_NUM           ((0x0) << SCG_FIRCDIV_DIV1_SHIFT)

void scg_a7_firc_init(void)
{
	/* Wait for FIRC clock ready */
	while (!(readl(&scg1_regs->firccsr) & SCG_FIRC_CSR_FIRCVLD_MASK))
		;

	/* Configure A7 FIRC DIV1 ~ DIV3 */
	writel((SCG1_FIRCDIV_DIV3_NUM |
			SCG1_FIRCDIV_DIV2_NUM |
			SCG1_FIRCDIV_DIV1_NUM), &scg1_regs->fircdiv);
}

/* SCG1(A7) NICCCR configurations */
/* NIC clock source is DDR clock (396/198MHz) */
#define SCG1_NICCCR_NICCS_NUM		((0x1) << SCG_NICCCR_NICCS_SHIFT)

/* NIC0 clock = DDR Clock / 2 = 396MHz / 2 = 198MHz */
#define SCG1_NICCCR_NIC0_DIV_NUM	((0x1) << SCG_NICCCR_NIC0_DIV_SHIFT)
/* NIC0 clock = DDR Clock / 1 = 198MHz / 1 = 198MHz */
#define SCG1_NICCCR_NIC0_DIV_LF_NUM	((0x0) << SCG_NICCCR_NIC0_DIV_SHIFT)
/* NIC1 clock = NIC0 Clock / 1 = 198MHz / 2 = 198MHz */
#define SCG1_NICCCR_NIC1_DIV_NUM	((0x0) << SCG_NICCCR_NIC1_DIV_SHIFT)
/* NIC1 bus clock = NIC1 Clock / 3 = 198MHz / 3 = 66MHz */
#define SCG1_NICCCR_NIC1_DIVBUS_NUM	((0x2) << SCG_NICCCR_NIC1_DIVBUS_SHIFT)
#define SCG1_NICCCR_CFG_NUM		(SCG1_NICCCR_NICCS_NUM      | \
					 SCG1_NICCCR_NIC0_DIV_NUM   | \
					 SCG1_NICCCR_NIC1_DIV_NUM   | \
					 SCG1_NICCCR_NIC1_DIVBUS_NUM)

void scg_a7_nicclk_init(void)
{
	writel(SCG1_NICCCR_CFG_NUM, &scg1_regs->nicccr);
}

/* SCG1(A7) FIRC DIV configurations */
/* Enable FIRC DIV3 */
#define SCG1_SOSCDIV_DIV3_NUM		((0x1) << SCG_SOSCDIV_DIV3_SHIFT)
/* FIRC DIV2 = 48MHz / 1 = 48MHz */
#define SCG1_SOSCDIV_DIV2_NUM		((0x1) << SCG_SOSCDIV_DIV2_SHIFT)
/* Enable FIRC DIV1 */
#define SCG1_SOSCDIV_DIV1_NUM		((0x1) << SCG_SOSCDIV_DIV1_SHIFT)

void scg_a7_soscdiv_init(void)
{
	/* Wait for FIRC clock ready */
	while (!(readl(&scg1_regs->sosccsr) & SCG_SOSC_CSR_SOSCVLD_MASK))
		;

	/* Configure A7 FIRC DIV1 ~ DIV3 */
	writel((SCG1_SOSCDIV_DIV3_NUM | SCG1_SOSCDIV_DIV2_NUM |
	       SCG1_SOSCDIV_DIV1_NUM), &scg1_regs->soscdiv);
}

void scg_a7_sys_clk_sel(enum scg_sys_src clk)
{
	u32 rccr_reg_val = 0;

	clk_debug("%s: system clock selected as %s\n", "[SCG]",
		  clk == SCG_SCS_SYS_OSC ? "SYS_OSC" :
		  clk == SCG_SCS_SLOW_IRC  ? "SLOW_IRC" :
		  clk == SCG_SCS_FAST_IRC  ? "FAST_IRC" :
		  clk == SCG_SCS_RTC_OSC   ? "RTC_OSC" :
		  clk == SCG_SCS_AUX_PLL   ? "AUX_PLL" :
		  clk == SCG_SCS_SYS_PLL   ? "SYS_PLL" :
		  clk == SCG_SCS_USBPHY_PLL ? "USBPHY_PLL" :
		  "Invalid source"
	);

	rccr_reg_val = readl(&scg1_regs->rccr);
	rccr_reg_val &= ~SCG_CCR_SCS_MASK;
	rccr_reg_val |= (clk << SCG_CCR_SCS_SHIFT);
	writel(rccr_reg_val, &scg1_regs->rccr);
}

void scg_a7_info(void)
{
	debug("SCG Version: 0x%x\n", readl(&scg1_regs->verid));
	debug("SCG Parameter: 0x%x\n", readl(&scg1_regs->param));
	debug("SCG RCCR Value: 0x%x\n", readl(&scg1_regs->rccr));
	debug("SCG Clock Status: 0x%x\n", readl(&scg1_regs->csr));
}

void scg_a7_init_core_clk(void)
{
	u32 val = 0;

	/*
	 * The normal target frequency for ULP B0 is 500Mhz,
	 * but ROM set it to 413Mhz, need to change SPLL PFD0 FRAC
	 */
	if (soc_rev() >= CHIP_REV_2_0) {
		/* Switch RCCR SCG to SOSC, firstly check the SOSC is valid */
		if ((readl(&scg1_regs->sosccsr) & SCG_SOSC_CSR_SOSCVLD_MASK)) {
			val = readl(&scg1_regs->rccr);
			val &= (~SCG_CCR_SCS_MASK);
			val |= ((SCG_SCS_SYS_OSC) << SCG_CCR_SCS_SHIFT);
			writel(val, &scg1_regs->rccr);

			/* Switch the PLLS to SPLL clk */
			val = readl(&scg1_regs->spllcfg);
			val &= ~SCG_PLL_CFG_PLLSEL_MASK;
			writel(val, &scg1_regs->spllcfg);

			/*
			 * Re-configure PFD0 to 19,
			 * A7 SPLL(528MHz) * 18 / 19 = 500MHz
			 */
			scg_enable_pll_pfd(SCG_SPLL_PFD0_CLK, 19);

			/* Switch the PLLS to SPLL PFD0 */
			val = readl(&scg1_regs->spllcfg);
			val |= SCG_PLL_CFG_PLLSEL_MASK;
			writel(val, &scg1_regs->spllcfg);

			/* Set RCCR SCG to SPLL clk out */
			val = readl(&scg1_regs->rccr);
			val &= (~SCG_CCR_SCS_MASK);
			val |= ((SCG_SCS_SYS_PLL) << SCG_CCR_SCS_SHIFT);
			writel(val, &scg1_regs->rccr);
		}
	}
}
