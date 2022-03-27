/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2012-2014  Intel Corporation. All rights reserved.
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <limits.h>
#include <string.h>

#include "src/shared/util.h"

void *btd_malloc(size_t size)
{
	if (__builtin_expect(!!size, 1)) {
		void *ptr;

		ptr = malloc(size);
		if (ptr)
			return ptr;

		fprintf(stderr, "failed to allocate %zu bytes\n", size);
		abort();
	}

	return NULL;
}

void util_debug(util_debug_func_t function, void *user_data,
						const char *format, ...)
{
	char str[78];
	va_list ap;

	if (!function || !format)
		return;

	usleep(20*1000);
	va_start(ap, format);
	vsnprintf(str, sizeof(str), format, ap);
	va_end(ap);

	function(str, user_data);
}

void util_hexdump(const char dir, const unsigned char *buf, size_t len,
				util_debug_func_t function, void *user_data)
{
	static const char hexdigits[] = "0123456789abcdef";
	char str[68];
	size_t i;

	if (!function || !len)
		return;

	str[0] = dir;

	for (i = 0; i < len; i++) {
		str[((i % 16) * 3) + 1] = ' ';
		str[((i % 16) * 3) + 2] = hexdigits[buf[i] >> 4];
		str[((i % 16) * 3) + 3] = hexdigits[buf[i] & 0xf];
		str[(i % 16) + 51] = isprint(buf[i]) ? buf[i] : '.';

		if ((i + 1) % 16 == 0) {
			str[49] = ' ';
			str[50] = ' ';
			str[67] = '\0';
			function(str, user_data);
			str[0] = ' ';
		}
	}

	if (i % 16 > 0) {
		size_t j;
		for (j = (i % 16); j < 16; j++) {
			str[(j * 3) + 1] = ' ';
			str[(j * 3) + 2] = ' ';
			str[(j * 3) + 3] = ' ';
			str[j + 51] = ' ';
		}
		str[49] = ' ';
		str[50] = ' ';
		str[67] = '\0';
		function(str, user_data);
	}
}

/* Helper for getting the dirent type in case readdir returns DT_UNKNOWN */
unsigned char util_get_dt(const char *parent, const char *name)
{
	char filename[PATH_MAX];
	struct stat st;

	snprintf(filename, sizeof(filename), "%s/%s", parent, name);
	if (lstat(filename, &st) == 0 && S_ISDIR(st.st_mode))
		return DT_DIR;

	return DT_UNKNOWN;
}

/* Helpers for bitfield operations */

/* Find unique id in range from 1 to max but no bigger then
 * sizeof(int) * 8. ffs() is used since it is POSIX standard
 */
uint8_t util_get_uid(unsigned int *bitmap, uint8_t max)
{
	uint8_t id;

	id = ffs(~*bitmap);

	if (!id || id > max)
		return 0;

	*bitmap |= 1 << (id - 1);

	return id;
}

/* Clear id bit in bitmap */
void util_clear_uid(unsigned int *bitmap, uint8_t id)
{
	if (!id)
		return;

	*bitmap &= ~(1 << (id - 1));
}
