/*
 * Copyright (C) 2009 Martin Willi
 * Copyright (C) 2008 Tobias Brunner
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

#include <openssl/opensslconf.h>

#ifndef OPENSSL_NO_RSA

#include "openssl_rsa_private_key.h"
#include "openssl_rsa_public_key.h"

#include <utils/debug.h>

#include <openssl/evp.h>
#include <openssl/rsa.h>
#ifndef OPENSSL_NO_ENGINE
#include <openssl/engine.h>
#endif /* OPENSSL_NO_ENGINE */

/**
 *  Public exponent to use for key generation.
 */
#define PUBLIC_EXPONENT 0x10001

typedef struct private_openssl_rsa_private_key_t private_openssl_rsa_private_key_t;

/**
 * Private data of a openssl_rsa_private_key_t object.
 */
struct private_openssl_rsa_private_key_t {
	/**
	 * Public interface for this signer.
	 */
	openssl_rsa_private_key_t public;

	/**
	 * RSA object from OpenSSL
	 */
	RSA *rsa;

	/**
	 * TRUE if the key is from an OpenSSL ENGINE and might not be readable
	 */
	bool engine;

	/**
	 * reference count
	 */
	refcount_t ref;
};

/* implemented in rsa public key */
bool openssl_rsa_fingerprint(RSA *rsa, cred_encoding_type_t type, chunk_t *fp);

/**
 * Build an EMPSA PKCS1 signature described in PKCS#1
 */
static bool build_emsa_pkcs1_signature(private_openssl_rsa_private_key_t *this,
									   int type, chunk_t data, chunk_t *sig)
{
	bool success = FALSE;

	*sig = chunk_alloc(RSA_size(this->rsa));

	if (type == NID_undef)
	{
		if (RSA_private_encrypt(data.len, data.ptr, sig->ptr, this->rsa,
								RSA_PKCS1_PADDING) == sig->len)
		{
			success = TRUE;
		}
	}
	else
	{
		EVP_MD_CTX *ctx;
		EVP_PKEY *key;
		const EVP_MD *hasher;
		u_int len;

		hasher = EVP_get_digestbynid(type);
		if (!hasher)
		{
			return FALSE;
		}

		ctx = EVP_MD_CTX_create();
		key = EVP_PKEY_new();
		if (!ctx || !key)
		{
			goto error;
		}
		if (!EVP_PKEY_set1_RSA(key, this->rsa))
		{
			goto error;
		}
		if (!EVP_SignInit_ex(ctx, hasher, NULL))
		{
			goto error;
		}
		if (!EVP_SignUpdate(ctx, data.ptr, data.len))
		{
			goto error;
		}
		if (EVP_SignFinal(ctx, sig->ptr, &len, key))
		{
			success = TRUE;
		}

error:
		if (key)
		{
			EVP_PKEY_free(key);
		}
		if (ctx)
		{
			EVP_MD_CTX_destroy(ctx);
		}
	}
	if (!success)
	{
		free(sig->ptr);
	}
	return success;
}


METHOD(private_key_t, get_type, key_type_t,
	private_openssl_rsa_private_key_t *this)
{
	return KEY_RSA;
}

METHOD(private_key_t, sign, bool,
	private_openssl_rsa_private_key_t *this, signature_scheme_t scheme,
	chunk_t data, chunk_t *signature)
{
	switch (scheme)
	{
		case SIGN_RSA_EMSA_PKCS1_NULL:
			return build_emsa_pkcs1_signature(this, NID_undef, data, signature);
		case SIGN_RSA_EMSA_PKCS1_SHA1:
			return build_emsa_pkcs1_signature(this, NID_sha1, data, signature);
		case SIGN_RSA_EMSA_PKCS1_SHA224:
			return build_emsa_pkcs1_signature(this, NID_sha224, data, signature);
		case SIGN_RSA_EMSA_PKCS1_SHA256:
			return build_emsa_pkcs1_signature(this, NID_sha256, data, signature);
		case SIGN_RSA_EMSA_PKCS1_SHA384:
			return build_emsa_pkcs1_signature(this, NID_sha384, data, signature);
		case SIGN_RSA_EMSA_PKCS1_SHA512:
			return build_emsa_pkcs1_signature(this, NID_sha512, data, signature);
		case SIGN_RSA_EMSA_PKCS1_MD5:
			return build_emsa_pkcs1_signature(this, NID_md5, data, signature);
		default:
			DBG1(DBG_LIB, "signature scheme %N not supported in RSA",
				 signature_scheme_names, scheme);
			return FALSE;
	}
}

