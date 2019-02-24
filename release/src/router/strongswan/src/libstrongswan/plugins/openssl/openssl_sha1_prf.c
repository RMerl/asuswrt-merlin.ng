/*
 * Copyright (C) 2010 Martin Willi
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

#include <openssl/opensslconf.h>

#ifndef OPENSSL_NO_SHA1

#include "openssl_sha1_prf.h"

#include <openssl/sha.h>
#include <crypto/hashers/hasher.h>

typedef struct private_openssl_sha1_prf_t private_openssl_sha1_prf_t;

/**
 * Private data of an openssl_sha1_prf_t object.
 */
struct private_openssl_sha1_prf_t {

	/**
	 * Public openssl_sha1_prf_t interface.
	 */
	openssl_sha1_prf_t public;

	/**
	 * SHA1 context
	 */
	SHA_CTX ctx;
};

METHOD(prf_t, get_bytes, bool,
	private_openssl_sha1_prf_t *this, chunk_t seed, uint8_t *bytes)
{
#if OPENSSL_VERSION_NUMBER >= 0x10000000L
	if (!SHA1_Update(&this->ctx, seed.ptr, seed.len))
	{
		return FALSE;
	}
#else /* OPENSSL_VERSION_NUMBER < 1.0 */
	SHA1_Update(&this->ctx, seed.ptr, seed.len);
#endif

	if (bytes)
	{
		uint32_t *hash = (uint32_t*)bytes;

		hash[0] = htonl(this->ctx.h0);
		hash[1] = htonl(this->ctx.h1);
		hash[2] = htonl(this->ctx.h2);
		hash[3] = htonl(this->ctx.h3);
		hash[4] = htonl(this->ctx.h4);
	}

	return TRUE;
}

METHOD(prf_t, get_block_size, size_t,
	private_openssl_sha1_prf_t *this)
{
	return HASH_SIZE_SHA1;
}

METHOD(prf_t, allocate_bytes, bool,
	private_openssl_sha1_prf_t *this, chunk_t seed, chunk_t *chunk)
{
	if (chunk)
	{
		*chunk = chunk_alloc(HASH_SIZE_SHA1);
		return get_bytes(this, seed, chunk->ptr);
	}
	return get_bytes(this, seed, NULL);
}

METHOD(prf_t, get_key_size, size_t,
	private_openssl_sha1_prf_t *this)
{
	return HASH_SIZE_SHA1;
}

METHOD(prf_t, set_key, bool,
	private_openssl_sha1_prf_t *this, chunk_t key)
{
#if OPENSSL_VERSION_NUMBER >= 0x10000000L
	if (!SHA1_Init(&this->ctx))
	{
		return FALSE;
	}
#else /* OPENSSL_VERSION_NUMBER < 1.0 */
	SHA1_Init(&this->ctx);
#endif

	if (key.len % 4)
	{
		return FALSE;
	}
	if (key.len >= 4)
	{
		this->ctx.h0 ^= untoh32(key.ptr);
	}
	if (key.len >= 8)
	{
		this->ctx.h1 ^= untoh32(key.ptr + 4);
	}
	if (key.len >= 12)
	{
		this->ctx.h2 ^= untoh32(key.ptr + 8);
	}
	if (key.len >= 16)
	{
		this->ctx.h3 ^= untoh32(key.ptr + 12);
	}
	if (key.len >= 20)
	{
		this->ctx.h4 ^= untoh32(key.ptr + 16);
	}
	return TRUE;
}

METHOD(prf_t, destroy, void,
	private_openssl_sha1_prf_t *this)
{
	free(this);
}

/**
 * See header
 */
openssl_sha1_prf_t *openssl_sha1_prf_create(pseudo_random_function_t algo)
{
	private_openssl_sha1_prf_t *this;

	if (algo != PRF_KEYED_SHA1)
	{
		return NULL;
	}

	INIT(this,
		.public = {
			.prf = {
				.get_block_size = _get_block_size,
				.get_bytes = _get_bytes,
				.allocate_bytes = _allocate_bytes,
				.get_key_size = _get_key_size,
				.set_key = _set_key,
				.destroy = _destroy,
			},
		},
	);

	return &this->public;
}

#endif /* OPENSSL_NO_SHA1 */
