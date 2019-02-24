/*
 * Copyright (C) 2015 Martin Willi
 * Copyright (C) 2015 revosec AG
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include "aesni_gcm.h"
#include "aesni_key.h"

#include <crypto/iv/iv_gen_seq.h>

#include <tmmintrin.h>

#define NONCE_SIZE 12
#define IV_SIZE 8
#define SALT_SIZE (NONCE_SIZE - IV_SIZE)

/**
 * Parallel pipelining
 */
#define GCM_CRYPT_PARALLELISM 4

typedef struct private_aesni_gcm_t private_aesni_gcm_t;

/**
 * GCM en/decryption method type
 */
typedef void (*aesni_gcm_fn_t)(private_aesni_gcm_t*, size_t, u_char*, u_char*,
							   u_char*, size_t, u_char*, u_char*);

/**
 * Private data of an aesni_gcm_t object.
 */
struct private_aesni_gcm_t {

	/**
	 * Public aesni_gcm_t interface.
	 */
	aesni_gcm_t public;

	/**
	 * Encryption key schedule
	 */
	aesni_key_t *key;

	/**
	 * IV generator.
	 */
	iv_gen_t *iv_gen;

	/**
	 * Length of the integrity check value
	 */
	size_t icv_size;

	/**
	 * Length of the key in bytes
	 */
	size_t key_size;

	/**
	 * GCM encryption function
	 */
	aesni_gcm_fn_t encrypt;

	/**
	 * GCM decryption function
	 */
	aesni_gcm_fn_t decrypt;

	/**
	 * salt to add to nonce
	 */
	u_char salt[SALT_SIZE];

	/**
	 * GHASH subkey H, big-endian
	 */
	__m128i h;

	/**
	 * GHASH key H^2, big-endian
	 */
	__m128i hh;

	/**
	 * GHASH key H^3, big-endian
	 */
	__m128i hhh;

	/**
	 * GHASH key H^4, big-endian
	 */
	__m128i hhhh;
};

/**
 * Byte-swap a 128-bit integer
 */
static inline __m128i swap128(__m128i x)
{
	return _mm_shuffle_epi8(x,
			_mm_set_epi8(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15));
}

/**
 * Multiply two blocks in GF128
 */
static __m128i mult_block(__m128i h, __m128i y)
{
	__m128i t1, t2, t3, t4, t5, t6;

	y = swap128(y);

	t1 = _mm_clmulepi64_si128(h, y, 0x00);
	t2 = _mm_clmulepi64_si128(h, y, 0x01);
	t3 = _mm_clmulepi64_si128(h, y, 0x10);
	t4 = _mm_clmulepi64_si128(h, y, 0x11);

	t2 = _mm_xor_si128(t2, t3);
	t3 = _mm_slli_si128(t2, 8);
	t2 = _mm_srli_si128(t2, 8);
	t1 = _mm_xor_si128(t1, t3);
	t4 = _mm_xor_si128(t4, t2);

	t5 = _mm_srli_epi32(t1, 31);
	t1 = _mm_slli_epi32(t1, 1);
	t6 = _mm_srli_epi32(t4, 31);
	t4 = _mm_slli_epi32(t4, 1);

	t3 = _mm_srli_si128(t5, 12);
	t6 = _mm_slli_si128(t6, 4);
	t5 = _mm_slli_si128(t5, 4);
	t1 = _mm_or_si128(t1, t5);
	t4 = _mm_or_si128(t4, t6);
	t4 = _mm_or_si128(t4, t3);

	t5 = _mm_slli_epi32(t1, 31);
	t6 = _mm_slli_epi32(t1, 30);
	t3 = _mm_slli_epi32(t1, 25);

	t5 = _mm_xor_si128(t5, t6);
	t5 = _mm_xor_si128(t5, t3);
	t6 = _mm_srli_si128(t5, 4);
	t4 = _mm_xor_si128(t4, t6);
	t5 = _mm_slli_si128(t5, 12);
	t1 = _mm_xor_si128(t1, t5);
	t4 = _mm_xor_si128(t4, t1);

	t5 = _mm_srli_epi32(t1, 1);
	t2 = _mm_srli_epi32(t1, 2);
	t3 = _mm_srli_epi32(t1, 7);
	t4 = _mm_xor_si128(t4, t2);
	t4 = _mm_xor_si128(t4, t3);
	t4 = _mm_xor_si128(t4, t5);

	return swap128(t4);
}

/**
 * Multiply four consecutive blocks by their respective GHASH key, XOR
 */
static inline __m128i mult4xor(__m128i h1, __m128i h2, __m128i h3, __m128i h4,
							   __m128i d1, __m128i d2, __m128i d3, __m128i d4)
{
	__m128i t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;

	d1 = swap128(d1);
	d2 = swap128(d2);
	d3 = swap128(d3);
	d4 = swap128(d4);

	t0 = _mm_clmulepi64_si128(h1, d1, 0x00);
	t1 = _mm_clmulepi64_si128(h2, d2, 0x00);
	t2 = _mm_clmulepi64_si128(h3, d3, 0x00);
	t3 = _mm_clmulepi64_si128(h4, d4, 0x00);
	t8 = _mm_xor_si128(t0, t1);
	t8 = _mm_xor_si128(t8, t2);
	t8 = _mm_xor_si128(t8, t3);

	t4 = _mm_clmulepi64_si128(h1, d1, 0x11);
	t5 = _mm_clmulepi64_si128(h2, d2, 0x11);
	t6 = _mm_clmulepi64_si128(h3, d3, 0x11);
	t7 = _mm_clmulepi64_si128(h4, d4, 0x11);
	t9 = _mm_xor_si128(t4, t5);
	t9 = _mm_xor_si128(t9, t6);
	t9 = _mm_xor_si128(t9, t7);

	t0 = _mm_shuffle_epi32(h1, 78);
	t4 = _mm_shuffle_epi32(d1, 78);
	t0 = _mm_xor_si128(t0, h1);
	t4 = _mm_xor_si128(t4, d1);
	t1 = _mm_shuffle_epi32(h2, 78);
	t5 = _mm_shuffle_epi32(d2, 78);
	t1 = _mm_xor_si128(t1, h2);
	t5 = _mm_xor_si128(t5, d2);
	t2 = _mm_shuffle_epi32(h3, 78);
	t6 = _mm_shuffle_epi32(d3, 78);
	t2 = _mm_xor_si128(t2, h3);
	t6 = _mm_xor_si128(t6, d3);
	t3 = _mm_shuffle_epi32(h4, 78);
	t7 = _mm_shuffle_epi32(d4, 78);
	t3 = _mm_xor_si128(t3, h4);
	t7 = _mm_xor_si128(t7, d4);

	t0 = _mm_clmulepi64_si128(t0, t4, 0x00);
	t1 = _mm_clmulepi64_si128(t1, t5, 0x00);
	t2 = _mm_clmulepi64_si128(t2, t6, 0x00);
	t3 = _mm_clmulepi64_si128(t3, t7, 0x00);
	t0 = _mm_xor_si128(t0, t8);
	t0 = _mm_xor_si128(t0, t9);
	t0 = _mm_xor_si128(t1, t0);
	t0 = _mm_xor_si128(t2, t0);

	t0 = _mm_xor_si128(t3, t0);
	t4 = _mm_slli_si128(t0, 8);
	t0 = _mm_srli_si128(t0, 8);
	t3 = _mm_xor_si128(t4, t8);
	t6 = _mm_xor_si128(t0, t9);
	t7 = _mm_srli_epi32(t3, 31);
	t8 = _mm_srli_epi32(t6, 31);
	t3 = _mm_slli_epi32(t3, 1);
	t6 = _mm_slli_epi32(t6, 1);
	t9 = _mm_srli_si128(t7, 12);
	t8 = _mm_slli_si128(t8, 4);
	t7 = _mm_slli_si128(t7, 4);
	t3 = _mm_or_si128(t3, t7);
	t6 = _mm_or_si128(t6, t8);
	t6 = _mm_or_si128(t6, t9);
	t7 = _mm_slli_epi32(t3, 31);
	t8 = _mm_slli_epi32(t3, 30);
	t9 = _mm_slli_epi32(t3, 25);
	t7 = _mm_xor_si128(t7, t8);
	t7 = _mm_xor_si128(t7, t9);
	t8 = _mm_srli_si128(t7, 4);
	t7 = _mm_slli_si128(t7, 12);
	t3 = _mm_xor_si128(t3, t7);
	t2 = _mm_srli_epi32(t3, 1);
	t4 = _mm_srli_epi32(t3, 2);
	t5 = _mm_srli_epi32(t3, 7);
	t2 = _mm_xor_si128(t2, t4);
	t2 = _mm_xor_si128(t2, t5);
	t2 = _mm_xor_si128(t2, t8);
	t3 = _mm_xor_si128(t3, t2);
	t6 = _mm_xor_si128(t6, t3);

	return swap128(t6);
}

