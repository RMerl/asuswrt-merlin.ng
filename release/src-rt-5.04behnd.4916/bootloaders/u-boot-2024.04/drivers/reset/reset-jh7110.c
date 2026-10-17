// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 StarFive Technology Co., Ltd.
 * Author:	Yanhong Wang <yanhong.wang@starfivetech.com>
 *
 */

#include <common.h>
#include <dm.h>
#include <dm/ofnode.h>
#include <dt-bindings/reset/starfive,jh7110-crg.h>
#include <errno.h>
#include <linux/iopoll.h>
#include <reset-uclass.h>

struct jh7110_reset_priv {
	void __iomem *reg;
	u32	assert;
	u32	status;
	u32	resets;
};

struct reset_info {
	const char *compat;
	const u32 nr_resets;
	const u32 assert_offset;
	const u32 status_offset;
};

static const struct reset_info jh7110_rst_info[] = {
	{
		.compat = "starfive,jh7110-syscrg",
		.nr_resets = JH7110_SYSRST_END,
		.assert_offset = 0x2F8,
		.status_offset = 0x308,
	},
	{
		.compat = "starfive,jh7110-aoncrg",
		.nr_resets = JH7110_AONRST_END,
		.assert_offset = 0x38,
		.status_offset = 0x3C,
	},
	{
		.compat = "starfive,jh7110-stgcrg",
		.nr_resets = JH7110_STGRST_END,
		.assert_offset = 0x74,
		.status_offset = 0x78,
	}
};

static const struct reset_info *jh7110_reset_get_cfg(const char *compat)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(jh7110_rst_info); i++)
		if (!strcmp(compat, jh7110_rst_info[i].compat))
			return &jh7110_rst_info[i];

	return NULL;
}

static int jh7110_reset_trigger(struct jh7110_reset_priv *priv,
				unsigned long id, bool assert)
{
	ulong group;
	u32 mask, value, done = 0;
	ulong addr;

	group = id / 32;
	mask = BIT(id % 32);

	if (!assert)
		done ^= mask;

	addr = (ulong)priv->reg + priv->assert + group * sizeof(u32);
	value = readl((ulong *)addr);

	if (assert)
		value |= mask;
	else
		value &= ~mask;

	writel(value, (ulong *)addr);
	addr = (ulong)priv->reg + priv->status + group * sizeof(u32);

	return readl_poll_timeout((ulong *)addr, value,
						(value & mask) == done, 1000);
}

static int jh7110_reset_assert(struct reset_ctl *rst)
{
	struct jh7110_reset_priv *priv = dev_get_priv(rst->dev);

	jh7110_reset_trigger(priv, rst->id, true);

	return 0;
}

static int jh7110_reset_deassert(struct reset_ctl *rst)
{
	struct jh7110_reset_priv *priv = dev_get_priv(rst->dev);

	jh7110_reset_trigger(priv, rst->id, false);

	return 0;
}

static int jh7110_reset_free(struct reset_ctl *rst)
{
	return 0;
}

static int jh7110_reset_request(struct reset_ctl *rst)
{
	struct jh7110_reset_priv *priv = dev_get_priv(rst->dev);

	if (rst->id >= priv->resets)
		return -EINVAL;

	return 0;
}

static int jh7110_reset_probe(struct udevice *dev)
{
	struct jh7110_reset_priv *priv = dev_get_priv(dev);
	const struct reset_info *cfg;
	const char *compat;

	compat = ofnode_get_property(dev_ofnode(dev), "compatible", NULL);
	if (!compat)
		return -EINVAL;

	cfg = jh7110_reset_get_cfg(compat);
	if (!cfg)
		return -EINVAL;

	priv->assert = cfg->assert_offset;
	priv->status = cfg->status_offset;
	priv->resets = cfg->nr_resets;
	priv->reg = (void __iomem *)dev_read_addr_index(dev, 0);

	return 0;
}

const struct reset_ops jh7110_reset_reset_ops = {
	.rfree = jh7110_reset_free,
	.request = jh7110_reset_request,
	.rst_assert = jh7110_reset_assert,
	.rst_deassert = jh7110_reset_deassert,
};

U_BOOT_DRIVER(jh7110_reset) = {
	.name = "jh7110_reset",
	.id = UCLASS_RESET,
	.ops = &jh7110_reset_reset_ops,
	.probe = jh7110_reset_probe,
	.priv_auto = sizeof(struct jh7110_reset_priv),
};
