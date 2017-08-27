/*
 * misc.c	Miscellaneous TIPC helper functions.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Richard Alpe <richard.alpe@ericsson.com>
 */

#include <stdio.h>
#include <stdint.h>
#include <linux/tipc.h>

#include "misc.h"

#define IN_RANGE(val, low, high) ((val) <= (high) && (val) >= (low))

uint32_t str2addr(char *str)
{
	unsigned int z, c, n;
	char dummy;

	if (sscanf(str, "%u.%u.%u%c", &z, &c, &n, &dummy) != 3) {
		fprintf(stderr, "invalid network address, syntax: Z.C.N\n");
		return 0;
	}

	if (IN_RANGE(z, 0, 255) && IN_RANGE(c, 0, 4095) && IN_RANGE(n, 0, 4095))
		return tipc_addr(z, c, n);

	fprintf(stderr, "invalid network address \"%s\"\n", str);
	return 0;
}
