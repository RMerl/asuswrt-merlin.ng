/*
 * Copyright (C) 2009 Martin Willi
 * Copyright (C) 2008 Tobias Brunner
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

#ifndef OPENSSL_NO_ECDSA

#include "openssl_ec_public_key.h"
#include "openssl_util.h"

#include <utils/debug.h>

#include <openssl/evp.h>
#include <openssl/ecdsa.h>
#include <openssl/x509.h>

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
#include <openssl/core_names.h>
#endif

#if OPENSSL_VERSION_NUMBER < 0x10100000L
OPENSSL_KEY_FALLBACK(ECDSA_SIG, r, s)
#endif

typedef struct private_openssl_ec_public_key_t private_openssl_ec_public_key_t;

/**
 * Private data structure with signing context.
 */
struct private_openssl_ec_public_key_t {
	/**
	 * Public interface for this signer.
	 */
	openssl_ec_public_key_t public;

	/**
	 * EC key object
	 */
	EVP_PKEY *key;

	/**
	 * reference counter
	 */
	refcount_t ref;
};

/**
 * Verification of a DER encoded signature as in RFC 3279
 */
static bool verify_der_signature(private_openssl_ec_public_key_t *this,
								 int nid_hash, chunk_t data, chunk_t signature)
{
	EVP_MD_CTX *ctx;
	const EVP_MD *md;

	/* remove any preceding 0-bytes from signature */
	while (signature.len && signature.ptr[0] == 0x00)
	{
		signature = chunk_skip(signature, 1);
	}
	md = EVP_get_digestbynid(nid_hash);
	if (!md)
	{
		return FALSE;
	}
	ctx = EVP_MD_CTX_create();
	if (!ctx ||
		EVP_DigestVerifyInit(ctx, NULL, md, NULL, this->key) <= 0 ||
		EVP_DigestVerifyUpdate(ctx, data.ptr, data.len) <= 0 ||
		EVP_DigestVerifyFinal(ctx, signature.ptr, signature.len) != 1)
	{
		EVP_MD_CTX_destroy(ctx);
		return FALSE;
	}
	EVP_MD_CTX_destroy(ctx);
	return TRUE;
}

/**
 * Verification of a signature as in RFC 4754
 */
static bool verify_signature(private_openssl_ec_public_key_t *this,
							 int nid_hash, chunk_t data, chunk_t signature)
{
	EVP_PKEY_CTX *ctx;
	BIGNUM *r, *s;
	ECDSA_SIG *sig;
	chunk_t der_sig;
	bool valid = FALSE;

	sig = ECDSA_SIG_new();
	if (sig)
	{
		r = BN_new();
		s = BN_new();
		if (!openssl_bn_split(signature, r, s))
		{
			BN_free(r);
			BN_free(s);
			ECDSA_SIG_free(sig);
			return FALSE;
		}
		if (ECDSA_SIG_set0(sig, r, s))
		{
			der_sig = openssl_i2chunk(ECDSA_SIG, sig);
			if (!nid_hash)
			{	/* EVP_DigestVerify*() has issues with NULL EVP_MD */
				ctx = EVP_PKEY_CTX_new(this->key, NULL);
				valid = ctx && EVP_PKEY_verify_init(ctx) > 0 &&
						EVP_PKEY_verify(ctx, der_sig.ptr, der_sig.len,
										data.ptr, data.len) > 0;
				EVP_PKEY_CTX_free(ctx);
			}
			else
			{
				valid = verify_der_signature(this, nid_hash, data, der_sig);
			}
			chunk_free(&der_sig);
		}
		ECDSA_SIG_free(sig);
	}
	return valid;
}

/**
 * Check that the given key's curve matches a specific one. Also used by
 * private key.
 */
bool openssl_check_ec_key_curve(EVP_PKEY *key, int nid_curve)
{
	EC_GROUP *req_group, *my_group = NULL;
	bool matches = FALSE;

	req_group = EC_GROUP_new_by_curve_name(nid_curve);
	if (!req_group)
	{
		goto error;
	}

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
	char name[BUF_LEN];
	OSSL_PARAM params[] = {
		OSSL_PARAM_utf8_string(OSSL_PKEY_PARAM_GROUP_NAME, name, sizeof(name)),
		OSSL_PARAM_END,
	};

	if (!EVP_PKEY_get_group_name(key, name, sizeof(name), NULL))
	{
		goto error;
	}
	my_group = EC_GROUP_new_from_params(params, NULL, NULL);
#elif OPENSSL_VERSION_NUMBER >= 0x1010000fL
	EC_KEY *ec = EVP_PKEY_get0_EC_KEY(key);
	my_group = EC_GROUP_dup(EC_KEY_get0_group(ec));
#else
	EC_KEY *ec = EVP_PKEY_get1_EC_KEY(key);
	my_group = EC_GROUP_dup(EC_KEY_get0_group(ec));
	EC_KEY_free(ec);
#endif

	if (EC_GROUP_cmp(my_group, req_group, NULL) == 0)
	{
		matches = TRUE;
	}

error:
	EC_GROUP_free(my_group);
	EC_GROUP_free(req_group);
	return matches;
}

/**
 * Verify a RFC 4754 signature for a specified curve and hash algorithm
 */
