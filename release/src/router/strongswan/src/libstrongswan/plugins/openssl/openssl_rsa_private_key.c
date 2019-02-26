/*
 * Copyright (C) 2008-2017 Tobias Brunner
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

#include <openssl/opensslconf.h>

#ifndef OPENSSL_NO_RSA

#include "openssl_rsa_private_key.h"
#include "openssl_rsa_public_key.h"
#include "openssl_hasher.h"
#include "openssl_util.h"

#include <utils/debug.h>
#include <credentials/keys/signature_params.h>

#include <openssl/bn.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>

/**
 *  Public exponent to use for key generation.
 */
#define PUBLIC_EXPONENT 0x10001

#if OPENSSL_VERSION_NUMBER < 0x10100000L
OPENSSL_KEY_FALLBACK(RSA, key, n, e, d)
OPENSSL_KEY_FALLBACK(RSA, factors, p, q)
OPENSSL_KEY_FALLBACK(RSA, crt_params, dmp1, dmq1, iqmp)
#define BN_secure_new() BN_new()
#endif

typedef struct private_openssl_rsa_private_key_t private_openssl_rsa_private_key_t;

/**
 * Private data of a openssl_rsa_private_key_t object.
 */
struct private_openssl_rsa_private_key_t {
	/**
	 * Public interface for this signer.
	 */
	openssl_rsa_private_key_t public;

	/**
	 * RSA object from OpenSSL
	 */
	RSA *rsa;

	/**
	 * TRUE if the key is from an OpenSSL ENGINE and might not be readable
	 */
	bool engine;

	/**
	 * reference count
	 */
	refcount_t ref;
};

/* implemented in rsa public key */
bool openssl_rsa_fingerprint(RSA *rsa, cred_encoding_type_t type, chunk_t *fp);

#if OPENSSL_VERSION_NUMBER >= 0x10000000L

/**
 * Build RSA signature
 */
static bool build_signature(private_openssl_rsa_private_key_t *this,
							const EVP_MD *md, rsa_pss_params_t *pss,
							chunk_t data, chunk_t *sig)
{
	EVP_PKEY_CTX *pctx = NULL;
	EVP_MD_CTX *mctx = NULL;
	EVP_PKEY *key;
	bool success = FALSE;

	mctx = EVP_MD_CTX_create();
	key = EVP_PKEY_new();
	if (!mctx || !key)
	{
		goto error;
	}
	if (!EVP_PKEY_set1_RSA(key, this->rsa))
	{
		goto error;
	}
	if (EVP_DigestSignInit(mctx, &pctx, md, NULL, key) <= 0)
	{
		goto error;
	}
	if (pss)
	{
		const EVP_MD *mgf1md = openssl_get_md(pss->mgf1_hash);
		if (EVP_PKEY_CTX_set_rsa_padding(pctx, RSA_PKCS1_PSS_PADDING) <= 0 ||
			EVP_PKEY_CTX_set_rsa_pss_saltlen(pctx, pss->salt_len) <= 0 ||
			EVP_PKEY_CTX_set_rsa_mgf1_md(pctx, mgf1md) <= 0)
		{
			goto error;
		}
	}
	if (EVP_DigestSignUpdate(mctx, data.ptr, data.len) <= 0)
	{
		goto error;
	}
	success = (EVP_DigestSignFinal(mctx, sig->ptr, &sig->len) == 1);

error:
	if (key)
	{
		EVP_PKEY_free(key);
	}
	if (mctx)
	{
		EVP_MD_CTX_destroy(mctx);
	}
	return success;
}

/**
 * Build an EMSA PKCS1 signature described in PKCS#1
 */
static bool build_emsa_pkcs1_signature(private_openssl_rsa_private_key_t *this,
									   int type, chunk_t data, chunk_t *sig)
{
	const EVP_MD *md;

	*sig = chunk_alloc(RSA_size(this->rsa));

	if (type == NID_undef)
	{
		if (RSA_private_encrypt(data.len, data.ptr, sig->ptr, this->rsa,
								RSA_PKCS1_PADDING) == sig->len)
		{
			return TRUE;
		}
	}
	else
	{
		md = EVP_get_digestbynid(type);
		if (md && build_signature(this, md, NULL, data, sig))
		{
			return TRUE;
		}
	}
	chunk_free(sig);
	return FALSE;
}

/**
 * Build an EMSA PSS signature described in PKCS#1
 */
