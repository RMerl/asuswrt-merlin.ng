/*
 * Copyright (C) 2008-2015 Martin Willi
 * Copyright (C) 2012 Tobias Brunner
 * HSR Hochschule fuer Technik Rapperswil
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

#include "aesni_xcbc.h"
#include "aesni_key.h"

#include <crypto/prfs/mac_prf.h>
#include <crypto/signers/mac_signer.h>

typedef struct private_aesni_mac_t private_aesni_mac_t;

/**
 * Private data of a mac_t object.
 */
struct private_aesni_mac_t {

	/**
	 * Public mac_t interface.
	 */
	mac_t public;

	/**
	 * Key schedule for K1
	 */
	aesni_key_t *k1;

	/**
	 * k2
	 */
	__m128i k2;

	/**
	 * k3
	 */
	__m128i k3;

	/**
	 * E
	 */
	__m128i e;

	/**
	 * remaining, unprocessed bytes in append mode
	 */
	u_char rem[AES_BLOCK_SIZE];

	/**
	 * number of bytes used in remaining
	 */
	int rem_size;

	/**
	 * TRUE if we have zero bytes to xcbc in final()
	 */
	bool zero;
};

METHOD(mac_t, get_mac, bool,
	private_aesni_mac_t *this, chunk_t data, uint8_t *out)
{
	__m128i *ks, e, *bi;
	u_int blocks, rem, i;

	if (!this->k1)
	{
		return FALSE;
	}

	ks = this->k1->schedule;

	e = this->e;

	if (data.len)
	{
		this->zero = FALSE;
	}

	if (this->rem_size + data.len > AES_BLOCK_SIZE)
	{
		/* (3) For each block M[i], where i = 1 ... n-1:
		 *     XOR M[i] with E[i-1], then encrypt the result with Key K1,
		 *     yielding E[i].
		 */

		/* append data to remaining bytes, process block M[1] */
		memcpy(this->rem + this->rem_size, data.ptr,
			   AES_BLOCK_SIZE - this->rem_size);
		data = chunk_skip(data, AES_BLOCK_SIZE - this->rem_size);

		e = _mm_xor_si128(e, _mm_loadu_si128((__m128i*)this->rem));

		e = _mm_xor_si128(e, ks[0]);
		e = _mm_aesenc_si128(e, ks[1]);
		e = _mm_aesenc_si128(e, ks[2]);
		e = _mm_aesenc_si128(e, ks[3]);
		e = _mm_aesenc_si128(e, ks[4]);
		e = _mm_aesenc_si128(e, ks[5]);
		e = _mm_aesenc_si128(e, ks[6]);
		e = _mm_aesenc_si128(e, ks[7]);
		e = _mm_aesenc_si128(e, ks[8]);
		e = _mm_aesenc_si128(e, ks[9]);
		e = _mm_aesenclast_si128(e, ks[10]);

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
			e = _mm_xor_si128(e, _mm_loadu_si128(bi + i));

			e = _mm_xor_si128(e, ks[0]);
			e = _mm_aesenc_si128(e, ks[1]);
			e = _mm_aesenc_si128(e, ks[2]);
			e = _mm_aesenc_si128(e, ks[3]);
			e = _mm_aesenc_si128(e, ks[4]);
			e = _mm_aesenc_si128(e, ks[5]);
			e = _mm_aesenc_si128(e, ks[6]);
			e = _mm_aesenc_si128(e, ks[7]);
			e = _mm_aesenc_si128(e, ks[8]);
			e = _mm_aesenc_si128(e, ks[9]);
			e = _mm_aesenclast_si128(e, ks[10]);
		}

		/* store remaining bytes of block M[n] */
		memcpy(this->rem, data.ptr + data.len - rem, rem);
		this->rem_size = rem;
	}
	else
	{
		/* no complete block, just copy into remaining */
		memcpy(this->rem + this->rem_size, data.ptr, data.len);
		this->rem_size += data.len;
	}

	if (out)
	{
		/* (4) For block M[n]: */
		if (this->rem_size == AES_BLOCK_SIZE && !this->zero)
		{
			/* a) If the blocksize of M[n] is 128 bits:
			 *    XOR M[n] with E[n-1] and Key K2, then encrypt the result with
			 *    Key K1, yielding E[n].
			 */
			e = _mm_xor_si128(e, this->k2);
		}
		else
		{
			/* b) If the blocksize of M[n] is less than 128 bits:
			 *
			 *  i) Pad M[n] with a single "1" bit, followed by the number of
			 *     "0" bits (possibly none) required to increase M[n]'s
			 *     blocksize to 128 bits.
			 */
			if (this->rem_size < AES_BLOCK_SIZE)
			{
				memset(this->rem + this->rem_size, 0,
					   AES_BLOCK_SIZE - this->rem_size);
				this->rem[this->rem_size] = 0x80;
			}
			/*  ii) XOR M[n] with E[n-1] and Key K3, then encrypt the result
			 *      with Key K1, yielding E[n].
			 */
			e = _mm_xor_si128(e, this->k3);
		}
		e = _mm_xor_si128(e, _mm_loadu_si128((__m128i*)this->rem));

		e = _mm_xor_si128(e, ks[0]);
		e = _mm_aesenc_si128(e, ks[1]);
		e = _mm_aesenc_si128(e, ks[2]);
		e = _mm_aesenc_si128(e, ks[3]);
		e = _mm_aesenc_si128(e, ks[4]);
		e = _mm_aesenc_si128(e, ks[5]);
		e = _mm_aesenc_si128(e, ks[6]);
		e = _mm_aesenc_si128(e, ks[7]);
		e = _mm_aesenc_si128(e, ks[8]);
		e = _mm_aesenc_si128(e, ks[9]);
		e = _mm_aesenclast_si128(e, ks[10]);
		_mm_storeu_si128((__m128i*)out, e);

		/* (2) Define E[0] = 0x00000000000000000000000000000000 */
		e = _mm_setzero_si128();
		this->rem_size = 0;
		this->zero = TRUE;
	}
	this->e = e;
	return TRUE;
}

