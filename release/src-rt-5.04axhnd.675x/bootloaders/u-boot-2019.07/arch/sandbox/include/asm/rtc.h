/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Simulate an I2C real time clock
 *
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef __asm_rtc_h
#define __asm_rtc_h

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

	REG_COUNT	= 0x80,
};

#endif
