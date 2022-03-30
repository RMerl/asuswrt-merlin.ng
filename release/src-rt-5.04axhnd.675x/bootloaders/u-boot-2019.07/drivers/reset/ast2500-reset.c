// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2017 Google, Inc
 */

#include <common.h>
#include <dm.h>
#include <misc.h>
#include <reset.h>
#include <reset-uclass.h>
#include <wdt.h>
#include <asm/io.h>
#include <asm/arch/scu_ast2500.h>
#include <asm/arch/wdt.h>

struct ast2500_reset_priv {
	/* WDT used to perform resets. */
	struct udevice *wdt;
	struct ast2500_scu *scu;
};

static int ast2500_ofdata_to_platdata(struct udevice *dev)
{
	struct ast2500_reset_priv *priv = dev_get_priv(dev);
	int ret;

	ret = uclass_get_device_by_phandle(UCLASS_WDT, dev, "aspeed,wdt",
					   &priv->wdt);
	if (ret) {
		debug("%s: can't find WDT for reset controller", __func__);
		return ret;
	}

	return 0;
}

static int ast2500_reset_assert(struct reset_ctl *reset_ctl)
{
	struct ast2500_reset_priv *priv = dev_get_priv(reset_ctl->dev);
	u32 reset_mode, reset_mask;
	bool reset_sdram;
	int ret;

	/*
	 * To reset SDRAM, a specifal flag in SYSRESET register
	 * needs to be enabled first
	 */
	reset_mode = ast_reset_mode_from_flags(reset_ctl->id);
	reset_mask = ast_reset_mask_from_flags(reset_ctl->id);
	reset_sdram = reset_mode == WDT_CTRL_RESET_SOC &&
		(reset_mask & WDT_RESET_SDRAM);

	if (reset_sdram) {
		ast_scu_unlock(priv->scu);
		setbits_le32(&priv->scu->sysreset_ctrl1,
			     SCU_SYSRESET_SDRAM_WDT);
		ret = wdt_expire_now(priv->wdt, reset_ctl->id);
		clrbits_le32(&priv->scu->sysreset_ctrl1,
			     SCU_SYSRESET_SDRAM_WDT);
		ast_scu_lock(priv->scu);
	} else {
		ret = wdt_expire_now(priv->wdt, reset_ctl->id);
	}

	return ret;
}

static int ast2500_reset_request(struct reset_ctl *reset_ctl)
{
	debug("%s(reset_ctl=%p) (dev=%p, id=%lu)\n", __func__, reset_ctl,
	      reset_ctl->dev, reset_ctl->id);

	return 0;
}

static int ast2500_reset_probe(struct udevice *dev)
{
	struct ast2500_reset_priv *priv = dev_get_priv(dev);

	priv->scu = ast_get_scu();

	return 0;
}

static const struct udevice_id ast2500_reset_ids[] = {
	{ .compatible = "aspeed,ast2500-reset" },
	{ }
};

struct reset_ops ast2500_reset_ops = {
	.rst_assert = ast2500_reset_assert,
	.request = ast2500_reset_request,
};

U_BOOT_DRIVER(ast2500_reset) = {
	.name		= "ast2500_reset",
	.id		= UCLASS_RESET,
	.of_match = ast2500_reset_ids,
	.probe = ast2500_reset_probe,
	.ops = &ast2500_reset_ops,
	.ofdata_to_platdata = ast2500_ofdata_to_platdata,
	.priv_auto_alloc_size = sizeof(struct ast2500_reset_priv),
};
