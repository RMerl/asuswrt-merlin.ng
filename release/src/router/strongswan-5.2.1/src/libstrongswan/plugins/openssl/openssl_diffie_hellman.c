/*
 * Copyright (C) 2008-2010 Tobias Brunner
 * Copyright (C) 2008 Martin Willi
 * Hochschule fuer Technik Rapperswil
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

#include <openssl/opensslconf.h>

#ifndef OPENSSL_NO_DH

#include <openssl/dh.h>

#include "openssl_diffie_hellman.h"

#include <utils/debug.h>

typedef struct private_openssl_diffie_hellman_t private_openssl_diffie_hellman_t;

/**
 * Private data of an openssl_diffie_hellman_t object.
 */
struct private_openssl_diffie_hellman_t {
	/**
	 * Public openssl_diffie_hellman_t interface.
	 */
	openssl_diffie_hellman_t public;

	/**
	 * Diffie Hellman group number.
	 */
	diffie_hellman_group_t group;

	/**
	 * Diffie Hellman object
	 */
	DH *dh;

	/**
	 * Other public value
	 */
	BIGNUM *pub_key;

	/**
	 * Shared secret
	 */
	chunk_t shared_secret;

	/**
	 * True if shared secret is computed
	 */
	bool computed;
};

METHOD(diffie_hellman_t, get_my_public_value, void,
	private_openssl_diffie_hellman_t *this, chunk_t *value)
{
	*value = chunk_alloc(DH_size(this->dh));
	memset(value->ptr, 0, value->len);
	BN_bn2bin(this->dh->pub_key,
			  value->ptr + value->len - BN_num_bytes(this->dh->pub_key));
}

METHOD(diffie_hellman_t, get_shared_secret, status_t,
	private_openssl_diffie_hellman_t *this, chunk_t *secret)
{
	if (!this->computed)
	{
		return FAILED;
	}
	/* shared secret should requires a len according the DH group */
	*secret = chunk_alloc(DH_size(this->dh));
	memset(secret->ptr, 0, secret->len);
	memcpy(secret->ptr + secret->len - this->shared_secret.len,
		   this->shared_secret.ptr, this->shared_secret.len);
	return SUCCESS;
}


METHOD(diffie_hellman_t, set_other_public_value, void,
	private_openssl_diffie_hellman_t *this, chunk_t value)
{
	int len;

	BN_bin2bn(value.ptr, value.len, this->pub_key);
	chunk_clear(&this->shared_secret);
	this->shared_secret.ptr = malloc(DH_size(this->dh));
	memset(this->shared_secret.ptr, 0xFF, this->shared_secret.len);
	len = DH_compute_key(this->shared_secret.ptr, this->pub_key, this->dh);
	if (len < 0)
	{
		DBG1(DBG_LIB, "DH shared secret computation failed");
		return;
	}
	this->shared_secret.len = len;
	this->computed = TRUE;
}

METHOD(diffie_hellman_t, get_dh_group, diffie_hellman_group_t,
	private_openssl_diffie_hellman_t *this)
{
	return this->group;
}

/**
 * Lookup the modulus in modulo table
 */
static status_t set_modulus(private_openssl_diffie_hellman_t *this)
{
	diffie_hellman_params_t *params = diffie_hellman_get_params(this->group);
	if (!params)
	{
		return NOT_FOUND;
	}
	this->dh->p = BN_bin2bn(params->prime.ptr, params->prime.len, NULL);
	this->dh->g = BN_bin2bn(params->generator.ptr, params->generator.len, NULL);
	if (params->exp_len != params->prime.len)
	{
		this->dh->length = params->exp_len * 8;
	}
	return SUCCESS;
}

METHOD(diffie_hellman_t, destroy, void,
	private_openssl_diffie_hellman_t *this)
{
	BN_clear_free(this->pub_key);
	DH_free(this->dh);
	chunk_clear(&this->shared_secret);
	free(this);
}

/*
 * Described in header.
 */
openssl_diffie_hellman_t *openssl_diffie_hellman_create(
							diffie_hellman_group_t group, chunk_t g, chunk_t p)
{
	private_openssl_diffie_hellman_t *this;

	INIT(this,
		.public = {
			.dh = {
				.get_shared_secret = _get_shared_secret,
				.set_other_public_value = _set_other_public_value,
				.get_my_public_value = _get_my_public_value,
				.get_dh_group = _get_dh_group,
				.destroy = _destroy,
			},
		},
	);

	this->dh = DH_new();
	if (!this->dh)
	{
		free(this);
		return NULL;
	}

	this->group = group;
	this->computed = FALSE;
	this->pub_key = BN_new();
	this->shared_secret = chunk_empty;

	if (group == MODP_CUSTOM)
	{
		this->dh->p = BN_bin2bn(p.ptr, p.len, NULL);
		this->dh->g = BN_bin2bn(g.ptr, g.len, NULL);
	}
	else
	{
		/* find a modulus according to group */
		if (set_modulus(this) != SUCCESS)
		{
			destroy(this);
			return NULL;
		}
	}

	/* generate my public and private values */
	if (!DH_generate_key(this->dh))
	{
		destroy(this);
		return NULL;
	}
	DBG2(DBG_LIB, "size of DH secret exponent: %d bits",
		 BN_num_bits(this->dh->priv_key));

	return &this->public;
}

#endif /* OPENSSL_NO_DH */
