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
 * @defgroup gmp_rsa_public_key gmp_rsa_public_key
 * @{ @ingroup gmp_p
 */

#ifndef GMP_RSA_PUBLIC_KEY_H_
#define GMP_RSA_PUBLIC_KEY_H_

#include <credentials/builder.h>
#include <credentials/keys/public_key.h>

typedef struct gmp_rsa_public_key_t gmp_rsa_public_key_t;

/**
 * public_key_t implementation of RSA algorithm using libgmp.
 */
struct gmp_rsa_public_key_t {

	/**
	 * Implements the public_key_t interface
	 */
	public_key_t key;
};

/**
 * Load a RSA public key using libgmp.
 *
 * Accepts BUILD_RSA_* components.
 *
 * @param type		type of the key, must be KEY_RSA
 * @param args		builder_part_t argument list
 * @return 			loaded key, NULL on failure
 */
gmp_rsa_public_key_t *gmp_rsa_public_key_load(key_type_t type, va_list args);

#endif /** GMP_RSA_PUBLIC_KEY_H_ @}*/
