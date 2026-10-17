// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2021 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com
 * Author: Marek Szyprowski <m.szyprowski@samsung.com>
 */

#include <common.h>
#include <adc.h>
#include <button.h>
#include <log.h>
#include <dm.h>
#include <dm/lists.h>
#include <dm/of_access.h>
#include <dm/uclass-internal.h>

/**
 * struct button_adc_priv - private data for button-adc driver.
 *
 * @adc: Analog to Digital Converter device to which button is connected.
 * @channel: channel of the ADC device to probe the button state.
 * @min: minimal uV value to consider button as pressed.
 * @max: maximal uV value to consider button as pressed.
 */
struct button_adc_priv {
	struct udevice *adc;
	int channel;
	int min;
	int max;
};

static enum button_state_t button_adc_get_state(struct udevice *dev)
{
	struct button_adc_priv *priv = dev_get_priv(dev);
	unsigned int val;
	int ret, uV;

	ret = adc_start_channel(priv->adc, priv->channel);
	if (ret)
		return ret;

	ret = adc_channel_data(priv->adc, priv->channel, &val);
	if (ret)
		return ret;

	ret = adc_raw_to_uV(priv->adc, val, &uV);
	if (ret)
		return ret;

	return (uV >= priv->min && uV < priv->max) ? BUTTON_ON : BUTTON_OFF;
}

static int button_adc_of_to_plat(struct udevice *dev)
{
	struct button_uc_plat *uc_plat = dev_get_uclass_plat(dev);
	struct button_adc_priv *priv = dev_get_priv(dev);
	struct ofnode_phandle_args args;
	u32 down_threshold = 0, up_threshold, voltage, t;
	ofnode node;
	int ret;

	/* Ignore the top-level button node */
	if (!uc_plat->label)
		return 0;

	ret = dev_read_phandle_with_args(dev->parent, "io-channels",
					 "#io-channel-cells", 0, 0, &args);
	if (ret)
		return ret;

	ret = uclass_get_device_by_ofnode(UCLASS_ADC, args.node, &priv->adc);
	if (ret)
		return ret;

	ret = ofnode_read_u32(dev_ofnode(dev->parent),
			      "keyup-threshold-microvolt", &up_threshold);
	if (ret)
		return ret;

	ret = ofnode_read_u32(dev_ofnode(dev), "press-threshold-microvolt",
			      &voltage);
	if (ret)
		return ret;

	dev_for_each_subnode(node, dev->parent) {
		ret = ofnode_read_u32(node, "press-threshold-microvolt", &t);
		if (ret)
			return ret;

		if (t > voltage && t < up_threshold)
			up_threshold = t;
		else if (t < voltage && t > down_threshold)
			down_threshold = t;
	}

	priv->channel = args.args[0];

	/*
	 * Define the voltage range such that the button is only pressed
	 * when the voltage is closest to its own press-threshold-microvolt
	 */
	if (down_threshold == 0)
		priv->min = 0;
	else
		priv->min = down_threshold + (voltage - down_threshold) / 2;

	priv->max = voltage + (up_threshold - voltage) / 2;

	return ret;
}

static int button_adc_bind(struct udevice *parent)
{
	struct udevice *dev;
	ofnode node;
	int ret;

	dev_for_each_subnode(node, parent) {
		struct button_uc_plat *uc_plat;
		const char *label;

		label = ofnode_read_string(node, "label");
		if (!label) {
			debug("%s: node %s has no label\n", __func__,
			      ofnode_get_name(node));
			return -EINVAL;
		}
		ret = device_bind_driver_to_node(parent, "button_adc",
						 ofnode_get_name(node),
						 node, &dev);
		if (ret)
			return ret;
		uc_plat = dev_get_uclass_plat(dev);
		uc_plat->label = label;
	}

	return 0;
}

static const struct button_ops button_adc_ops = {
	.get_state	= button_adc_get_state,
};

static const struct udevice_id button_adc_ids[] = {
	{ .compatible = "adc-keys" },
	{ }
};

U_BOOT_DRIVER(button_adc) = {
	.name		= "button_adc",
	.id		= UCLASS_BUTTON,
	.of_match	= button_adc_ids,
	.ops		= &button_adc_ops,
	.priv_auto	= sizeof(struct button_adc_priv),
	.bind		= button_adc_bind,
	.of_to_plat	= button_adc_of_to_plat,
};
