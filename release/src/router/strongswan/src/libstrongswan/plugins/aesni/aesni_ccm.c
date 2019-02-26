/*
 * Copyright (C) 2010-2015 Martin Willi
 * Copyright (C) 2010-2015 revosec AG
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

#include "aesni_ccm.h"
#include "aesni_key.h"

#include <crypto/iv/iv_gen_seq.h>

#include <tmmintrin.h>

#define SALT_SIZE 3
#define IV_SIZE 8
#define NONCE_SIZE (SALT_SIZE + IV_SIZE) /* 11 */
#define Q_SIZE (AES_BLOCK_SIZE - NONCE_SIZE - 1) /* 4 */

typedef struct private_aesni_ccm_t private_aesni_ccm_t;

/**
 * CCM en/decryption method type
 */
typedef void (*aesni_ccm_fn_t)(private_aesni_ccm_t*, size_t, u_char*, u_char*,
							   u_char*, size_t, u_char*, u_char*);

/**
 * Private data of an aesni_ccm_t object.
 */
struct private_aesni_ccm_t {

	/**
	 * Public aesni_ccm_t interface.
	 */
	aesni_ccm_t public;

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
	 * CCM encryption function
	 */
	aesni_ccm_fn_t encrypt;

	/**
	 * CCM decryption function
	 */
	aesni_ccm_fn_t decrypt;

	/**
	 * salt to add to nonce
	 */
	u_char salt[SALT_SIZE];
};

/**
 * First block with control information
 */
typedef struct __attribute__((packed)) {
	BITFIELD4(uint8_t,
		/* size of p length field q, as q-1 */
		q_len: 3,
		/* size of our ICV t, as (t-2)/2 */
		t_len: 3,
		/* do we have associated data */
		assoc: 1,
		reserved: 1,
	) flags;
	/* nonce value */
	struct __attribute__((packed)) {
		u_char salt[SALT_SIZE];
		u_char iv[IV_SIZE];
	} nonce;
	/* length of plain text, q */
	u_char q[Q_SIZE];
} b0_t;

/**
 * Counter block
 */
typedef struct __attribute__((packed)) {
	BITFIELD3(uint8_t,
		/* size of p length field q, as q-1 */
		q_len: 3,
		zero: 3,
		reserved: 2,
	) flags;
	/* nonce value */
	struct __attribute__((packed)) {
		u_char salt[SALT_SIZE];
		u_char iv[IV_SIZE];
	} nonce;
	/* counter value */
	u_char i[Q_SIZE];
} ctr_t;

/**
 * Build the first block B0
 */
static void build_b0(private_aesni_ccm_t *this, size_t len, size_t alen,
					 u_char *iv, void *out)
{
	b0_t *block = out;

	block->flags.reserved = 0;
	block->flags.assoc = alen ? 1 : 0;
	block->flags.t_len = (this->icv_size - 2) / 2;
	block->flags.q_len = Q_SIZE - 1;
	memcpy(block->nonce.salt, this->salt, SALT_SIZE);
	memcpy(block->nonce.iv, iv, IV_SIZE);
	htoun32(block->q, len);
}

/**
 * Build a counter block for counter i
 */
static void build_ctr(private_aesni_ccm_t *this, uint32_t i, u_char *iv,
					  void *out)
{
	ctr_t *ctr = out;

	ctr->flags.reserved = 0;
	ctr->flags.zero = 0;
	ctr->flags.q_len = Q_SIZE - 1;
	memcpy(ctr->nonce.salt, this->salt, SALT_SIZE);
	memcpy(ctr->nonce.iv, iv, IV_SIZE);
	htoun32(ctr->i, i);
}

/**
 * Calculate the ICV for the b0 and associated data
 */
