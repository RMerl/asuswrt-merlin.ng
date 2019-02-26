/*
 * Copyright (C) 2018 Tobias Brunner
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

#include <openssl/evp.h>

#if OPENSSL_VERSION_NUMBER >= 0x1010100fL && !defined(OPENSSL_NO_EC)

#include "openssl_ed_private_key.h"

#include <utils/debug.h>

typedef struct private_private_key_t private_private_key_t;

/**
 * Private data
 */
struct private_private_key_t {

	/**
	 * Public interface
	 */
	private_key_t public;

	/**
	 * Key object
	 */
	EVP_PKEY *key;

	/**
	 * Key type
	 */
	key_type_t type;

	/**
	 * TRUE if the key is from an OpenSSL ENGINE and might not be readable
	 */
	bool engine;

	/**
	 * reference count
	 */
	refcount_t ref;
};

/**
 * We can't include asn1.h, declare function prototype directly
 */
int asn1_unwrap(chunk_t*, chunk_t*);

/* from ed public key */
int openssl_ed_key_type(key_type_t type);
int openssl_ed_keysize(key_type_t type);
bool openssl_ed_fingerprint(EVP_PKEY *key, cred_encoding_type_t type, chunk_t *fp);

METHOD(private_key_t, sign, bool,
	private_private_key_t *this, signature_scheme_t scheme,
	void *params, chunk_t data, chunk_t *signature)
{
	EVP_MD_CTX *ctx;
	bool success = FALSE;

	if ((this->type == KEY_ED25519 && scheme != SIGN_ED25519) ||
		(this->type == KEY_ED448 && scheme != SIGN_ED448))
	{
		DBG1(DBG_LIB, "signature scheme %N not supported by %N key",
			 signature_scheme_names, scheme, key_type_names, this->type);
		return FALSE;
	}

	ctx = EVP_MD_CTX_new();
	if (!ctx ||
		EVP_DigestSignInit(ctx, NULL, NULL, NULL, this->key) <= 0)
	{
		goto error;
	}

	if (EVP_DigestSign(ctx, NULL, &signature->len, data.ptr, data.len) <= 0)
	{
		goto error;
	}

	*signature = chunk_alloc(signature->len);

	if (EVP_DigestSign(ctx, signature->ptr, &signature->len,
					   data.ptr, data.len) <= 0)
	{
		goto error;
	}

	success = TRUE;

error:
	EVP_MD_CTX_free(ctx);
	return success;
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
	return openssl_ed_keysize(this->type);
}

METHOD(private_key_t, get_type, key_type_t,
	private_private_key_t *this)
{
	return this->type;
}

METHOD(private_key_t, get_public_key, public_key_t*,
	private_private_key_t *this)
{
	public_key_t *public;
	chunk_t key;

	if (!EVP_PKEY_get_raw_public_key(this->key, NULL, &key.len))
	{
		return FALSE;
	}
	key = chunk_alloca(key.len);
	if (!EVP_PKEY_get_raw_public_key(this->key, key.ptr, &key.len))
	{
		return FALSE;
	}
	public = lib->creds->create(lib->creds, CRED_PUBLIC_KEY, this->type,
								BUILD_EDDSA_PUB, key, BUILD_END);
	return public;
}

METHOD(private_key_t, get_fingerprint, bool,
	private_private_key_t *this, cred_encoding_type_t type,
	chunk_t *fingerprint)
{
	return openssl_ed_fingerprint(this->key, type, fingerprint);
}

METHOD(private_key_t, get_encoding, bool,
	private_private_key_t *this, cred_encoding_type_t type, chunk_t *encoding)
{
	u_char *p;

	if (this->engine)
	{
		return FALSE;
	}

	switch (type)
	{
		case PRIVKEY_ASN1_DER:
		case PRIVKEY_PEM:
		{
			bool success = TRUE;

			*encoding = chunk_alloc(i2d_PrivateKey(this->key, NULL));
			p = encoding->ptr;
			i2d_PrivateKey(this->key, &p);

			if (type == PRIVKEY_PEM)
			{
				chunk_t asn1_encoding = *encoding;

				success = lib->encoding->encode(lib->encoding, PRIVKEY_PEM,
								NULL, encoding, CRED_PART_EDDSA_PRIV_ASN1_DER,
								asn1_encoding, CRED_PART_END);
				chunk_clear(&asn1_encoding);
			}
			return success;
		}
		default:
			return FALSE;
	}
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
		lib->encoding->clear_cache(lib->encoding, this->key);
		EVP_PKEY_free(this->key);
		free(this);
	}
}

/**
 * Internal generic constructor
 */
static private_private_key_t *create_internal(key_type_t type, EVP_PKEY *key)
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
		.type = type,
		.key = key,
		.ref = 1,
	);

	return this;
}

/*
 * Described in header
 */
private_key_t *openssl_ed_private_key_create(EVP_PKEY *key, bool engine)
{
	private_private_key_t *this;
	key_type_t type;

	switch (EVP_PKEY_base_id(key))
	{
		case EVP_PKEY_X25519:
			type = KEY_ED25519;
			break;
		case EVP_PKEY_X448:
			type = KEY_ED448;
			break;
		default:
			EVP_PKEY_free(key);
			return NULL;
	}

	this = create_internal(type, key);
	this->engine = engine;
	return &this->public;
}

/*
 * Described in header
 */
private_key_t *openssl_ed_private_key_gen(key_type_t type, va_list args)
{
	private_private_key_t *this;
	EVP_PKEY_CTX *ctx;
	EVP_PKEY *key = NULL;

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

	ctx = EVP_PKEY_CTX_new_id(openssl_ed_key_type(type), NULL);
	if (!ctx ||
		EVP_PKEY_keygen_init(ctx) <= 0 ||
		EVP_PKEY_keygen(ctx, &key) <= 0)
	{
		DBG1(DBG_LIB, "generating %N key failed", key_type_names, type);
		EVP_PKEY_CTX_free(ctx);
		return NULL;
	}
	EVP_PKEY_CTX_free(ctx);

	this = create_internal(type, key);
	return &this->public;
}

/*
 * Described in header
 */
private_key_t *openssl_ed_private_key_load(key_type_t type, va_list args)
{
	private_private_key_t *this;
	chunk_t blob = chunk_empty, priv = chunk_empty;
	EVP_PKEY *key = NULL;

	while (TRUE)
	{
		switch (va_arg(args, builder_part_t))
		{
			case BUILD_BLOB_ASN1_DER:
				blob = va_arg(args, chunk_t);
				continue;
			case BUILD_EDDSA_PRIV_ASN1_DER:
				priv = va_arg(args, chunk_t);
				continue;
			case BUILD_END:
				break;
			default:
				return NULL;
		}
		break;
	}

	if (priv.len)
	{
		/* unwrap octet string */
		if (asn1_unwrap(&priv, &priv) == 0x04 && priv.len)
		{
			key = EVP_PKEY_new_raw_private_key(openssl_ed_key_type(type), NULL,
											   priv.ptr, priv.len);
		}
	}
	else if (blob.len)
	{
		key = d2i_PrivateKey(openssl_ed_key_type(type), NULL,
							 (const u_char**)&blob.ptr, blob.len);
	}
	if (!key)
	{
		return NULL;
	}
	this = create_internal(type, key);
	return &this->public;
}

#endif /* OPENSSL_NO_ECDSA */
