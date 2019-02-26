/*
 * Copyright (C) 2012 Tobias Brunner
 * HSR Hochschule fuer Technik Rapperswil
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

#include "aesni_cmac.h"
#include "aesni_key.h"

#include <crypto/prfs/mac_prf.h>
#include <crypto/signers/mac_signer.h>

typedef struct private_mac_t private_mac_t;

/**
 * Private data of a mac_t object.
 */
struct private_mac_t {

	/**
	 * Public interface.
	 */
	mac_t public;

	/**
	 * Key schedule for key K
	 */
	aesni_key_t *k;

	/**
	 * K1
	 */
	__m128i k1;

	/**
	 * K2
	 */
	__m128i k2;

	/**
	 * T
	 */
	__m128i t;

	/**
	 * remaining, unprocessed bytes in append mode
	 */
	u_char rem[AES_BLOCK_SIZE];

	/**
	 * number of bytes in remaining
	 */
	int rem_size;
};

METHOD(mac_t, get_mac, bool,
	private_mac_t *this, chunk_t data, uint8_t *out)
{
	__m128i *ks, t, l, *bi;
	u_int blocks, rem, i;

	if (!this->k)
	{
		return FALSE;
	}

	ks = this->k->schedule;
	t = this->t;

	if (this->rem_size + data.len > AES_BLOCK_SIZE)
	{
		/* T := 0x00000000000000000000000000000000 (initially)
		 * for each block M_i (except the last)
		 *   X := T XOR M_i;
		 *   T := AES-128(K, X);
		 */

		/* append data to remaining bytes, process block M_1 */
		memcpy(this->rem + this->rem_size, data.ptr,
			   AES_BLOCK_SIZE - this->rem_size);
		data = chunk_skip(data, AES_BLOCK_SIZE - this->rem_size);

		t = _mm_xor_si128(t, _mm_loadu_si128((__m128i*)this->rem));

		t = _mm_xor_si128(t, ks[0]);
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

		/* process blocks M_2 ... M_n-1 */
		bi = (__m128i*)data.ptr;
		rem = data.len % AES_BLOCK_SIZE;
		blocks = data.len / AES_BLOCK_SIZE;
		if (!rem && blocks)
		{	/* don't do last block */
			rem = AES_BLOCK_SIZE;
			blocks--;
		}

		/* process blocks M[2] ... M[n-1] */
		for (i = 0; i < blocks; i++)
		{
			t = _mm_xor_si128(t, _mm_loadu_si128(bi + i));

			t = _mm_xor_si128(t, ks[0]);
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
		}

		/* store remaining bytes of block M_n */
		memcpy(this->rem, data.ptr + data.len - rem, rem);
		this->rem_size = rem;
	}
	else
	{
		/* no complete block (or last block), just copy into remaining */
		memcpy(this->rem + this->rem_size, data.ptr, data.len);
		this->rem_size += data.len;
	}
	if (out)
	{
		/* if last block is complete
		 *   M_last := M_n XOR K1;
		 * else
		 *   M_last := padding(M_n) XOR K2;
		 */
		if (this->rem_size == AES_BLOCK_SIZE)
		{
			l = _mm_loadu_si128((__m128i*)this->rem);
			l = _mm_xor_si128(l, this->k1);
		}
		else
		{
			/* padding(x) = x || 10^i  where i is 128-8*r-1
			 * That is, padding(x) is the concatenation of x and a single '1',
			 * followed by the minimum number of '0's, so that the total length is
			 * equal to 128 bits.
			 */
			if (this->rem_size < AES_BLOCK_SIZE)
			{
				memset(this->rem + this->rem_size, 0,
					   AES_BLOCK_SIZE - this->rem_size);
				this->rem[this->rem_size] = 0x80;
			}
			l = _mm_loadu_si128((__m128i*)this->rem);
			l = _mm_xor_si128(l, this->k2);
		}
		/* T := M_last XOR T;
		 * T := AES-128(K,T);
		 */
		t = _mm_xor_si128(l, t);

		t = _mm_xor_si128(t, ks[0]);
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

		_mm_storeu_si128((__m128i*)out, t);

		/* reset state */
		t = _mm_setzero_si128();
		this->rem_size = 0;
	}
	this->t = t;
	return TRUE;
}

METHOD(mac_t, get_mac_size, size_t,
	private_mac_t *this)
{
	return AES_BLOCK_SIZE;
}

