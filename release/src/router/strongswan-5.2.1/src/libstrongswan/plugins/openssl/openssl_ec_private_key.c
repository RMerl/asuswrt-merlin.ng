/*
 * Copyright (C) 2008-2012 Tobias Brunner
 * Copyright (C) 2009 Martin Willi
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

#ifndef OPENSSL_NO_ECDSA

#include "openssl_ec_private_key.h"
#include "openssl_ec_public_key.h"
#include "openssl_util.h"

#include <utils/debug.h>

#include <openssl/evp.h>
#include <openssl/ecdsa.h>
#include <openssl/x509.h>

typedef struct private_openssl_ec_private_key_t private_openssl_ec_private_key_t;

/**
 * Private data of a openssl_ec_private_key_t object.
 */
struct private_openssl_ec_private_key_t {
	/**
	 * Public interface for this signer.
	 */
	openssl_ec_private_key_t public;

	/**
	 * EC key object
	 */
	EC_KEY *ec;

	/**
	 * reference count
	 */
	refcount_t ref;
};

/* from ec public key */
bool openssl_ec_fingerprint(EC_KEY *ec, cred_encoding_type_t type, chunk_t *fp);

/**
 * Build a signature as in RFC 4754
 */
static bool build_signature(private_openssl_ec_private_key_t *this,
							chunk_t hash, chunk_t *signature)
{
	bool built = FALSE;
	ECDSA_SIG *sig;

	sig = ECDSA_do_sign(hash.ptr, hash.len, this->ec);
	if (sig)
	{
		/* concatenate BNs r/s to a signature chunk */
		built = openssl_bn_cat(EC_FIELD_ELEMENT_LEN(EC_KEY_get0_group(this->ec)),
							   sig->r, sig->s, signature);
		ECDSA_SIG_free(sig);
	}
	return built;
}

/**
 * Build a RFC 4754 signature for a specified curve and hash algorithm
 */
static bool build_curve_signature(private_openssl_ec_private_key_t *this,
								signature_scheme_t scheme, int nid_hash,
								int nid_curve, chunk_t data, chunk_t *signature)
{
	const EC_GROUP *my_group;
	EC_GROUP *req_group;
	chunk_t hash;
	bool built;

	req_group = EC_GROUP_new_by_curve_name(nid_curve);
	if (!req_group)
	{
		DBG1(DBG_LIB, "signature scheme %N not supported in EC (required curve "
			 "not supported)", signature_scheme_names, scheme);
		return FALSE;
	}
	my_group = EC_KEY_get0_group(this->ec);
	if (EC_GROUP_cmp(my_group, req_group, NULL) != 0)
	{
		DBG1(DBG_LIB, "signature scheme %N not supported by private key",
			 signature_scheme_names, scheme);
		return FALSE;
	}
	EC_GROUP_free(req_group);
	if (!openssl_hash_chunk(nid_hash, data, &hash))
	{
		return FALSE;
	}
	built = build_signature(this, hash, signature);
	chunk_free(&hash);
	return built;
}

/**
 * Build a DER encoded signature as in RFC 3279
 */
static bool build_der_signature(private_openssl_ec_private_key_t *this,
								int hash_nid, chunk_t data, chunk_t *signature)
{
	chunk_t hash, sig;
	int siglen = 0;
	bool built;

	if (!openssl_hash_chunk(hash_nid, data, &hash))
	{
		return FALSE;
	}
	sig = chunk_alloc(ECDSA_size(this->ec));
	built = ECDSA_sign(0, hash.ptr, hash.len, sig.ptr, &siglen, this->ec) == 1;
	sig.len = siglen;
	if (built)
	{
		*signature = sig;
	}
	else
	{
		free(sig.ptr);
	}
	free(hash.ptr);
	return built;
}

METHOD(private_key_t, sign, bool,
	private_openssl_ec_private_key_t *this, signature_scheme_t scheme,
	chunk_t data, chunk_t *signature)
{
	switch (scheme)
	{
		case SIGN_ECDSA_WITH_NULL:
			return build_signature(this, data, signature);
		case SIGN_ECDSA_WITH_SHA1_DER:
			return build_der_signature(this, NID_sha1, data, signature);
		case SIGN_ECDSA_WITH_SHA256_DER:
			return build_der_signature(this, NID_sha256, data, signature);
		case SIGN_ECDSA_WITH_SHA384_DER:
			return build_der_signature(this, NID_sha384, data, signature);
		case SIGN_ECDSA_WITH_SHA512_DER:
			return build_der_signature(this, NID_sha512, data, signature);
		case SIGN_ECDSA_256:
			return build_curve_signature(this, scheme, NID_sha256,
										 NID_X9_62_prime256v1, data, signature);
		case SIGN_ECDSA_384:
			return build_curve_signature(this, scheme, NID_sha384,
										 NID_secp384r1, data, signature);
		case SIGN_ECDSA_521:
			return build_curve_signature(this, scheme, NID_sha512,
										 NID_secp521r1, data, signature);
		default:
			DBG1(DBG_LIB, "signature scheme %N not supported",
				 signature_scheme_names, scheme);
			return FALSE;
	}
}

