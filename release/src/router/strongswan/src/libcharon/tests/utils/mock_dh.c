/*
 * Copyright (C) 2016 Tobias Brunner
 * Copyright (C) 2008 Martin Willi
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

#include "mock_dh.h"

typedef struct private_diffie_hellman_t private_diffie_hellman_t;

/** Mock DH public and shared key */
static chunk_t mock_key = chunk_from_chars(
									0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,
									0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,
									0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,
									0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08);

/**
 * Private data
 */
struct private_diffie_hellman_t {

	/**
	 * Public interface
	 */
	diffie_hellman_t public;

	/**
	 * Instantiated DH group
	 */
	diffie_hellman_group_t group;
};

METHOD(diffie_hellman_t, get_my_public_value, bool,
	private_diffie_hellman_t *this, chunk_t *value)
{
	*value = chunk_clone(mock_key);
	return TRUE;
}

METHOD(diffie_hellman_t, set_other_public_value, bool,
	private_diffie_hellman_t *this, chunk_t value)
{
	return TRUE;
}

METHOD(diffie_hellman_t, get_shared_secret, bool,
	private_diffie_hellman_t *this, chunk_t *secret)
{
	*secret = chunk_clone(mock_key);
	return TRUE;
}

METHOD(diffie_hellman_t, get_dh_group, diffie_hellman_group_t,
	private_diffie_hellman_t *this)
{
	return this->group;
}

METHOD(diffie_hellman_t, destroy, void,
	private_diffie_hellman_t *this)
{
	free(this);
}

/**
 * See header
 */
diffie_hellman_t *mock_dh_create(diffie_hellman_group_t group)
{
	private_diffie_hellman_t *this;

	INIT(this,
		.public = {
			.get_shared_secret = _get_shared_secret,
			.set_other_public_value = _set_other_public_value,
			.get_my_public_value = _get_my_public_value,
			.get_dh_group = _get_dh_group,
			.destroy = _destroy,
		},
		.group = group,
	);
	return &this->public;
}
