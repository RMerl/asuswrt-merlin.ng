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

/**
 * @defgroup wolfssl_hasher wolfssl_hasher
 * @{ @ingroup wolfssl_p
 */

#ifndef WOLFSSL_PLUGIN_HASHER_H_
#define WOLFSSL_PLUGIN_HASHER_H_

typedef struct wolfssl_hasher_t wolfssl_hasher_t;

#include <crypto/hashers/hasher.h>

/**
 * Implementation of hashers using wolfSSL.
 */
struct wolfssl_hasher_t {

	/**
	 * Implements hasher_t interface.
	 */
	hasher_t hasher;
};

/**
 * Constructor to create wolfssl_hasher_t.
 *
 * @param algo			algorithm
 * @return				wolfssl_hasher_t, NULL if not supported
 */
wolfssl_hasher_t *wolfssl_hasher_create(hash_algorithm_t algo);

#endif /** WOLFSSL_PLUGIN_HASHER_H_ @}*/
