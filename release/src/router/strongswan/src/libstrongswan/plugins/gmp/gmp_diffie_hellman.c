/*
 * Copyright (C) 1998-2002  D. Hugh Redelmeier.
 * Copyright (C) 1999, 2000, 2001  Henry Spencer.
 * Copyright (C) 2010 Tobias Brunner
 * Copyright (C) 2005-2008 Martin Willi
 * Copyright (C) 2005 Jan Hutter
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

#include <gmp.h>

#include "gmp_diffie_hellman.h"

#include <utils/debug.h>

#ifdef HAVE_MPZ_POWM_SEC
# undef mpz_powm
# define mpz_powm mpz_powm_sec
#endif

typedef struct private_gmp_diffie_hellman_t private_gmp_diffie_hellman_t;

/**
 * Private data of an gmp_diffie_hellman_t object.
 */
struct private_gmp_diffie_hellman_t {
	/**
	 * Public gmp_diffie_hellman_t interface.
	 */
	gmp_diffie_hellman_t public;

	/**
	 * Diffie Hellman group number.
	 */
	diffie_hellman_group_t group;

	/*
	 * Generator value.
	 */
	mpz_t g;

	/**
	 * My private value.
	 */
	mpz_t xa;

	/**
	 * My public value.
	 */
	mpz_t ya;

	/**
	 * Other public value.
	 */
	mpz_t yb;

	/**
	 * Shared secret.
	 */
	mpz_t zz;

	/**
	 * Modulus.
	 */
	mpz_t p;

	/**
	 * Modulus length.
	 */
	size_t p_len;

	/**
	 * True if shared secret is computed and stored in my_public_value.
	 */
	bool computed;
};

METHOD(diffie_hellman_t, set_other_public_value, bool,
	private_gmp_diffie_hellman_t *this, chunk_t value)
{
	mpz_t p_min_1;

	if (!diffie_hellman_verify_value(this->group, value))
	{
		return FALSE;
	}

	mpz_init(p_min_1);
	mpz_sub_ui(p_min_1, this->p, 1);

	mpz_import(this->yb, value.len, 1, 1, 1, 0, value.ptr);

	/* check public value:
	 * 1. 0 or 1 is invalid as 0^a = 0 and 1^a = 1
	 * 2. a public value larger or equal the modulus is invalid */
	if (mpz_cmp_ui(this->yb, 1) > 0 &&
		mpz_cmp(this->yb, p_min_1) < 0)
	{
#ifdef EXTENDED_DH_TEST
		/* 3. test if y ^ q mod p = 1, where q = (p - 1)/2. */
		mpz_t q, one;
		diffie_hellman_params_t *params;

		mpz_init(q);
		mpz_init(one);

		params = diffie_hellman_get_params(this->group);
		if (!params->subgroup.len)
		{
			mpz_fdiv_q_2exp(q, p_min_1, 1);
		}
		else
		{
			mpz_import(q, params->subgroup.len, 1, 1, 1, 0, params->subgroup.ptr);
		}
		mpz_powm(one, this->yb, q, this->p);
		mpz_clear(q);
		if (mpz_cmp_ui(one, 1) == 0)
		{
			mpz_powm(this->zz, this->yb, this->xa, this->p);
			this->computed = TRUE;
		}
		else
		{
			DBG1(DBG_LIB, "public DH value verification failed:"
				 " y ^ q mod p != 1");
		}
		mpz_clear(one);
#else
		mpz_powm(this->zz, this->yb, this->xa, this->p);
		this->computed = TRUE;
#endif
	}
	else
	{
		DBG1(DBG_LIB, "public DH value verification failed:"
			 " y < 2 || y > p - 1 ");
	}
	mpz_clear(p_min_1);
	return this->computed;
}

METHOD(diffie_hellman_t, get_my_public_value, bool,
	private_gmp_diffie_hellman_t *this,chunk_t *value)
{
	value->len = this->p_len;
	value->ptr = mpz_export(NULL, NULL, 1, value->len, 1, 0, this->ya);
	if (value->ptr == NULL)
	{
		value->len = 0;
	}
	return TRUE;
}

METHOD(diffie_hellman_t, set_private_value, bool,
	private_gmp_diffie_hellman_t *this, chunk_t value)
{
	mpz_import(this->xa, value.len, 1, 1, 1, 0, value.ptr);
	mpz_powm(this->ya, this->g, this->xa, this->p);
	this->computed = FALSE;
	return TRUE;
}

METHOD(diffie_hellman_t, get_shared_secret, bool,
	private_gmp_diffie_hellman_t *this, chunk_t *secret)
{
	if (!this->computed)
	{
		return FALSE;
	}
	secret->len = this->p_len;
	secret->ptr = mpz_export(NULL, NULL, 1, secret->len, 1, 0, this->zz);
	if (secret->ptr == NULL)
	{
		return FALSE;
	}
	return TRUE;
}

METHOD(diffie_hellman_t, get_dh_group, diffie_hellman_group_t,
	private_gmp_diffie_hellman_t *this)
{
	return this->group;
}

METHOD(diffie_hellman_t, destroy, void,
	private_gmp_diffie_hellman_t *this)
{
	mpz_clear(this->p);
	mpz_clear(this->xa);
	mpz_clear(this->ya);
	mpz_clear(this->yb);
	mpz_clear(this->zz);
	mpz_clear(this->g);
	free(this);
}

/**
 * Generic internal constructor
 */
static gmp_diffie_hellman_t *create_generic(diffie_hellman_group_t group,
											size_t exp_len, chunk_t g, chunk_t p)
{
	private_gmp_diffie_hellman_t *this;
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

	mpz_init(this->p);
	mpz_init(this->yb);
	mpz_init(this->ya);
	mpz_init(this->xa);
	mpz_init(this->zz);
	mpz_init(this->g);
	mpz_import(this->g, g.len, 1, 1, 1, 0, g.ptr);
	mpz_import(this->p, p.len, 1, 1, 1, 0, p.ptr);

	rng = lib->crypto->create_rng(lib->crypto, RNG_STRONG);
	if (!rng)
	{
		DBG1(DBG_LIB, "no RNG found for quality %N", rng_quality_names,
			 RNG_STRONG);
		destroy(this);
		return NULL;
	}
	if (!rng->allocate_bytes(rng, exp_len, &random))
	{
		DBG1(DBG_LIB, "failed to allocate DH secret");
		rng->destroy(rng);
		destroy(this);
		return NULL;
	}
	rng->destroy(rng);

	if (exp_len == this->p_len)
	{
		/* achieve bitsof(p)-1 by setting MSB to 0 */
		*random.ptr &= 0x7F;
	}
	mpz_import(this->xa, random.len, 1, 1, 1, 0, random.ptr);
	chunk_clear(&random);
	DBG2(DBG_LIB, "size of DH secret exponent: %u bits",
		 mpz_sizeinbase(this->xa, 2));

	mpz_powm(this->ya, this->g, this->xa, this->p);

	return &this->public;
}

/*
 * Described in header
 */
gmp_diffie_hellman_t *gmp_diffie_hellman_create(diffie_hellman_group_t group)
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
 * Described in header
 */
gmp_diffie_hellman_t *gmp_diffie_hellman_create_custom(
											diffie_hellman_group_t group, ...)
{
	if (group == MODP_CUSTOM)
	{
		chunk_t g, p;

		VA_ARGS_GET(group, g, p);
		return create_generic(MODP_CUSTOM, p.len, g, p);
	}
	return NULL;
}
