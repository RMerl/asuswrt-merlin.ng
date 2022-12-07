/*
 * Copyright (C) 2017-2018 Tobias Brunner
 * Copyright (C) 2005 Jan Hutter
 * Copyright (C) 2005-2009 Martin Willi
 * Copyright (C) 2012-2019 Andreas Steffen
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
#include <string.h>

#include "gmp_rsa_private_key.h"
#include "gmp_rsa_public_key.h"

#include <utils/debug.h>
#include <asn1/oid.h>
#include <asn1/asn1.h>
#include <asn1/asn1_parser.h>
#include <credentials/keys/signature_params.h>

#ifdef HAVE_MPZ_POWM_SEC
# undef mpz_powm
# define mpz_powm mpz_powm_sec
#endif

/**
 *  Public exponent to use for key generation.
 */
#define PUBLIC_EXPONENT 0x10001

typedef struct private_gmp_rsa_private_key_t private_gmp_rsa_private_key_t;

/**
 * Private data of a gmp_rsa_private_key_t object.
 */
struct private_gmp_rsa_private_key_t {
	/**
	 * Public interface for this signer.
	 */
	gmp_rsa_private_key_t public;

	/**
	 * Public modulus.
	 */
	mpz_t n;

	/**
	 * Public exponent.
	 */
	mpz_t e;

	/**
	 * Private prime 1.
	 */
	mpz_t p;

	/**
	 * Private Prime 2.
	 */
	mpz_t q;

	/**
	 * Carmichael function m = lambda(n) = lcm(p-1,q-1).
	*/
	mpz_t m;

	/**
	 * Private exponent and optional secret sharing polynomial coefficients.
	 */
	mpz_t *d;

	/**
	 * Private exponent 1.
	 */
	mpz_t exp1;

	/**
	 * Private exponent 2.
	 */
	mpz_t exp2;

	/**
	 * Private coefficient.
	 */
	mpz_t coeff;

	/**
	 * Total number of private key shares
	 */
	u_int shares;

	/**
	 * Secret sharing threshold
	 */
	u_int threshold;

	/**
	 * Optional verification key (threshold > 1).
	 */
	mpz_t v;

	/**
	 * Keysize in bytes.
	 */
	size_t k;

	/**
	 * reference count
	 */
	refcount_t ref;
};

/**
 * Convert a MP integer into a chunk_t
 */
chunk_t gmp_mpz_to_chunk(const mpz_t value)
{
	chunk_t n;

	n.len = 1 + mpz_sizeinbase(value, 2) / BITS_PER_BYTE;
	n.ptr = mpz_export(NULL, NULL, 1, n.len, 1, 0, value);
	if (n.ptr == NULL)
	{	/* if we have zero in "value", gmp returns NULL */
		n.len = 0;
	}
	return n;
}

/**
 * Auxiliary function overwriting private key material with zero bytes
 */
static void mpz_clear_sensitive(mpz_t z)
{
	size_t len = mpz_size(z) * GMP_LIMB_BITS / BITS_PER_BYTE;
	uint8_t *zeros = alloca(len);

	memset(zeros, 0, len);
	/* overwrite mpz_t with zero bytes before clearing it */
	mpz_import(z, len, 1, 1, 1, 0, zeros);
	mpz_clear(z);
}

/**
 * Create a mpz prime of at least prime_size
 */
static status_t compute_prime(drbg_t *drbg, size_t prime_size, bool safe, mpz_t *p, mpz_t *q)
{
	chunk_t random_bytes;
	int count = 0;

	mpz_init(*p);
	mpz_init(*q);
	random_bytes = chunk_alloc(prime_size);

	do
	{
		if (!drbg->generate(drbg, random_bytes.len, random_bytes.ptr))
		{
			DBG1(DBG_LIB, "failed to allocate random prime");
			mpz_clear(*p);
			mpz_clear(*q);
			chunk_free(&random_bytes);
			return FAILED;
		}

		/* make sure the two most significant bits are set */
		if (safe)
		{
			random_bytes.ptr[0] &= 0x7F;
			random_bytes.ptr[0] |= 0x60;
			mpz_import(*q, random_bytes.len, 1, 1, 1, 0, random_bytes.ptr);
			do
			{
				count++;
				mpz_nextprime (*q, *q);
				mpz_mul_ui(*p, *q, 2);
				mpz_add_ui(*p, *p, 1);
			}
			while (mpz_probab_prime_p(*p, 10) == 0);
			DBG2(DBG_LIB, "safe prime found after %d iterations", count);
		}
		else
		{
			random_bytes.ptr[0] |= 0xC0;
			mpz_import(*p, random_bytes.len, 1, 1, 1, 0, random_bytes.ptr);
			mpz_nextprime (*p, *p);
		}
	}
	/* check if the prime isn't too large */
	while (((mpz_sizeinbase(*p, 2) + 7) / 8) > prime_size);

	chunk_clear(&random_bytes);

	/* additionally return p-1 */
	mpz_sub_ui(*q, *p, 1);

	return SUCCESS;
}

