/*
 * Copyright (C) 2013 Reto Buerki
 * Copyright (C) 2013 Adrian-Ken Rueegsegger
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

#include <utils/debug.h>
#include <asn1/asn1.h>
#include <asn1/oid.h>

#include "tkm_encoder.h"

/**
 * Build the SHA1 hash of pubkey(info) ASN.1 data.
 */
static bool hash_pubkey(chunk_t pubkey, chunk_t *hash)
{
	hasher_t *hasher;

	hasher = lib->crypto->create_hasher(lib->crypto, HASH_SHA1);
	if (!hasher || !hasher->allocate_hash(hasher, pubkey, hash))
	{
		DBG1(DBG_LIB, "SHA1 hash algorithm not supported, "
			 "fingerprinting failed");
		DESTROY_IF(hasher);
		chunk_free(&pubkey);
		return FALSE;
	}
	hasher->destroy(hasher);
	chunk_free(&pubkey);
	return TRUE;
}

/**
 * Encode the public key blob into subjectPublicKeyInfo.
 */
static bool build_pub_info(chunk_t *encoding, va_list args)
{
	chunk_t blob;

	if (cred_encoding_args(args, CRED_PART_RSA_PUB_ASN1_DER, &blob,
						   CRED_PART_END))
	{
		*encoding = asn1_wrap(ASN1_SEQUENCE, "mm",
							  asn1_algorithmIdentifier(OID_RSA_ENCRYPTION),
							  asn1_bitstring("c", blob));
		return TRUE;
	}
	return FALSE;
}

/**
 * Build the fingerprint of the subjectPublicKeyInfo object.
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
 * Build the fingerprint of the subjectPublicKey object.
 */
static bool build_sha1(chunk_t *encoding, va_list args)
{
	chunk_t blob;

	if (cred_encoding_args(args, CRED_PART_RSA_PUB_ASN1_DER, &blob,
						   CRED_PART_END))
	{
		return hash_pubkey(chunk_clone(blob), encoding);
	}
	return FALSE;
}

/**
 * See header.
 */
bool tkm_encoder_encode(cred_encoding_type_t type, chunk_t *encoding,
						va_list args)
{
	switch (type)
	{
		case KEYID_PUBKEY_INFO_SHA1:
			return build_info_sha1(encoding, args);
		case KEYID_PUBKEY_SHA1:
			return build_sha1(encoding, args);
		default:
			return FALSE;
	}
}
