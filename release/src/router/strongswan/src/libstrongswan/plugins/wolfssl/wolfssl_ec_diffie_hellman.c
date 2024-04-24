/*
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

#ifdef HAVE_ECC_DHE

#include <wolfssl/wolfcrypt/ecc.h>

#include "wolfssl_ec_diffie_hellman.h"
#include "wolfssl_util.h"

#include <utils/debug.h>

#if defined(ECC_TIMING_RESISTANT) && \
    (!defined(HAVE_FIPS) || \
     (defined(HAVE_FIPS_VERSION) && (HAVE_FIPS_VERSION >= 5)))
    #define USE_RNG_FOR_TIMING_RESISTANCE
#endif

#ifndef WOLFSSL_HAVE_ECC_KEY_GET_PRIV
    #define wc_ecc_key_get_priv(key) (&((key)->k))
#endif

typedef struct private_wolfssl_ec_diffie_hellman_t private_wolfssl_ec_diffie_hellman_t;

/**
 * Private data of an wolfssl_ec_diffie_hellman_t object.
 */
struct private_wolfssl_ec_diffie_hellman_t {
	/**
	 * Public wolfssl_ec_diffie_hellman_t interface.
	 */
	wolfssl_ec_diffie_hellman_t public;

	/**
	 * Diffie Hellman group number.
	 */
	key_exchange_method_t group;

	/**
	 * EC curve id for creating keys
	 */
	ecc_curve_id curve_id;

	/**
	 * Size of an ordinate in bytes
	 */
	int keysize;

	/**
	 * EC private (public) key
	 */
	ecc_key key;

	/**
	 * Public key provided by peer
	 */
	ecc_key pubkey;

	/**
	 * Shared secret
	 */
	chunk_t shared_secret;
};

/**
 * Convert an ec_point to a chunk by concatenating the x and y coordinates of
 * the point. This function allocates memory for the chunk.
 */
static bool ecp2chunk(int keysize, ecc_point *point, chunk_t *chunk,
					  bool x_coordinate_only)
{
	mp_int *y = NULL;

	if (!x_coordinate_only)
	{
		keysize *= 2;
		y = point->y;
	}
	return wolfssl_mp_cat(keysize, point->x, y, chunk);
}

/**
 * Perform the elliptic curve scalar multiplication.
 */
static bool wolfssl_ecc_multiply(const ecc_set_type *ecc_set, mp_int *scalar,
								 ecc_point *point, ecc_point *r)
{
	mp_int a, prime;
	int ret;

	if (mp_init(&a) != 0)
	{
		return FALSE;
	}
	if (mp_init(&prime) != 0)
	{
		mp_free(&a);
		return FALSE;
	}

	ret = mp_read_radix(&a, ecc_set->Af, MP_RADIX_HEX);
	if (ret == 0)
	{
		ret = mp_read_radix(&prime, ecc_set->prime, MP_RADIX_HEX);
	}
	if (ret == 0)
	{
		/* multiply the point by our secret */
		ret = wc_ecc_mulmod(scalar, point, r, &a, &prime, 1);
	}

	mp_free(&prime);
	mp_free(&a);

	return ret == 0;
}

METHOD(key_exchange_t, set_public_key, bool,
	private_wolfssl_ec_diffie_hellman_t *this, chunk_t value)
{
	chunk_t uncomp;

	if (!key_exchange_verify_pubkey(this->group, value))
	{
		return FALSE;
	}

	/* prepend 0x04 to indicate uncompressed point format */
	uncomp = chunk_cata("cc", chunk_from_chars(0x04), value);
	if (wc_ecc_import_x963_ex(uncomp.ptr, uncomp.len, &this->pubkey,
							  this->curve_id) != 0)
	{
		DBG1(DBG_LIB, "ECDH public value is malformed");
		return FALSE;
	}

	if (wc_ecc_check_key(&this->pubkey) != 0)
	{
		DBG1(DBG_LIB, "ECDH public value is invalid");
		return FALSE;
	}
	return TRUE;
}

METHOD(key_exchange_t, get_public_key, bool,
	private_wolfssl_ec_diffie_hellman_t *this,chunk_t *value)
{
	return ecp2chunk(this->keysize, &this->key.pubkey, value, FALSE);
}

METHOD(key_exchange_t, set_private_key, bool,
	private_wolfssl_ec_diffie_hellman_t *this, chunk_t value)
{
	bool success = FALSE;
	ecc_point *base;
	int ret;

	if ((base = wc_ecc_new_point()) == NULL)
	{
		return FALSE;
	}

	ret = mp_read_unsigned_bin(wc_ecc_key_get_priv(&this->key), value.ptr,
							   value.len);
	/* get base point */
	if (ret == 0)
	{
		ret = mp_read_radix(base->x, this->key.dp->Gx, MP_RADIX_HEX);
	}
	if (ret == 0)
	{
		ret = mp_read_radix(base->y, this->key.dp->Gy, MP_RADIX_HEX);
	}
	if (ret == 0)
	{
		ret = mp_set(base->z, 1);
	}
	if (ret == 0)
	{
		/* calculate public key */
		success = wolfssl_ecc_multiply(this->key.dp,
									   wc_ecc_key_get_priv(&this->key), base,
									   &this->key.pubkey);
	}

	wc_ecc_del_point(base);

	return success;
}

