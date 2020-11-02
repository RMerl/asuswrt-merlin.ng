/*
 * sha2.c
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
 *  <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: $
 */

#include <typedefs.h>
#ifdef BCMDRIVER
#include <osl.h>
#else
#include <string.h>
#endif	/* BCMDRIVER */

#include <bcmutils.h>
#include <bcmendian.h>
#include <sha2.h>

/* enable to get md5 support */
#define ENABLE_MD5

/* Enable SHA384 and SHA512 */
#define ENABLE_SHA2X

#ifdef ENABLE_MD5
#include <md5.h>
#endif // endif

#ifdef ENABLE_SHA2X
#include <sha2x.h>
#endif /* ENABLE_SHA2X */

/* md5 support */
#ifdef ENABLE_MD5
#include <md5.h>
static int sha2_md5_init(void *ctx, sha2_hash_type_t hash_type);
static void sha2_md5_update(void *ctx, const uint8 *data, int data_len);
static void sha2_md5_final(void *ctx, uint8* digest, int digest_len);
#endif /* ENABLE_MD5 */

/* sha1 and sha256 support */
typedef struct sha2l_context sha2l_context_t;
static int sha2l_init(void* ctx, sha2_hash_type_t hash_type);
static void sha2l_update(void* ctx, const uint8* data, int data_len);
static void sha2l_final(void* ctx, uint8* digest, int out_digest_len);

/* hash impl callbacks */
typedef int (*sha2_init_fn_t)(void *ctx, sha2_hash_type_t hash_type);
typedef void (*sha2_update_fn_t)(void *ctx, const uint8 *data, int data_len);
typedef void (*sha2_final_fn_t)(void *ctx, uint8* digest, int digest_len);

/* algo entries - register a set of hash types */
typedef uint32 sha2_hash_type_mask_t;
#define ALGO_MASK(_a) ((sha2_hash_type_mask_t)1 << (_a))
#define ALGO_MASK2(_a, _b) (ALGO_MASK(_a) | ALGO_MASK(_b))
#define ALGO_MASK4(_a, _b, _c, _d) (ALGO_MASK2(_a, _b) | ALGO_MASK2(_c, _d))
struct sha2_hash_impl_entry {
	sha2_hash_type_mask_t types;
	uint32 context_size;
	sha2_init_fn_t init;
	sha2_update_fn_t update;
	sha2_final_fn_t final;
};
typedef struct sha2_hash_impl_entry sha2_hash_impl_entry_t;

/* sha2l implementation */
#define SHA2L_BLOCK_SZ 64   /* legacy sha1, sha256 */

struct sha2l_context {
    sha2_hash_type_t hash_type;
    uint32 state[8];
    uint8 block[SHA2L_BLOCK_SZ];
    uint32 len;
    uint32 ctr[2];
    void (*sha2_transform)(uint32 *state, const uint8 *block);
};

static const sha2_hash_impl_entry_t sha2_hash_impl_entries[] = {
	{ALGO_MASK2(HASH_SHA1, HASH_SHA256),
	sizeof(sha2l_context_t), sha2l_init, sha2l_update, sha2l_final},
	{ALGO_MASK4(HASH_SHA384, HASH_SHA512, HASH_SHA512_224, HASH_SHA512_256),
	sizeof(sha2x_context_t), sha2x_init, sha2x_update, sha2x_final},
	{ALGO_MASK(HASH_MD5), sizeof(MD5_CTX), sha2_md5_init, sha2_md5_update, sha2_md5_final}
};
static const int n_sha2_hash_impl_entries = ARRAYSIZE(sha2_hash_impl_entries);

/***************   S H A - 1   **************************** */

static const uint32 sha1_Ka = 0x5A827999;
static const uint32 sha1_Kb = 0x6ED9EBA1;
static const uint32 sha1_Kc = 0x8F1BBCDC;
static const uint32 sha1_Kd = 0xCA62C1D6;

static const uint32 sha1_D[5] = {
	0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0
};

#define Ch(x, y, z)	((x & y) ^ (~x & z))
#define Xor(x, y, z) (x ^ y ^ z)
#define Maj(x, y, z)	((x & y) ^ (x & z) ^ (y & z))
#define ROL5(x)	((x << 5) | (x >> 27))
#define ROL(x)	((x << 1) | (x >> 31))
#define ROR2(x) ((x >> 2) | (x << 30))

