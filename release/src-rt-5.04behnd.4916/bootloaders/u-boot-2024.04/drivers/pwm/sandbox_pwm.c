// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <pwm.h>
#include <asm/test.h>

enum {
	NUM_CHANNELS	= 3,
};

/**
 * struct sandbox_pwm_chan - a sandbox PWM channel
 *
 * @period_ns: Period of the PWM in nanoseconds
 * @duty_ns: Current duty cycle of the PWM in nanoseconds
 * @enable: true if the PWM is enabled
 * @polarity: true if the PWM polarity is active high
 */
struct sandbox_pwm_chan {
	uint period_ns;
	uint duty_ns;
	bool enable;
	bool polarity;
};

struct sandbox_pwm_priv {
	struct sandbox_pwm_chan chan[NUM_CHANNELS];
};

int sandbox_pwm_get_config(struct udevice *dev, uint channel, uint *period_nsp,
			   uint *duty_nsp, bool *enablep, bool *polarityp)
{
	struct sandbox_pwm_priv *priv = dev_get_priv(dev);
	struct sandbox_pwm_chan *chan;

	if (channel >= NUM_CHANNELS)
		return -ENOSPC;
	chan = &priv->chan[channel];
	*period_nsp = chan->period_ns;
	*duty_nsp = chan->duty_ns;
	*enablep = chan->enable;
	*polarityp = chan->polarity;

	return 0;
}

static int sandbox_pwm_set_config(struct udevice *dev, uint channel,
				  uint period_ns, uint duty_ns)
{
	struct sandbox_pwm_priv *priv = dev_get_priv(dev);
	struct sandbox_pwm_chan *chan;

	if (channel >= NUM_CHANNELS)
		return -ENOSPC;
	chan = &priv->chan[channel];

	if (channel == 2) {
		/* Pretend to have some fixed period */
		chan->period_ns = 4096;
		chan->duty_ns =  duty_ns * 4096 / period_ns;
	} else {
		chan->period_ns = period_ns;
		chan->duty_ns = duty_ns;
	}

	return 0;
}

static int sandbox_pwm_set_enable(struct udevice *dev, uint channel,
				  bool enable)
{
	struct sandbox_pwm_priv *priv = dev_get_priv(dev);
	struct sandbox_pwm_chan *chan;

	if (channel >= NUM_CHANNELS)
		return -ENOSPC;
	chan = &priv->chan[channel];
	chan->enable = enable;

	return 0;
}

static int sandbox_pwm_set_invert(struct udevice *dev, uint channel,
				  bool polarity)
{
	struct sandbox_pwm_priv *priv = dev_get_priv(dev);
	struct sandbox_pwm_chan *chan;

	if (channel >= NUM_CHANNELS)
		return -ENOSPC;
	chan = &priv->chan[channel];
	chan->polarity = polarity;

	return 0;
}

static const struct pwm_ops sandbox_pwm_ops = {
	.set_config	= sandbox_pwm_set_config,
	.set_enable	= sandbox_pwm_set_enable,
	.set_invert	= sandbox_pwm_set_invert,
};

static const struct udevice_id sandbox_pwm_ids[] = {
	{ .compatible = "sandbox,pwm" },
	{ }
};

U_BOOT_DRIVER(warm_pwm_sandbox) = {
	.name		= "pwm_sandbox",
	.id		= UCLASS_PWM,
	.of_match	= sandbox_pwm_ids,
	.ops		= &sandbox_pwm_ops,
	.priv_auto	= sizeof(struct sandbox_pwm_priv),
};
