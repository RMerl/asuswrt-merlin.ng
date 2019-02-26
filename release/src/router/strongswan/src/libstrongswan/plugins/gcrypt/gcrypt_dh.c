/*
 * Copyright (C) 2010 Tobias Brunner
 * Copyright (C) 2009 Martin Willi
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

#include <gcrypt.h>

#include "gcrypt_dh.h"

#include <utils/debug.h>

typedef struct private_gcrypt_dh_t private_gcrypt_dh_t;

/**
 * Private data of an gcrypt_dh_t object.
 */
struct private_gcrypt_dh_t {

	/**
	 * Public gcrypt_dh_t interface
	 */
	gcrypt_dh_t public;

	/**
	 * Diffie Hellman group number
	 */
	diffie_hellman_group_t group;

	/*
	 * Generator value
	 */
	gcry_mpi_t g;

	/**
	 * Own private value
	 */
	gcry_mpi_t xa;

	/**
	 * Own public value
	 */
	gcry_mpi_t ya;

	/**
	 * Other public value
	 */
	gcry_mpi_t yb;

	/**
	 * Shared secret
	 */
	gcry_mpi_t zz;

	/**
	 * Modulus
	 */
	gcry_mpi_t p;

	/**
	 * Modulus length.
	 */
	size_t p_len;
};

METHOD(diffie_hellman_t, set_other_public_value, bool,
	private_gcrypt_dh_t *this, chunk_t value)
{
	gcry_mpi_t p_min_1;
	gcry_error_t err;

	if (!diffie_hellman_verify_value(this->group, value))
	{
		return FALSE;
	}

	if (this->yb)
	{
		gcry_mpi_release(this->yb);
		this->yb = NULL;
	}
	err = gcry_mpi_scan(&this->yb, GCRYMPI_FMT_USG, value.ptr, value.len, NULL);
	if (err)
	{
		DBG1(DBG_LIB, "importing mpi yb failed: %s", gpg_strerror(err));
		return FALSE;
	}

	p_min_1 = gcry_mpi_new(this->p_len * 8);
	gcry_mpi_sub_ui(p_min_1, this->p, 1);

	/* check public value:
	 * 1. 0 or 1 is invalid as 0^a = 0 and 1^a = 1
	 * 2. a public value larger or equal the modulus is invalid */
	if (gcry_mpi_cmp_ui(this->yb, 1) > 0 &&
		gcry_mpi_cmp(this->yb, p_min_1) < 0)
	{
		if (!this->zz)
		{
			this->zz = gcry_mpi_new(this->p_len * 8);
		}
		gcry_mpi_powm(this->zz, this->yb, this->xa, this->p);
	}
	else
	{
		DBG1(DBG_LIB, "public DH value verification failed:"
			 " y < 2 || y > p - 1 ");
	}
	gcry_mpi_release(p_min_1);
	return this->zz != NULL;
}

/**
 * export a gcry_mpi to an allocated chunk of len bytes
 */
static chunk_t export_mpi(gcry_mpi_t value, size_t len)
{
	chunk_t chunk;
	size_t written;

	chunk = chunk_alloc(len);
	gcry_mpi_print(GCRYMPI_FMT_USG, chunk.ptr, chunk.len, &written, value);
	if (written < len)
	{	/* right-align number of written bytes in chunk */
		memmove(chunk.ptr + (len - written), chunk.ptr, written);
		memset(chunk.ptr, 0, len - written);
	}
	return chunk;
}

METHOD(diffie_hellman_t, get_my_public_value, bool,
	private_gcrypt_dh_t *this, chunk_t *value)
{
	*value = export_mpi(this->ya, this->p_len);
	return TRUE;
}

METHOD(diffie_hellman_t, set_private_value, bool,
	private_gcrypt_dh_t *this, chunk_t value)
{
	gcry_error_t err;
	gcry_mpi_t xa;

	err = gcry_mpi_scan(&xa, GCRYMPI_FMT_USG, value.ptr, value.len, NULL);
	if (!err)
	{
		gcry_mpi_release(this->xa);
		this->xa = xa;
		gcry_mpi_powm(this->ya, this->g, this->xa, this->p);
		gcry_mpi_release(this->zz);
		this->zz = NULL;
	}
	return !err;
}

