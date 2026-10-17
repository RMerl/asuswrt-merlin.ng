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

#define PCC_CLKSRC_TYPES 2
#define PCC_CLKSRC_NUM 7

static enum scg_clk pcc_clksrc[PCC_CLKSRC_TYPES][PCC_CLKSRC_NUM] = {
	{	SCG_NIC1_BUS_CLK,
		SCG_NIC1_CLK,
		SCG_DDR_CLK,
		SCG_APLL_PFD2_CLK,
		SCG_APLL_PFD1_CLK,
		SCG_APLL_PFD0_CLK,
		USB_PLL_OUT,
	},
	{	SCG_SOSC_DIV2_CLK,  /* SOSC BUS clock */
		MIPI_PLL_OUT,
		SCG_FIRC_DIV2_CLK,  /* FIRC BUS clock */
		SCG_ROSC_CLK,
		SCG_NIC1_BUS_CLK,
		SCG_NIC1_CLK,
		SCG_APLL_PFD3_CLK,
	},
};

static struct pcc_entry pcc_arrays[] = {
	{PCC2_RBASE, DMA1_PCC2_SLOT,		CLKSRC_NO_PCS, PCC_NO_DIV},
	{PCC2_RBASE, RGPIO1_PCC2_SLOT,		CLKSRC_NO_PCS, PCC_NO_DIV},
	{PCC2_RBASE, FLEXBUS0_PCC2_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV},
	{PCC2_RBASE, SEMA42_1_PCC2_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV},
	{PCC2_RBASE, DMA1_CH_MUX0_PCC2_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV},
	{PCC2_RBASE, SNVS_PCC2_SLOT,		CLKSRC_NO_PCS, PCC_NO_DIV},
	{PCC2_RBASE, CAAM_PCC2_SLOT,		CLKSRC_NO_PCS, PCC_NO_DIV},
	{PCC2_RBASE, LPTPM4_PCC2_SLOT,		CLKSRC_PER_BUS, PCC_NO_DIV},
	{PCC2_RBASE, LPTPM5_PCC2_SLOT,		CLKSRC_PER_BUS, PCC_NO_DIV},
	{PCC2_RBASE, LPIT1_PCC2_SLOT,		CLKSRC_PER_BUS, PCC_NO_DIV},
	{PCC2_RBASE, LPSPI2_PCC2_SLOT,		CLKSRC_PER_BUS, PCC_NO_DIV},
	{PCC2_RBASE, LPSPI3_PCC2_SLOT,		CLKSRC_PER_BUS, PCC_NO_DIV},
	{PCC2_RBASE, LPI2C4_PCC2_SLOT,		CLKSRC_PER_BUS, PCC_NO_DIV},
	{PCC2_RBASE, LPI2C5_PCC2_SLOT,		CLKSRC_PER_BUS, PCC_NO_DIV},
	{PCC2_RBASE, LPUART4_PCC2_SLOT,		CLKSRC_PER_BUS, PCC_NO_DIV},
	{PCC2_RBASE, LPUART5_PCC2_SLOT,		CLKSRC_PER_BUS, PCC_NO_DIV},
	{PCC2_RBASE, FLEXIO1_PCC2_SLOT,		CLKSRC_PER_BUS, PCC_NO_DIV},
	{PCC2_RBASE, USBOTG0_PCC2_SLOT,		CLKSRC_PER_PLAT, PCC_HAS_DIV},
	{PCC2_RBASE, USBOTG1_PCC2_SLOT,		CLKSRC_PER_PLAT, PCC_HAS_DIV},
	{PCC2_RBASE, USBPHY_PCC2_SLOT,		CLKSRC_NO_PCS, PCC_NO_DIV},
	{PCC2_RBASE, USB_PL301_PCC2_SLOT,	CLKSRC_NO_PCS, PCC_NO_DIV},
	{PCC2_RBASE, USDHC0_PCC2_SLOT,		CLKSRC_PER_PLAT, PCC_HAS_DIV},
	{PCC2_RBASE, USDHC1_PCC2_SLOT,		CLKSRC_PER_PLAT, PCC_HAS_DIV},
	{PCC2_RBASE, WDG1_PCC2_SLOT,		CLKSRC_PER_BUS,	PCC_HAS_DIV},
	{PCC2_RBASE, WDG2_PCC2_SLOT,		CLKSRC_PER_BUS, PCC_HAS_DIV},

