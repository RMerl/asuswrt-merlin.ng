/*
 * Copyright (C) 2006 Martin Willi
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

#include "fips_prf.h"

#include <utils/debug.h>

typedef struct private_fips_prf_t private_fips_prf_t;

/**
 * Private data of a fips_prf_t object.
 */
struct private_fips_prf_t {
	/**
	 * Public fips_prf_t interface.
	 */
	fips_prf_t public;

	/**
	 * key of prf function, "b" long
	 */
	uint8_t *key;

	/**
	 * size of "b" in bytes
	 */
	size_t b;

	/**
	 * Keyed SHA1 prf: It does not use SHA1Final operation
	 */
	prf_t *keyed_prf;

	/**
	 * G function, either SHA1 or DES
	 */
	bool (*g)(private_fips_prf_t *this, chunk_t c, uint8_t res[]);
};

/**
 * sum = (a + b) mod 2 ^ (length * 8)
 */
static void add_mod(size_t length, uint8_t a[], uint8_t b[], uint8_t sum[])
{
	int i, c = 0;

	for(i = length - 1; i >= 0; i--)
	{
		uint32_t tmp;

		tmp = a[i] + b[i] + c;
		sum[i] = 0xff & tmp;
		c = tmp >> 8;
	}
}

/**
 * calculate "chunk mod 2^(length*8)" and save it into buffer
 */
static void chunk_mod(size_t length, chunk_t chunk, uint8_t buffer[])
{
	if (chunk.len < length)
	{
		/* apply seed as least significant bits, others are zero */
		memset(buffer, 0, length - chunk.len);
		memcpy(buffer + length - chunk.len, chunk.ptr, chunk.len);
	}
	else
	{
		/* use least significant bytes from seed, as we use mod 2^b */
		memcpy(buffer, chunk.ptr + chunk.len - length, length);
	}
}

/**
 * Implementation of prf_t.get_bytes.
 *
 * Test vector:
 *
 * key:
 * 0xbd, 0x02, 0x9b, 0xbe, 0x7f, 0x51, 0x96, 0x0b,
 * 0xcf, 0x9e, 0xdb, 0x2b, 0x61, 0xf0, 0x6f, 0x0f,
 * 0xeb, 0x5a, 0x38, 0xb6
 *
 * seed:
 * 0x00
 *
 * result:
 * 0x20, 0x70, 0xb3, 0x22, 0x3d, 0xba, 0x37, 0x2f,
 * 0xde, 0x1c, 0x0f, 0xfc, 0x7b, 0x2e, 0x3b, 0x49,
 * 0x8b, 0x26, 0x06, 0x14, 0x3c, 0x6c, 0x18, 0xba,
 * 0xcb, 0x0f, 0x6c, 0x55, 0xba, 0xbb, 0x13, 0x78,
 * 0x8e, 0x20, 0xd7, 0x37, 0xa3, 0x27, 0x51, 0x16
 */
METHOD(prf_t, get_bytes, bool,
	private_fips_prf_t *this, chunk_t seed, uint8_t w[])
{
	int i;
	uint8_t xval[this->b];
	uint8_t xseed[this->b];
	uint8_t sum[this->b];
	uint8_t *xkey = this->key;
	uint8_t one[this->b];

	if (!w)
	{
		/* append mode is not supported */
		return FALSE;
	}

	memset(one, 0, this->b);
	one[this->b - 1] = 0x01;

	/* 3.1 */
	chunk_mod(this->b, seed, xseed);

	/* 3.2 */
	for (i = 0; i < 2; i++) /* twice */
	{
		/* a. XVAL = (XKEY + XSEED j) mod 2^b */
		add_mod(this->b, xkey, xseed, xval);
		DBG3(DBG_LIB, "XVAL %b", xval, (u_int)this->b);
		/* b. wi = G(t, XVAL ) */
		this->g(this, chunk_create(xval, this->b), &w[i * this->b]);
		DBG3(DBG_LIB, "w[%d] %b", i, &w[i * this->b], (u_int)this->b);
		/* c. XKEY = (1 + XKEY + wi) mod 2b */
		add_mod(this->b, xkey, &w[i * this->b], sum);
		add_mod(this->b, sum, one, xkey);
		DBG3(DBG_LIB, "XKEY %b", xkey, (u_int)this->b);
	}

	/* 3.3 done already, mod q not used */

	return TRUE;
}

METHOD(prf_t, get_block_size, size_t,
	private_fips_prf_t *this)
{
	return 2 * this->b;
}
METHOD(prf_t, allocate_bytes, bool,
	private_fips_prf_t *this, chunk_t seed, chunk_t *chunk)
{
	*chunk = chunk_alloc(get_block_size(this));
	return get_bytes(this, seed, chunk->ptr);
}

METHOD(prf_t, get_key_size, size_t,
	private_fips_prf_t *this)
{
	return this->b;
}

METHOD(prf_t, set_key, bool,
	private_fips_prf_t *this, chunk_t key)
{
	/* save key as "key mod 2^b" */
	chunk_mod(this->b, key, this->key);
	return TRUE;
}

/**
 * Implementation of the G() function based on SHA1
 */
static bool g_sha1(private_fips_prf_t *this, chunk_t c, uint8_t res[])
{
	uint8_t buf[64];

	if (c.len < sizeof(buf))
	{
		/* pad c with zeros */
		memset(buf, 0, sizeof(buf));
		memcpy(buf, c.ptr, c.len);
		c.ptr = buf;
		c.len = sizeof(buf);
	}
	else
	{
		/* not more than 512 bits can be G()-ed */
		c.len = sizeof(buf);
	}

	/* use the keyed hasher, but use an empty key to use SHA1 IV */
	if (!this->keyed_prf->set_key(this->keyed_prf, chunk_empty) ||
		!this->keyed_prf->get_bytes(this->keyed_prf, c, res))
	{
		return FALSE;
	}
	return TRUE;
}

METHOD(prf_t, destroy, void,
	private_fips_prf_t *this)
{
	this->keyed_prf->destroy(this->keyed_prf);
	free(this->key);
	free(this);
}

/*
 * Described in header.
 */
fips_prf_t *fips_prf_create(pseudo_random_function_t algo)
{
	private_fips_prf_t *this;

	INIT(this,
		.public = {
			.prf_interface = {
				.get_bytes = _get_bytes,
				.allocate_bytes = _allocate_bytes,
				.get_block_size = _get_block_size,
				.get_key_size = _get_key_size,
				.set_key = _set_key,
				.destroy = _destroy,
			},
		},
	);

	switch (algo)
	{
		case PRF_FIPS_SHA1_160:
		{
			this->g = g_sha1;
			this->b = 20;
			this->keyed_prf = lib->crypto->create_prf(lib->crypto, PRF_KEYED_SHA1);
			if (this->keyed_prf == NULL)
			{
				free(this);
				return NULL;
			}
			break;
		}
		case PRF_FIPS_DES:
			/* not implemented yet */
		default:
			free(this);
			return NULL;
	}
	this->key = malloc(this->b);

	return &this->public;
}