static __m128i icv_header(private_aesni_ccm_t *this, size_t len, u_char *iv,
						  uint16_t alen, u_char *assoc)
{
	__m128i *ks, b, t, c;
	u_int i, round, blocks, rem;

	ks = this->key->schedule;
	build_b0(this, len, alen, iv, &b);
	c = _mm_loadu_si128(&b);
	c = _mm_xor_si128(c, ks[0]);
	for (round = 1; round < this->key->rounds; round++)
	{
		c = _mm_aesenc_si128(c, ks[round]);
	}
	c = _mm_aesenclast_si128(c, ks[this->key->rounds]);

	if (alen)
	{
		blocks = (alen + sizeof(alen)) / AES_BLOCK_SIZE;
		rem = (alen + sizeof(alen)) % AES_BLOCK_SIZE;
		if (rem)
		{
			blocks++;
		}
		for (i = 0; i < blocks; i++)
		{
			if (i == 0)
			{	/* first block */
				memset(&b, 0, sizeof(b));
				htoun16(&b, alen);
				memcpy(((u_char*)&b) + sizeof(alen), assoc,
					   min(alen, sizeof(b) - sizeof(alen)));
				t = _mm_loadu_si128(&b);
			}
			else if (i == blocks - 1 && rem)
			{	/* last block with padding */
				memset(&b, 0, sizeof(b));
				memcpy(&b, ((__m128i*)(assoc - sizeof(alen))) + i, rem);
				t = _mm_loadu_si128(&b);
			}
			else
			{	/* full block */
				t = _mm_loadu_si128(((__m128i*)(assoc - sizeof(alen))) + i);
			}
			c = _mm_xor_si128(t, c);
			c = _mm_xor_si128(c, ks[0]);
			for (round = 1; round < this->key->rounds; round++)
			{
				c = _mm_aesenc_si128(c, ks[round]);
			}
			c = _mm_aesenclast_si128(c, ks[this->key->rounds]);
		}
	}
	return c;
}

/**
 * En-/Decrypt the ICV, trim and store it
 */
static void crypt_icv(private_aesni_ccm_t *this, u_char *iv,
					  __m128i c, u_char *icv)
{
	__m128i *ks, b, t;
	u_int round;

	ks = this->key->schedule;
	build_ctr(this, 0, iv, &b);

	t = _mm_loadu_si128(&b);
	t = _mm_xor_si128(t, ks[0]);
	for (round = 1; round < this->key->rounds; round++)
	{
		t = _mm_aesenc_si128(t, ks[round]);
	}
	t = _mm_aesenclast_si128(t, ks[this->key->rounds]);

	t = _mm_xor_si128(t, c);

	_mm_storeu_si128(&b, t);
	memcpy(icv, &b, this->icv_size);
}

/**
 * Do big-endian increment on x
 */
static inline __m128i increment_be(__m128i x)
{
	__m128i swap;

	swap = _mm_set_epi8(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);

	x = _mm_shuffle_epi8(x, swap);
	x = _mm_add_epi64(x, _mm_set_epi32(0, 0, 0, 1));
	x = _mm_shuffle_epi8(x, swap);

	return x;
}

/**
 * Encrypt a remaining incomplete block
 */
static __m128i encrypt_ccm_rem(aesni_key_t *key, u_int rem, __m128i state,
							   void *in, void *out, __m128i c)
{
	__m128i *ks, t, b, d;
	u_int round;

	ks = key->schedule;
	memset(&b, 0, sizeof(b));
	memcpy(&b, in, rem);
	d = _mm_loadu_si128(&b);

	c = _mm_xor_si128(d, c);
	c = _mm_xor_si128(c, ks[0]);
	t = _mm_xor_si128(state, ks[0]);
	for (round = 1; round < key->rounds; round++)
	{
		c = _mm_aesenc_si128(c, ks[round]);
		t = _mm_aesenc_si128(t, ks[round]);
	}
	c = _mm_aesenclast_si128(c, ks[key->rounds]);
	t = _mm_aesenclast_si128(t, ks[key->rounds]);

	t = _mm_xor_si128(t, d);
	_mm_storeu_si128(&b, t);

	memcpy(out, &b, rem);

	return c;
}

/**
 * Decrypt a remaining incomplete block
 */
