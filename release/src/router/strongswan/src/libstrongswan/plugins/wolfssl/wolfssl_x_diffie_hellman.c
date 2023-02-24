/*
 * Copyright (C) 2020 Tobias Brunner
 *
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

#if defined(HAVE_CURVE25519) || defined(HAVE_CURVE448)

#include "wolfssl_x_diffie_hellman.h"

#include <utils/debug.h>

#ifdef HAVE_CURVE25519
#include <wolfssl/wolfcrypt/curve25519.h>
#endif
#ifdef HAVE_CURVE448
#include <wolfssl/wolfcrypt/curve448.h>
#endif

#include <wolfssl/wolfcrypt/fe_operations.h>

typedef struct private_diffie_hellman_t private_diffie_hellman_t;

/**
 * Private data
 */
struct private_diffie_hellman_t {

	/**
	 * Public interface.
	 */
	key_exchange_t public;

	/**
	 * Diffie Hellman group number.
	 */
	key_exchange_method_t group;

	/**
	 * Private (public) key
	 */
	union {
#ifdef HAVE_CURVE25519
		curve25519_key key25519;
#endif
#ifdef HAVE_CURVE448
		curve448_key key448;
#endif
	} key;

	/**
	 * Public key provided by peer
	 */
	union {
#ifdef HAVE_CURVE25519
		curve25519_key key25519;
#endif
#ifdef HAVE_CURVE448
		curve448_key key448;
#endif
	} pub;

	/**
	 * Shared secret
	 */
	chunk_t shared_secret;
};

#ifdef HAVE_CURVE25519

METHOD(key_exchange_t, get_shared_secret_25519, bool,
	private_diffie_hellman_t *this, chunk_t *secret)
{
	word32 len = CURVE25519_KEYSIZE;

	if (!this->shared_secret.len)
	{
		this->shared_secret = chunk_alloc(len);
		if (wc_curve25519_shared_secret_ex(&this->key.key25519, &this->pub.key25519,
					this->shared_secret.ptr, &len, EC25519_LITTLE_ENDIAN) != 0)
		{
			DBG1(DBG_LIB, "%N shared secret computation failed",
				 key_exchange_method_names, this->group);
			chunk_clear(&this->shared_secret);
			return FALSE;
		}
	}
	*secret = chunk_clone(this->shared_secret);
	return TRUE;
}

METHOD(key_exchange_t, set_public_key_25519, bool,
	private_diffie_hellman_t *this, chunk_t value)
{
	if (!key_exchange_verify_pubkey(this->group, value))
	{
		return FALSE;
	}

	if (wc_curve25519_import_public_ex(value.ptr, value.len, &this->pub.key25519,
									   EC25519_LITTLE_ENDIAN) != 0)
	{
		DBG1(DBG_LIB, "%N public value is malformed",
			 key_exchange_method_names, this->group);
		return FALSE;
	}
	return TRUE;
}

METHOD(key_exchange_t, get_public_key_25519, bool,
	private_diffie_hellman_t *this, chunk_t *value)
{
	word32 len = CURVE25519_KEYSIZE;

	*value = chunk_alloc(len);
	if (wc_curve25519_export_public_ex(&this->key.key25519, value->ptr, &len,
									   EC25519_LITTLE_ENDIAN) != 0)
	{
		chunk_free(value);
		return FALSE;
	}
	return TRUE;
}

METHOD(key_exchange_t, set_private_key_25519, bool,
	private_diffie_hellman_t *this, chunk_t value)
{
	curve25519_key pub;
	u_char basepoint[CURVE25519_KEYSIZE] = {9};
	word32 len = CURVE25519_KEYSIZE;
	int ret;

	ret = wc_curve25519_init(&pub);
	/* create base point for calculating public key */
	if (ret == 0)
	{
		ret = wc_curve25519_import_public_ex(basepoint, CURVE25519_KEYSIZE,
											 &pub, EC25519_LITTLE_ENDIAN);
	}
	if (ret == 0)
	{
		ret = wc_curve25519_import_private_ex(value.ptr, value.len,
									&this->key.key25519, EC25519_LITTLE_ENDIAN);
	}
	if (ret == 0)
	{
		ret = wc_curve25519_shared_secret_ex(&this->key.key25519, &pub,
											 this->key.key25519.p.point, &len,
											 EC25519_LITTLE_ENDIAN);
	}
	return ret == 0;
}

#endif /* HAVE_CURVE25519 */

#ifdef HAVE_CURVE448

METHOD(key_exchange_t, get_shared_secret_448, bool,
	private_diffie_hellman_t *this, chunk_t *secret)
{
	word32 len = CURVE448_KEY_SIZE;

	if (!this->shared_secret.len)
	{
		this->shared_secret = chunk_alloc(len);
		if (wc_curve448_shared_secret_ex(&this->key.key448, &this->pub.key448,
					this->shared_secret.ptr, &len, EC448_LITTLE_ENDIAN) != 0)
		{
			DBG1(DBG_LIB, "%N shared secret computation failed",
				 key_exchange_method_names, this->group);
			chunk_clear(&this->shared_secret);
			return FALSE;
		}
	}
	*secret = chunk_clone(this->shared_secret);
	return TRUE;
}

