// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Google, Inc
 */

#include <common.h>
#include <dm.h>
#include <keyboard.h>

static int keyboard_start(struct stdio_dev *sdev)
{
	struct udevice *dev = sdev->priv;
	struct keyboard_ops *ops = keyboard_get_ops(dev);

	if (ops->start)
		return ops->start(dev);

	return 0;
}

static int keyboard_stop(struct stdio_dev *sdev)
{
	struct udevice *dev = sdev->priv;
	struct keyboard_ops *ops = keyboard_get_ops(dev);

	if (ops->stop)
		return ops->stop(dev);

	return 0;
}

static int keyboard_tstc(struct stdio_dev *sdev)
{
	struct udevice *dev = sdev->priv;
	struct keyboard_priv *priv = dev_get_uclass_priv(dev);
	struct keyboard_ops *ops = keyboard_get_ops(dev);

	/* Just get input to do this for us if we can */
	if (priv->input.dev)
		return input_tstc(&priv->input);
	else if (ops->tstc)
		return ops->tstc(dev);

	return -ENOSYS;
}

static int keyboard_getc(struct stdio_dev *sdev)
{
	struct udevice *dev = sdev->priv;
	struct keyboard_priv *priv = dev_get_uclass_priv(dev);
	struct keyboard_ops *ops = keyboard_get_ops(dev);

	/* Just get input to do this for us if we can */
	if (priv->input.dev)
		return input_getc(&priv->input);
	else if (ops->getc)
		return ops->getc(dev);

	return -ENOSYS;
}

static int keyboard_pre_probe(struct udevice *dev)
{
	struct keyboard_priv *priv = dev_get_uclass_priv(dev);
	struct stdio_dev *sdev = &priv->sdev;
	int ret;

	strlcpy(sdev->name, dev->name, sizeof(sdev->name));
	sdev->flags = DEV_FLAGS_INPUT;
	sdev->getc = keyboard_getc;
	sdev->tstc = keyboard_tstc;
	sdev->start = keyboard_start;
	sdev->stop = keyboard_stop;
	sdev->priv = dev;
	ret = input_init(&priv->input, 0);
	if (ret) {
		debug("%s: Cannot set up input, ret=%d - please add DEBUG to drivers/input/input.c to figure out the cause\n",
		      __func__, ret);
		return ret;
	}

	return 0;
}

UCLASS_DRIVER(keyboard) = {
	.id		= UCLASS_KEYBOARD,
	.name		= "keyboard",
	.pre_probe	= keyboard_pre_probe,
	.per_device_auto_alloc_size = sizeof(struct keyboard_priv),
};
