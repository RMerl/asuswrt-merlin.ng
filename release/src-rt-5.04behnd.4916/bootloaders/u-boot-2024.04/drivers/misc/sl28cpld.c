// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2021 Michael Walle <michael@walle.cc>
 */

#include <common.h>
#include <dm.h>
#include <i2c.h>

struct sl28cpld_child_plat {
	uint offset;
};

/*
 * The access methods works either with the first argument being a child
 * device or with the MFD device itself.
 */
static int sl28cpld_read_child(struct udevice *dev, uint offset)
{
	struct sl28cpld_child_plat *plat = dev_get_parent_plat(dev);
	struct udevice *mfd = dev_get_parent(dev);

	return dm_i2c_reg_read(mfd, offset + plat->offset);
}

int sl28cpld_read(struct udevice *dev, uint offset)
{
	if (dev->driver == DM_DRIVER_GET(sl28cpld))
		return dm_i2c_reg_read(dev, offset);
	else
		return sl28cpld_read_child(dev, offset);
}

static int sl28cpld_write_child(struct udevice *dev, uint offset,
				uint8_t value)
{
	struct sl28cpld_child_plat *plat = dev_get_parent_plat(dev);
	struct udevice *mfd = dev_get_parent(dev);

	return dm_i2c_reg_write(mfd, offset + plat->offset, value);
}

int sl28cpld_write(struct udevice *dev, uint offset, uint8_t value)
{
	if (dev->driver == DM_DRIVER_GET(sl28cpld))
		return dm_i2c_reg_write(dev, offset, value);
	else
		return sl28cpld_write_child(dev, offset, value);
}

int sl28cpld_update(struct udevice *dev, uint offset, uint8_t clear,
		    uint8_t set)
{
	int val;

	val = sl28cpld_read(dev, offset);
	if (val < 0)
		return val;

	val &= ~clear;
	val |= set;

	return sl28cpld_write(dev, offset, val);
}

static int sl28cpld_probe(struct udevice *dev)
{
	i2c_set_chip_flags(dev, DM_I2C_CHIP_RD_ADDRESS |
			   DM_I2C_CHIP_WR_ADDRESS);

	return 0;
}

static int sl28cpld_child_post_bind(struct udevice *dev)
{
	struct sl28cpld_child_plat *plat = dev_get_parent_plat(dev);
	int offset;

	if (!dev_has_ofnode(dev))
		return 0;

	offset = dev_read_u32_default(dev, "reg", -1);
	if (offset == -1)
		return -EINVAL;

	plat->offset = offset;

	return 0;
}

static const struct udevice_id sl28cpld_ids[] = {
	{ .compatible = "kontron,sl28cpld" },
	{}
};

U_BOOT_DRIVER(sl28cpld) = {
	.name		= "sl28cpld",
	.id		= UCLASS_NOP,
	.of_match	= sl28cpld_ids,
	.probe		= sl28cpld_probe,
	.bind		= dm_scan_fdt_dev,
	.flags		= DM_FLAG_PRE_RELOC,
	.per_child_plat_auto = sizeof(struct sl28cpld_child_plat),
	.child_post_bind = sl28cpld_child_post_bind,
};
