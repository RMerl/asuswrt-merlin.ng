/*
 * Copyright (C) 2013-2016 Andreas Steffen
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

#include "mgf1_xof.h"

#include "crypto/hashers/hasher.h"
#include "utils/debug.h"

typedef struct private_mgf1_xof_t private_mgf1_xof_t;

/**
 * Private data of an mgf1_xof_t object.
 */
struct private_mgf1_xof_t {

	/**
	 * Public mgf1_xof_t interface.
	 */
	mgf1_xof_t public;

	/**
	 * XOF type of the MGF1 Mask Generation Function
	 */
	ext_out_function_t type;

	/**
	 * Hasher the MGF1 Mask Generation Function is based on
	 */
	hasher_t *hasher;

	/**
	 * Is the seed hashed before using it as a seed for MGF1 ?
	 */
	bool hash_seed;

	/**
	 * Counter
	 */
	uint32_t counter;

	/**
	 * Set if counter has reached 2^32
	 */
	bool overflow;

	/**
	 * Current state to be hashed
	 */
	chunk_t state;

	/**
	 * Position of the 4 octet counter string
	 */
	uint8_t *ctr_str;

	/**
	 * Latest hash block
	 */
	uint8_t buf[HASH_SIZE_SHA512];

	/**
	 * Index pointing to the current position in the hash block
	 */
	size_t buf_index;

};

METHOD(xof_t, get_type, ext_out_function_t,
	private_mgf1_xof_t *this)
{
	return this->type;
}

static bool get_next_block(private_mgf1_xof_t *this, uint8_t *buffer)
{
	/* detect overflow, set counter string and increment counter */
	if (this->overflow)
	{
		DBG1(DBG_LIB, "MGF1 overflow occurred");
		return FALSE;
	}
	htoun32(this->ctr_str, this->counter++);
	if (this->counter == 0)
	{
		this->overflow = TRUE;
	}

	/* get the next block from the hash function */
	if (!this->hasher->get_hash(this->hasher, this->state, buffer))
	{
		return FALSE;
	}

	return TRUE;
}

METHOD(xof_t, get_bytes, bool,
	private_mgf1_xof_t *this, size_t out_len, uint8_t *buffer)
{
	size_t index = 0, blocks, len, hash_size;

	hash_size = this->hasher->get_hash_size(this->hasher);

	/* empty the current hash block buffer first */
	len = min(out_len, hash_size - this->buf_index);
	if (len)
	{
		memcpy(buffer, this->buf + this->buf_index, len);
		index += len;
		this->buf_index += len;
	}

	/* copy whole hash blocks directly to output buffer */
	blocks = (out_len - index) / hash_size;
	while (blocks--)
	{
		if (!get_next_block(this, buffer + index))
		{
			return FALSE;
		}
		index += hash_size;
	}

	/* get another hash block if some more output bytes are needed */
	len = out_len - index;
	if (len)
	{
		if (!get_next_block(this, this->buf))
		{
			return FALSE;
		}
		memcpy(buffer + index, this->buf, len);
		this->buf_index = len;
	}

	return TRUE;
}

METHOD(xof_t, allocate_bytes, bool,
	private_mgf1_xof_t *this, size_t out_len, chunk_t *chunk)
{
	*chunk = chunk_alloc(out_len);

	if (!get_bytes(this, out_len, chunk->ptr))
	{
		chunk_free(chunk);
		return FALSE;
	}

	return TRUE;
}

METHOD(xof_t, get_block_size, size_t,
	private_mgf1_xof_t *this)
{
	return this->hasher->get_hash_size(this->hasher);
}

METHOD(xof_t, get_seed_size, size_t,
	private_mgf1_xof_t *this)
{
	return this->hasher->get_hash_size(this->hasher);
}

METHOD(xof_t, set_seed, bool,
	private_mgf1_xof_t *this, chunk_t seed)
{
	size_t hash_size, state_len;

	if (seed.len == 0)
	{
		DBG1(DBG_LIB, "empty seed for MGF1");
		return FALSE;
	}

	/* determine state size and allocate space accordingly */
	hash_size = this->hasher->get_hash_size(this->hasher);
	state_len = (this->hash_seed ? hash_size : seed.len) + 4;
	chunk_clear(&this->state);
	this->state = chunk_alloc(state_len);

	/* hash block buffer is empty */
	this->buf_index = hash_size;

	/* reset counter */
	this->counter = 0;

	/* determine position of the 4 octet counter string */
	this->ctr_str = this->state.ptr + state_len - 4;

	if (this->hash_seed)
	{
		if (!this->hasher->get_hash(this->hasher, seed, this->state.ptr))
		{
			DBG1(DBG_LIB, "failed to hash seed for MGF1");
			return FALSE;
		}
	}
	else
	{
		memcpy(this->state.ptr, seed.ptr, seed.len);
	}

	return TRUE;
}

METHOD(xof_t, destroy, void,
	private_mgf1_xof_t *this)
{
	this->hasher->destroy(this->hasher);
	chunk_clear(&this->state);
	free(this);
}

METHOD(mgf1_t, set_hash_seed, void,
	private_mgf1_xof_t *this, bool yes)
{
	this->hash_seed = yes;
}

/*
 * Described in header.
 */
mgf1_xof_t *mgf1_xof_create(ext_out_function_t algorithm)
{
	private_mgf1_xof_t *this;
	hash_algorithm_t hash_alg;
	hasher_t *hasher;

	switch (algorithm)
	{
		case XOF_MGF1_SHA1:
			hash_alg = HASH_SHA1;
			break;
		case XOF_MGF1_SHA224:
			hash_alg = HASH_SHA224;
			break;
		case XOF_MGF1_SHA256:
			hash_alg = HASH_SHA256;
			break;
		case XOF_MGF1_SHA384:
			hash_alg = HASH_SHA384;
			break;
		case XOF_MGF1_SHA512:
			hash_alg = HASH_SHA512;
			break;
		default:
			return NULL;
	}

	hasher = lib->crypto->create_hasher(lib->crypto, hash_alg);
	if (!hasher)
	{
		DBG1(DBG_LIB, "failed to create %N hasher for MGF1",
			 hash_algorithm_names, hash_alg);
		return NULL;
	}

	INIT(this,
		.public = {
			.mgf1_interface = {
				.xof_interface = {
					.get_type = _get_type,
					.get_bytes = _get_bytes,
					.allocate_bytes = _allocate_bytes,
					.get_block_size = _get_block_size,
					.get_seed_size = _get_seed_size,
					.set_seed = _set_seed,
					.destroy = _destroy,
				},
				.set_hash_seed = _set_hash_seed,
			},
		},
		.type = algorithm,
		.hasher = hasher,
	);

	return &this->public;
}
