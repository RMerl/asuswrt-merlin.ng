/*
 * Copyright (C) 2018 René Korthaus
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
 * Implements HMAC based PRF and signer using Botan's HMAC functions.
 *
 * @defgroup botan_hmac botan_hmac
 * @{ @ingroup botan_p
 */

#ifndef BOTAN_HMAC_H_
#define BOTAN_HMAC_H_

#include <crypto/prfs/prf.h>
#include <crypto/signers/signer.h>

/**
 * Creates a new prf_t object based on an HMAC.
 *
 * @param algo		algorithm to implement
 * @return			prf_t object, NULL if not supported
 */
prf_t *botan_hmac_prf_create(pseudo_random_function_t algo);

/**
 * Creates a new signer_t object based on an HMAC.
 *
 * @param algo		algorithm to implement
 * @return			signer_t, NULL if not supported
 */
signer_t *botan_hmac_signer_create(integrity_algorithm_t algo);

#endif /** BOTAN_HMAC_H_ @}*/
