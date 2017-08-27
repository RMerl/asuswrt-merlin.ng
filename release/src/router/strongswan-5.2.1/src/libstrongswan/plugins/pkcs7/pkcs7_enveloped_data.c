/*
 * Copyright (C) 2012 Martin Willi
 * Copyright (C) 2012 revosec AG
 * Copyright (C) 2012 Tobias Brunner
 * Copyright (C) 2002-2008 Andreas Steffen
 * Copyright (C) 2005 Jan Hutter, Martin Willi
 * Hochschule fuer Technik Rapperswil, Switzerland
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

#include "pkcs7_enveloped_data.h"

#include <asn1/asn1.h>
#include <asn1/asn1_parser.h>
#include <asn1/oid.h>
#include <credentials/certificates/x509.h>
#include <utils/debug.h>

typedef struct private_pkcs7_enveloped_data_t private_pkcs7_enveloped_data_t;

/**
 * Private data of a PKCS#7 signed-data container.
 */
struct private_pkcs7_enveloped_data_t {

	/**
	 * Implements pkcs7_t.
	 */
	pkcs7_t public;

	/**
	 * Decrypted content
	 */
	chunk_t content;

	/**
	 * Encrypted and encoded PKCS#7 enveloped-data
	 */
	chunk_t encoding;
};

/**
 * ASN.1 definition of the PKCS#7 envelopedData type
 */
static const asn1Object_t envelopedDataObjects[] = {
	{ 0, "envelopedData",					ASN1_SEQUENCE,		ASN1_NONE }, /*  0 */
	{ 1,   "version",						ASN1_INTEGER, 		ASN1_BODY }, /*  1 */
	{ 1,   "recipientInfos",				ASN1_SET,    		ASN1_LOOP }, /*  2 */
	{ 2,     "recipientInfo",				ASN1_SEQUENCE, 		ASN1_BODY }, /*  3 */
	{ 3,       "version",					ASN1_INTEGER, 		ASN1_BODY }, /*  4 */
	{ 3,       "issuerAndSerialNumber",		ASN1_SEQUENCE,		ASN1_BODY }, /*  5 */
	{ 4,         "issuer",					ASN1_SEQUENCE,		ASN1_OBJ  }, /*  6 */
	{ 4,         "serial",					ASN1_INTEGER,		ASN1_BODY }, /*  7 */
	{ 3,       "encryptionAlgorithm",		ASN1_EOC,			ASN1_RAW  }, /*  8 */
	{ 3,       "encryptedKey",				ASN1_OCTET_STRING,	ASN1_BODY }, /*  9 */
	{ 1,   "end loop",						ASN1_EOC,			ASN1_END  }, /* 10 */
	{ 1,   "encryptedContentInfo",			ASN1_SEQUENCE,		ASN1_OBJ  }, /* 11 */
	{ 2,     "contentType",					ASN1_OID,			ASN1_BODY }, /* 12 */
	{ 2,     "contentEncryptionAlgorithm",	ASN1_EOC,			ASN1_RAW  }, /* 13 */
	{ 2,     "encryptedContent",			ASN1_CONTEXT_S_0, 	ASN1_BODY }, /* 14 */
	{ 0, "exit",							ASN1_EOC,			ASN1_EXIT }
};
#define PKCS7_VERSION					 1
#define PKCS7_RECIPIENT_INFO_VERSION	 4
#define PKCS7_ISSUER					 6
#define PKCS7_SERIAL_NUMBER				 7
#define PKCS7_ENCRYPTION_ALG			 8
#define PKCS7_ENCRYPTED_KEY				 9
#define PKCS7_CONTENT_TYPE				12
#define PKCS7_CONTENT_ENC_ALGORITHM		13
#define PKCS7_ENCRYPTED_CONTENT			14

/**
 * Find a private key for issuerAndSerialNumber
 */
static private_key_t *find_private(identification_t *issuer,
								   identification_t *serial)
{
	enumerator_t *enumerator;
	certificate_t *cert;
	public_key_t *public;
	private_key_t *private = NULL;
	identification_t *id;
	chunk_t fp;

	enumerator = lib->credmgr->create_cert_enumerator(lib->credmgr,
											CERT_X509, KEY_RSA, serial, FALSE);
	while (enumerator->enumerate(enumerator, &cert))
	{
		if (issuer->equals(issuer, cert->get_issuer(cert)))
		{
			public = cert->get_public_key(cert);
			if (public)
			{
				if (public->get_fingerprint(public, KEYID_PUBKEY_SHA1, &fp))
				{
					id = identification_create_from_encoding(ID_KEY_ID, fp);
					private = lib->credmgr->get_private(lib->credmgr,
														KEY_ANY, id, NULL);
					id->destroy(id);
				}
				public->destroy(public);
			}
		}
		if (private)
		{
			break;
		}
	}
	enumerator->destroy(enumerator);
	return private;
}

