/*
 * Copyright (C) 2022 Tobias Brunner
 *
 * Copyright (C) secunet Security Networks AG
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

#include <openssl/opensslv.h>
#include <openssl/opensslconf.h>

#if !defined(OPENSSL_NO_HMAC) && OPENSSL_VERSION_NUMBER >= 0x10101000L

#include <openssl/evp.h>
#include <openssl/kdf.h>

#include "openssl_kdf.h"

typedef struct private_kdf_t private_kdf_t;

/**
 * Private data.
 */
struct private_kdf_t {

	/**
	 * Public interface.
	 */
	kdf_t public;

	/**
	 * KDF type.
	 */
	key_derivation_function_t type;

	/**
	 * Hasher to use for underlying PRF.
	 */
	const EVP_MD *hasher;

	/**
	 * Key for KDF. Stored here because OpenSSL's HKDF API does not provide a
	 * way to clear the "info" field in the context, new data is always
	 * appended (up to 1024 bytes).
	 */
	chunk_t key;

	/**
	 * Salt for prf+ (see above).
	 */
	chunk_t salt;
};

METHOD(kdf_t, get_type, key_derivation_function_t,
	private_kdf_t *this)
{
	return this->type;
}

METHOD(kdf_t, get_length, size_t,
	private_kdf_t *this)
{
	if (this->type == KDF_PRF_PLUS)
	{
		return SIZE_MAX;
	}
	return EVP_MD_size(this->hasher);
}

/**
 * Set the parameters as a appropriate for the given KDF type.
 */
static bool set_params(private_kdf_t *this, EVP_PKEY_CTX *ctx)
{
	if (this->type == KDF_PRF)
	{
		return EVP_PKEY_CTX_hkdf_mode(ctx, EVP_PKEY_HKDEF_MODE_EXTRACT_ONLY) > 0 &&
			   EVP_PKEY_CTX_set1_hkdf_key(ctx, this->key.ptr, this->key.len) > 0 &&
			   EVP_PKEY_CTX_set1_hkdf_salt(ctx, this->salt.ptr, this->salt.len) > 0;
	}
	/* for HKDF-Expand() we map the salt to the "info" field */
	return EVP_PKEY_CTX_hkdf_mode(ctx, EVP_PKEY_HKDEF_MODE_EXPAND_ONLY) > 0 &&
		   EVP_PKEY_CTX_set1_hkdf_key(ctx, this->key.ptr, this->key.len) > 0 &&
		   EVP_PKEY_CTX_add1_hkdf_info(ctx, this->salt.ptr, this->salt.len) > 0;
}

METHOD(kdf_t, get_bytes, bool,
	private_kdf_t *this, size_t out_len, uint8_t *buffer)
{
	EVP_PKEY_CTX *ctx;

	if (this->type == KDF_PRF && out_len != get_length(this))
	{
		return FALSE;
	}

	ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_HKDF, NULL);
	if (!ctx ||
		EVP_PKEY_derive_init(ctx) <= 0 ||
		EVP_PKEY_CTX_set_hkdf_md(ctx, this->hasher) <= 0 ||
		!set_params(this, ctx) ||
		EVP_PKEY_derive(ctx, buffer, &out_len) <= 0)
	{
		EVP_PKEY_CTX_free(ctx);
		return FALSE;
	}
	EVP_PKEY_CTX_free(ctx);
	return TRUE;
}

METHOD(kdf_t, allocate_bytes, bool,
	private_kdf_t *this, size_t out_len, chunk_t *chunk)
{
	if (this->type == KDF_PRF)
	{
		out_len = out_len ?: get_length(this);
	}

	*chunk = chunk_alloc(out_len);

	if (!get_bytes(this, out_len, chunk->ptr))
	{
		chunk_free(chunk);
		return FALSE;
	}
	return TRUE;
}

METHOD(kdf_t, set_param, bool,
	private_kdf_t *this, kdf_param_t param, ...)
{
	chunk_t chunk;

	switch (param)
	{
		case KDF_PARAM_KEY:
			VA_ARGS_GET(param, chunk);
			chunk_clear(&this->key);
			this->key = chunk_clone(chunk);
			break;
		case KDF_PARAM_SALT:
			VA_ARGS_GET(param, chunk);
			chunk_clear(&this->salt);
			this->salt = chunk_clone(chunk);
			break;
	}
	return TRUE;
}

METHOD(kdf_t, destroy, void,
	private_kdf_t *this)
{
	chunk_clear(&this->salt);
	chunk_clear(&this->key);
	free(this);
}

/*
 * Described in header
 */
kdf_t *openssl_kdf_create(key_derivation_function_t algo, va_list args)
{
	private_kdf_t *this;
	pseudo_random_function_t prf_alg;
	char *name, buf[EVP_MAX_MD_SIZE];

	if (algo != KDF_PRF && algo != KDF_PRF_PLUS)
	{
		return NULL;
	}

	VA_ARGS_VGET(args, prf_alg);
	name = enum_to_name(hash_algorithm_short_names,
						hasher_algorithm_from_prf(prf_alg));
	if (!name)
	{
		return NULL;
	}

	INIT(this,
		.public = {
			.get_type = _get_type,
			.get_length = _get_length,
			.get_bytes = _get_bytes,
			.allocate_bytes = _allocate_bytes,
			.set_param = _set_param,
			.destroy = _destroy,
		},
		.type = algo,
		.hasher = EVP_get_digestbyname(name),
		/* use a lengthy key to test the implementation below to make sure the
		 * algorithms are usable, see openssl_hmac.c for details */
		.key = chunk_clone(chunk_from_str("00000000000000000000000000000000")),
	);

	if (!this->hasher ||
		!get_bytes(this, algo == KDF_PRF ? get_length(this) : sizeof(buf), buf))
	{
		destroy(this);
		return NULL;
	}
	return &this->public;
}

#endif /* OPENSSL_NO_HMAC && OPENSSL_VERSION_NUMBER */
