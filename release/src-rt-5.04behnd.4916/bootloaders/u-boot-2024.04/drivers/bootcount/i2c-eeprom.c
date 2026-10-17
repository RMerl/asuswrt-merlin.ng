// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2019 Collabora
 * (C) Copyright 2019 GE
 */

#include <common.h>
#include <bootcount.h>
#include <dm.h>
#include <i2c_eeprom.h>
#include <log.h>

static const u8 bootcount_magic = 0xbc;

struct bootcount_i2c_eeprom_priv {
	struct udevice *i2c_eeprom;
	u32 offset;
};

static int bootcount_i2c_eeprom_set(struct udevice *dev, const u32 a)
{
	struct bootcount_i2c_eeprom_priv *priv = dev_get_priv(dev);
	const u16 val = bootcount_magic << 8 | (a & 0xff);

	if (i2c_eeprom_write(priv->i2c_eeprom, priv->offset,
			     (uint8_t *)&val, 2) < 0) {
		debug("%s: write failed\n", __func__);
		return -EIO;
	}

	return 0;
}

static int bootcount_i2c_eeprom_get(struct udevice *dev, u32 *a)
{
	struct bootcount_i2c_eeprom_priv *priv = dev_get_priv(dev);
	u16 val;

	if (i2c_eeprom_read(priv->i2c_eeprom, priv->offset,
			    (uint8_t *)&val, 2) < 0) {
		debug("%s: read failed\n", __func__);
		return -EIO;
	}

	if (val >> 8 == bootcount_magic) {
		*a = val & 0xff;
		return 0;
	}

	debug("%s: bootcount magic does not match on %04x\n", __func__, val);
	return -EIO;
}

static int bootcount_i2c_eeprom_probe(struct udevice *dev)
{
	struct ofnode_phandle_args phandle_args;
	struct bootcount_i2c_eeprom_priv *priv = dev_get_priv(dev);
	struct udevice *i2c_eeprom;

	if (dev_read_phandle_with_args(dev, "i2c-eeprom", NULL, 0, 0,
				       &phandle_args)) {
		debug("%s: i2c-eeprom backing device not specified\n",
		      dev->name);
		return -ENOENT;
	}

	if (uclass_get_device_by_ofnode(UCLASS_I2C_EEPROM, phandle_args.node,
					&i2c_eeprom)) {
		debug("%s: could not get backing device\n", dev->name);
		return -ENODEV;
	}

	priv->i2c_eeprom = i2c_eeprom;
	priv->offset = dev_read_u32_default(dev, "offset", 0);

	return 0;
}

static const struct bootcount_ops bootcount_i2c_eeprom_ops = {
	.get = bootcount_i2c_eeprom_get,
	.set = bootcount_i2c_eeprom_set,
};

static const struct udevice_id bootcount_i2c_eeprom_ids[] = {
	{ .compatible = "u-boot,bootcount-i2c-eeprom" },
	{ }
};

U_BOOT_DRIVER(bootcount_spi_flash) = {
	.name	= "bootcount-i2c-eeprom",
	.id	= UCLASS_BOOTCOUNT,
	.priv_auto	= sizeof(struct bootcount_i2c_eeprom_priv),
	.probe	= bootcount_i2c_eeprom_probe,
	.of_match = bootcount_i2c_eeprom_ids,
	.ops	= &bootcount_i2c_eeprom_ops,
};
