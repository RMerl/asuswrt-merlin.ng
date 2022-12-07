/*
 * Copyright (C) 2008-2017 Tobias Brunner
 * Copyright (C) 2009 Martin Willi
 *
 * Copyright (C) secunet Security Networks AG
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
#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
#include <openssl/param_build.h>
#include <openssl/core_names.h>
#endif

/**
 *  Public exponent to use for key generation.
 */
#define PUBLIC_EXPONENT 0x10001

#if OPENSSL_VERSION_NUMBER < 0x10100000L
OPENSSL_KEY_FALLBACK(RSA, key, n, e, d)
OPENSSL_KEY_FALLBACK(RSA, factors, p, q)
OPENSSL_KEY_FALLBACK(RSA, crt_params, dmp1, dmq1, iqmp)
#define BN_secure_new() BN_new()
#define BN_CTX_secure_new() BN_CTX_new()
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
	 * RSA key object
	 */
	EVP_PKEY *key;

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
bool openssl_rsa_fingerprint(EVP_PKEY *key, cred_encoding_type_t type, chunk_t *fp);

/**
 * Build RSA signature
 */
static bool build_signature(private_openssl_rsa_private_key_t *this,
							const EVP_MD *md, rsa_pss_params_t *pss,
							chunk_t data, chunk_t *sig)
{
	EVP_PKEY_CTX *pctx = NULL;
	EVP_MD_CTX *mctx = NULL;
	bool success = FALSE;

	mctx = EVP_MD_CTX_create();
	if (!mctx)
	{
		return FALSE;
	}
	if (EVP_DigestSignInit(mctx, &pctx, md, NULL, this->key) <= 0)
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
	EVP_MD_CTX_destroy(mctx);
	return success;
}

/**
 * Build an EMSA PKCS1 signature without hashing
 */
static bool build_plain_signature(private_openssl_rsa_private_key_t *this,
								  chunk_t data, chunk_t *sig)
{
	EVP_PKEY_CTX *ctx;

	ctx = EVP_PKEY_CTX_new(this->key, NULL);
	if (!ctx ||
		EVP_PKEY_sign_init(ctx) <= 0 ||
		EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PADDING) <= 0 ||
		EVP_PKEY_sign(ctx, sig->ptr, &sig->len, data.ptr, data.len) <= 0)
	{
		EVP_PKEY_CTX_free(ctx);
		return FALSE;
	}
	EVP_PKEY_CTX_free(ctx);
	return TRUE;
}

/**
 * Build an EMSA PKCS1 signature described in PKCS#1
 */
static bool build_emsa_pkcs1_signature(private_openssl_rsa_private_key_t *this,
									   int type, chunk_t data, chunk_t *sig)
{
	const EVP_MD *md;

	*sig = chunk_alloc(EVP_PKEY_size(this->key));

	if (type == NID_undef)
	{
		if (build_plain_signature(this, data, sig))
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

	*sig = chunk_alloc(EVP_PKEY_size(this->key));

	md = openssl_get_md(params->hash);
	if (md && build_signature(this, md, params, data, sig))
	{
		return TRUE;
	}
	chunk_free(sig);
	return FALSE;
}

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
#if OPENSSL_VERSION_NUMBER >= 0x1010100fL && !defined(OPENSSL_NO_SHA3)
		case SIGN_RSA_EMSA_PKCS1_SHA3_224:
			return build_emsa_pkcs1_signature(this, NID_sha3_224, data, signature);
		case SIGN_RSA_EMSA_PKCS1_SHA3_256:
			return build_emsa_pkcs1_signature(this, NID_sha3_256, data, signature);
		case SIGN_RSA_EMSA_PKCS1_SHA3_384:
			return build_emsa_pkcs1_signature(this, NID_sha3_384, data, signature);
		case SIGN_RSA_EMSA_PKCS1_SHA3_512:
			return build_emsa_pkcs1_signature(this, NID_sha3_512, data, signature);
#endif
		case SIGN_RSA_EMSA_PKCS1_SHA1:
			return build_emsa_pkcs1_signature(this, NID_sha1, data, signature);
		case SIGN_RSA_EMSA_PKCS1_MD5:
			return build_emsa_pkcs1_signature(this, NID_md5, data, signature);
		case SIGN_RSA_EMSA_PSS:
			return build_emsa_pss_signature(this, params, data, signature);
		default:
			DBG1(DBG_LIB, "signature scheme %N not supported in RSA",
				 signature_scheme_names, scheme);
			return FALSE;
	}
}

