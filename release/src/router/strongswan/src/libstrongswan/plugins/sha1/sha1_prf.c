/*
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

#include "sha1_prf.h"
#include "sha1_hasher.h"

#include <library.h>

typedef struct private_sha1_prf_t private_sha1_prf_t;
typedef struct private_sha1_hasher_t private_sha1_hasher_t;

/**
 * Private data structure with hasing context.
 */
struct private_sha1_hasher_t {
	/**
	 * Public interface for this hasher.
	 */
	sha1_hasher_t public;

	/*
	 * State of the hasher. From sha1_hasher.c, do not change it!
	 */
	uint32_t state[5];
	uint32_t count[2];
	uint8_t buffer[64];
};

/**
 * Private data structure with keyed prf context.
 */
struct private_sha1_prf_t {

	/**
	 * public prf interface
	 */
	sha1_prf_t public;

	/**
	 * internal used hasher
	 */
	private_sha1_hasher_t *hasher;
};

/**
 * From sha1_hasher.c
 */
extern void SHA1Update(private_sha1_hasher_t* this, uint8_t *data, uint32_t len);

METHOD(prf_t, get_bytes, bool,
	private_sha1_prf_t *this, chunk_t seed, uint8_t *bytes)
{
	uint32_t *hash = (uint32_t*)bytes;

	SHA1Update(this->hasher, seed.ptr, seed.len);

	hash[0] = htonl(this->hasher->state[0]);
	hash[1] = htonl(this->hasher->state[1]);
	hash[2] = htonl(this->hasher->state[2]);
	hash[3] = htonl(this->hasher->state[3]);
	hash[4] = htonl(this->hasher->state[4]);

	return TRUE;
}

METHOD(prf_t, get_block_size, size_t,
	private_sha1_prf_t *this)
{
	return HASH_SIZE_SHA1;
}

METHOD(prf_t, allocate_bytes, bool,
	private_sha1_prf_t *this, chunk_t seed, chunk_t *chunk)
{
	*chunk = chunk_alloc(HASH_SIZE_SHA1);
	return get_bytes(this, seed, chunk->ptr);
}

METHOD(prf_t, get_key_size, size_t,
	private_sha1_prf_t *this)
{
	return sizeof(this->hasher->state);
}

METHOD(prf_t, set_key, bool,
	private_sha1_prf_t *this, chunk_t key)
{
	int i, rounds;
	uint32_t *iv = (uint32_t*)key.ptr;

	if (!this->hasher->public.hasher_interface.reset(
										&this->hasher->public.hasher_interface))
	{
		return FALSE;
	}
	rounds = min(key.len/sizeof(uint32_t), sizeof(this->hasher->state));
	for (i = 0; i < rounds; i++)
	{
		this->hasher->state[i] ^= htonl(iv[i]);
	}
	return TRUE;
}

METHOD(prf_t, destroy, void,
	private_sha1_prf_t *this)
{
	this->hasher->public.hasher_interface.destroy(&this->hasher->public.hasher_interface);
	free(this);
}

/**
 * see header
 */
sha1_prf_t *sha1_prf_create(pseudo_random_function_t algo)
{
	private_sha1_prf_t *this;

	if (algo != PRF_KEYED_SHA1)
	{
		return NULL;
	}

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
		.hasher = (private_sha1_hasher_t*)sha1_hasher_create(HASH_SHA1),
	);

	return &this->public;
}
