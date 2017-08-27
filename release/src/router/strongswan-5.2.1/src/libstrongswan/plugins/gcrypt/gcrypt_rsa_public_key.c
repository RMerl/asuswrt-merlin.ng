/*
 * Copyright (C) 2005-2009 Martin Willi
 * Hochschule fuer Technik Rapperswil
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

#include <gcrypt.h>

#include "gcrypt_rsa_public_key.h"

#include <utils/debug.h>
#include <asn1/oid.h>
#include <asn1/asn1.h>
#include <asn1/asn1_parser.h>
#include <crypto/hashers/hasher.h>

typedef struct private_gcrypt_rsa_public_key_t private_gcrypt_rsa_public_key_t;

/**
 * Private data structure with signing context.
 */
struct private_gcrypt_rsa_public_key_t {

	/**
	 * Public interface for this signer.
	 */
	gcrypt_rsa_public_key_t public;

	/**
	 * gcrypt S-expression representing an public RSA key
	 */
	gcry_sexp_t key;

	/**
	 * reference counter
	 */
	refcount_t ref;
};

/**
 * Implemented in gcrypt_rsa_private_key.c
 */
chunk_t gcrypt_rsa_find_token(gcry_sexp_t sexp, char *name, gcry_sexp_t key);

/**
 * verification of a padded PKCS1 signature without an OID
 */
static bool verify_raw(private_gcrypt_rsa_public_key_t *this,
						 chunk_t data, chunk_t signature)
{
	gcry_sexp_t in, sig;
	gcry_error_t err;
	chunk_t em;
	size_t k;

	/* EM = 0x00 || 0x01 || PS || 0x00 || T
	 * PS = 0xFF padding, with length to fill em
	 * T  = data
	 */
	k = gcry_pk_get_nbits(this->key) / 8;
	if (data.len > k - 3)
	{
		return FALSE;
	}
	em = chunk_alloc(k);
	memset(em.ptr, 0xFF, em.len);
	em.ptr[0] = 0x00;
	em.ptr[1] = 0x01;
	em.ptr[em.len - data.len - 1] = 0x00;
	memcpy(em.ptr + em.len - data.len, data.ptr, data.len);

	err = gcry_sexp_build(&in, NULL, "(data(flags raw)(value %b))",
						  em.len, em.ptr);
	chunk_free(&em);
	if (err)
	{
		DBG1(DBG_LIB, "building data S-expression failed: %s",
			 gpg_strerror(err));
		return FALSE;
	}
	err = gcry_sexp_build(&sig, NULL, "(sig-val(rsa(s %b)))",
						  signature.len, signature.ptr);
	if (err)
	{
		DBG1(DBG_LIB, "building signature S-expression failed: %s",
			 gpg_strerror(err));
		gcry_sexp_release(in);
		return FALSE;
	}
	err = gcry_pk_verify(sig, in, this->key);
	gcry_sexp_release(in);
	gcry_sexp_release(sig);
	if (err)
	{
		DBG1(DBG_LIB, "RSA signature verification failed: %s",
			 gpg_strerror(err));
		return FALSE;
	}
	return TRUE;
}

/**
 * Verification of an EMSA PKCS1 signature described in PKCS#1
 */
static bool verify_pkcs1(private_gcrypt_rsa_public_key_t *this,
						 hash_algorithm_t algorithm, char *hash_name,
						 chunk_t data, chunk_t signature)
{
	hasher_t *hasher;
	chunk_t hash;
	gcry_error_t err;
	gcry_sexp_t in, sig;

	hasher = lib->crypto->create_hasher(lib->crypto, algorithm);
	if (!hasher || !hasher->allocate_hash(hasher, data, &hash))
	{
		DESTROY_IF(hasher);
		return FALSE;
	}
	hasher->destroy(hasher);

	err = gcry_sexp_build(&in, NULL, "(data(flags pkcs1)(hash %s %b))",
						  hash_name, hash.len, hash.ptr);
	chunk_free(&hash);
	if (err)
	{
		DBG1(DBG_LIB, "building data S-expression failed: %s",
			 gpg_strerror(err));
		return FALSE;
	}

	err = gcry_sexp_build(&sig, NULL, "(sig-val(rsa(s %b)))",
						  signature.len, signature.ptr);
	if (err)
	{
		DBG1(DBG_LIB, "building signature S-expression failed: %s",
			 gpg_strerror(err));
		gcry_sexp_release(in);
		return FALSE;
	}
	err = gcry_pk_verify(sig, in, this->key);
	gcry_sexp_release(in);
	gcry_sexp_release(sig);
	if (err)
	{
		DBG1(DBG_LIB, "RSA signature verification failed: %s",
			 gpg_strerror(err));
		return FALSE;
	}
	return TRUE;
}

METHOD(public_key_t, get_type, key_type_t,
	private_gcrypt_rsa_public_key_t *this)
{
	return KEY_RSA;
}

