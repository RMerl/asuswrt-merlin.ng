// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018 NXP.
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <i2c.h>
#include <asm/io.h>

#include "emc2305.h"

DECLARE_GLOBAL_DATA_PTR;

void set_fan_speed(u8 data)
{
	u8 index;
	u8 Fan[NUM_OF_FANS] = {I2C_EMC2305_FAN1,
			       I2C_EMC2305_FAN2,
			       I2C_EMC2305_FAN3,
			       I2C_EMC2305_FAN4,
			       I2C_EMC2305_FAN5};

	for (index = 0; index < NUM_OF_FANS; index++) {
		if (i2c_write(I2C_EMC2305_ADDR, Fan[index], 1, &data, 1) != 0) {
			printf("Error: failed to change fan speed @%x\n",
			       Fan[index]);
		}
	}
}

void emc2305_init(void)
{
	u8 data;

	data = I2C_EMC2305_CMD;
	if (i2c_write(I2C_EMC2305_ADDR, I2C_EMC2305_CONF, 1, &data, 1) != 0)
		printf("Error: failed to configure EMC2305\n");
}
