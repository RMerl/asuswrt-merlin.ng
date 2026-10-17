// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2017 General Electric Company
 */

#include <common.h>
#include <env.h>
#include <dm/uclass.h>
#include <rtc.h>

void check_time(void)
{
	struct udevice *dev;
	int ret, i;
	struct rtc_time tm;
	u8 retry = 3;

	ret = uclass_get_device(UCLASS_RTC, 0, &dev);
	if (ret) {
		env_set("rtc_status", "FAIL");
		return;
	}

	for (i = 0; i < retry; i++) {
		ret = dm_rtc_get(dev, &tm);
		if (!ret || ret == -EINVAL)
			break;
	}

	if (!ret && tm.tm_year > 2037) {
		tm.tm_sec  = 0;
		tm.tm_min  = 0;
		tm.tm_hour = 0;
		tm.tm_mday = 1;
		tm.tm_wday = 2;
		tm.tm_mon  = 1;
		tm.tm_year = 2036;

		for (i = 0; i < retry; i++) {
			ret = dm_rtc_set(dev, &tm);
			if (!ret)
				break;
		}

		if (ret >= 0)
			ret = 2038;
	}

	if (ret < 0)
		env_set("rtc_status", "FAIL");
	else if (ret == 2038)
		env_set("rtc_status", "2038");
	else
		env_set("rtc_status", "OK");
}
