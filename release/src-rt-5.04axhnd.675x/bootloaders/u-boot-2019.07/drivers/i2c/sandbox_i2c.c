// SPDX-License-Identifier: GPL-2.0+
/*
 * Simulate an I2C port
 *
 * Copyright (c) 2014 Google, Inc
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <i2c.h>
#include <asm/test.h>
#include <dm/lists.h>
#include <dm/device-internal.h>

struct sandbox_i2c_priv {
	bool test_mode;
};

static int get_emul(struct udevice *dev, struct udevice **devp,
		    struct dm_i2c_ops **opsp)
{
	struct dm_i2c_chip *plat;
	int ret;

	*devp = NULL;
	*opsp = NULL;
	plat = dev_get_parent_platdata(dev);
	if (!plat->emul) {
		ret = i2c_emul_find(dev, &plat->emul);
		if (ret)
			return ret;
	}
	*devp = plat->emul;
	*opsp = i2c_get_ops(plat->emul);

	return 0;
}

void sandbox_i2c_set_test_mode(struct udevice *bus, bool test_mode)
{
	struct sandbox_i2c_priv *priv = dev_get_priv(bus);

	priv->test_mode = test_mode;
}

static int sandbox_i2c_xfer(struct udevice *bus, struct i2c_msg *msg,
			    int nmsgs)
{
	struct dm_i2c_bus *i2c = dev_get_uclass_priv(bus);
	struct sandbox_i2c_priv *priv = dev_get_priv(bus);
	struct dm_i2c_ops *ops;
	struct udevice *emul, *dev;
	bool is_read;
	int ret;

	/* Special test code to return success but with no emulation */
	if (priv->test_mode && msg->addr == SANDBOX_I2C_TEST_ADDR)
		return 0;

	ret = i2c_get_chip(bus, msg->addr, 1, &dev);
	if (ret)
		return ret;

	ret = get_emul(dev, &emul, &ops);
	if (ret)
		return ret;

	if (priv->test_mode) {
		/*
		* For testing, don't allow writing above 100KHz for writes and
		* 400KHz for reads.
		*/
		is_read = nmsgs > 1;
		if (i2c->speed_hz > (is_read ? 400000 : 100000)) {
			debug("%s: Max speed exceeded\n", __func__);
			return -EINVAL;
		}
	}

	return ops->xfer(emul, msg, nmsgs);
}

static const struct dm_i2c_ops sandbox_i2c_ops = {
	.xfer		= sandbox_i2c_xfer,
};

static const struct udevice_id sandbox_i2c_ids[] = {
	{ .compatible = "sandbox,i2c" },
	{ }
};

U_BOOT_DRIVER(i2c_sandbox) = {
	.name	= "i2c_sandbox",
	.id	= UCLASS_I2C,
	.of_match = sandbox_i2c_ids,
	.ops	= &sandbox_i2c_ops,
	.priv_auto_alloc_size = sizeof(struct sandbox_i2c_priv),
};
