/*
 * Copyright (C) 2018 Tobias Brunner
 * Copyright (C) 2018 Andreas Steffen
 * HSR Hochschule fuer Technik Rapperswil
 *
 * Copyright (C) 2018 René Korthaus
 * Rohde & Schwarz Cybersecurity GmbH
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "botan_rsa_private_key.h"
#include "botan_rsa_public_key.h"

#include <botan/build.h>

#ifdef BOTAN_HAS_RSA

#include "botan_util.h"

#include <botan/ffi.h>

#include <utils/debug.h>

typedef struct private_botan_rsa_private_key_t private_botan_rsa_private_key_t;

/**
 * Private data of a botan_rsa_private_key_t object.
 */
struct private_botan_rsa_private_key_t {

	/**
	 * Public interface for this signer.
	 */
	botan_rsa_private_key_t public;

	/**
	 * Botan private key
	 */
	botan_privkey_t key;

	/**
	 * reference count
	 */
	refcount_t ref;
};

/**
 * Get the Botan string identifier for an EMSA PSS signature
 */
bool botan_emsa_pss_identifier(rsa_pss_params_t *params, char *id, size_t len)
{
	const char *hash;

	if (!params)
	{
		return FALSE;
	}

	/* botan currently does not support passing the mgf1 hash */
	if (params->hash != params->mgf1_hash)
	{
		DBG1(DBG_LIB, "passing mgf1 hash not supported via botan");
		return FALSE;
	}

	hash = botan_get_hash(params->hash);
	if (!hash)
	{
		return FALSE;
	}
	return snprintf(id, len, "EMSA-PSS(%s,MGF1,%zd)", hash,
					params->salt_len) < len;
}

/**
 * Build an EMSA PSS signature described in PKCS#1
 */
static bool build_emsa_pss_signature(private_botan_rsa_private_key_t *this,
									 rsa_pss_params_t *params, chunk_t data,
									 chunk_t *sig)
{
	char hash_and_padding[BUF_LEN];

	if (!botan_emsa_pss_identifier(params, hash_and_padding,
								   sizeof(hash_and_padding)))
	{
		return FALSE;
	}
	return botan_get_signature(this->key, hash_and_padding, data, sig);
}

METHOD(private_key_t, get_type, key_type_t,
	private_botan_rsa_private_key_t *this)
{
	return KEY_RSA;
}

METHOD(private_key_t, sign, bool,
	private_botan_rsa_private_key_t *this, signature_scheme_t scheme,
	void *params, chunk_t data, chunk_t *signature)
{
	switch (scheme)
	{
		case SIGN_RSA_EMSA_PKCS1_NULL:
			return botan_get_signature(this->key, "EMSA_PKCS1(Raw)", data,
									   signature);
		case SIGN_RSA_EMSA_PKCS1_SHA1:
			return botan_get_signature(this->key, "EMSA_PKCS1(SHA-1)", data,
									   signature);
		case SIGN_RSA_EMSA_PKCS1_SHA2_224:
			return botan_get_signature(this->key, "EMSA_PKCS1(SHA-224)", data,
									   signature);
		case SIGN_RSA_EMSA_PKCS1_SHA2_256:
			return botan_get_signature(this->key, "EMSA_PKCS1(SHA-256)", data,
									   signature);
		case SIGN_RSA_EMSA_PKCS1_SHA2_384:
			return botan_get_signature(this->key, "EMSA_PKCS1(SHA-384)", data,
									   signature);
		case SIGN_RSA_EMSA_PKCS1_SHA2_512:
			return botan_get_signature(this->key, "EMSA_PKCS1(SHA-512)", data,
									   signature);
		case SIGN_RSA_EMSA_PKCS1_SHA3_224:
			return botan_get_signature(this->key, "EMSA_PKCS1(SHA-3(224))", data,
									   signature);
		case SIGN_RSA_EMSA_PKCS1_SHA3_256:
			return botan_get_signature(this->key, "EMSA_PKCS1(SHA-3(256))", data,
									   signature);
		case SIGN_RSA_EMSA_PKCS1_SHA3_384:
			return botan_get_signature(this->key, "EMSA_PKCS1(SHA-3(384))", data,
									   signature);
		case SIGN_RSA_EMSA_PKCS1_SHA3_512:
			return botan_get_signature(this->key, "EMSA_PKCS1(SHA-3(512))", data,
									   signature);
		case SIGN_RSA_EMSA_PSS:
			return build_emsa_pss_signature(this, params, data, signature);
		default:
			DBG1(DBG_LIB, "signature scheme %N not supported via botan",
				 signature_scheme_names, scheme);
			return FALSE;
	}
}

