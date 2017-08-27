/*
 * Copyright (C) 2009 Martin Willi
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
 * @defgroup pgp_public_key pgp_public_key
 * @{ @ingroup pgp
 */

#ifndef PGP_BUILDER_H_
#define PGP_BUILDER_H_

#include <credentials/builder.h>
#include <credentials/keys/private_key.h>

/**
 * Load a generic or an RSA public key using PGP decoding.
 *
 * @param type		type of the key, either KEY_ANY or KEY_RSA
 * @param args		builder_part_t argument list
 * @return 			private key, NULL if failed
 */
public_key_t *pgp_public_key_load(key_type_t type, va_list args);

/**
 * Load a generic or RSA private key using PGP decoding.
 *
 * @param type		type of the key, either KEY_ANY or KEY_RSA
 * @param args		builder_part_t argument list
 * @return 			builder instance
 */
private_key_t *pgp_private_key_load(key_type_t type, va_list args);

#endif /** PGP_BUILDER_H_ @}*/
