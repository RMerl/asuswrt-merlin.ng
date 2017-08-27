/*
 * Copyright (C) 2013 Tobias Brunner
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

#include "iv_gen_rand.h"

#include <library.h>

typedef struct private_iv_gen_t private_iv_gen_t;

/**
 * Private data of an iv_gen_t object.
 */
struct private_iv_gen_t {

	/**
	 * Public iv_gen_t interface.
	 */
	iv_gen_t public;

	/**
	 * rng_t object
	 */
	rng_t *rng;
};

METHOD(iv_gen_t, get_iv, bool,
	private_iv_gen_t *this, u_int64_t seq, size_t size, u_int8_t *buffer)
{
	if (!this->rng)
	{
		return FALSE;
	}
	return this->rng->get_bytes(this->rng, size, buffer);
}

METHOD(iv_gen_t, allocate_iv, bool,
	private_iv_gen_t *this, u_int64_t seq, size_t size, chunk_t *chunk)
{
	if (!this->rng)
	{
		return FALSE;
	}
	return this->rng->allocate_bytes(this->rng, size, chunk);
}

METHOD(iv_gen_t, destroy, void,
	private_iv_gen_t *this)
{
	DESTROY_IF(this->rng);
	free(this);
}

iv_gen_t *iv_gen_rand_create()
{
	private_iv_gen_t *this;

	INIT(this,
		.public = {
			.get_iv = _get_iv,
			.allocate_iv = _allocate_iv,
			.destroy = _destroy,
		},
		.rng = lib->crypto->create_rng(lib->crypto, RNG_WEAK),
	);

	return &this->public;
}
