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
 * Implementation of the X25519/X448 Diffie-Hellman algorithm using wolfSSL.
 *
 * @defgroup wolfssl_x_diffie_hellman wolfssl_x_diffie_hellman
 * @{ @ingroup wolfssl_p
 */

#ifndef WOLFSSL_PLUGIN_X_DIFFIE_HELLMAN_H_
#define WOLFSSL_PLUGIN_X_DIFFIE_HELLMAN_H_

#include <library.h>

/**
 * Creates a new diffie_hellman_t object.
 *
 * @param group			Diffie Hellman group number to use
 * @return				object, NULL if not supported
 */
diffie_hellman_t *wolfssl_x_diffie_hellman_create(diffie_hellman_group_t group);

#endif /** WOLFSSL_PLUGIN_X_DIFFIE_HELLMAN_H_ @}*/
