/*
 * Copyright (C) 2007, 2008 Nokia Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 * This file contains various common stuff.
 *
 * Authors: Artem Bityutskiy
 *          Adrian Hunter
 */

#define PROGRAM_NAME "mtd-utils"

#include <sys/time.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include "common.h"

/**
 * get_multiplier - convert size specifier to an integer multiplier.
 * @str: the size specifier string
 *
 * This function parses the @str size specifier, which may be one of
 * 'KiB', 'MiB', or 'GiB' into an integer multiplier. Returns positive
 * size multiplier in case of success and %-1 in case of failure.
 */
static int get_multiplier(const char *str)
{
	if (!str)
		return 1;

	/* Remove spaces before the specifier */
	while (*str == ' ' || *str == '\t')
		str += 1;

	if (!strcmp(str, "KiB"))
		return 1024;
	if (!strcmp(str, "MiB"))
		return 1024 * 1024;
	if (!strcmp(str, "GiB"))
		return 1024 * 1024 * 1024;

	return -1;
}

/**
 * util_get_bytes - convert a string containing amount of bytes into an
 * integer
 * @str: string to convert
 *
 * This function parses @str which may have one of 'KiB', 'MiB', or 'GiB'
 * size specifiers. Returns positive amount of bytes in case of success and %-1
 * in case of failure.
 */
long long util_get_bytes(const char *str)
{
	char *endp;
	long long bytes = strtoull(str, &endp, 0);

	if (endp == str || bytes < 0) {
		fprintf(stderr, "incorrect amount of bytes: \"%s\"\n", str);
		return -1;
	}

	if (*endp != '\0') {
		int mult = get_multiplier(endp);

		if (mult == -1) {
			fprintf(stderr, "bad size specifier: \"%s\" - "
			        "should be 'KiB', 'MiB' or 'GiB'\n", endp);
			return -1;
		}
		bytes *= mult;
	}

	return bytes;
}

/**
 * util_print_bytes - print bytes.
 * @bytes: variable to print
 * @bracket: whether brackets have to be put or not
 *
 * This is a helper function which prints amount of bytes in a human-readable
 * form, i.e., it prints the exact amount of bytes following by the approximate
 * amount of Kilobytes, Megabytes, or Gigabytes, depending on how big @bytes
 * is.
 */
void util_print_bytes(long long bytes, int bracket)
{
	const char *p;
	int GiB = 1024 * 1024 * 1024;
	int MiB = 1024 * 1024;
	int KiB = 1024;

	if (bracket)
		p = " (";
	else
		p = ", ";

	printf("%lld bytes", bytes);

	if (bytes > GiB)
		printf("%s%lld.%lld GiB", p,
		       bytes / GiB, bytes % GiB / (GiB / 10));
	else if (bytes > MiB)
		printf("%s%lld.%lld MiB", p,
		        bytes / MiB, bytes % MiB / (MiB / 10));
	else if (bytes > KiB && bytes != 0)
		printf("%s%lld.%lld KiB", p,
		        bytes / KiB, bytes % KiB / (KiB / 10));
	else
		return;

	if (bracket)
		printf(")");
}

/**
 * util_srand - randomly seed the standard pseudo-random generator.
 *
 * This helper function seeds the standard libc pseudo-random generator with a
 * more or less random value to make sure the 'rand()' call does not return the
 * same sequence every time UBI utilities run. Returns zero in case of success
 * and a %-1 in case of error.
 */
int util_srand(void)
{
	struct timeval tv;
	struct timezone tz;
	unsigned int seed;

	/*
	 * Just assume that a combination of the PID + current time is a
	 * reasonably random number.
	 */
	if (gettimeofday(&tv, &tz))
		return -1;

	seed = (unsigned int)tv.tv_sec;
	seed += (unsigned int)tv.tv_usec;
	seed *= getpid();
	seed %= RAND_MAX;
	srand(seed);
	return 0;
}
