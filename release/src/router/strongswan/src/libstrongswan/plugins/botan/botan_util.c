/*
 * Copyright (C) 2018-2023 Tobias Brunner
 * Copyright (C) 2018 Andreas Steffen
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

#include "botan_util.h"

#include <utils/debug.h>

#include <botan/ffi.h>

/*
 * Described in header
 */
bool chunk_to_botan_mp(chunk_t value, botan_mp_t *mp)
{
	if (botan_mp_init(mp))
	{
		return FALSE;
	}

	if (botan_mp_from_bin(*mp, value.ptr, value.len))
	{
		botan_mp_destroy(*mp);
		return FALSE;
	}
	return TRUE;
}

/*
 * Described in header
 */
const char *botan_get_hash(hash_algorithm_t hash)
{
	switch (hash)
	{
		case HASH_MD5:
			return "MD5";
		case HASH_SHA1:
			return "SHA-1";
		case HASH_SHA224:
			return "SHA-224";
		case HASH_SHA256:
			return "SHA-256";
		case HASH_SHA384:
			return "SHA-384";
		case HASH_SHA512:
			return "SHA-512";
		case HASH_SHA3_224:
			return "SHA-3(224)";
		case HASH_SHA3_256:
			return "SHA-3(256)";
		case HASH_SHA3_384:
			return "SHA-3(384)";
		case HASH_SHA3_512:
			return "SHA-3(512)";
		default:
			return NULL;
	}
}

/**
 * Encode the given RSA public key parameter as chunk.
 */
static bool encode_rsa_field(botan_pubkey_t pubkey, const char *name,
							 chunk_t *chunk)
{
	botan_mp_t val = NULL;
	size_t len = 0;

	if (botan_mp_init(&val) ||
		botan_pubkey_get_field(val, pubkey, name) ||
		botan_mp_num_bytes(val, &len) || !len)
	{
		botan_mp_destroy(val);
		return FALSE;
	}

	*chunk = chunk_alloc(len);
	if (botan_mp_to_bin(val, chunk->ptr))
	{
		botan_mp_destroy(val);
		chunk_free(chunk);
		return FALSE;
	}
	botan_mp_destroy(val);
	return TRUE;
}

/*
 * Described in header
 */
bool botan_get_encoding(botan_pubkey_t pubkey, cred_encoding_type_t type,
						chunk_t *encoding)
{
	chunk_t asn1_encoding, n = chunk_empty, e = chunk_empty;
	cred_encoding_part_t part = CRED_PART_END;
	char algo[8];
	size_t len = sizeof(algo);
	bool success = FALSE;

	encoding->len = 0;
	if (botan_pubkey_export(pubkey, NULL, &encoding->len,
							BOTAN_PRIVKEY_EXPORT_FLAG_DER)
		!= BOTAN_FFI_ERROR_INSUFFICIENT_BUFFER_SPACE)
	{
		return FALSE;
	}

	*encoding = chunk_alloc(encoding->len);
	if (botan_pubkey_export(pubkey, encoding->ptr, &encoding->len,
							BOTAN_PRIVKEY_EXPORT_FLAG_DER))
	{
		chunk_free(encoding);
		return FALSE;
	}

	if (type == PUBKEY_SPKI_ASN1_DER)
	{
		return TRUE;
	}

	asn1_encoding = *encoding;
	if (botan_pubkey_algo_name(pubkey, algo, &len))
	{
		chunk_free(&asn1_encoding);
		return FALSE;
	}

	if (streq(algo, "RSA") &&
		encode_rsa_field(pubkey, "n", &n) &&
		encode_rsa_field(pubkey, "e", &e))
	{
		success = lib->encoding->encode(lib->encoding, type, NULL, encoding,
									CRED_PART_RSA_PUB_ASN1_DER, asn1_encoding,
									CRED_PART_RSA_MODULUS, n,
									CRED_PART_RSA_PUB_EXP, e, CRED_PART_END);
	}
	else
	{
		if (streq(algo, "ECDSA"))
		{
			part = CRED_PART_ECDSA_PUB_ASN1_DER;
		}
		else if (streq(algo, "Ed25519"))
		{
			part = CRED_PART_EDDSA_PUB_ASN1_DER;
		}
		success = lib->encoding->encode(lib->encoding, type, NULL, encoding,
										part, asn1_encoding, CRED_PART_END);
	}
	chunk_free(&asn1_encoding);
	chunk_free(&n);
	chunk_free(&e);
	return success;
}

