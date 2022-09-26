/*
 * Copyright (C) 2022 Tobias Brunner, codelabs GmbH
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

#define _GNU_SOURCE
#include "botan_kdf.h"
#include "botan_util.h"

#include <botan/build.h>

#ifdef BOTAN_HAS_HKDF

#include <botan/ffi.h>

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
	 * Name of the KDF algorithm in Botan.
	 */
	char *name;

	/**
	 * Key for KDF.
	 */
	chunk_t key;

	/**
	 * Salt for KDF.
	 */
	chunk_t salt;

	/**
	 * Length of the hash output.
	 */
	size_t hash_size;
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
	return this->hash_size;
}

METHOD(kdf_t, get_bytes, bool,
	private_kdf_t *this, size_t out_len, uint8_t *buffer)
{
	if (this->type == KDF_PRF)
	{
		if (out_len != get_length(this) ||
			botan_kdf(this->name, buffer, out_len, this->key.ptr, this->key.len,
					  this->salt.ptr, this->salt.len, NULL, 0))
		{
			return FALSE;
		}
		return TRUE;
	}

#if BOTAN_VERSION_MAJOR == 2
	/* Botan 2 doesn't check the length, just silently prevents wrapping the
	 * counter and returns truncated output, so do this manually */
	if (out_len > this->hash_size * 255)
	{
		return FALSE;
	}
#endif
	if (botan_kdf(this->name, buffer, out_len, this->key.ptr, this->key.len,
				  NULL, 0, this->salt.ptr, this->salt.len))
	{
		return FALSE;
	}
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
	free(this->name);
	free(this);
}

/*
 * Described in header
 */
kdf_t *botan_kdf_create(key_derivation_function_t algo, va_list args)
{
	private_kdf_t *this;
	pseudo_random_function_t prf_alg;
	const char *hash_name;
	char *name, buf[HASH_SIZE_SHA512];

	if (algo != KDF_PRF && algo != KDF_PRF_PLUS)
	{
		return NULL;
	}

	VA_ARGS_VGET(args, prf_alg);
	hash_name = botan_get_hash(hasher_algorithm_from_prf(prf_alg));
	if (!hash_name)
	{
		return NULL;
	}
	if (algo == KDF_PRF)
	{
		if (asprintf(&name, "HKDF-Extract(%s)", hash_name) <= 0)
		{
			return NULL;
		}
	}
	else if (asprintf(&name, "HKDF-Expand(%s)", hash_name) <= 0)
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
		.name = name,
		.hash_size = hasher_hash_size(hasher_algorithm_from_prf(prf_alg)),
	);

	/* test if we can actually use the algorithm */
	if (!get_bytes(this, algo == KDF_PRF ? get_length(this) : sizeof(buf), buf))
	{
		destroy(this);
		return NULL;
	}
	return &this->public;
}

#endif
