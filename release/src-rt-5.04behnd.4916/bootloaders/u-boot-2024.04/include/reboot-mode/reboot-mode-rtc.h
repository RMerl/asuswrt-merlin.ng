/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c), Vaisala Oyj
 */

#ifndef REBOOT_MODE_REBOOT_MODE_RTC_H_
#define REBOOT_MODE_REBOOT_MODE_RTC_H_

struct reboot_mode_rtc_platdata {
	struct udevice *rtc;
	bool is_big_endian;
	int addr;
	size_t size;
};

#endif /* REBOOT_MODE_REBOOT_MODE_RTC_H_ */
