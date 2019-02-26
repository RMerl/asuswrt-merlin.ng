/*
 * Copyright (C) 2008 Tobias Brunner
 * Copyright (C) 2005-2010 Martin Willi
 * Copyright (C) 2010 revosec AG
 * Copyright (C) 2005 Jan Hutter
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

#include <stddef.h>
#include <ctype.h>

#include <daemon.h>

#include "cert_payload.h"

ENUM(cert_encoding_names, ENC_PKCS7_WRAPPED_X509, ENC_OCSP_CONTENT,
	"ENC_PKCS7_WRAPPED_X509",
	"ENC_PGP",
	"ENC_DNS_SIGNED_KEY",
	"ENC_X509_SIGNATURE",
	"ENC_X509_KEY_EXCHANGE",
	"ENC_KERBEROS_TOKENS",
	"ENC_CRL",
	"ENC_ARL",
	"ENC_SPKI",
	"ENC_X509_ATTRIBUTE",
	"ENC_RAW_RSA_KEY",
	"ENC_X509_HASH_AND_URL",
	"ENC_X509_HASH_AND_URL_BUNDLE",
	"ENC_OCSP_CONTENT",
);

typedef struct private_cert_payload_t private_cert_payload_t;

/**
 * Private data of an cert_payload_t object.
 */
struct private_cert_payload_t {

	/**
	 * Public cert_payload_t interface.
	 */
	cert_payload_t public;

	/**
	 * Next payload type.
	 */
	uint8_t  next_payload;

	/**
	 * Critical flag.
	 */
	bool critical;

	/**
	 * reserved bits
	 */
	bool reserved[7];

	/**
	 * Length of this payload.
	 */
	uint16_t payload_length;

	/**
	 * Encoding of the CERT Data.
	 */
	uint8_t encoding;

	/**
	 * The contained cert data value.
	 */
	chunk_t data;

	/**
	 * TRUE if the "Hash and URL" data is invalid
	 */
	bool invalid_hash_and_url;

	/**
	 * The payload type.
	 */
	payload_type_t type;
};

/**
 * Encoding rules to parse or generate a CERT payload
 *
 * The defined offsets are the positions in a object of type
 * private_cert_payload_t.
 *
 */
static encoding_rule_t encodings[] = {
	/* 1 Byte next payload type, stored in the field next_payload */
	{ U_INT_8,			offsetof(private_cert_payload_t, next_payload)	},
	/* the critical bit */
	{ FLAG,				offsetof(private_cert_payload_t, critical)		},
	/* 7 Bit reserved bits, nowhere stored */
	{ RESERVED_BIT,		offsetof(private_cert_payload_t, reserved[0])	},
	{ RESERVED_BIT,		offsetof(private_cert_payload_t, reserved[1])	},
	{ RESERVED_BIT,		offsetof(private_cert_payload_t, reserved[2])	},
	{ RESERVED_BIT,		offsetof(private_cert_payload_t, reserved[3])	},
	{ RESERVED_BIT,		offsetof(private_cert_payload_t, reserved[4])	},
	{ RESERVED_BIT,		offsetof(private_cert_payload_t, reserved[5])	},
	{ RESERVED_BIT,		offsetof(private_cert_payload_t, reserved[6])	},
	/* Length of the whole payload*/
	{ PAYLOAD_LENGTH,	offsetof(private_cert_payload_t, payload_length)},
	/* 1 Byte CERT type*/
	{ U_INT_8,			offsetof(private_cert_payload_t, encoding)		},
	/* some cert data bytes, length is defined in PAYLOAD_LENGTH */
	{ CHUNK_DATA,		offsetof(private_cert_payload_t, data)			}
};

/*
                           1                   2                   3
       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      ! Next Payload  !C!  RESERVED   !         Payload Length        !
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      ! Cert Encoding !                                               !
      +-+-+-+-+-+-+-+-+                                               !
      ~                       Certificate Data                        ~
      !                                                               !
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/

METHOD(payload_t, verify, status_t,
	private_cert_payload_t *this)
{
	if (this->encoding == ENC_X509_HASH_AND_URL ||
		this->encoding == ENC_X509_HASH_AND_URL_BUNDLE)
	{
		int i;

		/* coarse verification of "Hash and URL" encoded certificates */
		if (this->data.len <= 20)
		{
			DBG1(DBG_ENC, "invalid payload length for hash-and-url (%d), ignore",
				 this->data.len);
			this->invalid_hash_and_url = TRUE;
			return SUCCESS;
		}
		for (i = 20; i < this->data.len; ++i)
		{
			if (this->data.ptr[i] == '\0')
			{
				/* null terminated, fine */
				return SUCCESS;
			}
			else if (!isprint(this->data.ptr[i]))
			{
				DBG1(DBG_ENC, "non printable characters in url of hash-and-url"
					 " encoded certificate payload, ignore");
				this->invalid_hash_and_url = TRUE;
				return SUCCESS;
			}
		}
		/* URL is not null terminated, correct that */
		this->data = chunk_cat("mc", this->data, chunk_from_chars(0));
	}
	return SUCCESS;
}

METHOD(payload_t, get_encoding_rules, int,
	private_cert_payload_t *this, encoding_rule_t **rules)
{
	*rules = encodings;
	return countof(encodings);
}

METHOD(payload_t, get_header_length, int,
	private_cert_payload_t *this)
{
	return 5;
}

