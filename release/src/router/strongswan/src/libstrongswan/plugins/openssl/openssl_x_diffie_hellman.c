/*
 * Copyright (C) 2018 Tobias Brunner
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

#include <openssl/evp.h>

/* basic support for X25519 was added with 1.1.0a, but we require features (e.g.
 * to load the keys) that were only added with 1.1.1 */
#if OPENSSL_VERSION_NUMBER >= 0x1010100fL && !defined(OPENSSL_NO_ECDH)

#include "openssl_x_diffie_hellman.h"
#include "openssl_util.h"

#include <utils/debug.h>

typedef struct private_key_exchange_t private_key_exchange_t;

/**
 * Private data
 */
struct private_key_exchange_t {
	/**
	 * Public interface.
	 */
	key_exchange_t public;

	/**
	 * Key exchange method.
	 */
	key_exchange_method_t ke;

	/**
	 * Private (public) key
	 */
	EVP_PKEY *key;

	/**
	 * Public key provided by peer
	 */
	EVP_PKEY *pub;

	/**
	 * Shared secret
	 */
	chunk_t shared_secret;
};

/**
 * Map a key exchange method to a key type
 */
static int map_key_type(key_exchange_method_t ke)
{
	switch (ke)
	{
		case CURVE_25519:
			return EVP_PKEY_X25519;
		case CURVE_448:
			return EVP_PKEY_X448;
		default:
			return 0;
	}
}

METHOD(key_exchange_t, set_public_key, bool,
	private_key_exchange_t *this, chunk_t value)
{
	if (!key_exchange_verify_pubkey(this->ke, value))
	{
		return FALSE;
	}

	EVP_PKEY_free(this->pub);
	this->pub = EVP_PKEY_new_raw_public_key(map_key_type(this->ke), NULL,
											value.ptr, value.len);
	if (!this->pub)
	{
		DBG1(DBG_LIB, "%N public value is malformed",
			 key_exchange_method_names, this->ke);
		return FALSE;
	}
	chunk_clear(&this->shared_secret);
	return TRUE;
}

METHOD(key_exchange_t, get_public_key, bool,
	private_key_exchange_t *this, chunk_t *value)
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

METHOD(key_exchange_t, set_private_key, bool,
	private_key_exchange_t *this, chunk_t value)
{
	EVP_PKEY_free(this->key);
	this->key = EVP_PKEY_new_raw_private_key(map_key_type(this->ke), NULL,
											 value.ptr, value.len);
	if (!this->key)
	{
		return FALSE;
	}
	return TRUE;
}

METHOD(key_exchange_t, get_shared_secret, bool,
	private_key_exchange_t *this, chunk_t *secret)
{
	if (!this->shared_secret.len &&
		!openssl_compute_shared_key(this->key, this->pub, &this->shared_secret))
	{
		DBG1(DBG_LIB, "%N shared secret computation failed",
			 key_exchange_method_names, this->ke);
		return FALSE;
	}
	*secret = chunk_clone(this->shared_secret);
	return TRUE;
}

METHOD(key_exchange_t, get_method, key_exchange_method_t,
	private_key_exchange_t *this)
{
	return this->ke;
}

METHOD(key_exchange_t, destroy, void,
	private_key_exchange_t *this)
{
	EVP_PKEY_free(this->key);
	EVP_PKEY_free(this->pub);
	chunk_clear(&this->shared_secret);
	free(this);
}

/*
 * Described in header
 */
key_exchange_t *openssl_x_diffie_hellman_create(key_exchange_method_t ke)
{
	private_key_exchange_t *this;
	EVP_PKEY_CTX *ctx = NULL;
	EVP_PKEY *key = NULL;

	switch (ke)
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
			 key_exchange_method_names, ke);
		EVP_PKEY_CTX_free(ctx);
		return NULL;
	}
	EVP_PKEY_CTX_free(ctx);

	INIT(this,
		.public = {
			.get_shared_secret = _get_shared_secret,
			.set_public_key = _set_public_key,
			.get_public_key = _get_public_key,
			.set_private_key = _set_private_key,
			.get_method = _get_method,
			.destroy = _destroy,
		},
		.ke = ke,
		.key = key,
	);
	return &this->public;
}

#endif /* OPENSSL_NO_ECDH */
