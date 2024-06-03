// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 National Instruments
 */

#include <common.h>
#include <dm.h>
#include <serial.h>

static int nulldev_serial_setbrg(struct udevice *dev, int baudrate)
{
	return 0;
}

static int nulldev_serial_getc(struct udevice *dev)
{
	return -EAGAIN;
}

static int nulldev_serial_pending(struct udevice *dev, bool input)
{
	return 0;
}

static int nulldev_serial_input(struct udevice *dev)
{
	return 0;
}

static int nulldev_serial_putc(struct udevice *dev, const char ch)
{
	return 0;
}

static const struct udevice_id nulldev_serial_ids[] = {
	{ .compatible = "nulldev-serial" },
	{ }
};


const struct dm_serial_ops nulldev_serial_ops = {
	.putc = nulldev_serial_putc,
	.pending = nulldev_serial_pending,
	.getc = nulldev_serial_getc,
	.setbrg = nulldev_serial_setbrg,
};

U_BOOT_DRIVER(serial_nulldev) = {
	.name	= "serial_nulldev",
	.id	= UCLASS_SERIAL,
	.of_match = nulldev_serial_ids,
	.ops	= &nulldev_serial_ops,
};
