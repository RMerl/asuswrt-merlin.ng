// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY UCLASS_VIDEO_BRIDGE

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <edid.h>
#include <log.h>
#include <video_bridge.h>
#include <linux/delay.h>

int video_bridge_set_backlight(struct udevice *dev, int percent)
{
	struct video_bridge_ops *ops = video_bridge_get_ops(dev);

	if (!ops->set_backlight)
		return -ENOSYS;

	return ops->set_backlight(dev, percent);
}

int video_bridge_attach(struct udevice *dev)
{
	struct video_bridge_ops *ops = video_bridge_get_ops(dev);

	if (!ops->attach)
		return -ENOSYS;

	return ops->attach(dev);
}

int video_bridge_check_attached(struct udevice *dev)
{
	struct video_bridge_priv *uc_priv = dev_get_uclass_priv(dev);
	struct video_bridge_ops *ops = video_bridge_get_ops(dev);
	int ret;

	if (!ops->check_attached) {
		ret = dm_gpio_get_value(&uc_priv->hotplug);

		return ret > 0 ? 0 : ret == 0 ? -ENOTCONN : ret;
	}

	return ops->check_attached(dev);
}

int video_bridge_read_edid(struct udevice *dev, u8 *buf, int buf_size)
{
	struct video_bridge_ops *ops = video_bridge_get_ops(dev);

	if (!ops || !ops->read_edid)
		return -ENOSYS;
	return ops->read_edid(dev, buf, buf_size);
}

static int video_bridge_pre_probe(struct udevice *dev)
{
	struct video_bridge_priv *uc_priv = dev_get_uclass_priv(dev);
	int ret;

	debug("%s\n", __func__);
	ret = gpio_request_by_name(dev, "sleep-gpios", 0,
				   &uc_priv->sleep, GPIOD_IS_OUT);
	if (ret) {
		debug("%s: Could not decode sleep-gpios (%d)\n", __func__, ret);
		if (ret != -ENOENT)
			return ret;
	}
	/*
	 * Drop this for now as we do not have driver model pinctrl support
	 *
	 * ret = dm_gpio_set_pull(&uc_priv->sleep, GPIO_PULL_NONE);
	 * if (ret) {
	 *	debug("%s: Could not set sleep pull value\n", __func__);
	 *	return ret;
	 * }
	 */
	ret = gpio_request_by_name(dev, "reset-gpios", 0, &uc_priv->reset,
				   GPIOD_IS_OUT);
	if (ret) {
		debug("%s: Could not decode reset-gpios (%d)\n", __func__, ret);
		if (ret != -ENOENT)
			return ret;
	}
	/*
	 * Drop this for now as we do not have driver model pinctrl support
	 *
	 * ret = dm_gpio_set_pull(&uc_priv->reset, GPIO_PULL_NONE);
	 * if (ret) {
	 *	debug("%s: Could not set reset pull value\n", __func__);
	 *	return ret;
	 * }
	 */
	ret = gpio_request_by_name(dev, "hotplug-gpios", 0, &uc_priv->hotplug,
				   GPIOD_IS_IN);
	if (ret) {
		debug("%s: Could not decode hotplug (%d)\n", __func__, ret);
		if (ret != -ENOENT)
			return ret;
	}

	return 0;
}

int video_bridge_set_active(struct udevice *dev, bool active)
{
	struct video_bridge_priv *uc_priv = dev_get_uclass_priv(dev);
	int ret = 0;

	debug("%s: %d\n", __func__, active);
	if (uc_priv->sleep.dev) {
		ret = dm_gpio_set_value(&uc_priv->sleep, !active);
		if (ret)
			return ret;
	}

	if (!active)
		return 0;

	if (uc_priv->reset.dev) {
		ret = dm_gpio_set_value(&uc_priv->reset, true);
		if (ret)
			return ret;
		udelay(10);
		ret = dm_gpio_set_value(&uc_priv->reset, false);
	}

	return ret;
}

UCLASS_DRIVER(video_bridge) = {
	.id		= UCLASS_VIDEO_BRIDGE,
	.name		= "video_bridge",
	.per_device_auto	= sizeof(struct video_bridge_priv),
	.pre_probe	= video_bridge_pre_probe,
};