/**
 * Decrypt content using a private key from "issuer"
 */
static bool decrypt(private_key_t *private, chunk_t key, chunk_t iv, int oid,
					chunk_t encrypted, chunk_t *plain)
{
	encryption_algorithm_t alg;
	chunk_t plain_key;
	crypter_t *crypter;
	size_t key_size;

	alg = encryption_algorithm_from_oid(oid, &key_size);
	if (alg == ENCR_UNDEFINED)
	{
		DBG1(DBG_LIB, "unsupported content encryption algorithm");
		return FALSE;
	}
	if (!private->decrypt(private, ENCRYPT_RSA_PKCS1, key, &plain_key))
	{
		DBG1(DBG_LIB, "symmetric key could not be decrypted with rsa");
		return FALSE;
	}
	crypter = lib->crypto->create_crypter(lib->crypto, alg, key_size / 8);
	if (!crypter)
	{
		DBG1(DBG_LIB, "crypter %N-%d not available",
			 encryption_algorithm_names, alg, key_size);
		free(plain_key.ptr);
		return FALSE;
	}
	if (plain_key.len != crypter->get_key_size(crypter))
	{
		DBG1(DBG_LIB, "symmetric key length %d is wrong", plain_key.len);
		free(plain_key.ptr);
		crypter->destroy(crypter);
		return FALSE;
	}
	if (iv.len != crypter->get_iv_size(crypter))
	{
		DBG1(DBG_LIB, "IV length %d is wrong", iv.len);
		free(plain_key.ptr);
		crypter->destroy(crypter);
		return FALSE;
	}
	if (!crypter->set_key(crypter, plain_key) ||
		!crypter->decrypt(crypter, encrypted, iv, plain))
	{
		free(plain_key.ptr);
		crypter->destroy(crypter);
		return FALSE;
	}
	DBG4(DBG_LIB, "decrypted content with padding: %B", plain);
	free(plain_key.ptr);
	crypter->destroy(crypter);
	return TRUE;
}

/**
 * Remove the padding from plain data
 */
static bool remove_padding(private_pkcs7_enveloped_data_t *this)
{
	u_char *pos = this->content.ptr + this->content.len - 1;
	u_char pattern = *pos;
	size_t padding = pattern;

	if (padding > this->content.len)
	{
		DBG1(DBG_LIB, "padding greater than data length");
		return FALSE;
	}
	this->content.len -= padding;

	while (padding-- > 0)
	{
		if (*pos-- != pattern)
		{
			DBG1(DBG_LIB, "wrong padding pattern");
			return FALSE;
		}
	}
	return TRUE;
}

/**
 * Parse and decrypt enveloped-data
 */
