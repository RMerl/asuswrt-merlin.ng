/*
 * Copyright (C) 2012 Tobias Brunner
 * Copyright (C) 2005-2008 Martin Willi
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

#include "mac_signer.h"

typedef struct private_signer_t private_signer_t;

/**
 * Private data of a mac_signer_t object.
 */
struct private_signer_t {

	/**
	 * Public interface
	 */
	signer_t public;

	/**
	 * MAC to use
	 */
	mac_t *mac;

	/**
	 * Truncation of MAC output
	 */
	size_t truncation;
};

METHOD(signer_t, get_signature, bool,
	private_signer_t *this, chunk_t data, u_int8_t *buffer)
{
	if (buffer)
	{
		u_int8_t mac[this->mac->get_mac_size(this->mac)];

		if (!this->mac->get_mac(this->mac, data, mac))
		{
			return FALSE;
		}
		memcpy(buffer, mac, this->truncation);
		return TRUE;
	}
	return this->mac->get_mac(this->mac, data, NULL);
}

METHOD(signer_t, allocate_signature, bool,
	private_signer_t *this, chunk_t data, chunk_t *chunk)
{
	if (chunk)
	{
		u_int8_t mac[this->mac->get_mac_size(this->mac)];

		if (!this->mac->get_mac(this->mac, data, mac))
		{
			return FALSE;
		}
		*chunk = chunk_alloc(this->truncation);
		memcpy(chunk->ptr, mac, this->truncation);
		return TRUE;
	}
	return this->mac->get_mac(this->mac, data, NULL);
}

METHOD(signer_t, verify_signature, bool,
	private_signer_t *this, chunk_t data, chunk_t signature)
{
	u_int8_t mac[this->mac->get_mac_size(this->mac)];

	if (signature.len != this->truncation)
	{
		return FALSE;
	}
	return this->mac->get_mac(this->mac, data, mac) &&
		   memeq(signature.ptr, mac, this->truncation);
}

METHOD(signer_t, get_key_size, size_t,
	private_signer_t *this)
{
	return this->mac->get_mac_size(this->mac);
}

METHOD(signer_t, get_block_size, size_t,
	private_signer_t *this)
{
	return this->truncation;
}

METHOD(signer_t, set_key, bool,
	private_signer_t *this, chunk_t key)
{
	return this->mac->set_key(this->mac, key);
}

METHOD(signer_t, destroy, void,
	private_signer_t *this)
{
	this->mac->destroy(this->mac);
	free(this);
}

/*
 * Described in header
 */
signer_t *mac_signer_create(mac_t *mac, size_t len)
{
	private_signer_t *this;

	INIT(this,
		.public = {
			.get_signature = _get_signature,
			.allocate_signature = _allocate_signature,
			.verify_signature = _verify_signature,
			.get_block_size = _get_block_size,
			.get_key_size = _get_key_size,
			.set_key = _set_key,
			.destroy = _destroy,
		},
		.truncation = min(len, mac->get_mac_size(mac)),
		.mac = mac,
	);

	return &this->public;
}

