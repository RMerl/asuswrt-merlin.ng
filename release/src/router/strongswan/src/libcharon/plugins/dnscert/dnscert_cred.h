/*
 * Copyright (C) 2013 Ruslan Marchenko
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
 * @defgroup dnscert_cred_i dnscert_cred
 * @{ @ingroup dnscert
 */

#ifndef DNSCERT_CRED_H_
#define DNSCERT_CRED_H_

#include <credentials/credential_set.h>
#include <resolver/resolver.h>

typedef struct dnscert_cred_t dnscert_cred_t;

/**
 * DNSCERT credential set.
 *
 * The dnscert credential set contains CERT RRs as certificates.
 */
struct dnscert_cred_t {

	/**
	 * Implements credential_set_t interface
	 */
	credential_set_t set;

	/**
	 * Destroy the dnscert_cred.
	 */
	void (*destroy)(dnscert_cred_t *this);
};

/**
 * Create a dnscert_cred instance which uses the given resolver
 * to query the DNS for CERT resource records.
 *
 * @param res		resolver to use (gets adopted)
 * @return			credential set
 */
dnscert_cred_t *dnscert_cred_create(resolver_t *res);

#endif /** DNSCERT_CRED_H_ @}*/
