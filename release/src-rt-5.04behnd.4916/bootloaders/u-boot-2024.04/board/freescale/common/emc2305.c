// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018-2020 NXP.
 *
 */

#include <common.h>
#include <command.h>
#include <i2c.h>
#include <asm/global_data.h>
#include <asm/io.h>

#include "emc2305.h"

DECLARE_GLOBAL_DATA_PTR;

void set_fan_speed(u8 data, int chip_addr)
{
	u8 index;
	u8 Fan[NUM_OF_FANS] = {I2C_EMC2305_FAN1,
			       I2C_EMC2305_FAN2,
			       I2C_EMC2305_FAN3,
			       I2C_EMC2305_FAN4,
			       I2C_EMC2305_FAN5};

	for (index = 0; index < NUM_OF_FANS; index++) {
#if !CONFIG_IS_ENABLED(DM_I2C)
		if (i2c_write(chip_addr, Fan[index], 1, &data, 1) != 0) {
			printf("Error: failed to change fan speed @%x\n",
			       Fan[index]);
		}
#else
		struct udevice *dev;

		if (i2c_get_chip_for_busnum(0, chip_addr, 1, &dev))
			continue;

		if (dm_i2c_write(dev, Fan[index], &data, 1) != 0) {
			printf("Error: failed to change fan speed @%x\n",
			       Fan[index]);
		}
#endif
	}
}

void emc2305_init(int chip_addr)
{
	u8 data;

	data = I2C_EMC2305_CMD;
#if !CONFIG_IS_ENABLED(DM_I2C)
	if (i2c_write(chip_addr, I2C_EMC2305_CONF, 1, &data, 1) != 0)
		printf("Error: failed to configure EMC2305\n");
#else
	struct udevice *dev;

	if (!i2c_get_chip_for_busnum(0, chip_addr, 1, &dev))
		if (dm_i2c_write(dev, I2C_EMC2305_CONF, &data, 1))
			printf("Error: failed to configure EMC2305\n");
#endif

}
