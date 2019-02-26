/*
 * Copyright (C) 2012 Tobias Brunner
 * HSR Hochschule fuer Technik Rapperswil
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

#include <string.h>

#include "cmac.h"

#include <utils/debug.h>
#include <crypto/mac.h>
#include <crypto/prfs/mac_prf.h>
#include <crypto/signers/mac_signer.h>

typedef struct private_mac_t private_mac_t;

/**
 * Private data of a mac_t object.
 *
 * The variable names are the same as in the RFC.
 */
struct private_mac_t {

	/**
	 * Public interface.
	 */
	mac_t public;

	/**
	 * Block size, in bytes
	 */
	uint8_t b;

	/**
	 * Crypter with key K
	 */
	crypter_t *k;

	/**
	 * K1
	 */
	uint8_t *k1;

	/**
	 * K2
	 */
	uint8_t *k2;

	/**
	 * T
	 */
	uint8_t *t;

	/**
	 * remaining, unprocessed bytes in append mode
	 */
	uint8_t *remaining;

	/**
	 * number of bytes in remaining
	 */
	int remaining_bytes;
};

/**
 * process supplied data, but do not run final operation
 */
static bool update(private_mac_t *this, chunk_t data)
{
	chunk_t iv;

	if (this->remaining_bytes + data.len <= this->b)
	{	/* no complete block (or last block), just copy into remaining */
		memcpy(this->remaining + this->remaining_bytes, data.ptr, data.len);
		this->remaining_bytes += data.len;
		return TRUE;
	}

	iv = chunk_alloca(this->b);
	memset(iv.ptr, 0, iv.len);

	/* T := 0x00000000000000000000000000000000 (initially)
	 * for each block M_i (except the last)
	 *   X := T XOR M_i;
	 *   T := AES-128(K, X);
	 */

	/* append data to remaining bytes, process block M_1 */
	memcpy(this->remaining + this->remaining_bytes, data.ptr,
		   this->b - this->remaining_bytes);
	data = chunk_skip(data, this->b - this->remaining_bytes);
	memxor(this->t, this->remaining, this->b);
	if (!this->k->encrypt(this->k, chunk_create(this->t, this->b), iv, NULL))
	{
		return FALSE;
	}

	/* process blocks M_2 ... M_n-1 */
	while (data.len > this->b)
	{
		memcpy(this->remaining, data.ptr, this->b);
		data = chunk_skip(data, this->b);
		memxor(this->t, this->remaining, this->b);
		if (!this->k->encrypt(this->k, chunk_create(this->t, this->b), iv, NULL))
		{
			return FALSE;
		}
	}

	/* store remaining bytes of block M_n */
	memcpy(this->remaining, data.ptr, data.len);
	this->remaining_bytes = data.len;

	return TRUE;
}

/**
 * process last block M_last
 */
static bool final(private_mac_t *this, uint8_t *out)
{
	chunk_t iv;

	iv = chunk_alloca(this->b);
	memset(iv.ptr, 0, iv.len);

	/* if last block is complete
	 *   M_last := M_n XOR K1;
	 * else
	 *   M_last := padding(M_n) XOR K2;
	 */
	if (this->remaining_bytes == this->b)
	{
		memxor(this->remaining, this->k1, this->b);
	}
	else
	{
		/* padding(x) = x || 10^i  where i is 128-8*r-1
		 * That is, padding(x) is the concatenation of x and a single '1',
		 * followed by the minimum number of '0's, so that the total length is
		 * equal to 128 bits.
		 */
		if (this->remaining_bytes < this->b)
		{
			this->remaining[this->remaining_bytes] = 0x80;
			while (++this->remaining_bytes < this->b)
			{
				this->remaining[this->remaining_bytes] = 0x00;
			}
		}
		memxor(this->remaining, this->k2, this->b);
	}
	/* T := M_last XOR T;
	 * T := AES-128(K,T);
	 */
	memxor(this->t, this->remaining, this->b);
	if (!this->k->encrypt(this->k, chunk_create(this->t, this->b), iv, NULL))
	{
		return FALSE;
	}

	memcpy(out, this->t, this->b);

	/* reset state */
	memset(this->t, 0, this->b);
	this->remaining_bytes = 0;

	return TRUE;
}

