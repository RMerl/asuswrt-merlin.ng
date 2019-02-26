/*
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

#include <daemon.h>
#include <crypto/hashers/hasher.h>
#include <encoding/payloads/cert_payload.h>

#include "certreq_payload.h"

typedef struct private_certreq_payload_t private_certreq_payload_t;

/**
 * Private data of an certreq_payload_t object.
 */
struct private_certreq_payload_t {

	/**
	 * Public certreq_payload_t interface.
	 */
	certreq_payload_t public;

	/**
	 * Next payload type.
	 */
	uint8_t  next_payload;

	/**
	 * Critical flag.
	 */
	bool critical;

	/**
	 * Reserved bits
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
	 * The contained certreq data value.
	 */
	chunk_t data;

	/**
	 * Payload type PLV2_CERTREQ or PLV1_CERTREQ
	 */
	payload_type_t type;
};

/**
 * Encoding rules for CERTREQ payload.
 */
static encoding_rule_t encodings[] = {
	/* 1 Byte next payload type, stored in the field next_payload */
	{ U_INT_8,			offsetof(private_certreq_payload_t, next_payload)	},
	/* the critical bit */
	{ FLAG,				offsetof(private_certreq_payload_t, critical)		},
	/* 7 Bit reserved bits */
	{ RESERVED_BIT,		offsetof(private_certreq_payload_t, reserved[0])	},
	{ RESERVED_BIT,		offsetof(private_certreq_payload_t, reserved[1])	},
	{ RESERVED_BIT,		offsetof(private_certreq_payload_t, reserved[2])	},
	{ RESERVED_BIT,		offsetof(private_certreq_payload_t, reserved[3])	},
	{ RESERVED_BIT,		offsetof(private_certreq_payload_t, reserved[4])	},
	{ RESERVED_BIT,		offsetof(private_certreq_payload_t, reserved[5])	},
	{ RESERVED_BIT,		offsetof(private_certreq_payload_t, reserved[6])	},
	/* Length of the whole payload*/
	{ PAYLOAD_LENGTH,	offsetof(private_certreq_payload_t, payload_length)	},
	/* 1 Byte CERTREQ type*/
	{ U_INT_8,			offsetof(private_certreq_payload_t, encoding)		},
	/* some certreq data bytes, length is defined in PAYLOAD_LENGTH */
	{ CHUNK_DATA,		offsetof(private_certreq_payload_t, data)			}
};

/*
                           1                   2                   3
       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      ! Next Payload  !C!  RESERVED   !         Payload Length        !
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      ! Cert Encoding !                                               !
      +-+-+-+-+-+-+-+-+                                               !
      ~                    Certification Authority                    ~
      !                                                               !
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/

METHOD(payload_t, verify, status_t,
	private_certreq_payload_t *this)
{
	if (this->type == PLV2_CERTREQ &&
		this->encoding == ENC_X509_SIGNATURE)
	{
		if (this->data.len % HASH_SIZE_SHA1)
		{
			DBG1(DBG_ENC, "invalid X509 hash length (%d) in certreq",
				 this->data.len);
			return FAILED;
		}
	}
	return SUCCESS;
}

METHOD(payload_t, get_encoding_rules, int,
	private_certreq_payload_t *this, encoding_rule_t **rules)
{
	*rules = encodings;
	return countof(encodings);
}

METHOD(payload_t, get_header_length, int,
	private_certreq_payload_t *this)
{
	return 5;
}

METHOD(payload_t, get_type, payload_type_t,
	private_certreq_payload_t *this)
{
	return this->type;
}

METHOD(payload_t, get_next_type, payload_type_t,
	private_certreq_payload_t *this)
{
	return this->next_payload;
}

METHOD(payload_t, set_next_type, void,
	private_certreq_payload_t *this, payload_type_t type)
{
	this->next_payload = type;
}

METHOD(payload_t, get_length, size_t,
	private_certreq_payload_t *this)
{
	return this->payload_length;
}

METHOD(certreq_payload_t, get_dn, identification_t*,
	private_certreq_payload_t *this)
{
	if (this->data.len)
	{
		return identification_create_from_encoding(ID_DER_ASN1_DN, this->data);
	}
	return NULL;
}

METHOD(certreq_payload_t, add_keyid, void,
	private_certreq_payload_t *this, chunk_t keyid)
{
	this->data = chunk_cat("mc", this->data, keyid);
	this->payload_length += keyid.len;
}

typedef struct keyid_enumerator_t keyid_enumerator_t;

/**
 * enumerator to enumerate keyids
 */
struct keyid_enumerator_t  {
	enumerator_t public;
	chunk_t full;
	u_char *pos;
};

METHOD(enumerator_t, keyid_enumerate, bool,
	keyid_enumerator_t *this, va_list args)
{
	chunk_t *chunk;

	VA_ARGS_VGET(args, chunk);

	if (this->pos == NULL)
	{
		this->pos = this->full.ptr;
	}
	else
	{
		this->pos += HASH_SIZE_SHA1;
		if (this->pos > (this->full.ptr + this->full.len - HASH_SIZE_SHA1))
		{
			this->pos = NULL;
		}
	}
	if (this->pos)
	{
		chunk->ptr = this->pos;
		chunk->len = HASH_SIZE_SHA1;
		return TRUE;
	}
	return FALSE;
}

METHOD(certreq_payload_t, create_keyid_enumerator, enumerator_t*,
	private_certreq_payload_t *this)
{
	keyid_enumerator_t *enumerator;

	if (this->type == PLV1_CERTREQ)
	{
		return enumerator_create_empty();
	}
	INIT(enumerator,
		.public = {
			.enumerate = enumerator_enumerate_default,
			.venumerate = _keyid_enumerate,
			.destroy = (void*)free,
		},
		.full = this->data,
	);
	return &enumerator->public;
}

METHOD(certreq_payload_t, get_cert_type, certificate_type_t,
	private_certreq_payload_t *this)
{
	switch (this->encoding)
	{
		case ENC_X509_SIGNATURE:
			return CERT_X509;
		default:
			return CERT_ANY;
	}
}

METHOD2(payload_t, certreq_payload_t, destroy, void,
	private_certreq_payload_t *this)
{
	chunk_free(&this->data);
	free(this);
}

/*
 * Described in header
 */
certreq_payload_t *certreq_payload_create(payload_type_t type)
{
	private_certreq_payload_t *this;

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
			.create_keyid_enumerator = _create_keyid_enumerator,
			.get_cert_type = _get_cert_type,
			.add_keyid = _add_keyid,
			.destroy = _destroy,
			.get_dn = _get_dn,
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
certreq_payload_t *certreq_payload_create_type(certificate_type_t type)
{
	private_certreq_payload_t *this;

	this = (private_certreq_payload_t*)
					certreq_payload_create(PLV2_CERTREQ);
	switch (type)
	{
		case CERT_X509:
			this->encoding = ENC_X509_SIGNATURE;
			break;
		default:
			DBG1(DBG_ENC, "certificate type %N not supported in requests",
				 certificate_type_names, type);
			free(this);
			return NULL;
	}
	return &this->public;
}

/*
 * Described in header
 */
certreq_payload_t *certreq_payload_create_dn(identification_t *id)
{
	private_certreq_payload_t *this;

	this = (private_certreq_payload_t*)
					certreq_payload_create(PLV1_CERTREQ);

	this->encoding = ENC_X509_SIGNATURE;
	this->data = chunk_clone(id->get_encoding(id));
	this->payload_length = get_header_length(this) + this->data.len;

	return &this->public;
}
