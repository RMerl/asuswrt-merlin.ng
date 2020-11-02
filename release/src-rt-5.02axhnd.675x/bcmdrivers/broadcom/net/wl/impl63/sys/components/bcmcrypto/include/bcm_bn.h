/*
 * bcm_bn.h
 *
 * Copyright (C) 2020, Broadcom. All Rights Reserved.
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
 *   <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: $
 */

/* This header defines the API support big numbers for use in ec etc. It is
 * a small subset of openssl functionality that provides the abstractions for
 * use by bcm firmware/software
 */

#ifndef _BCM_BN_H_
#define _BCM_BN_H_

#include <typedefs.h>
#ifdef BCMDRIVER
#include <osl.h>
#else
#include <stddef.h>
#endif // endif

/* API calls return BCME_* status codes unless otherwise specified */

/* opaque context for dealing with big nums */
typedef struct  bn_ctx bn_ctx_t;

/* opaque big num */
typedef struct bn bn_t;

/* allocation support */
typedef void* (*bn_alloc_fn_t)(void* ctx, int size);
typedef void (*bn_free_fn_t)(void* ctx, void *ptr, int size);

/* rng support - note: use of ctx unlike openssl */
typedef bn_t* (*bn_rand_fn_t)(void *ctx, bn_ctx_t *bn_ctx, int bit_len);

enum {
	BN_FMT_BE = 1,
	BN_FMT_LE = 2
};
typedef int8 bn_format_t;

/* create a bn context; numbers are expected to be of given bit length (at most) */
bn_ctx_t* bn_ctx_alloc(bn_alloc_fn_t alloc_fn, bn_free_fn_t free_fn, void *ctx, int max_bit_len);

/* free the context and initialize ctx to NULL */
void bn_ctx_destroy(bn_ctx_t **ctx);

/* create a bn; initialize with data from buf (length in bytes) or 0 if NULL.
 * Context used for bn is expected to be valid for the lifetime of bn.
 * length of bn in bits does not include zero MSB bits, if any
 */
bn_t* bn_alloc(bn_ctx_t *ctx, bn_format_t fmt, const uint8 *buf, int len);

/* destroy a bn and initialize bn to NULL */
void bn_free(bn_t **bn);

/* bn length in bits */
int bn_get_len_bits(const bn_t *bn);

/* truncate bn */
void bn_truncate(bn_t *bn, int len_bits);

/* shift bn - shift is negative for left shift */
void bn_shift(bn_t *bn, int shift);

/* init a bn with data from buffer */
int bn_set(bn_t *bn, bn_format_t fmt, const uint8 *buf, int buf_len);

/* return byte length of buffer needed to output bn in a given format */
int bn_get_len_bytes(const bn_t *bn, bn_format_t fmt);

/* serialize a bn, returns status BCME_* or output length >= 0 */
int bn_get(const bn_t *bn, bn_format_t fmt, uint8 *buf, int buf_len);

/* get context - context may be used for bn allocation */
bn_ctx_t* bn_get_ctx(bn_t *bn);

/* note: bn_rand is not defined here because caller can use a suitable PRNG (e.g h/w)
 * to generate enough randomness and initialize bn or use a callback
 */

/* arithmetic */

/* operations may use bn_ctx corresponding to non-const operands to allocate and
 * free additional bns' if necessary.
 */

/* r = a + b mod m; b may be negative; m may be NULL */
void bn_iadd(bn_t *r, const bn_t *a, int b, const bn_t *m);

/* r = a * b mod m; m may be NULL */
void bn_imul(bn_t *r, const bn_t *a, uint b, const bn_t *m);

/* r = a + b mod m; m may be NULL */
void bn_add(bn_t *r, const bn_t *a, const bn_t *b, const bn_t *m);

/* r = a - b mod m; m may be NULL */
void bn_sub(bn_t *r, const bn_t *a, const bn_t *b, const bn_t *m);

/* r = a * b mod m ;m may be NULL */
void bn_mul(bn_t *r, const bn_t *a, const bn_t *b, const bn_t *m);

/* sqaure r = a * a; m may be NULL */
void bn_sqr(bn_t *r, const bn_t *a, const bn_t *m);

/* mod: r = a mod m */
void bn_mod(bn_t *r, const bn_t *a, const bn_t *m);

/* inverse a.inv = 1 mod (n) - used by ecdsa, for e.g. */
void bn_inv(const bn_t *a, const bn_t *n, bn_t *inv);

/* other operations as necessary */

/* tbd  - bn generally used BE format. conversion function if needed */

/* compare - returns -1, 0, or +1 if a < b, a == b or a > b respectively */
int bn_cmp(const bn_t *a, const bn_t *b);

/* copy */
int bn_copy(bn_t *dst, const bn_t *src);

#endif /* _BCM_BN_H_ */