METHOD(private_key_t, decrypt, bool,
	private_botan_rsa_private_key_t *this, encryption_scheme_t scheme,
	chunk_t crypto, chunk_t *plain)
{
	botan_pk_op_decrypt_t decrypt_op;
	const char *padding;

	switch (scheme)
	{
		case ENCRYPT_RSA_PKCS1:
			padding = "PKCS1v15";
			break;
		case ENCRYPT_RSA_OAEP_SHA1:
			padding = "OAEP(SHA-1)";
			break;
		case ENCRYPT_RSA_OAEP_SHA224:
			padding = "OAEP(SHA-224)";
			break;
		case ENCRYPT_RSA_OAEP_SHA256:
			padding = "OAEP(SHA-256)";
			break;
		case ENCRYPT_RSA_OAEP_SHA384:
			padding = "OAEP(SHA-384)";
			break;
		case ENCRYPT_RSA_OAEP_SHA512:
			padding = "OAEP(SHA-512)";
			break;
		default:
			DBG1(DBG_LIB, "encryption scheme %N not supported via botan",
				 encryption_scheme_names, scheme);
			return FALSE;
	}

	if (botan_pk_op_decrypt_create(&decrypt_op, this->key, padding, 0))
	{
		return FALSE;
	}

	plain->len = 0;
	if (botan_pk_op_decrypt_output_length(decrypt_op, crypto.len, &plain->len))
	{
		botan_pk_op_decrypt_destroy(decrypt_op);
		return FALSE;
	}

	*plain = chunk_alloc(plain->len);
	if (botan_pk_op_decrypt(decrypt_op, plain->ptr, &plain->len, crypto.ptr,
							crypto.len))
	{
		chunk_free(plain);
		botan_pk_op_decrypt_destroy(decrypt_op);
		return FALSE;
	}
	botan_pk_op_decrypt_destroy(decrypt_op);
	return TRUE;
}

METHOD(private_key_t, get_keysize, int,
	private_botan_rsa_private_key_t *this)
{
	botan_mp_t n;
	size_t bits = 0;

	if (botan_mp_init(&n))
	{
		return 0;
	}

	if (botan_privkey_rsa_get_n(n, this->key) ||
		botan_mp_num_bits(n, &bits))
	{
		botan_mp_destroy(n);
		return 0;
	}

	botan_mp_destroy(n);
	return bits;
}

METHOD(private_key_t, get_public_key, public_key_t*,
	private_botan_rsa_private_key_t *this)
{
	botan_pubkey_t pubkey;

	if (botan_privkey_export_pubkey(&pubkey, this->key))
	{
		return NULL;
	}
	return (public_key_t*)botan_rsa_public_key_adopt(pubkey);
}

METHOD(private_key_t, get_fingerprint, bool,
	private_botan_rsa_private_key_t *this, cred_encoding_type_t type,
	chunk_t *fingerprint)
{
	botan_pubkey_t pubkey;
	bool success = FALSE;

	/* check the cache before doing the export */
	if (lib->encoding->get_cache(lib->encoding, type, this, fingerprint))
	{
		return TRUE;
	}

	if (botan_privkey_export_pubkey(&pubkey, this->key))
	{
		return FALSE;
	}
	success = botan_get_fingerprint(pubkey, this, type, fingerprint);
	botan_pubkey_destroy(pubkey);
	return success;
}

METHOD(private_key_t, get_encoding, bool,
	private_botan_rsa_private_key_t *this, cred_encoding_type_t type,
	chunk_t *encoding)
{
	return botan_get_privkey_encoding(this->key, type, encoding);
}

METHOD(private_key_t, get_ref, private_key_t*,
	private_botan_rsa_private_key_t *this)
{
	ref_get(&this->ref);
	return &this->public.key;
}

