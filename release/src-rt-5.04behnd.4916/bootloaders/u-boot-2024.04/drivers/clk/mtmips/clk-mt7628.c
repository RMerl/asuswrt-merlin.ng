// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2019 MediaTek Inc. All Rights Reserved.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <dt-bindings/clock/mt7628-clk.h>
#include <linux/bitops.h>
#include <linux/io.h>

/* SYSCFG0 */
#define XTAL_40M_SEL			BIT(6)

/* CLKCFG0 */
#define CLKCFG0_REG			0x0
#define PERI_CLK_FROM_XTAL_SEL		BIT(4)
#define CPU_PLL_FROM_BBP		BIT(1)
#define CPU_PLL_FROM_XTAL		BIT(0)

/* CLKCFG1 */
#define CLKCFG1_REG			0x4

#define CLK_SRC_CPU			-1
#define CLK_SRC_CPU_D2			-2
#define CLK_SRC_SYS			-3
#define CLK_SRC_XTAL			-4
#define CLK_SRC_PERI			-5

struct mt7628_clk_priv {
	void __iomem *base;
	int cpu_clk;
	int sys_clk;
	int xtal_clk;
};

static const int mt7628_clks[] = {
	[CLK_SYS] = CLK_SRC_SYS,
	[CLK_CPU] = CLK_SRC_CPU,
	[CLK_XTAL] = CLK_SRC_XTAL,
	[CLK_PWM] = CLK_SRC_PERI,
	[CLK_MIPS_CNT] = CLK_SRC_CPU_D2,
	[CLK_UART2] = CLK_SRC_PERI,
	[CLK_UART1] = CLK_SRC_PERI,
	[CLK_UART0] = CLK_SRC_PERI,
	[CLK_SPI] = CLK_SRC_SYS,
	[CLK_I2C] = CLK_SRC_PERI,
};

static ulong mt7628_clk_get_rate(struct clk *clk)
{
	struct mt7628_clk_priv *priv = dev_get_priv(clk->dev);
	u32 val;

	if (clk->id >= ARRAY_SIZE(mt7628_clks))
		return 0;

	switch (mt7628_clks[clk->id]) {
	case CLK_SRC_CPU:
		return priv->cpu_clk;
	case CLK_SRC_CPU_D2:
		return priv->cpu_clk / 2;
	case CLK_SRC_SYS:
		return priv->sys_clk;
	case CLK_SRC_XTAL:
		return priv->xtal_clk;
	case CLK_SRC_PERI:
		val = readl(priv->base + CLKCFG0_REG);
		if (val & PERI_CLK_FROM_XTAL_SEL)
			return priv->xtal_clk;
		else
			return 40000000;
	default:
		return mt7628_clks[clk->id];
	}
}

static int mt7628_clk_enable(struct clk *clk)
{
	struct mt7628_clk_priv *priv = dev_get_priv(clk->dev);

	if (clk->id > 31)
		return -1;

	setbits_32(priv->base + CLKCFG1_REG, BIT(clk->id));

	return 0;
}

static int mt7628_clk_disable(struct clk *clk)
{
	struct mt7628_clk_priv *priv = dev_get_priv(clk->dev);

	if (clk->id > 31)
		return -1;

	clrbits_32(priv->base + CLKCFG1_REG, BIT(clk->id));

	return 0;
}

const struct clk_ops mt7628_clk_ops = {
	.enable = mt7628_clk_enable,
	.disable = mt7628_clk_disable,
	.get_rate = mt7628_clk_get_rate,
};

static int mt7628_clk_probe(struct udevice *dev)
{
	struct mt7628_clk_priv *priv = dev_get_priv(dev);
	void __iomem *syscfg_base;
	u32 val;

	priv->base = (void __iomem *)dev_remap_addr_index(dev, 0);
	if (!priv->base)
		return -EINVAL;

	syscfg_base = (void __iomem *)dev_remap_addr_index(dev, 1);
	if (!syscfg_base)
		return -EINVAL;

	val = readl(syscfg_base);
	if (val & XTAL_40M_SEL)
		priv->xtal_clk = 40000000;
	else
		priv->xtal_clk = 25000000;

	val = readl(priv->base + CLKCFG0_REG);
	if (val & CPU_PLL_FROM_BBP)
		priv->cpu_clk = 480000000;
	else if (val & CPU_PLL_FROM_XTAL)
		priv->cpu_clk = priv->xtal_clk;
	else if (priv->xtal_clk == 40000000)
		priv->cpu_clk = 580000000;	/* (xtal_freq / 2) * 29 */
	else
		priv->cpu_clk = 575000000;	/* xtal_freq * 23 */

	priv->sys_clk = priv->cpu_clk / 3;

	return 0;
}

static const struct udevice_id mt7628_clk_ids[] = {
	{ .compatible = "mediatek,mt7628-clk" },
	{ }
};

U_BOOT_DRIVER(mt7628_clk) = {
	.name = "mt7628-clk",
	.id = UCLASS_CLK,
	.of_match = mt7628_clk_ids,
	.probe = mt7628_clk_probe,
	.priv_auto	= sizeof(struct mt7628_clk_priv),
	.ops = &mt7628_clk_ops,
};
