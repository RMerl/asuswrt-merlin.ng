/*
 * Copyright (C) 2015 Tobias Brunner
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
 * @defgroup hash_algorithm_set hash_algorithm_set
 * @{ @ingroup crypto
 */

#ifndef HASH_ALGORITHM_SET_H_
#define HASH_ALGORITHM_SET_H_

typedef struct hash_algorithm_set_t hash_algorithm_set_t;

#include <library.h>
#include <crypto/hashers/hasher.h>

/**
 * A set of hash algorithms
 */
struct hash_algorithm_set_t {

	/**
	 * Add the given algorithm to the set.
	 *
	 * @param alg		hash algorithm
	 */
	void (*add)(hash_algorithm_set_t *this, hash_algorithm_t alg);

	/**
	 * Check if the given algorithm is contained in the set.
	 *
	 * @param alg		hash algorithm
	 * @return			TRUE if contained in set
	 */
	bool (*contains)(hash_algorithm_set_t *this, hash_algorithm_t alg);

	/**
	 * Number of hash algorithms contained in the set.
	 *
	 * @return			number of algorithms
	 */
	int (*count)(hash_algorithm_set_t *this);

	/**
	 * Enumerate the algorithms contained in the set.
	 *
	 * @return			enumerator over hash_algorithm_t (sorted by identifier)
	 */
	enumerator_t *(*create_enumerator)(hash_algorithm_set_t *this);

	/**
	 * Destroy a hash_algorithm_set_t instance
	 */
	void (*destroy)(hash_algorithm_set_t *this);
};

/**
 * Create a set of hash algorithms.
 *
 * @return				hash_algorithm_set_t instance
 */
hash_algorithm_set_t *hash_algorithm_set_create();

#endif /** HASH_ALGORITHM_SET_H_ @}*/
