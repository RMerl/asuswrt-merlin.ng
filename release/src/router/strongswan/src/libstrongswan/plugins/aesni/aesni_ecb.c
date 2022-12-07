/*
 * Copyright (C) 2019 Andreas Steffen
 * Copyright (C) 2015 Martin Willi
 *
 * Copyright (C) secunet Security Networks AG
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

#include "aesni_ecb.h"
#include "aesni_key.h"

/**
 * Pipeline parallelism we use for ECB encryption/decryption
 */
#define ECB_PARALLELISM 4

typedef struct private_aesni_ecb_t private_aesni_ecb_t;

/**
 * ECB en/decryption method type
 */
typedef void (*aesni_ecb_fn_t)(aesni_key_t*, u_int, u_char*, u_char*);

/**
 * Private data of an aesni_ecb_t object.
 */
struct private_aesni_ecb_t {

	/**
	 * Public aesni_ecb_t interface.
	 */
	aesni_ecb_t public;

	/**
	 * Key size
	 */
	u_int key_size;

	/**
	 * Encryption key schedule
	 */
	aesni_key_t *ekey;

	/**
	 * Decryption key schedule
	 */
	aesni_key_t *dkey;

	/**
	 * Encryption method
	 */
	aesni_ecb_fn_t encrypt;

	/**
	 * Decryption method
	 */
	aesni_ecb_fn_t decrypt;
};

/**
 * AES-128 ECB encryption
 */
static void encrypt_ecb128(aesni_key_t *key, u_int blocks, u_char *in,
						   u_char *out)
{
	__m128i *ks, *bi, *bo;
	__m128i t1, t2, t3, t4;
	u_int i, pblocks;

	ks = key->schedule;
	bi = (__m128i*)in;
	bo = (__m128i*)out;
	pblocks = blocks - (blocks % ECB_PARALLELISM);

	for (i = 0; i < pblocks; i += ECB_PARALLELISM)
	{
		t1 = _mm_loadu_si128(bi + i + 0);
		t2 = _mm_loadu_si128(bi + i + 1);
		t3 = _mm_loadu_si128(bi + i + 2);
		t4 = _mm_loadu_si128(bi + i + 3);

		t1 = _mm_xor_si128(t1, ks[0]);
		t2 = _mm_xor_si128(t2, ks[0]);
		t3 = _mm_xor_si128(t3, ks[0]);
		t4 = _mm_xor_si128(t4, ks[0]);

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

		_mm_storeu_si128(bo + i + 0, t1);
		_mm_storeu_si128(bo + i + 1, t2);
		_mm_storeu_si128(bo + i + 2, t3);
		_mm_storeu_si128(bo + i + 3, t4);
	}

	for (i = pblocks; i < blocks; i++)
	{
		t1 = _mm_loadu_si128(bi + i);
		t1 = _mm_xor_si128(t1, ks[0]);

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
		_mm_storeu_si128(bo + i, t1);
	}
}

/**
 * AES-128 ECB decryption
 */