/**
 * PKCS#1 RSADP function
 */
static chunk_t rsadp(private_gmp_rsa_private_key_t *this, chunk_t data)
{
	mpz_t t1, t2;
	chunk_t decrypted;

	mpz_init(t1);
	mpz_init(t2);

	mpz_import(t1, data.len, 1, 1, 1, 0, data.ptr);

	mpz_powm(t2, t1, this->exp1, this->p);	/* m1 = c^dP mod p */
	mpz_powm(t1, t1, this->exp2, this->q);	/* m2 = c^dQ mod Q */
	mpz_sub(t2, t2, t1);					/* h = qInv (m1 - m2) mod p */
	mpz_mod(t2, t2, this->p);
	mpz_mul(t2, t2, this->coeff);
	mpz_mod(t2, t2, this->p);

	mpz_mul(t2, t2, this->q);				/* m = m2 + h q */
	mpz_add(t1, t1, t2);

	decrypted.len = this->k;
	decrypted.ptr = mpz_export(NULL, NULL, 1, decrypted.len, 1, 0, t1);
	if (decrypted.ptr == NULL)
	{
		decrypted.len = 0;
	}

	mpz_clear_sensitive(t1);
	mpz_clear_sensitive(t2);

	return decrypted;
}

/**
 * PKCS#1 RSASP1 function
 */
static chunk_t rsasp1(private_gmp_rsa_private_key_t *this, chunk_t data)
{
	return rsadp(this, data);
}

/**
 * Hashes the data and builds the plaintext signature value with EMSA
 * PKCS#1 v1.5 padding.
 *
 * Allocates the signature data.
 */
bool gmp_emsa_pkcs1_signature_data(hash_algorithm_t hash_algorithm,
								   chunk_t data, size_t keylen, chunk_t *em)
{
	chunk_t digestInfo = chunk_empty;

	if (hash_algorithm != HASH_UNKNOWN)
	{
		hasher_t *hasher;
		chunk_t hash;
		int hash_oid = hasher_algorithm_to_oid(hash_algorithm);

		if (hash_oid == OID_UNKNOWN)
		{
			return FALSE;
		}

		hasher = lib->crypto->create_hasher(lib->crypto, hash_algorithm);
		if (!hasher || !hasher->allocate_hash(hasher, data, &hash))
		{
			DESTROY_IF(hasher);
			return FALSE;
		}
		hasher->destroy(hasher);

		/* build DER-encoded digestInfo */
		digestInfo = asn1_wrap(ASN1_SEQUENCE, "mm",
						asn1_algorithmIdentifier(hash_oid),
						asn1_wrap(ASN1_OCTET_STRING, "m", hash));

		data = digestInfo;
	}

	if (keylen < 11 || data.len > keylen - 11)
	{
		chunk_free(&digestInfo);
		DBG1(DBG_LIB, "signature value of %zu bytes is too long for key of "
			 "%zu bytes", data.len, keylen);
		return FALSE;
	}

	/* EM = 0x00 || 0x01 || PS || 0x00 || T.
	 * PS = 0xFF padding, with length to fill em (at least 8 bytes)
	 * T = encoded_hash
	 */
	*em = chunk_alloc(keylen);

	/* fill em with padding */
	memset(em->ptr, 0xFF, em->len);
	/* set magic bytes */
	*(em->ptr) = 0x00;
	*(em->ptr+1) = 0x01;
	*(em->ptr + em->len - data.len - 1) = 0x00;
	/* set encoded hash */
	memcpy(em->ptr + em->len - data.len, data.ptr, data.len);

	chunk_clear(&digestInfo);
	return TRUE;
}