METHOD(mac_t, get_mac_size, size_t,
	private_aesni_mac_t *this)
{
	return AES_BLOCK_SIZE;
}

METHOD(mac_t, set_key, bool,
	private_aesni_mac_t *this, chunk_t key)
{
	__m128i t1, t2, t3;
	u_char k1[AES_BLOCK_SIZE];
	u_int round;
	chunk_t k;

	/* reset state */
	this->e = _mm_setzero_si128();
	this->rem_size = 0;
	this->zero = TRUE;

	/* Create RFC4434 variable keys if required */
	if (key.len == AES_BLOCK_SIZE)
	{
		k = key;
	}
	else if (key.len < AES_BLOCK_SIZE)
	{	/* pad short keys */
		k = chunk_alloca(AES_BLOCK_SIZE);
		memset(k.ptr, 0, k.len);
		memcpy(k.ptr, key.ptr, key.len);
	}
	else
	{	/* shorten key using XCBC */
		k = chunk_alloca(AES_BLOCK_SIZE);
		memset(k.ptr, 0, k.len);
		if (!set_key(this, k) || !get_mac(this, key, k.ptr))
		{
			return FALSE;
		}
	}

	/*
	 * (1) Derive 3 128-bit keys (K1, K2 and K3) from the 128-bit secret
	 *     key K, as follows:
	 *     K1 = 0x01010101010101010101010101010101 encrypted with Key K
	 *     K2 = 0x02020202020202020202020202020202 encrypted with Key K
	 *     K3 = 0x03030303030303030303030303030303 encrypted with Key K
	 */

	DESTROY_IF(this->k1);
	this->k1 = aesni_key_create(TRUE, k);
	if (!this->k1)
	{
		return FALSE;
	}

	t1 = _mm_set1_epi8(0x01);
	t2 = _mm_set1_epi8(0x02);
	t3 = _mm_set1_epi8(0x03);

	t1 = _mm_xor_si128(t1, this->k1->schedule[0]);
	t2 = _mm_xor_si128(t2, this->k1->schedule[0]);
	t3 = _mm_xor_si128(t3, this->k1->schedule[0]);

	for (round = 1; round < this->k1->rounds; round++)
	{
		t1 = _mm_aesenc_si128(t1, this->k1->schedule[round]);
		t2 = _mm_aesenc_si128(t2, this->k1->schedule[round]);
		t3 = _mm_aesenc_si128(t3, this->k1->schedule[round]);
	}

	t1 = _mm_aesenclast_si128(t1, this->k1->schedule[this->k1->rounds]);
	t2 = _mm_aesenclast_si128(t2, this->k1->schedule[this->k1->rounds]);
	t3 = _mm_aesenclast_si128(t3, this->k1->schedule[this->k1->rounds]);

	_mm_storeu_si128((__m128i*)k1, t1);
	this->k2 = t2;
	this->k3 = t3;

	this->k1->destroy(this->k1);
	this->k1 = aesni_key_create(TRUE, chunk_from_thing(k1));

	memwipe(k1, AES_BLOCK_SIZE);
	return this->k1 != NULL;
}

METHOD(mac_t, destroy, void,
	private_aesni_mac_t *this)
{
	DESTROY_IF(this->k1);
	memwipe(&this->k2, sizeof(this->k2));
	memwipe(&this->k3, sizeof(this->k3));
	free_align(this);
}

/*
 * Described in header
 */
mac_t *aesni_xcbc_create(encryption_algorithm_t algo, size_t key_size)
{
	private_aesni_mac_t *this;

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
prf_t *aesni_xcbc_prf_create(pseudo_random_function_t algo)
{
	mac_t *xcbc;

	switch (algo)
	{
		case PRF_AES128_XCBC:
			xcbc = aesni_xcbc_create(ENCR_AES_CBC, 16);
			break;
		default:
			return NULL;
	}
	if (xcbc)
	{
		return mac_prf_create(xcbc);
	}
	return NULL;
}

/*
 * Described in header
 */
signer_t *aesni_xcbc_signer_create(integrity_algorithm_t algo)
{
	size_t trunc;
	mac_t *xcbc;

	switch (algo)
	{
		case AUTH_AES_XCBC_96:
			xcbc = aesni_xcbc_create(ENCR_AES_CBC, 16);
			trunc = 12;
			break;
		default:
			return NULL;
	}
	if (xcbc)
	{
		return mac_signer_create(xcbc, trunc);
	}
	return NULL;
}