static void decrypt_ecb128(aesni_key_t *key, u_int blocks, u_char *in,
						   u_char *out)
{
	__m128i *ks, *bi, *bo;
	__m128i t1, t2, t3, t4;
	u_int i, pblocks;

	ks = key->schedule;
	bi = (__m128i*)in;
	bo = (__m128i*)out;
	pblocks = blocks - (blocks % ECB_PARALLELISM);

	for (i = 0; i < pblocks; i += ECB_PARALLELISM)
	{
		t1 = _mm_loadu_si128(bi + i + 0);
		t2 = _mm_loadu_si128(bi + i + 1);
		t3 = _mm_loadu_si128(bi + i + 2);
		t4 = _mm_loadu_si128(bi + i + 3);

		t1 = _mm_xor_si128(t1, ks[0]);
		t2 = _mm_xor_si128(t2, ks[0]);
		t3 = _mm_xor_si128(t3, ks[0]);
		t4 = _mm_xor_si128(t4, ks[0]);

		t1 = _mm_aesdec_si128(t1, ks[1]);
		t2 = _mm_aesdec_si128(t2, ks[1]);
		t3 = _mm_aesdec_si128(t3, ks[1]);
		t4 = _mm_aesdec_si128(t4, ks[1]);
		t1 = _mm_aesdec_si128(t1, ks[2]);
		t2 = _mm_aesdec_si128(t2, ks[2]);
		t3 = _mm_aesdec_si128(t3, ks[2]);
		t4 = _mm_aesdec_si128(t4, ks[2]);
		t1 = _mm_aesdec_si128(t1, ks[3]);
		t2 = _mm_aesdec_si128(t2, ks[3]);
		t3 = _mm_aesdec_si128(t3, ks[3]);
		t4 = _mm_aesdec_si128(t4, ks[3]);
		t1 = _mm_aesdec_si128(t1, ks[4]);
		t2 = _mm_aesdec_si128(t2, ks[4]);
		t3 = _mm_aesdec_si128(t3, ks[4]);
		t4 = _mm_aesdec_si128(t4, ks[4]);
		t1 = _mm_aesdec_si128(t1, ks[5]);
		t2 = _mm_aesdec_si128(t2, ks[5]);
		t3 = _mm_aesdec_si128(t3, ks[5]);
		t4 = _mm_aesdec_si128(t4, ks[5]);
		t1 = _mm_aesdec_si128(t1, ks[6]);
		t2 = _mm_aesdec_si128(t2, ks[6]);
		t3 = _mm_aesdec_si128(t3, ks[6]);
		t4 = _mm_aesdec_si128(t4, ks[6]);
		t1 = _mm_aesdec_si128(t1, ks[7]);
		t2 = _mm_aesdec_si128(t2, ks[7]);
		t3 = _mm_aesdec_si128(t3, ks[7]);
		t4 = _mm_aesdec_si128(t4, ks[7]);
		t1 = _mm_aesdec_si128(t1, ks[8]);
		t2 = _mm_aesdec_si128(t2, ks[8]);
		t3 = _mm_aesdec_si128(t3, ks[8]);
		t4 = _mm_aesdec_si128(t4, ks[8]);
		t1 = _mm_aesdec_si128(t1, ks[9]);
		t2 = _mm_aesdec_si128(t2, ks[9]);
		t3 = _mm_aesdec_si128(t3, ks[9]);
		t4 = _mm_aesdec_si128(t4, ks[9]);

		t1 = _mm_aesdeclast_si128(t1, ks[10]);
		t2 = _mm_aesdeclast_si128(t2, ks[10]);
		t3 = _mm_aesdeclast_si128(t3, ks[10]);
		t4 = _mm_aesdeclast_si128(t4, ks[10]);

		_mm_storeu_si128(bo + i + 0, t1);
		_mm_storeu_si128(bo + i + 1, t2);
		_mm_storeu_si128(bo + i + 2, t3);
		_mm_storeu_si128(bo + i + 3, t4);
	}

	for (i = pblocks; i < blocks; i++)
	{
		t1 = _mm_loadu_si128(bi + i);
		t1 = _mm_xor_si128(t1, ks[0]);

		t1 = _mm_aesdec_si128(t1, ks[1]);
		t1 = _mm_aesdec_si128(t1, ks[2]);
		t1 = _mm_aesdec_si128(t1, ks[3]);
		t1 = _mm_aesdec_si128(t1, ks[4]);
		t1 = _mm_aesdec_si128(t1, ks[5]);
		t1 = _mm_aesdec_si128(t1, ks[6]);
		t1 = _mm_aesdec_si128(t1, ks[7]);
		t1 = _mm_aesdec_si128(t1, ks[8]);
		t1 = _mm_aesdec_si128(t1, ks[9]);

		t1 = _mm_aesdeclast_si128(t1, ks[10]);
		_mm_storeu_si128(bo + i, t1);
	}
}

/**
 * AES-192 ECB encryption
 */
