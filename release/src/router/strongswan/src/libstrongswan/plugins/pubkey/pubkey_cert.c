/*
 * Copyright (C) 2008 Martin Willi
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

#include "pubkey_cert.h"

#include <time.h>

#include <utils/debug.h>

typedef struct private_pubkey_cert_t private_pubkey_cert_t;

/**
 * private data of pubkey_cert
 */
struct private_pubkey_cert_t {

	/**
	 * public functions
	 */
	pubkey_cert_t public;

	/**
	 * wrapped public key
	 */
	public_key_t *key;

	/**
	 * dummy issuer id, ID_ANY
	 */
	identification_t *issuer;

	/**
	 * subject, ID_KEY_ID of the public key
	 */
	identification_t *subject;

	/**
	 * key inception time
	 */
	time_t notBefore;

	/**
	 * key expiration time
	 */
	time_t notAfter;

	/**
	 * reference count
	 */
	refcount_t ref;
};

METHOD(certificate_t, get_type, certificate_type_t,
	private_pubkey_cert_t *this)
{
	return CERT_TRUSTED_PUBKEY;
}

METHOD(certificate_t, get_subject, identification_t*,
	private_pubkey_cert_t *this)
{
	return this->subject;
}

METHOD(certificate_t, get_issuer, identification_t*,
	private_pubkey_cert_t *this)
{
	return this->issuer;
}

METHOD(certificate_t, has_subject, id_match_t,
	private_pubkey_cert_t *this, identification_t *subject)
{
	if (subject->get_type(subject) == ID_KEY_ID)
	{
		cred_encoding_type_t type;
		chunk_t fingerprint;

		for (type = 0; type < CRED_ENCODING_MAX; type++)
		{
			if (this->key->get_fingerprint(this->key, type, &fingerprint) &&
				chunk_equals(fingerprint, subject->get_encoding(subject)))
			{
				return ID_MATCH_PERFECT;
			}
		}
	}

	return this->subject->matches(this->subject, subject);
}

METHOD(certificate_t, has_issuer, id_match_t,
	private_pubkey_cert_t *this, identification_t *issuer)
{
	return ID_MATCH_NONE;
}

METHOD(certificate_t, equals, bool,
	private_pubkey_cert_t *this, certificate_t *other)
{
	identification_t *other_subject;
	public_key_t *other_key;

	if (this == (private_pubkey_cert_t*)other)
	{
		return TRUE;
	}
	if (other->get_type(other) != CERT_TRUSTED_PUBKEY)
	{
		return FALSE;
	}
	other_key = other->get_public_key(other);
	if (other_key)
	{
		if (public_key_equals(this->key, other_key))
		{
			other_key->destroy(other_key);
			other_subject = other->get_subject(other);
			return other_subject->equals(other_subject, this->subject);
		}
		other_key->destroy(other_key);
	}
	return FALSE;
}

METHOD(certificate_t, issued_by, bool,
	private_pubkey_cert_t *this, certificate_t *issuer,
	signature_params_t **scheme)
{
	bool valid = equals(this, issuer);
	if (valid && scheme)
	{
		INIT(*scheme,
			.scheme = SIGN_UNKNOWN,
		);
	}
	return valid;
}

METHOD(certificate_t, get_public_key,  public_key_t*,
	private_pubkey_cert_t *this)
{
	this->key->get_ref(this->key);
	return this->key;
}

METHOD(certificate_t, get_validity, bool,
	private_pubkey_cert_t *this, time_t *when, time_t *not_before,
	time_t *not_after)
{
	time_t t = when ? *when : time(NULL);

	if (not_before)
	{
		*not_before = this->notBefore;
	}
	if (not_after)
	{
		*not_after = this->notAfter;
	}
	return ((this->notBefore == UNDEFINED_TIME || t >= this->notBefore) &&
			(this->notAfter  == UNDEFINED_TIME || t <= this->notAfter));
}

METHOD(certificate_t, get_encoding, bool,
	private_pubkey_cert_t *this, cred_encoding_type_t type, chunk_t *encoding)
{
	return this->key->get_encoding(this->key, type, encoding);
}

METHOD(certificate_t, get_ref, certificate_t*,
	private_pubkey_cert_t *this)
{
	ref_get(&this->ref);
	return &this->public.interface;
}

METHOD(certificate_t, destroy, void,
	private_pubkey_cert_t *this)
{
	if (ref_put(&this->ref))
	{
		this->subject->destroy(this->subject);
		this->issuer->destroy(this->issuer);
		this->key->destroy(this->key);
		free(this);
	}
}

METHOD(pubkey_cert_t, set_subject, void,
	private_pubkey_cert_t *this, identification_t *subject)
{
	DESTROY_IF(this->subject);
	this->subject = subject->clone(subject);
}

/*
 * see header file
 */
static pubkey_cert_t *pubkey_cert_create(public_key_t *key,
										 time_t notBefore, time_t notAfter,
										 identification_t *subject)
{
	private_pubkey_cert_t *this;
	chunk_t fingerprint;

	INIT(this,
		.public = {
			.interface = {
				.get_type = _get_type,
				.get_subject = _get_subject,
				.get_issuer = _get_issuer,
				.has_subject = _has_subject,
				.has_issuer = _has_issuer,
				.issued_by = _issued_by,
				.get_public_key = _get_public_key,
				.get_validity = _get_validity,
				.get_encoding = _get_encoding,
				.equals = _equals,
				.get_ref = _get_ref,
				.destroy = _destroy,
			},
			.set_subject = _set_subject,
		},
		.ref = 1,
		.key = key,
		.notBefore = notBefore,
		.notAfter = notAfter,
		.issuer = identification_create_from_encoding(ID_ANY, chunk_empty),
	);

	if (subject)
	{
		this->subject = subject->clone(subject);
	}
	else if (key->get_fingerprint(key, KEYID_PUBKEY_INFO_SHA1, &fingerprint))
	{
		this->subject = identification_create_from_encoding(ID_KEY_ID, fingerprint);
	}
	else
	{
		this->subject = identification_create_from_encoding(ID_ANY, chunk_empty);
	}

	return &this->public;
}

/**
 * See header.
 */
pubkey_cert_t *pubkey_cert_wrap(certificate_type_t type, va_list args)
{
	public_key_t *key = NULL;
	chunk_t blob = chunk_empty;
	identification_t *subject = NULL;
	time_t notBefore = UNDEFINED_TIME, notAfter = UNDEFINED_TIME;

	while (TRUE)
	{
		switch (va_arg(args, builder_part_t))
		{
			case BUILD_BLOB_ASN1_DER:
				blob = va_arg(args, chunk_t);
				continue;
			case BUILD_PUBLIC_KEY:
				key = va_arg(args, public_key_t*);
				continue;
			case BUILD_NOT_BEFORE_TIME:
				notBefore = va_arg(args, time_t);
				continue;
			case BUILD_NOT_AFTER_TIME:
				notAfter = va_arg(args, time_t);
				continue;
			case BUILD_SUBJECT:
				subject = va_arg(args, identification_t*);
				continue;
			case BUILD_END:
				break;
			default:
				return NULL;
		}
		break;
	}
	if (key)
	{
		key->get_ref(key);
	}
	else if (blob.ptr)
	{
		key = lib->creds->create(lib->creds, CRED_PUBLIC_KEY, KEY_ANY,
								 BUILD_BLOB_ASN1_DER, blob, BUILD_END);
	}
	if (key)
	{
		return pubkey_cert_create(key, notBefore, notAfter, subject);
	}
	return NULL;
}

