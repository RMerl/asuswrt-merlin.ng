/*
 * bcm_ecdsa.c
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
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <bcm_ecdsa.h>

#include <sha2.h>

typedef struct {
	ecg_t *ecg;
	ecdsa_hash_type_t hash_type;
	bn_t *priv;
	bn_ctx_t *bnx;
	ecdsa_rand_fn_t rand_fn;
	void* rand_ctx;
} ecdsa_s;

static
bn_t *hash(bn_ctx_t *bnx, ecdsa_hash_type_t hash_type, const uint8 *msg, int msg_len)
{
	int digest_len;
	uint8 digest[64];
	bn_t *bn_hash;
	digest_len = sha2_digest_len(hash_type);
	sha2(hash_type, NULL, 0, (uint8 *)msg, msg_len, digest, digest_len);
	bn_hash = bn_alloc(bnx, BN_FMT_BE, digest, digest_len);
	return bn_hash;
}

/* allocate an ecdsa context */
ecdsa_t* ecdsa_alloc(ecg_t *ecg, ecdsa_hash_type_t hash_type,
	ecdsa_rand_fn_t rand_fn, void *rand_ctx, bn_t *priv, /* optional */
	bn_ctx_t* bnx)
{
	ecdsa_s e;

	e.ecg = ecg;
	e.hash_type = hash_type;
	e.rand_fn = rand_fn;
	e.rand_ctx = rand_ctx;
	e.priv = priv;
	e.bnx = bnx;

	return (ecdsa_t *)bn_alloc(bnx, BN_FMT_BE, (uint8 *)&e, sizeof(e));
}

/* free an ecdsa context and clear arg and any keys */
void ecdsa_free(ecdsa_t **ecdsa)
{
	bn_free((bn_t **)ecdsa);
}

static
void key_25519(bn_t *key)
{
	uint8 v[32];
	bn_get(key, BN_FMT_BE, v, 32);
	v[0] |= 0x40;
	v[31] &= 0xf8;
	bn_set(key, BN_FMT_BE, v, 32);
}

/* generate key pair - see FIPS 186-4 Appendix B.4 */
int ecdsa_gen_keys(ecdsa_t *ecdsa, bn_t **priv, ecg_elt_t *pub)
{
	ecdsa_s e;
	int bit_len;
	bn_get((bn_t *)ecdsa, BN_FMT_BE, (uint8 *)&e, sizeof(e));
	bit_len = ecg_get_bn_param(e.ecg, ECG_PARAM_BN_BIT_LEN, 0);
	if (*priv == NULL)
		*priv = e.rand_fn(e.rand_ctx, e.bnx, bit_len);

	if (ecg_get_type(e.ecg) == ECG_25519)
		key_25519(*priv);

	ecg_elt_mul(ecg_get_base_elt(e.ecg), *priv, pub);
	return 1;
}

/* sign a message of given length in bytes */
int ecdsa_sign(ecdsa_t *ecdsa, const uint8 *msg, int msg_len, ecdsa_signature_t *sig)
{
	ecdsa_s e;
	int bit_len, byte_len, genPriv;
	const ecg_elt_t *B;
	const bn_t *kx, *ky;
	ecg_elt_t *R;
	bn_t *Order, *h, *k = NULL;

	bn_get((bn_t *)ecdsa, BN_FMT_BE, (uint8 *)&e, sizeof(e));

	bit_len = ecg_get_bn_param(e.ecg, ECG_PARAM_BN_BIT_LEN, 0);
	byte_len = ecg_get_bn_param(e.ecg, ECG_PARAM_BN_BYTE_LEN, 0);
	B = ecg_get_base_elt(e.ecg);
	R = ecg_elt_alloc(e.ecg);
	Order = bn_alloc(e.bnx, BN_FMT_LE, 0, byte_len);
	h = hash(e.bnx, e.hash_type, msg, msg_len);	/*  h = hash(msg) */
	if (ecg_get_type(e.ecg) == ECG_25519)
		bn_truncate(h, 4);

	genPriv = (e.priv == NULL);
	if (genPriv)
		ecdsa_gen_keys(ecdsa, &e.priv, sig->pub);

	ecdsa_gen_keys(ecdsa, &k, R);
	ecg_elt_get_xy(R, &kx, &ky);

	bn_copy(sig->r, kx);  /*  r = R.x */

	ecg_get_bn_param(e.ecg, ECG_PARAM_BN_ORDER, Order);

	bn_inv(k, Order, k);  /*  k^-1	 */

	bn_mul(sig->s, e.priv, sig->r, Order);	/*  s = dr */

	bn_add(sig->s, sig->s, h, Order);  /*  s = h + dr  */

	bn_mul(sig->s, sig->s, k, Order);  /*  s = (h + dr) / k  */

	bn_free(&k);
	if (genPriv)
		bn_free(&e.priv);

	bn_free(&h);
	bn_free(&Order);
	ecg_elt_free(&R);
	return 1;
}

