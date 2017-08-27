/*
 * Copyright (C) 2011 Tobias Brunner
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

#include "pkcs11_rng.h"

#include <utils/debug.h>

#include "pkcs11_manager.h"

typedef struct private_pkcs11_rng_t private_pkcs11_rng_t;

/**
 * Private data of an pkcs11_rng_t object.
 */
struct private_pkcs11_rng_t {

	/**
	 * Public interface.
	 */
	pkcs11_rng_t public;

	/**
	 * PKCS#11 library
	 */
	pkcs11_library_t *lib;

	/**
	 * Mechanism for this rng
	 */
	CK_SESSION_HANDLE session;

};

METHOD(rng_t, get_bytes, bool,
	private_pkcs11_rng_t *this, size_t bytes, u_int8_t *buffer)
{
	CK_RV rv;
	rv = this->lib->f->C_GenerateRandom(this->session, buffer, bytes);
	if (rv != CKR_OK)
	{
		DBG1(DBG_CFG, "C_GenerateRandom() failed: %N", ck_rv_names, rv);
		return FALSE;
	}
	return TRUE;
}

METHOD(rng_t, allocate_bytes, bool,
	private_pkcs11_rng_t *this, size_t bytes, chunk_t *chunk)
{
	*chunk = chunk_alloc(bytes);
	if (!get_bytes(this, chunk->len, chunk->ptr))
	{
		chunk_clear(chunk);
		return FALSE;
	}
	return TRUE;
}

METHOD(rng_t, destroy, void,
	private_pkcs11_rng_t *this)
{
	this->lib->f->C_CloseSession(this->session);
	free(this);
}

/**
 * Find a token with its own RNG
 */
static pkcs11_library_t *find_token(CK_SESSION_HANDLE *session)
{
	enumerator_t *tokens;
	pkcs11_manager_t *manager;
	pkcs11_library_t *current, *found = NULL;
	CK_SLOT_ID slot;

	manager = lib->get(lib, "pkcs11-manager");
	if (!manager)
	{
		return NULL;
	}
	tokens = manager->create_token_enumerator(manager);
	while (tokens->enumerate(tokens, &current, &slot))
	{
		CK_TOKEN_INFO info;
		CK_RV rv;
		rv = current->f->C_GetTokenInfo(slot, &info);
		if (rv != CKR_OK)
		{
			continue;
		}
		if (info.flags & CKF_RNG)
		{
			if (current->f->C_OpenSession(slot, CKF_SERIAL_SESSION,
										  NULL, NULL, session) == CKR_OK)
			{
				found = current;
				break;
			}
		}
	}
	tokens->destroy(tokens);
	return found;
}

/*
 * Described in header.
 */
pkcs11_rng_t *pkcs11_rng_create(rng_quality_t quality)
{
	private_pkcs11_rng_t *this;

	INIT(this,
		.public = {
			.rng = {
				.get_bytes = _get_bytes,
				.allocate_bytes = _allocate_bytes,
				.destroy = _destroy,
			},
		},
	);

	this->lib = find_token(&this->session);
	if (!this->lib)
	{
		free(this);
		return NULL;
	}

	return &this->public;
}

