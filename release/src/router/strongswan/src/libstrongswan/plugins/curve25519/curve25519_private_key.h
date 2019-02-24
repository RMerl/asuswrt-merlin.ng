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
 * @defgroup curve25519_private_key curve25519_private_key
 * @{ @ingroup curve25519_p
 */

#ifndef CURVE25519_PRIVATE_KEY_H_
#define CURVE25519_PRIVATE_KEY_H_

#include <credentials/builder.h>
#include <credentials/keys/private_key.h>

typedef struct curve25519_private_key_t curve25519_private_key_t;

/**
 * Private_key_t implementation of Ed25519 signature algorithm.
 */
struct curve25519_private_key_t {

	/**
	 * Implements private_key_t interface
	 */
	private_key_t key;
};

/**
 * Generate an Ed25519 private key.
 *
 * @param type		type of the key, must be KEY_ED25519
 * @param args		builder_part_t argument list
 * @return 			generated key, NULL on failure
 */
curve25519_private_key_t *curve25519_private_key_gen(key_type_t type,
													 va_list args);

/**
 * Load an Ed25519 private key.
 *
 * @param type		type of the key, must be KEY_ED25519
 * @param args		builder_part_t argument list
 * @return 			loaded key, NULL on failure
 */
curve25519_private_key_t *curve25519_private_key_load(key_type_t type,
													  va_list args);

#endif /** CURVE25519_PRIVATE_KEY_H_ @}*/
