/*
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2010 revosec AG
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

#include "pkcs11_hasher.h"

#include <unistd.h>

#include <utils/debug.h>
#include <threading/mutex.h>

#include "pkcs11_manager.h"

typedef struct private_pkcs11_hasher_t private_pkcs11_hasher_t;

/**
 * Private data of an pkcs11_hasher_t object.
 */
struct private_pkcs11_hasher_t {

	/**
	 * Public pkcs11_hasher_t interface.
	 */
	pkcs11_hasher_t public;

	/**
	 * PKCS#11 library
	 */
	pkcs11_library_t *lib;

	/**
	 * Mechanism for this hasher
	 */
	CK_MECHANISM_PTR mech;

	/**
	 * Token session
	 */
	CK_SESSION_HANDLE session;

	/**
	 * size of the hash
	 */
	size_t size;

	/**
	 * Mutex to lock the tokens hashing engine
	 */
	mutex_t *mutex;

	/**
	 * do we have an initialized state?
	 */
	bool have_state;

	/**
	 * state buffer
	 */
	CK_BYTE_PTR state;

	/**
	 * Length of the state buffer
	 */
	CK_ULONG state_len;
};

METHOD(hasher_t, get_hash_size, size_t,
	private_pkcs11_hasher_t *this)
{
	return this->size;
}

/**
 * Save the Operation state to host memory
 */
static bool save_state(private_pkcs11_hasher_t *this)
{
	CK_RV rv;

	while (TRUE)
	{
		if (!this->state)
		{
			rv = this->lib->f->C_GetOperationState(this->session, NULL,
												   &this->state_len);
			if (rv != CKR_OK)
			{
				break;
			}
			this->state = malloc(this->state_len);
		}
		rv = this->lib->f->C_GetOperationState(this->session, this->state,
											   &this->state_len);
		switch (rv)
		{
			case CKR_BUFFER_TOO_SMALL:
				free(this->state);
				this->state = NULL;
				continue;
			case CKR_OK:
				this->have_state = TRUE;
				return TRUE;
			default:
				break;
		}
		break;
	}
	DBG1(DBG_CFG, "C_GetOperationState() failed: %N", ck_rv_names, rv);
	return FALSE;
}

/**
 * Load the Operation state from host memory
 */
static bool load_state(private_pkcs11_hasher_t *this)
{
	CK_RV rv;

	rv = this->lib->f->C_SetOperationState(this->session, this->state,
						this->state_len, CK_INVALID_HANDLE, CK_INVALID_HANDLE);
	if (rv != CKR_OK)
	{
		DBG1(DBG_CFG, "C_SetOperationState() failed: %N", ck_rv_names, rv);
		return FALSE;
	}
	this->have_state = FALSE;
	return TRUE;
}

METHOD(hasher_t, reset, bool,
	private_pkcs11_hasher_t *this)
{
	this->have_state = FALSE;
	return TRUE;
}

METHOD(hasher_t, get_hash, bool,
	private_pkcs11_hasher_t *this, chunk_t chunk, uint8_t *hash)
{
	CK_RV rv;
	CK_ULONG len;

	this->mutex->lock(this->mutex);
	if (this->have_state)
	{
		if (!load_state(this))
		{
			this->mutex->unlock(this->mutex);
			return FALSE;
		}
	}
	else
	{
		rv = this->lib->f->C_DigestInit(this->session, this->mech);
		if (rv != CKR_OK)
		{
			DBG1(DBG_CFG, "C_DigestInit() failed: %N", ck_rv_names, rv);
			this->mutex->unlock(this->mutex);
			return FALSE;
		}
	}
	if (chunk.len)
	{
		rv = this->lib->f->C_DigestUpdate(this->session, chunk.ptr, chunk.len);
		if (rv != CKR_OK)
		{
			DBG1(DBG_CFG, "C_DigestUpdate() failed: %N", ck_rv_names, rv);
			this->mutex->unlock(this->mutex);
			return FALSE;
		}
	}
	if (hash)
	{
		len = this->size;
		rv = this->lib->f->C_DigestFinal(this->session,
								hash, &len);
		if (rv != CKR_OK)
		{
			DBG1(DBG_CFG, "C_DigestFinal() failed: %N", ck_rv_names, rv);
			this->mutex->unlock(this->mutex);
			return FALSE;
		}
	}
	else
	{
		if (!save_state(this))
		{
			this->mutex->unlock(this->mutex);
			return FALSE;
		}
	}
	this->mutex->unlock(this->mutex);
	return TRUE;
}

