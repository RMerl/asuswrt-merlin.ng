/*
 * Copyright (C) 2018 Tobias Brunner
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

#include <openssl/evp.h>

/* basic support for X25519 was added with 1.1.0a, but we require features (e.g.
 * to load the keys) that were only added with 1.1.1 */
#if OPENSSL_VERSION_NUMBER >= 0x1010100fL && !defined(OPENSSL_NO_ECDH)

#include "openssl_x_diffie_hellman.h"

#include <utils/debug.h>

typedef struct private_diffie_hellman_t private_diffie_hellman_t;

/**
 * Private data
 */
struct private_diffie_hellman_t {
	/**
	 * Public interface.
	 */
	diffie_hellman_t public;

	/**
	 * Diffie Hellman group number.
	 */
	diffie_hellman_group_t group;

	/**
	 * Private (public) key
	 */
	EVP_PKEY *key;

	/**
	 * Shared secret
	 */
	chunk_t shared_secret;

	/**
	 * True if shared secret is computed
	 */
	bool computed;
};

/**
 * Map a DH group to a key type
 */
static int map_key_type(diffie_hellman_group_t group)
{
	switch (group)
	{
		case CURVE_25519:
			return EVP_PKEY_X25519;
		case CURVE_448:
			return EVP_PKEY_X448;
		default:
			return 0;
	}
}

/**
 * Compute the shared secret
 */
static bool compute_shared_key(private_diffie_hellman_t *this, EVP_PKEY *pub,
							   chunk_t *shared_secret)
{
	EVP_PKEY_CTX *ctx;
	bool success = FALSE;

	ctx = EVP_PKEY_CTX_new(this->key, NULL);
	if (!ctx)
	{
		return FALSE;
	}

	if (EVP_PKEY_derive_init(ctx) <= 0)
	{
		goto error;
	}

	if (EVP_PKEY_derive_set_peer(ctx, pub) <= 0)
	{
		goto error;
	}

	if (EVP_PKEY_derive(ctx, NULL, &shared_secret->len) <= 0)
	{
		goto error;
	}

	*shared_secret = chunk_alloc(shared_secret->len);

	if (EVP_PKEY_derive(ctx, shared_secret->ptr, &shared_secret->len) <= 0)
	{
		goto error;
	}

	success = TRUE;

error:
	EVP_PKEY_CTX_free(ctx);
	return success;
}

METHOD(diffie_hellman_t, set_other_public_value, bool,
	private_diffie_hellman_t *this, chunk_t value)
{
	EVP_PKEY *pub;

	if (!diffie_hellman_verify_value(this->group, value))
	{
		return FALSE;
	}

	pub =  EVP_PKEY_new_raw_public_key(map_key_type(this->group), NULL,
                                       value.ptr, value.len);
	if (!pub)
	{
		DBG1(DBG_LIB, "%N public value is malformed",
			 diffie_hellman_group_names, this->group);
		return FALSE;
	}

	chunk_clear(&this->shared_secret);

	if (!compute_shared_key(this, pub, &this->shared_secret))
	{
		DBG1(DBG_LIB, "%N shared secret computation failed",
			 diffie_hellman_group_names, this->group);
		EVP_PKEY_free(pub);
		return FALSE;
	}
	this->computed = TRUE;
	EVP_PKEY_free(pub);
	return TRUE;
}

METHOD(diffie_hellman_t, get_my_public_value, bool,
	private_diffie_hellman_t *this, chunk_t *value)
{
	size_t len;

	if (!EVP_PKEY_get_raw_public_key(this->key, NULL, &len))
	{
		return FALSE;
	}

	*value = chunk_alloc(len);

	if (!EVP_PKEY_get_raw_public_key(this->key, value->ptr, &value->len))
	{
		chunk_free(value);
		return FALSE;
	}
	return TRUE;
}

METHOD(diffie_hellman_t, set_private_value, bool,
	private_diffie_hellman_t *this, chunk_t value)
{
	EVP_PKEY_free(this->key);
	this->key = EVP_PKEY_new_raw_private_key(map_key_type(this->group), NULL,
											 value.ptr, value.len);
	if (!this->key)
	{
		return FALSE;
	}
	return TRUE;
}

METHOD(diffie_hellman_t, get_shared_secret, bool,
	private_diffie_hellman_t *this, chunk_t *secret)
{
	if (!this->computed)
	{
		return FALSE;
	}
	*secret = chunk_clone(this->shared_secret);
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
	EVP_PKEY_free(this->key);
	chunk_clear(&this->shared_secret);
	free(this);
}

/*
 * Described in header
 */
diffie_hellman_t *openssl_x_diffie_hellman_create(diffie_hellman_group_t group)
{
	private_diffie_hellman_t *this;
	EVP_PKEY_CTX *ctx = NULL;
	EVP_PKEY *key = NULL;

	switch (group)
	{
		case CURVE_25519:
			ctx = EVP_PKEY_CTX_new_id(NID_X25519, NULL);
			break;
		case CURVE_448:
			ctx = EVP_PKEY_CTX_new_id(NID_X448, NULL);
			break;
		default:
			break;
	}

	if (!ctx ||
		EVP_PKEY_keygen_init(ctx) <= 0 ||
		EVP_PKEY_keygen(ctx, &key) <= 0)
	{
		DBG1(DBG_LIB, "generating key for %N failed",
			 diffie_hellman_group_names, group);
		EVP_PKEY_CTX_free(ctx);
		return NULL;
	}
	EVP_PKEY_CTX_free(ctx);

	INIT(this,
		.public = {
			.get_shared_secret = _get_shared_secret,
			.set_other_public_value = _set_other_public_value,
			.get_my_public_value = _get_my_public_value,
			.set_private_value = _set_private_value,
			.get_dh_group = _get_dh_group,
			.destroy = _destroy,
		},
		.group = group,
		.key = key,
	);
	return &this->public;
}

#endif /* OPENSSL_NO_ECDH */