static bool build_emsa_pss_signature(private_openssl_rsa_private_key_t *this,
									 rsa_pss_params_t *params, chunk_t data,
									 chunk_t *sig)
{
	const EVP_MD *md;

	if (!params)
	{
		return FALSE;
	}

	*sig = chunk_alloc(RSA_size(this->rsa));

	md = openssl_get_md(params->hash);
	if (md && build_signature(this, md, params, data, sig))
	{
		return TRUE;
	}
	chunk_free(sig);
	return FALSE;
}

#else /* OPENSSL_VERSION_NUMBER < 1.0 */

/**
 * Build an EMSA PKCS1 signature described in PKCS#1
 */
static bool build_emsa_pkcs1_signature(private_openssl_rsa_private_key_t *this,
									   int type, chunk_t data, chunk_t *sig)
{
	bool success = FALSE;

	*sig = chunk_alloc(RSA_size(this->rsa));

	if (type == NID_undef)
	{
		if (RSA_private_encrypt(data.len, data.ptr, sig->ptr, this->rsa,
								RSA_PKCS1_PADDING) == sig->len)
		{
			success = TRUE;
		}
	}
	else
	{
		EVP_MD_CTX *ctx = NULL;
		EVP_PKEY *key = NULL;
		const EVP_MD *hasher;
		u_int len;

		hasher = EVP_get_digestbynid(type);
		if (!hasher)
		{
			goto error;
		}

		ctx = EVP_MD_CTX_create();
		key = EVP_PKEY_new();
		if (!ctx || !key)
		{
			goto error;
		}
		if (!EVP_PKEY_set1_RSA(key, this->rsa))
		{
			goto error;
		}
		if (!EVP_SignInit_ex(ctx, hasher, NULL))
		{
			goto error;
		}
		if (!EVP_SignUpdate(ctx, data.ptr, data.len))
		{
			goto error;
		}
		if (EVP_SignFinal(ctx, sig->ptr, &len, key))
		{
			success = TRUE;
		}

error:
		if (key)
		{
			EVP_PKEY_free(key);
		}
		if (ctx)
		{
			EVP_MD_CTX_destroy(ctx);
		}
	}
	if (!success)
	{
		free(sig->ptr);
	}
	return success;
}
#endif /* OPENSSL_VERSION_NUMBER < 1.0 */

METHOD(private_key_t, get_type, key_type_t,
	private_openssl_rsa_private_key_t *this)
{
	return KEY_RSA;
}

METHOD(private_key_t, sign, bool,
	private_openssl_rsa_private_key_t *this, signature_scheme_t scheme,
	void *params, chunk_t data, chunk_t *signature)
{
	switch (scheme)
	{
		case SIGN_RSA_EMSA_PKCS1_NULL:
			return build_emsa_pkcs1_signature(this, NID_undef, data, signature);
		case SIGN_RSA_EMSA_PKCS1_SHA2_224:
			return build_emsa_pkcs1_signature(this, NID_sha224, data, signature);
		case SIGN_RSA_EMSA_PKCS1_SHA2_256:
			return build_emsa_pkcs1_signature(this, NID_sha256, data, signature);
		case SIGN_RSA_EMSA_PKCS1_SHA2_384:
			return build_emsa_pkcs1_signature(this, NID_sha384, data, signature);
		case SIGN_RSA_EMSA_PKCS1_SHA2_512:
			return build_emsa_pkcs1_signature(this, NID_sha512, data, signature);
		case SIGN_RSA_EMSA_PKCS1_SHA1:
			return build_emsa_pkcs1_signature(this, NID_sha1, data, signature);
		case SIGN_RSA_EMSA_PKCS1_MD5:
			return build_emsa_pkcs1_signature(this, NID_md5, data, signature);
#if OPENSSL_VERSION_NUMBER >= 0x10000000L
		case SIGN_RSA_EMSA_PSS:
			return build_emsa_pss_signature(this, params, data, signature);
#endif
		default:
			DBG1(DBG_LIB, "signature scheme %N not supported in RSA",
				 signature_scheme_names, scheme);
			return FALSE;
	}
}

METHOD(private_key_t, decrypt, bool,
	private_openssl_rsa_private_key_t *this, encryption_scheme_t scheme,
	chunk_t crypto, chunk_t *plain)
{
	int padding, len;
	char *decrypted;

	switch (scheme)
	{
		case ENCRYPT_RSA_PKCS1:
			padding = RSA_PKCS1_PADDING;
			break;
		case ENCRYPT_RSA_OAEP_SHA1:
			padding = RSA_PKCS1_OAEP_PADDING;
			break;
		default:
			DBG1(DBG_LIB, "encryption scheme %N not supported via openssl",
				 encryption_scheme_names, scheme);
			return FALSE;
	}
	decrypted = malloc(RSA_size(this->rsa));
	len = RSA_private_decrypt(crypto.len, crypto.ptr, decrypted,
							  this->rsa, padding);
	if (len < 0)
	{
		DBG1(DBG_LIB, "RSA decryption failed");
		free(decrypted);
		return FALSE;
	}
	*plain = chunk_create(decrypted, len);
	return TRUE;
}

