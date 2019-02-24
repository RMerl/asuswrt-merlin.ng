/*
 * Copyright (C) 2013-2018 Tobias Brunner
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

#include "sshkey_encoder.h"

#include <asn1/asn1.h>
#include <asn1/oid.h>
#include <bio/bio_writer.h>

#define ECDSA_PREFIX "ecdsa-sha2-"

/**
 * Write an EC domain parameter identifier as defined in RFC 5656
 */
static void write_ec_identifier(bio_writer_t *writer, char *prefix, int oid,
								chunk_t enc)
{
	char *curve, identifier[128];

	switch (oid)
	{
		case OID_PRIME256V1:
			curve = strdup("nistp256");
			break;
		case OID_SECT384R1:
			curve = strdup("nistp384");
			break;
		case OID_SECT521R1:
			curve = strdup("nistp521");
			break;
		default:
			curve = asn1_oid_to_string(enc);
			break;
	}
	if (curve && snprintf(identifier, sizeof(identifier), "%s%s", prefix,
						  curve) < sizeof(identifier))
	{
		writer->write_data32(writer, chunk_from_str(identifier));
	}
	free(curve);
}

/**
 * Encode the public key as Base64 encoded SSH key blob
 */
static bool build_public_key(chunk_t *encoding, va_list args)
{
	bio_writer_t *writer;
	chunk_t n, e;

	if (cred_encoding_args(args, CRED_PART_RSA_MODULUS, &n,
						   CRED_PART_RSA_PUB_EXP, &e, CRED_PART_END))
	{
		writer = bio_writer_create(0);
		writer->write_data32(writer, chunk_from_str("ssh-rsa"));

		writer->write_data32(writer, e);
		writer->write_data32(writer, n);
		*encoding = chunk_to_base64(writer->get_buf(writer), NULL);
		writer->destroy(writer);
		return TRUE;
	}
	else if (cred_encoding_args(args, CRED_PART_EDDSA_PUB_ASN1_DER, &n,
								CRED_PART_END))
	{
		chunk_t alg;
		char *prefix;
		int oid;

		/* parse subjectPublicKeyInfo */
		if (asn1_unwrap(&n, &n) != ASN1_SEQUENCE)
		{
			return FALSE;
		}
		oid = asn1_parse_algorithmIdentifier(n, 1, NULL);
		switch (oid)
		{
			case OID_ED25519:
				prefix = "ssh-ed25519";
				break;
			case OID_ED448:
				prefix = "ssh-ed448";
				break;
			default:
				return FALSE;
		}
		if (asn1_unwrap(&n, &alg) != ASN1_SEQUENCE ||
			asn1_unwrap(&n, &n) != ASN1_BIT_STRING || !n.len)
		{
			return FALSE;
		}
		writer = bio_writer_create(0);
		writer->write_data32(writer, chunk_from_str(prefix));
		writer->write_data32(writer, chunk_skip(n, 1));
		*encoding = chunk_to_base64(writer->get_buf(writer), NULL);
		writer->destroy(writer);
		return TRUE;
	}
	else if (cred_encoding_args(args, CRED_PART_ECDSA_PUB_ASN1_DER, &n,
								CRED_PART_END))
	{
		chunk_t params, alg, q;
		int oid;

		/* parse subjectPublicKeyInfo */
		if (asn1_unwrap(&n, &n) != ASN1_SEQUENCE)
		{
			return FALSE;
		}
		oid = asn1_parse_algorithmIdentifier(n, 1, &params);
		if (oid != OID_EC_PUBLICKEY ||
			asn1_unwrap(&params, &params) != ASN1_OID)
		{
			return FALSE;
		}
		oid = asn1_known_oid(params);
		if (oid == OID_UNKNOWN)
		{
			return FALSE;
		}
		if (asn1_unwrap(&n, &alg) != ASN1_SEQUENCE ||
			asn1_unwrap(&n, &q) != ASN1_BIT_STRING)
		{
			return FALSE;
		}
		writer = bio_writer_create(0);
		write_ec_identifier(writer, ECDSA_PREFIX, oid, params);
		write_ec_identifier(writer, "", oid, params);

		q = chunk_skip_zero(q);
		writer->write_data32(writer, q);
		*encoding = chunk_to_base64(writer->get_buf(writer), NULL);
		writer->destroy(writer);
		return TRUE;
	}
	return FALSE;
}

bool sshkey_encoder_encode(cred_encoding_type_t type, chunk_t *encoding,
						   va_list args)
{
	switch (type)
	{
		case PUBKEY_SSHKEY:
			return build_public_key(encoding, args);
		default:
			return FALSE;
	}
}
