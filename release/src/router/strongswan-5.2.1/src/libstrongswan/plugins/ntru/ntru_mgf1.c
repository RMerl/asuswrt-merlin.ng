/*
 * Copyright (C) 2013 Andreas Steffen
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

#include "ntru_mgf1.h"

#include <crypto/hashers/hasher.h>
#include <utils/debug.h>
#include <utils/test.h>

typedef struct private_ntru_mgf1_t private_ntru_mgf1_t;

/**
 * Private data of an ntru_mgf1_t object.
 */
struct private_ntru_mgf1_t {

	/**
	 * Public ntru_mgf1_t interface.
	 */
	ntru_mgf1_t public;

	/**
	 * Hasher the MGF1 Mask Generation Function is based on
	 */
	hasher_t *hasher;

	/**
	 * Counter
	 */
	u_int32_t counter;

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
	u_char *ctr_str;

};

METHOD(ntru_mgf1_t, get_hash_size, size_t,
	private_ntru_mgf1_t *this)
{
	return this->hasher->get_hash_size(this->hasher);
}

METHOD(ntru_mgf1_t, get_mask, bool,
	private_ntru_mgf1_t *this, size_t mask_len, u_char *mask)
{
	u_char buf[HASH_SIZE_SHA512];
	size_t hash_len;

	hash_len = this->hasher->get_hash_size(this->hasher);

	while (mask_len > 0)
	{
		/* detect overflow, set counter string and increment counter */
		if (this->overflow)
		{
			return FALSE;
		}
		htoun32(this->ctr_str, this->counter++);
		if (this->counter == 0)
		{
			this->overflow = TRUE;
		}

		/* get the next or final mask block from the hash function */
		if (!this->hasher->get_hash(this->hasher, this->state,
								   (mask_len < hash_len) ? buf : mask))
		{
			return FALSE;
		}
		if (mask_len < hash_len)
		{
			memcpy(mask, buf, mask_len);
			return TRUE;
		}
		mask_len -= hash_len;
		mask += hash_len;
	}
	return TRUE;
}

METHOD(ntru_mgf1_t, allocate_mask, bool,
	private_ntru_mgf1_t *this, size_t mask_len, chunk_t *mask)
{
	if (mask_len == 0)
	{
		*mask = chunk_empty;
		return TRUE;
	}
	*mask = chunk_alloc(mask_len);

	return get_mask(this, mask_len, mask->ptr);
}

METHOD(ntru_mgf1_t, destroy, void,
	private_ntru_mgf1_t *this)
{
	this->hasher->destroy(this->hasher);
	chunk_clear(&this->state);
	free(this);
}

/*
 * Described in header.
 */
ntru_mgf1_t *ntru_mgf1_create(hash_algorithm_t alg, chunk_t seed,
							  bool hash_seed)
{
	private_ntru_mgf1_t *this;
	hasher_t *hasher;
	size_t state_len;

	if (seed.len == 0)
	{
		DBG1(DBG_LIB, "empty seed for MGF1");
		return NULL;
	}

	hasher = lib->crypto->create_hasher(lib->crypto, alg);
	if (!hasher)
	{
		DBG1(DBG_LIB, "failed to create %N hasher for MGF1",
			 hash_algorithm_names, alg);
		return NULL;
	}
	state_len = (hash_seed ? hasher->get_hash_size(hasher) : seed.len) + 4;
	
	INIT(this,
		.public = {
			.get_hash_size = _get_hash_size,
			.allocate_mask = _allocate_mask,
			.get_mask = _get_mask,
			.destroy = _destroy,
		},
		.hasher = hasher,
		.state = chunk_alloc(state_len),
	);

	/* determine position of the 4 octet counter string */
	this->ctr_str = this->state.ptr + state_len - 4;

	if (hash_seed)
	{
		if (!hasher->get_hash(hasher, seed, this->state.ptr))
		{
			DBG1(DBG_LIB, "failed to hash seed for MGF1");
			destroy(this);
			return NULL;
		}
	}
	else
	{
		memcpy(this->state.ptr, seed.ptr, seed.len);
	}

	return &this->public;
}

EXPORT_FUNCTION_FOR_TESTS(ntru, ntru_mgf1_create);
