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

#include "aesni_ctr.h"
#include "aesni_key.h"

#include <tmmintrin.h>

/**
 * Pipeline parallelism we use for CTR en/decryption
 */
#define CTR_CRYPT_PARALLELISM 4

typedef struct private_aesni_ctr_t private_aesni_ctr_t;

/**
 * CTR en/decryption method type
 */
typedef void (*aesni_ctr_fn_t)(private_aesni_ctr_t*, size_t, u_char*, u_char*);

/**
 * Private data of an aesni_ctr_t object.
 */
struct private_aesni_ctr_t {

	/**
	 * Public aesni_ctr_t interface.
	 */
	aesni_ctr_t public;

	/**
	 * Key size
	 */
	u_int key_size;

	/**
	 * Key schedule
	 */
	aesni_key_t *key;

	/**
	 * Encryption method
	 */
	aesni_ctr_fn_t crypt;

	/**
	 * Counter state
	 */
	struct {
		char nonce[4];
		char iv[8];
		uint32_t counter;
	} __attribute__((packed, aligned(sizeof(__m128i)))) state;
};

/**
 * Do big-endian increment on x
 */
static inline __m128i increment_be(__m128i x)
{
	__m128i swap;

	swap = _mm_setr_epi8(15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);

	x = _mm_shuffle_epi8(x, swap);
	x = _mm_add_epi64(x, _mm_set_epi32(0, 0, 0, 1));
	x = _mm_shuffle_epi8(x, swap);

	return x;
}

/**
 * AES-128 CTR encryption
 */
static void encrypt_ctr128(private_aesni_ctr_t *this,
						   size_t len, u_char *in, u_char *out)
{
	__m128i t1, t2, t3, t4;
	__m128i d1, d2, d3, d4;
	__m128i *ks, state, b, *bi, *bo;
	u_int i, blocks, pblocks, rem;

	state = _mm_load_si128((__m128i*)&this->state);
	blocks = len / AES_BLOCK_SIZE;
	pblocks = blocks - (blocks % CTR_CRYPT_PARALLELISM);
	rem = len % AES_BLOCK_SIZE;
	bi = (__m128i*)in;
	bo = (__m128i*)out;

	ks = this->key->schedule;

	for (i = 0; i < pblocks; i += CTR_CRYPT_PARALLELISM)
	{
		d1 = _mm_loadu_si128(bi + i + 0);
		d2 = _mm_loadu_si128(bi + i + 1);
		d3 = _mm_loadu_si128(bi + i + 2);
		d4 = _mm_loadu_si128(bi + i + 3);

		t1 = _mm_xor_si128(state, ks[0]);
		state = increment_be(state);
		t2 = _mm_xor_si128(state, ks[0]);
		state = increment_be(state);
		t3 = _mm_xor_si128(state, ks[0]);
		state = increment_be(state);
		t4 = _mm_xor_si128(state, ks[0]);
		state = increment_be(state);

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

		t1 = _mm_xor_si128(state, ks[0]);
		state = increment_be(state);

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
	}

	if (rem)
	{
		memset(&b, 0, sizeof(b));
		memcpy(&b, bi + blocks, rem);

		d1 = _mm_loadu_si128(&b);
		t1 = _mm_xor_si128(state, ks[0]);

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
		_mm_storeu_si128(&b, t1);

		memcpy(bo + blocks, &b, rem);
	}
}

/**
 * AES-192 CTR encryption
 */
static void encrypt_ctr192(private_aesni_ctr_t *this,
						   size_t len, u_char *in, u_char *out)
{
	__m128i t1, t2, t3, t4;
	__m128i d1, d2, d3, d4;
	__m128i *ks, state, b, *bi, *bo;
	u_int i, blocks, pblocks, rem;

	state = _mm_load_si128((__m128i*)&this->state);
	blocks = len / AES_BLOCK_SIZE;
	pblocks = blocks - (blocks % CTR_CRYPT_PARALLELISM);
	rem = len % AES_BLOCK_SIZE;
	bi = (__m128i*)in;
	bo = (__m128i*)out;

	ks = this->key->schedule;

	for (i = 0; i < pblocks; i += CTR_CRYPT_PARALLELISM)
	{
		d1 = _mm_loadu_si128(bi + i + 0);
		d2 = _mm_loadu_si128(bi + i + 1);
		d3 = _mm_loadu_si128(bi + i + 2);
		d4 = _mm_loadu_si128(bi + i + 3);

		t1 = _mm_xor_si128(state, ks[0]);
		state = increment_be(state);
		t2 = _mm_xor_si128(state, ks[0]);
		state = increment_be(state);
		t3 = _mm_xor_si128(state, ks[0]);
		state = increment_be(state);
		t4 = _mm_xor_si128(state, ks[0]);
		state = increment_be(state);

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

		t1 = _mm_xor_si128(state, ks[0]);
		state = increment_be(state);

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
	}

	if (rem)
	{
		memset(&b, 0, sizeof(b));
		memcpy(&b, bi + blocks, rem);

		d1 = _mm_loadu_si128(&b);
		t1 = _mm_xor_si128(state, ks[0]);

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
		_mm_storeu_si128(&b, t1);

		memcpy(bo + blocks, &b, rem);
	}
}