static bool parse(private_pkcs7_enveloped_data_t *this, chunk_t content)
{
	asn1_parser_t *parser;
	chunk_t object;
	int objectID, version, alg = OID_UNKNOWN;
	bool success = FALSE;
	identification_t *issuer = NULL, *serial = NULL;
	private_key_t *private = NULL;
	chunk_t iv = chunk_empty, key = chunk_empty, encrypted = chunk_empty;

	parser = asn1_parser_create(envelopedDataObjects, content);
	parser->set_top_level(parser, 0);

	while (parser->iterate(parser, &objectID, &object))
	{
		u_int level = parser->get_level(parser);

		switch (objectID)
		{
			case PKCS7_VERSION:
				version = object.len ? (int)*object.ptr : 0;
				DBG2(DBG_LIB, "  v%d", version);
				if (version != 0)
				{
					DBG1(DBG_LIB, "envelopedData version is not 0");
					goto end;
				}
				break;
			case PKCS7_RECIPIENT_INFO_VERSION:
				version = object.len ? (int)*object.ptr : 0;
				DBG2(DBG_LIB, "  v%d", version);
				if (version != 0)
				{
					DBG1(DBG_LIB, "recipient info version is not 0");
					goto end;
				}
				break;
			case PKCS7_ISSUER:
				if (!issuer)
				{
					issuer = identification_create_from_encoding(ID_DER_ASN1_DN,
																 object);
				}
				break;
			case PKCS7_SERIAL_NUMBER:
				if (!serial)
				{
					serial = identification_create_from_encoding(ID_KEY_ID,
																 object);
				}
				break;
			case PKCS7_ENCRYPTION_ALG:
				if (asn1_parse_algorithmIdentifier(object, level,
												   NULL) != OID_RSA_ENCRYPTION)
				{
					DBG1(DBG_LIB, "only rsa encryption supported");
					goto end;
				}
				break;
			case PKCS7_ENCRYPTED_KEY:
				key = object;
				break;
			case PKCS7_CONTENT_TYPE:
				if (asn1_known_oid(object) != OID_PKCS7_DATA)
				{
					DBG1(DBG_LIB, "encrypted content not of type pkcs7 data");
					goto end;
				}
				break;
			case PKCS7_CONTENT_ENC_ALGORITHM:
				alg = asn1_parse_algorithmIdentifier(object, level, &iv);
				if (!asn1_parse_simple_object(&iv, ASN1_OCTET_STRING,
											  level + 1, "IV"))
				{
					DBG1(DBG_LIB, "IV could not be parsed");
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
	if (!success)
	{
		goto failed;
	}
	success = FALSE;
	if (!issuer)
	{
		goto failed;
	}
	private = find_private(issuer, serial);
	if (!private)
	{
		DBG1(DBG_LIB, "no private key found to decrypt pkcs7");
		goto failed;
	}
	if (!decrypt(private, key, iv, alg, encrypted, &this->content))
	{
		goto failed;
	}
	if (!remove_padding(this))
	{
		goto failed;
	}

	success = TRUE;
failed:
	DESTROY_IF(issuer);
	DESTROY_IF(serial);
	DESTROY_IF(private);
	return success;
}

METHOD(container_t, get_type, container_type_t,
	private_pkcs7_enveloped_data_t *this)
{
	return CONTAINER_PKCS7_ENVELOPED_DATA;
}

METHOD(container_t, create_signature_enumerator, enumerator_t*,
	private_pkcs7_enveloped_data_t *this)
{
	return enumerator_create_empty();
}

METHOD(container_t, get_data, bool,
	private_pkcs7_enveloped_data_t *this, chunk_t *data)
{
	if (this->content.len)
	{
		*data = chunk_clone(this->content);
		return TRUE;
	}
	return FALSE;
}

METHOD(container_t, get_encoding, bool,
	private_pkcs7_enveloped_data_t *this, chunk_t *data)
{
	*data = chunk_clone(this->encoding);
	return TRUE;
}

METHOD(container_t, destroy, void,
	private_pkcs7_enveloped_data_t *this)
{
	free(this->content.ptr);
	free(this->encoding.ptr);
	free(this);
}

/**
 * Generic constructor
 */
static private_pkcs7_enveloped_data_t* create_empty()
{
	private_pkcs7_enveloped_data_t *this;

	INIT(this,
		.public = {
			.container = {
				.get_type = _get_type,
				.create_signature_enumerator = _create_signature_enumerator,
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
pkcs7_t *pkcs7_enveloped_data_load(chunk_t encoding, chunk_t content)
{
	private_pkcs7_enveloped_data_t *this = create_empty();

	this->encoding = chunk_clone(encoding);
	if (!parse(this, content))
	{
		destroy(this);
		return NULL;
	}

	return &this->public;
}

/**
 * Allocate data with an RNG
 */
static bool get_random(rng_quality_t quality, size_t size, chunk_t *out)
{
	rng_t *rng;

	rng = lib->crypto->create_rng(lib->crypto, quality);
	if (!rng)
	{
		return FALSE;
	}
	if (!rng->allocate_bytes(rng, size, out))
	{
		rng->destroy(rng);
		return FALSE;
	}
	rng->destroy(rng);
	return TRUE;
}

/**
 * Encrypt symmetric key using a public key from a certificate
 */
static bool encrypt_key(certificate_t *cert, chunk_t in, chunk_t *out)
{
	public_key_t *key;

	key = cert->get_public_key(cert);
	if (!key)
	{
		return FALSE;
	}
	if (!key->encrypt(key, ENCRYPT_RSA_PKCS1, in, out))
	{
		key->destroy(key);
		return FALSE;
	}
	key->destroy(key);
	return TRUE;
}

/**
 * build a DER-encoded issuerAndSerialNumber object
 */
static chunk_t build_issuerAndSerialNumber(certificate_t *cert)
{
	identification_t *issuer = cert->get_issuer(cert);
	chunk_t serial = chunk_empty;

	if (cert->get_type(cert) == CERT_X509)
	{
		x509_t *x509 = (x509_t*)cert;
		serial = x509->get_serial(x509);
	}

	return asn1_wrap(ASN1_SEQUENCE, "cm",
					 issuer->get_encoding(issuer),
					 asn1_integer("c", serial));
}

/**
 * Generate a new PKCS#7 enveloped-data container
 */
static bool generate(private_pkcs7_enveloped_data_t *this,
				certificate_t *cert, encryption_algorithm_t alg, int key_size)
{
	chunk_t contentEncryptionAlgorithm, encryptedContentInfo, recipientInfo;
	chunk_t iv, symmetricKey, protectedKey, content;
	crypter_t *crypter;
	size_t bs, padding;
	int alg_oid;

	alg_oid = encryption_algorithm_to_oid(alg, key_size);
	if (alg_oid == OID_UNKNOWN)
	{
		DBG1(DBG_LIB, "  encryption algorithm %N not supported",
			 encryption_algorithm_names, alg);
		return FALSE;
	}
	crypter = lib->crypto->create_crypter(lib->crypto, alg, key_size / 8);
	if (crypter == NULL)
	{
		DBG1(DBG_LIB, "  could not create crypter for algorithm %N",
			 encryption_algorithm_names, alg);
		return FALSE;
	}

	if (!get_random(RNG_TRUE, crypter->get_key_size(crypter), &symmetricKey))
	{
		DBG1(DBG_LIB, "  failed to allocate symmetric encryption key");
		crypter->destroy(crypter);
		return FALSE;
	}
	DBG4(DBG_LIB, "  symmetric encryption key: %B", &symmetricKey);

	if (!get_random(RNG_WEAK, crypter->get_iv_size(crypter), &iv))
	{
		DBG1(DBG_LIB, "  failed to allocate initialization vector");
		crypter->destroy(crypter);
		return FALSE;
	}
	DBG4(DBG_LIB, "  initialization vector: %B", &iv);

	bs = crypter->get_block_size(crypter);
	padding = bs - this->content.len % bs;
	content = chunk_alloc(this->content.len + padding);
	memcpy(content.ptr, this->content.ptr, this->content.len);
	memset(content.ptr + this->content.len, padding, padding);
	DBG3(DBG_LIB, "  padded unencrypted data: %B", &content);

	/* symmetric inline encryption of content */
	if (!crypter->set_key(crypter, symmetricKey) ||
		!crypter->encrypt(crypter, content, iv, NULL))
	{
		crypter->destroy(crypter);
		chunk_clear(&symmetricKey);
		chunk_free(&iv);
		return FALSE;
	}
	crypter->destroy(crypter);
	DBG3(DBG_LIB, "  encrypted data: %B", &content);

	if (!encrypt_key(cert, symmetricKey, &protectedKey))
	{
		DBG1(DBG_LIB, "  encrypting symmetric key failed");
		chunk_clear(&symmetricKey);
		chunk_free(&iv);
		chunk_free(&content);
		return FALSE;
	}
	chunk_clear(&symmetricKey);

	contentEncryptionAlgorithm = asn1_wrap(ASN1_SEQUENCE, "mm",
				asn1_build_known_oid(alg_oid),
				asn1_wrap(ASN1_OCTET_STRING, "m", iv));

	encryptedContentInfo = asn1_wrap(ASN1_SEQUENCE, "mmm",
				asn1_build_known_oid(OID_PKCS7_DATA),
				contentEncryptionAlgorithm,
				asn1_wrap(ASN1_CONTEXT_S_0, "m", content));

	recipientInfo = asn1_wrap(ASN1_SEQUENCE, "cmmm",
				ASN1_INTEGER_0,
				build_issuerAndSerialNumber(cert),
				asn1_algorithmIdentifier(OID_RSA_ENCRYPTION),
				asn1_wrap(ASN1_OCTET_STRING, "m", protectedKey));

	this->encoding = asn1_wrap(ASN1_SEQUENCE, "mm",
		asn1_build_known_oid(OID_PKCS7_ENVELOPED_DATA),
		asn1_wrap(ASN1_CONTEXT_C_0, "m",
			asn1_wrap(ASN1_SEQUENCE, "cmm",
				ASN1_INTEGER_0,
				asn1_wrap(ASN1_SET, "m", recipientInfo),
				encryptedContentInfo)));

	return TRUE;
}

/**
 * See header.
 */
pkcs7_t *pkcs7_enveloped_data_gen(container_type_t type, va_list args)
{
	private_pkcs7_enveloped_data_t *this;
	chunk_t blob = chunk_empty;
	encryption_algorithm_t alg = ENCR_AES_CBC;
	certificate_t *cert = NULL;
	int key_size = 128;

	while (TRUE)
	{
		switch (va_arg(args, builder_part_t))
		{
			case BUILD_CERT:
				cert = va_arg(args, certificate_t*);
				continue;
			case BUILD_ENCRYPTION_ALG:
				alg = va_arg(args, int);
				continue;
			case BUILD_KEY_SIZE:
				key_size = va_arg(args, int);
				continue;
			case BUILD_BLOB:
				blob = va_arg(args, chunk_t);
				continue;
			case BUILD_END:
				break;
			default:
				return NULL;
		}
		break;
	}
	if (blob.len && cert)
	{
		this = create_empty();

		this->content = chunk_clone(blob);
		if (generate(this, cert, alg, key_size))
		{
			return &this->public;
		}
		destroy(this);
	}
	return NULL;
}
