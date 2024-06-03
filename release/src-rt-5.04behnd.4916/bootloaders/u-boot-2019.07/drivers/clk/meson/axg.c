// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2018 - Beniamino Galvani <b.galvani@gmail.com>
 * (C) Copyright 2018 - BayLibre, SAS
 * Author: Neil Armstrong <narmstrong@baylibre.com>
 */

#include <common.h>
#include <asm/arch/clock-axg.h>
#include <asm/io.h>
#include <clk-uclass.h>
#include <dm.h>
#include <regmap.h>
#include <syscon.h>
#include <div64.h>
#include <dt-bindings/clock/axg-clkc.h>
#include "clk_meson.h"

#define XTAL_RATE 24000000

struct meson_clk {
	struct regmap *map;
};

static ulong meson_clk_get_rate_by_id(struct clk *clk, unsigned long id);

static struct meson_gate gates[] = {
	/* Everything Else (EE) domain gates */
	MESON_GATE(CLKID_SPICC0, HHI_GCLK_MPEG0, 8),
	MESON_GATE(CLKID_I2C, HHI_GCLK_MPEG0, 9),
	MESON_GATE(CLKID_UART0, HHI_GCLK_MPEG0, 13),
	MESON_GATE(CLKID_SPICC1, HHI_GCLK_MPEG0, 15),
	MESON_GATE(CLKID_SD_EMMC_B, HHI_GCLK_MPEG0, 25),
	MESON_GATE(CLKID_SD_EMMC_C, HHI_GCLK_MPEG0, 26),
	MESON_GATE(CLKID_ETH, HHI_GCLK_MPEG1, 3),
	MESON_GATE(CLKID_UART1, HHI_GCLK_MPEG1, 16),

	/* Always On (AO) domain gates */
	MESON_GATE(CLKID_AO_I2C, HHI_GCLK_AO, 4),

	/* PLL Gates */
	/* CLKID_FCLK_DIV2 is critical for the SCPI Processor */
	MESON_GATE(CLKID_MPLL2, HHI_MPLL_CNTL9, 14),
	/* CLKID_CLK81 is critical for the system */

	/* Peripheral Gates */
	MESON_GATE(CLKID_SD_EMMC_B_CLK0, HHI_SD_EMMC_CLK_CNTL, 23),
	MESON_GATE(CLKID_SD_EMMC_C_CLK0, HHI_NAND_CLK_CNTL, 7),
};

static int meson_set_gate(struct clk *clk, bool on)
{
	struct meson_clk *priv = dev_get_priv(clk->dev);
	struct meson_gate *gate;

	if (clk->id >= ARRAY_SIZE(gates))
		return -ENOENT;

	gate = &gates[clk->id];

	if (gate->reg == 0)
		return 0;

	regmap_update_bits(priv->map, gate->reg,
			   BIT(gate->bit), on ? BIT(gate->bit) : 0);

	return 0;
}

static int meson_clk_enable(struct clk *clk)
{
	return meson_set_gate(clk, true);
}

static int meson_clk_disable(struct clk *clk)
{
	return meson_set_gate(clk, false);
}

static unsigned long meson_clk81_get_rate(struct clk *clk)
{
	struct meson_clk *priv = dev_get_priv(clk->dev);
	unsigned long parent_rate;
	uint reg;
	int parents[] = {
		-1,
		-1,
		CLKID_FCLK_DIV7,
		CLKID_MPLL1,
		CLKID_MPLL2,
		CLKID_FCLK_DIV4,
		CLKID_FCLK_DIV3,
		CLKID_FCLK_DIV5
	};

	/* mux */
	regmap_read(priv->map, HHI_MPEG_CLK_CNTL, &reg);
	reg = (reg >> 12) & 7;

	switch (reg) {
	case 0:
		parent_rate = XTAL_RATE;
		break;
	case 1:
		return -ENOENT;
	default:
		parent_rate = meson_clk_get_rate_by_id(clk, parents[reg]);
	}

	/* divider */
	regmap_read(priv->map, HHI_MPEG_CLK_CNTL, &reg);
	reg = reg & ((1 << 7) - 1);

	return parent_rate / reg;
}

static long mpll_rate_from_params(unsigned long parent_rate,
				  unsigned long sdm,
				  unsigned long n2)
{
	unsigned long divisor = (SDM_DEN * n2) + sdm;

	if (n2 < N2_MIN)
		return -EINVAL;

	return DIV_ROUND_UP_ULL((u64)parent_rate * SDM_DEN, divisor);
}

static struct parm meson_mpll0_parm[3] = {
	{HHI_MPLL_CNTL7, 0, 14}, /* psdm */
	{HHI_MPLL_CNTL7, 16, 9}, /* pn2 */
};

static struct parm meson_mpll1_parm[3] = {
	{HHI_MPLL_CNTL8, 0, 14}, /* psdm */
	{HHI_MPLL_CNTL8, 16, 9}, /* pn2 */
};

static struct parm meson_mpll2_parm[3] = {
	{HHI_MPLL_CNTL9, 0, 14}, /* psdm */
	{HHI_MPLL_CNTL9, 16, 9}, /* pn2 */
};

/*
 * MultiPhase Locked Loops are outputs from a PLL with additional frequency
 * scaling capabilities. MPLL rates are calculated as:
 *
 * f(N2_integer, SDM_IN ) = 2.0G/(N2_integer + SDM_IN/16384)
 */
