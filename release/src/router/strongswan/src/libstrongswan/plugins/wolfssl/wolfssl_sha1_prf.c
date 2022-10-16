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

#ifndef NO_SHA

#include "wolfssl_sha1_prf.h"

#include <wolfssl/wolfcrypt/sha.h>
#include <crypto/hashers/hasher.h>

typedef struct private_wolfssl_sha1_prf_t private_wolfssl_sha1_prf_t;

/**
 * Private data of an wolfssl_sha1_prf_t object.
 */
struct private_wolfssl_sha1_prf_t {

	/**
	 * Public wolfssl_sha1_prf_t interface
	 */
	wolfssl_sha1_prf_t public;

	/**
	 * SHA1 context
	 */
	wc_Sha sha1;
};

METHOD(prf_t, get_bytes, bool,
	private_wolfssl_sha1_prf_t *this, chunk_t seed, uint8_t *bytes)
{
	if (wc_ShaUpdate(&this->sha1, seed.ptr, seed.len) != 0)
	{
		return FALSE;
	}

	if (bytes)
	{
		uint32_t *hash = (uint32_t*)bytes;

		hash[0] = htonl(this->sha1.digest[0]);
		hash[1] = htonl(this->sha1.digest[1]);
		hash[2] = htonl(this->sha1.digest[2]);
		hash[3] = htonl(this->sha1.digest[3]);
		hash[4] = htonl(this->sha1.digest[4]);
	}
	return TRUE;
}

METHOD(prf_t, get_block_size, size_t,
	private_wolfssl_sha1_prf_t *this)
{
	return HASH_SIZE_SHA1;
}

METHOD(prf_t, allocate_bytes, bool,
	private_wolfssl_sha1_prf_t *this, chunk_t seed, chunk_t *chunk)
{
	if (chunk)
	{
		*chunk = chunk_alloc(HASH_SIZE_SHA1);
		return get_bytes(this, seed, chunk->ptr);
	}
	return get_bytes(this, seed, NULL);
}

METHOD(prf_t, get_key_size, size_t,
	private_wolfssl_sha1_prf_t *this)
{
	return HASH_SIZE_SHA1;
}

METHOD(prf_t, set_key, bool,
	private_wolfssl_sha1_prf_t *this, chunk_t key)
{
	if (wc_InitSha(&this->sha1) != 0)
	{
		return FALSE;
	}

	if (key.len % 4)
	{
		return FALSE;
	}
	if (key.len >= 4)
	{
		this->sha1.digest[0] ^= untoh32(key.ptr);
	}
	if (key.len >= 8)
	{
		this->sha1.digest[1] ^= untoh32(key.ptr + 4);
	}
	if (key.len >= 12)
	{
		this->sha1.digest[2] ^= untoh32(key.ptr + 8);
	}
	if (key.len >= 16)
	{
		this->sha1.digest[3] ^= untoh32(key.ptr + 12);
	}
	if (key.len >= 20)
	{
		this->sha1.digest[4] ^= untoh32(key.ptr + 16);
	}
	return TRUE;
}

METHOD(prf_t, destroy, void,
	private_wolfssl_sha1_prf_t *this)
{
	wc_ShaFree(&this->sha1);
	free(this);
}

/*
 * Described in header
 */
wolfssl_sha1_prf_t *wolfssl_sha1_prf_create(pseudo_random_function_t algo)
{
	private_wolfssl_sha1_prf_t *this;

	INIT(this,
		.public = {
			.prf = {
				.get_block_size = _get_block_size,
				.get_bytes = _get_bytes,
				.allocate_bytes = _allocate_bytes,
				.get_key_size = _get_key_size,
				.set_key = _set_key,
				.destroy = _destroy,
			},
		},
	);

	if (wc_InitSha(&this->sha1) != 0)
	{
		free(this);
		return NULL;
	}
	return &this->public;
}

#endif /* NO_SHA */
