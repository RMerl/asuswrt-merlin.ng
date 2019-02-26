/*
 * Copyright (C) 2012-2018 Tobias Brunner
 * HSR Hochschule fuer Technik Rapperswil
 *
 * Copyright (C) 2012 Aleksandr Grinberg
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

#include <library.h>
#include <utils/debug.h>

#include <openssl/rand.h>

#include "openssl_rng.h"

typedef struct private_openssl_rng_t private_openssl_rng_t;

/**
 * Private data of openssl_rng_t
 */
struct private_openssl_rng_t {

	/**
	 * Public part of this class.
	 */
	openssl_rng_t public;

	/**
	 * Quality of randomness
	 */
	rng_quality_t quality;
};

METHOD(rng_t, get_bytes, bool,
	private_openssl_rng_t *this, size_t bytes, uint8_t *buffer)
{
#if OPENSSL_VERSION_NUMBER >= 0x1010100fL
	if (this->quality > RNG_WEAK)
	{	/* use a separate DRBG for data we want to keep private, compared
		 * to e.g. nonces */
		return RAND_priv_bytes((char*)buffer, bytes) == 1;
	}
#endif
	return RAND_bytes((char*)buffer, bytes) == 1;
}

METHOD(rng_t, allocate_bytes, bool,
	private_openssl_rng_t *this, size_t bytes, chunk_t *chunk)
{
	*chunk = chunk_alloc(bytes);
	if (!get_bytes(this, chunk->len, chunk->ptr))
	{
		chunk_free(chunk);
		return FALSE;
	}
	return TRUE;
}

METHOD(rng_t, destroy, void,
	private_openssl_rng_t *this)
{
	free(this);
}

/*
 * Described in header.
 */
openssl_rng_t *openssl_rng_create(rng_quality_t quality)
{
	private_openssl_rng_t *this;

	INIT(this,
		.public = {
			.rng = {
				.get_bytes = _get_bytes,
				.allocate_bytes = _allocate_bytes,
				.destroy = _destroy,
			},
		},
		.quality = quality,
	);

	return &this->public;
}
