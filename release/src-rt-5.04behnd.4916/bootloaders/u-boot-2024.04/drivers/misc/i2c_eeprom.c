// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2014 Google, Inc
 */

#define LOG_CATEGORY UCLASS_I2C_EEPROM

#include <common.h>
#include <eeprom.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/kernel.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <i2c.h>
#include <i2c_eeprom.h>

struct i2c_eeprom_drv_data {
	u32 size; /* size in bytes */
	u32 pagesize; /* page size in bytes */
	u32 addr_offset_mask; /* bits in addr used for offset overflow */
	u32 offset_len; /* size in bytes of offset */
	u32 start_offset; /* valid start offset inside memory, by default 0 */
};

int i2c_eeprom_read(struct udevice *dev, int offset, uint8_t *buf, int size)
{
	const struct i2c_eeprom_ops *ops = device_get_ops(dev);

	if (!ops->read)
		return -ENOSYS;

	return ops->read(dev, offset, buf, size);
}

int i2c_eeprom_write(struct udevice *dev, int offset, const uint8_t *buf,
		     int size)
{
	const struct i2c_eeprom_ops *ops = device_get_ops(dev);

	if (!ops->write)
		return -ENOSYS;

	return ops->write(dev, offset, buf, size);
}

int i2c_eeprom_size(struct udevice *dev)
{
	const struct i2c_eeprom_ops *ops = device_get_ops(dev);

	if (!ops->size)
		return -ENOSYS;

	return ops->size(dev);
}

static int i2c_eeprom_std_read(struct udevice *dev, int offset, uint8_t *buf,
			       int size)
{
	return dm_i2c_read(dev, offset, buf, size);
}

static int i2c_eeprom_len(int offset, int len, int pagesize)
{
	int page_offset = offset & (pagesize - 1);
	int maxlen = pagesize - page_offset;

	if (len > maxlen)
		len = maxlen;

	return len;
}

