// SPDX-License-Identifier: GPL-2.0+

#include <common.h>
#include <cros_ec.h>
#include <dm.h>
#include <errno.h>
#include <log.h>
#include <pwm.h>

struct cros_ec_pwm_priv {
	bool enabled;
	uint duty;
};

static int cros_ec_pwm_set_config(struct udevice *dev, uint channel,
				  uint period_ns, uint duty_ns)
{
	struct cros_ec_pwm_priv *priv = dev_get_priv(dev);
	uint duty;
	int ret;

	debug("%s: period_ns=%u, duty_ns=%u asked\n", __func__,
	      period_ns, duty_ns);

	/* No way to set the period, only a relative duty cycle */
	duty = EC_PWM_MAX_DUTY * duty_ns / period_ns;
	if (duty > EC_PWM_MAX_DUTY)
		duty = EC_PWM_MAX_DUTY;

	if (!priv->enabled) {
		priv->duty = duty;
		debug("%s: duty=%#x to-be-set\n", __func__, duty);
		return 0;
	}

	ret = cros_ec_set_pwm_duty(dev->parent, channel, duty);
	if (ret) {
		debug("%s: duty=%#x failed\n", __func__, duty);
		return ret;
	}

	priv->duty = duty;
	debug("%s: duty=%#x set\n", __func__, duty);

	return 0;
}

static int cros_ec_pwm_set_enable(struct udevice *dev, uint channel,
				  bool enable)
{
	struct cros_ec_pwm_priv *priv = dev_get_priv(dev);
	int ret;

	ret = cros_ec_set_pwm_duty(dev->parent, channel,
				   enable ? priv->duty : 0);
	if (ret) {
		debug("%s: enable=%d failed\n", __func__, enable);
		return ret;
	}

	priv->enabled = enable;
	debug("%s: enable=%d (duty=%#x) set\n", __func__,
	      enable, priv->duty);

	return 0;
}

static const struct pwm_ops cros_ec_pwm_ops = {
	.set_config	= cros_ec_pwm_set_config,
	.set_enable	= cros_ec_pwm_set_enable,
};

static const struct udevice_id cros_ec_pwm_ids[] = {
	{ .compatible = "google,cros-ec-pwm" },
	{ }
};

U_BOOT_DRIVER(cros_ec_pwm) = {
	.name	= "cros_ec_pwm",
	.id	= UCLASS_PWM,
	.of_match = cros_ec_pwm_ids,
	.ops	= &cros_ec_pwm_ops,
	.priv_auto = sizeof(struct cros_ec_pwm_priv),
};
