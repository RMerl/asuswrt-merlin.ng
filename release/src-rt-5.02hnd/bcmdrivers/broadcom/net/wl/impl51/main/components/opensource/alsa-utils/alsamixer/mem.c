/*
 * mem.c - memory allocation checkers
 * Copyright (c) Clemens Ladisch <clemens@ladisch.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define _GNU_SOURCE
#include "aconfig.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include "die.h"
#include "mem.h"

static void check(void *p)
{
	if (!p)
		fatal_error("out of memory");
}

void *ccalloc(size_t n, size_t size)
{
	void *mem = calloc(n, size);
	if (n && size)
		check(mem);
	return mem;
}

void *crealloc(void *ptr, size_t new_size)
{
	ptr = realloc(ptr, new_size);
	if (new_size)
		check(ptr);
	return ptr;
}

char *cstrdup(const char *s)
{
	char *str = strdup(s);
	check(str);
	return str;
}

char *casprintf(const char *fmt, ...)
{
	va_list ap;
	char *str;

	va_start(ap, fmt);
	if (vasprintf(&str, fmt, ap) < 0)
		check(NULL);
	va_end(ap);
	return str;
}
