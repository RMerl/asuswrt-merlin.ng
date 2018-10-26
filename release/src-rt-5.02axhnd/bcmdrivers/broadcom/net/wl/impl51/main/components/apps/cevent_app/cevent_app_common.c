/*
 * Cevent app common utils
 *
 *
 * Copyright (C) 2018, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 *
 * $Id $
 */

#include "cevent_app_common.h"

void
ca_hexdump_ascii(const char *title, const unsigned char *buf, unsigned int len)
{
	int i, llen;
	const unsigned char *pos = buf;
	const int line_len = 16;

	printf("%s - (data len=%lu):\n", title, (unsigned long) len);
	while (len) {
		llen = len > line_len ? line_len : len;
		printf("    ");
		for (i = 0; i < llen; i++)
			printf(" %02x", pos[i]);
		for (i = llen; i < line_len; i++)
			printf("   ");
		printf("   ");
		for (i = 0; i < llen; i++) {
			if (isprint(pos[i])) {
				printf("%c", pos[i]);
			} else {
				printf("*");
			}
		}
		for (i = llen; i < line_len; i++)
			printf(" ");
		printf("\n");
		pos += llen;
		len -= llen;
	}
}

/* get current time in ms */
uint64
ca_get_curr_time()
{
	struct timespec tp;
	uint64 curr_ts = 0;

	if (clock_gettime(CLOCK_REALTIME, &tp) < 0) {
		fprintf(stderr, "%s: clock_gettime failed\n", __FUNCTION__);
		return BCME_ERROR;
	}

	curr_ts = (uint64)tp.tv_sec*1000LL + (uint64)tp.tv_nsec/1000000LL;

	return curr_ts;
}

/**
 * Wrapper on snprintf returns exactly used number of bytes excluding trailing null byte.
 * Note: More importantly, this avoids snprintf return value crossing size limit.
 * Max return value of this funciton is (size -1).
 */
int
ca_snprintf(char *str, size_t size, const char *format, ...)
{
	int ret;
	va_list va;

	va_start(va, format);
	ret = vsnprintf(str, size, format, va);
	va_end(va);

	return (ret < size ? ret : (size - 1));
}

/* Writes size bytes from buffer (buf) to file descriptor 'fd' and handles write failures signals */
int
ca_swrite(int fd, char *buf, unsigned int size)
{
	int ret = 0, len = 0;

	do {
		errno = 0;
		ret = write(fd, &buf[len], size - len);
		if (ret < 0) {
			if (errno == EINTR) {
				continue;
			} else {
				break;
			}
		}
		if (ret > 0) {
			len += ret;
		}
	} while (len < size);

	return ((len > 0) ? len : ret);
}

/* Reads size bytes into buffer (buf) from file descriptor 'fd' and handles read failures signals */
int
ca_sread(int fd, char *buf, unsigned int size)
{
	int ret = 0, len = 0;

	do {
		errno = 0;
		ret = read(fd, &buf[len], size - len);
		if (ret < 0) {
			if (errno == EINTR) {
				continue;
			} else {
				break;
			}
		}
		if (ret > 0) {
			len += ret;
		}
	} while ((len < size) && ret);

	return ((len > 0) ? len : ret);
}
