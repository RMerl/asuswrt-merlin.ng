// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2022 MediaTek Inc. All rights reserved.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#include <clk-uclass.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <regmap.h>
#include <syscon.h>
#include <dt-bindings/clock/mt7621-clk.h>
#include <linux/io.h>
#include <linux/bitops.h>
#include <linux/bitfield.h>

#define SYSC_MAP_SIZE			0x100
#define MEMC_MAP_SIZE			0x1000

/* SYSC */
#define SYSCFG0_REG			0x10
#define XTAL_MODE_SEL			GENMASK(8, 6)

#define CLKCFG0_REG			0x2c
#define CPU_CLK_SEL			GENMASK(31, 30)
#define PERI_CLK_SEL			BIT(4)

#define CLKCFG1_REG			0x30

#define CUR_CLK_STS_REG			0x44
#define CUR_CPU_FDIV			GENMASK(12, 8)
#define CUR_CPU_FFRAC			GENMASK(4, 0)

/* MEMC */
#define MEMPLL1_REG			0x0604
#define RG_MEPL_DIV2_SEL		GENMASK(2, 1)

#define MEMPLL6_REG			0x0618
#define MEMPLL18_REG			0x0648
#define RG_MEPL_PREDIV			GENMASK(13, 12)
#define RG_MEPL_FBDIV			GENMASK(10, 4)

/* Fixed 500M clock */
#define GMPLL_CLK			500000000

struct mt7621_clk_priv {
	void __iomem *sysc_base;
	int cpu_clk;
	int ddr_clk;
	int sys_clk;
	int xtal_clk;
};

enum mt7621_clk_src {
	CLK_SRC_CPU,
	CLK_SRC_DDR,
	CLK_SRC_SYS,
	CLK_SRC_XTAL,
	CLK_SRC_PERI,
	CLK_SRC_125M,
	CLK_SRC_150M,
	CLK_SRC_250M,
	CLK_SRC_270M,

	__CLK_SRC_MAX
};

struct mt7621_clk_map {
	u32 cgbit;
	enum mt7621_clk_src clksrc;
};

#define CLK_MAP(_id, _cg, _src) \
	[_id] = { .cgbit = (_cg), .clksrc = (_src) }

#define CLK_MAP_SRC(_id, _src) \
	[_id] = { .cgbit = UINT32_MAX, .clksrc = (_src) }

static const struct mt7621_clk_map mt7621_clk_mappings[] = {
	CLK_MAP_SRC(MT7621_CLK_XTAL, CLK_SRC_XTAL),
	CLK_MAP_SRC(MT7621_CLK_CPU, CLK_SRC_CPU),
	CLK_MAP_SRC(MT7621_CLK_BUS, CLK_SRC_SYS),
	CLK_MAP_SRC(MT7621_CLK_50M, CLK_SRC_PERI),
	CLK_MAP_SRC(MT7621_CLK_125M, CLK_SRC_125M),
	CLK_MAP_SRC(MT7621_CLK_150M, CLK_SRC_150M),
	CLK_MAP_SRC(MT7621_CLK_250M, CLK_SRC_250M),
	CLK_MAP_SRC(MT7621_CLK_270M, CLK_SRC_270M),

	CLK_MAP(MT7621_CLK_HSDMA, 5, CLK_SRC_150M),
	CLK_MAP(MT7621_CLK_FE, 6, CLK_SRC_250M),
	CLK_MAP(MT7621_CLK_SP_DIVTX, 7, CLK_SRC_270M),
	CLK_MAP(MT7621_CLK_TIMER, 8, CLK_SRC_PERI),
	CLK_MAP(MT7621_CLK_PCM, 11, CLK_SRC_270M),
	CLK_MAP(MT7621_CLK_PIO, 13, CLK_SRC_PERI),
	CLK_MAP(MT7621_CLK_GDMA, 14, CLK_SRC_SYS),
	CLK_MAP(MT7621_CLK_NAND, 15, CLK_SRC_125M),
	CLK_MAP(MT7621_CLK_I2C, 16, CLK_SRC_PERI),
	CLK_MAP(MT7621_CLK_I2S, 17, CLK_SRC_270M),
	CLK_MAP(MT7621_CLK_SPI, 18, CLK_SRC_SYS),
	CLK_MAP(MT7621_CLK_UART1, 19, CLK_SRC_PERI),
	CLK_MAP(MT7621_CLK_UART2, 20, CLK_SRC_PERI),
	CLK_MAP(MT7621_CLK_UART3, 21, CLK_SRC_PERI),
	CLK_MAP(MT7621_CLK_ETH, 23, CLK_SRC_PERI),
	CLK_MAP(MT7621_CLK_PCIE0, 24, CLK_SRC_125M),
	CLK_MAP(MT7621_CLK_PCIE1, 25, CLK_SRC_125M),
	CLK_MAP(MT7621_CLK_PCIE2, 26, CLK_SRC_125M),
	CLK_MAP(MT7621_CLK_CRYPTO, 29, CLK_SRC_250M),
	CLK_MAP(MT7621_CLK_SHXC, 30, CLK_SRC_PERI),

	CLK_MAP_SRC(MT7621_CLK_MAX, __CLK_SRC_MAX),

	CLK_MAP_SRC(MT7621_CLK_DDR, CLK_SRC_DDR),
};

