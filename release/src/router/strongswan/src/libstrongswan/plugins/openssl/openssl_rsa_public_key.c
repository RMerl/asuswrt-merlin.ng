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

#include "openssl_rsa_public_key.h"
#include "openssl_hasher.h"
#include "openssl_util.h"

#include <utils/debug.h>
#include <credentials/keys/signature_params.h>

#include <openssl/bn.h>
#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/x509.h>

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
#include <openssl/param_build.h>
#include <openssl/core_names.h>
#endif

#if OPENSSL_VERSION_NUMBER < 0x10100000L
OPENSSL_KEY_FALLBACK(RSA, key, n, e, d)
#endif

typedef struct private_openssl_rsa_public_key_t private_openssl_rsa_public_key_t;

/**
 * Private data structure with signing context.
 */
struct private_openssl_rsa_public_key_t {
	/**
	 * Public interface for this signer.
	 */
	openssl_rsa_public_key_t public;

	/**
	 * RSA key object
	 */
	EVP_PKEY *key;

	/**
	 * reference counter
	 */
	refcount_t ref;
};

/**
 * Verify RSA signature
 */
static bool verify_signature(private_openssl_rsa_public_key_t *this,
							 const EVP_MD *md, rsa_pss_params_t *pss,
							 chunk_t data, chunk_t signature)
{
	EVP_PKEY_CTX *pctx = NULL;
	EVP_MD_CTX *mctx = NULL;
	int rsa_size = EVP_PKEY_size(this->key);
	bool valid = FALSE;

	/* OpenSSL expects a signature of exactly RSA size (no leading 0x00) */
	if (signature.len > rsa_size)
	{
		signature = chunk_skip(signature, signature.len - rsa_size);
	}

	mctx = EVP_MD_CTX_create();
	if (!mctx)
	{
		return FALSE;
	}
	if (EVP_DigestVerifyInit(mctx, &pctx, md, NULL, this->key) <= 0)
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
	if (EVP_DigestVerifyUpdate(mctx, data.ptr, data.len) <= 0)
	{
		goto error;
	}
	valid = (EVP_DigestVerifyFinal(mctx, signature.ptr, signature.len) == 1);

error:
	EVP_MD_CTX_destroy(mctx);
	return valid;
}

/**
 * Verification of a signature without hashing
 */
static bool verify_plain_signature(private_openssl_rsa_public_key_t *this,
								   chunk_t data, chunk_t signature)
{
	char *buf;
	size_t rsa_size = EVP_PKEY_size(this->key);
	bool valid = FALSE;

	/* OpenSSL expects a signature of exactly RSA size (no leading 0x00) */
	if (signature.len > rsa_size)
	{
		signature = chunk_skip(signature, signature.len - rsa_size);
	}
#if defined(OPENSSL_IS_BORINGSSL) && \
	(!defined(BORINGSSL_API_VERSION) || BORINGSSL_API_VERSION < 10)
	RSA *rsa = EVP_PKEY_get1_RSA(this->key);
	int len;

	buf = malloc(rsa_size);
	len = RSA_public_decrypt(signature.len, signature.ptr, buf, rsa,
							 RSA_PKCS1_PADDING);
	if (len != -1)
	{
		valid = chunk_equals_const(data, chunk_create(buf, len));
	}
	RSA_free(rsa);
#else
	EVP_PKEY_CTX *ctx;
	size_t len = rsa_size;

	ctx = EVP_PKEY_CTX_new(this->key, NULL);
	if (!ctx ||
		EVP_PKEY_verify_recover_init(ctx) <= 0 ||
		EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PADDING) <= 0)
	{
		EVP_PKEY_CTX_free(ctx);
		return FALSE;
	}
	buf = malloc(rsa_size);
	if (EVP_PKEY_verify_recover(ctx, buf, &len, signature.ptr, signature.len) > 0)
	{
		valid = chunk_equals_const(data, chunk_create(buf, len));
	}
	free(buf);
	EVP_PKEY_CTX_free(ctx);
#endif
	return valid;
}

/**
 * Verification of an EMSA PKCS1 signature described in PKCS#1
 */
