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

#include "tls_prf.h"

typedef struct private_tls_prf12_t private_tls_prf12_t;

#include <library.h>

/**
 * Private data of an tls_prf_t object.
 */
struct private_tls_prf12_t {

	/**
	 * Public tls_prf_t interface.
	 */
	tls_prf_t public;

	/**
	 * Underlying primitive PRF
	 */
	prf_t *prf;
};

METHOD(tls_prf_t, set_key12, bool,
	private_tls_prf12_t *this, chunk_t key)
{
	return this->prf->set_key(this->prf, key);
}

/**
 * The P_hash function as in TLS 1.0/1.2
 */
static bool p_hash(prf_t *prf, char *label, chunk_t seed, size_t block_size,
				   size_t bytes, char *out)
{
	char buf[block_size], abuf[block_size];
	chunk_t a;

	/* seed = label + seed */
	seed = chunk_cata("cc", chunk_create(label, strlen(label)), seed);
	/* A(0) = seed */
	a = seed;

	while (TRUE)
	{
		/* A(i) = HMAC_hash(secret, A(i-1)) */
		if (!prf->get_bytes(prf, a, abuf))
		{
			return FALSE;
		}
		a = chunk_from_thing(abuf);
		/* HMAC_hash(secret, A(i) + seed) */
		if (!prf->get_bytes(prf, a, NULL) ||
			!prf->get_bytes(prf, seed, buf))
		{
			return FALSE;
		}

		if (bytes <= block_size)
		{
			memcpy(out, buf, bytes);
			break;
		}
		memcpy(out, buf, block_size);
		out += block_size;
		bytes -= block_size;
	}
	return TRUE;
}

METHOD(tls_prf_t, get_bytes12, bool,
	private_tls_prf12_t *this, char *label, chunk_t seed,
	size_t bytes, char *out)
{
	return p_hash(this->prf, label, seed, this->prf->get_block_size(this->prf),
				  bytes, out);
}

METHOD(tls_prf_t, destroy12, void,
	private_tls_prf12_t *this)
{
	this->prf->destroy(this->prf);
	free(this);
}

/**
 * See header
 */
tls_prf_t *tls_prf_create_12(pseudo_random_function_t prf)
{
	private_tls_prf12_t *this;

	INIT(this,
		.public = {
			.set_key = _set_key12,
			.get_bytes = _get_bytes12,
			.destroy = _destroy12,
		},
		.prf = lib->crypto->create_prf(lib->crypto, prf),
	);
	if (!this->prf)
	{
		free(this);
		return NULL;
	}
	return &this->public;
}


typedef struct private_tls_prf10_t private_tls_prf10_t;

/**
 * Private data of an tls_prf_t object.
 */
struct private_tls_prf10_t {

	/**
	 * Public tls_prf_t interface.
	 */
	tls_prf_t public;

	/**
	 * Underlying MD5 PRF
	 */
	prf_t *md5;

	/**
	 * Underlying SHA1 PRF
	 */
	prf_t *sha1;
};

METHOD(tls_prf_t, set_key10, bool,
	private_tls_prf10_t *this, chunk_t key)
{
	size_t len = key.len / 2 + key.len % 2;

	return this->md5->set_key(this->md5, chunk_create(key.ptr, len)) &&
		   this->sha1->set_key(this->sha1, chunk_create(key.ptr + key.len - len,
														len));
}

METHOD(tls_prf_t, get_bytes10, bool,
	private_tls_prf10_t *this, char *label, chunk_t seed,
	size_t bytes, char *out)
{
	char buf[bytes];

	if (!p_hash(this->md5, label, seed, this->md5->get_block_size(this->md5),
				bytes, out) ||
		!p_hash(this->sha1, label, seed, this->sha1->get_block_size(this->sha1),
				bytes, buf))
	{
		return FALSE;
	}
	memxor(out, buf, bytes);
	return TRUE;
}

METHOD(tls_prf_t, destroy10, void,
	private_tls_prf10_t *this)
{
	DESTROY_IF(this->md5);
	DESTROY_IF(this->sha1);
	free(this);
}

/**
 * See header
 */
tls_prf_t *tls_prf_create_10(pseudo_random_function_t prf)
{
	private_tls_prf10_t *this;

	INIT(this,
		.public = {
			.set_key = _set_key10,
			.get_bytes = _get_bytes10,
			.destroy = _destroy10,
		},
		.md5 = lib->crypto->create_prf(lib->crypto, PRF_HMAC_MD5),
		.sha1 = lib->crypto->create_prf(lib->crypto, PRF_HMAC_SHA1),
	);
	if (!this->md5 || !this->sha1)
	{
		destroy10(this);
		return NULL;
	}
	return &this->public;
}
