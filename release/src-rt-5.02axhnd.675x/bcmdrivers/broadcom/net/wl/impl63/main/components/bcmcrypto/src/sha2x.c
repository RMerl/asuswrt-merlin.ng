/*
 * sha2x.c
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

/*
 * This header defines the API for 64-bit sha   FIPS 180-4
 */

#include <typedefs.h>
#ifdef BCMDRIVER
#include <osl.h>
#else
#include <string.h>
#endif	/* BCMDRIVER */

#include <bcmutils.h>
#include <bcmendian.h>
#include <sha2x.h>

static const uint64 kk[80] = {
	0x428a2f98d728ae22ULL, 0x7137449123ef65cdULL, 0xb5c0fbcfec4d3b2fULL, 0xe9b5dba58189dbbcULL,
	0x3956c25bf348b538ULL, 0x59f111f1b605d019ULL, 0x923f82a4af194f9bULL, 0xab1c5ed5da6d8118ULL,
	0xd807aa98a3030242ULL, 0x12835b0145706fbeULL, 0x243185be4ee4b28cULL, 0x550c7dc3d5ffb4e2ULL,
	0x72be5d74f27b896fULL, 0x80deb1fe3b1696b1ULL, 0x9bdc06a725c71235ULL, 0xc19bf174cf692694ULL,
	0xe49b69c19ef14ad2ULL, 0xefbe4786384f25e3ULL, 0x0fc19dc68b8cd5b5ULL, 0x240ca1cc77ac9c65ULL,
	0x2de92c6f592b0275ULL, 0x4a7484aa6ea6e483ULL, 0x5cb0a9dcbd41fbd4ULL, 0x76f988da831153b5ULL,
	0x983e5152ee66dfabULL, 0xa831c66d2db43210ULL, 0xb00327c898fb213fULL, 0xbf597fc7beef0ee4ULL,
	0xc6e00bf33da88fc2ULL, 0xd5a79147930aa725ULL, 0x06ca6351e003826fULL, 0x142929670a0e6e70ULL,
	0x27b70a8546d22ffcULL, 0x2e1b21385c26c926ULL, 0x4d2c6dfc5ac42aedULL, 0x53380d139d95b3dfULL,
	0x650a73548baf63deULL, 0x766a0abb3c77b2a8ULL, 0x81c2c92e47edaee6ULL, 0x92722c851482353bULL,
	0xa2bfe8a14cf10364ULL, 0xa81a664bbc423001ULL, 0xc24b8b70d0f89791ULL, 0xc76c51a30654be30ULL,
	0xd192e819d6ef5218ULL, 0xd69906245565a910ULL, 0xf40e35855771202aULL, 0x106aa07032bbd1b8ULL,
	0x19a4c116b8d2d0c8ULL, 0x1e376c085141ab53ULL, 0x2748774cdf8eeb99ULL, 0x34b0bcb5e19b48a8ULL,
	0x391c0cb3c5c95a63ULL, 0x4ed8aa4ae3418acbULL, 0x5b9cca4f7763e373ULL, 0x682e6ff3d6b2b8a3ULL,
	0x748f82ee5defb2fcULL, 0x78a5636f43172f60ULL, 0x84c87814a1f0ab72ULL, 0x8cc702081a6439ecULL,
	0x90befffa23631e28ULL, 0xa4506cebde82bde9ULL, 0xbef9a3f7b2c67915ULL, 0xc67178f2e372532bULL,
	0xca273eceea26619cULL, 0xd186b8c721c0c207ULL, 0xeada7dd6cde0eb1eULL, 0xf57d4f7fee6ed178ULL,
	0x06f067aa72176fbaULL, 0x0a637dc5a2c898a6ULL, 0x113f9804bef90daeULL, 0x1b710b35131c471bULL,
	0x28db77f523047d84ULL, 0x32caab7b40c72493ULL, 0x3c9ebe0a15c9bebcULL, 0x431d67c49c100d4cULL,
	0x4cc5d4becb3e42b6ULL, 0x597f299cfc657e2aULL, 0x5fcb6fab3ad6faecULL, 0x6c44198c4a475817ULL
};

