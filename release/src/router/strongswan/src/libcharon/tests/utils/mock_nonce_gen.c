/*
 * Copyright (C) 2016 Tobias Brunner
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

#include "mock_nonce_gen.h"

typedef struct private_nonce_gen_t private_nonce_gen_t;

struct private_nonce_gen_t {

	/**
	 * Public interface
	 */
	nonce_gen_t public;

	/**
	 * Random number generator
	 */
	rng_t* rng;

	/**
	 * First byte to set to the nonces
	 */
	u_char first;
};

METHOD(nonce_gen_t, get_nonce, bool,
	private_nonce_gen_t *this, size_t size, uint8_t *buffer)
{
	if (size > 0)
	{
		buffer[0] = this->first;
		buffer++;
		size--;
	}
	return this->rng->get_bytes(this->rng, size, buffer);
}

METHOD(nonce_gen_t, allocate_nonce, bool,
	private_nonce_gen_t *this, size_t size, chunk_t *chunk)
{
	*chunk = chunk_alloc(size);
	if (!get_nonce(this, chunk->len, chunk->ptr))
	{
		chunk_free(chunk);
		return FALSE;
	}
	return TRUE;
}

METHOD(nonce_gen_t, destroy, void,
	private_nonce_gen_t *this)
{
	DESTROY_IF(this->rng);
	free(this);
}

/*
 * Described in header
 */
nonce_gen_t *mock_nonce_gen_create(u_char first)
{
	private_nonce_gen_t *this;

	INIT(this,
		.public = {
			.get_nonce = _get_nonce,
			.allocate_nonce = _allocate_nonce,
			.destroy = _destroy,
		},
		.rng = lib->crypto->create_rng(lib->crypto, RNG_WEAK),
		.first = first,
	);
	if (!this->rng)
	{
		destroy(this);
		return NULL;
	}
	return &this->public;
}