/**
 * GHASH on a single block
 */
static __m128i ghash(__m128i h, __m128i y, __m128i x)
{
	return mult_block(h, _mm_xor_si128(y, x));
}

/**
 * Start constructing the ICV for the associated data
 */
static __m128i icv_header(private_aesni_gcm_t *this, void *assoc, size_t alen)
{
	u_int blocks, pblocks, rem, i;
	__m128i h1, h2, h3, h4, d1, d2, d3, d4;
	__m128i y, last, *ab;

	h1 = this->hhhh;
	h2 = this->hhh;
	h3 = this->hh;
	h4 = this->h;

	y = _mm_setzero_si128();
	ab = assoc;
	blocks = alen / AES_BLOCK_SIZE;
	pblocks = blocks - (blocks % GCM_CRYPT_PARALLELISM);
	rem = alen % AES_BLOCK_SIZE;
	for (i = 0; i < pblocks; i += GCM_CRYPT_PARALLELISM)
	{
		d1 = _mm_loadu_si128(ab + i + 0);
		d2 = _mm_loadu_si128(ab + i + 1);
		d3 = _mm_loadu_si128(ab + i + 2);
		d4 = _mm_loadu_si128(ab + i + 3);
		y = _mm_xor_si128(y, d1);
		y = mult4xor(h1, h2, h3, h4, y, d2, d3, d4);
	}
	for (i = pblocks; i < blocks; i++)
	{
		y = ghash(this->h, y, _mm_loadu_si128(ab + i));
	}
	if (rem)
	{
		last = _mm_setzero_si128();
		memcpy(&last, ab + blocks, rem);

		y = ghash(this->h, y, last);
	}

	return y;
}

/**
 * Complete the ICV by hashing a assoc/data length block
 */
static __m128i icv_tailer(private_aesni_gcm_t *this, __m128i y,
						  size_t alen, size_t dlen)
{
	__m128i b;

	htoun64(&b, alen * 8);
	htoun64((u_char*)&b + sizeof(uint64_t), dlen * 8);

	return ghash(this->h, y, b);
}

/**
 * En-/Decrypt the ICV, trim and store it
 */
static void icv_crypt(private_aesni_gcm_t *this, __m128i y, __m128i j,
					  u_char *icv)
{
	__m128i *ks, t, b;
	u_int round;

	ks = this->key->schedule;
	t = _mm_xor_si128(j, ks[0]);
	for (round = 1; round < this->key->rounds; round++)
	{
		t = _mm_aesenc_si128(t, ks[round]);
	}
	t = _mm_aesenclast_si128(t, ks[this->key->rounds]);

	t = _mm_xor_si128(y, t);

	_mm_storeu_si128(&b, t);
	memcpy(icv, &b, this->icv_size);
}

/**
 * Do big-endian increment on x
 */
static inline __m128i increment_be(__m128i x)
{
	x = swap128(x);
	x = _mm_add_epi64(x, _mm_set_epi32(0, 0, 0, 1));
	x = swap128(x);

	return x;
}

/**
 * Generate the block J0
 */
static inline __m128i create_j(private_aesni_gcm_t *this, u_char *iv)
{
	u_char j[AES_BLOCK_SIZE];

	memcpy(j, this->salt, SALT_SIZE);
	memcpy(j + SALT_SIZE, iv, IV_SIZE);
	htoun32(j + SALT_SIZE + IV_SIZE, 1);

	return _mm_loadu_si128((__m128i*)j);
}

/**
 * Encrypt a remaining incomplete block, return updated Y
 */
static __m128i encrypt_gcm_rem(private_aesni_gcm_t *this, u_int rem,
							   void *in, void *out, __m128i cb, __m128i y)
{
	__m128i *ks, t, b;
	u_int round;

	memset(&b, 0, sizeof(b));
	memcpy(&b, in, rem);

	ks = this->key->schedule;
	t = _mm_xor_si128(cb, ks[0]);
	for (round = 1; round < this->key->rounds; round++)
	{
		t = _mm_aesenc_si128(t, ks[round]);
	}
	t = _mm_aesenclast_si128(t, ks[this->key->rounds]);
	b = _mm_xor_si128(t, b);

	memcpy(out, &b, rem);

	memset((u_char*)&b + rem, 0, AES_BLOCK_SIZE - rem);
	return ghash(this->h, y, b);
}

/**
 * Decrypt a remaining incomplete block, return updated Y
 */
static __m128i decrypt_gcm_rem(private_aesni_gcm_t *this, u_int rem,
							   void *in, void *out, __m128i cb, __m128i y)
{
	__m128i *ks, t, b;
	u_int round;

	memset(&b, 0, sizeof(b));
	memcpy(&b, in, rem);

	y = ghash(this->h, y, b);

	ks = this->key->schedule;
	t = _mm_xor_si128(cb, ks[0]);
	for (round = 1; round < this->key->rounds; round++)
	{
		t = _mm_aesenc_si128(t, ks[round]);
	}
	t = _mm_aesenclast_si128(t, ks[this->key->rounds]);
	b = _mm_xor_si128(t, b);

	memcpy(out, &b, rem);

	return y;
}

/**
 * AES-128 GCM encryption/ICV generation
 */