/**
 * Build a signature using the PKCS#1 EMSA scheme
 */
static bool build_emsa_pkcs1_signature(private_gmp_rsa_private_key_t *this,
									   hash_algorithm_t hash_algorithm,
									   chunk_t data, chunk_t *signature)
{
	chunk_t em;

	if (!gmp_emsa_pkcs1_signature_data(hash_algorithm, data, this->k, &em))
	{
		return FALSE;
	}

	/* build signature */
	*signature = rsasp1(this, em);

	chunk_free(&em);
	return TRUE;
}

/**
 * Build a signature using the PKCS#1 EMSA PSS scheme
 */
static bool build_emsa_pss_signature(private_gmp_rsa_private_key_t *this,
									 rsa_pss_params_t *params, chunk_t data,
									 chunk_t *signature)
{
	ext_out_function_t xof;
	hasher_t *hasher = NULL;
	rng_t *rng = NULL;
	xof_t *mgf = NULL;
	chunk_t hash, salt = chunk_empty, m, ps, db, dbmask, em;
	size_t embits, emlen, maskbits;
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
	/* emBits = modBits - 1 */
	embits = mpz_sizeinbase(this->n, 2) - 1;
	/* emLen = ceil(emBits/8) */
	emlen = (embits + 7) / BITS_PER_BYTE;
	/* mHash = Hash(M) */
	hasher = lib->crypto->create_hasher(lib->crypto, params->hash);
	if (!hasher)
	{
		DBG1(DBG_LIB, "hash algorithm %N not supported",
			 hash_algorithm_names, params->hash);
		return FALSE;
	}
	hash = chunk_alloca(hasher->get_hash_size(hasher));
	if (!hasher->get_hash(hasher, data, hash.ptr))
	{
		goto error;
	}

	salt.len = params->salt_len;
	if (params->salt.len)
	{
		salt = params->salt;
	}
	if (emlen < (hash.len + salt.len + 2))
	{	/* too long */
		goto error;
	}
	if (salt.len && !params->salt.len)
	{
		salt = chunk_alloca(salt.len);
		rng = lib->crypto->create_rng(lib->crypto, RNG_STRONG);
		if (!rng || !rng->get_bytes(rng, salt.len, salt.ptr))
		{
			goto error;
		}
	}
	/* M' = 0x0000000000000000 | mHash | salt */
	m = chunk_cata("ccc",
				   chunk_from_chars(0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00),
				   hash, salt);
	/* H = Hash(M') */
	if (!hasher->get_hash(hasher, m, hash.ptr))
	{
		goto error;
	}
	/* PS = 00...<padding depending on hash and salt length> */
	ps = chunk_alloca(emlen - salt.len - hash.len - 2);
	memset(ps.ptr, 0, ps.len);
	/* DB = PS | 0x01 | salt */
	db = chunk_cata("ccc", ps, chunk_from_chars(0x01), salt);
	/* dbMask = MGF(H, emLen - hLen - 1) */
	mgf = lib->crypto->create_xof(lib->crypto, xof);
	dbmask = chunk_alloca(db.len);
	if (!mgf)
	{
		DBG1(DBG_LIB, "%N not supported", ext_out_function_names, xof);
		goto error;
	}
	if (!mgf->set_seed(mgf, hash) ||
		!mgf->get_bytes(mgf, dbmask.len, dbmask.ptr))
	{
		goto error;
	}
	/* maskedDB = DB xor dbMask */
	memxor(db.ptr, dbmask.ptr, db.len);
	/* zero out unused bits */
	maskbits = (8 * emlen) - embits;
	if (maskbits)
	{
		db.ptr[0] &= (0xff >> maskbits);
	}
	/* EM = maskedDB | H | 0xbc */
	em = chunk_cata("ccc", db, hash, chunk_from_chars(0xbc));
	/* S = RSASP1(K, EM) */
	*signature = rsasp1(this, em);
	success = TRUE;

error:
	DESTROY_IF(hasher);
	DESTROY_IF(rng);
	DESTROY_IF(mgf);
	return success;
}

