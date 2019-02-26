/*
 * Copyright (C) 2012 Tobias Brunner
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

#include "pkcs8_builder.h"

#include <utils/debug.h>
#include <asn1/oid.h>
#include <asn1/asn1.h>
#include <asn1/asn1_parser.h>
#include <crypto/pkcs5.h>
#include <credentials/keys/private_key.h>

/**
 * ASN.1 definition of a privateKeyInfo structure
 */
static const asn1Object_t pkinfoObjects[] = {
	{ 0, "privateKeyInfo",			ASN1_SEQUENCE,		ASN1_NONE	}, /* 0 */
	{ 1,   "version",				ASN1_INTEGER,		ASN1_BODY	}, /* 1 */
	{ 1,   "privateKeyAlgorithm",	ASN1_EOC,			ASN1_RAW	}, /* 2 */
	{ 1,   "privateKey",			ASN1_OCTET_STRING,	ASN1_BODY	}, /* 3 */
	{ 1,   "attributes",			ASN1_CONTEXT_C_0,	ASN1_OPT	}, /* 4 */
	{ 1,   "end opt",				ASN1_EOC,			ASN1_END	}, /* 5 */
	{ 0, "exit",					ASN1_EOC,			ASN1_EXIT	}
};
#define PKINFO_PRIVATE_KEY_ALGORITHM	2
#define PKINFO_PRIVATE_KEY				3

/**
 * Load a generic private key from an ASN.1 encoded blob
 */
static private_key_t *parse_private_key(chunk_t blob)
{
	asn1_parser_t *parser;
	chunk_t object, params = chunk_empty;
	int objectID;
	private_key_t *key = NULL;
	key_type_t type = KEY_ANY;
	builder_part_t part = BUILD_BLOB_ASN1_DER;

	parser = asn1_parser_create(pkinfoObjects, blob);
	parser->set_flags(parser, FALSE, TRUE);

	while (parser->iterate(parser, &objectID, &object))
	{
		switch (objectID)
		{
			case PKINFO_PRIVATE_KEY_ALGORITHM:
			{
				int oid = asn1_parse_algorithmIdentifier(object,
									parser->get_level(parser) + 1, &params);

				switch (oid)
				{
					case OID_RSASSA_PSS:
						/* TODO: parameters associated with such keys should be
						 * treated as restrictions later when signing (the type
						 * itself is already a restriction). However, the
						 * builders currently don't expect any parameters for
						 * RSA keys (we also only pass along the params, not the
						 * exact type, so we'd have to guess that params
						 * indicate RSA/PSS, but they are optional so that won't
						 * work for keys without specific restrictions) */
						params = chunk_empty;
						/* fall-through */
					case OID_RSA_ENCRYPTION:
						type = KEY_RSA;
						break;
					case OID_EC_PUBLICKEY:
						type = KEY_ECDSA;
						break;
					case OID_ED25519:
						type = KEY_ED25519;
						part = BUILD_EDDSA_PRIV_ASN1_DER;
						break;
					case OID_ED448:
						type = KEY_ED448;
						part = BUILD_EDDSA_PRIV_ASN1_DER;
						break;
					default:
						/* key type not supported */
						goto end;
				}
				break;
			}
			case PKINFO_PRIVATE_KEY:
			{
				DBG2(DBG_ASN, "-- > --");
				if (params.ptr)
				{
					key = lib->creds->create(lib->creds, CRED_PRIVATE_KEY,
											 type, BUILD_BLOB_ALGID_PARAMS,
											 params, part, object, BUILD_END);
				}
				else
				{
					key = lib->creds->create(lib->creds, CRED_PRIVATE_KEY,
											 type, part, object, BUILD_END);
				}
				DBG2(DBG_ASN, "-- < --");
				break;
			}
		}
	}

end:
	parser->destroy(parser);
	return key;
}

/**
 * Try to decrypt the given blob with multiple passwords using the given
 * pkcs5 object.
 */
static private_key_t *decrypt_private_key(pkcs5_t *pkcs5, chunk_t blob)
{
	enumerator_t *enumerator;
	shared_key_t *shared;
	private_key_t *private_key = NULL;

	enumerator = lib->credmgr->create_shared_enumerator(lib->credmgr,
										SHARED_PRIVATE_KEY_PASS, NULL, NULL);
	while (enumerator->enumerate(enumerator, &shared, NULL, NULL))
	{
		chunk_t decrypted;

		if (!pkcs5->decrypt(pkcs5, shared->get_key(shared), blob, &decrypted))
		{
			continue;
		}
		private_key = parse_private_key(decrypted);
		if (private_key)
		{
			chunk_clear(&decrypted);
			break;
		}
		chunk_free(&decrypted);
	}
	enumerator->destroy(enumerator);

	return private_key;
}

/**
 * ASN.1 definition of an encryptedPrivateKeyInfo structure
 */
static const asn1Object_t encryptedPKIObjects[] = {
	{ 0, "encryptedPrivateKeyInfo",	ASN1_SEQUENCE,		ASN1_NONE	}, /* 0 */
	{ 1,   "encryptionAlgorithm",	ASN1_EOC,			ASN1_RAW	}, /* 1 */
	{ 1,   "encryptedData",			ASN1_OCTET_STRING,	ASN1_BODY	}, /* 2 */
	{ 0, "exit",					ASN1_EOC,			ASN1_EXIT	}
};
#define EPKINFO_ENCRYPTION_ALGORITHM	1
#define EPKINFO_ENCRYPTED_DATA			2

/**
 * Load an encrypted private key from an ASN.1 encoded blob
 * Schemes per PKCS#5 (RFC 2898)
 */
static private_key_t *parse_encrypted_private_key(chunk_t blob)
{
	asn1_parser_t *parser;
	chunk_t object;
	int objectID;
	private_key_t *key = NULL;
	pkcs5_t *pkcs5 = NULL;

	parser = asn1_parser_create(encryptedPKIObjects, blob);

	while (parser->iterate(parser, &objectID, &object))
	{
		switch (objectID)
		{
			case EPKINFO_ENCRYPTION_ALGORITHM:
			{
				pkcs5 = pkcs5_from_algorithmIdentifier(object,
												parser->get_level(parser) + 1);
				if (!pkcs5)
				{
					goto end;
				}
				break;
			}
			case EPKINFO_ENCRYPTED_DATA:
			{
				key = decrypt_private_key(pkcs5, object);
				break;
			}
		}
	}

end:
	DESTROY_IF(pkcs5);
	parser->destroy(parser);
	return key;
}

/**
 * See header.
 */
private_key_t *pkcs8_private_key_load(key_type_t type, va_list args)
{
	chunk_t blob = chunk_empty;
	private_key_t *key;

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
	/* we don't know whether it is encrypted or not, try both ways */
	key = parse_encrypted_private_key(blob);
	if (!key)
	{
		key = parse_private_key(blob);
	}
	return key;
}

