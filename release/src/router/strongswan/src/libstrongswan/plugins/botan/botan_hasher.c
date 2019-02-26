/*
 * Copyright (C) 2018 René Korthaus
 * Rohde & Schwarz Cybersecurity GmbH
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

#include "botan_hasher.h"
#include "botan_util.h"

#include <utils/debug.h>

#include <botan/ffi.h>

typedef struct private_botan_hasher_t private_botan_hasher_t;

/**
 * Private data of botan_hasher_t
 */
struct private_botan_hasher_t {

	/**
	 * Public part of this class.
	 */
	botan_hasher_t public;

	/**
	 * botan hash instance
	 */
	botan_hash_t hash;
};

METHOD(hasher_t, get_hash_size, size_t,
	private_botan_hasher_t *this)
{
	size_t len = 0;

	if (botan_hash_output_length(this->hash, &len))
	{
		return 0;
	}
	return len;
}

METHOD(hasher_t, reset, bool,
	private_botan_hasher_t *this)
{
	if (botan_hash_clear(this->hash))
	{
		return FALSE;
	}
	return TRUE;
}

METHOD(hasher_t, get_hash, bool,
	private_botan_hasher_t *this, chunk_t chunk, uint8_t *hash)
{
	if (botan_hash_update(this->hash, chunk.ptr, chunk.len))
	{
		return FALSE;
	}

	if (hash && botan_hash_final(this->hash, hash))
	{
		return FALSE;
	}
	return TRUE;
}

METHOD(hasher_t, allocate_hash, bool,
	private_botan_hasher_t *this, chunk_t chunk, chunk_t *hash)
{
	if (hash)
	{
		*hash = chunk_alloc(get_hash_size(this));
		return get_hash(this, chunk, hash->ptr);
	}
	return get_hash(this, chunk, NULL);
}

METHOD(hasher_t, destroy, void,
	private_botan_hasher_t *this)
{
	botan_hash_destroy(this->hash);
	free(this);
}

/*
 * Described in header
 */
botan_hasher_t *botan_hasher_create(hash_algorithm_t algo)
{
	private_botan_hasher_t *this;
	const char* hash_name;

	hash_name = botan_get_hash(algo);
	if (!hash_name)
	{
		return FALSE;
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
	);

	if (botan_hash_init(&this->hash, hash_name, 0))
	{
		free(this);
		return NULL;
	}
	return &this->public;
}
