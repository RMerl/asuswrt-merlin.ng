/*
 * Copyright (C) 2012 Tobias Brunner
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

/*
 * Copyright (C) 2012 Aleksandr Grinberg
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <openssl/opensslconf.h>

#ifndef OPENSSL_NO_HMAC

#include <openssl/evp.h>
#include <openssl/hmac.h>

#include "openssl_hmac.h"

#include <crypto/mac.h>
#include <crypto/prfs/mac_prf.h>
#include <crypto/signers/mac_signer.h>

typedef struct private_mac_t private_mac_t;

/**
 * Private data of a mac_t object.
 */
struct private_mac_t {

	/**
	 * Public interface
	 */
	mac_t public;

	/**
	 * Hasher to use
	 */
	const EVP_MD *hasher;

	/**
	 * Current HMAC context
	 */
	HMAC_CTX *hmac;

#if OPENSSL_VERSION_NUMBER < 0x10100000L
	/**
	 * Static context for OpenSSL < 1.1.0
	 */
	HMAC_CTX hmac_ctx;
#endif

	/**
	 * Key set on HMAC_CTX?
	 */
	bool key_set;
};

METHOD(mac_t, set_key, bool,
	private_mac_t *this, chunk_t key)
{
#if OPENSSL_VERSION_NUMBER >= 0x10000000L
	if (HMAC_Init_ex(this->hmac, key.ptr, key.len, this->hasher, NULL))
	{
		this->key_set = TRUE;
		return TRUE;
	}
	return FALSE;
#else /* OPENSSL_VERSION_NUMBER < 1.0 */
	HMAC_Init_ex(this->hmac, key.ptr, key.len, this->hasher, NULL);
	this->key_set = TRUE;
	return TRUE;
#endif
}

METHOD(mac_t, get_mac, bool,
	private_mac_t *this, chunk_t data, uint8_t *out)
{
	if (!this->key_set)
	{
		return FALSE;
	}
#if OPENSSL_VERSION_NUMBER >= 0x10000000L
	if (!HMAC_Update(this->hmac, data.ptr, data.len))
	{
		return FALSE;
	}
	if (out == NULL)
	{
		return TRUE;
	}
	if (!HMAC_Final(this->hmac, out, NULL))
	{
		return FALSE;
	}
#else /* OPENSSL_VERSION_NUMBER < 1.0 */
	HMAC_Update(this->hmac, data.ptr, data.len);
	if (out == NULL)
	{
		return TRUE;
	}
	HMAC_Final(this->hmac, out, NULL);
#endif
	return set_key(this, chunk_empty);
}

METHOD(mac_t, get_mac_size, size_t,
	private_mac_t *this)
{
	return EVP_MD_size(this->hasher);
}

METHOD(mac_t, destroy, void,
	private_mac_t *this)
{
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
	HMAC_CTX_free(this->hmac);
#else
	HMAC_CTX_cleanup(&this->hmac_ctx);
#endif
	free(this);
}

/*
 * Create an OpenSSL-backed implementation of the mac_t interface
 */
static mac_t *hmac_create(hash_algorithm_t algo)
{
	private_mac_t *this;
	char *name;

	name = enum_to_name(hash_algorithm_short_names, algo);
	if (!name)
	{
		return NULL;
	}

	INIT(this,
		.public = {
			.get_mac = _get_mac,
			.get_mac_size = _get_mac_size,
			.set_key = _set_key,
			.destroy = _destroy,
		},
		.hasher = EVP_get_digestbyname(name),
	);

	if (!this->hasher)
	{
		free(this);
		return NULL;
	}

#if OPENSSL_VERSION_NUMBER >= 0x10100000L
	this->hmac = HMAC_CTX_new();
#else
	HMAC_CTX_init(&this->hmac_ctx);
	this->hmac = &this->hmac_ctx;
#endif

	return &this->public;
}

/*
 * Described in header
 */
prf_t *openssl_hmac_prf_create(pseudo_random_function_t algo)
{
	mac_t *hmac;

	hmac = hmac_create(hasher_algorithm_from_prf(algo));
	if (hmac)
	{
		return mac_prf_create(hmac);
	}
	return NULL;
}

/*
 * Described in header
 */
signer_t *openssl_hmac_signer_create(integrity_algorithm_t algo)
{
	mac_t *hmac;
	size_t trunc;

	hmac = hmac_create(hasher_algorithm_from_integrity(algo, &trunc));
	if (hmac)
	{
		return mac_signer_create(hmac, trunc);
	}
	return NULL;
}

#endif /* OPENSSL_NO_HMAC */