static bool verify_emsa_pkcs1_signature(private_openssl_rsa_public_key_t *this,
										int type, chunk_t data, chunk_t signature)
{
	const EVP_MD *md;

	if (type == NID_undef)
	{
		return verify_plain_signature(this, data, signature);
	}
	md = EVP_get_digestbynid(type);
	return md && verify_signature(this, md, NULL, data, signature);
}

/**
 * Verification of an EMSA PSS signature described in PKCS#1
 */
static bool verify_emsa_pss_signature(private_openssl_rsa_public_key_t *this,
									  rsa_pss_params_t *params, chunk_t data,
									  chunk_t signature)
{
	const EVP_MD *md;

	if (!params)
	{
		return FALSE;
	}
	md = openssl_get_md(params->hash);
	return md && verify_signature(this, md, params, data, signature);
}

METHOD(public_key_t, get_type, key_type_t,
	private_openssl_rsa_public_key_t *this)
{
	return KEY_RSA;
}

METHOD(public_key_t, verify, bool,
	private_openssl_rsa_public_key_t *this, signature_scheme_t scheme,
	void *params, chunk_t data, chunk_t signature)
{
	switch (scheme)
	{
		case SIGN_RSA_EMSA_PKCS1_NULL:
			return verify_emsa_pkcs1_signature(this, NID_undef, data, signature);
		case SIGN_RSA_EMSA_PKCS1_SHA2_224:
			return verify_emsa_pkcs1_signature(this, NID_sha224, data, signature);
		case SIGN_RSA_EMSA_PKCS1_SHA2_256:
			return verify_emsa_pkcs1_signature(this, NID_sha256, data, signature);
		case SIGN_RSA_EMSA_PKCS1_SHA2_384:
			return verify_emsa_pkcs1_signature(this, NID_sha384, data, signature);
		case SIGN_RSA_EMSA_PKCS1_SHA2_512:
			return verify_emsa_pkcs1_signature(this, NID_sha512, data, signature);
#if OPENSSL_VERSION_NUMBER >= 0x1010100fL && !defined(OPENSSL_NO_SHA3)
		case SIGN_RSA_EMSA_PKCS1_SHA3_224:
			return verify_emsa_pkcs1_signature(this, NID_sha3_224, data, signature);
		case SIGN_RSA_EMSA_PKCS1_SHA3_256:
			return verify_emsa_pkcs1_signature(this, NID_sha3_256, data, signature);
		case SIGN_RSA_EMSA_PKCS1_SHA3_384:
			return verify_emsa_pkcs1_signature(this, NID_sha3_384, data, signature);
		case SIGN_RSA_EMSA_PKCS1_SHA3_512:
			return verify_emsa_pkcs1_signature(this, NID_sha3_512, data, signature);
#endif
		case SIGN_RSA_EMSA_PKCS1_SHA1:
			return verify_emsa_pkcs1_signature(this, NID_sha1, data, signature);
		case SIGN_RSA_EMSA_PKCS1_MD5:
			return verify_emsa_pkcs1_signature(this, NID_md5, data, signature);
		case SIGN_RSA_EMSA_PSS:
			return verify_emsa_pss_signature(this, params, data, signature);
		default:
			DBG1(DBG_LIB, "signature scheme %N not supported in RSA",
				 signature_scheme_names, scheme);
			return FALSE;
	}
}

