/*
 * Copyright (C) 2008-2021 Tobias Brunner
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

#ifndef OPENSSL_NO_EC

#include <openssl/evp.h>
#include <openssl/ec.h>
#include <openssl/objects.h>

#if OPENSSL_VERSION_NUMBER < 0x1010000fL
#include <openssl/bn.h>
#elif OPENSSL_VERSION_NUMBER >= 0x30000000L
#include <openssl/bn.h>
#include <openssl/param_build.h>
#include <openssl/core_names.h>
#endif

#if OPENSSL_VERSION_NUMBER < 0x30000000L
#define EVP_PKEY_set1_encoded_public_key EVP_PKEY_set1_tls_encodedpoint
#define EVP_PKEY_get1_encoded_public_key EVP_PKEY_get1_tls_encodedpoint
#endif

#include "openssl_ec_diffie_hellman.h"
#include "openssl_util.h"

#include <utils/debug.h>

typedef struct private_openssl_ec_diffie_hellman_t private_openssl_ec_diffie_hellman_t;

/**
 * Private data of an openssl_ec_diffie_hellman_t object.
 */
struct private_openssl_ec_diffie_hellman_t {
	/**
	 * Public openssl_ec_diffie_hellman_t interface.
	 */
	openssl_ec_diffie_hellman_t public;

	/**
	 * Diffie Hellman group number.
	 */
	diffie_hellman_group_t group;

	/**
	 * EC private (public) key
	 */
	EVP_PKEY *key;

	/**
	 * Public key provided by peer
	 */
	EVP_PKEY *pub;

	/**
	 * EC group
	 */
	EC_GROUP *ec_group;

	/**
	 * Shared secret
	 */
	chunk_t shared_secret;

	/**
	 * True if shared secret is computed
	 */
	bool computed;
};

#if OPENSSL_VERSION_NUMBER < 0x1010000fL
/**
 * Convert a chunk to an EC_POINT and set it on the given key. The x and y
 * coordinates of the point have to be concatenated in the chunk.
 */
static bool chunk2ecp(const EC_GROUP *group, chunk_t chunk, EVP_PKEY *key)
{
	EC_POINT *point = NULL;
	EC_KEY *pub = NULL;
	BN_CTX *ctx;
	BIGNUM *x, *y;
	bool ret = FALSE;

	ctx = BN_CTX_new();
	if (!ctx)
	{
		return FALSE;
	}

	BN_CTX_start(ctx);
	x = BN_CTX_get(ctx);
	y = BN_CTX_get(ctx);
	if (!x || !y)
	{
		goto error;
	}

	if (!openssl_bn_split(chunk, x, y))
	{
		goto error;
	}

	point = EC_POINT_new(group);
	if (!point || !EC_POINT_set_affine_coordinates_GFp(group, point, x, y, ctx))
	{
		goto error;
	}

	if (!EC_POINT_is_on_curve(group, point, ctx))
	{
		goto error;
	}

	pub = EC_KEY_new();
	if (!pub || !EC_KEY_set_group(pub, group))
	{
		goto error;
	}

	if (EC_KEY_set_public_key(pub, point) != 1)
	{
		goto error;
	}

	if (EVP_PKEY_set1_EC_KEY(key, pub) != 1)
	{
		goto error;
	}

	ret = TRUE;

error:
	EC_POINT_clear_free(point);
	EC_KEY_free(pub);
	BN_CTX_end(ctx);
	BN_CTX_free(ctx);
	return ret;
}

/**
 * Convert a key to a chunk by concatenating the x and y coordinates of
 * the underlying EC point. This function allocates memory for the chunk.
 */
static bool ecp2chunk(const EC_GROUP *group, EVP_PKEY *key, chunk_t *chunk)
{
	EC_KEY *ec_key = NULL;
	const EC_POINT *point;
	BN_CTX *ctx;
	BIGNUM *x, *y;
	bool ret = FALSE;

	ctx = BN_CTX_new();
	if (!ctx)
	{
		return FALSE;
	}

	BN_CTX_start(ctx);
	x = BN_CTX_get(ctx);
	y = BN_CTX_get(ctx);
	if (!x || !y)
	{
		goto error;
	}

	ec_key = EVP_PKEY_get1_EC_KEY(key);
	point = EC_KEY_get0_public_key(ec_key);
	if (!point || !EC_POINT_get_affine_coordinates_GFp(group, point, x, y, ctx))
	{
		goto error;
	}

	if (!openssl_bn_cat(EC_FIELD_ELEMENT_LEN(group), x, y, chunk))
	{
		goto error;
	}

	ret = chunk->len != 0;
error:
	EC_KEY_free(ec_key);
	BN_CTX_end(ctx);
	BN_CTX_free(ctx);
	return ret;
}
#endif /* OPENSSL_VERSION_NUMBER < ... */

