/*
 * Copyright (C) 2012 Tobias Brunner
 * Copyright (C) 2005-2006 Martin Willi
 * Copyright (C) 2005 Jan Hutter
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

#include "mac_prf.h"

typedef struct private_prf_t private_prf_t;

/**
 * Private data of a mac_prf_t object.
 */
struct private_prf_t {

	/**
	 * Public interface
	 */
	prf_t public;

	/**
	 * MAC to use
	 */
	mac_t *mac;
};

METHOD(prf_t, get_bytes, bool,
	private_prf_t *this, chunk_t seed, u_int8_t *buffer)
{
	return this->mac->get_mac(this->mac, seed, buffer);
}

METHOD(prf_t, allocate_bytes, bool,
	private_prf_t *this, chunk_t seed, chunk_t *chunk)
{
	if (chunk)
	{
		*chunk = chunk_alloc(this->mac->get_mac_size(this->mac));
		return this->mac->get_mac(this->mac, seed, chunk->ptr);
	}
	return this->mac->get_mac(this->mac, seed, NULL);
}

METHOD(prf_t, get_block_size, size_t,
	private_prf_t *this)
{
	return this->mac->get_mac_size(this->mac);
}

METHOD(prf_t, get_key_size, size_t,
	private_prf_t *this)
{
	/* IKEv2 uses MAC size as key size */
	return this->mac->get_mac_size(this->mac);
}

METHOD(prf_t, set_key, bool,
	private_prf_t *this, chunk_t key)
{
	return this->mac->set_key(this->mac, key);
}

METHOD(prf_t, destroy, void,
	private_prf_t *this)
{
	this->mac->destroy(this->mac);
	free(this);
}

/*
 * Described in header.
 */
prf_t *mac_prf_create(mac_t *mac)
{
	private_prf_t *this;

	INIT(this,
		.public = {
			.get_bytes = _get_bytes,
			.allocate_bytes = _allocate_bytes,
			.get_block_size = _get_block_size,
			.get_key_size = _get_key_size,
			.set_key = _set_key,
			.destroy = _destroy,
		},
		.mac = mac,
	);

	return &this->public;
}
