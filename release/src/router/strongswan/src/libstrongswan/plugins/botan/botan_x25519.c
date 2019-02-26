/*
 * Copyright (C) 2018 Tobias Brunner
 * HSR Hochschule fuer Technik Rapperswil
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

#include "botan_x25519.h"

#include <botan/build.h>

#ifdef BOTAN_HAS_X25519

#include "botan_util.h"

#include <utils/debug.h>

#include <botan/ffi.h>

typedef struct private_diffie_hellman_t private_diffie_hellman_t;

/**
 * Private data
 */
struct private_diffie_hellman_t {

	/**
	 * Public interface
	 */
	diffie_hellman_t public;

	/**
	 * Private key
	 */
	botan_privkey_t key;

	/**
	 * Shared secret
	 */
	chunk_t shared_secret;
};

METHOD(diffie_hellman_t, set_other_public_value, bool,
	private_diffie_hellman_t *this, chunk_t value)
{
	if (!diffie_hellman_verify_value(CURVE_25519, value))
	{
		return FALSE;
	}

	chunk_clear(&this->shared_secret);

	return botan_dh_key_derivation(this->key, value, &this->shared_secret);
}

METHOD(diffie_hellman_t, get_my_public_value, bool,
	private_diffie_hellman_t *this, chunk_t *value)
{
	value->len = 0;
	if (botan_pk_op_key_agreement_export_public(this->key, NULL, &value->len)
		!= BOTAN_FFI_ERROR_INSUFFICIENT_BUFFER_SPACE)
	{
		return FALSE;
	}

	*value = chunk_alloc(value->len);
	if (botan_pk_op_key_agreement_export_public(this->key, value->ptr,
												&value->len))
	{
		chunk_free(value);
		return FALSE;
	}
	return TRUE;
}

METHOD(diffie_hellman_t, set_private_value, bool,
	private_diffie_hellman_t *this, chunk_t value)
{
	if (value.len != 32)
	{
		return FALSE;
	}

	chunk_clear(&this->shared_secret);

	if (botan_privkey_destroy(this->key))
	{
		return FALSE;
	}

	if (botan_privkey_load_x25519(&this->key, value.ptr))
	{
		return FALSE;
	}
	return TRUE;
}

METHOD(diffie_hellman_t, get_shared_secret, bool,
	private_diffie_hellman_t *this, chunk_t *secret)
{
	if (!this->shared_secret.len)
	{
		return FALSE;
	}
	*secret = chunk_clone(this->shared_secret);
	return TRUE;
}

METHOD(diffie_hellman_t, get_dh_group, diffie_hellman_group_t,
	private_diffie_hellman_t *this)
{
	return CURVE_25519;
}

METHOD(diffie_hellman_t, destroy, void,
	private_diffie_hellman_t *this)
{
	botan_privkey_destroy(this->key);
	chunk_clear(&this->shared_secret);
	free(this);
}

/*
 * Described in header
 */
diffie_hellman_t *botan_x25519_create(diffie_hellman_group_t group)
{
	private_diffie_hellman_t *this;
	botan_rng_t rng;

	INIT(this,
		.public = {
			.get_shared_secret = _get_shared_secret,
			.set_other_public_value = _set_other_public_value,
			.get_my_public_value = _get_my_public_value,
			.set_private_value = _set_private_value,
			.get_dh_group = _get_dh_group,
			.destroy = _destroy,
		},
	);

	if (botan_rng_init(&rng, "user"))
	{
		free(this);
		return NULL;
	}

	if (botan_privkey_create_ecdh(&this->key, rng, "curve25519"))
	{
		DBG1(DBG_LIB, "x25519 private key generation failed");
		botan_rng_destroy(rng);
		free(this);
		return NULL;
	}

	botan_rng_destroy(rng);
	return &this->public;
}

#endif