METHOD(private_key_t, decrypt, bool,
	private_openssl_rsa_private_key_t *this, encryption_scheme_t scheme,
	void *params, chunk_t crypto, chunk_t *plain)
{
	EVP_PKEY_CTX *ctx = NULL;
	chunk_t label = chunk_empty;
	hash_algorithm_t hash_alg = HASH_UNKNOWN;
	size_t len;
	int padding;
	char *decrypted;
	bool success = FALSE;

	switch (scheme)
	{
		case ENCRYPT_RSA_PKCS1:
			padding = RSA_PKCS1_PADDING;
			break;
		case ENCRYPT_RSA_OAEP_SHA1:
			hash_alg = HASH_SHA1;
			padding = RSA_PKCS1_OAEP_PADDING;
			break;
		case ENCRYPT_RSA_OAEP_SHA224:
			hash_alg = HASH_SHA224;
			padding = RSA_PKCS1_OAEP_PADDING;
			break;
		case ENCRYPT_RSA_OAEP_SHA256:
			hash_alg = HASH_SHA256;
			padding = RSA_PKCS1_OAEP_PADDING;
			break;
		case ENCRYPT_RSA_OAEP_SHA384:
			hash_alg = HASH_SHA384;
			padding = RSA_PKCS1_OAEP_PADDING;
			break;
		case ENCRYPT_RSA_OAEP_SHA512:
			hash_alg = HASH_SHA512;
			padding = RSA_PKCS1_OAEP_PADDING;
			break;
		default:
			DBG1(DBG_LIB, "encryption scheme %N not supported by openssl",
				 encryption_scheme_names, scheme);
			return FALSE;
	}

	ctx = EVP_PKEY_CTX_new(this->key, NULL);
	if (!ctx)
	{
		DBG1(DBG_LIB, "could not create EVP context");
		return FALSE;
	}

	if (EVP_PKEY_decrypt_init(ctx) <= 0)
	{
		DBG1(DBG_LIB, "could not initialize RSA decryption");
		goto error;
	}
	if (EVP_PKEY_CTX_set_rsa_padding(ctx, padding) <= 0)
	{
		DBG1(DBG_LIB, "could not set RSA padding");
		goto error;
	}
	if (padding == RSA_PKCS1_OAEP_PADDING)
	{
 		const EVP_MD *md = openssl_get_md(hash_alg);

		if (EVP_PKEY_CTX_set_rsa_oaep_md(ctx, md) <= 0)
		{
 			DBG1(DBG_LIB, "could not set RSA OAEP hash algorithm");
  			goto error;
		}

		if (params)
		{
			label = *(chunk_t *)params;
		}
		if (label.len > 0)
		{
			 uint8_t *label_cpy;

			 /* Openssl requires a copy of its own */
			 label_cpy = (uint8_t *)OPENSSL_malloc(label.len);
			 memcpy(label_cpy, label.ptr, label.len);

			if (EVP_PKEY_CTX_set0_rsa_oaep_label(ctx, label_cpy, label.len) <= 0)
			{
				OPENSSL_free(label_cpy);
				DBG1(DBG_LIB, "could not set RSA OAEP label");
				goto error;
			}
		}
	}

	/* determine maximum plaintext size */
	len = EVP_PKEY_size(this->key);
	decrypted = malloc(len);

	/* decrypt data */
	if (EVP_PKEY_decrypt(ctx, decrypted, &len, crypto.ptr, crypto.len) <= 0)
	{
		DBG1(DBG_LIB, "RSA decryption failed");
		free(decrypted);
		goto error;
	}
	*plain = chunk_create(decrypted, len);
	success = TRUE;

error:
	EVP_PKEY_CTX_free(ctx);
	return success;
}

METHOD(private_key_t, get_keysize, int,
	private_openssl_rsa_private_key_t *this)
{
	return EVP_PKEY_bits(this->key);
}

METHOD(private_key_t, get_public_key, public_key_t*,
	private_openssl_rsa_private_key_t *this)
{
	public_key_t *key;
	chunk_t enc;

	enc = openssl_i2chunk(PublicKey, this->key);
	key = lib->creds->create(lib->creds, CRED_PUBLIC_KEY, KEY_RSA,
							 BUILD_BLOB_ASN1_DER, enc, BUILD_END);
	free(enc.ptr);
	return key;
}

METHOD(private_key_t, get_fingerprint, bool,
	private_openssl_rsa_private_key_t *this, cred_encoding_type_t type,
	chunk_t *fingerprint)
{
	return openssl_rsa_fingerprint(this->key, type, fingerprint);
}