METHOD(payload_t, get_type, payload_type_t,
	private_cert_payload_t *this)
{
	return this->type;
}

METHOD(payload_t, get_next_type, payload_type_t,
	private_cert_payload_t *this)
{
	return this->next_payload;
}

METHOD(payload_t, set_next_type, void,
	private_cert_payload_t *this, payload_type_t type)
{
	this->next_payload = type;
}

METHOD(payload_t, get_length, size_t,
	private_cert_payload_t *this)
{
	return this->payload_length;
}

METHOD(cert_payload_t, get_cert_encoding, cert_encoding_t,
	private_cert_payload_t *this)
{
	return this->encoding;
}

METHOD(cert_payload_t, get_cert, certificate_t*,
	private_cert_payload_t *this)
{
	int type;

	switch (this->encoding)
	{
		case ENC_X509_SIGNATURE:
			type = CERT_X509;
			break;
		case ENC_X509_ATTRIBUTE:
			type = CERT_X509_AC;
			break;
		case ENC_CRL:
			type = CERT_X509_CRL;
			break;
		default:
			return NULL;
	}
	return lib->creds->create(lib->creds, CRED_CERTIFICATE, type,
							  BUILD_BLOB_ASN1_DER, this->data, BUILD_END);
}

METHOD(cert_payload_t, get_container, container_t*,
	private_cert_payload_t *this)
{
	int type;

	switch (this->encoding)
	{
		case ENC_PKCS7_WRAPPED_X509:
			type = CONTAINER_PKCS7;
			break;
		default:
			return NULL;
	}
	return lib->creds->create(lib->creds, CRED_CONTAINER, type,
							  BUILD_BLOB_ASN1_DER, this->data, BUILD_END);
}

METHOD(cert_payload_t, get_hash, chunk_t,
	private_cert_payload_t *this)
{
	chunk_t hash = chunk_empty;

	if ((this->encoding != ENC_X509_HASH_AND_URL &&
		 this->encoding != ENC_X509_HASH_AND_URL_BUNDLE) ||
		this->invalid_hash_and_url)
	{
		return hash;
	}
	hash.ptr = this->data.ptr;
	hash.len = 20;
	return hash;
}

METHOD(cert_payload_t, get_url, char*,
	private_cert_payload_t *this)
{
	if ((this->encoding != ENC_X509_HASH_AND_URL &&
		 this->encoding != ENC_X509_HASH_AND_URL_BUNDLE) ||
		this->invalid_hash_and_url)
	{
		return NULL;
	}
	return (char*)this->data.ptr + 20;
}

METHOD2(payload_t, cert_payload_t, destroy, void,
	private_cert_payload_t *this)
{
	free(this->data.ptr);
	free(this);
}

/*
 * Described in header
 */
cert_payload_t *cert_payload_create(payload_type_t type)
{
	private_cert_payload_t *this;

	INIT(this,
		.public = {
			.payload_interface = {
				.verify = _verify,
				.get_encoding_rules = _get_encoding_rules,
				.get_header_length = _get_header_length,
				.get_length = _get_length,
				.get_next_type = _get_next_type,
				.set_next_type = _set_next_type,
				.get_type = _get_type,
				.destroy = _destroy,
			},
			.get_cert = _get_cert,
			.get_container = _get_container,
			.get_cert_encoding = _get_cert_encoding,
			.get_hash = _get_hash,
			.get_url = _get_url,
			.destroy = _destroy,
		},
		.next_payload = PL_NONE,
		.payload_length = get_header_length(this),
		.type = type,
	);
	return &this->public;
}

/*
 * Described in header
 */
cert_payload_t *cert_payload_create_from_cert(payload_type_t type,
											  certificate_t *cert)
{
	private_cert_payload_t *this;

	this = (private_cert_payload_t*)cert_payload_create(type);
	switch (cert->get_type(cert))
	{
		case CERT_X509:
			this->encoding = ENC_X509_SIGNATURE;
			break;
		case CERT_X509_AC:
			this->encoding = ENC_X509_ATTRIBUTE;
			break;
		default:
			DBG1(DBG_ENC, "embedding %N certificate in payload failed",
				 certificate_type_names, cert->get_type(cert));
			free(this);
			return NULL;
	}
	if (!cert->get_encoding(cert, CERT_ASN1_DER, &this->data))
	{
		DBG1(DBG_ENC, "encoding certificate for cert payload failed");
		free(this);
		return NULL;
	}
	this->payload_length = get_header_length(this) + this->data.len;

	return &this->public;
}

/*
 * Described in header
 */
cert_payload_t *cert_payload_create_from_hash_and_url(chunk_t hash, char *url)
{
	private_cert_payload_t *this;

	this = (private_cert_payload_t*)cert_payload_create(PLV2_CERTIFICATE);
	this->encoding = ENC_X509_HASH_AND_URL;
	this->data = chunk_cat("cc", hash, chunk_create(url, strlen(url)));
	this->payload_length = get_header_length(this) + this->data.len;

	return &this->public;
}

/*
 * Described in header
 */
cert_payload_t *cert_payload_create_custom(payload_type_t type,
										cert_encoding_t encoding, chunk_t data)
{
	private_cert_payload_t *this;

	this = (private_cert_payload_t*)cert_payload_create(type);
	this->encoding = encoding;
	this->data = data;
	this->payload_length = get_header_length(this) + this->data.len;

	return &this->public;
}
