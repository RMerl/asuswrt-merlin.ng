// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2017-2018 Vasily Khoruzhick <anarsoul@gmail.com>
 */

#include <common.h>
#include <div64.h>
#include <dm.h>
#include <log.h>
#include <pwm.h>
#include <regmap.h>
#include <syscon.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/pwm.h>
#include <power/regulator.h>

DECLARE_GLOBAL_DATA_PTR;

#define OSC_24MHZ 24000000

struct sunxi_pwm_priv {
	struct sunxi_pwm *regs;
	bool invert;
	u32 prescaler;
};

static const u32 prescaler_table[] = {
	120,	/* 0000 */
	180,	/* 0001 */
	240,	/* 0010 */
	360,	/* 0011 */
	480,	/* 0100 */
	0,	/* 0101 */
	0,	/* 0110 */
	0,	/* 0111 */
	12000,	/* 1000 */
	24000,	/* 1001 */
	36000,	/* 1010 */
	48000,	/* 1011 */
	72000,	/* 1100 */
	0,	/* 1101 */
	0,	/* 1110 */
	1,	/* 1111 */
};

static int sunxi_pwm_set_invert(struct udevice *dev, uint channel,
				bool polarity)
{
	struct sunxi_pwm_priv *priv = dev_get_priv(dev);

	debug("%s: polarity=%u\n", __func__, polarity);
	priv->invert = polarity;

	return 0;
}

static int sunxi_pwm_set_config(struct udevice *dev, uint channel,
				uint period_ns, uint duty_ns)
{
	struct sunxi_pwm_priv *priv = dev_get_priv(dev);
	struct sunxi_pwm *regs = priv->regs;
	int best_prescaler = 0;
	u32 v, best_period = 0, duty;
	u64 best_scaled_freq = 0;
	const u32 nsecs_per_sec = 1000000000U;

	debug("%s: period_ns=%u, duty_ns=%u\n", __func__, period_ns, duty_ns);

	for (int prescaler = 0; prescaler <= SUNXI_PWM_CTRL_PRESCALE0_MASK;
	     prescaler++) {
		u32 period = 0;
		u64 scaled_freq = 0;
		if (!prescaler_table[prescaler])
			continue;
		scaled_freq = lldiv(OSC_24MHZ, prescaler_table[prescaler]);
		period = lldiv(scaled_freq * period_ns, nsecs_per_sec);
		if ((period - 1 <= SUNXI_PWM_CH0_PERIOD_MAX) &&
		    best_period < period) {
			best_period = period;
			best_scaled_freq = scaled_freq;
			best_prescaler = prescaler;
		}
	}

	if (best_period - 1 > SUNXI_PWM_CH0_PERIOD_MAX) {
		debug("%s: failed to find prescaler value\n", __func__);
		return -EINVAL;
	}

	duty = lldiv(best_scaled_freq * duty_ns, nsecs_per_sec);

	if (priv->prescaler != best_prescaler) {
		/* Mask clock to update prescaler */
		v = readl(&regs->ctrl);
		v &= ~SUNXI_PWM_CTRL_CLK_GATE;
		writel(v, &regs->ctrl);
		v &= ~SUNXI_PWM_CTRL_PRESCALE0_MASK;
		v |= (best_prescaler & SUNXI_PWM_CTRL_PRESCALE0_MASK);
		writel(v, &regs->ctrl);
		v |= SUNXI_PWM_CTRL_CLK_GATE;
		writel(v, &regs->ctrl);
		priv->prescaler = best_prescaler;
	}

	writel(SUNXI_PWM_CH0_PERIOD_PRD(best_period) |
	       SUNXI_PWM_CH0_PERIOD_DUTY(duty), &regs->ch0_period);

	debug("%s: prescaler: %d, period: %d, duty: %d\n",
	      __func__, priv->prescaler,
	      best_period, duty);

	return 0;
}

static int sunxi_pwm_set_enable(struct udevice *dev, uint channel, bool enable)
{
	struct sunxi_pwm_priv *priv = dev_get_priv(dev);
	struct sunxi_pwm *regs = priv->regs;
	u32 v;

	debug("%s: Enable '%s'\n", __func__, dev->name);

	v = readl(&regs->ctrl);
	if (!enable) {
		v &= ~SUNXI_PWM_CTRL_ENABLE0;
		writel(v, &regs->ctrl);
		return 0;
	}

	if (priv->invert)
		v &= ~SUNXI_PWM_CTRL_CH0_ACT_STA;
	else
		v |= SUNXI_PWM_CTRL_CH0_ACT_STA;
	v |= SUNXI_PWM_CTRL_ENABLE0;
	writel(v, &regs->ctrl);

	return 0;
}

static int sunxi_pwm_of_to_plat(struct udevice *dev)
{
	struct sunxi_pwm_priv *priv = dev_get_priv(dev);

	priv->regs = dev_read_addr_ptr(dev);

	return 0;
}

static int sunxi_pwm_probe(struct udevice *dev)
{
	return 0;
}

static const struct pwm_ops sunxi_pwm_ops = {
	.set_invert	= sunxi_pwm_set_invert,
	.set_config	= sunxi_pwm_set_config,
	.set_enable	= sunxi_pwm_set_enable,
};

static const struct udevice_id sunxi_pwm_ids[] = {
	{ .compatible = "allwinner,sun5i-a13-pwm" },
	{ .compatible = "allwinner,sun50i-a64-pwm" },
	{ }
};

U_BOOT_DRIVER(sunxi_pwm) = {
	.name	= "sunxi_pwm",
	.id	= UCLASS_PWM,
	.of_match = sunxi_pwm_ids,
	.ops	= &sunxi_pwm_ops,
	.of_to_plat	= sunxi_pwm_of_to_plat,
	.probe		= sunxi_pwm_probe,
	.priv_auto	= sizeof(struct sunxi_pwm_priv),
};
