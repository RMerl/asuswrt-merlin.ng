/*
 * Copyright (C) 2007 Martin Willi
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

#include <asn1/oid.h>

#include "public_key.h"

ENUM(key_type_names, KEY_ANY, KEY_DSA,
	"ANY",
	"RSA",
	"ECDSA",
	"DSA"
);

ENUM(signature_scheme_names, SIGN_UNKNOWN, SIGN_ECDSA_521,
	"UNKNOWN",
	"RSA_EMSA_PKCS1_NULL",
	"RSA_EMSA_PKCS1_MD5",
	"RSA_EMSA_PKCS1_SHA1",
	"RSA_EMSA_PKCS1_SHA224",
	"RSA_EMSA_PKCS1_SHA256",
	"RSA_EMSA_PKCS1_SHA384",
	"RSA_EMSA_PKCS1_SHA512",
	"ECDSA_WITH_SHA1_DER",
	"ECDSA_WITH_SHA256_DER",
	"ECDSA_WITH_SHA384_DER",
	"ECDSA_WITH_SHA512_DER",
	"ECDSA_WITH_NULL",
	"ECDSA-256",
	"ECDSA-384",
	"ECDSA-521",
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
			return SIGN_RSA_EMSA_PKCS1_SHA224;
		case OID_SHA256_WITH_RSA:
		case OID_SHA256:
			return SIGN_RSA_EMSA_PKCS1_SHA256;
		case OID_SHA384_WITH_RSA:
		case OID_SHA384:
			return SIGN_RSA_EMSA_PKCS1_SHA384;
		case OID_SHA512_WITH_RSA:
		case OID_SHA512:
			return SIGN_RSA_EMSA_PKCS1_SHA512;
		case OID_ECDSA_WITH_SHA1:
		case OID_EC_PUBLICKEY:
			return SIGN_ECDSA_WITH_SHA1_DER;
		case OID_ECDSA_WITH_SHA256:
			return SIGN_ECDSA_WITH_SHA256_DER;
		case OID_ECDSA_WITH_SHA384:
			return SIGN_ECDSA_WITH_SHA384_DER;
		case OID_ECDSA_WITH_SHA512:
			return SIGN_ECDSA_WITH_SHA512_DER;
		default:
			return SIGN_UNKNOWN;
	}
}

