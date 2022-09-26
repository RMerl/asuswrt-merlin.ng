/*
 * Copyright (C) 2008-2010 Tobias Brunner
 * Copyright (C) 2008 Martin Willi
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

#include <openssl/opensslconf.h>

#ifndef OPENSSL_NO_DH

#include <openssl/evp.h>
#include <openssl/bn.h>
#include <openssl/dh.h>

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
#include <openssl/param_build.h>
#include <openssl/core_names.h>
#endif

#include "openssl_diffie_hellman.h"
#include "openssl_util.h"

#include <utils/debug.h>

/* these were added with 1.1.0 when DH was made opaque */
#if OPENSSL_VERSION_NUMBER < 0x10100000L
OPENSSL_KEY_FALLBACK(DH, key, pub_key, priv_key)
OPENSSL_KEY_FALLBACK(DH, pqg, p, q, g)
#define DH_set_length(dh, len) ({ (dh)->length = len; 1; })
#endif

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

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
	/**
	 * Diffie Hellman key
	 */
	EVP_PKEY *key;

	/**
	 * Other public value
	 */
	EVP_PKEY *pub;
#else
	/**
	 * Diffie Hellman object
	 */
	DH *dh;

	/**
	 * Other public value
	 */
	BIGNUM *pub_key;
#endif

	/**
	 * Shared secret
	 */
	chunk_t shared_secret;
};

METHOD(diffie_hellman_t, get_dh_group, diffie_hellman_group_t,
	private_openssl_diffie_hellman_t *this)
{
	return this->group;
}

METHOD(diffie_hellman_t, get_my_public_value, bool,
	private_openssl_diffie_hellman_t *this, chunk_t *value)
{
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
	chunk_t pub;

	pub.len = EVP_PKEY_get1_encoded_public_key(this->key, &pub.ptr);
	if (pub.len != 0)
	{
		*value = chunk_clone(pub);
		OPENSSL_free(pub.ptr);
		return value->len != 0;
	}
	return FALSE;
#else
	const BIGNUM *pubkey;

	*value = chunk_alloc(DH_size(this->dh));
	memset(value->ptr, 0, value->len);
	DH_get0_key(this->dh, &pubkey, NULL);
	BN_bn2bin(pubkey, value->ptr + value->len - BN_num_bytes(pubkey));
	return TRUE;
#endif
}

METHOD(diffie_hellman_t, get_shared_secret, bool,
	private_openssl_diffie_hellman_t *this, chunk_t *secret)
{
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
	if (!this->shared_secret.len &&
		!openssl_compute_shared_key(this->key, this->pub, &this->shared_secret))
	{
		DBG1(DBG_LIB, "DH shared secret computation failed");
		return FALSE;
	}
	*secret = chunk_clone(this->shared_secret);
	return TRUE;
#else
	int len;

	if (!this->shared_secret.len)
	{
		this->shared_secret = chunk_alloc(DH_size(this->dh));
		memset(this->shared_secret.ptr, 0xFF, this->shared_secret.len);
		len = DH_compute_key(this->shared_secret.ptr, this->pub_key, this->dh);
		if (len < 0)
		{
			DBG1(DBG_LIB, "DH shared secret computation failed");
			chunk_clear(&this->shared_secret);
			return FALSE;
		}
		this->shared_secret.len = len;
	}
	/* shared secret requires a length according to the DH group */
	*secret = chunk_copy_pad(chunk_alloc(DH_size(this->dh)),
							 this->shared_secret, 0);
	return TRUE;
#endif
}