METHOD(private_key_t, get_encoding, bool,
	private_openssl_rsa_private_key_t *this, cred_encoding_type_t type,
	chunk_t *encoding)
{
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

			*encoding = openssl_i2chunk(PrivateKey, this->key);

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
		if (this->key)
		{
			lib->encoding->clear_cache(lib->encoding, this->key);
			EVP_PKEY_free(this->key);
		}
		free(this);
	}
}

/**
 * Internal generic constructor
 */
static private_openssl_rsa_private_key_t *create_internal(EVP_PKEY *key)
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
		.key = key,
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
	EVP_PKEY *key = NULL;
	u_int key_size = 0;
	BIGNUM *e;

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
		BN_free(e);
		return NULL;
	}

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
	/* EVP_RSA_gen() does not allow specifying the public exponent, the default
	 * value is the same, but let's still use this more flexible approach */
	EVP_PKEY_CTX *ctx;

	ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
	if (!ctx ||
		EVP_PKEY_keygen_init(ctx) <= 0 ||
		EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, key_size) <= 0 ||
		EVP_PKEY_CTX_set1_rsa_keygen_pubexp(ctx, e) <= 0 ||
		EVP_PKEY_keygen(ctx, &key) <= 0)
	{
		EVP_PKEY_CTX_free(ctx);
		return NULL;
	}
	EVP_PKEY_CTX_free(ctx);
#else  /* OPENSSL_VERSION_NUMBER */
	RSA *rsa = RSA_new();

	if (RSA_generate_key_ex(rsa, key_size, e, NULL))
	{
		key = EVP_PKEY_new();
		if (!EVP_PKEY_assign_RSA(key, rsa))
		{
			RSA_free(rsa);
			EVP_PKEY_free(key);
			key = NULL;
		}
	}
	else
	{
		RSA_free(rsa);
	}
#endif /* OPENSSL_VERSION_NUMBER */

	if (!key)
	{
		BN_free(e);
		return NULL;
	}
	this = create_internal(key);
	BN_free(e);
	return &this->public;
}

/*
 * See header
 */
private_key_t *openssl_rsa_private_key_create(EVP_PKEY *key, bool engine)
{
	private_openssl_rsa_private_key_t *this;

	if (EVP_PKEY_base_id(key) != EVP_PKEY_RSA)
	{
		EVP_PKEY_free(key);
		return NULL;
	}
	this = create_internal(key);
	this->engine = engine;
	return &this->public.key;
}

/**
 * Recover the primes from n, e and d using the algorithm described in
 * Appendix C of NIST SP 800-56B.
 */
static bool calculate_pq(BN_CTX *ctx, BIGNUM *n, BIGNUM *e, BIGNUM *d,
						 BIGNUM *p, BIGNUM *q)
{
	BIGNUM *k, *r, *g, *y, *n1, *x;
	int i, t, j;
	bool success = FALSE;

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
		if (!BN_rand_range(g, n))
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
	if (!BN_gcd(p, y, n, ctx))
	{
		goto error;
	}
	/* q = n/p */
	if (!BN_div(q, NULL, n, p, ctx))
	{
		goto error;
	}
	success = TRUE;

error:
	BN_CTX_end(ctx);
	return success;
}

/**
 * Calculates dp = d (mod p-1) or dq = d (mod q-1) for the Chinese remainder
 * algorithm.
 */
static bool dmodpq1(BN_CTX *ctx, BIGNUM *d, BIGNUM *pq, BIGNUM *res)
{
	BIGNUM *pq1;

	BN_CTX_start(ctx);
	pq1 = BN_CTX_get(ctx);
	/* p|q - 1
	 * d (mod p|q -1) */
	if (!BN_sub(pq1, pq, BN_value_one()) ||
		!BN_mod(res, d, pq1, ctx))
	{
		BN_CTX_end(ctx);
		return FALSE;
	}
	BN_CTX_end(ctx);
	return TRUE;
}

/**
 * Calculates qinv = q^-1 (mod p) for the Chinese remainder algorithm.
 */
static bool qinv(BN_CTX *ctx, BIGNUM *q, BIGNUM *p, BIGNUM *res)
{
	/* q^-1 (mod p) */
	return BN_mod_inverse(res, q, p, ctx);
}

/*
 * See header
 */
