// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) 2015 Hans de Goede <hdegoede@redhat.com>
 */

/*
 * Support for the ANX9804 bridge chip, which can take pixel data coming
 * from a parallel LCD interface and translate it on the flight into a DP
 * interface for driving eDP TFT displays.
 */

#include <common.h>
#include <i2c.h>
#include <linux/delay.h>
#include "anx98xx-edp.h"
#include "anx9804.h"

/**
 * anx9804_init() - Init anx9804 parallel lcd to edp bridge chip
 *
 * This function will init an anx9804 parallel lcd to dp bridge chip
 * using the passed in parameters.
 *
 * @i2c_bus:	Device of the i2c bus to which the anx9804 is connected.
 * @lanes:	Number of displayport lanes to use
 * @data_rate:	Register value for the bandwidth reg 0x06: 1.62G, 0x0a: 2.7G
 * @bpp:	Bits per pixel, must be 18 or 24
 */
void anx9804_init(struct udevice *i2c_bus, u8 lanes, u8 data_rate, int bpp)
{
	struct udevice *chip0, *chip1;
	int c, colordepth, i, ret;

	ret = i2c_get_chip(i2c_bus, 0x38, 1, &chip0);
	if (ret)
		return;

	ret = i2c_get_chip(i2c_bus, 0x39, 1, &chip1);
	if (ret)
		return;

	if (bpp == 18)
		colordepth = 0x00; /* 6 bit */
	else
		colordepth = 0x10; /* 8 bit */

	/* Reset */
	dm_i2c_reg_write(chip1, ANX9804_RST_CTRL_REG, 1);
	mdelay(100);
	dm_i2c_reg_write(chip1, ANX9804_RST_CTRL_REG, 0);

	/* Write 0 to the powerdown reg (powerup everything) */
	dm_i2c_reg_write(chip1, ANX9804_POWERD_CTRL_REG, 0);

	c = dm_i2c_reg_read(chip1, ANX9804_DEV_IDH_REG);
	if (c != 0x98) {
		printf("Error anx9804 chipid mismatch\n");
		return;
	}

	for (i = 0; i < 100; i++) {
		c = dm_i2c_reg_read(chip0, ANX9804_SYS_CTRL2_REG);
		dm_i2c_reg_write(chip0, ANX9804_SYS_CTRL2_REG, c);
		c = dm_i2c_reg_read(chip0, ANX9804_SYS_CTRL2_REG);
		if ((c & ANX9804_SYS_CTRL2_CHA_STA) == 0)
			break;

		mdelay(5);
	}
	if (i == 100)
		printf("Error anx9804 clock is not stable\n");

	dm_i2c_reg_write(chip1, ANX9804_VID_CTRL2_REG, colordepth);

	/* Set a bunch of analog related register values */
	dm_i2c_reg_write(chip0, ANX9804_PLL_CTRL_REG, 0x07);
	dm_i2c_reg_write(chip1, ANX9804_PLL_FILTER_CTRL3, 0x19);
	dm_i2c_reg_write(chip1, ANX9804_PLL_CTRL3, 0xd9);
	dm_i2c_reg_write(chip1, ANX9804_RST_CTRL2_REG, ANX9804_RST_CTRL2_AC_MODE);
	dm_i2c_reg_write(chip1, ANX9804_ANALOG_DEBUG_REG1, 0xf0);
	dm_i2c_reg_write(chip1, ANX9804_ANALOG_DEBUG_REG3, 0x99);
	dm_i2c_reg_write(chip1, ANX9804_PLL_FILTER_CTRL1, 0x7b);
	dm_i2c_reg_write(chip0, ANX9804_LINK_DEBUG_REG, 0x30);
	dm_i2c_reg_write(chip1, ANX9804_PLL_FILTER_CTRL, 0x06);

	/* Force HPD */
	dm_i2c_reg_write(chip0, ANX9804_SYS_CTRL3_REG,
			 ANX9804_SYS_CTRL3_F_HPD | ANX9804_SYS_CTRL3_HPD_CTRL);

	/* Power up and configure lanes */
	dm_i2c_reg_write(chip0, ANX9804_ANALOG_POWER_DOWN_REG, 0x00);
	dm_i2c_reg_write(chip0, ANX9804_TRAINING_LANE0_SET_REG, 0x00);
	dm_i2c_reg_write(chip0, ANX9804_TRAINING_LANE1_SET_REG, 0x00);
	dm_i2c_reg_write(chip0, ANX9804_TRAINING_LANE2_SET_REG, 0x00);
	dm_i2c_reg_write(chip0, ANX9804_TRAINING_LANE3_SET_REG, 0x00);

	/* Reset AUX CH */
	dm_i2c_reg_write(chip1, ANX9804_RST_CTRL2_REG,
			 ANX9804_RST_CTRL2_AC_MODE | ANX9804_RST_CTRL2_AUX);
	dm_i2c_reg_write(chip1, ANX9804_RST_CTRL2_REG,
			 ANX9804_RST_CTRL2_AC_MODE);

	/* Powerdown audio and some other unused bits */
	dm_i2c_reg_write(chip1, ANX9804_POWERD_CTRL_REG, ANX9804_POWERD_AUDIO);
	dm_i2c_reg_write(chip0, ANX9804_HDCP_CONTROL_0_REG, 0x00);
	dm_i2c_reg_write(chip0, 0xa7, 0x00);

	/* Set data-rate / lanes */
	dm_i2c_reg_write(chip0, ANX9804_LINK_BW_SET_REG, data_rate);
	dm_i2c_reg_write(chip0, ANX9804_LANE_COUNT_SET_REG, lanes);

	/* Link training */
	dm_i2c_reg_write(chip0, ANX9804_LINK_TRAINING_CTRL_REG,
			 ANX9804_LINK_TRAINING_CTRL_EN);
	mdelay(5);
	for (i = 0; i < 100; i++) {
		c = dm_i2c_reg_read(chip0, ANX9804_LINK_TRAINING_CTRL_REG);
		if ((c & 0x01) == 0)
			break;

		mdelay(5);
	}
	if(i == 100) {
		printf("Error anx9804 link training timeout\n");
		return;
	}

	/* Enable */
	dm_i2c_reg_write(chip1, ANX9804_VID_CTRL1_REG,
			 ANX9804_VID_CTRL1_VID_EN | ANX9804_VID_CTRL1_EDGE);
	/* Force stream valid */
	dm_i2c_reg_write(chip0, ANX9804_SYS_CTRL3_REG,
			 ANX9804_SYS_CTRL3_F_HPD | ANX9804_SYS_CTRL3_HPD_CTRL |
			 ANX9804_SYS_CTRL3_F_VALID | ANX9804_SYS_CTRL3_VALID_CTRL);
}
