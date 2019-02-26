/*
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2010 revosec AG
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
 * @defgroup pkcs11_hasher pkcs11_hasher
 * @{ @ingroup pkcs11
 */

#ifndef PKCS11_HASHER_H_
#define PKCS11_HASHER_H_

#include <crypto/hashers/hasher.h>

typedef struct pkcs11_hasher_t pkcs11_hasher_t;

/**
 * Hash implementation using a PKCS#11 token.
 */
struct pkcs11_hasher_t {

	/**
	 * Implements hasher_t interface.
	 */
	hasher_t hasher;
};

/**
 * Creates a PKCS#11 based hasher.
 *
 * @param algo		hash algorithm
 * @return			hasher, NULL if not supported
 */
pkcs11_hasher_t *pkcs11_hasher_create(hash_algorithm_t algo);

#endif /** PKCS11_HASHER_H_ @}*/