METHOD(private_key_t, decrypt, bool,
	private_openssl_rsa_private_key_t *this, encryption_scheme_t scheme,
	chunk_t crypto, chunk_t *plain)
{
	int padding, len;
	char *decrypted;

	switch (scheme)
	{
		case ENCRYPT_RSA_PKCS1:
			padding = RSA_PKCS1_PADDING;
			break;
		case ENCRYPT_RSA_OAEP_SHA1:
			padding = RSA_PKCS1_OAEP_PADDING;
			break;
		default:
			DBG1(DBG_LIB, "encryption scheme %N not supported via openssl",
				 encryption_scheme_names, scheme);
			return FALSE;
	}
	decrypted = malloc(RSA_size(this->rsa));
	len = RSA_private_decrypt(crypto.len, crypto.ptr, decrypted,
							  this->rsa, padding);
	if (len < 0)
	{
		DBG1(DBG_LIB, "RSA decryption failed");
		free(decrypted);
		return FALSE;
	}
	*plain = chunk_create(decrypted, len);
	return TRUE;
}

METHOD(private_key_t, get_keysize, int,
	private_openssl_rsa_private_key_t *this)
{
	return RSA_size(this->rsa) * 8;
}

METHOD(private_key_t, get_public_key, public_key_t*,
	private_openssl_rsa_private_key_t *this)
{
	chunk_t enc;
	public_key_t *key;
	u_char *p;

	enc = chunk_alloc(i2d_RSAPublicKey(this->rsa, NULL));
	p = enc.ptr;
	i2d_RSAPublicKey(this->rsa, &p);
	key = lib->creds->create(lib->creds, CRED_PUBLIC_KEY, KEY_RSA,
							 BUILD_BLOB_ASN1_DER, enc, BUILD_END);
	free(enc.ptr);
	return key;
}

METHOD(private_key_t, get_fingerprint, bool,
	private_openssl_rsa_private_key_t *this, cred_encoding_type_t type,
	chunk_t *fingerprint)
{
	return openssl_rsa_fingerprint(this->rsa, type, fingerprint);
}

