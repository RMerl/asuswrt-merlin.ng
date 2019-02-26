/*
 * Copyright (C) 2018 René Korthaus
 * Copyright (C) 2018 Konstantinos Kolelis
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
 * @defgroup botan_diffie_hellman botan_diffie_hellman
 * @{ @ingroup botan_p
 */

#ifndef BOTAN_DIFFIE_HELLMAN_H_
#define BOTAN_DIFFIE_HELLMAN_H_

typedef struct botan_diffie_hellman_t botan_diffie_hellman_t;

#include <crypto/diffie_hellman.h>

/**
 * Implementation of the Diffie-Hellman algorithm using Botan.
 */
struct botan_diffie_hellman_t {

	/**
	 * Implements diffie_hellman_t interface.
	 */
	diffie_hellman_t dh;
};

/**
 * Creates a new botan_diffie_hellman_t object.
 *
 * @param group			Diffie Hellman group number to use
 * @param ...			expects generator and prime as chunk_t if MODP_CUSTOM
 * @return				botan_diffie_hellman_t object,
 *						NULL if not supported
 */
botan_diffie_hellman_t *botan_diffie_hellman_create(
											diffie_hellman_group_t group, ...);

#endif /** BOTAN_DIFFIE_HELLMAN_H_ @}*/