/* verify a message of given length in bytes */
int ecdsa_verify(ecdsa_t *ecdsa, const uint8* msg, int msg_len, const ecdsa_signature_t *sig)
{
	ecdsa_s e;
	int bit_len, byte_len, Ok;
	const ecg_elt_t *B;
	const bn_t *rx, *ry;
	ecg_elt_t *R1, *R2, *R3;
	bn_t *Order, *h, *h1, *h2, *sInv;

	bn_get((bn_t *)ecdsa, BN_FMT_BE, (uint8 *)&e, sizeof(e));

	bit_len = ecg_get_bn_param(e.ecg, ECG_PARAM_BN_BIT_LEN, 0);
	byte_len = ecg_get_bn_param(e.ecg, ECG_PARAM_BN_BYTE_LEN, 0);
	B = ecg_get_base_elt(e.ecg);
	Order = bn_alloc(e.bnx, BN_FMT_LE, 0, byte_len);
	h1 = bn_alloc(e.bnx, BN_FMT_LE, 0, byte_len);
	h2 = bn_alloc(e.bnx, BN_FMT_LE, 0, byte_len);
	sInv = bn_alloc(e.bnx, BN_FMT_LE, 0, byte_len);
	R1 = ecg_elt_alloc(e.ecg);
	R2 = ecg_elt_alloc(e.ecg);
	R3 = ecg_elt_alloc(e.ecg);

	h = hash(e.bnx, e.hash_type, msg, msg_len);	/*  h = hash(msg) */
	if (ecg_get_type(e.ecg) == ECG_25519)
		bn_truncate(h, 4);

	ecg_get_bn_param(e.ecg, ECG_PARAM_BN_ORDER, Order);
	bn_inv(sig->s, Order, sInv);  /*  sInv = s^-1  */

	bn_mul(h1, h, sInv, Order);   /*  h1 = h * sInv	*/

	ecg_elt_mul(B, h1, R1);    /*  R1 = h1 * Base */

	bn_mul(h2, sig->r, sInv, Order); /*  h2 = r * sInv */

	ecg_elt_mul(sig->pub, h2, R2);  /*  R2 = h2 * Pub */

	ecg_elt_add(R2, R1, R3);      /*  R3 = R1 + R2 */
	ecg_elt_get_xy(R3, &rx, &ry);

	Ok = !bn_cmp(sig->r, rx);
	if (!Ok && ecg_get_type(e.ecg) == ECG_25519) {
		ecg_elt_inv(R1, R1);
		ecg_elt_add(R2, R1, R3);      /*  R3 = R1 + R2 */
		ecg_elt_get_xy(R3, &rx, &ry);
		Ok = !bn_cmp(sig->r, rx);
	}

	bn_free(&h);
	ecg_elt_free(&R3);
	ecg_elt_free(&R2);
	ecg_elt_free(&R1);
	bn_free(&sInv);
	bn_free(&h2);
	bn_free(&h1);
	bn_free(&Order);

	return Ok;
}