METHOD(private_key_t, get_type, key_type_t,
	private_gmp_rsa_private_key_t *this)
{
	return KEY_RSA;
}

METHOD(private_key_t, sign, bool,
	private_gmp_rsa_private_key_t *this, signature_scheme_t scheme,
	void *params, chunk_t data, chunk_t *signature)
{
	switch (scheme)
	{
		case SIGN_RSA_EMSA_PKCS1_NULL:
			return build_emsa_pkcs1_signature(this, HASH_UNKNOWN, data, signature);
		case SIGN_RSA_EMSA_PKCS1_SHA2_224:
			return build_emsa_pkcs1_signature(this, HASH_SHA224, data, signature);
		case SIGN_RSA_EMSA_PKCS1_SHA2_256:
			return build_emsa_pkcs1_signature(this, HASH_SHA256, data, signature);
		case SIGN_RSA_EMSA_PKCS1_SHA2_384:
			return build_emsa_pkcs1_signature(this, HASH_SHA384, data, signature);
		case SIGN_RSA_EMSA_PKCS1_SHA2_512:
			return build_emsa_pkcs1_signature(this, HASH_SHA512, data, signature);
		case SIGN_RSA_EMSA_PKCS1_SHA3_224:
			return build_emsa_pkcs1_signature(this, HASH_SHA3_224, data, signature);
		case SIGN_RSA_EMSA_PKCS1_SHA3_256:
			return build_emsa_pkcs1_signature(this, HASH_SHA3_256, data, signature);
		case SIGN_RSA_EMSA_PKCS1_SHA3_384:
			return build_emsa_pkcs1_signature(this, HASH_SHA3_384, data, signature);
		case SIGN_RSA_EMSA_PKCS1_SHA3_512:
			return build_emsa_pkcs1_signature(this, HASH_SHA3_512, data, signature);
		case SIGN_RSA_EMSA_PKCS1_SHA1:
			return build_emsa_pkcs1_signature(this, HASH_SHA1, data, signature);
		case SIGN_RSA_EMSA_PKCS1_MD5:
			return build_emsa_pkcs1_signature(this, HASH_MD5, data, signature);
		case SIGN_RSA_EMSA_PSS:
			return build_emsa_pss_signature(this, params, data, signature);
		default:
			DBG1(DBG_LIB, "signature scheme %N not supported in RSA",
				 signature_scheme_names, scheme);
			return FALSE;
	}
}

METHOD(private_key_t, decrypt, bool,
	private_gmp_rsa_private_key_t *this, encryption_scheme_t scheme,
	void *params, chunk_t crypto, chunk_t *plain)
{
	chunk_t em, stripped;
	bool success = FALSE;

	if (scheme != ENCRYPT_RSA_PKCS1)
	{
		DBG1(DBG_LIB, "encryption scheme %N not supported",
			 encryption_scheme_names, scheme);
		return FALSE;
	}
	/* rsa decryption using PKCS#1 RSADP */
	stripped = em = rsadp(this, crypto);

	/* PKCS#1 v1.5 8.1 encryption-block formatting (EB = 00 || 02 || PS || 00 || D) */

	/* check for hex pattern 00 02 in decrypted message */
	if ((*stripped.ptr++ != 0x00) || (*(stripped.ptr++) != 0x02))
	{
		DBG1(DBG_LIB, "incorrect padding - probably wrong rsa key");
		goto end;
	}
	stripped.len -= 2;

	/* the plaintext data starts after first 0x00 byte */
	while (stripped.len-- > 0 && *stripped.ptr++ != 0x00)

	if (stripped.len == 0)
	{
		DBG1(DBG_LIB, "no plaintext data");
		goto end;
	}

	*plain = chunk_clone(stripped);
	success = TRUE;

end:
	chunk_clear(&em);
	return success;
}

METHOD(private_key_t, get_keysize, int,
	private_gmp_rsa_private_key_t *this)
{
	return mpz_sizeinbase(this->n, 2);
}