static ulong meson_mpll_get_rate(struct clk *clk, unsigned long id)
{
	struct meson_clk *priv = dev_get_priv(clk->dev);
	struct parm *psdm, *pn2;
	unsigned long sdm, n2;
	unsigned long parent_rate;
	uint reg;

	switch (id) {
	case CLKID_MPLL0:
		psdm = &meson_mpll0_parm[0];
		pn2 = &meson_mpll0_parm[1];
		break;
	case CLKID_MPLL1:
		psdm = &meson_mpll1_parm[0];
		pn2 = &meson_mpll1_parm[1];
		break;
	case CLKID_MPLL2:
		psdm = &meson_mpll2_parm[0];
		pn2 = &meson_mpll2_parm[1];
		break;
	default:
		return -ENOENT;
	}

	parent_rate = meson_clk_get_rate_by_id(clk, CLKID_FIXED_PLL);
	if (IS_ERR_VALUE(parent_rate))
		return parent_rate;

	regmap_read(priv->map, psdm->reg_off, &reg);
	sdm = PARM_GET(psdm->width, psdm->shift, reg);

	regmap_read(priv->map, pn2->reg_off, &reg);
	n2 = PARM_GET(pn2->width, pn2->shift, reg);

	return mpll_rate_from_params(parent_rate, sdm, n2);
}

static struct parm meson_fixed_pll_parm[3] = {
	{HHI_MPLL_CNTL, 0, 9}, /* pm */
	{HHI_MPLL_CNTL, 9, 5}, /* pn */
	{HHI_MPLL_CNTL, 16, 2}, /* pod */
};

static struct parm meson_sys_pll_parm[3] = {
	{HHI_SYS_PLL_CNTL, 0, 9}, /* pm */
	{HHI_SYS_PLL_CNTL, 9, 5}, /* pn */
	{HHI_SYS_PLL_CNTL, 16, 2}, /* pod */
};

static ulong meson_pll_get_rate(struct clk *clk, unsigned long id)
{
	struct meson_clk *priv = dev_get_priv(clk->dev);
	struct parm *pm, *pn, *pod;
	unsigned long parent_rate_mhz = XTAL_RATE / 1000000;
	u16 n, m, od;
	uint reg;

	switch (id) {
	case CLKID_FIXED_PLL:
		pm = &meson_fixed_pll_parm[0];
		pn = &meson_fixed_pll_parm[1];
		pod = &meson_fixed_pll_parm[2];
		break;
	case CLKID_SYS_PLL:
		pm = &meson_sys_pll_parm[0];
		pn = &meson_sys_pll_parm[1];
		pod = &meson_sys_pll_parm[2];
		break;
	default:
		return -ENOENT;
	}

	regmap_read(priv->map, pn->reg_off, &reg);
	n = PARM_GET(pn->width, pn->shift, reg);

	regmap_read(priv->map, pm->reg_off, &reg);
	m = PARM_GET(pm->width, pm->shift, reg);

	regmap_read(priv->map, pod->reg_off, &reg);
	od = PARM_GET(pod->width, pod->shift, reg);

	return ((parent_rate_mhz * m / n) >> od) * 1000000;
}

static ulong meson_clk_get_rate_by_id(struct clk *clk, unsigned long id)
{
	ulong rate;

	switch (id) {
	case CLKID_FIXED_PLL:
	case CLKID_SYS_PLL:
		rate = meson_pll_get_rate(clk, id);
		break;
	case CLKID_FCLK_DIV2:
		rate = meson_pll_get_rate(clk, CLKID_FIXED_PLL) / 2;
		break;
	case CLKID_FCLK_DIV3:
		rate = meson_pll_get_rate(clk, CLKID_FIXED_PLL) / 3;
		break;
	case CLKID_FCLK_DIV4:
		rate = meson_pll_get_rate(clk, CLKID_FIXED_PLL) / 4;
		break;
	case CLKID_FCLK_DIV5:
		rate = meson_pll_get_rate(clk, CLKID_FIXED_PLL) / 5;
		break;
	case CLKID_FCLK_DIV7:
		rate = meson_pll_get_rate(clk, CLKID_FIXED_PLL) / 7;
		break;
	case CLKID_MPLL0:
	case CLKID_MPLL1:
	case CLKID_MPLL2:
		rate = meson_mpll_get_rate(clk, id);
		break;
	case CLKID_CLK81:
		rate = meson_clk81_get_rate(clk);
		break;
	default:
		if (gates[id].reg != 0) {
			/* a clock gate */
			rate = meson_clk81_get_rate(clk);
			break;
		}
		return -ENOENT;
	}

	debug("clock %lu has rate %lu\n", id, rate);
	return rate;
}

static ulong meson_clk_get_rate(struct clk *clk)
{
	return meson_clk_get_rate_by_id(clk, clk->id);
}

static int meson_clk_probe(struct udevice *dev)
{
	struct meson_clk *priv = dev_get_priv(dev);

	priv->map = syscon_node_to_regmap(dev_get_parent(dev)->node);
	if (IS_ERR(priv->map))
		return PTR_ERR(priv->map);

	debug("meson-clk-axg: probed\n");

	return 0;
}

static struct clk_ops meson_clk_ops = {
	.disable	= meson_clk_disable,
	.enable		= meson_clk_enable,
	.get_rate	= meson_clk_get_rate,
};

static const struct udevice_id meson_clk_ids[] = {
	{ .compatible = "amlogic,axg-clkc" },
	{ }
};

U_BOOT_DRIVER(meson_clk_axg) = {
	.name		= "meson_clk_axg",
	.id		= UCLASS_CLK,
	.of_match	= meson_clk_ids,
	.priv_auto_alloc_size = sizeof(struct meson_clk),
	.ops		= &meson_clk_ops,
	.probe		= meson_clk_probe,
};