METHOD(diffie_hellman_t, get_shared_secret, bool,
	private_gcrypt_dh_t *this, chunk_t *secret)
{
	if (!this->zz)
	{
		return FALSE;
	}
	*secret = export_mpi(this->zz, this->p_len);
	return TRUE;
}

METHOD(diffie_hellman_t, get_dh_group, diffie_hellman_group_t,
	private_gcrypt_dh_t *this)
{
	return this->group;
}

METHOD(diffie_hellman_t, destroy, void,
	private_gcrypt_dh_t *this)
{
	gcry_mpi_release(this->p);
	gcry_mpi_release(this->xa);
	gcry_mpi_release(this->ya);
	gcry_mpi_release(this->g);
	gcry_mpi_release(this->yb);
	gcry_mpi_release(this->zz);
	free(this);
}

/*
 * Generic internal constructor
 */
static gcrypt_dh_t *create_generic(diffie_hellman_group_t group, size_t exp_len,
								   chunk_t g, chunk_t p)
{
	private_gcrypt_dh_t *this;
	gcry_error_t err;
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
		.p_len = p.len,
	);
	err = gcry_mpi_scan(&this->p, GCRYMPI_FMT_USG, p.ptr, p.len, NULL);
	if (err)
	{
		DBG1(DBG_LIB, "importing mpi modulus failed: %s", gpg_strerror(err));
		free(this);
		return NULL;
	}
	err = gcry_mpi_scan(&this->g, GCRYMPI_FMT_USG, g.ptr, g.len, NULL);
	if (err)
	{
		DBG1(DBG_LIB, "importing mpi generator failed: %s", gpg_strerror(err));
		gcry_mpi_release(this->p);
		free(this);
		return NULL;
	}

	rng = lib->crypto->create_rng(lib->crypto, RNG_STRONG);
	if (rng && rng->allocate_bytes(rng, exp_len, &random))
	{	/* prefer external randomizer */
		rng->destroy(rng);
		err = gcry_mpi_scan(&this->xa, GCRYMPI_FMT_USG,
							random.ptr, random.len, NULL);
		chunk_clear(&random);
		if (err)
		{
			DBG1(DBG_LIB, "importing mpi xa failed: %s", gpg_strerror(err));
			gcry_mpi_release(this->p);
			gcry_mpi_release(this->g);
			free(this);
			return NULL;
		}
	}
	else
	{	/* fallback to gcrypt internal randomizer, shouldn't ever happen */
		DESTROY_IF(rng);
		this->xa = gcry_mpi_new(exp_len * 8);
		gcry_mpi_randomize(this->xa, exp_len * 8, GCRY_STRONG_RANDOM);
	}
	if (exp_len == this->p_len)
	{
		/* achieve bitsof(p)-1 by setting MSB to 0 */
		gcry_mpi_clear_bit(this->xa, exp_len * 8 - 1);
	}

	this->ya = gcry_mpi_new(this->p_len * 8);

	gcry_mpi_powm(this->ya, this->g, this->xa, this->p);

	return &this->public;
}


/*
 * Described in header.
 */
gcrypt_dh_t *gcrypt_dh_create(diffie_hellman_group_t group)
{

	diffie_hellman_params_t *params;

	params = diffie_hellman_get_params(group);
	if (!params)
	{
		return NULL;
	}
	return create_generic(group, params->exp_len,
						  params->generator, params->prime);
}

/*
 * Described in header.
 */
gcrypt_dh_t *gcrypt_dh_create_custom(diffie_hellman_group_t group, ...)
{
	if (group == MODP_CUSTOM)
	{
		chunk_t g, p;

		VA_ARGS_GET(group, g, p);
		return create_generic(group, p.len, g, p);
	}
	return NULL;
}
