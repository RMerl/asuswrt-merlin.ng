/*
 * Copyright (C) 2015 Martin Willi
 * Copyright (C) 2015 revosec AG
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

#include <string.h>
#include <stdint.h>

#include "curve25519_dh.h"
#include "curve25519_drv.h"

typedef struct private_curve25519_dh_t private_curve25519_dh_t;

/**
 * Private data of an curve25519_dh_t object.
 */
struct private_curve25519_dh_t {

	/**
	 * Public curve25519_dh_t interface.
	 */
	curve25519_dh_t public;

	/**
	 * Shared key, if computed
	 */
	u_char shared[CURVE25519_KEY_SIZE];

	/**
	 * TRUE if shared secret is computed
	 */
	bool computed;

	/**
	 * Curve25519 backend
	 */
	curve25519_drv_t *drv;
};

/**
 * Generate a valid Curve25519 key
 */
static bool generate_key(private_curve25519_dh_t *this)
{
	u_char key[CURVE25519_KEY_SIZE];
	rng_t *rng;

	rng = lib->crypto->create_rng(lib->crypto, RNG_STRONG);
	if (!rng)
	{
		DBG1(DBG_LIB, "no RNG found for quality %N",
			 rng_quality_names, RNG_STRONG);
		return FALSE;
	}
	if (!rng->get_bytes(rng, CURVE25519_KEY_SIZE, key))
	{
		rng->destroy(rng);
		return FALSE;
	}
	rng->destroy(rng);

	return this->drv->set_key(this->drv, key);
}

METHOD(diffie_hellman_t, set_other_public_value, bool,
	private_curve25519_dh_t *this, chunk_t value)
{
	if (value.len == CURVE25519_KEY_SIZE)
	{
		if (this->drv->curve25519(this->drv, value.ptr, this->shared))
		{
			this->computed = TRUE;
			return TRUE;
		}
	}
	return FALSE;
}

METHOD(diffie_hellman_t, get_my_public_value, bool,
	private_curve25519_dh_t *this, chunk_t *value)
{
	u_char basepoint[CURVE25519_KEY_SIZE] = { 9 };

	*value = chunk_alloc(CURVE25519_KEY_SIZE);
	if (this->drv->curve25519(this->drv, basepoint, value->ptr))
	{
		return TRUE;
	}
	free(value->ptr);
	return FALSE;
}

METHOD(diffie_hellman_t, set_private_value, bool,
	private_curve25519_dh_t *this, chunk_t value)
{
	if (value.len != CURVE25519_KEY_SIZE)
	{
		return FALSE;
	}
	return this->drv->set_key(this->drv, value.ptr);
}

METHOD(diffie_hellman_t, get_shared_secret, bool,
	private_curve25519_dh_t *this, chunk_t *secret)
{
	if (!this->computed)
	{
		return FALSE;
	}
	*secret = chunk_clone(chunk_from_thing(this->shared));
	return TRUE;
}

METHOD(diffie_hellman_t, get_dh_group, diffie_hellman_group_t,
	private_curve25519_dh_t *this)
{
	return CURVE_25519;
}

METHOD(diffie_hellman_t, destroy, void,
	private_curve25519_dh_t *this)
{
	this->drv->destroy(this->drv);
	free(this);
}

/*
 * Described in header.
 */
curve25519_dh_t *curve25519_dh_create(diffie_hellman_group_t group)
{
	private_curve25519_dh_t *this;

	if (group != CURVE_25519)
	{
		return FALSE;
	}

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
		.drv = curve25519_drv_probe(),
	);

	if (!this->drv)
	{
		free(this);
		return NULL;
	}
	if (!generate_key(this))
	{
		destroy(this);
		return NULL;
	}
	return &this->public;
}