/*
 * Described in header
 */
bool botan_get_privkey_encoding(botan_privkey_t key, cred_encoding_type_t type,
								chunk_t *encoding)
{
	uint32_t format = BOTAN_PRIVKEY_EXPORT_FLAG_DER;

	switch (type)
	{
		case PRIVKEY_PEM:
			format = BOTAN_PRIVKEY_EXPORT_FLAG_PEM;
			/* fall-through */
		case PRIVKEY_ASN1_DER:
			encoding->len = 0;
			if (botan_privkey_export(key, NULL, &encoding->len, format)
				!= BOTAN_FFI_ERROR_INSUFFICIENT_BUFFER_SPACE)
			{
				return FALSE;
			}
			*encoding = chunk_alloc(encoding->len);
			if (botan_privkey_export(key, encoding->ptr, &encoding->len,
									 format))
			{
				chunk_free(encoding);
				return FALSE;
			}
			return TRUE;
		default:
			return FALSE;
	}
}

/*
 * Described in header
 */
bool botan_get_fingerprint(botan_pubkey_t pubkey, void *cache,
						   cred_encoding_type_t type, chunk_t *fp)
{
	hasher_t *hasher;
	chunk_t key;

	if (cache &&
		lib->encoding->get_cache(lib->encoding, type, cache, fp))
	{
		return TRUE;
	}

	switch (type)
	{
		case KEYID_PUBKEY_SHA1:
			/* subjectPublicKey -> use botan_pubkey_fingerprint() */
			*fp = chunk_alloc(HASH_SIZE_SHA1);
			if (botan_pubkey_fingerprint(pubkey, "SHA-1", fp->ptr, &fp->len))
			{
				chunk_free(fp);
				return FALSE;
			}
			break;
		case KEYID_PUBKEY_INFO_SHA1:
			/* subjectPublicKeyInfo -> use botan_pubkey_export(), then hash */
			if (!botan_get_encoding(pubkey, PUBKEY_SPKI_ASN1_DER, &key))
			{
				return FALSE;
			}

			hasher = lib->crypto->create_hasher(lib->crypto, HASH_SHA1);
			if (!hasher || !hasher->allocate_hash(hasher, key, fp))
			{
				DBG1(DBG_LIB, "SHA1 hash algorithm not supported, "
					 "fingerprinting failed");
				DESTROY_IF(hasher);
				chunk_free(&key);
				return FALSE;
			}
			hasher->destroy(hasher);
			chunk_free(&key);
			break;
		default:
			return FALSE;
	}

	if (cache)
	{
		lib->encoding->cache(lib->encoding, type, cache, fp);
	}
	return TRUE;
}

/*
 * Described in header
 */
bool botan_get_signature(botan_privkey_t key, const char *scheme,
						 chunk_t data, chunk_t *signature)
{
	botan_pk_op_sign_t sign_op;
	botan_rng_t rng;

	if (!scheme || !signature)
	{
		return FALSE;
	}

	if (botan_pk_op_sign_create(&sign_op, key, scheme, 0))
	{
		return FALSE;
	}

	if (botan_pk_op_sign_update(sign_op, data.ptr, data.len))
	{
		botan_pk_op_sign_destroy(sign_op);
		return FALSE;
	}

	signature->len = 0;
	if (botan_pk_op_sign_output_length(sign_op, &signature->len))
	{
		botan_pk_op_sign_destroy(sign_op);
		return FALSE;
	}

	if (!botan_get_rng(&rng, RNG_STRONG))
	{
		botan_pk_op_sign_destroy(sign_op);
		return FALSE;
	}

	*signature = chunk_alloc(signature->len);
	if (botan_pk_op_sign_finish(sign_op, rng, signature->ptr, &signature->len))
	{
		chunk_free(signature);
		botan_rng_destroy(rng);
		botan_pk_op_sign_destroy(sign_op);
		return FALSE;
	}

	botan_rng_destroy(rng);
	botan_pk_op_sign_destroy(sign_op);
	return TRUE;
}

