/*
 * dld - debugability support for dumping logs to file (debug log dump)
 *
 * Copyright (C) 2023, Broadcom. All Rights Reserved.
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
 * $Id$
 */
#ifndef __DLD_H__
#define __DLD_H__

#include <osl.h>
#include <bcmutils.h>

typedef enum {
	DLD_BUF_GENERAL = 0,
	DLD_BUF_MAX
} dld_buf_type_t;

typedef struct dld_buf_alloc_info {
	dld_buf_type_t type;
	uint size;
	char *name;
} dld_buf_alloc_info_t;

void *dld_init(osl_t *osh);
void dld_deinit(void *info);
int dld_buf_init(void *info, const dld_buf_alloc_info_t *allocinfo, uint input_count);
void dld_buf_deinit(void *info, dld_buf_type_t type);
void dld_write(void* info, dld_buf_type_t type, const char *fmt, ...);
int dld_get_buf(void *info, dld_buf_type_t type, char *buf, uint *size);

#define _DLD_REMOVE_PAREN(...) __VA_ARGS__
#define DLD_REMOVE_PAREN(args) _DLD_REMOVE_PAREN args

#define DLD_WRITE_TO_BUF(info, type, args) \
	dld_write(info, type, DLD_REMOVE_PAREN(args))

#endif /* __DLD_H__ */