METHOD(private_key_t, get_public_key, public_key_t*,
	private_gmp_rsa_private_key_t *this)
{
	chunk_t n, e;
	public_key_t *public;

	n = gmp_mpz_to_chunk(this->n);
	e = gmp_mpz_to_chunk(this->e);

	public = lib->creds->create(lib->creds, CRED_PUBLIC_KEY, KEY_RSA,
						BUILD_RSA_MODULUS, n, BUILD_RSA_PUB_EXP, e, BUILD_END);
	chunk_free(&n);
	chunk_free(&e);

	return public;
}

METHOD(private_key_t, get_encoding, bool,
	private_gmp_rsa_private_key_t *this, cred_encoding_type_t type,
	chunk_t *encoding)
{
	chunk_t n, e, d, p, q, exp1, exp2, coeff;
	bool success;

	n = gmp_mpz_to_chunk(this->n);
	e = gmp_mpz_to_chunk(this->e);
	d = gmp_mpz_to_chunk(*this->d);
	p = gmp_mpz_to_chunk(this->p);
	q = gmp_mpz_to_chunk(this->q);
	exp1 = gmp_mpz_to_chunk(this->exp1);
	exp2 = gmp_mpz_to_chunk(this->exp2);
	coeff = gmp_mpz_to_chunk(this->coeff);

	success = lib->encoding->encode(lib->encoding,
							type, NULL, encoding, CRED_PART_RSA_MODULUS, n,
							CRED_PART_RSA_PUB_EXP, e, CRED_PART_RSA_PRIV_EXP, d,
							CRED_PART_RSA_PRIME1, p, CRED_PART_RSA_PRIME2, q,
							CRED_PART_RSA_EXP1, exp1, CRED_PART_RSA_EXP2, exp2,
							CRED_PART_RSA_COEFF, coeff, CRED_PART_END);
	chunk_free(&n);
	chunk_free(&e);
	chunk_clear(&d);
	chunk_clear(&p);
	chunk_clear(&q);
	chunk_clear(&exp1);
	chunk_clear(&exp2);
	chunk_clear(&coeff);

	return success;
}

METHOD(private_key_t, get_fingerprint, bool,
	private_gmp_rsa_private_key_t *this, cred_encoding_type_t type, chunk_t *fp)
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

METHOD(private_key_t, get_ref, private_key_t*,
	private_gmp_rsa_private_key_t *this)
{
	ref_get(&this->ref);
	return &this->public.key;
}

METHOD(private_key_t, destroy, void,
	private_gmp_rsa_private_key_t *this)
{
	if (ref_put(&this->ref))
	{
		int i;

		mpz_clear(this->n);
		mpz_clear(this->e);
		mpz_clear(this->v);
		mpz_clear_sensitive(this->p);
		mpz_clear_sensitive(this->q);
		mpz_clear_sensitive(this->m);
		mpz_clear_sensitive(this->exp1);
		mpz_clear_sensitive(this->exp2);
		mpz_clear_sensitive(this->coeff);

		for (i = 0; i < this->threshold; i++)
		{
			mpz_clear_sensitive(*this->d + i);
		}
		free(this->d);

		lib->encoding->clear_cache(lib->encoding, this);
		free(this);
	}
}

/**
 * Check the loaded key if it is valid and usable
 */
