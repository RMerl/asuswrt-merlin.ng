/*
 * Copyright (C) 2019 Sean Parkinson, wolfSSL Inc.
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

#include "wolfssl_common.h"

#ifndef NO_HMAC

#include <wolfssl/wolfcrypt/hmac.h>

#include "wolfssl_hmac.h"
#include "wolfssl_util.h"

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
	 * Current HMAC
	 */
	Hmac hmac;

	/**
	 * Hasher to use
	 */
	enum wc_HashType type;

	/**
	 * Key set on Hmac?
	 */
	bool key_set;
};

METHOD(mac_t, set_key, bool,
	private_mac_t *this, chunk_t key)
{
	int ret = wc_HmacSetKey(&this->hmac, this->type, key.ptr, key.len);
	this->key_set = (ret == 0);
	return ret == 0;
}

METHOD(mac_t, get_mac, bool,
	private_mac_t *this, chunk_t data, uint8_t *out)
{
	int ret = -1;

	if (this->key_set)
	{
		ret = wc_HmacUpdate(&this->hmac, data.ptr, data.len);
		if (ret == 0 && out)
		{
			ret = wc_HmacFinal(&this->hmac, out);
		}
	}
	return ret == 0;
}

METHOD(mac_t, get_mac_size, size_t,
	private_mac_t *this)
{
	return wc_HmacSizeByType(this->type);
}

METHOD(mac_t, destroy, void,
	private_mac_t *this)
{
	wc_HmacFree(&this->hmac);
	free(this);
}

/*
 * Create an wolfSSL-backed implementation of the mac_t interface
 */
static mac_t *hmac_create(hash_algorithm_t algo)
{
	private_mac_t *this;
	enum wc_HashType type;

	if (!wolfssl_hash2type(algo, &type))
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
		.type = type,
	);

	if (wc_HmacInit(&this->hmac, NULL, INVALID_DEVID) != 0)
	{
		DBG1(DBG_LIB, "HMAC init failed, hmac create failed\n");
		free(this);
		return NULL;
	}
	return &this->public;
}

/*
 * Described in header
 */
prf_t *wolfssl_hmac_prf_create(pseudo_random_function_t algo)
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
signer_t *wolfssl_hmac_signer_create(integrity_algorithm_t algo)
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

#endif /* NO_HMAC */