METHOD(public_key_t, encrypt, bool,
	private_openssl_rsa_public_key_t *this, encryption_scheme_t scheme,
	void *params, chunk_t plain, chunk_t *crypto)
{
	EVP_PKEY_CTX *ctx = NULL;
	chunk_t label = chunk_empty;
	hash_algorithm_t hash_alg = HASH_UNKNOWN;
	size_t len;
	int padding;
	char *encrypted;
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

	if (EVP_PKEY_encrypt_init(ctx) <= 0)
	{
		DBG1(DBG_LIB, "could not initialize RSA encryption");
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

	/* determine maximum ciphertext size */
	len = EVP_PKEY_size(this->key);
	encrypted = malloc(len);

	/* decrypt data */
	if (EVP_PKEY_encrypt(ctx, encrypted, &len, plain.ptr, plain.len) <= 0)
	{
		DBG1(DBG_LIB, "RSA encryption failed");
		free(encrypted);
		goto error;
	}
	*crypto = chunk_create(encrypted, len);
	success = TRUE;

error:
	EVP_PKEY_CTX_free(ctx);
	return success;
}

METHOD(public_key_t, get_keysize, int,
	private_openssl_rsa_public_key_t *this)
{
	return EVP_PKEY_bits(this->key);
}

/**
 * Get n and e of the given RSA key (allocated).
 */
static bool get_n_and_e(EVP_PKEY *key, chunk_t *n, chunk_t *e)
{
	const BIGNUM *cbn_n, *cbn_e;
	BIGNUM *bn_n = NULL, *bn_e = NULL;
	bool success = FALSE;

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
	if (EVP_PKEY_get_bn_param(key, OSSL_PKEY_PARAM_RSA_N, &bn_n) <= 0 ||
		EVP_PKEY_get_bn_param(key, OSSL_PKEY_PARAM_RSA_E, &bn_e) <= 0)
	{
		goto error;
	}
	cbn_n = bn_n;
	cbn_e = bn_e;
#elif OPENSSL_VERSION_NUMBER >= 0x1010000fL
	RSA *rsa = EVP_PKEY_get0_RSA(key);
	RSA_get0_key(rsa, &cbn_n, &cbn_e, NULL);
#else
	RSA *rsa = EVP_PKEY_get1_RSA(key);
	RSA_get0_key(rsa, &cbn_n, &cbn_e, NULL);
	RSA_free(rsa);
#endif

	*n = *e = chunk_empty;
	if (!openssl_bn2chunk(cbn_n, n) ||
		!openssl_bn2chunk(cbn_e, e))
	{
		chunk_free(n);
		chunk_free(e);
		goto error;
	}
	success = TRUE;

error:
	BN_free(bn_n);
	BN_free(bn_e);
	return success;
}

/**
 * Calculate fingerprint from a RSA key, also used in rsa private key.
 */
bool openssl_rsa_fingerprint(EVP_PKEY *key, cred_encoding_type_t type, chunk_t *fp)
{
	if (!openssl_fingerprint(key, type, fp))
	{
		chunk_t n = chunk_empty, e = chunk_empty;
		bool success = FALSE;

		if (get_n_and_e(key, &n, &e))
		{
			success = lib->encoding->encode(lib->encoding, type, key, fp,
									CRED_PART_RSA_MODULUS, n,
									CRED_PART_RSA_PUB_EXP, e, CRED_PART_END);
		}
		chunk_free(&n);
		chunk_free(&e);
		return success;
	}
	return TRUE;
}

METHOD(public_key_t, get_fingerprint, bool,
	private_openssl_rsa_public_key_t *this, cred_encoding_type_t type,
	chunk_t *fingerprint)
{
	return openssl_rsa_fingerprint(this->key, type, fingerprint);
}

METHOD(public_key_t, get_encoding, bool,
	private_openssl_rsa_public_key_t *this, cred_encoding_type_t type,
	chunk_t *encoding)
{
	bool success = FALSE;

	switch (type)
	{
		case PUBKEY_SPKI_ASN1_DER:
		case PUBKEY_PEM:
		{
			*encoding = openssl_i2chunk(PUBKEY, this->key);
			success = TRUE;

			if (type == PUBKEY_PEM)
			{
				chunk_t asn1_encoding = *encoding;

				success = lib->encoding->encode(lib->encoding, PUBKEY_PEM,
								NULL, encoding, CRED_PART_RSA_PUB_ASN1_DER,
								asn1_encoding, CRED_PART_END);
				chunk_clear(&asn1_encoding);
			}
			return success;
		}
		case PUBKEY_ASN1_DER:
		{
			*encoding = openssl_i2chunk(PublicKey, this->key);
			return TRUE;
		}
		default:
		{
			chunk_t n = chunk_empty, e = chunk_empty;

			if (get_n_and_e(this->key, &n, &e))
			{
				success = lib->encoding->encode(lib->encoding, type, NULL,
									encoding, CRED_PART_RSA_MODULUS, n,
									CRED_PART_RSA_PUB_EXP, e, CRED_PART_END);
			}
			chunk_free(&n);
			chunk_free(&e);
			return success;
		}
	}
}

METHOD(public_key_t, get_ref, public_key_t*,
	private_openssl_rsa_public_key_t *this)
{
	ref_get(&this->ref);
	return &this->public.key;
}

METHOD(public_key_t, destroy, void,
	private_openssl_rsa_public_key_t *this)
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
 * Generic private constructor
 */
static private_openssl_rsa_public_key_t *create_internal(EVP_PKEY *key)
{
	private_openssl_rsa_public_key_t *this;

	INIT(this,
		.public = {
			.key = {
				.get_type = _get_type,
				.verify = _verify,
				.encrypt = _encrypt,
				.equals = public_key_equals,
				.get_keysize = _get_keysize,
				.get_fingerprint = _get_fingerprint,
				.has_fingerprint = public_key_has_fingerprint,
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

/**
 * See header.
 */
openssl_rsa_public_key_t *openssl_rsa_public_key_load(key_type_t type,
													  va_list args)
{
	private_openssl_rsa_public_key_t *this;
	EVP_PKEY *key = NULL;
	chunk_t blob, n, e;

	n = e = blob = chunk_empty;
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
			case BUILD_END:
				break;
			default:
				return NULL;
		}
		break;
	}

	if (blob.ptr)
	{
		switch (type)
		{
			case KEY_ANY:
				key = d2i_PUBKEY(NULL, (const u_char**)&blob.ptr, blob.len);
				if (key && EVP_PKEY_base_id(key) != EVP_PKEY_RSA)
				{
					EVP_PKEY_free(key);
					key = NULL;
				}
				break;
			case KEY_RSA:
#if defined(OPENSSL_IS_BORINGSSL) && \
	(!defined(BORINGSSL_API_VERSION) || BORINGSSL_API_VERSION < 10)
			{
				RSA *rsa = d2i_RSAPublicKey(NULL, (const u_char**)&blob.ptr,
											blob.len);
				key = EVP_PKEY_new();
				if (!key || !EVP_PKEY_assign_RSA(key, rsa))
				{
					RSA_free(rsa);
					EVP_PKEY_free(key);
					key = NULL;
				}
			}
#else
				key = d2i_PublicKey(EVP_PKEY_RSA, NULL, (const u_char**)&blob.ptr,
									blob.len);
#endif
				break;
			default:
				break;
		}
	}
	else if (n.ptr && e.ptr && type == KEY_RSA)
	{
		BIGNUM *bn_n, *bn_e;

		bn_n = BN_bin2bn((const u_char*)n.ptr, n.len, NULL);
		bn_e = BN_bin2bn((const u_char*)e.ptr, e.len, NULL);

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
		OSSL_PARAM_BLD *bld;
		OSSL_PARAM *params = NULL;
		EVP_PKEY_CTX *ctx;

		bld = OSSL_PARAM_BLD_new();
		if (bld &&
			OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_RSA_N, bn_n) &&
			OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_RSA_E, bn_e))
		{
			params = OSSL_PARAM_BLD_to_param(bld);
		}
		OSSL_PARAM_BLD_free(bld);
		BN_free(bn_n);
		BN_free(bn_e);

		ctx = EVP_PKEY_CTX_new_from_name(NULL, "RSA", NULL);
		if (!params || !ctx ||
			EVP_PKEY_fromdata_init(ctx) <= 0 ||
			EVP_PKEY_fromdata(ctx, &key, EVP_PKEY_PUBLIC_KEY, params) <= 0)
		{
			key = NULL;
		}
		EVP_PKEY_CTX_free(ctx);
		OSSL_PARAM_free(params);
#else /* OPENSSL_VERSION_NUMBER */
		RSA *rsa = RSA_new();

		if (RSA_set0_key(rsa, bn_n, bn_e, NULL))
		{
			key = EVP_PKEY_new();
			if (!key || !EVP_PKEY_assign_RSA(key, rsa))
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
	}
	if (!key)
	{
		return NULL;
	}
	this = create_internal(key);
	return &this->public;
}

#endif /* OPENSSL_NO_RSA */
