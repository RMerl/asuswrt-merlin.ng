/*
 * Copyright (C) 2018 René Korthaus
 * Copyright (C) 2018 Konstantinos Kolelis
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
 * @defgroup botan_crypter botan_crypter
 * @{ @ingroup botan_p
 */

#ifndef BOTAN_CRYPTER_H_
#define BOTAN_CRYPTER_H_

typedef struct botan_crypter_t botan_crypter_t;

#include <crypto/crypters/crypter.h>

/**
 * Implementation of crypters using Botan.
 */
struct botan_crypter_t {

	/**
	 * Implements crypter_t interface.
	 */
	crypter_t crypter;
};

/**
 * Constructor to create botan_crypter_t.
 *
 * @param algo			algorithm to implement
 * @param key_size		key size in bytes
 * @return				botan_crypter_t, NULL if not supported
 */
botan_crypter_t *botan_crypter_create(encryption_algorithm_t algo,
									  size_t key_size);

#endif /** BOTAN_CRYPTER_H_ @}*/