METHOD(key_exchange_t, set_public_key_448, bool,
	private_diffie_hellman_t *this, chunk_t value)
{
	if (!key_exchange_verify_pubkey(this->group, value))
	{
		return FALSE;
	}

	if (wc_curve448_import_public_ex(value.ptr, value.len, &this->pub.key448,
									 EC448_LITTLE_ENDIAN) != 0)
	{
		DBG1(DBG_LIB, "%N public value is malformed",
			 key_exchange_method_names, this->group);
		return FALSE;
	}
	return TRUE;
}

METHOD(key_exchange_t, get_public_key_448, bool,
	private_diffie_hellman_t *this, chunk_t *value)
{
	word32 len = CURVE448_KEY_SIZE;

	*value = chunk_alloc(len);
	if (wc_curve448_export_public_ex(&this->key.key448, value->ptr, &len,
									 EC448_LITTLE_ENDIAN) != 0)
	{
		chunk_free(value);
		return FALSE;
	}
	return TRUE;
}

METHOD(key_exchange_t, set_private_key_448, bool,
	private_diffie_hellman_t *this, chunk_t value)
{
	curve448_key pub;
	u_char basepoint[CURVE448_KEY_SIZE] = {5};
	word32 len = CURVE448_KEY_SIZE;
	int ret;

	ret = wc_curve448_init(&pub);
	/* create base point for calculating public key */
	if (ret == 0)
	{
		ret = wc_curve448_import_public_ex(basepoint, CURVE448_KEY_SIZE,
										   &pub, EC448_LITTLE_ENDIAN);
	}
	if (ret == 0)
	{
		ret = wc_curve448_import_private_ex(value.ptr, value.len,
										&this->key.key448, EC448_LITTLE_ENDIAN);
	}
	if (ret == 0)
	{
		ret = wc_curve448_shared_secret_ex(&this->key.key448, &pub,
										   this->key.key448.p, &len,
										   EC448_LITTLE_ENDIAN);
	}
	return ret == 0;
}

#endif /* HAVE_CURVE448 */

METHOD(key_exchange_t, get_method, key_exchange_method_t,
	private_diffie_hellman_t *this)
{
	return this->group;
}

METHOD(key_exchange_t, destroy, void,
	private_diffie_hellman_t *this)
{
	if (this->group == CURVE_25519)
	{
#ifdef HAVE_CURVE25519
		wc_curve25519_free(&this->key.key25519);
		wc_curve25519_free(&this->pub.key25519);
#endif
	}
	else if (this->group == CURVE_448)
	{
#ifdef HAVE_CURVE448
		wc_curve448_free(&this->key.key448);
		wc_curve448_free(&this->pub.key448);
#endif
	}
	chunk_clear(&this->shared_secret);
	free(this);
}

/*
 * Described in header
 */
key_exchange_t *wolfssl_x_diffie_hellman_create(key_exchange_method_t group)
{
	private_diffie_hellman_t *this;
	WC_RNG rng;
	int ret = -1;

	INIT(this,
		.public = {
			.get_method = _get_method,
			.destroy = _destroy,
		},
		.group = group,
	);

	if (wc_InitRng(&rng) != 0)
	{
		DBG1(DBG_LIB, "initializing a random number generator failed");
		destroy(this);
		return NULL;
	}

	if (group == CURVE_25519)
	{
#ifdef HAVE_CURVE25519
		this->public.get_shared_secret = _get_shared_secret_25519;
		this->public.set_public_key = _set_public_key_25519;
		this->public.get_public_key = _get_public_key_25519;
		this->public.set_private_key = _set_private_key_25519;

		if (wc_curve25519_init(&this->key.key25519) != 0 ||
			wc_curve25519_init(&this->pub.key25519) != 0)
		{
			DBG1(DBG_LIB, "initializing key failed");
			destroy(this);
			return NULL;
		}
		ret = wc_curve25519_make_key(&rng, CURVE25519_KEYSIZE,
									 &this->key.key25519);
#endif
	}
	else if (group == CURVE_448)
	{
#ifdef HAVE_CURVE448
		this->public.get_shared_secret = _get_shared_secret_448;
		this->public.set_public_key = _set_public_key_448;
		this->public.get_public_key = _get_public_key_448;
		this->public.set_private_key = _set_private_key_448;

		if (wc_curve448_init(&this->key.key448) != 0 ||
			wc_curve448_init(&this->pub.key448) != 0)
		{
			DBG1(DBG_LIB, "initializing key failed");
			destroy(this);
			return NULL;
		}
		ret = wc_curve448_make_key(&rng, CURVE448_KEY_SIZE, &this->key.key448);
#endif
	}
	wc_FreeRng(&rng);
	if (ret != 0)
	{
		DBG1(DBG_LIB, "making a key failed");
		destroy(this);
		return NULL;
	}
	return &this->public;
}

#endif /* HAVE_CURVE25519 || HAVE_CURVE448 */
