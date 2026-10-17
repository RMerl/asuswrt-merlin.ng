// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2022 Nuvoton Technology Corp.
 */

#include <dm.h>
#include <dt-bindings/clock/nuvoton,npcm845-clk.h>
#include "clk_npcm.h"

/* Parent clock map */
static const struct parent_data pll_parents[] = {
	{NPCM8XX_CLK_PLL0, 0},
	{NPCM8XX_CLK_PLL1, 1},
	{NPCM8XX_CLK_REFCLK, 2},
	{NPCM8XX_CLK_PLL2DIV2, 3}
};

static const struct parent_data cpuck_parents[] = {
	{NPCM8XX_CLK_PLL0, 0},
	{NPCM8XX_CLK_PLL1, 1},
	{NPCM8XX_CLK_REFCLK, 2},
	{NPCM8XX_CLK_PLL2, 7}
};

static const struct parent_data apb_parent[] = {{NPCM8XX_CLK_AHB, 0}};

static struct npcm_clk_pll npcm8xx_clk_plls[] = {
	{NPCM8XX_CLK_PLL0, NPCM8XX_CLK_REFCLK, PLLCON0, 0},
	{NPCM8XX_CLK_PLL1, NPCM8XX_CLK_REFCLK, PLLCON1, 0},
	{NPCM8XX_CLK_PLL2, NPCM8XX_CLK_REFCLK, PLLCON2, 0},
	{NPCM8XX_CLK_PLL2DIV2, NPCM8XX_CLK_REFCLK, PLLCON2, POST_DIV2}
};

static struct npcm_clk_select npcm8xx_clk_selectors[] = {
	{NPCM8XX_CLK_AHB, cpuck_parents, CLKSEL, NPCM8XX_CPUCKSEL, 4, 0},
	{NPCM8XX_CLK_APB2, apb_parent, 0, 0, 1, FIXED_PARENT},
	{NPCM8XX_CLK_APB5, apb_parent, 0, 0, 1, FIXED_PARENT},
	{NPCM8XX_CLK_SPI0, apb_parent, 0, 0, 1, FIXED_PARENT},
	{NPCM8XX_CLK_SPI1, apb_parent, 0, 0, 1, FIXED_PARENT},
	{NPCM8XX_CLK_SPI3, apb_parent, 0, 0, 1, FIXED_PARENT},
	{NPCM8XX_CLK_SPIX, apb_parent, 0, 0, 1, FIXED_PARENT},
	{NPCM8XX_CLK_UART, pll_parents, CLKSEL, UARTCKSEL, 4, 0},
	{NPCM8XX_CLK_UART2, pll_parents, CLKSEL, UARTCKSEL, 4, 0},
	{NPCM8XX_CLK_SDHC, pll_parents, CLKSEL, SDCKSEL, 4, 0}
};

static struct npcm_clk_div npcm8xx_clk_dividers[] = {
	{NPCM8XX_CLK_AHB, CLKDIV1, CLK4DIV, DIV_TYPE1 | PRE_DIV2 | DIV_RO},
	{NPCM8XX_CLK_APB2, CLKDIV2, APB2CKDIV, DIV_TYPE2 | DIV_RO},
	{NPCM8XX_CLK_APB5, CLKDIV2, APB5CKDIV, DIV_TYPE2 | DIV_RO},
	{NPCM8XX_CLK_SPI0, CLKDIV3, SPI0CKDIV, DIV_TYPE1 | DIV_RO},
	{NPCM8XX_CLK_SPI1, CLKDIV3, SPI1CKDIV, DIV_TYPE1 | DIV_RO},
	{NPCM8XX_CLK_SPI3, CLKDIV1, SPI3CKDIV, DIV_TYPE1 | DIV_RO},
	{NPCM8XX_CLK_SPIX, CLKDIV3, SPIXCKDIV, DIV_TYPE1},
	{NPCM8XX_CLK_UART, CLKDIV1, UARTDIV1, DIV_TYPE1},
	{NPCM8XX_CLK_UART2, CLKDIV3, UARTDIV2, DIV_TYPE1},
	{NPCM8XX_CLK_SDHC, CLKDIV1, MMCCKDIV, DIV_TYPE1}
};

static struct npcm_clk_data npcm8xx_clk_data = {
	.clk_plls = npcm8xx_clk_plls,
	.num_plls = ARRAY_SIZE(npcm8xx_clk_plls),
	.clk_selectors = npcm8xx_clk_selectors,
	.num_selectors = ARRAY_SIZE(npcm8xx_clk_selectors),
	.clk_dividers = npcm8xx_clk_dividers,
	.num_dividers = ARRAY_SIZE(npcm8xx_clk_dividers),
	.refclk_id = NPCM8XX_CLK_REFCLK,
	.pll0_id = NPCM8XX_CLK_PLL0,
};

static int npcm8xx_clk_probe(struct udevice *dev)
{
	struct npcm_clk_priv *priv = dev_get_priv(dev);

	priv->base = dev_read_addr_ptr(dev);
	if (!priv->base)
		return -EINVAL;

	priv->clk_data = &npcm8xx_clk_data;
	priv->num_clks = NPCM8XX_NUM_CLOCKS;

	return 0;
}

static const struct udevice_id npcm8xx_clk_ids[] = {
	{ .compatible = "nuvoton,npcm845-clk" },
	{ }
};

U_BOOT_DRIVER(clk_npcm) = {
	.name           = "clk_npcm",
	.id             = UCLASS_CLK,
	.of_match       = npcm8xx_clk_ids,
	.ops            = &npcm_clk_ops,
	.priv_auto	= sizeof(struct npcm_clk_priv),
	.probe          = npcm8xx_clk_probe,
	.flags = DM_FLAG_PRE_RELOC,
};
