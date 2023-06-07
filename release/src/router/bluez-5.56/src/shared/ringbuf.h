/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2012-2014  Intel Corporation. All rights reserved.
 *
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