static __m128i decrypt_ccm_rem(aesni_key_t *key, u_int rem, __m128i state,
							   void *in, void *out, __m128i c)
{
	__m128i *ks, t, b, d;
	u_int round;

	ks = key->schedule;
	memset(&b, 0, sizeof(b));
	memcpy(&b, in, rem);
	d = _mm_loadu_si128(&b);

	t = _mm_xor_si128(state, ks[0]);
	for (round = 1; round < key->rounds; round++)
	{
		t = _mm_aesenc_si128(t, ks[round]);
	}
	t = _mm_aesenclast_si128(t, ks[key->rounds]);
	t = _mm_xor_si128(t, d);
	_mm_storeu_si128(&b, t);

	memset((u_char*)&b + rem, 0, sizeof(b) - rem);
	t = _mm_loadu_si128(&b);
	c = _mm_xor_si128(t, c);
	c = _mm_xor_si128(c, ks[0]);
	for (round = 1; round < key->rounds; round++)
	{
		c = _mm_aesenc_si128(c, ks[round]);
	}
	c = _mm_aesenclast_si128(c, ks[key->rounds]);

	memcpy(out, &b, rem);

	return c;
}

/**
 * AES-128 CCM encryption/ICV generation
 */
static void encrypt_ccm128(private_aesni_ccm_t *this,
						   size_t len, u_char *in, u_char *out, u_char *iv,
						   size_t alen, u_char *assoc, u_char *icv)
{
	__m128i *ks, d, t, c, b, state, *bi, *bo;
	u_int blocks, rem, i;

	c = icv_header(this, len, iv, alen, assoc);
	build_ctr(this, 1, iv, &b);
	state = _mm_load_si128(&b);
	blocks = len / AES_BLOCK_SIZE;
	rem = len % AES_BLOCK_SIZE;
	bi = (__m128i*)in;
	bo = (__m128i*)out;

	ks = this->key->schedule;

	for (i = 0; i < blocks; i++)
	{
		d = _mm_loadu_si128(bi + i);

		c = _mm_xor_si128(d, c);
		c = _mm_xor_si128(c, ks[0]);
		t = _mm_xor_si128(state, ks[0]);

		c = _mm_aesenc_si128(c, ks[1]);
		t = _mm_aesenc_si128(t, ks[1]);
		c = _mm_aesenc_si128(c, ks[2]);
		t = _mm_aesenc_si128(t, ks[2]);
		c = _mm_aesenc_si128(c, ks[3]);
		t = _mm_aesenc_si128(t, ks[3]);
		c = _mm_aesenc_si128(c, ks[4]);
		t = _mm_aesenc_si128(t, ks[4]);
		c = _mm_aesenc_si128(c, ks[5]);
		t = _mm_aesenc_si128(t, ks[5]);
		c = _mm_aesenc_si128(c, ks[6]);
		t = _mm_aesenc_si128(t, ks[6]);
		c = _mm_aesenc_si128(c, ks[7]);
		t = _mm_aesenc_si128(t, ks[7]);
		c = _mm_aesenc_si128(c, ks[8]);
		t = _mm_aesenc_si128(t, ks[8]);
		c = _mm_aesenc_si128(c, ks[9]);
		t = _mm_aesenc_si128(t, ks[9]);

		c = _mm_aesenclast_si128(c, ks[10]);
		t = _mm_aesenclast_si128(t, ks[10]);

		t = _mm_xor_si128(t, d);
		_mm_storeu_si128(bo + i, t);

		state = increment_be(state);
	}

	if (rem)
	{
		c = encrypt_ccm_rem(this->key, rem, state, bi + blocks, bo + blocks, c);
	}
	crypt_icv(this, iv, c, icv);
}

/**
 * AES-128 CCM decryption/ICV generation
 */
