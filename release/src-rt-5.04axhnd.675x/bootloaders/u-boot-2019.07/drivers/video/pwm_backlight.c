// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2016 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY UCLASS_PANEL_BACKLIGHT

#include <common.h>
#include <dm.h>
#include <backlight.h>
#include <pwm.h>
#include <asm/gpio.h>
#include <power/regulator.h>

/**
 * Private information for the PWM backlight
 *
 * If @num_levels is 0 then the levels are simple values with the backlight
 * value going between the minimum (default 0) and the maximum (default 255).
 * Otherwise the levels are an index into @levels (0..n-1).
 *
 * @reg: Regulator to enable to turn the backlight on (NULL if none)
 * @enable, GPIO to set to enable the backlight (can be missing)
 * @pwm: PWM to use to change the backlight brightness
 * @channel: PWM channel to use
 * @period_ns: Period of the backlight in nanoseconds
 * @levels: Levels for the backlight, or NULL if not using indexed levels
 * @num_levels: Number of levels
 * @cur_level: Current level for the backlight (index or value)
 * @default_level: Default level for the backlight (index or value)
 * @min_level: Minimum level of the backlight (full off)
 * @min_level: Maximum level of the backlight (full on)
 * @enabled: true if backlight is enabled
 */
struct pwm_backlight_priv {
	struct udevice *reg;
	struct gpio_desc enable;
	struct udevice *pwm;
	uint channel;
	uint period_ns;
	/*
	 * the polarity of one PWM
	 * 0: normal polarity
	 * 1: inverted polarity
	 */
	bool polarity;
	u32 *levels;
	int num_levels;
	uint default_level;
	int cur_level;
	uint min_level;
	uint max_level;
	bool enabled;
};

static int set_pwm(struct pwm_backlight_priv *priv)
{
	uint duty_cycle;
	int ret;

	duty_cycle = priv->period_ns * (priv->cur_level - priv->min_level) /
		(priv->max_level - priv->min_level + 1);
	ret = pwm_set_config(priv->pwm, priv->channel, priv->period_ns,
			     duty_cycle);
	if (ret)
		return log_ret(ret);

	ret = pwm_set_invert(priv->pwm, priv->channel, priv->polarity);
	if (ret == -ENOSYS && !priv->polarity)
		ret = 0;

	return log_ret(ret);
}

static int enable_sequence(struct udevice *dev, int seq)
{
	struct pwm_backlight_priv *priv = dev_get_priv(dev);
	int ret;

	switch (seq) {
	case 0:
		if (priv->reg) {
			__maybe_unused struct dm_regulator_uclass_platdata
				*plat;

			plat = dev_get_uclass_platdata(priv->reg);
			log_debug("Enable '%s', regulator '%s'/'%s'\n",
				  dev->name, priv->reg->name, plat->name);
			ret = regulator_set_enable(priv->reg, true);
			if (ret) {
				log_debug("Cannot enable regulator for PWM '%s'\n",
					  dev->name);
				return log_ret(ret);
			}
			mdelay(120);
		}
		break;
	case 1:
		mdelay(10);
		dm_gpio_set_value(&priv->enable, 1);
		break;
	}

	return 0;
}

static int pwm_backlight_enable(struct udevice *dev)
{
	struct pwm_backlight_priv *priv = dev_get_priv(dev);
	int ret;

	ret = enable_sequence(dev, 0);
	if (ret)
		return log_ret(ret);
	ret = set_pwm(priv);
	if (ret)
		return log_ret(ret);
	ret = pwm_set_enable(priv->pwm, priv->channel, true);
	if (ret)
		return log_ret(ret);
	ret = enable_sequence(dev, 1);
	if (ret)
		return log_ret(ret);
	priv->enabled = true;

	return 0;
}

