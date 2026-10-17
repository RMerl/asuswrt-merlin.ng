// SPDX-License-Identifier: GPL-2.0
/*
 * Texas Instruments DRA7 reset driver
 *
 * Copyright (C) 2019 Texas Instruments Incorporated - https://www.ti.com/
 * Author: Keerthy <j-keerthy@ti.com>
 */

#include <asm/io.h>
#include <common.h>
#include <dm.h>
#include <reset-uclass.h>
#include <dm/device_compat.h>

struct dra7_reset_priv {
	u32 rstctrl;
	u32 rstst;
	u8 nreset;
};

static inline void dra7_reset_rmw(u32 addr, u32 value, u32 mask)
{
	writel(((readl(addr) & (~mask)) | (value & mask)), addr);
}

static int dra7_reset_deassert(struct reset_ctl *reset_ctl)
{
	struct dra7_reset_priv *priv = dev_get_priv(reset_ctl->dev);
	int mask = 1 << reset_ctl->id;

	if (reset_ctl->id < 0 || reset_ctl->id >= priv->nreset)
		return -EINVAL;

	dra7_reset_rmw(priv->rstctrl, 0x0, mask);

	while ((readl(priv->rstst) & mask) != mask)
		;

	return 0;
}

static int dra7_reset_assert(struct reset_ctl *reset_ctl)
{
	struct dra7_reset_priv *priv = dev_get_priv(reset_ctl->dev);
	int mask = 1 << reset_ctl->id;

	if (reset_ctl->id < 0 || reset_ctl->id >= priv->nreset)
		return -EINVAL;

	dra7_reset_rmw(priv->rstctrl, mask, 0x0);

	return 0;
}

struct reset_ops dra7_reset_ops = {
	.rst_assert = dra7_reset_assert,
	.rst_deassert = dra7_reset_deassert,
};

static const struct udevice_id dra7_reset_ids[] = {
	{ .compatible = "ti,dra7-reset" },
	{ }
};

static int dra7_reset_probe(struct udevice *dev)
{
	struct dra7_reset_priv *priv = dev_get_priv(dev);

	priv->rstctrl = dev_read_addr(dev);
	priv->rstst = priv->rstctrl + 0x4;
	priv->nreset = dev_read_u32_default(dev, "ti,nresets", 1);

	dev_info(dev, "dra7-reset successfully probed %s\n", dev->name);

	return 0;
}

U_BOOT_DRIVER(dra7_reset) = {
	.name = "dra7_reset",
	.id = UCLASS_RESET,
	.of_match = dra7_reset_ids,
	.probe = dra7_reset_probe,
	.ops = &dra7_reset_ops,
	.priv_auto = sizeof(struct dra7_reset_priv),
};