METHOD(public_key_t, verify, bool,
	private_gcrypt_rsa_public_key_t *this, signature_scheme_t scheme,
	chunk_t data, chunk_t signature)
{
	switch (scheme)
	{
		case SIGN_RSA_EMSA_PKCS1_NULL:
			return verify_raw(this, data, signature);
		case SIGN_RSA_EMSA_PKCS1_MD5:
			return verify_pkcs1(this, HASH_MD5, "md5", data, signature);
		case SIGN_RSA_EMSA_PKCS1_SHA1:
			return verify_pkcs1(this, HASH_SHA1, "sha1", data, signature);
		case SIGN_RSA_EMSA_PKCS1_SHA224:
			return verify_pkcs1(this, HASH_SHA224, "sha224", data, signature);
		case SIGN_RSA_EMSA_PKCS1_SHA256:
			return verify_pkcs1(this, HASH_SHA256, "sha256", data, signature);
		case SIGN_RSA_EMSA_PKCS1_SHA384:
			return verify_pkcs1(this, HASH_SHA384, "sha384", data, signature);
		case SIGN_RSA_EMSA_PKCS1_SHA512:
			return verify_pkcs1(this, HASH_SHA512, "sha512", data, signature);
		default:
			DBG1(DBG_LIB, "signature scheme %N not supported in RSA",
				 signature_scheme_names, scheme);
			return FALSE;
	}
}

METHOD(public_key_t, encrypt_, bool,
	private_gcrypt_rsa_public_key_t *this, encryption_scheme_t scheme,
	chunk_t plain, chunk_t *encrypted)
{
	gcry_sexp_t in, out;
	gcry_error_t err;

	if (scheme != ENCRYPT_RSA_PKCS1)
	{
		DBG1(DBG_LIB, "encryption scheme %N not supported",
			 encryption_scheme_names, scheme);
		return FALSE;
	}
	/* "pkcs1" uses PKCS 1.5 (section 8.1) block type 2 encryption:
	 * 00 | 02 | RANDOM | 00 | DATA */
	err = gcry_sexp_build(&in, NULL, "(data(flags pkcs1)(value %b))",
						  plain.len, plain.ptr);
	if (err)
	{
		DBG1(DBG_LIB, "building encryption S-expression failed: %s",
			 gpg_strerror(err));
		return FALSE;
	}
	err = gcry_pk_encrypt(&out, in, this->key);
	gcry_sexp_release(in);
	if (err)
	{
		DBG1(DBG_LIB, "encrypting data using pkcs1 failed: %s",
			 gpg_strerror(err));
		return FALSE;
	}
	*encrypted = gcrypt_rsa_find_token(out, "a", this->key);
	gcry_sexp_release(out);
	return !!encrypted->len;
}

METHOD(public_key_t, get_keysize, int,
	private_gcrypt_rsa_public_key_t *this)
{
	return gcry_pk_get_nbits(this->key);
}

METHOD(public_key_t, get_encoding, bool,
	private_gcrypt_rsa_public_key_t *this, cred_encoding_type_t type,
	chunk_t *encoding)
{
	chunk_t n, e;
	bool success;

	n = gcrypt_rsa_find_token(this->key, "n", NULL);
	e = gcrypt_rsa_find_token(this->key, "e", NULL);
	success = lib->encoding->encode(lib->encoding, type, NULL, encoding,
							CRED_PART_RSA_MODULUS, n, CRED_PART_RSA_PUB_EXP, e,
							CRED_PART_END);
	chunk_free(&n);
	chunk_free(&e);

	return success;
}

METHOD(public_key_t, get_fingerprint, bool,
	private_gcrypt_rsa_public_key_t *this, cred_encoding_type_t type,
	chunk_t *fp)
{
	chunk_t n, e;
	bool success;

	if (lib->encoding->get_cache(lib->encoding, type, this, fp))
	{
		return TRUE;
	}
	n = gcrypt_rsa_find_token(this->key, "n", NULL);
	e = gcrypt_rsa_find_token(this->key, "e", NULL);

	success = lib->encoding->encode(lib->encoding,
								type, this, fp, CRED_PART_RSA_MODULUS, n,
								CRED_PART_RSA_PUB_EXP, e, CRED_PART_END);
	chunk_free(&n);
	chunk_free(&e);
	return success;
}

METHOD(public_key_t, get_ref, public_key_t*,
	private_gcrypt_rsa_public_key_t *this)
{
	ref_get(&this->ref);
	return &this->public.key;
}

METHOD(public_key_t, destroy, void,
	private_gcrypt_rsa_public_key_t *this)
{
	if (ref_put(&this->ref))
	{
		gcry_sexp_release(this->key);
		lib->encoding->clear_cache(lib->encoding, this);
		free(this);
	}
}

/**
 * See header.
 */
gcrypt_rsa_public_key_t *gcrypt_rsa_public_key_load(key_type_t type,
													va_list args)
{
	private_gcrypt_rsa_public_key_t *this;
	gcry_error_t err;
	chunk_t n, e;

	n = e = chunk_empty;
	while (TRUE)
	{
		switch (va_arg(args, builder_part_t))
		{
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

	INIT(this,
		.public = {
			.key = {
				.get_type = _get_type,
				.verify = _verify,
				.encrypt = _encrypt_,
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

	err = gcry_sexp_build(&this->key, NULL, "(public-key(rsa(n %b)(e %b)))",
						  n.len, n.ptr, e.len, e.ptr);
	if (err)
	{
		DBG1(DBG_LIB, "loading public key failed: %s", gpg_strerror(err));
		free(this);
		return NULL;
	}

	return &this->public;
}

