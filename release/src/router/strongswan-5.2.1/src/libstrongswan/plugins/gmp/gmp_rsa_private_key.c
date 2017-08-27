/*
 * Copyright (C) 2005 Jan Hutter
 * Copyright (C) 2005-2009 Martin Willi
 * Copyright (C) 2012 Andreas Steffen
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
#include <string.h>

#include "gmp_rsa_private_key.h"
#include "gmp_rsa_public_key.h"

#include <utils/debug.h>
#include <asn1/oid.h>
#include <asn1/asn1.h>
#include <asn1/asn1_parser.h>

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
	u_int8_t *zeros = alloca(len);

	memset(zeros, 0, len);
	/* overwrite mpz_t with zero bytes before clearing it */
	mpz_import(z, len, 1, 1, 1, 0, zeros);
	mpz_clear(z);
}

/**
 * Create a mpz prime of at least prime_size
 */
static status_t compute_prime(size_t prime_size, bool safe, mpz_t *p, mpz_t *q)
{
	rng_t *rng;
	chunk_t random_bytes;
	int count = 0;

	rng = lib->crypto->create_rng(lib->crypto, RNG_TRUE);
	if (!rng)
	{
		DBG1(DBG_LIB, "no RNG of quality %N found", rng_quality_names,
			 RNG_TRUE);
		return FAILED;
	}

	mpz_init(*p);
	mpz_init(*q);

	do
	{
		if (!rng->allocate_bytes(rng, prime_size, &random_bytes))
		{
			DBG1(DBG_LIB, "failed to allocate random prime");
			mpz_clear(*p);
			mpz_clear(*q);
			rng->destroy(rng);
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
		chunk_clear(&random_bytes);
	}

	/* check if the prime isn't too large */
	while (((mpz_sizeinbase(*p, 2) + 7) / 8) > prime_size);

	rng->destroy(rng);

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
 * Build a signature using the PKCS#1 EMSA scheme
 */
static bool build_emsa_pkcs1_signature(private_gmp_rsa_private_key_t *this,
									   hash_algorithm_t hash_algorithm,
									   chunk_t data, chunk_t *signature)
{
	chunk_t digestInfo = chunk_empty;
	chunk_t em;

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
						asn1_simple_object(ASN1_OCTET_STRING, hash)
					  );
		chunk_free(&hash);
		data = digestInfo;
	}

	if (data.len > this->k - 3)
	{
		free(digestInfo.ptr);
		DBG1(DBG_LIB, "unable to sign %d bytes using a %dbit key", data.len,
			 mpz_sizeinbase(this->n, 2));
		return FALSE;
	}

	/* build chunk to rsa-decrypt:
	 * EM = 0x00 || 0x01 || PS || 0x00 || T.
	 * PS = 0xFF padding, with length to fill em
	 * T = encoded_hash
	 */
	em.len = this->k;
	em.ptr = malloc(em.len);

	/* fill em with padding */
	memset(em.ptr, 0xFF, em.len);
	/* set magic bytes */
	*(em.ptr) = 0x00;
	*(em.ptr+1) = 0x01;
	*(em.ptr + em.len - data.len - 1) = 0x00;
	/* set DER-encoded hash */
	memcpy(em.ptr + em.len - data.len, data.ptr, data.len);

	/* build signature */
	*signature = rsasp1(this, em);

	free(digestInfo.ptr);
	free(em.ptr);

	return TRUE;
}

METHOD(private_key_t, get_type, key_type_t,
	private_gmp_rsa_private_key_t *this)
{
	return KEY_RSA;
}

METHOD(private_key_t, sign, bool,
	private_gmp_rsa_private_key_t *this, signature_scheme_t scheme,
	chunk_t data, chunk_t *signature)
{
	switch (scheme)
	{
		case SIGN_RSA_EMSA_PKCS1_NULL:
			return build_emsa_pkcs1_signature(this, HASH_UNKNOWN, data, signature);
		case SIGN_RSA_EMSA_PKCS1_SHA1:
			return build_emsa_pkcs1_signature(this, HASH_SHA1, data, signature);
		case SIGN_RSA_EMSA_PKCS1_SHA224:
			return build_emsa_pkcs1_signature(this, HASH_SHA224, data, signature);
		case SIGN_RSA_EMSA_PKCS1_SHA256:
			return build_emsa_pkcs1_signature(this, HASH_SHA256, data, signature);
		case SIGN_RSA_EMSA_PKCS1_SHA384:
			return build_emsa_pkcs1_signature(this, HASH_SHA384, data, signature);
		case SIGN_RSA_EMSA_PKCS1_SHA512:
			return build_emsa_pkcs1_signature(this, HASH_SHA512, data, signature);
		case SIGN_RSA_EMSA_PKCS1_MD5:
			return build_emsa_pkcs1_signature(this, HASH_MD5, data, signature);
		default:
			DBG1(DBG_LIB, "signature scheme %N not supported in RSA",
				 signature_scheme_names, scheme);
			return FALSE;
	}
}