METHOD(hasher_t, allocate_hash, bool,
	private_pkcs11_hasher_t *this, chunk_t chunk, chunk_t *hash)
{
	if (hash)
	{
		*hash = chunk_alloc(this->size);
		return get_hash(this, chunk, hash->ptr);
	}
	return get_hash(this, chunk, NULL);
}

METHOD(hasher_t, destroy, void,
	private_pkcs11_hasher_t *this)
{
	this->lib->f->C_CloseSession(this->session);
	this->mutex->destroy(this->mutex);
	free(this);
}

/**
 * Get the Cryptoki mechanism for a hash algorithm
 */
static CK_MECHANISM_PTR algo_to_mechanism(hash_algorithm_t algo, size_t *size)
{
	static struct {
		hash_algorithm_t algo;
		CK_MECHANISM mechanism;
		size_t size;
	} mappings[] = {
		{HASH_MD2,		{CKM_MD2,		NULL, 0},	HASH_SIZE_MD2},
		{HASH_MD5,		{CKM_MD5,		NULL, 0},	HASH_SIZE_MD5},
		{HASH_SHA1,		{CKM_SHA_1,		NULL, 0},	HASH_SIZE_SHA1},
		{HASH_SHA256,	{CKM_SHA256,	NULL, 0},	HASH_SIZE_SHA256},
		{HASH_SHA384,	{CKM_SHA384,	NULL, 0},	HASH_SIZE_SHA384},
		{HASH_SHA512,	{CKM_SHA512,	NULL, 0},	HASH_SIZE_SHA512},
	};
	int i;

	for (i = 0; i < countof(mappings); i++)
	{
		if (mappings[i].algo == algo)
		{
			*size = mappings[i].size;
			return &mappings[i].mechanism;
		}
	}
	return NULL;
}

/**
 * Find a token we can use for a hash algorithm
 */
static pkcs11_library_t* find_token(hash_algorithm_t algo,
			CK_SESSION_HANDLE *session, CK_MECHANISM_PTR *mout, size_t *size)
{
	enumerator_t *tokens, *mechs;
	pkcs11_manager_t *manager;
	pkcs11_library_t *current, *found = NULL;
	CK_MECHANISM_TYPE type;
	CK_MECHANISM_PTR mech;
	CK_SLOT_ID slot;

	mech = algo_to_mechanism(algo, size);
	if (!mech)
	{
		return NULL;
	}
	manager = lib->get(lib, "pkcs11-manager");
	if (!manager)
	{
		return NULL;
	}
	tokens = manager->create_token_enumerator(manager);
	while (tokens->enumerate(tokens, &current, &slot))
	{
		mechs = current->create_mechanism_enumerator(current, slot);
		while (mechs->enumerate(mechs, &type, NULL))
		{
			if (type == mech->mechanism)
			{
				if (current->f->C_OpenSession(slot, CKF_SERIAL_SESSION,
											  NULL, NULL, session) == CKR_OK)
				{
					found = current;
					*mout = mech;
					break;
				}
			}
		}
		mechs->destroy(mechs);
		if (found)
		{
			break;
		}
	}
	tokens->destroy(tokens);
	return found;
}

/**
 * See header
 */
pkcs11_hasher_t *pkcs11_hasher_create(hash_algorithm_t algo)
{
	private_pkcs11_hasher_t *this;

	INIT(this,
		.public = {
			.hasher = {
				.get_hash_size = _get_hash_size,
				.reset = _reset,
				.get_hash = _get_hash,
				.allocate_hash = _allocate_hash,
				.destroy = _destroy,
			},
		},
		.mutex = mutex_create(MUTEX_TYPE_DEFAULT),
	);

	this->lib = find_token(algo, &this->session, &this->mech, &this->size);
	if (!this->lib)
	{
		this->mutex->destroy(this->mutex);
		free(this);
		return NULL;
	}

	return &this->public;
}
