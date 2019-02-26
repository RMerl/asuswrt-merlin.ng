/*
 * Copyright (C) 2009 Martin Willi
 * Copyright (C) 2008 Tobias Brunner
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

#ifndef OPENSSL_NO_ECDSA

#include "openssl_ec_public_key.h"
#include "openssl_util.h"

#include <utils/debug.h>

#include <openssl/evp.h>
#include <openssl/ecdsa.h>
#include <openssl/x509.h>

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
	EC_KEY *ec;

	/**
	 * reference counter
	 */
	refcount_t ref;
};

/**
 * Verification of a signature as in RFC 4754
 */
static bool verify_signature(private_openssl_ec_public_key_t *this,
							 chunk_t hash, chunk_t signature)
{
	BIGNUM *r, *s;
	ECDSA_SIG *sig;
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
			valid = (ECDSA_do_verify(hash.ptr, hash.len, sig, this->ec) == 1);
		}
		ECDSA_SIG_free(sig);
	}
	return valid;
}

/**
 * Verify a RFC 4754 signature for a specified curve and hash algorithm
 */
static bool verify_curve_signature(private_openssl_ec_public_key_t *this,
								signature_scheme_t scheme, int nid_hash,
								int nid_curve, chunk_t data, chunk_t signature)
{
	const EC_GROUP *my_group;
	EC_GROUP *req_group;
	chunk_t hash;
	bool valid;

	req_group = EC_GROUP_new_by_curve_name(nid_curve);
	if (!req_group)
	{
		DBG1(DBG_LIB, "signature scheme %N not supported in EC (required curve "
			 "not supported)", signature_scheme_names, scheme);
		return FALSE;
	}
	my_group = EC_KEY_get0_group(this->ec);
	if (EC_GROUP_cmp(my_group, req_group, NULL) != 0)
	{
		DBG1(DBG_LIB, "signature scheme %N not supported by private key",
			 signature_scheme_names, scheme);
		return FALSE;
	}
	EC_GROUP_free(req_group);
	if (!openssl_hash_chunk(nid_hash, data, &hash))
	{
		return FALSE;
	}
	valid = verify_signature(this, hash, signature);
	chunk_free(&hash);
	return valid;
}

/**
 * Verification of a DER encoded signature as in RFC 3279
 */
static bool verify_der_signature(private_openssl_ec_public_key_t *this,
								 int nid_hash, chunk_t data, chunk_t signature)
{
	chunk_t hash;
	bool valid = FALSE;

	/* remove any preceding 0-bytes from signature */
	while (signature.len && signature.ptr[0] == 0x00)
	{
		signature = chunk_skip(signature, 1);
	}
	if (openssl_hash_chunk(nid_hash, data, &hash))
	{
		valid = ECDSA_verify(0, hash.ptr, hash.len,
							 signature.ptr, signature.len, this->ec) == 1;
		free(hash.ptr);
	}
	return valid;
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
			return verify_signature(this, data, signature);
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
	chunk_t crypto, chunk_t *plain)
{
	DBG1(DBG_LIB, "EC public key encryption not implemented");
	return FALSE;
}

METHOD(public_key_t, get_keysize, int,
	private_openssl_ec_public_key_t *this)
{
	return EC_GROUP_get_degree(EC_KEY_get0_group(this->ec));
}

/**
 * Calculate fingerprint from a EC_KEY, also used in ec private key.
 */
bool openssl_ec_fingerprint(EC_KEY *ec, cred_encoding_type_t type, chunk_t *fp)
{
	hasher_t *hasher;
	chunk_t key;
	u_char *p;

	if (lib->encoding->get_cache(lib->encoding, type, ec, fp))
	{
		return TRUE;
	}
	switch (type)
	{
		case KEYID_PUBKEY_SHA1:
			key = chunk_alloc(i2o_ECPublicKey(ec, NULL));
			p = key.ptr;
			i2o_ECPublicKey(ec, &p);
			break;
		case KEYID_PUBKEY_INFO_SHA1:
			key = chunk_alloc(i2d_EC_PUBKEY(ec, NULL));
			p = key.ptr;
			i2d_EC_PUBKEY(ec, &p);
			break;
		default:
			return FALSE;
	}
	hasher = lib->crypto->create_hasher(lib->crypto, HASH_SHA1);
	if (!hasher || !hasher->allocate_hash(hasher, key, fp))
	{
		DBG1(DBG_LIB, "SHA1 hash algorithm not supported, fingerprinting failed");
		DESTROY_IF(hasher);
		free(key.ptr);
		return FALSE;
	}
	hasher->destroy(hasher);
	free(key.ptr);
	lib->encoding->cache(lib->encoding, type, ec, *fp);
	return TRUE;
}

METHOD(public_key_t, get_fingerprint, bool,
	private_openssl_ec_public_key_t *this, cred_encoding_type_t type,
	chunk_t *fingerprint)
{
	return openssl_ec_fingerprint(this->ec, type, fingerprint);
}

METHOD(public_key_t, get_encoding, bool,
	private_openssl_ec_public_key_t *this, cred_encoding_type_t type,
	chunk_t *encoding)
{
	bool success = TRUE;
	u_char *p;

	*encoding = chunk_alloc(i2d_EC_PUBKEY(this->ec, NULL));
	p = encoding->ptr;
	i2d_EC_PUBKEY(this->ec, &p);

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
		if (this->ec)
		{
			lib->encoding->clear_cache(lib->encoding, this->ec);
			EC_KEY_free(this->ec);
		}
		free(this);
	}
}

/**
 * Generic private constructor
 */
static private_openssl_ec_public_key_t *create_empty()
{
	private_openssl_ec_public_key_t *this;

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
	);

	return this;
}

/**
 * See header.
 */
openssl_ec_public_key_t *openssl_ec_public_key_load(key_type_t type,
													va_list args)
{
	private_openssl_ec_public_key_t *this;
	chunk_t blob = chunk_empty;

	if (type != KEY_ECDSA)
	{
		return NULL;
	}

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
	this = create_empty();
	this->ec = d2i_EC_PUBKEY(NULL, (const u_char**)&blob.ptr, blob.len);
	if (!this->ec)
	{
		destroy(this);
		return NULL;
	}
	return &this->public;
}
#endif /* OPENSSL_NO_ECDSA */