static void encrypt_ecb192(aesni_key_t *key, u_int blocks, u_char *in,
						   u_char *out)
{
	__m128i *ks, *bi, *bo;
	__m128i t1, t2, t3, t4;
	u_int i, pblocks;

	ks = key->schedule;
	bi = (__m128i*)in;
	bo = (__m128i*)out;
	pblocks = blocks - (blocks % ECB_PARALLELISM);

	for (i = 0; i < pblocks; i += ECB_PARALLELISM)
	{
		t1 = _mm_loadu_si128(bi + i + 0);
		t2 = _mm_loadu_si128(bi + i + 1);
		t3 = _mm_loadu_si128(bi + i + 2);
		t4 = _mm_loadu_si128(bi + i + 3);

		t1 = _mm_xor_si128(t1, ks[0]);
		t2 = _mm_xor_si128(t2, ks[0]);
		t3 = _mm_xor_si128(t3, ks[0]);
		t4 = _mm_xor_si128(t4, ks[0]);

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

		_mm_storeu_si128(bo + i + 0, t1);
		_mm_storeu_si128(bo + i + 1, t2);
		_mm_storeu_si128(bo + i + 2, t3);
		_mm_storeu_si128(bo + i + 3, t4);
	}

	for (i = pblocks; i < blocks; i++)
	{
		t1 = _mm_loadu_si128(bi + i);
		t1 = _mm_xor_si128(t1, ks[0]);

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
		_mm_storeu_si128(bo + i, t1);
	}
}

/**
 * AES-192 ECB decryption
 */
static void decrypt_ecb192(aesni_key_t *key, u_int blocks, u_char *in,
						   u_char *out)
{
	__m128i *ks, *bi, *bo;
	__m128i t1, t2, t3, t4;
	u_int i, pblocks;

	ks = key->schedule;
	bi = (__m128i*)in;
	bo = (__m128i*)out;
	pblocks = blocks - (blocks % ECB_PARALLELISM);

	for (i = 0; i < pblocks; i += ECB_PARALLELISM)
	{
		t1 = _mm_loadu_si128(bi + i + 0);
		t2 = _mm_loadu_si128(bi + i + 1);
		t3 = _mm_loadu_si128(bi + i + 2);
		t4 = _mm_loadu_si128(bi + i + 3);

		t1 = _mm_xor_si128(t1, ks[0]);
		t2 = _mm_xor_si128(t2, ks[0]);
		t3 = _mm_xor_si128(t3, ks[0]);
		t4 = _mm_xor_si128(t4, ks[0]);

		t1 = _mm_aesdec_si128(t1, ks[1]);
		t2 = _mm_aesdec_si128(t2, ks[1]);
		t3 = _mm_aesdec_si128(t3, ks[1]);
		t4 = _mm_aesdec_si128(t4, ks[1]);
		t1 = _mm_aesdec_si128(t1, ks[2]);
		t2 = _mm_aesdec_si128(t2, ks[2]);
		t3 = _mm_aesdec_si128(t3, ks[2]);
		t4 = _mm_aesdec_si128(t4, ks[2]);
		t1 = _mm_aesdec_si128(t1, ks[3]);
		t2 = _mm_aesdec_si128(t2, ks[3]);
		t3 = _mm_aesdec_si128(t3, ks[3]);
		t4 = _mm_aesdec_si128(t4, ks[3]);
		t1 = _mm_aesdec_si128(t1, ks[4]);
		t2 = _mm_aesdec_si128(t2, ks[4]);
		t3 = _mm_aesdec_si128(t3, ks[4]);
		t4 = _mm_aesdec_si128(t4, ks[4]);
		t1 = _mm_aesdec_si128(t1, ks[5]);
		t2 = _mm_aesdec_si128(t2, ks[5]);
		t3 = _mm_aesdec_si128(t3, ks[5]);
		t4 = _mm_aesdec_si128(t4, ks[5]);
		t1 = _mm_aesdec_si128(t1, ks[6]);
		t2 = _mm_aesdec_si128(t2, ks[6]);
		t3 = _mm_aesdec_si128(t3, ks[6]);
		t4 = _mm_aesdec_si128(t4, ks[6]);
		t1 = _mm_aesdec_si128(t1, ks[7]);
		t2 = _mm_aesdec_si128(t2, ks[7]);
		t3 = _mm_aesdec_si128(t3, ks[7]);
		t4 = _mm_aesdec_si128(t4, ks[7]);
		t1 = _mm_aesdec_si128(t1, ks[8]);
		t2 = _mm_aesdec_si128(t2, ks[8]);
		t3 = _mm_aesdec_si128(t3, ks[8]);
		t4 = _mm_aesdec_si128(t4, ks[8]);
		t1 = _mm_aesdec_si128(t1, ks[9]);
		t2 = _mm_aesdec_si128(t2, ks[9]);
		t3 = _mm_aesdec_si128(t3, ks[9]);
		t4 = _mm_aesdec_si128(t4, ks[9]);
		t1 = _mm_aesdec_si128(t1, ks[10]);
		t2 = _mm_aesdec_si128(t2, ks[10]);
		t3 = _mm_aesdec_si128(t3, ks[10]);
		t4 = _mm_aesdec_si128(t4, ks[10]);
		t1 = _mm_aesdec_si128(t1, ks[11]);
		t2 = _mm_aesdec_si128(t2, ks[11]);
		t3 = _mm_aesdec_si128(t3, ks[11]);
		t4 = _mm_aesdec_si128(t4, ks[11]);

		t1 = _mm_aesdeclast_si128(t1, ks[12]);
		t2 = _mm_aesdeclast_si128(t2, ks[12]);
		t3 = _mm_aesdeclast_si128(t3, ks[12]);
		t4 = _mm_aesdeclast_si128(t4, ks[12]);

		_mm_storeu_si128(bo + i + 0, t1);
		_mm_storeu_si128(bo + i + 1, t2);
		_mm_storeu_si128(bo + i + 2, t3);
		_mm_storeu_si128(bo + i + 3, t4);
	}

	for (i = pblocks; i < blocks; i++)
	{
		t1 = _mm_loadu_si128(bi + i);
		t1 = _mm_xor_si128(t1, ks[0]);

		t1 = _mm_aesdec_si128(t1, ks[1]);
		t1 = _mm_aesdec_si128(t1, ks[2]);
		t1 = _mm_aesdec_si128(t1, ks[3]);
		t1 = _mm_aesdec_si128(t1, ks[4]);
		t1 = _mm_aesdec_si128(t1, ks[5]);
		t1 = _mm_aesdec_si128(t1, ks[6]);
		t1 = _mm_aesdec_si128(t1, ks[7]);
		t1 = _mm_aesdec_si128(t1, ks[8]);
		t1 = _mm_aesdec_si128(t1, ks[9]);
		t1 = _mm_aesdec_si128(t1, ks[10]);
		t1 = _mm_aesdec_si128(t1, ks[11]);

		t1 = _mm_aesdeclast_si128(t1, ks[12]);
		_mm_storeu_si128(bo + i, t1);
	}
}

