/*
 * Copyright (C) 2012 Tobias Brunner
 * Copyright (C) 2005-2006 Martin Willi
 * Copyright (C) 2005 Jan Hutter
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

#include "hmac.h"

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
	 * Implements mac_t interface
	 */
	mac_t public;

	/**
	 * Block size, as in RFC.
	 */
	uint8_t b;

	/**
	 * Hash function.
	 */
	hasher_t *h;

	/**
	 * Previously xor'ed key using opad.
	 */
	chunk_t opaded_key;

	/**
	 * Previously xor'ed key using ipad.
	 */
	chunk_t ipaded_key;
};

METHOD(mac_t, get_mac, bool,
	private_mac_t *this, chunk_t data, uint8_t *out)
{
	/* H(K XOR opad, H(K XOR ipad, text))
	 *
	 * if out is NULL, we append text to the inner hash.
	 * else, we complete the inner and do the outer.
	 *
	 */

	uint8_t buffer[this->h->get_hash_size(this->h)];
	chunk_t inner;

	if (out == NULL)
	{
		/* append data to inner */
		return this->h->get_hash(this->h, data, NULL);
	}

	/* append and do outer hash */
	inner.ptr = buffer;
	inner.len = this->h->get_hash_size(this->h);

	/* complete inner, do outer and reinit for next call */
	return this->h->get_hash(this->h, data, buffer) &&
		   this->h->get_hash(this->h, this->opaded_key, NULL) &&
		   this->h->get_hash(this->h, inner, out) &&
		   this->h->get_hash(this->h, this->ipaded_key, NULL);
}

METHOD(mac_t, get_mac_size, size_t,
	private_mac_t *this)
{
	return this->h->get_hash_size(this->h);
}

METHOD(mac_t, set_key, bool,
	private_mac_t *this, chunk_t key)
{
	int i;
	uint8_t buffer[this->b];

	memset(buffer, 0, this->b);

	if (key.len > this->b)
	{
		/* if key is too long, it will be hashed */
		if (!this->h->reset(this->h) ||
			!this->h->get_hash(this->h, key, buffer))
		{
			return FALSE;
		}
	}
	else
	{
		/* if not, just copy it in our pre-padded k */
		memcpy(buffer, key.ptr, key.len);
	}

	/* apply ipad and opad to key */
	for (i = 0; i < this->b; i++)
	{
		this->ipaded_key.ptr[i] = buffer[i] ^ 0x36;
		this->opaded_key.ptr[i] = buffer[i] ^ 0x5C;
	}

	/* begin hashing of inner pad */
	return this->h->reset(this->h) &&
		   this->h->get_hash(this->h, this->ipaded_key, NULL);
}

METHOD(mac_t, destroy, void,
	private_mac_t *this)
{
	this->h->destroy(this->h);
	chunk_clear(&this->opaded_key);
	chunk_clear(&this->ipaded_key);
	free(this);
}

/*
 * Creates an mac_t object
 */
static mac_t *hmac_create(hash_algorithm_t hash_algorithm)
{
	private_mac_t *this;

	INIT(this,
		.public = {
			.get_mac = _get_mac,
			.get_mac_size = _get_mac_size,
			.set_key = _set_key,
			.destroy = _destroy,
		},
	);

	/* set b, according to hasher */
	switch (hash_algorithm)
	{
		case HASH_SHA1:
		case HASH_MD5:
		case HASH_SHA256:
			this->b = 64;
			break;
		case HASH_SHA384:
		case HASH_SHA512:
			this->b = 128;
			break;
		default:
			free(this);
			return NULL;
	}

	this->h = lib->crypto->create_hasher(lib->crypto, hash_algorithm);
	if (this->h == NULL)
	{
		free(this);
		return NULL;
	}

	/* build ipad and opad */
	this->opaded_key.ptr = malloc(this->b);
	this->opaded_key.len = this->b;

	this->ipaded_key.ptr = malloc(this->b);
	this->ipaded_key.len = this->b;

	return &this->public;
}

/*
 * Described in header
 */
prf_t *hmac_prf_create(pseudo_random_function_t algo)
{
	mac_t *hmac;

	hmac = hmac_create(hasher_algorithm_from_prf(algo));
	if (hmac)
	{
		return mac_prf_create(hmac);
	}
	return NULL;
}

/*
 * Described in header
 */
signer_t *hmac_signer_create(integrity_algorithm_t algo)
{
	mac_t *hmac;
	size_t trunc;

	hmac = hmac_create(hasher_algorithm_from_integrity(algo, &trunc));
	if (hmac)
	{
		return mac_signer_create(hmac, trunc);
	}
	return NULL;
}
