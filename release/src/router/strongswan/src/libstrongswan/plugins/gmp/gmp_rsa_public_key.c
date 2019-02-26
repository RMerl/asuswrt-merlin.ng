/*
 * Copyright (C) 2017-2018 Tobias Brunner
 * Copyright (C) 2005-2009 Martin Willi
 * Copyright (C) 2005 Jan Hutter
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
#include <credentials/keys/signature_params.h>

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
chunk_t gmp_mpz_to_chunk(const mpz_t value);
bool gmp_emsa_pkcs1_signature_data(hash_algorithm_t hash_algorithm,
								   chunk_t data, size_t keylen, chunk_t *em);

/**
 * RSAEP algorithm specified in PKCS#1.
 */
static chunk_t rsaep(private_gmp_rsa_public_key_t *this, chunk_t data)
{
	mpz_t m, c;
	chunk_t encrypted;

	mpz_init(m);
	mpz_import(m, data.len, 1, 1, 1, 0, data.ptr);

	if (mpz_cmp_ui(m, 0) <= 0 || mpz_cmp(m, this->n) >= 0)
	{	/* m must be <= n-1, and while 0 is technically a valid value, it
		 * doesn't really make sense here, so we filter that too */
		mpz_clear(m);
		return chunk_empty;
	}

	mpz_init(c);
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
 * Verification of an EMSA PKCS1 signature described in PKCS#1
 */
static bool verify_emsa_pkcs1_signature(private_gmp_rsa_public_key_t *this,
										hash_algorithm_t algorithm,
										chunk_t data, chunk_t signature)
{
	chunk_t em_expected, em;
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

	/* generate expected signature value */
	if (!gmp_emsa_pkcs1_signature_data(algorithm, data, this->k, &em_expected))
	{
		return FALSE;
	}

	/* unpack signature */
	em = rsavp1(this, signature);

	success = chunk_equals_const(em_expected, em);

	chunk_free(&em_expected);
	chunk_free(&em);
	return success;
}

/**
 * Verification of an EMSA PSS signature described in PKCS#1
 */
static bool verify_emsa_pss_signature(private_gmp_rsa_public_key_t *this,
									  rsa_pss_params_t *params, chunk_t data,
									  chunk_t signature)
{
	ext_out_function_t xof;
	hasher_t *hasher = NULL;
	xof_t *mgf = NULL;
	chunk_t em, hash, salt, db, h, dbmask, m;
	size_t embits, maskbits;
	int i;
	bool success = FALSE;

	if (!params)
	{
		return FALSE;
	}
	xof = xof_mgf1_from_hash_algorithm(params->mgf1_hash);
	if (xof == XOF_UNDEFINED)
	{
		DBG1(DBG_LIB, "%N is not supported for MGF1", hash_algorithm_names,
			 params->mgf1_hash);
		return FALSE;
	}
	chunk_skip_zero(signature);
	if (signature.len == 0 || signature.len > this->k)
	{
		return FALSE;
	}
	/* EM = RSAVP1((n, e), S) */
	em = rsavp1(this, signature);
	if (!em.len)
	{
		goto error;
	}
	/* emBits = modBits - 1 */
	embits = mpz_sizeinbase(this->n, 2) - 1;
	/* mHash = Hash(M) */
	hasher = lib->crypto->create_hasher(lib->crypto, params->hash);
	if (!hasher)
	{
		DBG1(DBG_LIB, "hash algorithm %N not supported",
			 hash_algorithm_names, params->hash);
		goto error;
	}
	hash = chunk_alloca(hasher->get_hash_size(hasher));
	if (!hasher->get_hash(hasher, data, hash.ptr))
	{
		goto error;
	}
	salt.len = params->salt_len;
	/* verify general structure of EM */
	maskbits = (8 * em.len) - embits;
	if (em.len < (hash.len + salt.len + 2) || em.ptr[em.len-1] != 0xbc ||
		(em.ptr[0] & (0xff << (8-maskbits))))
	{	/* inconsistent */
		goto error;
	}
	/* split EM in maskedDB and H */
	db = chunk_create(em.ptr, em.len - hash.len - 1);
	h = chunk_create(em.ptr + db.len, hash.len);
	/* dbMask = MGF(H, emLen - hLen - 1) */
	mgf = lib->crypto->create_xof(lib->crypto, xof);
	if (!mgf)
	{
		DBG1(DBG_LIB, "%N not supported", ext_out_function_names, xof);
		goto error;
	}
	dbmask = chunk_alloca(db.len);
	if (!mgf->set_seed(mgf, h) ||
		!mgf->get_bytes(mgf, dbmask.len, dbmask.ptr))
	{
		DBG1(DBG_LIB, "%N not supported or failed", ext_out_function_names, xof);
		goto error;
	}
	/* DB = maskedDB xor dbMask */
	memxor(db.ptr, dbmask.ptr, db.len);
	if (maskbits)
	{
		db.ptr[0] &= (0xff >> maskbits);
	}
	/* check DB = PS | 0x01 | salt */
	for (i = 0; i < (db.len - salt.len - 1); i++)
	{
		if (db.ptr[i])
		{	/* padding not 0 */
			goto error;
		}
	}
	if (db.ptr[i++] != 0x01)
	{	/* 0x01 not found */
		goto error;
	}
	salt.ptr = &db.ptr[i];
	/* M' = 0x0000000000000000 | mHash | salt */
	m = chunk_cata("ccc",
				   chunk_from_chars(0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00),
				   hash, salt);
	if (!hasher->get_hash(hasher, m, hash.ptr))
	{
		goto error;
	}
	success = memeq_const(h.ptr, hash.ptr, hash.len);

error:
	DESTROY_IF(hasher);
	DESTROY_IF(mgf);
	free(em.ptr);
	return success;
}

METHOD(public_key_t, get_type, key_type_t,
	private_gmp_rsa_public_key_t *this)
{
	return KEY_RSA;
}

METHOD(public_key_t, verify, bool,
	private_gmp_rsa_public_key_t *this, signature_scheme_t scheme, void *params,
	chunk_t data, chunk_t signature)
{
	switch (scheme)
	{
		case SIGN_RSA_EMSA_PKCS1_NULL:
			return verify_emsa_pkcs1_signature(this, HASH_UNKNOWN, data, signature);
		case SIGN_RSA_EMSA_PKCS1_SHA2_224:
			return verify_emsa_pkcs1_signature(this, HASH_SHA224, data, signature);
		case SIGN_RSA_EMSA_PKCS1_SHA2_256:
			return verify_emsa_pkcs1_signature(this, HASH_SHA256, data, signature);
		case SIGN_RSA_EMSA_PKCS1_SHA2_384:
			return verify_emsa_pkcs1_signature(this, HASH_SHA384, data, signature);
		case SIGN_RSA_EMSA_PKCS1_SHA2_512:
			return verify_emsa_pkcs1_signature(this, HASH_SHA512, data, signature);
		case SIGN_RSA_EMSA_PKCS1_SHA3_224:
			return verify_emsa_pkcs1_signature(this, HASH_SHA3_224, data, signature);
		case SIGN_RSA_EMSA_PKCS1_SHA3_256:
			return verify_emsa_pkcs1_signature(this, HASH_SHA3_256, data, signature);
		case SIGN_RSA_EMSA_PKCS1_SHA3_384:
			return verify_emsa_pkcs1_signature(this, HASH_SHA3_384, data, signature);
		case SIGN_RSA_EMSA_PKCS1_SHA3_512:
			return verify_emsa_pkcs1_signature(this, HASH_SHA3_512, data, signature);
		case SIGN_RSA_EMSA_PKCS1_SHA1:
			return verify_emsa_pkcs1_signature(this, HASH_SHA1, data, signature);
		case SIGN_RSA_EMSA_PKCS1_MD5:
			return verify_emsa_pkcs1_signature(this, HASH_MD5, data, signature);
		case SIGN_RSA_EMSA_PSS:
			return verify_emsa_pss_signature(this, params, data, signature);
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
	if (!e.len || !n.len || (n.ptr[n.len-1] & 0x01) == 0)
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

	if (!mpz_sgn(this->e))
	{
		destroy(this);
		return NULL;
	}
	return &this->public;
}
