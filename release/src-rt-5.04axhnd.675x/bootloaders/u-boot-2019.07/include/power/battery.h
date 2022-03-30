/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  Copyright (C) 2012 Samsung Electronics
 *  Lukasz Majewski <l.majewski@samsung.com>
 */

#ifndef __POWER_BATTERY_H_
#define __POWER_BATTERY_H_

struct battery {
	unsigned int version;
	unsigned int state_of_chrg;
	unsigned int time_to_empty;
	unsigned int capacity;
	unsigned int voltage_uV;

	unsigned int state;
};

int power_bat_init(unsigned char bus);
#endif /* __POWER_BATTERY_H_ */