METHOD(private_key_t, get_keysize, int,
	private_openssl_rsa_private_key_t *this)
{
	return RSA_size(this->rsa) * 8;
}

METHOD(private_key_t, get_public_key, public_key_t*,
	private_openssl_rsa_private_key_t *this)
{
	chunk_t enc;
	public_key_t *key;
	u_char *p;

	enc = chunk_alloc(i2d_RSAPublicKey(this->rsa, NULL));
	p = enc.ptr;
	i2d_RSAPublicKey(this->rsa, &p);
	key = lib->creds->create(lib->creds, CRED_PUBLIC_KEY, KEY_RSA,
							 BUILD_BLOB_ASN1_DER, enc, BUILD_END);
	free(enc.ptr);
	return key;
}

METHOD(private_key_t, get_fingerprint, bool,
	private_openssl_rsa_private_key_t *this, cred_encoding_type_t type,
	chunk_t *fingerprint)
{
	return openssl_rsa_fingerprint(this->rsa, type, fingerprint);
}

METHOD(private_key_t, get_encoding, bool,
	private_openssl_rsa_private_key_t *this, cred_encoding_type_t type,
	chunk_t *encoding)
{
	u_char *p;

	if (this->engine)
	{
		return FALSE;
	}
	switch (type)
	{
		case PRIVKEY_ASN1_DER:
		case PRIVKEY_PEM:
		{
			bool success = TRUE;

			*encoding = chunk_alloc(i2d_RSAPrivateKey(this->rsa, NULL));
			p = encoding->ptr;
			i2d_RSAPrivateKey(this->rsa, &p);

			if (type == PRIVKEY_PEM)
			{
				chunk_t asn1_encoding = *encoding;

				success = lib->encoding->encode(lib->encoding, PRIVKEY_PEM,
								NULL, encoding, CRED_PART_RSA_PRIV_ASN1_DER,
								asn1_encoding, CRED_PART_END);
				chunk_clear(&asn1_encoding);
			}
			return success;
		}
		default:
			return FALSE;
	}
}

METHOD(private_key_t, get_ref, private_key_t*,
	private_openssl_rsa_private_key_t *this)
{
	ref_get(&this->ref);
	return &this->public.key;
}

METHOD(private_key_t, destroy, void,
	private_openssl_rsa_private_key_t *this)
{
	if (ref_put(&this->ref))
	{
		if (this->rsa)
		{
			lib->encoding->clear_cache(lib->encoding, this->rsa);
			RSA_free(this->rsa);
		}
		free(this);
	}
}

/**
 * Internal generic constructor
 */
static private_openssl_rsa_private_key_t *create_empty()
{
	private_openssl_rsa_private_key_t *this;

	INIT(this,
		.public = {
			.key = {
				.get_type = _get_type,
				.sign = _sign,
				.decrypt = _decrypt,
				.get_keysize = _get_keysize,
				.get_public_key = _get_public_key,
				.equals = private_key_equals,
				.belongs_to = private_key_belongs_to,
				.get_fingerprint = _get_fingerprint,
				.has_fingerprint = private_key_has_fingerprint,
				.get_encoding = _get_encoding,
				.get_ref = _get_ref,
				.destroy = _destroy,
			},
		},
		.ref = 1,
	);

	return this;
}

/*
 * See header.
 */
openssl_rsa_private_key_t *openssl_rsa_private_key_gen(key_type_t type,
													   va_list args)
{
	private_openssl_rsa_private_key_t *this;
	u_int key_size = 0;
	RSA *rsa = NULL;
	BIGNUM *e = NULL;

	while (TRUE)
	{
		switch (va_arg(args, builder_part_t))
		{
			case BUILD_KEY_SIZE:
				key_size = va_arg(args, u_int);
				continue;
			case BUILD_END:
				break;
			default:
				return NULL;
		}
		break;
	}
	if (!key_size)
	{
		return NULL;
	}
	e = BN_new();
	if (!e || !BN_set_word(e, PUBLIC_EXPONENT))
	{
		goto error;
	}
	rsa = RSA_new();
	if (!rsa || !RSA_generate_key_ex(rsa, key_size, e, NULL))
	{
		goto error;
	}
	this = create_empty();
	this->rsa = rsa;
	BN_free(e);
	return &this->public;

error:
	if (e)
	{
		BN_free(e);
	}
	if (rsa)
	{
		RSA_free(rsa);
	}
	return NULL;
}

