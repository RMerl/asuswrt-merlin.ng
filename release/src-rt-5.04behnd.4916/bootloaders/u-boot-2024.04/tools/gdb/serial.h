/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2000
 * Murray Jensen <Murray.Jensen@csiro.au>
 */

#include <termios.h>

#define SERIAL_ERROR	-1	/* General error, see errno for details */
#define SERIAL_TIMEOUT	-2
#define SERIAL_EOF	-3

extern speed_t cvtspeed(char *);
extern int serialopen(char *, speed_t);
extern int serialreadchar(int, int);
extern int serialwrite(int, char *, int);
extern int serialclose(int);
