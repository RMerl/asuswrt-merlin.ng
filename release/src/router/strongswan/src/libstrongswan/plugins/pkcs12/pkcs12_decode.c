/*
 * Copyright (C) 2013 Tobias Brunner
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

#include "pkcs12_decode.h"

#include <utils/debug.h>
#include <asn1/oid.h>
#include <asn1/asn1.h>
#include <asn1/asn1_parser.h>
#include <credentials/sets/mem_cred.h>

typedef struct private_pkcs12_t private_pkcs12_t;

/**
 * Private data of a pkcs12_t object
 */
struct private_pkcs12_t {

	/**
	 * Public interface
	 */
	pkcs12_t public;

	/**
	 * Contained credentials
	 */
	mem_cred_t *creds;
};

METHOD(container_t, get_type, container_type_t,
	private_pkcs12_t *this)
{
	return CONTAINER_PKCS12;
}

METHOD(container_t, get_data, bool,
	private_pkcs12_t *this, chunk_t *data)
{
	/* we could return the content of the outer-most PKCS#7 container (authSafe)
	 * don't really see the point though */
	return FALSE;
}

METHOD(container_t, get_encoding, bool,
	private_pkcs12_t *this, chunk_t *encoding)
{
	/* similar to get_data() we don't have any use for it at the moment */
	return FALSE;
}

METHOD(pkcs12_t, create_cert_enumerator, enumerator_t*,
	private_pkcs12_t *this)
{
	return this->creds->set.create_cert_enumerator(&this->creds->set, CERT_ANY,
												   KEY_ANY, NULL, FALSE);
}

METHOD(pkcs12_t, create_key_enumerator, enumerator_t*,
	private_pkcs12_t *this)
{
	return this->creds->set.create_private_enumerator(&this->creds->set,
													  KEY_ANY, NULL);
}

METHOD(container_t, destroy, void,
	private_pkcs12_t *this)
{
	this->creds->destroy(this->creds);
	free(this);
}

static private_pkcs12_t *pkcs12_create()
{
	private_pkcs12_t *this;

	INIT(this,
		.public = {
			.container = {
				.get_type = _get_type,
				.create_signature_enumerator = (void*)enumerator_create_empty,
				.get_data = _get_data,
				.get_encoding = _get_encoding,
				.destroy = _destroy,
			},
			.create_cert_enumerator = _create_cert_enumerator,
			.create_key_enumerator = _create_key_enumerator,
		},
		.creds = mem_cred_create(),
	);
	return this;
}

/**
 * ASN.1 definition of an CertBag structure
 */
static const asn1Object_t certBagObjects[] = {
	{ 0, "CertBag",			ASN1_SEQUENCE,		ASN1_BODY			}, /* 0 */
	{ 1,   "certId",		ASN1_OID,			ASN1_BODY			}, /* 1 */
	{ 1,   "certValue",		ASN1_CONTEXT_C_0,	ASN1_BODY  			}, /* 2 */
	{ 0, "exit",			ASN1_EOC,			ASN1_EXIT			}
};
#define CERT_BAG_ID 	1
#define CERT_BAG_VALUE 	2

/**
 * Parse a CertBag structure and extract certificate
 */
static bool add_certificate(private_pkcs12_t *this, int level0, chunk_t blob)
{
	asn1_parser_t *parser;
	chunk_t object;
	int objectID;
	int oid = OID_UNKNOWN;
	bool success = FALSE;

	parser = asn1_parser_create(certBagObjects, blob);
	parser->set_top_level(parser, level0);

	while (parser->iterate(parser, &objectID, &object))
	{
		switch (objectID)
		{
			case CERT_BAG_ID:
				oid = asn1_known_oid(object);
				break;
			case CERT_BAG_VALUE:
			{
				if (oid == OID_X509_CERTIFICATE &&
					asn1_parse_simple_object(&object, ASN1_OCTET_STRING,
								parser->get_level(parser)+1, "x509Certificate"))
				{
					certificate_t *cert;

					DBG2(DBG_ASN, "-- > parsing certificate from PKCS#12");
					cert = lib->creds->create(lib->creds,
											  CRED_CERTIFICATE, CERT_X509,
											  BUILD_BLOB_ASN1_DER, object,
											  BUILD_END);
					if (cert)
					{
						this->creds->add_cert(this->creds, FALSE, cert);
						DBG2(DBG_ASN, "-- < --");
					}
					else
					{
						DBG2(DBG_ASN, "-- < failed parsing certificate from "
							 "PKCS#12");
					}
				}
				break;
			}
		}
	}
	success = parser->success(parser);
	parser->destroy(parser);
	return success;
}