METHOD(diffie_hellman_t, set_other_public_value, bool,
	private_openssl_diffie_hellman_t *this, chunk_t value)
{
	if (!diffie_hellman_verify_value(this->group, value))
	{
		return FALSE;
	}

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
	if (!this->pub)
	{
		this->pub = EVP_PKEY_new();
	}
	if (EVP_PKEY_copy_parameters(this->pub, this->key) <= 0 ||
		EVP_PKEY_set1_encoded_public_key(this->pub, value.ptr, value.len) <= 0)
	{
		DBG1(DBG_LIB, "DH public value is malformed");
		return FALSE;
	}
#else
	if (!BN_bin2bn(value.ptr, value.len, this->pub_key))
	{
		return FALSE;
	}
#endif
	chunk_clear(&this->shared_secret);
	return TRUE;
}

#if OPENSSL_VERSION_NUMBER >= 0x30000000L

/**
 * Calculate the public key for the given private key and DH parameters.
 * Setting only the private key and generating the public key internally is
 * not supported anymore with OpenSSL 3.0.0.
 */
static BIGNUM *calculate_public_key(BIGNUM *priv, const BIGNUM *g,
									const BIGNUM *p)
{
	BN_CTX *ctx = BN_CTX_new();
	BIGNUM *pub = BN_new();

	BN_set_flags(priv, BN_FLG_CONSTTIME);
	/* pub = g^priv mod p */
	if (!ctx || ! pub || !BN_mod_exp(pub, g, priv, p, ctx))
	{
		BN_free(pub);
		pub = NULL;
	}
	BN_CTX_free(ctx);
	return pub;
}

METHOD(diffie_hellman_t, set_private_value, bool,
	private_openssl_diffie_hellman_t *this, chunk_t value)
{
	BIGNUM *priv, *g = NULL, *p = NULL, *pub = NULL;
	OSSL_PARAM_BLD *bld = NULL;
	OSSL_PARAM *params = NULL;
	EVP_PKEY *key = NULL;
	EVP_PKEY_CTX *ctx = NULL;
	bool ret = FALSE;

	priv = BN_bin2bn(value.ptr, value.len, NULL);
	if (EVP_PKEY_get_bn_param(this->key, OSSL_PKEY_PARAM_FFC_G, &g) <= 0 ||
		EVP_PKEY_get_bn_param(this->key, OSSL_PKEY_PARAM_FFC_P, &p) <= 0)
	{
		goto error;
	}
	pub = calculate_public_key(priv, g, p);
	bld = OSSL_PARAM_BLD_new();
	if (pub && bld &&
		OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_FFC_G, g) &&
		OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_FFC_P, p) &&
		OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_PRIV_KEY, priv) &&
		OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_PUB_KEY, pub))
	{
		params = OSSL_PARAM_BLD_to_param(bld);
	}
	ctx = EVP_PKEY_CTX_new_from_name(NULL, "DH", NULL);
	if (!params || !ctx ||
		EVP_PKEY_fromdata_init(ctx) <= 0 ||
		EVP_PKEY_fromdata(ctx, &key, EVP_PKEY_KEYPAIR, params) <= 0)
	{
		goto error;
	}
	EVP_PKEY_free(this->key);
	this->key = key;
	ret = TRUE;

error:
	EVP_PKEY_CTX_free(ctx);
	OSSL_PARAM_free(params);
	OSSL_PARAM_BLD_free(bld);
	BN_free(pub);
	BN_free(p);
	BN_free(g);
	BN_free(priv);
	return ret;
}

#else /* OPENSSL_VERSION_NUMBER */

METHOD(diffie_hellman_t, set_private_value, bool,
	private_openssl_diffie_hellman_t *this, chunk_t value)
{
	BIGNUM *privkey;

	privkey = BN_bin2bn(value.ptr, value.len, NULL);
	if (privkey)
	{
		if (!DH_set0_key(this->dh, NULL, privkey))
		{
			return FALSE;
		}
		chunk_clear(&this->shared_secret);
		return DH_generate_key(this->dh);
	}
	return FALSE;
}

#endif /* OPENSSL_VERSION_NUMBER */

METHOD(diffie_hellman_t, destroy, void,
	private_openssl_diffie_hellman_t *this)
{
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
	EVP_PKEY_free(this->key);
	EVP_PKEY_free(this->pub);
#else
	BN_clear_free(this->pub_key);
	DH_free(this->dh);
#endif
	chunk_clear(&this->shared_secret);
	free(this);
}

