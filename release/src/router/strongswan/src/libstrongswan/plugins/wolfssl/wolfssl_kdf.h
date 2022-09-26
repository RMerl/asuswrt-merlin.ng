/*
 * Copyright (C) 2022 Tobias Brunner, codelabs GmbH
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
 * Implements key derivation functions (KDF) using wolfSSL, in particular prf+,
 * which is implemented via wolfSSL's HKDF implementation.
 *
 * @defgroup wolfssl_kdf wolfssl_kdf
 * @{ @ingroup wolfssl_p
 */

#ifndef WOLFSSL_KDF_H_
#define WOLFSSL_KDF_H_

#include <crypto/kdfs/kdf.h>

/**
 * Creates a new kdf_t object.
 *
 * @param algo		algorithm to instantiate
 * @param args		algorithm-specific arguments
 * @return			kdf_t object, NULL if not supported
 */
kdf_t *wolfssl_kdf_create(key_derivation_function_t algo, va_list args);

#endif /** WOLFSSL_KDF_H_ @}*/
