// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2016 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY UCLASS_PWM

#include <common.h>
#include <dm.h>
#include <pwm.h>

int pwm_set_invert(struct udevice *dev, uint channel, bool polarity)
{
	struct pwm_ops *ops = pwm_get_ops(dev);

	if (!ops->set_invert)
		return -ENOSYS;

	return ops->set_invert(dev, channel, polarity);
}

int pwm_set_config(struct udevice *dev, uint channel, uint period_ns,
		   uint duty_ns)
{
	struct pwm_ops *ops = pwm_get_ops(dev);

	if (!ops->set_config)
		return -ENOSYS;

	return ops->set_config(dev, channel, period_ns, duty_ns);
}

int pwm_set_enable(struct udevice *dev, uint channel, bool enable)
{
	struct pwm_ops *ops = pwm_get_ops(dev);

	if (!ops->set_enable)
		return -ENOSYS;

	return ops->set_enable(dev, channel, enable);
}

UCLASS_DRIVER(pwm) = {
	.id		= UCLASS_PWM,
	.name		= "pwm",
};
