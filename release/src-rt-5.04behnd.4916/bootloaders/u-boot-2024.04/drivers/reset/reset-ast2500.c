// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2017 Google, Inc
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
#include <asm/arch/scu_ast2500.h>

struct ast2500_reset_priv {
	struct ast2500_scu *scu;
};

static int ast2500_reset_assert(struct reset_ctl *reset_ctl)
{
	struct ast2500_reset_priv *priv = dev_get_priv(reset_ctl->dev);
	struct ast2500_scu *scu = priv->scu;

	debug("%s: reset_ctl->id: %lu\n", __func__, reset_ctl->id);

	if (reset_ctl->id < 32)
		setbits_le32(&scu->sysreset_ctrl1, BIT(reset_ctl->id));
	else
		setbits_le32(&scu->sysreset_ctrl2, BIT(reset_ctl->id - 32));

	return 0;
}

static int ast2500_reset_deassert(struct reset_ctl *reset_ctl)
{
	struct ast2500_reset_priv *priv = dev_get_priv(reset_ctl->dev);
	struct ast2500_scu *scu = priv->scu;

	debug("%s: reset_ctl->id: %lu\n", __func__, reset_ctl->id);

	if (reset_ctl->id < 32)
		clrbits_le32(&scu->sysreset_ctrl1, BIT(reset_ctl->id));
	else
		clrbits_le32(&scu->sysreset_ctrl2, BIT(reset_ctl->id - 32));

	return 0;
}

static int ast2500_reset_status(struct reset_ctl *reset_ctl)
{
	struct ast2500_reset_priv *priv = dev_get_priv(reset_ctl->dev);
	struct ast2500_scu *scu = priv->scu;
	int status;

	debug("%s: reset_ctl->id: %lu\n", __func__, reset_ctl->id);

	if (reset_ctl->id < 32)
		status = BIT(reset_ctl->id) & readl(&scu->sysreset_ctrl1);
	else
		status = BIT(reset_ctl->id - 32) & readl(&scu->sysreset_ctrl2);

	return !!status;
}



static int ast2500_reset_probe(struct udevice *dev)
{
	int rc;
	struct ast2500_reset_priv *priv = dev_get_priv(dev);
	struct udevice *scu_dev;

	/* get SCU base from clock device */
	rc = uclass_get_device_by_driver(UCLASS_CLK,
					 DM_DRIVER_GET(aspeed_ast2500_scu), &scu_dev);
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

static const struct udevice_id ast2500_reset_ids[] = {
	{ .compatible = "aspeed,ast2500-reset" },
	{ }
};

struct reset_ops ast2500_reset_ops = {
	.rst_assert = ast2500_reset_assert,
	.rst_deassert = ast2500_reset_deassert,
	.rst_status = ast2500_reset_status,
};

U_BOOT_DRIVER(ast2500_reset) = {
	.name = "ast2500_reset",
	.id = UCLASS_RESET,
	.of_match = ast2500_reset_ids,
	.probe = ast2500_reset_probe,
	.ops = &ast2500_reset_ops,
	.priv_auto	= sizeof(struct ast2500_reset_priv),
};