METHOD(private_key_t, destroy, void,
	private_botan_rsa_private_key_t *this)
{
	if (ref_put(&this->ref))
	{
		lib->encoding->clear_cache(lib->encoding, this);
		botan_privkey_destroy(this->key);
		free(this);
	}
}

/**
 * Internal generic constructor
 */
static private_botan_rsa_private_key_t *create_empty()
{
	private_botan_rsa_private_key_t *this;

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
	);

	return this;
}

/*
 * Described in header
 */
botan_rsa_private_key_t *botan_rsa_private_key_adopt(botan_privkey_t key)
{
	private_botan_rsa_private_key_t *this;

	this = create_empty();
	this->key = key;

	return &this->public;
}

/*
 * Described in header
 */
botan_rsa_private_key_t *botan_rsa_private_key_gen(key_type_t type,
												   va_list args)
{
	private_botan_rsa_private_key_t *this;
	botan_rng_t rng;
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

	if (botan_rng_init(&rng, "system"))
	{
		return NULL;
	}

	this = create_empty();

	if (botan_privkey_create_rsa(&this->key, rng, key_size))
	{
		botan_rng_destroy(rng);
		free(this);
		return NULL;
	}
	botan_rng_destroy(rng);
	return &this->public;
}

/**
 * Recover the primes from n, e and d using the algorithm described in
 * Appendix C of NIST SP 800-56B.
 */
static bool calculate_pq(botan_mp_t *n, botan_mp_t *e, botan_mp_t *d,
						 botan_mp_t *p, botan_mp_t *q)
{
	botan_mp_t k = NULL, one = NULL, r = NULL, zero = NULL, two = NULL;
	botan_mp_t n1 = NULL, x = NULL, y = NULL, g = NULL, rem = NULL;
	botan_rng_t rng = NULL;
	int i, t, j;
	bool success = FALSE;

	if (botan_mp_init(&k) ||
		botan_mp_init(&one) ||
		botan_mp_set_from_int(one, 1))
	{
		goto error;
	}

	/* 1. k = d * e - 1 */
	if (botan_mp_mul(k, *d, *e) || botan_mp_sub(k, k, one))
	{
		goto error;
	}

	/* k must be even */
	if (!botan_mp_is_even(k))
	{
		goto error;
	}

	/* 2. k = 2^t * r, where r is the largest odd integer dividing k, and t >= 1 */
	if (botan_mp_init(&r) ||
		botan_mp_set_from_mp(r, k))
	{
		goto error;
	}

	for (t = 0; !botan_mp_is_odd(r); t++)
	{
		if (botan_mp_rshift(r, r, 1))
		{
			goto error;
		}
	}

	/* need 0 and n-1 below */
	if (botan_mp_init(&zero) ||
		botan_mp_init(&n1) ||
		botan_mp_sub(n1, *n, one))
	{
		goto error;
	}

	if (botan_mp_init(&g))
	{
		goto error;
	}

	if (botan_rng_init(&rng, "user"))
	{
		goto error;
	}

	if (botan_mp_init(&two))
	{
		goto error;
	}

	if (botan_mp_set_from_int(two, 2))
	{
		goto error;
	}

	if (botan_mp_init(&y) ||
		botan_mp_init(&x))
	{
		goto error;
	}

	for (i = 0; i < 100; i++)
	{
		/* 3a. generate a random integer g in the range [0, n-1] */
		if (botan_mp_rand_range(g, rng, zero, n1))
		{
			goto error;
		}
		/* 3b. y = g^r mod n */
		if (botan_mp_powmod(y, g, r, *n))
		{
			goto error;
		}

		/* 3c. If y = 1 or y = n – 1, try again */
		if (botan_mp_equal(y, one) || botan_mp_equal(y, n1))
		{
			continue;
		}

		for (j = 0; j < t; j++)
		{
			/* x = y^2 mod n */
			if (botan_mp_powmod(x, y, two, *n))
			{
				goto error;
			}

			/* stop if x == 1 */
			if (botan_mp_equal(x, one))
			{
				goto done;
			}

			/* retry with new g if x = n-1 */
			if (botan_mp_equal(x, n1))
			{
				break;
			}

			/* let y = x */
			if (botan_mp_set_from_mp(y, x))
			{
				goto error;
			}
		}
	}

done:
	/* 5. p = GCD(y – 1, n) and q = n/p */
	if (botan_mp_sub(y, y, one))
	{
		goto error;
	}

	if (botan_mp_init(p) ||
		botan_mp_gcd(*p, y, *n))
	{
		goto error;
	}

	if (botan_mp_init(q) ||
		botan_mp_init(&rem) ||
		botan_mp_div(*q, rem, *n, *p))
	{
		goto error;
	}

	if (!botan_mp_is_zero(rem))
	{
		goto error;
	}

	success = TRUE;

error:
	if (!success)
	{
		botan_mp_destroy(*p);
		botan_mp_destroy(*q);
	}
	botan_rng_destroy(rng);
	botan_mp_destroy(k);
	botan_mp_destroy(one);
	botan_mp_destroy(r);
	botan_mp_destroy(zero);
	botan_mp_destroy(two);
	botan_mp_destroy(n1);
	botan_mp_destroy(x);
	botan_mp_destroy(y);
	botan_mp_destroy(g);
	botan_mp_destroy(rem);
	return success;
}

