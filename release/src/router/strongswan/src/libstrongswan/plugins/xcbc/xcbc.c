/*
 * Copyright (C) 2012 Tobias Brunner
 * Copyright (C) 2008 Martin Willi
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

#include "xcbc.h"

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
	 * Public mac_t interface.
	 */
	mac_t public;

	/**
	 * Block size, in bytes
	 */
	uint8_t b;

	/**
	 * crypter using k1
	 */
	crypter_t *k1;

	/**
	 * k2
	 */
	uint8_t *k2;

	/**
	 * k3
	 */
	uint8_t *k3;

	/**
	 * E
	 */
	uint8_t *e;

	/**
	 * remaining, unprocessed bytes in append mode
	 */
	uint8_t *remaining;

	/**
	 * number of bytes in remaining
	 */
	int remaining_bytes;

	/**
	 * TRUE if we have zero bytes to xcbc in final()
	 */
	bool zero;
};

/**
 * xcbc supplied data, but do not run final operation
 */
static bool update(private_mac_t *this, chunk_t data)
{
	chunk_t iv;

	if (data.len)
	{
		this->zero = FALSE;
	}

	if (this->remaining_bytes + data.len <= this->b)
	{	/* no complete block, just copy into remaining */
		memcpy(this->remaining + this->remaining_bytes, data.ptr, data.len);
		this->remaining_bytes += data.len;
		return TRUE;
	}

	iv = chunk_alloca(this->b);
	memset(iv.ptr, 0, iv.len);

	/* (3) For each block M[i], where i = 1 ... n-1:
	 *     XOR M[i] with E[i-1], then encrypt the result with Key K1,
	 *     yielding E[i].
	 */

	/* append data to remaining bytes, process block M[1] */
	memcpy(this->remaining + this->remaining_bytes, data.ptr,
		   this->b - this->remaining_bytes);
	data = chunk_skip(data, this->b - this->remaining_bytes);
	memxor(this->e, this->remaining, this->b);
	if (!this->k1->encrypt(this->k1, chunk_create(this->e, this->b), iv, NULL))
	{
		return FALSE;
	}

	/* process blocks M[2] ... M[n-1] */
	while (data.len > this->b)
	{
		memcpy(this->remaining, data.ptr, this->b);
		data = chunk_skip(data, this->b);
		memxor(this->e, this->remaining, this->b);
		if (!this->k1->encrypt(this->k1, chunk_create(this->e, this->b),
							   iv, NULL))
		{
			return FALSE;
		}
	}

	/* store remaining bytes of block M[n] */
	memcpy(this->remaining, data.ptr, data.len);
	this->remaining_bytes = data.len;

	return TRUE;
}

/**
 * run last round, data is in this->e
 */
static bool final(private_mac_t *this, uint8_t *out)
{
	chunk_t iv;

	iv = chunk_alloca(this->b);
	memset(iv.ptr, 0, iv.len);

	/* (4) For block M[n]: */
	if (this->remaining_bytes == this->b && !this->zero)
	{
		/* a) If the blocksize of M[n] is 128 bits:
		 *    XOR M[n] with E[n-1] and Key K2, then encrypt the result with
		 *    Key K1, yielding E[n].
		 */
		memxor(this->e, this->remaining, this->b);
		memxor(this->e, this->k2, this->b);
	}
	else
	{
		/* b) If the blocksize of M[n] is less than 128 bits:
		 *
		 *  i) Pad M[n] with a single "1" bit, followed by the number of
		 *     "0" bits (possibly none) required to increase M[n]'s
		 *     blocksize to 128 bits.
		 */
		if (this->remaining_bytes < this->b)
		{
			this->remaining[this->remaining_bytes] = 0x80;
			while (++this->remaining_bytes < this->b)
			{
				this->remaining[this->remaining_bytes] = 0x00;
			}
		}
		/*  ii) XOR M[n] with E[n-1] and Key K3, then encrypt the result
		 *      with Key K1, yielding E[n].
		 */
		memxor(this->e, this->remaining, this->b);
		memxor(this->e, this->k3, this->b);
	}
	if (!this->k1->encrypt(this->k1, chunk_create(this->e, this->b), iv, NULL))
	{
		return FALSE;
	}

	memcpy(out, this->e, this->b);

	/* (2) Define E[0] = 0x00000000000000000000000000000000 */
	memset(this->e, 0, this->b);
	this->remaining_bytes = 0;
	this->zero = TRUE;

	return TRUE;
}

