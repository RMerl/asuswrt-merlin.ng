// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2020 MediaTek Inc. All Rights Reserved.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#include <clk-uclass.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <dt-bindings/clock/mt7620-clk.h>
#include <misc.h>
#include <mach/mt7620-sysc.h>

/* CLKCFG1 */
#define CLKCFG1_REG			0x30

#define CLK_SRC_CPU			-1
#define CLK_SRC_CPU_D2			-2
#define CLK_SRC_SYS			-3
#define CLK_SRC_XTAL			-4
#define CLK_SRC_PERI			-5

struct mt7620_clk_priv {
	struct udevice *dev;
	struct udevice *sysc;
	struct mt7620_sysc_clks clks;
};

static const int mt7620_clks[] = {
	[CLK_SYS] = CLK_SRC_SYS,
	[CLK_CPU] = CLK_SRC_CPU,
	[CLK_XTAL] = CLK_SRC_XTAL,
	[CLK_MIPS_CNT] = CLK_SRC_CPU_D2,
	[CLK_UARTF] = CLK_SRC_PERI,
	[CLK_UARTL] = CLK_SRC_PERI,
	[CLK_SPI] = CLK_SRC_SYS,
	[CLK_I2C] = CLK_SRC_PERI,
	[CLK_I2S] = CLK_SRC_PERI,
};

static ulong mt7620_clk_get_rate(struct clk *clk)
{
	struct mt7620_clk_priv *priv = dev_get_priv(clk->dev);

	if (clk->id >= ARRAY_SIZE(mt7620_clks))
		return 0;

	switch (mt7620_clks[clk->id]) {
	case CLK_SRC_CPU:
		return priv->clks.cpu_clk;
	case CLK_SRC_CPU_D2:
		return priv->clks.cpu_clk / 2;
	case CLK_SRC_SYS:
		return priv->clks.sys_clk;
	case CLK_SRC_XTAL:
		return priv->clks.xtal_clk;
	case CLK_SRC_PERI:
		return priv->clks.peri_clk;
	default:
		return mt7620_clks[clk->id];
	}
}

static int mt7620_clkcfg1_rmw(struct mt7620_clk_priv *priv, u32 clr, u32 set)
{
	u32 val;
	int ret;

	ret = misc_read(priv->sysc, CLKCFG1_REG, &val, sizeof(val));
	if (ret) {
		dev_err(priv->dev, "mt7620_clk: failed to read CLKCFG1\n");
		return ret;
	}

	val &= ~clr;
	val |= set;

	ret = misc_write(priv->sysc, CLKCFG1_REG, &val, sizeof(val));
	if (ret) {
		dev_err(priv->dev, "mt7620_clk: failed to write CLKCFG1\n");
		return ret;
	}

	return 0;
}

static int mt7620_clk_enable(struct clk *clk)
{
	struct mt7620_clk_priv *priv = dev_get_priv(clk->dev);

	if (clk->id > 30)
		return -1;

	return mt7620_clkcfg1_rmw(priv, 0, BIT(clk->id));
}

static int mt7620_clk_disable(struct clk *clk)
{
	struct mt7620_clk_priv *priv = dev_get_priv(clk->dev);

	if (clk->id > 30)
		return -1;

	return mt7620_clkcfg1_rmw(priv, BIT(clk->id), 0);
}

const struct clk_ops mt7620_clk_ops = {
	.enable = mt7620_clk_enable,
	.disable = mt7620_clk_disable,
	.get_rate = mt7620_clk_get_rate,
};

static int mt7620_clk_probe(struct udevice *dev)
{
	struct mt7620_clk_priv *priv = dev_get_priv(dev);
	struct ofnode_phandle_args sysc_args;
	int ret;

	ret = ofnode_parse_phandle_with_args(dev_ofnode(dev), "mediatek,sysc", NULL,
					     0, 0, &sysc_args);
	if (ret) {
		dev_err(dev, "mt7620_clk: sysc property not found\n");
		return ret;
	}

	ret = uclass_get_device_by_ofnode(UCLASS_MISC, sysc_args.node,
					  &priv->sysc);
	if (ret) {
		dev_err(dev, "mt7620_clk: failed to sysc device\n");
		return ret;
	}

	ret = misc_ioctl(priv->sysc, MT7620_SYSC_IOCTL_GET_CLK,
			 &priv->clks);
	if (ret) {
		dev_err(dev, "mt7620_clk: failed to get base clocks\n");
		return ret;
	}

	priv->dev = dev;

	return 0;
}

static const struct udevice_id mt7620_clk_ids[] = {
	{ .compatible = "mediatek,mt7620-clk" },
	{ }
};

U_BOOT_DRIVER(mt7620_clk) = {
	.name = "mt7620-clk",
	.id = UCLASS_CLK,
	.of_match = mt7620_clk_ids,
	.probe = mt7620_clk_probe,
	.priv_auto = sizeof(struct mt7620_clk_priv),
	.ops = &mt7620_clk_ops,
	.flags = DM_FLAG_PRE_RELOC,
};