/**
 * AES-256 ECB encryption
 */
static void encrypt_ecb256(aesni_key_t *key, u_int blocks, u_char *in,
						   u_char *out)
{
	__m128i *ks, *bi, *bo;
	__m128i t1, t2, t3, t4;
	u_int i, pblocks;

	ks = key->schedule;
	bi = (__m128i*)in;
	bo = (__m128i*)out;
	pblocks = blocks - (blocks % ECB_PARALLELISM);

	for (i = 0; i < pblocks; i += ECB_PARALLELISM)
	{
		t1 = _mm_loadu_si128(bi + i + 0);
		t2 = _mm_loadu_si128(bi + i + 1);
		t3 = _mm_loadu_si128(bi + i + 2);
		t4 = _mm_loadu_si128(bi + i + 3);

		t1 = _mm_xor_si128(t1, ks[0]);
		t2 = _mm_xor_si128(t2, ks[0]);
		t3 = _mm_xor_si128(t3, ks[0]);
		t4 = _mm_xor_si128(t4, ks[0]);

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

		_mm_storeu_si128(bo + i + 0, t1);
		_mm_storeu_si128(bo + i + 1, t2);
		_mm_storeu_si128(bo + i + 2, t3);
		_mm_storeu_si128(bo + i + 3, t4);
	}

	for (i = pblocks; i < blocks; i++)
	{
		t1 = _mm_loadu_si128(bi + i);
		t1 = _mm_xor_si128(t1, ks[0]);

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
		_mm_storeu_si128(bo + i, t1);
	}
}

/**
 * AES-256 ECB decryption
 */