METHOD(diffie_hellman_t, set_other_public_value, bool,
	private_openssl_ec_diffie_hellman_t *this, chunk_t value)
{
	if (!diffie_hellman_verify_value(this->group, value))
	{
		return FALSE;
	}

	if (!this->pub)
	{
		this->pub = EVP_PKEY_new();
	}

#if OPENSSL_VERSION_NUMBER < 0x1010000fL
	if (!chunk2ecp(this->ec_group, value, this->pub))
	{
		DBG1(DBG_LIB, "ECDH public value is malformed");
		return FALSE;
	}
#else
	/* OpenSSL expects the pubkey in the format specified in section 2.3.4 of
	 * SECG SEC 1, i.e. prefixed with 0x04 to indicate an uncompressed point */
	value = chunk_cata("cc", chunk_from_chars(0x04), value);
	if (EVP_PKEY_copy_parameters(this->pub, this->key) <= 0 ||
		EVP_PKEY_set1_encoded_public_key(this->pub, value.ptr, value.len) <= 0)
	{
		DBG1(DBG_LIB, "ECDH public value is malformed");
		return FALSE;
	}
#endif
	chunk_clear(&this->shared_secret);
	return TRUE;
}

METHOD(diffie_hellman_t, get_my_public_value, bool,
	private_openssl_ec_diffie_hellman_t *this, chunk_t *value)
{
#if OPENSSL_VERSION_NUMBER < 0x1010000fL
	return ecp2chunk(this->ec_group, this->key, value);
#else
	chunk_t pub;

	/* OpenSSL returns the pubkey in the format specified in section 2.3.4 of
	 * SECG SEC 1, i.e. prefixed with 0x04 to indicate an uncompressed point */
	pub.len = EVP_PKEY_get1_encoded_public_key(this->key, &pub.ptr);
	if (pub.len != 0)
	{
		*value = chunk_clone(chunk_skip(pub, 1));
		OPENSSL_free(pub.ptr);
		return value->len != 0;
	}
	return FALSE;
#endif
}

METHOD(diffie_hellman_t, get_shared_secret, bool,
	private_openssl_ec_diffie_hellman_t *this, chunk_t *secret)
{
	if (!this->shared_secret.len &&
		!openssl_compute_shared_key(this->key, this->pub, &this->shared_secret))
	{
		DBG1(DBG_LIB, "ECDH shared secret computation failed");
		return FALSE;
	}
	*secret = chunk_clone(this->shared_secret);
	return TRUE;
}

METHOD(diffie_hellman_t, get_dh_group, diffie_hellman_group_t,
	private_openssl_ec_diffie_hellman_t *this)
{
	return this->group;
}

/*
 * Described in header
 */
int openssl_ecdh_group_to_nid(diffie_hellman_group_t group)
{
	switch (group)
	{
		case ECP_192_BIT:
			return NID_X9_62_prime192v1;
		case ECP_224_BIT:
			return NID_secp224r1;
		case ECP_256_BIT:
			return NID_X9_62_prime256v1;
		case ECP_384_BIT:
			return NID_secp384r1;
		case ECP_521_BIT:
			return NID_secp521r1;
/* added with 1.0.2 */
#if OPENSSL_VERSION_NUMBER >= 0x10002000L
		case ECP_224_BP:
			return NID_brainpoolP224r1;
		case ECP_256_BP:
			return NID_brainpoolP256r1;
		case ECP_384_BP:
			return NID_brainpoolP384r1;
		case ECP_512_BP:
			return NID_brainpoolP512r1;
#endif
		default:
			return 0;
	}
}

/**
 * Parse the given private key as BIGNUM and calculate the corresponding public
 * key as EC_POINT.
 */
static bool get_keypair(EC_GROUP *group, chunk_t value, BIGNUM **priv,
						EC_POINT **pub)
{
	*priv = BN_bin2bn(value.ptr, value.len, NULL);
	*pub = EC_POINT_new(group);
	return *priv && *pub &&
			EC_POINT_mul(group, *pub, *priv, NULL, NULL, NULL) == 1;
}

#if OPENSSL_VERSION_NUMBER >= 0x30000000L

/**
 * Convert the given EC_POINT to a chunk.
 */
static bool ecp2chunk(EC_GROUP *group, EC_POINT *point, chunk_t *chunk)
{
	BN_CTX *ctx = BN_CTX_new();

	if (ctx)
	{
		chunk->len = EC_POINT_point2buf(group, point,
										POINT_CONVERSION_UNCOMPRESSED,
										&chunk->ptr, ctx);
	}
	BN_CTX_free(ctx);
	return chunk->len;
}

