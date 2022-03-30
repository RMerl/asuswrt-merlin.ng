// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2016 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <clk.h>
#include <div64.h>
#include <dm.h>
#include <pwm.h>
#include <regmap.h>
#include <syscon.h>
#include <asm/io.h>
#include <asm/arch-rockchip/pwm.h>
#include <power/regulator.h>

struct rk_pwm_priv {
	struct rk3288_pwm *regs;
	ulong freq;
	uint enable_conf;
};

static int rk_pwm_set_invert(struct udevice *dev, uint channel, bool polarity)
{
	struct rk_pwm_priv *priv = dev_get_priv(dev);

	debug("%s: polarity=%u\n", __func__, polarity);
	priv->enable_conf &= ~(PWM_DUTY_MASK | PWM_INACTIVE_MASK);
	if (polarity)
		priv->enable_conf |= PWM_DUTY_NEGATIVE | PWM_INACTIVE_POSTIVE;
	else
		priv->enable_conf |= PWM_DUTY_POSTIVE | PWM_INACTIVE_NEGATIVE;

	return 0;
}

static int rk_pwm_set_config(struct udevice *dev, uint channel, uint period_ns,
			     uint duty_ns)
{
	struct rk_pwm_priv *priv = dev_get_priv(dev);
	struct rk3288_pwm *regs = priv->regs;
	unsigned long period, duty;

	debug("%s: period_ns=%u, duty_ns=%u\n", __func__, period_ns, duty_ns);
	writel(PWM_SEL_SRC_CLK | PWM_OUTPUT_LEFT | PWM_LP_DISABLE |
		PWM_CONTINUOUS | priv->enable_conf |
		RK_PWM_DISABLE,
		&regs->ctrl);

	period = lldiv((uint64_t)(priv->freq / 1000) * period_ns, 1000000);
	duty = lldiv((uint64_t)(priv->freq / 1000) * duty_ns, 1000000);

	writel(period, &regs->period_hpr);
	writel(duty, &regs->duty_lpr);
	debug("%s: period=%lu, duty=%lu\n", __func__, period, duty);

	return 0;
}

static int rk_pwm_set_enable(struct udevice *dev, uint channel, bool enable)
{
	struct rk_pwm_priv *priv = dev_get_priv(dev);
	struct rk3288_pwm *regs = priv->regs;

	debug("%s: Enable '%s'\n", __func__, dev->name);
	clrsetbits_le32(&regs->ctrl, RK_PWM_ENABLE, enable ? RK_PWM_ENABLE : 0);

	return 0;
}

static int rk_pwm_ofdata_to_platdata(struct udevice *dev)
{
	struct rk_pwm_priv *priv = dev_get_priv(dev);

	priv->regs = (struct rk3288_pwm *)dev_read_addr(dev);

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
	priv->enable_conf = PWM_DUTY_POSTIVE | PWM_INACTIVE_POSTIVE;

	return 0;
}

static const struct pwm_ops rk_pwm_ops = {
	.set_invert	= rk_pwm_set_invert,
	.set_config	= rk_pwm_set_config,
	.set_enable	= rk_pwm_set_enable,
};

static const struct udevice_id rk_pwm_ids[] = {
	{ .compatible = "rockchip,rk3288-pwm" },
	{ }
};

U_BOOT_DRIVER(rk_pwm) = {
	.name	= "rk_pwm",
	.id	= UCLASS_PWM,
	.of_match = rk_pwm_ids,
	.ops	= &rk_pwm_ops,
	.ofdata_to_platdata	= rk_pwm_ofdata_to_platdata,
	.probe		= rk_pwm_probe,
	.priv_auto_alloc_size	= sizeof(struct rk_pwm_priv),
};
