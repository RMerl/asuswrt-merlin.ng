/*
 * Copyright (C) 2005-2008 Martin Willi
 * Copyright (C) 2005 Jan Hutter
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
 * @defgroup sha1_hasher sha1_hasher
 * @{ @ingroup sha1_p
 */

#ifndef SHA1_HASHER_H_
#define SHA1_HASHER_H_

typedef struct sha1_hasher_t sha1_hasher_t;

#include <crypto/hashers/hasher.h>

/**
 * Implementation of hasher_t interface using the SHA1 algorithm.
 */
struct sha1_hasher_t {

	/**
	 * Implements hasher_t interface.
	 */
	hasher_t hasher_interface;
};

/**
 * Creates a new sha1_hasher_t.
 *
 * @param algo		algorithm, must be HASH_SHA1
 * @return			sha1_hasher_t object
 */
sha1_hasher_t *sha1_hasher_create(hash_algorithm_t algo);

#endif /** SHA1_HASHER_H_ @}*/
