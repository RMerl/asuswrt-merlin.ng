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
#include <log.h>
#include <power/tps65090.h>

static int cros_ec_ldo_set_bus_speed(struct udevice *dev, unsigned int speed)
{
	return 0;
}

static int cros_ec_ldo_xfer(struct udevice *dev, struct i2c_msg *msg,
			    int nmsgs)
{
	bool is_read = nmsgs > 1;
	int fet_id, ret;

	/*
	 * Look for reads and writes of the LDO registers. In either case the
	 * first message is a write with the register number as the first byte.
	 */
	if (!nmsgs || !msg->len || (msg->flags & I2C_M_RD)) {
		debug("%s: Invalid message\n", __func__);
		goto err;
	}

	fet_id = msg->buf[0] - REG_FET_BASE;
	if (fet_id < 1 || fet_id > MAX_FET_NUM) {
		debug("%s: Invalid FET %d\n", __func__, fet_id);
		goto err;
	}

	if (is_read) {
		uint8_t state;

		ret = cros_ec_get_ldo(dev->parent, fet_id, &state);
		if (!ret)
			msg[1].buf[0] = state ?
				FET_CTRL_ENFET | FET_CTRL_PGFET : 0;
	} else {
		bool on = msg->buf[1] & FET_CTRL_ENFET;

		ret = cros_ec_set_ldo(dev->parent, fet_id, on);
	}

	return ret;

err:
	/* Indicate that the message is unimplemented */
	return -ENOSYS;
}

static const struct dm_i2c_ops cros_ec_i2c_ops = {
	.xfer		= cros_ec_ldo_xfer,
	.set_bus_speed	= cros_ec_ldo_set_bus_speed,
};

static const struct udevice_id cros_ec_i2c_ids[] = {
	{ .compatible = "google,cros-ec-ldo-tunnel" },
	{ }
};

U_BOOT_DRIVER(cros_ec_ldo) = {
	.name	= "cros_ec_ldo_tunnel",
	.id	= UCLASS_I2C,
	.of_match = cros_ec_i2c_ids,
	.ops	= &cros_ec_i2c_ops,
};
