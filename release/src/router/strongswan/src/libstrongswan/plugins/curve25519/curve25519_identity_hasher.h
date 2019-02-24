/*
 * Copyright (C) 2016 Andreas Steffen
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
 * @defgroup curve25519_identity_hasher curve25519_identity_hasher
 * @{ @ingroup curve25519_p
 */

#ifndef CURVE25519_IDENTITY_HASHER_H_
#define CURVE25519_IDENTITY_HASHER_H_

typedef struct curve25519_identity_hasher_t curve25519_identity_hasher_t;

#include <crypto/hashers/hasher.h>

/**
 * Implementation of hasher_t interface using the Identity algorithm.
 */
struct curve25519_identity_hasher_t {

	/**
	 * Implements hasher_t interface.
	 */
	hasher_t hasher_interface;
};

/**
 * Creates a new curve25519_identity_hasher_t.
 *
 * @param algo		algorithm, must be HASH_IDENTITY
 * @return			curve25519_identity_hasher_t object
 */
curve25519_identity_hasher_t *curve25519_identity_hasher_create(hash_algorithm_t algo);

#endif /** CURVE25519_IDENTITY_HASHER_H_ @}*/
