// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2016 Google Inc.
 */

#include <common.h>
#include <dm.h>
#include <pwm.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/pwm.h>

struct tegra_pwm_priv {
	struct pwm_ctlr *regs;
};

static int tegra_pwm_set_config(struct udevice *dev, uint channel,
				uint period_ns, uint duty_ns)
{
	struct tegra_pwm_priv *priv = dev_get_priv(dev);
	struct pwm_ctlr *regs = priv->regs;
	uint pulse_width;
	u32 reg;

	if (channel >= 4)
		return -EINVAL;
	debug("%s: Configure '%s' channel %u\n", __func__, dev->name, channel);
	/* We ignore the period here and just use 32KHz */
	clock_start_periph_pll(PERIPH_ID_PWM, CLOCK_ID_SFROM32KHZ, 32768);

	pulse_width = duty_ns * 255 / period_ns;

	reg = pulse_width << PWM_WIDTH_SHIFT;
	reg |= 1 << PWM_DIVIDER_SHIFT;
	writel(reg, &regs[channel].control);
	debug("%s: pulse_width=%u\n", __func__, pulse_width);

	return 0;
}

static int tegra_pwm_set_enable(struct udevice *dev, uint channel, bool enable)
{
	struct tegra_pwm_priv *priv = dev_get_priv(dev);
	struct pwm_ctlr *regs = priv->regs;

	if (channel >= 4)
		return -EINVAL;
	debug("%s: Enable '%s' channel %u\n", __func__, dev->name, channel);
	clrsetbits_le32(&regs[channel].control, PWM_ENABLE_MASK,
			enable ? PWM_ENABLE_MASK : 0);

	return 0;
}

static int tegra_pwm_ofdata_to_platdata(struct udevice *dev)
{
	struct tegra_pwm_priv *priv = dev_get_priv(dev);

	priv->regs = (struct pwm_ctlr *)dev_read_addr(dev);

	return 0;
}

static const struct pwm_ops tegra_pwm_ops = {
	.set_config	= tegra_pwm_set_config,
	.set_enable	= tegra_pwm_set_enable,
};

static const struct udevice_id tegra_pwm_ids[] = {
	{ .compatible = "nvidia,tegra124-pwm" },
	{ .compatible = "nvidia,tegra20-pwm" },
	{ }
};

U_BOOT_DRIVER(tegra_pwm) = {
	.name	= "tegra_pwm",
	.id	= UCLASS_PWM,
	.of_match = tegra_pwm_ids,
	.ops	= &tegra_pwm_ops,
	.ofdata_to_platdata	= tegra_pwm_ofdata_to_platdata,
	.priv_auto_alloc_size	= sizeof(struct tegra_pwm_priv),
};
