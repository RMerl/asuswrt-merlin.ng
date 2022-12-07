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

#include "wolfssl_common.h"

#if !defined(NO_HMAC) && defined(HAVE_HKDF)

#include <wolfssl/wolfcrypt/hmac.h>

#define _GNU_SOURCE
#include "wolfssl_kdf.h"
#include "wolfssl_util.h"

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
	 * Hash algorithm type.
	 */
	enum wc_HashType hash;

	/**
	 * Key for KDF.
	 */
	chunk_t key;

	/**
	 * Salt for KDF.
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
	return wc_HashGetDigestSize(this->hash);
}

METHOD(kdf_t, get_bytes, bool,
	private_kdf_t *this, size_t out_len, uint8_t *buffer)
{
	if (this->type == KDF_PRF)
	{
		if (out_len != get_length(this) ||
			wc_HKDF_Extract(this->hash, this->salt.ptr, this->salt.len,
							this->key.ptr, this->key.len, buffer))
		{
			return FALSE;
		}
		return TRUE;
	}
	if (wc_HKDF_Expand(this->hash, this->key.ptr, this->key.len,
					   this->salt.ptr, this->salt.len, buffer, out_len))
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
	free(this);
}

/*
 * Described in header
 */
kdf_t *wolfssl_kdf_create(key_derivation_function_t algo, va_list args)
{
	private_kdf_t *this;
	pseudo_random_function_t prf_alg;
	enum wc_HashType hash;
	char buf[HASH_SIZE_SHA512];

	if (algo != KDF_PRF && algo != KDF_PRF_PLUS)
	{
		return NULL;
	}

	VA_ARGS_VGET(args, prf_alg);
	if (!wolfssl_hash2type(hasher_algorithm_from_prf(prf_alg), &hash))
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
		.hash = hash,
	);

	/* test if we can actually use the algorithm */
	if (!get_bytes(this, algo == KDF_PRF ? get_length(this) : sizeof(buf), buf))
	{
		destroy(this);
		return NULL;
	}
	return &this->public;
}

#endif /* !NO_HMAC && HAVE_HKDF */
