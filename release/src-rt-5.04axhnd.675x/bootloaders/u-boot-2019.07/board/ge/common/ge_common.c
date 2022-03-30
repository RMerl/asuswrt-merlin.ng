// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2017 General Electric Company
 */

#include <common.h>
#include <i2c.h>
#include <rtc.h>

void check_time(void)
{
	int ret, i;
	struct rtc_time tm;
	u8 retry = 3;

	unsigned int current_i2c_bus = i2c_get_bus_num();

	ret = i2c_set_bus_num(CONFIG_SYS_RTC_BUS_NUM);
	if (ret < 0)
		return;

	rtc_init();

	for (i = 0; i < retry; i++) {
		ret = rtc_get(&tm);
		if (!ret || ret == -EINVAL)
			break;
	}

	if (ret < 0)
		env_set("rtc_status", "RTC_ERROR");

	if (tm.tm_year > 2037) {
		tm.tm_sec  = 0;
		tm.tm_min  = 0;
		tm.tm_hour = 0;
		tm.tm_mday = 1;
		tm.tm_wday = 2;
		tm.tm_mon  = 1;
		tm.tm_year = 2036;

		for (i = 0; i < retry; i++) {
			ret = rtc_set(&tm);
			if (!ret)
				break;
		}

		if (ret < 0)
			env_set("rtc_status", "RTC_ERROR");
	}

	i2c_set_bus_num(current_i2c_bus);
}

