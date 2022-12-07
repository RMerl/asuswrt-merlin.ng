/*
 * utils.c
 *
 * Copyright (C) 2009 Hector Martin <hector@marcansoft.com>
 * Copyright (C) 2009 Nikias Bassen <nikias@gmx.li>
 * Copyright (c) 2013 Federico Mena Quintero
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 2.1 of the
 * License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#ifdef __APPLE__
#include <mach/mach_time.h>
#endif

#include "utils.h"

#include "log.h"
#define util_error(...) usbmuxd_log(LL_ERROR, __VA_ARGS__)

void fdlist_create(struct fdlist *list)
{
	list->count = 0;
	list->capacity = 4;
	list->owners = malloc(sizeof(*list->owners) * list->capacity);
	list->fds = malloc(sizeof(*list->fds) * list->capacity);
}
void fdlist_add(struct fdlist *list, enum fdowner owner, int fd, short events)
{
	if(list->count == list->capacity) {
		list->capacity *= 2;
		list->owners = realloc(list->owners, sizeof(*list->owners) * list->capacity);
		list->fds = realloc(list->fds, sizeof(*list->fds) * list->capacity);
	}
	list->owners[list->count] = owner;
	list->fds[list->count].fd = fd;
	list->fds[list->count].events = events;
	list->fds[list->count].revents = 0;
	list->count++;
}

void fdlist_free(struct fdlist *list)
{
	list->count = 0;
	list->capacity = 0;
	free(list->owners);
	list->owners = NULL;
	free(list->fds);
	list->fds = NULL;
}

void fdlist_reset(struct fdlist *list)
{
	list->count = 0;
}

#define CAPACITY_STEP 8

void collection_init(struct collection *col)
{
	col->list = malloc(sizeof(void *) * CAPACITY_STEP);
	memset(col->list, 0, sizeof(void *) * CAPACITY_STEP);
	col->capacity = CAPACITY_STEP;
}

void collection_free(struct collection *col)
{
	free(col->list);
	col->list = NULL;
	col->capacity = 0;
}

void collection_add(struct collection *col, void *element)
{
	int i;
	for(i=0; i<col->capacity; i++) {
		if(!col->list[i]) {
			col->list[i] = element;
			return;
		}
	}
	col->list = realloc(col->list, sizeof(void*) * (col->capacity + CAPACITY_STEP));
	memset(&col->list[col->capacity], 0, sizeof(void *) * CAPACITY_STEP);
	col->list[col->capacity] = element;
	col->capacity += CAPACITY_STEP;
}

void collection_remove(struct collection *col, void *element)
{
	int i;
	for(i=0; i<col->capacity; i++) {
		if(col->list[i] == element) {
			col->list[i] = NULL;
			return;
		}
	}
	util_error("collection_remove: element %p not present in collection %p (cap %d)", element, col, col->capacity);
}

int collection_count(struct collection *col)
{
	int i, cnt = 0;
	for(i=0; i<col->capacity; i++) {
		if(col->list[i])
			cnt++;
	}
	return cnt;
}

void collection_copy(struct collection *dest, struct collection *src)
{
	if (!dest || !src) return;
	dest->capacity = src->capacity;
	dest->list = malloc(sizeof(void*) * src->capacity);
	memcpy(dest->list, src->list, sizeof(void*) * src->capacity);
}

#ifndef HAVE_STPCPY
/**
 * Copy characters from one string into another
 *
 * @note: The strings should not overlap, as the behavior is undefined.
 *
 * @s1: The source string.
 * @s2: The destination string.
 *
 * @return a pointer to the terminating `\0' character of @s1,
 * or NULL if @s1 or @s2 is NULL.
 */
char *stpcpy(char * s1, const char * s2)
{
	if (s1 == NULL || s2 == NULL)
		return NULL;

	strcpy(s1, s2);

	return s1 + strlen(s2);
}
#endif

#if 0
/**
 * Concatenate strings into a newly allocated string
 *
 * @note: Specify NULL for the last string in the varargs list
 *
 * @str: The first string in the list
 * @...: Subsequent strings.  Use NULL for the last item.
 *
 * @return a newly allocated string, or NULL if @str is NULL.  This will also
 * return NULL and set errno to ENOMEM if memory is exhausted.
 */
