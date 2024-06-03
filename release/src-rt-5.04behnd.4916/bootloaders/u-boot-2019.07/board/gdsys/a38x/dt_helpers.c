// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016
 * Mario Six, Guntermann & Drunck GmbH, mario.six@gdsys.cc
 */

#include <common.h>
#include <i2c.h>
#include <fdt_support.h>
#include <asm-generic/gpio.h>
#include <dm.h>

int fdt_disable_by_ofname(void *rw_fdt_blob, char *ofname)
{
	int offset = fdt_path_offset(rw_fdt_blob, ofname);

	return fdt_status_disabled(rw_fdt_blob, offset);
}

bool dm_i2c_simple_probe(struct udevice *bus, uint chip_addr)
{
	struct udevice *dev;

	return !dm_i2c_probe(bus, chip_addr, DM_I2C_CHIP_RD_ADDRESS |
			     DM_I2C_CHIP_WR_ADDRESS, &dev);
}

int request_gpio_by_name(struct gpio_desc *gpio, const char *gpio_dev_name,
			 uint offset, char *gpio_name)
{
	struct udevice *gpio_dev = NULL;

	if (uclass_get_device_by_name(UCLASS_GPIO, gpio_dev_name, &gpio_dev))
		return 1;

	gpio->dev = gpio_dev;
	gpio->offset = offset;
	gpio->flags = 0;

	return dm_gpio_request(gpio, gpio_name);
}

