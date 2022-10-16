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
 * Compatibility code for legacy ENGINE support.
 *
 * @defgroup openssl_engine openssl_engine
 * @{ @ingroup openssl_p
 */

#ifndef OPENSSL_ENGINE_H_
#define OPENSSL_ENGINE_H_

#include <credentials/keys/private_key.h>

/**
 * Load a private key from a token/ENGINE.
 *
 * @param type		key type to load
 * @param args		build arguments
 */
private_key_t *openssl_private_key_connect(key_type_t type, va_list args);

/**
 * Initialize ENGINE support.
 */
void openssl_engine_init();

/**
 * Deinitialize ENGINE support.
 */
void openssl_engine_deinit();

#endif /** OPENSSL_ENGINE_H_ @}*/
