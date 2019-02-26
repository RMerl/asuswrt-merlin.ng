/*
 * Copyright (C) 2018 Tobias Brunner
 * Copyright (C) 2017-2018 Andreas Steffen
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

#include "tpm_private_key.h"

#include <tpm_tss.h>
#include <utils/debug.h>

typedef struct private_tpm_private_key_t private_tpm_private_key_t;

/**
 * Private data of an tpm_private_key_t object.
 */
struct private_tpm_private_key_t {

	/**
	 * Public tpm_private_key_t interface.
	 */
	tpm_private_key_t public;

	/**
	 * Token keyid used to reference optional PIN for TPM key
	 */
	identification_t *keyid;

	/**
	 * Trusted Platform Module
	 */
	tpm_tss_t *tpm;

	/**
	 * TPM key object handle
	 */
	uint32_t handle;

	/**
	 * Hierarchy the TPM key object is attached to
	 */
	uint32_t hierarchy;

	/**
	 * Associated public key
	 */
	public_key_t *pubkey;

	/**
	 * References to this key
	 */
	refcount_t ref;

};


METHOD(private_key_t, get_type, key_type_t,
	private_tpm_private_key_t *this)
{
	return this->pubkey->get_type(this->pubkey);
}

METHOD(private_key_t, get_keysize, int,
	private_tpm_private_key_t *this)
{
	return this->pubkey->get_keysize(this->pubkey);
}

METHOD(private_key_t, supported_signature_schemes, enumerator_t*,
	private_tpm_private_key_t *this)
{
	return this->tpm->supported_signature_schemes(this->tpm, this->handle);
}

METHOD(private_key_t, sign, bool,
	private_tpm_private_key_t *this, signature_scheme_t scheme, void *params,
	chunk_t data, chunk_t *signature)
{
	chunk_t pin = chunk_empty;
	shared_key_t *shared;
	enumerator_t *enumerator;

	/* check for optional PIN */
	enumerator = lib->credmgr->create_shared_enumerator(lib->credmgr,
										SHARED_PIN, this->keyid, NULL);
	if (enumerator->enumerate(enumerator, &shared, NULL, NULL))
	{
		pin = shared->get_key(shared);
	}
	enumerator->destroy(enumerator);

	return this->tpm->sign(this->tpm, this->hierarchy, this->handle, scheme,
						   params, data, pin, signature);
}

METHOD(private_key_t, decrypt, bool,
	private_tpm_private_key_t *this, encryption_scheme_t scheme,
	chunk_t crypt, chunk_t *plain)
{
	return FALSE;
}

METHOD(private_key_t, get_public_key, public_key_t*,
	private_tpm_private_key_t *this)
{
	return this->pubkey->get_ref(this->pubkey);
}

METHOD(private_key_t, get_fingerprint, bool,
	private_tpm_private_key_t *this, cred_encoding_type_t type,
	chunk_t *fingerprint)
{
	return this->pubkey->get_fingerprint(this->pubkey, type, fingerprint);
}

METHOD(private_key_t, get_encoding, bool,
	private_tpm_private_key_t *this, cred_encoding_type_t type,
	chunk_t *encoding)
{
	return FALSE;
}

METHOD(private_key_t, get_ref, private_key_t*,
	private_tpm_private_key_t *this)
{
	ref_get(&this->ref);
	return &this->public.key;
}

METHOD(private_key_t, destroy, void,
	private_tpm_private_key_t *this)
{
	if (ref_put(&this->ref))
	{
		DESTROY_IF(this->pubkey);
		this->tpm->destroy(this->tpm);
		this->keyid->destroy(this->keyid);
		free(this);
	}
}

/**
 * See header.
 */
tpm_private_key_t *tpm_private_key_connect(key_type_t type, va_list args)
{
	private_tpm_private_key_t *this;
	tpm_tss_t *tpm;
	chunk_t keyid = chunk_empty, pubkey_blob = chunk_empty;
	char handle_str[4];
	size_t len;
	uint32_t hierarchy = 0x4000000B;  /* TPM_RH_ENDORSEMENT */
	uint32_t handle;

	while (TRUE)
	{
		switch (va_arg(args, builder_part_t))
		{
			case BUILD_PKCS11_KEYID:
				keyid = va_arg(args, chunk_t);
				continue;
			case BUILD_PKCS11_SLOT:
				hierarchy = va_arg(args, int);
				continue;
			case BUILD_PKCS11_MODULE:
				va_arg(args, char*);
				continue;
			case BUILD_END:
				break;
			default:
				return NULL;
		}
		break;
	}

	/* convert keyid into 32 bit TPM key object handle */
	if (!keyid.len)
	{
		return NULL;
	}
	len = min(keyid.len, 4);
	memset(handle_str, 0x00, 4);
	memcpy(handle_str + 4 - len, keyid.ptr + keyid.len - len, len);
	handle = untoh32(handle_str);

	/* try to find a TPM 2.0 */
	tpm = tpm_tss_probe(TPM_VERSION_2_0);
	if (!tpm)
	{
		DBG1(DBG_LIB, "no TPM 2.0 found");
		return NULL;
	}

	INIT(this,
		.public = {
			.key = {
				.get_type = _get_type,
				.sign = _sign,
				.decrypt = _decrypt,
				.get_keysize = _get_keysize,
				.supported_signature_schemes = _supported_signature_schemes,
				.get_public_key = _get_public_key,
				.equals = private_key_equals,
				.belongs_to = private_key_belongs_to,
				.get_fingerprint = _get_fingerprint,
				.has_fingerprint = private_key_has_fingerprint,
				.get_encoding = _get_encoding,
				.get_ref = _get_ref,
				.destroy = _destroy,
			},
		},
		.tpm = tpm,
		.keyid = identification_create_from_encoding(ID_KEY_ID, keyid),
		.handle = handle,
		.hierarchy = hierarchy,
		.ref = 1,
	);

	/* get public key from TPM */
	pubkey_blob = tpm->get_public(tpm, handle);
	if (!pubkey_blob.len)
	{
		destroy(this);
		return NULL;
	}
	this->pubkey = lib->creds->create(lib->creds, CRED_PUBLIC_KEY, KEY_ANY,
								BUILD_BLOB_ASN1_DER, pubkey_blob, BUILD_END);
	chunk_free(&pubkey_blob);

	if (!this->pubkey)
	{
		destroy(this);
		return NULL;
	}

	return &this->public;
}