static const uint64 sha512_224_init[8] = {
	0x8C3D37C819544DA2ULL, 0x73E1996689DCD4D6ULL, 0x1DFAB7AE32FF9C82ULL, 0x679DD514582F9FCFULL,
	0x0F6D2B697BD44DA8ULL, 0x77E36F7304C48942ULL, 0x3F9D85A86A1D36C8ULL, 0x1112E6AD91D692A1ULL
};

static const uint64 sha512_256_init[8] = {
	0x22312194FC2BF72CULL, 0x9F555FA3C84C64C2ULL, 0x2393B86B6F53B151ULL, 0x963877195940EABDULL,
	0x96283EE2A88EFFE3ULL, 0xBE5E1E2553863992ULL, 0x2B0199FC2C85B8AAULL, 0x0EB72DDC81C52CA2ULL
};

static const uint64 sha384_init[8] = {
	0xcbbb9d5dc1059ed8ULL, 0x629a292a367cd507ULL, 0x9159015a3070dd17ULL, 0x152fecd8f70e5939ULL,
	0x67332667ffc00b31ULL, 0x8eb44a8768581511ULL, 0xdb0c2e0d64f98fa7ULL, 0x47b5481dbefa4fa4ULL
};

static const uint64 sha512_init[8] = {
	0x6A09E667F3BCC908ULL, 0xBB67AE8584CAA73BULL, 0x3C6EF372FE94F82BULL, 0xA54FF53A5F1D36F1ULL,
	0x510E527FADE682D1ULL, 0x9B05688C2B3E6C1FULL, 0x1F83D9ABFB41BD6BULL, 0x5BE0CD19137E2179ULL
};

#define Ch(x, y, z)	((x & y) ^ (~x & z))
#define Maj(x, y, z)	((x & y) ^ (x & z) ^ (y & z))
#define Rot(x, n)	((x >> n) | (x << (64-n)))
#define Sum0(x)		(Rot(x, 28) ^ Rot(x, 34) ^ Rot(x, 39))
#define Sum1(x)		(Rot(x, 14) ^ Rot(x, 18) ^ Rot(x, 41))
#define Sigma0(x)	(Rot(x, 1) ^ Rot(x, 8) ^ (x >> 7))
#define Sigma1(x)	(Rot(x, 19)^ Rot(x, 61) ^ (x >> 6))

static
void sha2x_txfm_iter0to15(uint64 *hh, const uint8 *M, int j)
{
	uint64 T1;
	T1 = hh[8] + Sum1(hh[5]) + Ch(hh[5], hh[6], hh[7]) + kk[j];
	T1 += hh[8] = ntoh64_ua(M); /* M0..7 - BE format */
	M += 8;
	hh[4] += T1;
	hh[0] = T1 + Sum0(hh[1]) + Maj(hh[1], hh[2], hh[3]);
}

static
void sha2x_txfm_iter16to80(uint64 *hh, int j)
{
	uint64 T1;
	T1 = hh[8] + Sum1(hh[5]) + Ch(hh[5], hh[6], hh[7]) + kk[j];
	T1 += hh[8] = Sigma1(hh[10]) + hh[15] + Sigma0(hh[23]) + hh[24];
	hh[4] += T1;
	hh[0] = T1 + Sum0(hh[1]) + Maj(hh[1], hh[2], hh[3]);
}