static void decrypt_ccm128(private_aesni_ccm_t *this,
						   size_t len, u_char *in, u_char *out, u_char *iv,
						   size_t alen, u_char *assoc, u_char *icv)
{
	__m128i *ks, d, t, c, b, state, *bi, *bo;
	u_int blocks, rem, i;

	c = icv_header(this, len, iv, alen, assoc);
	build_ctr(this, 1, iv, &b);
	state = _mm_load_si128(&b);
	blocks = len / AES_BLOCK_SIZE;
	rem = len % AES_BLOCK_SIZE;
	bi = (__m128i*)in;
	bo = (__m128i*)out;

	ks = this->key->schedule;

	for (i = 0; i < blocks; i++)
	{
		d = _mm_loadu_si128(bi + i);

		t = _mm_xor_si128(state, ks[0]);

		t = _mm_aesenc_si128(t, ks[1]);
		t = _mm_aesenc_si128(t, ks[2]);
		t = _mm_aesenc_si128(t, ks[3]);
		t = _mm_aesenc_si128(t, ks[4]);
		t = _mm_aesenc_si128(t, ks[5]);
		t = _mm_aesenc_si128(t, ks[6]);
		t = _mm_aesenc_si128(t, ks[7]);
		t = _mm_aesenc_si128(t, ks[8]);
		t = _mm_aesenc_si128(t, ks[9]);

		t = _mm_aesenclast_si128(t, ks[10]);
		t = _mm_xor_si128(t, d);
		_mm_storeu_si128(bo + i, t);

		c = _mm_xor_si128(t, c);
		c = _mm_xor_si128(c, ks[0]);

		c = _mm_aesenc_si128(c, ks[1]);
		c = _mm_aesenc_si128(c, ks[2]);
		c = _mm_aesenc_si128(c, ks[3]);
		c = _mm_aesenc_si128(c, ks[4]);
		c = _mm_aesenc_si128(c, ks[5]);
		c = _mm_aesenc_si128(c, ks[6]);
		c = _mm_aesenc_si128(c, ks[7]);
		c = _mm_aesenc_si128(c, ks[8]);
		c = _mm_aesenc_si128(c, ks[9]);

		c = _mm_aesenclast_si128(c, ks[10]);

		state = increment_be(state);
	}

	if (rem)
	{
		c = decrypt_ccm_rem(this->key, rem, state, bi + blocks, bo + blocks, c);
	}
	crypt_icv(this, iv, c, icv);
}

/**
 * AES-192 CCM encryption/ICV generation
 */
static void encrypt_ccm192(private_aesni_ccm_t *this,
						   size_t len, u_char *in, u_char *out, u_char *iv,
						   size_t alen, u_char *assoc, u_char *icv)
{
	__m128i *ks, d, t, c, b, state, *bi, *bo;
	u_int blocks, rem, i;

	c = icv_header(this, len, iv, alen, assoc);
	build_ctr(this, 1, iv, &b);
	state = _mm_load_si128(&b);
	blocks = len / AES_BLOCK_SIZE;
	rem = len % AES_BLOCK_SIZE;
	bi = (__m128i*)in;
	bo = (__m128i*)out;

	ks = this->key->schedule;

	for (i = 0; i < blocks; i++)
	{
		d = _mm_loadu_si128(bi + i);

		c = _mm_xor_si128(d, c);
		c = _mm_xor_si128(c, ks[0]);
		t = _mm_xor_si128(state, ks[0]);

		c = _mm_aesenc_si128(c, ks[1]);
		t = _mm_aesenc_si128(t, ks[1]);
		c = _mm_aesenc_si128(c, ks[2]);
		t = _mm_aesenc_si128(t, ks[2]);
		c = _mm_aesenc_si128(c, ks[3]);
		t = _mm_aesenc_si128(t, ks[3]);
		c = _mm_aesenc_si128(c, ks[4]);
		t = _mm_aesenc_si128(t, ks[4]);
		c = _mm_aesenc_si128(c, ks[5]);
		t = _mm_aesenc_si128(t, ks[5]);
		c = _mm_aesenc_si128(c, ks[6]);
		t = _mm_aesenc_si128(t, ks[6]);
		c = _mm_aesenc_si128(c, ks[7]);
		t = _mm_aesenc_si128(t, ks[7]);
		c = _mm_aesenc_si128(c, ks[8]);
		t = _mm_aesenc_si128(t, ks[8]);
		c = _mm_aesenc_si128(c, ks[9]);
		t = _mm_aesenc_si128(t, ks[9]);
		c = _mm_aesenc_si128(c, ks[10]);
		t = _mm_aesenc_si128(t, ks[10]);
		c = _mm_aesenc_si128(c, ks[11]);
		t = _mm_aesenc_si128(t, ks[11]);

		c = _mm_aesenclast_si128(c, ks[12]);
		t = _mm_aesenclast_si128(t, ks[12]);

		t = _mm_xor_si128(t, d);
		_mm_storeu_si128(bo + i, t);

		state = increment_be(state);
	}

	if (rem)
	{
		c = encrypt_ccm_rem(this->key, rem, state, bi + blocks, bo + blocks, c);
	}
	crypt_icv(this, iv, c, icv);
}