/*
 * Described in header
 */
botan_rsa_private_key_t *botan_rsa_private_key_load(key_type_t type,
													va_list args)
{
	private_botan_rsa_private_key_t *this;
	chunk_t n, e, d, p, q, blob;

	n = e = d = p = q = blob = chunk_empty;
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
			case BUILD_RSA_EXP2:
			case BUILD_RSA_COEFF:
				/* not required for botan */
				va_arg(args, chunk_t);
				continue;
			case BUILD_END:
				break;
			default:
				return NULL;
		}
		break;
	}

	if (type == KEY_ANY && !blob.ptr)
	{
		return NULL;
	}

	if (blob.ptr)
	{
		this = create_empty();

		if (botan_privkey_load_rsa_pkcs1(&this->key, blob.ptr, blob.len))
		{
			free(this);
			return NULL;
		}
		return &this->public;
	}

	if (n.ptr && e.ptr && d.ptr)
	{
		botan_mp_t n_mp, e_mp, d_mp, p_mp = NULL, q_mp = NULL;

		if (!chunk_to_botan_mp(n, &n_mp))
		{
			return NULL;
		}

		if (!chunk_to_botan_mp(e, &e_mp))
		{
			botan_mp_destroy(n_mp);
			return NULL;
		}

		if (!chunk_to_botan_mp(d, &d_mp))
		{
			botan_mp_destroy(n_mp);
			botan_mp_destroy(e_mp);
			return NULL;
		}

		if (p.ptr && q.ptr)
		{
			if (!chunk_to_botan_mp(p, &p_mp))
			{
				botan_mp_destroy(n_mp);
				botan_mp_destroy(e_mp);
				botan_mp_destroy(d_mp);
				return NULL;
			}

			if (!chunk_to_botan_mp(q, &q_mp))
			{
				botan_mp_destroy(n_mp);
				botan_mp_destroy(e_mp);
				botan_mp_destroy(d_mp);
				botan_mp_destroy(p_mp);
				return NULL;
			}
		}
		else
		{
			/* calculate p,q from n, e, d */
			if (!calculate_pq(&n_mp, &e_mp, &d_mp, &p_mp, &q_mp))
			{
				botan_mp_destroy(n_mp);
				botan_mp_destroy(e_mp);
				botan_mp_destroy(d_mp);
				return NULL;
			}
		}
		botan_mp_destroy(n_mp);
		botan_mp_destroy(d_mp);

		this = create_empty();

		if (botan_privkey_load_rsa(&this->key, p_mp, q_mp, e_mp))
		{
			botan_mp_destroy(e_mp);
			botan_mp_destroy(p_mp);
			botan_mp_destroy(q_mp);
			free(this);
			return NULL;
		}

		botan_mp_destroy(e_mp);
		botan_mp_destroy(p_mp);
		botan_mp_destroy(q_mp);

		return &this->public;
	}

	return NULL;
}

#endif
