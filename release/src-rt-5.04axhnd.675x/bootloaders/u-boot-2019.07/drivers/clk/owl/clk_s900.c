// SPDX-License-Identifier: GPL-2.0+
/*
 * Actions Semi S900 clock driver
 *
 * Copyright (C) 2015 Actions Semi Co., Ltd.
 * Copyright (C) 2018 Manivannan Sadhasivam <manivannan.sadhasivam@linaro.org>
 */

#include <common.h>
#include <dm.h>
#include <asm/arch-owl/clk_s900.h>
#include <asm/arch-owl/regs_s900.h>
#include <asm/io.h>

#include <dt-bindings/clock/s900_cmu.h>

void owl_clk_init(struct owl_clk_priv *priv)
{
	u32 bus_clk = 0, core_pll, dev_pll;

	/* Enable ASSIST_PLL */
	setbits_le32(priv->base + CMU_ASSISTPLL, BIT(0));

	udelay(PLL_STABILITY_WAIT_US);

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

void owl_uart_clk_enable(struct owl_clk_priv *priv)
{
	/* Source HOSC for UART5 interface */
	clrbits_le32(priv->base + CMU_UART5CLK, CMU_UARTCLK_SRC_DEVPLL);

	/* Enable UART5 interface clock */
	setbits_le32(priv->base + CMU_DEVCLKEN1, CMU_DEVCLKEN1_UART5);
}

void owl_uart_clk_disable(struct owl_clk_priv *priv)
{
	/* Disable UART5 interface clock */
	clrbits_le32(priv->base + CMU_DEVCLKEN1, CMU_DEVCLKEN1_UART5);
}

int owl_clk_enable(struct clk *clk)
{
	struct owl_clk_priv *priv = dev_get_priv(clk->dev);

	switch (clk->id) {
	case CLOCK_UART5:
		owl_uart_clk_enable(priv);
		break;
	default:
		return 0;
	}

	return 0;
}

int owl_clk_disable(struct clk *clk)
{
	struct owl_clk_priv *priv = dev_get_priv(clk->dev);

	switch (clk->id) {
	case CLOCK_UART5:
		owl_uart_clk_disable(priv);
		break;
	default:
		return 0;
	}

	return 0;
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

static struct clk_ops owl_clk_ops = {
	.enable = owl_clk_enable,
	.disable = owl_clk_disable,
};

static const struct udevice_id owl_clk_ids[] = {
	{ .compatible = "actions,s900-cmu" },
	{ }
};

U_BOOT_DRIVER(clk_owl) = {
	.name		= "clk_s900",
	.id		= UCLASS_CLK,
	.of_match	= owl_clk_ids,
	.ops		= &owl_clk_ops,
	.priv_auto_alloc_size = sizeof(struct owl_clk_priv),
	.probe		= owl_clk_probe,
};