METHOD(private_key_t, decrypt, bool,
	private_gmp_rsa_private_key_t *this, encryption_scheme_t scheme,
	chunk_t crypto, chunk_t *plain)
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
	u_int key_size = 0, shares = 0, threshold = 1;
	bool safe_prime = FALSE, rng_failed = FALSE, invert_failed = FALSE;
	mpz_t p, q, p1, q1, d;
;

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

	/* Get values of primes p and q  */
	if (compute_prime(key_size/2, safe_prime, &p, &p1) != SUCCESS)
	{
		return NULL;
	}
	if (compute_prime(key_size/2, safe_prime, &q, &q1) != SUCCESS)
	{
		mpz_clear(p);
		mpz_clear(p1);
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
	this->shares = shares;
	this->threshold = threshold;
	this->d = malloc(threshold * sizeof(mpz_t));
	*this->p = *p;
	*this->q = *q;

	mpz_init_set_ui(this->e, PUBLIC_EXPONENT);
	mpz_init(this->n);
	mpz_init(this->m);
	mpz_init(this->exp1);
	mpz_init(this->exp2);
	mpz_init(this->coeff);
	mpz_init(this->v);
	mpz_init(d);

	mpz_mul(this->n, p, q);					/* n = p*q */
	mpz_lcm(this->m, p1, q1);				/* m = lcm(p-1,q-1) */
	mpz_invert(d, this->e, this->m);		/* e has an inverse mod m */
	mpz_mod(this->exp1, d, p1);				/* exp1 = d mod p-1 */
	mpz_mod(this->exp2, d, q1);				/* exp2 = d mod q-1 */
	mpz_invert(this->coeff, q, p);			/* coeff = q^-1 mod p */

	invert_failed = mpz_cmp_ui(this->m, 0) == 0 ||
					mpz_cmp_ui(this->coeff, 0) == 0;

    /* store secret exponent d */
	(*this->d)[0] = *d;

	/* generate and store random coefficients of secret sharing polynomial */
	if (threshold > 1)
	{
		rng_t *rng;
		chunk_t random_bytes;
		mpz_t u;
		int i;

		rng = lib->crypto->create_rng(lib->crypto, RNG_TRUE);
		mpz_init(u);

		for (i = 1; i < threshold; i++)
		{
			mpz_init(d);

			if (!rng->allocate_bytes(rng, key_size, &random_bytes))
			{
				rng_failed = TRUE;
				continue;
			}
			mpz_import(d, random_bytes.len, 1, 1, 1, 0, random_bytes.ptr);
			mpz_mod(d, d, this->m);
			(*this->d)[i] = *d;
			chunk_clear(&random_bytes);
		}

		/* generate verification key v as a square number */
		do
		{
			if (!rng->allocate_bytes(rng, key_size, &random_bytes))
			{
				rng_failed = TRUE;
				break;
			}
			mpz_import(this->v, random_bytes.len, 1, 1, 1, 0, random_bytes.ptr);
			mpz_mul(this->v, this->v, this->v);
			mpz_mod(this->v, this->v, this->n);
			mpz_gcd(u, this->v, this->n);
			chunk_free(&random_bytes);
		}
		while (mpz_cmp_ui(u, 1) != 0);

		mpz_clear(u);
		rng->destroy(rng);
	}

	mpz_clear_sensitive(p1);
	mpz_clear_sensitive(q1);

	if (rng_failed || invert_failed)
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
	mpz_import(this->p, p.len, 1, 1, 1, 0, p.ptr);
	mpz_import(this->q, q.len, 1, 1, 1, 0, q.ptr);
	mpz_import(this->coeff, coeff.len, 1, 1, 1, 0, coeff.ptr);
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
	this->k = (mpz_sizeinbase(this->n, 2) + 7) / BITS_PER_BYTE;
	if (check(this) != SUCCESS)
	{
		destroy(this);
		return NULL;
	}
	return &this->public;
}

