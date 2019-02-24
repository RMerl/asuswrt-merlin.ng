/*
 * Copyright (C) 2018 Tobias Brunner
 * HSR Hochschule fuer Technik Rapperswil
 *
 * Copyright (C) 2018 Atanas Filyanov
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
 * Implements the aead_t interface using Botan.
 *
 * @defgroup botan_aead botan_aead
 * @{ @ingroup botan_p
 */

#ifndef BOTAN_AEAD_H_
#define BOTAN_AEAD_H_

#include <crypto/aead.h>

/**
 * Constructor to create aead_t implementation.
 *
 * @param algo			algorithm to implement
 * @param key_size		key size in bytes
 * @param salt_size		size of implicit salt length
 * @return				aead_t object, NULL if not supported
 */
aead_t *botan_aead_create(encryption_algorithm_t algo, size_t key_size,
						  size_t salt_size);

#endif /** BOTAN_AEAD_H_ @}*/
