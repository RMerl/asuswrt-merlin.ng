/*
 * max77686.h - Driver for the Maxim 77686/802
 *
 *  Copyright (C) 2012 Samsung Electrnoics
 *  Chiwoong Byun <woong.byun@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * This driver is based on max8997.h
 *
 * MAX77686 has PMIC, RTC devices.
 * The devices share the same I2C bus and included in
 * this mfd driver.
 */

#ifndef __LINUX_MFD_MAX77686_H
#define __LINUX_MFD_MAX77686_H

#include <linux/regulator/consumer.h>

/* MAX77686 regulator IDs */
enum max77686_regulators {
	MAX77686_LDO1 = 0,
	MAX77686_LDO2,
	MAX77686_LDO3,
	MAX77686_LDO4,
	MAX77686_LDO5,
	MAX77686_LDO6,
	MAX77686_LDO7,
	MAX77686_LDO8,
	MAX77686_LDO9,
	MAX77686_LDO10,
	MAX77686_LDO11,
	MAX77686_LDO12,
	MAX77686_LDO13,
	MAX77686_LDO14,
	MAX77686_LDO15,
	MAX77686_LDO16,
	MAX77686_LDO17,
	MAX77686_LDO18,
	MAX77686_LDO19,
	MAX77686_LDO20,
	MAX77686_LDO21,
	MAX77686_LDO22,
	MAX77686_LDO23,
	MAX77686_LDO24,
	MAX77686_LDO25,
	MAX77686_LDO26,
	MAX77686_BUCK1,
	MAX77686_BUCK2,
	MAX77686_BUCK3,
	MAX77686_BUCK4,
	MAX77686_BUCK5,
	MAX77686_BUCK6,
	MAX77686_BUCK7,
	MAX77686_BUCK8,
	MAX77686_BUCK9,

	MAX77686_REG_MAX,
};

/* MAX77802 regulator IDs */
enum max77802_regulators {
	MAX77802_BUCK1 = 0,
	MAX77802_BUCK2,
	MAX77802_BUCK3,
	MAX77802_BUCK4,
	MAX77802_BUCK5,
	MAX77802_BUCK6,
	MAX77802_BUCK7,
	MAX77802_BUCK8,
	MAX77802_BUCK9,
	MAX77802_BUCK10,
	MAX77802_LDO1,
	MAX77802_LDO2,
	MAX77802_LDO3,
	MAX77802_LDO4,
	MAX77802_LDO5,
	MAX77802_LDO6,
	MAX77802_LDO7,
	MAX77802_LDO8,
	MAX77802_LDO9,
	MAX77802_LDO10,
	MAX77802_LDO11,
	MAX77802_LDO12,
	MAX77802_LDO13,
	MAX77802_LDO14,
	MAX77802_LDO15,
	MAX77802_LDO17,
	MAX77802_LDO18,
	MAX77802_LDO19,
	MAX77802_LDO20,
	MAX77802_LDO21,
	MAX77802_LDO23,
	MAX77802_LDO24,
	MAX77802_LDO25,
	MAX77802_LDO26,
	MAX77802_LDO27,
	MAX77802_LDO28,
	MAX77802_LDO29,
	MAX77802_LDO30,
	MAX77802_LDO32,
	MAX77802_LDO33,
	MAX77802_LDO34,
	MAX77802_LDO35,

	MAX77802_REG_MAX,
};

enum max77686_opmode {
	MAX77686_OPMODE_NORMAL,
	MAX77686_OPMODE_LP,
	MAX77686_OPMODE_STANDBY,
};

struct max77686_opmode_data {
	int id;
	int mode;
};

#endif /* __LINUX_MFD_MAX77686_H */
