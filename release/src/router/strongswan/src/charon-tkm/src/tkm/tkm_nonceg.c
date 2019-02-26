/*
 * Copyright (C) 2012 Reto Buerki
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
	 * Nonce chunk.
	 */
	chunk_t nonce;
};

METHOD(nonce_gen_t, get_nonce, bool,
	private_tkm_nonceg_t *this, size_t size, uint8_t *buffer)
{
	nonce_type nonce;
	uint64_t nc_id;

	nc_id = tkm->idmgr->acquire_id(tkm->idmgr, TKM_CTX_NONCE);
	if (!nc_id)
	{
		return FALSE;
	}

	if (ike_nc_create(nc_id, size, &nonce) != TKM_OK)
	{
		tkm->idmgr->release_id(tkm->idmgr, TKM_CTX_NONCE, nc_id);
		return FALSE;
	}

	memcpy(buffer, &nonce.data, size);
	this->nonce = chunk_clone(chunk_create(buffer, size));
	tkm->chunk_map->insert(tkm->chunk_map, &this->nonce, nc_id);
	return TRUE;
}

METHOD(nonce_gen_t, allocate_nonce, bool,
	private_tkm_nonceg_t *this, size_t size, chunk_t *chunk)
{
	*chunk = chunk_alloc(size);
	return get_nonce(this, chunk->len, chunk->ptr);
}

METHOD(nonce_gen_t, destroy, void,
	private_tkm_nonceg_t *this)
{
	uint64_t nc_id;

	nc_id = tkm->chunk_map->get_id(tkm->chunk_map, &this->nonce);
	if (nc_id)
	{
		DBG1(DBG_IKE, "resetting stale nonce context %llu", nc_id);

		if (ike_nc_reset(nc_id) != TKM_OK)
		{
			DBG1(DBG_IKE, "failed to reset nonce context %llu", nc_id);
		}
		tkm->idmgr->release_id(tkm->idmgr, TKM_CTX_NONCE, nc_id);
		tkm->chunk_map->remove(tkm->chunk_map, &this->nonce);
	}
	chunk_free(&this->nonce);
	free(this);
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
		},
	);

	return &this->public;
}