static
void sha1_transform(uint32* state, const uint8* M)
{
	uint32  hh_buf[22];
	uint32* hh = &hh_buf[16];
	int j = 0;

	memcpy(hh, state, 20);
	for (; j < 16; ++j) {
		hh--;
		hh[0] = ROL5(hh[1]) + Ch(hh[2], hh[3], hh[4]) + hh[5] + sha1_Ka;
		hh[0] += hh[5] = M[0]<<24 | M[1]<<16 | M[2]<<8 | M[3];
		M += 4;
		hh[2] = ROR2(hh[2]);
	}

	/* now hh is &hh_buf[0] */

	for (; j < 20; ++j) {
		memmove(&hh[1], &hh[0], sizeof(hh_buf) - sizeof(hh_buf[0]));
		hh[0] = ROL5(hh[1]) + Ch(hh[2], hh[3], hh[4]) + hh[5] + sha1_Ka;
		hh[0] += hh[5] = ROL((hh[8] ^ hh[13] ^ hh[19] ^ hh[21]));
		hh[2] = ROR2(hh[2]);
	}

	for (; j < 40; ++j) {
		memmove(&hh[1], &hh[0], sizeof(hh_buf) - sizeof(hh_buf[0]));
		hh[0] = ROL5(hh[1]) + Xor(hh[2], hh[3], hh[4]) + hh[5] + sha1_Kb;
		hh[0] += hh[5] = ROL((hh[8] ^ hh[13] ^ hh[19] ^ hh[21]));
		hh[2] = ROR2(hh[2]);
	}

	for (; j < 60; ++j) {
		memmove(&hh[1], &hh[0], sizeof(hh_buf) - sizeof(hh_buf[0]));
		hh[0] = ROL5(hh[1]) + Maj(hh[2], hh[3], hh[4]) + hh[5] + sha1_Kc;
		hh[0] += hh[5] = ROL((hh[8] ^ hh[13] ^ hh[19] ^ hh[21]));
		hh[2] = ROR2(hh[2]);
	}

	for (; j < 80; ++j) {
		memmove(&hh[1], &hh[0], sizeof(hh_buf) - sizeof(hh_buf[0]));
		hh[0] = ROL5(hh[1]) + Xor(hh[2], hh[3], hh[4]) + hh[5] + sha1_Kd;
		hh[0] += hh[5] = ROL((hh[8] ^ hh[13] ^ hh[19] ^ hh[21]));
		hh[2] = ROR2(hh[2]);
	}

	state[0] += hh[0];
	state[1] += hh[1];
	state[2] += hh[2];
	state[3] += hh[3];
	state[4] += hh[4];
}

/***************   S H A - 2 5 6   ************************ */

