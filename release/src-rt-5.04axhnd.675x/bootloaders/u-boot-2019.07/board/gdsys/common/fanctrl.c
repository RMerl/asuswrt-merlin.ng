// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015
 * Dirk Eibach,  Guntermann & Drunck GmbH, dirk.eibach@gdsys.cc
 */

#ifdef CONFIG_GDSYS_LEGACY_DRIVERS

#include <common.h>
#include <i2c.h>

enum {
	FAN_CONFIG = 0x03,
	FAN_TACHLIM_LSB = 0x48,
	FAN_TACHLIM_MSB = 0x49,
	FAN_PWM_FREQ = 0x4D,
};

void init_fan_controller(u8 addr)
{
	int val;

	/* set PWM Frequency to 2.5% resolution */
	i2c_reg_write(addr, FAN_PWM_FREQ, 20);

	/* set Tachometer Limit */
	i2c_reg_write(addr, FAN_TACHLIM_LSB, 0x10);
	i2c_reg_write(addr, FAN_TACHLIM_MSB, 0x0a);

	/* enable Tach input */
	val = i2c_reg_read(addr, FAN_CONFIG) | 0x04;
	i2c_reg_write(addr, FAN_CONFIG, val);
}

#endif /* CONFIG_GDSYS_LEGACY_DRIVERS */
