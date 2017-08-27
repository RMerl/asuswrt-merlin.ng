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

#include "test_rng.h"

typedef struct private_rng_t private_rng_t;

/**
 * Private data.
 */
struct private_rng_t {

	/**
	 * Public interface.
	 */
	rng_t public;

	/**
	 * Entropy string.
	 */
	chunk_t entropy;
};

METHOD(rng_t, get_bytes, bool,
	private_rng_t *this, size_t bytes, u_int8_t *buffer)
{
	if (bytes > this->entropy.len)
	{
		return FALSE;
	}
	memcpy(buffer, this->entropy.ptr, bytes);
	this->entropy = chunk_skip(this->entropy, bytes);
	return TRUE;
}

METHOD(rng_t, allocate_bytes, bool,
	private_rng_t *this, size_t bytes, chunk_t *chunk)
{
	if (bytes > this->entropy.len)
	{
		*chunk = chunk_empty;
		return FALSE;
	}

	*chunk = chunk_alloc(bytes);
	memcpy(chunk->ptr, this->entropy.ptr, bytes);
	this->entropy = chunk_skip(this->entropy, bytes);
	return TRUE;
}

METHOD(rng_t, destroy, void,
	private_rng_t *this)
{
	free(this);
}

/*
 * Described in header.
 */
rng_t *test_rng_create(chunk_t entropy)
{
	private_rng_t *this;

	INIT(this,
		.public = {
			.get_bytes = _get_bytes,
			.allocate_bytes = _allocate_bytes,
			.destroy = _destroy,
		},
		.entropy = entropy,
	);

	return &this->public;
}
