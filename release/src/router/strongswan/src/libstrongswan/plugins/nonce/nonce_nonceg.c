/*
 * Copyright (C) 2012 Adrian-Ken Rueegsegger
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

#include "nonce_nonceg.h"

#include <utils/debug.h>

typedef struct private_nonce_nonceg_t private_nonce_nonceg_t;

/**
 * Private data of a nonce_nonceg_t object.
 */
struct private_nonce_nonceg_t {

	/**
	 * Public nonce_nonceg_t interface.
	 */
	nonce_nonceg_t public;

	/**
	 * Random number generator
	 */
	rng_t* rng;
};

METHOD(nonce_gen_t, get_nonce, bool,
	private_nonce_nonceg_t *this, size_t size, uint8_t *buffer)
{
	return this->rng->get_bytes(this->rng, size, buffer);
}

METHOD(nonce_gen_t, allocate_nonce, bool,
	private_nonce_nonceg_t *this, size_t size, chunk_t *chunk)
{
	return this->rng->allocate_bytes(this->rng, size, chunk);
}

METHOD(nonce_gen_t, destroy, void,
	private_nonce_nonceg_t *this)
{
	DESTROY_IF(this->rng);
	free(this);
}

/*
 * Described in header.
 */
nonce_nonceg_t *nonce_nonceg_create()
{
	private_nonce_nonceg_t *this;

	INIT(this,
		.public = {
			.nonce_gen = {
				.get_nonce = _get_nonce,
				.allocate_nonce = _allocate_nonce,
				.destroy = _destroy,
			},
		},
	);

	this->rng = lib->crypto->create_rng(lib->crypto, NONCE_RNG_QUALITY);
	if (!this->rng)
	{
		DBG1(DBG_LIB, "no RNG found for quality %N", rng_quality_names,
			 RNG_WEAK);
		destroy(this);
		return NULL;
	}

	return &this->public;
}
