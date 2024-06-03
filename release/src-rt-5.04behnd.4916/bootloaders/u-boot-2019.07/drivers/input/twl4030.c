// SPDX-License-Identifier: GPL-2.0+
/*
 * TWL4030 input
 *
 * Copyright (C) 2015 Paul Kocialkowski <contact@paulk.fr>
 */

#include <twl4030.h>

int twl4030_input_power_button(void)
{
	u8 data;

	twl4030_i2c_read_u8(TWL4030_CHIP_PM_MASTER,
			    TWL4030_PM_MASTER_STS_HW_CONDITIONS, &data);

	if (data & TWL4030_PM_MASTER_STS_HW_CONDITIONS_PWON)
		return 1;

	return 0;
}

int twl4030_input_charger(void)
{
	u8 data;

	twl4030_i2c_read_u8(TWL4030_CHIP_PM_MASTER,
			    TWL4030_PM_MASTER_STS_HW_CONDITIONS, &data);

	if (data & TWL4030_PM_MASTER_STS_HW_CONDITIONS_CHG)
		return 1;

	return 0;
}

int twl4030_input_usb(void)
{
	u8 data;

	twl4030_i2c_read_u8(TWL4030_CHIP_PM_MASTER,
			    TWL4030_PM_MASTER_STS_HW_CONDITIONS, &data);

	if (data & TWL4030_PM_MASTER_STS_HW_CONDITIONS_USB ||
	    data & TWL4030_PM_MASTER_STS_HW_CONDITIONS_VBUS)
		return 1;

	return 0;
}

int twl4030_keypad_scan(unsigned char *matrix)
{
	u8 data;
	u8 c, r;

	twl4030_i2c_read_u8(TWL4030_CHIP_KEYPAD,
			    TWL4030_KEYPAD_KEYP_CTRL_REG, &data);

	data |= TWL4030_KEYPAD_CTRL_SOFT_NRST | TWL4030_KEYPAD_CTRL_KBD_ON;
	data &= ~TWL4030_KEYPAD_CTRL_SOFTMODEN;

	twl4030_i2c_write_u8(TWL4030_CHIP_KEYPAD,
			     TWL4030_KEYPAD_KEYP_CTRL_REG, data);

	for (c = 0; c < 8; c++) {
		data = 0xff & ~(1 << c);
		twl4030_i2c_write_u8(TWL4030_CHIP_KEYPAD,
				     TWL4030_KEYPAD_KBC_REG, data);

		data = 0xff;
		twl4030_i2c_read_u8(TWL4030_CHIP_KEYPAD,
				    TWL4030_KEYPAD_KBR_REG, &data);

		for (r = 0; r < 8; r++)
			matrix[c * 8 + r] = !(data & (1 << r));
	}

	data = 0xff & ~(TWL4030_KEYPAD_CTRL_SOFT_NRST);
	twl4030_i2c_write_u8(TWL4030_CHIP_KEYPAD,
			     TWL4030_KEYPAD_KEYP_CTRL_REG, data);

	return 0;
}

int twl4030_keypad_key(unsigned char *matrix, u8 c, u8 r)
{
	return matrix[c * 8 + r];
}
