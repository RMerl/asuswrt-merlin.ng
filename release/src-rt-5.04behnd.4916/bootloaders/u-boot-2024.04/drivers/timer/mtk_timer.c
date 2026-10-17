// SPDX-License-Identifier: GPL-2.0
/*
 * MediaTek timer driver
 *
 * Copyright (C) 2018 MediaTek Inc.
 * Author: Ryder Lee <ryder.lee@mediatek.com>
 */

#include <clk.h>
#include <common.h>
#include <dm.h>
#include <timer.h>
#include <asm/io.h>
#include <linux/bitops.h>

#define MTK_GPT4_OFFSET_V1	0x40
#define MTK_GPT4_OFFSET_V2	0x80

#define MTK_GPT_CON		0x0
#define MTK_GPT_V1_CLK		0x4
#define MTK_GPT_CNT		0x8

#define GPT_ENABLE		BIT(0)
#define GPT_CLEAR		BIT(1)
#define GPT_V1_FREERUN		GENMASK(5, 4)
#define GPT_V2_FREERUN		GENMASK(6, 5)

enum mtk_gpt_ver {
	MTK_GPT_V1,
	MTK_GPT_V2
};

struct mtk_timer_priv {
	void __iomem *base;
	unsigned int gpt4_offset;
};

static u64 mtk_timer_get_count(struct udevice *dev)
{
	struct mtk_timer_priv *priv = dev_get_priv(dev);
	u32 val = readl(priv->base + priv->gpt4_offset + MTK_GPT_CNT);

	return timer_conv_64(val);
}

static int mtk_timer_probe(struct udevice *dev)
{
	struct timer_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct mtk_timer_priv *priv = dev_get_priv(dev);
	struct clk clk, parent;
	int ret, gpt_ver;

	priv->base = dev_read_addr_ptr(dev);
	gpt_ver = dev_get_driver_data(dev);

	if (!priv->base)
		return -ENOENT;

	if (gpt_ver == MTK_GPT_V2) {
		priv->gpt4_offset = MTK_GPT4_OFFSET_V2;

		writel(GPT_V2_FREERUN | GPT_CLEAR | GPT_ENABLE,
		       priv->base + priv->gpt4_offset + MTK_GPT_CON);
	} else {
		priv->gpt4_offset = MTK_GPT4_OFFSET_V1;

		writel(GPT_V1_FREERUN | GPT_CLEAR | GPT_ENABLE,
		       priv->base + priv->gpt4_offset + MTK_GPT_CON);
		writel(0, priv->base + priv->gpt4_offset + MTK_GPT_V1_CLK);
	}

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret)
		return ret;

	ret = clk_get_by_index(dev, 1, &parent);
	if (!ret) {
		ret = clk_set_parent(&clk, &parent);
		if (ret)
			return ret;
	}

	uc_priv->clock_rate = clk_get_rate(&clk);
	if (!uc_priv->clock_rate)
		return -EINVAL;

	return 0;
}

static const struct timer_ops mtk_timer_ops = {
	.get_count = mtk_timer_get_count,
};

static const struct udevice_id mtk_timer_ids[] = {
	{ .compatible = "mediatek,timer", .data = MTK_GPT_V1 },
	{ .compatible = "mediatek,mt6577-timer", .data = MTK_GPT_V1 },
	{ .compatible = "mediatek,mt7981-timer", .data = MTK_GPT_V2 },
	{ .compatible = "mediatek,mt7986-timer", .data = MTK_GPT_V2 },
	{ }
};

U_BOOT_DRIVER(mtk_timer) = {
	.name = "mtk_timer",
	.id = UCLASS_TIMER,
	.of_match = mtk_timer_ids,
	.priv_auto	= sizeof(struct mtk_timer_priv),
	.probe = mtk_timer_probe,
	.ops = &mtk_timer_ops,
	.flags = DM_FLAG_PRE_RELOC,
};
