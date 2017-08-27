/*
 * Copyrigth (C) 2012 Reto Buerki
 * Copyright (C) 2012 Adrian-Ken Rueegsegger
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

#include <tkm/client.h>
#include <tkm/constants.h>

#include "tkm.h"
#include "tkm_nonceg.h"

typedef struct private_tkm_nonceg_t private_tkm_nonceg_t;

/**
 * Private data of a tkm_nonceg_t object.
 */
struct private_tkm_nonceg_t {

	/**
	 * Public tkm_nonceg_t interface.
	 */
	tkm_nonceg_t public;

	/**
	 * Context id.
	 */
	nc_id_type context_id;

};

METHOD(nonce_gen_t, get_nonce, bool,
	private_tkm_nonceg_t *this, size_t size, u_int8_t *buffer)
{
	nonce_type nonce;

	if (ike_nc_create(this->context_id, size, &nonce) != TKM_OK)
	{
		return FALSE;
	}

	memcpy(buffer, &nonce.data, size);
	return TRUE;
}

METHOD(nonce_gen_t, allocate_nonce, bool,
	private_tkm_nonceg_t *this, size_t size, chunk_t *chunk)
{
	*chunk = chunk_alloc(size);
	if (get_nonce(this, chunk->len, chunk->ptr))
	{
		tkm->chunk_map->insert(tkm->chunk_map, chunk, this->context_id);
		return TRUE;
	}
	return FALSE;
}

METHOD(nonce_gen_t, destroy, void,
	private_tkm_nonceg_t *this)
{
	free(this);
}

METHOD(tkm_nonceg_t, get_id, nc_id_type,
	private_tkm_nonceg_t *this)
{
	return this->context_id;
}

/*
 * Described in header.
 */
tkm_nonceg_t *tkm_nonceg_create()
{
	private_tkm_nonceg_t *this;

	INIT(this,
		.public = {
			.nonce_gen = {
				.get_nonce = _get_nonce,
				.allocate_nonce = _allocate_nonce,
				.destroy = _destroy,
			},
			.get_id = _get_id,
		},
		.context_id = tkm->idmgr->acquire_id(tkm->idmgr, TKM_CTX_NONCE),
	);

	if (!this->context_id)
	{
		free(this);
		return NULL;
	}

	return &this->public;
}