static int pwm_backlight_set_brightness(struct udevice *dev, int percent)
{
	struct pwm_backlight_priv *priv = dev_get_priv(dev);
	bool disable = false;
	int level;
	int ret;

	if (!priv->enabled) {
		ret = enable_sequence(dev, 0);
		if (ret)
			return log_ret(ret);
	}
	if (percent == BACKLIGHT_OFF) {
		disable = true;
		percent = 0;
	}
	if (percent == BACKLIGHT_DEFAULT) {
		level = priv->default_level;
	} else {
		if (priv->levels) {
			level = priv->levels[percent * (priv->num_levels - 1)
				/ 100];
		} else {
			level = priv->min_level +
				(priv->max_level - priv->min_level) *
				percent / 100;
		}
	}
	priv->cur_level = level;

	ret = set_pwm(priv);
	if (ret)
		return log_ret(ret);
	if (!priv->enabled) {
		ret = enable_sequence(dev, 1);
		if (ret)
			return log_ret(ret);
		priv->enabled = true;
	}
	if (disable) {
		dm_gpio_set_value(&priv->enable, 0);
		if (priv->reg) {
			ret = regulator_set_enable(priv->reg, false);
			if (ret)
				return log_ret(ret);
		}
		priv->enabled = false;
	}

	return 0;
}

static int pwm_backlight_ofdata_to_platdata(struct udevice *dev)
{
	struct pwm_backlight_priv *priv = dev_get_priv(dev);
	struct ofnode_phandle_args args;
	int index, ret, count, len;
	const u32 *cell;

	log_debug("start\n");
	ret = uclass_get_device_by_phandle(UCLASS_REGULATOR, dev,
					   "power-supply", &priv->reg);
	if (ret)
		log_debug("Cannot get power supply: ret=%d\n", ret);
	ret = gpio_request_by_name(dev, "enable-gpios", 0, &priv->enable,
				   GPIOD_IS_OUT);
	if (ret) {
		log_debug("Warning: cannot get enable GPIO: ret=%d\n", ret);
		if (ret != -ENOENT)
			return log_ret(ret);
	}
	ret = dev_read_phandle_with_args(dev, "pwms", "#pwm-cells", 0, 0,
					 &args);
	if (ret) {
		log_debug("Cannot get PWM phandle: ret=%d\n", ret);
		return log_ret(ret);
	}

	ret = uclass_get_device_by_ofnode(UCLASS_PWM, args.node, &priv->pwm);
	if (ret) {
		log_debug("Cannot get PWM: ret=%d\n", ret);
		return log_ret(ret);
	}
	if (args.args_count < 2)
		return log_msg_ret("Not enough arguments to pwm\n", -EINVAL);
	priv->channel = args.args[0];
	priv->period_ns = args.args[1];
	if (args.args_count > 2)
		priv->polarity = args.args[2];

	index = dev_read_u32_default(dev, "default-brightness-level", 255);
	cell = dev_read_prop(dev, "brightness-levels", &len);
	count = len / sizeof(u32);
	if (cell && count > index) {
		priv->levels = malloc(len);
		if (!priv->levels)
			return log_ret(-ENOMEM);
		dev_read_u32_array(dev, "brightness-levels", priv->levels,
				   count);
		priv->num_levels = count;
		priv->default_level = priv->levels[index];
		priv->max_level = priv->levels[count - 1];
	} else {
		priv->default_level = index;
		priv->max_level = 255;
	}
	priv->cur_level = priv->default_level;
	log_debug("done\n");


	return 0;
}

static int pwm_backlight_probe(struct udevice *dev)
{
	return 0;
}

static const struct backlight_ops pwm_backlight_ops = {
	.enable		= pwm_backlight_enable,
	.set_brightness	= pwm_backlight_set_brightness,
};

static const struct udevice_id pwm_backlight_ids[] = {
	{ .compatible = "pwm-backlight" },
	{ }
};

U_BOOT_DRIVER(pwm_backlight) = {
	.name	= "pwm_backlight",
	.id	= UCLASS_PANEL_BACKLIGHT,
	.of_match = pwm_backlight_ids,
	.ops	= &pwm_backlight_ops,
	.ofdata_to_platdata	= pwm_backlight_ofdata_to_platdata,
	.probe		= pwm_backlight_probe,
	.priv_auto_alloc_size	= sizeof(struct pwm_backlight_priv),
};
