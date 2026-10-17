// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * STMicroelectronics STUSB Type-C controller driver
 * based on Linux drivers/usb/typec/stusb160x.c
 *
 * Copyright (C) 2020, STMicroelectronics - All Rights Reserved
 */

#define LOG_CATEGORY UCLASS_I2C_GENERIC

#include <common.h>
#include <dm.h>
#include <i2c.h>

/* REGISTER */
#define STUSB160X_CC_CONNECTION_STATUS		0x0E

/* STUSB160X_CC_CONNECTION_STATUS bitfields */
#define STUSB160X_CC_ATTACH			BIT(0)

int stusb160x_cable_connected(void)
{
	struct udevice *dev;
	int ret;

	ret = uclass_get_device_by_driver(UCLASS_I2C_GENERIC,
					  DM_DRIVER_GET(stusb160x),
					  &dev);
	if (ret < 0)
		return ret;

	ret = dm_i2c_reg_read(dev, STUSB160X_CC_CONNECTION_STATUS);
	if (ret < 0)
		return 0;

	return ret & STUSB160X_CC_ATTACH;
}

static const struct udevice_id stusb160x_ids[] = {
	{ .compatible = "st,stusb1600" },
	{}
};

U_BOOT_DRIVER(stusb160x) = {
	.name = "stusb160x",
	.id = UCLASS_I2C_GENERIC,
	.of_match = stusb160x_ids,
};
