// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2019, Linaro Limited
 */

#include <asm/io.h>
#include <common.h>
#include <dm.h>
#include <dt-bindings/reset/ti-syscon.h>
#include <reset-uclass.h>

struct hisi_reset_priv {
	void __iomem *base;
};

static int hisi_reset_deassert(struct reset_ctl *rst)
{
	struct hisi_reset_priv *priv = dev_get_priv(rst->dev);
	u32 val;

	val = readl(priv->base + rst->data);
	if (rst->polarity & DEASSERT_SET)
		val |= BIT(rst->id);
	else
		val &= ~BIT(rst->id);
	writel(val, priv->base + rst->data);

	return 0;
}

static int hisi_reset_assert(struct reset_ctl *rst)
{
	struct hisi_reset_priv *priv = dev_get_priv(rst->dev);
	u32 val;

	val = readl(priv->base + rst->data);
	if (rst->polarity & ASSERT_SET)
		val |= BIT(rst->id);
	else
		val &= ~BIT(rst->id);
	writel(val, priv->base + rst->data);

	return 0;
}

static int hisi_reset_free(struct reset_ctl *rst)
{
	return 0;
}

static int hisi_reset_request(struct reset_ctl *rst)
{
	return 0;
}

static int hisi_reset_of_xlate(struct reset_ctl *rst,
			       struct ofnode_phandle_args *args)
{
	if (args->args_count != 3) {
		debug("Invalid args_count: %d\n", args->args_count);
		return -EINVAL;
	}

	/* Use .data field as register offset and .id field as bit shift */
	rst->data = args->args[0];
	rst->id = args->args[1];
	rst->polarity = args->args[2];

	return 0;
}

static const struct reset_ops hisi_reset_reset_ops = {
	.of_xlate = hisi_reset_of_xlate,
	.request = hisi_reset_request,
	.free = hisi_reset_free,
	.rst_assert = hisi_reset_assert,
	.rst_deassert = hisi_reset_deassert,
};

static const struct udevice_id hisi_reset_ids[] = {
	{ .compatible = "hisilicon,hi3798cv200-reset" },
	{ }
};

static int hisi_reset_probe(struct udevice *dev)
{
	struct hisi_reset_priv *priv = dev_get_priv(dev);

	priv->base = dev_remap_addr(dev);
	if (!priv->base)
		return -ENOMEM;

	return 0;
}

U_BOOT_DRIVER(hisi_reset) = {
	.name = "hisilicon_reset",
	.id = UCLASS_RESET,
	.of_match = hisi_reset_ids,
	.ops = &hisi_reset_reset_ops,
	.probe = hisi_reset_probe,
	.priv_auto_alloc_size = sizeof(struct hisi_reset_priv),
};