	{PCC3_RBASE, LPTPM6_PCC3_SLOT,		CLKSRC_PER_BUS, PCC_NO_DIV},
	{PCC3_RBASE, LPTPM7_PCC3_SLOT,		CLKSRC_PER_BUS, PCC_NO_DIV},
	{PCC3_RBASE, LPI2C6_PCC3_SLOT,		CLKSRC_PER_BUS, PCC_NO_DIV},
	{PCC3_RBASE, LPI2C7_PCC3_SLOT,		CLKSRC_PER_BUS, PCC_NO_DIV},
	{PCC3_RBASE, LPUART6_PCC3_SLOT,		CLKSRC_PER_BUS, PCC_NO_DIV},
	{PCC3_RBASE, LPUART7_PCC3_SLOT,		CLKSRC_PER_BUS, PCC_NO_DIV},
	{PCC3_RBASE, VIU0_PCC3_SLOT,		CLKSRC_NO_PCS, PCC_NO_DIV},
	{PCC3_RBASE, DSI0_PCC3_SLOT,		CLKSRC_PER_BUS, PCC_HAS_DIV},
	{PCC3_RBASE, LCDIF0_PCC3_SLOT,		CLKSRC_PER_PLAT, PCC_HAS_DIV},
	{PCC3_RBASE, MMDC0_PCC3_SLOT,		CLKSRC_NO_PCS, PCC_NO_DIV},
	{PCC3_RBASE, PORTC_PCC3_SLOT,		CLKSRC_NO_PCS, PCC_NO_DIV},
	{PCC3_RBASE, PORTD_PCC3_SLOT,		CLKSRC_NO_PCS, PCC_NO_DIV},
	{PCC3_RBASE, PORTE_PCC3_SLOT,		CLKSRC_NO_PCS, PCC_NO_DIV},
	{PCC3_RBASE, PORTF_PCC3_SLOT,		CLKSRC_NO_PCS, PCC_NO_DIV},
	{PCC3_RBASE, GPU3D_PCC3_SLOT,		CLKSRC_PER_PLAT, PCC_NO_DIV},
	{PCC3_RBASE, GPU2D_PCC3_SLOT,		CLKSRC_PER_PLAT, PCC_NO_DIV},
};

int pcc_clock_enable(enum pcc_clk clk, bool enable)
{
	u32 reg, val;

	if (clk >= ARRAY_SIZE(pcc_arrays))
		return -EINVAL;

	reg = pcc_arrays[clk].pcc_base + pcc_arrays[clk].pcc_slot * 4;

	val = readl(reg);

	clk_debug("pcc_clock_enable: clk %d, reg 0x%x, val 0x%x, enable %d\n",
		  clk, reg, val, enable);

	if (!(val & PCC_PR_MASK) || (val & PCC_INUSE_MASK))
		return -EPERM;

	if (enable)
		val |= PCC_CGC_MASK;
	else
		val &= ~PCC_CGC_MASK;

	writel(val, reg);

	clk_debug("pcc_clock_enable: val 0x%x\n", val);

	return 0;
}

/* The clock source select needs clock is disabled */
int pcc_clock_sel(enum pcc_clk clk, enum scg_clk src)
{
	u32 reg, val, i, clksrc_type;

	if (clk >= ARRAY_SIZE(pcc_arrays))
		return -EINVAL;

	clksrc_type = pcc_arrays[clk].clksrc;
	if (clksrc_type >= CLKSRC_NO_PCS) {
		printf("No PCS field for the PCC %d, clksrc type %d\n",
		       clk, clksrc_type);
		return -EPERM;
	}

	for (i = 0; i < PCC_CLKSRC_NUM; i++) {
		if (pcc_clksrc[clksrc_type][i] == src) {
			/* Find the clock src, then set it to PCS */
			break;
		}
	}

	if (i == PCC_CLKSRC_NUM) {
		printf("Not find the parent scg_clk in PCS of PCC %d, invalid scg_clk %d\n", clk, src);
		return -EINVAL;
	}

	reg = pcc_arrays[clk].pcc_base + pcc_arrays[clk].pcc_slot * 4;

	val = readl(reg);

	clk_debug("pcc_clock_sel: clk %d, reg 0x%x, val 0x%x, clksrc_type %d\n",
		  clk, reg, val, clksrc_type);

	if (!(val & PCC_PR_MASK) || (val & PCC_INUSE_MASK) ||
	    (val & PCC_CGC_MASK)) {
		printf("Not permit to select clock source val = 0x%x\n", val);
		return -EPERM;
	}

	val &= ~PCC_PCS_MASK;
	val |= ((i + 1) << PCC_PCS_OFFSET);

	writel(val, reg);

	clk_debug("pcc_clock_sel: val 0x%x\n", val);

	return 0;
}