openssl_rsa_private_key_t *openssl_rsa_private_key_load(key_type_t type,
														va_list args)
{
	private_openssl_rsa_private_key_t *this;
	EVP_PKEY *key = NULL;
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

	if (blob.ptr)
	{
		key = d2i_PrivateKey(EVP_PKEY_RSA, NULL, (const u_char**)&blob.ptr,
							 blob.len);
	}
	else if (n.ptr && e.ptr && d.ptr)
	{
		BN_CTX *ctx;
		BIGNUM *bn_n, *bn_e, *bn_d, *bn_p, *bn_q, *dmp1, *dmq1, *iqmp;

		ctx = BN_CTX_secure_new();
		if (!ctx)
		{
			goto error;
		}
		BN_CTX_start(ctx);
		bn_n = BN_CTX_get(ctx);
		bn_e = BN_CTX_get(ctx);
		bn_d = BN_CTX_get(ctx);
		bn_p = BN_CTX_get(ctx);
		bn_q = BN_CTX_get(ctx);
		dmp1 = BN_CTX_get(ctx);
		dmq1 = BN_CTX_get(ctx);
		iqmp = BN_CTX_get(ctx);

		bn_n = BN_bin2bn((const u_char*)n.ptr, n.len, bn_n);
		bn_e = BN_bin2bn((const u_char*)e.ptr, e.len, bn_e);
		bn_d = BN_bin2bn((const u_char*)d.ptr, d.len, bn_d);
		if (p.ptr && q.ptr)
		{
			bn_p = BN_bin2bn((const u_char*)p.ptr, p.len, bn_p);
			bn_q = BN_bin2bn((const u_char*)q.ptr, q.len, bn_q);
		}
		else if (!calculate_pq(ctx, bn_n, bn_e, bn_d, bn_p, bn_q))
		{
			goto error;
		}
		if (exp1.ptr)
		{
			dmp1 = BN_bin2bn((const u_char*)exp1.ptr, exp1.len, dmp1);
		}
		else if (!dmodpq1(ctx, bn_d, bn_p, dmp1))
		{
			goto error;
		}
		if (exp2.ptr)
		{
			dmq1 = BN_bin2bn((const u_char*)exp2.ptr, exp2.len, dmq1);
		}
		else if (!dmodpq1(ctx, bn_d, bn_q, dmq1))
		{
			goto error;
		}
		if (coeff.ptr)
		{
			iqmp = BN_bin2bn((const u_char*)coeff.ptr, coeff.len, iqmp);
		}
		else if (!qinv(ctx, bn_q, bn_p, iqmp))
		{
			goto error;
		}

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
		OSSL_PARAM_BLD *bld;
		OSSL_PARAM *params = NULL;
		EVP_PKEY_CTX *pctx;

		bld = OSSL_PARAM_BLD_new();
		if (bld &&
			OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_RSA_N, bn_n) &&
			OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_RSA_E, bn_e) &&
			OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_RSA_D, bn_d) &&
			OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_RSA_FACTOR1, bn_p) &&
			OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_RSA_FACTOR2, bn_q) &&
			OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_RSA_EXPONENT1, dmp1) &&
			OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_RSA_EXPONENT2, dmq1) &&
			OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_RSA_COEFFICIENT1, iqmp))
		{
			params = OSSL_PARAM_BLD_to_param(bld);
		}
		OSSL_PARAM_BLD_free(bld);

		pctx = EVP_PKEY_CTX_new_from_name(NULL, "RSA", NULL);
		if (!params || !pctx ||
			EVP_PKEY_fromdata_init(pctx) <= 0 ||
			EVP_PKEY_fromdata(pctx, &key, EVP_PKEY_KEYPAIR, params) <= 0)
		{
			key = NULL;
		}
		EVP_PKEY_CTX_free(pctx);
		OSSL_PARAM_free(params);
#else /* OPENSSL_VERSION_NUMBER */
		RSA *rsa = RSA_new();
		if (!RSA_set0_key(rsa, BN_dup(bn_n), BN_dup(bn_e), BN_dup(bn_d)) ||
			!RSA_set0_factors(rsa, BN_dup(bn_p), BN_dup(bn_q)) ||
			!RSA_set0_crt_params(rsa, BN_dup(dmp1), BN_dup(dmq1), BN_dup(iqmp)) ||
			RSA_check_key(rsa) <= 0)
		{
			RSA_free(rsa);
			goto error;
		}
		key = EVP_PKEY_new();
		if (!EVP_PKEY_assign_RSA(key, rsa))
		{
			RSA_free(rsa);
			EVP_PKEY_free(key);
			key = NULL;
		}
#endif /* OPENSSL_VERSION_NUMBER */

error:
		BN_CTX_end(ctx);
		BN_CTX_free(ctx);
	}
	if (!key)
	{
		return NULL;
	}
	this = create_internal(key);
	return &this->public;
}

#endif /* OPENSSL_NO_RSA */
