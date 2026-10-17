// SPDX-License-Identifier: GPL-2.0+
/*
 * Common clock driver for Actions Semi SoCs.
 *
 * Copyright (C) 2015 Actions Semi Co., Ltd.
 * Copyright (C) 2018 Manivannan Sadhasivam <manivannan.sadhasivam@linaro.org>
 */

#include <common.h>
#include <dm.h>
#include "clk_owl.h"
#include <asm/io.h>
#if defined(CONFIG_MACH_S900)
#include <asm/arch-owl/regs_s900.h>
#include <dt-bindings/clock/actions,s900-cmu.h>
#elif defined(CONFIG_MACH_S700)
#include <asm/arch-owl/regs_s700.h>
#include <dt-bindings/clock/actions,s700-cmu.h>
#endif
#include <linux/bitops.h>
#include <linux/delay.h>

#define CMU_DEVCLKEN0_SD0	BIT(22)

void owl_clk_init(struct owl_clk_priv *priv)
{
	u32 bus_clk = 0, core_pll, dev_pll;

#if defined(CONFIG_MACH_S900)
	/* Enable ASSIST_PLL */
	setbits_le32(priv->base + CMU_ASSISTPLL, BIT(0));
	udelay(PLL_STABILITY_WAIT_US);
#endif

	/* Source HOSC to DEV_CLK */
	clrbits_le32(priv->base + CMU_DEVPLL, CMU_DEVPLL_CLK);

	/* Configure BUS_CLK */
	bus_clk |= (CMU_PDBGDIV_DIV | CMU_PERDIV_DIV | CMU_NOCDIV_DIV |
			CMU_DMMCLK_SRC | CMU_APBCLK_DIV | CMU_AHBCLK_DIV |
			CMU_NOCCLK_SRC | CMU_CORECLK_HOSC);
	writel(bus_clk, priv->base + CMU_BUSCLK);

	udelay(PLL_STABILITY_WAIT_US);

	/* Configure CORE_PLL */
	core_pll = readl(priv->base + CMU_COREPLL);
	core_pll |= (CMU_COREPLL_EN | CMU_COREPLL_HOSC_EN | CMU_COREPLL_OUT);
	writel(core_pll, priv->base + CMU_COREPLL);

	udelay(PLL_STABILITY_WAIT_US);

	/* Configure DEV_PLL */
	dev_pll = readl(priv->base + CMU_DEVPLL);
	dev_pll |= (CMU_DEVPLL_EN | CMU_DEVPLL_OUT);
	writel(dev_pll, priv->base + CMU_DEVPLL);

	udelay(PLL_STABILITY_WAIT_US);

	/* Source CORE_PLL for CORE_CLK */
	clrsetbits_le32(priv->base + CMU_BUSCLK, CMU_CORECLK_MASK,
			CMU_CORECLK_CPLL);

	/* Source DEV_PLL for DEV_CLK */
	setbits_le32(priv->base + CMU_DEVPLL, CMU_DEVPLL_CLK);

	udelay(PLL_STABILITY_WAIT_US);
}