METHOD(private_key_t, get_encoding, bool,
	private_openssl_rsa_private_key_t *this, cred_encoding_type_t type,
	chunk_t *encoding)
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

			*encoding = chunk_alloc(i2d_RSAPrivateKey(this->rsa, NULL));
			p = encoding->ptr;
			i2d_RSAPrivateKey(this->rsa, &p);

			if (type == PRIVKEY_PEM)
			{
				chunk_t asn1_encoding = *encoding;

				success = lib->encoding->encode(lib->encoding, PRIVKEY_PEM,
								NULL, encoding, CRED_PART_RSA_PRIV_ASN1_DER,
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
	private_openssl_rsa_private_key_t *this)
{
	ref_get(&this->ref);
	return &this->public.key;
}

METHOD(private_key_t, destroy, void,
	private_openssl_rsa_private_key_t *this)
{
	if (ref_put(&this->ref))
	{
		if (this->rsa)
		{
			lib->encoding->clear_cache(lib->encoding, this->rsa);
			RSA_free(this->rsa);
		}
		free(this);
	}
}

/**
 * Internal generic constructor
 */
static private_openssl_rsa_private_key_t *create_empty()
{
	private_openssl_rsa_private_key_t *this;

	INIT(this,
		.public = {
			.key = {
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
		},
		.ref = 1,
	);

	return this;
}

/**
 * See header.
 */
openssl_rsa_private_key_t *openssl_rsa_private_key_gen(key_type_t type,
													   va_list args)
{
	private_openssl_rsa_private_key_t *this;
	u_int key_size = 0;
	RSA *rsa = NULL;
	BIGNUM *e = NULL;

	while (TRUE)
	{
		switch (va_arg(args, builder_part_t))
		{
			case BUILD_KEY_SIZE:
				key_size = va_arg(args, u_int);
				continue;
			case BUILD_END:
				break;
			default:
				return NULL;
		}
		break;
	}
	if (!key_size)
	{
		return NULL;
	}
	e = BN_new();
	if (!e || !BN_set_word(e, PUBLIC_EXPONENT))
	{
		goto error;
	}
	rsa = RSA_new();
	if (!rsa || !RSA_generate_key_ex(rsa, key_size, e, NULL))
	{
		goto error;
	}
	this = create_empty();
	this->rsa = rsa;
	BN_free(e);
	return &this->public;

error:
	if (e)
	{
		BN_free(e);
	}
	if (rsa)
	{
		RSA_free(rsa);
	}
	return NULL;
}

/**
 * See header
 */
openssl_rsa_private_key_t *openssl_rsa_private_key_load(key_type_t type,
														va_list args)
{
	private_openssl_rsa_private_key_t *this;
	chunk_t blob, n, e, d, p, q, exp1, exp2, coeff;

	blob = n = e = d = p = q = exp1 = exp2 = coeff = chunk_empty;
	while (TRUE)
	{
		switch (va_arg(args, builder_part_t))
		{
			case BUILD_BLOB_ASN1_DER:
				blob = va_arg(args, chunk_t);
				continue;
			case BUILD_RSA_MODULUS:
				n = va_arg(args, chunk_t);
				continue;
			case BUILD_RSA_PUB_EXP:
				e = va_arg(args, chunk_t);
				continue;
			case BUILD_RSA_PRIV_EXP:
				d = va_arg(args, chunk_t);
				continue;
			case BUILD_RSA_PRIME1:
				p = va_arg(args, chunk_t);
				continue;
			case BUILD_RSA_PRIME2:
				q = va_arg(args, chunk_t);
				continue;
			case BUILD_RSA_EXP1:
				exp1 = va_arg(args, chunk_t);
				continue;
			case BUILD_RSA_EXP2:
				exp2 = va_arg(args, chunk_t);
				continue;
			case BUILD_RSA_COEFF:
				coeff = va_arg(args, chunk_t);
				continue;
			case BUILD_END:
				break;
			default:
				return NULL;
		}
		break;
	}

	this = create_empty();
	if (blob.ptr)
	{
		this->rsa = d2i_RSAPrivateKey(NULL, (const u_char**)&blob.ptr, blob.len);
		if (this->rsa && RSA_check_key(this->rsa) == 1)
		{
			return &this->public;
		}
	}
	else if (n.ptr && e.ptr && d.ptr && p.ptr && q.ptr && coeff.ptr)
	{
		this->rsa = RSA_new();
		this->rsa->n = BN_bin2bn((const u_char*)n.ptr, n.len, NULL);
		this->rsa->e = BN_bin2bn((const u_char*)e.ptr, e.len, NULL);
		this->rsa->d = BN_bin2bn((const u_char*)d.ptr, d.len, NULL);
		this->rsa->p = BN_bin2bn((const u_char*)p.ptr, p.len, NULL);
		this->rsa->q = BN_bin2bn((const u_char*)q.ptr, q.len, NULL);
		if (exp1.ptr)
		{
			this->rsa->dmp1 = BN_bin2bn((const u_char*)exp1.ptr, exp1.len, NULL);
		}
		if (exp2.ptr)
		{
			this->rsa->dmq1 = BN_bin2bn((const u_char*)exp2.ptr, exp2.len, NULL);
		}
		this->rsa->iqmp = BN_bin2bn((const u_char*)coeff.ptr, coeff.len, NULL);
		if (RSA_check_key(this->rsa) == 1)
		{
			return &this->public;
		}
	}
	destroy(this);
	return NULL;
}

#ifndef OPENSSL_NO_ENGINE
/**
 * Login to engine with a PIN specified for a keyid
 */
static bool login(ENGINE *engine, chunk_t keyid)
{
	enumerator_t *enumerator;
	shared_key_t *shared;
	identification_t *id;
	chunk_t key;
	char pin[64];
	bool found = FALSE, success = FALSE;

	id = identification_create_from_encoding(ID_KEY_ID, keyid);
	enumerator = lib->credmgr->create_shared_enumerator(lib->credmgr,
														SHARED_PIN, id, NULL);
	while (enumerator->enumerate(enumerator, &shared, NULL, NULL))
	{
		found = TRUE;
		key = shared->get_key(shared);
		if (snprintf(pin, sizeof(pin),
					 "%.*s", (int)key.len, key.ptr) >= sizeof(pin))
		{
			continue;
		}
		if (ENGINE_ctrl_cmd_string(engine, "PIN", pin, 0))
		{
			success = TRUE;
			break;
		}
		else
		{
			DBG1(DBG_CFG, "setting PIN on engine failed");
		}
	}
	enumerator->destroy(enumerator);
	id->destroy(id);
	if (!found)
	{
		DBG1(DBG_CFG, "no PIN found for %#B", &keyid);
	}
	return success;
}
#endif /* OPENSSL_NO_ENGINE */

/**
 * See header.
 */
openssl_rsa_private_key_t *openssl_rsa_private_key_connect(key_type_t type,
														   va_list args)
{
#ifndef OPENSSL_NO_ENGINE
	private_openssl_rsa_private_key_t *this;
	char *engine_id = NULL;
	char keyname[64];
	chunk_t keyid = chunk_empty;;
	EVP_PKEY *key;
	ENGINE *engine;
	int slot = -1;

	while (TRUE)
	{
		switch (va_arg(args, builder_part_t))
		{
			case BUILD_PKCS11_KEYID:
				keyid = va_arg(args, chunk_t);
				continue;
			case BUILD_PKCS11_SLOT:
				slot = va_arg(args, int);
				continue;
			case BUILD_PKCS11_MODULE:
				engine_id = va_arg(args, char*);
				continue;
			case BUILD_END:
				break;
			default:
				return NULL;
		}
		break;
	}
	if (!keyid.len || keyid.len > 40)
	{
		return NULL;
	}

	memset(keyname, 0, sizeof(keyname));
	if (slot != -1)
	{
		snprintf(keyname, sizeof(keyname), "%d:", slot);
	}
	if (sizeof(keyname) - strlen(keyname) <= keyid.len * 4 / 3 + 1)
	{
		return NULL;
	}
	chunk_to_hex(keyid, keyname + strlen(keyname), FALSE);

	if (!engine_id)
	{
		engine_id = lib->settings->get_str(lib->settings,
							"%s.plugins.openssl.engine_id", "pkcs11", lib->ns);
	}
	engine = ENGINE_by_id(engine_id);
	if (!engine)
	{
		DBG2(DBG_LIB, "engine '%s' is not available", engine_id);
		return NULL;
	}
	if (!ENGINE_init(engine))
	{
		DBG1(DBG_LIB, "failed to initialize engine '%s'", engine_id);
		ENGINE_free(engine);
		return NULL;
	}
	if (!login(engine, keyid))
	{
		DBG1(DBG_LIB, "login to engine '%s' failed", engine_id);
		ENGINE_free(engine);
		return NULL;
	}
	key = ENGINE_load_private_key(engine, keyname, NULL, NULL);
	if (!key)
	{
		DBG1(DBG_LIB, "failed to load private key with ID '%s' from "
			 "engine '%s'", keyname, engine_id);
		ENGINE_free(engine);
		return NULL;
	}
	ENGINE_free(engine);

	this = create_empty();
	this->rsa = EVP_PKEY_get1_RSA(key);
	this->engine = TRUE;
	if (!this->rsa)
	{
		destroy(this);
		return NULL;
	}

	return &this->public;
#else /* OPENSSL_NO_ENGINE */
	return NULL;
#endif /* OPENSSL_NO_ENGINE */
}

#endif /* OPENSSL_NO_RSA */