static ulong mt7621_clk_get_rate(struct clk *clk)
{
	struct mt7621_clk_priv *priv = dev_get_priv(clk->dev);
	u32 val;

	switch (mt7621_clk_mappings[clk->id].clksrc) {
	case CLK_SRC_CPU:
		return priv->cpu_clk;
	case CLK_SRC_DDR:
		return priv->ddr_clk;
	case CLK_SRC_SYS:
		return priv->sys_clk;
	case CLK_SRC_XTAL:
		return priv->xtal_clk;
	case CLK_SRC_PERI:
		val = readl(priv->sysc_base + CLKCFG0_REG);
		if (val & PERI_CLK_SEL)
			return priv->xtal_clk;
		else
			return GMPLL_CLK / 10;
	case CLK_SRC_125M:
		return 125000000;
	case CLK_SRC_150M:
		return 150000000;
	case CLK_SRC_250M:
		return 250000000;
	case CLK_SRC_270M:
		return 270000000;
	default:
		return 0;
	}
}

static int mt7621_clk_enable(struct clk *clk)
{
	struct mt7621_clk_priv *priv = dev_get_priv(clk->dev);
	u32 cgbit;

	cgbit = mt7621_clk_mappings[clk->id].cgbit;
	if (cgbit == UINT32_MAX)
		return -ENOSYS;

	setbits_32(priv->sysc_base + CLKCFG1_REG, BIT(cgbit));

	return 0;
}

static int mt7621_clk_disable(struct clk *clk)
{
	struct mt7621_clk_priv *priv = dev_get_priv(clk->dev);
	u32 cgbit;

	cgbit = mt7621_clk_mappings[clk->id].cgbit;
	if (cgbit == UINT32_MAX)
		return -ENOSYS;

	clrbits_32(priv->sysc_base + CLKCFG1_REG, BIT(cgbit));

	return 0;
}

static int mt7621_clk_request(struct clk *clk)
{
	if (clk->id >= ARRAY_SIZE(mt7621_clk_mappings))
		return -EINVAL;
	return 0;
}

const struct clk_ops mt7621_clk_ops = {
	.request = mt7621_clk_request,
	.enable = mt7621_clk_enable,
	.disable = mt7621_clk_disable,
	.get_rate = mt7621_clk_get_rate,
};

static void mt7621_get_clocks(struct mt7621_clk_priv *priv, struct regmap *memc)
{
	u32 bs, xtal_sel, clkcfg0, cur_clk, mempll, dividx, fb;
	u32 xtal_clk, xtal_div, ffiv, ffrac, cpu_clk, ddr_clk;
	static const u32 xtal_div_tbl[] = {0, 1, 2, 2};

	bs = readl(priv->sysc_base + SYSCFG0_REG);
	clkcfg0 = readl(priv->sysc_base + CLKCFG0_REG);
	cur_clk = readl(priv->sysc_base + CUR_CLK_STS_REG);

	xtal_sel = FIELD_GET(XTAL_MODE_SEL, bs);

	if (xtal_sel <= 2)
		xtal_clk = 20 * 1000 * 1000;
	else if (xtal_sel <= 5)
		xtal_clk = 40 * 1000 * 1000;
	else
		xtal_clk = 25 * 1000 * 1000;

	switch (FIELD_GET(CPU_CLK_SEL, clkcfg0)) {
	case 0:
		cpu_clk = GMPLL_CLK;
		break;
	case 1:
		regmap_read(memc, MEMPLL18_REG, &mempll);
		dividx = FIELD_GET(RG_MEPL_PREDIV, mempll);
		fb = FIELD_GET(RG_MEPL_FBDIV, mempll);
		xtal_div = 1 << xtal_div_tbl[dividx];
		cpu_clk = (fb + 1) * xtal_clk / xtal_div;
		break;
	default:
		cpu_clk = xtal_clk;
	}

	ffiv = FIELD_GET(CUR_CPU_FDIV, cur_clk);
	ffrac = FIELD_GET(CUR_CPU_FFRAC, cur_clk);
	cpu_clk = cpu_clk / ffiv * ffrac;

	regmap_read(memc, MEMPLL6_REG, &mempll);
	dividx = FIELD_GET(RG_MEPL_PREDIV, mempll);
	fb = FIELD_GET(RG_MEPL_FBDIV, mempll);
	xtal_div = 1 << xtal_div_tbl[dividx];
	ddr_clk = fb * xtal_clk / xtal_div;

	regmap_read(memc, MEMPLL1_REG, &bs);
	if (!FIELD_GET(RG_MEPL_DIV2_SEL, bs))
		ddr_clk *= 2;

	priv->cpu_clk = cpu_clk;
	priv->sys_clk = cpu_clk / 4;
	priv->ddr_clk = ddr_clk;
	priv->xtal_clk = xtal_clk;
}

static int mt7621_clk_probe(struct udevice *dev)
{
	struct mt7621_clk_priv *priv = dev_get_priv(dev);
	struct ofnode_phandle_args args;
	struct udevice *pdev;
	struct regmap *memc;
	int ret;

	pdev = dev_get_parent(dev);
	if (!pdev)
		return -ENODEV;

	priv->sysc_base = dev_remap_addr(pdev);
	if (!priv->sysc_base)
		return -EINVAL;

	/* get corresponding memc phandle */
	ret = dev_read_phandle_with_args(dev, "mediatek,memc", NULL, 0, 0,
					 &args);
	if (ret)
		return ret;

	memc = syscon_node_to_regmap(args.node);
	if (IS_ERR(memc))
		return PTR_ERR(memc);

	mt7621_get_clocks(priv, memc);

	return 0;
}

static const struct udevice_id mt7621_clk_ids[] = {
	{ .compatible = "mediatek,mt7621-clk" },
	{ }
};

U_BOOT_DRIVER(mt7621_clk) = {
	.name = "mt7621-clk",
	.id = UCLASS_CLK,
	.of_match = mt7621_clk_ids,
	.probe = mt7621_clk_probe,
	.priv_auto = sizeof(struct mt7621_clk_priv),
	.ops = &mt7621_clk_ops,
};
