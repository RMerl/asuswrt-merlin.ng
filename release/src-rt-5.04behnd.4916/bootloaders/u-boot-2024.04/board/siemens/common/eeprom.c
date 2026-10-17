// SPDX-License-Identifier: GPL-2.0+
/*
 *
 * Read EEPROM data
 * (C) Copyright 2024 Siemens AG
 */

#include <dm/uclass.h>
#include <i2c.h>
#include "eeprom.h"

#if CONFIG_IS_ENABLED(DM_I2C)
static struct udevice *i2c_dev;
#endif

/* Probe I2C and set-up EEPROM */
int siemens_ee_setup(void)
{
#if CONFIG_IS_ENABLED(DM_I2C)
	struct udevice *bus;
	int ret;

	ret = uclass_get_device_by_seq(UCLASS_I2C, SIEMENS_EE_I2C_BUS, &bus);
	if (ret)
		goto err;

	ret = dm_i2c_probe(bus, SIEMENS_EE_I2C_ADDR, 0, &i2c_dev);
	if (ret)
		goto err;
	if (i2c_set_chip_offset_len(i2c_dev, 2))
		goto err;
#else
	i2c_set_bus_num(SIEMENS_EE_I2C_BUS);
	if (i2c_probe(SIEMENS_EE_I2C_ADDR))
		goto err;
#endif
	return 0;

err:
	printf("Could not probe the EEPROM; something fundamentally wrong on the I2C bus.\n");
	return 1;
}

/* Read data from EEPROM */
int siemens_ee_read_data(uint address, uchar *buffer, int len)
{
#if CONFIG_IS_ENABLED(DM_I2C)
	return dm_i2c_read(i2c_dev, address, buffer, len);
#else
	return i2c_read(SIEMENS_EE_I2C_ADDR, address, 2, buffer, len);
#endif
}
