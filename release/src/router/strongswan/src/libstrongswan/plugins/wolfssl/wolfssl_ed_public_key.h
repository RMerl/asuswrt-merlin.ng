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
 * @defgroup wolfssl_ed_public_key wolfssl_ed_public_key
 * @{ @ingroup wolfssl_p
 */

#ifndef WOLFSSL_PLUGIN_ED_PUBLIC_KEY_H_
#define WOLFSSL_PLUGIN_ED_PUBLIC_KEY_H_

#include <credentials/builder.h>
#include <credentials/keys/public_key.h>

/**
 * Load an EdDSA public key using wolfSSL.
 *
 * Accepts a BUILD_BLOB_ASN1_DER argument.
 *
 * @param type		type of the key, must be KEY_ED25519 or KEY_ED448
 * @param args		builder_part_t argument list
 * @return 			loaded key, NULL on failure
 */
public_key_t *wolfssl_ed_public_key_load(key_type_t type, va_list args);

#endif /** WOLFSSL_PLUGIN_ED_PUBLIC_KEY_H_ @}*/