static bool verify_curve_signature(private_openssl_ec_public_key_t *this,
								signature_scheme_t scheme, int nid_hash,
								int nid_curve, chunk_t data, chunk_t signature)
{
	if (!openssl_check_ec_key_curve(this->key, nid_curve))
	{
		DBG1(DBG_LIB, "signature scheme %N not supported by key",
			 signature_scheme_names, scheme);
		return FALSE;
	}
	return verify_signature(this, nid_hash, data, signature);
}

METHOD(public_key_t, get_type, key_type_t,
	private_openssl_ec_public_key_t *this)
{
	return KEY_ECDSA;
}

METHOD(public_key_t, verify, bool,
	private_openssl_ec_public_key_t *this, signature_scheme_t scheme,
	void *params, chunk_t data, chunk_t signature)
{
	switch (scheme)
	{
		case SIGN_ECDSA_WITH_SHA1_DER:
			return verify_der_signature(this, NID_sha1, data, signature);
		case SIGN_ECDSA_WITH_SHA256_DER:
			return verify_der_signature(this, NID_sha256, data, signature);
		case SIGN_ECDSA_WITH_SHA384_DER:
			return verify_der_signature(this, NID_sha384, data, signature);
		case SIGN_ECDSA_WITH_SHA512_DER:
			return verify_der_signature(this, NID_sha512, data, signature);
		case SIGN_ECDSA_WITH_NULL:
			return verify_signature(this, 0, data, signature);
		case SIGN_ECDSA_256:
			return verify_curve_signature(this, scheme, NID_sha256,
										  NID_X9_62_prime256v1, data, signature);
		case SIGN_ECDSA_384:
			return verify_curve_signature(this, scheme, NID_sha384,
										  NID_secp384r1, data, signature);
		case SIGN_ECDSA_521:
			return verify_curve_signature(this, scheme, NID_sha512,
										  NID_secp521r1, data, signature);
		default:
			DBG1(DBG_LIB, "signature scheme %N not supported in EC",
				 signature_scheme_names, scheme);
			return FALSE;
	}
}

METHOD(public_key_t, encrypt, bool,
	private_openssl_ec_public_key_t *this, encryption_scheme_t scheme,
	void *params, chunk_t crypto, chunk_t *plain)
{
	DBG1(DBG_LIB, "EC public key encryption not implemented");
	return FALSE;
}

METHOD(public_key_t, get_keysize, int,
	private_openssl_ec_public_key_t *this)
{
	return EVP_PKEY_bits(this->key);
}

METHOD(public_key_t, get_fingerprint, bool,
	private_openssl_ec_public_key_t *this, cred_encoding_type_t type,
	chunk_t *fingerprint)
{
	return openssl_fingerprint(this->key, type, fingerprint);
}

METHOD(public_key_t, get_encoding, bool,
	private_openssl_ec_public_key_t *this, cred_encoding_type_t type,
	chunk_t *encoding)
{
	bool success = TRUE;

	*encoding = openssl_i2chunk(PUBKEY, this->key);

	if (type != PUBKEY_SPKI_ASN1_DER)
	{
		chunk_t asn1_encoding = *encoding;

		success = lib->encoding->encode(lib->encoding, type,
						NULL, encoding, CRED_PART_ECDSA_PUB_ASN1_DER,
						asn1_encoding, CRED_PART_END);
		chunk_clear(&asn1_encoding);
	}
	return success;
}

METHOD(public_key_t, get_ref, public_key_t*,
	private_openssl_ec_public_key_t *this)
{
	ref_get(&this->ref);
	return &this->public.key;
}

METHOD(public_key_t, destroy, void,
	private_openssl_ec_public_key_t *this)
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
 * Check whether the EC key was decoded with explicit curve parameters instead
 * of a named curve.
 */
bool openssl_check_explicit_params(const EVP_PKEY *key)
{
	int explicit = 0;

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
	if (!EVP_PKEY_get_int_param(key, OSSL_PKEY_PARAM_EC_DECODED_FROM_EXPLICIT_PARAMS,
								&explicit))
	{
		return FALSE;
	}
#elif OPENSSL_VERSION_NUMBER >= 0x1010108fL
	explicit = EC_KEY_decoded_from_explicit_params(EVP_PKEY_get0_EC_KEY((EVP_PKEY*)key));
#endif
	return explicit == 1;
}

/**
 * See header.
 */
openssl_ec_public_key_t *openssl_ec_public_key_load(key_type_t type,
													va_list args)
{
	private_openssl_ec_public_key_t *this;
	chunk_t blob = chunk_empty;
	EVP_PKEY *key;

	while (TRUE)
	{
		switch (va_arg(args, builder_part_t))
		{
			case BUILD_BLOB_ASN1_DER:
				blob = va_arg(args, chunk_t);
				continue;
			case BUILD_END:
				break;
			default:
				return NULL;
		}
		break;
	}
	key = d2i_PUBKEY(NULL, (const u_char**)&blob.ptr, blob.len);
	if (!key || EVP_PKEY_base_id(key) != EVP_PKEY_EC ||
		openssl_check_explicit_params(key))
	{
		EVP_PKEY_free(key);
		return NULL;
	}

	INIT(this,
		.public = {
			.key = {
				.get_type = _get_type,
				.verify = _verify,
				.encrypt = _encrypt,
				.get_keysize = _get_keysize,
				.equals = public_key_equals,
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
	return &this->public;
}

#endif /* OPENSSL_NO_ECDSA */
