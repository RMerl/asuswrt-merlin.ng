/*
 * Panasonic MN88472 DVB-T/T2/C demodulator driver
 *
 * Copyright (C) 2013 Antti Palosaari <crope@iki.fi>
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 */

#ifndef MN88472_H
#define MN88472_H

#include <linux/dvb/frontend.h>

enum ts_clock {
	VARIABLE_TS_CLOCK,
	FIXED_TS_CLOCK,
};

enum ts_mode {
	SERIAL_TS_MODE,
	PARALLEL_TS_MODE,
};

struct mn88472_config {
	/*
	 * Max num of bytes given I2C adapter could write at once.
	 * Default: none
	 */
	u16 i2c_wr_max;


	/* Everything after that is returned by the driver. */

	/*
	 * DVB frontend.
	 */
	struct dvb_frontend **fe;

	/*
	 * Xtal frequency.
	 * Hz
	 */
	u32 xtal;
	int ts_mode;
	int ts_clock;
};

#endif
