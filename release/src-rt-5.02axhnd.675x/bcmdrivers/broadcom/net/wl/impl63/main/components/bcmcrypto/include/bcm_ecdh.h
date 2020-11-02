/*
 * bcm_ecdh.h
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
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: $
 */

/* This header defines the API support elliptic curve diffie-hellman FIPS SP800-56Ar2 */

#ifndef _BCM_ECDH_H_
#define _BCM_ECDH_H_

#include <typedefs.h>
#ifdef BCMDRIVER
#include <osl.h>
#else
#include <stddef.h>
#endif // endif

#include <bcm_ec.h>

/* allocation support */
typedef bn_rand_fn_t ecdh_rand_fn_t;

/* generate ecdh ephemeral keys */
int ecdh_gen_keys(ecg_t *ecg,  ecg_elt_t *pub, bn_t **priv,
	ecdh_rand_fn_t rand_fn, void* rand_ctx, bn_ctx_t *bnx);

/* generate a shared secret using local private key and peer public key */
int ecdh_gen_secret(ecg_t *ecg, const bn_t *priv,
	const ecg_elt_t *r_pub, bn_t *secret);

#endif /* _BCM_ECDH_H_ */
