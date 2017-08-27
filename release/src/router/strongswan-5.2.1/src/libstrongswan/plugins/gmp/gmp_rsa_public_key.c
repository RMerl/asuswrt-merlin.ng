/*
 * Copyright (C) 2005-2009 Martin Willi
 * Copyright (C) 2005 Jan Hutter
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

#include <gmp.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "gmp_rsa_public_key.h"

#include <utils/debug.h>
#include <asn1/oid.h>
#include <asn1/asn1.h>
#include <asn1/asn1_parser.h>
#include <crypto/hashers/hasher.h>

#ifdef HAVE_MPZ_POWM_SEC
# undef mpz_powm
# define mpz_powm mpz_powm_sec
#endif

typedef struct private_gmp_rsa_public_key_t private_gmp_rsa_public_key_t;

/**
 * Private data structure with signing context.
 */
struct private_gmp_rsa_public_key_t {
	/**
	 * Public interface for this signer.
	 */
	gmp_rsa_public_key_t public;

	/**
	 * Public modulus.
	 */
	mpz_t n;

	/**
	 * Public exponent.
	 */
	mpz_t e;

	/**
	 * Keysize in bytes.
	 */
	size_t k;

	/**
	 * reference counter
	 */
	refcount_t ref;
};

/**
 * Shared functions defined in gmp_rsa_private_key.c
 */
extern chunk_t gmp_mpz_to_chunk(const mpz_t value);

/**
 * RSAEP algorithm specified in PKCS#1.
 */
static chunk_t rsaep(private_gmp_rsa_public_key_t *this, chunk_t data)
{
	mpz_t m, c;
	chunk_t encrypted;

	mpz_init(c);
	mpz_init(m);

	mpz_import(m, data.len, 1, 1, 1, 0, data.ptr);

	mpz_powm(c, m, this->e, this->n);

	encrypted.len = this->k;
	encrypted.ptr = mpz_export(NULL, NULL, 1, encrypted.len, 1, 0, c);
	if (encrypted.ptr == NULL)
	{
		encrypted.len = 0;
	}

	mpz_clear(c);
	mpz_clear(m);

	return encrypted;
}

/**
 * RSAVP1 algorithm specified in PKCS#1.
 */
static chunk_t rsavp1(private_gmp_rsa_public_key_t *this, chunk_t data)
{
	return rsaep(this, data);
}

/**
 * ASN.1 definition of digestInfo
 */
static const asn1Object_t digestInfoObjects[] = {
	{ 0, "digestInfo",			ASN1_SEQUENCE,		ASN1_OBJ  }, /*  0 */
	{ 1,   "digestAlgorithm",	ASN1_EOC,			ASN1_RAW  }, /*  1 */
	{ 1,   "digest",			ASN1_OCTET_STRING,	ASN1_BODY }, /*  2 */
	{ 0, "exit",				ASN1_EOC,			ASN1_EXIT }
};
#define DIGEST_INFO					0
#define DIGEST_INFO_ALGORITHM		1
#define DIGEST_INFO_DIGEST			2

/**
 * Verification of an EMPSA PKCS1 signature described in PKCS#1
 */