static void encrypt_gcm128(private_aesni_gcm_t *this,
						   size_t len, u_char *in, u_char *out, u_char *iv,
						   size_t alen, u_char *assoc, u_char *icv)
{
	__m128i d1, d2, d3, d4, t1, t2, t3, t4;
	__m128i *ks, y, j, cb, *bi, *bo;
	u_int blocks, pblocks, rem, i;

	j = create_j(this, iv);
	cb = increment_be(j);
	y = icv_header(this, assoc, alen);
	blocks = len / AES_BLOCK_SIZE;
	pblocks = blocks - (blocks % GCM_CRYPT_PARALLELISM);
	rem = len % AES_BLOCK_SIZE;
	bi = (__m128i*)in;
	bo = (__m128i*)out;

	ks = this->key->schedule;

	for (i = 0; i < pblocks; i += GCM_CRYPT_PARALLELISM)
	{
		d1 = _mm_loadu_si128(bi + i + 0);
		d2 = _mm_loadu_si128(bi + i + 1);
		d3 = _mm_loadu_si128(bi + i + 2);
		d4 = _mm_loadu_si128(bi + i + 3);

		t1 = _mm_xor_si128(cb, ks[0]);
		cb = increment_be(cb);
		t2 = _mm_xor_si128(cb, ks[0]);
		cb = increment_be(cb);
		t3 = _mm_xor_si128(cb, ks[0]);
		cb = increment_be(cb);
		t4 = _mm_xor_si128(cb, ks[0]);
		cb = increment_be(cb);

		t1 = _mm_aesenc_si128(t1, ks[1]);
		t2 = _mm_aesenc_si128(t2, ks[1]);
		t3 = _mm_aesenc_si128(t3, ks[1]);
		t4 = _mm_aesenc_si128(t4, ks[1]);
		t1 = _mm_aesenc_si128(t1, ks[2]);
		t2 = _mm_aesenc_si128(t2, ks[2]);
		t3 = _mm_aesenc_si128(t3, ks[2]);
		t4 = _mm_aesenc_si128(t4, ks[2]);
		t1 = _mm_aesenc_si128(t1, ks[3]);
		t2 = _mm_aesenc_si128(t2, ks[3]);
		t3 = _mm_aesenc_si128(t3, ks[3]);
		t4 = _mm_aesenc_si128(t4, ks[3]);
		t1 = _mm_aesenc_si128(t1, ks[4]);
		t2 = _mm_aesenc_si128(t2, ks[4]);
		t3 = _mm_aesenc_si128(t3, ks[4]);
		t4 = _mm_aesenc_si128(t4, ks[4]);
		t1 = _mm_aesenc_si128(t1, ks[5]);
		t2 = _mm_aesenc_si128(t2, ks[5]);
		t3 = _mm_aesenc_si128(t3, ks[5]);
		t4 = _mm_aesenc_si128(t4, ks[5]);
		t1 = _mm_aesenc_si128(t1, ks[6]);
		t2 = _mm_aesenc_si128(t2, ks[6]);
		t3 = _mm_aesenc_si128(t3, ks[6]);
		t4 = _mm_aesenc_si128(t4, ks[6]);
		t1 = _mm_aesenc_si128(t1, ks[7]);
		t2 = _mm_aesenc_si128(t2, ks[7]);
		t3 = _mm_aesenc_si128(t3, ks[7]);
		t4 = _mm_aesenc_si128(t4, ks[7]);
		t1 = _mm_aesenc_si128(t1, ks[8]);
		t2 = _mm_aesenc_si128(t2, ks[8]);
		t3 = _mm_aesenc_si128(t3, ks[8]);
		t4 = _mm_aesenc_si128(t4, ks[8]);
		t1 = _mm_aesenc_si128(t1, ks[9]);
		t2 = _mm_aesenc_si128(t2, ks[9]);
		t3 = _mm_aesenc_si128(t3, ks[9]);
		t4 = _mm_aesenc_si128(t4, ks[9]);

		t1 = _mm_aesenclast_si128(t1, ks[10]);
		t2 = _mm_aesenclast_si128(t2, ks[10]);
		t3 = _mm_aesenclast_si128(t3, ks[10]);
		t4 = _mm_aesenclast_si128(t4, ks[10]);

		t1 = _mm_xor_si128(t1, d1);
		t2 = _mm_xor_si128(t2, d2);
		t3 = _mm_xor_si128(t3, d3);
		t4 = _mm_xor_si128(t4, d4);

		y = _mm_xor_si128(y, t1);
		y = mult4xor(this->hhhh, this->hhh, this->hh, this->h, y, t2, t3, t4);

		_mm_storeu_si128(bo + i + 0, t1);
		_mm_storeu_si128(bo + i + 1, t2);
		_mm_storeu_si128(bo + i + 2, t3);
		_mm_storeu_si128(bo + i + 3, t4);
	}

	for (i = pblocks; i < blocks; i++)
	{
		d1 = _mm_loadu_si128(bi + i);

		t1 = _mm_xor_si128(cb, ks[0]);
		t1 = _mm_aesenc_si128(t1, ks[1]);
		t1 = _mm_aesenc_si128(t1, ks[2]);
		t1 = _mm_aesenc_si128(t1, ks[3]);
		t1 = _mm_aesenc_si128(t1, ks[4]);
		t1 = _mm_aesenc_si128(t1, ks[5]);
		t1 = _mm_aesenc_si128(t1, ks[6]);
		t1 = _mm_aesenc_si128(t1, ks[7]);
		t1 = _mm_aesenc_si128(t1, ks[8]);
		t1 = _mm_aesenc_si128(t1, ks[9]);
		t1 = _mm_aesenclast_si128(t1, ks[10]);

		t1 = _mm_xor_si128(t1, d1);
		_mm_storeu_si128(bo + i, t1);

		y = ghash(this->h, y, t1);

		cb = increment_be(cb);
	}

	if (rem)
	{
		y = encrypt_gcm_rem(this, rem, bi + blocks, bo + blocks, cb, y);
	}
	y = icv_tailer(this, y, alen, len);
	icv_crypt(this, y, j, icv);
}

/**
 * AES-128 GCM decryption/ICV generation
 */