/**
 * AES-192 CCM decryption/ICV generation
 */
static void decrypt_ccm192(private_aesni_ccm_t *this,
						   size_t len, u_char *in, u_char *out, u_char *iv,
						   size_t alen, u_char *assoc, u_char *icv)
{
	__m128i *ks, d, t, c, b, state, *bi, *bo;
	u_int blocks, rem, i;

	c = icv_header(this, len, iv, alen, assoc);
	build_ctr(this, 1, iv, &b);
	state = _mm_load_si128(&b);
	blocks = len / AES_BLOCK_SIZE;
	rem = len % AES_BLOCK_SIZE;
	bi = (__m128i*)in;
	bo = (__m128i*)out;

	ks = this->key->schedule;

	for (i = 0; i < blocks; i++)
	{
		d = _mm_loadu_si128(bi + i);

		t = _mm_xor_si128(state, ks[0]);

		t = _mm_aesenc_si128(t, ks[1]);
		t = _mm_aesenc_si128(t, ks[2]);
		t = _mm_aesenc_si128(t, ks[3]);
		t = _mm_aesenc_si128(t, ks[4]);
		t = _mm_aesenc_si128(t, ks[5]);
		t = _mm_aesenc_si128(t, ks[6]);
		t = _mm_aesenc_si128(t, ks[7]);
		t = _mm_aesenc_si128(t, ks[8]);
		t = _mm_aesenc_si128(t, ks[9]);
		t = _mm_aesenc_si128(t, ks[10]);
		t = _mm_aesenc_si128(t, ks[11]);

		t = _mm_aesenclast_si128(t, ks[12]);
		t = _mm_xor_si128(t, d);
		_mm_storeu_si128(bo + i, t);

		c = _mm_xor_si128(t, c);
		c = _mm_xor_si128(c, ks[0]);

		c = _mm_aesenc_si128(c, ks[1]);
		c = _mm_aesenc_si128(c, ks[2]);
		c = _mm_aesenc_si128(c, ks[3]);
		c = _mm_aesenc_si128(c, ks[4]);
		c = _mm_aesenc_si128(c, ks[5]);
		c = _mm_aesenc_si128(c, ks[6]);
		c = _mm_aesenc_si128(c, ks[7]);
		c = _mm_aesenc_si128(c, ks[8]);
		c = _mm_aesenc_si128(c, ks[9]);
		c = _mm_aesenc_si128(c, ks[10]);
		c = _mm_aesenc_si128(c, ks[11]);

		c = _mm_aesenclast_si128(c, ks[12]);

		state = increment_be(state);
	}

	if (rem)
	{
		c = decrypt_ccm_rem(this->key, rem, state, bi + blocks, bo + blocks, c);
	}
	crypt_icv(this, iv, c, icv);
}

/**
 * AES-256 CCM encryption/ICV generation
 */
