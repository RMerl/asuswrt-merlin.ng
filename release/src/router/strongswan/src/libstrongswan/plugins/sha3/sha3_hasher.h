/*
 * Copyright (C) 2015 Andreas Steffen
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
 * @defgroup sha3_hasher sha3_hasher
 * @{ @ingroup sha3_p
 */

#ifndef SHA3_HASHER_H_
#define SHA3_HASHER_H_

typedef struct sha3_hasher_t sha3_hasher_t;

#include <crypto/hashers/hasher.h>

/**
 * Implementation of hasher_t interface using the SHA-3 algorithm family
 * SHA3_224, SHA3_256, SHA3_384 and SHA3_512 as defined by FIPS-202.
 */
struct sha3_hasher_t {

	/**
	 * Generic hasher_t interface for this hasher.
	 */
	hasher_t hasher_interface;
};

/**
 * Creates a new sha3_hasher_t.
 *
 * @param	algorithm	HASH3_224, HASH_SHA3_256, HASH_SHA3_384 or HASH_SHA3_512
 * @return				sha3_hasher_t object, NULL if not supported
 */
sha3_hasher_t *sha3_hasher_create(hash_algorithm_t algorithm);

#endif /** SHA3_HASHER_H_ @}*/
