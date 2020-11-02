/*
 * bcm_ecdh.c
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <bcm_ecdh.h>

static
void key_25519(bn_t *key)
{
	uint8 v[32];
	bn_get(key, BN_FMT_BE, v, 32);
	v[0] |= 0x40;
	v[31] &= 0xf8;
	bn_set(key, BN_FMT_BE, v, 32);
}

/* generate ecdh ephemeral keys */
int ecdh_gen_keys(ecg_t *ecg,  ecg_elt_t *pub, bn_t **priv,
	ecdh_rand_fn_t rand_fn, void* rand_ctx, bn_ctx_t* bnx)
{
	const ecg_elt_t *base_elt = ecg_get_base_elt(ecg);
	int bit_len = ecg_get_bn_param(ecg, ECG_PARAM_BN_BIT_LEN, 0);
	if (*priv == NULL)
		*priv = rand_fn(rand_ctx, bnx, bit_len);

	if (ecg_get_type(ecg) == ECG_25519)
		key_25519(*priv);

	ecg_elt_mul(base_elt, *priv, pub);
	return 1;
}

/* generate a shared secret using local private key and peer public key */
int ecdh_gen_secret(ecg_t *ecg, const bn_t *priv,
	const ecg_elt_t *r_pub, bn_t *secret)
{
	const bn_t *x, *y;
	ecg_elt_t *secret_elt = ecg_elt_alloc(ecg);
	ecg_elt_mul(r_pub, priv, secret_elt);
	ecg_elt_get_xy(secret_elt, &x, &y);
	bn_copy(secret, x);
	ecg_elt_free(&secret_elt);
	return 1;
}