static status_t check(private_gmp_rsa_private_key_t *this)
{
	mpz_t u, p1, q1;
	status_t status = SUCCESS;

	/* PKCS#1 1.5 section 6 requires modulus to have at least 12 octets.
	 * We actually require more (for security).
	 */
	if (this->k < 512 / BITS_PER_BYTE)
	{
		DBG1(DBG_LIB, "key shorter than 512 bits");
		return FAILED;
	}

	/* we picked a max modulus size to simplify buffer allocation */
	if (this->k > 8192 / BITS_PER_BYTE)
	{
		DBG1(DBG_LIB, "key larger than 8192 bits");
		return FAILED;
	}

	mpz_init(u);
	mpz_init(p1);
	mpz_init(q1);

	/* precompute p1 = p-1 and q1 = q-1 */
	mpz_sub_ui(p1, this->p, 1);
	mpz_sub_ui(q1, this->q, 1);

	/* check that n == p * q */
	mpz_mul(u, this->p, this->q);
	if (mpz_cmp(u, this->n) != 0)
	{
		status = FAILED;
	}

	/* check that e divides neither p-1 nor q-1 */
	mpz_mod(u, p1, this->e);
	if (mpz_cmp_ui(u, 0) == 0)
	{
		status = FAILED;
	}

	mpz_mod(u, q1, this->e);
	if (mpz_cmp_ui(u, 0) == 0)
	{
		status = FAILED;
	}

	/* check that d is e^-1 (mod lcm(p-1, q-1)) */
	/* see PKCS#1v2, aka RFC 2437, for the "lcm" */
	mpz_lcm(this->m, p1, q1);
	mpz_mul(u, *this->d, this->e);
	mpz_mod(u, u, this->m);
	if (mpz_cmp_ui(u, 1) != 0)
	{
		status = FAILED;
	}

	/* check that exp1 is d mod (p-1) */
	mpz_mod(u, *this->d, p1);
	if (mpz_cmp(u, this->exp1) != 0)
	{
		status = FAILED;
	}

	/* check that exp2 is d mod (q-1) */
	mpz_mod(u, *this->d, q1);
	if (mpz_cmp(u, this->exp2) != 0)
	{
		status = FAILED;
	}

	/* check that coeff is (q^-1) mod p */
	mpz_mul(u, this->coeff, this->q);
	mpz_mod(u, u, this->p);
	if (mpz_cmp_ui(u, 1) != 0)
	{
		status = FAILED;
	}

	mpz_clear_sensitive(u);
	mpz_clear_sensitive(p1);
	mpz_clear_sensitive(q1);

	if (status != SUCCESS)
	{
		DBG1(DBG_LIB, "key integrity tests failed");
	}
	return status;
}

/**
 * Internal generic constructor
 */
static private_gmp_rsa_private_key_t *gmp_rsa_private_key_create_empty(void)
{
	private_gmp_rsa_private_key_t *this;

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
		.threshold = 1,
		.ref = 1,
	);
	return this;
}

/**
 * See header.
 */
