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

#include "openssl_rsa_public_key.h"
#include "openssl_hasher.h"
#include "openssl_util.h"

#include <utils/debug.h>
#include <credentials/keys/signature_params.h>

#include <openssl/bn.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/x509.h>

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
	 * RSA object from OpenSSL
	 */
	RSA *rsa;

	/**
	 * reference counter
	 */
	refcount_t ref;
};


#if OPENSSL_VERSION_NUMBER >= 0x10000000L

/**
 * Verify RSA signature
 */
static bool verify_signature(private_openssl_rsa_public_key_t *this,
							 const EVP_MD *md, rsa_pss_params_t *pss,
							 chunk_t data, chunk_t signature)
{
	EVP_PKEY_CTX *pctx = NULL;
	EVP_MD_CTX *mctx = NULL;
	EVP_PKEY *key;
	int rsa_size = RSA_size(this->rsa);
	bool valid = FALSE;

	/* OpenSSL expects a signature of exactly RSA size (no leading 0x00) */
	if (signature.len > rsa_size)
	{
		signature = chunk_skip(signature, signature.len - rsa_size);
	}

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
	if (EVP_DigestVerifyInit(mctx, &pctx, md, NULL, key) <= 0)
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
	if (key)
	{
		EVP_PKEY_free(key);
	}
	if (mctx)
	{
		EVP_MD_CTX_destroy(mctx);
	}
	return valid;
}

/**
 * Verification of a signature without hashing
 */
