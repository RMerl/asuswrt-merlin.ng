// SPDX-License-Identifier: GPL-2.0+
/*
 * Marvell Armada 37xx SoC Time Base Generator clocks
 *
 * Marek Behún <kabel@kernel.org>
 *
 * Based on Linux driver by:
 *   Gregory CLEMENT <gregory.clement@free-electrons.com>
 */

#include <common.h>
#include <clk-uclass.h>
#include <clk.h>
#include <dm.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <dm/device_compat.h>

#define NUM_TBG	    4

#define TBG_CTRL0		0x4
#define TBG_CTRL1		0x8
#define TBG_CTRL7		0x20
#define TBG_CTRL8		0x30

#define TBG_DIV_MASK		0x1FF

#define TBG_A_REFDIV		0
#define TBG_B_REFDIV		16

#define TBG_A_FBDIV		2
#define TBG_B_FBDIV		18

#define TBG_A_VCODIV_SE		0
#define TBG_B_VCODIV_SE		16

#define TBG_A_VCODIV_DIFF	1
#define TBG_B_VCODIV_DIFF	17

struct tbg_def {
	const char *name;
	u32 refdiv_offset;
	u32 fbdiv_offset;
	u32 vcodiv_reg;
	u32 vcodiv_offset;
};

static const struct tbg_def tbg[NUM_TBG] = {
	{"TBG-A-P", TBG_A_REFDIV, TBG_A_FBDIV, TBG_CTRL8, TBG_A_VCODIV_DIFF},
	{"TBG-B-P", TBG_B_REFDIV, TBG_B_FBDIV, TBG_CTRL8, TBG_B_VCODIV_DIFF},
	{"TBG-A-S", TBG_A_REFDIV, TBG_A_FBDIV, TBG_CTRL1, TBG_A_VCODIV_SE},
	{"TBG-B-S", TBG_B_REFDIV, TBG_B_FBDIV, TBG_CTRL1, TBG_B_VCODIV_SE},
};

struct a37xx_tbgclk {
	ulong rates[NUM_TBG];
	unsigned int mult[NUM_TBG];
	unsigned int div[NUM_TBG];
};

static unsigned int tbg_get_mult(void __iomem *reg, const struct tbg_def *ptbg)
{
	u32 val;

	val = readl(reg + TBG_CTRL0);

	return ((val >> ptbg->fbdiv_offset) & TBG_DIV_MASK) << 2;
}

static unsigned int tbg_get_div(void __iomem *reg, const struct tbg_def *ptbg)
{
	u32 val;
	unsigned int div;

	val = readl(reg + TBG_CTRL7);

	div = (val >> ptbg->refdiv_offset) & TBG_DIV_MASK;
	if (div == 0)
		div = 1;
	val = readl(reg + ptbg->vcodiv_reg);

	div *= 1 << ((val >>  ptbg->vcodiv_offset) & TBG_DIV_MASK);

	return div;
}

static ulong armada_37xx_tbg_clk_get_rate(struct clk *clk)
{
	struct a37xx_tbgclk *priv = dev_get_priv(clk->dev);

	if (clk->id >= NUM_TBG)
		return -ENODEV;

	return priv->rates[clk->id];
}

#if defined(CONFIG_CMD_CLK) && defined(CONFIG_CLK_ARMADA_3720)
int armada_37xx_tbg_clk_dump(struct udevice *dev)
{
	struct a37xx_tbgclk *priv = dev_get_priv(dev);
	int i;

	for (i = 0; i < NUM_TBG; ++i)
		printf("  %s at %lu Hz\n", tbg[i].name,
		       priv->rates[i]);
	printf("\n");

	return 0;
}
#endif

static int armada_37xx_tbg_clk_probe(struct udevice *dev)
{
	struct a37xx_tbgclk *priv = dev_get_priv(dev);
	void __iomem *reg;
	ulong xtal;
	int i;

	reg = dev_read_addr_ptr(dev);
	if (!reg) {
		dev_err(dev, "no io address\n");
		return -ENODEV;
	}

	xtal = (ulong)get_ref_clk() * 1000000;

	for (i = 0; i < NUM_TBG; ++i) {
		unsigned int mult, div;

		mult = tbg_get_mult(reg, &tbg[i]);
		div = tbg_get_div(reg, &tbg[i]);

		priv->rates[i] = (xtal * mult) / div;
	}

	return 0;
}

static const struct clk_ops armada_37xx_tbg_clk_ops = {
	.get_rate = armada_37xx_tbg_clk_get_rate,
};

static const struct udevice_id armada_37xx_tbg_clk_ids[] = {
	{ .compatible = "marvell,armada-3700-tbg-clock" },
	{}
};

U_BOOT_DRIVER(armada_37xx_tbg_clk) = {
	.name		= "armada_37xx_tbg_clk",
	.id		= UCLASS_CLK,
	.of_match	= armada_37xx_tbg_clk_ids,
	.ops		= &armada_37xx_tbg_clk_ops,
	.priv_auto	= sizeof(struct a37xx_tbgclk),
	.probe		= armada_37xx_tbg_clk_probe,
	.flags		= DM_FLAG_PRE_RELOC,
};
