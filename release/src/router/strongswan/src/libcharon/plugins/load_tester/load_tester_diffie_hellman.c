/*
 * Copyright (C) 2008 Martin Willi
 *
 * Copyright (C) secunet Security Networks AG
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

#include "load_tester_diffie_hellman.h"

METHOD(key_exchange_t, get_public_key, bool,
	load_tester_diffie_hellman_t *this, chunk_t *value)
{
	*value = chunk_empty;
	return TRUE;
}

METHOD(key_exchange_t, set_public_key, bool,
	load_tester_diffie_hellman_t *this, chunk_t value)
{
	return TRUE;
}

METHOD(key_exchange_t, get_shared_secret, bool,
	load_tester_diffie_hellman_t *this, chunk_t *secret)
{
	*secret = chunk_empty;
	return TRUE;
}

METHOD(key_exchange_t, get_method, key_exchange_method_t,
	load_tester_diffie_hellman_t *this)
{
	return MODP_NULL;
}

METHOD(key_exchange_t, destroy, void,
	load_tester_diffie_hellman_t *this)
{
	free(this);
}

/**
 * See header
 */
load_tester_diffie_hellman_t *load_tester_diffie_hellman_create(
												key_exchange_method_t group)
{
	load_tester_diffie_hellman_t *this;

	if (group != MODP_NULL)
	{
		return NULL;
	}

	INIT(this,
		.ke = {
			.get_shared_secret = _get_shared_secret,
			.set_public_key = _set_public_key,
			.get_public_key = _get_public_key,
			.get_method = _get_method,
			.destroy = _destroy,
		}
	);

	return this;
}
