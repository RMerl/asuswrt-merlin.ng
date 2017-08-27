/*
 * Copyright (C) 2008-2009 Martin Willi
 * Copyright (C) 2008 Tobias Brunner
 * Copyright (C) 2000-2008 Andreas Steffen
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

#include "pkcs1_builder.h"

#include <utils/debug.h>
#include <asn1/oid.h>
#include <asn1/asn1.h>
#include <asn1/asn1_parser.h>
#include <credentials/keys/private_key.h>

/**
 * ASN.1 definition of a subjectPublicKeyInfo structure
 */
static const asn1Object_t pkinfoObjects[] = {
	{ 0, "subjectPublicKeyInfo",ASN1_SEQUENCE,		ASN1_NONE	}, /* 0 */
	{ 1,   "algorithm",			ASN1_EOC,			ASN1_RAW	}, /* 1 */
	{ 1,   "subjectPublicKey",	ASN1_BIT_STRING,	ASN1_BODY	}, /* 2 */
	{ 0, "exit",				ASN1_EOC,			ASN1_EXIT	}
};
#define PKINFO_SUBJECT_PUBLIC_KEY_ALGORITHM	1
#define PKINFO_SUBJECT_PUBLIC_KEY			2

/**
 * Load a generic public key from an ASN.1 encoded blob
 */
static public_key_t *parse_public_key(chunk_t blob)
{
	asn1_parser_t *parser;
	chunk_t object;
	int objectID;
	public_key_t *key = NULL;
	key_type_t type = KEY_ANY;

	parser = asn1_parser_create(pkinfoObjects, blob);

	while (parser->iterate(parser, &objectID, &object))
	{
		switch (objectID)
		{
			case PKINFO_SUBJECT_PUBLIC_KEY_ALGORITHM:
			{
				int oid = asn1_parse_algorithmIdentifier(object,
										parser->get_level(parser)+1, NULL);

				if (oid == OID_RSA_ENCRYPTION || oid == OID_RSAES_OAEP)
				{
					type = KEY_RSA;
				}
				else if (oid == OID_EC_PUBLICKEY)
				{
					/* we need the whole subjectPublicKeyInfo for EC public keys */
					key = lib->creds->create(lib->creds, CRED_PUBLIC_KEY,
								KEY_ECDSA, BUILD_BLOB_ASN1_DER, blob, BUILD_END);
					goto end;
				}
				else
				{
					/* key type not supported */
					goto end;
				}
				break;
			}
			case PKINFO_SUBJECT_PUBLIC_KEY:
				if (object.len > 0 && *object.ptr == 0x00)
				{
					/* skip initial bit string octet defining 0 unused bits */
					object = chunk_skip(object, 1);
				}
				DBG2(DBG_ASN, "-- > --");
				key = lib->creds->create(lib->creds, CRED_PUBLIC_KEY, type,
										 BUILD_BLOB_ASN1_DER, object, BUILD_END);
				DBG2(DBG_ASN, "-- < --");
				break;
		}
	}

end:
	parser->destroy(parser);
	return key;
}

/**
 * ASN.1 definition of RSApublicKey
 */
static const asn1Object_t pubkeyObjects[] = {
	{ 0, "RSAPublicKey",		ASN1_SEQUENCE,	ASN1_OBJ  }, /*  0 */
	{ 1,   "modulus",			ASN1_INTEGER,	ASN1_BODY }, /*  1 */
	{ 1,   "publicExponent",	ASN1_INTEGER,	ASN1_BODY }, /*  2 */
	{ 0, "exit",				ASN1_EOC,		ASN1_EXIT }
};
#define PUB_KEY_RSA_PUBLIC_KEY		0
#define PUB_KEY_MODULUS				1
#define PUB_KEY_EXPONENT			2

/**
 * Load a RSA public key from an ASN.1 encoded blob.
 */
static public_key_t *parse_rsa_public_key(chunk_t blob)
{
	chunk_t n, e;
	asn1_parser_t *parser;
	chunk_t object;
	int objectID;
	bool success = FALSE;

	parser = asn1_parser_create(pubkeyObjects, blob);

	while (parser->iterate(parser, &objectID, &object))
	{
		switch (objectID)
		{
			case PUB_KEY_MODULUS:
				n = object;
				break;
			case PUB_KEY_EXPONENT:
				e = object;
				break;
		}
	}
	success = parser->success(parser);
	parser->destroy(parser);

	if (!success)
	{
		return NULL;
	}
	return lib->creds->create(lib->creds, CRED_PUBLIC_KEY, KEY_RSA,
						BUILD_RSA_MODULUS, n, BUILD_RSA_PUB_EXP, e, BUILD_END);
}

/**
 * ASN.1 definition of a PKCS#1 RSA private key
 */