/**
 * ASN.1 definition of an AuthenticatedSafe structure
 */
static const asn1Object_t safeContentsObjects[] = {
	{ 0, "SafeContents",	ASN1_SEQUENCE,		ASN1_LOOP			}, /* 0 */
	{ 1,   "SafeBag",		ASN1_SEQUENCE,		ASN1_BODY			}, /* 1 */
	{ 2,     "bagId",		ASN1_OID,			ASN1_BODY			}, /* 2 */
	{ 2,     "bagValue",	ASN1_CONTEXT_C_0,	ASN1_BODY  			}, /* 3 */
	{ 2,     "bagAttr",		ASN1_SET,			ASN1_OPT|ASN1_RAW 	}, /* 4 */
	{ 2,     "end opt",		ASN1_EOC,			ASN1_END 			}, /* 5 */
	{ 0, "end loop",		ASN1_EOC,			ASN1_END			}, /* 6 */
	{ 0, "exit",			ASN1_EOC,			ASN1_EXIT			}
};
#define SAFE_BAG_ID 	2
#define SAFE_BAG_VALUE 	3

/**
 * Parse a SafeContents structure and extract credentials
 */
static bool parse_safe_contents(private_pkcs12_t *this, int level0,
								chunk_t blob)
{
	asn1_parser_t *parser;
	chunk_t object;
	int objectID;
	int oid = OID_UNKNOWN;
	bool success = FALSE;

	parser = asn1_parser_create(safeContentsObjects, blob);
	parser->set_top_level(parser, level0);

	while (parser->iterate(parser, &objectID, &object))
	{
		switch (objectID)
		{
			case SAFE_BAG_ID:
				oid = asn1_known_oid(object);
				break;
			case SAFE_BAG_VALUE:
			{
				switch (oid)
				{
					case OID_P12_CERT_BAG:
					{
						add_certificate(this, parser->get_level(parser)+1,
										object);
						break;
					}
					case OID_P12_KEY_BAG:
					case OID_P12_PKCS8_KEY_BAG:
					{
						private_key_t *key;

						DBG2(DBG_ASN, "-- > parsing private key from PKCS#12");
						key = lib->creds->create(lib->creds, CRED_PRIVATE_KEY,
										KEY_ANY, BUILD_BLOB_ASN1_DER, object,
										BUILD_END);
						if (key)
						{
							this->creds->add_key(this->creds, key);
							DBG2(DBG_ASN, "-- < --");
						}
						else
						{
							DBG2(DBG_ASN, "-- < failed parsing private key "
								 "from PKCS#12");
						}
					}
					default:
						break;
				}
				break;
			}
		}
	}
	success = parser->success(parser);
	parser->destroy(parser);
	return success;
}

/**
 * ASN.1 definition of an AuthenticatedSafe structure
 */
static const asn1Object_t authenticatedSafeObjects[] = {
	{ 0, "AuthenticatedSafe",	ASN1_SEQUENCE,	ASN1_LOOP	}, /* 0 */
	{ 1,   "ContentInfo",		ASN1_SEQUENCE,	ASN1_OBJ	}, /* 1 */
	{ 0, "end loop",			ASN1_EOC,		ASN1_END	}, /* 2 */
	{ 0, "exit",				ASN1_EOC,		ASN1_EXIT	}
};
#define AUTHENTICATED_SAFE_DATA 	1

/**
 * Parse an AuthenticatedSafe structure
 */
