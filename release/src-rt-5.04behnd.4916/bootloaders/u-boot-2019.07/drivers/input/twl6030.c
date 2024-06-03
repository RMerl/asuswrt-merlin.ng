// SPDX-License-Identifier: GPL-2.0+
/*
 * TWL6030 input
 *
 * Copyright (C) 2016 Paul Kocialkowski <contact@paulk.fr>
 */

#include <twl6030.h>

int twl6030_input_power_button(void)
{
	u8 value;

	twl6030_i2c_read_u8(TWL6030_CHIP_PM, TWL6030_STS_HW_CONDITIONS, &value);

	/* Power button is active low. */
	if (value & TWL6030_STS_HW_CONDITIONS_PWRON)
		return 0;

	return 1;
}

int twl6030_input_charger(void)
{
	u8 value;

	twl6030_i2c_read_u8(TWL6030_CHIP_CHARGER, TWL6030_CONTROLLER_STAT1,
		&value);

	if (value & TWL6030_CONTROLLER_STAT1_VAC_DET)
		return 1;

	return 0;
}

int twl6030_input_usb(void)
{
	u8 value;

	twl6030_i2c_read_u8(TWL6030_CHIP_CHARGER, TWL6030_CONTROLLER_STAT1,
		&value);

	if (value & TWL6030_CONTROLLER_STAT1_VBUS_DET)
		return 1;

	return 0;
}