static const asn1Object_t privkeyObjects[] = {
	{ 0, "RSAPrivateKey",		ASN1_SEQUENCE,     ASN1_NONE }, /*  0 */
	{ 1,   "version",			ASN1_INTEGER,      ASN1_BODY }, /*  1 */
	{ 1,   "modulus",			ASN1_INTEGER,      ASN1_BODY }, /*  2 */
	{ 1,   "publicExponent",	ASN1_INTEGER,      ASN1_BODY }, /*  3 */
	{ 1,   "privateExponent",	ASN1_INTEGER,      ASN1_BODY }, /*  4 */
	{ 1,   "prime1",			ASN1_INTEGER,      ASN1_BODY }, /*  5 */
	{ 1,   "prime2",			ASN1_INTEGER,      ASN1_BODY }, /*  6 */
	{ 1,   "exponent1",			ASN1_INTEGER,      ASN1_BODY }, /*  7 */
	{ 1,   "exponent2",			ASN1_INTEGER,      ASN1_BODY }, /*  8 */
	{ 1,   "coefficient",		ASN1_INTEGER,      ASN1_BODY }, /*  9 */
	{ 1,   "otherPrimeInfos",	ASN1_SEQUENCE,     ASN1_OPT |
												   ASN1_LOOP }, /* 10 */
	{ 2,     "otherPrimeInfo",	ASN1_SEQUENCE,     ASN1_NONE }, /* 11 */
	{ 3,       "prime",			ASN1_INTEGER,      ASN1_BODY }, /* 12 */
	{ 3,       "exponent",		ASN1_INTEGER,      ASN1_BODY }, /* 13 */
	{ 3,       "coefficient",	ASN1_INTEGER,      ASN1_BODY }, /* 14 */
	{ 1,   "end opt or loop",	ASN1_EOC,          ASN1_END  }, /* 15 */
	{ 0, "exit",				ASN1_EOC,          ASN1_EXIT }
};
#define PRIV_KEY_VERSION		 1
#define PRIV_KEY_MODULUS		 2
#define PRIV_KEY_PUB_EXP		 3
#define PRIV_KEY_PRIV_EXP		 4
#define PRIV_KEY_PRIME1			 5
#define PRIV_KEY_PRIME2			 6
#define PRIV_KEY_EXP1			 7
#define PRIV_KEY_EXP2			 8
#define PRIV_KEY_COEFF			 9

/**
 * Load a RSA private key from a ASN1 encoded blob.
 */
static private_key_t *parse_rsa_private_key(chunk_t blob)
{
	chunk_t n, e, d, p, q, exp1, exp2, coeff;
	asn1_parser_t *parser;
	chunk_t object;
	int objectID ;
	bool success = FALSE;

	parser = asn1_parser_create(privkeyObjects, blob);
	parser->set_flags(parser, FALSE, TRUE);

	while (parser->iterate(parser, &objectID, &object))
	{
		switch (objectID)
		{
			case PRIV_KEY_VERSION:
				if (object.len > 0 && *object.ptr != 0)
				{
					DBG1(DBG_ASN, "PKCS#1 private key format is not version 1");
					goto end;
				}
				break;
			case PRIV_KEY_MODULUS:
				n = object;
				break;
			case PRIV_KEY_PUB_EXP:
				e = object;
				break;
			case PRIV_KEY_PRIV_EXP:
				d = object;
				break;
			case PRIV_KEY_PRIME1:
				p = object;
				break;
			case PRIV_KEY_PRIME2:
				q = object;
				break;
			case PRIV_KEY_EXP1:
				exp1 = object;
				break;
			case PRIV_KEY_EXP2:
				exp2 = object;
				break;
			case PRIV_KEY_COEFF:
				coeff = object;
				break;
		}
	}
	success = parser->success(parser);

end:
	parser->destroy(parser);
	if (!success)
	{
		return NULL;
	}
	return lib->creds->create(lib->creds, CRED_PRIVATE_KEY, KEY_RSA,
			BUILD_RSA_MODULUS, n, BUILD_RSA_PUB_EXP, e, BUILD_RSA_PRIV_EXP, d,
			BUILD_RSA_PRIME1, p,  BUILD_RSA_PRIME2, q, BUILD_RSA_EXP1, exp1,
			BUILD_RSA_EXP2, exp2, BUILD_RSA_COEFF, coeff, BUILD_END);
}

/**
 * See header.
 */
public_key_t *pkcs1_public_key_load(key_type_t type, va_list args)
{
	chunk_t blob = chunk_empty;

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
	switch (type)
	{
		case KEY_ANY:
			return parse_public_key(blob);
		case KEY_RSA:
			return parse_rsa_public_key(blob);
		default:
			return NULL;
	}
}

/**
 * See header.
 */
private_key_t *pkcs1_private_key_load(key_type_t type, va_list args)
{
	chunk_t blob = chunk_empty;

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
	return parse_rsa_private_key(blob);
}

