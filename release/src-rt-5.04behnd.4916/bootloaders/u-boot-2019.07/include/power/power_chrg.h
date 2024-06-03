/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  Copyright (C) 2012 Samsung Electronics
 *  Lukasz Majewski <l.majewski@samsung.com>
 */

#ifndef __POWER_CHARGER_H_
#define __POWER_CHARGER_H_

/* Type of available chargers */
enum {
	CHARGER_NO = 0,
	CHARGER_TA,
	CHARGER_USB,
	CHARGER_TA_500,
	CHARGER_UNKNOWN,
};

enum {
	UNKNOWN,
	EXT_SOURCE,
	CHARGE,
	NORMAL,
};

#endif /* __POWER_CHARGER_H_ */
