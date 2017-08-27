/*
 * Copyright (C) 2010 Martin Willi
 * Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

/**
 * @defgroup openssl_sha1_prf openssl_sha1_prf
 * @{ @ingroup openssl_p
 */

#ifndef OPENSSL_SHA1_PRF_H_
#define OPENSSL_SHA1_PRF_H_

typedef struct openssl_sha1_prf_t openssl_sha1_prf_t;

#include <crypto/prfs/prf.h>

/**
 * Implementation of prf_t interface using keyed SHA1 algorithm as used
 * in EAP-AKA/FIPS_PRF.
 */
struct openssl_sha1_prf_t {

	/**
	 * Implements prf_t interface.
	 */
	prf_t prf;
};

/**
 * Creates a new openssl_sha1_prf_t.
 *
 * @param algo		algorithm, must be PRF_KEYED_SHA1
 * @return			sha1_keyed_prf_tobject
 */
openssl_sha1_prf_t *openssl_sha1_prf_create(pseudo_random_function_t algo);

#endif /** OPENSSL_SHA1_PRF_H_ @}*/