/*
 * See header
 */
private_key_t *openssl_rsa_private_key_create(EVP_PKEY *key, bool engine)
{
	private_openssl_rsa_private_key_t *this;
	RSA *rsa;

	rsa = EVP_PKEY_get1_RSA(key);
	EVP_PKEY_free(key);
	if (!rsa)
	{
		return NULL;
	}
	this = create_empty();
	this->rsa = rsa;
	this->engine = engine;
	return &this->public.key;
}

/**
 * Recover the primes from n, e and d using the algorithm described in
 * Appendix C of NIST SP 800-56B.
 */
static bool calculate_pq(BIGNUM *n, BIGNUM *e, BIGNUM *d,
						 BIGNUM **p, BIGNUM **q)
{
	BN_CTX *ctx;
	BIGNUM *k, *r, *g, *y, *n1, *x;
	int i, t, j;
	bool success = FALSE;

	ctx = BN_CTX_new();
	if (!ctx)
	{
		return FALSE;
	}
	BN_CTX_start(ctx);
	k = BN_CTX_get(ctx);
	r = BN_CTX_get(ctx);
	g = BN_CTX_get(ctx);
	y = BN_CTX_get(ctx);
	n1 = BN_CTX_get(ctx);
	x = BN_CTX_get(ctx);
	if (!x)
	{
		goto error;
	}
	/* k = (d * e) - 1 */
	if (!BN_mul(k, d, e, ctx) || !BN_sub(k, k, BN_value_one()))
	{
		goto error;
	}
	/* k must be even */
	if (BN_is_odd(k))
	{
		goto error;
	}
	/* k = 2^t * r, where r is the largest odd integer dividing k, and t >= 1 */
	if (!BN_copy(r, k))
	{
		goto error;
	}
	for (t = 0; !BN_is_odd(r); t++)
	{	/* r = r/2 */
		if (!BN_rshift(r, r, 1))
		{
			goto error;
		}
	}
	/* we need n-1 below */
	if (!BN_sub(n1, n, BN_value_one()))
	{
		goto error;
	}
	for (i = 0; i < 100; i++)
	{	/* generate random integer g in [0, n-1] */
		if (!BN_pseudo_rand_range(g, n))
		{
			goto error;
		}
		/* y = g^r mod n */
		if (!BN_mod_exp(y, g, r, n, ctx))
		{
			goto error;
		}
		/* try again if y == 1 or y == n-1 */
		if (BN_is_one(y) || BN_cmp(y, n1) == 0)
		{
			continue;
		}
		for (j = 0; j < t; j++)
		{	/* x = y^2 mod n */
			if (!BN_mod_sqr(x, y, n, ctx))
			{
				goto error;
			}
			/* stop if x == 1 */
			if (BN_is_one(x))
			{
				goto done;
			}
			/* retry with new g if x = n-1 */
			if (BN_cmp(x, n1) == 0)
			{
				break;
			}
			/* y = x */
			if (!BN_copy(y, x))
			{
				goto error;
			}
		}
	}
	goto error;

done:
	/* p = gcd(y-1, n) */
	if (!BN_sub(y, y, BN_value_one()))
	{
		goto error;
	}
	*p = BN_secure_new();
	if (!BN_gcd(*p, y, n, ctx))
	{
		BN_clear_free(*p);
		goto error;
	}
	/* q = n/p */
	*q = BN_secure_new();
	if (!BN_div(*q, NULL, n, *p, ctx))
	{
		BN_clear_free(*p);
		BN_clear_free(*q);
		goto error;
	}
	success = TRUE;

error:
	BN_CTX_end(ctx);
	BN_CTX_free(ctx);
	return success;
}

/**
 * Calculates dp = d (mod p-1) or dq = d (mod q-1) for the Chinese remainder
 * algorithm.
 */
static BIGNUM *dmodpq1(BIGNUM *d, BIGNUM *pq)
{
	BN_CTX *ctx;
	BIGNUM *res = NULL, *pq1;

	ctx = BN_CTX_new();
	if (!ctx)
	{
		return NULL;
	}
	BN_CTX_start(ctx);
	pq1 = BN_CTX_get(ctx);
	/* p|q - 1 */
	if (!BN_sub(pq1, pq, BN_value_one()))
	{
		goto error;
	}
	/* d (mod p|q -1) */
	res = BN_secure_new();
	if (!BN_mod(res, d, pq1, ctx))
	{
		BN_clear_free(res);
		res = NULL;
		goto error;
	}

error:
	BN_CTX_end(ctx);
	BN_CTX_free(ctx);
	return res;
}

