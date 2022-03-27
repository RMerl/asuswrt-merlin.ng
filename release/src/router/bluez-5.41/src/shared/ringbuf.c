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
#include <string.h>
#include <sys/uio.h>
#include <sys/param.h>

#include "src/shared/util.h"
#include "src/shared/ringbuf.h"

#ifndef MIN
#define MIN(x,y) ((x)<(y)?(x):(y))
#endif

struct ringbuf {
	void *buffer;
	size_t size;
	size_t in;
	size_t out;
	ringbuf_tracing_func_t in_tracing;
	void *in_data;
};

#define RINGBUF_RESET 0

/* Find last (most siginificant) set bit */
static inline unsigned int fls(unsigned int x)
{
	return x ? sizeof(x) * 8 - __builtin_clz(x) : 0;
}

/* Round up to nearest power of two */
static inline unsigned int align_power2(unsigned int u)
{
	return 1 << fls(u - 1);
}

struct ringbuf *ringbuf_new(size_t size)
{
	struct ringbuf *ringbuf;
	size_t real_size;

	if (size < 2 || size > UINT_MAX)
		return NULL;

	/* Find the next power of two for size */
	real_size = align_power2(size);

	ringbuf = new0(struct ringbuf, 1);
	ringbuf->buffer = malloc(real_size);
	if (!ringbuf->buffer) {
		free(ringbuf);
		return NULL;
	}

	ringbuf->size = real_size;
	ringbuf->in = RINGBUF_RESET;
	ringbuf->out = RINGBUF_RESET;

	return ringbuf;
}

void ringbuf_free(struct ringbuf *ringbuf)
{
	if (!ringbuf)
		return;

	free(ringbuf->buffer);
	free(ringbuf);
}

bool ringbuf_set_input_tracing(struct ringbuf *ringbuf,
			ringbuf_tracing_func_t callback, void *user_data)
{
	if (!ringbuf)
		return false;

	ringbuf->in_tracing = callback;
	ringbuf->in_data = user_data;

	return true;
}

size_t ringbuf_capacity(struct ringbuf *ringbuf)
{
	if (!ringbuf)
		return 0;

	return ringbuf->size;
}

size_t ringbuf_len(struct ringbuf *ringbuf)
{
	if (!ringbuf)
		return 0;

	return ringbuf->in - ringbuf->out;
}

size_t ringbuf_drain(struct ringbuf *ringbuf, size_t count)
{
	size_t len;

	if (!ringbuf)
		return 0;

	len = MIN(count, ringbuf->in - ringbuf->out);
	if (!len)
		return 0;

	ringbuf->out += len;

	if (ringbuf->out == ringbuf->in) {
		ringbuf->in = RINGBUF_RESET;
		ringbuf->out = RINGBUF_RESET;
	}

	return len;
}

void *ringbuf_peek(struct ringbuf *ringbuf, size_t offset, size_t *len_nowrap)
{
	if (!ringbuf)
		return NULL;

	offset = (ringbuf->out + offset) & (ringbuf->size - 1);

	if (len_nowrap) {
		size_t len = ringbuf->in - ringbuf->out;
		*len_nowrap = MIN(len, ringbuf->size - offset);
	}

	return ringbuf->buffer + offset;
}

ssize_t ringbuf_write(struct ringbuf *ringbuf, int fd)
{
	size_t len, offset, end;
	struct iovec iov[2];
	ssize_t consumed;

	if (!ringbuf || fd < 0)
		return -1;

	/* Determine how much data is available */
	len = ringbuf->in - ringbuf->out;
	if (!len)
		return 0;

	/* Grab data from buffer starting at offset until the end */
	offset = ringbuf->out & (ringbuf->size - 1);
	end = MIN(len, ringbuf->size - offset);

	iov[0].iov_base = ringbuf->buffer + offset;
	iov[0].iov_len = end;

	/* Use second vector for remainder from the beginning */
	iov[1].iov_base = ringbuf->buffer;
	iov[1].iov_len = len - end;

	consumed = writev(fd, iov, 2);
	if (consumed < 0)
		return -1;

	ringbuf->out += consumed;

	if (ringbuf->out == ringbuf->in) {
		ringbuf->in = RINGBUF_RESET;
		ringbuf->out = RINGBUF_RESET;
	}

	return consumed;
}

size_t ringbuf_avail(struct ringbuf *ringbuf)
{
	if (!ringbuf)
		return 0;

	return ringbuf->size - ringbuf->in + ringbuf->out;
}

int ringbuf_printf(struct ringbuf *ringbuf, const char *format, ...)
{
	va_list ap;
	int len;

	va_start(ap, format);
	len = ringbuf_vprintf(ringbuf, format, ap);
	va_end(ap);

	return len;
}

int ringbuf_vprintf(struct ringbuf *ringbuf, const char *format, va_list ap)
{
	size_t avail, offset, end;
	char *str;
	int len;

	if (!ringbuf || !format)
		return -1;

	/* Determine maximum length available for string */
	avail = ringbuf->size - ringbuf->in + ringbuf->out;
	if (!avail)
		return -1;

	len = vasprintf(&str, format, ap);
	if (len < 0)
		return -1;

	if ((size_t) len > avail) {
		free(str);
		return -1;
	}

	/* Determine possible length of string before wrapping */
	offset = ringbuf->in & (ringbuf->size - 1);
	end = MIN((size_t) len, ringbuf->size - offset);
	memcpy(ringbuf->buffer + offset, str, end);

	if (ringbuf->in_tracing)
		ringbuf->in_tracing(ringbuf->buffer + offset, end,
							ringbuf->in_data);

	if (len - end > 0) {
		/* Put the remainder of string at the beginning */
		memcpy(ringbuf->buffer, str + end, len - end);

		if (ringbuf->in_tracing)
			ringbuf->in_tracing(ringbuf->buffer, len - end,
							ringbuf->in_data);
	}

	free(str);

	ringbuf->in += len;

	return len;
}

ssize_t ringbuf_read(struct ringbuf *ringbuf, int fd)
{
	size_t avail, offset, end;
	struct iovec iov[2];
	ssize_t consumed;

	if (!ringbuf || fd < 0)
		return -1;

	/* Determine how much can actually be consumed */
	avail = ringbuf->size - ringbuf->in + ringbuf->out;
	if (!avail)
		return -1;

	/* Determine how much to consume before wrapping */
	offset = ringbuf->in & (ringbuf->size - 1);
	end = MIN(avail, ringbuf->size - offset);

	iov[0].iov_base = ringbuf->buffer + offset;
	iov[0].iov_len = end;

	/* Now put the remainder into the second vector */
	iov[1].iov_base = ringbuf->buffer;
	iov[1].iov_len = avail - end;

	consumed = readv(fd, iov, 2);
	if (consumed < 0)
		return -1;

	if (ringbuf->in_tracing) {
		size_t len = MIN((size_t) consumed, end);
		ringbuf->in_tracing(ringbuf->buffer + offset, len,
							ringbuf->in_data);
		if (consumed - len > 0)
			ringbuf->in_tracing(ringbuf->buffer, consumed - len,
							ringbuf->in_data);
	}

	ringbuf->in += consumed;

	return consumed;
}