METHOD(private_key_t, decrypt, bool,
	private_openssl_ec_private_key_t *this, encryption_scheme_t scheme,
	chunk_t crypto, chunk_t *plain)
{
	DBG1(DBG_LIB, "EC private key decryption not implemented");
	return FALSE;
}

METHOD(private_key_t, get_keysize, int,
	private_openssl_ec_private_key_t *this)
{
	return EC_GROUP_get_degree(EC_KEY_get0_group(this->ec));
}

METHOD(private_key_t, get_type, key_type_t,
	private_openssl_ec_private_key_t *this)
{
	return KEY_ECDSA;
}

METHOD(private_key_t, get_public_key, public_key_t*,
	private_openssl_ec_private_key_t *this)
{
	public_key_t *public;
	chunk_t key;
	u_char *p;

	key = chunk_alloc(i2d_EC_PUBKEY(this->ec, NULL));
	p = key.ptr;
	i2d_EC_PUBKEY(this->ec, &p);

	public = lib->creds->create(lib->creds, CRED_PUBLIC_KEY, KEY_ECDSA,
								BUILD_BLOB_ASN1_DER, key, BUILD_END);
	free(key.ptr);
	return public;
}

METHOD(private_key_t, get_fingerprint, bool,
	private_openssl_ec_private_key_t *this, cred_encoding_type_t type,
	chunk_t *fingerprint)
{
	return openssl_ec_fingerprint(this->ec, type, fingerprint);
}

METHOD(private_key_t, get_encoding, bool,
	private_openssl_ec_private_key_t *this, cred_encoding_type_t type,
	chunk_t *encoding)
{
	u_char *p;

	switch (type)
	{
		case PRIVKEY_ASN1_DER:
		case PRIVKEY_PEM:
		{
			bool success = TRUE;

			*encoding = chunk_alloc(i2d_ECPrivateKey(this->ec, NULL));
			p = encoding->ptr;
			i2d_ECPrivateKey(this->ec, &p);

			if (type == PRIVKEY_PEM)
			{
				chunk_t asn1_encoding = *encoding;

				success = lib->encoding->encode(lib->encoding, PRIVKEY_PEM,
								NULL, encoding, CRED_PART_ECDSA_PRIV_ASN1_DER,
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
	private_openssl_ec_private_key_t *this)
{
	ref_get(&this->ref);
	return &this->public.key;
}

METHOD(private_key_t, destroy, void,
	private_openssl_ec_private_key_t *this)
{
	if (ref_put(&this->ref))
	{
		if (this->ec)
		{
			lib->encoding->clear_cache(lib->encoding, this->ec);
			EC_KEY_free(this->ec);
		}
		free(this);
	}
}

/**
 * Internal generic constructor
 */
static private_openssl_ec_private_key_t *create_empty(void)
{
	private_openssl_ec_private_key_t *this;

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
openssl_ec_private_key_t *openssl_ec_private_key_gen(key_type_t type,
													 va_list args)
{
	private_openssl_ec_private_key_t *this;
	u_int key_size = 0;

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
	this = create_empty();
	switch (key_size)
	{
		case 256:
			this->ec = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
			break;
		case 384:
			this->ec = EC_KEY_new_by_curve_name(NID_secp384r1);
			break;
		case 521:
			this->ec = EC_KEY_new_by_curve_name(NID_secp521r1);
			break;
		default:
			DBG1(DBG_LIB, "EC private key size %d not supported", key_size);
			destroy(this);
			return NULL;
	}
	if (EC_KEY_generate_key(this->ec) != 1)
	{
		DBG1(DBG_LIB, "EC private key generation failed", key_size);
		destroy(this);
		return NULL;
	}
	/* encode as a named curve key (no parameters), uncompressed public key */
	EC_KEY_set_asn1_flag(this->ec, OPENSSL_EC_NAMED_CURVE);
	EC_KEY_set_conv_form(this->ec, POINT_CONVERSION_UNCOMPRESSED);
	return &this->public;
}

/**
 * See header.
 */
openssl_ec_private_key_t *openssl_ec_private_key_load(key_type_t type,
													  va_list args)
{
	private_openssl_ec_private_key_t *this;
	chunk_t par = chunk_empty, key = chunk_empty;

	while (TRUE)
	{
		switch (va_arg(args, builder_part_t))
		{
			case BUILD_BLOB_ALGID_PARAMS:
				par = va_arg(args, chunk_t);
				continue;
			case BUILD_BLOB_ASN1_DER:
				key = va_arg(args, chunk_t);
				continue;
			case BUILD_END:
				break;
			default:
				return NULL;
		}
		break;
	}

	this = create_empty();

	if (par.ptr)
	{
		this->ec = d2i_ECParameters(NULL, (const u_char**)&par.ptr, par.len);
		if (!this->ec)
		{
			goto error;
		}
		if (!d2i_ECPrivateKey(&this->ec, (const u_char**)&key.ptr, key.len))
		{
			goto error;
		}
	}
	else
	{
		this->ec = d2i_ECPrivateKey(NULL, (const u_char**)&key.ptr, key.len);
		if (!this->ec)
		{
			goto error;
		}
	}
	if (!EC_KEY_check_key(this->ec))
	{
		goto error;
	}
	return &this->public;

error:
	destroy(this);
	return NULL;
}
#endif /* OPENSSL_NO_ECDSA */
