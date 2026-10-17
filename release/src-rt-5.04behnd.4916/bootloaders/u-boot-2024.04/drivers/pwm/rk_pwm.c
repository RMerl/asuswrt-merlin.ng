// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2016 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <clk.h>
#include <div64.h>
#include <dm.h>
#include <log.h>
#include <pwm.h>
#include <regmap.h>
#include <syscon.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch-rockchip/pwm.h>
#include <linux/bitops.h>
#include <power/regulator.h>

DECLARE_GLOBAL_DATA_PTR;

struct rockchip_pwm_data {
	struct rockchip_pwm_regs regs;
	unsigned int prescaler;
	bool supports_polarity;
	bool supports_lock;
	u32 enable_conf;
	u32 enable_conf_mask;
};

struct rk_pwm_priv {
	uintptr_t base;
	ulong freq;
	u32 conf_polarity;
	const struct rockchip_pwm_data *data;
};

static int rk_pwm_set_invert(struct udevice *dev, uint channel, bool polarity)
{
	struct rk_pwm_priv *priv = dev_get_priv(dev);

	if (!priv->data->supports_polarity) {
		debug("%s: Do not support polarity\n", __func__);
		return 0;
	}

	debug("%s: polarity=%u\n", __func__, polarity);
	if (polarity)
		priv->conf_polarity = PWM_DUTY_NEGATIVE | PWM_INACTIVE_POSTIVE;
	else
		priv->conf_polarity = PWM_DUTY_POSTIVE | PWM_INACTIVE_NEGATIVE;

	return 0;
}

static int rk_pwm_set_config(struct udevice *dev, uint channel, uint period_ns,
			     uint duty_ns)
{
	struct rk_pwm_priv *priv = dev_get_priv(dev);
	const struct rockchip_pwm_regs *regs = &priv->data->regs;
	unsigned long period, duty;
	u32 ctrl;

	debug("%s: period_ns=%u, duty_ns=%u\n", __func__, period_ns, duty_ns);

	ctrl = readl(priv->base + regs->ctrl);
	/*
	 * Lock the period and duty of previous configuration, then
	 * change the duty and period, that would not be effective.
	 */
	if (priv->data->supports_lock) {
		ctrl |= PWM_LOCK;
		writel(ctrl, priv->base + regs->ctrl);
	}

	period = lldiv((uint64_t)priv->freq * period_ns,
		       priv->data->prescaler * 1000000000);
	duty = lldiv((uint64_t)priv->freq * duty_ns,
		     priv->data->prescaler * 1000000000);

	writel(period, priv->base + regs->period);
	writel(duty, priv->base + regs->duty);

	if (priv->data->supports_polarity) {
		ctrl &= ~(PWM_DUTY_MASK | PWM_INACTIVE_MASK);
		ctrl |= priv->conf_polarity;
	}

	/*
	 * Unlock and set polarity at the same time,
	 * the configuration of duty, period and polarity
	 * would be effective together at next period.
	 */
	if (priv->data->supports_lock)
		ctrl &= ~PWM_LOCK;
	writel(ctrl, priv->base + regs->ctrl);

	debug("%s: period=%lu, duty=%lu\n", __func__, period, duty);

	return 0;
}

static int rk_pwm_set_enable(struct udevice *dev, uint channel, bool enable)
{
	struct rk_pwm_priv *priv = dev_get_priv(dev);
	const struct rockchip_pwm_regs *regs = &priv->data->regs;
	u32 ctrl;

	debug("%s: Enable '%s'\n", __func__, dev->name);

	ctrl = readl(priv->base + regs->ctrl);
	ctrl &= ~priv->data->enable_conf_mask;

	if (enable)
		ctrl |= priv->data->enable_conf;
	else
		ctrl &= ~priv->data->enable_conf;

	writel(ctrl, priv->base + regs->ctrl);

	return 0;
}

static int rk_pwm_of_to_plat(struct udevice *dev)
{
	struct rk_pwm_priv *priv = dev_get_priv(dev);

	priv->base = dev_read_addr(dev);

	return 0;
}

static int rk_pwm_probe(struct udevice *dev)
{
	struct rk_pwm_priv *priv = dev_get_priv(dev);
	struct clk clk;
	int ret = 0;

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret < 0) {
		debug("%s get clock fail!\n", __func__);
		return -EINVAL;
	}

	priv->freq = clk_get_rate(&clk);
	priv->data = (struct rockchip_pwm_data *)dev_get_driver_data(dev);

	if (priv->data->supports_polarity)
		priv->conf_polarity = PWM_DUTY_POSTIVE | PWM_INACTIVE_NEGATIVE;

	return 0;
}

static const struct pwm_ops rk_pwm_ops = {
	.set_invert	= rk_pwm_set_invert,
	.set_config	= rk_pwm_set_config,
	.set_enable	= rk_pwm_set_enable,
};

static const struct rockchip_pwm_data pwm_data_v1 = {
	.regs = {
		.duty = 0x04,
		.period = 0x08,
		.cntr = 0x00,
		.ctrl = 0x0c,
	},
	.prescaler = 2,
	.supports_polarity = false,
	.supports_lock = false,
	.enable_conf = PWM_CTRL_OUTPUT_EN | PWM_CTRL_TIMER_EN,
	.enable_conf_mask = BIT(1) | BIT(3),
};

static const struct rockchip_pwm_data pwm_data_v2 = {
	.regs = {
		.duty = 0x08,
		.period = 0x04,
		.cntr = 0x00,
		.ctrl = 0x0c,
	},
	.prescaler = 1,
	.supports_polarity = true,
	.supports_lock = false,
	.enable_conf = PWM_OUTPUT_LEFT | PWM_LP_DISABLE | RK_PWM_ENABLE |
		       PWM_CONTINUOUS,
	.enable_conf_mask = GENMASK(2, 0) | BIT(5) | BIT(8),
};

static const struct rockchip_pwm_data pwm_data_v3 = {
	.regs = {
		.duty = 0x08,
		.period = 0x04,
		.cntr = 0x00,
		.ctrl = 0x0c,
	},
	.prescaler = 1,
	.supports_polarity = true,
	.supports_lock = true,
	.enable_conf = PWM_OUTPUT_LEFT | PWM_LP_DISABLE | RK_PWM_ENABLE |
		       PWM_CONTINUOUS,
	.enable_conf_mask = GENMASK(2, 0) | BIT(5) | BIT(8),
};

static const struct udevice_id rk_pwm_ids[] = {
	{ .compatible = "rockchip,rk2928-pwm", .data = (ulong)&pwm_data_v1},
	{ .compatible = "rockchip,rk3288-pwm", .data = (ulong)&pwm_data_v2},
	{ .compatible = "rockchip,rk3328-pwm", .data = (ulong)&pwm_data_v3},
	{ }
};

U_BOOT_DRIVER(rk_pwm) = {
	.name	= "rk_pwm",
	.id	= UCLASS_PWM,
	.of_match = rk_pwm_ids,
	.ops	= &rk_pwm_ops,
	.of_to_plat	= rk_pwm_of_to_plat,
	.probe		= rk_pwm_probe,
	.priv_auto	= sizeof(struct rk_pwm_priv),
};
