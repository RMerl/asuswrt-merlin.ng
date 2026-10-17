/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2014 Google, Inc
 */

#ifndef __serial_pl01x_h
#define __serial_pl01x_h

enum pl01x_type {
	TYPE_PL010,
	TYPE_PL011,
};

/*
 *Information about a serial port
 *
 * @base: Register base address
 * @type: Port type
 * @clock: Input clock rate, used for calculating the baud rate divisor
 * @skip_init: Don't attempt to change port configuration (also means @clock
 * is ignored)
 */
#include <dt-structs.h>
struct pl01x_serial_plat {
#if CONFIG_IS_ENABLED(OF_PLATDATA)
	struct dtd_serial_pl01x dtplat;
#endif
	unsigned long base;
	enum pl01x_type type;
	unsigned int clock;
	bool skip_init;
};

#endif