static void decrypt_gcm128(private_aesni_gcm_t *this,
						   size_t len, u_char *in, u_char *out, u_char *iv,
						   size_t alen, u_char *assoc, u_char *icv)
{
	__m128i d1, d2, d3, d4, t1, t2, t3, t4;
	__m128i *ks, y, j, cb, *bi, *bo;
	u_int blocks, pblocks, rem, i;

	j = create_j(this, iv);
	cb = increment_be(j);
	y = icv_header(this, assoc, alen);
	blocks = len / AES_BLOCK_SIZE;
	pblocks = blocks - (blocks % GCM_CRYPT_PARALLELISM);
	rem = len % AES_BLOCK_SIZE;
	bi = (__m128i*)in;
	bo = (__m128i*)out;

	ks = this->key->schedule;

	for (i = 0; i < pblocks; i += GCM_CRYPT_PARALLELISM)
	{
		d1 = _mm_loadu_si128(bi + i + 0);
		d2 = _mm_loadu_si128(bi + i + 1);
		d3 = _mm_loadu_si128(bi + i + 2);
		d4 = _mm_loadu_si128(bi + i + 3);

		y = _mm_xor_si128(y, d1);
		y = mult4xor(this->hhhh, this->hhh, this->hh, this->h, y, d2, d3, d4);

		t1 = _mm_xor_si128(cb, ks[0]);
		cb = increment_be(cb);
		t2 = _mm_xor_si128(cb, ks[0]);
		cb = increment_be(cb);
		t3 = _mm_xor_si128(cb, ks[0]);
		cb = increment_be(cb);
		t4 = _mm_xor_si128(cb, ks[0]);
		cb = increment_be(cb);

		t1 = _mm_aesenc_si128(t1, ks[1]);
		t2 = _mm_aesenc_si128(t2, ks[1]);
		t3 = _mm_aesenc_si128(t3, ks[1]);
		t4 = _mm_aesenc_si128(t4, ks[1]);
		t1 = _mm_aesenc_si128(t1, ks[2]);
		t2 = _mm_aesenc_si128(t2, ks[2]);
		t3 = _mm_aesenc_si128(t3, ks[2]);
		t4 = _mm_aesenc_si128(t4, ks[2]);
		t1 = _mm_aesenc_si128(t1, ks[3]);
		t2 = _mm_aesenc_si128(t2, ks[3]);
		t3 = _mm_aesenc_si128(t3, ks[3]);
		t4 = _mm_aesenc_si128(t4, ks[3]);
		t1 = _mm_aesenc_si128(t1, ks[4]);
		t2 = _mm_aesenc_si128(t2, ks[4]);
		t3 = _mm_aesenc_si128(t3, ks[4]);
		t4 = _mm_aesenc_si128(t4, ks[4]);
		t1 = _mm_aesenc_si128(t1, ks[5]);
		t2 = _mm_aesenc_si128(t2, ks[5]);
		t3 = _mm_aesenc_si128(t3, ks[5]);
		t4 = _mm_aesenc_si128(t4, ks[5]);
		t1 = _mm_aesenc_si128(t1, ks[6]);
		t2 = _mm_aesenc_si128(t2, ks[6]);
		t3 = _mm_aesenc_si128(t3, ks[6]);
		t4 = _mm_aesenc_si128(t4, ks[6]);
		t1 = _mm_aesenc_si128(t1, ks[7]);
		t2 = _mm_aesenc_si128(t2, ks[7]);
		t3 = _mm_aesenc_si128(t3, ks[7]);
		t4 = _mm_aesenc_si128(t4, ks[7]);
		t1 = _mm_aesenc_si128(t1, ks[8]);
		t2 = _mm_aesenc_si128(t2, ks[8]);
		t3 = _mm_aesenc_si128(t3, ks[8]);
		t4 = _mm_aesenc_si128(t4, ks[8]);
		t1 = _mm_aesenc_si128(t1, ks[9]);
		t2 = _mm_aesenc_si128(t2, ks[9]);
		t3 = _mm_aesenc_si128(t3, ks[9]);
		t4 = _mm_aesenc_si128(t4, ks[9]);

		t1 = _mm_aesenclast_si128(t1, ks[10]);
		t2 = _mm_aesenclast_si128(t2, ks[10]);
		t3 = _mm_aesenclast_si128(t3, ks[10]);
		t4 = _mm_aesenclast_si128(t4, ks[10]);

		t1 = _mm_xor_si128(t1, d1);
		t2 = _mm_xor_si128(t2, d2);
		t3 = _mm_xor_si128(t3, d3);
		t4 = _mm_xor_si128(t4, d4);

		_mm_storeu_si128(bo + i + 0, t1);
		_mm_storeu_si128(bo + i + 1, t2);
		_mm_storeu_si128(bo + i + 2, t3);
		_mm_storeu_si128(bo + i + 3, t4);
	}

	for (i = pblocks; i < blocks; i++)
	{
		d1 = _mm_loadu_si128(bi + i);

		y = ghash(this->h, y, d1);

		t1 = _mm_xor_si128(cb, ks[0]);
		t1 = _mm_aesenc_si128(t1, ks[1]);
		t1 = _mm_aesenc_si128(t1, ks[2]);
		t1 = _mm_aesenc_si128(t1, ks[3]);
		t1 = _mm_aesenc_si128(t1, ks[4]);
		t1 = _mm_aesenc_si128(t1, ks[5]);
		t1 = _mm_aesenc_si128(t1, ks[6]);
		t1 = _mm_aesenc_si128(t1, ks[7]);
		t1 = _mm_aesenc_si128(t1, ks[8]);
		t1 = _mm_aesenc_si128(t1, ks[9]);
		t1 = _mm_aesenclast_si128(t1, ks[10]);

		t1 = _mm_xor_si128(t1, d1);
		_mm_storeu_si128(bo + i, t1);

		cb = increment_be(cb);
	}

	if (rem)
	{
		y = decrypt_gcm_rem(this, rem, bi + blocks, bo + blocks, cb, y);
	}
	y = icv_tailer(this, y, alen, len);
	icv_crypt(this, y, j, icv);
}

/**
 * AES-192 GCM encryption/ICV generation
 */
static void encrypt_gcm192(private_aesni_gcm_t *this,
						   size_t len, u_char *in, u_char *out, u_char *iv,
						   size_t alen, u_char *assoc, u_char *icv)
{
	__m128i d1, d2, d3, d4, t1, t2, t3, t4;
	__m128i *ks, y, j, cb, *bi, *bo;
	u_int blocks, pblocks, rem, i;

	j = create_j(this, iv);
	cb = increment_be(j);
	y = icv_header(this, assoc, alen);
	blocks = len / AES_BLOCK_SIZE;
	pblocks = blocks - (blocks % GCM_CRYPT_PARALLELISM);
	rem = len % AES_BLOCK_SIZE;
	bi = (__m128i*)in;
	bo = (__m128i*)out;

	ks = this->key->schedule;

	for (i = 0; i < pblocks; i += GCM_CRYPT_PARALLELISM)
	{
		d1 = _mm_loadu_si128(bi + i + 0);
		d2 = _mm_loadu_si128(bi + i + 1);
		d3 = _mm_loadu_si128(bi + i + 2);
		d4 = _mm_loadu_si128(bi + i + 3);

		t1 = _mm_xor_si128(cb, ks[0]);
		cb = increment_be(cb);
		t2 = _mm_xor_si128(cb, ks[0]);
		cb = increment_be(cb);
		t3 = _mm_xor_si128(cb, ks[0]);
		cb = increment_be(cb);
		t4 = _mm_xor_si128(cb, ks[0]);
		cb = increment_be(cb);

		t1 = _mm_aesenc_si128(t1, ks[1]);
		t2 = _mm_aesenc_si128(t2, ks[1]);
		t3 = _mm_aesenc_si128(t3, ks[1]);
		t4 = _mm_aesenc_si128(t4, ks[1]);
		t1 = _mm_aesenc_si128(t1, ks[2]);
		t2 = _mm_aesenc_si128(t2, ks[2]);
		t3 = _mm_aesenc_si128(t3, ks[2]);
		t4 = _mm_aesenc_si128(t4, ks[2]);
		t1 = _mm_aesenc_si128(t1, ks[3]);
		t2 = _mm_aesenc_si128(t2, ks[3]);
		t3 = _mm_aesenc_si128(t3, ks[3]);
		t4 = _mm_aesenc_si128(t4, ks[3]);
		t1 = _mm_aesenc_si128(t1, ks[4]);
		t2 = _mm_aesenc_si128(t2, ks[4]);
		t3 = _mm_aesenc_si128(t3, ks[4]);
		t4 = _mm_aesenc_si128(t4, ks[4]);
		t1 = _mm_aesenc_si128(t1, ks[5]);
		t2 = _mm_aesenc_si128(t2, ks[5]);
		t3 = _mm_aesenc_si128(t3, ks[5]);
		t4 = _mm_aesenc_si128(t4, ks[5]);
		t1 = _mm_aesenc_si128(t1, ks[6]);
		t2 = _mm_aesenc_si128(t2, ks[6]);
		t3 = _mm_aesenc_si128(t3, ks[6]);
		t4 = _mm_aesenc_si128(t4, ks[6]);
		t1 = _mm_aesenc_si128(t1, ks[7]);
		t2 = _mm_aesenc_si128(t2, ks[7]);
		t3 = _mm_aesenc_si128(t3, ks[7]);
		t4 = _mm_aesenc_si128(t4, ks[7]);
		t1 = _mm_aesenc_si128(t1, ks[8]);
		t2 = _mm_aesenc_si128(t2, ks[8]);
		t3 = _mm_aesenc_si128(t3, ks[8]);
		t4 = _mm_aesenc_si128(t4, ks[8]);
		t1 = _mm_aesenc_si128(t1, ks[9]);
		t2 = _mm_aesenc_si128(t2, ks[9]);
		t3 = _mm_aesenc_si128(t3, ks[9]);
		t4 = _mm_aesenc_si128(t4, ks[9]);
		t1 = _mm_aesenc_si128(t1, ks[10]);
		t2 = _mm_aesenc_si128(t2, ks[10]);
		t3 = _mm_aesenc_si128(t3, ks[10]);
		t4 = _mm_aesenc_si128(t4, ks[10]);
		t1 = _mm_aesenc_si128(t1, ks[11]);
		t2 = _mm_aesenc_si128(t2, ks[11]);
		t3 = _mm_aesenc_si128(t3, ks[11]);
		t4 = _mm_aesenc_si128(t4, ks[11]);

		t1 = _mm_aesenclast_si128(t1, ks[12]);
		t2 = _mm_aesenclast_si128(t2, ks[12]);
		t3 = _mm_aesenclast_si128(t3, ks[12]);
		t4 = _mm_aesenclast_si128(t4, ks[12]);

		t1 = _mm_xor_si128(t1, d1);
		t2 = _mm_xor_si128(t2, d2);
		t3 = _mm_xor_si128(t3, d3);
		t4 = _mm_xor_si128(t4, d4);

		y = _mm_xor_si128(y, t1);
		y = mult4xor(this->hhhh, this->hhh, this->hh, this->h, y, t2, t3, t4);

		_mm_storeu_si128(bo + i + 0, t1);
		_mm_storeu_si128(bo + i + 1, t2);
		_mm_storeu_si128(bo + i + 2, t3);
		_mm_storeu_si128(bo + i + 3, t4);
	}

	for (i = pblocks; i < blocks; i++)
	{
		d1 = _mm_loadu_si128(bi + i);

		t1 = _mm_xor_si128(cb, ks[0]);
		t1 = _mm_aesenc_si128(t1, ks[1]);
		t1 = _mm_aesenc_si128(t1, ks[2]);
		t1 = _mm_aesenc_si128(t1, ks[3]);
		t1 = _mm_aesenc_si128(t1, ks[4]);
		t1 = _mm_aesenc_si128(t1, ks[5]);
		t1 = _mm_aesenc_si128(t1, ks[6]);
		t1 = _mm_aesenc_si128(t1, ks[7]);
		t1 = _mm_aesenc_si128(t1, ks[8]);
		t1 = _mm_aesenc_si128(t1, ks[9]);
		t1 = _mm_aesenc_si128(t1, ks[10]);
		t1 = _mm_aesenc_si128(t1, ks[11]);
		t1 = _mm_aesenclast_si128(t1, ks[12]);

		t1 = _mm_xor_si128(t1, d1);
		_mm_storeu_si128(bo + i, t1);

		y = ghash(this->h, y, t1);

		cb = increment_be(cb);
	}

	if (rem)
	{
		y = encrypt_gcm_rem(this, rem, bi + blocks, bo + blocks, cb, y);
	}
	y = icv_tailer(this, y, alen, len);
	icv_crypt(this, y, j, icv);
}

