/*
 * Copyright (C) 2008-2016 Tobias Brunner
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

#ifndef OPENSSL_NO_ECDSA

#include "openssl_ec_private_key.h"
#include "openssl_ec_public_key.h"
#include "openssl_util.h"

#include <utils/debug.h>

#include <openssl/evp.h>
#include <openssl/ecdsa.h>
#include <openssl/x509.h>

#if OPENSSL_VERSION_NUMBER < 0x10100000L
OPENSSL_KEY_FALLBACK(ECDSA_SIG, r, s)
#endif

typedef struct private_openssl_ec_private_key_t private_openssl_ec_private_key_t;

/**
 * Private data of a openssl_ec_private_key_t object.
 */
struct private_openssl_ec_private_key_t {
	/**
	 * Public interface for this signer.
	 */
	openssl_ec_private_key_t public;

	/**
	 * EC key object
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

/* from openssl_ec_public_key */
bool openssl_check_ec_key_curve(EVP_PKEY *key, int nid_curve);
bool openssl_check_explicit_params(EVP_PKEY *key);

/**
 * Build a DER encoded signature as in RFC 3279
 */
static bool build_der_signature(private_openssl_ec_private_key_t *this,
								int nid_hash, chunk_t data, chunk_t *signature)
{
	EVP_MD_CTX *ctx;
	const EVP_MD *md;

	md = EVP_get_digestbynid(nid_hash);
	if (!md)
	{
		return FALSE;
	}
	*signature = chunk_alloc(EVP_PKEY_size(this->key));
	ctx = EVP_MD_CTX_create();
	if (!ctx ||
		EVP_DigestSignInit(ctx, NULL, md, NULL, this->key) <= 0 ||
		EVP_DigestSignUpdate(ctx, data.ptr, data.len) <= 0 ||
		EVP_DigestSignFinal(ctx, signature->ptr, &signature->len) != 1)
	{
		chunk_free(signature);
		EVP_MD_CTX_destroy(ctx);
		return FALSE;
	}
	EVP_MD_CTX_destroy(ctx);
	return TRUE;
}

/**
 * Build a signature as in RFC 4754
 */
static bool build_signature(private_openssl_ec_private_key_t *this,
							int nid_hash, chunk_t data, chunk_t *signature)
{
	EVP_PKEY_CTX *ctx;
	ECDSA_SIG *sig;
	const BIGNUM *r, *s;
	const u_char *p;
	chunk_t der_sig;
	bool built = FALSE;

	if (!nid_hash)
	{	/* EVP_DigestSign*() has issues with NULL EVP_MD */
		der_sig = chunk_alloc(EVP_PKEY_size(this->key));
		ctx = EVP_PKEY_CTX_new(this->key, NULL);
		if (!ctx ||
			EVP_PKEY_sign_init(ctx) <= 0 ||
			EVP_PKEY_sign(ctx, der_sig.ptr, &der_sig.len, data.ptr, data.len) <= 0)
		{
			chunk_free(&der_sig);
			EVP_PKEY_CTX_free(ctx);
			return FALSE;
		}
		EVP_PKEY_CTX_free(ctx);
	}
	else if (!build_der_signature(this, nid_hash, data, &der_sig))
	{
		return FALSE;
	}
	/* extract r and s from the DER-encoded signature */
	p = der_sig.ptr;
	sig = d2i_ECDSA_SIG(NULL, &p, der_sig.len);
	chunk_free(&der_sig);
	if (sig)
	{
		ECDSA_SIG_get0(sig, &r, &s);
		/* concatenate BNs r/s to a signature chunk */
		built = openssl_bn_cat((EVP_PKEY_bits(this->key) + 7) / 8,
							   r, s, signature);
		ECDSA_SIG_free(sig);
	}
	return built;
}

/**
 * Build a RFC 4754 signature for a specified curve and hash algorithm
 */
static bool build_curve_signature(private_openssl_ec_private_key_t *this,
								signature_scheme_t scheme, int nid_hash,
								int nid_curve, chunk_t data, chunk_t *signature)
{
	if (!openssl_check_ec_key_curve(this->key, nid_curve))
	{
		DBG1(DBG_LIB, "signature scheme %N not supported by key",
			 signature_scheme_names, scheme);
		return FALSE;
	}
	return build_signature(this, nid_hash, data, signature);
}

METHOD(private_key_t, sign, bool,
	private_openssl_ec_private_key_t *this, signature_scheme_t scheme,
	void *params, chunk_t data, chunk_t *signature)
{
	switch (scheme)
	{
		case SIGN_ECDSA_WITH_NULL:
			return build_signature(this, 0, data, signature);
		case SIGN_ECDSA_WITH_SHA1_DER:
			return build_der_signature(this, NID_sha1, data, signature);
		case SIGN_ECDSA_WITH_SHA256_DER:
			return build_der_signature(this, NID_sha256, data, signature);
		case SIGN_ECDSA_WITH_SHA384_DER:
			return build_der_signature(this, NID_sha384, data, signature);
		case SIGN_ECDSA_WITH_SHA512_DER:
			return build_der_signature(this, NID_sha512, data, signature);
		case SIGN_ECDSA_256:
			return build_curve_signature(this, scheme, NID_sha256,
										 NID_X9_62_prime256v1, data, signature);
		case SIGN_ECDSA_384:
			return build_curve_signature(this, scheme, NID_sha384,
										 NID_secp384r1, data, signature);
		case SIGN_ECDSA_521:
			return build_curve_signature(this, scheme, NID_sha512,
										 NID_secp521r1, data, signature);
		default:
			DBG1(DBG_LIB, "signature scheme %N not supported",
				 signature_scheme_names, scheme);
			return FALSE;
	}
}

METHOD(private_key_t, decrypt, bool,
	private_openssl_ec_private_key_t *this, encryption_scheme_t scheme,
	void *params, chunk_t crypto, chunk_t *plain)
{
	DBG1(DBG_LIB, "EC private key decryption not implemented");
	return FALSE;
}

METHOD(private_key_t, get_keysize, int,
	private_openssl_ec_private_key_t *this)
{
	return EVP_PKEY_bits(this->key);
}

METHOD(private_key_t, get_type, key_type_t,
	private_openssl_ec_private_key_t *this)
{
	return KEY_ECDSA;
}

METHOD(private_key_t, get_public_key, public_key_t*,
	private_openssl_ec_private_key_t *this)
{
	public_key_t *public;
	chunk_t key;

	key = openssl_i2chunk(PUBKEY, this->key);
	public = lib->creds->create(lib->creds, CRED_PUBLIC_KEY, KEY_ECDSA,
								BUILD_BLOB_ASN1_DER, key, BUILD_END);
	free(key.ptr);
	return public;
}

METHOD(private_key_t, get_fingerprint, bool,
	private_openssl_ec_private_key_t *this, cred_encoding_type_t type,
	chunk_t *fingerprint)
{
	return openssl_fingerprint(this->key, type, fingerprint);
}

METHOD(private_key_t, get_encoding, bool,
	private_openssl_ec_private_key_t *this, cred_encoding_type_t type,
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
								NULL, encoding, CRED_PART_ECDSA_PRIV_ASN1_DER,
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
	private_openssl_ec_private_key_t *this)
{
	ref_get(&this->ref);
	return &this->public.key;
}

METHOD(private_key_t, destroy, void,
	private_openssl_ec_private_key_t *this)
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
static private_openssl_ec_private_key_t *create_internal(EVP_PKEY *key)
{
	private_openssl_ec_private_key_t *this;

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
private_key_t *openssl_ec_private_key_create(EVP_PKEY *key, bool engine)
{
	private_openssl_ec_private_key_t *this;

	if (EVP_PKEY_base_id(key) != EVP_PKEY_EC)
	{
		EVP_PKEY_free(key);
		return NULL;
	}
	this = create_internal(key);
	this->engine = engine;
	return &this->public.key;
}

/*
 * See header.
 */
openssl_ec_private_key_t *openssl_ec_private_key_gen(key_type_t type,
													 va_list args)
{
	private_openssl_ec_private_key_t *this;
	EVP_PKEY *key = NULL;
	u_int key_size = 0;

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

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
	switch (key_size)
	{
		case 256:
			key = EVP_EC_gen("P-256");
			break;
		case 384:
			key = EVP_EC_gen("P-384");
			break;
		case 521:
			key = EVP_EC_gen("P-521");
			break;
		default:
			DBG1(DBG_LIB, "EC private key size %d not supported", key_size);
			return NULL;
	}
#else /* OPENSSL_VERSION_NUMBER */
	EC_KEY *ec;

	switch (key_size)
	{
		case 256:
			ec = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
			break;
		case 384:
			ec = EC_KEY_new_by_curve_name(NID_secp384r1);
			break;
		case 521:
			ec = EC_KEY_new_by_curve_name(NID_secp521r1);
			break;
		default:
			DBG1(DBG_LIB, "EC private key size %d not supported", key_size);
			return NULL;
	}
	if (ec && EC_KEY_generate_key(ec) == 1)
	{
		key = EVP_PKEY_new();
		if (!EVP_PKEY_assign_EC_KEY(key, ec))
		{
			EC_KEY_free(ec);
			EVP_PKEY_free(key);
			key = NULL;
		}
	}
#endif /* OPENSSL_VERSION_NUMBER */

	if (!key)
	{
		return NULL;
	}
	this = create_internal(key);
	return &this->public;
}

/**
 * See header.
 */
openssl_ec_private_key_t *openssl_ec_private_key_load(key_type_t type,
													  va_list args)
{
	private_openssl_ec_private_key_t *this;
	chunk_t par = chunk_empty, blob = chunk_empty;
	EVP_PKEY *key = NULL;

	while (TRUE)
	{
		switch (va_arg(args, builder_part_t))
		{
			case BUILD_BLOB_ALGID_PARAMS:
				par = va_arg(args, chunk_t);
				continue;
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

	if (par.ptr)
	{
		/* for OpenSSL 3, the combination of d2i_KeyParams/d2i_PrivateKey, which
		 * are intended to replace the functions below, does currently not work
		 * because OpenSSL does not pass the internal EC_KEY that stores the
		 * parameters from the first call to the call that parses the private
		 * key. however, since parsing PKCS#8 is the only use case for this and
		 * OpenSSL 3 parses this format directly, there isn't really any need
		 * for it anyway */
#if OPENSSL_VERSION_NUMBER < 0x30000000L
		EC_KEY *ec;

		ec = d2i_ECParameters(NULL, (const u_char**)&par.ptr, par.len);
		if (ec && d2i_ECPrivateKey(&ec, (const u_char**)&blob.ptr, blob.len))
		{
			key = EVP_PKEY_new();
			if (!EVP_PKEY_assign_EC_KEY(key, ec))
			{
				EC_KEY_free(ec);
				EVP_PKEY_free(key);
				key = NULL;
			}
		}
		else
		{
			EC_KEY_free(ec);
		}
#endif
	}
	else
	{
		key = d2i_PrivateKey(EVP_PKEY_EC, NULL, (const u_char**)&blob.ptr,
							 blob.len);
	}

	if (!key || openssl_check_explicit_params(key))
	{
		EVP_PKEY_free(key);
		return NULL;
	}
	this = create_internal(key);
	return &this->public;
}
#endif /* OPENSSL_NO_ECDSA */
