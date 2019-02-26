/*
 * Copyright (C) 2018 René Korthaus
 * Rohde & Schwarz Cybersecurity GmbH
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

#include "botan_hmac.h"

#include <botan/build.h>

#ifdef BOTAN_HAS_HMAC

#include <crypto/mac.h>
#include <crypto/prfs/mac_prf.h>
#include <crypto/signers/mac_signer.h>

#include <botan/ffi.h>

typedef struct private_botan_mac_t private_botan_mac_t;

/**
 * Private data of a mac_t object.
 */
struct private_botan_mac_t {

	/**
	 * Public interface
	 */
	mac_t public;

	/**
	 * HMAC
	 */
	botan_mac_t hmac;
};

METHOD(mac_t, set_key, bool,
	private_botan_mac_t *this, chunk_t key)
{
	if (botan_mac_set_key(this->hmac, key.ptr, key.len))
	{
		return FALSE;
	}
	return TRUE;
}

METHOD(mac_t, get_mac, bool,
	private_botan_mac_t *this, chunk_t data, uint8_t *out)
{
	if (botan_mac_update(this->hmac, data.ptr, data.len))
	{
		return FALSE;
	}

	if (out && botan_mac_final(this->hmac, out))
	{
		return FALSE;
	}
	return TRUE;
}

METHOD(mac_t, get_mac_size, size_t,
	private_botan_mac_t *this)
{
	size_t len = 0;

	if (botan_mac_output_length(this->hmac, &len))
	{
		return 0;
	}
	return len;
}

METHOD(mac_t, destroy, void,
	private_botan_mac_t *this)
{
	botan_mac_destroy(this->hmac);
	free(this);
}

/*
 * Create a Botan-backed implementation of the mac_t interface
 */
static mac_t *hmac_create(hash_algorithm_t algo)
{
	private_botan_mac_t *this;
	const char* hmac_name;

	switch (algo)
	{
		case HASH_SHA1:
			hmac_name = "HMAC(SHA-1)";
			break;
		case HASH_SHA256:
			hmac_name = "HMAC(SHA-256)";
			break;
		case HASH_SHA384:
			hmac_name = "HMAC(SHA-384)";
			break;
		case HASH_SHA512:
			hmac_name = "HMAC(SHA-512)";
			break;
		default:
			return NULL;
	}

	INIT(this,
		.public = {
			.get_mac = _get_mac,
			.get_mac_size = _get_mac_size,
			.set_key = _set_key,
			.destroy = _destroy,
		}
	);

	if (botan_mac_init(&this->hmac, hmac_name, 0))
	{
		free(this);
		return NULL;
	}
	return &this->public;
}

/*
 * Described in header
 */
prf_t *botan_hmac_prf_create(pseudo_random_function_t algo)
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
signer_t *botan_hmac_signer_create(integrity_algorithm_t algo)
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

#endif
