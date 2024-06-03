// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2014 Google, Inc
 */

#include <common.h>
#include <linux/err.h>
#include <linux/kernel.h>
#include <dm.h>
#include <i2c.h>
#include <i2c_eeprom.h>

int i2c_eeprom_read(struct udevice *dev, int offset, uint8_t *buf, int size)
{
	const struct i2c_eeprom_ops *ops = device_get_ops(dev);

	if (!ops->read)
		return -ENOSYS;

	return ops->read(dev, offset, buf, size);
}

int i2c_eeprom_write(struct udevice *dev, int offset, uint8_t *buf, int size)
{
	const struct i2c_eeprom_ops *ops = device_get_ops(dev);

	if (!ops->write)
		return -ENOSYS;

	return ops->write(dev, offset, buf, size);
}

static int i2c_eeprom_std_read(struct udevice *dev, int offset, uint8_t *buf,
			       int size)
{
	return dm_i2c_read(dev, offset, buf, size);
}

static int i2c_eeprom_std_write(struct udevice *dev, int offset,
				const uint8_t *buf, int size)
{
	struct i2c_eeprom *priv = dev_get_priv(dev);
	int ret;

	while (size > 0) {
		int write_size = min_t(int, size, priv->pagesize);

		ret = dm_i2c_write(dev, offset, buf, write_size);
		if (ret)
			return ret;

		offset += write_size;
		buf += write_size;
		size -= write_size;

		udelay(10000);
	}

	return 0;
}

static const struct i2c_eeprom_ops i2c_eeprom_std_ops = {
	.read	= i2c_eeprom_std_read,
	.write	= i2c_eeprom_std_write,
};

static int i2c_eeprom_std_ofdata_to_platdata(struct udevice *dev)
{
	struct i2c_eeprom *priv = dev_get_priv(dev);
	u64 data = dev_get_driver_data(dev);
	u32 pagesize;

	if (dev_read_u32(dev, "pagesize", &pagesize) == 0) {
		priv->pagesize = pagesize;
		return 0;
	}

	/* 6 bit -> page size of up to 2^63 (should be sufficient) */
	priv->pagewidth = data & 0x3F;
	priv->pagesize = (1 << priv->pagewidth);

	return 0;
}

static int i2c_eeprom_std_probe(struct udevice *dev)
{
	return 0;
}

static const struct udevice_id i2c_eeprom_std_ids[] = {
	{ .compatible = "i2c-eeprom", .data = 0 },
	{ .compatible = "microchip,24aa02e48", .data = 3 },
	{ .compatible = "atmel,24c01a", .data = 3 },
	{ .compatible = "atmel,24c02", .data = 3 },
	{ .compatible = "atmel,24c04", .data = 4 },
	{ .compatible = "atmel,24c08", .data = 4 },
	{ .compatible = "atmel,24c08a", .data = 4 },
	{ .compatible = "atmel,24c16a", .data = 4 },
	{ .compatible = "atmel,24mac402", .data = 4 },
	{ .compatible = "atmel,24c32", .data = 5 },
	{ .compatible = "atmel,24c64", .data = 5 },
	{ .compatible = "atmel,24c128", .data = 6 },
	{ .compatible = "atmel,24c256", .data = 6 },
	{ .compatible = "atmel,24c512", .data = 6 },
	{ }
};

U_BOOT_DRIVER(i2c_eeprom_std) = {
	.name			= "i2c_eeprom",
	.id			= UCLASS_I2C_EEPROM,
	.of_match		= i2c_eeprom_std_ids,
	.probe			= i2c_eeprom_std_probe,
	.ofdata_to_platdata	= i2c_eeprom_std_ofdata_to_platdata,
	.priv_auto_alloc_size	= sizeof(struct i2c_eeprom),
	.ops			= &i2c_eeprom_std_ops,
};

UCLASS_DRIVER(i2c_eeprom) = {
	.id		= UCLASS_I2C_EEPROM,
	.name		= "i2c_eeprom",
};
