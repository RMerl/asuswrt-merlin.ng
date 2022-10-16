/*
 * Copyright (C) 2019 Sean Parkinson, wolfSSL Inc.
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
#include "wolfssl_hasher.h"
#include "wolfssl_util.h"

#include <wolfssl/wolfcrypt/hash.h>

typedef struct private_wolfssl_hasher_t private_wolfssl_hasher_t;

/**
 * Private data of wolfssl_hasher_t
 */
struct private_wolfssl_hasher_t {

	/**
	 * Public interface
	 */
	wolfssl_hasher_t public;

	/**
	 * The hasher to use
	 */
	wc_HashAlg hasher;

	/**
	 * The hash algorithm
	 */
	enum wc_HashType type;
};

METHOD(hasher_t, get_hash_size, size_t,
	private_wolfssl_hasher_t *this)
{
	return wc_HashGetDigestSize(this->type);
}

METHOD(hasher_t, reset, bool,
	private_wolfssl_hasher_t *this)
{
	return wc_HashInit(&this->hasher, this->type) == 0;
}

METHOD(hasher_t, get_hash, bool,
	private_wolfssl_hasher_t *this, chunk_t chunk, uint8_t *hash)
{
	int ret;

	ret = wc_HashUpdate(&this->hasher, this->type, chunk.ptr, chunk.len);
	if (ret == 0 && hash)
	{
		ret = wc_HashFinal(&this->hasher, this->type, hash);
		if (ret == 0)
		{
			return reset(this);
		}
	}
	return ret == 0;
}

METHOD(hasher_t, allocate_hash, bool,
	private_wolfssl_hasher_t *this, chunk_t chunk, chunk_t *hash)
{
	if (hash)
	{
		*hash = chunk_alloc(get_hash_size(this));
		return get_hash(this, chunk, hash->ptr);
	}
	return get_hash(this, chunk, NULL);
}

METHOD(hasher_t, destroy, void,
	private_wolfssl_hasher_t *this)
{
	wc_HashFree(&this->hasher, this->type);
	free(this);
}

/*
 * Described in header
 */
wolfssl_hasher_t *wolfssl_hasher_create(hash_algorithm_t algo)
{
	private_wolfssl_hasher_t *this;
	enum wc_HashType type;

	if (!wolfssl_hash2type(algo, &type))
	{
		return NULL;
	}

	INIT(this,
		.public = {
			.hasher = {
				.get_hash = _get_hash,
				.allocate_hash = _allocate_hash,
				.get_hash_size = _get_hash_size,
				.reset = _reset,
				.destroy = _destroy,
			},
		},
		.type = type,
	);

	/* initialization */
	if (!reset(this))
	{
		destroy(this);
		return NULL;
	}
	return &this->public;
}
