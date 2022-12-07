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
 * @defgroup wolfssl_rng wolfssl_rng
 * @{ @ingroup wolfssl_p
 */

#ifndef WOLFSSL_PLUGIN_RNG_H_
#define WOLFSSL_PLUGIN_RNG_H_

#include <library.h>

typedef struct wolfssl_rng_t wolfssl_rng_t;

/**
 * Implementation of random number using wolfSSL.
 */
struct wolfssl_rng_t {

	/**
	 * Implements rng_t interface.
	 */
	rng_t rng;
};

/**
 * Constructor to create wolfssl_rng_t.
 *
 * @param quality	quality of randomness
 * @return			wolfssl_rng_t
 */
wolfssl_rng_t *wolfssl_rng_create(rng_quality_t quality);

/**
 * Initializes the global random number generator.
 *
 * @return	TRUE on success and FALSE on failure.
 */
int wolfssl_rng_global_init();

/**
 * Finalizes the global random number generator.
 */
void wolfssl_rng_global_final();

#endif /** WOLFSSL_PLUGIN_RNG_H_ @}*/
