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
 * Implements HMAC based PRF and signer using wolfSSL's HMAC functions.
 *
 * @defgroup wolfssl_hmac wolfssl_hmac
 * @{ @ingroup wolfssl_p
 */

#ifndef WOLFSSL_PLUGIN_HMAC_H_
#define WOLFSSL_PLUGIN_HMAC_H_

#include <crypto/prfs/prf.h>
#include <crypto/signers/signer.h>

/**
 * Creates a new prf_t object based on an HMAC.
 *
 * @param algo		algorithm to implement
 * @return			prf_t object, NULL if not supported
 */
prf_t *wolfssl_hmac_prf_create(pseudo_random_function_t algo);

/**
 * Creates a new signer_t object based on an HMAC.
 *
 * @param algo		algorithm to implement
 * @return			signer_t, NULL if not supported
 */
signer_t *wolfssl_hmac_signer_create(integrity_algorithm_t algo);

#endif /** WOLFSSL_PLUGIN_HMAC_H_ @}*/