/**
 * Derive the shared secret
 */
static bool compute_shared_key(private_wolfssl_ec_diffie_hellman_t *this)
{
	word32 len;
#ifdef USE_RNG_FOR_TIMING_RESISTANCE
	WC_RNG rng;

	if (wc_InitRng(&rng) != 0)
	{
		return FALSE;
	}

	if (wc_ecc_set_rng(&this->key, &rng) != 0)
	{
		wc_FreeRng(&rng);
		return FALSE;
	}
#endif

	this->shared_secret = chunk_alloc(this->keysize);
	len = this->shared_secret.len;

	if (wc_ecc_shared_secret(&this->key, &this->pubkey, this->shared_secret.ptr,
							 &len) != 0)
	{
		DBG1(DBG_LIB, "ECDH shared secret computation failed");
		chunk_clear(&this->shared_secret);
#ifdef USE_RNG_FOR_TIMING_RESISTANCE
		wc_FreeRng(&rng);
#endif
		return FALSE;
	}
	this->shared_secret.len = len;
#ifdef USE_RNG_FOR_TIMING_RESISTANCE
	wc_FreeRng(&rng);
#endif
	return TRUE;
}

METHOD(key_exchange_t, get_shared_secret, bool,
	private_wolfssl_ec_diffie_hellman_t *this, chunk_t *secret)
{
	if (!this->shared_secret.len &&
		!compute_shared_key(this))
	{
		return FALSE;
	}
	*secret = chunk_clone(this->shared_secret);
	return TRUE;
}

METHOD(key_exchange_t, get_method, key_exchange_method_t,
	private_wolfssl_ec_diffie_hellman_t *this)
{
	return this->group;
}

METHOD(key_exchange_t, destroy, void,
	private_wolfssl_ec_diffie_hellman_t *this)
{
	wc_ecc_free(&this->key);
	wc_ecc_free(&this->pubkey);
	chunk_clear(&this->shared_secret);
	free(this);
}

/*
 * Described in header
 */
wolfssl_ec_diffie_hellman_t *wolfssl_ec_diffie_hellman_create(key_exchange_method_t group)
{
	private_wolfssl_ec_diffie_hellman_t *this;
	WC_RNG rng;

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

	if (wc_ecc_init(&this->key) != 0 || wc_ecc_init(&this->pubkey) != 0)
	{
		DBG1(DBG_LIB, "key init failed, ecdh create failed");
		destroy(this);
		return NULL;
	}

	switch (group)
	{
		case ECP_192_BIT:
			this->curve_id = ECC_SECP192R1;
			this->keysize = 192 / 8;
			break;
		case ECP_224_BIT:
			this->curve_id = ECC_SECP224R1;
			this->keysize = 224 / 8;
			break;
		case ECP_256_BIT:
			this->curve_id = ECC_SECP256R1;
			this->keysize = 256 / 8;
			break;
		case ECP_384_BIT:
			this->curve_id = ECC_SECP384R1;
			this->keysize = 384 / 8;
			break;
		case ECP_521_BIT:
			this->curve_id = ECC_SECP521R1;
			this->keysize = (521 + 7) / 8;
			break;
#ifdef HAVE_ECC_BRAINPOOL
		case ECP_224_BP:
			this->curve_id = ECC_BRAINPOOLP224R1;
			this->keysize = 224 / 8;
			break;
		case ECP_256_BP:
			this->curve_id = ECC_BRAINPOOLP256R1;
			this->keysize = 256 / 8;
			break;
		case ECP_384_BP:
			this->curve_id = ECC_BRAINPOOLP384R1;
			this->keysize = 384 / 8;
			break;
		case ECP_512_BP:
			this->curve_id = ECC_BRAINPOOLP512R1;
			this->keysize = 512 / 8;
			break;
#endif
		default:
			destroy(this);
			return NULL;
	}

	if (wc_InitRng(&rng) != 0)
	{
		DBG1(DBG_LIB, "RNG init failed, ecdh create failed");
		destroy(this);
		return NULL;
	}

	/* generate an EC private (public) key */
	if (wc_ecc_make_key_ex(&rng, this->keysize, &this->key,
						   this->curve_id) != 0)
	{
		DBG1(DBG_LIB, "make key failed, wolfssl ECDH create failed");
		destroy(this);
		return NULL;
	}
	wc_FreeRng(&rng);

	return &this->public;
}
#endif /* HAVE_ECC_DHE */
