// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <dm.h>
#include <cros_ec.h>
#include <errno.h>
#include <i2c.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

struct cros_ec_i2c_bus {
	int remote_bus;
};

static int cros_ec_i2c_set_bus_speed(struct udevice *dev, unsigned int speed)
{
	return 0;
}

static int cros_ec_i2c_xfer(struct udevice *dev, struct i2c_msg *msg,
			    int nmsgs)
{
	struct cros_ec_i2c_bus *i2c_bus = dev_get_priv(dev);

	return cros_ec_i2c_tunnel(dev->parent, i2c_bus->remote_bus, msg, nmsgs);
}

static int cros_ec_i2c_of_to_plat(struct udevice *dev)
{
	struct cros_ec_i2c_bus *i2c_bus = dev_get_priv(dev);
	const void *blob = gd->fdt_blob;
	int node = dev_of_offset(dev);

	i2c_bus->remote_bus = fdtdec_get_uint(blob, node, "google,remote-bus",
					      0);

	return 0;
}

static const struct dm_i2c_ops cros_ec_i2c_ops = {
	.xfer		= cros_ec_i2c_xfer,
	.set_bus_speed	= cros_ec_i2c_set_bus_speed,
};

static const struct udevice_id cros_ec_i2c_ids[] = {
	{ .compatible = "google,cros-ec-i2c-tunnel" },
	{ }
};

U_BOOT_DRIVER(cros_ec_tunnel) = {
	.name	= "cros_ec_tunnel",
	.id	= UCLASS_I2C,
	.of_match = cros_ec_i2c_ids,
	.of_to_plat = cros_ec_i2c_of_to_plat,
	.priv_auto	= sizeof(struct cros_ec_i2c_bus),
	.ops	= &cros_ec_i2c_ops,
};
