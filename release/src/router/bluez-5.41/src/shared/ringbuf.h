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

#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>

typedef void (*ringbuf_tracing_func_t)(const void *buf, size_t count,
							void *user_data);

struct ringbuf;

struct ringbuf *ringbuf_new(size_t size);
void ringbuf_free(struct ringbuf *ringbuf);

bool ringbuf_set_input_tracing(struct ringbuf *ringbuf,
			ringbuf_tracing_func_t callback, void *user_data);

size_t ringbuf_capacity(struct ringbuf *ringbuf);

size_t ringbuf_len(struct ringbuf *ringbuf);
size_t ringbuf_drain(struct ringbuf *ringbuf, size_t count);
void *ringbuf_peek(struct ringbuf *ringbuf, size_t offset, size_t *len_nowrap);
ssize_t ringbuf_write(struct ringbuf *ringbuf, int fd);

size_t ringbuf_avail(struct ringbuf *ringbuf);
int ringbuf_printf(struct ringbuf *ringbuf, const char *format, ...)
					__attribute__((format(printf, 2, 3)));
int ringbuf_vprintf(struct ringbuf *ringbuf, const char *format, va_list ap);
ssize_t ringbuf_read(struct ringbuf *ringbuf, int fd);
