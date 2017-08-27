/*
 * Copyright (C) 2008 Tobias Brunner
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

#include "openssl_hasher.h"

#include <openssl/evp.h>

typedef struct private_openssl_hasher_t private_openssl_hasher_t;

/**
 * Private data of openssl_hasher_t
 */
struct private_openssl_hasher_t {

	/**
	 * Public part of this class.
	 */
	openssl_hasher_t public;

	/**
	 * the hasher to use
	 */
	const EVP_MD *hasher;

	/**
	 * the current digest context
	 */
	EVP_MD_CTX *ctx;
};

METHOD(hasher_t, get_hash_size, size_t,
	private_openssl_hasher_t *this)
{
	return this->hasher->md_size;
}

METHOD(hasher_t, reset, bool,
	private_openssl_hasher_t *this)
{
	return EVP_DigestInit_ex(this->ctx, this->hasher, NULL) == 1;
}

METHOD(hasher_t, get_hash, bool,
	private_openssl_hasher_t *this, chunk_t chunk, u_int8_t *hash)
{
	if (EVP_DigestUpdate(this->ctx, chunk.ptr, chunk.len) != 1)
	{
		return FALSE;
	}
	if (hash)
	{
		if (EVP_DigestFinal_ex(this->ctx, hash, NULL) != 1)
		{
			return FALSE;
		}
		return reset(this);
	}
	return TRUE;
}

METHOD(hasher_t, allocate_hash, bool,
	private_openssl_hasher_t *this, chunk_t chunk, chunk_t *hash)
{
	if (hash)
	{
		*hash = chunk_alloc(get_hash_size(this));
		return get_hash(this, chunk, hash->ptr);
	}
	return get_hash(this, chunk, NULL);
}

METHOD(hasher_t, destroy, void,
	private_openssl_hasher_t *this)
{
	EVP_MD_CTX_destroy(this->ctx);
	free(this);
}

/*
 * Described in header
 */
openssl_hasher_t *openssl_hasher_create(hash_algorithm_t algo)
{
	private_openssl_hasher_t *this;
	char* name;

	name = enum_to_name(hash_algorithm_short_names, algo);
	if (!name)
	{
		return NULL;
	}

	INIT(this,
		.public = {
			.hasher = {
				.get_hash = _get_hash,
				.allocate_hash = _allocate_hash,
				.get_hash_size = _get_hash_size,
				.reset = _reset,
				.destroy = _destroy,
			},
		},
	);

	this->hasher = EVP_get_digestbyname(name);
	if (!this->hasher)
	{
		/* OpenSSL does not support the requested algo */
		free(this);
		return NULL;
	}

	this->ctx = EVP_MD_CTX_create();

	/* initialization */
	if (!reset(this))
	{
		destroy(this);
		return NULL;
	}

	return &this->public;
}