static
void sha2x_transform(uint64 *state, const uint8 *M)
{
	uint64  hh_buf[25];
	uint64* hh = &hh_buf[16];
	int j = 0;

	memcpy(&hh[0], state, 64);
	for (; j < 16; ++j) {
		sha2x_txfm_iter0to15(--hh, M, j);
		M += 8;
	}

	for (; j < 80; ++j) {
		memmove(&hh[1], &hh[0], sizeof(hh_buf) - sizeof(hh_buf[0]));
		sha2x_txfm_iter16to80(hh, j);
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
sha2x_u64tou8(uint8* out, const uint64* in, int len)
{
	int nw = len >> 3; /* number of 8-octet words */
	int i;
	uint64 k;

	for (i = 0; i < nw; ++i) {
		k = *in++;
		*out++ = (uint8)((k >> 56) & 0xff);
		*out++ = (uint8)((k >> 48) & 0xff);
		*out++ = (uint8)((k >> 40) & 0xff);
		*out++ = (uint8)((k >> 32) & 0xff);
		*out++ = (uint8)((k >> 24) & 0xff);
		*out++ = (uint8)((k >> 16) & 0xff);
		*out++ = (uint8)((k >> 8) & 0xff);
		*out++ = (uint8)(k & 0xff);
	}

	/* copy partial word */
	len = len & 0x07;
	if (!len)
		goto done;

	/* len is from [1..7] */
	do {
		k = *in;
		*out++ = (uint8)((k >> 56) & 0xff); len--;
		if (!len) break;
		*out++ = (uint8)((k >> 48) & 0xff); len--;
		if (!len) break;
		*out++ = (uint8)((k >> 40) & 0xff); len--;
		if (!len) break;
		*out++ = (uint8)((k >> 32) & 0xff); len--;
		if (!len) break;
		*out++ = (uint8)((k >> 24) & 0xff); len--;
		if (!len) break;
		*out++ = (uint8)((k >> 16) & 0xff); len--;
		if (!len) break;
		*out++ = (uint8)((k >> 8) & 0xff);
		/* len is 1 - we are done */
	} while (0);
done:;
}

void sha2x_update(void *in_ctx, const uint8 *data, int data_len)
{
	sha2x_context_t *ctx = (sha2x_context_t *)in_ctx;
	ctx->ctr[1] += data_len;
	if (ctx->ctr[1] < (uint64)data_len)
		ctx->ctr[0]++;

	if (ctx->len + data_len < SHA2X_BLOCK_SZ) {
		memcpy(ctx->block + ctx->len, data, data_len);
		ctx->len += data_len;
	} else {
		if (ctx->len > 0) {
			memcpy(ctx->block + ctx->len, data, SHA2X_BLOCK_SZ - ctx->len);
			sha2x_transform(ctx->state, ctx->block);
			data += SHA2X_BLOCK_SZ - ctx->len;
			data_len -= SHA2X_BLOCK_SZ - ctx->len;
		}
		while (data_len >= SHA2X_BLOCK_SZ) {
			sha2x_transform(ctx->state, data);
			data += SHA2X_BLOCK_SZ;
			data_len -= SHA2X_BLOCK_SZ;
		}
		memcpy(ctx->block, data, data_len);
		ctx->len = data_len;
	}
}

void sha2x_final(void *in_ctx, uint8 *digest, int out_digest_len)
{
	sha2x_context_t *ctx = (sha2x_context_t *)in_ctx;
	int digest_len;

	digest_len = sha2_digest_len(ctx->hash_type);
	if (!digest_len)
		goto done;
	digest_len = MIN(out_digest_len, digest_len);

	ctx->block[ctx->len++] = 0x80;
	if (ctx->len > 112) {
		memset(ctx->block + ctx->len, 0, SHA2X_BLOCK_SZ - ctx->len);
		sha2x_transform(ctx->state, ctx->block);
		ctx->len = 0;
	}
	memset(ctx->block + ctx->len, 0, 112 - ctx->len);
	ctx->ctr[0] = ctx->ctr[0]<<3 | ctx->ctr[1]>>61;
	ctx->ctr[1] <<= 3;
	sha2x_u64tou8(ctx->block+112, ctx->ctr, 2 * sizeof(uint64));
	sha2x_transform(ctx->state, ctx->block);

	sha2x_u64tou8(digest, ctx->state, digest_len);
done:;
}

int sha2x_init(void *in_ctx, sha2_hash_type_t hash_type)
{
	sha2x_context_t *ctx = (sha2x_context_t *)in_ctx;
	const uint64 *p;

	ctx->hash_type = hash_type;
	switch (hash_type) {
	case HASH_SHA512_224:
		p = sha512_224_init; break;
	case HASH_SHA512_256:
		p = sha512_256_init; break;
	case HASH_SHA384:
		p = sha384_init; break;
	case HASH_SHA512:
		p = sha512_init; break;
	default:
		return BCME_UNSUPPORTED;
	}

	memcpy(ctx->state, p, 64);
	ctx->len = 0;
	ctx->ctr[0] = 0;
	ctx->ctr[1] = 0;
	return BCME_OK;
}
