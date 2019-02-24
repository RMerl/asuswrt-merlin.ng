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
 * @defgroup gcrypt_rsa_private_key gcrypt_rsa_private_key
 * @{ @ingroup gcrypt_p
 */

#ifndef GCRYPT_RSA_PRIVATE_KEY_H_
#define GCRYPT_RSA_PRIVATE_KEY_H_

#include <credentials/builder.h>
#include <credentials/keys/private_key.h>

typedef struct gcrypt_rsa_private_key_t gcrypt_rsa_private_key_t;

/**
 * Private_key_t implementation of RSA algorithm using libgcrypt.
 */
struct gcrypt_rsa_private_key_t {

	/**
	 * Implements private_key_t interface
	 */
	private_key_t key;
};

/**
 * Generate a private key using gcrypt.
 *
 * Accepts the BUILD_KEY_SIZE argument.
 *
 * @param type		type of the key, must be KEY_RSA
 * @param args		builder_part_t argument list
 * @return			generated key, NULL on failure
 */
gcrypt_rsa_private_key_t *gcrypt_rsa_private_key_gen(key_type_t type,
													 va_list args);

/**
 * Load a gcrypt RSA private keys.
 *
 * Accepts BUILD_RSA_* components.
 *
 * @param type		type of the key, must be KEY_RSA
 * @param args		builder_part_t argument list
 * @return			loaded key, NULL on failure
 */
gcrypt_rsa_private_key_t *gcrypt_rsa_private_key_load(key_type_t type,
													  va_list args);

#endif /** GCRYPT_RSA_PRIVATE_KEY_H_ @}*/