static void decrypt_ecb256(aesni_key_t *key, u_int blocks, u_char *in,
						   u_char *out)
{
	__m128i *ks, *bi, *bo;
	__m128i t1, t2, t3, t4;
	u_int i, pblocks;

	ks = key->schedule;
	bi = (__m128i*)in;
	bo = (__m128i*)out;
	pblocks = blocks - (blocks % ECB_PARALLELISM);

	for (i = 0; i < pblocks; i += ECB_PARALLELISM)
	{
		t1 = _mm_loadu_si128(bi + i + 0);
		t2 = _mm_loadu_si128(bi + i + 1);
		t3 = _mm_loadu_si128(bi + i + 2);
		t4 = _mm_loadu_si128(bi + i + 3);

		t1 = _mm_xor_si128(t1, ks[0]);
		t2 = _mm_xor_si128(t2, ks[0]);
		t3 = _mm_xor_si128(t3, ks[0]);
		t4 = _mm_xor_si128(t4, ks[0]);

		t1 = _mm_aesdec_si128(t1, ks[1]);
		t2 = _mm_aesdec_si128(t2, ks[1]);
		t3 = _mm_aesdec_si128(t3, ks[1]);
		t4 = _mm_aesdec_si128(t4, ks[1]);
		t1 = _mm_aesdec_si128(t1, ks[2]);
		t2 = _mm_aesdec_si128(t2, ks[2]);
		t3 = _mm_aesdec_si128(t3, ks[2]);
		t4 = _mm_aesdec_si128(t4, ks[2]);
		t1 = _mm_aesdec_si128(t1, ks[3]);
		t2 = _mm_aesdec_si128(t2, ks[3]);
		t3 = _mm_aesdec_si128(t3, ks[3]);
		t4 = _mm_aesdec_si128(t4, ks[3]);
		t1 = _mm_aesdec_si128(t1, ks[4]);
		t2 = _mm_aesdec_si128(t2, ks[4]);
		t3 = _mm_aesdec_si128(t3, ks[4]);
		t4 = _mm_aesdec_si128(t4, ks[4]);
		t1 = _mm_aesdec_si128(t1, ks[5]);
		t2 = _mm_aesdec_si128(t2, ks[5]);
		t3 = _mm_aesdec_si128(t3, ks[5]);
		t4 = _mm_aesdec_si128(t4, ks[5]);
		t1 = _mm_aesdec_si128(t1, ks[6]);
		t2 = _mm_aesdec_si128(t2, ks[6]);
		t3 = _mm_aesdec_si128(t3, ks[6]);
		t4 = _mm_aesdec_si128(t4, ks[6]);
		t1 = _mm_aesdec_si128(t1, ks[7]);
		t2 = _mm_aesdec_si128(t2, ks[7]);
		t3 = _mm_aesdec_si128(t3, ks[7]);
		t4 = _mm_aesdec_si128(t4, ks[7]);
		t1 = _mm_aesdec_si128(t1, ks[8]);
		t2 = _mm_aesdec_si128(t2, ks[8]);
		t3 = _mm_aesdec_si128(t3, ks[8]);
		t4 = _mm_aesdec_si128(t4, ks[8]);
		t1 = _mm_aesdec_si128(t1, ks[9]);
		t2 = _mm_aesdec_si128(t2, ks[9]);
		t3 = _mm_aesdec_si128(t3, ks[9]);
		t4 = _mm_aesdec_si128(t4, ks[9]);
		t1 = _mm_aesdec_si128(t1, ks[10]);
		t2 = _mm_aesdec_si128(t2, ks[10]);
		t3 = _mm_aesdec_si128(t3, ks[10]);
		t4 = _mm_aesdec_si128(t4, ks[10]);
		t1 = _mm_aesdec_si128(t1, ks[11]);
		t2 = _mm_aesdec_si128(t2, ks[11]);
		t3 = _mm_aesdec_si128(t3, ks[11]);
		t4 = _mm_aesdec_si128(t4, ks[11]);
		t1 = _mm_aesdec_si128(t1, ks[12]);
		t2 = _mm_aesdec_si128(t2, ks[12]);
		t3 = _mm_aesdec_si128(t3, ks[12]);
		t4 = _mm_aesdec_si128(t4, ks[12]);
		t1 = _mm_aesdec_si128(t1, ks[13]);
		t2 = _mm_aesdec_si128(t2, ks[13]);
		t3 = _mm_aesdec_si128(t3, ks[13]);
		t4 = _mm_aesdec_si128(t4, ks[13]);

		t1 = _mm_aesdeclast_si128(t1, ks[14]);
		t2 = _mm_aesdeclast_si128(t2, ks[14]);
		t3 = _mm_aesdeclast_si128(t3, ks[14]);
		t4 = _mm_aesdeclast_si128(t4, ks[14]);

		_mm_storeu_si128(bo + i + 0, t1);
		_mm_storeu_si128(bo + i + 1, t2);
		_mm_storeu_si128(bo + i + 2, t3);
		_mm_storeu_si128(bo + i + 3, t4);
	}

	for (i = pblocks; i < blocks; i++)
	{
		t1 = _mm_loadu_si128(bi + i);
		t1 = _mm_xor_si128(t1, ks[0]);

		t1 = _mm_aesdec_si128(t1, ks[1]);
		t1 = _mm_aesdec_si128(t1, ks[2]);
		t1 = _mm_aesdec_si128(t1, ks[3]);
		t1 = _mm_aesdec_si128(t1, ks[4]);
		t1 = _mm_aesdec_si128(t1, ks[5]);
		t1 = _mm_aesdec_si128(t1, ks[6]);
		t1 = _mm_aesdec_si128(t1, ks[7]);
		t1 = _mm_aesdec_si128(t1, ks[8]);
		t1 = _mm_aesdec_si128(t1, ks[9]);
		t1 = _mm_aesdec_si128(t1, ks[10]);
		t1 = _mm_aesdec_si128(t1, ks[11]);
		t1 = _mm_aesdec_si128(t1, ks[12]);
		t1 = _mm_aesdec_si128(t1, ks[13]);

		t1 = _mm_aesdeclast_si128(t1, ks[14]);
		_mm_storeu_si128(bo + i, t1);
	}
}

