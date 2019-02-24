/*
 * Copyright (C) 2015-2017 Tobias Brunner
 * Copyright (C) 2014-2016 Andreas Steffen
 * Copyright (C) 2007 Martin Willi
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

#include <asn1/oid.h>

#include "public_key.h"
#include "signature_params.h"

ENUM(key_type_names, KEY_ANY, KEY_BLISS,
	"ANY",
	"RSA",
	"ECDSA",
	"DSA",
	"ED25519",
	"ED448",
	"BLISS"
);

ENUM(signature_scheme_names, SIGN_UNKNOWN, SIGN_BLISS_WITH_SHA3_512,
	"UNKNOWN",
	"RSA_EMSA_PKCS1_NULL",
	"RSA_EMSA_PKCS1_MD5",
	"RSA_EMSA_PKCS1_SHA1",
	"RSA_EMSA_PKCS1_SHA2_224",
	"RSA_EMSA_PKCS1_SHA2_256",
	"RSA_EMSA_PKCS1_SHA2_384",
	"RSA_EMSA_PKCS1_SHA2_512",
	"RSA_EMSA_PKCS1_SHA3_224",
	"RSA_EMSA_PKCS1_SHA3_256",
	"RSA_EMSA_PKCS1_SHA3_384",
	"RSA_EMSA_PKCS1_SHA3_512",
	"RSA_EMSA_PSS",
	"ECDSA_WITH_SHA1_DER",
	"ECDSA_WITH_SHA256_DER",
	"ECDSA_WITH_SHA384_DER",
	"ECDSA_WITH_SHA512_DER",
	"ECDSA_WITH_NULL",
	"ECDSA-256",
	"ECDSA-384",
	"ECDSA-521",
	"ED25519",
	"ED448",
	"BLISS_WITH_SHA2_256",
	"BLISS_WITH_SHA2_384",
	"BLISS_WITH_SHA2_512",
	"BLISS_WITH_SHA3_256",
	"BLISS_WITH_SHA3_384",
	"BLISS_WITH_SHA3_512",
);

ENUM(encryption_scheme_names, ENCRYPT_UNKNOWN, ENCRYPT_RSA_OAEP_SHA512,
	"ENCRYPT_UNKNOWN",
	"ENCRYPT_RSA_PKCS1",
	"ENCRYPT_RSA_OAEP_SHA1",
	"ENCRYPT_RSA_OAEP_SHA224",
	"ENCRYPT_RSA_OAEP_SHA256",
	"ENCRYPT_RSA_OAEP_SHA384",
	"ENCRYPT_RSA_OAEP_SHA512",
);

/**
 * See header.
 */
bool public_key_equals(public_key_t *this, public_key_t *other)
{
	cred_encoding_type_t type;
	chunk_t a, b;

	if (this == other)
	{
		return TRUE;
	}

	for (type = 0; type < CRED_ENCODING_MAX; type++)
	{
		if (this->get_fingerprint(this, type, &a) &&
			other->get_fingerprint(other, type, &b))
		{
			return chunk_equals(a, b);
		}
	}
	return FALSE;
}

/**
 * See header.
 */
bool public_key_has_fingerprint(public_key_t *public, chunk_t fingerprint)
{
	cred_encoding_type_t type;
	chunk_t current;

	for (type = 0; type < KEYID_MAX; type++)
	{
		if (public->get_fingerprint(public, type, &current) &&
			chunk_equals(current, fingerprint))
		{
			return TRUE;
		}
	}
	return FALSE;
}

/*
 * Defined in header.
 */
signature_scheme_t signature_scheme_from_oid(int oid)
{
	switch (oid)
	{
		case OID_MD5_WITH_RSA:
		case OID_MD5:
			return SIGN_RSA_EMSA_PKCS1_MD5;
		case OID_SHA1_WITH_RSA:
		case OID_SHA1:
			return SIGN_RSA_EMSA_PKCS1_SHA1;
		case OID_SHA224_WITH_RSA:
		case OID_SHA224:
			return SIGN_RSA_EMSA_PKCS1_SHA2_224;
		case OID_SHA256_WITH_RSA:
		case OID_SHA256:
			return SIGN_RSA_EMSA_PKCS1_SHA2_256;
		case OID_SHA384_WITH_RSA:
		case OID_SHA384:
			return SIGN_RSA_EMSA_PKCS1_SHA2_384;
		case OID_SHA512_WITH_RSA:
		case OID_SHA512:
			return SIGN_RSA_EMSA_PKCS1_SHA2_512;
		case OID_RSASSA_PKCS1V15_WITH_SHA3_224:
			return SIGN_RSA_EMSA_PKCS1_SHA3_224;
		case OID_RSASSA_PKCS1V15_WITH_SHA3_256:
			return SIGN_RSA_EMSA_PKCS1_SHA3_256;
		case OID_RSASSA_PKCS1V15_WITH_SHA3_384:
			return SIGN_RSA_EMSA_PKCS1_SHA3_384;
		case OID_RSASSA_PKCS1V15_WITH_SHA3_512:
			return SIGN_RSA_EMSA_PKCS1_SHA3_512;
		case OID_RSASSA_PSS:
			return SIGN_RSA_EMSA_PSS;
		case OID_ECDSA_WITH_SHA1:
		case OID_EC_PUBLICKEY:
			return SIGN_ECDSA_WITH_SHA1_DER;
		case OID_ECDSA_WITH_SHA256:
			return SIGN_ECDSA_WITH_SHA256_DER;
		case OID_ECDSA_WITH_SHA384:
			return SIGN_ECDSA_WITH_SHA384_DER;
		case OID_ECDSA_WITH_SHA512:
			return SIGN_ECDSA_WITH_SHA512_DER;
		case OID_ED25519:
			return SIGN_ED25519;
		case OID_ED448:
			return SIGN_ED448;
		case OID_BLISS_PUBLICKEY:
		case OID_BLISS_WITH_SHA2_512:
			return SIGN_BLISS_WITH_SHA2_512;
		case OID_BLISS_WITH_SHA2_384:
			return SIGN_BLISS_WITH_SHA2_384;
		case OID_BLISS_WITH_SHA2_256:
			return SIGN_BLISS_WITH_SHA2_256;
		case OID_BLISS_WITH_SHA3_512:
			return SIGN_BLISS_WITH_SHA3_512;
		case OID_BLISS_WITH_SHA3_384:
			return SIGN_BLISS_WITH_SHA3_384;
		case OID_BLISS_WITH_SHA3_256:
			return SIGN_BLISS_WITH_SHA3_256;
	}
	return SIGN_UNKNOWN;
}