/**
 * AES-192 GCM decryption/ICV generation
 */
static void decrypt_gcm192(private_aesni_gcm_t *this,
						   size_t len, u_char *in, u_char *out, u_char *iv,
						   size_t alen, u_char *assoc, u_char *icv)
{
	__m128i d1, d2, d3, d4, t1, t2, t3, t4;
	__m128i *ks, y, j, cb, *bi, *bo;
	u_int blocks, pblocks, rem, i;

	j = create_j(this, iv);
	cb = increment_be(j);
	y = icv_header(this, assoc, alen);
	blocks = len / AES_BLOCK_SIZE;
	pblocks = blocks - (blocks % GCM_CRYPT_PARALLELISM);
	rem = len % AES_BLOCK_SIZE;
	bi = (__m128i*)in;
	bo = (__m128i*)out;

	ks = this->key->schedule;

	for (i = 0; i < pblocks; i += GCM_CRYPT_PARALLELISM)
	{
		d1 = _mm_loadu_si128(bi + i + 0);
		d2 = _mm_loadu_si128(bi + i + 1);
		d3 = _mm_loadu_si128(bi + i + 2);
		d4 = _mm_loadu_si128(bi + i + 3);

		y = _mm_xor_si128(y, d1);
		y = mult4xor(this->hhhh, this->hhh, this->hh, this->h, y, d2, d3, d4);

		t1 = _mm_xor_si128(cb, ks[0]);
		cb = increment_be(cb);
		t2 = _mm_xor_si128(cb, ks[0]);
		cb = increment_be(cb);
		t3 = _mm_xor_si128(cb, ks[0]);
		cb = increment_be(cb);
		t4 = _mm_xor_si128(cb, ks[0]);
		cb = increment_be(cb);

		t1 = _mm_aesenc_si128(t1, ks[1]);
		t2 = _mm_aesenc_si128(t2, ks[1]);
		t3 = _mm_aesenc_si128(t3, ks[1]);
		t4 = _mm_aesenc_si128(t4, ks[1]);
		t1 = _mm_aesenc_si128(t1, ks[2]);
		t2 = _mm_aesenc_si128(t2, ks[2]);
		t3 = _mm_aesenc_si128(t3, ks[2]);
		t4 = _mm_aesenc_si128(t4, ks[2]);
		t1 = _mm_aesenc_si128(t1, ks[3]);
		t2 = _mm_aesenc_si128(t2, ks[3]);
		t3 = _mm_aesenc_si128(t3, ks[3]);
		t4 = _mm_aesenc_si128(t4, ks[3]);
		t1 = _mm_aesenc_si128(t1, ks[4]);
		t2 = _mm_aesenc_si128(t2, ks[4]);
		t3 = _mm_aesenc_si128(t3, ks[4]);
		t4 = _mm_aesenc_si128(t4, ks[4]);
		t1 = _mm_aesenc_si128(t1, ks[5]);
		t2 = _mm_aesenc_si128(t2, ks[5]);
		t3 = _mm_aesenc_si128(t3, ks[5]);
		t4 = _mm_aesenc_si128(t4, ks[5]);
		t1 = _mm_aesenc_si128(t1, ks[6]);
		t2 = _mm_aesenc_si128(t2, ks[6]);
		t3 = _mm_aesenc_si128(t3, ks[6]);
		t4 = _mm_aesenc_si128(t4, ks[6]);
		t1 = _mm_aesenc_si128(t1, ks[7]);
		t2 = _mm_aesenc_si128(t2, ks[7]);
		t3 = _mm_aesenc_si128(t3, ks[7]);
		t4 = _mm_aesenc_si128(t4, ks[7]);
		t1 = _mm_aesenc_si128(t1, ks[8]);
		t2 = _mm_aesenc_si128(t2, ks[8]);
		t3 = _mm_aesenc_si128(t3, ks[8]);
		t4 = _mm_aesenc_si128(t4, ks[8]);
		t1 = _mm_aesenc_si128(t1, ks[9]);
		t2 = _mm_aesenc_si128(t2, ks[9]);
		t3 = _mm_aesenc_si128(t3, ks[9]);
		t4 = _mm_aesenc_si128(t4, ks[9]);
		t1 = _mm_aesenc_si128(t1, ks[10]);
		t2 = _mm_aesenc_si128(t2, ks[10]);
		t3 = _mm_aesenc_si128(t3, ks[10]);
		t4 = _mm_aesenc_si128(t4, ks[10]);
		t1 = _mm_aesenc_si128(t1, ks[11]);
		t2 = _mm_aesenc_si128(t2, ks[11]);
		t3 = _mm_aesenc_si128(t3, ks[11]);
		t4 = _mm_aesenc_si128(t4, ks[11]);

		t1 = _mm_aesenclast_si128(t1, ks[12]);
		t2 = _mm_aesenclast_si128(t2, ks[12]);
		t3 = _mm_aesenclast_si128(t3, ks[12]);
		t4 = _mm_aesenclast_si128(t4, ks[12]);

		t1 = _mm_xor_si128(t1, d1);
		t2 = _mm_xor_si128(t2, d2);
		t3 = _mm_xor_si128(t3, d3);
		t4 = _mm_xor_si128(t4, d4);

		_mm_storeu_si128(bo + i + 0, t1);
		_mm_storeu_si128(bo + i + 1, t2);
		_mm_storeu_si128(bo + i + 2, t3);
		_mm_storeu_si128(bo + i + 3, t4);
	}

	for (i = pblocks; i < blocks; i++)
	{
		d1 = _mm_loadu_si128(bi + i);

		y = ghash(this->h, y, d1);

		t1 = _mm_xor_si128(cb, ks[0]);
		t1 = _mm_aesenc_si128(t1, ks[1]);
		t1 = _mm_aesenc_si128(t1, ks[2]);
		t1 = _mm_aesenc_si128(t1, ks[3]);
		t1 = _mm_aesenc_si128(t1, ks[4]);
		t1 = _mm_aesenc_si128(t1, ks[5]);
		t1 = _mm_aesenc_si128(t1, ks[6]);
		t1 = _mm_aesenc_si128(t1, ks[7]);
		t1 = _mm_aesenc_si128(t1, ks[8]);
		t1 = _mm_aesenc_si128(t1, ks[9]);
		t1 = _mm_aesenc_si128(t1, ks[10]);
		t1 = _mm_aesenc_si128(t1, ks[11]);
		t1 = _mm_aesenclast_si128(t1, ks[12]);

		t1 = _mm_xor_si128(t1, d1);
		_mm_storeu_si128(bo + i, t1);

		cb = increment_be(cb);
	}

	if (rem)
	{
		y = decrypt_gcm_rem(this, rem, bi + blocks, bo + blocks, cb, y);
	}
	y = icv_tailer(this, y, alen, len);
	icv_crypt(this, y, j, icv);
}