char *string_concat(const char *str, ...)
{
	size_t len;
	va_list args;
	char *s;
	char *result;
	char *dest;

	if (!str)
		return NULL;

	/* Compute final length */

	len = strlen(str) + 1; /* plus 1 for the null terminator */

	va_start(args, str);
	s = va_arg(args, char *);
	while (s) {
		len += strlen(s);
		s = va_arg(args, char*);
	}
	va_end(args);

	/* Concat each string */

	result = malloc(len);
	if (!result)
		return NULL; /* errno remains set */

	dest = result;

	dest = stpcpy(dest, str);

	va_start(args, str);
	s = va_arg(args, char *);
	while (s) {
		dest = stpcpy(dest, s);
		s = va_arg(args, char *);
	}
	va_end(args);

	return result;
}

int buffer_read_from_filename(const char *filename, char **buffer, uint64_t *length)
{
	FILE *f;
	uint64_t size;

	*length = 0;

	f = fopen(filename, "rb");
	if (!f) {
		return 0;
	}

	fseek(f, 0, SEEK_END);
	size = ftell(f);
	rewind(f);

	if (size == 0) {
		fclose(f);
		return 0;
	}

	*buffer = (char*)malloc(sizeof(char)*(size+1));

	if (!buffer) {
		return 0;
	}

	int ret = 1;
	if (fread(*buffer, sizeof(char), size, f) != size) {
		usbmuxd_log(LL_ERROR, "%s: ERROR: couldn't read %d bytes from %s", __func__, (int)size, filename);
		free(*buffer);
		ret = 0;
		errno = EIO;
	}
	fclose(f);

	*length = size;
	return ret;
}

int buffer_write_to_filename(const char *filename, const char *buffer, uint64_t length)
{
	FILE *f;

	f = fopen(filename, "wb");
	if (f) {
		size_t written = fwrite(buffer, sizeof(char), length, f);
		fclose(f);

		if (written == length) {
			return 1;
		}
		else {
			// Not all data could be written.
			errno = EIO;
			return 0;
		}
	}
	else {
		// Failed to open the file, let the caller know.
		return 0;
	}
}

int plist_read_from_filename(plist_t *plist, const char *filename)
{
	char *buffer = NULL;
	uint64_t length;

	if (!filename)
		return 0;

	if (!buffer_read_from_filename(filename, &buffer, &length)) {
		return 0;
	}

	if ((length > 8) && (memcmp(buffer, "bplist00", 8) == 0)) {
		plist_from_bin(buffer, length, plist);
	} else {
		plist_from_xml(buffer, length, plist);
	}

	free(buffer);

	return 1;
}

int plist_write_to_filename(plist_t plist, const char *filename, enum plist_format_t format)
{
	char *buffer = NULL;
	uint32_t length;

	if (!plist || !filename)
		return 0;

	if (format == PLIST_FORMAT_XML)
		plist_to_xml(plist, &buffer, &length);
	else if (format == PLIST_FORMAT_BINARY)
		plist_to_bin(plist, &buffer, &length);
	else
		return 0;

	int res  = buffer_write_to_filename(filename, buffer, length);

	free(buffer);

	return res;
}
#endif

#ifndef HAVE_CLOCK_GETTIME
typedef int clockid_t;
#define CLOCK_MONOTONIC 1

static int clock_gettime(clockid_t clk_id, struct timespec *ts)
{
	// See http://developer.apple.com/library/mac/qa/qa1398

	uint64_t mach_time, nano_sec;

	static mach_timebase_info_data_t base_info;

	mach_time = mach_absolute_time();

	if (base_info.denom == 0) {
		(void) mach_timebase_info(&base_info);
	}

	if (base_info.numer == 1 && base_info.denom == 1)
		nano_sec = mach_time;
	else
		nano_sec = mach_time * base_info.numer / base_info.denom;

	ts->tv_sec = nano_sec / 1000000000;
	ts->tv_nsec = nano_sec % 1000000000;

	return 0;
}
#endif

void get_tick_count(struct timeval * tv)
{
	struct timespec ts;
	if(0 == clock_gettime(CLOCK_MONOTONIC, &ts)) {
		tv->tv_sec = ts.tv_sec;
		tv->tv_usec = ts.tv_nsec / 1000;
	} else {
		gettimeofday(tv, NULL);
	}
}

/**
 * Get number of milliseconds since the epoch.
 */
uint64_t mstime64(void)
{
	struct timeval tv;
	get_tick_count(&tv);

	// Careful, avoid overflow on 32 bit systems
	// time_t could be 4 bytes
	return ((long long)tv.tv_sec) * 1000LL + ((long long)tv.tv_usec) / 1000LL;
}
