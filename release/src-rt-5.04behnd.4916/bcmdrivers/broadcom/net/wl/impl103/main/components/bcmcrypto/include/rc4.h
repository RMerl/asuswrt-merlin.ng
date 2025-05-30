/*
 * rc4.h
 * RC4 stream cipher
 *
 * Copyright (C) 2024, Broadcom. All Rights Reserved.
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
 * $Id: rc4.h 679390 2017-01-14 00:13:47Z $
 */

#ifndef _RC4_H_
#define _RC4_H_

#include <typedefs.h>

#define RC4_STATE_NBYTES 256

typedef struct rc4_ks {
	uchar state[RC4_STATE_NBYTES];
	uchar x;
	uchar y;
} rc4_ks_t;

void prepare_key(uchar *key_data_ptr, int key_data_len, rc4_ks_t *key);

void rc4(uchar *buffer_ptr, int buffer_len, rc4_ks_t *key);

#endif /* _RC4_H_ */