/**
 * Do inline or allocated de/encryption using key schedule
 */
static bool crypt(aesni_ecb_fn_t fn, aesni_key_t *key, chunk_t data,
				  chunk_t *out)
{
	u_char *buf;

	if (!key || data.len % AES_BLOCK_SIZE)
	{
		return FALSE;
	}
	if (out)
	{
		*out = chunk_alloc(data.len);
		buf = out->ptr;
	}
	else
	{
		buf = data.ptr;
	}
	fn(key, data.len / AES_BLOCK_SIZE, data.ptr, buf);
	return TRUE;
}

METHOD(crypter_t, encrypt, bool,
	private_aesni_ecb_t *this, chunk_t data, chunk_t iv, chunk_t *encrypted)
{
	return crypt(this->encrypt, this->ekey, data, encrypted);
}

METHOD(crypter_t, decrypt, bool,
	private_aesni_ecb_t *this, chunk_t data, chunk_t iv, chunk_t *decrypted)
{
	return crypt(this->decrypt, this->dkey, data, decrypted);
}

METHOD(crypter_t, get_block_size, size_t,
	private_aesni_ecb_t *this)
{
	return AES_BLOCK_SIZE;
}

METHOD(crypter_t, get_iv_size, size_t,
	private_aesni_ecb_t *this)
{
	return 0;
}

METHOD(crypter_t, get_key_size, size_t,
	private_aesni_ecb_t *this)
{
	return this->key_size;
}

METHOD(crypter_t, set_key, bool,
	private_aesni_ecb_t *this, chunk_t key)
{
	if (key.len != this->key_size)
	{
		return FALSE;
	}

	DESTROY_IF(this->ekey);
	DESTROY_IF(this->dkey);

	this->ekey = aesni_key_create(TRUE, key);
	this->dkey = aesni_key_create(FALSE, key);

	return this->ekey && this->dkey;
}

METHOD(crypter_t, destroy, void,
	private_aesni_ecb_t *this)
{
	DESTROY_IF(this->ekey);
	DESTROY_IF(this->dkey);
	free_align(this);
}

/**
 * See header
 */
aesni_ecb_t *aesni_ecb_create(encryption_algorithm_t algo, size_t key_size)
{
	private_aesni_ecb_t *this;

	if (algo != ENCR_AES_ECB)
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
				.encrypt = _encrypt,
				.decrypt = _decrypt,
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
			this->encrypt = encrypt_ecb128;
			this->decrypt = decrypt_ecb128;
			break;
		case 24:
			this->encrypt = encrypt_ecb192;
			this->decrypt = decrypt_ecb192;
			break;
		case 32:
			this->encrypt = encrypt_ecb256;
			this->decrypt = decrypt_ecb256;
			break;
	}

	return &this->public;
}