METHOD(mac_t, get_mac, bool,
	private_mac_t *this, chunk_t data, uint8_t *out)
{
	/* update E, do not process last block */
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

METHOD(mac_t, set_key, bool,
	private_mac_t *this, chunk_t key)
{
	chunk_t iv, k1, lengthened;

	memset(this->e, 0, this->b);
	this->remaining_bytes = 0;
	this->zero = TRUE;

	/* we support variable keys from RFC4434 */
	if (key.len == this->b)
	{
		lengthened = key;
	}
	else if (key.len < this->b)
	{	/* pad short keys */
		lengthened = chunk_alloca(this->b);
		memset(lengthened.ptr, 0, lengthened.len);
		memcpy(lengthened.ptr, key.ptr, key.len);
	}
	else
	{	/* shorten key using xcbc */
		lengthened = chunk_alloca(this->b);
		memset(lengthened.ptr, 0, lengthened.len);
		if (!set_key(this, lengthened) ||
			!get_mac(this, key, lengthened.ptr))
		{
			return FALSE;
		}
	}

	k1 = chunk_alloca(this->b);
	iv = chunk_alloca(this->b);
	memset(iv.ptr, 0, iv.len);

	/*
	 * (1) Derive 3 128-bit keys (K1, K2 and K3) from the 128-bit secret
	 *     key K, as follows:
	 *     K1 = 0x01010101010101010101010101010101 encrypted with Key K
	 *     K2 = 0x02020202020202020202020202020202 encrypted with Key K
	 *     K3 = 0x03030303030303030303030303030303 encrypted with Key K
	 */

	memset(k1.ptr, 0x01, this->b);
	memset(this->k2, 0x02, this->b);
	memset(this->k3, 0x03, this->b);

	if (!this->k1->set_key(this->k1, lengthened) ||
		!this->k1->encrypt(this->k1, chunk_create(this->k2, this->b), iv, NULL) ||
		!this->k1->encrypt(this->k1, chunk_create(this->k3, this->b), iv, NULL) ||
		!this->k1->encrypt(this->k1, k1, iv, NULL) ||
		!this->k1->set_key(this->k1, k1))
	{
		memwipe(k1.ptr, k1.len);
		return FALSE;
	}
	memwipe(k1.ptr, k1.len);
	return TRUE;
}

METHOD(mac_t, destroy, void,
	private_mac_t *this)
{
	this->k1->destroy(this->k1);
	memwipe(this->k2, this->b);
	free(this->k2);
	memwipe(this->k3, this->b);
	free(this->k3);
	free(this->e);
	free(this->remaining);
	free(this);
}

/*
 * Described in header
 */
static mac_t *xcbc_create(encryption_algorithm_t algo, size_t key_size)
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
	/* input and output of crypter must be equal for xcbc */
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
		.k1 = crypter,
		.k2 = malloc(b),
		.k3 = malloc(b),
		.e = malloc(b),
		.remaining = malloc(b),
		.zero = TRUE,
	);
	memset(this->e, 0, b);

	return &this->public;
}

/*
 * Described in header.
 */
prf_t *xcbc_prf_create(pseudo_random_function_t algo)
{
	mac_t *xcbc;

	switch (algo)
	{
		case PRF_AES128_XCBC:
			xcbc = xcbc_create(ENCR_AES_CBC, 16);
			break;
		case PRF_CAMELLIA128_XCBC:
			xcbc = xcbc_create(ENCR_CAMELLIA_CBC, 16);
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
signer_t *xcbc_signer_create(integrity_algorithm_t algo)
{
	size_t trunc;
	mac_t *xcbc;

	switch (algo)
	{
		case AUTH_AES_XCBC_96:
			xcbc = xcbc_create(ENCR_AES_CBC, 16);
			trunc = 12;
			break;
		case AUTH_CAMELLIA_XCBC_96:
			xcbc = xcbc_create(ENCR_CAMELLIA_CBC, 16);
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