/**
 * Left-shift the given chunk by one bit.
 */
static void bit_shift(chunk_t chunk)
{
	size_t i;

	for (i = 0; i < chunk.len; i++)
	{
		chunk.ptr[i] <<= 1;
		if (i < chunk.len - 1 && chunk.ptr[i + 1] & 0x80)
		{
			chunk.ptr[i] |= 0x01;
		}
	}
}

METHOD(mac_t, set_key, bool,
	private_mac_t *this, chunk_t key)
{
	__m128i rb, msb, l, a;
	u_int round;
	chunk_t k;

	this->t = _mm_setzero_si128();
	this->rem_size = 0;

	/* we support variable keys as defined in RFC 4615 */
	if (key.len == AES_BLOCK_SIZE)
	{
		k = key;
	}
	else
	{	/* use cmac recursively to resize longer or shorter keys */
		k = chunk_alloca(AES_BLOCK_SIZE);
		memset(k.ptr, 0, k.len);
		if (!set_key(this, k) || !get_mac(this, key, k.ptr))
		{
			return FALSE;
		}
	}

	DESTROY_IF(this->k);
	this->k = aesni_key_create(TRUE, k);
	if (!this->k)
	{
		return FALSE;
	}

	/*
	 * Rb = 0x00000000000000000000000000000087
	 * L = 0x00000000000000000000000000000000 encrypted with K
	 * if MSB(L) == 0
	 *   K1 = L << 1
	 * else
	 *   K1 = (L << 1) XOR Rb
	 * if MSB(K1) == 0
	 *   K2 = K1 << 1
	 * else
	 *   K2 = (K1 << 1) XOR Rb
	 */

	rb = _mm_set_epi32(0x87000000, 0, 0, 0);
	msb = _mm_set_epi32(0, 0, 0, 0x80);

	l = _mm_setzero_si128();

	l = _mm_xor_si128(l, this->k->schedule[0]);
	for (round = 1; round < this->k->rounds; round++)
	{
		l = _mm_aesenc_si128(l, this->k->schedule[round]);
	}
	l = _mm_aesenclast_si128(l, this->k->schedule[this->k->rounds]);

	this->k1 = l;
	bit_shift(chunk_from_thing(this->k1));
	a = _mm_and_si128(l, msb);
	if (memchr(&a, 0x80, 1))
	{
		this->k1 = _mm_xor_si128(this->k1, rb);
	}
	this->k2 = this->k1;
	bit_shift(chunk_from_thing(this->k2));
	a = _mm_and_si128(this->k1, msb);
	if (memchr(&a, 0x80, 1))
	{
		this->k2 = _mm_xor_si128(this->k2, rb);
	}

	return TRUE;
}

METHOD(mac_t, destroy, void,
	private_mac_t *this)
{
	DESTROY_IF(this->k);
	memwipe(&this->k1, sizeof(this->k1));
	memwipe(&this->k2, sizeof(this->k2));
	free_align(this);
}

/*
 * Described in header
 */
mac_t *aesni_cmac_create(encryption_algorithm_t algo, size_t key_size)
{
	private_mac_t *this;

	INIT_ALIGN(this, sizeof(__m128i),
		.public = {
			.get_mac = _get_mac,
			.get_mac_size = _get_mac_size,
			.set_key = _set_key,
			.destroy = _destroy,
		},
	);

	return &this->public;
}

/*
 * Described in header.
 */
prf_t *aesni_cmac_prf_create(pseudo_random_function_t algo)
{
	mac_t *cmac;

	switch (algo)
	{
		case PRF_AES128_CMAC:
			cmac = aesni_cmac_create(ENCR_AES_CBC, 16);
			break;
		default:
			return NULL;
	}
	if (cmac)
	{
		return mac_prf_create(cmac);
	}
	return NULL;
}

/*
 * Described in header
 */
signer_t *aesni_cmac_signer_create(integrity_algorithm_t algo)
{
	size_t truncation;
	mac_t *cmac;

	switch (algo)
	{
		case AUTH_AES_CMAC_96:
			cmac = aesni_cmac_create(ENCR_AES_CBC, 16);
			truncation = 12;
			break;
		default:
			return NULL;
	}
	if (cmac)
	{
		return mac_signer_create(cmac, truncation);
	}
	return NULL;
}