/*
 * Defined in header.
 */
int signature_scheme_to_oid(signature_scheme_t scheme)
{
	switch (scheme)
	{
		case SIGN_UNKNOWN:
		case SIGN_RSA_EMSA_PKCS1_NULL:
		case SIGN_ECDSA_WITH_NULL:
		case SIGN_ECDSA_256:
		case SIGN_ECDSA_384:
		case SIGN_ECDSA_521:
			break;
		case SIGN_RSA_EMSA_PKCS1_MD5:
			return OID_MD5_WITH_RSA;
		case SIGN_RSA_EMSA_PKCS1_SHA1:
			return OID_SHA1_WITH_RSA;
		case SIGN_RSA_EMSA_PKCS1_SHA2_224:
			return OID_SHA224_WITH_RSA;
		case SIGN_RSA_EMSA_PKCS1_SHA2_256:
			return OID_SHA256_WITH_RSA;
		case SIGN_RSA_EMSA_PKCS1_SHA2_384:
			return OID_SHA384_WITH_RSA;
		case SIGN_RSA_EMSA_PKCS1_SHA2_512:
			return OID_SHA512_WITH_RSA;
		case SIGN_RSA_EMSA_PKCS1_SHA3_224:
			return OID_RSASSA_PKCS1V15_WITH_SHA3_224;
		case SIGN_RSA_EMSA_PKCS1_SHA3_256:
			return OID_RSASSA_PKCS1V15_WITH_SHA3_256;
		case SIGN_RSA_EMSA_PKCS1_SHA3_384:
			return OID_RSASSA_PKCS1V15_WITH_SHA3_384;
		case SIGN_RSA_EMSA_PKCS1_SHA3_512:
			return OID_RSASSA_PKCS1V15_WITH_SHA3_384;
		case SIGN_RSA_EMSA_PSS:
			return OID_RSASSA_PSS;
		case SIGN_ECDSA_WITH_SHA1_DER:
			return OID_ECDSA_WITH_SHA1;
		case SIGN_ECDSA_WITH_SHA256_DER:
			return OID_ECDSA_WITH_SHA256;
		case SIGN_ECDSA_WITH_SHA384_DER:
			return OID_ECDSA_WITH_SHA384;
		case SIGN_ECDSA_WITH_SHA512_DER:
			return OID_ECDSA_WITH_SHA512;
		case SIGN_ED25519:
			return OID_ED25519;
		case SIGN_ED448:
			return OID_ED448;
		case SIGN_BLISS_WITH_SHA2_256:
			return OID_BLISS_WITH_SHA2_256;
		case SIGN_BLISS_WITH_SHA2_384:
			return OID_BLISS_WITH_SHA2_384;
		case SIGN_BLISS_WITH_SHA2_512:
			return OID_BLISS_WITH_SHA2_512;
		case SIGN_BLISS_WITH_SHA3_256:
			return OID_BLISS_WITH_SHA3_256;
		case SIGN_BLISS_WITH_SHA3_384:
			return OID_BLISS_WITH_SHA3_384;
		case SIGN_BLISS_WITH_SHA3_512:
			return OID_BLISS_WITH_SHA3_512;
	}
	return OID_UNKNOWN;
}

/**
 * Parameters for RSA/PSS signature schemes
 */
#define PSS_PARAMS(bits) static rsa_pss_params_t pss_params_sha##bits = { \
	.hash = HASH_SHA##bits, \
	.mgf1_hash = HASH_SHA##bits, \
	.salt_len = HASH_SIZE_SHA##bits, \
}

PSS_PARAMS(256);
PSS_PARAMS(384);
PSS_PARAMS(512);