/**
 * AES-256 GCM encryption/ICV generation
 */
static void encrypt_gcm256(private_aesni_gcm_t *this,
						   size_t len, u_char *in, u_char *out, u_char *iv,
						   size_t alen, u_char *assoc, u_char *icv)
{
	__m128i d1, d2, d3, d4, t1, t2, t3, t4;
	__m128i *ks, y, j, cb, *bi, *bo;
	u_int blocks, pblocks, rem, i;

	j = create_j(this, iv);
	cb = increment_be(j);
	y = icv_header(this, assoc, alen);
	blocks = len / AES_BLOCK_SIZE;
	pblocks = blocks - (blocks % GCM_CRYPT_PARALLELISM);
	rem = len % AES_BLOCK_SIZE;
	bi = (__m128i*)in;
	bo = (__m128i*)out;

	ks = this->key->schedule;

	for (i = 0; i < pblocks; i += GCM_CRYPT_PARALLELISM)
	{
		d1 = _mm_loadu_si128(bi + i + 0);
		d2 = _mm_loadu_si128(bi + i + 1);
		d3 = _mm_loadu_si128(bi + i + 2);
		d4 = _mm_loadu_si128(bi + i + 3);

		t1 = _mm_xor_si128(cb, ks[0]);
		cb = increment_be(cb);
		t2 = _mm_xor_si128(cb, ks[0]);
		cb = increment_be(cb);
		t3 = _mm_xor_si128(cb, ks[0]);
		cb = increment_be(cb);
		t4 = _mm_xor_si128(cb, ks[0]);
		cb = increment_be(cb);

		t1 = _mm_aesenc_si128(t1, ks[1]);
		t2 = _mm_aesenc_si128(t2, ks[1]);
		t3 = _mm_aesenc_si128(t3, ks[1]);
		t4 = _mm_aesenc_si128(t4, ks[1]);
		t1 = _mm_aesenc_si128(t1, ks[2]);
		t2 = _mm_aesenc_si128(t2, ks[2]);
		t3 = _mm_aesenc_si128(t3, ks[2]);
		t4 = _mm_aesenc_si128(t4, ks[2]);
		t1 = _mm_aesenc_si128(t1, ks[3]);
		t2 = _mm_aesenc_si128(t2, ks[3]);
		t3 = _mm_aesenc_si128(t3, ks[3]);
		t4 = _mm_aesenc_si128(t4, ks[3]);
		t1 = _mm_aesenc_si128(t1, ks[4]);
		t2 = _mm_aesenc_si128(t2, ks[4]);
		t3 = _mm_aesenc_si128(t3, ks[4]);
		t4 = _mm_aesenc_si128(t4, ks[4]);
		t1 = _mm_aesenc_si128(t1, ks[5]);
		t2 = _mm_aesenc_si128(t2, ks[5]);
		t3 = _mm_aesenc_si128(t3, ks[5]);
		t4 = _mm_aesenc_si128(t4, ks[5]);
		t1 = _mm_aesenc_si128(t1, ks[6]);
		t2 = _mm_aesenc_si128(t2, ks[6]);
		t3 = _mm_aesenc_si128(t3, ks[6]);
		t4 = _mm_aesenc_si128(t4, ks[6]);
		t1 = _mm_aesenc_si128(t1, ks[7]);
		t2 = _mm_aesenc_si128(t2, ks[7]);
		t3 = _mm_aesenc_si128(t3, ks[7]);
		t4 = _mm_aesenc_si128(t4, ks[7]);
		t1 = _mm_aesenc_si128(t1, ks[8]);
		t2 = _mm_aesenc_si128(t2, ks[8]);
		t3 = _mm_aesenc_si128(t3, ks[8]);
		t4 = _mm_aesenc_si128(t4, ks[8]);
		t1 = _mm_aesenc_si128(t1, ks[9]);
		t2 = _mm_aesenc_si128(t2, ks[9]);
		t3 = _mm_aesenc_si128(t3, ks[9]);
		t4 = _mm_aesenc_si128(t4, ks[9]);
		t1 = _mm_aesenc_si128(t1, ks[10]);
		t2 = _mm_aesenc_si128(t2, ks[10]);
		t3 = _mm_aesenc_si128(t3, ks[10]);
		t4 = _mm_aesenc_si128(t4, ks[10]);
		t1 = _mm_aesenc_si128(t1, ks[11]);
		t2 = _mm_aesenc_si128(t2, ks[11]);
		t3 = _mm_aesenc_si128(t3, ks[11]);
		t4 = _mm_aesenc_si128(t4, ks[11]);
		t1 = _mm_aesenc_si128(t1, ks[12]);
		t2 = _mm_aesenc_si128(t2, ks[12]);
		t3 = _mm_aesenc_si128(t3, ks[12]);
		t4 = _mm_aesenc_si128(t4, ks[12]);
		t1 = _mm_aesenc_si128(t1, ks[13]);
		t2 = _mm_aesenc_si128(t2, ks[13]);
		t3 = _mm_aesenc_si128(t3, ks[13]);
		t4 = _mm_aesenc_si128(t4, ks[13]);

		t1 = _mm_aesenclast_si128(t1, ks[14]);
		t2 = _mm_aesenclast_si128(t2, ks[14]);
		t3 = _mm_aesenclast_si128(t3, ks[14]);
		t4 = _mm_aesenclast_si128(t4, ks[14]);

		t1 = _mm_xor_si128(t1, d1);
		t2 = _mm_xor_si128(t2, d2);
		t3 = _mm_xor_si128(t3, d3);
		t4 = _mm_xor_si128(t4, d4);

		y = _mm_xor_si128(y, t1);
		y = mult4xor(this->hhhh, this->hhh, this->hh, this->h, y, t2, t3, t4);

		_mm_storeu_si128(bo + i + 0, t1);
		_mm_storeu_si128(bo + i + 1, t2);
		_mm_storeu_si128(bo + i + 2, t3);
		_mm_storeu_si128(bo + i + 3, t4);
	}

	for (i = pblocks; i < blocks; i++)
	{
		d1 = _mm_loadu_si128(bi + i);

		t1 = _mm_xor_si128(cb, ks[0]);
		t1 = _mm_aesenc_si128(t1, ks[1]);
		t1 = _mm_aesenc_si128(t1, ks[2]);
		t1 = _mm_aesenc_si128(t1, ks[3]);
		t1 = _mm_aesenc_si128(t1, ks[4]);
		t1 = _mm_aesenc_si128(t1, ks[5]);
		t1 = _mm_aesenc_si128(t1, ks[6]);
		t1 = _mm_aesenc_si128(t1, ks[7]);
		t1 = _mm_aesenc_si128(t1, ks[8]);
		t1 = _mm_aesenc_si128(t1, ks[9]);
		t1 = _mm_aesenc_si128(t1, ks[10]);
		t1 = _mm_aesenc_si128(t1, ks[11]);
		t1 = _mm_aesenc_si128(t1, ks[12]);
		t1 = _mm_aesenc_si128(t1, ks[13]);
		t1 = _mm_aesenclast_si128(t1, ks[14]);

		t1 = _mm_xor_si128(t1, d1);
		_mm_storeu_si128(bo + i, t1);

		y = ghash(this->h, y, t1);

		cb = increment_be(cb);
	}

	if (rem)
	{
		y = encrypt_gcm_rem(this, rem, bi + blocks, bo + blocks, cb, y);
	}
	y = icv_tailer(this, y, alen, len);
	icv_crypt(this, y, j, icv);
}

