/*
 * Copyright (C) 2012 Tobias Brunner
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

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
#include <openssl/core_names.h>
#endif

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

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
	/**
	 * HMAC context
	 */
	EVP_MAC_CTX *hmac;

	/**
	 * Base context because EVP_MAC_init() does not reset the internal state if
	 * no key is passed, so the above is a copy that's replaced with every
	 * reset that does not change the key
	 */
	EVP_MAC_CTX *hmac_base;
#else
	/**
	 * Hasher to use
	 */
	const EVP_MD *hasher;

	/**
	 * Current HMAC context
	 */
	HMAC_CTX *hmac;
#endif

#if OPENSSL_VERSION_NUMBER < 0x10100000L
	/**
	 * Static context for OpenSSL < 1.1.0
	 */
	HMAC_CTX hmac_ctx;
#endif
};

/**
 * Resets the state with the given key, or only resets the internal state
 * if key is chunk_empty.
 */
static bool reset(private_mac_t *this, chunk_t key)
{
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
	if (!key.len || EVP_MAC_init(this->hmac_base, key.ptr, key.len, NULL))
	{
		EVP_MAC_CTX_free(this->hmac);
		this->hmac = EVP_MAC_CTX_dup(this->hmac_base);
		return TRUE;
	}
#else
	if (HMAC_Init_ex(this->hmac, key.ptr, key.len, this->hasher, NULL))
	{
		return TRUE;
	}
#endif
	return FALSE;
}

METHOD(mac_t, set_key, bool,
	private_mac_t *this, chunk_t key)
{
	if (!key.ptr)
	{	/* HMAC_Init_ex() won't reset the key if a NULL pointer is passed,
		 * use a lengthy string in case there is a limit in FIPS-mode */
		key = chunk_from_str("00000000000000000000000000000000");
	}
	if (!reset(this, key))
	{
		return FALSE;
	}
	return TRUE;
}

METHOD(mac_t, get_mac_size, size_t,
	private_mac_t *this)
{
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
	return EVP_MAC_CTX_get_mac_size(this->hmac);
#else
	return EVP_MD_size(this->hasher);
#endif
}

METHOD(mac_t, get_mac, bool,
	private_mac_t *this, chunk_t data, uint8_t *out)
{
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
	if (!EVP_MAC_update(this->hmac, data.ptr, data.len))
	{
		return FALSE;
	}
#else
	if (!HMAC_Update(this->hmac, data.ptr, data.len))
	{
		return FALSE;
	}
#endif
	if (!out)
	{
		return TRUE;
	}
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
	if (!EVP_MAC_final(this->hmac, out, NULL, get_mac_size(this)))
	{
		return FALSE;
	}
#else
	if (!HMAC_Final(this->hmac, out, NULL))
	{
		return FALSE;
	}
#endif
	return reset(this, chunk_empty);
}

METHOD(mac_t, destroy, void,
	private_mac_t *this)
{
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
	EVP_MAC_CTX_free(this->hmac_base);
	EVP_MAC_CTX_free(this->hmac);
#elif OPENSSL_VERSION_NUMBER >= 0x10100000L
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
	);

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
	OSSL_PARAM params[] = {
		OSSL_PARAM_utf8_string(OSSL_MAC_PARAM_DIGEST, name, 0),
		OSSL_PARAM_END,
	};
	EVP_MAC *mac = EVP_MAC_fetch(NULL, "HMAC", NULL);

	if (!mac)
	{
		free(this);
		return NULL;
	}
	this->hmac_base = EVP_MAC_CTX_new(mac);
	EVP_MAC_free(mac);
	if (!this->hmac_base || !EVP_MAC_CTX_set_params(this->hmac_base, params))
	{
		free(this);
		return NULL;
	}
#else /* OPENSSL_VERSION_NUMBER */
	this->hasher = EVP_get_digestbyname(name);
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
#endif /* OPENSSL_VERSION_NUMBER */

	/* make sure the underlying hash algorithm is supported */
	if (!set_key(this, chunk_empty))
	{
		destroy(this);
		return NULL;
	}
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