METHOD(diffie_hellman_t, set_private_value, bool,
	private_openssl_ec_diffie_hellman_t *this, chunk_t value)
{
	BIGNUM *priv = NULL;
	EC_POINT *pub = NULL;
	chunk_t pub_chunk = chunk_empty;
	const char *name;
	OSSL_PARAM_BLD *bld;
	OSSL_PARAM *params = NULL;
	EVP_PKEY_CTX *ctx;
	EVP_PKEY *key = NULL;
	bool ret = FALSE;

	if (!get_keypair(this->ec_group, value, &priv, &pub) ||
		!ecp2chunk(this->ec_group, pub, &pub_chunk))
	{
		EC_POINT_free(pub);
		BN_free(priv);
		return FALSE;
	}
	EC_POINT_free(pub);

	name = OSSL_EC_curve_nid2name(openssl_ecdh_group_to_nid(this->group));
	bld = OSSL_PARAM_BLD_new();
	if (name && bld &&
		OSSL_PARAM_BLD_push_utf8_string(bld, OSSL_PKEY_PARAM_GROUP_NAME,
										(char*)name, 0) &&
		OSSL_PARAM_BLD_push_octet_string(bld, OSSL_PKEY_PARAM_PUB_KEY,
										 pub_chunk.ptr, pub_chunk.len) &&
		OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_PRIV_KEY, priv))
	{
		params = OSSL_PARAM_BLD_to_param(bld);
	}
	OSSL_PARAM_BLD_free(bld);
	chunk_free(&pub_chunk);
	BN_free(priv);

	ctx = EVP_PKEY_CTX_new_from_name(NULL, "EC", NULL);
	if (params && ctx &&
		EVP_PKEY_fromdata_init(ctx) > 0 &&
		EVP_PKEY_fromdata(ctx, &key, EVP_PKEY_KEYPAIR, params) > 0)
	{
		EVP_PKEY_free(this->key);
		this->key = key;
		ret = TRUE;
	}
	EVP_PKEY_CTX_free(ctx);
	OSSL_PARAM_free(params);
	return ret;
}

#else /* OPENSSL_VERSION_NUMBER */

METHOD(diffie_hellman_t, set_private_value, bool,
	private_openssl_ec_diffie_hellman_t *this, chunk_t value)
{
	EC_KEY *key = NULL;
	EC_POINT *pub = NULL;
	BIGNUM *priv = NULL;
	bool ret = FALSE;

	if (!get_keypair(this->ec_group, value, &priv, &pub))
	{
		goto error;
	}
	key = EC_KEY_new();
	if (!key || !EC_KEY_set_group(key, this->ec_group))
	{
		goto error;
	}
	if (EC_KEY_set_private_key(key, priv) != 1)
	{
		goto error;
	}
	if (EC_KEY_set_public_key(key, pub) != 1)
	{
		goto error;
	}
	if (EVP_PKEY_set1_EC_KEY(this->key, key) != 1)
	{
		goto error;
	}
	ret = TRUE;

error:
	EC_POINT_free(pub);
	BN_free(priv);
	EC_KEY_free(key);
	return ret;
}

#endif /* OPENSSL_VERSION_NUMBER */

METHOD(diffie_hellman_t, destroy, void,
	private_openssl_ec_diffie_hellman_t *this)
{
	EC_GROUP_free(this->ec_group);
	EVP_PKEY_free(this->key);
	EVP_PKEY_free(this->pub);
	chunk_clear(&this->shared_secret);
	free(this);
}

/*
 * Described in header
 */
openssl_ec_diffie_hellman_t *openssl_ec_diffie_hellman_create(diffie_hellman_group_t group)
{
	private_openssl_ec_diffie_hellman_t *this;
	int curve;

	curve = openssl_ecdh_group_to_nid(group);
	if (!curve)
	{
		return NULL;
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
		.group = group,
	);

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
	this->ec_group = EC_GROUP_new_by_curve_name(curve);
	this->key = EVP_EC_gen(OSSL_EC_curve_nid2name(curve));
	if (!this->key)
	{
		destroy(this);
		return NULL;
	}
#else
	EC_KEY *key = EC_KEY_new_by_curve_name(curve);
	if (!key || !EC_KEY_generate_key(key))
	{
		EC_KEY_free(key);
		destroy(this);
		return NULL;
	}
	this->ec_group = EC_GROUP_dup(EC_KEY_get0_group(key));
	this->key = EVP_PKEY_new();
	if (!this->key || !EVP_PKEY_assign_EC_KEY(this->key, key))
	{
		EC_KEY_free(key);
		destroy(this);
		return NULL;
	}
#endif

	return &this->public;
}

#endif /* OPENSSL_NO_EC */