static bool verify_emsa_pkcs1_signature(private_gmp_rsa_public_key_t *this,
										hash_algorithm_t algorithm,
										chunk_t data, chunk_t signature)
{
	chunk_t em_ori, em;
	bool success = FALSE;

	/* remove any preceding 0-bytes from signature */
	while (signature.len && *(signature.ptr) == 0x00)
	{
		signature = chunk_skip(signature, 1);
	}

	if (signature.len == 0 || signature.len > this->k)
	{
		return FALSE;
	}

	/* unpack signature */
	em_ori = em = rsavp1(this, signature);

	/* result should look like this:
	 * EM = 0x00 || 0x01 || PS || 0x00 || T.
	 * PS = 0xFF padding, with length to fill em
	 * T = oid || hash
	 */

	/* check magic bytes */
	if (*(em.ptr) != 0x00 || *(em.ptr+1) != 0x01)
	{
		goto end;
	}
	em = chunk_skip(em, 2);

	/* find magic 0x00 */
	while (em.len > 0)
	{
		if (*em.ptr == 0x00)
		{
			/* found magic byte, stop */
			em = chunk_skip(em, 1);
			break;
		}
		else if (*em.ptr != 0xFF)
		{
			/* bad padding, decryption failed ?!*/
			goto end;
		}
		em = chunk_skip(em, 1);
	}

	if (em.len == 0)
	{
		/* no digestInfo found */
		goto end;
	}

	if (algorithm == HASH_UNKNOWN)
	{   /* IKEv1 signatures without digestInfo */
		if (em.len != data.len)
		{
			DBG1(DBG_LIB, "hash size in signature is %u bytes instead of"
				 " %u bytes", em.len, data.len);
			goto end;
		}
		success = memeq(em.ptr, data.ptr, data.len);
	}
	else
	{   /* IKEv2 and X.509 certificate signatures */
		asn1_parser_t *parser;
		chunk_t object;
		int objectID;
		hash_algorithm_t hash_algorithm = HASH_UNKNOWN;

		DBG2(DBG_LIB, "signature verification:");
		parser = asn1_parser_create(digestInfoObjects, em);

		while (parser->iterate(parser, &objectID, &object))
		{
			switch (objectID)
			{
				case DIGEST_INFO:
				{
					if (em.len > object.len)
					{
						DBG1(DBG_LIB, "digestInfo field in signature is"
							 " followed by %u surplus bytes",
							 em.len - object.len);
						goto end_parser;
					}
					break;
				}
				case DIGEST_INFO_ALGORITHM:
				{
					int hash_oid = asn1_parse_algorithmIdentifier(object,
										 parser->get_level(parser)+1, NULL);

					hash_algorithm = hasher_algorithm_from_oid(hash_oid);
					if (hash_algorithm == HASH_UNKNOWN || hash_algorithm != algorithm)
					{
						DBG1(DBG_LIB, "expected hash algorithm %N, but found"
							 " %N (OID: %#B)", hash_algorithm_names, algorithm,
							 hash_algorithm_names, hash_algorithm,  &object);
						goto end_parser;
					}
					break;
				}
				case DIGEST_INFO_DIGEST:
				{
					chunk_t hash;
					hasher_t *hasher;

					hasher = lib->crypto->create_hasher(lib->crypto, hash_algorithm);
					if (hasher == NULL)
					{
						DBG1(DBG_LIB, "hash algorithm %N not supported",
							 hash_algorithm_names, hash_algorithm);
						goto end_parser;
					}

					if (object.len != hasher->get_hash_size(hasher))
					{
						DBG1(DBG_LIB, "hash size in signature is %u bytes"
							 " instead of %u bytes", object.len,
							 hasher->get_hash_size(hasher));
						hasher->destroy(hasher);
						goto end_parser;
					}

					/* build our own hash and compare */
					if (!hasher->allocate_hash(hasher, data, &hash))
					{
						hasher->destroy(hasher);
						goto end_parser;
					}
					hasher->destroy(hasher);
					success = memeq(object.ptr, hash.ptr, hash.len);
					free(hash.ptr);
					break;
				}
				default:
					break;
			}
		}

end_parser:
		success &= parser->success(parser);
		parser->destroy(parser);
	}

end:
	free(em_ori.ptr);
	return success;
}

METHOD(public_key_t, get_type, key_type_t,
	private_gmp_rsa_public_key_t *this)
{
	return KEY_RSA;
}

METHOD(public_key_t, verify, bool,
	private_gmp_rsa_public_key_t *this, signature_scheme_t scheme,
	chunk_t data, chunk_t signature)
{
	switch (scheme)
	{
		case SIGN_RSA_EMSA_PKCS1_NULL:
			return verify_emsa_pkcs1_signature(this, HASH_UNKNOWN, data, signature);
		case SIGN_RSA_EMSA_PKCS1_MD5:
			return verify_emsa_pkcs1_signature(this, HASH_MD5, data, signature);
		case SIGN_RSA_EMSA_PKCS1_SHA1:
			return verify_emsa_pkcs1_signature(this, HASH_SHA1, data, signature);
		case SIGN_RSA_EMSA_PKCS1_SHA224:
			return verify_emsa_pkcs1_signature(this, HASH_SHA224, data, signature);
		case SIGN_RSA_EMSA_PKCS1_SHA256:
			return verify_emsa_pkcs1_signature(this, HASH_SHA256, data, signature);
		case SIGN_RSA_EMSA_PKCS1_SHA384:
			return verify_emsa_pkcs1_signature(this, HASH_SHA384, data, signature);
		case SIGN_RSA_EMSA_PKCS1_SHA512:
			return verify_emsa_pkcs1_signature(this, HASH_SHA512, data, signature);
		default:
			DBG1(DBG_LIB, "signature scheme %N not supported in RSA",
				 signature_scheme_names, scheme);
			return FALSE;
	}
}

#define MIN_PS_PADDING 8

