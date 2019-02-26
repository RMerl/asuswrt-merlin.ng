/*
 * Copyright (C) 2018 René Korthaus
 * Copyright (C) 2018 Konstantinos Kolelis
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

#include "botan_diffie_hellman.h"

#include <botan/build.h>

#ifdef BOTAN_HAS_DIFFIE_HELLMAN

#include "botan_util.h"

#include <botan/ffi.h>

#include <utils/debug.h>

typedef struct private_botan_diffie_hellman_t private_botan_diffie_hellman_t;

/**
 * Private data of an botan_diffie_hellman_t object.
 */
struct private_botan_diffie_hellman_t {

	/**
	 * Public botan_diffie_hellman_t interface
	 */
	botan_diffie_hellman_t public;

	/**
	 * Diffie Hellman group number
	 */
	diffie_hellman_group_t group;

	/**
	 * Private key
	 */
	botan_privkey_t dh_key;

	/**
	 * Diffie hellman shared secret
	 */
	chunk_t shared_secret;

	/**
	 * Generator value
	 */
	botan_mp_t g;

	/**
	 * Modulus
	 */
	botan_mp_t p;
};

/**
 * Load a DH private key
 */
bool load_private_key(private_botan_diffie_hellman_t *this, chunk_t value)
{
	botan_mp_t xa;

	if (!chunk_to_botan_mp(value, &xa))
	{
		return FALSE;
	}

	if (botan_privkey_destroy(this->dh_key) ||
		botan_privkey_load_dh(&this->dh_key, this->p, this->g, xa))
	{
		botan_mp_destroy(xa);
		return FALSE;
	}
	botan_mp_destroy(xa);
	return TRUE;
}

METHOD(diffie_hellman_t, set_other_public_value, bool,
	private_botan_diffie_hellman_t *this, chunk_t value)
{
	if (!diffie_hellman_verify_value(this->group, value))
	{
		return FALSE;
	}

	chunk_clear(&this->shared_secret);

	return botan_dh_key_derivation(this->dh_key, value, &this->shared_secret);
}

METHOD(diffie_hellman_t, get_my_public_value, bool,
	private_botan_diffie_hellman_t *this, chunk_t *value)
{
	*value = chunk_empty;

	/* get key size of public key first */
	if (botan_pk_op_key_agreement_export_public(this->dh_key, NULL, &value->len)
		!= BOTAN_FFI_ERROR_INSUFFICIENT_BUFFER_SPACE)
	{
		return FALSE;
	}

	*value = chunk_alloc(value->len);
	if (botan_pk_op_key_agreement_export_public(this->dh_key, value->ptr,
												&value->len))
	{
		chunk_clear(value);
		return FALSE;
	}
	return TRUE;
}

METHOD(diffie_hellman_t, set_private_value, bool,
	private_botan_diffie_hellman_t *this, chunk_t value)
{
	chunk_clear(&this->shared_secret);
	return load_private_key(this, value);
}

METHOD(diffie_hellman_t, get_shared_secret, bool,
	private_botan_diffie_hellman_t *this, chunk_t *secret)
{
	if (!this->shared_secret.len)
	{
		return FALSE;
	}
	*secret = chunk_clone(this->shared_secret);
	return TRUE;
}

METHOD(diffie_hellman_t, get_dh_group, diffie_hellman_group_t,
	private_botan_diffie_hellman_t *this)
{
	return this->group;
}

METHOD(diffie_hellman_t, destroy, void,
	private_botan_diffie_hellman_t *this)
{
	botan_mp_destroy(this->p);
	botan_mp_destroy(this->g);
	botan_privkey_destroy(this->dh_key);
	chunk_clear(&this->shared_secret);
	free(this);
}

/*
 * Generic internal constructor
 */
static botan_diffie_hellman_t *create_generic(diffie_hellman_group_t group,
										chunk_t g, chunk_t p, size_t exp_len)
{
	private_botan_diffie_hellman_t *this;
	chunk_t random;
	rng_t *rng;

	INIT(this,
		.public = {
			.dh = {
				.get_shared_secret = _get_shared_secret,
				.set_other_public_value = _set_other_public_value,
				.get_my_public_value = _get_my_public_value,
				.set_private_value = _set_private_value,
				.get_dh_group = _get_dh_group,
				.destroy = _destroy,
			},
		},
		.group = group,
	);

	if (!chunk_to_botan_mp(p, &this->p))
	{
		destroy(this);
		return NULL;
	}

	if (!chunk_to_botan_mp(g, &this->g))
	{
		destroy(this);
		return NULL;
	}

	rng = lib->crypto->create_rng(lib->crypto, RNG_STRONG);
	if (!rng || !rng->allocate_bytes(rng, exp_len, &random))
	{
		DESTROY_IF(rng);
		destroy(this);
		return NULL;
	}
	rng->destroy(rng);

	if (!load_private_key(this, random))
	{
		chunk_clear(&random);
		destroy(this);
		return NULL;
	}
	chunk_clear(&random);
	return &this->public;
}

/*
 * Described in header.
 */
botan_diffie_hellman_t *botan_diffie_hellman_create(
											diffie_hellman_group_t group, ...)
{
	diffie_hellman_params_t *params;
	chunk_t g, p;

	if (group == MODP_CUSTOM)
	{
		VA_ARGS_GET(group, g, p);
		return create_generic(group, g, p, p.len);
	}

	params = diffie_hellman_get_params(group);
	if (!params)
	{
		return NULL;
	}
	return create_generic(group, params->generator, params->prime,
						  params->exp_len);
}

#endif
