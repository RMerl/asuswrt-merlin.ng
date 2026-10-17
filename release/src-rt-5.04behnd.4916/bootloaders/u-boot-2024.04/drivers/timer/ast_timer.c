// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2016 Google Inc.
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <timer.h>
#include <asm/io.h>
#include <asm/arch/timer.h>
#include <linux/err.h>

#define AST_TICK_TIMER  1
#define AST_TMC_RELOAD_VAL  0xffffffff

struct ast_timer_priv {
	struct ast_timer *regs;
	struct ast_timer_counter *tmc;
};

static struct ast_timer_counter *ast_get_timer_counter(struct ast_timer *timer,
						       int n)
{
	if (n > 3)
		return &timer->timers2[n - 4];
	else
		return &timer->timers1[n - 1];
}

static int ast_timer_probe(struct udevice *dev)
{
	struct ast_timer_priv *priv = dev_get_priv(dev);
	struct timer_dev_priv *uc_priv = dev_get_uclass_priv(dev);

	writel(AST_TMC_RELOAD_VAL, &priv->tmc->reload_val);

	/*
	 * Stop the timer. This will also load reload_val into
	 * the status register.
	 */
	clrbits_le32(&priv->regs->ctrl1,
		     AST_TMC_EN << AST_TMC_CTRL1_SHIFT(AST_TICK_TIMER));
	/* Start the timer from the fixed 1MHz clock. */
	setbits_le32(&priv->regs->ctrl1,
		     (AST_TMC_EN | AST_TMC_1MHZ) <<
		     AST_TMC_CTRL1_SHIFT(AST_TICK_TIMER));

	uc_priv->clock_rate = AST_TMC_RATE;

	return 0;
}

static u64 ast_timer_get_count(struct udevice *dev)
{
	struct ast_timer_priv *priv = dev_get_priv(dev);

	return AST_TMC_RELOAD_VAL - readl(&priv->tmc->status);
}

static int ast_timer_of_to_plat(struct udevice *dev)
{
	struct ast_timer_priv *priv = dev_get_priv(dev);

	priv->regs = dev_read_addr_ptr(dev);
	if (!priv->regs)
		return -EINVAL;

	priv->tmc = ast_get_timer_counter(priv->regs, AST_TICK_TIMER);

	return 0;
}

static const struct timer_ops ast_timer_ops = {
	.get_count = ast_timer_get_count,
};

static const struct udevice_id ast_timer_ids[] = {
	{ .compatible = "aspeed,ast2500-timer" },
	{ .compatible = "aspeed,ast2400-timer" },
	{ }
};

U_BOOT_DRIVER(ast_timer) = {
	.name = "ast_timer",
	.id = UCLASS_TIMER,
	.of_match = ast_timer_ids,
	.probe = ast_timer_probe,
	.priv_auto	= sizeof(struct ast_timer_priv),
	.of_to_plat = ast_timer_of_to_plat,
	.ops = &ast_timer_ops,
};
