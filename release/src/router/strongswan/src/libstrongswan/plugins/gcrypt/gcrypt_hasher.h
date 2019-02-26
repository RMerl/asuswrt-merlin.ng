/*
 * Copyright (C) 2009 Martin Willi
 * HSR Hochschule fuer Technik Rapperswil
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
 * @defgroup gcrypt_hasher gcrypt_hasher
 * @{ @ingroup gcrypt_p
 */

#ifndef GCRYPT_HASHER_H_
#define GCRYPT_HASHER_H_

typedef struct gcrypt_hasher_t gcrypt_hasher_t;

#include <crypto/hashers/hasher.h>

/**
 * Implementation of hashers using libgcrypt.
 */
struct gcrypt_hasher_t {

	/**
	 * The hasher_t interface.
	 */
	hasher_t hasher;
};

/**
 * Constructor to create gcrypt_hasher_t.
 *
 * @param algo			algorithm
 * @return				gcrypt_hasher_t, NULL if not supported
 */
gcrypt_hasher_t *gcrypt_hasher_create(hash_algorithm_t algo);

#endif /** GCRYPT_HASHER_H_ @}*/
