/*
 * Copyright (C) 2006-2008 Martin Willi
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
 * @defgroup sha2_hasher sha2_hasher
 * @{ @ingroup sha2_p
 */

#ifndef SHA2_HASHER_H_
#define SHA2_HASHER_H_

typedef struct sha2_hasher_t sha2_hasher_t;

#include <crypto/hashers/hasher.h>

/**
 * Implementation of hasher_t interface using the SHA2 algorithms.
 *
 * SHA2 is an other name for the SHA-256, SHA-384 and SHA-512 variants of
 * the SHA hash algorithm.
 */
struct sha2_hasher_t {

	/**
	 * Generic hasher_t interface for this hasher.
	 */
	hasher_t hasher_interface;
};

/**
 * Creates a new sha2_hasher_t.
 *
 * @param	algorithm	HASH_SHA256, HASH_SHA384 or HASH_SHA512
 * @return				sha2_hasher_t object, NULL if not supported
 */
sha2_hasher_t *sha2_hasher_create(hash_algorithm_t algorithm);

#endif /** SHA2_HASHER_H_ @}*/
