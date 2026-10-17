// SPDX-License-Identifier:	GPL-2.0+
/*
 *
 * Copyright (c) 2015 Free Electrons
 * Copyright (c) 2015 NextThing Co
 * Copyright (c) 2018 Microchip Technology, Inc.
 *
 */

#include <common.h>
#include <linux/err.h>
#include <dm.h>
#include <w1-eeprom.h>
#include <w1.h>

#define W1_F2D_READ_EEPROM	0xf0

static int ds24xxx_read_buf(struct udevice *dev, unsigned int offset,
			    u8 *buf, unsigned int count)
{
	w1_reset_select(dev);

	w1_write_byte(dev, W1_F2D_READ_EEPROM);
	w1_write_byte(dev, offset & 0xff);
	w1_write_byte(dev, offset >> 8);

	return w1_read_buf(dev, buf, count);
}

static int ds24xxx_probe(struct udevice *dev)
{
	struct w1_device *w1;

	w1 = dev_get_parent_plat(dev);
	w1->id = 0;
	return 0;
}

static const struct w1_eeprom_ops ds24xxx_ops = {
	.read_buf	= ds24xxx_read_buf,
};

static const struct udevice_id ds24xxx_id[] = {
	{ .compatible = "maxim,ds24b33", .data = W1_FAMILY_DS24B33 },
	{ .compatible = "maxim,ds2431", .data = W1_FAMILY_DS2431 },
	{ },
};

U_BOOT_DRIVER(ds24xxx) = {
	.name		= "ds24xxx",
	.id		= UCLASS_W1_EEPROM,
	.of_match	= ds24xxx_id,
	.ops		= &ds24xxx_ops,
	.probe		= ds24xxx_probe,
};

u8 family_supported[] = {
	W1_FAMILY_DS24B33,
	W1_FAMILY_DS2431,
};

U_BOOT_W1_DEVICE(ds24xxx, family_supported);