METHOD(mac_t, get_mac, bool,
	private_mac_t *this, chunk_t data, uint8_t *out)
{
	/* update T, do not process last block */
	if (!update(this, data))
	{
		return FALSE;
	}

	if (out)
	{	/* if not in append mode, process last block and output result */
		return final(this, out);
	}
	return TRUE;
}

METHOD(mac_t, get_mac_size, size_t,
	private_mac_t *this)
{
	return this->b;
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

/**
 * Apply the following key derivation (in-place):
 * if MSB(C) == 0
 *   C := C << 1
 * else
 *   C := (C << 1) XOR 0x00000000000000000000000000000087
 */
static void derive_key(chunk_t chunk)
{
	if (chunk.ptr[0] & 0x80)
	{
		chunk_t rb;

		rb = chunk_alloca(chunk.len);
		memset(rb.ptr, 0, rb.len);
		rb.ptr[rb.len - 1] = 0x87;
		bit_shift(chunk);
		memxor(chunk.ptr, rb.ptr, chunk.len);
	}
	else
	{
		bit_shift(chunk);
	}
}

METHOD(mac_t, set_key, bool,
	private_mac_t *this, chunk_t key)
{
	chunk_t resized, iv, l;

	memset(this->t, 0, this->b);
	this->remaining_bytes = 0;

	/* we support variable keys as defined in RFC 4615 */
	if (key.len == this->b)
	{
		resized = key;
	}
	else
	{	/* use cmac recursively to resize longer or shorter keys */
		resized = chunk_alloca(this->b);
		memset(resized.ptr, 0, resized.len);
		if (!set_key(this, resized) ||
			!get_mac(this, key, resized.ptr))
		{
			return FALSE;
		}
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
	iv = chunk_alloca(this->b);
	memset(iv.ptr, 0, iv.len);
	l = chunk_alloca(this->b);
	memset(l.ptr, 0, l.len);
	if (!this->k->set_key(this->k, resized) ||
		!this->k->encrypt(this->k, l, iv, NULL))
	{
		return FALSE;
	}
	derive_key(l);
	memcpy(this->k1, l.ptr, l.len);
	derive_key(l);
	memcpy(this->k2, l.ptr, l.len);
	memwipe(l.ptr, l.len);

	return TRUE;
}

METHOD(mac_t, destroy, void,
	private_mac_t *this)
{
	this->k->destroy(this->k);
	memwipe(this->k1, this->b);
	free(this->k1);
	memwipe(this->k2, this->b);
	free(this->k2);
	free(this->t);
	free(this->remaining);
	free(this);
}

/*
 * Described in header
 */
mac_t *cmac_create(encryption_algorithm_t algo, size_t key_size)
{
	private_mac_t *this;
	crypter_t *crypter;
	uint8_t b;

	crypter = lib->crypto->create_crypter(lib->crypto, algo, key_size);
	if (!crypter)
	{
		return NULL;
	}
	b = crypter->get_block_size(crypter);
	/* input and output of crypter must be equal for cmac */
	if (b != key_size)
	{
		crypter->destroy(crypter);
		return NULL;
	}

	INIT(this,
		.public = {
			.get_mac = _get_mac,
			.get_mac_size = _get_mac_size,
			.set_key = _set_key,
			.destroy = _destroy,
		},
		.b = b,
		.k = crypter,
		.k1 = malloc(b),
		.k2 = malloc(b),
		.t = malloc(b),
		.remaining = malloc(b),
	);
	memset(this->t, 0, b);

	return &this->public;
}

/*
 * Described in header.
 */
prf_t *cmac_prf_create(pseudo_random_function_t algo)
{
	mac_t *cmac;

	switch (algo)
	{
		case PRF_AES128_CMAC:
			cmac = cmac_create(ENCR_AES_CBC, 16);
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
signer_t *cmac_signer_create(integrity_algorithm_t algo)
{
	size_t truncation;
	mac_t *cmac;

	switch (algo)
	{
		case AUTH_AES_CMAC_96:
			cmac = cmac_create(ENCR_AES_CBC, 16);
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