static const uint32 sha256_K[64] = {
	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
	0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
	0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
	0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
	0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
	0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
	0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
	0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
	0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
	0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
	0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
	0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
	0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
	0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
	0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
	0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

static const uint32 sha256_D[8] = {
	0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
	0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
};

#define Rot(x, n)	((x >> n) | (x << (32-n)))
#define Sum0(x)		(Rot(x, 2) ^ Rot(x, 13) ^ Rot(x, 22))
#define Sum1(x)		(Rot(x, 6) ^ Rot(x, 11) ^ Rot(x, 25))
#define Sigma0(x)	(Rot(x, 7) ^ Rot(x, 18) ^ (x >> 3))
#define Sigma1(x)	(Rot(x, 17)^ Rot(x, 19) ^ (x >> 10))

static
void sha256_transform(uint32* state, const uint8* M)
{
	uint32 T1;
	uint32 hh_buf[25];
	uint32* hh = &hh_buf[16];
	int j = 0;

	memcpy(&hh[0], state, 32);
	for (; j < 16; ++j) {
		hh--;
		T1 = hh[8] + Sum1(hh[5]) + Ch(hh[5], hh[6], hh[7]) + sha256_K[j];
		T1 += hh[8] = M[0]<<24 | M[1]<<16 | M[2]<<8 | M[3];
		M += 4;
		hh[4] += T1;
		hh[0] = T1 + Sum0(hh[1]) + Maj(hh[1], hh[2], hh[3]);
	}

	for (; j < 64; ++j) {
		memmove(&hh[1], &hh[0], sizeof(hh_buf) - sizeof(hh_buf[0]));
		T1 = hh[8] + Sum1(hh[5]) + Ch(hh[5], hh[6], hh[7]) + sha256_K[j];
		T1 += hh[8] = Sigma1(hh[10]) + hh[15] + Sigma0(hh[23]) + hh[24];
		hh[4] += T1;
		hh[0] = T1 + Sum0(hh[1]) + Maj(hh[1], hh[2], hh[3]);
	}

	state[0] += hh[0];
	state[1] += hh[1];
	state[2] += hh[2];
	state[3] += hh[3];
	state[4] += hh[4];
	state[5] += hh[5];
	state[6] += hh[6];
	state[7] += hh[7];
}

static void
sha2l_u32tou8(uint8* out, const uint32* in, int len)
{
	int nw = len >> 2; /* number of 4-octet words */
	int i;

	for (i = 0; i < nw; ++i) {
		hton32_ua_store(*in, out);
		in++; out += sizeof(uint32);
	}

	len = len & 0x03;
	if (len) {
		uint32 k = *in;
		for (i = 0; i < len; ++i) {
			out[i] = (uint8)(k >> (8 * (3 - i)));
		}
	}
}

static void
sha2l_update(void *in_ctx, const uint8 *data, int data_len)
{
	sha2l_context_t *ctx = (sha2l_context_t *)in_ctx;
	ctx->ctr[1] += data_len;
	if (ctx->ctr[1] < (uint32)data_len)
		ctx->ctr[0]++;

	if (ctx->len + data_len < SHA2L_BLOCK_SZ) {
		memcpy(ctx->block + ctx->len, data, data_len);
		ctx->len += data_len;
	} else {
		if (ctx->len > 0) {
			memcpy(ctx->block + ctx->len, data, SHA2L_BLOCK_SZ - ctx->len);
			ctx->sha2_transform(ctx->state, ctx->block);
			data += SHA2L_BLOCK_SZ - ctx->len;
			data_len -= SHA2L_BLOCK_SZ - ctx->len;
		}
		while (data_len >= SHA2L_BLOCK_SZ) {
			ctx->sha2_transform(ctx->state, data);
			data += SHA2L_BLOCK_SZ;
			data_len -= SHA2L_BLOCK_SZ;
		}
		memcpy(ctx->block, data, data_len);
		ctx->len = data_len;
	}
}

static void
sha2l_final(void *in_ctx, uint8 *digest, int out_digest_len)
{
	sha2l_context_t *ctx = (sha2l_context_t *)in_ctx;
	int digest_len;

	digest_len = sha2_digest_len(ctx->hash_type);
	if (!digest_len)
		goto done;

	digest_len = MIN(out_digest_len, digest_len);
	ctx->block[ctx->len++] = 0x80;
	if (ctx->len > 56) {
		memset(ctx->block + ctx->len, 0, SHA2L_BLOCK_SZ - ctx->len);
		ctx->sha2_transform(ctx->state, ctx->block);
		ctx->len = 0;
	}
	memset(ctx->block + ctx->len, 0, 56 - ctx->len);
	ctx->ctr[0] = ctx->ctr[0]<<3 | ctx->ctr[1]>>29;
	ctx->ctr[1] <<= 3;
	sha2l_u32tou8(ctx->block+56, ctx->ctr, 2 * sizeof(uint32));
	ctx->sha2_transform(ctx->state, ctx->block);

	sha2l_u32tou8(digest, ctx->state, digest_len);

done:;
}

static int
sha2l_init(void* in_ctx, sha2_hash_type_t hash_type)
{
	sha2l_context_t *ctx = (sha2l_context_t *)in_ctx;
	ctx->hash_type = hash_type;
	if (ctx->hash_type == HASH_SHA1) {
		ctx->sha2_transform = sha1_transform;
		memcpy(ctx->state, sha1_D, 20);
	} else  {
		ctx->sha2_transform = sha256_transform;
		memcpy(ctx->state, sha256_D, 32);
	}

	ctx->len = 0;
	ctx->ctr[0] = 0;
	ctx->ctr[1] = 0;
	return BCME_OK;
}

/* external api - see sha2.h */

int
sha2_init(sha2_ctx_t *ctx, sha2_hash_type_t hash_type)
{
	int i;
	int err = BCME_UNSUPPORTED;

	if (!ctx) {
		err = BCME_BADARG;
		goto done;
	}

	ctx->hash_type = HASH_NONE;
	ctx->impl = NULL;
	for (i = 0; i < n_sha2_hash_impl_entries; ++i) {
		const sha2_hash_impl_entry_t *ent = &sha2_hash_impl_entries[i];
		if (!(ALGO_MASK(hash_type) & ent->types))
			continue;
		ctx->hash_type = hash_type;
		ctx->impl = ent;
		ctx->impl_ctx = (void*)ctx->buf;
		ctx->impl->init(ctx->impl_ctx, hash_type);
		err = BCME_OK;
		break;
	}

done:
	return err;
}

void
sha2_update(sha2_ctx_t *ctx, const uint8 *data, int data_len)
{
	if (ctx->impl)
		ctx->impl->update(ctx->impl_ctx, data, data_len);
}

void
sha2_final(sha2_ctx_t *ctx, uint8 *digest, int digest_len)
{
	if (ctx->impl)
		ctx->impl->final(ctx->impl_ctx, digest, digest_len);
}

static void
sha2_update_with_prefixes(sha2_ctx_t *ctx, const bcm_const_xlvp_t *prefixes, int num_prefixes,
	const uint8 *data, int data_len)
{
	int j;

	/* handle prefixes, if any */
	for (j = 0; prefixes != NULL && j < num_prefixes; ++ j) {
		if (prefixes[j].len)
			sha2_update(ctx, prefixes[j].data, prefixes[j].len);
	}

	/* handle data */
	if (data && data_len)
		sha2_update(ctx, data, data_len);
}

int
sha2(sha2_hash_type_t hash_type, const bcm_const_xlvp_t *prefixes, int num_prefixes,
	const uint8 *data, int data_len, uint8 *digest, int digest_len)
{
	int err;

	sha2_ctx_t ctx;
	err = sha2_init(&ctx, hash_type);
	if (err != BCME_OK)
		goto done;

	sha2_update_with_prefixes(&ctx, prefixes, num_prefixes, data, data_len);
	sha2_final(&ctx, digest, digest_len);
done:
	return err;
}

int
sha2_digest_len(sha2_hash_type_t hash_type)
{
	int l;
	switch (hash_type) {
	case HASH_SHA1: l = 20; break;
	case HASH_SHA256: l = 32; break;
	case HASH_SHA384: l = 48; break;
	case HASH_SHA512: l = 64; break;
	case HASH_SHA512_224: l = 28; break;
	case HASH_SHA512_256: l = 32; break;
	case HASH_MD5: l = 16; break;
	default: l = 0; break;
	}

	return l;
}

int
sha2_block_size(sha2_hash_type_t hash_type)
{
	int s;
	switch (hash_type) {
	case HASH_SHA1: /* fall through */
	case HASH_SHA256:
	case HASH_MD5:
		s = 64;
		break;

	case HASH_SHA384: /* fall through */
	case HASH_SHA512:
	case HASH_SHA512_224:
	case HASH_SHA512_256:
		s = 128;
		break;

	default:
		s = 0;
		break;
	}
	return s;
}

/* hmac support */

static void
hmac_sha2_update_with_prefixes(hmac_sha2_ctx_t *ctx,
	const bcm_const_xlvp_t *prefixes, int num_prefixes, const uint8 *data, int data_len)
{
	int j;

	/* handle prefixes, if any */
	for (j = 0; prefixes != NULL && j < num_prefixes; ++ j) {
		if (prefixes[j].len)
			hmac_sha2_update(ctx, prefixes[j].data, prefixes[j].len);
	}

	/* handle data */
	if (data && data_len)
		hmac_sha2_update(ctx, data, data_len);
}

int
hmac_sha2(sha2_hash_type_t hash_type, const uint8 *key, int key_len,
	const bcm_const_xlvp_t *prefixes, int num_prefixes,
	const uint8 *data, int data_len, uint8 *digest, int digest_len)
{
	hmac_sha2_ctx_t ctx;
	int err;

	err = hmac_sha2_init(&ctx, hash_type, key, key_len);
	if (err != BCME_OK)
		goto done;

	hmac_sha2_update_with_prefixes(&ctx, prefixes, num_prefixes, data, data_len);
	hmac_sha2_final(&ctx, digest, digest_len);
done:
	return err;
}

int
hmac_sha2_init(hmac_sha2_ctx_t *ctx, sha2_hash_type_t hash_type,
	const uint8 *key, int key_len)
{
	unsigned char digest[SHA2_MAX_DIGEST_LEN];
	uint8 k_ipad[SHA2_MAX_BLOCK_SZ];
	int digest_len;
	sha2_ctx_t *sha2_ctx;
	uint8 *k_opad;
	int i;
	int block_sz;
	int err = BCME_OK;

	if (!ctx) {
		err = BCME_BADARG;
		goto done;
	}

	digest_len = sha2_digest_len(hash_type);
	block_sz = sha2_block_size(hash_type);
	k_opad = ctx->opad;
	sha2_ctx = &ctx->sha2_ctx;

	/* reduce key to block */
	if (block_sz < key_len) {
		sha2_init(sha2_ctx, hash_type);
		sha2_update(sha2_ctx, key, key_len);
		sha2_final(sha2_ctx, digest, digest_len);
		key = digest;
		key_len = block_sz; /* assume digest len is greater than block sz */
	}

	/* init ipad and opad */
	memset(k_ipad, 0, block_sz);
	memset(k_opad, 0, block_sz);
	memcpy(k_ipad, key, key_len);
	memcpy(k_opad, key, key_len);

	for (i = 0; i < block_sz; i++) {
		k_ipad[i] ^= 0x36;
		k_opad[i] ^= 0x5c;
	}

	/* inner hash */
	err = sha2_init(sha2_ctx, hash_type);
	if (err != BCME_OK)
		goto done;

	sha2_update(sha2_ctx, k_ipad, block_sz);
	/* to be finalized in hmac_sha2_final() */

done:
	return err;
}

void
hmac_sha2_update(hmac_sha2_ctx_t *ctx, const uint8 *data, int data_len)
{
	if (ctx)
		sha2_update(&ctx->sha2_ctx, data, data_len);
}

void
hmac_sha2_final(hmac_sha2_ctx_t *ctx, uint8* out, int out_len)
{
	unsigned char digest[SHA2_MAX_DIGEST_LEN];
	int digest_len;
	sha2_ctx_t *sha2_ctx;
	uint8 *k_opad;
	sha2_hash_type_t hash_type;
	int block_sz;

	if (!ctx)
		return;

	k_opad = ctx->opad;
	sha2_ctx = &ctx->sha2_ctx;
	hash_type = sha2_ctx->hash_type;
	block_sz = sha2_block_size(hash_type);
	digest_len = sha2_digest_len(hash_type);

	/* finalize inner hash */
	sha2_final(&ctx->sha2_ctx, digest, digest_len);

	/* outer hash */
	sha2_init(sha2_ctx, hash_type);
	sha2_update(sha2_ctx, k_opad, block_sz);
	sha2_update(sha2_ctx, digest, digest_len);
	sha2_final(sha2_ctx, digest, digest_len);

	digest_len = MIN(out_len, digest_len);
	memcpy(out, digest, digest_len);
}

int
hmac_sha2_n(sha2_hash_type_t hash_type, const uint8 *key, int key_len,
	const bcm_const_xlvp_t *prefixes, int num_prefixes,
	const uint8 *data, int data_len, uint8 *out, int out_len)
{
	int digest_len;
	hmac_sha2_ctx_t ctx;
	int i;
	int err = BCME_OK;

	digest_len = sha2_digest_len(hash_type);
	if (digest_len <= 0)
		goto done;

	for (i = 0; i < CEIL(out_len, digest_len); ++i) {
		uint8 d[2];
		int copy_len;

		err = hmac_sha2_init(&ctx, hash_type, key, key_len);
		if (err != BCME_OK)
			goto done;

		if (hash_type != HASH_SHA1) {
			/* KDF: handle leading iterator */
			htol16_ua_store(i + 1, d);
			hmac_sha2_update(&ctx, d, sizeof(d));
		}

		hmac_sha2_update_with_prefixes(&ctx, prefixes, num_prefixes, data, data_len);

		if (hash_type != HASH_SHA1) {
			/* KDF: handle output bit len */
			htol16_ua_store(out_len << 3, d);
			hmac_sha2_update(&ctx, d, sizeof(d));
		} else {
			/* PRF: handle trailing iterator */
			d[0] = i & 0xFF;
			hmac_sha2_update(&ctx, d, 1);
		}

		/* truncate if necessary */
		copy_len = digest_len;
		if (((i + 1) * digest_len) > out_len)
			copy_len = out_len % digest_len;
		hmac_sha2_final(&ctx, &out[i * digest_len], copy_len);
	}

done:
	return err;
}

#ifdef ENABLE_MD5
static int
sha2_md5_init(void *in_ctx, sha2_hash_type_t hash_type)
{
	MD5_CTX *ctx = (MD5_CTX *)in_ctx;
	BCM_REFERENCE(hash_type);
	MD5Init(ctx);
	return BCME_OK;
}

static void
sha2_md5_update(void *in_ctx, const uint8 *data, int data_len)
{
	MD5_CTX *ctx = (MD5_CTX *)in_ctx;
	MD5Update(ctx, data, data_len);
}

static void sha2_md5_final(void *in_ctx, uint8* digest, int out_digest_len)
{
	uint8 tmp_digest[sizeof(((MD5_CTX *)0)->digest)];
	MD5_CTX *ctx = (MD5_CTX *)in_ctx;
	int digest_len;

	MD5Final(tmp_digest, ctx);
	digest_len = MIN((int)sizeof(tmp_digest), out_digest_len);
	memcpy(digest, tmp_digest, digest_len);
}
#endif /* ENABLE_MD5 */

/* compute kdf specified in rfc 5295. Also HKDF-Expand in rfc 5869 */
#define NUM_KDF5295_PREFIXES 4
int
kdf5295(sha2_hash_type_t hash_type,
	const uint8 *key, int key_len, const char *label,
	const uint8 *info, int info_len, uint8 *out, int out_len)
{
	int err = BCME_OK;
	int i, niter;
	int npfxs = 0;
	uint8 iter_pfx;
	int out_off = 0;
	bcm_const_xlvp_t pfxs[NUM_KDF5295_PREFIXES];
	int digest_len;
	int label_len;

	if (!out || !out_len) {
		goto done;
	}

	label_len = label ? strlen(label) + 1 : 0;
	digest_len = sha2_digest_len(hash_type);
	niter = digest_len ? CEIL(out_len, digest_len) : 256;
	if (niter > 255 || label_len > 65535 ||
			info_len > 65535) {
		err = BCME_UNSUPPORTED;
		goto done;
	}

	pfxs[npfxs].data =  out;
	pfxs[npfxs++].len = (uint16)digest_len;

	pfxs[npfxs].data =  (const uint8 *)(label ? label : "");
	pfxs[npfxs++].len = (uint16)label_len;

	pfxs[npfxs].data =  info;
	pfxs[npfxs++].len = (uint16)info_len;

	iter_pfx = 1;
	pfxs[npfxs].data =  &iter_pfx;
	pfxs[npfxs++].len = sizeof(iter_pfx);

	/* T[1] = PRF (K, S | 0x01) */
	err = hmac_sha2(hash_type, key, key_len, pfxs + 1, npfxs - 1, NULL, 0,
		out, MIN(out_len, digest_len));
	if (err != BCME_OK) {
		goto done;
	}

	for (i = 1; i <= niter; ++i) {
		/* T[i]  is in place, prep for next iter  */
		pfxs[0].data = &out[out_off];
		out_off += digest_len;
		out_len -= digest_len;
		if (out_len <= 0) {
			break;
		}

		iter_pfx = (i + 1) & 0xff;

		/* j = iter_pfx, T[j] = PRF (K, T[j-1] | S | j) */
		err = hmac_sha2(hash_type, key, key_len, pfxs, npfxs, NULL, 0,
			&out[out_off], MIN(out_len, digest_len));
		if (err != BCME_OK) {
			break;
		}
	}

done:
	return err;
}
