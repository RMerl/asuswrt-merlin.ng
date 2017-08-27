/*
 * Copyright (C) 2009 Martin Willi
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

#include "pkcs1_encoder.h"

#include <utils/debug.h>
#include <asn1/asn1.h>
#include <asn1/oid.h>

/**
 * Encode a public key in PKCS#1/ASN.1 DER
 */
static bool build_pub(chunk_t *encoding, va_list args)
{
	chunk_t n, e;

	if (cred_encoding_args(args, CRED_PART_RSA_MODULUS, &n,
						   CRED_PART_RSA_PUB_EXP, &e, CRED_PART_END))
	{
		*encoding = asn1_wrap(ASN1_SEQUENCE, "mm",
						asn1_integer("c", n),
						asn1_integer("c", e));
		return TRUE;
	}
	return FALSE;
}

/**
 * Encode a public key in PKCS#1/ASN.1 DER, contained in subjectPublicKeyInfo
 */
static bool build_pub_info(chunk_t *encoding, va_list args)
{
	chunk_t n, e;

	if (cred_encoding_args(args, CRED_PART_RSA_MODULUS, &n,
						   CRED_PART_RSA_PUB_EXP, &e, CRED_PART_END))
	{
		*encoding = asn1_wrap(ASN1_SEQUENCE, "mm",
						asn1_algorithmIdentifier(OID_RSA_ENCRYPTION),
						asn1_bitstring("m",
							asn1_wrap(ASN1_SEQUENCE, "mm",
								asn1_integer("c", n),
								asn1_integer("c", e))));
		return TRUE;
	}
	return FALSE;
}

/**
 * Encode the RSA modulus of a public key only
 */
static bool build_pub_modulus(chunk_t *encoding, va_list args)
{
	chunk_t n;

	if (cred_encoding_args(args, CRED_PART_RSA_MODULUS, &n, CRED_PART_END))
	{
		/* remove preceding zero bytes */
		while (n.len > 0 && *n.ptr == 0x00)
		{
			n.ptr++;
			n.len--;
		}
		*encoding = chunk_clone(n);
		return TRUE;
	}
	return FALSE;
}

/**
 * Encode a private key in PKCS#1/ASN.1 DER
 */
static bool build_priv(chunk_t *encoding, va_list args)
{
	chunk_t n, e, d, p, q, exp1, exp2, coeff;

	if (cred_encoding_args(args, CRED_PART_RSA_MODULUS, &n,
					CRED_PART_RSA_PUB_EXP, &e, CRED_PART_RSA_PRIV_EXP, &d,
					CRED_PART_RSA_PRIME1, &p, CRED_PART_RSA_PRIME2, &q,
					CRED_PART_RSA_EXP1, &exp1, CRED_PART_RSA_EXP2, &exp2,
					CRED_PART_RSA_COEFF, &coeff, CRED_PART_END))
	{
		*encoding = asn1_wrap(ASN1_SEQUENCE, "cmmssssss",
						ASN1_INTEGER_0,
						asn1_integer("c", n),
						asn1_integer("c", e),
						asn1_integer("c", d),
						asn1_integer("c", p),
						asn1_integer("c", q),
						asn1_integer("c", exp1),
						asn1_integer("c", exp2),
						asn1_integer("c", coeff));
		return TRUE;
	}
	return FALSE;
}

/**
 * Build the SHA1 hash of pubkey(info) ASN.1 data
 */
static bool hash_pubkey(chunk_t pubkey, chunk_t *hash)
{
	hasher_t *hasher;

	hasher = lib->crypto->create_hasher(lib->crypto, HASH_SHA1);
	if (!hasher || !hasher->allocate_hash(hasher, pubkey, hash))
	{
		DESTROY_IF(hasher);
		chunk_free(&pubkey);
		DBG1(DBG_LIB, "SHA1 hash algorithm not supported, "
			 "fingerprinting failed");
		return FALSE;
	}
	hasher->destroy(hasher);
	chunk_free(&pubkey);
	return TRUE;
}

/**
 * build the fingerprint of the subjectPublicKeyInfo object
 */
static bool build_info_sha1(chunk_t *encoding, va_list args)
{
	chunk_t pubkey;

	if (build_pub_info(&pubkey, args))
	{
		return hash_pubkey(pubkey, encoding);
	}
	return FALSE;
}

/**
 * build the fingerprint of the subjectPublicKey object
 */
static bool build_sha1(chunk_t *encoding, va_list args)
{
	chunk_t pubkey;

	if (build_pub(&pubkey, args))
	{
		return hash_pubkey(pubkey, encoding);
	}
	return FALSE;
}

/**
 * See header.
 */
bool pkcs1_encoder_encode(cred_encoding_type_t type, chunk_t *encoding,
						  va_list args)
{
	switch (type)
	{
		case KEYID_PUBKEY_INFO_SHA1:
			return build_info_sha1(encoding, args);
		case KEYID_PUBKEY_SHA1:
			return build_sha1(encoding, args);
		case PUBKEY_ASN1_DER:
			return build_pub(encoding, args);
		case PUBKEY_SPKI_ASN1_DER:
			return build_pub_info(encoding, args);
		case PUBKEY_RSA_MODULUS:
			return build_pub_modulus(encoding, args);
		case PRIVKEY_ASN1_DER:
			return build_priv(encoding, args);
		default:
			return FALSE;
	}
}


