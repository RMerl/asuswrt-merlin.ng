/*
 * Copyright (C) 2015-2016 Andreas Steffen
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

#include "sha3_hasher.h"
#include "sha3_keccak.h"

typedef struct private_sha3_hasher_t private_sha3_hasher_t;

/**
 * Private data structure with hashing context for SHA-3
 */
struct private_sha3_hasher_t {

	/**
	 * Public interface for this hasher.
	 */
	sha3_hasher_t public;

	/**
	 * SHA-3 algorithm to be used
	 */
	hash_algorithm_t algorithm;

	/**
	 * SHA-3 Keccak state
	 */
	sha3_keccak_t *keccak;

};

METHOD(hasher_t, reset, bool,
	private_sha3_hasher_t *this)
{
	this->keccak->reset(this->keccak);
	return TRUE;
}

METHOD(hasher_t, get_hash_size, size_t,
	private_sha3_hasher_t *this)
{
	switch (this->algorithm)
	{
		case HASH_SHA3_224:
			return HASH_SIZE_SHA224;
		case HASH_SHA3_256:
			return HASH_SIZE_SHA256;
		case HASH_SHA3_384:
			return HASH_SIZE_SHA384;
		case HASH_SHA3_512:
			return HASH_SIZE_SHA512;
		default:
			return 0;
	}
}


METHOD(hasher_t, get_hash, bool,
	private_sha3_hasher_t *this, chunk_t chunk, uint8_t *buffer)
{
	this->keccak->absorb(this->keccak, chunk);

	if (buffer != NULL)
	{
		this->keccak->finalize(this->keccak);
		this->keccak->squeeze(this->keccak, get_hash_size(this), buffer);
		this->keccak->reset(this->keccak);
	}
	return TRUE;
}

METHOD(hasher_t, allocate_hash, bool,
	private_sha3_hasher_t *this, chunk_t chunk, chunk_t *hash)
{
	chunk_t allocated_hash;

	this->keccak->absorb(this->keccak, chunk);

	if (hash != NULL)
	{
		this->keccak->finalize(this->keccak);
		allocated_hash = chunk_alloc(get_hash_size(this));
		this->keccak->squeeze(this->keccak, allocated_hash.len,
											allocated_hash.ptr);
		this->keccak->reset(this->keccak);
		*hash = allocated_hash;
	}
	return TRUE;
}

METHOD(hasher_t, destroy, void,
	private_sha3_hasher_t *this)
{
	this->keccak->destroy(this->keccak);
	free(this);
}

/*
 * Described in header.
 */
sha3_hasher_t *sha3_hasher_create(hash_algorithm_t algorithm)
{
	private_sha3_hasher_t *this;

	switch (algorithm)
	{
		case HASH_SHA3_224:
		case HASH_SHA3_256:
		case HASH_SHA3_384:
		case HASH_SHA3_512:
			break;
		default:
			return NULL;
	}

	INIT(this,
		.public = {
			.hasher_interface = {
				.reset = _reset,
				.get_hash_size = _get_hash_size,
				.get_hash = _get_hash,
				.allocate_hash = _allocate_hash,
				.destroy = _destroy,
			},
		},
		.algorithm = algorithm,
	);

	this->keccak = sha3_keccak_create(2*get_hash_size(this), 0x06);
	if (!this->keccak)
	{
		free(this);
		return NULL;
	}

	return &this->public;
}