static bool verify_plain_signature(private_openssl_rsa_public_key_t *this,
								   chunk_t data, chunk_t signature)
{
	char *buf;
	int len, rsa_size = RSA_size(this->rsa);
	bool valid = FALSE;

	/* OpenSSL expects a signature of exactly RSA size (no leading 0x00) */
	if (signature.len > rsa_size)
	{
		signature = chunk_skip(signature, signature.len - rsa_size);
	}
	buf = malloc(rsa_size);
	len = RSA_public_decrypt(signature.len, signature.ptr, buf, this->rsa,
							 RSA_PKCS1_PADDING);
	if (len != -1)
	{
		valid = chunk_equals_const(data, chunk_create(buf, len));
	}
	free(buf);
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

#else /* OPENSSL_VERSION_NUMBER < 1.0 */

/**
 * Verification of an EMSA PKCS1 signature described in PKCS#1
 */
static bool verify_emsa_pkcs1_signature(private_openssl_rsa_public_key_t *this,
										int type, chunk_t data, chunk_t signature)
{
	bool valid = FALSE;
	int rsa_size = RSA_size(this->rsa);

	/* OpenSSL expects a signature of exactly RSA size (no leading 0x00) */
	if (signature.len > rsa_size)
	{
		signature = chunk_skip(signature, signature.len - rsa_size);
	}

	if (type == NID_undef)
	{
		char *buf;
		int len;

		buf = malloc(rsa_size);
		len = RSA_public_decrypt(signature.len, signature.ptr, buf, this->rsa,
								 RSA_PKCS1_PADDING);
		if (len != -1)
		{
			valid = chunk_equals_const(data, chunk_create(buf, len));
		}
		free(buf);
	}
	else
	{
		EVP_MD_CTX *ctx;
		EVP_PKEY *key;
		const EVP_MD *hasher;

		hasher = EVP_get_digestbynid(type);
		if (!hasher)
		{
			return FALSE;
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
		if (!EVP_VerifyInit_ex(ctx, hasher, NULL))
		{
			goto error;
		}
		if (!EVP_VerifyUpdate(ctx, data.ptr, data.len))
		{
			goto error;
		}
		valid = (EVP_VerifyFinal(ctx, signature.ptr, signature.len, key) == 1);

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
	return valid;
}

#endif /* OPENSSL_VERSION_NUMBER < 1.0 */

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
		case SIGN_RSA_EMSA_PKCS1_SHA1:
			return verify_emsa_pkcs1_signature(this, NID_sha1, data, signature);
		case SIGN_RSA_EMSA_PKCS1_MD5:
			return verify_emsa_pkcs1_signature(this, NID_md5, data, signature);
#if OPENSSL_VERSION_NUMBER >= 0x10000000L
		case SIGN_RSA_EMSA_PSS:
			return verify_emsa_pss_signature(this, params, data, signature);
#endif
		default:
			DBG1(DBG_LIB, "signature scheme %N not supported in RSA",
				 signature_scheme_names, scheme);
			return FALSE;
	}
}

METHOD(public_key_t, encrypt, bool,
	private_openssl_rsa_public_key_t *this, encryption_scheme_t scheme,
	chunk_t plain, chunk_t *crypto)
{
	int padding, len;
	char *encrypted;

	switch (scheme)
	{
		case ENCRYPT_RSA_PKCS1:
			padding = RSA_PKCS1_PADDING;
			break;
		case ENCRYPT_RSA_OAEP_SHA1:
			padding = RSA_PKCS1_OAEP_PADDING;
			break;
		default:
			DBG1(DBG_LIB, "decryption scheme %N not supported via openssl",
				 encryption_scheme_names, scheme);
			return FALSE;
	}
	encrypted = malloc(RSA_size(this->rsa));
	len = RSA_public_encrypt(plain.len, plain.ptr, encrypted,
							 this->rsa, padding);
	if (len < 0)
	{
		DBG1(DBG_LIB, "RSA decryption failed");
		free(encrypted);
		return FALSE;
	}
	*crypto = chunk_create(encrypted, len);
	return TRUE;
}

METHOD(public_key_t, get_keysize, int,
	private_openssl_rsa_public_key_t *this)
{
	return RSA_size(this->rsa) * 8;
}

/**
 * Calculate fingerprint from a RSA key, also used in rsa private key.
 */
bool openssl_rsa_fingerprint(RSA *rsa, cred_encoding_type_t type, chunk_t *fp)
{
	hasher_t *hasher;
	chunk_t key;
	u_char *p;

	if (lib->encoding->get_cache(lib->encoding, type, rsa, fp))
	{
		return TRUE;
	}
	switch (type)
	{
		case KEYID_PUBKEY_SHA1:
			key = chunk_alloc(i2d_RSAPublicKey(rsa, NULL));
			p = key.ptr;
			i2d_RSAPublicKey(rsa, &p);
			break;
		case KEYID_PUBKEY_INFO_SHA1:
			key = chunk_alloc(i2d_RSA_PUBKEY(rsa, NULL));
			p = key.ptr;
			i2d_RSA_PUBKEY(rsa, &p);
			break;
		default:
		{
			const BIGNUM *bn_n, *bn_e;
			chunk_t n = chunk_empty, e = chunk_empty;
			bool success = FALSE;

			RSA_get0_key(rsa, &bn_n, &bn_e, NULL);
			if (openssl_bn2chunk(bn_n, &n) &&
				openssl_bn2chunk(bn_e, &e))
			{
				success = lib->encoding->encode(lib->encoding, type, rsa, fp,
									CRED_PART_RSA_MODULUS, n,
									CRED_PART_RSA_PUB_EXP, e, CRED_PART_END);
			}
			chunk_free(&n);
			chunk_free(&e);
			return success;
		}
	}
	hasher = lib->crypto->create_hasher(lib->crypto, HASH_SHA1);
	if (!hasher || !hasher->allocate_hash(hasher, key, fp))
	{
		DBG1(DBG_LIB, "SHA1 hash algorithm not supported, fingerprinting failed");
		DESTROY_IF(hasher);
		free(key.ptr);
		return FALSE;
	}
	free(key.ptr);
	hasher->destroy(hasher);
	lib->encoding->cache(lib->encoding, type, rsa, *fp);
	return TRUE;
}

METHOD(public_key_t, get_fingerprint, bool,
	private_openssl_rsa_public_key_t *this, cred_encoding_type_t type,
	chunk_t *fingerprint)
{
	return openssl_rsa_fingerprint(this->rsa, type, fingerprint);
}

METHOD(public_key_t, get_encoding, bool,
	private_openssl_rsa_public_key_t *this, cred_encoding_type_t type,
	chunk_t *encoding)
{
	bool success = FALSE;
	u_char *p;

	switch (type)
	{
		case PUBKEY_SPKI_ASN1_DER:
		case PUBKEY_PEM:
		{
			*encoding = chunk_alloc(i2d_RSA_PUBKEY(this->rsa, NULL));
			p = encoding->ptr;
			i2d_RSA_PUBKEY(this->rsa, &p);
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
			*encoding = chunk_alloc(i2d_RSAPublicKey(this->rsa, NULL));
			p = encoding->ptr;
			i2d_RSAPublicKey(this->rsa, &p);
			return TRUE;
		}
		default:
		{
			const BIGNUM *bn_n, *bn_e;
			chunk_t n = chunk_empty, e = chunk_empty;

			RSA_get0_key(this->rsa, &bn_n, &bn_e, NULL);
			if (openssl_bn2chunk(bn_n, &n) &&
				openssl_bn2chunk(bn_e, &e))
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
		if (this->rsa)
		{
			lib->encoding->clear_cache(lib->encoding, this->rsa);
			RSA_free(this->rsa);
		}
		free(this);
	}
}

/**
 * Generic private constructor
 */
static private_openssl_rsa_public_key_t *create_empty()
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

	this = create_empty();
	if (blob.ptr)
	{
		switch (type)
		{
			case KEY_ANY:
				this->rsa = d2i_RSA_PUBKEY(NULL, (const u_char**)&blob.ptr,
										   blob.len);
				break;
			case KEY_RSA:
				this->rsa = d2i_RSAPublicKey(NULL, (const u_char**)&blob.ptr,
											 blob.len);
				break;
			default:
				break;
		}
		if (this->rsa)
		{
			return &this->public;
		}
	}
	else if (n.ptr && e.ptr && type == KEY_RSA)
	{
		BIGNUM *bn_n, *bn_e;

		this->rsa = RSA_new();
		bn_n = BN_bin2bn((const u_char*)n.ptr, n.len, NULL);
		bn_e = BN_bin2bn((const u_char*)e.ptr, e.len, NULL);
		if (RSA_set0_key(this->rsa, bn_n, bn_e, NULL))
		{
			return &this->public;
		}
	}
	destroy(this);
	return NULL;
}

#endif /* OPENSSL_NO_RSA */
