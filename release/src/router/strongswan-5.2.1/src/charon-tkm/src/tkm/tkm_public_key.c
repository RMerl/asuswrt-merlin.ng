/*
 * Copyright (C) 2012-2013 Reto Buerki
 * Copyright (C) 2012-2013 Adrian-Ken Rueegsegger
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

#include <utils/debug.h>

#include "tkm_public_key.h"

typedef struct private_tkm_public_key_t private_tkm_public_key_t;

/**
 * Private data of tkm_public_key_t object.
 */
struct private_tkm_public_key_t {

	/**
	 * Public interface for this signer.
	 */
	tkm_public_key_t public;

	/**
	 * ASN.1 blob of pubkey.
	 */
	chunk_t asn_blob;

	/**
	 * Key type.
	 */
	key_type_t key_type;

	/**
	 * Reference count.
	 */
	refcount_t ref;
};

METHOD(public_key_t, get_type, key_type_t,
	private_tkm_public_key_t *this)
{
	return this->key_type;
}

METHOD(public_key_t, verify, bool,
	private_tkm_public_key_t *this, signature_scheme_t scheme,
	chunk_t data, chunk_t signature)
{
	return TRUE;
}

METHOD(public_key_t, encrypt_, bool,
	private_tkm_public_key_t *this, encryption_scheme_t scheme,
	chunk_t plain, chunk_t *crypto)
{
	return FALSE;
}

METHOD(public_key_t, get_keysize, int,
	private_tkm_public_key_t *this)
{
	return 0;
}

METHOD(public_key_t, get_encoding, bool,
	private_tkm_public_key_t *this, cred_encoding_type_t type,
	chunk_t *encoding)
{
	return NULL;
}

METHOD(public_key_t, get_fingerprint, bool,
	private_tkm_public_key_t *this, cred_encoding_type_t type, chunk_t *fp)
{
	if (lib->encoding->get_cache(lib->encoding, type, this, fp))
	{
		return TRUE;
	}
	switch(this->key_type)
	{
		case KEY_RSA:
			return lib->encoding->encode(lib->encoding, type, this, fp,
										 CRED_PART_RSA_PUB_ASN1_DER,
										 this->asn_blob, CRED_PART_END);
		default:
			DBG1(DBG_LIB, "%N public key not supported, fingerprinting failed",
				 key_type_names, this->key_type);
			return FALSE;
	}
}

METHOD(public_key_t, get_ref, public_key_t*,
	private_tkm_public_key_t *this)
{
	ref_get(&this->ref);
	return &this->public.key;
}

METHOD(public_key_t, destroy, void,
	private_tkm_public_key_t *this)
{
	if (ref_put(&this->ref))
	{
		lib->encoding->clear_cache(lib->encoding, this);
		chunk_free(&this->asn_blob);
		free(this);
	}
}

/**
 * See header.
 */
tkm_public_key_t *tkm_public_key_load(key_type_t type, va_list args)
{
	private_tkm_public_key_t *this;
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

	if (!blob.ptr)
	{
		return NULL;
	}

	INIT(this,
		.public = {
			.key = {
				.get_type = _get_type,
				.verify = _verify,
				.encrypt = _encrypt_,
				.equals = public_key_equals,
				.get_keysize = _get_keysize,
				.get_fingerprint = _get_fingerprint,
				.has_fingerprint = public_key_has_fingerprint,
				.get_encoding = _get_encoding,
				.get_ref = _get_ref,
				.destroy = _destroy,
			},
		},
		.ref = 1,
		.asn_blob = chunk_clone(blob),
		.key_type = type,
	);

	return &this->public;
}
