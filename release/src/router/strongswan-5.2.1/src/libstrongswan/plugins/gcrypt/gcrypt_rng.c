/*
 * Copyright (C) 2009 Martin Willi
 * Hochschule fuer Technik Rapperswil
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

#include "gcrypt_rng.h"

#include <gcrypt.h>

typedef struct private_gcrypt_rng_t private_gcrypt_rng_t;

/**
 * Private data of an gcrypt_rng_t object.
 */
struct private_gcrypt_rng_t {

	/**
	 * Public gcrypt_rng_t interface.
	 */
	gcrypt_rng_t public;

	/**
	 * RNG quality of this instance
	 */
	rng_quality_t quality;
};

METHOD(rng_t, get_bytes, bool,
	private_gcrypt_rng_t *this, size_t bytes, u_int8_t *buffer)
{
	switch (this->quality)
	{
		case RNG_WEAK:
			gcry_create_nonce(buffer, bytes);
			break;
		case RNG_STRONG:
			gcry_randomize(buffer, bytes, GCRY_STRONG_RANDOM);
			break;
		case RNG_TRUE:
			gcry_randomize(buffer, bytes, GCRY_VERY_STRONG_RANDOM);
			break;
	}
	return TRUE;
}

METHOD(rng_t, allocate_bytes, bool,
	private_gcrypt_rng_t *this, size_t bytes, chunk_t *chunk)
{
	*chunk = chunk_alloc(bytes);
	get_bytes(this, chunk->len, chunk->ptr);
	return TRUE;
}

METHOD(rng_t, destroy, void,
	private_gcrypt_rng_t *this)
{
	free(this);
}

/*
 * Described in header.
 */
gcrypt_rng_t *gcrypt_rng_create(rng_quality_t quality)
{
	private_gcrypt_rng_t *this;

	switch (quality)
	{
		case RNG_WEAK:
		case RNG_STRONG:
		case RNG_TRUE:
			break;
		default:
			return NULL;
	}

	INIT(this,
		.public = {
			.rng = {
				.get_bytes = _get_bytes,
				.allocate_bytes = _allocate_bytes,
				.destroy = _destroy,
			},
		},
		.quality = quality,
	);

	return &this->public;
}

