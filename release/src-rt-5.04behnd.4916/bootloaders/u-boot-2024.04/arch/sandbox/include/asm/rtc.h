/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Simulate an I2C real time clock
 *
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef __asm_rtc_h
#define __asm_rtc_h

#include <dt-structs.h>

/* Register numbers in the sandbox RTC */
enum {
	REG_SEC		= 5,
	REG_MIN,
	REG_HOUR,
	REG_MDAY,
	REG_MON,
	REG_YEAR,
	REG_WDAY,

	REG_RESET	= 0x20,

	REG_AUX0	= 0x30,
	REG_AUX1,
	REG_AUX2,
	REG_AUX3,

	REG_COUNT	= 0x80,
};

/**
 * struct sandbox_i2c_rtc_plat_data - platform data for the RTC
 *
 * @base_time:		Base system time when RTC device was bound
 * @offset:		RTC offset from current system time
 * @use_system_time:	true to use system time, false to use @base_time
 * @reg:		Register values
 */
struct sandbox_i2c_rtc_plat_data {
#if CONFIG_IS_ENABLED(OF_PLATDATA) && IS_ENABLED(CONFIG_RTC_SANDBOX)
	struct dtd_sandbox_i2c_rtc_emul dtplat;
#endif
	long base_time;
	long offset;
	bool use_system_time;
	u8 reg[REG_COUNT];
};

struct sandbox_i2c_rtc {
	unsigned int offset_secs;
};

#endif
