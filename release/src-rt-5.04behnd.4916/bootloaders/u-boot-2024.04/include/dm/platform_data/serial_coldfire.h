/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2015  Angelo Dureghello <angelo@sysam.it>
 */

#ifndef __serial_coldfire_h
#define __serial_coldfire_h

/*
 * struct coldfire_serial_plat - information about a coldfire port
 *
 * @base:               Uart port base register address
 * @port:               Uart port index, for cpu with pinmux for uart / gpio
 * baudrtatre:          Uart port baudrate
 */
struct coldfire_serial_plat {
	unsigned long base;
	int port;
	int baudrate;
};

#endif /* __serial_coldfire_h */
