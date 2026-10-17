// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2019 MediaTek Inc. All Rights Reserved.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <log.h>
#include <malloc.h>
#include <reset-uclass.h>
#include <linux/bitops.h>
#include <linux/io.h>

struct mtmips_reset_priv {
	void __iomem *base;
};

static int mtmips_reset_assert(struct reset_ctl *reset_ctl)
{
	struct mtmips_reset_priv *priv = dev_get_priv(reset_ctl->dev);

	setbits_32(priv->base, BIT(reset_ctl->id));

	return 0;
}

static int mtmips_reset_deassert(struct reset_ctl *reset_ctl)
{
	struct mtmips_reset_priv *priv = dev_get_priv(reset_ctl->dev);

	clrbits_32(priv->base, BIT(reset_ctl->id));

	return 0;
}

static const struct reset_ops mtmips_reset_ops = {
	.rst_assert	= mtmips_reset_assert,
	.rst_deassert	= mtmips_reset_deassert,
};

static int mtmips_reset_probe(struct udevice *dev)
{
	return 0;
}

static int mtmips_reset_of_to_plat(struct udevice *dev)
{
	struct mtmips_reset_priv *priv = dev_get_priv(dev);

	priv->base = (void __iomem *)dev_remap_addr_index(dev, 0);
	if (!priv->base)
		return -EINVAL;

	return 0;
}

static const struct udevice_id mtmips_reset_ids[] = {
	{ .compatible = "mediatek,mtmips-reset" },
	{ }
};

U_BOOT_DRIVER(mtmips_reset) = {
	.name = "mtmips-reset",
	.id = UCLASS_RESET,
	.of_match = mtmips_reset_ids,
	.of_to_plat = mtmips_reset_of_to_plat,
	.probe = mtmips_reset_probe,
	.priv_auto	= sizeof(struct mtmips_reset_priv),
	.ops = &mtmips_reset_ops,
	.flags = DM_FLAG_PRE_RELOC,
};
