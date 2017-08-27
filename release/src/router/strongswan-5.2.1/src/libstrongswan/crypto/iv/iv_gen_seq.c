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

#include "iv_gen_seq.h"

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
	 * Salt to mask counter
	 */
	u_int8_t *salt;
};

METHOD(iv_gen_t, get_iv, bool,
	private_iv_gen_t *this, u_int64_t seq, size_t size, u_int8_t *buffer)
{
	u_int8_t iv[sizeof(u_int64_t)];
	size_t len = size;

	if (!this->salt)
	{
		return FALSE;
	}
	if (len > sizeof(u_int64_t))
	{
		len = sizeof(u_int64_t);
		memset(buffer, 0, size - len);
	}
	htoun64(iv, seq);
	memxor(iv, this->salt, sizeof(u_int64_t));
	memcpy(buffer + size - len, iv + sizeof(u_int64_t) - len, len);
	return TRUE;
}

METHOD(iv_gen_t, allocate_iv, bool,
	private_iv_gen_t *this, u_int64_t seq, size_t size, chunk_t *chunk)
{
	*chunk = chunk_alloc(size);
	if (!get_iv(this, seq, chunk->len, chunk->ptr))
	{
		chunk_free(chunk);
		return FALSE;
	}
	return TRUE;
}

METHOD(iv_gen_t, destroy, void,
	private_iv_gen_t *this)
{
	free(this->salt);
	free(this);
}

iv_gen_t *iv_gen_seq_create()
{
	private_iv_gen_t *this;
	rng_t *rng;

	INIT(this,
		.public = {
			.get_iv = _get_iv,
			.allocate_iv = _allocate_iv,
			.destroy = _destroy,
		},
	);

	rng = lib->crypto->create_rng(lib->crypto, RNG_STRONG);
	if (rng)
	{
		this->salt = malloc(sizeof(u_int64_t));
		if (!rng->get_bytes(rng, sizeof(u_int64_t), this->salt))
		{
			free(this->salt);
			this->salt = NULL;
		}
		rng->destroy(rng);
	}

	return &this->public;
}
