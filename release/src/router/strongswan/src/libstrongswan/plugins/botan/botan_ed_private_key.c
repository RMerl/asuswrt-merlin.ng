/*
 * Copyright (C) 2018 Tobias Brunner
 * HSR Hochschule fuer Technik Rapperswil
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "botan_ed_private_key.h"
#include "botan_ed_public_key.h"
#include "botan_util.h"

#include <botan/build.h>

#ifdef BOTAN_HAS_ED25519

#include <asn1/asn1.h>
#include <utils/debug.h>

typedef struct private_private_key_t private_private_key_t;

#define ED25519_KEY_LEN 32

/**
 * Private data
 */
struct private_private_key_t {

	/**
	 * Public interface
	 */
	private_key_t public;

	/**
	 * Botan private key object
	 */
	botan_privkey_t key;

	/**
	 * Reference count
	 */
	refcount_t ref;
};

METHOD(private_key_t, sign, bool,
	private_private_key_t *this, signature_scheme_t scheme,
	void *params, chunk_t data, chunk_t *signature)
{
	switch (scheme)
	{
		case SIGN_ED25519:
			return botan_get_signature(this->key, "Pure", data, signature);
		default:
			DBG1(DBG_LIB, "signature scheme %N not supported via botan",
				 signature_scheme_names, scheme);
			return FALSE;
	}
}

METHOD(private_key_t, decrypt, bool,
	private_private_key_t *this, encryption_scheme_t scheme,
	chunk_t crypto, chunk_t *plain)
{
	DBG1(DBG_LIB, "EdDSA private key decryption not implemented");
	return FALSE;
}

METHOD(private_key_t, get_keysize, int,
	private_private_key_t *this)
{
	return ED25519_KEY_LEN * 8;
}

METHOD(private_key_t, get_type, key_type_t,
	private_private_key_t *this)
{
	return KEY_ED25519;
}

METHOD(private_key_t, get_public_key, public_key_t*,
	private_private_key_t *this)
{
	botan_pubkey_t pubkey;

	if (botan_privkey_export_pubkey(&pubkey, this->key))
	{
		return NULL;
	}
	return botan_ed_public_key_adopt(pubkey);
}

METHOD(private_key_t, get_fingerprint, bool,
	private_private_key_t *this, cred_encoding_type_t type,
	chunk_t *fingerprint)
{
	botan_pubkey_t pubkey;
	bool success = FALSE;

	/* check the cache before doing the export */
	if (lib->encoding->get_cache(lib->encoding, type, this, fingerprint))
	{
		return TRUE;
	}

	if (botan_privkey_export_pubkey(&pubkey, this->key))
	{
		return FALSE;
	}
	success = botan_get_fingerprint(pubkey, this, type, fingerprint);
	botan_pubkey_destroy(pubkey);
	return success;
}

METHOD(private_key_t, get_encoding, bool,
	private_private_key_t *this, cred_encoding_type_t type,
	chunk_t *encoding)
{
	return botan_get_privkey_encoding(this->key, type, encoding);
}

METHOD(private_key_t, get_ref, private_key_t*,
	private_private_key_t *this)
{
	ref_get(&this->ref);
	return &this->public;
}

METHOD(private_key_t, destroy, void,
	private_private_key_t *this)
{
	if (ref_put(&this->ref))
	{
		lib->encoding->clear_cache(lib->encoding, this);
		botan_privkey_destroy(this->key);
		free(this);
	}
}

/**
 * Internal generic constructor
 */
static private_private_key_t *create_empty()
{
	private_private_key_t *this;

	INIT(this,
		.public = {
			.get_type = _get_type,
			.sign = _sign,
			.decrypt = _decrypt,
			.get_keysize = _get_keysize,
			.get_public_key = _get_public_key,
			.equals = private_key_equals,
			.belongs_to = private_key_belongs_to,
			.get_fingerprint = _get_fingerprint,
			.has_fingerprint = private_key_has_fingerprint,
			.get_encoding = _get_encoding,
			.get_ref = _get_ref,
			.destroy = _destroy,
		},
		.ref = 1,
	);

	return this;
}

/*
 * Described in header
 */
private_key_t *botan_ed_private_key_adopt(botan_privkey_t key)
{
	private_private_key_t *this;

	this = create_empty();
	this->key = key;

	return &this->public;
}

/*
 * Described in header
 */
private_key_t *botan_ed_private_key_gen(key_type_t type, va_list args)
{
	private_private_key_t *this;
	botan_rng_t rng;

	while (TRUE)
	{
		switch (va_arg(args, builder_part_t))
		{
			case BUILD_KEY_SIZE:
				/* just ignore the key size */
				va_arg(args, u_int);
				continue;
			case BUILD_END:
				break;
			default:
				return NULL;
		}
		break;
	}

	if (botan_rng_init(&rng, "system"))
	{
		return NULL;
	}

	this = create_empty();

	if (botan_privkey_create(&this->key, "Ed25519", NULL, rng))
	{
		DBG1(DBG_LIB, "EdDSA private key generation failed");
		botan_rng_destroy(rng);
		free(this);
		return NULL;
	}

	botan_rng_destroy(rng);
	return &this->public;
}

/*
 * Described in header
 */
private_key_t *botan_ed_private_key_load(key_type_t type, va_list args)
{
	private_private_key_t *this;
	chunk_t key = chunk_empty;

	while (TRUE)
	{
		switch (va_arg(args, builder_part_t))
		{
			case BUILD_EDDSA_PRIV_ASN1_DER:
				key = va_arg(args, chunk_t);
				continue;
			case BUILD_END:
				break;
			default:
				return NULL;
		}
		break;
	}

	/* PKCS#8-encoded keys are handled generically, so we only handle the
	 * explicit case */
	if (asn1_unwrap(&key, &key) != ASN1_OCTET_STRING ||
		key.len != ED25519_KEY_LEN)
	{
		return NULL;
	}

	this = create_empty();

	if (botan_privkey_load_ed25519(&this->key, key.ptr))
	{
		free(this);
		return NULL;
	}
	return &this->public;
}

#endif