gmp_rsa_private_key_t *gmp_rsa_private_key_gen(key_type_t type, va_list args)
{
	private_gmp_rsa_private_key_t *this;
	drbg_type_t drbg_type = DRBG_HMAC_SHA512;
	drbg_t* drbg;
	rng_t *rng;
	u_int strength = 256, key_size = 0, shares = 0, threshold = 1;
	bool safe_prime = FALSE, drbg_failed = FALSE, invert_failed = FALSE;
	mpz_t p, q, p1, q1;
	int i;


	while (TRUE)
	{
		switch (va_arg(args, builder_part_t))
		{
			case BUILD_KEY_SIZE:
				key_size = va_arg(args, u_int);
				continue;
			case BUILD_SAFE_PRIMES:
				safe_prime = TRUE;
				continue;
			case BUILD_SHARES:
				shares = va_arg(args, u_int);
				continue;
			case BUILD_THRESHOLD:
				threshold = va_arg(args, u_int);
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
	key_size = key_size / BITS_PER_BYTE;

	/* Initiate a NIST SP 800-90A DRBG fed by a true rng owned by the drbg */
	rng = lib->crypto->create_rng(lib->crypto, RNG_TRUE);
	if (!rng)
	{
		DBG1(DBG_LIB, "no RNG of quality %N found", rng_quality_names, RNG_TRUE);
		return NULL;
	}
	drbg = lib->crypto->create_drbg(lib->crypto, drbg_type, strength, rng,
									chunk_empty);
	if (!drbg)
	{
		DBG1(DBG_LIB, "instantiation of %N failed", drbg_type_names, drbg_type);
		rng->destroy(rng);
		return NULL;
	}

	/* Get values of primes p and q  */
	if (compute_prime(drbg, key_size/2, safe_prime, &p, &p1) != SUCCESS)
	{
		drbg->destroy(drbg);
		return NULL;
	}
	if (compute_prime(drbg, key_size/2, safe_prime, &q, &q1) != SUCCESS)
	{
		mpz_clear(p);
		mpz_clear(p1);
		drbg->destroy(drbg);
		return NULL;
	}

	/* Swapping Primes so p is larger then q */
	if (mpz_cmp(p, q) < 0)
	{
		mpz_swap(p, q);
		mpz_swap(p1, q1);
	}

	/* Create and initialize RSA private key object */
	this = gmp_rsa_private_key_create_empty();
	*this->p = *p;
	*this->q = *q;

	/* allocate space for private exponent d with optional threshold scheme */
	this->shares = shares;
	this->threshold = threshold;
	this->d = malloc(threshold * sizeof(mpz_t));
	for (i = 0; i < threshold; i++)
	{
		mpz_init(this->d[i]);
	}

	mpz_init_set_ui(this->e, PUBLIC_EXPONENT);
	mpz_init(this->n);
	mpz_init(this->m);
	mpz_init(this->exp1);
	mpz_init(this->exp2);
	mpz_init(this->coeff);
	mpz_init(this->v);

	mpz_mul(this->n, p, q);                    /* n = p*q */
	mpz_lcm(this->m, p1, q1);                  /* m = lcm(p-1,q-1) */
	mpz_invert(this->d[0], this->e, this->m);  /* e has an inverse mod m */
	mpz_mod(this->exp1, this->d[0], p1);       /* exp1 = d mod p-1 */
	mpz_mod(this->exp2, this->d[0], q1);       /* exp2 = d mod q-1 */
	mpz_invert(this->coeff, q, p);             /* coeff = q^-1 mod p */

	invert_failed = mpz_cmp_ui(this->m, 0) == 0 ||
					mpz_cmp_ui(this->coeff, 0) == 0;

	/* generate and store random coefficients of secret sharing polynomial */
	if (threshold > 1)
	{
		chunk_t random_bytes;
		mpz_t u;

		mpz_init(u);
		random_bytes = chunk_alloc(key_size);

		for (i = 1; i < threshold; i++)
		{
			if (!drbg->generate(drbg, random_bytes.len, random_bytes.ptr))
			{
				drbg_failed = TRUE;
				continue;
			}
			mpz_import(this->d[i], random_bytes.len, 1, 1, 1, 0, random_bytes.ptr);
			mpz_mod(this->d[i], this->d[i], this->m);
		}

		/* generate verification key v as a square number */
		do
		{
			if (!drbg->generate(drbg, random_bytes.len, random_bytes.ptr))
			{
				drbg_failed = TRUE;
				break;
			}
			mpz_import(this->v, random_bytes.len, 1, 1, 1, 0, random_bytes.ptr);
			mpz_mul(this->v, this->v, this->v);
			mpz_mod(this->v, this->v, this->n);
			mpz_gcd(u, this->v, this->n);
		}
		while (mpz_cmp_ui(u, 1) != 0);

		mpz_clear(u);
		chunk_clear(&random_bytes);
	}

	mpz_clear_sensitive(p1);
	mpz_clear_sensitive(q1);
	drbg->destroy(drbg);

	if (drbg_failed || invert_failed)
	{
		DBG1(DBG_LIB, "rsa key generation failed");
		destroy(this);
		return NULL;
	}

	/* set key size in bytes */
	this->k = key_size;

	return &this->public;
}

/**
 * Recover the primes from n, e and d using the algorithm described in
 * Appendix C of NIST SP 800-56B.
 */
static bool calculate_pq(private_gmp_rsa_private_key_t *this)
{
	gmp_randstate_t rstate;
	mpz_t k, r, g, y, n1, x;
	int i, t, j;
	bool success = FALSE;

	gmp_randinit_default(rstate);
	mpz_init(k);
	mpz_init(r);
	mpz_init(g);
	mpz_init(y);
	mpz_init(n1);
	mpz_init(x);
	/* k = (d * e) - 1 */
	mpz_mul(k, *this->d, this->e);
	mpz_sub_ui(k, k, 1);
	if (mpz_odd_p(k))
	{
		goto error;
	}
	/* k = 2^t * r, where r is the largest odd integer dividing k, and t >= 1 */
	mpz_set(r, k);
	for (t = 0; !mpz_odd_p(r); t++)
	{	/* r = r/2 */
		mpz_divexact_ui(r, r, 2);
	}
	/* we need n-1 below */
	mpz_sub_ui(n1, this->n, 1);
	for (i = 0; i < 100; i++)
	{	/* generate random integer g in [0, n-1] */
		mpz_urandomm(g, rstate, this->n);
		/* y = g^r mod n */
		mpz_powm(y, g, r, this->n);
		/* try again if y == 1 or y == n-1 */
		if (mpz_cmp_ui(y, 1) == 0 || mpz_cmp(y, n1) == 0)
		{
			continue;
		}
		for (j = 0; j < t; j++)
		{	/* x = y^2 mod n */
			mpz_powm_ui(x, y, 2, this->n);
			/* stop if x == 1 */
			if (mpz_cmp_ui(x, 1) == 0)
			{
				goto done;
			}
			/* retry with new g if x = n-1 */
			if (mpz_cmp(x, n1) == 0)
			{
				break;
			}
			/* y = x */
			mpz_set(y, x);
		}
	}
	goto error;

done:
	/* p = gcd(y-1, n) */
	mpz_sub_ui(y, y, 1);
	mpz_gcd(this->p, y, this->n);
	/* q = n/p */
	mpz_divexact(this->q, this->n, this->p);
	success = TRUE;

error:
	mpz_clear_sensitive(k);
	mpz_clear_sensitive(r);
	mpz_clear_sensitive(g);
	mpz_clear_sensitive(y);
	mpz_clear_sensitive(x);
	mpz_clear(n1);
	gmp_randclear(rstate);
	return success;
}

/**
 * See header.
 */
gmp_rsa_private_key_t *gmp_rsa_private_key_load(key_type_t type, va_list args)
{
	private_gmp_rsa_private_key_t *this;
	chunk_t n, e, d, p, q, exp1, exp2, coeff;

	n = e = d = p = q = exp1 = exp2 = coeff = chunk_empty;
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

	this = gmp_rsa_private_key_create_empty();

	this->d = malloc(sizeof(mpz_t));
	mpz_init(this->n);
	mpz_init(this->e);
	mpz_init(*this->d);
	mpz_init(this->p);
	mpz_init(this->q);
	mpz_init(this->m);
	mpz_init(this->exp1);
	mpz_init(this->exp2);
	mpz_init(this->coeff);
	mpz_init(this->v);

	mpz_import(this->n, n.len, 1, 1, 1, 0, n.ptr);
	mpz_import(this->e, e.len, 1, 1, 1, 0, e.ptr);
	mpz_import(*this->d, d.len, 1, 1, 1, 0, d.ptr);
	if (p.len)
	{
		mpz_import(this->p, p.len, 1, 1, 1, 0, p.ptr);
	}
	if (q.len)
	{
		mpz_import(this->q, q.len, 1, 1, 1, 0, q.ptr);
	}
	if (!p.len && !q.len)
	{	/* p and q missing in key, recalculate from n, e and d */
		if (!calculate_pq(this))
		{
			destroy(this);
			return NULL;
		}
	}
	else if (!p.len)
	{	/* p missing in key, recalculate: p = n / q */
		mpz_divexact(this->p, this->n, this->q);
	}
	else if (!q.len)
	{	/* q missing in key, recalculate: q = n / p */
		mpz_divexact(this->q, this->n, this->p);
	}
	if (!exp1.len)
	{	/* exp1 missing in key, recalculate: exp1 = d mod (p-1) */
		mpz_sub_ui(this->exp1, this->p, 1);
		mpz_mod(this->exp1, *this->d, this->exp1);
	}
	else
	{
		mpz_import(this->exp1, exp1.len, 1, 1, 1, 0, exp1.ptr);
	}
	if (!exp2.len)
	{	/* exp2 missing in key, recalculate: exp2 = d mod (q-1) */
		mpz_sub_ui(this->exp2, this->q, 1);
		mpz_mod(this->exp2, *this->d, this->exp2);
	}
	else
	{
		mpz_import(this->exp2, exp2.len, 1, 1, 1, 0, exp2.ptr);
	}
	if (!coeff.len)
	{	/* coeff missing in key, recalculate: coeff = q^-1 mod p */
		mpz_invert(this->coeff, this->q, this->p);
	}
	else
	{
		mpz_import(this->coeff, coeff.len, 1, 1, 1, 0, coeff.ptr);
	}
	this->k = (mpz_sizeinbase(this->n, 2) + 7) / BITS_PER_BYTE;
	if (check(this) != SUCCESS)
	{
		destroy(this);
		return NULL;
	}
	return &this->public;
}
