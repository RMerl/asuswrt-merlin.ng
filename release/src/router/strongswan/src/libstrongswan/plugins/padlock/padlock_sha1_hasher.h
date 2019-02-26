/*
 * Copyright (C) 2008 Thomas Kallenberg
 * Copyright (C) 2008 Martin Willi
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
 * @defgroup padlock_sha1_hasher padlock_sha1_hasher
 * @{ @ingroup padlock_p
 */

#ifndef PADLOCK_SHA1_HASHER_H_
#define PADLOCK_SHA1_HASHER_H_

typedef struct padlock_sha1_hasher_t padlock_sha1_hasher_t;

#include <crypto/hashers/hasher.h>

/**
 * Implementation of hasher_t interface using the SHA1 algorithm.
 */
struct padlock_sha1_hasher_t {

	/**
	 * Implements hasher_t interface.
	 */
	hasher_t hasher;
};

/**
 * Creates a new sha1_hasher_t.
 *
 * @param algo		algorithm, must be HASH_SHA1
 * @return			sha1_hasher_t object
 */
padlock_sha1_hasher_t *padlock_sha1_hasher_create(hash_algorithm_t algo);

#endif /** SHA1_HASHER_H_ @}*/