static void encrypt_ccm256(private_aesni_ccm_t *this,
						   size_t len, u_char *in, u_char *out, u_char *iv,
						   size_t alen, u_char *assoc, u_char *icv)
{
	__m128i *ks, d, t, c, b, state, *bi, *bo;
	u_int blocks, rem, i;

	c = icv_header(this, len, iv, alen, assoc);
	build_ctr(this, 1, iv, &b);
	state = _mm_load_si128(&b);
	blocks = len / AES_BLOCK_SIZE;
	rem = len % AES_BLOCK_SIZE;
	bi = (__m128i*)in;
	bo = (__m128i*)out;

	ks = this->key->schedule;

	for (i = 0; i < blocks; i++)
	{
		d = _mm_loadu_si128(bi + i);

		c = _mm_xor_si128(d, c);
		c = _mm_xor_si128(c, ks[0]);
		t = _mm_xor_si128(state, ks[0]);

		c = _mm_aesenc_si128(c, ks[1]);
		t = _mm_aesenc_si128(t, ks[1]);
		c = _mm_aesenc_si128(c, ks[2]);
		t = _mm_aesenc_si128(t, ks[2]);
		c = _mm_aesenc_si128(c, ks[3]);
		t = _mm_aesenc_si128(t, ks[3]);
		c = _mm_aesenc_si128(c, ks[4]);
		t = _mm_aesenc_si128(t, ks[4]);
		c = _mm_aesenc_si128(c, ks[5]);
		t = _mm_aesenc_si128(t, ks[5]);
		c = _mm_aesenc_si128(c, ks[6]);
		t = _mm_aesenc_si128(t, ks[6]);
		c = _mm_aesenc_si128(c, ks[7]);
		t = _mm_aesenc_si128(t, ks[7]);
		c = _mm_aesenc_si128(c, ks[8]);
		t = _mm_aesenc_si128(t, ks[8]);
		c = _mm_aesenc_si128(c, ks[9]);
		t = _mm_aesenc_si128(t, ks[9]);
		c = _mm_aesenc_si128(c, ks[10]);
		t = _mm_aesenc_si128(t, ks[10]);
		c = _mm_aesenc_si128(c, ks[11]);
		t = _mm_aesenc_si128(t, ks[11]);
		c = _mm_aesenc_si128(c, ks[12]);
		t = _mm_aesenc_si128(t, ks[12]);
		c = _mm_aesenc_si128(c, ks[13]);
		t = _mm_aesenc_si128(t, ks[13]);

		c = _mm_aesenclast_si128(c, ks[14]);
		t = _mm_aesenclast_si128(t, ks[14]);

		t = _mm_xor_si128(t, d);
		_mm_storeu_si128(bo + i, t);

		state = increment_be(state);
	}

	if (rem)
	{
		c = encrypt_ccm_rem(this->key, rem, state, bi + blocks, bo + blocks, c);
	}
	crypt_icv(this, iv, c, icv);
}

/**
 * AES-256 CCM decryption/ICV generation
 */