/**
 * Calculates qinv = q^-1 (mod p) for the Chinese remainder algorithm.
 */
static BIGNUM *qinv(BIGNUM *q, BIGNUM *p)
{
	BN_CTX *ctx;
	BIGNUM *res = NULL;

	ctx = BN_CTX_new();
	if (!ctx)
	{
		return NULL;
	}
	BN_CTX_start(ctx);
	/* q^-1 (mod p) */
	res = BN_secure_new();
	if (!BN_mod_inverse(res, q, p, ctx))
	{
		BN_clear_free(res);
		res = NULL;
		goto error;
	}

error:
	BN_CTX_end(ctx);
	BN_CTX_free(ctx);
	return res;
}

/*
 * See header
 */
openssl_rsa_private_key_t *openssl_rsa_private_key_load(key_type_t type,
														va_list args)
{
	private_openssl_rsa_private_key_t *this;
	chunk_t blob, n, e, d, p, q, exp1, exp2, coeff;

	blob = n = e = d = p = q = exp1 = exp2 = coeff = chunk_empty;
	while (TRUE)
	{
		switch (va_arg(args, builder_part_t))
		{
			case BUILD_BLOB_ASN1_DER:
				blob = va_arg(args, chunk_t);
				continue;
			case BUILD_RSA_MODULUS:
				n = va_arg(args, chunk_t);
				continue;
			case BUILD_RSA_PUB_EXP:
				e = va_arg(args, chunk_t);
				continue;
			case BUILD_RSA_PRIV_EXP:
				d = va_arg(args, chunk_t);
				continue;
			case BUILD_RSA_PRIME1:
				p = va_arg(args, chunk_t);
				continue;
			case BUILD_RSA_PRIME2:
				q = va_arg(args, chunk_t);
				continue;
			case BUILD_RSA_EXP1:
				exp1 = va_arg(args, chunk_t);
				continue;
			case BUILD_RSA_EXP2:
				exp2 = va_arg(args, chunk_t);
				continue;
			case BUILD_RSA_COEFF:
				coeff = va_arg(args, chunk_t);
				continue;
			case BUILD_END:
				break;
			default:
				return NULL;
		}
		break;
	}

	this = create_empty();
	if (blob.ptr)
	{
		this->rsa = d2i_RSAPrivateKey(NULL, (const u_char**)&blob.ptr, blob.len);
		if (this->rsa && RSA_check_key(this->rsa) == 1)
		{
			return &this->public;
		}
	}
	else if (n.ptr && e.ptr && d.ptr)
	{
		BIGNUM *bn_n, *bn_e, *bn_d, *bn_p, *bn_q;
		BIGNUM *dmp1 = NULL, *dmq1 = NULL, *iqmp = NULL;

		this->rsa = RSA_new();

		bn_n = BN_bin2bn((const u_char*)n.ptr, n.len, NULL);
		bn_e = BN_bin2bn((const u_char*)e.ptr, e.len, NULL);
		bn_d = BN_bin2bn((const u_char*)d.ptr, d.len, NULL);
		if (!RSA_set0_key(this->rsa, bn_n, bn_e, bn_d))
		{
			goto error;

		}
		if (p.ptr && q.ptr)
		{
			bn_p = BN_bin2bn((const u_char*)p.ptr, p.len, NULL);
			bn_q = BN_bin2bn((const u_char*)q.ptr, q.len, NULL);
		}
		else
		{
			if (!calculate_pq(bn_n, bn_e, bn_d, &bn_p, &bn_q))
			{
				goto error;
			}
		}
		if (!RSA_set0_factors(this->rsa, bn_p, bn_q))
		{
			goto error;
		}
		if (exp1.ptr)
		{
			dmp1 = BN_bin2bn((const u_char*)exp1.ptr, exp1.len, NULL);
		}
		else
		{
			dmp1 = dmodpq1(bn_d, bn_p);
		}
		if (exp2.ptr)
		{
			dmq1 = BN_bin2bn((const u_char*)exp2.ptr, exp2.len, NULL);
		}
		else
		{
			dmq1 = dmodpq1(bn_d, bn_q);
		}
		if (coeff.ptr)
		{
			iqmp = BN_bin2bn((const u_char*)coeff.ptr, coeff.len, NULL);
		}
		else
		{
			iqmp = qinv(bn_q, bn_p);
		}
		if (RSA_set0_crt_params(this->rsa, dmp1, dmq1, iqmp) &&
			RSA_check_key(this->rsa) == 1)
		{
			return &this->public;
		}
	}
error:
	destroy(this);
	return NULL;
}

#endif /* OPENSSL_NO_RSA */
