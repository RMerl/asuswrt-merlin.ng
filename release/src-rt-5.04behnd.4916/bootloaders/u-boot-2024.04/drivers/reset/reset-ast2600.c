// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2020 ASPEED Technology Inc.
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <misc.h>
#include <reset.h>
#include <reset-uclass.h>
#include <linux/err.h>
#include <asm/io.h>
#include <asm/arch/scu_ast2600.h>

struct ast2600_reset_priv {
	struct ast2600_scu *scu;
};

static int ast2600_reset_assert(struct reset_ctl *reset_ctl)
{
	struct ast2600_reset_priv *priv = dev_get_priv(reset_ctl->dev);
	struct ast2600_scu *scu = priv->scu;

	debug("%s: reset_ctl->id: %lu\n", __func__, reset_ctl->id);

	if (reset_ctl->id < 32)
		writel(BIT(reset_ctl->id), &scu->modrst_ctrl1);
	else
		writel(BIT(reset_ctl->id - 32), &scu->modrst_ctrl2);

	return 0;
}

static int ast2600_reset_deassert(struct reset_ctl *reset_ctl)
{
	struct ast2600_reset_priv *priv = dev_get_priv(reset_ctl->dev);
	struct ast2600_scu *scu = priv->scu;

	debug("%s: reset_ctl->id: %lu\n", __func__, reset_ctl->id);

	if (reset_ctl->id < 32)
		writel(BIT(reset_ctl->id), &scu->modrst_clr1);
	else
		writel(BIT(reset_ctl->id - 32), &scu->modrst_clr2);

	return 0;
}

static int ast2600_reset_status(struct reset_ctl *reset_ctl)
{
	struct ast2600_reset_priv *priv = dev_get_priv(reset_ctl->dev);
	struct ast2600_scu *scu = priv->scu;
	int status;

	debug("%s: reset_ctl->id: %lu\n", __func__, reset_ctl->id);

	if (reset_ctl->id < 32)
		status = BIT(reset_ctl->id) & readl(&scu->modrst_ctrl1);
	else
		status = BIT(reset_ctl->id - 32) & readl(&scu->modrst_ctrl2);

	return !!status;
}

static int ast2600_reset_probe(struct udevice *dev)
{
	int rc;
	struct ast2600_reset_priv *priv = dev_get_priv(dev);
	struct udevice *scu_dev;

	/* get SCU base from clock device */
	rc = uclass_get_device_by_driver(UCLASS_CLK,
					 DM_DRIVER_GET(aspeed_ast2600_scu), &scu_dev);
	if (rc) {
		debug("%s: clock device not found, rc=%d\n", __func__, rc);
		return rc;
	}

	priv->scu = devfdt_get_addr_ptr(scu_dev);
	if (IS_ERR_OR_NULL(priv->scu)) {
		debug("%s: invalid SCU base pointer\n", __func__);
		return PTR_ERR(priv->scu);
	}

	return 0;
}

static const struct udevice_id ast2600_reset_ids[] = {
	{ .compatible = "aspeed,ast2600-reset" },
	{ }
};

struct reset_ops ast2600_reset_ops = {
	.rst_assert = ast2600_reset_assert,
	.rst_deassert = ast2600_reset_deassert,
	.rst_status = ast2600_reset_status,
};

U_BOOT_DRIVER(ast2600_reset) = {
	.name = "ast2600_reset",
	.id = UCLASS_RESET,
	.of_match = ast2600_reset_ids,
	.probe = ast2600_reset_probe,
	.ops = &ast2600_reset_ops,
	.priv_auto = sizeof(struct ast2600_reset_priv),
};