static void decrypt_ccm256(private_aesni_ccm_t *this,
						   size_t len, u_char *in, u_char *out, u_char *iv,
						   size_t alen, u_char *assoc, u_char *icv)
{
	__m128i *ks, d, t, c, b, state, *bi, *bo;
	u_int blocks, rem, i;

	c = icv_header(this, len, iv, alen, assoc);
	build_ctr(this, 1, iv, &b);
	state = _mm_load_si128(&b);
	blocks = len / AES_BLOCK_SIZE;
	rem = len % AES_BLOCK_SIZE;
	bi = (__m128i*)in;
	bo = (__m128i*)out;

	ks = this->key->schedule;

	for (i = 0; i < blocks; i++)
	{
		d = _mm_loadu_si128(bi + i);

		t = _mm_xor_si128(state, ks[0]);

		t = _mm_aesenc_si128(t, ks[1]);
		t = _mm_aesenc_si128(t, ks[2]);
		t = _mm_aesenc_si128(t, ks[3]);
		t = _mm_aesenc_si128(t, ks[4]);
		t = _mm_aesenc_si128(t, ks[5]);
		t = _mm_aesenc_si128(t, ks[6]);
		t = _mm_aesenc_si128(t, ks[7]);
		t = _mm_aesenc_si128(t, ks[8]);
		t = _mm_aesenc_si128(t, ks[9]);
		t = _mm_aesenc_si128(t, ks[10]);
		t = _mm_aesenc_si128(t, ks[11]);
		t = _mm_aesenc_si128(t, ks[12]);
		t = _mm_aesenc_si128(t, ks[13]);

		t = _mm_aesenclast_si128(t, ks[14]);
		t = _mm_xor_si128(t, d);
		_mm_storeu_si128(bo + i, t);

		c = _mm_xor_si128(t, c);
		c = _mm_xor_si128(c, ks[0]);

		c = _mm_aesenc_si128(c, ks[1]);
		c = _mm_aesenc_si128(c, ks[2]);
		c = _mm_aesenc_si128(c, ks[3]);
		c = _mm_aesenc_si128(c, ks[4]);
		c = _mm_aesenc_si128(c, ks[5]);
		c = _mm_aesenc_si128(c, ks[6]);
		c = _mm_aesenc_si128(c, ks[7]);
		c = _mm_aesenc_si128(c, ks[8]);
		c = _mm_aesenc_si128(c, ks[9]);
		c = _mm_aesenc_si128(c, ks[10]);
		c = _mm_aesenc_si128(c, ks[11]);
		c = _mm_aesenc_si128(c, ks[12]);
		c = _mm_aesenc_si128(c, ks[13]);

		c = _mm_aesenclast_si128(c, ks[14]);

		state = increment_be(state);
	}

	if (rem)
	{
		c = decrypt_ccm_rem(this->key, rem, state, bi + blocks, bo + blocks, c);
	}
	crypt_icv(this, iv, c, icv);
}

METHOD(aead_t, encrypt, bool,
	private_aesni_ccm_t *this, chunk_t plain, chunk_t assoc, chunk_t iv,
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
	private_aesni_ccm_t *this, chunk_t encr, chunk_t assoc, chunk_t iv,
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
	private_aesni_ccm_t *this)
{
	return 1;
}

METHOD(aead_t, get_icv_size, size_t,
	private_aesni_ccm_t *this)
{
	return this->icv_size;
}

METHOD(aead_t, get_iv_size, size_t,
	private_aesni_ccm_t *this)
{
	return IV_SIZE;
}

METHOD(aead_t, get_iv_gen, iv_gen_t*,
	private_aesni_ccm_t *this)
{
	return this->iv_gen;
}

METHOD(aead_t, get_key_size, size_t,
	private_aesni_ccm_t *this)
{
	return this->key_size + SALT_SIZE;
}

METHOD(aead_t, set_key, bool,
	private_aesni_ccm_t *this, chunk_t key)
{
	if (key.len != this->key_size + SALT_SIZE)
	{
		return FALSE;
	}

	memcpy(this->salt, key.ptr + key.len - SALT_SIZE, SALT_SIZE);
	key.len -= SALT_SIZE;

	DESTROY_IF(this->key);
	this->key = aesni_key_create(TRUE, key);
	return TRUE;
}

METHOD(aead_t, destroy, void,
	private_aesni_ccm_t *this)
{
	DESTROY_IF(this->key);
	this->iv_gen->destroy(this->iv_gen);
	free_align(this);
}

/**
 * See header
 */
aesni_ccm_t *aesni_ccm_create(encryption_algorithm_t algo,
							  size_t key_size, size_t salt_size)
{
	private_aesni_ccm_t *this;
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
		case ENCR_AES_CCM_ICV8:
			algo = ENCR_AES_CBC;
			icv_size = 8;
			break;
		case ENCR_AES_CCM_ICV12:
			algo = ENCR_AES_CBC;
			icv_size = 12;
			break;
		case ENCR_AES_CCM_ICV16:
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
			this->encrypt = encrypt_ccm128;
			this->decrypt = decrypt_ccm128;
			break;
		case 24:
			this->encrypt = encrypt_ccm192;
			this->decrypt = decrypt_ccm192;
			break;
		case 32:
			this->encrypt = encrypt_ccm256;
			this->decrypt = decrypt_ccm256;
			break;
	}

	return &this->public;
}
