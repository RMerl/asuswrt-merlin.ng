/*
 * Copyright (C) 2009 Martin Willi
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

#include "gcrypt_hasher.h"

#include <utils/debug.h>

#include <gcrypt.h>

typedef struct private_gcrypt_hasher_t private_gcrypt_hasher_t;

/**
 * Private data of gcrypt_hasher_t
 */
struct private_gcrypt_hasher_t {

	/**
	 * Public part of this class.
	 */
	gcrypt_hasher_t public;

	/**
	 * gcrypt hasher context
	 */
	gcry_md_hd_t hd;
};

METHOD(hasher_t, get_hash_size, size_t,
	private_gcrypt_hasher_t *this)
{
	return gcry_md_get_algo_dlen(gcry_md_get_algo(this->hd));
}

METHOD(hasher_t, reset, bool,
	private_gcrypt_hasher_t *this)
{
	gcry_md_reset(this->hd);
	return TRUE;
}

METHOD(hasher_t, get_hash, bool,
	private_gcrypt_hasher_t *this, chunk_t chunk, uint8_t *hash)
{
	gcry_md_write(this->hd, chunk.ptr, chunk.len);
	if (hash)
	{
		memcpy(hash, gcry_md_read(this->hd, 0), get_hash_size(this));
		gcry_md_reset(this->hd);
	}
	return TRUE;
}

METHOD(hasher_t, allocate_hash, bool,
	private_gcrypt_hasher_t *this, chunk_t chunk, chunk_t *hash)
{
	if (hash)
	{
		*hash = chunk_alloc(get_hash_size(this));
		return get_hash(this, chunk, hash->ptr);
	}
	return get_hash(this, chunk, NULL);
}

METHOD(hasher_t, destroy, void,
	private_gcrypt_hasher_t *this)
{
	gcry_md_close(this->hd);
	free(this);
}

/*
 * Described in header
 */
gcrypt_hasher_t *gcrypt_hasher_create(hash_algorithm_t algo)
{
	private_gcrypt_hasher_t *this;
	int gcrypt_alg;
	gcry_error_t err;

	switch (algo)
	{
		case HASH_MD2:
			gcrypt_alg = GCRY_MD_MD2;
			break;
		case HASH_MD4:
			gcrypt_alg = GCRY_MD_MD4;
			break;
		case HASH_MD5:
			gcrypt_alg = GCRY_MD_MD5;
			break;
		case HASH_SHA1:
			gcrypt_alg = GCRY_MD_SHA1;
			break;
		case HASH_SHA224:
			gcrypt_alg = GCRY_MD_SHA224;
			break;
		case HASH_SHA256:
			gcrypt_alg = GCRY_MD_SHA256;
			break;
		case HASH_SHA384:
			gcrypt_alg = GCRY_MD_SHA384;
			break;
		case HASH_SHA512:
			gcrypt_alg = GCRY_MD_SHA512;
			break;
		default:
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

	err = gcry_md_open(&this->hd, gcrypt_alg, 0);
	if (err)
	{
		DBG1(DBG_LIB, "grcy_md_open(%N) failed: %s",
			 hash_algorithm_names, algo, gpg_strerror(err));
		free(this);
		return NULL;
	}

	return &this->public;
}