/*
 * Described in header.
 */
openssl_diffie_hellman_t *openssl_diffie_hellman_create(
											diffie_hellman_group_t group, ...)
{
	private_openssl_diffie_hellman_t *this;
	BIGNUM *g, *p;
	int priv_len = 0;

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

	if (group == MODP_CUSTOM)
	{
		chunk_t g_chunk, p_chunk;

		VA_ARGS_GET(group, g_chunk, p_chunk);
		g = BN_bin2bn(g_chunk.ptr, g_chunk.len, NULL);
		p = BN_bin2bn(p_chunk.ptr, p_chunk.len, NULL);
	}
	else
	{
		diffie_hellman_params_t *params = diffie_hellman_get_params(group);
		if (!params)
		{
			destroy(this);
			return NULL;
		}
		g = BN_bin2bn(params->generator.ptr, params->generator.len, NULL);
		p = BN_bin2bn(params->prime.ptr, params->prime.len, NULL);
		if (params->exp_len != params->prime.len)
		{
			priv_len = params->exp_len * 8;
		}
	}

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
	OSSL_PARAM_BLD *bld;
	OSSL_PARAM *params = NULL;
	EVP_PKEY_CTX *ctx;

	/* if we abandoned MODP_CUSTOM, we could set OSSL_PKEY_PARAM_GROUP_NAME,
	 * which wouldn't require the first ctx/key for the parameters */
	bld = OSSL_PARAM_BLD_new();
	if (bld &&
		OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_FFC_G, g) &&
		OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_FFC_P, p) &&
		(!priv_len ||
		  OSSL_PARAM_BLD_push_int(bld, OSSL_PKEY_PARAM_DH_PRIV_LEN, priv_len)))
	{
		params = OSSL_PARAM_BLD_to_param(bld);
	}
	OSSL_PARAM_BLD_free(bld);
	BN_free(g);
	BN_free(p);

	ctx = EVP_PKEY_CTX_new_from_name(NULL, "DH", NULL);
	if (!params || !ctx ||
		EVP_PKEY_fromdata_init(ctx) <= 0 ||
		EVP_PKEY_fromdata(ctx, &this->key, EVP_PKEY_KEY_PARAMETERS, params) <= 0)
	{
		EVP_PKEY_CTX_free(ctx);
		OSSL_PARAM_free(params);
		destroy(this);
		return NULL;
	}
	OSSL_PARAM_free(params);
	EVP_PKEY_CTX_free(ctx);

	ctx = EVP_PKEY_CTX_new(this->key, NULL);
	if (!ctx ||
		EVP_PKEY_keygen_init(ctx) <= 0 ||
		EVP_PKEY_generate(ctx, &this->key) <= 0)
	{
		EVP_PKEY_CTX_free(ctx);
		destroy(this);
		return NULL;
	}
	EVP_PKEY_CTX_free(ctx);
#else /* OPENSSL_VERSION_NUMBER */
	this->dh = DH_new();
	this->pub_key = BN_new();
	if (!DH_set0_pqg(this->dh, p, NULL, g))
	{
		BN_free(g);
		BN_free(p);
		destroy(this);
		return NULL;
	}
	if (priv_len)
	{
#if defined(OPENSSL_IS_BORINGSSL) && \
	(!defined(BORINGSSL_API_VERSION) || BORINGSSL_API_VERSION < 11)
		this->dh->priv_length = priv_len;
#else
		if (!DH_set_length(this->dh, priv_len))
		{
			destroy(this);
			return NULL;
		}
#endif
	}
	if (!DH_generate_key(this->dh))
	{
		destroy(this);
		return NULL;
	}
#endif /* OPENSSL_VERSION_NUMBER */
	return &this->public;
}

#endif /* OPENSSL_NO_DH */