static int i2c_eeprom_std_write(struct udevice *dev, int offset,
				const uint8_t *buf, int size)
{
	struct i2c_eeprom *priv = dev_get_priv(dev);
	int ret;

	while (size > 0) {
		int write_size = i2c_eeprom_len(offset, size, priv->pagesize);

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

static int i2c_eeprom_std_size(struct udevice *dev)
{
	struct i2c_eeprom *priv = dev_get_priv(dev);

	return priv->size;
}

static const struct i2c_eeprom_ops i2c_eeprom_std_ops = {
	.read	= i2c_eeprom_std_read,
	.write	= i2c_eeprom_std_write,
	.size	= i2c_eeprom_std_size,
};

static int i2c_eeprom_std_of_to_plat(struct udevice *dev)
{
	struct i2c_eeprom *priv = dev_get_priv(dev);
	struct i2c_eeprom_drv_data *data =
		(struct i2c_eeprom_drv_data *)dev_get_driver_data(dev);
	u32 pagesize;
	u32 size;

	if (dev_read_u32(dev, "pagesize", &pagesize) == 0)
		priv->pagesize = pagesize;
	else
		/* 6 bit -> page size of up to 2^63 (should be sufficient) */
		priv->pagesize = data->pagesize;

	if (dev_read_u32(dev, "size", &size) == 0)
		priv->size = size;
	else
		priv->size = data->size;

	return 0;
}

static int i2c_eeprom_std_bind(struct udevice *dev)
{
	ofnode partitions = ofnode_find_subnode(dev_ofnode(dev), "partitions");
	ofnode partition;
	const char *name;

	if (!ofnode_valid(partitions))
		return 0;
	if (!ofnode_device_is_compatible(partitions, "fixed-partitions"))
		return -ENOTSUPP;

	ofnode_for_each_subnode(partition, partitions) {
		name = ofnode_get_name(partition);
		if (!name)
			continue;

		device_bind(dev, DM_DRIVER_GET(i2c_eeprom_partition), name,
			    NULL, partition, NULL);
	}

	return 0;
}

static int i2c_eeprom_std_probe(struct udevice *dev)
{
	u8 test_byte;
	int ret;
	struct i2c_eeprom_drv_data *data =
		(struct i2c_eeprom_drv_data *)dev_get_driver_data(dev);

	i2c_set_chip_offset_len(dev, data->offset_len);
	i2c_set_chip_addr_offset_mask(dev, data->addr_offset_mask);

	/* Verify that the chip is functional */
	/*
	 * Not all eeproms start from offset 0. Valid offset is available
	 * in the platform data struct.
	 */
	ret = i2c_eeprom_read(dev, data->start_offset, &test_byte, 1);
	if (ret)
		return -ENODEV;

	return 0;
}

static const struct i2c_eeprom_drv_data eeprom_data = {
	.size = 0,
	.pagesize = 1,
	.addr_offset_mask = 0,
	.offset_len = 1,
};

static const struct i2c_eeprom_drv_data atmel24c01a_data = {
	.size = 128,
	.pagesize = 8,
	.addr_offset_mask = 0,
	.offset_len = 1,
};

static const struct i2c_eeprom_drv_data atmel24c02_data = {
	.size = 256,
	.pagesize = 8,
	.addr_offset_mask = 0,
	.offset_len = 1,
};

static const struct i2c_eeprom_drv_data atmel24c04_data = {
	.size = 512,
	.pagesize = 16,
	.addr_offset_mask = 0x1,
	.offset_len = 1,
};

static const struct i2c_eeprom_drv_data atmel24c08_data = {
	.size = 1024,
	.pagesize = 16,
	.addr_offset_mask = 0x3,
	.offset_len = 1,
};

static const struct i2c_eeprom_drv_data atmel24c08a_data = {
	.size = 1024,
	.pagesize = 16,
	.addr_offset_mask = 0x3,
	.offset_len = 1,
};

static const struct i2c_eeprom_drv_data atmel24c16a_data = {
	.size = 2048,
	.pagesize = 16,
	.addr_offset_mask = 0x7,
	.offset_len = 1,
};

static const struct i2c_eeprom_drv_data atmel24mac402_data = {
	.size = 256,
	.pagesize = 16,
	.addr_offset_mask = 0,
	.offset_len = 1,
	.start_offset = 0x80,
};

static const struct i2c_eeprom_drv_data atmel24c32_data = {
	.size = 4096,
	.pagesize = 32,
	.addr_offset_mask = 0,
	.offset_len = 2,
};

static const struct i2c_eeprom_drv_data atmel24c32d_wlp_data = {
	.size = 32,
	.pagesize = 32,
	.addr_offset_mask = 0,
	.offset_len = 2,
};

static const struct i2c_eeprom_drv_data atmel24c64_data = {
	.size = 8192,
	.pagesize = 32,
	.addr_offset_mask = 0,
	.offset_len = 2,
};

static const struct i2c_eeprom_drv_data atmel24c128_data = {
	.size = 16384,
	.pagesize = 64,
	.addr_offset_mask = 0,
	.offset_len = 2,
};

static const struct i2c_eeprom_drv_data atmel24c256_data = {
	.size = 32768,
	.pagesize = 64,
	.addr_offset_mask = 0,
	.offset_len = 2,
};

static const struct i2c_eeprom_drv_data atmel24c512_data = {
	.size = 65536,
	.pagesize = 64,
	.addr_offset_mask = 0,
	.offset_len = 2,
};

static const struct udevice_id i2c_eeprom_std_ids[] = {
	{ .compatible = "i2c-eeprom", (ulong)&eeprom_data },
	{ .compatible = "atmel,24c01", (ulong)&atmel24c01a_data },
	{ .compatible = "atmel,24c01a", (ulong)&atmel24c01a_data },
	{ .compatible = "atmel,24c02", (ulong)&atmel24c02_data },
	{ .compatible = "atmel,24c04", (ulong)&atmel24c04_data },
	{ .compatible = "atmel,24c08", (ulong)&atmel24c08_data },
	{ .compatible = "atmel,24c08a", (ulong)&atmel24c08a_data },
	{ .compatible = "atmel,24c16a", (ulong)&atmel24c16a_data },
	{ .compatible = "atmel,24mac402", (ulong)&atmel24mac402_data },
	{ .compatible = "atmel,24c32", (ulong)&atmel24c32_data },
	{ .compatible = "atmel,24c32d-wl", (ulong)&atmel24c32d_wlp_data },
	{ .compatible = "atmel,24c64", (ulong)&atmel24c64_data },
	{ .compatible = "atmel,24c128", (ulong)&atmel24c128_data },
	{ .compatible = "atmel,24c256", (ulong)&atmel24c256_data },
	{ .compatible = "atmel,24c512", (ulong)&atmel24c512_data },
	{ }
};

U_BOOT_DRIVER(i2c_eeprom_std) = {
	.name			= "i2c_eeprom",
	.id			= UCLASS_I2C_EEPROM,
	.of_match		= i2c_eeprom_std_ids,
	.bind			= i2c_eeprom_std_bind,
	.probe			= i2c_eeprom_std_probe,
	.of_to_plat	= i2c_eeprom_std_of_to_plat,
	.priv_auto	= sizeof(struct i2c_eeprom),
	.ops			= &i2c_eeprom_std_ops,
};

struct i2c_eeprom_partition {
	u32 offset;
	u32 size;
};

static int i2c_eeprom_partition_probe(struct udevice *dev)
{
	return 0;
}

static int i2c_eeprom_partition_of_to_plat(struct udevice *dev)
{
	struct i2c_eeprom_partition *priv = dev_get_priv(dev);
	u32 reg[2];
	int ret;

	ret = dev_read_u32_array(dev, "reg", reg, 2);
	if (ret)
		return ret;

	if (!reg[1])
		return -EINVAL;

	priv->offset = reg[0];
	priv->size = reg[1];

	debug("%s: base %x, size %x\n", __func__, priv->offset, priv->size);

	return 0;
}

static int i2c_eeprom_partition_read(struct udevice *dev, int offset,
				     u8 *buf, int size)
{
	struct i2c_eeprom_partition *priv = dev_get_priv(dev);
	struct udevice *parent = dev_get_parent(dev);

	if (!parent)
		return -ENODEV;
	if (offset + size > priv->size)
		return -EINVAL;

	return i2c_eeprom_read(parent, offset + priv->offset, buf, size);
}

static int i2c_eeprom_partition_write(struct udevice *dev, int offset,
				      const u8 *buf, int size)
{
	struct i2c_eeprom_partition *priv = dev_get_priv(dev);
	struct udevice *parent = dev_get_parent(dev);

	if (!parent)
		return -ENODEV;
	if (offset + size > priv->size)
		return -EINVAL;

	return i2c_eeprom_write(parent, offset + priv->offset, (uint8_t *)buf,
				size);
}

static int i2c_eeprom_partition_size(struct udevice *dev)
{
	struct i2c_eeprom_partition *priv = dev_get_priv(dev);

	return priv->size;
}

static const struct i2c_eeprom_ops i2c_eeprom_partition_ops = {
	.read	= i2c_eeprom_partition_read,
	.write	= i2c_eeprom_partition_write,
	.size	= i2c_eeprom_partition_size,
};

U_BOOT_DRIVER(i2c_eeprom_partition) = {
	.name			= "i2c_eeprom_partition",
	.id			= UCLASS_I2C_EEPROM,
	.probe			= i2c_eeprom_partition_probe,
	.of_to_plat	= i2c_eeprom_partition_of_to_plat,
	.priv_auto	= sizeof(struct i2c_eeprom_partition),
	.ops			= &i2c_eeprom_partition_ops,
};

UCLASS_DRIVER(i2c_eeprom) = {
	.id		= UCLASS_I2C_EEPROM,
	.name		= "i2c_eeprom",
};
