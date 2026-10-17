// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 VK
 * Author: Ivan Vozvakhov <i.vozvakhov@vk.team>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <led.h>
#include <malloc.h>
#include <pwm.h>

#define LEDS_PWM_DRIVER_NAME	"led_pwm"

struct led_pwm_priv {
	struct udevice *pwm;
	uint period;	/* period in ns */
	uint duty;	/* duty cycle in ns */
	uint channel;	/* pwm channel number */
	bool active_low;	/* pwm polarity */
	bool enabled;
};

static int led_pwm_enable(struct udevice *dev)
{
	struct led_pwm_priv *priv = dev_get_priv(dev);
	int ret;

	ret = pwm_set_invert(priv->pwm, priv->channel, priv->active_low);
	if (ret)
		return ret;

	ret = pwm_set_config(priv->pwm, priv->channel, priv->period, priv->duty);
	if (ret)
		return ret;

	ret = pwm_set_enable(priv->pwm, priv->channel, true);
	if (ret)
		return ret;

	priv->enabled = true;

	return 0;
}

static int led_pwm_disable(struct udevice *dev)
{
	struct led_pwm_priv *priv = dev_get_priv(dev);
	int ret;

	ret = pwm_set_config(priv->pwm, priv->channel, priv->period, 0);
	if (ret)
		return ret;

	ret = pwm_set_enable(priv->pwm, priv->channel, false);
	if (ret)
		return ret;

	priv->enabled = false;

	return 0;
}

static int led_pwm_set_state(struct udevice *dev, enum led_state_t state)
{
	struct led_pwm_priv *priv = dev_get_priv(dev);
	int ret;

	switch (state) {
	case LEDST_OFF:
		ret = led_pwm_disable(dev);
		break;
	case LEDST_ON:
		ret = led_pwm_enable(dev);
		break;
	case LEDST_TOGGLE:
		ret = (priv->enabled) ? led_pwm_disable(dev) : led_pwm_enable(dev);
		break;
	default:
		ret = -ENOSYS;
	}

	return ret;
}

static enum led_state_t led_pwm_get_state(struct udevice *dev)
{
	struct led_pwm_priv *priv = dev_get_priv(dev);

	return (priv->enabled) ? LEDST_ON : LEDST_OFF;
}

static int led_pwm_probe(struct udevice *dev)
{
	struct led_pwm_priv *priv = dev_get_priv(dev);

	return led_pwm_set_state(dev, (priv->enabled) ? LEDST_ON : LEDST_OFF);
}

static int led_pwm_of_to_plat(struct udevice *dev)
{
	struct led_pwm_priv *priv = dev_get_priv(dev);
	struct ofnode_phandle_args args;
	uint def_brightness, max_brightness;
	int ret;

	ret = dev_read_phandle_with_args(dev, "pwms", "#pwm-cells", 0, 0, &args);
	if (ret)
		return ret;

	ret = uclass_get_device_by_ofnode(UCLASS_PWM, args.node, &priv->pwm);
	if (ret)
		return ret;

	priv->channel = args.args[0];
	priv->period = args.args[1];
	priv->active_low = dev_read_bool(dev, "active-low");

	def_brightness = dev_read_u32_default(dev, "u-boot,default-brightness", 0);
	max_brightness = dev_read_u32_default(dev, "max-brightness", 255);
	priv->enabled =  !!def_brightness;

	/*
	 * No need to handle pwm inverted case (active_low)
	 * because of pwm_set_invert function
	 */
	if (def_brightness < max_brightness)
		priv->duty = priv->period * def_brightness / max_brightness;
	else
		priv->duty = priv->period;

	return 0;
}

static int led_pwm_bind(struct udevice *parent)
{
	return led_bind_generic(parent, LEDS_PWM_DRIVER_NAME);
}

static const struct led_ops led_pwm_ops = {
	.set_state = led_pwm_set_state,
	.get_state = led_pwm_get_state,
};

static const struct udevice_id led_pwm_ids[] = {
	{ .compatible = "pwm-leds" },
	{ }
};

U_BOOT_DRIVER(led_pwm) = {
	.name = LEDS_PWM_DRIVER_NAME,
	.id = UCLASS_LED,
	.ops = &led_pwm_ops,
	.priv_auto = sizeof(struct led_pwm_priv),
	.probe = led_pwm_probe,
	.of_to_plat = led_pwm_of_to_plat,
};

U_BOOT_DRIVER(led_pwm_wrap) = {
	.name = LEDS_PWM_DRIVER_NAME "_wrap",
	.id = UCLASS_NOP,
	.of_match = led_pwm_ids,
	.bind = led_pwm_bind,
};