int pcc_clock_div_config(enum pcc_clk clk, bool frac, u8 div)
{
	u32 reg, val;

	if (clk >= ARRAY_SIZE(pcc_arrays) || div > 8 ||
	    (div == 1 && frac != 0))
		return -EINVAL;

	if (pcc_arrays[clk].div >= PCC_NO_DIV) {
		printf("No DIV/FRAC field for the PCC %d\n", clk);
		return -EPERM;
	}

	reg = pcc_arrays[clk].pcc_base + pcc_arrays[clk].pcc_slot * 4;

	val = readl(reg);

	if (!(val & PCC_PR_MASK) || (val & PCC_INUSE_MASK) ||
	    (val & PCC_CGC_MASK)) {
		printf("Not permit to set div/frac val = 0x%x\n", val);
		return -EPERM;
	}

	if (frac)
		val |= PCC_FRAC_MASK;
	else
		val &= ~PCC_FRAC_MASK;

	val &= ~PCC_PCD_MASK;
	val |= (div - 1) & PCC_PCD_MASK;

	writel(val, reg);

	return 0;
}

bool pcc_clock_is_enable(enum pcc_clk clk)
{
	u32 reg, val;

	if (clk >= ARRAY_SIZE(pcc_arrays))
		return -EINVAL;

	reg = pcc_arrays[clk].pcc_base + pcc_arrays[clk].pcc_slot * 4;
	val = readl(reg);

	if ((val & PCC_INUSE_MASK) || (val & PCC_CGC_MASK))
		return true;

	return false;
}

int pcc_clock_get_clksrc(enum pcc_clk clk, enum scg_clk *src)
{
	u32 reg, val, clksrc_type;

	if (clk >= ARRAY_SIZE(pcc_arrays))
		return -EINVAL;

	clksrc_type = pcc_arrays[clk].clksrc;
	if (clksrc_type >= CLKSRC_NO_PCS) {
		printf("No PCS field for the PCC %d, clksrc type %d\n",
		       clk, clksrc_type);
		return -EPERM;
	}

	reg = pcc_arrays[clk].pcc_base + pcc_arrays[clk].pcc_slot * 4;

	val = readl(reg);

	clk_debug("pcc_clock_get_clksrc: clk %d, reg 0x%x, val 0x%x, type %d\n",
		  clk, reg, val, clksrc_type);

	if (!(val & PCC_PR_MASK)) {
		printf("This pcc slot is not present = 0x%x\n", val);
		return -EPERM;
	}

	val &= PCC_PCS_MASK;
	val = (val >> PCC_PCS_OFFSET);

	if (!val) {
		printf("Clock source is off\n");
		return -EIO;
	}

	*src = pcc_clksrc[clksrc_type][val - 1];

	clk_debug("pcc_clock_get_clksrc: parent scg clk %d\n", *src);

	return 0;
}

u32 pcc_clock_get_rate(enum pcc_clk clk)
{
	u32 reg, val, rate, frac, div;
	enum scg_clk parent;
	int ret;

	ret = pcc_clock_get_clksrc(clk, &parent);
	if (ret)
		return 0;

	rate = scg_clk_get_rate(parent);

	clk_debug("pcc_clock_get_rate: parent rate %u\n", rate);

	if (pcc_arrays[clk].div == PCC_HAS_DIV) {
		reg = pcc_arrays[clk].pcc_base + pcc_arrays[clk].pcc_slot * 4;
		val = readl(reg);

		frac = (val & PCC_FRAC_MASK) >> PCC_FRAC_OFFSET;
		div = (val & PCC_PCD_MASK) >> PCC_PCD_OFFSET;

		/*
		 * Theoretically don't have overflow in the calc,
		 * the rate won't exceed 2G
		 */
		rate = rate * (frac + 1) / (div + 1);
	}

	clk_debug("pcc_clock_get_rate: rate %u\n", rate);
	return rate;
}
