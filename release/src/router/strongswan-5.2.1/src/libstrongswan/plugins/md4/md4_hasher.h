/*
 * Copyright (C) 2008 Martin Willi
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
 * @defgroup md4_hasher md4_hasher
 * @{ @ingroup md4_p
 */

#ifndef MD4_HASHER_H_
#define MD4_HASHER_H_

typedef struct md4_hasher_t md4_hasher_t;

#include <crypto/hashers/hasher.h>

/**
 * Implementation of hasher_t interface using the MD4 algorithm.
 */
struct md4_hasher_t {

	/**
	 * Generic hasher_t interface for this hasher.
	 */
	hasher_t hasher_interface;
};

/**
 * Creates a new md4_hasher_t.
 *
 * @param algo		hash algorithm, must be HASH_MD4
 * @return			md4_hasher_t object, NULL if not supported
 */
md4_hasher_t *md4_hasher_create(hash_algorithm_t algo);

#endif /** MD4_HASHER_H_ @}*/