static bool parse_authenticated_safe(private_pkcs12_t *this, chunk_t blob)
{
	asn1_parser_t *parser;
	chunk_t object;
	int objectID;
	bool success = FALSE;

	parser = asn1_parser_create(authenticatedSafeObjects, blob);

	while (parser->iterate(parser, &objectID, &object))
	{
		switch (objectID)
		{
			case AUTHENTICATED_SAFE_DATA:
			{
				container_t *container;
				chunk_t data;

				container = lib->creds->create(lib->creds, CRED_CONTAINER,
										CONTAINER_PKCS7, BUILD_BLOB_ASN1_DER,
										object, BUILD_END);
				if (!container)
				{
					goto end;
				}
				switch (container->get_type(container))
				{
					case CONTAINER_PKCS7_DATA:
					case CONTAINER_PKCS7_ENCRYPTED_DATA:
					case CONTAINER_PKCS7_ENVELOPED_DATA:
						if (container->get_data(container, &data))
						{
							break;
						}
						/* fall-through */
					default:
						container->destroy(container);
						goto end;
				}
				container->destroy(container);

				if (!parse_safe_contents(this, parser->get_level(parser)+1,
										 data))
				{
					chunk_free(&data);
					goto end;
				}
				chunk_free(&data);
				break;
			}
		}
	}
	success = parser->success(parser);
end:
	parser->destroy(parser);
	return success;
}

/**
 * Verify the given MAC with available passwords.
 */
static bool verify_mac(hash_algorithm_t hash, chunk_t salt,
					   uint64_t iterations, chunk_t data, chunk_t mac)
{
	integrity_algorithm_t integ;
	enumerator_t *enumerator;
	shared_key_t *shared;
	signer_t *signer;
	chunk_t key, calculated;
	bool success = FALSE;

	integ = hasher_algorithm_to_integrity(hash, mac.len);
	signer = lib->crypto->create_signer(lib->crypto, integ);
	if (!signer)
	{
		return FALSE;
	}
	key = chunk_alloca(signer->get_key_size(signer));
	calculated = chunk_alloca(signer->get_block_size(signer));

	enumerator = lib->credmgr->create_shared_enumerator(lib->credmgr,
										SHARED_PRIVATE_KEY_PASS, NULL, NULL);
	while (enumerator->enumerate(enumerator, &shared, NULL, NULL))
	{
		if (!pkcs12_derive_key(hash, shared->get_key(shared), salt, iterations,
							   PKCS12_KEY_MAC, key))
		{
			break;
		}
		if (!signer->set_key(signer, key) ||
			!signer->get_signature(signer, data, calculated.ptr))
		{
			break;
		}
		if (chunk_equals_const(mac, calculated))
		{
			success = TRUE;
			break;
		}
	}
	enumerator->destroy(enumerator);
	signer->destroy(signer);
	return success;
}

/**
 * ASN.1 definition of digestInfo
 */
static const asn1Object_t digestInfoObjects[] = {
	{ 0, "digestInfo",			ASN1_SEQUENCE,		ASN1_OBJ	}, /*  0 */
	{ 1,   "digestAlgorithm",	ASN1_EOC,			ASN1_RAW	}, /*  1 */
	{ 1,   "digest",			ASN1_OCTET_STRING,	ASN1_BODY	}, /*  2 */
	{ 0, "exit",				ASN1_EOC,			ASN1_EXIT	}
};
#define DIGEST_INFO_ALGORITHM		1
#define DIGEST_INFO_DIGEST			2

/**
 * Parse a digestInfo structure
 */
static bool parse_digest_info(chunk_t blob, int level0, hash_algorithm_t *hash,
							  chunk_t *digest)
{
	asn1_parser_t *parser;
	chunk_t object;
	int objectID;
	bool success;

	parser = asn1_parser_create(digestInfoObjects, blob);
	parser->set_top_level(parser, level0);

	while (parser->iterate(parser, &objectID, &object))
	{
		switch (objectID)

		{
			case DIGEST_INFO_ALGORITHM:
			{
				int oid = asn1_parse_algorithmIdentifier(object,
									 parser->get_level(parser)+1, NULL);

				*hash = hasher_algorithm_from_oid(oid);
				break;
			}
			case DIGEST_INFO_DIGEST:
			{
				*digest = object;
				break;
			}
			default:
				break;
		}
	}
	success = parser->success(parser);
	parser->destroy(parser);
	return success;
}

/**
 * ASN.1 definition of a PFX structure
 */
