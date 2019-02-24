/*
 * Copyright (C) 2018 Tobias Brunner
 * HSR Hochschule fuer Technik Rapperswil
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

#include "botan_util_keys.h"
#include "botan_ec_public_key.h"
#include "botan_ec_private_key.h"
#include "botan_ed_public_key.h"
#include "botan_ed_private_key.h"
#include "botan_rsa_public_key.h"
#include "botan_rsa_private_key.h"

#include <asn1/asn1.h>
#include <asn1/oid.h>

/**
 * Get the algorithm name of a public key
 */
static char *get_algo_name(botan_pubkey_t pubkey)
{
	char *name;
	size_t len = 0;

	if (botan_pubkey_algo_name(pubkey, NULL, &len)
		!= BOTAN_FFI_ERROR_INSUFFICIENT_BUFFER_SPACE)
	{
		return NULL;
	}

	name = malloc(len);
	if (botan_pubkey_algo_name(pubkey, name, &len))
	{
		free(name);
		return NULL;
	}
	return name;
}

/*
 * Described in header
 */
public_key_t *botan_public_key_load(key_type_t type, va_list args)
{
	public_key_t *this = NULL;
	botan_pubkey_t pubkey;
	chunk_t blob = chunk_empty;
	botan_rng_t rng;
	char *name;

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

	if (botan_rng_init(&rng, "user"))
	{
		return NULL;
	}
	if (botan_pubkey_load(&pubkey, blob.ptr, blob.len))
	{
		botan_rng_destroy(rng);
		return NULL;
	}
	if (botan_pubkey_check_key(pubkey, rng, BOTAN_CHECK_KEY_EXPENSIVE_TESTS))
	{
		DBG1(DBG_LIB, "public key failed key checks");
		botan_pubkey_destroy(pubkey);
		botan_rng_destroy(rng);
		return NULL;
	}
	botan_rng_destroy(rng);

	name = get_algo_name(pubkey);
	if (!name)
	{
		botan_pubkey_destroy(pubkey);
		return NULL;
	}

#ifdef BOTAN_HAS_RSA
	if (streq(name, "RSA") && (type == KEY_ANY || type == KEY_RSA))
	{
		this = (public_key_t*)botan_rsa_public_key_adopt(pubkey);
	}
	else
#endif
#ifdef BOTAN_HAS_ECDSA
	if (streq(name, "ECDSA") && (type == KEY_ANY || type == KEY_ECDSA))
	{
		this = (public_key_t*)botan_ec_public_key_adopt(pubkey);
	}
	else
#endif
#ifdef BOTAN_HAS_ED25519
	if (streq(name, "Ed25519") && (type == KEY_ANY || type == KEY_ED25519))
	{
		this = botan_ed_public_key_adopt(pubkey);
	}
	else
#endif
	{
		botan_pubkey_destroy(pubkey);
	}
	free(name);
	return this;
}

#ifdef BOTAN_HAS_ECDSA
/**
 * Determine the curve OID from a PKCS#8 structure
 */
static int determine_ec_oid(chunk_t pkcs8)
{
	int oid = OID_UNKNOWN;
	chunk_t inner, params = chunk_empty;

	if (asn1_unwrap(&pkcs8, &pkcs8) == ASN1_SEQUENCE &&
		asn1_unwrap(&pkcs8, &inner) == ASN1_INTEGER &&
		asn1_parse_integer_uint64(inner) == 0 &&
		asn1_parse_algorithmIdentifier(pkcs8, 0, &params) == OID_EC_PUBLICKEY &&
		params.len &&
		asn1_unwrap(&params, &params) == ASN1_OID)
	{
		oid = asn1_known_oid(params);
	}
	return oid;
}
#endif

/*
 * Described in header
 */
private_key_t *botan_private_key_load(key_type_t type, va_list args)
{
	private_key_t *this = NULL;
	botan_privkey_t key;
	botan_pubkey_t pubkey;
	chunk_t blob = chunk_empty;
	botan_rng_t rng;
	char *name;

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

	if (botan_rng_init(&rng, "user"))
	{
		return NULL;
	}
	if (botan_privkey_load(&key, rng, blob.ptr, blob.len, NULL))
	{
		botan_rng_destroy(rng);
		return NULL;
	}
	botan_rng_destroy(rng);

	if (botan_privkey_export_pubkey(&pubkey, key))
	{
		botan_privkey_destroy(key);
		return NULL;
	}
	name = get_algo_name(pubkey);
	botan_pubkey_destroy(pubkey);
	if (!name)
	{
		botan_privkey_destroy(key);
		return NULL;
	}

#ifdef BOTAN_HAS_RSA
	if (streq(name, "RSA") && (type == KEY_ANY || type == KEY_RSA))
	{
		this = (private_key_t*)botan_rsa_private_key_adopt(key);
	}
	else
#endif
#ifdef BOTAN_HAS_ECDSA
	if (streq(name, "ECDSA") && (type == KEY_ANY || type == KEY_ECDSA))
	{
		int oid = determine_ec_oid(blob);
		if (oid != OID_UNKNOWN)
		{
			this = (private_key_t*)botan_ec_private_key_adopt(key, oid);
		}
	}
	else
#endif
#ifdef BOTAN_HAS_ED25519
	if (streq(name, "Ed25519") && (type == KEY_ANY || type == KEY_ED25519))
	{
		this = botan_ed_private_key_adopt(key);
	}
#endif

	if (!this)
	{
		botan_privkey_destroy(key);
	}
	free(name);
	return this;
}