/*
 * Described in header
 */
bool botan_verify_signature(botan_pubkey_t key, const char *scheme,
							chunk_t data, chunk_t signature)
{
	botan_pk_op_verify_t verify_op;
	bool valid = FALSE;

	if (botan_pk_op_verify_create(&verify_op, key, scheme, 0))
	{
		return FALSE;
	}

	if (botan_pk_op_verify_update(verify_op, data.ptr, data.len))
	{
		botan_pk_op_verify_destroy(verify_op);
		return FALSE;
	}

	valid =	!botan_pk_op_verify_finish(verify_op, signature.ptr, signature.len);

	botan_pk_op_verify_destroy(verify_op);
	return valid;
}

/*
 * Described in header
 */
bool botan_dh_key_derivation(botan_privkey_t key, chunk_t pub, chunk_t *secret)
{
	botan_pk_op_ka_t ka;

	if (botan_pk_op_key_agreement_create(&ka, key, "Raw", 0))
	{
		return FALSE;
	}

	if (botan_pk_op_key_agreement_size(ka, &secret->len))
	{
		botan_pk_op_key_agreement_destroy(ka);
		return FALSE;
	}

	*secret = chunk_alloc(secret->len);
	if (botan_pk_op_key_agreement(ka, secret->ptr, &secret->len, pub.ptr,
								  pub.len, NULL, 0))
	{
		chunk_clear(secret);
		botan_pk_op_key_agreement_destroy(ka);
		return FALSE;
	}
	botan_pk_op_key_agreement_destroy(ka);
	return TRUE;
}

/*
 * Described in header
 */
const char *botan_map_rng_quality(rng_quality_t quality)
{
	const char *rng_name;

	switch (quality)
	{
		case RNG_WEAK:
		case RNG_STRONG:
			/* some rng_t instances of this class (e.g. in the ike-sa-manager)
			 * may be called concurrently by different threads. the Botan RNGs
			 * are not reentrant, by default, so use the threadsafe version.
			 * because we build without threading support when running tests
			 * with leak-detective (lots of reports of frees of unknown memory)
			 * there is a fallback to the default */
#ifdef BOTAN_TARGET_OS_HAS_THREADS
			rng_name = "user-threadsafe";
#else
			rng_name = "user";
#endif
			break;
		case RNG_TRUE:
			rng_name = "system";
			break;
		default:
			return NULL;
	}
	return rng_name;
}

#ifdef HAVE_BOTAN_RNG_INIT_CUSTOM

CALLBACK(get_random, int,
	rng_t *rng, uint8_t *out, size_t out_len)
{
	if (!rng->get_bytes(rng, out_len, out))
	{
		return -1;
	}
	return 0;
}

CALLBACK(destroy_rng, void,
	rng_t *rng)
{
	if (rng)
	{
		rng->destroy(rng);
	}
}

#endif /* HAVE_BOTAN_RNG_INIT_CUSTOM */

/*
 * Described in header
 */
bool botan_get_rng(botan_rng_t *botan_rng, rng_quality_t quality)
{
#ifdef HAVE_BOTAN_RNG_INIT_CUSTOM
	if (!lib->settings->get_bool(lib->settings,
						"%s.plugins.botan.internal_rng_only", FALSE, lib->ns))
	{
		rng_t *rng = lib->crypto->create_rng(lib->crypto, quality);

		if (!rng)
		{
			DBG1(DBG_LIB, "no RNG found for quality %N", rng_quality_names,
				 quality);
			return FALSE;
		}
		if (botan_rng_init_custom(botan_rng, "strongswan", rng,
								  get_random, NULL, destroy_rng))
		{
			DBG1(DBG_LIB, "Botan RNG creation failed");
			return FALSE;
		}
	}
	else
#endif /* HAVE_BOTAN_RNG_INIT_CUSTOM */
	{
		const char *rng_name = botan_map_rng_quality(quality);

		if (!rng_name || botan_rng_init(botan_rng, rng_name))
		{
			return FALSE;
		}
	}
	return TRUE;
}
