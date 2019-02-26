/*
 * Copyright (C) 2015 Tobias Brunner
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

#include "iv_gen_null.h"

typedef struct private_iv_gen_t private_iv_gen_t;

/**
 * Private data of an iv_gen_t object.
 */
struct private_iv_gen_t {

	/**
	 * Public iv_gen_t interface.
	 */
	iv_gen_t public;
};

METHOD(iv_gen_t, get_iv, bool,
	private_iv_gen_t *this, uint64_t seq, size_t size, uint8_t *buffer)
{
	return size == 0;
}

METHOD(iv_gen_t, allocate_iv, bool,
	private_iv_gen_t *this, uint64_t seq, size_t size, chunk_t *chunk)
{
	*chunk = chunk_empty;
	return size == 0;
}

METHOD(iv_gen_t, destroy, void,
	private_iv_gen_t *this)
{
	free(this);
}

iv_gen_t *iv_gen_null_create()
{
	private_iv_gen_t *this;

	INIT(this,
		.public = {
			.get_iv = _get_iv,
			.allocate_iv = _allocate_iv,
			.destroy = _destroy,
		},
	);

	return &this->public;
}
