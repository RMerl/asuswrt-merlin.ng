// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2017 Google, Inc
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <wdt.h>
#include <asm/io.h>
#include <asm/arch/wdt.h>

#define WDT_AST2500	2500
#define WDT_AST2400	2400

struct ast_wdt_priv {
	struct ast_wdt *regs;
};

static int ast_wdt_start(struct udevice *dev, u64 timeout, ulong flags)
{
	struct ast_wdt_priv *priv = dev_get_priv(dev);
	ulong driver_data = dev_get_driver_data(dev);
	u32 reset_mode = ast_reset_mode_from_flags(flags);

	/* 32 bits at 1MHz is 4294967ms */
	timeout = min_t(u64, timeout, 4294967);

	/* WDT counts in ticks of 1MHz clock. 1ms / 1e3 * 1e6 */
	timeout *= 1000;

	clrsetbits_le32(&priv->regs->ctrl,
			WDT_CTRL_RESET_MASK << WDT_CTRL_RESET_MODE_SHIFT,
			reset_mode << WDT_CTRL_RESET_MODE_SHIFT);

	if (driver_data >= WDT_AST2500 && reset_mode == WDT_CTRL_RESET_SOC)
		writel(ast_reset_mask_from_flags(flags),
		       &priv->regs->reset_mask);

	writel((u32) timeout, &priv->regs->counter_reload_val);
	writel(WDT_COUNTER_RESTART_VAL, &priv->regs->counter_restart);
	/*
	 * Setting CLK1MHZ bit is just for compatibility with ast2400 part.
	 * On ast2500 watchdog timer clock is fixed at 1MHz and the bit is
	 * read-only
	 */
	setbits_le32(&priv->regs->ctrl,
		     WDT_CTRL_EN | WDT_CTRL_RESET | WDT_CTRL_CLK1MHZ);

	return 0;
}

static int ast_wdt_stop(struct udevice *dev)
{
	struct ast_wdt_priv *priv = dev_get_priv(dev);

	clrbits_le32(&priv->regs->ctrl, WDT_CTRL_EN);

	writel(WDT_RESET_DEFAULT, &priv->regs->reset_mask);
	return 0;
}

static int ast_wdt_reset(struct udevice *dev)
{
	struct ast_wdt_priv *priv = dev_get_priv(dev);

	writel(WDT_COUNTER_RESTART_VAL, &priv->regs->counter_restart);

	return 0;
}

static int ast_wdt_expire_now(struct udevice *dev, ulong flags)
{
	struct ast_wdt_priv *priv = dev_get_priv(dev);
	int ret;

	ret = ast_wdt_start(dev, 1, flags);
	if (ret)
		return ret;

	while (readl(&priv->regs->ctrl) & WDT_CTRL_EN)
		;

	return ast_wdt_stop(dev);
}

static int ast_wdt_ofdata_to_platdata(struct udevice *dev)
{
	struct ast_wdt_priv *priv = dev_get_priv(dev);

	priv->regs = devfdt_get_addr_ptr(dev);
	if (IS_ERR(priv->regs))
		return PTR_ERR(priv->regs);

	return 0;
}

static const struct wdt_ops ast_wdt_ops = {
	.start = ast_wdt_start,
	.reset = ast_wdt_reset,
	.stop = ast_wdt_stop,
	.expire_now = ast_wdt_expire_now,
};

static const struct udevice_id ast_wdt_ids[] = {
	{ .compatible = "aspeed,wdt", .data = WDT_AST2500 },
	{ .compatible = "aspeed,ast2500-wdt", .data = WDT_AST2500 },
	{ .compatible = "aspeed,ast2400-wdt", .data = WDT_AST2400 },
	{}
};

static int ast_wdt_probe(struct udevice *dev)
{
	debug("%s() wdt%u\n", __func__, dev->seq);
	ast_wdt_stop(dev);

	return 0;
}

U_BOOT_DRIVER(ast_wdt) = {
	.name = "ast_wdt",
	.id = UCLASS_WDT,
	.of_match = ast_wdt_ids,
	.probe = ast_wdt_probe,
	.priv_auto_alloc_size = sizeof(struct ast_wdt_priv),
	.ofdata_to_platdata = ast_wdt_ofdata_to_platdata,
	.ops = &ast_wdt_ops,
};
