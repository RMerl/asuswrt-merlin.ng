/*
 * Copyright (C) 2018 Tobias Brunner
 * HSR Hochschule fuer Technik Rapperswil
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
 * Helper functions to load public and private keys in a generic way
 *
 * @defgroup botan_util_keys botan_util_keys
 * @{ @ingroup botan_p
 */

#ifndef BOTAN_UTIL_KEYS_H_
#define BOTAN_UTIL_KEYS_H_

#include <botan/ffi.h>

#include <credentials/keys/public_key.h>
#include <credentials/keys/private_key.h>

/**
 * Load a public key in subjectPublicKeyInfo encoding
 *
 * Accepts a BUILD_BLOB_ASN1_DER argument.
 *
 * @param type		type of the key
 * @param args		builder_part_t argument list
 * @return 			loaded key, NULL on failure
 */
public_key_t *botan_public_key_load(key_type_t type, va_list args);

/**
 * Load a private key in PKCS#8 encoding
 *
 * Accepts a BUILD_BLOB_ASN1_DER argument.
 *
 * @param type		type of the key
 * @param args		builder_part_t argument list
 * @return 			loaded key, NULL on failure
 */
private_key_t *botan_private_key_load(key_type_t type, va_list args);

#endif /** BOTAN_UTIL_KEYS_H_ @}*/