/**
 * Map for signature schemes to the key type and maximum key size allowed.
 * We only cover schemes with hash algorithms supported by IKEv2 signature
 * authentication.
 */
static struct {
	key_type_t type;
	int max_keysize;
	signature_params_t params;
} scheme_map[] = {
	{ KEY_RSA,  3072, { .scheme = SIGN_RSA_EMSA_PSS, .params = &pss_params_sha256, }},
	{ KEY_RSA,  7680, { .scheme = SIGN_RSA_EMSA_PSS, .params = &pss_params_sha384, }},
	{ KEY_RSA,     0, { .scheme = SIGN_RSA_EMSA_PSS, .params = &pss_params_sha512, }},
	{ KEY_RSA,  3072, { .scheme = SIGN_RSA_EMSA_PKCS1_SHA2_256 }},
	{ KEY_RSA,  7680, { .scheme = SIGN_RSA_EMSA_PKCS1_SHA2_384 }},
	{ KEY_RSA,     0, { .scheme = SIGN_RSA_EMSA_PKCS1_SHA2_512 }},
	{ KEY_ECDSA, 256, { .scheme = SIGN_ECDSA_WITH_SHA256_DER }},
	{ KEY_ECDSA, 384, { .scheme = SIGN_ECDSA_WITH_SHA384_DER }},
	{ KEY_ECDSA,   0, { .scheme = SIGN_ECDSA_WITH_SHA512_DER }},
	{ KEY_ED25519, 0, { .scheme = SIGN_ED25519 }},
	{ KEY_ED448,   0, { .scheme = SIGN_ED448 }},
	{ KEY_BLISS, 128, { .scheme = SIGN_BLISS_WITH_SHA2_256 }},
	{ KEY_BLISS, 192, { .scheme = SIGN_BLISS_WITH_SHA2_384 }},
	{ KEY_BLISS,   0, { .scheme = SIGN_BLISS_WITH_SHA2_512 }},
};

/**
 * Private data for signature scheme enumerator
 */
typedef struct  {
	enumerator_t public;
	int index;
	key_type_t type;
	int size;
} private_enumerator_t;

METHOD(enumerator_t, signature_schemes_enumerate, bool,
	private_enumerator_t *this, va_list args)
{
	signature_params_t **params;

	VA_ARGS_VGET(args, params);

	while (++this->index < countof(scheme_map))
	{
		if (this->type == scheme_map[this->index].type &&
		   (this->size <= scheme_map[this->index].max_keysize ||
			!scheme_map[this->index].max_keysize))
		{
			*params = &scheme_map[this->index].params;
			return TRUE;
		}
	}
	return FALSE;
}

/*
 * Defined in header.
 */
enumerator_t *signature_schemes_for_key(key_type_t type, int size)
{
	private_enumerator_t *this;

	INIT(this,
		.public = {
			.enumerate = enumerator_enumerate_default,
			.venumerate = _signature_schemes_enumerate,
			.destroy = (void*)free,
		},
		.index = -1,
		.type = type,
		.size = size,
	);

	return &this->public;
}

/*
 * Defined in header.
 */
key_type_t key_type_from_signature_scheme(signature_scheme_t scheme)
{
	switch (scheme)
	{
		case SIGN_UNKNOWN:
			break;
		case SIGN_RSA_EMSA_PKCS1_NULL:
		case SIGN_RSA_EMSA_PKCS1_MD5:
		case SIGN_RSA_EMSA_PKCS1_SHA1:
		case SIGN_RSA_EMSA_PKCS1_SHA2_224:
		case SIGN_RSA_EMSA_PKCS1_SHA2_256:
		case SIGN_RSA_EMSA_PKCS1_SHA2_384:
		case SIGN_RSA_EMSA_PKCS1_SHA2_512:
		case SIGN_RSA_EMSA_PKCS1_SHA3_224:
		case SIGN_RSA_EMSA_PKCS1_SHA3_256:
		case SIGN_RSA_EMSA_PKCS1_SHA3_384:
		case SIGN_RSA_EMSA_PKCS1_SHA3_512:
		case SIGN_RSA_EMSA_PSS:
			return KEY_RSA;
		case SIGN_ECDSA_WITH_SHA1_DER:
		case SIGN_ECDSA_WITH_SHA256_DER:
		case SIGN_ECDSA_WITH_SHA384_DER:
		case SIGN_ECDSA_WITH_SHA512_DER:
		case SIGN_ECDSA_WITH_NULL:
		case SIGN_ECDSA_256:
		case SIGN_ECDSA_384:
		case SIGN_ECDSA_521:
			return KEY_ECDSA;
		case SIGN_ED25519:
			return KEY_ED25519;
		case SIGN_ED448:
			return KEY_ED448;
		case SIGN_BLISS_WITH_SHA2_256:
		case SIGN_BLISS_WITH_SHA2_384:
		case SIGN_BLISS_WITH_SHA2_512:
		case SIGN_BLISS_WITH_SHA3_256:
		case SIGN_BLISS_WITH_SHA3_384:
		case SIGN_BLISS_WITH_SHA3_512:
			return KEY_BLISS;
	}
	return KEY_ANY;
}