/**
 * AES-256 CTR encryption
 */
static void encrypt_ctr256(private_aesni_ctr_t *this,
						   size_t len, u_char *in, u_char *out)
{
	__m128i t1, t2, t3, t4;
	__m128i d1, d2, d3, d4;
	__m128i *ks, state, b, *bi, *bo;
	u_int i, blocks, pblocks, rem;

	state = _mm_load_si128((__m128i*)&this->state);
	blocks = len / AES_BLOCK_SIZE;
	pblocks = blocks - (blocks % CTR_CRYPT_PARALLELISM);
	rem = len % AES_BLOCK_SIZE;
	bi = (__m128i*)in;
	bo = (__m128i*)out;

	ks = this->key->schedule;

	for (i = 0; i < pblocks; i += CTR_CRYPT_PARALLELISM)
	{
		d1 = _mm_loadu_si128(bi + i + 0);
		d2 = _mm_loadu_si128(bi + i + 1);
		d3 = _mm_loadu_si128(bi + i + 2);
		d4 = _mm_loadu_si128(bi + i + 3);

		t1 = _mm_xor_si128(state, ks[0]);
		state = increment_be(state);
		t2 = _mm_xor_si128(state, ks[0]);
		state = increment_be(state);
		t3 = _mm_xor_si128(state, ks[0]);
		state = increment_be(state);
		t4 = _mm_xor_si128(state, ks[0]);
		state = increment_be(state);

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

		t1 = _mm_xor_si128(state, ks[0]);
		state = increment_be(state);

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
	}

	if (rem)
	{
		memset(&b, 0, sizeof(b));
		memcpy(&b, bi + blocks, rem);

		d1 = _mm_loadu_si128(&b);
		t1 = _mm_xor_si128(state, ks[0]);

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
		_mm_storeu_si128(&b, t1);

		memcpy(bo + blocks, &b, rem);
	}
}

METHOD(crypter_t, crypt, bool,
	private_aesni_ctr_t *this, chunk_t in, chunk_t iv, chunk_t *out)
{
	u_char *buf;

	if (!this->key || iv.len != sizeof(this->state.iv))
	{
		return FALSE;
	}
	memcpy(this->state.iv, iv.ptr, sizeof(this->state.iv));
	this->state.counter = htonl(1);

	buf = in.ptr;
	if (out)
	{
		*out = chunk_alloc(in.len);
		buf = out->ptr;
	}
	this->crypt(this, in.len, in.ptr, buf);
	return TRUE;
}

METHOD(crypter_t, get_block_size, size_t,
	private_aesni_ctr_t *this)
{
	return 1;
}

METHOD(crypter_t, get_iv_size, size_t,
	private_aesni_ctr_t *this)
{
	return sizeof(this->state.iv);
}

METHOD(crypter_t, get_key_size, size_t,
	private_aesni_ctr_t *this)
{
	return this->key_size + sizeof(this->state.nonce);
}

METHOD(crypter_t, set_key, bool,
	private_aesni_ctr_t *this, chunk_t key)
{
	if (key.len != get_key_size(this))
	{
		return FALSE;
	}

	memcpy(this->state.nonce, key.ptr + key.len - sizeof(this->state.nonce),
		   sizeof(this->state.nonce));
	key.len -= sizeof(this->state.nonce);

	DESTROY_IF(this->key);
	this->key = aesni_key_create(TRUE, key);

	return this->key;
}

METHOD(crypter_t, destroy, void,
	private_aesni_ctr_t *this)
{
	DESTROY_IF(this->key);
	free_align(this);
}

/**
 * See header
 */
aesni_ctr_t *aesni_ctr_create(encryption_algorithm_t algo, size_t key_size)
{
	private_aesni_ctr_t *this;

	if (algo != ENCR_AES_CTR)
	{
		return NULL;
	}
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

	INIT_ALIGN(this, sizeof(__m128i),
		.public = {
			.crypter = {
				.encrypt = _crypt,
				.decrypt = _crypt,
				.get_block_size = _get_block_size,
				.get_iv_size = _get_iv_size,
				.get_key_size = _get_key_size,
				.set_key = _set_key,
				.destroy = _destroy,
			},
		},
		.key_size = key_size,
	);

	switch (key_size)
	{
		case 16:
			this->crypt = encrypt_ctr128;
			break;
		case 24:
			this->crypt = encrypt_ctr192;
			break;
		case 32:
			this->crypt = encrypt_ctr256;
			break;
	}

	return &this->public;
}
