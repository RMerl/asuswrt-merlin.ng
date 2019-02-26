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

#include "pkcs7_encrypted_data.h"

#include <asn1/asn1.h>
#include <asn1/asn1_parser.h>
#include <asn1/oid.h>
#include <crypto/pkcs5.h>
#include <utils/debug.h>

typedef struct private_pkcs7_encrypted_data_t private_pkcs7_encrypted_data_t;

/**
 * Private data of a PKCS#7 signed-data container.
 */
struct private_pkcs7_encrypted_data_t {

	/**
	 * Implements pkcs7_t.
	 */
	pkcs7_t public;

	/**
	 * Decrypted content
	 */
	chunk_t content;

	/**
	 * Encrypted and encoded PKCS#7 encrypted-data
	 */
	chunk_t encoding;
};

/**
 * Decrypt encrypted-data with available passwords
 */
static bool decrypt(pkcs5_t *pkcs5, chunk_t data, chunk_t *decrypted)
{
	enumerator_t *enumerator;
	shared_key_t *shared;
	bool success = FALSE;

	enumerator = lib->credmgr->create_shared_enumerator(lib->credmgr,
										SHARED_PRIVATE_KEY_PASS, NULL, NULL);
	while (enumerator->enumerate(enumerator, &shared, NULL, NULL))
	{
		if (pkcs5->decrypt(pkcs5, shared->get_key(shared), data, decrypted))
		{
			success = TRUE;
			break;
		}
	}
	enumerator->destroy(enumerator);
	return success;
}

/**
 * ASN.1 definition of the PKCS#7 encrypted-data type
 */
static const asn1Object_t encryptedDataObjects[] = {
	{ 0, "encryptedData",					ASN1_SEQUENCE,		ASN1_NONE }, /* 0 */
	{ 1,   "version",						ASN1_INTEGER,		ASN1_BODY }, /* 1 */
	{ 1,   "encryptedContentInfo",			ASN1_SEQUENCE,		ASN1_OBJ  }, /* 2 */
	{ 2,     "contentType",					ASN1_OID,			ASN1_BODY }, /* 3 */
	{ 2,     "contentEncryptionAlgorithm",	ASN1_EOC,			ASN1_RAW  }, /* 4 */
	{ 2,     "encryptedContent",			ASN1_CONTEXT_S_0,	ASN1_BODY }, /* 5 */
	{ 0, "exit",							ASN1_EOC,			ASN1_EXIT }
};
#define PKCS7_VERSION					1
#define PKCS7_CONTENT_TYPE				3
#define PKCS7_CONTENT_ENC_ALGORITHM		4
#define PKCS7_ENCRYPTED_CONTENT			5

/**
 * Parse and decrypt encrypted-data
 */
static bool parse(private_pkcs7_encrypted_data_t *this, chunk_t content)
{
	asn1_parser_t *parser;
	chunk_t object;
	int objectID, version;
	bool success = FALSE;
	chunk_t encrypted = chunk_empty;
	pkcs5_t *pkcs5 = NULL;

	parser = asn1_parser_create(encryptedDataObjects, content);

	while (parser->iterate(parser, &objectID, &object))
	{
		int level = parser->get_level(parser);

		switch (objectID)
		{
			case PKCS7_VERSION:
				version = object.len ? (int)*object.ptr : 0;
				DBG2(DBG_LIB, "  v%d", version);
				if (version != 0)
				{
					DBG1(DBG_LIB, "encryptedData version is not 0");
					goto end;
				}
				break;
			case PKCS7_CONTENT_TYPE:
				if (asn1_known_oid(object) != OID_PKCS7_DATA)
				{
					DBG1(DBG_LIB, "encrypted content not of type pkcs7 data");
					goto end;
				}
				break;
			case PKCS7_CONTENT_ENC_ALGORITHM:
				pkcs5 = pkcs5_from_algorithmIdentifier(object, level + 1);
				if (!pkcs5)
				{
					DBG1(DBG_LIB, "failed to detect PKCS#5 scheme");
					goto end;
				}
				break;
			case PKCS7_ENCRYPTED_CONTENT:
				encrypted = object;
				break;
		}
	}
	success = parser->success(parser);

end:
	parser->destroy(parser);
	success = success && decrypt(pkcs5, encrypted, &this->content);
	DESTROY_IF(pkcs5);
	return success;
}

METHOD(container_t, get_type, container_type_t,
	private_pkcs7_encrypted_data_t *this)
{
	return CONTAINER_PKCS7_ENCRYPTED_DATA;
}

METHOD(container_t, get_data, bool,
	private_pkcs7_encrypted_data_t *this, chunk_t *data)
{
	if (this->content.len)
	{
		*data = chunk_clone(this->content);
		return TRUE;
	}
	return FALSE;
}

METHOD(container_t, get_encoding, bool,
	private_pkcs7_encrypted_data_t *this, chunk_t *data)
{
	*data = chunk_clone(this->encoding);
	return TRUE;
}

METHOD(container_t, destroy, void,
	private_pkcs7_encrypted_data_t *this)
{
	free(this->content.ptr);
	free(this->encoding.ptr);
	free(this);
}

/**
 * Generic constructor
 */
static private_pkcs7_encrypted_data_t* create_empty()
{
	private_pkcs7_encrypted_data_t *this;

	INIT(this,
		.public = {
			.container = {
				.get_type = _get_type,
				.create_signature_enumerator = (void*)enumerator_create_empty,
				.get_data = _get_data,
				.get_encoding = _get_encoding,
				.destroy = _destroy,
			},
			.create_cert_enumerator = (void*)enumerator_create_empty,
			.get_attribute = (void*)return_false,
		},
	);

	return this;
}

/**
 * See header.
 */
pkcs7_t *pkcs7_encrypted_data_load(chunk_t encoding, chunk_t content)
{
	private_pkcs7_encrypted_data_t *this = create_empty();

	this->encoding = chunk_clone(encoding);
	if (!parse(this, content))
	{
		destroy(this);
		return NULL;
	}

	return &this->public;
}
