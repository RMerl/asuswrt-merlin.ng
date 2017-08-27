/*
 * Copyright (C) 2008 Tobias Brunner
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
 * @defgroup openssl_hasher openssl_hasher
 * @{ @ingroup openssl_p
 */

#ifndef OPENSSL_HASHER_H_
#define OPENSSL_HASHER_H_

typedef struct openssl_hasher_t openssl_hasher_t;

#include <crypto/hashers/hasher.h>

/**
 * Implementation of hashers using OpenSSL.
 */
struct openssl_hasher_t {

	/**
	 * Implements hasher_t interface.
	 */
	hasher_t hasher;
};

/**
 * Constructor to create openssl_hasher_t.
 *
 * @param algo			algorithm
 * @return				openssl_hasher_t, NULL if not supported
 */
openssl_hasher_t *openssl_hasher_create(hash_algorithm_t algo);

#endif /** OPENSSL_HASHER_H_ @}*/