/**
 * AES-256 GCM decryption/ICV generation
 */
static void decrypt_gcm256(private_aesni_gcm_t *this,
						   size_t len, u_char *in, u_char *out, u_char *iv,
						   size_t alen, u_char *assoc, u_char *icv)
{
	__m128i d1, d2, d3, d4, t1, t2, t3, t4;
	__m128i *ks, y, j, cb, *bi, *bo;
	u_int blocks, pblocks, rem, i;

	j = create_j(this, iv);
	cb = increment_be(j);
	y = icv_header(this, assoc, alen);
	blocks = len / AES_BLOCK_SIZE;
	pblocks = blocks - (blocks % GCM_CRYPT_PARALLELISM);
	rem = len % AES_BLOCK_SIZE;
	bi = (__m128i*)in;
	bo = (__m128i*)out;

	ks = this->key->schedule;

	for (i = 0; i < pblocks; i += GCM_CRYPT_PARALLELISM)
	{
		d1 = _mm_loadu_si128(bi + i + 0);
		d2 = _mm_loadu_si128(bi + i + 1);
		d3 = _mm_loadu_si128(bi + i + 2);
		d4 = _mm_loadu_si128(bi + i + 3);

		y = _mm_xor_si128(y, d1);
		y = mult4xor(this->hhhh, this->hhh, this->hh, this->h, y, d2, d3, d4);

		t1 = _mm_xor_si128(cb, ks[0]);
		cb = increment_be(cb);
		t2 = _mm_xor_si128(cb, ks[0]);
		cb = increment_be(cb);
		t3 = _mm_xor_si128(cb, ks[0]);
		cb = increment_be(cb);
		t4 = _mm_xor_si128(cb, ks[0]);
		cb = increment_be(cb);

		t1 = _mm_aesenc_si128(t1, ks[1]);
		t2 = _mm_aesenc_si128(t2, ks[1]);
		t3 = _mm_aesenc_si128(t3, ks[1]);
		t4 = _mm_aesenc_si128(t4, ks[1]);
		t1 = _mm_aesenc_si128(t1, ks[2]);
		t2 = _mm_aesenc_si128(t2, ks[2]);
		t3 = _mm_aesenc_si128(t3, ks[2]);
		t4 = _mm_aesenc_si128(t4, ks[2]);
		t1 = _mm_aesenc_si128(t1, ks[3]);
		t2 = _mm_aesenc_si128(t2, ks[3]);
		t3 = _mm_aesenc_si128(t3, ks[3]);
		t4 = _mm_aesenc_si128(t4, ks[3]);
		t1 = _mm_aesenc_si128(t1, ks[4]);
		t2 = _mm_aesenc_si128(t2, ks[4]);
		t3 = _mm_aesenc_si128(t3, ks[4]);
		t4 = _mm_aesenc_si128(t4, ks[4]);
		t1 = _mm_aesenc_si128(t1, ks[5]);
		t2 = _mm_aesenc_si128(t2, ks[5]);
		t3 = _mm_aesenc_si128(t3, ks[5]);
		t4 = _mm_aesenc_si128(t4, ks[5]);
		t1 = _mm_aesenc_si128(t1, ks[6]);
		t2 = _mm_aesenc_si128(t2, ks[6]);
		t3 = _mm_aesenc_si128(t3, ks[6]);
		t4 = _mm_aesenc_si128(t4, ks[6]);
		t1 = _mm_aesenc_si128(t1, ks[7]);
		t2 = _mm_aesenc_si128(t2, ks[7]);
		t3 = _mm_aesenc_si128(t3, ks[7]);
		t4 = _mm_aesenc_si128(t4, ks[7]);
		t1 = _mm_aesenc_si128(t1, ks[8]);
		t2 = _mm_aesenc_si128(t2, ks[8]);
		t3 = _mm_aesenc_si128(t3, ks[8]);
		t4 = _mm_aesenc_si128(t4, ks[8]);
		t1 = _mm_aesenc_si128(t1, ks[9]);
		t2 = _mm_aesenc_si128(t2, ks[9]);
		t3 = _mm_aesenc_si128(t3, ks[9]);
		t4 = _mm_aesenc_si128(t4, ks[9]);
		t1 = _mm_aesenc_si128(t1, ks[10]);
		t2 = _mm_aesenc_si128(t2, ks[10]);
		t3 = _mm_aesenc_si128(t3, ks[10]);
		t4 = _mm_aesenc_si128(t4, ks[10]);
		t1 = _mm_aesenc_si128(t1, ks[11]);
		t2 = _mm_aesenc_si128(t2, ks[11]);
		t3 = _mm_aesenc_si128(t3, ks[11]);
		t4 = _mm_aesenc_si128(t4, ks[11]);
		t1 = _mm_aesenc_si128(t1, ks[12]);
		t2 = _mm_aesenc_si128(t2, ks[12]);
		t3 = _mm_aesenc_si128(t3, ks[12]);
		t4 = _mm_aesenc_si128(t4, ks[12]);
		t1 = _mm_aesenc_si128(t1, ks[13]);
		t2 = _mm_aesenc_si128(t2, ks[13]);
		t3 = _mm_aesenc_si128(t3, ks[13]);
		t4 = _mm_aesenc_si128(t4, ks[13]);

		t1 = _mm_aesenclast_si128(t1, ks[14]);
		t2 = _mm_aesenclast_si128(t2, ks[14]);
		t3 = _mm_aesenclast_si128(t3, ks[14]);
		t4 = _mm_aesenclast_si128(t4, ks[14]);

		t1 = _mm_xor_si128(t1, d1);
		t2 = _mm_xor_si128(t2, d2);
		t3 = _mm_xor_si128(t3, d3);
		t4 = _mm_xor_si128(t4, d4);

		_mm_storeu_si128(bo + i + 0, t1);
		_mm_storeu_si128(bo + i + 1, t2);
		_mm_storeu_si128(bo + i + 2, t3);
		_mm_storeu_si128(bo + i + 3, t4);
	}

	for (i = pblocks; i < blocks; i++)
	{
		d1 = _mm_loadu_si128(bi + i);

		y = ghash(this->h, y, d1);

		t1 = _mm_xor_si128(cb, ks[0]);
		t1 = _mm_aesenc_si128(t1, ks[1]);
		t1 = _mm_aesenc_si128(t1, ks[2]);
		t1 = _mm_aesenc_si128(t1, ks[3]);
		t1 = _mm_aesenc_si128(t1, ks[4]);
		t1 = _mm_aesenc_si128(t1, ks[5]);
		t1 = _mm_aesenc_si128(t1, ks[6]);
		t1 = _mm_aesenc_si128(t1, ks[7]);
		t1 = _mm_aesenc_si128(t1, ks[8]);
		t1 = _mm_aesenc_si128(t1, ks[9]);
		t1 = _mm_aesenc_si128(t1, ks[10]);
		t1 = _mm_aesenc_si128(t1, ks[11]);
		t1 = _mm_aesenc_si128(t1, ks[12]);
		t1 = _mm_aesenc_si128(t1, ks[13]);
		t1 = _mm_aesenclast_si128(t1, ks[14]);

		t1 = _mm_xor_si128(t1, d1);
		_mm_storeu_si128(bo + i, t1);

		cb = increment_be(cb);
	}

	if (rem)
	{
		y = decrypt_gcm_rem(this, rem, bi + blocks, bo + blocks, cb, y);
	}
	y = icv_tailer(this, y, alen, len);
	icv_crypt(this, y, j, icv);
}

