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
 * Implements a KDF wrapper around PRFs, and prf+ as defined in RFC 7296,
 * section 2.13:
 *
 * @verbatim
     prf+ (K,S) = T1 | T2 | T3 | T4 | ...

     where:
     T1 = prf (K, S | 0x01)
     T2 = prf (K, T1 | S | 0x02)
     T3 = prf (K, T2 | S | 0x03)
     T4 = prf (K, T3 | S | 0x04)
     ...
   @endverbatim
 *
 * @defgroup kdf_kdf kdf_kdf
 * @{ @ingroup kdf_p
 */

#ifndef KDF_KDF_H_
#define KDF_KDF_H_

#include <crypto/kdfs/kdf.h>

/**
 * Create a kdf_t object
 *
 * @param algo			algorithm to instantiate
 * @param args			pseudo_random_function_t of the underlying PRF
 * @return				kdf_t object, NULL if not supported
 */
kdf_t *kdf_kdf_create(key_derivation_function_t algo, va_list args);

#endif /** KDF_KDF_H_ @}*/
