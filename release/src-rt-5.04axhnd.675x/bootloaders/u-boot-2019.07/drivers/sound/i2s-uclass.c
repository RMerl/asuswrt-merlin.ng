// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <dm.h>
#include <i2s.h>

int i2s_tx_data(struct udevice *dev, void *data, uint data_size)
{
	struct i2s_ops *ops = i2s_get_ops(dev);

	if (!ops->tx_data)
		return -ENOSYS;

	return ops->tx_data(dev, data, data_size);
}

UCLASS_DRIVER(i2s) = {
	.id		= UCLASS_I2S,
	.name		= "i2s",
	.per_device_auto_alloc_size	= sizeof(struct i2s_uc_priv),
};
