// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2023
 * Philip Richard Oberfichtner <pro@denx.de>
 *
 * Based on previous work from Heiko Schocher (legacy bootcount_i2c.c driver)
 */

#include <bootcount.h>
#include <dm.h>
#include <i2c.h>

#define BC_MAGIC	0x55

struct bootcount_i2c_priv {
	struct udevice *bcdev;
	unsigned int offset;
};

static int bootcount_i2c_set(struct udevice *dev, const u32 val)
{
	int ret;
	struct bootcount_i2c_priv *priv = dev_get_priv(dev);

	ret = dm_i2c_reg_write(priv->bcdev, priv->offset, BC_MAGIC);
	if (ret < 0)
		goto err_exit;

	ret = dm_i2c_reg_write(priv->bcdev, priv->offset + 1, val & 0xff);
	if (ret < 0)
		goto err_exit;

	return 0;

err_exit:
	log_debug("%s: Error writing to I2C device (%d)\n", __func__, ret);
	return ret;
}

static int bootcount_i2c_get(struct udevice *dev, u32 *val)
{
	int ret;
	struct bootcount_i2c_priv *priv = dev_get_priv(dev);

	ret = dm_i2c_reg_read(priv->bcdev, priv->offset);
	if (ret < 0)
		goto err_exit;

	if ((ret & 0xff) != BC_MAGIC) {
		log_debug("%s: Invalid Magic, reset bootcounter.\n", __func__);
		*val = 0;
		return bootcount_i2c_set(dev, 0);
	}

	ret = dm_i2c_reg_read(priv->bcdev, priv->offset + 1);
	if (ret < 0)
		goto err_exit;

	*val = ret;
	return 0;

err_exit:
	log_debug("%s: Error reading from I2C device (%d)\n", __func__, ret);
	return ret;
}

static int bootcount_i2c_probe(struct udevice *dev)
{
	struct bootcount_i2c_priv *priv = dev_get_priv(dev);
	int ret;

	ret = dev_read_u32(dev, "offset", &priv->offset);
	if (ret)
		goto exit;

	ret = i2c_get_chip_by_phandle(dev, "i2cbcdev", &priv->bcdev);

exit:
	if (ret)
		log_debug("%s failed, ret = %d\n", __func__, ret);

	return ret;
}

static const struct bootcount_ops bootcount_i2c_ops = {
	.get = bootcount_i2c_get,
	.set = bootcount_i2c_set,
};

static const struct udevice_id bootcount_i2c_ids[] = {
	{ .compatible = "u-boot,bootcount-i2c" },
	{ }
};

U_BOOT_DRIVER(bootcount_i2c) = {
	.name		= "bootcount-i2c",
	.id		= UCLASS_BOOTCOUNT,
	.priv_auto	= sizeof(struct bootcount_i2c_priv),
	.probe		= bootcount_i2c_probe,
	.of_match	= bootcount_i2c_ids,
	.ops		= &bootcount_i2c_ops,
};
