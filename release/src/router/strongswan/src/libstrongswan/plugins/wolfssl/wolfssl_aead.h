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
 * Implements the aead_t interface using wolfSSL.
 *
 * @defgroup wolfssl_aead wolfssl_aead
 * @{ @ingroup wolfssl_p
 */

#ifndef WOLFSSL_PLUGIN_AEAD_H_
#define WOLFSSL_PLUGIN_AEAD_H_

#include <crypto/aead.h>

/**
 * Constructor to create aead_t implementation.
 *
 * @param algo			algorithm to implement
 * @param key_size		key size in bytes
 * @param salt_size		size of implicit salt length
 * @return				aead_t object, NULL if not supported
 */
aead_t *wolfssl_aead_create(encryption_algorithm_t algo, size_t key_size,
							size_t salt_size);

#endif /** WOLFSSL_PLUGIN_AEAD_H_ @}*/