METHOD(public_key_t, encrypt_, bool,
	private_gmp_rsa_public_key_t *this, encryption_scheme_t scheme,
	chunk_t plain, chunk_t *crypto)
{
	chunk_t em;
	u_char *pos;
	int padding;
	rng_t *rng;

	if (scheme != ENCRYPT_RSA_PKCS1)
	{
		DBG1(DBG_LIB, "encryption scheme %N not supported",
			 encryption_scheme_names, scheme);
		return FALSE;
	}
	/* number of pseudo-random padding octets */
	padding = this->k - plain.len - 3;
	if (padding < MIN_PS_PADDING)
	{
		DBG1(DBG_LIB, "pseudo-random padding must be at least %d octets",
			 MIN_PS_PADDING);
		return FALSE;
	}
	rng = lib->crypto->create_rng(lib->crypto, RNG_WEAK);
	if (rng == NULL)
	{
		DBG1(DBG_LIB, "no random generator available");
		return FALSE;
	}

	/* padding according to PKCS#1 7.2.1 (RSAES-PKCS1-v1.5-ENCRYPT) */
	DBG2(DBG_LIB, "padding %u bytes of data to the rsa modulus size of"
		 " %u bytes", plain.len, this->k);
	em.len = this->k;
	em.ptr = malloc(em.len);
	pos = em.ptr;
	*pos++ = 0x00;
	*pos++ = 0x02;

	/* fill with pseudo random octets */
	if (!rng_get_bytes_not_zero(rng, padding, pos, TRUE))
	{
		DBG1(DBG_LIB, "failed to allocate padding");
		chunk_clear(&em);
		rng->destroy(rng);
		return FALSE;
	}
	rng->destroy(rng);

	pos += padding;

	/* append the padding terminator */
	*pos++ = 0x00;

	/* now add the data */
	memcpy(pos, plain.ptr, plain.len);
	DBG3(DBG_LIB, "padded data before rsa encryption: %B", &em);

	/* rsa encryption using PKCS#1 RSAEP */
	*crypto = rsaep(this, em);
	DBG3(DBG_LIB, "rsa encrypted data: %B", crypto);
	chunk_clear(&em);
	return TRUE;
}

METHOD(public_key_t, get_keysize, int,
	private_gmp_rsa_public_key_t *this)
{
	return mpz_sizeinbase(this->n, 2);
}

METHOD(public_key_t, get_encoding, bool,
	private_gmp_rsa_public_key_t *this, cred_encoding_type_t type,
	chunk_t *encoding)
{
	chunk_t n, e;
	bool success;

	n = gmp_mpz_to_chunk(this->n);
	e = gmp_mpz_to_chunk(this->e);

	success = lib->encoding->encode(lib->encoding, type, NULL, encoding,
			CRED_PART_RSA_MODULUS, n, CRED_PART_RSA_PUB_EXP, e, CRED_PART_END);
	chunk_free(&n);
	chunk_free(&e);

	return success;
}

METHOD(public_key_t, get_fingerprint, bool,
	private_gmp_rsa_public_key_t *this, cred_encoding_type_t type, chunk_t *fp)
{
	chunk_t n, e;
	bool success;

	if (lib->encoding->get_cache(lib->encoding, type, this, fp))
	{
		return TRUE;
	}
	n = gmp_mpz_to_chunk(this->n);
	e = gmp_mpz_to_chunk(this->e);

	success = lib->encoding->encode(lib->encoding, type, this, fp,
			CRED_PART_RSA_MODULUS, n, CRED_PART_RSA_PUB_EXP, e, CRED_PART_END);
	chunk_free(&n);
	chunk_free(&e);

	return success;
}

METHOD(public_key_t, get_ref, public_key_t*,
	private_gmp_rsa_public_key_t *this)
{
	ref_get(&this->ref);
	return &this->public.key;
}

METHOD(public_key_t, destroy, void,
	private_gmp_rsa_public_key_t *this)
{
	if (ref_put(&this->ref))
	{
		mpz_clear(this->n);
		mpz_clear(this->e);
		lib->encoding->clear_cache(lib->encoding, this);
		free(this);
	}
}

/**
 * See header.
 */
gmp_rsa_public_key_t *gmp_rsa_public_key_load(key_type_t type, va_list args)
{
	private_gmp_rsa_public_key_t *this;
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
	if (!e.ptr || !n.ptr)
	{
		return NULL;
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

	mpz_init(this->n);
	mpz_init(this->e);

	mpz_import(this->n, n.len, 1, 1, 1, 0, n.ptr);
	mpz_import(this->e, e.len, 1, 1, 1, 0, e.ptr);

	this->k = (mpz_sizeinbase(this->n, 2) + 7) / BITS_PER_BYTE;

	return &this->public;
}

