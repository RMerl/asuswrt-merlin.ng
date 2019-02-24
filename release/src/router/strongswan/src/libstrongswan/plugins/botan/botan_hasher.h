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

/**
 * @defgroup botan_hasher botan_hasher
 * @{ @ingroup botan_p
 */

#ifndef BOTAN_HASHER_H_
#define BOTAN_HASHER_H_

typedef struct botan_hasher_t botan_hasher_t;

#include <crypto/hashers/hasher.h>

/**
 * Implementation of hashers using botan.
 */
struct botan_hasher_t {

	/**
	 * The hasher_t interface.
	 */
	hasher_t hasher;
};

/**
 * Constructor to create botan_hasher_t.
 *
 * @param algo			algorithm
 * @return				botan_hasher_t, NULL if not supported
 */
botan_hasher_t *botan_hasher_create(hash_algorithm_t algo);

#endif /** BOTAN_HASHER_H_ @}*/
