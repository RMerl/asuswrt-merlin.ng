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

#include "botan_ec_diffie_hellman.h"

#include <botan/build.h>

#ifdef BOTAN_HAS_ECDH

#include "botan_util.h"

#include <utils/debug.h>

#include <botan/ffi.h>

typedef struct private_botan_ec_diffie_hellman_t private_botan_ec_diffie_hellman_t;

/**
 * Private data of a botan_ec_diffie_hellman_t object.
 */
struct private_botan_ec_diffie_hellman_t {

	/**
	 * Public interface
	 */
	botan_ec_diffie_hellman_t public;

	/**
	 * Diffie Hellman group
	 */
	key_exchange_method_t group;

	/**
	 * EC curve name
	 */
	const char* curve_name;

	/**
	 * EC private key
	 */
	botan_privkey_t key;

	/**
	 * Public key value provided by peer
	 */
	chunk_t pubkey;

	/**
	 * Shared secret
	 */
	chunk_t shared_secret;
};

METHOD(key_exchange_t, set_public_key, bool,
	private_botan_ec_diffie_hellman_t *this, chunk_t value)
{
	if (!key_exchange_verify_pubkey(this->group, value))
	{
		return FALSE;
	}

	chunk_clear(&this->pubkey);
	/* prepend 0x04 to indicate uncompressed point format */
	this->pubkey = chunk_cat("cc", chunk_from_chars(0x04), value);
	return TRUE;
}

METHOD(key_exchange_t, get_public_key, bool,
	private_botan_ec_diffie_hellman_t *this, chunk_t *value)
{
	chunk_t pkey = chunk_empty;

	if (botan_pk_op_key_agreement_export_public(this->key, NULL, &pkey.len)
		!= BOTAN_FFI_ERROR_INSUFFICIENT_BUFFER_SPACE)
	{
		return FALSE;
	}

	pkey = chunk_alloca(pkey.len);
	if (botan_pk_op_key_agreement_export_public(this->key, pkey.ptr, &pkey.len))
	{
		return FALSE;
	}

	/* skip 0x04 byte prepended by botan */
	*value = chunk_clone(chunk_skip(pkey, 1));
	return TRUE;
}

METHOD(key_exchange_t, set_private_key, bool,
	private_botan_ec_diffie_hellman_t *this, chunk_t value)
{
	botan_mp_t scalar;

	chunk_clear(&this->shared_secret);

	if (!chunk_to_botan_mp(value, &scalar))
	{
		return FALSE;
	}

	if (botan_privkey_destroy(this->key))
	{
		botan_mp_destroy(scalar);
		return FALSE;
	}

	if (botan_privkey_load_ecdh(&this->key, scalar, this->curve_name))
	{
		botan_mp_destroy(scalar);
		return FALSE;
	}

	botan_mp_destroy(scalar);
	return TRUE;
}

METHOD(key_exchange_t, get_shared_secret, bool,
	private_botan_ec_diffie_hellman_t *this, chunk_t *secret)
{
	if (!this->shared_secret.len &&
		!botan_dh_key_derivation(this->key, this->pubkey, &this->shared_secret))
	{
		return FALSE;
	}
	*secret = chunk_clone(this->shared_secret);
	return TRUE;
}

METHOD(key_exchange_t, get_method, key_exchange_method_t,
	private_botan_ec_diffie_hellman_t *this)
{
	return this->group;
}

METHOD(key_exchange_t, destroy, void,
	private_botan_ec_diffie_hellman_t *this)
{
	botan_privkey_destroy(this->key);
	chunk_clear(&this->shared_secret);
	chunk_clear(&this->pubkey);
	free(this);
}

/*
 * Described in header.
 */
botan_ec_diffie_hellman_t *botan_ec_diffie_hellman_create(
												key_exchange_method_t group)
{
	private_botan_ec_diffie_hellman_t *this;
	botan_rng_t rng;

	INIT(this,
		.public = {
			.ke = {
				.get_shared_secret = _get_shared_secret,
				.set_public_key = _set_public_key,
				.get_public_key = _get_public_key,
				.set_private_key = _set_private_key,
				.get_method = _get_method,
				.destroy = _destroy,
			},
		},
		.group = group,
	);

	switch (group)
	{
		case ECP_256_BIT:
			this->curve_name = "secp256r1";
			break;
		case ECP_384_BIT:
			this->curve_name = "secp384r1";
			break;
		case ECP_521_BIT:
			this->curve_name = "secp521r1";
			break;
		case ECP_256_BP:
			this->curve_name = "brainpool256r1";
			break;
		case ECP_384_BP:
			this->curve_name = "brainpool384r1";
			break;
		case ECP_512_BP:
			this->curve_name = "brainpool512r1";
			break;
		default:
			free(this);
			return NULL;
	}

	if (!botan_get_rng(&rng, RNG_STRONG))
	{
		free(this);
		return NULL;
	}

	if (botan_privkey_create(&this->key, "ECDH", this->curve_name, rng))
	{
		DBG1(DBG_LIB, "ECDH private key generation failed");
		botan_rng_destroy(rng);
		free(this);
		return NULL;
	}

	botan_rng_destroy(rng);
	return &this->public;
}

#endif
