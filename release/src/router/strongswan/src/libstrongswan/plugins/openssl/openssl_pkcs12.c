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

#define _GNU_SOURCE /* for asprintf() */
#include <stdio.h>
#include <openssl/pkcs12.h>

#include "openssl_pkcs12.h"
#include "openssl_util.h"

#include <library.h>
#include <credentials/sets/mem_cred.h>

typedef struct private_pkcs12_t private_pkcs12_t;

/**
 * Private data of a pkcs12_t object.
 */
struct private_pkcs12_t {

	/**
	 * Public pkcs12_t interface.
	 */
	pkcs12_t public;

	/**
	 * OpenSSL PKCS#12 structure
	 */
	PKCS12 *p12;

	/**
	 * Credentials contained in container
	 */
	mem_cred_t *creds;
};

/**
 * Decode certificate and add it to our credential set
 */
static bool add_cert(private_pkcs12_t *this, X509 *x509)
{
	certificate_t *cert = NULL;
	chunk_t encoding;

	if (!x509)
	{	/* no certificate is ok */
		return TRUE;
	}
	encoding = openssl_i2chunk(X509, x509);
	if (encoding.ptr)
	{
		cert = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_X509,
								  BUILD_BLOB_ASN1_DER, encoding,
								  BUILD_END);
		if (cert)
		{
			this->creds->add_cert(this->creds, FALSE, cert);
		}
	}
	chunk_free(&encoding);
	X509_free(x509);
	return cert != NULL;
}

/**
 * Add CA certificates to our credential set
 */
static bool add_cas(private_pkcs12_t *this, STACK_OF(X509) *cas)
{
	bool success = TRUE;
	int i;

	if (!cas)
	{	/* no CAs is ok */
		return TRUE;
	}
	for (i = 0; i < sk_X509_num(cas); i++)
	{
		if (!add_cert(this, sk_X509_value(cas, i)))
		{	/* continue to free all X509 objects */
			success = FALSE;
		}
	}
	sk_X509_free(cas);
	return success;
}

/**
 * Decode private key and add it to our credential set
 */
static bool add_key(private_pkcs12_t *this, EVP_PKEY *private)
{
	private_key_t *key = NULL;
	chunk_t encoding;
	key_type_t type;

	if (!private)
	{	/* no private key is ok */
		return TRUE;
	}
	switch (EVP_PKEY_base_id(private))
	{
		case EVP_PKEY_RSA:
			type = KEY_RSA;
			break;
		case EVP_PKEY_EC:
			type = KEY_ECDSA;
			break;
		default:
			EVP_PKEY_free(private);
			return FALSE;
	}
	encoding = openssl_i2chunk(PrivateKey, private);
	if (encoding.ptr)
	{
		key = lib->creds->create(lib->creds, CRED_PRIVATE_KEY, type,
								 BUILD_BLOB_ASN1_DER, encoding,
								 BUILD_END);
		if (key)
		{
			this->creds->add_key(this->creds, key);
		}
	}
	chunk_clear(&encoding);
	EVP_PKEY_free(private);
	return key != NULL;
}

/**
 * Decrypt PKCS#12 file and unpack credentials
 */
static bool decrypt_and_unpack(private_pkcs12_t *this)
{
	enumerator_t *enumerator;
	shared_key_t *shared;
	STACK_OF(X509) *cas = NULL;
	EVP_PKEY *private;
	X509 *cert;
	chunk_t key;
	char *password;
	bool success = FALSE;

	enumerator = lib->credmgr->create_shared_enumerator(lib->credmgr,
										SHARED_PRIVATE_KEY_PASS, NULL, NULL);
	while (enumerator->enumerate(enumerator, &shared, NULL, NULL))
	{
		key = shared->get_key(shared);
		if (!key.ptr || asprintf(&password, "%.*s", (int)key.len, key.ptr) < 0)
		{
			password = NULL;
		}
		if (PKCS12_parse(this->p12, password, &private, &cert, &cas))
		{
			success = add_key(this, private);
			success &= add_cert(this, cert);
			success &= add_cas(this, cas);
			free(password);
			break;
		}
		free(password);
	}
	enumerator->destroy(enumerator);
	return success;
}

METHOD(container_t, get_type, container_type_t,
	private_pkcs12_t *this)
{
	return CONTAINER_PKCS12;
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
	if (this->p12)
	{
		PKCS12_free(this->p12);
	}
	this->creds->destroy(this->creds);
	free(this);
}

/**
 * Parse a PKCS#12 container
 */
static pkcs12_t *parse(chunk_t blob)
{
	private_pkcs12_t *this;
	BIO *bio;

	INIT(this,
		.public = {
			.container = {
				.get_type = _get_type,
				.create_signature_enumerator = (void*)enumerator_create_empty,
				.get_data = (void*)return_false,
				.get_encoding = (void*)return_false,
				.destroy = _destroy,
			},
			.create_cert_enumerator = _create_cert_enumerator,
			.create_key_enumerator = _create_key_enumerator,
		},
		.creds = mem_cred_create(),
	);

	bio = BIO_new_mem_buf(blob.ptr, blob.len);
	this->p12 = d2i_PKCS12_bio(bio, NULL);
	BIO_free(bio);

	if (!this->p12 || !decrypt_and_unpack(this))
	{
		destroy(this);
		return NULL;
	}
	return &this->public;
}

/*
 * Defined in header
 */
pkcs12_t *openssl_pkcs12_load(container_type_t type, va_list args)
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
	return blob.len ? parse(blob) : NULL;
}
