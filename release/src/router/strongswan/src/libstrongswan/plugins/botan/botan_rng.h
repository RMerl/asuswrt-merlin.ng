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
 * @defgroup botan_rng botan_rng
 * @{ @ingroup botan_p
 */

#ifndef BOTAN_RNG_H_
#define BOTAN_RNG_H_

typedef struct botan_random_t botan_random_t;

#include <library.h>

/**
 * rng_t implementation using botan.
 *
 * @note botan_rng_t is a botan reserved type.
 */
struct botan_random_t {

	/**
	 * Implements rng_t.
	 */
	rng_t rng;
};

/**
 * Creates a botan_random_t instance.
 *
 * @param quality	required quality of randomness
 * @return			botan_random_t instance
 */
botan_random_t *botan_rng_create(rng_quality_t quality);

#endif /** BOTAN_RNG_H_ @} */