static const asn1Object_t PFXObjects[] = {
	{ 0, "PFX",				ASN1_SEQUENCE,		ASN1_NONE			}, /* 0 */
	{ 1,   "version",		ASN1_INTEGER,		ASN1_BODY			}, /* 1 */
	{ 1,   "authSafe",		ASN1_SEQUENCE,		ASN1_OBJ			}, /* 2 */
	{ 1,   "macData",		ASN1_SEQUENCE,		ASN1_OPT|ASN1_BODY	}, /* 3 */
	{ 2,     "mac",			ASN1_SEQUENCE,		ASN1_RAW			}, /* 4 */
	{ 2,     "macSalt",		ASN1_OCTET_STRING,	ASN1_BODY			}, /* 5 */
	{ 2,     "iterations",	ASN1_INTEGER,		ASN1_DEF|ASN1_BODY	}, /* 6 */
	{ 1,   "end opt",		ASN1_EOC,			ASN1_END			}, /* 7 */
	{ 0, "exit",			ASN1_EOC,			ASN1_EXIT			}
};
#define PFX_AUTH_SAFE	2
#define PFX_MAC			4
#define PFX_SALT		5
#define PFX_ITERATIONS	6

/**
 * Parse an ASN.1 encoded PFX structure
 */
static bool parse_PFX(private_pkcs12_t *this, chunk_t blob)
{
	asn1_parser_t *parser;
	int objectID;
	chunk_t object, auth_safe, digest = chunk_empty, salt = chunk_empty,
			data = chunk_empty;
	hash_algorithm_t hash = HASH_UNKNOWN;
	container_t *container = NULL;
	uint64_t iterations = 0;
	bool success = FALSE;

	parser = asn1_parser_create(PFXObjects, blob);

	while (parser->iterate(parser, &objectID, &object))
	{
		switch (objectID)
		{
			case PFX_AUTH_SAFE:
			{
				auth_safe = object;
				break;
			}
			case PFX_MAC:
			{
				if (!parse_digest_info(object, parser->get_level(parser)+1,
									   &hash, &digest))
				{
					goto end_parse;
				}
				break;
			}
			case PFX_SALT:
			{
				salt = object;
				break;
			}
			case PFX_ITERATIONS:
			{
				iterations = object.len ? asn1_parse_integer_uint64(object) : 1;
				break;
			}
		}
	}
	success = parser->success(parser);

end_parse:
	parser->destroy(parser);
	if (!success)
	{
		return FALSE;
	}

	success = FALSE;
	DBG2(DBG_ASN, "-- > --");
	container = lib->creds->create(lib->creds, CRED_CONTAINER, CONTAINER_PKCS7,
								   BUILD_BLOB_ASN1_DER, auth_safe, BUILD_END);
	if (container && container->get_data(container, &data))
	{
		if (hash != HASH_UNKNOWN)
		{
			if (container->get_type(container) != CONTAINER_PKCS7_DATA)
			{
				goto end;
			}
			if (!verify_mac(hash, salt, iterations, data, digest))
			{
				DBG1(DBG_ASN, "  MAC verification of PKCS#12 container failed");
				goto end;
			}
		}
		else
		{
			enumerator_t *enumerator;
			auth_cfg_t *auth;

			if (container->get_type(container) != CONTAINER_PKCS7_SIGNED_DATA)
			{
				goto end;
			}
			enumerator = container->create_signature_enumerator(container);
			if (!enumerator->enumerate(enumerator, &auth))
			{
				DBG1(DBG_ASN, "  signature verification of PKCS#12 container "
					 "failed");
				enumerator->destroy(enumerator);
				goto end;
			}
			enumerator->destroy(enumerator);
		}
		success = parse_authenticated_safe(this, data);
	}
end:
	DBG2(DBG_ASN, "-- < --");
	DESTROY_IF(container);
	chunk_free(&data);
	return success;
}

/**
 * See header.
 */
pkcs12_t *pkcs12_decode(container_type_t type, va_list args)
{
	private_pkcs12_t *this;
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
	if (blob.len)
	{
		if (blob.len >= 2 &&
			blob.ptr[0] == ASN1_SEQUENCE && blob.ptr[1] == 0x80)
		{	/* looks like infinite length BER encoding, but we can't handle it.
			 */
			return NULL;
		}
		this = pkcs12_create();
		if (parse_PFX(this, blob))
		{
			return &this->public;
		}
		destroy(this);
	}
	return NULL;
}