int owl_clk_enable(struct clk *clk)
{
	struct owl_clk_priv *priv = dev_get_priv(clk->dev);
	enum owl_soc model = dev_get_driver_data(clk->dev);

	switch (clk->id) {
	case CLK_UART5:
		if (model != S900)
			return -EINVAL;
		/* Source HOSC for UART5 interface */
		clrbits_le32(priv->base + CMU_UART5CLK, CMU_UARTCLK_SRC_DEVPLL);
		/* Enable UART5 interface clock */
		setbits_le32(priv->base + CMU_DEVCLKEN1, CMU_DEVCLKEN1_UART5);
		break;
	case CLK_UART3:
		if (model != S700)
			return -EINVAL;
		/* Source HOSC for UART3 interface */
		clrbits_le32(priv->base + CMU_UART3CLK, CMU_UARTCLK_SRC_DEVPLL);
		/* Enable UART3 interface clock */
		setbits_le32(priv->base + CMU_DEVCLKEN1, CMU_DEVCLKEN1_UART3);
		break;
	case CLK_RMII_REF:
	case CLK_ETHERNET:
		setbits_le32(priv->base + CMU_DEVCLKEN1, CMU_DEVCLKEN1_ETH);
		setbits_le32(priv->base + CMU_ETHERNETPLL, 5);
		break;
	case CLK_SD0:
		setbits_le32(priv->base + CMU_DEVCLKEN0, CMU_DEVCLKEN0_SD0);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

int owl_clk_disable(struct clk *clk)
{
	struct owl_clk_priv *priv = dev_get_priv(clk->dev);
	enum owl_soc model = dev_get_driver_data(clk->dev);

	switch (clk->id) {
	case CLK_UART5:
		if (model != S900)
			return -EINVAL;
		/* Disable UART5 interface clock */
		clrbits_le32(priv->base + CMU_DEVCLKEN1, CMU_DEVCLKEN1_UART5);
		break;
	case CLK_UART3:
		if (model != S700)
			return -EINVAL;
		/* Disable UART3 interface clock */
		clrbits_le32(priv->base + CMU_DEVCLKEN1, CMU_DEVCLKEN1_UART3);
		break;
	case CLK_RMII_REF:
	case CLK_ETHERNET:
		clrbits_le32(priv->base + CMU_DEVCLKEN1, CMU_DEVCLKEN1_ETH);
		break;
	case CLK_SD0:
		clrbits_le32(priv->base + CMU_DEVCLKEN0, CMU_DEVCLKEN0_SD0);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static ulong get_sd_parent_rate(struct owl_clk_priv *priv, u32 dev_index)
{
	ulong rate;
	u32 reg;

	reg = readl(priv->base + (CMU_SD0CLK + dev_index * 0x4));
	/* Clock output of DEV/NAND_PLL
	 * Range: 48M ~ 756M
	 * Frequency= PLLCLK * 6
	 */
	if (reg & 0x200)
		rate = readl(priv->base + CMU_NANDPLL) & 0x7f;
	else
		rate = readl(priv->base + CMU_DEVPLL) & 0x7f;

	rate *= 6000000;

	return rate;
}

static ulong owl_get_sd_clk_rate(struct owl_clk_priv *priv, int sd_index)
{
	uint div, val;
	ulong parent_rate = get_sd_parent_rate(priv, sd_index);

	val = readl(priv->base + (CMU_SD0CLK + sd_index * 0x4));
	div = (val & 0x1f) + 1;

	return (parent_rate / div);
}

static ulong owl_set_sd_clk_rate(struct owl_clk_priv *priv, ulong rate,
				 int sd_index)
{
	uint div, val;
	ulong parent_rate = get_sd_parent_rate(priv, sd_index);

	if (rate == 0)
		return rate;

	div = (parent_rate / rate);

	val = readl(priv->base + (CMU_SD0CLK + sd_index * 0x4));
	/* Bits 4..0 is used to program div value and bit 8 to enable
	 * divide by 128 circuit
	 */
	val &= ~0x11f;
	if (div >= 128) {
		div = div / 128;
		val |= 0x100; /* enable divide by 128 circuit */
	}
	val |= ((div - 1) & 0x1f);
	writel(val, priv->base + (CMU_SD0CLK + sd_index * 0x4));

	return owl_get_sd_clk_rate(priv, 0);
}

static ulong owl_clk_get_rate(struct clk *clk)
{
	struct owl_clk_priv *priv = dev_get_priv(clk->dev);
	ulong rate;

	switch (clk->id) {
	case CLK_SD0:
		rate = owl_get_sd_clk_rate(priv, 0);
		break;
	default:
		return -ENOENT;
	}

	return rate;
}

static ulong owl_clk_set_rate(struct clk *clk, ulong rate)
{
	struct owl_clk_priv *priv = dev_get_priv(clk->dev);
	ulong new_rate;

	switch (clk->id) {
	case CLK_SD0:
		new_rate = owl_set_sd_clk_rate(priv, rate, 0);
		break;
	default:
		return -ENOENT;
	}

	return new_rate;
}

static int owl_clk_probe(struct udevice *dev)
{
	struct owl_clk_priv *priv = dev_get_priv(dev);

	priv->base = dev_read_addr(dev);
	if (priv->base == FDT_ADDR_T_NONE)
		return -EINVAL;

	/* setup necessary clocks */
	owl_clk_init(priv);

	return 0;
}

static const struct clk_ops owl_clk_ops = {
	.enable = owl_clk_enable,
	.disable = owl_clk_disable,
	.get_rate = owl_clk_get_rate,
	.set_rate = owl_clk_set_rate,
};

static const struct udevice_id owl_clk_ids[] = {
#if defined(CONFIG_MACH_S900)
	{ .compatible = "actions,s900-cmu", .data = S900 },
#elif defined(CONFIG_MACH_S700)
	{ .compatible = "actions,s700-cmu", .data = S700 },
#endif
	{ }
};

U_BOOT_DRIVER(clk_owl) = {
	.name		= "clk_owl",
	.id		= UCLASS_CLK,
	.of_match	= owl_clk_ids,
	.ops		= &owl_clk_ops,
	.priv_auto	= sizeof(struct owl_clk_priv),
	.probe		= owl_clk_probe,
};