METHOD(aead_t, encrypt, bool,
	private_aesni_gcm_t *this, chunk_t plain, chunk_t assoc, chunk_t iv,
	chunk_t *encr)
{
	u_char *out;

	if (!this->key || iv.len != IV_SIZE)
	{
		return FALSE;
	}
	out = plain.ptr;
	if (encr)
	{
		*encr = chunk_alloc(plain.len + this->icv_size);
		out = encr->ptr;
	}
	this->encrypt(this, plain.len, plain.ptr, out, iv.ptr,
				  assoc.len, assoc.ptr, out + plain.len);
	return TRUE;
}

METHOD(aead_t, decrypt, bool,
	private_aesni_gcm_t *this, chunk_t encr, chunk_t assoc, chunk_t iv,
	chunk_t *plain)
{
	u_char *out, icv[this->icv_size];

	if (!this->key || iv.len != IV_SIZE || encr.len < this->icv_size)
	{
		return FALSE;
	}
	encr.len -= this->icv_size;
	out = encr.ptr;
	if (plain)
	{
		*plain = chunk_alloc(encr.len);
		out = plain->ptr;
	}
	this->decrypt(this, encr.len, encr.ptr, out, iv.ptr,
				  assoc.len, assoc.ptr, icv);
	return memeq_const(icv, encr.ptr + encr.len, this->icv_size);
}

METHOD(aead_t, get_block_size, size_t,
	private_aesni_gcm_t *this)
{
	return 1;
}

METHOD(aead_t, get_icv_size, size_t,
	private_aesni_gcm_t *this)
{
	return this->icv_size;
}

METHOD(aead_t, get_iv_size, size_t,
	private_aesni_gcm_t *this)
{
	return IV_SIZE;
}

METHOD(aead_t, get_iv_gen, iv_gen_t*,
	private_aesni_gcm_t *this)
{
	return this->iv_gen;
}

METHOD(aead_t, get_key_size, size_t,
	private_aesni_gcm_t *this)
{
	return this->key_size + SALT_SIZE;
}

METHOD(aead_t, set_key, bool,
	private_aesni_gcm_t *this, chunk_t key)
{
	u_int round;
	__m128i *ks, h;

	if (key.len != this->key_size + SALT_SIZE)
	{
		return FALSE;
	}

	memcpy(this->salt, key.ptr + key.len - SALT_SIZE, SALT_SIZE);
	key.len -= SALT_SIZE;

	DESTROY_IF(this->key);
	this->key = aesni_key_create(TRUE, key);

	ks = this->key->schedule;
	h = _mm_xor_si128(_mm_setzero_si128(), ks[0]);
	for (round = 1; round < this->key->rounds; round++)
	{
		h = _mm_aesenc_si128(h, ks[round]);
	}
	h = _mm_aesenclast_si128(h, ks[this->key->rounds]);

	this->h = h;
	h = swap128(h);
	this->hh = mult_block(h, this->h);
	this->hhh = mult_block(h, this->hh);
	this->hhhh = mult_block(h, this->hhh);
	this->h = swap128(this->h);
	this->hh = swap128(this->hh);
	this->hhh = swap128(this->hhh);
	this->hhhh = swap128(this->hhhh);

	return TRUE;
}

METHOD(aead_t, destroy, void,
	private_aesni_gcm_t *this)
{
	DESTROY_IF(this->key);
	memwipe(&this->h, sizeof(this->h));
	memwipe(&this->hh, sizeof(this->hh));
	memwipe(&this->hhh, sizeof(this->hhh));
	memwipe(&this->hhhh, sizeof(this->hhhh));
	this->iv_gen->destroy(this->iv_gen);
	free_align(this);
}

/**
 * See header
 */
aesni_gcm_t *aesni_gcm_create(encryption_algorithm_t algo,
							  size_t key_size, size_t salt_size)
{
	private_aesni_gcm_t *this;
	size_t icv_size;

	switch (key_size)
	{
		case 0:
			key_size = 16;
			break;
		case 16:
		case 24:
		case 32:
			break;
		default:
			return NULL;
	}
	if (salt_size && salt_size != SALT_SIZE)
	{
		/* currently not supported */
		return NULL;
	}
	switch (algo)
	{
		case ENCR_AES_GCM_ICV8:
			algo = ENCR_AES_CBC;
			icv_size = 8;
			break;
		case ENCR_AES_GCM_ICV12:
			algo = ENCR_AES_CBC;
			icv_size = 12;
			break;
		case ENCR_AES_GCM_ICV16:
			algo = ENCR_AES_CBC;
			icv_size = 16;
			break;
		default:
			return NULL;
	}

	INIT_ALIGN(this, sizeof(__m128i),
		.public = {
			.aead = {
				.encrypt = _encrypt,
				.decrypt = _decrypt,
				.get_block_size = _get_block_size,
				.get_icv_size = _get_icv_size,
				.get_iv_size = _get_iv_size,
				.get_iv_gen = _get_iv_gen,
				.get_key_size = _get_key_size,
				.set_key = _set_key,
				.destroy = _destroy,
			},
		},
		.key_size = key_size,
		.iv_gen = iv_gen_seq_create(),
		.icv_size = icv_size,
	);

	switch (key_size)
	{
		case 16:
			this->encrypt = encrypt_gcm128;
			this->decrypt = decrypt_gcm128;
			break;
		case 24:
			this->encrypt = encrypt_gcm192;
			this->decrypt = decrypt_gcm192;
			break;
		case 32:
			this->encrypt = encrypt_gcm256;
			this->decrypt = decrypt_gcm256;
			break;
	}

	return &this->public;
}
