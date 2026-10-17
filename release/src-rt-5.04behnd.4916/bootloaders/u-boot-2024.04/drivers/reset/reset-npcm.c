// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2023 Nuvoton Technology Corp.
 */

#include <dm.h>
#include <reset-uclass.h>
#include <asm/io.h>
#include <dm/device_compat.h>

struct npcm_reset_priv {
	void __iomem *base;
};

static int npcm_reset_request(struct reset_ctl *rst)
{
	return 0;
}

static int npcm_reset_free(struct reset_ctl *rst)
{
	return 0;
}

static int npcm_reset_assert(struct reset_ctl *rst)
{
	struct npcm_reset_priv *priv = dev_get_priv(rst->dev);
	u32 val;

	debug("%s: id 0x%lx, data %ld\n", __func__, rst->id, rst->data);
	val = readl(priv->base + rst->id);
	val |= BIT(rst->data);
	writel(val, priv->base + rst->id);

	return 0;
}

static int npcm_reset_deassert(struct reset_ctl *rst)
{
	struct npcm_reset_priv *priv = dev_get_priv(rst->dev);
	u32 val;

	debug("%s: id 0x%lx, data %ld\n", __func__, rst->id, rst->data);
	val = readl(priv->base + rst->id);
	val &= ~BIT(rst->data);
	writel(val, priv->base + rst->id);

	return 0;
}

static int npcm_reset_xlate(struct reset_ctl *rst,
			    struct ofnode_phandle_args *args)
{
	if (args->args_count != 2) {
		dev_err(rst->dev, "Invalid args_count: %d\n", args->args_count);
		return -EINVAL;
	}

	/* Use id field as register offset and data field as reset bit positiion */
	rst->id = args->args[0];
	rst->data = args->args[1];

	return 0;
}

static int npcm_reset_probe(struct udevice *dev)
{
	struct npcm_reset_priv *priv = dev_get_priv(dev);

	priv->base = dev_remap_addr(dev);
	if (!priv->base)
		return -EINVAL;

	return 0;
}

static int npcm_reset_bind(struct udevice *dev)
{
	void __iomem *reg_base;
	u32 *rcr_values;
	int num_fields;
	u32 reg, val;
	int ret, i;

	reg_base = dev_remap_addr(dev);
	if (!reg_base)
		return -EINVAL;

	/*
	 * Set RCR initial value
	 * The rcr-initial-values cell is <reg_offset val>
	 */
	num_fields = dev_read_size(dev, "rcr-initial-values");
	if (num_fields < 2)
		return 0;

	num_fields /= sizeof(u32);
	if (num_fields % 2)
		return -EINVAL;

	num_fields = num_fields / 2;
	rcr_values = malloc(num_fields * 2 * sizeof(u32));
	if (!rcr_values)
		return -ENOMEM;

	ret = dev_read_u32_array(dev, "rcr-initial-values", rcr_values,
				 num_fields * 2);
	if (ret < 0) {
		free(rcr_values);
		return -EINVAL;
	}

	for (i = 0; i < num_fields; i++) {
		reg = rcr_values[2 * i];
		val = rcr_values[2 * i + 1];
		writel(val, reg_base + reg);
	}
	free(rcr_values);

	return 0;
}

static const struct udevice_id npcm_reset_ids[] = {
	{ .compatible = "nuvoton,npcm845-reset" },
	{ .compatible = "nuvoton,npcm750-reset" },
	{ }
};

struct reset_ops npcm_reset_ops = {
	.request = npcm_reset_request,
	.rfree = npcm_reset_free,
	.rst_assert = npcm_reset_assert,
	.rst_deassert = npcm_reset_deassert,
	.of_xlate = npcm_reset_xlate,
};

U_BOOT_DRIVER(npcm_reset) = {
	.name = "npcm_reset",
	.id = UCLASS_RESET,
	.of_match = npcm_reset_ids,
	.bind = npcm_reset_bind,
	.probe = npcm_reset_probe,
	.ops = &npcm_reset_ops,
	.priv_auto = sizeof(struct npcm_reset_priv),
};
